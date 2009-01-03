
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/*#include <string>
#include <vector>
#include <set>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include <boost/smart_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <boost/signal.hpp>
#include <boost/optional.hpp>
#include <boost/function.hpp>

#include <boost/smart_ptr.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
*/

#include <boost/algorithm/string.hpp>

#include <stlsoft/properties/method_properties.hpp>
#include <stlsoft/util/operator_bool_adaptor.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

#include "halTypes.hpp"
#include "halPeers.hpp"

namespace hal 
{

template<typename T>
bool hal_details_ptr_compare(T l, T r, size_t index = 0, bool cmp_less = true)
{
	if (cmp_less)
		return l->less(*r, index);
	else
		return r->less(*l, index);
}

template<typename T>
bool hal_details_compare(T l, T r, size_t index = 0, bool cmp_less = true)
{
	if (cmp_less)
		return l.less(r, index);
	else
		return r.less(l, index);
}

inline boost::wformat wform(const std::wstring & f_string) 
{
    using namespace boost::io;

	boost::wformat fmter(f_string);
    fmter.exceptions( no_error_bits  );
    return fmter;
}

struct torrentBriefDetail 
{
	std::wstring filename;
	std::wstring status;
	std::pair<float,float> speed;
	float completion;
	int peers;
	int seeds;
};

struct queue_settings;
struct timeouts;
struct dht_settings;
struct cache_settings;
struct pe_settings;
struct connections;
struct cache_details;


struct file_details
{
	enum details
	{
		filename_e = 0,
		branch_e,
		size_e,
		progress_e,
		priority_e,
		type_e
	};

	file_details(boost::filesystem::wpath p, 
			boost::int64_t s=0, 
			boost::int64_t pg=0, 
			int pr=1, 
			size_t o=0, 
			unsigned t=file_details::file) :
		branch(p.parent_path()),
		filename(p.filename()),
		type(t),
		size(s),
		progress(pg),
		priority(pr),
		order_(o)
	{}
	
	bool operator==(const file_details& file) const
	{
		return (branch == file.branch);
	}
	
	bool operator<(const file_details& file) const
	{
		return (branch < file.branch);
	}
	
	bool less(const file_details& r, size_t index = 0) const;
	std::wstring to_wstring(size_t index = 0);
	
	enum file_type
	{
		folder,
		file
	};
	
	size_t order() const { return order_; }
	
	boost::filesystem::wpath branch;
	std::wstring filename;
	unsigned type;
	boost::int64_t size;
	boost::int64_t progress;
	int priority;
	
private:
	size_t order_;
};

inline bool file_details_names_equal(const file_details& l, const file_details& r)
{
	return l.filename == r.filename;
}

inline bool file_details_names_less(const file_details& l, const file_details& r)
{
	return l.filename < r.filename;
}

typedef std::vector<file_details> file_details_vec;

void file_details_sort(file_details_vec& f, size_t index, bool cmp_less = true);

class torrent_details 
{
public:
	torrent_details(std::wstring n, std::wstring f, 
			std::wstring sd, 
			std::wstring s, 
			std::wstring cT, 
			std::pair<float,float> sp=std::pair<float,float>(0,0),
			float c=0, float d=0, 
			size_type tWD=0, size_type tW=0, 
			size_type tU=0, size_type tpU=0, 
			size_type tD=0, size_type tpD=0, 
			boost::tuple<size_type, size_type, size_type, size_type> connections = 
				boost::tuple<size_type, size_type, size_type, size_type>(0,0,0,0), 
			float r=0, 
			boost::posix_time::time_duration eta=boost::posix_time::seconds(0), 
			boost::posix_time::time_duration uIn=boost::posix_time::seconds(0),
			boost::posix_time::time_duration actve=boost::posix_time::seconds(0), 
			boost::posix_time::time_duration seding=boost::posix_time::seconds(0), 
			boost::posix_time::ptime srt=boost::posix_time::second_clock::universal_time(), 
			boost::posix_time::ptime fin=boost::posix_time::second_clock::universal_time(), 
			int q_p=-1, 
			bool man=false) :
		filename_(f),
		name_(n),
		saveDir_(sd),
		state_(s),
		currentTracker_(cT),
		speed_(sp),
		completion_(c),
		distributed_copies_(d),
		total_wanted_done_(tWD),
		total_wanted_(tW),
		total_uploaded_(tU),
		total_payload_uploaded_(tpU),
		total_downloaded_(tD),
		total_payload_downloaded_(tpD),
		peers_(connections.get<0>()),
		connected_peers_(connections.get<1>()),
		seeds_(connections.get<2>()),
		connected_seeds_(connections.get<3>()),
		ratio_(r),
		estimated_time_left_(eta),
		update_tracker_in_(uIn),
		peer_details_filled_(false),
		file_details_filled_(false),
		active_(actve),
		seeding_(seding),
		start_time_(srt),
		finish_time_(fin),
		queue_position_(q_p),
		managed_(man)
	{}

