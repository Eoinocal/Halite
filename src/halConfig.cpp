
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdAfx.hpp"
#include "Halite.hpp"

#include "halConfig.hpp"
#include "ProgressDialog.hpp"

namespace hal
{

bool Config::settingsChanged()
{
//	thread settings(bind(&BitTConfig::settingsThread, this));
	return settingsThread();
}

bool Config::settingsThread()
{	
	event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Applying BitTorrent session settings.")));	

	bittorrent().set_mapping(mappingType);	

	event_log.post(shared_ptr<EventDetail>(new EventMsg(
			hal::wform(L"Trying port in range %1% - %2%.") % portFrom % portTo)));
	try
	{
	bool success = bittorrent().listen_on(
		std::make_pair(portFrom, portTo));
	if (!success)
	{
		hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventDebug(event_logger::critical, L"settingsThread, Init")));
		
		return false;
	}
	}
	catch(const std::exception& e)
	{
		hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(event_logger::critical, e, L"settingsThread, Init"))); 
		
		return false;
	}

	event_log.post(shared_ptr<EventDetail>(new EventMsg(hal::wform(L"Opened listen port; %1%.") % bittorrent().is_listening_on())));
	
	try
	{
	if (enableIPFilter)
	{
		ProgressDialog progDlg(L"Loading IP filters...", bind(
			&bit::ensure_ip_filter_on, &bittorrent(), _1));
		progDlg.DoModal();
	}
	else
		bittorrent().ensure_ip_filter_off();
	}
	catch(const std::exception& e)
	{
		hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(event_logger::critical, e, L"settingsThread, Load IP Filter"))); 
	}	

	try
	{
	if (enablePe)
	{
		bittorrent().ensure_pe_on(peEncLevel, peConInPolicy, peConOutPolicy, pePerferRc4);
	}
	else
		bittorrent().ensure_pe_off();
	}
	catch(const std::exception& e)
	{
		hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventStdException(event_logger::critical, e, L"settingsThread, Protocol Encryption"))); 
	}
	
	bittorrent().setSessionHalfOpenLimit(halfConnLimit);
	
	bittorrent().resume_all();	
	
	bittorrent().set_session_limits(maxConnections, maxUploads);
	bittorrent().set_session_speed(	downRate, upRate);
		
	bittorrent().setTorrentDefaults(torrentMaxConnections,
		torrentMaxUploads, torrentDownRate,	torrentUpRate);
	
	bittorrent().set_dht_settings(dhtMaxPeersReply, 
		dhtSearchBranching, dhtServicePort, dhtMaxFailCount);

	bittorrent().set_timeouts(timeouts_);	
	bittorrent().set_queue_settings(queue_settings_);
	
	if (enableDHT)
	{
		if (!bittorrent().ensure_dht_on())
		{
			bittorrent().ensure_dht_off();
			
			hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventDebug(event_logger::critical, L"settingsThread, DHT Error"))); 
		}
	}
	else
		bittorrent().ensure_dht_off();
		
	// Settings seem to have applied ok!
	save_to_ini();	
	return true;
}

Config& config()
{
	static Config c;
	return c;
}

} // namespace hal
