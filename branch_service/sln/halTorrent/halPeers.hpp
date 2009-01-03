
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <stlsoft/properties/method_properties.hpp>
#include <stlsoft/util/operator_bool_adaptor.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

#include "halTypes.hpp"

namespace libtorrent { struct peer_info; }

namespace hal 
{

struct peer_detail 
{
	enum details
	{
		ip_address_e = 0,
		country_e,
		speed_down_e,
		speed_up_e,
		seed_e,
		client_e,
		status_e
	};

	peer_detail(const std::wstring& ip_address) :
		ip_address(ip_address)
	{}
	peer_detail(libtorrent::peer_info& peer_info);
	
	bool operator==(const peer_detail& peer) const
	{
		return (ip_address == peer.ip_address);
	}
	
	bool operator<(const peer_detail& peer) const
	{
		return (ip_address < peer.ip_address);
	}
	
	bool less(const peer_detail& r, size_t index = 0) const;
	std::wstring to_wstring(size_t index = 0);
	
	std::wstring ip_address;
	std::wstring country;
	std::pair<float,float> speed;
	bool seed;
	std::wstring client;
	std::wstring status;
};

typedef boost::shared_ptr<peer_detail> peer_detail_ptr;
typedef std::vector<peer_detail> peer_details_vec;

void peer_details_sort(peer_details_vec& p, size_t index, bool cmp_less = true);

};
