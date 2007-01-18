
#pragma once

#include <boost/array.hpp>
#include <boost/signals.hpp>

#include "../stdAfx.hpp"
#include "../global/string_conv.hpp"
#include "../HaliteListViewCtrl.hpp"

class TrackerListViewCtrl : 
	public CHaliteListViewCtrl<TrackerListViewCtrl>,
	private boost::noncopyable
{
public:	
	enum { ID_MENU = IDR_LISTVIEW_MENU };	

	BEGIN_MSG_MAP(TrackerListViewCtrl)		
		COMMAND_ID_HANDLER(ID__LVM_PAUSE, OnPause)
		COMMAND_ID_HANDLER(ID_LVM_STOP, OnStop)
		COMMAND_ID_HANDLER(ID_LVM_RESUME, OnResume)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_T, OnRemove)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_TD, OnRemoveWipeFiles)
		
		REFLECTED_NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDoubleClick)
		
		CHAIN_MSG_MAP(CHaliteListViewCtrl<TrackerListViewCtrl>)
		
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	void OnAttach();
	LRESULT OnDoubleClick(int i, LPNMHDR pnmh, BOOL&);
	void saveStatus();
	void updateListView();
	
	LRESULT OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:

};

typedef selection_manager<TrackerListViewCtrl> TrackerListViewManager;
