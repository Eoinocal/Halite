
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/function.hpp>
#include "halTorrent.hpp"

class AddTorrentDialog :
	public CDialogImpl<AddTorrentDialog>,
    public CWinDataExchangeEx<AddTorrentDialog>
{
protected:
	typedef AddTorrentDialog thisClass;
	typedef CDialogImpl<thisClass> baseClass;

public:
	AddTorrentDialog(wstring& d, bool& p, bool& c) :
		saveDirectory_(d),
		startPaused_(p),
		compactStorage_(c)
	{}
	
	enum { IDD = IDD_ADD_TORRENT };

    BEGIN_MSG_MAP_EX(thisClass)
        MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER_EX(IDOK, OnOk)
		COMMAND_ID_HANDLER_EX(IDC_BC_SAVEBROWSE, OnBrowse)
    END_MSG_MAP()
	
    BEGIN_DDX_MAP(thisClass)
        DDX_CHECK(IDC_CHECK_COMPACT, compactStorage_)
        DDX_CHECK(IDC_CHECK_PAUSED, startPaused_)
		DDX_EX_STDWSTRING(IDC_BC_SAVEFOLDER, saveDirectory_)
    END_DDX_MAP()	
	
	LRESULT OnInitDialog(HWND hwndFocus, LPARAM lParam)
	{
		CenterWindow();
		
		BOOL retval =  DoDataExchange(false);
		return retval;
	}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
	
	void OnCancel(UINT, int, HWND hWnd)
	{
		EndDialog(IDCANCEL);
	}
	
	void OnOk(UINT, int, HWND hWnd)
	{
		DoDataExchange(true);
		EndDialog(IDOK);
	}
	
	void OnBrowse(UINT, int, HWND hWnd)
	{
		std::wstring save_prompt = hal::app().res_wstr(IDS_SAVEPROMPT);		
		CFolderDialog fldDlg(NULL, save_prompt.c_str(),
			BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE);
		
		wstring defaultSaveFolder = saveDirectory_;
		fldDlg.SetInitialFolder(defaultSaveFolder.c_str());
	 
		if (IDOK == fldDlg.DoModal())
			saveDirectory_ = wstring(fldDlg.m_szFolderPath);
	}
	
private:
	wstring& saveDirectory_;
	bool& startPaused_;
	bool& compactStorage_;
};
