
//         Copyright Eóin O'Callaghan 2010 - 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <libtorrent/file_storage.hpp>

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
		priority_(p),
		finished_(false),
		with_hash_(h)
	{}

	torrent_file(const fs::wpath& on, bool h, int p=1) :
		priority_(p),
		finished_(false),
		with_hash_(h)
	{}

	void set_finished(const fs::wpath& on)
	{
		finished_ = true;
		with_hash_ = false;
		
		if (completed_name_.empty())
			completed_name_ = on;
	}

	void change_filename(const fs::wpath& fn)
	{
		completed_name_ = fn;
	}

	void set_priority(int p)
	{
		priority_ = p;
	}

	void change_root_name(const std::wstring& root, const fs::wpath& on)
	{
		completed_name_ = change_root_name(on, root);
	}

	const fs::wpath& completed_name() const { return completed_name_; }

	int priority() const { return priority_; };
	bool with_hash() const { return with_hash_; }
	bool is_finished() const { return finished_; }
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;

		if (version < 2)
		{
			// deprecated

			fs::wpath current_name_dont_use_;
			ar & make_nvp("current_name", current_name_dont_use_);
		}

		ar & make_nvp("completed_name", completed_name_);
		ar & make_nvp("priority", priority_);
		ar & make_nvp("finished", finished_);
		ar & make_nvp("with_hash", with_hash_);
	}	

private:
	fs::wpath completed_name_;

	int priority_;
	bool finished_;
	bool with_hash_;
};

class torrent_file_proxy
{
public:
	torrent_file_proxy(const torrent_file& f, const std::wstring& h, const fs::wpath& cn, const fs::wpath& on) :
		file_(f),
		hash_(h),
		original_name_(on)
	{}

		
	int priority() const { return file_.priority(); };
	bool with_hash() const { return file_.with_hash(); }
	bool is_finished() const { return file_.is_finished(); }

	fs::wpath active_name() const 
	{ 		
		if (file_.is_finished())
			return file_.completed_name();
		else if (file_.with_hash())
			return torrent_file::add_hash(original_name_, hash_);
		else
			return original_name_; 
	}
		
	const fs::wpath& completed_name() const { return file_.completed_name().empty() ? original_name_ : file_.completed_name(); }
		
private:
	const torrent_file& file_;
	const std::wstring& hash_;
	
	fs::wpath original_name_;
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
			>
		>
	> torrent_file_index_impl_t;

	//typedef torrent_file_index_impl_t::index<by_filename>::type torrent_file_by_filename;
	typedef torrent_file_index_impl_t::index<by_random>::type torrent_file_by_random;

public:
	typedef function<void (size_t, int, upgrade_lock&)> set_priority_fn;
	typedef function<void (size_t, upgrade_lock&)> changed_filename_fn;

	torrent_files(boost::shared_mutex& m, set_priority_fn sp, changed_filename_fn cf) :
		mutex_(m),
		set_priority_fn_(sp),
		changed_filename_fn_(cf)
	{}

	void set_file_priorities(std::vector<int> file_indices, int priority, upgrade_lock& l)
	{
		if (!files_.empty())
		{
			foreach(int i, file_indices)
			{
				torrent_file_by_random::iterator file_i = files_.get<by_random>().begin() + i; 

				torrent_file tmp_file = *(file_i);
				tmp_file.set_priority(priority);

				{	upgrade_to_unique_lock up_l(l);
					files_.get<by_random>().replace(file_i, tmp_file);
				}

				set_priority_fn_(i, priority, l);
			}
		}
	}
	
	void set_hash(const std::wstring& h)
	{
		hash_ = h;
	}

	void set_libt_files(const libt::file_storage& lofs)
	{
		libt_orig_files_ = lofs;
	}

	void set_root_name(const wstring& root, upgrade_lock& l)
	{
		torrent_file_index_impl_t new_files;

		for (torrent_file_by_random::iterator i=files_.get<by_random>().begin(), e=files_.get<by_random>().end();
			i != e; ++i)
		{
			torrent_file tmp_file = *(i);
			tmp_file.change_root_name(root, 
				path_from_utf8(libt_orig_files_.at(
					static_cast<int>(std::distance(files_.get<by_random>().begin(), i))).path));

			new_files.push_back(tmp_file);
		}

		{	upgrade_to_unique_lock up_l(l);
			std::swap(files_, new_files);
		}
	}

	void push_back(const torrent_file& t, upgrade_lock& l)
	{
		upgrade_to_unique_lock up_l(l);

		files_.push_back(t);
	}

	bool empty(upgrade_lock& l) const
	{
		return files_.empty();
	}

	size_t size(upgrade_lock& l) const
	{
		return files_.size();
	}

	void set_file_finished(size_t i, upgrade_lock& l)
	{
		torrent_file_by_random::iterator file_i = files_.get<by_random>().begin() + i; 

		torrent_file tmp_file = *(file_i);
		tmp_file.set_finished(path_from_utf8(libt_orig_files_.at(i).path));

		{	upgrade_to_unique_lock up_l(l);
			files_.get<by_random>().replace(file_i, tmp_file);
		}

		changed_filename_fn_(i, l);
	}

	void change_filename(size_t i, const fs::wpath& fn)
	{		
		upgrade_lock l(mutex_);
		
		change_filename(i, fn, l);
	}

	void change_filename(size_t i, const fs::wpath& fn, upgrade_lock& l)
	{
		torrent_file_by_random::iterator file_i = files_.get<by_random>().begin() + i; 

		torrent_file tmp_file = *(file_i);
		tmp_file.change_filename(fn);

		{	upgrade_to_unique_lock up_l(l);
			files_.get<by_random>().replace(file_i, tmp_file);
		}

		changed_filename_fn_(i, l);
	}

	const torrent_file_proxy operator[](size_t n) const
	{
		return torrent_file_proxy(files_.get<by_random>()[n], hash_, 
			path_from_utf8(libt_orig_files_.at(n).path), path_from_utf8(libt_orig_files_.at(n).path));
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

	boost::shared_mutex& mutex_;
	torrent_file_index_impl_t files_;

	std::wstring hash_;
	
	libt::file_storage libt_orig_files_;
};


} // namespace hal

BOOST_CLASS_VERSION(hal::torrent_file, 2)
