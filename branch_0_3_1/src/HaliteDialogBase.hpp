
//         Copyright Eóin O'Callaghan 2006 - 2008.
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
			connection_.unblock();
		}
		else
		{
			connection_.block();
		}
		
		SetMsgHandled(false);
	}
	
	void InitializeHalDialogBase()
	{
		TBase* pT = static_cast<TBase*>(this);	
		OnShow(pT->IsWindowVisible(), 0);
	}
	
	HaliteListViewCtrl& torrentsList() 
	{ 
		return haliteWindow_.torrentsList(); 
	}
	
	void requestUiUpdate()
	{
		haliteWindow_.issueUiUpdate();
	}
	
	#define logical_xor !=0==!
	
	void handleUiUpdate(const hal::TorrentDetails& tD)
	{
		TBase* pT = static_cast<TBase*>(this);
		
		hal::TorrentDetail_ptr focused = tD.focusedTorrent();
		
		if ((focusedTorrent_ logical_xor focused) ||
				(focused && focusedTorrent_->name() != focused->name()))
		{
			pT->focusChanged(focusedTorrent_ = focused);
			
			hal::event_log.post(shared_ptr<hal::EventDetail>(
				new hal::EventMsg(L"Adv dialog focusChanged")));
		}
		else
			focusedTorrent_ = focused;
	
		pT->uiUpdate(tD);
	}

	void uiUpdate(const hal::TorrentDetails& tD)
	{}	
	
	void focusChanged(const hal::TorrentDetail_ptr pT)
	{}
	
	const hal::TorrentDetail_ptr focusedTorrent() { return focusedTorrent_; }
	
	template<typename T>
	BOOL SetDlgItemInfo(int nID, T info)
	{
		std::wostringstream oss;
		oss << info;
		TBase* pT = static_cast<TBase*>(this);
		return pT->SetDlgItemText(nID, oss.str().c_str());
	}
	
protected:
	hal::TorrentDetail_ptr focusedTorrent_;

private:
	HaliteWindow& haliteWindow_;
	boost::signals::scoped_connection connection_;
};

