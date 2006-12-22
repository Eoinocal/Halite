
#pragma once
 
#include "stdAfx.hpp"
#include "DdxEx.hpp"

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
//		MSG_WM_CLOSE(onClose)	
//		MSG_WM_SIZE(OnSize)
		MSG_WM_CTLCOLORDLG(OnCltColorDlg)
		MSG_WM_CTLCOLORBTN(OnCltColor)
		MSG_WM_CTLCOLORSTATIC(OnCltColor)
		MSG_WM_CTLCOLOREDIT(OnCltColor)
		
		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;
		
		CHAIN_MSG_MAP(resizeClass)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(thisClass)

	END_DLGRESIZE_MAP()
	
	LRESULT OnCltColorDlg(HDC pDC, HWND pWnd)
	{
		RECT rect;
		GetClientRect(&rect);
		DrawThemeParentBackground(pWnd, pDC, &rect);
		
		return  (LRESULT)::GetStockObject(HOLLOW_BRUSH);
	}
	
	LRESULT OnCltColor(HDC pDC, HWND pWnd)
	{
		RECT rect;
		::GetClientRect(pWnd, &rect);
		DrawThemeParentBackground(pWnd, pDC, &rect);
		
		return  (LRESULT)::GetStockObject(HOLLOW_BRUSH);
	}	
	
	LRESULT onInitDialog(HWND, LPARAM) { return 0; }	
	
//	void OnSize(UINT, CSize);
//	void onClose();	
	
protected:
};
