
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
	context<torrent_internal>().apply_settings();
}

in_the_session::~in_the_session()
{
	torrent_internal& t_i = context<torrent_internal>();

	HAL_DEV_MSG(L"Removing handle from session");
	t_i.the_session_->remove_torrent(t_i.handle_);

	TORRENT_STATE_LOG(L"Exiting ~in_the_session()");
}

// -------- out_of_session --------

out_of_session::out_of_session(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering out_of_session()");

	context<torrent_internal>().in_session_ = false;
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

	if (evt.pause())
		return transit< paused >();
	else
		return transit< active >();

}

active::active(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering active()");

	torrent_internal& t_i = context<torrent_internal>();
	t_i.state(torrent_details::torrent_active);
}

active::~active()
{
	context<torrent_internal>().handle_.pause();

	TORRENT_STATE_LOG(L"Exiting ~active()");
}

sc::result active::react(const ev_force_recheck& evt)
{
	TORRENT_STATE_LOG(L"React active::react(const ev_force_recheck& evt)");

	context<torrent_internal>().handle_.force_recheck();

	return discard_event();
}

pausing::pausing(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering pausing()");

	torrent_internal& t_i = context<torrent_internal>();
	t_i.state(torrent_details::torrent_pausing);
}

pausing::~pausing()
{
	TORRENT_STATE_LOG(L"Exiting ~pausing()");
}

paused::paused(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering paused()");

	torrent_internal& t_i = context<torrent_internal>();
	t_i.state(torrent_details::torrent_paused);

	post_event(ev_write_resume_data());
}

paused::~paused()
{
	TORRENT_STATE_LOG(L"Exiting ~paused()");
}

sc::result paused::react(const ev_stop& evt)
{
	if (state_downcast<const resume_data_waiting*>() != 0 )
		return transit< stopping >();
	else
		return transit< stopped >();
}

sc::result paused::react(const ev_resume& evt)
{
	context<torrent_internal>().handle_.resume();

	return transit< active >();
}

sc::result paused::react(const ev_force_recheck& evt)
{
	TORRENT_STATE_LOG(L"React paused::react(const ev_force_recheck& evt)");

	context<torrent_internal>().handle_.force_recheck();

	return discard_event();
}

in_error::in_error(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	torrent_internal& t_i = context<torrent_internal>();

	TORRENT_STATE_LOG(hal::wform(L"Entering in_error()() - %1%") % t_i.check_error());

	t_i.state(torrent_details::torrent_in_error);
}

in_error::~in_error()
{
	TORRENT_STATE_LOG(L"Exiting ~in_error()");
}

stopping::stopping(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering stopping()");

	torrent_internal& t_i = context<torrent_internal>();
	t_i.state(torrent_details::torrent_stopping);
}

stopping::~stopping()
{
	TORRENT_STATE_LOG(L"Exiting ~stopping()");
}

sc::result stopping::react(const ev_paused_alert& evt)
{
	TORRENT_STATE_LOG(L"React stopping::react(const ev_paused_alert& evt)");

	post_event(ev_write_resume_data());

	return discard_event();
}

stopped::stopped(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering stopped()");

	torrent_internal& t_i = context<torrent_internal>();
	t_i.state(torrent_details::torrent_stopped);
}

stopped::~stopped()
{
	TORRENT_STATE_LOG(L"Exiting ~stopped()");
}

sc::result stopped::react(const ev_resume& evt)
{
	post_event(ev_add_to_session(false));

	return discard_event();
}

resume_data_waiting::resume_data_waiting(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering resume_data_waiting()");

	context<torrent_internal>().awaiting_resume_data_ = true;
	context<torrent_internal>().handle_.save_resume_data();
}

resume_data_waiting::~resume_data_waiting()
{
	context<torrent_internal>().awaiting_resume_data_ = false;

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
