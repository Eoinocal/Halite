
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#define TORRENT_MAX_ALERT_TYPES 20

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

#include "global/wtl_app.hpp"
#include "global/string_conv.hpp"
#include "global/ini_adapter.hpp"

#include "halTorrent.hpp"
#include "halTypes.hpp"
#include "halEvent.hpp"
#include "halSignaler.hpp"

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
	libtorrent::session* torrent_internal::the_session_ = 0;
	wpath torrent_internal::workingDir_;
}

namespace hal 
{

bit& bittorrent()
{
	static bit t;
	return t;
}

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

const PeerDetails& TorrentDetail::peerDetails() const
{
	if (!peerDetailsFilled_)
	{
		bittorrent().getAllPeerDetails(hal::to_utf8(name_), peerDetails_);
		peerDetailsFilled_ = true;
	}
	
	return peerDetails_;
}

const FileDetails& TorrentDetail::fileDetails() const
{
	if (!fileDetailsFilled_)
	{
		bittorrent().getAllFileDetails(hal::to_utf8(name_), fileDetails_);
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

web_seed_or_dht_node_detail::web_seed_or_dht_node_detail() : 
	url(L""), 
	port(-1), 
	type(hal::app().res_wstr(HAL_INT_NEWT_ADD_PEERS_WEB)) 
{}

web_seed_or_dht_node_detail::web_seed_or_dht_node_detail(std::wstring u) : 
	url(u), 
	port(-1), 
	type(hal::app().res_wstr(HAL_INT_NEWT_ADD_PEERS_WEB)) 
{}

web_seed_or_dht_node_detail::web_seed_or_dht_node_detail(std::wstring u, int p) : 
	url(u), 
	port(p), 
	type(hal::app().res_wstr(HAL_INT_NEWT_ADD_PEERS_DHT)) 
{}

class bit_impl
{
	friend class bit;
	
public:
	
	~bit_impl()
	{
		stopAlertHandler();
		
		//saveTorrentData();
		
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
		HAL_GENERIC_FN_EXCEPTION_CATCH(L"bit_impl::~bit_impl()")
	}

	struct 
	{
		signaler<> successful_listen;
		signaler<> torrent_finished;
	} 
	signals;

	void stopAlertHandler()
	{
		mutex_t::scoped_lock l(mutex_);

		keepChecking_ = false;
	}
		
	void alertHandler()
	{
		win32_exception::install_handler();

		try
		{
			mutex_t::scoped_lock l(mutex_);

			if (keepChecking_)
			{
			
			std::auto_ptr<libt::alert> p_alert = theSession.pop_alert();
			
			class AlertHandler
			{
			public:
			AlertHandler(bit_impl& bit_impl) :
				bit_impl_(bit_impl)
			{}
			
			void operator()(libt::torrent_finished_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventMsg((wform(hal::app().res_wstr(LBT_EVENT_TORRENT_FINISHED)) 
							% get(a.handle)->name()), 
						event_logger::info, a.timestamp())));
				
				get(a.handle)->finished();	
			}
			
			void operator()(libt::torrent_paused_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventMsg((wform(hal::app().res_wstr(LBT_EVENT_TORRENT_PAUSED)) 
							% get(a.handle)->name()), 
						event_logger::info, a.timestamp())));

				get(a.handle)->signals().torrent_paused();
			}
			
			void operator()(libt::peer_error_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
						wform(hal::app().res_wstr(HAL_PEER_ALERT))
							% hal::from_utf8_safe(a.msg())
							% hal::from_utf8_safe(a.ip.address().to_string()))
				)	);				
			}
				
