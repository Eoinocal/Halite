
#include "../stdAfx.hpp"
#include "../HaliteWindow.hpp"
#include "../HaliteListView.hpp"

//#include "../GlobalIni.hpp"
//#include "../ini/Dialog.hpp"

#include "../global/logger.hpp"

#include "Peers.hpp"

AdvPeerDialog::AdvPeerDialog(ui_signal& ui_sig, ListViewManager& single_sel) :
	ui_(ui_sig),
	selection_manager_(single_sel)
{}

void AdvPeerDialog::selectionChanged(const string& torrent_name)
{	

//	DoDataExchange(false);	
	ui_.update();
}

void AdvPeerDialog::updateDialog()
{}

LRESULT AdvPeerDialog::onInitDialog(HWND, LPARAM)
{
	ui_.attach(bind(&AdvPeerDialog::updateDialog, this));
	selection_manager_.attach(bind(&AdvPeerDialog::selectionChanged, this, _1));
	
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
	
	m_list.Attach(GetDlgItem(IDC_PEERLIST));	
	
//	DoDataExchange(false);	
	return 0;
}

void AdvPeerDialog::onClose()
{
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}


