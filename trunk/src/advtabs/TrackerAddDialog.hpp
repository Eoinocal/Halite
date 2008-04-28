
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define HAL_TRACKER_ADD_BEGIN	 	15500
#define IDC_TRACKER_EDIT_URL        HAL_TRACKER_ADD_BEGIN + 1
#define IDC_TRACKER_EDIT_TIER       HAL_TRACKER_ADD_BEGIN + 2
#define IDC_TRACKER_TEXT_URL        HAL_TRACKER_ADD_BEGIN + 3
#define IDC_TRACKER_TEXT_TIER       HAL_TRACKER_ADD_BEGIN + 4

#ifndef RC_INVOKED

#include "GenericAddDialog.hpp"

class TrackerAddDialog :
	public CDialogImpl<TrackerAddDialog>,
	public WTLx::GenericAddDialog<TrackerAddDialog, IDD_TRACKEDIT>,
	public CDialogResize<TrackerAddDialog>,
	public CWinDataExchangeEx<TrackerAddDialog>
{
public:
	typedef TrackerAddDialog thisClass;
	typedef WTLx::GenericAddDialog<thisClass, IDD_TRACKEDIT> genericBaseClass;
	typedef CDialogResize<thisClass> resizeClass;
	
public:
	TrackerAddDialog(wstring title, hal::tracker_detail& tracker) :
		genericBaseClass(title, "genericAddDlgs/AddTracker", "AddTracker"),
		tracker_(tracker)
	{}

    BEGIN_MSG_MAP_EX(TrackerAddDialog)
		CHAIN_MSG_MAP(resizeClass)
		CHAIN_MSG_MAP(genericBaseClass)
    END_MSG_MAP()

    BEGIN_DDX_MAP(TrackerAddDialog)
		DDX_EX_STDWSTRING(IDC_TRACKER_EDIT_URL, tracker_.url);
        DDX_INT(IDC_TRACKER_EDIT_TIER, tracker_.tier)
    END_DDX_MAP()	

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(IDC_TRACKER_EDIT_URL, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_TRACKER_EDIT_TIER, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_TRACKER_TEXT_URL, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_TRACKER_TEXT_TIER, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

private:
	wstring title_;
	hal::tracker_detail& tracker_;
};

#endif
