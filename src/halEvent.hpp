
#pragma once

#include <string>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/smart_ptr.hpp>

#include "halTorrent.hpp"

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
		EventDetail(Event::debug, boost::posix_time::second_clock::universal_time(), HAL_EVENT_XMLEXP),
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

class EventSession : public EventDetail
{

};

}// namespace hal
