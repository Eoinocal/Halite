
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdAfx.hpp"
#include "Halite.hpp"

#include "DdxEx.hpp"
#include "global/string_conv.hpp"

#include "../HaliteTabPage.hpp"
#include "../HaliteListManager.hpp"
#include "../HaliteDialogBase.hpp"
#include "../halIni.hpp"
#include "../HaliteListViewCtrl.hpp"

#include "../WTLx/UpdateLockable.hpp"

struct FileLink
{
	FileLink(const boost::filesystem::wpath& b, size_t o=0) :
		branch(b),
		order_(o)
	{}

	FileLink(const wstring& f, size_t o=0) :
		filename(f),
		order_(o)
	{}

	FileLink(const hal::file_details& f) :
		branch(f.branch),
		filename(f.filename.wstring()),
		order_(f.order())
	{}
	
	bool operator==(const FileLink& f) const
	{
		return (branch == f.branch);
	}
	
	bool operator<(const FileLink& f) const
	{
		return (branch < f.branch);
	}
	
	enum FileType
	{
		folder,
		file
	};
	
	size_t order() { return order_; }
	
	boost::filesystem::path branch;
	wstring filename;
	unsigned type;
	
private:
	size_t order_;
};


inline bool FileLinkNamesEqual(const FileLink& l, const FileLink& r)
{
	return l.filename == r.filename;
}

inline bool FileLinkNamesLess(const FileLink& l, const FileLink& r)
{
	return l.filename < r.filename;
}

class FileListView :
	public CHaliteSortListViewCtrl<FileListView, const hal::file_details>,
	public hal::IniBase<FileListView>,
	private boost::noncopyable
{
public:
	typedef FileListView this_class_t;
	typedef const hal::file_details data_class_t;
	typedef CHaliteSortListViewCtrl<this_class_t, data_class_t> list_class_t;
	typedef hal::IniBase<this_class_t> ini_class_t;
	
	typedef boost::function<void ()> do_ui_update_fn;

	friend class list_class_t;

public:	
	class scoped_files
	{

	public:
		scoped_files(hal::mutex_t& m, hal::file_details_vec* f) :
			l_(m),
			f_(f)
		{}

		hal::file_details_vec* operator->() const { return f_; }

		hal::file_details_vec& operator*() const { return *f_; }

	private:
		hal::mutex_t::scoped_lock l_;
		hal::file_details_vec* f_;
	};

	enum { 
		LISTVIEW_ID_MENU = HAL_FILESLISTVIEW_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_DIALOGFILE_LISTVIEW_ADV,
		LISTVIEW_ID_COLUMNWIDTHS = 0
	};
	
	BEGIN_MSG_MAP_EX(this_class_t)
		try
		{
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_RANGE_HANDLER_EX(ID_HAL_FILE_PRIORITY_0, ID_HAL_FILE_PRIORITY_7, OnMenuPriority)

		REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
		REFLECTED_NOTIFY_CODE_HANDLER(SLVN_SORTCHANGED, OnSortChanged)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in FileListView MSG_MAP")

		CHAIN_MSG_MAP(list_class_t)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	FileListView(do_ui_update_fn uiu, HWND hwnd);
	
	void saveSettings()
	{
		GetListViewDetails();
		save_to_ini();
	}
	
	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0,
		ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL);
	
	void OnDestroy()
	{
		saveSettings();
	}
	
	void OnMenuPriority(UINT, int, HWND);
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 2:			
			ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<list_class_t>(*this));
		
		break;

		case 1:
		default:
			assert(false);
		}
	}
	
	void setFocused(const hal::torrent_details_ptr& f) { focused_ = f; }
	const hal::torrent_details_ptr focused() { return focused_; }

	scoped_files files() { return scoped_files(list_class_t::mutex_, &files_); }

protected:	
	LRESULT OnGetDispInfo(int, LPNMHDR pnmh, BOOL&);
	LRESULT OnSortChanged(int, LPNMHDR pnmh, BOOL&);

private:
	boost::shared_ptr<hal::try_update_lock<list_class_t> > lock_ptr_;
	WTL::CCursorHandle cursor_;

	do_ui_update_fn do_ui_update_;
	hal::file_details_vec files_;
	hal::torrent_details_ptr focused_;
	HWND treeview_;
};

class FileTreeView :
	public ATL::CWindowImpl<FileTreeView, WTL::CTreeViewCtrlEx>,
	public hal::IniBase<FileTreeView>,
	public hal::update_lockable<FileTreeView>,
	private boost::noncopyable
{
protected:
	typedef FileTreeView this_class_t;
	typedef ATL::CWindowImpl<this_class_t, WTL::CTreeViewCtrlEx> treeClass;
	typedef hal::IniBase<this_class_t> ini_class_t;
	
	typedef boost::function<void ()> do_ui_update_fn;

	friend class treeClass;
	
public:	
	FileTreeView(do_ui_update_fn uiu) :
		ini_class_t(L"treeviews/adv_files", L"file_treeview"),
		do_ui_update_(uiu)
	{}
	
	BEGIN_MSG_MAP_EX(this_class_t)
		try
		{
		MSG_WM_DESTROY(OnDestroy)

		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRClick)
		REFLECTED_NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnSelChanged)

		COMMAND_RANGE_HANDLER_EX(ID_HAL_FILE_PRIORITY_0, ID_HAL_FILE_PRIORITY_7, OnMenuPriority)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in FileTreeView MSG_MAP")
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0,
		ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL);
	
	LRESULT OnRClick(int i, LPNMHDR pnmh, BOOL&);
	void OnMenuPriority(UINT, int, HWND);
	
	wpath focused() { return focused_; }		
	void determineFocused();
	
