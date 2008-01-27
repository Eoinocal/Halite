
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define IDD_NEWTORRENT_BEGIN			1950
//#define IDD_PROGRESS                    1901
//#define IDC_PROG_CANCEL                 IDD_NEWTORRENT_BEGIN + 2
//#define IDC_PROG_PROGRESS               IDD_NEWTORRENT_BEGIN + 3

#ifndef RC_INVOKED

#include <boost/function.hpp>
#include "halTorrent.hpp"

class NewTorrentDialog :
	public CDialogImpl<NewTorrentDialog>
{
protected:
	typedef NewTorrentDialog thisClass;
	typedef CDialogImpl<thisClass> baseClass;

public:
	thisClass()
	{}
	
	enum { IDD = IDD_NEWTORRENT };

    BEGIN_MSG_MAP_EX(thisClass)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
//		COMMAND_ID_HANDLER_EX(IDC_PROG_CANCEL, onCancel)
    END_MSG_MAP()
	
	LRESULT OnInitDialog(...)
	{
		CenterWindow();
/*		SetWindowText(windowText_.c_str());
		prog_.Attach(GetDlgItem(IDC_PROG_PROGRESS));
		prog_.SetRange(0, 100);
		
		thread_ptr.reset(new thread(bind(&ProgressDialog::ProgressThread, this)));
*/		
		return TRUE;
	}

	void ProgressThread()
	{
//		fn_(bind(&ProgressDialog::Callback, this, _1));
		
//		EndDialog(0);
	}
	
	bool Callback(size_t progress)
	{
//		prog_.SetPos(progress);
		
		return true;
	}
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
//		return this->IsDialogMessage(pMsg);
	}
	
	void onCancel(UINT, int, HWND hWnd)
	{
//		stop_ = true;
	}
	
private:

};

#endif // RC_INVOKED
