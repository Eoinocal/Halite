
#pragma once

#include "halTorrent.hpp"
#include "halEvent.hpp"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/version.hpp>

#include "HaliteIni.hpp"
	
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
		logToFile_(true)
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
	}	
	
	bool logToFile() { return logToFile_; }
	bool logDebug() { return logDebug_; }
	
	friend class GeneralOptions;
	friend class SplashDialog;
	friend class AdvDebugDialog;
	friend int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	
private:
	bool oneInst;
	bool logDebug_;
	bool showMessage;
	bool logToFile_;
};

Halite& halite();

BOOST_CLASS_VERSION(Halite, 0)
