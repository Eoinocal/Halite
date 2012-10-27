
#pragma once

#include "global/crtp_iarchive.hpp"
#include "global/string_conv.hpp"
#include "global/txml.hpp"

namespace hal { namespace xml
{
		
class txml_iarchive : 
	public crtp_iarchive<txml_iarchive>
{
public:
	txml_iarchive(std::istream& is, unsigned int flags = 0) :
		crtp_iarchive<txml_iarchive>(is, flags),
		boost_xml_compat_(false),
		previous_child_node_(0)
	{		
		is >> xml_;

		current_node_ = xml_.root_element();

		if (current_node_)
		{
			boost_xml_compat_ = (current_node_->value_str() == "boost_serialization");
			TXML_LOG(boost::wformat(L" << boost_serialization compatibility %1%") % (boost_xml_compat_ ? "on" : "off"));

			std::string signature = read_attribute<std::string>("signature");
			int version = read_attribute<int>("version");

			TXML_LOG(boost::wformat(L" << siganture: %1%, version: %2%") % from_utf8(signature) % version);
		}
	}

	~txml_iarchive()
	{}

	template<typename T>
	T read_attribute(const char* attribute_name, const char* fallback_name=0)
	{
		T type = T();

		if (current_node_)
		{
			TXML_LOG(boost::wformat(L" << attribute_name: %1%\n") % from_utf8(attribute_name));

			xml::element* e = current_node_->to_element();
			if (!e) return type;

			int result = e->query_value_attribute(attribute_name, &type);

			if (result == xml::TIXML_NO_ATTRIBUTE && fallback_name != 0)
			{
				TXML_LOG(boost::wformat(L" << -- fallback_name: %1%") % from_utf8(fallback_name));

				result = e->query_value_attribute(fallback_name, &type);
			}

			assert(result == xml::TIXML_SUCCESS);

			TXML_LOG(boost::wformat(L" << -- value: %1%\n") % type);
		}
		
		return type;
	}	

	template<>
	std::string read_attribute(const char* attribute_name, const char* fallback_name)
	{
		std::string type;

		if (current_node_)
		{
			TXML_LOG(boost::wformat(L" << attribute_name: %1%\n") % from_utf8(attribute_name));

			xml::element* e = current_node_->to_element();
			if (!e) return type;

			int result = e->query_value_attribute(attribute_name, &type);

			if (result == xml::TIXML_NO_ATTRIBUTE && fallback_name != 0)
			{
				TXML_LOG(boost::wformat(L" << -- fallback_name: %1%") % from_utf8(fallback_name));

				result = e->query_value_attribute(fallback_name, &type);
			}

			assert(result == xml::TIXML_SUCCESS);

			TXML_LOG(boost::wformat(L" << -- string value: %1%\n") % from_utf8(type));
		}
	
		return type;
	}
	
	bool load_start(const char* name)
	{
		node_stack_.push(current_node_);

		if (current_node_ && name)
		{
			TXML_LOG(boost::wformat(L" << load_start: %1%") % from_utf8(name));

			xml::node* failsafe_current = current_node_;

			boost::filesystem::path location(name);

			if (previous_child_node_ &&
					previous_child_branch_ == location.parent_path())
			{
			//	TXML_LOG(boost::wformat(L" << previous_child: %1%") % previous_child_node_->to_element()->get_text());
				failsafe_current = previous_child_node_->next_sibling(location.filename().string().c_str());
				previous_child_node_ = 0;
				
				if (!failsafe_current) 
					failsafe_current = current_node_->first_child(location.filename().string());
			}
			else
			{
				BOOST_FOREACH(boost::filesystem::path elem, location)
				{
					TXML_LOG(boost::wformat(L" >> >> %1%") % from_utf8(elem.string()));

					if (elem == ".")
					{}
					else if (elem == "..")
					{
						failsafe_current = failsafe_current->parent();
					}
					else
					{
						failsafe_current = failsafe_current->first_child(elem.string());
						
						if (!failsafe_current) return false;
					}
				}
			}

		//	xml::node* n = new xml::element(leaf);			

			if (!failsafe_current) 
				return false;
			else
			{
				current_node_ = failsafe_current;

				previous_child_branch_ = location.parent_path();
			}
		}
		return true;
	}
    
	void load_end(const char* name)
	{
		if (name)
		{
			TXML_LOG(boost::wformat(L" << load_end: %1%\n") % from_utf8(name));

			previous_child_node_ = current_node_;
		}

		current_node_ = node_stack_.top();
		node_stack_.pop();
	}

	std::string load_value()
	{
		if (current_node_ && current_node_->first_child())
		{
			std::string value = current_node_->first_child()->value_str();
				
			TXML_LOG(boost::wformat(L" >> value >> %1%\n") % from_utf8(value));

			return value;
		}
		else 
			return std::string();
			
	}

private:	
	bool boost_xml_compat_;

	xml::document xml_;

	xml::node* current_node_;
	xml::node* previous_child_node_;
	boost::filesystem::path previous_child_branch_;

	std::stack<xml::node*> node_stack_;
};

} }

BOOST_SERIALIZATION_REGISTER_ARCHIVE(hal::xml::txml_iarchive)
