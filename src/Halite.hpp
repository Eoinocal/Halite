
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
		logLevel(0),
		showMessage(true)
	{
		load();
	}
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
        ar & BOOST_SERIALIZATION_NVP(oneInst);
		ar & BOOST_SERIALIZATION_NVP(logLevel);
        ar & BOOST_SERIALIZATION_NVP(showMessage);
	}	
	
	friend int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	
private:
	bool oneInst;
	int logLevel;
	bool showMessage;
};

Halite& halite();

BOOST_CLASS_VERSION(Halite, 0)
