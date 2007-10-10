
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define IDC_SECURITY_IPF_GB 	110001


#define TORRENT_LIMITS_LAYOUT \
	WMB_HEAD(WMB_COL(_exp|20), WMB_COL(_exp|30), WMB_COL(_exp|20), WMB_COL(_exp|30)), \
		WMB_ROW(10,	IDC_TL,	_r, _r, _r), \
		WMB_ROW(11,	IDC_TLD, IDC_EDITTLD, IDC_TLU, IDC_EDITTLU), \
		WMB_ROW(10,	IDC_NC,	_r, _r, _r), \
		WMB_ROW(11,	IDC_NCD, IDC_EDITNCD, IDC_NCU, IDC_EDITNCU), \
	WMB_END()

#define TORRENT_STATUS_LAYOUT \
	WMB_HEAD(WMB_COL(45), WMB_COLNOMIN(_exp|100), WMB_COL(_eq|0), WMB_COL(_exp|200)), \
		WMB_ROW(10,	IDC_NAME_STATUS_LABEL, IDC_NAME, _r, _r), \
		WMB_ROW(10,	IDC_PEERS_LABEL, IDC_PEERS, IDC_TRACKER, IDC_SEEDS), \
		WMB_ROW(10,	IDC_TRANSFERED_LABEL, IDC_STATUS, IDC_OVERHEAD_LABEL, IDC_OVERHEAD), \
		WMB_ROW(10,	IDC_REMAINING_LABEL, IDC_COMPLETE, IDC_ETA_LABEL, IDC_COMPLETE), \
	WMB_END()
		
#define TORRENT_REANNOUNCE_LAYOUT \
	WMB_HEAD(WMB_COL(50), WMB_COLNOMIN(_exp)), \
		WMB_ROW(10,	IDC_UPDATESTAT, IDC_UPDATE), \
	WMB_END()	
