
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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
		connection_ = haliteWindow.connectUiUpdate(bind(&thisClass::handleUiUpdate, this, _1));
	}

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_SHOWWINDOW(OnShow)
//		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
	
	void OnShow(BOOL show, int flags)
	{
		if (show)
		{
			hal::event().post(shared_ptr<hal::EventDetail>(
				new hal::EventMsg(L"True")));
			connection_.unblock();
		}
		else
		{
			hal::event().post(shared_ptr<hal::EventDetail>(
				new hal::EventMsg(L"False")));
			connection_.block();
		}
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
	
	void handleUiUpdate(const hal::TorrentDetails& tD)
	{
		TBase* pT = static_cast<TBase*>(this);
		wstring torrent_name = L"";
		
		if (hal::TorrentDetail_ptr torrent = tD.focusedTorrent()) 	
			torrent_name = torrent->name();
		
		if (current_torrent_name_ != torrent_name)
		{	
			current_torrent_name_ = torrent_name;
			
			hal::event().post(shared_ptr<hal::EventDetail>(
				new hal::EventMsg(L"focusChanged")));
				
			pT->focusChanged(tD.focusedTorrent());
		}
	
		pT->uiUpdate(tD);
	}

	void focusChanged(const hal::TorrentDetail_ptr pT)
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
	wstring current_torrent_name_;

private:
	HaliteWindow& haliteWindow_;
	boost::signals::scoped_connection connection_;
};

