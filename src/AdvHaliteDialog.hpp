
#pragma once
 
#include "stdAfx.hpp"
#include "DdxEx.hpp"

#include "uxtheme.h"

#include "HalTabCtrl.hpp"
#include "ThemeTestDialog.hpp"

class HaliteWindow;
class ThemeTestDialog;

class AdvHaliteDialog :
	public CDialogImpl<AdvHaliteDialog>,
	public CDialogResize<AdvHaliteDialog>
{
protected:
	typedef AdvHaliteDialog thisClass;
	typedef CDialogImpl<AdvHaliteDialog> baseClass;
	typedef CDialogResize<AdvHaliteDialog> resizeClass;
public:
	enum { IDD = IDD_ADVHALITEDLG };
	
	AdvHaliteDialog(HaliteWindow* halWnd)
		: mainHaliteWindow(halWnd)
	{}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
  	
	BEGIN_MSG_MAP(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)	
		MSG_WM_SIZE(OnSize)
		
		
		if(uMsg == WM_FORWARDMSG)
			if(PreTranslateMessage((LPMSG)lParam)) return TRUE;
		
		CHAIN_MSG_MAP(resizeClass)
	END_MSG_MAP()
	


	BEGIN_DLGRESIZE_MAP(thisClass)
/*		DLGRESIZE_CONTROL(BTNPAUSE, (DLSZ_MOVE_X))
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
		
		DLGRESIZE_CONTROL(LISTPEERS, (DLSZ_SIZE_X | DLSZ_SIZE_Y))*/
		DLGRESIZE_CONTROL(IDC_TAB, (DLSZ_SIZE_X | DLSZ_SIZE_Y))

	END_DLGRESIZE_MAP()
	
	LRESULT onInitDialog(HWND, LPARAM);	
	void OnSize(UINT, CSize);
	void onClose();	
	
//	void selectionChanged();	
//	void updateDialog();
//	void saveStatus();
	
protected:
	void InitializeControls(void);
	void InitializeValues(void);

	CTabCtrl m_tabCtrl;
	boost::scoped_ptr<ThemeTestDialog> mp_dlg;
	
	HaliteWindow* mainHaliteWindow;	
};
