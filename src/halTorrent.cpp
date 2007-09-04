
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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
#include <boost/archive/text_woarchive.hpp>
#include <boost/archive/text_wiarchive.hpp>
#include <boost/archive/binary_woarchive.hpp>
#include <boost/archive/binary_wiarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

#include <libtorrent/file.hpp>
#include <libtorrent/hasher.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/ip_filter.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/peer_connection.hpp>
#include <libtorrent/extensions/metadata_transfer.hpp>
#include <libtorrent/extensions/ut_pex.hpp>

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


#include "halTorrentInternal.hpp"

namespace hal 
{
	libtorrent::session* TorrentInternal::the_session_ = 0;
}

namespace hal 
{

namespace lbt = libtorrent;
namespace fs = boost::filesystem;

using fs::path;
using fs::ifstream;
using fs::ofstream;
using boost::serialization::make_nvp;

lbt::entry haldecode(const wpath &file) 
{
	ifstream fs(file, ifstream::binary);
	if (fs.is_open()) 
	{
		fs.unsetf(ifstream::skipws);
		return lbt::bdecode(std::istream_iterator<char>(fs), std::istream_iterator<char>());
	}
	else return lbt::entry();
}

bool halencode(const wpath &file, const lbt::entry &e) 
{
	fs::ofstream fs(file, ofstream::binary);

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
	case lbt::alert::info:
		return Event::debug;
	
	case lbt::alert::warning:
		return Event::info;
	
	case lbt::alert::critical:
	case lbt::alert::fatal:
		return Event::critical;
	
	default:
		return Event::none;
	}
}

const PeerDetails& TorrentDetail::peerDetails() const
{
	if (!peerDetailsFilled_)
	{
		bittorrent().getAllPeerDetails(hal::to_utf8(filename_), peerDetails_);
		peerDetailsFilled_ = true;
	}
	
	return peerDetails_;
}

const FileDetails& TorrentDetail::fileDetails() const
{
	if (!fileDetailsFilled_)
	{
		bittorrent().getAllFileDetails(hal::to_utf8(filename_), fileDetails_);
		fileDetailsFilled_ = true;
	}
	
	return fileDetails_;
}

bool nameLess(const TorrentDetail_ptr& left, const TorrentDetail_ptr& right)
{
	return left->state() < right->state();
}

void TorrentDetails::sort(
	boost::function<bool (const TorrentDetail_ptr&, const TorrentDetail_ptr&)> fn) const
{
	std::stable_sort(torrents_.begin(), torrents_.end(), fn);
}

class BitTorrent_impl
{
	friend class BitTorrent;
	
public:
	
	~BitTorrent_impl()
	{
		keepChecking_ = false;
		
		saveTorrentData();
		
		try
		{
		
		if (ip_filter_changed_)
		{	
			fs::ofstream ofs(workingDirectory/L"IPFilter.bin", std::ios::binary);
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
			hal::event().post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventStdException(Event::critical, e, L"~BitTorrent_impl"))); 
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
							p_alert->timestamp(), hal::str_to_wstr(p_alert->msg()))));			
				}
				else
				{
					event().post(shared_ptr<EventDetail>(
						new EventLibtorrent(lbtAlertToHalEvent(p_alert->severity()), 
							p_alert->timestamp(), Event::unclassified, hal::str_to_wstr(p_alert->msg()))));
				}
				
				p_alert = theSession.pop_alert();
			}
			
			timer_.expires_from_now(boost::posix_time::seconds(5));
			timer_.async_wait(bind(&BitTorrent_impl::pop_alerts, this));
		}
	}
	
	void saveTorrentData()
	{	try
		{
		
		fs::wofstream ofs(workingDirectory/L"Torrents.xml");
		boost::archive::xml_woarchive oxa(ofs);
		
		oxa << make_nvp("torrents", torrents);
			
		if (dht_on_) 
		{	
			halencode(workingDirectory/L"DHTState.bin", theSession.dht_state());
		}
		
		}		
		catch(std::exception& e)
		{
			event().post(shared_ptr<EventDetail>(\
				new EventStdException(Event::critical, e, L"saveTorrentData")));
		}
	}
	
	int defTorrentMaxConn() { return defTorrentMaxConn_; }
	int defTorrentMaxUpload() { return defTorrentMaxUpload_; }
	float defTorrentDownload() { return defTorrentDownload_; }
	float defTorrentUpload() { return defTorrentUpload_; }
	
