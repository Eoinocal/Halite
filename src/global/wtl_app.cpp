
#include <boost/array.hpp>

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#define _WIN32_IE 0x0500
#define _RICHEDIT_VER 0x0200
#define VC_EXTRALEAN

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include "wtl_app.hpp"
#include "string_conv.hpp"

namespace hal
{

app_module::app_module()
{
	LPWSTR *szArglist; int nArgs;		
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	
	if( NULL == szArglist )
	{
	}
	else
	{
		exe_string_  = szArglist[0];
		exe_path_ = boost::filesystem::path(wcstombs(exe_string_));
		
		for(int i=1; i<nArgs; ++i) 
			command_args_.push_back(szArglist[i]);
	}		
	LocalFree(szArglist);	
}

std::wstring app_module::load_res_wstring(unsigned uID)
{
	const int buffer_size = 512;
	boost::array<wchar_t, buffer_size> buffer;
	::LoadString(_Module.GetResourceInstance(), uID, buffer.elems, buffer_size);
	
	return std::wstring(buffer.elems);
}

app_module& app()
{
	static app_module app;
	return app;
}

} // namespace gbl
