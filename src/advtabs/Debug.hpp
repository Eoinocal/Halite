
#pragma once

#include "../stdAfx.hpp"
#include "../global/string_conv.hpp"
#include "../global/logger.hpp"

#include "../DdxEx.hpp"
#include "../Halite.hpp"
#include "../halEvent.hpp"

#include "../HaliteTabPage.hpp"
#include "../HaliteListManager.hpp"
#include "../HaliteIni.hpp"
#include "../HaliteListViewCtrl.hpp"

class ui_signal;

class HaliteListViewCtrl;
template <class TBase> class CHaliteListViewCtrl;
typedef selection_manager<CHaliteListViewCtrl<HaliteListViewCtrl> > ListViewManager;

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
	enum { ID_MENU = IDR_LISTVIEW_MENU };

	BEGIN_MSG_MAP_EX(LogListViewCtrl)
		MSG_WM_DESTROY(OnDestroy)

		CHAIN_MSG_MAP(CHaliteListViewCtrl<LogListViewCtrl>)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LogListViewCtrl() :
		iniClass("listviews/eventLog", "LogListView")
	{
		listColumnWidth[0] = 67;
		listColumnWidth[1] = 419;
		listColumnWidth[2] = 69;

		listColumnOrder[0] = 0;
		listColumnOrder[1] = 1;
		listColumnOrder[2] = 2;

		load();
	}

	void saveSettings()
	{
		assert (GetHeader().GetItemCount() == numListColumnWidth);

		GetColumnOrderArray(numListColumnWidth, (int*)&listColumnOrder);

		for (int i=0; i<numListColumnWidth; ++i)
			listColumnWidth[i] = GetColumnWidth(i);

		save();
	}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(listColumnWidth);
        ar & BOOST_SERIALIZATION_NVP(listColumnOrder);
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
		SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);

		CHeaderCtrl hdr = GetHeader();
		hdr.ModifyStyle(0, HDS_DRAGDROP|HDS_FULLDRAG);

		AddColumn(L"Time", hdr.GetItemCount());
		AddColumn(L"Message", hdr.GetItemCount());
		AddColumn(L"Severity", hdr.GetItemCount());

		assert (hdr.GetItemCount() == numListColumnWidth);

		for (int i=0; i<numListColumnWidth; ++i)
			SetColumnWidth(i, listColumnWidth[i]);

		SetColumnOrderArray(numListColumnWidth, (int*)&listColumnOrder);


		boost::signals::scoped_connection* p = new boost::signals::scoped_connection(
			hal::event().attach(bind(&LogListViewCtrl::operator(), this, _1))
		);

		pconn_.reset(new boost::signals::scoped_connection(*p));
	}

	void OnDestroy()
	{
		saveSettings();
	}

	static const size_t numListColumnWidth = 3;
	int listColumnWidth[numListColumnWidth];
	int listColumnOrder[numListColumnWidth];

	scoped_ptr<boost::signals::scoped_connection> pconn_;
};

class AdvDebugDialog :
	public CHalTabPageImpl<AdvDebugDialog>,
	public CDialogResize<AdvDebugDialog>,
	public CWinDataExchangeEx<AdvDebugDialog>,
	private boost::noncopyable
{
protected:
	typedef AdvDebugDialog thisClass;
	typedef CHalTabPageImpl<AdvDebugDialog> baseClass;
	typedef CDialogResize<AdvDebugDialog> resizeClass;

public:
	enum { IDD = IDD_ADVDEBUGLOG };

	AdvDebugDialog(ui_signal& ui_sig, ListViewManager& single_sel);

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
	ui_signal& ui_;
	ListViewManager& selection_manager_;
};
