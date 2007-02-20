
#pragma once

#include <string>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/smart_ptr.hpp>

#include "halTorrent.hpp"
#include "global/string_conv.hpp"

namespace hal 
{

class Event
{
public:
	enum eventLevel { debug, info, warning, critical, fatal, none };
	static std::wstring eventLevelToStr(eventLevel);
	
	void post(boost::shared_ptr<EventDetail> event)
	{
		event_signal_(event);
	}
	
	boost::signals::scoped_connection attach(boost::function<void (shared_ptr<EventDetail>)> fn)
	{
		return event_signal_.connect(fn);
	}

private:
	boost::signal<void (shared_ptr<EventDetail>)> event_signal_;
};

Event& event();

class EventDetail
{
public:
	EventDetail(Event::eventLevel l, boost::posix_time::ptime t, unsigned c) :
		level_(l),
		timeStamp_(t),
		code_(c)
	{}
	virtual ~EventDetail() 
	{}
	
	virtual std::wstring msg()
	{
		return (wformat(L"Code %1%") % code()).str();
	}

	Event::eventLevel level() { return level_; }
	boost::posix_time::ptime timeStamp() { return timeStamp_; }
	unsigned code() { return code_; }
	
private:	
	Event::eventLevel level_;
	boost::posix_time::ptime timeStamp_;
	unsigned code_;
};

class EventLibtorrent : public EventDetail
{
public:
	EventLibtorrent(Event::eventLevel l, boost::posix_time::ptime t, std::wstring m) :
		EventDetail(l, t, Event::debug),
		msg_(m)
	{}
	
	virtual std::wstring msg()
	{
		return (wformat(L"Code %1%, %2%") % code() % msg_).str();
	}
	
private:
	std::wstring msg_;
};

class EventPeerAlert : public EventDetail
{
public:
	EventPeerAlert(Event::eventLevel l, boost::posix_time::ptime t, std::wstring m) :
		EventDetail(l, t, Event::debug),
		msg_(m)
	{}
	
	virtual std::wstring msg()
	{
		return (wformat(hal::app().load_res_wstring(HAL_PEERALERT)) % msg_).str();
	}
	
private:
	std::wstring msg_;
};

class EventXmlException : public EventDetail
{
public:
	EventXmlException(std::wstring e, std::wstring m) :
		EventDetail(Event::info, boost::posix_time::second_clock::universal_time(), HAL_EVENT_XMLEXP),
		exp_(e),
		msg_(m)
	{}
	
	virtual std::wstring msg()
	{
		return (wformat(hal::app().load_res_wstring(HAL_EVENT_XMLEXP)) % exp_ % msg_).str();
	}
	
private:
	std::wstring exp_;
	std::wstring msg_;
};

class EventInvalidTorrent : public EventDetail
{
public:
	EventInvalidTorrent(Event::eventLevel l, unsigned code, std::string t, std::string f) :
		EventDetail(l, boost::posix_time::second_clock::universal_time(), code),
		torrent_(hal::to_wstr(t)),
		function_(hal::to_wstr(f)),
		code_(code)
	{}
	
	virtual std::wstring msg()
	{
		return (wformat(hal::app().load_res_wstring(code_)) % torrent_).str();
	}
	
private:
	unsigned code_;
	std::wstring function_;
	std::wstring torrent_;
	std::wstring exception_;
};

class EventTorrentException : public EventDetail
{
public:
	EventTorrentException(Event::eventLevel l, unsigned code, std::string e, std::string t, std::string f) :
		EventDetail(l, boost::posix_time::second_clock::universal_time(), code),
		torrent_(hal::to_wstr(t)),
		function_(hal::to_wstr(f)),
		exception_(hal::to_wstr(e)),
		code_(code)
	{}
	
	virtual std::wstring msg()
	{
		return (wformat(hal::app().load_res_wstring(code_)) % torrent_ % exception_).str();
	}
	
private:
	unsigned code_;
	std::wstring torrent_;
	std::wstring function_;
	std::wstring exception_;
};

class EventSession : public EventDetail
{

};

}// namespace hal
