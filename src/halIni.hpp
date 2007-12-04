
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

#include "global/ini_adapter.hpp"
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
	
	void Save()
	{
		std::wstringstream xml_data;
		
		boost::archive::xml_woarchive oxml(xml_data);
		T* pT = static_cast<T*>(this);	
		oxml << boost::serialization::make_nvp(name_.c_str(), *pT);
		
		adapter_.save_stream_data(xml_data);
	}
	
	void save() { Save(); }
	
	void Load()
	{
		std::wstringstream xml_data;		
		adapter_.load_stream_data(xml_data);
		
		try 
		{
		
		boost::archive::xml_wiarchive ixml(xml_data);	
		
		T* pT = static_cast<T*>(this);	
		ixml >> boost::serialization::make_nvp(name_.c_str(), *pT);
		
		}
		catch (const std::exception& e)
		{			
			hal::event().post(boost::shared_ptr<hal::EventDetail>(
				new hal::EventXmlException(hal::from_utf8(e.what()), hal::from_utf8(name_)))); 
		}
	}
	
	void load() { Load(); }
	
private:
	hal::ini_adapter adapter_;
	std::string name_;	
};

}

