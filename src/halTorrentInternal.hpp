
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/tag.hpp>

namespace hal 
{
class TorrentInternal;
}

BOOST_CLASS_VERSION(hal::TorrentInternal, 8)

namespace hal 
{

namespace lbt = libtorrent;
namespace fs = boost::filesystem;

using fs::path;
using fs::ifstream;
using fs::ofstream;
using boost::serialization::make_nvp;

lbt::entry haldecode(const wpath &file) 
{
	ifstream fs(file, ifstream::binary);
	if (fs.is_open()) 
	{
		fs.unsetf(ifstream::skipws);
		return lbt::bdecode(std::istream_iterator<char>(fs), std::istream_iterator<char>());
	}
	else return lbt::entry();
}

bool halencode(const wpath &file, const lbt::entry &e) 
{
	fs::ofstream fs(file, ofstream::binary);

	if (!fs.is_open()) 
		return false;
	
	lbt::bencode(std::ostream_iterator<char>(fs), e);
	return true;
}

class invalidTorrent : public std::exception
{
public:
	invalidTorrent(const std::wstring& who) :
		who_(who)
	{}
	
	virtual ~invalidTorrent() throw () {}

	wstring who() const throw ()
	{
		return who_;
	}       
      
private:
	std::wstring who_;	
};

namespace lbt = libtorrent;
namespace fs = boost::filesystem;

using fs::path;
using fs::ifstream;
using fs::ofstream;
using boost::serialization::make_nvp;

class TorrentInternal
{
	friend class BitTorrent_impl;
	
	template<typename T>
	class TransferTracker
	{
	public:
		TransferTracker() :
			total_(0),
			total_offset_(0)
		{}
		
		TransferTracker(T total) :
			total_(total),
			total_offset_(0)
		{}
		
		TransferTracker(T total, T offset) :
			total_(total),
			total_offset_(offset)
		{}
		
		void reset(T total) const
		{
			total_ = total;
			total_offset_ = 0;
		}
		
		T update(T rel_total) const
		{
			total_ += (rel_total - total_offset_);
			total_offset_ = rel_total;
			
			return total_;
		}
		
		void setOffset(T offset) const
		{
			total_offset_ = offset;
		}
		
		operator T() const { return total_; }
		
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & make_nvp("total", total_);
		}
		
	private:
		mutable T total_;
		mutable T total_offset_;
	};
	
	class DurationTracker
	{
	public:
		DurationTracker() :
			total_(boost::posix_time::time_duration(0,0,0,0),boost::posix_time::time_duration(0,0,0,0)),
			start_(boost::posix_time::second_clock::universal_time())
		{}
		
		boost::posix_time::time_duration update() const
		{
			return total_.update(boost::posix_time::second_clock::universal_time() - start_);
		}
		
		void reset() const
		{
			total_.setOffset(boost::posix_time::time_duration(0,0,0,0));
			start_ = boost::posix_time::second_clock::universal_time();
		}
		
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & make_nvp("total", total_);
		}
		
		operator boost::posix_time::time_duration() const { return total_; }
		
	private:
		TransferTracker<boost::posix_time::time_duration> total_;	
		mutable boost::posix_time::ptime start_;
		
	};
	
