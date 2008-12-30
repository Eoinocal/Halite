
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

FileListView::FileListView(do_ui_update_fn uiu) :
	iniClass("listviews/advFiles", "FileListView"),
	do_ui_update_(uiu)
{}

HWND FileListView::Create(HWND hWndParent, ATL::_U_RECT rect, LPCTSTR szWindowName,
	DWORD dwStyle, DWORD dwExStyle,
	ATL::_U_MENUorID MenuOrID, LPVOID lpCreateParam)
{
	HWND hwnd = listClass::Create(hWndParent, rect.m_lpRect, szWindowName, dwStyle|LVS_OWNERDATA, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
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
	
	for (unsigned i=0, e = hal::file_details::priority_e-hal::file_details::filename_e; i <= e; ++i)
		SetColumnSortType(i, i + (WTL::LVCOLSORT_LAST+1+hal::file_details::filename_e), NULL);

	load_from_ini();

	return hwnd;
}

void FileListView::OnMenuPriority(UINT uCode, int nCtrlID, HWND hwndCtrl)
{	
	hal::mutex_t::scoped_lock l(mutex_);

	std::vector<int> indices;
	
	foreach(const list_value_type val, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		HAL_DEV_MSG(hal::wform(L"OnMenuPriority() = %1%, %2%, %3%") 
				% std::wstring(winstl::c_str_ptr(val)) % val.index() % files_[val.index()].order());
		indices.push_back(files_[val.index()].order());
	}
	
	int priority = nCtrlID-ID_HAL_FILE_PRIORITY_0;	

	if (hal::bit::torrent t = hal::bittorrent().get(hal::bittorrent().torrentDetails().focusedTorrent()))
		t.file_priorities = std::pair<std::vector<int>, int>(indices, priority);

	do_ui_update_();
}

LRESULT FileListView::OnGetDispInfo(int, LPNMHDR pnmh, BOOL&)
{	
	hal::mutex_t::scoped_lock l(mutex_);

	NMLVDISPINFO* pdi = (NMLVDISPINFO*)pnmh;

	if (pdi->item.iItem < files_.size())
	{
		if (pdi->item.mask & LVIF_TEXT)
		{
			wstring str = files_[pdi->item.iItem].to_wstring(pdi->item.iSubItem);
			
			int len = str.copy(pdi->item.pszText, min(pdi->item.cchTextMax - 1, str.size()));
			pdi->item.pszText[len] = '\0';
		}	
		
		if (pdi->item.mask & LVIF_PARAM)
		{			
			HAL_DEV_MSG(hal::wform(L"LVIF_PARAM() = %1%, %2%") 
				% pdi->item.iItem % files_[pdi->item.iItem].order());

			pdi->item.lParam = files_[pdi->item.iItem].order();
		}
	}
	
	return 0;
}

LRESULT FileListView::OnSortChanged(int, LPNMHDR pnmh, BOOL&)
{
	do_ui_update_();
	
	return 0;
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
	if (lock) do_ui_update_();
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
		do_ui_update_();
	}	
	return 0;
}

AdvFilesDialog::AdvFilesDialog(HaliteWindow& halWindow) :
	dialogBaseClass(halWindow),
	treeManager_(tree_),
	iniClass("AdvFilesDlg", "settings"),
	splitterPos(150),
	tree_(boost::bind(&AdvFilesDialog::doUiUpdate, this)),
	list_(boost::bind(&AdvFilesDialog::doUiUpdate, this))
{
	load_from_ini();
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
		
	splitter_.SetSplitterPanes(tree_, list_);
	splitter_.SetSplitterPos(splitterPos);
	
	WTL::CTreeItem ti = tree_.InsertItem(hal::app().res_wstr(HAL_TORRENT_ROOT).c_str(), TVI_ROOT, TVI_LAST);
		
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
	tree_.determineFocused();
	
	range_ = std::equal_range(fileLinks_.begin(), fileLinks_.end(),
		FileLink(tree_.focused()));
	
	std::sort(range_.first, range_.second, &FileLinkNamesLess);

	if (focusedTorrent())
	{
		hal::file_details_vec all_files = focusedTorrent()->get_file_details();	
		FileListView::scoped_files list_files = list_.files();

		list_files->clear();

		for (std::vector<FileLink>::iterator i=range_.first, e=range_.second;
			i != e; ++i)
		{		
			list_files->push_back(all_files[(*i).order()]);
		}
			
		list_.SetItemCountEx(list_files->size(),LVSICF_NOSCROLL);
	}

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
		FileListView::scoped_files list_files = list_.files();

		foreach (hal::file_details& file, *list_files)
		{
			file = all_files[file.order()];
		}

		if (list_.AutoSort() || list_.IsSortOnce())
		{
			int col_sort_index = list_.GetSortColumn();

			if (col_sort_index != -1)
			{		
				int index = list_.GetColumnSortType(col_sort_index);
		
				if (index > WTL::LVCOLSORT_LAST)
				{
					hal::file_details_sort(*list_files, index - (WTL::LVCOLSORT_LAST+1+hal::file_details::filename_e), 
						list_.IsSortDescending());
				}
			}
		}
	}

	list_.InvalidateRect(NULL,true);
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
