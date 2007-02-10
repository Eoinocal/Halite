
#include "ini_adapter.hpp"

#include <tinyxml.h>

namespace hal 
{

void ini_adapter::load_stream_data(std::ostream& data)
{
	tinyxml::node* data_node = ini_.load(location_);
	
	if (data_node)
	{
	tinyxml::document doc;
	doc.Parse("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?><!DOCTYPE boost_serialization>");
	
	doc.LinkEndChild(data_node);
	
	data << doc;
	}
}

void ini_adapter::save_stream_data(std::istream& data)
{
	tinyxml::document doc;	
	data >> doc;
	
	tinyxml::node* data_node = doc.RootElement();
	
	ini_.save(location_, data_node->Clone());
}

}
