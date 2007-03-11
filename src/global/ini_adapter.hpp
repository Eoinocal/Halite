
#ifndef GLOBAL_INI_ADAPTER
#define GLOBAL_INI_ADAPTER

#include <iostream>
#include <boost/filesystem/path.hpp>

#include "ini.hpp"

namespace hal 
{

class ini_adapter
{
public:	
	ini_adapter(boost::filesystem::path loc, ini_file& ini = hal::ini()) :
		ini_(ini),
		location_(loc)
	{}

	void load_stream_data(std::ostream& data);
	void save_stream_data(std::istream& data);
	
	void load_stream_data(std::wostream& data);
	void save_stream_data(std::wistream& data);
	
private:
	ini_file& ini_;
	boost::filesystem::path location_;
};

} // namespace hal

#endif // GLOBAL_INI_ADAPTER
