
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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
		size_t length = lang_list_.GetTextLen(lang_list_.GetCurSel());
		boost::scoped_array<wchar_t> buffer(new wchar_t[length+1]);
		
		lang_list_.GetText(lang_list_.GetCurSel(), buffer.get());
		std::wstring language(buffer.get(), length);
		
		if (language == L"English")
		{
			halite().dll_ = L"";
			hal::app().revert_res();
		}
		else
		{
			halite().dll_ = lang_map_[language].external_file_string();
			hal::app().set_res_dll(lang_map_[language].external_file_string());
		}
		
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
		COMMAND_ID_HANDLER_EX(IDC_BC_PORTCHECK, onPortCheck)
		COMMAND_ID_HANDLER_EX(IDC_BC_HALFCONN_CHECK, onHalfCheck)
		COMMAND_ID_HANDLER_EX(IDC_BC_DHT, onDHTCheck)
        CHAIN_MSG_MAP(CPropertyPageImpl<BitTorrentOptions>)
    END_MSG_MAP()

    BEGIN_DDX_MAP(BitTorrentOptions)
        DDX_CHECK(IDC_BC_PORTCHECK, hal::config().portRange)
        DDX_INT(IDC_BC_PORTFROM, hal::config().portFrom)
        DDX_INT(IDC_BC_PORTTO, hal::config().portTo)		
        DDX_CHECK(IDC_BC_HALFCONN_CHECK, hal::config().halfConn)
        DDX_EX_INT_POSITIVE(IDC_BC_HALFCONN_NUM, hal::config().halfConnLimit)
        DDX_CHECK(IDC_BC_DHT, hal::config().enableDHT)
        DDX_INT(IDC_BC_DHTPORT, hal::config().dhtServicePort)
    END_DDX_MAP()

    BOOL OnInitDialog (HWND hwndFocus, LPARAM lParam)
	{	
	//	helpLink.SubclassWindow(GetDlgItem(IDC_BC_HELP_LINK));
	//	helpLink.SetHyperLink(L"http://www.binarynotions.com/");
		
	//	whyHalfConn.SubclassWindow(GetDlgItem(IDC_BC_CON_WHY));
	//	whyHalfConn.SetHyperLink(L"http://www.binarynotions.com/");
		
		BOOL retval = DoDataExchange(false);

		onHalfCheck(0, 0, GetDlgItem(IDC_BC_HALFCONN_CHECK));
		onPortCheck(0, 0, GetDlgItem(IDC_BC_PORTCHECK));
		onDHTCheck(0, 0, GetDlgItem(IDC_BC_DHT));

		return retval;
	}

    int OnApply()
	{
		return DoDataExchange(true);
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

	void onHalfCheck(UINT, int, HWND hWnd)
	{
		LRESULT result = ::SendMessage(hWnd, BM_GETCHECK, 0, 0);

		if (result == BST_CHECKED)
		{
			::EnableWindow(GetDlgItem(IDC_BC_HALFCONN_NUM), true);
		}
		else
		{
			::EnableWindow(GetDlgItem(IDC_BC_HALFCONN_NUM), false);
			SetDlgItemText(IDC_BC_HALFCONN_NUM, L"∞");
		}
	}
private:
	CHyperLink helpLink;
	CHyperLink whyHalfConn;
};

class ProxyOptions :
    public CPropertyPageImpl<ProxyOptions>,
    public CWinDataExchangeEx<ProxyOptions>
{
public:
    enum { IDD = IDD_CONFIGPROXY };

	ProxyOptions()
	{}

	~ProxyOptions()
	{}

    BEGIN_MSG_MAP_EX(ProxyOptions)
        MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BC_PROXYCHECK, onProxyCheck)
        CHAIN_MSG_MAP(CPropertyPageImpl<ProxyOptions>)
    END_MSG_MAP()

    BEGIN_DDX_MAP(ProxyOptions)
	
    END_DDX_MAP()

    BOOL OnInitDialog (HWND hwndFocus, LPARAM lParam)
	{
		BOOL retval =  DoDataExchange(false);

		onProxyCheck(0, 0, GetDlgItem(IDC_BC_PROXYCHECK));

		return retval;
	}

    int OnApply()
	{
		return DoDataExchange(true);
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
};

