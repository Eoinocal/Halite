
#pragma once

#include "../stdAfx.hpp"

#include "../GlobalIni.hpp"
#include "BitTConfig.hpp"
#include "../ProgressDialog.hpp"
#include "../Halite.hpp"

void BitTConfig::settingsChanged()
{
//	thread settings(bind(&BitTConfig::settingsThread, this));
	settingsThread();
}

void BitTConfig::settingsThread()
{
	try
	{
	if (INI().bitTConfig().enableIPFilter)
	{
		ProgressDialog progDlg(L"Loading IP filters...", bind(
			&hal::BitTorrent::ensure_ip_filter_on, &hal::bittorrent(), _1));
		progDlg.DoModal();
	}
	else
		hal::bittorrent().ensure_ip_filter_off();
	}
	catch(const std::exception& ex)
	{
		::MessageBoxA(0, ex.what(), "Loading IP Filter Exception", MB_ICONERROR|MB_OK);
	}
	
	try
	{
	bool success = hal::bittorrent().listenOn(
		std::make_pair(INI().bitTConfig().portFrom, INI().bitTConfig().portTo));
	if (!success)
	{
		MessageBox(0, hal::app().load_res_wstring(IDS_TRYANOTHERPORT).c_str(), L"Init Exception", MB_ICONERROR|MB_OK);
	}
	}
	catch(const std::exception& ex)
	{
		::MessageBoxA(0, ex.what(), "Init Exception", MB_ICONERROR|MB_OK);
	}
	
	hal::bittorrent().resumeAll();
	
	hal::bittorrent().setSessionLimits(
		INI().bitTConfig().maxConnections, INI().bitTConfig().maxUploads);
	hal::bittorrent().setSessionSpeed(
		INI().bitTConfig().downRate, INI().bitTConfig().upRate);
		
	hal::bittorrent().setTorrentDefaults(INI().bitTConfig().torrentMaxConnections,
		INI().bitTConfig().torrentMaxUploads, INI().bitTConfig().torrentDownRate,
		INI().bitTConfig().torrentUpRate);
	
	hal::bittorrent().setDhtSettings(INI().bitTConfig().dhtMaxPeersReply, 
		INI().bitTConfig().dhtSearchBranching, INI().bitTConfig().dhtServicePort, 
		INI().bitTConfig().dhtMaxFailCount);
	
	if (INI().bitTConfig().enableDHT)
	{
		if (!hal::bittorrent().ensure_dht_on())
		{
			hal::bittorrent().ensure_dht_off();
			MessageBox(0, hal::app().load_res_wstring(IDS_DHTTRYANOTHERPORT).c_str(), L"DHT Error", MB_ICONERROR|MB_OK);
		}
	}
	else
		hal::bittorrent().ensure_dht_off();
	
/*	if (INI().remoteConfig().isEnabled)
	{
		hal::xmlRpc().bindHost(INI().remoteConfig().port);
	}
*/
}