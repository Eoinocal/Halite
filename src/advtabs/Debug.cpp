
#include "../stdAfx.hpp"
#include "../HaliteWindow.hpp"
#include "../HaliteListViewCtrl.hpp"

#include "../GlobalIni.hpp"
#include "../ini/General.hpp"

#include "Debug.hpp"

AdvDebugDialog::AdvDebugDialog(ui_signal& ui_sig, ListViewManager& single_sel) :
	ui_(ui_sig),
	selection_manager_(single_sel)
{
	ui_.attach(bind(&AdvDebugDialog::updateDialog, this));
	selection_manager_.attach(bind(&AdvDebugDialog::selectionChanged, this, _1));
}

void AdvDebugDialog::selectionChanged(const string& torrent_name)
{	
	if (halite::bittorrent().isTorrent(torrent_name))
	{		
		::EnableWindow(GetDlgItem(IDC_TRACKER_LOGINCHECK), true);
		::EnableWindow(GetDlgItem(IDC_TRACKERLIST), true);
	}
	else
	{		
		::EnableWindow(GetDlgItem(IDC_TRACKER_LOGINCHECK), false);
		::EnableWindow(GetDlgItem(IDC_TRACKERLIST), false);
	}
	
	onLoginCheck(0, 0, GetDlgItem(IDC_TRACKER_LOGINCHECK));
	
	DoDataExchange(false);	
	ui_.update();
}

void AdvDebugDialog::onLoginCheck(UINT, int, HWND hWnd)
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
	}
}

LRESULT AdvDebugDialog::onInitDialog(HWND, LPARAM)
{
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
	
//	logEdit.SubclassWindow(GetDlgItem(IDC_DEBUGEDIT));
	logList.SubclassWindow(GetDlgItem(IDC_DEBUGLIST));
	
	DoDataExchange(false);
	return 0;
}

BOOL AdvDebugDialog::DoDataExchange(BOOL bSaveAndValidate, UINT nCtlID)
{	
	DDX_RADIO(IDC_DEBUGNONE, INI().generalConfig().logLevel)

	return TRUE;
}

void AdvDebugDialog::onClose()
{
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

LRESULT AdvDebugDialog::OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl)
{
	string torrent_name = selection_manager_.selected();
	
	if (halite::bittorrent().isTorrent(torrent_name))
	{			
		const int buffer_size = 512;
		array<wchar_t, buffer_size> username;
		array<wchar_t, buffer_size> password;
		
		GetDlgItemText(IDC_TRACKER_USER, username.elems, buffer_size);
		GetDlgItemText(IDC_TRACKER_PASS, password.elems, buffer_size);
		
		halite::bittorrent().setTorrentLogin(torrent_name, wstring(username.elems),
			wstring(password.elems));
	}
	
	DoDataExchange(true);
	
	return 0;
}

void AdvDebugDialog::onDebugOption(UINT, int, HWND)
{
	DoDataExchange(true);
}

void AdvDebugDialog::updateDialog()
{
//	halite::TorrentDetail_ptr pTD = halite::bittorrent().getTorrentDetails(
//		selection_manager_.selected());
	

}
