
//         Copyright Eóin O'Callaghan 2006 - 2009.
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
	ini_class_t("listviews/advFiles", "FileListView"),
	do_ui_update_(uiu)
{}

HWND FileListView::Create(HWND hWndParent, ATL::_U_RECT rect, LPCTSTR szWindowName,
	DWORD dwStyle, DWORD dwExStyle,
	ATL::_U_MENUorID MenuOrID, LPVOID lpCreateParam)
{
	HWND hwnd = list_class_t::Create(hWndParent, rect.m_lpRect, szWindowName, dwStyle|LVS_OWNERDATA|LVS_EDITLABELS, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
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
		indices.push_back(numeric_cast<int>(files_[val.index()].order()));
	}
	
	int priority = nCtrlID-ID_HAL_FILE_PRIORITY_0;	

	if (hal::bit::torrent t = hal::bittorrent::Instance().get(hal::bittorrent::Instance().torrentDetails().focused_torrent()))
		t.file_priorities = std::pair<std::vector<int>, int>(indices, priority);

	do_ui_update_();
}

LRESULT FileListView::OnGetDispInfo(int, LPNMHDR pnmh, BOOL&)
{	
	hal::mutex_t::scoped_lock l(mutex_);

	NMLVDISPINFO* pdi = (NMLVDISPINFO*)pnmh;

	if (pdi->item.iItem < static_cast<int>(files_.size()))
	{
		if (pdi->item.mask & LVIF_TEXT)
		{
			wstring str = files_[pdi->item.iItem].to_wstring(pdi->item.iSubItem);
			
			size_t len = str.copy(pdi->item.pszText, min(pdi->item.cchTextMax - 1, static_cast<int>(str.size())));
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

LRESULT FileListView::OnBeginLabelEdit(int i, LPNMHDR pnmh, BOOL&)
{		
	HAL_DEV_MSG(hal::wform(L"OnBeginLabelEdit(int i = %1%)") % i);

	lock_ptr_.reset(new hal::try_update_lock<list_class_t>(this));
	if (*lock_ptr_) 
	{	
		NMLVDISPINFO* nmlv = (NMLVDISPINFO*)pnmh;
		
		HAL_DEV_MSG(hal::wform(L"OnBeginLabelEdit(int i = %1%)") % i);

		return false;
	}

	//

	return false;
}

LRESULT FileListView::OnEndLabelEdit(int i, LPNMHDR pnmh, BOOL&)
{	
	HAL_DEV_MSG(hal::wform(L"OnEndLabelEdit(int i = %1%)") % i);

	NMLVDISPINFO* pdi = (NMLVDISPINFO*)pnmh;
	wstring str;

	if (pdi->item.iItem < static_cast<int>(files_.size()))
	{
		if (pdi->item.mask & LVIF_TEXT)
		{
			str = files_[pdi->item.iItem].to_wstring(pdi->item.iSubItem);
		}
	}
	
	HAL_DEV_MSG(hal::wform(L"iItem: %1%, text: %2%, orig: %3%") % pdi->item.iItem % pdi->item.pszText % str);



	lock_ptr_.reset();

	return false;
}

LRESULT FileListView::OnSortChanged(int, LPNMHDR pnmh, BOOL&)
{
	do_ui_update_();
	
	return 0;
}

HWND FileTreeView::Create(HWND hWndParent, ATL::_U_RECT rect, LPCTSTR szWindowName, DWORD dwStyle, DWORD dwExStyle,
	ATL::_U_MENUorID MenuOrID, LPVOID lpCreateParam)
{
	HWND hwnd = treeClass::Create(hWndParent, (RECT&)rect.m_lpRect, szWindowName, dwStyle|TVS_EDITLABELS, dwExStyle, (UINT)MenuOrID.m_hMenu, lpCreateParam);
	assert(hwnd);
	
	WTL::CMenuHandle menu;
	BOOL menu_created = menu.LoadMenu(HAL_FILESLISTVIEW_MENU);
	assert(menu_created);	
	
	menu_.Attach(menu.GetSubMenu(0));
	
	return hwnd;
}

LRESULT FileTreeView::OnBeginLabelEdit(int i, LPNMHDR pnmh, BOOL&)
{		
	HAL_DEV_MSG(hal::wform(L"OnBeginLabelEdit(int i = %1%)") % i);

	lock_ptr_.reset(new hal::try_update_lock<this_class_t>(this));
	if (*lock_ptr_) 
	{	
		NMTVDISPINFO* pdi = (NMTVDISPINFO*)pnmh;;
		
		HAL_DEV_MSG(hal::wform(L"OnBeginLabelEdit(int i = %1%)") % i);

		return false;
	}

	//

	return false;
}

LRESULT FileTreeView::OnEndLabelEdit(int i, LPNMHDR pnmh, BOOL&)
{	
	HAL_DEV_MSG(hal::wform(L"OnEndLabelEdit(int i = %1%)") % i);

	NMTVDISPINFO* pdi = (NMTVDISPINFO*)pnmh;
	wstring str;

/*	if (pdi->item.iItem < static_cast<int>(files_.size()))
	{
		if (pdi->item.mask & LVIF_TEXT)
		{
			str = files_[pdi->item.iItem].to_wstring(pdi->item.iSubItem);
		}
	}
*/	
	HAL_DEV_MSG(hal::wform(L"state: %1%, text: %2%") % pdi->item.state % pdi->item.pszText);

	lock_ptr_.reset();

	return false;
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

		menu_.TrackPopupMenu(0, cur_pt.x, cur_pt.y, m_hWnd);
	}

	return 0;
}

void FileTreeView::OnMenuPriority(UINT uCode, int nCtrlID, HWND hwndCtrl)
{	
	hal::file_details_vec file_details;
	
	if (hal::torrent_details_ptr torrent = hal::bittorrent::Instance().torrentDetails().focused_torrent())
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
			indices.push_back(numeric_cast<int>((*i).order()));
		}
	}
	
	int priority = nCtrlID-ID_HAL_FILE_PRIORITY_0;
	
	if (hal::bit::torrent t = hal::bittorrent::Instance().get(hal::bittorrent::Instance().torrentDetails().focused_torrent()))
		t.file_priorities = std::pair<std::vector<int>, int>(indices, priority);
	
	if (hal::try_update_lock<this_class_t> lock = hal::try_update_lock<this_class_t>(this)) 
		do_ui_update_();
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
	if (hal::try_update_lock<this_class_t> lock = hal::try_update_lock<this_class_t>(this))
	{		
		determineFocused();
		do_ui_update_();
	}

	return 0;
}

