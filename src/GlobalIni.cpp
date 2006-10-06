
#include <boost/array.hpp>

#include "GlobalIni.hpp"
#include "ini/Remote.hpp"
#include "ini/Dialog.hpp"
#include "ini/Window.hpp"
#include "ini/BitTConfig.hpp"
#include "ini/Torrent.hpp"

using namespace std;
using namespace boost;
using namespace boost::filesystem;

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
	remoteConfig_(new RemoteConfig())	
{
	array<char, MAX_PATH> pathBuffer;
	GetCurrentDirectoryA(MAX_PATH, pathBuffer.c_array());
	workingFile_ = path(pathBuffer.data(), native)/"Halite.ini.xml";
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
			archive::xml_iarchive ia(ifs);
			
			ia >> serialization::make_nvp("bitConfig", *bitTConfig_);
			ia >> serialization::make_nvp("haliteWindow", *haliteWindow_);
			ia >> serialization::make_nvp("haliteDialog", *haliteDialog_);
			ia >> serialization::make_nvp("remoteConfig", *remoteConfig_);
			ia >> serialization::make_nvp("torrentConfig", *torrentConfig_);
		}
		return true;
	}
	catch(exception& e)
	{
		::MessageBoxA(0,e.what(),"Error",0);
		return false;
	}
}
	
bool ArchivalData::SaveData()
{
	try
	{
		boost::filesystem::ofstream ofs(workingFile_);
		archive::xml_oarchive oa(ofs);
		
		oa << serialization::make_nvp("bitConfig", *bitTConfig_);
		oa << serialization::make_nvp("haliteWindow", *haliteWindow_);
		oa << serialization::make_nvp("haliteDialog", *haliteDialog_);
		oa << serialization::make_nvp("remoteConfig", *remoteConfig_);	
		oa << serialization::make_nvp("torrentConfig", *torrentConfig_);			
		return true;
	}
	catch(exception& e)
	{
		::MessageBoxA(0,e.what(),"Error",0);
		return false;
	}
}