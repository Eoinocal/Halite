
//         Copyright Eóin O'Callaghan 2008 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include "halTorrentIntStates.hpp"

namespace hal
{

// -------- in_the_session --------

in_the_session::in_the_session(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering in_the_session()");

	assert(context<torrent_internal>().in_session());
}

sc::result in_the_session::react(const ev_remove_from_session& evt)
{
	torrent_internal& t_i = context<torrent_internal>();

	if (evt.write_data())
	{
		HAL_DEV_MSG(L"requesting resume data");
		t_i.save_resume_data();	

		return transit< leaving_session >();
	}
	else
	{
		HAL_DEV_MSG(L"removing handle from session");
		t_i.remove_torrent();

		assert(!t_i.in_session());	
		HAL_DEV_MSG(L"Removed from session!");

		return transit< out_of_session >();
	}
}

in_the_session::~in_the_session()
{
	TORRENT_STATE_LOG(L"Exiting ~in_the_session()");
}

// -------- leaving_session --------

leaving_session::leaving_session(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering leaving_session()");
}

leaving_session::~leaving_session()
{
	TORRENT_STATE_LOG(L"Exiting ~leaving_session()");
}

sc::result leaving_session::react(const ev_add_to_session& evt)
{
	torrent_internal& t_i = context<torrent_internal>();
	assert(t_i.in_session());

	return transit< in_the_session >();
}

sc::result leaving_session::react(const ev_resume_data_written& evt)
{
	torrent_internal& t_i = context<torrent_internal>();

	HAL_DEV_MSG(L"Removing handle from session");
	t_i.remove_torrent();
	HAL_DEV_MSG(L"Removed from session!");

	return transit< out_of_session >();
}

// -------- out_of_session --------

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

	return transit< in_the_session >();
}

active::active(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering active()");
}

active::~active()
{
	TORRENT_STATE_LOG(L"Exiting ~active()");
}

sc::result active::react(const ev_pause& evt)
{
	context<torrent_internal>().handle_.pause();

	return transit< pausing >();
}

pausing::pausing()
{
	TORRENT_STATE_LOG(L"Entering pausing()");
}

pausing::~pausing()
{
	TORRENT_STATE_LOG(L"Exiting ~pausing()");
}

paused::paused(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering paused()");
}

paused::~paused()
{
	TORRENT_STATE_LOG(L"Exiting ~paused()");
}

stopping::stopping()
{
	TORRENT_STATE_LOG(L"Entering stopping()");
}

stopping::~stopping()
{
	TORRENT_STATE_LOG(L"Exiting ~stopping()");
}

stopped::stopped()
{
	TORRENT_STATE_LOG(L"Entering stopped()");
}

stopped::~stopped()
{
	TORRENT_STATE_LOG(L"Exiting ~stopped()");
}

resume_data_waiting::resume_data_waiting()
{
	TORRENT_STATE_LOG(L"Entering resume_data_waiting()");
}

resume_data_waiting::~resume_data_waiting()
{
	TORRENT_STATE_LOG(L"Exiting ~resume_data_waiting()");
}

resume_data_idling::resume_data_idling()
{
	TORRENT_STATE_LOG(L"Entering resume_data_idling()");
}

resume_data_idling::~resume_data_idling()
{
	TORRENT_STATE_LOG(L"Exiting ~resume_data_idling()");
}

} // namespace hal
