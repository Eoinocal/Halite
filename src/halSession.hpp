
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/tuple/tuple.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/tag.hpp>

#pragma warning (push, 1)
#	include <libtorrent/file.hpp>
#	include <libtorrent/hasher.hpp>
#	include <libtorrent/storage.hpp>
#	include <libtorrent/file_pool.hpp>
#	include <libtorrent/alert_types.hpp>
#	include <libtorrent/entry.hpp>
#	include <libtorrent/bencode.hpp>
#	include <libtorrent/upnp.hpp>
#	include <libtorrent/natpmp.hpp>
#	include <libtorrent/session.hpp>
#	include <libtorrent/ip_filter.hpp>
#	include <libtorrent/torrent_handle.hpp>
#	include <libtorrent/peer_connection.hpp>
#	include <libtorrent/extensions/metadata_transfer.hpp>
#	include <libtorrent/extensions/ut_pex.hpp>
#	include <libtorrent/extensions/ut_metadata.hpp>
#	include <libtorrent/extensions/smart_ban.hpp>
#pragma warning (pop) 

#include "halIni.hpp"
#include "halTypes.hpp"
#include "halEvent.hpp"
#include "halConfig.hpp"
#include "halTorrentInternal.hpp"
#include "halTorrentManager.hpp"
#include "halSignaler.hpp"
#include "halCatchDefines.hpp"

namespace boost {
namespace serialization {

template<class Archive, class address_type>
void save(Archive& ar, const address_type& ip, const unsigned int version)
{	
	unsigned long addr = ip.to_ulong();	
	ar & BOOST_SERIALIZATION_NVP(addr);
}

template<class Archive, class address_type>
void load(Archive& ar, address_type& ip, const unsigned int version)
{	
	unsigned long addr;
	ar & BOOST_SERIALIZATION_NVP(addr);	
	ip = address_type(addr);
}

template<class Archive, class String, class Traits>
void save(Archive& ar, const boost::filesystem::basic_path<String, Traits>& p, const unsigned int version)
{	
	String str = p.string();
	ar & BOOST_SERIALIZATION_NVP(str);
}

template<class Archive, class String, class Traits>
void load(Archive& ar, boost::filesystem::basic_path<String, Traits>& p, const unsigned int version)
{	
	String str;
	ar & BOOST_SERIALIZATION_NVP(str);

	p = str;
}

template<class Archive, class String, class Traits>
inline void serialize(
        Archive & ar,
        boost::filesystem::basic_path<String, Traits>& p,
        const unsigned int file_version)
{
        split_free(ar, p, file_version);            
}

template<class Archive, class address_type>
void serialize(Archive& ar, libtorrent::ip_range<address_type>& addr, const unsigned int version)
{	
	ar & BOOST_SERIALIZATION_NVP(addr.first);
	ar & BOOST_SERIALIZATION_NVP(addr.last);
	addr.flags = libtorrent::ip_filter::blocked;
}

template<class Archive>
void serialize(Archive& ar, hal::tracker_detail& tracker, const unsigned int version)
{	
	ar & BOOST_SERIALIZATION_NVP(tracker.url);
	ar & BOOST_SERIALIZATION_NVP(tracker.tier);
}

} // namespace serialization
} // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(asio::ip::address_v4)
BOOST_SERIALIZATION_SPLIT_FREE(asio::ip::address_v6)

namespace libtorrent
{

template<class Addr>
bool operator==(const libtorrent::ip_range<Addr>& lhs, const int flags)
{
	return (lhs.flags == flags);
}

inline std::ostream& operator<<(std::ostream& os, libtorrent::ip_range<asio::ip::address_v4>& ip)
{
	os << ip.first.to_ulong();
	os << ip.last.to_ulong();
	
	return os;
}

} // namespace libtorrent

