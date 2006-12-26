
#pragma once

#include "stdAfx.hpp"
#include "DropFileTarget.h"
#include "NTray.hpp"

#include <boost/array.hpp>
#include <boost/signals.hpp>

class HaliteListViewCtrl;
class HaliteDialog;
class AdvHaliteDialog;

class ui_signal : noncopyable
{
public:
	void attach(boost::function<void ()> fn) { ui_.connect(fn); }
	void update() { ui_(); }
	
private:
	boost::signal<void ()> ui_;
};

class single_selection_manager : noncopyable
{
public:
	single_selection_manager(boost::shared_ptr<HaliteListViewCtrl> p_list) :
		mp_list_(p_list)
	{}
	typedef const string& param_type;
	
	void sync_with_list(bool list_to_manager);
	
	param_type selected() const { return selected_; }
	
	void setSelected(const string& sel) 
	{
		selected_ = sel;
		sync_with_list(false);
	}
	
	void setSelected(int itemPos);
	
	void clear();
	
	void attach(boost::function<void (param_type)> fn) { selection_.connect(fn); }
	void signal() { selection_(selected_); }
	
private:
	string selected_;	
	boost::signal<void (param_type)> selection_;
	boost::shared_ptr<HaliteListViewCtrl> mp_list_;
};

class config_signal
{
public:
	void attach(boost::function<void (bool)> fn) { config_.connect(fn); }
	void update(bool firstTime) { config_(firstTime); }
	
private:
	boost::signal<void (bool)> config_;
};

class HaliteWindow : 
	public CFrameWindowImpl<HaliteWindow>,
	public CUpdateUI<HaliteWindow>,
	public CDropFileTarget<HaliteWindow>,
	public CMessageFilter,
	public CIdleHandler,
	private noncopyable
{
public:	
	HaliteWindow(unsigned ARE_YOU_ME);
	~HaliteWindow();

	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME);
	
	ui_signal& ui() { return ui_; }
	single_selection_manager& selection() { return single_selection_manager_; }

	BEGIN_MSG_MAP(HaliteWindow)
		MSG_WM_NOTIFY(OnNotify)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_MOVE(OnMove)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_TIMER(OnTimer)	
		MSG_WM_COPYDATA(OnCopyData);
		MESSAGE_HANDLER(WM_AreYouMe_, OnAreYouMe)	
		MESSAGE_HANDLER_EX(WM_TRAYNOTIFY, OnTrayNotification)
		
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER(ID_RESUME, OnResumeAll)
		COMMAND_ID_HANDLER(ID_PAUSE, OnPauseAll)
		COMMAND_ID_HANDLER(ID_SETTINGS, OnSettings)
		COMMAND_ID_HANDLER(ID_HELP, OnHelp)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_TRAY_OPENHALITE, OnTrayOpenHalite)
		COMMAND_ID_HANDLER(ID_TRAY_EXIT, OnTrayExit)
		
		CHAIN_MSG_MAP(CUpdateUI<HaliteWindow>)
		CHAIN_MSG_MAP(CFrameWindowImpl<HaliteWindow>)
		CHAIN_MSG_MAP(CDropFileTarget<HaliteWindow>)
	END_MSG_MAP()
	
	BEGIN_UPDATE_UI_MAP(HaliteWindow)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(IDR_TRAY_MENU, UPDUI_MENUPOPUP)
		
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(0, UPDUI_STATUSBAR)
        UPDATE_ELEMENT(1, UPDUI_STATUSBAR)
        UPDATE_ELEMENT(2, UPDUI_STATUSBAR)
	END_UPDATE_UI_MAP()
	
	void ProcessFile(LPCTSTR lpszPath);

protected:
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
	LRESULT OnFileNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFileOpen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPauseAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSettings(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	LRESULT OnViewStatusBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEraseBkgnd(HDC hdc);
	
	LRESULT OnCopyData(HWND, PCOPYDATASTRUCT pCSD);
	LRESULT OnAreYouMe(UINT, WPARAM, LPARAM, BOOL&) { return WM_AreYouMe_; }
	
	void updateWindow();
	
//	const string& getSelected() const;
//	void setSelected(int index);
//	void selectionChanged();
//	void clearSelected();

private:		
	CCommandBarCtrl m_CmdBar;
	CHorSplitterWindow m_Split;
    CMultiPaneStatusBarCtrl m_StatusBar;
	CTrayNotifyIcon m_trayIcon;
	
	boost::shared_ptr<HaliteListViewCtrl> mp_list;
	boost::scoped_ptr<HaliteDialog> mp_dlg;
	boost::scoped_ptr<AdvHaliteDialog> mp_advDlg;

	ui_signal ui_;
	config_signal config_;
	single_selection_manager single_selection_manager_;
	
//	boost::signal<void ()> updateSelection_;
	
//	string selectedTorrent_;
	unsigned WM_AreYouMe_;
	
	void updateConfigSettings();
};
