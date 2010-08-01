
//         Copyright Eóin O'Callaghan 2010 - 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GLOBAL_VERSIONED_FILE
#define GLOBAL_VERSIONED_FILE

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "work_file.hpp"

namespace hal 
{

class file_header
{
public:	
	file_header(const boost::uuids::uuid& u, int v) :
		uuid_(u),
		version_(v)
	{}

	std::pair<bool, int> check_header(std::wistream& is)
	{
		boost::uuids::uuid uuid;
		is >> uuid;

		if (uuid != uuid_)
			return make_pair(false, -1);

		int version;
		is >> version;

		return make_pair(true, version);
	}

	void add_header(std::wostream& os)
	{
		os << uuid_ << std::endl;
		os << version_ << std::endl;;
	}

private:	
	boost::uuids::uuid uuid_;
	int version_;
};

class versioned_file
{
public:
	versioned_file(const std::wstring& f, const boost::uuids::uuid& u, int v) :
		file_(f),
		header_(u, v),
		loaded_version_(-1)
	{}
		
	shared_wostream_ptr wostream()
	{
		shared_wostream_ptr owfs_p = file_.wostream();

		header_.add_header(*owfs_p);

		return owfs_p;
	}

	boost::optional<shared_wistream_ptr> wistream()
	{
		shared_wistream_ptr iwfs_p = file_.wistream();

		std::pair<bool, int> p = header_.check_header(*iwfs_p);

		if (p.first)
		{
			loaded_version_ = p.second;

			return boost::optional<shared_wistream_ptr>(iwfs_p);
		}
		else
		{
			loaded_version_ = -1;

			return boost::optional<shared_wistream_ptr>();
		}
	}

	int loaded_version() const { return loaded_version_; }

	boost::filesystem::wpath main_file() const
	{
		return file_.main_file();
	}

	boost::filesystem::wpath working_file() const
	{
		return file_.working_file();
	}

private:
	work_file file_;
	file_header header_;
	int loaded_version_;
};

} // namespace hal

#endif // GLOBAL_VERSIONED_FILE
