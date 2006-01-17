
#pragma once

#include "stdafx.h"
#include "Halite.h"
#include "GlobalIni.h"

using namespace std;
using namespace boost;
	
#define WM_UPDATEUIINFO  WM_USER+13
const size_t numMainCols = 7;

#include "HaliteDialog.hpp"
#include "ConfigOptions.hpp"

class HaliteWindow : 
	public CFrameWindowImpl<HaliteWindow>,
	public CUpdateUI<HaliteWindow>,
	public CMessageFilter,
	public CIdleHandler
{
public:	

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CFrameWindowImpl<HaliteWindow>::PreTranslateMessage(pMsg))
			return TRUE;

		return m_hdlg.PreTranslateMessage(pMsg);
	}
	
	virtual BOOL OnIdle()
	{
		UIUpdateStatusBar();
		UIUpdateToolBar();
		return FALSE;
	}
	
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME);
	
	BEGIN_MSG_MAP(HaliteWindow)
		MESSAGE_HANDLER(WM_UPDATEUIINFO, OnUpdateUIInfo)
		MSG_WM_NOTIFY(OnNotify)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_TIMER(OnTimer)
		COMMAND_ID_HANDLER(ID_RESUME, OnResumeAll)
		COMMAND_ID_HANDLER(ID_PAUSE, OnPauseAll)
		COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER(ID_SETTINGS, OnSettings)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		CHAIN_MSG_MAP(CUpdateUI<HaliteWindow>)
		CHAIN_MSG_MAP(CFrameWindowImpl<HaliteWindow>)
	END_MSG_MAP()
	
	BEGIN_UPDATE_UI_MAP(HaliteWindow)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(1, UPDUI_STATUSBAR)
        UPDATE_ELEMENT(2, UPDUI_STATUSBAR)
	END_UPDATE_UI_MAP()
	
	LRESULT OnUpdateUIInfo(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT HaliteWindow::OnNotify(int wParam, LPNMHDR lParam);
	LRESULT OnCreate(LPCREATESTRUCT lpcs);	
    void OnTimer(UINT uTimerID, TIMERPROC pTimerProc);
	void OnClose();
	LRESULT OnResumeAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPauseAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSettings(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFileOpen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnViewStatusBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEraseBkgnd(HDC hdc);
	
protected:	
	CCommandBarCtrl m_CmdBar;
	CHorSplitterWindow m_hzSplit;
	CListViewCtrl m_list;
	CEdit m_edit;
	HaliteDialog m_hdlg;
    CMultiPaneStatusBarCtrl m_wndStatusBar;
};
