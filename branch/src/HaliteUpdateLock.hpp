
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

template<class T>
class UpdateLock
{
public:
	UpdateLock(T& window) :
		window_(window)
	{
		++window_.updateLock_;
//		window_.LockWindowUpdate(true);
	}
	
	~UpdateLock()
	{
		if (!--window_.updateLock_)
			unlock();
	}
	
	void unlock()
	{
//		window_.LockWindowUpdate(false);
//		window_.InvalidateRect(NULL, true);
	}
	
private:
	T& window_;
};

template<class T>
class TryUpdateLock
{
public:
	TryUpdateLock(T& window) :
		window_(window),
		locked_(false)
	{
		if (0 == window_.updateLock_)
		{
			locked_=  true;
			++window_.updateLock_;
//			window_.LockWindowUpdate(true);
		}
	}
	
	~TryUpdateLock()
	{
		if (locked_ && !--window_.updateLock_)
			unlock();
	}
	
	void unlock()
	{
//		window_.LockWindowUpdate(false);
//		window_.InvalidateRect(NULL, true);
	}
	
	operator bool() const { return locked_; }
	
private:
	T& window_;
	bool locked_;
};
