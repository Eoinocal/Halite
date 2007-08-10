
#pragma once

#include <boost/array.hpp>
#include <boost/signals.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_free.hpp>

#include "stdAfx.hpp"
#include "halTorrent.hpp"

template <class TBase>
class CHaliteListViewCtrl : public CWindowImpl<TBase, CListViewCtrl>
{
public:
	typedef CHaliteListViewCtrl<TBase> thisClass;
	
protected:
	template <typename L, typename S=std::string>
	class selection_manager : 
		private boost::noncopyable
	{	
	public:
		selection_manager(L& m_list) :
			m_list_(m_list)
		{}
		
		typedef const S& param_type;
		
		void sync_list(bool list_to_manager, bool signal_change=true)
		{
//			hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::Event::info, (wformat(L"%1%, %2% %3%") % hal::from_utf8(selected_) % list_to_manager % signal_change).str().c_str())));
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
						if (signal_change) signal();
					}
				}
				else
				{
					selected_ = "";
					if (signal_change) signal();
				}
			}
			else
			{
			/*	foreach (string name, all_selected_)
				{
					selectMatch(name);
				}
			*/
				int itemPos = selectMatch(selected_);				
			//	if (itemPos != -1) m_list_.SetSelectionMark(itemPos);
				
				if (signal_change) signal();
			}
		}
		
		int selectMatch(const string& name)
		{
			LV_FINDINFO findInfo = { sizeof(LV_FINDINFO) }; 
			findInfo.flags = LVFI_STRING;
			
			wstring torrent_name = hal::from_utf8(name);		
			findInfo.psz = torrent_name.c_str();
			
			int itemPos = m_list_.FindItem(&findInfo, -1);	
			
//			hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::Event::info, (wformat(L"%1%, %2%") % torrent_name % itemPos).str().c_str())));
			
			if (itemPos == -1)
				return itemPos;
				
			UINT flags = m_list_.GetItemState(itemPos, LVIS_SELECTED);
			
			//if (!flags && LVIS_SELECTED)
			{
				LVITEM lvi = { LVIF_STATE };
				lvi.state = LVIS_SELECTED;
				lvi.stateMask = LVIS_SELECTED;
				m_list_.SetItemState(itemPos, &lvi);
			}
		
			return itemPos;
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
		
		void signal() 
		{ 
			selection_(selected_); 
		}
		
	private:
		S selected_;
		std::vector<S> all_selected_;
		
		boost::signal<void (param_type)> selection_;
		L& m_list_;
	};
	
	class CHaliteHeaderCtrl : public CWindowImpl<CHaliteHeaderCtrl, CHeaderCtrl>
	{
	public:
		BEGIN_MSG_MAP(CHaliteHeaderCtrl)
			REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRClick)
		END_MSG_MAP()
		
		void Attach(HWND hWndNew)
		{
			ATLASSERT(::IsWindow(hWndNew));
			CWindowImpl<CHaliteHeaderCtrl, CHeaderCtrl>::SubclassWindow(hWndNew);

			menu_.CreatePopupMenu();
			
			MENUITEMINFO minfo = {sizeof(MENUITEMINFO)};
			minfo.fMask = MIIM_STRING|MIIM_ID|MIIM_FTYPE|MIIM_STATE;
			minfo.fType = MFT_STRING;
			minfo.fState = MFS_CHECKED;
			minfo.dwTypeData = L"Hello";
			
			menu_.InsertMenuItem(0, false, &minfo);
//			TBase* pT = static_cast<TBase*>(this);
//			pT->OnAttach();
		}
		
		LRESULT OnRClick(int i, LPNMHDR pnmh, BOOL&)
		{
			POINT ptPoint;
			GetCursorPos(&ptPoint);
			menu_.TrackPopupMenu(0, ptPoint.x, ptPoint.y, m_hWnd);

			return 0;
		}
		
	private:
		WTL::CMenu menu_;
	};
	
	template<typename T>
	class column_type
	{
	public:
		virtual ~column_type() = 0		
		virtual bool less(cont T&, const T&) = 0
		virtual std::string str() = 0;
	};
	
	// EG T is a TorrentDetail, so have array column_types

