
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"
#include "../res/resource.h"
#include "NewTorrentDialog.hpp"

LRESULT NewTorrentDialog::onInitDialog(HWND, LPARAM)
{
	hal::event().post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(L"NewTorrentDialog::onInitDialog(HWND, LPARAM)")));	
	
	MoveWindow(rect_.left, rect_.top, rect_.right-rect_.left, rect_.bottom-rect_.top, true);	

/*	SetWindowText(windowText_.c_str());
	prog_.Attach(GetDlgItem(IDC_PROG_PROGRESS));
	prog_.SetRange(0, 100);
	
	thread_ptr.reset(new thread(bind(&ProgressDialog::ProgressThread, this)));
*/		
	return 0;
}

void NewTorrentDialog::onCancel(UINT, int, HWND hWnd)
{
	EndDialog(0);
}

#define NEWTORRENT_SELECT_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp|150), WMB_COL(14)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_SELECT_TEXT,  _r), \
		WMB_ROW(_auto,	IDC_NEWT_FILE, IDC_NEWT_BROWSE), \
	WMB_END()

#define NEWTORRENT_TRACKERS_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp), WMB_COL(_auto)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_TRACKERS_TEXT,  _r), \
		WMB_ROW(_auto,	IDC_NEWT_TRACKER, IDC_NEWT_ADDTRACKER), \
		WMB_ROW(_exp,	IDC_NEWT_LISTTRACKERS,  _r), \
	WMB_END()

#define NEWTORRENT_BUTTONS_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp), WMB_COL(_auto), WMB_COL(_auto)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_PRIVATE,  IDOK, IDCANCEL), \
	WMB_END()

NewTorrentDialog::CWindowMapStruct* NewTorrentDialog::GetWindowMap()
{
	BEGIN_WINDOW_MAP_INLINE(NewTorrentDialog, 3, 3, 3, 3)
		WMB_HEAD(WMB_COL(_exp)), 
		WMB_ROW(_auto, NEWTORRENT_SELECT_LAYOUT),
		WMB_ROW(_exp, NEWTORRENT_TRACKERS_LAYOUT),
		WMB_ROW(_auto, IDC_NEWTORRENT_CREATOR_TEXT),
		WMB_ROW(_auto, IDC_NEWTORRENT_CREATOR),
		WMB_ROW(_auto, IDC_NEWTORRENT_COMMENT_TEXT),
		WMB_ROW(_auto, IDC_NEWTORRENT_COMMENT),
		WMB_ROW(_auto, NEWTORRENT_BUTTONS_LAYOUT),
		WMB_END() 
	END_WINDOW_MAP_INLINE()	
}

void NewTorrentDialog::OnClose()
{
	EndDialog(0);
}
 
void NewTorrentDialog::OnDestroy()
{
	GetWindowRect(rect_);
	Save();
}

void NewTorrentDialog::OnSize(UINT type, CSize)
{
	GetWindowRect(rect_);
	
	SetMsgHandled(false);
}	