	torrent_details() :	
		peer_details_filled_(false),
		file_details_filled_(false)
	{};	
	
	enum state
	{
		torrent_active = 0,
		torrent_paused,
		torrent_stopped,
		torrent_pausing,
		torrent_stopping
	};

	enum details
	{
		name_e = 0,
		state_e,
		progress_e,
		speed_down_e,
		speed_up_e,
		peers_e,
		seeds_e,
		eta_e,
		distributed_copies_e,
		tracker,
		update_tracker_in_e,
		ratio_e,
		total_wanted_e,
		completed_e,
		remaining_e,
		downloaded_e,
		uploaded_e,
		active_time_e,
		seeding_time_e,
		start_time_e,
		finish_time_e,
		managed_e,
		queue_position_e
	};
	
//	const std::wstring& filename() const { return filename_; }
	const std::wstring& name() const { return name_; }
	const std::wstring& save_directory() const { return saveDir_; }
	const std::wstring& state() const { return state_; }
	const std::wstring& current_tracker() const { return currentTracker_; }
	
	std::pair<float,float> speed() const { return speed_; }
	const float& completion() const { return completion_; }
	const float& distributed_copies() const { return distributed_copies_; }
	
	size_type total_uploaded() const { return total_uploaded_; }
	size_type total_payload_uploaded() const { return total_payload_uploaded_; }
	size_type total_downloaded() const { return total_downloaded_; }
	size_type total_payload_downloaded() const { return total_payload_downloaded_; }
	size_type total_wanted_done() const { return total_wanted_done_; }
	size_type total_wanted() const { return total_wanted_; }
	
	size_type peers() const { return peers_; }
	size_type peers_connected() const { return connected_peers_; }
	size_type seeds() const { return seeds_; }
	size_type seeds_connected() const { return connected_seeds_; }
	
	float ratio() { return ratio_; }
	
	const boost::posix_time::time_duration& estimated_time_left() { return estimated_time_left_; }
	const boost::posix_time::time_duration& update_tracker_in() { return update_tracker_in_; }
	
	const peer_details_vec& get_peer_details() const;
	const file_details_vec& get_file_details() const;
	
	const boost::posix_time::time_duration& active() { return active_; }
	const boost::posix_time::time_duration& seeding() { return seeding_; }
	const boost::posix_time::ptime& start_time() { return start_time_; }
	const boost::posix_time::ptime& finish_time() { return finish_time_; }

	int queue_position() const { return queue_position_; }
	bool managed() const { return managed_; }

	bool less(const torrent_details& r, size_t index = 0) const;
	std::wstring to_wstring(size_t index = 0);
	
public:
	std::wstring filename_;
	std::wstring name_;
	std::wstring saveDir_;
	std::wstring state_;
	std::wstring currentTracker_;

	std::pair<float,float> speed_;		
	float completion_;	
	float distributed_copies_;
	
	size_type total_wanted_done_;
	size_type total_wanted_;
	size_type total_uploaded_;
	size_type total_payload_uploaded_;
	size_type total_downloaded_;
	size_type total_payload_downloaded_;
	
	size_type peers_;
	size_type connected_peers_;
	size_type seeds_;
	size_type connected_seeds_;
	
	float ratio_;
	
	boost::posix_time::time_duration estimated_time_left_;
	boost::posix_time::time_duration update_tracker_in_;
	
	boost::posix_time::time_duration active_;
	boost::posix_time::time_duration seeding_;
	boost::posix_time::ptime start_time_;
	boost::posix_time::ptime finish_time_;

