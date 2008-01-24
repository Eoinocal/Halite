
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "halTorrent.hpp"
#include "halEvent.hpp"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/version.hpp>

#include "halIni.hpp"
#include "../res/resource.h"
	
class Halite :
	public hal::IniBase<Halite>,
	private boost::noncopyable
{
public:
	Halite() :
		hal::IniBase<Halite>("globals/halite", "Halite"),
		oneInst(false),
#ifdef TORRENT_LOGGING
		logDebug_(true),
#else
		logDebug_(false),
#endif
		showMessage(true),
		logToFile_(true),
		logListLen_(128),
		dll_(L"")
	{
		load();
	}

	void saveIniData()
	{
		HAL_DEV_MSG(L"Halite saving INI");

		save();		
		hal::ini().save_data();
	}
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
        ar & BOOST_SERIALIZATION_NVP(oneInst);
		ar & BOOST_SERIALIZATION_NVP(logDebug_);
        ar & BOOST_SERIALIZATION_NVP(showMessage);
		ar & BOOST_SERIALIZATION_NVP(logToFile_);
		if (version > 1)
			ar & BOOST_SERIALIZATION_NVP(logListLen_);
		if (version > 0)
			ar & BOOST_SERIALIZATION_NVP(dll_);
	}	
	
	bool logToFile() { return logToFile_; }
	bool logDebug() { return logDebug_; }
	const std::wstring& dll() { return dll_; }
	const int logListLen() { return logListLen_; }
	
	friend class GeneralOptions;
	friend class SplashDialog;
	friend class AdvDebugDialog;
	friend int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	
private:
	std::wstring dll_;
	bool oneInst;
	bool logDebug_;
	bool showMessage;
	bool logToFile_;
	size_t logListLen_;
};

Halite& halite();

BOOST_CLASS_VERSION(Halite, 2)
