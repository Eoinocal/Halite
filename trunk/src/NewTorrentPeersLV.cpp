
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
	WTL::CMenuHandle menu;
	BOOL menu_created = menu.LoadMenu(HAL_GENERIC_ADD_LV_MENU);
	InitialSetup(menu);	

	std::vector<wstring> names;	
	wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);

	// "Tracker;Tier"
	boost::split(names, column_names, boost::is_any_of(L";"));
	
	array<int, 3> widths = {287,50,50};
	array<int, 3> order = {0,1,2};
	array<bool, 3> visible = {true,true,true};
	
	for (int i=0, e=3; i < e; ++i)
	{
		AddColumn(names[i].c_str(), i, visible[i], widths[i]);
	}	

	load_from_ini();	
	
	SetColumnSortType(1, WTL::LVCOLSORT_LONG);
}

void NewTorrent_PeersListViewCtrl::OnDestroy()
{
	saveSettings();
}

void NewTorrent_PeersListViewCtrl::saveSettings()
{		
	GetListViewDetails();
	save_to_ini();
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
	GetItemText(index, 0, str_url, str_url.size());

	hal::win_c_str<std::wstring> str_port(MAX_PATH);	
	GetItemText(index, 1, str_port, str_port.size());

	hal::win_c_str<std::wstring> str_type(MAX_PATH);
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
		hal::event_log.post(boost::shared_ptr<hal::EventDetail>(
			new hal::EventStdException(hal::event_logger::critical, e, 
				L"NewTorrent_PeersListViewCtrl::editItem")));
	}
}

void NewTorrent_PeersListViewCtrl::deleteItem(int index)
{
	DeleteItem(index);
}
