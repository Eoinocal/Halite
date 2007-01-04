
#pragma once
 
#include "stdAfx.hpp"
#include "DdxEx.hpp"

#include "HaliteTabCtrl.hpp"
#include "ThemeTestDialog.hpp"

class HaliteWindow;
class ThemeTestDialog;

/*class CMyTabCtrlWithDisable : public CTabCtrlWithDisable
{
public:
    BOOL IsTabEnabled(int nTab)
    {
        return (nTab != 2);
    }
};
*/
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
		DLGRESIZE_CONTROL(IDC_TAB, (DLSZ_SIZE_X | DLSZ_SIZE_Y))

	END_DLGRESIZE_MAP()
	
	LRESULT onInitDialog(HWND, LPARAM);	
	void OnSize(UINT, CSize);
	void onClose();	
	
protected:
	void InitializeControls(void);
	void InitializeValues(void);

//	CHalTabCtrl m_tabCtrl1;
	CHalTabCtrl m_tabCtrl;
	boost::scoped_ptr<ThemeTestDialog> mp_dlg;
	
	HaliteWindow* mainHaliteWindow;	
};
