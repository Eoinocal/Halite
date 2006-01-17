
#include "halTorrent.h"

#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/peer_connection.hpp>


namespace halite 
{
	using namespace libtorrent;
	using boost::filesystem::ifstream;
	using boost::filesystem::ofstream;
	
	struct Torrent 
	{
		wstring file;
		
		pair<float,float> transferLimit;
		
		int connections;
		int uploads;
		
		torrent_handle handle;
	};	
	
	static libtorrent::session* session = NULL;
	static map<wstring,Torrent> torrents;
	static path workingDirectory;
	static bool pauseAll = false;
	
	typedef map<wstring,Torrent>::iterator torrentIter;
	
	entry haldecode(const path &file) 
	{
		ifstream fs(file, ifstream::binary);
		if(fs.is_open()) 
		{
			fs.unsetf(ifstream::skipws);
			return bdecode(istream_iterator<char>(fs), istream_iterator<char>());
		}
		else return entry();
	}
	
	bool halencode(const path &file, const entry &e) 
	{
		ofstream fs(file, ofstream::binary);
		if(!fs.is_open()) 
			return false;
			
		bencode(ostream_iterator<char>(fs), e);
		return true;
	}
	
	wstring mbstowcs(const string &str) {
		size_t len=::mbstowcs(NULL, str.c_str(), str.length());
		boost::scoped_array<wchar_t> buf(new wchar_t[len]);
	
		len=::mbstowcs(buf.get(), str.c_str(), str.length());
		if(len==static_cast<size_t>(-1)) throw runtime_error("mbstowcs(): invalid multi-byte character");
	
		return wstring(buf.get(), len);
	}

	string wcstombs(const wstring &str) {
		size_t len=::wcstombs(NULL, str.c_str(), 0);
		boost::scoped_array<char> buf(new char[len]);
	
		len=::wcstombs(buf.get(), str.c_str(), len);
		if(len==static_cast<size_t>(-1)) throw runtime_error("wcstombs(): unable to convert character");
	
		return string(buf.get(), len);
	}

	bool listenOn(pair<int,int> range) 
	{
		LPSTR pathBuffer = static_cast<LPSTR>(malloc(1024));
		GetCurrentDirectoryA(1024,pathBuffer);
		workingDirectory = path(pathBuffer,native);
		free(static_cast<void*>(pathBuffer));
			
		session = new libtorrent::session();
		return session->listen_on(range);
		
		
	}
	
	bool closeDown() 
	{
		delete session;
		session = NULL;
		return true;
	}
	
	void resumeAll()
	{
		path resume = workingDirectory/"torrents";
		if (!exists(resume)) 
			return;
			
		directory_iterator end_itr;			
		for (directory_iterator itr(resume); itr != end_itr; ++itr ) 
		{
			if (!is_directory(*itr)) 
			{
				if(iends_with(itr->leaf(),".torrent")) 
				{
					addTorrent(*itr);
				}
			}
		}
	}
	
	void setLimits(int downloads, int uploads)
	{
		if (session)
		{
			session->set_max_uploads(uploads);
			session->set_max_connections(downloads);
		}
	}
	
	pair<float,float> sessionSpeed() 
	{
		if (session)
		{	
			session_status sStatus = session->status();		
			return pair<float,float>(sStatus.download_rate, sStatus.upload_rate);
		} 
		else
		{
			return pair<float,float>(0,0);
		}
	}
	
