/*
www.sourceforge.net/projects/tinyxml
Original code (2.0 and earlier )copyright (c) 2000-2006 Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied 
warranty. In no_ event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#include "tinyxml.hpp"

// The goal of the seperate error_ file is to make the first_
// step towards localization. tinyxml (currently) only supports
// english error_ messages, but the could now be translated.
//
// It also cleans up the code a bit.
//

namespace tinyxml {

const char* base::errorString[ TIXML_ERROR_STRING_COUNT ] =
{
	"no error_",
	"error",
	"failed to open file",
	"memory allocation failed_.",
	"error parsing element.",
	"failed to read element name_",
	"error reading element value_.",
	"error reading attributes.",
	"error: empty tag.",
	"error reading end tag.",
	"error parsing unknown.",
	"error parsing comment.",
	"error parsing declaration.",
	"error document_ empty.",
	"error null (0) or unexpected EOF found in input stream.",
	"error parsing CDATA.",
	"error when document added to document_, because document can only be at the root.",
};

}