
//         Copyright Eoin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once 

#define WM_USER_HAL_EDITCHANGED	WM_USER + 100

#include "stdAfx.hpp"
#include <boost/logic/tribool.hpp>
#include "global/string_conv.hpp"

template<typename T>
class CHaliteEditCtrl :
	public CWindowImpl<CHaliteEditCtrl<T>, WTL::CEdit>,
	private boost::noncopyable
{
protected:
	typedef CHaliteEditCtrl<T> thisClass;
	typedef CWindowImpl<thisClass, WTL::CEdit> baseClass;
	
public:
	explicit CHaliteEditCtrl(T lwr = 0, bool include = false, bool signal = true) :
		range_lwr_(lwr),
		range_inc_(include),
		unapplied_(false),
		signal_(signal)
	{}

    BEGIN_MSG_MAP_EX(thisClass)
		REFLECTED_COMMAND_CODE_HANDLER_EX(EN_KILLFOCUS, OnKillFocus)
		REFLECTED_COMMAND_CODE_HANDLER_EX(EN_CHANGE, OnChange)
		MSG_OCM_CTLCOLOREDIT(OnReflectedCtlColorEdit)
		
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

	void Attach(HWND hWndNew)
	{
		ATLASSERT(::IsWindow(hWndNew));
        baseClass::SubclassWindow(hWndNew);
	}
	
	LRESULT OnChange(UINT uNotifyCode, int nID, CWindow wndCtl)
	{	
		if (unapplied_)
		{
		}
		else if (!unapplied_)
		{
			unapplied_ = true;
			InvalidateRect(0, true);
		}
		else
		{
			unapplied_ = false;
		}
		
		return 0;
	}
	
	LRESULT OnKillFocus(UINT uNotifyCode, int nID, CWindow wndCtl)
	{	
		const int buffer_size = 512;
		boost::array<wchar_t, buffer_size> buffer;
		GetWindowText(buffer.elems, buffer_size);
		
		try
		{
		value_ = lexical_cast<T>(buffer.elems);
			
		if (range_inc_)
		{
			if (value_ < range_lwr_) value_ = -1;
		}
		else
		{
			if (value_ <= range_lwr_) value_ = -1;
		}
			
		}        
		catch(boost::bad_lexical_cast &)
		{
		value_ = -1;
		}
		
		if (value_ < 0)	SetWindowText(hal::app().res_wstr(HAL_INF).c_str());
		
		if (signal_)
		{
			::SendMessage(GetParent(), WM_USER_HAL_EDITCHANGED, 0, 0);
			unapplied_ = false;
		}
		
		return 0;
	}
	
	HBRUSH OnReflectedCtlColorEdit(WTL::CDCHandle dc, WTL::CEdit edit)
	{
		if (signal_ && unapplied_)
		{
			SetTextColor(dc, RGB(0,0,255));
			return GetSysColorBrush(COLOR_3DHILIGHT);
		}
		else
		{
			return false;
		}
	}
	
	T& Value() { return value_; }
	
	operator T& () { return Value(); }
	
	T& operator=(T val) 
	{
		if (!unapplied_) unapplied_ = boost::logic::indeterminate;
		
		if (range_inc_)
		{
			if (val < range_lwr_) val = -1;
		}
		else
		{
			if (val <= range_lwr_) val = -1;
		}
		
		if (val < 0)	
		{
			value_ = -1;
			SetWindowText(hal::app().res_wstr(HAL_INF).c_str());
		}
		else
		{
			value_ = val;
			SetWindowText((lexical_cast<wstring>(value_)).c_str());
		}
		
		return value_;
	}		
	
private:
	T value_;
	
	T range_lwr_;
	bool range_inc_;
	
	boost::logic::tribool unapplied_;
	bool signal_;
};

