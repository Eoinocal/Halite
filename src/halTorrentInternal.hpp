
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "halTorrentDefines.hpp"

#ifndef HAL_TORRENT_STATE_LOGGING
#	define TORRENT_STATE_LOG(s)
#else
#	include "../halEvent.hpp"
#	define TORRENT_STATE_LOG(msg) \
	hal::event_log.post(boost::shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(msg, hal::event_logger::torrent_dev))) 
#endif

#pragma warning (push, 1)
#	include <libtorrent/file.hpp>
#	include <libtorrent/hasher.hpp>
#	include <libtorrent/storage.hpp>
#	include <libtorrent/file_pool.hpp>
#	include <libtorrent/alert_types.hpp>
#	include <libtorrent/entry.hpp>
#	include <libtorrent/bencode.hpp>
#	include <libtorrent/session.hpp>
#	include <libtorrent/ip_filter.hpp>
#	include <libtorrent/torrent_handle.hpp>
#	include <libtorrent/peer_connection.hpp>
#	include <libtorrent/extensions/metadata_transfer.hpp>
#	include <libtorrent/extensions/ut_pex.hpp>
#pragma warning (pop) 

#include <boost/tuple/tuple.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/tag.hpp>

#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>

#include "halIni.hpp"
#include "halTypes.hpp"
#include "halSignaler.hpp"

namespace hal 
{
class TorrentInternalOld;
class torrent_internal;
}

BOOST_CLASS_VERSION(hal::TorrentInternalOld, 9)
BOOST_CLASS_VERSION(hal::torrent_internal, 2)

namespace hal 
{

namespace libt = libtorrent;
namespace sc = boost::statechart;

inline libt::entry haldecode(const wpath &file) 
{
	fs::ifstream ifs(file, fs::ifstream::binary);
	if (ifs.is_open()) 
	{
		ifs.unsetf(fs::ifstream::skipws);
		return libt::bdecode(std::istream_iterator<char>(ifs), std::istream_iterator<char>());
	}
	else return libt::entry();
}

inline bool halencode(const wpath &file, const libt::entry &e) 
{
	fs::ofstream ofs(file, fs::ofstream::binary);

	if (!ofs.is_open()) 
		return false;
	
	libt::bencode(std::ostream_iterator<char>(ofs), e);
	return true;
}

inline path path_to_utf8(const wpath& wp)
{
	return path(to_utf8(wp.string()));
}

inline wpath path_from_utf8(const path& p)
{
	return wpath(from_utf8(p.string()));
}

inline std::pair<std::string, std::string> extract_names(const wpath &file)
{
	if (fs::exists(file)) 
	{	
		libt::torrent_info info(path_to_utf8(file));

		std::string name = info.name();	
		std::string filename = name;

		if (!boost::find_last(filename, ".torrent")) 
				filename += ".torrent";
		
		event_log.post(shared_ptr<EventDetail>(new EventMsg(
			hal::wform(L"Loaded names: %1%, %2%") % from_utf8(name) % from_utf8(filename))));

		return std::make_pair(name, filename);
	}
	else
		return std::make_pair("", "");
}

inline libt::storage_mode_t hal_allocation_to_libt(bit::allocations alloc)
{
	switch (alloc)
	{
	case bit::full_allocation:
		return libt::storage_mode_allocate;
	case bit::compact_allocation:
		return libt::storage_mode_compact;
	case bit::sparse_allocation:
	default:
		return libt::storage_mode_sparse;
	}
}

class invalid_torrent : public std::exception
{
public:
	invalid_torrent(const wstring& who) :
		who_(who)
	{}
	
	virtual ~invalid_torrent() throw () {}

	wstring who() const throw ()
	{
		return who_;
	}       
	
private:
	wstring who_;	
};
	
template<typename T>
class transfer_tracker
{
public:
	transfer_tracker() :
		total_(0),
		total_offset_(0)
	{}
	
	transfer_tracker(T total) :
		total_(total),
		total_offset_(0)
	{}
	
	transfer_tracker(T total, T offset) :
		total_(total),
		total_offset_(offset)
	{}
	
	void reset(T total) const
	{
		total_ = total;
		total_offset_ = 0;
	}
	
	T update(T rel_total) const
	{
		total_ += (rel_total - total_offset_);
		total_offset_ = rel_total;
		
		return total_;
	}
	
	void setOffset(T offset) const
	{
		total_offset_ = offset;
	}
	
