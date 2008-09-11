
#pragma once

#include <stack>

#include <boost/mpl/assert.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>

//#include <boost/archive/xml_oarchive.hpp>

#pragma warning (push)
#pragma warning (disable : 4099)
#	include <boost/serialization/version.hpp>
#	include <boost/serialization/vector.hpp>
#	include <boost/serialization/map.hpp>
#	include <boost/serialization/split_free.hpp>

#	include <boost/archive/impl/basic_text_oprimitive.ipp>
#	include <boost/archive/impl/xml_oarchive_impl.ipp>
#	include <boost/archive/impl/basic_xml_oarchive.ipp>
#	include <boost/archive/impl/archive_pointer_oserializer.ipp>
#pragma warning (pop)

#include "global/string_conv.hpp"
#include "txml.hpp"

#define foreach BOOST_FOREACH

#ifndef TXML_ARCHIVE_LOGGING
#	define TXML_LOG(s)
#else
#	include "../halEvent.hpp"
#	define TXML_LOG(msg) \
	hal::event_log.post(boost::shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(msg, hal::event_logger::xml_dev))) 
#endif

namespace 
{

struct o_stringstream_holder
{
	std::stringstream stream_ ;
};

}

namespace hal { namespace xml
{

namespace serial = boost::serialization;
namespace arc = boost::archive;

class txml_oarchive : 
	private o_stringstream_holder,
	public arc::basic_text_oprimitive<std::ostream>,
	public arc::detail::common_oarchive<txml_oarchive>
{
	typedef arc::detail::common_oarchive<txml_oarchive> detail_common_oarchive;

public:

    txml_oarchive(std::ostream& os, unsigned int flags = 0) :
		basic_text_oprimitive<std::ostream>(stream_, 0 != (flags & arc::no_codecvt)),
		common_oarchive(flags),
		os_(os),
		current_node_(0)
    {
		xml_.link_end_child(new xml::declaration("1.0", "", "")); 
		
		if (!current_node_)
		{
			current_node_ = new xml::element("serial");
			xml_.link_end_child(current_node_);
		}

		if(0 == (flags & arc::no_header))
			init();
	}

    ~txml_oarchive()
	{
#ifdef TXML_ARCHIVE_LOGGING
		xml_.save_file("debug.xml");
#endif
		os_ << xml_;
	}

    template<class T>
    void save(const T & t)
	{
		basic_text_oprimitive<std::ostream>::save(t);
	}
	    
	void save(const char *  s)
	{
		typedef boost::archive::iterators::xml_escape<
			const char * 
		> xml_escape_translator;
		std::copy(
			xml_escape_translator(BOOST_MAKE_PFTO_WRAPPER(s)),
			xml_escape_translator(BOOST_MAKE_PFTO_WRAPPER(s + std::strlen(s))), 
			boost::archive::iterators::ostream_iterator<char>(os)
		);
	}

    #ifndef BOOST_NO_INTRINSIC_WCHAR_T
    void save(const wchar_t* ws)
	{
		arc::save_iterator(os, ws, ws + std::wcslen(ws));
	}
    #endif

    void save(const std::string &s)
	{
	//  at least one library doesn't typedef value_type for strings
	//  so rather than using string directly make a pointer iterator out of it
		typedef boost::archive::iterators::xml_escape<
			const char * 
		> xml_escape_translator;
		std::copy(
			xml_escape_translator(BOOST_MAKE_PFTO_WRAPPER(s.data())),
			xml_escape_translator(BOOST_MAKE_PFTO_WRAPPER(s.data()+ s.size())), 
			boost::archive::iterators::ostream_iterator<char>(os)
		);
	}

    #ifndef BOOST_NO_STD_WSTRING
    void save(const std::wstring &ws)
	{
	//  at least one library doesn't typedef value_type for strings
	//  so rather than using string directly make a pointer iterator out of it
	//    save_iterator(os, ws.data(), ws.data() + std::wcslen(ws.data()));
	//		arc::save_iterator(os, ws.data(), ws.data() + ws.size());
			os << to_utf8(ws);
	}
    #endif

    BOOST_ARCHIVE_DECL(BOOST_PP_EMPTY()) 

    void save_binary(const void *address, std::size_t count)
	{
        this->end_preamble();
        this->basic_text_oprimitive<std::ostream>::save_binary(
            address, 
            count
        );
        //this->indent_next = true;
    }

    void write_attribute(
        const char *attribute_name,
        int t,
        const char *conjunction = 0)
	{ 
		assert(attribute_name);

		xml::element* e = dynamic_cast<xml::element*>(current_node_);

		if (true || !conjunction)
		{
			e->set_attribute(attribute_name, t);

			TXML_LOG(boost::wformat(L" >> write_attribute: %1%, t:%2%") % attribute_name % t);
		}
		else
		{
			std::string attr = conjunction;
			attr += boost::lexical_cast<std::string>(t);
			e->set_attribute(attribute_name, attr);
			
			TXML_LOG(boost::wformat(L" >> write_attribute: %1%, t:%2%") % attribute_name % from_utf8(attr));
		}
	}	

    void write_attribute(const char *attribute_name, const char *key)
	{
		assert(attribute_name);
		assert(key);

		xml::element* e = dynamic_cast<xml::element*>(current_node_);
		e->set_attribute(attribute_name, key);

		TXML_LOG(boost::wformat(L" >> write_attribute: %1%, key:%2%") % attribute_name % key);
	}

    void save_start(const char *name)
	{
		if (name)
		{
			TXML_LOG(boost::wformat(L" >> save_start: %1%") % name);
			node_stack_.push(current_node_);

			boost::filesystem::path branch(name);

			std::string leaf = branch.filename();
			branch = branch.parent_path();

			foreach(std::string elem, branch)
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

			stream_.str("");
		}
	}
    
    void save_end(const char *name)
	{
		if (name)
		{
			assert(!node_stack_.empty());

			std::string s = stream_.str();

			TXML_LOG(boost::wformat(L" >> stringstream: %1%") % from_utf8(s));
			xml::text* t = new xml::text(s);

			current_node_->link_end_child(t);

			stream_.str("");
			stream_ << std::noboolalpha;

			TXML_LOG(boost::wformat(L" >> save_end: %1%") % name);

			current_node_ = node_stack_.top();
			node_stack_.pop();
		}		
	}

	// Anything not an attribute and not a name-value pair is an
    // error and should be trapped here.
    template<class T>
    void save_override(T & t, BOOST_PFTO int)
    {
        BOOST_MPL_ASSERT((::boost::serialization::is_wrapper<T>));

        this->detail_common_oarchive::save_override(t, 0);
        // If your program fails to compile here, its most likely due to
        // not specifying an nvp wrapper around the variable to
        // be serialized.
    }

   // special treatment for name-value pairs.

    template<class T>
    void save_override( const ::boost::serialization::nvp<T> & t, int)
    {
		save_start(t.name());
		this->detail_common_oarchive::save_override(t.const_value(), 0);
        save_end(t.name());
    }

	void init()
	{
		// xml header
		write_attribute("signature", arc::ARCHIVE_SIGNATURE());
		write_attribute("version", arc::ARCHIVE_VERSION());
	}

	// specific overrides for attributes - not name value pairs so we
    // want to trap them before the above "fall through"

	void save_override(const arc::object_id_type & t, int i)
	{ 
		write_attribute(arc::OBJECT_ID(), t);
	}

    void save_override(const arc::object_reference_type & t, int)
	{
		write_attribute(arc::OBJECT_REFERENCE(), t);
	}
    
	void save_override(const arc::version_type & t, int i)
	{ 
		write_attribute(arc::VERSION(), t);
	}
    
	void save_override(const arc::class_id_type & t, int i)
	{ 
		write_attribute(arc::CLASS_ID(), t);
	}
    
	void save_override(const arc::class_id_optional_type & t, int i)
	{ 
		write_attribute(arc::CLASS_ID(), t); 
	}
    
	void save_override(const arc::class_id_reference_type & t, int i)
	{ 
		write_attribute(arc::CLASS_ID_REFERENCE(), t); 
	}
    
	void save_override(const arc::class_name_type & t, int i)
	{ 
		const char * key = t;
		if(NULL == key)
			return;

		write_attribute(arc::CLASS_NAME(), key);
	}
    
	void save_override(const arc::tracking_type & t, int i)
	{ 
		write_attribute(arc::TRACKING(), t.t); 
	}

	xml::document xml_;
	xml::node* current_node_;
	std::stack<xml::node*> node_stack_;

	std::ostream& os_;
};

} }

//BOOST_SERIALIZATION_REGISTER_ARCHIVE(xml::txml_oarchive)
