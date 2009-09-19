
//         Copyright Eóin O'Callaghan 2009 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "halSession.hpp"

#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace hal 
{

namespace sc = boost::statechart;

struct ev_init_session : sc::event<ev_init_session>
{
public:
	ev_init_session(bool pause) :
		pause_(pause)
	{}

	const bool& pause() const { return pause_; }

private:
	bool pause_;
};

struct uninitialized : sc::state<uninitialized, bit_impl>
{
	typedef sc::state<uninitialized, bit_impl> base_type;

	typedef mpl::list<
		sc::custom_reaction< ev_init_session >
	> reactions;

	uninitialized(base_type::my_context ctx);
	~uninitialized();

	sc::result react(const ev_init_session& evt);
};

}; // namespace hal
