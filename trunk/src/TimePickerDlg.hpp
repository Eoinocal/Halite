
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define ID_TIME_DLG_BEGIN	 				13900
#define HAL_TIME_DLG						13901

#define HAL_TIME_PICKER				ID_TIME_DLG_BEGIN + 2
#define HAL_TIME_DATE_PICKER			ID_TIME_DLG_BEGIN + 3
#define HAL_TIME_EDITABOUT			ID_TIME_DLG_BEGIN + 4
#define HAL_TIME_TIMEOUT_DISPLAY		ID_TIME_DLG_BEGIN + 5
#define HAL_TIME_TIME_REMAINING		ID_TIME_DLG_BEGIN + 6
#define HAL_TIME_ACTIONS			ID_TIME_DLG_BEGIN + 7
#define HAL_TIME_ACTION_PAUSEALL		ID_TIME_DLG_BEGIN + 8
#define HAL_TIME_ACTION_EXIT			ID_TIME_DLG_BEGIN + 9
#define HAL_TIME_ACTION_LOGOFF		ID_TIME_DLG_BEGIN + 10
#define HAL_TIME_ACTION_SHUTDOWN		ID_TIME_DLG_BEGIN + 11
#define HAL_TIME_SETTEXT			ID_TIME_DLG_BEGIN + 12


#ifndef RC_INVOKED

#include "stdAfx.hpp"
#include "DdxEx.hpp"

#include "HaliteWindow.hpp"
#include "UxthemeWrapper.hpp"

#ifndef NDEBUG
#	include "global/logger.hpp"
#endif

class DateTimePicker : 
	public ATL::CWindowImpl<DateTimePicker, WTL::CDateTimePickerCtrl>
{
protected:
	typedef DateTimePicker thisClass;
	typedef ATL::CWindowImpl<thisClass, WTL::CDateTimePickerCtrl> baseClass;

public:
	typedef boost::function<void ()> date_time_changed_fn;

	DateTimePicker(date_time_changed_fn dtc) :
		date_time_changed_(dtc)
	{}

	BEGIN_MSG_MAP_EX(thisClass)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(DTN_DATETIMECHANGE, OnDateTimeChange)

		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LRESULT OnDateTimeChange(LPNMHDR pnmh)
	{	
		date_time_changed_();
		
		return 0;
	}

	void Attach(HWND hWndNew)
	{
		ATLASSERT(::IsWindow(hWndNew));
		baseClass::SubclassWindow(hWndNew);
	}

private:
	date_time_changed_fn date_time_changed_;
};


