
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include "../HaliteWindow.hpp"
#include "../HaliteListView.hpp"
#include "../global/logger.hpp"

#include "WebSeeds.hpp"

void WebSeedListViewCtrl::OnAttach()
{
	WTL::CMenuHandle menu;
	BOOL menu_created = menu.LoadMenu(LISTVIEW_ID_MENU);
	InitialSetup(menu);	

	std::vector<wstring> names;	
	wstring column_names = hal::app().res_wstr(HAL_TRACKER_LISTVIEW_COLUMNS);
	boost::split(names, column_names, boost::is_any_of(L";"));
	
	array<int, 2> widths = {		512,				64 };
	array<int, 2> order = {		0,				1 };
	array<bool, 2> visible = {	true,			true };
	array<int, 5> formats = {		LVCFMT_LEFT,		LVCFMT_RIGHT };	

	for (int i=0, e=2; i < e; ++i)
	{
		AddColumn(names[i].c_str(), i, visible[i], widths[i], formats[i]);
	}	

	load_from_ini();	
	
	SetColumnSortType(1, WTL::LVCOLSORT_LONG);
}

void WebSeedListViewCtrl::OnDestroy()
{
	saveSettings();
}

void WebSeedListViewCtrl::saveSettings()
{		
	GetListViewDetails();
	save_to_ini();
}

void WebSeedListViewCtrl::uiUpdate(const hal::torrent_details_ptr pT)
{
	if (hal::bit::torrent t = hal::bittorrent::Instance().get(pT))
	{			
		if (auto lock = hal::try_update_lock<list_class_t>(this)) 
		{		
			auto web_seeds = t.web_seeds();
			DeleteAllItems();
			
			for (auto& web_seed : web_seeds)
			{
				int itemPos = AddItem(0, 0, web_seed.url.c_str(), 0);
				SetItemText(itemPos, 1,
					web_seed.type == hal::web_seed_detail::types::url ? hal::app().res_wstr(HAL_WEB_SEED_TYPE_URL).c_str() : hal::app().res_wstr(HAL_WEB_SEED_TYPE_HTTP).c_str());
			}
		}
	}
	else
	{		
		DeleteAllItems();
	}
}

LRESULT WebSeedListViewCtrl::OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	for (auto i = begin_selected(), e = end_selected(); i != e; ++i)
	{
		seedDeleted_(hal::web_seed_detail(i->text(), 
			i->text(1) == hal::app().res_wstr(HAL_WEB_SEED_TYPE_URL) ? hal::web_seed_detail::types::url : hal::web_seed_detail::types::http));

		DeleteItem(i->index());
	}

	return 0;
}

LRESULT AdvWebSeedsDialog::onInitDialog(HWND, LPARAM)
{
	dlg_base_class_t::InitializeHalDialogBase();	
	
	list_.Attach(GetDlgItem(HAL_WEB_SEEDS_LISTVIEW));	
	list_.attachDeletedConnection([this] (hal::web_seed_detail seed)
		{					
			if (auto t = hal::bittorrent::Instance().get(focused_torrent()))
				t.delete_web_seed(seed.url, seed.type);
		});

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
	
	list_.uiUpdate(pT);

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
	
	if (auto t = hal::bittorrent::Instance().get(focused_torrent()))
		t.add_web_seed(url_, hal::web_seed_detail::types::http);

	int itemPos = list_.AddItem(0, 0, url_.c_str(), 0);
	list_.SetItemText(itemPos, 1, hal::app().res_wstr(HAL_WEB_SEED_TYPE_HTTP).c_str());
}

void AdvWebSeedsDialog::onAddUrl(UINT, int, HWND)
{
	DoDataExchange(true);
	if (url_.empty()) return;

	HAL_DEV_MSG(hal::wform(L"Adding URL seed: %1%") % url_);
	
	if (auto t = hal::bittorrent::Instance().get(focused_torrent()))
		t.add_web_seed(url_, hal::web_seed_detail::types::url);

	int itemPos = list_.AddItem(0, 0, url_.c_str(), 0);
	list_.SetItemText(itemPos, 1, hal::app().res_wstr(HAL_WEB_SEED_TYPE_URL).c_str());

}
