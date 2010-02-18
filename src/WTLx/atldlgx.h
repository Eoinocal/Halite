// atldlgx.h
//
// Unusual but useful specializable WTL dialog classes
//
// Copyright (c) Alain Rist 2007
//
// The use and distribution terms for this software are covered by the
// Common Public License 1.0 (http://opensource.org/osi3.0/licenses/cpl1.0.php)
// By using this software in any fashion, you are agreeing to be bound by
// the terms of this license. You must not remove this notice, or
// any other, from this software.

#ifndef __ATLDLGX_H__
#define __ATLDLGX_H__

#pragma once

#ifndef _WTL_VER
	#error atldlgx.h requires the Windows Template Library
#elif _WTL_VER < 0x800
	#error atldlgx.h requires WTL version 8.0 or over
#endif

#ifndef __ATLDLGS_H__
	#error atldlgx.h requires atldlgs.h to be included first
#endif

#ifndef __ATLCTRLS_H__
	#error atldlgx.h requires atlctrls.h to be included first
#endif

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CEmptyDlgTemplate - Empty dialog template loading the dialog title from a resource string
// CEmptyDialogImpl - Implements empty dialogs
// CMenuDialog - MI class for any modal dialog to behave as a menu
// CCellMenu - Specializable cell composed dialog to use as a menu
// CControlDialogImpl - Single control dialog implementation
// CControlDialog - Specializable single control dialog
// CInPlaceEditor - In place editing dialog
// CSplitDialogImpl - Split dialog implementation
// CSplitDialog - Specializable split dialog
// CVSplitDialog - Specializable vertical WTL::CSplitterImpl based split dialog
// CHSplitDialog - Specializable horizontal WTL::CSplitterImpl based split dialog
//
// aero::CEmptyDialogImpl - Aero enabled CEmptyDialogImpl
// aero::CControlDialog - Aero enabled CControlDialog
// aero::CSplitDialog - Aero enabled CSplitDialog
//
// CStdEmptyDialogImpl - base class for Mobile Device standard empty dialog classes
// CStdControlDialog - Mobile Device specializable standard single control dialog class
// CStdSplitDialog - Mobile Device specializable standard split dialog class
// CStdVSplitDialog - Specializable vertical WTL::CSplitterImpl based split dialog
// CStdHSplitDialog - Specializable horizontal WTL::CSplitterImpl based split dialog

// Macros

#ifndef MESSAGE_ANSWER
#define MESSAGE_ANSWER(msg, res) \
	if(uMsg == msg) \
	{ \
		lResult = res; \
		return bHandled = TRUE; \
	}
#endif

namespace WTL
{

/////////////////////////////////////////////////////////////////
// CEmptyDlgTemplate : Empty dialog template loading the dialog title from a resource string
//

typedef ATL::CWinTraits<WS_VISIBLE | WS_POPUP> CEmptyDlgTraits;

template <UINT t_uIDS, class TDlgTraits = CEmptyDlgTraits>
class CEmptyDlgTemplate : public CMemDlgTemplate
{
public:
	void Create(POINT point, SIZE size)
	{
		DWORD dw0 = NULL;
		DLGTEMPLATE dt = {TDlgTraits::GetWndStyle(0), TDlgTraits::GetWndExStyle(0), 0, (SHORT)point.x, (SHORT)point.y, (SHORT)size.cx, (SHORT)size.cy};
		AddData(&dt, sizeof(dt));
		AddData(&dw0, sizeof(DWORD));
		if (t_uIDS)
		{
	#ifdef _WIN32_WCE
			AddString(AtlLoadString(t_uIDS));
	#else
			CTempBuffer<TCHAR> sTitle(255);
			AtlLoadString(t_uIDS, sTitle, 255);
			AddString(sTitle);
	#endif
		}
	}

};

///////////////////////////////////////////////////////////////////////////////
// CEmptyDialogImpl - Implements empty dialogs
//                 

template
	< class T,		// Actual dialog class: ie CMyEmptyDialog  
	UINT t_uIDD,	// Dialog IDD, title, icon, toolbar, menu (if matching resources exist) 
	class TDlgTemplate	// In memory dialog template
		= CEmptyDlgTemplate<t_uIDD, CEmptyDlgTraits>, // default for TDlgTemplate
	class TBase			// CIndirectDialogImpl::TBase class
		= ATL::CDialogImpl<T, ATL::CWindow>  // default for TBase 
	>
class ATL_NO_VTABLE CEmptyDialogImpl : 
	public CIndirectDialogImpl<
	T,
	TDlgTemplate, 
	TBase
	> // CIndirectDialogImpl
{
	typedef CEmptyDialogImpl<T, t_uIDD, TDlgTemplate, TBase> this_class_t;
public:
	typedef this_class_t EmptyDialog;

// Constructors
	// Empty constructor : derived class performs m_Template and m_Data initializations
	CEmptyDialogImpl(){}

	// Sizing constructor : sets origin to {0,0}
	CEmptyDialogImpl(SIZE size, bool bDLU = true) : m_Data(0)
	{
		POINT point = {0};
		if(bDLU)
			PixToDLU(size);
		m_Template.Create(point, size);
	}

	// Positioning constructor
	CEmptyDialogImpl(POINT point, SIZE size, bool bDLU = true) : m_Data(0)
	{
		if(bDLU)
		{
			PixToDLU(point);
			PixToDLU(size);
		}
		m_Template.Create(point, size);
	}

	// Positioning constructor
	CEmptyDialogImpl(RECT rect, bool bDLU = true) : m_Data(0)
	{
		POINT point = {rect.left, rect.top};
		SIZE size = { rect.right - rect.left, rect.bottom - rect.top};
		if(bDLU)
		{
			PixToDLU(point);
			PixToDLU(size);
		}
		m_Template.Create(point, size);
	}

	// Data members
	enum {IDD = t_uIDD};
	LPARAM m_Data; // user data

	// ToolBar management
#ifndef _WIN32_WCE // WTL 8.0 does not support WinCE standalone toolbars
	HWND m_hWndToolBar;

	HWND CreateToolBar(UINT nID = t_uIDD)
	{
		return m_hWndToolBar = AtlCreateSimpleToolBar(m_hWnd, nID);
	}

	BOOL GetClientRect(LPRECT lpRect) 
	{
		ATLASSERT(IsWindow());

		BOOL bRes = ::GetClientRect(m_hWnd, lpRect);
		if(::IsWindow(m_hWndToolBar) && bRes)
		{
			RECT rTB;
			::GetWindowRect(m_hWndToolBar, &rTB);
			// assume CCS_TOP style
			lpRect->top += rTB.bottom - rTB.top;
		}
		return bRes;
	}
#endif // !_WIN32_WCE

// Overrideable
	bool Init(LPARAM lParam)
	{
		return DefInit(lParam);
	}

	bool Size(WPARAM /*wParam*/,LPARAM /*lParam*/)
	{
		Invalidate();
		return false;
	}

	bool Paint(HDC /*hdc*/)
	{
		return false;
	}

	void Close(INT iCmd) 
	{
		// Modal closing
		ATLVERIFY(EndDialog(iCmd));
	}

// Implementation
	bool DefInit(LPARAM lParam)
	{
		m_Data = lParam;
#ifndef _WIN32_WCE
		CreateToolBar();
		SetMenu(AtlLoadMenu(t_uIDD));
#endif
		if (HICON hIconSmall = (HICON)::LoadImage(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(t_uIDD), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR))
				SetIcon(hIconSmall, FALSE);

		m_Template.Reset(); // free DLGTEMPLATE memory
		return false; // WM_INITDIALOG not handled
	}

	// Prevent non modal execution
	HWND Create(HWND, LPARAM)
	{
		ATLASSERT(FALSE); 
		return NULL;
	}

// Message map and handlers
	BEGIN_MSG_MAP(CEmptyDialogImpl)
		MESSAGE_ANSWER(WM_ERASEBKGND, TRUE)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
		COMMAND_RANGE_HANDLER(IDOK, IDCANCEL, OnCloseCommand)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
#ifndef _WIN32_WCE
		MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
#endif
	END_MSG_MAP()

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
#ifndef _WIN32_WCE
		if(::IsWindow(m_hWndToolBar))
			CToolBarCtrl(m_hWndToolBar).AutoSize();
#endif
		bHandled = static_cast<T*>(this)->Size(wParam, lParam);
		return 0;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		if (wParam == NULL)
		{
			CPaintDC dc(/*pT->*/m_hWnd);
			return bHandled = pT->Paint(dc.m_hDC);
		}
		else
			return bHandled = pT->Paint((HDC)wParam);
	}

	LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam == SC_CLOSE)
			static_cast<T*>(this)->Close(IDCANCEL);
		else
			bHandled = FALSE;
		return 0;
	}

	LRESULT OnCloseCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		static_cast<T*>(this)->Close(wID);
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = static_cast<T*>(this)->Init(lParam);
		return FALSE; // don't set focus to default control
	}

// Implementation helpers
	bool MoveToTemplatePix()
	{
		ATLASSERT(IsWindow());
		ATLASSERT(m_Template.IsValid());

		LPDLGTEMPLATE pdt = m_Template.GetTemplatePtr();
		return MoveWindow(pdt->x, pdt->y, pdt->cx, pdt->cy, FALSE) == TRUE;
	}

	static void PixToDLU(LONG &x, LONG &y) 
	{
		LONG lBaseUnit = ::GetDialogBaseUnits();
		x = ::MulDiv(x , 4, LOWORD(lBaseUnit));
		y = ::MulDiv(y , 8, HIWORD(lBaseUnit));
	}

	static void PixToDLU(SIZE &size) 
	{
		PixToDLU(size.cx, size.cy);
	}

	static void PixToDLU(POINT &point) 
	{
		PixToDLU(point.x, point.y);
	}

};

///////////////////////////////////////////////////////////////////
// CMenuDialog - MI class for any modal dialog to behave as a menu
//

template <class T>
class CMenuDialog 
{
public:
	// Constructor
	CMenuDialog() : m_bAccept(false)
	{}

	bool m_bAccept;

// Message map and handlers
	BEGIN_MSG_MAP(CMenuDialog)
#ifndef _WIN32_WCE
		MESSAGE_HANDLER(WM_INITDIALOG, OnInit)
#else
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_CANCELMODE, OnKill)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSetting)
#endif // _WIN32_WCE
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		COMMAND_ID_HANDLER(IDOK, OnOK)
	END_MSG_MAP()

	LRESULT OnActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if ((wParam == WA_INACTIVE) && !m_bAccept)
			static_cast<T*>(this)->PostMessage(WM_COMMAND, IDCANCEL);
		return bHandled =FALSE;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		m_bAccept = true;
		return bHandled = FALSE;
	}

#ifndef _WIN32_WCE
	LRESULT OnInit(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		::EnableWindow(pT->GetParent(), TRUE);
		pT->SetFocus();
		return bHandled = FALSE;
	}
#else // _WIN32_WCE
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		::EnableWindow(static_cast<T*>(this)->GetParent(), TRUE);
		return bHandled = FALSE;
	}

	LRESULT OnKill(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(!m_bAccept)
			static_cast<T*>(this)->PostMessage(WM_COMMAND, IDCANCEL);
		return bHandled =FALSE;
	}

	LRESULT OnSetting(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(!m_bAccept && (wParam == SETTINGCHANGE_RESET))
			static_cast<T*>(this)->PostMessage(WM_COMMAND, IDCANCEL);
		return bHandled = FALSE;
	}
#endif // _WIN32_WCE

};

/////////////////////////////////////////////////////////////////////////////
// CCellMenu - Cell composed dialog to use as a menu
//
// This is a specializable class. The default code loads a ribbon BITMAP resource
// with t_uIDD identifier, sizes the cells and paints them from this bitmap.
//
// If there is no t_uIDD BITMAP resource, the cell size is computed from SM_CXSMICON and
// SM_CYSMICON and you must provide at least a specialized PaintCell() member.
//
// The initial selection index is passed as a LPARAM to DoModal().
// The return value is the selected cell index or -1 if cancelled.
//
// TrackCellMenu() positions the dialog with same semantics as TrackPopupMenu 
// and returns the result of DoModal().
//

typedef ATL::CWinTraits<WS_POPUP | WS_BORDER> CMenuDialogTraits;

class CMenuDlgTemplate : public CEmptyDlgTemplate<0, CMenuDialogTraits>
{};

template 
	< UINT t_uIDD,	// Menu dialog IDD and bitmap (if matching resource exists)
	INT t_nCol,		// Number of columns
	INT t_nRow		// Number of rows
	>
