
#include <algorithm>
#include <boost/format.hpp>
#include <boost/array.hpp>

#include "../stdAfx.hpp"
#include "../HaliteWindow.hpp"
#include "../HaliteListViewCtrl.hpp"

#include "Torrent.hpp"

void AdvTorrentDialog::selectionChanged(const string& torrent_name)
{	
	pair<float, float> tranLimit(-1.0, -1.0);
	pair<int, int> connLimit(-1, -1);
	float ratio = 0;

#	if 0	
	if (hal::bittorrent().isTorrent(torrent_name))
	{
		tranLimit = hal::bittorrent().getTorrentSpeed(torrent_name);
		connLimit = hal::bittorrent().getTorrentLimit(torrent_name);
		
		ratio = hal::bittorrent().getTorrentRatio(torrent_name);
		
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
		::EnableWindow(GetDlgItem(IDC_EDITRATIO), true);
	}
	else
	{
		SetDlgItemText(IDC_NAME, L"N/A");
		SetDlgItemText(IDC_TRACKER, L"N/A");
		SetDlgItemText(IDC_STATUS, L"N/A");
		SetDlgItemText(IDC_AVAIL, L"N/A");
		SetDlgItemText(IDC_TRANS, L"N/A");
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
		::EnableWindow(GetDlgItem(IDC_EDITRATIO), false);
	}
	
	NoConnDown = connLimit.first;
	NoConnUp = connLimit.second;
	TranLimitDown = tranLimit.first;
	TranLimitUp = tranLimit.second;
	Ratio = ratio;
#	endif	

	DoDataExchange(false);	
//	ui().update();
}

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
	
/*	hal::bittorrent().setTorrentSpeed(selection_manager().selected(), TranLimitDown, TranLimitUp);
	hal::bittorrent().setTorrentLimit(selection_manager().selected(), NoConnDown, NoConnUp);
	hal::bittorrent().setTorrentRatio(selection_manager().selected(), Ratio);
	*/
	return 0;
}

void AdvTorrentDialog::uiUpdate(const hal::TorrentDetails& tD)
{
	uiUpdateSingle(tD.selectedTorrent());
}

void AdvTorrentDialog::uiUpdateMultiple(const hal::TorrentDetail_vec& torrents)
{
}

void AdvTorrentDialog::uiUpdateSingle(const hal::TorrentDetail_ptr& torrent)
{	
	if (torrent) 	
	{
		SetDlgItemText(IDC_NAME, torrent->filename().c_str());
		SetDlgItemText(IDC_TRACKER, torrent->currentTracker().c_str());
		SetDlgItemText(IDC_STATUS, torrent->state().c_str());
		m_prog.SetPos(static_cast<int>(torrent->completion()*100));	

		wstring eta = L"∞";	
		
		if (!torrent->estimatedTimeLeft().is_special())
		{
			eta = hal::from_utf8(boost::posix_time::to_simple_string(torrent->estimatedTimeLeft()));
		}
		
		SetDlgItemText(IDC_TRANS_ETA,
			(wformat(hal::app().res_wstr(HAL_COMPLETED_SUMMARY)) 
				% (static_cast<float>(torrent->totalWantedDone())/(1024*1024))
				% (static_cast<float>(torrent->totalWanted())/(1024*1024))
				% eta
			).str().c_str());
			
		float ratio = (torrent->totalDownloaded()) 
			? static_cast<float>(torrent->totalUploaded())
				/ static_cast<float>(torrent->totalDownloaded())
			: 0;
		
		SetDlgItemInfo(IDC_TRANS,
			wformat(hal::app().res_wstr(HAL_DOWNLOADT_SUMMARY)) 
				% (static_cast<float>(torrent->totalDownloaded())/(1024*1024))
				% (static_cast<float>(torrent->totalUploaded())/(1024*1024))
				% ratio);	
			
		if (!torrent->updateTrackerIn().is_special())
		{
			SetDlgItemText(IDC_UPDATE,	
				(hal::from_utf8(boost::posix_time::to_simple_string(torrent->updateTrackerIn())).c_str()));
		}
		else SetDlgItemText(IDC_UPDATE,	L"N/A");	
	}
}

void AdvTorrentDialog::uiUpdateNone()
{
}

void AdvTorrentDialog::updateDialog()
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
			SetDlgItemText(IDC_ETA,
				(hal::from_utf8(boost::posix_time::to_simple_string(pTD->estimatedTimeLeft())).c_str()));
		}
		else SetDlgItemText(IDC_ETA,L"∞");
		
/*		SetDlgItemText(IDC_COMPLETE,
			(wformat(L"%1$.2fmb of %2$.2fmb") 
				% (static_cast<float>(pTD->totalWantedDone())/(1024*1024))
				% (static_cast<float>(pTD->totalWanted())/(1024*1024))
			).str().c_str());
*/			
		float ratio = (pTD->totalWantedDone()) 
			? static_cast<float>(pTD->totalUploaded())
				/ static_cast<float>(pTD->totalWantedDone())
			: 0;
		
		SetDlgItemText(IDC_TRANS,
			(wformat(hal::app().res_wstr(HAL_DOWNLOAD_SUMMARY)) 
				% (static_cast<float>(pTD->totalWantedDone())/(1024*1024))
				% (static_cast<float>(pTD->totalUploaded())/(1024*1024))
				% (ratio)
			).str().c_str());	
			
		if (!pTD->updateTrackerIn().is_special())
		{
			SetDlgItemText(IDC_UPDATE,	
				(hal::from_utf8(boost::posix_time::to_simple_string(pTD->updateTrackerIn())).c_str()));
		}
		else SetDlgItemText(IDC_UPDATE,	L"N/A");		
	}
#	endif
}
