
#pragma once

#include <string>
#include <vector>

#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>  

using boost::filesystem::path;

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

struct PeerDetail {
	PeerDetail(wstring ip_address, pair<float,float> Speed, bool Seed)	:
		ipAddress(ip_address),
		speed(Speed),
		seed(Seed)
	{}
	
	bool operator==(wstring ip_address)
	{
		return (ipAddress == ip_address);
	}
	
	wstring ipAddress;
	pair<float,float> speed;
	bool seed;
};

typedef shared_ptr<std::vector<torrentBriefDetail> > torrentBriefDetails;
typedef shared_ptr<TorrentDetail> TorrentDetail_ptr;
typedef std::vector<TorrentDetail_ptr> TorrentDetails;

class BitTorrent_impl;

class BitTorrent
{
public:
	void shutDownSession();
	
	bool listenOn(std::pair<int, int> const& portRange);
	int isListeningOn();
	void stopListening();
	
	void setSessionSpeed(pair<double, double> kilobyteRate);	
	pair<double, double> sessionSpeed();
	
	void addTorrent(boost::filesystem::path file);
	TorrentDetails getAllTorrentDetails();
	TorrentDetail_ptr getTorrentDetails(std::string filename);
	void resumeAll();
	void closeAll();
	void pauseTorrent(std::string filename);
	void resumeTorrent(std::string filename);
	bool isTorrentPaused(std::string filename);
	
	friend BitTorrent& bittorrent();
	
private:
	BitTorrent();
	
	boost::scoped_ptr<BitTorrent_impl> pimpl;
};

BitTorrent& bittorrent();

wstring mbstowcs(const string &str);
string wcstombs(const wstring &str);

/*	bool initSession();
bool listenOn(pair<int,int>);
int isListeningOn();
bool closeDown();
void resumeAll();

pair<float,float> sessionSpeed();
void addTorrent(path file);
void reannounceAll();
torrentBriefDetails getTorrents();
torrentDetails getTorrentDetails(wstring filename);

void pauseTorrent(wstring filename);
void resumeTorrent(wstring filename);
bool isPausedTorrent(wstring filename);
void reannounceTorrent(wstring filename);
void removeTorrent(wstring filename);

void pauseTorrents();
void resumeTorrents();

void setTorrentTransferLimits(wstring filename, float down, float up);
pair<float,float> getTorrentTransferLimits(wstring filename);
void setTorrentConnectionLimits(wstring filename, int, int);
pair<int,int> getTorrentConnectionLimits(wstring filename);

void getTorrentPeerDetails(wstring filename, vector<PeerDetail>& peerDetails);

void closeTorrents();
void setLimits(int download, int uploads);
*/

};