AdvFilesDialog::AdvFilesDialog(HaliteWindow& halWindow) :
	dlg_base_class_t(halWindow),
	treeManager_(tree_),
	ini_class_t("AdvFilesDlg", "settings"),
	splitterPos(150),
	tree_(boost::bind(&AdvFilesDialog::doUiUpdate, this)),
	list_(boost::bind(&AdvFilesDialog::doUiUpdate, this))
{
	load_from_ini();
}

LRESULT AdvFilesDialog::onInitDialog(HWND, LPARAM)
{
	resize_class_t::DlgResize_Init(false, true, WS_CLIPCHILDREN);
	
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
	resize_class_t::DlgResize_UpdateLayout(cxWidth, cyHeight);
	
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

	if (focused_torrent())
	{
		hal::file_details_vec all_files = focused_torrent()->get_file_details();	
		FileListView::scoped_files list_files = list_.files();

		list_files->clear();

		for (std::vector<FileLink>::iterator i=range_.first, e=range_.second;
			i != e; ++i)
		{		
			list_files->push_back(all_files[(*i).order()]);
		}
			
		list_.SetItemCountEx(numeric_cast<int>(list_files->size()),LVSICF_NOSCROLL);
	}

	requestUiUpdate();
}

void AdvFilesDialog::uiUpdate(const hal::torrent_details_manager& tD)
{
	list_.setFocused(focused_torrent());
	
	if (fileLinks_.empty() || !(focused_torrent() && !focused_torrent()->get_file_details().empty())) 
	{
		list_.DeleteAllItems();
		return;
	}
	
	if (hal::try_update_lock<FileListView::list_class_t> lock = hal::try_update_lock<FileListView::list_class_t>(&list_)) 
	{
		hal::file_details_vec all_files = focused_torrent()->get_file_details();	
		FileListView::scoped_files list_files = list_.files();

		if (focused_torrent() && all_files.size() != list_files->size())
		{
			list_files->clear();

			for (std::vector<FileLink>::iterator i=range_.first, e=range_.second;
				i != e; ++i)
			{		
				list_files->push_back(all_files[(*i).order()]);
			}
				
			list_.SetItemCountEx(numeric_cast<int>(list_files->size()),LVSICF_NOSCROLL);
		}

		foreach (hal::file_details& file, *list_files)
		{
			file = all_files[file.order()];
		}

		if (list_.AutoSort() || list_.IsSortOnce())
		{
			int col_sort_index = list_.GetSortColumn();

			if (col_sort_index != -1)
			{		
				if (list_.GetSecondarySortColumn() != -1)
				{
					int index = list_.GetColumnSortType(list_.GetSecondarySortColumn());
					
					if (index > WTL::LVCOLSORT_LAST)
					{
						hal::file_details_sort(*list_files, index - (WTL::LVCOLSORT_LAST+1+hal::file_details::filename_e), 
							list_.IsSecondarySortDescending());
					}
				}

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
	
	{ 	hal::mutex_update_lock<FileTreeView> lock(&tree_);
	
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

	if (focused_torrent())
	{
		hal::file_details_vec all_files = focused_torrent()->get_file_details();	
		FileListView::scoped_files list_files = list_.files();
		list_files->clear();

		for (std::vector<FileLink>::iterator i=range_.first, e=range_.second;
			i != e; ++i)
		{		
			list_files->push_back(all_files[(*i).order()]);
		}
			
		list_.SetItemCountEx(numeric_cast<int>(list_files->size()),LVSICF_NOSCROLL);
	}
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
