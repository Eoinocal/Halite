
//         Copyright Eóin O'Callaghan 2009 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "halIni.hpp"
#include "global/versioned_file.hpp"
#include "halTorrentInternal.hpp"


namespace hal 
{

class torrent_manager : 
	public boost::enable_shared_from_this<torrent_manager>,
	public hal::IniBase<torrent_manager>
{
	typedef torrent_manager this_class_t;
	typedef hal::IniBase<this_class_t> ini_class_t;

	struct torrent_holder
	{
		mutable torrent_internal_ptr torrent;
		
		boost::uuids::uuid id;
		wstring name;
		libt::big_number hash;
		
		torrent_holder()
		{}
		
		explicit torrent_holder(torrent_internal_ptr t) :
			torrent(t), id(torrent->id()), name(torrent->name()), hash(torrent->hash())
		{}

		friend class boost::serialization::access;
		template<class Archive>
		void load(Archive& ar, const unsigned int version)
		{
			using boost::serialization::make_nvp;

			wstring filename;

			switch (version)
			{
			case 2:
			ar & make_nvp("hash", hash);
			ar & make_nvp("uuid", id);

			case 1:
			ar & make_nvp("torrent", torrent);

			if (version == 1)
				ar & make_nvp("filename", filename);

			ar & make_nvp("name", name);
			}
			
			if (version == 1)
				id = torrent->id();
		}

		template<class Archive>
		void save(Archive& ar, const unsigned int version) const
		{
			using boost::serialization::make_nvp;

			switch (version)
			{
			case 2:
			ar & make_nvp("hash", hash);
			ar & make_nvp("uuid", id);

			case 1:
			ar & make_nvp("torrent", torrent);
			ar & make_nvp("name", name);
			}
		}

		BOOST_SERIALIZATION_SPLIT_MEMBER()
	};
	
	struct by_uuid{};
	struct by_name{};
	struct by_hash{};
	
	typedef boost::multi_index_container<
		torrent_holder,
		boost::multi_index::indexed_by<
			boost::multi_index::ordered_unique<
				boost::multi_index::tag<by_uuid>,
				boost::multi_index::member<
					torrent_holder, boost::uuids::uuid, &torrent_holder::id>
				>,
			boost::multi_index::ordered_non_unique<
				boost::multi_index::tag<by_name>,
				boost::multi_index::member<
					torrent_holder, wstring, &torrent_holder::name>
				>,
			boost::multi_index::ordered_non_unique<
				boost::multi_index::tag<by_hash>,
				boost::multi_index::member<
					torrent_holder, libt::big_number, &torrent_holder::hash>
				>
		>
	> torrent_multi_index;
	
public:
	typedef torrent_multi_index::index<by_uuid>::type torrent_by_uuid;
	typedef torrent_multi_index::index<by_name>::type torrent_by_name;
	typedef torrent_multi_index::index<by_hash>::type torrent_by_hash;

	torrent_manager(ini_file& ini) :
		ini_class_t("bittorrent", "torrent_manager", ini),
		work_file_(L"BitTorrent.data", boost::lexical_cast<boost::uuids::uuid>("7246289F-C92C-4781-A574-A1E944FD1183"), 1),
		ini_(ini)
	{}

	~torrent_manager()
	{
		for (torrent_by_name::iterator i= torrents_.get<by_name>().begin(), 
			e = torrents_.get<by_name>().end(); i!=e; ++i)
		{
			(*i).torrent->stop();
		}
	}

	void save_to_ini()
	{
		shared_wostream_ptr ofs = work_file_.wostream();		
		boost::archive::text_woarchive ot(*ofs);

		ot << boost::serialization::make_nvp("BitTorrent", *this);
	}	

	bool load_from_ini()
	{
		try 
		{

		if (!boost::filesystem::exists(work_file_.working_file()) &&
			boost::filesystem::exists(ini_.main_file()))
		{	
			ini_class_t::load_from_ini();
			ini_.clear();

			return true;
		}
		else
		{			
			if (boost::optional<shared_wistream_ptr> ifs = work_file_.wistream())
			{
				boost::archive::text_wiarchive it(**ifs);

				it >> boost::serialization::make_nvp("BitTorrent", *this);

				return true;
			}
			else
				return false;
		}
		
		}
		catch (const std::exception& e)
		{			
			hal::event_log().post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventXmlException(hal::from_utf8(e.what()), L"load_from_ini"))); 

			return false;
		}
	}

	void start_all()
	{
		for (torrent_by_uuid::iterator i= torrents_.get<by_uuid>().begin(), 
			e = torrents_.get<by_uuid>().end(); i!=e; /**/)
		{
		//	wpath file = wpath(hal::app().get_working_directory())/L"torrents"/(*i).torrent->filename();
			
		//	if (exists(file))
		//	{		
				try 
				{
										
				(*i).torrent->initialize_non_serialized(bind(&torrent_manager::update_torrent, this, _1));
				(*i).torrent->start();	
				
				++i;
				
				}
				catch(const libt::duplicate_torrent&)
				{
					hal::event_log().post(shared_ptr<hal::EventDetail>(
						new hal::EventDebug(hal::event_logger::debug, L"Encountered duplicate torrent")));
					
					++i; // Harmless, don't worry about it.
				}
				catch(const std::exception& e) 
				{
					hal::event_log().post(shared_ptr<hal::EventDetail>(
						new hal::EventStdException(hal::event_logger::warning, e, L"torrent_manager::start_all")));
					
					erase(i++);
				}			
		//	}
		//	else
		//	{
		//		the_torrents_.erase(i++);
		//	}
		}

		apply_queue_positions();
	}

	void apply_queue_positions()
	{
		for (torrent_by_uuid::iterator i= torrents_.get<by_uuid>().begin(), 
			e = torrents_.get<by_uuid>().end(); i!=e; ++i)
		{
			(*i).torrent->apply_queue_position();
		}
	}

	torrent_internal_ptr create_torrent(const wpath& filename, const wpath& save_directory, 
		bit::allocations alloc, const wpath& move_to_directory=L"")
	{
		torrent_internal_ptr t = torrent_internal_ptr(new torrent_internal(filename, save_directory, alloc, move_to_directory));

		std::pair<torrent_by_name::iterator, bool> p = torrents_.get<by_name>().insert(torrent_holder(t));

		if (!p.second) // Torrent already present
			t.reset();
		else
			t->initialize_non_serialized(bind(&torrent_manager::update_torrent, this, _1));

		return t;
	}

	torrent_internal_ptr create_torrent(const wstring& uri, const wpath& save_directory, 
		bit::allocations alloc, const wpath& move_to_directory=L"")
	{
		torrent_internal_ptr t = torrent_internal_ptr(new torrent_internal(uri, save_directory, alloc, move_to_directory));

		std::pair<torrent_by_hash::iterator, bool> p = torrents_.get<by_hash>().insert(torrent_holder(t));

		if (!p.second) // Torrent already present
			t.reset();
		else
			t->initialize_non_serialized(bind(&torrent_manager::update_torrent, this, _1));

		return t;
	}
	
	size_t remove_torrent(const uuid& name)
	{		
		return torrents_.get<by_uuid>().erase(name);
	}

