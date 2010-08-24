
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#if defined(HALTORRENT_PCH)
#	include "halPch.hpp"
#else
#	include "halTypes.hpp"
#endif

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
	std::wstring to_wstring(size_t index = 0) const;
	
	std::wstring ip_address;
	std::wstring country;
	std::pair<float,float> speed;
	bool seed;
	std::wstring client;
	std::wstring status;
};

class peer_details_vec : public std::set<peer_detail>
{
public:
	typedef boost::optional<const peer_detail&> optional_type;

	optional_type find_peer(const std::wstring& ip_address)
	{
		return find_peer(peer_detail(ip_address));
	}

	optional_type find_peer(const peer_detail& pd)
	{
		std::set<peer_detail>::const_iterator i = find(pd);

		if (i != end())
			return optional_type(*i);
		else
			return optional_type();
	}

private:
	//std::set<peer_detail> details_;
};

//void peer_details_sort(peer_details_vec& p, size_t index, bool cmp_less = true);

};
