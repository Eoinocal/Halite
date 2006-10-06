
#include "stdAfx.hpp"
#include "Halite.hpp"
#include "GlobalIni.hpp"
#include "ini/Window.hpp"

using namespace std;
using namespace boost;

#include "HaliteWindow.hpp"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//HRESULT hRes = ::CoInitialize(NULL);
	//assert (SUCCEEDED(hRes));	

	boost::filesystem::path::default_name_check(boost::filesystem::native);
	
	INI().LoadData();
	
	int nRet;
	{
		AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	
		HINSTANCE hInstRich = ::LoadLibrary(CRichEditCtrl::GetLibraryName());
	   
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
					INI().windowConfig().rect.left,
					INI().windowConfig().rect.top,
					INI().windowConfig().rect.right-INI().windowConfig().rect.left,
					INI().windowConfig().rect.bottom-INI().windowConfig().rect.top,
					false);
				wndMain.ShowWindow(nCmdShow);
				
				nRet = theLoop.Run();
				
			}	
		}	_Module.RemoveMessageLoop();
		_Module.Term();
	}//::CoUninitialize();	
			
//	halite::closeTorrents();
//	bool success = halite::closeDown();
//	assert(success);	

	INI().SaveData();
	
	return nRet;
}