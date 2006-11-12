
#include "stdAfx.hpp"

#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <boost/array.hpp>

#include <libtorrent/file.hpp>
#include "libtorrent/hasher.hpp"
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/peer_connection.hpp>

#include "halTorrent.hpp"
#include "GlobalIni.hpp"
#include "ini/Torrent.hpp"

namespace halite 
{

namespace lbt = libtorrent;
namespace fs = boost::filesystem;

using fs::path;
using fs::ifstream;
using fs::ofstream;

lbt::entry haldecode(const path &file) 
{
	ifstream fs(file, ifstream::binary);
	if (fs.is_open()) 
	{
		fs.unsetf(ifstream::skipws);
		return lbt::bdecode(std::istream_iterator<char>(fs), std::istream_iterator<char>());
	}
	else return lbt::entry();
}

bool halencode(const path &file, const lbt::entry &e) 
{
	ofstream fs(file, ofstream::binary);
	if (!fs.is_open()) 
		return false;
	
	lbt::bencode(std::ostream_iterator<char>(fs), e);
	return true;
}

wstring mbstowcs(const string &str) 
{
	size_t len=::mbstowcs(NULL, str.c_str(), str.length());
	boost::scoped_array<wchar_t> buf(new wchar_t[len]);

	len=::mbstowcs(buf.get(), str.c_str(), str.length());
	if(len==static_cast<size_t>(-1)) 
		throw std::runtime_error("mbstowcs(): invalid multi-byte character");

	return wstring(buf.get(), len);
}

string wcstombs(const wstring &str) 
{
	size_t len=::wcstombs(NULL, str.c_str(), 0);
	boost::scoped_array<char> buf(new char[len]);

	len=::wcstombs(buf.get(), str.c_str(), len);
	if(len==static_cast<size_t>(-1)) 
		throw std::runtime_error("wcstombs(): unable to convert character");

	return string(buf.get(), len);
}

BitTorrent& bittorrent()
{
	static BitTorrent t;
	return t;
}

void TorrentInternal::setConnectionLimit(int maxConn, int maxUpload)
{
	handle_.set_max_connections(maxConn);
	handle_.set_max_uploads(maxUpload);

	connections_ = 	maxConn;
	uploads_ = maxUpload;
}

void TorrentInternal::setConnectionLimit()
{
	handle_.set_max_connections(connections_);
	handle_.set_max_uploads(uploads_);
}

pair<int, int> TorrentInternal::getConnectionLimit()
{
	return make_pair(connections_, uploads_);
}

void TorrentInternal::setTransferSpeed(float download, float upload)
{
	int down = (download > 0) ? static_cast<int>(download*1024) : -1;
	handle_.set_download_limit(down);
	int up = (upload > 0) ? static_cast<int>(upload*1024) : -1;
	handle_.set_upload_limit(up);
	
	transferLimit_ = make_pair(download, upload);
}

void TorrentInternal::setTransferSpeed()
{
	int down = (transferLimit_.first > 0) ? static_cast<int>(transferLimit_.first*1024) : -1;
	handle_.set_download_limit(down);
	int up = (transferLimit_.second > 0) ? static_cast<int>(transferLimit_.second*1024) : -1;
	handle_.set_upload_limit(up);
}

pair<float, float> TorrentInternal::getTransferSpeed()
{
	return transferLimit_;
}

TorrentDetail_ptr TorrentInternal::getTorrentDetails() const
{
	if (inSession())
	{
		lbt::torrent_status tS = handle_.status();
		wstring state;
		
		if (paused_)
			state = L"Paused";
		else
		{
			switch (tS.state)
			{
			case lbt::torrent_status::queued_for_checking:
				state = L"Queued For Checking";
				break;
			case lbt::torrent_status::checking_files:
				state = L"Checking Files";
				break;
			case lbt::torrent_status::connecting_to_tracker:
				state = L"Connecting To Tracker";
				break;
			case lbt::torrent_status::downloading_metadata:
				state = L"Downloading Metadata";
				break;
			case lbt::torrent_status::downloading:
				state = L"Downloading";
				break;
			case lbt::torrent_status::finished:
				state = L"Finished";
				break;
			case lbt::torrent_status::seeding:
				state = L"Seeding";
				break;
			case lbt::torrent_status::allocating:
				state = L"Allocating";
				break;
			}	
		}
			
		return TorrentDetail_ptr(new TorrentDetail(filename_, state, mbstowcs(tS.current_tracker), 
			pair<float, float>(tS.download_payload_rate, tS.upload_payload_rate),
			tS.progress, tS.distributed_copies, tS.total_wanted_done, tS.total_wanted,
			tS.num_peers, tS.num_seeds));
	}
	else
	{
		return TorrentDetail_ptr(new TorrentDetail(filename_, L"Not in Session", L"No tracker"));
	}
}

void TorrentInternal::pause()
{
	if (inSession()) handle_.pause();
	paused_ = true;
}

void TorrentInternal::resume()
{
	if (inSession()) handle_.resume();
	paused_ = false;
}

bool TorrentInternal::isPaused() const
{
	return paused_;
}

class BitTorrent_impl
{
	friend class BitTorrent;
	
private:
	BitTorrent_impl() :
		theSession(lbt::fingerprint("HL", 0, 3, 0, 0)),
		torrents(INI().torrentConfig().torrents)
	{
		boost::array<char, MAX_PATH> pathBuffer;
		GetCurrentDirectoryA(MAX_PATH, pathBuffer.c_array());
		workingDirectory = path(pathBuffer.data(), fs::native);
	}
	
