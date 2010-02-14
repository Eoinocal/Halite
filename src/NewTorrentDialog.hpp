
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define ID_NEWTORRENT_BEGIN			14000
#define HAL_NEWTORRENT_SELECT_TEXT		ID_NEWTORRENT_BEGIN + 1
#define HAL_NEWTORRENT_TRACKERS_TEXT	ID_NEWTORRENT_BEGIN + 2
#define HAL_NEWTORRENT_CREATOR_TEXT	ID_NEWTORRENT_BEGIN + 3
#define HAL_NEWTORRENT_CREATOR		ID_NEWTORRENT_BEGIN + 4
#define HAL_NEWTORRENT_COMMENT_TEXT	ID_NEWTORRENT_BEGIN + 5
#define HAL_NEWTORRENT_COMMENT		ID_NEWTORRENT_BEGIN + 6
#define HAL_NEWTORRENT_PRIVATE		ID_NEWTORRENT_BEGIN + 7
#define HAL_NEWT_FILE_BROWSE			ID_NEWTORRENT_BEGIN + 8
#define HAL_NEWT_DIR_BROWSE			ID_NEWTORRENT_BEGIN + 9
#define HAL_NEWT_LISTFILES			ID_NEWTORRENT_BEGIN + 10
#define HAL_FILES_LISTVIEW_ADV			ID_NEWTORRENT_BEGIN + 11
#define HAL_NEW_PANE_DLG			ID_NEWTORRENT_BEGIN + 12
#define HAL_NEWT_TRACKERTIER			ID_NEWTORRENT_BEGIN + 13
#define HAL_NEWTORRENT_PEERS_TEXT		ID_NEWTORRENT_BEGIN + 14
#define HAL_NEWT_LISTPEERS			ID_NEWTORRENT_BEGIN + 15
#define HAL_SAVE_TEXT				ID_NEWTORRENT_BEGIN + 16
#define HAL_NEWT_OUT_BROWSE			ID_NEWTORRENT_BEGIN + 17
#define HAL_NEWT_OUTFILE_TEXT			ID_NEWTORRENT_BEGIN + 18
#define HAL_NEWT_SAVING_TORRENT		ID_NEWTORRENT_BEGIN + 19
#define HAL_NEWT_DIALOG_TITLE			ID_NEWTORRENT_BEGIN + 20
#define HAL_NEWT_FILE_NAME			ID_NEWTORRENT_BEGIN + 21
#define HAL_NEWT_FILE_NAME_EDIT		ID_NEWTORRENT_BEGIN + 22
#define HAL_NEWT_PIECESIZE_TEXT		ID_NEWTORRENT_BEGIN + 23
#define HAL_NEWT_PIECESIZE			ID_NEWTORRENT_BEGIN + 24

#ifndef RC_INVOKED

#include <boost/function.hpp>

#include "halTorrent.hpp"
#include "halIni.hpp"
#include "halEvent.hpp"
#include "DdxEx.hpp"

#include "HaliteSortListViewCtrl.hpp"
#include "NewTorrentTracker.hpp"
#include "NewTorrentPeers.hpp"

typedef boost::function<void (bool)> enable_save;

class FilesListViewCtrl :
	public CHaliteSortListViewCtrl<FilesListViewCtrl>,
	public hal::IniBase<FilesListViewCtrl>,
	private boost::noncopyable
{

	typedef hal::IniBase<FilesListViewCtrl> ini_class_t;
	typedef CHaliteSortListViewCtrl<FilesListViewCtrl> list_class_t;

	friend class list_class_t;
	
public:
	enum { 
		LISTVIEW_ID_MENU = 0,
		LISTVIEW_ID_COLUMNNAMES = HAL_FILES_LISTVIEW_ADV,
		LISTVIEW_ID_COLUMNWIDTHS = 0
	};
	
	FilesListViewCtrl() :
		ini_class_t("listviews/new_files", "NewFilesListView")
	{}

	BEGIN_MSG_MAP_EX(FilesListViewCtrl)
		try
	{
		MSG_WM_DESTROY(OnDestroy)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in FilesListViewCtrl MSG_MAP")

		CHAIN_MSG_MAP(list_class_t)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void uiUpdate(const hal::torrent_details_ptr pT);

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<list_class_t>(*this));
    }

private:
	void OnAttach();
	void OnDestroy();
	void saveSettings();

	boost::signal<void ()> listEdited_;
};

