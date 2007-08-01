
#include "HaliteListView.hpp"
#include "HaliteWindow.hpp"
#include "halTorrent.hpp"

HaliteListViewCtrl::HaliteListViewCtrl(HaliteWindow& HalWindow) :
	halWindow_(HalWindow),
	iniClass("listviews/halite", "HaliteListView")
{		
	HalWindow.connectUiUpdate(bind(&HaliteListViewCtrl::uiUpdate, this, _1));
	load();
	
	adapters_.push_back(new Adapters::Filename());
	adapters_.push_back(new Adapters::State());
	adapters_.push_back(new Adapters::Tracker());
}
	
void HaliteListViewCtrl::OnShowWindow(UINT, INT)
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);
	SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
	
	SetListViewDetails();
}

void HaliteListViewCtrl::OnDestroy()
{
	saveSettings();
}

void HaliteListViewCtrl::saveSettings()
{
	GetListViewDetails();
	save();
}

void HaliteListViewCtrl::uiUpdate(const hal::TorrentDetails& tD)
{
	if (canUpdate())
	{
	UpdateLock<listClass> rLock(*this);
	
	tD.sort(bind(&Adapter::less, &adapters_[1], _1, _2));
	
//	DeleteAllItems();
	
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

//	manager_.sync_list(false, false);
	}
}

int HaliteListViewCtrl::CompareItemsCustom(LVCompareParam* pItem1, LVCompareParam* pItem2, int iSortCol)
{	
	boost::array<wchar_t, MAX_PATH> buffer;
	
	GetItemText(pItem1->iItem, 0, buffer.c_array(), buffer.size());		
	wstring torrent1 = buffer.data();
	
	GetItemText(pItem2->iItem, 0, buffer.c_array(), buffer.size());		
	wstring torrent2 = buffer.data();
		
	bool less = adapters_[1].less(halWindow_.torrents().get(torrent1), halWindow_.torrents().get(torrent2));
		
	return (less) ? 1 : -1;
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
