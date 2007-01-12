
#pragma once

#include <boost/serialization/version.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

class GeneralConfig
{
public:
	GeneralConfig() :
		oneInst(true),
		logLevel(0)
	{}
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(oneInst);
		if (version > 0) {
			ar & BOOST_SERIALIZATION_NVP(logLevel);
		}
    }
	
	friend class GeneralOptions;
	friend class AdvDebugDialog;
	friend int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

private:	
	bool oneInst;
	int logLevel;
};

BOOST_CLASS_VERSION(GeneralConfig, 1)