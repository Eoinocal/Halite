
#pragma once
 
#include "../stdAfx.hpp"
#include "../DdxEx.hpp"
#include "../Halite.hpp"

#include "../HaliteTabPage.hpp"

class ui_signal;
class selection_manager;

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
	
	AdvTrackerDialog(ui_signal& ui_sig, selection_manager& single_sel);
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
  	
	BEGIN_MSG_MAP(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)
		COMMAND_ID_HANDLER_EX(BTNREANNOUNCE, onReannounce)
		COMMAND_ID_HANDLER_EX(IDC_TRACKER_LOGINCHECK, onLoginCheck)
		COMMAND_RANGE_CODE_HANDLER_EX(IDC_TRACKER_USER, IDC_TRACKER_PASS, EN_KILLFOCUS, OnEditKillFocus)
		
		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;
		
		CHAIN_MSG_MAP(resizeClass)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()
	
	BEGIN_DDX_MAP(thisClass)
		DDX_EX_STDWSTRING(IDC_TRACKER_USER, username_);
		DDX_EX_STDWSTRING(IDC_TRACKER_PASS, password_);
    END_DDX_MAP()
	
	BEGIN_DLGRESIZE_MAP(thisClass)		
		DLGRESIZE_CONTROL(IDC_TRACKERLIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		
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
	
	void setLoginUiState(const string& torrent_name);	
	void selectionChanged(const string& torrent_name);	
	void updateDialog();
		
protected:	
	ui_signal& ui_;
	selection_manager& selection_manager_;
	
	wstring username_;
	wstring password_;
};
