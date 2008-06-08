
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/thread/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace hal
{	
	namespace fs = boost::filesystem;
	namespace pt = boost::posix_time;
	namespace xp = boost::xpressive;

	using std::wstring;
	using std::string;

	using std::pair;
	using std::make_pair;	

	typedef boost::int64_t size_type;
	typedef boost::mutex mutex_t;
	typedef boost::thread thread_t;

	using boost::tuple;
	using boost::shared_ptr;
	using boost::bind;

	using boost::wformat;
	using boost::format;

	using fs::wpath;
	using fs::path;
}

#define foreach BOOST_FOREACH
