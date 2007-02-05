
#pragma once
 
#include "../stdAfx.hpp"
#include "../global/string_conv.hpp"

#include "../DdxEx.hpp"
#include "../Halite.hpp"

#include "TrackerListView.hpp"
#include "../HaliteTabPage.hpp"
#include "../HaliteListManager.hpp"

class ui_signal;
class HaliteListViewCtrl;
class CHaliteListViewCtrl;

typedef selection_manager<CHaliteListViewCtrl<HaliteListViewCtrl> > ListViewManager;

class AdvTrackerDialog :
	public CHalTabPageImpl<AdvTrackerDialog>,
	public CDialogResize<AdvTrackerDialog>,
	public CWinDataExchangeEx<AdvTrackerDialog>
{
protected:
	typedef AdvTrackerDialog thisClass;
	typedef CHalTabPageImpl<AdvTrackerDialog> baseClass;
	typedef CDialogResize<AdvTrackerDialog> resizeClass;
	
public:
	enum { IDD = IDD_ADVTRACKER };	
	
	AdvTrackerDialog(ui_signal& ui_sig, ListViewManager& single_sel);
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
  	
	BEGIN_MSG_MAP(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)
		
		COMMAND_ID_HANDLER_EX(BTNREANNOUNCE, onReannounce)
		COMMAND_ID_HANDLER_EX(IDC_TRACKER_LOGINCHECK, onLoginCheck)
		COMMAND_ID_HANDLER_EX(IDC_TRACKER_RESET, onReset)
		COMMAND_ID_HANDLER_EX(IDC_TRACKER_APPLY, onApply)
		
		COMMAND_RANGE_CODE_HANDLER_EX(IDC_TRACKER_USER, IDC_TRACKER_PASS, EN_KILLFOCUS, OnEditKillFocus)
		
		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;
		
		CHAIN_MSG_MAP(resizeClass)
		CHAIN_MSG_MAP(baseClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
	
	BEGIN_DDX_MAP(thisClass)
		DDX_EX_STDWSTRING(IDC_TRACKER_USER, username_);
		DDX_EX_STDWSTRING(IDC_TRACKER_PASS, password_);
    END_DDX_MAP()
	
	BEGIN_DLGRESIZE_MAP(thisClass)		
		DLGRESIZE_CONTROL(IDC_TRACKERLIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_TRACKER_RESET, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_TRACKER_APPLY, DLSZ_MOVE_X)
		
		DLGRESIZE_CONTROL(IDC_TRACKER_LOGINCHECK, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_TRACKER_USER_S, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_TRACKER_USER, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_TRACKER_PASS_S, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_TRACKER_PASS, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(BTNREANNOUNCE, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()
	
	LRESULT onInitDialog(HWND, LPARAM);
	void onClose();	
	
	void onLoginCheck(UINT, int, HWND hWnd);
	LRESULT OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl);
	void onReannounce(UINT, int, HWND);
	
	void onApply(UINT, int, HWND);	
	void onReset(UINT, int, HWND);
	
	void setLoginUiState(const string& torrent_name);
	void selectionChanged(const string& torrent_name);
	void updateDialog();
	
	void trackerListEdited();
	
protected:	
	TrackerListViewCtrl m_list;
	
	ui_signal& ui_;
	ListViewManager& selection_manager_;
	
	wstring username_;
	wstring password_;
};
