
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "halPch.hpp"

#include "win32_exception.hpp"

#include "global/wtl_app.hpp"
#include "global/string_conv.hpp"
#include "global/ini_adapter.hpp"

#include "halIni.hpp"
#include "halTypes.hpp"
#include "halEvent.hpp"
#include "halSignaler.hpp"
#include "halSession.hpp"


namespace hal
{

bit_impl::bit_impl() :
	bittorrent_ini_(L"BitTorrent.xml"),
	the_torrents_(bittorrent_ini_),
	action_timer_(io_service_),
	alert_timer_(io_service_),
	keep_checking_(false),
	default_torrent_max_connections_(-1),
	default_torrent_max_uploads_(-1),
	default_torrent_download_(-1),
	default_torrent_upload_(-1),
	resolve_countries_(true),
	ip_filter_on_(false),
	ip_filter_loaded_(false),
	ip_filter_changed_(false),
	ip_filter_count_(0),
	dht_on_(false)
{
	session_ = boost::in_place(libt::fingerprint(HALITE_FINGERPRINT), 0, libt::alert::all_categories);

	try
	{

	if (exists(hal::app().get_working_directory()/L"libtorrent.state"))
	{
		try
		{
			std::vector<char> in;			
			boost::system::error_code ec;
			int pos;

			if (libt::load_file((hal::app().get_working_directory()/L"libtorrent.state").string(), in, ec) == 0)
			{
				libt::lazy_entry state;
				libt::lazy_bdecode(&in[0], &in[0] + in.size(), state, ec, &pos);

				session_->load_state(state);

				hal::event_log().post(shared_ptr<hal::EventDetail>(
					new hal::EventMsg(L"Loaded libtorrent State", hal::event_logger::info)));		
			}
		}		
		catch(const std::exception& e)
		{
			event_log().post(shared_ptr<EventDetail>(
				new EventStdException(event_logger::critical, e, L"Loading libtorrent State")));
		}
	}
	
	libt::session_settings s = session_->settings();
	s.half_open_limit = 10;
	s.ssl_listen = 0;
	s.user_agent = string("Halite ") + HALITE_VERSION_STRING;
	session_->set_settings(s);
	
	torrent_internal::set_the_session(&session_);
	
	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"Loading BitTorrent.xml.", hal::event_logger::info)));		
	bittorrent_ini_.load_data();
	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"Loading torrent parameters.", hal::event_logger::info)));	
	the_torrents_.load_from_ini();
	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"Loading done!", hal::event_logger::info)));

	//acquire_work_object();
	start_alert_handler();

	service_threads_.push_back(shared_thread_ptr(new 
		thread_t(boost::bind(&boost::asio::io_service::run, &io_service_))));


	} HAL_GENERIC_FN_EXCEPTION_CATCH(L"bit_impl::bit_impl()")
}

bit_impl::~bit_impl()
{	
	try
	{
	HAL_DEV_MSG(L"Commence ~BitTorrent_impl"); 

//	discard_work_object();

	stop_alert_handler();

	io_service_.stop();
	for (std::vector<shared_thread_ptr>::iterator i=service_threads_.begin(), e=service_threads_.end(); i != e; ++i)
		(*i)->join();
	
	HAL_DEV_MSG(L"Handler stopped!"); 
	if (ip_filter_changed_)
	{	
		HAL_DEV_MSG(L"IP Filter needs saving."); 

		fs::ofstream ofs(hal::app().get_working_directory()/L"IPFilter.bin", std::ios::binary);
//		boost::archive::binary_oarchive oba(ofs);
		
		libt::ip_filter::filter_tuple_t vectors = ip_filter_.export_filter();	
		
		std::vector<libt::ip_range<boost::asio::ip::address_v4> > v4(vectors.get<0>());
		std::vector<libt::ip_range<boost::asio::ip::address_v6> > v6(vectors.get<1>());
		
		v4.erase(std::remove(v4.begin(), v4.end(), 0), v4.end());
		v6.erase(std::remove(v6.begin(), v6.end(), 0), v6.end());

		write_vec_range(ofs, v4);
//		write_vec_range(ofs, v6);
	}	

	} HAL_GENERIC_FN_EXCEPTION_CATCH(L"~BitTorrent_impl")
}

bool bit_impl::listen_on(std::pair<int, int> const& range)
{
	try
	{

	boost::system::error_code ec;
		
	if (!session_->is_listening())
	{
		session_->listen_on(range, ec);
		return !ec;
	}
	else
	{
		int port = session_->listen_port();
			
		if (port < range.first || port > range.second)
		{				
			session_->listen_on(range, ec);
			return !ec;
		}
		else
		{
			signals.successful_listen();
				
			return true;
		}
	}
		
	}
	catch (const std::exception& e)
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventStdException(event_logger::fatal, e, L"From bit::listen_on.")));

		return false;
	}
	catch (...)
	{
		return false;
	}
}