public:
	#define TORRENT_INTERNALS_DEFAULTS \
		transferLimit_(std::pair<float, float>(-1, -1)), \
		connections_(-1), \
		uploads_(-1), \
		ratio_(0), \
		resolve_countries_(true), \
		state_(TorrentDetail::torrent_active), \
		totalUploaded_(0), \
		totalBase_(0), \
		startTime_(boost::posix_time::second_clock::universal_time())
		
	TorrentInternal() :	
		TORRENT_INTERNALS_DEFAULTS,		
		in_session_(false)
	{}
	
	TorrentInternal(libtorrent::torrent_handle h, std::wstring f, wpath saveDirectory) :
		TORRENT_INTERNALS_DEFAULTS,
		filename_(f),
		save_directory_(saveDirectory.string()),
		in_session_(true),
		handle_(h)
	{
		name_ = L"";
	}
	
	TorrentInternal(wpath filename, wpath saveDirectory, wpath workingDirectory, bool compactStorage) :
		TORRENT_INTERNALS_DEFAULTS,
		save_directory_(saveDirectory.string()),
		compactStorage_(compactStorage),		
		in_session_(false)
	{
		assert(the_session_);
		
		prep(filename, save_directory_, workingDirectory);
	}
	
	#undef TORRENT_INTERNALS_DEFAULTS
	
	TorrentDetail_ptr getTorrentDetail_ptr();
	void setTransferSpeed(float down, float up);
	void setConnectionLimit(int maxConn, int maxUpload);
	pair<float, float> getTransferSpeed();
	pair<int, int> getConnectionLimit();
	
	const wstring& name() const { return name_; }
	
	void setRatio(float ratio) 
	{ 
		if (ratio < 0) ratio = 0;
		ratio_ = ratio; 
		
		applyRatio();
	}
	
	float getRatio()
	{
		return ratio_;
	}
		
	void addToSession(bool paused = false)
	{
		if (!in_session_ && the_session_) 
		{
			string dir = to_utf8(save_directory_);
			
			handle_ = the_session_->add_torrent(metadata_, dir, resumedata_, compactStorage_, paused);
				
			in_session_ = true;
			if (paused)
				state_ = TorrentDetail::torrent_paused;	

			applySettings();
		}	
	}
	
	bool inSession() const 
	{ 
		return (in_session_ && the_session_); 
	}
	
	void resume()
	{
		addToSession();	
		assert(in_session_);
		
		handle_.resume();
		state_ = TorrentDetail::torrent_active;
		
		activeDuration_.reset();
		seedingDuration_.reset();
		
		assert(!handle_.is_paused());
	}
	
	void pause()
	{
		if (state_ != TorrentDetail::torrent_stopped)
		{	
			addToSession(true);
			assert(in_session_);
			
			state_ = TorrentDetail::torrent_paused;	
			
			assert(handle_.is_paused());
		}
	}
	
	void stop()
	{
		if (in_session_ && the_session_) 
		{
			the_session_->remove_torrent(handle_);
			in_session_ = false;
		}
		assert(!in_session_);
		
		state_ = TorrentDetail::torrent_stopped;	
	}
	
	bool isActive() const { return state_ == TorrentDetail::torrent_active;	}
	
	unsigned state() const { return state_; }
	
	void setTrackerLogin(wstring username, wstring password)
	{
		trackerUsername_ = username;
		trackerPassword_ = password;
		
		applyTrackerLogin();
	}	
	
	pair<wstring, wstring> getTrackerLogin() const
	{
		return make_pair(trackerUsername_, trackerPassword_);
	}
	
	const std::wstring& filename() const { return filename_; }
	
	const libtorrent::torrent_handle& handle() const { return handle_; }

	void setHandle(libtorrent::torrent_handle h) 
	{ 
		handle_ = h; 
		in_session_ = true;
	}
	
	void resetTrackers()
	{
		if (inSession())
		{
			handle_.replace_trackers(torrent_trackers_);		
			trackers_.clear();
		}
	}
	
	const std::vector<TrackerDetail>& getTrackers()
	{
		if (inSession() && trackers_.empty())
		{
			std::vector<lbt::announce_entry> trackers = handle_.trackers();
			
			foreach (const lbt::announce_entry& entry, trackers)
			{
				trackers_.push_back(
					TrackerDetail(hal::from_utf8(entry.url), entry.tier));
			}
		}		
		return trackers_;
	}
	
	void setTrackers(const std::vector<TrackerDetail>& trackerDetails)
	{
		trackers_.clear();
		trackers_.assign(trackerDetails.begin(), trackerDetails.end());
		
		applyTrackers();
	}
	
	void setFilePriorities(std::vector<int> fileIndices, int priority)
	{
		if (!filePriorities_.empty())
		{
			foreach(int i, fileIndices)
				filePriorities_[i] = priority;
				
			applyFilePriorities();
		}
	}

	const wstring& saveDirectory() { return save_directory_; }
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & make_nvp("transferLimit", transferLimit_);
        ar & make_nvp("connections", connections_);
        ar & make_nvp("uploads", uploads_);
        ar & make_nvp("filename", filename_);
        ar & make_nvp("saveDirectory", save_directory_);
		
		if (version > 3) {
			ar & make_nvp("payloadUploaded_", payloadUploaded_);
			ar & make_nvp("payloadDownloaded_", payloadDownloaded_);
			ar & make_nvp("uploaded_", uploaded_);
			ar & make_nvp("downloaded_", downloaded_);		
		} 
		else if (version > 1)
		{
			ar & make_nvp("totalUploaded", totalUploaded_);
			ar & make_nvp("ratio", ratio_);
			
			payloadUploaded_.reset(totalUploaded_);
		}
		
		if (version > 0) {
			ar & make_nvp("trackerUsername", trackerUsername_);
			ar & make_nvp("trackerPassword", trackerPassword_);
		}
		if (version > 1) {
			ar & make_nvp("state", state_);
			ar & make_nvp("trackers", trackers_);
		}
	
		if (version > 2) {
			ar & make_nvp("resolve_countries", resolve_countries_);
		}
		if (version > 4) {
			ar & make_nvp("file_priorities", filePriorities_);
		}
		if (version > 5) {
			ar & make_nvp("startTime", startTime_);
			ar & make_nvp("activeDuration", activeDuration_);
			ar & make_nvp("seedingDuration", seedingDuration_);
		}
		if (version > 6) {
			ar & make_nvp("name", name_);		
		}
		else
		{
			name_ = filename_;
		}
		if (version > 7) {
			ar & make_nvp("compactStorage", compactStorage_);		
		}
		
    }
	
	void setEntryData(libtorrent::entry metadata, libtorrent::entry resumedata)
	{		
		metadata_ = metadata;
		resumedata_ = resumedata;
	}

	std::vector<lbt::peer_info>& peers() { return peers_; }
	
	void updatePeers()
	{
		if (inSession())
			handle_.get_peer_info(peers_);
		else
			peers_.clear();
	}
	
	void getPeerDetails(PeerDetails& peerDetails) const
	{
		if (inSession())
			foreach (lbt::peer_info peer, peers_) 
			{
				peerDetails.push_back(peer);
			}	
	}

	void getFileDetails(FileDetails& fileDetails)
	{
		if (inSession())
		{
			lbt::torrent_info info = handle_.get_torrent_info();
			std::vector<lbt::file_entry> files;
			
			std::copy(info.begin_files(), info.end_files(), 
				std::back_inserter(files));
			
			std::vector<float> fileProgress;			
			handle_.file_progress(fileProgress);
			
			if (filePriorities_.size() != files.size())
			{
				filePriorities_.clear();
				filePriorities_.assign(files.size(), 1);
			}
			
			for(size_t i=0, e=files.size(); i<e; ++i)
			{
				wstring fullPath = hal::from_utf8(files[i].path.string());
				
				fileDetails.push_back(FileDetail(fullPath, files[i].size, fileProgress[i], filePriorities_[i], i));
			}			
		}
	}

