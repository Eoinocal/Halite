
//         Copyright Eóin O'Callaghan 2010 - 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/tag.hpp>

#include "halTorrentDefines.hpp"
#include "halTypes.hpp"

namespace hal 
{

namespace libt = libtorrent;
namespace sc = boost::statechart;
namespace mpl = boost::mpl;
namespace mi = boost::multi_index;

class torrent_file
{
public:
	torrent_file()
	{}

	torrent_file(const wstring& on, const wstring& cn) :
		original_name_(on),
		current_name_(cn),
		priority_(1)
	{}

	torrent_file(const fs::wpath& on, const fs::wpath& cn) :
		original_name_(on),
		current_name_(cn),
		priority_(1)
	{}

	void change_filename(const fs::wpath& fn)
	{
		completed_name_ = fn;
	}

	void set_priority(int p)
	{
		priority_ = p;
	}

	const fs::wpath& original_name() const { return original_name_; };
	int priority() const { return priority_; };
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;

		ar & make_nvp("original_name", original_name_);
		ar & make_nvp("completed_name", completed_name_);
		ar & make_nvp("current_name", current_name_);
		ar & make_nvp("priority", priority_);
	}	

private:
	fs::wpath original_name_;
	fs::wpath completed_name_;

	fs::wpath current_name_;

	int priority_;
};

class torrent_files
{
	struct by_filename{};
	struct by_random{};

	typedef boost::multi_index_container<
		torrent_file,
		mi::indexed_by<
			mi::random_access<
				mi::tag<by_random>
			>,
			mi::ordered_unique<
				mi::tag<by_filename>,
				mi::const_mem_fun<
					torrent_file, const fs::wpath&, &torrent_file::original_name> 
			>
		>
	> torrent_file_index_impl_t;

	typedef torrent_file_index_impl_t::index<by_filename>::type torrent_file_by_filename;
	typedef torrent_file_index_impl_t::index<by_random>::type torrent_file_by_random;

public:
	void set_file_priorities(std::vector<int> file_indices, int priority)
	{
		if (!files_.empty())
		{
			foreach(int i, file_indices)
			{
				torrent_file_by_random::iterator file_i = files_.get<by_random>().begin() + i; 

				torrent_file tmp_file = *(file_i);
				tmp_file.set_priority(priority);

				files_.get<by_random>().replace(file_i, tmp_file);
			}
		}
	}

	void push_back(const torrent_file& t)
	{
		files_.push_back(t);
	}

	bool empty() const
	{
		return files_.empty();
	}

	const torrent_file& operator[](size_type n) const
	{
		return files_.get<by_random>()[n];
	}
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & boost::serialization::make_nvp("files", files_);
	}	

private:
	torrent_file_index_impl_t files_;
};


} // namespace hal

BOOST_CLASS_VERSION(hal::torrent_file, 1)
