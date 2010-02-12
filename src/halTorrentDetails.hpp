
//         Copyright Eóin O'Callaghan 2006 - 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/*
#include <string>
#include <vector>
#include <set>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include <boost/smart_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <boost/signal.hpp>
#include <boost/optional.hpp>
#include <boost/function.hpp>

#include <boost/smart_ptr.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
*/

#include <boost/algorithm/string.hpp>

#include <stlsoft/properties/method_properties.hpp>
#include <stlsoft/util/operator_bool_adaptor.hpp>

#include <loki/Singleton.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "halTypes.hpp"
#include "halPeers.hpp"

namespace hal 
{

template<typename T>
bool hal_details_ptr_compare(T l, T r, size_t index = 0, bool cmp_less = true)
{
	if (cmp_less)
		return l->less(*r, index);
	else
		return r->less(*l, index);
}

template<typename T>
bool hal_details_compare(T l, T r, size_t index = 0, bool cmp_less = true)
{
	if (cmp_less)
		return l.less(r, index);
	else
		return r.less(l, index);
}

struct torrentBriefDetail 
{
	std::wstring filename;
	std::wstring status;
	std::pair<float,float> speed;
	float completion;
	int peers;
	int seeds;
};

struct queue_settings;
struct timeouts;
struct dht_settings;
struct cache_settings;
struct pe_settings;
struct connections;
struct cache_details;


struct file_details
{
	enum details
	{
		filename_e = 0,
		branch_e,
		size_e,
		progress_e,
		priority_e,
		type_e
	};

	file_details(boost::filesystem::wpath p, 
			boost::int64_t s=0, 
			boost::int64_t pg=0, 
			int pr=1, 
			size_t o=0, 
			unsigned t=file_details::file) :
		branch(p.parent_path()),
		filename(p.filename()),
		type(t),
		size(s),
		progress(pg),
		priority(pr),
		order_(o)
	{}
	
	bool operator==(const file_details& file) const
	{
		return (branch == file.branch);
	}
	
	bool operator<(const file_details& file) const
	{
		return (branch < file.branch);
	}
	
	bool less(const file_details& r, size_t index = 0) const;
	std::wstring to_wstring(size_t index = 0);
	
	enum file_type
	{
		folder,
		file
	};
	
	size_t order() const { return order_; }
	
	boost::filesystem::wpath branch;
	std::wstring filename;
	unsigned type;
	boost::int64_t size;
	boost::int64_t progress;
	int priority;
	
private:
	size_t order_;
};

inline bool file_details_names_equal(const file_details& l, const file_details& r)
{
	return l.filename == r.filename;
}

inline bool file_details_names_less(const file_details& l, const file_details& r)
{
	return l.filename < r.filename;
}

typedef std::vector<file_details> file_details_vec;

void file_details_sort(file_details_vec& f, size_t index, bool cmp_less = true);



class torrent_details 
{
public:
	torrent_details(std::wstring n, std::wstring f, 
			std::wstring sd, 
			std::wstring s, 
			std::wstring cT, 
			std::pair<float,float> sp=std::pair<float,float>(0,0),
			float c=0, float d=0, 
			size_type tWD=0, size_type tW=0, 
			size_type tU=0, size_type tpU=0, 
			size_type tD=0, size_type tpD=0, 
			boost::tuple<size_type, size_type, size_type, size_type> connections = 
				boost::tuple<size_type, size_type, size_type, size_type>(0,0,0,0), 
			float r=0, 
			boost::posix_time::time_duration eta=boost::posix_time::seconds(0), 
			boost::posix_time::time_duration uIn=boost::posix_time::seconds(0),
			boost::posix_time::time_duration actve=boost::posix_time::seconds(0), 
			boost::posix_time::time_duration seding=boost::posix_time::seconds(0), 
			boost::posix_time::ptime srt=boost::posix_time::second_clock::universal_time(), 
			boost::posix_time::ptime fin=boost::posix_time::second_clock::universal_time(), 
			int q_p=-1, 
			bool man=false) :
		filename_(f),
		name_(n),
		saveDir_(sd),
		state_(s),
		currentTracker_(cT),
		speed_(sp),
		completion_(c),
		distributed_copies_(d),
		total_wanted_done_(tWD),
		total_wanted_(tW),
		total_uploaded_(tU),
		total_payload_uploaded_(tpU),
		total_downloaded_(tD),
		total_payload_downloaded_(tpD),
		peers_(connections.get<0>()),
		connected_peers_(connections.get<1>()),
		seeds_(connections.get<2>()),
		connected_seeds_(connections.get<3>()),
		ratio_(r),
		estimated_time_left_(eta),
		update_tracker_in_(uIn),
		peer_details_filled_(false),
		file_details_filled_(false),
		active_(actve),
		seeding_(seding),
		start_time_(srt),
		finish_time_(fin),
		queue_position_(q_p),
		managed_(man)
	{}

