
#include <fstream>
#include <boost/smart_ptr.hpp>

#include "string_conv.hpp"
//#include "utf8.hpp"
#include "unicode.hpp"
#include "logger.hpp"

namespace hal
{

std::wstring mbstowcs(const std::string& ustr) 
{
	std::wstring wstr;
	
	try
	{
	unicode::transcode<unicode::utf8, unicode::wchar_encoding>(
	   ustr.begin(),
	   ustr.end(),
	   std::insert_iterator<std::wstring>(wstr, wstr.end())
	);
//	wstr = utf8_wchar(ustr);
	return wstr;
	
	}
	catch (const unicode::unicode_error& e)
	{
		log_file(L"mbstowcs unicode_error- ");
		log_file(e.what());
		log_file(L" : ");
		log_file(ustr);
		log_file(L"\r\n");
	}
	catch (const std::exception& e)
	{
		log_file(L"mbstowcs std::exception- ");
		log_file(e.what());
		log_file(L" : ");
		log_file(ustr);
		log_file(L"\r\n");
	}

	return L"";
}

std::string wcstombs(const std::wstring& wstr) 
{	
	std::string ustr;
	
	try
	{
	unicode::transcode<unicode::wchar_encoding, unicode::utf8>(
	   wstr.begin(),
	   wstr.end(),
	   std::insert_iterator<std::string>(ustr, ustr.end())
	);
//	ustr = wchar_utf8(wstr);
	return ustr;
	
	}
	catch (const unicode::unicode_error& e)
	{
		log_file(L"mbstowcs unicode_error- ");
		log_file(e.what());
		log_file(L" : ");
		log_file(wstr);
		log_file(L"\r\n");
	}
	catch (const std::exception& e)
	{
		log_file(L"mbstowcs std::exception- ");
		log_file(e.what());
		log_file(L" : ");
		log_file(wstr);
		log_file(L"\r\n");
	}
	
	return "";	
}

} // namespace hal
