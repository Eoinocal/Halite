
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

const PeerDetails& TorrentDetail::peerDetails() const
{
	if (!peerDetailsFilled_)
	{
		bittorrent().getAllPeerDetails(hal::to_utf8(name_), peerDetails_);
		peerDetailsFilled_ = true;
	}
	
	return peerDetails_;
}

const FileDetails& TorrentDetail::fileDetails() const
{
	if (!fileDetailsFilled_)
	{
		bittorrent().getAllFileDetails(hal::to_utf8(name_), fileDetails_);
		fileDetailsFilled_ = true;
	}
	
	return fileDetails_;
}

bool nameLess(const TorrentDetail_ptr& left, const TorrentDetail_ptr& right)
{
	return left->state() < right->state();
}

void TorrentDetails::sort(
	boost::function<bool (const TorrentDetail_ptr&, const TorrentDetail_ptr&)> fn) const
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

#define HAL_GENERIC_TORRENT_EXCEPTION_CATCH(TORRENT, FUNCTION) \
catch (const libt::invalid_handle&) \
{\
	event_log.post(shared_ptr<EventDetail>( \
		new EventInvalidTorrent(event_logger::critical, event_logger::invalidTorrent, TORRENT, std::string(FUNCTION)))); \
}\
catch (const invalidTorrent& t) \
{\
	event_log.post(shared_ptr<EventDetail>( \
		new EventInvalidTorrent(event_logger::info, event_logger::invalidTorrent, t.who(), std::string(FUNCTION)))); \
}\
catch (const std::exception& e) \
{\
	event_log.post(shared_ptr<EventDetail>( \
		new EventTorrentException(event_logger::critical, event_logger::torrentException, std::string(e.what()), TORRENT, std::string(FUNCTION)))); \
}

void bit::shutDownSession()
{
	pimpl.reset();
}

void bit::saveTorrentData()
{
	pimpl->saveTorrentData();
}

bool bit::create_torrent(const create_torrent_params& params, fs::wpath out_file, progress_callback fn)
{
	return pimpl->create_torrent(params, out_file, fn);
}

bit::torrent bit::get_wstr(const std::wstring& filename)
{
	return bit::torrent(pimpl->theTorrents.get(filename));
}

bool bit::listenOn(std::pair<int, int> const& range)
{
	return pimpl->listenOn(range);
}

int bit::isListeningOn() 
{
	return pimpl->isListeningOn();
}

void bit::stopListening()
{
	pimpl->stopListening();
}

bool bit::ensureDhtOn()
{
	return pimpl->ensureDhtOn();
}

void bit::ensureDhtOff()
{
	pimpl->ensureDhtOff();
}

void bit::setDhtSettings(int max_peers_reply, int search_branching, 
	int service_port, int max_fail_count)
{
	pimpl->setDhtSettings(max_peers_reply, search_branching, service_port, max_fail_count);
}

void bit::setMapping(int mapping)
{
	pimpl->setMapping(mapping);
}

void bit::setTimeouts(int peers, int tracker)
{
	pimpl->setTimeouts(peers, tracker);
}

void bit::setSessionLimits(int maxConn, int maxUpload)
{		
	pimpl->setSessionLimits(maxConn, maxUpload);
}

void bit::setSessionSpeed(float download, float upload)
{
	pimpl->setSessionSpeed(download, upload);
}

bool bit::ensureIpFilterOn(progress_callback fn)
{
	return pimpl->ensureIpFilterOn(fn);
}

void bit::ensureIpFilterOff()
{
	pimpl->ensureIpFilterOff();
}

#ifndef TORRENT_DISABLE_ENCRYPTION	

void bit::ensurePeOn(int enc_level, int in_enc_policy, int out_enc_policy, bool prefer_rc4)
{
	pimpl->ensurePeOn(enc_level, in_enc_policy, out_enc_policy, prefer_rc4);
}