int bit_impl::is_listening_on() 
{
	if (!session_->is_listening())
		return -1;	
	else
		return session_->listen_port();
}

void bit_impl::stop_listening()
{
	ensure_dht_off();
	boost::system::error_code ec;
	session_->listen_on(std::make_pair(0, 0), ec);
}

bool bit_impl::ensure_dht_on(const dht_settings& dht)
{		
	libt::dht_settings settings;
	settings.max_peers_reply = dht.max_peers_reply;
	settings.search_branching = dht.search_branching;
//	settings.service_port = dht.service_port;
	settings.max_fail_count = dht.max_fail_count;
		
//	HAL_DEV_MSG(hal::wform(L"Seleted DHT port = %1%") % settings.service_port);
		
	if (dht_settings_ != settings)
	{
		dht_settings_ = settings;
		session_->set_dht_settings(dht_settings_);
	}

	if (!dht_on_)
	{		
		try
		{
		session_->start_dht();

		// DTH state no longer saved!!!!!

		//session_->load_state(dht_state_);

		dht_on_ = true;
		}
		catch(...)
		{}
	}
		return dht_on_;
}

void bit_impl::ensure_dht_off()
{
	if (dht_on_)
	{
		session_->stop_dht();		
		dht_on_ = false;
	}
}

void bit_impl::set_mapping(bool upnp, bool nat_pmp)
{
	upnp = true;

	if (upnp)
	{
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Starting UPnP mapping.")));

		upnp_ = session_->start_upnp();
	}
	else
	{
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Stopping UPnP mapping.")));

		session_->stop_upnp();
		upnp_ = NULL;
	}

	if (nat_pmp)
	{
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Starting NAT-PMP mapping.")));

		natpmp_ = session_->start_natpmp();
	}
	else
	{
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Stopping NAT-PMP mapping.")));

		session_->stop_natpmp();
		natpmp_ = NULL;
	}
}

std::wstring bit_impl::upnp_router_model()
{
	if (upnp_)
		return to_wstr_shim(upnp_->router_model());
	else
		return L"UPnP not started";
}

void bit_impl::set_timeouts(int peers, int tracker)
{
	libt::session_settings settings = session_->settings();
	settings.peer_connect_timeout = peers;
	settings.tracker_completion_timeout = tracker;

	session_->set_settings(settings);

	event_log().post(shared_ptr<EventDetail>(new EventMsg(
		hal::wform(L"Set Timeouts, peer %1%, tracker %2%.") % peers % tracker)));
}

cache_settings bit_impl::get_cache_settings()
{
	libt::session_settings settings = session_->settings();
	cache_settings cache;

	cache.cache_size = settings.cache_size;
	cache.cache_expiry = settings.cache_expiry;

	return cache;
}

void bit_impl::set_cache_settings(const cache_settings& cache)
{
	libt::session_settings settings = session_->settings();

	settings.cache_size = cache.cache_size;
	settings.cache_expiry = cache.cache_expiry;

	session_->set_settings(settings);

	event_log().post(shared_ptr<EventDetail>(new EventMsg(
		hal::wform(L"Set cache parameters, %1% size and %2% expiry.") 
			% settings.cache_size % settings.cache_expiry)));
}

queue_settings bit_impl::get_queue_settings()
{		
	libt::session_settings settings = session_->settings();
	queue_settings queue;

	queue.auto_manage_interval = settings.auto_manage_interval;
	queue.active_downloads = settings.active_downloads;
	queue.active_seeds = settings.active_seeds;
	queue.seeds_hard_limit = settings.active_limit;
	queue.seed_ratio_limit = settings.share_ratio_limit;
	queue.seed_ratio_time_limit = settings.seed_time_ratio_limit;
	queue.seed_time_limit = settings.seed_time_limit;
	queue.dont_count_slow_torrents = settings.dont_count_slow_torrents;
	queue.auto_scrape_min_interval = settings.auto_scrape_min_interval;
	queue.auto_scrape_interval = settings.auto_scrape_interval;
	queue.close_redundant_connections = settings.close_redundant_connections;

	return queue;
}

void bit_impl::set_queue_settings(const queue_settings& queue)
{
	libt::session_settings settings = session_->settings();

	settings.auto_manage_interval = queue.auto_manage_interval;
	settings.active_downloads = queue.active_downloads;
	settings.active_seeds = queue.active_seeds;
	settings.active_limit = queue.seeds_hard_limit;
	settings.share_ratio_limit = queue.seed_ratio_limit;
	settings.seed_time_ratio_limit = queue.seed_ratio_time_limit;
	settings.seed_time_limit = queue.seed_time_limit;
	settings.dont_count_slow_torrents = queue.dont_count_slow_torrents;
	settings.auto_scrape_min_interval = queue.auto_scrape_min_interval;
	settings.auto_scrape_interval = queue.auto_scrape_interval;
	settings.close_redundant_connections = queue.close_redundant_connections;

	session_->set_settings(settings);

	event_log().post(shared_ptr<EventDetail>(new EventMsg(
		hal::wform(L"Set queue parameters, %1% downloads and %2% active seeds.") 
			% settings.active_downloads % settings.active_seeds)));
}

