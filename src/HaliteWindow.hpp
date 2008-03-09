
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define ID_WINDOW_BEGIN	 			13000
#define HAL_WINDOW_CLOSECONFRIM		ID_WINDOW_BEGIN + 1
#define HAL_WINDOW_SOCKETS_FAILED	ID_WINDOW_BEGIN + 2

#ifndef RC_INVOKED

#include "stdAfx.hpp"
#include "DropFileTarget.h"
#include "NTray.hpp"
#include "HaliteListView.hpp"
#include "halIni.hpp"
#include "halTorrent.hpp"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/signals.hpp>

class HaliteDialog;
class AdvHaliteDialog;

class ui_signal : noncopyable
{
public:
	void attach(boost::function<void ()> fn) { ui_.connect(fn); }
	void update() {  }

private:
	boost::signal<void ()> ui_;
};

class HaliteWindow :
	public WTL::CFrameWindowImpl<HaliteWindow>,
	public WTL::CUpdateUI<HaliteWindow>,
	public CDropFileTarget<HaliteWindow>,
	public WTL::CMessageFilter,
	public WTL::CIdleHandler,
	public hal::IniBase<HaliteWindow>,
	private boost::noncopyable
{
public:
	HaliteWindow(unsigned ARE_YOU_ME);
	~HaliteWindow();

	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME);

	ui_signal& ui() { return ui_; }

	BEGIN_MSG_MAP_EX(HaliteWindow)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_MOVE(OnMove)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_COPYDATA(OnCopyData);
		MESSAGE_HANDLER(WM_AreYouMe_, OnAreYouMe)
		MESSAGE_HANDLER_EX(WM_TRAYNOTIFY, OnTrayNotification)	
		
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		//MSG_WM_PAINT(OnPaint)

		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER(ID_RESUME, OnResumeAll)
		COMMAND_ID_HANDLER(ID_PAUSE, OnPauseAll)
		COMMAND_ID_HANDLER(ID_SETTINGS, OnSettings)
		COMMAND_ID_HANDLER(ID_ADVANCED, OnAdvanced)
		COMMAND_ID_HANDLER(ID_HELP, OnHelp)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)

		COMMAND_ID_HANDLER(ID_TRAY_OPENHALITE, OnTrayOpenHalite)
		COMMAND_ID_HANDLER(ID_TRAY_RESUMEALL, OnResumeAll)
		COMMAND_ID_HANDLER(ID_TRAY_PAUSEALL, OnPauseAll)
		COMMAND_ID_HANDLER(ID_TRAY_EXIT, OnTrayExit)

		REFLECT_NOTIFICATIONS()
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
        UPDATE_ELEMENT(3, UPDUI_STATUSBAR)
	END_UPDATE_UI_MAP()

	void ProcessFile(LPCTSTR lpszPath);
	
	boost::signals::connection connectUiUpdate(boost::function<void (const hal::TorrentDetails& allTorrents)> fn) 
	{ 
		return ui_update_signal_.connect(fn); 
	}
	
	boost::signal<void (const hal::TorrentDetails& allTorrents)> & ui_sig()
	{ 
		return ui_update_signal_; 
	}
	
	void issueUiUpdate();
	
	void connectSaveState(boost::function<void ()> fn) 
	{ 
		save_state_signal_.connect(fn); 
	}
	
	HaliteListViewCtrl& torrentsList() { return haliteList; }
	LRESULT OnAreYouMe(UINT, WPARAM, LPARAM, BOOL&);
	
protected:
	typedef hal::IniBase<HaliteWindow> iniClass;

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual BOOL OnIdle()
	{
		UIUpdateStatusBar();
		UIUpdateToolBar();
		return FALSE;
	}

	enum {
		ID_UPDATE_TIMER = 1,
		ID_SAVE_TIMER,
		WM_TRAYNOTIFY = WM_USER+123
	};

	LRESULT OnUpdateUIInfo(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT HaliteWindow::OnNotify(int wParam, LPNMHDR lParam);
	LRESULT OnCreate(LPCREATESTRUCT lpcs);
    void OnTimer(UINT uTimerID);
	void OnClose();
	void OnDestroy();
	void OnSize(UINT, WTL::CSize);
	void OnMove(WTL::CSize);
	void OnShowWindow(BOOL bShow, UINT nStatus);
	LRESULT OnTrayNotification(UINT, WPARAM wParam, LPARAM lParam);
	LRESULT OnResumeAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnTrayOpenHalite(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnTrayExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFileNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFileOpen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPauseAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSettings(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAdvanced(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnViewStatusBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEraseBkgnd(HDC hdc);
	LRESULT OnPaint(HDC dc);

	LRESULT OnCopyData(HWND, PCOPYDATASTRUCT pCSD);

	void updateWindow();
	void setCorrectDialog();

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(rect);
		ar & BOOST_SERIALIZATION_NVP(splitterPos);
		ar & BOOST_SERIALIZATION_NVP(use_tray);
		ar & BOOST_SERIALIZATION_NVP(advancedUI);
		ar & BOOST_SERIALIZATION_NVP(activeTab);
		if (version > 0) {
			ar & BOOST_SERIALIZATION_NVP(closeToTray);
			ar & BOOST_SERIALIZATION_NVP(confirmClose);
		}
	}
	
	friend class GeneralOptions;

private:
	// These two gotta be first!!!
	boost::signal<void (const hal::TorrentDetails& tD)> ui_update_signal_;
	boost::signal<void ()> save_state_signal_;	
	
	WTL::CCommandBarCtrl m_CmdBar;
	WTL::CHorSplitterWindow m_Split;
    WTL::CMultiPaneStatusBarCtrl m_StatusBar;
	CTrayNotifyIcon trayIcon_;

	HaliteListViewCtrl haliteList;
	
	boost::scoped_ptr<HaliteDialog> mp_dlg;
	boost::scoped_ptr<AdvHaliteDialog> mp_advDlg;

	bool whichSimple_;

	ui_signal ui_;

	unsigned WM_AreYouMe_;

	WTL::CRect rect;
	unsigned int splitterPos;
	bool use_tray;
	bool advancedUI;
	bool closeToTray;
	bool confirmClose;
	int activeTab;
};

BOOST_CLASS_VERSION(HaliteWindow, 1)

#endif // RC_INVOKED
