
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"
#include "Halite.hpp"
#include "HaliteWindow.hpp"

#include "CSSFileDialog.hpp"
#include "RadioPaneDlg.hpp"

#include "HaliteDialog.hpp"
#include "AdvHaliteDialog.hpp"
#include "AddTorrentDialog.hpp"
#include "NewTorrentDialog.hpp"
#include "SplashDialog.hpp"
#include "TimePickerDlg.hpp"
#include "ProgressDialog.hpp"

#include "ConfigOptions.hpp"
#include "halConfig.hpp"

HaliteWindow::HaliteWindow(unsigned areYouMe = 0) :
	ini_class_t(L"halite_window", L"halite_window"),
	haliteList(*this),
	WM_AreYouMe_(areYouMe),
	splitterPos(100),
	use_tray(true),
	advancedUI(false),
	closeToTray(false),
	confirmClose(true),
	activeTab(0),
	torrent_complete_balloon_(true),
	balloons_timout_(10),
	action_time_(boost::posix_time::not_a_date_time),
	action_action_(TimePickerDlg::action_na)
{
	rect.top = 10;
	rect.left = 10;
	rect.bottom = 440;
	rect.right = 744;
	
	load_from_ini();
}

HaliteWindow::~HaliteWindow()
{
	save_to_ini();
	ATLASSERT(!::IsWindow(m_hWnd));
}

BOOL HaliteWindow::PreTranslateMessage(MSG* pMsg)
{
	if(CFrameWindowImpl<this_class_t>::PreTranslateMessage(pMsg))
		return TRUE;

	if (!advancedUI)
		return mp_dlg->PreTranslateMessage(pMsg);
	else
		return mp_advDlg->PreTranslateMessage(pMsg);
}

LRESULT HaliteWindow::OnCreate(LPCREATESTRUCT lpcs)
{
	try
	{
	HAL_DEV_MSG(L"HaliteWindow::OnCreate");
	
	SetWindowText(L"Halite");
	MoveWindow(rect.left, rect.top,	rect.right-rect.left, rect.bottom-rect.top, false);	

//		MARGINS m = {20, 20, 0, 100};
//		SetMargins(m);

	hal::event_log().set_debug_logging(true);
	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"Loading Halite configuration ...")));
	hal::config().load_from_ini();
	hal::config().set_callback(boost::bind(&HaliteWindow::runProgressCommand, this, _1, _2));

	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"	... Done")));
	
	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"Applying BitTorrent settings ...")));
	if (!hal::config().settingsChanged())
	{
		hal::event_log().post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventDebug(hal::event_logger::critical, hal::app().res_wstr(HAL_WINDOW_SOCKETS_FAILED))));
			
		MessageBox(hal::app().res_wstr(HAL_WINDOW_SOCKETS_FAILED).c_str(), 0, 0);
		
		DestroyWindow();
		return 0;
	}
	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"	... Done")));
	
	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"Starting Halite GUI ...")));
	
	RECT rc; GetClientRect(&rc);
	SetMenu(0);
	
	//Init ToolBar
	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, HAL_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	
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
	
	m_hWndClient = m_Split.m_hWnd;

	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"	... Creating main listview")));

	// Create ListView and Dialog
	haliteList.Create(m_Split.m_hWnd, rc, NULL, 
		LVS_REPORT|WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|LVS_SHOWSELALWAYS|LVS_OWNERDATA);//|LVS_EDITLABELS);
//	haliteList.manager().attach(bind(&HaliteWindow::issueUiUpdate, this));

	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"	... Creating classic dialog...")));		
	mp_dlg.reset(new HaliteDialog(*this)),
	mp_dlg->Create(m_Split.m_hWnd);
//	mp_dlg->ShowWindow(true);	

	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"	... Creating advanced dialog")));
	mp_advDlg.reset(new AdvHaliteDialog(*this));
	mp_advDlg->Create(m_Split.m_hWnd);
//	mp_advDlg->ShowWindow(true);
	
//	m_Split.SetSplitterPanes(*mp_list, *mp_dlg);
	
	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"	... Creating tray icon...")));	
	// Create the tray icon.
	trayIcon_.Create(this, HAL_TRAY_MENU, L"Halite", 
		CTrayNotifyIcon::LoadIconResource(HAL_APP_ICON), WM_TRAYNOTIFY, HAL_TRAY_MENU);
	trayIcon_.Hide();

	//Set callback for completed torrents
	hal::bittorrent().connect_torrent_completed_signal(
					boost::bind(&HaliteWindow::torrentCompletedCallback, this, _1));

	// Add ToolBar and register it along with StatusBar for UIUpdates
	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, true);
	UISetCheck(ID_VIEW_STATUS_BAR, true);
	UISetCheck(HAL_TRAY_MENU, true);	
	