timeouts bit_impl::get_timeouts()
{		
	libt::session_settings settings = session_->settings();
	timeouts times;

	times.tracker_completion_timeout = settings.tracker_completion_timeout;
	times.tracker_receive_timeout = settings.tracker_receive_timeout;
	times.stop_tracker_timeout = settings.stop_tracker_timeout;

	times.request_queue_time = boost::numeric_cast<float>(settings.request_queue_time);
	times.piece_timeout = settings.piece_timeout;
	times.min_reconnect_time = settings.min_reconnect_time;		

	times.peer_timeout = settings.peer_timeout;
	times.urlseed_timeout = settings.urlseed_timeout;
	times.peer_connect_timeout = settings.peer_connect_timeout;
	times.inactivity_timeout = settings.inactivity_timeout;
	times.handshake_timeout = settings.handshake_timeout;

	return times;
}

void bit_impl::set_timeouts(const timeouts& times)
{
	libt::session_settings settings = session_->settings();

	settings.tracker_completion_timeout = times.tracker_completion_timeout;
	settings.tracker_receive_timeout = times.tracker_receive_timeout;
	settings.stop_tracker_timeout = times.stop_tracker_timeout;

	settings.request_queue_time = boost::numeric_cast<int>(times.request_queue_time);
	settings.piece_timeout = times.piece_timeout;
	settings.min_reconnect_time = times.min_reconnect_time;		

	settings.peer_timeout = times.peer_timeout;
	settings.urlseed_timeout = times.urlseed_timeout;
	settings.peer_connect_timeout = times.peer_connect_timeout;
	settings.inactivity_timeout = times.inactivity_timeout;
	settings.handshake_timeout = times.handshake_timeout;

	session_->set_settings(settings);

	event_log().post(shared_ptr<EventDetail>(new EventMsg(
		hal::wform(L"Set timeouts, peers- %1% secs, tracker- %2% secs.") 
			% settings.peer_timeout % settings.tracker_receive_timeout)));
}

void bit_impl::set_session_limits(int maxConn, int maxUpload)
{		
	libt::session_settings s = session_->settings();

	s.unchoke_slots_limit = maxUpload;
	s.connections_limit = maxConn;

	session_->set_settings(s);
		
	event_log().post(shared_ptr<EventDetail>(new EventMsg(
		hal::wform(L"Set connections totals %1% and uploads %2%.") 
			% maxConn % maxUpload)));
}

void bit_impl::set_session_speed(float download, float upload)
{
	libt::session_settings s = session_->settings();

	s.download_rate_limit = (download > 0) ? static_cast<int>(download*1024) : -1;
	s.upload_rate_limit = (upload > 0) ? static_cast<int>(upload*1024) : -1;

	session_->set_settings(s);
		
	event_log().post(shared_ptr<EventDetail>(new EventMsg(
		hal::wform(L"Set session rates at download %1% and upload %2%.") 
			% s.download_rate_limit % s.upload_rate_limit)));
}

cache_details bit_impl::get_cache_details() const
{
	libt::cache_status cs = session_->get_cache_status();

	return cache_details(cs.blocks_written, cs.writes, 
		cs.blocks_read, cs.blocks_read_hit, cs.reads,
		cs.cache_size, cs.read_cache_size);
}

bool bit_impl::ensure_ip_filter_on(progress_callback fn)
{
	try
	{
		
	if (!ip_filter_loaded_)
	{
		ip_filter_load(fn);
		ip_filter_loaded_ = true;
	}
		
	if (!ip_filter_on_)
	{
		session_->set_ip_filter(ip_filter_);
		ip_filter_on_ = true;
		ip_filter_count();
	}
		
	}
	catch(const std::exception& e)
	{		
		hal::event_log().post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(event_logger::critical, e, L"ensure_ip_filter_on"))); 

		ensure_ip_filter_off();
	}

	event_log().post(shared_ptr<EventDetail>(new EventMsg(L"IP filters on.")));	

	return false;
}

void bit_impl::ensure_ip_filter_off()
{
	session_->set_ip_filter(libt::ip_filter());
	ip_filter_on_ = false;
		
	event_log().post(shared_ptr<EventDetail>(new EventMsg(L"IP filters off.")));	
}

void bit_impl::ip_filter_count()
{
	libt::ip_filter::filter_tuple_t vectors = ip_filter_.export_filter();
	
	vectors.get<0>().erase(std::remove(vectors.get<0>().begin(), vectors.get<0>().end(), 0),
		vectors.get<0>().end());
	vectors.get<1>().erase(std::remove(vectors.get<1>().begin(), vectors.get<1>().end(), 0),
		vectors.get<1>().end());
	ip_filter_count_ = vectors.get<0>().size() + vectors.get<1>().size();
}

