
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define HALITE_VERSION					0, 3, 0, 399
#define HALITE_VERSION_STRING			"v 0.3.0.5 dev 399"
#define	HALITE_FINGERPRINT				"HL", 0, 3, 0, 5

#ifndef HAL_NA
#define HAL_NA 40013
#endif

#define HAL_TORRENT_EXT_BEGIN 				41000
#define LBT_EVENT_TORRENT_FINISHED			HAL_TORRENT_EXT_BEGIN + 1
#define HAL_PEER_BAN_ALERT					HAL_TORRENT_EXT_BEGIN + 2
#define HAL_HASH_FAIL_ALERT					HAL_TORRENT_EXT_BEGIN + 3
#define HAL_URL_SEED_ALERT					HAL_TORRENT_EXT_BEGIN + 5
#define HAL_TRACKER_WARNING_ALERT			HAL_TORRENT_EXT_BEGIN + 4
#define HAL_TRACKER_ANNOUNCE_ALERT			HAL_TORRENT_EXT_BEGIN + 6
#define HAL_TRACKER_ALERT					HAL_TORRENT_EXT_BEGIN + 7
#define HAL_TRACKER_REPLY_ALERT				HAL_TORRENT_EXT_BEGIN + 8
#define LBT_EVENT_TORRENT_PAUSED			HAL_TORRENT_EXT_BEGIN + 9
#define HAL_FAST_RESUME_ALERT				HAL_TORRENT_EXT_BEGIN + 10
#define HAL_PIECE_FINISHED_ALERT			HAL_TORRENT_EXT_BEGIN + 11
#define HAL_BLOCK_FINISHED_ALERT			HAL_TORRENT_EXT_BEGIN + 12
#define HAL_BLOCK_DOWNLOADING_ALERT			HAL_TORRENT_EXT_BEGIN + 13
#define HAL_LISTEN_SUCCEEDED_ALERT			HAL_TORRENT_EXT_BEGIN + 14
#define HAL_LISTEN_FAILED_ALERT				HAL_TORRENT_EXT_BEGIN + 15
#define HAL_IPFILTER_ALERT					HAL_TORRENT_EXT_BEGIN + 16
#define HAL_INCORRECT_ENCODING_LEVEL		HAL_TORRENT_EXT_BEGIN + 17
#define HAL_INCORRECT_CONNECT_POLICY    	HAL_TORRENT_EXT_BEGIN + 18
#define HAL_PEER_ALERT						HAL_TORRENT_EXT_BEGIN + 19
#define HAL_LISTEN_V6_FAILED_ALERT			HAL_TORRENT_EXT_BEGIN + 20

#define HAL_TORRENT_INT_BEGIN 				42000
#define HAL_PEER_INTERESTING            	HAL_TORRENT_INT_BEGIN + 1
#define HAL_PEER_CHOKED             		HAL_TORRENT_INT_BEGIN + 2
#define HAL_PEER_REMOTE_INTERESTING			HAL_TORRENT_INT_BEGIN + 3
#define HAL_PEER_REMOTE_CHOKED				HAL_TORRENT_INT_BEGIN + 4
#define HAL_PEER_SUPPORT_EXTENSIONS			HAL_TORRENT_INT_BEGIN + 5
#define HAL_PEER_LOCAL_CONNECTION			HAL_TORRENT_INT_BEGIN + 6
#define HAL_PEER_HANDSHAKE					HAL_TORRENT_INT_BEGIN + 7
#define HAL_PEER_CONNECTING					HAL_TORRENT_INT_BEGIN + 8
#define HAL_PEER_QUEUED						HAL_TORRENT_INT_BEGIN + 9
#define HAL_PEER_RC4_ENCRYPTED				HAL_TORRENT_INT_BEGIN + 10
#define HAL_PEER_PLAINTEXT_ENCRYPTED		HAL_TORRENT_INT_BEGIN + 11
#define HAL_TORRENT_QUEUED_CHECKING			HAL_TORRENT_INT_BEGIN + 12
#define HAL_TORRENT_CHECKING_FILES			HAL_TORRENT_INT_BEGIN + 13
#define HAL_TORRENT_CONNECTING				HAL_TORRENT_INT_BEGIN + 14
#define HAL_TORRENT_DOWNLOADING				HAL_TORRENT_INT_BEGIN + 15
#define HAL_TORRENT_FINISHED				HAL_TORRENT_INT_BEGIN + 16
#define HAL_TORRENT_SEEDING					HAL_TORRENT_INT_BEGIN + 17
#define HAL_TORRENT_ALLOCATING				HAL_TORRENT_INT_BEGIN + 18
#define HAL_TORRENT_QUEUED					HAL_TORRENT_INT_BEGIN + 19
#define HAL_TORRENT_STOPPED					HAL_TORRENT_INT_BEGIN + 20
#define HAL_TORRENT_PAUSED					HAL_TORRENT_INT_BEGIN + 21
#define HAL_TORRENT_STOPPING				HAL_TORRENT_INT_BEGIN + 22
#define HAL_TORRENT_PAUSING					HAL_TORRENT_INT_BEGIN + 23
#define HAL_TORRENT_METADATA            	HAL_TORRENT_INT_BEGIN + 24

