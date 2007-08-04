
#pragma once

#include <boost/function.hpp>
#include <boost/cstdint.hpp>

inline VOID CALLBACK TimerAPCProc(LPVOID, DWORD, DWORD);

class WinAPIWaitableTimer
{
public:
	WinAPIWaitableTimer()
	{
		timer_ = ::CreateWaitableTimer(NULL, true, NULL);
	}
	
	~WinAPIWaitableTimer()
	{
		::CloseHandle(timer_);
	}
	
	bool reset(unsigned period, boost::function<void ()> fn)
	{
		LARGE_INTEGER liDueTime;
		liDueTime.QuadPart = -((int)1000 * 10000);

		if(::SetWaitableTimer(timer_, &liDueTime, 0, &TimerAPCProc, (LPVOID)this, 0))
		{
			fn_ = fn;			
			return true;
		}
		else return false;
	}

private:
	HANDLE timer_;
	boost::function<void ()> fn_;
	boost::int64_t timeout_;
	
	friend VOID CALLBACK TimerAPCProc(LPVOID, DWORD, DWORD);
};

inline VOID CALLBACK TimerAPCProc(LPVOID lpArgToCompletionRoutine, 
	DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	WinAPIWaitableTimer* timer_event = (WinAPIWaitableTimer*)lpArgToCompletionRoutine;
	
	timer_event->fn_();
}

