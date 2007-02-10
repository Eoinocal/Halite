
#include "TrackerListView.hpp"

#include "../GlobalIni.hpp"
#include "../ini/Window.hpp"
#include "../halTorrent.hpp"
#include "TrackerAddDialog.hpp"

void TrackerListViewCtrl::OnAttach()
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);

	CHeaderCtrl hdr = GetHeader();
	hdr.ModifyStyle(0, HDS_DRAGDROP|HDS_FULLDRAG);

	AddColumn(L"Tracker", hdr.GetItemCount());
	AddColumn(L"Tier", hdr.GetItemCount());
	
	SetColumnWidth(0, 200);
/*
	for (size_t i=0; i<WindowConfig::numMainCols; ++i)
		SetColumnWidth(i, INI().windowConfig().mainListColWidth[i]);

	for (size_t i=0; i<WindowConfig::numMainColsEx; ++i)
		SetColumnWidth(i+WindowConfig::numMainCols, INI().windowConfig().mainListColWidthEx[i]);
*/
}

void TrackerListViewCtrl::updateListView()
{

}

void TrackerListViewCtrl::saveStatus()
{
/*	for (size_t i=0; i<WindowConfig::numMainCols; ++i)
		INI().windowConfig().mainListColWidth[i] = GetColumnWidth(i);

	for (size_t i=0; i<WindowConfig::numMainColsEx; ++i)
		INI().windowConfig().mainListColWidthEx[i] = GetColumnWidth(i+WindowConfig::numMainCols);
*/
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
