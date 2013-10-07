
//         Copyright Eóin O'Callaghan 2006 - 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define WINVER				0x0600
#define _WIN32_WINNT		0x0600
#define _WIN32_IE			0x0700
#define _RICHEDIT_VER		0x0200

#ifndef VC_EXTRALEAN
#	define VC_EXTRALEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#define HALITE_SENDING_CMD 68816889

#pragma warning (push)
#pragma warning (disable : 4996)
#	define _ATL_NO_AUTOMATIC_NAMESPACE
#	define _WTL_NO_AUTOMATIC_NAMESPACE
#	define _ATL_USE_DDX_FLOAT

#	include <winsock2.h>
#	include <shellapi.h>
#	include <atlbase.h>
#	include <atlapp.h>

extern WTL::CAppModule _Module;

#	include <atlwin.h>
#	include <atlframe.h>
#	include <atlmisc.h>
#	include <atlcrack.h>
#	include <atldlgs.h>
#	include <atlsplit.h>
#	include <atlctrls.h>
#	include <atlctrlw.h>
#	include <atlctrlx.h>
#	include <atlddx.h>
#	include <atlscrl.h>
#	include <atltheme.h>
#	include <atldwm.h>

#	include "AtlAutosizeDlg.h"
#	include "CSSFileDialog.hpp"
#pragma warning (pop)

//#include <stlsoft/util/nulldef.h>

#include "../res/resource.h"

// Include very common C++ and Boost libraries

#include <string>
#include <vector>

#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <map>
#include <algorithm>
#include <string>
#include <vector>

#include <boost/regex.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/signals2.hpp>
#include <boost/function.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>

#include <boost/xpressive/xpressive.hpp>

#pragma warning (push)
#pragma warning (disable : 4099)
#pragma warning (disable : 4267)
#pragma warning (disable : 4244)
#	include <boost/archive/text_woarchive.hpp>
#	include <boost/archive/text_wiarchive.hpp>
#	include <boost/archive/binary_woarchive.hpp>
#	include <boost/archive/binary_wiarchive.hpp>
#	include <boost/archive/text_oarchive.hpp>
#	include <boost/archive/text_iarchive.hpp>
#	include <boost/archive/binary_oarchive.hpp>
#	include <boost/archive/binary_iarchive.hpp>
#	include <boost/archive/basic_xml_archive.hpp>
#	include <boost/archive/xml_woarchive.hpp>
#	include <boost/archive/xml_wiarchive.hpp>
#	include <boost/archive/xml_oarchive.hpp>
#	include <boost/archive/xml_iarchive.hpp>

#	include <boost/serialization/version.hpp>
#	include <boost/serialization/vector.hpp>
#	include <boost/serialization/map.hpp>
#	include <boost/serialization/split_free.hpp>
#	include <boost/serialization/vector.hpp>
#	include <boost/serialization/shared_ptr.hpp>
#pragma warning (pop)

#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