protected:
	void OnDestroy()
	{
	//	saveSettings();
	}
	
	LRESULT OnSelChanged(int, LPNMHDR pnmh, BOOL&);
	
	mutable hal::mutex_t mutex_;
	boost::shared_ptr<hal::try_update_lock<this_class_t> > lock_ptr_;
	
	wpath focused_;

private:
	do_ui_update_fn do_ui_update_;
	WTL::CMenu menu_;
};

template<typename T>
class TreeViewManager
{
public:
	TreeViewManager(T& t) :
		tree_(t)
	{}
	
	struct ValidTreeItem
	{
		ValidTreeItem() 
		{}

		ValidTreeItem(WTL::CTreeItem& t) :
			valid(true),
			treeItem(t)
		{}
		
		bool valid;
		WTL::CTreeItem treeItem;
	};
	
	typedef std::map<wpath, ValidTreeItem> MapType;
	
	void EnsureValid(wpath p)
	{		
		wpath branchPath = p;
		
		MapType::iterator i = map_.find(branchPath);		
		if (i == map_.end())
		{
			WTL::CTreeItem ti = tree_.GetRootItem();
			
			wpath branch;
			BOOST_FOREACH (wpath b, branchPath)
			{
				branch /= b;				
				MapType::iterator j = map_.find(branch);
				
				if (j == map_.end())
				{
					WTL::CTreeItem tmp = ti.AddTail(b.c_str(), -1);
					ti.Expand();
					ti = tmp;
					map_[branch] = ValidTreeItem(ti);
				}
				else
				{
					(*j).second.valid = true;
					ti = (*j).second.treeItem;
				}				
			}
		}
		else
		{
			if (!(*i).second.valid)
			{
				(*i).second.valid = true;
				EnsureValid(branchPath.parent_path());
			}
		}
	}
	
	void InvalidateAll()
	{
		for(MapType::iterator i=map_.begin(), e=map_.end(); i!=e; ++i)
		{
			(*i).second.valid = false;
		}
	}
	
	void ClearInvalid()
	{
		for(MapType::iterator i=map_.begin(), e=map_.end(); i!=e; /**/)
		{
			if ((*i).second.valid)
			{
				++i;
			}
			else
			{
				(*i).second.treeItem.Delete();
				map_.erase(i++);
			}
		}	
	}
	
private:
	T& tree_;
	MapType map_;
};

class FileStatic :
	public ATL::CWindowImpl<FileStatic, WTL::CStatic>
{	
public:
	BEGIN_MSG_MAP_EX(FileStatic)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
};

class AdvFilesDialog :
	public CHalTabPageImpl<AdvFilesDialog>,
	public WTL::CDialogResize<AdvFilesDialog>,
	public CHaliteDialogBase<AdvFilesDialog>,
	public WTLx::WinDataExchangeEx<AdvFilesDialog>,
	public hal::IniBase<AdvFilesDialog>,
	private boost::noncopyable
{
protected:
	typedef AdvFilesDialog this_class_t;
	typedef CHalTabPageImpl<this_class_t> base_class_t;
	typedef WTL::CDialogResize<this_class_t> resize_class_t;
	typedef CHaliteDialogBase<this_class_t> dlg_base_class_t;
	typedef hal::IniBase<this_class_t> ini_class_t;

public:
	enum { IDD = HAL_ADVFILES };

	AdvFilesDialog(HaliteWindow& halWindow);
	
	~AdvFilesDialog() {}
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(this_class_t)
		try
		{
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)
		MSG_WM_DESTROY(OnDestroy)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in AdvFilesDialog MSG_MAP")
		
		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;
		
		CHAIN_MSG_MAP(dlg_base_class_t)
		CHAIN_MSG_MAP(resize_class_t)
		CHAIN_MSG_MAP(base_class_t)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(this_class_t)
		DLGRESIZE_CONTROL(HAL_CONTAINER, DLSZ_SIZE_X|DLSZ_SIZE_Y|DLSZ_REPAINT)
	END_DLGRESIZE_MAP()

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 2:			
			ar & make_nvp("splitter_position", splitterPos);
			
		break;

		case 1:
		default:
			assert(false);
		}
	}

	LRESULT onInitDialog(HWND, LPARAM);
	void onClose();
	LRESULT OnGetDispInfo(int, LPNMHDR pnmh, BOOL&);
	
	void DlgResize_UpdateLayout(int cxWidth, int cyHeight);
	void doUiUpdate();
	void uiUpdate(const hal::torrent_details_manager& tD);
	void focusChanged(const hal::torrent_details_ptr pT);
	void OnDestroy();

protected:
	WTL::CSplitterWindow splitter_;
	unsigned int splitterPos;
	
	FileStatic static_;
	FileTreeView tree_;
	FileListView list_;
	
	std::map<wpath, WTL::CTreeItem> fileTreeMap_;
	TreeViewManager<FileTreeView> treeManager_;
	
	std::vector<FileLink> fileLinks_;
	
	hal::file_details_vec file_details_;
	std::pair<std::vector<FileLink>::iterator, std::vector<FileLink>::iterator> range_;
};

BOOST_CLASS_VERSION(FileListView, 2)
BOOST_CLASS_VERSION(AdvFilesDialog, 2)
