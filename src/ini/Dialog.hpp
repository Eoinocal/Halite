
#pragma once

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

class DialogConfig
{
public:
	DialogConfig()
	{
		for(size_t i=0; i<numPeers; ++i)
			peerListColWidth[i] = 70;
	}
	
    friend class boost::serialization::access;
    template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(peerListColWidth);
	}
	
	friend class HaliteDialog;

private:
	static const unsigned numPeers = 4;	
	unsigned int peerListColWidth[numPeers];
};