namespace hal
{

namespace libt = libtorrent;

inline bool operator!=(const libt::dht_settings& lhs, const libt::dht_settings& rhs)
{
	return lhs.max_peers_reply != rhs.max_peers_reply ||
		   lhs.search_branching != rhs.search_branching ||
		   lhs.service_port != rhs.service_port ||
           lhs.max_fail_count != rhs.max_fail_count;
}

template<typename Addr>
void write_range(fs::ofstream& ofs, const libt::ip_range<Addr>& range)
{ 
	const typename Addr::bytes_type first = range.first.to_bytes();
	const typename Addr::bytes_type last = range.last.to_bytes();
	ofs.write((char*)first.elems, first.size());
	ofs.write((char*)last.elems, last.size());
}

template<typename Addr>
void write_vec_range(fs::ofstream& ofs, const std::vector<libt::ip_range<Addr> >& vec)
{ 
	ofs << vec.size();
	
	for (typename std::vector<libt::ip_range<Addr> >::const_iterator i=vec.begin(); 
		i != vec.end(); ++i)
	{
		write_range(ofs, *i);
	}
}

template<typename Addr>
void read_range_to_filter(fs::ifstream& ifs, libt::ip_filter& ip_filter)
{ 
	typename Addr::bytes_type first;
	typename Addr::bytes_type last;
	ifs.read((char*)first.elems, first.size());
	ifs.read((char*)last.elems, last.size());	
	
	ip_filter.add_rule(Addr(first), Addr(last),
		libt::ip_filter::blocked);
}

static event_logger::eventLevel lbtAlertToHalEvent(libt::alert::severity_t severity)
{
	switch (severity)
	{
	case libt::alert::debug:
		return event_logger::debug;
	
	case libt::alert::info:
		return event_logger::info;
	
	case libt::alert::warning:
		return event_logger::warning;
	
	case libt::alert::critical:
	case libt::alert::fatal:
		return event_logger::critical;
	
	default:
		return event_logger::none;
	}
}

static event_logger::eventLevel lbt_category_to_event(int category)
{
	switch (category)
	{
	case libt::alert::debug_notification:
		return event_logger::debug;
	
	case libt::alert::peer_notification:
	case libt::alert::port_mapping_notification:
	case libt::alert::storage_notification:
	case libt::alert::tracker_notification:
	case libt::alert::status_notification:
	case libt::alert::progress_notification:
	case libt::alert::ip_block_notification:
		return event_logger::info;
	
	case libt::alert::performance_warning:
		return event_logger::warning;
	
	case libt::alert::error_notification:
		return event_logger::critical;
	
	default:
		return event_logger::none;
	}
}

struct uninitialized;

class bit_impl :
	public sc::state_machine<bit_impl, uninitialized>
{
	friend class bit;

private:
	bit_impl();

public:	
	~bit_impl();

	bool listen_on(std::pair<int, int> const& range)
	{
		try
		{
		
		if (!session_->is_listening())
		{
			return session_->listen_on(range);
		}
		else
		{
			int port = session_->listen_port();
			
			if (port < range.first || port > range.second)
				return session_->listen_on(range);	
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
				new EventStdException(event_logger::fatal, e, L"From bit::listenOn.")));

			return false;
		}
		catch (...)
		{
			return false;
		}
	}

	int is_listening_on() 
	{
		if (!session_->is_listening())
			return -1;	
		else
			return session_->listen_port();
	}

	void stop_listening()
	{
		ensure_dht_off();
		session_->listen_on(std::make_pair(0, 0));
	}

	bool ensure_dht_on(const dht_settings& dht)
	{		
		libt::dht_settings settings;
		settings.max_peers_reply = dht.max_peers_reply;
		settings.search_branching = dht.search_branching;
		settings.service_port = dht.service_port;
		settings.max_fail_count = dht.max_fail_count;
		
		HAL_DEV_MSG(hal::wform(L"Seleted DHT port = %1%") % settings.service_port);
		
		if (dht_settings_ != settings)
		{
			dht_settings_ = settings;
			session_->set_dht_settings(dht_settings_);
		}

		if (!dht_on_)
		{		
			try
			{
			session_->start_dht(dht_state_);
			dht_on_ = true;
			}
			catch(...)
			{}
		}
			return dht_on_;
	}

	void ensure_dht_off()
	{
		if (dht_on_)
		{
			session_->stop_dht();		
			dht_on_ = false;
		}
	}

