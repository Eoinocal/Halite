
#pragma once

#include "WTL.hpp"
#include <string>
#include <tinyxml.h>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>  

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

extern TiXmlDocument GlobalIni;

using namespace std;
using namespace boost;
using namespace boost::filesystem;

template<class Archive>
void serialize(Archive& ar, CRect& rect, const unsigned int version)
{	
	ar & rect.top;
	ar & rect.bottom;		
	ar & rect.left;
	ar & rect.right;
}

struct BitTConfig
{
	BitTConfig() :
		maxConnections(50),
		maxUploads(50),
		portFrom(6881),
		portTo(6889)
	{ }
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & maxConnections;
		ar & maxUploads;
		ar & portFrom;
		ar & portTo;
	}

	int maxConnections;
	int maxUploads;
	
	int portFrom;
	int portTo;
};

struct HaliteWindowConfig
{
	HaliteWindowConfig() :
		splitterPos(300)
	{
		rect.top = 10;
		rect.left = 10;
		rect.bottom = 400;
		rect.right = 500;
		
		for(size_t i=0; i<7; ++i)
			mainListColWidth[i] = 50;
	}
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & rect;
		ar & splitterPos;
		ar & mainListColWidth;
    }
	
	CRect rect;
	unsigned int splitterPos;
	unsigned int mainListColWidth[7];
};


struct HaliteRemoteConfig
{
	HaliteRemoteConfig() :
		isEnabled(false),
		port(80)
	{}
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & isEnabled;
		ar & port;
    }
	
	bool isEnabled;
	unsigned int port;
};

class ArchivalData
{
public:
	ArchivalData(path filename);
	
	bool LoadData()
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
	
	bool SaveData()
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
	
	HaliteRemoteConfig remoteConfig;
	BitTConfig bitTConfig;
	HaliteWindowConfig haliteWindow;
	
private:
	template<class T>
	const T const_save(T t)
	{
		T temp(t);
		return temp;
	}
	path workingFile;
};

extern ArchivalData* INI;
