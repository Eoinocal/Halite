
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define HAL_EVENT_BEGIN				81000
#define HAL_EVENT_EXP				HAL_EVENT_BEGIN + 1
#define HAL_EVENT_XML_EXP			HAL_EVENT_BEGIN + 2
#define HAL_EVENT_UNICODE_EXP		HAL_EVENT_BEGIN + 3
#define HAL_EVENT_DEBUG				HAL_EVENT_BEGIN + 4
#define HAL_EVENT_UNCLASSIFIED		HAL_EVENT_BEGIN + 5
#define HAL_EVENT_PEER				HAL_EVENT_BEGIN + 6
#define HAL_EVENT_TRACKER			HAL_EVENT_BEGIN + 7
#define HAL_EVENT_TORRENTEXP		HAL_EVENT_BEGIN + 8
#define HAL_EVENT_INVTORRENT		HAL_EVENT_BEGIN + 9
#define HAL_EVENT_DEV				HAL_EVENT_BEGIN + 10

#ifndef RC_INVOKED

#if defined(HALTORRENT_PCH)
#	include "halPch.hpp"
#else
#	include "halTypes.hpp"
#endif

#ifndef NDEBUG
#	define HAL_TORRENT_DEV_MSGES
#	define HAL_TORRENT_STATE_LOGGING
//#	define HAL_SORT_LOGGING
//#	define TXML_ARCHIVE_LOGGING
#endif

#include <string>
#include <vector>

#include <loki/Singleton.h>

#include "global/wtl_app.hpp"
#include "global/string_conv.hpp"

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include "win32_exception.hpp"

#include "halTorrent.hpp"

#ifdef HAL_TORRENT_DEV_MSGES
#	define HAL_DEV_MSG(msg) \
	hal::event_log().post(boost::shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(msg, hal::event_logger::dev))) 
#else
#	define HAL_DEV_MSG(msg)
#endif

#ifdef HAL_SORT_LOGGING
#	define HAL_DEV_SORT_MSG(msg) \
	hal::event_log().post(boost::shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(msg, hal::event_logger::dev))) 
#else
#	define HAL_DEV_SORT_MSG(msg)
#endif

#define HAL_FILESYSTEM_EXCEPTION_CATCH(FN_MSG) \
	catch (const boost::filesystem::filesystem_error& e) \
	{ \
		if (!e.path1().empty()) \
		{ \
			hal::event_log().post(shared_ptr<hal::EventDetail>( \
				new hal::EventMsg(hal::wform(L"File related error %1%. %2% with %3%") \
						% FN_MSG \
						% hal::from_utf8(e.what()) \
						% e.path1().wstring(), \
					hal::event_logger::warning))); \
		} \
		else \
		{ \
			hal::event_log().post(shared_ptr<hal::EventDetail>( \
				new hal::EventMsg(hal::wform(L"Filesystem related error %1%. %2%") \
						% FN_MSG \
						% hal::from_utf8(e.what()), \
					hal::event_logger::warning))); \
		} \
	} 

#define HAL_NUMERIC_EXCEPTION_CATCH(FN_MSG) \
	catch (const boost::numeric::positive_overflow& e) \
	{ \
		hal::event_log().post(shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(hal::wform(L"Numeric positive overflow %1%. %2%") \
					% FN_MSG \
					% hal::from_utf8(e.what()), \
				hal::event_logger::warning))); \
	} \
	catch (const boost::numeric::negative_overflow& e) \
	{ \
		hal::event_log().post(shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(hal::wform(L"Numeric negative overflow in %1%. %2%") \
					% FN_MSG \
					% hal::from_utf8(e.what()), \
				hal::event_logger::warning))); \
	} \
	catch (const boost::numeric::bad_numeric_cast& e) \
	{ \
		hal::event_log().post(shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(hal::wform(L"Bad numeric cast %1%. %2%") \
					% FN_MSG \
					% hal::from_utf8(e.what()), \
				hal::event_logger::warning))); \
	}

