
#pragma once

#include "../stdAfx.hpp"
#include "../global/string_conv.hpp"
#include "../global/logger.hpp"

#include "../DdxEx.hpp"
#include "../Halite.hpp"
#include "../halEvent.hpp"

#include "../HaliteTabPage.hpp"
#include "../HaliteListManager.hpp"
#include "../HaliteDialogBase.hpp"
#include "../HaliteIni.hpp"
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
	public CHaliteListViewCtrl<LogListViewCtrl>,
	public CHaliteIni<LogListViewCtrl>,
	private boost::noncopyable
{
protected:
	typedef CHaliteIni<LogListViewCtrl> iniClass;
	typedef CHaliteListViewCtrl<LogListViewCtrl> listClass;

	friend class listClass;

public:	
	enum { 
		LISTVIEW_ID_MENU = 0,
		LISTVIEW_ID_COLUMNNAMES = HAL_DEBUG_LISTVIEW_COLUMNS,
		LISTVIEW_ID_COLUMNWIDTHS = HAL_DEBUG_LISTVIEW_DEFAULTS
	};

	BEGIN_MSG_MAP_EX(LogListViewCtrl)
		MSG_WM_DESTROY(OnDestroy)

		CHAIN_MSG_MAP(CHaliteListViewCtrl<LogListViewCtrl>)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LogListViewCtrl() :
		iniClass("listviews/eventLog", "LogListView")
	{
		load();
	}
	
	~LogListViewCtrl()
	{
		if (conn_.connected()) conn_.disconnect();
	}

	void saveSettings()
	{
		GetListViewDetails();
		save();
	}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<listClass>(*this));
    }

	void operator()(shared_ptr<hal::EventDetail> event)
	{
		int itemPos = AddItem(0, 0, lexical_cast<wstring>(event->timeStamp()).c_str(), 0);

		SetItemText(itemPos, 1,	event->msg().c_str());

		SetItemText(itemPos, 2,
			hal::Event::eventLevelToStr(event->level()).c_str());
	}

	void saveStatus() {}
	void updateListView() {}

private:
	void OnAttach()
	{
		SetListViewDetails();
		conn_ = hal::event().attach(bind(&LogListViewCtrl::operator(), this, _1));
	}

	void OnDestroy()
	{
		conn_.disconnect();
		saveSettings();
	}

	boost::signals::connection conn_;
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

		CHAIN_MSG_MAP(resizeClass)
		CHAIN_MSG_MAP(baseClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BOOL DoDataExchange(BOOL bSaveAndValidate = FALSE, UINT nCtlID = (UINT)-1);

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(IDC_DEBUGLISTVIEW, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_DEBUGFILECHECK, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_DEBUGDEBUGCHECK, DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	LRESULT onInitDialog(HWND, LPARAM);
	void onClose();

	void onLoginCheck(UINT, int, HWND hWnd);
	void onFileCheck(UINT, int, HWND hWnd) { DoDataExchange(true); }
	void onDebugCheck(UINT, int, HWND hWnd) { DoDataExchange(true); }

	LRESULT OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl);
	void onDebugOption(UINT, int, HWND);

	void selectionChanged(const string& torrent_name);
	void updateDialog();

protected:
	LogListViewCtrl logList;
	int debugLevel;
};
