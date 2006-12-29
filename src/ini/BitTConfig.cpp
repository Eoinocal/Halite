
#pragma once

#include "../stdAfx.hpp"

#include "../GlobalIni.hpp"
#include "BitTConfig.hpp"
#include "../halTorrent.hpp"

void BitTConfig::settingsChanged()
{
	try
	{
	bool success = halite::bittorrent().listenOn(
		std::make_pair(INI().bitTConfig().portFrom, INI().bitTConfig().portTo));
	assert(success);	
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
	
	halite::bittorrent().setDhtSettings(INI().bitTConfig().dhtMaxPeersReply, 
		INI().bitTConfig().dhtSearchBranching, INI().bitTConfig().dhtServicePort, INI().bitTConfig().dhtMaxFailCount);
	
	if (INI().bitTConfig().enableDHT)
		halite::bittorrent().ensure_dht_on();
	else
		halite::bittorrent().ensure_dht_off();
	
/*	if (INI().remoteConfig().isEnabled)
	{
		halite::xmlRpc().bindHost(INI().remoteConfig().port);
	}
*/	
}