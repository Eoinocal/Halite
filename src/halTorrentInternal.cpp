
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#pragma warning (push, 1)
#	include <libtorrent/escape_string.hpp>
#	include <libtorrent/magnet_uri.hpp>
#pragma warning (pop) 

#include "halTorrentInternal.hpp"
#include "halTorrentManager.hpp"

namespace hal 
{
	
boost::optional<libt::session>* torrent_internal::the_session_ = 0;	
boost::optional<torrent_manager&> torrent_internal::the_manager_ = 0;

void torrent_internal::adjust_queue_position(bit::queue_adjustments adjust)
{
	if (in_session() && is_managed())
	{
		switch (adjust)
		{
		case bit::move_up:
			handle_.queue_position_up();
			break;
		case bit::move_down:
			handle_.queue_position_down();
			break;
		case bit::move_to_top:
			handle_.queue_position_top();
			break;
		case bit::move_to_bottom:
			handle_.queue_position_bottom();
			break;
		};
	}
}	
void torrent_internal::set_managed(bool m)
{
	mutex_t::scoped_lock l(mutex_);
	managed_ = m;
	
	if (in_session()) handle_.auto_managed(managed_);
}

bool torrent_internal::is_managed()
{
	if (in_session())
	{
		managed_ = handle_.is_auto_managed();
	}

	return managed_;
}

const wstring& torrent_internal::name() 
{ 
	if (name_.empty() && in_session())
		name_ = hal::from_utf8_safe(handle_.name());
	
	return name_; 
}

void torrent_internal::set_name(const wstring& n)
{
	mutex_t::scoped_lock l(mutex_);

	//name_ = n;
	files_.set_root_name(n);

	init_file_details();
	apply_file_names();
}

void torrent_internal::set_superseeding(bool ss)
{
	superseeding_ = ss;

	apply_superseeding();
}

bool torrent_internal::get_superseeding(bool actually)
{
	if (actually)
		if (in_session())
			return handle_.super_seeding();
		else
			return false;
	else
		return superseeding_;
}

void torrent_internal::add_to_session(bool paused)
{
	try
	{
	HAL_DEV_MSG(hal::wform(L"add_to_session() paused=%1%") % paused);

	mutex_t::scoped_lock l(mutex_);	

	process_event(ev_add_to_session(paused));
	assert(in_session());

	}
	catch(std::exception& e)
	{
		hal::event_log().post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(event_logger::critical, e, L"add_to_session"))); 
	}
}

bool torrent_internal::remove_from_session(bool write_data)
{
	try
	{
	HAL_DEV_MSG(hal::wform(L"remove_from_session() write_data=%1%") % write_data);

	mutex_t::scoped_lock l(mutex_);
	
	process_event(ev_remove_from_session(write_data));
	assert(in_session());

	}
	catch(std::exception& e)
	{
		hal::event_log().post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(event_logger::critical, e, L"remove_from_session()"))); 
		return false;
	}

	return true;
}

