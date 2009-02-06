
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdAfx.hpp"

#include <functional>

#include <boost/array.hpp>
#include <boost/signals.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/ptr_container/ptr_map.hpp>

#include <boost/detail/iterator.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/random_access_index.hpp>

#include <winstl/controls/listview_sequence.hpp>

#include "Halite.hpp"
#include "halTorrent.hpp"
#include "halEvent.hpp"
#include "WinAPIWaitableTimer.hpp"

#include "UxthemeWrapper.hpp"

#define LVS_EX_DOUBLEBUFFER     0x00010000

#include "WTLx/SelectionManager.hpp"
#include "WTLx/ListViewIterators.hpp"
#include "WTLx/ListViewSortMixin.hpp"
#include "HaliteUpdateLock.hpp"

namespace hal
{
/*
template<typename T>
int compare(const T& l, const T& r)
{
	if (l == r) 
		return 0;
	else if (l > r) 
		return 1;
	else 
		return -1;
}
*/

}
template <class TBase, typename DataType=void*>
class CHaliteSortListViewCtrl : 
	public ATL::CWindowImpl<TBase, WTL::CListViewCtrl>,
	public WTLx::ListViewIterators<CHaliteSortListViewCtrl<TBase, DataType> >,
	public WTLx::ListViewSortMixin<CHaliteSortListViewCtrl<TBase, DataType> >
{
public:
	typedef CHaliteSortListViewCtrl<TBase, DataType> thisClass;
	typedef ATL::CWindowImpl<TBase, WTL::CListViewCtrl> parentClass;
	typedef WTLx::ListViewSortMixin<thisClass> listClass;
	
	class CHaliteHeaderCtrl : public CWindowImpl<CHaliteHeaderCtrl, WTL::CHeaderCtrl>
	{
	public:
		enum { COL_MENU_NAMES = 123, COL_MAX_NAMES = 256 };

		CHaliteHeaderCtrl(thisClass& listView) :
			listView_(listView)
		{}

		BEGIN_MSG_MAP(CHaliteHeaderCtrl)
			REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRClick)
			COMMAND_RANGE_HANDLER(COL_MENU_NAMES, COL_MENU_NAMES+COL_MAX_NAMES, OnMenuNames)

			DEFAULT_REFLECTION_HANDLER()
		END_MSG_MAP()
		
		void Attach(HWND hWndNew)
		{
			ATLASSERT(::IsWindow(hWndNew));
			CWindowImpl<CHaliteHeaderCtrl, WTL::CHeaderCtrl>::SubclassWindow(hWndNew);
		}
		
		LRESULT OnRClick(int i, LPNMHDR pnmh, BOOL&)
		{
			POINT ptPoint;
			GetCursorPos(&ptPoint);
			menu_.TrackPopupMenu(0, ptPoint.x, ptPoint.y, m_hWnd);

			return 0;
		}

		LRESULT OnMenuNames(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
		{		
			ATLASSERT(wID-COL_MENU_NAMES <= GetItemCount());

			bool visible = listView_.OnNameChecked(wID-COL_MENU_NAMES);

			return 0;
		}

		WTL::CMenu& Menu()
		{
			return menu_;
		}
		
	private:
		WTL::CMenu menu_;
		thisClass& listView_;
	};
	
/*	struct ColumnAdapter
	{
		virtual int compare(AdapterType& l, AdapterType& r) = 0;
		virtual std::wstring print(AdapterType& t) = 0;
	};
*/

public:
	typedef WTLx::selection_manager<thisClass, std::wstring> SelectionManager;
	typedef SelectionManager selection_manage_class;
	
	thisClass() :
	//	manager_(*this),
		header_(*this),
		update_lock_(0),
		auto_sort_(false),
		descending_(false),
		sortCol_(-1)
	{		
		if (TBase::LISTVIEW_ID_MENU)
		{
			WTL::CMenuHandle menu;
			BOOL menu_created = menu.LoadMenu(TBase::LISTVIEW_ID_MENU);
			assert(menu_created);	
			
			menu_.Attach(menu.GetSubMenu(0));
		}
	}

	BEGIN_MSG_MAP_EX(thisClass)
		COMMAND_ID_HANDLER(ID_LVM_AUTOSORT, OnAutoSort)
		
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)

		CHAIN_MSG_MAP(listClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void Attach(HWND hWndNew)
	{
		ATLASSERT(::IsWindow(hWndNew));
		parentClass::SubclassWindow(hWndNew);

		TBase* pT = static_cast<TBase*>(this);
		pT->OnAttach();
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0,
		ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		HWND hwnd = parentClass::Create(hWndParent, 
			(RECT &)rect.m_lpRect, szWindowName, dwStyle, dwExStyle, (UINT)MenuOrID.m_hMenu, lpCreateParam);
			
		SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_DOUBLEBUFFER|LVS_EX_SUBITEMIMAGES);
		SetListViewSortMixinExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
		
		return hwnd;
	}
	
	bool SubclassWindow(HWND hwnd)
	{
		if(!parentClass::SubclassWindow(hwnd))
			return false;
			
		SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_DOUBLEBUFFER);
		SetListViewSortMixinExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
		
		return true;
	}		
	
	void SafeLoadFromIni()
	{
		std::vector<wstring> listNames;
		std::vector<int> listWidths;
		std::vector<int> listOrder;
		std::vector<bool> listVisible;

		listNames.assign(list_names_.begin(), list_names_.end());
		listWidths.assign(list_widths_.begin(), list_widths_.end());
		listOrder.assign(list_order_.begin(), list_order_.end());
		listVisible.assign(list_visible_.begin(), list_visible_.end());

		TBase* pT = static_cast<TBase*>(this);
		if (!pT->load_from_ini() || !vector_size_pre_conditions())
		{
			list_names_.assign(listNames.begin(), listNames.end());
			list_widths_.assign(listWidths.begin(), listWidths.end());
			list_order_.assign(listOrder.begin(), listOrder.end());
			list_visible_.assign(listVisible.begin(), listVisible.end());
		}		
	}

	void InitialSetup(WTL::CMenuHandle menu=WTL::CMenuHandle())
	{
		SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP|LVS_EX_DOUBLEBUFFER);
		SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS,SORTLV_USESHELLBITMAPS);

		MENUITEMINFO minfo = {sizeof(MENUITEMINFO)};
		
		if (!menu)
		{
			menu_.CreatePopupMenu();
		}
		else
		{		
			menu_.Attach(menu.GetSubMenu(0));

			minfo.fMask = MIIM_SUBMENU;
			minfo.fType = MFT_SEPARATOR;
			
			menu_.InsertMenuItem(menu_.GetMenuItemCount(), true, &minfo);		
		}

		minfo.fMask = MIIM_STRING|MIIM_ID|MIIM_FTYPE|MIIM_STATE;
		minfo.fType = MFT_STRING;
		minfo.fState = auto_sort_ ? MFS_CHECKED : MFS_UNCHECKED;
		minfo.wID = ID_LVM_AUTOSORT;
		
		std::wstring autoarrange = hal::app().res_wstr(HAL_AUTOSORT);
		minfo.dwTypeData = (LPWSTR)autoarrange.c_str();
		
		menu_.InsertMenuItem(menu_.GetMenuItemCount(), true, &minfo);

		header_.SubclassWindow(this->GetHeader());
		header_.ModifyStyle(0, HDS_DRAGDROP|HDS_FULLDRAG);
		if (header_.Menu().IsNull()) 
			header_.Menu().CreatePopupMenu();
	}
	
	void GetListViewDetails()
	{
		vector_size_pre_conditions();		
		
		for (size_t i=0; i<list_names_.size(); ++i)
		{
			if (list_visible_[i])
				list_widths_[i] = GetColumnWidth(i);
		}
		
		GetColumnOrderArray(list_names_.size(), &list_order_[0]);
		
		sortCol_ = GetSortColumn();
		descending_ = IsSortDescending();	
	}
	
	LRESULT OnAutoSort(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		auto_sort_ = !auto_sort_;
		
		MENUITEMINFO minfo = {sizeof(MENUITEMINFO)};
		
		minfo.fMask = MIIM_STATE;
		minfo.fState = auto_sort_ ? MFS_CHECKED : MFS_UNCHECKED;
		
		menu_.SetMenuItemInfo(ID_LVM_AUTOSORT, false, &minfo);
		
		return 0;
	}

	bool OnNameChecked(int i)
	{
		if (!list_visible_[i])
		{		
			GetColumnOrderArray(list_names_.size(), &list_order_[0]);
			SetColumnWidth(i, list_widths_[i]);

			list_order_.erase(std::find(list_order_.begin(), list_order_.end(), i));
			
			int index = i + std::count(list_visible_.begin()+i, list_visible_.end(), false) - 1;
			list_order_.insert(list_order_.begin()+index, i);

			SetColumnOrderArray(list_names_.size(), &list_order_[0]);
			list_visible_[i] = true;
		}
		else
		{
			list_widths_[i] = GetColumnWidth(i);	
			GetColumnOrderArray(list_names_.size(), &list_order_[0]);

			SetColumnWidth(i, 0);

			list_order_.erase(std::find(list_order_.begin(), list_order_.end(), i));
			list_order_.insert(list_order_.begin(), i);

			SetColumnOrderArray(list_names_.size(), &list_order_[0]);
			list_visible_[i] = false;
		}
		
		MENUITEMINFO minfo = {sizeof(MENUITEMINFO)};	
		minfo.fMask = MIIM_STATE;
		minfo.fState = list_visible_[i] ? MFS_CHECKED : MFS_UNCHECKED;	
		header_.Menu().SetMenuItemInfo(CHaliteHeaderCtrl::COL_MENU_NAMES+i, false, &minfo);
	
		InvalidateRect(NULL, true);
		return list_visible_[i];
	}

	LRESULT OnClick(int, LPNMHDR pnmh, BOOL&)
	{
		return 0;
	}

	LRESULT OnItemChanged(int, LPNMHDR pnmh, BOOL&)
	{		
		hal::try_update_lock<thisClass> lock(*this);
		
//		if (lock) manager_.sync_list(true, true);
		
		return 0;
	}

	LRESULT OnRClick(int i, LPNMHDR pnmh, BOOL&)
	{
		LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)pnmh;
	//	manager_.sync_list(true);
		
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

	bool DoSortItemsExternal(int iCol, bool bDescending = false)
	{
		HAL_DEV_MSG(hal::wform(L"sort_once_ = %1%") % sort_once_);

		sort_once_ = true;

		return true;
	}
	
	bool IsSortOnce(bool mod_value =  true) 
	{ 
		if (!sort_once_)
			return false; 

		if (mod_value) sort_once_ = false;
			return true;
	}

	int AddColumn(LPCTSTR strItem, int nItem, bool visible, int width=-1)
	{
		return AddColumn(strItem, nItem, -1,
			LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
			LVCFMT_LEFT, visible, width);
	}

	int AddColumn(LPCTSTR strItem, int nItem, int nSubItem = -1,
		int nMask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
		int nFmt = LVCFMT_LEFT, bool visible=true, int width=-1)
	{
		int i = parentClass::AddColumn(strItem, nItem, nSubItem, nMask, nFmt);

		if (i == -1) return i;

		if (width != -1) SetColumnWidth(i, width);

		if (header_.Menu().IsNull()) 
			header_.Menu().CreatePopupMenu();

		WTL::CMenuHandle menu = header_.Menu();

		MENUITEMINFO minfo = {sizeof(MENUITEMINFO)};
		minfo.fMask = MIIM_STRING|MIIM_ID|MIIM_FTYPE|MIIM_STATE;
		minfo.fType = MFT_STRING;
		minfo.dwTypeData = (LPTSTR)strItem;
		minfo.wID = CHaliteHeaderCtrl::COL_MENU_NAMES+i;

		if (visible)
			minfo.fState = MFS_CHECKED;
		else
		{
			minfo.fState = MFS_UNCHECKED;
			SetColumnWidth(i, 0);
		}

		int w = GetColumnWidth(i);

		list_names_.push_back(strItem);
		list_visible_.push_back(visible);
		list_widths_.push_back(w);
		list_order_.push_back(i);

		menu.InsertMenuItem(menu.GetMenuItemCount(), false, &minfo);
		return i;
	}

	void SetColumnSortType(int iCol, WORD wType, void* colAdapter=NULL)
	{
		listClass::SetColumnSortType(iCol, wType);
		
	//	if (WTL::LVCOLSORT_CUSTOM == wType)
	//		regColumnAdapter(iCol, colAdapter);
	}

	void SetColumnOrderState()
	{
		while ((int)list_order_.size() < header_.GetItemCount())
			list_order_.push_back(header_.GetItemCount());

		GetColumnOrderArray(list_order_.size(), &list_order_[0]);
	}

	void SetSortState()
	{
		MENUITEMINFO minfo = {sizeof(MENUITEMINFO)};
		
		minfo.fMask = MIIM_STATE;
		minfo.fState = auto_sort_ ? MFS_CHECKED : MFS_UNCHECKED;
		
		menu_.SetMenuItemInfo(ID_LVM_AUTOSORT, false, &minfo);

		if (sortCol_ >= 0 && sortCol_ < m_arrColSortType.GetSize())
			SetSortColumn(sortCol_);
	}

	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		for (size_t i=0; i<list_widths_.size(); ++i)
		{
			if (list_visible_[i])
				list_widths_[i] = GetColumnWidth(i);
		}

		GetColumnOrderArray(list_order_.size(), &list_order_[0]);
		sortCol_ = GetSortColumn();
		descending_ = IsSortDescending();	

		using boost::serialization::make_nvp;

		ar & make_nvp("width", list_widths_);
		ar & make_nvp("order", list_order_);
		ar & make_nvp("visible", list_visible_);
		ar & make_nvp("autoSort", auto_sort_);

		ar & make_nvp("descending", descending_);
		ar & make_nvp("sortCol", sortCol_);

		ar & make_nvp("secondary_descending", listClass::bSecondaryDescending);
		ar & make_nvp("secondary_sort_column", listClass::iSecondarySort);		
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;

		ar & make_nvp("width", list_widths_);
		ar & make_nvp("order", list_order_);
		ar & make_nvp("visible", list_visible_);
		ar & make_nvp("autoSort", auto_sort_);

		ar & make_nvp("descending", descending_);
		ar & make_nvp("sortCol", sortCol_);

		ar & make_nvp("secondary_descending", listClass::bSecondaryDescending);
		ar & make_nvp("secondary_sort_column", listClass::iSecondarySort);		
		
		SetColumnOrderArray(list_order_.size(), &list_order_[0]);

		m_bSortDescending = descending_;
		if (sortCol_ >= 0 && sortCol_ < m_arrColSortType.GetSize())
			SetSortColumn(sortCol_);

		for (size_t i=0; i<list_widths_.size(); ++i)
		{
			SetColumnWidth(i, list_widths_[i]);
			if (!list_visible_[i])
			{
				list_visible_[i] = true;
				OnNameChecked(i);
			}
		}

		SetColumnOrderState();
		SetSortState();
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()