void bit::ensurePeOff()
{
	pimpl->ensurePeOff();
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

void bit::clearIpFilter()
{
	pimpl->clearIpFilter();
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

std::pair<libt::entry, libt::entry> bit_impl::prepTorrent(wpath filename, wpath saveDirectory)
{
	libt::entry metadata = haldecode(filename);
	libt::torrent_info info(metadata);
 	
	wstring torrentName = hal::from_utf8_safe(info.name());
	if (!boost::find_last(torrentName, L".torrent")) 
		torrentName += L".torrent";
	
	wpath torrentFilename = torrentName;
	const wpath resumeFile = workingDirectory/L"resume"/torrentFilename.leaf();
	
	//  vvv Handle old naming style!
	const wpath oldResumeFile = workingDirectory/L"resume"/filename.leaf();
	
	if (filename.leaf() != torrentFilename.leaf() && exists(oldResumeFile))
		fs::rename(oldResumeFile, resumeFile);
	//  ^^^ Handle old naming style!	
	
	libt::entry resumeData;	
	
	if (fs::exists(resumeFile)) 
	{
		try 
		{
			resumeData = haldecode(resumeFile);
		}
		catch(std::exception &e) 
		{		
			hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventStdException(event_logger::critical, e, L"prepTorrent, Resume"))); 
	
			fs::remove(resumeFile);
		}
	}

	if (!fs::exists(workingDirectory/L"torrents"))
		fs::create_directory(workingDirectory/L"torrents");

	if (!fs::exists(workingDirectory/L"torrents"/torrentFilename.leaf()))
		fs::copy_file(filename.string(), workingDirectory/L"torrents"/torrentFilename.leaf());

	if (!fs::exists(saveDirectory))
		fs::create_directory(saveDirectory);
	
	return std::make_pair(metadata, resumeData);
}

