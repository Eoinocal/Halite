
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
		port_range_(6881,6881),
		use_port_range_(false),
		enable_dht_(true),
		dht_settings_(),
		enableIPFilter(false),
		enableProxy(false),
		proxyPort(0),
		default_save_folder_((hal::app().exe_path().parent_path()/L"incoming").string()),
		default_move_folder_((hal::app().exe_path().parent_path()/L"completed").string()),
		use_move_to_(false),
		save_prompt_(true),
		enable_pe_(false),
		pe_settings_(),
		half_connections_(true),
		half_connections_limit_(10),
		mapping_upnp_(false),
		mapping_nat_pmp_(false)
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
			& make_nvp("default_save_folder", default_save_folder_)
			& make_nvp("default_move_folder", default_move_folder_)
			& make_nvp("use_move_to", use_move_to_)
			& make_nvp("save_prompt", save_prompt_)
			& make_nvp("torrent_defaults", torrent_defaults_)
			& make_nvp("queue_settings", queue_settings_)
			& make_nvp("timeouts", timeouts_)
			& make_nvp("enable_dht", enable_dht_)
			& make_nvp("dht_settings", dht_settings_)
			& make_nvp("enable_pe", enable_pe_)
			& make_nvp("pe_settings", pe_settings_)
			& make_nvp("port_range", port_range_)
			& make_nvp("use_port_range", use_port_range_)
			& make_nvp("half_connections", half_connections_)
			& make_nvp("half_connections_limit", half_connections_limit_)
			& make_nvp("mapping_upnp", mapping_upnp_)
			& make_nvp("mapping_nat_pmp", mapping_nat_pmp_);
		break;

		case 4:
		ar	& make_nvp("defaultMoveToFolder", default_move_folder_)
			& make_nvp("useMoveTo", use_move_to_);

		case 3:
		case 2:
		ar	& make_nvp("halfConn", half_connections_)
			& make_nvp("halfConnLimit", half_connections_limit_);

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
			& make_nvp("portFrom", port_range_.first)
			& make_nvp("portTo", port_range_.second);
		
		ar	& make_nvp("enableDHT", enable_dht_)
			& make_nvp("dhtMaxPeersReply", dht_settings_.max_peers_reply)
			& make_nvp("dhtSearchBranching", dht_settings_.search_branching)
			& make_nvp("dhtServicePort", dht_settings_.service_port)
			& make_nvp("dhtMaxFailCount", dht_settings_.max_fail_count);
		
		ar	& make_nvp("peerTimeout", timeouts_.peer_connect_timeout)
			& make_nvp("trackerTimeout", timeouts_.tracker_receive_timeout);

		ar	& BOOST_SERIALIZATION_NVP(enableIPFilter)
			& make_nvp("portRange", use_port_range_)
			& make_nvp("torrentMaxConnections", torrent_defaults_.total)
			& make_nvp("torrentMaxUploads", torrent_defaults_.uploads)
			& make_nvp("torrentDownRate", torrent_defaults_.download_rate)
			& make_nvp("torrentUpRate", torrent_defaults_.upload_rate)
			& make_nvp("defaultSaveFolder", default_save_folder_)
			& make_nvp("savePrompt", save_prompt_);
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

	std::pair<int, int> port_range_;
	bool use_port_range_;

	hal::connections torrent_defaults_;

	std::wstring default_save_folder_;
	std::wstring default_move_folder_;
	bool use_move_to_;
	bool save_prompt_;
	
	bool enable_dht_;
	hal::dht_settings dht_settings_;
	
	bool enableIPFilter;
	
	bool enableProxy;
	std::wstring proxyHost;
	int proxyPort;
	std::wstring proxyUsername;
	std::wstring proxyPassword;
	

    bool enable_pe_;
	hal::pe_settings pe_settings_;
	
	bool half_connections_;
	int half_connections_limit_;

	bool mapping_upnp_;
	bool mapping_nat_pmp_;

	hal::queue_settings queue_settings_;
	hal::timeouts timeouts_;
};

Config& config();

} // namespace hal

BOOST_CLASS_VERSION(hal::Config, 6)