	int queue_position_;
	bool managed_;
	
private:
	mutable bool peer_details_filled_;
	mutable peer_details_vec peer_details_;
	
	mutable bool file_details_filled_;
	mutable file_details_vec file_details_;
};

typedef boost::shared_ptr<torrent_details> torrent_details_ptr;
typedef boost::scoped_ptr<torrent_details> torrent_details_sptr;
typedef boost::weak_ptr<torrent_details> torrent_details_wptr;
typedef std::vector<torrent_details_ptr> torrent_details_vec;
typedef std::map<std::wstring, torrent_details_ptr> torrent_details_map;

class torrent_details_manager
{
public:	
	void sort(size_t index, bool cmp_less = true) const;
	
	const torrent_details_vec torrents() const 
	{
		mutex_t::scoped_lock l(mutex_);	
		return torrents_; 
	}
	
	const torrent_details_vec selectedTorrents() const 
	{ 
		mutex_t::scoped_lock l(mutex_);	
		return selectedTorrents_; 
	}
	
	const torrent_details_ptr focusedTorrent() const 
	{
		mutex_t::scoped_lock l(mutex_);	
		return selectedTorrent_; 
	}
	
	const torrent_details_ptr get(std::wstring filename) const
	{
		mutex_t::scoped_lock l(mutex_);	
		
		torrent_details_map::const_iterator i = torrentMap_.find(filename);
		
		if (i != torrentMap_.end())
			return i->second;
		else
			return torrent_details_ptr();
	}
	
	friend class bit;

private:
	void clearAll(const mutex_t::scoped_lock&)
	{
		// !! No mutex lock, it should only be called from functions which 
		// have the lock themselves, hence the unused function param
		
		torrents_.clear();
		torrentMap_.clear();
		selectedTorrents_.clear();
		selectedTorrent_.reset();
	}

	mutable torrent_details_vec torrents_;
	
	torrent_details_map torrentMap_;
	torrent_details_vec selectedTorrents_;
	torrent_details_ptr selectedTorrent_;
	
	mutable mutex_t mutex_;
};

struct tracker_detail
{
	tracker_detail() {}
	tracker_detail(std::wstring u, int t) : url(u), tier(t) {}
	
	bool operator<(const tracker_detail& t) const
	{
		return (tier < t.tier);
	}
	
	std::wstring url;
	int tier;
};

typedef std::vector<tracker_detail> tracker_details_t;

struct web_seed_or_dht_node_detail
{
	web_seed_or_dht_node_detail();
	web_seed_or_dht_node_detail(std::wstring u);
	web_seed_or_dht_node_detail(std::wstring u, int p);
		
	std::wstring url;
	int port;
	std::wstring type;
};

typedef std::vector<pair<fs::wpath, size_type> > file_size_pairs_t;

struct dht_node_detail
{
	dht_node_detail() {}
	dht_node_detail(std::wstring u, int p) : url(u), port(p) {}
	
	std::wstring url;
	int port;
};

typedef std::vector<dht_node_detail> dht_node_details_t;

struct web_seed_detail
{
	web_seed_detail() {}
	web_seed_detail(std::wstring u) : url(u) {}
	
	std::wstring url;
};

typedef std::vector<web_seed_detail> web_seed_details_t;

struct create_torrent_params
{
	create_torrent_params() {}

	std::wstring creator;
	std::wstring comment;
	int piece_size;
	bool private_torrent;

	file_size_pairs_t file_size_pairs;
	fs::wpath root_path;

	tracker_details_t trackers;
	dht_node_details_t dht_nodes;
	web_seed_details_t web_seeds;
};

class EventDetail;

struct SessionDetail
{
	int port;
	
	std::pair<double, double> speed;
	
	bool dht_on;
	size_t dht_nodes;
	size_t dht_torrents;
	
	bool ip_filter_on;
	size_t ip_ranges_filtered;
};

typedef boost::function<bool (size_t, size_t, size_t)> filterCallback;
typedef boost::function<bool (size_t, std::wstring)> progress_callback;
typedef boost::function<void (int)> report_num_active;
typedef std::pair<wstring, wstring> wstring_pair;
typedef std::pair<float, float> float_pair;
typedef std::pair<int, int> int_pair;
typedef std::pair<std::vector<int>, int> vec_int_pair;

class bit_impl;
class torrent_internal;

class bit
{
public:		

