
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
		return libt::internal_storage_mode_compact_deprecated;
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
	torrent_internal();
	torrent_internal(const wpath& filename, const wpath& save_directory, bit::allocations alloc, const wpath& move_to_directory=L"");
	torrent_internal(const wstring& uri, const wpath& save_directory, bit::allocations alloc, const wpath& move_to_directory=L"");
			
public:
	~torrent_internal()
	{
		terminate();
	}

	static void set_the_session(boost::optional<libt::session>*);
	bool in_session() const;
	
	torrent_details_ptr get_torrent_details_ptr() const;

	void adjust_queue_position(bit::queue_adjustments adjust);

	void set_transfer_speed(float down, float up)
	{	
		upgrade_lock l(mutex_);

		{	upgrade_to_unique_lock up_l(l);
			
			transfer_limit_ = std::make_pair(down, up);
		}
		
		apply_transfer_speed(l);
	}

	std::pair<float, float> get_transfer_speed() const
	{
		upgrade_lock l(mutex_);

		return transfer_limit_;
	}

	void set_connection_limit(int maxConn, int maxUpload)		
	{
		upgrade_lock l(mutex_);

		{	upgrade_to_unique_lock up_l(l);
			
			connections_ = maxConn;
			uploads_ = maxUpload;
		}
		
		apply_connection_limit(l);
	}

	std::pair<int, int> get_connection_limit() const
	{
		upgrade_lock l(mutex_);

		return std::make_pair(connections_, uploads_);
	}
	
	void set_name(const wstring& n);
	const wstring& name() const;
	
	const libt::sha1_hash& hash() const
	{
		upgrade_lock l(mutex_);

		return hash_; 
	}

	const uuid& id() const;

	void set_managed(bool m);

	void set_superseeding(bool ss);
	bool get_superseeding(bool actually=false) const;
	
	const wpath& save_directory() const;
	
	void add_to_session(bool paused = false);
	
	bool remove_from_session(bool write_data=true);
	
	void remove_files(function<void (wpath, boost::shared_ptr<std::vector<std::wstring> >)> fn);

	void resume()
	{		
		process_event(ev_resume());
	}
	
	void pause()
	{		
		process_event(ev_pause());		
	}
	
	void stop()
	{		
		process_event(ev_stop());
	}
	
	void start()
	{		
		process_event(ev_start());
	}
	
	void set_state_stopped()
	{
		upgrade_lock l(mutex_);

		state(l, torrent_details::torrent_stopped);
	}

	void force_recheck()
	{		
		process_event(ev_force_recheck());	
	}
	
	void write_resume_data(const libt::entry& ent)
	{		
		upgrade_lock l(mutex_);
			
		HAL_DEV_MSG(L"write_resume_data()");

		wpath resume_dir = hal::app().get_working_directory()/L"resume";
		
		if (!exists(resume_dir))
			fs::create_directories(resume_dir);

		boost::filesystem::ofstream out(resume_dir/(name(l) + L".fastresume"), std::ios_base::binary);
		out.unsetf(std::ios_base::skipws);
		bencode(std::ostream_iterator<char>(out), ent);

		HAL_DEV_MSG(L"Written!");
	}

	void save_resume_and_info_data() const
	{
		upgrade_lock l(mutex_);

		handle_.save_resume_data();

		write_torrent_info(l);
	}
	
	void clear_resume_data()
	{
		upgrade_lock l(mutex_);

		try {

		wpath resume_file = hal::app().get_working_directory() / L"resume" / (name(l) + L".fastresume");

		if (exists(resume_file))
			remove(resume_file);

		} 
		catch (const boost::filesystem::filesystem_error&)
		{
			event_log().post(shared_ptr<EventDetail>(
				new EventMsg(L"Resume data removal error.", event_logger::warning)));
		}
	}
	
	void clear_torrent_info()
	{
		upgrade_lock l(mutex_);

		try {

		wpath torrent_info_file = hal::app().get_working_directory() / L"resume" / (name(l) + L".torrent_info");

		if (exists(torrent_info_file))
			remove(torrent_info_file);

		} 
		catch (const boost::filesystem::filesystem_error&)
		{
			event_log().post(shared_ptr<EventDetail>(
				new EventMsg(L"Torrent info removal error.", event_logger::warning)));
		}
	}

	void delete_torrent_file()
	{		
		upgrade_lock l(mutex_);

		try {

		wpath torrent_file = hal::app().get_working_directory() / L"torrents" / filename_;
		
		if (exists(torrent_file))
			remove(torrent_file);

		}
		catch (const boost::filesystem::filesystem_error&)
		{
			event_log().post(shared_ptr<EventDetail>(
				new EventMsg(L"Torrent file removal error.", event_logger::warning)));
		}
	}
	
	void set_file_priorities(std::vector<int> file_indices, int priority)
	{
		upgrade_lock l(mutex_);

		{	upgrade_to_unique_lock up_l(l);
			
			files_.set_file_priorities(file_indices, priority, l);
		}
				
		apply_file_priorities(l);
	}

	void get_file_details(file_details_vec& files);

	file_details_vec get_file_details();

	void set_save_directory(wpath s, bool force=false)
	{
		upgrade_lock l(mutex_);

		if (in_session(l) && !is_finished() &&
				s != path_from_utf8(handle_.save_path()))
		{
			upgrade_to_unique_lock up_l(l);

			handle_.move_storage(s.string());
			
			save_directory_ = s;
		}
		else if (!in_session(l) && force)
		{
			upgrade_to_unique_lock up_l(l);
			
			save_directory_ = s;
		}
	}

	const wpath get_move_to_directory() const
	{
		upgrade_lock l(mutex_);

		return move_to_directory_;
	}
	
	void set_move_to_directory(wpath m)
	{
		upgrade_lock l(mutex_);

		if (is_finished() && !m.empty())
		{
			if (m != path_from_utf8(handle_.save_path()))
			{
				upgrade_to_unique_lock up_l(l);

				handle_.move_storage(m.string());
				save_directory_ = move_to_directory_ = m;
			}
		}
		else
		{
			upgrade_to_unique_lock up_l(l);

			move_to_directory_ = m;
		}
	}

	bool is_finished() const;
	
	bool is_active() const;

	void set_tracker_login(wstring username, wstring password)
	{
		upgrade_lock l(mutex_);

		{	upgrade_to_unique_lock up_l(l);
			tracker_username_ = username;
			tracker_password_ = password;
		}
		
		apply_tracker_login(l);
	}
	
	std::pair<wstring, wstring> get_tracker_login() const
	{
		upgrade_lock l(mutex_);

		return make_pair(tracker_username_, tracker_password_);
	}
	
	const wstring& filename() const 
	{
		upgrade_lock l(mutex_);
	
		return filename_; 
	}
	
	const wstring& original_filename() const 
	{ 
		upgrade_lock l(mutex_);

		return original_filename_; 
	}

	const libt::torrent_handle& set_handle(const libt::torrent_handle& h)
	{
		upgrade_lock l(mutex_);
		upgrade_to_unique_lock up_l(l);

		return handle_ = h;
	}
	
	const libt::torrent_handle& handle() const 
	{ 
		upgrade_lock l(mutex_);

		return handle_; 
	}

	void reset_trackers()
	{
		upgrade_lock l(mutex_);

		if (in_session(l))
		{
			handle_.replace_trackers(torrent_trackers_);	

			{	upgrade_to_unique_lock up_l(l);	
				trackers_.clear();
			}
		}
	}
	
	void set_trackers(const std::vector<tracker_detail>& tracker_details)
	{
		upgrade_lock l(mutex_);

		{	upgrade_to_unique_lock up_l(l);	
			
			trackers_.clear();
			trackers_.assign(tracker_details.begin(), tracker_details.end());
		}
		
		apply_trackers(l);
	}
	
	const std::vector<tracker_detail>& get_trackers()
	{
		upgrade_lock l(mutex_);

		if (trackers_.empty() && info_memory(l))
		{
			std::vector<libt::announce_entry> trackers = info_memory(l)->trackers();
			
			BOOST_FOREACH (const libt::announce_entry& entry, trackers)
			{
				upgrade_to_unique_lock up_l(l);	

				trackers_.push_back(
					tracker_detail(hal::from_utf8(entry.url), entry.tier));
			}
		}		
		return trackers_;
	}

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		upgrade_lock l(mutex_);

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
			upgrade_to_unique_lock up_l(l);	

			uuid_ = boost::uuids::random_generator()();
		}
	}

	const std::vector<libt::peer_info>& peers() const
	{
		upgrade_lock l(mutex_);

		return peers_; 
	}
		
	void get_peer_details(peer_details_vec& peer_details) const
	{
		upgrade_lock l(mutex_);

		if (in_session(l))
		{
			BOOST_FOREACH (libt::peer_info peer, peers_) 
			{
				peer_details.insert(peer);
			}	
		}
	}
	
	void alert_finished();
	void alert_file_completed(int index);
	void alert_metadata_completed();
	void alert_storage_moved(const fs::wpath& p);

	void set_resolve_countries(bool b)
	{
		upgrade_lock l(mutex_);

		{	upgrade_to_unique_lock up_l(l);	
			resolve_countries_ = b;
		}
		apply_resolve_countries(l);
	}

	void set_use_external_interface(std::wstring inter)
	{
		upgrade_lock l(mutex_);

		{	upgrade_to_unique_lock up_l(l);	
			external_interface_.reset(inter);
		}
		apply_external_interface(l);
	}

	void set_no_external_interface()
	{
		upgrade_lock l(mutex_);

		{	upgrade_to_unique_lock up_l(l);	
			external_interface_.reset();
		}
		apply_external_interface(l);
	}
	
	unsigned state() const;

	wstring check_error() const;

	bool awaiting_resume_data() { return (state_downcast<const resume_data_waiting*>() != 0); }

	void output_torrent_debug_details() const;

	torrent_files& files() { return files_; }		

	friend class torrent_manager;