private:
	void prep(wpath filename, wpath saveDirectory, wpath workingDirectory)
	{
	
		metadata_ = haldecode(filename);
		info_ = lbt::torrent_info(metadata_);
		
		name_ = hal::safe_from_utf8(info_.name());
		
		filename_ = name_;
		
		if (!boost::find_last(filename_, L".torrent")) 
			filename_ += L".torrent";		
		
		const wpath resumeFile = workingDirectory/L"resume"/filename_;
		
		//  vvv Handle old naming style!
		const wpath oldResumeFile = workingDirectory/L"resume"/filename.leaf();
		
		if (resumeFile != oldResumeFile && exists(oldResumeFile))
			fs::rename(oldResumeFile, resumeFile);
		//  ^^^ Handle old naming style!	
	
		if (exists(resumeFile)) 
		{
			try 
			{
				resumedata_ = haldecode(resumeFile);
			}
			catch(std::exception &e) 
			{		
				hal::event().post(boost::shared_ptr<hal::EventDetail>(
					new hal::EventStdException(Event::critical, e, L"prepTorrent, Resume"))); 
		
				remove(resumeFile);
			}
		}

		if (!exists(workingDirectory/L"torrents"))
			create_directory(workingDirectory/L"torrents");

		if (!exists(workingDirectory/L"torrents"/filename_))
			copy_file(filename.string(), workingDirectory/L"torrents"/filename_);

		if (!exists(saveDirectory))
			create_directory(saveDirectory);	
	}
	
	void applySettings()
	{		
		applyTransferSpeed();
		applyConnectionLimit();
		applyRatio();
		applyTrackers();
		applyTrackerLogin();
		applyFilePriorities();
		applyResolveCountries();
	}
	
	void applyTransferSpeed()
	{
		if (inSession())
		{
			int down = (transferLimit_.first > 0) ? static_cast<int>(transferLimit_.first*1024) : -1;
			handle_.set_download_limit(down);
			
			int up = (transferLimit_.second > 0) ? static_cast<int>(transferLimit_.second*1024) : -1;
			handle_.set_upload_limit(up);
		}
	}

	void applyConnectionLimit()
	{
		if (inSession())
		{
			handle_.set_max_connections(connections_);
			handle_.set_max_uploads(uploads_);
		}
	}
	
	void applyRatio()
	{ 
		if (inSession())
			handle_.set_ratio(ratio_);
	}
	
	void applyTrackers()
	{
		if (inSession())
		{
			if (torrent_trackers_.empty())
				torrent_trackers_ = handle_.trackers();
			
			if (!trackers_.empty())
			{
				std::vector<lbt::announce_entry> trackers;
				
				foreach (const TrackerDetail& tracker, trackers_)
				{
					trackers.push_back(
						lbt::announce_entry(hal::to_utf8(tracker.url)));
					trackers.back().tier = tracker.tier;
				}
				handle_.replace_trackers(trackers);
			}
		}
	}
	
	void applyTrackerLogin()
	{
		if (inSession())
		{
			if (trackerUsername_ != L"")
			{
				handle_.set_tracker_login(hal::to_utf8(trackerUsername_),
					hal::to_utf8(trackerPassword_));
			}
		}
	}
	
	void applyFilePriorities()
	{		
		if (inSession()) 
		{
			if (!filePriorities_.empty())
				handle_.prioritize_files(filePriorities_);
		}
	}	
	
	void applyResolveCountries()
	{
		if (inSession())
			handle_.resolve_countries(resolve_countries_);
	}
	
	static libtorrent::session* the_session_;
	
	std::pair<float, float> transferLimit_;
	
	unsigned state_;
	int connections_;
	int uploads_;
	bool in_session_;
	float ratio_;
	bool resolve_countries_;
	
	std::wstring filename_;
	std::wstring name_;
	std::wstring save_directory_;
	libtorrent::torrent_handle handle_;	
	
	libtorrent::entry metadata_;
	libtorrent::entry resumedata_;
	
	std::wstring trackerUsername_;	
	std::wstring trackerPassword_;
	
	boost::int64_t totalUploaded_;
	boost::int64_t totalBase_;
	
	TransferTracker<boost::int64_t> payloadUploaded_;
	TransferTracker<boost::int64_t> payloadDownloaded_;
	TransferTracker<boost::int64_t> uploaded_;
	TransferTracker<boost::int64_t> downloaded_;
	
	boost::posix_time::ptime startTime_;
	DurationTracker activeDuration_;
	DurationTracker seedingDuration_;
	
	std::vector<TrackerDetail> trackers_;
	std::vector<lbt::announce_entry> torrent_trackers_;
	std::vector<lbt::peer_info> peers_;	
	std::vector<int> filePriorities_;
	
	lbt::torrent_status statusMemory_;
	lbt::torrent_info info_;
	
	bool compactStorage_;
};

