
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define HAL_PROGRESS                    15000
#define ID_PROGRESS_BEGIN				15001

#define HAL_PROG_DESCP                  ID_PROGRESS_BEGIN + 1
#define HAL_PROG_CANCEL                 ID_PROGRESS_BEGIN + 2
#define HAL_PROG_PROGRESS               ID_PROGRESS_BEGIN + 3

#ifndef RC_INVOKED

#include <boost/function.hpp>

#include "Halite.hpp"
#include "halTorrent.hpp"

class ProgressDialog :
	public ATL::CDialogImpl<ProgressDialog>
{
protected:
	typedef ProgressDialog this_class_t;
	typedef ATL::CDialogImpl<ProgressDialog> base_class_t;
	
	typedef boost::function<bool (hal::progress_callback fn)> threadFunction;

public:
	ProgressDialog(wstring windowText, threadFunction fn) :
		fn_(fn),
		windowText_(windowText),
		stop_(false)
	{}
	
	enum { IDD = HAL_PROGRESS };

	BEGIN_MSG_MAP_EX(ProgressDialog)
		try
		{
			MSG_WM_INITDIALOG(onInitDialog)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in ProgressDialog MSG_MAP")

		COMMAND_ID_HANDLER_EX(HAL_PROG_CANCEL, onCancel)
	END_MSG_MAP()
	
	LRESULT onInitDialog(HWND, LPARAM)
	{
		CenterWindow();
		SetWindowText(windowText_.c_str());
		prog_.Attach(GetDlgItem(HAL_PROG_PROGRESS));
		prog_.SetRange(0, 100);
		
		thread_ptr.reset(new thread(bind(&ProgressDialog::ProgressThread, this)));
		
		return TRUE;
	}

	void ProgressThread()
	{
		try
		{

		int err_code = (fn_(boost::bind(&ProgressDialog::Callback, this, _1, _2, _3)) ? 1 : 0);
		
		EndDialog(err_code);

		}
		catch(std::exception& e)
		{
			hal::event_log().post(shared_ptr<hal::EventDetail>(\
				new hal::EventStdException(hal::event_logger::debug, e, L"ProgressThread()")));
		}
		catch(...)
		{
			HAL_DEV_MSG(L"ProgressThread() catch all");
		}
	}
	
	bool Callback(size_t progress, size_t total, wstring description)
	{
		SetDlgItemText(HAL_PROG_DESCP, description.c_str());
		prog_.SetPos(numeric_cast<int>((100*progress)/total));
		
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
	WTL::CProgressBarCtrl prog_;
};

#endif // RC_INVOKED