private:
	BitTorrent_impl() :
		theSession(lbt::fingerprint("HL", 0, 2, 9, 0)),
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
		TorrentInternal::the_session_ = &theSession;
		
		theSession.set_severity_level(lbt::alert::debug);
		
		theSession.add_extension(&lbt::create_metadata_plugin);
		theSession.add_extension(&lbt::create_ut_pex_plugin);
		theSession.set_max_half_open_connections(10);

		try
		{	
			fs::wifstream ifs(workingDirectory/L"Torrents.xml");
			if (ifs)
			{
				boost::archive::xml_wiarchive ia(ifs);			
				ia >> make_nvp("torrents", torrents);
			}
		}
		catch(const std::exception& e)
		{
			event().post(shared_ptr<EventDetail>(\
				new EventStdException(Event::fatal, e, L"Loading Torrents.xml")));
		}
		
		if (exists(workingDirectory/L"DHTState.bin"))
		{
			try
			{
				dht_state_ = haldecode(workingDirectory/L"DHTState.bin");
			}		
			catch(const std::exception& e)
			{
				event().post(shared_ptr<EventDetail>(
					new EventStdException(Event::critical, e, L"Loading DHTState.bin")));
			}
		}
		
		{	lbt::session_settings settings = theSession.settings();
			settings.user_agent = "Halite v 0.2.9";
			theSession.set_settings(settings);
		}
		
		timer_.expires_from_now(boost::posix_time::seconds(5));
		timer_.async_wait(bind(&BitTorrent_impl::pop_alerts, this));
	}
	
	lbt::entry prepTorrent(wpath filename, wpath saveDirectory);
	void removalThread(lbt::torrent_handle handle, bool wipeFiles);
	
	lbt::session theSession;
	asio::io_service io_;
	asio::deadline_timer timer_;
	bool keepChecking_;
	
	TorrentMap torrents;
	const wpath workingDirectory;
	
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

#define HAL_GENERIC_TORRENT_EXCEPTION_CATCH(TORRENT, FUNCTION) \
catch (const lbt::invalid_handle&)\
{\
	event().post(shared_ptr<EventDetail>(\
		new EventInvalidTorrent(Event::critical, Event::invalidTorrent, TORRENT, FUNCTION)));\
}\
catch (const std::exception& e)\
{\
	event().post(shared_ptr<EventDetail>(\
		new EventTorrentException(Event::critical, Event::torrentException, e.what(), TORRENT, FUNCTION)));\
}

void BitTorrent::shutDownSession()
{
	pimpl.reset();
}

void BitTorrent::saveTorrentData()
{
	pimpl->saveTorrentData();
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
	fs::ifstream ifs(workingDirectory/L"IPFilter.bin", std::ios::binary);
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

#ifndef TORRENT_DISABLE_ENCRYPTION	
void BitTorrent::ensure_pe_on(int enc_level, int in_enc_policy, int out_enc_policy, bool prefer_rc4)
{
	lbt::pe_settings pe;
	
	switch (enc_level)
	{
		case 0:
			pe.allowed_enc_level = lbt::pe_settings::plaintext;
			break;
		case 1:
			pe.allowed_enc_level = lbt::pe_settings::rc4;
			break;
		case 2:
			pe.allowed_enc_level = lbt::pe_settings::both;
			break;
		default:
			pe.allowed_enc_level = lbt::pe_settings::both;
			
			hal::event().post(shared_ptr<hal::EventDetail>(
				new hal::EventGeneral(hal::Event::warning, hal::Event::unclassified, 
					(wformat(hal::app().res_wstr(HAL_INCORRECT_ENCODING_LEVEL)) % enc_level).str())));
	}

	switch (in_enc_policy)
	{
		case 0:
			pe.in_enc_policy = lbt::pe_settings::forced;
			break;
		case 1:
			pe.in_enc_policy = lbt::pe_settings::enabled;
			break;
		case 2:
			pe.in_enc_policy = lbt::pe_settings::disabled;
			break;
		default:
			pe.in_enc_policy = lbt::pe_settings::enabled;
			
			hal::event().post(shared_ptr<hal::EventDetail>(
				new hal::EventGeneral(hal::Event::warning, hal::Event::unclassified, 
					(wformat(hal::app().res_wstr(HAL_INCORRECT_CONNECT_POLICY)) % in_enc_policy).str())));
	}

	switch (out_enc_policy)
	{
		case 0:
			pe.out_enc_policy = lbt::pe_settings::forced;
			break;
		case 1:
			pe.out_enc_policy = lbt::pe_settings::enabled;
			break;
		case 2:
			pe.out_enc_policy = lbt::pe_settings::disabled;
			break;
		default:
			pe.out_enc_policy = lbt::pe_settings::enabled;
			
			hal::event().post(shared_ptr<hal::EventDetail>(
				new hal::EventGeneral(hal::Event::warning, hal::Event::unclassified, 
					(wformat(hal::app().res_wstr(HAL_INCORRECT_CONNECT_POLICY)) % in_enc_policy).str())));
	}
	
	pe.prefer_rc4 = prefer_rc4;
	
	pimpl->theSession.set_pe_settings(pe);
}

void BitTorrent::ensure_pe_off()
{
	lbt::pe_settings pe;
	pe.out_enc_policy = lbt::pe_settings::disabled;
	pe.in_enc_policy = lbt::pe_settings::disabled;
	
	pe.allowed_enc_level = lbt::pe_settings::both;
	pe.prefer_rc4 = true;
	
	pimpl->theSession.set_pe_settings(pe);
}
#endif

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
	try
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
				catch(...)
				{
					hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::Event::info, from_utf8((format("Invalid IP range: %1%-%2%.") % first % last).str()))));
				}
			}
		}
	}
	
	pimpl->ip_filter_changed_ = true;
	pimpl->ip_filter_count();
	
	}
	catch(const std::exception& e)
	{
		event().post(shared_ptr<EventDetail>(
			new EventStdException(Event::critical, e, L"ip_filter_import_dat")));
	}
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