	lbt::entry prepTorrent(path filename);
	lbt::torrent_handle addTorrent(lbt::entry metadata, lbt::entry resumedata);
	
	lbt::session theSession;
	path workingDirectory;
	TorrentMap& torrents;
};

BitTorrent::BitTorrent() :
	pimpl(new BitTorrent_impl())
{}

void BitTorrent::shutDownSession()
{
	pimpl.reset();
}

bool BitTorrent::listenOn(pair<int, int> const& range)
{
	if (!pimpl->theSession.is_listening())
	{
//		pimpl->theSession.start_dht();
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
		return -1;	
	else
		return pimpl->theSession.listen_port();
}

void BitTorrent::stopListening()
{
//	pimpl->theSession.stop_dht();
	pimpl->theSession.listen_on(make_pair(0, 0));
}

void BitTorrent::setSessionLimits(int maxConn, int maxUpload)
{		
	pimpl->theSession.set_max_uploads(maxUpload);
	pimpl->theSession.set_max_connections(maxConn);
}

void BitTorrent::setSessionSpeed(float download, float upload)
{
	int down = (download > 0) ? static_cast<int>(download*1024) : -1;
	pimpl->theSession.set_download_rate_limit(down);
	int up = (upload > 0) ? static_cast<int>(upload*1024) : -1;
	pimpl->theSession.set_upload_rate_limit(up);
}

pair<double, double> BitTorrent::sessionSpeed() 
{
	lbt::session_status sStatus = pimpl->theSession.status();		
	return pair<double, double>(sStatus.download_rate, sStatus.upload_rate);
}

lbt::entry BitTorrent_impl::prepTorrent(path filename)
{
	lbt::entry resumeData;	
	const path resumeFile = workingDirectory/"resume"/filename.leaf();
	
	if (exists(resumeFile)) 
	{
		try 
		{
			resumeData = haldecode(resumeFile);
		}
		catch(std::exception &ex) 
		{			
			::MessageBoxW(0, mbstowcs(ex.what()).c_str(), L"Resume Exception", MB_ICONERROR|MB_OK);
			remove(resumeFile);
		}
	}

	if (!exists(workingDirectory/"torrents"))
		create_directory(workingDirectory/"torrents");

	if (!exists(workingDirectory/"torrents"/filename.leaf()))
		copy_file(filename, workingDirectory/"torrents"/filename.leaf());

	if (!exists(workingDirectory/"incoming"))
		create_directory(workingDirectory/"incoming");
	
	return resumeData;
}

lbt::torrent_handle BitTorrent_impl::addTorrent(lbt::entry metadata, lbt::entry resumedata)
{
	return theSession.add_torrent(metadata, workingDirectory/"incoming", resumedata);
}

void BitTorrent::addTorrent(path file) 
{
	try 
	{	
	lbt::entry metadata = haldecode(file);
	lbt::entry resumedata = pimpl->prepTorrent(file);
	
	TorrentMap::const_iterator existing = pimpl->torrents.find(file.leaf());
	
	if (existing == pimpl->torrents.end())
	{		
		lbt::torrent_handle handle = pimpl->theSession.add_torrent(metadata,
			pimpl->workingDirectory/"incoming", resumedata);
		
		pimpl->torrents.insert(TorrentMap::value_type(file.leaf(), 
			TorrentInternal(handle, mbstowcs(file.leaf()))));
	}

	}
	catch(std::exception &ex) 
	{
		wstring caption=L"Add Torrent Exception";
		
		MessageBox(0, mbstowcs(ex.what()).c_str(), caption.c_str(), MB_ICONERROR|MB_OK);
	}
}

void add_files(lbt::torrent_info& t, fs::path const& p, fs::path const& l)
{
	fs::path f(p / l);
	if (fs::is_directory(f))
	{
		for (fs::directory_iterator i(f), end; i != end; ++i)
			add_files(t, p, l / i->leaf());
	}
	else
	{
	//	std::cerr << "adding \"" << l.string() << "\"\n";
		lbt::file fi(f, lbt::file::in);
		fi.seek(0, lbt::file::end);
		libtorrent::size_type size = fi.tell();
		t.add_file(l, size);
	}
}

void BitTorrent::newTorrent(fs::path filename, fs::path files)
{
	try
	{
	
	libtorrent::torrent_info t;
	path full_path = pimpl->workingDirectory/"incoming"/files.leaf();
	
	ofstream out(filename, std::ios_base::binary);
	
	int piece_size = 256 * 1024;
	char const* creator_str = "Halite v0.3 (libtorrent v0.11)";

	add_files(t, full_path.branch_path(), full_path.leaf());
	t.set_piece_size(piece_size);

	lbt::storage st(t, full_path.branch_path());
//	t.add_tracker("http://www.nitcom.com.au/announce.php");
	t.set_priv(false);
	t.add_node(make_pair("127.0.0.1", 6882));

	// calculate the hash for all pieces
	int num = t.num_pieces();
	std::vector<char> buf(piece_size);
	for (int i = 0; i < num; ++i)
	{
			st.read(&buf[0], i, 0, t.piece_size(i));
			libtorrent::hasher h(&buf[0], t.piece_size(i));
			t.set_hash(i, h.final());
		//	std::cerr << (i+1) << "/" << num << "\r";
	}

	t.set_creator(creator_str);

	// create the torrent and print it to out
	lbt::entry e = t.create_torrent();
	lbt::bencode(std::ostream_iterator<char>(out), e);
	}
	catch (std::exception& e)
	{
		::MessageBoxA(0, e.what(), "Create Torrent exception.", 0);
	}
}

void BitTorrent::getAllTorrentDetails(TorrentDetails& torrentsContainer)
{
	for (TorrentMap::const_iterator iter = pimpl->torrents.begin(); 
		iter != pimpl->torrents.end(); ++iter)
	{
		torrentsContainer.push_back(iter->second.getTorrentDetails());
	}
}

TorrentDetail_ptr BitTorrent::getTorrentDetails(string filename)
{
	TorrentMap::const_iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		return i->second.getTorrentDetails();
	}
	
	return TorrentDetail_ptr();
}

void BitTorrent::resumeAll()
{
	for (TorrentMap::iterator iter = pimpl->torrents.begin(); 
		iter != pimpl->torrents.end(); /*Nothing here*/)
	{
		path file = pimpl->workingDirectory/"torrents"/(*iter).first;
		
		if (exists(file))
		{		
			try 
			{	
			lbt::entry metadata = haldecode(file);
			lbt::entry resumedata = pimpl->prepTorrent(file);
			
			(*iter).second.setHandle(pimpl->theSession.add_torrent(metadata,
				pimpl->workingDirectory/"incoming", resumedata));
			
			if ((*iter).second.isPaused())
				(*iter).second.pause();
			
			(*iter).second.setTransferSpeed();
			(*iter).second.setConnectionLimit();
			
			++iter;
			}
			catch(std::exception &ex) 
			{
				MessageBox(0, mbstowcs(ex.what()).c_str(), L"Resume Torrent Exception", MB_ICONERROR|MB_OK);
				
				pimpl->torrents.erase(iter++);
			}
			
		}
		else
		{
			pimpl->torrents.erase(iter++);
		}
	}
}

void BitTorrent::closeAll()
{
	path resumeDir=pimpl->workingDirectory/"resume";
	
	if(!exists(resumeDir))
		create_directory(resumeDir);
	
	for (TorrentMap::iterator iter = pimpl->torrents.begin(); 
		iter != pimpl->torrents.end(); ++iter)
	{
		lbt::entry resumedata = (*iter).second.handle().write_resume_data();
		pimpl->theSession.remove_torrent((*iter).second.handle());
		
		halencode(resumeDir/(*iter).first, resumedata);
	}
}

PeerDetail::PeerDetail(lbt::peer_info& peerInfo) :
	ipAddress(mbstowcs(peerInfo.ip.address().to_string())),
	speed(make_pair(peerInfo.payload_down_speed, peerInfo.payload_up_speed)),
	seed(peerInfo.seed),
	client(mbstowcs(peerInfo.client))
{}

void BitTorrent::getAllPeerDetails(string filename, PeerDetails& peerContainer)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		std::vector<lbt::peer_info> peerInfo;
		(*i).second.handle().get_peer_info(peerInfo);
		
