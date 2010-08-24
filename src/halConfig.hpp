
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#if defined(HALTORRENT_PCH)
#	include "halPch.hpp"
#else
#	include "halTypes.hpp"
#endif

class BitTorrentOptions;
class SecurityOptions;
class ProxyOptions;
class TorrentsOptions;
	
namespace hal
{

template<typename T>
class action_setting
{
public:
	typedef boost::function<void (const T&)> function_type;

	action_setting()
	{}

	action_setting(const T& d) :
		data_(d)
	{}

	action_setting(function_type f) :
		fn_(f)
	{}

	action_setting(const T& d, function_type f) :
		data_(d),
		fn_(f)
	{}
	
	operator const T& () const
	{ 
		return data_;
	}

	void set_no_action(const T& d)
	{
		data_ = d;
	}

	T& operator=(const T& d) 
	{	
		if (data_ != d && fn_)
			fn_(d);

		data_ = d;

		return data_;
	}

	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::make_nvp("data", data_);
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{	
		T d = data_;
		ar & boost::serialization::make_nvp("data", d);

		if (data_ != d && fn_)
			fn_(d);

		data_ = d;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
	T data_;
	function_type fn_;
};

struct queue_settings
{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
		ar & BOOST_SERIALIZATION_NVP(auto_manage_interval);
		ar & BOOST_SERIALIZATION_NVP(active_downloads);
		ar & BOOST_SERIALIZATION_NVP(active_seeds);
		ar & BOOST_SERIALIZATION_NVP(seeds_hard_limit);
		ar & BOOST_SERIALIZATION_NVP(seed_ratio_limit);
		ar & BOOST_SERIALIZATION_NVP(seed_ratio_time_limit);
		ar & BOOST_SERIALIZATION_NVP(seed_time_limit);
		ar & BOOST_SERIALIZATION_NVP(dont_count_slow_torrents);
		ar & BOOST_SERIALIZATION_NVP(auto_scrape_min_interval);
		ar & BOOST_SERIALIZATION_NVP(auto_scrape_interval);
		ar & BOOST_SERIALIZATION_NVP(close_redundant_connections);
	}

	bool operator==(const queue_settings& s) const
	{
		return (auto_manage_interval == s.auto_manage_interval &&
			active_downloads == s.active_downloads &&
			active_seeds == s.active_seeds &&
			seeds_hard_limit == s.seeds_hard_limit &&
			seed_ratio_limit == s.seed_ratio_limit &&
			seed_ratio_time_limit == s.seed_ratio_time_limit &&
			seed_time_limit == s.seed_time_limit &&
			dont_count_slow_torrents == s.dont_count_slow_torrents &&
			auto_scrape_min_interval == s.auto_scrape_min_interval &&
			auto_scrape_interval == s.auto_scrape_interval &&
			close_redundant_connections == s.close_redundant_connections);
	}
	
	bool operator!=(const queue_settings& s) const
	{
		return !(*this == s);
	}

	int auto_manage_interval;

	int active_downloads;
	int active_seeds;
	int seeds_hard_limit;
	float seed_ratio_limit;
	float seed_ratio_time_limit;
	int seed_time_limit;
	bool dont_count_slow_torrents;

	int auto_scrape_min_interval;
	int auto_scrape_interval;
	bool close_redundant_connections;
};

struct timeouts
{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
		ar & BOOST_SERIALIZATION_NVP(tracker_completion_timeout);
		ar & BOOST_SERIALIZATION_NVP(tracker_receive_timeout);
		ar & BOOST_SERIALIZATION_NVP(stop_tracker_timeout);

		ar & BOOST_SERIALIZATION_NVP(request_queue_time);
		ar & BOOST_SERIALIZATION_NVP(piece_timeout);
		ar & BOOST_SERIALIZATION_NVP(min_reconnect_time);

		ar & BOOST_SERIALIZATION_NVP(peer_timeout);
		ar & BOOST_SERIALIZATION_NVP(urlseed_timeout);
		ar & BOOST_SERIALIZATION_NVP(peer_connect_timeout);
		ar & BOOST_SERIALIZATION_NVP(inactivity_timeout);
		ar & BOOST_SERIALIZATION_NVP(handshake_timeout);
	}

