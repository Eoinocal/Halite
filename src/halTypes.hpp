
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/smart_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <stlsoft/properties/method_properties.hpp>
#include <stlsoft/util/operator_bool_adaptor.hpp>

#include <loki/Singleton.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

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
	typedef boost::recursive_mutex mutex_t;

	typedef boost::thread thread_t;

	typedef boost::uuids::uuid uuid;

	using boost::tuple;
	using boost::shared_ptr;
	using boost::bind;
	using boost::function;

	using fs::wpath;
	using fs::path;	

	inline boost::wformat wform(const std::wstring & f_string) 
	{
		using namespace boost::io;

		boost::wformat fmter(f_string);
		fmter.exceptions(no_error_bits);

		return fmter;
	}
}

#define foreach BOOST_FOREACH