typedef FilesListViewCtrl::SelectionManager FilesListViewManager;

class DetailsSheet :
	public WTL::CPropertyPageImpl<DetailsSheet>,
	public WTLx::WinDataExchangeEx<DetailsSheet>,
	public ATL::CAutoSizeWindow<DetailsSheet, false>
{
protected:
	typedef DetailsSheet this_class_t;
	typedef WTL::CPropertyPageImpl<this_class_t> sheetClass;
	typedef ATL::CAutoSizeWindow<this_class_t, false> autosizeClass;

public:
	DetailsSheet(enable_save enableSave) :
		EnableSave_(enableSave),  
		private_(false)
	{}
	
	enum { IDD = HAL_NEWTORRENT };

	BEGIN_MSG_MAP_EX(this_class_t)
		try
	{
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER_EX(HAL_NEWT_OUT_BROWSE, OnOutBrowse)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in DetailsSheet MSG_MAP")

		CHAIN_MSG_MAP(autosizeClass)
		CHAIN_MSG_MAP(sheetClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DDX_MAP(this_class_t)
		DDX_EX_STDWSTRING(HAL_NEWTORRENT_CREATOR, creator_);
		DDX_EX_STDWSTRING(HAL_NEWTORRENT_COMMENT, comment_);
		DDX_EX_STDWSTRING(HAL_NEWT_FILE, outFile_);
		DDX_INT(HAL_NEWT_PIECESIZE, pieceSize_);
		DDX_CHECK(HAL_NEWTORRENT_PRIVATE, private_)
	END_DDX_MAP()

	static CWindowMapStruct* GetWindowMap();
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
	
	void OnOutBrowse(UINT, int, HWND hWnd);
	LRESULT onInitDialog(HWND, LPARAM);
	void OnDestroy() {};

	wpath OutputFile();
	wstring Creator();
	int PieceSize();
	wstring Comment();
	bool Private();
	
private:
	enable_save EnableSave_;

	wstring creator_;
	wstring comment_;
	int pieceSize_;
	wstring outFile_;
	bool private_;
};

class FilesSheet :
	public WTL::CPropertyPageImpl<FilesSheet>,
	public ATL::CAutoSizeWindow<FilesSheet, false>
{
protected:
	typedef FilesSheet this_class_t;
	typedef WTL::CPropertyPageImpl<this_class_t> sheetClass;
	typedef ATL::CAutoSizeWindow<this_class_t, false> autosizeClass;

public:	
	enum { IDD = HAL_NEWT_FILES };

	BEGIN_MSG_MAP_EX(this_class_t)
		try
		{
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER_EX(HAL_NEWT_FILE_BROWSE, OnFileBrowse)
		COMMAND_ID_HANDLER_EX(HAL_NEWT_DIR_BROWSE, OnDirBrowse)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in FilesSheet MSG_MAP")

		CHAIN_MSG_MAP(autosizeClass)
		CHAIN_MSG_MAP(sheetClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
	
	LRESULT onInitDialog(HWND, LPARAM)
	{	
		filesList_.Attach(GetDlgItem(HAL_NEWT_LISTFILES));	

		return 0;
	}

	static CWindowMapStruct* GetWindowMap();

	void OnDestroy() {}
	void OnFileBrowse(UINT, int, HWND hWnd);
	void OnDirBrowse(UINT, int, HWND hWnd);

	wpath FileFullPath() const;
	hal::file_size_pairs_t FileSizePairs() const;
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

private:
	void UpdateFileList();

	FilesListViewCtrl filesList_;
	
	wpath fileRoot_;
	std::vector<wpath> files_;
};

class TrackerSheet :
	public WTL::CPropertyPageImpl<TrackerSheet>,
	public ATL::CAutoSizeWindow<TrackerSheet, false>
{
protected:
	typedef TrackerSheet this_class_t;
	typedef WTL::CPropertyPageImpl<this_class_t> sheetClass;
	typedef ATL::CAutoSizeWindow<this_class_t, false> autosizeClass;

public:	
	enum { IDD = HAL_NEWT_TRACKERS };

	BEGIN_MSG_MAP_EX(this_class_t)
		try
		{
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in TrackerSheet MSG_MAP")

		CHAIN_MSG_MAP(autosizeClass)
		CHAIN_MSG_MAP(sheetClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	static CWindowMapStruct* GetWindowMap();
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}
	
	LRESULT onInitDialog(HWND, LPARAM)
	{	
		trackerList_.Attach(GetDlgItem(HAL_NEWT_LISTTRACKERS));	

		return 0;
	}

	void OnDestroy() {}

	hal::tracker_details_t Trackers() const;
	
private:
	NewTorrent_TrackerListViewCtrl trackerList_;
};

class PeersSheet :
	public WTL::CPropertyPageImpl<PeersSheet>,
	public ATL::CAutoSizeWindow<PeersSheet, false>
{
protected:
	typedef PeersSheet this_class_t;
	typedef WTL::CPropertyPageImpl<this_class_t> sheetClass;
	typedef ATL::CAutoSizeWindow<this_class_t, false> autosizeClass;

public:	
	enum { IDD = HAL_NEWT_PEERS };

	BEGIN_MSG_MAP_EX(this_class_t)
		try
		{
		MSG_WM_INITDIALOG(onInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in PeersSheet MSG_MAP")

		CHAIN_MSG_MAP(autosizeClass)
		CHAIN_MSG_MAP(sheetClass)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	static CWindowMapStruct* GetWindowMap();
	
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return this->IsDialogMessage(pMsg);
	}

	LRESULT onInitDialog(HWND, LPARAM)
	{	
		peersList_.Attach(GetDlgItem(HAL_NEWT_LISTPEERS));	

		return 0;
	}

	void OnDestroy() {}
	
	hal::dht_node_details_t DhtNodes() const;	
	hal::web_seed_details_t WebSeeds() const;
	
private:
	NewTorrent_PeersListViewCtrl peersList_;
};

class NewTorrentDialog :
	public WTL::CPropertySheet,
	public hal::IniBase<NewTorrentDialog>,
	public WTL::CDialogResize<NewTorrentDialog>
{
	typedef NewTorrentDialog this_class_t;
	typedef hal::IniBase<this_class_t> ini_class_t;
	typedef WTL::CDialogResize<this_class_t> resize_class_t;

public:
    NewTorrentDialog(LPCTSTR title = (LPCTSTR)NULL,
			UINT uStartPage = 0, HWND hWndParent = NULL) :
        CPropertySheet(title, uStartPage, hWndParent),
		ini_class_t("NewTorrentDialog", "Dialog"),
		inited_(false),
		rect_(0,0,0,0),
		fileSheet_(bind(&NewTorrentDialog::EnableSave, this, _1))
	{
		load_from_ini();

		AddPage(fileSheet_);
		AddPage(filesSheet_);
		AddPage(trackerSheet_);
		AddPage(detailsSheet_);		
	}

	BEGIN_MSG_MAP_EX(this_class_t)
		try
		{
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_SIZE(OnSize)
		MSG_WM_CLOSE(OnClose)	
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER(0x1, OnSave)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in CPropertySheet MSG_MAP")

		CHAIN_MSG_MAP(resize_class_t)
		CHAIN_MSG_MAP(CPropertySheet)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(this_class_t)
		DLGRESIZE_CONTROL(0x3020, DLSZ_SIZE_X|DLSZ_SIZE_Y)

		DLGRESIZE_CONTROL(0x1, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(0x2, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(0x3021, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	void OnShowWindow(BOOL bShow, UINT nStatus);

	void OnSize(UINT, WTL::CSize)
	{
		SetMsgHandled(false);

		resizeActiveSheet();
	}
	
	void OnClose()
	{
		SetMsgHandled(false);

		OnDestroy();
	}

	void OnDestroy() 
	{
		SetMsgHandled(false);

		GetWindowRect(rect_);
		save_to_ini();
	}

	void EnableSave(bool enable)
	{
		::EnableWindow(GetDlgItem(0x1), enable);
	}
	
	LRESULT OnSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(rect_);
	}

private:
	bool inited_;
	WTL::CRect rect_;

	void resizeActiveSheet()
	{
		HWND propTabs = GetDlgItem(0x3020);

		RECT rect;
		::GetWindowRect(propTabs, &rect);

		::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rect, 2);
		::SendMessage(propTabs, TCM_ADJUSTRECT, false, (LPARAM)&rect);

		::MoveWindow(GetActivePage(), rect.left, rect.top,
		rect.right-rect.left, rect.bottom-rect.top, true);
	}

	DetailsSheet fileSheet_;
	FilesSheet filesSheet_;
	TrackerSheet trackerSheet_;
	PeersSheet detailsSheet_;
};

#endif // RC_INVOKED
