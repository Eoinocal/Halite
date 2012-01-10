
#pragma once

#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>

#include "global/crtp_oarchive.hpp"
#include "global/string_conv.hpp"
#include "global/txml.hpp"

namespace hal { namespace xml
{
		
class txml_oarchive : 
	public crtp_oarchive<txml_oarchive>
{
public:
	txml_oarchive(std::ostream& os, unsigned int flags = 0) :
		crtp_oarchive<txml_oarchive>(os, flags),
		current_node_(0)
	{
		xml_.link_end_child(new xml::declaration("1.0", "", "")); 

		if (!current_node_)
		{
			current_node_ = new xml::element("serial");
			xml_.link_end_child(current_node_);
		}
		
		if(0 == (flags & arc::no_header))
		{			
			write_attribute("signature", arc::BOOST_ARCHIVE_SIGNATURE());
			write_attribute("version", arc::BOOST_ARCHIVE_VERSION());
		}
	}

	~txml_oarchive()
	{
		os_ << xml_;
	}
	
	template<typename T>
	void write_attribute(const char* attribute_name, T key)
	{
		std::stringstream s;
		s << key;
		
		write_attribute(attribute_name, s.str().c_str());
	}

	template<>
	void write_attribute(const char* attribute_name, const char* key)
	{
		xml::element* e = dynamic_cast<xml::element*>(current_node_);
		e->set_attribute(attribute_name, key);

		TXML_LOG(boost::wformat(L" >> write_attribute: %1%, key:%2%\n") 
			% from_utf8(attribute_name) % from_utf8(key));
	}

	void save_start(const std::string& name)
	{
		TXML_LOG(boost::wformat(L" >> save_start: %1%") % from_utf8(name));
		node_stack_.push(current_node_);

		boost::filesystem::path branch(name);

		std::string leaf = branch.filename();
		branch = branch.parent_path();

		BOOST_FOREACH(std::string elem, branch)
		{
			TXML_LOG(boost::wformat(L" >> >> %1%") % from_utf8(elem));

			if (elem == ".")
			{}
			else if (elem == "..")
			{
				current_node_ = current_node_->parent();
			}
			else
			{
				xml::node* child_node = current_node_->first_child(elem);
					
				if (!child_node)
				{
					child_node = new xml::element(elem);
					current_node_->link_end_child(child_node);
				}
					
				current_node_ = child_node;
			}
		}

		xml::node* n = new xml::element(leaf);
		current_node_ = current_node_->link_end_child(n);

		TXML_LOG(boost::wformat(L" >> save_start: %1%\n") % from_utf8(name));
	}
    
	void save_end(const std::string& name, const std::string& s)
	{			
		assert(!node_stack_.empty());

		if (!s.empty())
		{
			TXML_LOG(boost::wformat(L" >> stringstream: %1%") % from_utf8(s));
			xml::text* t = new xml::text(s);

			current_node_->link_end_child(t);
		}
			
		current_node_ = node_stack_.top();
		node_stack_.pop();	

		TXML_LOG(boost::wformat(L" << << data: %1%\n << save_end: %2%\n") 
			% from_utf8(s) % from_utf8(name));
	}

private:	
	xml::document xml_;
	xml::node* current_node_;
	std::stack<xml::node*> node_stack_;
};

} }

BOOST_SERIALIZATION_REGISTER_ARCHIVE(hal::xml::txml_oarchive)
