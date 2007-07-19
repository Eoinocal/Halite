
#pragma once

#include "stdAfx.hpp"
#include "global/string_conv.hpp"

#include <boost/array.hpp>
#include <boost/signals.hpp>
#include <boost/serialization/vector.hpp>

#include "HaliteIni.hpp"
#include "HaliteListViewCtrl.hpp"
#include "HaliteListViewCtrl2.hpp"

class HaliteWindow;

class HaliteListViewCtrl :
	public CHaliteListViewCtrl2<HaliteListViewCtrl>,
	public CHaliteIni<HaliteListViewCtrl>,
	private boost::noncopyable
{
protected:
	typedef CHaliteIni<HaliteListViewCtrl> iniClass;
	typedef CHaliteListViewCtrl2<HaliteListViewCtrl> listClass;

	friend class listClass;

public:
	enum { 
		LISTVIEW_ID_MENU = IDR_LISTVIEW_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_LISTVIEW_COLUMNS,
		LISTVIEW_ID_COLUMNWIDTHS = HAL_LISTVIEW_DEFAULTS
	};

	HaliteListViewCtrl(HaliteWindow& HalWindow);

	BEGIN_MSG_MAP_EX(HaliteListViewCtrl)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER(ID__LVM_PAUSE, OnPause)
		COMMAND_ID_HANDLER(ID_LVM_STOP, OnStop)
		COMMAND_ID_HANDLER(ID_LVM_RESUME, OnResume)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_T, OnRemove)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_TD, OnRemoveWipeFiles)

		CHAIN_MSG_MAP(listClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void OnShowWindow(UINT, INT);
	void OnDestroy();
	void saveSettings();
	void uiUpdate(const hal::TorrentDetails& allTorrents); 

	LRESULT OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<listClass>(*this));
    }
				
private:
	void OnAttach();
	void OnDetach();
};

typedef HaliteListViewCtrl::selection_manage_class ListViewManager;
