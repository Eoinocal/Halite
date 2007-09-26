
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "halTorrent.hpp"
#include "halEvent.hpp"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/version.hpp>

#include "HaliteIni.hpp"
#include "../res/resource.h"
	
class Halite :
	public CHaliteIni<Halite>,
	private boost::noncopyable
{
public:
	Halite() :
		CHaliteIni<Halite>("globals/halite", "Halite"),
		oneInst(true),
		logDebug_(false),
		showMessage(true),
		logToFile_(true),
		logListLen_(128),
		dll_(L"")
	{
		load();
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
	const wstring& dll() { return dll_; }
	const size_t logListLen() { return logListLen_; }
	
	friend class GeneralOptions;
	friend class SplashDialog;
	friend class AdvDebugDialog;
	friend int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	
private:
	wstring dll_;
	bool oneInst;
	bool logDebug_;
	bool showMessage;
	bool logToFile_;
	size_t logListLen_;
};

Halite& halite();

BOOST_CLASS_VERSION(Halite, 2)
