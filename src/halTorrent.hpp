
#pragma once

#include <string>
#include <vector>

#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>  

using boost::filesystem::path;

namespace libtorrent { struct peer_info; }

namespace halite 
{

struct torrentBriefDetail 
{
	wstring filename;
	wstring status;
	pair<float,float> speed;
	float completion;
	int peers;
	int seeds;
};

class TorrentDetail 
{
public:
	TorrentDetail(std::wstring f, std::wstring s, std::wstring cT, std::pair<float,float> sp=std::pair<float,float>(0,0),
			float c=0, float a=0, boost::int64_t tWD=0, boost::int64_t tW=0, int p=0, int sd=0) :
		filename_(f),
		state_(s),
		currentTracker_(cT),
		speed_(sp),
		completion_(c),
		available_(a),
		totalWantedDone_(tWD),
		totalWanted_(tW),
		peers_(p),
		seeds_(sd)
	{}		

	TorrentDetail() {};
	
	const std::wstring& filename() const { return filename_; }
	const std::wstring& state() const { return state_; }
	const std::wstring& currentTracker() { return currentTracker_; }
	
	std::pair<float,float> speed() const { return speed_; }
	const float& completion() const { return completion_; }
	const float& available() const { return available_; }
	
	const boost::int64_t& totalWantedDone() const { return totalWantedDone_; }
	const boost::int64_t& totalWanted() const { return totalWanted_; }
	
	const int& peers() const { return peers_; }
	const int& seeds() const { return seeds_; }

public:
	std::wstring filename_;
	std::wstring state_;
	std::wstring currentTracker_;

	std::pair<float,float> speed_;		
	float completion_;	
	float available_;
	
	boost::int64_t totalWantedDone_;
	boost::int64_t totalWanted_;
	
	int peers_;
	int seeds_;
};

typedef shared_ptr<TorrentDetail> TorrentDetail_ptr;
typedef std::vector<TorrentDetail_ptr> TorrentDetails;

struct PeerDetail 
{
	PeerDetail(const wstring& ip_address) :
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
	pair<float,float> speed;
	bool seed;
	wstring client;
};

typedef shared_ptr<PeerDetail> PeerDetail_ptr;
typedef std::vector<PeerDetail> PeerDetails;

class BitTorrent_impl;

class BitTorrent
{
public:
	void shutDownSession();
	
	bool listenOn(pair<int, int> const& portRange);
	int isListeningOn();
	void stopListening();
	
	void setSessionLimits(int maxConn, int maxUpload);
	void setSessionSpeed(float download, float upload);
	pair<double, double> sessionSpeed();
	
	void newTorrent(boost::filesystem::path filename, boost::filesystem::path files);
	void addTorrent(boost::filesystem::path file);
	void getAllTorrentDetails(TorrentDetails& torrentsContainer);
	TorrentDetail_ptr getTorrentDetails(string filename);
	
	void getAllPeerDetails(string filename, PeerDetails& peerContainer);
	
	void resumeAll();
	void closeAll();
	
	bool isTorrent(string filename);
	
	void pauseTorrent(string filename);
	void resumeTorrent(string filename);
	bool isTorrentPaused(string filename);
	
	void pauseAllTorrents();
	void resumeAllTorrents();
	
	void removeTorrent(string filename);
	void reannounceTorrent(string filename);
	
	void setTorrentLimit(string filename, int maxConn, int maxUpload);
	void setTorrentSpeed(string filename, float download, float upload);
	pair<int, int> getTorrentLimit(string filename);
	pair<float, float> getTorrentSpeed(string filename);
	
	friend BitTorrent& bittorrent();
	
private:
	BitTorrent();
	
	boost::scoped_ptr<BitTorrent_impl> pimpl;
};

BitTorrent& bittorrent();

wstring mbstowcs(const string &str);
string wcstombs(const wstring &str);

};