	int tracker_completion_timeout;
	int tracker_receive_timeout;
	int stop_tracker_timeout;

	float request_queue_time;
	int piece_timeout;
	int min_reconnect_time;

	int peer_timeout;
	int urlseed_timeout;
	int peer_connect_timeout;
	int inactivity_timeout;
	int handshake_timeout;
};

struct dht_settings
{
	dht_settings() :
		max_peers_reply(50),
		search_branching(5),
		service_port(6881),
		max_fail_count(20)
	{}

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
		ar & BOOST_SERIALIZATION_NVP(max_peers_reply);
		ar & BOOST_SERIALIZATION_NVP(search_branching);
		ar & BOOST_SERIALIZATION_NVP(service_port);
		ar & BOOST_SERIALIZATION_NVP(max_fail_count);
	}

	int max_peers_reply;
	int search_branching;
	int service_port;
	int max_fail_count;
};

struct cache_settings
{
	cache_settings() :
		cache_size(512),
		cache_expiry(60)
	{}

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
		ar & BOOST_SERIALIZATION_NVP(cache_size);
		ar & BOOST_SERIALIZATION_NVP(cache_expiry);
	}

	int cache_size;
	int cache_expiry;
};

struct pe_settings
{
	pe_settings() :
		encrypt_level(0),
		prefer_rc4(false),
		conn_in_policy(1),
		conn_out_policy(1)
	{}

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
		ar & BOOST_SERIALIZATION_NVP(encrypt_level);
		ar & BOOST_SERIALIZATION_NVP(prefer_rc4);
		ar & BOOST_SERIALIZATION_NVP(conn_in_policy);
		ar & BOOST_SERIALIZATION_NVP(conn_out_policy);
	}

    int encrypt_level;
    bool prefer_rc4;
    int conn_in_policy;
    int conn_out_policy;
};

struct connections
{
	connections() :
		total(50),
		uploads(10),
		download_rate(-1),
		upload_rate(-1)
	{}

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
		ar & BOOST_SERIALIZATION_NVP(total);
		ar & BOOST_SERIALIZATION_NVP(uploads);
		ar & BOOST_SERIALIZATION_NVP(download_rate);
		ar & BOOST_SERIALIZATION_NVP(upload_rate);
	}

    int total;
    int uploads;
    float download_rate;
    float upload_rate;
};

struct cache_details
{
	cache_details() :
		blocks_written(-1),
		writes(-1),
		blocks_read(-1),
		blocks_read_hit(-1),
		reads(-1),
		cache_size(-1),
		read_cache_size(-1),
		write_ratio(-1),
		read_ratio(-1),
		write_cache_size(-1)
	{}

	cache_details(size_type b_w,
			size_type w,
			size_type b_r,
			size_type b_r_h,
			size_type r,
			int c_s,
			int r_c_s) :
		blocks_written(b_w),
		writes(w),
		blocks_read(b_r),
		blocks_read_hit(b_r_h),
		reads(r),
		cache_size(c_s),
		read_cache_size(r_c_s),
		write_cache_size(cache_size-read_cache_size)
	{
		write_ratio = (blocks_written == 0) ? 0 :
			static_cast<double>(blocks_written-writes) / blocks_written;

		read_ratio = (blocks_read == 0) ? 0 :
			static_cast<double>(blocks_read_hit) / blocks_read;}

    size_type blocks_written;
    size_type writes;
    size_type blocks_read;
    size_type blocks_read_hit;
    size_type reads;
    int cache_size;
    int read_cache_size;

	double write_ratio;
	double read_ratio;
	int write_cache_size;
};
	
