
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GLOBAL_WTL_APP
#define GLOBAL_WTL_APP

#include <string>
#include <vector>
#include <sstream>

#include <boost/function.hpp>

#include <boost/static_assert.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <stlsoft/properties/method_properties.hpp>
#include <stlsoft/util/operator_bool_adaptor.hpp>

namespace hal
{

class app_impl;

class app_module
{
public:	
	const std::wstring& exe_string() const;

	const boost::filesystem::wpath& exe_path() const;
	const boost::filesystem::wpath& initial_path() const;

	const boost::filesystem::wpath& get_working_directory() const;
	void set_working_directory(const boost::filesystem::wpath& p);
	
	const boost::optional<boost::filesystem::wpath>& get_local_appdata() const;
	const boost::optional<boost::filesystem::wpath>& get_my_documents() const;
	
	const std::vector<std::wstring>& command_args() const;
	
	std::wstring res_wstr(unsigned uID);	
	std::pair<void*,size_t> res_find_lock(unsigned name, unsigned type);

#	if (_ATL_VER > 0x0700)
	void res_set_dll(std::wstring dll);
	void res_revert();
#	endif // (_ATL_VER > 0x0700)
	
	friend app_module& app();

	STLSOFT_METHOD_PROPERTY_GET_EXTERNAL(const boost::optional<boost::filesystem::wpath>&, app_module, 
		get_local_appdata, local_appdata);

	STLSOFT_METHOD_PROPERTY_GET_EXTERNAL(const boost::optional<boost::filesystem::wpath>&, app_module, 
		get_my_documents, local_my_doc_);

	STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(const boost::filesystem::wpath&, const boost::filesystem::wpath&, 
		app_module, get_working_directory, set_working_directory, working_directory);
	
private:
	boost::scoped_ptr<app_impl> pimpl_;
	
	app_module();
};

app_module& app();

}

#endif // GLOBAL_WTL_APP
