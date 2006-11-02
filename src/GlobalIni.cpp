
#include <boost/array.hpp>

#include "GlobalIni.hpp"
#include "ini/Remote.hpp"
#include "ini/Dialog.hpp"
#include "ini/Window.hpp"
#include "ini/BitTConfig.hpp"
#include "ini/Torrent.hpp"
#include "ini/Splash.hpp"

using boost::filesystem::path;
using boost::serialization::make_nvp;

ArchivalData& INI()
{
	static ArchivalData ini;
	return ini;
}

ArchivalData::ArchivalData() :
	bitTConfig_(new BitTConfig()),
	haliteWindow_(new WindowConfig()),
	haliteDialog_(new DialogConfig()),
	torrentConfig_(new halite::TorrentConfig()),
	remoteConfig_(new RemoteConfig()),
	splashConfig_(new SplashConfig())	
{
	boost::array<char, MAX_PATH> pathBuffer;
	GetCurrentDirectoryA(MAX_PATH, pathBuffer.c_array());
	workingFile_ = path(pathBuffer.data(), boost::filesystem::native)/"Halite.ini.xml";
}

template<class Archive>
void serialize(Archive& ar, CRect& rect, const unsigned int version)
{	
	ar & BOOST_SERIALIZATION_NVP(rect.top);
	ar & BOOST_SERIALIZATION_NVP(rect.bottom);		
	ar & BOOST_SERIALIZATION_NVP(rect.left);
	ar & BOOST_SERIALIZATION_NVP(rect.right);
}

bool ArchivalData::LoadData()
{	
	try
	{
		boost::filesystem::ifstream ifs(workingFile_);
		if (ifs)
		{
			boost::archive::xml_iarchive ia(ifs);
			
			ia >> make_nvp("bitConfig", *bitTConfig_);
			ia >> make_nvp("haliteWindow", *haliteWindow_);
			ia >> make_nvp("haliteDialog", *haliteDialog_);
			ia >> make_nvp("remoteConfig", *remoteConfig_);
			ia >> make_nvp("splashConfig", *splashConfig_);
			ia >> make_nvp("torrentConfig", *torrentConfig_);
		}
		return true;
	}
	catch(std::exception& e)
	{
		::MessageBoxA(0, e.what(), "Load INI data exception.", 0);
		return false;
	}
}
	
bool ArchivalData::SaveData()
{
	try
	{
		boost::filesystem::ofstream ofs(workingFile_);
		boost::archive::xml_oarchive oa(ofs);
		
		oa << make_nvp("bitConfig", *bitTConfig_);
		oa << make_nvp("haliteWindow", *haliteWindow_);
		oa << make_nvp("haliteDialog", *haliteDialog_);
		oa << make_nvp("remoteConfig", *remoteConfig_);	
		oa << make_nvp("splashConfig", *splashConfig_);
		oa << make_nvp("torrentConfig", *torrentConfig_);			
		return true;
	}
	catch(std::exception& e)
	{
		::MessageBoxA(0, e.what(), "Save INI data exception.", 0);
		return false;
	}
}
