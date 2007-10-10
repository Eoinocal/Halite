
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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

HaliteDialog::CWindowMapStruct* HaliteDialog::GetWindowMap()
{
#define TORRENT_LIMITS_LAYOUT \
	WMB_HEAD(WMB_COL(_exp|20), WMB_COL(_exp|30), WMB_COL(_exp|20), WMB_COL(_exp|30)), \
		WMB_ROW(10,	IDC_TL,	_r, _r, _r), \
		WMB_ROW(11,	IDC_TLD, IDC_EDITTLD, IDC_TLU, IDC_EDITTLU), \
		WMB_ROW(10,	IDC_NC,	_r, _r, _r), \
		WMB_ROW(11,	IDC_NCD, IDC_EDITNCD, IDC_NCU, IDC_EDITNCU), \
	WMB_END()

#define TORRENT_STATUS_LAYOUT \
	WMB_HEAD(WMB_COL(45), WMB_COLNOMIN(_exp|100), WMB_COL(_eq|0), WMB_COL(_exp|200)), \
		WMB_ROW(_auto,	IDC_NAME_STATIC, IDC_NAME, _r, _r), \
		WMB_ROW(_auto,	IDC_TRACKER_STATIC, IDC_TRACKER, _r, _r), \
		WMB_ROW(_auto,	IDC_STATUS_STATIC, IDC_STATUS, _r, _r), \
		WMB_ROW(_auto,	IDC_TIME_STATIC, IDC_AVAIL, IDC_COMPLETED_STATIC, IDC_COMPLETE), \
	WMB_END()
	
#define TORRENT_BUTTON_LAYOUT \
	WMB_HEAD(WMB_COL(_exp)), \
		WMB_ROW(_gap), \
		WMB_ROWMINNOMAX(_exp, 13, BTNPAUSE), \
		WMB_ROWMINNOMAX(_exp, 13, BTNREANNOUNCE), \
		WMB_ROWMINNOMAX(_exp, 13, BTNREMOVE), \
	WMB_END()	

	BEGIN_WINDOW_MAP_INLINE(HaliteDialog, 6, 6, 3, 3)
		WMB_HEAD(WMB_COL(_gap), WMB_COL(_exp), WMB_COL(120), WMB_COL(60), WMB_COL(_gap)), 
			WMB_ROW(_gap,	IDC_DETAILS_GROUP, _r, _r, _r, _r), 
			WMB_ROW(_auto,	_d, TORRENT_STATUS_LAYOUT, TORRENT_LIMITS_LAYOUT, TORRENT_BUTTON_LAYOUT), 
			WMB_ROWMIN(_auto, 8, _d, TORRENTPROG, _r, _r), 
			WMB_ROW(_gap, _d), 
			WMB_ROWNOMAX(_exp, _d, LISTPEERS, _r, _r), 
			WMB_ROW(_gap,	_d), 
		WMB_END() 
	END_WINDOW_MAP_INLINE()	
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

void HaliteDialog::focusChanged(const hal::TorrentDetail_ptr pT)
{
	pair<float, float> tranLimit(-1.0, -1.0);
	pair<int, int> connLimit(-1, -1);
	
	if (pT)
	{
		tranLimit = hal::bittorrent().getTorrentSpeed(pT->name());
		connLimit = hal::bittorrent().getTorrentLimit(pT->name());
		
		if (!hal::bittorrent().isTorrentActive(pT->name()))
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
		string torrent_name = hal::to_utf8(torrent->name());
		
		SetDlgItemText(IDC_NAME, torrent->name().c_str());
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
}
