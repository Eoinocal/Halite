
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "halPch.hpp"

#define HAL_EVENT_IMPL_UNIT

#ifdef TXML_ARCHIVE_LOGGING
#undef TXML_ARCHIVE_LOGGING
#endif

#include "halEvent.hpp"
#include "../res/resource.h"


namespace hal
{

struct event_impl
{
	event_impl() : debug_(false) {}

	mutable mutex_t mutex_;

	bool debug_;
	boost::signals2::signal<void (boost::shared_ptr<EventDetail>)> event_signal_;
};

event_logger::event_logger()
{
	init();
}

void event_logger::init()
{
	static boost::shared_ptr<event_impl> s_event_impl;

	if (!s_event_impl)
		s_event_impl.reset(new event_impl());

	pimpl_ = s_event_impl;
}

event_logger::~event_logger()
{}

boost::signals2::connection event_logger::attach(boost::function<void (boost::shared_ptr<EventDetail>)> fn)
{
	if (pimpl_)
	{
		unique_lock_t l(pimpl_->mutex_);
		return pimpl_->event_signal_.connect(fn);
	}
	else
		return boost::signals2::connection();
}

void event_logger::dettach(const boost::signals2::connection& c)
{
	if (pimpl_)
	{
		unique_lock_t l(pimpl_->mutex_);
		pimpl_->event_signal_.disconnect(c);
	}
}

void event_logger::set_debug_logging(bool d)
{
	pimpl_->debug_ = d;
}

void event_logger::post(boost::shared_ptr<EventDetail> e)
{
	if (pimpl_)
	{
		unique_lock_t l(pimpl_->mutex_);

		if (pimpl_->debug_ || !(e->level() == hal::event_logger::debug || e->level() == hal::event_logger::dev))
			pimpl_->event_signal_(e);
	}
}
	
std::wstring event_logger::eventLevelToStr(eventLevel event)
{
	switch (event)
	{
	case dev:
		return L"Dev";
	case debug:
		return hal::app().res_wstr(HAL_EVENTDEBUG);
	case info:
		return hal::app().res_wstr(HAL_EVENTINFO);
	case warning:
		return hal::app().res_wstr(HAL_EVENTWARNING);
	case critical:
		return hal::app().res_wstr(HAL_EVENTCRITICAL);
	case fatal:
		return hal::app().res_wstr(HAL_EVENTCRITICAL);
	case xml_dev:
		return L"XML Log";
	case torrent_dev:
		return L"Torrent Log";
	default:
		return hal::app().res_wstr(HAL_EVENTNONE);
	}
}

} // namespace hal