void bit_impl::ip_filter_load(progress_callback fn)
{
	fs::ifstream ifs(hal::app().get_working_directory()/L"IPFilter.bin", std::ios::binary);
	if (ifs)
	{
		size_t v4_size;
		ifs >> v4_size;
		
		size_t total = v4_size/100;
		size_t previous = 0;
		
		for(unsigned i=0; i<v4_size; ++i)
		{
			if (i-previous > total)
			{
				previous = i;

				if (fn) if (fn(i, v4_size, hal::app().res_wstr(HAL_TORRENT_LOAD_FILTERS))) break;
			}
			
			read_range_to_filter<boost::asio::ip::address_v4>(ifs, ip_filter_);
		}
	}	
}

void bit_impl::ip_filter_import(std::vector<libt::ip_range<boost::asio::ip::address_v4> >& v4,
	std::vector<libt::ip_range<boost::asio::ip::address_v6> >& v6)
{
	for(std::vector<libt::ip_range<boost::asio::ip::address_v4> >::iterator i=v4.begin();
		i != v4.end(); ++i)
	{
		ip_filter_.add_rule(i->first, i->last, libt::ip_filter::blocked);
	}
/*	for(std::vector<libt::ip_range<boost::asio::ip::address_v6> >::iterator i=v6.begin();
		i != v6.end(); ++i)
	{
		ip_filter_.add_rule(i->first, i->last, libt::ip_filter::blocked);
	}
*/	
	/* Note here we do not set ip_filter_changed_ */
}

bool bit_impl::ip_filter_import_dat(boost::filesystem::path file, progress_callback fn, bool octalFix)
{
	try
	{

	fs::ifstream ifs(file);	
	if (ifs)
	{
		boost::uintmax_t total = fs::file_size(file);
		boost::uintmax_t progress = 0;
		boost::uintmax_t previous = 0;
		
		boost::regex reg("\\s*(\\d+\\.\\d+\\.\\d+\\.\\d+)\\s*-\\s*(\\d+\\.\\d+\\.\\d+\\.\\d+)\\s*.*");
		boost::regex ip_reg("0*(\\d*)\\.0*(\\d*)\\.0*(\\d*)\\.0*(\\d*)");
		boost::smatch m;
		
		string ip_address_line;		
		while (!std::getline(ifs, ip_address_line).eof())
		{		
			progress += (ip_address_line.length() + 2);
			if (progress-previous > total)
			{
				previous = progress;
				if (fn)
				{
					if (fn(boost::numeric_cast<size_t>(progress), boost::numeric_cast<size_t>(total), 
							hal::app().res_wstr(HAL_TORRENT_IMPORT_FILTERS))) 
						break;
				}
			}
			
			if (boost::regex_match(ip_address_line, m, reg))
			{
				string first = m[1];
				string last = m[2];
				
				if (octalFix)
				{
					if (boost::regex_match(first, m, ip_reg))
					{
						first = ((m.length(1) != 0) ? m[1] : string("0")) + "." +
								((m.length(2) != 0) ? m[2] : string("0")) + "." +
								((m.length(3) != 0) ? m[3] : string("0")) + "." +
								((m.length(4) != 0) ? m[4] : string("0"));
					}					
					if (boost::regex_match(last, m, ip_reg))
					{
						last = ((m.length(1) != 0) ? m[1] : string("0")) + "." +
							   ((m.length(2) != 0) ? m[2] : string("0")) + "." +
							   ((m.length(3) != 0) ? m[3] : string("0")) + "." +
							   ((m.length(4) != 0) ? m[4] : string("0"));
					}
				}
				
				try
				{			
					ip_filter_.add_rule(boost::asio::ip::address_v4::from_string(first),
						boost::asio::ip::address_v4::from_string(last), libt::ip_filter::blocked);	
				}
				catch(...)
				{
					hal::event_log().post(shared_ptr<hal::EventDetail>(
						new hal::EventDebug(hal::event_logger::info, 
						from_utf8((boost::format("Invalid IP range: %1%-%2%.") % first % last).str()))));
				}
			}
		}
	}
	
	ip_filter_changed_ = true;
	ip_filter_count();
	
	}
	catch(const std::exception& e)
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventStdException(event_logger::critical, e, L"ip_filter_import_dat")));
	}

	return false;
}

