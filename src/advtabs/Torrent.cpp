
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <boost/format.hpp>
#include <boost/array.hpp>

#include "../stdAfx.hpp"
#include "../HaliteWindow.hpp"
#include "../HaliteListViewCtrl.hpp"

#include "Torrent.hpp"

LRESULT AdvTorrentDialog::onInitDialog(HWND, LPARAM)
{
	dialogBaseClass::InitializeHalDialogBase();	
	
	m_prog.Attach(GetDlgItem(TORRENTPROG));
	m_prog.SetRange(0, 100);	
	
	NoConnDown = -1;
	NoConnUp = -1;
	TranLimitDown = -1;
	TranLimitUp = -1;	
	
	DoDataExchange(false);
	return 0;
}

AdvTorrentDialog::CWindowMapStruct* AdvTorrentDialog::GetWindowMap()
{
	#define TORRENT_LIMITS_LAYOUT \
	WMB_HEAD(WMB_COL(_exp|20), WMB_COL(_exp|30), WMB_COL(_exp|20), WMB_COL(_exp|30)), \
		WMB_ROW(10,	IDC_TL,	_r, _r, _r), \
		WMB_ROW(11,	IDC_TLD, IDC_EDITTLD, IDC_TLU, IDC_EDITTLU), \
		WMB_ROW(10,	IDC_NC,	_r, _r, _r), \
		WMB_ROW(11,	IDC_NCD, IDC_EDITNCD, IDC_NCU, IDC_EDITNCU), \
		WMB_ROW(11,	IDC_RATIOESTATIC, _r, _r, IDC_EDITRATIO), \
	WMB_END()

#define TORRENT_STATUS_LAYOUT \
	WMB_HEAD(WMB_COL(45), WMB_COLNOMIN(_exp|150), WMB_COL(_eq|0), WMB_COL(_exp|100)), \
		WMB_ROW(10,	IDC_NAME_STATUS_LABEL, IDC_NAME_STATUS, _r, _r), \
		WMB_ROW(10,	IDC_PEERS_LABEL, IDC_PEERS, IDC_SEEDS_LABEL, IDC_SEEDS), \
		WMB_ROW(10,	IDC_TRANSFERED_LABEL, IDC_TRANSFERED, IDC_OVERHEAD_LABEL, IDC_OVERHEAD), \
		WMB_ROW(10,	IDC_REMAINING_LABEL, IDC_REMAINING, IDC_ETA_LABEL, IDC_ETA), \
		WMB_ROW(10,	IDC_RATE_LABEL, IDC_RATE, IDC_RATIO_LABEL, IDC_RATIO), \
	WMB_END()
		
#define TORRENT_REANNOUNCE_LAYOUT \
	WMB_HEAD(WMB_COL(50), WMB_COLNOMIN(_exp)), \
		WMB_ROW(10,	IDC_UPDATESTAT, IDC_UPDATE), \
	WMB_END()	

	BEGIN_WINDOW_MAP_INLINE(AdvTorrentDialog, 6, 6, 3, 3)
		WMB_HEAD(WMB_COL(_gap), WMB_COL(_exp), WMB_COL(120), WMB_COL(_gap)), 
			WMB_ROW(_gap|3,	IDC_GROUP_TORRENT, _r, _r, _r), 
			WMB_ROW(_auto,	_d, TORRENT_STATUS_LAYOUT, TORRENT_LIMITS_LAYOUT), 
			WMB_ROWMIN(_exp, 8,	_d, TORRENTPROG, _r), 
			WMB_ROW(_gap,	_d), 
			WMB_ROW(_gap|3,	IDC_GROUP_TRACKER, _r, _r, _r), 
			WMB_ROW(_auto,	_d, IDC_TRACKER, TORRENT_REANNOUNCE_LAYOUT), 
			WMB_ROW(_gap,	_d), 
		WMB_END() 
	END_WINDOW_MAP_INLINE()	
}
	
void AdvTorrentDialog::onClose()
{
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

LRESULT AdvTorrentDialog::OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl)
{
	DoDataExchange(true);
	
	if (hal::TorrentDetail_ptr torrent = hal::bittorrent().torrentDetails().selectedTorrent()) 
	{
		string torrentName = hal::to_utf8(torrent->filename());
		
		hal::bittorrent().setTorrentSpeed(torrentName, TranLimitDown, TranLimitUp);
		hal::bittorrent().setTorrentLimit(torrentName, NoConnDown, NoConnUp);
		hal::bittorrent().setTorrentRatio(torrentName, Ratio);
	}
	
	return 0;
}

