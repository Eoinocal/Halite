
#pragma once

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/version.hpp>

class BitTConfig
{
public:
	BitTConfig() :
		maxConnections(-1),
		maxUploads(-1),
		downRate(-1),
		upRate(-1),
		portFrom(6881),
		portTo(6889),
		enableDHT(false),
		dhtMaxPeersReply(50),
		dhtSearchBranching(5),		
		dhtServicePort(6881),
		dhtMaxFailCount(20),
		enableIPFilter(false)
	{}
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
		ar & BOOST_SERIALIZATION_NVP(maxConnections);
		ar & BOOST_SERIALIZATION_NVP(maxUploads);
		ar & BOOST_SERIALIZATION_NVP(downRate);
		ar & BOOST_SERIALIZATION_NVP(upRate);
		ar & BOOST_SERIALIZATION_NVP(portFrom);
		ar & BOOST_SERIALIZATION_NVP(portTo);
		if(version > 0) {
			ar & BOOST_SERIALIZATION_NVP(enableDHT);
			ar & BOOST_SERIALIZATION_NVP(dhtMaxPeersReply);
			ar & BOOST_SERIALIZATION_NVP(dhtSearchBranching);
			ar & BOOST_SERIALIZATION_NVP(dhtServicePort);
			ar & BOOST_SERIALIZATION_NVP(dhtMaxFailCount);
			ar & BOOST_SERIALIZATION_NVP(enableIPFilter);
		}
	}
	
	void settingsChanged();
	
	friend class BitTorrentOptions;
	friend class HaliteWindow;

private:
	void settingsThread();
	
	int maxConnections;
	int maxUploads;
	
	float downRate;
	float upRate;
	
	int portFrom;
	int portTo;
	
	bool enableDHT;
	int dhtMaxPeersReply;
	int dhtSearchBranching;
	int dhtServicePort;
	int dhtMaxFailCount;
	
	bool enableIPFilter;
};

BOOST_CLASS_VERSION(BitTConfig, 1)