typedef std::map<std::string, TorrentInternal> TorrentMap;
typedef std::pair<std::string, TorrentInternal> TorrentPair;

class TorrentManager
{
	struct TorrentHolder
	{
		mutable TorrentInternal torrent;
		
		wstring filename;
		wstring name;		
		
		TorrentHolder() :
			torrent(), filename(L""), name(L"")
		{}
		
		explicit TorrentHolder(const TorrentInternal& t) :
			torrent(t), filename(t.filename()), name(t.name())
		{}
		
		TorrentHolder(const wstring& f, const wstring& n, const TorrentInternal& t) :
			torrent(t), filename(f), name(n)
		{}
				
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & make_nvp("torrent", torrent);
			ar & make_nvp("filename", filename);
			ar & make_nvp("name", name);
		}
	};
	
	struct byFilename{};
	struct byName{};
	
	typedef boost::multi_index_container<
		TorrentHolder,
		boost::multi_index::indexed_by<
			boost::multi_index::ordered_unique<
				boost::multi_index::tag<byFilename>,
				boost::multi_index::member<
					TorrentHolder, wstring, &TorrentHolder::filename> 
				>,
			boost::multi_index::ordered_unique<
				boost::multi_index::tag<byName>,
				boost::multi_index::member<
					TorrentHolder, wstring, &TorrentHolder::name> 
				>
		>
	> TorrentMultiIndex;
	
