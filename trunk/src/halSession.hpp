
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
#include "halSignaler.hpp"

namespace hal
{

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
		catch(std::exception& e)
		{
			hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventStdException(event_logger::critical, e, L"~BitTorrent_impl"))); 
		}
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
				new EventMsg((wformat(hal::app().res_wstr(LBT_EVENT_TORRENT_FINISHED)) 
						% get(a.handle)->name()), 
					event_logger::info, a.timestamp())));
			
			get(a.handle)->finished();	
		}
		
		void operator()(libt::torrent_paused_alert const& a) const
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventMsg((wformat(hal::app().res_wstr(LBT_EVENT_TORRENT_PAUSED)) 
						% get(a.handle)->name()), 
					event_logger::info, a.timestamp())));

			get(a.handle)->signals().torrent_paused();
		}
		
		void operator()(libt::peer_error_alert const& a) const
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
					wformat(hal::app().res_wstr(HAL_PEER_ALERT))
						% hal::from_utf8_safe(a.msg())
						% hal::from_utf8_safe(a.ip.address().to_string()))
			)	);				
		}
			
		void operator()(libt::peer_ban_alert const& a) const
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
					wformat(hal::app().res_wstr(HAL_PEER_BAN_ALERT))
						% get(a.handle)->name()
						% hal::from_utf8_safe(a.ip.address().to_string()))
			)	);				
		}
			
		void operator()(libt::hash_failed_alert const& a) const
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
					wformat(hal::app().res_wstr(HAL_HASH_FAIL_ALERT))
						% get(a.handle)->name()
						% a.piece_index)
			)	);				
		}
			
		void operator()(libt::url_seed_alert const& a) const
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
					wformat(hal::app().res_wstr(HAL_URL_SEED_ALERT))
						% get(a.handle)->name()
						% hal::from_utf8_safe(a.url)
						% hal::from_utf8_safe(a.msg()))
			)	);				
		}
		
		void operator()(libt::tracker_warning_alert const& a) const
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
					wformat(hal::app().res_wstr(HAL_TRACKER_WARNING_ALERT))
						% get(a.handle)->name()
						% hal::from_utf8_safe(a.msg()))
			)	);				
		}
		
		void operator()(libt::tracker_announce_alert const& a) const
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventMsg((wformat(hal::app().res_wstr(HAL_TRACKER_ANNOUNCE_ALERT)) 
						% get(a.handle)->name()), 
					event_logger::info, a.timestamp())));
		}
		
		void operator()(libt::tracker_alert const& a) const
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
					wformat(hal::app().res_wstr(HAL_TRACKER_ALERT))
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
					wformat(hal::app().res_wstr(HAL_TRACKER_REPLY_ALERT))
						% get(a.handle)->name()
						% hal::from_utf8_safe(a.msg())
						% a.num_peers)
			)	);				
		}
		
		void operator()(libt::fastresume_rejected_alert const& a) const
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
					wformat(hal::app().res_wstr(HAL_FAST_RESUME_ALERT))
						% get(a.handle)->name()
						% hal::from_utf8_safe(a.msg()))
			)	);				
		}
		
		void operator()(libt::piece_finished_alert const& a) const
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventGeneral(event_logger::debug, a.timestamp(),
					wformat(hal::app().res_wstr(HAL_PIECE_FINISHED_ALERT))
						% get(a.handle)->name()
						% a.piece_index)
			)	);				
		}
		
		void operator()(libt::block_finished_alert const& a) const
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventGeneral(event_logger::debug, a.timestamp(),
					wformat(hal::app().res_wstr(HAL_BLOCK_FINISHED_ALERT))
						% get(a.handle)->name()
						% a.block_index
						% a.piece_index)
			)	);				
		}
		
		void operator()(libt::block_downloading_alert const& a) const
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventGeneral(event_logger::debug, a.timestamp(),
					wformat(hal::app().res_wstr(HAL_BLOCK_DOWNLOADING_ALERT))
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
						wformat(hal::app().res_wstr(HAL_LISTEN_FAILED_ALERT))
							% hal::from_utf8_safe(a.msg()))
				)	);
			}
		}
		
		void operator()(libt::listen_succeeded_alert const& a) const
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventGeneral(event_logger::info, a.timestamp(),
					wformat(hal::app().res_wstr(HAL_LISTEN_SUCCEEDED_ALERT))
						% hal::from_utf8_safe(a.msg()))
			)	);	

			bit_impl_.signals.successful_listen();
		}
		
		void operator()(libt::peer_blocked_alert const& a) const
		{
			event_log.post(shared_ptr<EventDetail>(
				new EventGeneral(event_logger::debug, a.timestamp(),
					wformat(hal::app().res_wstr(HAL_IPFILTER_ALERT))
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
	
	void saveTorrentData()
	{	
		mutex_t::scoped_lock l(mutex_);
		try
		{
		
		theTorrents.save();
		bittorrentIni.save_data();
			
		if (dht_on_) 
		{	
			halencode(workingDirectory/L"DHTState.bin", theSession.dht_state());
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
		theTorrents.load();
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
				wformat(L"Total %1%.") % theTorrents.size())));				
			
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

}