void BitTorrent::setSessionHalfOpenLimit(int halfConn)
{
	pimpl->theSession.set_max_half_open_connections(halfConn);
}

void BitTorrent::setTorrentDefaults(int maxConn, int maxUpload, float download, float upload)
{
	pimpl->defTorrentMaxConn_ = maxConn;
	pimpl->defTorrentMaxUpload_ = maxUpload;
	pimpl->defTorrentDownload_ = download;
	pimpl->defTorrentUpload_ = upload;
}

lbt::entry BitTorrent_impl::prepTorrent(wpath filename, wpath saveDirectory)
{
	lbt::entry resumeData;	
	const wpath resumeFile = workingDirectory/L"resume"/filename.leaf();
	
	if (exists(resumeFile)) 
	{
		try 
		{
			resumeData = haldecode(resumeFile);
		}
		catch(std::exception &e) 
		{		
			hal::event().post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventStdException(Event::critical, e, L"prepTorrent, Resume"))); 
	
//			::MessageBoxW(0, hal::str_to_wstr(ex.what()).c_str(), L"Resume Exception", MB_ICONERROR|MB_OK);
			remove(resumeFile);
		}
	}

	if (!exists(workingDirectory/L"torrents"))
		create_directory(workingDirectory/L"torrents");

	if (!exists(workingDirectory/L"torrents"/filename.leaf()))
		copy_file(filename.string(), workingDirectory/L"torrents"/filename.leaf());

	if (!exists(saveDirectory))
		create_directory(saveDirectory);
	
	return resumeData;
}

