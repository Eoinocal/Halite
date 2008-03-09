
//         Copyright Eoin O'Callaghan 2006 - 2008.
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
	typedef EditHilightImpl< T, TBase, TWinTraits> thisClass;
	typedef ATL::CWindowImpl<T, TBase, TWinTraits> baseClass;

public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

	explicit EditHilightImpl(bool signal = false) :
		unapplied_(false),
		signal_(signal)
	{}

    BEGIN_MSG_MAP_EX(thisClass)
		MSG_OCM_CTLCOLOREDIT(OnReflectedCtlColorEdit)

		REFLECTED_COMMAND_CODE_HANDLER_EX(EN_KILLFOCUS, OnKillFocus)
		REFLECTED_COMMAND_CODE_HANDLER_EX(EN_CHANGE, OnChange)
		
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

	void SubclassWindow(HWND hWndNew)
	{
		ATLASSERT(::IsWindow(hWndNew));
        baseClass::SubclassWindow(hWndNew);
	}
	
	LRESULT OnChange(UINT uNotifyCode, int nID, CWindow wndCtl)
	{	
		HAL_DEV_MSG(wformat(L"OnChange %1%, %2%") % unapplied_ % signal_);

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
		HAL_DEV_MSG(wformat(L"OnKillFocus %1%, %2%") % unapplied_ % signal_);

		const int buffer_size = 512;
		boost::array<wchar_t, buffer_size> buffer;
		GetWindowText(buffer.elems, buffer_size);
				
		if (signal_)
		{
			::SendMessage(GetParent(), WM_USER_WTLx_EDITHILIGHTCHANGED, 0, 0);
			unapplied_ = false;
		}
		
		return 0;
	}

	HBRUSH OnReflectedCtlColorEdit(WTL::CDCHandle dc, WTL::CEdit edit)
	{
		HAL_DEV_MSG(wformat(L"OnReflectedCtlColorEdit %1%, %2%") % unapplied_ % signal_);

		if (unapplied_)
		{
			SetTextColor(dc, RGB(0,0,255));
			return GetSysColorBrush(COLOR_3DHILIGHT);
		}
		else
		{
			return false;
		}
	}	
	
	operator std::wstring () 
	{ 
		size_t maxLen = GetWindowTextLength();

		std::vector<wchar_t> buffer(maxLen);
		size_t len = GetWindowText(&buffer[0], maxLen);

		unapplied_ = false;		
		return std::wstring(buffer.begin(), buffer.begin()+len);
	}

	std::wstring& operator=(std::wstring& val) 
	{	
		SetWindowText(val.c_str());

		unapplied_ = boost::logic::indeterminate;;
		return val;
	}
	
private:	
	boost::logic::tribool unapplied_;
	bool signal_;
};

class EditHilight : public EditHilightImpl<EditHilight>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTLx_EditHilight"), GetWndClassName())

	EditHilight(bool signal = false) : 
		EditHilightImpl<EditHilight>(signal)
	{}

	operator std::wstring () 
	{ 
		//return *this;
		return *static_cast<EditHilightImpl<EditHilight>* >(this);
	}

	std::wstring& operator=(std::wstring& val) 
	{
		return EditHilightImpl<EditHilight>::operator =(val);
	}
};

};
