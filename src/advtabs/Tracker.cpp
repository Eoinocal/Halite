
#include "../stdAfx.hpp"
#include "../HaliteWindow.hpp"
#include "../HaliteListView.hpp"

//#include "../GlobalIni.hpp"
//#include "../ini/Dialog.hpp"

#include "../global/logger.hpp"

#include "Tracker.hpp"

AdvTrackerDialog::AdvTrackerDialog(ui_signal& ui_sig, ListViewManager& single_sel) :
	ui_(ui_sig),
	selection_manager_(single_sel)
{
}

void AdvTrackerDialog::selectionChanged(const string& torrent_name)
{	
	if (hal::bittorrent().isTorrent(torrent_name))
	{		
		::EnableWindow(GetDlgItem(IDC_TRACKER_LOGINCHECK), true);
		::EnableWindow(GetDlgItem(IDC_TRACKERLIST), true);
		
		pair<wstring, wstring> details = 
			hal::bittorrent().getTorrentLogin(selection_manager_.selected());
		
		username_ = details.first;
		password_ = details.second;
		
		std::vector<hal::TrackerDetail> trackers =
			hal::bittorrent().getTorrentTrackers(torrent_name);
		m_list.manager().clearAll();
		
		foreach (const hal::TrackerDetail& tracker, trackers)
		{
			int itemPos = m_list.AddItem(0, 0, tracker.url.c_str(), 0);
			m_list.SetItemText(itemPos, 1, lexical_cast<wstring>(tracker.tier).c_str());
		}
	}
	else
	{		
		::EnableWindow(GetDlgItem(IDC_TRACKER_LOGINCHECK), false);
		::EnableWindow(GetDlgItem(IDC_TRACKERLIST), false);
		
		username_ = L"";
		password_ = L"";
	}
	
	::EnableWindow(GetDlgItem(IDC_TRACKER_APPLY), false);
	
	setLoginUiState(torrent_name);
	
	DoDataExchange(false);	

	ui_.update();
}

void AdvTrackerDialog::onLoginCheck(UINT, int, HWND hWnd)
{
	LRESULT result = ::SendMessage(hWnd, BM_GETCHECK, 0, 0);
	
	if (result == BST_CHECKED)
	{
		::EnableWindow(GetDlgItem(IDC_TRACKER_USER), true);
		::EnableWindow(GetDlgItem(IDC_TRACKER_PASS), true);
	}
	else
	{
		::EnableWindow(GetDlgItem(IDC_TRACKER_USER), false);
		::EnableWindow(GetDlgItem(IDC_TRACKER_PASS), false);	

		username_ = L"";	
		password_ = L"";
		
		DoDataExchange(false);		
	}
}

LRESULT AdvTrackerDialog::onInitDialog(HWND, LPARAM)
{
	ui_.attach(bind(&AdvTrackerDialog::updateDialog, this));
	selection_manager_.attach(bind(&AdvTrackerDialog::selectionChanged, this, _1));
	
	resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);
	
	m_list.Attach(GetDlgItem(IDC_TRACKERLIST));	
	m_list.attachEditedConnection(bind(&AdvTrackerDialog::trackerListEdited, this));
	
	if (hal::bittorrent().isTorrent(selection_manager_.selected()))
	{		
		::EnableWindow(GetDlgItem(IDC_TRACKER_LOGINCHECK), true);
		::EnableWindow(GetDlgItem(IDC_TRACKERLIST), true);
		
		pair<wstring, wstring> details = 
			hal::bittorrent().getTorrentLogin(selection_manager_.selected());
		
		username_ = details.first;
		password_ = details.second;
	}
	else
	{		
		::EnableWindow(GetDlgItem(IDC_TRACKER_LOGINCHECK), false);
		::EnableWindow(GetDlgItem(IDC_TRACKERLIST), false);
		
		username_ = L"";
		password_ = L"";
	}
		
	setLoginUiState(selection_manager_.selected());
	DoDataExchange(false);	
	return 0;
}

void AdvTrackerDialog::setLoginUiState(const string& torrent_name)
{
	if (username_ == L"")
	{
		::SendMessage(GetDlgItem(IDC_TRACKER_LOGINCHECK), BM_SETCHECK, BST_UNCHECKED, 0);
		password_ = L"";
	}
	else
	{	
		::SendMessage(GetDlgItem(IDC_TRACKER_LOGINCHECK), BM_SETCHECK, BST_CHECKED, 0);
	}
	
	onLoginCheck(0, 0, GetDlgItem(IDC_TRACKER_LOGINCHECK));	
}

void AdvTrackerDialog::onClose()
{
	if(::IsWindow(m_hWnd)) 
	{
		::DestroyWindow(m_hWnd);
	}
}

LRESULT AdvTrackerDialog::OnEditKillFocus(UINT uCode, int nCtrlID, HWND hwndCtrl)
{
	DoDataExchange(true);
	
	setLoginUiState(selection_manager_.selected());
	hal::bittorrent().setTorrentLogin(selection_manager_.selected(), username_, password_);
	
	return 0;
}

void AdvTrackerDialog::onReannounce(UINT, int, HWND)
{
	hal::bittorrent().reannounceTorrent(selection_manager_.selected());
	
//	hal::event().post(shared_ptr<hal::EventDetail>(new hal::EventDetail(hal::Event::critical, 
//		boost::posix_time::second_clock::universal_time(), 123456)));
}

void AdvTrackerDialog::trackerListEdited()
{
	::EnableWindow(GetDlgItem(IDC_TRACKER_APPLY), true);
}

void AdvTrackerDialog::updateDialog()
{}

void AdvTrackerDialog::onReset(UINT, int, HWND)
{
	hal::bittorrent().resetTorrentTrackers(selection_manager_.selected());
	
	std::vector<hal::TrackerDetail> trackers =
		hal::bittorrent().getTorrentTrackers(selection_manager_.selected());
	m_list.manager().clearAll();
	
	foreach (const hal::TrackerDetail& tracker, trackers)
	{
		int itemPos = m_list.AddItem(0, 0, tracker.url.c_str(), 0);
		m_list.SetItemText(itemPos, 1, lexical_cast<wstring>(tracker.tier).c_str());
	}

	::EnableWindow(GetDlgItem(IDC_TRACKER_APPLY), false);
}

void AdvTrackerDialog::onApply(UINT, int, HWND)
{
	int total = m_list.GetItemCount();
	std::vector<hal::TrackerDetail> trackers;
	
	for (int i=0; i<total; ++i)
	{
		array<wchar_t, MAX_PATH> buffer;		
		
		m_list.GetItemText(i, 0, buffer.elems, buffer.size());
		trackers.push_back(hal::TrackerDetail(wstring(buffer.elems), 0));
		
		m_list.GetItemText(i, 1, buffer.elems, buffer.size());
		trackers.back().tier = lexical_cast<int>(wstring(buffer.elems));
	}
	
	hal::bittorrent().setTorrentTrackers(selection_manager_.selected(), trackers);
	
	::EnableWindow(GetDlgItem(IDC_TRACKER_APPLY), false);
}