			void operator()(libt::peer_ban_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
						wform(hal::app().res_wstr(HAL_PEER_BAN_ALERT))
							% get(a.handle)->name()
							% hal::from_utf8_safe(a.ip.address().to_string()))
				)	);				
			}
				
			void operator()(libt::hash_failed_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
						wform(hal::app().res_wstr(HAL_HASH_FAIL_ALERT))
							% get(a.handle)->name()
							% a.piece_index)
				)	);				
			}
				
			void operator()(libt::url_seed_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
						wform(hal::app().res_wstr(HAL_URL_SEED_ALERT))
							% get(a.handle)->name()
							% hal::from_utf8_safe(a.url)
							% hal::from_utf8_safe(a.msg()))
				)	);				
			}
			
			void operator()(libt::tracker_warning_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
						wform(hal::app().res_wstr(HAL_TRACKER_WARNING_ALERT))
							% get(a.handle)->name()
							% hal::from_utf8_safe(a.msg()))
				)	);				
			}
			
			void operator()(libt::tracker_announce_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventMsg((wform(hal::app().res_wstr(HAL_TRACKER_ANNOUNCE_ALERT)) 
							% get(a.handle)->name()), 
						event_logger::info, a.timestamp())));
			}
			
			void operator()(libt::tracker_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
						wform(hal::app().res_wstr(HAL_TRACKER_ALERT))
							% get(a.handle)->name()
							% hal::from_utf8_safe(a.msg())
							% a.times_in_row
							% a.status_code)
				)	);				
			}
			
			void operator()(libt::tracker_reply_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
						wform(hal::app().res_wstr(HAL_TRACKER_REPLY_ALERT))
							% get(a.handle)->name()
							% hal::from_utf8_safe(a.msg())
							% a.num_peers)
				)	);				
			}
			
			void operator()(libt::fastresume_rejected_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
						wform(hal::app().res_wstr(HAL_FAST_RESUME_ALERT))
							% get(a.handle)->name()
							% hal::from_utf8_safe(a.msg()))
				)	);				
			}
			
			void operator()(libt::piece_finished_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventGeneral(event_logger::debug, a.timestamp(),
						wform(hal::app().res_wstr(HAL_PIECE_FINISHED_ALERT))
							% get(a.handle)->name()
							% a.piece_index)
				)	);				
			}
			
			void operator()(libt::block_finished_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventGeneral(event_logger::debug, a.timestamp(),
						wform(hal::app().res_wstr(HAL_BLOCK_FINISHED_ALERT))
							% get(a.handle)->name()
							% a.block_index
							% a.piece_index)
				)	);				
			}
			
			void operator()(libt::block_downloading_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventGeneral(event_logger::debug, a.timestamp(),
						wform(hal::app().res_wstr(HAL_BLOCK_DOWNLOADING_ALERT))
							% get(a.handle)->name()
							% a.block_index
							% a.piece_index)
				)	);				
			}
			
			void operator()(libt::listen_failed_alert const& a) const
			{
				if (a.endpoint.address().is_v6())
				{	
					event_log.post(shared_ptr<EventDetail>(
						new EventGeneral(event_logger::info, a.timestamp(),
							hal::app().res_wstr(HAL_LISTEN_V6_FAILED_ALERT))
					)	);		
				}
				else
				{
					event_log.post(shared_ptr<EventDetail>(
						new EventGeneral(event_logger::info, a.timestamp(),
							wform(hal::app().res_wstr(HAL_LISTEN_FAILED_ALERT))
								% hal::from_utf8_safe(a.msg()))
					)	);
				}
			}
			
			void operator()(libt::listen_succeeded_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventGeneral(event_logger::info, a.timestamp(),
						wform(hal::app().res_wstr(HAL_LISTEN_SUCCEEDED_ALERT))
							% hal::from_utf8_safe(a.msg()))
				)	);	

				bit_impl_.signals.successful_listen();
			}
			
			void operator()(libt::peer_blocked_alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
					new EventGeneral(event_logger::debug, a.timestamp(),
						wform(hal::app().res_wstr(HAL_IPFILTER_ALERT))
							% hal::from_utf8_safe(a.ip.to_string())
							% hal::from_utf8_safe(a.msg()))
				)	);				
			}
			
			void operator()(libt::alert const& a) const
			{
				event_log.post(shared_ptr<EventDetail>(
						new EventLibtorrent(lbtAlertToHalEvent(a.severity()), 
							a.timestamp(), event_logger::unclassified, hal::from_utf8_safe(a.msg()))));		
			}
			
			private:
				bit_impl& bit_impl_;
				
				torrent_internal_ptr get(libt::torrent_handle h) const 
				{ 
					return bit_impl_.theTorrents.get(from_utf8_safe(h.get_torrent_info().name())); 
				}
			
			} handler(*this);
			
			while (p_alert.get())
			{	
				try
				{

					int* p = 0;
					int q = * p;
				
				libt::handle_alert<
					libt::torrent_finished_alert,
					libt::torrent_paused_alert,
					libt::peer_error_alert,
					libt::peer_ban_alert,
					libt::hash_failed_alert,
					libt::url_seed_alert,
					libt::tracker_alert,
					libt::tracker_warning_alert,
					libt::tracker_announce_alert,
					libt::tracker_reply_alert,
					libt::fastresume_rejected_alert,
					libt::piece_finished_alert,
					libt::block_finished_alert,
					libt::block_downloading_alert,
					libt::listen_failed_alert,
					libt::listen_succeeded_alert,
					libt::peer_blocked_alert,
					libt::alert
				>::handle_alert(p_alert, handler);			
				
				}
				catch(const access_violation& e)
				{
					throw e;
				}
				catch(const win32_exception& e)
				{
					throw e;
				}
				catch(libt::unhandled_alert&)
				{
					handler(*p_alert);
				}
				catch(std::exception& e)
				{
					// These are logged as debug because they are rarely important to act on!
					event_log.post(shared_ptr<EventDetail>(\
						new EventStdException(event_logger::debug, e, L"alertHandler")));
				}
				
				p_alert = theSession.pop_alert();
			}
			
			timer_.expires_from_now(boost::posix_time::seconds(2));
			timer_.async_wait(bind(&bit_impl::alertHandler, this));
			}

		}
		HAL_GENERIC_FN_EXCEPTION_CATCH(L"bit_impl::alertHandler()")
	}
	
	void saveTorrentData()
	{	
		mutex_t::scoped_lock l(mutex_);
		try
		{
		
		theTorrents.save_to_ini();
		bittorrentIni.save_data();
			
		if (dht_on_) 
		{	
			halencode(workingDirectory/L"DHTState.bin", theSession.dht_state());
		}
		
		}	
		HAL_GENERIC_FN_EXCEPTION_CATCH(L"bit_impl::saveTorrentData()")
	}
	
	int defTorrentMaxConn() { return defTorrentMaxConn_; }
	int defTorrentMaxUpload() { return defTorrentMaxUpload_; }
	float defTorrentDownload() { return defTorrentDownload_; }
	float defTorrentUpload() { return defTorrentUpload_; }
	
	const wpath workingDir() { return workingDirectory; };

