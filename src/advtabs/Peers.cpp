
#include "../stdAfx.hpp"
#include "../HaliteWindow.hpp"
#include "../HaliteListView.hpp"

#include "../global/logger.hpp"

#include "Peers.hpp"

#if 0

void AdvPeerDialog::updateDialog()
{
	hal::PeerDetails peerDetails;
	hal::bittorrent().getAllPeerDetails(selection_manager().selected(), peerDetails);
	
	if (!peerDetails.empty())
	{
		// Here we remove any peers no longer connected.
		
		std::sort(peerDetails.begin(), peerDetails.end());
		
		for(int i = 0; i < m_list.GetItemCount(); /*nothing here*/)
		{
			boost::array<wchar_t, MAX_PATH> ip_address;
			m_list.GetItemText(i, 0, ip_address.c_array(), MAX_PATH);
			
			hal::PeerDetail ip(ip_address.data());
			hal::PeerDetails::iterator iter = 
				std::lower_bound(peerDetails.begin(), peerDetails.end(), ip);
			
			if (iter == peerDetails.end() || !((*iter) == ip))
				m_list.DeleteItem(i);
			else
				++i;
		}
		
		// And now here we add/update the connected peers
		
		for (hal::PeerDetails::iterator i = peerDetails.begin(); 
			i != peerDetails.end(); ++i)
		{			
			LV_FINDINFO findInfo; 
			findInfo.flags = LVFI_STRING;
			findInfo.psz = const_cast<LPTSTR>((*i).ipAddress.c_str());
			
			int itemPos = m_list.FindItem(&findInfo, -1);
			if (itemPos < 0)
				itemPos = m_list.AddItem(0, 0, (*i).ipAddress.c_str(), 0);
			
			m_list.SetItemText(itemPos, 1,
				(wformat(L"%1$.2fKB/s") 
					% ((*i).speed.first/1024)
				).str().c_str());	
			
			m_list.SetItemText(itemPos, 2,
				(wformat(L"%1$.2fKB/s") 
					% ((*i).speed.second/1024)
				).str().c_str());	
			
			if ((*i).seed)
				m_list.SetItemText(itemPos, 3, L"Seed");
			
			m_list.SetItemText(itemPos, 4, (*i).client.c_str());
			
			m_list.SetItemText(itemPos, 5, (*i).status.c_str());
		}			
	}
	else
	{
		m_list.DeleteAllItems();
	}
}

#endif

void PeerListView::uiUpdate(const hal::TorrentDetails& tD)
{
	TryUpdateLock<listClass> lock(*this);
	if (lock) 
	{		
		peerDetails_.clear();
		
		foreach (const hal::TorrentDetail_ptr torrent, tD.selectedTorrents())
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

void AdvPeerDialog::uiUpdate(const hal::TorrentDetails& tD)
{
	peerList_.uiUpdate(tD);
}

LRESULT AdvPeerDialog::onInitDialog(HWND, LPARAM)
{	
	peerList_.Attach(GetDlgItem(IDC_PEERLIST));
	
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);	
	
	return 0;
}

void AdvPeerDialog::onClose()
{
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

