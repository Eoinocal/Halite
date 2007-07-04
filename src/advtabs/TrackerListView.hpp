
#pragma once

#include <boost/signals.hpp>
#include <boost/function.hpp>

#include "../stdAfx.hpp"
#include "../global/string_conv.hpp"
#include "../HaliteIni.hpp"
#include "../HaliteListViewCtrl.hpp"

class TrackerListViewCtrl :
	public CHaliteListViewCtrl<TrackerListViewCtrl>,
	public CHaliteIni<TrackerListViewCtrl>,
	private boost::noncopyable
{

	typedef CHaliteIni<TrackerListViewCtrl> iniClass;
	typedef CHaliteListViewCtrl<TrackerListViewCtrl> listClass;

	friend class listClass;
	
public:
	enum { 
		LISTVIEW_ID_MENU = IDR_TRACKERLV_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_TRACKER_LISTVIEW_COLUMNS	
	};
	
	TrackerListViewCtrl() :
		iniClass("listviews/tracker", "TrackerListView")
	{
		array<int, 2> a = {{287, 50}};
		SetDefaults(a);

		load();
	}

	BEGIN_MSG_MAP_EX(TrackerListViewCtrl)
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER(ID_TLVM_NEW, OnNew)
		COMMAND_ID_HANDLER(ID_TLVM_EDIT, OnEdit)
		COMMAND_ID_HANDLER(ID_TLVM_DELETE, OnDelete)

		REFLECTED_NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDoubleClick)

		CHAIN_MSG_MAP(CHaliteListViewCtrl<TrackerListViewCtrl>)

		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void updateListView();
	void enterNewTracker();
	void saveSettings();

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<listClass>(*this));
    }

	LRESULT OnDoubleClick(int i, LPNMHDR pnmh, BOOL&);
	LRESULT OnNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	void attachEditedConnection(boost::function<void ()> fn) { listEdited_.connect(fn); }

private:
	void OnAttach();
	void OnDestroy();

/*	static const size_t numListColumnWidth = 2;
	int listColumnWidth[numListColumnWidth];
	int listColumnOrder[numListColumnWidth];
*/
	boost::signal<void ()> listEdited_;
};

typedef TrackerListViewCtrl::selection_manage_class TrackerListViewManager;
