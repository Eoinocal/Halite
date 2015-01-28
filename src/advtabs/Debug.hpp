
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define WM_USER_LOGPOST	WM_USER + 101

#include "stdAfx.hpp"
#include "Halite.hpp"
#include "DdxEx.hpp"

#include "global/string_conv.hpp"
#include "global/logger.hpp"

#include "../halEvent.hpp"
#include "../HaliteTabPage.hpp"
#include "../HaliteListManager.hpp"
#include "../HaliteDialogBase.hpp"
#include "../halIni.hpp"
#include "../HaliteListViewCtrl.hpp"

class LogEdit : public ATL::CWindowImpl<LogEdit, WTL::CEdit>
{
public:
	BEGIN_MSG_MAP_EX(CEditImpl)
	END_MSG_MAP()

	LogEdit() :
		editLogger(hal::wlog().attach(boost::bind(&LogEdit::log, this, _1)))
	{}

	void log(const std::wstring& text)
	{
		int len = numeric_cast<int>(::SendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0));
		SetSel(len, len);
		ReplaceSel(text.c_str(), false);
	}

private:
	boost::signals2::scoped_connection editLogger;
};

class LogList : public ATL::CWindowImpl<LogList, WTL::CListBox>
{
public:
	BEGIN_MSG_MAP_EX(LogList)
	END_MSG_MAP()

	LogList() :
		listLogger(hal::wlog().attach(boost::bind(&LogList::log, this, _1)))
	{}

	void log(const std::wstring& text)
	{
		AddString(text.c_str());
	}

private:
	boost::signals2::scoped_connection listLogger;
};

class LogListViewCtrl :
	public CHaliteSortListViewCtrl<LogListViewCtrl>,
	public hal::IniBase<LogListViewCtrl>,
	private boost::noncopyable
{
protected:
	typedef hal::IniBase<LogListViewCtrl> ini_class_t;
	typedef CHaliteSortListViewCtrl<LogListViewCtrl> list_class_t;

	friend class list_class_t;

public:	
	enum { 
		LISTVIEW_ID_MENU = 0,
		LISTVIEW_ID_COLUMNNAMES = 0,
		LISTVIEW_ID_COLUMNWIDTHS = 0
	};

	BEGIN_MSG_MAP_EX(LogListViewCtrl)
		try
		{
		MSG_WM_DESTROY(OnDestroy)
		MESSAGE_HANDLER_EX(WM_USER_LOGPOST, OnMessageLogPost)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in LogListViewCtrl MSG_MAP")

		CHAIN_MSG_MAP(list_class_t)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LogListViewCtrl() :
		ini_class_t(L"listviews/event_log", L"log_listview")
	{}
	
	~LogListViewCtrl()
	{
		hal::mutex_t::scoped_lock l(mutex_);

		if (conn_.connected()) conn_.disconnect();
	}

	void saveSettings()
	{
		GetListViewDetails();
		save_to_ini();
	}

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 2:			
			ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<list_class_t>(*this));
			
		break;

		case 1:
		default:
			assert(false);
		}
	}

	void operator()(shared_ptr<hal::EventDetail> event)
	{
		hal::mutex_t::scoped_lock l(mutex_);

		try
		{
		events_.push_back(event);
		
		PostMessage(WM_USER_LOGPOST, 0, 0);
		
		}
		catch(...)
		{}
	}

	LRESULT OnMessageLogPost(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		hal::mutex_t::scoped_lock l(mutex_);

		try
		{
		shared_ptr<hal::EventDetail> event(*events_.begin());

		wstring timeStamp = lexical_cast<wstring>(event->timeStamp());

		int itemPos = AddItem(0, 0, timeStamp.c_str());

		SetItemText(itemPos, 1,	event->msg().c_str());

		SetItemText(itemPos, 2,
			hal::event_logger::eventLevelToStr(event->level()).c_str());
			
		if (halite().logListLen() <= GetItemCount())
			DeleteItem(halite().logListLen());

		}
		catch(...)
		{}

		events_.pop_front();

		return 0;
	}

	void saveStatus() {}
	void updateListView() {}

private:
	void OnAttach()
	{	
		hal::mutex_t::scoped_lock l(mutex_);
	
		InitialSetup();		

		std::vector<wstring> names;	
		wstring column_names = hal::app().res_wstr(HAL_DEBUG_LISTVIEW_COLUMNS);

		// "Time;Message;Severity"
		boost::split(names, column_names, boost::is_any_of(L";"));
		
		array<int, 3> widths = {67,419,69};
		array<int, 3> order = {0,1,2};
		array<bool, 3> visible = {true,true,true};

		for (int i=0, e=3; i < e; ++i)
			AddColumn(names[i].c_str(), i, visible[i], widths[i]);
	
		safe_load_from_ini();
		
		conn_ = hal::event_log().attach(bind(&LogListViewCtrl::operator(), this, _1));
	}

	void OnDestroy()
	{
		hal::mutex_t::scoped_lock l(mutex_);

		conn_.disconnect();
		saveSettings();
	}

	mutable hal::mutex_t mutex_;
	boost::signals2::connection conn_;
	std::deque<shared_ptr<hal::EventDetail> > events_;
};

class AdvDebugDialog :
	public CHalTabPageImpl<AdvDebugDialog>,
	public WTL::CDialogResize<AdvDebugDialog>,
	public CHaliteDialogBase<AdvDebugDialog>,
	public WTLx::WinDataExchangeEx<AdvDebugDialog>,
	private boost::noncopyable
{
protected:
	typedef AdvDebugDialog this_class_t;
	typedef CHalTabPageImpl<AdvDebugDialog> base_class_t;
	typedef WTL::CDialogResize<AdvDebugDialog> resize_class_t;
	typedef CHaliteDialogBase<AdvDebugDialog> dlg_base_class_t;

public:
	enum { IDD = HAL_ADVDEBUGLOG };

	AdvDebugDialog(HaliteWindow& halWindow) :
		dlg_base_class_t(halWindow)
	{}
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(this_class_t)
		try
		{
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)

		COMMAND_ID_HANDLER_EX(HAL_DEBUGFILECHECK, onFileCheck)
		COMMAND_ID_HANDLER_EX(HAL_DEBUGDEBUGCHECK, onDebugCheck)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in AdvDebugDialog MSG_MAP")

		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;

		CHAIN_MSG_MAP(dlg_base_class_t)
		CHAIN_MSG_MAP(resize_class_t)
		CHAIN_MSG_MAP(base_class_t)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BOOL DoDataExchange(BOOL bSaveAndValidate = FALSE, UINT nCtlID = (UINT)-1);

	BEGIN_DLGRESIZE_MAP(this_class_t)
		DLGRESIZE_CONTROL(HAL_DEBUGLISTVIEW, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(HAL_DEBUGFILECHECK, DLSZ_SIZE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(HAL_DEBUGDEBUGCHECK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	LRESULT onInitDialog(HWND, LPARAM);
	void onClose();

	void onLoginCheck(UINT, int, HWND hWnd);
	void onFileCheck(UINT, int, HWND hWnd) { DoDataExchange(true); }
	void onDebugCheck(UINT, int, HWND hWnd) { DoDataExchange(true); }

	void onDebugOption(UINT, int, HWND);

protected:
	LogListViewCtrl logList;
	int debugLevel;
};

BOOST_CLASS_VERSION(LogListViewCtrl, 2)
