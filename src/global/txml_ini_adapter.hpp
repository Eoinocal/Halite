
//         Copyright Eóin O'Callaghan 2008 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef AUX_TXML_INI_ADAPTER_HPP
#define AUX_TXML_INI_ADAPTER_HPP

#include <iostream>
#include <boost/filesystem/path.hpp>

#include "ini.hpp"

namespace hal 
{

class txml_ini_adapter
{
public:	
	txml_ini_adapter(boost::filesystem::path loc, ini_file& ini = hal::ini()) :
		ini_(ini)
	{
		locations_.push_back(loc);
	}

	txml_ini_adapter(std::vector<boost::filesystem::path> locs, ini_file& ini = hal::ini()) :
		ini_(ini),
		locations_(locs)
	{}

	bool load_stream_data(std::ostream& data);
	void save_stream_data(std::istream& data);
	
	bool load_stream_data(std::wostream& data);
	void save_stream_data(std::wistream& data);
	
private:
	
	xml::node* get_load_data_node();

	ini_file& ini_;
	
	std::vector<boost::filesystem::path> locations_;
};

} // namespace aux

#endif // AUX_TXML_INI_ADAPTER_HPP
