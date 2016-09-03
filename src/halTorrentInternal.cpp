
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "halPch.hpp"

#pragma warning (push, 1)
//#	include <libtorrent/escape_string.hpp>
#	include <libtorrent/magnet_uri.hpp>
#pragma warning (pop) 

#include "halTorrentInternal.hpp"
#include "halTorrentManager.hpp"

namespace hal 
{

	
boost::optional<libt::session>* torrent_internal::the_session_ = 0;	

template<typename F>
void iterate_info_files(const libt::torrent_info& info, F&& f)
{
	for (size_t i = 0, e = info.num_files(); i<e; ++i)
		f(info.file_at(static_cast<int>(i)), i);
}

template<typename F>
void iterate_handle_files(const libt::torrent_handle& handle, F&& f)
{
	if (auto pt = handle_.torrent_file())
		iterate_info_files(*pt, std::forward<F>(f));
}

// Constructors

#define TORRENT_INTERNALS_DEFAULTS \
	transfer_limit_(std::make_pair(-1.f, -1.f)), \
	connections_(-1), \
	uploads_(-1), \
	resolve_countries_(true), \
	total_uploaded_(0), \
	total_base_(0), \
	progress_(0), \
	managed_(false), \
	start_time_(boost::posix_time::second_clock::universal_time()), \
	in_session_(false), \
	queue_position_(-1), \
	hash_(0), \
	awaiting_resume_data_(false), \
	superseeding_(false), \
	files_(mutex_, \
		bind(&torrent_internal::set_file_priority_cb, this, _1, _2, _3)) 
		

torrent_internal::torrent_internal() :	
#	pragma warning (push)
#	pragma warning (disable : 4355)
		TORRENT_INTERNALS_DEFAULTS,
#	pragma warning (pop)
	allocation_(bit::sparse_allocation)
{
	upgrade_lock l(mutex_);

	state(l, torrent_details::torrent_stopped);
}

torrent_internal::torrent_internal(const wpath& filename, const wpath& save_directory, bit::allocations alloc, const wpath& move_to_directory) :
#	pragma warning (push)
#	pragma warning (disable : 4355)
		TORRENT_INTERNALS_DEFAULTS,
#	pragma warning (pop)
	uuid_(boost::uuids::random_generator()()),
	save_directory_(save_directory),
	move_to_directory_(move_to_directory),
	allocation_(alloc)
{
	upgrade_lock l(mutex_);
		
	HAL_DEV_MSG(hal::wform(L"torrent_internal filename = %1%") % filename);

	state(l, torrent_details::torrent_stopped);
	assert(the_session_);

	prepare(l, new libt::torrent_info(path_to_utf8(filename)));
}

torrent_internal::torrent_internal(const wstring& uri, const wpath& save_directory, bit::allocations alloc, const wpath& move_to_directory) :
#	pragma warning (push)
#	pragma warning (disable : 4355)
		TORRENT_INTERNALS_DEFAULTS,
#	pragma warning (pop)
	uuid_(boost::uuids::random_generator()()),
	save_directory_(save_directory),
	move_to_directory_(move_to_directory),
	allocation_(alloc),
	magnet_uri_(to_utf8(uri))
{
	upgrade_lock l(mutex_);

	state(l, torrent_details::torrent_stopped);
	assert(the_session_);

	extract_hash(l);
}

#undef TORRENT_INTERNALS_DEFAULTS


void torrent_internal::set_the_session(boost::optional<libt::session>* s)
{
	the_session_ = s;
}

bool torrent_internal::in_session() const
{	
	upgrade_lock l(mutex_);

	return in_session(l);
}

void torrent_internal::adjust_queue_position(bit::queue_adjustments adjust)
{
	upgrade_lock l(mutex_);

	if (in_session(l) && managed_)
	{
		switch (adjust)
		{
		case bit::move_up:
			handle_.queue_position_up();
			break;
		case bit::move_down:
			handle_.queue_position_down();
			break;
		case bit::move_to_top:
			handle_.queue_position_top();
			break;
		case bit::move_to_bottom:
			handle_.queue_position_bottom();
			break;
		};
	}
}

bool torrent_internal::in_session(upgrade_lock& l) const
{ 
	return (in_session_ && the_session_ && handle_.is_valid());
}

void torrent_internal::set_managed(bool m)
{
	upgrade_lock l(mutex_);
	upgrade_to_unique_lock up_l(l);

	managed_ = m;
	
	if (in_session(l))
		handle_.auto_managed(managed_);
}

bool torrent_internal::is_finished() const
{
	upgrade_lock l(mutex_);

	return is_finished(l);
}

bool torrent_internal::is_finished(upgrade_lock& l) const
{
	if (in_session(l))
	{
		libt::torrent_status::state_t s = handle_.status().state;

		return (s == libt::torrent_status::seeding ||
					s == libt::torrent_status::finished);
	}
	else 
		return false;
}

void torrent_internal::alert_finished()
{
	upgrade_lock l(mutex_);

	if (finish_time_.is_special())
	{
		upgrade_to_unique_lock up_l(l);

		finish_time_ = boost::posix_time::second_clock::universal_time();
	}

	if (is_finished(l))
	{
		if (!move_to_directory_.empty() && 
				move_to_directory_ != path_from_utf8(handle_.status(libt::torrent_handle::query_save_path).save_path))
		{				
			upgrade_to_unique_lock up_l(l);

			handle_.move_storage(path_to_utf8(move_to_directory_));

			save_directory_ = move_to_directory_;
		}

		apply_superseeding(l);
	}
}

void torrent_internal::alert_storage_moved(const fs::path& p)
{
	HAL_DEV_MSG(hal::wform(L"alert_storage_moved = %1%") % p.wstring());
}

void torrent_internal::alert_metadata_completed()
{
	upgrade_lock l(mutex_);

	prepare(l);

	apply_settings(l);
	update_manager(l);
}

void torrent_internal::alert_file_completed(int i)
{
	upgrade_lock l(mutex_);

	files_.set_file_finished(i, l);
}

bool torrent_internal::is_active() const 
{ 
	upgrade_lock l(mutex_);

	return is_active(l); 
}

bool torrent_internal::is_active(upgrade_lock& l) const 
{ 
	return state(l) == torrent_details::torrent_active; 
}

const wstring& torrent_internal::name() const
{
	upgrade_lock l(mutex_);

	return name(l);
}

const wstring& torrent_internal::name(upgrade_lock& l) const
{ 
	if (name_.empty() && in_session(l))
	{
		upgrade_to_unique_lock up_l(l);
			
		name_ = hal::from_utf8_safe(handle_.status(libt::torrent_handle::query_name).name);
	}
	
	return name_; 
}

void torrent_internal::set_name(const wstring& n)
{
	upgrade_lock l(mutex_);

	{	upgrade_to_unique_lock up_l(l);
			
		//name_ = n;
		files_.set_root_name(n, l);
	}

	init_file_details(l);
	apply_file_names(l);
}

void torrent_internal::add_web_seed(const wstring& url, web_seed_detail::types type)
{
	upgrade_lock l(mutex_);
	
	if (type == web_seed_detail::types::url)
	{
		handle_.add_url_seed(hal::to_utf8(url));
		
		HAL_DEV_MSG(hal::wform(L"add_url_seed(%1%)") % url);
	}
	else if (type == web_seed_detail::types::http)
	{
		handle_.add_http_seed(hal::to_utf8(url));
		
		HAL_DEV_MSG(hal::wform(L"add_http_seed(%1%)") % url);
	}

	web_seeds_.clear();
}

void torrent_internal::delete_web_seed(const wstring& url, web_seed_detail::types type)
{
	upgrade_lock l(mutex_);
	
	if (type == web_seed_detail::types::url)
	{
		handle_.remove_url_seed(hal::to_utf8(url));
		
		HAL_DEV_MSG(hal::wform(L"delete_web_seed(%1%)") % url);
	}
	else if (type == web_seed_detail::types::http)
	{
		handle_.remove_http_seed(hal::to_utf8(url));
		
		HAL_DEV_MSG(hal::wform(L"delete_http_seed(%1%)") % url);
	}

	web_seeds_.clear();
}

const std::vector<web_seed_detail>& torrent_internal::get_web_seeds()
{	
	upgrade_lock l(mutex_);

	if (web_seeds_.empty() && info_memory(l))
	{
		upgrade_to_unique_lock up_l(l);	
		
		for (auto& url : handle_.url_seeds())
			web_seeds_.emplace_back(hal::from_utf8(url), web_seed_detail::types::url);
		for (auto& url : handle_.http_seeds())
			web_seeds_.emplace_back(hal::from_utf8(url), web_seed_detail::types::http);
	}

	return web_seeds_;
}

const uuid& torrent_internal::id() const
{ 
	upgrade_lock l(mutex_);

	return id(l);
}

const uuid& torrent_internal::id(upgrade_lock& l) const
{ 
	return uuid_;
}

const wpath& torrent_internal::save_directory() const
{ 
	upgrade_lock l(mutex_);

	return save_directory(l); 
}

const wpath& torrent_internal::save_directory(upgrade_lock& l) const
{ 
	return save_directory_; 
}

unsigned torrent_internal::state() const 
{
	upgrade_lock l(mutex_);

	return state(l);
}

unsigned torrent_internal::state(upgrade_lock& l) const 
{
	return state_;
}
void torrent_internal::set_superseeding(bool ss)
{
	upgrade_lock l(mutex_);

	{	upgrade_to_unique_lock up_l(l);

		superseeding_ = ss;
	}

	apply_superseeding(l);
}

bool torrent_internal::get_superseeding() const
{
	upgrade_lock l(mutex_);

	return get_superseeding(l);
}

bool torrent_internal::get_superseeding(upgrade_lock& l) const
{
	return superseeding_;
}

wstring torrent_internal::check_error() const
{		
	upgrade_lock l(mutex_);
	return check_error(l);
}

wstring torrent_internal::check_error(upgrade_lock& l) const
{
	if (in_session(l))
	{ 
		return from_utf8(renew_status_cache(l).error);
	}
	else
		return L"";
}

void torrent_internal::add_to_session(bool paused)
{
	try
	{
	upgrade_lock l(mutex_);

	HAL_DEV_MSG(hal::wform(L"add_to_session() paused=%1%") % paused);

	process_event(new ev_add_to_session(paused));

	}
	catch(std::exception& e)
	{
		hal::event_log().post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(event_logger::critical, e, L"add_to_session"))); 
	}
}

