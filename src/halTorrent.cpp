
#include "stdAfx.hpp"

#include <boost/array.hpp>

#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/peer_connection.hpp>

#include "halTorrent.hpp"

namespace halite 
{
	using namespace std;
	using namespace boost;
	using namespace libtorrent;
	using boost::filesystem::ifstream;
	using boost::filesystem::ofstream;
	
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
	
	wstring mbstowcs(const string &str) 
	{
		size_t len=::mbstowcs(NULL, str.c_str(), str.length());
		boost::scoped_array<wchar_t> buf(new wchar_t[len]);
	
		len=::mbstowcs(buf.get(), str.c_str(), str.length());
		if(len==static_cast<size_t>(-1)) 
			throw runtime_error("mbstowcs(): invalid multi-byte character");
	
		return wstring(buf.get(), len);
	}

	string wcstombs(const wstring &str) 
	{
		size_t len=::wcstombs(NULL, str.c_str(), 0);
		boost::scoped_array<char> buf(new char[len]);
	
		len=::wcstombs(buf.get(), str.c_str(), len);
		if(len==static_cast<size_t>(-1)) 
			throw runtime_error("wcstombs(): unable to convert character");
	
		return string(buf.get(), len);
	}
	
	BitTorrent& bittorrent()
	{
		static BitTorrent t;
		return t;
	}
	
	class TorrentInternal
	{
	public:
		TorrentInternal(pair<float, float> tL, int c, int u, bool p, 
				wstring f, torrent_handle h) :
			transferLimit_(tL),
			connections_(c),
			uploads_(u),
			paused_(p),
			filename_(f),
			handle_(h)
		{}
		
		void setTransferLimit(float down, float up)
		{
			transferLimit_ = pair<float, float>(down, up);
			
			if (down > 0) 
				handle_.set_download_limit(static_cast<int>(down*1024));
			else
				handle_.set_download_limit(-1);
				
			if (up > 0)
				handle_.set_upload_limit(static_cast<int>(up*1024));
			else
				handle_.set_upload_limit(-1);
		}
		
		torrentDetail* getTorrentDetails() const
		{
			torrent_status tS = handle_.status();
			
			wstring state;
			switch (tS.state)
			{
				case torrent_status::queued_for_checking:
					state = L"Queued For Checking";
					break;
				case torrent_status::checking_files:
					state = L"Checking Files";
					break;
				case torrent_status::connecting_to_tracker:
					state = L"Connecting To Tracker";
					break;
				case torrent_status::downloading_metadata:
					state = L"Downloading Metadata";
					break;
				case torrent_status::downloading:
					state = L"Downloading";
					break;
				case torrent_status::finished:
					state = L"Finished";
					break;
				case torrent_status::seeding:
					state = L"Seeding";
					break;
				case torrent_status::allocating:
					state = L"Allocating";
					break;
			}	
			
			return new torrentDetail(filename_, state, mbstowcs(tS.current_tracker), 
				pair<float, float>(tS.download_payload_rate, tS.upload_payload_rate),
				tS.progress, tS.distributed_copies, tS.total_wanted_done, tS.total_wanted,
				tS.num_peers, tS.num_seeds);
		}
		
		const torrent_handle& handle() const { return handle_; } 
		
	private:		
		pair<float, float> transferLimit_;
		
		int connections_;
		int uploads_;
		bool paused_;
		
		wstring filename_;
		torrent_handle handle_;	
	};
	
	typedef map<wstring, TorrentInternal> TorrentMap;

	
	class BitTorrent::BitTorrent_impl
	{
		friend class BitTorrent;
		
	private:
		BitTorrent_impl() :
			theSession(fingerprint("HL", 0, 2, 0, 0))
		{
			array<char, MAX_PATH> pathBuffer;
			GetCurrentDirectoryA(MAX_PATH, pathBuffer.c_array());
			workingDirectory = path(pathBuffer.data(), native);
		}
		
		session theSession;
		path workingDirectory;
		TorrentMap torrents;
	};
	
	BitTorrent::BitTorrent() :
		pimpl(new BitTorrent_impl())
	{}
	
	bool BitTorrent::listenOn(pair<int, int> const& range)
	{
		if (!pimpl->theSession.is_listening())
		{
			return pimpl->theSession.listen_on(range);	
		}
		else
		{
			int port = pimpl->theSession.listen_port();
			
			if (port < range.first || port > range.second)
				return pimpl->theSession.listen_on(range);	
			else
				return true;
		}
	}
	
