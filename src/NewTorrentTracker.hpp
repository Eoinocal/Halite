
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define ID_NTTLVM_BEGIN	 			18000
#define ID_NTTLVM_NEW 				ID_NTTLVM_BEGIN + 1
#define ID_NTTLVM_EDIT 				ID_NTTLVM_BEGIN + 2
#define ID_NTTLVM_DELETE				ID_NTTLVM_BEGIN + 3
#define HAL_NEWT_EDIT_TRACKER		ID_NTTLVM_BEGIN + 4
#define HAL_NEWT_ADD_NEW_TRACKER	ID_NTTLVM_BEGIN + 5

#define HAL_TRACKER_ADD_BEGIN	 	15500
#define IDC_TRACKER_EDIT_URL		HAL_TRACKER_ADD_BEGIN + 1
#define IDC_TRACKER_EDIT_TIER		HAL_TRACKER_ADD_BEGIN + 2
#define IDC_TRACKER_TEXT_URL		HAL_TRACKER_ADD_BEGIN + 3
#define IDC_TRACKER_TEXT_TIER		HAL_TRACKER_ADD_BEGIN + 4

#ifndef RC_INVOKED

#include "stdAfx.hpp"
#include "global/string_conv.hpp"
#include "halIni.hpp"
#include "HaliteSortListViewCtrl.hpp"

#include "GenericAddListView.hpp"
#include "GenericAddDialog.hpp"

class NewTorrent_TrackerAddDialog :
	public ATL::CDialogImpl<NewTorrent_TrackerAddDialog>,
	public WTLx::GenericAddDialog<NewTorrent_TrackerAddDialog, HAL_TRACKEDIT>,
	public WTL::CDialogResize<NewTorrent_TrackerAddDialog>,
	public WTLx::WinDataExchangeEx<NewTorrent_TrackerAddDialog>
{
public:
	typedef NewTorrent_TrackerAddDialog this_class_t;
	typedef WTLx::GenericAddDialog<this_class_t, HAL_TRACKEDIT> generic_dlg_class_t;
	typedef WTL::CDialogResize<this_class_t> resize_class_t;
	
public:
	NewTorrent_TrackerAddDialog(wstring title, hal::tracker_detail& tracker) :
		generic_dlg_class_t(title, "genericAddDlgs/NewTorrentAddTracker", "NewTorrentAddTracker"),
		tracker_(tracker)
	{}

	BEGIN_MSG_MAP_EX(TrackerAddDialog)
		CHAIN_MSG_MAP(resize_class_t)
		CHAIN_MSG_MAP(generic_dlg_class_t)
	END_MSG_MAP()

	BEGIN_DDX_MAP(TrackerAddDialog)
		DDX_EX_STDWSTRING(IDC_TRACKER_EDIT_URL, tracker_.url);
		DDX_INT(IDC_TRACKER_EDIT_TIER, tracker_.tier)
	END_DDX_MAP()	

	BEGIN_DLGRESIZE_MAP(this_class_t)
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

class NewTorrent_TrackerListViewCtrl :
	public CHaliteSortListViewCtrl<NewTorrent_TrackerListViewCtrl>,
	public hal::IniBase<NewTorrent_TrackerListViewCtrl>,
	public WTLx::GenericAddListView<NewTorrent_TrackerListViewCtrl, true>,
	private boost::noncopyable
{
	typedef NewTorrent_TrackerListViewCtrl this_class_t;
	typedef hal::IniBase<this_class_t> ini_class_t;
	typedef CHaliteSortListViewCtrl<this_class_t> list_class_t;
	typedef WTLx::GenericAddListView<this_class_t, true> genericAddlist_class_t;

	friend class list_class_t;
	
public:
	enum { 
		LISTVIEW_ID_MENU = HAL_GENERIC_ADD_LV_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_TRACKER_LISTVIEW_COLUMNS,
		LISTVIEW_ID_COLUMNWIDTHS = HAL_TRACKER_LISTVIEW_DEFAULTS
	};
	
	NewTorrent_TrackerListViewCtrl() :
		ini_class_t("listviews/NewTorrent", "NewTorrentListView")
	{}

	BEGIN_MSG_MAP_EX(TrackerListViewCtrl)
		try
		{
		MSG_WM_DESTROY(OnDestroy)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in NewTorrent_TrackerListViewCtrl MSG_MAP")

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
		ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<list_class_t>(*this));
	}

	void newItem();
	void editItem(int);
	void deleteItem(int);

private:
	void OnAttach();
	void OnDestroy();
};

typedef NewTorrent_TrackerListViewCtrl::SelectionManager NewTorrent_TrackerListViewManager;

#endif
