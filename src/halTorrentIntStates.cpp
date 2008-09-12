
//         Copyright Eóin O'Callaghan 2008 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include "halTorrentInternal.hpp"


#ifndef HAL_TORRENT_STATE_LOGGING
#	define TORRENT_STATE_LOG(s)
#else
#	include "../halEvent.hpp"
#	define TORRENT_STATE_LOG(msg) \
	hal::event_log.post(boost::shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(msg, hal::event_logger::torrent_dev))) 
#endif

namespace hal
{

torrent_internal::in_the_session::in_the_session()
{
	TORRENT_STATE_LOG(L"Entering in_the_session()");
}

torrent_internal::in_the_session::~in_the_session()
{
	TORRENT_STATE_LOG(L"Exiting ~in_the_session()");
}

torrent_internal::paused::paused()
{
	TORRENT_STATE_LOG(L"Entering paused()");
}

torrent_internal::paused::~paused()
{
	TORRENT_STATE_LOG(L"Exiting ~paused()");
}

torrent_internal::active::active()
{
	TORRENT_STATE_LOG(L"Entering active()");
}

torrent_internal::active::~active()
{
	TORRENT_STATE_LOG(L"Exiting ~active()");
}

} // namespace hal
