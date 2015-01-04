
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "halPch.hpp"
//#include "Halite.hpp"

#include "global/string_conv.hpp"
#include "global/wtl_app.hpp"

#include "halIni.hpp"
#include "halConfig.hpp"
//#include "ProgressDialog.hpp"

namespace hal
{

Config::Config() :
	hal::IniBase<Config>(L"globals/bittorrent", L"Config"),
	globals_(),
	torrent_defaults_(),
	port_range_(10000,30000),
	use_port_range_(true),
	randomize_port_(true),
	enable_dht_(true),
	dht_settings_(),
	dht_random_port_(0),
	dht_upper_port_(0),
	dht_radio_(1),
	enable_ip_filter_(false),
	enableProxy(false),
	proxyPort(0),
	use_move_to_(false),
	save_prompt_(true),
	enable_pe_(true),
	pe_settings_(),
	half_connections_(true),
	half_connections_limit_(10),
	mapping_upnp_(true),
	mapping_nat_pmp_(false),
	resolve_countries_(false),
	ut_metadata_plugin_(true),
	announce_all_trackers_(true),
	announce_all_tiers_(true),
	ut_pex_plugin_(true),
	smart_ban_plugin_(true),
	lt_trackers_plugin_(true),
	queue_settings_(bind(&hal::bit::set_queue_settings, &bittorrent(), _1))
{
	if (hal::app().get_my_documents())
	{
		default_save_folder_ = (hal::app().get_my_documents()->parent_path()/L"Halite Downloads"/L"Incoming").wstring();
		default_move_folder_ = (hal::app().get_my_documents()->parent_path()/L"Halite Downloads"/L"Completed").wstring();
	}
	else
	{
		default_save_folder_ = (hal::app().exe_path().parent_path()/L"Incoming").wstring();
		default_move_folder_ = (hal::app().exe_path().parent_path()/L"Completed").wstring();
	}

	queue_settings_.set_no_action(hal::bittorrent().get_queue_settings());
	timeouts_ = hal::bittorrent().get_timeouts();
}

bool Config::settingsChanged()
{
//	thread settings(bind(&BitTConfig::settingsThread, this));
	return settingsThread();
}

bool Config::settingsThread()
{	
	win32_exception::install_handler();
	std::srand(GetTickCount());

	try
	{

	event_log().post(shared_ptr<EventDetail>(new EventMsg(L"Applying BitTorrent session settings.")));

	unsigned listen_port = port_range_.first;
	int current_port = bittorrent().is_listening_on();

	bittorrent().set_mapping(false, false);
	
	try
	{
	if (enable_ip_filter_)
	{
		if (pdc_)
		{
			pdc_(L"Loading IP filters...", boost::bind(
				&bit::ensure_ip_filter_on, &bittorrent(), _1));
		}
		else
			bittorrent().ensure_ip_filter_on(progress_callback());
	}
	else
		bittorrent().ensure_ip_filter_off();
	}
	catch(const std::exception& e)
	{
		event_log().post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(event_logger::critical, e, L"settingsThread, Load IP Filter"))); 
	}	

	try
	{
	if (enable_pe_)
	{
		bittorrent().ensure_pe_on(pe_settings_);
	}
	else
		bittorrent().ensure_pe_off();
	}
	catch(const std::exception& e)
	{
		hal::event_log().post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(event_logger::critical, e, L"settingsThread, Protocol Encryption"))); 
	}
	
	bittorrent().set_session_half_open_limit(half_connections_limit_);	
	
	bittorrent().set_session_limits(globals_.total, globals_.uploads);
	bittorrent().set_session_speed(globals_.download_rate, globals_.upload_rate);
		
	bittorrent().set_torrent_defaults(torrent_defaults_);

	bittorrent().set_timeouts(timeouts_);	
//	bittorrent().set_queue_settings(queue_settings_);
	bittorrent().set_resolve_countries(resolve_countries_);
	bittorrent().set_announce_to_all(announce_all_trackers_, announce_all_tiers_);

	if (use_custom_interface_)
		bittorrent().set_external_interface(custom_interface_);
	else
		bittorrent().set_external_interface();

	if (ut_metadata_plugin_)
		bittorrent().start_ut_metadata_plugin();
	if (ut_pex_plugin_)
		bittorrent().start_ut_pex_plugin();
	if (smart_ban_plugin_)
		bittorrent().start_smart_ban_plugin();
	if (lt_trackers_plugin_)
		bittorrent().start_lt_trackers_plugin();

	try
	{		

	if (randomize_port_ && (current_port < port_range_.first || current_port > port_range_.second))
	{
		for (int i=0, e=10; i!=e; ++i)
		{
			unsigned range = port_range_.second - port_range_.first;
			listen_port = port_range_.first + (range * std::rand())/RAND_MAX;

			event_log().post(shared_ptr<EventDetail>(new EventMsg(
					hal::wform(L"Attempting port %1%.") % listen_port)));

			bool success = bittorrent().listen_on(std::make_pair(listen_port, listen_port));
			if (success) break;
		}
	}
	else
	{
		event_log().post(shared_ptr<EventDetail>(new EventMsg(
				hal::wform(L"Trying port in range %1% - %2%.") % port_range_.first % port_range_.second)));

		bool success = bittorrent().listen_on(port_range_);
		if (!success)
		{
			hal::event_log().post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventDebug(event_logger::critical, L"settingsThread, Listen")));
			
			return false;
		}
	}

	}
	catch(const std::exception& e)
	{
		hal::event_log().post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(event_logger::critical, e, L"settingsThread, Listen"))); 
		
		return false;
	}


	event_log().post(shared_ptr<EventDetail>(
		new EventMsg(hal::wform(L"Opened listen port; %1%.") % bittorrent().is_listening_on())));

	
	if (enable_dht_)
	{
		unsigned old_port = dht_settings_.service_port;

		if (dht_random_port_)
		{
			unsigned range = dht_upper_port_ - dht_settings_.service_port;
			dht_settings_.service_port = dht_settings_.service_port + (range * std::rand())/RAND_MAX;
		}
		else if (dht_radio_ == 1)
		{
			dht_settings_.service_port = bittorrent().is_listening_on();
		}

		if (!bittorrent().ensure_dht_on(dht_settings_))
		{
			bittorrent().ensure_dht_off();
			
			hal::event_log().post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventDebug(event_logger::critical, L"settingsThread, DHT Error"))); 
		}
		
		dht_settings_.service_port = old_port;
	}
	else
		bittorrent().ensure_dht_off();

	
	bittorrent().set_mapping(mapping_upnp_, mapping_nat_pmp_);

		
	// Settings seem to have applied ok!
	
	save_to_ini();	
	return true;

	} HAL_GENERIC_FN_EXCEPTION_CATCH(L"Config::settingsThread()")

	return false;
}

Config& config()
{
	static Config c;
	return c;
}

} // namespace hal