public:
	
	typedef TorrentMultiIndex::index<byFilename>::type torrentByFilename;
	typedef TorrentMultiIndex::index<byName>::type torrentByName;
	
	TorrentManager()
	{}
	
	TorrentManager(const TorrentMap& map)
	{			
		for (TorrentMap::const_iterator i=map.begin(), e=map.end(); i != e; ++i)
		{
			torrents_.insert(TorrentHolder((*i).second));
		}
	}
	
	std::pair<torrentByName::iterator, bool> insert(const TorrentHolder& h)
	{
		return torrents_.get<byName>().insert(h);
	}
	
	std::pair<torrentByName::iterator, bool> insert(const TorrentInternal& t)
	{
		return insert(TorrentHolder(t));
	}

	TorrentInternal& getByFile(const wstring& filename)
	{
		torrentByFilename::iterator it = torrents_.get<byFilename>().find(filename);
		
		if (it != torrents_.get<byFilename>().end())
		{
			return (*it).torrent;
		}
		
		throw invalidTorrent(filename);
	}
	
	TorrentInternal& get(const wstring& name)
	{
		torrentByName::iterator it = torrents_.get<byName>().find(name);
		
		if (it != torrents_.get<byName>().end())
		{
			return (*it).torrent;
		}
		
		throw invalidTorrent(name);
	}
	
	torrentByName::iterator begin() { return torrents_.get<byName>().begin(); }
	torrentByName::iterator end() { return torrents_.get<byName>().end(); }
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & make_nvp("torrents", torrents_);
	}	
	
private:
	TorrentMultiIndex torrents_;
};

void TorrentInternal::setConnectionLimit(int maxConn, int maxUpload)
{
	connections_ = 	maxConn;
	uploads_ = maxUpload;
	
		::MessageBoxA(0, "Here Alright.", "Hi", 0);
	applyConnectionLimit();
}

pair<int, int> TorrentInternal::getConnectionLimit()
{
	return make_pair(connections_, uploads_);
}

void TorrentInternal::setTransferSpeed(float download, float upload)
{	
	transferLimit_ = make_pair(download, upload);
	
	applyTransferSpeed();
}

pair<float, float> TorrentInternal::getTransferSpeed()
{
	return transferLimit_;
}

