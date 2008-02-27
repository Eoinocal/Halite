
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define IDD_NEWTORRENT_BEGIN			1950
#define IDC_NEWTORRENT_SELECT_TEXT		IDD_NEWTORRENT_BEGIN+1
#define IDC_NEWTORRENT_TRACKERS_TEXT	IDD_NEWTORRENT_BEGIN+2
#define IDC_NEWTORRENT_CREATOR_TEXT		IDD_NEWTORRENT_BEGIN+3
#define IDC_NEWTORRENT_CREATOR			IDD_NEWTORRENT_BEGIN+4
#define IDC_NEWTORRENT_COMMENT_TEXT		IDD_NEWTORRENT_BEGIN+5
#define IDC_NEWTORRENT_COMMENT			IDD_NEWTORRENT_BEGIN+6
#define IDC_NEWTORRENT_PRIVATE			IDD_NEWTORRENT_BEGIN+7
#define IDC_NEWT_FILE_BROWSE			IDD_NEWTORRENT_BEGIN+8
#define IDC_NEWT_DIR_BROWSE				IDD_NEWTORRENT_BEGIN+9
#define IDC_NEWT_LISTFILES				IDD_NEWTORRENT_BEGIN+10
//#define IDC_PROG_CANCEL                 IDD_NEWTORRENT_BEGIN + 2
//#define IDC_PROG_PROGRESS               IDD_NEWTORRENT_BEGIN + 3

#ifndef RC_INVOKED

#include <boost/function.hpp>
#include "halTorrent.hpp"
#include "halIni.hpp"
#include "halEvent.hpp"



class FileSheet :
    public CPropertyPageImpl<FileSheet>,
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
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER_EX(IDC_NEWT_FILE_BROWSE, OnFileBrowse)
		COMMAND_ID_HANDLER_EX(IDC_NEWT_DIR_BROWSE, OnDirBrowse)

		CHAIN_MSG_MAP(autosizeClass)
		CHAIN_MSG_MAP(sheetClass)
		REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

	static CWindowMapStruct* GetWindowMap();
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
	
	void OnFileBrowse(UINT, int, HWND hWnd);
	void OnDirBrowse(UINT, int, HWND hWnd);
	void OnDestroy() {};
	
private:
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
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER_EX(IDC_NEWT_ADDTRACKER, OnFileBrowse)
//		COMMAND_ID_HANDLER_EX(IDC_NEWT_DIR_BROWSE, OnDirBrowse)

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
