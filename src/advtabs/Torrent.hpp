
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
	
	TRANSPARENT_LIST(thisClass, IDC_GROUP_TORRENT, IDC_GROUP_TRACKER)

#define TORRENT_LIMITS_LAYOUT \
	WMB_HEAD(WMB_COL(_exp|20), WMB_COL(_exp|30), WMB_COL(_exp|20), WMB_COL(_exp|30)), \
		WMB_ROW(10,	IDC_TL,	_r, _r, _r), \
		WMB_ROW(11,	IDC_TLD, IDC_EDITTLD, IDC_TLU, IDC_EDITTLU), \
		WMB_ROW(10,	IDC_NC,	_r, _r, _r), \
		WMB_ROW(11,	IDC_NCD, IDC_EDITNCD, IDC_NCU, IDC_EDITNCU), \
		WMB_ROW(11,	IDC_RATIOESTATIC, _r, _r, IDC_EDITRATIO), \
	WMB_END()

#define TORRENT_STATUS_LAYOUT \
	WMB_HEAD(WMB_COL(50), WMB_COLNOMIN(_exp|150), WMB_COL(_eq|0), WMB_COL(_exp|100)), \
		WMB_ROW(10,	IDC_NAME_STATUS_LABEL, IDC_NAME_STATUS, _r, _r), \
		WMB_ROW(10,	IDC_PEERS_LABEL, IDC_PEERS, IDC_SEEDS_LABEL, IDC_SEEDS), \
		WMB_ROW(10,	IDC_TRANSFERED_LABEL, IDC_TRANSFERED, IDC_OVERHEAD_LABEL, IDC_OVERHEAD), \
		WMB_ROW(10,	IDC_REMAINING_LABEL, IDC_REMAINING, IDC_ETA_LABEL, IDC_ETA), \
		WMB_ROW(10,	IDC_RATE_LABEL, IDC_RATE, IDC_RATIO_LABEL, IDC_RATIO), \
	WMB_END()
		
#define TORRENT_REANNOUNCE_LAYOUT \
	WMB_HEAD(WMB_COL(50), WMB_COLNOMIN(_exp)), \
		WMB_ROW(10,	IDC_UPDATESTAT, IDC_UPDATE), \
	WMB_END()	

	BEGIN_WINDOW_MAP(thisClass, 6, 6, 3, 3)
		WMB_HEAD(WMB_COL(_gap), WMB_COL(_exp), WMB_COL(120), WMB_COL(_gap)), 
			WMB_ROW(_gap|3,	IDC_GROUP_TORRENT, _r, _r, _r), 
			WMB_ROW(_auto,	_d, TORRENT_STATUS_LAYOUT, TORRENT_LIMITS_LAYOUT), 
			WMB_ROWMIN(_exp, 8,	_d, TORRENTPROG, _r), 
			WMB_ROW(_gap,	_d), 
			WMB_ROW(_gap|3,	IDC_GROUP_TRACKER, _r, _r, _r), 
			WMB_ROW(_auto,	_d, IDC_TRACKER, TORRENT_REANNOUNCE_LAYOUT), 
			WMB_ROW(_gap,	_d), 
		WMB_END() 		
	END_WINDOW_MAP()	

