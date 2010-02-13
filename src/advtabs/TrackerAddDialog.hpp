
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define HAL_TRACKER_ADD_BEGIN	 	15500
#define HAL_TRACKER_EDIT_URL        HAL_TRACKER_ADD_BEGIN + 1
#define HAL_TRACKER_EDIT_TIER       HAL_TRACKER_ADD_BEGIN + 2
#define HAL_TRACKER_TEXT_URL        HAL_TRACKER_ADD_BEGIN + 3
#define HAL_TRACKER_TEXT_TIER       HAL_TRACKER_ADD_BEGIN + 4

#ifndef RC_INVOKED

#include "GenericAddDialog.hpp"

class TrackerAddDialog :
	public ATL::CDialogImpl<TrackerAddDialog>,
	public WTLx::GenericAddDialog<TrackerAddDialog, HAL_TRACKEDIT>,
	public WTL::CDialogResize<TrackerAddDialog>,
	public WTLx::WinDataExchangeEx<TrackerAddDialog>
{
public:
	typedef TrackerAddDialog this_class_t;
	typedef WTLx::GenericAddDialog<this_class_t, HAL_TRACKEDIT> generic_dlg_class_t;
	typedef WTL::CDialogResize<this_class_t> resize_class_t;
	
public:
	TrackerAddDialog(wstring title, hal::tracker_detail& tracker) :
		generic_dlg_class_t(title, "genericAddDlgs/AddTracker", "AddTracker"),
		tracker_(tracker)
	{}

	BEGIN_MSG_MAP_EX(TrackerAddDialog)
		CHAIN_MSG_MAP(resize_class_t)
		CHAIN_MSG_MAP(generic_dlg_class_t)
	END_MSG_MAP()

	BEGIN_DDX_MAP(TrackerAddDialog)
		DDX_EX_STDWSTRING(HAL_TRACKER_EDIT_URL, tracker_.url);
		DDX_INT(HAL_TRACKER_EDIT_TIER, tracker_.tier)
	END_DDX_MAP()	

	BEGIN_DLGRESIZE_MAP(this_class_t)
		DLGRESIZE_CONTROL(HAL_TRACKER_EDIT_URL, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(HAL_TRACKER_EDIT_TIER, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(HAL_TRACKER_TEXT_URL, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(HAL_TRACKER_TEXT_TIER, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

private:
	wstring title_;
	hal::tracker_detail& tracker_;
};

#endif
