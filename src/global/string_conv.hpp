
#ifndef GLOBAL_STRING_CONV
#define GLOBAL_STRING_CONV

#include <string>
#include <boost/array.hpp>

namespace hal
{

std::wstring mbstowcs(const std::string& ustr);
std::string wcstombs(const std::wstring& wstr);

inline std::string wstr_to_str(const std::wstring& wstr) 
{ 		
	std::string str;
	str.reserve(wstr.length());
	
	for (std::string::const_iterator i=str.begin(); i!=str.end(); ++i)
	{
		wchar_t wide_char = *i;
		char narrow_char = 0;
		
		narrow_char = *(reinterpret_cast<char*>(&wide_char));
		str.push_back(narrow_char);
	}
	
	return str;
}

inline std::string to_utf8(const std::wstring& str) 
{ 
	return wcstombs(str); 
}

inline std::wstring str_to_wstr(const std::string& str) 
{ 		
	std::wstring wstr;
	wstr.reserve(str.length());
	
	for (std::string::const_iterator i=str.begin(); i!=str.end(); ++i)
	{
		char narrow_char = *i;
		wchar_t wide_char = 0;
		
		*(reinterpret_cast<char*>(&wide_char)) = narrow_char;
		wstr.push_back(wide_char);
	}
	
	return wstr;
}

inline std::wstring safe_from_utf8(const std::string& str) 
{ 		
	try
	{
	
	return mbstowcs(str); 
	
	}
	catch(...)
	{
	
	std::wstring wstr;
	wstr.reserve(str.length());
	
	for (std::string::const_iterator i=str.begin(); i!=str.end(); ++i)
	{
		char narrow_char = *i;
		wchar_t wide_char = 0;
		
		*(reinterpret_cast<char*>(&wide_char)) = narrow_char;
		wstr.push_back(wide_char);
	}
	
	return wstr;
	
	}
}

inline std::wstring from_utf8(const std::string& str) 
{ 
	return mbstowcs(str);  
}

} // namespace hal

#endif // GLOBAL_STRING_CONV