		for (std::vector<lbt::peer_info>::iterator j = peerInfo.begin(); 
			j != peerInfo.end(); ++j)
		{
			peerContainer.push_back(PeerDetail(*j));
		}		
	}
}

void BitTorrent::pauseTorrent(string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.pause();
	}
}

void BitTorrent::resumeTorrent(string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.resume();
	}
}

bool BitTorrent::isTorrentPaused(string filename)
{
	TorrentMap::const_iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		return (*i).second.isPaused();
	}
	
	return false; // ??? is this correct
}

void BitTorrent::removeTorrent(string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		pimpl->theSession.remove_torrent((*i).second.handle());
		pimpl->torrents.erase(i);
	}
}

void BitTorrent::reannounceTorrent(string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.handle().force_reannounce();
	}
}

void BitTorrent::pauseAllTorrents()
{	
	for (TorrentMap::iterator iter = pimpl->torrents.begin(); 
		iter != pimpl->torrents.end(); ++iter)
	{
		(*iter).second.pause();
	}
}

void BitTorrent::resumeAllTorrents()
{	
	for (TorrentMap::iterator iter = pimpl->torrents.begin(); 
		iter != pimpl->torrents.end(); ++iter)
	{
		(*iter).second.resume();
	}
}

void BitTorrent::setTorrentLimit(string filename, int maxConn, int maxUpload)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.setConnectionLimit(maxConn, maxUpload);
	}
}

