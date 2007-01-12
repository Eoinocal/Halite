
#pragma once
 
#include "stdAfx.hpp"
#include "UxthemeWrapper.hpp"

template <class T, class TBase = CWindow>
class ATL_NO_VTABLE CHalTabPageImpl : public ATL::CDialogImpl< T >
{
public:
    BEGIN_MSG_MAP(CHalTabPageImpl)
		MSG_WM_CTLCOLORDLG(OnCltColorDlg)
		MSG_WM_CTLCOLORBTN(OnCltColor)
//		MSG_WM_CTLCOLOREDIT(OnCltColor)
		MSG_WM_CTLCOLORSTATIC(OnCltColor)
		
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()
	
protected:
	LRESULT OnCltColorDlg(HDC hDC, HWND hWnd)
	{
		SetMsgHandled(false);
		
		if (halite::uxtheme().pIsAppThemed)
			if(halite::uxtheme().pIsAppThemed())
			{
				RECT rect;
				GetClientRect(&rect);
				if (halite::uxtheme().pDrawThemeParentBackground)
				{
					halite::uxtheme().pDrawThemeParentBackground(hWnd, hDC, &rect);
					SetMsgHandled(true);
				}
			}
			
		return (LRESULT)::GetStockObject(HOLLOW_BRUSH);
	}
	
	LRESULT OnCltColor(HDC hDC, HWND hWnd)
	{	
		SetMsgHandled(false);
		
		if (halite::uxtheme().pIsAppThemed)
			if(halite::uxtheme().pIsAppThemed())
			{
				RECT rect;
				::GetClientRect(hWnd, &rect);
				::SetBkMode(hDC, TRANSPARENT); 
				if (halite::uxtheme().pDrawThemeParentBackground)
				{
					halite::uxtheme().pDrawThemeParentBackground(hWnd, hDC, &rect);
					SetMsgHandled(true);
				}
			}
			
		return (LRESULT)::GetStockObject(HOLLOW_BRUSH);
	}	
};