void AdvTorrentDialog::focusChanged(const hal::TorrentDetail_ptr pT)
{
	pair<float, float> tranLimit(-1.0, -1.0);
	pair<int, int> connLimit(-1, -1);
	float ratio = 0;

	if (pT)
	{
		tranLimit = hal::bittorrent().getTorrentSpeed(pT->name());
		connLimit = hal::bittorrent().getTorrentLimit(pT->name());
		
		ratio = hal::bittorrent().getTorrentRatio(pT->name());
		
		::EnableWindow(GetDlgItem(IDC_EDITTLD), true);
		::EnableWindow(GetDlgItem(IDC_EDITTLU), true);
		::EnableWindow(GetDlgItem(IDC_EDITNCD), true);
		::EnableWindow(GetDlgItem(IDC_EDITNCU), true);
		::EnableWindow(GetDlgItem(IDC_EDITRATIO), true);
	}
	else
	{
		SetDlgItemText(IDC_NAME_STATUS, L"N/A");
		SetDlgItemText(IDC_PEERS, L"N/A");
		SetDlgItemText(IDC_SEEDS, L"N/A");
		SetDlgItemText(IDC_TRANSFERED, L"N/A");
		SetDlgItemText(IDC_OVERHEAD, L"N/A");
		SetDlgItemText(IDC_REMAINING, L"N/A");
		SetDlgItemText(IDC_ETA, L"N/A");
		SetDlgItemText(IDC_RATE, L"N/A");
		SetDlgItemText(IDC_RATIO, L"N/A");
		SetDlgItemText(IDC_TRACKER, L"N/A");
		SetDlgItemText(IDC_UPDATE, L"N/A");
		
		m_prog.SetPos(0);
		
		::EnableWindow(GetDlgItem(IDC_EDITTLD), false);
		::EnableWindow(GetDlgItem(IDC_EDITTLU), false);
		::EnableWindow(GetDlgItem(IDC_EDITNCD), false);
		::EnableWindow(GetDlgItem(IDC_EDITNCU), false);
		::EnableWindow(GetDlgItem(IDC_EDITRATIO), false);
	}
	
	NoConnDown = connLimit.first;
	NoConnUp = connLimit.second;
	TranLimitDown = tranLimit.first;
	TranLimitUp = tranLimit.second;
	Ratio = ratio;

	DoDataExchange(false);	
}

void AdvTorrentDialog::uiUpdate(const hal::TorrentDetails& tD)
{	
	if (hal::TorrentDetail_ptr torrent = tD.focusedTorrent()) 	
	{			
		uiUpdateSingle(torrent);	
	}
}

void AdvTorrentDialog::uiUpdateSingle(const hal::TorrentDetail_ptr& torrent)
{	
	if (torrent) 	
	{
/*		HAL_NAME_STATUS		  "Name: %1%, %2%."
		HAL_SECOND			  "Peers %1% (%2%).		Seeds %3% (%4%)."
		HAL_TRANSFERED		  "Transfered (Overhead): %1$.2fMB (%2$.2fMB) Down, %3$.2fMB (%4$.2fMB) Up."
		HAL_REMAINING		  "Remaining: %1$.2fMB of %2$.2fMB, ETA %3%."
		HAL_RATE			  "Downloading at %1$.2fkb/s, Uploading at %2$.2fkb/s, Ratio %3$.2f."
*/	
		SetDlgItemInfo(IDC_NAME_STATUS, 
			wformat(hal::app().res_wstr(HAL_NAME_STATUS)) 
				% torrent->filename()
				% torrent->state());

		SetDlgItemInfo(IDC_PEERS,
			wformat(L"%1% (%2%)")
				% torrent->peersConnected()
				% torrent->peers());

		SetDlgItemInfo(IDC_SEEDS,
			wformat(L"%1% (%2%)")
				% torrent->seedsConnected()
				% torrent->seeds());

		SetDlgItemInfo(IDC_TRANSFERED,
			wformat(hal::app().res_wstr(HAL_TRANSFERED)) 
				% (static_cast<float>(torrent->totalPayloadDownloaded())/(1024*1024))
				% (static_cast<float>(torrent->totalPayloadUploaded())/(1024*1024)));

		SetDlgItemInfo(IDC_OVERHEAD,
			wformat(L"%1$.2fMB - %2$.2fMB") 
				% (static_cast<float>(torrent->totalDownloaded() - torrent->totalPayloadDownloaded())/(1024*1024))
				% (static_cast<float>(torrent->totalUploaded() - torrent->totalPayloadUploaded())/(1024*1024)));

		SetDlgItemInfo(IDC_REMAINING,
			wformat(hal::app().res_wstr(HAL_REMAINING))
				% (static_cast<float>(torrent->totalWanted()-torrent->totalWantedDone())/(1024*1024))
				% (static_cast<float>(torrent->totalWanted())/(1024*1024)));
		
		wstring eta = L"∞";			
		if (!torrent->estimatedTimeLeft().is_special())
			eta = hal::from_utf8(boost::posix_time::to_simple_string(torrent->estimatedTimeLeft()));
		
		SetDlgItemInfo(IDC_ETA, eta);
			
		SetDlgItemInfo(IDC_RATE,
			wformat(hal::app().res_wstr(HAL_RATE))
				% (torrent->speed().first/1024)
				% (torrent->speed().second/1024));
				
		float ratio = (torrent->totalPayloadDownloaded()) 
			? static_cast<float>(torrent->totalPayloadUploaded())
				/ static_cast<float>(torrent->totalPayloadDownloaded())
			: 0;
		
		SetDlgItemInfo(IDC_RATIO, 
			wformat(L"%1$.2f") % ratio);		
		
		m_prog.SetPos(static_cast<int>(torrent->completion()*100));	
		
		SetDlgItemText(IDC_TRACKER, torrent->currentTracker().c_str());
		
		if (!torrent->updateTrackerIn().is_special())
		{
			SetDlgItemText(IDC_UPDATE,	
				(hal::from_utf8(boost::posix_time::to_simple_string(torrent->updateTrackerIn())).c_str()));
		}
		else SetDlgItemText(IDC_UPDATE,	L"N/A");		
	}
}

void AdvTorrentDialog::uiUpdateMultiple(const hal::TorrentDetail_vec& torrents)
{}

void AdvTorrentDialog::uiUpdateNone()
{}