class SecurityOptions :
    public CPropertyPageImpl<SecurityOptions>,
    public CWinDataExchangeEx<SecurityOptions>
{
	typedef SecurityOptions thisClass;
public:
    enum { IDD = IDD_CONFIGSECURITY };

	SecurityOptions()
	{}

	~SecurityOptions()
	{}

    BEGIN_MSG_MAP_EX(thisClass)
        MSG_WM_INITDIALOG(OnInitDialog)
		
		COMMAND_ID_HANDLER_EX(IDC_BC_FILTERLOAD, onFilterImport)
		COMMAND_ID_HANDLER_EX(IDC_BC_FILTERCLEAR, onFilterClear)
		COMMAND_ID_HANDLER_EX(IDC_BC_FILTERCHECK, onFilterCheck)
		
		COMMAND_ID_HANDLER_EX(IDC_SC_ENABLE_PE, onPeCheck)
		
        CHAIN_MSG_MAP(CPropertyPageImpl<thisClass>)
    END_MSG_MAP()

    BEGIN_DDX_MAP(thisClass)
        DDX_CHECK(IDC_BC_FILTERCHECK, hal::config().enableIPFilter)
        DDX_CHECK(IDC_SC_ENABLE_PE, hal::config().enablePe)
        DDX_RADIO(IDC_SC_PE_ENC_PLAIN, hal::config().peEncLevel)
        DDX_CHECK(IDC_SC_PE_ENC_RC4_PERFER, hal::config().pePerferRc4)
        DDX_RADIO(IDC_SC_PE_CP_IN_FORCED, hal::config().peConInPolicy)
        DDX_RADIO(IDC_SC_PE_CP_OUT_FORCED, hal::config().peConOutPolicy)
    END_DDX_MAP()

    BOOL OnInitDialog (HWND hwndFocus, LPARAM lParam)
	{
	//	helpLink.SubclassWindow(GetDlgItem(IDC_SC_HELP_LINK));
	//	helpLink.SetHyperLink(L"http://www.binarynotions.com/");
		
		BOOL retval =  DoDataExchange(false);

		onFilterCheck(0, 0, GetDlgItem(IDC_BC_FILTERCHECK));
		onPeCheck(0, 0, GetDlgItem(IDC_SC_ENABLE_PE));

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

	void onPeCheck(UINT, int, HWND hWnd)
	{
		LRESULT result = ::SendMessage(hWnd, BM_GETCHECK, 0, 0);

		if (result == BST_CHECKED)
		{
			::EnableWindow(GetDlgItem(IDC_SC_PE_ENC_BOTH), true);
			::EnableWindow(GetDlgItem(IDC_SC_PE_ENC_RC4), true);
			::EnableWindow(GetDlgItem(IDC_SC_PE_ENC_PLAIN), true);
			::EnableWindow(GetDlgItem(IDC_SC_PE_ENC_RC4_PERFER), true);
			::EnableWindow(GetDlgItem(IDC_SC_PE_CP_IN_FORCED), true);
			::EnableWindow(GetDlgItem(IDC_SC_PE_CP_IN_DISABLED), true);
			::EnableWindow(GetDlgItem(IDC_SC_PE_CP_IN_ENABLED), true);
			::EnableWindow(GetDlgItem(IDC_SC_PE_CP_OUT_DISABLED), true);
			::EnableWindow(GetDlgItem(IDC_SC_PE_CP_OUT_ENABLED), true);
			::EnableWindow(GetDlgItem(IDC_SC_PE_CP_OUT_FORCED), true);
		}
		else
		{
			::EnableWindow(GetDlgItem(IDC_SC_PE_ENC_BOTH), false);
			::EnableWindow(GetDlgItem(IDC_SC_PE_ENC_RC4), false);
			::EnableWindow(GetDlgItem(IDC_SC_PE_ENC_PLAIN), false);
			::EnableWindow(GetDlgItem(IDC_SC_PE_ENC_RC4_PERFER), false);
			::EnableWindow(GetDlgItem(IDC_SC_PE_CP_IN_FORCED), false);
			::EnableWindow(GetDlgItem(IDC_SC_PE_CP_IN_DISABLED), false);
			::EnableWindow(GetDlgItem(IDC_SC_PE_CP_IN_ENABLED), false);
			::EnableWindow(GetDlgItem(IDC_SC_PE_CP_OUT_DISABLED), false);
			::EnableWindow(GetDlgItem(IDC_SC_PE_CP_OUT_ENABLED), false);
			::EnableWindow(GetDlgItem(IDC_SC_PE_CP_OUT_FORCED), false);
		}
	}

	void onFilterClear(UINT, int, HWND hWnd)
	{
		hal::bittorrent().clearIpFilter();
	}

	void onFilterImport(UINT, int, HWND hWnd);

private:
	CHyperLink helpLink;
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

    BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam)
	{
		BOOL retval =  DoDataExchange(false);

		return retval;
	}

	void onFolderBrowse(UINT, int, HWND hWnd)
	{
		hal::bittorrent().clearIpFilter();
		
		wstring folderSelect = hal::app().res_wstr(HAL_FOLDER_SELECT);

		CFolderDialog fldDlg (NULL, folderSelect.c_str(),
			BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);

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
	
    BEGIN_MSG_MAP_EX(RemoteOptions)
		MSG_WM_INITDIALOG(OnInitDialog)
    END_MSG_MAP()

    BOOL OnInitDialog (HWND hwndFocus, LPARAM lParam)
	{
		richEdit_.Attach(GetDlgItem(IDC_RICHEDITABOUT));
		
		std::pair<void*, size_t> res = hal::app().find_lock_res(IDR_ABOUT_TEXT, HALITE_RTF_RES_TYPE);
		
		string rtf(static_cast<char*>(res.first), res.second);		
		wstring wrtf = hal::safe_from_utf8(rtf);
		
		richEdit_.SendMessage(WM_SETTEXT, 0, (LPARAM)wrtf.c_str());		
		richEdit_.SetSelNone();
		
		return true;
	}
	
private:
	CRichEditCtrl richEdit_;
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
		AddPage(securityOptions);
		AddPage(torrentsOptions);
	//	AddPage(remoteControlOptions);
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
	SecurityOptions securityOptions;
	TorrentsOptions torrentsOptions;
	RemoteOptions remoteControlOptions;
	AboutOptions aboutOptions;
};
