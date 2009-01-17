
//         Copyright Eóin O'Callaghan 2009 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "halTorrentInternal.hpp"

#ifndef HAL_TORRENT_STATE_LOGGING
#	define TORRENT_STATE_LOG(s)
#else
#	include "../halEvent.hpp"
#	define TORRENT_STATE_LOG(msg) \
	hal::event_log.post(boost::shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(msg, hal::event_logger::torrent_dev))) 
#endif

#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace hal 
{

namespace sc = boost::statechart;

struct stopped;

struct out_of_session : sc::state<out_of_session, torrent_internal, mpl::list< stopped > > 
{
	typedef sc::state<out_of_session, torrent_internal, mpl::list< stopped > > base_type;

	typedef mpl::list<
		sc::custom_reaction< ev_add_to_session >
	> reactions;

	out_of_session(base_type::my_context ctx);
	~out_of_session();	

	sc::result react(const ev_add_to_session& evt);
};

struct leaving_session : sc::state<leaving_session, torrent_internal> 
{
	typedef sc::state<leaving_session, torrent_internal> base_type;

	typedef mpl::list<
		sc::custom_reaction< ev_add_to_session >,
		sc::custom_reaction< ev_resume_data_written >
	> reactions;

	leaving_session(base_type::my_context ctx);
	~leaving_session();	

	sc::result react(const ev_resume_data_written& evt);
	sc::result react(const ev_add_to_session& evt);
};

struct active;
struct pausing;
struct paused;
struct stopping;
struct resume_data_idling;

struct in_the_session : sc::state<in_the_session, torrent_internal, mpl::list< resume_data_idling, paused > > 
{
	typedef sc::state<in_the_session, torrent_internal, mpl::list< resume_data_idling, paused > > base_type;

	typedef mpl::list<
		sc::custom_reaction< ev_remove_from_session >
	> reactions;

	in_the_session(base_type::my_context ctx);
	~in_the_session();

	sc::result react(const ev_remove_from_session& evt);
};

struct resume_data_idling : sc::simple_state<resume_data_idling, in_the_session::orthogonal< 0 > >
{
	resume_data_idling();
	~resume_data_idling();
};

struct resume_data_waiting : sc::simple_state<resume_data_waiting, in_the_session::orthogonal< 0 > >
{
	resume_data_waiting();
	~resume_data_waiting();
};

struct active : sc::state<active, in_the_session::orthogonal< 1 > >
{
	typedef sc::state<active, in_the_session::orthogonal< 1 > > base_type;

	typedef mpl::list<
		sc::custom_reaction< ev_pause >
	> reactions;

	active(base_type::my_context ctx);
	~active();

	sc::result react(const ev_pause& evt);
};

struct pausing : sc::simple_state<pausing, in_the_session::orthogonal< 1 > >
{
	typedef sc::transition< ev_paused_alert, paused > reactions;

	pausing();
	~pausing();
};

struct paused : sc::state<paused, in_the_session::orthogonal< 1 > >
{
	typedef sc::state<paused, in_the_session::orthogonal< 1 > > base_type;

	paused(base_type::my_context ctx);
	~paused();
};

struct stopping : sc::simple_state<stopping, in_the_session::orthogonal< 1 > >
{
	stopping();
	~stopping();
};

struct stopped : sc::simple_state<stopped, out_of_session>
{
	stopped();
	~stopped();
};

}; // namespace hal
