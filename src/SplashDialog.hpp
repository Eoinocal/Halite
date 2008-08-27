
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

//#define HAL_NEWTORRENT_CREATOR_TEXT		HAL_SPLASHDIALOG_BEGIN+3
//#define HAL_NEWTORRENT_CREATOR			HAL_SPLASHDIALOG_BEGIN+4

#ifndef RC_INVOKED

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
	typedef SplashDialog thisClass;
	typedef ATL::CDialogImpl<thisClass> baseClass;
	typedef hal::IniBase<thisClass> iniClass;
	typedef WTL::CDialogResize<thisClass> resizeClass;

public:
	enum { IDD = HAL_CLOSESPLASH };

	SplashDialog() :	
		iniClass("SplashDialog", "dialog"),	
		rect_(0,0,0,0)
	{
		load_from_ini();
	}

    BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_INITDIALOG(onInitDialog)
		COMMAND_ID_HANDLER_EX(HAL_SPLASH_FORCE_CLOSE, OnForceClose)

		CHAIN_MSG_MAP(resizeClass)
    END_MSG_MAP()
	
	BEGIN_DDX_MAP(CMainDlg)
        DDX_CHECK(HAL_SPLASH_MSG, halite().showMessage_)
    END_DDX_MAP()

    BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(HAL_CSPLASH_MSG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(HAL_CSPLASH_NUM_ACT, DLSZ_MOVE_Y|DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(HAL_SPLASH_MSG, DLSZ_MOVE_Y|DLSZ_SIZE_X)
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
		thread_ptr.reset();
		Sleep(200);

		GetWindowRect(rect_);
		save_to_ini();
		EndDialog(0);
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

		hal::bittorrent().close_all(boost::optional<boost::function<void (int)> >(bind(&SplashDialog::ReportNumActive, this, _1)));
		
		SetDlgItemText(HAL_CSPLASH_NUM_ACT, hal::app().res_wstr(HAL_CSPLASH_SHUTDOWN_MSG).c_str());

		HAL_DEV_MSG(L"SplashThread() calling hal::bittorrent().stopEventReceiver()");
		hal::bittorrent().stopEventReceiver();
		HAL_DEV_MSG(L"SplashThread() calling hal::bittorrent().shutDownSession()");
		hal::bittorrent().shutDownSession();
		
		DoDataExchange(true);

		GetWindowRect(rect_);
		save_to_ini();

		}
		catch (const access_violation& e) 
		{
			hal::event_log.post(shared_ptr<hal::EventDetail>(
				new hal::EventGeneral(hal::event_logger::warning, hal::event_logger::unclassified, 
				(hal::wform(L"Alert handler access_violation (code %1$x) at %2$x. IsWrite %3%, badd address %4$x") % e.code() % (unsigned)e.where() % e.isWrite() % (unsigned)e.badAddress()).str())));
		}
		catch (const win32_exception& e) 
		{
			hal::event_log.post(shared_ptr<hal::EventDetail>(
				new hal::EventGeneral(hal::event_logger::warning, hal::event_logger::unclassified, 
				(hal::wform(L"Alert handler win32_exception (code %1$x) at %2$x") % e.code() % (unsigned)e.where()).str())));
		}
		catch(std::exception& e)
		{
			hal::event_log.post(shared_ptr<hal::EventDetail>(\
				new hal::EventStdException(hal::event_logger::debug, e, L"SplashThread()")));
		}
		catch(...)
		{
			HAL_DEV_MSG(L"SplashThread() catch all");
		}

		EndDialog(0);
	}
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
	
private:
	boost::scoped_ptr<thread> thread_ptr;

	WTL::CRect rect_;
};

#endif // RC_INVOKED
