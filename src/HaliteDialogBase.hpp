
#pragma once

#include "global/ini_adapter.hpp"
#include "halEvent.hpp"
#include "HaliteWindow.hpp"
#include "HaliteListView.hpp"

#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

template <class TBase>
class CHaliteDialogBase
{
	typedef CHaliteDialogBase<TBase> thisClass;
	
public:
	CHaliteDialogBase(HaliteWindow& haliteWindow) :
		haliteWindow_(haliteWindow)
	{		
		haliteWindow.connectUiUpdate(bind(&TBase::uiUpdate, static_cast<TBase*>(this), _1));
	}
	
	void InitializeHalDialogBase()
	{	
	//	ui_.attach(bind(&TBase::updateDialog, static_cast<TBase*>(this)));
	//	selection_manager_.attach(bind(&TBase::selectionChanged, static_cast<TBase*>(this), _1));
	}
	
	void requestUiUpdate()
	{
		haliteWindow_.issueUiUpdate();
	}
	
	HaliteListViewCtrl& torrentsList() 
	{ 
		return haliteWindow_.torrentsList(); 
	}
	
	void uiUpdate(const hal::TorrentDetails& tD)
	{}
	
	template<typename T>
	BOOL SetDlgItemInfo(int nID, T info)
	{
		std::wostringstream oss;
		oss << info;
		TBase* pT = static_cast<TBase*>(this);
		return pT->SetDlgItemText(nID, oss.str().c_str());
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
protected:

private:
	HaliteWindow& haliteWindow_;
};