bool torrent_internal::remove_from_session(bool write_data)
{
	try
	{
	upgrade_lock l(mutex_);

	HAL_DEV_MSG(hal::wform(L"remove_from_session() write_data=%1%") % write_data);
	
	process_event(new ev_remove_from_session(write_data));

	}
	catch(std::exception& e)
	{
		hal::event_log().post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(event_logger::critical, e, L"remove_from_session()"))); 
		return false;
	}

	return true;
}

void torrent_internal::remove_files(function<void (wpath path, boost::shared_ptr<std::vector<std::wstring> > files)> fn)
{		
	boost::shared_ptr<std::vector<std::wstring> > files = boost::shared_ptr<std::vector<std::wstring> >(new std::vector<std::wstring> );

	fs::path active_directory = save_directory();

	{	upgrade_lock l(mutex_);
	
		for (int i = 0; i < info_memory(l)->num_files(); ++i)
			files->push_back((save_directory(l)/files_[i].active_name()).wstring());

		files->push_back((hal::app().get_working_directory() / L"resume" / (name(l) + L".fastresume")).wstring());
		files->push_back((hal::app().get_working_directory() / L"resume" / (name(l) + L".torrent_info")).wstring());
		
		if (1 == files_.size(l))
		{
			active_directory /= hash_str_;
		}
		else
		{
			active_directory /= name(l);
		}
	}

	process_event(new ev_remove(boost::bind(fn, active_directory, files)));
}