	torrentBriefDetails getTorrents() {
		
		shared_ptr<vector<torrentBriefDetail> > spTbd;
		
		if (!torrents.empty()) 
		{
			spTbd.reset(new vector<torrentBriefDetail>);
			for(torrentIter i=torrents.begin(); i!=torrents.end(); ++i)
			{
				torrentBriefDetail tbd;
				torrent_status ts = i->second.handle.status();
				
				switch (ts.state)
				{
					case torrent_status::state_t::queued_for_checking:
						tbd.status = L"Queued For Checking";
						break;
					case torrent_status::state_t::checking_files:
						tbd.status = L"Checking Files";
						break;
					case torrent_status::state_t::connecting_to_tracker:
						tbd.status = L"Connecting To Tracker";
						break;
					case torrent_status::state_t::downloading_metadata:
						tbd.status = L"Downloading Metadata";
						break;
					case torrent_status::state_t::downloading:
						tbd.status = L"Downloading";
						break;
					case torrent_status::state_t::finished:
						tbd.status = L"Finished";
						break;
					case torrent_status::state_t::seeding:
						tbd.status = L"Seeding";
						break;
					case torrent_status::state_t::allocating:
						tbd.status = L"Allocating";
						break;
				}
				tbd.filename = i->second.file;
				
				if (i->second.handle.is_paused())
					tbd.status = L"Paused";
				
				tbd.speed = pair<float,float>(ts.download_rate,ts.upload_rate);
				tbd.completion = ts.progress;
				tbd.seeds = ts.num_seeds;
				tbd.peers = ts.num_peers;
				
				spTbd->push_back(tbd);
			}
		}
		return spTbd;
	}	
	
	void setTorrentConnectionLimits(wstring filename, int connections, int uploads)
	{
		if (filename != L"") {
			torrentIter existing = torrents.find(filename);		
			if (existing != torrents.end());
			{
				existing->second.connections = connections;
				existing->second.uploads = uploads;
				
				if (connections > 0) 
					existing->second.handle.set_max_connections(connections);
				if (uploads > 0)
					existing->second.handle.set_max_uploads(uploads);
			}
		}				
	}
	
	pair<int,int> getTorrentConnectionLimits(wstring filename)
	{
		if (filename != L"") {
			torrentIter existing = torrents.find(filename);		
			if (existing != torrents.end());
			{
				return pair<int,int>(existing->second.connections,existing->second.uploads);
			}
		}	
		return pair<int,int>(0,0);			
	}
	
	void setTorrentTransferLimits(wstring filename, float down, float up)
	{
		if (filename != L"") {
			torrentIter existing = torrents.find(filename);		
			if (existing != torrents.end());
			{
				existing->second.transferLimit = pair<float,float>(down,up);
				
				if (down > 0) 
					existing->second.handle.set_download_limit(static_cast<int>(down*1024));
				if (up > 0)
					existing->second.handle.set_upload_limit(static_cast<int>(up*1024));
			}
		}				
	}
	
	pair<float,float> getTorrentTransferLimits(wstring filename)
	{
		if (filename != L"") {
			torrentIter existing = torrents.find(filename);		
			if (existing != torrents.end());
			{
				return existing->second.transferLimit;
			}
		}	
		return pair<float,float>(0,0);			
	}
	
	torrentDetails getTorrentDetails(wstring filename)
	{
		torrentIter existing = torrents.find(filename);
		torrentDetails pTD;
		
		if (existing != torrents.end());
		{	
			pTD.reset(new torrentDetail);
			torrent_status ts = existing->second.handle.status();
			
			switch (ts.state)
			{
				case torrent_status::state_t::queued_for_checking:
					pTD->status = L"Queued For Checking";
					break;
				case torrent_status::state_t::checking_files:
					pTD->status = L"Checking Files";
					break;
				case torrent_status::state_t::connecting_to_tracker:
					pTD->status = L"Connecting To Tracker";
					break;
				case torrent_status::state_t::downloading_metadata:
					pTD->status = L"Downloading Metadata";
					break;
				case torrent_status::state_t::downloading:
					pTD->status = L"Downloading";
					break;
				case torrent_status::state_t::finished:
					pTD->status = L"Finished";
					break;
				case torrent_status::state_t::seeding:
					pTD->status = L"Seeding";
					break;
				case torrent_status::state_t::allocating:
					pTD->status = L"Allocating";
					break;
			}	
			
			if (existing->second.handle.is_paused())
				pTD->status = L"Paused";

			pTD->current_tracker = mbstowcs(ts.current_tracker);
			pTD->available = ts.distributed_copies;
			
		}		
		return pTD;
	}
	
	void reannounceAll() 
	{		
		if (!torrents.empty()) 
		{
			for(torrentIter i=torrents.begin(); i!=torrents.end(); ++i)
			{	
				i->second.handle.force_reannounce();
			}
		}
	}
	
