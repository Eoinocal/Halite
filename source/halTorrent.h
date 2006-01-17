
#include "stdafx.h"

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
	};
	
	typedef shared_ptr<vector<torrentBriefDetail> > torrentBriefDetails;
	typedef shared_ptr<torrentDetail> torrentDetails;
	
	wstring mbstowcs(const string &str);
	string wcstombs(const wstring &str);
	
	bool listenOn(pair<int,int>);
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
	
	void closeTorrents();
	void setLimits(int download, int uploads);
};
