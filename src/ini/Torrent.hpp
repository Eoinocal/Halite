
#pragma once

#include <map>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/filesystem/path.hpp>

#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/peer_connection.hpp>

namespace halite
{

class TorrentDetail;
typedef boost::shared_ptr<TorrentDetail> TorrentDetail_ptr;

class TorrentInternal
{
public:
	TorrentInternal() :		
		transferLimit_(std::pair<float, float>(-1, -1)),
		connections_(-1),
		uploads_(-1),
		paused_(false),
		inSession_(false)
	{}
	
	TorrentInternal(libtorrent::torrent_handle h, std::wstring f, path saveDirectory) :		
		transferLimit_(std::pair<float, float>(-1, -1)),
		connections_(-1),
		uploads_(-1),
		paused_(false),
		filename_(f),
		saveDirectory_(saveDirectory.string()),
		inSession_(true),
		handle_(h)
	{}
	
	TorrentDetail_ptr getTorrentDetails() const;
	void setTransferSpeed(float down, float up);
	void setConnectionLimit(int maxConn, int maxUpload);
	void setTransferSpeed();
	void setConnectionLimit();
	pair<float, float> getTransferSpeed();
	pair<int, int> getConnectionLimit();
	void pause();
	void resume();
	bool isPaused() const;
	
	const libtorrent::torrent_handle& handle() const { return handle_; }
	void setHandle(libtorrent::torrent_handle h) 
	{ 
		handle_ = h; 
		inSession_ = true;
	}	 
	
	bool inSession() const { return inSession_; }
	const string& saveDirectory() { return saveDirectory_; }
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(transferLimit_);
        ar & BOOST_SERIALIZATION_NVP(connections_);
        ar & BOOST_SERIALIZATION_NVP(uploads_);
        ar & BOOST_SERIALIZATION_NVP(paused_);
        ar & BOOST_SERIALIZATION_NVP(filename_);
        ar & BOOST_SERIALIZATION_NVP(saveDirectory_);
    }
	
private:		
	std::pair<float, float> transferLimit_;
	
	int connections_;
	int uploads_;
	bool paused_;
	bool inSession_;
	
	std::wstring filename_;
	std::string saveDirectory_;
	libtorrent::torrent_handle handle_;	
};

typedef std::map<std::string, TorrentInternal> TorrentMap;

class TorrentConfig
{
public:
	TorrentConfig()
	{}
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(torrents);
    }
	
	friend class BitTorrent_impl;

private:	
	TorrentMap torrents;	
};

};
