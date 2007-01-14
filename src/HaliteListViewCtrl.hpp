
#pragma once

#include "stdAfx.hpp"

#include <boost/array.hpp>
#include <boost/signals.hpp>

class HaliteListViewCtrl;

class selection_manager : noncopyable
{
public:
	selection_manager(HaliteListViewCtrl& m_list) :
		m_list_(m_list)
	{}
	typedef const string& param_type;
	
	void sync_list(bool list_to_manager);
	
	param_type selected() const { return selected_; }
	const std::vector<string>& allSelected() const { return all_selected_; }
	
	void setSelected(const string& sel) 
	{
		selected_ = sel;
		sync_list(false);
	}
	
	void setSelected(int itemPos);
	
	void clear();
	void clearAllSelected();
	
	void attach(boost::function<void (param_type)> fn) { selection_.connect(fn); }
	void signal() { selection_(selected_); }
	
private:
	string selected_;
	std::vector<string> all_selected_;
	boost::signal<void (param_type)> selection_;
	HaliteListViewCtrl& m_list_;
};

class HaliteListViewCtrl : 
	public CWindowImpl<HaliteListViewCtrl, CListViewCtrl>,
	private noncopyable
{
public:	
	HaliteListViewCtrl();
	
	BEGIN_MSG_MAP(HaliteListViewCtrl)
		MSG_WM_SHOWWINDOW(onShowWindow)
		
		COMMAND_ID_HANDLER(ID__LVM_PAUSE, OnPause)
		COMMAND_ID_HANDLER(ID_LVM_STOP, OnStop)
		COMMAND_ID_HANDLER(ID_LVM_RESUME, OnResume)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_T, OnRemove)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_TD, OnRemoveWipeFiles)
		
		REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK , OnColClick)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	void onShowWindow(UINT, INT);
	void saveStatus();
	void updateListView();
	
	LRESULT OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	LRESULT OnClick(int, LPNMHDR pnmh, BOOL&);
	LRESULT OnRClick(int, LPNMHDR pnmh, BOOL&);
	LRESULT OnColClick(int, LPNMHDR pnmh, BOOL&);
	
	selection_manager& manager() { return manager_; }

private:
	selection_manager manager_;
	WTL::CMenu torrentMenu_;
};
