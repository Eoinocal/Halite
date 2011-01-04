
#pragma once

#include <stack>
#include <fstream>

#include <boost/mpl/assert.hpp>

#include <boost/archive/impl/archive_serializer_map.ipp>
#include <boost/archive/detail/auto_link_archive.hpp>
#include <boost/archive/basic_xml_archive.hpp>
#include <boost/archive/detail/common_oarchive.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/string.hpp>

#include <boost/archive/basic_text_oprimitive.hpp>
#include <boost/archive/detail/register_archive.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

#ifndef BOOST_NO_CWCHAR
#	include <boost/archive/wcslen.hpp>
#	include <boost/archive/iterators/mb_from_wchar.hpp>
#endif

#include "../halEvent.hpp"

#ifndef TXML_ARCHIVE_LOGGING
#	define TXML_LOG(s)
#else
#	define TXML_LOG(msg) \
	hal::event_log().post(boost::shared_ptr<hal::EventDetail>( \
			new hal::EventMsg(msg, hal::event_logger::xml_dev))) 
#endif

namespace hal
{

namespace detail
{

struct o_stringstream_holder
{
	std::stringstream stream_ ;
};

}

namespace serial = boost::serialization;
namespace arc = boost::archive;

template<typename Base>
class crtp_oarchive : 
	private detail::o_stringstream_holder,
	public arc::basic_text_oprimitive<std::ostream>,
    public arc::detail::common_oarchive<Base>
{
	typedef arc::detail::common_oarchive<Base> detail_common_oarchive;

public:
    friend class arc::detail::interface_oarchive<Base>;
	friend class arc::save_access;

	BOOST_ARCHIVE_DECL(BOOST_PP_EMPTY()) 
	crtp_oarchive(std::ostream& os, unsigned int flags = 0) :
		basic_text_oprimitive<std::ostream>(stream_, 0 != (flags & arc::no_codecvt)),
		arc::detail::common_oarchive<Base>(flags),
		os_(os)
	{
		init();
	}

	~crtp_oarchive()
	{}
	
	void save_binary(const void *address, std::size_t count)
	{
		this->end_preamble();
		this->basic_text_oprimitive<std::ostream>::save_binary(address, count);
	}

	template<typename T>
	void write_attribute(const char* attribute_name, T key)
	{
		assert(attribute_name);
				
		Base* ptr = static_cast<Base*>(this);
		ptr->write_attribute(attribute_name, key);
	}

	void save_start(const char *name)
	{
		if (name)
		{
			Base* ptr = static_cast<Base*>(this);
			ptr->save_start(name);

			stream_.str(std::string());
		}
	}
    
	void save_end(const char *name)
	{
		if (name)
		{
			Base* ptr = static_cast<Base*>(this);
			ptr->save_end(name, stream_.str());

			stream_.str(std::string());
			stream_ << std::noboolalpha;
		}		
	}
	
#	ifndef BOOST_NO_STD_WSTRING	
	void save(const std::wstring &ws)
	{
		std::string ustr;

		unicode::transcode<unicode::wchar_encoding, unicode::utf8>(
			ws.begin(),
			ws.end(),
			std::insert_iterator<std::string>(ustr, ustr.end()));

		save(ustr);
	}
#	endif

	template<class T>
	void save(const T & t)
	{
		basic_text_oprimitive<std::ostream>::save(t);
	}

	// Anything not an attribute and not a name-value pair is an
	// error and should be trapped here.
	template<class T>
	void save_override(T & t, BOOST_PFTO int)
	{
		// If your program fails to compile here, its most likely due to
		// not specifying an nvp wrapper around the variable to
		// be serialized.
		BOOST_MPL_ASSERT((serialization::is_wrapper<T>));
		this->detail_common_oarchive::save_override(t, 0);
	}

	// special treatment for name-value pairs.
	template<class T>
	void save_override(
		#ifndef BOOST_NO_FUNCTION_TEMPLATE_ORDERING
			const
		#endif
			::boost::serialization::nvp<T> & t,
			int
		)
	{
		save_start(t.name());
		this->detail_common_oarchive::save_override(t.const_value(), 0);
		save_end(t.name());
	}

	void init()
	{
		// xml header
//		write_attribute("signature", arc::BOOST_ARCHIVE_SIGNATURE());
//		write_attribute("version", arc::BOOST_ARCHIVE_VERSION());
	}

	// specific overrides for attributes - not name value pairs so we
	// want to trap them before the above "fall through"

	void save_override(const arc::object_id_type & t, int i)
	{ 
		write_attribute(arc::BOOST_ARCHIVE_XML_OBJECT_ID(), t);
	}

	void save_override(const arc::object_reference_type & t, int)
	{
		write_attribute(arc::BOOST_ARCHIVE_XML_OBJECT_ID(), t);
	}
    
	void save_override(const arc::version_type & t, int i)
	{ 
		write_attribute(arc::BOOST_ARCHIVE_XML_VERSION(), t);
	}
    
	void save_override(const arc::class_id_type & t, int i)
	{ 
		write_attribute(arc::BOOST_ARCHIVE_XML_CLASS_ID(), t);
	}
    
	void save_override(const arc::class_id_optional_type & t, int i)
	{ 
		write_attribute(arc::BOOST_ARCHIVE_XML_CLASS_ID(), t); 
	}
    
	void save_override(const arc::class_id_reference_type & t, int i)
	{ 
		write_attribute(arc::BOOST_ARCHIVE_XML_CLASS_ID(), t); 
	}
    
	void save_override(const arc::class_name_type & t, int i)
	{ 
		const char * key = t;
		if (NULL == key)
			return;

		write_attribute(arc::BOOST_ARCHIVE_XML_CLASS_NAME(), key);
	}
    
	void save_override(const arc::tracking_type & t, int i)
	{ 
		write_attribute(arc::BOOST_ARCHIVE_XML_TRACKING(), t.t); 
	}
	
	std::ostream& os_;
};

}
