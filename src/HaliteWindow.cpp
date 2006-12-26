
#include <string>
#include <boost/format.hpp>
#include <boost/bind.hpp>

#include "stdAfx.hpp"
#include "Halite.hpp"
#include "HaliteWindow.hpp"

#include "CSSFileDialog.hpp"
#include "HaliteListViewCtrl.hpp"
#include "HaliteDialog.hpp"
#include "AdvHaliteDialog.hpp"

#include "ConfigOptions.hpp"
#include "GlobalIni.hpp"
#include "ini/Window.hpp"

void single_selection_manager::sync_with_list(bool list_to_manager)
{
	if (list_to_manager)
	{	
		int itemPos = mp_list_->GetSelectionMark();		
		if (itemPos != -1)
		{
			boost::array<wchar_t, MAX_PATH> pathBuffer;
			mp_list_->GetItemText(itemPos, 0, pathBuffer.c_array(), pathBuffer.size());	
			string selected = wcstombs(pathBuffer.data());
			
			if (selected_ != selected)
			{
				selected_ = selected;
				signal();
			}
		}
		else
		{
			selected_ = "";
			signal();
		}
	}
	else
	{
		LV_FINDINFO findInfo = { sizeof(LV_FINDINFO) }; 
		findInfo.flags = LVFI_STRING;
		
		wstring torrent_name = mbstowcs(selected_);		
		findInfo.psz = torrent_name.c_str();
		
		int itemPos = mp_list_->FindItem(&findInfo, -1);	
		
		if (itemPos != mp_list_->GetSelectionMark())
		{
			mp_list_->SelectItem(itemPos);
			signal();
		}
	}
}

void single_selection_manager::setSelected(int itemPos)
{
	mp_list_->SelectItem(itemPos);
	sync_with_list(true);
}

void single_selection_manager::clear()
{
	mp_list_->DeleteItem(mp_list_->GetSelectionMark());
	halite::bittorrent().removeTorrent(selected_);
	
	mp_list_->SelectItem(0);
	sync_with_list(true);
}

HaliteWindow::HaliteWindow(unsigned areYouMe = 0) :
	mp_list(new HaliteListViewCtrl()),
	single_selection_manager_(mp_list),
	WM_AreYouMe_(areYouMe)
{}

HaliteWindow::~HaliteWindow()
{}

BOOL HaliteWindow::PreTranslateMessage(MSG* pMsg)
{
	if(CFrameWindowImpl<HaliteWindow>::PreTranslateMessage(pMsg))
		return TRUE;

	return mp_dlg->PreTranslateMessage(pMsg);
}