	torrent_details() :	
		peer_details_filled_(false),
		file_details_filled_(false)
	{};
	
	enum state
	{
		torrent_active = 0,
		torrent_paused,
		torrent_stopped,
		torrent_pausing,
		torrent_stopping,
		torrent_in_error,
		torrent_not_started
	};

	enum details
	{
		name_e = 0,
		state_e,
		progress_e,
		speed_down_e,
		speed_up_e,
		peers_e,
		seeds_e,
		eta_e,
		distributed_copies_e,
		tracker,
		update_tracker_in_e,
		ratio_e,
		total_wanted_e,
		completed_e,
		remaining_e,
		downloaded_e,
		uploaded_e,
		active_time_e,
		seeding_time_e,
		start_time_e,
		finish_time_e,
		managed_e,
		queue_position_e
	};
	
//	const std::wstring& filename() const { return filename_; }
	const std::wstring& name() const { return name_; }
	const std::wstring& save_directory() const { return saveDir_; }
	const std::wstring& state() const { return state_; }
	const std::wstring& current_tracker() const { return currentTracker_; }
	
	std::pair<float,float> speed() const { return speed_; }
	const float& completion() const { return completion_; }
	const float& distributed_copies() const { return distributed_copies_; }
	
	size_type total_uploaded() const { return total_uploaded_; }
	size_type total_payload_uploaded() const { return total_payload_uploaded_; }
	size_type total_downloaded() const { return total_downloaded_; }
	size_type total_payload_downloaded() const { return total_payload_downloaded_; }
	size_type total_wanted_done() const { return total_wanted_done_; }
	size_type total_wanted() const { return total_wanted_; }
	
	size_type peers() const { return peers_; }
	size_type peers_connected() const { return connected_peers_; }
	size_type seeds() const { return seeds_; }
	size_type seeds_connected() const { return connected_seeds_; }
	
	float ratio() { return ratio_; }
	
	const boost::posix_time::time_duration& estimated_time_left() { return estimated_time_left_; }
	const boost::posix_time::time_duration& update_tracker_in() { return update_tracker_in_; }
	
	const peer_details_vec& get_peer_details() const;
	const file_details_vec& get_file_details() const;
	
	const boost::posix_time::time_duration& active() { return active_; }
	const boost::posix_time::time_duration& seeding() { return seeding_; }
	const boost::posix_time::ptime& start_time() { return start_time_; }
	const boost::posix_time::ptime& finish_time() { return finish_time_; }

	int queue_position() const { return queue_position_; }
	bool managed() const { return managed_; }

	bool less(const torrent_details& r, size_t index = 0) const;
	std::wstring to_wstring(size_t index = 0);
	
public:
	std::wstring filename_;
	std::wstring name_;
	std::wstring saveDir_;
	std::wstring state_;
	std::wstring currentTracker_;

	std::pair<float,float> speed_;		
	float completion_;	
	float distributed_copies_;
	
	size_type total_wanted_done_;
	size_type total_wanted_;
	size_type total_uploaded_;
	size_type total_payload_uploaded_;
	size_type total_downloaded_;
	size_type total_payload_downloaded_;
	
