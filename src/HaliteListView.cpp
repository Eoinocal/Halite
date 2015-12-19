
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include <boost/iterator/filter_iterator.hpp>

#include "Halite.hpp"

#include "halTorrent.hpp"

#include "HaliteListView.hpp"
#include "HaliteWindow.hpp"
#include "HaliteListViewDlg.hpp"

#define HAL_CUSTOMDRAW_TITLEDATA 1000000000

template<>
inline const std::wstring hal::to_wstr_shim<const HaliteListViewCtrl::list_value_type>
	(const HaliteListViewCtrl::list_value_type& v)
{
	return std::wstring(v.text());
}

template<>
inline const std::wstring hal::to_wstr_shim<HaliteListViewCtrl::list_value_type>
	(HaliteListViewCtrl::list_value_type& v)
{
	return std::wstring(v.text());
}

HaliteListViewCtrl::ex_list_value_type::ex_list_value_type(const list_class_t::list_value_type& l) :
	list_class_t::list_value_type(l)
{}

HaliteListViewCtrl::HaliteListViewCtrl(HaliteWindow& HalWindow) :
	halite_window_(HalWindow),
	ini_class_t(L"listviews/halite", L"halite_listview"),
	editing_lock_(false),
	queue_view_(false)
{		
	HalWindow.connectUiUpdate(bind(&HaliteListViewCtrl::uiUpdate, this, _1));
}

void HaliteListViewCtrl::OnShowWindow(UINT, INT)
{
	WTL::CMenuHandle menu;
	BOOL menu_created = menu.LoadMenu(HAL_LISTVIEW_MENU);
	InitialSetup(menu);	

	std::vector<wstring> names;	
	wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);
	
	boost::split(names, column_names, boost::is_any_of(L";"));
	
	// 	Name;			Status;			Progress;			Download;		Upload;			Peers;			Seeds;	
	//	ETA;				Copies;			Tracker;			Reannounce;		Ratio;			Total;			Completed;
	//	Remaining;		Downloaded;		Uploaded;		Active;			Seeding;			Start Time;		Finish Time;
	//	Managed;		Queue Position	Uuid				Hash"

	array<int, NumberOfColumns_s> widths = {
		180,				110,				60,				60,				60,				42,				45,		
		61,				45,				45,				45,				45,				45,				45,
		45,				45,				45,				45,				45,				45,				45,
		30,				45,				210,				240};

	array<int, NumberOfColumns_s> formats = {
		0,				0,				LVCFMT_RIGHT,	LVCFMT_RIGHT,	LVCFMT_RIGHT,	LVCFMT_CENTER,	LVCFMT_CENTER,	
		LVCFMT_RIGHT,	LVCFMT_RIGHT,	0,				LVCFMT_RIGHT,	LVCFMT_RIGHT,	LVCFMT_RIGHT,	LVCFMT_RIGHT,	
		LVCFMT_RIGHT,	LVCFMT_RIGHT,	LVCFMT_RIGHT,	LVCFMT_RIGHT,	LVCFMT_RIGHT,	LVCFMT_RIGHT,	LVCFMT_RIGHT,	
		0,				LVCFMT_RIGHT,	0,				0};

	array<bool, NumberOfColumns_s> visible = {
		true,			true,			true,			true,			true,			true,			true,
		true,			true,			false,			false,			true,			false,			false,
		false,			false,			false,			false,			false,			false,			false,
		false,			false,			false,			false};

	for (int i=0, e=NumberOfColumns_s; i < e; ++i)
	{
		AddColumn(names[i].c_str(), i, visible[i], widths[i], formats[i]);
	}	
	
	safe_load_from_ini();

	for (unsigned i=0, e = hal::torrent_details::hash_e-hal::torrent_details::name_e; i <= e; ++i)
		SetColumnSortType(i, i + (WTL::LVCOLSORT_LAST+1+hal::torrent_details::name_e));
	
	queue_view_mode();
}

void HaliteListViewCtrl::OnDestroy()
{
	SaveSettings();
}

void HaliteListViewCtrl::SaveSettings()
{
	GetListViewDetails();
	save_to_ini();
}

DWORD HaliteListViewCtrl::OnPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCD)
{
	return CDRF_NOTIFYITEMDRAW;
}

