
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
	void load_data();
	void save_data();
	
	bool save(boost::filesystem::path location, std::string data);
	bool save(boost::filesystem::path location, tinyxml::node* data);
	
	tinyxml::node* load(boost::filesystem::path location);
	
	friend ini_file& ini();
	
private:
	ini_file(std::string filename);
	
	boost::scoped_ptr<ini_impl> pimpl;
};

ini_file& ini();

}

class ini_base
{
public:
	virtual void load_data() = 0;
	virtual void save_data() = 0;
};

#endif // GLOBAL_INI_FILE
