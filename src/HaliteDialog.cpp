
#include "WTL.hpp"
#include "HaliteDialog.hpp"

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
	return 0;	
}	
LRESULT HaliteDialog::OnReannounce(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
		
	return 0;	
}	
LRESULT HaliteDialog::OnRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{

	halite::removeTorrent(selectedTorrent);
	
	/* This is why the messy hack was necessary, some method 
	   was needed to remove the torrent from the main ListView
	   and tell the UI to update */
	   
	LV_FINDINFO findInfo; 
	findInfo.flags = LVFI_STRING;
	findInfo.psz = const_cast<LPTSTR>(selectedTorrent.c_str());
			
	int itemPos = torrentsLVC.FindItem(&findInfo, -1);
	if (itemPos >= 0)
		torrentsLVC.DeleteItem(itemPos);		
		
	::SendMessage(mainHaliteWindow,WM_UPDATEUIINFO,0,0);	
	
	return 0;	
}

LRESULT HaliteDialog::OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl )
{
	DoDataExchange(true);
	
	halite::setTorrentTransferLimits(selectedTorrent,TranLimitDown,TranLimitUp);
	halite::setTorrentConnectionLimits(selectedTorrent,NoConnDown,NoConnUp);
	
	return 0;
}