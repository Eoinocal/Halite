
//         Copyright Eóin O'Callaghan 2008 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "halPch.hpp"

#include "halTorrentIntStates.hpp"

#pragma warning (push, 1)
#	include <libtorrent/magnet_uri.hpp>
#pragma warning (pop) 

#include "halEvent.hpp"
#ifndef HAL_TORRENT_STATE_LOGGING
#	define TORRENT_STATE_LOG(s)
#else
#	define TORRENT_STATE_LOG(msg) \
	hal::event_log().post(boost::shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(msg, hal::event_logger::torrent_dev))) 
#endif

namespace hal
{

class torrent_state_exception : public std::runtime_error
{
public:
	torrent_state_exception(const uuid& who, const std::string& what) :
		runtime_error(what),
		who_(who)
	{}
	
	virtual ~torrent_state_exception() throw () {}

	uuid who() const throw ()
	{
		return who_;
	}       
	
private:
	uuid who_;	
};

torrent_internal_sm::torrent_internal_sm(my_context ctx, torrent_internal_ptr ptr) :
	my_base(ctx),
	ptr_(ptr)
{
	TORRENT_STATE_LOG(L"Creating torrent_internal_sm");
}

torrent_internal_sm::~torrent_internal_sm()
{
	TORRENT_STATE_LOG(L"Destroying torrent_internal_sm");
}

// -------- in_the_session --------

in_the_session::in_the_session(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering in_the_session()");

	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		torrent_internal& t_i = *tp.get();
		upgrade_lock l(t_i.mutex_);

		{	upgrade_to_unique_lock up_l(l);

			if (!t_i.handle_.is_valid())
				throw torrent_state_exception(t_i.id(l), "Handle is invalid");

			t_i.in_session_ = true;
		}

		t_i.apply_settings(l);
	}
}

in_the_session::~in_the_session()
{
	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		torrent_internal& t_i = *tp.get();
		upgrade_lock l(t_i.mutex_);

		TORRENT_STATE_LOG(L"Removing handle from session");
		(*t_i.the_session_)->remove_torrent(t_i.handle_);

		TORRENT_STATE_LOG(L"Exiting ~in_the_session()");
	}
}

sc::result in_the_session::react(const ev_remove& evt)
{	
	TORRENT_STATE_LOG(L"in_the_session ev_remove()");	

	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		torrent_internal& t_i = *tp.get();
		upgrade_lock l(t_i.mutex_);

		if (!evt.remove_callback().empty())
		{	
			upgrade_to_unique_lock up_l(l);
			t_i.remove_callback(l) = evt.remove_callback();
		}

		post_event(ev_stop());
	}

	return discard_event();
}

// -------- out_of_session --------

out_of_session::out_of_session(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering out_of_session()");

	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
		tp->in_session_ = false;
}

out_of_session::~out_of_session()
{
	TORRENT_STATE_LOG(L"Exiting ~out_of_session()");
}

sc::result out_of_session::react(const ev_add_to_session& evt)
{
	TORRENT_STATE_LOG(hal::wform(L"Adding to session, paused - %1%") % evt.pause());
	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		torrent_internal& t_i = *tp.get();
		upgrade_lock l(t_i.mutex_);

		libt::add_torrent_params p;
		path resume_file = (hal::app().get_working_directory()/L"resume" / (t_i.name_ + L".fastresume")).wstring();

		boost::system::error_code ec;
		p.resume_data = load_file<std::vector<char>>(resume_file, ec);

		if (!p.resume_data.empty())
			{HAL_DEV_MSG(L" -- Using resume data");}

		p.save_path = path_to_utf8(t_i.save_directory_);
		p.storage_mode = hal_allocation_to_libt(t_i.allocation_);

		p.flags = 
			(evt.pause() ? libt::add_torrent_params::flag_paused : 0) | 
			(t_i.managed_ ? libt::add_torrent_params::flag_auto_managed : 0);

		// This ordering is important
		
		if (t_i.info_memory(l))
		{
			HAL_DEV_MSG(L" -- We have saved torrent info to use");
			p.ti = boost::make_shared<libt::torrent_info>(*t_i.info_memory(l));

			(*t_i.the_session_)->async_add_torrent(p);
		}
		else if (!t_i.magnet_uri_.empty())
		{
			HAL_DEV_MSG(L" -- We have a magnet uri to use");

			libt::error_code ec;
			libt::parse_magnet_uri(t_i.magnet_uri_, p, ec);
			
			if (!ec)
				(*t_i.the_session_)->async_add_torrent(p);
			else
			{
				HAL_DEV_MSG(wstring(L" -- Saved magnet URI was invalid: ") + from_utf8(ec.message()));
				throw torrent_state_exception(t_i.id(l), string("Saved magnet URI was invalid: ) " + ec.message()));
			}

		}
		else if (!t_i.hash_.is_all_zeros())
		{
			HAL_DEV_MSG(L" -- We have a saved hash to use");
			p.ti = boost::make_shared<libt::torrent_info>(t_i.hash_);		

			(*t_i.the_session_)->async_add_torrent(p);
		}
		else
		{
			HAL_DEV_MSG(L" -- We do not have any information with which to resume the torrent.");
			throw torrent_state_exception(t_i.id(l), "We do not have any information with which to resume the torrent");
		}

		t_i.state(l, torrent_details::torrent_starting);
	}

	return discard_event();
}

