
//         Copyright Eóin O'Callaghan 2006 - 20078.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "UxthemeWrapper.hpp"

namespace hal 
{

uxthemeWrapper& uxtheme()
{
	static uxthemeWrapper ux;
	return ux;
}

};