TorrentDetail_ptr TorrentInternal::getTorrentDetail_ptr()
{	
	try
	{

	if (inSession())
	{
		statusMemory_ = handle_.status();
		name_ = hal::safe_from_utf8(handle_.get_torrent_info().name());
	}
	
	wstring state;
	
	if (state_ == TorrentDetail::torrent_paused)
		state = app().res_wstr(HAL_TORRENT_PAUSED);
	else if (state_ == TorrentDetail::torrent_stopped)
		state = app().res_wstr(HAL_TORRENT_STOPPED);
	else
	{
		switch (statusMemory_.state)
		{
		case lbt::torrent_status::queued_for_checking:
			state = app().res_wstr(HAL_TORRENT_QUEUED_CHECKING);
			break;
		case lbt::torrent_status::checking_files:
			state = app().res_wstr(HAL_TORRENT_CHECKING_FILES);
			break;
		case lbt::torrent_status::connecting_to_tracker:
			state = app().res_wstr(HAL_TORRENT_CONNECTING);
			break;
		case lbt::torrent_status::downloading_metadata:
			state = app().res_wstr(HAL_TORRENT_METADATA);
			break;
		case lbt::torrent_status::downloading:
			state = app().res_wstr(HAL_TORRENT_DOWNLOADING);
			break;
		case lbt::torrent_status::finished:
			state = app().res_wstr(HAL_TORRENT_FINISHED);
			break;
		case lbt::torrent_status::seeding:
			state = app().res_wstr(HAL_TORRENT_SEEDING);
			break;
		case lbt::torrent_status::allocating:
			state = app().res_wstr(HAL_TORRENT_ALLOCATING);
			break;
		}	
	}
	
	boost::posix_time::time_duration td(boost::posix_time::pos_infin);
	
	if (statusMemory_.download_payload_rate != 0)
	{
		td = boost::posix_time::seconds(	
			long(float(statusMemory_.total_wanted-statusMemory_.total_wanted_done) / statusMemory_.download_payload_rate));
	}
	
	totalUploaded_ += (statusMemory_.total_payload_upload - totalBase_);
	totalBase_ = statusMemory_.total_payload_upload;
	
	uploaded_.update(statusMemory_.total_upload);
	payloadUploaded_.update(statusMemory_.total_payload_upload);
	downloaded_.update(statusMemory_.total_download);
	payloadDownloaded_.update(statusMemory_.total_payload_download);
	
	if (isActive())
	{
		activeDuration_.update();
	}
	
	updatePeers();
	
	size_t totalPeers = 0;
	size_t peersConnected = 0;
	size_t totalSeeds = 0;
	size_t seedsConnected = 0;
	
	foreach (lbt::peer_info peer, peers_) 
	{
		float speedSum = peer.down_speed + peer.up_speed;
		
		if (!(peer.flags & lbt::peer_info::seed))
		{
			++totalPeers;
			
			if (speedSum > 0)
				++peersConnected;
		}
		else
		{
			++totalSeeds;
			
			if (speedSum > 0)
				++seedsConnected;
		}
	}			

	return TorrentDetail_ptr(new TorrentDetail(filename_, state, hal::from_utf8(statusMemory_.current_tracker), 
		pair<float, float>(statusMemory_.download_payload_rate, statusMemory_.upload_payload_rate),
		statusMemory_.progress, statusMemory_.distributed_copies, statusMemory_.total_wanted_done, statusMemory_.total_wanted, uploaded_, payloadUploaded_,
		downloaded_, payloadDownloaded_, totalPeers, peersConnected, totalSeeds, seedsConnected, ratio_, td, statusMemory_.next_announce, activeDuration_, seedingDuration_, startTime_));

	}
	catch (const lbt::invalid_handle&)
	{
		event().post(shared_ptr<EventDetail>(
			new EventInvalidTorrent(Event::critical, Event::invalidTorrent, "addTorrent", "addTorrent")));\
	}
	catch (const std::exception& e)
	{
		event().post(shared_ptr<EventDetail>(
			new EventTorrentException(Event::critical, Event::torrentException, e.what(), "addTorrent", "addTorrent")));
	}
	
	return TorrentDetail_ptr(new TorrentDetail(filename_, app().res_wstr(HAL_TORRENT_STOPPED),  app().res_wstr(IDS_NA)));
}

} //namespace hal