bool bit_impl::create_torrent(const create_torrent_params& params, fs::wpath out_file, progress_callback fn)
{		
	try
	{
	libt::file_storage fs;
	libt::file_pool f_pool;

	HAL_DEV_MSG(L"Files");
	for (file_size_pairs_t::const_iterator i = params.file_size_pairs.begin(), e = params.file_size_pairs.end();
			i != e; ++i)
	{
		HAL_DEV_MSG(hal::wform(L"file path: %1%, size: %2%") % (*i).first % (*i).second);
		fs.add_file((*i).first.string(), (*i).second);
	}

	int piece_size = params.piece_size;
	HAL_DEV_MSG(hal::wform(L"piece size: %1%") % piece_size);
	
	libt::create_torrent t(fs, piece_size);
	
/*	boost::scoped_ptr<libt::storage_interface> store(
		libt::default_storage_constructor(t_info, to_utf8(params.root_path.string()),
			f_pool));
*/
	HAL_DEV_MSG(L"Trackers");
	for (tracker_details_t::const_iterator i = params.trackers.begin(), e = params.trackers.end();
			i != e; ++i)
	{
		HAL_DEV_MSG(hal::wform(L"URL: %1%, Tier: %2%") % (*i).url % (*i).tier);
		t.add_tracker(to_utf8((*i).url), (*i).tier);
	}

	HAL_DEV_MSG(L"Web Seeds");
/*	for (web_seed_details_t::const_iterator i = params.web_seeds.begin(), e = params.web_seeds.end();
			i != e; ++i)
	{
		HAL_DEV_MSG(hal::wform(L"URL: %1%") % (*i).url);
		t.add_url_seed(to_utf8((*i).url));
	}
*/
	HAL_DEV_MSG(L"DHT Nodes");
	for (dht_node_details_t::const_iterator i = params.dht_nodes.begin(), e = params.dht_nodes.end();
			i != e; ++i)
	{
		HAL_DEV_MSG(hal::wform(L"URL: %1%, port: %2%") % (*i).url % (*i).port);
		t.add_node(hal::make_pair(to_utf8((*i).url), (*i).port));
	}

	HAL_DEV_MSG(hal::wform(L"root_path: %1%") % params.root_path.wstring());

	set_piece_hashes(t, params.root_path.string(),
		boost::bind(fn, _1, t.num_pieces(), hal::app().res_wstr(HAL_NEWT_HASHING_PIECES)));

	t.set_creator(to_utf8(params.creator).c_str());
	t.set_comment(to_utf8(params.comment).c_str());
	
	t.set_priv(params.private_torrent);

	// create the torrent and print it to out
	libt::entry e = t.generate();
	
	HAL_DEV_MSG(hal::wform(L"Writing to: %1%") % out_file);
	fs::ofstream out(out_file, std::ios_base::binary);
	libt::bencode(std::ostream_iterator<char>(out), t.generate());

	} HAL_GENERIC_FN_EXCEPTION_CATCH(L"bit_impl::create_torrent()")
	
	HAL_DEV_MSG(L"Torrent creation completed!");

	return false;
}

void bit_impl::execute_action(const boost::system::error_code& e, bit::timeout_actions action)
{
	if (e != boost::asio::error::operation_aborted)
	{
		HAL_DEV_MSG(hal::wform(L"Doing action %1%") % action);

		switch(action)
		{
		case bit::action_pause:
			session_->pause();
			break;
		case bit::action_resume:
			session_->resume();
			break;
		default:
			break;
		};
	}
	else
	{
		HAL_DEV_MSG(hal::wform(L"Action %1% aborted") % action);
	}
}

void bit_impl::execute_callback(const boost::system::error_code& e, action_callback_t action)
{
	if (e != boost::asio::error::operation_aborted)
	{
		HAL_DEV_MSG(L"Doing callback");

		action();
	}
	else
	{
		HAL_DEV_MSG(L"Callback aborted");
	}
}

void bit_impl::schedual_action(boost::posix_time::ptime time, bit::timeout_actions action)
{
	HAL_DEV_MSG(hal::wform(L"Schedual absolute action %1% at %2%") % action % time);

	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	assert(time > now);

	schedual_action(time - now, action);
}

void bit_impl::schedual_action(boost::posix_time::time_duration duration, bit::timeout_actions action)
{
	HAL_DEV_MSG(hal::wform(L"Schedual relative action %1% in %2%") % action % duration);

	schedual_cancel();

	action_timer_.expires_from_now(duration);
	action_timer_.async_wait(bind(&bit_impl::execute_action, this, _1, action));
}

void bit_impl::schedual_callback(boost::posix_time::ptime time, action_callback_t action)
{
	HAL_DEV_MSG(hal::wform(L"Schedual absolute callback %1%") % time);

	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	assert(time > now);

	schedual_callback(time - now, action);
}

void bit_impl::schedual_callback(boost::posix_time::time_duration duration, action_callback_t action)
{
	HAL_DEV_MSG(hal::wform(L"Schedual relative callback %1%") % duration);

	schedual_cancel();

	action_timer_.expires_from_now(duration);
	action_timer_.async_wait(bind(&bit_impl::execute_callback, this, _1, action));
}	

void bit_impl::schedual_cancel()
{
	if (action_timer_.cancel() > 0)
		HAL_DEV_MSG(L"Schedualed action canceled");
}

