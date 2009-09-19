
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdAfx.hpp"
#include "DdxEx.hpp"

#include "HaliteTabPage.hpp"
#include "UxthemeWrapper.hpp"

#ifndef NDEBUG
#	include "global/logger.hpp"
#endif

class ThemeTestDialog :
	public CHalTabPageImpl<ThemeTestDialog>,
	public WTL::CDialogResize<ThemeTestDialog>
{
protected:
	typedef ThemeTestDialog thisClass;
	typedef CHalTabPageImpl<ThemeTestDialog> baseClass;
	typedef WTL::CDialogResize<ThemeTestDialog> resizeClass;
public:
	enum { IDD = HAL_THEMETEST };

	ThemeTestDialog()
	{}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(thisClass)
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
		::SetWindowText(GetDlgItem(HAL_EDIT2), L"Eoin");
		return 0;
	}

protected:
	std::wstring giveme;
};
