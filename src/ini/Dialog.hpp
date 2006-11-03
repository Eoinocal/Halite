
#pragma once

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

class DialogConfig
{
public:
	DialogConfig()
	{
		peerListColWidth[0] = 100;
		peerListColWidth[1] = 70;
		peerListColWidth[2] = 70;
		peerListColWidth[3] = 70;
		peerListColWidth[4] = 100;
	}
	
    friend class boost::serialization::access;
    template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(peerListColWidth);
	}
	
	friend class HaliteDialog;

private:
	static const unsigned numPeers = 5;	
	unsigned int peerListColWidth[numPeers];
};
