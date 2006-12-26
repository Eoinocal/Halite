
#pragma once

#include "stdAfx.hpp"

class HaliteListViewCtrl : 
	public CWindowImpl<HaliteListViewCtrl, CListViewCtrl>,
	private noncopyable
{
public:	
	BEGIN_MSG_MAP(HaliteListViewCtrl)
		MSG_WM_SHOWWINDOW(onShowWIndow)
	END_MSG_MAP()
	
	void onShowWIndow(UINT, INT);
	void saveStatus();
	void updateListView();
};
