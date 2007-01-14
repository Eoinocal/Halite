
#pragma once

#include "../stdAfx.hpp"

#include "../GlobalIni.hpp"
#include "BitTConfig.hpp"
#include "../ProgressDialog.hpp"
#include "../halTorrent.hpp"

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
			&halite::BitTorrent::ensure_ip_filter_on, &halite::bittorrent(), _1));
		progDlg.DoModal();
	}
	else
		halite::bittorrent().ensure_ip_filter_off();
	}
	catch(const std::exception& ex)
	{
		::MessageBoxA(0, ex.what(), "Loading IP Filter Exception", MB_ICONERROR|MB_OK);
	}
	
	try
	{
	bool success = halite::bittorrent().listenOn(
		std::make_pair(INI().bitTConfig().portFrom, INI().bitTConfig().portTo));
	if (!success)
	{
		MessageBox(0, globalModule().loadResString(IDS_TRYANOTHERPORT).c_str(), L"Init Exception", MB_ICONERROR|MB_OK);
	}
	}
	catch(const std::exception& ex)
	{
		::MessageBoxA(0, ex.what(), "Init Exception", MB_ICONERROR|MB_OK);
	}
	
	halite::bittorrent().resumeAll();
	
	halite::bittorrent().setSessionLimits(
		INI().bitTConfig().maxConnections, INI().bitTConfig().maxUploads);
	halite::bittorrent().setSessionSpeed(
		INI().bitTConfig().downRate, INI().bitTConfig().upRate);
		
	halite::bittorrent().setTorrentDefaults(INI().bitTConfig().torrentMaxConnections,
		INI().bitTConfig().torrentMaxUploads, INI().bitTConfig().torrentDownRate,
		INI().bitTConfig().torrentUpRate);
	
	halite::bittorrent().setDhtSettings(INI().bitTConfig().dhtMaxPeersReply, 
		INI().bitTConfig().dhtSearchBranching, INI().bitTConfig().dhtServicePort, 
		INI().bitTConfig().dhtMaxFailCount);
	
	if (INI().bitTConfig().enableDHT)
		if (!halite::bittorrent().ensure_dht_on())
			MessageBox(0, globalModule().loadResString(IDS_DHTTRYANOTHERPORT).c_str(), L"DHT Error", MB_ICONERROR|MB_OK);
	else
		halite::bittorrent().ensure_dht_off();
	
/*	if (INI().remoteConfig().isEnabled)
	{
		halite::xmlRpc().bindHost(INI().remoteConfig().port);
	}
*/
}