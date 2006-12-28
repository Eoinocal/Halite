
#pragma once
 
#include "stdAfx.hpp"

class CHalTabCtrl : public CWindowImpl<CHalTabCtrl, CTabCtrl>
{
public:	

    BEGIN_MSG_MAP(CTabCtrlWithDisableImpl)
//        MESSAGE_HANDLER(WM_CREATE, OnCreate)
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
	
};
