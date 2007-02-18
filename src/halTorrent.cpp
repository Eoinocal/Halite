
#include "stdAfx.hpp"

#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <map>
#include <algorithm>
#include <string>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/regex.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/split_free.hpp>

#include <libtorrent/file.hpp>
#include <libtorrent/hasher.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/ip_filter.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/peer_connection.hpp>

#include "halTorrent.hpp"
#include "halEvent.hpp"
#include "global/string_conv.hpp"

#define foreach BOOST_FOREACH

namespace boost {
namespace serialization {

#define IP_SAVE  3

template<class Archive, class address_type>
void save(Archive& ar, const address_type& ip, const unsigned int version)
{	
#if IP_SAVE == 1
	typename address_type::bytes_type bytes = ip.to_bytes();	
	for (typename address_type::bytes_type::iterator i=bytes.begin(); i != bytes.end(); ++i)
		ar & BOOST_SERIALIZATION_NVP(*i);
#elif IP_SAVE == 2
	string dotted = ip.to_string(); 
	ar & BOOST_SERIALIZATION_NVP(dotted);
#elif IP_SAVE == 3
	unsigned long addr = ip.to_ulong();	
	ar & BOOST_SERIALIZATION_NVP(addr);
#endif
}

template<class Archive, class address_type>
void load(Archive& ar, address_type& ip, const unsigned int version)
{	
#if IP_SAVE == 1
	typename address_type::bytes_type bytes;	
	for (typename address_type::bytes_type::iterator i=bytes.begin(); i != bytes.end(); ++i)
		ar & BOOST_SERIALIZATION_NVP(*i);	
	ip = address_type(bytes);
#elif IP_SAVE == 2	
	string dotted;
	ar & BOOST_SERIALIZATION_NVP(dotted);	
	ip = address_type::from_string(dotted);
#elif IP_SAVE == 3
	unsigned long addr;
	ar & BOOST_SERIALIZATION_NVP(addr);	
	ip = address_type(addr);
#endif
}

template<class Archive, class address_type>
void serialize(Archive& ar, libtorrent::ip_range<address_type>& addr, const unsigned int version)
{	
	ar & BOOST_SERIALIZATION_NVP(addr.first);
	ar & BOOST_SERIALIZATION_NVP(addr.last);
	addr.flags = libtorrent::ip_filter::blocked;
}

template<class Archive>
void serialize(Archive& ar, hal::TrackerDetail& tracker, const unsigned int version)
{	
	ar & BOOST_SERIALIZATION_NVP(tracker.url);
	ar & BOOST_SERIALIZATION_NVP(tracker.tier);
}

} // namespace serialization
} // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(asio::ip::address_v4)
BOOST_SERIALIZATION_SPLIT_FREE(asio::ip::address_v6)

namespace libtorrent
{
template<class Addr>
bool operator==(const libtorrent::ip_range<Addr>& lhs, const int flags)
{
	return (lhs.flags == flags);
}

std::ostream& operator<<(std::ostream& os, libtorrent::ip_range<asio::ip::address_v4>& ip)
{
	os << ip.first.to_ulong();
	os << ip.last.to_ulong();
	
	return os;
}

} // namespace libtorrent

namespace hal 
{
class TorrentInternal;
}

BOOST_CLASS_VERSION(hal::TorrentInternal, 2)

