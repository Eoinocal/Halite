
#include "UxthemeWrapper.hpp"

namespace halite 
{

uxthemeWrapper& uxtheme()
{
	static uxthemeWrapper ux;
	return ux;
}

};
