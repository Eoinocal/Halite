
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
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 2:			
			ar & make_nvp("auto_manage_interval", auto_manage_interval);
			ar & make_nvp("active_downloads", active_downloads);
			ar & make_nvp("active_seeds", active_seeds);
			ar & make_nvp("seeds_hard_limit", seeds_hard_limit);
			ar & make_nvp("seed_ratio_limit", seed_ratio_limit);
			ar & make_nvp("seed_ratio_time_limit", seed_ratio_time_limit);
			ar & make_nvp("seed_time_limit", seed_time_limit);
			ar & make_nvp("dont_count_slow_torrents", dont_count_slow_torrents);
			ar & make_nvp("auto_scrape_min_interval", auto_scrape_min_interval);
			ar & make_nvp("auto_scrape_interval", auto_scrape_interval);
			ar & make_nvp("close_redundant_connections", close_redundant_connections);
		
		break;

		case 1:
		default:
			assert(false);
		}
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
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 2:			
			ar & make_nvp("tracker_completion_timeout", tracker_completion_timeout);
			ar & make_nvp("tracker_receive_timeout", tracker_receive_timeout);
			ar & make_nvp("stop_tracker_timeout", stop_tracker_timeout);
			ar & make_nvp("request_queue_time", request_queue_time);
			ar & make_nvp("piece_timeout", piece_timeout);
			ar & make_nvp("min_reconnect_time", min_reconnect_time);
			ar & make_nvp("peer_timeout", peer_timeout);
			ar & make_nvp("urlseed_timeout", urlseed_timeout);
			ar & make_nvp("peer_connect_timeout", peer_connect_timeout);
			ar & make_nvp("inactivity_timeout", inactivity_timeout);
			ar & make_nvp("handshake_timeout", handshake_timeout);
		
		break;

		case 1:
		default:
			assert(false);
		}
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
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 2:			
			ar & make_nvp("max_peers_reply", max_peers_reply);
			ar & make_nvp("search_branching", search_branching);
			ar & make_nvp("service_port", service_port);
			ar & make_nvp("max_fail_count", max_fail_count);
		
		break;

		case 1:
		default:
			assert(false);
		}
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
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 2:			
			ar & make_nvp("cache_size", cache_size);
			ar & make_nvp("cache_expiry", cache_expiry);
		
		break;

		case 1:
		default:
			assert(false);
		}
	}

	int cache_size;
	int cache_expiry;
};

struct pe_settings
{
	pe_settings() :
		encrypt_level(2),
		prefer_rc4(true),
		conn_in_policy(1),
		conn_out_policy(1)
	{}

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{			
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 2:			
			ar & make_nvp("encrypt_level", encrypt_level);
			ar & make_nvp("prefer_rc4", prefer_rc4);
			ar & make_nvp("conn_in_policy", conn_in_policy);
			ar & make_nvp("conn_out_policy", conn_out_policy);
		
		break;

		case 1:
		default:
			assert(false);
		}
	}

    int encrypt_level;
    bool prefer_rc4;
    int conn_in_policy;
    int conn_out_policy;
};

struct connections
{
	connections() :
		total(500),
		uploads(100),
		download_rate(-1),
		upload_rate(-1)
	{}

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{	
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 2:			
			ar & make_nvp("total", total);
			ar & make_nvp("uploads", uploads);
			ar & make_nvp("download_rate", download_rate);
			ar & make_nvp("upload_rate", upload_rate);
		
		break;

		case 1:
		default:
			assert(false);
		}
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
		case 9:	
			ar & make_nvp("default_allocation_type", default_allocation_type_);
		case 8:			
			ar & make_nvp("announce_all_trackers", announce_all_trackers_);
			ar & make_nvp("announce_all_tiers", announce_all_tiers_);
			ar & make_nvp("custom_interface_use", use_custom_interface_);
			ar & make_nvp("custom_interface_interface", custom_interface_);
			ar & make_nvp("globals", globals_);
			ar & make_nvp("default_save_folder", default_save_folder_);
			ar & make_nvp("default_move_folder", default_move_folder_);
			ar & make_nvp("use_move_to", use_move_to_);
			ar & make_nvp("save_prompt", save_prompt_);
			ar & make_nvp("randomize_port", randomize_port_);
			ar & make_nvp("torrent_defaults", torrent_defaults_);
			ar & make_nvp("queue_settings", queue_settings_);
			ar & make_nvp("timeouts", timeouts_);
			ar & make_nvp("enable_dht", enable_dht_);
			ar & make_nvp("dht_settings", dht_settings_);
			ar & make_nvp("dht_settings_random_port", dht_random_port_);
			ar & make_nvp("dht_settings_upper_port", dht_upper_port_);
			ar & make_nvp("dht_radio", dht_radio_);
			ar & make_nvp("enable_pe", enable_pe_);
			ar & make_nvp("enable_ip_filter", enable_ip_filter_);
			ar & make_nvp("pe_settings", pe_settings_);
			ar & make_nvp("port_range", port_range_);
			ar & make_nvp("use_port_range", use_port_range_);
			ar & make_nvp("half_connections", half_connections_);
			ar & make_nvp("half_connections_limit", half_connections_limit_);
			ar & make_nvp("mapping_upnp", mapping_upnp_);
			ar & make_nvp("mapping_nat_pmp", mapping_nat_pmp_);
			ar & make_nvp("cache_settings", cache_settings_);
			ar & make_nvp("resolve_countries", resolve_countries_);
			ar & make_nvp("plugins_ut_metadata", ut_metadata_plugin_);
			ar & make_nvp("plugins_ut_pex", ut_pex_plugin_);
			ar & make_nvp("plugins_smart_ban", smart_ban_plugin_);
			ar & make_nvp("plugins_lt_trackers", lt_trackers_plugin_);
		
		break;
		
		case 7:
		case 6:
		case 5:
		case 4:
		case 3:
		case 2:
		case 1:
		default:
			assert(false);
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

	hal::bit::allocations default_allocation_type_;
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
	bool ut_metadata_plugin_;
	bool ut_pex_plugin_;
	bool smart_ban_plugin_;
	bool lt_trackers_plugin_;

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

BOOST_CLASS_VERSION(hal::Config, 9)
BOOST_CLASS_VERSION(hal::queue_settings, 2)
BOOST_CLASS_VERSION(hal::timeouts, 2)
BOOST_CLASS_VERSION(hal::dht_settings, 2)
BOOST_CLASS_VERSION(hal::cache_settings, 2)
BOOST_CLASS_VERSION(hal::pe_settings, 2)
BOOST_CLASS_VERSION(hal::connections, 2)

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