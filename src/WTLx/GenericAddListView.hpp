
//         Copyright E�in O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define HAL_GENERIC_ALV_BEGIN	 	20000
#define HAL_GENERIC_ALV_NEW 		HAL_GENERIC_ALV_BEGIN + 1
#define HAL_GENERIC_ALV_EDIT 		HAL_GENERIC_ALV_BEGIN + 2
#define HAL_GENERIC_ALV_DELETE		HAL_GENERIC_ALV_BEGIN + 3

#ifndef RC_INVOKED

#include <boost/signals.hpp>
#include <boost/function.hpp>

#include "stdAfx.hpp"

namespace WTLx
{

template<class ListClass, int listID_NEW=HAL_GENERIC_ALV_NEW, 
	int listID_EDIT=HAL_GENERIC_ALV_EDIT, int listID_DELETE=HAL_GENERIC_ALV_DELETE
	>
class GenericAddListView
{
	friend class ListClass;
	
public:
	BEGIN_MSG_MAP_EX(TrackerListViewCtrl)
		COMMAND_ID_HANDLER(listID_NEW, OnNew)
		COMMAND_ID_HANDLER(listID_EDIT, OnEdit)
		COMMAND_ID_HANDLER(listID_DELETE, OnDelete)

		REFLECTED_NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDoubleClick)
	END_MSG_MAP()

	LRESULT OnDoubleClick(int i, LPNMHDR pnmh, BOOL&)
	{		
		ListClass* pT = static_cast<ListClass*>(this);

		LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pnmh;
		LVHITTESTINFO hit;

		hit.pt = lpnmitem->ptAction;
		pT->SubItemHitTest(&hit);

		if (hit.iItem == -1)			
			pT->newItem();
		else
			pT->editItem(hit.iItem);

		return 0;
	}

	LRESULT OnNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		ListClass* pT = static_cast<ListClass*>(this);
		pT->newItem();

		return 0;
	}

	LRESULT OnEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		ListClass* pT = static_cast<ListClass*>(this);
		pT->editItem(pT->manager().selectedIndex());

		return 0;
	}

	LRESULT OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		ListClass* pT = static_cast<ListClass*>(this);		
		pT->deleteItem(pT->manager().selectedIndex());

		return 0;
	}
};

} // namespace WTLx

#endif // RC_INVOKED
