
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace hal
{

template<class T>
class mutex_update_lock
{
public:
	mutex_update_lock(T& window) :
		window_(window),
		lock_(window_.mutex_)
	{
		++window_.update_lock_;

//		window_.LockWindowUpdate(true);
	}
	
	~mutex_update_lock()
	{
		if (!--window_.update_lock_)
			unlock();
	}
	
	void unlock()
	{
//		window_.LockWindowUpdate(false);
//		window_.InvalidateRect(NULL, true);
	}
	
private:
	T& window_;

	mutex_t::scoped_lock lock_;
};

template<class T>
class try_update_lock
{
public:
	try_update_lock(T& window) :
		window_(window),
		lock_(window_.mutex_),
		locked_(false)
	{
		if (0 == window_.update_lock_)
		{
			locked_ =  true;
			++window_.update_lock_;

//			window_.LockWindowUpdate(true);
		}
	}
	
	~try_update_lock()
	{
		if (locked_ && !--window_.update_lock_)
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

	mutex_t::scoped_lock lock_;
	bool locked_;
};

}