namespace hal 
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
		state_(TorrentDetail::torrent_active),
		inSession_(false),
		totalUploaded_(0),
		totalBase_(0)
	{}
	
	TorrentInternal(libtorrent::torrent_handle h, std::wstring f, path saveDirectory) :		
		transferLimit_(std::pair<float, float>(bittorrent().defTorrentDownload(), bittorrent().defTorrentUpload())),
		connections_(bittorrent().defTorrentMaxConn()),
		uploads_(bittorrent().defTorrentMaxUpload()),
		state_(TorrentDetail::torrent_active),
		filename_(f),
		saveDirectory_(saveDirectory.string()),
		inSession_(true),
		handle_(h),
		totalUploaded_(0),
		totalBase_(0)
	{}
	
	TorrentDetail_ptr getTorrentDetails() const;
	void setTransferSpeed(float down, float up);
	void setConnectionLimit(int maxConn, int maxUpload);
	void setTransferSpeed();
	void setConnectionLimit();
	pair<float, float> getTransferSpeed();
	pair<int, int> getConnectionLimit();
	
	void resume()
	{
		if (inSession()) handle_.resume();
		state_ = TorrentDetail::torrent_active;
	}
	
	void pause()
	{
		if (state_ == TorrentDetail::torrent_active)
		{
			if (inSession()) handle_.pause();
			state_ = TorrentDetail::torrent_paused;	
		}
	}
	
	void stop()
	{
		if (inSession()) handle_.pause();
		state_ = TorrentDetail::torrent_stopped;	
	}
	
	bool isActive() const { return state_ == TorrentDetail::torrent_active;	}
	
	unsigned state() const { return state_; }
	
	void setTrackerLogin()
	{
		if (trackerUsername_ != L"")
		{
			handle_.set_tracker_login(hal::to_str(trackerUsername_),
				hal::to_gen_str<std::string, std::wstring>(trackerPassword_));
		}
	}
	
	void setTrackerLogin(wstring username, wstring password)
	{
		trackerUsername_ = username;
		trackerPassword_ = password;
		setTrackerLogin();
	}	
	
	pair<wstring, wstring> getTrackerLogin()
	{
		return make_pair(trackerUsername_, trackerPassword_);
	}
	
	const libtorrent::torrent_handle& handle() const { return handle_; }
	void setHandle(libtorrent::torrent_handle h) 
	{ 
		handle_ = h; 
		inSession_ = true;
	}
	
	void resetTrackers()
	{
		handle_.replace_trackers(torrent_trackers_);		
		trackers_.clear();	
	}
	
	const std::vector<TrackerDetail>& getTrackers()
	{
		if (trackers_.empty())
		{
			std::vector<lbt::announce_entry> trackers = handle_.trackers();
			
			foreach (const lbt::announce_entry& entry, trackers)
			{
				trackers_.push_back(
					TrackerDetail(hal::to_wstr(entry.url), entry.tier));
			}
		}		
		return trackers_;
	}
	
	void applyTrackers()
	{
		if (torrent_trackers_.empty())
			torrent_trackers_ = handle_.trackers();
		
		if (!trackers_.empty())
		{
			std::vector<lbt::announce_entry> trackers;
			
			foreach (const TrackerDetail& tracker, trackers_)
			{
				trackers.push_back(
					lbt::announce_entry(hal::to_str(tracker.url)));
				trackers.back().tier = tracker.tier;
			}
			handle_.replace_trackers(trackers);
		}
	}
	
	void setTrackers(const std::vector<TrackerDetail>& trackerDetails)
	{
		trackers_.clear();
		trackers_.assign(trackerDetails.begin(), trackerDetails.end());
		
		applyTrackers();
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
        ar & make_nvp("filename", filename_);
        ar & make_nvp("saveDirectory", saveDirectory_);
		if (version > 0) {
			ar & make_nvp("trackerUsername", trackerUsername_);
			ar & make_nvp("trackerPassword", trackerPassword_);
		}
		if (version > 1) {
			ar & make_nvp("state", state_);
			ar & make_nvp("trackers", trackers_);
			ar & make_nvp("totalUploaded", totalUploaded_);
		}
    }
	
private:		
	std::pair<float, float> transferLimit_;
	
	unsigned state_;
	int connections_;
	int uploads_;
	bool inSession_;
	
	std::wstring filename_;
	std::string saveDirectory_;
	libtorrent::torrent_handle handle_;	
	
	std::wstring trackerUsername_;	
	std::wstring trackerPassword_;
	
	mutable boost::int64_t totalUploaded_;
	mutable boost::int64_t totalBase_;
	
	std::vector<TrackerDetail> trackers_;
	std::vector<lbt::announce_entry> torrent_trackers_;
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
		
		if (state_ == TorrentDetail::torrent_paused)
			state = L"Paused";
		else if (state_ == TorrentDetail::torrent_stopped)
			state = L"Stopped";
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
		
		totalUploaded_ += (tS.total_payload_upload - totalBase_);
		totalBase_ = tS.total_payload_upload;

		return TorrentDetail_ptr(new TorrentDetail(filename_, state, hal::to_wstr(tS.current_tracker), 
			pair<float, float>(tS.download_payload_rate, tS.upload_payload_rate),
			tS.progress, tS.distributed_copies, tS.total_wanted_done, tS.total_wanted, totalUploaded_,
			tS.num_peers, tS.num_seeds, td));
	}
	else
	{
		return TorrentDetail_ptr(new TorrentDetail(filename_, L"Not in Session", L"No tracker"));
	}
}

