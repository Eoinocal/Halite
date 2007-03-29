
#pragma once

#include <boost/array.hpp>
#include <boost/signals.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_free.hpp>

#include "stdAfx.hpp"
#include "HaliteListManager.hpp"

template <class TBase>
class CHaliteListViewCtrl : public CWindowImpl<TBase, CListViewCtrl>
{

public:
	CHaliteListViewCtrl<TBase>() :
		manager_(*this)
	{
		BOOL menu_created = menu_.LoadMenu(TBase::LISTVIEW_ID_MENU);
		assert(menu_created);	

		wstring column_names = hal::app().res_wstr(TBase::LISTVIEW_ID_COLUMNNAMES);		
		boost::split(names_, column_names, boost::is_any_of(L";"));
				
		listColumnWidth_.assign(names_.size(), 0);	
		listColumnOrder_.assign(names_.size(), 0);
	}

	BEGIN_MSG_MAP_EX(CHaliteListViewCtrl<TBase>)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
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
		assert (listColumnWidth_.size() == names_.size());
		assert (listColumnOrder_.size() == names_.size());
		
		CHeaderCtrl hdr = GetHeader();
		hdr.ModifyStyle(0, HDS_DRAGDROP|HDS_FULLDRAG);
			
		foreach (wstring name, names_)
		{
			AddColumn(name.c_str(), hdr.GetItemCount());
		}		

		for (unsigned i=0; i<names_.size(); ++i)
			SetColumnWidth(i, listColumnWidth_[i]);
		
		SetColumnOrderArray(names_.size(), &listColumnOrder_[0]);	
	}
	
	template<std::size_t Size>
	void SetDefaults(array<int, Size> a)
	{
		assert (Size == names_.size());
		assert (listColumnWidth_.size() == names_.size());
		assert (listColumnOrder_.size() == names_.size());
		
		for (size_t i=0; i<names_.size(); ++i)
		{
			listColumnWidth_[i] = a[i];
			listColumnOrder_[i] = i;
		}		
	}	
	
	void GetListViewDetails()
	{
		assert (listColumnWidth_.size() == names_.size());
		assert (listColumnOrder_.size() == names_.size());
		
		GetColumnOrderArray(names_.size(), &listColumnOrder_[0]);
		
		for (size_t i=0; i<names_.size(); ++i)
			listColumnWidth_[i] = GetColumnWidth(i);	
	}

	LRESULT OnClick(int, LPNMHDR pnmh, BOOL&)
	{
		manager().sync_list(true);

		return 0;
	}

	LRESULT OnRClick(int i, LPNMHDR pnmh, BOOL&)
	{
		LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)pnmh;
		manager().sync_list(true);

		assert (menu_.IsMenu());
		CMenuHandle sMenu = menu_.GetSubMenu(0);
		assert (sMenu.IsMenu());

		POINT ptPoint;
		GetCursorPos(&ptPoint);
		sMenu.TrackPopupMenu(0, ptPoint.x, ptPoint.y, m_hWnd);

		return 0;
	}

	LRESULT OnColClick(int i, LPNMHDR pnmh, BOOL&)
	{
		LPNMLISTVIEW pnlv = (LPNMLISTVIEW)pnmh;
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
	
	std::vector<int>& listColumnWidth() { return listColumnWidth_; }
	std::vector<int>& listColumnOrder() { return listColumnOrder_; }
	
	const std::vector<int>& listColumnWidth() const { return listColumnWidth_; }
	const std::vector<int>& listColumnOrder() const { return listColumnOrder_; }
	
private:
	selection_manager<CHaliteListViewCtrl> manager_;
	
	WTL::CMenu menu_;
	std::vector<wstring> names_;
	std::vector<int> listColumnWidth_;
	std::vector<int> listColumnOrder_;
};
