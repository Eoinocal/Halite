
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
//        MESSAGE_HANDLER(WM_CREATE, OnCreate)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGE, OnSelChange)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

	void Attach(HWND hWndNew)
	{
		ATLASSERT(::IsWindow(hWndNew));
        CWindowImpl<CHalTabCtrl, CTabCtrl>::SubclassWindow(hWndNew);
	}

	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = TRUE;
		return bHandled;
	}

	void SetCurrentPage(unsigned index)
	{
		if (currentPage_)
			::ShowWindow(currentPage_, false);

		if (!pages_.empty())
		{
			currentPage_ = pages_[index];
			::ShowWindow(currentPage_, true);

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
