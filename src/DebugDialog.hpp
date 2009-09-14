
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdAfx.hpp"

class LogEdit : public CWindowImpl<LogEdit, CEdit>
{
public:
    BEGIN_MSG_MAP_EX(CEditImpl)
    END_MSG_MAP()

	LogEdit() :
		editLogger(hal::wlog().attach(bind(&LogEdit::log, this, _1)))
	{}

	void log(const std::wstring& text)
	{
		int len = ::SendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0);
		SetSel(len, len);
		ReplaceSel(text.c_str(), false);
	}

private:
	boost::signals::scoped_connection editLogger;
};

class DebugDialog :
	public CDialogImpl<DebugDialog>,
	public CDialogResize<DebugDialog>
{
private:
	typedef DebugDialog thisClass;
	typedef CDialogImpl<thisClass> baseClass;
	typedef CDialogResize<thisClass> resizeClass;

	LogEdit logEdit;

public:
	enum { IDD = HAL_DEBUGDIALOG };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(thisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)

		if(uMsg == WM_FORWARDMSG)
			if(PreTranslateMessage((LPMSG)lParam)) return TRUE;

		CHAIN_MSG_MAP(resizeClass)

	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(ED_CON, DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
		logEdit.SubclassWindow(GetDlgItem(ED_CON));

		return true;
	}

	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&)
	{
		ShowWindow(false);
		return true;
	}
};
