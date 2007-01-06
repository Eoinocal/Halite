
#pragma once
 
#include "stdAfx.hpp"
#include "DdxEx.hpp"

#include "HaliteTabPage.hpp"
#include "UxthemeWrapper.hpp"

#ifndef NDEBUG
#	include <global_log.hpp>
	using glb::wlog;
#endif

class ThemeTestDialog :
	public CHalTabPageImpl<ThemeTestDialog>,
	public CDialogResize<ThemeTestDialog>
{
protected:
	typedef ThemeTestDialog thisClass;
	typedef CHalTabPageImpl<ThemeTestDialog> baseClass;
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
		
		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;
		
		CHAIN_MSG_MAP(baseClass)
		CHAIN_MSG_MAP(resizeClass)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(thisClass)

	END_DLGRESIZE_MAP()
	
	LRESULT onInitDialog(HWND, LPARAM) 
	{
		::SetWindowText(GetDlgItem(IDC_EDIT2), L"Eoin");
		return 0; 
	}	
		
protected:
	wstring giveme;
};