DWORD HaliteListViewCtrl::OnItemPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCD)
{
	NMLVCUSTOMDRAW* pnmlv = (NMLVCUSTOMDRAW*) lpNMCD;

	if (HAL_CUSTOMDRAW_TITLEDATA == pnmlv->nmcd.lItemlParam)
	{
		pnmlv->clrText = RGB(50,50,200);
	}

	return CDRF_DODEFAULT;
}

bool HaliteListViewCtrl::sort_list_comparison(list_class_data_t l, list_class_data_t r, size_t index, bool ascending)
{
	try
	{

	if (hal::torrent_details_ptr left = hal::bittorrent().torrentDetails().get(l))
	{
		if (hal::torrent_details_ptr right = hal::bittorrent().torrentDetails().get(r))
			return hal::hal_details_ptr_compare(left, right, index, ascending);
		else
			return true;	// Huh?
	}
	else
		return false;	// Huh also?  Well these two being opposite means an 
						// invalid entry always compares as greater. Unless
						// are two invalids!!
	}
	HAL_GENERIC_FN_EXCEPTION_CATCH(L"in HaliteListViewCtrl::sort_list_comparison")

	return false;
}

LRESULT HaliteListViewCtrl::OnGetDispInfo(int, LPNMHDR pnmh, BOOL&)
{	
	if (hal::try_update_lock<list_class_t> lock{ this })
	{	

	if (pnmh == 0)
		return 0;
	
	NMLVDISPINFO* pdi = (NMLVDISPINFO*)pnmh;
	hal::torrent_details_ptr td = hal::bittorrent().torrentDetails().get(key_from_index(pdi->item.iItem));

	if (td && pdi->item.mask & LVIF_TEXT)
	{
		wstring str = td->to_wstring(pdi->item.iSubItem);
		
		size_t len = str.copy(pdi->item.pszText, min(pdi->item.cchTextMax - 1, static_cast<int>(str.size())));
		pdi->item.pszText[len] = '\0';
	}

	}
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnBeginLabelEdit(int i, LPNMHDR pnmh, BOOL&)
{		
	HAL_DEV_MSG(hal::wform(L"OnBeginLabelEdit(int i = %1%)") % i);

	editing_lock_ = true;
	{	
		NMLVDISPINFO* nmlv = (NMLVDISPINFO*)pnmh;
		
		HAL_DEV_MSG(hal::wform(L"LabelEdit Locked!") % i);
	}

	return false;
}

LRESULT HaliteListViewCtrl::OnEndLabelEdit(int i, LPNMHDR pnmh, BOOL&)
{	
	NMLVDISPINFO* pdi = (NMLVDISPINFO*)pnmh;

/*	if (pdi->item.iItem < static_cast<int>(files_.size()) && pdi->item.mask & LVIF_TEXT)
	{
		wstring str = files_[pdi->item.iItem].to_wstring(pdi->item.iSubItem);
		
		HAL_DEV_MSG(hal::wform(L"iItem: %1%, Order: %2%, text: %3%, orig: %4%") 
			% pdi->item.iItem % files_[pdi->item.iItem].order() % pdi->item.pszText % str);

		if (hal::bit::torrent t = hal::bittorrent().get(focused()))
		{
			wpath old_name = t.files()[files_[pdi->item.iItem].order()].name;
			t.files()[files_[pdi->item.iItem].order()].name = old_name.parent_path()/wstring(pdi->item.pszText);
		}
	}
*/
	editing_lock_ = false;
	HAL_DEV_MSG(hal::wform(L"OnEndLabelEdit(int i = %1%) Unlocked!") % i);

	return false;
}

void HaliteListViewCtrl::uiUpdate(const hal::torrent_details_manager& tD)
{
	if (hal::try_update_lock<list_class_t> lock{ this })
	{		

	selection_from_listview();
		
	std::set<hal::uuid> torrent_set;
	BOOST_FOREACH (hal::torrent_details_ptr t,  tD.torrents())
	{
		torrent_set.insert(t->uuid());
	}
	
	erase_based_on_set(torrent_set, true);	

	if (!editing_lock_)
	{
		if (IsSortOnce() || AutoSort())
		{
			if (GetSecondarySortColumn() != -1)
			{
				int index = GetColumnSortType(GetSecondarySortColumn());					
				if (index > WTL::LVCOLSORT_LAST)
					sort(index - (WTL::LVCOLSORT_LAST+1+hal::torrent_details::name_e), IsSecondarySortDescending());
			}

			if (GetSortColumn() != -1)
			{		
				int index = GetColumnSortType(GetSortColumn());				
				if (index > WTL::LVCOLSORT_LAST)
					sort(index - (WTL::LVCOLSORT_LAST+1+hal::torrent_details::name_e), IsSortDescending());
			}
		}
		
		if (queue_view_)
			sort(hal::torrent_details::managed_e, false);
	}

	set_keys(torrent_set);	
	InvalidateRect(NULL,true);

	}
}

LRESULT HaliteListViewCtrl::OnSortChanged(int, LPNMHDR pnmh, BOOL&)
{
	halite_window_.issueUiUpdate();
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	for (auto i = begin_selected(), e = end_selected(); i != e; ++i)
	{
		hal::bittorrent().resume_torrent(item_hash(i));
	}
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	for (auto i = begin_selected(), e = end_selected(); i != e; ++i)
	{
		hal::bittorrent().pause_torrent(item_hash(i));
	}
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	for (auto i = begin_selected(), e = end_selected(); i != e; ++i)
	{
		hal::bittorrent().stop_torrent(item_hash(i));
	}

	return 0;
}

LRESULT HaliteListViewCtrl::OnRemoveFocused(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	hal::bittorrent().remove_torrent(item_hash(begin_selected()));
	erase_from_list(*begin_selected());

	return 0;
}


LRESULT HaliteListViewCtrl::OnRecheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	for (auto i = begin_selected(), e = end_selected(); i != e; ++i)
	{
		//HAL_DEV_MSG(hal::wform(L"UUid: %1%") % v.text(hal::torrent_details::uuid_e-hal::torrent_details::name_e));

		hal::bittorrent().recheck_torrent(item_hash(i));
	}

	return 0;
}

