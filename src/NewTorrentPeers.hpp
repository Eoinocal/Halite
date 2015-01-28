
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define ID_NTPLVM_BEGIN			19000
#define ID_NTPLVM_NEW 			ID_NTPLVM_BEGIN + 1
#define ID_NTPLVM_EDIT 			ID_NTPLVM_BEGIN + 2
#define ID_NTPLVM_DELETE			ID_NTPLVM_BEGIN + 3
#define HAL_NTPLVM_NAMES		ID_NTPLVM_BEGIN + 4

#define HAL_NEWT_ADD_PEERS				14500
#define HAL_NEWT_ADD_PEERS_URL_TEXT	HAL_NEWT_ADD_PEERS + 1
#define HAL_NEWT_ADD_PEERS_URL_EDIT	HAL_NEWT_ADD_PEERS + 2
#define HAL_NEWT_ADD_PEERS_TYP_TEXT	HAL_NEWT_ADD_PEERS + 3
#define HAL_NEWT_ADD_PEERS_TYP_CBOX	HAL_NEWT_ADD_PEERS + 4
#define HAL_NEWT_ADD_DHT_PORT			HAL_NEWT_ADD_PEERS + 7
#define HAL_NEWT_ADD_DHT_PORT_TEXT		HAL_NEWT_ADD_PEERS + 8

#define HAL_NEWT_ADD_PEERS_WEB			HAL_NEWT_ADD_PEERS + 5
#define HAL_NEWT_ADD_PEERS_DHT			HAL_NEWT_ADD_PEERS + 6
#define HAL_NEWT_EDIT_PEER				HAL_NEWT_ADD_PEERS + 9
#define HAL_NEWT_ADD_NEW_PEER			HAL_NEWT_ADD_PEERS + 10

#ifndef RC_INVOKED

#include "stdAfx.hpp"
#include "global/string_conv.hpp"
#include "halIni.hpp"
#include "HaliteSortListViewCtrl.hpp"

#include "GenericAddDialog.hpp"
#include "GenericAddListView.hpp"

class NewTorrent_PeersAddDialog :
	public ATL::CDialogImpl<NewTorrent_PeersAddDialog>,
	public WTLx::GenericAddDialog<NewTorrent_PeersAddDialog, HAL_PEEREDIT>,
	public WTL::CDialogResize<NewTorrent_PeersAddDialog>,
	public WTLx::WinDataExchangeEx<NewTorrent_PeersAddDialog>
{
public:
	typedef NewTorrent_PeersAddDialog this_class_t;
	typedef WTLx::GenericAddDialog<this_class_t, HAL_PEEREDIT> generic_dlg_class_t;
	typedef WTL::CDialogResize<this_class_t> resize_class_t;
	
public:
	NewTorrent_PeersAddDialog(wstring title, hal::web_seed_or_dht_node_detail& peer) :
		generic_dlg_class_t(title, L"genericAddDlgs/NewTorrentAddPeers", L"NewTorrentAddPeers"),
		peer_(peer)
	{}

	BEGIN_MSG_MAP_EX(this_class_t)
		try
		{
		MSG_WM_INITDIALOG(onInitDialog)
		COMMAND_HANDLER_EX(HAL_NEWT_ADD_PEERS_TYP_CBOX, CBN_SELCHANGE, OnTypeChanged)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in NewTorrent_PeersAddDialog MSG_MAP")

		CHAIN_MSG_MAP(resize_class_t)
		CHAIN_MSG_MAP(generic_dlg_class_t)
	END_MSG_MAP()

	BEGIN_DDX_MAP(this_class_t)
		DDX_EX_STDWSTRING(HAL_NEWT_ADD_PEERS_URL_EDIT, peer_.url);
		DDX_INT(HAL_NEWT_ADD_DHT_PORT, peer_.port)
		DDX_EX_STDWSTRING(HAL_NEWT_ADD_PEERS_TYP_CBOX, peer_.type)
	END_DDX_MAP()	

	BEGIN_DLGRESIZE_MAP(this_class_t)
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

		resize_class_t::DlgResize_Init(false, true, WS_CLIPCHILDREN);

		SetMsgHandled(false);
		return 0;
	}
	
	void OnTypeChanged(UINT uNotifyCode, int nID, CWindow wndCtl)
	{	
		hal::win_c_str<std::wstring> str_buf(MAX_PATH);		
		wndCtl.GetWindowText(str_buf, numeric_cast<int>(str_buf.size()));
		
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

class NewTorrent_PeersListViewCtrl :
	public CHaliteSortListViewCtrl<NewTorrent_PeersListViewCtrl>,
	public hal::IniBase<NewTorrent_PeersListViewCtrl>,
	public WTLx::GenericAddListView<NewTorrent_PeersListViewCtrl, true>,
	private boost::noncopyable
{
	typedef NewTorrent_PeersListViewCtrl this_class_t;
	typedef hal::IniBase<this_class_t> ini_class_t;
	typedef CHaliteSortListViewCtrl<this_class_t> list_class_t;
	typedef WTLx::GenericAddListView<this_class_t, true> genericAddlist_class_t;

	friend class list_class_t;
	
public:
	enum { 
		LISTVIEW_ID_MENU = HAL_GENERIC_ADD_LV_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_NTPLVM_NAMES,
		LISTVIEW_ID_COLUMNWIDTHS = HAL_TRACKER_LISTVIEW_DEFAULTS
	};
	
	NewTorrent_PeersListViewCtrl() :
		ini_class_t(L"listviews/new_torrent_peers", L"new_peers_listview")
	{}

	BEGIN_MSG_MAP_EX(this_class_t)
		try
	{
		MSG_WM_DESTROY(OnDestroy)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in NewTorrent_PeersListViewCtrl MSG_MAP")

		CHAIN_MSG_MAP(genericAddlist_class_t)
		CHAIN_MSG_MAP(list_class_t)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void uiUpdate(const hal::torrent_details_ptr pT);
	void saveSettings();

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 2:			
			ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<list_class_t>(*this));
		
		break;

		case 1:
		default:
			assert(false);
		}
	}

	void newItem();
	void editItem(int);
	void deleteItem(int);

private:
	void OnAttach();
	void OnDestroy();
};

typedef NewTorrent_PeersListViewCtrl::SelectionManager NewTorrent_PeersListViewManager;

BOOST_CLASS_VERSION(NewTorrent_PeersListViewCtrl, 2)

#endif
