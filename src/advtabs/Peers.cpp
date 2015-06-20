
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include "../HaliteWindow.hpp"
#include "../HaliteListView.hpp"
#include "../global/logger.hpp"

#include "Peers.hpp"


bool PeerListView::SubclassWindow(HWND hwnd)
{
	if(!list_class_t::SubclassWindow(hwnd))
		return false;

	InitialSetup();	
	
	std::vector<wstring> names;
	wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);

	boost::split(names, column_names, boost::is_any_of(L";"));

	//	Peer;			Port;			Country;			Download;		Upload;			Type;			Client,			Status"
	array<int, 8> widths = {
		100,				50,				50,				70,				70,				70,				100,				200};
	array<int, 8> order = {
		0,				1,				2,				3,				4,				5,				6,				7};
	array<bool, 8> visible = {
		true,			true,			true,			true,			true,			true,			true,			true};

	array<int, 8> formats = {
		LVCFMT_CENTER,	LVCFMT_CENTER,	LVCFMT_CENTER,	LVCFMT_RIGHT,	LVCFMT_RIGHT,	LVCFMT_CENTER,	LVCFMT_CENTER,	LVCFMT_LEFT};

	for (int i=0, e=8; i < e; ++i)
		AddColumn(names[i].c_str(), i, visible[i], widths[i], formats[i]);

	safe_load_from_ini();	

	for (unsigned i=0, e = hal::peer_detail::status_e-hal::peer_detail::ip_address_e; i <= e; ++i)
		SetColumnSortType(i, i + (WTL::LVCOLSORT_LAST+1+hal::peer_detail::ip_address_e));		

//	SetColumnSortType(2, hal::peer_detail::speed_down_e + (WTL::LVCOLSORT_LAST+1+hal::peer_detail::ip_address_e), NULL);		
//	SetColumnSortType(3, hal::peer_detail::speed_up_e + (WTL::LVCOLSORT_LAST+1+hal::peer_detail::ip_address_e), NULL);
	
	return true;
}

bool PeerListView::sort_list_comparison(std::wstring l, std::wstring r, size_t index, bool ascending)
{
	hal::peer_details_vec::optional_type pdl = peer_details_.find_peer(l);
	hal::peer_details_vec::optional_type pdr = peer_details_.find_peer(r);

	if (pdl && pdr) 
		return hal::hal_details_ptr_compare(pdl, pdr, index, ascending);
	else
		return false;
}

LRESULT PeerListView::OnGetDispInfo(int, LPNMHDR pnmh, BOOL&)
{	
	hal::mutex_t::scoped_lock l(list_class_t::mutex_);

	NMLVDISPINFO* pdi = (NMLVDISPINFO*)pnmh;

	HAL_DEV_SORT_MSG(hal::wform(L"OnGetDispInfo index = %1% size = %2%") % pdi->item.iItem % peer_details_.size());

	if (pdi->item.iItem >= 0 && peer_details_.size() >= numeric_cast<unsigned>(pdi->item.iItem)) 
	{	

	hal::peer_details_vec::optional_type pd = peer_details_.find_peer(key_from_index(pdi->item.iItem));

	if (pd && pdi->item.mask & LVIF_TEXT)
	{
		wstring str = pd->to_wstring(pdi->item.iSubItem);
		
		size_t len = str.copy(pdi->item.pszText, min(pdi->item.cchTextMax - 1, static_cast<int>(str.size())));
		pdi->item.pszText[len] = '\0';
	}

	}
	
	return 0;
}

void PeerListView::uiUpdate(const hal::torrent_details_manager& tD)
{
	if (hal::try_update_lock<list_class_t> lock = hal::try_update_lock<list_class_t>(this)) 
	{		
		selection_from_listview();
		
		peer_details_.clear();
		
		BOOST_FOREACH (const hal::uuid& id, tD.selected_uuids())
		{
			const hal::torrent_details_ptr t = tD.get(id);
			if (t)
			{
			std::copy(t->get_peer_details().begin(), t->get_peer_details().end(), 
				std::inserter(peer_details_, peer_details_.begin()));
			}
		}
		
		std::set<std::wstring> ip_set;
		BOOST_FOREACH (const hal::peer_detail& pd,  peer_details_)
			ip_set.insert(pd.ip_address);
		
		erase_based_on_set(ip_set, true);

		if (IsSortOnce() || AutoSort())
		{
			if (GetSecondarySortColumn() != -1)
			{
				int index = GetColumnSortType(GetSecondarySortColumn());					
				if (index > WTL::LVCOLSORT_LAST)
					sort(index - (WTL::LVCOLSORT_LAST+1+hal::peer_detail::ip_address_e), IsSecondarySortDescending());
			}

			if (GetSortColumn() != -1)
			{		
				int index = GetColumnSortType(GetSortColumn());				
				if (index > WTL::LVCOLSORT_LAST)
					sort(index - (WTL::LVCOLSORT_LAST+1+hal::peer_detail::ip_address_e), IsSortDescending());
			}
		}
		
		set_keys(ip_set);		
		InvalidateRect(NULL,true);
	}
}

LRESULT PeerListView::OnSortChanged(int, LPNMHDR pnmh, BOOL&)
{
	halite_window_.issueUiUpdate();
	
	return 0;
}

void AdvPeerDialog::uiUpdate(const hal::torrent_details_manager& tD)
{
	peerList_.uiUpdate(tD);
}

LRESULT AdvPeerDialog::OnInitDialog(HWND, LPARAM)
{	
	peerList_.SubclassWindow(GetDlgItem(HAL_PEERLIST));
	
	resize_class_t::DlgResize_Init(false, true, WS_CLIPCHILDREN);	

	return 0;
}

void AdvPeerDialog::OnClose()
{
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

void AdvPeerDialog::OnDestroy()
{	 
	peerList_.saveSettings();
}