	int BitTorrent::isListeningOn() 
	{
		if (!pimpl->theSession.is_listening())
			return 0;	
		else
			return pimpl->theSession.listen_port();
	}
	
	void BitTorrent::stopListening()
	{
		pimpl->theSession.listen_on(make_pair(0, 0));
	}
	
	pair<float, float> BitTorrent::sessionSpeed() 
	{
		session_status sStatus = pimpl->theSession.status();		
		return pair<float,float>(sStatus.download_rate, sStatus.upload_rate);
	}
	
	void BitTorrent::addTorrent(path file) 
	{
		try 
		{
		
		const path resumeFile = pimpl->workingDirectory/"resume"/file.leaf();
		const wstring filename = mbstowcs(file.leaf());

		entry metadata = haldecode(file);
		entry resumedata;
		
		if (exists(resumeFile)) 
		{
			try 
			{
				resumedata = haldecode(resumeFile);
			}
			catch(exception &ex) 
			{			
				::MessageBoxW(0, mbstowcs(ex.what()).c_str(), L"Exception", MB_ICONERROR|MB_OK);
				remove(resumeFile);
			}
		}

		if (!exists(pimpl->workingDirectory/"torrents"))
			create_directory(pimpl->workingDirectory/"torrents");

		if (!exists(pimpl->workingDirectory/"torrents"/file.leaf()))
			copy_file(file, pimpl->workingDirectory/"torrents"/file.leaf());

		if (!exists(pimpl->workingDirectory/"incoming"))
			create_directory(pimpl->workingDirectory/"incoming");
		
		TorrentMap::const_iterator existing = pimpl->torrents.find(filename);
		
		if (existing == pimpl->torrents.end())
		{
			torrent_handle handle = pimpl->theSession.add_torrent(metadata,
				pimpl->workingDirectory/"incoming", resumedata);
			
			pimpl->torrents.insert(TorrentMap::value_type(filename,
				TorrentInternal(pair<float, float>(0, 0), 0, 0, false, filename, handle)));
		}

		}
		catch(exception &ex) 
		{
			wstring caption=L"Error";
			
			MessageBox(0, mbstowcs(ex.what()).c_str(), caption.c_str(), MB_ICONERROR|MB_OK);
		}
	}
	
	vecTorrentDetails BitTorrent::getAllTorrentDetails()
	{
		vecTorrentDetails vTorrents;
		
		for (TorrentMap::const_iterator iter = pimpl->torrents.begin(); 
			iter != pimpl->torrents.end(); ++iter)
		{
			vTorrents.push_back(torrentDetails(iter->second.getTorrentDetails()));
		}
		
		return vTorrents;
	}

	
//	bool Torrent::closeDown() 
//	{
	//	delete session;
	//	session = NULL;
//		return true;
//	}
	
	struct Torrent_ 
	{
		wstring file;
		
		pair<float,float> transferLimit;
		
		int connections;
		int uploads;
		
		torrent_handle handle;
	};	
	
	static libtorrent::session* session = NULL;
	static map<wstring,Torrent_> torrents;
	static path workingDirectory;
	static bool pauseAll = false;
	
	typedef map<wstring,Torrent_>::iterator torrentIter;
	


	bool initSession()
	{
		try
		{
			LPSTR pathBuffer = static_cast<LPSTR>(malloc(1024));
			GetCurrentDirectoryA(1024,pathBuffer);
			workingDirectory = path(pathBuffer,native);
			free(static_cast<void*>(pathBuffer));
				
			session = new libtorrent::session();
			
			return true;
		}
		catch(...)
		{
			return false;
		}
	}
	
	bool listenOn(pair<int,int> range) 
	{
		if (!session->is_listening())
		{
			return session->listen_on(range);	
		}
		else
		{
			int port = session->listen_port();
			if (port > range.second || port < range.first)
			{
				return session->listen_on(range);	
			}
			else
			{
				return true;
			}
		}
	}
		
