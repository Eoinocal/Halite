
//         Copyright Eóin O'Callaghan 2006 - 2008.
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
	
	IniBase(std::string name, hal::ini_file& ini = hal::ini()) :
		adapter_(boost::filesystem::path(""), ini),
		name_(name)
	{}
	
	void save_to_ini()
	{
		std::wstringstream xml_data;
		
		boost::archive::xml_woarchive oxml(xml_data);
		T* pT = static_cast<T*>(this);	
		oxml << boost::serialization::make_nvp(name_.c_str(), *pT);
		
		adapter_.save_stream_data(xml_data);
	}

	template<typename P>
	void save_standalone(const P& location)
	{
		fs::wofstream ofs(location);
		
		boost::archive::xml_woarchive oxml(ofs);
		T* pT = static_cast<T*>(this);	
		oxml << boost::serialization::make_nvp(name_.c_str(), *pT);
	}
	
	template<typename P>
	bool load_standalone(const P& location)
	{
		bool result = false;
		
		try 
		{		
		fs::wifstream ifs(location);

		boost::archive::xml_wiarchive ixml(ifs);

		T* pT = static_cast<T*>(this);	
		ixml >> boost::serialization::make_nvp(name_.c_str(), *pT);

		result = true;	

		}
		HAL_GENERIC_FN_EXCEPTION_CATCH(L"IniBase::load_standalone()")

		return result;
	}
	
	bool load_from_ini()
	{
		bool result = false;
		
		try 
		{

		std::wstringstream xml_data;		
		adapter_.load_stream_data(xml_data);
		
		boost::archive::xml_wiarchive ixml(xml_data);	
		
		T* pT = static_cast<T*>(this);	
		ixml >> boost::serialization::make_nvp(name_.c_str(), *pT);

		result = true;	

		}
		HAL_GENERIC_FN_EXCEPTION_CATCH(L"IniBase::load_from_ini()")

		return result;
	}
	
private:
	hal::ini_adapter adapter_;
	std::string name_;	
};

}

