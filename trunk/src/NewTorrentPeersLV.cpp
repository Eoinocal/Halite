
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include "halTorrent.hpp"

#include "NewTorrentPeersLV.hpp"
#include "NewTorrentPeersAD.hpp"

void NewTorrent_PeersListViewCtrl::OnAttach()
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);
	SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
	
	ApplyDetails();
	
	SetColumnSortType(1, LVCOLSORT_LONG);
}

void NewTorrent_PeersListViewCtrl::OnDestroy()
{
	saveSettings();
}

void NewTorrent_PeersListViewCtrl::saveSettings()
{		
	GetListViewDetails();
	save();
}

void NewTorrent_PeersListViewCtrl::newItem()
{
	hal::TrackerDetail tracker(L"", 0);	
	NewTorrent_PeersAddDialog trackDlg(L"Add New Peer", tracker);
	
	if (trackDlg.DoModal() == 1 && !tracker.url.empty()) 
	{
		int itemPos = AddItem(0, 0, tracker.url.c_str(), 0);		
		SetItemText(itemPos, 1, lexical_cast<wstring>(tracker.tier).c_str());
	}
}

void NewTorrent_PeersListViewCtrl::editItem(int index)
{
	array<wchar_t, MAX_PATH> buffer;

	GetItemText(index, 0, buffer.elems, buffer.size());
	hal::TrackerDetail tracker(wstring(buffer.elems), 0);
	
	GetItemText(index, 1, buffer.elems, buffer.size());
	tracker.tier = lexical_cast<int>(wstring(buffer.elems));

	NewTorrent_PeersAddDialog trackDlg(L"Edit Tracker", tracker);
	
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

void NewTorrent_PeersListViewCtrl::deleteItem(int index)
{
	DeleteItem(index);
}
