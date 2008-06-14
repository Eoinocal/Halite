
//         Copyright Eóin O'Callaghan 2006 - 2008.
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

class LogEdit : public CWindowImpl<LogEdit, CEdit>
{
public:
    BEGIN_MSG_MAP_EX(CEditImpl)
    END_MSG_MAP()

	LogEdit() :
		editLogger(hal::wlog().attach(bind(&LogEdit::log, this, _1)))
	{}

	void log(const std::wstring& text)
	{
		int len = ::SendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0);
		SetSel(len, len);
		ReplaceSel(text.c_str(), false);
	}

private:
	boost::signals::scoped_connection editLogger;
};

class LogList : public CWindowImpl<LogList, CListBox>
{
public:
    BEGIN_MSG_MAP_EX(CListBox)
    END_MSG_MAP()

	LogList() :
		listLogger(hal::wlog().attach(bind(&LogList::log, this, _1)))
	{}

	void log(const std::wstring& text)
	{
		AddString(text.c_str());
	}

private:
	boost::signals::scoped_connection listLogger;
};

class LogListViewCtrl :
	public CHaliteSortListViewCtrl<LogListViewCtrl>,
	public hal::IniBase<LogListViewCtrl>,
	private boost::noncopyable
{
protected:
	typedef hal::IniBase<LogListViewCtrl> iniClass;
	typedef CHaliteSortListViewCtrl<LogListViewCtrl> listClass;

	friend class listClass;

public:	
	enum { 
		LISTVIEW_ID_MENU = 0,
		LISTVIEW_ID_COLUMNNAMES = 0,
		LISTVIEW_ID_COLUMNWIDTHS = 0
	};

	BEGIN_MSG_MAP_EX(LogListViewCtrl)
		MSG_WM_DESTROY(OnDestroy)
		MESSAGE_HANDLER_EX(WM_USER_LOGPOST, OnMessageLogPost)

		CHAIN_MSG_MAP(listClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LogListViewCtrl() :
		iniClass("listviews/eventLog", "LogListView")
	{
		std::vector<wstring> names;	
		wstring column_names = hal::app().res_wstr(HAL_DEBUG_LISTVIEW_COLUMNS);

		// "Time;Message;Severity"
		boost::split(names, column_names, boost::is_any_of(L";"));
		
		array<int, 3> widths = {67,419,69};
		array<int, 3> order = {0,1,2};
		array<bool, 3> visible = {true,true,true};
		
		SetDefaults(names, widths, order, visible, true);		
		load_from_ini();
	}
	
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
		ar & boost::serialization::make_nvp("listview", 
			boost::serialization::base_object<listClass>(*this));
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
	
		SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);
		SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
		
		ApplyDetails();
		
		conn_ = hal::event_log.attach(bind(&LogListViewCtrl::operator(), this, _1));
	}

	void OnDestroy()
	{
		hal::mutex_t::scoped_lock l(mutex_);

		conn_.disconnect();
		saveSettings();
	}

	mutable hal::mutex_t mutex_;
	boost::signals::connection conn_;
	std::deque<shared_ptr<hal::EventDetail> > events_;
};

class AdvDebugDialog :
	public CHalTabPageImpl<AdvDebugDialog>,
	public CDialogResize<AdvDebugDialog>,
	public CHaliteDialogBase<AdvDebugDialog>,
	public CWinDataExchangeEx<AdvDebugDialog>,
	private boost::noncopyable
{
protected:
	typedef AdvDebugDialog thisClass;
	typedef CHalTabPageImpl<AdvDebugDialog> baseClass;
	typedef CDialogResize<AdvDebugDialog> resizeClass;
	typedef CHaliteDialogBase<AdvDebugDialog> dialogBaseClass;

public:
	enum { IDD = IDD_ADVDEBUGLOG };

	AdvDebugDialog(HaliteWindow& halWindow) :
		dialogBaseClass(halWindow)
	{}
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)

		COMMAND_ID_HANDLER_EX(IDC_DEBUGFILECHECK, onFileCheck)
		COMMAND_ID_HANDLER_EX(IDC_DEBUGDEBUGCHECK, onDebugCheck)

		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;

		CHAIN_MSG_MAP(dialogBaseClass)
		CHAIN_MSG_MAP(resizeClass)
		CHAIN_MSG_MAP(baseClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BOOL DoDataExchange(BOOL bSaveAndValidate = FALSE, UINT nCtlID = (UINT)-1);

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(IDC_DEBUGLISTVIEW, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_DEBUGFILECHECK, DLSZ_SIZE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_DEBUGDEBUGCHECK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
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
