
//         Copyright Eóin O'Callaghan 2006 - 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once 

#define WM_USER_WTLx_EDITHILIGHTCHANGED	WM_USER + 100

#include "stdAfx.hpp"
#include <boost/logic/tribool.hpp>

namespace WTLx
{

template <class T, class TBase = WTL::CEdit, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE EditHilightImpl : public ATL::CWindowImpl<T, TBase, TWinTraits>
{
protected:
	typedef EditHilightImpl< T, TBase, TWinTraits> this_class_t;
	typedef ATL::CWindowImpl<T, TBase, TWinTraits> base_class_t;

public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

	explicit EditHilightImpl() :
		unapplied_(false)
	{}

	BEGIN_MSG_MAP_EX(this_class_t)
		MSG_OCM_CTLCOLOREDIT(OnReflectedCtlColorEdit)
		REFLECTED_COMMAND_CODE_HANDLER_EX(EN_CHANGE, OnChange)
		
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void SubclassWindow(HWND hWndNew)
	{
		ATLASSERT(::IsWindow(hWndNew));
        base_class_t::SubclassWindow(hWndNew);
	}
	
	LRESULT OnChange(UINT uNotifyCode, int nID, ATL::CWindow wndCtl)
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

	HBRUSH OnReflectedCtlColorEdit(WTL::CDCHandle dc, WTL::CEdit edit)
	{
		if (unapplied_)
		{
			SetTextColor(dc, GetSysColor(COLOR_HOTLIGHT));
			return GetSysColorBrush(COLOR_WINDOW);
		}
		else
		{
			return false;
		}
	}	
	
	operator std::wstring () 
	{ 
		int max_len = GetWindowTextLength()+1;

		std::vector<wchar_t> buffer(max_len);
		size_t len = GetWindowText(&buffer[0], max_len);

		unapplied_ = false;		
		InvalidateRect(0, true);

		return std::wstring(buffer.begin(), buffer.begin()+len);
	}

	std::wstring& operator=(std::wstring& val) 
	{	
		SetWindowText(val.c_str());

		unapplied_ = false;
		InvalidateRect(0, true);

		return val;
	}
	
private:	
	boost::logic::tribool unapplied_;
};

class EditHilight : public EditHilightImpl<EditHilight>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTLx_EditHilight"), GetWndClassName())

	EditHilight() : 
		EditHilightImpl<EditHilight>()
	{}

	operator std::wstring () 
	{ 
		return *static_cast<EditHilightImpl<EditHilight>* >(this);
	}

	std::wstring& operator=(std::wstring& val) 
	{
		return EditHilightImpl<EditHilight>::operator =(val);
	}
};

};