private:
	bit_impl() :
		theSession(libt::fingerprint(HALITE_FINGERPRINT)),
		timer_(io_),
		keepChecking_(false),
		bittorrentIni(L"BitTorrent.xml"),
		theTorrents(bittorrentIni),
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
		try
		{
			torrent_internal::the_session_ = &theSession;
			torrent_internal::workingDir_ = workingDir();
			
			theSession.set_severity_level(libt::alert::debug);		
			theSession.add_extension(&libt::create_metadata_plugin);
			theSession.add_extension(&libt::create_ut_pex_plugin);
			theSession.set_max_half_open_connections(10);
			
			hal::event_log.post(shared_ptr<hal::EventDetail>(
				new hal::EventMsg(L"Loading BitTorrent.xml.", hal::event_logger::info)));		
			bittorrentIni.load_data();

			hal::event_log.post(shared_ptr<hal::EventDetail>(
				new hal::EventMsg(L"Loading torrent parameters.", hal::event_logger::info)));	
			theTorrents.load_from_ini();

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
				
				theTorrents = torrents;
				}
				
				event_log.post(shared_ptr<EventDetail>(new EventMsg(
					wform(L"Total %1%.") % theTorrents.size())));				
				
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
			
			{	libt::session_settings settings = theSession.settings();
				settings.user_agent = string("Halite ") + HALITE_VERSION_STRING;
				theSession.set_settings(settings);
			}
			
			timer_.expires_from_now(boost::posix_time::seconds(5));
			timer_.async_wait(bind(&bit_impl::alertHandler, this));
		}
		HAL_GENERIC_FN_EXCEPTION_CATCH(L"bit_impl::bit_impl()")
	}

	bool create_torrent(const create_torrent_params& params, fs::wpath out_file, progress_callback fn)
	{		
	try
	{
		boost::intrusive_ptr<libt::torrent_info> t_info(new libt::torrent_info);

		int piece_size = params.piece_size;
		HAL_DEV_MSG(wform(L"piece size: %1%") % piece_size);
		t_info->set_piece_size(piece_size);

		HAL_DEV_MSG(L"Files");
		for (file_size_pairs_t::const_iterator i = params.file_size_pairs.begin(), e = params.file_size_pairs.end();
				i != e; ++i)
		{
			HAL_DEV_MSG(wform(L"file path: %1%, size: %2%") % (*i).first % (*i).second);
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
			HAL_DEV_MSG(wform(L"URL: %1%, Tier: %2%") % (*i).url % (*i).tier);
			t_info->add_tracker(to_utf8((*i).url), (*i).tier);
		}

		HAL_DEV_MSG(L"Web Seeds");
		for (web_seed_details_t::const_iterator i = params.web_seeds.begin(), e = params.web_seeds.end();
				i != e; ++i)
		{
			HAL_DEV_MSG(wform(L"URL: %1%") % (*i).url);
			t_info->add_url_seed(to_utf8((*i).url));
		}

		HAL_DEV_MSG(L"DHT Nodes");
		for (dht_node_details_t::const_iterator i = params.dht_nodes.begin(), e = params.dht_nodes.end();
				i != e; ++i)
		{
			HAL_DEV_MSG(wform(L"URL: %1%, port: %2%") % (*i).url % (*i).port);
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
		HAL_GENERIC_FN_EXCEPTION_CATCH(L"bit_impl::create_torrent()")

		return false;
	}
	
	std::pair<libt::entry, libt::entry> prepTorrent(wpath filename, wpath saveDirectory);
	void removalThread(torrent_internal_ptr pIT, bool wipeFiles);
	
	libt::session theSession;	
	mutable mutex_t mutex_;

	asio::io_service io_;
	asio::deadline_timer timer_;
	bool keepChecking_;
	
	static wpath workingDirectory;
	ini_file bittorrentIni;
	TorrentManager theTorrents;	
	
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

wpath bit_impl::workingDirectory = hal::app().working_directory();

bit::bit() :
	pimpl(new bit_impl())
{}

#define HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH(FUNCTION) \
catch (const libt::invalid_handle&) \
{\
	event_log.post(shared_ptr<EventDetail>( \
		new EventInvalidTorrent(event_logger::critical, event_logger::invalidTorrent, name, std::string(FUNCTION)))); \
}\
catch (const invalidTorrent& t) \
{ \
	event_log.post(shared_ptr<EventDetail>( \
		new EventInvalidTorrent(event_logger::info, event_logger::invalidTorrent, t.who(), std::string(FUNCTION)))); \
} \
catch (const access_violation& e) \
{ \
	hal::event_log.post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"Torrent property %1% access_violation (code %2$x) at %3$x. Bad address %4$x") % hal::from_utf8(FUNCTION) % e.code() % (unsigned)e.where() % (unsigned)e.badAddress(), \
			hal::event_logger::dev))); \
} \
catch (const win32_exception& e) \
{ \
	hal::event_log.post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"Torrent property %1% win32_exception (code %2$x) at %3$x") % hal::from_utf8(FUNCTION) % e.code() % (unsigned)e.where(), \
			hal::event_logger::dev))); \
} \
catch (const std::exception& e) \
{ \
	event_log.post(shared_ptr<EventDetail>( \
		new EventTorrentException(event_logger::critical, event_logger::torrentException, std::string(e.what()), name, std::string(FUNCTION)))); \
} \
catch(...) \
{ \
	hal::event_log.post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"%1% catch all") % hal::from_utf8(FUNCTION), \
			hal::event_logger::dev))); \
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
catch (const access_violation& e) \
{ \
	hal::event_log.post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"Generic Torrent %1% access_violation (code %2$x) at %3$x. Bad address %4$x (%5%)") % hal::from_utf8(FUNCTION) % e.code() % (unsigned)e.where() % (unsigned)e.badAddress() % TORRENT, \
			hal::event_logger::dev))); \
} \
catch (const win32_exception& e) \
{ \
	hal::event_log.post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"Generic Torrent %1% win32_exception (code %2$x) at %3$x (%4%)") % hal::from_utf8(FUNCTION) % e.code() % (unsigned)e.where() % TORRENT, \
			hal::event_logger::dev))); \
} \
catch (const std::exception& e) \
{ \
	event_log.post(shared_ptr<EventDetail>( \
		new EventTorrentException(event_logger::critical, event_logger::torrentException, std::string(e.what()), TORRENT, std::string(FUNCTION)))); \
} \
catch (...) \
{ \
	hal::event_log.post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"Generic Torrent %1% catch all") % hal::from_utf8(FUNCTION), \
			hal::event_logger::dev))); \
}

