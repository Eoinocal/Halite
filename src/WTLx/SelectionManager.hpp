
//         Copyright Eóin O'Callaghan 2006 - 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once 

#include "stdAfx.hpp"

namespace WTLx
{

template<typename L, typename S>
class selection_manager : 
	private boost::noncopyable
{	
public:
	selection_manager(L& list) :
		list_(list)
	{}
	
	typedef std::wstring string_t; 
	typedef const string_t& param_type;
	
	void sync_list(bool list_to_manager, bool signal_change=true)
	{
		if (list_to_manager)
		{	
			if (from_listview() && signal_change) signal();
		}
		else
		{
			if (managerToList() && signal_change) signal();
		}
	}
	
	bool from_listview()
	{
		hal::win_c_str<string_t, MAX_PATH> c_string;

		std::set<string_t> all_selected;
		string_t selected;
		
		bool do_signal = false;
		
		for (int i=0, e=list_.GetItemCount(); i<e; ++i)
		{
			UINT flags = list_.GetItemState(i, LVIS_SELECTED);
			
			if (flags && LVIS_SELECTED)
			{
				list_.GetItemText(i, 0, c_string, c_string.size());	
				all_selected.insert(c_string);
			}
			if (flags && LVIS_FOCUSED)
			{
				selected = string_t(c_string);
			}
		}

		if (all_selected != all_selected_)
		{
			std::swap(all_selected_, all_selected);
			do_signal = true;
		}
				
		if (selected_ != selected)
		{
			std::swap(selected_, selected);
			do_signal = true;
		}
		
		return do_signal;
	}
	
	bool managerToList()
	{
		// Prevent changing states from signaling another sync
		hal::mutex_update_lock<L> lock(list_);
		
		boost::array<wchar_t, MAX_PATH> pathBuffer;
		LV_FINDINFO findInfo = { sizeof(LV_FINDINFO) }; 
		findInfo.flags = LVFI_STRING;
		
		bool do_signal = true; // Always signal for now!
		
		int total = list_.GetItemCount();
		
		for (int i=0; i<total; ++i)
		{
			list_.GetItemText(i, 0, pathBuffer.c_array(), pathBuffer.size());
			string_t temp_name = pathBuffer.data();
			
			LVITEM lvi = { LVIF_STATE };
			lvi.state = 0;
			lvi.stateMask = LVIS_SELECTED|LVIS_FOCUSED;
			
			if (temp_name == selected_)
			{
				lvi.state |= LVIS_FOCUSED;
			}
			if (all_selected_.find(temp_name) != all_selected_.end())
			{
				lvi.state |= LVIS_SELECTED;
			}
			
			list_.SetItemState(i, &lvi);
		}			
		
		return do_signal;
	}
	
	int selectMatch(const string_t& name)
	{
		LV_FINDINFO findInfo = { sizeof(LV_FINDINFO) }; 
		findInfo.flags = LVFI_STRING;
				
		findInfo.psz = name.c_str();
		
		int itemPos = list_.FindItem(&findInfo, -1);	
					
		if (itemPos == -1)
			return itemPos;
			
		UINT flags = list_.GetItemState(itemPos, LVIS_SELECTED);
		
		//if (!flags && LVIS_SELECTED)
		{
			LVITEM lvi = { LVIF_STATE };
			lvi.state = LVIS_SELECTED;
			lvi.stateMask = LVIS_SELECTED;
			list_.SetItemState(itemPos, &lvi);
		}
	
		return itemPos;
	}
	
	param_type selected() const { return selected_; }
	
	int selectedIndex() const
	{
		LV_FINDINFO findInfo = { sizeof(LV_FINDINFO) }; 	
		findInfo.psz = selected_.c_str();
		
		return list_.FindItem(&findInfo, -1);		
	}
	
	const std::set<string_t>& allSelected() const { return all_selected_; }
	
	void setSelected(const string_t& sel) 
	{
		selected_ = sel;
	}
	
	void setSelected(int itemPos)
	{		
		hal::event_log().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::event_logger::info, (hal::wform(L"Set Selected %1%") % itemPos).str().c_str())));

		LVITEM lvi = { LVIF_STATE };
		lvi.state = LVIS_SELECTED|LVIS_FOCUSED;
		lvi.stateMask = LVIS_SELECTED|LVIS_FOCUSED;
		list_.SetItemState(itemPos, &lvi);
		
		list_.SetSelectionMark(itemPos);
		
		sync_list(true);
	}
	
	void clear()
	{
		// Prevent changing states from signaling another sync
		hal::mutex_update_lock<L> lock(list_);
		
		list_.DeleteItem(selectedIndex());
		
		sync_list(true);	
	}
	
	void clear_all_selected()
	{
		// Prevent changing states from signaling another sync
		hal::mutex_update_lock<L> lock(list_);
		
		int total = list_.GetItemCount();
		
		for (int i=total-1; i>=0; --i)
		{
			UINT flags = list_.GetItemState(i, LVIS_SELECTED);
			
			if (flags && LVIS_SELECTED)
				list_.DeleteItem(i);
		}
		all_selected_.clear();
		
		sync_list(true);	
	}
	
	void clear_all()
	{
		// Prevent changing states from signaling another sync
		hal::mutex_update_lock<L> lock(list_);
		
		list_.DeleteAllItems();
		all_selected_.clear();
		
		sync_list(true);		
	}
	
	void attach(boost::function<void (param_type)> fn) const { selection_.connect(fn); }
	
	void signal() 
	{ 
		selection_(selected_); 
	}
	
private:
	string_t selected_;
	std::set<string_t> all_selected_;
	
	mutable boost::signal<void (param_type)> selection_;
	L& list_;
};

} // anmespace WTLx