class CCellMenu : 
	public CEmptyDialogImpl<
		/*this_class_t*/CCellMenu<t_uIDD, t_nCol, t_nRow>, 
		t_uIDD,
		/*TDlgTemplate*/CMenuDlgTemplate
		>, // CEmptyDialogImpl
	public CMenuDialog<
		/*this_class_t*/CCellMenu<t_uIDD, t_nCol, t_nRow> 
		> // CMenuDialog
{
	typedef CCellMenu<t_uIDD, t_nCol, t_nRow> this_class_t;
public:
	typedef CMenuDialog<this_class_t> MenuDialog;

	// Helper for size and position computations
	struct CELL
	{
		// Construct from coordinates
		CELL(int xCol, int yRow) : xcol(xCol), yrow(yRow)
		{}

		// Construct from index
		CELL(int index) : xcol(index % t_nCol), yrow(index / t_nCol)
		{}

		// Construct from mouse message lParam
		CELL(LPARAM lParam) : xcol(GET_X_LPARAM(lParam) / Size().cx), yrow(GET_Y_LPARAM(lParam) / Size().cy)
		{}

#if (_MSC_VER < 1300) || (_ATL_VER < 0x700)  // eVC-VC6 and VCExpress-ATL3
		static const int Cxy()
		{
			return 2;
		}

		static const SIZE Size()
		{
			static SIZE s = CellSize();
			return s;
		}

#else // VS 7.0 and over, not VCExpress-ATL3
		static const int cxy;
		static const SIZE size;

		static const int Cxy()
		{
			return cxy;
		}

		static const SIZE Size()
		{
			return size;
		}

#endif // _MSC_VER < 1300 || (_ATL_VER < 0x700)

		const int Left(bool bPaint = false)
		{
			return xcol * Size().cx + Cxy() * bPaint;
		}

		const int Top(bool bPaint = false)
		{
			return yrow * Size().cy + Cxy() * bPaint;
		}

		const RECT Rect(bool bPaint = false)
		{
			RECT r = {Left(bPaint), Top(bPaint), 
				Left(bPaint) + Size().cx - 1 - 2 * Cxy() * bPaint, 
				Top(bPaint) + Size().cy - 1 - 2 * Cxy() * bPaint};
			return r;
		}

		int xcol;
		int yrow;

		const int Index()
		{
			return yrow * t_nCol + xcol;
		}

		void Key(UINT uKey)
		{
			// First and last cell specific
			if ((xcol + yrow == 0) && ((uKey == VK_UP) || (uKey == VK_LEFT)))
				uKey = VK_END;
			else if ((xcol == t_nCol - 1) && (yrow  == t_nRow - 1) && ((uKey == VK_DOWN) || (uKey == VK_RIGHT)))
				uKey = VK_HOME;
			// Adjust cell position
			switch (uKey)
			{
				case VK_LEFT:	xcol -- ; break;
				case VK_UP: 	yrow -- ; break;
				case VK_RIGHT:	xcol ++ ; break;
				case VK_DOWN:	yrow ++ ; break;
				case VK_HOME:	xcol = 0; // fall through
				case VK_PRIOR:	yrow = 0; return;
				case VK_END:	xcol = t_nCol - 1; // fall through
				case VK_NEXT:	yrow = t_nRow - 1; // fall through
				default: return;
			}
			// Move around if out of bounds
			if (xcol < 0)
				xcol = t_nCol - 1, yrow--;
			else if (xcol >= t_nCol)
				xcol = 0, yrow ++;
			if (yrow < 0)
				yrow = t_nRow - 1, xcol--;
			else if (yrow >= t_nRow)
				yrow = 0, xcol++;
		}
	}; // struct CELL

	// Constructor
	CCellMenu(POINT ptOrig) : EmptyDialog(ptOrig, MenuSize(), false)
	{}

	// Overrides
	void Close(INT iCmd)
	{
		EndDialog(iCmd == IDOK ? m_Data : -1);
	}

	bool Init(LPARAM lParam)
	{
		m_Data = lParam;
		MoveToTemplatePix();
		m_Template.Reset(); // free DLGTEMPLATE memory
		return false;
	}

	bool Mouse(UINT uMsg, WPARAM /*wParam*/,LPARAM lParam)
	{
		switch(uMsg)
		{
		case WM_LBUTTONUP:
			m_Data = CELL(lParam).Index();
			PostMessage(WM_COMMAND,IDOK);
			break;
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
			{
				CELL cell(lParam);
				if ((INT)m_Data != cell.Index())
					UpdateSelection(cell);
			}
			break;
		default:
			return false;
		}
		return true;
	}

	bool Key(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
	{
		if (uMsg == WM_KEYUP)
		{
			CELL cell((INT)m_Data);
			cell.Key((UINT)wParam);
			if ((INT)m_Data != cell.Index())
				UpdateSelection(cell);
			return true;
		}
		return false;
	};

// Specializables
	void PrePaint(HDC hdc)
	{
		SIZE s = MenuSize();
		RECT rect = {0, 0, s.cx, s.cy};
		CDCHandle(hdc).FillSolidRect(&rect, GetSysColor(COLOR_MENU));
	}

	void PaintCells(HDC hdc)
	{
		for (int iRow = 0; iRow <  t_nRow; iRow++)
			for (int iCol = 0; iCol < t_nCol; iCol++)
			{
				CELL cell(iCol, iRow);
				PaintCell(hdc, cell);
			}
	}

	void PaintCell(HDC hdc, CELL &cell)
	{
		GetImageList().Draw(hdc, cell.Index(), cell.Left(true), cell.Top(true), 
			cell.Index() == (INT)m_Data ? ILD_SELECTED : ILD_NORMAL);
	}

	void FocusSelection(HDC hdc)
	{
		RECT rect = CELL((INT)m_Data).Rect();
		CDCHandle(hdc).DrawFocusRect(&rect);
	}

	bool Paint(HDC hdc)
	{
		PrePaint(hdc);
		PaintCells(hdc);
		FocusSelection(hdc);
		return true;
	}

// Operation
	static int TrackCellMenu(POINT pt, UINT uFlags = 0, LPARAM lParam = 0, HWND hWndParent = GetActiveWindow())
	{
		const SIZE sAll = MenuSize();

		// Horizontal position 
		if (uFlags & TPM_RIGHTALIGN)
			pt.x -= sAll.cx;
		else if (uFlags & TPM_CENTERALIGN)
			pt.x -= sAll.cx / 2;
		// Try to stay inside parent
		if (pt.x < 0)
			pt.x = 0;
		else 
		{
			int xmax = GetDeviceCaps(::GetDC(hWndParent), HORZRES);
			if ((pt.x + sAll.cx) > xmax)
				pt.x = xmax - sAll.cx;
		}

		// Vertical position 
		if (uFlags & TPM_BOTTOMALIGN)
			pt.y -= sAll.cy;
		else if (uFlags & TPM_VCENTERALIGN)
			pt.y -= sAll.cy / 2;
		// Try to stay inside parent
		if (pt.y < 0)
			pt.y = 0;
		else
		{
			int ymax = GetDeviceCaps(::GetDC(hWndParent), VERTRES);
			if ((pt.y + sAll.cy) > ymax)
				pt.y = ymax - sAll.cy;
		}

		// Return user selection
		return CCellMenu(pt).DoModal(hWndParent, lParam);
	}

	// Implementation
	void  UpdateSelection(CELL &cell)
	{
		RECT r = CELL((INT)m_Data).Rect();
		InvalidateRect(&r, FALSE);
		r = cell.Rect();
		InvalidateRect(&r, FALSE);
		m_Data = cell.Index();
	}

	static CImageList& GetImageList()
	{
		static CImageList iml;
		if (iml.IsNull())
#ifndef _WIN32_WCE
			iml.CreateFromImage(t_uIDD, CELL::Size().cx - 2 * CELL::Cxy(), 0, 
				AtlIsAlphaBitmapResource(t_uIDD) ? CLR_NONE : CLR_DEFAULT,
				IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
#else
			iml.Create(t_uIDD, CELL::Size().cx - 2 * CELL::Cxy(), 0, CLR_DEFAULT);
#endif // _WIN32_WCE
		ATLASSERT(!iml.IsNull());
		return iml;
	}

	// Message map and handlers
	BEGIN_MSG_MAP(CCellMenu)
		MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnMouse)
		MESSAGE_RANGE_HANDLER(WM_KEYFIRST, WM_KEYLAST, OnKey)
		CHAIN_MSG_MAP(MenuDialog)
		CHAIN_MSG_MAP(EmptyDialog)
	END_MSG_MAP()
		
	LRESULT OnMouse(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = Mouse(uMsg, wParam, lParam);
		return 0;
	}

	LRESULT OnKey(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
#if defined(WIN32_PLATFORM_WFSP)
		if( wParam == VK_TBACK) // SmartPhone back key
			Close(IDCANCEL);
		else
#endif
			bHandled = Key(uMsg, wParam, lParam);
		return 0;
	}

	// Cell constants overrideables
	static const SIZE CellSize()
	{
		SIZE size;
		CBitmap bm = AtlLoadBitmap(t_uIDD);

		if (bm.IsNull())
		{
			size.cx = GetSystemMetrics(SM_CXSMICON) + 2 * CELL::Cxy();
			size.cy = GetSystemMetrics(SM_CYSMICON) + 2 * CELL::Cxy();
		}
		else
		{
			SIZE sbm;
			bm.GetSize(sbm);
			size.cx = sbm.cx / (t_nCol * t_nRow) + 2 * CELL::Cxy();
			size.cy = sbm.cy + 2 * CELL::Cxy();
		}
		return size;
	}

	static const SIZE MenuSize() 
	{
		static SIZE sMenu = {CELL::Size().cx * t_nCol + CELL::Cxy() - 1, 
			CELL::Size().cy * t_nRow + CELL::Cxy() - 1};
		return sMenu;
	}
};

#if (_MSC_VER >= 1300) && (_ATL_VER >= 0x700)// VC7.0 and over except VCExpress-ATL3

template <UINT t_uIDD, INT t_nCol, INT t_nRow>
#ifdef _WTL_CE_DRA // Mobile resolution awareness
_declspec(selectany) const int CCellMenu<t_uIDD, t_nCol, t_nRow>::CELL::cxy = DRA::SCALEX(2);
#else
_declspec(selectany) const int CCellMenu<t_uIDD, t_nCol, t_nRow>::CELL::cxy = 2;
#endif

template <UINT t_uIDD, INT t_nCol, INT t_nRow>
_declspec(selectany) const SIZE CCellMenu<t_uIDD, t_nCol, t_nRow>::CELL::size = CellSize();

#endif // (_MSC_VER >= 1300) && (_ATL_VER >= 0x700)

// -- Single control dialog classes

///////////////////////////////////////////////////////////////////////////////
// CControlDialogImpl - Single control dialog implementation
//

typedef ATL::CWinTraits<WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_CENTER, WS_EX_DLGMODALFRAME> CControlDlgTraits;

template 
	< class T,		// Actual dialog class: ie CMyControlDialog 
	UINT t_uIDD,	// Dialog IDD, title, icon, toolbar, menu (if matching resources exist) 
	class TCtrl,	// Control class 
	class TControlTraits // Control styles 
		= ATL::CControlWinTraits,	// default control styles 
	class TDlgImpl	// Empty dialog base class
		= CEmptyDialogImpl<T, t_uIDD, CControlDlgTraits> // default for TDlgImpl
	>
class CControlDialogImpl :  
	public TDlgImpl
{
	typedef CControlDialogImpl<T, t_uIDD, TCtrl, TControlTraits, TDlgImpl> this_class_t;
public:
	typedef this_class_t ControlDialog;

// Construction
	CControlDialogImpl(){}
	CControlDialogImpl(SIZE size, bool bDLU = true) : EmptyDialog(size, bDLU){}
	CControlDialogImpl(POINT point, SIZE size, bool bDLU = true) : EmptyDialog(point, size, bDLU){}
	CControlDialogImpl(RECT rect, bool bDLU = true) : EmptyDialog(rect, bDLU){}

// Control member
	TCtrl m_Ctrl;

// Overrideables
	bool Init(LPARAM lParam)
	{
		return DefInit(lParam);
	}

	bool Size(WPARAM wParam, LPARAM lParam)
	{
		return DefSize(wParam, lParam); 
	}

	bool Notify(int /*idCtrl*/, LPNMHDR /*pnmhdr*/)
	{
		return false;
	}

	bool Command(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		return false;
	}

	bool Key(UINT /*uMsg*/, WPARAM /*wParam*/,LPARAM /*lParam*/)
	{
		return false;
	}

	HBRUSH CtlColor(HDC /*hdc*/)
	{
		return NULL;
	}

	// Implementation
	bool DefInit(LPARAM lParam)
	{
		EmptyDialog::Init(lParam);
		ControlInit();
		return false; 
	}

	void ControlInit()
	{
		RECT rCtrl;
		GetClientRect(&rCtrl);
		ATLVERIFY(m_Ctrl.Create(m_hWnd, rCtrl, NULL, 
			TControlTraits::GetWndStyle(0), 
			TControlTraits::GetWndExStyle(0), ATL_IDM_WINDOW_FIRST));
		m_Ctrl.SetFocus();
	}

	bool DefSize(WPARAM wParam, LPARAM /*lParam*/)
	{
		if (m_Ctrl.IsWindow() && wParam != SIZE_MINIMIZED)
		{
			RECT rClient;
			GetClientRect(&rClient);
			m_Ctrl.MoveWindow(&rClient);
		}
		return false; 
	}

	BEGIN_MSG_MAP(CControlDialogImpl)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_RANGE_HANDLER(WM_CTLCOLOREDIT, WM_CTLCOLORSTATIC, OnCtlColor)
		MESSAGE_RANGE_HANDLER(WM_KEYFIRST, WM_KEYLAST, OnKey)
		CHAIN_MSG_MAP(EmptyDialog)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return (LRESULT)static_cast<T*>(this)->CtlColor((HDC)wParam);
	}

	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (m_Ctrl.IsWindow())
			m_Ctrl.SetFocus();
		return 0;
	}

	LRESULT OnNotify(UINT /*uMsg*/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled)
	{
		return bHandled = static_cast<T*>(this)->Notify((int) wParam, (LPNMHDR)lParam);
	}

	LRESULT OnCommand(UINT /*uMsg*/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled)
	{
		bHandled = static_cast<T*>(this)->Command(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
		return !bHandled;
	}

	LRESULT OnKey(UINT uMsg/**/, WPARAM wParam/**/, LPARAM lParam/**/, BOOL& bHandled)
	{
		return bHandled = static_cast<T*>(this)->Key(uMsg, wParam, lParam);
	}

};

