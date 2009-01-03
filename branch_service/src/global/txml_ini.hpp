
//         Copyright Eóin O'Callaghan 2008 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef AUX_TXML_INI_HPP_INCLUDED
#define AUX_TXML_INI_HPP_INCLUDED

#include <map>
#include <string>
#include <boost/smart_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace hal 
{ 

namespace xml
{
class node;
} 

class ini_impl;

class txml_ini
{
public:	
	txml_ini();
	txml_ini(boost::filesystem::wpath filename);

	~txml_ini();

	void init(boost::filesystem::wpath filename);
	
	bool load_data();
	bool save_data();
	
//	bool save(boost::filesystem::path location, std::string data);
	bool save(boost::filesystem::path location, xml::node* data);
	
	xml::node* load(boost::filesystem::path location);
		
private:	
	boost::scoped_ptr<ini_impl> pimpl;
};

} // namespace aux

#endif // AUX_TXML_INI_HPP_INCLUDED
