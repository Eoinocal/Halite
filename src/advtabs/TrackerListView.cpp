
#include "TrackerListView.hpp"

//#include "../GlobalIni.hpp"
//#include "../ini/Window.hpp"
#include "../halTorrent.hpp"
#include "TrackerAddDialog.hpp"

void TrackerListViewCtrl::OnAttach()
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);

	CHeaderCtrl hdr = GetHeader();
	hdr.ModifyStyle(0, HDS_DRAGDROP|HDS_FULLDRAG);

	AddColumn(L"Tracker", hdr.GetItemCount());
	AddColumn(L"Tier", hdr.GetItemCount());
	
	assert (hdr.GetItemCount() == numListColumnWidth);
	
	for (int i=0; i<numListColumnWidth; ++i)
	{
		SetColumnWidth(i, listColumnWidth[i]);
	}
	
	SetColumnOrderArray(numListColumnWidth, (int*)&listColumnOrder);	
}

void TrackerListViewCtrl::OnDestroy()
{
	saveSettings();
}

void TrackerListViewCtrl::saveSettings()
{
	assert (GetHeader().GetItemCount() == numListColumnWidth);
	
	GetColumnOrderArray(numListColumnWidth, (int*)&listColumnOrder);
	
	for (int i=0; i<numListColumnWidth; ++i)
	{
		listColumnWidth[i] = GetColumnWidth(i);
	}
	
	save();
}

void TrackerListViewCtrl::updateListView()
{
	// Should I be doing something here?
}

void TrackerListViewCtrl::enterNewTracker()
{
	hal::TrackerDetail tracker(L"", 0);	
	TrackerAddDialog trackDlg(L"Add New Tracker", tracker);
	
	if (trackDlg.DoModal() == 1 && !tracker.url.empty()) 
	{
		int itemPos = AddItem(0, 0, tracker.url.c_str(), 0);		
		SetItemText(itemPos, 1, lexical_cast<wstring>(tracker.tier).c_str());
		
		listEdited_();	
	}
}

LRESULT TrackerListViewCtrl::OnDoubleClick(int i, LPNMHDR pnmh, BOOL&)
{
	enterNewTracker();

	return 0;
}

LRESULT TrackerListViewCtrl::OnNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	enterNewTracker();
	
	return 0;
}

LRESULT TrackerListViewCtrl::OnEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	array<wchar_t, MAX_PATH> buffer;
	int index = manager().selectedIndex();
	
	GetItemText(index, 0, buffer.elems, buffer.size());
	hal::TrackerDetail tracker(wstring(buffer.elems), 0);
	
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
	return 0;
}

LRESULT TrackerListViewCtrl::OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DeleteItem(manager().selectedIndex());
	listEdited_();
	
	return 0;
}
