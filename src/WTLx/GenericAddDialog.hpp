
//         Copyright Eóin O'Callaghan 2006 - 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define HAL_GENERIC_HOLDER		 	20000

#ifndef RC_INVOKED

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
		load_from_ini();	
	}

	~GenericAddDialog()
	{
		save_to_ini();
	}
	
	enum { IDD = dialogIDD };

	BEGIN_MSG_MAP_EX(thisClass)
		try
		{
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)	

		COMMAND_ID_HANDLER_EX(IDOK, OnOk)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in GenericAddDialog MSG_MAP")
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND, LPARAM)
	{
		TBase* pT = static_cast<TBase*>(this);

		pT->SetWindowText(title_.c_str());

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
		end_dialog(0);
	}

	void OnCancel(UINT uNotifyCode, int nID, ATL::CWindow wndCtl)
	{
		end_dialog(0);
	}

	void OnOk(UINT uNotifyCode, int nID, ATL::CWindow wndCtl)
	{
		end_dialog(1);
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
	void end_dialog(int i)
	{
		TBase* pT = static_cast<TBase*>(this);

		pT->DoDataExchange(true);
		pT->GetWindowRect(rect_);

		pT->EndDialog(i);
	}

	std::wstring title_;
	WTL::CRect rect_;
};

template<class TBase, class dlgClass, int dialogIDD> 
class GenericAddContainerDialog :
	public ATL::CDialogImpl<GenericAddContainerDialog<TBase, dlgClass, dialogIDD> >,
	public WTL::CDialogResize<GenericAddContainerDialog<TBase, dlgClass, dialogIDD> >,
	private hal::IniBase<GenericAddContainerDialog<TBase, dlgClass, dialogIDD> >,
	private boost::noncopyable
{
	typedef GenericAddContainerDialog<TBase, dlgClass, dialogIDD> thisClass;
	typedef WTL::CDialogResize<thisClass> resizeClass;
	typedef hal::IniBase<thisClass> iniClass;
	
public:
	GenericAddContainerDialog(std::wstring title, boost::filesystem::path location, std::string name, dlgClass& dlg) :
		dlg_(dlg),
		title_(title),
		iniClass(location, name),
		rect_(0,0,0,0)
	{
		load_from_ini();	
	}

	~GenericAddContainerDialog()
	{
		save_to_ini();
	}
	
	enum { IDD = HAL_GENERIC_HOLDER };

	BEGIN_MSG_MAP_EX(thisClass)
		try
		{
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)	

		COMMAND_ID_HANDLER_EX(IDOK, OnOk)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		}
		HAL_ALL_EXCEPTION_CATCH(L"in GenericAddContainerDialog MSG_MAP")

		CHAIN_MSG_MAP(resizeClass)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(thisClass)
		DLGRESIZE_CONTROL(dialogIDD, DLSZ_SIZE_X|DLSZ_SIZE_Y)

		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(HWND, LPARAM)
	{
		TBase* pT = static_cast<TBase*>(this);
		
		dlg_.Create(*pT);
		dlg_.SetDlgCtrlID(dialogIDD);

		resizeClass::DlgResize_Init(false, true, WS_CLIPCHILDREN);

		dlg_.ShowWindow(SW_SHOW);

		SetWindowText(title_.c_str());

		if (rect_.left == rect_.right)
		{
			pT->CenterWindow();
			pT->GetWindowRect(rect_);
			pT->MoveWindow(rect_.left, rect_.top, rect_.right-rect_.left, rect_.bottom-rect_.top+28, false);
		}
		else
			pT->MoveWindow(rect_.left, rect_.top, rect_.right-rect_.left, rect_.bottom-rect_.top, false);	

		pT->DoDataExchange(false);				
		return TRUE;
	}
	
	void OnClose()
	{
		end_dialog(0);
	}

	void OnCancel(UINT uNotifyCode, int nID, ATL::CWindow wndCtl)
	{
		end_dialog(0);
	}

	void OnOk(UINT uNotifyCode, int nID, ATL::CWindow wndCtl)
	{
		end_dialog(1);
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
	void end_dialog(int i)
	{
		TBase* pT = static_cast<TBase*>(this);

		pT->DoDataExchange(true);
		pT->GetWindowRect(rect_);

		pT->EndDialog(i);
	}

	dlgClass& dlg_;
	std::wstring title_;
	WTL::CRect rect_;
};

}

#endif
