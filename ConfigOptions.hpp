#pragma once
 
#include "stdafx.h"
#include "Halite.h"
#include "GlobalIni.h"

using namespace std;
using namespace boost;


class BitTorrentOptions :
    public CPropertyPageImpl<BitTorrentOptions>,
    public CWinDataExchange<BitTorrentOptions>
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
        DDX_INT(IDC_BC_MAXCONN, INI->bitTConfig.maxConnections)
        DDX_INT(IDC_BC_MAXUP, INI->bitTConfig.maxUploads)
        DDX_INT(IDC_BC_PORTFROM, INI->bitTConfig.portFrom)
        DDX_INT(IDC_BC_PORTTO, INI->bitTConfig.portTo)
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
		AddPage(bitTorrentOptions);
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
	
	BitTorrentOptions bitTorrentOptions;
};