
#include "HaliteListViewCtrl.hpp"

#include "GlobalIni.hpp"
#include "ini/Window.hpp"
#include "halTorrent.hpp"

void selection_manager::sync_list(bool list_to_manager)
{
	if (list_to_manager)
	{	
		all_selected_.clear();
		int itemPos = m_list_.GetSelectionMark();	
	
		if (itemPos != -1)
		{
			boost::array<wchar_t, MAX_PATH> pathBuffer;
			m_list_.GetItemText(itemPos, 0, pathBuffer.c_array(), pathBuffer.size());	
			
			// Multi-Selected
			int total = m_list_.GetItemCount();
			
			for (int i=0; i<total; ++i)
			{
				UINT flags = m_list_.GetItemState(i, LVIS_SELECTED);
				
				if (flags && LVIS_SELECTED)
				{
					m_list_.GetItemText(i, 0, pathBuffer.c_array(), pathBuffer.size());	
					all_selected_.push_back(wcstombs(pathBuffer.data()));
				}
			}
			
			// Single-Selected
			string selected = wcstombs(pathBuffer.data());
			
			if (selected_ != selected)
			{
				selected_ = selected;
				signal();
			}
		}
		else
		{
			selected_ = "";
			signal();
		}
	}
	else
	{
		LV_FINDINFO findInfo = { sizeof(LV_FINDINFO) }; 
		findInfo.flags = LVFI_STRING;
		
		wstring torrent_name = mbstowcs(selected_);		
		findInfo.psz = torrent_name.c_str();
		
		int itemPos = m_list_.FindItem(&findInfo, -1);	
		
		if (itemPos != m_list_.GetSelectionMark())
		{
			LVITEM lvi = { LVIF_STATE };
			lvi.state = LVIS_SELECTED;
			lvi.stateMask = LVIS_SELECTED;
			m_list_.SetItemState(itemPos, &lvi);
			m_list_.SetSelectionMark(itemPos);
			signal();
		}
	}
}

void selection_manager::setSelected(int itemPos)
{
	LVITEM lvi = { LVIF_STATE };
	lvi.state = LVIS_SELECTED;
	lvi.stateMask = LVIS_SELECTED;
	m_list_.SetItemState(itemPos, &lvi);
	m_list_.SetSelectionMark(itemPos);
	sync_list(true);
}

void selection_manager::clear()
{
	m_list_.DeleteItem(m_list_.GetSelectionMark());
	
//	m_list_.SelectItem(0);
	sync_list(true);
}

void selection_manager::clearAllSelected()
{
	int total = m_list_.GetItemCount();
	
	for (int i=0; i<total; ++i)
	{
		UINT flags = m_list_.GetItemState(i, LVIS_SELECTED);
		
		if (flags && LVIS_SELECTED)
			m_list_.DeleteItem(i);
	}
	all_selected_.clear();
	
//	m_list_.SelectItem(0);
	sync_list(true);
}

HaliteListViewCtrl::HaliteListViewCtrl() :
	manager_(*this)
{
	BOOL menu_not_created = torrentMenu_.LoadMenu(IDR_LISTVIEW_MENU);
	assert(menu_not_created);

}

void HaliteListViewCtrl::onShowWindow(UINT, INT)
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);

	CHeaderCtrl hdr = GetHeader();
	hdr.ModifyStyle(0, HDS_DRAGDROP|HDS_FULLDRAG);

	AddColumn(L"Name", hdr.GetItemCount());
	AddColumn(L"Status", hdr.GetItemCount());
	AddColumn(L"Completed", hdr.GetItemCount());
	AddColumn(L"Downloaded", hdr.GetItemCount());
	AddColumn(L"Upload", hdr.GetItemCount());
	AddColumn(L"Peers", hdr.GetItemCount());
	AddColumn(L"Seeds", hdr.GetItemCount());
	AddColumn(L"ETA", hdr.GetItemCount());
	AddColumn(L"Copies", hdr.GetItemCount());

	for (size_t i=0; i<WindowConfig::numMainCols; ++i)
		SetColumnWidth(i, INI().windowConfig().mainListColWidth[i]);

	for (size_t i=0; i<WindowConfig::numMainColsEx; ++i)
		SetColumnWidth(i+WindowConfig::numMainCols, INI().windowConfig().mainListColWidthEx[i]);
}

