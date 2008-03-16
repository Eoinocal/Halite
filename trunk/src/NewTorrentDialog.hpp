
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define ID_NEWTORRENT_BEGIN				14000
#define IDC_NEWTORRENT_SELECT_TEXT		ID_NEWTORRENT_BEGIN+1
#define IDC_NEWTORRENT_TRACKERS_TEXT	ID_NEWTORRENT_BEGIN+2
#define IDC_NEWTORRENT_CREATOR_TEXT		ID_NEWTORRENT_BEGIN+3
#define IDC_NEWTORRENT_CREATOR			ID_NEWTORRENT_BEGIN+4
#define IDC_NEWTORRENT_COMMENT_TEXT		ID_NEWTORRENT_BEGIN+5
#define IDC_NEWTORRENT_COMMENT			ID_NEWTORRENT_BEGIN+6
#define IDC_NEWTORRENT_PRIVATE			ID_NEWTORRENT_BEGIN+7
#define IDC_NEWT_FILE_BROWSE			ID_NEWTORRENT_BEGIN+8
#define IDC_NEWT_DIR_BROWSE				ID_NEWTORRENT_BEGIN+9
#define IDC_NEWT_LISTFILES				ID_NEWTORRENT_BEGIN+10
#define HAL_FILES_LISTVIEW_ADV			ID_NEWTORRENT_BEGIN+11
#define HAL_NEW_PANE_DLG				ID_NEWTORRENT_BEGIN+12
//#define IDC_PROG_CANCEL                 ID_NEWTORRENT_BEGIN + 2
//#define IDC_PROG_PROGRESS               ID_NEWTORRENT_BEGIN + 3

#ifndef RC_INVOKED

#include <boost/function.hpp>
#include "halTorrent.hpp"
#include "halIni.hpp"
#include "halEvent.hpp"
#include "DdxEx.hpp"
#include "ListViewEdit.hpp"
#include "HaliteSortListViewCtrl.hpp"


class FilesListViewCtrl :
	public CHaliteSortListViewCtrl<FilesListViewCtrl>,
	public hal::IniBase<FilesListViewCtrl>,
	private boost::noncopyable
{

	typedef hal::IniBase<FilesListViewCtrl> iniClass;
	typedef CHaliteSortListViewCtrl<FilesListViewCtrl> listClass;

	friend class listClass;
	
public:
	enum { 
		LISTVIEW_ID_MENU = 0,
		LISTVIEW_ID_COLUMNNAMES = HAL_FILES_LISTVIEW_ADV,
		LISTVIEW_ID_COLUMNWIDTHS = 0
	};
	
	FilesListViewCtrl() :
		listClass(true,false,false),
		iniClass("listviews/new_files", "NewFilesListView")
	{
		std::vector<wstring> names;	
		wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);

		// "Tracker;Tier"
		boost::split(names, column_names, boost::is_any_of(L";"));
		
		array<int, 3> widths = {50,287,50};
		array<int, 3> order = {0,1,2};
		array<bool, 3> visible = {true,true,true};
		
		SetDefaults(names, widths, order, visible, true);
		Load();
	}

	BEGIN_MSG_MAP_EX(FilesListViewCtrl)
		MSG_WM_DESTROY(OnDestroy)

/*		COMMAND_ID_HANDLER(ID_TLVM_NEW, OnNew)
		COMMAND_ID_HANDLER(ID_TLVM_EDIT, OnEdit)
		COMMAND_ID_HANDLER(ID_TLVM_DELETE, OnDelete)
		COMMAND_ID_HANDLER(ID_TLVM_PRIMARY, OnPrimary)

		REFLECTED_NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDoubleClick)
*/
		CHAIN_MSG_MAP(listClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void uiUpdate(const hal::TorrentDetail_ptr pT);
//	void enterNewTracker();
//	void saveSettings();

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<listClass>(*this));
    }

private:
	void OnAttach();
	void OnDestroy();
	void saveSettings();

	boost::signal<void ()> listEdited_;
};

typedef FilesListViewCtrl::SelectionManager FilesListViewManager;

class FileSheet :
    public CPropertyPageImpl<FileSheet>,
    public CWinDataExchangeEx<FileSheet>,
	public CAutoSizeWindow<FileSheet, false>
{
protected:
	typedef FileSheet thisClass;
	typedef CPropertyPageImpl<thisClass> sheetClass;
	typedef CAutoSizeWindow<thisClass, false> autosizeClass;

public:
	FileSheet()
	{}

	~FileSheet()
	{}
	
	enum { IDD = IDD_NEWTORRENT };

    BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER_EX(IDC_NEWT_FILE_BROWSE, OnFileBrowse)
		COMMAND_ID_HANDLER_EX(IDC_NEWT_DIR_BROWSE, OnDirBrowse)

		CHAIN_MSG_MAP(autosizeClass)
		CHAIN_MSG_MAP(sheetClass)
		REFLECT_NOTIFICATIONS()
    END_MSG_MAP()
	
    BEGIN_DDX_MAP(thisClass)
		DDX_EX_STDWSTRING(IDC_NEWTORRENT_CREATOR, creator_);
		DDX_EX_STDWSTRING(IDC_NEWTORRENT_COMMENT, comment_);
        DDX_CHECK(IDC_NEWTORRENT_PRIVATE, private_)
    END_DDX_MAP()

	static CWindowMapStruct* GetWindowMap();
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
	
	void OnFileBrowse(UINT, int, HWND hWnd);
	void OnDirBrowse(UINT, int, HWND hWnd);
	LRESULT onInitDialog(HWND, LPARAM);
	void OnDestroy() {};
	
private:
	FilesListViewCtrl filesList_;
	
	wpath fileRoot_;
	std::vector<wpath> files_;

	wstring creator_;
	wstring comment_;
	bool private_;

};