torrent_details_ptr torrent_internal::get_torrent_details_ptr()
{	
	try
	{
	mutex_t::scoped_lock l(mutex_);

	if (in_session() && is_active())
	{
		status_memory_ = handle_.status();
		progress_ = status_memory_.progress;

		queue_position_ = handle_.queue_position();
	}
	else
	{
		// Wipe these cause they don't make sense for a non-active torrent.
		
		status_memory_.download_payload_rate = 0;
		status_memory_.upload_payload_rate = 0;
		status_memory_.total_payload_download = 0;
		status_memory_.total_payload_upload = 0;
		status_memory_.next_announce = boost::posix_time::seconds(0);		
	}
	
	wstring state_str;
	
	switch (state())
	{
	case torrent_details::torrent_paused:
		state_str = app().res_wstr(HAL_TORRENT_PAUSED);
		break;
		
	case torrent_details::torrent_pausing:
		state_str = app().res_wstr(HAL_TORRENT_PAUSING);
		break;
		
	case torrent_details::torrent_stopped:
		state_str = app().res_wstr(HAL_TORRENT_STOPPED);
		break;
		
	case torrent_details::torrent_stopping:
		state_str = app().res_wstr(HAL_TORRENT_STOPPING);
		break;
		
	case torrent_details::torrent_in_error:
		state_str = app().res_wstr(HAL_TORRENT_IN_ERROR);
		break;
		
	default:
		switch (status_memory_.state)
		{
		case libt::torrent_status::queued_for_checking:
			state_str = app().res_wstr(HAL_TORRENT_QUEUED_CHECKING);
			break;
		case libt::torrent_status::checking_files:
			state_str = app().res_wstr(HAL_TORRENT_CHECKING_FILES);
			break;
//			case libt::torrent_status::connecting_to_tracker:
//				state = app().res_wstr(HAL_TORRENT_CONNECTING);
//				break;
		case libt::torrent_status::downloading_metadata:
			state_str = app().res_wstr(HAL_TORRENT_METADATA);
			break;
		case libt::torrent_status::downloading:
			state_str = app().res_wstr(HAL_TORRENT_DOWNLOADING);
			break;
		case libt::torrent_status::finished:
			state_str = app().res_wstr(HAL_TORRENT_FINISHED);
			break;
		case libt::torrent_status::seeding:
			if (!get_superseeding(true))
				state_str = app().res_wstr(HAL_TORRENT_SEEDING);
			else
				state_str = app().res_wstr(HAL_TORRENT_SUPERSEEDING);
			break;
		case libt::torrent_status::allocating:
			state_str = app().res_wstr(HAL_TORRENT_ALLOCATING);
			break;
		case libt::torrent_status::checking_resume_data:
			state_str = app().res_wstr(HAL_TORRENT_CHECKING_RESUME);
			break;
		}	
	}
	
	pt::time_duration td(pt::pos_infin);
	
	if (status_memory_.download_payload_rate != 0)
	{
		td = boost::posix_time::seconds(	
			long(float(status_memory_.total_wanted-status_memory_.total_wanted_done) / status_memory_.download_payload_rate));
	}
	
	total_uploaded_ += (status_memory_.total_payload_upload - total_base_);
	total_base_ = status_memory_.total_payload_upload;
	
	uploaded_.update(status_memory_.total_upload);
	payload_uploaded_.update(status_memory_.total_payload_upload);
	downloaded_.update(status_memory_.total_download);
	payload_downloaded_.update(status_memory_.total_payload_download);
	
	if (is_active())
	{
		active_duration_.update();
		
		if (libt::torrent_status::seeding == status_memory_.state)
			seeding_duration_.update();
	}	
	
	boost::tuple<size_t, size_t, size_t, size_t> connections = update_peers();	

	return torrent_details_ptr(new torrent_details(
		name(), filename_, 
		save_directory().string(), 
		state_str, 
		uuid(),
		hal::from_utf8(status_memory_.current_tracker), 
		hash_str_,
		std::pair<float, float>(
			boost::numeric_cast<float>(status_memory_.download_payload_rate), 
			boost::numeric_cast<float>(status_memory_.upload_payload_rate)),
		progress_, 
		status_memory_.distributed_copies, 
		status_memory_.total_wanted_done, 
		status_memory_.total_wanted, 
		uploaded_, payload_uploaded_,
		downloaded_, payload_downloaded_, 
		connections, 
		ratio_, 
		td, 
		status_memory_.next_announce, 
		active_duration_, seeding_duration_, 
		start_time_, finish_time_, 
		queue_position_,
		is_managed()));

	}
	catch (const libt::invalid_handle&)
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventInvalidTorrent(event_logger::critical, 
				event_logger::invalid_torrent, to_utf8(name()), "get_torrent_details_ptr")));
	}
	catch (const std::exception& e)
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventTorrentException(event_logger::critical, 
				event_logger::torrentException, e.what(), to_utf8(name()), "get_torrent_details_ptr")));
	}
	
	return torrent_details_ptr(new torrent_details(
		name(), filename_, 
		save_directory().string(), 
		app().res_wstr(HAL_TORRENT_STOPPED), 
		uuid(), 
		app().res_wstr(HAL_NA),
		hash_str_));
}

file_details_vec torrent_internal::get_file_details()
{
	mutex_t::scoped_lock l(mutex_);

	file_details_vec files;
	get_file_details(files);

	return files;
}

void torrent_internal::set_file_finished(int i)
{
	mutex_t::scoped_lock l(mutex_);

	files_.set_file_finished(i);
}

void torrent_internal::init_file_details()
{
	mutex_t::scoped_lock l(mutex_);

	file_details_memory_.clear();
	file_priorities_.clear();

	if (info_memory() && !files_.empty())
	{	
		libt::torrent_info::file_iterator file_it = info_memory()->begin_files();

		for(size_t i=0, e=files_.size(); i<e; ++i)
		{
			if (file_it == info_memory()->end_files()) break;

			boost::int64_t size = static_cast<boost::int64_t>(file_it->size);

			torrent_file::split_path_pair_t split = torrent_file::split_root(files_[i].completed_name());
			
			file_details_memory_.push_back(file_details(split.second, size, 0, files_[i].priority(), i));
			file_priorities_.push_back(files_[i].priority());

			++file_it;
		}	
	}
}

