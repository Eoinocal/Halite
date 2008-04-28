
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
	namespace sl = boost::serialization;

	using std::pair;
	using std::make_pair;	
	using boost::tuple;

	typedef boost::int64_t size_type;
	typedef boost::recursive_mutex mutex_t;
	typedef boost::thread thread_t;
}