	enum mappings
	{
		mappingNone = 0,
		mappingUPnP,
		mappingNatPMP
	};

	enum allocations
	{
		sparse_allocation = 1,
		compact_allocation,
		full_allocation
	};

	enum queue_adjustments
	{
		move_up = 0,
		move_down,
		move_to_top,
		move_to_bottom
	};

	class null_torrent : public std::exception
	{
	public:
		null_torrent() {}		
		virtual ~null_torrent() throw () {}
	};

	class torrent : public stlsoft::operator_bool_adaptor<torrent>
	{
		typedef torrent class_type;

	public:
		class exec_around_ptr
		{
		public:
			class proxy 
			{
			public:
				explicit proxy(torrent_internal* t);

				torrent_internal* operator->() 
				{
					return t_;
				}

				~proxy ();

			private:
				torrent_internal* t_;
				mutex_t::scoped_lock l_;
			};

			exec_around_ptr() {}
			exec_around_ptr(boost::shared_ptr<torrent_internal> p) : ptr(p) {}

			proxy operator->() const
			{
				if (!ptr)
					throw null_torrent();

				return proxy(&(*ptr));
			}

			operator bool() const { return ptr; }

		private:
			boost::shared_ptr<torrent_internal> ptr;
		};

		torrent();
		torrent(boost::shared_ptr<torrent_internal> p);

		const std::wstring get_name() const;

		float get_ratio() const;
		void set_ratio(float new_ratio);
		
		std::pair<int, int> get_connection_limits() const;
		void set_connection_limits(const std::pair<int, int>&);
		std::pair<float, float> get_rate_limits() const;
		void set_rate_limits(const std::pair<float, float>&);

		wpath get_save_directory() const;
		void set_save_directory(const wpath&);
		wpath get_move_to_directory() const;
		void set_move_to_directory(const wpath&);

		std::pair<wstring, wstring> get_tracker_login() const;
		void set_tracker_login(const std::pair<wstring, wstring>&);

		std::vector<tracker_detail> get_trackers() const;
		void set_trackers(const std::vector<tracker_detail>&);

		bool get_is_active() const;
		bool get_in_session() const;

		void set_file_priorities(const vec_int_pair&);

		void set_managed(bool);
		bool get_managed() const;

	public:
		STLSOFT_METHOD_PROPERTY_GET_EXTERNAL(const std::wstring, class_type, 
			get_name, name);

		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(float, float, class_type, 
			get_ratio, set_ratio, ratio);

		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(int_pair, const int_pair&, class_type, 
			get_connection_limits, set_connection_limits, connection_limits);
		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(float_pair, const float_pair&, class_type, 
			get_rate_limits, set_rate_limits, rate_limits);
		
		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(wpath, const wpath&, class_type, 
			get_save_directory, set_save_directory, save_directory);
		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(wpath, const wpath&, class_type, 
			get_move_to_directory, set_move_to_directory, move_to_directory);

		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(wstring_pair, const wstring_pair&, 
			class_type, get_tracker_login, set_tracker_login, tracker_login);

		STLSOFT_METHOD_PROPERTY_GET_EXTERNAL(bool, class_type, 
			get_is_active, is_active);
		STLSOFT_METHOD_PROPERTY_GET_EXTERNAL(bool, class_type, 
			get_in_session, in_session);

		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(std::vector<tracker_detail>, const std::vector<tracker_detail>&, 
			class_type, get_trackers, set_trackers, trackers);

		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(bool, bool, class_type, 
			get_managed, set_managed, managed);

		STLSOFT_METHOD_PROPERTY_SET_EXTERNAL(const vec_int_pair&, class_type, 
			set_file_priorities, file_priorities);

		void reset_trackers();
		bool is_open() const;
		void adjust_queue_position(bit::queue_adjustments adjust);

	private:
		exec_around_ptr ptr;
	};

	void shutdown_session();
	void save_torrent_data();

	bool create_torrent(const create_torrent_params& params, fs::wpath out_file, progress_callback fn);

	template<typename T>
	torrent get(T t)
	{
		return get_wstr(to_wstr_shim(t));
	}
	
	template<>
	torrent get(const hal::torrent_details_ptr t)
	{
		if (t) 
			return get_wstr(t->name());
		else
			return torrent();
	}	

