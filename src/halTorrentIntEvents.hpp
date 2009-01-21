
//         Copyright Eóin O'Callaghan 2009 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "halTorrentInternal.hpp"

#include <boost/statechart/event.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace hal 
{

namespace sc = boost::statechart;

struct ev_remove_from_session : sc::event<ev_remove_from_session>
{
public:
    ev_remove_from_session(bool write_data) :
		write_data_(write_data)
    {}
   
    const bool& write_data() const { return write_data_; }

private:
    bool write_data_;
};

struct ev_add_to_session : sc::event<ev_add_to_session>
{
public:
    ev_add_to_session(bool pause) :
		pause_(pause)
    {}
   
    const bool& pause() const { return pause_; }

private:
    bool pause_;
};

struct ev_error_alert : sc::event<ev_error_alert>
{
public:
    ev_error_alert(wstring& e) :
		err_(e)
    {}
   
    const wstring& error() const { return err_; }

private:
    wstring err_;
};

struct ev_pause : sc::event< ev_pause > {};
struct ev_paused_alert : sc::event<ev_paused_alert> {};

struct ev_stop : sc::event< ev_stop > {};
struct ev_resume : sc::event< ev_resume > {};
struct ev_resumed_alert : sc::event<ev_paused_alert> {};

struct ev_force_recheck : sc::event< ev_force_recheck > {};

struct ev_write_resume_data : sc::event< ev_write_resume_data > {};
struct ev_resume_data_alert : sc::event< ev_resume_data_alert > {};
struct ev_resume_data_failed_alert : sc::event< ev_resume_data_alert > {};

} // namespace hal
