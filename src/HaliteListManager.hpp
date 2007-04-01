
#pragma once

#include <vector>
#include <boost/signal.hpp>

template <typename L, typename S=std::string>
class selection_manager : 
	private boost::noncopyable
{
public:
	selection_manager(L& m_list) :
		m_list_(m_list)
	{}
	typedef const S& param_type;
	
	void sync_list(bool list_to_manager)
	{
		if (list_to_manager)
		{	
			all_selected_.clear();
			int itemPos = m_list_.GetSelectionMark();	
		
			if (itemPos != -1)
			{
				boost::array<wchar_t, MAX_PATH> pathBuffer;
				m_list_.GetItemText(itemPos, 0, pathBuffer.c_array(), pathBuffer.size());	
				
				// Multi-Selected
				int total = m_list_.GetItemCount();
				
				for (int i=0; i<total; ++i)
				{
					UINT flags = m_list_.GetItemState(i, LVIS_SELECTED);
					
					if (flags && LVIS_SELECTED)
					{
						m_list_.GetItemText(i, 0, pathBuffer.c_array(), pathBuffer.size());	
						all_selected_.push_back(hal::to_utf8(pathBuffer.data()));
					}
				}
				
				// Single-Selected
				string selected = hal::to_utf8(pathBuffer.data());
				
				if (selected_ != selected)
				{
					selected_ = selected;
					signal();
				}
			}
			else
			{
				selected_ = "";
				signal();
			}
		}
		else
		{
			LV_FINDINFO findInfo = { sizeof(LV_FINDINFO) }; 
			findInfo.flags = LVFI_STRING;
			
			wstring torrent_name = hal::from_utf8(selected_);		
			findInfo.psz = torrent_name.c_str();
			
			int itemPos = m_list_.FindItem(&findInfo, -1);	
			
			if (itemPos != m_list_.GetSelectionMark())
			{
				LVITEM lvi = { LVIF_STATE };
				lvi.state = LVIS_SELECTED;
				lvi.stateMask = LVIS_SELECTED;
				m_list_.SetItemState(itemPos, &lvi);
				m_list_.SetSelectionMark(itemPos);
				signal();
			}
		}
	}
	
	param_type selected() const { return selected_; }
	
	int selectedIndex()
	{
		wstring torrent_name = hal::from_utf8(selected_);	
		LV_FINDINFO findInfo = { sizeof(LV_FINDINFO) }; 	
		findInfo.psz = torrent_name.c_str();
		
		return m_list_.FindItem(&findInfo, -1);		
	}
	
	const std::vector<string>& allSelected() const { return all_selected_; }
	
	void setSelected(const string& sel) 
	{
		selected_ = sel;
		sync_list(false);
	}
	
	void setSelected(int itemPos)
	{
		LVITEM lvi = { LVIF_STATE };
		lvi.state = LVIS_SELECTED;
		lvi.stateMask = LVIS_SELECTED;
		m_list_.SetItemState(itemPos, &lvi);
		m_list_.SetSelectionMark(itemPos);
		sync_list(true);
	}
	
	void clear()
	{
		m_list_.DeleteItem(m_list_.GetSelectionMark());
		
	//	m_list_.SelectItem(0);
		sync_list(true);	
	}
	
	void clearAllSelected()
	{
		int total = m_list_.GetItemCount();
		
		for (int i=total-1; i>=0; --i)
		{
			UINT flags = m_list_.GetItemState(i, LVIS_SELECTED);
			
			if (flags && LVIS_SELECTED)
				m_list_.DeleteItem(i);
		}
		all_selected_.clear();
		
	//	m_list_.SelectItem(0);
		sync_list(true);	
	}
	
	void clearAll()
	{
		m_list_.DeleteAllItems();
		all_selected_.clear();
		sync_list(true);		
	}
	
	void attach(boost::function<void (param_type)> fn) { selection_.connect(fn); }
	void signal() { selection_(selected_); }
	
private:
	S selected_;
	std::vector<S> all_selected_;
	
	boost::signal<void (param_type)> selection_;
	L& m_list_;
};
