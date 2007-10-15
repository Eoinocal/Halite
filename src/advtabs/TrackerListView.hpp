
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define ID_TLVM_BEGIN	 	11000
#define ID_TLVM_NEW 		ID_TLVM_BEGIN +	1
#define ID_TLVM_EDIT 		1526
#define ID_TLVM_DELETE	 	ID_TLVM_BEGIN +	3
#define ID_TLVM_PRIMARY 	ID_TLVM_BEGIN +	4

#ifndef RC_INVOKED

#include <boost/signals.hpp>
#include <boost/function.hpp>

#include "../stdAfx.hpp"
#include "../global/string_conv.hpp"
#include "../HaliteIni.hpp"
#include "../HaliteSortListViewCtrl.hpp"

class TrackerListViewCtrl :
	public CHaliteSortListViewCtrl<TrackerListViewCtrl>,
	public CHaliteIni<TrackerListViewCtrl>,
	private boost::noncopyable
{

	typedef CHaliteIni<TrackerListViewCtrl> iniClass;
	typedef CHaliteSortListViewCtrl<TrackerListViewCtrl> listClass;

	friend class listClass;
	
public:
	enum { 
		LISTVIEW_ID_MENU = IDR_TRACKERLV_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_TRACKER_LISTVIEW_COLUMNS,
		LISTVIEW_ID_COLUMNWIDTHS = HAL_TRACKER_LISTVIEW_DEFAULTS
	};
	
	TrackerListViewCtrl() :
		listClass(true,false,false),
		iniClass("listviews/tracker", "TrackerListView")
	{
		std::vector<wstring> names;	
		wstring column_names = hal::app().res_wstr(HAL_TRACKER_LISTVIEW_COLUMNS);

		// "Tracker;Tier"
		boost::split(names, column_names, boost::is_any_of(L";"));
		
		array<int, 2> widths = {287,50};
		array<int, 2> order = {0,1};
		array<bool, 2> visible = {true,true};
		
		SetDefaults(names, widths, order, visible, true);
		Load();
	}

	BEGIN_MSG_MAP_EX(TrackerListViewCtrl)
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER(ID_TLVM_NEW, OnNew)
		COMMAND_ID_HANDLER(ID_TLVM_EDIT, OnEdit)
		COMMAND_ID_HANDLER(ID_TLVM_DELETE, OnDelete)
		COMMAND_ID_HANDLER(ID_TLVM_PRIMARY, OnPrimary)

		REFLECTED_NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDoubleClick)

		CHAIN_MSG_MAP(listClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void uiUpdate(const hal::TorrentDetail_ptr pT);
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
	LRESULT OnPrimary(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	void attachEditedConnection(boost::function<void ()> fn) { listEdited_.connect(fn); }

private:
	void OnAttach();
	void OnDestroy();

	boost::signal<void ()> listEdited_;
};

typedef TrackerListViewCtrl::SelectionManager TrackerListViewManager;

#endif // RC_INVOKED
