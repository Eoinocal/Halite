
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define ID_WEB_SEEDS_BEGIN				16101
#define HAL_WEB_SEED_MENU	 			16100
#define HAL_WEB_SEEDS_LISTVIEW			ID_WEB_SEEDS_BEGIN + 1
#define HAL_SEED_URL_TEXT	 			ID_WEB_SEEDS_BEGIN + 2
#define HAL_ADD_SEED_AS					ID_WEB_SEEDS_BEGIN + 3
#define HAL_HTTP_SEED		 			ID_WEB_SEEDS_BEGIN + 4
#define HAL_URL_SEED					ID_WEB_SEEDS_BEGIN + 5
#define HAL_SEED_URL					ID_WEB_SEEDS_BEGIN + 6
#define HAL_WEB_SEED_TAB	 			ID_WEB_SEEDS_BEGIN + 7
#define HAL_WEB_SEED_URL_HDR			ID_WEB_SEEDS_BEGIN + 8
#define HAL_WEB_SEED_TYPE_HDR			ID_WEB_SEEDS_BEGIN + 9
#define HAL_WEB_SEED_TYPE_URL			ID_WEB_SEEDS_BEGIN + 10
#define HAL_WEB_SEED_TYPE_HTTP			ID_WEB_SEEDS_BEGIN + 11	
#define HAL_WEB_SEED_LISTVIEW_COLUMNS 	ID_WEB_SEEDS_BEGIN + 12
#define HAL_WEB_SEED_DELETE	 			ID_WEB_SEEDS_BEGIN + 13

#ifndef RC_INVOKED

#pragma once

#include "stdAfx.hpp"
#include "Halite.hpp"

#include "DdxEx.hpp"
#include "EditHilight.hpp"
#include "global/string_conv.hpp"

#include "../HaliteTabPage.hpp"
#include "../HaliteListManager.hpp"
#include "../HaliteDialogBase.hpp"
#include "../halIni.hpp"

#include "../HaliteSortListViewCtrl.hpp"

class WebSeedListViewCtrl :
	public CHaliteSortListViewCtrl<WebSeedListViewCtrl>,
	public hal::IniBase<WebSeedListViewCtrl>,
	private boost::noncopyable
{
	typedef WebSeedListViewCtrl this_class_t;
	typedef CHaliteSortListViewCtrl<this_class_t> list_class_t;
	typedef hal::IniBase<this_class_t> ini_class_t;

	friend class list_class_t;
	
public:
	enum { 
		LISTVIEW_ID_MENU = HAL_WEB_SEED_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_WEB_SEED_LISTVIEW_COLUMNS,
		LISTVIEW_ID_COLUMNWIDTHS = HAL_TRACKER_LISTVIEW_DEFAULTS
	};
	
	WebSeedListViewCtrl(boost::filesystem::wpath location, std::wstring name) :
		ini_class_t(location, name)
	{}

	BEGIN_MSG_MAP_EX(WebSeedListViewCtrl)
		try
		{
		MSG_WM_DESTROY(OnDestroy)
		COMMAND_ID_HANDLER(HAL_WEB_SEED_DELETE, OnDelete)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in WebSeedListViewCtrl MSG_MAP")

		CHAIN_MSG_MAP(list_class_t)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void uiUpdate(const hal::torrent_details_ptr pT);
	void enterNewTracker();
	void saveSettings();
	LRESULT OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		using boost::serialization::make_nvp;
		switch (version)
		{
		case 2:			
			ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<list_class_t>(*this));
		
		break;

		case 1:
		default:
			assert(false);
		}
	}
	
	template<typename F>
	void attachDeletedConnection(F&& fn) { seedDeleted_.connect(fn); }

private:
	void OnAttach();
	void OnDestroy();

	boost::signals2::signal<void (hal::web_seed_detail)> seedDeleted_;
};

typedef WebSeedListViewCtrl::SelectionManager WebSeedListViewManager;

class AdvWebSeedsDialog :
	public CHalTabPageImpl<AdvWebSeedsDialog>,
	public ATL::CAutoSizeWindow<AdvWebSeedsDialog, false>,
	public CHaliteDialogBase<AdvWebSeedsDialog>,
	public WTLx::WinDataExchangeEx<AdvWebSeedsDialog>
{
protected:
	typedef AdvWebSeedsDialog this_class_t;
	typedef CHalTabPageImpl<this_class_t> base_class_t;
	typedef ATL::CAutoSizeWindow<this_class_t, false> autosizeClass;
	typedef CHaliteDialogBase<this_class_t> dlg_base_class_t;

public:
	enum { IDD = HAL_ADVWEBPEERS };

	AdvWebSeedsDialog(HaliteWindow& HalWindow) :
		dlg_base_class_t(HalWindow),
		list_(L"listviews/web_seeds", L"web_seed_listview")
	{}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(this_class_t)
		try
		{
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)

		COMMAND_ID_HANDLER_EX(HAL_HTTP_SEED, onAddHttp)
		COMMAND_ID_HANDLER_EX(HAL_URL_SEED, onAddUrl)

		}
		HAL_ALL_EXCEPTION_CATCH(L"in AdvWebSeedsDialog MSG_MAP")

		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;

		CHAIN_MSG_MAP(dlg_base_class_t)
		CHAIN_MSG_MAP(autosizeClass)
		CHAIN_MSG_MAP(base_class_t)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DDX_MAP(this_class_t)
		DDX_WTLx_WSTRING(urlEdit_, url_)
    END_DDX_MAP()
	
	static CWindowMapStruct* GetWindowMap();

	LRESULT onInitDialog(HWND, LPARAM);
	void onClose();

	LRESULT OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl);

	void onAddHttp(UINT, int, HWND);
	void onAddUrl(UINT, int, HWND);

	void uiUpdate(const hal::torrent_details_manager& tD);
	void focusChanged(const hal::torrent_details_ptr pT);

protected:
	string current_torrent_name_;
	WebSeedListViewCtrl list_;

	wstring url_;
	WTLx::EditHilight urlEdit_;
};

BOOST_CLASS_VERSION(WebSeedListViewCtrl, 2)

#endif // RC_INVOKED
