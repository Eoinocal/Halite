
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include <boost/iterator/filter_iterator.hpp>
#pragma warning (push)
#pragma warning (disable : 4244)
#	include <winstl/controls/listview_sequence.hpp>
#pragma warning (pop)

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
	return std::wstring(v.text().c_str());
}

template<>
inline const std::wstring hal::to_wstr_shim<HaliteListViewCtrl::list_value_type>
	(HaliteListViewCtrl::list_value_type& v)
{
	return std::wstring(v.text().c_str());
}

HaliteListViewCtrl::ex_list_value_type::ex_list_value_type(const list_class_t::list_value_type& l) :
	list_class_t::list_value_type(l)
{}

hal::uuid HaliteListViewCtrl::ex_list_value_type::hash() const
{
	std::wstringstream ss(wstring(text(hal::torrent_details::uuid_e-hal::torrent_details::name_e).c_str()));
	hal::uuid id;

	ss >> id;

	return id;
}	

HaliteListViewCtrl::HaliteListViewCtrl(HaliteWindow& HalWindow) :
	halite_window_(HalWindow),
	ini_class_t("listviews/halite", "HaliteListView"),
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

	SafeLoadFromIni();

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

	if (hal::torrent_details_ptr left = hal::bittorrent::Instance().torrentDetails().get(l))
	{
		if (hal::torrent_details_ptr right = hal::bittorrent::Instance().torrentDetails().get(r))
			return hal::hal_details_ptr_compare(left, right, index, ascending);
		else
			return true;	// Huh?
	}
	else
		return false;	// Huh also?  Well these two being opposite means an 
						// invalid entry always comapres as greater. Unless
						// are two invalids!!
	}
	HAL_GENERIC_FN_EXCEPTION_CATCH(L"in HaliteListViewCtrl::sort_list_comparison")

	return false;
}

LRESULT HaliteListViewCtrl::OnGetDispInfo(int, LPNMHDR pnmh, BOOL&)
{	
	if (hal::try_update_lock<list_class_t> lock = hal::try_update_lock<list_class_t>(this)) 
	{	

	NMLVDISPINFO* pdi = (NMLVDISPINFO*)pnmh;
	hal::torrent_details_ptr td = hal::bittorrent::Instance().torrentDetails().get(key_from_index(pdi->item.iItem));

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

		if (hal::bit::torrent t = hal::bittorrent::Instance().get(focused()))
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
	if (hal::try_update_lock<list_class_t> lock = hal::try_update_lock<list_class_t>(this)) 
	{		

	selection_from_listview();
		
	std::set<hal::uuid> torrent_set;
	foreach (hal::torrent_details_ptr t,  tD.torrents())
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
	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bittorrent::Instance().resume_torrent(val.hash());
	}
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bittorrent::Instance().pause_torrent(val.hash());
	}
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bittorrent::Instance().stop_torrent(val.hash());
	}

	return 0;
}

LRESULT HaliteListViewCtrl::OnRemoveFocused(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	hal::bittorrent::Instance().remove_torrent(list_value_type(*is_selected_begin()).hash());
	erase_from_list(*is_selected_begin());

	return 0;
}

LRESULT HaliteListViewCtrl::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::set<hal::uuid> torrent_ids;

	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
		torrent_ids.insert(val.hash());
	
	erase_based_on_set(torrent_ids, false);

	foreach(const hal::uuid& id, torrent_ids)
		hal::bittorrent::Instance().remove_torrent(id);

	return 0;
}

LRESULT HaliteListViewCtrl::OnRecheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	foreach(const list_value_type& v, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		//HAL_DEV_MSG(hal::wform(L"UUid: %1%") % v.text(hal::torrent_details::uuid_e-hal::torrent_details::name_e));

		hal::bittorrent::Instance().recheck_torrent(v.hash());
	}

	return 0;
}