bool operator!=(const lbt::dht_settings& lhs, const lbt::dht_settings& rhs)
{
	return lhs.max_peers_reply != rhs.max_peers_reply ||
		   lhs.search_branching != rhs.search_branching ||
		   lhs.service_port != rhs.service_port ||
           lhs.max_fail_count != rhs.max_fail_count;
}

template<typename Addr>
void write_range(fs::ofstream& ofs, const lbt::ip_range<Addr>& range)
{ 
	const typename Addr::bytes_type first = range.first.to_bytes();
	const typename Addr::bytes_type last = range.last.to_bytes();
	ofs.write((char*)first.elems, first.size());
	ofs.write((char*)last.elems, last.size());
}

template<typename Addr>
void write_vec_range(fs::ofstream& ofs, const std::vector<lbt::ip_range<Addr> >& vec)
{ 
	ofs << vec.size();
	
	for (typename std::vector<lbt::ip_range<Addr> >::const_iterator i=vec.begin(); 
		i != vec.end(); ++i)
	{
		write_range(ofs, *i);
	}
}

template<typename Addr>
void read_range_to_filter(fs::ifstream& ifs, lbt::ip_filter& ip_filter)
{ 
	typename Addr::bytes_type first;
	typename Addr::bytes_type last;
	ifs.read((char*)first.elems, first.size());
	ifs.read((char*)last.elems, last.size());	
	
	ip_filter.add_rule(Addr(first), Addr(last),
		lbt::ip_filter::blocked);
}

static Event::eventLevel lbtAlertToHalEvent(lbt::alert::severity_t severity)
{
	switch (severity)
	{
	case lbt::alert::debug:
		return Event::debug;
		
	case lbt::alert::info:
	case lbt::alert::warning:
		return Event::info;
		
	case lbt::alert::critical:
	case lbt::alert::fatal:
		return Event::critical;
		
	default:
		return Event::none;
	}
}

class BitTorrent_impl
{
	friend class BitTorrent;
public:
	
	~BitTorrent_impl()
	{
		keepChecking_ = false;
		
		try
		{
		{	fs::ofstream ofs(workingDirectory/"Torrents.xml");
			boost::archive::xml_oarchive oxa(ofs);
			
			oxa << make_nvp("torrents", torrents);
		}	
		if (dht_on_) 
		{	
			halencode(workingDirectory/"DHTState.bin", theSession.dht_state());
		}
		if (ip_filter_changed_)
		{	
			fs::ofstream ofs(workingDirectory/"IPFilter.bin", std::ios::binary);
//			boost::archive::binary_oarchive oba(ofs);
			
			lbt::ip_filter::filter_tuple_t vectors = ip_filter_.export_filter();	
			
			std::vector<lbt::ip_range<asio::ip::address_v4> > v4(vectors.get<0>());
			std::vector<lbt::ip_range<asio::ip::address_v6> > v6(vectors.get<1>());
			
			v4.erase(std::remove(v4.begin(), v4.end(), 0), v4.end());
			v6.erase(std::remove(v6.begin(), v6.end(), 0), v6.end());

			write_vec_range(ofs, v4);
//			write_vec_range(ofs, v6);
		}	
		}
		catch(std::exception& e)
		{
			::MessageBox(0, (wformat(L"Save data exception: %1%") % e.what()).str().c_str(), L"Exception", 0);
		}
	}
	
	void pop_alerts()
	{
		if (keepChecking_)
		{
			std::auto_ptr<lbt::alert> p_alert = theSession.pop_alert();
			
			while (p_alert.get())
			{	
				if (lbt::peer_error_alert* peer = dynamic_cast<lbt::peer_error_alert*>(p_alert.get()))
				{
					event().post(shared_ptr<EventDetail>(
						new EventPeerAlert(lbtAlertToHalEvent(p_alert->severity()), 
							p_alert->timestamp(), hal::to_wstr(p_alert->msg()))));			
				}
				else
				{
					event().post(shared_ptr<EventDetail>(
						new EventLibtorrent(lbtAlertToHalEvent(p_alert->severity()), 
							p_alert->timestamp(), hal::to_wstr(p_alert->msg()))));
				}
				
				p_alert = theSession.pop_alert();
			}
			
			timer_.expires_from_now(boost::posix_time::seconds(5));
			timer_.async_wait(bind(&BitTorrent_impl::pop_alerts, this));
		}
	}
	