torrent_details_ptr torrent_internal::get_torrent_details_ptr() const
{	
	if (scoped_try_lock ll = scoped_try_lock(details_mutex_))
	{
		upgrade_lock l(mutex_);

		try
		{

		renew_status_cache(l);	
		wstring state_str = state_string(l);
		
		pt::time_duration td(pt::pos_infin);
		
		if (status_cache(l).download_payload_rate != 0)
		{
			td = boost::posix_time::seconds(	
				long(float(status_cache(l).total_wanted-status_cache(l).total_wanted_done) / status_cache(l).download_payload_rate));
		}
		
		{	upgrade_to_unique_lock up_l(l);
				
			auto& sc = status_cache(l);

			total_uploaded_ += (sc.total_payload_upload - total_base_);
			total_base_ = sc.total_payload_upload;
			
			uploaded_.update(sc.total_upload);
			payload_uploaded_.update(sc.total_payload_upload);
			downloaded_.update(sc.total_download);
			payload_downloaded_.update(sc.total_payload_download);

			// just in case these were wrong

//			managed_ = sc.auto_managed;
//			superseeding_ = sc.super_seeding;
		}
		
		if (is_active(l))
		{
			upgrade_to_unique_lock up_l(l);

			active_duration_.update();
			
			if (libt::torrent_status::seeding == status_cache(l).state)
				seeding_duration_.update();
		}	
		
		boost::tuple<size_t, size_t, size_t, size_t> connections = update_peers(l);	

		details_ptr_.reset(new torrent_details(
			name(l), filename_, 
			save_directory(l).wstring(), 
			state_str, 
			id(l),
			hal::from_utf8(status_cache(l).current_tracker), 
			hash_str_,
			std::make_pair(status_cache(l).download_payload_rate, status_cache(l).upload_payload_rate),
			progress_, 
			status_cache(l).distributed_copies, 
			status_cache(l).total_wanted_done, 
			status_cache(l).total_wanted, 
			uploaded_, payload_uploaded_,
			downloaded_, payload_downloaded_, 
			connections, 
			td, 
			status_cache(l).next_announce, 
			active_duration_, seeding_duration_, 
			start_time_, finish_time_, 
			queue_position_,
			status_cache(l).auto_managed));

		}
		catch (const libt::invalid_handle&)
		{
			event_log().post(shared_ptr<EventDetail>(
				new EventInvalidTorrent(event_logger::critical, 
					event_logger::invalid_torrent, id(l), "get_torrent_details_ptr")));

			details_ptr_.reset(new torrent_details(
				name(l), filename_, 
				save_directory(l).wstring(), 
				app().res_wstr(HAL_TORRENT_STOPPED), 
				id(l), 
				app().res_wstr(HAL_NA),
				hash_str_));
		}
		catch (const std::exception& e)
		{
			event_log().post(shared_ptr<EventDetail>(
				new EventTorrentException(event_logger::critical, 
					event_logger::torrentException, e.what(), id(l), "get_torrent_details_ptr")));

			details_ptr_.reset(new torrent_details(
				name(l), filename_, 
				save_directory(l).wstring(), 
				app().res_wstr(HAL_TORRENT_STOPPED), 
				id(l), 
				app().res_wstr(HAL_NA),
				hash_str_));
		}		
	}

	return details_ptr_;
}

