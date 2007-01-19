
#pragma once

#include <boost/signals.hpp>
#include <boost/function.hpp>

#include "../stdAfx.hpp"
#include "../global/string_conv.hpp"
#include "../HaliteListViewCtrl.hpp"

class TrackerListViewCtrl : 
	public CHaliteListViewCtrl<TrackerListViewCtrl>,
	private boost::noncopyable
{
public:	
	enum { ID_MENU = IDR_TRACKERLV_MENU };	

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
	
	LRESULT OnDoubleClick(int i, LPNMHDR pnmh, BOOL&);
	LRESULT OnNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	void attachEditedConnection(boost::function<void ()> fn) { listEdited_.connect(fn); }

private:

	boost::signal<void ()> listEdited_;
};

typedef selection_manager<TrackerListViewCtrl> TrackerListViewManager;
