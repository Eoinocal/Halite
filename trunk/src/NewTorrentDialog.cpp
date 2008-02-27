
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"
#include "../res/resource.h"

#include "NewTorrentDialog.hpp"
#include "CSSFileDialog.hpp"

void FileSheet::OnFileBrowse(UINT, int, HWND hWnd)
{	
	CSSFileDialog dlgOpen(TRUE, NULL, NULL, OFN_HIDEREADONLY, L"Torrents (*.torrent)|*.torrent|", m_hWnd);

	if (dlgOpen.DoModal() == IDOK) 
	{
	//	ProcessFile(dlgOpen.m_ofn.lpstrFile);
	}
}

void FileSheet::OnDirBrowse(UINT, int, HWND hWnd)
{	
	CFolderDialog fldDlg (NULL, L"",
		BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);

	if (IDOK == fldDlg.DoModal())
	{
//		SetDlgItemText(IDC_BC_SAVEFOLDER, fldDlg.m_szFolderPath);
	}
}

#define NEWTORRENT_SELECT_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp|150), WMB_COL(_auto), WMB_COL(_auto)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_SELECT_TEXT,  _r, _r), \
		WMB_ROW(_auto,	IDC_NEWT_FILE, IDC_NEWT_FILE_BROWSE, IDC_NEWT_DIR_BROWSE), \
		WMB_ROWNOMAX(_exp|50,	IDC_NEWT_LISTFILES,  _r, _r), \
	WMB_END()

#define NEWTORRENT_TRACKERS_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp), WMB_COL(_auto)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_TRACKERS_TEXT, _r), \
		WMB_ROW(_auto,	IDC_NEWT_TRACKER, IDC_NEWT_ADDTRACKER), \
		WMB_ROWNOMAX(_exp|50,	IDC_NEWT_LISTTRACKERS,  _r), \
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

FileSheet::CWindowMapStruct* FileSheet::GetWindowMap()
{
	BEGIN_WINDOW_MAP_INLINE(NewTorrentDialog, 3, 3, 3, 3)
		WMB_HEAD(WMB_COL(_exp)), 
		WMB_ROWNOMIN(_exp, NEWTORRENT_SELECT_LAYOUT),
//		WMB_ROW(_auto, NEWTORRENT_CREATOR_LAYOUT),
//		WMB_ROW(_auto, NEWTORRENT_COMMENT_LAYOUT),
//		WMB_ROW(_auto, NEWTORRENT_BUTTONS_LAYOUT),
		WMB_END() 
	END_WINDOW_MAP_INLINE()	
}	

#define NEWTORRENT_TRACKERS_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp), WMB_COL(_auto)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_TRACKERS_TEXT, _r), \
		WMB_ROW(_auto,	IDC_NEWT_TRACKER, IDC_NEWT_ADDTRACKER), \
		WMB_ROWNOMAX(_exp|50,	IDC_NEWT_LISTTRACKERS,  _r), \
	WMB_END()

TrackerSheet::CWindowMapStruct* TrackerSheet::GetWindowMap()
{
	BEGIN_WINDOW_MAP_INLINE(NewTorrentDialog, 3, 3, 3, 3)
		WMB_HEAD(WMB_COL(_exp)), 
		WMB_ROWNOMIN(_exp, NEWTORRENT_TRACKERS_LAYOUT),
		WMB_END() 
	END_WINDOW_MAP_INLINE()	
}	

#define NEWTORRENT_DETAILS_LAYOUT \
	WMB_HEAD(WMB_COLNOMIN(_exp|33), WMB_COLNOMIN(_exp)), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_COMMENT_TEXT,  IDC_NEWTORRENT_COMMENT), \
		WMB_ROW(_auto,	IDC_NEWTORRENT_CREATOR_TEXT,  IDC_NEWTORRENT_CREATOR), \
	WMB_END()

DetailsSheet::CWindowMapStruct* DetailsSheet::GetWindowMap()
{
	BEGIN_WINDOW_MAP_INLINE(NewTorrentDialog, 3, 3, 3, 3)
		WMB_HEAD(WMB_COL(_exp)), 
		WMB_ROWNOMIN(_exp, NEWTORRENT_DETAILS_LAYOUT),
		WMB_ROW(_gap),
		WMB_ROW(_auto, IDC_NEWTORRENT_PRIVATE),
		WMB_END() 
	END_WINDOW_MAP_INLINE()	
}	