sc::result out_of_session::react(const sc::exception_thrown& e)
{
	try
	{
		throw;
	}
	catch (const torrent_state_exception& e)
	{
		event_log().post(shared_ptr<EventDetail>(new EventMsg(wform(L"Torrent State Exception: %1%") % e.what())));
	}
	catch (const std::exception& e)
	{
		event_log().post(shared_ptr<EventDetail>(new EventMsg(wform(L"Torrent Std Exception: %1%") % e.what())));
	}
	
	return transit<invalid>();
}

sc::result out_of_session::react(const ev_resume& evt)
{
	post_event(ev_add_to_session(false));

	return discard_event();
}

sc::result out_of_session::react(const ev_added_alert& evt)
{
	if (evt.ec)
	{
		HAL_DEV_MSG(L"Torrent add error");
		return transit<in_error>();
	}
	else if (evt.paused)
		return transit<paused>();
	else
		return transit<active>();
}	

sc::result out_of_session::react(const ev_remove& evt)
{	
	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		torrent_internal& t_i = *tp.get();
		upgrade_lock l(t_i.mutex_);

		if (!evt.remove_callback().empty())

		{	TORRENT_STATE_LOG(L"Calling remove_callback");
			thread_t t(evt.remove_callback());

			evt.clear_callback();
		}
	}

	return discard_event();
}

invalid::invalid(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering invalid()");
	
	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		torrent_internal& t_i = *tp.get();
		upgrade_lock l(t_i.mutex_);

		t_i.state(l, torrent_details::torrent_invalid);
	}
}

invalid::~invalid()
{
	TORRENT_STATE_LOG(L"Exiting ~invalid()");
}

active::active(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering active()");
	
	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		torrent_internal& t_i = *tp.get();
		upgrade_lock l(t_i.mutex_);

		t_i.state(l, torrent_details::torrent_active);
		t_i.handle_.resume();
	}
}

active::~active()
{
//	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
//		tp->handle_.pause();

	TORRENT_STATE_LOG(L"Exiting ~active()");
}

sc::result active::react(const ev_force_recheck& evt)
{
	TORRENT_STATE_LOG(L"React active::react(const ev_force_recheck& evt)");

	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
		tp->handle_.force_recheck();

	return discard_event();
}

pausing::pausing(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering pausing()");
	
	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		tp->handle_.pause();
		torrent_internal& t_i = *tp.get();
		upgrade_lock l(t_i.mutex_);

		t_i.state(l, torrent_details::torrent_pausing);
	}
}

pausing::~pausing()
{
	TORRENT_STATE_LOG(L"Exiting ~pausing()");
}

paused::paused(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering paused()");
	
	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		torrent_internal& t_i = *tp.get();
		upgrade_lock l(t_i.mutex_);

		t_i.state(l, torrent_details::torrent_paused);

		post_event(ev_write_resume_data());
	}
}

paused::~paused()
{
	TORRENT_STATE_LOG(L"Exiting ~paused()");
}

sc::result paused::react(const ev_stop& evt)
{
	if (state_downcast<const resume_data_waiting*>() != 0 )
		return transit<stopping>();
	else
		return transit<stopped>();
}

sc::result paused::react(const ev_resume& evt)
{
	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
		tp->handle_.resume();

	return transit<active>();
}

sc::result paused::react(const ev_force_recheck& evt)
{
	TORRENT_STATE_LOG(L"React paused::react(const ev_force_recheck& evt)");

	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
		tp->handle_.force_recheck();

	return discard_event();
}

