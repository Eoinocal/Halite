
#include "../stdAfx.hpp"
#include "../HaliteWindow.hpp"
#include "../HaliteListView.hpp"

#include "../global/logger.hpp"

#include "Peers.hpp"

void AdvPeerDialog::selectionChanged(const string& torrent_name)
{	
	AdvPeerDialog::updateDialog();
}

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

LRESULT AdvPeerDialog::onInitDialog(HWND, LPARAM)
{
	dialogBaseClass::InitializeHalDialogBase();	
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);	
	m_list.Attach(GetDlgItem(IDC_PEERLIST));
	
	return 0;
}

void AdvPeerDialog::onClose()
{
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