	void set_mapping(bool upnp, bool nat_pmp)
	{
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

	std::wstring upnp_router_model()
	{
		if (upnp_)
			return to_wstr_shim(upnp_->router_model());
		else
			return L"UPnP not started";
	}

	void set_timeouts(int peers, int tracker)
	{
		libt::session_settings settings = session_->settings();
		settings.peer_connect_timeout = peers;
		settings.tracker_completion_timeout = tracker;

		session_->set_settings(settings);

		event_log().post(shared_ptr<EventDetail>(new EventMsg(
			hal::wform(L"Set Timeouts, peer %1%, tracker %2%.") % peers % tracker)));
	}

	cache_settings get_cache_settings()
	{
		libt::session_settings settings = session_->settings();
		cache_settings cache;

		cache.cache_size = settings.cache_size;
		cache.cache_expiry = settings.cache_expiry;

		return cache;
	}

	void set_cache_settings(const cache_settings& cache)
	{
		libt::session_settings settings = session_->settings();

		settings.cache_size = cache.cache_size;
		settings.cache_expiry = cache.cache_expiry;

		session_->set_settings(settings);

		event_log().post(shared_ptr<EventDetail>(new EventMsg(
			hal::wform(L"Set cache parameters, %1% size and %2% expiry.") 
				% settings.cache_size % settings.cache_expiry)));
	}

	queue_settings get_queue_settings()
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

	void set_queue_settings(const queue_settings& queue)
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

	timeouts get_timeouts()
	{		
		libt::session_settings settings = session_->settings();
		timeouts times;

		times.tracker_completion_timeout = settings.tracker_completion_timeout;
		times.tracker_receive_timeout = settings.tracker_receive_timeout;
		times.stop_tracker_timeout = settings.stop_tracker_timeout;

		times.request_queue_time = settings.request_queue_time;
		times.piece_timeout = settings.piece_timeout;
		times.min_reconnect_time = settings.min_reconnect_time;		

		times.peer_timeout = settings.peer_timeout;
		times.urlseed_timeout = settings.urlseed_timeout;
		times.peer_connect_timeout = settings.peer_connect_timeout;
		times.inactivity_timeout = settings.inactivity_timeout;
		times.handshake_timeout = settings.handshake_timeout;

		return times;
	}

	void set_timeouts(const timeouts& times)
	{
		libt::session_settings settings = session_->settings();

		settings.tracker_completion_timeout = times.tracker_completion_timeout;
		settings.tracker_receive_timeout = times.tracker_receive_timeout;
		settings.stop_tracker_timeout = times.stop_tracker_timeout;

		settings.request_queue_time = times.request_queue_time;
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

	void set_session_limits(int maxConn, int maxUpload)
	{		
		session_->set_max_uploads(maxUpload);
		session_->set_max_connections(maxConn);
		
		event_log().post(shared_ptr<EventDetail>(new EventMsg(
			hal::wform(L"Set connections totals %1% and uploads %2%.") 
				% maxConn % maxUpload)));
	}

	void set_session_speed(float download, float upload)
	{
		int down = (download > 0) ? static_cast<int>(download*1024) : -1;
		session_->set_download_rate_limit(down);
		int up = (upload > 0) ? static_cast<int>(upload*1024) : -1;
		session_->set_upload_rate_limit(up);
		
		event_log().post(shared_ptr<EventDetail>(new EventMsg(
			hal::wform(L"Set session rates at download %1% and upload %2%.") 
				% session_->download_rate_limit() % session_->upload_rate_limit())));
	}

	cache_details get_cache_details() const
	{
		libt::cache_status cs = session_->get_cache_status();

		return cache_details(cs.blocks_written, cs.writes, 
			cs.blocks_read, cs.blocks_read_hit, cs.reads,
			cs.cache_size, cs.read_cache_size);
	}

	bool ensure_ip_filter_on(progress_callback fn)
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
				new hal::EventStdException(event_logger::critical, e, L"ensureIpFilterOn"))); 

			ensure_ip_filter_off();
		}

		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"IP filters on.")));	

		return false;
	}

	void ensure_ip_filter_off()
	{
		session_->set_ip_filter(libt::ip_filter());
		ip_filter_on_ = false;
		
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"IP filters off.")));	
	}

