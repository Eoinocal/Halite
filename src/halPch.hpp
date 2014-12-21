
#pragma once

#define WINVER				0x0600
#define _WIN32_IE			0x0700
#define _RICHEDIT_VER		0x0200

#ifndef VC_EXTRALEAN
#	define VC_EXTRALEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

// Include very common C++ and Boost libraries

#include <string>
#include <vector>

#include <functional>
#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <map>
#include <algorithm>
#include <string>
#include <vector>

#include <boost/regex.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>
#include <boost/function.hpp>

#include <boost/utility/in_place_factory.hpp>
#include <boost/none.hpp>

#include <boost/mpl/list.hpp>

#include <boost/lambda/lambda.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>

#include <boost/xpressive/xpressive.hpp>

#pragma warning (push)
#pragma warning (disable : 4099)
#pragma warning (disable : 4267)
#pragma warning (disable : 4244)
#	include <boost/archive/text_woarchive.hpp>
#	include <boost/archive/text_wiarchive.hpp>
#	include <boost/archive/binary_woarchive.hpp>
#	include <boost/archive/binary_wiarchive.hpp>
#	include <boost/archive/text_oarchive.hpp>
#	include <boost/archive/text_iarchive.hpp>
#	include <boost/archive/binary_oarchive.hpp>
#	include <boost/archive/binary_iarchive.hpp>
#	include <boost/archive/basic_xml_archive.hpp>
#	include <boost/archive/xml_woarchive.hpp>
#	include <boost/archive/xml_wiarchive.hpp>
#	include <boost/archive/xml_oarchive.hpp>
#	include <boost/archive/xml_iarchive.hpp>

#	include <boost/serialization/version.hpp>
#	include <boost/serialization/vector.hpp>
#	include <boost/serialization/map.hpp>
#	include <boost/serialization/split_free.hpp>
#	include <boost/serialization/vector.hpp>
#	include <boost/serialization/shared_ptr.hpp>
#pragma warning (pop)

#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>

#include <boost/tuple/tuple.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/member.hpp>

#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_serialize.hpp>

#define TORRENT_MAX_ALERT_TYPES 48

#pragma warning (push, 1)
#	include <libtorrent/file.hpp>
#	include <libtorrent/hasher.hpp>
#	include <libtorrent/storage.hpp>
#	include <libtorrent/file_pool.hpp>
#	include <libtorrent/alert_types.hpp>
#	include <libtorrent/entry.hpp>
#	include <libtorrent/bencode.hpp>
#	include <libtorrent/upnp.hpp>
#	include <libtorrent/natpmp.hpp>
#	include <libtorrent/session.hpp>
#	include <libtorrent/ip_filter.hpp>
#	include <libtorrent/torrent_handle.hpp>
//#	include <libtorrent/dht_tracker.hpp>
#	include <libtorrent/create_torrent.hpp>
#	include <libtorrent/peer_connection.hpp>
#	include <libtorrent/peer_info.hpp>
#	include <libtorrent/peer_id.hpp>
#	include <libtorrent/peer_connection.hpp>

#	include <libtorrent/extensions/metadata_transfer.hpp>
#	include <libtorrent/extensions/ut_pex.hpp>
#	include <libtorrent/extensions/ut_metadata.hpp>
#	include <libtorrent/extensions/smart_ban.hpp>
#	include <libtorrent/extensions/lt_trackers.hpp>
#pragma warning (pop) 

#include "halTypes.hpp"