///////////////////////////////////////////////////////////////////////////////
// CControlDialog - Specializable single control dialog
//

template 
	< UINT t_uIDD,		// Dialog IDD, title, icon, toolbar, menu (if matching resources exist)
	class TCtrl,		// Control class
	class TControlTraits 	// Control styles
		= ATL::CControlWinTraits,	// default for TControlTraits	
	class TControlDlgTraits		// Dialog styles
		= CControlDlgTraits		// default for TControlDlgTraits
	>
class CControlDialog : 
	public CControlDialogImpl< 
		/*this_class_t*/CControlDialog<t_uIDD, TCtrl, TControlTraits, TControlDlgTraits>, 
		t_uIDD, 
		TCtrl, 
		TControlTraits, 
		/*TDlgImpl*/CEmptyDialogImpl< 
			/*this_class_t*/CControlDialog<t_uIDD, TCtrl, TControlTraits, TControlDlgTraits> , 
			t_uIDD, 
			/*TDlgTemplate*/CEmptyDlgTemplate<t_uIDD, TControlDlgTraits> 
		> // CEmptyDialogImpl
	> // CControlDialogImpl
{
	typedef CControlDialog<t_uIDD, TCtrl, TControlTraits, TControlDlgTraits> this_class_t;
public:
	// Constructors
	CControlDialog(){}
	CControlDialog(SIZE size, bool bDLU = true) : ControlDialog(size, bDLU){}
	CControlDialog(POINT point, SIZE size, bool bDLU = true) : ControlDialog(point, size, bDLU){}
	CControlDialog(RECT rect, bool bDLU = true) : ControlDialog(rect, bDLU){}

};

////////////////////////////////////////////////////////////
// CInPlaceEditor : In place editing dialog
//
typedef ATL::CWinTraitsOR<ES_AUTOHSCROLL> CInPlaceEditTraits;

template <
	UINT t_uLength, // length of text buffer passed as lParam in DoModal() call
	class TEditCtrl // edit control class
		= CEdit,	// default for TEditCtrl
	class TEditTraits	// edit control styles
		= CInPlaceEditTraits // default for TEditTraits
	>
class CInPlaceEditor : 
	public CControlDialogImpl<
		/*this_class_t*/CInPlaceEditor<t_uLength, TEditCtrl, TEditTraits>, 
		/*t_uIDD*/t_uLength, 
		/*TCtrl*/TEditCtrl, 
		/*TCtrlTraits*/TEditTraits,
		/*TDlgImpl*/CEmptyDialogImpl<
			/*this_class_t*/CInPlaceEditor<t_uLength, TEditCtrl, TEditTraits>,
			/*t_uIDD*/t_uLength, 
			/*TDlgTemplate*/CMenuDlgTemplate
			> // CEmptyDialogImpl
		>, // CControlDialogImpl
	public CMenuDialog<CInPlaceEditor<t_uLength, TEditCtrl, TEditTraits> >
{
	typedef CInPlaceEditor<t_uLength, TEditCtrl, TEditTraits> this_class_t;
public:
	// Constructor
	CInPlaceEditor(RECT rect) : ControlDialog(rect)
	{}

	// Operation
	static bool Edit(RECT& rEdit, LPTSTR sText, HWND hwndParent = GetActiveWindow())
	{
		return this_class_t(rEdit).DoModal(hwndParent, (LPARAM)sText) == IDOK;
	}

	// Specialized
	bool Init(LPARAM lParam)
	{
		return DefInit(lParam);
	}

	// Implementation
	bool DefInit(LPARAM lParam)
	{
		m_Data = lParam;
		m_Template.Reset(); // free DLGTEMPLATE memory
		ControlInit();

		if (!m_Ctrl.GetFont())
			m_Ctrl.SetFont(AtlGetDefaultGuiFont());
		m_Ctrl.LimitText(t_uLength);

		if(lParam)
		{
			ATLASSERT(!IsBadWritePtr((LPVOID)lParam, t_uLength * sizeof(TCHAR)));
			m_Ctrl.SetWindowText((LPCTSTR)lParam);
			m_Ctrl.SetSel((INT)t_uLength, t_uLength);
		}
		return false;
	}

	HBRUSH CtlColor(HDC /*hdc*/)
	{
		return GetSysColorBrush(COLOR_HIGHLIGHT);
	}

	void Close(INT iCmd) 
	{
		if((iCmd == IDOK) && m_Data)
			m_Ctrl.GetWindowText((LPTSTR)m_Data, t_uLength);
		EmptyDialog::Close(iCmd);
	}

	// Message map
	BEGIN_MSG_MAP(CInPlaceEditor)
		CHAIN_MSG_MAP(CMenuDialog<this_class_t>)
		CHAIN_MSG_MAP(ControlDialog)
	END_MSG_MAP()

};

