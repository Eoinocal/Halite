
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
	public CDialogResize<AdvTorrentDialog>,
	public CHaliteDialogBase<AdvTorrentDialog>,
	public CWinDataExchangeEx<AdvTorrentDialog>
{
protected:
	typedef AdvTorrentDialog thisClass;
	typedef CHalTabPageImpl<AdvTorrentDialog> baseClass;
	typedef CDialogResize<AdvTorrentDialog> resizeClass;
	typedef CHaliteDialogBase<AdvTorrentDialog> dialogBaseClass;

public:
	enum { IDD = IDD_ADVOVERVIEW };

	AdvTorrentDialog(HaliteWindow& HalWindow) :
		dialogBaseClass(HalWindow)
	{}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)
		COMMAND_RANGE_CODE_HANDLER_EX(IDC_EDITTLU, IDC_EDITRATIO, EN_KILLFOCUS, OnEditKillFocus)

		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;

		CHAIN_MSG_MAP(resizeClass)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	BEGIN_DDX_MAP(thisClass)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_EDITNCD, NoConnDown, 2, true)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_EDITNCU, NoConnUp, 2, true)
        DDX_EX_FLOAT_POSITIVE(IDC_EDITTLD, TranLimitDown)
        DDX_EX_FLOAT_POSITIVE(IDC_EDITTLU, TranLimitUp)
        DDX_EX_INT_FLOAT_LIMIT(IDC_EDITRATIO, Ratio, 1, true)
    END_DDX_MAP()

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(IDC_TL, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_NC, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_TLD, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_TLU, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_NCU, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_NCD, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_RATIOESTATIC, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_EDITRATIO, (DLSZ_MOVE_X))
//		DLGRESIZE_CONTROL(IDC_RATIO, (DLSZ_MOVE_X))
//		DLGRESIZE_CONTROL(IDC_RATIOSTAT, (DLSZ_MOVE_X))

		DLGRESIZE_CONTROL(IDC_UPDATESTAT, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_UPDATE, (DLSZ_MOVE_X))

		DLGRESIZE_CONTROL(IDC_EDITNCD, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_EDITTLD, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_EDITTLU, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_EDITNCU, (DLSZ_MOVE_X))

		DLGRESIZE_CONTROL(IDC_GROUP_TORRENT, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(IDC_GROUP_TRACKER, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(TORRENTPROG, (DLSZ_SIZE_X))

		DLGRESIZE_CONTROL(IDC_NAME, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(IDC_TRACKER, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(IDC_STATUS, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(IDC_TRANS, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(IDC_TRANS_SES, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(IDC_TRANS_ETA, (DLSZ_SIZE_X))

	END_DLGRESIZE_MAP()

	LRESULT onInitDialog(HWND, LPARAM);
	void onClose();

	LRESULT OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl);

	void selectionChanged(const string& torrent_name);
	void updateDialog();	
	void uiUpdate(const hal::TorrentDetails& tD);
	void uiUpdateMultiple(const hal::TorrentDetail_vec& torrents);
	void uiUpdateSingle(const hal::TorrentDetail_ptr& torrent);
	void uiUpdateNone();

protected:
	CProgressBarCtrl m_prog;

	int NoConnDown, NoConnUp;
	float TranLimitDown, TranLimitUp;
	float Ratio;
};
