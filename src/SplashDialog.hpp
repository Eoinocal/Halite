
//         Copyright E�in O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define HAL_SPLASHDIALOG_BEGIN			20200
#define HAL_CSPLASH_MSG					HAL_SPLASHDIALOG_BEGIN+1
#define HAL_CSPLASH_NUM_ACT			HAL_SPLASHDIALOG_BEGIN+2
#define HAL_CSPLASH_ACT_MSG			HAL_SPLASHDIALOG_BEGIN+3
#define HAL_CSPLASH_SHUTDOWN_MSG		HAL_SPLASHDIALOG_BEGIN+4
#define HAL_SPLASH_FORCE_CLOSE			HAL_SPLASHDIALOG_BEGIN+5

//#define HAL_NEWTORRENT_CREATOR_TEXT		HAL_SPLASHDIALOG_BEGIN+3
//#define HAL_NEWTORRENT_CREATOR			HAL_SPLASHDIALOG_BEGIN+4

#ifndef RC_INVOKED

#include <boost/utility/in_place_factory.hpp>
#include <boost/none.hpp>

#ifndef HALITE_MINI
#	include "Halite.hpp"
#else
#	include "Halite.hpp"
#endif

#include "win32_exception.hpp"
#include "halTorrent.hpp"

class SplashDialog :
	public ATL::CDialogImpl<SplashDialog>,
	public hal::IniBase<SplashDialog>,
	public WTL::CWinDataExchange<SplashDialog>,
	public WTL::CDialogResize<SplashDialog>
{
protected:
	typedef SplashDialog this_class_t;
	typedef ATL::CDialogImpl<this_class_t> base_class_t;
	typedef hal::IniBase<this_class_t> ini_class_t;
	typedef WTL::CDialogResize<this_class_t> resize_class_t;

public:
	enum { IDD = HAL_CLOSESPLASH };

	SplashDialog() :	
		ini_class_t(L"splash_dialog", L"dialog"),	
		rect_(0,0,0,0)
	{
		load_from_ini();
	}

	BEGIN_MSG_MAP_EX(CMainDlg)
		try
		{
		MSG_WM_INITDIALOG(onInitDialog)
		COMMAND_ID_HANDLER_EX(HAL_SPLASH_FORCE_CLOSE, OnForceClose)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in SplashDialog MSG_MAP")

		CHAIN_MSG_MAP(resize_class_t)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CMainDlg)
		DDX_CHECK(HAL_SPLASH_MSG, halite().showMessage_)
	END_DDX_MAP()

	BEGIN_DLGRESIZE_MAP(this_class_t)
		DLGRESIZE_CONTROL(HAL_CSPLASH_MSG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(HAL_CSPLASH_NUM_ACT, DLSZ_MOVE_Y|DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(HAL_SPLASH_MSG, DLSZ_MOVE_Y|DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(HAL_SPLASH_FORCE_CLOSE, DLSZ_MOVE_Y|DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 2:			
			ar & boost::serialization::make_nvp("rect", rect_);
		
		break;

		case 1:
		default:
			assert(false);
		}
	}
	
	LRESULT onInitDialog(HWND, LPARAM)
	{
		resize_class_t::DlgResize_Init(false, true, WS_CLIPCHILDREN);

		if (rect_.left != rect_.right)
			MoveWindow(rect_.left, rect_.top, 
				rect_.right-rect_.left, rect_.bottom-rect_.top, true);	

		CenterWindow();

		DoDataExchange(false);		
		
		thread_splash = boost::in_place<boost::function<void (void)> >(bind(&SplashDialog::SplashThread, this));
		
		return 0;
	}

	void OnForceClose(UINT, int, HWND hWnd)
	{		
		HAL_DEV_MSG(L"SplashDialog::OnForceClose()");

		thread_splash->interrupt();
		Sleep(20);
		
		RequiredToEnd();
	}

	void RequiredToEnd()
	{
		SetDlgItemText(HAL_CSPLASH_NUM_ACT, hal::app().res_wstr(HAL_CSPLASH_SHUTDOWN_MSG).c_str());

		HAL_DEV_MSG(L"SplashThread() calling hal::bittorrent().stop_event_receiver()");
		hal::bittorrent().stop_event_receiver();
		HAL_DEV_MSG(L"SplashThread() calling hal::bittorrent().shutdown_session()");
		hal::bittorrent().shutdown_session();
		HAL_DEV_MSG(L"SplashThread() Session shutdown");

		DoDataExchange(true);

		GetWindowRect(rect_);
		save_to_ini();
	}

	void ReportNumActive(int num)
	{
		SetDlgItemText(HAL_CSPLASH_NUM_ACT, (hal::wform(hal::app().res_wstr(HAL_CSPLASH_ACT_MSG)) % num).str().c_str());
	}

	void SplashThread()
	{
		win32_exception::install_handler();

		try
		{

		HAL_DEV_MSG(L"SplashThread() calling hal::bittorrent().close_all()");

		hal::bittorrent().close_all(boost::optional<boost::function<void (int)> >
			(bind(&SplashDialog::ReportNumActive, this, _1)));
		
		RequiredToEnd();		

		} HAL_GENERIC_FN_EXCEPTION_CATCH(L"SplashThread()")

		EndDialog(0);
	}
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
	
private:
	boost::optional<hal::thread_t> thread_splash;

	WTL::CRect rect_;
};

BOOST_CLASS_VERSION(SplashDialog, 2)

#endif // RC_INVOKED
