
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdAfx.hpp"
//#include "UxthemeWrapper.hpp"

template <class T>
class ATL_NO_VTABLE CHalTabPageImpl : 
	public ATL::CDialogImpl< T >,
	public WTL::CMessageFilter,
	public WTL::CThemeImpl<CHalTabPageImpl<T> >
{
public:
    BEGIN_MSG_MAP_EX(CHalTabPageImpl)
		MSG_WM_CTLCOLORDLG(OnCltColorDlg)
		MSG_WM_CTLCOLORBTN(OnCltColor)
		MSG_WM_CTLCOLORSTATIC(OnCltColor)

		CHAIN_MSG_MAP(WTL::CThemeImpl<CHalTabPageImpl<T> >)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

protected:
	LRESULT OnEraseBkgnd(HDC dc)
	{
		return 1;
		
		CRect rect;
		GetClientRect(rect);
		
		if(IsThemingSupported() && IsAppThemed())
		{
			if(GetThemeClassList() == NULL)
			{
				SetThemeClassList(L"Tab");
				OpenThemeData();
			}
		//	if (uxtheme().pDrawThemeParentBackground)
		//	{
				DrawThemeParentBackground(dc, rect);
		//	}
		}
		
		return 1;
	}
	

	LRESULT OnCltColorDlg(HDC hDC, HWND hWnd)
	{
		SetMsgHandled(false);

		if(IsThemingSupported() && IsAppThemed())
		{
			if(GetThemeClassList() == NULL)
			{
				SetThemeClassList(L"Tab");
				OpenThemeData();
			}

			WTL::CRect rect;
			GetClientRect(rect);

			DrawThemeParentBackground(hDC, rect);
			SetMsgHandled(true);
		}

		return (LRESULT)::GetStockObject(HOLLOW_BRUSH);
	}

	LRESULT OnCltColor(HDC hDC, HWND hWnd)
	{
		SetMsgHandled(false);

		if(IsThemingSupported() && IsAppThemed())
		{
			if(GetThemeClassList() == NULL)
			{
				SetThemeClassList(L"Tab");
				OpenThemeData();
			}

			WTL::CRect rect;
			::GetClientRect(hWnd, &rect);
			::SetBkMode(hDC, TRANSPARENT);

			::DrawThemeParentBackground(hWnd, hDC, rect);
			SetMsgHandled(true);
		}

		return (LRESULT)::GetStockObject(HOLLOW_BRUSH);
	}
};