void bit_impl::start_alert_handler()
{
	unique_lock_t l(mutex_);

	keep_checking_ = true;

	if (!alert_timer_.expires_at().is_special())
	{
		HAL_DEV_MSG(hal::wform(L"Alert handler already active"));

		return;
	}
	else
	{
		alert_timer_.expires_from_now(pt::milliseconds(100));

		HAL_DEV_MSG(L"Beginning timer async_wait");
		alert_timer_.async_wait(bind(&bit_impl::alert_handler_wait, this, _1));
	}



/*	if (alert_checker_ == boost::none)
	{	
		HAL_DEV_MSG(hal::wform(L"start_alert_handler"));

		boost::function<void (void)> f = bind(&bit_impl::alert_handler, this);

		keepChecking_ = true;
		alert_checker_ = boost::in_place<boost::function<void (void)> >(bind(&bit_impl::alert_handler, this));
	}	*/
}
	
void bit_impl::stop_alert_handler()
{
	unique_lock_t l(mutex_);

	if (keep_checking_ = false)
	{
		HAL_DEV_MSG(hal::wform(L"Stopped alert handler..."));
	}
	else
	{
		HAL_DEV_MSG(hal::wform(L"Alert handler already stopped"));
	}
}
	
void bit_impl::service_thread(size_t id)
{
	win32_exception::install_handler();

	HAL_DEV_MSG(hal::wform(L"Begining a service thread, id %1%") % id);

	for ( ; ; )
	{
		try
		{
			io_service_.run();

			// run exited normally
			break; 

		} 
		HAL_GENERIC_FN_EXCEPTION_CATCH(L"bit_impl::service_thread()")
	}

	HAL_DEV_MSG(hal::wform(L"Service thread id %1% exiting normally") % id);
}

void bit_impl::alert_handler_wait(const boost::system::error_code& e)
{	
	if (e != boost::asio::error::operation_aborted)
	{		
		if (keep_checking_)
		{
			alert_timer_.expires_from_now(pt::seconds(2));
			alert_timer_.async_wait(bind(&bit_impl::alert_handler_wait, this, _1));			
		}

		alert_handler();
	}
	else
	{
		HAL_DEV_MSG(L"Alert deadline canceled");
	}
}

