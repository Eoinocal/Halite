
#pragma once

#include <boost/array.hpp>
#include <boost/signals.hpp>

#include "stdAfx.hpp"
#include "HaliteListManager.hpp"

template <class TBase>
class CHaliteListViewCtrl : public CWindowImpl<TBase, CListViewCtrl>
{
public:
	CHaliteListViewCtrl<TBase>() :
		manager_(*this)
	{
		BOOL menu_not_created = torrentMenu_.LoadMenu(TBase::ID_MENU);
		assert(menu_not_created);
	}

	BEGIN_MSG_MAP_EX(CHaliteListViewCtrl<TBase>)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK , OnColClick)

		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void Attach(HWND hWndNew)
	{
		ATLASSERT(::IsWindow(hWndNew));
        CWindowImpl<TBase, CListViewCtrl>::SubclassWindow(hWndNew);

		TBase* pT = static_cast<TBase*>(this);
		pT->OnAttach();
	}

	LRESULT OnClick(int, LPNMHDR pnmh, BOOL&)
	{
		manager().sync_list(true);

		return 0;
	}

	LRESULT OnRClick(int i, LPNMHDR pnmh, BOOL&)
	{
		LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)pnmh;
		manager().sync_list(true);

		assert (torrentMenu_.IsMenu());
		CMenuHandle sMenu = torrentMenu_.GetSubMenu(0);
		assert (sMenu.IsMenu());

		POINT ptPoint;
		GetCursorPos(&ptPoint);
		sMenu.TrackPopupMenu(0, ptPoint.x, ptPoint.y, m_hWnd);

		return 0;
	}

	LRESULT OnColClick(int i, LPNMHDR pnmh, BOOL&)
	{
		LPNMLISTVIEW pnlv = (LPNMLISTVIEW)pnmh;
		return 0;
	}

	selection_manager<CHaliteListViewCtrl>& manager() { return manager_; }

private:
	selection_manager<CHaliteListViewCtrl> manager_;
	WTL::CMenu torrentMenu_;
};