void torrent_internal::get_file_details(file_details_vec& files_vec)
{
	mutex_t::scoped_lock l(mutex_);

	if (file_details_memory_.empty())
		init_file_details();	
	
	if (in_session())
	{			
		std::vector<libt::size_type> file_progress;			
		handle_.file_progress(file_progress);
		
		for(size_t i=0, e=file_details_memory_.size(); i<e; ++i)
			file_details_memory_[i].progress =  file_progress[i];	
	}

	files_vec = file_details_memory_;
}

void torrent_internal::prepare(boost::intrusive_ptr<libt::torrent_info> info)
{
	mutex_t::scoped_lock l(mutex_);

	if (info)
	{				
		set_info_memory(info);
				
		extract_hash();
		extract_names();
		extract_filenames();	

		write_torrent_info();

		if (!fs::exists(save_directory_))
			fs::create_directories(save_directory_);

		// These here should not make state changes based on torrent 
		// session status since it has not been initialized yet.
		if (state_ == torrent_details::torrent_stopping)
			state(torrent_details::torrent_stopped);
		else if (state_ == torrent_details::torrent_pausing)
			state(torrent_details::torrent_paused);
	}
}

void torrent_internal::metadata_completed()
{
	prepare();

	apply_settings();
	update_manager();
}

void torrent_internal::update_manager()
{
	if (update_manager_)
	{
		update_manager_(shared_from_this());
	}
}

void torrent_internal::extract_names()
{
	mutex_t::scoped_lock l(mutex_);

	if (info_memory())
	{				
		name_ = hal::from_utf8_safe(info_memory()->name());
		
		filename_ = name();
		if (!boost::find_last(filename_, L".torrent")) 
				filename_ += L".torrent";
		
		event_log().post(shared_ptr<EventDetail>(new EventMsg(
			hal::wform(L"Loaded names: %1%, %2%") % name() % filename_)));
	}
}

void torrent_internal::extract_hash()
{
	mutex_t::scoped_lock l(mutex_);
	
	HAL_DEV_MSG(L"Extracting hash...");

	if (in_session())
	{		
		HAL_DEV_MSG(L"    from handle");
		hash_ = handle_.info_hash();
	}
	else if (info_memory())
	{		
		HAL_DEV_MSG(L"    from info_memory");
		hash_ = info_memory()->info_hash();
	}
	else if (!magnet_uri_.empty())
	{
		HAL_DEV_MSG(L"    from magnet uri");
		libt::add_torrent_params p;

		p.ti = info_memory();
		p.save_path = path_to_utf8(save_directory_).string();
		p.storage_mode = hal_allocation_to_libt(allocation_);
		p.paused = true;
		p.duplicate_is_error = false;
		p.auto_managed = false;

		libt::torrent_handle h = libt::add_magnet_uri(**the_session_, magnet_uri_, p);
		hash_ = h.info_hash();	

		HAL_DEV_MSG(L"    removing temp handle");
		(*the_session_)->remove_torrent(h);
	}
	else
	{		
		HAL_DEV_MSG(L"    No Hash!");
		hash_.clear();
	}
	
	libt::sha1_hash const& ih = hash_;
	hash_str_ = from_utf8(libt::base32encode(std::string((char const*)&ih[0], 20)));
	HAL_DEV_MSG(hal::wform(L"    hash : %1%") % hash_str_);
}

void torrent_internal::extract_filenames()
{
	mutex_t::scoped_lock l(mutex_);
			
	if (info_memory() && files_.empty())
	{		
		for (libt::torrent_info::file_iterator i = info_memory()->begin_files(), e = info_memory()->end_files();
			i != e; ++i)
		{
			fs::wpath p_orig = path_from_utf8((*i).path);
			fs::wpath p_new = torrent_file::add_hash(p_orig, hash_str_);

			bool using_hash = true;

			// Compensate for existing files outside HASHED subdir

			if (save_directory_ != move_to_directory_)
			{
				if (fs::exists(save_directory_/p_orig) && !fs::exists(save_directory_/p_new)) 	
				{
					if (!fs::exists((save_directory_/p_new).parent_path()))
						fs::create_directories((save_directory_/p_new).parent_path());

					fs::rename(save_directory_/p_orig, save_directory_/p_new);

					event_log().post(shared_ptr<EventDetail>(new EventMsg(
						wform(L"Existing File renamed: %1%") % p_orig, event_logger::info)));
				}
				else	 if (fs::exists(save_directory_/p_orig) && fs::exists(save_directory_/p_new)) 
				{
					HAL_DEV_MSG(wform(L"Two files exists, defaulting to using the new style"));
				}
			}
			else
			{
				if (fs::exists(save_directory_/p_orig) && p_orig != p_new) 	
				{
					using_hash = false;

					HAL_DEV_MSG(wform(L"Not using hashed directory") % p_orig);
				}
			}	

			int p = file_priorities_.empty() ? 1 : file_priorities_[std::distance(info_memory()->begin_files(), i)];
			files_.push_back(torrent_file(p_orig, using_hash, p));
		}
	}
	else
	{
		for (libt::torrent_info::file_iterator i = info_memory()->begin_files(), e = info_memory()->end_files();
			i != e; ++i)
		{
			fs::wpath p = path_from_utf8((*i).path);

//			assert(p == files_[std::distance(info_memory()->begin_files(), i)].original_name());
		}
	}
}

