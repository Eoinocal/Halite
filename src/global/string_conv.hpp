
// Copyright (c) 2007 Eóin O'Callaghan
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef GLOBAL_STRING_CONV
#define GLOBAL_STRING_CONV

#include <string>
#include <boost/array.hpp>
#include "unicode.hpp"

namespace hal
{

inline std::string to_utf8(const std::wstring& wstr) 
{ 
	std::string ustr;

	unicode::transcode<unicode::wchar_encoding, unicode::utf8>(
	   wstr.begin(),
	   wstr.end(),
	   std::insert_iterator<std::string>(ustr, ustr.end())
	);

	return ustr;
}

inline std::wstring from_utf8(const std::string& ustr) 
{ 
	std::wstring wstr;
	
	unicode::transcode<unicode::utf8, unicode::wchar_encoding>(
	   ustr.begin(),
	   ustr.end(),
	   std::insert_iterator<std::wstring>(wstr, wstr.end())
	);

	return wstr; 
}

inline std::wstring from_ascii(const std::string& s)
{
	std::wstring ret;

	for (const char *i = &s[0], *end = &s[0] + s.size(); i < end;)
	{
		wchar_t c = '.';
		int result = std::mbtowc(&c, i, end - i);
		if (result > 0) i += result;
		else ++i;
		ret += c;
	}

	return ret;
}

inline std::wstring from_utf8_safe(const std::string& s)
{
	try
	{
		return from_utf8(s);
	}
	catch (std::exception&)
	{
		return from_ascii(s);
	}
}

inline std::wstring to_wstr(const std::wstring& s)
{
		return s;
}

inline std::wstring to_wstr(const std::string& s)
{
		return from_utf8_safe(s);
}

} // namespace hal

#endif // GLOBAL_STRING_CONV
