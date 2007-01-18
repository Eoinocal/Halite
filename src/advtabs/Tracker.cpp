
#include "../stdAfx.hpp"
#include "../HaliteWindow.hpp"
#include "../HaliteListView.hpp"

#include "../GlobalIni.hpp"
#include "../ini/Dialog.hpp"

#include <global_log.hpp>
using glb::wlog;

#include "Tracker.hpp"

AdvTrackerDialog::AdvTrackerDialog(ui_signal& ui_sig, ListViewManager& single_sel) :
	ui_(ui_sig),
	selection_manager_(single_sel)
{
	ui_.attach(bind(&AdvTrackerDialog::updateDialog, this));
	selection_manager_.attach(bind(&AdvTrackerDialog::selectionChanged, this, _1));
}

void AdvTrackerDialog::selectionChanged(const string& torrent_name)
{	
	if (halite::bittorrent().isTorrent(torrent_name))
	{		
		::EnableWindow(GetDlgItem(IDC_TRACKER_LOGINCHECK), true);
		::EnableWindow(GetDlgItem(IDC_TRACKERLIST), true);
		
		pair<wstring, wstring> details = 
			halite::bittorrent().getTorrentLogin(selection_manager_.selected());
		
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
	
	setLoginUiState(torrent_name);
	DoDataExchange(false);	

	ui_.update();
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
		
		DoDataExchange(false);		
	}
}

LRESULT AdvTrackerDialog::onInitDialog(HWND, LPARAM)
{
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
	
	m_list.Attach(GetDlgItem(IDC_TRACKERLIST));
	
	if (halite::bittorrent().isTorrent(selection_manager_.selected()))
	{		
		::EnableWindow(GetDlgItem(IDC_TRACKER_LOGINCHECK), true);
		::EnableWindow(GetDlgItem(IDC_TRACKERLIST), true);
		
		pair<wstring, wstring> details = 
			halite::bittorrent().getTorrentLogin(selection_manager_.selected());
		
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
		
	setLoginUiState(selection_manager_.selected());
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
	
	setLoginUiState(selection_manager_.selected());
	halite::bittorrent().setTorrentLogin(selection_manager_.selected(), username_, password_);
	
	return 0;
}

void AdvTrackerDialog::onReannounce(UINT, int, HWND)
{
	halite::bittorrent().reannounceTorrent(selection_manager_.selected());
}

void AdvTrackerDialog::updateDialog()
{

}
