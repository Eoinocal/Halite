
#pragma once
 
#include "stdAfx.hpp"
#include "HaliteTabPage.hpp"

class CHalTabCtrl : public CWindowImpl<CHalTabCtrl, CTabCtrl>
{
public:	
	CHalTabCtrl() :
		currentPage_(0)
	{}

    BEGIN_MSG_MAP(CTabCtrlWithDisableImpl)
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
			
			::SetWindowPos(currentPage_, HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE);
		}
	}
	
	LRESULT OnSelChange(LPNMHDR lpHdr)
	{
		SetCurrentPage(GetCurSel());
		
		return 0;
	}
	
	void AddPage(HWND hWnd, wstring wndText)
	{	
		TCITEM tie = { TCIF_TEXT, 0, 0, const_cast<wchar_t*>(wndText.c_str()), 0, -1, 0 };
		InsertItem(0, &tie);
		
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
