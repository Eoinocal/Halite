
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class WinAPIMutexLock;

class WinAPIMutex
{
public:
	WinAPIMutex(std::wstring mutexName) : owner_(true)
	{
		mutex_ = ::CreateMutex(0, true, mutexName.c_str());
		
		if (mutex_ && ::GetLastError() == ERROR_ALREADY_EXISTS)
			owner_ = false;
	}
	
	~WinAPIMutex()
	{
		release();
		::CloseHandle(mutex_);
	}
	
	void release()
	{
		if (owner_) 
		{
			owner_ = false;
			::ReleaseMutex(mutex_);	
		}
	}
	
	bool owner() { return owner_; }
	
	friend class WinAPIMutexLock;

private:
	HANDLE mutex_;
	bool owner_;
};

class WinAPIMutexLock
{
public:
	explicit WinAPIMutexLock(WinAPIMutex& mutex, unsigned timeout) : 
		mutex_(mutex.mutex_), 
		locked_(false)
	{
		DWORD waitResult = ::WaitForSingleObject(mutex_, timeout);
		
		switch (waitResult) 
		{
			case WAIT_OBJECT_0: 
				locked_ = true;
			break; 
			
			case WAIT_TIMEOUT: 	
			case WAIT_ABANDONED: 
				locked_ = false;
		}	
	}
	
	~WinAPIMutexLock()
	{
		release();
	}
	
	void release() 
	{
		if (locked_) ::ReleaseMutex(mutex_);	
	}
	
	bool locked() { return locked_; }
	operator bool() { return locked_; }
	
private:
	HANDLE mutex_;
	bool locked_;
};
