
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define HALITE_GUID L"HALITE-{E3A8BF7D-962F-476E-886B-FECEDD2F0FC7}"
#define WMU_ARE_YOU_ME_STRING  L"WMU_ARE_YOU_ME_HALITE-{E3A8BF7D-962F-476E-886B-FECEDD2F0FC7}"
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "stdAfx.hpp"

#include "WinAPIMutex.hpp"

#include "global/ini.hpp"
#include "global/logger.hpp"
#include "halConfig.hpp"

#include "HaliteWindow.hpp"
#include "SplashDialog.hpp"

#include "Halite.hpp"

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
			
			wofs << (wformat(L"%1% %2%, %3%\r\n") 
				% event->timeStamp() % hal::Event::eventLevelToStr(event->level()) 
				% event->msg()).str();
			
			wofs.flush();
		}
	}
	
	void connect() 
	{ 
		conn_ = hal::event().attach(bind(&halite_log_file::operator(), this, _1)); 
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

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	HAL_DEV_MSG(L"Hello")

//	hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::Event::info, L"Hello")));
	
	try 
	{
	
	boost::filesystem::path::default_name_check(boost::filesystem::native);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	
	HINSTANCE hInstRich = ::LoadLibrary(CRichEditCtrl::GetLibraryName());
	ATLASSERT(hInstRich != NULL);
   
	int nRet;	
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
//		INI().LoadData();	
		hal::event().post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(wformat(L"Exe Path: %1%.") % hal::app().exe_path())));		
		
		hal::event().post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg(wformat(L"Initial Path: %1%.") % hal::app().initial_path())));		
		
		hal::event().post(shared_ptr<hal::EventDetail>(
			new hal::EventMsg((wformat(L"Working Directory: %1%.") % hal::app().working_directory()), hal::Event::info)));		
		
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
			
			nRet = theLoop.Run();				
		
		_Module.RemoveMessageLoop();
		
		hal::bittorrent().stopEventReceiver();
		
		if (halite().showMessage)
		{
			SplashDialog splDlg;
			splDlg.DoModal();
		}
		else
		{
			hal::bittorrent().closeAll();
			hal::bittorrent().shutDownSession();		
		}
		
//		INI().SaveData();
		
		halite().save();
		hal::ini().save_data();
	}
	}

	::FreeLibrary(hInstRich);	
	_Module.Term();
	
	return nRet;
	
	}
	catch (const std::exception& e)
	{
	MessageBoxA(0, e.what(), "Exception Thrown!", 0);
	
	return -1;
	}
	
}