void torrent_internal::write_torrent_info()
{
	try {

	if (info_memory())
	{
		wpath torrent_info_file = hal::app().get_working_directory() / L"resume" / (name() + L".torrent_info");
		wpath resume_dir = hal::app().get_working_directory()/L"resume";
		
		if (!exists(resume_dir))
			fs::create_directories(resume_dir);

		boost::filesystem::ofstream out(resume_dir/(name() + L".torrent_info"), std::ios_base::binary);
		out.unsetf(std::ios_base::skipws);

		libt::create_torrent t(*info_memory());
		bencode(std::ostream_iterator<char>(out), t.generate());

		HAL_DEV_MSG(L"Torrent info written!");
	}

	} 
	catch (const boost::filesystem::wfilesystem_error&)
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg(L"Write torrent info error.", event_logger::warning)));
	}
	catch (const libt::libtorrent_exception&)
	{
		HAL_DEV_MSG(L"No torrent info");
	}
}

boost::tuple<size_t, size_t, size_t, size_t> torrent_internal::update_peers()
{
	mutex_t::scoped_lock l(mutex_);

	if (in_session())
		handle_.get_peer_info(peers_);
	
	size_t totalPeers = 0;
	size_t peersConnected = 0;
	size_t totalSeeds = 0;
	size_t seedsConnected = 0;
	
	foreach (libt::peer_info& peer, peers_) 
	{
		float speedSum = boost::numeric_cast<float>(peer.down_speed + peer.up_speed);
		
		if (!(peer.flags & libt::peer_info::seed))
		{
			++totalPeers;
			
			if (speedSum > 0)
				++peersConnected;
		}
		else
		{
			++totalSeeds;
			
			if (speedSum > 0)
				++seedsConnected;
		}
	}	
	
	return boost::make_tuple(totalPeers, peersConnected, totalSeeds, seedsConnected);
}

// ----------------- private -----------------

void torrent_internal::apply_settings()
{		
	apply_transfer_speed();
	apply_connection_limit();
	apply_ratio();
	apply_trackers();
	apply_tracker_login();
	apply_file_priorities();
	apply_resolve_countries();
	apply_superseeding();
	apply_file_names();
	apply_external_interface();
}

void torrent_internal::apply_transfer_speed()
{
	mutex_t::scoped_lock l(mutex_);
	if (in_session())
	{
		int down = (transfer_limit_.first > 0) ? static_cast<int>(transfer_limit_.first*1024) : -1;
		handle_.set_download_limit(down);
		
		int up = (transfer_limit_.second > 0) ? static_cast<int>(transfer_limit_.second*1024) : -1;
		handle_.set_upload_limit(up);

		HAL_DEV_MSG(hal::wform(L"Applying Transfer Speed %1% - %2%") % down % up);
	}
}

void torrent_internal::apply_external_interface()
{
	mutex_t::scoped_lock l(mutex_);
	if (in_session())
	{
		if (external_interface_)
		{
			handle_.use_interface(hal::to_utf8(*external_interface_).c_str());

			HAL_DEV_MSG(hal::wform(L"Applying external interface: %1%") % *external_interface_);
		}
		else
		{
		//	handle_.use_interface(0);

			HAL_DEV_MSG(L"Applying no custom external interface.");
		}
	}
}

void torrent_internal::apply_connection_limit()
{
	mutex_t::scoped_lock l(mutex_);
	if (in_session())
	{
		handle_.set_max_connections(connections_);
		handle_.set_max_uploads(uploads_);

		HAL_DEV_MSG(hal::wform(L"Applying Connection Limit %1% - %2%") % connections_ % uploads_);
	}
}

