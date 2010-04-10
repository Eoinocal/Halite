
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdAfx.hpp"
#include "Halite.hpp"

#include "DdxEx.hpp"
#include "global/string_conv.hpp"

#include "../HaliteTabPage.hpp"
#include "../HaliteDialogBase.hpp"
#include "../HaliteListManager.hpp"

class PeerListView :
	public CHaliteSortListViewCtrl<PeerListView, std::wstring>,
	public hal::IniBase<PeerListView>,
	private boost::noncopyable
{
protected:
	typedef PeerListView this_class_t;
	typedef hal::IniBase<this_class_t> ini_class_t;
	typedef CHaliteSortListViewCtrl<this_class_t, std::wstring> list_class_t;

	friend class list_class_t;
	
public:	
	enum { 
		LISTVIEW_ID_MENU = 0,
		LISTVIEW_ID_COLUMNNAMES = HAL_DIALOGPEER_LISTVIEW_ADV,
		LISTVIEW_ID_COLUMNWIDTHS = 0
	};
	
	BEGIN_MSG_MAP_EX(this_class_t)
		try
		{
		MSG_WM_DESTROY(OnDestroy)

		REFLECTED_NOTIFY_CODE_HANDLER(SLVN_SORTCHANGED, OnSortChanged)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in PeerListView MSG_MAP")

		CHAIN_MSG_MAP(list_class_t)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	PeerListView(HaliteWindow& halWindow) :
		ini_class_t("listviews/advPeers", "PeerListView"),
		halite_window_(halWindow)
	{}
	
	void saveSettings()
	{
		GetListViewDetails();
		save_to_ini();
	}
	
	bool SubclassWindow(HWND hwnd);
	
	void OnDestroy()
	{
		saveSettings();
	}

	LRESULT OnGetDispInfo(int, LPNMHDR pnmh, BOOL&);
	LRESULT OnSortChanged(int, LPNMHDR pnmh, BOOL&);

	bool sort_list_comparison(std::wstring l,  std::wstring r, size_t index, bool ascending);
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & boost::serialization::make_nvp("listview", 
			boost::serialization::base_object<list_class_t>(*this));
	}

	void uiUpdate(const hal::torrent_details_manager& tD);
	
private:
	hal::peer_details_vec peer_details_;
	HaliteWindow& halite_window_;
};

class AdvPeerDialog :
	public CHalTabPageImpl<AdvPeerDialog>,
	public CHaliteDialogBase<AdvPeerDialog>,
	public WTL::CDialogResize<AdvPeerDialog>
{
protected:
	typedef AdvPeerDialog this_class_t;
	typedef CHalTabPageImpl<this_class_t> base_class_t;
	typedef WTL::CDialogResize<this_class_t> resize_class_t;
	typedef CHaliteDialogBase<AdvPeerDialog> dlg_base_class_t;

public:
	enum { IDD = HAL_ADVPEER };

	AdvPeerDialog(HaliteWindow& halWindow) :
		dlg_base_class_t(halWindow),
		peerList_(halWindow)
	{}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(this_class_t)
		try
		{
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in AdvPeerDialog MSG_MAP")
	
		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;

		CHAIN_MSG_MAP(dlg_base_class_t)
		CHAIN_MSG_MAP(resize_class_t)
		CHAIN_MSG_MAP(base_class_t)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(this_class_t)
		DLGRESIZE_CONTROL(HAL_PEERLIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(HWND, LPARAM);
	void OnClose();
	void uiUpdate(const hal::torrent_details_manager& tD);

protected:
	PeerListView peerList_;
};
