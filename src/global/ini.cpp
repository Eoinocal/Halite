
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include "txml.hpp"
#include "wtl_app.hpp"
#include "logger.hpp"
#include "string_conv.hpp"
#include "ini.hpp"

namespace hal 
{

class ini_impl
{
public:
	ini_impl(std::wstring filename) :
		main_file_(app().get_working_directory()/filename),
		working_file_(app().get_working_directory()/(filename + L".working")),
		filename_(filename)
	{		
		if (boost::filesystem::exists(working_file_))
			boost::filesystem::rename(working_file_, generate_backup_name());

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
	
	void load_data()
	{
		if (!boost::filesystem::exists(working_file_) || !xml_.load_file(working_file_.string()))
		{
			generate_default_file();
		}
	}
	
	void save_data()
	{		
		xml_.save_file(working_file_.string());

		if (boost::filesystem::exists(working_file_))
		{
			boost::filesystem::remove(main_file_);
			boost::filesystem::copy_file(working_file_, main_file_);
		}
	}
	
	bool save(boost::filesystem::path location, std::string data)
	{
		xml::node* data_node = get_data_node(location);
		
		// Should have correct node		
		
		return true;
	}

	bool save(boost::filesystem::path location, xml::node* data)
	{
		xml::node* data_node = get_data_node(location);
		
		data_node->clear();
		data_node->link_end_child(data);
		
		return true;
	}
	
	xml::node* load(boost::filesystem::path location)
	{
		xml::node* data_node = get_data_node(location);
		
		xml::node* data = data_node->first_child();
		
		if (data)
			return data->clone();
		else
			return 0;
	}

	boost::filesystem::wpath main_file() const { return main_file_; }
	boost::filesystem::wpath working_file() const { return working_file_; }
	
	void clear() 
	{				
		if (boost::filesystem::exists(working_file_))
			boost::filesystem::copy_file(working_file_, generate_backup_name());	

		xml_.clear();
		generate_default_file();
	}

private:
	void generate_default_file()
	{
		xml_.link_end_child(new xml::declaration("1.0", "", ""));
		
		xml_.link_end_child(new xml::element("ini"));
	}

	boost::filesystem::wpath generate_backup_name()
	{
		std::wstringstream sstr;
		boost::posix_time::wtime_facet* facet = new boost::posix_time::wtime_facet(L"%Y-%m-%d.%H-%M-%S");
		sstr.imbue(std::locale(std::cout.getloc(), facet));
		sstr << boost::posix_time::second_clock::universal_time();

		return app().get_working_directory()/(filename_ + L"." + sstr.str());
	}
	
	xml::node* get_data_node(boost::filesystem::path location)
	{
		xml::node* data_node = xml_.first_child("ini");
		
		if (!data_node)
		{
			data_node = new xml::element("ini");
			xml_.link_end_child(data_node);
		}
		
		BOOST_FOREACH (std::string elem, location)
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
	
	boost::filesystem::wpath main_file_;
	boost::filesystem::wpath working_file_;
	std::wstring filename_;
	xml::document xml_;
};

ini_file::ini_file(std::wstring filename) :
	pimpl_(new ini_impl(filename))
{}

ini_file::~ini_file()
{}

void ini_file::load_data()
{
	pimpl_->load_data();
}

void ini_file::save_data()
{
	pimpl_->save_data();
}

bool ini_file::save(boost::filesystem::path location, std::string data)
{
	return pimpl_->save(location, data);
}

bool ini_file::save(boost::filesystem::path location, xml::node* data)
{
	return pimpl_->save(location, data);
}

xml::node* ini_file::load(boost::filesystem::path location)
{
	return pimpl_->load(location);
}

boost::filesystem::wpath ini_file::main_file() const 
{ 
	return pimpl_->main_file();
}
	
boost::filesystem::wpath ini_file::working_file() const 
{ 
	return pimpl_->working_file();
}

void ini_file::clear()
{
	pimpl_->clear();
}

ini_file& ini()
{
	static ini_file ini(L"Halite.xml");
	return ini;
}

} // namespace hal
