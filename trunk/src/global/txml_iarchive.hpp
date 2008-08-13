
#pragma once

#include <stack>

#include <boost/mpl/assert.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <boost/archive/xml_iarchive.hpp>

#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/split_free.hpp>

#include <boost/archive/impl/basic_text_iprimitive.ipp>
#include <boost/archive/impl/xml_iarchive_impl.ipp>
#include <boost/archive/impl/basic_xml_iarchive.ipp>
#include <boost/archive/impl/archive_pointer_iserializer.ipp>
#include <boost/archive/shared_ptr_helper.hpp>

#include "global/string_conv.hpp"
#include "txml.hpp"

#define foreach BOOST_FOREACH

#ifndef TXML_ARCHIVE_LOGGING
#	define TXML_LOG(s)
#else
#	include "logger.hpp"
#	define TXML_LOG(s) wlog() << s
#endif

namespace 
{

struct i_stringstream_holder
{
	std::stringstream stream_;
};

}

namespace hal { namespace xml
{

namespace serial = boost::serialization;
namespace arc = boost::archive;

class txml_iarchive : 
	private i_stringstream_holder,
	public arc::basic_text_iprimitive<std::istream>,
	public arc::detail::common_iarchive<txml_iarchive>,
	public boost::archive::detail::shared_ptr_helper
{
	typedef arc::detail::common_iarchive<txml_iarchive> detail_common_iarchive;

public:
    txml_iarchive(std::istream& is, unsigned int flags = 0) :
		basic_text_iprimitive<std::istream>(stream_, 0 != (flags & arc::no_codecvt)),
		detail_common_iarchive(flags),
		is_(is),		
		previous_child_node_(0)
    {
		is_ >> xml_;

		current_node_ = xml_.root_element();
		init();
	}

    ~txml_iarchive()
	{}
	    
	template<class T>
    void load(T& t)
	{
		std::string tstring = current_node_->first_child()->value_str();

		TXML_LOG(boost::wformat(L" << basic_text_iprimitive: %1%") % from_utf8(tstring));

		stream_ << tstring;
        basic_text_iprimitive<std::istream>::load(t);

		stream_.clear();
		stream_ << std::noboolalpha;
    }

/*	void load(char* s) 
	{
		std::string tstring = current_node_->first_child()->value_str();

		std::memcpy(s, tstring.data(), tstring.size());
		s[tstring.size()] = 0;
		TXML_LOG(boost::wformat(L" << load char*: %1%") % from_utf8(tstring));
	}

	void load(wchar_t* t) 
	{
		TXML_LOG(boost::wformat(L" << load wchar_t*: %1%") % from_utf8(current_node_->first_child()->value_str());
	}
*/
	void load(std::string& s) 
	{		
		if (xml::node* child = current_node_->first_child())
			s = child->value_str();
		else
			s = "";

		TXML_LOG(boost::wformat(L" << load string: %1%") % from_utf8(s));
	}

	void load(std::wstring& ws) 
	{
		if (xml::node* child = current_node_->first_child())
			ws = from_utf8(child->value_str());
		else
			ws = L"";

		TXML_LOG(boost::wformat(L" << load wstring: %1%") % ws);
	}

	void load_override(arc::class_name_type& t, int) 
	{    
		std::string tstring = current_node_->first_child()->value_str();

		TXML_LOG(boost::wformat(L" << load class_name_type: %1%") % from_utf8(tstring));

		char * tptr = t;
		std::memcpy(tptr, tstring.data(), tstring.size());
		tptr[tstring.size()] = '\0';
	}

	void init() 
	{
		std::string signature = read_attribute<std::string>("signature");
		int version = read_attribute<int>("version");

		TXML_LOG(boost::wformat(L" << siganture: %1%, version: %2%") % from_utf8(signature) % version);
	}

	template<typename T>
	T read_attribute(const char* attribute_name, const char* fallback_name=0)
	{
		T type;

		TXML_LOG(boost::wformat(L" << attribute_name: %1%") % from_utf8(attribute_name));

		xml::element* e = current_node_->to_element();
		int result = e->query_value_attribute(attribute_name, &type);

		if (result == xml::TIXML_NO_ATTRIBUTE && fallback_name != 0)
		{
			TXML_LOG(boost::wformat(L" << -- fallback_name: %1%") % from_utf8(fallback_name));

			result = e->query_value_attribute(fallback_name, &type);
		}

		assert(result == xml::TIXML_SUCCESS);
		TXML_LOG(boost::wformat(L" << -- value: %2%") % from_utf8(attribute_name) % type);
		
		return type;
	}	

	template<>
	std::string read_attribute(const char* attribute_name, const char* fallback_name)
	{
		std::string type;

		TXML_LOG(boost::wformat(L" << attribute_name: %1%") % from_utf8(attribute_name));

		xml::element* e = current_node_->to_element();
		int result = e->query_value_attribute(attribute_name, &type);

		if (result == xml::TIXML_NO_ATTRIBUTE && fallback_name != 0)
		{
			TXML_LOG(boost::wformat(L" << -- fallback_name: %1%") % from_utf8(fallback_name));

			result = e->query_value_attribute(fallback_name, &type);
		}

		assert(result == xml::TIXML_SUCCESS);

		TXML_LOG(boost::wformat(L" << -- value: %2%") % from_utf8(attribute_name) % from_utf8(type));

		return type;
	}	

    bool load_start(const char* name)
	{
		if (name)
		{
			xml::node* failsafe_current = 0;

			TXML_LOG(boost::wformat(L" << load_start: %1%") % name);

			node_stack_.push(current_node_);
			failsafe_current = current_node_;

			boost::filesystem::path location(name);

			if (previous_child_node_ &&
					previous_child_branch_ == location.branch_path())
			{
			//	TXML_LOG(boost::wformat(L" << previous_child: %1%") % previous_child_node_->to_element()->get_text());
				failsafe_current = previous_child_node_->next_sibling(location.leaf());
				previous_child_node_ = 0;
			}
			else
			{
				foreach(std::string elem, location)
				{
					TXML_LOG(boost::wformat(L" >> >> %1%") % from_utf8(elem));

					if (elem == ".")
					{}
					else if (elem == "..")
					{
						failsafe_current = failsafe_current->parent();
					}
					else
					{
						failsafe_current = failsafe_current->first_child(elem);
						
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

				previous_child_branch_ = location.branch_path();
			}
		}
		return true;
	}
    
    void load_end(const char *name)
	{
		if (name)
		{
			TXML_LOG(boost::wformat(L" << load_end: %1%") % name);

			previous_child_node_ = current_node_;

			current_node_ = node_stack_.top();
			node_stack_.pop();
		}
	}

	// Anything not an attribute and not a name-value pair is an
    // error and should be trapped here.
    template<class T>
    void load_override(T & t, BOOST_PFTO int)
    {
		BOOST_MPL_ASSERT((boost::serialization::is_wrapper<T>));

        this->detail_common_iarchive::load_override(t, 0);
        // If your program fails to compile here, its most likely due to
        // not specifying an nvp wrapper around the variable to
        // be serialized.
    }

	// special treatment for name-value pairs.
    template<class T>
    void load_override(const ::boost::serialization::nvp<T>& t, int)
	{
		if (load_start(t.name()))
		{
			this->detail_common_iarchive::load_override(t.value(), 0);
			load_end(t.name());
		}
		else
			TXML_LOG(boost::wformat(L" << load_aborted: %1%") % t.name());
    }

	// specific overrides for attributes - not name value pairs so we
    // want to trap them before the above "fall through"
	void load_override(arc::class_id_optional_type&, int)
	{}

	void load_override(arc::object_id_type& t, int)
	{ 
		t = read_attribute<arc::object_id_type>(arc::OBJECT_ID(), arc::OBJECT_REFERENCE());		
	}
    
	void load_override(arc::version_type& t, int)
	{ 
		t = read_attribute<arc::version_type>(arc::VERSION());
	}
    
	void load_override(arc::class_id_type& t, int)
	{ 
		t = read_attribute<arc::class_id_type>(arc::CLASS_ID(), arc::CLASS_ID_REFERENCE());
	}
    
	void load_override(arc::tracking_type& t, int)
	{ 
		t = read_attribute<arc::tracking_type>(arc::TRACKING()); 
	}

	xml::document xml_;

	xml::node* current_node_;
	xml::node* previous_child_node_;
	boost::filesystem::path previous_child_branch_;

	std::stack<xml::node*> node_stack_;

	std::istream& is_;
};

} }

//BOOST_SERIALIZATION_REGISTER_ARCHIVE(xml::txml_iarchive)
