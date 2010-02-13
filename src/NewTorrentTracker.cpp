
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include "halTorrent.hpp"
#include "NewTorrentTracker.hpp"

void NewTorrent_TrackerListViewCtrl::OnAttach()
{
	WTL::CMenuHandle menu;
	BOOL menu_created = menu.LoadMenu(LISTVIEW_ID_MENU);
	InitialSetup(menu);

	std::vector<wstring> names;	
	wstring column_names = hal::app().res_wstr(HAL_TRACKER_LISTVIEW_COLUMNS);

	// "Tracker;Tier"
	boost::split(names, column_names, boost::is_any_of(L";"));
	
	array<int, 2> widths = {287, 50};
	array<int, 2> order = {0, 1};
	array<bool, 2> visible = {true, true};	
	
	for (int i=0, e=2; i < e; ++i)
		AddColumn(names[i].c_str(), i, visible[i], widths[i]);	
	
	load_from_ini();	
	SetColumnSortType(1, WTL::LVCOLSORT_LONG);
}

void NewTorrent_TrackerListViewCtrl::OnDestroy()
{
	saveSettings();
}

void NewTorrent_TrackerListViewCtrl::saveSettings()
{		
	GetListViewDetails();
	save_to_ini();
}

void NewTorrent_TrackerListViewCtrl::uiUpdate(const hal::torrent_details_ptr pT)
{
	if (hal::bit::torrent t = hal::bittorrent::Instance().get(pT))
	{			
		if (hal::try_update_lock<listClass> lock = hal::try_update_lock<listClass>(this)) 
		{			
			std::vector<hal::tracker_detail> trackers = t.trackers;
			DeleteAllItems();
			
			foreach (const hal::tracker_detail& tracker, trackers)
			{
				int item_pos = AddItem(0, 0, tracker.url.c_str(), 0);
				SetItemText(item_pos, 1, lexical_cast<wstring>(tracker.tier).c_str());
			}
		}
	}
	else
	{		
		DeleteAllItems();
	}
}

void NewTorrent_TrackerListViewCtrl::newItem()
{
	hal::tracker_detail tracker(L"", 0);	
	NewTorrent_TrackerAddDialog trackDlg(hal::app().res_wstr(HAL_NEWT_ADD_NEW_TRACKER), tracker);
	
	if (trackDlg.DoModal() == 1 && !tracker.url.empty()) 
	{
		int itemPos = AddItem(0, 0, tracker.url.c_str(), 0);		
		SetItemText(itemPos, 1, lexical_cast<wstring>(tracker.tier).c_str());
	}
}

void NewTorrent_TrackerListViewCtrl::editItem(int index)
{
	array<wchar_t, MAX_PATH> buffer;

	GetItemText(index, 0, buffer.elems, numeric_cast<int>(buffer.size()));
	hal::tracker_detail tracker(wstring(buffer.elems), 0);
	
	GetItemText(index, 1, buffer.elems, numeric_cast<int>(buffer.size()));
	tracker.tier = lexical_cast<int>(wstring(buffer.elems));

	NewTorrent_TrackerAddDialog trackDlg(hal::app().res_wstr(HAL_NEWT_EDIT_TRACKER), tracker);
	
	if (trackDlg.DoModal() == 1) 
	{
		if (tracker.url.empty())
		{
			DeleteItem(index);
		}
		else
		{
			SetItemText(index, 0, tracker.url.c_str());	
			SetItemText(index, 1, lexical_cast<wstring>(tracker.tier).c_str());
		}		
	}	
}

void NewTorrent_TrackerListViewCtrl::deleteItem(int index)
{
	DeleteItem(index);
}