void bit::shutDownSession()
{
	pimpl.reset();
}

void bit::saveTorrentData()
{
	pimpl->saveTorrentData();
}

bool bit::create_torrent(const create_torrent_params& params, fs::wpath out_file, progress_callback fn)
{
	return pimpl->create_torrent(params, out_file, fn);
}

bit::torrent bit::get_wstr(const std::wstring& filename)
{
	return bit::torrent(pimpl->theTorrents.get(filename));
}

bool bit::listenOn(std::pair<int, int> const& range)
{
	try
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
		{
			pimpl->signals.successful_listen();
			
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

int bit::isListeningOn() 
{
	if (!pimpl->theSession.is_listening())
		return -1;	
	else
		return pimpl->theSession.listen_port();
}

void bit::stopListening()
{
	ensureDhtOff();
	pimpl->theSession.listen_on(std::make_pair(0, 0));
}

bool bit::ensureDhtOn()
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

void bit::ensureDhtOff()
{
	if (pimpl->dht_on_)
	{
		pimpl->theSession.stop_dht();		
		pimpl->dht_on_ = false;
	}
}

void bit::setDhtSettings(int max_peers_reply, int search_branching, 
	int service_port, int max_fail_count)
{
	libt::dht_settings settings;
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

void bit::setMapping(int mapping)
{
	if (mapping != mappingNone)
	{
		if (mapping == mappingUPnP)
		{
			event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Starting UPnP mapping.")));
			pimpl->theSession.stop_upnp();
			pimpl->theSession.stop_natpmp();

			pimpl->signals.successful_listen.connect_once(bind(&libt::session::start_upnp, &pimpl->theSession));
		}
		else
		{
			event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Starting NAT-PMP mapping.")));
			pimpl->theSession.stop_upnp();
			pimpl->theSession.stop_natpmp();

			pimpl->signals.successful_listen.connect_once(bind(&libt::session::start_natpmp, &pimpl->theSession));
		}
	}
	else
	{
		event_log.post(shared_ptr<EventDetail>(new EventMsg(L"No mapping.")));
		pimpl->theSession.stop_upnp();
		pimpl->theSession.stop_natpmp();
	}
}

void bit::setTimeouts(int peers, int tracker)
{
	libt::session_settings settings = pimpl->theSession.settings();
	settings.peer_connect_timeout = peers;
	settings.tracker_completion_timeout = tracker;

	pimpl->theSession.set_settings(settings);

	event_log.post(shared_ptr<EventDetail>(new EventMsg(
		wform(L"Set Timeouts, peer %1%, tracker %2%") % peers % tracker)));
}

void bit::setSessionLimits(int maxConn, int maxUpload)
{		
	pimpl->theSession.set_max_uploads(maxUpload);
	pimpl->theSession.set_max_connections(maxConn);
	
	event_log.post(shared_ptr<EventDetail>(new EventMsg(
		wform(L"Set connections totals %1% and uploads %2%.") 
			% maxConn % maxUpload)));
}

void bit::setSessionSpeed(float download, float upload)
{
	int down = (download > 0) ? static_cast<int>(download*1024) : -1;
	pimpl->theSession.set_download_rate_limit(down);
	int up = (upload > 0) ? static_cast<int>(upload*1024) : -1;
	pimpl->theSession.set_upload_rate_limit(up);
	
	event_log.post(shared_ptr<EventDetail>(new EventMsg(
		wform(L"Set session rates at download %1% and upload %2%.") 
			% pimpl->theSession.download_rate_limit() % pimpl->theSession.upload_rate_limit())));
}

void bit_impl::ip_filter_count()
{
	libt::ip_filter::filter_tuple_t vectors = ip_filter_.export_filter();
	
	vectors.get<0>().erase(std::remove(vectors.get<0>().begin(), vectors.get<0>().end(), 0),
		vectors.get<0>().end());
	vectors.get<1>().erase(std::remove(vectors.get<1>().begin(), vectors.get<1>().end(), 0),
		vectors.get<1>().end());
	ip_filter_count_ = vectors.get<0>().size() + vectors.get<1>().size();
}

void bit_impl::ip_filter_load(progress_callback fn)
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

				if (fn) if (fn(size_t(i/total), hal::app().res_wstr(HAL_TORRENT_LOAD_FILTERS))) break;
			}
			
			read_range_to_filter<asio::ip::address_v4>(ifs, ip_filter_);
		}
	}	
}

