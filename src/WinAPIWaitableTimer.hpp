
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/function.hpp>
#include <boost/cstdint.hpp>

inline void CALLBACK TimerProc(void* lpParametar, BOOLEAN TimerOrWaitFired);

class WinAPIWaitableTimer
{
public:
	WinAPIWaitableTimer() :
		timer_(INVALID_HANDLE_VALUE)
	{}
	
	~WinAPIWaitableTimer()
	{
		if (timer_ != INVALID_HANDLE_VALUE) deleteTimer();
	}
	
	bool reset(unsigned dueTime, unsigned period, boost::function<void ()> fn)
	{
		if (timer_ != INVALID_HANDLE_VALUE) deleteTimer();
		
		fn_ = fn;
		
		return (::CreateTimerQueueTimer(
			&timer_,
			NULL,
			&TimerProc,
			(LPVOID)this,
			dueTime, 
			period, 
			WT_EXECUTEINTIMERTHREAD) == TRUE);
	}

private:
	void deleteTimer()
	{
		::DeleteTimerQueueTimer(NULL, timer_, NULL);  
		::CloseHandle (timer_);
		timer_ = INVALID_HANDLE_VALUE;	
	}
	
	HANDLE timer_;
	boost::function<void ()> fn_;
	
	friend void CALLBACK TimerProc(void* lpParametar, BOOLEAN TimerOrWaitFired);
};

inline void CALLBACK TimerProc(void* lpParametar, BOOLEAN TimerOrWaitFired)
{
	WinAPIWaitableTimer* timer_event = (WinAPIWaitableTimer*)lpParametar;
	
	timer_event->fn_();
}
