
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

	typedef std::pair<std::wstring, fs::wpath> split_path_pair_t;

	static split_path_pair_t split_root(const fs::wpath& p_orig)
	{
		if (!p_orig.empty())
		{
		std::wstring root = *p_orig.begin();
		fs::wpath p_new;

		if (++p_orig.begin() != p_orig.end())
		{			
			for (fs::wpath::iterator i = ++p_orig.begin(), e = p_orig.end(); i != e; ++i)
			{
				p_new /= *i;
			}

			return std::make_pair(root, p_new);
		}
		else
			return std::make_pair(L"", p_orig);
		}
		else
			return split_path_pair_t(); 
	}

	static fs::wpath add_hash(const fs::wpath& p_orig, const std::wstring& hash)
	{
		split_path_pair_t split = split_root(p_orig);

		return fs::wpath(split.first) / hash / split.second;
	}	
	
	static fs::wpath change_root_name(const fs::wpath& p_orig, const std::wstring& root)
	{
		split_path_pair_t split = split_root(p_orig);

		if (split.first.empty())
		{
			return p_orig;
		}
		else
		{
			return root / split.second;
		}
	}

	torrent_file()
	{}

	torrent_file(const wstring& on, bool h, int p=1) :
		current_name_(on),
		priority_(p),
		finished_(false),
		with_hash_(h)
	{}

	torrent_file(const fs::wpath& on, bool h, int p=1) :
		current_name_(on),
		priority_(p),
		finished_(false),
		with_hash_(h)
	{}

	void set_finished()
	{
		finished_ = true;
		with_hash_ = false;
		
		if (!completed_name_.empty())
			current_name_ = completed_name_;
	}

	void change_filename(const fs::wpath& fn)
	{
		completed_name_ = fn;

		if (finished_)
			current_name_ = fn;
	}

	void set_priority(int p)
	{
		priority_ = p;
	}

	fs::wpath active_name(const std::wstring& hash) const 
	{ 
		if (with_hash_)
			return add_hash(current_name_, hash);
		else
			return current_name_; 
	}

	void change_root_name(const std::wstring& root)
	{
		current_name_ = change_root_name(current_name_, root);
		completed_name_ = change_root_name(completed_name_, root);
	}

	const fs::wpath& completed_name() const { return completed_name_ != L"" ? completed_name_ : current_name_; }

	int priority() const { return priority_; };
	bool with_hash() const { return with_hash_; }
	bool is_finished() const { return finished_; }
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;

		ar & make_nvp("current_name", current_name_);
		ar & make_nvp("completed_name", completed_name_);
		ar & make_nvp("priority", priority_);
		ar & make_nvp("finished", finished_);
		ar & make_nvp("with_hash", with_hash_);
	}	

private:
	fs::wpath current_name_;
	fs::wpath completed_name_;

	int priority_;
	bool finished_;
	bool with_hash_;
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
					torrent_file, const fs::wpath&, &torrent_file::completed_name> 
			>
		>
	> torrent_file_index_impl_t;

	typedef torrent_file_index_impl_t::index<by_filename>::type torrent_file_by_filename;
	typedef torrent_file_index_impl_t::index<by_random>::type torrent_file_by_random;

public:
	typedef function<void (size_t, int)> set_priority_fn;
	typedef function<void (size_t)> changed_filename_fn;

	torrent_files(set_priority_fn sp, changed_filename_fn cf) :
		set_priority_fn_(sp),
		changed_filename_fn_(cf)
	{}

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

				set_priority_fn_(i, priority);
			}
		}
	}

	void set_root_name(const wstring& root)
	{
		torrent_file_index_impl_t new_files;

		for (torrent_file_by_random::iterator i=files_.get<by_random>().begin(), e=files_.get<by_random>().end();
			i != e; ++i)
		{
			torrent_file tmp_file = *(i);
			tmp_file.change_root_name(root);

			new_files.push_back(tmp_file);
		}

		std::swap(files_, new_files);
	}

	void push_back(const torrent_file& t)
	{
		files_.push_back(t);
	}

	bool empty() const
	{
		return files_.empty();
	}

	size_t size() const
	{
		return files_.size();
	}

	void set_file_finished(size_t i)
	{
		torrent_file_by_random::iterator file_i = files_.get<by_random>().begin() + i; 

		torrent_file tmp_file = *(file_i);
		tmp_file.set_finished();
		files_.get<by_random>().replace(file_i, tmp_file);

		changed_filename_fn_(i);
	}

	void change_filename(size_t i, const fs::wpath& fn)
	{
		torrent_file_by_random::iterator file_i = files_.get<by_random>().begin() + i; 

		torrent_file tmp_file = *(file_i);
		tmp_file.change_filename(fn);
		files_.get<by_random>().replace(file_i, tmp_file);

		changed_filename_fn_(i);
	}

	const torrent_file& operator[](size_t n) const
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
	set_priority_fn set_priority_fn_;
	changed_filename_fn changed_filename_fn_;

	torrent_file_index_impl_t files_;
};


} // namespace hal

BOOST_CLASS_VERSION(hal::torrent_file, 1)
