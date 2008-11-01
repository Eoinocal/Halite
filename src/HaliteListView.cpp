
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include <boost/iterator/filter_iterator.hpp>
#include <winstl/controls/listview_sequence.hpp>

#include "Halite.hpp"

#include "HaliteListView.hpp"
#include "HaliteWindow.hpp"
#include "HaliteListViewDlg.hpp"

#include "halTorrent.hpp"

#define HAL_CUSTOMDRAW_TITLEDATA 1000000000

HaliteListViewCtrl::HaliteListViewCtrl(HaliteWindow& HalWindow) :
	halWindow_(HalWindow),
	iniClass("listviews/halite", "HaliteListView")
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
	
	array<int, NumberOfColumns_s> widths = {100,110,60,60,60,42,45,61,45,45,45,45,45,45,45,45,45,45,45,45,45,45};
	array<bool, NumberOfColumns_s> visible = {true,true,true,true,true,true,true,true,true,true,true,\
		true,true,true,true,true,true,true,true,true,true,true};

	for (int i=0, e=NumberOfColumns_s; i < e; ++i)
	{
		AddColumn(names[i].c_str(), i, visible[i], widths[i]);
	}	


//	int ret = EnableGroupView(true);
/*	if (IsGroupViewEnabled())
	{
//		RemoveAllGroups();

		LVGROUP lvg = { sizeof(LVGROUP) };

		lvg.mask = LVGF_HEADER|LVGF_GROUPID|LVGF_STATE|LVGF_ALIGN ;
		lvg.pszHeader = L"Eoin";
		lvg.cchHeader = 5;
		lvg.iGroupId = 1;
		lvg.state = LVGS_NORMAL;
		lvg.uAlign = LVGA_HEADER_LEFT;

		int grp = InsertGroup(-1, &lvg);
//		MoveItemToGroup(1, 1);
	}
*/
	SafeLoadFromIni();
	
	SetColumnSortType(0, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::Name());
	SetColumnSortType(1, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::State());
	SetColumnSortType(2, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::Progress());
	SetColumnSortType(3, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::SpeedDown());
	SetColumnSortType(4, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::SpeedUp());
	SetColumnSortType(5, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::Peers());
	SetColumnSortType(6, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::Seeds());
	SetColumnSortType(7, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::ETA());
	SetColumnSortType(8, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::DistributedCopies());
	SetColumnSortType(9, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::Tracker());
	SetColumnSortType(10, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::UpdateTrackerIn());
	SetColumnSortType(11, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::Ratio());
	SetColumnSortType(12, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::TotalWanted());
	SetColumnSortType(13, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::Completed());
	SetColumnSortType(14, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::Remaining());
	SetColumnSortType(15, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::Downloaded());
	SetColumnSortType(16, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::Uploaded());
	SetColumnSortType(17, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::ActiveTime());
	SetColumnSortType(18, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::SeedingTime());
	SetColumnSortType(19, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::StartTime());
	SetColumnSortType(20, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::FinishTime());
	SetColumnSortType(21, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::QueuePosition());

	
/*	int item_pos = AddItem(0, 0, L"Unmanaged", 0);
	SetItemData(item_pos, HAL_CUSTOMDRAW_TITLEDATA);
			
	item_pos = AddItem(0, 0, L"Downloading", 0);
	SetItemData(item_pos, HAL_CUSTOMDRAW_TITLEDATA);
			
	item_pos = AddItem(0, 0, L"Seeding", 0);
	SetItemData(item_pos, HAL_CUSTOMDRAW_TITLEDATA);*/
}

void HaliteListViewCtrl::OnDestroy()
{
	saveSettings();
}

void HaliteListViewCtrl::saveSettings()
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

