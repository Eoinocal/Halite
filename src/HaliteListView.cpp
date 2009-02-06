
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include <boost/iterator/filter_iterator.hpp>
#include <winstl/controls/listview_sequence.hpp>

#include "Halite.hpp"

#include "halTorrent.hpp"

#include "HaliteListView.hpp"
#include "HaliteWindow.hpp"
#include "HaliteListViewDlg.hpp"

#define HAL_CUSTOMDRAW_TITLEDATA 1000000000

HaliteListViewCtrl::HaliteListViewCtrl(HaliteWindow& HalWindow) :
	halite_window_(HalWindow),
	iniClass("listviews/halite", "HaliteListView"),
	queue_view_(false)
{		
	HalWindow.connectUiUpdate(bind(&HaliteListViewCtrl::uiUpdate, this, _1));
}

void HaliteListViewCtrl::OnShowWindow(UINT, INT)
{
	WTL::CMenuHandle menu;
	BOOL menu_created = menu.LoadMenu(HAL_LISTVIEW_MENU);
	InitialSetup(menu);	

	std::vector<wstring> names;	
	wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);
	
	boost::split(names, column_names, boost::is_any_of(L";"));
	
	array<int, NumberOfColumns_s> widths = {100,110,60,60,60,42,45,61,45,45,45,45,45,45,45,45,45,45,45,45,45,30,45};
	array<bool, NumberOfColumns_s> visible = {true,true,true,true,true,true,true,true,true,true,true,\
		true,true,true,true,true,true,true,true,true,true,true,true};

	for (int i=0, e=NumberOfColumns_s; i < e; ++i)
	{
		AddColumn(names[i].c_str(), i, visible[i], widths[i]);
	}	

	SafeLoadFromIni();

	for (unsigned i=0, e = hal::torrent_details::queue_position_e-hal::torrent_details::name_e; i <= e; ++i)
		SetColumnSortType(i, i + (WTL::LVCOLSORT_LAST+1+hal::torrent_details::name_e), NULL);
	
//	SetColumnSortType(0, WTL::LVCOLSORT_TEXTNOCASE, NULL);
	
	queue_view_mode();
}

void HaliteListViewCtrl::OnDestroy()
{
	SaveSettings();
}

void HaliteListViewCtrl::SaveSettings()
{
	GetListViewDetails();
	save_to_ini();
}

DWORD HaliteListViewCtrl::OnPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCD)
{
	return CDRF_NOTIFYITEMDRAW;
}

DWORD HaliteListViewCtrl::OnItemPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCD)
{
	NMLVCUSTOMDRAW* pnmlv = (NMLVCUSTOMDRAW*) lpNMCD;

	if (HAL_CUSTOMDRAW_TITLEDATA == pnmlv->nmcd.lItemlParam)
	{
		pnmlv->clrText = RGB(50,50,200);
	}

	return CDRF_DODEFAULT;
}

bool HaliteListViewCtrl::sort_list_comparison(std::wstring l, std::wstring r, size_t index, bool ascending)
{
	return hal::hal_details_ptr_compare(
		hal::bittorrent().torrentDetails().get(l), hal::bittorrent().torrentDetails().get(r), index, ascending);
}

void HaliteListViewCtrl::uiUpdate(const hal::torrent_details_manager& tD)
{
	hal::try_update_lock<listClass> lock(*this);
	if (lock) 
	{		
	
	int col_sort_index = GetSortColumn();

	// Perform external ListView sort here.
	if (col_sort_index != -1)
	{		
		if (GetSecondarySortColumn() != -1)
		{
			int index = GetColumnSortType(GetSecondarySortColumn());
			
		//	if (index > WTL::LVCOLSORT_LAST)
		//		sort(index - (WTL::LVCOLSORT_LAST+1+hal::torrent_details::name_e), IsSecondarySortDescending());
		}

		int index = GetColumnSortType(col_sort_index);
		
		if (index > WTL::LVCOLSORT_LAST)
			sort(index - (WTL::LVCOLSORT_LAST+1+hal::torrent_details::name_e), IsSortDescending());
	}

//	if (IsGroupViewEnabled())
//		sort(hal::torrent_details::managed_e, true);

	bool sort_once = IsSortOnce();

	// Update details here.
	for (size_t td_index=0, e=tD.torrents().size(); td_index<e; ++td_index)
	{
		hal::torrent_details_ptr td = tD.torrents()[td_index];
	
		int item_pos = td_index;
		
		HAL_DEV_SORT_MSG(hal::wform(L"AutoSort() = %1%, SortOnce() = %2%, !AutoSort() && !SortOnce() = %3%") 
			% AutoSort() % sort_once % (!AutoSort() && !sort_once));
		
		HAL_DEV_SORT_MSG(hal::wform(L"Item = %1%, Index = %2%") % td->name() % item_pos);

		LVITEM lvItem = { 0 };
		lvItem.mask = LVIF_TEXT|LVIF_STATE;
		lvItem.stateMask = LVIS_SELECTED;
		lvItem.state = 0;
		lvItem.iSubItem = 0;
		lvItem.pszText = (LPTSTR)td->name().c_str();

		if (tD.selected_names().find(td->name()) != tD.selected_names().end())
			lvItem.state = LVIS_SELECTED;

		if (IsGroupViewEnabled())
		{
			lvItem.mask |= LVIF_GROUPID|LVIF_COLUMNS;

			if (td->managed())
				lvItem.iGroupId = HAL_AUTO_MANAGED;
			else
				lvItem.iGroupId = HAL_UNMANAGED;
		}

		lvItem.mask |= LVIF_IMAGE;
		lvItem.iImage = 0;

		item_pos = InsertKeyItem(td->name(), &lvItem);

		for (size_t i=1; i<NumberOfColumns_s; ++i)
		{
			SetItemText(item_pos, i, td->to_wstring(i).c_str());
		}
	}
	
	// Perform internal ListView sort here.
/*	if (AutoSort() && col_sort_index >= 0 && col_sort_index < m_arrColSortType.GetSize())
	{
		if (GetColumnSortType(col_sort_index) <= WTL::LVCOLSORT_CUSTOM)
			DoSortItems(col_sort_index, IsSortDescending());
	}
*/	
	}
}

