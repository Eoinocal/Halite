
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include <algorithm>

#include "../HaliteWindow.hpp"
#include "../HaliteListViewCtrl.hpp"

#include "Files.hpp"

#define TVS_EX_DOUBLEBUFFER 0x0004

FileListView::FileListView() :
	iniClass("listviews/advFiles", "FileListView")
{}

HWND FileListView::Create(HWND hWndParent, ATL::_U_RECT rect, LPCTSTR szWindowName,
	DWORD dwStyle, DWORD dwExStyle,
	ATL::_U_MENUorID MenuOrID, LPVOID lpCreateParam)
{
	HWND hwnd = listClass::Create(hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	assert(hwnd);
	
	WTL::CMenuHandle menu;
	BOOL menu_created = menu.LoadMenu(LISTVIEW_ID_MENU);
	InitialSetup(menu);	

	std::vector<wstring> names;	
	wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);

	boost::split(names, column_names, boost::is_any_of(L";"));
	
	array<int, 5> widths = {100,70,70,70,70};
	array<int, 5> order = {0,1,2,3,4};
	array<bool, 5> visible = {true,true,true,true,true};

	for (int i=0, e=5; i < e; ++i)
	{
		AddColumn(names[i].c_str(), i, visible[i], widths[i]);
	}	

	load_from_ini();
	
/*	SetColumnSortType(2, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::Size());
	SetColumnSortType(3, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::Progress());
	SetColumnSortType(4, WTL::LVCOLSORT_CUSTOM, new ColumnAdapters::Priority());
	*/
	return hwnd;
}

void FileListView::OnMenuPriority(UINT uCode, int nCtrlID, HWND hwndCtrl)
{	
	std::vector<int> indices;
	
	for (int i=0, e=GetItemCount(); i<e; ++i)
	{
		UINT flags = GetItemState(i, LVIS_SELECTED);
		
		if (flags & LVIS_SELECTED)
			indices.push_back(GetItemData(i));
	}
	
	int priority = nCtrlID-ID_HAL_FILE_PRIORITY_0;	

	if (hal::bit::torrent t = hal::bittorrent().get(hal::bittorrent().torrentDetails().focusedTorrent()))
		t.file_priorities = std::pair<std::vector<int>, int>(indices, priority);
}

HWND FileTreeView::Create(HWND hWndParent, ATL::_U_RECT rect, LPCTSTR szWindowName, DWORD dwStyle, DWORD dwExStyle,
	ATL::_U_MENUorID MenuOrID, LPVOID lpCreateParam)
{
	HWND hwnd = treeClass::Create(hWndParent, (RECT &)rect.m_lpRect, szWindowName, dwStyle, dwExStyle, (UINT)MenuOrID.m_hMenu, lpCreateParam);
	assert(hwnd);
	
	WTL::CMenuHandle menu;
	BOOL menu_created = menu.LoadMenu(HAL_FILESLISTVIEW_MENU);
	assert(menu_created);	
	
	menu_.Attach(menu.GetSubMenu(0));
	
	return hwnd;
}

LRESULT FileTreeView::OnRClick(int i, LPNMHDR pnmh, BOOL&)
{
	determineFocused();
	LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)pnmh;
	
	if (menu_)
	{
		assert(menu_.IsMenu());

		WTL::CPoint cur_pt;
		GetCursorPos(&cur_pt);

		WTL::CPoint pt = cur_pt;			
		ScreenToClient(&pt);

		WTL::CTreeItem ti = HitTest(pt, 0);

		menu_.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
	}

	return 0;
}

void FileTreeView::OnMenuPriority(UINT uCode, int nCtrlID, HWND hwndCtrl)
{	
	hal::file_details_vec file_details;
	
	if (hal::torrent_details_ptr torrent = hal::bittorrent().torrentDetails().focusedTorrent())
	{
		std::copy(torrent->get_file_details().begin(), torrent->get_file_details().end(), 
			std::back_inserter(file_details));
	}

	wpath branch;
	
	if (WTL::CTreeItem ti = GetSelectedItem())
	{			
		do
		{
			if (!ti.GetParent()) break;
			
			boost::array<wchar_t, MAX_PATH> buffer;
			ti.GetText(buffer.elems, MAX_PATH);
			
			branch = wstring(buffer.elems)/branch;
		} 
		while (ti = ti.GetParent());
	}

	std::vector<int> indices;
	
	for (hal::file_details_vec::iterator i=file_details.begin(), e=file_details.end();
		i != e; ++i)
	{			
		if (std::equal(branch.begin(), branch.end(), (*i).branch.begin()))
		{
			indices.push_back((*i).order());
		}
	}
	
	int priority = nCtrlID-ID_HAL_FILE_PRIORITY_0;
	
	if (hal::bit::torrent t = hal::bittorrent().get(hal::bittorrent().torrentDetails().focusedTorrent()))
		t.file_priorities = std::pair<std::vector<int>, int>(indices, priority);
	
	hal::try_update_lock<thisClass> lock(*this);
	if (lock) signal();
}

