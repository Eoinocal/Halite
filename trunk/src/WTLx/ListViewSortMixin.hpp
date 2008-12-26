
//         Copyright Eóin O'Callaghan 2008 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef LISTVIEW_SORT_MIXIN_HPP_INCLUDED
#define LISTVIEW_SORT_MIXIN_HPP_INCLUDED

#include <boost/iterator/filter_iterator.hpp>
#include <winstl/controls/listview_sequence.hpp>

#define SORTLV_USESHELLBITMAPS	0x00000001

namespace WTLx
{

template<typename T>
class ListViewSortMixin : public WTL::CSortListViewImpl<T>
{
protected:
	
	// Column sort types. Can be set on a per-column basis with the SetColumnSortType method.
/*	enum
	{
		LVCOLSORT_NONE,
		LVCOLSORT_TEXT,   // default
		LVCOLSORT_TEXTNOCASE,
		LVCOLSORT_LONG,
		LVCOLSORT_DOUBLE,
		LVCOLSORT_DECIMAL,
		LVCOLSORT_DATETIME,
		LVCOLSORT_DATE,
		LVCOLSORT_TIME,
		LVCOLSORT_CUSTOM,
		LVCOLSORT_LAST = LVCOLSORT_CUSTOM
	};

	enum
	{
		m_cchCmpTextMax = 32, // overrideable
		m_cxSortImage = 16,
		m_cySortImage = 15,
		m_cxSortArrow = 11,
		m_cySortArrow = 6,
		m_iSortUp = 0,        // index of sort bitmaps
		m_iSortDown = 1,
		m_nShellSortUpID = 133
	};	
*/
	BEGIN_MSG_MAP(ListViewSortMixin)
		MESSAGE_HANDLER(LVM_INSERTCOLUMN, WTL::CSortListViewImpl<T>::OnInsertColumn)
		MESSAGE_HANDLER(LVM_DELETECOLUMN, WTL::CSortListViewImpl<T>::OnDeleteColumn)
		NOTIFY_CODE_HANDLER(HDN_ITEMCLICKA, OnHeaderItemClick)
		NOTIFY_CODE_HANDLER(HDN_ITEMCLICKW, OnHeaderItemClick)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, WTL::CSortListViewImpl<T>::OnSettingChange)
	END_MSG_MAP()

/*	LRESULT OnInsertColumn(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& )	
	{
		T* pT = static_cast<T*>(this);
		LRESULT lRet = pT->DefWindowProc(uMsg, wParam, lParam);
		if(lRet == -1)
			return -1;

		WORD wType = 0;
		m_arrColSortType.Add(wType);
		int nCount = m_arrColSortType.GetSize();
		ATLASSERT(nCount == GetColumnCount());

		for(int i = nCount - 1; i > lRet; i--)
			m_arrColSortType[i] = m_arrColSortType[i - 1];
		m_arrColSortType[(int)lRet] = LVCOLSORT_TEXT;

		if(lRet <= m_iSortColumn)
			m_iSortColumn++;

		return lRet;
	}

	LRESULT OnDeleteColumn(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)	
	{
		T* pT = static_cast<T*>(this);
		LRESULT lRet = pT->DefWindowProc(uMsg, wParam, lParam);
		if(lRet == 0)
			return 0;

		int iCol = (int)wParam; 
		if(m_iSortColumn == iCol)
			m_iSortColumn = -1;
		else if(m_iSortColumn > iCol)
			m_iSortColumn--;
		m_arrColSortType.RemoveAt(iCol);

		return lRet;
	}

	LRESULT OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
#ifndef _WIN32_WCE
		if(wParam == SPI_SETNONCLIENTMETRICS)
			GetSystemSettings();
#else  // CE specific
		wParam; // avoid level 4 warning
		GetSystemSettings();
#endif // _WIN32_WCE
		bHandled = FALSE;
		return 0;
	}

	void GetSystemSettings()
	{
		if(!m_bCommCtrl6 && !m_bmSort[m_iSortUp].IsNull())
		{
			T* pT = static_cast<T*>(this);
			pT->CreateSortBitmaps();
			if(m_iSortColumn != -1)
				SetSortColumn(m_iSortColumn);
		}
	}
*/
	DWORD SetListViewSortMixinExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0)
	{
		DWORD dwPrevStyle = m_dwSortLVExtendedStyle;
		if(dwMask == 0)
			m_dwSortLVExtendedStyle = dwExtendedStyle;
		else
			m_dwSortLVExtendedStyle = (m_dwSortLVExtendedStyle & ~dwMask) | (dwExtendedStyle & dwMask);
		return dwPrevStyle;
	}

	DWORD GetListViewSortMixibExtendedStyle() const
	{
		return m_dwSortLVExtendedStyle;
	}
	

	void SetColumnSortType(int iCol, WORD wType)
	{
		HAL_DEV_MSG(hal::wform(L"SetColumnSortType(int iCol = %1%, WORD wType = %2%)") % iCol % wType);

		ATLASSERT(iCol >= 0 && iCol < m_arrColSortType.GetSize());
		ATLASSERT(wType >= WTL::LVCOLSORT_NONE);
		m_arrColSortType[iCol] = wType;		
	}

