
#include "stdAfx.hpp"
#include "HaliteDialog.hpp"
#include "HaliteWindow.hpp"
#include <boost/format.hpp>

void HaliteDialog::setSelectedTorrent(wstring torrent)
{
	selectedTorrent = torrent;
	
	pair<float,float> tranLimit = halite::getTorrentTransferLimits(torrent);
	pair<int,int> connLimit = halite::getTorrentConnectionLimits(torrent);
	
	NoConnDown = connLimit.first;
	NoConnUp = connLimit.second;
	TranLimitDown = tranLimit.first;
	TranLimitUp = tranLimit.second;
	
	DoDataExchange(false);
	
	if(halite::isPausedTorrent(selectedTorrent))
		SetDlgItemText(BTNPAUSE,L"Resume");
	else		
		SetDlgItemText(BTNPAUSE,L"Pause");
		
	::EnableWindow(GetDlgItem(BTNPAUSE),true);
	::EnableWindow(GetDlgItem(BTNREANNOUNCE),true);
	::EnableWindow(GetDlgItem(BTNREMOVE),true);
	
	::EnableWindow(GetDlgItem(IDC_EDITTLD),true);
	::EnableWindow(GetDlgItem(IDC_EDITTLU),true);
	::EnableWindow(GetDlgItem(IDC_EDITNCD),true);
	::EnableWindow(GetDlgItem(IDC_EDITNCU),true);
}

LRESULT HaliteDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
	
	selectedTorrent = L"";
	
	NoConnDown = 0;
	NoConnUp = 0;
	TranLimitDown = 0;
	TranLimitUp = 0;	
	
	DoDataExchange(false);
	return TRUE;
}

LRESULT HaliteDialog::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	if(::IsWindow(m_hWnd)) {
		::DestroyWindow(m_hWnd);
	}
	bHandled = TRUE;
	return 0;
}

LRESULT HaliteDialog::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{	
	bHandled = FALSE;
	return 0;
}

LRESULT HaliteDialog::OnPause(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(halite::isPausedTorrent(selectedTorrent))
	{
		SetDlgItemText(BTNPAUSE,L"Pause");
		halite::resumeTorrent(selectedTorrent);
	}
	else
	{
		SetDlgItemText(BTNPAUSE,L"Resume");
		halite::pauseTorrent(selectedTorrent);
	}
	
	mainHaliteWindow->updateUI();
	return 0;	
}	
LRESULT HaliteDialog::OnReannounce(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
		
	return 0;	
}	
LRESULT HaliteDialog::OnRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	halite::removeTorrent(selectedTorrent);
		   
	LV_FINDINFO findInfo; 
	findInfo.flags = LVFI_STRING;
	findInfo.psz = const_cast<LPTSTR>(selectedTorrent.c_str());
			
	int itemPos = mainHaliteWindow->m_list.FindItem(&findInfo, -1);
	if (itemPos >= 0)
		mainHaliteWindow->m_list.DeleteItem(itemPos);		
		
	mainHaliteWindow->updateUI();	
	return 0;	
}

LRESULT HaliteDialog::OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl )
{
	DoDataExchange(true);
	
	halite::setTorrentTransferLimits(selectedTorrent,TranLimitDown,TranLimitUp);
	halite::setTorrentConnectionLimits(selectedTorrent,NoConnDown,NoConnUp);
	
	return 0;
}

void HaliteDialog::updateDialog()
{
	int itemPos = mainHaliteWindow->m_list.GetSelectionMark();
	
	if (itemPos != -1)
	{
		wchar_t filenameBuffer[256];
		format tmpStr;
		
		mainHaliteWindow->m_list.GetItemText(itemPos,0,static_cast<LPTSTR>(filenameBuffer),256);		
		halite::torrentDetails pTD = halite::getTorrentDetails(filenameBuffer);
		
		SetDlgItemText(IDC_NAME,filenameBuffer);
		SetDlgItemText(IDC_TRACKER,pTD->current_tracker.c_str());
		SetDlgItemText(IDC_STATUS,pTD->status.c_str());
		
		SetDlgItemText(IDC_AVAIL,
			(wformat(L"%1$.2f%%") 
				% (pTD->available*100)
			).str().c_str());		
	}
	else
	{
		SetDlgItemText(IDC_NAME,L"N/A");
		SetDlgItemText(IDC_TRACKER,L"N/A");
		SetDlgItemText(IDC_STATUS,L"N/A");
		SetDlgItemText(IDC_AVAIL,L"N/A");
		
		SetDlgItemText(BTNPAUSE,L"Pause");
		
		::EnableWindow(GetDlgItem(BTNPAUSE),false);
		::EnableWindow(GetDlgItem(BTNREANNOUNCE),false);
		::EnableWindow(GetDlgItem(BTNREMOVE),false);
		
		::EnableWindow(GetDlgItem(IDC_EDITTLD),false);
		::EnableWindow(GetDlgItem(IDC_EDITTLU),false);
		::EnableWindow(GetDlgItem(IDC_EDITNCD),false);
		::EnableWindow(GetDlgItem(IDC_EDITNCU),false);
	}
}