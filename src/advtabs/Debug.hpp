
#pragma once
 
#include <global_log.hpp>
using glb::wlog;

#include "../stdAfx.hpp"
#include "../DdxEx.hpp"
#include "../Halite.hpp"

#include "../HaliteTabPage.hpp"

class ui_signal;
class selection_manager;

class LogEdit : public CWindowImpl<LogEdit, CEdit>
{
public:
    BEGIN_MSG_MAP_EX(CEditImpl)	
    END_MSG_MAP()
	
	LogEdit() :
		editLogger(glb::wlog().attach(bind(LogEdit::log, this, _1)))
	{}
 
	void log(const std::wstring& text) 
	{
		int len = ::SendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0);
		SetSel(len, len);
		ReplaceSel(text.c_str(), false);
	}
	
private:
	boost::signals::scoped_connection editLogger;
};

class LogList : public CWindowImpl<LogList, CListBox>
{
public:
    BEGIN_MSG_MAP_EX(CListBox)	
    END_MSG_MAP()
	
	LogList() :
		listLogger(glb::wlog().attach(bind(LogList::log, this, _1)))
	{}
 
	void log(const std::wstring& text) 
	{
		AddString(text.c_str());
	}
	
private:
	boost::signals::scoped_connection listLogger;
};

class AdvDebugDialog :
	public CHalTabPageImpl<AdvDebugDialog>,
	public CDialogResize<AdvDebugDialog>,
	public CWinDataExchangeEx<AdvDebugDialog>
{
protected:
	typedef AdvDebugDialog thisClass;
	typedef CHalTabPageImpl<AdvDebugDialog> baseClass;
	typedef CDialogResize<AdvDebugDialog> resizeClass;
	
public:
	enum { IDD = IDD_ADVDEBUGLOG };	
	
	AdvDebugDialog(ui_signal& ui_sig, selection_manager& single_sel);
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
  	
	BEGIN_MSG_MAP(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)
//		COMMAND_ID_HANDLER_EX(BTNREANNOUNCE, onReannounce)
//		COMMAND_ID_HANDLER_EX(IDC_TRACKER_LOGINCHECK, onLoginCheck)
		COMMAND_RANGE_HANDLER_EX(IDC_DEBUGNONE, IDC_DEBUGDEBUG, onDebugOption)
		COMMAND_RANGE_CODE_HANDLER_EX(IDC_TRACKER_USER, IDC_TRACKER_PASS, EN_KILLFOCUS, OnEditKillFocus)
		
		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;
		
		CHAIN_MSG_MAP(resizeClass)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()
		
	BOOL DoDataExchange(BOOL bSaveAndValidate = FALSE, UINT nCtlID = (UINT)-1);

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(IDC_DEBUGEDIT, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_DEBUGLIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_DEBUGFILECHECK, DLSZ_MOVE_Y)
		
		DLGRESIZE_CONTROL(IDC_DEBUGSTATIC, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_DEBUGNONE, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_DEBUGFATAL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_DEBUGCRITICAL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_DEBUGWARNING, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_DEBUGINFO, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_DEBUGDEBUG, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()
	
	LRESULT onInitDialog(HWND, LPARAM);
	void onClose();	
	
	void onLoginCheck(UINT, int, HWND hWnd);
	LRESULT OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl);
	void onDebugOption(UINT, int, HWND);
	
	void selectionChanged(const string& torrent_name);	
	void updateDialog();
		
protected:
	
//	LogEdit logEdit;
	LogList logList;
	
	int debugLevel;
	ui_signal& ui_;
	selection_manager& selection_manager_;
};
