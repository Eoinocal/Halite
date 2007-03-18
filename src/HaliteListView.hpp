
#pragma once

#include "stdAfx.hpp"
#include "global/string_conv.hpp"

#include <boost/array.hpp>
#include <boost/signals.hpp>

#include "HaliteIni.hpp"
#include "HaliteListViewCtrl.hpp"

class HaliteListViewCtrl :
	public CHaliteListViewCtrl<HaliteListViewCtrl>,
	public CHaliteIni<HaliteListViewCtrl>,
	private boost::noncopyable
{
protected:
	typedef CHaliteIni<HaliteListViewCtrl> iniClass;
	typedef CHaliteListViewCtrl<HaliteListViewCtrl> listClass;

	friend class listClass;

public:
	enum { ID_MENU = IDR_LISTVIEW_MENU };

	HaliteListViewCtrl() :
		iniClass("listviews/halite", "HaliteListView")
	{
		listColumnWidth[0] = 100;
		listColumnWidth[1] = 110;
		listColumnWidth[2] = 60;
		listColumnWidth[3] = 60;
		listColumnWidth[4] = 60;
		listColumnWidth[5] = 42;
		listColumnWidth[6] = 45;
		listColumnWidth[7] = 61;
		listColumnWidth[8] = 45;

		for (int i=0; i<numListColumnWidth; ++i)
			listColumnOrder[i] = i;

		load();
	}

	BEGIN_MSG_MAP_EX(HaliteListViewCtrl)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER(ID__LVM_PAUSE, OnPause)
		COMMAND_ID_HANDLER(ID_LVM_STOP, OnStop)
		COMMAND_ID_HANDLER(ID_LVM_RESUME, OnResume)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_T, OnRemove)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_TD, OnRemoveWipeFiles)

		CHAIN_MSG_MAP(CHaliteListViewCtrl<HaliteListViewCtrl>)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void OnShowWindow(UINT, INT);
	void OnDestroy();
	void updateListView();
	void saveSettings();

	LRESULT OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(listColumnWidth);
        ar & BOOST_SERIALIZATION_NVP(listColumnOrder);
    }

private:
	void OnAttach();
	void OnDetach();

	static const size_t numListColumnWidth = 9;
	int listColumnWidth[numListColumnWidth];
	int listColumnOrder[numListColumnWidth];
};

typedef selection_manager<CHaliteListViewCtrl<HaliteListViewCtrl> > ListViewManager;
