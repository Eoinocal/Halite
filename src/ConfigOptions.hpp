
#pragma once
 
#include "stdAfx.hpp"
#include "DdxEx.hpp"

#include "ini/Window.hpp"
#include "ini/General.hpp"
#include "ini/Remote.hpp"
#include "GlobalIni.hpp"
#include "ini/BitTConfig.hpp"

class GeneralOptions :
    public CPropertyPageImpl<GeneralOptions>,
    public CWinDataExchange<GeneralOptions>
{
public:
    enum { IDD = IDD_CONFIGGENERAL };

	GeneralOptions() 
	{}	
	
	~GeneralOptions()
	{}
 
    BEGIN_MSG_MAP(GeneralOptions)
		MSG_WM_INITDIALOG(OnInitDialog)
     	CHAIN_MSG_MAP(CPropertyPageImpl<GeneralOptions>)
    END_MSG_MAP()
 
    BEGIN_DDX_MAP(GeneralOptions)
    	DDX_CHECK(IDC_GENERAL_ONEINST, INI().generalConfig().oneInst)
    	DDX_CHECK(IDC_GENERAL_TRAY, INI().windowConfig().use_tray)
//    	DDX_CHECK(IDC_GENERAL_ADVGUI, INI().remoteConfig().isEnabled)
    END_DDX_MAP()
 
    BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam)
	{
		return DoDataExchange(false);
	}	
	
    int OnApply()
	{
		return DoDataExchange(true);
	}
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
 
    BEGIN_MSG_MAP(BitTorrentOptions)
        MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BC_FILTERBTN, onFilterBtn)
		COMMAND_ID_HANDLER_EX(IDC_BC_FILTERCHECK, onFilterCheck)	
        CHAIN_MSG_MAP(CPropertyPageImpl<BitTorrentOptions>)
    END_MSG_MAP()
 
    BEGIN_DDX_MAP(BitTorrentOptions)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_BC_MAXCONN, INI().bitTConfig().maxConnections, 2, true)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_BC_MAXUP, INI().bitTConfig().maxUploads, 2, true)
        DDX_EX_FLOAT_POSITIVE(IDC_BC_DOWNRATE, INI().bitTConfig().downRate)
        DDX_EX_FLOAT_POSITIVE(IDC_BC_UPRATE, INI().bitTConfig().upRate)
        DDX_INT(IDC_BC_PORTFROM, INI().bitTConfig().portFrom)
        DDX_INT(IDC_BC_PORTTO, INI().bitTConfig().portTo)
        DDX_CHECK(IDC_BC_DHT, INI().bitTConfig().enableDHT)
        DDX_CHECK(IDC_BC_FILTERCHECK, INI().bitTConfig().enableIPFilter)
		DDX_EX_STDWSTRING(IDC_BC_FILTEREDIT, INI().bitTConfig().ipFilterFile);
    END_DDX_MAP()
 
    BOOL OnInitDialog (HWND hwndFocus, LPARAM lParam)
	{
		BOOL retval =  DoDataExchange(false);
		
		LRESULT result = ::SendMessage(GetDlgItem(IDC_BC_FILTERCHECK), BM_GETCHECK, 0, 0);
		
		if (result == BST_CHECKED)
		{
			::EnableWindow(GetDlgItem(IDC_BC_FILTERSTATIC), true);
			::EnableWindow(GetDlgItem(IDC_BC_FILTEREDIT), true);
			::EnableWindow(GetDlgItem(IDC_BC_FILTERBTN), true);
		}
		else
		{
			::EnableWindow(GetDlgItem(IDC_BC_FILTERSTATIC), false);
			::EnableWindow(GetDlgItem(IDC_BC_FILTEREDIT), false);
			::EnableWindow(GetDlgItem(IDC_BC_FILTERBTN), false);		
		}
		
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
			::EnableWindow(GetDlgItem(IDC_BC_FILTERSTATIC), true);
			::EnableWindow(GetDlgItem(IDC_BC_FILTEREDIT), true);
			::EnableWindow(GetDlgItem(IDC_BC_FILTERBTN), true);
		}
		else
		{
			::EnableWindow(GetDlgItem(IDC_BC_FILTERSTATIC), false);
			::EnableWindow(GetDlgItem(IDC_BC_FILTEREDIT), false);
			::EnableWindow(GetDlgItem(IDC_BC_FILTERBTN), false);		
		}
	}	
	
	void onFilterBtn(UINT, int, HWND hWnd)
	{
		CSSFileDialog dlgOpen(TRUE, NULL, NULL, OFN_HIDEREADONLY, L"eMule ipfilter.dat. (*.dat)|*.dat|", m_hWnd);
	
		if (dlgOpen.DoModal() == IDOK) 
			SetDlgItemText(IDC_BC_FILTEREDIT, dlgOpen.m_ofn.lpstrFile);	
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
 
    BEGIN_MSG_MAP(RemoteOptions)
		MSG_WM_INITDIALOG(OnInitDialog)
     	CHAIN_MSG_MAP(CPropertyPageImpl<RemoteOptions>)
    END_MSG_MAP()
 
    BEGIN_DDX_MAP(RemoteOptions)
    	DDX_CHECK(IDC_REMOTECTRL, INI().remoteConfig().isEnabled)
    	DDX_INT(IDC_REMOTEPORT, INI().remoteConfig().port)
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
    ConfigOptionsProp(LPCTSTR title = 
          (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
        : CPropertySheet(title, uStartPage, hWndParent), m_bCentered(false)
    {
		AddPage(generalOptions);
		AddPage(bitTorrentOptions);
		AddPage(remoteControlOptions);
		AddPage(aboutOptions);
    }
    
    BEGIN_MSG_MAP(ConfigOptionsProp)
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
	RemoteOptions remoteControlOptions;
	AboutOptions aboutOptions;
};
