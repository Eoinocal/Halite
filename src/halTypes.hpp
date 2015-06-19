
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/smart_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "global/string_conv.hpp"
#include "global/load_file.hpp"

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
	typedef boost::mutex::scoped_try_lock scoped_try_lock;
	typedef mutex_t::scoped_lock unique_lock_t;

	typedef boost::shared_lock<boost::shared_mutex> shared_lock;
	typedef boost::unique_lock<boost::shared_mutex> unique_lock;
	typedef boost::upgrade_lock<boost::shared_mutex> upgrade_lock;
	typedef boost::upgrade_to_unique_lock<boost::shared_mutex> upgrade_to_unique_lock;

	typedef boost::thread thread_t;

	typedef boost::uuids::uuid uuid;

	using boost::tuple;
	using boost::shared_ptr;
	//using boost::bind;
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
	
	inline string path_to_utf8(const wpath& wp)
	{
		return hal::to_utf8(wp.wstring());
	}

	inline wpath path_from_utf8(const path& p)
	{
		return wpath(p.string());
	}
}
