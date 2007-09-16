
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>
#include <vector>
#include <set>

#include <boost/signal.hpp>
#include <boost/optional.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <asio/ip/tcp.hpp>
#include <asio/ip/udp.hpp>

using boost::filesystem::path;
using boost::posix_time::time_duration;
using boost::posix_time::ptime;

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
	
	wstring ipAddress;
	wstring country;
	std::pair<float,float> speed;
	bool seed;
	std::wstring client;
	std::wstring status;
};

typedef shared_ptr<PeerDetail> PeerDetail_ptr;
typedef std::vector<PeerDetail> PeerDetails;

struct FileDetail
{
	FileDetail(boost::filesystem::wpath p, size_t s=0, float pg=0, int pr=1, size_t o=0) :
		branch(p.branch_path()),
		filename(p.leaf()),
		size(s),
		progress(pg),
		priority(pr),
		order_(o)
	{}
	
	FileDetail(boost::filesystem::wpath b, wstring f, size_t s=0, float pg=0, int pr=1, size_t o=0) :
		branch(b),
		filename(f),
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
	
	size_t order() { return order_; }
	
	boost::filesystem::wpath branch;
	wstring filename;
	size_t size;
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
	TorrentDetail(std::wstring f, std::wstring n, std::wstring s, std::wstring cT, std::pair<float,float> sp=std::pair<float,float>(0,0),
			float c=0, float d=0, boost::int64_t tWD=0, boost::int64_t tW=0, boost::int64_t tU=0, boost::int64_t tpU=0, boost::int64_t tD=0, boost::int64_t tpD=0, int prs=0, int prsCnt=0, int sds=0, int sdsCnt=0,  float r=0, 
			time_duration eta=boost::posix_time::seconds(0), time_duration uIn=boost::posix_time::seconds(0),
			time_duration actve=boost::posix_time::seconds(0), time_duration seding=boost::posix_time::seconds(0), ptime srt=boost::posix_time::second_clock::universal_time(), ptime fin=boost::posix_time::second_clock::universal_time()) :
		filename_(f),
		name_(n),
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
		peers_(prs),
		connectedPeers_(prsCnt),
		seeds_(sds),
		connectedSeeds_(sdsCnt),
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
	
	const std::wstring& filename() const { return filename_; }
	const std::wstring& name() const { return name_; }
	const std::wstring& state() const { return state_; }
	const std::wstring& currentTracker() const { return currentTracker_; }
	
	std::pair<float,float> speed() const { return speed_; }
	const float& completion() const { return completion_; }
	const float& distributedCopies() const { return distributed_copies_; }
	
	const boost::int64_t& totalUploaded() const { return totalUploaded_; }
	const boost::int64_t& totalPayloadUploaded() const { return totalPayloadUploaded_; }
	const boost::int64_t& totalDownloaded() const { return totalDownloaded_; }
	const boost::int64_t& totalPayloadDownloaded() const { return totalPayloadDownloaded_; }
	const boost::int64_t& totalWantedDone() const { return totalWantedDone_; }
	const boost::int64_t& totalWanted() const { return totalWanted_; }
	
	int peers() const { return peers_; }
	int peersConnected() const { return connectedPeers_; }
	int seeds() const { return seeds_; }
	int seedsConnected() const { return connectedSeeds_; }
	
	float ratio() { return ratio_; }
	
	const time_duration& estimatedTimeLeft() { return estimatedTimeLeft_; }
	const time_duration& updateTrackerIn() { return updateTrackerIn_; }
	
	const PeerDetails& peerDetails() const;
	const FileDetails& fileDetails() const;
	
	const time_duration& active() { return active_; }
	const time_duration& seeding() { return seeding_; }
	const ptime& startTime() { return startTime_; }
	const ptime& finishTime() { return finishTime_; }
	
public:
	std::wstring filename_;
	std::wstring name_;
	std::wstring state_;
	std::wstring currentTracker_;

	std::pair<float,float> speed_;		
	float completion_;	
	float distributed_copies_;
	
	boost::int64_t totalWantedDone_;
	boost::int64_t totalWanted_;
	boost::int64_t totalUploaded_;
	boost::int64_t totalPayloadUploaded_;
	boost::int64_t totalDownloaded_;
	boost::int64_t totalPayloadDownloaded_;
	
	int peers_;
	int connectedPeers_;
	int seeds_;
	int connectedSeeds_;
	
	float ratio_;
	
	time_duration estimatedTimeLeft_;
	time_duration updateTrackerIn_;
	
	time_duration active_;
	time_duration seeding_;
	ptime startTime_;
	ptime finishTime_;
	
private:
	mutable bool peerDetailsFilled_;
	mutable PeerDetails peerDetails_;
	
	mutable bool fileDetailsFilled_;
	mutable FileDetails fileDetails_;
};

typedef shared_ptr<TorrentDetail> TorrentDetail_ptr;
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
	
	const TorrentDetail_ptr selectedTorrent() const { return focusedTorrent(); }
	
	const TorrentDetail_ptr get(std::wstring filename) const
	{
		mutex_t::scoped_lock l(mutex_);	
		
		TorrentDetail_map::const_iterator i = torrentMap_.find(filename);
		
		if (i != torrentMap_.end())
			return i->second;
		else
			return TorrentDetail_ptr();
	}
	
	friend class BitTorrent;

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

