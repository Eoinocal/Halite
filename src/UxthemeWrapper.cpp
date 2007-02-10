
#include "UxthemeWrapper.hpp"

namespace hal 
{

uxthemeWrapper& uxtheme()
{
	static uxthemeWrapper ux;
	return ux;
}

};
