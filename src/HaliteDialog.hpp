
#pragma once
 
#include "stdAfx.hpp"
#include "Halite.hpp"

class HaliteWindow;
 
class HaliteDialog :
	public CDialogImpl<HaliteDialog>,
	public CDialogResize<HaliteDialog>,
	public CWinDataExchange<HaliteDialog>
{
protected:
	typedef HaliteDialog thisClass;
	typedef CDialogImpl<HaliteDialog> baseClass;
	typedef CDialogResize<HaliteDialog> resizeClass;

	CButton m_btn_start;
	CListViewCtrl m_list;
	CContainedWindow m_wndNCD;
	CProgressBarCtrl m_prog;
	
	int NoConnDown, NoConnUp;
	float TranLimitDown, TranLimitUp;
	
	std::string selectedTorrent;
	HaliteWindow* mainHaliteWindow;

public:
	enum { IDD = IDD_HALITEDLG };
	
	HaliteDialog(HaliteWindow* halWnd)
		: mainHaliteWindow(halWnd)
	{}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
  	
	BEGIN_MSG_MAP(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)	
		COMMAND_RANGE_CODE_HANDLER_EX(IDC_EDITTLU, IDC_EDITNCU, EN_KILLFOCUS, OnEditKillFocus)
		
		if(uMsg == WM_FORWARDMSG)
			if(PreTranslateMessage((LPMSG)lParam)) return TRUE;

		COMMAND_ID_HANDLER_EX(BTNPAUSE, onPause)		
		COMMAND_ID_HANDLER_EX(BTNREANNOUNCE, onReannounce)
		COMMAND_ID_HANDLER_EX(BTNREMOVE, onRemove)
		
		CHAIN_MSG_MAP(resizeClass)
	END_MSG_MAP()
	
	BEGIN_DDX_MAP(CMainDlg)
        DDX_INT(IDC_EDITNCD, NoConnDown)
        DDX_INT(IDC_EDITNCU, NoConnUp)
        DDX_FLOAT(IDC_EDITTLD, TranLimitDown)
        DDX_FLOAT(IDC_EDITTLU, TranLimitUp)
    END_DDX_MAP()

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(BTNPAUSE, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(BTNREANNOUNCE, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(BTNREMOVE, (DLSZ_MOVE_X))
		
		DLGRESIZE_CONTROL(IDC_TL, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_NC, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_TLD, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_TLU, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_NCU, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_NCD, (DLSZ_MOVE_X))
		
		DLGRESIZE_CONTROL(IDC_EDITNCD, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_EDITTLD, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_EDITTLU, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_EDITNCU, (DLSZ_MOVE_X))
		
		DLGRESIZE_CONTROL(TORRENTPROG, (DLSZ_SIZE_X))
		
		DLGRESIZE_CONTROL(IDC_NAME, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(IDC_TRACKER, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(IDC_STATUS, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(IDC_COMPLETE, (DLSZ_SIZE_X))
		
		DLGRESIZE_CONTROL(LISTPEERS, (DLSZ_SIZE_X | DLSZ_SIZE_Y))
		DLGRESIZE_CONTROL(IDC_DETAILS_GROUP, (DLSZ_SIZE_X | DLSZ_SIZE_Y))
	END_DLGRESIZE_MAP()
	
	LRESULT onInitDialog(HWND, LPARAM);	
	void onClose();	
	
	void onPause(UINT, int, HWND);	
	void onReannounce(UINT, int, HWND);	
	void onRemove(UINT, int, HWND);
	
	LRESULT OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl );
	
	void setSelectedTorrent(string torrent);	
	void updateDialog();
	void saveStatus();
				
protected:
	void InitializeControls(void);
	void InitializeValues(void);
};