#ifndef RC_INVOKED

#include <boost/tuple/tuple.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include "halIni.hpp"
#include "halSignaler.hpp"

namespace hal 
{
class TorrentInternalOld;
class TorrentInternal;
}

BOOST_CLASS_VERSION(hal::TorrentInternalOld, 9)
BOOST_CLASS_VERSION(hal::TorrentInternal, 1)

namespace hal 
{

namespace lbt = libtorrent;
namespace fs = boost::filesystem;
namespace pt = boost::posix_time;

typedef std::wstring wstring_t;
typedef std::string string_t;

typedef boost::wformat wformat_t;
typedef boost::format format_t;

typedef boost::filesystem::wpath wpath_t;
typedef boost::filesystem::path path_t;

using boost::serialization::make_nvp;
using boost::shared_ptr;
using boost::bind;

lbt::entry haldecode(const wpath_t &file) 
{
	fs::ifstream ifs(file, fs::ifstream::binary);
	if (ifs.is_open()) 
	{
		ifs.unsetf(fs::ifstream::skipws);
		return lbt::bdecode(std::istream_iterator<char>(ifs), std::istream_iterator<char>());
	}
	else return lbt::entry();
}

bool halencode(const wpath_t &file, const lbt::entry &e) 
{
	fs::ofstream ofs(file, fs::ofstream::binary);

	if (!ofs.is_open()) 
		return false;
	
	lbt::bencode(std::ostream_iterator<char>(ofs), e);
	return true;
}

class invalidTorrent : public std::exception
{
public:
	invalidTorrent(const wstring_t& who) :
		who_(who)
	{}
	
	virtual ~invalidTorrent() throw () {}

	wstring_t who() const throw ()
	{
		return who_;
	}       
	
private:
	wstring_t who_;	
};
	
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
		total_(boost::posix_time::time_duration(0,0,0,0), 
			boost::posix_time::time_duration(0,0,0,0))
	{}
	
	boost::posix_time::time_duration update() const
	{
		if (start_.is_not_a_date_time()) 
			start_ = boost::posix_time::second_clock::universal_time();

		if (static_cast<boost::posix_time::time_duration>(total_).is_special()) 
			total_.setOffset(boost::posix_time::time_duration(0,0,0,0));
		
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
	
class TorrentInternalOld
{
public:	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & make_nvp("transferLimit", transferLimit_);
        ar & make_nvp("connections", connections_);
        ar & make_nvp("uploads", uploads_);	
		
		if (version > 6) {
			ar & make_nvp("filename", filename_);
		}
		else 
		{
			wstring_t originalFilename;
			ar & make_nvp("filename", originalFilename);
			
			updatePreVersion7Files(originalFilename);
		}
		
        ar & make_nvp("saveDirectory", save_directory_);
		
		if (version > 7) {
			ar & make_nvp("payloadUploaded_", payloadUploaded_);
			ar & make_nvp("payloadDownloaded_", payloadDownloaded_);
			ar & make_nvp("uploaded_", uploaded_);
			ar & make_nvp("downloaded_", downloaded_);	
			ar & make_nvp("ratio", ratio_);	
		} 
		else if (version > 3) {
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
			ar & make_nvp("compactStorage", compactStorage_);
			ar & make_nvp("finishTime", finishTime_);
		}
		if (version > 8) {
			ar & make_nvp("progress", progress_);
		}
    }
	
	void extractNames(lbt::entry& metadata)
	{		
		lbt::torrent_info info(metadata);				
		name_ = hal::from_utf8_safe(info.name());
		
		filename_ = name_;
		if (!boost::find_last(filename_, L".torrent")) 
				filename_ += L".torrent";
		
		event().post(shared_ptr<EventDetail>(new EventMsg(
			wformat_t(L"Loaded names: %1%, %2%") % name_ % filename_)));
	}
	
	void updatePreVersion7Files(wstring_t originalFilename)
	{
		try 
		{

		wpath_t oldFile = app().working_directory()/L"torrents"/originalFilename;
		
		if (fs::exists(oldFile)) 
			extractNames(haldecode(oldFile));
		
		wpath_t oldResumeFile = app().working_directory()/L"resume"/originalFilename;
		
		if (filename_ != originalFilename)
		{
			fs::rename(oldFile, app().working_directory()/L"torrents"/filename_);
			
			if (fs::exists(oldResumeFile))
				fs::rename(oldResumeFile, app().working_directory()/L"resume"/filename_);
		}
		
		}
		catch(std::exception &e) 
		{		
			hal::event().post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventStdException(Event::critical, e, L"updatePreVersion7Files"))); 
		}
	}
	
	std::pair<float, float> transferLimit_;
	
	unsigned state_;
	int connections_;
	int uploads_;
	bool in_session_;
	float ratio_;
	bool resolve_countries_;
	
	wstring_t filename_;
	wstring_t name_;
	wstring_t save_directory_;
	wstring_t originalFilename_;
	lbt::torrent_handle handle_;	
	
	lbt::entry metadata_;
	lbt::entry resumedata_;
	
	wstring_t trackerUsername_;	
	wstring_t trackerPassword_;
	
	boost::int64_t totalUploaded_;
	boost::int64_t totalBase_;
	
	TransferTracker<boost::int64_t> payloadUploaded_;
	TransferTracker<boost::int64_t> payloadDownloaded_;
	TransferTracker<boost::int64_t> uploaded_;
	TransferTracker<boost::int64_t> downloaded_;
	
	boost::posix_time::ptime startTime_;
	boost::posix_time::ptime finishTime_;
	DurationTracker activeDuration_;
	DurationTracker seedingDuration_;
	
	std::vector<TrackerDetail> trackers_;
	std::vector<lbt::announce_entry> torrent_trackers_;
	std::vector<lbt::peer_info> peers_;	
	std::vector<int> filePriorities_;
	
	float progress_;
	
	lbt::torrent_info infoMemory_;
	lbt::torrent_status statusMemory_;
	FileDetails fileDetailsMemory_;
	
	bool compactStorage_;
};


