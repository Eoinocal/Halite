
#pragma once
 
#include "stdAfx.hpp"
#include "DdxEx.hpp"

#include "UxthemeWrapper.hpp"

#ifndef NDEBUG
#	include <global_log.hpp>
	using glb::wlog;
#endif

class ThemeTestDialog :
	public CDialogImpl<ThemeTestDialog>,
	public CDialogResize<ThemeTestDialog>
{
protected:
	typedef ThemeTestDialog thisClass;
	typedef CDialogImpl<ThemeTestDialog> baseClass;
	typedef CDialogResize<ThemeTestDialog> resizeClass;
public:
	enum { IDD = IDD_THEMETEST };
	
	ThemeTestDialog()
	{}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
  	
	BEGIN_MSG_MAP(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CTLCOLORDLG(OnCltColorDlg)
		MSG_WM_CTLCOLORBTN(OnCltColor)
		MSG_WM_CTLCOLORSTATIC(OnCltColor)
		
		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;
		
		CHAIN_MSG_MAP(resizeClass)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(thisClass)

	END_DLGRESIZE_MAP()
	
	LRESULT onInitDialog(HWND, LPARAM) 
	{
		::SetWindowText(GetDlgItem(IDC_EDIT2), L"Eoin");
		return 0; 
	}	
	
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
		
protected:
	wstring giveme;
};
