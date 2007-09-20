
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdAfx.hpp"
#include "DdxEx.hpp"
#include "HaliteSortListViewCtrl.hpp"
#include "HaliteDialogBase.hpp"
#include "HaliteIni.hpp"

#include "halTorrent.hpp"
#include "halEvent.hpp"

class ui_signal;

class HaliteDialog :
	public CDialogImpl<HaliteDialog>,
	public CDialogResize<HaliteDialog>,
	public CWinDataExchangeEx<HaliteDialog>,
	public CHaliteDialogBase<HaliteDialog>,
	private boost::noncopyable
{	

	typedef HaliteDialog thisClass;
	typedef CDialogImpl<HaliteDialog> baseClass;
	typedef CDialogResize<HaliteDialog> resizeClass;
	typedef CHaliteDialogBase<HaliteDialog> dialogBaseClass;
		
	class DialogListView :
		public CHaliteSortListViewCtrl<DialogListView, const hal::PeerDetail>,
		public CHaliteIni<DialogListView>,
		private boost::noncopyable
	{
	protected:
		typedef HaliteDialog::DialogListView thisClass;
		typedef CHaliteIni<thisClass> iniClass;
		typedef CHaliteSortListViewCtrl<DialogListView, const hal::PeerDetail> listClass;
		typedef const hal::PeerDetail pD;
	
		friend class listClass;
		
		struct ColumnAdapters
		{
		
		typedef listClass::ColumnAdapter ColAdapter_t;
		
		struct SpeedDown : public ColAdapter_t
		{
			virtual int less(pD& l, pD& r)	{ return equalsOrLess(l.speed.first, r.speed.first); }		
			virtual std::wstring print(pD& p) 
			{
				return (wformat(L"%1$.2fkb/s") % (p.speed.first/1024)).str(); 
			}		
		};
		
		struct SpeedUp : public ColAdapter_t
		{
			virtual int less(pD& l, pD& r)	{ return equalsOrLess(l.speed.second, r.speed.second); }		
			virtual std::wstring print(pD& p) 
			{
				return (wformat(L"%1$.2fkb/s") % (p.speed.second/1024)).str(); 
			}		
		};
		
		};
	
	public:	
		enum { 
			LISTVIEW_ID_MENU = 0,
			LISTVIEW_ID_COLUMNNAMES = HAL_DIALOGPEER_LISTVIEW_COS,
			LISTVIEW_ID_COLUMNWIDTHS = HAL_DIALOGPEER_LISTVIEW_COS_DEFAULTS
		};
	
		BEGIN_MSG_MAP_EX(thisClass)
			MSG_WM_DESTROY(OnDestroy)
	
			CHAIN_MSG_MAP(listClass)
			DEFAULT_REFLECTION_HANDLER()
		END_MSG_MAP()
	
		DialogListView() :
			iniClass("listviews/dialog", "DialogPeersList"),
			listClass(true,false,false)
		{					
			std::vector<wstring> names;	
			wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);

			// "Peer;Country;Download;Upload;Type;Client"
			boost::split(names, column_names, boost::is_any_of(L";"));
			
			array<int, 6> widths = {100,20,70,70,70,100};
			array<int, 6> order = {0,1,2,3,4,5};
			array<bool, 6> visible = {true,true,true,true,true,true};
			
			SetDefaults(names, widths, order, visible);
			Load();
		}
		
		void saveSettings()
		{
			GetListViewDetails();
			save();
		}
		
		bool SubclassWindow(HWND hwnd)
		{
			if(!parentClass::SubclassWindow(hwnd))
				return false;
				
			SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_DOUBLEBUFFER);
			SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
			
			ApplyDetails();
			
			SetColumnSortType(2, LVCOLSORT_CUSTOM, new ColumnAdapters::SpeedDown());
			SetColumnSortType(3, LVCOLSORT_CUSTOM, new ColumnAdapters::SpeedUp());
				
			return true;
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
		
		pD CustomItemConversion(LVCompareParam* param, int iSortCol)
		{			
			return peerDetails_[param->dwItemData];
		}		
		
		int CustomItemComparision(pD left, pD right, int iSortCol)
		{
			ColumnAdapter* pCA = getColumnAdapter(iSortCol);
			
			if (pCA)
				return (pCA->less(left, right)) ? 1 : -1;
			else 
				return 0;
		}
		
		void uiUpdate(const hal::TorrentDetails& tD);
		
	private:
		hal::PeerDetails peerDetails_;
	};
	
public:
	enum { IDD = IDD_HALITEDLG };

	HaliteDialog(HaliteWindow& HalWindow);
	BOOL PreTranslateMessage(MSG* pMsg)	{ return this->IsDialogMessage(pMsg); }

	void saveStatus();

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_RANGE_CODE_HANDLER_EX(IDC_EDITTLU, IDC_EDITNCU, EN_KILLFOCUS, OnEditKillFocus)

		if(uMsg == WM_FORWARDMSG)
			if(PreTranslateMessage((LPMSG)lParam)) return TRUE;

		COMMAND_ID_HANDLER_EX(BTNPAUSE, OnPause)
		COMMAND_ID_HANDLER_EX(BTNREANNOUNCE, OnReannounce)
		COMMAND_ID_HANDLER_EX(BTNREMOVE, OnRemove)

	//	MSG_WM_CTLCOLORSTATIC(OnCltColor)

		CHAIN_MSG_MAP(resizeClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DDX_MAP(thisClass)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_EDITNCD, NoConnDown, 2, true)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_EDITNCU, NoConnUp, 2, true)
        DDX_EX_INT_FLOAT_LIMIT(IDC_EDITTLD, TranLimitDown, 1, false)
        DDX_EX_INT_FLOAT_LIMIT(IDC_EDITTLU, TranLimitUp, 1, false)
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
	
	void uiUpdate(const hal::TorrentDetails& allTorrents); 
	void focusChanged(string& torrent_name);
	
protected:
	LRESULT OnInitDialog(HWND, LPARAM);
	void OnClose();

	void OnPause(UINT, int, HWND);
	void OnReannounce(UINT, int, HWND);
	void OnRemove(UINT, int, HWND);

	LRESULT OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl);
	LRESULT OnCltColor(HDC hDC, HWND hWnd);

private:
	CButton m_btn_start;
	DialogListView m_list;
	CContainedWindow m_wndNCD;
	CProgressBarCtrl m_prog;
	
	string current_torrent_name_;

	int NoConnDown, NoConnUp;
	float TranLimitDown, TranLimitUp;
};
