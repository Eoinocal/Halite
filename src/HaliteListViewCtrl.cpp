
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "HaliteListViewCtrl.hpp"
#include "halTorrent.hpp"

using namespace std;
using namespace boost;

void HaliteListViewCtrl::updateListView()
{
	halite::vecTorrentDetails vTD = halite::bittorrent().getAllTorrentDetails();
	
	for (halite::vecTorrentDetails::const_iterator i = vTD.begin(); 
		i != vTD.end(); ++i) 
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