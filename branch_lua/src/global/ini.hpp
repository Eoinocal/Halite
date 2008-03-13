
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GLOBAL_INI_FILE
#define GLOBAL_INI_FILE

#include <boost/smart_ptr.hpp>
#include <boost/filesystem/path.hpp>

namespace tinyxml
{
class node;
}

namespace hal 
{

class ini_impl;

class ini_file
{
public:	
	ini_file(std::wstring filename);
	~ini_file();
	
	void load_data();
	void save_data();
	
	bool save(boost::filesystem::path location, std::string data);
	bool save(boost::filesystem::path location, tinyxml::node* data);
	
	tinyxml::node* load(boost::filesystem::path location);
	
	friend ini_file& ini();
	
private:	
	std::auto_ptr<ini_impl> pimpl;
};

ini_file& ini();

} // namespace hal

class ini_base
{
public:
	virtual void load_data() = 0;
	virtual void save_data() = 0;
};

#endif // GLOBAL_INI_FILE