	torrent get_wstr(const std::wstring& filename);
	
	bool listen_on(std::pair<int, int> const& portRange);
	int is_listening_on();
	void stop_listening();
	
	bool ensure_dht_on(const dht_settings& dht);
	void ensure_dht_off();
	
	void ensure_pe_on(const pe_settings& pe);
	void ensure_pe_off();
	
	bool ensure_ip_filter_on(progress_callback fn);
	void ensure_ip_filter_off();

	void set_resolve_countries(bool);
	void start_smart_ban_plugin();
	void start_ut_pex_plugin();
	void start_ut_metadata_plugin();
	void start_metadata_plugin();

	void set_mapping(bool upnp, bool nat_pmp);
	std::wstring upnp_router_model();

	void ip_v4_filter_block(boost::asio::ip::address_v4 first, boost::asio::ip::address_v4 last);
	void ip_v6_filter_block(boost::asio::ip::address_v6 first, boost::asio::ip::address_v6 last);
	bool ip_filter_import_dat(boost::filesystem::path file, progress_callback fn, bool octalFix);
	size_t ip_filter_size();
	void clear_ip_filter();	
	
	void set_session_half_open_limit(int halfConn);
	void set_session_limits(int maxConn, int maxUpload);
	void set_session_speed(float download, float upload);

	queue_settings get_queue_settings();
	void set_queue_settings(const queue_settings& s);
	
	timeouts get_timeouts();
	void set_timeouts(const timeouts& t);

	const cache_details get_cache_details() const;

	void set_cache_settings(const cache_settings& cache);
	cache_settings get_cache_settings() const;
	
	const SessionDetail get_session_details();

	void set_torrent_defaults(const connections& defaults);	
	void add_torrent(boost::filesystem::wpath file, boost::filesystem::wpath saveDirectory, 
		bool startPaused=false, bool managed=false, allocations alloc=hal::bit::sparse_allocation, 
		boost::filesystem::wpath moveToDirectory=L"", bool useMoveTo=false);
	
	void get_all_peer_details(const std::string& filename, peer_details_vec& peerContainer);
	void get_all_peer_details(const std::wstring& filename, peer_details_vec& peerContainer);
	void get_all_file_details(const std::string& filename, file_details_vec& file_details);
	void get_all_file_details(const std::wstring& filename, file_details_vec& file_details);
	
	void resume_all();
	void close_all(boost::optional<report_num_active> fn);
	
	bool is_torrent(const std::string& filename);
	bool is_torrent(const std::wstring& filename);	
	
	void pause_torrent(const std::string& filename);
	void pause_torrent(const std::wstring& filename);
	void resume_torrent(const std::string& filename);
	void resume_torrent(const std::wstring& filename);
	void stop_torrent(const std::string& filename);
	void stop_torrent(const std::wstring& filename);
	bool is_torrent_active(const std::string& filename);
	bool is_torrent_active(const std::wstring& filename);
	void reannounce_torrent(const std::string& filename);
	void reannounce_torrent(const std::wstring& filename);
	void recheck_torrent(const std::string& filename);
	void recheck_torrent(const std::wstring& filename);
	
	void pause_all_torrents();
	void unpause_all_torrents();

	template<typename S>
	void remove_torrent(S filename)
	{ 
		remove_torrent_wstr(to_wstr_shim(filename)); 
	}	

	template<typename S>
	void remove_torrent_wipe_files(S filename)
	{ 
		remove_torrent_wipe_files_wstr(to_wstr_shim(filename)); 
	}	

	void start_event_receiver();
	void stop_event_receiver();
	
	friend bit& bittorrent();
	
	int default_torrent_max_connections();
	int default_torrent_max_uploads();
	float default_torrent_download();
	float default_torrent_upload();	

	const torrent_details_manager& torrentDetails();
	const torrent_details_manager& updatetorrent_details_manager(const std::wstring& focused, const std::set<std::wstring>& selected);
	
private:
	bit();

	bit_impl* pimpl();
	const bit_impl* pimpl() const;
	boost::scoped_ptr<bit_impl> pimpl_;
	
	void remove_torrent_wstr(const std::wstring& filename);
	void remove_torrent_wipe_files_wstr(const std::wstring&  filename);
	
	torrent_details_manager torrentDetails_;
};

bit& bittorrent();

};
