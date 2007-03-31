
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

static class halite_log_file : public boost::signals::trackable
{
public:
	halite_log_file();
	
	void operator()(shared_ptr<hal::EventDetail> event)
	{
		if (halite().logToFile())
		{
			if (!wofs.is_open()) wofs.open("HaliteLog.txt");
			
			wofs << (wformat(L"%1% %2%, %3%\r\n") 
				% event->timeStamp() % hal::Event::eventLevelToStr(event->level()) 
				% event->msg());
			
			wofs.flush();
		}
	}
	
	void disconnect() { conn_.disconnect(); }
	
private:
	std::wofstream wofs;
	boost::signals::scoped_connection conn_;
	
} halite_log_file_;

halite_log_file::halite_log_file() :
	conn_(hal::event().attach(bind(&halite_log_file::operator(), &halite_log_file_, _1)))
{}

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

//	hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDebug(hal::Event::info, L"Hello")));
	
	try 
	{
	
	boost::filesystem::path::default_name_check(boost::filesystem::native);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);
   
	int nRet;	
	HRESULT hRes = _Module.Init(NULL, hInstance);
	assert (SUCCEEDED(hRes));	
	
	hal::ini().load_data();
	
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
		
		CMessageLoop theLoop;
		_Module.AddMessageLoop(&theLoop);
		{	
			HaliteWindow wndMain(WMU_ARE_YOU_ME);
			if (wndMain.CreateEx() == NULL)
				return 1;
			
			oneInstance.release();
			
			if (!hal::app().command_args().empty())
				wndMain.ProcessFile(hal::app().command_args().front().c_str());
			
			wndMain.SetIcon(LoadIcon(hInstance, MAKEINTRESOURCE(IDR_APP_ICON)), false);	
			wndMain.ShowWindow(nCmdShow);
			
			nRet = theLoop.Run();				
		}	
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
	
	_Module.Term();
	
	return nRet;
	
	}
	catch (const std::exception& e)
	{
	MessageBoxA(0, e.what(), "Exception Thrown!", 0);
	
	return -1;
	}
	
}
