
//         Copyright Eóin O'Callaghan 2009 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH(FUNCTION) \
catch (const libt::invalid_handle&) \
{\
	event_log().post(shared_ptr<EventDetail>( \
		new EventInvalidTorrent(event_logger::info, event_logger::invalid_torrent, name, std::string(FUNCTION)))); \
}\
catch (const invalid_torrent& t) \
{ \
	event_log().post(shared_ptr<EventDetail>( \
		new EventInvalidTorrent(event_logger::info, event_logger::invalid_torrent, t.who(), std::string(FUNCTION)))); \
} \
catch (const access_violation& e) \
{ \
	hal::event_log().post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"Torrent property %1% access_violation (code %2$x) at %3$x. Bad address %4$x") % hal::from_utf8(FUNCTION) % e.code() % (unsigned)e.where() % (unsigned)e.badAddress(), \
			hal::event_logger::critical))); \
} \
catch (const win32_exception& e) \
{ \
	hal::event_log().post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"Torrent property %1% win32_exception (code %2$x) at %3$x") % hal::from_utf8(FUNCTION) % e.code() % (unsigned)e.where(), \
			hal::event_logger::critical))); \
} \
catch (const std::exception& e) \
{ \
	event_log().post(shared_ptr<EventDetail>( \
		new EventTorrentException(event_logger::critical, event_logger::torrentException, std::string(e.what()), name, std::string(FUNCTION)))); \
} \
catch(...) \
{ \
	hal::event_log().post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"%1% catch all") % hal::from_utf8(FUNCTION), \
			hal::event_logger::critical))); \
}

#define HAL_GENERIC_TORRENT_EXCEPTION_CATCH(TORRENT, FUNCTION) \
catch (const libt::invalid_handle&) \
{\
	event_log().post(shared_ptr<EventDetail>( \
		new EventInvalidTorrent(event_logger::info, event_logger::invalid_torrent, TORRENT, std::string(FUNCTION)))); \
}\
catch (const invalid_torrent& t) \
{\
	event_log().post(shared_ptr<EventDetail>( \
		new EventInvalidTorrent(event_logger::info, event_logger::invalid_torrent, t.who(), std::string(FUNCTION)))); \
}\
catch (const access_violation& e) \
{ \
	hal::event_log().post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"Generic Torrent %1% access_violation (code %2$x) at %3$x. Bad address %4$x (%5%)") % hal::from_utf8(FUNCTION) % e.code() % (unsigned)e.where() % (unsigned)e.badAddress() % TORRENT, \
			hal::event_logger::critical))); \
} \
catch (const win32_exception& e) \
{ \
	hal::event_log().post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"Generic Torrent %1% win32_exception (code %2$x) at %3$x (%4%)") % hal::from_utf8(FUNCTION) % e.code() % (unsigned)e.where() % TORRENT, \
			hal::event_logger::critical))); \
} \
catch (const std::exception& e) \
{ \
	event_log().post(shared_ptr<EventDetail>( \
		new EventTorrentException(event_logger::critical, event_logger::torrentException, std::string(e.what()), TORRENT, std::string(FUNCTION)))); \
} \
catch (...) \
{ \
	hal::event_log().post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"Generic Torrent %1% catch all") % hal::from_utf8(FUNCTION), \
			hal::event_logger::critical))); \
}

#define HAL_GENERIC_PIMPL_EXCEPTION_CATCH(FUNCTION) \
catch (const access_violation& e) \
{ \
	hal::event_log().post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"Generic Session Pimpl %1% access_violation (code %2$x) at %3$x. Bad address %4$x") % hal::from_utf8(FUNCTION) % e.code() % (unsigned)e.where() % (unsigned)e.badAddress(), \
			hal::event_logger::critical))); \
} \
catch (const win32_exception& e) \
{ \
	hal::event_log().post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"Generic Session Pimpl %1% win32_exception (code %2$x) at %3$x") % hal::from_utf8(FUNCTION) % e.code() % (unsigned)e.where(), \
			hal::event_logger::critical))); \
} \
catch (const std::exception& e) \
{ \
	hal::event_log().post(shared_ptr<hal::EventDetail>( \
	new hal::EventMsg(hal::wform(L"Generic Session Pimpl %1% std_exception: %2%") % hal::from_utf8(FUNCTION) % hal::from_utf8(e.what()), \
			hal::event_logger::critical))); \
} \
catch (...) \
{ \
	hal::event_log().post(shared_ptr<hal::EventDetail>( \
		new hal::EventMsg(hal::wform(L"Generic Session Pimpl %1% catch all") % hal::from_utf8(FUNCTION), \
			hal::event_logger::critical))); \
}