//	const SelectionManager& manager() { return manager_; }
		
	std::vector<int>& ListColumnWidth() { return listColumnWidth_; }
	std::vector<int>& ListColumnOrder() { return listColumnOrder_; }
	
	const std::vector<int>& ListColumnWidth() const { return listColumnWidth_; }
	const std::vector<int>& ListColumnOrder() const { return listColumnOrder_; }
	
	bool CanUpdate() const { return updateLock_ == 0; }
	
//	void clearFocused() { manager_.clear(); }
//	void clearSelected() { manager_.clear_all_selected(); }
//	void clearAll() { manager_.clear_all(); }
	
/*	int CompareItemsCustom(LVCompareParam* pItem1, LVCompareParam* pItem2, int iSortCol)
	{
		hal::mutex_update_lock<thisClass> lock(*this);
		
		TBase* pT = static_cast<TBase*>(this);
		
		AdapterType left = pT->CustomItemConversion(pItem1, iSortCol);
		AdapterType right = pT->CustomItemConversion(pItem2, iSortCol);
		
		return pT->CustomItemComparision(left, right, iSortCol);
	}
*/
	bool AutoSort() { return auto_sort_; }
	
/*	void ConditionallyDoAutoSort()
	{
		int iCol = GetSortColumn();
		if (AutoSort() && iCol >= 0 && iCol < m_arrColSortType.GetSize())
			DoSortItems(iCol, IsSortDescending());	
	}
*/		
/*	ColumnAdapter* getColumnAdapter(size_t index)
	{
		boost::ptr_map<size_t, ColumnAdapter>::iterator 
			i = column_adapters_.find(index);
	
		if (i != column_adapters_.end())
		{
			return i->second;
		}		
		return NULL;
	}
*/
	static bool is_selected (const winstl::listview_sequence::sequence_value_type& v) 
	{ 
		return (v.state() & LVIS_SELECTED) != 0; 
	}

