
#include <algorithm>
#include <boost/format.hpp>
#include <boost/array.hpp>

#include "stdAfx.hpp"
#include "AdvHaliteDialog.hpp"

#include "ThemeTestDialog.hpp"

#include "GlobalIni.hpp"
#include "ini/Dialog.hpp"


LRESULT AdvHaliteDialog::onInitDialog(HWND, LPARAM)
{	
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
	
	m_tabCtrl.SubclassWindow(GetDlgItem(IDC_TAB));
	
	TCITEM tie = { TCIF_TEXT, 0, 0, L"Overview", 0, -1, 0 };
	m_tabCtrl.InsertItem(0, &tie);
	
	mp_dlg.reset(new ThemeTestDialog());
	
	mp_dlg->Create(m_tabCtrl);
	mp_dlg->ShowWindow(true);	
	
	RECT rect;
	m_tabCtrl.GetClientRect(&rect);
	m_tabCtrl.AdjustRect(false, &rect);
	
	mp_dlg->SetWindowPos(HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE);
	
	return 0;
}

void AdvHaliteDialog::OnSize(UINT type, CSize)
{
	RECT rect;
	m_tabCtrl.GetClientRect(&rect);
	m_tabCtrl.AdjustRect(false, &rect);
	
	mp_dlg->SetWindowPos(HWND_TOP, 0, 0, rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE);
	
	SetMsgHandled(false);
}	

void AdvHaliteDialog::onClose()
{
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

/*void AdvHaliteDialog::updateDialog()
{

}*/