void BitTorrent::addTorrent(wpath file, wpath saveDirectory) 
{
	try 
	{	
	lbt::entry metadata = haldecode(file);
	lbt::entry resumedata = pimpl->prepTorrent(file, saveDirectory);
	
	TorrentMap::const_iterator existing = pimpl->torrents.find(to_utf8(file.leaf()));
	
	if (existing == pimpl->torrents.end())
	{		
		lbt::torrent_handle handle = pimpl->theSession.add_torrent(metadata,
			to_utf8(saveDirectory.string()), resumedata);
		
		pimpl->torrents.insert(TorrentMap::value_type(to_utf8(file.leaf()), 
			TorrentInternal(handle, file.leaf(), saveDirectory)));
	}

	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(to_utf8(file.string()), "addTorrent")
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

void BitTorrent::newTorrent(fs::wpath filename, fs::wpath files)
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

void BitTorrent::getAllTorrentDetail_vec(TorrentDetail_vec& torrentsContainer)
{
	try {
	
	torrentsContainer.reserve(pimpl->torrents.size());
	
	for (TorrentMap::const_iterator i=pimpl->torrents.begin(), e=pimpl->torrents.end(); i != e; ++i)
	{
		torrentsContainer.push_back((*i).second.getTorrentDetail_ptr());
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "getAllTorrentDetail_vec")
}

TorrentDetail_ptr BitTorrent::getTorrentDetail_vec(string filename)
{
	try {
	
	TorrentMap::const_iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		return i->second.getTorrentDetail_ptr();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "getTorrentDetail_ptr")
	
	return TorrentDetail_ptr();
}

/*const TorrentDetails& BitTorrent::getTorrentDetails(std::string selected, std::set<std::string> allSelected)
{
	return updateTorrentDetails(selected, allSelected);
}
*/
const TorrentDetails& BitTorrent::torrentDetails()
{
	return torrentDetails_;
}

const TorrentDetails& BitTorrent::updateTorrentDetails(std::string focused, std::set<std::string> selected)
{
	try {
	
	mutex_t::scoped_lock l(torrentDetails_.mutex_);	
	
	torrentDetails_.clearAll(l);
	
	torrentDetails_.torrents_.reserve(pimpl->torrents.size());
	
	for (TorrentMap::const_iterator i=pimpl->torrents.begin(), e=pimpl->torrents.end(); i != e; ++i)
	{	
		string utf8Filename = hal::to_utf8((*i).second.filename());
		TorrentDetail_ptr pT = (*i).second.getTorrentDetail_ptr();
		
		if (selected.find(utf8Filename) != selected.end())
		{
			torrentDetails_.selectedTorrents_.push_back(pT);
		}
		
		if (focused == utf8Filename)
			torrentDetails_.selectedTorrent_ = pT;
		
		torrentDetails_.torrentMap_[(*i).second.filename()] = pT;
		torrentDetails_.torrents_.push_back(pT);
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "updateTorrentDetails")
	
	return torrentDetails_;
}

void BitTorrent::resumeAll()
{
	try {
	
	for (TorrentMap::iterator iter = pimpl->torrents.begin(); 
		iter != pimpl->torrents.end(); /*Nothing here*/)
	{
		wpath file = pimpl->workingDirectory/L"torrents"/from_utf8((*iter).first);
		
		if (exists(file))
		{		
			try 
			{				
			lbt::entry metadata = haldecode(file);
			lbt::entry resumedata = pimpl->prepTorrent(file, (*iter).second.saveDirectory());
			
			(*iter).second.setEntryData(metadata, resumedata);
			
			if ((*iter).second.state() == TorrentDetail::torrent_paused)
				(*iter).second.pause();
			else if ((*iter).second.state() == TorrentDetail::torrent_stopped)
				(*iter).second.stop();	
			else
				(*iter).second.resume();
			
			++iter;
			}
			catch(const lbt::duplicate_torrent&)
			{
				hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::Event::debug, L"Encountered duplicate torrent")));
				
				++iter; // Harmless, don't worry about it.
			}
			catch(const std::exception& e) 
			{
				hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventStdException(hal::Event::warning, e, L"resumeAll")));
				
				pimpl->torrents.erase(iter++);
			}			
		}
		else
		{
			pimpl->torrents.erase(iter++);
		}
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "resumeAll")
}

void BitTorrent::closeAll()
{
	try {
	
	wpath resumeDir=pimpl->workingDirectory/L"resume";
	
	if (!pimpl->torrents.empty() && !exists(resumeDir))
		create_directory(resumeDir);

	event().post(shared_ptr<EventDetail>(
		new EventInfo(L"Stopping all torrents.")));
	
	for (TorrentMap::const_iterator i=pimpl->torrents.begin(), e=pimpl->torrents.end(); i != e; ++i)
	{
		if ((*i).second.inSession())
		{
			(*i).second.handle().pause(); // NB. internal pause, not registered in Torrents.xml
		}
	}

	
	// Ok this polling loop here is a bit curde, but a blocking wait is actually appropiate.
	lbt::session_status status = pimpl->theSession.status();	
	while (status.download_rate > 0 || status.upload_rate > 0)
	{
		Sleep(200);
		status = pimpl->theSession.status();
	}
	
	event().post(shared_ptr<EventDetail>(
		new EventInfo(L"Torrents stopped.")));
		
	for (TorrentMap::const_iterator i=pimpl->torrents.begin(), e=pimpl->torrents.end(); i != e; ++i)
	{
		if ((*i).second.inSession())
		{
			lbt::entry resumedata = (*i).second.handle().write_resume_data();
			pimpl->theSession.remove_torrent((*i).second.handle());
			
			bool halencode_result = halencode(resumeDir/from_utf8((*i).first), resumedata);
			assert(halencode_result);
		}
	}
	
	event().post(shared_ptr<EventDetail>(
		new EventInfo(L"Resume data written.")));
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "closeAll")
}