LRESULT HaliteListViewCtrl::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::set<hal::uuid> torrent_ids;

	for (auto i = begin_selected(), e = end_selected(); i != e; ++i)
		torrent_ids.insert(item_hash(i));

	BOOST_FOREACH(const hal::uuid& id, torrent_ids)
		hal::bittorrent().remove_torrent_callback(
			id, 			
			[=](hal::fs::wpath active_directory, boost::shared_ptr<std::vector<std::wstring> > files) 
			{
				erase_from_list(id);
				hal::bittorrent().remove_torrent(id);
			});

	return 0;
}

void HaliteListViewCtrl::remove_to_bin(const hal::uuid& id, hal::fs::wpath active_directory, boost::shared_ptr<std::vector<std::wstring> > files)
{
	std::vector<wchar_t> file_names_buffer;

	BOOST_FOREACH(std::wstring file, *files)
	{
		if (hal::fs::exists(file))
		{
			HAL_DEV_MSG(hal::wform(L"File %1%") % file);

			std::copy(file.begin(), file.end(), 
				std::back_inserter(file_names_buffer));
			file_names_buffer.push_back(L'\0');
		}
	}
	file_names_buffer.push_back(L'\0');

	boost::thread t([=]() {
			SHFILEOPSTRUCT shf;

			shf.hwnd = *this;
			shf.wFunc = FO_DELETE;
			shf.pFrom = &file_names_buffer[0];
			shf.pTo = 0;
			shf.fFlags = FOF_ALLOWUNDO;

			HAL_DEV_MSG(L"Calling SHFileOperation to remove files");
			SHFileOperation(&shf);

			HAL_DEV_MSG(hal::wform(L"Clearing empty directories at %1%") % active_directory.wstring());
			hal::remove_empty_directories(active_directory);
		});

	erase_from_list(id);
	hal::bittorrent().remove_torrent(id);
}

LRESULT HaliteListViewCtrl::OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::set<hal::uuid> torrent_names;
	
	for (auto i = begin_selected(), e = end_selected(); i != e; ++i)
		torrent_names.insert(item_hash(i));

	BOOST_FOREACH(const hal::uuid& id, torrent_names)
		hal::bittorrent().remove_torrent_callback(id, boost::bind(&HaliteListViewCtrl::remove_to_bin, this, id, _1, _2));

	return 0;
}

