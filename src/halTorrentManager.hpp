
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
		
		explicit torrent_holder(const libt::big_number& h) :
			hash(h)
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

		bool operator==(const torrent_holder& right) const
		{
			return hash == right.hash;
		}

		bool operator<(const torrent_holder& right) const
		{
			return hash < right.hash;
		}

		bool operator==(const libt::big_number& right) const
		{
			return hash == right;
		}

		bool operator<(const libt::big_number& right) const
		{
			return hash < right;
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

	torrent_manager() :
		ini_class_t(L"bittorrent", L"torrent_manager"),
		work_file_(L"BitTorrent.data", boost::lexical_cast<boost::uuids::uuid>("7246289F-C92C-4781-A574-A1E944FD1183"), 1),
		scheduler_(false)
//		ini_(ini)
	{}

	~torrent_manager()
	{
/*		for (torrent_by_name::iterator i= torrents_.get<by_name>().begin(), 
			e = torrents_.get<by_name>().end(); i!=e; ++i)
		{
			(*i).torrent->stop();
			scheduler_.terminate_processor((*i).torrent->state_handle());
			scheduler_.destroy_processor((*i).torrent->state_handle());
		}

		scheduler_.terminate();
*/	}

	void save_to_ini()
	{
		shared_wostream_ptr ofs = work_file_.wostream();		
		boost::archive::text_woarchive ot(*ofs);

		ot << boost::serialization::make_nvp("BitTorrent", *this);

		{
			std::ofstream ofs(work_file_.main_file().string()+".xml");
			boost::archive::xml_oarchive ot(ofs);

			ot << boost::serialization::make_nvp("BitTorrent", *this);

			ofs.flush();
		}
	}	

	bool load_from_ini()
	{
		try 
		{

/*		if (!boost::filesystem::exists(work_file_.working_file()) &&
			boost::filesystem::exists(ini_.main_file()))
		{	
			ini_class_t::load_from_ini();
			ini_.clear();

			return true;
		}
		else
*/		{			
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
		HAL_DEV_MSG(wform(L"Manager start all %1%") % torrents_.size());

		for (torrent_by_uuid::iterator i= torrents_.get<by_uuid>().begin(), e = torrents_.get<by_uuid>().end(); i != e; /**/)
	//	for (const torrent_holder& t : torrents_)
		{
		//	wpath file = wpath(hal::app().get_working_directory())/L"torrents"/(*i).torrent->filename();
			
		//	if (exists(file))
		//	{		
				try 
				{

				const torrent_holder& t = *i;

				if (!t.hash.is_all_zeros())
					if (std::count(torrents_.get<by_hash>().begin(), torrents_.get<by_hash>().end(), t) > 1)
					{
						hal::event_log().post(shared_ptr<hal::EventDetail>(
							new hal::EventDebug(hal::event_logger::debug, L"Erasing duplicate torrent")));

						erase(i++);
						continue;
					}

				if (t.torrent && t.torrent->id() != t.id)
				{
					hal::event_log().post(shared_ptr<hal::EventDetail>(
						new hal::EventDebug(hal::event_logger::debug, L"ID mismatch, Erasing torrent")));
					
					erase(i++);
					continue;
				}
										
				initiate_torrent((*i).torrent, bind(&torrent_manager::update_torrent, this, _1));
				(*i).torrent->start();	
				
				++i;
				
				}
		/*		catch(const libt::duplicate_torrent&)
				{
					hal::event_log().post(shared_ptr<hal::EventDetail>(
						new hal::EventDebug(hal::event_logger::debug, L"Encountered duplicate torrent")));
					
					++i; // Harmless, don't worry about it.
				}
		*/		catch(const std::exception& e) 
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

	void terminate_all()
	{	
		HAL_DEV_MSG(wform(L"Manager terminate all %1%") % torrents_.size());

		for (torrent_by_uuid::iterator i = torrents_.get<by_uuid>().begin(), 
			e = torrents_.get<by_uuid>().end(); i!=e; ++i)
		{
		//	assert(i->torrent->state() == torrent_details::torrent_stopped);
			terminate_torrent(i->torrent);
		}

		scheduler_();
		scheduler_.terminate();
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
			initiate_torrent(t, bind(&torrent_manager::update_torrent, this, _1));

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
			initiate_torrent(t, bind(&torrent_manager::update_torrent, this, _1));

		return t;
	}
	
	void remove_torrent(const uuid& id)
	{		
		torrent_by_uuid::iterator it = torrents_.get<by_uuid>().find(id);
		
		if (it != torrents_.get<by_uuid>().end())
			erase(it);
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
		return erase_duplicates_by_hash(hash);
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

	void process_torrent_event(sc::fifo_scheduler<>::processor_handle h, sc::fifo_scheduler<>::event_ptr_type e)
	{
		scheduler_.queue_event(h, e);
	}

	void process_events()
	{
		scheduler_();
	}

	template<typename F>
	void initiate_torrent(torrent_internal_ptr t, F&& f)
	{
		t->initialize_non_serialized(scheduler_.create_processor<torrent_internal_sm>(t), 
			f,
			bind(&torrent_manager::process_torrent_event, this, _1, _2));

		scheduler_.initiate_processor(*t->state_handle());
	}

/*	void erase_torrent(torrent_internal_ptr torrent)
	{
		torrent_by_uuid::iterator it = torrents_.get<by_uuid>().find(torrent->id());
		
		
		HAL_DEV_MSG(wform(L"Manager erase torrent %1%") % torrent->name());
		
		if (it != torrents_.get<by_uuid>().end() && (*it).torrent)
		{
			erase(it);
		}
	}
	*/

	void terminate_torrent(torrent_internal_ptr torrent)
	{
		if (auto handle = torrent->state_handle())
		{
			HAL_DEV_MSG(wform(L"Manager terminate torrent %1%") % torrent->name());

			scheduler_.terminate_processor(*handle);
			scheduler_.destroy_processor(*handle);
		}
	}

	torrent_by_uuid::iterator erase(torrent_by_uuid::iterator where)
	{
		terminate_torrent(where->torrent);
		return torrents_.get<by_uuid>().erase(where);
	}
	
	torrent_by_hash::iterator erase(torrent_by_hash::iterator where)
	{
		terminate_torrent(where->torrent);
		return torrents_.get<by_hash>().erase(where);
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
	torrent_internal_ptr erase_duplicates_by_hash(const libt::big_number& hash)
	{
		auto p = std::equal_range(torrents_.get<by_hash>().begin(), torrents_.get<by_hash>().end(), torrent_holder(hash));
		auto d = std::distance(p.first, p.second);
				
/*		hal::event_log().post(shared_ptr<hal::EventDetail>(
			new hal::EventDebug(hal::event_logger::debug, (hal::wform(L"%1% matching torrents for hash %2%") 
				% d
				% from_utf8(libt::base32encode(std::string((char const*)&hash[0], 20)))).str())));
*/
		if (d == 0)
		{			
			throw invalid_torrent(uuid());
		}
		else if (d > 1)
		{
			for (auto i = ++p.first, e = p.second; i != e; /* */)
				erase(i++);
							
			return p.first->torrent;
		}
		
		return p.first->torrent;
	}

	void display_holder(const torrent_holder& t)
	{
		HAL_DEV_MSG(hal::wform(L"Holder name : %1%") % t.name);
		HAL_DEV_MSG(hal::wform(L" -- uuid : %1%") % t.id);
		HAL_DEV_MSG(hal::wform(L" -- hash : %1%") % from_utf8(libt::base32encode(std::string((char const*)&t.hash[0], 20))));
	}

	versioned_file work_file_;
//	ini_file& ini_;
	torrent_multi_index torrents_;
	sc::fifo_scheduler<> scheduler_;
};

};

BOOST_CLASS_VERSION(hal::torrent_manager::torrent_holder, 2)