class TrackerSheet :
    public CPropertyPageImpl<TrackerSheet>,
	public CAutoSizeWindow<TrackerSheet, false>
{
protected:
	typedef TrackerSheet thisClass;
	typedef CPropertyPageImpl<thisClass> sheetClass;
	typedef CAutoSizeWindow<thisClass, false> autosizeClass;

public:	
	enum { IDD = IDD_NEWT_TRACKERS };

    BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_DESTROY(OnDestroy)

//		COMMAND_ID_HANDLER_EX(IDC_NEWT_ADDTRACKER, OnFileBrowse)
//		COMMAND_ID_HANDLER_EX(IDC_NEWT_DIR_BROWSE, OnDirBrowse)

		REFLECT_NOTIFICATIONS()
		CHAIN_MSG_MAP(autosizeClass)
		CHAIN_MSG_MAP(sheetClass)
    END_MSG_MAP()

	static CWindowMapStruct* GetWindowMap();
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
	
	LRESULT onInitDialog(HWND, LPARAM)
	{	
		editList_.SubclassWindow(GetDlgItem(IDC_NEWT_LISTTRACKERS));	

		editList_.AddColumn(L"Col 1", 0);
		editList_.AddColumn(L"Col 2", 1);

		int itemPos = editList_.AddItem(0, 0, L"Yoke 1", 0);
		editList_.SetItemText(itemPos, 1, L"Thingy 2");

		itemPos = editList_.AddItem(0, 0, L"Stuff 3", 0);
		editList_.SetItemText(itemPos, 1, L"Empty 4");

		return 0;
	}

	void OnDestroy() {}
	
private:
	WTLx::CEditListViewCtrl editList_;
};

class DetailsSheet :
    public CPropertyPageImpl<DetailsSheet>,
	public CAutoSizeWindow<DetailsSheet, false>
{
protected:
	typedef DetailsSheet thisClass;
	typedef CPropertyPageImpl<thisClass> sheetClass;
	typedef CAutoSizeWindow<thisClass, false> autosizeClass;

public:	
	enum { IDD = IDD_NEWT_DETAILS };

    BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_DESTROY(OnDestroy)

		CHAIN_MSG_MAP(autosizeClass)
		CHAIN_MSG_MAP(sheetClass)
		REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

	static CWindowMapStruct* GetWindowMap();
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
	
	void OnFileBrowse(UINT, int, HWND hWnd) {}
	void OnDestroy() {}
	
private:
};

class NewTorrentDialog :
	public CPropertySheet,
	public hal::IniBase<NewTorrentDialog>,
	public CDialogResize<NewTorrentDialog>
{
	typedef NewTorrentDialog thisClass;
	typedef hal::IniBase<thisClass> iniClass;
	typedef CDialogResize<thisClass> resizeClass;

public:
    NewTorrentDialog(LPCTSTR title = (LPCTSTR)NULL,
			UINT uStartPage = 0, HWND hWndParent = NULL) :
        CPropertySheet(title, uStartPage, hWndParent),
		iniClass("NewTorrentDialog", "Dialog"),
		m_bCentered(false),
		rect_(50,50,400,500)
    {
		AddPage(fileSheet);
		AddPage(trackerSheet);
		AddPage(detailsSheet);		
	}

    BEGIN_MSG_MAP_EX(thisClass)
        MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_SIZE(OnSize)
		MSG_WM_DESTROY(OnDestroy)

		CHAIN_MSG_MAP(resizeClass)
        CHAIN_MSG_MAP(CPropertySheet)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(0x3020, DLSZ_SIZE_X|DLSZ_SIZE_Y)

		DLGRESIZE_CONTROL(0x1, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(0x2, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(0x3021, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(HWND, LPARAM);
	void OnDestroy() {}

	void OnSize(UINT, CSize)
	{
        SetMsgHandled(false);

		GetWindowRect(rect_);
		resizeActiveSheet();
	}

    void OnShowWindow(BOOL bShow, UINT nStatus)
    {
		hal::event().post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(L"NewTorrentDialog::OnShowWindow()")));	

        SetMsgHandled(false);

        if (bShow == TRUE)
        {
            CMenuHandle pSysMenu = GetSystemMenu(FALSE);

        	if (pSysMenu != NULL)
                pSysMenu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, SC_SIZE, L"&Size");

            ModifyStyle(0, WS_THICKFRAME, 0);
            Center();

            resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
        }

		resizeActiveSheet();
    }

    void Center(void)
    {
        if (!m_bCentered)
        {
            CenterWindow();
            m_bCentered = true;
        }
    }

	friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(rect_);
	}

private:
    bool m_bCentered;
	CRect rect_;

	void resizeActiveSheet()
	{
        HWND propTabs = GetDlgItem(0x3020);

		RECT rect;
		::GetWindowRect(propTabs, &rect);

		::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rect, 2);
		::SendMessage(propTabs, TCM_ADJUSTRECT, false, (LPARAM)&rect);

		::MoveWindow(GetActivePage(), rect.left, rect.top,
            rect.right-rect.left, rect.bottom-rect.top, true);
	}

	FileSheet fileSheet;
	TrackerSheet trackerSheet;
	DetailsSheet detailsSheet;

};


#endif // RC_INVOKED
