
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#if defined(HALTORRENT_PCH)
#	include "halPch.hpp"
#else
#	include "halTypes.hpp"
#endif

#include "global/wtl_app.hpp"
#include "halEvent.hpp"

namespace hal 
{

template <class T>
class IniBase
{
public:
	IniBase(boost::filesystem::wpath location, std::wstring name) :
		name_(name),
		directory_(app().get_working_directory() / L"config" / location)
	{
		initialise();
	}

	void initialise()
	{
//		HAL_DEV_MSG(hal::wform(L"Ini directory is %1%") % directory_);
		
		if (!fs::exists(directory_))
			fs::create_directories(directory_);
	}
	
	void save_to_ini()
	{
		fs::wofstream ofs(directory_ / (name_ + L".xml"));

		ofs.imbue(std::locale(ofs.getloc(),
			new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>));

		boost::archive::xml_woarchive woa(ofs);

		T* pT = static_cast<T*>(this);	
		woa << boost::serialization::make_nvp(to_utf8(name_).c_str(), *pT);
		
	}
		
	bool load_from_ini(bool report_exception = true)
	{
		try 
		{

		fs::wifstream ifs(directory_ / (name_ + L".xml"));

		ifs.imbue(std::locale(ifs.getloc(),
			new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>));

		boost::archive::xml_wiarchive wia(ifs);

		T* pT = static_cast<T*>(this);	
		wia >> boost::serialization::make_nvp(to_utf8(name_).c_str(), *pT);
		
		}
		catch (const std::exception& e)
		{			
			if (report_exception)
				hal::event_log().post(boost::shared_ptr<hal::EventDetail>(
					new hal::EventXmlException(hal::from_utf8(e.what()), name_))); 

			return false;
		}

		return true;
	}
	
private:
	std::wstring name_;	
	fs::wpath directory_;
};

}

