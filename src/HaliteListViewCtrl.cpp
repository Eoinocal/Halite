
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "HaliteListViewCtrl.hpp"
#include "GlobalIni.hpp"
#include "ini/Window.hpp"
#include "halTorrent.hpp"

using namespace std;
using namespace boost;

void HaliteListViewCtrl::onShowWIndow(UINT, INT)
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT);

	CHeaderCtrl hdr = GetHeader();
	hdr.ModifyStyle(HDS_BUTTONS, 0);

	AddColumn(L"Name", hdr.GetItemCount());
	AddColumn(L"Status", hdr.GetItemCount());
	AddColumn(L"Completed", hdr.GetItemCount());
	AddColumn(L"Download", hdr.GetItemCount());
	AddColumn(L"Upload", hdr.GetItemCount());
	AddColumn(L"Peers", hdr.GetItemCount());
	AddColumn(L"Seeds", hdr.GetItemCount());

	for (size_t i=0; i<WindowConfig::numMainCols; ++i)
		SetColumnWidth(i, INI().windowConfig().mainListColWidth[i]);
}

void HaliteListViewCtrl::updateListView()
{
	halite::TorrentDetails TD = halite::bittorrent().getAllTorrentDetails();
	
	for (halite::TorrentDetails::const_iterator i = TD.begin(); i != TD.end(); ++i) 
	{
		LV_FINDINFO findInfo; 
		findInfo.flags = LVFI_STRING;
		findInfo.psz = const_cast<LPTSTR>((*i)->filename().c_str());
		
		int itemPos = FindItem(&findInfo, -1);
		if (itemPos < 0)
			itemPos = AddItem(0, 0, (*i)->filename().c_str(), 0);
		
		SetItemText(itemPos, 1, (*i)->state().c_str());
		
		SetItemText(itemPos, 2, (wformat(L"%1$.2f%%") 
				% ((*i)->completion()*100)).str().c_str());
		
		SetItemText(itemPos, 3, (wformat(L"%1$.2fkb/s") 
				% ((*i)->speed().first/1024)).str().c_str());	
		
		SetItemText(itemPos, 4, (wformat(L"%1$.2fkb/s") 
				% ((*i)->speed().second/1024)).str().c_str());
		
		SetItemText(itemPos, 5,	(lexical_cast<wstring>((*i)->peers())).c_str());
		
		SetItemText(itemPos, 6,	(lexical_cast<wstring>((*i)->seeds())).c_str());	
	}	
}

void HaliteListViewCtrl::saveStatus()
{
	for (size_t i=0; i<WindowConfig::numMainCols; ++i)
		INI().windowConfig().mainListColWidth[i] = GetColumnWidth(i);
}