file_details_vec torrent_internal::get_file_details()
{
	file_details_vec files;
	get_file_details(files);

	return files;
}

void torrent_internal::get_file_details(file_details_vec& files_vec)
{
	upgrade_lock l(mutex_);

	get_file_details(l, files_vec);
}

void torrent_internal::get_file_details(upgrade_lock& l, file_details_vec& files_vec)
{
	if (file_details_memory_.empty())
		init_file_details(l);	
	
	if (in_session(l))
	{	
		std::vector<libt::size_type> file_progress;			
		handle_.file_progress(file_progress, libt::torrent_handle::piece_granularity);
		
		{	upgrade_to_unique_lock up_l(l);

			for(size_t i=0, e=file_details_memory_.size(); i<e; ++i)
				file_details_memory_[i].progress =  file_progress[i];	
		}
	}

	files_vec = file_details_memory_;
}

void torrent_internal::init_file_details(upgrade_lock& l)
{	
	upgrade_to_unique_lock up_l(l);

	file_details_memory_.clear();
	file_priorities_.clear();

	if (info_memory(l) && !files_.empty(l))
		if (auto pt = handle_.torrent_file())
		{	
			const auto& info = *pt;

			files_.set_hash(hash_str_);
			files_.set_libt_files(info.files());

			assert(files_.size(l) == info.num_files());
			
			file_details_memory_.reserve(info.num_files());
			file_priorities_.reserve(info.num_files());

			iterate_info_files(info, [&](libt::file_entry file, size_t i)
			{
				boost::int64_t size = static_cast<boost::int64_t>(file.size);
				torrent_file::split_path_pair_t split = torrent_file::split_root(files_[i].completed_name());
			
				file_details_memory_.push_back(file_details(split.second, size, 0, files_[i].priority(), i));
				file_priorities_.push_back(files_[i].priority());
			});
		}
}

void torrent_internal::prepare(upgrade_lock& l, torrent_info_ptr info)
{	
	HAL_DEV_MSG(hal::wform(L"Prepare torrent %s") % (info ? L"true" : L"false"));

	if (info)
	{				
		set_info_cache(info, l);
				
		extract_hash(l);
		extract_names(l);
		extract_filenames(l);	

		write_torrent_info(l);

		if (!fs::exists(save_directory_))
			fs::create_directories(save_directory_);

		// These here should not make state changes based on torrent 
		// session status since it has not been initialized yet.

		if (state_ == torrent_details::torrent_stopping)
			state(l, torrent_details::torrent_stopped);
		else if (state_ == torrent_details::torrent_pausing)
			state(l, torrent_details::torrent_paused);
	}
}

void torrent_internal::update_manager(upgrade_lock& l)
{
	if (update_manager_)
	{
		l.unlock();

		update_manager_(shared_from_this());
	}
}

void torrent_internal::extract_names(upgrade_lock& l)
{
	if (info_memory(l))
	{		
		upgrade_to_unique_lock up_l(l);

		name_ = hal::from_utf8_safe(info_memory(l)->name());
		
		filename_ = name_;
		if (!boost::find_last(filename_, L".torrent")) 
				filename_ += L".torrent";
		
		event_log().post(shared_ptr<EventDetail>(new EventMsg(
			hal::wform(L"Loaded names: %1%, %2%") % name_ % filename_)));
	}
}