	int defTorrentMaxConn() { return defTorrentMaxConn_; }
	int defTorrentMaxUpload() { return defTorrentMaxUpload_; }
	float defTorrentDownload() { return defTorrentDownload_; }
	float defTorrentUpload() { return defTorrentUpload_; }
	
private:
	BitTorrent_impl() :
		theSession(lbt::fingerprint("HL", 0, 2, 0, 9)),
		timer_(io_),
		keepChecking_(false),
		workingDirectory(hal::app().exe_path().branch_path()),
		defTorrentMaxConn_(-1),
		defTorrentMaxUpload_(-1),
		defTorrentDownload_(-1),
		defTorrentUpload_(-1),
		ip_filter_on_(false),
		ip_filter_loaded_(false),
		ip_filter_changed_(false),
		ip_filter_count_(0),
		dht_on_(false)
	{
		theSession.set_severity_level(lbt::alert::debug);
		
		{	fs::ifstream ifs(workingDirectory/"Torrents.xml");
			if (ifs)
			{
				boost::archive::xml_iarchive ia(ifs);			
				ia >> make_nvp("torrents", torrents);
			}
		}
		if (exists(workingDirectory/"DHTState.bin"))
			dht_state_ = haldecode(workingDirectory/"DHTState.bin");
				
		{	lbt::session_settings settings = theSession.settings();
			settings.user_agent = "Halite v 0.2.9 dev3";
			theSession.set_settings(settings);
		}
		
		timer_.expires_from_now(boost::posix_time::seconds(5));
		timer_.async_wait(bind(&BitTorrent_impl::pop_alerts, this));
	}
	
	lbt::entry prepTorrent(path filename, path saveDirectory);
	void removalThread(lbt::torrent_handle handle, bool wipeFiles);
	
	lbt::session theSession;
	asio::io_service io_;
	asio::deadline_timer timer_;
	bool keepChecking_;
	
	TorrentMap torrents;
	const path workingDirectory;
	
	int defTorrentMaxConn_;
	int defTorrentMaxUpload_;
	float defTorrentDownload_;
	float defTorrentUpload_;
	
	bool ip_filter_on_;
	bool ip_filter_loaded_;
	bool ip_filter_changed_;
	lbt::ip_filter ip_filter_;
	size_t ip_filter_count_;
	
	void ip_filter_count();
	void ip_filter_load(progressCallback fn);
	void ip_filter_import(std::vector<lbt::ip_range<asio::ip::address_v4> >& v4,
		std::vector<lbt::ip_range<asio::ip::address_v6> >& v6);
	
	bool dht_on_;
	lbt::dht_settings dht_settings_;
	lbt::entry dht_state_;
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
	ensure_dht_off();
	pimpl->theSession.listen_on(make_pair(0, 0));
}

bool BitTorrent::ensure_dht_on()
{
	if (!pimpl->dht_on_)
	{		
		try
		{
		pimpl->theSession.start_dht(pimpl->dht_state_);
		pimpl->dht_on_ = true;
		}
		catch(...)
		{}
	}
		return pimpl->dht_on_;
}

void BitTorrent::ensure_dht_off()
{
	if (pimpl->dht_on_)
	{
		pimpl->theSession.stop_dht();		
		pimpl->dht_on_ = false;
	}
}

