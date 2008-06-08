
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>
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
#include <boost/algorithm/string.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <stlsoft/properties/method_properties.hpp>
#include <stlsoft/util/operator_bool_adaptor.hpp>


//#if BOOST_VERSION < 103500
//#include <asio/ip/tcp.hpp>
//#include <asio/ip/udp.hpp>
//#else
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
//#endif

#include "halTypes.hpp"

namespace libtorrent { struct peer_info; }

namespace hal 
{
	
struct torrentBriefDetail 
{
	std::wstring filename;
	std::wstring status;
	std::pair<float,float> speed;
	float completion;
	int peers;
	int seeds;
};

struct PeerDetail 
{
	PeerDetail(const std::wstring& ip_address) :
		ipAddress(ip_address)
	{}
	PeerDetail(libtorrent::peer_info& peerInfo);
	
	bool operator==(const PeerDetail& peer) const
	{
		return (ipAddress == peer.ipAddress);
	}
	
	bool operator<(const PeerDetail& peer) const
	{
		return (ipAddress < peer.ipAddress);
	}
	
	std::wstring ipAddress;
	std::wstring country;
	std::pair<float,float> speed;
	bool seed;
	std::wstring client;
	std::wstring status;
};

typedef boost::shared_ptr<PeerDetail> PeerDetail_ptr;
typedef std::vector<PeerDetail> PeerDetails;

struct FileDetail
{
	FileDetail(boost::filesystem::wpath p, boost::int64_t s=0, float pg=0, int pr=1, size_t o=0, unsigned t=FileDetail::file) :
		branch(p.branch_path()),
		filename(p.leaf()),
		type(t),
		size(s),
		progress(pg),
		priority(pr),
		order_(o)
	{}
	
	bool operator==(const FileDetail& file) const
	{
		return (branch == file.branch);
	}
	
	bool operator<(const FileDetail& file) const
	{
		return (branch < file.branch);
	}
	
	enum FileType
	{
		folder,
		file
	};
	
	size_t order() const { return order_; }
	
	boost::filesystem::wpath branch;
	std::wstring filename;
	unsigned type;
	boost::int64_t size;
	float progress;
	int priority;
	
private:
	size_t order_;
};

inline bool FileDetailNamesEqual(const FileDetail& l, const FileDetail& r)
{
	return l.filename == r.filename;
}

inline bool FileDetailNamesLess(const FileDetail& l, const FileDetail& r)
{
	return l.filename < r.filename;
}

typedef std::vector<FileDetail> FileDetails;

class TorrentDetail 
{
public:
	TorrentDetail(std::wstring n, std::wstring f, std::wstring sd, std::wstring s, std::wstring cT, std::pair<float,float> sp=std::pair<float,float>(0,0),
			float c=0, float d=0, size_type tWD=0, size_type tW=0, size_type tU=0, size_type tpU=0, size_type tD=0, size_type tpD=0, boost::tuple<size_type, size_type, size_type, size_type> connections = boost::tuple<size_type, size_type, size_type, size_type>(0,0,0,0), float r=0, 
			boost::posix_time::time_duration eta=boost::posix_time::seconds(0), boost::posix_time::time_duration uIn=boost::posix_time::seconds(0),
			boost::posix_time::time_duration actve=boost::posix_time::seconds(0), boost::posix_time::time_duration seding=boost::posix_time::seconds(0), boost::posix_time::ptime srt=boost::posix_time::second_clock::universal_time(), boost::posix_time::ptime fin=boost::posix_time::second_clock::universal_time()) :
		filename_(f),
		name_(n),
		saveDir_(sd),
		state_(s),
		currentTracker_(cT),
		speed_(sp),
		completion_(c),
		distributed_copies_(d),
		totalWantedDone_(tWD),
		totalWanted_(tW),
		totalUploaded_(tU),
		totalPayloadUploaded_(tpU),
		totalDownloaded_(tD),
		totalPayloadDownloaded_(tpD),
		peers_(connections.get<0>()),
		connectedPeers_(connections.get<1>()),
		seeds_(connections.get<2>()),
		connectedSeeds_(connections.get<3>()),
		ratio_(r),
		estimatedTimeLeft_(eta),
		updateTrackerIn_(uIn),
		peerDetailsFilled_(false),
		fileDetailsFilled_(false),
		active_(actve),
		seeding_(seding),
		startTime_(srt),
		finishTime_(fin)
	{}

	TorrentDetail() :	
		peerDetailsFilled_(false),
		fileDetailsFilled_(false)
	{};	
	
