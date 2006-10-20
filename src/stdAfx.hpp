#pragma once

#define WINVER 0x0500
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0501
#define _RICHEDIT_VER 0x0200
#define VC_EXTRALEAN

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;
#define _ATL_USE_DDX_FLOAT

#include <atlwin.h>
#include <atlframe.h>
#include <atlmisc.h>
#include <atlcrack.h>
#include <atldlgs.h>
#include <atlsplit.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atlddx.h>

#include "..\res\resource.h"

// Include very common C++ and Boost libraries

#include <string>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/smart_ptr.hpp>

using std::string;
using std::wstring;
using std::pair;
using std::make_pair;

using boost::lexical_cast;
using boost::format;
using boost::wformat;
using boost::bind;
using boost::thread;
using boost::shared_ptr;
using boost::scoped_ptr;

