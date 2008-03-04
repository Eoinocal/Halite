
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#define _WIN32_IE 0x0500
#define _RICHEDIT_VER 0x0200
#define VC_EXTRALEAN

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define HALITE_SENDING_CMD 68816889

//	#define _WTL_NO_AUTOMATIC_NAMESPACE

#include <winsock2.h>
#include <shellapi.h>
#include <atlbase.h>
#include <atlapp.h>

extern WTL::CAppModule _Module;
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
#include <atlscrl.h>

#include "AtlAutosizeDlg.h"
//#include <stlsoft/util/nulldef.h>

#include "global/wtl_app.hpp"
#include "global/string_conv.hpp"

template<class T>
class RedrawLock
{
public:
	RedrawLock(T& window) :
		window_(window)
	{
		window_.SetRedraw(false);
	}
	
	~RedrawLock()
	{
		unlock();
	}
	
	void unlock()
	{
		window_.SetRedraw(true);
		window_.InvalidateRect(NULL, true);
	}
	
private:
	T& window_;
};

// Include very common C++ and Boost libraries

#include <string>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/noncopyable.hpp>
#include <boost/tuple/tuple.hpp>

#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

using std::string;
using std::wstring;

using boost::lexical_cast;
using boost::array;
using boost::format;
using boost::wformat;
using boost::bind;
using boost::thread;
using boost::shared_ptr;
using boost::scoped_ptr;
using boost::filesystem::path;
using boost::filesystem::wpath;
using boost::noncopyable;

template<class Archive>
void serialize(Archive& ar, WTL::CRect& rect, const unsigned int version)
{
	ar & BOOST_SERIALIZATION_NVP(rect.top);
	ar & BOOST_SERIALIZATION_NVP(rect.bottom);
	ar & BOOST_SERIALIZATION_NVP(rect.left);
	ar & BOOST_SERIALIZATION_NVP(rect.right);
}

namespace hal
{
	
	namespace fs = boost::filesystem;
	namespace pt = boost::posix_time;

	using std::pair;
	using std::make_pair;
	
	using boost::tuple;
	
}

#define foreach BOOST_FOREACH
