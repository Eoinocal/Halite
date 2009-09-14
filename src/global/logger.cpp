
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

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