#	ifndef TORRENT_DISABLE_ENCRYPTION	
	void ensure_pe_on(const pe_settings& pe_s)
	{
		libt::pe_settings pe;
		
		switch (pe_s.encrypt_level)
		{
			case 0:
				pe.allowed_enc_level = libt::pe_settings::plaintext;
				break;
			case 1:
				pe.allowed_enc_level = libt::pe_settings::rc4;
				break;
			case 2:
				pe.allowed_enc_level = libt::pe_settings::both;
				break;
			default:
				pe.allowed_enc_level = libt::pe_settings::both;
				
				hal::event_log().post(shared_ptr<hal::EventDetail>(
					new hal::EventGeneral(hal::event_logger::warning, hal::event_logger::unclassified, 
						(hal::wform(hal::app().res_wstr(HAL_INCORRECT_ENCODING_LEVEL)) % pe_s.encrypt_level).str())));
		}

		switch (pe_s.conn_in_policy)
		{
			case 0:
				pe.in_enc_policy = libt::pe_settings::forced;
				break;
			case 1:
				pe.in_enc_policy = libt::pe_settings::enabled;
				break;
			case 2:
				pe.in_enc_policy = libt::pe_settings::disabled;
				break;
			default:
				pe.in_enc_policy = libt::pe_settings::enabled;
				
				hal::event_log().post(shared_ptr<hal::EventDetail>(
					new hal::EventGeneral(hal::event_logger::warning, hal::event_logger::unclassified, 
						(hal::wform(hal::app().res_wstr(HAL_INCORRECT_CONNECT_POLICY)) % pe_s.conn_in_policy).str())));
		}

		switch (pe_s.conn_out_policy)
		{
			case 0:
				pe.out_enc_policy = libt::pe_settings::forced;
				break;
			case 1:
				pe.out_enc_policy = libt::pe_settings::enabled;
				break;
			case 2:
				pe.out_enc_policy = libt::pe_settings::disabled;
				break;
			default:
				pe.out_enc_policy = libt::pe_settings::enabled;
				
				hal::event_log().post(shared_ptr<hal::EventDetail>(
					new hal::EventGeneral(hal::event_logger::warning, hal::event_logger::unclassified, 
						(hal::wform(hal::app().res_wstr(HAL_INCORRECT_CONNECT_POLICY)) % pe_s.conn_out_policy).str())));
		}
		
		pe.prefer_rc4 = pe_s.prefer_rc4;
		
		try
		{
		
		session_->set_pe_settings(pe);
		
		}
		catch(const std::exception& e)
		{
			hal::event_log().post(boost::shared_ptr<hal::EventDetail>(
					new hal::EventStdException(event_logger::critical, e, L"ensurePeOn"))); 
					
			ensure_pe_off();		
		}
		
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Protocol encryption on.")));
	}

	void ensure_pe_off()
	{
		libt::pe_settings pe;
		pe.out_enc_policy = libt::pe_settings::disabled;
		pe.in_enc_policy = libt::pe_settings::disabled;
		
		pe.allowed_enc_level = libt::pe_settings::both;
		pe.prefer_rc4 = true;
		
		session_->set_pe_settings(pe);

		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Protocol encryption off.")));
	}
