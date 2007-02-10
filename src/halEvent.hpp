
#pragma once

#include <string>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/smart_ptr.hpp>

#include "halTorrent.hpp"

namespace hal 
{

class EventDetail
{
public:
	EventDetail(BitTorrent::eventLevel l, boost::posix_time::ptime t, unsigned c) :
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

	BitTorrent::eventLevel level() { return level_; }
	boost::posix_time::ptime timeStamp() { return timeStamp_; }
	unsigned code() { return code_; }
	
private:	
	BitTorrent::eventLevel level_;
	boost::posix_time::ptime timeStamp_;
	unsigned code_;
};

class EventLibtorrent : public EventDetail
{
public:
	EventLibtorrent(BitTorrent::eventLevel l, boost::posix_time::ptime t, std::wstring m) :
		EventDetail(l, t, BitTorrent::debug),
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
	EventPeerAlert(BitTorrent::eventLevel l, boost::posix_time::ptime t, std::wstring m) :
		EventDetail(l, t, BitTorrent::debug),
		msg_(m)
	{}
	
	virtual std::wstring msg()
	{
		return (wformat(hal::app().load_res_wstring(HAL_PEERALERT)) % msg_).str();
	}
	
private:
	std::wstring msg_;
};

class EventSession : public EventDetail
{

};

}// namespace hal