/*	BEGIN_DLGRESIZE_MAP(thisClass)
//		BEGIN_DLGRESIZE_GROUP()
			DLGRESIZE_CONTROL(IDC_TL, (DLSZ_MOVE_X))
			DLGRESIZE_CONTROL(IDC_NC, (DLSZ_MOVE_X))
			DLGRESIZE_CONTROL(IDC_TLD, (DLSZ_MOVE_X))
			DLGRESIZE_CONTROL(IDC_TLU, (DLSZ_MOVE_X))
			DLGRESIZE_CONTROL(IDC_NCU, (DLSZ_MOVE_X))
			DLGRESIZE_CONTROL(IDC_NCD, (DLSZ_MOVE_X))
			DLGRESIZE_CONTROL(IDC_RATIOESTATIC, (DLSZ_MOVE_X))
			DLGRESIZE_CONTROL(IDC_EDITRATIO, (DLSZ_MOVE_X))
			DLGRESIZE_CONTROL(IDC_EDITNCD, (DLSZ_MOVE_X))
			DLGRESIZE_CONTROL(IDC_EDITTLD, (DLSZ_MOVE_X))
			DLGRESIZE_CONTROL(IDC_EDITTLU, (DLSZ_MOVE_X))
			DLGRESIZE_CONTROL(IDC_EDITNCU, (DLSZ_MOVE_X))
//		END_DLGRESIZE_GROUP()
		
//		DLGRESIZE_CONTROL(IDC_RATIO, (DLSZ_MOVE_X))
//		DLGRESIZE_CONTROL(IDC_RATIOSTAT, (DLSZ_MOVE_X))

		DLGRESIZE_CONTROL(IDC_UPDATESTAT, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_UPDATE, (DLSZ_MOVE_X))


		DLGRESIZE_CONTROL(IDC_GROUP_TORRENT, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(IDC_GROUP_TRACKER, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(TORRENTPROG, (DLSZ_SIZE_X))

		DLGRESIZE_CONTROL(IDC_NAME_STATUS_LABEL, (0))
		DLGRESIZE_CONTROL(IDC_NAME_STATUS, (DLSZ_SIZE_X))

		BEGIN_DLGRESIZE_GROUP()			
			DLGRESIZE_CONTROL(IDC_PEERS_LABEL, (0))
			DLGRESIZE_CONTROL(IDC_TRANSFERED_LABEL, (0))
			DLGRESIZE_CONTROL(IDC_REMAINING_LABEL, (0))
			DLGRESIZE_CONTROL(IDC_RATE_LABEL, (0))
			
			DLGRESIZE_CONTROL(IDC_SEEDS_LABEL, (DLSZ_MOVE_X))
			DLGRESIZE_CONTROL(IDC_OVERHEAD_LABEL, (DLSZ_MOVE_X))
			DLGRESIZE_CONTROL(IDC_ETA_LABEL, (DLSZ_MOVE_X))
			DLGRESIZE_CONTROL(IDC_RATIO_LABEL, (DLSZ_MOVE_X))
		END_DLGRESIZE_GROUP()

		BEGIN_DLGRESIZE_GROUP()			
			DLGRESIZE_CONTROL(IDC_PEERS, (DLSZ_SIZE_X))
			DLGRESIZE_CONTROL(IDC_SEEDS, (DLSZ_MOVE_X|DLSZ_SIZE_X))
		END_DLGRESIZE_GROUP()
		
		BEGIN_DLGRESIZE_GROUP()		
			DLGRESIZE_CONTROL(IDC_TRANSFERED, (DLSZ_SIZE_X))
			DLGRESIZE_CONTROL(IDC_OVERHEAD, (DLSZ_MOVE_X|DLSZ_SIZE_X))
		END_DLGRESIZE_GROUP()
		
		BEGIN_DLGRESIZE_GROUP()		
			DLGRESIZE_CONTROL(IDC_REMAINING, (DLSZ_SIZE_X))
			DLGRESIZE_CONTROL(IDC_ETA, (DLSZ_MOVE_X|DLSZ_SIZE_X))
		END_DLGRESIZE_GROUP()
		
		BEGIN_DLGRESIZE_GROUP()		
			DLGRESIZE_CONTROL(IDC_RATE, (DLSZ_SIZE_X))
			DLGRESIZE_CONTROL(IDC_RATIO, (DLSZ_MOVE_X|DLSZ_SIZE_X))	
		END_DLGRESIZE_GROUP()
		
		DLGRESIZE_CONTROL(IDC_TRACKER, (DLSZ_SIZE_X))
	END_DLGRESIZE_MAP()
*/
	LRESULT onInitDialog(HWND, LPARAM);
	void onClose();

	LRESULT OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl);

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
