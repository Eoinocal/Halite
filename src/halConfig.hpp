
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/version.hpp>

#include "global/string_conv.hpp"
#include "global/wtl_app.hpp"

#include "halIni.hpp"

class BitTorrentOptions;
class SecurityOptions;
class ProxyOptions;
class TorrentsOptions;
	
namespace hal
{

class Config :
	public hal::IniBase<Config>,
	private boost::noncopyable
{
public:
	Config() :
		hal::IniBase<Config>("globals/bittorrent", "Config"),
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
		defaultMoveToFolder((hal::app().exe_path().branch_path()/L"completed").string()),
		useMoveTo(false),
		savePrompt(true),
		enablePe(false),
		peEncLevel(0),
		pePerferRc4(false),
		peConInPolicy(1),
		peConOutPolicy(1),
		halfConn(true),
		halfConnLimit(10),
		mappingType(0),
		peerTimeout(30),
		trackerTimeout(60)
	{
		queue_settings_ = hal::bittorrent().get_queue_settings();
		timeouts_ = hal::bittorrent().get_timeouts();
	}
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
		using boost::serialization::make_nvp;

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
		if (version > 3) {
			ar & BOOST_SERIALIZATION_NVP(defaultMoveToFolder);
			ar & BOOST_SERIALIZATION_NVP(useMoveTo);
		}
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

		if (version > 2 && version < 6) {
			ar & BOOST_SERIALIZATION_NVP(mappingType);
			ar & BOOST_SERIALIZATION_NVP(peerTimeout);
			ar & BOOST_SERIALIZATION_NVP(trackerTimeout);
		} else if (version > 5) {
			ar & BOOST_SERIALIZATION_NVP(mappingType);
			ar & make_nvp("queue_settings", queue_settings_);
			ar & make_nvp("timeouts", timeouts_);
		}
	}
	
	bool settingsChanged();
	
	friend class HaliteWindow;
	friend class BitTorrentOptions;
	friend class SecurityOptions;
	friend class ProxyOptions;
	friend class TorrentsOptions;
	friend class GlobalOptions;
	friend class PortOptions;

private:
	bool settingsThread();
	
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
	std::wstring proxyHost;
	int proxyPort;
	std::wstring proxyUsername;
	std::wstring proxyPassword;
	
	std::wstring defaultSaveFolder;
	std::wstring defaultMoveToFolder;
	bool useMoveTo;
	bool savePrompt;

    bool enablePe;
    int peEncLevel;
    bool pePerferRc4;
    int peConInPolicy;
    int peConOutPolicy;
	
	bool halfConn;
	int halfConnLimit;

	int mappingType;
	int peerTimeout;
	int trackerTimeout;	

	hal::queue_settings queue_settings_;
	hal::timeouts timeouts_;
};

Config& config();

} // namespace hal

BOOST_CLASS_VERSION(hal::Config, 6)
