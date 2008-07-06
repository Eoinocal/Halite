
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
#define HAL_NEWT_ADD_DHT_PORT		HAL_NEWT_ADD_PEERS + 7
#define HAL_NEWT_ADD_DHT_PORT_TEXT	HAL_NEWT_ADD_PEERS + 8

#define HAL_NEWT_ADD_PEERS_WEB		HAL_NEWT_ADD_PEERS + 5
#define HAL_NEWT_ADD_PEERS_DHT		HAL_NEWT_ADD_PEERS + 6
#define HAL_NEWT_EDIT_PEER			HAL_NEWT_ADD_PEERS + 9
#define HAL_NEWT_ADD_NEW_PEER		HAL_NEWT_ADD_PEERS + 10

#ifndef RC_INVOKED

#include "GenericAddDialog.hpp"

class NewTorrent_PeersAddDialog :
	public ATL::CDialogImpl<NewTorrent_PeersAddDialog>,
	public WTLx::GenericAddDialog<NewTorrent_PeersAddDialog, HAL_PEEREDIT>,
	public WTL::CDialogResize<NewTorrent_PeersAddDialog>,
	public CWinDataExchangeEx<NewTorrent_PeersAddDialog>
{
public:
	typedef NewTorrent_PeersAddDialog thisClass;
	typedef WTLx::GenericAddDialog<thisClass, HAL_PEEREDIT> genericBaseClass;
	typedef WTL::CDialogResize<thisClass> resizeClass;
	
public:
	NewTorrent_PeersAddDialog(wstring title, hal::web_seed_or_dht_node_detail& peer) :
		genericBaseClass(title, "genericAddDlgs/NewTorrentAddPeers", "NewTorrentAddPeers"),
		peer_(peer)
	{}

    BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		COMMAND_HANDLER_EX(HAL_NEWT_ADD_PEERS_TYP_CBOX, CBN_SELCHANGE, OnTypeChanged)

		CHAIN_MSG_MAP(resizeClass)
		CHAIN_MSG_MAP(genericBaseClass)
    END_MSG_MAP()

    BEGIN_DDX_MAP(thisClass)
		DDX_EX_STDWSTRING(HAL_NEWT_ADD_PEERS_URL_EDIT, peer_.url);
        DDX_INT(HAL_NEWT_ADD_DHT_PORT, peer_.port)
        DDX_EX_STDWSTRING(HAL_NEWT_ADD_PEERS_TYP_CBOX, peer_.type)
    END_DDX_MAP()	

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(HAL_NEWT_ADD_PEERS_URL_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(HAL_NEWT_ADD_PEERS_TYP_CBOX, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(HAL_NEWT_ADD_PEERS_URL_TEXT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(HAL_NEWT_ADD_PEERS_TYP_TEXT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(HAL_NEWT_ADD_DHT_PORT_TEXT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(HAL_NEWT_ADD_DHT_PORT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	LRESULT onInitDialog(HWND, LPARAM)
	{
		WTL::CComboBox peerTypes;

		peerTypes.Attach(GetDlgItem(HAL_NEWT_ADD_PEERS_TYP_CBOX));
		
		peerTypes.AddString(hal::app().res_wstr(HAL_NEWT_ADD_PEERS_WEB).c_str());
		peerTypes.AddString(hal::app().res_wstr(HAL_NEWT_ADD_PEERS_DHT).c_str());

		if (hal::app().res_wstr(HAL_NEWT_ADD_PEERS_WEB) == peer_.type)
			peerTypes.SetCurSel(1);
		else
		{
			peerTypes.SetCurSel(0);
			::EnableWindow(GetDlgItem(HAL_NEWT_ADD_DHT_PORT_TEXT), true);
			::EnableWindow(GetDlgItem(HAL_NEWT_ADD_DHT_PORT), true);
		}

		resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);

		SetMsgHandled(false);
		return 0;
	}
	
	void OnTypeChanged(UINT uNotifyCode, int nID, CWindow wndCtl)
	{	
		hal::win_c_str<std::wstring> str_buf(MAX_PATH);		
		wndCtl.GetWindowText(str_buf, str_buf.size());
		
		if (str_buf.str() == hal::app().res_wstr(HAL_NEWT_ADD_PEERS_WEB))
		{
			::EnableWindow(GetDlgItem(HAL_NEWT_ADD_DHT_PORT_TEXT), false);
			::EnableWindow(GetDlgItem(HAL_NEWT_ADD_DHT_PORT), false);
		}
		else
		{
			::EnableWindow(GetDlgItem(HAL_NEWT_ADD_DHT_PORT_TEXT), true);
			::EnableWindow(GetDlgItem(HAL_NEWT_ADD_DHT_PORT), true);
		}		
	}		

private:
	wstring title_;
	hal::web_seed_or_dht_node_detail& peer_;
};

#endif