//	TBBUTTONINFO tbinfo = { sizeof(TBBUTTONINFO) };
//	tbinfo.dwMask = TBIF_STATE;
//	tbinfo.fsState = TBSTATE_INDETERMINATE;
//	::SendMessage(hWndToolBar, TB_SETBUTTONINFO, ID_FILE_NEW, (LPARAM)&tbinfo);

	// Register UIEvents and the timer for the monitoring interval
	SetTimer(ID_UPDATE_TIMER, 500);
	SetTimer(ID_SAVE_TIMER, 300000);
	connectUiUpdate(bind(&HaliteWindow::updateWindow, this));
	
	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"	... Registering drop target")));	
	RegisterDropTarget();
	
	// Register object for message filtering and idle updates
	WTL::CMessageLoop* pLoop = _Module.GetMessageLoop();
	assert(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);
	
	setCorrectDialog();
	
	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"Starting BitTorrent event receiver ...")));
	hal::bittorrent().start_event_receiver();

	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"Restoring torrent states ...")));
	hal::bittorrent().resume_all();

	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"Initial setup complete!")));

	UpdateLayout();
	issueUiUpdate();

	m_Split.SetSplitterPos(splitterPos);
		
	}
	HAL_GENERIC_FN_EXCEPTION_CATCH(L"in HaliteWindow::OnCreate")

	return 0;
}

LRESULT HaliteWindow::OnAdvanced(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	advancedUI = !advancedUI;
	setCorrectDialog();
	
	return 0;
}

LRESULT HaliteWindow::OnTrayNotification(UINT, WPARAM wParam, LPARAM lParam)
{
	trayIcon_.OnTrayNotification(wParam, lParam);

	return 0;
}

void HaliteWindow::setCorrectDialog()
{
	if (!advancedUI)
	{		
		mp_dlg->ShowWindow(SW_SHOW);
		mp_advDlg->ShowWindow(SW_HIDE);
		m_Split.SetSplitterPanes(haliteList, *mp_dlg);
	}
	else
	{		
		mp_dlg->ShowWindow(SW_HIDE);
		mp_advDlg->ShowWindow(SW_SHOW);
		m_Split.SetSplitterPanes(haliteList, *mp_advDlg);
	}

	issueUiUpdate();
}

void HaliteWindow::updateWindow()
{
	try
	{
	
	hal::SessionDetail details = hal::bittorrent().get_session_details();
	
	if (details.port > -1)
		UISetText(0, 
			(hal::wform(hal::app().res_wstr(HAL_PORT_OPEN)) % details.port ).str().c_str());	
	else
		UISetText(0, hal::app().res_wstr(HAL_NOT_LISTENING).c_str());
	
	wstring downloadRates = (hal::wform(hal::app().res_wstr(HAL_DOWN_RATES)) 
			% hal::to_bytes_size(details.speed.first, true) 
			% hal::to_bytes_size(details.speed.second, true)).str();
	
	UISetText(3, downloadRates.c_str());	
	trayIcon_.SetTooltipText(downloadRates.c_str());
	
	if (details.dht_on)
	{
		wstring dht = (hal::wform(hal::app().res_wstr(HAL_DHT_ON))
			% details.dht_nodes).str();
			
		UISetText(2, dht.c_str());
	}
	else
	{
		UISetText(2, hal::app().res_wstr(HAL_DHT_OFF).c_str());
	}
	
	if (details.ip_filter_on)
	{
		wstring filter = (hal::wform(hal::app().res_wstr(HAL_IPFILTER_ON))
			% details.ip_ranges_filtered).str();
		
		UISetText(1, filter.c_str());
	}
	else
	{
		UISetText(1, hal::app().res_wstr(HAL_IPFILTER_OFF).c_str());
	}

	UpdateLayout();
	
	} HAL_GENERIC_FN_EXCEPTION_CATCH(L"in HaliteWindow::updateWindow()")
}

void HaliteWindow::OnTimer(UINT_PTR uTimerID)
{		
	try
	{

	if (uTimerID == ID_UPDATE_TIMER) 
	{	
		issueUiUpdate();
	}
	else if (uTimerID == ID_SAVE_TIMER) 
	{
		try
		{

		hal::bittorrent().save_torrent_data();	
	
		} HAL_GENERIC_FN_EXCEPTION_CATCH(L"HaliteWindow::OnTimer(ID_SAVE_TIMER)")
	}
	else 
	{		
		SetMsgHandled(false);
	}	
	
	} HAL_GENERIC_FN_EXCEPTION_CATCH(L"HaliteWindow::OnTimer")
}	

