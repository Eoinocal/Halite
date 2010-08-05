
//         Copyright E�in O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#pragma warning (push, 1)
#	include <libtorrent/peer_id.hpp>
#	include <libtorrent/peer_connection.hpp>
#pragma warning (pop) 


namespace libtorrent
{

template<class Addr>
bool operator==(const libtorrent::ip_range<Addr>& lhs, const int flags)
{
	return (lhs.flags == flags);
}

inline std::ostream& operator<<(std::ostream& os, libtorrent::ip_range<asio::ip::address_v4>& ip)
{
	os << ip.first.to_ulong();
	os << ip.last.to_ulong();
	
	return os;
}

} // namespace libtorrent


namespace boost {
namespace serialization {


template<class Archive, class address_type>
void save(Archive& ar, const address_type& ip, const unsigned int version)
{	
	unsigned long addr = ip.to_ulong();	
	ar & BOOST_SERIALIZATION_NVP(addr);
}

template<class Archive, class address_type>
void load(Archive& ar, address_type& ip, const unsigned int version)
{	
	unsigned long addr;
	ar & BOOST_SERIALIZATION_NVP(addr);	
	ip = address_type(addr);
}


template<class Archive, class String, class Traits>
void save(Archive& ar, const boost::filesystem::basic_path<String, Traits>& p, const unsigned int version)
{	
	String str = p.string();
	ar & BOOST_SERIALIZATION_NVP(str);
}

template<class Archive, class String, class Traits>
void load(Archive& ar, boost::filesystem::basic_path<String, Traits>& p, const unsigned int version)
{	
	String str;
	ar & BOOST_SERIALIZATION_NVP(str);

	p = str;
}


template<class Archive, class String, class Traits>
inline void serialize(
        Archive & ar,
        boost::filesystem::basic_path<String, Traits>& p,
        const unsigned int file_version)
{
        split_free(ar, p, file_version);            
}


template<class Archive, class address_type>
void serialize(Archive& ar, libtorrent::ip_range<address_type>& addr, const unsigned int version)
{	
	ar & BOOST_SERIALIZATION_NVP(addr.first);
	ar & BOOST_SERIALIZATION_NVP(addr.last);
	addr.flags = libtorrent::ip_filter::blocked;
}


template<class Archive>
void serialize(Archive& ar, hal::tracker_detail& tracker, const unsigned int version)
{	
	ar & BOOST_SERIALIZATION_NVP(tracker.url);
	ar & BOOST_SERIALIZATION_NVP(tracker.tier);
}


template<class Archive>
void save(Archive& ar, const libtorrent::big_number& num, const unsigned int version)
{	
	std::stringstream hex_number;
	hex_number << num;

	ar & BOOST_SERIALIZATION_NVP(hex_number.str());
}

template<class Archive>
void load(Archive& ar, libtorrent::big_number& num, const unsigned int version)
{	
	std::string hex_number;
	ar & BOOST_SERIALIZATION_NVP(hex_number);	

	std::stringstream hex_stream(hex_number);
	hex_stream >> num;
}

} // namespace serialization
} // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(libtorrent::big_number)

BOOST_SERIALIZATION_SPLIT_FREE(asio::ip::address_v4)
BOOST_SERIALIZATION_SPLIT_FREE(asio::ip::address_v6)