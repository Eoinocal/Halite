
#include "GlobalIni.hpp"

ArchivalData* INI;

template<class Archive>
void serialize(Archive& ar, CRect& rect, const unsigned int version)
{	
	ar & rect.top;
	ar & rect.bottom;		
	ar & rect.left;
	ar & rect.right;
}

ArchivalData::ArchivalData(path filename)
{
	LPSTR pathBuffer = static_cast<LPSTR>(malloc(1024));
	GetCurrentDirectoryA(1024,pathBuffer);
	workingFile = path(pathBuffer,native)/filename;
	free(static_cast<void*>(pathBuffer));
}

template<class Archive>
void BitTConfig::serialize(Archive& ar, const unsigned int version)
{
	ar & maxConnections;
	ar & maxUploads;
	ar & portFrom;
	ar & portTo;
}

template<class Archive>
void HaliteWindowConfig::serialize(Archive& ar, const unsigned int version)
{
	ar & rect;
	ar & splitterPos;
	ar & mainListColWidth;
}

bool ArchivalData::LoadData()
{	
	try
	{
		boost::filesystem::ifstream ifs(workingFile);
		if (ifs)
		{
			archive::text_iarchive ia(ifs);
			
			ia >> bitTConfig;
			ia >> haliteWindow;
			ia >> remoteConfig;
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
		boost::filesystem::ofstream ofs(workingFile);
		archive::text_oarchive oa(ofs);
		
		oa << const_save(bitTConfig);
		oa << const_save(haliteWindow);
		oa << const_save(remoteConfig);			
		return true;
	}
	catch(exception& e)
	{
		::MessageBoxA(0,e.what(),"Error",0);
		return false;
	}
}