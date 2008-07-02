
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include "../halTorrent.hpp"

#include "TrackerListView.hpp"
#include "TrackerAddDialog.hpp"

void TrackerListViewCtrl::OnAttach()
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);
	SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
	
	ApplyDetails();
	
	SetColumnSortType(1, WTL::LVCOLSORT_LONG);
}

void TrackerListViewCtrl::OnDestroy()
{
	saveSettings();
}

void TrackerListViewCtrl::saveSettings()
{		
	GetListViewDetails();
	save_to_ini();
}

void TrackerListViewCtrl::uiUpdate(const hal::TorrentDetail_ptr pT)
{
	if (hal::bit::torrent t = hal::bittorrent().get(pT))
	{			
		hal::try_update_lock<listClass> lock(*this);
		if (lock) 
		{			
			std::vector<hal::tracker_detail> trackers = t.trackers;				
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

void TrackerListViewCtrl::newItem()
{
	hal::tracker_detail tracker(L"", 0);	
	TrackerAddDialog trackDlg(L"Add New Tracker", tracker);
	
	if (trackDlg.DoModal() == 1 && !tracker.url.empty()) 
	{
		int itemPos = AddItem(0, 0, tracker.url.c_str(), 0);		
		SetItemText(itemPos, 1, lexical_cast<wstring>(tracker.tier).c_str());
		
		listEdited_();	
	}
}

void TrackerListViewCtrl::editItem(int index)
{
	array<wchar_t, MAX_PATH> buffer;

	GetItemText(index, 0, buffer.elems, buffer.size());
	hal::tracker_detail tracker(wstring(buffer.elems), 0);
	
	GetItemText(index, 1, buffer.elems, buffer.size());
	tracker.tier = lexical_cast<int>(wstring(buffer.elems));

	TrackerAddDialog trackDlg(L"Edit Tracker", tracker);
	
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
		listEdited_();
	}	
}

void TrackerListViewCtrl::deleteItem(int index)
{
	DeleteItem(index);
	listEdited_();
}

LRESULT TrackerListViewCtrl::OnPrimary(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	hal::try_update_lock<listClass> lock(*this);
	if (lock) 
	{	
	
	for (int i=0, e=GetItemCount(); i<e; ++i)
	{
		array<wchar_t, MAX_PATH> buffer;		
		GetItemText(i, 1, buffer.elems, buffer.size());
		
		if (wstring(buffer.elems) == L"0")
			SetItemText(i, 1, L"1");
	}
	
	SetItemText(manager().selectedIndex(), 1, L"0");
	
	listEdited_();
	
	}	
	return 0;
}
