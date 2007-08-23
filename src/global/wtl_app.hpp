
#ifndef GLOBAL_WTL_APP
#define GLOBAL_WTL_APP

#include <string>
#include <vector>
#include <sstream>

#include <boost/signals.hpp>
#include <boost/function.hpp>

#include <boost/static_assert.hpp>
#include <boost/filesystem/path.hpp>

namespace hal
{

class app_module
{
public:	
	const std::wstring& exe_string() const { return exe_string_; }
	const boost::filesystem::wpath& exe_path() const { return exe_path_; }
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
	std::vector<std::wstring> command_args_;	
};

app_module& app();

}

#endif // GLOBAL_WTL_APP