
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <libtorrent/file.hpp>
#include <libtorrent/hasher.hpp>
#include <libtorrent/storage.hpp>
#include <libtorrent/file_pool.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/ip_filter.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/peer_connection.hpp>
#include <libtorrent/extensions/metadata_transfer.hpp>
#include <libtorrent/extensions/ut_pex.hpp>

#include <boost/tuple/tuple.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include "halIni.hpp"
#include "halTypes.hpp"
#include "halEvent.hpp"
#include "halTorrentInternal.hpp"
#include "halSignaler.hpp"

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

template<class Archive, class String, class Traits>
void save(Archive& ar, const boost::filesystem::basic_path<String, Traits>& p, const unsigned int version)
{	
	String str = p.string();
	ar & BOOST_SERIALIZATION_NVP(str);
}

template<class Archive, class String, class Traits>
void load(Archive& ar, boost::filesystem::basic_path<String, Traits>& p, const unsigned int version)
{	
	String str;
	ar & BOOST_SERIALIZATION_NVP(str);

	p = str;
}

template<class Archive, class String, class Traits>
inline void serialize(
        Archive & ar,
        boost::filesystem::basic_path<String, Traits>& p,
        const unsigned int file_version
){
        split_free(ar, p, file_version);            
}

template<class Archive, class address_type>
void serialize(Archive& ar, libtorrent::ip_range<address_type>& addr, const unsigned int version)
{	
	ar & BOOST_SERIALIZATION_NVP(addr.first);
	ar & BOOST_SERIALIZATION_NVP(addr.last);
	addr.flags = libtorrent::ip_filter::blocked;
}

template<class Archive>
void serialize(Archive& ar, hal::tracker_detail& tracker, const unsigned int version)
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

inline
std::ostream& operator<<(std::ostream& os, libtorrent::ip_range<asio::ip::address_v4>& ip)
{
	os << ip.first.to_ulong();
	os << ip.last.to_ulong();
	
	return os;
}

} // namespace libtorrent

namespace hal
{

namespace libt = libtorrent;

inline
bool operator!=(const libt::dht_settings& lhs, const libt::dht_settings& rhs)
{
	return lhs.max_peers_reply != rhs.max_peers_reply ||
		   lhs.search_branching != rhs.search_branching ||
		   lhs.service_port != rhs.service_port ||
           lhs.max_fail_count != rhs.max_fail_count;
}

template<typename Addr>
void write_range(fs::ofstream& ofs, const libt::ip_range<Addr>& range)
{ 
	const typename Addr::bytes_type first = range.first.to_bytes();
	const typename Addr::bytes_type last = range.last.to_bytes();
	ofs.write((char*)first.elems, first.size());
	ofs.write((char*)last.elems, last.size());
}

template<typename Addr>
void write_vec_range(fs::ofstream& ofs, const std::vector<libt::ip_range<Addr> >& vec)
{ 
	ofs << vec.size();
	
	for (typename std::vector<libt::ip_range<Addr> >::const_iterator i=vec.begin(); 
		i != vec.end(); ++i)
	{
		write_range(ofs, *i);
	}
}

template<typename Addr>
void read_range_to_filter(fs::ifstream& ifs, libt::ip_filter& ip_filter)
{ 
	typename Addr::bytes_type first;
	typename Addr::bytes_type last;
	ifs.read((char*)first.elems, first.size());
	ifs.read((char*)last.elems, last.size());	
	
	ip_filter.add_rule(Addr(first), Addr(last),
		libt::ip_filter::blocked);
}

static event_logger::eventLevel lbtAlertToHalEvent(libt::alert::severity_t severity)
{
	switch (severity)
	{
	case libt::alert::debug:
		return event_logger::debug;
	
	case libt::alert::info:
		return event_logger::info;
	
	case libt::alert::warning:
		return event_logger::warning;
	
	case libt::alert::critical:
	case libt::alert::fatal:
		return event_logger::critical;
	
	default:
		return event_logger::none;
	}
}

#define HAL_GENERIC_TORRENT_EXCEPTION_CATCH(TORRENT, FUNCTION) \
catch (const libt::invalid_handle&) \
{\
	event_log.post(shared_ptr<EventDetail>( \
		new EventInvalidTorrent(event_logger::critical, event_logger::invalidTorrent, TORRENT, std::string(FUNCTION)))); \
}\
catch (const invalidTorrent& t) \
{\
	event_log.post(shared_ptr<EventDetail>( \
		new EventInvalidTorrent(event_logger::info, event_logger::invalidTorrent, t.who(), std::string(FUNCTION)))); \
}\
catch (const std::exception& e) \
{\
	event_log.post(shared_ptr<EventDetail>( \
		new EventTorrentException(event_logger::critical, event_logger::torrentException, std::string(e.what()), TORRENT, std::string(FUNCTION)))); \
}

class bit_impl
{
	friend class bit;
	
public:
	
