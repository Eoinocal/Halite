
#pragma once

#include "global/ini_adapter.hpp"
#include "halEvent.hpp"
#include "HaliteWindow.hpp"
#include "HaliteListView.hpp"

#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

template <class T>
class CHaliteDialogBase
{
public:
	CHaliteDialogBase(ui_signal& ui_sig, ListViewManager& single_sel) :
		ui_(ui_sig),
		selection_manager_(single_sel)
	{}
	
	void InitializeHalDialogBase()
	{	
		ui_.attach(bind(&T::updateDialog, static_cast<T*>(this)));
		selection_manager_.attach(bind(&T::selectionChanged, static_cast<T*>(this), _1));
	}
	
/*	void save()
	{
		std::wstringstream xml_data;
		
		boost::archive::xml_woarchive oxml(xml_data);
		T* pT = static_cast<T*>(this);	
		oxml << boost::serialization::make_nvp(name_.c_str(), *pT);
		
		adapter_.save_stream_data(xml_data);
	}
	
	void load()
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
*/	
	ui_signal& ui() { return ui_; }
	ListViewManager&  selection_manager() { return selection_manager_; }
	
private:
	ui_signal& ui_;
	ListViewManager& selection_manager_;
};

