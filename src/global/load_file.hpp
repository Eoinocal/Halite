
//         Copyright Eóin O'Callaghan 2010 - 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// From http://insanecoding.blogspot.ie/2011/11/how-to-read-in-file-in-c.html

#ifndef GLOBAL_LOAD_FILE
#define GLOBAL_LOAD_FILE

#include <fstream>
#include <string>
#include <cerrno>
#include <boost\filesystem\path.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace hal 
{

template<typename C>
C load_file(const boost::filesystem::path file, boost::system::error_code& ec)
{	
	C contents;
	fs::ifstream in(file, std::ios::in | std::ios::binary);

	if (in)
	{
		in.seekg(0, std::ios::end);
		contents.resize(static_cast<size_t>(in.tellg()));
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
	}
	else
		ec.assign(errno, boost::system::system_category());
	
	return contents;
}

template<typename C>
C load_file(const boost::filesystem::path file)
{	
	boost::system::error_code ec;
	C contents(load_file<C>(file, ec));

	if (ec)
		throw boost::system::system_error(ec);
	else	
		return contents;
}

template<typename C>
C load_file(const wchar_t* file)
{
	return load_file<C>(path(file));
}

} // namespace hal

#endif // GLOBAL_WORKING_FILE
