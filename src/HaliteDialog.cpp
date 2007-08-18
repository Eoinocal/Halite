
#include <algorithm>
#include <boost/format.hpp>
#include <boost/array.hpp>

#include "stdAfx.hpp"
#include "HaliteDialog.hpp"
#include "HaliteWindow.hpp"
#include "HaliteListView.hpp"
#include "halEvent.hpp"

HaliteDialog::HaliteDialog(HaliteWindow& halWindow) :
		dialogBaseClass(halWindow)
{	
//	load();
}

void HaliteDialog::selectionChanged(const string& torrent_name)
{	
#	if 0	
	pair<float, float> tranLimit(-1.0, -1.0);
	pair<int, int> connLimit(-1, -1);
	
	if (hal::bittorrent().isTorrent(torrent_name))
	{
		tranLimit = hal::bittorrent().getTorrentSpeed(torrent_name);
		connLimit = hal::bittorrent().getTorrentLimit(torrent_name);
		
		if (!hal::bittorrent().isTorrentActive(torrent_name))
			SetDlgItemText(BTNPAUSE, L"Resume");
		else		
			SetDlgItemText(BTNPAUSE, L"Pause");
		
		::EnableWindow(GetDlgItem(BTNPAUSE), true);
		::EnableWindow(GetDlgItem(BTNREANNOUNCE), true);
		::EnableWindow(GetDlgItem(BTNREMOVE), true);
		
		::EnableWindow(GetDlgItem(IDC_EDITTLD), true);
		::EnableWindow(GetDlgItem(IDC_EDITTLU), true);
		::EnableWindow(GetDlgItem(IDC_EDITNCD), true);
		::EnableWindow(GetDlgItem(IDC_EDITNCU), true);
	}
	else
	{
		SetDlgItemText(IDC_NAME, L"N/A");
		SetDlgItemText(IDC_TRACKER, L"N/A");
		SetDlgItemText(IDC_STATUS, L"N/A");
		SetDlgItemText(IDC_AVAIL, L"N/A");
		SetDlgItemText(IDC_COMPLETE, L"N/A");
		
		SetDlgItemText(BTNPAUSE, L"Pause");		
		m_prog.SetPos(0);
		
		::EnableWindow(GetDlgItem(BTNPAUSE), false);
		::EnableWindow(GetDlgItem(BTNREANNOUNCE), false);
		::EnableWindow(GetDlgItem(BTNREMOVE), false);
		
		::EnableWindow(GetDlgItem(IDC_EDITTLD), false);
		::EnableWindow(GetDlgItem(IDC_EDITTLU), false);
		::EnableWindow(GetDlgItem(IDC_EDITNCD), false);
		::EnableWindow(GetDlgItem(IDC_EDITNCU), false);
	}
	
	NoConnDown = connLimit.first;
	NoConnUp = connLimit.second;
	TranLimitDown = tranLimit.first;
	TranLimitUp = tranLimit.second;
	
	DoDataExchange(false);	
	m_list.DeleteAllItems();	
	ui().update();
#	endif
}

LRESULT HaliteDialog::onInitDialog(HWND, LPARAM)
{
	dialogBaseClass::InitializeHalDialogBase();
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
	
{	m_prog.Attach(GetDlgItem(TORRENTPROG));
	m_prog.SetRange(0, 100);
}	
	m_list.SubclassWindow(GetDlgItem(LISTPEERS));
	
	NoConnDown = -1;
	NoConnUp = -1;
	TranLimitDown = -1;
	TranLimitUp = -1;	
	
	DoDataExchange(false);
	return 0;
}

void HaliteDialog::saveStatus()
{
	m_list.saveSettings();
}

void HaliteDialog::onClose()
{
	saveStatus();
	
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

void HaliteDialog::onPause(UINT, int, HWND)
{
	string torrentName = hal::to_utf8(hal::bittorrent().torrentDetails().selectedTorrent()->filename());
	
	if (!hal::bittorrent().isTorrentActive(torrentName))
	{
		SetDlgItemText(BTNPAUSE,L"Pause");
		hal::bittorrent().resumeTorrent(torrentName);
	}
	else
	{
		SetDlgItemText(BTNPAUSE,L"Resume");
		hal::bittorrent().pauseTorrent(torrentName);
	}
	
	requestUiUpdate();	
}

void HaliteDialog::onReannounce(UINT, int, HWND)
{
//	hal::event().post(boost::shared_ptr<hal::EventDetail>(
//		new hal::EventXmlException(hal::to_wstr("I'm an exception!"), hal::to_wstr("I'm a name!")))); 

/*	hal::bittorrent().reannounceTorrent(selection_manager().selected());
*/
}

void HaliteDialog::onRemove(UINT, int, HWND)
{
	string torrentName = hal::to_utf8(hal::bittorrent().torrentDetails().selectedTorrent()->filename());

	hal::bittorrent().removeTorrent(torrentName);
	torrentsList().clearFocused();
}

LRESULT HaliteDialog::OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl)
{
	DoDataExchange(true);
	
/*	hal::bittorrent().setTorrentSpeed(selection_manager().selected(), TranLimitDown, TranLimitUp);
	hal::bittorrent().setTorrentLimit(selection_manager().selected(), NoConnDown, NoConnUp);
*/	
	return 0;
}

