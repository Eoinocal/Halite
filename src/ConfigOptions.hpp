
#pragma once

#include "stdAfx.hpp"

#include "Halite.hpp"
#include "halConfig.hpp"
#include "HaliteWindow.hpp"
#include "DdxEx.hpp"
#include "CSSFileDialog.hpp"

class GeneralOptions :
    public CPropertyPageImpl<GeneralOptions>,
    public CWinDataExchange<GeneralOptions>
{
public:
    enum { IDD = IDD_CONFIGGENERAL };

	GeneralOptions(HaliteWindow* haliteWindow) :
		haliteWindow_(haliteWindow)
	{}

	~GeneralOptions()
	{}

    BEGIN_MSG_MAP_EX(GeneralOptions)
		MSG_WM_INITDIALOG(OnInitDialog)
     	CHAIN_MSG_MAP(CPropertyPageImpl<GeneralOptions>)
    END_MSG_MAP()

    BEGIN_DDX_MAP(GeneralOptions)
    	DDX_CHECK(IDC_GENERAL_ONEINST, halite().oneInst)
    	DDX_CHECK(IDC_GENERAL_TRAY, haliteWindow_->use_tray)
    	DDX_CHECK(IDC_GENERAL_ADVGUI, haliteWindow_->advancedUI)
    END_DDX_MAP()

    BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam);

    int OnApply()
	{
		return DoDataExchange(true);
	}

private:
	CListBox lang_list_;
	std::map<wstring, wpath> lang_map_;
	HaliteWindow* haliteWindow_;
};

