
#include "stdAfx.hpp"
#include "WinAPIMutex.hpp"

#include "GlobalIni.hpp"
#include "ini/Window.hpp"
#include "ini/General.hpp"

#include "HaliteWindow.hpp"
#include "SplashDialog.hpp"

#include "Halite.hpp"

#ifndef NDEBUG
#	include <global_log.hpp>
	using glb::wlog;
#	include "DebugDialog.hpp"
	
	static DebugDialog global_debugDialog_;	

	static class global_log_file
	{
	public:
		global_log_file();
		
		void operator()(const wstring& text)
		{
			if (!wofs.is_open()) wofs.open("Log.txt");			
			wofs << text;
		}
		
	private:	
		std::wofstream wofs;
		boost::signals::scoped_connection conn_;
		
	} global_log_file_;
	
	global_log_file::global_log_file() :
		conn_(wlog().attach(bind(global_log_file::operator(), &global_log_file_, _1)))
	{}

#endif

static class halite_log_file
{
public:	
	void operator()(std::auto_ptr<halite::EventDetail> event)
	{
		if (!wofs.is_open()) wofs.open("HaliteLog.txt");
		
		wofs << (wformat(L"%1% %2%, %3%\r\n") 
			% event->timeStamp() % halite::BitTorrent::eventLevelToStr(event->level()) 
			% event->msg());
	}
	
private:
	std::wofstream wofs;
	boost::signals::scoped_connection conn_;
	
} halite_log_file_;

halite_log_file::halite_log_file() :
	conn_(halite::bittorrent().attachEventReceiver(bind(&halite_log_file::operator(), &halite_log_file_, _1)))
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
	boost::filesystem::path::default_name_check(boost::filesystem::native);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);
   
	int nRet;	
	HRESULT hRes = _Module.Init(NULL, hInstance);
	assert (SUCCEEDED(hRes));	
	
	{ WinAPIMutex oneInstance(HALITE_GUID);
	
	if (!oneInstance.owner() && INI().generalConfig().oneInst)
	{
		WinAPIMutexLock lock(oneInstance, 5000L);
		
		HWND hOther = NULL;
		::EnumWindows(static_cast<WNDENUMPROC>(hwndSearcher), (LPARAM)&hOther);
		
		if (hOther != NULL)
		{
			::SetForegroundWindow(hOther);
			
			if (::IsIconic(hOther))
				::ShowWindow(hOther, SW_RESTORE);
			
			if (!globalModule().commandArgs().empty())
			{
				COPYDATASTRUCT cmdLine; 
				cmdLine.dwData = HALITE_SENDING_CMD; 
				cmdLine.cbData = 
					globalModule().commandArgs().front().length()*sizeof(wchar_t); 
				cmdLine.lpData = const_cast<wchar_t*>(
					globalModule().commandArgs().front().c_str());
				
				::SendMessage(hOther, WM_COPYDATA, 0, (LPARAM)(LPVOID)&cmdLine);
			}
		}				
	}
	else
	{
		INI().LoadData();
		
		CMessageLoop theLoop;
		_Module.AddMessageLoop(&theLoop);
		{	
			HaliteWindow wndMain(WMU_ARE_YOU_ME);
			if (wndMain.CreateEx() == NULL)
				return 1;
			
			oneInstance.release();
			
			#ifndef NDEBUG
			global_debugDialog_.Create(wndMain);
			global_debugDialog_.ShowWindow(false);
			#endif
			
			if (!globalModule().commandArgs().empty())
				wndMain.ProcessFile(globalModule().commandArgs().front().c_str());
			
			wndMain.SetWindowText(L"Halite");
			wndMain.MoveWindow(
				INI().windowConfig().rect.left,
				INI().windowConfig().rect.top,
				INI().windowConfig().rect.right-INI().windowConfig().rect.left,
				INI().windowConfig().rect.bottom-INI().windowConfig().rect.top,
				false);
			
			wndMain.SetIcon(LoadIcon(hInstance, MAKEINTRESOURCE(IDR_APP_ICON)), false);
			wndMain.ShowWindow(nCmdShow);
			
			nRet = theLoop.Run();				
		}	
		_Module.RemoveMessageLoop();
		
		if (INI().splashConfig().showMessage)
		{
			SplashDialog splDlg;
			splDlg.DoModal();
		}
		else
		{
			halite::bittorrent().closeAll();
			halite::bittorrent().shutDownSession();		
		}		
		INI().SaveData();
	}
	}
	
	_Module.Term();
	
	return nRet;
}