LRESULT HaliteListViewCtrl::OnSortChanged(int, LPNMHDR pnmh, BOOL&)
{
	halite_window_.issueUiUpdate();
	
	return 0;
}

/*HaliteListViewCtrl::tD HaliteListViewCtrl::CustomItemConversion(LVCompareParam* param, int iSortCol)
{
	boost::array<wchar_t, MAX_PATH> buffer;
	GetItemText(param->iItem, 0, buffer.c_array(), buffer.size());		
	wstring torrent = buffer.data();
	
	return hal::bittorrent().torrentDetails().get(torrent);
}*/
/*
HaliteListViewCtrl::tD HaliteListViewCtrl::convert(const LPLVITEM item)
{	
	win_c_str<std::wstring> str(MAX_PATH);
	GetItemText(item->iItem, 0, str, str.size());

	return hal::bittorrent().torrentDetails().get(str);
}

void HaliteListViewCtrl::convert(LPLVITEM item, const HaliteListViewCtrl::AdapterType tD)
{
	win_c_str<std::wstring> str(MAX_PATH);

	GetItemText(item);
}
*/
LRESULT HaliteListViewCtrl::OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bittorrent().resume_torrent(val.text().c_str());
	}
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bittorrent().pause_torrent(val.text().c_str());
	}
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bittorrent().stop_torrent(val.text().c_str());
	}

	return 0;
}

LRESULT HaliteListViewCtrl::OnRemoveFocused(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	hal::bittorrent().remove_torrent(hal::to_utf8(is_selected_begin()->text().c_str()));
	erase_from_list(*is_selected_begin());

	return 0;
}

LRESULT HaliteListViewCtrl::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	foreach(const list_value_type& v, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bittorrent().remove_torrent(v.text().c_str());
		erase_from_list(v);
	}

	return 0;
}

LRESULT HaliteListViewCtrl::OnRecheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	foreach(const list_value_type& v, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bittorrent().recheck_torrent(v.text().c_str());
	}

	return 0;
}

LRESULT HaliteListViewCtrl::OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(MessageBox(hal::app().res_wstr(HAL_LISTVIEW_CONFIRMDELETE).c_str(), 
				hal::app().res_wstr(HAL_HALITE).c_str(), MB_YESNO) == IDYES)
	{
		is_selected_iterator i=is_selected_end();
		if (i != is_selected_begin())
		{
			--i;
			while (i != is_selected_begin())
			{
				hal::bittorrent().remove_torrent_wipe_files(i->text().c_str());
				erase_from_list(*i);
			} 
		}
	}

	return 0;
}

LRESULT HaliteListViewCtrl::OnDownloadFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HAL_DEV_MSG(L"OnDownloadFolder");

	std::set<wpath> uniquePaths;

//	for(std::set<wstring>::const_iterator i=manager().allSelected().begin(), e=manager().allSelected().end();
//		i != e; ++i)
	foreach(const list_value_type& v, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		wpath saveDir = hal::bittorrent().get(v).save_directory;		
		HAL_DEV_MSG(hal::wform(L"Name %1%, Save dir: %2%.") % v.text() % saveDir);

		uniquePaths.insert(saveDir);
	}

	SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };

	for(std::set<wpath>::const_iterator i=uniquePaths.begin(), e=uniquePaths.end();
		i != e; ++i)
	{	
		wstring p = (*i).file_string();

		HAL_DEV_MSG(hal::wform(L"Unique Save dir: %1%.") % p);

		sei.lpDirectory = p.c_str();
		sei.lpFile = p.c_str();
		sei.lpVerb = L"open";
		sei.nShow = true;

		if (!::ShellExecuteEx(&sei))
			HAL_DEV_MSG(L"Fail");
		else
			HAL_DEV_MSG(L"Success");
	}	

	return 0;
}