void FileTreeView::determineFocused()
{
	wpath branch;
	
	if (WTL::CTreeItem ti = GetSelectedItem())
	{			
		do
		{
			if (!ti.GetParent()) break;
			
			boost::array<wchar_t, MAX_PATH> buffer;
			ti.GetText(buffer.elems, MAX_PATH);
			
			branch = wstring(buffer.elems)/branch;
		} 
		while (ti = ti.GetParent());
	}
	
	focused_ = branch;
}

LRESULT FileTreeView::OnSelChanged(int, LPNMHDR pnmh, BOOL&)
{	
	hal::try_update_lock<thisClass> lock(*this);
	if (lock)
	{		
		determineFocused();
		signal();
	}
	
	return 0;
}

LRESULT AdvFilesDialog::onInitDialog(HWND, LPARAM)
{
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
	
	WTL::CRect rc; GetClientRect(&rc);
	
	static_.SubclassWindow(GetDlgItem(HAL_CONTAINER));
	
	splitter_.Create(GetDlgItem(HAL_CONTAINER), rc, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
	splitter_.SetSplitterExtendedStyle(!SPLIT_PROPORTIONAL, SPLIT_PROPORTIONAL);
	
	list_.Create(splitter_, rc, NULL, 
		LVS_REPORT|WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|LVS_SHOWSELALWAYS,
		WS_EX_STATICEDGE|LVS_EX_DOUBLEBUFFER);
		
	tree_.Create(splitter_, rc, NULL, 
		WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|TVS_HASBUTTONS|
			TVS_HASLINES|TVS_TRACKSELECT|TVS_SHOWSELALWAYS,
		TVS_EX_DOUBLEBUFFER|WS_EX_STATICEDGE);
	
	tree_.attach(bind(&AdvFilesDialog::doUiUpdate, this));
	
	splitter_.SetSplitterPanes(tree_, list_);
	splitter_.SetSplitterPos(splitterPos);
	
	WTL::CTreeItem ti = tree_.InsertItem(hal::app().res_wstr(HAL_TORRENT_ROOT).c_str(), TVI_ROOT, TVI_LAST);
	
//	DoDataExchange(false);
	
	return 0;
}

void AdvFilesDialog::DlgResize_UpdateLayout(int cxWidth, int cyHeight)
{
	resizeClass::DlgResize_UpdateLayout(cxWidth, cyHeight);
	
	WTL::CRect rect; ::GetClientRect(GetDlgItem(HAL_CONTAINER), &rect);
	
	splitter_.SetWindowPos(NULL, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		SWP_NOZORDER | SWP_NOACTIVATE);
}

void AdvFilesDialog::doUiUpdate()
{
//	hal::event_log.post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::event_logger::info, (hal::wform(L"doUiUpdate %1%") % current_torrent_name_).str().c_str())));

	tree_.determineFocused();
	
	range_ = std::equal_range(fileLinks_.begin(), fileLinks_.end(),
		FileLink(tree_.focused()));
	
	std::sort(range_.first, range_.second, &FileLinkNamesLess);
	
	requestUiUpdate();
}

void AdvFilesDialog::uiUpdate(const hal::torrent_details_manager& tD)
{
	list_.setFocused(focusedTorrent());
	
	if (fileLinks_.empty() || !(focusedTorrent() && !focusedTorrent()->get_file_details().empty())) 
	{
		list_.DeleteAllItems();
		return;
	}
	
	hal::try_update_lock<FileListView::listClass> lock(list_);
	if (lock) 
	{
		hal::file_details_vec all_files = focusedTorrent()->get_file_details();
		hal::file_details_vec files;
		for (std::vector<FileLink>::iterator i=range_.first, e=range_.second;
			i != e; ++i)
		{		
			files.push_back(all_files[(*i).order()]);
		}

		// Wipe details not present
		for(int i = 0; i < list_.GetItemCount(); /*nothing here*/)
		{
			boost::array<wchar_t, MAX_PATH> fullPath;
			list_.GetItemText(i, 0, fullPath.c_array(), MAX_PATH);
			
			FileLink file(wstring(fullPath.c_array()));
			std::vector<FileLink>::iterator iter = 
				std::lower_bound(range_.first, range_.second, file, &FileLinkNamesLess);
			
			if (iter == range_.second || !(FileLinkNamesEqual((*iter), file)))
			{
				list_.DeleteItem(i);
			}
			else
			{
				list_.SetItemData(i, std::distance(range_.first, iter));
				++i;
			}
		}

		int col_sort_index = list_.GetSortColumn();
		if (col_sort_index != -1)
		{		
			int index = list_.GetColumnSortType(col_sort_index);
			
			HAL_DEV_MSG(hal::wform(L"col_sort_index() = %1%, index() = %2%") 
				% col_sort_index % index);

			if (index > WTL::LVCOLSORT_LAST)
				hal::file_details_sort(files, index - (WTL::LVCOLSORT_LAST+1+hal::file_details::branch_e), 
					list_.IsSortDescending());
		}

		bool sort_once = list_.IsSortOnce();

		if (list_.GetItemCount() < files.size())
		{
		// Add additional details
		for (size_t index = 0, e = files.size(); index < e; ++index)
		{
			hal::file_details& fd = files[index];
			
			int item_pos = index;
		
			HAL_DEV_SORT_MSG(hal::wform(L"AutoSort() = %1%, SortOnce() = %2%, !AutoSort() && !SortOnce() = %3%") 
				% AutoSort() % sort_once % (!AutoSort() && !sort_once));

			if (!list_.AutoSort() && !sort_once)
			{
				LV_FINDINFO findInfo; 
				findInfo.flags = LVFI_STRING;
				findInfo.psz = const_cast<LPTSTR>(fd.to_wstring(hal::file_details::filename_e).c_str());
				
				item_pos = list_.FindItem(&findInfo, -1);
			}

			if (item_pos == -1 || item_pos > list_.GetItemCount())
				item_pos = list_.AddItem(list_.GetItemCount(), 0, fd.to_wstring(hal::file_details::filename_e).c_str(), 0);
			
			list_.SetItemData(item_pos, fd.order());
			
			list_.SetItemText(item_pos, 0, fd.filename.c_str());		
			for (size_t i = hal::file_details::filename_e; i <= hal::file_details::priority_e; ++i)
			{
				list_.SetItemText(item_pos, i, fd.to_wstring(i).c_str());
			}	
		}
		}

		if (list_.AutoSort() && col_sort_index >= 0 && col_sort_index < list_.m_arrColSortType.GetSize())
		{
			if (list_.GetColumnSortType(col_sort_index) <= WTL::LVCOLSORT_CUSTOM)
				list_.DoSortItems(col_sort_index, list_.IsSortDescending());
		}
	}
}

void AdvFilesDialog::focusChanged(const hal::torrent_details_ptr pT)
{
	fileLinks_.clear();
	if (pT)
	{
		std::copy(pT->get_file_details().begin(), pT->get_file_details().end(), 
			std::back_inserter(fileLinks_));
	}
	
	list_.setFocused(pT);
	
	std::sort(fileLinks_.begin(), fileLinks_.end());
	
	{ 	hal::mutex_update_lock<FileTreeView> lock(tree_);
	
		treeManager_.InvalidateAll();
		
		foreach (FileLink file, fileLinks_)
		{
			treeManager_.EnsureValid(file.branch);
		}
		
		treeManager_.ClearInvalid();
	}
	
	tree_.determineFocused();
	
	range_ = std::equal_range(fileLinks_.begin(), fileLinks_.end(),
		FileLink(tree_.focused()));
	
	std::sort(range_.first, range_.second, &FileLinkNamesLess);
	
	splitterPos = splitter_.GetSplitterPos();
}

void AdvFilesDialog::onClose()
{	 
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

void AdvFilesDialog::OnDestroy()
{	 	
	save_to_ini(); 
}
