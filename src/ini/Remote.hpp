
#pragma once

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

class RemoteConfig
{
public:
	RemoteConfig() :
		isEnabled(false),
		port(80)
	{}
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(isEnabled);
		ar & BOOST_SERIALIZATION_NVP(port);
    }
	
	friend class RemoteOptions;
	friend class HaliteWindow;

private:	
	bool isEnabled;
	unsigned int port;
};
