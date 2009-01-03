
//         Copyright Eóin O'Callaghan 2008 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "wtl_app.hpp"
#include "logger.hpp"
#include "string_conv.hpp"

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "txml_ini.hpp"
#include "txml.hpp"

#define foreach BOOST_FOREACH

namespace hal 
{

class ini_impl
{
public:
	ini_impl(boost::filesystem::wpath filename)
	{
		wlog() << boost::wformat(L"Ini initialized; %1%") % filename;

		main_file_ = filename;
		working_file_ = filename.string() + L".working";

		if (boost::filesystem::exists(working_file_))
		{			
			std::wstringstream sstr;
			boost::posix_time::wtime_facet* facet = new boost::posix_time::wtime_facet(L"%Y-%m-%d.%H-%M-%S");
			sstr.imbue(std::locale(std::cout.getloc(), facet));
			sstr << boost::posix_time::second_clock::universal_time();

			boost::filesystem::rename(working_file_, (filename.string() + L"." + sstr.str()));			
		}

		if (boost::filesystem::exists(main_file_))
			boost::filesystem::copy_file(main_file_, working_file_);
	}

	~ini_impl()
	{
		if (boost::filesystem::exists(working_file_))
		{
			if (boost::filesystem::last_write_time(main_file_) ==
					boost::filesystem::last_write_time(working_file_))
			{
				boost::filesystem::remove(working_file_);
			}
		}
	}
	
	bool load_data()
	{
		wlog() << L"Ini load data file";

		if (!xml_.load_file(working_file_.string()))
		{
			generate_default_file();

			return false;
		}

		return true;
	}
	
	bool save_data()
	{		
		wlog() << L"Ini save data file";

		bool result = xml_.save_file(working_file_.string());

		if (boost::filesystem::exists(working_file_))
		{
			boost::filesystem::remove(main_file_);
			boost::filesystem::copy_file(working_file_, main_file_);
		}

		return result;
	}

	bool save(boost::filesystem::path location, xml::node* data)
	{
		wlog() << L"Ini save ...";

		xml::node* data_node = get_save_data_node(location);

		if (data_node)
		{
			wlog() << boost::wformat(L"Ini got save data node; %1%") % to_wstr_shim(location.string());

			data_node->clear();
			data_node->link_end_child(data);
			
			return true;
		}
		else
		{
			wlog() << boost::wformat(L"Not got save data node; %1%") % to_wstr_shim(location.string());

			return false;
		}
	}
	
	xml::node* load(boost::filesystem::path location)
	{
		xml::node* data_node = get_load_data_node(location);

		if (!data_node) return data_node;
		
		xml::node* data = data_node->first_child();
		
		if (data)
			return data->clone();
		else
			return 0;
	}

private:
	void generate_default_file()
	{
		wlog() << L"Ini generate default data";

		xml_.link_end_child(new xml::declaration("1.0", "", ""));
		
		xml_.link_end_child(new xml::element("ini"));

		wlog() << boost::wformat(L"Default file generated");
	}
	
	xml::node* get_save_data_node(boost::filesystem::path location)
	{
		wlog() << L"Get save data node";

		xml::node* data_node = xml_.first_child("ini");
		
		if (!data_node)
		{
			data_node = new xml::element("ini");
			xml_.link_end_child(data_node);
		}
		
		foreach(std::string elem, location)
		{
			xml::node* child_node = data_node->first_child(elem);
			
			if (!child_node)
			{
				child_node = new xml::element(elem);
				data_node->link_end_child(child_node);
			}
			
			data_node = child_node;
		}

		return data_node;
	}

	xml::node* get_load_data_node(boost::filesystem::path location)
	{
		xml::node* data_node = xml_.first_child("ini");
		
		if (!data_node) return data_node;
		
		foreach(std::string elem, location)
		{
			data_node = data_node->first_child(elem);
			
			if (!data_node) return data_node;
		}

		return data_node;
	}
	
	boost::filesystem::wpath main_file_;
	boost::filesystem::wpath working_file_;

	xml::document xml_;
};

txml_ini::txml_ini()
{}

txml_ini::txml_ini(boost::filesystem::wpath filename)
{
	init(filename);
}

void txml_ini::init(boost::filesystem::wpath filename)
{
	assert(!pimpl);

	if (!pimpl) pimpl.reset(new ini_impl(filename));
}

txml_ini::~txml_ini()
{}

bool txml_ini::load_data()
{
	assert(pimpl);

	return pimpl->load_data();
}

bool txml_ini::save_data()
{
	assert(pimpl);

	return pimpl->save_data();
}

/*bool txml_ini::save(boost::filesystem::path location, std::string data)
{
	assert(pimpl);

	return pimpl->save(location, data);
}
*/
bool txml_ini::save(boost::filesystem::path location, xml::node* data)
{
	assert(pimpl);

	return pimpl->save(location, data);
}

xml::node* txml_ini::load(boost::filesystem::path location)
{
	assert(pimpl);

	return pimpl->load(location);
}

}
