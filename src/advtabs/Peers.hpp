
#pragma once

#include "../stdAfx.hpp"
#include "../global/string_conv.hpp"

#include "../DdxEx.hpp"
#include "../Halite.hpp"

#include "../HaliteTabPage.hpp"
#include "../HaliteDialogBase.hpp"
#include "../HaliteListManager.hpp"

class PeerListViewCtrl :
	public CHaliteListViewCtrl<PeerListViewCtrl>,
	public CHaliteIni<PeerListViewCtrl>,
	private boost::noncopyable
{
	typedef PeerListViewCtrl thisClass;
	typedef CHaliteIni<thisClass> iniClass;
	typedef CHaliteListViewCtrl<thisClass> listClass;

	friend class listClass;
	
public:
	enum { 
		LISTVIEW_ID_MENU = 0,
		LISTVIEW_ID_COLUMNNAMES = HAL_DIALOGPEER_LISTVIEW_ADV	
	};
	
	thisClass() :
		iniClass("listviews/advPeers", "PeerListView")
	{
		array<int, 6> a = {{95, 64, 64, 50, 100, 500}};
		SetDefaults(a);

		load();
	}

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_DESTROY(OnDestroy)

		CHAIN_MSG_MAP(listClass)

		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	void updateListView()
	{}
	
	void saveSettings()
	{
		GetListViewDetails();
		save();
	}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<listClass>(*this));
    }

private:
	void OnAttach()
	{
		SetListViewDetails();
	}
	
	void OnDestroy()
	{
		saveSettings();
	}
};

class AdvPeerDialog :
	public CHalTabPageImpl<AdvPeerDialog>,
	public CHaliteDialogBase<AdvPeerDialog>,
	public CDialogResize<AdvPeerDialog>
{
protected:
	typedef AdvPeerDialog thisClass;
	typedef CHalTabPageImpl<thisClass> baseClass;
	typedef CDialogResize<thisClass> resizeClass;
	typedef CHaliteDialogBase<AdvPeerDialog> dialogBaseClass;

public:
	enum { IDD = IDD_ADVPEER };

	AdvPeerDialog(ui_signal& ui_sig, ListViewManager& single_sel) :
		dialogBaseClass(ui_sig, single_sel)
	{}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(thisClass)
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_CLOSE(onClose)

		if (uMsg == WM_FORWARDMSG)
			if (PreTranslateMessage((LPMSG)lParam)) return TRUE;

		CHAIN_MSG_MAP(resizeClass)
		CHAIN_MSG_MAP(baseClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(IDC_PEERLIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	LRESULT onInitDialog(HWND, LPARAM);
	void onClose();

	LRESULT OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl);

	void selectionChanged(const string& torrent_name);
	void updateDialog();

protected:
	PeerListViewCtrl m_list;
};