void BitTorrent::setDhtSettings(int max_peers_reply, int search_branching, 
	int service_port, int max_fail_count)
{
	lbt::dht_settings settings;
	settings.max_peers_reply = max_peers_reply;
	settings.search_branching = search_branching;
	settings.service_port = service_port;
	settings.max_fail_count = max_fail_count;
	
	if (pimpl->dht_settings_ != settings)
	{
		pimpl->dht_settings_ = settings;
		pimpl->theSession.set_dht_settings(pimpl->dht_settings_);
	}
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

void BitTorrent_impl::ip_filter_count()
{
	lbt::ip_filter::filter_tuple_t vectors = ip_filter_.export_filter();
	
	vectors.get<0>().erase(std::remove(vectors.get<0>().begin(), vectors.get<0>().end(), 0),
		vectors.get<0>().end());
	vectors.get<1>().erase(std::remove(vectors.get<1>().begin(), vectors.get<1>().end(), 0),
		vectors.get<1>().end());
	ip_filter_count_ = vectors.get<0>().size() + vectors.get<1>().size();
}

void BitTorrent_impl::ip_filter_load(progressCallback fn)
{
	fs::ifstream ifs(workingDirectory/"IPFilter.bin", std::ios::binary);
	if (ifs)
	{
		size_t v4_size;
		ifs >> v4_size;
		
		size_t total = v4_size/100;
		size_t previous = 0;
					
		for(unsigned i=0; i<v4_size; ++i)
		{
			if (i-previous > total)
			{
				previous = i;
				if (fn) if (fn(size_t(i/total))) break;
			}
			
			read_range_to_filter<asio::ip::address_v4>(ifs, ip_filter_);
		}
	}	
}

void  BitTorrent_impl::ip_filter_import(std::vector<lbt::ip_range<asio::ip::address_v4> >& v4,
	std::vector<lbt::ip_range<asio::ip::address_v6> >& v6)
{
	for(std::vector<lbt::ip_range<asio::ip::address_v4> >::iterator i=v4.begin();
		i != v4.end(); ++i)
	{
		ip_filter_.add_rule(i->first, i->last, lbt::ip_filter::blocked);
	}
/*	for(std::vector<lbt::ip_range<asio::ip::address_v6> >::iterator i=v6.begin();
		i != v6.end(); ++i)
	{
		ip_filter_.add_rule(i->first, i->last, lbt::ip_filter::blocked);
	}
*/	
	/* Note here we do not set ip_filter_changed_ */
}

void BitTorrent::ensure_ip_filter_on(progressCallback fn)
{
	if (!pimpl->ip_filter_loaded_)
	{
		pimpl->ip_filter_load(fn);
		pimpl->ip_filter_loaded_ = true;
	}
	
	if (!pimpl->ip_filter_on_)
	{
		pimpl->theSession.set_ip_filter(pimpl->ip_filter_);
		pimpl->ip_filter_on_ = true;
		pimpl->ip_filter_count();
	}
}

void BitTorrent::ensure_ip_filter_off()
{
	pimpl->theSession.set_ip_filter(lbt::ip_filter());
	pimpl->ip_filter_on_ = false;
}

void BitTorrent::ip_v4_filter_block(asio::ip::address_v4 first, asio::ip::address_v4 last)
{
	pimpl->ip_filter_.add_rule(first, last, lbt::ip_filter::blocked);
	pimpl->ip_filter_count();
	pimpl->ip_filter_changed_ = true;
}

void BitTorrent::ip_v6_filter_block(asio::ip::address_v6 first, asio::ip::address_v6 last)
{
	pimpl->ip_filter_.add_rule(first, last, lbt::ip_filter::blocked);
	pimpl->ip_filter_count();
	pimpl->ip_filter_changed_ = true;
}

size_t BitTorrent::ip_filter_size()
{
	return pimpl->ip_filter_count_;
}

void BitTorrent::clearIpFilter()
{
	pimpl->ip_filter_ = lbt::ip_filter();
	pimpl->theSession.set_ip_filter(lbt::ip_filter());	
	pimpl->ip_filter_changed_ = true;
	pimpl->ip_filter_count();
}

void BitTorrent::ip_filter_import_dat(boost::filesystem::path file, progressCallback fn, bool octalFix)
{
	fs::ifstream ifs(file);	
	if (ifs)
	{
		boost::uintmax_t total = fs::file_size(file)/100;
		boost::uintmax_t progress = 0;
		boost::uintmax_t previous = 0;
		
		boost::regex reg("\\s*(\\d+\\.\\d+\\.\\d+\\.\\d+)\\s*-\\s*(\\d+\\.\\d+\\.\\d+\\.\\d+)\\s*.*");
		boost::regex ip_reg("0*(\\d*)\\.0*(\\d*)\\.0*(\\d*)\\.0*(\\d*)");
		boost::smatch m;
		
		string ip_address_line;		
		while (!std::getline(ifs, ip_address_line).eof())
		{		
			progress += (ip_address_line.length() + 2);
			if (progress-previous > total)
			{
				previous = progress;
				if (fn)
				{
					if (fn(size_t(progress/total))) 
						break;
				}
			}
			
			if (boost::regex_match(ip_address_line, m, reg))
			{
				string first = m[1];
				string last = m[2];
				
				if (octalFix)
				{
					if (boost::regex_match(first, m, ip_reg))
					{
						first = ((m.length(1) != 0) ? m[1] : string("0")) + "." +
								((m.length(2) != 0) ? m[2] : string("0")) + "." +
								((m.length(3) != 0) ? m[3] : string("0")) + "." +
								((m.length(4) != 0) ? m[4] : string("0"));
					}					
					if (boost::regex_match(last, m, ip_reg))
					{
						last = ((m.length(1) != 0) ? m[1] : string("0")) + "." +
							   ((m.length(2) != 0) ? m[2] : string("0")) + "." +
							   ((m.length(3) != 0) ? m[3] : string("0")) + "." +
							   ((m.length(4) != 0) ? m[4] : string("0"));
					}
				}
				
				try
				{			
				pimpl->ip_filter_.add_rule(asio::ip::address_v4::from_string(first),
					asio::ip::address_v4::from_string(last), lbt::ip_filter::blocked);	
				}
				catch(const std::exception& e)
				{
					::MessageBoxA(0, (format("%1%: %1% %2%") % e.what() % first % last).str().c_str(), "Load ipfilter.dat Exception!", 0);
				}
			}
		}
	}
	
	pimpl->ip_filter_changed_ = true;
	pimpl->ip_filter_count();
}

const SessionDetail BitTorrent::getSessionDetails()
{
	SessionDetail details;
	
	details.port = pimpl->theSession.is_listening() ? pimpl->theSession.listen_port() : -1;
	
	lbt::session_status status = pimpl->theSession.status();
	
	details.speed = pair<double, double>(status.download_rate, status.upload_rate);
	
	details.dht_on = pimpl->dht_on_;
	details.dht_nodes = status.dht_nodes;
	details.dht_torrents = status.dht_torrents;
	
	details.ip_filter_on = pimpl->ip_filter_on_;
	details.ip_ranges_filtered = pimpl->ip_filter_count_;
	
	return details;
}

void BitTorrent::setTorrentDefaults(int maxConn, int maxUpload, float download, float upload)
{
	pimpl->defTorrentMaxConn_ = maxConn;
	pimpl->defTorrentMaxUpload_ = maxUpload;
	pimpl->defTorrentDownload_ = download;
	pimpl->defTorrentUpload_ = upload;
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
			::MessageBoxW(0, hal::to_wstr(ex.what()).c_str(), L"Resume Exception", MB_ICONERROR|MB_OK);
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
			TorrentInternal(handle, hal::to_wstr(file.leaf()), saveDirectory)));
	}

	}
	catch(std::exception &ex) 
	{
		wstring caption=L"Add Torrent Exception";
		
		MessageBox(0, hal::to_wstr(ex.what()).c_str(), caption.c_str(), MB_ICONERROR|MB_OK);
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
			
			if ((*iter).second.state() == TorrentDetail::torrent_paused)
				(*iter).second.pause();
			else if ((*iter).second.state() == TorrentDetail::torrent_stopped)
				(*iter).second.stop();
			
			(*iter).second.setTransferSpeed();
			(*iter).second.setConnectionLimit();
			(*iter).second.applyTrackers();
			
			++iter;
			}
			catch(const lbt::duplicate_torrent&)
			{
				++iter; // Harmless, don't worry about it.
			}
			catch(std::exception &ex) 
			{
				MessageBox(0, hal::to_wstr(ex.what()).c_str(), L"Resume Torrent Exception", MB_ICONERROR|MB_OK);
				
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
	ipAddress(hal::to_wstr(peerInfo.ip.address().to_string())),
	speed(make_pair(peerInfo.payload_down_speed, peerInfo.payload_up_speed)),
	seed(peerInfo.seed),
	client(hal::to_wstr(peerInfo.client))
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

void BitTorrent::stopTorrent(string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.stop();
	}
}