	int isListeningOn() 
	{
		if (!session->is_listening())
		{
			return -1;	
		}
		else
		{
			return session->listen_port();	
		}
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
//					addTorrent(*itr);
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
			if (existing != torrents.end())
			{
				existing->second.connections = connections;
				existing->second.uploads = uploads;
				
				if (connections > 0) 
					existing->second.handle.set_max_connections(connections);
				else
					existing->second.handle.set_max_connections(200);
					
				if (uploads > 0)
					existing->second.handle.set_max_uploads(uploads);
				else
					existing->second.handle.set_max_uploads(200);
			}
		}				
	}
	
	pair<int,int> getTorrentConnectionLimits(wstring filename)
	{
		if (filename != L"") {
			torrentIter existing = torrents.find(filename);		
			if (existing != torrents.end())
			{
				return pair<int,int>(existing->second.connections,existing->second.uploads);
			}
		}	
		return pair<int,int>(0,0);			
	}	
	
	void getTorrentPeerDetails(wstring filename, vector<PeerDetail>& peerDetails)
	{
		if (filename != L"") {
			torrentIter existing = torrents.find(filename);		
			if (existing != torrents.end())
			{
				peerDetails.clear();
				std::vector<peer_info> peer_infos_;
				existing->second.handle.get_peer_info(peer_infos_);
				
				for(vector<peer_info>::iterator i=peer_infos_.begin(); i != peer_infos_.end(); ++i)
				{
					peerDetails.push_back(PeerDetail(
						mbstowcs((*i).ip.address().to_string()),
						std::make_pair((*i).payload_down_speed, (*i).payload_up_speed),
						(*i).seed)
						);
				}
			}
		}	
		return;			
	}
	
	void setTorrentTransferLimits(wstring filename, float down, float up)
	{
		if (filename != L"") {
			torrentIter existing = torrents.find(filename);		
			if (existing != torrents.end())
			{
				existing->second.transferLimit = pair<float,float>(down,up);
				
				if (down > 0) 
					existing->second.handle.set_download_limit(static_cast<int>(down*1024));
				else
					existing->second.handle.set_download_limit(-1);
					
				if (up > 0)
					existing->second.handle.set_upload_limit(static_cast<int>(up*1024));
				else
					existing->second.handle.set_upload_limit(-1);
			}
		}				
	}
	
	pair<float,float> getTorrentTransferLimits(wstring filename)
	{
		if (filename != L"") {
			torrentIter existing = torrents.find(filename);		
			if (existing != torrents.end())
			{
				return existing->second.transferLimit;
			}
		}	
		return pair<float, float>(0, 0);			
	}
	/*
	torrentDetails getTorrentDetails(wstring filename)
	{
		torrentIter existing = torrents.find(filename);
		torrentDetails pTD;
		
		if (existing != torrents.end())
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

			pTD->completion = ts.progress;
			pTD->currentTracker = mbstowcs(ts.current_tracker);
			pTD->available = ts.distributed_copies;
			pTD->totalWantedDone = ts.total_wanted_done;
			pTD->totalWanted = ts.total_wanted;
			
		}		
		return pTD;
	}
*/	
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
			if (existing != torrents.end())
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
			if (existing != torrents.end())
			{
				existing->second.handle.force_reannounce();
			}
		}			
	}
	
	void pauseTorrent(wstring filename)
	{
		if (filename != L"") {
			torrentIter existing = torrents.find(filename);		
			if (existing != torrents.end())
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
			if (existing != torrents.end())
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
			if (existing != torrents.end())
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
				::MessageBoxW(0, mbstowcs(ex.what()).c_str(), L"Exception", MB_ICONERROR|MB_OK);
				remove(rfile);
			}
		}

		if(!exists(workingDirectory/"torrents"))
			create_directory(workingDirectory/"torrents");

		if(!exists(workingDirectory/"torrents"/file.leaf()))
			copy_file(file, workingDirectory/"torrents"/file.leaf());

		if(!exists(workingDirectory/"incoming"))
			create_directory(workingDirectory/"incoming");

		Torrent_ t;
		
		t.file = filename;
		t.handle = session->add_torrent(metadata, workingDirectory/"incoming", resumedata);
		
		t.transferLimit = pair<float,float>(0,0);		
		t.connections = 0;
		t.uploads = 0;

//		const torrent_info& info = t.handle.get_torrent_info();
		torrentIter existing = torrents.find(filename);
		
		if (existing == torrents.end())
			torrents.insert(pair<wstring,Torrent_>(filename,t));
			
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
