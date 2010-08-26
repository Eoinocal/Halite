
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

#include "halTorrentDefines.hpp"

#ifndef HAL_TORRENT_STATE_LOGGING
#	define TORRENT_STATE_LOG(s)
#else
#	include "../halEvent.hpp"
#	define TORRENT_STATE_LOG(msg) \
	hal::event_log().post(boost::shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(msg, hal::event_logger::torrent_dev))) 
#endif

#include "halIni.hpp"
#include "halTypes.hpp"
#include "halSignaler.hpp"

#include "halTorrentSerialization.hpp"
#include "halTorrentFile.hpp"
#include "halTorrentIntEvents.hpp"

namespace hal 
{
class TorrentInternalOld;
class torrent_internal;
class torrent_manager;
}

BOOST_CLASS_VERSION(hal::TorrentInternalOld, 9)
BOOST_CLASS_VERSION(hal::torrent_internal, 3)

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
		libt::torrent_info info(file.string());

		std::string name = info.name();	
		std::string filename = name;

		if (!boost::find_last(filename, ".torrent")) 
				filename += ".torrent";
		
		event_log().post(shared_ptr<EventDetail>(new EventMsg(
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
		if (rel_total >= total_offset_)
			total_ += (rel_total - total_offset_);

		total_offset_ = rel_total;
		
		return total_;
	}
	
	void set_offset(T offset) const
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
			total_.set_offset(boost::posix_time::time_duration(0,0,0,0));
		
		return total_.update(boost::posix_time::second_clock::universal_time() - start_);
	}
	
	void reset() const
	{
		total_.set_offset(boost::posix_time::time_duration(0,0,0,0));
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
	
class torrent_internal;
typedef shared_ptr<torrent_internal> torrent_internal_ptr;

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
	friend struct not_started;
	friend struct resume_data_waiting;
	friend struct resume_data_idling;
	friend struct removing;

private:
	#define TORRENT_INTERNALS_DEFAULTS \
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
		queue_position_(-1), \
		hash_(0), \
		superseeding_(false), \
		files_(bind(&torrent_internal::set_file_priority_cb, this, _1, _2), \
			bind(&torrent_internal::changed_file_filename_cb, this, _1))
		
	torrent_internal() :	
		TORRENT_INTERNALS_DEFAULTS,
		allocation_(bit::sparse_allocation)
	{
		state(torrent_details::torrent_stopped);
	}

	torrent_internal(const wpath& filename, const wpath& save_directory, bit::allocations alloc, const wpath& move_to_directory=L"") :
		TORRENT_INTERNALS_DEFAULTS,
		uuid_(boost::uuids::random_generator()()),
		save_directory_(save_directory.string()),
		move_to_directory_(move_to_directory.string()),
		allocation_(alloc)
	{
		unique_lock_t l(mutex_);

		state(torrent_details::torrent_stopped);
		assert(the_session_);

		prepare(l, new libt::torrent_info(filename.string()));
	}

	torrent_internal(const wstring& uri, const wpath& save_directory, bit::allocations alloc, const wpath& move_to_directory=L"") :
		TORRENT_INTERNALS_DEFAULTS,
		uuid_(boost::uuids::random_generator()()),
		save_directory_(save_directory.string()),
		move_to_directory_(move_to_directory.string()),
		allocation_(alloc),
		magnet_uri_(to_utf8(uri))
	{
		unique_lock_t l(mutex_);

		state(torrent_details::torrent_stopped);
		assert(the_session_);

		extract_hash(l);
	}

	#undef TORRENT_INTERNALS_DEFAULTS

public:
	void locked_process_event(const sc::event_base & e)
	{
		unique_lock_t l(mutex_);
		process_event(e);
	}

	~torrent_internal()
	{
		terminate();
		TORRENT_STATE_LOG(L"Torrent state machine terminate");
	}

	static void set_the_session(boost::optional<libt::session>*);
	bool in_session() const;
	
	torrent_details_ptr get_torrent_details_ptr() const;

	void adjust_queue_position(bit::queue_adjustments adjust);

	void set_transfer_speed(float down, float up)
	{	
		unique_lock_t l(mutex_);

		transfer_limit_ = std::make_pair(down, up);
		
		apply_transfer_speed(l);
	}

	std::pair<float, float> get_transfer_speed() const
	{
		unique_lock_t l(mutex_);

		return transfer_limit_;
	}

	void set_connection_limit(int maxConn, int maxUpload)		
	{
		unique_lock_t l(mutex_);

		connections_ = maxConn;
		uploads_ = maxUpload;
		
		apply_connection_limit(l);
	}

	std::pair<int, int> get_connection_limit() const
	{
		unique_lock_t l(mutex_);

		return std::make_pair(connections_, uploads_);
	}
	
	const wstring& name() const;
	void set_name(const wstring& n);
	
	const libt::sha1_hash& hash() const
	{
		unique_lock_t l(mutex_);

		return hash_; 
	}

	const boost::uuids::uuid& uuid() const
	{ 
		unique_lock_t l(mutex_);

		return uuid_;
	}
	
	void set_ratio(float ratio) 
	{ 
		unique_lock_t l(mutex_);

		if (ratio < 0) ratio = 0;
		ratio_ = ratio; 
		
		apply_ratio(l);
	}
	
	float get_ratio() const
	{
		unique_lock_t l(mutex_);

		return ratio_;
	}

	void set_managed(bool m);
	bool is_managed() const;

	void set_superseeding(bool ss);
	bool get_superseeding(bool actually=false) const;
	
	void add_to_session(bool paused = false);
	
	bool remove_from_session(bool write_data=true);

	void resume()
	{
		HAL_DEV_MSG(hal::wform(L"resume() - %1%") % name());
		
		locked_process_event(ev_resume());
	}
	
	void pause()
	{
		HAL_DEV_MSG(hal::wform(L"pause() - %1%") % name());
		
		locked_process_event(ev_pause());		
	}
	
	void stop()
	{
		HAL_DEV_MSG(hal::wform(L"stop() - %1%") % name());
		
		locked_process_event(ev_stop());
	}
	
	void start()
	{
		HAL_DEV_MSG(hal::wform(L"start() - %1%") % name());
		
		locked_process_event(ev_start());
	}
	
	void remove_files(boost::function<void (void)> fn)
	{
		HAL_DEV_MSG(hal::wform(L"remove_files() - %1%") % name());

		removed_callback_ = fn;
		
		locked_process_event(ev_stop());
	}

	void set_state_stopped()
	{
		unique_lock_t l(mutex_);

		state(torrent_details::torrent_stopped);
	}

	void force_recheck()
	{
		HAL_DEV_MSG(hal::wform(L"force_recheck() - %1%") % name());
		
		locked_process_event(ev_force_recheck());	
	}
	
	void write_resume_data(const libt::entry& ent)
	{		
		unique_lock_t l(mutex_);
			
		HAL_DEV_MSG(L"write_resume_data()");

		wpath resume_dir = hal::app().get_working_directory()/L"resume";
		
		if (!exists(resume_dir))
			fs::create_directories(resume_dir);

		boost::filesystem::ofstream out(resume_dir/(name() + L".fastresume"), std::ios_base::binary);
		out.unsetf(std::ios_base::skipws);
		bencode(std::ostream_iterator<char>(out), ent);

		HAL_DEV_MSG(L"Written!");
	}

	void write_torrent_info() const;

	void save_resume_and_info_data() const
	{
		unique_lock_t l(mutex_);

		handle_.save_resume_data();

		write_torrent_info();
	}
	
	void clear_resume_data()
	{
		unique_lock_t l(mutex_);

		try {

		wpath resume_file = hal::app().get_working_directory() / L"resume" / (name() + L".fastresume");

		if (exists(resume_file))
			remove(resume_file);

		} 
		catch (const boost::filesystem::wfilesystem_error&)
		{
			event_log().post(shared_ptr<EventDetail>(
				new EventMsg(L"Resume data removal error.", event_logger::warning)));
		}
	}
	
	void clear_torrent_info()
	{
		unique_lock_t l(mutex_);

		try {

		wpath torrent_info_file = hal::app().get_working_directory() / L"resume" / (name() + L".torrent_info");

		if (exists(torrent_info_file))
			remove(torrent_info_file);

		} 
		catch (const boost::filesystem::wfilesystem_error&)
		{
			event_log().post(shared_ptr<EventDetail>(
				new EventMsg(L"Torrent info removal error.", event_logger::warning)));
		}
	}

	void delete_torrent_file()
	{		
		unique_lock_t l(mutex_);

		try {

		wpath torrent_file = hal::app().get_working_directory() / L"torrents" / filename_;
		
		if (exists(torrent_file))
			remove(torrent_file);

		}
		catch (const boost::filesystem::wfilesystem_error&)
		{
			event_log().post(shared_ptr<EventDetail>(
				new EventMsg(L"Torrent file removal error.", event_logger::warning)));
		}
	}
	
	void set_file_priorities(std::vector<int> file_indices, int priority)
	{
		unique_lock_t l(mutex_);

		files_.set_file_priorities(file_indices, priority);
				
		apply_file_priorities(l);
	}

	void set_file_finished(int index);

	void init_file_details();
	void get_file_details(file_details_vec& files);

	file_details_vec get_file_details();

	const wpath get_save_directory() const
	{
		unique_lock_t l(mutex_);

		return save_directory_;
	}

	void set_save_directory(wpath s, bool force=false)
	{
		unique_lock_t l(mutex_);

		if (in_session(l) && !is_finished() &&
				s != path_from_utf8(handle_.save_path()))
		{
			handle_.move_storage(s.string());
			save_directory_ = s;
		}
		else if (!in_session(l) && force)
		{
			save_directory_ = s;
		}
	}

	const wpath get_move_to_directory() const
	{
		unique_lock_t l(mutex_);

		return move_to_directory_;
	}
	
	void set_move_to_directory(wpath m)
	{
		unique_lock_t l(mutex_);

		if (is_finished() && !m.empty())
		{
			if (m != path_from_utf8(handle_.save_path()))
			{
				handle_.move_storage(m.string());
				save_directory_ = move_to_directory_ = m;
			}
		}
		else
		{
			move_to_directory_ = m;
		}
	}

	bool is_finished() const
	{
		unique_lock_t l(mutex_);

		if (in_session(l))
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
		unique_lock_t l(mutex_);

		if (finish_time_.is_special())
			finish_time_ = boost::posix_time::second_clock::universal_time();

		if (is_finished())
		{
			if (!move_to_directory_.empty() && 
					move_to_directory_ !=  path_from_utf8(handle_.save_path()))
			{
				handle_.move_storage(move_to_directory_.string());
				save_directory_ = move_to_directory_;
			}

			apply_superseeding(l);
		}
	}
	
	bool is_active() const 
	{ 
		unique_lock_t l(mutex_);

		return state() == torrent_details::torrent_active; 
	}

	void set_tracker_login(wstring username, wstring password)
	{
		unique_lock_t l(mutex_);

		tracker_username_ = username;
		tracker_password_ = password;
		
		apply_tracker_login(l);
	}
	
	std::pair<wstring, wstring> get_tracker_login() const
	{
		unique_lock_t l(mutex_);

		return make_pair(tracker_username_, tracker_password_);
	}
	
	const wstring& filename() const 
	{
		unique_lock_t l(mutex_);
	
		return filename_; 
	}
	
	const wstring& original_filename() const 
	{ 
		unique_lock_t l(mutex_);

		return original_filename_; 
	}
	
	const libt::torrent_handle& handle() const 
	{ 
		unique_lock_t l(mutex_);

		return handle_; 
	}

	void reset_trackers()
	{
		unique_lock_t l(mutex_);

		if (in_session(l))
		{
			handle_.replace_trackers(torrent_trackers_);		
			trackers_.clear();
		}
	}
	
	void set_trackers(const std::vector<tracker_detail>& tracker_details)
	{
		unique_lock_t l(mutex_);

		trackers_.clear();
		trackers_.assign(tracker_details.begin(), tracker_details.end());
		
		apply_trackers(l);
	}
	
	const std::vector<tracker_detail>& get_trackers()
	{
		unique_lock_t l(mutex_);

		if (trackers_.empty() && info_memory(l))
		{
			std::vector<libt::announce_entry> trackers = info_memory(l)->trackers();
			
			foreach (const libt::announce_entry& entry, trackers)
			{
				trackers_.push_back(
					tracker_detail(hal::from_utf8(entry.url), entry.tier));
			}
		}		
		return trackers_;
	}

	const wpath& save_directory() const
	{ 
		unique_lock_t l(mutex_);

		return save_directory_; 
	}
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		unique_lock_t l(mutex_);

		using boost::serialization::make_nvp;

		if (version > 1) {

			switch (version)
			{
			case 3:
			ar & make_nvp("super_seeding", superseeding_);
			ar & make_nvp("files", files_);
			ar & make_nvp("uuid", uuid_);
			ar & make_nvp("hash", hash_);
			ar & make_nvp("hash_string", hash_str_);

			case 2:
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
			ar & make_nvp("queue_position", queue_position_);
			ar & make_nvp("payload_downloaded", payload_downloaded_);
			ar & make_nvp("uploaded", uploaded_);
			ar & make_nvp("downloaded", downloaded_);			
			
			if (version == 2)
				ar & make_nvp("file_priorities", file_priorities_);
			
			ar & make_nvp("start_time", start_time_);
			ar & make_nvp("finish_time", finish_time_);
			ar & make_nvp("active_duration", active_duration_);
			ar & make_nvp("seeding_duration", seeding_duration_);
			ar & make_nvp("managed", managed_);		
			}
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

		if (version < 3)
		{
			uuid_ = boost::uuids::random_generator()();
		}
	}

	const std::vector<libt::peer_info>& peers() const
	{
		unique_lock_t l(mutex_);

		return peers_; 
	}
	
	boost::tuple<size_t, size_t, size_t, size_t> update_peers() const;
	
	void get_peer_details(peer_details_vec& peer_details) const
	{
		unique_lock_t l(mutex_);

		if (in_session(l))
		{
			foreach (libt::peer_info peer, peers_) 
			{
				peer_details.insert(peer);
			}	
		}
	}

	void metadata_completed();

	void set_resolve_countries(bool b)
	{
		unique_lock_t l(mutex_);

		resolve_countries_ = b;
		apply_resolve_countries(l);
	}

	void set_use_external_interface(std::wstring inter)
	{
		unique_lock_t l(mutex_);

		external_interface_.reset(inter);
		apply_external_interface(l);
	}

	void set_no_external_interface()
	{
		unique_lock_t l(mutex_);

		external_interface_.reset();
		apply_external_interface(l);
	}
	
	void set_info_memory(boost::intrusive_ptr<libt::torrent_info> info)
	{		
		info_memory_ = info;
	}
	
	unsigned state() const 
	{
		return state_;
	}

	wstring check_error() const;

	bool awaiting_resume_data() { return (state_downcast<const resume_data_waiting*>() != 0); }

	void output_torrent_debug_details() const;

	torrent_files& files() { return files_; }		

	friend class torrent_manager;

private:
	void apply_settings(unique_lock_t&);	
	void apply_transfer_speed(unique_lock_t&);
	void apply_connection_limit(unique_lock_t&);	
	void apply_ratio(unique_lock_t&);	
	void apply_trackers(unique_lock_t&);	
	void apply_tracker_login(unique_lock_t&);	
	void apply_file_priorities(unique_lock_t&);	
	void apply_file_names(unique_lock_t&);	
	void apply_resolve_countries(unique_lock_t&);	
	void apply_superseeding(unique_lock_t&);
	void apply_queue_position();
	void apply_queue_position(unique_lock_t&);	
	void apply_external_interface(unique_lock_t&);	
	
	void extract_hash(unique_lock_t&);
	void extract_names(unique_lock_t&);
	void extract_filenames(unique_lock_t&);	
	
	static boost::optional<libt::session>* the_session_;
	bool in_session(unique_lock_t& l) const;

	boost::intrusive_ptr<libt::torrent_info> info_memory(unique_lock_t&) const;	
	bool is_managed(unique_lock_t&) const;	
	void state(unsigned s);

	void prepare(unique_lock_t& l) { prepare(l, info_memory(l)); }
	void prepare(unique_lock_t& l, boost::intrusive_ptr<libt::torrent_info> info);

	void update_manager(unique_lock_t&);
	void initialize_non_serialized(function<void (torrent_internal_ptr)>);
	
	void output_torrent_debug_details(unique_lock_t& l) const;

	void set_file_priority_cb(size_t i, int p)
	{
		if (i < file_priorities_.size())
		{
			file_priorities_[i] = p;
			file_details_memory_[i].priority = p;
		}
	}
	
	void changed_file_filename_cb(size_t i)
	{
		if (i < file_details_memory_.size())
		{
			torrent_file::split_path_pair_t split = torrent_file::split_root(files_[i].completed_name());

			file_details_memory_[i].filename = split.second.filename();
			file_details_memory_[i].branch = split.second.parent_path();

			if (files_[i].is_finished())
				handle_.rename_file(i, files_[i].completed_name());
		}
	}

	function<void ()> removed_callback_;
	function<void (torrent_internal_ptr)> update_manager_;

	mutable mutex_t mutex_;
	
	std::pair<float, float> transfer_limit_;
	
	mutable unsigned state_;
	int connections_;
	int uploads_;
	bool in_session_;
	float ratio_;
	bool resolve_countries_;

	boost::optional<std::wstring> external_interface_;
	
	wstring filename_;
	mutable wstring name_;
	wpath save_directory_;
	wpath move_to_directory_;
	wstring original_filename_;
	libt::torrent_handle handle_;	
	wstring tracker_username_;	
	wstring tracker_password_;
	
	pt::ptime start_time_;
	pt::ptime finish_time_;
	duration_tracker active_duration_;
	duration_tracker seeding_duration_;
	
	std::vector<tracker_detail> trackers_;
	std::vector<libt::announce_entry> torrent_trackers_;
	std::vector<int> file_priorities_;

	torrent_files files_;
	
	bool compact_storage_;
	bit::allocations allocation_;

	boost::uuids::uuid uuid_;
	libt::sha1_hash hash_;
	wstring hash_str_;

	bool superseeding_;
	
	std::string magnet_uri_;


	// Cached values	
	mutable boost::int64_t total_uploaded_;
	mutable boost::int64_t total_base_;
	
	transfer_tracker<boost::int64_t> payload_uploaded_;
	transfer_tracker<boost::int64_t> payload_downloaded_;
	transfer_tracker<boost::int64_t> uploaded_;
	transfer_tracker<boost::int64_t> downloaded_;

	mutable std::vector<libt::peer_info> peers_;	

	mutable float progress_;	
	mutable int queue_position_;
	mutable bool managed_;
	
	mutable boost::intrusive_ptr<libt::torrent_info> info_memory_;
	mutable libt::torrent_status status_memory_;
	file_details_vec file_details_memory_;
};

} // namespace hal

#include "halTorrentIntStates.hpp"
