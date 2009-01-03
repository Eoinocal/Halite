
//         Copyright Eóin O'Callaghan 2006 - 2007.
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
		peerDetails_.clear();
		
		foreach (const hal::torrent_details_ptr torrent, tD.selectedTorrents())
		{
			std::copy(torrent->peerDetails().begin(), torrent->peerDetails().end(), 
				std::back_inserter(peerDetails_));
		}
		
		std::sort(peerDetails_.begin(), peerDetails_.end());
		
		// Wipe details not present
		for(int i = 0; i < GetItemCount(); /*nothing here*/)
		{
			boost::array<wchar_t, MAX_PATH> ip_address;
			GetItemText(i, 0, ip_address.c_array(), MAX_PATH);
			
			hal::PeerDetail ip(ip_address.data());
			hal::PeerDetails::iterator iter = 
				std::lower_bound(peerDetails_.begin(), peerDetails_.end(), ip);
			
			if (iter == peerDetails_.end() || !((*iter) == ip))
			{
				DeleteItem(i);
			}
			else
			{
				SetItemData(i, std::distance(peerDetails_.begin(), iter));
				++i;
			}
		}
		
		// Add additional details
		for (hal::PeerDetails::iterator i=peerDetails_.begin(), e=peerDetails_.end();
			i != e; ++i)
		{			
			LV_FINDINFO findInfo; 
			findInfo.flags = LVFI_STRING;
			findInfo.psz = const_cast<LPTSTR>((*i).ipAddress.c_str());
			
			int itemPos = FindItem(&findInfo, -1);
			if (itemPos < 0)
				itemPos = AddItem(GetItemCount(), 0, (*i).ipAddress.c_str(), 0);
			
			SetItemData(itemPos, std::distance(peerDetails_.begin(), i));
			
			SetItemText(itemPos, 1, (*i).country.c_str());
			
			SetItemText(itemPos, 2, getColumnAdapter(2)->print(*i).c_str());
			
			SetItemText(itemPos, 3, getColumnAdapter(3)->print(*i).c_str());
			
			if ((*i).seed)
				SetItemText(itemPos, 4, L"Seed");
			
			SetItemText(itemPos, 5, (*i).client.c_str());
			
			SetItemText(itemPos, 6, (*i).status.c_str());
		}
		
		ConditionallyDoAutoSort();
	}
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
