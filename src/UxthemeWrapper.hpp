
#pragma once

#include "stdAfx.hpp"

namespace halite 
{

class uxthemeWrapper
{
public:
	typedef BOOL (WINAPI *ISAPPTHEMEDPROC)();
	typedef HRESULT (WINAPI *DRAWPARENTTHEMEBACKGROUND)(HWND, HDC, RECT*);
		
	uxthemeWrapper() :
		pIsAppThemed(0),
		pDrawThemeParentBackground(0)
	{			
		hinstDll = ::LoadLibrary(_T("UxTheme.dll"));
		if (hinstDll)
		{
			pIsAppThemed = 
			  (ISAPPTHEMEDPROC) ::GetProcAddress(hinstDll, "IsAppThemed");

			pDrawThemeParentBackground = 
			  (DRAWPARENTTHEMEBACKGROUND) ::GetProcAddress(hinstDll, "DrawThemeParentBackground");
			  
//			::MessageBox(0, (wformat(L"%1%, %2%") % pIsAppThemed % pDrawThemeParentBackground).str().c_str(), L"Result", 0);
		}
	}
	
	~uxthemeWrapper()
	{
		::FreeLibrary(hinstDll);
	}
	
	ISAPPTHEMEDPROC pIsAppThemed;
	DRAWPARENTTHEMEBACKGROUND pDrawThemeParentBackground;
	
private:
	HMODULE hinstDll;	
};

uxthemeWrapper& uxtheme();

};