LRESULT HaliteDialog::OnCltColor(HDC hDC, HWND hWnd)
{	
	::SetTextColor(hDC, RGB(255, 0, 255)); 
	
	return (LRESULT)::GetCurrentObject(hDC, OBJ_BRUSH);
}

void HaliteDialog::DialogListView::uiUpdate(const hal::TorrentDetails& tD) 
{	
	TryUpdateLock<listClass> lock(*this);
	if (lock) 
	{		
		peerDetails_ = tD.selectedTorrent()->peerDetails();
		
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
		}
		
		ConditionallyDoAutoSort();
	}
}

void HaliteDialog::uiUpdate(const hal::TorrentDetails& tD) 
{	
	if (tD.selectedTorrent()) 	
	{
		SetDlgItemText(IDC_NAME, tD.selectedTorrent()->filename().c_str());
		SetDlgItemText(IDC_TRACKER, tD.selectedTorrent()->currentTracker().c_str());
		SetDlgItemText(IDC_STATUS, tD.selectedTorrent()->state().c_str());
		m_prog.SetPos(static_cast<int>(tD.selectedTorrent()->completion()*100));
		
		if (!tD.selectedTorrent()->estimatedTimeLeft().is_special())
		{
			SetDlgItemText(IDC_AVAIL,
				(hal::from_utf8(boost::posix_time::to_simple_string(
					tD.selectedTorrent()->estimatedTimeLeft())).c_str()));
		}
		else
		{
			SetDlgItemText(IDC_AVAIL,L"∞");		
		}
		
		SetDlgItemText(IDC_COMPLETE,
			(wformat(L"%1$.2fmb of %2$.2fmb") 
				% (static_cast<float>(tD.selectedTorrent()->totalWantedDone())/(1024*1024))
				% (static_cast<float>(tD.selectedTorrent()->totalWanted())/(1024*1024))
			).str().c_str());
		
		m_list.uiUpdate(tD);
	}
	else
	{
		
/*		SetDlgItemText(IDC_NAME, L"N/A");
		SetDlgItemText(IDC_TRACKER, L"N/A");
		SetDlgItemText(IDC_STATUS, L"N/A");
		SetDlgItemText(IDC_AVAIL, L"N/A");
		SetDlgItemText(IDC_COMPLETE, L"N/A");
		
		SetDlgItemText(BTNPAUSE, L"Pause");
		
		::EnableWindow(GetDlgItem(BTNPAUSE), false);
		::EnableWindow(GetDlgItem(BTNREANNOUNCE), false);
		::EnableWindow(GetDlgItem(BTNREMOVE), false);
		
		::EnableWindow(GetDlgItem(IDC_EDITTLD), false);
		::EnableWindow(GetDlgItem(IDC_EDITTLU), false);
		::EnableWindow(GetDlgItem(IDC_EDITNCD), false);
		::EnableWindow(GetDlgItem(IDC_EDITNCU), false);
		
		m_list.DeleteAllItems();
*/
	}
}

void HaliteDialog::updateDialog()
{
#	if 0

	hal::TorrentDetail_ptr pTD = hal::bittorrent().getTorrentDetail_vec(
		selection_manager().selected());
	
	if (pTD) 	
	{
		SetDlgItemText(IDC_NAME, pTD->filename().c_str());
		SetDlgItemText(IDC_TRACKER, pTD->currentTracker().c_str());
		SetDlgItemText(IDC_STATUS, pTD->state().c_str());
		m_prog.SetPos(static_cast<int>(pTD->completion()*100));
		
		if (!pTD->estimatedTimeLeft().is_special())
		{
			SetDlgItemText(IDC_AVAIL,
				(hal::from_utf8(boost::posix_time::to_simple_string(pTD->estimatedTimeLeft())).c_str()));
		}
		else
		{
			SetDlgItemText(IDC_AVAIL,L"∞");		
		}
		
		SetDlgItemText(IDC_COMPLETE,
			(wformat(L"%1$.2fmb of %2$.2fmb") 
				% (static_cast<float>(pTD->totalWantedDone())/(1024*1024))
				% (static_cast<float>(pTD->totalWanted())/(1024*1024))
			).str().c_str());
		
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
			}			
		}
	}
	else
	{
		
/*		SetDlgItemText(IDC_NAME, L"N/A");
		SetDlgItemText(IDC_TRACKER, L"N/A");
		SetDlgItemText(IDC_STATUS, L"N/A");
		SetDlgItemText(IDC_AVAIL, L"N/A");
		SetDlgItemText(IDC_COMPLETE, L"N/A");
		
		SetDlgItemText(BTNPAUSE, L"Pause");
		
		::EnableWindow(GetDlgItem(BTNPAUSE), false);
		::EnableWindow(GetDlgItem(BTNREANNOUNCE), false);
		::EnableWindow(GetDlgItem(BTNREMOVE), false);
		
		::EnableWindow(GetDlgItem(IDC_EDITTLD), false);
		::EnableWindow(GetDlgItem(IDC_EDITTLU), false);
		::EnableWindow(GetDlgItem(IDC_EDITNCD), false);
		::EnableWindow(GetDlgItem(IDC_EDITNCU), false);
		
		m_list.DeleteAllItems();
*/
	}
#	endif
}

