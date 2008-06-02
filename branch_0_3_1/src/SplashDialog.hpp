
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define HAL_SPLASHDIALOG_BEGIN			20200
#define HAL_CSPLASH_MSG					HAL_SPLASHDIALOG_BEGIN+1
#define HAL_CSPLASH_NUM_ACT				HAL_SPLASHDIALOG_BEGIN+2
#define HAL_CSPLASH_ACT_MSG				HAL_SPLASHDIALOG_BEGIN+3
#define HAL_CSPLASH_SHUTDOWN_MSG		HAL_SPLASHDIALOG_BEGIN+4
#define HAL_SPLASH_FORCE_CLOSE			HAL_SPLASHDIALOG_BEGIN+5

//#define IDC_NEWTORRENT_CREATOR_TEXT		HAL_SPLASHDIALOG_BEGIN+3
//#define IDC_NEWTORRENT_CREATOR			HAL_SPLASHDIALOG_BEGIN+4

#ifndef RC_INVOKED

#ifndef HALITE_MINI
#	include "Halite.hpp"
#else
#	include "Halite.hpp"
#endif

#include "halTorrent.hpp"

class SplashDialog :
	public CDialogImpl<SplashDialog>,
	public hal::IniBase<SplashDialog>,
	public CWinDataExchange<SplashDialog>,
	public CDialogResize<SplashDialog>
{
protected:
	typedef SplashDialog thisClass;
	typedef CDialogImpl<thisClass> baseClass;
	typedef hal::IniBase<thisClass> iniClass;
	typedef CDialogResize<thisClass> resizeClass;

public:
	enum { IDD = IDD_CLOSESPLASH };

	SplashDialog() :	
		iniClass("SplashDialog", "dialog"),	
		rect_(0,0,0,0)
	{
		Load();
	}

    BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_INITDIALOG(onInitDialog)
		COMMAND_ID_HANDLER_EX(HAL_SPLASH_FORCE_CLOSE, OnForceClose)

		CHAIN_MSG_MAP(resizeClass)
    END_MSG_MAP()
	
	BEGIN_DDX_MAP(CMainDlg)
        DDX_CHECK(IDC_SPLASH_MSG, halite().showMessage_)
    END_DDX_MAP()

    BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(HAL_CSPLASH_MSG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(HAL_CSPLASH_NUM_ACT, DLSZ_MOVE_Y|DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_SPLASH_MSG, DLSZ_MOVE_Y|DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(HAL_SPLASH_FORCE_CLOSE, DLSZ_MOVE_Y|DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(rect_);
	}
	
	LRESULT onInitDialog(HWND, LPARAM)
	{
		resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);

		if (rect_.left != rect_.right)
			MoveWindow(rect_.left, rect_.top, 
				rect_.right-rect_.left, rect_.bottom-rect_.top, true);	

		CenterWindow();

		DoDataExchange(false);
		
		thread_ptr.reset(new thread(bind(&SplashDialog::SplashThread, this)));
		
		return 0;
	}

	void OnForceClose(UINT, int, HWND hWnd)
	{
		GetWindowRect(rect_);
		Save();
		EndDialog(0);
	}

	void ReportNumActive(int num)
	{
		SetDlgItemText(HAL_CSPLASH_NUM_ACT, (wformat(hal::app().res_wstr(HAL_CSPLASH_ACT_MSG)) % num).str().c_str());
	}

	void SplashThread()
	{
		hal::bittorrent().closeAll(boost::optional<boost::function<void (int)> >(bind(&SplashDialog::ReportNumActive, this, _1)));
		
		SetDlgItemText(HAL_CSPLASH_NUM_ACT, hal::app().res_wstr(HAL_CSPLASH_SHUTDOWN_MSG).c_str());

		hal::bittorrent().shutDownSession();
		
		DoDataExchange(true);

		GetWindowRect(rect_);
		Save();
		EndDialog(0);
	}
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
	
private:
	boost::scoped_ptr<thread> thread_ptr;

	CRect rect_;
};

#endif // RC_INVOKED