in_error::in_error(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		torrent_internal& t_i = *tp.get();
		upgrade_lock l(t_i.mutex_);

		TORRENT_STATE_LOG(hal::wform(L"Entering in_error()() - %1%") % t_i.check_error(l));

		t_i.state(l, torrent_details::torrent_in_error);
	}
}

in_error::~in_error()
{
	TORRENT_STATE_LOG(L"Exiting ~in_error()");
}

stopping::stopping(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering stopping()");

	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		tp->handle_.pause();
		torrent_internal& t_i = *tp.get();
		upgrade_lock l(t_i.mutex_);

		t_i.state(l, torrent_details::torrent_stopping);
	}
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
	
	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		torrent_internal& t_i = *tp.get();
		boost::function<void()> callback;

		{	upgrade_lock l(t_i.mutex_);

			t_i.state(l, torrent_details::torrent_stopped);
			callback = t_i.remove_callback(l);
		}

		if (!callback.empty())
		{	
			TORRENT_STATE_LOG(L"Calling removed_callback_");
			callback();
		}
	}
}

stopped::~stopped()
{
	TORRENT_STATE_LOG(L"Exiting ~stopped()");
}

not_started::not_started(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		torrent_internal& t_i = *tp.get();
		upgrade_lock l(t_i.mutex_);

		stored_state_ = t_i.state(l);
		t_i.state(l, torrent_details::torrent_not_started);

		TORRENT_STATE_LOG(hal::wform(L"Entering not_started() - %1%") % stored_state_);
	}
}

not_started::~not_started()
{
	TORRENT_STATE_LOG(L"Exiting ~not_started()");
}

sc::result not_started::react(const ev_start& evt)
{
	TORRENT_STATE_LOG(L"React not_started::react(const ev_start& evt)");
	
	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		torrent_internal& t_i = *tp.get();
		{	upgrade_lock l(t_i.mutex_);

			if (!t_i.info_memory(l) && !t_i.name_.empty())
			{		
				path torrent_info_file = (hal::app().get_working_directory()/L"resume" / (t_i.name_ + L".torrent_info"));
				path torrent_file = (hal::app().get_working_directory()/L"torrents"/t_i.filename_);

				if (fs::exists(torrent_info_file))
				{
					upgrade_to_unique_lock up_l(l);

					try {

					HAL_DEV_MSG(hal::wform(L"Using torrent info data file %1%") % torrent_info_file);
					t_i.info_memory_reset(boost::make_shared<libt::torrent_info>(path_to_utf8(torrent_info_file)), l);

					}
					catch (const libt::libtorrent_exception&)
					{
						HAL_DEV_MSG(L"   invalid torrent file!");
					}
				}
				else if (fs::exists(torrent_file))
				{
					upgrade_to_unique_lock up_l(l);

					HAL_DEV_MSG(L"Using torrent file");
					t_i.info_memory_reset(boost::make_shared<libt::torrent_info>(path_to_utf8(torrent_file)), l);
				}
			}

			if (t_i.hash_str_.empty())
			{
				t_i.extract_hash(l);
				t_i.update_manager(l);
			}
		}
	
		HAL_DEV_MSG(hal::wform(L"Stored state %1%") % stored_state_);

		switch (stored_state_)
		{
		case torrent_details::torrent_active:
		case torrent_details::torrent_starting:
			post_event(ev_add_to_session(false));
			break;

		case torrent_details::torrent_paused:
		case torrent_details::torrent_pausing:
			post_event(ev_add_to_session(true));
			break;
		
		case torrent_details::torrent_stopped:
		case torrent_details::torrent_stopping:		
		case torrent_details::torrent_in_error:
		case torrent_details::torrent_not_started:		
		case torrent_details::torrent_invalid:		
			return transit<stopped>();
		};

	}

	return discard_event();
}

resume_data_waiting::resume_data_waiting(base_type::my_context ctx) :
	base_type::my_base(ctx)
{
	TORRENT_STATE_LOG(L"Entering resume_data_waiting()");
	
	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
	{
		tp->save_resume_and_info_data();
		tp->awaiting_resume_data_ = true;
	}
}

resume_data_waiting::~resume_data_waiting()
{
	TORRENT_STATE_LOG(L"Exiting ~resume_data_waiting()");
	
	if (torrent_internal_ptr tp = context<torrent_internal_sm>().ptr_.lock())
		tp->awaiting_resume_data_ = false;
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