class Config :
	public hal::IniBase<Config>,
	private boost::noncopyable
{
public:
	Config();
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 7:
			
		ar	& make_nvp("announce_all/trackers", announce_all_trackers_)
			& make_nvp("announce_all/tiers", announce_all_tiers_)
			& make_nvp("custom_interface/use", use_custom_interface_)
			& make_nvp("custom_interface/interface", custom_interface_);

		case 6:
		ar	& make_nvp("globals", globals_)
			& make_nvp("default_save_folder", default_save_folder_)
			& make_nvp("default_move_folder", default_move_folder_)
			& make_nvp("use_move_to", use_move_to_)
			& make_nvp("save_prompt", save_prompt_)
			& make_nvp("randomize_port", randomize_port_)			
			& make_nvp("torrent_defaults", torrent_defaults_)
			& make_nvp("queue_settings", queue_settings_)
			& make_nvp("timeouts", timeouts_)
			& make_nvp("enable_dht", enable_dht_)
			& make_nvp("dht_settings", dht_settings_)
			& make_nvp("dht_settings/random_port", dht_random_port_)
			& make_nvp("dht_settings/upper_port", dht_upper_port_)
			& make_nvp("dht_radio", dht_radio_)
			& make_nvp("enable_pe", enable_pe_)
			& make_nvp("enable_ip_filter", enable_ip_filter_)
			& make_nvp("pe_settings", pe_settings_)
			& make_nvp("port_range", port_range_)
			& make_nvp("use_port_range", use_port_range_)
			& make_nvp("half_connections", half_connections_)
			& make_nvp("half_connections_limit", half_connections_limit_)
			& make_nvp("mapping_upnp", mapping_upnp_)
			& make_nvp("mapping_nat_pmp", mapping_nat_pmp_)
			& make_nvp("cache_settings", cache_settings_)
			& make_nvp("resolve_countries", resolve_countries_)
			& make_nvp("plugins/metadata", metadata_plugin_)
			& make_nvp("plugins/ut_metadata", ut_metadata_plugin_)
			& make_nvp("plugins/ut_pex", ut_pex_plugin_)
			& make_nvp("plugins/smart_ban", smart_ban_plugin_);
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

		ar	& make_nvp("enableIPFilter", enable_ip_filter_)
			& make_nvp("portRange", use_port_range_)
			& make_nvp("torrentMaxConnections", torrent_defaults_.total)
			& make_nvp("torrentMaxUploads", torrent_defaults_.uploads)
			& make_nvp("torrentDownRate", torrent_defaults_.download_rate)
			& make_nvp("torrentUpRate", torrent_defaults_.upload_rate)
			& make_nvp("defaultSaveFolder", default_save_folder_)
			& make_nvp("savePrompt", save_prompt_);
		}
	}
	
	void set_callback(progress_display_callback f) { pdc_ = f; }
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

	hal::progress_display_callback pdc_;
	
	hal::connections globals_;

	std::pair<int, int> port_range_;
	bool use_port_range_;
	bool randomize_port_;

	hal::connections torrent_defaults_;

	std::wstring default_save_folder_;
	std::wstring default_move_folder_;
	bool use_move_to_;
	bool save_prompt_;
	
	bool enable_dht_;
	hal::dht_settings dht_settings_;
	bool dht_random_port_;
	unsigned dht_upper_port_;
	int dht_radio_;
	
	bool enable_ip_filter_;
	
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

	bool resolve_countries_;
	bool metadata_plugin_;
	bool ut_metadata_plugin_;
	bool ut_pex_plugin_;
	bool smart_ban_plugin_;

	bool announce_all_trackers_;
	bool announce_all_tiers_;	

	bool use_custom_interface_;
	std::wstring custom_interface_;	

	hal::cache_settings cache_settings_;

	action_setting<hal::queue_settings> queue_settings_;
	hal::timeouts timeouts_;
};

Config& config();

} // namespace hal

BOOST_CLASS_VERSION(hal::Config, 7)

/*namespace boost {
namespace serialization {
	template <typename T>
	struct version< hal::action_setting<T> >
	{
		typedef mpl::int_<1> type;
		typedef mpl::integral_c_tag tag;
		BOOST_STATIC_CONSTANT(unsigned int, value = version::type::value);                                                             
	};
}
}
*/