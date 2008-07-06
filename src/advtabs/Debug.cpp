
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include "../HaliteWindow.hpp"
#include "../HaliteListViewCtrl.hpp"

#include "Debug.hpp"

void AdvDebugDialog::onLoginCheck(UINT, int, HWND hWnd)
{
	LRESULT result = ::SendMessage(hWnd, BM_GETCHECK, 0, 0);
	
	if (result == BST_CHECKED)
	{
		::EnableWindow(GetDlgItem(HAL_TRACKER_USER), true);
		::EnableWindow(GetDlgItem(HAL_TRACKER_PASS), true);
	}
	else
	{
		::EnableWindow(GetDlgItem(HAL_TRACKER_USER), false);
		::EnableWindow(GetDlgItem(HAL_TRACKER_PASS), false);
	}
}

LRESULT AdvDebugDialog::onInitDialog(HWND, LPARAM)
{
	logList.Attach(GetDlgItem(HAL_DEBUGLISTVIEW));

	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);	
	DoDataExchange(false);
	
	return 0;
}

BOOL AdvDebugDialog::DoDataExchange(BOOL bSaveAndValidate, UINT nCtlID)
{	
	DDX_CHECK(HAL_DEBUGFILECHECK, halite().logToFile_)
	DDX_CHECK(HAL_DEBUGDEBUGCHECK, halite().logDebug_)

	return TRUE;
}

void AdvDebugDialog::onClose()
{	
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

void AdvDebugDialog::onDebugOption(UINT, int, HWND)
{
	DoDataExchange(true);
}
