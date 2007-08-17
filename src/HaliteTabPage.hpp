
#pragma once

#include "stdAfx.hpp"
#include "UxthemeWrapper.hpp"

template <class T, class TBase = CWindow>
class ATL_NO_VTABLE CHalTabPageImpl : public ATL::CDialogImpl< T >
{
public:
    BEGIN_MSG_MAP_EX(CHalTabPageImpl)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_CTLCOLORDLG(OnCltColorDlg)
		MSG_WM_CTLCOLORBTN(OnCltColor)
//		MSG_WM_CTLCOLOREDIT(OnCltColor)
		MSG_WM_CTLCOLORSTATIC(OnCltColor)

        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

protected:
	LRESULT OnEraseBkgnd(HDC dc)
	{
		CRect rect;
		GetClientRect(rect);
		
		if(hal::uxtheme().pIsAppThemed && hal::uxtheme().pIsAppThemed())
		{
			if (hal::uxtheme().pDrawThemeParentBackground)
			{
				hal::uxtheme().pDrawThemeParentBackground(*this, dc, rect);
			}
		}
		
		return 1;
	}
	
	LRESULT OnCltColorDlg(HDC hDC, HWND hWnd)
	{
		SetMsgHandled(false);

		if (hal::uxtheme().pIsAppThemed)
			if(hal::uxtheme().pIsAppThemed())
			{
				RECT rect;
				GetClientRect(&rect);
				if (hal::uxtheme().pDrawThemeParentBackground)
				{
					hal::uxtheme().pDrawThemeParentBackground(hWnd, hDC, &rect);
					SetMsgHandled(true);
				}
			}

		return (LRESULT)::GetStockObject(HOLLOW_BRUSH);
	}

	LRESULT OnCltColor(HDC hDC, HWND hWnd)
	{
		SetMsgHandled(false);

		if (hal::uxtheme().pIsAppThemed)
			if(hal::uxtheme().pIsAppThemed())
			{
				RECT rect;
				::GetClientRect(hWnd, &rect);
				::SetBkMode(hDC, TRANSPARENT);
				if (hal::uxtheme().pDrawThemeParentBackground)
				{
					hal::uxtheme().pDrawThemeParentBackground(hWnd, hDC, &rect);
					SetMsgHandled(true);
				}
			}

		return (LRESULT)::GetStockObject(HOLLOW_BRUSH);
	}
};
