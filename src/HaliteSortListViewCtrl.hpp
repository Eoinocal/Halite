
#pragma once

#include <boost/array.hpp>
#include <boost/signals.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/ptr_container/ptr_map.hpp>

#include "stdAfx.hpp"
#include "../res/resource.h"
#include "halTorrent.hpp"
#include "WinAPIWaitableTimer.hpp"

#include "UxthemeWrapper.hpp"

#define LVS_EX_DOUBLEBUFFER     0x00010000

template<class T>
class UpdateLock
{
public:
	UpdateLock(T& window) :
		window_(window)
	{
		++window_.updateLock_;
//		window_.LockWindowUpdate(true);
	}
	
	~UpdateLock()
	{
		if (!--window_.updateLock_)
			unlock();
	}
	
	void unlock()
	{
//		window_.LockWindowUpdate(false);
//		window_.InvalidateRect(NULL, true);
	}
	
private:
	T& window_;
};

template<class T>
class TryUpdateLock
{
public:
	TryUpdateLock(T& window) :
		window_(window),
		locked_(false)
	{
		if (0 == window_.updateLock_)
		{
			locked_=  true;
			++window_.updateLock_;
//			window_.LockWindowUpdate(true);
		}
	}
	
	~TryUpdateLock()
	{
		if (locked_ && !--window_.updateLock_)
			unlock();
	}
	
	void unlock()
	{
//		window_.LockWindowUpdate(false);
//		window_.InvalidateRect(NULL, true);
	}
	
	operator bool() const { return locked_; }
	
private:
	T& window_;
	bool locked_;
};

