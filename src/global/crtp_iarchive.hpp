
#pragma once

#include <stack>
#include <fstream>

#include <boost/mpl/assert.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>

#include <boost/archive/detail/auto_link_archive.hpp>
#include <boost/archive/basic_text_iprimitive.hpp>
#include <boost/archive/basic_xml_archive.hpp>
#include <boost/archive/detail/common_iarchive.hpp>

#include <boost/archive/detail/register_archive.hpp>
#include <boost/archive/shared_ptr_helper.hpp>

#ifndef BOOST_NO_CWCHAR
#	include "global/unicode.hpp"
#endif

#include "../halEvent.hpp"

#ifndef TXML_ARCHIVE_LOGGING
#	define TXML_LOG(s)
#else
#	define TXML_LOG(msg) \
	hal::event_log().post(boost::shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(msg, hal::event_logger::xml_dev))) 
#endif

namespace hal {

namespace detail {

struct i_stringstream_holder
{
	std::stringstream stream_ ;
};

}

namespace serial = boost::serialization;
namespace arc = boost::archive;

template<typename Base>
class crtp_iarchive : 
	private detail::i_stringstream_holder,
	public arc::detail::shared_ptr_helper,
	public arc::basic_text_iprimitive<std::istream>,
	public arc::detail::common_iarchive<Base>
{
	typedef arc::detail::common_iarchive<Base> detail_common_iarchive;

public:
	crtp_iarchive(std::istream& is, unsigned int flags = 0) :
		basic_text_iprimitive<std::istream>(stream_, 0 != (flags & arc::no_codecvt)),
		arc::detail::common_iarchive<Base>(flags)
	{}

	~crtp_iarchive()
	{}

	template<typename T>
	T read_attribute(const char* attribute_name, const char* fallback_name=0)
	{
		Base* ptr = static_cast<Base*>(this);
		
		return ptr->read_attribute<T>(attribute_name, fallback_name);
	}
	    
	void load_end(const char *name)
	{
		Base* ptr = static_cast<Base*>(this);
		ptr->load_end(name, stream_.str());
	}

	// Anything not an attribute and not a name-value pair is an
	// error and should be trapped here.
	template<class T>
	void load_override(T & t, BOOST_PFTO int)
	{
/*		BOOST_MPL_ASSERT((boost::serialization::is_wrapper<T>));

		this->detail_common_iarchive::load_override(t, 0);
		// If your program fails to compile here, its most likely due to
		// not specifying an nvp wrapper around the variable to
		// be serialized.
		*/
	}
	
	void load_override(arc::class_name_type& t, int) 
	{    
	/*	std::string tstring = current_node_->first_child()->value_str();

		TXML_LOG(boost::wformat(L" << load class_name_type: %1%") % from_utf8(tstring));

		char * tptr = t;
		std::memcpy(tptr, tstring.data(), tstring.size());
		tptr[tstring.size()] = '\0';*/

		//t = read_attribute<arc::class_name_type>(arc::BOOST_ARCHIVE_XML_CLASS_NAME());
	}

	// special treatment for name-value pairs.
	template<class T>
	void load_override(const ::boost::serialization::nvp<T>& t, int)
	{	
		Base* ptr = static_cast<Base*>(this);
		if (ptr->load_start(t.name()))
		{		
			this->detail_common_iarchive::load_override(t.value(), 0);
			ptr->load_end(t.name());
		}
	}
	
	template<class T>
	void load(T& t)
	{		
		Base* ptr = static_cast<Base*>(this);
		stream_.str(ptr->load_value());

		basic_text_iprimitive<std::istream>::load(t);

		stream_.clear();
		stream_ << std::noboolalpha;
	}
	
	template<>
	void load(std::string& t)
	{		
		Base* ptr = static_cast<Base*>(this);
		t = ptr->load_value();
	}

#	ifndef BOOST_NO_STD_WSTRING	
	template<>
	void load(std::wstring& t)
	{
		std::string utf8;
		load(utf8);

		t.clear();

		unicode::transcode<unicode::utf8, unicode::wchar_encoding>(
			utf8.begin(),
			utf8.end(),
			std::insert_iterator<std::wstring>(t, t.end()));
	}
#endif

	// specific overrides for attributes - not name value pairs so we
	// want to trap them before the above "fall through"

	void load_override(arc::object_id_type& t, int i)
	{
		t = read_attribute<arc::object_id_type>(arc::BOOST_ARCHIVE_XML_OBJECT_ID());
	}

	void load_override(arc::object_reference_type& t, int)
	{
		t = arc::object_reference_type(
			read_attribute<arc::object_id_type>(arc::BOOST_ARCHIVE_XML_OBJECT_REFERENCE()));
	}
    
	void load_override(arc::version_type& t, int i)
	{ 
		t = read_attribute<arc::version_type>(arc::BOOST_ARCHIVE_XML_VERSION());
	}
    
	void load_override(arc::class_id_type& t, int i)
	{
		t = read_attribute<arc::class_id_type>(arc::BOOST_ARCHIVE_XML_CLASS_ID());
	}
    
	void load_override(arc::class_id_optional_type& t, int i)
	{ 
		t = arc::class_id_optional_type(
			read_attribute<arc::class_id_type>(arc::BOOST_ARCHIVE_XML_CLASS_ID())); 
	}
    
	void load_override(arc::class_id_reference_type& t, int i)
	{ 
		t = arc::class_id_reference_type(
			read_attribute<arc::class_id_type>(arc::BOOST_ARCHIVE_XML_CLASS_ID_REFERENCE())); 
	}
    
	void load_override(arc::tracking_type& t, int i)
	{ 
		t = read_attribute<arc::tracking_type>(arc::BOOST_ARCHIVE_XML_TRACKING()); 
	}
};

}
