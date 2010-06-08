
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#ifndef NDEBUG
#	define HALITE_GUID L"HALITE-{E3A8BF7D-962F-476E-886B-FECEDD2F0FC7}-DEBUG"
#	define WMU_ARE_YOU_ME_STRING L"WMU_ARE_YOU_ME_HALITE-{E3A8BF7D-962F-476E-886B-FECEDD2F0FC7}-DEBUG"
#else
#	define HALITE_GUID L"HALITE-{E3A8BF7D-962F-476E-886B-FECEDD2F0FC7}"
#	define WMU_ARE_YOU_ME_STRING L"WMU_ARE_YOU_ME_HALITE-{E3A8BF7D-962F-476E-886B-FECEDD2F0FC7}"
#endif

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "Halite.hpp"

#include <winstl/registry/registry.hpp>
#include <winstl/registry/reg_key.hpp>
#include <winstl/registry/reg_value.hpp>

#include "WinAPIMutex.hpp"

#include "global/ini.hpp"
#include "global/logger.hpp"
#include "halConfig.hpp"

#include "HaliteWindow.hpp"
#include "SplashDialog.hpp"

Halite& halite()
{
	static Halite h;
	return h;
}

Halite::Halite() :
	hal::IniBase<Halite>("globals/halite", "Halite"),
	oneInst(true),
#	ifdef TORRENT_LOGGING
		logDebug_(true),
#	else
		logDebug_(false),
#	endif
	showMessage_(true),
	logToFile_(true),
	logListLen_(128),
	dll_(L"")
{
	hal::event_log().init();
	load_from_ini();
}

namespace fs = boost::filesystem;
using fs::ifstream;
using fs::ofstream;
	
static class halite_log_file : public boost::signals::trackable
{
public:	
	void operator()(shared_ptr<hal::EventDetail> event)
	{
		if (halite().logToFile())
		{
			if (!wofs.is_open()) wofs.open(hal::app().get_working_directory()/L"HaliteLog.txt");
			
			wofs << (hal::wform(L"%1% %2%, %3%\n") 
				% event->timeStamp() % hal::event_logger::eventLevelToStr(event->level()) 
				% event->msg()).str();
			
			wofs.flush();
		}
	}
	
	void connect() 
	{ 
		hal::event_log().init();
		conn_ = hal::event_log().attach(bind(&halite_log_file::operator(), this, _1)); 
		assert(conn_.connected());
	}
	
	void disconnect() { conn_.disconnect(); }
	
private:
	fs::wofstream wofs;
	boost::signals::connection conn_;
	
} halite_log_file_;

static const unsigned WMU_ARE_YOU_ME = ::RegisterWindowMessage(WMU_ARE_YOU_ME_STRING);

static BOOL CALLBACK hwndSearcher(HWND hWnd, LPARAM lParam)
{
	ULONG_PTR result;
	LRESULT ok = ::SendMessageTimeout(hWnd, WMU_ARE_YOU_ME,
		0, 0, SMTO_BLOCK | SMTO_ABORTIFHUNG, 200, &result);
	
	if (ok == 0)
		return false;
	
	if (result == WMU_ARE_YOU_ME)
	{
		HWND* target = (HWND*)lParam;
		*target = hWnd;
		return false;
	}
	
	return true;
}

