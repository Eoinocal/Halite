
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

// #define TXML_ARCHIVE_LOGGING
#include "global/txml_ini_adapter.hpp"
#include "global/txml_oarchive.hpp"
#include "global/txml_iarchive.hpp"

#include "halEvent.hpp"

namespace hal 
{

template <class T>
class IniBase
{
public:
	IniBase(boost::filesystem::path location, std::string name, hal::ini_file& ini = hal::ini()) :
		adapter_(location, ini),
		name_(name)
	{}
	
	IniBase(std::string name, hal::ini_file& ini = hal::ini()) :
		adapter_(boost::filesystem::path(""), ini),
		name_(name)
	{}
	
	~IniBase()
	{
		TXML_LOG(L"~IniBase()");
	}
	
	void save_to_ini()
	{
		std::stringstream xml_data;	
		{
		xml::txml_oarchive oxml(xml_data);	
		T* pT = static_cast<T*>(this);	
		oxml << boost::serialization::make_nvp(name_.c_str(), *pT);
		}

		adapter_.save_stream_data(xml_data);
	}

	template<typename P>
	void save_standalone(const P& location)
	{
		fs::ofstream ofs(location);
		
		xml::txml_oarchive oxml(ofs);
		T* pT = static_cast<T*>(this);	
		oxml << boost::serialization::make_nvp(name_.c_str(), *pT);
	}
	
	template<typename P>
	bool load_standalone(const P& location)
	{
		try 
		{		
		fs::ifstream ifs(location);

		xml::txml_iarchive ixml(ifs);

		T* pT = static_cast<T*>(this);	
		ixml >> boost::serialization::make_nvp(name_.c_str(), *pT);

		return true;
		
		}
		catch (const std::exception& e)
		{			
			hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventXmlException(hal::from_utf8(e.what()), L"load_standalone"))); 

			return false;
		}
	}
	
	bool load_from_ini()
	{
		std::stringstream xml_data;		
		adapter_.load_stream_data(xml_data);
		
		try 
		{

		xml::txml_iarchive ixml(xml_data);	
		
		T* pT = static_cast<T*>(this);	
		ixml >> boost::serialization::make_nvp(name_.c_str(), *pT);
		
		}
		catch (const std::exception& e)
		{			
			hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventXmlException(hal::from_utf8(e.what()), hal::from_utf8(name_)))); 

			return false;
		}

		return true;
	}
	
private:
	hal::txml_ini_adapter adapter_;
	std::string name_;	
};

}

