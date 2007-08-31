
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../stdAfx.hpp"
#include "../global/string_conv.hpp"

#include "../DdxEx.hpp"
#include "../Halite.hpp"

#include "../HaliteTabPage.hpp"
#include "../HaliteListManager.hpp"
#include "../HaliteDialogBase.hpp"
#include "../HaliteIni.hpp"
#include "../HaliteListViewCtrl.hpp"

#include "../HaliteUpdateLock.hpp"

class FileListView :
	public CHaliteSortListViewCtrl<FileListView>,
	public CHaliteIni<FileListView>,
	private boost::noncopyable
{
public:
	typedef FileListView thisClass;
	typedef CHaliteSortListViewCtrl<thisClass> listClass;
	typedef CHaliteIni<thisClass> iniClass;

	friend class listClass;
	
	struct ColumnAdapters
	{	
	typedef listClass::ColumnAdapter ColAdapter_t;	
	};

public:	
	enum { 
		LISTVIEW_ID_MENU = 0,
		LISTVIEW_ID_COLUMNNAMES = HAL_DIALOGFILE_LISTVIEW_ADV,
		LISTVIEW_ID_COLUMNWIDTHS = 0
	};
	
	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_DESTROY(OnDestroy)

		CHAIN_MSG_MAP(listClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	FileListView();
	
	void saveSettings()
	{
		GetListViewDetails();
		Save();
	}
	
	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0,
		ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL);
	
	bool SubclassWindow(HWND hwnd)
	{
		if(!listClass::SubclassWindow(hwnd))
			return false;
		
		ApplyDetails();
		
//		SetColumnSortType(2, LVCOLSORT_CUSTOM, new ColumnAdapters::SpeedDown());
		
		return true;
	}
	
	void OnDestroy()
	{
		saveSettings();
	}
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & boost::serialization::make_nvp("listview", 
			boost::serialization::base_object<listClass>(*this));
	}
	
	void* CustomItemConversion(LVCompareParam* param, int iSortCol)
	{			
		return 0;
	}		
	
	int CustomItemComparision(void* left, void* right, int iSortCol)
	{
		return 0;
	}
};

class FileTreeView :
	public CWindowImpl<FileTreeView, CTreeViewCtrlEx>,
	public CHaliteIni<FileTreeView>,
	private boost::noncopyable
{
protected:
	typedef FileTreeView thisClass;
	typedef CWindowImpl<thisClass, CTreeViewCtrlEx> treeClass;
	typedef CHaliteIni<thisClass> iniClass;

	friend class treeClass;
	
public:	
	thisClass() :
		iniClass("treeviews/advFiles", "FileTreeView"),
		updateLock_(0)
	{}
	
	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_DESTROY(OnDestroy)

		REFLECTED_NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnSelChanged)
//		CHAIN_MSG_MAP(treeClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	wpath focused() { return focused_; }
	
	void attach(boost::function<void ()> fn) const { selection_.connect(fn); }
	
	void signal() { selection_(); }
	
protected:
	void OnDestroy()
	{
	//	saveSettings();
	}
	
	LRESULT OnSelChanged(int, LPNMHDR pnmh, BOOL&);
	
	int updateLock_;
	friend class UpdateLock<thisClass>;	
	friend class TryUpdateLock<thisClass>;	
	
	mutable boost::signal<void ()> selection_;
	wpath focused_;
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

		ValidTreeItem(CTreeItem& t) :
			valid(true),
			treeItem(t)
		{}
		
		bool valid;
		CTreeItem treeItem;
	};
	
	typedef std::map<wpath, ValidTreeItem> MapType;
	
	void EnsureValid(wpath p)
	{		
		wpath branchPath = p;
		
		MapType::iterator i = map_.find(branchPath);		
		if (i == map_.end())
		{
			CTreeItem ti = tree_.GetRootItem();
			
			wpath branch;
			foreach (wstring b, branchPath)
			{
				branch /= b;				
				MapType::iterator j = map_.find(branch);
				
				if (j == map_.end())
				{
					CTreeItem tmp = ti.AddTail(b.c_str(), -1);
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
				EnsureValid(branchPath.branch_path());
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
		for(MapType::reverse_iterator i=map_.rbegin(), e=map_.rend(); i!=e; /**/)
		{
			hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::Event::info, (wformat(L"Aalid %1% -- %2%, %3%") % (*i).second.valid % (*i).first.string() % map_.size()).str().c_str())));

			if ((*i).second.valid)
			{
				++i;
			}
			else
			{
			boost::array<wchar_t, MAX_PATH> buffer;
			(*i).second.treeItem.GetText(buffer.elems, MAX_PATH);
			wstring tmp(buffer.elems);
			
				hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::Event::info, (wformat(L"Valid %1% -- %2% -- %3%, %4%") % (*i).second.valid % tmp % (*i).first.string() % map_.size()).str().c_str())));

				MapType::reverse_iterator j = i;
				++j;
				(*i).second.treeItem.Delete();
				map_.erase(i.base());
				if (j == e) break;
				i = j;
				hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::Event::info, (wformat(L"Zalid %1% -- %2%, %3%") % (*i).second.valid % (*i).first.string() % map_.size()).str().c_str())));
			}
		}		
	}
	
private:
	T& tree_;
	MapType map_;
};

class FileStatic :
	public CWindowImpl<FileStatic, CStatic>
{	
public:
	BEGIN_MSG_MAP_EX(FileStatic)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
};

class AdvFilesDialog :
	public CHalTabPageImpl<AdvFilesDialog>,
	public CDialogResize<AdvFilesDialog>,
	public CHaliteDialogBase<AdvFilesDialog>,
	public CWinDataExchangeEx<AdvFilesDialog>,
	public CHaliteIni<AdvFilesDialog>,
	private boost::noncopyable
{
protected:
	typedef AdvFilesDialog thisClass;
	typedef CHalTabPageImpl<thisClass> baseClass;
	typedef CDialogResize<thisClass> resizeClass;
	typedef CHaliteDialogBase<thisClass> dialogBaseClass;
	typedef CHaliteIni<thisClass> iniClass;

public:
	enum { IDD = IDD_ADVFILES };

	AdvFilesDialog(HaliteWindow& halWindow) :
		dialogBaseClass(halWindow),
		treeManager_(tree_),
		iniClass("AdvFiles", "AdvFiles"),
		splitterPos(50)
	{
		Load();
	}
	
	~AdvFilesDialog() { Save(); }
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)
		
		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;
		
		CHAIN_MSG_MAP(resizeClass)
		CHAIN_MSG_MAP(baseClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(IDC_CONTAINER, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(splitterPos);
	}

	LRESULT onInitDialog(HWND, LPARAM);
	void onClose();
	
	void DlgResize_UpdateLayout(int cxWidth, int cyHeight);
	void doUiUpdate();
	void uiUpdate(const hal::TorrentDetails& tD);

protected:
	CSplitterWindow splitter_;
	unsigned int splitterPos;
	
	FileStatic static_;
	FileTreeView tree_;
	FileListView list_;
	
	//hal::FileDetails fileDetails_;
	std::map<wpath, CTreeItem> fileTreeMap_;
	TreeViewManager<FileTreeView> treeManager_;
};
