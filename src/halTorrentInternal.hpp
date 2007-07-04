
#pragma once

namespace hal 
{
class TorrentInternal;
}

BOOST_CLASS_VERSION(hal::TorrentInternal, 3)

namespace hal 
{

namespace lbt = libtorrent;
namespace fs = boost::filesystem;

using fs::path;
using fs::ifstream;
using fs::ofstream;
using boost::serialization::make_nvp;

class TorrentInternal
{
	friend class BitTorrent_impl;
	
public:
	TorrentInternal() :	
		transferLimit_(std::pair<float, float>(-1, -1)),
		connections_(-1),
		uploads_(-1),
		ratio_(0),
		resolve_countries_(true),
		state_(TorrentDetail::torrent_active),
		in_session_(false),
		totalUploaded_(0),
		totalBase_(0)
	{}
	
	TorrentInternal(libtorrent::torrent_handle h, std::wstring f, wpath saveDirectory) :
		transferLimit_(std::pair<float, float>(bittorrent().defTorrentDownload(), bittorrent().defTorrentUpload())),
		connections_(bittorrent().defTorrentMaxConn()),
		uploads_(bittorrent().defTorrentMaxUpload()),
		ratio_(0),
		resolve_countries_(true),                            // **********
		state_(TorrentDetail::torrent_active),
		filename_(f),
		save_directory_(saveDirectory.string()),
		in_session_(true),
		handle_(h),
		totalUploaded_(0),
		totalBase_(0)
	{}
	
	TorrentDetail_ptr getTorrentDetails(bool with_peers) const;
	void setTransferSpeed(float down, float up);
	void setConnectionLimit(int maxConn, int maxUpload);
	void setTransferSpeed();
	void setConnectionLimit();
	pair<float, float> getTransferSpeed();
	pair<int, int> getConnectionLimit();
	
	void setRatio(float ratio) 
	{ 
		if (ratio < 0) ratio = 0;
		
		handle_.set_ratio(ratio);
		ratio_ = ratio; 
	}
	
	void setRatio()
	{ 		
		handle_.set_ratio(ratio_);
	}
	
	float getRatio()
	{
		return ratio_;
	}
	
	void setResolveCountries()
	{
		handle_.resolve_countries(resolve_countries_);
	}
	
	void resume()
	{
		if (!in_session_ && the_session_) 
		{
			handle_ = the_session_->add_torrent(metadata_, 
				to_utf8(save_directory_.c_str()), resumedata_);
				
			in_session_ = true;
			applySettings();
		}		
		assert(in_session_);
		
		handle_.resume();
		state_ = TorrentDetail::torrent_active;
	}
	
