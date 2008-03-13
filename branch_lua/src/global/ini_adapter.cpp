
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include <iterator>
#include <strstream>

#include "ini_adapter.hpp"
#include "tinyxml.hpp"
#include "unicode.hpp"

namespace hal 
{

void ini_adapter::load_stream_data(std::ostream& data)
{
	tinyxml::node* data_node = ini_.load(location_);
	
	if (data_node)
	{
	tinyxml::document doc;
	doc.parse("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?><!DOCTYPE boost_serialization>");
	
	doc.link_end_child(data_node);
	
	data << doc;
	}
}

void ini_adapter::save_stream_data(std::istream& data)
{	
	tinyxml::document doc;	
	data >> doc;
	
	tinyxml::node* data_node = doc.root_element();
	
	ini_.save(location_, data_node->clone());
}

void ini_adapter::load_stream_data(std::wostream& data)
{
	tinyxml::node* data_node = ini_.load(location_);
	
	if (data_node)
	{
		tinyxml::document doc;
		doc.parse("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\
			<!DOCTYPE boost_serialization>");
		doc.link_end_child(data_node);
		
		std::stringstream str;
		str << doc;
		
		unicode::transcode<unicode::utf8, unicode::wchar_encoding>(
		   std::istreambuf_iterator<char> (str),
		   std::istreambuf_iterator<char> (),
		   std::ostreambuf_iterator<wchar_t> (data)
		);
	}
}

void ini_adapter::save_stream_data(std::wistream& data)
{
	std::stringstream sstr;
	
	unicode::transcode<unicode::wchar_encoding, unicode::utf8>(
	   std::istreambuf_iterator<wchar_t> (data),
	   std::istreambuf_iterator<wchar_t> (),
	   std::ostreambuf_iterator<char> (sstr)
	);
	
	tinyxml::document doc;	
	sstr >> doc;
	
	tinyxml::node* data_node = doc.root_element();
	
	ini_.save(location_, data_node->clone());
}

} // namespace hal
