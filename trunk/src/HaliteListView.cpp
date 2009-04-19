
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
		SetColumnSortType(i, i + (WTL::LVCOLSORT_LAST+1+hal::torrent_details::name_e));
	
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
		hal::bittorrent::Instance().torrentDetails().get(l), hal::bittorrent::Instance().torrentDetails().get(r), index, ascending);
}

LRESULT HaliteListViewCtrl::OnGetDispInfo(int, LPNMHDR pnmh, BOOL&)
{	
	hal::try_update_lock<listClass> lock(*this);
	if (lock) 
	{	

	NMLVDISPINFO* pdi = (NMLVDISPINFO*)pnmh;
	hal::torrent_details_ptr td = hal::bittorrent::Instance().torrentDetails().get(key_from_index(pdi->item.iItem));

	if (td && pdi->item.mask & LVIF_TEXT)
	{
		wstring str = td->to_wstring(pdi->item.iSubItem);
		
		size_t len = str.copy(pdi->item.pszText, min(pdi->item.cchTextMax - 1, static_cast<int>(str.size())));
		pdi->item.pszText[len] = '\0';
	}

	}
	
	return 0;
}

void HaliteListViewCtrl::uiUpdate(const hal::torrent_details_manager& tD)
{
	hal::try_update_lock<listClass> lock(*this);
	if (lock) 
	{		

	selection_from_listview();
		
	std::set<std::wstring> torrent_set;
	for (size_t td_index=0, e=tD.torrents().size(); td_index<e; ++td_index)
	{
		hal::torrent_details_ptr td = tD.torrents()[td_index];
		torrent_set.insert(td->name());
	}
	
	erase_based_on_set(torrent_set, true);	

	if (IsSortOnce() || AutoSort())
	{
		if (GetSecondarySortColumn() != -1)
		{
			int index = GetColumnSortType(GetSecondarySortColumn());					
			if (index > WTL::LVCOLSORT_LAST)
				sort(index - (WTL::LVCOLSORT_LAST+1+hal::torrent_details::name_e), IsSecondarySortDescending());
		}

		if (GetSortColumn() != -1)
		{		
			int index = GetColumnSortType(GetSortColumn());				
			if (index > WTL::LVCOLSORT_LAST)
				sort(index - (WTL::LVCOLSORT_LAST+1+hal::torrent_details::name_e), IsSortDescending());
		}
	}
	
	if (queue_view_)
		sort(hal::torrent_details::managed_e, false);

	set_keys(torrent_set);	
	InvalidateRect(NULL,true);

	}
}

LRESULT HaliteListViewCtrl::OnSortChanged(int, LPNMHDR pnmh, BOOL&)
{
	halite_window_.issueUiUpdate();
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bittorrent::Instance().resume_torrent(val.text().c_str());
	}
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bittorrent::Instance().pause_torrent(val.text().c_str());
	}
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bittorrent::Instance().stop_torrent(val.text().c_str());
	}

	return 0;
}

LRESULT HaliteListViewCtrl::OnRemoveFocused(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	hal::bittorrent::Instance().remove_torrent(hal::to_utf8(is_selected_begin()->text().c_str()));
	erase_from_list(*is_selected_begin());

	return 0;
}

LRESULT HaliteListViewCtrl::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::set<wstring>  torrent_names;

	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
		torrent_names.insert(hal::to_wstr_shim(val));
	
	erase_based_on_set(torrent_names, false);

	foreach(wstring name, torrent_names)
		hal::bittorrent::Instance().remove_torrent(name);

	return 0;
}

LRESULT HaliteListViewCtrl::OnRecheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	foreach(const list_value_type& v, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bittorrent::Instance().recheck_torrent(v.text().c_str());
	}

	return 0;
}

void HaliteListViewCtrl::remove_to_bin(hal::fs::wpath root, boost::shared_ptr<hal::file_details_vec> files)
{
	std::vector<wchar_t> file_names_buffer;

	foreach(hal::file_details file, *files)
	{
		std::wstring file_location = (hal::wform(L"File %1%\\%2%\\%3%") 
			% root.file_string() % file.branch % file.filename).str();

		wstring full_file = hal::fs::wpath(root / file.branch / file.filename).file_string();
		 
		HAL_DEV_MSG(hal::wform(L"File %1%") % full_file);

		std::copy(full_file.begin(), full_file.end(), 
			std::back_inserter(file_names_buffer));
		file_names_buffer.push_back(L'\0');
	}
	file_names_buffer.push_back(L'\0');

	SHFILEOPSTRUCT shf;

	shf.hwnd = *this;
	shf.wFunc = FO_DELETE;
	shf.pFrom = &file_names_buffer[0];
	shf.pTo = 0;
	shf.fFlags = FOF_ALLOWUNDO;

	SHFileOperation(&shf);
}

LRESULT HaliteListViewCtrl::OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::set<wstring>  torrent_names;

	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
		torrent_names.insert(hal::to_wstr_shim(val));
	
	erase_based_on_set(torrent_names, false);

	foreach(wstring name, torrent_names)
		hal::bittorrent::Instance().remove_torrent_wipe_files(name, bind(&HaliteListViewCtrl::remove_to_bin, this, _1, _2));

	return 0;
}

LRESULT HaliteListViewCtrl::OnDownloadFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HAL_DEV_MSG(L"OnDownloadFolder");

	std::set<wpath> uniquePaths;

	foreach(const list_value_type& v, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		wpath saveDir = hal::bittorrent::Instance().get(v).save_directory;		
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

	if (hal::bit::torrent t = hal::bittorrent::Instance().get(*is_selected_begin()))
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
/*	hal::try_update_lock<listClass> lock(*this);
	if (lock) 
	{		
*/
	std::set<wstring>  torrent_names;

	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
		torrent_names.insert(hal::to_wstr_shim(val));
	
//	erase_based_on_set(torrent_names, false);

	foreach(wstring name, torrent_names)
		hal::bittorrent::Instance().get(name).managed = true;
//	}

	halite_window_.issueUiUpdate();

	return 0;
}

LRESULT HaliteListViewCtrl::OnSetUnmanaged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
/*	hal::try_update_lock<listClass> lock(*this);
	if (lock) 
	{		
*/
	std::set<wstring>  torrent_names;

	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
		torrent_names.insert(hal::to_wstr_shim(val));
	
//	erase_based_on_set(torrent_names, false);

	foreach(wstring name, torrent_names)
		hal::bittorrent::Instance().get(name).managed = false;
//	}

	halite_window_.issueUiUpdate();

	return 0;
}

LRESULT HaliteListViewCtrl::OnAdjustQueuePosition(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	foreach(const list_value_type v, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bit::torrent t = hal::bittorrent::Instance().get(v);

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
//	queue_view_ ^= true;
	queue_view_ = false;

	queue_view_mode();
	
	return 0;
}

void HaliteListViewCtrl::erase_torrent_name(wstring name)
{
	erase_from_list(name);
}

void HaliteListViewCtrl::queue_view_mode()
{
	erase_all_from_list();

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
