
#include "HaliteListViewCtrl.hpp"

#include "GlobalIni.hpp"
#include "ini/Window.hpp"
#include "halTorrent.hpp"

void selection_manager::sync_list(bool list_to_manager)
{
	if (list_to_manager)
	{	
		int itemPos = m_list_.GetSelectionMark();	
	
		if (itemPos != -1)
		{
			boost::array<wchar_t, MAX_PATH> pathBuffer;
			m_list_.GetItemText(itemPos, 0, pathBuffer.c_array(), pathBuffer.size());	
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
	halite::bittorrent().removeTorrent(selected_);
	
//	m_list_.SelectItem(0);
	sync_list(true);
}

HaliteListViewCtrl::HaliteListViewCtrl() :
	manager_(*this)
{}

void HaliteListViewCtrl::onShowWindow(UINT, INT)
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT);

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

//LRESULT HaliteListViewCtrl::OnDeleteItem(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
//{
//	LPNMLISTVIEW pnmv=(LPNMLISTVIEW)pnmh;
//	T* pItem=(T*)GetItemData(pnmv->iItem);
//	ATLASSERT(pItem);
//	if (pItem)	// Delete attached structure
//		delete pItem;
//	return 0;
//}