void torrent_internal::apply_ratio()
{ 
	mutex_t::scoped_lock l(mutex_);
	if (in_session())
	{
		handle_.set_ratio(ratio_);

		HAL_DEV_MSG(hal::wform(L"Applying Ratio %1%") % ratio_);
	}
}

void torrent_internal::apply_trackers()
{
	mutex_t::scoped_lock l(mutex_);
	if (in_session())
	{
		if (torrent_trackers_.empty())
			torrent_trackers_ = handle_.trackers();
		
		if (!trackers_.empty())
		{
			std::vector<libt::announce_entry> trackers;
			
			foreach (const tracker_detail& tracker, trackers_)
			{
				trackers.push_back(
					libt::announce_entry(hal::to_utf8(tracker.url)));
				trackers.back().tier = tracker.tier;
			}
			handle_.replace_trackers(trackers);
		}
		
		HAL_DEV_MSG(L"Applying Trackers");
	}
}

void torrent_internal::apply_tracker_login()
{
	mutex_t::scoped_lock l(mutex_);
	if (in_session())
	{
		if (tracker_username_ != L"")
		{
			handle_.set_tracker_login(hal::to_utf8(tracker_username_),
				hal::to_utf8(tracker_password_));
		}

		HAL_DEV_MSG(hal::wform(L"Applying Tracker Login User: %1% with password")
			% tracker_username_);
	}
}

void torrent_internal::apply_file_priorities()
{
	mutex_t::scoped_lock l(mutex_);
	if (in_session())
	{
		if (!file_priorities_.empty())
			handle_.prioritize_files(file_priorities_);
		
		HAL_DEV_MSG(L"Applying File Priorities");
	}
}

void torrent_internal::apply_file_names()
{		
	mutex_t::scoped_lock l(mutex_);

	if (in_session() && info_memory())
	{
		bool want_recheck = false;

		if (files_.size() != info_memory()->num_files())
		{
			extract_filenames();
			want_recheck = true;
		}

		for (int i = 0; i < info_memory()->num_files(); ++i)
		{
			handle_.rename_file(i, files_[i].active_name(hash_str_).string());
		}

		if (want_recheck)
			handle_.force_recheck();
		
		HAL_DEV_MSG(L"Applying File Names");
	}
}	

void torrent_internal::apply_resolve_countries()
{
	mutex_t::scoped_lock l(mutex_);
	if (in_session())
	{
		handle_.resolve_countries(resolve_countries_);
		
		HAL_DEV_MSG(hal::wform(L"Applying Resolve Countries %1%") % resolve_countries_);
	}
}

void torrent_internal::apply_superseeding()
{
	mutex_t::scoped_lock l(mutex_);
	if (in_session())
	{
		handle_.super_seeding(superseeding_);
		
		HAL_DEV_MSG(hal::wform(L"Applying Superseeding %1%") % superseeding_);
	}
}

void torrent_internal::apply_queue_position()
{
	mutex_t::scoped_lock l(mutex_);
	if (in_session())
	{
		if (handle_.queue_position() != -1 && queue_position_ != -1)
		{
			while (handle_.queue_position() != queue_position_)
			{
				HAL_DEV_MSG(hal::wform(L"Queue position libtorrent %1% - Halite %2%") 
					% handle_.queue_position() % queue_position_);

				if (handle_.queue_position() < queue_position_)
					handle_.queue_position_down();
				if (handle_.queue_position() > queue_position_)
					handle_.queue_position_up();
			}
		}
	}
	HAL_DEV_MSG(L"Applying Queue Position");
}

void torrent_internal::state(unsigned s)
{
	switch (s)
	{
	case torrent_details::torrent_stopped:
		HAL_DEV_MSG(L"state() - stopped");
		break;
	case torrent_details::torrent_stopping:
		HAL_DEV_MSG(L"state() - stopping");
		break;
	case torrent_details::torrent_pausing:
		HAL_DEV_MSG(L"state() - pausing");
		break;
	case torrent_details::torrent_active:
		HAL_DEV_MSG(L"state() - active");
		break;
	case torrent_details::torrent_paused:
		HAL_DEV_MSG(L"state() - paused");
		break;
	case torrent_details::torrent_in_error:
		HAL_DEV_MSG(L"state() - in error");
		break;
	default:
		HAL_DEV_MSG(L"state() - unknown");
		break;
	};
	state_ = s;
}

void torrent_internal::initialize_non_serialized(function<void (torrent_internal_ptr)> f)
{
	update_manager_ = f;

	TORRENT_STATE_LOG(L"Torrent state machine initiate");
	initiate();
}

};