protected:	
/*	inline void* CustomItemConversion(LVCompareParam* param, int iSortCol)
	{
		assert(false);
		return NULL;
	}
	
	int CustomItemComparision(AdapterType left, AdapterType right, int iSortCol)
	{
		ColumnAdapter* pCA = getColumnAdapter(iSortCol);
		
		if (pCA)
			return pCA->compare(left, right);
		else 
			return 0;
	}
*/
/*	void regColumnAdapter(size_t key, ColumnAdapter* colAdapter)
	{
		assert (colAdapter);
		column_adapters_.insert(key, colAdapter);
	}
*/
//	AdapterType convert(const LPLVITEM item);
//	void convert(LPLVITEM item, AdapterType adapter);
	
	mutable int update_lock_;
	mutable hal::mutex_t mutex_;

	friend class hal::mutex_update_lock<thisClass>;	
	friend class hal::try_update_lock<thisClass>;	

//	SelectionManager manager_;
	WTL::CMenu menu_;
	CHaliteHeaderCtrl header_;	

	template<typename T>
	class implicit_reference_wrapper : public boost::reference_wrapper<T>
	{
	private:
		typedef boost::reference_wrapper<T> super;

	public:
		implicit_reference_wrapper(T& t) : super(t) {}
	};

	template <typename T>
	bool implicit_comparison(const implicit_reference_wrapper<T>& l, const implicit_reference_wrapper<T>& r, size_t index, bool ascending) 
	{
		TBase* pT = static_cast<TBase*>(this);

		return pT->less(static_cast<T>(l).second, static_cast<T>(r).second, index, ascending);
	}

	struct by_key {};
	typedef std::pair<bool, DataType> list_pair_t;

	typedef boost::multi_index_container<
		list_pair_t,
		boost::multi_index::indexed_by<
			boost::multi_index::random_access<>,
			boost::multi_index::ordered_unique<
				boost::multi_index::tag<by_key>,  
				boost::multi_index::member<list_pair_t, DataType, &list_pair_t::second> >
		>
	> pair_container;

	typedef typename pair_container::index_iterator<by_key>::type key_iterator;

	int InsertKeyItem(DataType key, LVITEM* pItem)
	{
		key_iterator i = pair_container_.get<by_key>().find(key);
		pair_container::iterator i_pos = pair_container_.project<0>(i);

		int list_item_index = -1;

		if (i != pair_container_.get<by_key>().end())
		{
			list_item_index = std::distance(pair_container_.begin(), i_pos);

			pItem->iItem = list_item_index;

			if ((*i).first)
			{
				pItem->stateMask |= LVIS_SELECTED;
				pItem->state |= LVIS_SELECTED;
			}

			SetItem(pItem);
		}
		else
		{
			list_item_index = pair_container_.size();

			bool selected = (pItem->stateMask & LVIS_SELECTED) && (pItem->state & LVIS_SELECTED);
			list_pair_t lp = std::make_pair(selected, key);

			pair_container_.push_back(lp);

			pItem->iItem = list_item_index;
			int list_pos = InsertItem(pItem);
		}

		ATLASSERT(::IsWindow(m_hWnd));
		return list_item_index;
	}

	bool data_type_comparison(list_pair_t r, list_pair_t l, size_t index, bool ascending)
	{
		TBase* pT = static_cast<TBase*>(this);

		return pT->sort_list_comparison((l).second, (r).second, index, ascending);
	}

	void selection_from_listview()
	{
		foreach(const list_value_type val, std::make_pair(const_begin(), const_end()))
		{
			list_pair_t pi = pair_container_[val.index()];

			if (val.state() & LVIS_SELECTED)
				pi.first = true;
			else
				pi.first = false;
		}
	}

	void sort(size_t index, bool ascending)
	{	
		selection_from_listview();
		std::vector<implicit_reference_wrapper<const list_pair_t> > sv;

		std::copy(pair_container_.begin(), pair_container_.end(), std::back_inserter(sv));

		std::stable_sort(sv.begin(), sv.end(), 
			bind(&thisClass::data_type_comparison, this, _1, _2, index, ascending));

		pair_container_.rearrange(sv.begin());
	}

	void erase_from_list(const list_value_type& val)
	{
		erase_from_list(val.index());
	}

	void erase_from_list(size_t index)
	{
		pair_container_.erase(pair_container_.begin() + index);
		DeleteItem(index);
	}
	
	void erase_from_list(const DataType& str)
	{
		key_iterator i = pair_container_.get<by_key>().find(str);
		pair_container::iterator i_pos = pair_container_.project<0>(i);

		if (i != pair_container_.get<by_key>().end())
		{
			DeleteItem(std::distance(pair_container_.begin(), i_pos));
			pair_container_.erase(i_pos);
		}
	}
	