struct TrackerDetail
{
	TrackerDetail() {}
	TrackerDetail(std::wstring u, int t) : url(u), tier(t) {}
	
	std::wstring url;
	int tier;
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
typedef boost::function<bool (size_t)> progressCallback;

class BitTorrent_impl;

class BitTorrent
{
public:	
	void shutDownSession();
	void saveTorrentData();
	
	bool listenOn(std::pair<int, int> const& portRange);
	int isListeningOn();
	void stopListening();
	
	bool ensureDhtOn();
	void ensureDhtOff();
	
	void ensurePeOn(int enc_level, int in_enc_policy, int out_enc_policy, bool prefer_rc4);
	void ensurePeOff();
	
	void ensureIpFilterOn(progressCallback fn);
	void ensureIpFilterOff();
	
	void ip_v4_filter_block(asio::ip::address_v4 first, asio::ip::address_v4 last);
	void ip_v6_filter_block(asio::ip::address_v6 first, asio::ip::address_v6 last);
	void ip_filter_import_dat(boost::filesystem::path file, progressCallback fn, bool octalFix);
	size_t ip_filter_size();
	void clearIpFilter();
	
	
	void setSessionHalfOpenLimit(int halfConn);
	void setSessionLimits(int maxConn, int maxUpload);
	void setSessionSpeed(float download, float upload);
	void setDhtSettings(int max_peers_reply, int search_branching, 
		int service_port, int max_fail_count);
	
	const SessionDetail getSessionDetails();

	void setTorrentDefaults(int maxConn, int maxUpload, float download, float upload);	
	void newTorrent(boost::filesystem::wpath filename, boost::filesystem::wpath files);
	void addTorrent(boost::filesystem::wpath file, wpath saveDirectory, 
		bool startPaused=false, bool compactStorage=false);
	
	void setTorrentRatio(const std::string&, float ratio);
	void setTorrentRatio(const std::wstring&, float ratio);
	float getTorrentRatio(const std::string&);
	float getTorrentRatio(const std::wstring&);
	
	void getAllPeerDetails(const std::string& filename, PeerDetails& peerContainer);
	void getAllPeerDetails(const std::wstring& filename, PeerDetails& peerContainer);
	void getAllFileDetails(const std::string& filename, FileDetails& fileDetails);
	void getAllFileDetails(const std::wstring& filename, FileDetails& fileDetails);
	
	void resumeAll();
	void closeAll();
	
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
	
	void pauseAllTorrents();
	void unpauseAllTorrents();
	
	void removeTorrent(const std::string& filename);
	void removeTorrent(const std::wstring&  filename);
	void removeTorrentWipeFiles(const std::string& filename);
	void removeTorrentWipeFiles(const std::wstring&  filename);
	
	void setTorrentLogin(const std::string& filename, std::wstring username, std::wstring password);
	void setTorrentLogin(const std::wstring& filename, std::wstring username, std::wstring password);
	std::pair<std::wstring, std::wstring> getTorrentLogin(const std::string& filename);
	std::pair<std::wstring, std::wstring> getTorrentLogin(const std::wstring&  filename);
	
	void setTorrentLimit(const std::string& filename, int maxConn, int maxUpload);
	void setTorrentLimit(const std::wstring& filename, int maxConn, int maxUpload);
	void setTorrentSpeed(const std::string& filename, float download, float upload);
	void setTorrentSpeed(const std::wstring& filename, float download, float upload);
	pair<int, int> getTorrentLimit(const std::string& filename);
	pair<int, int> getTorrentLimit(const std::wstring& filename);
	pair<float, float> getTorrentSpeed(const std::string& filename);
	pair<float, float> getTorrentSpeed(const std::wstring& filename);
	
	void setTorrentTrackers(const std::string& filename, const std::vector<TrackerDetail>& trackers);
	void setTorrentTrackers(const std::wstring& filename, const std::vector<TrackerDetail>& trackers);
	void resetTorrentTrackers(const std::string& filename);
	void resetTorrentTrackers(const std::wstring& filename);
	std::vector<TrackerDetail> getTorrentTrackers(const std::string& filename);
	std::vector<TrackerDetail> getTorrentTrackers(const std::wstring& filename);
	
	void setTorrentFilePriorities(const std::string& filename, std::vector<int> fileIndices, int priority);
	void setTorrentFilePriorities(const std::wstring& filename, std::vector<int> fileIndices, int priority);

	void startEventReceiver();
	void stopEventReceiver();
	
	friend BitTorrent& bittorrent();
	
	int defTorrentMaxConn();
	int defTorrentMaxUpload();
	float defTorrentDownload();
	float defTorrentUpload();	

	const TorrentDetails& torrentDetails();
	const TorrentDetails& updateTorrentDetails(const std::wstring& focused, const std::set<std::wstring>& selected);
	
private:
	BitTorrent();
	boost::scoped_ptr<BitTorrent_impl> pimpl;
	
	TorrentDetails torrentDetails_;
};

BitTorrent& bittorrent();

};