void torrent_internal::extract_hash(upgrade_lock& l)
{	
	HAL_DEV_MSG(L"Extracting hash...");

	if (in_session(l))
	{		
		upgrade_to_unique_lock up_l(l);

		HAL_DEV_MSG(L"    from handle");
		hash_ = handle_.info_hash();
	}
	else if (info_memory(l))
	{			
		upgrade_to_unique_lock up_l(l);

		HAL_DEV_MSG(L"    from info_memory");
		hash_ = info_memory(l)->info_hash();
	}
	else if (!magnet_uri_.empty())
	{
		HAL_DEV_MSG(L"    from magnet uri");
		libt::add_torrent_params p;
		
		p.save_path = path_to_utf8(save_directory_);
		p.storage_mode = hal_allocation_to_libt(allocation_);

		p.flags = libt::add_torrent_params::flag_paused || libt::add_torrent_params::flag_update_subscribe;
		
		libt::error_code ec;		
		libt::parse_magnet_uri(magnet_uri_, p, ec);
			
		libt::torrent_handle h = (ec) ? libt::torrent_handle() : (*the_session_)->add_torrent(p, ec);

	//	libt::torrent_handle h = libt::add_magnet_uri(**the_session_, magnet_uri_, p);
			
		{	upgrade_to_unique_lock up_l(l);

			hash_ = h.info_hash();	
		}

		HAL_DEV_MSG(L"    removing temp handle");
		(*the_session_)->remove_torrent(h);
	}
	else
	{		
		upgrade_to_unique_lock up_l(l);

		HAL_DEV_MSG(L"    No Hash!");
		hash_.clear();
	}
	
	libt::sha1_hash const& ih = hash_;

	{	upgrade_to_unique_lock up_l(l);

		hash_str_ = from_utf8(libt::base32encode(std::string((char const*)&ih[0], 20)));
	}

	files_.set_hash(hash_str_);

	HAL_DEV_MSG(hal::wform(L"    hash : %1%") % hash_str_);
}

void torrent_internal::extract_filenames(upgrade_lock& l)
{			
	if (auto info_ptr = info_memory(l))
		if (files_.empty(l))
		{
			iterate_info_files(*info_ptr, [&](libt::file_entry file, size_t i)
			{
				fs::wpath p_orig = path_from_utf8(file.path);

				int p = file_priorities_.empty() ? 1 : file_priorities_[i];

				files_.push_back(torrent_file(path_from_utf8(file.path), false, p), l);
			});
		
			// Primes the Files manager with default names.
			files_.set_libt_files(info_ptr->orig_files());
		}
		else
		{
			iterate_info_files(*info_ptr, [&](libt::file_entry file, size_t i)
			{
				fs::wpath p = path_from_utf8(file.path);

	//			assert(p == files_[std::distance(info_memory()->begin_files(), i)].original_name());
			});
		}
}

void torrent_internal::write_torrent_info(upgrade_lock& l) const
{
	try {

	if (auto info_ptr = info_memory(l))
	{
		wpath resume_dir = hal::app().get_working_directory()/L"resume";
		
		if (!exists(resume_dir))
			fs::create_directories(resume_dir);

		boost::filesystem::ofstream out(resume_dir/(name(l) + L".torrent_info"), std::ios_base::binary);
		out.unsetf(std::ios_base::skipws);

		if (info_ptr->metadata())
		{
			libt::create_torrent t(*info_ptr);
			bencode(std::ostream_iterator<char>(out), t.generate());

			HAL_DEV_MSG(L"Torrent info written!");
		}
		else
			{HAL_DEV_MSG(L"Skipping torrent with no meta data");}
	}
	else
		{HAL_DEV_MSG(L"Skipping torrent with no info");}

	} 
	catch (const boost::filesystem::filesystem_error&)
	{
		event_log().post(shared_ptr<EventDetail>(
			new EventMsg(L"Write torrent info error.", event_logger::warning)));
	}
	catch (const libt::libtorrent_exception&)
	{
		HAL_DEV_MSG(L"No torrent info");
	}
}

boost::tuple<size_t, size_t, size_t, size_t> torrent_internal::update_peers(upgrade_lock& l) const
{
	if (in_session(l))
	{
		upgrade_to_unique_lock up_l(l);

		handle_.get_peer_info(peers_);
	}
	
	size_t totalPeers = 0;
	size_t peersConnected = 0;
	size_t totalSeeds = 0;
	size_t seedsConnected = 0;
	
	BOOST_FOREACH (libt::peer_info& peer, peers_) 
	{
		float speedSum = boost::numeric_cast<float>(peer.down_speed + peer.up_speed);
		
		if (!(peer.flags & libt::peer_info::seed))
		{
			++totalPeers;
			
			if (speedSum > 0)
				++peersConnected;
		}
		else
		{
			++totalSeeds;
			
			if (speedSum > 0)
				++seedsConnected;
		}
	}	
	
	return boost::make_tuple(totalPeers, peersConnected, totalSeeds, seedsConnected);
}

