
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdAfx.hpp"
#include "HaliteTabPage.hpp"

class CHalTabCtrl : 
	public ATL::CWindowImpl<CHalTabCtrl, WTL::CTabCtrl>,
	public WTL::CMessageFilter
{
	struct CHalTabPage
	{
		CHalTabPage() :
			hWnd(0),
			msgFilter(NULL)
		{}

		template <class T>
		CHalTabPage(CHalTabPageImpl<T>& tabPage) :
			hWnd(tabPage),
			msgFilter(&tabPage)
		{}

		HWND hWnd;
		WTL::CMessageFilter* msgFilter;
	};

public:
	CHalTabCtrl() :
		currentPage_(0)
	{}

	BEGIN_MSG_MAP_EX(CHalTabCtrl)
		MSG_WM_SIZE(OnSize)

		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGE, OnSelChange)

		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if (::IsWindow(m_hWnd))
		{
			assert(pages_[GetCurSel()].msgFilter);
		
			return pages_[GetCurSel()].msgFilter->PreTranslateMessage(pMsg);
		}

		return false;
	}

	void Attach(HWND hWndNew)
	{
		ATLASSERT(::IsWindow(hWndNew));
		CWindowImpl<CHalTabCtrl, WTL::CTabCtrl>::SubclassWindow(hWndNew);
	}

	void SetCurrentPage(unsigned index)
	{
		if (currentPage_)
			::ShowWindow(currentPage_, SW_HIDE);

		if (!pages_.empty())
		{
			currentPage_ = pages_[index].hWnd;
			::ShowWindow(currentPage_, SW_SHOW);

			WTL::CRect rect;
			GetClientRect(&rect);
			AdjustRect(false, &rect);

			::SetWindowPos(currentPage_, HWND_TOP, 
				rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, 0);
		}
	}

	LRESULT OnSelChange(LPNMHDR lpHdr)
	{
		SetCurrentPage(GetCurSel());

		return 0;
	}

	void OnSize(UINT, WTL::CSize)
	{
		WTL::CRect rect;
		GetClientRect(&rect);
		AdjustRect(false, &rect);

		::MoveWindow(currentPage_,  rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, true);
	}

	template <class T>
	void AddPage(CHalTabPageImpl<T>& tabPage, std::wstring wndText)
	{
		TCITEM tie = { TCIF_TEXT, 0, 0, const_cast<wchar_t*>(wndText.c_str()), 0, -1, 0 };
		InsertItem(numeric_cast<int>(pages_.size()), &tie);

		pages_.push_back(CHalTabPage(tabPage));
	}

	HWND GetCurrentPage() const { return currentPage_; }

protected:
	std::vector<CHalTabPage> pages_;
	HWND currentPage_;
};
