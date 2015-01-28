
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define ID_TLVM_BEGIN	 		17000
#define ID_TLVM_NEW 			ID_TLVM_BEGIN + 1
#define ID_TLVM_EDIT 			ID_TLVM_BEGIN + 2
#define ID_TLVM_DELETE	 		ID_TLVM_BEGIN + 3
#define ID_TLVM_PRIMARY 		ID_TLVM_BEGIN + 4

#ifndef RC_INVOKED

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
	typedef hal::IniBase<TrackerListViewCtrl> ini_class_t;
	typedef CHaliteSortListViewCtrl<TrackerListViewCtrl> list_class_t;
	typedef WTLx::GenericAddListView<TrackerListViewCtrl, false, ID_TLVM_NEW, ID_TLVM_EDIT, ID_TLVM_DELETE> genericAddlist_class_t;

	friend class list_class_t;
	
public:
	enum { 
		LISTVIEW_ID_MENU = HAL_TRACKERLV_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_TRACKER_LISTVIEW_COLUMNS,
		LISTVIEW_ID_COLUMNWIDTHS = HAL_TRACKER_LISTVIEW_DEFAULTS
	};
	
	TrackerListViewCtrl(boost::filesystem::wpath location, std::wstring name) :
		ini_class_t(location, name)
	{}

	BEGIN_MSG_MAP_EX(TrackerListViewCtrl)
		try
		{
		MSG_WM_DESTROY(OnDestroy)
		COMMAND_ID_HANDLER(ID_TLVM_PRIMARY, OnPrimary)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in TrackerListViewCtrl MSG_MAP")

		CHAIN_MSG_MAP(genericAddlist_class_t)
		CHAIN_MSG_MAP(list_class_t)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void uiUpdate(const hal::torrent_details_ptr pT);
	void enterNewTracker();
	void saveSettings();

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

	LRESULT OnPrimary(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	void newItem();
	void editItem(int);
	void deleteItem(int);

	template<typename F>
	void attachEditedConnection(F&& fn) { listEdited_.connect(fn); }

private:
	void OnAttach();
	void OnDestroy();

	boost::signals2::signal<void ()> listEdited_;
};

typedef TrackerListViewCtrl::SelectionManager TrackerListViewManager;

BOOST_CLASS_VERSION(TrackerListViewCtrl, 2)

#endif // RC_INVOKED
