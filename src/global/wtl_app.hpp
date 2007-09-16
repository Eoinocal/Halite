
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

class app_module
{
public:	
	const std::wstring& exe_string() const { return exe_string_; }
	const boost::filesystem::wpath& exe_path() const { return exe_path_; }
	const boost::filesystem::wpath& initial_path() const { return initial_path_; }
	const boost::filesystem::wpath& working_directory() const { return working_directory_; }
	
	const std::vector<std::wstring>& command_args() const { return command_args_; }
	
	std::wstring res_wstr(unsigned uID);
	void set_res_dll(std::wstring dll);
	void revert_res();
	std::pair<void*,size_t> find_lock_res(unsigned name, unsigned type);
	
	void set_initial_hinstance(HINSTANCE instance) { instance_ = instance; }
	
	friend app_module& app();
	
private:
	app_module();
	
	HMODULE hmod_;
	HINSTANCE instance_;
	std::wstring exe_string_;
	std::wstring res_dll_;
	
	boost::filesystem::wpath exe_path_;
	boost::filesystem::wpath initial_path_;
	boost::filesystem::wpath working_directory_;
	
	std::vector<std::wstring> command_args_;	
};

app_module& app();

}

#endif // GLOBAL_WTL_APP