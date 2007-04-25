
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

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#define BOOST_UTF8_BEGIN_NAMESPACE \
     namespace boost { namespace filesystem { namespace detail {

#define BOOST_UTF8_END_NAMESPACE }}}
#define BOOST_UTF8_DECL BOOST_FILESYSTEM_DECL

#include <boost/detail/utf8_codecvt_facet.hpp>

namespace hal
{

app_module::app_module()
{
	LPWSTR *szArglist; int nArgs;		
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	
	std::locale global_loc = std::locale();
	std::locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet);
	boost::filesystem::wpath_traits::imbue(loc);
	
	if (NULL == szArglist)
	{
	}
	else
	{
		exe_string_  = szArglist[0];
		exe_path_ = boost::filesystem::wpath(exe_string_);
		
		for (int i=1; i<nArgs; ++i) 
			command_args_.push_back(szArglist[i]);
	}		
	LocalFree(szArglist);	
	
	HMODULE hMod = ::LoadLibraryEx(L"Template.dll", 0, LOAD_LIBRARY_AS_DATAFILE);
	_Module.SetResourceInstance(reinterpret_cast<HINSTANCE>(hMod));
}

std::wstring app_module::res_wstr(unsigned uID)
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
