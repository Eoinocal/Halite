
//         Copyright E�in O'Callaghan 2010 - 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include "wtl_app.hpp"
#include "logger.hpp"
#include "string_conv.hpp"
#include "work_file.hpp"

namespace hal 
{

work_file::work_file(const std::wstring& filename) :
	main_file_(app().get_working_directory()/filename),
	working_file_(app().get_working_directory()/(filename + L".working")),
	filename_(filename)
{		
	if (boost::filesystem::exists(working_file_))
		boost::filesystem::rename(working_file_, generate_backup_name());

	if (boost::filesystem::exists(main_file_))
		boost::filesystem::copy_file(main_file_, working_file_);
}

work_file::~work_file()
{
	if (boost::filesystem::exists(working_file_))
	{
		boost::filesystem::remove(main_file_);
		boost::filesystem::copy_file(working_file_, main_file_);

		if (boost::filesystem::last_write_time(main_file_) >=
				boost::filesystem::last_write_time(working_file_))
		{
			boost::filesystem::remove(working_file_);
		}
	}
}

shared_wostream_ptr work_file::wostream()
{		
	if (boost::filesystem::exists(working_file_))
	{
		boost::filesystem::remove(main_file_);
		boost::filesystem::copy_file(working_file_, main_file_);
	}

	shared_wostream_ptr owfs_p(new boost::filesystem::wofstream(working_file_));

	owfs_p->imbue(std::locale(owfs_p->getloc(),
		new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>));

	return owfs_p;
}

shared_wistream_ptr work_file::wistream()
{
	shared_wistream_ptr iwfs_p(new boost::filesystem::wifstream(working_file_));


	iwfs_p->imbue(std::locale(iwfs_p->getloc(),
		new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>));

	return iwfs_p;
}

boost::filesystem::wpath work_file::generate_backup_name()
{
	std::wstringstream sstr;
	boost::posix_time::wtime_facet* facet = new boost::posix_time::wtime_facet(L"%Y-%m-%d.%H-%M-%S");
	sstr.imbue(std::locale(std::cout.getloc(), facet));
	sstr << boost::posix_time::second_clock::universal_time();

	return app().get_working_directory()/(filename_ + L"." + sstr.str());
}

boost::filesystem::wpath work_file::main_file() const { return main_file_; }
boost::filesystem::wpath work_file::working_file() const { return working_file_; }

} // namespace hal
