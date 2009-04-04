
//         Copyright Eóin O'Callaghan 2006 - 2009.
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

#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include <boost/mpl/list.hpp>

#include "halIni.hpp"
#include "halTypes.hpp"
#include "halSignaler.hpp"
#include "halTorrentIntEvents.hpp"

namespace hal 
{
class TorrentInternalOld;
class torrent_internal;
class torrent_manager;
}

BOOST_CLASS_VERSION(hal::TorrentInternalOld, 9)
BOOST_CLASS_VERSION(hal::torrent_internal, 2)

namespace hal 
{

namespace libt = libtorrent;
namespace sc = boost::statechart;
namespace mpl = boost::mpl;


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

struct out_of_session;

class torrent_internal :
	public boost::enable_shared_from_this<torrent_internal>,
	public sc::state_machine<torrent_internal, out_of_session>
{
	friend class bit::torrent::exec_around_ptr::proxy;

	friend struct out_of_session;	
	friend struct in_the_session;
	
	friend struct active;
	friend struct pausing;
	friend struct paused;
	friend struct stopping;
	friend struct stopped;
	friend struct in_error;
	friend struct resume_data_waiting;
	friend struct resume_data_idling;

private:
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
	}

	torrent_internal(wpath filename, wpath saveDirectory, bit::allocations alloc, wpath move_to_directory=L"") :
		TORRENT_INTERNALS_DEFAULTS,
		save_directory_(saveDirectory.string()),
		move_to_directory_(move_to_directory.string()),
		allocation_(alloc)
	{
		state(torrent_details::torrent_stopped);
		assert(the_session_);

		prepare(filename);
	}

	#undef TORRENT_INTERNALS_DEFAULTS

public:
	void locked_process_event(const sc::event_base & e)
	{
		mutex_t::scoped_lock l(mutex_);
		process_event(e);
	}

	~torrent_internal()
	{
		terminate();
		TORRENT_STATE_LOG(L"Torrent state machine terminate");
	}
	
	torrent_details_ptr get_torrent_details_ptr();

	void adjust_queue_position(bit::queue_adjustments adjust);

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
	
	void add_to_session(bool paused = false);
	
	bool remove_from_session(bool write_data=true);

	bool in_session() const
	{ 
		mutex_t::scoped_lock l(mutex_);

		return (in_session_ && the_session_ != 0 && handle_.is_valid());
	}

	void resume()
	{
		mutex_t::scoped_lock l(mutex_);
		HAL_DEV_MSG(hal::wform(L"resume() - %1%") % name_);
		
		process_event(ev_resume());
	}
	
	void pause()
	{
		mutex_t::scoped_lock l(mutex_);
		HAL_DEV_MSG(hal::wform(L"pause() - %1%") % name_);
		
		process_event(ev_pause());		
	}
	
	void stop()
	{
		mutex_t::scoped_lock l(mutex_);
		HAL_DEV_MSG(hal::wform(L"stop() - %1%") % name_);
		
		process_event(ev_stop());
	}

	void set_state_stopped()
	{
		state(torrent_details::torrent_stopped);
	}

	void force_recheck()
	{
		mutex_t::scoped_lock l(mutex_);
		HAL_DEV_MSG(hal::wform(L"force_recheck() - %1%") % name_);
		
		process_event(ev_force_recheck());	
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
		wpath resume_file = hal::app().get_working_directory() / L"resume" / (name_ + L".fastresume");
		
		if (exists(resume_file))
			remove(resume_file);
	}

	void delete_torrent_file()
	{		
		wpath torrent_file = hal::app().get_working_directory() / L"torrents" / filename_;
		
		if (exists(torrent_file))
			remove(torrent_file);
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
		else 
			return false;
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
	}

	std::vector<libt::peer_info>& peers() { return peers_; }
	
	boost::tuple<size_t, size_t, size_t, size_t> update_peers();
	
	void get_peer_details(peer_details_vec& peer_details) const
	{
		if (in_session())
		{
			foreach (libt::peer_info peer, peers_) 
			{
				peer_details.insert(peer);
			}	
		}
	}

	void get_file_details(file_details_vec& files);
	file_details_vec get_file_details();
	
	void prepare(wpath filename);

	void set_resolve_countries(bool b)
	{
		resolve_countries_ = b;
		apply_resolve_countries();
	}
	
	void extract_names(boost::intrusive_ptr<libt::torrent_info> metadata);
	
	boost::intrusive_ptr<libt::torrent_info> info_memory()
	{
		if (!info_memory_) 
			info_memory_ = boost::intrusive_ptr<libt::torrent_info>
				(new libt::torrent_info(path_to_utf8(filename())));
		
		return info_memory_;
	}
	
	signalers& signals()
	{
		mutex_t::scoped_lock l(mutex_);
		return signals_;
	}	
	
	unsigned state() const 
	{ 
		if (!in_session())
		{
			if (state_ != torrent_details::torrent_stopped)
			{			
				HAL_DEV_MSG(L"Should really be stopped!");
				state_ = torrent_details::torrent_stopped;
			}
		}
		
		return state_; 
	}

	wstring check_error() 
	{		
		if (in_session())
		{
			status_memory_ = handle_.status();
			return from_utf8(status_memory_.error);
		}
		else
			return L"";
	}

	bool awaiting_resume_data() { return (state_downcast<const resume_data_waiting*>() != 0); }

	void output_torrent_debug_details()
	{
		HAL_DEV_MSG(wform(L"Name %1%") % name_);
		HAL_DEV_MSG(wform(L" >> In session       %1%") % in_session());

		if (in_session())
		{

		wstring state_str;

		switch (state())
		{
		case torrent_details::torrent_active:
			state_str = L"Active";
			break;

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
		};
		
		HAL_DEV_MSG(wform(L" >> State            %1%") % state_str);
		HAL_DEV_MSG(wform(L" >> Session Paused   %1%") % handle_.is_paused());
		HAL_DEV_MSG(wform(L" >> Error state      %1%") % check_error());

		}
	}

	static libt::session* the_session_;	

	friend class torrent_manager;

private:
	void apply_settings();	
	void apply_transfer_speed();
	void apply_connection_limit();	
	void apply_ratio();	
	void apply_trackers();	
	void apply_tracker_login();	
	void apply_file_priorities();	
	void apply_resolve_countries();
	void state(unsigned s);

	void initialize_state_machine(torrent_internal_ptr p)
	{
		own_weak_ptr_ = boost::weak_ptr<torrent_internal>(p);

		TORRENT_STATE_LOG(L"Torrent state machine initiate");
		initiate();
	}

	mutable mutex_t mutex_;
	signalers signals_;

	boost::weak_ptr<torrent_internal> own_weak_ptr_;
	
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
	int queue_position_;
	bool compact_storage_;
	bool managed_;
	bit::allocations allocation_;
	
	boost::intrusive_ptr<libt::torrent_info> info_memory_;
	libt::torrent_status status_memory_;
	file_details_vec file_details_memory_;
};

} // namespace hal

#include "halTorrentIntStates.hpp"