struct signalers
{
	signaler<> torrent_finished;
	signaler<> torrent_paused;
};

class TorrentInternal : 
	boost::noncopyable
{
	friend class BitTorrent_impl;	
public:
	#define TORRENT_INTERNALS_DEFAULTS \
		originalFilename_(L""), \
		transferLimit_(std::pair<float, float>(-1, -1)), \
		connections_(-1), \
		uploads_(-1), \
		ratio_(0), \
		resolve_countries_(true), \
		totalUploaded_(0), \
		totalBase_(0), \
		progress_(0), \
		startTime_(boost::posix_time::second_clock::universal_time())
		
	TorrentInternal() :	
		TORRENT_INTERNALS_DEFAULTS,
		compactStorage_(true),
		state_(TorrentDetail::torrent_stopped),	
		in_session_(false)
	{}
	
	TorrentInternal(wpath_t filename, wpath_t saveDirectory, bool compactStorage, wpath_t move_to_directory=L"") :
		TORRENT_INTERNALS_DEFAULTS,
		save_directory_(saveDirectory.string()),
		move_to_directory_(move_to_directory.string()),
		compactStorage_(compactStorage),	
		state_(TorrentDetail::torrent_stopped),	
		in_session_(false)
	{
		assert(the_session_);
		
		prepare(filename, save_directory_);
	}
	
	TorrentInternal(const TorrentInternalOld& t) :
		transferLimit_(t.transferLimit_),
		state_(t.state_),
		connections_(t.connections_),
		uploads_(t.uploads_),
		in_session_(false),
		ratio_(t.ratio_),
		resolve_countries_(t.resolve_countries_),
		filename_(t.filename_),
		name_(t.name_),
		save_directory_(t.save_directory_),
		originalFilename_(t.originalFilename_),
		handle_(t.handle_),
		metadata_(t.metadata_),
		resumedata_(t.resumedata_),
		trackerUsername_(t.trackerUsername_),	
		trackerPassword_(t.trackerPassword_),
		totalUploaded_(t.totalUploaded_),
		totalBase_(t.totalBase_),
		payloadUploaded_(t.payloadUploaded_),
		payloadDownloaded_(t.payloadDownloaded_),
		uploaded_(t.uploaded_),
		downloaded_(t.downloaded_),
		startTime_(t.startTime_),
		finishTime_(t.finishTime_),
		activeDuration_(t.activeDuration_),
		seedingDuration_(t.seedingDuration_),
		trackers_(t.trackers_),
		torrent_trackers_(t.torrent_trackers_),
		peers_(t.peers_),
		filePriorities_(t.filePriorities_),
		progress_(t.progress_),
		infoMemory_(t.infoMemory_),
		statusMemory_(t.statusMemory_),
		fileDetailsMemory_(t.fileDetailsMemory_),
		compactStorage_(t.compactStorage_)
	{}
	
	#undef TORRENT_INTERNALS_DEFAULTS
	
	TorrentDetail_ptr getTorrentDetail_ptr();
	void setTransferSpeed(float down, float up);
	void setConnectionLimit(int maxConn, int maxUpload);
	std::pair<float, float> getTransferSpeed();
	std::pair<int, int> getConnectionLimit();
	
	const wstring_t& name() const { return name_; }
	
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
		try
		{

		mutex_t::scoped_lock l(mutex_);	
		assert(the_session_ != 0);

		HAL_DEV_MSG(wformat_t(L"addToSession() paused=%1%") % paused);
		
		if (!inSession()) 
		{			
			string_t dir = to_utf8(save_directory_);
			
			lbt::storage_mode_t storage = lbt::storage_mode_sparse;
			
			if (compactStorage_)
				storage = lbt::storage_mode_compact;
			
			handle_ = the_session_->add_torrent(metadata_, dir, resumedata_, storage, paused);			
			assert(handle_.is_valid());
			
			clearResumeData();
			
			in_session_ = true;
			if (paused)
				state_ = TorrentDetail::torrent_paused;	
			else
				state_ = TorrentDetail::torrent_active;	
				
			applySettings();
			handle_.force_reannounce();
		}	

		assert(inSession());
		HAL_DEV_MSG(L"Added to session");

		}
		catch(std::exception& e)
		{
			hal::event().post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventStdException(Event::critical, e, L"addToSession"))); 
		}
	}
	
	void removeFromSession(bool writeData=true)
	{
		try
		{

		mutex_t::scoped_lock l(mutex_);
		assert(inSession());

		HAL_DEV_MSG(wformat_t(L"removeFromSession() writeData=%1%") % writeData);
		
		if (writeData)
		{
			HAL_DEV_MSG(L"getting resume data");
			resumedata_ = handle_.write_resume_data(); // Update the fast-resume data
			HAL_DEV_MSG(L"writing resume data");
			writeResumeData();
		}
		
		HAL_DEV_MSG(L"removing handle from session");
		the_session_->remove_torrent(handle_);
		in_session_ = false;

		assert(!inSession());	
		HAL_DEV_MSG(L"Removed from session!");

		}
		catch(std::exception& e)
		{
			hal::event().post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventStdException(Event::critical, e, L"removeFromSession"))); 
		}
	}
	
	bool inSession() const
	{ 
		mutex_t::scoped_lock l(mutex_);

		return (in_session_ && the_session_ != 0 && handle_.is_valid());
	}
	
	void resume()
	{
		mutex_t::scoped_lock l(mutex_);

		if (state_ == TorrentDetail::torrent_stopped)
		{	
			addToSession(false);
			assert(inSession());			
		}
		else
		{
			assert(inSession());
			handle_.resume();
		}	
		
		state_ = TorrentDetail::torrent_active;			
		assert(!handle_.is_paused());
	}
	
	void pause()
	{
		mutex_t::scoped_lock l(mutex_);

		if (state_ == TorrentDetail::torrent_stopped)
		{	
			addToSession(true);

			assert(inSession());
		//	assert(handle_.is_paused());
		}
		else
		{
			assert(inSession());

			handle_.pause();
			signals().torrent_paused.disconnect_all_once();
			signals().torrent_paused.connect_once(bind(&TorrentInternal::completed_pause, this));

			state_ = TorrentDetail::torrent_pausing;	
		}			
	}
	
	void stop()
	{
		mutex_t::scoped_lock l(mutex_);

		if (state_ != TorrentDetail::torrent_stopped)
		{
			if (state_ == TorrentDetail::torrent_active)
			{
				assert(inSession());

				handle_.pause();
				signals().torrent_paused.disconnect_all_once();
				signals().torrent_paused.connect_once(bind(&TorrentInternal::completed_stop, this));

				state_ = TorrentDetail::torrent_stopping;
			}
			else if (state_ == TorrentDetail::torrent_paused)
			{			
				removeFromSession();
				state_ = TorrentDetail::torrent_stopped;				
			}
		}
	}

	void forceRecheck()
	{
		mutex_t::scoped_lock l(mutex_);		
		HAL_DEV_MSG(L"forceRecheck()");

		switch (state_)
		{
		case TorrentDetail::torrent_stopped:
			clearResumeData();
			resume();
			break;

		case TorrentDetail::torrent_stopping:
		case TorrentDetail::torrent_pausing:
			signals().torrent_paused.disconnect_all_once();

		case TorrentDetail::torrent_active:
			signals().torrent_paused.connect_once(bind(&TorrentInternal::handle_recheck, this));
			handle_.pause();
			break;

		default:
			assert(false);
		};
	}
	
	void writeResumeData()
	{					
		HAL_DEV_MSG(L"writeResumeData()");
		wpath_t resumeDir = workingDir_/L"resume";
		
		if (!exists(resumeDir))
			create_directory(resumeDir);
				
		bool halencode_result = halencode(resumeDir/filename_, resumedata_);
		assert(halencode_result);
		HAL_DEV_MSG(L"Written!");
	}
	
	void clearResumeData()
	{
		wpath_t resumeFile = workingDir_/L"resume"/filename_;
		
		if (exists(resumeFile))
			remove(resumeFile);
	}
	
	void finished()
	{
		if (finishTime_.is_special())
			finishTime_ = boost::posix_time::second_clock::universal_time();

		if (move_to_directory_ != L"" && move_to_directory_ != save_directory_)
		{
			handle_.move_storage(to_utf8(move_to_directory_));
			save_directory_ = move_to_directory_;
		}
	}
	
	bool isActive() const { return state_ == TorrentDetail::torrent_active;	}
	
	unsigned state() const { return state_; }
	
	void setTrackerLogin(wstring_t username, wstring_t password)
	{
		trackerUsername_ = username;
		trackerPassword_ = password;
		
		applyTrackerLogin();
	}	
	
	std::pair<wstring_t, wstring_t> getTrackerLogin() const
	{
		return make_pair(trackerUsername_, trackerPassword_);
	}
	
	const wstring_t& filename() const { return filename_; }
	
	const wstring_t& originalFilename() const { return originalFilename_; }
	
	const lbt::torrent_handle& handle() const { return handle_; }

	void resetTrackers()
	{
		if (inSession())
		{
			handle_.replace_trackers(torrent_trackers_);		
			trackers_.clear();
		}
	}
	
	void setTrackers(const std::vector<TrackerDetail>& trackerDetails)
	{
		trackers_.clear();
		trackers_.assign(trackerDetails.begin(), trackerDetails.end());
		
		applyTrackers();
	}
	
	const std::vector<TrackerDetail>& getTrackers()
	{
		if (trackers_.empty())
		{
			std::vector<lbt::announce_entry> trackers = infoMemory_.trackers();
			
			foreach (const lbt::announce_entry& entry, trackers)
			{
				trackers_.push_back(
					TrackerDetail(hal::from_utf8(entry.url), entry.tier));
			}
		}		
		return trackers_;
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

	const wstring_t& saveDirectory() { return save_directory_; }
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & make_nvp("transferLimit", transferLimit_);
        ar & make_nvp("connections", connections_);
        ar & make_nvp("uploads", uploads_);			
		ar & make_nvp("filename", filename_);		
        ar & make_nvp("saveDirectory", save_directory_);
		if (version > 0) {
			ar & make_nvp("moveToDirectory", move_to_directory_);
		} else {
			move_to_directory_ = save_directory_;
		}
		
		ar & make_nvp("payloadUploaded_", payloadUploaded_);
		ar & make_nvp("payloadDownloaded_", payloadDownloaded_);
		ar & make_nvp("uploaded_", uploaded_);
		ar & make_nvp("downloaded_", downloaded_);	
		ar & make_nvp("ratio", ratio_);	
		ar & make_nvp("trackerUsername", trackerUsername_);
		ar & make_nvp("trackerPassword", trackerPassword_);
		
		ar & make_nvp("state", state_);
		ar & make_nvp("trackers", trackers_);
		
		ar & make_nvp("resolve_countries", resolve_countries_);
		
		ar & make_nvp("file_priorities", filePriorities_);
		
		ar & make_nvp("startTime", startTime_);
		ar & make_nvp("activeDuration", activeDuration_);
		ar & make_nvp("seedingDuration", seedingDuration_);
		
		ar & make_nvp("name", name_);
		ar & make_nvp("compactStorage", compactStorage_);
		ar & make_nvp("finishTime", finishTime_);
		
		ar & make_nvp("progress", progress_);
    }

	void setEntryData(libtorrent::entry metadata, libtorrent::entry resumedata)
	{		
		metadata_ = metadata;
		resumedata_ = resumedata;
	}

	std::vector<lbt::peer_info>& peers() { return peers_; }
	
	boost::tuple<size_t, size_t, size_t, size_t> updatePeers()
	{
		if (inSession())
			handle_.get_peer_info(peers_);
		
		size_t totalPeers = 0;
		size_t peersConnected = 0;
		size_t totalSeeds = 0;
		size_t seedsConnected = 0;
		
		foreach (lbt::peer_info& peer, peers_) 
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
		
		return boost::make_tuple(totalPeers, peersConnected, totalSeeds, seedsConnected);
	}
	
	void getPeerDetails(PeerDetails& peerDetails) const
	{
		if (inSession())
		{
			foreach (lbt::peer_info peer, peers_) 
			{
				peerDetails.push_back(peer);
			}	
		}
	}

	void getFileDetails(FileDetails& fileDetails)
	{
		if (fileDetailsMemory_.empty())
		{
			lbt::torrent_info& info = infoMemory();
			std::vector<lbt::file_entry> files;
			
			std::copy(info.begin_files(), info.end_files(), 
				std::back_inserter(files));					
				
			if (filePriorities_.size() != files.size())
			{
				filePriorities_.clear();
				filePriorities_.assign(files.size(), 1);
			}
			
			for(size_t i=0, e=files.size(); i<e; ++i)
			{
				wstring_t fullPath = hal::from_utf8(files[i].path.string());
				boost::int64_t size = static_cast<boost::int64_t>(files[i].size);
				
				fileDetailsMemory_.push_back(FileDetail(fullPath, size, 0, filePriorities_[i], i));
			}	
		}		
		
		if (inSession())
		{			
			std::vector<float> fileProgress;			
			handle_.file_progress(fileProgress);
			
			for(size_t i=0, e=fileDetailsMemory_.size(); i<e; ++i)
				fileDetailsMemory_[i].progress =  fileProgress[i];			
		}

		for(size_t i=0, e=fileDetailsMemory_.size(); i<e; ++i)
			fileDetailsMemory_[i].priority =  filePriorities_[i];
		
		fileDetails = fileDetailsMemory_;
	}
	
	void prepare(wpath_t filename, wpath_t saveDirectory)
	{
		mutex_t::scoped_lock l(mutex_);
		
		if (fs::exists(filename)) 
			metadata_ = haldecode(filename);
		
		extractNames(metadata_);			
		
		const wpath_t resumeFile = workingDir_/L"resume"/filename_;
		const wpath_t torrentFile = workingDir_/L"torrents"/filename_;
		
		event().post(shared_ptr<EventDetail>(new EventMsg(
			wformat_t(L"File: %1%, %2%.") % resumeFile % torrentFile)));
		
		if (exists(resumeFile)) 
			resumedata_ = haldecode(resumeFile);

		if (!exists(workingDir_/L"torrents"))
			create_directory(workingDir_/L"torrents");

		if (!exists(torrentFile))
			copy_file(filename.string(), torrentFile);

		if (!exists(saveDirectory))
			create_directory(saveDirectory);

		if (state_ == TorrentDetail::torrent_stopping)
			state_ = TorrentDetail::torrent_stopped;
		else if (state_ == TorrentDetail::torrent_pausing)
			state_ = TorrentDetail::torrent_paused;
	}
	
	void extractNames(lbt::entry& metadata)
	{
		mutex_t::scoped_lock l(mutex_);
		
		lbt::torrent_info info(metadata);				
		name_ = hal::from_utf8_safe(info.name());
		
		filename_ = name_;
		if (!boost::find_last(filename_, L".torrent")) 
				filename_ += L".torrent";
		
		event().post(shared_ptr<EventDetail>(new EventMsg(
			wformat_t(L"Loaded names: %1%, %2%") % name_ % filename_)));
	}
	
	lbt::torrent_info& infoMemory()
	{
		if (!infoMemory_.is_valid()) infoMemory_ = lbt::torrent_info(metadata_);
		
		return infoMemory_;
	}
	
	signalers& signals()
	{
		mutex_t::scoped_lock l(mutex_);
		return signals_;
	}

