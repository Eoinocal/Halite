
//         Copyright Eóin O'Callaghan 2008 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef LISTVIEW_SORT_MIXIN_HPP_INCLUDED
#define LISTVIEW_SORT_MIXIN_HPP_INCLUDED

#include <boost/iterator/filter_iterator.hpp>
#pragma warning (push)
#pragma warning (disable : 4244)
#	include <winstl/controls/listview_sequence.hpp>
#pragma warning (pop)

#define SLVN_SECONDSORTCHANGED		SLVN_SORTCHANGED+1

namespace WTLx
{

template<typename T>
class ListViewSortMixin : public WTL::CSortListViewImpl<T>
{
protected:
	ListViewSortMixin() :
		iSecondarySort(-1),
		bSecondaryDescending(false)
	{}

	BEGIN_MSG_MAP(ListViewSortMixin)
		MESSAGE_HANDLER(LVM_INSERTCOLUMN, WTL::CSortListViewImpl<T>::OnInsertColumn)
		MESSAGE_HANDLER(LVM_DELETECOLUMN, WTL::CSortListViewImpl<T>::OnDeleteColumn)
		NOTIFY_CODE_HANDLER(HDN_ITEMCLICKA, OnHeaderItemClick)
		NOTIFY_CODE_HANDLER(HDN_ITEMCLICKW, OnHeaderItemClick)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, WTL::CSortListViewImpl<T>::OnSettingChange)
	END_MSG_MAP()

	DWORD SetListViewSortMixinExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0)
	{
		DWORD dwPrevStyle = m_dwSortLVExtendedStyle;
		if(dwMask == 0)
			m_dwSortLVExtendedStyle = dwExtendedStyle;
		else
			m_dwSortLVExtendedStyle = (m_dwSortLVExtendedStyle & ~dwMask) | (dwExtendedStyle & dwMask);
		return dwPrevStyle;
	}

	DWORD GetListViewSortMixinExtendedStyle() const
	{
		return m_dwSortLVExtendedStyle;
	}
	
	void SetColumnSortType(int iCol, WORD wType)
	{
		HAL_DEV_SORT_MSG(hal::wform(L"SetColumnSortType(int iCol = %1%, WORD wType = %2%)") % iCol % wType);

		ATLASSERT(iCol >= 0 && iCol < m_arrColSortType.GetSize());
		ATLASSERT(wType >= WTL::LVCOLSORT_NONE);
		m_arrColSortType[iCol] = wType;		
	}

	LRESULT OnHeaderItemClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{		
		HAL_DEV_SORT_MSG(hal::wform(L"OnHeaderItemClick(int idCtrl = %1%, LPNMHDR pnmh, BOOL& bHandled)") % idCtrl);

		LPNMHEADER p = (LPNMHEADER)pnmh;
		if(p->iButton == 0)
		{
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				int iOld = iSecondarySort;

				bSecondaryDescending = (iSecondarySort == p->iItem) ? !bSecondaryDescending : false;
				iSecondarySort = p->iItem;

				T* pT = static_cast<T*>(this);
				bool bRet = pT->DoSortItemsExternal(iSecondarySort, bSecondaryDescending);
			}
			else
			{
				int iOld = m_iSortColumn;
				bool bDescending = (m_iSortColumn == p->iItem) ? !m_bSortDescending : false;

				if (DoSortItems(p->iItem, bDescending))
					NotifyParentSortChanged(p->iItem, iOld);
			}
		}

		bHandled = FALSE;
		return 0;
	}

	void NotifyParentSecondarySortChanged(int iNewSortCol, int iOldSortCol)
	{
		T* pT = static_cast<T*>(this);
		int nID = pT->GetDlgCtrlID();
		WTL::NMSORTLISTVIEW nm = { { pT->m_hWnd, nID, SLVN_SECONDSORTCHANGED }, iNewSortCol, iOldSortCol };
		::SendMessage(pT->GetParent(), WM_NOTIFY, (WPARAM)nID, (LPARAM)&nm);
	}

//  Operations
	bool DoSortItems(int iCol, bool bDescending = false)
	{
		HAL_DEV_SORT_MSG(hal::wform(L"DoSortItems(int iCol = %1%, bool bDescending = %2%)") % iCol % bDescending);

		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		ATLASSERT(iCol >= 0 && iCol < m_arrColSortType.GetSize());

		WORD wType = m_arrColSortType[iCol];
		if(wType == WTL::LVCOLSORT_NONE)
			return false;
		else if (wType <= WTL::LVCOLSORT_LAST)
		{
			HAL_DEV_SORT_MSG(hal::wform(L"wType = %1%, passing DoSort() to base class") % wType);
			return WTL::CSortListViewImpl<T>::DoSortItems(iCol, bDescending);
		}

		int nCount = pT->GetItemCount();
		if(nCount < 2)
		{
			m_bSortDescending = bDescending;
			SetSortColumn(iCol);
			return true;
		}

		WTL::CWaitCursor waitCursor(false);
		if(m_bUseWaitCursor)
			waitCursor.Set();

		bool bRet = pT->DoSortItemsExternal(iCol, bDescending);

		if(bRet)
		{
			m_bSortDescending = bDescending;
			SetSortColumn(iCol);
		}

		if(m_bUseWaitCursor)
			waitCursor.Restore();

		return bRet;
	}

	const int GetSecondarySortColumn() const { return iSecondarySort; }
	const bool IsSecondarySortDescending() const { return bSecondaryDescending; }

	int iSecondarySort;
	bool bSecondaryDescending;
};

}

#endif // LISTVIEW_SORT_MIXIN_HPP_INCLUDED