void  bit_impl::ip_filter_import(std::vector<libt::ip_range<asio::ip::address_v4> >& v4,
	std::vector<libt::ip_range<asio::ip::address_v6> >& v6)
{
	for(std::vector<libt::ip_range<asio::ip::address_v4> >::iterator i=v4.begin();
		i != v4.end(); ++i)
	{
		ip_filter_.add_rule(i->first, i->last, libt::ip_filter::blocked);
	}
/*	for(std::vector<libt::ip_range<asio::ip::address_v6> >::iterator i=v6.begin();
		i != v6.end(); ++i)
	{
		ip_filter_.add_rule(i->first, i->last, libt::ip_filter::blocked);
	}
*/	
	/* Note here we do not set ip_filter_changed_ */
}

bool bit::ensureIpFilterOn(progress_callback fn)
{
	try
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
	HAL_GENERIC_FN_EXCEPTION_CATCH(L"bit::ensureIpFilterOn")

	event_log.post(shared_ptr<EventDetail>(new EventMsg(L"IP filters on.")));	

	return false;
}

void bit::ensureIpFilterOff()
{
	pimpl->theSession.set_ip_filter(libt::ip_filter());
	pimpl->ip_filter_on_ = false;
	
	event_log.post(shared_ptr<EventDetail>(new EventMsg(L"IP filters off.")));	
}

#ifndef TORRENT_DISABLE_ENCRYPTION	
void bit::ensurePeOn(int enc_level, int in_enc_policy, int out_enc_policy, bool prefer_rc4)
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
					(wform(hal::app().res_wstr(HAL_INCORRECT_ENCODING_LEVEL)) % enc_level).str())));
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
					(wform(hal::app().res_wstr(HAL_INCORRECT_CONNECT_POLICY)) % in_enc_policy).str())));
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
					(wform(hal::app().res_wstr(HAL_INCORRECT_CONNECT_POLICY)) % in_enc_policy).str())));
	}
	
	pe.prefer_rc4 = prefer_rc4;
	
	try
	{
	
	pimpl->theSession.set_pe_settings(pe);
	
	}
	HAL_GENERIC_FN_EXCEPTION_CATCH(L"bit::ensurePeOn")
	
	event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Protocol encryption on.")));
}

void bit::ensurePeOff()
{
	libt::pe_settings pe;
	pe.out_enc_policy = libt::pe_settings::disabled;
	pe.in_enc_policy = libt::pe_settings::disabled;
	
	pe.allowed_enc_level = libt::pe_settings::both;
	pe.prefer_rc4 = true;
	
	pimpl->theSession.set_pe_settings(pe);

	event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Protocol encryption off.")));
}
#endif

void bit::ip_v4_filter_block(asio::ip::address_v4 first, asio::ip::address_v4 last)
{
	pimpl->ip_filter_.add_rule(first, last, libt::ip_filter::blocked);
	pimpl->ip_filter_count();
	pimpl->ip_filter_changed_ = true;
}

void bit::ip_v6_filter_block(asio::ip::address_v6 first, asio::ip::address_v6 last)
{
	pimpl->ip_filter_.add_rule(first, last, libt::ip_filter::blocked);
	pimpl->ip_filter_count();
	pimpl->ip_filter_changed_ = true;
}

size_t bit::ip_filter_size()
{
	return pimpl->ip_filter_count_;
}

void bit::clearIpFilter()
{
	pimpl->ip_filter_ = libt::ip_filter();
	pimpl->theSession.set_ip_filter(libt::ip_filter());	
	pimpl->ip_filter_changed_ = true;
	pimpl->ip_filter_count();
}

bool bit::ip_filter_import_dat(boost::filesystem::path file, progress_callback fn, bool octalFix)
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
					if (fn(size_t(progress/total), hal::app().res_wstr(HAL_TORRENT_IMPORT_FILTERS))) 
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
					asio::ip::address_v4::from_string(last), libt::ip_filter::blocked);	
				}
				catch(...)
				{
					hal::event_log.post(shared_ptr<hal::EventDetail>(
						new hal::EventDebug(hal::event_logger::info, 
						from_utf8((boost::format("Invalid IP range: %1%-%2%.") % first % last).str()))));
				}
			}
		}
	}
	
	pimpl->ip_filter_changed_ = true;
	pimpl->ip_filter_count();
	
	}
	HAL_GENERIC_FN_EXCEPTION_CATCH(L"bit::ip_filter_import_dat()")

	return false;
}

const SessionDetail bit::getSessionDetails()
{
	SessionDetail details;
	
	details.port = pimpl->theSession.is_listening() ? pimpl->theSession.listen_port() : -1;
	
	libt::session_status status = pimpl->theSession.status();
	
	details.speed = std::pair<double, double>(status.download_rate, status.upload_rate);
	
	details.dht_on = pimpl->dht_on_;
	details.dht_nodes = status.dht_nodes;
	details.dht_torrents = status.dht_torrents;
	
	details.ip_filter_on = pimpl->ip_filter_on_;
	details.ip_ranges_filtered = pimpl->ip_filter_count_;
	
	return details;
}

void bit::setSessionHalfOpenLimit(int halfConn)
{
	pimpl->theSession.set_max_half_open_connections(halfConn);

	event_log.post(shared_ptr<EventDetail>(new EventMsg(
		wform(L"Set half-open connections limit to %1%.") % pimpl->theSession.max_half_open_connections())));
}