	operator T() const { return total_; }
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & boost::serialization::make_nvp("total", total_);
	}
	
private:
	mutable T total_;
	mutable T total_offset_;
};

class duration_tracker
{
public:
	duration_tracker() :
		total_(boost::posix_time::time_duration(0,0,0,0), 
			boost::posix_time::time_duration(0,0,0,0))
	{}
	
	boost::posix_time::time_duration update() const
	{
		if (start_.is_not_a_date_time()) 
			start_ = boost::posix_time::second_clock::universal_time();

		if (static_cast<boost::posix_time::time_duration>(total_).is_special()) 
			total_.setOffset(boost::posix_time::time_duration(0,0,0,0));
		
		return total_.update(boost::posix_time::second_clock::universal_time() - start_);
	}
	
	void reset() const
	{
		total_.setOffset(boost::posix_time::time_duration(0,0,0,0));
		start_ = boost::posix_time::second_clock::universal_time();
	}
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & boost::serialization::make_nvp("total", total_);
	}
	
	operator boost::posix_time::time_duration() const { return total_; }
	
private:
	transfer_tracker<boost::posix_time::time_duration> total_;	
	mutable boost::posix_time::ptime start_;		
};
	
struct signalers
{
	signaler<> torrent_finished;

	boost::signal<void ()> torrent_paused;
	boost::signal<void ()> resume_data;
};

class torrent_internal;
typedef shared_ptr<torrent_internal> torrent_internal_ptr;

struct torrent_standalone :
	public hal::IniBase<torrent_standalone>
{
	typedef torrent_standalone thisClass;
	typedef hal::IniBase<thisClass> iniClass;

	torrent_standalone() :
		iniClass("torrent")
	{}

	torrent_standalone(torrent_internal_ptr t) :
		iniClass("torrent"),
		torrent(t),
		save_time(pt::second_clock::universal_time())
	{}

	torrent_internal_ptr torrent;
	pt::ptime save_time;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & boost::serialization::make_nvp("torrent", torrent);
		ar & boost::serialization::make_nvp("save_time", save_time);
    }
};

