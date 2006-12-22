
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
        CHAIN_MSG_MAP(CPropertyPageImpl<BitTorrentOptions>)
    END_MSG_MAP()
 
    BEGIN_DDX_MAP(BitTorrentOptions)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_BC_MAXCONN, INI().bitTConfig().maxConnections, 2, true)
        DDX_EX_INT_POSITIVE_LIMIT(IDC_BC_MAXUP, INI().bitTConfig().maxUploads, 2, true)
        DDX_EX_FLOAT_POSITIVE(IDC_BC_DOWNRATE, INI().bitTConfig().downRate)
        DDX_EX_FLOAT_POSITIVE(IDC_BC_UPRATE, INI().bitTConfig().upRate)
        DDX_INT(IDC_BC_PORTFROM, INI().bitTConfig().portFrom)
        DDX_INT(IDC_BC_PORTTO, INI().bitTConfig().portTo)
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