bool BitTorrent::isTorrentActive(string filename)
{
	TorrentMap::const_iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		return (*i).second.isActive();
	}
	
	return false; // ??? is this correct
}

void BitTorrent::setTorrentLogin(std::string filename, std::wstring username, std::wstring password)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.setTrackerLogin(username, password);
	}
}

std::pair<std::wstring, std::wstring>  BitTorrent::getTorrentLogin(std::string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		return (*i).second.getTrackerLogin();
	}
	
	return std::make_pair(L"", L"");
}

void BitTorrent_impl::removalThread(lbt::torrent_handle handle, bool wipeFiles)
{
	if (!wipeFiles)
		theSession.remove_torrent(handle);
	else
	{
		fs::path saveDirectory = handle.save_path();
		lbt::torrent_info info = handle.get_torrent_info();
		
		theSession.remove_torrent(handle);
		
		foreach (const lbt::file_entry& entry, make_pair(info.begin_files(), info.end_files()))
		{
			path file_path = saveDirectory / entry.path;
			
			if (exists(file_path) && !file_path.empty())
				remove_all(file_path);
		}
		
		if (info.num_files() != 1)
		{
			path dir_path = saveDirectory / info.name();
			if (exists(dir_path) && is_empty(dir_path))
				remove_all(dir_path);
		}
	}	
}

