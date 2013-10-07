
#pragma once

#include <boost/signals2.hpp>

#include "halEvent.hpp"

namespace hal
{

template<typename F=boost::function<bool ()> >
struct signaler_wrapper : public boost::signals2::trackable
{
	signaler_wrapper(F f) :
		f_(f)
	{}

	void operator()() 
	{
		if (f_())
		{
			HAL_DEV_MSG(L"signaler_wrapper Disconnecting");
			delete this;
		}
	}

private:
	 F f_;
};

struct once
{
	template<typename S, typename F>
	once(S& s, F&& f) :
		f_(f)
	{
		c_ = s.connect(*this);
		HAL_DEV_MSG(L"Once ctor");
	}

	void operator()() 
	{
		HAL_DEV_MSG(L"Once");

		f_();
		
		HAL_DEV_MSG(L"Once disconnecting");

		c_.disconnect();
	}

	boost::function<void ()> f_;
	boost::signals2::connection c_;
};

template<typename S=boost::signals2::signal<void()>>
class signaler
{
public:

	void connect_repeat(const typename S::slot_type& slot)
	{
		HAL_DEV_MSG(L"connect_repeat");
		repeat_.connect(slot);
	}

	void connect_once(const typename S::slot_type& slot)
	{
		HAL_DEV_MSG(L"connect_once");
		once_.connect(slot);
	}

	void operator()() 
	{
		HAL_DEV_MSG(L"operator()");
		repeat_();

		once_();
		once_.disconnect_all_slots();
		HAL_DEV_MSG(L"Once disconnected");
	}	

	void disconnect_all_once()
	{
		once_.disconnect_all_slots();
		HAL_DEV_MSG(L"All disconnected");
	}

private:
	S repeat_;
	S once_;
};

}