private:	
	signalers signals_;

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
		mutex_t::scoped_lock l(mutex_);
		if (inSession())
		{
			int down = (transferLimit_.first > 0) ? static_cast<int>(transferLimit_.first*1024) : -1;
			handle_.set_download_limit(down);
			
			int up = (transferLimit_.second > 0) ? static_cast<int>(transferLimit_.second*1024) : -1;
			handle_.set_upload_limit(up);

			HAL_DEV_MSG(wformat_t(L"Applying Transfer Speed %1% - %2%") % down % up);
		}
	}

	void applyConnectionLimit()
	{
		mutex_t::scoped_lock l(mutex_);
		if (inSession())
		{
			handle_.set_max_connections(connections_);
			handle_.set_max_uploads(uploads_);

			HAL_DEV_MSG(wformat_t(L"Applying Connection Limit %1% - %2%") % connections_ % uploads_);
		}
	}
	
	void applyRatio()
	{ 
		mutex_t::scoped_lock l(mutex_);
		if (inSession())
		{
			handle_.set_ratio(ratio_);

			HAL_DEV_MSG(wformat_t(L"Applying Ratio %1%") % ratio_);
		}
	}
	
	void applyTrackers()
	{
		mutex_t::scoped_lock l(mutex_);
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
			
			HAL_DEV_MSG(L"Applying Trackers");
		}
	}
	
	void applyTrackerLogin()
	{
		mutex_t::scoped_lock l(mutex_);
		if (inSession())
		{
			if (trackerUsername_ != L"")
			{
				handle_.set_tracker_login(hal::to_utf8(trackerUsername_),
					hal::to_utf8(trackerPassword_));
			}

			HAL_DEV_MSG(wformat_t(L"Applying Tracker Login User: %1%, Pass: %2%") % trackerUsername_ % trackerPassword_ );
		}
	}
	
	void applyFilePriorities()
	{		
		mutex_t::scoped_lock l(mutex_);
		if (inSession()) 
		{
			if (!filePriorities_.empty())
				handle_.prioritize_files(filePriorities_);
			
			HAL_DEV_MSG(L"Applying File Priorities");
		}
	}	
	
	void applyResolveCountries()
	{
		mutex_t::scoped_lock l(mutex_);
		if (inSession())
		{
			handle_.resolve_countries(resolve_countries_);
			
			HAL_DEV_MSG(wformat_t(L"Applying Resolve Countries %1%") % resolve_countries_);
		}
	}
	
	void completed_pause()
	{
		mutex_t::scoped_lock l(mutex_);
		assert(inSession());
		assert(handle_.is_paused());	
				
		state_ = TorrentDetail::torrent_paused;	

		HAL_DEV_MSG(L"completed_pause()");
	}

	void completed_stop()
	{
		mutex_t::scoped_lock l(mutex_);
		assert(inSession());
		assert(handle_.is_paused());	
		
		state_ = TorrentDetail::torrent_stopped;
		
		removeFromSession();
		assert(!inSession());

		HAL_DEV_MSG(L"completed_stop()");
	}

	void handle_recheck()
	{
		mutex_t::scoped_lock l(mutex_);
		state_ = TorrentDetail::torrent_stopped;

		removeFromSession(false);
		assert(!inSession());

		resume();
		assert(inSession());

		HAL_DEV_MSG(L"handle_recheck()");
	}
		
	static lbt::session* the_session_;
	static wpath_t workingDir_;
	
	mutable mutex_t mutex_;
	
	std::pair<float, float> transferLimit_;
	
	unsigned state_;
	int connections_;
	int uploads_;
	bool in_session_;
	float ratio_;
	bool resolve_countries_;
	
	wstring_t filename_;
	wstring_t name_;
	wstring_t save_directory_;
	wstring_t move_to_directory_;
	wstring_t originalFilename_;
	lbt::torrent_handle handle_;	
	
	lbt::entry metadata_;
	lbt::entry resumedata_;
	
	wstring_t trackerUsername_;	
	wstring_t trackerPassword_;
	
	boost::int64_t totalUploaded_;
	boost::int64_t totalBase_;
	
	TransferTracker<boost::int64_t> payloadUploaded_;
	TransferTracker<boost::int64_t> payloadDownloaded_;
	TransferTracker<boost::int64_t> uploaded_;
	TransferTracker<boost::int64_t> downloaded_;
	
	pt::ptime startTime_;
	pt::ptime finishTime_;
	DurationTracker activeDuration_;
	DurationTracker seedingDuration_;
	
	std::vector<TrackerDetail> trackers_;
	std::vector<lbt::announce_entry> torrent_trackers_;
	std::vector<lbt::peer_info> peers_;	
	std::vector<int> filePriorities_;
	
	float progress_;
	
	lbt::torrent_info infoMemory_;
	lbt::torrent_status statusMemory_;
	FileDetails fileDetailsMemory_;
	
	bool compactStorage_;
};

