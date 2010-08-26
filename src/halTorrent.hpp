
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
	web_seed_details_t web_seeds;
};

typedef boost::function<bool (size_t, size_t, size_t)> filter_callback;

typedef boost::function<bool (size_t, size_t, std::wstring)> progress_callback;
typedef boost::function<bool (progress_callback fn)> progress_callback_callback;
typedef boost::function<void (std::wstring, progress_callback_callback)> progress_display_callback;

typedef boost::function<void ()> action_callback_t;
typedef boost::function<void (int)> report_num_active;
typedef boost::function<void (wpath path, boost::shared_ptr<file_details_vec> files)> remove_files;

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
		sparse_allocation = 1,
		compact_allocation,
		full_allocation
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

	class torrent : public stlsoft::operator_bool_adaptor<torrent>
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
				unique_lock_t l_;
			};

			exec_around_ptr() {}
			exec_around_ptr(boost::shared_ptr<torrent_internal> p) : ptr(p) {}

			proxy operator->() const
			{
				if (!ptr)
					throw null_torrent();

				return proxy(&(*ptr));
			}

			operator bool() const { return ptr; }


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
				wpath get_name() const;

				STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(wpath, const wpath&, file_proxy, 
					get_name, set_name, name);
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

		const std::wstring get_name() const;
		void set_name(const wstring&);

		const uuid get_uuid() const;

		float get_ratio() const;
		void set_ratio(float new_ratio);
		
		std::pair<int, int> get_connection_limits() const;
		void set_connection_limits(const std::pair<int, int>&);
		std::pair<float, float> get_rate_limits() const;
		void set_rate_limits(const std::pair<float, float>&);

		wpath get_save_directory() const;
		void set_save_directory(const wpath&);
		wpath get_move_to_directory() const;
		void set_move_to_directory(const wpath&);

		std::pair<wstring, wstring> get_tracker_login() const;
		void set_tracker_login(const std::pair<wstring, wstring>&);

		std::vector<tracker_detail> get_trackers() const;
		void set_trackers(const std::vector<tracker_detail>&);

		bool get_is_active() const;
		bool get_in_session() const;

		void set_file_priorities(const vec_int_pair&);

		void set_managed(bool);
		bool get_managed() const;

		void set_superseeding(bool);
		bool get_superseeding() const;
		
	public:
		STLSOFT_METHOD_PROPERTY_GET_EXTERNAL(const uuid, class_type, 
			get_uuid, uuid);

		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(const wstring, const wstring&, class_type, 
			get_name, set_name, name);

		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(float, float, class_type, 
			get_ratio, set_ratio, ratio);

		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(int_pair, const int_pair&, class_type, 
			get_connection_limits, set_connection_limits, connection_limits);
		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(float_pair, const float_pair&, class_type, 
			get_rate_limits, set_rate_limits, rate_limits);
		
		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(wpath, const wpath&, class_type, 
			get_save_directory, set_save_directory, save_directory);
		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(wpath, const wpath&, class_type, 
			get_move_to_directory, set_move_to_directory, move_to_directory);

		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(wstring_pair, const wstring_pair&, 
			class_type, get_tracker_login, set_tracker_login, tracker_login);

		STLSOFT_METHOD_PROPERTY_GET_EXTERNAL(bool, class_type, 
			get_is_active, is_active);
		STLSOFT_METHOD_PROPERTY_GET_EXTERNAL(bool, class_type, 
			get_in_session, in_session);

		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(std::vector<tracker_detail>, const std::vector<tracker_detail>&, 
			class_type, get_trackers, set_trackers, trackers);

		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(bool, bool, class_type, 
			get_managed, set_managed, managed);

		STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(bool, bool, class_type, 
			get_superseeding, set_superseeding, superseeding);

		STLSOFT_METHOD_PROPERTY_SET_EXTERNAL(const vec_int_pair&, class_type, 
			set_file_priorities, file_priorities);

		void reset_trackers();
		bool is_open() const;
		void adjust_queue_position(bit::queue_adjustments adjust);
		files_proxy files();

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
	void start_metadata_plugin();

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

typedef Loki::SingletonHolder<bit, Loki::CreateUsingNew, Loki::PhoenixSingleton> bittorrent;

};
