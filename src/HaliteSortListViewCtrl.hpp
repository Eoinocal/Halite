
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdAfx.hpp"

#include <functional>

#include <boost/array.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/ptr_container/ptr_map.hpp>

#include <boost/detail/iterator.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/random_access_index.hpp>

#include "Halite.hpp"
#include "halTorrent.hpp"
#include "halEvent.hpp"
#include "WinAPIWaitableTimer.hpp"

#include "UxthemeWrapper.hpp"

#define LVS_EX_DOUBLEBUFFER     0x00010000

#include "WTLx/SelectionManager.hpp"
#include "WTLx/ListViewIterators.hpp"
#include "WTLx/ListViewSortMixin.hpp"
#include "WTLx/UpdateLockable.hpp"

template <class TBase, typename DataType=void*>
class CHaliteSortListViewCtrl : 
	public ATL::CWindowImpl<TBase, WTL::CListViewCtrl>,
	public WTLx::ListViewIterators<CHaliteSortListViewCtrl<TBase, DataType> >,
	public WTLx::ListViewSortMixin<CHaliteSortListViewCtrl<TBase, DataType> >,
	public hal::update_lockable<CHaliteSortListViewCtrl<TBase, DataType> >
{
public:
	typedef CHaliteSortListViewCtrl<TBase, DataType> this_class_t;
	typedef ATL::CWindowImpl<TBase, WTL::CListViewCtrl> parent_class_t;
	typedef WTLx::ListViewSortMixin<this_class_t> list_class_t;

	typedef DataType list_class_data_t;
	
	class CHaliteHeaderCtrl : public CWindowImpl<CHaliteHeaderCtrl, WTL::CHeaderCtrl>
	{
	public:
		enum { COL_MENU_NAMES = 123, COL_MAX_NAMES = 256 };

		CHaliteHeaderCtrl(this_class_t& listView) :
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
		this_class_t& listView_;
	};
	
public:
	typedef WTLx::selection_manager<this_class_t, std::wstring> SelectionManager;
	typedef SelectionManager selection_manage_class;
	
	this_class_t() :
		header_(*this),
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

	BEGIN_MSG_MAP_EX(this_class_t)
		try
		{
		COMMAND_ID_HANDLER(ID_LVM_AUTOSORT, OnAutoSort)
		
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in CHaliteSortListViewCtrl MSG_MAP")

		CHAIN_MSG_MAP(list_class_t)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void Attach(HWND hWndNew)
	{
		ATLASSERT(::IsWindow(hWndNew));
		parent_class_t::SubclassWindow(hWndNew);

		TBase* pT = static_cast<TBase*>(this);
		pT->OnAttach();
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0,
		ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		HWND hwnd = parent_class_t::Create(hWndParent, 
			(RECT &)rect.m_lpRect, szWindowName, dwStyle, dwExStyle, (UINT)MenuOrID.m_hMenu, lpCreateParam);
			
		SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_DOUBLEBUFFER|LVS_EX_SUBITEMIMAGES);
		SetListViewSortMixinExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
		
		return hwnd;
	}
	
	bool SubclassWindow(HWND hwnd)
	{
		if(!parent_class_t::SubclassWindow(hwnd))
			return false;
			
		SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_DOUBLEBUFFER);
		SetListViewSortMixinExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
		
		return true;
	}		
	
	void safe_load_from_ini()
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
		pT->load_from_ini();
		if (!vector_size_pre_conditions())
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
				list_widths_[i] = GetColumnWidth(static_cast<int>(i));
		}
		
		GetColumnOrderArray(static_cast<int>(list_names_.size()), &list_order_[0]);
		
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
			GetColumnOrderArray(static_cast<int>(list_names_.size()), &list_order_[0]);

			int w = (list_widths_[i] == 0) ? 60 : list_widths_[i];
			SetColumnWidth(i, w);

			list_order_.erase(std::find(list_order_.begin(), list_order_.end(), i));
			
			int index = static_cast<int>(i + std::count(list_visible_.begin()+i, list_visible_.end(), false) - 1);
			list_order_.insert(list_order_.begin()+index, i);

			SetColumnOrderArray(static_cast<int>(list_names_.size()), &list_order_[0]);
			list_visible_[i] = true;
		}
		else
		{
			list_widths_[i] = GetColumnWidth(i);	
			GetColumnOrderArray(static_cast<int>(list_names_.size()), &list_order_[0]);

			SetColumnWidth(i, 0);

			list_order_.erase(std::find(list_order_.begin(), list_order_.end(), i));
			list_order_.insert(list_order_.begin(), i);

			SetColumnOrderArray(static_cast<int>(list_names_.size()), &list_order_[0]);
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
		hal::try_update_lock<this_class_t> lock(this);
		
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
		HAL_DEV_SORT_MSG(hal::wform(L"sort_once_ = %1%") % sort_once_);

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

	int AddColumn(LPCTSTR strItem, int nItem, bool visible, int width=-1, int fmt=LVCFMT_LEFT)
	{
		return AddColumn(strItem, nItem, -1,
			LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
			fmt, visible, width);
	}

	int AddColumn(LPCTSTR strItem, int nItem, int nSubItem = -1,
		int nMask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
		int nFmt = LVCFMT_LEFT, bool visible=true, int width=-1)
	{
		int i = parent_class_t::AddColumn(strItem, nItem, nSubItem, nMask, nFmt);

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
		list_class_t::SetColumnSortType(iCol, wType);
		
	//	if (WTL::LVCOLSORT_CUSTOM == wType)
	//		regColumnAdapter(iCol, colAdapter);
	}

	void SetColumnOrderState()
	{
		while ((int)list_order_.size() < header_.GetItemCount())
			list_order_.push_back(header_.GetItemCount());

		GetColumnOrderArray(static_cast<int>(list_order_.size()), &list_order_[0]);
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
				list_widths_[i] = GetColumnWidth(numeric_cast<int>(i));
		}

		GetColumnOrderArray(static_cast<int>(list_order_.size()), &list_order_[0]);
		sortCol_ = GetSortColumn();
		descending_ = IsSortDescending();	

		using boost::serialization::make_nvp;

		ar & make_nvp("width", list_widths_);
		ar & make_nvp("order", list_order_);
		ar & make_nvp("visible", list_visible_);
		ar & make_nvp("auto_sort", auto_sort_);

		ar & make_nvp("descending", descending_);
		ar & make_nvp("sort_column", sortCol_);

		ar & make_nvp("secondary_descending", list_class_t::bSecondaryDescending);
		ar & make_nvp("secondary_sort_column", list_class_t::iSecondarySort);		
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;

		ar & make_nvp("width", list_widths_);
		ar & make_nvp("order", list_order_);
		ar & make_nvp("visible", list_visible_);
		ar & make_nvp("auto_sort", auto_sort_);

		ar & make_nvp("descending", descending_);
		ar & make_nvp("sort_column", sortCol_);

		ar & make_nvp("secondary_descending", list_class_t::bSecondaryDescending);
		ar & make_nvp("secondary_sort_column", list_class_t::iSecondarySort);		
		
		SetColumnOrderArray(numeric_cast<int>(list_order_.size()), &list_order_[0]);

		m_bSortDescending = descending_;
		if (sortCol_ >= 0 && sortCol_ < m_arrColSortType.GetSize())
			SetSortColumn(sortCol_);

		for (size_t i=0; i<list_widths_.size(); ++i)
		{
			SetColumnWidth(numeric_cast<int>(i), list_widths_[i]);
			if (!list_visible_[i])
			{
				list_visible_[i] = true;
				OnNameChecked(numeric_cast<int>(i));
			}
		}

		SetColumnOrderState();
		SetSortState();
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()
		
	std::vector<int>& ListColumnWidth() { return listColumnWidth_; }
	std::vector<int>& ListColumnOrder() { return listColumnOrder_; }
	
	const std::vector<int>& ListColumnWidth() const { return listColumnWidth_; }
	const std::vector<int>& ListColumnOrder() const { return listColumnOrder_; }
	
	bool AutoSort() { return auto_sort_; }
	
/*	static bool is_selected (const winstl::listview_sequence::sequence_value_type& v) 
	{ 
		return (v.state() & LVIS_SELECTED) != 0; 
	}
*/
protected:		
	mutable hal::mutex_t mutex_;

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

	template <typename T>
	struct first_mutable_pair
	{
		first_mutable_pair(const bool& f, const T& s) :
			first(f),
			second(s)
		{}

		mutable bool first;
		T second;
	};
	typedef first_mutable_pair<DataType> list_pair_t;

	typedef boost::multi_index_container<
		list_pair_t,
		boost::multi_index::indexed_by<
			boost::multi_index::random_access<>,
			boost::multi_index::ordered_unique<
				boost::multi_index::tag<by_key>,  
				boost::multi_index::member<list_pair_t, DataType, &list_pair_t::second> >
		>
	> pair_container;

//	typedef typename pair_container::index_iterator<by_key>::type key_iterator;

	int set_key(DataType key)
	{
		LVITEM lvItem = { 0 };
		lvItem.mask = LVIF_STATE;
		lvItem.stateMask = LVIS_SELECTED;
		lvItem.state = 0;
		lvItem.iSubItem = 0;

		return set_key_item(key, &lvItem);
	}

	template<typename T>
	void set_keys(T s)
	{
		BOOST_FOREACH (const DataType& key, s)
		{
			set_key(key);
		}
	}

	int set_key_item(DataType key, LVITEM* pItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		auto i = pair_container_.get<by_key>().find(key);
		boost::optional<size_t> index = index_from_key(key);

		if (index)
		{
			HAL_DEV_SORT_MSG(hal::wform(L"Existing index %1%, key %3%, selected %2%") % *index % (*i).first % key);

			if ((*i).first)
				SetItemState(numeric_cast<int>(*index), LVIS_SELECTED, LVIS_SELECTED);
			else
				SetItemState(numeric_cast<int>(*index), 0, LVIS_SELECTED);

			return numeric_cast<int>(*index);
		}
		else
		{
			int list_item_index = numeric_cast<int>(pair_container_.size());

			bool selected = (pItem->stateMask & LVIS_SELECTED) && (pItem->state & LVIS_SELECTED);
			list_pair_t lp = list_pair_t(selected, key);

			pair_container_.push_back(lp);

			SetItemCountEx(numeric_cast<int>(pair_container_.size()), LVSICF_NOSCROLL);
			SetItemState(list_item_index, 0, LVIS_SELECTED);

			HAL_DEV_SORT_MSG(hal::wform(L"New index %1%, key %3%, selected %2%") % list_item_index % false % key);

			return list_item_index;
		}
	}

	bool data_type_comparison(list_pair_t r, list_pair_t l, size_t index, bool ascending)
	{
		TBase* pT = static_cast<TBase*>(this);

		return pT->sort_list_comparison((l).second, (r).second, index, ascending);
	}

	void selection_from_listview()
	{
		for (auto val : *this)
		{
			const list_pair_t& i_pos = pair_container_.get<0>()[val.index()];

			if (val.state(LVIS_SELECTED) & LVIS_SELECTED)
				i_pos.first = true;
			else
				i_pos.first = false;

			
			HAL_DEV_SORT_MSG(hal::wform(L" Name %1%, index %2%, selected %3%") % key_from_index(val.index()) % val.index() % i_pos.first);
		}
		
		HAL_DEV_SORT_MSG(hal::wform(L" -----"));
	}

	void sort(size_t index, bool ascending)
	{	
		std::vector<implicit_reference_wrapper<const list_pair_t> > sv;

		std::copy(pair_container_.begin(), pair_container_.end(), std::back_inserter(sv));

		std::stable_sort(sv.begin(), sv.end(), 
			boost::bind(&this_class_t::data_type_comparison, this, _1, _2, index, ascending));

		pair_container_.rearrange(sv.begin());
	}

	DataType key_from_index(size_t index)
	{
		list_pair_t pi = pair_container_[index];

		return pi.second;
	}

	boost::optional<size_t> index_from_key(const DataType& key)
	{
		auto i = pair_container_.get<by_key>().find(key);	
		
		if (i != pair_container_.get<by_key>().end())
		{
			pair_container::iterator i_pos = pair_container_.project<0>(i);	
			return std::distance(pair_container_.begin(), i_pos);
		}
		else
			return boost::optional<size_t>();
	}

	void erase_from_list(const list_value_type& val)
	{
		erase_from_list(val.index());
	}

	void erase_from_list(size_t index)
	{
		pair_container_.erase(pair_container_.begin() + index);
		DeleteItem(numeric_cast<int>(index));
	}
	
	void erase_from_list(const DataType& str)
	{
		auto i = pair_container_.get<by_key>().find(str);
		pair_container::iterator i_pos = pair_container_.project<0>(i);

		if (i != pair_container_.get<by_key>().end())
		{
			DeleteItem(numeric_cast<int>(std::distance(pair_container_.begin(), i_pos)));
			pair_container_.erase(i_pos);
		}
	}

	void erase_based_on_set(const std::set<DataType>& s, bool within=true)
	{
		for (pair_container::const_iterator i=pair_container_.begin(), e=pair_container_.end(); i!=e; /**/)
		{			
			HAL_DEV_SORT_MSG(hal::wform(L" Checking %1%,") % (*i).second);

			if ((s.find((*i).second) != s.end()) ^ within)
			{				
				HAL_DEV_SORT_MSG(hal::wform(L" Erasing,"));
				
				DeleteItem(numeric_cast<int>(std::distance(pair_container_.begin(), i)));
				i  = pair_container_.erase(i);
			}
			else
			{
				++i;
			}
		}
	}

	void erase_all_from_list()
	{
		pair_container_.clear();
		DeleteAllItems();
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
inline const std::wstring hal::to_wstr_shim<const LVITEM>(const LVITEM& v)
{
	return std::wstring(v.pszText);
}

template<>
inline const std::wstring hal::to_wstr_shim<LVITEM>(LVITEM& v)
{
	return std::wstring(v.pszText);
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
