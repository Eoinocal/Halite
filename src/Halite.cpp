
#include "WTL.hpp"
#include "stdafx.h"
#include "Halite.h"

using namespace std;
using namespace boost;
			
#include "HaliteWindow.hpp"

CAppModule _Module;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//HRESULT hRes = ::CoInitialize(NULL);
	//assert (SUCCEEDED(hRes));	

	boost::filesystem::path::default_name_check(boost::filesystem::native);
	
	int nRet;
	{
		AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	
		HRESULT hRes = _Module.Init(NULL, hInstance);
		assert (SUCCEEDED(hRes));	
		{	
			CMessageLoop theLoop;
			_Module.AddMessageLoop(&theLoop);
			{	
				HaliteWindow wndMain;
				if (wndMain.CreateEx() == NULL)
					return 1;
				
				wndMain.SetWindowText(L"Halite");		
				wndMain.MoveWindow(
					INI->haliteWindow.rect.left,
					INI->haliteWindow.rect.top,
					INI->haliteWindow.rect.right-INI->haliteWindow.rect.left,
					INI->haliteWindow.rect.bottom-INI->haliteWindow.rect.top,
					false);
				wndMain.ShowWindow(nCmdShow);
				
				nRet = theLoop.Run();
			}	
		}	_Module.RemoveMessageLoop();
		_Module.Term();
	}//::CoUninitialize();	
			
	halite::closeTorrents();
	bool success = halite::closeDown();
	assert(success);	
	
	return nRet;
}