
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
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
	
{	m_prog.Attach(GetDlgItem(TORRENTPROG));
	m_prog.SetRange(0, 100);
}	
	
	NoConnDown = -1;
	NoConnUp = -1;
	TranLimitDown = -1;
	TranLimitUp = -1;	
	
	DoDataExchange(false);
	return 0;
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

void AdvTorrentDialog::focusChanged(string& torrent_name) 
{
	pair<float, float> tranLimit(-1.0, -1.0);
	pair<int, int> connLimit(-1, -1);
	float ratio = 0;

	if (hal::bittorrent().isTorrent(torrent_name))
	{
		tranLimit = hal::bittorrent().getTorrentSpeed(torrent_name);
		connLimit = hal::bittorrent().getTorrentLimit(torrent_name);
		
		ratio = hal::bittorrent().getTorrentRatio(torrent_name);
		
		::EnableWindow(GetDlgItem(IDC_EDITTLD), true);
		::EnableWindow(GetDlgItem(IDC_EDITTLU), true);
		::EnableWindow(GetDlgItem(IDC_EDITNCD), true);
		::EnableWindow(GetDlgItem(IDC_EDITNCU), true);
		::EnableWindow(GetDlgItem(IDC_EDITRATIO), true);
	}
	else
	{
		SetDlgItemText(IDC_NAME_STATUS, L"N/A");
	//	SetDlgItemText(IDC_SECOND, L"N/A");
		SetDlgItemText(IDC_TRANSFERED, L"N/A");
		SetDlgItemText(IDC_REMAINING, L"N/A");
		SetDlgItemText(IDC_RATE, L"N/A");
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
	if (hal::TorrentDetail_ptr torrent = tD.selectedTorrent()) 	
	{	
		string torrent_name = hal::to_utf8(torrent->filename());
		
		if (current_torrent_name_ != torrent_name)
		{	
			current_torrent_name_ = torrent_name;
			focusChanged(current_torrent_name_);
		}
		
		uiUpdateSingle(tD.selectedTorrent());	
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
