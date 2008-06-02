
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GLOBAL_WTL_APP
#define GLOBAL_WTL_APP

#include <string>
#include <vector>
#include <sstream>

#include <boost/signals.hpp>
#include <boost/function.hpp>

#include <boost/static_assert.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace hal
{

class app_impl;

class app_module
{
public:	
	const std::wstring& exe_string() const;
	const boost::filesystem::wpath& exe_path() const;
	const boost::filesystem::wpath& initial_path() const;
	const boost::filesystem::wpath& working_directory() const;
	
	const std::vector<std::wstring>& command_args() const;
	
	std::wstring res_wstr(unsigned uID);	
	void res_set_dll(std::wstring dll);
	void res_revert();
	std::pair<void*,size_t> res_find_lock(unsigned name, unsigned type);
	
	friend app_module& app();
	
private:
	boost::scoped_ptr<app_impl> pimpl;
	
	app_module();
};

app_module& app();

}

#endif // GLOBAL_WTL_APP