void HaliteWindow::issueUiUpdate()
{	
	try
	{
	std::set<hal::uuid> s;
	
	for (auto i = haliteList.begin_selected(), e = haliteList.end_selected(); i != e; ++i)
	{
		s.insert(item_hash(i));
	}
	
	const hal::torrent_details_manager& torrents = hal::bittorrent().update_torrent_details_manager(
		s.empty() ? hal::uuid() : item_hash(haliteList.begin_selected()), 
		s);

	Sleep(1);

	ui_update_signal_(torrents);

	} HAL_GENERIC_FN_EXCEPTION_CATCH(L"HaliteWindow::issueUiUpdate()")
}

LRESULT HaliteWindow::OnCopyData(HWND, PCOPYDATASTRUCT pCSD)
{
	hal::event_log().post(shared_ptr<hal::EventDetail>(
		new hal::EventMsg(L"I received data.")));
		
	switch (pCSD->dwData)
	{
		case HALITE_SENDING_CMD:
		{	
			wstring filename(static_cast<wchar_t*>(pCSD->lpData), pCSD->cbData/sizeof(wchar_t));
			
			hal::event_log().post(shared_ptr<hal::EventDetail>(
				new hal::EventMsg((hal::wform(L"Received data: %1%.") % filename), hal::event_logger::info)));
		
			ProcessFile(filename.c_str());
			break;
		}
		default:
			break;
	}
	return 0;
}

void HaliteWindow::ProcessFile(wstring path_or_uri)
{
	try
	{	

	wstring default_save_folder = wpath(hal::config().default_save_folder_).wstring();
	wstring default_move_folder = wpath(hal::config().default_move_folder_).wstring();
	bool use_move_to = hal::config().use_move_to_;
	bool startPaused = false;
	bool managed = true;
	hal::bit::allocations allocation_type = hal::config().default_allocation_type_;
	bool is_magnet = boost::find_first(path_or_uri, L"magnet:");
	
	if (!boost::filesystem::exists(default_save_folder))
		boost::filesystem::create_directories(default_save_folder);

	if (hal::config().save_prompt_)
	{
		AddTorrentDialog addTorrent(path_or_uri, false, default_save_folder, default_move_folder, use_move_to, startPaused, managed, allocation_type);	
		
		if (IDOK != addTorrent.DoModal())
			return;
	}
		
	HAL_DEV_MSG(hal::wform(L"Path : %s, Is magnet : %s") % path_or_uri.c_str() % is_magnet);

	if (is_magnet) 
	{
		if (use_move_to)
			hal::bittorrent().add_torrent(path_or_uri, wpath(default_save_folder), startPaused, managed, allocation_type, wpath(default_move_folder));
		else
			hal::bittorrent().add_torrent(path_or_uri, wpath(default_save_folder), startPaused, managed, allocation_type);
	}
	else
	{
		wpath file(path_or_uri, boost::filesystem::native);

		if (use_move_to)
			hal::bittorrent().add_torrent(file, wpath(default_save_folder), startPaused, managed, allocation_type, wpath(default_move_folder));
		else
			hal::bittorrent().add_torrent(file, wpath(default_save_folder), startPaused, managed, allocation_type);
	}

	issueUiUpdate();

	}
	HAL_FILESYSTEM_EXCEPTION_CATCH("in ProcessFile")
	catch (const std::exception&)
	{
		hal::event_log().post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(L"File creation error.", hal::event_logger::warning)));
	}
	catch (...)
	{
		hal::event_log().post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(L"File creation error.", hal::event_logger::warning)));	
	}
}

void HaliteWindow::OnClose()
{
	if (closeToTray && trayIcon_.IsHidden())
	{		
		ShowWindow(SW_HIDE);
		trayIcon_.Show();
	}
	else
	{
		TryToCloseWithConfirmation();
	}
}

void HaliteWindow::ShutdownThread()
{
	try
	{
	win32_exception::install_handler();

	hal::bittorrent().close_all();

	hal::bittorrent().stop_event_receiver();
	Sleep(500);

	hal::bittorrent().shutdown_session();

	} HAL_GENERIC_FN_EXCEPTION_CATCH(L"HaliteWindow::ShutdownThread()")
}

