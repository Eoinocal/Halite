
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "DdxEx.hpp"

class TrackerAddDialog :
	public CDialogImpl<TrackerAddDialog>,
    public CWinDataExchangeEx<TrackerAddDialog>
{
protected:
	typedef TrackerAddDialog thisClass;
	typedef CDialogImpl<TrackerAddDialog> baseClass;
	
public:
	TrackerAddDialog(wstring title, hal::TrackerDetail& tracker) :
		title_(title),
		tracker_(tracker)
	{}
	
	enum { IDD = IDD_TRACKEDIT };

    BEGIN_MSG_MAP_EX(TrackerAddDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_CLOSE(OnClose)	
		COMMAND_ID_HANDLER_EX(IDOK, OnOk)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
    END_MSG_MAP()

    BEGIN_DDX_MAP(TrackerAddDialog)
		DDX_EX_STDWSTRING(IDC_TRACKER_EDIT_URL, tracker_.url);
        DDX_INT(IDC_TRACKER_EDIT_TIER, tracker_.tier)
    END_DDX_MAP()
	
	LRESULT OnInitDialog(...)
	{
		SetWindowText(title_.c_str());
		CenterWindow();
		BOOL retval =  DoDataExchange(false);
		
		return TRUE;
	}
	
	void OnClose()
	{
		DoDataExchange(true);
		EndDialog(0);
	}

	void OnCancel(...)
	{
		DoDataExchange(true);
		EndDialog(0);
	}

	void OnOk(...)
	{
		DoDataExchange(true);
		EndDialog(1);
	}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
	
	void onCancel(UINT, int, HWND hWnd)
	{
	
	}

private:
	wstring title_;
	hal::TrackerDetail& tracker_;
};
