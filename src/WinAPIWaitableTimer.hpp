
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
		
		return ::CreateTimerQueueTimer(
			&timer_,
			NULL,
			&TimerProc,
			(LPVOID)this,
			dueTime, 
			period, 
			WT_EXECUTEINTIMERTHREAD);
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