void HaliteListViewCtrl::uiUpdate(const hal::torrent_details_manager& tD)
{
	hal::try_update_lock<listClass> lock(*this);
	if (lock) 
	{
	


	foreach (const hal::torrent_details_ptr td, tD.torrents()) 
	{
		LV_FINDINFO findInfo; 
		findInfo.flags = LVFI_STRING;
		findInfo.psz = const_cast<LPTSTR>(td->name().c_str());
		
		int itemPos = FindItem(&findInfo, -1);
		if (itemPos < 0)
		{
/*			LVITEM lvItem = { 0 };
			lvItem.mask = LVIF_TEXT|LVIF_GROUPID;
			lvItem.iItem = 0;
			lvItem.iSubItem = 0;
			lvItem.pszText = (LPTSTR)td->name().c_str();
			lvItem.iGroupId = 0;

			lvItem.mask |= LVIF_IMAGE;
			lvItem.iImage = 0;

			itemPos =  InsertItem(&lvItem);
*/
			AddItem(0, 0, td->name().c_str(), 0);
		//	MoveItemToGroup(itemPos, 0);
		}

		for (size_t i=1; i<NumberOfColumns_s; ++i)
		{
			SetItemText(itemPos, i, getColumnAdapter(i)->print(td).c_str());
		}
	}
	
	int iCol = GetSortColumn();
	if (autoSort() && iCol >= 0 && iCol < m_arrColSortType.GetSize())
		DoSortItems(iCol, IsSortDescending());
	
	}
}

HaliteListViewCtrl::tD HaliteListViewCtrl::CustomItemConversion(LVCompareParam* param, int iSortCol)
{
	boost::array<wchar_t, MAX_PATH> buffer;
	GetItemText(param->iItem, 0, buffer.c_array(), buffer.size());		
	wstring torrent = buffer.data();
	
	return hal::bittorrent().torrentDetails().get(torrent);
}

LRESULT HaliteListViewCtrl::OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind((void (hal::bit::*)(const std::wstring&))&hal::bit::resumeTorrent, 
			&hal::bittorrent(), _1));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind((void (hal::bit::*)(const std::wstring&))&hal::bit::pauseTorrent, 
			&hal::bittorrent(), _1));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind((void (hal::bit::*)(const std::wstring&))&hal::bit::stopTorrent, 
			&hal::bittorrent(), _1));

	return 0;
}

LRESULT HaliteListViewCtrl::OnRemoveFocused(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	hal::bittorrent().remove_torrent(hal::to_utf8(manager_.selected()));

	clearFocused();	
	return 0;
}

LRESULT HaliteListViewCtrl::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bittorrent().remove_torrent(val.text().c_str());
	}
	clearSelected();

	return 0;
}

LRESULT HaliteListViewCtrl::OnRecheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind((void (hal::bit::*)(const std::wstring&))&hal::bit::recheckTorrent, 
			&hal::bittorrent(), _1));	

	return 0;
}

LRESULT HaliteListViewCtrl::OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(MessageBox(hal::app().res_wstr(HAL_LISTVIEW_CONFIRMDELETE).c_str(), 
				hal::app().res_wstr(HAL_HALITE).c_str(), MB_YESNO) == IDYES)
	{
		std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
			bind((void (hal::bit::*)(const std::wstring&))&hal::bit::remove_torrent_wipe_files, 
				&hal::bittorrent(), _1));
		
		clearSelected();
	}
	return 0;
}

LRESULT HaliteListViewCtrl::OnDownloadFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HAL_DEV_MSG(L"OnDownloadFolder");

	std::set<wpath> uniquePaths;

	for(std::set<wstring>::const_iterator i=manager().allSelected().begin(), e=manager().allSelected().end();
		i != e; ++i)
	{
		wpath saveDir = hal::bittorrent().get(*i).save_directory;		
		HAL_DEV_MSG(hal::wform(L"Name %1%, Save dir: %2%.") % *i % saveDir);

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

	if (hal::bit::torrent t = hal::bittorrent().get(manager_.selected()))
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

//LRESULT HaliteListViewCtrl::OnDeleteItem(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
//{
//	LPNMLISTVIEW pnmv=(LPNMLISTVIEW)pnmh;
//	T* pItem=(T*)GetItemData(pnmv->iItem);
//	ATLASSERT(pItem);
//	if (pItem)	// Delete attached structure
//		delete pItem;
//	return 0;
//}
