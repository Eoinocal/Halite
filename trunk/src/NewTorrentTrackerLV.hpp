
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define ID_NTTLVM_BEGIN	 			18000
#define ID_NTTLVM_NEW 				ID_NTTLVM_BEGIN + 1
#define ID_NTTLVM_EDIT 				ID_NTTLVM_BEGIN + 2
#define ID_NTTLVM_DELETE			ID_NTTLVM_BEGIN + 3
#define HAL_NEWT_EDIT_TRACKER		ID_NTTLVM_BEGIN + 4
#define HAL_NEWT_ADD_NEW_TRACKER	ID_NTTLVM_BEGIN + 5

#ifndef RC_INVOKED

#include <boost/signals.hpp>
#include <boost/function.hpp>

#include "stdAfx.hpp"
#include "global/string_conv.hpp"
#include "halIni.hpp"
#include "HaliteSortListViewCtrl.hpp"

#include "GenericAddListView.hpp"

class NewTorrent_TrackerListViewCtrl :
	public CHaliteSortListViewCtrl<NewTorrent_TrackerListViewCtrl>,
	public hal::IniBase<NewTorrent_TrackerListViewCtrl>,
	public WTLx::GenericAddListView<NewTorrent_TrackerListViewCtrl, true, ID_NTTLVM_NEW, ID_NTTLVM_EDIT, ID_NTTLVM_DELETE>,
	private boost::noncopyable
{
	typedef NewTorrent_TrackerListViewCtrl thisClass;
	typedef hal::IniBase<thisClass> iniClass;
	typedef CHaliteSortListViewCtrl<thisClass> listClass;
	typedef WTLx::GenericAddListView<thisClass, true, ID_NTTLVM_NEW, ID_NTTLVM_EDIT, ID_NTTLVM_DELETE> genericAddlistClass;

	friend class listClass;
	
public:
	enum { 
		LISTVIEW_ID_MENU = HAL_GENERIC_ADD_LV_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_TRACKER_LISTVIEW_COLUMNS,
		LISTVIEW_ID_COLUMNWIDTHS = HAL_TRACKER_LISTVIEW_DEFAULTS
	};
	
	NewTorrent_TrackerListViewCtrl() :
		iniClass("listviews/NewTorrent", "NewTorrentListView")
	{
		std::vector<wstring> names;	
		wstring column_names = hal::app().res_wstr(HAL_TRACKER_LISTVIEW_COLUMNS);

		// "Tracker;Tier"
		boost::split(names, column_names, boost::is_any_of(L";"));
		
		array<int, 2> widths = {287,50};
		array<int, 2> order = {0,1};
		array<bool, 2> visible = {true,true};
		
		SetDefaults(names, widths, order, visible, true);
		load_from_ini();
	}

	BEGIN_MSG_MAP_EX(TrackerListViewCtrl)
		MSG_WM_DESTROY(OnDestroy)

		CHAIN_MSG_MAP(genericAddlistClass)
		CHAIN_MSG_MAP(listClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void uiUpdate(const hal::torrent_details_ptr pT);
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

typedef NewTorrent_TrackerListViewCtrl::SelectionManager NewTorrent_TrackerListViewManager;

#endif // RC_INVOKED
