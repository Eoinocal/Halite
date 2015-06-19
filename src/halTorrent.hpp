
//         Copyright Eóin O'Callaghan 2006 - 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#if defined(HALTORRENT_PCH)
#	include "halPch.hpp"
#else
#	include "halTypes.hpp"
#endif

#include "halTorrentDetails.hpp"

namespace hal 
{
	

bool remove_empty_directories(const fs::wpath& p);


struct create_torrent_params
{
	create_torrent_params() {}

	std::wstring creator;
	std::wstring comment;
	int piece_size;
	bool private_torrent;

	file_size_pairs_t file_size_pairs;
	fs::wpath root_path;

	tracker_details_t trackers;
	dht_node_details_t dht_nodes;
	std::vector<web_seed_detail> web_seeds;
};

typedef boost::function<bool (size_t, size_t, size_t)> filter_callback;

typedef boost::function<bool (size_t, size_t, std::wstring)> progress_callback;
typedef boost::function<bool (progress_callback fn)> progress_callback_callback;
typedef boost::function<void (std::wstring, progress_callback_callback)> progress_display_callback;

typedef boost::function<void ()> action_callback_t;
typedef boost::function<void (int)> report_num_active;

typedef boost::function<void (wpath path, boost::shared_ptr<std::vector<std::wstring> > files)> remove_files;

class bit_impl;
class torrent_internal;
	
class invalid_torrent : public std::exception
{
public:
	invalid_torrent(const uuid& who) :
		who_(who)
	{}
	
	virtual ~invalid_torrent() throw () {}

	uuid who() const throw ()
	{
		return who_;
	}       
	
private:
	uuid who_;	
};

class bit
{
public:
	enum mappings
	{
		mappingNone = 0,
		mappingUPnP,
		mappingNatPMP
	};

	enum allocations
	{
		compact_allocation = 0,
		full_allocation = 1,
		sparse_allocation = 2
	};

	enum queue_adjustments
	{
		move_up = 0,
		move_down,
		move_to_top,
		move_to_bottom
	};
	
	enum timeout_actions
	{
		action_pause = 0,
		action_resume,
		action_callback
	};

	class null_torrent : public std::exception
	{
	public:
		null_torrent() {}		
		virtual ~null_torrent() throw () {}
	};

	class torrent
	{
		typedef torrent class_type;

	public:
		class exec_around_ptr
		{
		public:
			class proxy 
			{
			public:
				explicit proxy(torrent_internal* t);

				torrent_internal* operator->() 
				{
					return t_;
				}

				~proxy ();

			private:
				torrent_internal* t_;
			};

			exec_around_ptr() {}
			exec_around_ptr(boost::shared_ptr<torrent_internal> p) : ptr(p) {}

			proxy operator->() const
			{
				if (!ptr)
					throw null_torrent();

				return proxy(&(*ptr));
			}

			explicit operator bool() const { return static_cast<bool>(ptr); }


		private:
			boost::shared_ptr<torrent_internal> ptr;
		};

		class files_proxy
		{
		public:
			class file_proxy
			{
			public:
				file_proxy(class_type& t, size_t n) :
					t_(t),
					n_(n)
				{}

				void set_name(const wpath& new_ratio);
				wpath name() const;

			private:
				class_type& t_;
				size_t n_;
			};
			
			files_proxy(class_type& t) :
				t_(t)
			{}

			file_proxy operator[](size_t n)
			{
				return file_proxy(t_, n);
			}

		private:
			class_type& t_;
		};

		torrent();
		torrent(boost::shared_ptr<torrent_internal> p);

		const std::wstring name() const;
		void set_name(const wstring&);

		const uuid uuid() const;

//		float get_ratio() const;
//		void set_ratio(float new_ratio);
		
		std::pair<int, int> connection_limits() const;
		void set_connection_limits(const std::pair<int, int>&);
		void set_connection_limits(int u, int d) { set_connection_limits(std::make_pair(u, d)); }

		std::pair<float, float> rate_limits() const;
		void set_rate_limits(const std::pair<float, float>&);
		void set_rate_limits(float u, float d) { set_rate_limits(std::make_pair(u, d)); }

		wpath save_directory() const;
		void set_save_directory(const wpath&);
		wpath move_to_directory() const;
		void set_move_to_directory(const wpath&);

		std::pair<wstring, wstring> tracker_login() const;
		void set_tracker_login(const std::pair<wstring, wstring>&);
		void set_tracker_login(const wstring& u, const wstring& p) { set_tracker_login(std::make_pair(u, p)); }

		std::vector<tracker_detail> trackers() const;
		void set_trackers(const std::vector<tracker_detail>&);

		bool is_active() const;
		bool in_session() const;
		
		void set_file_priorities(const vec_int_pair&);
		void set_file_priorities(const std::vector<int>& i, int p) { set_file_priorities(std::make_pair(i, p)); }

		void set_managed(bool);

		void set_superseeding(bool);
		bool superseeding() const;
		
