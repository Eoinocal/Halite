
#ifndef GLOBAL_STRING_CONV
#define GLOBAL_STRING_CONV

#include <string>
#include <boost/array.hpp>

#include "utf8.hpp"
#include "unicode.hpp"

namespace hal
{

inline std::wstring mbstowcs(const std::string& ustr) 
{
	std::wstring wstr;
	
/*	unicode::transcode<unicode::utf8, unicode::wchar_encoding>(
	   ustr.begin(),
	   ustr.end(),
	   std::insert_iterator<std::wstring>(wstr, wstr.end())
	);
*/	

	wstr = utf8_wchar(ustr);
	return wstr;
}

inline std::string wcstombs(const std::wstring& wstr) 
{	
	std::string ustr;
	
/*	unicode::transcode<unicode::wchar_encoding, unicode::utf8>(
	   wstr.begin(),
	   wstr.end(),
	   std::insert_iterator<std::string>(ustr, ustr.end())
	);
*/
	
	ustr = wchar_utf8(wstr);	
	return ustr;
}

template <typename U, typename T>
inline U to_gen_str(const T& str) 
	{ return str; }

template <>
inline std::string to_gen_str<std::string, std::string>(const std::string& str) 
	{ return str; }

template <>
inline std::string to_gen_str<std::string, std::wstring>(const std::wstring& str) 
	{ return wcstombs(str); }

template <>
inline std::wstring to_gen_str<std::wstring, std::string>(const std::string& str) 
	{ return mbstowcs(str); }
	
template <>
inline std::wstring to_gen_str<std::wstring, std::wstring>(const std::wstring& str) 
	{ return str; }

inline std::string to_str(const std::string& str) 
	{ return str; }
	
inline std::string to_str(const std::wstring& str) 
	{ return wcstombs(str); }

inline std::string to_utf8(const std::wstring& str) 
	{ return wcstombs(str); }

inline std::wstring to_wstr(const std::wstring& str) 
	{ return str; }
	
inline std::wstring to_wstr(const std::string& str) 
	{ return mbstowcs(str); }

} // namespace hal

#endif // GLOBAL_STRING_CONV