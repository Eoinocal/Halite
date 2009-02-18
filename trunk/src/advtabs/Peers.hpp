
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
	typedef PeerListView thisClass;
	typedef hal::IniBase<thisClass> iniClass;
	typedef CHaliteSortListViewCtrl<thisClass, std::wstring> listClass;

	friend class listClass;
	
public:	
	enum { 
		LISTVIEW_ID_MENU = 0,
		LISTVIEW_ID_COLUMNNAMES = HAL_DIALOGPEER_LISTVIEW_ADV,
		LISTVIEW_ID_COLUMNWIDTHS = 0
	};
	
	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_DESTROY(OnDestroy)

		REFLECTED_NOTIFY_CODE_HANDLER(SLVN_SORTCHANGED, OnSortChanged)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)

		CHAIN_MSG_MAP(listClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	PeerListView(HaliteWindow& halWindow) :
		iniClass("listviews/advPeers", "PeerListView"),
		halite_window_(halWindow)
	{}
	
	void saveSettings()
	{
		GetListViewDetails();
		save_to_ini();
	}
	
	bool SubclassWindow(HWND hwnd)
	{
		if(!listClass::SubclassWindow(hwnd))
			return false;

		InitialSetup();	
		
		std::vector<wstring> names;	
		wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);

		// "Peer;Country;Download;Upload;Type;Client,Status"
		boost::split(names, column_names, boost::is_any_of(L";"));
		
		array<int, 7> widths = {100,20,70,70,70,100,200};
		array<int, 7> order = {0,1,2,3,4,5,6};
		array<bool, 7> visible = {true,true,true,true,true,true,true};

		for (int i=0, e=7; i < e; ++i)
		{
			AddColumn(names[i].c_str(), i, visible[i], widths[i]);
		}	
					
		load_from_ini();		

		for (unsigned i=0, e = hal::peer_detail::status_e-hal::peer_detail::ip_address_e; i <= e; ++i)
			SetColumnSortType(i, i + (WTL::LVCOLSORT_LAST+1+hal::peer_detail::ip_address_e), NULL);		

	//	SetColumnSortType(2, hal::peer_detail::speed_down_e + (WTL::LVCOLSORT_LAST+1+hal::peer_detail::ip_address_e), NULL);		
	//	SetColumnSortType(3, hal::peer_detail::speed_up_e + (WTL::LVCOLSORT_LAST+1+hal::peer_detail::ip_address_e), NULL);
		
		return true;
	}
	
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
			boost::serialization::base_object<listClass>(*this));
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
	typedef AdvPeerDialog thisClass;
	typedef CHalTabPageImpl<thisClass> baseClass;
	typedef WTL::CDialogResize<thisClass> resizeClass;
	typedef CHaliteDialogBase<AdvPeerDialog> dialogBaseClass;

public:
	enum { IDD = HAL_ADVPEER };

	AdvPeerDialog(HaliteWindow& halWindow) :
		dialogBaseClass(halWindow),
		peerList_(halWindow)
	{}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
	
		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;

		CHAIN_MSG_MAP(dialogBaseClass)
		CHAIN_MSG_MAP(resizeClass)
		CHAIN_MSG_MAP(baseClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(HAL_PEERLIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(HWND, LPARAM);
	void OnClose();
	void uiUpdate(const hal::torrent_details_manager& tD);

protected:
	PeerListView peerList_;
};
