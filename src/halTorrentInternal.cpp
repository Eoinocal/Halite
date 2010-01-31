
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include "halTorrentInternal.hpp"

namespace hal 
{

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
			state_str = app().res_wstr(HAL_TORRENT_SEEDING);
			break;
		case libt::torrent_status::allocating:
			state_str = app().res_wstr(HAL_TORRENT_ALLOCATING);
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
		name_, filename_, 
		save_directory().string(), 
		state_str, 
		hal::from_utf8(status_memory_.current_tracker), 
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
				event_logger::invalid_torrent, to_utf8(name_), "get_torrent_details_ptr")));
	}
	catch (const std::exception& e)
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventTorrentException(event_logger::critical, 
				event_logger::torrentException, e.what(), to_utf8(name_), "get_torrent_details_ptr")));
	}
	
	return torrent_details_ptr(new torrent_details(
		name_, filename_, 
		save_directory().string(), 
		app().res_wstr(HAL_TORRENT_STOPPED), 
		app().res_wstr(HAL_NA)));
}

file_details_vec torrent_internal::get_file_details()
{
	mutex_t::scoped_lock l(mutex_);

	file_details_vec files;
	get_file_details(files);

	return files;
}

void torrent_internal::get_file_details(file_details_vec& files_vec)
{
	mutex_t::scoped_lock l(mutex_);

	if (file_details_memory_.empty())
	{
		boost::intrusive_ptr<libt::torrent_info> info = info_memory();
		std::vector<libt::file_entry> files;
		
		std::copy(info->begin_files(), info->end_files(), 
			std::back_inserter(files));					
			
		if (file_priorities_.size() != files.size())
		{
			file_priorities_.clear();
			file_priorities_.assign(files.size(), 1);
		}
		
		for(size_t i=0, e=files.size(); i<e; ++i)
		{
			wstring fullPath = hal::from_utf8(files[i].path.string());
			boost::int64_t size = static_cast<boost::int64_t>(files[i].size);
			
			file_details_memory_.push_back(file_details(fullPath, size, 0, file_priorities_[i], i));
		}	
	}		
	
	if (in_session())
	{			
		std::vector<libt::size_type> fileProgress;			
		handle_.file_progress(fileProgress);
		
		for(size_t i=0, e=file_details_memory_.size(); i<e; ++i)
			file_details_memory_[i].progress =  fileProgress[i];			
	}

	for(size_t i=0, e=file_details_memory_.size(); i<e; ++i)
		file_details_memory_[i].priority =  file_priorities_[i];
	
	files_vec = file_details_memory_;
}

void torrent_internal::prepare(wpath filename)
{
	mutex_t::scoped_lock l(mutex_);
	
	if (fs::exists(filename)) 
		info_memory_ = new libt::torrent_info(path_to_utf8(filename));
	
	extract_names(info_memory());
	extract_filenames(info_memory());	
	
	const wpath resumeFile = hal::app().get_working_directory()/L"resume"/filename_;
	const wpath torrentFile = hal::app().get_working_directory()/L"torrents"/filename_;
	
	event_log().post(shared_ptr<EventDetail>(new EventMsg(
		hal::wform(L"File: %1%, %2%.") % resumeFile % torrentFile)));
	
//	if (exists(resumeFile)) 
//		resumedata_ = haldecode(resumeFile);

	if (!exists(hal::app().get_working_directory()/L"torrents"))
		create_directory(hal::app().get_working_directory()/L"torrents");

	if (!exists(torrentFile))
		copy_file(filename.string(), torrentFile);

	if (!fs::exists(save_directory_))
		fs::create_directory(save_directory_);

	// These here should not make state changes based on torrent 
	// session status since it has not been initialized yet.
	if (state_ == torrent_details::torrent_stopping)
		state(torrent_details::torrent_stopped);
	else if (state_ == torrent_details::torrent_pausing)
		state(torrent_details::torrent_paused);
}

void torrent_internal::extract_names(boost::intrusive_ptr<libt::torrent_info> metadata)
{
	mutex_t::scoped_lock l(mutex_);
			
	name_ = hal::from_utf8_safe(metadata->name());
	
	filename_ = name_;
	if (!boost::find_last(filename_, L".torrent")) 
			filename_ += L".torrent";
	
	event_log().post(shared_ptr<EventDetail>(new EventMsg(
		hal::wform(L"Loaded names: %1%, %2%") % name_ % filename_)));
}

void torrent_internal::extract_filenames(boost::intrusive_ptr<libt::torrent_info> metadata)
{
	mutex_t::scoped_lock l(mutex_);
			
	if (files_.empty())
	{
		for (libt::torrent_info::file_iterator i = metadata->begin_files(), e = metadata->end_files();
			i != e; ++i)
		{
			fs::wpath p_orig = path_from_utf8((*i).path);
			fs::wpath p_new = *p_orig.begin();

			if (++p_orig.begin() != p_orig.end())
			{
				p_new /= L"incoming";
				
				for (fs::wpath::iterator i = p_orig.begin(), e = p_orig.end(); i != e; ++i)
				{
					p_new /= *i;
				}
			}
			else
				p_new = L"incoming" / p_new;

			files_.push_back(torrent_file(p_orig, p_new));
		}
	}
	else
	{
		for (libt::torrent_info::file_iterator i = metadata->begin_files(), e = metadata->end_files();
			i != e; ++i)
		{
			fs::wpath p = path_from_utf8((*i).path);

			assert(p == files_[std::distance(metadata->begin_files(), i)].original_name());
		}
	}
	
	filename_ = name_;
	if (!boost::find_last(filename_, L".torrent")) 
			filename_ += L".torrent";
	
	event_log().post(shared_ptr<EventDetail>(new EventMsg(
		hal::wform(L"Loaded names: %1%, %2%") % name_ % filename_)));
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
			% tracker_username_ );
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

void torrent_internal::apply_resolve_countries()
{
	mutex_t::scoped_lock l(mutex_);
	if (in_session())
	{
		handle_.resolve_countries(resolve_countries_);
		
		HAL_DEV_MSG(hal::wform(L"Applying Resolve Countries %1%") % resolve_countries_);
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

};