		void add_web_seed(const wstring&, web_seed_detail::types type);
		void delete_web_seed(const wstring&, web_seed_detail::types type);
		std::vector<web_seed_detail> web_seeds() const;
		
		void reset_trackers();
		bool is_open() const;
		void adjust_queue_position(bit::queue_adjustments adjust);
		files_proxy files();				

		explicit operator bool() const { return bool(ptr); }

		friend class files_proxy::file_proxy;
		typedef files_proxy::file_proxy file;

	private:
		exec_around_ptr ptr;
	};

	void shutdown_session();
	void save_torrent_data();

	bool create_torrent(const create_torrent_params& params, fs::wpath out_file, progress_callback fn);

	torrent get(torrent_details_ptr p)
	{
		if (!p)			
			return torrent();
		else
			return get(p->uuid());
	}

	torrent get(const uuid& id);
	
	bool listen_on(std::pair<int, int> const& port_range);
	int is_listening_on();
	void stop_listening();
	
	bool ensure_dht_on(const dht_settings& dht);
	void ensure_dht_off();
	
	void ensure_pe_on(const pe_settings& pe);
	void ensure_pe_off();
	
	bool ensure_ip_filter_on(progress_callback fn);
	void ensure_ip_filter_off();

	void set_announce_to_all(bool trackers, bool tiers);

	void set_resolve_countries(bool);
	void start_smart_ban_plugin();
	void start_ut_pex_plugin();
	void start_ut_metadata_plugin();
	void start_lt_trackers_plugin();

	void set_mapping(bool upnp, bool nat_pmp);
	std::wstring upnp_router_model();

	void ip_v4_filter_block(boost::asio::ip::address_v4 first, boost::asio::ip::address_v4 last);
	void ip_v6_filter_block(boost::asio::ip::address_v6 first, boost::asio::ip::address_v6 last);
	bool ip_filter_import_dat(boost::filesystem::path file, progress_callback fn, bool octal_fix);
	size_t ip_filter_size();
	void clear_ip_filter();	
	
	void set_session_half_open_limit(int halfConn);
	void set_session_limits(int max_conn, int max_upload);
	void set_session_speed(float download, float upload);

	queue_settings get_queue_settings();
	void set_queue_settings(const queue_settings& s);
	
	timeouts get_timeouts();
	void set_timeouts(const timeouts& t);

	const cache_details get_cache_details() const;

	void set_cache_settings(const cache_settings& cache);
	cache_settings get_cache_settings() const;
	
	const SessionDetail get_session_details();

	void set_torrent_defaults(const connections& defaults);	

	void add_torrent(const boost::filesystem::wpath& file, const boost::filesystem::wpath& save_directory, 
		bool start_paused=false, bool managed=false, allocations alloc=hal::bit::sparse_allocation, 
		const boost::filesystem::wpath& move_to_directory=L"");
	void add_torrent(const std::wstring& uri, const boost::filesystem::wpath& save_directory, 
		bool start_paused=false, bool managed=false, allocations alloc=hal::bit::sparse_allocation, 
		const boost::filesystem::wpath& move_to_directory=L"");
	
	void get_all_peer_details(const uuid&, peer_details_vec&);
	void get_all_file_details(const uuid&, file_details_vec&);
	
	void resume_all();
	void close_all(boost::optional<report_num_active> fn);
	
	bool is_torrent(const uuid&);
	
	void pause_torrent(const uuid&);
	void resume_torrent(const uuid&);
	void stop_torrent(const uuid&);
	bool is_torrent_active(const uuid&);
	void reannounce_torrent(const uuid&);
	void recheck_torrent(const uuid&);
	
	void pause_all_torrents();
	void unpause_all_torrents();
	bool is_any_torrent_active();

	void remove_torrent(const uuid& id);
	void remove_torrent_wipe_files(const uuid& id, remove_files fn);

	void start_event_receiver();
	void stop_event_receiver();

	void schedual_action(boost::posix_time::ptime time, timeout_actions action);
	void schedual_action(boost::posix_time::time_duration duration, timeout_actions action);

	void schedual_callback(boost::posix_time::ptime time, action_callback_t);
	void schedual_callback(boost::posix_time::time_duration duration, action_callback_t);

	void schedual_cancel();
	
	std::wstring get_external_interface();
	void set_external_interface(const std::wstring& ip);
	void set_external_interface();
	
	int default_torrent_max_connections();
	int default_torrent_max_uploads();
	float default_torrent_download();
	float default_torrent_upload();	

	const torrent_details_manager& torrentDetails();
	const torrent_details_manager& update_torrent_details_manager(
		const uuid& id, const std::set<uuid>& selected);

	void connect_torrent_completed_signal(function<void (wstring torrent_name)> fn);
	
	bit();
	~bit();

private:
	bit_impl* pimpl();
	const bit_impl* pimpl() const;

	boost::scoped_ptr<bit_impl> pimpl_;

	void update_torrent_details_manager_thread(
		const uuid& focused, const std::set<uuid>& selected);
	
	torrent_details_manager torrent_details_;
};

inline bit& bittorrent()
{
	static bit bittorrent_;
	return bittorrent_;
}

};