LRESULT HaliteListViewCtrl::OnDownloadFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	try
	{

	HAL_DEV_MSG(L"OnDownloadFolder");

	std::set<wpath> unique_paths;
	
	for (auto i = begin_selected(), e = end_selected(); i != e; ++i)
	{
		hal::bit::torrent t = hal::bittorrent().get(item_hash(i));

		auto save_dir = t.save_directory();
		if (boost::filesystem::is_directory(save_dir/wpath(t.name())))
			save_dir /= wpath(t.name());

		HAL_DEV_MSG(hal::wform(L"Name %1%, Save dir: %2%.") % i->text() % save_dir);

		unique_paths.insert(save_dir);
	}

	SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };

	for (std::set<wpath>::const_iterator i=unique_paths.begin(), e=unique_paths.end();
		i != e; ++i)
	{	
		wstring p = (*i).wstring();

		HAL_DEV_MSG(hal::wform(L"Unique Save dir: %1%.") % p);

		sei.lpDirectory = p.c_str();
		sei.lpFile = p.c_str();
		sei.lpVerb = L"open";
		sei.nShow = true;

		if (!::ShellExecuteEx(&sei))
			HAL_DEV_MSG(L"Fail");
		else
			HAL_DEV_MSG(L"Success");
	}	

	}
	HAL_GENERIC_FN_EXCEPTION_CATCH(L"in HaliteListViewCtrl::OnDownloadFolder")

	return 0;
}

LRESULT HaliteListViewCtrl::OnEditFolders(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	try
	{

	HAL_DEV_MSG(L"OnEditFolders");

	if (hal::bit::torrent t = hal::bittorrent().get(item_hash(begin_selected())))
	{
		auto saveDirectory = static_cast<path>(t.save_directory()).wstring();
		auto moveToDirectory = static_cast<path>(t.move_to_directory()).wstring();

		bool useMoveTo = !moveToDirectory.empty();
		bool disableSaveDir = !t.in_session();

		HaliteListViewAdjustDlg addTorrent(hal::app().res_wstr(HAL_ADDT_TITLE), saveDirectory, moveToDirectory, 
			useMoveTo, disableSaveDir);	
		
		if (IDOK == addTorrent.DoModal())
		{
			if (!disableSaveDir) t.set_save_directory(saveDirectory);

			t.set_move_to_directory(useMoveTo ? moveToDirectory : wstring());
		}
	}

	}
	HAL_GENERIC_FN_EXCEPTION_CATCH(L"in HaliteListViewCtrl::OnEditFolders")

	return 0;
}


LRESULT HaliteListViewCtrl::OnToggleSuperseeding(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	try
	{

	std::set<hal::uuid> torrent_names;
	
	for (auto i = begin_selected(), e = end_selected(); i != e; ++i)
		torrent_names.insert(item_hash(i));

	BOOST_FOREACH(const hal::uuid& id, torrent_names)
		if (hal::bit::torrent t = hal::bittorrent().get(id))
		{
			t.set_superseeding(!t.superseeding());
		}

	halite_window_.issueUiUpdate();

	}
	HAL_GENERIC_FN_EXCEPTION_CATCH(L"in HaliteListViewCtrl::OnToggleSuperseeding")

	return 0;
}

LRESULT HaliteListViewCtrl::OnSetManaged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	try
	{

/*	hal::try_update_lock<list_class_t> lock(*this);
	if (lock) 
	{		
*/
	std::set<hal::uuid> torrent_names;
	
	for (auto i = begin_selected(), e = end_selected(); i != e; ++i)
		torrent_names.insert(item_hash(i));
	
//	erase_based_on_set(torrent_names, false);

	BOOST_FOREACH(const hal::uuid& id, torrent_names)
		hal::bittorrent().get(id).set_managed(true);
//	}

	halite_window_.issueUiUpdate();

	}
	HAL_GENERIC_FN_EXCEPTION_CATCH(L"in HaliteListViewCtrl::OnSetManaged")

	return 0;
}

