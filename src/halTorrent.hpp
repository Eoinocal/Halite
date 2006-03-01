
#include "stdAfx.hpp"

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>  

using namespace std;
using namespace boost;
using namespace boost::filesystem;

namespace halite {

	struct torrentBriefDetail {
		wstring filename;
		wstring status;
		pair<float,float> speed;
		float completion;
		int peers;
		int seeds;
	};
	
	struct torrentDetail {
		wstring filename;
		wstring status;
		wstring current_tracker;
		float completion;	
		float available;
		size_t total_wanted_done;
		size_t total_wanted;
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
	
	typedef shared_ptr<vector<torrentBriefDetail> > torrentBriefDetails;
	typedef shared_ptr<torrentDetail> torrentDetails;
	
	wstring mbstowcs(const string &str);
	string wcstombs(const wstring &str);
	
	bool initSession();
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
};