void HaliteListViewCtrl::remove_to_bin(const hal::uuid& id, hal::fs::wpath root, boost::shared_ptr<hal::file_details_vec> files)
{
	std::vector<wchar_t> file_names_buffer;

	foreach(hal::file_details file, *files)
	{
		std::wstring file_location = (hal::wform(L"File %1%\\%2%\\%3%") 
			% root.file_string() % file.branch % file.filename).str();

		wstring full_file = hal::fs::wpath(root / file.branch / file.filename).file_string();

		if (hal::fs::exists(full_file))
		{
			HAL_DEV_MSG(hal::wform(L"File %1%") % full_file);

			std::copy(full_file.begin(), full_file.end(), 
				std::back_inserter(file_names_buffer));
			file_names_buffer.push_back(L'\0');
		}
	}
	file_names_buffer.push_back(L'\0');

	SHFILEOPSTRUCT shf;

	shf.hwnd = *this;
	shf.wFunc = FO_DELETE;
	shf.pFrom = &file_names_buffer[0];
	shf.pTo = 0;
	shf.fFlags = FOF_ALLOWUNDO;

	HAL_DEV_MSG(L"Calling SHFileOperation");
	SHFileOperation(&shf);

	erase_from_list(id);
}

LRESULT HaliteListViewCtrl::OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::set<hal::uuid> torrent_names;

	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
		torrent_names.insert(val.hash());

	foreach(const hal::uuid& id, torrent_names)
		hal::bittorrent::Instance().remove_torrent_wipe_files(id, boost::bind(&HaliteListViewCtrl::remove_to_bin, this, id, _1, _2));

	return 0;
}

LRESULT HaliteListViewCtrl::OnDownloadFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	try
	{

	HAL_DEV_MSG(L"OnDownloadFolder");

	std::set<wpath> unique_paths;

	foreach(const list_value_type& v, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bit::torrent t = hal::bittorrent::Instance().get(v.hash());

		wpath save_dir = t.save_directory;
		if (boost::filesystem::is_directory(save_dir/wpath(t.name)))
			save_dir /= t.name;
		HAL_DEV_MSG(hal::wform(L"Name %1%, Save dir: %2%.") % v.text() % save_dir);

		unique_paths.insert(save_dir);
	}

	SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };

	for (std::set<wpath>::const_iterator i=unique_paths.begin(), e=unique_paths.end();
		i != e; ++i)
	{	
		wstring p = (*i).file_string();

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

	if (hal::bit::torrent t = hal::bittorrent::Instance().get(list_value_type(*is_selected_begin()).hash()))
	{
		wstring saveDirectory = static_cast<wpath>(t.save_directory).native_file_string();
		wstring moveToDirectory = static_cast<wpath>(t.move_to_directory).native_file_string();

		bool useMoveTo = !moveToDirectory.empty();
		bool disableSaveDir = !t.in_session;

		HaliteListViewAdjustDlg addTorrent(hal::app().res_wstr(HAL_ADDT_TITLE), saveDirectory, moveToDirectory, 
			useMoveTo, disableSaveDir);	
		
		if (IDOK == addTorrent.DoModal())
		{
			if (!disableSaveDir) t.save_directory = saveDirectory;

			if (useMoveTo)
				t.move_to_directory = moveToDirectory;
			else
				t.move_to_directory = wstring();
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

	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
		torrent_names.insert(val.hash());

	foreach(const hal::uuid& id, torrent_names)
		if (hal::bit::torrent t = hal::bittorrent::Instance().get(id))
		{
			t.superseeding = !t.superseeding;
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

	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
		torrent_names.insert(val.hash());
	
//	erase_based_on_set(torrent_names, false);

	foreach(const hal::uuid& id, torrent_names)
		hal::bittorrent::Instance().get(id).managed = true;
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

	foreach(const list_value_type& val, std::make_pair(is_selected_begin(), is_selected_end()))
		torrent_names.insert(val.hash());
	
//	erase_based_on_set(torrent_names, false);

	foreach(const hal::uuid& id, torrent_names)
		hal::bittorrent::Instance().get(id).managed = false;
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

	foreach(const list_value_type v, std::make_pair(is_selected_begin(), is_selected_end()))
	{
		hal::bit::torrent t = hal::bittorrent::Instance().get(v.hash());

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

//LRESULT HaliteListViewCtrl::OnDeleteItem(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
//{
//	LPNMLISTVIEW pnmv=(LPNMLISTVIEW)pnmh;
//	T* pItem=(T*)GetItemData(pnmv->iItem);
//	ATLASSERT(pItem);
//	if (pItem)	// Delete attached structure
//		delete pItem;
//	return 0;
//}