class TimePickerDlg :
	public ATL::CDialogImpl<TimePickerDlg>,
	public ATL::CAutoSizeWindow<TimePickerDlg, true>
{
protected:
	typedef TimePickerDlg thisClass;
	typedef ATL::CDialogImpl<thisClass> baseClass;
	typedef ATL::CAutoSizeWindow<thisClass, true> autosizeClass;
public:
	enum { IDD = HAL_TIME_DLG };

	enum timeout_actions
	{
		action_pause = 0,
		action_exit,
		action_logoff,
		action_shutdown
	};

	TimePickerDlg(boost::posix_time::time_duration& time, unsigned& action) :
		time_ctrl_(bind(&TimePickerDlg::updateTimeoutDisplay, this)),
		date_ctrl_(bind(&TimePickerDlg::updateTimeoutDisplay, this)),
		action_time_(time),
		action_(action)
	{}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(OnClose)	

		COMMAND_ID_HANDLER_EX(IDOK, OnOk)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)

		COMMAND_HANDLER_EX(HAL_TIME_ACTIONS, CBN_SELCHANGE, OnActionChanged)

		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;

		CHAIN_MSG_MAP(autosizeClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
	
#define OK_CANCEL_BUTTON_LAYOUT \
	WMB_HEAD(WMB_COL(_exp), WMB_COL(60), WMB_COL(60)), \
		WMB_ROW(_auto, 0 , IDOK, IDCANCEL), \
	WMB_END()

	BEGIN_WINDOW_MAP(thisClass, 6, 6, 3, 3)
		WMB_HEAD(WMB_COL(80), WMB_COL(_exp), WMB_COL(_exp)),
			WMB_ROW(_auto, HAL_TIME_EDITABOUT, HAL_TIME_ACTIONS), 
			WMB_ROW(_auto, HAL_TIME_SETTEXT, HAL_TIME_PICKER, HAL_TIME_DATE_PICKER), 
			WMB_ROW(_auto, HAL_TIME_TIMEOUT_DISPLAY, _r, _r), 
			WMB_ROW(_auto, OK_CANCEL_BUTTON_LAYOUT, _r, _r), 
		WMB_END()
	END_WINDOW_MAP()	

	LRESULT onInitDialog(HWND, LPARAM)
	{
		WTL::CComboBox action_types;

		action_types.Attach(GetDlgItem(HAL_TIME_ACTIONS));
		
		action_types.AddString(hal::app().res_wstr(HAL_TIME_ACTION_PAUSEALL).c_str());
		action_types.AddString(hal::app().res_wstr(HAL_TIME_ACTION_EXIT).c_str());
		action_types.AddString(hal::app().res_wstr(HAL_TIME_ACTION_LOGOFF).c_str());
		action_types.AddString(hal::app().res_wstr(HAL_TIME_ACTION_SHUTDOWN).c_str());
		
		action_types.SetCurSel(0);
		OnActionChanged(0, HAL_TIME_ACTIONS, action_types);
		//action_ = 0;

		time_ctrl_.Attach(GetDlgItem(HAL_TIME_PICKER));
		date_ctrl_.Attach(GetDlgItem(HAL_TIME_DATE_PICKER));

		WTL::CRect rect_;;
		CenterWindow();
		GetWindowRect(rect_);
		MoveWindow(rect_.left, rect_.top, rect_.right-rect_.left, rect_.bottom-rect_.top, true);

		return 0;
	}

	void OnActionChanged(UINT uNotifyCode, int nID, CWindow wndCtl)
	{	
		hal::win_c_str<std::wstring> str_buf(MAX_PATH);		
		wndCtl.GetWindowText(str_buf, str_buf.size());
		
		if (str_buf.str() == hal::app().res_wstr(HAL_TIME_ACTION_PAUSEALL))
		{
			action_ = action_pause;
		}
		else if (str_buf.str() == hal::app().res_wstr(HAL_TIME_ACTION_EXIT))
		{
			action_ = action_exit;
		}	
		else if (str_buf.str() == hal::app().res_wstr(HAL_TIME_ACTION_LOGOFF))
		{
			action_ = action_logoff;
		}	
		else if (str_buf.str() == hal::app().res_wstr(HAL_TIME_ACTION_SHUTDOWN))
		{
			action_ = action_shutdown;
		}		
	}
	
	void OnClose()
	{
		action_time_ = boost::posix_time::not_a_date_time;

		EndDialog(0);
	}

	void OnCancel(UINT uNotifyCode, int nID, ATL::CWindow wndCtl)
	{		
		action_time_ = boost::posix_time::not_a_date_time;

		EndDialog(0);
	}

	void OnOk(UINT uNotifyCode, int nID, ATL::CWindow wndCtl)
	{
		EndDialog(1);
	}

private:
	void updateTimeoutDisplay()
	{
		using namespace boost::posix_time;
		using namespace boost::gregorian;

		SYSTEMTIME sys_time;

		date_ctrl_.GetSystemTime(&sys_time);
		boost::gregorian::date date(sys_time.wYear, sys_time.wMonth, sys_time.wDay);

		time_ctrl_.GetSystemTime(&sys_time);
		boost::posix_time::time_duration duration(sys_time.wHour, sys_time.wMinute, sys_time.wSecond, sys_time.wMilliseconds);

		ptime time(date, duration);
		ptime now = second_clock::local_time();
		

		SetDlgItemText(HAL_TIME_TIMEOUT_DISPLAY, hal::from_utf8(to_simple_string(time)).c_str());
		SetDlgItemText(HAL_TIME_EDITABOUT, hal::from_utf8(to_simple_string(time)).c_str());

		if (time > now)
		{
			time_duration time_left = time - now;
			
			SetDlgItemText(HAL_TIME_TIMEOUT_DISPLAY, (hal::wform(hal::app().res_wstr(HAL_TIME_TIME_REMAINING)) 
				% time_left.hours() % time_left.minutes() % time_left.seconds()).str().c_str());

			action_time_ = time_left;
		}
		else
			action_time_ = not_a_date_time;

		hal::event_log().post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(hal::wform(L"updateTimeoutDisplay %1%") % hal::from_utf8(to_simple_string(action_time_)))));
	}

	DateTimePicker time_ctrl_;
	DateTimePicker date_ctrl_;

	unsigned& action_;
	boost::posix_time::time_duration& action_time_;
};

#endif // RC_INVOKED
