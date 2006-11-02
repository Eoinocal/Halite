
#include <string>
#include <boost/format.hpp>
#include <boost/bind.hpp>

#include "stdAfx.hpp"
#include "Halite.hpp"
#include "HaliteWindow.hpp"

#include "CSSFileDialog.hpp"
#include "HaliteListViewCtrl.hpp"
#include "HaliteDialog.hpp"

#include "ConfigOptions.hpp"
#include "GlobalIni.hpp"
#include "ini/Window.hpp"

HaliteWindow::HaliteWindow() :
	mp_list(new HaliteListViewCtrl()),
	mp_dlg(new HaliteDialog(this))
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
	bool success = halite::bittorrent().listenOn(
		std::make_pair(INI().bitTConfig().portFrom, INI().bitTConfig().portTo));
	assert(success);	
	
	halite::bittorrent().resumeAll();
	
	halite::bittorrent().setSessionLimits(
		INI().bitTConfig().maxConnections, INI().bitTConfig().maxUploads);
	halite::bittorrent().setSessionSpeed(
		INI().bitTConfig().downRate, INI().bitTConfig().upRate);
	
	if (INI().remoteConfig().isEnabled)
	{
		halite::xmlRpc().bindHost(INI().remoteConfig().port);
	}
	

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
	
	mp_dlg->Create(m_Split.m_hWnd);
	mp_dlg->ShowWindow(true);
	
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
	SetTimer(ID_UPDATE_TIMER, 1000);
	attachUIEvent(bind(&HaliteWindow::updateWindow, this));
	attachUIEvent(bind(&HaliteListViewCtrl::updateListView, &*mp_list));
	attachUIEvent(bind(&HaliteDialog::updateDialog, &*mp_dlg));
	
	RegisterDropTarget();
	
	// Register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	assert(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);
	
	updateUI();	
	
	return 0;
}

LRESULT HaliteWindow::OnNotify(int wParam, LPNMHDR lParam)
{
	if (lParam->hwndFrom == *mp_list && lParam->code == NM_CLICK)
	{
		updateUI();	
		
		int itemPos = mp_list->GetSelectionMark();
		
		if (itemPos != -1)
		{
			wchar_t filenameBuffer[MAX_PATH];		
			mp_list->GetItemText(itemPos,0,static_cast<LPTSTR>(filenameBuffer),256);		
			mp_dlg->setSelectedTorrent(halite::wcstombs(filenameBuffer));
		}
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
	
	if (INI().remoteConfig().isEnabled)
		halite::xmlRpc().bindHost(INI().remoteConfig().port);
	else
		halite::xmlRpc().stopHost();
}

void HaliteWindow::OnTimer(UINT uTimerID, TIMERPROC pTimerProc)
{
	if (uTimerID == ID_UPDATE_TIMER) 
	{
		updateUI();
	}
	else 
	{		
		SetMsgHandled(false);
	}
}	

void HaliteWindow::ProcessFile(LPCTSTR lpszPath)
{
    halite::bittorrent().addTorrent(path(halite::wcstombs(lpszPath), boost::filesystem::native));
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
	SetMsgHandled(false);
	
	if (type == SIZE_MINIMIZED)
	{
		ShowWindow(SW_HIDE);
		m_trayIcon.Show();
	}
	else
	{
		GetWindowRect(INI().windowConfig().rect);
	}
}	

void HaliteWindow::OnMove(CSize)
{
	SetMsgHandled(false);
	
	GetWindowRect(INI().windowConfig().rect);
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
	CSSFileDialog dlgOpen(TRUE, NULL, NULL, OFN_HIDEREADONLY, L"Torrents (*.*)|*.torrent|", m_hWnd);

	if (dlgOpen.DoModal() == IDOK) 
	{
		wstring filename = dlgOpen.m_ofn.lpstrFile;
		halite::bittorrent().addTorrent(path(halite::wcstombs(filename),boost::filesystem::native));
	}
	updateUI();
		
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
	updateUI();
	return 0;
}

LRESULT HaliteWindow::OnResumeAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	halite::bittorrent().resumeAllTorrents();
	updateUI();
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
