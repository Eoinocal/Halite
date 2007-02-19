
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
protected:
	typedef CHaliteIni<TrackerListViewCtrl> iniClass;
	typedef CHaliteListViewCtrl<TrackerListViewCtrl> listClass;
	
	friend class listClass;
public:	
	enum { ID_MENU = IDR_TRACKERLV_MENU };	
	
	TrackerListViewCtrl() :
		iniClass("listviews/tracker", "TrackerListView")
	{
		listColumnWidth[0] = 287;
		listColumnWidth[1] = 50;
		
		listColumnOrder[0] = 0;
		listColumnOrder[1] = 1;
		
		load();
	}

	BEGIN_MSG_MAP(TrackerListViewCtrl)	
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
        ar & BOOST_SERIALIZATION_NVP(listColumnWidth);
        ar & BOOST_SERIALIZATION_NVP(listColumnOrder);
    }
	
	LRESULT OnDoubleClick(int i, LPNMHDR pnmh, BOOL&);
	LRESULT OnNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	void attachEditedConnection(boost::function<void ()> fn) { listEdited_.connect(fn); }

private:	
	void OnAttach();
	void OnDestroy();
	
	static const size_t numListColumnWidth = 2;
	int listColumnWidth[numListColumnWidth];
	int listColumnOrder[numListColumnWidth];	

	boost::signal<void ()> listEdited_;
};

typedef selection_manager<TrackerListViewCtrl> TrackerListViewManager;
