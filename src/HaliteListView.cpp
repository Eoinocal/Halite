
#include "HaliteListView.hpp"
#include "HaliteWindow.hpp"
#include "halTorrent.hpp"

HaliteListViewCtrl::HaliteListViewCtrl(HaliteWindow& HalWindow) :
	halWindow_(HalWindow),
	iniClass("listviews/halite", "HaliteListView")
{		
	HalWindow.connectUiUpdate(bind(&HaliteListViewCtrl::uiUpdate, this, _1));
	
	std::vector<wstring> names;	
	wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);
	
	boost::split(names, column_names, boost::is_any_of(L";"));
	
	array<int, 9> widths = {100,110,60,60,60,42,45,61,45};
	array<int, 9> order = {0,1,2,3,4,5,6,7,8};
	array<bool, 9> visible = {true,true,true,true,true,true,true,true,true};
	
	SetDefaults(names, widths, order, visible);
	Load();
}
	
void HaliteListViewCtrl::OnShowWindow(UINT, INT)
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);
	SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
	
	ApplyDetails();
	
	SetColumnSortType(0, LVCOLSORT_CUSTOM, new ColumnAdapters::Filename());
	SetColumnSortType(1, LVCOLSORT_CUSTOM, new ColumnAdapters::State());
	SetColumnSortType(2, LVCOLSORT_CUSTOM, new ColumnAdapters::Tracker());
	SetColumnSortType(8, LVCOLSORT_CUSTOM, new ColumnAdapters::DistributedCopies());
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
	if (canUpdate())
	{
	UpdateLock<listClass> rLock(*this);
	
	foreach (const hal::TorrentDetail_ptr td, tD.torrents()) 
	{
		LV_FINDINFO findInfo; 
		findInfo.flags = LVFI_STRING;
		findInfo.psz = const_cast<LPTSTR>(td->filename().c_str());
		
		int itemPos = FindItem(&findInfo, -1);
		if (itemPos < 0)
			itemPos = AddItem(0, 0, td->filename().c_str(), 0);
		
		SetItemText(itemPos, 1, td->state().c_str());
		
		SetItemText(itemPos, 2, (wformat(L"%1$.2f%%") 
				% (td->completion()*100)).str().c_str());
		
		SetItemText(itemPos, 3, (wformat(L"%1$.2fkb/s") 
				% (td->speed().first/1024)).str().c_str());	
		
		SetItemText(itemPos, 4, (wformat(L"%1$.2fkb/s") 
				% (td->speed().second/1024)).str().c_str());
		
		SetItemText(itemPos, 5,	(lexical_cast<wstring>(td->peers())).c_str());
		
		SetItemText(itemPos, 6,	(lexical_cast<wstring>(td->seeds())).c_str());	

		if (!td->estimatedTimeLeft().is_special())
		{
			SetItemText(itemPos, 7,	(hal::from_utf8(
				boost::posix_time::to_simple_string(td->estimatedTimeLeft())).c_str()));
		}
		else
		{
			SetItemText(itemPos, 7,	L"∞");		
		}
		
		SetItemText(itemPos, 8,	(wformat(L"%1$.2f") 
				% (td->distributedCopies())
			).str().c_str());	
	}
	
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
