
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../stdAfx.hpp"
#include "../global/string_conv.hpp"

#include "../DdxEx.hpp"
#include "../Halite.hpp"

#include "../HaliteTabPage.hpp"
#include "../HaliteEditCtrl.hpp"
#include "../HaliteDialogBase.hpp"

class AdvTorrentDialog :
	public CHalTabPageImpl<AdvTorrentDialog>,
	public CAutoSizeWindow<AdvTorrentDialog, false>,
	public CHaliteDialogBase<AdvTorrentDialog>,
	public CWinDataExchangeEx<AdvTorrentDialog>
{
protected:
	typedef AdvTorrentDialog thisClass;
	typedef CHalTabPageImpl<AdvTorrentDialog> baseClass;
	typedef CAutoSizeWindow<AdvTorrentDialog, false> autosizeClass;
	typedef CHaliteDialogBase<AdvTorrentDialog> dialogBaseClass;

public:
	enum { IDD = IDD_ADVOVERVIEW };

	AdvTorrentDialog(HaliteWindow& HalWindow) :
		dialogBaseClass(HalWindow)
	{}
	
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}
	
	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)
		COMMAND_RANGE_CODE_HANDLER_EX(IDC_EDITTLU, IDC_EDITRATIO, EN_KILLFOCUS, OnEditKillFocus)

		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;

		CHAIN_MSG_MAP(autosizeClass)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	BEGIN_DDX_MAP(thisClass)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_EDITNCD, NoConnDown, 2, true)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_EDITNCU, NoConnUp, 2, true)
        DDX_EX_INT_FLOAT_LIMIT(IDC_EDITTLD, TranLimitDown, 1, false)
        DDX_EX_INT_FLOAT_LIMIT(IDC_EDITTLU, TranLimitUp, 1, false)
        DDX_EX_INT_FLOAT_LIMIT(IDC_EDITRATIO, Ratio, 1, false)
    END_DDX_MAP()
	
	TRANSPARENT_LIST(thisClass, IDC_GROUP_TORRENT, IDC_GROUP_TRACKER, IDC_TL, IDC_NAME_STATUS_LABEL)
	
	static CWindowMapStruct* GetWindowMap();
	
	LRESULT onInitDialog(HWND, LPARAM);
	void onClose();

	LRESULT OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl);
//	void DlgResize_UpdateLayout(int cxWidth, int cyHeight);

	void uiUpdate(const hal::TorrentDetails& tD);
	void uiUpdateMultiple(const hal::TorrentDetail_vec& torrents);
	void uiUpdateSingle(const hal::TorrentDetail_ptr& torrent);
	void uiUpdateNone();
	void focusChanged(const hal::TorrentDetail_ptr pT);

protected:
	CProgressBarCtrl m_prog;

	int NoConnDown, NoConnUp;
	float TranLimitDown, TranLimitUp;
	float Ratio;
	
	string current_torrent_name_;
};
