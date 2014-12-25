
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#if defined(HALTORRENT_PCH)
#	include "halPch.hpp"
#else
#	include "halTypes.hpp"
#endif

#include "halIni.hpp"
#include "halEvent.hpp"
#include "halConfig.hpp"
#include "halTorrentSerialization.hpp"
#include "halTorrentInternal.hpp"
#include "halTorrentManager.hpp"
#include "halSignaler.hpp"
#include "halCatchDefines.hpp"

namespace hal
{

namespace libt = libtorrent;

template<typename T>
void lazy_bdecode_file(T&& file, libt::lazy_entry& state)
{
	auto data(load_file<std::vector<char>>(file));

	boost::system::error_code ec;
	int pos;

	libt::lazy_bdecode(&data[0], &data[0] + data.size(), state, ec, &pos);

	if (ec)
		throw boost::system::system_error(ec);
}

inline bool operator!=(const libt::dht_settings& lhs, const libt::dht_settings& rhs)
{
	return lhs.max_peers_reply != rhs.max_peers_reply ||
		   lhs.search_branching != rhs.search_branching ||
//		   lhs.service_port != rhs.service_port ||
           lhs.max_fail_count != rhs.max_fail_count;
}

template<typename Addr>
void write_range(fs::ofstream& ofs, const libt::ip_range<Addr>& range)
{ 
	const typename Addr::bytes_type first = range.first.to_bytes();
	const typename Addr::bytes_type last = range.last.to_bytes();
	ofs.write((char*)first.data(), first.size());
	ofs.write((char*)last.data(), last.size());
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
	ifs.read((char*)first.data(), first.size());
	ifs.read((char*)last.data(), last.size());	
	
	ip_filter.add_rule(Addr(first), Addr(last),
		libt::ip_filter::blocked);
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

	bool listen_on(std::pair<int, int> const& range);
	int is_listening_on();
	void stop_listening();

	bool ensure_dht_on(const dht_settings& dht);
	void ensure_dht_off();

	void set_mapping(bool upnp, bool nat_pmp);
	std::wstring upnp_router_model();

	void set_timeouts(int peers, int tracker);

	cache_settings get_cache_settings();
	void set_cache_settings(const cache_settings& cache);
	cache_details get_cache_details() const;

	queue_settings get_queue_settings();
	void set_queue_settings(const queue_settings& queue);

	timeouts get_timeouts();
	void set_timeouts(const timeouts& times);

	void set_session_limits(int maxConn, int maxUpload);
	void set_session_speed(float download, float upload);

	bool ensure_ip_filter_on(progress_callback fn);
	void ensure_ip_filter_off();

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
					new hal::EventStdException(event_logger::critical, e, L"ensure_pe_on"))); 
					
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
	
	void set_announce_to_all(bool trackers, bool tiers)
	{
		libt::session_settings settings = session_->settings();

		settings.announce_to_all_trackers = trackers;
		settings.announce_to_all_tiers = tiers;

		session_->set_settings(settings);

		event_log().post(shared_ptr<EventDetail>(new EventMsg(
			hal::wform(L"Set announcing style- Trackers: %1%, Tiers: %2%.") 
				% settings.announce_to_all_trackers % settings.announce_to_all_tiers)));
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

	void start_lt_trackers_plugin()
	{
		session_->add_extension(&libt::create_lt_trackers_plugin);
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Started lt tracker plugin.")));
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
	
	std::wstring get_external_interface()
	{
		if (external_interface_)
			return *external_interface_;
		else
			return L"";
	}

	void set_external_interface(const std::wstring& ip)
	{
		use_custom_interface_ = true;

		for (torrent_manager::torrent_by_name::iterator i=the_torrents_.begin(), e=the_torrents_.end(); 
			i != e; ++i)
		{
			(*i).torrent->set_use_external_interface(ip);
		}

		external_interface_.reset(ip);
		
		event_log().post(shared_ptr<EventDetail>(new EventMsg(hal::wform(
			L"Set a custom external interface: %1%") % ip)));
	}

	void set_no_external_interface()
	{
		use_custom_interface_ = false;

		for (torrent_manager::torrent_by_name::iterator i=the_torrents_.begin(), e=the_torrents_.end(); 
			i != e; ++i)
		{
			(*i).torrent->set_no_external_interface();
		}

		external_interface_.reset();		
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Disabled custom external interface")));
	}

	bool ip_filter_import_dat(boost::filesystem::path file, progress_callback fn, bool octalFix);

	struct 
	{
		signaler<> successful_listen;
		signaler<> torrent_finished;

		boost::signals2::signal<bool()> torrent_paused;
		boost::signals2::signal<void (wstring torrent_name)> torrent_completed;
	} 
	signals;

	void start_alert_handler();
	void stop_alert_handler();
	void alert_handler();

	void add_torrent(const wpath& file, const wpath& save_directory, bool start_stopped, bool managed, bit::allocations alloc, 
			const wpath& move_to_directory) 
	{
		try 
		{	

		torrent_internal_ptr TIp =
			the_torrents_.create_torrent(file, save_directory, alloc, move_to_directory);

		if (TIp)
		{
			TIp->set_managed(managed);
			TIp->set_transfer_speed(bittorrent().default_torrent_download(), 
				bittorrent().default_torrent_upload());
			TIp->set_connection_limit(bittorrent().default_torrent_max_connections(), 
				bittorrent().default_torrent_max_uploads());
			TIp->set_resolve_countries(resolve_countries_);

			if (use_custom_interface_ && external_interface_)
				TIp->set_use_external_interface(*external_interface_);

			TIp->start();

			if (!start_stopped) TIp->resume();
		}
		
		} 
		HAL_TORRENT_FILESYSTEM_EXCEPTION_CATCH(uuid(), "add_torrent")
		HAL_GENERIC_TORRENT_EXCEPTION_CATCH(uuid(), "add_torrent")
	}

	void add_torrent(const wstring& uri, const wpath& save_directory, bool start_stopped, bool managed, bit::allocations alloc, 
			const wpath& move_to_directory) 
	{
		try 
		{	

		torrent_internal_ptr TIp = the_torrents_.create_torrent(uri, save_directory, alloc, move_to_directory);
		
		HAL_DEV_MSG(hal::wform(L"URI Torrent: Created"));

		if (TIp)
		{
			HAL_DEV_MSG(hal::wform(L" -- Managed"));
			TIp->set_managed(managed);
		
			HAL_DEV_MSG(hal::wform(L" -- Speeds"));
			TIp->set_transfer_speed(bittorrent().default_torrent_download(), 
				bittorrent().default_torrent_upload());
			
			HAL_DEV_MSG(hal::wform(L" -- Limits"));
			TIp->set_connection_limit(bittorrent().default_torrent_max_connections(), bittorrent().default_torrent_max_uploads());
			
			TIp->set_resolve_countries(resolve_countries_);

			if (use_custom_interface_ && external_interface_)
				TIp->set_use_external_interface(*external_interface_);

			HAL_DEV_MSG(hal::wform(L" -- Starting"));
			TIp->start();

			HAL_DEV_MSG(hal::wform(L" -- Resuming"));
			if (!start_stopped) TIp->resume();
				
			HAL_DEV_MSG(hal::wform(L" -- Done"));
		}
		
		} 
		HAL_TORRENT_FILESYSTEM_EXCEPTION_CATCH(uuid(), "add_torrent")
		HAL_GENERIC_TORRENT_EXCEPTION_CATCH(uuid(), "add_torrent")
	}

	void remove_torrent(const uuid& id)
	{
		try {
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Removing Torrent.")));

		boost::shared_ptr<file_details_vec> files = boost::shared_ptr<file_details_vec>(new file_details_vec());		
		torrent_internal_ptr pTI = the_torrents_.get(id);

		the_torrents_.remove_torrent(id);
		
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Removed")));
		
		} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(id, "remove_torrent")
	}

	void remove_wrapper_function(const uuid& id, boost::function<void (boost::shared_ptr<std::vector<std::wstring> >)> fn, 
		boost::shared_ptr<std::vector<std::wstring> > files)
	{
		try {
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Remove wrapper.")));

		if (!fn.empty()) 
			fn(files);
		
		torrent_internal_ptr pTI = the_torrents_.get(id);
		if(pTI)
		{
			pTI->clear_resume_data();
			pTI->delete_torrent_file();

			the_torrents_.remove_torrent(id);
		}		

		} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(id, "remove_wrapper_function")
	}


	void remove_torrent_wipe_files(const uuid& id, remove_files fn)
	{
		try {
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Removing Torrent and files.")));
		
		torrent_internal_ptr pTI = the_torrents_.get(id);
		if(pTI)
		{
		//	boost::shared_ptr<file_details_vec> files = boost::shared_ptr<file_details_vec>(new file_details_vec());
		//	pTI->get_file_details(*files);

		//	boost::function<void (boost::shared_ptr<std::vector<std::wstring> >)> fn_wrapper = boost::bind(fn, pTI->save_directory(), _1);

			pTI->remove_files(fn);

		//	event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Removed, registered thread.")));
		}		
		
		} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(id, "remove_torrent_wipe_files")
	}

	void resume_all()
	{
		try {
			
		event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Resuming all torrents.")));
		
		the_torrents_.start_all();

		} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(uuid(), "bit_impl::resume_all")
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

		event_log().post(shared_ptr<EventDetail>(new EventInfo(L"Saving torrent data ...")));

		save_torrent_data();

		event_log().post(shared_ptr<EventDetail>(new EventInfo(L"	... stopping all torrents")));

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
					(*i).torrent->state() != torrent_details::torrent_starting
					&& 
					(*i).torrent->state() != torrent_details::torrent_invalid
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
//						(*i).torrent->output_torrent_debug_details();
#					endif
					num_active += 1;
				}
			}

			event_log().post(shared_ptr<EventDetail>(new EventInfo(hal::wform(L"	... %1% still active") % (num_active))));

			if (fn)	(*fn)(num_active);
			boost::this_thread::sleep(pt::milliseconds(500));
		} 
		
		}
		
		event_log().post(shared_ptr<EventDetail>(new EventInfo(L"	... all torrents stopped.")));		
		
		} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(uuid(), "close_all()")
	}
	
	void save_torrent_data()
	{	
		unique_lock_t l(mutex_);
		try
		{
		
		the_torrents_.save_to_ini();
		bittorrent_ini_.save_data();			
		
		{	libt::entry state;
			session_->save_state(state);
			halencode(hal::app().get_working_directory()/L"libtorrent.state", state);
			
			hal::event_log().post(shared_ptr<hal::EventDetail>(
				new hal::EventMsg(L"Saved libtorrent State", hal::event_logger::info)));	
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

	void service_thread(size_t);
	void alert_handler_wait(const boost::system::error_code&);

	void execute_action(const boost::system::error_code&, bit::timeout_actions action);
	void execute_callback(const boost::system::error_code&, action_callback_t action);

	void acquire_work_object()
	{
		HAL_DEV_MSG(hal::wform(L"Acquiring service work object."));

		work_.reset(new boost::asio::io_service::work(io_service_));
	}

	void discard_work_object()
	{
		work_.reset();

		HAL_DEV_MSG(hal::wform(L"Discarded service work object"));
	}

	void schedual_action(boost::posix_time::ptime time, bit::timeout_actions action);
	void schedual_action(boost::posix_time::time_duration duration, bit::timeout_actions action);
	void schedual_callback(boost::posix_time::ptime time, action_callback_t action);
	void schedual_callback(boost::posix_time::time_duration duration, action_callback_t action);	
	void schedual_cancel();
	
	boost::optional<libt::session> session_;	
	SessionDetail session_details_;

	mutable mutex_t mutex_;
	
	ini_file bittorrent_ini_;
	torrent_manager the_torrents_;	

	boost::asio::io_service io_service_;
	std::auto_ptr<boost::asio::io_service::work> work_;

	boost::asio::deadline_timer action_timer_;

	boost::asio::deadline_timer alert_timer_;
	bool keep_checking_;

	typedef boost::shared_ptr<thread_t> shared_thread_ptr;

	std::vector<shared_thread_ptr> service_threads_;
	
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

	boost::optional<std::wstring> external_interface_;
	bool use_custom_interface_;
	
	void ip_filter_count();
	void ip_filter_load(progress_callback fn);
	void ip_filter_import(std::vector<libt::ip_range<boost::asio::ip::address_v4> >& v4,
		std::vector<libt::ip_range<boost::asio::ip::address_v6> >& v6);
	
	bool dht_on_;
	libt::dht_settings dht_settings_;
	libt::lazy_entry dht_state_;	

	libt::upnp* upnp_;
	libt::natpmp* natpmp_;
};

}

#include "halSessionStates.hpp"
