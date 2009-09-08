
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#define TORRENT_MAX_ALERT_TYPES 32

#include <boost/utility/in_place_factory.hpp>
#include <boost/none.hpp>

#include "win32_exception.hpp"

#include "global/wtl_app.hpp"
#include "global/string_conv.hpp"
#include "global/ini_adapter.hpp"

#include "halIni.hpp"
#include "halTypes.hpp"
#include "halEvent.hpp"
#include "halSignaler.hpp"
#include "halSession.hpp"

#pragma warning (push, 1)
#	include <libtorrent/create_torrent.hpp>
#pragma warning (pop) 

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
	session_ = boost::in_place(libt::fingerprint(HALITE_FINGERPRINT));

	try
	{

	torrent_internal::the_session_ = &session_;

	session_->session::set_alert_mask(libt::alert::all_categories);		
	session_->add_extension(&libt::create_metadata_plugin);
	session_->add_extension(&libt::create_ut_pex_plugin);
	session_->set_max_half_open_connections(10);
	
	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"Loading BitTorrent.xml.", hal::event_logger::info)));		
	bittorrent_ini_.load_data();
	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"Loading torrent parameters.", hal::event_logger::info)));	
	the_torrents_.load_from_ini();
	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"Loading done!", hal::event_logger::info)));
	
#	if 0
	try
	{						
	if (fs::exists(hal::app().get_working_directory()/L"Torrents.xml"))
	{
		assert(false);
		{
		fs::wifstream ifs(hal::app().get_working_directory()/L"Torrents.xml");
	
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Loading old Torrents.xml")));
	
		TorrentMap torrents;
		boost::archive::xml_wiarchive ia(ifs);	
		ia >> boost::serialization::make_nvp("torrents", torrents);
		
		the_torrents_ = torrents;
		}
		
		event_log().post(shared_ptr<EventDetail>(new EventMsg(
			hal::wform(L"Total %1%.") % the_torrents_.size())));				
		
		fs::rename(hal::app().get_working_directory()/L"Torrents.xml", hal::app().get_working_directory()/L"Torrents.xml.safe.to.delete");

	}			
	}
	catch(const std::exception& e)
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventStdException(event_logger::fatal, e, L"Loading Old Torrents.xml")));
	}		
#	endif

	if (exists(hal::app().get_working_directory()/L"DHTState.bin"))
	{
		try
		{
			dht_state_ = haldecode(hal::app().get_working_directory()/L"DHTState.bin");
		}		
		catch(const std::exception& e)
		{
			event_log().post(shared_ptr<EventDetail>(
				new EventStdException(event_logger::critical, e, L"Loading DHTState.bin")));
		}
	}
	
	{	libt::session_settings settings = session_->settings();
		settings.user_agent = string("Halite ") + HALITE_VERSION_STRING;
		session_->set_settings(settings);
	}
	
	acquire_work_object();
	start_alert_handler();

	service_threads_.push_back(shared_thread_ptr(new 
		thread_t(bind(&bit_impl::service_thread, this, service_threads_.size()))));


	} HAL_GENERIC_FN_EXCEPTION_CATCH(L"bit_impl::bit_impl()")
}

