
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include "../HaliteWindow.hpp"
#include "../HaliteListView.hpp"
#include "../global/logger.hpp"

#include "Peers.hpp"

void PeerListView::uiUpdate(const hal::torrent_details_manager& tD)
{
	hal::try_update_lock<listClass> lock(*this);
	if (lock) 
	{		
		peer_details_.clear();
		
		foreach (const hal::torrent_details_ptr torrent, tD.selectedTorrents())
		{
			std::copy(torrent->get_peer_details().begin(), torrent->get_peer_details().end(), 
				std::back_inserter(peer_details_));
		}
		
		std::sort(peer_details_.begin(), peer_details_.end());
		
		// Wipe details not present
		for(int i = 0; i < GetItemCount(); /*nothing here*/)
		{
			boost::array<wchar_t, MAX_PATH> ip_address;
			GetItemText(i, 0, ip_address.c_array(), MAX_PATH);
			
			hal::peer_detail ip(ip_address.data());
			hal::peer_details_vec::iterator iter = 
				std::lower_bound(peer_details_.begin(), peer_details_.end(), ip);
			
			if (iter == peer_details_.end() || !((*iter) == ip))
			{
				DeleteItem(i);
			}
			else
			{
				SetItemData(i, std::distance(peer_details_.begin(), iter));
				++i;
			}
		}
			
		int col_sort_index = GetSortColumn();

		if (col_sort_index != -1)
		{		
			int index = GetColumnSortType(col_sort_index);
			
			HAL_DEV_SORT_MSG(hal::wform(L"col_sort_index() = %1%, index() = %2%") 
				% col_sort_index % index);

			if (index > WTL::LVCOLSORT_LAST)
				hal::peer_details_sort(peer_details_, index - (WTL::LVCOLSORT_LAST+1+hal::peer_detail::ip_address_e), 
					IsSortDescending());
		}

		bool sort_once = IsSortOnce();

		// Add additional details	
		for (size_t index = 0, e = peer_details_.size(); index < e; ++index)
		{
			hal::peer_detail& pd = peer_details_[index];
		
			int item_pos = index;
		
			HAL_DEV_SORT_MSG(hal::wform(L"AutoSort() = %1%, SortOnce() = %2%, !AutoSort() && !SortOnce() = %3%") 
				% AutoSort() % sort_once % (!AutoSort() && !sort_once));

			if (!AutoSort() && !sort_once)
			{
				LV_FINDINFO findInfo; 
				findInfo.flags = LVFI_STRING;
				findInfo.psz = const_cast<LPTSTR>(pd.to_wstring(hal::peer_detail::ip_address_e).c_str());
				
				item_pos = FindItem(&findInfo, -1);
			}

			if (item_pos == -1 || item_pos > GetItemCount())
				item_pos = AddItem(GetItemCount(), 0, pd.to_wstring(hal::peer_detail::ip_address_e).c_str(), 0);
			
			
			HAL_DEV_SORT_MSG(hal::wform(L"item_pos = %1%") % item_pos);

			SetItemData(item_pos, index);			

			for (size_t i = 0; i < 7; ++i)
			{
				SetItemText(item_pos, i, pd.to_wstring(i).c_str());
			}
		}
		
		if (AutoSort() && col_sort_index >= 0 && col_sort_index < m_arrColSortType.GetSize())
		{
			if (GetColumnSortType(col_sort_index) <= WTL::LVCOLSORT_CUSTOM)
				DoSortItems(col_sort_index, IsSortDescending());
		}
	}
}

LRESULT PeerListView::OnSortChanged(int, LPNMHDR pnmh, BOOL&)
{
	halite_window_.issueUiUpdate();
	
	return 0;
}

void AdvPeerDialog::uiUpdate(const hal::torrent_details_manager& tD)
{
	peerList_.uiUpdate(tD);
}

LRESULT AdvPeerDialog::OnInitDialog(HWND, LPARAM)
{	
	peerList_.SubclassWindow(GetDlgItem(HAL_PEERLIST));
	
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);	
	
	return 0;
}

void AdvPeerDialog::OnClose()
{
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}
