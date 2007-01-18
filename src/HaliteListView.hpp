
#pragma once

#include "stdAfx.hpp"
#include "global/string_conv.hpp"

#include <boost/array.hpp>
#include <boost/signals.hpp>

#include "HaliteListViewCtrl.hpp"

class HaliteListViewCtrl;

class HaliteListViewCtrl : 
	public CHaliteListViewCtrl<HaliteListViewCtrl>,
	private noncopyable
{
public:	

	enum { ID_MENU = IDR_LISTVIEW_MENU };	
	
	BEGIN_MSG_MAP(HaliteListViewCtrl)
		MSG_WM_SHOWWINDOW(onShowWindow)
		
		COMMAND_ID_HANDLER(ID__LVM_PAUSE, OnPause)
		COMMAND_ID_HANDLER(ID_LVM_STOP, OnStop)
		COMMAND_ID_HANDLER(ID_LVM_RESUME, OnResume)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_T, OnRemove)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_TD, OnRemoveWipeFiles)
		
		CHAIN_MSG_MAP(CHaliteListViewCtrl<HaliteListViewCtrl>)
		
/*		REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK , OnColClick)
*/		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	void onShowWindow(UINT, INT);
	void saveStatus();
	void updateListView();
	
	LRESULT OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
/*	LRESULT OnClick(int, LPNMHDR pnmh, BOOL&);
	LRESULT OnRClick(int, LPNMHDR pnmh, BOOL&);
	LRESULT OnColClick(int, LPNMHDR pnmh, BOOL&);
*/
//	selection_manager<HaliteListViewCtrl>& manager() { return manager_; }

private:
//	selection_manager<HaliteListViewCtrl> manager_;
//	WTL::CMenu torrentMenu_;
};

typedef selection_manager<CHaliteListViewCtrl<HaliteListViewCtrl> > ListViewManager;
