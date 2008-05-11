
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"
#include "halTorrent.hpp"

#include "NewTorrentPeersLV.hpp"
#include "NewTorrentPeersAD.hpp"

void NewTorrent_PeersListViewCtrl::OnAttach()
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);
	SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
	
	ApplyDetails();
	
	SetColumnSortType(1, LVCOLSORT_LONG);
}

void NewTorrent_PeersListViewCtrl::OnDestroy()
{
	saveSettings();
}

void NewTorrent_PeersListViewCtrl::saveSettings()
{		
	GetListViewDetails();
	save();
}

void NewTorrent_PeersListViewCtrl::newItem()
{
	hal::web_seed_or_dht_node_detail peer;	
	NewTorrent_PeersAddDialog trackDlg(hal::app().res_wstr(HAL_NEWT_ADD_NEW_PEER), peer);
	
	if (trackDlg.DoModal() == 1 && !peer.url.empty()) 
	{
		int itemPos = AddItem(0, 0, peer.url.c_str(), 0);		
		SetItemText(itemPos, 1, lexical_cast<wstring>(peer.port).c_str());
		SetItemText(itemPos, 2, peer.type.c_str());
	}
}

void NewTorrent_PeersListViewCtrl::editItem(int index)
{	
	try {

	hal::win_c_str<std::wstring> str_url(MAX_PATH);	
	hal::win_c_str<std::wstring> str_port(MAX_PATH);	
	hal::win_c_str<std::wstring> str_type(MAX_PATH);	

	GetItemText(index, 0, str_url, str_url.size());
	GetItemText(index, 1, str_port, str_port.size());
	GetItemText(index, 2, str_type, str_type.size());
	
	hal::web_seed_or_dht_node_detail peer;

	if (hal::app().res_wstr(HAL_NEWT_ADD_PEERS_WEB) == str_type.str())
	{
		peer.url = str_url.str();
		peer.port = -1;
		peer.type = hal::app().res_wstr(HAL_NEWT_ADD_PEERS_WEB);
	}
	else
	{
		peer.url = str_url.str();
		peer.port = lexical_cast<unsigned>(str_port.str());
		peer.type = hal::app().res_wstr(HAL_NEWT_ADD_PEERS_DHT);
	}

	NewTorrent_PeersAddDialog trackDlg(hal::app().res_wstr(HAL_NEWT_EDIT_PEER), peer);
	
	if (trackDlg.DoModal() == 1) 
	{
		if (peer.url.empty())
		{
			DeleteItem(index);
		}
		else
		{
			SetItemText(index, 0, peer.url.c_str());	
			SetItemText(index, 1, lexical_cast<wstring>(peer.port).c_str());
			SetItemText(index, 2, peer.type.c_str());
		}		
	}	

	}
	catch(const std::exception& e)
	{
		hal::event().post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(hal::Event::critical, e, 
				L"NewTorrent_PeersListViewCtrl::editItem")));
	}
}

void NewTorrent_PeersListViewCtrl::deleteItem(int index)
{
	DeleteItem(index);
}