private:
	mutable pair_container pair_container_;

	bool vector_size_pre_conditions()
	{
		bool ret = (list_names_.size() == list_widths_.size()) &&
			(list_names_.size() == list_order_.size()) &&
			(list_names_.size() == list_visible_.size());

		return ret;
	}	
	
	mutable std::vector<wstring> list_names_;
	mutable std::vector<int> list_widths_;
	mutable std::vector<int> list_order_;
	mutable std::vector<bool> list_visible_;

	mutable bool auto_sort_;
	mutable bool sort_once_;
	mutable bool descending_;
	mutable int sortCol_;
};

template<>
inline const std::wstring hal::to_wstr_shim<const winstl::listview_sequence::sequence_value_type>
	(const winstl::listview_sequence::sequence_value_type& v)
{
	return std::wstring(v.text().c_str());
}

template<>
inline const std::wstring hal::to_wstr_shim<winstl::listview_sequence::sequence_value_type>
	(winstl::listview_sequence::sequence_value_type& v)
{
	return std::wstring(v.text().c_str());
}

namespace boost {
namespace serialization {
	template <class TBase, typename AdapterTypeI>
	struct version< CHaliteSortListViewCtrl<TBase, AdapterTypeI> >
	{
		typedef mpl::int_<2> type;
		typedef mpl::integral_c_tag tag;
		BOOST_STATIC_CONSTANT(unsigned int, value = version::type::value);                                                             
	};
}
}
