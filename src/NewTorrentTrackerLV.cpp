
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include "halTorrent.hpp"

#include "NewTorrentTrackerLV.hpp"
#include "NewTorrentTrackerAD.hpp"

void NewTorrent_TrackerListViewCtrl::OnAttach()
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);
	SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
	
	ApplyDetails();
	
	SetColumnSortType(1, LVCOLSORT_LONG);
}

void NewTorrent_TrackerListViewCtrl::OnDestroy()
{
	saveSettings();
}

void NewTorrent_TrackerListViewCtrl::saveSettings()
{		
	GetListViewDetails();
	save();
}

void NewTorrent_TrackerListViewCtrl::uiUpdate(const hal::TorrentDetail_ptr pT)
{
	if (pT)
	{			
		hal::try_update_lock<listClass> lock(*this);
		if (lock) 
		{			
			std::vector<hal::tracker_detail> trackers =
				hal::bittorrent().getTorrentTrackers(pT->name());
			clearAll();
			
			foreach (const hal::tracker_detail& tracker, trackers)
			{
				int itemPos = AddItem(0, 0, tracker.url.c_str(), 0);
				SetItemText(itemPos, 1, lexical_cast<wstring>(tracker.tier).c_str());
			}
		}
	}
	else
	{		
		clearAll();
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

	GetItemText(index, 0, buffer.elems, buffer.size());
	hal::tracker_detail tracker(wstring(buffer.elems), 0);
	
	GetItemText(index, 1, buffer.elems, buffer.size());
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
