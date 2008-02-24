
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"
#include "../res/resource.h"

#include "NewTorrentDialog.hpp"
#include "CSSFileDialog.hpp"

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

void NewTorrentDialog::OnFileBrowse(UINT, int, HWND hWnd)
{	
	CSSFileDialog dlgOpen(TRUE, NULL, NULL, OFN_HIDEREADONLY, L"Torrents (*.torrent)|*.torrent|", m_hWnd);

	if (dlgOpen.DoModal() == IDOK) 
	{
	//	ProcessFile(dlgOpen.m_ofn.lpstrFile);
	}
}

void NewTorrentDialog::OnDirBrowse(UINT, int, HWND hWnd)
{	
	CFolderDialog fldDlg (NULL, L"",
		BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);

	if (IDOK == fldDlg.DoModal())
	{
//		SetDlgItemText(IDC_BC_SAVEFOLDER, fldDlg.m_szFolderPath);
	}
}

void NewTorrentDialog::onCancel(UINT, int, HWND hWnd)
{
	EndDialog(0);
}

#define NEWTORRENT_SELECT_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp|150), WMB_COL(_auto), WMB_COL(_auto)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_SELECT_TEXT,  _r, _r), \
		WMB_ROW(_auto,	IDC_NEWT_FILE, IDC_NEWT_FILE_BROWSE, IDC_NEWT_DIR_BROWSE), \
		WMB_ROWNOMAX(_exp|50,	IDC_NEWT_LISTFILES,  _r, _r), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_TRACKERS_TEXT, _r, _r), \
		WMB_ROW(_auto,	IDC_NEWT_TRACKER, _r, IDC_NEWT_ADDTRACKER), \
		WMB_ROWNOMAX(_exp|50,	IDC_NEWT_LISTTRACKERS,  _r, _r), \
	WMB_END()

#define NEWTORRENT_TRACKERS_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp), WMB_COL(_auto)), \
	WMB_END()

#define NEWTORRENT_BUTTONS_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp), WMB_COL(_auto), WMB_COL(_auto)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_PRIVATE,  IDOK, IDCANCEL), \
	WMB_END()

#define NEWTORRENT_COMMENT_LAYOUT \
	WMB_HEAD(WMB_COL(_auto), WMB_COLNOMIN(_exp)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_COMMENT_TEXT,  IDC_NEWTORRENT_COMMENT), \
	WMB_END()

#define NEWTORRENT_CREATOR_LAYOUT \
	WMB_HEAD(WMB_COL(_auto), WMB_COLNOMIN(_exp)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_CREATOR_TEXT,  IDC_NEWTORRENT_CREATOR), \
	WMB_END()

NewTorrentDialog::CWindowMapStruct* NewTorrentDialog::GetWindowMap()
{
	BEGIN_WINDOW_MAP_INLINE(NewTorrentDialog, 3, 3, 3, 3)
		WMB_HEAD(WMB_COL(_exp)), 
		WMB_ROWNOMIN(_exp, NEWTORRENT_SELECT_LAYOUT),
		WMB_ROW(_auto, NEWTORRENT_CREATOR_LAYOUT),
		WMB_ROW(_auto, NEWTORRENT_COMMENT_LAYOUT),
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

