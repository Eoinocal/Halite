
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace hal
{

template<class T>
class update_lockable;

template<class T>
class mutex_update_lock
{
public:
	mutex_update_lock(update_lockable<T>* window) :
		window_(window),
		lock_(window_->update_mutex_)
	{
		++window_->update_lock_;

//		window_.LockWindowUpdate(true);
	}
	
	~mutex_update_lock()
	{
		if (!--window_->update_lock_)
			unlock();
	}
	
	void unlock()
	{
//		window_.LockWindowUpdate(false);
//		window_.InvalidateRect(NULL, true);
	}
	
private:
	update_lockable<T>* window_;

	mutex_t::scoped_lock lock_;
};

template<class T>
class try_update_lock
{
public:
	try_update_lock(update_lockable<T>* window) :
		window_(window),
		lock_(window_->update_mutex_),
		locked_(false)
	{
		if (0 == window_->update_lock_)
		{
			++window_->update_lock_;

			T* pT = static_cast<T*>(window);
			pT->SetRedraw(false);
		}
	}
	
	~try_update_lock()
	{
		if (!--window_->update_lock_)
			unlock();
	}
	
	void unlock()
	{
		T* pT = static_cast<T*>(window_);
		pT->SetRedraw(true);
//		pT->RedrawWindow(NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
	
	operator bool() const { return 0 != window_->update_lock_; }
	
private:
	update_lockable<T>* window_;

	mutex_t::scoped_lock lock_;
	bool locked_;
};

}
