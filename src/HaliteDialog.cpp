
#include <boost/format.hpp>

#include "stdAfx.hpp"
#include "HaliteDialog.hpp"
#include "HaliteWindow.hpp"

#include "HaliteListViewCtrl.hpp"

#include "GlobalIni.hpp"
#include "ini/Dialog.hpp"

void HaliteDialog::setSelectedTorrent(string torrent)
{
	selectedTorrent = torrent;
	
	pair<float,float> tranLimit = halite::bittorrent().getTorrentSpeed(selectedTorrent);
	pair<int,int> connLimit = halite::bittorrent().getTorrentLimit(selectedTorrent);
	
	NoConnDown = connLimit.first;
	NoConnUp = connLimit.second;
	TranLimitDown = tranLimit.first;
	TranLimitUp = tranLimit.second;
	
	DoDataExchange(false);
	
	if (halite::bittorrent().isTorrentPaused(selectedTorrent))
		SetDlgItemText(BTNPAUSE,L"Resume");
	else		
		SetDlgItemText(BTNPAUSE,L"Pause");
		
	::EnableWindow(GetDlgItem(BTNPAUSE),true);
	::EnableWindow(GetDlgItem(BTNREANNOUNCE),true);
	::EnableWindow(GetDlgItem(BTNREMOVE),true);
	
	::EnableWindow(GetDlgItem(IDC_EDITTLD),true);
	::EnableWindow(GetDlgItem(IDC_EDITTLU),true);
	::EnableWindow(GetDlgItem(IDC_EDITNCD),true);
	::EnableWindow(GetDlgItem(IDC_EDITNCU),true);
	
	m_list.DeleteAllItems();
	
	mainHaliteWindow->updateUI();
}

LRESULT HaliteDialog::onInitDialog(HWND, LPARAM)
{
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
	
{	m_prog.Attach(GetDlgItem(TORRENTPROG));
	m_prog.SetRange(0,100);
}	
{	m_list.Attach(GetDlgItem(LISTPEERS));
	m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	
	CHeaderCtrl hdr = m_list.GetHeader();
	hdr.ModifyStyle(HDS_BUTTONS, 0);
	
	m_list.AddColumn(L"Peer", hdr.GetItemCount());
	m_list.AddColumn(L"Download", hdr.GetItemCount());
	m_list.AddColumn(L"Upload", hdr.GetItemCount());
	m_list.AddColumn(L"Status", hdr.GetItemCount());

	for (size_t i=0; i<4; ++i)
		m_list.SetColumnWidth(i, INI().dialogConfig().peerListColWidth[i]);
}	
	selectedTorrent = "";
	
	NoConnDown = -1;
	NoConnUp = -1;
	TranLimitDown = -1;
	TranLimitUp = -1;	
	
	DoDataExchange(false);
	return 0;
}

void HaliteDialog::saveStatus()
{
	for (size_t i=0; i<4; ++i)
		INI().dialogConfig().peerListColWidth[i] = m_list.GetColumnWidth(i);
}

void HaliteDialog::onClose()
{
	if(::IsWindow(m_hWnd)) {
		::DestroyWindow(m_hWnd);
	}
}

void HaliteDialog::onPause(UINT, int, HWND)
{
	if (halite::bittorrent().isTorrentPaused(selectedTorrent))
	{
		SetDlgItemText(BTNPAUSE,L"Pause");
		halite::bittorrent().resumeTorrent(selectedTorrent);
	}
	else
	{
		SetDlgItemText(BTNPAUSE,L"Resume");
		halite::bittorrent().pauseTorrent(selectedTorrent);
	}
	
	mainHaliteWindow->updateUI();
}

void HaliteDialog::onReannounce(UINT, int, HWND)
{
	halite::bittorrent().reannounceTorrent(selectedTorrent);
}

void HaliteDialog::onRemove(UINT, int, HWND)
{
	halite::bittorrent().removeTorrent(selectedTorrent);

	mainHaliteWindow->mp_list->DeleteItem(mainHaliteWindow->mp_list->GetSelectedIndex());		
		
	mainHaliteWindow->updateUI();
}

