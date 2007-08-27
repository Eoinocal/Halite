
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <boost/format.hpp>
#include <boost/array.hpp>

#include "stdAfx.hpp"
#include "AdvHaliteDialog.hpp"

#include "advtabs/ThemeTestDialog.hpp"

LRESULT AdvHaliteDialog::onInitDialog(HWND, LPARAM)
{	
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
	SetDlgItemText(IDC_ADVDLG_VERSION, hal::app().res_wstr(HAL_VERSION_STRING).c_str());
	
	m_tabCtrl.SubclassWindow(GetDlgItem(IDC_TAB));
	
/*	mp_dlg.reset(new ThemeTestDialog());
	mp_dlg->Create(m_tabCtrl);
	
	m_tabCtrl.AddPage(*mp_dlg, L"Test1");
*/		
	m_torrent.Create(m_tabCtrl);
	m_tabCtrl.AddPage(m_torrent, hal::app().res_wstr(HAL_TORRENT_TAB).c_str());
	
	m_peers.Create(m_tabCtrl);
	m_tabCtrl.AddPage(m_peers, hal::app().res_wstr(HAL_PEERS_TAB).c_str());
	
	m_files.Create(m_tabCtrl);
	m_tabCtrl.AddPage(m_files, hal::app().res_wstr(HAL_FILES_TAB).c_str());
	
	m_tracker.Create(m_tabCtrl);
	m_tabCtrl.AddPage(m_tracker, hal::app().res_wstr(HAL_TRACKER_TAB).c_str());
	
	m_debug.Create(m_tabCtrl);
	m_tabCtrl.AddPage(m_debug, hal::app().res_wstr(HAL_DEBUG_TAB).c_str());
	
	m_tabCtrl.SetCurrentPage(0);
	
	return 0;
}

void AdvHaliteDialog::OnSize(UINT type, CSize)
{
	RECT rect;
	m_tabCtrl.GetClientRect(&rect);
	m_tabCtrl.AdjustRect(false, &rect);
	
//	mp_dlg->SetWindowPos(HWND_TOP, 0, 0, rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE);
	
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
