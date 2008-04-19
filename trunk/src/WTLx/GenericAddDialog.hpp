
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "DdxEx.hpp"

namespace WTLx
{

template<class TBase, int dialogIDD> 
class GenericAddDialog :
	private hal::IniBase<GenericAddDialog<TBase, dialogIDD> >,
	private boost::noncopyable
{
	typedef GenericAddDialog<TBase, dialogIDD> thisClass;
	typedef hal::IniBase<thisClass> iniClass;
	
public:
	GenericAddDialog(std::wstring title, boost::filesystem::path location, std::string name) :
		title_(title),
		iniClass(location, name),
		rect_(0,0,0,0)
	{
		Load();	
	}

	~GenericAddDialog()
	{
		Save();
	}
	
	enum { IDD = dialogIDD };

    BEGIN_MSG_MAP_EX(thisClass)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_CLOSE(OnClose)	
		COMMAND_ID_HANDLER_EX(IDOK, OnOk)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
    END_MSG_MAP()

	LRESULT OnInitDialog(...)
	{
		TBase* pT = static_cast<TBase*>(this);

		pT->SetWindowText(title_.c_str());
		pT->TBase::resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);

		if (rect_.left == rect_.right)
		{
			pT->CenterWindow();
			pT->GetWindowRect(rect_);
		}
		else
			pT->MoveWindow(rect_.left, rect_.top, rect_.right-rect_.left, rect_.bottom-rect_.top, false);	

		pT->DoDataExchange(false);				
		return TRUE;
	}
	
	void OnClose()
	{
		endDialog(0);
	}

	void OnCancel(...)
	{
		endDialog(0);
	}

	void OnOk(...)
	{
		endDialog(1);
	}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return pT->IsDialogMessage(pMsg);
	}
	
	void onCancel(UINT, int, HWND hWnd)
	{}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
	{
		ar & boost::serialization::make_nvp("rect", rect_);
	}

private:

	void endDialog(int i)
	{
		TBase* pT = static_cast<TBase*>(this);
		pT->DoDataExchange(true);
		pT->GetWindowRect(rect_);

		pT->EndDialog(i);
	}

	std::wstring title_;
	WTL::CRect rect_;
};

}