// ----------------- private -----------------

void torrent_internal::apply_settings(upgrade_lock& l)
{		
	apply_transfer_speed(l);
	apply_connection_limit(l);
	apply_trackers(l);
	apply_tracker_login(l);
	apply_file_priorities(l);
	apply_resolve_countries(l);
	apply_superseeding(l);
	apply_file_names(l);
	apply_external_interface(l);
}

void torrent_internal::apply_transfer_speed(upgrade_lock& l)
{
	if (in_session(l))
	{
		int down = (transfer_limit_.first > 0) ? static_cast<int>(transfer_limit_.first*1024) : -1;
		handle_.set_download_limit(down);
		
		int up = (transfer_limit_.second > 0) ? static_cast<int>(transfer_limit_.second*1024) : -1;
		handle_.set_upload_limit(up);

		HAL_DEV_MSG(hal::wform(L"Applying Transfer Speed %1% - %2%") % down % up);
	}
}

void torrent_internal::apply_external_interface(upgrade_lock& l)
{
	if (in_session(l))
	{
		if (external_interface_)
		{
			handle_.use_interface(hal::to_utf8(*external_interface_).c_str());

			HAL_DEV_MSG(hal::wform(L"Applying external interface: %1%") % *external_interface_);
		}
		else
		{
		//	handle_.use_interface(0);

			HAL_DEV_MSG(L"Applying no custom external interface.");
		}
	}
}

void torrent_internal::apply_connection_limit(upgrade_lock& l)
{
	if (in_session(l))
	{
		handle_.set_max_connections(connections_);
		handle_.set_max_uploads(uploads_);

		HAL_DEV_MSG(hal::wform(L"Applying Connection Limit %1% - %2%") % connections_ % uploads_);
	}
}

void torrent_internal::apply_trackers(upgrade_lock& l)
{
	if (in_session(l))
	{
		if (torrent_trackers_.empty())
			torrent_trackers_ = handle_.trackers();
		
		if (!trackers_.empty())
		{
			std::vector<libt::announce_entry> trackers;
			
			BOOST_FOREACH (const tracker_detail& tracker, trackers_)
			{
				trackers.push_back(libt::announce_entry(hal::to_utf8(tracker.url)));
				trackers.back().tier = tracker.tier;
			}
			handle_.replace_trackers(trackers);
		}
		
		HAL_DEV_MSG(L"Applying Trackers");
	}
}

void torrent_internal::apply_tracker_login(upgrade_lock& l)
{
	if (in_session(l))
	{
		if (!tracker_username_.empty())
			handle_.set_tracker_login(hal::to_utf8(tracker_username_), hal::to_utf8(tracker_password_));

		HAL_DEV_MSG(hal::wform(L"Applying Tracker Login User: %1% with password") % tracker_username_);
	}
}

void torrent_internal::apply_file_priorities(upgrade_lock& l)
{
	if (in_session(l))
	{
		if (!file_priorities_.empty())
			handle_.prioritize_files(file_priorities_);
		
		HAL_DEV_MSG(L"Applying File Priorities");
	}
}

void torrent_internal::apply_file_names(upgrade_lock& l)
{
	if (in_session(l) && info_memory(l))
	{
		bool want_recheck = false;
		int know = static_cast<int>(files_.size(l));

		if (files_.size(l) != info_memory(l)->num_files())
		{
			extract_filenames(l);
			want_recheck = true;
		}
				
		if (want_recheck)
			handle_.force_recheck();
		
		HAL_DEV_MSG(L"Applying File Names");
	}
}	

void torrent_internal::apply_resolve_countries(upgrade_lock& l)
{
	if (in_session(l))
	{
		handle_.resolve_countries(resolve_countries_);
		
		HAL_DEV_MSG(hal::wform(L"Applying Resolve Countries %1%") % resolve_countries_);
	}
}

void torrent_internal::apply_superseeding(upgrade_lock& l)
{
	if (in_session(l))
	{
		handle_.super_seeding(superseeding_);
		
		HAL_DEV_MSG(hal::wform(L"Applying Superseeding %1%") % superseeding_);
	}
}

void torrent_internal::apply_queue_position()
{
	upgrade_lock l(mutex_);
	apply_queue_position(l);
}

