
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define ID_TLVM_BEGIN	 		17000
#define ID_TLVM_NEW 			ID_TLVM_BEGIN +	1
#define ID_TLVM_EDIT 			ID_TLVM_BEGIN + 2
#define ID_TLVM_DELETE	 		ID_TLVM_BEGIN +	3
#define ID_TLVM_PRIMARY 		ID_TLVM_BEGIN +	4

#ifndef RC_INVOKED

#include <boost/signals.hpp>
#include <boost/function.hpp>

#include "../stdAfx.hpp"
#include "../global/string_conv.hpp"
#include "../halIni.hpp"
#include "../HaliteSortListViewCtrl.hpp"

#include "GenericAddListView.hpp"

class TrackerListViewCtrl :
	public CHaliteSortListViewCtrl<TrackerListViewCtrl>,
	public hal::IniBase<TrackerListViewCtrl>,
	public WTLx::GenericAddListView<TrackerListViewCtrl, false, ID_TLVM_NEW, ID_TLVM_EDIT, ID_TLVM_DELETE>,
	private boost::noncopyable
{
	typedef hal::IniBase<TrackerListViewCtrl> iniClass;
	typedef CHaliteSortListViewCtrl<TrackerListViewCtrl> listClass;
	typedef WTLx::GenericAddListView<TrackerListViewCtrl, false, ID_TLVM_NEW, ID_TLVM_EDIT, ID_TLVM_DELETE> genericAddlistClass;

	friend class listClass;
	
public:
	enum { 
		LISTVIEW_ID_MENU = HAL_TRACKERLV_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_TRACKER_LISTVIEW_COLUMNS,
		LISTVIEW_ID_COLUMNWIDTHS = HAL_TRACKER_LISTVIEW_DEFAULTS
	};
	
	TrackerListViewCtrl(boost::filesystem::path location, std::string name) :
		iniClass(location, name)
	{}

	BEGIN_MSG_MAP_EX(TrackerListViewCtrl)
		MSG_WM_DESTROY(OnDestroy)
		COMMAND_ID_HANDLER(ID_TLVM_PRIMARY, OnPrimary)

		CHAIN_MSG_MAP(genericAddlistClass)
		CHAIN_MSG_MAP(listClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void uiUpdate(const hal::torrent_details_ptr pT);
	void enterNewTracker();
	void saveSettings();

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<listClass>(*this));
    }

	LRESULT OnPrimary(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	void newItem();
	void editItem(int);
	void deleteItem(int);

	void attachEditedConnection(boost::function<void ()> fn) { listEdited_.connect(fn); }

private:
	void OnAttach();
	void OnDestroy();

	boost::signal<void ()> listEdited_;
};

typedef TrackerListViewCtrl::SelectionManager TrackerListViewManager;

#endif // RC_INVOKED