LRESULT HaliteDialog::OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl)
{
	DoDataExchange(true);
	
	halite::bittorrent().setTorrentSpeed(selectedTorrent, TranLimitDown, TranLimitUp);
	halite::bittorrent().setTorrentLimit(selectedTorrent, NoConnDown, NoConnUp);
	
	return 0;
}


void HaliteDialog::updateDialog()
{
	wchar_t filenameBuffer[MAX_PATH];
	format tmpStr;
	halite::TorrentDetail_ptr pTD;

	int itemPos = mainHaliteWindow->mp_list->GetSelectionMark();	
	if (itemPos == -1) goto invalid_selection;
	
	mainHaliteWindow->mp_list->GetItemText(itemPos,0,static_cast<LPTSTR>(filenameBuffer),MAX_PATH);
	
	pTD = halite::bittorrent().getTorrentDetails(halite::wcstombs(filenameBuffer));
	if (!pTD) goto invalid_selection;
	
	{
		SetDlgItemText(IDC_NAME, pTD->filename().c_str());
		SetDlgItemText(IDC_TRACKER, pTD->currentTracker().c_str());
		SetDlgItemText(IDC_STATUS, pTD->state().c_str());
		m_prog.SetPos(static_cast<int>(pTD->completion()*100));
			
		SetDlgItemText(IDC_AVAIL,
			(wformat(L"%1$.2f%%") 
				% (pTD->available()*100)
			).str().c_str());		
		
		SetDlgItemText(IDC_COMPLETE,
			(wformat(L"%1$.2fmb of %2$.2fmb") 
				% (static_cast<float>(pTD->totalWantedDone())/(1024*1024))
				% (static_cast<float>(pTD->totalWanted())/(1024*1024))
			).str().c_str());
						
			std::vector<halite::PeerDetail> peerDetails;
	//		halite::getTorrentPeerDetails(filenameBuffer,peerDetails);
			
	/*		if (!peerDetails.empty())
			{
				
				int j = m_list.GetItemCount();
				for(int i=0; i<j; ++i)
				{
					wchar_t ip_address[256];
					m_list.GetItemText(i,0,static_cast<LPTSTR>(ip_address),256);
					
					vector<halite::PeerDetail>::iterator it = 
						find(peerDetails.begin(), peerDetails.end(), ip_address);
					if (it ==  peerDetails.end())
						m_list.DeleteItem(i);
				}
				
				for(size_t i=0; i<peerDetails.size(); ++i)
				{				
					LV_FINDINFO findInfo; 
					findInfo.flags = LVFI_STRING;
					findInfo.psz = const_cast<LPTSTR>(peerDetails[i].ipAddress.c_str());
					
					int itemPos = m_list.FindItem(&findInfo, -1);
					if (itemPos < 0)
						itemPos = m_list.AddItem(0,0,peerDetails[i].ipAddress.c_str(),0);
					
					m_list.SetItemText(itemPos,1,
						(wformat(L"%1$.2fkb/s") 
							% (peerDetails[i].speed.first/1024)
						).str().c_str());	
						
					m_list.SetItemText(itemPos,2,
						(wformat(L"%1$.2fkb/s") 
							% (peerDetails[i].speed.second/1024)
						).str().c_str());	
						
					if (peerDetails[i].seed)
						m_list.SetItemText(itemPos,3,L"Seed");
				}
			}
		}*/
		
		return;
	}

invalid_selection:

	SetDlgItemText(IDC_NAME,L"N/A");
	SetDlgItemText(IDC_TRACKER,L"N/A");
	SetDlgItemText(IDC_STATUS,L"N/A");
	SetDlgItemText(IDC_AVAIL,L"N/A");
	SetDlgItemText(IDC_COMPLETE,L"N/A");
	
	SetDlgItemText(BTNPAUSE,L"Pause");
	
	::EnableWindow(GetDlgItem(BTNPAUSE),false);
	::EnableWindow(GetDlgItem(BTNREANNOUNCE),false);
	::EnableWindow(GetDlgItem(BTNREMOVE),false);
	
	::EnableWindow(GetDlgItem(IDC_EDITTLD),false);
	::EnableWindow(GetDlgItem(IDC_EDITTLU),false);
	::EnableWindow(GetDlgItem(IDC_EDITNCD),false);
	::EnableWindow(GetDlgItem(IDC_EDITNCU),false);
	
	m_list.DeleteAllItems();
}
