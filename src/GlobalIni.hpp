
#pragma once

#include "stdAfx.hpp"

#include <boost/smart_ptr.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>  


class BitTConfig;
class WindowConfig;
class DialogConfig;
class RemoteConfig;
class SplashConfig;

namespace halite
{
class TorrentConfig;
};

class ArchivalData
{
public:	
	bool LoadData();	
	bool SaveData();
	
	RemoteConfig& remoteConfig() { return *remoteConfig_; }
	SplashConfig& splashConfig() { return *splashConfig_; }
	BitTConfig& bitTConfig() { return *bitTConfig_; }
	WindowConfig& windowConfig() { return *haliteWindow_; }
	DialogConfig& dialogConfig() { return *haliteDialog_; }
	halite::TorrentConfig& torrentConfig() { return *torrentConfig_; }
	
	const boost::filesystem::path& workingFile() { return workingFile_; }
	
	friend ArchivalData& INI();
	
private:
	ArchivalData();
	
	boost::scoped_ptr<RemoteConfig> remoteConfig_;
	boost::scoped_ptr<SplashConfig> splashConfig_;
	boost::scoped_ptr<BitTConfig> bitTConfig_;
	boost::scoped_ptr<WindowConfig> haliteWindow_;
	boost::scoped_ptr<DialogConfig> haliteDialog_;
	boost::scoped_ptr<halite::TorrentConfig> torrentConfig_;
	
	boost::filesystem::path workingFile_;
};

ArchivalData& INI();
