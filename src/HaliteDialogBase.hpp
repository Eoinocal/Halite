
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
	{}
	
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
	
protected:

private:
	HaliteWindow& haliteWindow_;
};