void bit::addTorrent(wpath file, wpath saveDirectory, bool startStopped, bool compactStorage, 
		boost::filesystem::wpath moveToDirectory, bool useMoveTo) 
{
	try 
	{	
	torrent_internal_ptr TIp;

	std::pair<std::string, std::string> names = extract_names(file);
	wstring xml_name = from_utf8(names.first) + L".xml";

	if (fs::exists(file.branch_path()/xml_name))
	{
		torrent_standalone tsa;
		
		if (tsa.load_standalone(file.branch_path()/xml_name))
		{
			TIp = tsa.torrent;
			
			TIp->set_save_directory(saveDirectory, true);			
			if (useMoveTo)
				TIp->set_move_to_directory(moveToDirectory);

			TIp->prepare(file);
		}
	}

	if (!TIp)
	{
		if (useMoveTo)
			TIp.reset(new torrent_internal(file, saveDirectory, compactStorage, moveToDirectory));		
		else
			TIp.reset(new torrent_internal(file, saveDirectory, compactStorage));

		TIp->setTransferSpeed(bittorrent().defTorrentDownload(), bittorrent().defTorrentUpload());
		TIp->setConnectionLimit(bittorrent().defTorrentMaxConn(), bittorrent().defTorrentMaxUpload());
	}
	
	std::pair<TorrentManager::torrentByName::iterator, bool> p =
		pimpl->theTorrents.insert(TIp);
	
	if (p.second)
	{
		torrent_internal_ptr me = pimpl->theTorrents.get(TIp->name());		
		
		if (!startStopped) 
			me->add_to_session();
		else
			me->set_state_stopped();
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(to_utf8(file.string()), "addTorrent")
}

const TorrentDetails& bit::torrentDetails()
{
	return torrentDetails_;
}

const TorrentDetails& bit::updateTorrentDetails(const wstring& focused, const std::set<wstring>& selected)
{
	try {
	
	mutex_t::scoped_lock l(torrentDetails_.mutex_);	
	
	torrentDetails_.clearAll(l);	
	torrentDetails_.torrents_.reserve(pimpl->theTorrents.size());
	
	for (TorrentManager::torrentByName::iterator i=pimpl->theTorrents.begin(), e=pimpl->theTorrents.end(); i != e; ++i)
	{
		wstring utf8Name = (*i).torrent->name();
		TorrentDetail_ptr pT = (*i).torrent->getTorrentDetail_ptr();
		
		if (selected.find(utf8Name) != selected.end())
		{
			torrentDetails_.selectedTorrents_.push_back(pT);
		}
		
		if (focused == utf8Name)
			torrentDetails_.selectedTorrent_ = pT;
		
		torrentDetails_.torrentMap_[(*i).torrent->name()] = pT;
		torrentDetails_.torrents_.push_back(pT);
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "updateTorrentDetails")
	
	return torrentDetails_;
}

void bit::resumeAll()
{
	try {
		
	event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Resuming torrent.")));
	
	for (TorrentManager::torrentByName::iterator i=pimpl->theTorrents.begin(), e=pimpl->theTorrents.end(); i != e;)
	{
		wpath file = wpath(pimpl->workingDirectory)/L"torrents"/(*i).torrent->filename();
		
		if (exists(file))
		{		
			try 
			{
				
			(*i).torrent->prepare(file);	

			switch ((*i).torrent->state())
			{
				case TorrentDetail::torrent_stopped:
					break;
				case TorrentDetail::torrent_paused:
					(*i).torrent->add_to_session(true);
					break;
				case TorrentDetail::torrent_active:
					(*i).torrent->add_to_session(false);
					break;
				default:
					assert(false);
			};
			
			++i;
			
			}
			catch(const libt::duplicate_torrent&)
			{
				hal::event_log.post(shared_ptr<hal::EventDetail>(
					new hal::EventDebug(hal::event_logger::debug, L"Encountered duplicate torrent")));
				
				++i; // Harmless, don't worry about it.
			}
			catch(const std::exception& e) 
			{
				hal::event_log.post(shared_ptr<hal::EventDetail>(
					new hal::EventStdException(hal::event_logger::warning, e, L"resumeAll")));
				
				pimpl->theTorrents.erase(i++);
			}			
		}
		else
		{
			pimpl->theTorrents.erase(i++);
		}
	}
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "resumeAll")
}

void bit::closeAll(boost::optional<report_num_active> fn)
{
	try 
	{	
	event_log.post(shared_ptr<EventDetail>(new EventInfo(L"Saving torrent data...")));

	pimpl->saveTorrentData();

	event_log.post(shared_ptr<EventDetail>(new EventInfo(L"Stopping all torrents...")));
	
	for (TorrentManager::torrentByName::iterator i=pimpl->theTorrents.begin(), e=pimpl->theTorrents.end(); 
		i != e; ++i)
	{
		(*i).torrent->stop();
	}
	
	// Ok this polling loop here is a bit curde, but a blocking wait is actually appropiate.
	for (int num_active = -1; num_active != 0; )
	{
		num_active = 0;

		for (TorrentManager::torrentByName::iterator i=pimpl->theTorrents.begin(), e=pimpl->theTorrents.end(); 
				i != e; ++i)
		{
			if ((*i).torrent->state() != TorrentDetail::torrent_stopped)
				++num_active;
		}
		
		event_log.post(shared_ptr<EventDetail>(new EventInfo(wformat(L"%1% still active") % num_active)));

		if (fn)	(*fn)(num_active);
		Sleep(200);
	}
	
	event_log.post(shared_ptr<EventDetail>(new EventInfo(L"All torrents stopped.")));		
	event_log.post(shared_ptr<EventDetail>(new EventInfo(L"Fast-resume data written.")));
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "closeAll")
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
	
	pimpl->theTorrents.get(filename)->getPeerDetails(peerContainer);
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "getAllPeerDetails")
}

void bit::getAllFileDetails(const std::string& filename, FileDetails& fileDetails)
{
	getAllFileDetails(from_utf8_safe(filename), fileDetails);
}

void bit::getAllFileDetails(const std::wstring& filename, FileDetails& fileDetails)
{
	try {
	
	pimpl->theTorrents.get(filename)->getFileDetails(fileDetails);
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "getAllFileDetails")
}

bool bit::isTorrent(const std::string& filename)
{	
	return isTorrent(hal::to_wstr_shim(filename));
}

bool bit::isTorrent(const std::wstring& filename)
{	
	try {
	
	return pimpl->theTorrents.exists(filename);
	
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
	
	pimpl->theTorrents.get(filename)->pause();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "pauseTorrent")
}

void bit::resumeTorrent(const std::string& filename)
{
	resumeTorrent(hal::to_wstr_shim(filename));
}

void bit::resumeTorrent(const std::wstring& filename)
{
	try {
	
	pimpl->theTorrents.get(filename)->resume();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "resumeTorrent")
}

void bit::stopTorrent(const std::string& filename)
{
	stopTorrent(hal::to_wstr_shim(filename));
}

void bit::stopTorrent(const std::wstring& filename)
{
	try {
	
	pimpl->theTorrents.get(filename)->stop();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "stopTorrent")
}

bool bit::isTorrentActive(const std::string& filename)
{
	return isTorrentActive(hal::to_wstr_shim(filename));
}

bool bit::isTorrentActive(const std::wstring& filename)
{
	try {
	
	return pimpl->theTorrents.get(filename)->is_active();
	
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
	
	pimpl->theTorrents.get(filename)->handle().force_reannounce();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "reannounceTorrent")
}

void bit_impl::removalThread(torrent_internal_ptr pIT, bool wipeFiles)
{
	try {

	if (!wipeFiles)
	{
		session_.remove_torrent(pIT->handle());
	}
	else
	{
		if (pIT->in_session())
		{
			session_.remove_torrent(pIT->handle(), libt::session::delete_files);
		}
		else
		{
			libt::torrent_info m_info = pIT->infoMemory();
			
			// delete the files from disk
			std::string error;
			std::set<std::string> directories;
			
			for (libt::torrent_info::file_iterator i = m_info.begin_files(true)
				, end(m_info.end_files(true)); i != end; ++i)
			{
				std::string p = (hal::path_to_utf8(pIT->saveDirectory()) / i->path).string();
				fs::path bp = i->path.branch_path();
				
				std::pair<std::set<std::string>::iterator, bool> ret;
				ret.second = true;
				while (ret.second && !bp.empty())
				{
					std::pair<std::set<std::string>::iterator, bool> ret = 
						directories.insert((hal::path_to_utf8(pIT->saveDirectory()) / bp).string());
					bp = bp.branch_path();
				}
				if (!fs::remove(hal::from_utf8(p).c_str()) && errno != ENOENT)
					error = std::strerror(errno);
			}

			// remove the directories. Reverse order to delete subdirectories first

			for (std::set<std::string>::reverse_iterator i = directories.rbegin()
				, end(directories.rend()); i != end; ++i)
			{
				if (!fs::remove(hal::from_utf8(*i).c_str()) && errno != ENOENT)
					error = std::strerror(errno);
			}
		}
	}

	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH("Torrent Unknown!", "removalThread")
}

void bit::removeTorrent(const std::string& filename)
{
	removeTorrent(hal::to_wstr_shim(filename));
}

void bit::removeTorrent(const std::wstring& filename)
{
	try {
	
	torrent_internal_ptr pTI = pimpl->theTorrents.get(filename);
	libt::torrent_handle handle = pTI->handle();
	pimpl->theTorrents.erase(filename);
	
	thread_t t(bind(&bit_impl::removalThread, &*pimpl, pTI, false));	
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "removeTorrent")
}

void bit::recheckTorrent(const std::string& filename)
{
	recheckTorrent(hal::to_wstr_shim(filename));
}

void bit::recheckTorrent(const std::wstring& filename)
{
	try {
	
	pimpl->theTorrents.get(filename)->force_recheck();
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "recheckTorrent")
}

void bit::removeTorrentWipeFiles(const std::string& filename)
{
	removeTorrentWipeFiles(hal::to_wstr_shim(filename));
}

void bit::removeTorrentWipeFiles(const std::wstring& filename)
{
	try {
	
	torrent_internal_ptr pTI = pimpl->theTorrents.get(filename);
	libt::torrent_handle handle = pTI->handle();
	pimpl->theTorrents.erase(filename);
	
	thread_t t(bind(&bit_impl::removalThread, &*pimpl, pTI, true));	
	
	} HAL_GENERIC_TORRENT_EXCEPTION_CATCH(filename, "removeTorrentWipeFiles")
}

void bit::pauseAllTorrents()
{	
	try {
	
	for (TorrentManager::torrentByName::iterator i=pimpl->theTorrents.begin(), e=pimpl->theTorrents.end();
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
	
	for (TorrentManager::torrentByName::iterator i=pimpl->theTorrents.begin(), e=pimpl->theTorrents.end();
		i != e; ++i)
	{
		if ((*i).torrent->in_session() && (*i).torrent->state() == TorrentDetail::torrent_paused)
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
	pimpl->keepChecking_ = true;
	thread_t(bind(&asio::io_service::run, &pimpl->io_));
}

void bit::stopEventReceiver()
{
	event_log.post(shared_ptr<EventDetail>(new EventMsg(L"Stopping event Handler.")));

	pimpl->stopAlertHandler();
}

int bit::defTorrentMaxConn() { return pimpl->defTorrentMaxConn_; }
int bit::defTorrentMaxUpload() { return pimpl->defTorrentMaxUpload_; }
float bit::defTorrentDownload() { return pimpl->defTorrentDownload_; }
float bit::defTorrentUpload() { return pimpl->defTorrentUpload_; }
	
};