class BitTorrentOptions :
    public CPropertyPageImpl<BitTorrentOptions>,
    public CWinDataExchangeEx<BitTorrentOptions>
{
public:
    enum { IDD = IDD_CONFIGBIT };

	BitTorrentOptions()
	{}

	~BitTorrentOptions()
	{}

    BEGIN_MSG_MAP_EX(BitTorrentOptions)
        MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BC_FILTERLOAD, onFilterImport)
		COMMAND_ID_HANDLER_EX(IDC_BC_FILTERCLEAR, onFilterClear)
		COMMAND_ID_HANDLER_EX(IDC_BC_PORTCHECK, onPortCheck)
		COMMAND_ID_HANDLER_EX(IDC_BC_FILTERCHECK, onFilterCheck)
		COMMAND_ID_HANDLER_EX(IDC_BC_PROXYCHECK, onProxyCheck)
		COMMAND_ID_HANDLER_EX(IDC_BC_DHT, onDHTCheck)
        CHAIN_MSG_MAP(CPropertyPageImpl<BitTorrentOptions>)
    END_MSG_MAP()

    BEGIN_DDX_MAP(BitTorrentOptions)
        DDX_INT(IDC_BC_PORTFROM, hal::config().portFrom)
        DDX_INT(IDC_BC_PORTTO, hal::config().portTo)
        DDX_CHECK(IDC_BC_DHT, hal::config().enableDHT)
        DDX_INT(IDC_BC_DHTPORT, hal::config().dhtServicePort)
        DDX_CHECK(IDC_BC_FILTERCHECK, hal::config().enableIPFilter)
    END_DDX_MAP()

    BOOL OnInitDialog (HWND hwndFocus, LPARAM lParam)
	{
		BOOL retval =  DoDataExchange(false);

		onFilterCheck(0, 0, GetDlgItem(IDC_BC_FILTERCHECK));
		onProxyCheck(0, 0, GetDlgItem(IDC_BC_PROXYCHECK));
		onPortCheck(0, 0, GetDlgItem(IDC_BC_PORTCHECK));
		onDHTCheck(0, 0, GetDlgItem(IDC_BC_DHT));

		return retval;
	}

    int OnApply()
	{
		return DoDataExchange(true);
	}

	void onFilterCheck(UINT, int, HWND hWnd)
	{
		LRESULT result = ::SendMessage(hWnd, BM_GETCHECK, 0, 0);

		if (result == BST_CHECKED)
		{
			::EnableWindow(GetDlgItem(IDC_BC_FILTERCLEAR), true);
			::EnableWindow(GetDlgItem(IDC_BC_FILTERLOAD), true);
		}
		else
		{
			::EnableWindow(GetDlgItem(IDC_BC_FILTERCLEAR), false);
			::EnableWindow(GetDlgItem(IDC_BC_FILTERLOAD), false);
		}
	}

	void onDHTCheck(UINT, int, HWND hWnd)
	{
		LRESULT result = ::SendMessage(hWnd, BM_GETCHECK, 0, 0);

		if (result == BST_CHECKED)
		{
			::EnableWindow(GetDlgItem(IDC_BC_DHTPORT), true);
			::EnableWindow(GetDlgItem(IDC_BC_DHTPORT_S), true);
		}
		else
		{
			::EnableWindow(GetDlgItem(IDC_BC_DHTPORT), false);
			::EnableWindow(GetDlgItem(IDC_BC_DHTPORT_S), false);
		}
	}

	void onPortCheck(UINT, int, HWND hWnd)
	{
		LRESULT result = ::SendMessage(hWnd, BM_GETCHECK, 0, 0);

		if (result == BST_CHECKED)
		{
			::EnableWindow(GetDlgItem(IDC_BC_PORTTO), true);
		}
		else
		{
			::EnableWindow(GetDlgItem(IDC_BC_PORTTO), false);
			boost::array<wchar_t, MAX_PATH> buffer;
			GetDlgItemText(IDC_BC_PORTFROM, buffer.elems, MAX_PATH);
			SetDlgItemText(IDC_BC_PORTTO, buffer.elems);
		}
	}

	void onProxyCheck(UINT, int, HWND hWnd)
	{
		LRESULT result = ::SendMessage(hWnd, BM_GETCHECK, 0, 0);

		if (result == BST_CHECKED)
		{
			::EnableWindow(GetDlgItem(IDC_BC_PROXYIP), true);
			::EnableWindow(GetDlgItem(IDC_BC_PROXYPORT), true);
			::EnableWindow(GetDlgItem(IDC_BC_PROXYUSER), true);
			::EnableWindow(GetDlgItem(IDC_BC_PROXYPASS), true);
			::EnableWindow(GetDlgItem(IDC_BC_PROXYIP_S), true);
			::EnableWindow(GetDlgItem(IDC_BC_PROXYPORT_S), true);
			::EnableWindow(GetDlgItem(IDC_BC_PROXYUSER_S), true);
			::EnableWindow(GetDlgItem(IDC_BC_PROXYPASS_S), true);
		}
		else
		{
			::EnableWindow(GetDlgItem(IDC_BC_PROXYIP), false);
			::EnableWindow(GetDlgItem(IDC_BC_PROXYPORT), false);
			::EnableWindow(GetDlgItem(IDC_BC_PROXYUSER), false);
			::EnableWindow(GetDlgItem(IDC_BC_PROXYPASS), false);
			::EnableWindow(GetDlgItem(IDC_BC_PROXYIP_S), false);
			::EnableWindow(GetDlgItem(IDC_BC_PROXYPORT_S), false);
			::EnableWindow(GetDlgItem(IDC_BC_PROXYUSER_S), false);
			::EnableWindow(GetDlgItem(IDC_BC_PROXYPASS_S), false);
		}
	}

	void onFilterClear(UINT, int, HWND hWnd)
	{
		hal::bittorrent().clearIpFilter();
	}

	void onFilterImport(UINT, int, HWND hWnd);
};