void torrent_internal::apply_queue_position(upgrade_lock& l)
{
	if (in_session(l))
	{
		if (handle_.queue_position() != -1 && queue_position_ != -1)
		{
			while (handle_.queue_position() != queue_position_)
			{
				HAL_DEV_MSG(hal::wform(L"Queue position libtorrent %1% - Halite %2%") 
					% handle_.queue_position() % queue_position_);

				if (handle_.queue_position() < queue_position_)
					handle_.queue_position_down();
				if (handle_.queue_position() > queue_position_)
					handle_.queue_position_up();
			}
		}
	}
	HAL_DEV_MSG(L"Applying Queue Position");
}

void torrent_internal::state(upgrade_lock& l, unsigned s)
{
	switch (s)
	{
	case torrent_details::torrent_stopped:
		HAL_DEV_MSG(L"state() - stopped");
		break;
	case torrent_details::torrent_stopping:
		HAL_DEV_MSG(L"state() - stopping");
		break;
	case torrent_details::torrent_pausing:
		HAL_DEV_MSG(L"state() - pausing");
		break;
	case torrent_details::torrent_active:
		HAL_DEV_MSG(L"state() - active");
		break;
	case torrent_details::torrent_paused:
		HAL_DEV_MSG(L"state() - paused");
		break;
	case torrent_details::torrent_in_error:
		HAL_DEV_MSG(L"state() - in error");
		break;
	case torrent_details::torrent_not_started:
		HAL_DEV_MSG(L"state() - not_started");
		break;
	case torrent_details::torrent_starting:
		HAL_DEV_MSG(L"state() - starting");
		break;
	default:
		HAL_DEV_MSG(L"state() - unknown");
		break;
	};

	{	upgrade_to_unique_lock up_l(l);
			
		state_ = s;
	}
}

void torrent_internal::initialize_non_serialized(sc::fifo_scheduler<>::processor_handle h, 
	function<void (torrent_internal_ptr)> um,
	function<void (sc::fifo_scheduler<>::processor_handle, sc::fifo_scheduler<>::event_ptr_type)> pte)
{
	{	upgrade_lock l(mutex_);
	
		state_handle_ = h;
		update_manager_ = um;	
		post_torrent_event_ =pte;
		
		details_ptr_.reset(new torrent_details(
			name(l), filename_, 
			save_directory(l).wstring(), 
			app().res_wstr(HAL_TORRENT_STOPPED), 
			id(l), 
			app().res_wstr(HAL_NA),
			hash_str_));
	}
}

libt::torrent_status& torrent_internal::status_cache(upgrade_lock& l) const
{
	return status_memory_;
}

libt::torrent_status& torrent_internal::renew_status_cache(upgrade_lock& l) const
{	
	if (in_session(l) && is_active(l))
	{
		libt::torrent_status ts = handle_.status();
		
		{	upgrade_to_unique_lock up_l(l);

			status_memory_ = ts;
			progress_ = status_memory_.progress;
			queue_position_ = handle_.queue_position();
		}
	}
	else
	{
		upgrade_to_unique_lock up_l(l);

		// Wipe these cause they don't make sense for a non-active torrent.
		
		status_memory_.download_payload_rate = 0;
		status_memory_.upload_payload_rate = 0;
		status_memory_.total_payload_download = 0;
		status_memory_.total_payload_upload = 0;
		status_memory_.next_announce = boost::posix_time::seconds(0);		
	}

	return status_memory_;
}
	