	~bit_impl()
	{
		stop_alert_handler();
		
		//save_torrent_data();
		
		try
		{
		
		if (ip_filter_changed_)
		{	
			fs::ofstream ofs(workingDirectory/L"IPFilter.bin", std::ios::binary);
//			boost::archive::binary_oarchive oba(ofs);
			
			libt::ip_filter::filter_tuple_t vectors = ip_filter_.export_filter();	
			
			std::vector<libt::ip_range<asio::ip::address_v4> > v4(vectors.get<0>());
			std::vector<libt::ip_range<asio::ip::address_v6> > v6(vectors.get<1>());
			
			v4.erase(std::remove(v4.begin(), v4.end(), 0), v4.end());
			v6.erase(std::remove(v6.begin(), v6.end(), 0), v6.end());

			write_vec_range(ofs, v4);
//			write_vec_range(ofs, v6);
		}	
		}
		catch(std::exception& e)
		{
			hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventStdException(event_logger::critical, e, L"~BitTorrent_impl"))); 
		}
	}

	bool listen_on(std::pair<int, int> const& range)
	{
		try
		{
		
		if (!session_.is_listening())
		{
			return session_.listen_on(range);
		}
		else
		{
			int port = session_.listen_port();
			
			if (port < range.first || port > range.second)
				return session_.listen_on(range);	
			else
			{
				signals.successful_listen();
				
				return true;
			}
		}
		
		}
		catch (const std::exception& e)
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventStdException(event_logger::fatal, e, L"From bit::listenOn.")));

			return false;
		}
		catch(...)
		{
			return false;
		}
	}

	int is_listening_on() 
	{
		if (!session_.is_listening())
			return -1;	
		else
			return session_.listen_port();
	}

	void stop_listening()
	{
		ensure_dht_off();
		session_.listen_on(std::make_pair(0, 0));
	}

	bool ensure_dht_on()
	{
		if (!dht_on_)
		{		
			try
			{
			session_.start_dht(dht_state_);
			dht_on_ = true;
			}
			catch(...)
			{}
		}
			return dht_on_;
	}

	void ensure_dht_off()
	{
		if (dht_on_)
		{
			session_.stop_dht();		
			dht_on_ = false;
		}
	}

	void set_dht_settings(int max_peers_reply, int search_branching, 
		int service_port, int max_fail_count)
	{
		libt::dht_settings settings;
		settings.max_peers_reply = max_peers_reply;
		settings.search_branching = search_branching;
		settings.service_port = service_port;
		settings.max_fail_count = max_fail_count;
		
		if (dht_settings_ != settings)
		{
			dht_settings_ = settings;
			session_.set_dht_settings(dht_settings_);
		}
	}

	void set_mapping(int mapping)
	{
		if (mapping != bit::mappingNone)
		{
			if (mapping == bit::mappingUPnP)
			{
				event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Starting UPnP mapping.")));
				session_.stop_upnp();
				session_.stop_natpmp();

				signals.successful_listen.connect_once(bind(&libt::session::start_upnp, &session_));
			}
			else
			{
				event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Starting NAT-PMP mapping.")));
				session_.stop_upnp();
				session_.stop_natpmp();

				signals.successful_listen.connect_once(bind(&libt::session::start_natpmp, &session_));
			}
		}
		else
		{
			event_log.post(shared_ptr<EventDetail>(new EventMsg(L"No mapping.")));
			session_.stop_upnp();
			session_.stop_natpmp();
		}
	}

	void set_timeouts(int peers, int tracker)
	{
		libt::session_settings settings = session_.settings();
		settings.peer_connect_timeout = peers;
		settings.tracker_completion_timeout = tracker;

		session_.set_settings(settings);

		event_log.post(shared_ptr<EventDetail>(new EventMsg(
			wformat(L"Set Timeouts, peer %1%, tracker %2%") % peers % tracker)));
	}

	void set_session_limits(int maxConn, int maxUpload)
	{		
		session_.set_max_uploads(maxUpload);
		session_.set_max_connections(maxConn);
		
		event_log.post(shared_ptr<EventDetail>(new EventMsg(
			wformat(L"Set connections totals %1% and uploads %2%.") 
				% maxConn % maxUpload)));
	}

	void set_session_speed(float download, float upload)
	{
		int down = (download > 0) ? static_cast<int>(download*1024) : -1;
		session_.set_download_rate_limit(down);
		int up = (upload > 0) ? static_cast<int>(upload*1024) : -1;
		session_.set_upload_rate_limit(up);
		
		event_log.post(shared_ptr<EventDetail>(new EventMsg(
			wformat(L"Set session rates at download %1% and upload %2%.") 
				% session_.download_rate_limit() % session_.upload_rate_limit())));
	}

	bool ensure_ip_filter_on(progress_callback fn)
	{
		try
		{
		
		if (!ip_filter_loaded_)
		{
			ip_filter_load(fn);
			ip_filter_loaded_ = true;
		}
		
		if (!ip_filter_on_)
		{
			session_.set_ip_filter(ip_filter_);
			ip_filter_on_ = true;
			ip_filter_count();
		}
		
		}
		catch(const std::exception& e)
		{		
			hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventStdException(event_logger::critical, e, L"ensureIpFilterOn"))); 

			ensure_ip_filter_off();
		}

		event_log.post(shared_ptr<EventDetail>(new EventMsg(L"IP filters on.")));	

		return false;
	}

	void ensure_ip_filter_off()
	{
		session_.set_ip_filter(libt::ip_filter());
		ip_filter_on_ = false;
		
		event_log.post(shared_ptr<EventDetail>(new EventMsg(L"IP filters off.")));	
	}

	#ifndef TORRENT_DISABLE_ENCRYPTION	
	void ensure_pe_on(int enc_level, int in_enc_policy, int out_enc_policy, bool prefer_rc4)
	{
		libt::pe_settings pe;
		
		switch (enc_level)
		{
			case 0:
				pe.allowed_enc_level = libt::pe_settings::plaintext;
				break;
			case 1:
				pe.allowed_enc_level = libt::pe_settings::rc4;
				break;
			case 2:
				pe.allowed_enc_level = libt::pe_settings::both;
				break;
			default:
				pe.allowed_enc_level = libt::pe_settings::both;
				
				hal::event_log.post(shared_ptr<hal::EventDetail>(
					new hal::EventGeneral(hal::event_logger::warning, hal::event_logger::unclassified, 
						(wformat(hal::app().res_wstr(HAL_INCORRECT_ENCODING_LEVEL)) % enc_level).str())));
		}

		switch (in_enc_policy)
		{
			case 0:
				pe.in_enc_policy = libt::pe_settings::forced;
				break;
			case 1:
				pe.in_enc_policy = libt::pe_settings::enabled;
				break;
			case 2:
				pe.in_enc_policy = libt::pe_settings::disabled;
				break;
			default:
				pe.in_enc_policy = libt::pe_settings::enabled;
				
				hal::event_log.post(shared_ptr<hal::EventDetail>(
					new hal::EventGeneral(hal::event_logger::warning, hal::event_logger::unclassified, 
						(wformat(hal::app().res_wstr(HAL_INCORRECT_CONNECT_POLICY)) % in_enc_policy).str())));
		}

		switch (out_enc_policy)
		{
			case 0:
				pe.out_enc_policy = libt::pe_settings::forced;
				break;
			case 1:
				pe.out_enc_policy = libt::pe_settings::enabled;
				break;
			case 2:
				pe.out_enc_policy = libt::pe_settings::disabled;
				break;
			default:
				pe.out_enc_policy = libt::pe_settings::enabled;
				
				hal::event_log.post(shared_ptr<hal::EventDetail>(
					new hal::EventGeneral(hal::event_logger::warning, hal::event_logger::unclassified, 
						(wformat(hal::app().res_wstr(HAL_INCORRECT_CONNECT_POLICY)) % in_enc_policy).str())));
		}
		
		pe.prefer_rc4 = prefer_rc4;
		
		try
		{
		
		session_.set_pe_settings(pe);
		
		}
		catch(const std::exception& e)
		{
			hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
					new hal::EventStdException(event_logger::critical, e, L"ensurePeOn"))); 
					
			ensure_pe_off();		
		}
		
		event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Protocol encryption on.")));
	}

	void ensure_pe_off()
	{
		libt::pe_settings pe;
		pe.out_enc_policy = libt::pe_settings::disabled;
		pe.in_enc_policy = libt::pe_settings::disabled;
		
		pe.allowed_enc_level = libt::pe_settings::both;
		pe.prefer_rc4 = true;
		
		session_.set_pe_settings(pe);

		event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Protocol encryption off.")));
	}
	#endif

	void ip_v4_filter_block(asio::ip::address_v4 first, asio::ip::address_v4 last)
	{
		ip_filter_.add_rule(first, last, libt::ip_filter::blocked);
		ip_filter_count();
		ip_filter_changed_ = true;
	}

	void ip_v6_filter_block(asio::ip::address_v6 first, asio::ip::address_v6 last)
	{
		ip_filter_.add_rule(first, last, libt::ip_filter::blocked);
		ip_filter_count();
		ip_filter_changed_ = true;
	}

	size_t ip_filter_size()
	{
		return ip_filter_count_;
	}

	void clear_ip_filter()
	{
		ip_filter_ = libt::ip_filter();
		session_.set_ip_filter(libt::ip_filter());	
		ip_filter_changed_ = true;
		ip_filter_count();
	}

	bool ip_filter_import_dat(boost::filesystem::path file, progress_callback fn, bool octalFix);

	struct 
	{
		signaler<> successful_listen;
		signaler<> torrent_finished;
	} 
	signals;

	void stop_alert_handler();
	void alert_handler();

	void add_torrent(wpath file, wpath saveDirectory, bool startStopped, bool compactStorage, 
			boost::filesystem::wpath moveToDirectory, bool useMoveTo) 
	{
		try 
		{	
		torrent_internal_ptr TIp;

		std::pair<std::string, std::string> names = extract_names(file);
		wstring xml_name = from_utf8(names.first) + L".xml";

		if (fs::exists(file.branch_path()/xml_name))
		{
			torrent_standalone tsa;
			
			if (tsa.load_standalone(file.branch_path()/xml_name))
			{
				TIp = tsa.torrent;
				
				TIp->set_save_directory(saveDirectory, true);			
				if (useMoveTo)
					TIp->set_move_to_directory(moveToDirectory);

				TIp->prepare(file);
			}
		}

		if (!TIp)
		{
			if (useMoveTo)
				TIp.reset(new torrent_internal(file, saveDirectory, compactStorage, moveToDirectory));		
			else
				TIp.reset(new torrent_internal(file, saveDirectory, compactStorage));

			TIp->setTransferSpeed(bittorrent().defTorrentDownload(), bittorrent().defTorrentUpload());
			TIp->setConnectionLimit(bittorrent().defTorrentMaxConn(), bittorrent().defTorrentMaxUpload());
		}
		
		std::pair<TorrentManager::torrentByName::iterator, bool> p =
			the_torrents_.insert(TIp);
		
		if (p.second)
		{
			torrent_internal_ptr me = the_torrents_.get(TIp->name());		
			
			if (!startStopped) 
				me->add_to_session();
			else
				me->set_state_stopped();
		}
		
		}
		catch (const std::exception& e)
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventTorrentException(event_logger::critical, event_logger::torrentException, 
					std::string(e.what()), to_utf8(file.string()), std::string("addTorrent"))));
		}
	}

	std::pair<libt::entry, libt::entry> prep_torrent(wpath filename, wpath saveDirectory)
	{
		libt::entry metadata = haldecode(filename);
		libt::torrent_info info(metadata);
	 	
		wstring torrentName = hal::from_utf8_safe(info.name());
		if (!boost::find_last(torrentName, L".torrent")) 
			torrentName += L".torrent";
		
		wpath torrentFilename = torrentName;
		const wpath resumeFile = workingDirectory/L"resume"/torrentFilename.leaf();
		
		//  vvv Handle old naming style!
		const wpath oldResumeFile = workingDirectory/L"resume"/filename.leaf();
		
		if (filename.leaf() != torrentFilename.leaf() && exists(oldResumeFile))
			fs::rename(oldResumeFile, resumeFile);
		//  ^^^ Handle old naming style!	
		
		libt::entry resumeData;	
		
		if (fs::exists(resumeFile)) 
		{
			try 
			{
				resumeData = haldecode(resumeFile);
			}
			catch(std::exception &e) 
			{		
				hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
					new hal::EventStdException(event_logger::critical, e, L"prepTorrent, Resume"))); 
		
				fs::remove(resumeFile);
			}
		}

		if (!fs::exists(workingDirectory/L"torrents"))
			fs::create_directory(workingDirectory/L"torrents");

		if (!fs::exists(workingDirectory/L"torrents"/torrentFilename.leaf()))
			fs::copy_file(filename.string(), workingDirectory/L"torrents"/torrentFilename.leaf());

		if (!fs::exists(saveDirectory))
			fs::create_directory(saveDirectory);
		
		return std::make_pair(metadata, resumeData);
	}

	void removal_thread(torrent_internal_ptr pIT, bool wipeFiles)
	{
		try {

		if (!wipeFiles)
		{
			session_.remove_torrent(pIT->handle());
		}
		else
		{
			if (pIT->in_session())
			{
				session_.remove_torrent(pIT->handle(), libt::session::delete_files);
			}
			else
			{
				libt::torrent_info m_info = pIT->infoMemory();
				
				// delete the files from disk
				std::string error;
				std::set<std::string> directories;
				
				for (libt::torrent_info::file_iterator i = m_info.begin_files(true)
					, end(m_info.end_files(true)); i != end; ++i)
				{
					std::string p = (hal::path_to_utf8(pIT->saveDirectory()) / i->path).string();
					fs::path bp = i->path.branch_path();
					
					std::pair<std::set<std::string>::iterator, bool> ret;
					ret.second = true;
					while (ret.second && !bp.empty())
					{
						std::pair<std::set<std::string>::iterator, bool> ret = 
							directories.insert((hal::path_to_utf8(pIT->saveDirectory()) / bp).string());
						bp = bp.branch_path();
					}
					if (!fs::remove(hal::from_utf8(p).c_str()) && errno != ENOENT)
						error = std::strerror(errno);
				}

				// remove the directories. Reverse order to delete subdirectories first

				for (std::set<std::string>::reverse_iterator i = directories.rbegin()
					, end(directories.rend()); i != end; ++i)
				{
					if (!fs::remove(hal::from_utf8(*i).c_str()) && errno != ENOENT)
						error = std::strerror(errno);
				}
			}
		}

		} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "removalThread")
	}

	void remove_torrent(const wstring& filename)
	{
		try {
		
		torrent_internal_ptr pTI = the_torrents_.get(filename);
		libt::torrent_handle handle = pTI->handle();
		the_torrents_.erase(filename);
		
		thread_t t(bind(&bit_impl::removal_thread, this, pTI, false));	
		
		} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "remove_torrent")
	}

	void remove_torrent_wipe_files(const std::wstring& filename)
	{
		try {
		
		torrent_internal_ptr pTI = the_torrents_.get(filename);
		libt::torrent_handle handle = pTI->handle();
		the_torrents_.erase(filename);
		
		thread_t t(bind(&bit_impl::removal_thread, this, pTI, true));	
		
		} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "remove_torrent_wipe_files")
	}

	void resume_all()
	{
		try {
			
		event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Resuming torrent.")));
		
		for (TorrentManager::torrentByName::iterator i=the_torrents_.begin(), e=the_torrents_.end(); i != e;)
		{
			wpath file = wpath(workingDirectory)/L"torrents"/(*i).torrent->filename();
			
			if (exists(file))
			{		
				try 
				{
					
				(*i).torrent->prepare(file);	

				switch ((*i).torrent->state())
				{
					case TorrentDetail::torrent_stopped:
						break;
					case TorrentDetail::torrent_paused:
						(*i).torrent->add_to_session(true);
						break;
					case TorrentDetail::torrent_active:
						(*i).torrent->add_to_session(false);
						break;
					default:
						assert(false);
				};
				
				++i;
				
				}
				catch(const libt::duplicate_torrent&)
				{
					hal::event_log.post(shared_ptr<hal::EventDetail>(
						new hal::EventDebug(hal::event_logger::debug, L"Encountered duplicate torrent")));
					
					++i; // Harmless, don't worry about it.
				}
				catch(const std::exception& e) 
				{
					hal::event_log.post(shared_ptr<hal::EventDetail>(
						new hal::EventStdException(hal::event_logger::warning, e, L"resumeAll")));
					
					the_torrents_.erase(i++);
				}			
			}
			else
			{
				the_torrents_.erase(i++);
			}
		}
		
		} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "closeAll")
	}

	void close_all(boost::optional<report_num_active> fn)
	{
		try 
		{	
		event_log.post(shared_ptr<EventDetail>(new EventInfo(L"Saving torrent data...")));

		save_torrent_data();

		event_log.post(shared_ptr<EventDetail>(new EventInfo(L"Stopping all torrents...")));
		
		for (TorrentManager::torrentByName::iterator i=the_torrents_.begin(), e=the_torrents_.end(); 
			i != e; ++i)
		{
			(*i).torrent->stop();
		}
		
		// Ok this polling loop here is a bit curde, but a blocking wait is actually appropiate.
		for (int num_active = -1; num_active != 0; )
		{
			num_active = 0;

			for (TorrentManager::torrentByName::iterator i=the_torrents_.begin(), e=the_torrents_.end(); 
					i != e; ++i)
			{
				if ((*i).torrent->state() != TorrentDetail::torrent_stopped)
					++num_active;
			}
			
			event_log.post(shared_ptr<EventDetail>(new EventInfo(wformat(L"%1% still active") % num_active)));

			if (fn)	(*fn)(num_active);
			Sleep(200);
		}
		
		event_log.post(shared_ptr<EventDetail>(new EventInfo(L"All torrents stopped.")));		
		event_log.post(shared_ptr<EventDetail>(new EventInfo(L"Fast-resume data written.")));
		
		} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "closeAll")
	}
	
	void save_torrent_data()
	{	
		mutex_t::scoped_lock l(mutex_);
		try
		{
		
		the_torrents_.save();
		bittorrentIni.save_data();
			
		if (dht_on_) 
		{	
			halencode(workingDirectory/L"DHTState.bin", session_.dht_state());
		}
		
		}		
		catch(std::exception& e)
		{
			event_log.post(shared_ptr<EventDetail>(\
				new EventStdException(event_logger::critical, e, L"saveTorrentData")));
		}
	}
	
	int defTorrentMaxConn() { return defTorrentMaxConn_; }
	int defTorrentMaxUpload() { return defTorrentMaxUpload_; }
	float defTorrentDownload() { return defTorrentDownload_; }
	float defTorrentUpload() { return defTorrentUpload_; }
	
	const wpath workingDir() { return workingDirectory; };

