#pragma once

#define WINVER 0x0500
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0501
#define _RICHEDIT_VER 0x0100

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
#include "ssfiledialog.hpp"

/*
#include <exception>
#include <string>
#include <vector>
#include <map>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/tss.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>  
*/