class torrent_internal :
	public boost::enable_shared_from_this<torrent_internal>,
	private boost::noncopyable
{
	friend class bit_impl;	
	friend class bit::torrent::exec_around_ptr::proxy;

private:
	struct out_of_session;
	struct in_the_session;

	struct torrent_state_machine : sc::state_machine<torrent_state_machine, out_of_session> {};

	struct out_of_session : sc::simple_state<out_of_session, torrent_state_machine> {};

	struct paused;
	struct active;

	struct in_the_session : sc::simple_state<in_the_session, torrent_state_machine, paused> 
	{
		in_the_session();
		~in_the_session();
	};

	struct paused : sc::simple_state<paused, in_the_session>
	{
		paused();
		~paused();
	};

	struct active : sc::simple_state<active, in_the_session>
	{
		active();
		~active();
	};

public:
	#define TORRENT_INTERNALS_DEFAULTS \
		original_filename_(L""), \
		transfer_limit_(std::pair<float, float>(-1, -1)), \
		connections_(-1), \
		uploads_(-1), \
		ratio_(0), \
		resolve_countries_(true), \
		total_uploaded_(0), \
		total_base_(0), \
		progress_(0), \
		managed_(false), \
		start_time_(boost::posix_time::second_clock::universal_time()), \
		in_session_(false), \
		queue_position_(0)
		
	torrent_internal() :	
		TORRENT_INTERNALS_DEFAULTS,
		allocation_(bit::sparse_allocation)
	{
		state(torrent_details::torrent_stopped);
		TORRENT_STATE_LOG(L"Torrent state machine initiate");
		machine_.initiate();
	}
	
		torrent_internal(wpath filename, wpath saveDirectory, bit::allocations alloc, wpath move_to_directory=L"") :
		TORRENT_INTERNALS_DEFAULTS,
		save_directory_(saveDirectory.string()),
		move_to_directory_(move_to_directory.string()),
		allocation_(alloc)
	{
		state(torrent_details::torrent_stopped);
		assert(the_session_);	
		
		TORRENT_STATE_LOG(L"Torrent state machine initiate");
		machine_.initiate();

		prepare(filename);
	}

	#undef TORRENT_INTERNALS_DEFAULTS
	
	torrent_details_ptr get_torrent_details_ptr()
	{	
		mutex_t::scoped_lock l(mutex_);

		try
		{

		if (in_session())
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
				status_memory_.download_payload_rate, 
				status_memory_.upload_payload_rate),
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
			event_log.post(shared_ptr<EventDetail>(
				new EventInvalidTorrent(event_logger::critical, event_logger::invalid_torrent, to_utf8(name_), "get_torrent_details_ptr")));
		}
		catch (const std::exception& e)
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventTorrentException(event_logger::critical, event_logger::torrentException, e.what(), to_utf8(name_), "get_torrent_details_ptr")));
		}
		
		return torrent_details_ptr(new torrent_details(
			name_, filename_, 
			save_directory().string(), 
			app().res_wstr(HAL_TORRENT_STOPPED), 
			app().res_wstr(HAL_NA)));
	}

	void adjust_queue_position(bit::queue_adjustments adjust)
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

	void set_transfer_speed(float down, float up)
	{	
		mutex_t::scoped_lock l(mutex_);

		transfer_limit_ = std::make_pair(down, up);
		
		apply_transfer_speed();
	}

	void set_connection_limit(int maxConn, int maxUpload)		
	{
		mutex_t::scoped_lock l(mutex_);

		connections_ = maxConn;
		uploads_ = maxUpload;
		
		apply_connection_limit();
	}

	std::pair<float, float> get_transfer_speed()
	{
		return transfer_limit_;
	}

	std::pair<int, int> get_connection_limit()
	{
		return std::make_pair(connections_, uploads_);
	}
	
	const wstring& name() const { return name_; }
	
	void set_ratio(float ratio) 
	{ 
		if (ratio < 0) ratio = 0;
		ratio_ = ratio; 
		
		apply_ratio();
	}
	
	float get_ratio()
	{
		return ratio_;
	}

	void set_managed(bool m)
	{
		mutex_t::scoped_lock l(mutex_);
		managed_ = m;
		
		if (in_session()) handle_.auto_managed(managed_);
	}

	bool is_managed()
	{
		if (in_session())
		{
			managed_ = handle_.is_auto_managed();
		}

		return managed_;
	}
	
	void add_to_session(bool paused = false)
	{
		try
		{

		mutex_t::scoped_lock l(mutex_);	
		assert(the_session_ != 0);

		HAL_DEV_MSG(hal::wform(L"add_to_session() paused=%1%") % paused);
		
		if (!in_session()) 
		{	
			libt::add_torrent_params p;

			string torrent_file = to_utf8((hal::app().get_working_directory()/L"torrents"/filename_).string());
			info_memory_.reset(new libt::torrent_info(torrent_file.c_str()));

			std::string resume_file = to_utf8((hal::app().get_working_directory()/L"resume" / (name_ + L".fastresume")).string());

			std::vector<char> buf;
			if (libt::load_file(resume_file.c_str(), buf) == 0)
			{
				HAL_DEV_MSG(L"Using resume data");
				p.resume_data = &buf;
			}

			p.ti = info_memory_;
			p.save_path = path_to_utf8(save_directory_);
			p.storage_mode = hal_allocation_to_libt(allocation_);
			p.paused = paused;
			p.duplicate_is_error = false;
			p.auto_managed = managed_;

			handle_ = the_session_->add_torrent(p);		
			assert(handle_.is_valid());
			in_session_ = true;
			
		//	clear_resume_data();
		//	handle_.force_reannounce();
		}	

		assert(in_session());
		HAL_DEV_MSG(L"Added to session");

		if (handle_.is_paused())
			state(torrent_details::torrent_paused);	

		}
		catch(std::exception& e)
		{
			hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventStdException(event_logger::critical, e, L"add_to_session"))); 
		}
	}
	
	bool remove_from_session(bool write_data=true)
	{
		try
		{
		HAL_DEV_MSG(hal::wform(L"remove_from_session() write_data=%1%") % write_data);

		mutex_t::scoped_lock l(mutex_);
		if (!in_session())
		{
			in_session_ = false;
			HAL_DEV_MSG(L"Was not is session!");

			return false;
		}
		
		if (write_data)
		{
			HAL_DEV_MSG(L"requesting resume data");			
		
			signaler_wrapper<>* sig = new signaler_wrapper<>(bind(&torrent_internal::remove_from_session, this, false));
			signals().resume_data.connect(bind(&signaler_wrapper<>::operator(), sig));
			
			save_resume_data();

			return false;
		}
		else
		{		
			HAL_DEV_MSG(L"removing handle from session");
			the_session_->remove_torrent(handle_);
			in_session_ = false;

			assert(!in_session());	
			HAL_DEV_MSG(L"Removed from session!");

			return true;
		}

		}
		catch(std::exception& e)
		{
			hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventStdException(event_logger::critical, e, L"remove_from_session()"))); 
			return false;
		}
	}
	
	bool in_session() const
	{ 
		mutex_t::scoped_lock l(mutex_);

		return (in_session_ && the_session_ != 0 && handle_.is_valid());
	}

	void resume()
	{
		mutex_t::scoped_lock l(mutex_);
		HAL_DEV_MSG(hal::wform(L"resume() - %1%") % name_);

		if (state() == torrent_details::torrent_stopped)
		{	
			add_to_session(false);
			assert(in_session());			
		}
		else
		{
			assert(in_session());
			handle_.resume();
		}	
		
		state(torrent_details::torrent_active);			
		//assert(!handle_.is_paused());
	}
	
	void pause()
	{
		mutex_t::scoped_lock l(mutex_);
		HAL_DEV_MSG(hal::wform(L"pause() - %1%") % name_);

		if (state() == torrent_details::torrent_stopped)
		{	
			add_to_session(true);

			assert(in_session());
			assert(handle_.is_paused());
		}
		else
		{
			assert(in_session());

			HAL_DEV_MSG(hal::wform(L"pause() - handle_.pause()"));
			handle_.pause();

			signaler_wrapper<>* sig = new signaler_wrapper<>(bind(&torrent_internal::completed_pause, this));
			signals().torrent_paused.connect(bind(&signaler_wrapper<>::operator(), sig));

			state(torrent_details::torrent_pausing);
		}			
	}
	
	void stop()
	{
		mutex_t::scoped_lock l(mutex_);
		HAL_DEV_MSG(hal::wform(L"stop() - %1%") % name_);

		HAL_DEV_MSG(hal::wform(L"stop() requesting"));

		if (state() != torrent_details::torrent_stopped)
		{
			if (state() == torrent_details::torrent_active)
			{
				assert(in_session());
				assert(!(handle_.is_paused()));

				signaler_wrapper<>* sig = new signaler_wrapper<>(bind(&torrent_internal::completed_stop, this));
				signals().torrent_paused.connect(bind(&signaler_wrapper<>::operator(), sig));
				
				HAL_DEV_MSG(hal::wform(L"stop() - handle_.pause()"));
				handle_.pause();

				state(torrent_details::torrent_stopping);
			}
			else if (state() == torrent_details::torrent_paused)
			{			
				remove_from_session();
				state(torrent_details::torrent_stopped);				
			}
		}
	}

	void set_state_stopped()
	{
		state(torrent_details::torrent_stopped);
	}

	void force_recheck()
	{
		mutex_t::scoped_lock l(mutex_);		
		HAL_DEV_MSG(L"force_recheck()");

		switch (state())
		{
		case torrent_details::torrent_stopped:
			clear_resume_data();
			resume();
			break;

		case torrent_details::torrent_stopping:
		case torrent_details::torrent_pausing:
//			signals().torrent_paused.disconnect_all_once();

		case torrent_details::torrent_active:
//			signals().torrent_paused.disconnect_all_once();
//			signals().torrent_paused.connect_once(bind(&torrent_internal::handle_recheck, this));
			handle_.pause();
			state(torrent_details::torrent_pausing);
			break;

		default:
			assert(false);
		};
	}
	
	void write_resume_data(const libt::entry& ent)
	{					
		HAL_DEV_MSG(L"write_resume_data()");

		wpath resume_dir = hal::app().get_working_directory()/L"resume";
		
		if (!exists(resume_dir))
			create_directory(resume_dir);

		boost::filesystem::ofstream out(resume_dir/(name_ + L".fastresume"), std::ios_base::binary);
		out.unsetf(std::ios_base::skipws);
		bencode(std::ostream_iterator<char>(out), ent);

		HAL_DEV_MSG(L"Written!");
	}

	void save_resume_data()
	{
		handle_.save_resume_data();
	}
	
	void clear_resume_data()
	{
		wpath resume_file = hal::app().get_working_directory()/L"resume"/filename_;
		
		if (exists(resume_file))
			remove(resume_file);

//		resumedata_ = libt::entry();
	}

	const wpath get_save_directory()
	{
		return save_directory_;
	}

	void set_save_directory(wpath s, bool force=false)
	{
		if (in_session() && !is_finished() &&
				s != path_from_utf8(handle_.save_path()))
		{
			handle_.move_storage(path_to_utf8(s));
			save_directory_ = s;
		}
		else if (!in_session() && force)
		{
			save_directory_ = s;
		}
	}

	const wpath get_move_to_directory()
	{
		return move_to_directory_;
	}
	
	void set_move_to_directory(wpath m)
	{
		if (is_finished() && !m.empty())
		{
			if (m != path_from_utf8(handle_.save_path()))
			{
				handle_.move_storage(path_to_utf8(m));
				save_directory_ = move_to_directory_ = m;
			}
		}
		else
		{
			move_to_directory_ = m;
		}
	}

	bool is_finished()
	{
		if (in_session())
		{
			libt::torrent_status::state_t s = handle_.status().state;

			return (s == libt::torrent_status::seeding ||
						s == libt::torrent_status::finished);
		}
		else return false;
	}
	
	void finished()
	{
		if (finish_time_.is_special())
			finish_time_ = boost::posix_time::second_clock::universal_time();

		if (is_finished())
		{
			if (!move_to_directory_.empty() && 
					move_to_directory_ !=  path_from_utf8(handle_.save_path()))
			{
				handle_.move_storage(path_to_utf8(move_to_directory_));
				save_directory_ = move_to_directory_;
			}
		}
	}
	
	bool is_active() const { return state() == torrent_details::torrent_active; }

	unsigned get_state()
	{
		return state_;
	}
	
	void set_tracker_login(wstring username, wstring password)
	{
		tracker_username_ = username;
		tracker_password_ = password;
		
		apply_tracker_login();
	}	
	
	std::pair<wstring, wstring> get_tracker_login() const
	{
		return make_pair(tracker_username_, tracker_password_);
	}
	
	const wstring& filename() const { return filename_; }
	
	const wstring& original_filename() const { return original_filename_; }
	
	const libt::torrent_handle& handle() const { return handle_; }

	void reset_trackers()
	{
		if (in_session())
		{
			handle_.replace_trackers(torrent_trackers_);		
			trackers_.clear();
		}
	}
	
	void set_trackers(const std::vector<tracker_detail>& tracker_details)
	{
		trackers_.clear();
		trackers_.assign(tracker_details.begin(), tracker_details.end());
		
		apply_trackers();
	}
	
	const std::vector<tracker_detail>& get_trackers()
	{
		if (trackers_.empty() && info_memory_)
		{
			std::vector<libt::announce_entry> trackers = info_memory_->trackers();
			
			foreach (const libt::announce_entry& entry, trackers)
			{
				trackers_.push_back(
					tracker_detail(hal::from_utf8(entry.url), entry.tier));
			}
		}		
		return trackers_;
	}
	
	void set_file_priorities(std::vector<int> fileIndices, int priority)
	{
		if (!file_priorities_.empty())
		{
			foreach(int i, fileIndices)
				file_priorities_[i] = priority;
				
			apply_file_priorities();
		}
	}

	const wpath& save_directory() { return save_directory_; }
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		using boost::serialization::make_nvp;

		if (version > 1) {
			ar & make_nvp("transfer_limits", transfer_limit_);
			ar & make_nvp("connection_limits", connections_);
			ar & make_nvp("upload_limits", uploads_);	

			ar & make_nvp("name", name_);
			ar & make_nvp("filename", filename_);	

			ar & make_nvp("ratio", ratio_);	
			ar & make_nvp("progress", progress_);
			ar & make_nvp("state", state_);
//			ar & make_nvp("compact_storage", compact_storage_);	
			ar & make_nvp("allocation_type", allocation_);	
			ar & make_nvp("resolve_countries", resolve_countries_);	

			ar & make_nvp("tracker_username", tracker_username_);
			ar & make_nvp("tracker_password", tracker_password_);
			ar & make_nvp("trackers", trackers_);

			ar & make_nvp("save_directory", save_directory_);
			ar & make_nvp("move_to_directory", move_to_directory_);
			
			ar & make_nvp("payload_uploaded", payload_uploaded_);
			ar & make_nvp("payload_downloaded", payload_downloaded_);
			ar & make_nvp("uploaded", uploaded_);
			ar & make_nvp("downloaded", downloaded_);			
					
			ar & make_nvp("file_priorities", file_priorities_);
			
			ar & make_nvp("start_time", start_time_);
			ar & make_nvp("finish_time", finish_time_);
			ar & make_nvp("active_duration", active_duration_);
			ar & make_nvp("seeding_duration", seeding_duration_);
			ar & make_nvp("managed", managed_);
					
		} 
		else 
		{
		    ar & make_nvp("transferLimit", transfer_limit_);
			ar & make_nvp("connections", connections_);
			ar & make_nvp("uploads", uploads_);			
			ar & make_nvp("filename", filename_);	

			wstring s;
			ar & make_nvp("saveDirectory", s);
			save_directory_ = s;

			if (version == 2) {
				wstring m;
				ar & make_nvp("moveToDirectory", m);
				move_to_directory_ = m;
			} else {
				move_to_directory_ = save_directory_;
			}
			
			ar & make_nvp("payload_uploaded_", payload_uploaded_);
			ar & make_nvp("payload_downloaded_", payload_downloaded_);
			ar & make_nvp("uploaded_", uploaded_);
			ar & make_nvp("downloaded_", downloaded_);	
			ar & make_nvp("ratio", ratio_);	
			ar & make_nvp("trackerUsername", tracker_username_);
			ar & make_nvp("trackerPassword", tracker_password_);
			
			ar & make_nvp("state", state_);
			ar & make_nvp("trackers", trackers_);
			
			ar & make_nvp("resolve_countries", resolve_countries_);
			
			ar & make_nvp("file_priorities", file_priorities_);
			
			ar & make_nvp("start_time", start_time_);
			ar & make_nvp("activeDuration", active_duration_);
			ar & make_nvp("seedingDuration", seeding_duration_);
			
			ar & make_nvp("name", name_);
			ar & make_nvp("compactStorage", compact_storage_);
			ar & make_nvp("finish_time", finish_time_);
			
			ar & make_nvp("progress", progress_);
	}
    }

	void set_entry_data(boost::intrusive_ptr<libt::torrent_info> metadata, libtorrent::entry resumedata)
	{		
		info_memory_ = metadata;
//		resumedata_ = resumedata;
	}

	std::vector<libt::peer_info>& peers() { return peers_; }
	
	boost::tuple<size_t, size_t, size_t, size_t> update_peers()
	{
		if (in_session())
			handle_.get_peer_info(peers_);
		
		size_t totalPeers = 0;
		size_t peersConnected = 0;
		size_t totalSeeds = 0;
		size_t seedsConnected = 0;
		
		foreach (libt::peer_info& peer, peers_) 
		{
			float speedSum = peer.down_speed + peer.up_speed;
			
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
	
	void get_peer_details(peer_details_vec& peer_details) const
	{
		if (in_session())
		{
			foreach (libt::peer_info peer, peers_) 
			{
				peer_details.push_back(peer);
			}	
		}
	}

	void get_file_details(file_details_vec& files)
	{
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
		
		files = file_details_memory_;
	}
	
	void prepare(wpath filename)
	{
		mutex_t::scoped_lock l(mutex_);
		
		if (fs::exists(filename)) 
			info_memory_ = new libt::torrent_info(path_to_utf8(filename));
		
		extract_names(info_memory());			
		
		const wpath resumeFile = hal::app().get_working_directory()/L"resume"/filename_;
		const wpath torrentFile = hal::app().get_working_directory()/L"torrents"/filename_;
		
		event_log.post(shared_ptr<EventDetail>(new EventMsg(
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

	void set_resolve_countries(bool b)
	{
		resolve_countries_ = b;
		apply_resolve_countries();
	}
	
	void extract_names(boost::intrusive_ptr<libt::torrent_info> metadata)
	{
		mutex_t::scoped_lock l(mutex_);
				
		name_ = hal::from_utf8_safe(metadata->name());
		
		filename_ = name_;
		if (!boost::find_last(filename_, L".torrent")) 
				filename_ += L".torrent";
		
		event_log.post(shared_ptr<EventDetail>(new EventMsg(
			hal::wform(L"Loaded names: %1%, %2%") % name_ % filename_)));
	}
	
	boost::intrusive_ptr<libt::torrent_info> info_memory()
	{
		if (!info_memory_) 
			info_memory_ = 
				boost::intrusive_ptr<libt::torrent_info>(new libt::torrent_info(path_to_utf8(filename())));
		
		return info_memory_;
	}
	
	signalers& signals()
	{
		mutex_t::scoped_lock l(mutex_);
		return signals_;
	}

private:	
	signalers signals_;

	void apply_settings()
	{		
		apply_transfer_speed();
		apply_connection_limit();
		apply_ratio();
		apply_trackers();
		apply_tracker_login();
		apply_file_priorities();
		apply_resolve_countries();
	}
	
	void apply_transfer_speed()
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

	void apply_connection_limit()
	{
		mutex_t::scoped_lock l(mutex_);
		if (in_session())
		{
			handle_.set_max_connections(connections_);
			handle_.set_max_uploads(uploads_);

			HAL_DEV_MSG(hal::wform(L"Applying Connection Limit %1% - %2%") % connections_ % uploads_);
		}
	}
	
	void apply_ratio()
	{ 
		mutex_t::scoped_lock l(mutex_);
		if (in_session())
		{
			handle_.set_ratio(ratio_);

			HAL_DEV_MSG(hal::wform(L"Applying Ratio %1%") % ratio_);
		}
	}
	
	void apply_trackers()
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
	
	void apply_tracker_login()
	{
		mutex_t::scoped_lock l(mutex_);
		if (in_session())
		{
			if (tracker_username_ != L"")
			{
				handle_.set_tracker_login(hal::to_utf8(tracker_username_),
					hal::to_utf8(tracker_password_));
			}

			HAL_DEV_MSG(hal::wform(L"Applying Tracker Login User: %1%, Pass: %2%")
				% tracker_username_ % tracker_password_ );
		}
	}
	
	void apply_file_priorities()
	{		
		mutex_t::scoped_lock l(mutex_);
		if (in_session()) 
		{
			if (!file_priorities_.empty())
				handle_.prioritize_files(file_priorities_);
			
			HAL_DEV_MSG(L"Applying File Priorities");
		}
	}	
	
	void apply_resolve_countries()
	{
		mutex_t::scoped_lock l(mutex_);
		if (in_session())
		{
			handle_.resolve_countries(resolve_countries_);
			
			HAL_DEV_MSG(hal::wform(L"Applying Resolve Countries %1%") % resolve_countries_);
		}
	}
	
	bool completed_pause()
	{
		mutex_t::scoped_lock l(mutex_);
		assert(in_session());
//		assert(handle_.is_paused());	

		HAL_DEV_MSG(L"completed_pause()");

		save_resume_data();				
		state(torrent_details::torrent_paused);

		return true;
	}

	bool completed_stop()
	{
		mutex_t::scoped_lock l(mutex_);
		assert(in_session());
//		assert(handle_.is_paused());			
		
		if (remove_from_session())
		{
			assert(!in_session());
			HAL_DEV_MSG(L"completed_stop()");
		}

		state(torrent_details::torrent_stopped);

		return true;
	}

	void handle_recheck()
	{
		mutex_t::scoped_lock l(mutex_);
		state(torrent_details::torrent_stopped);

		remove_from_session(false);
		assert(!in_session());

		clear_resume_data();

		resume();
		assert(in_session());

		HAL_DEV_MSG(L"handle_recheck()");
	}

	void state(unsigned s)
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
		default:
			HAL_DEV_MSG(L"state() - unknown");
			break;
		};
		state_ = s;
	}	
	
	unsigned state() const 
	{ 
		if (in_session())
		{
			if (handle_.is_paused())
			{
				if (state_ != torrent_details::torrent_paused)
				{			
					HAL_DEV_MSG(L"Should really be paused!");
					state_ = torrent_details::torrent_paused;
				}
			}
			else				
			{			
				if (state_ != torrent_details::torrent_active &&
					state_ != torrent_details::torrent_pausing &&
					state_ != torrent_details::torrent_stopping)
				{			
					HAL_DEV_MSG(L"Should really be active!");
					state_ = torrent_details::torrent_active;
				}
			}			
		}
		else
		{
			if (state_ != torrent_details::torrent_stopped)
			{			
				HAL_DEV_MSG(L"Should really be stopped!");
				state_ = torrent_details::torrent_stopped;
			}
		}
		
		return state_; 
	}
		
	static libt::session* the_session_;	
	mutable mutex_t mutex_;

	torrent_state_machine machine_;
	
	std::pair<float, float> transfer_limit_;
	
	mutable unsigned state_;
	int connections_;
	int uploads_;
	bool in_session_;
	float ratio_;
	bool resolve_countries_;
	
	wstring filename_;
	wstring name_;
	wpath save_directory_;
	wpath move_to_directory_;
	wstring original_filename_;
	libt::torrent_handle handle_;	
	
//	boost::intrusive_ptr<libt::torrent_info> metadata_;
//	boost::shared_ptr<libt::entry> resumedata_;
	
	wstring tracker_username_;	
	wstring tracker_password_;
	
	boost::int64_t total_uploaded_;
	boost::int64_t total_base_;
	
	transfer_tracker<boost::int64_t> payload_uploaded_;
	transfer_tracker<boost::int64_t> payload_downloaded_;
	transfer_tracker<boost::int64_t> uploaded_;
	transfer_tracker<boost::int64_t> downloaded_;
	
	pt::ptime start_time_;
	pt::ptime finish_time_;
	duration_tracker active_duration_;
	duration_tracker seeding_duration_;
	
	std::vector<tracker_detail> trackers_;
	std::vector<libt::announce_entry> torrent_trackers_;
	std::vector<libt::peer_info> peers_;	
	std::vector<int> file_priorities_;
	
	float progress_;
	
	boost::intrusive_ptr<libt::torrent_info> info_memory_;
	libt::torrent_status status_memory_;
	file_details_vec file_details_memory_;
	
	int queue_position_;
	bool compact_storage_;
	bool managed_;
	bit::allocations allocation_;
};

typedef std::map<std::string, TorrentInternalOld> TorrentMap;
typedef std::pair<std::string, TorrentInternalOld> TorrentPair;

class torrent_manager : 
	public hal::IniBase<torrent_manager>
{
	typedef torrent_manager thisClass;
	typedef hal::IniBase<thisClass> iniClass;

	struct torrent_holder
	{
		mutable torrent_internal_ptr torrent;
		
		wstring filename;
		wstring name;		
		
		torrent_holder()
		{}
		
		explicit torrent_holder(torrent_internal_ptr t) :
			torrent(t), filename(torrent->filename()), name(torrent->name())
		{}
						
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			using boost::serialization::make_nvp;

			ar & make_nvp("torrent", torrent);
			ar & make_nvp("filename", filename);
			ar & make_nvp("name", name);
		}
	};
	
	struct by_filename{};
	struct by_name{};
	
	typedef boost::multi_index_container<
		torrent_holder,
		boost::multi_index::indexed_by<
			boost::multi_index::ordered_unique<
				boost::multi_index::tag<by_filename>,
				boost::multi_index::member<
					torrent_holder, wstring, &torrent_holder::filename> 
				>,
			boost::multi_index::ordered_unique<
				boost::multi_index::tag<by_name>,
				boost::multi_index::member<
					torrent_holder, wstring, &torrent_holder::name> 
				>
		>
	> torrent_multi_index;
	
public:
	typedef torrent_multi_index::index<by_filename>::type torrent_by_filename;
	typedef torrent_multi_index::index<by_name>::type torrent_by_name;
	
	torrent_manager(ini_file& ini) :
		iniClass("bittorrent", "torrent_manager", ini)
	{}

	std::pair<torrent_by_name::iterator, bool> insert(const torrent_holder& h)
	{
		return torrents_.get<by_name>().insert(h);
	}
	
	std::pair<torrent_by_name::iterator, bool> insert(torrent_internal_ptr t)
	{
		return insert(torrent_holder(t));
	}

	torrent_internal_ptr get_by_file(const wstring& filename)
	{
		torrent_by_filename::iterator it = torrents_.get<by_filename>().find(filename);
		
		if (it != torrents_.get<by_filename>().end() && (*it).torrent)
		{
			return (*it).torrent;
		}
		
		throw invalid_torrent(filename);
	}
	
	torrent_internal_ptr get(const wstring& name)
	{
		torrent_by_name::iterator it = torrents_.get<by_name>().find(name);
		
		if (it != torrents_.get<by_name>().end() && (*it).torrent)
		{
			return (*it).torrent;
		}
		
		throw invalid_torrent(name);
	}
	
	torrent_by_name::iterator erase(torrent_by_name::iterator where)
	{
		return torrents_.get<by_name>().erase(where);
	}
	
	size_t size()
	{
		return torrents_.size();
	}
	
	size_t erase(const wstring& name)
	{
		return torrents_.get<by_name>().erase(name);
	}
	
	bool exists(const wstring& name)
	{
		torrent_by_name::iterator it = torrents_.get<by_name>().find(name);
		
		if (it != torrents_.get<by_name>().end())
			return true;
		else
			return false;
	}
	
	torrent_by_name::iterator begin() { return torrents_.get<by_name>().begin(); }
	torrent_by_name::iterator end() { return torrents_.get<by_name>().end(); }
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & boost::serialization::make_nvp("torrents", torrents_);
	}	
	
private:
	torrent_multi_index torrents_;
};

} // namespace hal

BOOST_CLASS_VERSION(hal::torrent_manager::torrent_holder, 1)