/*	torrent_internal_ptr get_by_file(const wstring& filename)
	{
		torrent_by_filename::iterator it = torrents_.get<by_filename>().find(filename);
		
		if (it != torrents_.get<by_filename>().end() && (*it).torrent)
		{
			return (*it).torrent;
		}
		
		throw invalid_torrent(filename);
	}
*/
	torrent_internal_ptr get_by_name(const wstring& name)
	{
		torrent_by_name::iterator it = torrents_.get<by_name>().find(name);
		
		if (it != torrents_.get<by_name>().end() && (*it).torrent)
		{
			return (*it).torrent;
		}
		
		throw invalid_torrent(uuid());
	}
	
	torrent_internal_ptr get_by_hash(const libt::big_number& hash)
	{
		torrent_by_hash::iterator it = torrents_.get<by_hash>().find(hash);
		
		if (it != torrents_.get<by_hash>().end() && (*it).torrent)
		{
			return (*it).torrent;
		}
		
		throw invalid_torrent(uuid());
	}
	
	torrent_internal_ptr get_by_uuid(const uuid& id)
	{
		torrent_by_uuid::iterator it = torrents_.get<by_uuid>().find(id);
		
		if (it != torrents_.get<by_uuid>().end() && (*it).torrent)
		{
			return (*it).torrent;
		}
		
		throw invalid_torrent(id);
	}

	torrent_internal_ptr get(const uuid& id)
	{
		return get_by_uuid(id);
	}

	void update_torrent_by_hash(torrent_internal_ptr torrent)
	{
		torrent_by_hash::iterator it = torrents_.get<by_hash>().find(torrent->hash());
		
		if (it != torrents_.get<by_hash>().end() && (*it).torrent)
		{
			torrents_.get<by_hash>().replace(it, torrent_holder(torrent));
			display_holder(torrent_holder(torrent));
		}
	}

	void update_torrent_by_name(torrent_internal_ptr torrent)
	{
		torrent_by_name::iterator it = torrents_.get<by_name>().find(torrent->name());
		
		if (it != torrents_.get<by_name>().end() && (*it).torrent)
		{
			torrents_.get<by_name>().replace(it, torrent_holder(torrent));
			display_holder(torrent_holder(torrent));
		}
	}

	void update_torrent(torrent_internal_ptr torrent)
	{
		torrent_by_uuid::iterator it = torrents_.get<by_uuid>().find(torrent->id());
		
		if (it != torrents_.get<by_uuid>().end() && (*it).torrent)
		{
			torrents_.get<by_uuid>().replace(it, torrent_holder(torrent));
			display_holder(torrent_holder(torrent));
		}
	}
	
	torrent_by_uuid::iterator erase(torrent_by_uuid::iterator where)
	{
		return torrents_.get<by_uuid>().erase(where);
	}
	
	size_t size()
	{
		return torrents_.size();
	}
	
	bool exists(const uuid& id)
	{
		torrent_by_uuid::iterator it = torrents_.get<by_uuid>().find(id);
		
		if (it != torrents_.get<by_uuid>().end())
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

	void display_holder(const torrent_holder& t)
	{
		HAL_DEV_MSG(hal::wform(L"Holder name : %1%") % t.name);
		HAL_DEV_MSG(hal::wform(L"       uuid : %1%") % t.id);
		HAL_DEV_MSG(hal::wform(L"       hash : %1%") % from_utf8(libt::base32encode(std::string((char const*)&t.hash[0], 20))));
	}

	versioned_file work_file_;
	ini_file& ini_;
	torrent_multi_index torrents_;
};

};

BOOST_CLASS_VERSION(hal::torrent_manager::torrent_holder, 2)
