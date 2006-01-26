
#include "HaliteListViewCtrl.hpp"
#include "halTorrent.hpp"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

void HaliteListViewCtrl::updateListView()
{
	halite::torrentBriefDetails tbd = halite::getTorrents();
	if (tbd) {
		
		for(size_t i=0; i<tbd->size(); i++) 
		{
			LV_FINDINFO findInfo; 
			findInfo.flags = LVFI_STRING;
			findInfo.psz = const_cast<LPTSTR>((*tbd)[i].filename.c_str());
			
			int itemPos = FindItem(&findInfo, -1);
			if (itemPos < 0)
				int itemPos = AddItem(0,0,(*tbd)[i].filename.c_str(),0);
			
			SetItemText(itemPos,1,(*tbd)[i].status.c_str());
			
			SetItemText(itemPos,2,
				(wformat(L"%1$.2f%%") 
					% ((*tbd)[i].completion * 100)
				).str().c_str());
			
			SetItemText(itemPos,3,
				(wformat(L"%1$.2fkb/s") 
					% ((*tbd)[i].speed.first/1000)
				).str().c_str());	
			
			SetItemText(itemPos,4,
				(wformat(L"%1$.2fkb/s") 
					% ((*tbd)[i].speed.second/1000)
				).str().c_str());	
			
			SetItemText(itemPos,5,
				(lexical_cast<wstring>((*tbd)[i].peers)).c_str()
				);
			
			SetItemText(itemPos,6,
				(lexical_cast<wstring>((*tbd)[i].seeds)).c_str()
				);
		}
	}	
}