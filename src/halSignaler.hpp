
#pragma once

#include <boost/signal.hpp>

#include "halEvent.hpp"

namespace hal
{

struct once
{
	template<typename S>
	once(S& s, boost::function<void ()> f) :
		f_(f)
	{
		c_ = s.connect(*this);
		HAL_DEV_MSG(L"Once ctor");
	}

	void operator()() 
	{
		f_();
		
		HAL_DEV_MSG(L"Once disconnecting");

		c_.disconnect();
	}

	boost::function<void ()> f_;
	boost::signals::connection c_;
};

template<typename S=boost::signal<void()> >
class signaler
{
public:

	void connect_repeat(const typename S::slot_type& slot)
	{
		repeat_.connect(slot);
	}

	void connect_once(const typename S::slot_type& slot)
	{
		once_.connect(slot);
	}

	void operator()() 
	{
		repeat_();

		once_();
		once_.disconnect_all_slots();
	}	

private:
	S repeat_;
	S once_;
};

}
