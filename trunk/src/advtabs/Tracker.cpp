
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "../stdAfx.hpp"
#include "../HaliteWindow.hpp"
#include "../HaliteListView.hpp"
#include "../global/logger.hpp"

#include "Tracker.hpp"

LRESULT AdvTrackerDialog::onInitDialog(HWND, LPARAM)
{
	dialogBaseClass::InitializeHalDialogBase();	
	
	m_list.Attach(GetDlgItem(IDC_TRACKERLIST));	
	m_list.attachEditedConnection(bind(&AdvTrackerDialog::trackerListEdited, this));

	::EnableWindow(GetDlgItem(IDC_TRACKER_LOGINCHECK), false);
	::EnableWindow(GetDlgItem(IDC_TRACKERLIST), false);
	
	username_ = L"";
	password_ = L"";
		
	DoDataExchange(false);	
	return 0;
}

void AdvTrackerDialog::setLoginUiState(const string& torrent_name)
{
	if (username_ == L"")
	{
		::SendMessage(GetDlgItem(IDC_TRACKER_LOGINCHECK), BM_SETCHECK, BST_UNCHECKED, 0);
		password_ = L"";
	}
	else
	{	
		::SendMessage(GetDlgItem(IDC_TRACKER_LOGINCHECK), BM_SETCHECK, BST_CHECKED, 0);
	}
	
	onLoginCheck(0, 0, GetDlgItem(IDC_TRACKER_LOGINCHECK));	
}

void AdvTrackerDialog::onClose()
{
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

LRESULT AdvTrackerDialog::OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl)
{
	DoDataExchange(true);
	
	string torrent_name;	
	if (hal::bittorrent().torrentDetails().focusedTorrent())
		torrent_name = hal::to_utf8(hal::bittorrent().torrentDetails().focusedTorrent()->name());
	
	setLoginUiState(torrent_name);
	hal::bittorrent().setTorrentLogin(torrent_name, username_, password_);
	
	return 0;
}

void AdvTrackerDialog::focusChanged(const hal::TorrentDetail_ptr pT)
{		
	if (pT)
	{		
		::EnableWindow(GetDlgItem(IDC_TRACKER_LOGINCHECK), true);
		::EnableWindow(GetDlgItem(IDC_TRACKERLIST), true);
		
		std::pair<wstring, wstring> details = 
			hal::bittorrent().getTorrentLogin(pT->name());
		
		username_ = details.first;
		password_ = details.second;
	}
	else
	{				
		::EnableWindow(GetDlgItem(IDC_TRACKER_LOGINCHECK), false);
		::EnableWindow(GetDlgItem(IDC_TRACKERLIST), false);
		
		username_ = L"";
		password_ = L"";
	}
	
	m_list.uiUpdate(pT);
			
	::EnableWindow(GetDlgItem(IDC_TRACKER_APPLY), false);	
	setLoginUiState(current_torrent_name_);

	DoDataExchange(false);
}

void AdvTrackerDialog::uiUpdate(const hal::TorrentDetails& tD)
{}

AdvTrackerDialog::CWindowMapStruct* AdvTrackerDialog::GetWindowMap()
{
#define TRACKER_LOGIN_LAYOUT \
	WMB_HEAD(WMB_COL(_exp|50), WMB_COL(_exp|66), WMB_COL(_exp|33)), \
		WMB_ROW(_auto,	IDC_TRACKER_LOGINCHECK,	_r,	_r), \
		WMB_ROW(_auto,	IDC_TRACKER_USER_S, IDC_TRACKER_USER, _r), \
		WMB_ROW(_auto,	IDC_TRACKER_PASS_S,	IDC_TRACKER_PASS, _r), \
		WMB_ROW(_auto,	_, BTNREANNOUNCE), \
	WMB_END()

#define TRACKER_LIST_LAYOUT \
	WMB_HEAD(WMB_COL(_exp), WMB_COLNOMIN(_exp|25), WMB_COL(_exp|25)), \
		WMB_ROW(_auto,	IDC_TRACKER_LABEL, IDC_TRACKER_RESET, IDC_TRACKER_APPLY), \
		WMB_ROW(_exp,	IDC_TRACKERLIST, _r, _r), \
	WMB_END()

	BEGIN_WINDOW_MAP_INLINE(AdvTrackerDialog, 6, 6, 3, 3)
		WMB_HEAD(WMB_COL(_exp|160), WMB_COL(_exp)),
			WMB_ROWNOMINNOMAX(_exp, TRACKER_LIST_LAYOUT, TRACKER_LOGIN_LAYOUT),
		WMB_END() 
	END_WINDOW_MAP_INLINE()	
}