PeerDetail::PeerDetail(lbt::peer_info& peerInfo) :
	ipAddress(hal::safe_from_utf8(peerInfo.ip.address().to_string())),
	country(L""),
	speed(make_pair(peerInfo.payload_down_speed, peerInfo.payload_up_speed)),
	seed(peerInfo.seed),
	client(hal::safe_from_utf8(peerInfo.client))
{
	std::vector<wstring> status_vec;
	
#ifndef TORRENT_DISABLE_RESOLVE_COUNTRIES
	if (peerInfo.country[0] != 0 && peerInfo.country[1] != 0)
		country = (wformat(L"(%1%)") % hal::safe_from_utf8(string(peerInfo.country, 2))).str().c_str();
#endif	

	if (peerInfo.flags & lbt::peer_info::handshake)
	{
		status_vec.push_back(app().res_wstr(HAL_PEER_HANDSHAKE));
	}		
	else if (peerInfo.flags & lbt::peer_info::connecting)
	{
		status_vec.push_back(app().res_wstr(HAL_PEER_CONNECTING));
	}
	else
	{
	#ifndef TORRENT_DISABLE_ENCRYPTION		
		if (peerInfo.flags & lbt::peer_info::rc4_encrypted)
			status_vec.push_back(app().res_wstr(HAL_PEER_RC4_ENCRYPTED));		
		if (peerInfo.flags & lbt::peer_info::plaintext_encrypted)
			status_vec.push_back(app().res_wstr(HAL_PEER_PLAINTEXT_ENCRYPTED));
	#endif
		
		if (peerInfo.flags & lbt::peer_info::interesting)
			status_vec.push_back(app().res_wstr(HAL_PEER_INTERESTING));	
		if (peerInfo.flags & lbt::peer_info::choked)
			status_vec.push_back(app().res_wstr(HAL_PEER_CHOKED));	
		if (peerInfo.flags & lbt::peer_info::remote_interested)
			status_vec.push_back(app().res_wstr(HAL_PEER_REMOTE_INTERESTING));	
		if (peerInfo.flags & lbt::peer_info::remote_choked)
			status_vec.push_back(app().res_wstr(HAL_PEER_REMOTE_CHOKED));	
		if (peerInfo.flags & lbt::peer_info::supports_extensions)
			status_vec.push_back(app().res_wstr(HAL_PEER_SUPPORT_EXTENSIONS));	
	//	if (peerInfo.flags & lbt::peer_info::local_connection)						// Not sure whats up here?
	//		status_vec.push_back(app().res_wstr(HAL_PEER_LOCAL_CONNECTION));			
		if (peerInfo.flags & lbt::peer_info::queued)
			status_vec.push_back(app().res_wstr(HAL_PEER_QUEUED));
	}
	
	seed = (peerInfo.flags & lbt::peer_info::seed) ? true : false;
	
	if (!status_vec.empty()) status = status_vec[0];
	
	if (status_vec.size() > 1)
	{
		for (size_t i=1; i<status_vec.size(); ++i)
		{
			status += L"; ";
			status += status_vec[i];
		}
	}	
}

void BitTorrent::getAllPeerDetails(string filename, PeerDetails& peerContainer)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
		(*i).second.getPeerDetails(peerContainer);
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "getAllPeerDetails")
}

void BitTorrent::getAllFileDetails(string filename, FileDetails& fileDetails)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
		(*i).second.getFileDetails(fileDetails);
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "getAllFileDetails")
}

bool BitTorrent::isTorrent(string filename)
{	
	try {
	
	return (pimpl->torrents.find(filename) != pimpl->torrents.end());
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "isTorrent")
	
	return false;
}

void BitTorrent::pauseTorrent(string filename)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.pause();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "pauseTorrent")
}

void BitTorrent::resumeTorrent(string filename)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.resume();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "resumeTorrent")
}

void BitTorrent::stopTorrent(string filename)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.stop();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "stopTorrent")
}

bool BitTorrent::isTorrentActive(string filename)
{
	try {
	
	TorrentMap::const_iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		return (*i).second.isActive();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "isTorrentActive")
	
	return false; // ??? is this correct
}

