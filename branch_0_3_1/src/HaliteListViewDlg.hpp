
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once


#define HAL_ADJUST_DLG	 				30100

#define HAL_ADJUST_DLG_BEGIN	 		30101
#define IDC_ADDT_MOVETO_FOLDER			HAL_ADJUST_DLG_BEGIN + 1
#define IDC_ADDT_MOVETO_BROWSE			HAL_ADJUST_DLG_BEGIN + 2
#define IDC_ADDT_MOVETO_CHECK			HAL_ADJUST_DLG_BEGIN + 3
#define IDC_ADDT_DEFFLD_TEXT			HAL_ADJUST_DLG_BEGIN + 4
#define IDC_ADDT_NOTE_TEXT				HAL_ADJUST_DLG_BEGIN + 5
#define HAL_ADDT_TITLE					HAL_ADJUST_DLG_BEGIN + 6

#ifndef RC_INVOKED

#include "GenericAddDialog.hpp"

class HaliteSaveAndMoveToDlg :
	public CDialogImpl<HaliteSaveAndMoveToDlg>,
	public CAutoSizeWindow<HaliteSaveAndMoveToDlg, true>,
	public CWinDataExchangeEx<HaliteSaveAndMoveToDlg>
{
public:
	typedef HaliteSaveAndMoveToDlg thisClass;
	typedef CAutoSizeWindow<thisClass, true> autosizeClass;
	
public:
	HaliteSaveAndMoveToDlg(wstring& s, wstring& m, bool& u, bool d = false) :
		saveDirectory_(s),
		moveToDirectory_(m),
		useMove_(u),
		disableSaveDir_(d)
	{}

	enum { IDD = HAL_ADJUST_DLG };

    BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_ADDT_MOVETO_CHECK, OnMoveTo)
		COMMAND_ID_HANDLER_EX(IDC_BC_SAVEBROWSE, OnBrowse)
		COMMAND_ID_HANDLER_EX(IDC_ADDT_MOVETO_BROWSE, OnMoveBrowse)

        CHAIN_MSG_MAP(autosizeClass)
    END_MSG_MAP()

    BEGIN_DDX_MAP(thisClass)
        DDX_CHECK(IDC_ADDT_MOVETO_CHECK, useMove_)
		DDX_EX_STDWSTRING(IDC_BC_SAVEFOLDER, saveDirectory_)
		DDX_EX_STDWSTRING(IDC_ADDT_MOVETO_FOLDER, moveToDirectory_)
    END_DDX_MAP()	

#define ADD_FOLDERS_LAYOUT \
	WMB_HEAD(WMB_COLNOMAX(_exp), WMB_COL(_auto)), \
		WMB_ROW(_auto,	IDC_ADDT_DEFFLD_TEXT, _r), \
		WMB_ROW(_auto,	IDC_BC_SAVEFOLDER, IDC_BC_SAVEBROWSE), \
		WMB_ROW(_auto,	IDC_ADDT_MOVETO_CHECK, _r), \
		WMB_ROW(_auto,	IDC_ADDT_MOVETO_FOLDER, IDC_ADDT_MOVETO_BROWSE), \
	WMB_END()

	BEGIN_WINDOW_MAP(thisClass, 0, 0, 3, 3)
		WMB_HEAD(WMB_COLNOMAX(_exp)),
			WMB_ROW(_auto,	ADD_FOLDERS_LAYOUT),
		WMB_END()
	END_WINDOW_MAP()	

	LRESULT onInitDialog(HWND, LPARAM)
	{
		BOOL retval = DoDataExchange(false);

		if (disableSaveDir_)
		{
			::EnableWindow(GetDlgItem(IDC_BC_SAVEFOLDER), false);
			::EnableWindow(GetDlgItem(IDC_BC_SAVEBROWSE), false);
		}

		OnMoveTo(0, 0, GetDlgItem(IDC_ADDT_MOVETO_CHECK));

		SetMsgHandled(false);
		return 0;
	}
	
	void OnMoveTo(UINT, int, HWND hWnd)
	{
		LRESULT result = ::SendMessage(hWnd, BM_GETCHECK, 0, 0);

		if (result == BST_CHECKED)
		{
			::EnableWindow(GetDlgItem(IDC_ADDT_MOVETO_FOLDER), true);
			::EnableWindow(GetDlgItem(IDC_ADDT_MOVETO_BROWSE), true);
		}
		else
		{
			::EnableWindow(GetDlgItem(IDC_ADDT_MOVETO_FOLDER), false);
			::EnableWindow(GetDlgItem(IDC_ADDT_MOVETO_BROWSE), false);
		}
	}
	
	void OnBrowse(UINT, int, HWND hWnd)
	{
		std::wstring save_prompt = hal::app().res_wstr(IDS_SAVEPROMPT);		
		CFolderDialog fldDlg(NULL, save_prompt.c_str(),
			BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
		
		wstring defaultSaveFolder = saveDirectory_;
		fldDlg.SetInitialFolder(defaultSaveFolder.c_str());
	 
		if (IDOK == fldDlg.DoModal())
		{
			saveDirectory_ = wstring(fldDlg.m_szFolderPath);
			DoDataExchange(false);
		}
	}
	
	void OnMoveBrowse(UINT, int, HWND hWnd)
	{
		std::wstring save_prompt = hal::app().res_wstr(IDS_SAVEPROMPT);		
		CFolderDialog fldDlg(NULL, save_prompt.c_str(),
			BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
		
		wstring defaultMoveFolder = moveToDirectory_;
		fldDlg.SetInitialFolder(defaultMoveFolder.c_str());
	 
		if (IDOK == fldDlg.DoModal())
		{
			moveToDirectory_ = wstring(fldDlg.m_szFolderPath);
			DoDataExchange(false);
		}
	}		

private:
	wstring& saveDirectory_;
	wstring& moveToDirectory_;
	bool& useMove_;
	bool disableSaveDir_;
};

class HaliteListViewAdjustDlg :
	public WTLx::GenericAddContainerDialog<HaliteListViewAdjustDlg, HaliteSaveAndMoveToDlg, HAL_ADJUST_DLG>
{
public:
	typedef HaliteListViewAdjustDlg thisClass;
	typedef WTLx::GenericAddContainerDialog<thisClass, HaliteSaveAndMoveToDlg, HAL_ADJUST_DLG> genericBaseClass;
	
public:
	HaliteListViewAdjustDlg(wstring title, wstring& s, wstring& m, bool& u, bool d=false) :
		genericBaseClass(title, "genericAddDlgs/HaliteListViewAdjustDlg", "HaliteListViewAdjustDlg", dlg_),
		dlg_(s, m, u, d)
	{}

	BOOL DoDataExchange(bool direction)
	{
		return dlg_.DoDataExchange(direction);
	}

private:	
	HaliteSaveAndMoveToDlg dlg_;
};

#endif
