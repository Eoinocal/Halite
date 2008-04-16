
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define HAL_NEWT_ADD_PEERS		 	14500
#define HAL_NEWT_ADD_PEERS_URL_TEXT	HAL_NEWT_ADD_PEERS + 1
#define HAL_NEWT_ADD_PEERS_URL_EDIT	HAL_NEWT_ADD_PEERS + 2
#define HAL_NEWT_ADD_PEERS_TYP_TEXT	HAL_NEWT_ADD_PEERS + 3
#define HAL_NEWT_ADD_PEERS_TYP_CBOX	HAL_NEWT_ADD_PEERS + 4

#ifndef RC_INVOKED

#include "GenericAddDialog.hpp"

class NewTorrent_PeersAddDialog :
	public CDialogImpl<NewTorrent_PeersAddDialog>,
	public WTLx::GenericAddDialog<NewTorrent_PeersAddDialog, IDD_PEEREDIT>,
	public CDialogResize<NewTorrent_PeersAddDialog>,
	public CWinDataExchangeEx<NewTorrent_PeersAddDialog>
{
public:
	typedef NewTorrent_PeersAddDialog thisClass;
	typedef WTLx::GenericAddDialog<thisClass, IDD_PEEREDIT> genericBaseClass;
	typedef CDialogResize<thisClass> resizeClass;
	
public:
	NewTorrent_PeersAddDialog(wstring title, hal::UrlDhtPeerDetail& peer) :
		genericBaseClass(title, "genericAddDlgs/NewTorrentAddPeers", "NewTorrentAddPeers"),
		peer_(peer)
	{}

    BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)

		CHAIN_MSG_MAP(resizeClass)
		CHAIN_MSG_MAP(genericBaseClass)
    END_MSG_MAP()

    BEGIN_DDX_MAP(thisClass)
		DDX_EX_STDWSTRING(HAL_NEWT_ADD_PEERS_URL_EDIT, peer_.url);
        DDX_INT(HAL_NEWT_ADD_PEERS_TYP_CBOX, peer_.type)
    END_DDX_MAP()	

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(HAL_NEWT_ADD_PEERS_URL_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(HAL_NEWT_ADD_PEERS_TYP_CBOX, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(HAL_NEWT_ADD_PEERS_URL_TEXT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(HAL_NEWT_ADD_PEERS_TYP_TEXT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	LRESULT onInitDialog(HWND, LPARAM)
	{
		CComboBox peerTypes;

		peerTypes.Attach(GetDlgItem(HAL_NEWT_ADD_PEERS_TYP_CBOX));
		
		peerTypes.AddString(L"123");
		peerTypes.AddString(L"456");
		peerTypes.AddString(L"789");

		SetMsgHandled(false);

		return 0;
	}

private:
	wstring title_;
	hal::UrlDhtPeerDetail& peer_;
};

#endif
