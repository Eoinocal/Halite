
#include <fstream>
#include <boost/smart_ptr.hpp>

#include "string_conv.hpp"
//#include "utf8.hpp"
#include "unicode.hpp"

namespace hal
{

std::wstring mbstowcs(const std::string& ustr) 
{
	std::wstring wstr;
	
	unicode::transcode<unicode::utf8, unicode::wchar_encoding>(
	   ustr.begin(),
	   ustr.end(),
	   std::insert_iterator<std::wstring>(wstr, wstr.end())
	);

//	wstr = utf8_wchar(ustr);
	return wstr;
}

std::string wcstombs(const std::wstring& wstr) 
{	
	std::string ustr;
	
	unicode::transcode<unicode::wchar_encoding, unicode::utf8>(
	   wstr.begin(),
	   wstr.end(),
	   std::insert_iterator<std::string>(ustr, ustr.end())
	);
	
//	ustr = wchar_utf8(wstr);	
	return ustr;
}

} // namespace hal