bit_impl::~bit_impl()
{	
	try
	{
	HAL_DEV_MSG(L"Commence ~BitTorrent_impl"); 

	discard_work_object();

	stop_alert_handler();

	for (std::vector<shared_thread_ptr>::iterator i=service_threads_.begin(), e=service_threads_.end(); i != e; ++i)
		(*i)->join();

	//save_torrent_data();
	
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
		fs.add_file(to_utf8((*i).first.string()), (*i).second);
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
	for (web_seed_details_t::const_iterator i = params.web_seeds.begin(), e = params.web_seeds.end();
			i != e; ++i)
	{
		HAL_DEV_MSG(hal::wform(L"URL: %1%") % (*i).url);
		t.add_url_seed(to_utf8((*i).url));
	}

	HAL_DEV_MSG(L"DHT Nodes");
	for (dht_node_details_t::const_iterator i = params.dht_nodes.begin(), e = params.dht_nodes.end();
			i != e; ++i)
	{
		HAL_DEV_MSG(hal::wform(L"URL: %1%, port: %2%") % (*i).url % (*i).port);
		t.add_node(hal::make_pair(to_utf8((*i).url), (*i).port));
	}

	HAL_DEV_MSG(hal::wform(L"root_path: %1%") % params.root_path.string());

	set_piece_hashes(t, to_utf8(params.root_path.string()),
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
	mutex_t::scoped_lock l(mutex_);

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
	mutex_t::scoped_lock l(mutex_);

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

	void operator()(libt::external_ip_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_EXTERNAL_IP_ALERT))
					% hal::from_utf8_safe(a.message())
					% hal::from_utf8_safe(a.external_address.to_string()))
		)	);				
	}

	void operator()(libt::portmap_error_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_PORTMAP_ERROR_ALERT))
				% (a.type == 0 ? 
					hal::app().res_wstr(HAL_PORTMAP_TYPE_PMP) : 
					hal::app().res_wstr(HAL_PORTMAP_TYPE_UPNP)))
		)	);				
	}

	void operator()(libt::portmap_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(lbt_category_to_event(a.category()), a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_PORTMAP_ALERT))
				% (a.type == 0 ? 
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
				% hal::from_utf8_safe(a.error.message()))
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
		
		get(a.handle)->finished();	
	}
	
	void operator()(libt::torrent_paused_alert const& a) const
	{
		get(a.handle)->signals().torrent_paused();

		wstring err = get(a.handle)->check_error();

		if (err == L"")
		{
			event_log().post(shared_ptr<EventDetail>(
				new EventMsg((hal::wform(hal::app().res_wstr(LBT_EVENT_TORRENT_PAUSED)) 
						% get(a.handle)->name()), 
					event_logger::debug, a.timestamp())));
			
			HAL_DEV_MSG(hal::wform(L"Torrent Paused alert, %1%.") % get(a.handle)->name());

			get(a.handle)->locked_process_event(ev_paused_alert());
		}
		else
		{
			HAL_DEV_MSG(hal::wform(L"Torrent Error alert %2%, %1%.") % get(a.handle)->name() % err);

			event_log().post(shared_ptr<EventDetail>(
				new EventMsg((hal::wform(hal::app().res_wstr(HAL_TORRENT_ERROR_PAUSE_ALERT)) 
						% err 
						% get(a.handle)->name()), 
					event_logger::warning, a.timestamp())));

			get(a.handle)->locked_process_event(ev_error_alert(err));
		}
	}
	
	void operator()(libt::torrent_resumed_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(HAL_TORRENT_RESUME_ALERT)) 
					% get(a.handle)->name()), 
				event_logger::debug, a.timestamp())));

		get(a.handle)->locked_process_event(ev_resumed_alert());
	}
	
	void operator()(libt::save_resume_data_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(HAL_WRITE_RESUME_ALERT)) 
					% get(a.handle)->name()), 
				event_logger::info, a.timestamp())));

		if (a.resume_data)
			get(a.handle)->write_resume_data(*a.resume_data);

		get(a.handle)->signals().resume_data();
		get(a.handle)->locked_process_event(ev_resume_data_alert());
	}
	
	void operator()(libt::save_resume_data_failed_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg((hal::wform(hal::app().res_wstr(HAL_WRITE_RESUME_FAIL_ALERT)) 
					% get(a.handle)->name()), 
				event_logger::warning, a.timestamp())));

		get(a.handle)->locked_process_event(ev_resume_data_failed_alert());
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
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(event_logger::debug, a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_BLOCK_FINISHED_ALERT))
					% get(a.handle)->name()
					% a.block_index
					% a.piece_index)
		)	);				
	}
	
	void operator()(libt::block_downloading_alert const& a) const
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventGeneral(event_logger::debug, a.timestamp(),
				hal::wform(hal::app().res_wstr(HAL_BLOCK_DOWNLOADING_ALERT))
					% get(a.handle)->name()
					% a.block_index
					% a.piece_index)
		)	);				
	}
	
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
			torrent_internal_ptr p = bit_impl_.the_torrents_.get(from_utf8_safe(h.get_torrent_info().name())); 

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
		//mutex_t::scoped_lock l(mutex_);
		
		libt::handle_alert<
			libt::save_resume_data_alert,
			libt::save_resume_data_failed_alert,
			libt::external_ip_alert,
			libt::portmap_error_alert,
			libt::portmap_alert,
			libt::file_error_alert,
			libt::torrent_finished_alert,
			libt::torrent_paused_alert,
			libt::torrent_resumed_alert,
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
			libt::peer_blocked_alert
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