LRESULT HaliteWindow::OnCreate(LPCREATESTRUCT lpcs)
{	
	try
	{
	bool success = halite::bittorrent().listenOn(
		std::make_pair(INI().bitTConfig().portFrom, INI().bitTConfig().portTo));
	assert(success);	
	}
	catch(const std::exception& ex)
	{
		::MessageBoxA(0, ex.what(), "Init Exception", MB_ICONERROR|MB_OK);
	}
	
	halite::bittorrent().resumeAll();
	
	halite::bittorrent().setSessionLimits(
		INI().bitTConfig().maxConnections, INI().bitTConfig().maxUploads);
	halite::bittorrent().setSessionSpeed(
		INI().bitTConfig().downRate, INI().bitTConfig().upRate);
	
/*	if (INI().remoteConfig().isEnabled)
	{
		halite::xmlRpc().bindHost(INI().remoteConfig().port);
	}
*/
	RECT rc; GetClientRect(&rc);
	SetMenu(0);
	
	//Init ToolBar
	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	
	// Init ReBar
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
	
	// Init the StatusBar	
	m_hWndStatusBar = m_StatusBar.Create(*this);
	UIAddStatusBar(m_hWndStatusBar);
	
	int panes[] = {ID_DEFAULT_PANE, IDPANE_STATUS, IDPANE_CAPS_INDICATOR};
	m_StatusBar.SetPanes(panes, 3, false);
	
	// Create the Splitter Control
	m_Split.Create(m_hWnd, rc, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
	m_Split.SetSplitterExtendedStyle(!SPLIT_PROPORTIONAL, SPLIT_PROPORTIONAL);
	m_Split.SetSplitterPos(INI().windowConfig().splitterPos);
	
	m_hWndClient = m_Split.m_hWnd;
	
	// Create ListView and Dialog
	mp_list->Create(m_Split.m_hWnd, rc, NULL, 
		LVS_REPORT|LVS_SINGLESEL|WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|LVS_SHOWSELALWAYS);
	
	mp_dlg.reset(new HaliteDialog(ui(), selection())),
	mp_dlg->Create(m_Split.m_hWnd);
	mp_dlg->ShowWindow(true);
	
	mp_advDlg.reset(new AdvHaliteDialog(this));
	mp_advDlg->Create(m_Split.m_hWnd);
//	mp_advDlg->ShowWindow(true);
	
	m_Split.SetSplitterPanes(*mp_list, *mp_dlg);
	
	// Create the tray icon.
	m_trayIcon.Create(this, IDR_TRAY_MENU, L"Halite", 
		CTrayNotifyIcon::LoadIconResource(IDR_APP_ICON), WM_TRAYNOTIFY, IDR_TRAY_MENU);
	m_trayIcon.Hide();
	
	// Add ToolBar and register it along with StatusBar for UIUpdates
	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	UISetCheck(IDR_TRAY_MENU, 1);
	
	// Register UIEvents and the timer for the monitoring interval
	SetTimer(ID_UPDATE_TIMER, 500);
	ui().attach(bind(&HaliteWindow::updateWindow, this));
	ui().attach(bind(&HaliteListViewCtrl::updateListView, &*mp_list));
	
	RegisterDropTarget();
	
	// Register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	assert(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);
	
	selection().setSelected(0);
	ui().update();
	
	return 0;
}

/*const string& HaliteWindow::getSelected() const 
{
	return selectedTorrent_; 
}

void HaliteWindow::setSelected(int index) 
{
	mp_list->SetSelectionMark(index);
	selectionChanged();
}

void HaliteWindow::clearSelected()
{
	mp_list->DeleteItem(mp_list->GetSelectionMark());
	halite::bittorrent().removeTorrent(selectedTorrent_);
	selectedTorrent_ = "";	
	
	selectionChanged();
}

void HaliteWindow::selectionChanged()
{
	int itemPos = mp_list->GetSelectionMark();
	
	if (itemPos != -1)
	{
		boost::array<wchar_t, MAX_PATH> pathBuffer;
		mp_list->GetItemText(itemPos, 0, pathBuffer.c_array(), pathBuffer.size());	
		selectedTorrent_ = wcstombs(pathBuffer.data());
	}
	
	mp_dlg->selectionChanged();
}
*/
LRESULT HaliteWindow::OnNotify(int wParam, LPNMHDR lParam)
{
	if (lParam->hwndFrom == *mp_list && lParam->code == NM_CLICK)
	{
		selection().sync_with_list(true);
	}
	return 0;
}

LRESULT HaliteWindow::OnTrayNotification(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
    m_trayIcon.OnTrayNotification(wParam, lParam);
    
    return 0;
}

void HaliteWindow::updateWindow()
{
	int port = halite::bittorrent().isListeningOn();
	if (port > -1)
	{
		UISetText(0, 
			(wformat(L"Listening on port %1%") % port ).str().c_str());	
	}
	else
		UISetText(0,L"Halite not listening, try adjusting the port range");
	
	pair<double, double> speed = halite::bittorrent().sessionSpeed();
	wstring downloadRates = (wformat(L"(D-U) %1$.2fkb/s - %2$.2fkb/s") 
			% (speed.first/1024) 
			% (speed.second/1024)).str();
	
	UISetText(1, downloadRates.c_str());	
	m_trayIcon.SetTooltipText(downloadRates.c_str());
}

void HaliteWindow::updateConfigSettings()
{
	halite::bittorrent().listenOn(
		std::make_pair(INI().bitTConfig().portFrom, INI().bitTConfig().portTo));
	
	halite::bittorrent().setSessionLimits(
		INI().bitTConfig().maxConnections, INI().bitTConfig().maxUploads);
	halite::bittorrent().setSessionSpeed(
		INI().bitTConfig().downRate, INI().bitTConfig().upRate);
	
//	if (INI().remoteConfig().isEnabled)
//		halite::xmlRpc().bindHost(INI().remoteConfig().port);
//	else
//		halite::xmlRpc().stopHost();
}

void HaliteWindow::OnTimer(UINT uTimerID, TIMERPROC pTimerProc)
{
	if (uTimerID == ID_UPDATE_TIMER) 
	{
		ui().update();
	}
	else 
	{		
		SetMsgHandled(false);
	}
}	

LRESULT HaliteWindow::OnCopyData(HWND, PCOPYDATASTRUCT pCSD)
{
	switch (pCSD->dwData)
	{
		case HALITE_SENDING_CMD:
		wstring filename(static_cast<wchar_t*>(pCSD->lpData), pCSD->cbData/sizeof(wchar_t));
		ProcessFile(filename.c_str());
	}

	return 0;
}

void HaliteWindow::ProcessFile(LPCTSTR lpszPath)
{
	try
	{
	
	// Big changes due here.
	
	path file(wcstombs(lpszPath), boost::filesystem::native);	
	halite::bittorrent().addTorrent(file, globalModule().exePath().branch_path()/"incoming");

	ui().update();
	
	int itemPos = mp_list->GetSelectionMark();
	if (itemPos == -1)
	{
		LV_FINDINFO findInfo = { sizeof(LV_FINDINFO) }; 
		findInfo.flags = LVFI_STRING;
		
		wstring filename = mbstowcs(file.leaf());		
		findInfo.psz = filename.c_str();
		
		int itemPos = mp_list->FindItem(&findInfo, -1);	
		selection().setSelected(itemPos);
	}	
	
	}
	catch(const boost::filesystem::filesystem_error&)
	{
		// Just ignore filesystem errors for now.
	}
}

void HaliteWindow::OnClose()
{
	INI().windowConfig().splitterPos = m_Split.GetSplitterPos();
	
	mp_list->saveStatus();	
	mp_dlg->saveStatus();
	
	SetMsgHandled(false);
}	

void HaliteWindow::OnSize(UINT type, CSize)
{
	if (type == SIZE_MINIMIZED)
	{
		if (INI().windowConfig().use_tray)
		{
			ShowWindow(SW_HIDE);
			m_trayIcon.Show();
		}
	}
	else
		GetWindowRect(INI().windowConfig().rect);
	
	SetMsgHandled(false);
}	

void HaliteWindow::OnMove(CSize)
{
	WINDOWPLACEMENT wnd = { sizeof(WINDOWPLACEMENT ) };
	GetWindowPlacement(&wnd);
	
	if (wnd.showCmd != SW_SHOWMINIMIZED)
		GetWindowRect(INI().windowConfig().rect);

	SetMsgHandled(false);	
}

LRESULT HaliteWindow::OnTrayOpenHalite(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ShowWindow(SW_RESTORE);
	m_trayIcon.Hide();
	
	return 0;
}

LRESULT HaliteWindow::OnTrayExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	PostMessage(WM_CLOSE, 0, 0);
	
	return 0;
}