void HaliteWindow::TryToCloseWithConfirmation()
{
	bool noTorrentsAreActive = !hal::bittorrent().is_any_torrent_active();
	
	auto a = hal::app().res_wstr(HAL_WINDOW_CLOSECONFRIM).c_str();
	auto b = hal::app().res_wstr(HAL_HALITE).c_str();

	if (noTorrentsAreActive || !confirmClose || (confirmClose && 
		MessageBox(hal::app().res_wstr(HAL_WINDOW_CLOSECONFRIM).c_str(), 
			hal::app().res_wstr(HAL_HALITE).c_str(), MB_YESNO) == IDYES))
	{
		DestroyWindow();
	}
}

void HaliteWindow::OnDestroy()
{	
	try
	{

	KillTimer(ID_UPDATE_TIMER);
	KillTimer(ID_SAVE_TIMER);

	Sleep(0);

	splitterPos = m_Split.GetSplitterPos();

	save_to_ini();
	
	if (halite().showMessage())
	{
		HAL_DEV_MSG(L"Showing SplashDialog");

		SplashDialog splDlg;
		splDlg.DoModal();
	}
	else
	{		
		HAL_DEV_MSG(L"No SplashDialog");

		thread shutdown(bind(& HaliteWindow::ShutdownThread, this));
		shutdown.join();
	}
		
	HAL_DEV_MSG(L"Saving before quiting");

	// Resave for sake of your health.
	save_to_ini();
	halite().save_to_ini();

	if (post_halite_function_) post_halite_function_();
		
	HAL_DEV_MSG(L"Posting Quit Message");
	PostQuitMessage(0);	

	} HAL_GENERIC_FN_EXCEPTION_CATCH(L"HaliteWindow::OnDestroy()")
}

void HaliteWindow::OnSize(UINT type, WTL::CSize)
{
	if (type == SIZE_MINIMIZED)
	{
		if (use_tray)
		{
			ShowWindow(SW_HIDE);
			trayIcon_.Show();
		}
	}
	else
	{
		if (trayIcon_.IsShowing())
			trayIcon_.Hide();

		GetWindowRect(rect);
	}
	
	UpdateLayout();
	SetMsgHandled(false);
}	

void HaliteWindow::OnMove(WTL::CSize)
{
	WINDOWPLACEMENT wnd = { sizeof(WINDOWPLACEMENT ) };
	GetWindowPlacement(&wnd);
	
	if (wnd.showCmd != SW_SHOWMINIMIZED)
		GetWindowRect(rect);

	SetMsgHandled(false);	
}

void HaliteWindow::OnShowWindow(BOOL bShow, UINT nStatus)
{
	SetMsgHandled(false);
}

LRESULT HaliteWindow::OnTrayOpenHalite(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (trayIcon_.IsShowing())
		trayIcon_.Hide();

	ShowWindow(SW_RESTORE);
	
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

LRESULT HaliteWindow::OnFileOpenMagnet(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	OpenClipboard();

	if (IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		HGLOBAL data = GetClipboardData(CF_UNICODETEXT);
		if (data != NULL)
		{
			LPTSTR uri_lp = (LPTSTR) GlobalLock(data);

			std::wstring uri(uri_lp);

			if (boost::find_first(uri_lp, L"magnet:")) 
				ProcessFile(uri);
		}

	}
	
	CloseClipboard();

	/*	WTL::CFolderDialog fldDlg (NULL, 0,
			BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);

		if (IDOK == fldDlg.DoModal())
			hal::remove_empty_directories(wpath(fldDlg.m_szFolderPath));
			*/
//	ProcessFile(L"magnet:?xt=urn:btih:08152215a724dc2ff23fc4065330913bdc23515d&dn=MythBusters+S08E29+Operation+Valkyrie+HDTV+XviD-FQM+%5Beztv%5D&tr=http%3A%2F%2Ftracker.prq.to%2Fannounce");
	
	return 0;	
}

LRESULT HaliteWindow::OnFileNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	wstring title = hal::app().res_wstr(HAL_NEWT_DIALOG_TITLE);

	NewTorrentDialog newTorrent(title.c_str());	
	newTorrent.DoModal();
	
	return 0;
}

LRESULT HaliteWindow::OnSettings(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ConfigOptionsProp sheet(this, L"Settings");	
	sheet.DoModal();
	
	hal::config().settingsChanged();
	setCorrectDialog();
	
	return 0;
}

LRESULT HaliteWindow::OnPauseAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	hal::bittorrent().pause_all_torrents();
	
	issueUiUpdate();
	return 0;
}

LRESULT HaliteWindow::OnResumeAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	hal::bittorrent().unpause_all_torrents();
	
	issueUiUpdate();
	return 0;
}

LRESULT HaliteWindow::OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ConfigOptionsProp sheet(this, L"Settings", 6);	
	sheet.DoModal();
	
	hal::config().settingsChanged();
	
	return 0;
}

