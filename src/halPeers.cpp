
//         Copyright Eóin O'Callaghan 2008 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"
#include <functional>

#include "global/wtl_app.hpp"
#include "global/string_conv.hpp"
#include "global/ini_adapter.hpp"

#include "halTorrent.hpp"
#include "halTypes.hpp"
#include "halEvent.hpp"
#include "halSignaler.hpp"

#include "halTorrentInternal.hpp"
#include "halTorrentManager.hpp"
#include "halSession.hpp"

namespace hal 
{

peer_detail::peer_detail(libt::peer_info& peerInfo) :
	ip_address(hal::from_utf8_safe(peerInfo.ip.address().to_string())),
	country(L""),
	speed(std::make_pair(peerInfo.payload_down_speed, peerInfo.payload_up_speed)),
	client(hal::from_utf8_safe(peerInfo.client))
{
	std::vector<wstring> status_vec;
	
#ifndef TORRENT_DISABLE_RESOLVE_COUNTRIES
	if (peerInfo.country[0] != 0 && peerInfo.country[1] != 0)
		country = (hal::wform(L"(%1%)") % hal::from_utf8_safe(string(peerInfo.country, 2))).str().c_str();
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

bool peer_detail::less(const peer_detail& r, size_t index) const
{	
	switch (index)
	{
	case ip_address_e: return ip_address < r.ip_address;
	case country_e: return country < r.country;

	case speed_down_e: return speed.first < r.speed.first;
	case speed_up_e: return speed.second < r.speed.second;

	case seed_e: return seed < r.seed;
	case client_e: return client < r.client;
	case status_e: return status < r.status;

	default: return false; // ???
	};
}

std::wstring peer_detail::to_wstring(size_t index) const
{
	switch (index)
	{
	case ip_address_e: return ip_address;
	case country_e: return country;

	case speed_down_e: return (wform(L"%1$.2fkb/s") % (speed.first/1024)).str(); 
	case speed_up_e: return (wform(L"%1$.2fkb/s") % (speed.second/1024)).str();

	case seed_e: return seed ? L"Seed" : L"";
	case client_e: return client; 
	case status_e: return status; 

	default: return L"(Undefined)"; // ???
	};
}

/*
void peer_details_sort(peer_details_vec& p, size_t index, bool cmp_less)
{
	std::stable_sort(p.begin(), p.end(), 
		bind(&hal_details_compare<const peer_detail&>, _1, _2, index, cmp_less));
}
*/
};
