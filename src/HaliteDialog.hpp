#pragma once
 
#include "stdAfx.hpp"
#include "Halite.hpp"

using namespace std;
using namespace boost;

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
	
	float NoConnDown, NoConnUp;
	float TranLimitDown, TranLimitUp;
	
	wstring selectedTorrent;
	HaliteWindow* mainHaliteWindow;

public:
	enum { IDD = HALITEDLG };
	
	HaliteDialog(HaliteWindow* halWnd)
		: mainHaliteWindow(halWnd)
	{}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
  				
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)		
		COMMAND_RANGE_CODE_HANDLER_EX(IDC_EDITTLU, IDC_EDITNCU, EN_KILLFOCUS, OnEditKillFocus)
		
		if(uMsg == WM_FORWARDMSG)
			if(PreTranslateMessage((LPMSG)lParam)) return TRUE;

		COMMAND_ID_HANDLER(BTNPAUSE, OnPause)		
		COMMAND_ID_HANDLER(BTNREANNOUNCE, OnReannounce)
		COMMAND_ID_HANDLER(BTNREMOVE, OnRemove)
		
		CHAIN_MSG_MAP(resizeClass)
	END_MSG_MAP()
	
	BEGIN_DDX_MAP(CMainDlg)
        DDX_FLOAT(IDC_EDITNCD, NoConnDown)
        DDX_FLOAT(IDC_EDITNCU, NoConnUp)
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
		DLGRESIZE_CONTROL(IDC_AVAIL, (DLSZ_SIZE_X))
		
		DLGRESIZE_CONTROL(LISTPEERS, (DLSZ_SIZE_X | DLSZ_SIZE_Y))
	END_DLGRESIZE_MAP()
	
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);	
	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&);	
	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&);	
	
	LRESULT OnPause(WORD, WORD, HWND, BOOL&);	
	LRESULT OnReannounce(WORD, WORD, HWND, BOOL&);	
	LRESULT OnRemove(WORD, WORD, HWND, BOOL&);	;
	
	LRESULT OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl );
	
	void setSelectedTorrent(wstring torrent);	
	void updateDialog();
				
protected:
	void InitializeControls(void);
	void InitializeValues(void);
};
