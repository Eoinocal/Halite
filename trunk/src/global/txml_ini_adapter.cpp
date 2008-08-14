
//         Copyright Eóin O'Callaghan 2008 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "../stdAfx.hpp"
#include "wtl_app.hpp"
#include "logger.hpp"
#include "string_conv.hpp"

#include <iterator>
#include <strstream>

#include <boost/format.hpp>

#include "txml_ini_adapter.hpp"
#include "txml.hpp"
#include "unicode.hpp"

#define TXML_ARCHIVE_LOGGING

#ifndef TXML_ARCHIVE_LOGGING
#	define TXML_LOG(s)
#else
#	include "../halEvent.hpp"
#	define TXML_LOG(msg) \
	hal::event_log.post(boost::shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(msg, hal::event_logger::xml_dev))) 
#endif

namespace hal 
{

xml::node* txml_ini_adapter::get_load_data_node()
{
	TXML_LOG(L"Ini Adpater loading");

	xml::node* data_node = 0;

	std::vector<boost::filesystem::path>::const_iterator 
		i = locations_.begin(), e = locations_.end();

	while (i!=e && !data_node)
	{
		TXML_LOG(boost::wformat(L" -- %1%") % to_wstr_shim(i->string()));

		data_node = ini_.load(*i);
		++i;
	}


	return data_node;
}

bool txml_ini_adapter::load_stream_data(std::ostream& data)
{
	xml::node* data_node = get_load_data_node();
	
	if (data_node)
	{
		xml::document doc;
		doc.parse("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?><!DOCTYPE boost_serialization>");
		
		doc.link_end_child(data_node);
		
		data << doc;

		return true;
	}

	return false;
}

void txml_ini_adapter::save_stream_data(std::istream& data)
{		
	TXML_LOG(boost::wformat(L"Ini Adapter Saving; %1%") % to_wstr_shim(locations_.front().string()));
	
	xml::document doc;	
	data >> doc;
	
	xml::node* data_node = doc.root_element();

	TXML_LOG(L"Data streamed");
	
	bool ret = ini_.save(locations_.front(), data_node->clone());

	TXML_LOG(boost::wformat(L" -- save %1%") % (ret ? L"successful" : L"failed"));
}

bool txml_ini_adapter::load_stream_data(std::wostream& data)
{
	xml::node* data_node = get_load_data_node();
	
	if (data_node)
	{
		xml::document doc;
		doc.parse("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>");
		doc.link_end_child(data_node);
		
		std::stringstream str;
		str << doc;
		
		unicode::transcode<unicode::utf8, unicode::wchar_encoding>(
		   std::istreambuf_iterator<char> (str),
		   std::istreambuf_iterator<char> (),
		   std::ostreambuf_iterator<wchar_t> (data)
		);

		return true;
	}
	return false;
}

void txml_ini_adapter::save_stream_data(std::wistream& data)
{
	std::stringstream sstr;
	
	unicode::transcode<unicode::wchar_encoding, unicode::utf8>(
	   std::istreambuf_iterator<wchar_t> (data),
	   std::istreambuf_iterator<wchar_t> (),
	   std::ostreambuf_iterator<char> (sstr)
	);
	
	xml::document doc;	
	sstr >> doc;
	
	xml::node* data_node = doc.root_element();
	
	ini_.save(locations_.front(), data_node->clone());
}

} // namespace aux
