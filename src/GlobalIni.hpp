
#pragma once
#include "stdAfx.hpp"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>  

const size_t numMainCols = 7;

using namespace std;
using namespace boost;
using namespace boost::filesystem;

struct BitTConfig
{
	BitTConfig() :
		maxConnections(50),
		maxUploads(50),
		portFrom(6881),
		portTo(6889)
	{}
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

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
		
		for(size_t i=0; i<numMainCols; ++i)
			mainListColWidth[i] = 50;
	}
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version);
	
	CRect rect;
	unsigned int splitterPos;
	unsigned int mainListColWidth[numMainCols];
};

struct HaliteDialogConfig
{
	HaliteDialogConfig()
	{
		for(size_t i=0; i<4; ++i)
			peerListColWidth[i] = 70;
	}
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version);
	
	unsigned int peerListColWidth[4];
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
	
	bool LoadData();	
	bool SaveData();
	
	HaliteRemoteConfig remoteConfig;
	BitTConfig bitTConfig;
	HaliteWindowConfig haliteWindow;
	HaliteDialogConfig haliteDialog;
	
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