class TorrentsOptions :
    public CPropertyPageImpl<TorrentsOptions>,
    public CWinDataExchangeEx<TorrentsOptions>
{
public:
    enum { IDD = IDD_CONFIGTORRENT };

    BEGIN_MSG_MAP_EX(TorrentsOptions)
        MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BC_SAVEBROWSE, onFolderBrowse)
        CHAIN_MSG_MAP(CPropertyPageImpl<TorrentsOptions>)
    END_MSG_MAP()

    BEGIN_DDX_MAP(TorrentsOptions)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_BC_MAXCONN, hal::config().maxConnections, 2, true)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_BC_MAXUP, hal::config().maxUploads, 2, true)
        DDX_EX_FLOAT_POSITIVE(IDC_BC_DOWNRATE, hal::config().downRate)
        DDX_EX_FLOAT_POSITIVE(IDC_BC_UPRATE, hal::config().upRate)

        DDX_EX_INT_POSITIVE_LIMIT(IDC_BC_TMAXCONN, hal::config().torrentMaxConnections, 2, true)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_BC_TMAXUP, hal::config().torrentMaxUploads, 2, true)
        DDX_EX_FLOAT_POSITIVE(IDC_BC_TDOWNRATE, hal::config().torrentDownRate)
        DDX_EX_FLOAT_POSITIVE(IDC_BC_TUPRATE, hal::config().torrentUpRate)

		DDX_EX_STDWSTRING(IDC_BC_SAVEFOLDER, hal::config().defaultSaveFolder);
        DDX_CHECK(IDC_BC_PROMPTSAVE, hal::config().savePrompt)
    END_DDX_MAP()

    BOOL OnInitDialog (HWND hwndFocus, LPARAM lParam)
	{
		BOOL retval =  DoDataExchange(false);

		return retval;
	}

	void onFolderBrowse(UINT, int, HWND hWnd)
	{
		hal::bittorrent().clearIpFilter();

		CFolderDialog fldDlg ( NULL, _T("Select a directory to save the downloads to. Select cancel to accept default 'incomming' location."),
				   BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE );

		if (IDOK == fldDlg.DoModal())
			SetDlgItemText(IDC_BC_SAVEFOLDER, fldDlg.m_szFolderPath);
	}

    int OnApply()
	{
		return DoDataExchange(true);
	}
};

class RemoteOptions :
    public CPropertyPageImpl<RemoteOptions>,
    public CWinDataExchange<RemoteOptions>
{
public:
    enum { IDD = IDD_CONFIGREMOTE };

	RemoteOptions()
	{}

	~RemoteOptions()
	{}

    BEGIN_MSG_MAP_EX(RemoteOptions)
		MSG_WM_INITDIALOG(OnInitDialog)
     	CHAIN_MSG_MAP(CPropertyPageImpl<RemoteOptions>)
    END_MSG_MAP()

    BEGIN_DDX_MAP(RemoteOptions)
//    	DDX_CHECK(IDC_REMOTECTRL, INI().remoteConfig().isEnabled)
//    	DDX_INT(IDC_REMOTEPORT, INI().remoteConfig().port)
    END_DDX_MAP()

    BOOL OnInitDialog ( HWND hwndFocus, LPARAM lParam )
	{
		return DoDataExchange(false);
	}

    int OnApply()
	{
		return DoDataExchange(true);
	}
};

class AboutOptions :
    public CPropertyPageImpl<AboutOptions>
{
public:
    enum { IDD = IDD_ABOUT };
};

class ConfigOptionsProp :
	public CPropertySheet
{
private:
    bool m_bCentered;

public:
    ConfigOptionsProp(HaliteWindow* haliteWindow, LPCTSTR title = (LPCTSTR)NULL,
		UINT uStartPage = 0, HWND hWndParent = NULL) :
        CPropertySheet(title, uStartPage, hWndParent),
		m_bCentered(false),
		generalOptions(haliteWindow)
    {
		AddPage(generalOptions);
		AddPage(bitTorrentOptions);
		AddPage(torrentsOptions);
		AddPage(remoteControlOptions);
		AddPage(aboutOptions);
    }

    BEGIN_MSG_MAP_EX(ConfigOptionsProp)
        MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
        CHAIN_MSG_MAP(CPropertySheet)
    END_MSG_MAP()

    LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        if (wParam == TRUE)
            Center();

        bHandled = FALSE;
        return 0;
    }

    void Center(void)
    {
        if (!m_bCentered)
        {
            CenterWindow();
            m_bCentered = true;
        }
    }

	GeneralOptions generalOptions;
	BitTorrentOptions bitTorrentOptions;
	TorrentsOptions torrentsOptions;
	RemoteOptions remoteControlOptions;
	AboutOptions aboutOptions;
};