void bit_impl::alert_handler()
{
	try
	{
	
	std::auto_ptr<libt::alert> p_alert = session_->pop_alert();
	
	class AlertHandler
	{
	public:
	AlertHandler(bit_impl& bit_impl) :
		bit_impl_(bit_impl)
	{}

	void operator()(libt::add_torrent_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(LBT_EVENT_TORRENT_ADDED)) 
					% get(a.handle)->name()), 
				event_logger::debug, a.timestamp())));
			
		HAL_DEV_MSG(hal::wform(L"Torrent Added alert, %1%.") % get(a.handle)->name());

		get(a.handle)->set_handle(a.handle);
		get(a.handle)->process_event(ev_added_alert((a.params.flags & libt::add_torrent_params::flag_paused) != 0, a.error));
	}

	void operator()(libt::external_ip_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_EXTERNAL_IP_ALERT))
					% hal::from_utf8_safe(a.message())
					% hal::from_utf8_safe(a.external_address.to_string()))
		)	);		

		if (!bit_impl_.use_custom_interface_ || !bit_impl_.external_interface_) 
			bit_impl_.external_interface_.reset(hal::from_utf8_safe(a.external_address.to_string()));
	}

	void operator()(libt::portmap_error_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_PORTMAP_ERROR_ALERT))
				% (a.type() == 0 ? 
					hal::app().res_wstr(HAL_PORTMAP_TYPE_PMP) : 
					hal::app().res_wstr(HAL_PORTMAP_TYPE_UPNP)))
		)	);				
	}

	void operator()(libt::portmap_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_PORTMAP_ALERT))
				% (a.type() == 0 ? 
					hal::app().res_wstr(HAL_PORTMAP_TYPE_PMP) : 
					hal::app().res_wstr(HAL_PORTMAP_TYPE_UPNP))
				% a.external_port)
		)	);				
	}
	
	void operator()(libt::file_error_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_FILE_ERROR_ALERT))
				% hal::from_utf8_safe(a.file)
				% hal::from_utf8_safe(a.message()))
		)	);				
	}
	
	void operator()(libt::dht_reply_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_DHT_REPLY_ALERT))
					% a.num_peers
					% get(a.handle)->name())
		)	);				
	}

	void operator()(libt::torrent_finished_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(LBT_EVENT_TORRENT_FINISHED)) 
					% get(a.handle)->name()), 
				event_logger::info, a.timestamp())));
		
		get(a.handle)->alert_finished();

		bit_impl_.signals.torrent_completed(get(a.handle)->name());
	}

	void operator()(libt::storage_moved_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(LBT_EVENT_TORRENT_STOREAGE_MOVED)) 
					% get(a.handle)->name()), 
				event_logger::info, a.timestamp())));
		
		get(a.handle)->alert_storage_moved(from_utf8(a.path));
	}

	void operator()(libt::storage_moved_failed_alert  const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(LBT_EVENT_TORRENT_FILE_RENAME_ERR)) 
					% get(a.handle)->name()
					% a.error), 
				event_logger::warning, a.timestamp())));
	}

	void operator()(libt::file_renamed_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(LBT_EVENT_TORRENT_FILE_RENAMED)) 
					% get(a.handle)->name()
					% a.index 
					% hal::from_utf8_safe(a.name)), 
				event_logger::debug, a.timestamp())));
	}

	void operator()(libt::file_rename_failed_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(LBT_EVENT_TORRENT_FILE_RENAME_ERR)) 
					% get(a.handle)->name()
					% a.index), 
				event_logger::warning, a.timestamp())));
	}

	void operator()(libt::file_completed_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(LBT_EVENT_TORRENT_FILE_COMPLETED)) 
					% get(a.handle)->name()
					% a.index), 
				event_logger::info, a.timestamp())));
		
		get(a.handle)->alert_file_completed(a.index);	
	}

	void operator()(libt::metadata_received_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(LBT_EVENT_TORRENT_METADATA)) 
					% get(a.handle)->name()), 
				event_logger::info, a.timestamp())));
		
		get(a.handle)->alert_metadata_completed();	
	}

	void operator()(libt::metadata_failed_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(LBT_EVENT_TORRENT_METADATA_FAILED)) 
					% get(a.handle)->name()), 
				event_logger::critical, a.timestamp())));
	}
	
	void operator()(libt::torrent_paused_alert const& a) const
	{
	//	get(a.handle)->signals().torrent_paused();

		wstring err = get(a.handle)->check_error();

		if (err.empty())
		{
			event_log().post(shared_ptr<EventDetail>(
				new EventMsg((hal::wform(hal::app().res_wstr(LBT_EVENT_TORRENT_PAUSED)) 
						% get(a.handle)->name()), 
					event_logger::debug, a.timestamp())));
			
			HAL_DEV_MSG(hal::wform(L"Torrent Paused alert, %1%.") % get(a.handle)->name());

			get(a.handle)->process_event(ev_paused_alert());
		}
		else
		{
			HAL_DEV_MSG(hal::wform(L"Torrent Error alert %2%, %1%.") % get(a.handle)->name() % err);

			event_log().post(shared_ptr<EventDetail>(
				new EventMsg((hal::wform(hal::app().res_wstr(HAL_TORRENT_ERROR_PAUSE_ALERT)) 
						% err 
						% get(a.handle)->name()), 
					event_logger::warning, a.timestamp())));

			get(a.handle)->process_event(ev_error_alert(err));
		}
	}
	
	void operator()(libt::torrent_resumed_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(HAL_TORRENT_RESUME_ALERT)) 
					% get(a.handle)->name()), 
				event_logger::debug, a.timestamp())));

		get(a.handle)->process_event(ev_resumed_alert());
	}
	
	void operator()(libt::save_resume_data_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(HAL_WRITE_RESUME_ALERT)) 
					% get(a.handle)->name()), 
				event_logger::info, a.timestamp())));

		if (a.resume_data)
			get(a.handle)->write_resume_data(*a.resume_data);

	//	get(a.handle)->signals().resume_data();
		get(a.handle)->process_event(ev_resume_data_alert());
	}
	
	void operator()(libt::save_resume_data_failed_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(HAL_WRITE_RESUME_FAIL_ALERT)) 
					% get(a.handle)->name()), 
				event_logger::warning, a.timestamp())));

		get(a.handle)->process_event(ev_resume_data_failed_alert());
	}
	
	void operator()(libt::peer_error_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_PEER_ALERT))
					% hal::from_utf8_safe(a.message())
					% hal::from_utf8_safe(a.ip.address().to_string()))
		)	);				
	}
		
	void operator()(libt::peer_ban_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_PEER_BAN_ALERT))
					% get(a.handle)->name()
					% hal::from_utf8_safe(a.ip.address().to_string()))
		)	);				
	}
		
	void operator()(libt::hash_failed_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_HASH_FAIL_ALERT))
					% get(a.handle)->name()
					% a.piece_index)
		)	);				
	}
		
	void operator()(libt::url_seed_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_URL_SEED_ALERT))
					% get(a.handle)->name()
					% hal::from_utf8_safe(a.url)
					% hal::from_utf8_safe(a.message()))
		)	);				
	}
	
	void operator()(libt::tracker_warning_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_TRACKER_WARNING_ALERT))
					% get(a.handle)->name()
					% hal::from_utf8_safe(a.message()))
		)	);				
	}
	
	void operator()(libt::tracker_announce_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(HAL_TRACKER_ANNOUNCE_ALERT)) 
					% get(a.handle)->name()), 
				event_logger::info, a.timestamp())));
	}
	
	void operator()(libt::tracker_error_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_TRACKER_ALERT))
					% get(a.handle)->name()
					% hal::from_utf8_safe(a.message())
					% a.times_in_row
					% a.status_code)
		)	);				
	}
	
	void operator()(libt::tracker_reply_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_TRACKER_REPLY_ALERT))
					% get(a.handle)->name()
					% hal::from_utf8_safe(a.message())
					% a.num_peers)
		)	);				
	}
	
	void operator()(libt::fastresume_rejected_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_FAST_RESUME_ALERT))
					% get(a.handle)->name()
					% hal::from_utf8_safe(a.message()))
		)	);				
	}
	
	void operator()(libt::piece_finished_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(event_logger::debug, a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_PIECE_FINISHED_ALERT))
					% get(a.handle)->name()
					% a.piece_index)
		)	);				
	}
	
	void operator()(libt::block_finished_alert const& a) const
	{
/*		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(event_logger::debug, a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_BLOCK_FINISHED_ALERT))
					% get(a.handle)->name()
					% a.block_index
					% a.piece_index)
		)	);				
*/	}
	
	void operator()(libt::block_downloading_alert const& a) const
	{
/*		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(event_logger::debug, a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_BLOCK_DOWNLOADING_ALERT))
					% get(a.handle)->name()
					% a.block_index
					% a.piece_index)
		)	);				
*/	}
	
	void operator()(libt::listen_failed_alert const& a) const
	{
		if (a.endpoint.address().is_v6())
		{	
			event_log().post(shared_ptr<EventDetail>(
				new EventGeneral(event_logger::info, a.timestamp(),
					hal::app().res_wstr(HAL_LISTEN_V6_FAILED_ALERT))
			)	);		
		}
		else
		{
			event_log().post(shared_ptr<EventDetail>(
				new EventGeneral(event_logger::info, a.timestamp(),
					hal::wform(hal::app().res_wstr(HAL_LISTEN_FAILED_ALERT))
						% hal::from_utf8_safe(a.message()))
			)	);
		}
	}
	
	void operator()(libt::listen_succeeded_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(event_logger::info, a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_LISTEN_SUCCEEDED_ALERT))
					% hal::from_utf8_safe(a.message()))
		)	);	

		//bit_impl_.signals.successful_listen();
	}
	
	void operator()(libt::peer_blocked_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(event_logger::debug, a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_IPFILTER_ALERT))
					% hal::from_utf8_safe(a.ip.to_string())
					% hal::from_utf8_safe(a.message()))
		)	);				
	}
	
