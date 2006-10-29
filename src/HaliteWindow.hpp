
#pragma once

#include "stdAfx.hpp"
#include "DropFileTarget.h"
#include "NTray.hpp"

#include <boost/signals.hpp>

class HaliteListViewCtrl;
class HaliteDialog;

class HaliteWindow : 
	public CFrameWindowImpl<HaliteWindow>,
	public CUpdateUI<HaliteWindow>,
	public CDropFileTarget<HaliteWindow>,
	public CMessageFilter,
	public CIdleHandler
{
public:	
	HaliteWindow();
	~HaliteWindow();
	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	
	virtual BOOL OnIdle()
	{
		UIUpdateStatusBar();
		UIUpdateToolBar();
		return FALSE;
	}
	
	enum { 
		ID_UPDATE_TIMER = 1,
		WM_TRAYNOTIFY = WM_USER+123
	};
	
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME);
	
	BEGIN_MSG_MAP(HaliteWindow)
		MSG_WM_NOTIFY(OnNotify)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_MOVE(OnMove)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_TIMER(OnTimer)		
		MESSAGE_HANDLER_EX(WM_TRAYNOTIFY, OnTrayNotification)
		COMMAND_ID_HANDLER(ID_RESUME, OnResumeAll)
		COMMAND_ID_HANDLER(ID_PAUSE, OnPauseAll)
		COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER(ID_SETTINGS, OnSettings)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_TRAY_OPENHALITE, OnTrayOpenHalite)
		COMMAND_ID_HANDLER(ID_TRAY_EXIT, OnTrayExit)
		CHAIN_MSG_MAP(CUpdateUI<HaliteWindow>)
		CHAIN_MSG_MAP(CFrameWindowImpl<HaliteWindow>)
		CHAIN_MSG_MAP(CDropFileTarget<HaliteWindow>)
	END_MSG_MAP()
	
	BEGIN_UPDATE_UI_MAP(HaliteWindow)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDR_TRAY_MENU, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(0, UPDUI_STATUSBAR)
        UPDATE_ELEMENT(1, UPDUI_STATUSBAR)
        UPDATE_ELEMENT(2, UPDUI_STATUSBAR)
	END_UPDATE_UI_MAP()
	
	LRESULT OnUpdateUIInfo(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT HaliteWindow::OnNotify(int wParam, LPNMHDR lParam);
	LRESULT OnCreate(LPCREATESTRUCT lpcs);	
    void OnTimer(UINT uTimerID, TIMERPROC pTimerProc);
	void OnClose();
	void OnSize(UINT, CSize);
	void OnMove(CSize);
	LRESULT OnTrayNotification(UINT, WPARAM wParam, LPARAM lParam);
	LRESULT OnResumeAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnTrayOpenHalite(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnTrayExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPauseAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSettings(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFileOpen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnViewStatusBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEraseBkgnd(HDC hdc);
	
	void attachUIEvent(boost::function<void ()> fn) { updateUI_.connect(fn); }
	void updateUI() { updateUI_(); }
	
	void updateWindow();
	void ProcessFile(LPCTSTR lpszPath);
	
	friend HaliteDialog;
	friend HaliteListViewCtrl;
	
protected:	
	CCommandBarCtrl m_CmdBar;
	CHorSplitterWindow m_Split;
    CMultiPaneStatusBarCtrl m_StatusBar;
	CTrayNotifyIcon m_trayIcon;
	
	boost::scoped_ptr<HaliteListViewCtrl> mp_list;
	boost::scoped_ptr<HaliteDialog> mp_dlg;
	
	boost::signal<void ()> updateUI_;	
};