/*	int GetColumnCount() const
	{
		const T* pT = static_cast<const T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		WTL::CHeaderCtrl header = pT->GetHeader();
		return header.m_hWnd != NULL ? header.GetItemCount() : 0;
	}
	*/
	LRESULT OnHeaderItemClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{		
		HAL_DEV_MSG(hal::wform(L"OnHeaderItemClick(int idCtrl = %1%, LPNMHDR pnmh, BOOL& bHandled)") % idCtrl);

		LPNMHEADER p = (LPNMHEADER)pnmh;
		if(p->iButton == 0)
		{
			int iOld = m_iSortColumn;
			bool bDescending = (m_iSortColumn == p->iItem) ? !m_bSortDescending : false;

			if (DoSortItems(p->iItem, bDescending))
				NotifyParentSortChanged(p->iItem, iOld);
		}
		bHandled = FALSE;
		return 0;
	}

//  Operations
	bool DoSortItems(int iCol, bool bDescending = false)
	{
		HAL_DEV_MSG(hal::wform(L"DoSortItems(int iCol = %1%, bool bDescending = %2%)") % iCol % bDescending);

		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		ATLASSERT(iCol >= 0 && iCol < m_arrColSortType.GetSize());

		WORD wType = m_arrColSortType[iCol];
		if(wType == WTL::LVCOLSORT_NONE)
			return false;
		else if (wType <= WTL::LVCOLSORT_LAST)
		{
			HAL_DEV_MSG(hal::wform(L"wType = %1%, passing DoSort() to base class") % wType);
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
/*
	void SetSortColumn(int iCol)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		WTL::CHeaderCtrl header = pT->GetHeader();
		ATLASSERT(header.m_hWnd != NULL);
		ATLASSERT(iCol >= -1 && iCol < m_arrColSortType.GetSize());

		int iOldSortCol = m_iSortColumn;
		m_iSortColumn = iCol;

		if(m_bCommCtrl6)
		{
#ifndef HDF_SORTUP
			const int HDF_SORTUP = 0x0400;	
#endif // HDF_SORTUP
#ifndef HDF_SORTDOWN
			const int HDF_SORTDOWN = 0x0200;	
#endif // HDF_SORTDOWN
			const int nMask = HDF_SORTUP | HDF_SORTDOWN;
			HDITEM hditem = { HDI_FORMAT };
			if(iOldSortCol != iCol && iOldSortCol >= 0 && header.GetItem(iOldSortCol, &hditem))
			{
				hditem.fmt &= ~nMask;
				header.SetItem(iOldSortCol, &hditem);
			}
			if(iCol >= 0 && header.GetItem(iCol, &hditem))
			{
				hditem.fmt &= ~nMask;
				hditem.fmt |= m_bSortDescending ? HDF_SORTDOWN : HDF_SORTUP;
				header.SetItem(iCol, &hditem);
			}
			return;
		}

		if(m_bmSort[m_iSortUp].IsNull())
			pT->CreateSortBitmaps();

		// restore previous sort column's bitmap, if any, and format
		HDITEM hditem = { HDI_BITMAP | HDI_FORMAT };
		if(iOldSortCol != iCol && iOldSortCol >= 0)
		{
			hditem.hbm = m_hbmOldSortCol;
			hditem.fmt = m_fmtOldSortCol;
			header.SetItem(iOldSortCol, &hditem);
		}

		// save new sort column's bitmap and format, and add our sort bitmap
		if(iCol >= 0 && header.GetItem(iCol, &hditem))
		{
			if(iOldSortCol != iCol)
			{
				m_fmtOldSortCol = hditem.fmt;
				m_hbmOldSortCol = hditem.hbm;
			}
			hditem.fmt &= ~HDF_IMAGE;
			hditem.fmt |= HDF_BITMAP | HDF_BITMAP_ON_RIGHT;
			int i = m_bSortDescending ? m_iSortDown : m_iSortUp;
			hditem.hbm = m_bmSort[i];
			header.SetItem(iCol, &hditem);
		}
	}

	void CreateSortBitmaps()
	{
		if((m_dwSortLVExtendedStyle & SORTLV_USESHELLBITMAPS) != 0)
		{
			bool bFree = false;
			LPCTSTR pszModule = _T("shell32.dll"); 
			HINSTANCE hShell = ::GetModuleHandle(pszModule);

			if (hShell == NULL)		
			{
				hShell = ::LoadLibrary(pszModule);
				bFree = true;
			}
 
			if (hShell != NULL)
			{
				bool bSuccess = true;
				for(int i = m_iSortUp; i <= m_iSortDown; i++)
				{
					if(!m_bmSort[i].IsNull())
						m_bmSort[i].DeleteObject();
					m_bmSort[i] = (HBITMAP)::LoadImage(hShell, MAKEINTRESOURCE(m_nShellSortUpID + i), 
#ifndef _WIN32_WCE
						IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
#else // CE specific
						IMAGE_BITMAP, 0, 0, 0);
#endif // _WIN32_WCE
					if(m_bmSort[i].IsNull())
					{
						bSuccess = false;
						break;
					}
				}
				if(bFree)
					::FreeLibrary(hShell);
				if(bSuccess)
					return;
			}
		}

		T* pT = static_cast<T*>(this);
		for(int i = m_iSortUp; i <= m_iSortDown; i++)
		{
			if(!m_bmSort[i].IsNull())
				m_bmSort[i].DeleteObject();

			WTL::CDC dcMem;
			WTL::CClientDC dc(::GetDesktopWindow());
			dcMem.CreateCompatibleDC(dc.m_hDC);
			m_bmSort[i].CreateCompatibleBitmap(dc.m_hDC, m_cxSortImage, m_cySortImage);
			HBITMAP hbmOld = dcMem.SelectBitmap(m_bmSort[i]);
			RECT rc = {0,0,m_cxSortImage, m_cySortImage};
			pT->DrawSortBitmap(dcMem.m_hDC, i, &rc);
			dcMem.SelectBitmap(hbmOld);
			dcMem.DeleteDC();
		}
	}

	void DrawSortBitmap(WTL::CDCHandle dc, int iBitmap, LPRECT prc)
	{
		dc.FillRect(prc, ::GetSysColorBrush(COLOR_BTNFACE));	
		HBRUSH hbrOld = dc.SelectBrush(::GetSysColorBrush(COLOR_BTNSHADOW));
		WTL::CPen pen;
		pen.CreatePen(PS_SOLID, 0, ::GetSysColor(COLOR_BTNSHADOW));
		HPEN hpenOld = dc.SelectPen(pen);
		POINT ptOrg = { (m_cxSortImage - m_cxSortArrow) / 2, (m_cySortImage - m_cySortArrow) / 2 };
		if(iBitmap == m_iSortUp)
		{
			POINT pts[3] = 
			{
				{ ptOrg.x + m_cxSortArrow / 2, ptOrg.y },
				{ ptOrg.x, ptOrg.y + m_cySortArrow - 1 }, 
				{ ptOrg.x + m_cxSortArrow - 1, ptOrg.y + m_cySortArrow - 1 }
			};
			dc.Polygon(pts, 3);
		}
		else
		{
			POINT pts[3] = 
			{
				{ ptOrg.x, ptOrg.y },
				{ ptOrg.x + m_cxSortArrow / 2, ptOrg.y + m_cySortArrow - 1 },
				{ ptOrg.x + m_cxSortArrow - 1, ptOrg.y }
			};
			dc.Polygon(pts, 3);
		}
		dc.SelectBrush(hbrOld);
		dc.SelectPen(hpenOld);
	}

	int GetSortColumn() const
	{
		return m_iSortColumn;
	}

	void SetColumnSortType(int iCol, WORD wType)
	{
		ATLASSERT(iCol >= 0 && iCol < m_arrColSortType.GetSize());
		ATLASSERT(wType >= LVCOLSORT_NONE && wType <= LVCOLSORT_LAST);
		m_arrColSortType[iCol] = wType;
	}

	WORD GetColumnSortType(int iCol) const
	{
		ATLASSERT((iCol >= 0) && iCol < m_arrColSortType.GetSize());
		return m_arrColSortType[iCol];
	}

	bool IsSortDescending() const
	{
		return m_bSortDescending;
	}

private:
	bool m_bSortDescending;
	bool m_bCommCtrl6;
	int m_iSortColumn;
	WTL::CBitmap m_bmSort[2];
	int m_fmtOldSortCol;
	HBITMAP m_hbmOldSortCol;
	DWORD m_dwSortLVExtendedStyle;
	ATL::CSimpleArray<WORD> m_arrColSortType;
	bool m_bUseWaitCursor;
	*/
};

}

#endif // LISTVIEW_SORT_MIXIN_HPP_INCLUDED
