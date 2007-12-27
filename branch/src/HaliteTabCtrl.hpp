
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdAfx.hpp"
#include "HaliteTabPage.hpp"

class CHalTabCtrl : public CWindowImpl<CHalTabCtrl, CTabCtrl>
{
public:
	CHalTabCtrl() :
		currentPage_(0)
	{}

    BEGIN_MSG_MAP_EX(CHalTabCtrl)
		MSG_WM_SIZE(OnSize)
//		MSG_WM_ERASEBKGND(OnEraseBkgnd)
//        MESSAGE_HANDLER(WM_CREATE, OnCreate)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGE, OnSelChange)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

	void Attach(HWND hWndNew)
	{
		ATLASSERT(::IsWindow(hWndNew));
        CWindowImpl<CHalTabCtrl, CTabCtrl>::SubclassWindow(hWndNew);
	}

	void SetCurrentPage(unsigned index)
	{
		if (currentPage_)
			::ShowWindow(currentPage_, SW_HIDE);

		if (!pages_.empty())
		{
			currentPage_ = pages_[index];
			::ShowWindow(currentPage_, SW_SHOW);

			RECT rect;
			GetClientRect(&rect);
			AdjustRect(false, &rect);

			::SetWindowPos(currentPage_, HWND_TOP, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, 0);
		}
	}

	LRESULT OnSelChange(LPNMHDR lpHdr)
	{
		SetCurrentPage(GetCurSel());

		return 0;
	}

	void OnSize(UINT, CSize)
	{
//		MessageBox(L"Here in size", L"Msg", 0);
		RECT rect;
		GetClientRect(&rect);
		AdjustRect(false, &rect);

		::MoveWindow(currentPage_,  rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, true);
	}

	void AddPage(HWND hWnd, wstring wndText)
	{
		TCITEM tie = { TCIF_TEXT, 0, 0, const_cast<wchar_t*>(wndText.c_str()), 0, -1, 0 };
		InsertItem(pages_.size(), &tie);

		pages_.push_back(hWnd);

/*		::ShowWindow(hWnd, true);

		RECT rect;
		GetClientRect(&rect);
		AdjustRect(false, &rect);

		::SetWindowPos(hWnd, HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE);
*/
	}

protected:
	std::vector<HWND> pages_;
	HWND currentPage_;
};
