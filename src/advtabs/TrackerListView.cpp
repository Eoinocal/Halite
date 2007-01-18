
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
/*
	for (size_t i=0; i<WindowConfig::numMainCols; ++i)
		SetColumnWidth(i, INI().windowConfig().mainListColWidth[i]);

	for (size_t i=0; i<WindowConfig::numMainColsEx; ++i)
		SetColumnWidth(i+WindowConfig::numMainCols, INI().windowConfig().mainListColWidthEx[i]);
*/
}

void TrackerListViewCtrl::updateListView()
{
	halite::TorrentDetails TD;
	halite::bittorrent().getAllTorrentDetails(TD);
	
	for (halite::TorrentDetails::const_iterator i = TD.begin(); i != TD.end(); ++i) 
	{
		LV_FINDINFO findInfo; 
		findInfo.flags = LVFI_STRING;
		findInfo.psz = const_cast<LPTSTR>((*i)->filename().c_str());
		
		int itemPos = FindItem(&findInfo, -1);
		if (itemPos < 0)
			itemPos = AddItem(0, 0, (*i)->filename().c_str(), 0);
		
		SetItemText(itemPos, 1, (*i)->state().c_str());
		
		SetItemText(itemPos, 2, (wformat(L"%1$.2f%%") 
				% ((*i)->completion()*100)).str().c_str());
		
		SetItemText(itemPos, 3, (wformat(L"%1$.2fkb/s") 
				% ((*i)->speed().first/1024)).str().c_str());	
		
		SetItemText(itemPos, 4, (wformat(L"%1$.2fkb/s") 
				% ((*i)->speed().second/1024)).str().c_str());
		
		SetItemText(itemPos, 5,	(lexical_cast<wstring>((*i)->peers())).c_str());
		
		SetItemText(itemPos, 6,	(lexical_cast<wstring>((*i)->seeds())).c_str());	

		if (!(*i)->estimatedTimeLeft().is_special())
		{
			SetItemText(itemPos, 7,	(mbstowcs(
				boost::posix_time::to_simple_string((*i)->estimatedTimeLeft())).c_str()));
		}
		else
		{
			SetItemText(itemPos, 7,	L"8");		
		}
		
		SetItemText(itemPos, 8,	(wformat(L"%1$.2f") 
				% ((*i)->distributedCopies())
			).str().c_str());	
	}	
}

void TrackerListViewCtrl::saveStatus()
{
/*	for (size_t i=0; i<WindowConfig::numMainCols; ++i)
		INI().windowConfig().mainListColWidth[i] = GetColumnWidth(i);

	for (size_t i=0; i<WindowConfig::numMainColsEx; ++i)
		INI().windowConfig().mainListColWidthEx[i] = GetColumnWidth(i+WindowConfig::numMainCols);
*/
}

LRESULT TrackerListViewCtrl::OnDoubleClick(int i, LPNMHDR pnmh, BOOL&)
{
	halite::TrackerDetail tracker(L"", 0);	
	TrackerAddDialog trackDlg(tracker);
	
	if (trackDlg.DoModal() == 1) 
	{
		int itemPos = AddItem(0, 0, tracker.url.c_str(), 0);		
		SetItemText(itemPos, 1, lexical_cast<wstring>(tracker.tier).c_str());	
	}

	return 0;
}

LRESULT TrackerListViewCtrl::OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&halite::BitTorrent::resumeTorrent, &halite::bittorrent(), _1));
	
	return 0;
}

LRESULT TrackerListViewCtrl::OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&halite::BitTorrent::pauseTorrent, &halite::bittorrent(), _1));
	
	return 0;
}

LRESULT TrackerListViewCtrl::OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&halite::BitTorrent::stopTorrent, &halite::bittorrent(), _1));
	
	return 0;
}

LRESULT TrackerListViewCtrl::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&halite::BitTorrent::removeTorrent, &halite::bittorrent(), _1));

	manager().clearAllSelected();	
	return 0;
}

LRESULT TrackerListViewCtrl::OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&halite::BitTorrent::removeTorrentWipeFiles, &halite::bittorrent(), _1));
	
	manager().clearAllSelected();
	return 0;
}