// Split dialog classes

///////////////////////////////////////////////////////////////////////////////
// CSplitDialogImpl - Split dialog implementation
//

typedef ATL::CWinTraitsOR<WS_THICKFRAME, 0, CControlDlgTraits> CSplitDlgTraits;

typedef ATL::CWinTraitsOR<WS_TABSTOP> CSplitControlTraits;

template<UINT t_uIDD>
class CSplitDlgTemplate : public CEmptyDlgTemplate<t_uIDD, CSplitDlgTraits>
{};

template 
	< class T,		// Actual dialog class: ie CMySplitDialog
	UINT t_uIDD,	// Dialog IDD, title, icon, toolbar, menu (if matching resources exist)
	class TSplitImpl,	// Splitter implementation class: ie WTL::CSplitterImpl<CMySplitDialog, true> 
	class TLeft,		// Left (or top) control class
	class TRight,		// Right (or bottom) control class
	class TLeftTraits 	// Left (or top) control styles
		= CSplitControlTraits,	// default for TLeftTraits
	class TRightTraits	// Right (or bottom) control styles 
		= CSplitControlTraits,	// default for TRightTraits
	class TDlgImpl	// Empty dialog base class 
		= CEmptyDialogImpl<T, t_uIDD, CSplitDlgTemplate<t_uIDD> > // default for TDlgImpl 
	>		
class ATL_NO_VTABLE CSplitDialogImpl : 
	public TDlgImpl, 
	public TSplitImpl
{
	typedef CSplitDialogImpl<T, t_uIDD, TSplitImpl, TLeft, TRight, TLeftTraits, TRightTraits, TDlgImpl> this_class_t;
public:
	typedef this_class_t SplitDialog;
	typedef TSplitImpl Splitter;
// Construction
	CSplitDialogImpl(){}
	CSplitDialogImpl(SIZE size, bool bDLU = true) : EmptyDialog(size, bDLU){}
	CSplitDialogImpl(POINT point, SIZE size, bool bDLU = true) : EmptyDialog(point, size, bDLU){}
	CSplitDialogImpl(RECT rect, bool bDLU = true) : EmptyDialog(rect, bDLU){}

	// Control members
	TLeft m_Left;	// Left or top control member
	TRight m_Right;	// Right or bottom control member

// Overrideable
	bool Init(LPARAM lParam)
	{
		return DefInit(lParam);
	}

	bool Size(WPARAM wParam, LPARAM /*lParam*/)
	{
		if(wParam != SIZE_MINIMIZED)
			Splitter::SetSplitterRect();
		return false;
	}

	bool Notify(int /*idCtrl*/, LPNMHDR pnmhdr)
	{
		return false;
	}

	HBRUSH CtlColor(HDC /*hdc*/, HWND /*hWnd*/)
	{
		return NULL;
	}

	bool Command(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
	{
		return static_cast<T*>(this)->PaneCommand(wID);
	}

// Implementation
	bool DefInit(LPARAM lParam)
	{
		EmptyDialog::Init(lParam);
		Splitter::GetSystemSettings(false);

		m_Left.Create(m_hWnd, rcDefault, NULL, TLeftTraits::GetWndStyle(0), TLeftTraits::GetWndExStyle(0), ATL_IDM_WINDOW_FIRST);
		m_Right.Create(m_hWnd, rcDefault, NULL, TRightTraits::GetWndStyle(0), TRightTraits::GetWndExStyle(0), ATL_IDM_WINDOW_LAST);
		Splitter::SetSplitterPanes(m_Left, m_Right);

		Splitter::SetSplitterRect();
		Splitter::SetSinglePaneMode();
		Splitter::SetSplitterPos();
		
		return false;// WM_INITDIALOG not handled
	}

	// Pane related commands
	bool PaneCommand(WORD wID)
	{
		switch(wID)
		{
		case ID_WINDOW_SPLIT:
			Splitter::SetSinglePaneMode(Splitter::GetSinglePaneMode() == SPLIT_PANE_NONE ? 
				Splitter::GetActivePane() : SPLIT_PANE_NONE);
			break;
		case ID_NEXT_PANE:
		case ID_PREV_PANE:
			if (Splitter::GetSinglePaneMode() != SPLIT_PANE_NONE)
				Splitter::SetSinglePaneMode(!Splitter::GetActivePane());
			else
				Splitter::ActivateNextPane();
			break;
		default:
			return false;
		}
#if defined(__ATLWINCE_H__)
		SetFocus();
#endif
		return true;
	}

	// CSplitterImpl requires this
	LRESULT DefWindowProc(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
	{
		return NULL;
	}

// Message map and handlers
	BEGIN_MSG_MAP(CSplitDialogImpl)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_RANGE_HANDLER(WM_CTLCOLOREDIT, WM_CTLCOLORSTATIC, OnCtlColor)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		CHAIN_MSG_MAP(EmptyDialog)
		CHAIN_MSG_MAP(Splitter)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		Splitter::DrawSplitterBar((HDC)wParam);
		return TRUE; // background is erased
	}

	LRESULT OnNotify(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return bHandled = static_cast<T*>(this)->Notify((int)wParam, (LPNMHDR)lParam);
	}

	LRESULT OnCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		return (LRESULT)static_cast<T*>(this)->CtlColor((HDC)wParam, (HWND)lParam);
	}

	LRESULT OnCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = static_cast<T*>(this)->Command(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
		return !bHandled;
	}

};

/////////////////////////////////////////////////////////////////
// CSplitDialog - Generic split dialog
// 

template 
	< UINT t_uIDD,	// Dialog IDD, title, icon, toolbar, menu (if matching resources exist) 
	class TSplitImpl,	// Splitter implementation class: ie WTL::CSplitterImpl<CMySplitDialog, true> 
	class TLeft,		// Left (or top) control class
	class TRight,		// Right (or bottom) control class
	class TLeftTraits 	// Left (or top) control styles
		= CSplitControlTraits,	// default for TLeftTraits
	class TRightTraits	// Right (or bottom) control styles 
		= CSplitControlTraits,	// default for TRightTraits
	class TSplitDialogTraits	// Dialog styles
		= CSplitDlgTraits	// default for TSplitDialogTraits
	>
class CSplitDialog : 
	public CSplitDialogImpl<
		/*this_class_t*/CSplitDialog<t_uIDD, TSplitImpl, TLeft, TRight, TLeftTraits, TRightTraits, TSplitDialogTraits>, 
		t_uIDD, 
		TSplitImpl, 
		TLeft, 
		TRight, 
		TLeftTraits, 
		TRightTraits,
		/*TDlgImpl*/CEmptyDialogImpl< 
			/*this_class_t*/CSplitDialog<t_uIDD, TSplitImpl, TLeft, TRight, TLeftTraits, TRightTraits, TSplitDialogTraits>, 
			t_uIDD,
			/*TDlgTemplate*/CEmptyDlgTemplate<t_uIDD, TSplitDialogTraits> 
		> // CEmptyDialogImpl 
	> // CSplitDialogImpl
{
	typedef CSplitDialog<t_uIDD, TSplitImpl, TLeft, TRight, TLeftTraits, TRightTraits, TSplitDialogTraits> this_class_t;
public:
// Constructors
	CSplitDialog(){}
	CSplitDialog(SIZE size, bool bDLU = true) : SplitDialog(size, bDLU){}
	CSplitDialog(POINT point, SIZE size, bool bDLU = true) : SplitDialog(point, size, bDLU){}
	CSplitDialog(RECT rect, bool bDLU = true) : SplitDialog(rect, bDLU){}

};

#ifdef __ATLSPLIT_H__