LRESULT HaliteWindow::OnToolbarExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TryToCloseWithConfirmation();
	
	return 0;
}

LRESULT HaliteWindow::OnUnconditionalShutdown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	HAL_DEV_MSG(L"In OnUnconditionalShutdown");

	DestroyWindow();

	return 0;
}

void HaliteWindow::torrentCompletedCallback(std::wstring torrent_name)
{
	HAL_DEV_MSG(hal::wform(L"In torrent completed callback, %1%") 
					% torrent_name);

	PostMessage(WM_HALITE_TORRENT_COMPLETED, 0, (LPARAM)new std::wstring(torrent_name));
}

LRESULT HaliteWindow::OnTorrentCompleted(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	try
	{

	boost::scoped_ptr<std::wstring> name(reinterpret_cast<std::wstring*>(lParam));

	if (torrent_complete_balloon_)
	{
		wstring msg = hal::app().res_wstr(HAL_BALLOON_TORRENT_COMPLETE);
		wstring title = hal::app().res_wstr(HAL_BALLOON_TITLE_COMPLETE);

		if (!trayIcon_.SetBalloonDetails(
			(hal::wform(msg) % *name).str().c_str(), title.c_str(), 
				CTrayNotifyIcon::Info, balloons_timout_, 0, 0))
		{
			HAL_DEV_MSG(L"Problem displaying balloon");
		}	
	}
	
	} HAL_GENERIC_FN_EXCEPTION_CATCH(L"HaliteWindow::OnTorrentCompleted")

	return 0;
}

void HaliteWindow::runProgressCommand(wstring title, hal::progress_callback_callback fn)
{	
	ProgressDialog progDlg(title, fn);
	progDlg.DoModal();
}

void HaliteWindow::exitCallback()
{
	HAL_DEV_MSG(L"In exit callback");

	PostMessage(WM_HALITE_UNCONDITIONAL_SHUTDOWN, 0, 0);
}

void HaliteWindow::logoffCallback()
{
	HAL_DEV_MSG(L"In logoff callback");

	post_halite_function_ = boost::bind(boost::function<BOOL (UINT, DWORD)>(ExitWindowsEx), 
		EWX_LOGOFF, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED);

	PostMessage(WM_HALITE_UNCONDITIONAL_SHUTDOWN, 0, 0);
}

void HaliteWindow::shutdownCallback()
{
	HAL_DEV_MSG(L"In shutdown callback");

	post_halite_function_ = boost::bind(boost::function<BOOL (UINT, DWORD)>(ExitWindowsEx), 
		EWX_SHUTDOWN, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED);

	PostMessage(WM_HALITE_UNCONDITIONAL_SHUTDOWN, 0, 0);
}

LRESULT HaliteWindow::OnAutoShutdown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	boost::posix_time::ptime time = action_time_;
	unsigned action = action_action_;

	TimePickerDlg dd(time, action);
	
	if (dd.DoModal() == 1)
	{		
		if (!time.is_not_a_date_time())
		{
			hal::event_log().post(shared_ptr<hal::EventDetail>(
				new hal::EventMsg(hal::wform(L"OnAutoShutdown %1% %2%") 
					% hal::from_utf8(to_simple_string(time)) % action)));

			switch(action)
			{
			case TimePickerDlg::action_pause:

				hal::bittorrent().schedual_action(time, hal::bit::action_pause);
				break;

			case TimePickerDlg::action_exit:

				hal::bittorrent().schedual_callback(
					time, bind(&HaliteWindow::exitCallback, this));
				break;

			case TimePickerDlg::action_logoff:

				hal::bittorrent().schedual_callback(
					time, bind(&HaliteWindow::logoffCallback, this));
				break;

			case TimePickerDlg::action_shutdown:

				hal::bittorrent().schedual_callback(
					time, bind(&HaliteWindow::shutdownCallback, this));
				break;

			case TimePickerDlg::action_na:
			default:
				action_time_ = boost::posix_time::not_a_date_time;
				hal::bittorrent().schedual_cancel();
				return 0;
			}

			action_time_ = time;
			action_action_ = action;
		}
		else
		{
			action_time_ = boost::posix_time::not_a_date_time;
			hal::bittorrent().schedual_cancel();
		}
	}
	else
	{		
		hal::event_log().post(shared_ptr<hal::EventDetail>(new hal::EventMsg(L"Not a date_time")));
	}

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
	hal::event_log().post(shared_ptr<hal::EventDetail>(new hal::EventMsg(L"I tried to contact me.", hal::event_logger::info)));		

	return WM_AreYouMe_; 
}