void num_active(int) {}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	win32_exception::install_handler();

	int return_result = -1;

	try 
	{
	
	boost::filesystem::path::default_name_check(boost::filesystem::native);

	try
	{
	winstl::reg_key_w reg_path(HKEY_CURRENT_USER, L"SOFTWARE\\Halite");
	winstl::reg_value_w reg_path_value = reg_path.get_value(L"path");

	if (hal::app().get_local_appdata())
	{
#	ifndef NDEBUG
		hal::app().working_directory = hal::app().get_local_appdata().get()/L"Halite.Backup";
#	else
		hal::app().working_directory = hal::app().get_local_appdata().get()/L"Halite";
#	endif
	}

	}
	catch(...)
	{
		hal::event_log().post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(L"No registry entry found, using portable mode", hal::event_logger::info)));		
	}
	
	if (!boost::filesystem::is_directory(hal::app().working_directory))
		boost::filesystem::create_directories(hal::app().working_directory);

	WTL::AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES);	
	HINSTANCE hInstRich = ::LoadLibrary(WTL::CRichEditCtrl::GetLibraryName());
	ATLASSERT(hInstRich != NULL);
   
	HRESULT hRes = _Module.Init(NULL, hInstance);
	assert (SUCCEEDED(hRes));	
	
	hal::ini().load_data();
	halite_log_file_.connect();
	
	{ WinAPIMutex oneInstance(HALITE_GUID);
	
	if (!oneInstance.owner() && halite().oneInst)
	{
		WinAPIMutexLock lock(oneInstance, 5000L);
		
		HWND hOther = NULL;
		::EnumWindows(static_cast<WNDENUMPROC>(hwndSearcher), (LPARAM)&hOther);
		
		if (hOther != NULL)
		{
			::SetForegroundWindow(hOther);
			
			if (::IsIconic(hOther))
				::ShowWindow(hOther, SW_RESTORE);
			
			if (!hal::app().command_args().empty())
			{
				COPYDATASTRUCT cmdLine; 
				cmdLine.dwData = HALITE_SENDING_CMD; 
				cmdLine.cbData = 
					numeric_cast<DWORD>(hal::app().command_args().front().length())*sizeof(wchar_t); 
				cmdLine.lpData = const_cast<wchar_t*>(
					hal::app().command_args().front().c_str());
				
				::SendMessage(hOther, WM_COPYDATA, 0, (LPARAM)(LPVOID)&cmdLine);
			}
		}				
	}
	else
	{
		hal::event_log().post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(hal::wform(L"App Data Path: %1%.") % hal::app().local_appdata)));		

		hal::event_log().post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(hal::wform(L"Exe Path: %1%.") % hal::app().exe_path())));		
		
		hal::event_log().post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(hal::wform(L"Initial Path: %1%.") % hal::app().initial_path())));		
		
		hal::event_log().post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg((hal::wform(L"Working Directory: %1%.") % hal::app().working_directory), hal::event_logger::info)));		
		
		WTL::CMessageLoop theLoop;
		_Module.AddMessageLoop(&theLoop);

#		if (_ATL_VER > 0x0700)
			if (halite().dll() != L"") hal::app().res_set_dll(halite().dll());
#		endif // (_ATL_VER > 0x0700)			
			HaliteWindow wndMain(WMU_ARE_YOU_ME);
			if (wndMain.CreateEx() == NULL)
				return 1;
			
			oneInstance.release();
			
			if (!hal::app().command_args().empty())
				wndMain.ProcessFile(hal::app().command_args().front().c_str());
			
			wndMain.SetIcon(LoadIcon(hInstance, MAKEINTRESOURCE(HAL_APP_ICON)), false);	
			wndMain.ShowWindow(nCmdShow);
			
			return_result = theLoop.Run();				
		
		_Module.RemoveMessageLoop();

		halite().save_to_ini();
		hal::ini().save_data();		
	}
	}

	::FreeLibrary(hInstRich);	
	_Module.Term();
	
	}
	catch (const access_violation& e)
	{
		wstring msg = (hal::wform(L"WinMain() access_violation (code %1$x) at %2$x. Bad address %3$x") 
			% e.code() % (unsigned)e.where() % (unsigned)e.badAddress()).str();

		MessageBox(0, msg.c_str(), L"Exception Thrown!", 0);
	} 
	catch (const win32_exception& e)
	{
		wstring msg = (hal::wform(L"WinMain() win32_exception (code %1$x) at %2$x") 
			% e.code() % (unsigned)e.where()).str();

		MessageBox(0, msg.c_str(), L"Exception Thrown!", 0);
	}
	catch (std::exception& e)
	{
		wstring msg = (hal::wform(L"WinMain() std::exception, %1%") % hal::from_utf8(e.what())).str();

		MessageBox(0, msg.c_str(), L"Exception Thrown!", 0);
	}
	catch (...)
	{
		MessageBox(0, L"WinMain() catch all", L"Exception Thrown!", 0);
	}
	
	return return_result;
}
