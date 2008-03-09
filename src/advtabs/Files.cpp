
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
	iniClass("listviews/advFiles", "FileListView"),
	listClass(true,false,false)
{					
	std::vector<wstring> names;	
	wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);

	// "Filename;Path;Size;Progress;Priority"
	boost::split(names, column_names, boost::is_any_of(L";"));
	
	array<int, 5> widths = {100,70,70,70,70};
	array<int, 5> order = {0,1,2,3,4};
	array<bool, 5> visible = {true,true,true,true,true};
	
	SetDefaults(names, widths, order, visible, true);
	Load();
}

HWND FileListView::Create(HWND hWndParent, ATL::_U_RECT rect, LPCTSTR szWindowName,
	DWORD dwStyle, DWORD dwExStyle,
	ATL::_U_MENUorID MenuOrID, LPVOID lpCreateParam)
{
	HWND hwnd = listClass::Create(hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	assert(hwnd);
	
	ApplyDetails();
	
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_HEADERDRAGDROP|LVS_EX_DOUBLEBUFFER);
	
	SetColumnSortType(2, LVCOLSORT_CUSTOM, new ColumnAdapters::Size());
	SetColumnSortType(3, LVCOLSORT_CUSTOM, new ColumnAdapters::Progress());
	SetColumnSortType(4, LVCOLSORT_CUSTOM, new ColumnAdapters::Priority());
	
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
	
	std::string torrent = hal::to_utf8(hal::bittorrent().torrentDetails().focusedTorrent()->name());
	hal::bittorrent().setTorrentFilePriorities(torrent, indices, priority);
}

HWND FileTreeView::Create(HWND hWndParent, ATL::_U_RECT rect, LPCTSTR szWindowName, DWORD dwStyle, DWORD dwExStyle,
	ATL::_U_MENUorID MenuOrID, LPVOID lpCreateParam)
{
	HWND hwnd = treeClass::Create(hWndParent, (RECT &)rect.m_lpRect, szWindowName, dwStyle, dwExStyle, (UINT)MenuOrID.m_hMenu, lpCreateParam);
	assert(hwnd);
	
	CMenuHandle menu;
	BOOL menu_created = menu.LoadMenu(IDR_FILESLISTVIEW_MENU);
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
		assert (menu_.IsMenu());

		POINT ptPoint;
		GetCursorPos(&ptPoint);
		menu_.TrackPopupMenu(0, ptPoint.x, ptPoint.y, m_hWnd);
	}

	return 0;
}

void FileTreeView::OnMenuPriority(UINT uCode, int nCtrlID, HWND hwndCtrl)
{	
	hal::FileDetails fileDetails;
	
	if (hal::TorrentDetail_ptr torrent = hal::bittorrent().torrentDetails().focusedTorrent())
	{
		std::copy(torrent->fileDetails().begin(), torrent->fileDetails().end(), 
			std::back_inserter(fileDetails));
	}

	wpath branch;
	
	if (CTreeItem ti = GetSelectedItem())
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
	
	for (hal::FileDetails::iterator i=fileDetails.begin(), e=fileDetails.end();
		i != e; ++i)
	{			
		if (std::equal(branch.begin(), branch.end(), (*i).branch.begin()))
		{
			indices.push_back((*i).order());
		}
	}
	
	int priority = nCtrlID-ID_HAL_FILE_PRIORITY_0;
	
	std::string torrent = hal::to_utf8(hal::bittorrent().torrentDetails().focusedTorrent()->name());
	hal::bittorrent().setTorrentFilePriorities(torrent, indices, priority);
	
	TryUpdateLock<thisClass> lock(*this);
	if (lock) signal();
}

void FileTreeView::determineFocused()
{
	wpath branch;
	
	if (CTreeItem ti = GetSelectedItem())
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
	TryUpdateLock<thisClass> lock(*this);
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
	
	CRect rc; GetClientRect(&rc);
	
	static_.SubclassWindow(GetDlgItem(IDC_CONTAINER));
	
	splitter_.Create(GetDlgItem(IDC_CONTAINER), rc, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
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
	
	CTreeItem ti = tree_.InsertItem(hal::app().res_wstr(HAL_TORRENT_ROOT).c_str(), TVI_ROOT, TVI_LAST);
	
//	DoDataExchange(false);
	
	return 0;
}

void AdvFilesDialog::DlgResize_UpdateLayout(int cxWidth, int cyHeight)
{
	resizeClass::DlgResize_UpdateLayout(cxWidth, cyHeight);
	
	CRect rect; ::GetClientRect(GetDlgItem(IDC_CONTAINER), &rect);
	
	splitter_.SetWindowPos(NULL, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		SWP_NOZORDER | SWP_NOACTIVATE);
}

void AdvFilesDialog::doUiUpdate()
{
//	hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::Event::info, (wformat(L"doUiUpdate %1%") % current_torrent_name_).str().c_str())));

	tree_.determineFocused();
	
	range_ = std::equal_range(fileLinks_.begin(), fileLinks_.end(),
		FileLink(tree_.focused()));
	
	std::sort(range_.first, range_.second, &FileLinkNamesLess);
	
	requestUiUpdate();
}

void AdvFilesDialog::uiUpdate(const hal::TorrentDetails& tD)
{
	list_.setFocused(focusedTorrent());
	
	if (fileLinks_.empty() || !(focusedTorrent() && !focusedTorrent()->fileDetails().empty())) 
	{
		list_.DeleteAllItems();
		return;
	}
	
	TryUpdateLock<FileListView::listClass> lock(list_);
	if (lock) 
	{			
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
		
		// Add additional details
		for (std::vector<FileLink>::iterator i=range_.first, e=range_.second;
			i != e; ++i)
		{
			hal::FileDetail fileD = focusedTorrent()->fileDetails()[(*i).order()];
			
			LV_FINDINFO findInfo; 
			findInfo.flags = LVFI_STRING;
			findInfo.psz = const_cast<LPTSTR>((*i).filename.c_str());
			
			int itemPos = list_.FindItem(&findInfo, -1);
			if (itemPos < 0)
				itemPos = list_.AddItem(list_.GetItemCount(), 0, (*i).filename.c_str(), 0);
			
			list_.SetItemData(itemPos, (*i).order());
			
			list_.SetItemText(itemPos, 1, (*i).branch.string().c_str());			
			list_.SetItemText(itemPos, 2, list_.getColumnAdapter(2)->print(fileD).c_str());
			list_.SetItemText(itemPos, 3, list_.getColumnAdapter(3)->print(fileD).c_str());
			list_.SetItemText(itemPos, 4, list_.getColumnAdapter(4)->print(fileD).c_str());			
		}

		list_.ConditionallyDoAutoSort();
	}
}

void AdvFilesDialog::focusChanged(const hal::TorrentDetail_ptr pT)
{
	fileLinks_.clear();
	if (pT)
	{
		std::copy(pT->fileDetails().begin(), pT->fileDetails().end(), 
			std::back_inserter(fileLinks_));
	}
	
	list_.setFocused(pT);
	
	std::sort(fileLinks_.begin(), fileLinks_.end());
	
	{ 	UpdateLock<FileTreeView> lock(tree_);
	
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
	Save(); 
}
