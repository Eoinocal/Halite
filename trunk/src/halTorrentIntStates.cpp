
//         Copyright Eóin O'Callaghan 2008 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include "halTorrentInternal.hpp"

namespace hal
{

in_the_session::in_the_session(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering in_the_session()");

	assert(context<torrent_internal>().in_session());
}

sc::result in_the_session::react(const ev_remove_from_session& evt)
{
	torrent_internal& t_i = context<torrent_internal>();

	HAL_DEV_MSG(L"removing handle from session");
	t_i.the_session_->remove_torrent(t_i.handle_);
	t_i.in_session_ = false;

	assert(!t_i.in_session());	
	HAL_DEV_MSG(L"Removed from session!");

	return transit< out_of_session >();
}

in_the_session::~in_the_session()
{
	TORRENT_STATE_LOG(L"Exiting ~in_the_session()");
}

out_of_session::out_of_session(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering out_of_session()");
}

out_of_session::~out_of_session()
{
	TORRENT_STATE_LOG(L"Exiting ~out_of_session()");
}

sc::result out_of_session::react(const ev_add_to_session& evt)
{
	TORRENT_STATE_LOG(L"Entering in_the_session()");
	torrent_internal& t_i = context<torrent_internal>();

	assert(!t_i.in_session());

	libt::add_torrent_params p;

	string torrent_file = to_utf8((hal::app().get_working_directory()/L"torrents"/t_i.filename_).string());
	t_i.info_memory_.reset(new libt::torrent_info(torrent_file.c_str()));

	std::string resume_file = to_utf8((hal::app().get_working_directory()/L"resume" / (t_i.name_ + L".fastresume")).string());

	std::vector<char> buf;
	if (libt::load_file(resume_file.c_str(), buf) == 0)
	{
		HAL_DEV_MSG(L"Using resume data");
		p.resume_data = &buf;
	}

	p.ti = t_i.info_memory_;
	p.save_path = path_to_utf8(t_i.save_directory_);
	p.storage_mode = hal_allocation_to_libt(t_i.allocation_);
	p.paused = evt.pause();
	p.duplicate_is_error = false;
	p.auto_managed = t_i.managed_;

	t_i.handle_ = t_i.the_session_->add_torrent(p);		
	assert(t_i.handle_.is_valid());
	t_i.in_session_ = true;
	
//	clear_resume_data();
//	handle_.force_reannounce();

	return transit< in_the_session >();
}

paused::paused()
{
	TORRENT_STATE_LOG(L"Entering paused()");
}

paused::~paused()
{
	TORRENT_STATE_LOG(L"Exiting ~paused()");
}

active::active()
{
	TORRENT_STATE_LOG(L"Entering active()");
}

active::~active()
{
	TORRENT_STATE_LOG(L"Exiting ~active()");
}

} // namespace hal
