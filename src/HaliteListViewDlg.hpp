
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once


#define HAL_ADJUST_DLG	 				30100

#define HAL_ADJUST_DLG_BEGIN	 		30101
#define HAL_ADDT_MOVETO_FOLDER			HAL_ADJUST_DLG_BEGIN + 1
#define HAL_ADDT_MOVETO_BROWSE			HAL_ADJUST_DLG_BEGIN + 2
#define HAL_ADDT_MOVETO_CHECK			HAL_ADJUST_DLG_BEGIN + 3
#define HAL_ADDT_DEFFLD_TEXT			HAL_ADJUST_DLG_BEGIN + 4
#define HAL_ADDT_NOTE_TEXT				HAL_ADJUST_DLG_BEGIN + 5
#define HAL_ADDT_TITLE					HAL_ADJUST_DLG_BEGIN + 6
#define HAL_FREE_TEXT                   HAL_ADJUST_DLG_BEGIN + 7
#define HAL_ADDT_DEFFLD_FREE   	        HAL_ADJUST_DLG_BEGIN + 8
#define HAL_ADDT_MOVETO_FOLDER_FREE   	HAL_ADJUST_DLG_BEGIN + 9

#ifndef RC_INVOKED

#include "GenericAddDialog.hpp"

class HaliteSaveAndMoveToDlg :
	public ATL::CDialogImpl<HaliteSaveAndMoveToDlg>,
	public ATL::CAutoSizeWindow<HaliteSaveAndMoveToDlg, true>,
	public WTLx::WinDataExchangeEx<HaliteSaveAndMoveToDlg>
{
public:
	typedef HaliteSaveAndMoveToDlg this_class_t;
	typedef ATL::CAutoSizeWindow<this_class_t, true> autosizeClass;
	
public:
	HaliteSaveAndMoveToDlg(wstring& s, wstring& m, bool& u, bool d = false) :
		saveDirectory_(s),
		moveToDirectory_(m),
		useMove_(u),
		disableSaveDir_(d)
	{}

	enum { IDD = HAL_ADJUST_DLG };

	BEGIN_MSG_MAP_EX(this_class_t)
		try
		{
		MSG_WM_INITDIALOG(onInitDialog)
		COMMAND_ID_HANDLER_EX(HAL_ADDT_MOVETO_CHECK, OnMoveTo)
		COMMAND_ID_HANDLER_EX(HAL_BC_SAVEBROWSE, OnBrowse)
		COMMAND_ID_HANDLER_EX(HAL_ADDT_MOVETO_BROWSE, OnMoveBrowse)
        COMMAND_ID_HANDLER_EX(HAL_BC_SAVEFOLDER, OnSaveFolderChange)
        COMMAND_ID_HANDLER_EX(HAL_ADDT_MOVETO_FOLDER, OnMoveFolderChange)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in HaliteSaveAndMoveToDlg MSG_MAP")

		CHAIN_MSG_MAP(autosizeClass)
	END_MSG_MAP()

	BEGIN_DDX_MAP(this_class_t)
		DDX_CHECK(HAL_ADDT_MOVETO_CHECK, useMove_)
		DDX_EX_STDWSTRING(HAL_BC_SAVEFOLDER, saveDirectory_)
		DDX_EX_STDWSTRING(HAL_ADDT_MOVETO_FOLDER, moveToDirectory_)
	END_DDX_MAP()	

#define ADD_FOLDERS_LAYOUT \
	WMB_HEAD(WMB_COLNOMAX(_exp), WMB_COL(60), WMB_COL(_auto)), \
		WMB_ROW(_auto,	HAL_ADDT_DEFFLD_TEXT, HAL_ADDT_DEFFLD_FREE, _r), \
		WMB_ROW(_auto,	HAL_BC_SAVEFOLDER, _r, HAL_BC_SAVEBROWSE), \
		WMB_ROW(_auto,	HAL_ADDT_MOVETO_CHECK, HAL_ADDT_MOVETO_FOLDER_FREE, _r), \
		WMB_ROW(_auto,	HAL_ADDT_MOVETO_FOLDER, _r, HAL_ADDT_MOVETO_BROWSE), \
	WMB_END()

	BEGIN_WINDOW_MAP(this_class_t, 0, 0, 3, 3)
		WMB_HEAD(WMB_COLNOMAX(_exp)),
			WMB_ROW(_auto,	ADD_FOLDERS_LAYOUT),
		WMB_END()
	END_WINDOW_MAP()	

	LRESULT onInitDialog(HWND, LPARAM)
	{
		BOOL retval = DoDataExchange(false);

		if (disableSaveDir_)
		{
			::EnableWindow(GetDlgItem(HAL_BC_SAVEFOLDER), false);
			::EnableWindow(GetDlgItem(HAL_BC_SAVEBROWSE), false);
		}

		OnMoveTo(0, 0, GetDlgItem(HAL_ADDT_MOVETO_CHECK));

		SetMsgHandled(false);
		return 0;
	}
	
	void OnMoveTo(UINT, int, HWND hWnd)
	{
		LRESULT result = ::SendMessage(hWnd, BM_GETCHECK, 0, 0);
		DoDataExchange(true);

		if (result == BST_CHECKED)
		{
			::EnableWindow(GetDlgItem(HAL_ADDT_MOVETO_FOLDER), true);
			::EnableWindow(GetDlgItem(HAL_ADDT_MOVETO_BROWSE), true);

			if (moveToDirectory_.empty())
			{
				moveToDirectory_ = saveDirectory_;
				
				DoDataExchange(false);
			}
		}
		else
		{
			::EnableWindow(GetDlgItem(HAL_ADDT_MOVETO_FOLDER), false);
			::EnableWindow(GetDlgItem(HAL_ADDT_MOVETO_BROWSE), false);
		}
		
	}
	
	void OnBrowse(UINT, int, HWND hWnd)
	{
		std::wstring save_prompt = hal::app().res_wstr(IDS_SAVEPROMPT);		
		WTL::CFolderDialog fldDlg(NULL, save_prompt.c_str(),
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
		WTL::CFolderDialog fldDlg(NULL, save_prompt.c_str(),
			BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
		
		wstring defaultMoveFolder = moveToDirectory_;
		fldDlg.SetInitialFolder(defaultMoveFolder.c_str());
	 
		if (IDOK == fldDlg.DoModal())
		{
			moveToDirectory_ = wstring(fldDlg.m_szFolderPath);
			DoDataExchange(false);
		}
	}

    void UpdateDiskSpace(int iSrcId, int iDstId)
    {
        hal::win_c_str<std::wstring> str_buf(MAX_PATH);
        GetDlgItem(iSrcId).GetWindowText(str_buf, numeric_cast<int>(str_buf.size()));

        boost::system::error_code ec;
        boost::filesystem::space_info si = boost::filesystem::space(str_buf.str(), ec);

        wstring free_space(L"?");
        if (!ec)
        {
            free_space = hal::to_bytes_size(si.free, false);
        }

        wstring free_space_string = (hal::wform(L"[%s: %s]") % hal::app().res_wstr(HAL_FREE_TEXT) % free_space).str();
        GetDlgItem(iDstId).SetWindowText(free_space_string.c_str());
    }

    void OnSaveFolderChange(UINT, int, HWND hWnd)
    {
        UpdateDiskSpace(HAL_BC_SAVEFOLDER, HAL_ADDT_DEFFLD_FREE);
    }

    void OnMoveFolderChange(UINT, int, HWND hWnd)
    {
        UpdateDiskSpace(HAL_ADDT_MOVETO_FOLDER, HAL_ADDT_MOVETO_FOLDER_FREE);
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
	typedef HaliteListViewAdjustDlg this_class_t;
	typedef WTLx::GenericAddContainerDialog<this_class_t, HaliteSaveAndMoveToDlg, HAL_ADJUST_DLG> generic_dlg_class_t;
	
public:
	HaliteListViewAdjustDlg(wstring title, wstring& s, wstring& m, bool& u, bool d=false) :
		generic_dlg_class_t(title, L"genericAddDlgs/HaliteListViewAdjustDlg", L"HaliteListViewAdjustDlg", dlg_),
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