template <class TBase, typename adapterType=void*, size_t N=-1>
class CHaliteSortListViewCtrl : 
	public CSortListViewCtrlImpl<CHaliteSortListViewCtrl<TBase, adapterType, N> >
{
public:
	typedef CHaliteSortListViewCtrl<TBase, adapterType, N> thisClass;
	typedef CSortListViewCtrlImpl<thisClass> parentClass;
	
	class selection_manager : 
		private boost::noncopyable
	{	
	public:
		selection_manager(thisClass& m_list) :
			m_list_(m_list)
		{}
		
		typedef std::wstring string_t; 
		typedef const string_t& param_type;
		
		void sync_list(bool list_to_manager, bool signal_change=true)
		{
			if (list_to_manager)
			{	
				if (listToManager() && signal_change) signal();
			}
			else
			{
				if (managerToList() && signal_change) signal();
			}
		}
		
		bool listToManager()
		{
			boost::array<wchar_t, MAX_PATH> pathBuffer;
			std::set<string_t> all_selected;
			string_t selected = L"";
			
			bool do_signal = false;
			
			int total = m_list_.GetItemCount();
			
			for (int i=0; i<total; ++i)
			{
				UINT flags = m_list_.GetItemState(i, LVIS_SELECTED);
				
				if (flags && LVIS_SELECTED)
				{
					m_list_.GetItemText(i, 0, pathBuffer.c_array(), pathBuffer.size());	
					all_selected.insert(pathBuffer.data());
				}
				if (flags && LVIS_FOCUSED)
				{
					selected = pathBuffer.data();
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
			UpdateLock<thisClass> lock(m_list_);
			
			boost::array<wchar_t, MAX_PATH> pathBuffer;
			LV_FINDINFO findInfo = { sizeof(LV_FINDINFO) }; 
			findInfo.flags = LVFI_STRING;
			
			bool do_signal = true; // Always signal for now!
			
			int total = m_list_.GetItemCount();
			
			for (int i=0; i<total; ++i)
			{
				m_list_.GetItemText(i, 0, pathBuffer.c_array(), pathBuffer.size());
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
				
				m_list_.SetItemState(i, &lvi);
			}			
			
			return do_signal;
		}
		
		int selectMatch(const string_t& name)
		{
			LV_FINDINFO findInfo = { sizeof(LV_FINDINFO) }; 
			findInfo.flags = LVFI_STRING;
					
			findInfo.psz = name.c_str();
			
			int itemPos = m_list_.FindItem(&findInfo, -1);	
						
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
		
		int selectedIndex() const
		{
			LV_FINDINFO findInfo = { sizeof(LV_FINDINFO) }; 	
			findInfo.psz = selected_.c_str();
			
			return m_list_.FindItem(&findInfo, -1);		
		}
		
		const std::set<string_t>& allSelected() const { return all_selected_; }
		
		void setSelected(const string_t& sel) 
		{
			selected_ = sel;
		}
		
		void setSelected(int itemPos)
		{		
			hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::Event::info, (wformat(L"Set Selected %1%") % itemPos).str().c_str())));

			LVITEM lvi = { LVIF_STATE };
			lvi.state = LVIS_SELECTED|LVIS_FOCUSED;
			lvi.stateMask = LVIS_SELECTED|LVIS_FOCUSED;
			m_list_.SetItemState(itemPos, &lvi);
			
			m_list_.SetSelectionMark(itemPos);
			
			sync_list(true);
		}
		
		void clear()
		{
			// Prevent changing states from signaling another sync
			UpdateLock<thisClass> lock(m_list_);
			
			m_list_.DeleteItem(selectedIndex());
			
			sync_list(true);	
		}
		
		void clear_all_selected()
		{
			// Prevent changing states from signaling another sync
			UpdateLock<thisClass> lock(m_list_);
			
			int total = m_list_.GetItemCount();
			
			for (int i=total-1; i>=0; --i)
			{
				UINT flags = m_list_.GetItemState(i, LVIS_SELECTED);
				
				if (flags && LVIS_SELECTED)
					m_list_.DeleteItem(i);
			}
			all_selected_.clear();
			
			sync_list(true);	
		}
		
		void clear_all()
		{
			// Prevent changing states from signaling another sync
			UpdateLock<thisClass> lock(m_list_);
			
			m_list_.DeleteAllItems();
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
		thisClass& m_list_;
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
	
	struct ColumnAdapter
	{
		virtual bool less(adapterType& l, adapterType& r) = 0;
		virtual std::wstring print(adapterType& t) = 0;
	};

public:
	typedef selection_manager SelectionManager;
	typedef SelectionManager selection_manage_class;
	
	thisClass(bool resMenu=true, bool resNames=true, bool resWidthsAndOrder=true) :
		manager_(*this),
		updateLock_(0),
		autoSort_(false),
		descending_(false),
		sortCol_(-1)
	{		
		if (resMenu && TBase::LISTVIEW_ID_MENU)
		{
			CMenuHandle menu;
			BOOL menu_created = menu.LoadMenu(TBase::LISTVIEW_ID_MENU);
			assert(menu_created);	
			
			menu_.Attach(menu.GetSubMenu(0));
		}

		if (resNames)
		{
			wstring column_names = hal::app().res_wstr(TBase::LISTVIEW_ID_COLUMNNAMES);
			boost::split(listNames_, column_names, boost::is_any_of(L";"));
		}
		
		if (resWidthsAndOrder)
		{
			wstring column_widths = hal::app().res_wstr(TBase::LISTVIEW_ID_COLUMNWIDTHS);
			std::vector<wstring> widths;
			boost::split(widths, column_widths, boost::is_any_of(L";"));
			
			listWidths_.reserve(listNames_.size());	
			listOrder_.reserve(listNames_.size());
			
			for (size_t i=0; i<listNames_.size(); ++i)
			{
				listWidths_.push_back(lexical_cast<int>(widths[i]));
				listOrder_.push_back(i);
			}
		}
	}

	BEGIN_MSG_MAP_EX(thisClass)
		COMMAND_ID_HANDLER(ID_LVM_AUTOSORT, OnAutoSort)
		
	//	REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
	//	REFLECTED_NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK , OnColClick)

		DEFAULT_REFLECTION_HANDLER()
		CHAIN_MSG_MAP(parentClass)
	END_MSG_MAP()

	void Attach(HWND hWndNew)
	{
		ATLASSERT(::IsWindow(hWndNew));
        parentClass::SubclassWindow(hWndNew);

		TBase* pT = static_cast<TBase*>(this);
		pT->OnAttach();
	}
	
	bool SubclassWindow(HWND hwnd)
	{
		if(!parentClass::SubclassWindow(hwnd))
			return false;
			
		SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_DOUBLEBUFFER);
		SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
		
		return true;
	}		
	
	void SetListViewDetails()
	{
		vectorSizePreConditions();
		
		header_.Attach(this->GetHeader());
		header_.ModifyStyle(0, HDS_DRAGDROP|HDS_FULLDRAG);
			
		foreach (wstring name, listNames_)
		{
			int i = header_.GetItemCount();
			
			AddColumn(name.c_str(), i);
		}		

		for (unsigned i=0; i<listNames_.size(); ++i)
			SetColumnWidth(i, listOrder_[i]);
		
		SetColumnOrderArray(listOrder_.size(), &listOrder_[0]);	
	}
	
	template<typename N, typename W, typename O, typename P>
	void SetDefaults(N nameList, W widthList, O orderList, P visibleList, bool autoSort=false)
	{
		listNames_.assign(nameList.begin(), nameList.end());
		listWidths_.assign(widthList.begin(), widthList.end());
		listOrder_.assign(orderList.begin(), orderList.end());
		listVisible_.assign(visibleList.begin(), visibleList.end());
		
		autoSort_ = autoSort;
	}
	
	template<std::size_t Size>
	void SetDefaults(array<int, Size> a)
	{
		assert (Size == listNames_.size());
		vectorSizePreConditions();
		
		for (size_t i=0; i<listNames_.size(); ++i)
		{
			listWidths_[i] = a[i];
			listOrder_[i] = i;
		}		
	}
	
	void ApplyDetails()
	{
		vectorSizePreConditions();
		
		MENUITEMINFO minfo = {sizeof(MENUITEMINFO)};
		
		if (!menu_)
		{
			menu_.CreatePopupMenu();
		}
		else
		{				
			minfo.fMask = MIIM_SUBMENU;
			minfo.fType = MFT_SEPARATOR;
			
			menu_.InsertMenuItem(menu_.GetMenuItemCount(), true, &minfo);		
		}

		minfo.fMask = MIIM_STRING|MIIM_ID|MIIM_FTYPE|MIIM_STATE;
		minfo.fType = MFT_STRING;
		minfo.fState = autoSort_ ? MFS_CHECKED : MFS_UNCHECKED;
		minfo.wID = ID_LVM_AUTOSORT;
		
		wstring autoarrange = hal::app().res_wstr(HAL_AUTOSORT);
		minfo.dwTypeData = (LPWSTR)autoarrange.c_str();
		
		menu_.InsertMenuItem(menu_.GetMenuItemCount(), true, &minfo);
		
		header_.Attach(this->GetHeader());
		header_.ModifyStyle(0, HDS_DRAGDROP|HDS_FULLDRAG);
		
		for (int i = header_.GetItemCount(); i<listNames_.size(); i = header_.GetItemCount())
		{
			AddColumn(listNames_[i].c_str(), i);
			SetColumnWidth(i, listWidths_[i]);
		}
		
		SetColumnOrderArray(listNames_.size(), &listOrder_[0]);
		
		m_bSortDescending = descending_;
		if (sortCol_ >= 0 && sortCol_ < m_arrColSortType.GetSize())
			SetSortColumn(sortCol_);
	}
	
	void GetListViewDetails()
	{
		vectorSizePreConditions();		
		
		for (size_t i=0; i<listNames_.size(); ++i)
		{
			listWidths_[i] = GetColumnWidth(i);
		}
		
		GetColumnOrderArray(listNames_.size(), &listOrder_[0]);
		
		sortCol_ = GetSortColumn();
		descending_ = IsSortDescending();	
	}
	
	LRESULT OnAutoSort(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		autoSort_ = !autoSort_;
		
		MENUITEMINFO minfo = {sizeof(MENUITEMINFO)};
		
		minfo.fMask = MIIM_STATE;
		minfo.fState = autoSort_ ? MFS_CHECKED : MFS_UNCHECKED;
		
		menu_.SetMenuItemInfo(ID_LVM_AUTOSORT, false, &minfo);
		
		return 0;
	}

	LRESULT OnClick(int, LPNMHDR pnmh, BOOL&)
	{
		return 0;
	}

	LRESULT OnItemChanged(int, LPNMHDR pnmh, BOOL&)
	{		
		TryUpdateLock<thisClass> lock(*this);
		if (lock) 
			manager_.sync_list(true, true);
			//syncTimer_.reset(50, 0, bind(&thisClass::syncTimeout, this));
		
		return 0;
	}

	LRESULT OnRClick(int i, LPNMHDR pnmh, BOOL&)
	{
		LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)pnmh;
		manager_.sync_list(true);
		
		if (menu_)
		{
			assert (menu_.IsMenu());
	
			POINT ptPoint;
			GetCursorPos(&ptPoint);
			menu_.TrackPopupMenu(0, ptPoint.x, ptPoint.y, m_hWnd);
		}

		return 0;
	}

	LRESULT OnColClick(int i, LPNMHDR pnmh, BOOL&)
	{
		LPNMLISTVIEW pnlv = (LPNMLISTVIEW)pnmh;
		
		MessageBox((lexical_cast<wstring>(pnlv->iSubItem)).c_str(), L"Hi", 0);
		return 0;
	}
	
	void SetColumnSortType(int iCol, WORD wType, ColumnAdapter* colAdapter=NULL)
	{
		parentClass::SetColumnSortType(iCol, wType);
		
		if (LVCOLSORT_CUSTOM == wType)
			regColumnAdapter(iCol, colAdapter);
	}
	
	friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		if (version >= 1)
		{
			ar & boost::serialization::make_nvp("width", listWidths_);
			ar & boost::serialization::make_nvp("order", listOrder_);
			ar & boost::serialization::make_nvp("visible", listVisible_);
			ar & boost::serialization::make_nvp("autoSort", autoSort_);
		}
		if (version >= 2)
		{
			ar & boost::serialization::make_nvp("descending", descending_);
			ar & boost::serialization::make_nvp("sortCol", sortCol_);
		}
    }

	const SelectionManager& manager() { return manager_; }
		
	std::vector<int>& listColumnWidth() { return listColumnWidth_; }
	std::vector<int>& listColumnOrder() { return listColumnOrder_; }
	
	const std::vector<int>& listColumnWidth() const { return listColumnWidth_; }
	const std::vector<int>& listColumnOrder() const { return listColumnOrder_; }
	
	bool canUpdate() const { return updateLock_ == 0; }
	
	void clearFocused() { manager_.clear(); }
	void clearSelected() { manager_.clear_all_selected(); }
	void clearAll() { manager_.clear_all(); }
	
	int CompareItemsCustom(LVCompareParam* pItem1, LVCompareParam* pItem2, int iSortCol)
	{
		UpdateLock<thisClass> lock(*this);
		
		TBase* pT = static_cast<TBase*>(this);
		
		adapterType left = pT->CustomItemConversion(pItem1, iSortCol);
		adapterType right = pT->CustomItemConversion(pItem2, iSortCol);
		
		return pT->CustomItemComparision(left, right, iSortCol);
	}
	
	bool autoSort() { return autoSort_; }
	
