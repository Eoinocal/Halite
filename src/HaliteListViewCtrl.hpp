
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
	
	void setSelected(const string& sel) 
	{
		selected_ = sel;
		sync_list(false);
	}
	
	void setSelected(int itemPos);
	
	void clear();
	
	void attach(boost::function<void (param_type)> fn) { selection_.connect(fn); }
	void signal() { selection_(selected_); }
	
private:
	string selected_;	
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
		
		REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK , OnClick)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	void onShowWindow(UINT, INT);
	void saveStatus();
	void updateListView();
	
	LRESULT OnClick(int, LPNMHDR pnmh, BOOL&);
	
	selection_manager& manager() { return manager_; }

private:
	selection_manager manager_;
};
