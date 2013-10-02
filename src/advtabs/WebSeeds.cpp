
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include "../HaliteWindow.hpp"
#include "../HaliteListView.hpp"
#include "../global/logger.hpp"

#include "WebSeeds.hpp"

LRESULT AdvWebSeedsDialog::onInitDialog(HWND, LPARAM)
{
	dlg_base_class_t::InitializeHalDialogBase();	
	
//	m_list.Attach(GetDlgItem(HAL_TRACKERLIST));	
//	m_list.attachEditedConnection(bind(&AdvTrackerDialog::trackerListEdited, this));

	::EnableWindow(GetDlgItem(HAL_TRACKER_LOGINCHECK), false);
	::EnableWindow(GetDlgItem(HAL_TRACKERLIST), false);
	
	url_.clear();
	urlEdit_.SubclassWindow(GetDlgItem(HAL_SEED_URL_TEXT));
		
	DoDataExchange(false); 

	return 0;
}

void AdvWebSeedsDialog::onClose()
{
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

LRESULT AdvWebSeedsDialog::OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl)
{
	DoDataExchange(true);

//	if (hal::bit::torrent t = hal::bittorrent::Instance().get(focused_torrent()))
//		t.set_tracker_login(username_, password_);
	
	return 0;
}

void AdvWebSeedsDialog::focusChanged(const hal::torrent_details_ptr pT)
{		
	if (pT)
	{		
		::EnableWindow(GetDlgItem(HAL_WEB_SEEDS_LISTVIEW), true);
		::EnableWindow(GetDlgItem(HAL_HTTP_SEED), true);
		::EnableWindow(GetDlgItem(HAL_URL_SEED), true);
		::EnableWindow(GetDlgItem(HAL_SEED_URL_TEXT), true);
	}
	else
	{				
		::EnableWindow(GetDlgItem(HAL_WEB_SEEDS_LISTVIEW), false);
		::EnableWindow(GetDlgItem(HAL_HTTP_SEED), false);
		::EnableWindow(GetDlgItem(HAL_URL_SEED), false);
		::EnableWindow(GetDlgItem(HAL_SEED_URL_TEXT), false);
	}
	
//	m_list.uiUpdate(pT);

	DoDataExchange(false);
}

void AdvWebSeedsDialog::uiUpdate(const hal::torrent_details_manager& tD)
{}

AdvWebSeedsDialog::CWindowMapStruct* AdvWebSeedsDialog::GetWindowMap()
{	
	BEGIN_WINDOW_MAP_INLINE(AdvWebSeedsDialog, 6, 6, 3, 3)
		WMB_HEAD(WMB_COLNOMIN(_exp|8), WMB_COL(_exp), WMB_COLNOMIN(_exp|10), WMB_COLNOMIN(_exp|15), WMB_COL(_exp|15)), \
			WMB_ROW(_exp,	HAL_WEB_SEEDS_LISTVIEW, _r, _r, _r, _r), \
			WMB_ROW(_auto,	HAL_SEED_URL, HAL_SEED_URL_TEXT, HAL_ADD_SEED_AS, HAL_HTTP_SEED, HAL_URL_SEED), \
		WMB_END()
	END_WINDOW_MAP_INLINE()
}

void AdvWebSeedsDialog::onAddHttp(UINT, int, HWND)
{
	DoDataExchange(true);
	if (url_.empty()) return;

	HAL_DEV_MSG(hal::wform(L"Adding HTTP seed: %1%") % url_);
}

void AdvWebSeedsDialog::onAddUrl(UINT, int, HWND)
{
	DoDataExchange(true);
	if (url_.empty()) return;

	HAL_DEV_MSG(hal::wform(L"Adding URL seed: %1%") % url_);
}
