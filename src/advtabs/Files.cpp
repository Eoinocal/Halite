
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "../stdAfx.hpp"
#include "../HaliteWindow.hpp"
#include "../HaliteListViewCtrl.hpp"

#include "Files.hpp"

#define TVS_EX_MULTISELECT 0x0002
#define TVS_EX_DOUBLEBUFFER 0x0004
#define TVS_EX_NOINDENTSTATE 0x0008
#define TVS_EX_RICHTOOLTIP 0x0010
#define TVS_EX_AUTOHSCROLL 0x0020
#define TVS_EX_FADEINOUTEXPANDOS 0x0040
#define TVS_EX_PARTIALCHECKBOXES 0x0080
#define TVS_EX_EXCLUSIONCHECKBOXES 0x0100
#define TVS_EX_DIMMEDCHECKBOXES 0x0200
#define TVS_EX_DRAWIMAGEASYNC 0x0400

FileListView::FileListView() :
	iniClass("listviews/advFiles", "FileListView"),
	listClass(true,false,false)
{					
	std::vector<wstring> names;	
	wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);

	// "Path;Filename;Size;Progress;Priority"
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
	
	return hwnd;
}
#if 0
void FileListView::uiUpdate(const hal::TorrentDetails& tD)
{
	TryUpdateLock<listClass> lock(*this);
	if (lock) 
	{		
		peerDetails_.clear();
		
		foreach (const hal::TorrentDetail_ptr torrent, tD.selectedTorrents())
		{
			std::copy(torrent->peerDetails().begin(), torrent->peerDetails().end(), 
				std::back_inserter(peerDetails_));
		}
		
		std::sort(peerDetails_.begin(), peerDetails_.end());
		
		// Wipe details not present
		for(int i = 0; i < GetItemCount(); /*nothing here*/)
		{
			boost::array<wchar_t, MAX_PATH> ip_address;
			GetItemText(i, 0, ip_address.c_array(), MAX_PATH);
			
			hal::PeerDetail ip(ip_address.data());
			hal::PeerDetails::iterator iter = 
				std::lower_bound(peerDetails_.begin(), peerDetails_.end(), ip);
			
			if (iter == peerDetails_.end() || !((*iter) == ip))
			{
				DeleteItem(i);
			}
			else
			{
				SetItemData(i, std::distance(peerDetails_.begin(), iter));
				++i;
			}
		}
		
		// Add additional details
		for (hal::PeerDetails::iterator i=peerDetails_.begin(), e=peerDetails_.end();
			i != e; ++i)
		{			
			LV_FINDINFO findInfo; 
			findInfo.flags = LVFI_STRING;
			findInfo.psz = const_cast<LPTSTR>((*i).ipAddress.c_str());
			
			int itemPos = FindItem(&findInfo, -1);
			if (itemPos < 0)
				itemPos = AddItem(GetItemCount(), 0, (*i).ipAddress.c_str(), 0);
			
			SetItemData(itemPos, std::distance(peerDetails_.begin(), i));
			
			SetItemText(itemPos, 1, (*i).country.c_str());
			
			SetItemText(itemPos, 2, getColumnAdapter(2)->print(*i).c_str());
			
			SetItemText(itemPos, 3, getColumnAdapter(3)->print(*i).c_str());
			
			if ((*i).seed)
				SetItemText(itemPos, 4, L"Seed");
			
			SetItemText(itemPos, 5, (*i).client.c_str());
			
			SetItemText(itemPos, 6, (*i).status.c_str());
		}
		
		ConditionallyDoAutoSort();
	}
}
#endif

