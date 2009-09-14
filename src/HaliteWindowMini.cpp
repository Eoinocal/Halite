
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <boost/format.hpp>
#include <boost/bind.hpp>

#include "stdAfx.hpp"
#include "HaliteMini.hpp"
#include "HaliteWindowMini.hpp"

#include "halConfig.hpp"

HaliteWindow::HaliteWindow(unsigned areYouMe = 0) :
	iniClass("HaliteWindow", "HaliteWindow"),
	WM_AreYouMe_(areYouMe),
	splitterPos(100),
	use_tray(true),
	advancedUI(false),
	activeTab(0)
{
	rect.top = 10;
	rect.left = 10;
	rect.bottom = 430;
	rect.right = 620;
	
	load();
}

HaliteWindow::~HaliteWindow()
{
	save();
	ATLASSERT(!::IsWindow(m_hWnd));
}

LRESULT HaliteWindow::OnCreate(LPCREATESTRUCT lpcs)
{
	SetWindowText(L"Halite");
	MoveWindow(rect.left, rect.top,	rect.right-rect.left, rect.bottom-rect.top, false);	
	
	hal::config().load();
	hal::config().settingsChanged();
	
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
	
	int panes[] = {ID_DEFAULT_PANE, IDPANE_FILTER, IDPANE_DHT, IDPANE_STATUS};
	m_StatusBar.SetPanes(panes, 4, false);
	
	// Create the Splitter Control
	m_Split.Create(m_hWnd, rc, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
	m_Split.SetSplitterExtendedStyle(!SPLIT_PROPORTIONAL, SPLIT_PROPORTIONAL);
	m_Split.SetSplitterPos(splitterPos);
	
	m_hWndClient = m_Split.m_hWnd;
	
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
	SetTimer(ID_SAVE_TIMER, 5000);
	connectUiUpdate(bind(&HaliteWindow::updateWindow, this));
	
	RegisterDropTarget();
	
	// Register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	assert(pLoop != NULL);
//	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);
	
	hal::bittorrent().startEventReceiver();
	return 0;
}

LRESULT HaliteWindow::OnAdvanced(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	advancedUI = !advancedUI;
	setCorrectDialog();
	
	return 0;
}

LRESULT HaliteWindow::OnTrayNotification(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
    m_trayIcon.OnTrayNotification(wParam, lParam);
    
    return 0;
}

void HaliteWindow::setCorrectDialog()
{

	ui().update();
}

void HaliteWindow::updateWindow()
{
	try
	{
	
	hal::SessionDetail details = hal::bittorrent().getSessionDetails();
	
	if (details.port > -1)
		UISetText(0, 
			(wformat(hal::app().res_wstr(HAL_PORT_OPEN)) % details.port ).str().c_str());	
	else
		UISetText(0, hal::app().res_wstr(HAL_NOT_LISTENING).c_str());
	
	wstring downloadRates = (wformat(hal::app().res_wstr(HAL_DOWN_RATES)) 
			% (details.speed.first/1024) 
			% (details.speed.second/1024)).str();
	
	UISetText(3, downloadRates.c_str());	
	m_trayIcon.SetTooltipText(downloadRates.c_str());
	
	if (details.dht_on)
	{
		wstring dht = (wformat(hal::app().res_wstr(HAL_DHT_ON))
			% details.dht_nodes).str();
			
		UISetText(2, dht.c_str());
	}
	else
	{
		UISetText(2, hal::app().res_wstr(HAL_DHT_OFF).c_str());
	}
	
	if (details.ip_filter_on)
	{
		wstring filter = (wformat(hal::app().res_wstr(HAL_IPFILTER_ON))
			% details.ip_ranges_filtered).str();
		
		UISetText(1, filter.c_str());
	}
	else
	{
		UISetText(1, hal::app().res_wstr(HAL_IPFILTER_OFF).c_str());
	}
	
	}
	catch (std::exception& e)
	{
		hal::event().post(shared_ptr<hal::EventDetail>(\
			new hal::EventStdException(hal::Event::info, e, L"updateWindow")));
	}
}

void HaliteWindow::OnTimer(UINT uTimerID)
{		
	if (uTimerID == ID_UPDATE_TIMER) 
	{	
		issueUiUpdate();
	}
	else if (uTimerID == ID_SAVE_TIMER) 
	{
		try
		{
		
		hal::ini().save_data();
		hal::bittorrent().saveTorrentData();	
	
		}
		catch (std::exception& e)
		{
			hal::event().post(shared_ptr<hal::EventDetail>(\
				new hal::EventStdException(hal::Event::info, e, L"saveTimer")));
		}
	}
	else 
	{		
		SetMsgHandled(false);
	}	
}	

void HaliteWindow::issueUiUpdate()
{	
	try
	{
	
//	const hal::TorrentDetails& torrents = hal::bittorrent().updateTorrentDetails(
//		haliteList.manager().selected(), haliteList.manager().allSelected());

//	ui_update_signal_(torrents);
	updateWindow();

	}
	catch (std::exception& e)
	{
		hal::event().post(shared_ptr<hal::EventDetail>(
			new hal::EventStdException(hal::Event::info, e, L"updateTimer")));
	}
}

LRESULT HaliteWindow::OnCopyData(HWND, PCOPYDATASTRUCT pCSD)
{
	hal::event().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"I recieved data.", hal::Event::info)));
		
	switch (pCSD->dwData)
	{
		case HALITE_SENDING_CMD:
		{	
			wstring filename(static_cast<wchar_t*>(pCSD->lpData), pCSD->cbData/sizeof(wchar_t));
			
			hal::event().post(shared_ptr<hal::EventDetail>(
				new hal::EventMsg((wformat(L"Recieved data: %1%.") % filename), hal::Event::info)));
		
			ProcessFile(filename.c_str());
			break;
		}
		default:
			break;
	}
	return 0;
}