void BitTorrent::setTorrentLogin(std::string filename, std::wstring username, std::wstring password)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.setTrackerLogin(username, password);
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "setTorrentLogin")
}

std::pair<std::wstring, std::wstring> BitTorrent::getTorrentLogin(std::string filename)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		return (*i).second.getTrackerLogin();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "getTorrentLogin")
	
	return std::make_pair(L"", L"");
}

void BitTorrent_impl::removalThread(lbt::torrent_handle handle, bool wipeFiles)
{
	try {
	
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
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "removalThread")
}

void BitTorrent::removeTorrent(string filename)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		lbt::torrent_handle handle = (*i).second.handle();
		pimpl->torrents.erase(i);
		thread t(bind(&BitTorrent_impl::removalThread, &*pimpl, handle, false));
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "removeTorrent")
}

void BitTorrent::removeTorrentWipeFiles(string filename)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		lbt::torrent_handle handle = (*i).second.handle();
		pimpl->torrents.erase(i);
		thread t(bind(&BitTorrent_impl::removalThread, &*pimpl, handle, true));
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "removeTorrentWipeFiles")
}

void BitTorrent::reannounceTorrent(string filename)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.handle().force_reannounce();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "reannounceTorrent")

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
	try {
	
	for (TorrentMap::iterator iter = pimpl->torrents.begin(); 
		iter != pimpl->torrents.end(); ++iter)
	{		
		if ((*iter).second.inSession())
			(*iter).second.pause();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "pauseAllTorrents")
}

void BitTorrent::unpauseAllTorrents()
{	
	try {
	
	for (TorrentMap::iterator iter = pimpl->torrents.begin(); 
		iter != pimpl->torrents.end(); ++iter)
	{
		if ((*iter).second.inSession() && iter->second.state() == TorrentDetail::torrent_paused)
			(*iter).second.resume();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "unpauseAllTorrents")
}

void BitTorrent::setTorrentLimit(string filename, int maxConn, int maxUpload)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.setConnectionLimit(maxConn, maxUpload);
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "setTorrentLimit")
}

void BitTorrent::setTorrentRatio(string filename, float ratio)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.setRatio(ratio);
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "setTorrentRatio")
}

float BitTorrent::getTorrentRatio(string filename)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		return (*i).second.getRatio();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "getTorrentRatio")
	
	return 0;
}

void BitTorrent::setTorrentSpeed(string filename, float download, float upload)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.setTransferSpeed(download, upload);
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "setTorrentSpeed")
}

pair<int, int> BitTorrent::getTorrentLimit(string filename)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		return (*i).second.getConnectionLimit();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "getTorrentLimit")
	
	return pair<int, int>(0, 0);
}

pair<float, float> BitTorrent::getTorrentSpeed(string filename)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		return (*i).second.getTransferSpeed();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "getTorrentSpeed")
	
	return pair<float, float>(0, 0);
}

void BitTorrent::setTorrentFilePriorities(std::string filename, 
	std::vector<int> fileIndices, int priority)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.setFilePriorities(fileIndices, priority);
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "setTorrentFilePriorities")
}

void BitTorrent::setTorrentTrackers(std::string filename, 
	const std::vector<TrackerDetail>& trackers)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.setTrackers(trackers);
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "setTorrentTrackers")
}

void BitTorrent::resetTorrentTrackers(std::string filename)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		(*i).second.resetTrackers();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "resetTorrentTrackers")
}

std::vector<TrackerDetail> BitTorrent::getTorrentTrackers(std::string filename)
{
	try {
	
	TorrentMap::iterator i = pimpl->torrents.find(filename);
	
	if (i != pimpl->torrents.end())
	{
		return (*i).second.getTrackers();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "getTorrentTrackers")
	
	return std::vector<TrackerDetail>();	
}

void BitTorrent::startEventReceiver()
{
	pimpl->keepChecking_ = true;
	thread(bind(&asio::io_service::run, &pimpl->io_));
}

void BitTorrent::stopEventReceiver()
{
	pimpl->keepChecking_ = false;
}

int BitTorrent::defTorrentMaxConn() { return pimpl->defTorrentMaxConn_; }
int BitTorrent::defTorrentMaxUpload() { return pimpl->defTorrentMaxUpload_; }
float BitTorrent::defTorrentDownload() { return pimpl->defTorrentDownload_; }
float BitTorrent::defTorrentUpload() { return pimpl->defTorrentUpload_; }
	
};