/////////////////////////////////////////////////////////////////
// CVSplitDialog - Vertical WTL::CSplitterImpl based split dialog
// 

template // see CSplitDialog template parameters description
	< UINT t_uIDD,
	class TLeft, class TRight, 
	class TLeftTraits = ATL::CControlWinTraits, 
	class TRightTraits = ATL::CControlWinTraits, 
	class TSplitDialogTraits = CSplitDlgTraits
	>
class CVSplitDialog : 
	public CSplitDialogImpl<
		/*this_class_t*/CVSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits, TSplitDialogTraits>, 
		t_uIDD, 
		/*TSplitImpl*/CSplitterImpl<
			/*this_class_t*/CVSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits, TSplitDialogTraits>,
			true
			>, // CSplitterImpl
		TLeft, TRight, TLeftTraits, TRightTraits,
		/*TDlgImpl*/CEmptyDialogImpl< 
			/*this_class_t*/CVSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits,TSplitDialogTraits> , 
			t_uIDD,
			/*TDlgTemplate*/CEmptyDlgTemplate<t_uIDD, TSplitDialogTraits> 
		> // CEmptyDialogImpl  
	> // CSplitDialogImpl
{
	typedef CVSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits, TSplitDialogTraits> this_class_t;
public:
// Constructors
	CVSplitDialog(){}
	CVSplitDialog(SIZE size, bool bDLU = true) : SplitDialog(size, bDLU){}
	CVSplitDialog(POINT point, SIZE size, bool bDLU = true) : SplitDialog(point, size, bDLU){}
	CVSplitDialog(RECT rect, bool bDLU = true) : SplitDialog(rect, bDLU){}

};

/////////////////////////////////////////////////////////////////
// CHSplitDialog - Horizontal WTL::CSplitterImpl based split dialog
// 

template // see CSplitDialog template parameters description 
	< UINT t_uIDD, 
	class TLeft, class TRight, 
	class TLeftTraits = ATL::CControlWinTraits, 
	class TRightTraits = ATL::CControlWinTraits, 
	class TSplitDialogTraits = CSplitDlgTraits
	>
class CHSplitDialog : 
	public CSplitDialogImpl<
		/*this_class_t*/CHSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits, TSplitDialogTraits>, 
		t_uIDD, 
		/*TSplitImpl*/CSplitterImpl<
			/*this_class_t*/CHSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits, TSplitDialogTraits>, 
			false
			>, // CSplitterImpl
		TLeft, TRight, TLeftTraits, TRightTraits,
		/*TDlgImpl*/CEmptyDialogImpl< 
			/*this_class_t*/CHSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits,TSplitDialogTraits> , 
			t_uIDD,
			/*TDlgTemplate*/CEmptyDlgTemplate<t_uIDD, TSplitDialogTraits> 
		> // CEmptyDialogImpl  
	> // CSplitDialogImpl
{
	typedef CHSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits, TSplitDialogTraits> this_class_t;
public:
// Constructors
	CHSplitDialog(){}
	CHSplitDialog(SIZE size, bool bDLU = true) : SplitDialog(size, bDLU){}
	CHSplitDialog(POINT point, SIZE size, bool bDLU = true) : SplitDialog(point, size, bDLU){}
	CHSplitDialog(RECT rect, bool bDLU = true) : SplitDialog(rect, bDLU){}

};
#endif // __ATLSPLIT_H__

#if defined __WTL_AERO_H__

// Aero enabled classes

namespace aero {

//////////////////////////////////////////////////////////
// aero::CEmptyDialogImpl - Aero enabled CEmptyDialogImpl

template
	< class T,		// Actual dialog class: ie CMyAeroEmptyDialog  
	UINT t_uIDD,	// Dialog IDD, title, icon, toolbar, menu (if matching resources exist) 
	class TDlgTemplate	// In memory dialog template
		= CEmptyDlgTemplate<t_uIDD, CEmptyDlgTraits> // default for TDlgTemplate
	>
class ATL_NO_VTABLE CEmptyDialogImpl : 
	public WTL::CEmptyDialogImpl<
	T,
	t_uIDD, 
	TDlgTemplate,
	aero::CDialogImpl<T>
	> // WTL::CEmptyDialogImpl
{
	typedef aero::CEmptyDialogImpl<T, t_uIDD, TDlgTemplate> this_class_t;
public:
	typedef aero::CDialogImpl<T> AeroDialog;
	typedef WTL::CEmptyDialogImpl<T, t_uIDD, TDlgTemplate, AeroDialog> BaseEmptyDialog;
	typedef this_class_t EmptyDialog;

// Constructors
	CEmptyDialogImpl(){}

	CEmptyDialogImpl(SIZE size, bool bDLU = true) : BaseEmptyDialog(size, bDLU)
	{}

	CEmptyDialogImpl(POINT point, SIZE size, bool bDLU = true) : BaseEmptyDialog(point, size, bDLU)
	{}

	CEmptyDialogImpl(RECT rect, bool bDLU = true) : BaseEmptyDialog(rect, bDLU)
	{}

	// Data member
	aero::CToolBarCtrl m_ATB;

	// EmptyDialog::Init() override 
	bool Init(LPARAM lParam)
	{
		bool bRes = DefInit(lParam);
		if (::IsWindow(m_hWndToolBar))
			aero::Subclass(m_ATB, m_hWndToolBar);
		return bRes;
	}

	// AeroDialog::Paint() override 
	void Paint(CDCHandle /*dc*/, RECT& /*rClient*/, RECT& /*rView*/, RECT& /*rDest*/)
	{}

	// EmptyDialog::Paint() override 
	bool Paint(HDC /*hdc*/)
	{
		return false;
	}

	// Message map
	BEGIN_MSG_MAP(CEmptyDialogImpl)
		CHAIN_MSG_MAP(AeroDialog)
		CHAIN_MSG_MAP(BaseEmptyDialog)
	END_MSG_MAP()

};

///////////////////////////////////////////////////////////////////////////////
// aero::CControlDialog - Aero enabled CControlDialog
//

template 
	< UINT t_uIDD,		// Dialog IDD, title, icon, toolbar, menu (if matching resources exist)
	class TCtrl,		// Aero enabled Control class
	class TControlTraits 	// Control styles
		= ATL::CControlWinTraits,	// default for TControlTraits	
	class TControlDlgTraits		// Dialog styles
		= CControlDlgTraits		// default for TControlDlgTraits
	>
class CControlDialog : 
	public WTL::CControlDialogImpl< 
	/*this_class_t*/aero::CControlDialog<t_uIDD, TCtrl, TControlTraits, TControlDlgTraits>, 
		t_uIDD, 
		TCtrl, 
		TControlTraits, 
		/*TDlgImpl*/aero::CEmptyDialogImpl< 
			/*this_class_t*/aero::CControlDialog<t_uIDD, TCtrl, TControlTraits, TControlDlgTraits> , 
			t_uIDD, 
			/*TDlgTemplate*/CEmptyDlgTemplate<t_uIDD, TControlDlgTraits> 
		> // aero::CEmptyDialogImpl
	> // WTL::CControlDialogImpl
{
	typedef aero::CControlDialog<t_uIDD, TCtrl, TControlTraits, TControlDlgTraits> this_class_t;
public:
	// Constructors
	CControlDialog(){}
	CControlDialog(SIZE size, bool bDLU = true) : ControlDialog(size, bDLU){}
	CControlDialog(POINT point, SIZE size, bool bDLU = true) : ControlDialog(point, size, bDLU){}
	CControlDialog(RECT rect, bool bDLU = true) : ControlDialog(rect, bDLU){}

};

/////////////////////////////////////////////////////////////////
// aero::CSplitDialog - Aero enabled CSplitDialog
// 

template 
	< UINT t_uIDD,	// Dialog IDD, title, icon, toolbar, menu (if matching resources exist) 
	class TSplitImpl,	// Aero enabled splitter implementation class: ie aero::CSplitterImpl<CMySplitDialog, true> 
	class TLeft,		// Aero enabled reft (or top) control class
	class TRight,		// Aero enabled right (or bottom) control class
	class TLeftTraits 	// Left (or top) control styles
		= CSplitControlTraits,	// default for TLeftTraits
	class TRightTraits	// Right (or bottom) control styles 
		= CSplitControlTraits,	// default for TRightTraits
	class TSplitDialogTraits	// Dialog styles
		= CSplitDlgTraits	// default for TSplitDialogTraits
	>
class CSplitDialog : 
	public WTL::CSplitDialogImpl<
	/*this_class_t*/aero::CSplitDialog<t_uIDD, TSplitImpl, TLeft, TRight, TLeftTraits, TRightTraits, TSplitDialogTraits>, 
		t_uIDD, 
		TSplitImpl, 
		TLeft, 
		TRight, 
		TLeftTraits, 
		TRightTraits,
		/*TDlgImpl*/aero::CEmptyDialogImpl< 
		/*this_class_t*/aero::CSplitDialog<t_uIDD, TSplitImpl, TLeft, TRight, TLeftTraits, TRightTraits, TSplitDialogTraits>, 
			t_uIDD,
			/*TDlgTemplate*/CEmptyDlgTemplate<t_uIDD, TSplitDialogTraits> 
		> // aero::CEmptyDialogImpl 
	> // WTL::CSplitDialogImpl
{
	typedef aero::CSplitDialog<t_uIDD, TSplitImpl, TLeft, TRight, TLeftTraits, TRightTraits, TSplitDialogTraits> this_class_t;
public:
// Constructors
	CSplitDialog(){}
	CSplitDialog(SIZE size, bool bDLU = true) : SplitDialog(size, bDLU){}
	CSplitDialog(POINT point, SIZE size, bool bDLU = true) : SplitDialog(point, size, bDLU){}
	CSplitDialog(RECT rect, bool bDLU = true) : SplitDialog(rect, bDLU){}

};

}; // namespace aero