typedef std::map<std::string, TorrentInternalOld> TorrentMap;
typedef std::pair<std::string, TorrentInternalOld> TorrentPair;
typedef shared_ptr<TorrentInternal> TorrentInternal_ptr;

class TorrentManager : 
	public hal::IniBase<TorrentManager>
{
	typedef TorrentManager thisClass;
	typedef hal::IniBase<thisClass> iniClass;

	struct TorrentHolder
	{
		mutable TorrentInternal_ptr torrent;
		
		wstring_t filename;
		wstring_t name;		
		
		TorrentHolder()
		{}
		
		explicit TorrentHolder(TorrentInternal_ptr t) :
			torrent(t), filename(torrent->filename()), name(torrent->name())
		{}
						
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			if (version < 1)
			{
				TorrentInternalOld t;
				ar & make_nvp("torrent", t);
				
				torrent.reset(new TorrentInternal(t));
			}
			else
			{
				ar & make_nvp("torrent", torrent);
			} 
			
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
					TorrentHolder, wstring_t, &TorrentHolder::filename> 
				>,
			boost::multi_index::ordered_unique<
				boost::multi_index::tag<byName>,
				boost::multi_index::member<
					TorrentHolder, wstring_t, &TorrentHolder::name> 
				>
		>
	> TorrentMultiIndex;
	
public:
	typedef TorrentMultiIndex::index<byFilename>::type torrentByFilename;
	typedef TorrentMultiIndex::index<byName>::type torrentByName;
	
	TorrentManager(ini_file& ini) :
		iniClass("bittorrent", "TorrentManager", ini)
	{}
	
	TorrentManager& operator=(const TorrentMap& map)
	{
		torrents_.clear();
		
		for (TorrentMap::const_iterator i=map.begin(), e=map.end(); i != e; ++i)
		{	
			TorrentInternal_ptr TIp(new TorrentInternal((*i).second));
			
			event().post(shared_ptr<EventDetail>(new EventMsg(
				wformat_t(L"Converting %1%.") % TIp->name())));
			
			torrents_.insert(TorrentHolder(TIp));
		}
		
		return *this;
	}
	
	std::pair<torrentByName::iterator, bool> insert(const TorrentHolder& h)
	{
		return torrents_.get<byName>().insert(h);
	}
	
	std::pair<torrentByName::iterator, bool> insert(TorrentInternal_ptr t)
	{
		return insert(TorrentHolder(t));
	}

	TorrentInternal_ptr getByFile(const wstring_t& filename)
	{
		torrentByFilename::iterator it = torrents_.get<byFilename>().find(filename);
		
		if (it != torrents_.get<byFilename>().end() && (*it).torrent)
		{
			return (*it).torrent;
		}
		
		throw invalidTorrent(filename);
	}
	
	TorrentInternal_ptr get(const wstring_t& name)
	{
		torrentByName::iterator it = torrents_.get<byName>().find(name);
		
		if (it != torrents_.get<byName>().end() && (*it).torrent)
		{
			return (*it).torrent;
		}
		
		throw invalidTorrent(name);
	}
	
	torrentByName::iterator erase(torrentByName::iterator where)
	{
		return torrents_.get<byName>().erase(where);
	}
	
	size_t size()
	{
		return torrents_.size();
	}
	
	size_t erase(const wstring_t& name)
	{
		return torrents_.get<byName>().erase(name);
	}
	
	bool exists(const wstring_t& name)
	{
		torrentByName::iterator it = torrents_.get<byName>().find(name);
		
		if (it != torrents_.get<byName>().end())
			return true;
		else
			return false;
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
	mutex_t::scoped_lock l(mutex_);

	connections_ = maxConn;
	uploads_ = maxUpload;
	
	applyConnectionLimit();
}

std::pair<int, int> TorrentInternal::getConnectionLimit()
{
	return std::make_pair(connections_, uploads_);
}

void TorrentInternal::setTransferSpeed(float download, float upload)
{	
	mutex_t::scoped_lock l(mutex_);

	transferLimit_ = std::make_pair(download, upload);
	
	applyTransferSpeed();
}

std::pair<float, float> TorrentInternal::getTransferSpeed()
{
	return transferLimit_;
}

TorrentDetail_ptr TorrentInternal::getTorrentDetail_ptr()
{	
	mutex_t::scoped_lock l(mutex_);

	try
	{

	if (inSession())
	{
		statusMemory_ = handle_.status();
		progress_ = statusMemory_.progress;
	}
	else
	{
		// Wipe these cause they don't make sense for a non-active torrent.
		
		statusMemory_.download_payload_rate = 0;
		statusMemory_.upload_payload_rate = 0;
		statusMemory_.next_announce = boost::posix_time::seconds(0);		
	}
	
	wstring_t state;
	
	switch (state_)
	{
	case TorrentDetail::torrent_paused:
		state = app().res_wstr(HAL_TORRENT_PAUSED);
		break;
		
	case TorrentDetail::torrent_pausing:
		state = app().res_wstr(HAL_TORRENT_PAUSING);
		break;
		
	case TorrentDetail::torrent_stopped:
		state = app().res_wstr(HAL_TORRENT_STOPPED);
		break;
		
	case TorrentDetail::torrent_stopping:
		state = app().res_wstr(HAL_TORRENT_STOPPING);
		break;
		
	default:
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
	
	pt::time_duration td(pt::pos_infin);
	
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
		
		if (lbt::torrent_status::seeding == statusMemory_.state)
			seedingDuration_.update();
	}	
	
	boost::tuple<size_t, size_t, size_t, size_t> connections = updatePeers();		

	return TorrentDetail_ptr(new TorrentDetail(name_, filename_, saveDirectory(), state, hal::from_utf8(statusMemory_.current_tracker), 
		std::pair<float, float>(statusMemory_.download_payload_rate, statusMemory_.upload_payload_rate),
		progress_, statusMemory_.distributed_copies, statusMemory_.total_wanted_done, statusMemory_.total_wanted, uploaded_, payloadUploaded_,
		downloaded_, payloadDownloaded_, connections, ratio_, td, statusMemory_.next_announce, activeDuration_, seedingDuration_, startTime_, finishTime_));

	}
	catch (const lbt::invalid_handle&)
	{
		event().post(shared_ptr<EventDetail>(
			new EventInvalidTorrent(Event::critical, Event::invalidTorrent, to_utf8(name_), "getTorrentDetail_ptr")));
	}
	catch (const std::exception& e)
	{
		event().post(shared_ptr<EventDetail>(
			new EventTorrentException(Event::critical, Event::torrentException, e.what(), to_utf8(name_), "getTorrentDetail_ptr")));
	}
	
	return TorrentDetail_ptr(new TorrentDetail(name_, filename_, saveDirectory(), app().res_wstr(HAL_TORRENT_STOPPED), app().res_wstr(HAL_NA)));
}

} // namespace hal

BOOST_CLASS_VERSION(hal::TorrentManager::TorrentHolder, 1)

#endif // RC_INVOKED