void BitTorrent::setTorrentSpeed(string filename, float download, float upload)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.setTransferSpeed(download, upload);
	}
}

pair<int, int> BitTorrent::getTorrentLimit(string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		return (*i).second.getConnectionLimit();
	}
	return pair<int, int>(0, 0);
}

pair<float, float> BitTorrent::getTorrentSpeed(string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		return (*i).second.getTransferSpeed();
	}
	return pair<float, float>(0, 0);
}

//	bool Torrent::closeDown() 
//	{
//	delete session;
//	session = NULL;
//		return true;
//	}

/*
struct Torrent_ 
{
	wstring file;
	
	pair<float,float> transferLimit;
	
	int connections;
	int uploads;
	
	torrent_handle handle;
};	

static libtorrent::session* session = NULL;
static std::map<wstring,Torrent_> torrents;
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

TorrentDetails getTorrentDetails(wstring filename)
{
	torrentIter existing = torrents.find(filename);
	TorrentDetails pTD;
	
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
	
	if(!fs::exists(p))
		fs::create_directory(p);
		
	for(torrentIter i=torrents.begin(); i!=torrents.end(); ++i)
	{
		i->second.handle.pause();
		entry e = i->second.handle.write_resume_data();
		
		halencode(p/wcstombs(i->second.file), e);
	}
	torrents.erase(torrents.begin(), torrents.end());
}
*/

};
