
//         Copyright Eóin O'Callaghan 2006 - 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once 

#include "stdAfx.hpp"
#include "atldlgx.h"

#include <boost/array.hpp>

namespace WTLx
{

template <class T>
class CEditListViewImpl
{
public:

	BEGIN_MSG_MAP(CEditListViewImpl)
	END_MSG_MAP()

private:
};

typedef CInPlaceEditor<MAX_PATH> CItemEditor;

typedef ATL::CWinTraits<WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|LVS_REPORT|LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE> CEditListViewCtrlTraits;

template <class T, class TBase = CListViewCtrl, class TWinTraits = CEditListViewCtrlTraits>
class ATL_NO_VTABLE CEditListViewCtrlImpl: public ATL::CWindowImpl<T, TBase, TWinTraits>, public CEditListViewImpl<T>
{
	typedef CEditListViewCtrlImpl< T, TBase, TWinTraits> this_class_t;
	typedef ATL::CWindowImpl<T, TBase, TWinTraits> base_class_t;

public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

	BEGIN_MSG_MAP_EX(this_class_t)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDoubleClick)

		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	void SubclassWindow(HWND hWndNew)
	{
		HAL_DEV_MSG(L"SubclassWindow");

		ATLASSERT(::IsWindow(hWndNew));
		base_class_t::SubclassWindow(hWndNew);
		hjk = hWndNew;
	}

	LRESULT OnDoubleClick(int i, LPNMHDR pnmh, BOOL&)
	{
		LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pnmh;
		LVHITTESTINFO hit;

		hit.pt = lpnmitem->ptAction;
		SubItemHitTest(&hit);

		HAL_DEV_MSG(hal::wform(L"OnDoubleClick %1% %2%") % hit.iItem % hit.iSubItem);

		if (hit.iItem != -1 && hit.iSubItem != -1)
		{
			const int buffer_size = MAX_PATH;
			boost::array<wchar_t, buffer_size> buffer;

			GetItemText(hit.iItem, hit.iSubItem, buffer.elems, buffer_size);
			
			CRect rect;
			CPoint pt;
			GetSubItemRect(hit.iItem, hit.iSubItem, LVIR_BOUNDS, rect);
			GetItemPosition(hit.iItem, &pt);

			//rect.InflateRect(-1,-1);
            //rect.OffsetRect(pt);

			HAL_DEV_MSG(hal::wform(L"rect %1%,%2%-%3%,%4%") % rect.left % rect.top % rect.right % rect.bottom);
			
			CRect rc(0,0, 50, 20);
			CRect rc1;
			CRect rc2;
			CRect rc3;
			CRect rc4;

			GetWindowRect(rc1); 
			::GetWindowRect(GetParent(), rc2); 

			HAL_DEV_MSG(hal::wform(L"rc %1%,%2%-%3%,%4%") % rc1.left % rc1.top % rc1.right % rc1.bottom);
			HAL_DEV_MSG(hal::wform(L"rc2 %1%,%2%-%3%,%4%") % rc2.left % rc2.top % rc2.right % rc2.bottom);

			//OffsetRect(rcOwner, -rect.left, -rect.top); 
			//OffsetRect(rcOwner, -rc.left, -rc.top); 
			//OffsetRect(rect, rcOwner.right, rcOwner.bottom); 

			//rect.OffsetRect(rcOwner.left - rc.left, rcOwner.top - rc.top);

			//rect.OffsetRect(rect1.left, rect1.top);

			bool bRes = CItemEditor::Edit(rc, buffer.elems, GetForegroundWindow());	

			HAL_DEV_MSG(hal::wform(L"CItemEditor::Edit %1%") % bRes);	
		}
		return true;
	}

	void OnLButtonDblClk(UINT nFlags, CPoint point)
	{
		HAL_DEV_MSG(L"OnLButtonDblClk");
	}

	HWND hjk;
};

class CEditListViewCtrl : public CEditListViewCtrlImpl<CEditListViewCtrl>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTLx_EditListViewCtrl"), GetWndClassName())
};


};
