
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
public:	
	enum { ID_MENU = IDR_TRACKERLV_MENU };	
	
	TrackerListViewCtrl() :
		CHaliteIni<TrackerListViewCtrl>("listviews/tracker", "TrackerListView"),
		hello(true)
	{
		load();
	}
	
	~TrackerListViewCtrl()
	{
		save();
	}

	BEGIN_MSG_MAP(TrackerListViewCtrl)		
		COMMAND_ID_HANDLER(ID_TLVM_NEW, OnNew)
		COMMAND_ID_HANDLER(ID_TLVM_EDIT, OnEdit)
		COMMAND_ID_HANDLER(ID_TLVM_DELETE, OnDelete)
		
		REFLECTED_NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDoubleClick)
		
		CHAIN_MSG_MAP(CHaliteListViewCtrl<TrackerListViewCtrl>)
		
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	void OnAttach();
	void saveStatus();
	void updateListView();
	void enterNewTracker();

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(hello);
    }
	
	LRESULT OnDoubleClick(int i, LPNMHDR pnmh, BOOL&);
	LRESULT OnNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	void attachEditedConnection(boost::function<void ()> fn) { listEdited_.connect(fn); }

private:

	boost::signal<void ()> listEdited_;
	bool hello;
};

typedef selection_manager<TrackerListViewCtrl> TrackerListViewManager;
