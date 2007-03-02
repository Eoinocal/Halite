
#pragma once

#include "stdAfx.hpp"

#include "halConfig.hpp"
#include "ProgressDialog.hpp"
#include "Halite.hpp"

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
			&BitTorrent::ensure_ip_filter_on, &bittorrent(), _1));
		progDlg.DoModal();
	}
	else
		bittorrent().ensure_ip_filter_off();
	}
	catch(const std::exception& ex)
	{
		::MessageBoxA(0, ex.what(), "Loading IP Filter Exception", MB_ICONERROR|MB_OK);
	}
	
	try
	{
	bool success = bittorrent().listenOn(
		std::make_pair(portFrom, portTo));
	if (!success)
	{
		MessageBox(0, app().res_wstr(IDS_TRYANOTHERPORT).c_str(), L"Init Exception", MB_ICONERROR|MB_OK);
	}
	}
	catch(const std::exception& ex)
	{
		::MessageBoxA(0, ex.what(), "Init Exception", MB_ICONERROR|MB_OK);
	}
	
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
		if (!bittorrent().ensure_dht_on())
		{
			bittorrent().ensure_dht_off();
			MessageBox(0, app().res_wstr(IDS_DHTTRYANOTHERPORT).c_str(), L"DHT Error", MB_ICONERROR|MB_OK);
		}
	}
	else
		bittorrent().ensure_dht_off();
}

Config& config()
{
	static Config c;
	return c;
}

} // namespace hal