void HaliteListViewCtrl::updateListView()
{
	hal::TorrentDetails TD;
	hal::bittorrent().getAllTorrentDetails(TD);
	
	for (hal::TorrentDetails::const_iterator i = TD.begin(); i != TD.end(); ++i) 
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
			SetItemText(itemPos, 7,	L"∞");		
		}
		
		SetItemText(itemPos, 8,	(wformat(L"%1$.2f") 
				% ((*i)->distributedCopies())
			).str().c_str());	
	}	
}

void HaliteListViewCtrl::saveStatus()
{
	for (size_t i=0; i<WindowConfig::numMainCols; ++i)
		INI().windowConfig().mainListColWidth[i] = GetColumnWidth(i);

	for (size_t i=0; i<WindowConfig::numMainColsEx; ++i)
		INI().windowConfig().mainListColWidthEx[i] = GetColumnWidth(i+WindowConfig::numMainCols);
}

LRESULT HaliteListViewCtrl::OnClick(int, LPNMHDR pnmh, BOOL&)
{
	manager().sync_list(true);

	return 0;
}

LRESULT HaliteListViewCtrl::OnDblClick(int, LPNMHDR pnmh, BOOL&)
{
	CEdit edit = EditLabel(0);
//	edit.SetWindowText(L"Hello");

	return 0;
}


LRESULT HaliteListViewCtrl::OnRClick(int i, LPNMHDR pnmh, BOOL&)
{
	LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)pnmh;	
	manager().sync_list(true);
	
	assert (torrentMenu_.IsMenu());
	CMenuHandle sMenu = torrentMenu_.GetSubMenu(0);
	assert (sMenu.IsMenu());
	
	POINT ptPoint;
	GetCursorPos(&ptPoint);
	sMenu.TrackPopupMenu(0, ptPoint.x, ptPoint.y, m_hWnd);
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnColClick(int i, LPNMHDR pnmh, BOOL&)
{
	LPNMLISTVIEW pnlv = (LPNMLISTVIEW)pnmh;
//	MessageBox(lexical_cast<wstring>(pnlv->iSubItem).c_str(), L"ListView",0);
//	DeleteColumn(pnlv->iSubItem);

	return 0;
}

LRESULT HaliteListViewCtrl::OnBeginEdit(int i, LPNMHDR pnmh, BOOL&)
{
//	LPNMLVDISPINFO pdi = (LPNMLVDISPINFO)pnmh;
	
	LVHITTESTINFO hti;	
	GetCursorPos(&hti.pt);
	ScreenToClient(&hti.pt);
	SubItemHitTest(&hti);
	
	RECT rect;
	GetSubItemRect(hti.iItem, hti.iSubItem, 0, &rect);
	ScreenToClient(&rect);
	MessageBox((wformat(L"%1%, %2% -> %3%, %4%") 
		% rect.left % rect.top % rect.right % rect.bottom).str().c_str(), L"Msg", 0);
	
	CEdit edit = GetEditControl();
//	edit.MoveWindow(rect.left, rect.top, rect.right, rect.bottom, true);
	edit.SetWindowPos(HWND_TOP, 0,0, 100, 50,SWP_SHOWWINDOW);
//	MessageBox(lexical_cast<wstring>(pnlv->iSubItem).c_str(), L"ListView",0);
//	DeleteColumn(pnlv->iSubItem);

	return 0;
}

LRESULT HaliteListViewCtrl::OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager_.allSelected().begin(), manager_.allSelected().end(),
		bind(&hal::BitTorrent::resumeTorrent, &hal::bittorrent(), _1));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager_.allSelected().begin(), manager_.allSelected().end(),
		bind(&hal::BitTorrent::pauseTorrent, &hal::bittorrent(), _1));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager_.allSelected().begin(), manager_.allSelected().end(),
		bind(&hal::BitTorrent::stopTorrent, &hal::bittorrent(), _1));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager_.allSelected().begin(), manager_.allSelected().end(),
		bind(&hal::BitTorrent::removeTorrent, &hal::bittorrent(), _1));

	manager_.clearAllSelected();	
	return 0;
}

LRESULT HaliteListViewCtrl::OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager_.allSelected().begin(), manager_.allSelected().end(),
		bind(&hal::BitTorrent::removeTorrentWipeFiles, &hal::bittorrent(), _1));
	
	manager_.clearAllSelected();
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
