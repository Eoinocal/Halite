
#include "stdAfx.hpp"
#include "HaliteWindow.hpp"

#include <string>
#include <boost/format.hpp>
#include <boost/bind.hpp>

LRESULT HaliteWindow::OnCreate(LPCREATESTRUCT lpcs)
{
	INI = new ArchivalData("Halite.ini");
	INI->LoadData();
	
	bool success = halite::initSession();
	assert(success);
	
	success = halite::listenOn(std::make_pair(INI->bitTConfig.portFrom,INI->bitTConfig.portTo));
	assert(success);	
	
	halite::setLimits(INI->bitTConfig.maxConnections,INI->bitTConfig.maxUploads);
	halite::resumeAll();
	
	if (INI->remoteConfig.isEnabled)
	{
		remote::initServer(INI->remoteConfig.port);
	}

	SetMenu(NULL);
	
	//Init ToolBar
	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	
	// Init ReBar
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	//AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
	
	// Init the StatusBar	
	m_hWndStatusBar = m_wndStatusBar.Create(*this);
	UIAddStatusBar (m_hWndStatusBar);
	
	int anPanes[] = { ID_DEFAULT_PANE, IDPANE_STATUS, IDPANE_CAPS_INDICATOR };
	m_wndStatusBar.SetPanes ( anPanes, 3, false );
	
	// Create the Splitter Control
	{
		RECT rc;
		GetClientRect(&rc);
		m_hzSplit.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		m_hWndClient = m_hzSplit.m_hWnd;
		m_hzSplit.SetSplitterPos();
	}
	
	// Create ListView and Dialog
	m_list.Create(m_hzSplit.m_hWnd, rcDefault, NULL, LVS_REPORT | LVS_SINGLESEL | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	m_hdlg.Create(m_hzSplit.m_hWnd);
	m_hdlg.ShowWindow(true);
	
	m_hzSplit.SetSplitterPanes(m_list, m_hdlg);
	
	// Add columns to ListView
	CHeaderCtrl hdr = m_list.GetHeader();
	hdr.ModifyStyle(HDS_BUTTONS, 0);

	m_list.AddColumn(L"Name", hdr.GetItemCount());
	m_list.AddColumn(L"Status", hdr.GetItemCount());
	m_list.AddColumn(L"Completed", hdr.GetItemCount());
	m_list.AddColumn(L"Download", hdr.GetItemCount());
	m_list.AddColumn(L"Upload", hdr.GetItemCount());
	m_list.AddColumn(L"Peers", hdr.GetItemCount());
	m_list.AddColumn(L"Seeds", hdr.GetItemCount());

	for (size_t i=0; i<numMainCols; ++i)
		m_list.SetColumnWidth(i, INI->haliteWindow.mainListColWidth[i]);
	
	// Add ToolBar and register it and StatusBar for UIUpdates
	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	
	SetTimer (1, 1500 );
	attachUIEvent(bind(&HaliteWindow::updateStatusbar,this));
	attachUIEvent(bind(&HaliteListViewCtrl::updateListView,&m_list));
	attachUIEvent(bind(&HaliteDialog::updateDialog,&m_hdlg));
		
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
	if (lParam->hwndFrom == m_list && lParam->code == NM_CLICK)
	{
		updateUI();	
			
		int itemPos = m_list.GetSelectionMark();
	
		if (itemPos != -1)
		{
			wchar_t filenameBuffer[256];		
			m_list.GetItemText(itemPos,0,static_cast<LPTSTR>(filenameBuffer),256);		
			m_hdlg.setSelectedTorrent(filenameBuffer);
		}
	}
	
	return 0;
}

// UI update signal and Halite Window specific functions.

void HaliteWindow::attachUIEvent(function<void ()> fn)
{
	updateUI_.connect(fn);
}
void HaliteWindow::updateUI()
{
	updateUI_();
}

void HaliteWindow::updateStatusbar()
{
	pair<float,float> speed = halite::sessionSpeed();
	UISetText (1, 
		(wformat(L"(U-D) %1$.2fkb/s - %2$.2fkb/s") 
			% (speed.first/1000) 
			% (speed.second/1000)
		).str().c_str());
}

void HaliteWindow::OnTimer ( UINT uTimerID, TIMERPROC pTimerProc )
{
	if ( 1 == uTimerID ) 
	{
		updateUI();
	}
	else if ( 2 == uTimerID )
	{
		halite::reannounceAll();
	}
	else {		
		SetMsgHandled(false);
	}
}	

void HaliteWindow::OnClose()
{
	GetWindowRect(INI->haliteWindow.rect);
	INI->haliteWindow.splitterPos = m_hzSplit.GetSplitterPos() + 101;
	
//	::MessageBoxA(0,lexical_cast<string>(INI->haliteWindow.splitterPos).c_str(),"Error",0);

	for (size_t i=0; i<numMainCols; ++i)
		INI->haliteWindow.mainListColWidth[i] = m_list.GetColumnWidth(i);
	
	INI->SaveData();
	
	SetMsgHandled(false);
}	

LRESULT HaliteWindow::OnFileOpen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CSSFileDialog dlgOpen(TRUE, NULL, NULL, OFN_HIDEREADONLY, L"Torrents (*.*)|*.torrent|", m_hWnd);

	if (dlgOpen.DoModal() == IDOK) 
	{
		wstring filename = dlgOpen.m_ofn.lpstrFile;
		halite::addTorrent(path(halite::wcstombs(filename),native));
	}
	updateUI();
		
	return 0;
}	

LRESULT HaliteWindow::OnSettings(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ConfigOptionsProp sheet(L"Settings");	
    sheet.DoModal();
	
	halite::listenOn(std::make_pair(INI->bitTConfig.portFrom,INI->bitTConfig.portTo));
	
	if (INI->remoteConfig.isEnabled)
	{
		remote::initServer(INI->remoteConfig.port);
	}
	else
	{
		remote::exitServer();
	}
	return 0;
}

LRESULT HaliteWindow::OnPauseAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	halite::pauseTorrents();
	updateUI();
	return 0;
}

LRESULT HaliteWindow::OnResumeAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	halite::resumeTorrents();
	updateUI();
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