	enum state
	{
		torrent_active = 0,
		torrent_paused,
		torrent_stopped,
		torrent_pausing,
		torrent_stopping
	};
	
//	const std::wstring& filename() const { return filename_; }
	const std::wstring& name() const { return name_; }
	const std::wstring& saveDirectory() const { return saveDir_; }
	const std::wstring& state() const { return state_; }
	const std::wstring& currentTracker() const { return currentTracker_; }
	
	std::pair<float,float> speed() const { return speed_; }
	const float& completion() const { return completion_; }
	const float& distributedCopies() const { return distributed_copies_; }
	
	size_type totalUploaded() const { return totalUploaded_; }
	size_type totalPayloadUploaded() const { return totalPayloadUploaded_; }
	size_type totalDownloaded() const { return totalDownloaded_; }
	size_type totalPayloadDownloaded() const { return totalPayloadDownloaded_; }
	size_type totalWantedDone() const { return totalWantedDone_; }
	size_type totalWanted() const { return totalWanted_; }
	
	size_type peers() const { return peers_; }
	size_type peersConnected() const { return connectedPeers_; }
	size_type seeds() const { return seeds_; }
	size_type seedsConnected() const { return connectedSeeds_; }
	
	float ratio() { return ratio_; }
	
	const boost::posix_time::time_duration& estimatedTimeLeft() { return estimatedTimeLeft_; }
	const boost::posix_time::time_duration& updateTrackerIn() { return updateTrackerIn_; }
	
	const PeerDetails& peerDetails() const;
	const FileDetails& fileDetails() const;
	
	const boost::posix_time::time_duration& active() { return active_; }
	const boost::posix_time::time_duration& seeding() { return seeding_; }
	const boost::posix_time::ptime& startTime() { return startTime_; }
	const boost::posix_time::ptime& finishTime() { return finishTime_; }
	
public:
	std::wstring filename_;
	std::wstring name_;
	std::wstring saveDir_;
	std::wstring state_;
	std::wstring currentTracker_;

	std::pair<float,float> speed_;		
	float completion_;	
	float distributed_copies_;
	
	size_type totalWantedDone_;
	size_type totalWanted_;
	size_type totalUploaded_;
	size_type totalPayloadUploaded_;
	size_type totalDownloaded_;
	size_type totalPayloadDownloaded_;
	
	size_type peers_;
	size_type connectedPeers_;
	size_type seeds_;
	size_type connectedSeeds_;
	
	float ratio_;
	
	boost::posix_time::time_duration estimatedTimeLeft_;
	boost::posix_time::time_duration updateTrackerIn_;
	
	boost::posix_time::time_duration active_;
	boost::posix_time::time_duration seeding_;
	boost::posix_time::ptime startTime_;
	boost::posix_time::ptime finishTime_;
	
private:
	mutable bool peerDetailsFilled_;
	mutable PeerDetails peerDetails_;
	
	mutable bool fileDetailsFilled_;
	mutable FileDetails fileDetails_;
};

typedef boost::shared_ptr<TorrentDetail> TorrentDetail_ptr;
typedef boost::scoped_ptr<TorrentDetail> TorrentDetail_sptr;
typedef boost::weak_ptr<TorrentDetail> TorrentDetail_wptr;
typedef std::vector<TorrentDetail_ptr> TorrentDetail_vec;
typedef std::map<std::wstring, TorrentDetail_ptr> TorrentDetail_map;

class TorrentDetails
{
public:	
	void sort(boost::function<bool (const TorrentDetail_ptr&, const TorrentDetail_ptr&)> fn) const;
	
	const TorrentDetail_vec torrents() const 
	{
		mutex_t::scoped_lock l(mutex_);	
		return torrents_; 
	}
	
	const TorrentDetail_vec selectedTorrents() const 
	{ 
		mutex_t::scoped_lock l(mutex_);	
		return selectedTorrents_; 
	}
	
	const TorrentDetail_ptr focusedTorrent() const 
	{
		mutex_t::scoped_lock l(mutex_);	
		return selectedTorrent_; 
	}
	
	const TorrentDetail_ptr get(std::wstring filename) const
	{
		mutex_t::scoped_lock l(mutex_);	
		
		TorrentDetail_map::const_iterator i = torrentMap_.find(filename);
		
		if (i != torrentMap_.end())
			return i->second;
		else
			return TorrentDetail_ptr();
	}
	
	friend class bit;

private:
	void clearAll(const mutex_t::scoped_lock&)
	{
		// !! No mutex lock, it should only be call from functions which 
		// have the lock themselves, hence the unused function param
		
		torrents_.clear();
		torrentMap_.clear();
		selectedTorrents_.clear();
		selectedTorrent_.reset();
	}