wstring torrent_internal::state_string(upgrade_lock& l) const
{
	wstring state_str;
	
	switch (state(l))
	{
	case torrent_details::torrent_paused:
		state_str = app().res_wstr(HAL_TORRENT_PAUSED);
		break;
		
	case torrent_details::torrent_pausing:
		state_str = app().res_wstr(HAL_TORRENT_PAUSING);
		break;
		
	case torrent_details::torrent_stopped:
		state_str = app().res_wstr(HAL_TORRENT_STOPPED);
		break;
		
	case torrent_details::torrent_stopping:
		state_str = app().res_wstr(HAL_TORRENT_STOPPING);
		break;
		
	case torrent_details::torrent_in_error:
		state_str = app().res_wstr(HAL_TORRENT_IN_ERROR);
		break;

	case torrent_details::torrent_starting:
		state_str = app().res_wstr(HAL_TORRENT_STARTING);
		break;

	case torrent_details::torrent_invalid:
		state_str = app().res_wstr(HAL_TORRENT_INVALID);
		break;
		
	default:
		switch (status_cache(l).state)
		{
		case libt::torrent_status::queued_for_checking:
			state_str = app().res_wstr(HAL_TORRENT_QUEUED_CHECKING);
			break;
		case libt::torrent_status::checking_files:
			state_str = app().res_wstr(HAL_TORRENT_CHECKING_FILES);
			break;
//			case libt::torrent_status::connecting_to_tracker:
//				state = app().res_wstr(HAL_TORRENT_CONNECTING);
//				break;
		case libt::torrent_status::downloading_metadata:
			state_str = app().res_wstr(HAL_TORRENT_METADATA);
			break;
		case libt::torrent_status::downloading:
			state_str = app().res_wstr(HAL_TORRENT_DOWNLOADING);
			break;
		case libt::torrent_status::finished:
			state_str = app().res_wstr(HAL_TORRENT_FINISHED);
			break;
		case libt::torrent_status::seeding:
			if (!get_superseeding(l))
				state_str = app().res_wstr(HAL_TORRENT_SEEDING);
			else
				state_str = app().res_wstr(HAL_TORRENT_SUPERSEEDING);
			break;
		case libt::torrent_status::allocating:
			state_str = app().res_wstr(HAL_TORRENT_ALLOCATING);
			break;
		case libt::torrent_status::checking_resume_data:
			state_str = app().res_wstr(HAL_TORRENT_CHECKING_RESUME);
			break;
		}	
	}

	return state_str;
}

torrent_internal::torrent_info_ptr torrent_internal::info_memory(upgrade_lock& l) const
{
	try 
	{

	if (!info_memory_ && in_session(l)) 
	{	
		// Yes I know how ugly this is!!!!
		HAL_DEV_MSG(L"That ugly const_cast happened!!!");

		torrent_info_ptr iptr;

		if (auto ip = handle_.status(libt::torrent_handle::query_torrent_file).torrent_file)
			const_cast<torrent_internal*>(this)->info_memory_reset(ip, l);
		else
			return torrent_info_ptr();
	}

	}
	catch (const libt::libtorrent_exception&)
	{
		HAL_DEV_MSG(L"No torrent info");
	}

	return info_memory_;
}

void torrent_internal::info_memory_reset(torrent_info_ptr im, upgrade_lock& l)
{	
		upgrade_to_unique_lock up_l(l);

		info_memory_ = im;

		// This informs the Files manager about names loaded from a fast resume file.
		files_.set_hash(hash_str_);	
		files_.set_libt_files(info_memory_->orig_files());
}

void torrent_internal::output_torrent_debug_details() const
{
	upgrade_lock l(mutex_);
	output_torrent_debug_details(l);
}

void torrent_internal::output_torrent_debug_details(upgrade_lock& l) const
{
	HAL_DEV_MSG(wform(L"Name %1%") % name(l));
	HAL_DEV_MSG(wform(L" >> In session       %1%") % in_session(l));

	if (in_session(l))
	{

	wstring state_str;

	switch (state(l))
	{
	case torrent_details::torrent_active:
		state_str = L"Active";
		break;

	case torrent_details::torrent_not_started:
		state_str = L"Not Started!";
		break;

	case torrent_details::torrent_paused:
		state_str = app().res_wstr(HAL_TORRENT_PAUSED);
		break;
		
	case torrent_details::torrent_pausing:
		state_str = app().res_wstr(HAL_TORRENT_PAUSING);
		break;
		
	case torrent_details::torrent_stopped:
		state_str = app().res_wstr(HAL_TORRENT_STOPPED);
		break;
		
	case torrent_details::torrent_stopping:
		state_str = app().res_wstr(HAL_TORRENT_STOPPING);
		break;
		
	case torrent_details::torrent_starting:
		state_str = app().res_wstr(HAL_TORRENT_STARTING);
		break;
		
	case torrent_details::torrent_in_error:
		state_str = app().res_wstr(HAL_TORRENT_IN_ERROR);
		break;
		
	case torrent_details::torrent_invalid:
		state_str = app().res_wstr(HAL_TORRENT_INVALID);
		break;
		
	default:
		state_str = L"Unknown State!";
		break;
	};
	
	HAL_DEV_MSG(wform(L" >> State			%1%") % state_str);
//	HAL_DEV_MSG(wform(L" >> Paused in session	%1%") % handle_.is_paused());
//	HAL_DEV_MSG(wform(L" >> Error state		%1%") % check_error());

	}
}


// --- Callbacks ---

void torrent_internal::set_file_priority_cb(size_t i, int p, upgrade_lock& l)
{		
	if (i < file_priorities_.size())
	{
		upgrade_to_unique_lock up_l(l);

		file_priorities_[i] = p;
		file_details_memory_[i].priority = p;
	}
}
	
};