#define HAL_GENERIC_FN_EXCEPTION_CATCH(FN_MSG) \
	catch (const access_violation& e) \
	{ \
		hal::event_log().post(shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(hal::wform(L"An access_violation %1% (code %2$x) at %3$x. Bad address %4$x") \
					% FN_MSG \
					% e.code() \
					% (unsigned)e.where() \
					% (unsigned)e.badAddress(), \
				hal::event_logger::critical))); \
	} \
	catch (const win32_exception& e) \
	{ \
		hal::event_log().post(shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(hal::wform(L"A win32_exception %1% (code %2$x) at %3$x") \
					% FN_MSG \
					% e.code() \
					% (unsigned)e.where(), \
				hal::event_logger::critical))); \
	} \
	catch (const hal::invalid_torrent&) \
	{ \
		hal::event_log().post(shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(hal::wform(L"Invalid torrent %1%") \
					% FN_MSG, \
				hal::event_logger::debug))); \
	} \
	catch (const std::exception& e) \
	{ \
		hal::event_log().post(shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(hal::wform(L"Std exception %1%, %2%") \
					% FN_MSG \
					% hal::from_utf8(e.what()), \
				hal::event_logger::critical))); \
	} \
	catch (...) \
	{ \
		hal::event_log().post(shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(hal::wform(L"Catch all exception %1%") \
					% FN_MSG, \
				hal::event_logger::critical))); \
	}

#define HAL_ALL_EXCEPTION_CATCH(FN_MSG) \
	HAL_NUMERIC_EXCEPTION_CATCH(FN_MSG) \
	HAL_FILESYSTEM_EXCEPTION_CATCH(FN_MSG) \
	HAL_GENERIC_FN_EXCEPTION_CATCH(FN_MSG)

namespace hal 
{

struct event_impl;

class event_logger : private boost::noncopyable
{	
public:	
	enum eventLevel { dev, xml_dev, torrent_dev, debug, info, warning, critical, fatal, none };
	
	enum codes {
		noEvent = 0,
		unclassified = HAL_EVENT_UNCLASSIFIED,
		debugEvent = HAL_EVENT_DEBUG,
		devEvent = HAL_EVENT_DEV,
		invalid_torrent = HAL_EVENT_INVTORRENT,
		torrentException = HAL_EVENT_TORRENTEXP,
		generalException = HAL_EVENT_EXP,
		xmlException = HAL_EVENT_XML_EXP,
		unicodeException = HAL_EVENT_UNICODE_EXP,
		peer = HAL_EVENT_PEER,
		tracker = HAL_EVENT_TRACKER,
		infoCode
	};
	
	event_logger();
	~event_logger();

	void init();

	bool is_active() { return static_cast<bool>(pimpl_); }
	void set_debug_logging(bool d);

	boost::signals2::connection attach(boost::function<void (boost::shared_ptr<EventDetail>)> fn);
	void dettach(const boost::signals2::connection& c);

	void post(boost::shared_ptr<EventDetail> e);
	
	static std::wstring eventLevelToStr(eventLevel);

private:
	boost::shared_ptr<event_impl> pimpl_;
};

typedef Loki::SingletonHolder<event_logger, Loki::CreateUsingNew, Loki::PhoenixSingleton> event_log_single;

inline event_logger& event_log()
{
	return event_log_single::Instance();
}

class EventDetail
{
public:
	EventDetail(event_logger::eventLevel l, boost::posix_time::ptime t, event_logger::codes c) :
		level_(l),
		timeStamp_(t),
		code_(c)
	{}
	virtual ~EventDetail() 
	{}
	
	virtual std::wstring msg()
	{
		return (wform(L"Code %1%") % code()).str();
	}

	event_logger::eventLevel level() { return level_; }
	boost::posix_time::ptime timeStamp() { return timeStamp_; }
	event_logger::codes code() { return code_; }
	
private:	
	event_logger::eventLevel level_;
	boost::posix_time::ptime timeStamp_;
	event_logger::codes code_;
};

class EventLibtorrent : public EventDetail
{
public:
	EventLibtorrent(event_logger::eventLevel l, boost::posix_time::ptime t, event_logger::codes c, std::wstring m) :
		EventDetail(l, t, c),
		msg_(m)
	{}
	
	virtual std::wstring msg()
	{
		return (wform(hal::app().res_wstr(code())) % msg_).str();
	}
	
private:
	std::wstring msg_;
};

class EventGeneral : public EventDetail
{
public:
	EventGeneral(event_logger::eventLevel l, event_logger::codes c, std::wstring m) :
		EventDetail(l, boost::posix_time::second_clock::universal_time(), c),
		msg_(m)
	{}
	
	EventGeneral(event_logger::eventLevel l, boost::posix_time::ptime t, std::wstring m) :
		EventDetail(l, t, event_logger::noEvent),
		msg_(m)
	{}
	
	template<typename str_t>
	EventGeneral(event_logger::eventLevel l, str_t m) :
		EventDetail(l, boost::posix_time::second_clock::universal_time(), event_logger::noEvent),
		msg_(hal::to_wstr_shim(m))
	{}
	