LRESULT FileTreeView::OnSelChanged(int, LPNMHDR pnmh, BOOL&)
{	
	TryUpdateLock<thisClass> lock(*this);
	if (lock)
	{
		CTreeItem ti = GetSelectedItem();	
		boost::array<wchar_t, MAX_PATH> buffer;
		
		if (ti)
		{
			ti.GetText(buffer.elems, MAX_PATH);
			wpath branch(wstring(buffer.elems));
			
			while (ti = ti.GetParent())
			{
				ti.GetText(buffer.elems, MAX_PATH);
				branch = wstring(buffer.elems)/branch;
			}		
		//	MessageBox(branch.string().c_str(),L"Hi",0);
		}
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
		WS_EX_STATICEDGE);
		
	tree_.Create(splitter_, rc, NULL, 
		WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|TVS_HASBUTTONS|
			TVS_HASLINES|TVS_TRACKSELECT|TVS_SHOWSELALWAYS,
		TVS_EX_DOUBLEBUFFER|WS_EX_STATICEDGE);
		
	splitter_.SetSplitterPanes(tree_, list_);
	splitter_.SetSplitterPos(splitterPos);
	
	CTreeItem ti = tree_.InsertItem(L"baz", TVI_ROOT, TVI_LAST);
	
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

void AdvFilesDialog::uiUpdate(const hal::TorrentDetails& tD)
{
	fileDetails_.clear();
	
	foreach (const hal::TorrentDetail_ptr torrent, tD.selectedTorrents())
	{
		std::copy(torrent->fileDetails().begin(), torrent->fileDetails().end(), 
			std::back_inserter(fileDetails_));
	}
	
	std::sort(fileDetails_.begin(), fileDetails_.end());
	
	{ 	UpdateLock<FileTreeView> lock(tree_);
	
		treeManager_.InvalidateAll();
		
		foreach (hal::FileDetail file, fileDetails_)
		{
			treeManager_.EnsureValid(file.branch);
		}
		
		treeManager_.ClearInvalid();
	}
	
	//std::pair<hal::FileDetails::iterator, hal::FileDetails::iterator> range =
	//	std::equal_range(fileDetails_.begin(), fileDetails_.end(), FileDetail(
	
	TryUpdateLock<FileListView::listClass> lock(list_);
	if (lock) 
	{			
		// Wipe details not present
		for(int i = 0; i < list_.GetItemCount(); /*nothing here*/)
		{
			boost::array<wchar_t, MAX_PATH> fullPath;
			list_.GetItemText(i, 0, fullPath.c_array(), MAX_PATH);
			
			hal::FileDetail file(wstring(fullPath.data()));
			hal::FileDetails::iterator iter = 
				std::lower_bound(fileDetails_.begin(), fileDetails_.end(), file);
			
			if (iter == fileDetails_.end() || !((*iter) == file))
			{
				list_.DeleteItem(i);
			}
			else
			{
				list_.SetItemData(i, std::distance(fileDetails_.begin(), iter));
				++i;
			}
		}
		
		// Add additional details
		for (hal::FileDetails::iterator i=fileDetails_.begin(), e=fileDetails_.end();
			i != e; ++i)
		{			
			LV_FINDINFO findInfo; 
			findInfo.flags = LVFI_STRING;
			findInfo.psz = const_cast<LPTSTR>((*i).filename.c_str());
			
			int itemPos = list_.FindItem(&findInfo, -1);
			if (itemPos < 0)
				itemPos = list_.AddItem(list_.GetItemCount(), 0, (*i).filename.c_str(), 0);
			
		//	list_.SetItemData(itemPos, std::distance(peerDetails_.begin(), i));
			
			list_.SetItemText(itemPos, 1, (*i).filename.c_str());
			list_.SetItemText(itemPos, 2, (*i).branch.string().c_str());
			
		//	list_.SetItemText(itemPos, 2, getColumnAdapter(2)->print(*i).c_str());
			
		//	list_.SetItemText(itemPos, 3, getColumnAdapter(3)->print(*i).c_str());
			
		}

		list_.ConditionallyDoAutoSort();
	}
	
	splitterPos = splitter_.GetSplitterPos();
}

void AdvFilesDialog::onClose()
{		
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}
