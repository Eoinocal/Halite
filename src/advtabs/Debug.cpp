
#include "../stdAfx.hpp"
#include "../HaliteWindow.hpp"
#include "../HaliteListViewCtrl.hpp"

#include "Debug.hpp"

void AdvDebugDialog::selectionChanged(const string& torrent_name)
{	
#	if 0
	if (hal::bittorrent().isTorrent(torrent_name))
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

#	endif	
	DoDataExchange(false);	
//	ui().update();
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
	dialogBaseClass::InitializeHalDialogBase();	
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);	
	logList.Attach(GetDlgItem(IDC_DEBUGLISTVIEW));
	
	DoDataExchange(false);
	return 0;
}

BOOL AdvDebugDialog::DoDataExchange(BOOL bSaveAndValidate, UINT nCtlID)
{	
	DDX_CHECK(IDC_DEBUGFILECHECK, halite().logToFile_)
	DDX_CHECK(IDC_DEBUGDEBUGCHECK, halite().logDebug_)

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
/*	string torrent_name = selection_manager().selected();
	
	if (hal::bittorrent().isTorrent(torrent_name))
	{			
		const int buffer_size = 512;
		array<wchar_t, buffer_size> username;
		array<wchar_t, buffer_size> password;
		
		GetDlgItemText(IDC_TRACKER_USER, username.elems, buffer_size);
		GetDlgItemText(IDC_TRACKER_PASS, password.elems, buffer_size);
		
		hal::bittorrent().setTorrentLogin(torrent_name, wstring(username.elems),
			wstring(password.elems));
	}
*/		
	DoDataExchange(true);

	return 0;
}

void AdvDebugDialog::onDebugOption(UINT, int, HWND)
{
	DoDataExchange(true);
}

void AdvDebugDialog::updateDialog()
{}
