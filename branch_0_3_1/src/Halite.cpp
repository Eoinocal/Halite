
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#define HALITE_GUID L"HALITE-{E3A8BF7D-962F-476E-886B-FECEDD2F0FC7}"
#define WMU_ARE_YOU_ME_STRING  L"WMU_ARE_YOU_ME_HALITE-{E3A8BF7D-962F-476E-886B-FECEDD2F0FC7}"
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "Halite.hpp"

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
			if (!wofs.is_open()) wofs.open(hal::app().working_directory()/L"HaliteLog.txt");
			
			wofs << (hal::wform(L"%1% %2%, %3%\r\n") 
				% event->timeStamp() % hal::event_logger::eventLevelToStr(event->level()) 
				% event->msg()).str();
			
			wofs.flush();
		}
	}
	
	void connect() 
	{ 
		conn_ = hal::event_log.attach(bind(&halite_log_file::operator(), this, _1)); 
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
	LRESULT ok = ::SendMessageTimeout(hWnd,	WMU_ARE_YOU_ME,
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
	int int_result = -1;
	
	try 
	{
	
	boost::filesystem::path::default_name_check(boost::filesystem::native);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	
	HINSTANCE hInstRich = ::LoadLibrary(CRichEditCtrl::GetLibraryName());
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
					hal::app().command_args().front().length()*sizeof(wchar_t); 
				cmdLine.lpData = const_cast<wchar_t*>(
					hal::app().command_args().front().c_str());
				
				::SendMessage(hOther, WM_COPYDATA, 0, (LPARAM)(LPVOID)&cmdLine);
			}
		}				
	}
	else
	{
		hal::event_log.post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(hal::wform(L"Exe Path: %1%.") % hal::app().exe_path())));		
		
		hal::event_log.post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(hal::wform(L"Initial Path: %1%.") % hal::app().initial_path())));		
		
		hal::event_log.post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg((hal::wform(L"Working Directory: %1%.") % hal::app().working_directory()), hal::event_logger::info)));		
		
		CMessageLoop theLoop;
		_Module.AddMessageLoop(&theLoop);
			
			if (halite().dll() != L"") hal::app().res_set_dll(halite().dll());
			
			HaliteWindow wndMain(WMU_ARE_YOU_ME);
			if (wndMain.CreateEx() == NULL)
				return 1;
			
			oneInstance.release();
			
			if (!hal::app().command_args().empty())
				wndMain.ProcessFile(hal::app().command_args().front().c_str());
			
			wndMain.SetIcon(LoadIcon(hInstance, MAKEINTRESOURCE(IDR_APP_ICON)), false);	
			wndMain.ShowWindow(nCmdShow);
			
			int_result = theLoop.Run();				
		
		_Module.RemoveMessageLoop();

		halite().save_to_ini();
		hal::ini().save_data();		
	}
	}

	::FreeLibrary(hInstRich);	
	_Module.Term();
	
	}
	catch(const access_violation& e)
	{
		MessageBoxW(0, (hal::wform(L"WinMain() access_violation (code %1$x) at %2$x. Bad address %3$x") 
			% e.code() % (unsigned)e.where() % (unsigned)e.badAddress()).str().c_str(), L"Exception Thrown!", 0);
	}
	catch(const win32_exception& e)
	{
		MessageBoxW(0, (hal::wform(L"WinMain() win32_exception (code %1$x) at %2$x") 
			% e.code() % (unsigned)e.where()).str().c_str(), L"Exception Thrown!", 0);
	}
	catch(const std::exception& e)
	{
		MessageBoxW(0, (hal::wform(L"WinMain() std::exception, %1%") 
			% hal::from_utf8(e.what())).str().c_str(), L"Exception Thrown!", 0);
	}
	catch(...)
	{
		MessageBoxW(0, L"WinMain() catch all", L"Exception Thrown!", 0);
	}
	
	return int_result;
}
