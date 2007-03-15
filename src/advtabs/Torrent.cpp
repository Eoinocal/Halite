
#include <algorithm>
#include <boost/format.hpp>
#include <boost/array.hpp>

#include "../stdAfx.hpp"
#include "../HaliteWindow.hpp"
#include "../HaliteListViewCtrl.hpp"

#include "Torrent.hpp"

AdvTorrentDialog::AdvTorrentDialog(ui_signal& ui_sig, ListViewManager& single_sel) :
	ui_(ui_sig),
	selection_manager_(single_sel)
{
}

void AdvTorrentDialog::selectionChanged(const string& torrent_name)
{	
	pair<float, float> tranLimit(-1.0, -1.0);
	pair<int, int> connLimit(-1, -1);
	float ratio = 0;
	
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
	
	DoDataExchange(false);	
	ui_.update();
}

LRESULT AdvTorrentDialog::onInitDialog(HWND, LPARAM)
{
	ui_.attach(bind(&AdvTorrentDialog::updateDialog, this));
	selection_manager_.attach(bind(&AdvTorrentDialog::selectionChanged, this, _1));
	
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
	
	hal::bittorrent().setTorrentSpeed(selection_manager_.selected(), TranLimitDown, TranLimitUp);
	hal::bittorrent().setTorrentLimit(selection_manager_.selected(), NoConnDown, NoConnUp);
	hal::bittorrent().setTorrentRatio(selection_manager_.selected(), Ratio);
	
	return 0;
}

void AdvTorrentDialog::updateDialog()
{
	hal::TorrentDetail_ptr pTD = hal::bittorrent().getTorrentDetails(
		selection_manager_.selected());
	
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
}