void bit::setTorrentDefaults(int maxConn, int maxUpload, float download, float upload)
{
	pimpl->defTorrentMaxConn_ = maxConn;
	pimpl->defTorrentMaxUpload_ = maxUpload;

	event_log.post(shared_ptr<EventDetail>(new EventMsg(
		wform(L"Set torrent connections total %1% and uploads %2%.") % maxConn % maxUpload)));

	pimpl->defTorrentDownload_ = download;
	pimpl->defTorrentUpload_ = upload;

	event_log.post(shared_ptr<EventDetail>(new EventMsg(
		wform(L"Set torrent default rates at %1$.2fkb/s down and %2$.2fkb/s upload.") % download % upload)));
}

std::pair<libt::entry, libt::entry> bit_impl::prepTorrent(wpath filename, wpath saveDirectory)
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

void bit::addTorrent(wpath file, wpath saveDirectory, bool startStopped, bool compactStorage, 
		boost::filesystem::wpath moveToDirectory, bool useMoveTo) 
{
	try 
	{	
	torrent_internal_ptr TIp;

	std::pair<std::string, std::string> names = extract_names(file);
	wstring xml_name = from_utf8(names.first) + L".xml";

	// Commented out purposely!
	if (false && fs::exists(file.branch_path()/xml_name))
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
		pimpl->theTorrents.insert(TIp);
	
	if (p.second)
	{
		torrent_internal_ptr me = pimpl->theTorrents.get(TIp->name());		
		
		if (!startStopped) 
			me->add_to_session();
		else
			me->set_state_stopped();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(file.string(), "addTorrent")
}

const TorrentDetails& bit::torrentDetails()
{
	return torrentDetails_;
}

const TorrentDetails& bit::updateTorrentDetails(const wstring& focused, const std::set<wstring>& selected)
{
	try {
	
	mutex_t::scoped_lock l(torrentDetails_.mutex_);	
	
	torrentDetails_.clearAll(l);	
	torrentDetails_.torrents_.reserve(pimpl->theTorrents.size());
	
	for (TorrentManager::torrentByName::iterator i=pimpl->theTorrents.begin(), e=pimpl->theTorrents.end(); i != e; ++i)
	{
		wstring utf8Name = (*i).torrent->name();
		TorrentDetail_ptr pT = (*i).torrent->getTorrentDetail_ptr();
		
		if (selected.find(utf8Name) != selected.end())
		{
			torrentDetails_.selectedTorrents_.push_back(pT);
		}
		
		if (focused == utf8Name)
			torrentDetails_.selectedTorrent_ = pT;
		
		torrentDetails_.torrentMap_[(*i).torrent->name()] = pT;
		torrentDetails_.torrents_.push_back(pT);
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "updateTorrentDetails")
	
	return torrentDetails_;
}

void bit::resumeAll()
{
	try {
		
	event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Resuming torrent.")));
	
	for (TorrentManager::torrentByName::iterator i=pimpl->theTorrents.begin(), e=pimpl->theTorrents.end(); i != e;)
	{
		wpath file = wpath(pimpl->workingDirectory)/L"torrents"/(*i).torrent->filename();
		
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
				
				pimpl->theTorrents.erase(i++);
			}			
		}
		else
		{
			pimpl->theTorrents.erase(i++);
		}
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "resumeAll")
}

void bit::closeAll(boost::optional<report_num_active> fn)
{
	try 
	{	
	event_log.post(shared_ptr<EventDetail>(new EventInfo(L"Saving torrent data...")));

	pimpl->saveTorrentData();

	event_log.post(shared_ptr<EventDetail>(new EventInfo(L"Stopping all torrents...")));
	
	for (TorrentManager::torrentByName::iterator i=pimpl->theTorrents.begin(), e=pimpl->theTorrents.end(); 
		i != e; ++i)
	{
		(*i).torrent->stop();
	}
	
	// Ok this polling loop here is a bit curde, but a blocking wait is actually appropiate.
	for (int num_active = -1; num_active != 0; )
	{
		num_active = 0;

		for (TorrentManager::torrentByName::iterator i=pimpl->theTorrents.begin(), e=pimpl->theTorrents.end(); 
				i != e; ++i)
		{
			if ((*i).torrent->state() != TorrentDetail::torrent_stopped)
				++num_active;
		}
		
		event_log.post(shared_ptr<EventDetail>(new EventInfo(wform(L"%1% still active") % num_active)));

		if (fn)	(*fn)(num_active);
		Sleep(200);
	}
	
	event_log.post(shared_ptr<EventDetail>(new EventInfo(L"All torrents stopped.")));		
	event_log.post(shared_ptr<EventDetail>(new EventInfo(L"Fast-resume data written.")));
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "closeAll")
}

PeerDetail::PeerDetail(libt::peer_info& peerInfo) :
	ipAddress(hal::from_utf8_safe(peerInfo.ip.address().to_string())),
	country(L""),
	speed(std::make_pair(peerInfo.payload_down_speed, peerInfo.payload_up_speed)),
	client(hal::from_utf8_safe(peerInfo.client))
{
	std::vector<wstring> status_vec;
	
#ifndef TORRENT_DISABLE_RESOLVE_COUNTRIES
	if (peerInfo.country[0] != 0 && peerInfo.country[1] != 0)
		country = (wform(L"(%1%)") % hal::from_utf8_safe(string(peerInfo.country, 2))).str().c_str();
#endif	

	if (peerInfo.flags & libt::peer_info::handshake)
	{
		status_vec.push_back(app().res_wstr(HAL_PEER_HANDSHAKE));
	}		
	else if (peerInfo.flags & libt::peer_info::connecting)
	{
		status_vec.push_back(app().res_wstr(HAL_PEER_CONNECTING));
	}
	else
	{
	#ifndef TORRENT_DISABLE_ENCRYPTION		
		if (peerInfo.flags & libt::peer_info::rc4_encrypted)
			status_vec.push_back(app().res_wstr(HAL_PEER_RC4_ENCRYPTED));		
		if (peerInfo.flags & libt::peer_info::plaintext_encrypted)
			status_vec.push_back(app().res_wstr(HAL_PEER_PLAINTEXT_ENCRYPTED));
	#endif
		
		if (peerInfo.flags & libt::peer_info::interesting)
			status_vec.push_back(app().res_wstr(HAL_PEER_INTERESTING));	
		if (peerInfo.flags & libt::peer_info::choked)
			status_vec.push_back(app().res_wstr(HAL_PEER_CHOKED));	
		if (peerInfo.flags & libt::peer_info::remote_interested)
			status_vec.push_back(app().res_wstr(HAL_PEER_REMOTE_INTERESTING));	
		if (peerInfo.flags & libt::peer_info::remote_choked)
			status_vec.push_back(app().res_wstr(HAL_PEER_REMOTE_CHOKED));	
		if (peerInfo.flags & libt::peer_info::supports_extensions)
			status_vec.push_back(app().res_wstr(HAL_PEER_SUPPORT_EXTENSIONS));	
	//	if (peerInfo.flags & libt::peer_info::local_connection)						// Not sure whats up here?
	//		status_vec.push_back(app().res_wstr(HAL_PEER_LOCAL_CONNECTION));			
		if (peerInfo.flags & libt::peer_info::queued)
			status_vec.push_back(app().res_wstr(HAL_PEER_QUEUED));
	}
	
	seed = (peerInfo.flags & libt::peer_info::seed) ? true : false;
	
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

void bit::getAllPeerDetails(const std::string& filename, PeerDetails& peerContainer)
{
	getAllPeerDetails(from_utf8_safe(filename), peerContainer);
}

void bit::getAllPeerDetails(const std::wstring& filename, PeerDetails& peerContainer)
{
	try {
	
	pimpl->theTorrents.get(filename)->getPeerDetails(peerContainer);
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "getAllPeerDetails")
}

void bit::getAllFileDetails(const std::string& filename, FileDetails& fileDetails)
{
	getAllFileDetails(from_utf8_safe(filename), fileDetails);
}

void bit::getAllFileDetails(const std::wstring& filename, FileDetails& fileDetails)
{
	try {
	
	pimpl->theTorrents.get(filename)->getFileDetails(fileDetails);
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "getAllFileDetails")
}

bool bit::isTorrent(const std::string& filename)
{	
	return isTorrent(hal::to_wstr_shim(filename));
}

bool bit::isTorrent(const std::wstring& filename)
{	
	try {
	
	return pimpl->theTorrents.exists(filename);
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "isTorrent")
	
	return false;
}

void bit::pauseTorrent(const std::string& filename)
{
	pauseTorrent(hal::to_wstr_shim(filename));
}

void bit::pauseTorrent(const std::wstring& filename)
{
	try {
	
	pimpl->theTorrents.get(filename)->pause();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "pauseTorrent")
}

void bit::resumeTorrent(const std::string& filename)
{
	resumeTorrent(hal::to_wstr_shim(filename));
}

void bit::resumeTorrent(const std::wstring& filename)
{
	try {
	
	pimpl->theTorrents.get(filename)->resume();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "resumeTorrent")
}

void bit::stopTorrent(const std::string& filename)
{
	stopTorrent(hal::to_wstr_shim(filename));
}

void bit::stopTorrent(const std::wstring& filename)
{
	try {
	
	pimpl->theTorrents.get(filename)->stop();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "stopTorrent")
}

bool bit::isTorrentActive(const std::string& filename)
{
	return isTorrentActive(hal::to_wstr_shim(filename));
}

bool bit::isTorrentActive(const std::wstring& filename)
{
	try {
	
	return pimpl->theTorrents.get(filename)->is_active();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "isTorrentActive")
	
	return false; // ??? is this correct
}

void bit::reannounceTorrent(const std::string& filename)
{
	reannounceTorrent(hal::to_wstr_shim(filename));
}

void bit::reannounceTorrent(const std::wstring& filename)
{
	try {
	
	pimpl->theTorrents.get(filename)->handle().force_reannounce();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "reannounceTorrent")
}

void bit_impl::removalThread(torrent_internal_ptr pIT, bool wipeFiles)
{
	try {

	if (!wipeFiles)
	{
		theSession.remove_torrent(pIT->handle());
	}
	else
	{
		if (pIT->in_session())
		{
			theSession.remove_torrent(pIT->handle(), libt::session::delete_files);
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

void bit::removeTorrent(const std::string& filename)
{
	removeTorrent(hal::to_wstr_shim(filename));
}

void bit::removeTorrent(const std::wstring& filename)
{
	try {
	
	torrent_internal_ptr pTI = pimpl->theTorrents.get(filename);
	libt::torrent_handle handle = pTI->handle();
	pimpl->theTorrents.erase(filename);
	
	thread_t t(bind(&bit_impl::removalThread, &*pimpl, pTI, false));	
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "removeTorrent")
}

void bit::recheckTorrent(const std::string& filename)
{
	recheckTorrent(hal::to_wstr_shim(filename));
}

void bit::recheckTorrent(const std::wstring& filename)
{
	try {
	
	pimpl->theTorrents.get(filename)->force_recheck();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "recheckTorrent")
}

void bit::removeTorrentWipeFiles(const std::string& filename)
{
	removeTorrentWipeFiles(hal::to_wstr_shim(filename));
}

void bit::removeTorrentWipeFiles(const std::wstring& filename)
{
	try {
	
	torrent_internal_ptr pTI = pimpl->theTorrents.get(filename);
	libt::torrent_handle handle = pTI->handle();
	pimpl->theTorrents.erase(filename);
	
	thread_t t(bind(&bit_impl::removalThread, &*pimpl, pTI, true));	
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "removeTorrentWipeFiles")
}

void bit::pauseAllTorrents()
{	
	try {
	
	for (TorrentManager::torrentByName::iterator i=pimpl->theTorrents.begin(), e=pimpl->theTorrents.end();
		i != e; ++i)
	{		
		if ((*i).torrent->in_session())
			(*i).torrent->pause();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "pauseAllTorrents")
}

void bit::unpauseAllTorrents()
{	
	try {
	
	for (TorrentManager::torrentByName::iterator i=pimpl->theTorrents.begin(), e=pimpl->theTorrents.end();
		i != e; ++i)
	{
		if ((*i).torrent->in_session() && (*i).torrent->state() == TorrentDetail::torrent_paused)
			(*i).torrent->resume();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "unpauseAllTorrents")
}

bit::torrent::torrent()
{}

bit::torrent::torrent(boost::shared_ptr<torrent_internal> p) :
	ptr(p)
{}

bool bit::torrent::is_open() const
{
	return ptr;
}

bit::torrent::exec_around_ptr::proxy::proxy(torrent_internal* t) : 
	t_(t),
	l_(t->mutex_)
{
	HAL_DEV_MSG(L"Ctor proxy");
}

bit::torrent::exec_around_ptr::proxy::~proxy() 
{
	HAL_DEV_MSG(L"Dtor proxy");
}

const std::wstring bit::torrent::get_name() const
{
	try {
	
	return ptr->name();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(L"Torrent Unknown", "torrent::get_name()")
	
	return 0;
}

float bit::torrent::get_ratio() const
{
	try {
	
	return ptr->get_ratio();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_ratio")
	
	return 0;
}

void bit::torrent::set_ratio(float r)
{
	try {

	ptr->set_ratio(r);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_ratio")
}

std::pair<int, int> bit::torrent::get_connection_limits() const
{
	try {
	
	return ptr->getConnectionLimit();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_connection_limits")
	
	return std::make_pair(-1, -1);
}

void bit::torrent::set_connection_limits(const std::pair<int, int>& l)
{
	try {
	
	ptr->setConnectionLimit(l.first, l.second);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_connection_limits")
}

std::pair<float, float> bit::torrent::get_rate_limits() const
{
	try {
	
	return ptr->getTransferSpeed();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_rate_limits")
	
	return std::pair<float, float>(-1.0, -1.0);
}

void bit::torrent::set_rate_limits(const std::pair<float, float>& l)
{
	try {
	
	ptr->setTransferSpeed(l.first, l.second);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_rate_limits")
}

wpath bit::torrent::get_save_directory() const
{
	try {
	
	return ptr->get_save_directory();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_save_directory")
	
	return L"";
}

void bit::torrent::set_save_directory(const wpath& s)
{
	try {
	
	ptr->set_save_directory(s);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_save_directory")
}

wpath bit::torrent::get_move_to_directory() const
{
	try {
	
	return ptr->get_move_to_directory();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_save_directory")
	
	return L"";
}

void bit::torrent::set_move_to_directory(const wpath& m)
{
	try {
	
	ptr->set_move_to_directory(m);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_move_to_directory")
}

std::pair<wstring, wstring> bit::torrent::get_tracker_login() const
{
	try {
	
	return ptr->getTrackerLogin();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("get_tracker_login")
	
	return std::make_pair(L"!!! exception thrown !!!", L"!!! exception thrown !!!");
}

void bit::torrent::set_tracker_login(const std::pair<wstring, wstring>& p)
{
	try {
	
	ptr->setTrackerLogin(p.first, p.second);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_tracker_login")
}

bool bit::torrent::get_is_active() const
{
	try {
	
	return ptr->is_active();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_is_active")
	
	return L"";
}

bool bit::torrent::get_in_session() const
{
	try {
	
	return ptr->in_session();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_in_session")
	
	return L"";
}

std::vector<tracker_detail> bit::torrent::get_trackers() const
{
	try {
	
	return ptr->getTrackers();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_trackers")
	
	return std::vector<tracker_detail>();
}

void bit::torrent::set_trackers(const std::vector<tracker_detail>& trackers)
{
	try {
	
	ptr->setTrackers(trackers);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_trackers")
}

void bit::torrent::reset_trackers()
{
	try {
	
	ptr->resetTrackers();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_trackers")
}

void bit::torrent::set_file_priorities(const std::pair<std::vector<int>, int>& p)
{
	try { 

	ptr->setFilePriorities(p.first, p.second);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_trackers")
}

void bit::startEventReceiver()
{
	pimpl->keepChecking_ = true;
	thread_t(bind(&asio::io_service::run, &pimpl->io_));
}

void bit::stopEventReceiver()
{
	event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Stopping event Handler.")));

	pimpl->stopAlertHandler();
}

int bit::defTorrentMaxConn() { return pimpl->defTorrentMaxConn_; }
int bit::defTorrentMaxUpload() { return pimpl->defTorrentMaxUpload_; }
float bit::defTorrentDownload() { return pimpl->defTorrentDownload_; }
float bit::defTorrentUpload() { return pimpl->defTorrentUpload_; }
	
};