LRESULT HaliteListViewCtrl::OnSetUnmanaged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	try
	{

/*	hal::try_update_lock<list_class_t> lock(*this);
	if (lock) 
	{		
*/
	std::set<hal::uuid> torrent_names;
	
	for (auto i = begin_selected(), e = end_selected(); i != e; ++i)
		torrent_names.insert(item_hash(i));
	
//	erase_based_on_set(torrent_names, false);

	BOOST_FOREACH(const hal::uuid& id, torrent_names)
		hal::bittorrent().get(id).set_managed(false);
//	}

	halite_window_.issueUiUpdate();

	}
	HAL_GENERIC_FN_EXCEPTION_CATCH(L"in HaliteListViewCtrl::OnSetUnmanaged")

	return 0;
}

LRESULT HaliteListViewCtrl::OnAdjustQueuePosition(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	try
	{
	
	for (auto i = begin_selected(), e = end_selected(); i != e; ++i)
	{
		hal::bit::torrent t = hal::bittorrent().get(item_hash(i));

		switch (wID)
		{
		case HAL_QUEUE_MOVE_TOP:
			t.adjust_queue_position(hal::bit::move_to_top);
			break;
		case HAL_QUEUE_MOVE_UP:
			t.adjust_queue_position(hal::bit::move_up);		
			break;
		case HAL_QUEUE_MOVE_DOWN:
			t.adjust_queue_position(hal::bit::move_down);		
			break;
		case HAL_QUEUE_MOVE_BOTTOM:
			t.adjust_queue_position(hal::bit::move_to_bottom);		
			break;
		};
	}

	halite_window_.issueUiUpdate();

	}
	HAL_GENERIC_FN_EXCEPTION_CATCH(L"in HaliteListViewCtrl::OnAdjustQueuePosition")
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnQueueView(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
//	queue_view_ ^= true;
	queue_view_ = false;

	queue_view_mode();
	
	return 0;
}

void HaliteListViewCtrl::erase_torrent_name(const hal::uuid& id)
{
	erase_from_list(id);
}

void HaliteListViewCtrl::queue_view_mode()
{
	erase_all_from_list();

	if (queue_view_)
	{
		int ret = EnableGroupView(true);
		if (IsGroupViewEnabled())
		{
			LVGROUP lvg = { sizeof(LVGROUP) };

			lvg.mask = LVGF_HEADER|LVGF_GROUPID ;
			wstring unmanaged = hal::app().res_wstr(HAL_UNMANAGED);
			lvg.pszHeader = (LPWSTR)unmanaged.c_str();
			lvg.iGroupId = HAL_UNMANAGED;

			int grp = InsertGroup(-1, &lvg);

			lvg.mask = LVGF_HEADER|LVGF_GROUPID ;
			wstring managed_seed = hal::app().res_wstr(HAL_MANAGED_SEEDING);
			lvg.pszHeader = (LPWSTR)managed_seed.c_str();
			lvg.iGroupId = HAL_MANAGED_SEEDING;

			grp = InsertGroup(-1, &lvg);

			lvg.mask = LVGF_HEADER|LVGF_GROUPID ;
			wstring managed_down = hal::app().res_wstr(HAL_MANAGED_DOWNLOADING);
			lvg.pszHeader = (LPWSTR)managed_down.c_str();
			lvg.iGroupId = HAL_MANAGED_DOWNLOADING;

			grp = InsertGroup(-1, &lvg);

			lvg.mask = LVGF_HEADER|LVGF_GROUPID ;
			wstring auto_managed = hal::app().res_wstr(HAL_AUTO_MANAGED);
			lvg.pszHeader = (LPWSTR)auto_managed.c_str();
			lvg.iGroupId = HAL_AUTO_MANAGED;

			grp = InsertGroup(-1, &lvg);
		}
	}
	else
	{
		RemoveAllGroups();
		int ret = EnableGroupView(false);
	}
	halite_window_.issueUiUpdate();

	MENUITEMINFO minfo = {sizeof(MENUITEMINFO)};
	
	minfo.fMask = MIIM_STATE;
	minfo.fState = queue_view_ ? MFS_CHECKED : MFS_UNCHECKED;
	
	menu_.SetMenuItemInfo(HAL_LVM_QUEUE_VIEW, false, &minfo);
}
