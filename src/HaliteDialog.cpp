
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
{}

LRESULT HaliteDialog::OnInitDialog(HWND, LPARAM)
{
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
	
	m_prog.Attach(GetDlgItem(TORRENTPROG));
	m_prog.SetRange(0, 100);
	
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

void HaliteDialog::OnClose()
{
	saveStatus();
	
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

void HaliteDialog::OnPause(UINT, int, HWND)
{
	if (hal::TorrentDetail_ptr torrent = hal::bittorrent().torrentDetails().selectedTorrent()) 
	{
		string torrentName = hal::to_utf8(torrent->filename());
		
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
}

void HaliteDialog::OnReannounce(UINT, int, HWND)
{
	if (hal::TorrentDetail_ptr torrent = hal::bittorrent().torrentDetails().selectedTorrent()) 
		hal::bittorrent().reannounceTorrent(hal::to_utf8(torrent->filename()));
}

void HaliteDialog::OnRemove(UINT, int, HWND)
{
	if (hal::TorrentDetail_ptr torrent = hal::bittorrent().torrentDetails().selectedTorrent()) 
	{
		string torrentName = hal::to_utf8(torrent->filename());

		hal::bittorrent().removeTorrent(torrentName);
		torrentsList().clearFocused();
	}
}

LRESULT HaliteDialog::OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl)
{
	DoDataExchange(true);
	
	if (hal::TorrentDetail_ptr torrent = hal::bittorrent().torrentDetails().selectedTorrent()) 
	{
		string torrentName = hal::to_utf8(torrent->filename());
		
		hal::bittorrent().setTorrentSpeed(torrentName, TranLimitDown, TranLimitUp);
		hal::bittorrent().setTorrentLimit(torrentName, NoConnDown, NoConnUp);
	}
	
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

void HaliteDialog::focusChanged(string& torrent_name) 
{
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
		
	m_list.clearAll();
		
	DoDataExchange(false);	
}

void HaliteDialog::uiUpdate(const hal::TorrentDetails& tD) 
{	
	pair<float, float> tranLimit(-1.0, -1.0);
	pair<int, int> connLimit(-1, -1);
	
	if (hal::TorrentDetail_ptr torrent = tD.selectedTorrent()) 	
	{	
		string torrent_name = hal::to_utf8(torrent->filename());
		
		if (current_torrent_name_ != torrent_name)
		{	
			current_torrent_name_ = torrent_name;
			focusChanged(current_torrent_name_);
		}	
		
		SetDlgItemText(IDC_NAME, torrent->filename().c_str());
		SetDlgItemText(IDC_TRACKER, torrent->currentTracker().c_str());
		SetDlgItemText(IDC_STATUS, torrent->state().c_str());
		m_prog.SetPos(static_cast<int>(torrent->completion()*100));
		
		if (!torrent->estimatedTimeLeft().is_special())
		{
			SetDlgItemText(IDC_AVAIL,
				(hal::from_utf8(boost::posix_time::to_simple_string(
					torrent->estimatedTimeLeft())).c_str()));
		}
		else
		{
			SetDlgItemText(IDC_AVAIL,L"∞");		
		}
		
		SetDlgItemText(IDC_COMPLETE,
			(wformat(L"%1$.2fmb of %2$.2fmb") 
				% (static_cast<float>(torrent->totalWantedDone())/(1024*1024))
				% (static_cast<float>(torrent->totalWanted())/(1024*1024))
			).str().c_str());
		
		m_list.uiUpdate(tD);
	}
	else
	{	
		if (current_torrent_name_ != "")
		{	
			current_torrent_name_ = "";
			focusChanged(current_torrent_name_);
		}	
	}
}