void HaliteWindow::ProcessFile(LPCTSTR lpszPath)
{
	try
	{

/*	
	wstring saveDirectory = wpath(hal::config().defaultSaveFolder).native_file_string();
	bool startPaused = false;
	bool compactStorage = false;
	
	if (!boost::filesystem::exists(saveDirectory))
		boost::filesystem::create_directory(saveDirectory);
	
	if (hal::config().savePrompt)
	{
		AddTorrentDialog addTorrent(saveDirectory, startPaused, compactStorage);	
		
		if (IDOK != addTorrent.DoModal())
			return;
	}
*/	
/*	wpath file(lpszPath, boost::filesystem::native);	
	hal::bittorrent().addTorrent(file, wpath(saveDirectory), startPaused, compactStorage);

	ui().update();
*/	
/*	int itemPos = haliteList.GetSelectionMark();
	if (itemPos == -1)
	{
		LV_FINDINFO findInfo = { sizeof(LV_FINDINFO) }; 
		findInfo.flags = LVFI_STRING;
		
		wstring filename = file.leaf();		
		findInfo.psz = filename.c_str();
		
		int itemPos = haliteList.FindItem(&findInfo, -1);	
		haliteList.manager().setSelected(itemPos);
	}	
*/	
	}
	catch(const boost::filesystem::filesystem_error&)
	{
		hal::event().post(shared_ptr<hal::EventDetail>(
			new hal::EventDebug(hal::Event::warning, L"filesystem error")));
	}
}

void HaliteWindow::OnClose()
{
	splitterPos = m_Split.GetSplitterPos();

	hal::config().save();
	save();
	
	DestroyWindow();
}
 
void HaliteWindow::OnDestroy()
{
	PostQuitMessage(0);
}

void HaliteWindow::OnSize(UINT type, CSize)
{
	if (type == SIZE_MINIMIZED)
	{
		if (use_tray)
		{
			ShowWindow(SW_HIDE);
			m_trayIcon.Show();
		}
	}
	else
		GetWindowRect(rect);
	
	SetMsgHandled(false);
}	

void HaliteWindow::OnMove(CSize)
{
	WINDOWPLACEMENT wnd = { sizeof(WINDOWPLACEMENT ) };
	GetWindowPlacement(&wnd);
	
	if (wnd.showCmd != SW_SHOWMINIMIZED)
		GetWindowRect(rect);

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
/*	CSSFileDialog dlgOpen(TRUE, NULL, NULL, OFN_HIDEREADONLY, L"Torrents (*.torrent)|*.torrent|", m_hWnd);

	if (dlgOpen.DoModal() == IDOK) 
	{
		ProcessFile(dlgOpen.m_ofn.lpstrFile);
	}
	*/
	return 0;	
}

LRESULT HaliteWindow::OnFileNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
/*	MessageBox(L"This feature is under development and currently disabled", 
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
	
	hal::bittorrent().newTorrent(wpath(torrent_filename), wpath(files));
	
	ui().update();
	*/
	return 0;
}

LRESULT HaliteWindow::OnSettings(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

	
	return 0;
}

LRESULT HaliteWindow::OnPauseAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	hal::bittorrent().pauseAllTorrents();
	
	ui().update();
	return 0;
}

LRESULT HaliteWindow::OnResumeAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	hal::bittorrent().unpauseAllTorrents();
	
	ui().update();
	return 0;
}

LRESULT HaliteWindow::OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
//	ConfigOptionsProp sheet(this, L"Settings", 4);	
//    sheet.DoModal();
	
	hal::config().settingsChanged();
	
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

LRESULT HaliteWindow::OnEraseBkgnd(HDC dc)
{
	return 1;
}

LRESULT HaliteWindow::OnPaint(HDC dc)
{
	return 1;
}

LRESULT HaliteWindow::OnAreYouMe(UINT, WPARAM, LPARAM, BOOL&) 
{
	hal::event().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"I tried to contact me.", hal::Event::info)));		

	return WM_AreYouMe_; 
}