#	endif
	
	void set_resolve_countries(bool b)
	{		
		resolve_countries_ = b;

		for (torrent_manager::torrent_by_name::iterator i=the_torrents_.begin(), e=the_torrents_.end(); 
			i != e; ++i)
		{
			(*i).torrent->set_resolve_countries(resolve_countries_);
		}

		if (b)			
			event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Set to resolve countries.")));
		else			
			event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Not resolving countries.")));
	}

	void start_smart_ban_plugin()
	{
		session_->add_extension(&libt::create_smart_ban_plugin);
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Started smart ban plugin.")));
	}

	void start_ut_pex_plugin()
	{
		session_->add_extension(&libt::create_ut_pex_plugin);
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Started uTorrent peer exchange plugin.")));
	}

	void start_ut_metadata_plugin()
	{
		session_->add_extension(&libt::create_ut_metadata_plugin);
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Started uTorrent metadata plugin.")));
	}

	void start_metadata_plugin()
	{
		session_->add_extension(&libt::create_metadata_plugin);
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Started metadata plugin.")));
	}

	void ip_v4_filter_block(boost::asio::ip::address_v4 first, boost::asio::ip::address_v4 last)
	{
		ip_filter_.add_rule(first, last, libt::ip_filter::blocked);
		ip_filter_count();
		ip_filter_changed_ = true;
	}

	void ip_v6_filter_block(boost::asio::ip::address_v6 first, boost::asio::ip::address_v6 last)
	{
		ip_filter_.add_rule(first, last, libt::ip_filter::blocked);
		ip_filter_count();
		ip_filter_changed_ = true;
	}

	size_t ip_filter_size()
	{
		return ip_filter_count_;
	}

	void clear_ip_filter()
	{
		ip_filter_ = libt::ip_filter();
		session_->set_ip_filter(libt::ip_filter());	
		ip_filter_changed_ = true;
		ip_filter_count();
	}

	bool ip_filter_import_dat(boost::filesystem::path file, progress_callback fn, bool octalFix);

	struct 
	{
		signaler<> successful_listen;
		signaler<> torrent_finished;

		boost::signal<bool()> torrent_paused;
	} 
	signals;

	void start_alert_handler();
	void stop_alert_handler();
	void alert_handler();

	void add_torrent(wpath file, wpath saveDirectory, bool startStopped, bool managed, bit::allocations alloc, 
			boost::filesystem::wpath moveToDirectory) 
	{
		try 
		{	
//		torrent_internal_ptr TIp;

		std::pair<std::string, std::string> names = extract_names(file);
		wstring xml_name = from_utf8(names.first) + L".xml";

/*		if (false && fs::exists(file.parent_path()/xml_name))
		{
			torrent_standalone tsa;
			
			if (tsa.load_standalone(file.parent_path()/xml_name))
			{
				TIp = tsa.torrent;
				
				TIp->set_save_directory(saveDirectory, true);			
				if (useMoveTo)
					TIp->set_move_to_directory(moveToDirectory);

				TIp->prepare(file);
			}
		}
*/
		torrent_internal_ptr TIp =
			the_torrents_.create_torrent(file, saveDirectory, alloc, moveToDirectory);

		if(TIp)
		{
			TIp->set_managed(managed);
			TIp->set_transfer_speed(bittorrent::Instance().default_torrent_download(), 
				bittorrent::Instance().default_torrent_upload());
			TIp->set_connection_limit(bittorrent::Instance().default_torrent_max_connections(), 
				bittorrent::Instance().default_torrent_max_uploads());
			TIp->set_resolve_countries(resolve_countries_);

			TIp->start();
		}
		
		}
		catch (const std::exception& e)
		{
			event_log().post(shared_ptr<EventDetail>(
				new EventTorrentException(event_logger::critical, event_logger::torrentException, 
					std::string(e.what()), to_utf8(file.string()), std::string("addTorrent"))));
		}
	}

	void remove_torrent(const wstring& name)
	{
		try {
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Removing Torrent.")));

		boost::shared_ptr<file_details_vec> files = boost::shared_ptr<file_details_vec>(new file_details_vec());		
		torrent_internal_ptr pTI = the_torrents_.get(name);

		the_torrents_.remove_torrent(name);
		
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Removed")));
		
		} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(name, "remove_torrent")
	}

	void remove_torrent_wipe_files(const std::wstring& name, remove_files fn)
	{
		try {
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Removing Torrent and files.")));

		boost::shared_ptr<file_details_vec> files = boost::shared_ptr<file_details_vec>(new file_details_vec());		
		torrent_internal_ptr pTI = the_torrents_.get(name);

		pTI->get_file_details(*files);
		thread_t t(bind(fn, pTI->get_save_directory(), files));

		pTI->clear_resume_data();
		pTI->delete_torrent_file();

		the_torrents_.remove_torrent(name);
		
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Removed, started thread.")));
		
		} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(name, "remove_torrent_wipe_files")
	}

	void resume_all()
	{
		try {
			
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Resuming all torrents.")));
		
		for (torrent_manager::torrent_by_name::iterator i=the_torrents_.begin(), e=the_torrents_.end(); i != e;)
		{
			wpath file = wpath(hal::app().get_working_directory())/L"torrents"/(*i).torrent->filename();
			
			if (exists(file))
			{		
				try 
				{
					
				(*i).torrent->prepare(file);	
				(*i).torrent->start();	
				
				++i;
				
				}
				catch(const libt::duplicate_torrent&)
				{
					hal::event_log().post(shared_ptr<hal::EventDetail>(
						new hal::EventDebug(hal::event_logger::debug, L"Encountered duplicate torrent")));
					
					++i; // Harmless, don't worry about it.
				}
				catch(const std::exception& e) 
				{
					hal::event_log().post(shared_ptr<hal::EventDetail>(
						new hal::EventStdException(hal::event_logger::warning, e, L"resume_all")));
					
					the_torrents_.erase(i++);
				}			
			}
			else
			{
				the_torrents_.erase(i++);
			}
		}
		
		} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "resume_all")
	}

	bool close_counter(int* count)
	{
		--(*count);
		return true;
	}
	
	void close_all(boost::optional<report_num_active> fn)
	{
		try 
		{	

		event_log().post(shared_ptr<EventDetail>(new EventInfo(L"Saving torrent data...")));

		save_torrent_data();

		event_log().post(shared_ptr<EventDetail>(new EventInfo(L"Stopping all torrents...")));

		session_->pause();		

		// Ok this polling loop here is a bit curde, but a blocking wait is actually appropiate.		
		{ 
		int num_active = 1;

		while (num_active > 0)
		{
			num_active = 0;

			for (torrent_manager::torrent_by_name::iterator i=the_torrents_.begin(), e=the_torrents_.end(); 
					i != e; ++i)
			{
				if (	(*i).torrent 
					&& 
					(*i).torrent->state() != torrent_details::torrent_in_error
					&& 
					(	(	(*i).torrent->state() != torrent_details::torrent_stopped 
							&& 
							(*i).torrent->state() != torrent_details::torrent_paused
						)
						|| 
						(*i).torrent->awaiting_resume_data()
					)
				)
				{
#					ifdef HAL_TORRENT_DEV_MSGES
						(*i).torrent->output_torrent_debug_details();
#					endif
					num_active += 1;
				}
			}

			event_log().post(shared_ptr<EventDetail>(new EventInfo(hal::wform(L"%1% still active") % (num_active))));

			if (fn)	(*fn)(num_active);
			boost::this_thread::sleep(pt::milliseconds(500));
		} 
		
		}
		
		event_log().post(shared_ptr<EventDetail>(new EventInfo(L"All torrents stopped.")));		
		
		} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "close_all()")
	}
	
	void save_torrent_data()
	{	
		mutex_t::scoped_lock l(mutex_);
		try
		{
		
		the_torrents_.save_to_ini();
		bittorrent_ini_.save_data();
			
		if (dht_on_) 
		{	
			halencode(hal::app().get_working_directory()/L"DHTState.bin", session_->dht_state());
		}
		
		}		
		catch(std::exception& e)
		{
			event_log().post(shared_ptr<EventDetail>(\
				new EventStdException(event_logger::critical, e, L"save_torrent_data()")));
		}
	}
	
	int default_torrent_max_connections() { return default_torrent_max_connections_; }
	int default_torrent_max_uploads() { return default_torrent_max_uploads_; }
	float default_torrent_download() { return default_torrent_download_; }
	float default_torrent_upload() { return default_torrent_upload_; }
	
private:
	bool create_torrent(const create_torrent_params& params, fs::wpath out_file, progress_callback fn);
	
	boost::optional<libt::session> session_;	
	mutable mutex_t mutex_;

	boost::optional<thread_t> alert_checker_;
	bool keepChecking_;
	
	ini_file bittorrent_ini_;
	torrent_manager the_torrents_;	
	
	int default_torrent_max_connections_;
	int default_torrent_max_uploads_;
	float default_torrent_download_;
	float default_torrent_upload_;
	
	bool resolve_countries_;
	bool ip_filter_on_;
	bool ip_filter_loaded_;
	bool ip_filter_changed_;
	libt::ip_filter ip_filter_;
	size_t ip_filter_count_;
	
	void ip_filter_count();
	void ip_filter_load(progress_callback fn);
	void ip_filter_import(std::vector<libt::ip_range<boost::asio::ip::address_v4> >& v4,
		std::vector<libt::ip_range<boost::asio::ip::address_v6> >& v6);
	
	bool dht_on_;
	libt::dht_settings dht_settings_;
	libt::entry dht_state_;	

	libt::upnp* upnp_;
	libt::natpmp* natpmp_;
};

}

#include "halSessionStates.hpp"
