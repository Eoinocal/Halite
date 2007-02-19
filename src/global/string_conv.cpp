
#include <boost/smart_ptr.hpp>

#include "string_conv.hpp"

namespace hal
{

std::wstring mbstowcs(const char* str, size_t length) 
{
	size_t len=::mbstowcs(NULL, str, length);
	boost::scoped_array<wchar_t> buf(new wchar_t[len]);

	len=::mbstowcs(buf.get(), str, length);
	if(len==static_cast<size_t>(-1)) 
		throw std::runtime_error("mbstowcs(): invalid multi-byte character");

	return std::wstring(buf.get(), len);
}

std::string wcstombs(const wchar_t* str, size_t length)//const std::wstring &str) 
{
	size_t len=::wcstombs(NULL, str, 0);
	boost::scoped_array<char> buf(new char[len]);

	len=::wcstombs(buf.get(), str, len);
	if(len==static_cast<size_t>(-1)) 
		throw std::runtime_error("wcstombs(): unable to convert character");

	return std::string(buf.get(), len);
}

} // namespace gbl