	template<typename str_t>	
	EventGeneral(event_logger::eventLevel l, boost::posix_time::ptime t, str_t m) :
		EventDetail(l, t, event_logger::noEvent),
		msg_(hal::to_wstr_shim(m))
	{}
	
	virtual std::wstring msg()
	{
		if (event_logger::noEvent != code())
			return (wform(hal::app().res_wstr(code())) % msg_).str();
		else
			return msg_;
	}
	
private:
	std::wstring msg_;
};

class EventMsg : public EventDetail
{
public:
	template<typename str_t>
	EventMsg(str_t m, event_logger::eventLevel l=event_logger::debug, 
		boost::posix_time::ptime t=boost::posix_time::second_clock::universal_time(), event_logger::codes c=event_logger::noEvent) :
		EventDetail(l, t, c),
		msg_(hal::to_wstr_shim(m))
	{}
	
	virtual std::wstring msg()
	{
		if (event_logger::noEvent != code())
			return (wform(hal::app().res_wstr(code())) % msg_).str();
		else
			return msg_;
	}
	
private:
	std::wstring msg_;
};

class EventPeerAlert : public EventDetail
{
public:
	EventPeerAlert(event_logger::eventLevel l, boost::posix_time::ptime t, std::wstring m) :
		EventDetail(l, t, event_logger::peer),
		msg_(m)
	{}
	
	virtual std::wstring msg()
	{
		return (wform(hal::app().res_wstr(code())) % msg_).str();
	}
	
private:
	std::wstring msg_;
};

class EventXmlException : public EventDetail
{
public:
	EventXmlException(std::wstring e, std::wstring m) :
		EventDetail(event_logger::warning, boost::posix_time::second_clock::universal_time(), event_logger::xmlException),
		exp_(e),
		msg_(m)
	{}
	
	virtual std::wstring msg()
	{
		return (wform(hal::app().res_wstr(HAL_EVENT_XML_EXP)) % exp_ % msg_).str();
	}
	
private:
	std::wstring exp_;
	std::wstring msg_;
};

class EventInvalidTorrent : public EventDetail
{
public:
	template<typename f_str>
	EventInvalidTorrent(event_logger::eventLevel l, event_logger::codes code, const uuid& id, f_str f) :
		EventDetail(l, boost::posix_time::second_clock::universal_time(), code),
		id_(id),
		function_(hal::to_wstr_shim(f))
	{}
	
	virtual std::wstring msg()
	{
		return (wform(hal::app().res_wstr(code())) % id_ % function_).str();
	}
	
private:
	std::wstring function_;
	uuid id_;
	std::wstring exception_;
};

class EventTorrentException : public EventDetail
{
public:
	template<typename e_str, typename f_str>
	EventTorrentException(event_logger::eventLevel l, event_logger::codes code, e_str e, const hal::uuid& id, f_str f) :
		EventDetail(l, boost::posix_time::second_clock::universal_time(), code),
		id_(id),
		function_(hal::to_wstr_shim(f)),
		exception_(hal::to_wstr_shim(e))
	{}
	
	virtual std::wstring msg()
	{
		return (wform(hal::app().res_wstr(code())) % id_ % exception_ % function_).str();
	}
	
private:
	hal::uuid id_;
	std::wstring function_;
	std::wstring exception_;
};

class EventStdException : public EventDetail
{
public:
	EventStdException(event_logger::eventLevel l, const std::exception& e, std::wstring from) :
		EventDetail(l, boost::posix_time::second_clock::universal_time(), event_logger::generalException),
		exception_(hal::from_utf8(e.what())),
		from_(from)
	{}
	
	virtual std::wstring msg()
	{
		return (wform(hal::app().res_wstr(code())) % exception_ % from_).str();
	}
	
private:
	std::wstring exception_;
	std::wstring from_;
};

class EventDebug : public EventDetail
{
public:
	EventDebug(event_logger::eventLevel l, std::wstring msg) :
		EventDetail(l, boost::posix_time::second_clock::universal_time(), event_logger::debugEvent),
		msg_(msg)
	{}
	
	virtual std::wstring msg()
	{
		return (wform(hal::app().res_wstr(code())) % msg_).str();
	}
	
private:
	std::wstring msg_;
};

class EventInfo : public EventDetail
{
public:
	template<typename T>
	EventInfo(T msg) :
		EventDetail(event_logger::info, boost::posix_time::second_clock::universal_time(), event_logger::infoCode),
		msg_(to_wstr_shim(msg))
	{}
	
	virtual std::wstring msg() { return msg_; }
	
private:
	std::wstring msg_;
};

class EventSession : public EventDetail
{

};

}// namespace hal

#endif
