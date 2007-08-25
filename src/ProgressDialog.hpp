
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/function.hpp>
#include "halTorrent.hpp"

class ProgressDialog :
	public CDialogImpl<ProgressDialog>
{
protected:
	typedef ProgressDialog thisClass;
	typedef CDialogImpl<ProgressDialog> baseClass;
	
	typedef boost::function<void (hal::progressCallback fn)> threadFunction;

public:
	ProgressDialog(wstring windowText, threadFunction fn) :
		fn_(fn),
		windowText_(windowText),
		stop_(false)
	{}
	
	enum { IDD = IDD_PROGRESS };

    BEGIN_MSG_MAP_EX(ProgressDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_PROG_CANCEL, onCancel)
    END_MSG_MAP()
	
	LRESULT OnInitDialog(...)
	{
		CenterWindow();
		SetWindowText(windowText_.c_str());
		prog_.Attach(GetDlgItem(IDC_PROG_PROGRESS));
		prog_.SetRange(0, 100);
		
		thread_ptr.reset(new thread(bind(&ProgressDialog::ProgressThread, this)));
		
		return TRUE;
	}

	void ProgressThread()
	{
		fn_(bind(&ProgressDialog::Callback, this, _1));
		
		EndDialog(0);
	}
	
	bool Callback(size_t progress)
	{
		prog_.SetPos(progress);
		
		return stop_;
	}
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
	
	void onCancel(UINT, int, HWND hWnd)
	{
		stop_ = true;
	}
	
private:
	boost::scoped_ptr<thread> thread_ptr;
	threadFunction fn_;
	
	wstring windowText_;
	bool stop_;
	CProgressBarCtrl prog_;
};
