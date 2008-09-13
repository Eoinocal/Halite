
//         Copyright Eóin O'Callaghan 2008 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include "halTorrentInternal.hpp"

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
