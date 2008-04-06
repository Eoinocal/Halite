
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "GenericAddDialog.hpp"

class NewTorrent_PeersAddDialog :
	public CDialogImpl<NewTorrent_PeersAddDialog>,
	public WTLx::GenericAddDialog<NewTorrent_PeersAddDialog, IDD_TRACKEDIT>,
	public CDialogResize<NewTorrent_PeersAddDialog>,
	public CWinDataExchangeEx<NewTorrent_PeersAddDialog>
{
public:
	typedef NewTorrent_PeersAddDialog thisClass;
	typedef WTLx::GenericAddDialog<thisClass, IDD_TRACKEDIT> genericBaseClass;
	typedef CDialogResize<thisClass> resizeClass;
	
public:
	NewTorrent_PeersAddDialog(wstring title, hal::TrackerDetail& tracker) :
		genericBaseClass(title, "genericAddDlgs/NewTorrentAddPeers", "NewTorrentAddPeers"),
		tracker_(tracker)
	{}

    BEGIN_MSG_MAP_EX(thisClass)
		CHAIN_MSG_MAP(resizeClass)
		CHAIN_MSG_MAP(genericBaseClass)
    END_MSG_MAP()

    BEGIN_DDX_MAP(thisClass)
		DDX_EX_STDWSTRING(IDC_TRACKER_EDIT_URL, tracker_.url);
        DDX_INT(IDC_TRACKER_EDIT_TIER, tracker_.tier)
    END_DDX_MAP()	

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(IDC_TRACKER_EDIT_URL, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_TRACKER_EDIT_TIER, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

private:
	wstring title_;
	hal::TrackerDetail& tracker_;
};