	void removeTorrent(wstring filename)
	{
		if (filename != L"") {
			torrentIter existing = torrents.find(filename);		
			if (existing != torrents.end());
			{
				session->remove_torrent(existing->second.handle);
				torrents.erase(existing);
				
				if(exists(workingDirectory/"torrents"/wcstombs(filename)))
					remove(workingDirectory/"torrents"/wcstombs(filename));
			}
		}			
	}	
	
	void reannounceTorrent(wstring filename)
	{
		if (filename != L"") {
			torrentIter existing = torrents.find(filename);		
			if (existing != torrents.end());
			{
				existing->second.handle.force_reannounce();
			}
		}			
	}
	
	void pauseTorrent(wstring filename)
	{
		if (filename != L"") {
			torrentIter existing = torrents.find(filename);		
			if (existing != torrents.end());
			{
				existing->second.handle.pause();
			}
		}			
	}	
	
	void pauseTorrents() 
	{			
		for(torrentIter i=torrents.begin(); i!=torrents.end(); ++i)
		{
			i->second.handle.pause();
		}
		pauseAll = true;
	}	
	
	void resumeTorrents() 
	{			
		for(torrentIter i=torrents.begin(); i!=torrents.end(); ++i)
		{
			i->second.handle.resume();
		}
		pauseAll = false;
	}
		
	bool isPausedTorrent(wstring filename)
	{
		if (filename != L"") {
			torrentIter existing = torrents.find(filename);		
			if (existing != torrents.end());
			{
				return existing->second.handle.is_paused();
			}
		}	
		return true;		
	}	
	
	void resumeTorrent(wstring filename)
	{
		if (filename != L"") {
			torrentIter existing = torrents.find(filename);		
			if (existing != torrents.end());
			{
				existing->second.handle.resume();
			}
		}			
	}
	
	void addTorrent(path file) {
	try {
		const path rfile=workingDirectory/"resume"/file.leaf();
		const wstring filename = mbstowcs(file.leaf());

		entry metadata = haldecode(file);
		entry resumedata;
		
		if(exists(rfile)) {
			try {
				resumedata = haldecode(rfile);
			}
			catch(exception &ex) 
			{			
				MessageBox(0, mbstowcs(ex.what()).c_str(), L"Exception", MB_ICONERROR|MB_OK);
				remove(rfile);
			}
		}

		if(!exists(workingDirectory/"torrents"))
			create_directory(workingDirectory/"torrents");

		if(!exists(workingDirectory/"torrents"/file.leaf()))
			copy_file(file, workingDirectory/"torrents"/file.leaf());

		if(!exists(workingDirectory/"incoming"))
			create_directory(workingDirectory/"incoming");

		Torrent t;
		
		t.file = filename;
		t.handle = session->add_torrent(metadata, workingDirectory/"incoming", resumedata);
		
		t.transferLimit = pair<float,float>(0,0);		
		t.connections = 0;
		t.uploads = 0;

//		const torrent_info& info = t.handle.get_torrent_info();
		torrentIter existing = torrents.find(filename);
		
		if (existing == torrents.end());
			torrents.insert(pair<wstring,Torrent>(filename,t));
			
		if(pauseAll) t.handle.pause();
	}
		catch(exception &ex) 
		{
			wstring caption=L"Error";//loadstring(IDS_EXCEPTION);
			
			MessageBox(0, mbstowcs(ex.what()).c_str(), caption.c_str(), MB_ICONERROR|MB_OK);
		}
	}
	
	void closeTorrents() 
	{
		path p=workingDirectory/"resume";
		
		if(!boost::filesystem::exists(p))
			boost::filesystem::create_directory(p);
			
		for(torrentIter i=torrents.begin(); i!=torrents.end(); ++i)
		{
			i->second.handle.pause();
			entry e = i->second.handle.write_resume_data();
			
			halencode(p/wcstombs(i->second.file), e);
		}
		torrents.erase(torrents.begin(), torrents.end());
	}
};
