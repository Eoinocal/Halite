
//         Copyright Eóin O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdAfx.hpp"
#include "global/string_conv.hpp"

#include <boost/array.hpp>
#include <boost/signals.hpp>
#include <boost/serialization/vector.hpp>

#include "HaliteIni.hpp"
#include "HaliteListViewCtrl.hpp"
#include "HaliteSortListViewCtrl.hpp"

class HaliteWindow;

class HaliteListViewCtrl :
	public CHaliteSortListViewCtrl<HaliteListViewCtrl, const hal::TorrentDetail_ptr>,
	private CHaliteIni<HaliteListViewCtrl>,
	private boost::noncopyable
{
protected:
	typedef CHaliteIni<HaliteListViewCtrl> iniClass;
	typedef CHaliteSortListViewCtrl<HaliteListViewCtrl, const hal::TorrentDetail_ptr> listClass;
	typedef const hal::TorrentDetail_ptr tD;

	friend class listClass;
	
	struct ColumnAdapters
	{
	
	typedef listClass::ColumnAdapter ColAdapter_t;
	
	struct Filename : public ColAdapter_t
	{	
		virtual bool less(tD& l, tD& r)	{ return l->filename() < r->filename(); }		
		virtual std::wstring print(tD& t) { return t->filename(); }		
	};
	
	struct State : public ColAdapter_t
	{	
		virtual bool less(tD& l, tD& r) { return l->state() < r->state(); }		
		virtual std::wstring print(tD& t) { return t->state(); }		
	};
	
	struct Tracker : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)	{ return l->currentTracker() < r->currentTracker(); }		
		virtual std::wstring print(tD& t) { return t->currentTracker(); }		
	};
	
	struct SpeedDown : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)	{ return l->speed().first < r->speed().first; }		
		virtual std::wstring print(tD& t) 
		{
			return (wformat(L"%1$.2fkb/s") % (t->speed().first/1024)).str(); 
		}		
	};
	
	struct SpeedUp : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)	{ return l->speed().second < r->speed().second; }		
		virtual std::wstring print(tD& t) 
		{
			return (wformat(L"%1$.2fkb/s") % (t->speed().second/1024)).str(); 
		}		
	};

	struct Progress : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)	{ return l->completion() < r->completion(); }		
		virtual std::wstring print(tD& t) 
		{
			return (wformat(L"%1$.2f%%") % (t->completion()*100)).str(); 
		}		
	};

	struct Peers : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)	{ return l->peers() < r->peers(); }		
		virtual std::wstring print(tD& t) 
		{
			return (wformat(L"%1% (%2%)") % t->peersConnected() % t->peers()).str(); 
		}
	};
	
	struct Seeds : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)	{ return l->seeds() < r->seeds(); }				
		virtual std::wstring print(tD& t) 
		{
			return (wformat(L"%1% (%2%)") % t->seedsConnected() % t->seeds()).str(); 
		}	
	};
	
	struct ETA : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)	{ return l->estimatedTimeLeft() < r->estimatedTimeLeft(); }		
		virtual std::wstring print(tD& t) 
		{ 
			if (!t->estimatedTimeLeft().is_special())
			{
				return hal::from_utf8(
					boost::posix_time::to_simple_string(t->estimatedTimeLeft()));
			}
			else
			{
				return L"∞";		
			}
		}		
	};
	
	struct UpdateTrackerIn : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)	{ return l->updateTrackerIn() < r->updateTrackerIn(); }		
		virtual std::wstring print(tD& t) 
		{ 
			if (!t->updateTrackerIn().is_special())
			{
				return hal::from_utf8(
					boost::posix_time::to_simple_string(t->updateTrackerIn()));
			}
			else
			{
				return L"∞";		
			}
		}		
	};
	
	struct Ratio : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)	{ return l->ratio() < r->ratio(); }		
		virtual std::wstring print(tD& t) { return lexical_cast<wstring>(t->ratio()); }		
	};
	
	struct DistributedCopies : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)	{ return l->distributedCopies() < r->distributedCopies(); }		
		virtual std::wstring print(tD& t) 
		{ 
			float copies = t->distributedCopies();
			
			if (copies < 0)
				return L"Seeding"; 
			else
				return (wformat(L"%1$.2f") % copies).str();		
		}		
	};

	struct Remaining : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)
		{
			boost::int64_t left = l->totalWanted()-l->totalWantedDone();
			boost::int64_t right = r->totalWanted()-r->totalWantedDone();
			
			return left < right; 
		}
		
		virtual std::wstring print(tD& t) 
		{
			return (wformat(L"%1$.2fMB") % (static_cast<float>(t->totalWanted()-t->totalWantedDone())/(1024*1024))).str(); 
		}		
	};

	struct Completed : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)
		{			
			return l->totalWantedDone() < r->totalWantedDone(); 
		}
		
		virtual std::wstring print(tD& t) 
		{
			return (wformat(L"%1$.2fMB") % (static_cast<float>(t->totalWantedDone())/(1024*1024))).str(); 
		}		
	};

	struct TotalWanted : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)
		{		
			return l->totalWanted() < r->totalWanted(); 
		}
		
		virtual std::wstring print(tD& t) 
		{
			return (wformat(L"%1$.2fMB") % (static_cast<float>(t->totalWanted())/(1024*1024))).str(); 
		}		
	};

	struct Downloaded : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)
		{		
			return l->totalPayloadDownloaded() < r->totalPayloadDownloaded(); 
		}
		
		virtual std::wstring print(tD& t) 
		{
			return (wformat(L"%1$.2fMB") % (static_cast<float>(t->totalPayloadDownloaded())/(1024*1024))).str(); 
		}		
	};

	struct Uploaded : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)
		{		
			return l->totalPayloadUploaded() < r->totalPayloadUploaded(); 
		}
		
		virtual std::wstring print(tD& t) 
		{
			return (wformat(L"%1$.2fMB") % (static_cast<float>(t->totalPayloadUploaded())/(1024*1024))).str(); 
		}		
	};

	struct ActiveTime : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)	{ return l->active() < r->active(); }		
		virtual std::wstring print(tD& t) 
		{ 
			if (!t->active().is_special())
			{
				return hal::from_utf8(
					boost::posix_time::to_simple_string(t->active()));
			}
			else
			{
				return L"∞";		
			}
		}		
	};
	
	struct SeedingTime : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)	{ return l->seeding() < r->seeding(); }		
		virtual std::wstring print(tD& t) 
		{ 
			if (!t->seeding().is_special())
			{
				return hal::from_utf8(
					boost::posix_time::to_simple_string(t->seeding()));
			}
			else
			{
				return L"∞";		
			}
		}		
	};
	
	struct StartTime : public ColAdapter_t
	{
		virtual bool less(tD& l, tD& r)	{ return l->startTime() < r->startTime(); }		
		virtual std::wstring print(tD& t) 
		{ 
			if (!t->startTime().is_special())
			{
				return hal::from_utf8(
					boost::posix_time::to_simple_string(t->startTime()));
			}
			else
			{
				return L"∞";		
			}
		}		
	};
	
	};

