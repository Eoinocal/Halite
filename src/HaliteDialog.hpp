
#pragma once

#include "stdAfx.hpp"
#include "DdxEx.hpp"
#include "HaliteListViewCtrl.hpp"
#include "HaliteDialogBase.hpp"
#include "HaliteIni.hpp"

#include "halTorrent.hpp"
#include "halEvent.hpp"

class ui_signal;

class HaliteListViewCtrl;
typedef selection_manager<CHaliteListViewCtrl<HaliteListViewCtrl> > ListViewManager;

class HaliteDialog :
	public CDialogImpl<HaliteDialog>,
	public CDialogResize<HaliteDialog>,
	public CWinDataExchangeEx<HaliteDialog>,
	public CHaliteDialogBase<HaliteDialog>,
//	public CHaliteIni<HaliteDialog>,
	private boost::noncopyable
{	

	typedef HaliteDialog thisClass;
	typedef CDialogImpl<HaliteDialog> baseClass;
	typedef CDialogResize<HaliteDialog> resizeClass;
//	typedef CHaliteIni<HaliteDialog> iniClass;
	typedef CHaliteDialogBase<HaliteDialog> dialogBaseClass;
	
	
	class DialogListView :
		public CHaliteListViewCtrl<DialogListView>,
		public CHaliteIni<DialogListView>,
		private boost::noncopyable
	{
	protected:
		typedef HaliteDialog::DialogListView thisClass;
		typedef CHaliteIni<thisClass> iniClass;
		typedef CHaliteListViewCtrl<thisClass> listClass;
	
		friend class listClass;
	
	public:	
		enum { 
			LISTVIEW_ID_MENU = 0,
			LISTVIEW_ID_COLUMNNAMES = HAL_DIALOGPEER_LISTVIEW_COS	
		};
	
		BEGIN_MSG_MAP_EX(DialogListView)
			MSG_WM_DESTROY(OnDestroy)
	
			CHAIN_MSG_MAP(CHaliteListViewCtrl<DialogListView>)
			DEFAULT_REFLECTION_HANDLER()
		END_MSG_MAP()
	
		DialogListView() :
			iniClass("listviews/dialog", "DialogPeersList")
		{		
			array<int, 5> a = {{100, 70, 70, 70, 100}};
			SetDefaults(a);			
			load();
		}
		
		void saveSettings()
		{
			GetListViewDetails();
			save();
		}
		
		void OnAttach()
		{
			SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);			
			SetListViewDetails();
		}
		
		void OnDestroy()
		{
			saveSettings();
		}
		
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & boost::serialization::make_nvp("listview", 
				boost::serialization::base_object<listClass>(*this));
		}
	};
	
public:
	enum { IDD = IDD_HALITEDLG };

	HaliteDialog(ui_signal& ui_sig, ListViewManager& single_sel);
	BOOL PreTranslateMessage(MSG* pMsg)	{ return this->IsDialogMessage(pMsg); }

	void saveStatus();

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)
		COMMAND_RANGE_CODE_HANDLER_EX(IDC_EDITTLU, IDC_EDITNCU, EN_KILLFOCUS, OnEditKillFocus)

		if(uMsg == WM_FORWARDMSG)
			if(PreTranslateMessage((LPMSG)lParam)) return TRUE;

		COMMAND_ID_HANDLER_EX(BTNPAUSE, onPause)
		COMMAND_ID_HANDLER_EX(BTNREANNOUNCE, onReannounce)
		COMMAND_ID_HANDLER_EX(BTNREMOVE, onRemove)

	//	MSG_WM_CTLCOLORSTATIC(OnCltColor)

		CHAIN_MSG_MAP(resizeClass)
	END_MSG_MAP()

	BEGIN_DDX_MAP(thisClass)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_EDITNCD, NoConnDown, 2, true)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_EDITNCU, NoConnUp, 2, true)
        DDX_EX_FLOAT_POSITIVE(IDC_EDITTLD, TranLimitDown)
        DDX_EX_FLOAT_POSITIVE(IDC_EDITTLU, TranLimitUp)
    END_DDX_MAP()

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(BTNPAUSE, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(BTNREANNOUNCE, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(BTNREMOVE, (DLSZ_MOVE_X))

		DLGRESIZE_CONTROL(IDC_TL, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_NC, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_TLD, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_TLU, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_NCU, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_NCD, (DLSZ_MOVE_X))

		DLGRESIZE_CONTROL(IDC_EDITNCD, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_EDITTLD, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_EDITTLU, (DLSZ_MOVE_X))
		DLGRESIZE_CONTROL(IDC_EDITNCU, (DLSZ_MOVE_X))

		DLGRESIZE_CONTROL(TORRENTPROG, (DLSZ_SIZE_X))

		DLGRESIZE_CONTROL(IDC_NAME, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(IDC_TRACKER, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(IDC_STATUS, (DLSZ_SIZE_X))
		DLGRESIZE_CONTROL(IDC_COMPLETE, (DLSZ_SIZE_X))

		DLGRESIZE_CONTROL(LISTPEERS, (DLSZ_SIZE_X | DLSZ_SIZE_Y))
		DLGRESIZE_CONTROL(IDC_DETAILS_GROUP, (DLSZ_SIZE_X | DLSZ_SIZE_Y))
	END_DLGRESIZE_MAP()
	
	void selectionChanged(const string& torrent_name);
	void updateDialog();
	
protected:
	LRESULT onInitDialog(HWND, LPARAM);
	void onClose();

	void onPause(UINT, int, HWND);
	void onReannounce(UINT, int, HWND);
	void onRemove(UINT, int, HWND);

	LRESULT OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl);
	LRESULT OnCltColor(HDC hDC, HWND hWnd);

private:
	CButton m_btn_start;
	DialogListView m_list;
	CContainedWindow m_wndNCD;
	CProgressBarCtrl m_prog;

	int NoConnDown, NoConnUp;
	float TranLimitDown, TranLimitUp;
};
