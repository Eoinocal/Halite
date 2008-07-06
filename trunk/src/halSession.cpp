
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#define TORRENT_MAX_ALERT_TYPES 32

#include <libtorrent/alert_types.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/ip_filter.hpp>
#include <libtorrent/torrent_handle.hpp>

#include "global/wtl_app.hpp"
#include "global/string_conv.hpp"
#include "global/ini_adapter.hpp"

#include "halIni.hpp"
#include "halTypes.hpp"
#include "halEvent.hpp"
#include "halSignaler.hpp"
#include "halSession.hpp"

namespace hal
{

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

bool bit_impl::ip_filter_import_dat(boost::filesystem::path file, progress_callback fn, bool octalFix)
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
				ip_filter_.add_rule(asio::ip::address_v4::from_string(first),
					asio::ip::address_v4::from_string(last), libt::ip_filter::blocked);	
				}
				catch(...)
				{
					hal::event_log.post(shared_ptr<hal::EventDetail>(
						new hal::EventDebug(hal::event_logger::info, 
							from_utf8((format("Invalid IP range: %1%-%2%.") % first % last).str()))));
				}
			}
		}
	}
	
	ip_filter_changed_ = true;
	ip_filter_count();
	
	}
	catch(const std::exception& e)
	{
		event_log.post(shared_ptr<EventDetail>(
			new EventStdException(event_logger::critical, e, L"ip_filter_import_dat")));
	}

	return false;
}

void bit_impl::stop_alert_handler()
{
	mutex_t::scoped_lock l(mutex_);

	keepChecking_ = false;
}
	
void bit_impl::alert_handler()
{
	mutex_t::scoped_lock l(mutex_);

	if (keepChecking_)
	{
	
	std::auto_ptr<libt::alert> p_alert = session_.pop_alert();
	
	class AlertHandler
	{
	public:
	AlertHandler(bit_impl& bit_impl) :
		bit_impl_(bit_impl)
	{}

	void operator()(libt::external_ip_alert const& a) const
	{
		event_log.post(shared_ptr<EventDetail>(
			new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
				wformat(hal::app().res_wstr(HAL_EXTERNAL_IP_ALERT))
					% hal::from_utf8_safe(a.message())
					% hal::from_utf8_safe(a.external_address.to_string()))
		)	);				
	}

	void operator()(libt::portmap_error_alert const& a) const
	{
		event_log.post(shared_ptr<EventDetail>(
			new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
				wformat(hal::app().res_wstr(HAL_PORTMAP_ERROR_ALERT))
				% (a.type == 0 ? 
					hal::app().res_wstr(HAL_PORTMAP_TYPE_PMP) : 
					hal::app().res_wstr(HAL_PORTMAP_TYPE_UPNP)))
		)	);				
	}

	void operator()(libt::portmap_alert const& a) const
	{
		event_log.post(shared_ptr<EventDetail>(
			new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
				wformat(hal::app().res_wstr(HAL_PORTMAP_ALERT))
				% (a.type == 0 ? 
					hal::app().res_wstr(HAL_PORTMAP_TYPE_PMP) : 
					hal::app().res_wstr(HAL_PORTMAP_TYPE_UPNP))
				% a.external_port)
		)	);				
	}
	
	void operator()(libt::file_error_alert const& a) const
	{
		event_log.post(shared_ptr<EventDetail>(
			new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
				wformat(hal::app().res_wstr(HAL_FILE_ERROR_ALERT))
				% hal::from_utf8_safe(a.file)
				% hal::from_utf8_safe(a.msg))
		)	);				
	}
	
	void operator()(libt::dht_reply_alert const& a) const
	{
		event_log.post(shared_ptr<EventDetail>(
			new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
				wformat(hal::app().res_wstr(HAL_DHT_REPLY_ALERT))
					% a.num_peers)
		)	);				
	}

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
					% hal::from_utf8_safe(a.message())
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
					% hal::from_utf8_safe(a.message()))
		)	);				
	}
	
	void operator()(libt::tracker_warning_alert const& a) const
	{
		event_log.post(shared_ptr<EventDetail>(
			new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
				wformat(hal::app().res_wstr(HAL_TRACKER_WARNING_ALERT))
					% get(a.handle)->name()
					% hal::from_utf8_safe(a.message()))
		)	);				
	}
	
	void operator()(libt::tracker_announce_alert const& a) const
	{
		event_log.post(shared_ptr<EventDetail>(
			new EventMsg((wformat(hal::app().res_wstr(HAL_TRACKER_ANNOUNCE_ALERT)) 
					% get(a.handle)->name()), 
				event_logger::info, a.timestamp())));
	}
	
	void operator()(libt::tracker_error_alert const& a) const
	{
		event_log.post(shared_ptr<EventDetail>(
			new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
				wformat(hal::app().res_wstr(HAL_TRACKER_ALERT))
					% get(a.handle)->name()
					% hal::from_utf8_safe(a.message())
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
					% hal::from_utf8_safe(a.message())
					% a.num_peers)
		)	);				
	}
	
	void operator()(libt::fastresume_rejected_alert const& a) const
	{
		event_log.post(shared_ptr<EventDetail>(
			new EventGeneral(lbtAlertToHalEvent(a.severity()), a.timestamp(),
				wformat(hal::app().res_wstr(HAL_FAST_RESUME_ALERT))
					% get(a.handle)->name()
					% hal::from_utf8_safe(a.message()))
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
						% hal::from_utf8_safe(a.message()))
			)	);
		}
	}
	
	void operator()(libt::listen_succeeded_alert const& a) const
	{
		event_log.post(shared_ptr<EventDetail>(
			new EventGeneral(event_logger::info, a.timestamp(),
				wformat(hal::app().res_wstr(HAL_LISTEN_SUCCEEDED_ALERT))
					% hal::from_utf8_safe(a.message()))
		)	);	

		bit_impl_.signals.successful_listen();
	}
	
	void operator()(libt::peer_blocked_alert const& a) const
	{
		event_log.post(shared_ptr<EventDetail>(
			new EventGeneral(event_logger::debug, a.timestamp(),
				wformat(hal::app().res_wstr(HAL_IPFILTER_ALERT))
					% hal::from_utf8_safe(a.ip.to_string())
					% hal::from_utf8_safe(a.message()))
		)	);				
	}
	
	void operator()(libt::alert const& a) const
	{
		event_log.post(shared_ptr<EventDetail>(
				new EventLibtorrent(lbtAlertToHalEvent(a.severity()), 
					a.timestamp(), event_logger::unclassified, hal::from_utf8_safe(a.message()))));		
	}
	
	private:
		bit_impl& bit_impl_;
		
		torrent_internal_ptr get(libt::torrent_handle h) const 
		{ 
			return bit_impl_.the_torrents_.get(from_utf8_safe(h.get_torrent_info().name())); 
		}
	
	} handler(*this);
	
	while (p_alert.get())
	{	
		try
		{
		
		libt::handle_alert<
			libt::external_ip_alert,
			libt::portmap_error_alert,
			libt::portmap_alert,
			libt::file_error_alert,
			libt::torrent_finished_alert,
			libt::torrent_paused_alert,
			libt::peer_error_alert,
			libt::peer_ban_alert,
			libt::hash_failed_alert,
			libt::url_seed_alert,
			libt::dht_reply_alert,
			libt::tracker_error_alert,
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
		
		p_alert = session_.pop_alert();
	}
	
	timer_.expires_from_now(boost::posix_time::seconds(2));
	timer_.async_wait(bind(&bit_impl::alert_handler, this));
	}
}

}
