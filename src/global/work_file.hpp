
//         Copyright Eóin O'Callaghan 2010 - 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GLOBAL_WORKING_FILE
#define GLOBAL_WORKING_FILE

#include <boost/smart_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/smart_ptr.hpp>

namespace hal 
{

typedef boost::shared_ptr<std::wostream> shared_wostream_ptr;
typedef boost::shared_ptr<std::wistream> shared_wistream_ptr;

class work_file
{
public:	
	work_file(std::wstring filename);
	~work_file();
	
	shared_wostream_ptr wostream();
	shared_wistream_ptr wistream();

	boost::filesystem::wpath main_file() const;
	boost::filesystem::wpath working_file() const;

private:	
	boost::filesystem::wpath generate_backup_name();

	boost::filesystem::wpath main_file_;
	boost::filesystem::wpath working_file_;
	std::wstring filename_;
};


} // namespace hal

#endif // GLOBAL_WORKING_FILE
