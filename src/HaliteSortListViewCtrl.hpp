
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdAfx.hpp"

#include <functional>

#include <boost/array.hpp>
#include <boost/signals.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/ptr_container/ptr_map.hpp>

#include <winstl/controls/listview_sequence.hpp>

#include "Halite.hpp"
#include "halTorrent.hpp"
#include "halEvent.hpp"
#include "WinAPIWaitableTimer.hpp"

#include "UxthemeWrapper.hpp"

#define LVS_EX_DOUBLEBUFFER     0x00010000

#include "WTLx/SelectionManager.hpp"
#include "HaliteUpdateLock.hpp"

namespace hal
{

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

}

template <class TBase, typename adapterType=void*>
class CHaliteSortListViewCtrl : 
	public WTL::CSortListViewCtrlImpl<CHaliteSortListViewCtrl<TBase, adapterType> >
{
public:
	typedef CHaliteSortListViewCtrl<TBase, adapterType> thisClass;
	typedef CSortListViewCtrlImpl<thisClass> parentClass;
	
	class CHaliteHeaderCtrl : public CWindowImpl<CHaliteHeaderCtrl, WTL::CHeaderCtrl>
	{
	public:
		enum { COL_MENU_NAMES = 123, COL_MAX_NAMES = 256};

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
			CWindowImpl<CHaliteHeaderCtrl, CHeaderCtrl>::SubclassWindow(hWndNew);
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

			MENUITEMINFO minfo = {sizeof(MENUITEMINFO)};
		
			minfo.fMask = MIIM_STATE;
			minfo.fState = visible ? MFS_CHECKED : MFS_UNCHECKED;
		
			menu_.SetMenuItemInfo(wID, false, &minfo);

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
	
	struct ColumnAdapter
	{
		virtual int compare(adapterType& l, adapterType& r) = 0;
		virtual std::wstring print(adapterType& t) = 0;
	};

public:
	typedef WTLx::selection_manager<thisClass, std::wstring> SelectionManager;
	typedef SelectionManager selection_manage_class;
	
	thisClass() :
		manager_(*this),
		header_(*this),
		update_lock_(0),
		autoSort_(false),
		descending_(false),
		sortCol_(-1)
	{		
		if (TBase::LISTVIEW_ID_MENU)
		{
			CMenuHandle menu;
			BOOL menu_created = menu.LoadMenu(TBase::LISTVIEW_ID_MENU);
			assert(menu_created);	
			
			menu_.Attach(menu.GetSubMenu(0));
		}
	}

	BEGIN_MSG_MAP_EX(thisClass)
		COMMAND_ID_HANDLER(ID_LVM_AUTOSORT, OnAutoSort)
		
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)

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
	
	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		HWND hwnd = parentClass::Create(hWndParent, 
			(RECT &)rect.m_lpRect, szWindowName, dwStyle, dwExStyle, (UINT)MenuOrID.m_hMenu, lpCreateParam);
			
		SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_DOUBLEBUFFER);
		SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
		
		return hwnd;
	}
	
	bool SubclassWindow(HWND hwnd)
	{
		if(!parentClass::SubclassWindow(hwnd))
			return false;
			
		SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_DOUBLEBUFFER);
		SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
		
		return true;
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

		header_.Menu().CreatePopupMenu();

		for (int i=header_.GetItemCount(), e=int(listNames_.size()); i<e; ++i)
		{
			MENUITEMINFO minfo = {sizeof(MENUITEMINFO)};
			minfo.fMask = MIIM_STRING|MIIM_ID|MIIM_FTYPE|MIIM_STATE;
			minfo.fType = MFT_STRING;
			minfo.dwTypeData = (LPWSTR)listNames_[i].c_str();
			minfo.wID = CHaliteHeaderCtrl::COL_MENU_NAMES+i;

			AddColumn(listNames_[i].c_str(), i);

			if (listVisible_[i])
			{
				minfo.fState = MFS_CHECKED;			
				SetColumnWidth(i, listWidths_[i]);
			}
			else
			{
				minfo.fState = MFS_UNCHECKED;
				SetColumnWidth(i, 0);
			}

			header_.Menu().InsertMenuItem(header_.Menu().GetMenuItemCount(), false, &minfo);
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
			if (listVisible_[i])
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

	bool OnNameChecked(int i)
	{
		if (!listVisible_[i])
		{		
			GetColumnOrderArray(listNames_.size(), &listOrder_[0]);
			SetColumnWidth(i, listWidths_[i]);

			listOrder_.erase(std::find(listOrder_.begin(), listOrder_.end(), i));
			
			int index = i + std::count(listVisible_.begin()+i, listVisible_.end(), false) - 1;
			listOrder_.insert(listOrder_.begin()+index, i);

			SetColumnOrderArray(listNames_.size(), &listOrder_[0]);
			listVisible_[i] = true;
		}
		else
		{
			listWidths_[i] = GetColumnWidth(i);	
			GetColumnOrderArray(listNames_.size(), &listOrder_[0]);

			SetColumnWidth(i, 0);

			listOrder_.erase(std::find(listOrder_.begin(), listOrder_.end(), i));
			listOrder_.insert(listOrder_.begin(), i);

			SetColumnOrderArray(listNames_.size(), &listOrder_[0]);
			listVisible_[i] = false;
		}
	
		InvalidateRect(NULL, true);
		return listVisible_[i];
	}

	LRESULT OnClick(int, LPNMHDR pnmh, BOOL&)
	{
		return 0;
	}

	LRESULT OnItemChanged(int, LPNMHDR pnmh, BOOL&)
	{		
		hal::try_update_lock<thisClass> lock(*this);
		
		if (lock) manager_.sync_list(true, true);
		
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
		using boost::serialization::make_nvp;
		if (version >= 1)
		{
			ar & make_nvp("width", listWidths_);
			ar & make_nvp("order", listOrder_);
			ar & make_nvp("visible", listVisible_);
			ar & make_nvp("autoSort", autoSort_);
		}
		if (version >= 2)
		{
			ar & make_nvp("descending", descending_);
			ar & make_nvp("sortCol", sortCol_);
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
		hal::mutex_update_lock<thisClass> lock(*this);
		
		TBase* pT = static_cast<TBase*>(this);
		
		adapterType left = pT->CustomItemConversion(pItem1, iSortCol);
		adapterType right = pT->CustomItemConversion(pItem2, iSortCol);
		
		return pT->CustomItemComparision(left, right, iSortCol);
	}
	
	bool autoSort() { return autoSort_; }
	
	void ConditionallyDoAutoSort()
	{
		int iCol = GetSortColumn();
		if (autoSort() && iCol >= 0 && iCol < m_arrColSortType.GetSize())
			DoSortItems(iCol, IsSortDescending());	
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

	static bool is_selected (const winstl::listview_sequence::sequence_value_type& v) 
	{ 
		return (v.state() & LVIS_SELECTED) != 0; 
	}

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
			return pCA->compare(left, right);
		else 
			return 0;
	}
	
	void regColumnAdapter(size_t key, ColumnAdapter* colAdapter)
	{
		assert (colAdapter);
		columnAdapters_.insert(key, colAdapter);
	}
	
	SelectionManager manager_;
	
private:
	void vectorSizePreConditions()
	{
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
	
	mutable int update_lock_;
	mutable hal::mutex_t mutex_;

	friend class hal::mutex_update_lock<thisClass>;	
	friend class hal::try_update_lock<thisClass>;		
	
	boost::ptr_map<size_t, ColumnAdapter> columnAdapters_;
	
	WinAPIWaitableTimer syncTimer_;
};

template<>
inline const std::wstring hal::to_wstr_shim<const winstl::listview_sequence::sequence_value_type>
	(const winstl::listview_sequence::sequence_value_type& v)
{
	return std::wstring(winstl::c_str_ptr(v));
}


namespace boost {
namespace serialization {
template <class TBase, typename adapterType>
struct version< CHaliteSortListViewCtrl<TBase, adapterType> >
{
    typedef mpl::int_<2> type;
    typedef mpl::integral_c_tag tag;
    BOOST_STATIC_CONSTANT(unsigned int, value = version::type::value);                                                             
};
}
}
