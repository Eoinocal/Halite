
//         Copyright Eóin O'Callaghan 2009 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#ifndef HAL_TORRENT_STATE_LOGGING
#	define TORRENT_STATE_LOG(s)
#else
#	include "../halEvent.hpp"
#	define TORRENT_STATE_LOG(msg) \
	hal::event_log().post(boost::shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(msg, hal::event_logger::torrent_dev))) 
#endif

#include <boost/tuple/tuple.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/tag.hpp>

#include "halIni.hpp"
#include "halTorrentInternal.hpp"

namespace hal 
{

class torrent_manager : 
	public hal::IniBase<torrent_manager>
{
	typedef torrent_manager thisClass;
	typedef hal::IniBase<thisClass> iniClass;

	struct torrent_holder
	{
		mutable torrent_internal_ptr torrent;
		
		wstring filename;
		wstring name;		
		
		torrent_holder()
		{}
		
		explicit torrent_holder(torrent_internal_ptr t) :
			torrent(t), filename(torrent->filename()), name(torrent->name())
		{}				

		friend class boost::serialization::access;
		template<class Archive>
		void load(Archive& ar, const unsigned int version)
		{
			using boost::serialization::make_nvp;

			ar & make_nvp("torrent", torrent);
			ar & make_nvp("filename", filename);
			ar & make_nvp("name", name);

			torrent->initialize_state_machine(torrent);
		}

		template<class Archive>
		void save(Archive& ar, const unsigned int version) const
		{
			using boost::serialization::make_nvp;

			ar & make_nvp("torrent", torrent);
			ar & make_nvp("filename", filename);
			ar & make_nvp("name", name);
		}

		BOOST_SERIALIZATION_SPLIT_MEMBER()
	};
	
	struct by_filename{};
	struct by_name{};
	
	typedef boost::multi_index_container<
		torrent_holder,
		boost::multi_index::indexed_by<
			boost::multi_index::ordered_unique<
				boost::multi_index::tag<by_filename>,
				boost::multi_index::member<
					torrent_holder, wstring, &torrent_holder::filename> 
				>,
			boost::multi_index::ordered_unique<
				boost::multi_index::tag<by_name>,
				boost::multi_index::member<
					torrent_holder, wstring, &torrent_holder::name> 
				>
		>
	> torrent_multi_index;
	
public:
	typedef torrent_multi_index::index<by_filename>::type torrent_by_filename;
	typedef torrent_multi_index::index<by_name>::type torrent_by_name;
	
	torrent_manager(ini_file& ini) :
		iniClass("bittorrent", "torrent_manager", ini)
	{}

	~torrent_manager()
	{
		for (torrent_by_name::iterator i= torrents_.get<by_name>().begin(), 
			e = torrents_.get<by_name>().end(); i!=e; ++i)
		{
			(*i).torrent->stop();
		}
	}

	void start_all()
	{
		for (torrent_by_name::iterator i= torrents_.get<by_name>().begin(), 
			e = torrents_.get<by_name>().end(); i!=e; ++i)
		{
			(*i).torrent->start();
		}
	}

	torrent_internal_ptr create_torrent(wpath filename, wpath saveDirectory, bit::allocations alloc, wpath move_to_directory=L"")
	{
		torrent_internal_ptr t = torrent_internal_ptr(new torrent_internal(filename, saveDirectory, alloc, move_to_directory));

		std::pair<torrent_by_name::iterator, bool> p = torrents_.get<by_name>().insert(torrent_holder(t));

		if (!p.second) 
			t.reset();
		else
			t->initialize_state_machine(t);

		return t;			
	}
	
	size_t remove_torrent(const wstring& name)
	{		
		TORRENT_STATE_LOG(L"Torrent manager erasing");

		return torrents_.get<by_name>().erase(name);
	}

	torrent_internal_ptr get_by_file(const wstring& filename)
	{
		torrent_by_filename::iterator it = torrents_.get<by_filename>().find(filename);
		
		if (it != torrents_.get<by_filename>().end() && (*it).torrent)
		{
			return (*it).torrent;
		}
		
		throw invalid_torrent(filename);
	}
	
	torrent_internal_ptr get(const wstring& name)
	{
		torrent_by_name::iterator it = torrents_.get<by_name>().find(name);
		
		if (it != torrents_.get<by_name>().end() && (*it).torrent)
		{
			return (*it).torrent;
		}
		
		throw invalid_torrent(name);
	}
	
	torrent_by_name::iterator erase(torrent_by_name::iterator where)
	{
		return torrents_.get<by_name>().erase(where);
	}
	
	size_t size()
	{
		return torrents_.size();
	}
	
	bool exists(const wstring& name)
	{
		torrent_by_name::iterator it = torrents_.get<by_name>().find(name);
		
		if (it != torrents_.get<by_name>().end())
			return true;
		else
			return false;
	}
	
	torrent_by_name::iterator begin() { return torrents_.get<by_name>().begin(); }
	torrent_by_name::iterator end() { return torrents_.get<by_name>().end(); }
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & boost::serialization::make_nvp("torrents", torrents_);
	}	
	
private:
	torrent_multi_index torrents_;
};

};

BOOST_CLASS_VERSION(hal::torrent_manager::torrent_holder, 1)
