
#pragma once

#include "stdAfx.hpp"

class HaliteListViewCtrl : public CWindowImpl<HaliteListViewCtrl, CListViewCtrl>
{
public:	
	BEGIN_MSG_MAP(HaliteListViewCtrl)
		MSG_WM_LBUTTONUP(OnLButtonDown)
	END_MSG_MAP()
	 
	void OnLButtonDown(UINT /*wParam*/, CPoint /*point*/) 
	{
		//::PostMessage(UIUpdateMsgWnd,WM_UPDATEUIINFO,0,0);
		SetMsgHandled(false);
	}

	void updateListView();
};