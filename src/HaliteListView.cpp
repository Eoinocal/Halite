
#include "HaliteListView.hpp"
#include "HaliteWindow.hpp"
#include "halTorrent.hpp"

HaliteListViewCtrl::HaliteListViewCtrl(HaliteWindow& HalWindow) :
	iniClass("listviews/halite", "HaliteListView")
{		
	HalWindow.connectUiUpdate(bind(&HaliteListViewCtrl::uiUpdate, this, _1));
	load();
}
	
void HaliteListViewCtrl::OnShowWindow(UINT, INT)
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);

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
	
	tD.sort(hal::TorrentDetails::name);
	DeleteAllItems();
	
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

	manager_.sync_list(false, false);
//	hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::Event::info, L"Two: Sync")));
	}
}

LRESULT HaliteListViewCtrl::OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&hal::BitTorrent::resumeTorrent, &hal::bittorrent(), _1));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&hal::BitTorrent::pauseTorrent, &hal::bittorrent(), _1));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&hal::BitTorrent::stopTorrent, &hal::bittorrent(), _1));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&hal::BitTorrent::removeTorrent, &hal::bittorrent(), _1));

//	manager().clearAllSelected();	
	return 0;
}

LRESULT HaliteListViewCtrl::OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind(&hal::BitTorrent::removeTorrentWipeFiles, &hal::bittorrent(), _1));
	
//	manager().clearAllSelected();
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