#endif // defined __WTL_AERO_H__

// Windows Mobile classes

#if defined(__ATLWINCE_H__) && !defined(_WTL_CE_NO_DIALOGS)

///////////////////////////////////////////////////////////////////////////////
// CStdEmptyDialogImpl - base class for Mobile Device standard empty dialog classes
// 

typedef ATL::CWinTraits<WS_VISIBLE | DS_CENTER | WS_POPUP> CStdEmptyDlgTraits;

template <UINT t_uIDS>
class CStdEmptyDlgTemplate : public CEmptyDlgTemplate<t_uIDS, CStdEmptyDlgTraits>
{};

template
	< class T,		// Actual dialog class: ie CMyStdEmptyDialog 
	UINT t_uIDD,	// Dialog IDD, title, MenuBar (if matching resources exist) 
	UINT t_shidiFlags	// Position flags for ::SHInitDialog() 
		= WTL_STD_SHIDIF,	// default for t_shidiFlags 
	class TDlgTemplate	// In memory dialog template 
		= CStdEmptyDlgTemplate<t_uIDD> // default for TDlgTemplate 
	>
class ATL_NO_VTABLE CStdEmptyDialogImpl : 
	public CEmptyDialogImpl<
		T, 
		t_uIDD, 
		TDlgTemplate, 
		/*TBase*/CStdDialogImpl<T, t_shidiFlags, true> 
	> // CEmptyDialogImpl
{
	typedef CStdEmptyDialogImpl<T, t_uIDD, t_shidiFlags, TDlgTemplate> this_class_t;
public:
	typedef CStdDialogImpl<T, t_shidiFlags, true> Std;
	//typedef CStdDialogImpl<this_class_t, t_shidiFlags, true> Std;
	typedef this_class_t EmptyDialog;
	typedef CEmptyDialogImpl<T, t_uIDD, TDlgTemplate, Std> BaseEmptyDialog;
	//typedef CEmptyDialogImpl<this_class_t, t_uIDD, TDlgTemplate, Std> BaseEmptyDialog;

	// Constructor
	CStdEmptyDialogImpl() 
	{
		m_Data = 0;
		SIZE size = {0};
		POINT pt = {0};
		m_Template.Create(pt, size);
	}

	// Formal unimplemented constructors
	CStdEmptyDialogImpl(SIZE size, bool bDLU = true);
	CStdEmptyDialogImpl(POINT point, SIZE size, bool bDLU = true);
	CStdEmptyDialogImpl(RECT rect, bool bDLU = true);

// Overrideable
	bool Init(LPARAM lParam)
	{
		return DefInit(lParam);
	}

// Implementation
	bool DefInit(LPARAM lParam, UINT uiMB = 0, int nBmpImages = 0)
	{
#ifdef _DEBUG
		ATLASSERT(static_cast<T*>(this)->m_bModal);
#endif
		m_Data = lParam;
		m_Template.Reset(); // free DLGTEMPLATE memory

		if (uiMB == 0)
			uiMB = ::FindResource(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(t_uIDD), RT_RCDATA) ? t_uIDD : ATL_IDM_MENU_DONECANCEL;
		CreateMenuBar(uiMB, nBmpImages);

#if defined(WIN32_PLATFORM_WFSP)
		StdSPInit();
#endif
		StdShidInit();
		return false; 
	}

#ifdef WIN32_PLATFORM_PSPC // WTL 8.0 bug fix
	BOOL GetClientRect(LPRECT lpRect) 
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(pT->IsWindow());
		BOOL bRes = ::GetClientRect(pT->m_hWnd, lpRect);
		if (nTitleHeight)
			lpRect->top += nTitleHeight + 1;
		return bRes;
	}
#endif

	// MenuBar command buttons helpers
#if _WIN32_WCE > 0x500 || defined WIN32_PLATFORM_PSPC // All platforms except SmartPhone 2003
	BOOL SetMenuBarCommand(INT iID, LPCTSTR psText, bool bRight = false)
	{
		CMenuBarCtrl mb = ::SHFindMenuBar(m_hWnd);
		ATLASSERT(mb.IsWindow());
		TBBUTTONINFO tbbi = {sizeof(TBBUTTONINFO), TBIF_TEXT | TBIF_COMMAND, iID};
		tbbi.pszText = (LPTSTR)psText;

#if _WIN32_WCE > 0x500
		tbbi.dwMask |= TBIF_BYINDEX;
		return mb.SetButtonInfo(bRight, &tbbi);
#else
		TBBUTTON tbb = {0};
		ATLVERIFY(mb.GetButton(bRight, &tbb));
		return mb.SetButtonInfo(tbb.idCommand, &tbbi);
#endif // _WIN32_WCE > 0x500
	}

	BOOL SetMenuBarCommand(INT iID, bool bRight = false)
	{
		return SetMenuBarCommand(iID, AtlLoadString(iID), bRight);
	}
#endif // _WIN32_WCE > 0x500 || defined WIN32_PLATFORM_PSPC

	BOOL SetMenuBarCommand(UINT uOldId, UINT uNewId, LPTSTR sText = NULL)
	{
		CMenuBarCtrl mb = ::SHFindMenuBar(m_hWnd);
		ATLASSERT(mb.IsWindow());

		TBBUTTONINFO tbbi = {sizeof(TBBUTTONINFO), TBIF_TEXT | TBIF_COMMAND, uNewId};
		tbbi.pszText = sText ? sText : (LPTSTR)AtlLoadString(uNewId);

		return mb.SetButtonInfo(uOldId, &tbbi);
	}

	// Message map and local handlers
	BEGIN_MSG_MAP(CStdEmptyDialogImpl)
		MESSAGE_ANSWER(WM_ERASEBKGND, TRUE)
		MESSAGE_HANDLER(WM_INITDIALOG, /*BaseEmptyDialog::*/OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, /*BaseEmptyDialog::*/OnSize)
		MESSAGE_HANDLER(WM_PAINT, /*this_class_t::*/OnPaint)
#ifdef WIN32_PLATFORM_PSPC 
		MESSAGE_HANDLER(WM_SETTINGCHANGE, /*Std::*/OnSettingChange)