protected:	
	inline void* CustomItemConversion(LVCompareParam* param, int iSortCol)
	{
		assert(false);
		return NULL;
	}
	
	int CustomItemComparision(adapterType left, adapterType right, int iSortCol)
	{
		ColumnAdapter* pCA = getColumnAdapter(iSortCol);
		
		if (pCA)
			return (pCA->less(left, right)) ? 1 : -1;
		else 
			return 0;
	}
	
	void regColumnAdapter(size_t key, ColumnAdapter* colAdapter)
	{
		assert (colAdapter);
		columnAdapters_.insert(key, colAdapter);
	}
	
	ColumnAdapter* getColumnAdapter(size_t index)
	{
		boost::ptr_map<size_t, ColumnAdapter>::iterator 
			i = columnAdapters_.find(index);
	
		if (i != columnAdapters_.end())
		{
			return i->second;
		}		
		return NULL;
	}
	
	void ConditionallyDoAutoSort()
	{
		int iCol = GetSortColumn();
		if (autoSort() && iCol >= 0 && iCol < m_arrColSortType.GetSize())
			DoSortItems(iCol, IsSortDescending());	
	}
	
	SelectionManager manager_;
	
private:
	void vectorSizePreConditions()
	{
/*		if (listColumnWidth_.size() != names_.size())
		{
			listColumnWidth_.clear();
			listColumnWidth_.insert(listColumnWidth_.end(), names_.size(), 50);	
		}
		
		if (listColumnOrder_.size() != names_.size())
		{		
			listColumnOrder_.clear();
			listColumnOrder_.insert(listColumnOrder_.end(), names_.size(), 0);
		}
*/
	}
	
	void syncTimeout()
	{
	//	hal::event().post(shared_ptr<hal::EventDetail>
	//		(new hal::EventDebug(hal::Event::info, (wformat(L"Signaled")).str().c_str())));
		
		manager_.sync_list(true, true);
	}
	
	WTL::CMenu menu_;
	CHaliteHeaderCtrl header_;	
	
	std::vector<wstring> listNames_;
	std::vector<int> listWidths_;
	std::vector<int> listOrder_;
	std::vector<bool> listVisible_;
	bool autoSort_;
	bool descending_;
	int sortCol_;
	
	int updateLock_;
	friend class UpdateLock<thisClass>;	
	friend class TryUpdateLock<thisClass>;		
	
	boost::ptr_map<size_t, ColumnAdapter> columnAdapters_;
	
	WinAPIWaitableTimer syncTimer_;
};

namespace boost {
namespace serialization {
template <class TBase, typename adapterType, size_t N>
struct version< CHaliteSortListViewCtrl<TBase, adapterType, N> >
{
    typedef mpl::int_<2> type;
    typedef mpl::integral_c_tag tag;
    BOOST_STATIC_CONSTANT(unsigned int, value = version::type::value);                                                             
};
}
}
