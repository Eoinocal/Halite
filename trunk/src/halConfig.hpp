
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
		globals_(),
		torrent_defaults_(),
		portFrom(6881),
		portTo(6881),
		portRange(false),
		enable_dht_(true),
		dht_settings_(),
		enableIPFilter(false),
		enableProxy(false),
		proxyPort(0),
		defaultSaveFolder((hal::app().exe_path().branch_path()/L"incoming").string()),
		defaultMoveToFolder((hal::app().exe_path().branch_path()/L"completed").string()),
		useMoveTo(false),
		savePrompt(true),
		enable_pe_(false),
		pe_settings_(),
		halfConn(true),
		halfConnLimit(10),
		mappingType(0)
	{
		queue_settings_ = hal::bittorrent().get_queue_settings();
		timeouts_ = hal::bittorrent().get_timeouts();
	}
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 6:
		ar	& make_nvp("globals", globals_)
			& make_nvp("torrent_defaults", torrent_defaults_)
			& make_nvp("queue_settings", queue_settings_)
			& make_nvp("timeouts", timeouts_)
			& make_nvp("enable_dht", enable_dht_)
			& make_nvp("dht_settings", dht_settings_)
			& make_nvp("enable_pe", enable_pe_)
			& make_nvp("pe_settings", pe_settings_);
		break;

		case 4:
		ar	& BOOST_SERIALIZATION_NVP(defaultMoveToFolder);
			& BOOST_SERIALIZATION_NVP(useMoveTo);

		case 3:
		ar	& BOOST_SERIALIZATION_NVP(mappingType);

		case 2:
		ar	& BOOST_SERIALIZATION_NVP(halfConn)
			& BOOST_SERIALIZATION_NVP(halfConnLimit);

		case 1:
		ar	& make_nvp("enablePe", enable_pe_)
			& make_nvp("peEncLevel", pe_settings_.encrypt_level)
			& make_nvp("pePerferRc4", pe_settings_.prefer_rc4)
			& make_nvp("peConInPolicy", pe_settings_.conn_in_policy)
			& make_nvp("peConOutPolicy", pe_settings_.conn_out_policy);

		case 0:
		ar	& make_nvp("maxConnections", globals_.total)
			& make_nvp("maxUploads", globals_.uploads)
			& make_nvp("downRate", globals_.download_rate)
			& make_nvp("upRate", globals_.upload_rate)
			& BOOST_SERIALIZATION_NVP(portFrom)
			& BOOST_SERIALIZATION_NVP(portTo);
		
		ar	& make_nvp("enableDHT", enable_dht_)
			& make_nvp("dhtMaxPeersReply", dht_settings_.max_peers_reply)
			& make_nvp("dhtSearchBranching", dht_settings_.search_branching)
			& make_nvp("dhtServicePort", dht_settings_.service_port)
			& make_nvp("dhtMaxFailCount", dht_settings_.max_fail_count);
		
		ar	& make_nvp("peerTimeout", timeouts_.peer_connect_timeout)
			& make_nvp("trackerTimeout", timeouts_.tracker_receive_timeout);

		ar	& BOOST_SERIALIZATION_NVP(enableIPFilter)
			& BOOST_SERIALIZATION_NVP(portRange)
			& make_nvp("torrentMaxConnections", torrent_defaults_.total)
			& make_nvp("torrentMaxUploads", torrent_defaults_.uploads)
			& make_nvp("torrentDownRate", torrent_defaults_.download_rate)
			& make_nvp("torrentUpRate", torrent_defaults_.upload_rate)
			& BOOST_SERIALIZATION_NVP(defaultSaveFolder)
			& BOOST_SERIALIZATION_NVP(savePrompt);
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
	
	hal::connections globals_;
	hal::connections torrent_defaults_;
	
	int portFrom;
	int portTo;
	bool portRange;
	
	bool enable_dht_;
	hal::dht_settings dht_settings_;
	
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

    bool enable_pe_;
	hal::pe_settings pe_settings_;
	
	bool halfConn;
	int halfConnLimit;

	int mappingType;

	hal::queue_settings queue_settings_;
	hal::timeouts timeouts_;
};

Config& config();

} // namespace hal

BOOST_CLASS_VERSION(hal::Config, 6)
