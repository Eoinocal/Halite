
#pragma once

#include "stdAfx.hpp"
#include "DdxEx.hpp"

#include "HaliteTabCtrl.hpp"
#include "advtabs/ThemeTestDialog.hpp"
#include "advtabs/Torrent.hpp"
#include "advtabs/Peers.hpp"
#include "advtabs/Tracker.hpp"
#include "advtabs/Debug.hpp"

#include "HaliteListManager.hpp"

class HaliteListViewCtrl;
template <class TBase>
class CHaliteListViewCtrl;

typedef CHaliteListViewCtrl<HaliteListViewCtrl>::selection_manage_class ListViewManager;

class HaliteWindow;

class AdvHaliteDialog :
	public CDialogImpl<AdvHaliteDialog>,
	public CDialogResize<AdvHaliteDialog>
{
protected:
	typedef AdvHaliteDialog thisClass;
	typedef CDialogImpl<AdvHaliteDialog> baseClass;
	typedef CDialogResize<AdvHaliteDialog> resizeClass;
public:
	enum { IDD = IDD_ADVHALITEDLG };

	AdvHaliteDialog(HaliteWindow& halWindow, ui_signal& ui_sig, ListViewManager& single_sel) :
		m_torrent(halWindow, ui_sig, single_sel),
		m_peers(halWindow, ui_sig, single_sel),
		m_tracker(halWindow, ui_sig, single_sel),
		m_debug(halWindow, ui_sig, single_sel)
	{}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)
		MSG_WM_SIZE(OnSize)

		if(uMsg == WM_FORWARDMSG)
			if(PreTranslateMessage((LPMSG)lParam)) return TRUE;

		CHAIN_MSG_MAP(resizeClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(IDC_TAB, (DLSZ_SIZE_X | DLSZ_SIZE_Y))
		DLGRESIZE_CONTROL(IDC_ADVDLG_VERSION, (DLSZ_MOVE_X))

	END_DLGRESIZE_MAP()

	LRESULT onInitDialog(HWND, LPARAM);
	void OnSize(UINT, CSize);
	void onClose();

protected:
	void InitializeControls(void);
	void InitializeValues(void);

	CHalTabCtrl m_tabCtrl;
	boost::scoped_ptr<ThemeTestDialog> mp_dlg;
	boost::scoped_ptr<AdvTorrentDialog> mp_dlg2;

	AdvTorrentDialog m_torrent;
	AdvPeerDialog m_peers;
	AdvTrackerDialog m_tracker;
	AdvDebugDialog m_debug;

	HaliteWindow* mainHaliteWindow;
};