LRESULT HaliteListViewCtrl::OnEditFolders(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HAL_DEV_MSG(L"OnEditFolders");

	if (hal::bit::torrent t = hal::bittorrent().get(*is_selected_begin()))
	{
		wstring saveDirectory = static_cast<wpath>(t.save_directory).native_file_string();
		wstring moveToDirectory = static_cast<wpath>(t.move_to_directory).native_file_string();

		bool useMoveTo = (L"" != moveToDirectory);
		bool disableSaveDir = !t.in_session;

		HaliteListViewAdjustDlg addTorrent(hal::app().res_wstr(HAL_ADDT_TITLE), saveDirectory, moveToDirectory, 
			useMoveTo, disableSaveDir);	
		
		if (IDOK == addTorrent.DoModal())
		{
			if (!disableSaveDir) t.save_directory = saveDirectory;

			if (useMoveTo)
				t.move_to_directory = moveToDirectory;
			else
				t.move_to_directory = L"";
		}
	}

	return 0;
}

LRESULT HaliteListViewCtrl::OnSetManaged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	for (is_selected_iterator i = is_selected_end(); 
			i != is_selected_begin() &&  is_selected_begin() != is_selected_end(); /**/)
	{
		--i;
		hal::bittorrent().get(*i).managed = true;
		erase_from_list(*i);
	}

	halite_window_.issueUiUpdate();

	return 0;
}

LRESULT HaliteListViewCtrl::OnSetUnmanaged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	for (is_selected_iterator i = is_selected_end(); 
			i != is_selected_begin() &&  is_selected_begin() != is_selected_end(); /**/)
	{
		--i;
		hal::bittorrent().get(*i).managed = false;
		erase_from_list(*i);
	}

	halite_window_.issueUiUpdate();

	return 0;
}

LRESULT HaliteListViewCtrl::OnAdjustQueuePosition(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	foreach(const list_value_type v, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bit::torrent t = hal::bittorrent().get(v);

		switch (wID)
		{
		case HAL_QUEUE_MOVE_TOP:
			t.adjust_queue_position(hal::bit::move_to_top);
			break;
		case HAL_QUEUE_MOVE_UP:
			t.adjust_queue_position(hal::bit::move_up);		
			break;
		case HAL_QUEUE_MOVE_DOWN:
			t.adjust_queue_position(hal::bit::move_down);		
			break;
		case HAL_QUEUE_MOVE_BOTTOM:
			t.adjust_queue_position(hal::bit::move_to_bottom);		
			break;
		};
	}

	halite_window_.issueUiUpdate();
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnQueueView(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	queue_view_ ^= true;

	queue_view_mode();
	
	return 0;
}

void HaliteListViewCtrl::queue_view_mode()
{
	DeleteAllItems();

	if (queue_view_)
	{
		int ret = EnableGroupView(true);
		if (IsGroupViewEnabled())
		{
			LVGROUP lvg = { sizeof(LVGROUP) };

			lvg.mask = LVGF_HEADER|LVGF_GROUPID ;
			wstring unmanaged = hal::app().res_wstr(HAL_UNMANAGED);
			lvg.pszHeader = (LPWSTR)unmanaged.c_str();
			lvg.iGroupId = HAL_UNMANAGED;

			int grp = InsertGroup(-1, &lvg);

			lvg.mask = LVGF_HEADER|LVGF_GROUPID ;
			wstring managed_seed = hal::app().res_wstr(HAL_MANAGED_SEEDING);
			lvg.pszHeader = (LPWSTR)managed_seed.c_str();
			lvg.iGroupId = HAL_MANAGED_SEEDING;

			grp = InsertGroup(-1, &lvg);

			lvg.mask = LVGF_HEADER|LVGF_GROUPID ;
			wstring managed_down = hal::app().res_wstr(HAL_MANAGED_DOWNLOADING);
			lvg.pszHeader = (LPWSTR)managed_down.c_str();
			lvg.iGroupId = HAL_MANAGED_DOWNLOADING;

			grp = InsertGroup(-1, &lvg);

			lvg.mask = LVGF_HEADER|LVGF_GROUPID ;
			wstring auto_managed = hal::app().res_wstr(HAL_AUTO_MANAGED);
			lvg.pszHeader = (LPWSTR)auto_managed.c_str();
			lvg.iGroupId = HAL_AUTO_MANAGED;

			grp = InsertGroup(-1, &lvg);
		}
	}
	else
	{
		RemoveAllGroups();
		int ret = EnableGroupView(false);
	}
	halite_window_.issueUiUpdate();

	MENUITEMINFO minfo = {sizeof(MENUITEMINFO)};
	
	minfo.fMask = MIIM_STATE;
	minfo.fState = queue_view_ ? MFS_CHECKED : MFS_UNCHECKED;
	
	menu_.SetMenuItemInfo(HAL_LVM_QUEUE_VIEW, false, &minfo);
}

//LRESULT HaliteListViewCtrl::OnDeleteItem(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
//{
//	LPNMLISTVIEW pnmv=(LPNMLISTVIEW)pnmh;
//	T* pItem=(T*)GetItemData(pnmv->iItem);
//	ATLASSERT(pItem);
//	if (pItem)	// Delete attached structure
//		delete pItem;
//	return 0;
//}
