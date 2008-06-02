
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"
#include "Halite.hpp"

#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <map>
#include <algorithm>
#include <string>
#include <vector>

#include <libtorrent/file.hpp>
#include <libtorrent/hasher.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/ip_filter.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/peer_connection.hpp>

#include "halEvent.hpp"

namespace hal
{

void Event::post(boost::shared_ptr<EventDetail> event)
{
	if (event->level() != hal::Event::debug || halite().logDebug())
		signal(event);
}
	
std::wstring Event::eventLevelToStr(eventLevel event)
{
	switch (event)
	{
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
	default:
		return hal::app().res_wstr(HAL_EVENTNONE);
	}
}

Event& event()
{
	static Event e;
	return e;
}

} // namespace hal