LRESULT HaliteWindow::OnFileOpen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CSSFileDialog dlgOpen(TRUE, NULL, NULL, OFN_HIDEREADONLY, L"Torrents (*.torrent)|*.torrent|", m_hWnd);

	if (dlgOpen.DoModal() == IDOK) 
	{
		ProcessFile(dlgOpen.m_ofn.lpstrFile);
	}
	
	return 0;	
}

LRESULT HaliteWindow::OnFileNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	MessageBox(L"This feature is under development and currently disabled", 
		L"Feature not availible", 0);
	return 0;
	
	CSSFileDialog dlgOpen(TRUE, NULL, NULL, OFN_HIDEREADONLY, L"All Files|*.*|", m_hWnd);

	if (dlgOpen.DoModal() != IDOK) 
		return 0;
	
	wstring files = dlgOpen.m_ofn.lpstrFile;
	
	CSSFileDialog dlgSave(FALSE, NULL, NULL, OFN_HIDEREADONLY, L"Torrents (*.torrent)|*.torrent|", m_hWnd);
	
	if (dlgSave.DoModal() != IDOK) 
		return 0;
	
	wstring torrent_filename = dlgSave.m_ofn.lpstrFile;
	
	halite::bittorrent().newTorrent(path(wcstombs(torrent_filename),boost::filesystem::native),
		path(wcstombs(files),boost::filesystem::native));
	
	ui().update();
	
	return 0;
}

LRESULT HaliteWindow::OnSettings(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ConfigOptionsProp sheet(L"Settings");	
    sheet.DoModal();
	
	updateConfigSettings();
	
	return 0;
}

LRESULT HaliteWindow::OnPauseAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	halite::bittorrent().pauseAllTorrents();
	ui().update();
	return 0;
}

LRESULT HaliteWindow::OnResumeAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	halite::bittorrent().resumeAllTorrents();
	ui().update();
	return 0;
}


LRESULT HaliteWindow::OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ConfigOptionsProp sheet(L"Settings", 2);	
    sheet.DoModal();
	
	updateConfigSettings();
	
	return 0;
}

LRESULT HaliteWindow::OnViewStatusBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	
	UpdateLayout();
	
	return 0;
}	

LRESULT HaliteWindow::OnEraseBkgnd(HDC hdc)
{

	return 1;
}