public:
	typedef selection_manager<thisClass> selection_manage_class;
	
	enum sortDirection
	{
		none,
		ascending,
		descending
	};
	
	CHaliteListViewCtrl<TBase>() :
		sortingDirection_(CHaliteListViewCtrl<TBase>::none),
		sortedColmun_(0),
		manager_(*this)
	{
		if (TBase::LISTVIEW_ID_MENU)
		{
			BOOL menu_created = menu_.LoadMenu(TBase::LISTVIEW_ID_MENU);
			assert(menu_created);	
		}

		wstring column_names = hal::app().res_wstr(TBase::LISTVIEW_ID_COLUMNNAMES);
		boost::split(names_, column_names, boost::is_any_of(L";"));
		
		wstring column_widths = hal::app().res_wstr(TBase::LISTVIEW_ID_COLUMNWIDTHS);
		std::vector<wstring> widths;
		boost::split(widths, column_widths, boost::is_any_of(L";"));
				
		listColumnWidth_.reserve(names_.size());	
		listColumnOrder_.reserve(names_.size());
		
		for (size_t i=0; i<names_.size(); ++i)
		{
			listColumnWidth_.push_back(lexical_cast<int>(widths[i]));
			listColumnOrder_.push_back(i);
		}	
	}

	BEGIN_MSG_MAP_EX(CHaliteListViewCtrl<TBase>)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK , OnColClick)

		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void Attach(HWND hWndNew)
	{
		ATLASSERT(::IsWindow(hWndNew));
        CWindowImpl<TBase, CListViewCtrl>::SubclassWindow(hWndNew);

		TBase* pT = static_cast<TBase*>(this);
		pT->OnAttach();
	}
	
	void SetListViewDetails()
	{
		vectorSizePreConditions();
		
		header_.Attach(this->GetHeader());
		header_.ModifyStyle(0, HDS_DRAGDROP|HDS_FULLDRAG);
			
		foreach (wstring name, names_)
		{
			AddColumn(name.c_str(), header_.GetItemCount());
		}		

		for (unsigned i=0; i<names_.size(); ++i)
			SetColumnWidth(i, listColumnWidth_[i]);
		
		SetColumnOrderArray(names_.size(), &listColumnOrder_[0]);	
	}
	
	template<std::size_t Size>
	void SetDefaults(array<int, Size> a)
	{
		assert (Size == names_.size());
		vectorSizePreConditions();
		
		for (size_t i=0; i<names_.size(); ++i)
		{
			listColumnWidth_[i] = a[i];
			listColumnOrder_[i] = i;
		}		
	}
	
	// Should probably make this redundant!!
	void GetListViewDetails()
	{
		vectorSizePreConditions();
		
		GetColumnOrderArray(names_.size(), &listColumnOrder_[0]);
		
		for (size_t i=0; i<names_.size(); ++i)
			listColumnWidth_[i] = GetColumnWidth(i);	
	}

	LRESULT OnClick(int, LPNMHDR pnmh, BOOL&)
	{
	//	manager().sync_list(true);

		return 0;
	}

	LRESULT OnItemChanged(int, LPNMHDR pnmh, BOOL&)
	{
		manager().sync_list(true);

		return 0;
	}

	LRESULT OnRClick(int i, LPNMHDR pnmh, BOOL&)
	{
		LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)pnmh;
		manager().sync_list(true);
		
		if (TBase::LISTVIEW_ID_MENU)
		{
			assert (menu_.IsMenu());
			CMenuHandle sMenu = menu_.GetSubMenu(0);
			assert (sMenu.IsMenu());
	
			POINT ptPoint;
			GetCursorPos(&ptPoint);
			sMenu.TrackPopupMenu(0, ptPoint.x, ptPoint.y, m_hWnd);
		}

		return 0;
	}

	LRESULT OnColClick(int i, LPNMHDR pnmh, BOOL&)
	{
		LPNMLISTVIEW pnlv = (LPNMLISTVIEW)pnmh;
		
		MessageBox((lexical_cast<wstring>(pnlv->iSubItem)).c_str(), L"Hi", 0);
		return 0;
	}
	
	friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::make_nvp("width", listColumnWidth_);
        ar & boost::serialization::make_nvp("order", listColumnOrder_);
    }

	selection_manager<CHaliteListViewCtrl>& manager() { return manager_; }
	
	size_t sortedColmun() { return sortedColmun_; }
	sortDirection sortingDirection() { return sortingDirection_; }
	
	std::vector<int>& listColumnWidth() { return listColumnWidth_; }
	std::vector<int>& listColumnOrder() { return listColumnOrder_; }
	
	const std::vector<int>& listColumnWidth() const { return listColumnWidth_; }
	const std::vector<int>& listColumnOrder() const { return listColumnOrder_; }
	
private:
	selection_manager<CHaliteListViewCtrl> manager_;
	
	void vectorSizePreConditions()
	{
		if (listColumnWidth_.size() != names_.size())
		{
			listColumnWidth_.clear();
			listColumnWidth_.insert(listColumnWidth_.end(), names_.size(), 50);	
		}
		
		if (listColumnOrder_.size() != names_.size())
		{		
			listColumnOrder_.clear();
			listColumnOrder_.insert(listColumnOrder_.end(), names_.size(), 0);
		}		
	}
	
	sortDirection sortingDirection_;
	size_t sortedColmun_;
	
	WTL::CMenu menu_;
	CHaliteHeaderCtrl header_;
	std::vector<wstring> names_;
	std::vector<int> listColumnWidth_;
	std::vector<int> listColumnOrder_;
};
