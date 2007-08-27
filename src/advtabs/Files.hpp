
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
		iniClass("treeviews/advFiles", "FileTreeView")
	{}
	
	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_DESTROY(OnDestroy)

		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
protected:
	void OnDestroy()
	{
	//	saveSettings();
	}
};

class AdvFilesDialog :
	public CHalTabPageImpl<AdvFilesDialog>,
	public CDialogResize<AdvFilesDialog>,
	public CHaliteDialogBase<AdvFilesDialog>,
	public CWinDataExchangeEx<AdvFilesDialog>,
	private boost::noncopyable
{
protected:
	typedef AdvFilesDialog thisClass;
	typedef CHalTabPageImpl<thisClass> baseClass;
	typedef CDialogResize<thisClass> resizeClass;
	typedef CHaliteDialogBase<thisClass> dialogBaseClass;

public:
	enum { IDD = IDD_ADVFILES };

	AdvFilesDialog(HaliteWindow& halWindow) :
		dialogBaseClass(halWindow)
	{}
	
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

	LRESULT onInitDialog(HWND, LPARAM);
	void onClose();
	
	void DlgResize_UpdateLayout(int cxWidth, int cyHeight);
	void uiUpdate(const hal::TorrentDetails& tD);

protected:
	CSplitterWindow splitter_;
	FileTreeView tree_;
	FileListView list_;
	
	hal::FileDetails fileDetails_;
};
