
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

class TorrentDetail 
{
public:
	TorrentDetail(std::wstring f, std::wstring s, std::wstring cT, std::pair<float,float> sp=std::pair<float,float>(0,0),
			float c=0, float d=0, boost::int64_t tWD=0, boost::int64_t tW=0, boost::int64_t tU=0, boost::int64_t tpU=0, boost::int64_t tD=0, boost::int64_t tpD=0, int prs=0, int prsCnt=0, int sds=0, int sdsCnt=0,  float r=0, 
			time_duration eta=boost::posix_time::seconds(0), time_duration uIn=boost::posix_time::seconds(0)) :
		filename_(f),
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
		peerDetailsFilled_(false)
	{}

	TorrentDetail() :	
		peerDetailsFilled_(false)
	{};	
	
	enum state
	{
		torrent_active = 0,
		torrent_paused,
		torrent_stopped
	};
	
	const std::wstring& filename() const { return filename_; }
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
	
public:
	std::wstring filename_;
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
	
private:
	mutable bool peerDetailsFilled_;
	mutable PeerDetails peerDetails_;
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
	
	bool ensure_dht_on();
	void ensure_dht_off();
	
	void ensure_ip_filter_on(progressCallback fn);
	void ensure_ip_filter_off();
	void ip_v4_filter_block(asio::ip::address_v4 first, asio::ip::address_v4 last);
	void ip_v6_filter_block(asio::ip::address_v6 first, asio::ip::address_v6 last);
	void ip_filter_import_dat(boost::filesystem::path file, progressCallback fn, bool octalFix);
	size_t ip_filter_size();
	void clearIpFilter();
	
	void ensure_pe_on(int enc_level, int in_enc_policy, int out_enc_policy, bool prefer_rc4);
	void ensure_pe_off();
	
	void setSessionHalfOpenLimit(int halfConn);
	void setSessionLimits(int maxConn, int maxUpload);
	void setSessionSpeed(float download, float upload);
	void setDhtSettings(int max_peers_reply, int search_branching, 
		int service_port, int max_fail_count);
	
	const SessionDetail getSessionDetails();

	void setTorrentDefaults(int maxConn, int maxUpload, float download, float upload);	
	void newTorrent(boost::filesystem::wpath filename, boost::filesystem::wpath files);
	void addTorrent(boost::filesystem::wpath file, wpath saveDirectory);
	void getAllTorrentDetail_vec(TorrentDetail_vec& torrentsContainer);
	TorrentDetail_ptr getTorrentDetail_vec(std::string filename);	
	
	void setTorrentRatio(std::string, float ratio);
	float getTorrentRatio(std::string);
	
	void getAllPeerDetails(std::string filename, PeerDetails& peerContainer);
	
	void resumeAll();
	void closeAll();
	
	bool isTorrent(std::string filename);
	
	void pauseTorrent(std::string filename);
	void resumeTorrent(std::string filename);
	void stopTorrent(std::string filename);
	bool isTorrentActive(std::string filename);
	
	void pauseAllTorrents();
	void unpauseAllTorrents();
	
	void removeTorrent(std::string filename);
	void removeTorrentWipeFiles(std::string filename);
	void reannounceTorrent(std::string filename);
	
	void setTorrentLogin(std::string filename, std::wstring username, std::wstring password);
	std::pair<std::wstring, std::wstring> getTorrentLogin(std::string filename);
	
	void setTorrentLimit(std::string filename, int maxConn, int maxUpload);
	void setTorrentSpeed(std::string filename, float download, float upload);
	pair<int, int> getTorrentLimit(std::string filename);
	pair<float, float> getTorrentSpeed(std::string filename);
	
	void setTorrentTrackers(std::string filename, const std::vector<TrackerDetail>& trackers);
	void resetTorrentTrackers(std::string filename);
	std::vector<TrackerDetail> getTorrentTrackers(std::string filename);

	void startEventReceiver();
	void stopEventReceiver();
	
	friend BitTorrent& bittorrent();
	
	int defTorrentMaxConn();
	int defTorrentMaxUpload();
	float defTorrentDownload();
	float defTorrentUpload();	

	const TorrentDetails& torrentDetails();
//	const TorrentDetails& getTorrentDetails(std::string selected, std::set<std::string> allSelected);	
	const TorrentDetails& updateTorrentDetails(std::string focused, std::set<std::string> selected);
	
private:
	BitTorrent();
	boost::scoped_ptr<BitTorrent_impl> pimpl;
	
	TorrentDetails torrentDetails_;
};

BitTorrent& bittorrent();

};
