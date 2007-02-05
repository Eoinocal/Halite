
#include "stdAfx.hpp"

#include <boost/array.hpp>
#include <boost/filesystem/fstream.hpp>  

#include "GlobalIni.hpp"
#include "ini/General.hpp"
#include "ini/Remote.hpp"
#include "ini/Dialog.hpp"
#include "ini/Window.hpp"
#include "ini/BitTConfig.hpp"
#include "ini/Splash.hpp"

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
	generalConfig_(new GeneralConfig()),
	remoteConfig_(new RemoteConfig()),
	splashConfig_(new SplashConfig())	
{
	workingFile_ = hal::app().exe_path().branch_path()/"Halite.ini.xml";
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
			ia >> make_nvp("generalConfig", *generalConfig_);
			ia >> make_nvp("remoteConfig", *remoteConfig_);
			ia >> make_nvp("splashConfig", *splashConfig_);
		}
		return true;
	}
	catch(std::exception& e)
	{
		::MessageBox(0, L"There was an error loading the INI XML file. This is a non-fatal error, \
most settings will simply revert to defaults. To continue downloading any torrents that \
were in progress simply manually reload the associated torrent files which can be found \
in the \'torrents\' sub-directory.\r\n\r\n\
See the Halite forum for more details.", 
			(wformat(L"Load INI data exception: %1%") % e.what()).str().c_str(), 0);
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
		oa << make_nvp("generalConfig", *generalConfig_);	
		oa << make_nvp("remoteConfig", *remoteConfig_);	
		oa << make_nvp("splashConfig", *splashConfig_);		
		return true;
	}
	catch(std::exception& e)
	{
		::MessageBoxA(0, e.what(), "Save INI data exception.", 0);
		return false;
	}
}
