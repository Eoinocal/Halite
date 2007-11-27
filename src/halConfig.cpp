
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdAfx.hpp"

#include "halConfig.hpp"
#include "ProgressDialog.hpp"

#ifndef HALITE_MINI
#	include "Halite.hpp"
#else
#	include "HaliteMini.hpp"
#endif

namespace hal
{

void Config::settingsChanged()
{
//	thread settings(bind(&BitTConfig::settingsThread, this));
	settingsThread();
}

void Config::settingsThread()
{
	try
	{
	if (enableIPFilter)
	{
		ProgressDialog progDlg(L"Loading IP filters...", bind(
			&BitTorrent::ensureIpFilterOn, &bittorrent(), _1));
		progDlg.DoModal();
	}
	else
		bittorrent().ensureIpFilterOff();
	}
	catch(const std::exception& e)
	{
		hal::event().post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(Event::critical, e, L"settingsThread, Load IP Filter"))); 
	}
	
	try
	{
	if (enablePe)
	{
		bittorrent().ensurePeOn(peEncLevel, peConInPolicy, peConOutPolicy, pePerferRc4);
	}
	else
		bittorrent().ensurePeOff();
	}
	catch(const std::exception& e)
	{
		hal::event().post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventStdException(Event::critical, e, L"settingsThread, Protocol Encryption"))); 
	}
	
	try
	{
	bool success = bittorrent().listenOn(
		std::make_pair(portFrom, portTo));
	if (!success)
	{
		hal::event().post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventDebug(Event::critical, L"settingsThread, Init"))); 

//		MessageBox(0, app().res_wstr(IDS_TRYANOTHERPORT).c_str(), L"Init Exception", MB_ICONERROR|MB_OK);
	}
	}
	catch(const std::exception& e)
	{
		hal::event().post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(Event::critical, e, L"settingsThread, Init"))); 
//		::MessageBoxA(0, e.what(), "Init Exception", MB_ICONERROR|MB_OK);
	}
	
	bittorrent().setSessionHalfOpenLimit(halfConnLimit);
	
	bittorrent().resumeAll();	
	
	bittorrent().setSessionLimits(
		maxConnections, maxUploads);
	bittorrent().setSessionSpeed(
		downRate, upRate);
		
	bittorrent().setTorrentDefaults(torrentMaxConnections,
		torrentMaxUploads, torrentDownRate,
		torrentUpRate);
	
	bittorrent().setDhtSettings(dhtMaxPeersReply, 
		dhtSearchBranching, dhtServicePort, 
		dhtMaxFailCount);
	
	if (enableDHT)
	{
		if (!bittorrent().ensureDhtOn())
		{
			bittorrent().ensureDhtOff();
			
			hal::event().post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventDebug(Event::critical, L"settingsThread, DHT Error"))); 
		}
	}
	else
		bittorrent().ensureDhtOff();
}

Config& config()
{
	static Config c;
	return c;
}

} // namespace hal