/*	void operator()(libt::alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
				new EventLibtorrent(lbtAlertToHalEvent(a.severity()), 
					a.timestamp(), event_logger::unclassified, hal::from_utf8_safe(a.message()))));		
	}*/
	
	private:
		bit_impl& bit_impl_;
		
		torrent_internal_ptr get(libt::torrent_handle h) const 
		{ 
			torrent_internal_ptr p = bit_impl_.the_torrents_.get_by_hash(h.info_hash()); 

			if (p)
				return p;
			else
				throw bit::null_torrent();
		}
	
	} handler(*this);
	
	while (p_alert.get())
	{	
		try
		{
		//unique_lock_t l(mutex_);
		
		libt::handle_alert<
			libt::add_torrent_alert,
			libt::save_resume_data_alert,
			libt::save_resume_data_failed_alert,
			libt::metadata_failed_alert,
			libt::metadata_received_alert,
			libt::external_ip_alert,
			libt::portmap_error_alert,
			libt::portmap_alert,
			libt::file_error_alert,
			libt::torrent_finished_alert,
			libt::torrent_paused_alert,
			libt::torrent_resumed_alert,
			libt::file_renamed_alert,
			libt::file_rename_failed_alert,
			libt::file_completed_alert,
			libt::peer_error_alert,
			libt::peer_ban_alert,
			libt::hash_failed_alert,
			libt::url_seed_alert,
			libt::dht_reply_alert,
			libt::tracker_error_alert,
			libt::tracker_warning_alert,
			libt::tracker_announce_alert,
			libt::tracker_reply_alert,
			libt::fastresume_rejected_alert,
			libt::piece_finished_alert,
			libt::block_finished_alert,
			libt::block_downloading_alert,
			libt::listen_failed_alert,
			libt::listen_succeeded_alert,
			libt::peer_blocked_alert,
			libt::storage_moved_alert,
			libt::storage_moved_failed_alert
		>::handle_alert(p_alert, handler);			
		
		}
		catch(const libt::unhandled_alert&)
		{
//			handler(*p_alert);
		}
		catch(const bit::null_torrent&)
		{
			// These are logged as debug because they are rarely important to act on!
			event_log().post(shared_ptr<EventDetail>(\
				new EventMsg(L"null_torrent exception", event_logger::info)));
		}
		catch(const std::exception& e)
		{
			// These are logged as debug because they are rarely important to act on!
			event_log().post(shared_ptr<EventDetail>(\
				new EventStdException(event_logger::debug, e, L"bit_impl::alert_handler()")));
		}
		
		p_alert = session_->pop_alert();
	}	
			
	} 
	HAL_GENERIC_FN_EXCEPTION_CATCH(L"bit_impl::alert_handler()")
}

}
