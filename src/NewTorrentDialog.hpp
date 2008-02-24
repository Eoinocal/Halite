
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

class NewTorrentDialog :
	public CDialogImpl<NewTorrentDialog>,	
	public hal::IniBase<NewTorrentDialog>,
	public CAutoSizeWindow<NewTorrentDialog, false>
{
protected:
	typedef NewTorrentDialog thisClass;
	typedef CDialogImpl<thisClass> baseClass;
	typedef hal::IniBase<NewTorrentDialog> iniClass;
	typedef CAutoSizeWindow<thisClass, false> autosizeClass;

public:
	NewTorrentDialog() :
		iniClass("NewTorrents", "Dialog"),
		rect_(50,50,400,500)
	{		
		Load();
	}

	~NewTorrentDialog()
	{}
	
	enum { IDD = IDD_NEWTORRENT };

    BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER_EX(IDC_NEWT_FILE_BROWSE, OnFileBrowse)
		COMMAND_ID_HANDLER_EX(IDC_NEWT_DIR_BROWSE, OnDirBrowse)

		COMMAND_ID_HANDLER_EX(IDCANCEL, onCancel)
		COMMAND_ID_HANDLER_EX(IDOK, onCancel)

		CHAIN_MSG_MAP(autosizeClass)
		REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

	friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(rect_);
	}

	LRESULT onInitDialog(HWND, LPARAM);
	static CWindowMapStruct* GetWindowMap();

	void ProgressThread()
	{
//		fn_(bind(&ProgressDialog::Callback, this, _1));
		
//		EndDialog(0);
	}
	
	bool Callback(size_t progress)
	{
//		prog_.SetPos(progress);
		
		return true;
	}
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
//		return this->IsDialogMessage(pMsg);
	}
	
	void OnFileBrowse(UINT, int, HWND hWnd);
	void OnDirBrowse(UINT, int, HWND hWnd);
	void onCancel(UINT, int, HWND hWnd);
	void OnClose();
	void OnSize(UINT, CSize);
	void OnDestroy();
	
private:
	CRect rect_;
};

#endif // RC_INVOKED