	void pause()
	{
		if (state_ != TorrentDetail::torrent_stopped)
		{	
			if (!in_session_ && the_session_) 
			{
				handle_ = the_session_->add_torrent(metadata_, 
					to_utf8(save_directory_.c_str()), resumedata_);
					
				in_session_ = true;
				applySettings();
			}
			assert(in_session_);
			
			handle_.pause();
			state_ = TorrentDetail::torrent_paused;	
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
	
	void setTrackerLogin()
	{
		if (trackerUsername_ != L"")
		{
			handle_.set_tracker_login(hal::to_utf8(trackerUsername_),
				hal::to_utf8(trackerPassword_));
		}
	}
	
	void setTrackerLogin(wstring username, wstring password)
	{
		trackerUsername_ = username;
		trackerPassword_ = password;
		setTrackerLogin();
	}	
	
	pair<wstring, wstring> getTrackerLogin()
	{
		return make_pair(trackerUsername_, trackerPassword_);
	}
	
	const libtorrent::torrent_handle& handle() const { return handle_; }
	void setHandle(libtorrent::torrent_handle h) 
	{ 
		handle_ = h; 
		in_session_ = true;
	}
	
	void resetTrackers()
	{
		handle_.replace_trackers(torrent_trackers_);		
		trackers_.clear();	
	}
	
	const std::vector<TrackerDetail>& getTrackers()
	{
		if (trackers_.empty())
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
	
	void applyTrackers()
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
	
	void setTrackers(const std::vector<TrackerDetail>& trackerDetails)
	{
		trackers_.clear();
		trackers_.assign(trackerDetails.begin(), trackerDetails.end());
		
		applyTrackers();
	}
	
	bool inSession() const { return in_session_; }
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
		
		if (version > 0) {
			ar & make_nvp("trackerUsername", trackerUsername_);
			ar & make_nvp("trackerPassword", trackerPassword_);
		}
		if (version > 1) {
			ar & make_nvp("state", state_);
			ar & make_nvp("trackers", trackers_);
			ar & make_nvp("totalUploaded", totalUploaded_);
			ar & make_nvp("ratio", ratio_);
		}		
		if (version > 2) {
			ar & make_nvp("resolve_countries", resolve_countries_);
		}
    }
	
	void setEntryData(libtorrent::entry metadata, libtorrent::entry resumedata)
	{		
		metadata_ = metadata;
		resumedata_ = resumedata;
	}
	
	void applySettings()
	{		
		setTransferSpeed();
		setConnectionLimit();
		setRatio();
		applyTrackers();
		setResolveCountries();
	}
	
	void getPeerDetails(PeerDetails& peerDetails) const
	{
		if (in_session_)
		{
			std::vector<lbt::peer_info> peerInfo;
			handle_.get_peer_info(peerInfo);
			
			foreach (lbt::peer_info peer, peerInfo) 
			{
				peerDetails.push_back(peer);
			}	
		}
	}
	
private:
	static libtorrent::session* the_session_;
	
	std::pair<float, float> transferLimit_;
	
	unsigned state_;
	int connections_;
	int uploads_;
	bool in_session_;
	float ratio_;
	bool resolve_countries_;
	
	std::wstring filename_;
	std::wstring save_directory_;
	libtorrent::torrent_handle handle_;	
	
	libtorrent::entry metadata_;
	libtorrent::entry resumedata_;
	
	std::wstring trackerUsername_;	
	std::wstring trackerPassword_;
	
	mutable boost::int64_t totalUploaded_;
	mutable boost::int64_t totalBase_;
	
	std::vector<TrackerDetail> trackers_;
	std::vector<lbt::announce_entry> torrent_trackers_;
};

typedef std::map<std::string, TorrentInternal> TorrentMap;
typedef std::pair<std::string, TorrentInternal> TorrentPair;

void TorrentInternal::setConnectionLimit(int maxConn, int maxUpload)
{
	handle_.set_max_connections(maxConn);
	handle_.set_max_uploads(maxUpload);

	connections_ = 	maxConn;
	uploads_ = maxUpload;
}

void TorrentInternal::setConnectionLimit()
{
	handle_.set_max_connections(connections_);
	handle_.set_max_uploads(uploads_);
}

pair<int, int> TorrentInternal::getConnectionLimit()
{
	return make_pair(connections_, uploads_);
}

void TorrentInternal::setTransferSpeed(float download, float upload)
{
	int down = (download > 0) ? static_cast<int>(download*1024) : -1;
	handle_.set_download_limit(down);
	int up = (upload > 0) ? static_cast<int>(upload*1024) : -1;
	handle_.set_upload_limit(up);
	
	transferLimit_ = make_pair(download, upload);
}

void TorrentInternal::setTransferSpeed()
{
	int down = (transferLimit_.first > 0) ? static_cast<int>(transferLimit_.first*1024) : -1;
	handle_.set_download_limit(down);
	int up = (transferLimit_.second > 0) ? static_cast<int>(transferLimit_.second*1024) : -1;
	handle_.set_upload_limit(up);
}

pair<float, float> TorrentInternal::getTransferSpeed()
{
	return transferLimit_;
}

TorrentDetail_ptr TorrentInternal::getTorrentDetails(bool with_peers) const
{
	if (inSession())
	{
		lbt::torrent_status tS = handle_.status();
		wstring state;
		
		if (state_ == TorrentDetail::torrent_paused)
			state = app().res_wstr(HAL_TORRENT_PAUSED);//L"Paused";
		else if (state_ == TorrentDetail::torrent_stopped)
			state = app().res_wstr(HAL_TORRENT_STOPPED);//L"Stopped";
		else
		{
			switch (tS.state)
			{
			case lbt::torrent_status::queued_for_checking:
				state = app().res_wstr(HAL_TORRENT_QUEUED_CHECKING);//L"Queued For Checking";
				break;
			case lbt::torrent_status::checking_files:
				state = app().res_wstr(HAL_TORRENT_CHECKING_FILES);//L"Checking Files";
				break;
			case lbt::torrent_status::connecting_to_tracker:
				state = app().res_wstr(HAL_TORRENT_CONNECTING);//L"Connecting To Tracker";
				break;
			case lbt::torrent_status::downloading_metadata:
				state = app().res_wstr(HAL_TORRENT_METADATA);//L"Downloading Metadata";
				break;
			case lbt::torrent_status::downloading:
				state = app().res_wstr(HAL_TORRENT_DOWNLOADING);//L"Downloading";
				break;
			case lbt::torrent_status::finished:
				state = app().res_wstr(HAL_TORRENT_FINISHED);//L"Finished";
				break;
			case lbt::torrent_status::seeding:
				state = app().res_wstr(HAL_TORRENT_SEEDING);//L"Seeding";
				break;
			case lbt::torrent_status::allocating:
				state = app().res_wstr(HAL_TORRENT_ALLOCATING);//L"Allocating";
				break;
			}	
		}
		
		boost::posix_time::time_duration td(boost::posix_time::pos_infin);
		
		if (tS.download_payload_rate != 0)
		{
			td = boost::posix_time::seconds(	
				long( float(tS.total_wanted-tS.total_wanted_done) / tS.download_payload_rate ));
		}
		
		totalUploaded_ += (tS.total_payload_upload - totalBase_);
		totalBase_ = tS.total_payload_upload;

		TorrentDetail_ptr td_p(new TorrentDetail(filename_, state, hal::from_utf8(tS.current_tracker), 
			pair<float, float>(tS.download_payload_rate, tS.upload_payload_rate),
			tS.progress, tS.distributed_copies, tS.total_wanted_done, tS.total_wanted, totalUploaded_,
			tS.num_peers, tS.num_seeds, ratio_, td, tS.next_announce));
		
		if (with_peers) getPeerDetails(td_p->peerDetails_);
		
		return td_p;
	}
	else
	{
		return TorrentDetail_ptr(new TorrentDetail(filename_, L"Not in Session", L"No tracker"));
	}
}

} //namespace hal
