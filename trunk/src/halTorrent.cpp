
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include <libtorrent/file.hpp>
#include <libtorrent/hasher.hpp>
#include <libtorrent/storage.hpp>
#include <libtorrent/file_pool.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/ip_filter.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/peer_connection.hpp>
#include <libtorrent/extensions/metadata_transfer.hpp>
#include <libtorrent/extensions/ut_pex.hpp>

#include "global/wtl_app.hpp"
#include "global/string_conv.hpp"
#include "global/ini_adapter.hpp"

#include "halTorrent.hpp"
#include "halTypes.hpp"
#include "halEvent.hpp"
#include "halSignaler.hpp"

#include "halTorrentInternal.hpp"
#include "halSession.hpp"
//#include "halSessionAlert.hpp"

namespace hal 
{
	libtorrent::session* torrent_internal::the_session_ = 0;
	wpath torrent_internal::workingDir_;
}

namespace hal 
{

bit& bittorrent()
{
	static bit t;
	return t;
}

const PeerDetails& torrent_details::peerDetails() const
{
	if (!peerDetailsFilled_)
	{
		bittorrent().getAllPeerDetails(hal::to_utf8(name_), peerDetails_);
		peerDetailsFilled_ = true;
	}
	
	return peerDetails_;
}

const FileDetails& torrent_details::fileDetails() const
{
	if (!fileDetailsFilled_)
	{
		bittorrent().getAllFileDetails(hal::to_utf8(name_), fileDetails_);
		fileDetailsFilled_ = true;
	}
	
	return fileDetails_;
}

bool nameLess(const torrent_details_ptr& left, const torrent_details_ptr& right)
{
	return left->state() < right->state();
}

void torrent_details_manager::sort(
	boost::function<bool (const torrent_details_ptr&, const torrent_details_ptr&)> fn) const
{
	std::stable_sort(torrents_.begin(), torrents_.end(), fn);
}

web_seed_or_dht_node_detail::web_seed_or_dht_node_detail() : 
	url(L""), 
	port(-1), 
	type(hal::app().res_wstr(HAL_INT_NEWT_ADD_PEERS_WEB)) 
{}

web_seed_or_dht_node_detail::web_seed_or_dht_node_detail(std::wstring u) : 
	url(u), 
	port(-1), 
	type(hal::app().res_wstr(HAL_INT_NEWT_ADD_PEERS_WEB)) 
{}

web_seed_or_dht_node_detail::web_seed_or_dht_node_detail(std::wstring u, int p) : 
	url(u), 
	port(p), 
	type(hal::app().res_wstr(HAL_INT_NEWT_ADD_PEERS_DHT)) 
{}
wpath bit_impl::workingDirectory = hal::app().working_directory();

bit::bit() :
	pimpl(new bit_impl())
{}

#define HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH(FUNCTION) \
catch (const libt::invalid_handle&) \
{\
	event_log.post(shared_ptr<EventDetail>( \
		new EventInvalidTorrent(event_logger::critical, event_logger::invalidTorrent, name, std::string(FUNCTION)))); \
}\
catch (const invalidTorrent& t) \
{\
	event_log.post(shared_ptr<EventDetail>( \
		new EventInvalidTorrent(event_logger::info, event_logger::invalidTorrent, t.who(), std::string(FUNCTION)))); \
}\
catch (const std::exception& e) \
{\
	event_log.post(shared_ptr<EventDetail>( \
		new EventTorrentException(event_logger::critical, event_logger::torrentException, std::string(e.what()), name, std::string(FUNCTION)))); \
}

void bit::shutDownSession()
{
	pimpl.reset();
}

void bit::save_torrent_data()
{
	pimpl->save_torrent_data();
}

bool bit::create_torrent(const create_torrent_params& params, fs::wpath out_file, progress_callback fn)
{
	return pimpl->create_torrent(params, out_file, fn);
}

bit::torrent bit::get_wstr(const std::wstring& filename)
{
	return bit::torrent(pimpl->the_torrents_.get(filename));
}

bool bit::listen_on(std::pair<int, int> const& range)
{
	return pimpl->listen_on(range);
}

int bit::is_listening_on() 
{
	return pimpl->is_listening_on();
}

void bit::stop_listening()
{
	pimpl->stop_listening();
}

bool bit::ensure_dht_on()
{
	return pimpl->ensure_dht_on();
}

void bit::ensure_dht_off()
{
	pimpl->ensure_dht_off();
}

void bit::set_dht_settings(int max_peers_reply, int search_branching, 
	int service_port, int max_fail_count)
{
	pimpl->set_dht_settings(max_peers_reply, search_branching, service_port, max_fail_count);
}

void bit::set_mapping(int mapping)
{
	pimpl->set_mapping(mapping);
}

queue_settings bit::get_queue_settings()
{
	return pimpl->get_queue_settings();
}

void bit::set_queue_settings(const queue_settings& s)
{
	pimpl->set_queue_settings(s);
}

timeouts bit::get_timeouts()
{
	return pimpl->get_timeouts();
}

void bit::set_timeouts(const timeouts& t)
{
	pimpl->set_timeouts(t);
}

void bit::set_session_limits(int maxConn, int maxUpload)
{		
	pimpl->set_session_limits(maxConn, maxUpload);
}

void bit::set_session_speed(float download, float upload)
{
	pimpl->set_session_speed(download, upload);
}

bool bit::ensure_ip_filter_on(progress_callback fn)
{
	return pimpl->ensure_ip_filter_on(fn);
}

void bit::ensure_ip_filter_off()
{
	pimpl->ensure_ip_filter_off();
}

#ifndef TORRENT_DISABLE_ENCRYPTION	

void bit::ensure_pe_on(int enc_level, int in_enc_policy, int out_enc_policy, bool prefer_rc4)
{
	pimpl->ensure_pe_on(enc_level, in_enc_policy, out_enc_policy, prefer_rc4);
}

void bit::ensure_pe_off()
{
	pimpl->ensure_pe_off();
}
#endif

void bit::ip_v4_filter_block(asio::ip::address_v4 first, asio::ip::address_v4 last)
{
	pimpl->ip_filter_.add_rule(first, last, libt::ip_filter::blocked);
	pimpl->ip_filter_count();
	pimpl->ip_filter_changed_ = true;
}

void bit::ip_v6_filter_block(asio::ip::address_v6 first, asio::ip::address_v6 last)
{
	pimpl->ip_v6_filter_block(first, last);
}

size_t bit::ip_filter_size()
{
	return pimpl->ip_filter_size();
}

void bit::clear_ip_filter()
{
	pimpl->clear_ip_filter();
}

bool bit::ip_filter_import_dat(boost::filesystem::path file, progress_callback fn, bool octalFix)
{
	return pimpl->ip_filter_import_dat(file, fn, octalFix);
}

const SessionDetail bit::getSessionDetails()
{
	SessionDetail details;
	
	details.port = pimpl->session_.is_listening() ? pimpl->session_.listen_port() : -1;
	
	libt::session_status status = pimpl->session_.status();
	
	details.speed = std::pair<double, double>(status.download_rate, status.upload_rate);
	
	details.dht_on = pimpl->dht_on_;
	details.dht_nodes = status.dht_nodes;
	details.dht_torrents = status.dht_torrents;
	
	details.ip_filter_on = pimpl->ip_filter_on_;
	details.ip_ranges_filtered = pimpl->ip_filter_count_;
	
	return details;
}

void bit::setSessionHalfOpenLimit(int halfConn)
{
	pimpl->session_.set_max_half_open_connections(halfConn);

	event_log.post(shared_ptr<EventDetail>(new EventMsg(
		wformat(L"Set half-open connections limit to %1%.") % pimpl->session_.max_half_open_connections())));
}

void bit::setTorrentDefaults(int maxConn, int maxUpload, float download, float upload)
{
	pimpl->defTorrentMaxConn_ = maxConn;
	pimpl->defTorrentMaxUpload_ = maxUpload;

	event_log.post(shared_ptr<EventDetail>(new EventMsg(
		wformat(L"Set torrent connections total %1% and uploads %2%.") % maxConn % maxUpload)));

	pimpl->defTorrentDownload_ = download;
	pimpl->defTorrentUpload_ = upload;

	event_log.post(shared_ptr<EventDetail>(new EventMsg(
		wformat(L"Set torrent default rates at %1$.2fkb/s down and %2$.2fkb/s upload.") % download % upload)));
}

void bit::add_torrent(wpath file, wpath saveDirectory, bool startStopped, bool compactStorage, 
		boost::filesystem::wpath moveToDirectory, bool useMoveTo) 
{
	pimpl->add_torrent(file, saveDirectory, startStopped, compactStorage, moveToDirectory, useMoveTo);
}

const torrent_details_manager& bit::torrentDetails()
{
	return torrentDetails_;
}

const torrent_details_manager& bit::updatetorrent_details_manager(const wstring& focused, const std::set<wstring>& selected)
{
	try {
	
	mutex_t::scoped_lock l(torrentDetails_.mutex_);	
	
	torrentDetails_.clearAll(l);	
	torrentDetails_.torrents_.reserve(pimpl->the_torrents_.size());
	
	for (TorrentManager::torrentByName::iterator i=pimpl->the_torrents_.begin(), e=pimpl->the_torrents_.end(); i != e; ++i)
	{
		wstring utf8Name = (*i).torrent->name();
		torrent_details_ptr pT = (*i).torrent->gettorrent_details_ptr();
		
		if (selected.find(utf8Name) != selected.end())
		{
			torrentDetails_.selectedTorrents_.push_back(pT);
		}
		
		if (focused == utf8Name)
			torrentDetails_.selectedTorrent_ = pT;
		
		torrentDetails_.torrentMap_[(*i).torrent->name()] = pT;
		torrentDetails_.torrents_.push_back(pT);
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "updatetorrent_details_manager")
	
	return torrentDetails_;
}

void bit::resume_all()
{
	pimpl->resume_all();
}

void bit::close_all(boost::optional<report_num_active> fn)
{
	pimpl->close_all(fn);
}

PeerDetail::PeerDetail(libt::peer_info& peerInfo) :
	ipAddress(hal::from_utf8_safe(peerInfo.ip.address().to_string())),
	country(L""),
	speed(std::make_pair(peerInfo.payload_down_speed, peerInfo.payload_up_speed)),
	client(hal::from_utf8_safe(peerInfo.client))
{
	std::vector<wstring> status_vec;
	
#ifndef TORRENT_DISABLE_RESOLVE_COUNTRIES
	if (peerInfo.country[0] != 0 && peerInfo.country[1] != 0)
		country = (wformat(L"(%1%)") % hal::from_utf8_safe(string(peerInfo.country, 2))).str().c_str();
#endif	

	if (peerInfo.flags & libt::peer_info::handshake)
	{
		status_vec.push_back(app().res_wstr(HAL_PEER_HANDSHAKE));
	}		
	else if (peerInfo.flags & libt::peer_info::connecting)
	{
		status_vec.push_back(app().res_wstr(HAL_PEER_CONNECTING));
	}
	else
	{
	#ifndef TORRENT_DISABLE_ENCRYPTION		
		if (peerInfo.flags & libt::peer_info::rc4_encrypted)
			status_vec.push_back(app().res_wstr(HAL_PEER_RC4_ENCRYPTED));		
		if (peerInfo.flags & libt::peer_info::plaintext_encrypted)
			status_vec.push_back(app().res_wstr(HAL_PEER_PLAINTEXT_ENCRYPTED));
	#endif
		
		if (peerInfo.flags & libt::peer_info::interesting)
			status_vec.push_back(app().res_wstr(HAL_PEER_INTERESTING));	
		if (peerInfo.flags & libt::peer_info::choked)
			status_vec.push_back(app().res_wstr(HAL_PEER_CHOKED));	
		if (peerInfo.flags & libt::peer_info::remote_interested)
			status_vec.push_back(app().res_wstr(HAL_PEER_REMOTE_INTERESTING));	
		if (peerInfo.flags & libt::peer_info::remote_choked)
			status_vec.push_back(app().res_wstr(HAL_PEER_REMOTE_CHOKED));	
		if (peerInfo.flags & libt::peer_info::supports_extensions)
			status_vec.push_back(app().res_wstr(HAL_PEER_SUPPORT_EXTENSIONS));	
	//	if (peerInfo.flags & libt::peer_info::local_connection)						// Not sure whats up here?
	//		status_vec.push_back(app().res_wstr(HAL_PEER_LOCAL_CONNECTION));			
		if (peerInfo.flags & libt::peer_info::queued)
			status_vec.push_back(app().res_wstr(HAL_PEER_QUEUED));
	}
	
	seed = (peerInfo.flags & libt::peer_info::seed) ? true : false;
	
	if (!status_vec.empty()) status = status_vec[0];
	
	if (status_vec.size() > 1)
	{
		for (size_t i=1; i<status_vec.size(); ++i)
		{
			status += L"; ";
			status += status_vec[i];
		}
	}	
}

void bit::getAllPeerDetails(const std::string& filename, PeerDetails& peerContainer)
{
	getAllPeerDetails(from_utf8_safe(filename), peerContainer);
}

void bit::getAllPeerDetails(const std::wstring& filename, PeerDetails& peerContainer)
{
	try {
	
	pimpl->the_torrents_.get(filename)->getPeerDetails(peerContainer);
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "getAllPeerDetails")
}

void bit::getAllFileDetails(const std::string& filename, FileDetails& fileDetails)
{
	getAllFileDetails(from_utf8_safe(filename), fileDetails);
}

void bit::getAllFileDetails(const std::wstring& filename, FileDetails& fileDetails)
{
	try {
	
	pimpl->the_torrents_.get(filename)->getFileDetails(fileDetails);
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "getAllFileDetails")
}

bool bit::isTorrent(const std::string& filename)
{	
	return isTorrent(hal::to_wstr_shim(filename));
}

bool bit::isTorrent(const std::wstring& filename)
{	
	try {
	
	return pimpl->the_torrents_.exists(filename);
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "isTorrent")
	
	return false;
}

void bit::pauseTorrent(const std::string& filename)
{
	pauseTorrent(hal::to_wstr_shim(filename));
}

void bit::pauseTorrent(const std::wstring& filename)
{
	try {
	
	pimpl->the_torrents_.get(filename)->pause();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "pauseTorrent")
}

void bit::resumeTorrent(const std::string& filename)
{
	resumeTorrent(hal::to_wstr_shim(filename));
}

void bit::resumeTorrent(const std::wstring& filename)
{
	try {
	
	pimpl->the_torrents_.get(filename)->resume();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "resumeTorrent")
}

void bit::stopTorrent(const std::string& filename)
{
	stopTorrent(hal::to_wstr_shim(filename));
}

void bit::stopTorrent(const std::wstring& filename)
{
	try {
	
	pimpl->the_torrents_.get(filename)->stop();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "stopTorrent")
}

bool bit::isTorrentActive(const std::string& filename)
{
	return isTorrentActive(hal::to_wstr_shim(filename));
}

bool bit::isTorrentActive(const std::wstring& filename)
{
	try {
	
	return pimpl->the_torrents_.get(filename)->is_active();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "isTorrentActive")
	
	return false; // ??? is this correct
}

void bit::reannounceTorrent(const std::string& filename)
{
	reannounceTorrent(hal::to_wstr_shim(filename));
}

void bit::reannounceTorrent(const std::wstring& filename)
{
	try {
	
	pimpl->the_torrents_.get(filename)->handle().force_reannounce();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "reannounceTorrent")
}


void bit::recheckTorrent(const std::string& filename)
{
	recheckTorrent(hal::to_wstr_shim(filename));
}

void bit::recheckTorrent(const std::wstring& filename)
{
	try {
	
	pimpl->the_torrents_.get(filename)->force_recheck();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "recheckTorrent")
}

void bit::remove_torrent_wstr(const std::wstring& filename)
{
	pimpl->remove_torrent(filename);
}

void bit::remove_torrent_wipe_files_wstr(const std::wstring& filename)
{
	pimpl->remove_torrent_wipe_files(hal::to_wstr_shim(filename));
}

void bit::pauseAllTorrents()
{	
	try {
	
	for (TorrentManager::torrentByName::iterator i=pimpl->the_torrents_.begin(), e=pimpl->the_torrents_.end();
		i != e; ++i)
	{		
		if ((*i).torrent->in_session())
			(*i).torrent->pause();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "pauseAllTorrents")
}

void bit::unpauseAllTorrents()
{	
	try {
	
	for (TorrentManager::torrentByName::iterator i=pimpl->the_torrents_.begin(), e=pimpl->the_torrents_.end();
		i != e; ++i)
	{
		if ((*i).torrent->in_session() && (*i).torrent->state() == torrent_details::torrent_paused)
			(*i).torrent->resume();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "unpauseAllTorrents")
}

bit::torrent::torrent()
{}

bit::torrent::torrent(boost::shared_ptr<torrent_internal> p) :
	ptr(p)
{}

bool bit::torrent::is_open() const
{
	return ptr;
}

bit::torrent::exec_around_ptr::proxy::proxy(torrent_internal* t) : 
	t_(t),
	l_(t->mutex_)
{
	HAL_DEV_MSG(L"Ctor proxy");
}

bit::torrent::exec_around_ptr::proxy::~proxy() 
{
	HAL_DEV_MSG(L"Dtor proxy");
}

const std::wstring bit::torrent::get_name() const
{
	try {
	
	return ptr->name();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(L"Torrent Unknown", "torrent::get_name()")
	
	return 0;
}

float bit::torrent::get_ratio() const
{
	try {
	
	return ptr->get_ratio();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_ratio")
	
	return 0;
}

void bit::torrent::set_ratio(float r)
{
	try {

	ptr->set_ratio(r);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_ratio")
}

std::pair<int, int> bit::torrent::get_connection_limits() const
{
	try {
	
	return ptr->getConnectionLimit();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_connection_limits")
	
	return std::make_pair(-1, -1);
}

void bit::torrent::set_connection_limits(const std::pair<int, int>& l)
{
	try {
	
	ptr->setConnectionLimit(l.first, l.second);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_connection_limits")
}

std::pair<float, float> bit::torrent::get_rate_limits() const
{
	try {
	
	return ptr->getTransferSpeed();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_rate_limits")
	
	return std::pair<float, float>(-1.0, -1.0);
}

void bit::torrent::set_rate_limits(const std::pair<float, float>& l)
{
	try {
	
	ptr->setTransferSpeed(l.first, l.second);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_rate_limits")
}

wpath bit::torrent::get_save_directory() const
{
	try {
	
	return ptr->get_save_directory();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_save_directory")
	
	return L"";
}

void bit::torrent::set_save_directory(const wpath& s)
{
	try {
	
	ptr->set_save_directory(s);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_save_directory")
}

wpath bit::torrent::get_move_to_directory() const
{
	try {
	
	return ptr->get_move_to_directory();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_save_directory")
	
	return L"";
}

void bit::torrent::set_move_to_directory(const wpath& m)
{
	try {
	
	ptr->set_move_to_directory(m);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_move_to_directory")
}

std::pair<wstring, wstring> bit::torrent::get_tracker_login() const
{
	try {
	
	return ptr->getTrackerLogin();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("get_tracker_login")
	
	return std::make_pair(L"!!! exception thrown !!!", L"!!! exception thrown !!!");
}

void bit::torrent::set_tracker_login(const std::pair<wstring, wstring>& p)
{
	try {
	
	ptr->setTrackerLogin(p.first, p.second);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_tracker_login")
}

bool bit::torrent::get_is_active() const
{
	try {
	
	return ptr->is_active();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_is_active")
	
	return L"";
}

bool bit::torrent::get_in_session() const
{
	try {
	
	return ptr->in_session();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_in_session")
	
	return L"";
}

std::vector<tracker_detail> bit::torrent::get_trackers() const
{
	try {
	
	return ptr->getTrackers();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::get_trackers")
	
	return std::vector<tracker_detail>();
}

void bit::torrent::set_trackers(const std::vector<tracker_detail>& trackers)
{
	try {
	
	ptr->setTrackers(trackers);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_trackers")
}

void bit::torrent::reset_trackers()
{
	try {
	
	ptr->resetTrackers();
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_trackers")
}

void bit::torrent::set_file_priorities(const std::pair<std::vector<int>, int>& p)
{
	try { 

	ptr->setFilePriorities(p.first, p.second);
	
	} HAL_GENERIC_TORRENT_PROP_EXCEPTION_CATCH("torrent::set_trackers")
}

void bit::startEventReceiver()
{
	event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Starting event handler.")));

	pimpl->start_alert_handler();
}

void bit::stopEventReceiver()
{
	event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Stopping event handler.")));

	pimpl->stop_alert_handler();
}

int bit::defTorrentMaxConn() { return pimpl->defTorrentMaxConn_; }
int bit::defTorrentMaxUpload() { return pimpl->defTorrentMaxUpload_; }
float bit::defTorrentDownload() { return pimpl->defTorrentDownload_; }
float bit::defTorrentUpload() { return pimpl->defTorrentUpload_; }
	
};