	size_type peers_;
	size_type connected_peers_;
	size_type seeds_;
	size_type connected_seeds_;
	
	float ratio_;
	
	boost::posix_time::time_duration estimated_time_left_;
	boost::posix_time::time_duration update_tracker_in_;
	
	boost::posix_time::time_duration active_;
	boost::posix_time::time_duration seeding_;
	boost::posix_time::ptime start_time_;
	boost::posix_time::ptime finish_time_;

	int queue_position_;
	bool managed_;
	
private:
	mutable bool peer_details_filled_;
	mutable peer_details_vec peer_details_;
	
	mutable bool file_details_filled_;
	mutable file_details_vec file_details_;
};

typedef boost::shared_ptr<torrent_details> torrent_details_ptr;
typedef boost::scoped_ptr<torrent_details> torrent_details_sptr;
typedef boost::weak_ptr<torrent_details> torrent_details_wptr;
typedef std::vector<torrent_details_ptr> torrent_details_vec;

typedef std::set<torrent_details_ptr> torrent_details_set;
typedef std::map<std::wstring, torrent_details_ptr> torrent_details_map;

class torrent_details_manager
{
public:		
	const torrent_details_set torrents() const 
	{
		mutex_t::scoped_lock l(mutex_);	

		torrent_details_set torrents;

		for (torrent_details_map::const_iterator i = torrent_map_.begin(), e = torrent_map_.end();
				i != e; ++i)
			torrents.insert(i->second);

		return torrents; 
	}

	const torrent_details_ptr focused_torrent() const 
	{
		return get(focused_); 
	}

	const std::set<wstring> selected_names() const
	{
		mutex_t::scoped_lock l(mutex_);
		return selected_names_; 
	}
	
	const torrent_details_ptr get(std::wstring filename) const
	{
		mutex_t::scoped_lock l(mutex_);	
		
		torrent_details_map::const_iterator i = torrent_map_.find(filename);
		
		if (i != torrent_map_.end())
			return i->second;
		else
			return torrent_details_ptr();
	}
	
	friend class bit;

private:
	std::set<wstring> selected_names_;
	torrent_details_map torrent_map_;

	wstring focused_;
	
	mutable mutex_t mutex_;
};


struct tracker_detail
{
	tracker_detail() {}
	tracker_detail(std::wstring u, int t) : url(u), tier(t) {}
	
	bool operator<(const tracker_detail& t) const
	{
		return (tier < t.tier);
	}
	
	std::wstring url;
	int tier;
};

typedef std::vector<tracker_detail> tracker_details_t;



struct web_seed_or_dht_node_detail
{
	web_seed_or_dht_node_detail();
	web_seed_or_dht_node_detail(std::wstring u);
	web_seed_or_dht_node_detail(std::wstring u, int p);
		
	std::wstring url;
	int port;
	std::wstring type;
};

typedef std::vector<pair<fs::wpath, size_type> > file_size_pairs_t;



struct dht_node_detail
{
	dht_node_detail() {}
	dht_node_detail(std::wstring u, int p) : url(u), port(p) {}
	
	std::wstring url;
	int port;
};

typedef std::vector<dht_node_detail> dht_node_details_t;



struct web_seed_detail
{
	web_seed_detail() {}
	web_seed_detail(std::wstring u) : url(u) {}
	
	std::wstring url;
};

typedef std::vector<web_seed_detail> web_seed_details_t;



class EventDetail;

struct SessionDetail
{
	int port;
	
	std::pair<double, double> speed;
	
	bool dht_on;
	size_t dht_nodes;
	size_t dht_torrents;
	
	bool ip_filter_on;
	size_t ip_ranges_filtered;
};

typedef std::pair<wstring, wstring> wstring_pair;
typedef std::pair<float, float> float_pair;
typedef std::pair<int, int> int_pair;
typedef std::pair<std::vector<int>, int> vec_int_pair;

};
