
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define ID_NTPLVM_BEGIN	 	19000
#define ID_NTPLVM_NEW 		ID_NTPLVM_BEGIN + 1
#define ID_NTPLVM_EDIT 		ID_NTPLVM_BEGIN + 2
#define ID_NTPLVM_DELETE	ID_NTPLVM_BEGIN + 3
#define HAL_NTPLVM_NAMES	ID_NTPLVM_BEGIN + 4

#ifndef RC_INVOKED

#include <boost/signals.hpp>
#include <boost/function.hpp>

#include "stdAfx.hpp"
#include "global/string_conv.hpp"
#include "halIni.hpp"
#include "HaliteSortListViewCtrl.hpp"

#include "GenericAddListView.hpp"

class NewTorrent_PeersListViewCtrl :
	public CHaliteSortListViewCtrl<NewTorrent_PeersListViewCtrl>,
	public hal::IniBase<NewTorrent_PeersListViewCtrl>,
	public WTLx::GenericAddListView<NewTorrent_PeersListViewCtrl, true>,
	private boost::noncopyable
{
	typedef NewTorrent_PeersListViewCtrl thisClass;
	typedef hal::IniBase<thisClass> iniClass;
	typedef CHaliteSortListViewCtrl<thisClass> listClass;
	typedef WTLx::GenericAddListView<thisClass, true> genericAddlistClass;

	friend class listClass;
	
public:
	enum { 
		LISTVIEW_ID_MENU = HAL_GENERIC_ADD_LV_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_NTPLVM_NAMES,
		LISTVIEW_ID_COLUMNWIDTHS = HAL_TRACKER_LISTVIEW_DEFAULTS
	};
	
	NewTorrent_PeersListViewCtrl() :
		listClass(true,false,false),
		iniClass("listviews/NewTorrentPeers", "NewPeersListView")
	{
		std::vector<wstring> names;	
		wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);

		// "Tracker;Tier"
		boost::split(names, column_names, boost::is_any_of(L";"));
		
		array<int, 3> widths = {287,50,50};
		array<int, 3> order = {0,1,2};
		array<bool, 3> visible = {true,true,true};
		
		SetDefaults(names, widths, order, visible, true);
		Load();
	}

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_DESTROY(OnDestroy)

		CHAIN_MSG_MAP(genericAddlistClass)
		CHAIN_MSG_MAP(listClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void uiUpdate(const hal::TorrentDetail_ptr pT);
	void saveSettings();

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<listClass>(*this));
    }

	void newItem();
	void editItem(int);
	void deleteItem(int);

private:
	void OnAttach();
	void OnDestroy();
};

typedef NewTorrent_PeersListViewCtrl::SelectionManager NewTorrent_PeersListViewManager;

#endif // RC_INVOKED
