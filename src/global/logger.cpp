
#include "logger.hpp"
#include "string_conv.hpp"

namespace hal 
{

#ifdef _UNICODE
Logger<std::wstring, std::wostringstream>& wlog()
{
	static Logger<std::wstring, std::wostringstream> l;
	return l;
}
#endif

Logger<std::string, std::ostringstream>& log()
{
	static Logger<std::string, std::ostringstream> l;
	return l;
}

}
