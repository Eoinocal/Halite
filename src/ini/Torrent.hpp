
#pragma once

#include <string>
#include <map>
#include <boost/smart_ptr.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/filesystem/path.hpp>

#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/peer_connection.hpp>

namespace halite
{

class torrentDetail;
typedef boost::shared_ptr<torrentDetail> torrentDetails;

class TorrentInternal
{
public:
	TorrentInternal() :		
		transferLimit_(std::pair<float, float>(0, 0)),
		connections_(0),
		uploads_(0),
		paused_(false),
		filename_(),
		inSession(false)
	{}

	TorrentInternal(std::pair<float, float> tL, int c, int u, bool p, bool inS,
			std::wstring f, libtorrent::torrent_handle h) :
		transferLimit_(tL),
		connections_(c),
		uploads_(u),
		paused_(p),
		inSession(inS),
		filename_(f),
		handle_(h)
	{}
	
	torrentDetails getTorrentDetails() const;
	void setTransferLimit(float down, float up);
	void setHandle(libtorrent::torrent_handle h) { handle_ = h; }
	
	const libtorrent::torrent_handle& handle() const { return handle_; } 
	
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(transferLimit_);
        ar & BOOST_SERIALIZATION_NVP(connections_);
        ar & BOOST_SERIALIZATION_NVP(uploads_);
        ar & BOOST_SERIALIZATION_NVP(paused_);
        ar & BOOST_SERIALIZATION_NVP(filename_);
    }
	
private:		
	std::pair<float, float> transferLimit_;
	
	int connections_;
	int uploads_;
	bool paused_;
	bool inSession;
	
	std::wstring filename_;
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
