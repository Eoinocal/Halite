
#ifndef GLOBAL_STRING_CONV
#define GLOBAL_STRING_CONV

#include <string>
#include <boost/array.hpp>

#include "unicode.hpp"

namespace hal
{

std::wstring mbstowcs(const char* str, size_t len);

inline std::wstring mbstowcs(const std::string& ustr) 
{
	std::wstring wstr;
	
	unicode::transcode<unicode::utf8, unicode::wchar_encoding>(
	   ustr.begin(),
	   ustr.end(),
	   std::insert_iterator<std::wstring>(wstr, wstr.end())
	);
	
	return wstr;
}

std::string wcstombs(const wchar_t* str, size_t length);

inline std::string wcstombs(const std::wstring& wstr) 
{	
	std::string ustr;
	
	unicode::transcode<unicode::wchar_encoding, unicode::utf8>(
	   wstr.begin(),
	   wstr.end(),
	   std::insert_iterator<std::string>(ustr, ustr.end())
	);
	
	return ustr;
}

template <typename U, typename T>
inline U to_gen_str(const T& str) { return str; }

template <>
inline std::string to_gen_str<std::string, std::string>(const std::string& str) { return str; }
template <>
inline std::string to_gen_str<std::string, std::wstring>(const std::wstring& str) { return wcstombs(str); }

template <>
inline std::wstring to_gen_str<std::wstring, std::string>(const std::string& str) { return mbstowcs(str); }
template <>
inline std::wstring to_gen_str<std::wstring, std::wstring>(const std::wstring& str) { return str; }

inline std::string to_str(const std::string& str) { return str; }
inline std::string to_str(const std::wstring& str) { return wcstombs(str); }

inline std::wstring to_wstr(const std::wstring& str) { return str; }
inline std::wstring to_wstr(const std::string& str) { return mbstowcs(str); }

/*inline std::string to_str(const boost::array<char>& buf) { return std::string(buf.get(), buf.size()); }
inline std::string to_str(const boost::array<char>& buf) { return std::string(buf.get(), buf.size()); }

inline std::string to_str(const boost::array<char>& buf) { return std::string(buf.get(), buf.size()); }
inline std::string to_str(const boost::array<char>& buf) { return std::string(buf.get(), buf.size()); }
*/

} // namespace gbl

#endif // GLOBAL_STRING_CONV