void AdvTrackerDialog::onLoginCheck(UINT, int, HWND hWnd)
{
	LRESULT result = ::SendMessage(hWnd, BM_GETCHECK, 0, 0);
	
	if (result == BST_CHECKED)
	{
		::EnableWindow(GetDlgItem(IDC_TRACKER_USER), true);
		::EnableWindow(GetDlgItem(IDC_TRACKER_PASS), true);
	}
	else
	{
		::EnableWindow(GetDlgItem(IDC_TRACKER_USER), false);
		::EnableWindow(GetDlgItem(IDC_TRACKER_PASS), false);	

		username_ = L"";	
		password_ = L"";
		
		if (hal::bittorrent().torrentDetails().focusedTorrent())
			hal::bittorrent().setTorrentLogin(
				hal::to_utf8(hal::bittorrent().torrentDetails().focusedTorrent()->name()),
				username_, password_);
		
		DoDataExchange(false);		
	}
}

void AdvTrackerDialog::onReannounce(UINT, int, HWND)
{
	if (hal::bittorrent().torrentDetails().focusedTorrent())
		hal::bittorrent().reannounceTorrent(hal::to_utf8(hal::bittorrent().torrentDetails().focusedTorrent()->name()));
}

void AdvTrackerDialog::trackerListEdited()
{
	::EnableWindow(GetDlgItem(IDC_TRACKER_APPLY), true);
}

void AdvTrackerDialog::onReset(UINT, int, HWND)
{
	string torrent_name;	
	if (hal::bittorrent().torrentDetails().focusedTorrent())
		torrent_name = hal::to_utf8(hal::bittorrent().torrentDetails().focusedTorrent()->name());
		
	hal::bittorrent().resetTorrentTrackers(torrent_name);
	
	std::vector<hal::TrackerDetail> trackers =
		hal::bittorrent().getTorrentTrackers(torrent_name);
	m_list.clearAll();
	
	foreach (const hal::TrackerDetail& tracker, trackers)
	{
		int itemPos = m_list.AddItem(0, 0, tracker.url.c_str(), 0);
		m_list.SetItemText(itemPos, 1, lexical_cast<wstring>(tracker.tier).c_str());
	}

	::EnableWindow(GetDlgItem(IDC_TRACKER_APPLY), false);
}

void AdvTrackerDialog::onApply(UINT, int, HWND)
{
	int total = m_list.GetItemCount();
	std::vector<hal::TrackerDetail> trackers;
	
	for (int i=0; i<total; ++i)
	{
		array<wchar_t, MAX_PATH> buffer;		
		
		m_list.GetItemText(i, 0, buffer.elems, buffer.size());
		trackers.push_back(hal::TrackerDetail(wstring(buffer.elems), 0));
		
		m_list.GetItemText(i, 1, buffer.elems, buffer.size());
		trackers.back().tier = lexical_cast<int>(wstring(buffer.elems));
	}
	
	std::sort(trackers.begin(), trackers.end());
		
	if (hal::bittorrent().torrentDetails().focusedTorrent())
		hal::bittorrent().setTorrentTrackers(hal::to_utf8(hal::bittorrent().torrentDetails().focusedTorrent()->name()), trackers);
	
	::EnableWindow(GetDlgItem(IDC_TRACKER_APPLY), false);
}
