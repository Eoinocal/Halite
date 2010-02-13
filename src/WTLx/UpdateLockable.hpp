
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "UpdateLocks.hpp"

namespace hal
{

template<typename Base>
class update_lockable
{
public:
	update_lockable() :
		update_lock_(0),
		notify_lock_(0)
	{}
	
	bool can_update() const { return update_lock_ == 0; }

protected:
	mutable int update_lock_;
	mutable int notify_lock_;
	mutable mutex_t update_mutex_;

	friend class hal::mutex_update_lock<Base>;	
	friend class hal::try_update_lock<Base>;	

//	friend class mutex_update_lock<Base>;	
//	friend class try_update_lock<Base>;	
};

}