#elif defined(WIN32_PLATFORM_WFSP) 
		MESSAGE_HANDLER(WM_HOTKEY, /*Std::*/OnHotKey)
		MESSAGE_ANSWER(WM_GETDLGCODE, DLGC_WANTALLKEYS)
#endif
		COMMAND_RANGE_HANDLER(IDOK, IDCANCEL, /*BaseEmptyDialog::*/OnCloseCommand)
		COMMAND_RANGE_HANDLER(ID_MENU_OK, ID_MENU_CANCEL, /*this_class_t::*/OnMenuClose)
	END_MSG_MAP()

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
#ifdef WIN32_PLATFORM_PSPC
		if(nTitleHeight)
			pT->DoPaintTitle();
#endif
		return bHandled = pT->Paint((HDC)wParam);
	}

	LRESULT OnMenuClose(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		static_cast<T*>(this)->Close(wID - ID_MENU_OK + IDOK);
		return 0;
	}

};

///////////////////////////////////////////////////////////////////////////////
// CStdControlDialog - Mobile Device specializable standard single control dialog class
// 

template 
	< UINT t_uIDD,	// Dialog IDD, title, MenuBar (if matching resources exist)   
	class TCtrl,	// Control class 
	class TControlTraits	// Control styles
		= ATL::CControlWinTraits,	// default for TControlTraits 
	UINT t_shidiFlags	// Position flags for ::SHInitDialog() 
		= WTL_STD_SHIDIF,	// default for t_shidiFlags 
	class TDlgTemplate	// In memory dialog template 
		= CStdEmptyDlgTemplate<t_uIDD> // default for TDlgTemplate 
	>
class CStdControlDialog : 
	public CControlDialogImpl< 
		/*this_class_t*/CStdControlDialog<t_uIDD, TCtrl, TControlTraits, t_shidiFlags, TDlgTemplate>, 
		t_uIDD, 
		TCtrl, 
		TControlTraits,
		/*TDlgImpl*/CStdEmptyDialogImpl<
			/*this_class_t*/CStdControlDialog<t_uIDD, TCtrl, TControlTraits, t_shidiFlags, TDlgTemplate> , 
			t_uIDD, 
			t_shidiFlags, 
			TDlgTemplate
			> //  CStdEmptyDialogImpl 
		> // CControlDialogImpl
{
	typedef CStdControlDialog<t_uIDD, TCtrl, TControlTraits, t_shidiFlags, TDlgTemplate> this_class_t;
};


///////////////////////////////////////////////////////////////////////////////
// CStdSplitDialog - Mobile Device specializable split standard dialog class
// 

typedef ATL::CWinTraitsOR<0, WS_EX_CLIENTEDGE, CStdEmptyDlgTraits> CStdSplitDlgTraits;

template <UINT t_uIDS>
class CStdSplitDlgTemplate : public CEmptyDlgTemplate<t_uIDS, CStdSplitDlgTraits>
{};

template 
	< UINT t_uIDD,	// Dialog IDD, title, MenuBar (if matching resources exist)  
	class TSplitImpl,	// Splitter implementation class: ie WTL::CSplitterImpl<CMyStdSplitDialog, true> 
	class TLeft,		// Left (or top) control class
	class TRight,		// Right (or bottom) control class
	class TLeftTraits 	// Left (or top) control styles
		= CSplitControlTraits,	// default for TLeftTraits
	class TRightTraits	// Right (or bottom) control styles 
		= CSplitControlTraits,	// default for TRightTraits
	UINT t_shidiFlags	// Position flags for ::SHInitDialog()
		= WTL_STD_SHIDIF,	// default for t_shidiFlags
	class TDlgTemplate	// In memory dialog template 
		= CStdSplitDlgTemplate<t_uIDD> // default for TDlgTemplate 
	>
class CStdSplitDialog : 
	public CSplitDialogImpl<
		/*this_class_t*/CStdSplitDialog<t_uIDD, TSplitImpl, TLeft, TRight, TLeftTraits, TRightTraits, t_shidiFlags, TDlgTemplate>, 
		t_uIDD, 
		TSplitImpl, 
		TLeft, 
		TRight, 
		TLeftTraits, 
		TRightTraits, 
		/*TDlgImpl*/CStdEmptyDialogImpl <
			/*this_class_t*/CStdSplitDialog<t_uIDD, TSplitImpl, TLeft, TRight, TLeftTraits, TRightTraits, t_shidiFlags, TDlgTemplate>, 
			t_uIDD, 
			t_shidiFlags, 
			TDlgTemplate
			> //  CStdEmptyDialogImpl
		> // CSplitDialogImpl
{
	typedef CStdSplitDialog<t_uIDD, TSplitImpl, TLeft, TRight, TLeftTraits, TRightTraits, t_shidiFlags, TDlgTemplate> this_class_t;
};

#ifdef __ATLSPLIT_H__

/////////////////////////////////////////////////////////////////
// CStdVSplitDialog - Vertical WTL::CSplitterImpl based Mobile split dialog
//

template // see CStdSplitDialog template parameters description 
	< UINT t_uIDD, 
	class TLeft, class TRight, 
	class TLeftTraits = CSplitControlTraits, 
	class TRightTraits = CSplitControlTraits, 
	UINT t_shidiFlags = WTL_STD_SHIDIF,
	class TDlgTemplate = CStdSplitDlgTemplate<t_uIDD>
	>
class CStdVSplitDialog : 
	public CSplitDialogImpl<
		/*this_class_t*/CStdVSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits, t_shidiFlags, TDlgTemplate>, 
		t_uIDD, 
		/*TSplitImpl*/CSplitterImpl<
			/*this_class_t*/CStdVSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits, t_shidiFlags, TDlgTemplate>, 
			true
			>, // CSplitterImpl 
		TLeft, 
		TRight, 
		TLeftTraits, 
		TRightTraits, 
		/*TDlgImpl*/CStdEmptyDialogImpl <
			/*this_class_t*/CStdVSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits, t_shidiFlags, TDlgTemplate>, 
			t_uIDD, 
			t_shidiFlags, 
			TDlgTemplate
			> // CEmptyDialogImpl 
		> // CSplitDialogImpl
{
	typedef CStdVSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits, t_shidiFlags, TDlgTemplate> this_class_t;
};

/////////////////////////////////////////////////////////////////
// CStdHSplitDialog - Horizontal WTL::CSplitterImpl based Mobile split dialog
//

template // see CStdSplitDialog template parameters description 
	< UINT t_uIDD, 
	class TLeft, class TRight, 
	class TLeftTraits = CSplitControlTraits, 
	class TRightTraits = CSplitControlTraits, 
	UINT t_shidiFlags = WTL_STD_SHIDIF,
	class TDlgTemplate = CStdSplitDlgTemplate<t_uIDD>
	>
class CStdHSplitDialog : 
	public CSplitDialogImpl<
		/*this_class_t*/CStdHSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits, t_shidiFlags, TDlgTemplate>, 
		t_uIDD, 
		/*TSplitImpl*/CSplitterImpl<
			/*this_class_t*/CStdHSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits, t_shidiFlags, TDlgTemplate>, 
			false
			>, // CSplitterImpl 
		TLeft, 
		TRight, 
		TLeftTraits, 
		TRightTraits, 
		/*TDlgImpl*/CStdEmptyDialogImpl <
			/*this_class_t*/CStdHSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits, t_shidiFlags, TDlgTemplate>, 
			t_uIDD, 
			t_shidiFlags, 
			TDlgTemplate
			> // CEmptyDialogImpl 
		> // CSplitDialogImpl
{
	typedef CStdHSplitDialog<t_uIDD, TLeft, TRight, TLeftTraits, TRightTraits, t_shidiFlags, TDlgTemplate> this_class_t;
};

#endif // __ATLSPLIT_H__

#endif // defined(__ATLWINCE_H__) && !defined(_WTL_CE_NO_DIALOGS)


}; // namespace WTL

#endif // __ATLDLGX_H__