private:
	bit_impl() :
		session_(libt::fingerprint(HALITE_FINGERPRINT)),
		timer_(io_),
		keepChecking_(false),
		bittorrentIni(L"BitTorrent.xml"),
		the_torrents_(bittorrentIni),
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
		torrent_internal::the_session_ = &session_;
		torrent_internal::workingDir_ = workingDir();
		
		session_.set_severity_level(libt::alert::debug);		
		session_.add_extension(&libt::create_metadata_plugin);
		session_.add_extension(&libt::create_ut_pex_plugin);
		session_.set_max_half_open_connections(10);
		
		hal::event_log.post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(L"Loading BitTorrent.xml.", hal::event_logger::info)));		
		bittorrentIni.load_data();
		hal::event_log.post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(L"Loading torrent parameters.", hal::event_logger::info)));	
		the_torrents_.load();
		hal::event_log.post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(L"Loading done!", hal::event_logger::info)));
		
		try
		{						
		if (fs::exists(workingDirectory/L"Torrents.xml"))
		{
			{
			fs::wifstream ifs(workingDirectory/L"Torrents.xml");
		
			event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Loading old Torrents.xml")));
		
			TorrentMap torrents;
			boost::archive::xml_wiarchive ia(ifs);	
			ia >> boost::serialization::make_nvp("torrents", torrents);
			
			the_torrents_ = torrents;
			}
			
			event_log.post(shared_ptr<EventDetail>(new EventMsg(
				wformat(L"Total %1%.") % the_torrents_.size())));				
			
			fs::rename(workingDirectory/L"Torrents.xml", workingDirectory/L"Torrents.xml.safe.to.delete");
		}			
		}
		catch(const std::exception& e)
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventStdException(event_logger::fatal, e, L"Loading Old Torrents.xml")));
		}		
				
		if (exists(workingDirectory/L"DHTState.bin"))
		{
			try
			{
				dht_state_ = haldecode(workingDirectory/L"DHTState.bin");
			}		
			catch(const std::exception& e)
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventStdException(event_logger::critical, e, L"Loading DHTState.bin")));
			}
		}
		
		{	libt::session_settings settings = session_.settings();
			settings.user_agent = string("Halite ") + HALITE_VERSION_STRING;
			session_.set_settings(settings);
		}
		
		timer_.expires_from_now(boost::posix_time::seconds(5));
		timer_.async_wait(bind(&bit_impl::alert_handler, this));
	}

	bool create_torrent(const create_torrent_params& params, fs::wpath out_file, progress_callback fn)
	{		
	try
	{
		boost::intrusive_ptr<libt::torrent_info> t_info(new libt::torrent_info);

		int piece_size = params.piece_size;
		HAL_DEV_MSG(wformat(L"piece size: %1%") % piece_size);
		t_info->set_piece_size(piece_size);

		HAL_DEV_MSG(L"Files");
		for (file_size_pairs_t::const_iterator i = params.file_size_pairs.begin(), e = params.file_size_pairs.end();
				i != e; ++i)
		{
			HAL_DEV_MSG(wformat(L"file path: %1%, size: %2%") % (*i).first % (*i).second);
			t_info->add_file(to_utf8((*i).first.string()), (*i).second);
		}

		libt::file_pool f_pool;
		
		boost::scoped_ptr<libt::storage_interface> store(
			libt::default_storage_constructor(t_info, to_utf8(params.root_path.string()),
				f_pool));

		HAL_DEV_MSG(L"Trackers");
		for (tracker_details_t::const_iterator i = params.trackers.begin(), e = params.trackers.end();
				i != e; ++i)
		{
			HAL_DEV_MSG(wformat(L"URL: %1%, Tier: %2%") % (*i).url % (*i).tier);
			t_info->add_tracker(to_utf8((*i).url), (*i).tier);
		}

		HAL_DEV_MSG(L"Web Seeds");
		for (web_seed_details_t::const_iterator i = params.web_seeds.begin(), e = params.web_seeds.end();
				i != e; ++i)
		{
			HAL_DEV_MSG(wformat(L"URL: %1%") % (*i).url);
			t_info->add_url_seed(to_utf8((*i).url));
		}

		HAL_DEV_MSG(L"DHT Nodes");
		for (dht_node_details_t::const_iterator i = params.dht_nodes.begin(), e = params.dht_nodes.end();
				i != e; ++i)
		{
			HAL_DEV_MSG(wformat(L"URL: %1%, port: %2%") % (*i).url % (*i).port);
			t_info->add_node(hal::make_pair(to_utf8((*i).url), (*i).port));
		}

		// calculate the hash for all pieces
		int num = t_info->num_pieces();
		std::vector<char> piece_buf(piece_size);

		for (int i = 0; i < num; ++i)
		{
			store->read(&piece_buf[0], i, 0, t_info->piece_size(i));

			libt::hasher h(&piece_buf[0], t_info->piece_size(i));
			t_info->set_hash(i, h.final());

			if (fn(100*i / num, hal::app().res_wstr(HAL_NEWT_HASHING_PIECES)))
			{
				// User canceled torrent creation.

				hal::event_log.post(shared_ptr<hal::EventDetail>(
					new hal::EventMsg(hal::app().res_wstr(HAL_NEWT_CREATION_CANCELED), hal::event_logger::info)));

				return true;
			}
		}

		t_info->set_creator(to_utf8(params.creator).c_str());
		t_info->set_comment(to_utf8(params.comment).c_str());
		
		t_info->set_priv(params.private_torrent);

		// create the torrent and print it to out
		libt::entry e = t_info->create_torrent();
		halencode(out_file, e);
		}
		catch(const std::exception& e)
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventStdException(event_logger::fatal, e, L"create_torrent")));
		}	

		return false;
	}
	
	libt::session session_;	
	mutable mutex_t mutex_;

	asio::io_service io_;
	asio::deadline_timer timer_;
	bool keepChecking_;
	
	static wpath workingDirectory;
	ini_file bittorrentIni;
	TorrentManager the_torrents_;	
	
	int defTorrentMaxConn_;
	int defTorrentMaxUpload_;
	float defTorrentDownload_;
	float defTorrentUpload_;
	
	bool ip_filter_on_;
	bool ip_filter_loaded_;
	bool ip_filter_changed_;
	libt::ip_filter ip_filter_;
	size_t ip_filter_count_;
	
	void ip_filter_count();
	void ip_filter_load(progress_callback fn);
	void ip_filter_import(std::vector<libt::ip_range<asio::ip::address_v4> >& v4,
		std::vector<libt::ip_range<asio::ip::address_v6> >& v6);
	
	bool dht_on_;
	libt::dht_settings dht_settings_;
	libt::entry dht_state_;
	
};

}
