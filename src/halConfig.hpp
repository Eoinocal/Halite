
#pragma once

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/version.hpp>

#include "global/string_conv.hpp"
#include "global/wtl_app.hpp"

#include "HaliteIni.hpp"

class BitTorrentOptions;
class SecurityOptions;
class ProxyOptions;
class TorrentsOptions;
	
namespace hal
{

class Config :
	public CHaliteIni<Config>,
	private boost::noncopyable
{
public:
	Config() :
		CHaliteIni<Config>("globals/bittorrent", "Config"),
		maxConnections(200),
		maxUploads(20),
		downRate(-1),
		upRate(-1),
		torrentMaxConnections(50),
		torrentMaxUploads(10),
		torrentDownRate(-1),
		torrentUpRate(-1),
		portFrom(6881),
		portTo(6881),
		portRange(false),
		enableDHT(false),
		dhtMaxPeersReply(50),
		dhtSearchBranching(5),		
		dhtServicePort(6881),
		dhtMaxFailCount(20),
		enableIPFilter(false),
		enableProxy(false),
		proxyPort(0),
		defaultSaveFolder((hal::app().exe_path().branch_path()/L"incoming").string()),
		savePrompt(true),
		enablePe(false),
		peEncLevel(0),
		pePerferRc4(false),
		peConInPolicy(1),
		peConOutPolicy(1),
		halfConn(true),
		halfConnLimit(10)
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
		
		ar & BOOST_SERIALIZATION_NVP(enableDHT);
		ar & BOOST_SERIALIZATION_NVP(dhtMaxPeersReply);
		ar & BOOST_SERIALIZATION_NVP(dhtSearchBranching);
		ar & BOOST_SERIALIZATION_NVP(dhtServicePort);
		ar & BOOST_SERIALIZATION_NVP(dhtMaxFailCount);
		ar & BOOST_SERIALIZATION_NVP(enableIPFilter);
		ar & BOOST_SERIALIZATION_NVP(portRange);
		
		ar & BOOST_SERIALIZATION_NVP(torrentMaxConnections);
		ar & BOOST_SERIALIZATION_NVP(torrentMaxUploads);
		ar & BOOST_SERIALIZATION_NVP(torrentDownRate);
		ar & BOOST_SERIALIZATION_NVP(torrentUpRate);
		
		if (version <= 1) {
			ar & BOOST_SERIALIZATION_NVP(enableProxy);
			ar & BOOST_SERIALIZATION_NVP(proxyHost);
			ar & BOOST_SERIALIZATION_NVP(proxyPort);
			ar & BOOST_SERIALIZATION_NVP(proxyUsername);
			ar & BOOST_SERIALIZATION_NVP(proxyPassword);
		}
		
		ar & BOOST_SERIALIZATION_NVP(defaultSaveFolder);
		ar & BOOST_SERIALIZATION_NVP(savePrompt);
		
		if (version > 0) {
			ar & BOOST_SERIALIZATION_NVP(enablePe);
			ar & BOOST_SERIALIZATION_NVP(peEncLevel);
			ar & BOOST_SERIALIZATION_NVP(pePerferRc4);
			ar & BOOST_SERIALIZATION_NVP(peConInPolicy);
			ar & BOOST_SERIALIZATION_NVP(peConOutPolicy);
		}
		if (version > 1) {
			ar & BOOST_SERIALIZATION_NVP(halfConn);
			ar & BOOST_SERIALIZATION_NVP(halfConnLimit);
		}
	}
	
	void settingsChanged();
	
	friend class HaliteWindow;
	friend class BitTorrentOptions;
	friend class SecurityOptions;
	friend class ProxyOptions;
	friend class TorrentsOptions;

private:
	void settingsThread();
	
	int maxConnections;
	int maxUploads;
	
	float downRate;
	float upRate;

	int torrentMaxConnections;
	int torrentMaxUploads;
	
	float torrentDownRate;
	float torrentUpRate;
	
	int portFrom;
	int portTo;
	bool portRange;
	
	bool enableDHT;
	int dhtMaxPeersReply;
	int dhtSearchBranching;
	int dhtServicePort;
	int dhtMaxFailCount;
	
	bool enableIPFilter;
	
	bool enableProxy;
	wstring proxyHost;
	int proxyPort;
	wstring proxyUsername;
	wstring proxyPassword;
	
	wstring defaultSaveFolder;
	bool savePrompt;

    bool enablePe;
    int peEncLevel;
    bool pePerferRc4;
    int peConInPolicy;
    int peConOutPolicy;
	
	bool halfConn;
	int halfConnLimit;
};

Config& config();

} // namespace hal

BOOST_CLASS_VERSION(hal::Config, 2)