public:
	enum { 
		LISTVIEW_ID_MENU = IDR_LISTVIEW_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_LISTVIEW_COLUMNS,
		LISTVIEW_ID_COLUMNWIDTHS = HAL_LISTVIEW_DEFAULTS
	};

	HaliteListViewCtrl(HaliteWindow& HalWindow);

	BEGIN_MSG_MAP_EX(HaliteListViewCtrl)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER(ID__LVM_PAUSE, OnPause)
		COMMAND_ID_HANDLER(ID_LVM_STOP, OnStop)
		COMMAND_ID_HANDLER(ID_LVM_RESUME, OnResume)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_T, OnRemove)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_TD, OnRemoveWipeFiles)

		CHAIN_MSG_MAP(listClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void OnShowWindow(UINT, INT);
	void OnDestroy();
	void saveSettings();
	void uiUpdate(const hal::TorrentDetails& allTorrents); 

	LRESULT OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemoveFocused(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		if (version > 1)
			ar & boost::serialization::make_nvp("listview", boost::serialization::base_object<listClass>(*this));
    }
	
	tD CustomItemConversion(LVCompareParam* param, int iSortCol);

private:
	void OnAttach();
	void OnDetach();
	
	enum { NumberOfColumns_s = 19 };
	
	HaliteWindow& halWindow_;
};

BOOST_CLASS_VERSION(HaliteListViewCtrl, 2)
typedef HaliteListViewCtrl::SelectionManager ListViewManager;