	mutable TorrentDetail_vec torrents_;
	
	TorrentDetail_map torrentMap_;
	TorrentDetail_vec selectedTorrents_;
	TorrentDetail_ptr selectedTorrent_;
	
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

		STLSOFT_METHOD_PROPERTY_SET_EXTERNAL(const vec_int_pair&, class_type, 
			set_file_priorities, file_priorities);

		void reset_trackers();

		bool is_open() const;


	private:
		exec_around_ptr ptr;
	};

	enum mappings
	{
		mappingNone = 0,
		mappingUPnP,
		mappingNatPMP
	};

	void shutDownSession();
	void save_torrent_data();

	bool create_torrent(const create_torrent_params& params, fs::wpath out_file, progress_callback fn);

	template<typename T>
	torrent get(T t)
	{
		return get_wstr(to_wstr_shim(t));
	}
	
	template<>
	torrent get(const hal::TorrentDetail_ptr t)
	{
		if (t) 
			return get_wstr(t->name());
		else
			return torrent();
	}	
	
	bool listen_on(std::pair<int, int> const& portRange);
	int is_listening_on();
	void stop_listening();
	
	bool ensure_dht_on();
	void ensure_dht_off();
	
	void ensure_pe_on(int enc_level, int in_enc_policy, int out_enc_policy, bool prefer_rc4);
	void ensure_pe_off();
	
	bool ensure_ip_filter_on(progress_callback fn);
	void ensure_ip_filter_off();

	void set_mapping(int mapping);

	void ip_v4_filter_block(boost::asio::ip::address_v4 first, boost::asio::ip::address_v4 last);
	void ip_v6_filter_block(boost::asio::ip::address_v6 first, boost::asio::ip::address_v6 last);
	bool ip_filter_import_dat(boost::filesystem::path file, progress_callback fn, bool octalFix);
	size_t ip_filter_size();
	void clear_ip_filter();	
	
	void setSessionHalfOpenLimit(int halfConn);
	void set_session_limits(int maxConn, int maxUpload);
	void set_session_speed(float download, float upload);
	void set_dht_settings(int max_peers_reply, int search_branching, 
		int service_port, int max_fail_count);
	void set_timeouts(int peers, int tracker);
	
	const SessionDetail getSessionDetails();

	void setTorrentDefaults(int maxConn, int maxUpload, float download, float upload);	
	void add_torrent(boost::filesystem::wpath file, boost::filesystem::wpath saveDirectory, 
		bool startPaused=false, bool compactStorage=false, 
		boost::filesystem::wpath moveToDirectory=L"", bool useMoveTo=false);
	
	void getAllPeerDetails(const std::string& filename, PeerDetails& peerContainer);
	void getAllPeerDetails(const std::wstring& filename, PeerDetails& peerContainer);
	void getAllFileDetails(const std::string& filename, FileDetails& fileDetails);
	void getAllFileDetails(const std::wstring& filename, FileDetails& fileDetails);
	
	void resume_all();
	void close_all(boost::optional<report_num_active> fn);
	
	bool isTorrent(const std::string& filename);
	bool isTorrent(const std::wstring& filename);	
	
	void pauseTorrent(const std::string& filename);
	void pauseTorrent(const std::wstring& filename);
	void resumeTorrent(const std::string& filename);
	void resumeTorrent(const std::wstring& filename);
	void stopTorrent(const std::string& filename);
	void stopTorrent(const std::wstring& filename);
	bool isTorrentActive(const std::string& filename);
	bool isTorrentActive(const std::wstring& filename);
	void reannounceTorrent(const std::string& filename);
	void reannounceTorrent(const std::wstring& filename);
	void recheckTorrent(const std::string& filename);
	void recheckTorrent(const std::wstring& filename);
	
	void pauseAllTorrents();
	void unpauseAllTorrents();

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

	void startEventReceiver();
	void stopEventReceiver();
	
	friend bit& bittorrent();
	
	int defTorrentMaxConn();
	int defTorrentMaxUpload();
	float defTorrentDownload();
	float defTorrentUpload();	

	const TorrentDetails& torrentDetails();
	const TorrentDetails& updateTorrentDetails(const std::wstring& focused, const std::set<std::wstring>& selected);
	
private:
	bit();
	boost::scoped_ptr<bit_impl> pimpl;

	torrent get_wstr(const std::wstring& filename);
	
	void remove_torrent_wstr(const std::wstring& filename);
	void remove_torrent_wipe_files_wstr(const std::wstring&  filename);
	
	TorrentDetails torrentDetails_;
};

bit& bittorrent();

};
