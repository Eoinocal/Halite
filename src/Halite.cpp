
#include "stdAfx.hpp"
#include "WinAPIMutex.hpp"

#include "GlobalIni.hpp"
#include "ini/Window.hpp"
#include "ini/General.hpp"

#include "HaliteWindow.hpp"
#include "SplashDialog.hpp"

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
	INI().LoadData();

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);
   
	int nRet;	
	HRESULT hRes = _Module.Init(NULL, hInstance);
	assert (SUCCEEDED(hRes));	
	{				
	WinAPIMutex oneInstance(HALITE_GUID);
	
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
		CMessageLoop theLoop;
		_Module.AddMessageLoop(&theLoop);
		{						
			HaliteWindow wndMain(WMU_ARE_YOU_ME);
			if (wndMain.CreateEx() == NULL)
				return 1;
			
			oneInstance.release();
			
			if (!globalModule().commandArgs().empty())
				wndMain.ProcessFile(globalModule().commandArgs().front().c_str());
			
			wndMain.SetWindowText(L"Halite");
			wndMain.MoveWindow(
				INI().windowConfig().rect.left,
				INI().windowConfig().rect.top,
				INI().windowConfig().rect.right-INI().windowConfig().rect.left,
				INI().windowConfig().rect.bottom-INI().windowConfig().rect.top,
				false);
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
	}
	}
	
	_Module.Term();
	
	INI().SaveData();
	
	return nRet;
}
