
#include "stdAfx.hpp"

#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <map>

#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/split_free.hpp>

#include <libtorrent/file.hpp>
#include <libtorrent/hasher.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/peer_connection.hpp>

#include "halTorrent.hpp"

namespace halite 
{

namespace lbt = libtorrent;
namespace fs = boost::filesystem;

using fs::path;
using fs::ifstream;
using fs::ofstream;
using boost::serialization::make_nvp;

class TorrentInternal
{
public:
	TorrentInternal() :		
		transferLimit_(std::pair<float, float>(-1, -1)),
		connections_(-1),
		uploads_(-1),
		paused_(false),
		inSession_(false)
	{}
	
	TorrentInternal(libtorrent::torrent_handle h, std::wstring f, path saveDirectory) :		
		transferLimit_(std::pair<float, float>(-1, -1)),
		connections_(-1),
		uploads_(-1),
		paused_(false),
		filename_(f),
		saveDirectory_(saveDirectory.string()),
		inSession_(true),
		handle_(h)
	{}
	
	TorrentDetail_ptr getTorrentDetails() const;
	void setTransferSpeed(float down, float up);
	void setConnectionLimit(int maxConn, int maxUpload);
	void setTransferSpeed();
	void setConnectionLimit();
	pair<float, float> getTransferSpeed();
	pair<int, int> getConnectionLimit();
	void pause();
	void resume();
	bool isPaused() const;
	
	const libtorrent::torrent_handle& handle() const { return handle_; }
	void setHandle(libtorrent::torrent_handle h) 
	{ 
		handle_ = h; 
		inSession_ = true;
	}	 
	
	bool inSession() const { return inSession_; }
	const string& saveDirectory() { return saveDirectory_; }
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & make_nvp("transferLimit", transferLimit_);
        ar & make_nvp("connections", connections_);
        ar & make_nvp("uploads", uploads_);
        ar & make_nvp("paused", paused_);
        ar & make_nvp("filename", filename_);
        ar & make_nvp("saveDirectory", saveDirectory_);
    }
	
private:		
	std::pair<float, float> transferLimit_;
	
	int connections_;
	int uploads_;
	bool paused_;
	bool inSession_;
	
	std::wstring filename_;
	std::string saveDirectory_;
	libtorrent::torrent_handle handle_;	
};

typedef std::map<std::string, TorrentInternal> TorrentMap;

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
		
		boost::posix_time::time_duration td(boost::posix_time::pos_infin);
		
		if (tS.download_payload_rate != 0)
		{
			td = boost::posix_time::seconds(	
				long( float(tS.total_wanted-tS.total_wanted_done) / tS.download_payload_rate ));
		}

		return TorrentDetail_ptr(new TorrentDetail(filename_, state, mbstowcs(tS.current_tracker), 
			pair<float, float>(tS.download_payload_rate, tS.upload_payload_rate),
			tS.progress, tS.distributed_copies, tS.total_wanted_done, tS.total_wanted,
			tS.num_peers, tS.num_seeds, td));
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
public:
	
	~BitTorrent_impl()
	{
		try
		{
			boost::filesystem::ofstream ofs(workingDirectory/"Torrents.xml");
			boost::archive::xml_oarchive oa(ofs);
			
			oa << make_nvp("torrents", torrents);
			return;
		}
		catch(const std::exception&)
		{
			return;
		}	
	}
	
private:
	BitTorrent_impl() :
		theSession(lbt::fingerprint("HL", 0, 2, 0, 8)),
		workingDirectory(globalModule().exePath().branch_path())
	{
		boost::filesystem::ifstream ifs(workingDirectory/"Torrents.xml");
		if (ifs)
		{
			boost::archive::xml_iarchive ia(ifs);
			
			ia >> make_nvp("torrents", torrents);
		}		
	}
	
	lbt::entry prepTorrent(path filename, path saveDirectory);
	void removalThread(lbt::torrent_handle handle);
	
	lbt::session theSession;
	TorrentMap torrents;
	const path workingDirectory;
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
		bool result = pimpl->theSession.listen_on(range);
		
/*		libtorrent::dht_settings settings;
		settings.service_port = pimpl->theSession.listen_port()+10;
		
		pimpl->theSession.set_dht_settings(settings);
		pimpl->theSession.start_dht();
		
		pimpl->theSession.add_dht_node(make_pair("192.168.11.12", 6891));
		pimpl->theSession.add_dht_node(make_pair("192.168.11.12", 6892));
		pimpl->theSession.add_dht_node(make_pair("192.168.11.12", 6893));
		pimpl->theSession.add_dht_node(make_pair("192.168.11.12", 6894));
*/		
		return result;	
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

lbt::entry BitTorrent_impl::prepTorrent(path filename, path saveDirectory)
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

	if (!exists(saveDirectory))
		create_directory(saveDirectory);
	
	return resumeData;
}

void BitTorrent::addTorrent(path file, path saveDirectory) 
{
	try 
	{	
	lbt::entry metadata = haldecode(file);
	lbt::entry resumedata = pimpl->prepTorrent(file, saveDirectory);
	
	TorrentMap::const_iterator existing = pimpl->torrents.find(file.leaf());
	
	if (existing == pimpl->torrents.end())
	{		
		lbt::torrent_handle handle = pimpl->theSession.add_torrent(metadata,
			saveDirectory, resumedata);
		
		pimpl->torrents.insert(TorrentMap::value_type(file.leaf(), 
			TorrentInternal(handle, mbstowcs(file.leaf()), saveDirectory)));
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
/*	try
	{
	
	libtorrent::torrent_info t;
	path full_path = pimpl->workingDirectory/"incoming"/files.leaf();
	
	ofstream out(filename, std::ios_base::binary);
	
	int piece_size = 256 * 1024;
	char const* creator_str = "Halite v0.3 (libtorrent v0.11)";

	add_files(t, full_path.branch_path(), full_path.leaf());
	t.set_piece_size(piece_size);

	lbt::storage st(t, full_path.branch_path());
	t.add_tracker("http://www.nitcom.com.au/announce.php");
	t.set_priv(false);
	t.add_node(make_pair("192.168.11.12", 6881));

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
*/
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
			lbt::entry resumedata = pimpl->prepTorrent(file, (*iter).second.saveDirectory());
			
			(*iter).second.setHandle(pimpl->theSession.add_torrent(metadata,
				path((*iter).second.saveDirectory()), resumedata));
			
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
	
	if (!pimpl->torrents.empty() && !exists(resumeDir))
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

bool BitTorrent::isTorrent(string filename)
{	
	return (pimpl->torrents.find(filename) != pimpl->torrents.end());
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

void BitTorrent_impl::removalThread(lbt::torrent_handle handle)
{
	theSession.remove_torrent(handle);
}

void BitTorrent::removeTorrent(string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		lbt::torrent_handle handle = (*i).second.handle();
		pimpl->torrents.erase(i);
		thread t(bind(&BitTorrent_impl::removalThread, &*pimpl, handle));
	}
}

void BitTorrent::reannounceTorrent(string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.handle().force_reannounce();
	}

//	Temporary Hijack

/*	ofstream out("dump.txt");
	
	lbt::entry ent = pimpl->theSession.dht_state();
	lbt::entry::dictionary_type dic = ent.dict();
	
	for (lbt::entry::dictionary_type::iterator j = dic.begin(); 
		j != dic.end(); ++j)
	{
		out << (*j).first << " " << (*j).second << std::endl;
	}		
*/	
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

};
