
#include "HaliteListView.hpp"
#include "HaliteWindow.hpp"
#include "halTorrent.hpp"



HaliteListViewCtrl::HaliteListViewCtrl(HaliteWindow& HalWindow) :
	halWindow_(HalWindow),
	iniClass("listviews/halite", "HaliteListView"),
	listClass(true, false, false)
{		
	HalWindow.connectUiUpdate(bind(&HaliteListViewCtrl::uiUpdate, this, _1));
	
	std::vector<wstring> names;	
	wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);
	
	// "Name;Status;Completed;Download;Upload;Peers;Seeds;ETA;Copies;Tracker;Reannounce;Ratio"
	boost::split(names, column_names, boost::is_any_of(L";"));
	
	array<int, 12> widths = {100,110,60,60,60,42,45,61,45,45,45,45};
	array<int, 12> order = {0,1,2,3,4,5,6,7,8,9,10,11};
	array<bool, 12> visible = {true,true,true,true,true,true,true,true,true,true,true,true};
	
	SetDefaults(names, widths, order, visible);
	Load();
}
	
void HaliteListViewCtrl::OnShowWindow(UINT, INT)
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_DOUBLEBUFFER);
	SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
	
	ApplyDetails();
	
	SetColumnSortType(0, LVCOLSORT_CUSTOM, new ColumnAdapters::Filename());
	SetColumnSortType(1, LVCOLSORT_CUSTOM, new ColumnAdapters::State());
	SetColumnSortType(2, LVCOLSORT_CUSTOM, new ColumnAdapters::Completion());
	SetColumnSortType(3, LVCOLSORT_CUSTOM, new ColumnAdapters::SpeedDown());
	SetColumnSortType(4, LVCOLSORT_CUSTOM, new ColumnAdapters::SpeedUp());
	SetColumnSortType(5, LVCOLSORT_CUSTOM, new ColumnAdapters::Peers());
	SetColumnSortType(6, LVCOLSORT_CUSTOM, new ColumnAdapters::Seeds());
	SetColumnSortType(7, LVCOLSORT_CUSTOM, new ColumnAdapters::ETA());
	SetColumnSortType(8, LVCOLSORT_CUSTOM, new ColumnAdapters::DistributedCopies());
	SetColumnSortType(9, LVCOLSORT_CUSTOM, new ColumnAdapters::Tracker());
	SetColumnSortType(10, LVCOLSORT_CUSTOM, new ColumnAdapters::UpdateTrackerIn());
	SetColumnSortType(11, LVCOLSORT_CUSTOM, new ColumnAdapters::Ratio());
}

void HaliteListViewCtrl::OnDestroy()
{
	saveSettings();
}

void HaliteListViewCtrl::saveSettings()
{
	GetListViewDetails();
	Save();
}

void HaliteListViewCtrl::uiUpdate(const hal::TorrentDetails& tD)
{
	TryUpdateLock<listClass> lock(*this);
	if (lock) 
	{
	
	foreach (const hal::TorrentDetail_ptr td, tD.torrents()) 
	{
		LV_FINDINFO findInfo; 
		findInfo.flags = LVFI_STRING;
		findInfo.psz = const_cast<LPTSTR>(td->filename().c_str());
		
		int itemPos = FindItem(&findInfo, -1);
		if (itemPos < 0)
			itemPos = AddItem(0, 0, td->filename().c_str(), 0);

		for (size_t i=1; i<HaliteListViewCtrl::NumberOfColumns_s; ++i)
		{
			SetItemText(itemPos, i, getColumnAdapter(i)->print(td).c_str());
		}
	}
	
	int iCol = GetSortColumn();
	if (autoSort() && iCol >= 0 && iCol < m_arrColSortType.GetSize())
		DoSortItems(iCol, IsSortDescending());
	
	}
}

HaliteListViewCtrl::tD HaliteListViewCtrl::CustomItemConversion(LVCompareParam* param, int iSortCol)
{
	boost::array<wchar_t, MAX_PATH> buffer;
	GetItemText(param->iItem, 0, buffer.c_array(), buffer.size());		
	wstring torrent = buffer.data();
	
	return hal::bittorrent().torrentDetails().get(torrent);
}

LRESULT HaliteListViewCtrl::OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&hal::BitTorrent::resumeTorrent, &hal::bittorrent(), bind(&hal::to_utf8, _1)));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&hal::BitTorrent::pauseTorrent, &hal::bittorrent(), bind(&hal::to_utf8, _1)));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&hal::BitTorrent::stopTorrent, &hal::bittorrent(),bind(&hal::to_utf8, _1)));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnRemoveFocused(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	hal::bittorrent().removeTorrent(hal::to_utf8(manager_.selected()));

	clearFocused();	
	return 0;
}

LRESULT HaliteListViewCtrl::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&hal::BitTorrent::removeTorrent, &hal::bittorrent(), bind(&hal::to_utf8, _1)));

	clearSelected();	
	return 0;
}

LRESULT HaliteListViewCtrl::OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&hal::BitTorrent::removeTorrentWipeFiles, &hal::bittorrent(), bind(&hal::to_utf8, _1)));
	
	clearSelected();
	return 0;
}


//LRESULT HaliteListViewCtrl::OnDeleteItem(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
//{
//	LPNMLISTVIEW pnmv=(LPNMLISTVIEW)pnmh;
//	T* pItem=(T*)GetItemData(pnmv->iItem);
//	ATLASSERT(pItem);
//	if (pItem)	// Delete attached structure
//		delete pItem;
//	return 0;
//}