void BitTorrent::removeTorrent(string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		lbt::torrent_handle handle = (*i).second.handle();
		pimpl->torrents.erase(i);
		thread t(bind(&BitTorrent_impl::removalThread, &*pimpl, handle, false));
	}
}

void BitTorrent::removeTorrentWipeFiles(string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		lbt::torrent_handle handle = (*i).second.handle();
		pimpl->torrents.erase(i);
		thread t(bind(&BitTorrent_impl::removalThread, &*pimpl, handle, true));
	}
}

void BitTorrent::reannounceTorrent(string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.handle().force_reannounce();
	}

//	pimpl->theSession.add_dht_node(make_pair(string("192.168.11.4"),6881));
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

void BitTorrent::unpauseAllTorrents()
{	
	for (TorrentMap::iterator iter = pimpl->torrents.begin(); 
		iter != pimpl->torrents.end(); ++iter)
	{
		if (iter->second.state() == TorrentDetail::torrent_paused)
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

void BitTorrent::setTorrentTrackers(std::string filename, 
	const std::vector<TrackerDetail>& trackers)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.setTrackers(trackers);
	}
}

void BitTorrent::resetTorrentTrackers(std::string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.resetTrackers();
	}
}

std::vector<TrackerDetail> BitTorrent::getTorrentTrackers(std::string filename)
{
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		return (*i).second.getTrackers();
	}
	
	return std::vector<TrackerDetail>();
}

/*void BitTorrent::setSeverityLevel(Event::eventLevel event)
{
	switch (event)
	{
	case Event::debug:
		pimpl->theSession.set_severity_level(lbt::alert::debug);
		break;
	case Event::info:
		pimpl->theSession.set_severity_level(lbt::alert::info);
		break;
	case Event::warning:
		pimpl->theSession.set_severity_level(lbt::alert::warning);
		break;
	case Event::critical:
		pimpl->theSession.set_severity_level(lbt::alert::critical);
		break;
	case fatal:
		Event::pimpl->theSession.set_severity_level(lbt::alert::fatal);
		break;
	default:
		Event::pimpl->theSession.set_severity_level(lbt::alert::none);
		break;
	}
}*/

void BitTorrent::startEventReceiver()
{
	pimpl->keepChecking_ = true;
	thread(bind(&asio::io_service::run, &pimpl->io_));
}

void BitTorrent::stopEventReceiver()
{
	pimpl->keepChecking_ = false;
}

/*boost::signals::scoped_connection BitTorrent::attachEventReceiver(boost::function<void (shared_ptr<EventDetail>)> fn)
{
	return pimpl->event_signal_.connect(fn);
}

void BitTorrent::postEvent(boost::shared_ptr<EventDetail> event)
{
	pimpl->event_signal_(event);
}
*/

int BitTorrent::defTorrentMaxConn() { return pimpl->defTorrentMaxConn_; }
int BitTorrent::defTorrentMaxUpload() { return pimpl->defTorrentMaxUpload_; }
float BitTorrent::defTorrentDownload() { return pimpl->defTorrentDownload_; }
float BitTorrent::defTorrentUpload() { return pimpl->defTorrentUpload_; }
	
};
