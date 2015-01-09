
//         Copyright Eóin O'Callaghan 2009 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "halTorrentInternal.hpp"

#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace hal 
{

namespace sc = boost::statechart;

struct not_started;

struct out_of_session : sc::state<out_of_session, torrent_internal_sm, mpl::list< not_started > > 
{
	typedef sc::state<out_of_session, torrent_internal_sm, mpl::list< not_started > > base_type;

	typedef mpl::list<
		sc::custom_reaction<ev_add_to_session>,
		sc::custom_reaction<ev_resume>,
		sc::custom_reaction<ev_remove>,
		sc::custom_reaction<ev_added_alert>,		
		sc::custom_reaction<sc::exception_thrown>
	> reactions;

	out_of_session(base_type::my_context ctx);
	~out_of_session();	

	sc::result react(const ev_add_to_session& evt);
	sc::result react(const ev_resume& evt);
	sc::result react(const ev_remove& evt);
	sc::result react(const ev_added_alert& evt);
	sc::result react(const sc::exception_thrown& e);
};


struct invalid : sc::state<invalid, out_of_session>
{
	typedef sc::state<invalid, out_of_session> base_type;

/*	typedef mpl::list<
		sc::transition< ev_stop, stopped >,
		sc::transition< ev_resumed_alert, active >
	> reactions;
*/
	invalid(base_type::my_context ctx);
	~invalid();
};

struct active;
struct pausing;
struct paused;
struct in_error;
struct stopping;
struct removing;
struct resume_data_waiting;
struct resume_data_idling;

struct in_the_session : sc::state<in_the_session, torrent_internal_sm, mpl::list< resume_data_idling, paused > > 
{
	typedef sc::state<in_the_session, torrent_internal_sm, mpl::list< resume_data_idling, paused > > base_type;

	typedef mpl::list<
		sc::custom_reaction< ev_remove >
	> reactions;

	in_the_session(base_type::my_context ctx);
	~in_the_session();

	sc::result react(const ev_remove& evt);
};

struct resume_data_idling : sc::simple_state<resume_data_idling, in_the_session::orthogonal< 0 > >
{
	typedef mpl::list<
		sc::transition< ev_write_resume_data, resume_data_waiting >
	> reactions;

	resume_data_idling();
	~resume_data_idling();
};

struct resume_data_waiting : sc::state<resume_data_waiting, in_the_session::orthogonal< 0 > >
{
	typedef sc::state<resume_data_waiting, in_the_session::orthogonal< 0 > > base_type;

	typedef mpl::list<
		sc::transition< ev_resume_data_alert, resume_data_idling >,
		sc::transition< ev_resume_data_failed_alert, resume_data_idling >
	> reactions;

	resume_data_waiting(base_type::my_context ctx);
	~resume_data_waiting();
};

struct active : sc::state<active, in_the_session::orthogonal< 1 > >
{
	typedef sc::state<active, in_the_session::orthogonal< 1 > > base_type;

	typedef mpl::list<
		sc::transition< ev_stop, stopping >,
		sc::transition< ev_pause, pausing >,
		sc::custom_reaction< ev_force_recheck >,
		sc::transition< ev_paused_alert, paused >,
		sc::transition< ev_error_alert, in_error >
	> reactions;

	active(base_type::my_context ctx);
	~active();

	sc::result react(const ev_force_recheck& evt);
};

struct pausing : sc::state<pausing, in_the_session::orthogonal< 1 > >
{
	typedef sc::state<pausing, in_the_session::orthogonal< 1 > > base_type;

	typedef mpl::list<
		sc::transition< ev_paused_alert, paused >,
		sc::transition< ev_error_alert, in_error >
	> reactions;

	pausing(base_type::my_context ctx);
	~pausing();
};

struct paused : sc::state<paused, in_the_session::orthogonal< 1 > >
{
	typedef sc::state<paused, in_the_session::orthogonal< 1 > > base_type;

	typedef mpl::list<
		sc::custom_reaction< ev_stop >,
		sc::custom_reaction< ev_resume >,
		sc::custom_reaction< ev_force_recheck >,
		sc::transition< ev_resumed_alert, active >,
		sc::transition< ev_error_alert, in_error >
	> reactions;

	paused(base_type::my_context ctx);
	~paused();

	sc::result react(const ev_stop& evt);
	sc::result react(const ev_resume& evt);
	sc::result react(const ev_force_recheck& evt);
};

struct in_error : sc::state<in_error, in_the_session::orthogonal< 1 > >
{
	typedef sc::state<in_error, in_the_session::orthogonal< 1 > > base_type;

	typedef mpl::list<
		sc::transition< ev_stop, stopped >,
		sc::transition< ev_resumed_alert, active >
	> reactions;

	in_error(base_type::my_context ctx);
	~in_error();
};

struct stopping : sc::state<stopping, in_the_session::orthogonal< 1 > >
{
	typedef sc::state<stopping, in_the_session::orthogonal< 1 > > base_type;

	typedef mpl::list<
		sc::custom_reaction< ev_paused_alert >,
		sc::transition< ev_resume_data_alert, stopped >,
		sc::transition< ev_resume_data_failed_alert, stopped >,
		sc::transition< ev_error_alert, in_error >
	> reactions;

	stopping(base_type::my_context ctx);
	~stopping();

	sc::result react(const ev_paused_alert& evt);
};

struct not_started : sc::state<not_started, out_of_session>
{
	typedef sc::state<not_started, out_of_session> base_type;

	typedef mpl::list<
		sc::custom_reaction< ev_start >
	> reactions;

	not_started(base_type::my_context ctx);
	~not_started();

	sc::result react(const ev_start& evt);

private:
	unsigned stored_state_;
};

struct stopped : sc::state<stopped, out_of_session>
{
	typedef sc::state<stopped, out_of_session> base_type;

	stopped(base_type::my_context ctx);
	~stopped();
};

}; // namespace hal
