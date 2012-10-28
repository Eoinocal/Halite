
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "halPch.hpp"

#include "../res/resource.h"

#include "global/wtl_app.hpp"
#include "global/string_conv.hpp"
#include "global/ini_adapter.hpp"

#include "halTorrent.hpp"
#include "halTypes.hpp"
#include "halEvent.hpp"
#include "halSignaler.hpp"

#include "halTorrentInternal.hpp"
#include "halTorrentManager.hpp"
#include "halSession.hpp"
#include "halConfig.hpp"

#include <algorithm>

namespace hal 
{

bool remove_empty_directories_recur(const fs::path& p, std::vector<fs::path>& dirs)
{
	if (!fs::exists(p)) return false;

	bool ret = true;

	for (fs::directory_iterator i = fs::directory_iterator(p), e = fs::directory_iterator(); i != e; ++i)
	{
		if (!fs::is_directory(*i))
			ret = false;
		else
		{
			if (!remove_empty_directories(*i))
				ret = false;
		}
	}	

	if (ret)
	{
		HAL_DEV_MSG(wform(L"Removing directory: %1%") % p.wstring());
		dirs.push_back(p);
	}

	return ret;
}
	
bool remove_empty_directories(const fs::wpath& p)
{
	std::vector<fs::path> dirs;

	bool ret = remove_empty_directories_recur(p, dirs);

	std::for_each(dirs.begin(), dirs.end(), (bool (*)(const fs::path&))fs::remove);

	return ret;
}

bool file_details::less(const file_details& r, size_t index) const
{	
	switch (index)
	{
	case branch_e: return branch < r.branch;
	case filename_e: 
		return (boost::algorithm::to_upper_copy(filename.wstring()) 
			< boost::algorithm::to_upper_copy(r.filename.wstring()));

	case type_e: return type < r.type;
	case size_e: return size < r.size;

	case progress_e: return (static_cast<double>(progress)/size) < (static_cast<double>(r.progress)/r.size);
	case priority_e: return priority < r.priority;

	default: return false; // ???
	};
}

std::wstring file_details::to_wstring(size_t index)
{
	switch (index)
	{
	case branch_e: return branch.wstring();
	case filename_e: return filename.wstring();

	case type_e: return L"(Undefined)"; // ???

	case size_e: return (wform(L"%1$.2fMB") % (static_cast<double>(size)/(1024*1024))).str();
	case progress_e: return (wform(L"%1$.2f%%") % (static_cast<double>(progress)/size*100)).str(); 

	case priority_e: 		
		{
			switch (priority)
			{
			case 0:
				return hal::app().res_wstr(HAL_FILE_PRIORITY_0);
			case 1:
				return hal::app().res_wstr(HAL_FILE_PRIORITY_1);
			case 2:
				return hal::app().res_wstr(HAL_FILE_PRIORITY_2);
			case 3:
				return hal::app().res_wstr(HAL_FILE_PRIORITY_3);
			case 4:
				return hal::app().res_wstr(HAL_FILE_PRIORITY_4);
			case 5:
				return hal::app().res_wstr(HAL_FILE_PRIORITY_5);
			case 6:
				return hal::app().res_wstr(HAL_FILE_PRIORITY_6);
			case 7:
				return hal::app().res_wstr(HAL_FILE_PRIORITY_7);
			default:
				return hal::app().res_wstr(HAL_FILE_PRIORITY_0);
			};
		} 

	default: return L"(Undefined)"; // ???
	};
}

void file_details_sort(file_details_vec& p, size_t index, bool cmp_less)
{
	std::stable_sort(p.begin(), p.end(), 
		bind(&hal_details_compare<const file_details&>, _1, _2, index, cmp_less));
}

const peer_details_vec& torrent_details::get_peer_details() const
{
	if (!peer_details_filled_)
	{
		bittorrent::Instance().get_all_peer_details(uuid_, peer_details_);
		peer_details_filled_ = true;
	}
	
	return peer_details_;
}

const file_details_vec& torrent_details::get_file_details() const
{
	if (!file_details_filled_)
	{
		bittorrent::Instance().get_all_file_details(uuid_, file_details_);
		file_details_filled_ = true;
	}
	
	return file_details_;
}

bool torrent_details::less(const torrent_details& r, size_t index) const
{
	switch (index)
	{
	case name_e: 
		return (boost::algorithm::to_upper_copy(name_) 
			< boost::algorithm::to_upper_copy(r.name_));
	case state_e: return state_ < r.state_;

	case speed_down_e: return speed_.first < r.speed_.first;
	case speed_up_e: return speed_.second < r.speed_.second;
	case progress_e: return completion_ < r.completion_;
	case distributed_copies_e: return distributed_copies_ < r.distributed_copies_;

	case remaining_e: 
		{
		boost::int64_t left = total_wanted_-total_wanted_done_;
		boost::int64_t right = r.total_wanted_-r.total_wanted_done_;

		return left < right;
		}

	case total_wanted_e: return total_wanted_ < r.total_wanted_;
	case completed_e: return total_wanted_done_ < r.total_wanted_done_; 

	case uploaded_e: return total_payload_uploaded_ < r.total_payload_uploaded_;
	case downloaded_e: return total_payload_downloaded_ < r.total_payload_downloaded_;

	case peers_e: return peers_ < r.peers_;
	case seeds_e: return seeds_ < r.seeds_;

	case ratio_e: 
		{
		float left = (total_payload_downloaded_) 
				? static_cast<float>(total_payload_uploaded_)
					/ static_cast<float>(total_payload_downloaded_)
				: 0;
		
		float right = (r.total_payload_downloaded_) 
				? static_cast<float>(r.total_payload_uploaded_)
					/ static_cast<float>(r.total_payload_downloaded_)
				: 0;
		
		return left < right; 
		}

	case eta_e: return estimated_time_left_ < r.estimated_time_left_;
	case tracker: return currentTracker_ < r.currentTracker_;
	case update_tracker_in_e: return update_tracker_in_ < r.update_tracker_in_;

	case active_time_e: return active_ < r.active_;
	case seeding_time_e: return seeding_ < r.seeding_;
	case start_time_e: return start_time_ < r.start_time_;
	case finish_time_e: return finish_time_ < r.finish_time_;

	case queue_position_e: return queue_position_ < r.queue_position_;
	case managed_e: return managed_ < r.managed_;
	case uuid_e: return uuid_ < r.uuid_;
	case hash_e: return hash_ < r.hash_;

	default: return false; // ???
	};
}

std::wstring torrent_details::to_wstring(size_t index)
{
	switch (index)
	{
	case name_e: return name_;
	case state_e: return state_;

	case progress_e: return (wform(L"%1$.2f%%") % (completion_*100)).str(); 
	case speed_down_e: return (wform(L"%1$.2fkb/s") % (speed_.first/1024)).str(); 
	case speed_up_e: return (wform(L"%1$.2fkb/s") % (speed_.second/1024)).str(); 

	case distributed_copies_e: 
		{
		float copies = distributed_copies_;
		
		if (copies < 0)
			return L"Seeding"; 
		else
			return (hal::wform(L"%1$.2f") % copies).str();	
		}

	case remaining_e: 
		{
		return (wform(L"%1$.2fMB") % (static_cast<float>(total_wanted_-total_wanted_done_)/(1024*1024))).str(); 
		}

	case completed_e: return (wform(L"%1$.2fMB") % (static_cast<float>(total_wanted_done_)/(1024*1024))).str();

	case total_wanted_e: 
		{
		return (wform(L"%1$.2fMB") % (static_cast<float>(total_wanted_)/(1024*1024))).str(); 
		}

	case uploaded_e: 
		{
		return (wform(L"%1$.2fMB") % (static_cast<float>(total_payload_uploaded_)/(1024*1024))).str(); 
		}

	case downloaded_e: 
		{
		return (wform(L"%1$.2fMB") % (static_cast<float>(total_payload_downloaded_)/(1024*1024))).str(); 
		}

	case peers_e: return (wform(L"%1% (%2%)") % connected_peers_ % peers_).str(); 
	case seeds_e: return (wform(L"%1% (%2%)") % connected_seeds_ % seeds_).str(); 

	case ratio_e: 
		{
			float ratio = (total_payload_downloaded_) 
				? static_cast<float>(total_payload_uploaded_)
					/ static_cast<float>(total_payload_downloaded_)
				: 0;
			
			return (wform(L"%1$.2f") % ratio).str(); 
		}

	case eta_e: 
		{ 
		if (!estimated_time_left_.is_special())
			return hal::from_utf8(
				boost::posix_time::to_simple_string(estimated_time_left_));
		else
			return app().res_wstr(HAL_INF);		
		}	

	case tracker: return currentTracker_;

	case update_tracker_in_e:		
		{ 
		if (!update_tracker_in_.is_special())
			return from_utf8(
				boost::posix_time::to_simple_string(update_tracker_in_));
		else
			return app().res_wstr(HAL_INF);		
		}	

	case active_time_e: 
		{
		if (!active_.is_special())
			return from_utf8(
				boost::posix_time::to_simple_string(active_));
		else
			return app().res_wstr(HAL_INF);		
		}

	case seeding_time_e: 
		{ 
		if (!seeding_.is_special())
			return from_utf8(
				boost::posix_time::to_simple_string(seeding_));
		else
			return app().res_wstr(HAL_INF);
		}	

	case start_time_e: 
		{ 
		if (!start_time_.is_special())
			return from_utf8(
				boost::posix_time::to_simple_string(start_time_));
		else
			return app().res_wstr(IDS_NA);
		}		

	case finish_time_e: 		
		{ 
		if (!finish_time_.is_special())
			return from_utf8(
				boost::posix_time::to_simple_string(finish_time_));
		else
			return app().res_wstr(IDS_NA);	
		}		

	case queue_position_e: 
		{
			if (queue_position_ != -1)
				return (wform(L"%1%") % queue_position_).str(); 
			else
				return app().res_wstr(IDS_NA);		
		}

	case managed_e: return managed_ ?  L"Yes" : L"No";

	case uuid_e: 
		{
			std::wstringstream ss;
			ss << uuid_;

			return ss.str();
		}
		
	case hash_e: return hash_;

	default: return L"(Undefined)"; // ???
	};
}

/*void torrent_details_manager::sort(size_t column_index, bool cmp_less) const
{
	std::stable_sort(torrents_.begin(), torrents_.end(), 
		bind(&hal_details_ptr_compare<torrent_details_ptr>, _1, _2, column_index, cmp_less));
}*/

web_seed_or_dht_node_detail::web_seed_or_dht_node_detail() : 
	port(-1), 
	type(hal::app().res_wstr(HAL_INT_NEWT_ADD_PEERS_WEB)) 
{}

web_seed_or_dht_node_detail::web_seed_or_dht_node_detail(std::wstring u) : 
	url(u), 
	port(-1), 
	type(hal::app().res_wstr(HAL_INT_NEWT_ADD_PEERS_WEB)) 
{}

web_seed_or_dht_node_detail::web_seed_or_dht_node_detail(std::wstring u, int p) : 
	url(u), 
	port(p), 
	type(hal::app().res_wstr(HAL_INT_NEWT_ADD_PEERS_DHT)) 
{}

bit::bit() :
	pimpl_(new bit_impl())
{}

bit::~bit()
{}

bit_impl* bit::pimpl()
{
	if (!pimpl_) throw std::runtime_error("bittorrent() accessed after destructer");

	return &*pimpl_;
}

const bit_impl* bit::pimpl() const
{
	if (!pimpl_) throw std::runtime_error("bittorrent() accessed after destructer");

	return &*pimpl_;
}

void bit::shutdown_session()
{
	HAL_DEV_MSG(L"Commence shutdown_session()"); 

	pimpl_.reset();

	HAL_DEV_MSG(L"End shutdown_session()"); 
}

void bit::save_torrent_data()
{
	pimpl()->save_torrent_data();
}

bool bit::create_torrent(const create_torrent_params& params, fs::wpath out_file, progress_callback fn)
{
	return pimpl()->create_torrent(params, out_file, fn);
}

bit::torrent bit::get(const uuid& id)
{
	return bit::torrent(pimpl()->the_torrents_.get(id));
}

bool bit::listen_on(std::pair<int, int> const& range)
{
	return pimpl()->listen_on(range);
}

int bit::is_listening_on() 
{
	return pimpl()->is_listening_on();
}

void bit::stop_listening()
{
	pimpl()->stop_listening();
}

bool bit::ensure_dht_on(const hal::dht_settings& dht)
{
	return pimpl()->ensure_dht_on(dht);
}

void bit::ensure_dht_off()
{
	pimpl()->ensure_dht_off();
}

void bit::set_mapping(bool upnp, bool nat_pmp)
{
	pimpl()->set_mapping(upnp, nat_pmp);
}

std::wstring bit::upnp_router_model()
{
	return pimpl()->upnp_router_model();
}

queue_settings bit::get_queue_settings()
{
	return pimpl()->get_queue_settings();
}

void bit::set_queue_settings(const queue_settings& s)
{
	pimpl()->set_queue_settings(s);
}

timeouts bit::get_timeouts()
{
	return pimpl()->get_timeouts();
}

void bit::set_timeouts(const timeouts& t)
{
	pimpl()->set_timeouts(t);
}

void bit::set_session_limits(int maxConn, int maxUpload)
{		
	pimpl()->set_session_limits(maxConn, maxUpload);
}

void bit::set_session_speed(float download, float upload)
{
	pimpl()->set_session_speed(download, upload);
}

bool bit::ensure_ip_filter_on(progress_callback fn)
{
	return pimpl()->ensure_ip_filter_on(fn);
}

void bit::ensure_ip_filter_off()
{
	pimpl()->ensure_ip_filter_off();
}

void bit::set_resolve_countries(bool b)
{
	pimpl()->set_resolve_countries(b);
}

void bit::set_announce_to_all(bool trackers, bool tiers)
{
	pimpl()->set_announce_to_all(trackers, tiers);
}

void bit::start_smart_ban_plugin()
{
	pimpl()->start_smart_ban_plugin();
}

void bit::start_ut_pex_plugin()
{
	pimpl()->start_ut_pex_plugin();
}

void bit::start_ut_metadata_plugin()
{
	pimpl()->start_ut_metadata_plugin();
}

void bit::start_metadata_plugin()
{
	pimpl()->start_metadata_plugin();
}

#ifndef TORRENT_DISABLE_ENCRYPTION	

void bit::ensure_pe_on(const pe_settings& pe)
{
	pimpl()->ensure_pe_on(pe);
}

void bit::ensure_pe_off()
{
	pimpl()->ensure_pe_off();
}
#endif

void bit::ip_v4_filter_block(boost::asio::ip::address_v4 first, boost::asio::ip::address_v4 last)
{
	pimpl()->ip_filter_.add_rule(first, last, libt::ip_filter::blocked);
	pimpl()->ip_filter_count();
	pimpl()->ip_filter_changed_ = true;
}

void bit::ip_v6_filter_block(boost::asio::ip::address_v6 first, boost::asio::ip::address_v6 last)
{
	pimpl()->ip_v6_filter_block(first, last);
}

size_t bit::ip_filter_size()
{
	return pimpl()->ip_filter_size();
}

void bit::clear_ip_filter()
{
	pimpl()->clear_ip_filter();
}

bool bit::ip_filter_import_dat(boost::filesystem::path file, progress_callback fn, bool octalFix)
{
	return pimpl()->ip_filter_import_dat(file, fn, octalFix);
}

std::wstring bit::get_external_interface()
{
	return pimpl()->get_external_interface();
}
	
void bit::set_external_interface(const std::wstring& ip)
{
	pimpl()->set_external_interface(ip);
}

void bit::set_external_interface()
{
	pimpl()->set_no_external_interface();
}

const SessionDetail bit::get_session_details()
{
	SessionDetail details;
	
	details.port = pimpl()->session_->is_listening() ? pimpl()->session_->listen_port() : -1;
	
	libt::session_status status = pimpl()->session_->status();
	
	details.speed = std::pair<double, double>(status.download_rate, status.upload_rate);
	
	details.dht_on = pimpl()->dht_on_;
	details.dht_nodes = status.dht_nodes;
	details.dht_torrents = status.dht_torrents;
	
	details.ip_filter_on = pimpl()->ip_filter_on_;
	details.ip_ranges_filtered = pimpl()->ip_filter_count_;
	
	return details;
}

void bit::set_session_half_open_limit(int halfConn)
{
	libt::session_settings s = pimpl()->session_->settings();
	s.half_open_limit = halfConn;
	pimpl()->session_->set_settings(s);

	event_log().post(shared_ptr<EventDetail>(new EventMsg(
		hal::wform(L"Set half-open connections limit to %1%.") % s.half_open_limit)));
}

void bit::set_torrent_defaults(const connections& defaults)
{
	pimpl()->default_torrent_max_connections_ = defaults.total;
	pimpl()->default_torrent_max_uploads_ = defaults.uploads;

	event_log().post(shared_ptr<EventDetail>(new EventMsg(
		hal::wform(L"Set torrent connections total %1% and uploads %2%.") 
			% defaults.total % defaults.uploads)));

	pimpl()->default_torrent_download_ = defaults.download_rate;
	pimpl()->default_torrent_upload_ = defaults.upload_rate;

	event_log().post(shared_ptr<EventDetail>(new EventMsg(
		hal::wform(L"Set torrent default rates at %1$.2fkb/s down and %2$.2fkb/s upload.") 
			% defaults.download_rate % defaults.upload_rate)));
}

void bit::add_torrent(const wpath& file, const wpath& save_directory, 
	bool start_stopped, bool managed, allocations alloc, const wpath& move_to_directory) 
{
	pimpl()->add_torrent(file, save_directory, start_stopped, managed, alloc, move_to_directory);
}

void bit::add_torrent(const wstring& uri, const wpath& save_directory, 
	bool start_stopped, bool managed, allocations alloc, const wpath& move_to_directory) 
{
	pimpl()->add_torrent(uri, save_directory, start_stopped, managed, alloc, move_to_directory);
}

const torrent_details_manager& bit::torrentDetails()
{
	return torrent_details_;
}

const torrent_details_manager& bit::update_torrent_details_manager(const uuid& focused, const std::set<uuid>& selected)
{
	try {

	boost::thread t(boost::bind(&bit::update_torrent_details_manager_thread, this, focused, selected));
	t.join();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(uuid(), "updatetorrent_details_manager")
	
	return torrent_details_;
}

void bit::update_torrent_details_manager_thread(const uuid& focused, const std::set<uuid>& selected)
{
	try {	

	torrent_details_map tmp_map;
	
	for (torrent_manager::torrent_by_name::iterator i=pimpl()->the_torrents_.begin(), e=pimpl()->the_torrents_.end(); i != e; ++i)
	{
		torrent_details_ptr pT = (*i).torrent->get_torrent_details_ptr();				
		tmp_map[(*i).torrent->id()] = pT;
	}

	{	
		unique_lock_t l(torrent_details_.mutex_);

		torrent_details_.focused_ = focused;
		torrent_details_.selected_names_ = selected;

		std::swap(tmp_map, torrent_details_.torrent_map_);
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(uuid(), "updatetorrent_details_manager")
}

void bit::resume_all()
{
	pimpl()->resume_all();
}

void bit::close_all(boost::optional<report_num_active> fn)
{
	pimpl()->close_all(fn);
}

const cache_details bit::get_cache_details() const
{
	return pimpl()->get_cache_details();
}

void bit::get_all_peer_details(const uuid& id, peer_details_vec& peer_container)
{
	try {
	
	pimpl()->the_torrents_.get(id)->get_peer_details(peer_container);
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(id, "get_all_peer_details")
}

void bit::get_all_file_details(const uuid& id, file_details_vec& file_details)
{
	try {
	
	pimpl()->the_torrents_.get(id)->get_file_details(file_details);
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(id, "get_all_file_details")
}

bool bit::is_torrent(const uuid& id)
{	
	try {
	
	return pimpl()->the_torrents_.exists(id);
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(id, "is_torrent")
	
	return false;
}

void bit::pause_torrent(const uuid& id)
{
	try {
	
	pimpl()->the_torrents_.get(id)->pause();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(id, "pause_torrent")
}

void bit::resume_torrent(const uuid& id)
{
	try {
	
	pimpl()->the_torrents_.get(id)->resume();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(id, "resume_torrent")
}

void bit::stop_torrent(const uuid& id)
{
	try {
	
	pimpl()->the_torrents_.get(id)->stop();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(id, "stop_torrent")
}

bool bit::is_torrent_active(const uuid& id)
{
	try {
	
	return pimpl()->the_torrents_.get(id)->is_active();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(id, "is_torrent_active")
	
	return false; // ??? is this correct
}

void bit::reannounce_torrent(const uuid& id)
{
	try {
	
	pimpl()->the_torrents_.get(id)->handle().force_reannounce();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(id, "reannounce_torrent")
}


void bit::recheck_torrent(const uuid& id)
{
	try {
	
//	pimpl()->the_torrents_.get(filename)->output_torrent_debug_details();

	pimpl()->the_torrents_.get(id)->force_recheck();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(id, "recheck_torrent")
}

void bit::remove_torrent(const uuid& id)
{
	pimpl()->remove_torrent(id);
}

void bit::remove_torrent_wipe_files(const uuid& id, remove_files f)
{
	pimpl()->remove_torrent_wipe_files(id, f);
}

void bit::pause_all_torrents()
{	
	try {

	pimpl()->session_->pause();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(uuid(), "pause_all_torrents")
}

void bit::unpause_all_torrents()
{	
	try {

	pimpl()->session_->resume();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(uuid(), "unpause_all_torrents")
}

bool bit::is_any_torrent_active()
{
	bool result = false;
	
	try {
	
	for (torrent_manager::torrent_by_name::iterator i=pimpl()->the_torrents_.begin(), e=pimpl()->the_torrents_.end();
		i != e; ++i)
	{
		if ((*i).torrent->is_active())
		{
			result = true;
			break;
		}
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(uuid(), "is_any_torrent_active")
	
	return result;
}

void bit::schedual_action(boost::posix_time::ptime time, timeout_actions action)
{
	return pimpl()->schedual_action(time, action);
}

void bit::schedual_action(boost::posix_time::time_duration duration, timeout_actions action)
{
	return pimpl()->schedual_action(duration, action);
}

void bit::schedual_callback(boost::posix_time::ptime time, action_callback_t action)
{
	return pimpl()->schedual_callback(time, action);
}

void bit::schedual_callback(boost::posix_time::time_duration duration, action_callback_t action)
{
	return pimpl()->schedual_callback(duration, action);
}

void bit::schedual_cancel()
{
	return pimpl()->schedual_cancel();
}

void bit::connect_torrent_completed_signal(function<void (wstring torrent_name)> fn)
{
	pimpl()->signals.torrent_completed.connect(fn);
}

bit::torrent::torrent() //:
//	files(*this)
{}

bit::torrent::torrent(boost::shared_ptr<torrent_internal> p) :	
//	files(*this),
	ptr(p)
{}

bool bit::torrent::is_open() const
{
	return ptr;
}

bit::torrent::exec_around_ptr::proxy::proxy(torrent_internal* t) : 
	t_(t)
{
//	HAL_DEV_MSG(L"Ctor proxy");
}

bit::torrent::exec_around_ptr::proxy::~proxy() 
{
//	HAL_DEV_MSG(L"Dtor proxy");
}

const std::wstring bit::torrent::get_name() const
{
	try {
	
	return ptr->name();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_name()")
	
	return 0;
}

void bit::torrent::set_name(const std::wstring& n)
{
	try {
	
	ptr->set_name(n);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_name()")
}

const uuid bit::torrent::get_uuid() const
{
	try {
	
	return ptr->id();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_uuid()")
	
	return hal::uuid();
}

bit::torrent::files_proxy bit::torrent::files()
{
	return bit::torrent::files_proxy(*this);
}

std::pair<int, int> bit::torrent::get_connection_limits() const
{
	try {
	
	return ptr->get_connection_limit();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_connection_limits")
	
	return std::make_pair(-1, -1);
}

void bit::torrent::set_connection_limits(const std::pair<int, int>& l)
{
	try {
	
	ptr->set_connection_limit(l.first, l.second);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_connection_limits")
}

std::pair<float, float> bit::torrent::get_rate_limits() const
{
	try {
	
	return ptr->get_transfer_speed();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_rate_limits")
	
	return std::pair<float, float>(-1.f, -1.f);
}

void bit::torrent::set_rate_limits(const std::pair<float, float>& l)
{
	try {
	
	ptr->set_transfer_speed(l.first, l.second);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_rate_limits")
}

wpath bit::torrent::get_save_directory() const
{
	try {
	
	return ptr->save_directory();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_save_directory")
	
	return L"";
}

void bit::torrent::set_save_directory(const wpath& s)
{
	try {
	
	ptr->set_save_directory(s);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_save_directory")
}

wpath bit::torrent::get_move_to_directory() const
{
	try {
	
	return ptr->get_move_to_directory();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_save_directory")
	
	return L"";
}

void bit::torrent::set_move_to_directory(const wpath& m)
{
	try {
	
	ptr->set_move_to_directory(m);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_move_to_directory")
}

std::pair<wstring, wstring> bit::torrent::get_tracker_login() const
{
	try {
	
	return ptr->get_tracker_login();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("get_tracker_login")
	
	return std::make_pair(L"!!! exception thrown !!!", L"!!! exception thrown !!!");
}

void bit::torrent::set_tracker_login(const std::pair<wstring, wstring>& p)
{
	try {
	
	ptr->set_tracker_login(p.first, p.second);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_tracker_login")
}

bool bit::torrent::get_is_active() const
{
	try {
	
	return ptr->is_active();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_is_active")
	
	return L"";
}

bool bit::torrent::get_in_session() const
{
	try {
	
	return ptr->in_session();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_in_session")
	
	return L"";
}

std::vector<tracker_detail> bit::torrent::get_trackers() const
{
	try {
	
	return ptr->get_trackers();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_trackers")
	
	return std::vector<tracker_detail>();
}

void bit::torrent::set_trackers(const std::vector<tracker_detail>& trackers)
{
	try {
	
	ptr->set_trackers(trackers);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_trackers")
}

void bit::torrent::reset_trackers()
{
	try {
	
	ptr->reset_trackers();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_trackers")
}

void bit::torrent::set_file_priorities(const std::pair<std::vector<int>, int>& p)
{
	try { 

	ptr->set_file_priorities(p.first, p.second);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_trackers")
}

void bit::torrent::adjust_queue_position(bit::queue_adjustments adjust)
{
	try { 

	ptr->adjust_queue_position(adjust);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::adjust_queue_position")
}

void bit::torrent::set_managed(bool m)
{
	try {
	
	ptr->set_managed(m);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_managed")
}

bool bit::torrent::get_superseeding() const
{
	try {
	
	return ptr->get_superseeding();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_superseeding")
	
	return false;
}

void bit::torrent::set_superseeding(bool ss)
{
	try {
	
	ptr->set_superseeding(ss);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_superseeding")
}


/*files_proxy bit::torrent::get_files() const
{
	return const files_proxy(*this);
}

files_proxy bit::torrent::set_files()
{
	return files_proxy(*this);
}*/

wpath bit::torrent::files_proxy::file_proxy::get_name() const
{
	try {

	return t_.ptr->files()[n_].completed_name();
	
	} HAL_GENERIC_PIMPL_EXCEPTION_CATCH("bit::torrent::files_proxy::file_proxy::get_name()")

	return wpath();
}

void bit::torrent::files_proxy::file_proxy::set_name(const wpath& filename)
{
	try {

	t_.ptr->files().change_filename(n_, filename);
	
	} HAL_GENERIC_PIMPL_EXCEPTION_CATCH("bit::torrent::files_proxy::file_proxy::set_name()")
}

void bit::start_event_receiver()
{
	try {

	pimpl()->start_alert_handler();

	event_log().post(shared_ptr<EventDetail>(new EventMsg(L"	... started handler")));
	
	} HAL_GENERIC_PIMPL_EXCEPTION_CATCH("bit::start_event_receiver()")
}

void bit::stop_event_receiver()
{
	try {

	event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Stopping event handler")));

	pimpl()->stop_alert_handler();
	
	} HAL_GENERIC_PIMPL_EXCEPTION_CATCH("bit::stop_event_receiver()")
}


int bit::default_torrent_max_connections() { return pimpl()->default_torrent_max_connections_; }
int bit::default_torrent_max_uploads() { return pimpl()->default_torrent_max_uploads_; }
float bit::default_torrent_download() { return pimpl()->default_torrent_download_; }
float bit::default_torrent_upload() { return pimpl()->default_torrent_upload_; }
	
};