private:
	void apply_settings(upgrade_lock& l);	
	void apply_transfer_speed(upgrade_lock& l);
	void apply_connection_limit(upgrade_lock& l);	
	void apply_trackers(upgrade_lock& l);	
	void apply_tracker_login(upgrade_lock& l);	
	void apply_file_priorities(upgrade_lock& l);	
	void apply_file_names(upgrade_lock& l);	
	void apply_resolve_countries(upgrade_lock& l);	
	void apply_superseeding(upgrade_lock& l);
	void apply_queue_position();
	void apply_queue_position(upgrade_lock& l);	
	void apply_external_interface(upgrade_lock& l);	
	
	void extract_hash(upgrade_lock& l);
	void extract_names(upgrade_lock& l);
	void extract_filenames(upgrade_lock& l);	
	void init_file_details(upgrade_lock& l);
	
	static boost::optional<libt::session>* the_session_;
	bool in_session(upgrade_lock& l) const;

	boost::intrusive_ptr<libt::torrent_info> info_memory(upgrade_lock& l) const;	
	void info_memory_reset(libt::torrent_info* im, upgrade_lock& l);

	libt::torrent_status& status_cache(upgrade_lock& l) const;
	libt::torrent_status& renew_status_cache(upgrade_lock& l) const;

	const wstring& name(upgrade_lock& l) const;	
	const uuid& id(upgrade_lock& l) const;
	const wpath& save_directory(upgrade_lock& l) const;
	bool is_managed(upgrade_lock& l) const;	
	bool is_active(upgrade_lock& l) const;	
	bool is_finished(upgrade_lock& l) const;
	bool get_superseeding(bool actually, upgrade_lock& l) const;

	unsigned state(upgrade_lock& l) const;
	void state(upgrade_lock& l, unsigned s);

	void prepare(upgrade_lock& l) { prepare(l, info_memory(l)); }
	void prepare(upgrade_lock& l, boost::intrusive_ptr<libt::torrent_info> info);	

	void write_torrent_info(upgrade_lock& l) const;
	boost::tuple<size_t, size_t, size_t, size_t> update_peers(upgrade_lock& l) const;
	void get_file_details(upgrade_lock& l, file_details_vec& files_vec);

	void update_manager(upgrade_lock& l);
	void initialize_non_serialized(function<void (torrent_internal_ptr)>);
	
	wstring state_string(upgrade_lock& l) const;
	void output_torrent_debug_details(upgrade_lock& l) const;
	wstring check_error(upgrade_lock& l) const;
	
	void set_info_cache(boost::intrusive_ptr<libt::torrent_info> info, upgrade_lock& l)
	{
		upgrade_to_unique_lock up_l(l);

		{	upgrade_to_unique_lock up_l(l);	
			info_memory_ = info;
		}
	}

	void set_file_priority_cb(size_t i, int p, upgrade_lock& l);	
	void changed_file_filename_cb(size_t i, upgrade_lock& l);

	function<void ()> remove_callback_;
	function<void (void)>& remove_callback(upgrade_lock& l) { return remove_callback_; }
	
	function<void (torrent_internal_ptr)> update_manager_;

	mutable boost::shared_mutex mutex_;
	mutable boost::mutex details_mutex_;
	mutable torrent_details_ptr details_ptr_;
	
	std::pair<float, float> transfer_limit_;
	
	mutable unsigned state_;
	int connections_;
	int uploads_;
	bool in_session_;
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
