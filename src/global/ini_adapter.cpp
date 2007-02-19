
#include "ini_adapter.hpp"
#include "tinyxml.hpp"

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

} // namespace hal
