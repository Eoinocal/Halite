/** \mainpage
 *	\subpage atlautosizedlg See atlautosizedlg for infos and the license of atlautosizedlg.h\n
 */

/** \page atlautosizedlg atlautosizedlg.h
 *
 *  \section License License
 *  Copyright (c) 2006 Massimiliano Alberti <xanatos@geocities.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated Subjectation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject
 *  to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  \section VersionHistory Version History
 *  \li atlautosizedlg.h  1.0   Apr 2006  Initial Release
 *  \li atlautosizedlg.h  2.0   Jun 2006  Nearly rebuilt. Now supports multilevel
 *
 *  \section TODO TODO
 *
 */

/** \file
 *  This header contains classes useful to reorder controls in a window
 */

#pragma once

#ifndef __ATLAUTOSIZEDLG_H__
#define __ATLAUTOSIZEDLG_H__

#ifndef __ATLWIN_H__
#error atlautosizedlg.h requires atlwin.h to be included first
#endif

#define TRACEAUTOSIZE

//

namespace ATL
{

//

// #define TRACEAUTOSIZE ///< Define this to see in the Output window the tracing of the base size of the cols and rows
// #define TRACEAUTOSIZE2 ///< Define this to see in the Output window the tracing of the calced size of the cols and rows

//

/// \name Transparent ctrls (Group Boxes and Static Controls)
/*@{*/

/// This "facility" is used to make the redraw of transparent controls (only Group Boxes and Static Controls) smoother. 
/** Simply list the IDs of the controls in the ... part of the TRANSPARENT_LIST()
 *  \param theClass Name of the class using the facility
 *  \param ... List of IDs of controls that are transparent. They'll be redrawn in a special way
 */
#define TRANSPARENT_LIST(theClass, ...) \
	BOOL HandleTransparentMap(WTL::CDCHandle dc = NULL) \
	{ \
		int tcCtrls[] = {0, __VA_ARGS__, INT_MAX}; \
		if (dc) { \
			int *ptcCtrl = tcCtrls + 1; \
			while (*ptcCtrl != INT_MAX) { \
				DrawTransparentCtrl(dc, *ptcCtrl); \
				ptcCtrl++; \
			} \
		} else { \
			int *ptcCtrl = tcCtrls + 1; \
			while (*ptcCtrl != INT_MAX) { \
				InitializeTransparentCtrl(*ptcCtrl); \
				ptcCtrl++; \
			} \
		} \
		return TRUE; \
	}

/// The main "facility" to control the positioning and sizing of the ctrls.
/** The BEGIN_WINDOW_MAP() MUST be paired by a END_WINDOW_MAP(). 
 *  \param theClass Name of the class using the facility
 *  \param ... In order: LeftRightBorder, TopBottomBorder, HorizontalGap, VerticalGap. Positive if DLGUnits, negative for Pixels
 */
 
#define BEGIN_WINDOW_MAP_INLINE(theClass, /*LeftRightBorder, TopBottomBorder, HGap, VGap*/...) \
	ATLASSERT(sizeof(int) == sizeof(theClass::CCtrlCounter) && "\"Strange\" compiler. The library is not compatible"); \
		CRowsIndex *p = NULL; p; \
		const int _auto = WMSRC_AUTO, _exp = WMSRC_EXPANDABLE, _contr = WMSRC_CONTRACTABLE, _eq = WMSRC_EQUAL, _gap = WMSRC_GAP, _gapm = WMSRC_GAPM, _nog = WMSRC_NOGAP, _ = WMSCTRL_EMPTY, __ = WMSCTRL_EMPTY, _r = WMSCTRL_EXPRIGHT, _d = WMSCTRL_EXPDOWN; \
		_auto; _exp; _contr; _eq; _gap, _gapm, _nog; _; __; _r; _d; \
		static CCtrlCounter s_iCtrls[] = {0, _FirstParam(__VA_ARGS__), _SecondParam(__VA_ARGS__), _ThirdParam(__VA_ARGS__), _FourthParam(__VA_ARGS__), 

#define BEGIN_WINDOW_MAP(theClass, /*LeftRightBorder, TopBottomBorder, HGap, VGap*/...) \
	static CWindowMapStruct* GetWindowMap() \
	{ \
		ATLASSERT(sizeof(int) == sizeof(CCtrlCounter) && "\"Strange\" compiler. The library is not compatible"); \
		CRowsIndex *p = NULL; p; \
		const int _auto = WMSRC_AUTO, _exp = WMSRC_EXPANDABLE, _contr = WMSRC_CONTRACTABLE, _eq = WMSRC_EQUAL, _gap = WMSRC_GAP, _gapm = WMSRC_GAPM, _nog = WMSRC_NOGAP, _ = WMSCTRL_EMPTY, __ = WMSCTRL_EMPTY, _r = WMSCTRL_EXPRIGHT, _d = WMSCTRL_EXPDOWN; \
		_auto; _exp; _contr; _eq; _gap, _gapm, _nog; _; __; _r; _d; \
		static CCtrlCounter s_iCtrls[] = {0, _FirstParam(__VA_ARGS__), _SecondParam(__VA_ARGS__), _ThirdParam(__VA_ARGS__), _FourthParam(__VA_ARGS__), 

/// "Footer" of the Window Map.
#define END_WINDOW_MAP_INLINE() \
		}; ATLASSERT(!p); \
		return (CWindowMapStruct*)s_iCtrls; 
		
		
/// "Footer" of the Window Map.
#define END_WINDOW_MAP() \
		}; ATLASSERT(!p); \
		return (CWindowMapStruct*)s_iCtrls; \
	}


/*@}*/

//

/// \name Ctrl group header and footer
/*@{*/

/// "Header" of a rect ctrl group. MUST be paired with a WMB_END()
/** \param ... List of WMB_COL() columns comma separated
 */
#define WMB_HEAD(...)	(_Init(p, (CRowsIndex*)_alloca(sizeof(CRowsIndex))), _Init(p, (CRowsIndex*)_alloca(sizeof(CRowsIndex))), _Init(p, (CRowsIndex*)_alloca(sizeof(CRowsIndex))), WMH_BEGIN), \
							CCtrlCounter(p->m_pPrev->m_pPrev) /* Size */, CCtrlCounter(p->m_pPrev) /* NumRows */, 0 /*NumCtrls*/, \
							0 /*ColWidthFixed*/, 0 /*ColWidthMin*/, 0 /*ColWidthMax*/, 0 /*ColExpand*/, 0 /*ColContract*/, \
							0 /*RowHeightFixed*/, 0 /*RowHeightMin*/, 0 /*RowHeightMax*/, 0 /*RowExpand*/, 0 /*RowContract*/, \
							CCtrlCounter(p) /* NumCols */, __VA_ARGS__, CCtrlCounter(p, 1, 3, WMH_END)

/// "Footer" of a rect ctrl group. MUST be paired with a WMB_HEAD()
#define WMB_END()		CCtrlCounter(p, -1, 1, _DeInit(p, WM_END))

/*@}*/

//

/// \name Ctrl group Cols
/*@{*/

/// Auto-min and auto-max column. To be used with WMB_HEAD()
/** \param _size The size and type of the column. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 */
#define WMB_COL(_size)							_size, WMSRCMM_SIZECALC, WMSRCMM_SIZECALC

/// Fixed-min and auto-max column. To be used with WMB_HEAD()
/** \param _size The size and type of the column. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 *	\param _min Min-size of the column. Positive if DLGUnits, negative for Pixels
 */
#define WMB_COLMIN(_size, _min)					_size, _min, WMSRCMM_SIZECALC

/// Auto-min and fixed-max column. To be used with WMB_HEAD()
/** \param _size The size and type of the column. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 *	\param _max Max-size of the column. Positive if DLGUnits, negative for Pixels
 */
#define WMB_COLMAX(_size, _max)					_size, WMSRCMM_SIZECALC, _max

/// Fixed-min and fixed-max column. To be used with WMB_HEAD()
/** \param _size The size and type of the column. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 *	\param _min Min-size of the column. Positive if DLGUnits, negative for Pixels
 *	\param _max Max-size of the column. Positive if DLGUnits, negative for Pixels
 */
#define WMB_COLMINMAX(_size, _min, _max)		_size, _min, _max

/// 0-min and auto-max column. To be used with WMB_HEAD()
/** \param _size The size and type of the column. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 */
#define WMB_COLNOMIN(_size)						_size, 0, WMSRCMM_SIZECALC

/// Auto-min and infinite-max column. To be used with WMB_HEAD()
/** \param _size The size and type of the column. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 */
#define WMB_COLNOMAX(_size)						_size, WMSRCMM_SIZECALC, WMSRCMM_MAXVAL

/// 0-min and infinite-max column. To be used with WMB_HEAD()
/** \param _size The size and type of the column. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 */
#define WMB_COLNOMINNOMAX(_size)				_size, 0, WMSRCMM_MAXVAL

/// Fixed-min and infinite-max column. To be used with WMB_HEAD()
/** \param _size The size and type of the column. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 *	\param _min Min-size of the column. Positive if DLGUnits, negative for Pixels
 */
#define WMB_COLMINNOMAX(_size, _min)			_size, _min, WMSRCMM_MAXVAL

/// 0-min and fixed-max column. To be used with WMB_HEAD()
/** \param _size The size and type of the column. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 *	\param _max Max-size of the column. Positive if DLGUnits, negative for Pixels
 */
#define WMB_COLNOMINMAX(_size, _max)			_size, 0, _max
/*@}*/

//

/// \name Ctrl group Rows
/*@{*/

/// Auto-min and auto-max row. To be used with WMB_HEAD()
/** \param _size The size and type of the row. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 *  \param ... Comma separated list of IDs if ctrls (or WMB_HEAD()/WMB_END() ctrl groups, or WMSCTRL_EMPTY/_/__ or WMSCTRL_EXPRIGHT/_r or WMSCTRL_EXPDOWN/_d)
 */
#define WMB_ROW(_size, ...)						(_IncRow(p), WMR_BEGIN), _size, WMSRCMM_SIZECALC, WMSRCMM_SIZECALC, __VA_ARGS__, WMR_END

/// Fixed-min and auto-max row. To be used with WMB_HEAD()
/** \param _size The size and type of the row. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 *	\param _min Min-size of the row. Positive if DLGUnits, negative for Pixels
 *  \param ... Comma separated list of IDs if ctrls (or WMB_HEAD()/WMB_END() ctrl groups, or WMSCTRL_EMPTY/_/__ or WMSCTRL_EXPRIGHT/_r or WMSCTRL_EXPDOWN/_d)
 */
#define WMB_ROWMIN(_size, _min, ...)			(_IncRow(p), WMR_BEGIN), _size, _min, WMSRCMM_SIZECALC, __VA_ARGS__, WMR_END

/// Auto-min and fixed-max row. To be used with WMB_HEAD()
/** \param _size The size and type of the row. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 *	\param _max Max-size of the row. Positive if DLGUnits, negative for Pixels
 *  \param ... Comma separated list of IDs if ctrls (or WMB_HEAD()/WMB_END() ctrl groups, or WMSCTRL_EMPTY/_/__ or WMSCTRL_EXPRIGHT/_r or WMSCTRL_EXPDOWN/_d)
 */
#define WMB_ROWMAX(_size, _max, ...)			(_IncRow(p), WMR_BEGIN), _size, WMSRCMM_SIZECALC, _max, __VA_ARGS__, WMR_END

/// Fixed-min and fixed-max row. To be used with WMB_HEAD()
/** \param _size The size and type of the row. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 *	\param _min Min-size of the row. Positive if DLGUnits, negative for Pixels
 *	\param _max Max-size of the row. Positive if DLGUnits, negative for Pixels
 *  \param ... Comma separated list of IDs if ctrls (or WMB_HEAD()/WMB_END() ctrl groups, or WMSCTRL_EMPTY/_/__ or WMSCTRL_EXPRIGHT/_r or WMSCTRL_EXPDOWN/_d)
 */
#define WMB_ROWMINMAX(_size, _min, _max, ...)	(_IncRow(p), WMR_BEGIN), _size, _min, _max, __VA_ARGS__, WMR_END

/// 0-min and auto-max row. To be used with WMB_HEAD()
/** \param _size The size and type of the row. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 *  \param ... Comma separated list of IDs if ctrls (or WMB_HEAD()/WMB_END() ctrl groups, or WMSCTRL_EMPTY/_/__ or WMSCTRL_EXPRIGHT/_r or WMSCTRL_EXPDOWN/_d)
 */
#define WMB_ROWNOMIN(_size, ...)						(_IncRow(p), WMR_BEGIN), _size, 0, WMSRCMM_SIZECALC, __VA_ARGS__, WMR_END

/// Auto-min and infinite-max row. To be used with WMB_HEAD()
/** \param _size The size and type of the row. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 *  \param ... Comma separated list of IDs if ctrls (or WMB_HEAD()/WMB_END() ctrl groups, or WMSCTRL_EMPTY/_/__ or WMSCTRL_EXPRIGHT/_r or WMSCTRL_EXPDOWN/_d)
 */
#define WMB_ROWNOMAX(_size, ...)						(_IncRow(p), WMR_BEGIN), _size, WMSRCMM_SIZECALC, WMSRCMM_MAXVAL, __VA_ARGS__, WMR_END

/// 0-min and infinite-max row. To be used with WMB_HEAD()
/** \param _size The size and type of the row. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 *  \param ... Comma separated list of IDs if ctrls (or WMB_HEAD()/WMB_END() ctrl groups, or WMSCTRL_EMPTY/_/__ or WMSCTRL_EXPRIGHT/_r or WMSCTRL_EXPDOWN/_d)
 */
#define WMB_ROWNOMINNOMAX(_size, ...)				(_IncRow(p), WMR_BEGIN), _size, 0, WMSRCMM_MAXVAL, __VA_ARGS__, WMR_END

/// Fixed-min and infinite-max row. To be used with WMB_HEAD()
/** \param _size The size and type of the row. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 *	\param _min Min-size of the row. Positive if DLGUnits, negative for Pixels
 *  \param ... Comma separated list of IDs if ctrls (or WMB_HEAD()/WMB_END() ctrl groups, or WMSCTRL_EMPTY/_/__ or WMSCTRL_EXPRIGHT/_r or WMSCTRL_EXPDOWN/_d)
 */
#define WMB_ROWMINNOMAX(_size, _min, ...)			(_IncRow(p), WMR_BEGIN), _size, _min, WMSRCMM_MAXVAL, __VA_ARGS__, WMR_END

/// 0-min and fixed-max row. To be used with WMB_HEAD()
/** \param _size The size and type of the row. For example WMSRC_AUTO/_auto or WMSRC_EXPANDABLE/_exp
 *	\param _max Max-size of the row. Positive if DLGUnits, negative for Pixels
 *  \param ... Comma separated list of IDs if ctrls (or WMB_HEAD()/WMB_END() ctrl groups, or WMSCTRL_EMPTY/_/__ or WMSCTRL_EXPRIGHT/_r or WMSCTRL_EXPDOWN/_d)
 */
#define WMB_ROWNOMINMAX(_size, _max, ...)			(_IncRow(p), WMR_BEGIN), _size, 0, _max, __VA_ARGS__, WMR_END

/*@}*/

//

/// \name User constants (format of columns/rows)
/*@{*/
#define WMSRC_AUTO				0x40000000L		///< [_auto] Use the maximum width/height of the controls in the column/row as the size. No optional parameter
#define WMSRC_EXPANDABLE		0x30000000L		///< [_exp] The column/row will be proportionally expanded. Optional parameter: "weight" of the column/row (DEFAULT: 100)
#define WMSRC_CONTRACTABLE		0x20000000L		///< [_contr] If all the other rows/columns are already maxed out, the remaining space will be divided proportionally between Contractable columns. Optional parameter: "weight" of the column/row (DEFAULT: 100)
#define WMSRC_EQUAL				0x10000000L		///< [_eq] The column/row will have the same width/height and the same min/max width/height of another column/row. Necessary parameter: the number of the column/row (zero based. The first column/row is the number 0)
#define WMSRC_GAP				0x07000000L		///< [_gap] The column/row will be GAP wide (usefull for some tricks). Optional parameter: "extra" width/height ADDED to the column. Positive if DLGUnits, negative (remember the ^) for Pixels
#define WMSRC_GAPM				0x06000000L		///< [_gapm] The column/row will be GAP wide (usefull for some tricks). Optional parameter: "extra" width/height SUBTRACTED (minimum 0) to the column. Positive if DLGUnits, negative (remember the ^) for Pixels
#define WMSRC_FIXED				0x00000000L		///< [(nothing)] The width/height of the column is fixed. Necessary parameter: the width/height of the column/row. Positive if DLGUnits, negative (remember the ^) for Pixels. Note that you don't need to use WMSRC_FIXED because it's 0

#define WMSRC_NOGAP				0x08000000L		///< [_nog] (this is a special flag to add to the other flags). Do not prepend the usual gap before this column/row. The first column/row doesn't have a gap
/*@}*/

/// \name Internal constants (format of columns/rows)
/*@{*/
#define WMSRC_TYPEMASK			0x77000000L		///< Extracts the Type of a column/row descriptor
#define WMSRC_EXSTYLEMASK		0x08000000L		///< Extracts the Extended Style (WMSRC_NOGAP) of a column/row descriptor
#define WMSRC_VALMASK			0x80FFFFFFL		///< Extracts the Parameter of a column/row descriptor. Note that if the parameter is negative then extra care will be needed for the Type and ExStyle

//

#define WMSRCMM_SIZECALC			0x40000000L		///< The minimum and/or maximum width/height of a column/row will be auto-calced by the library
#define WMSRCMM_NEEDCALC			0x20000000L		///< Internally used!

#define WMSRCMM_TYPEMASK			0x70000000L		///< Extracts the Type of a min/max column/row descriptor
#define WMSRCMM_VALMASK				0x8FFFFFFFL		///< Extracts the Value of a min/max column/row descriptor. Positive if DLGUnits, negative (remember the ^) for Pixels

#define WMSRCMM_MAXVAL				0x0FFFFFFFL		///< The maximum width/height of a min/max column/row descriptor ("infinite" width/height)

//

#define WMSRC_DEFEXPANDABLEVAL		100			///< The default Expandable "weight"
#define WMSRC_DEFCONTRACTABLEVAL	100			///< The default Contractable "weight"
/*@}*/

//

/// \name Internally used signatures
/*@{*/
#define WMH_BEGIN			0xFFFFFFFFL		///< Header Begin
#define WMH_END				0xFFFFFFFEL		///< Header End

#define WMR_BEGIN			0xFFFFFFFDL		///< Row Begin
#define WMR_END				0xFFFFFFFCL		///< Row End

#define WM_END				0xFFFFFFFBL		///< Ctrl Group End
/*@}*/

//

/// \name User constants (cell format)
/*@{*/
#define WMSCTRL_EXPRIGHT		0x40000001L		///< [_r] The ctrl on the left of this cell will continue in this cell
#define WMSCTRL_EXPDOWN			0x40000002L		///< [_d] The ctrl on the top of this cell will continue in this cell

#define WMSCTRL_EMPTY			0x00000000L		///< [_ & __] The cell is empty (you could directly use 0, but it would be more ugly to debug)
/*@}*/

/// \name Internal constants (cell format)
/*@{*/
#define WMSCTRL_CONTINUEMASK	0x40000000L		///< "continue" mask (can be used to check for a continue)

#define WMSCTRL_TYPEMASK		0x70000000L		///< Extracts the Type of a ctrl
#define WMSCTRL_VALMASK			0x8FFFFFFFL		///< Extracts the ID of a ctrl
/*@}*/

//

/// A no overflow integer class 
/** It's probably quite slow because I wasn't able to use the overflow 
 *  flag of the processor so I had to use int64s. I hope the compiler
 *  will optimize it. The class has the same size as an int and is an int
 *  with some operators overloaded. Only sums, subtractions and comparisons
 *  are supported.
 */
class int_no
{
public:
	int m_iVal;		///< The integer value

public:
	/// Constructor
	/** \param iVal Base value
	 */
	int_no(int iVal = 0) : m_iVal(iVal)
	{
	}

	/// Pointer to the integer
	operator int*()
	{
		return &m_iVal;
	}

	/// Reference to the integer
	operator int&() 
	{
		return m_iVal;
	}

	/// "=" operator
	/** \param iVal New value
	 */
	int_no& operator=(int iVal)
	{
		m_iVal = iVal;
		return *this;
	}

	/// Equal operator
	/** \param iVal Value to be compared with
	 */
	bool operator ==(int iVal) const
	{
		return m_iVal == iVal;
	}

	/// Unequal operator
	/** \param iVal Value to be compared with
	 */
	bool operator !=(int iVal) const
	{
		return m_iVal != iVal;
	}

	/// Greater than operator
	/** \param iVal Value to be compared with
	 */
	bool operator >(int iVal) const
	{
		return m_iVal > iVal;
	}

	/// Greater-or-equal operator
	/** \param iVal Value to be compared with
	 */
	bool operator >=(int iVal) const
	{
		return m_iVal >= iVal;
	}

	/// Lesser than operator
	/** \param iVal Value to be compared with
	 */
	bool operator <(int iVal) const
	{
		return m_iVal < iVal;
	}

	/// Lesser-or-equal operator
	/** \param iVal Value to be compared with
	 */
	bool operator <=(int iVal) const
	{
		return m_iVal <= iVal;
	}

	/// Add to the integer
	/** \param iVal Value to be added
	 */
	int_no& operator+=(int iVal)
	{
		__int64 iTmp = (__int64)m_iVal + iVal;

		if (iTmp > INT_MAX) {
			m_iVal = INT_MAX;
		} else if (iTmp < INT_MIN) {
			m_iVal = INT_MIN;
		} else {
			m_iVal = (int)iTmp;
		}

		return *this;
	}

	/// Subtract from the integer
	/** \param iVal Value to be subtracted
	 */
	int_no& operator-=(int iVal)
	{
		return operator-=(0 - iVal);
	}	

	/// Add operator
	/** \param iVal Value to be added
	 */
	const int operator+(int iVal) const
	{
		int_no iRes = *this;
		iRes += iVal;
		return iRes;
	}

	/// Subtract operator
	/** \param iVal Value to be subtracted
	 */
	const int operator-(int iVal) const
	{
		return operator+(0 - iVal);
	}
};

//

/// The main class for the positioning and sizing of the ctrls.
/** You MUST derive your class from this class. You CAN derive your class from CScrollImpl.
 *  You MUST chain this class to the BEGIN_MSG_MAP()/END_MSG_MAP(). You SHOULD chain this class
 *  before chaining CScrollImpl. You MUST remember to use the full name CAutoSizeWindow<T, ...> when
 *  using methods (hint: DoPaint) present in multiple base classes. 
 *  You MUST manually call CAutoSizeWindow<T, ...>::DoPaint() from T::DoPaint() if you derive your class
 *  from CScrollImpl. You MUST let the handling of some messages proceed and not stop it unless you know
 *  what you are doing. This class processes WM_CREATE/WM_INITDIALOG, WM_GETMINMAXINFO, WM_PAINT, WM_SIZE.
 *  Note that if you want to handle WM_PAINT you MUST use a T::DoPaint method. This class will stop the 
 *  handling of the WM_PAINT message.
 *  \param T The name of the derived class
 *  \param t_bAutoMinSize If true this class will automatically handle the minimum and maximum size of the window
 */
template <class T, bool t_bAutoMinSize = true>
class CAutoSizeWindow
{
protected: 
	typedef CAutoSizeWindow<T> this_class_t; ///< A shortcut to the complete name of this class

public:
#pragma pack(push, 4)
	/// The margins/gaps of the window
	struct CMargins
	{
		int m_iLeftRight;
		int m_iTopBottom;
		int m_iXGap;
		int m_iYGap;
	};

	/// The ctrl group 
	struct CCtrlGroup
	{
		int m_iSignature;

		int m_iSize;
		int m_iNumRows;
		int m_iNumCtrls;
		int m_iColWidthFixed;
		int m_iColWidthMin;
		int m_iColWidthMax;
		int m_iColExpand;
		int m_iColContract;

		int m_iRowHeightFixed;
		int m_iRowHeightMin;
		int m_iRowHeightMax;
		int m_iRowExpand;
		int m_iRowContract;

		int m_iNumCols;

		int m_iCtrls[1];
	};

	/// The "container" for the main ctrl group
	struct CWindowMapStruct
	{
		int m_iSignature;
		CMargins m_margins;
		CCtrlGroup m_header;
	};
#pragma pack(pop)

	TRANSPARENT_LIST(CAutoSizeWindow<T>); ///< Empty transparent list

protected:
	struct CCtrlCounter;

	/// Support struct of CCtrlCounter. It's built to be a linked list (each element points to the previous one)
	struct CRowsIndex
	{
		CCtrlCounter *m_piNum;
		CRowsIndex *m_pPrev;
	};

	/// Multiple "templated" static-based counter. t_iType is the Counter number 
	/** \return Counter value
	 */
	template<int t_iType>
	int Func()
	{
		static int i = 0;
		i++;
		
		return i;
	}

	/// This "class" counts the number of controls of a ctrl group. It uses an external CRowsIndex to support itself
	struct CCtrlCounter
	{
		int m_iID;

		CCtrlCounter(int iID) : m_iID(iID)
		{
		}

		CCtrlCounter(CRowsIndex* pri) : m_iID(0)
		{
			pri->m_piNum = this;
		}

		CCtrlCounter(CRowsIndex *&pcn, int iAdder, int iDiv, int iID) : m_iID(iID)
		{
			pcn->m_piNum->m_iID = (int)(this - pcn->m_piNum) - iAdder;
			ATLASSERT(pcn->m_piNum->m_iID % iDiv == 0 && _T("Are you forgetting a WMB_COL()?"));
			pcn->m_piNum->m_iID /= iDiv;
			pcn = pcn->m_pPrev;
		}

		const CCtrlCounter operator++()
		{
			m_iID++;
			return *this;
		}

		const CCtrlCounter operator++(int)
		{
			const CCtrlCounter tmp(*this); 
			++(*this); 
			return tmp; 
		}

		operator int()
		{
			return m_iID;
		}
	};

public:
	BEGIN_MSG_MAP(CAutoSizeWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_INITDIALOG, OnCreate)
		if (t_bAutoMinSize) {
			MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		}
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

public:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		if (!pT->CtrlsInitialize()) {
			ATLTRACE(_T("Not initialized.\n"));
		} 

		__if_not_exists(T::GetScrollOffset) {
			else if (t_bAutoMinSize) {
				SIZE sizeMin, sizeMax;
	
				pT->GetMinMaxSize(sizeMin, sizeMax);
	
				RECT rectNew = {0, 0};

				RECT rectClient;
				pT->GetClientRect(&rectClient);

				if (rectClient.right < sizeMin.cx) {
					rectNew.right = sizeMin.cx;
				} else if (rectClient.right > sizeMax.cy) {
					rectNew.right = sizeMax.cx;
				} else {
					rectNew.right = rectClient.right;
				}

				if (rectClient.bottom < sizeMin.cy) {
					rectNew.bottom = sizeMin.cy;
				} else if (rectClient.bottom > sizeMax.cy) {
					rectNew.bottom = sizeMax.cy;
				} else {
					rectNew.bottom = rectClient.bottom;
				}

				if (rectNew.right != rectClient.right || rectNew.bottom != rectClient.bottom) {
					RECT rectWindow;
					pT->GetWindowRect(&rectWindow);
					pT->MapWindowPoints(NULL, (LPPOINT)&rectNew, 2);

					rectNew.right += rectWindow.right - rectWindow.left - rectNew.right + rectNew.left;
					rectNew.bottom += rectWindow.bottom - rectWindow.top - rectNew.bottom + rectNew.top;

					pT->SetWindowPos(NULL, 0, 0, rectNew.right - rectNew.left, rectNew.bottom - rectNew.top, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_NOMOVE);
				}
			}
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		MINMAXINFO *pMinMax = (MINMAXINFO*)lParam;

		T* pT = static_cast<T*>(this);

		SIZE sizeMin, sizeMax;

		pT->GetMinMaxSize(sizeMin, sizeMax);

		RECT rectWindow, rectClient;
		pT->GetWindowRect(&rectWindow);
		pT->GetClientRect(&rectClient);
		pT->MapWindowPoints(NULL, (LPPOINT)&rectClient, 2);

		__if_not_exists(T::GetScrollOffset) {
			pMinMax->ptMinTrackSize.x = sizeMin.cx + rectWindow.right - rectWindow.left - rectClient.right + rectClient.left;
			pMinMax->ptMinTrackSize.y = sizeMin.cy + rectWindow.bottom - rectWindow.top - rectClient.bottom + rectClient.top;
		}
		
		if (pMinMax->ptMaxTrackSize.x > sizeMax.cx) {
			pMinMax->ptMaxTrackSize.x = sizeMax.cx;
		}

		if (pMinMax->ptMaxTrackSize.y > sizeMax.cy) {
			pMinMax->ptMaxTrackSize.y = sizeMax.cy;
		}

		if (pMinMax->ptMaxSize.x > sizeMax.cx) {
			pMinMax->ptMaxSize.x = sizeMax.cx;
		}

		if (pMinMax->ptMaxSize.y > sizeMax.cy) {
			pMinMax->ptMaxSize.y = sizeMax.cy;
		}

		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (!wParam) {
			T* pT = static_cast<T*>(this);
			pT->CtrlsArrange();
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);

		POINT pt = {0};
		__if_exists(T::GetScrollOffset) {
			pT->GetScrollOffset(pt);
		}

		if(wParam != NULL) {
			WTL::CDCHandle dc = (HDC)wParam;
			dc.SetViewportOrg(-pt.x, -pt.y);
			pT->AutoSizeDoPaint(dc);
		} else {
			WTL::CPaintDC dc(pT->m_hWnd);
			dc.SetViewportOrg(-pt.x, -pt.y);
			pT->AutoSizeDoPaint(dc.m_hDC);
		}
		
		return 0;
	}
	
	//

	/// Initializes the window. 
	/** \param piCtrls Ctrls map
	 *  \param bForce Force the initialization (the window will be reinitialized if already initialized)
	 *  \return TRUE if successful, FALSE if failed
	 */
	BOOL CtrlsInitialize(CWindowMapStruct *piCtrls = T::GetWindowMap(), BOOL bForce = FALSE)
	{
		if (piCtrls->m_iSignature && !bForce) {
			return TRUE;
		}

		T* pT = static_cast<T*>(this);

		if (!pT->m_hWnd) {
			ATLTRACE(_T("Window not initialized.\n"));
			return FALSE;
		}

#ifdef DEBUG
		if (!pT->IsWindow()) {
			ATLTRACE(_T("Window not initialized.\n"));
			return FALSE;
		}
#endif

		if (!piCtrls->m_iSignature) {
			pT->HandleTransparentMap();
		}

		RECT rectUnits = {4, 8, 0, 0};
		ATLVERIFY(pT->MapDialogRect(&rectUnits));
		LONG lXUnits = rectUnits.left, lYUnits = rectUnits.top;

		CMargins marginsPix;
		_CheckMargins(lXUnits, 4, lYUnits, 8, &piCtrls->m_margins, &marginsPix);

		BOOL bRes;
		ATLVERIFY((bRes = pT->_CtrlsInitialize(&piCtrls->m_header, &marginsPix, lXUnits, lYUnits)) != FALSE);
		if (bRes) {
			piCtrls->m_iSignature = 1;
			return TRUE;
		} else {
			return FALSE;
		}
	}

	/// Arranges the ctrls of the window. Initializes the window if necessary
	/** \param piCtrls Ctrls map
	 *  \return TRUE if successful, FALSE if failure
	 */
	BOOL CtrlsArrange(CWindowMapStruct *piCtrls = T::GetWindowMap())
	{
		T* pT = static_cast<T*>(this);

		if (!pT->CtrlsInitialize(piCtrls)) {
			ATLTRACE(_T("Not initialized.\n"));
			return FALSE;
		}

		RECT rectUnits = {4, 8, 0, 0};
		ATLVERIFY(pT->MapDialogRect(&rectUnits));
		LONG lXUnits = rectUnits.left, lYUnits = rectUnits.top;

		RECT rect;
		ATLVERIFY(pT->GetClientRect(&rect));

		CMargins marginsPix;
		_CheckMargins(lXUnits, 4, lYUnits, 8, &piCtrls->m_margins, &marginsPix);

		rect.right = (rect.right > marginsPix.m_iLeftRight ? rect.right - marginsPix.m_iLeftRight : 0);
		rect.bottom = (rect.bottom > marginsPix.m_iTopBottom ? rect.bottom - marginsPix.m_iTopBottom : 0);

		CCtrlGroup *pheader = &piCtrls->m_header;

		if (rect.right < pheader->m_iColWidthMin) {
			rect.right = pheader->m_iColWidthMin;
		}

		if (rect.bottom < pheader->m_iRowHeightMin) {
			rect.bottom = pheader->m_iRowHeightMin;
		}

		HDWP hdwp;
		ATLVERIFY((hdwp = ::BeginDeferWindowPos(pheader->m_iNumCtrls)) != NULL);
		if (!hdwp) {
			return FALSE;
		}

		BOOL bRes;
		ATLVERIFY((bRes = _CtrlsArrange(hdwp, pheader, &marginsPix, lXUnits, lYUnits, &rect)) != FALSE);

		ATLVERIFY(::EndDeferWindowPos(hdwp));

		return bRes;
	}

	/// Gets the minimum size of the window. Initializes the window if necessary
	/** \param sizeMin This variable will contain the minimum size of the window
	 *  \param piCtrls Ctrls map
	 *  \return TRUE if successful, FALSE if failure
	 */
	BOOL GetMinSize(SIZE &sizeMin, CWindowMapStruct *piCtrls = T::GetWindowMap())
	{
		SIZE sz;
		return GetMinMaxSize(sizeMin, sz, piCtrls);
	}

	/// Gets the maximum size of the window. Initializes the window if necessary
	/** \param sizeMax This variable will contain the maximum size of the window
	 *  \param piCtrls Ctrls map
	 *  \return TRUE if successful, FALSE if failure
	 */
	BOOL GetMaxSize(SIZE &sizeMax, CWindowMapStruct *piCtrls = T::GetWindowMap())
	{
		SIZE sz;
		return GetMinMaxSize(sz, sizeMax, piCtrls);
	}

	/// Gets the minimum and maximum size of the window. Initializes the window if necessary
	/** \param sizeMin This variable will contain the minimum size of the window
	 *  \param sizeMax This variable will contain the maximum size of the window
	 *  \param piCtrls Ctrls map
	 *  \return TRUE if successful, FALSE if failure
	 */
	BOOL GetMinMaxSize(SIZE &sizeMin, SIZE &sizeMax, CWindowMapStruct *piCtrls = T::GetWindowMap())
	{
		if (!CtrlsInitialize(piCtrls)) {
			ATLTRACE(_T("Not initialized.\n"));
			return FALSE;
		}

		T* pT = static_cast<T*>(this);

		RECT rectUnits = {4, 8, 0, 0};
		ATLVERIFY(pT->MapDialogRect(&rectUnits));
		LONG lXUnits = rectUnits.left, lYUnits = rectUnits.top;

		CMargins marginsPix;
		_CheckMargins(lXUnits, 4, lYUnits, 8, &piCtrls->m_margins, &marginsPix);

		CCtrlGroup *pheader = &piCtrls->m_header;
		sizeMin.cx = pheader->m_iColWidthMin + marginsPix.m_iLeftRight;
		sizeMin.cy = pheader->m_iRowHeightMin + marginsPix.m_iTopBottom;

		if (pheader->m_iColWidthMax != INT_MAX) {
			sizeMax.cx = pheader->m_iColWidthMax + marginsPix.m_iLeftRight;
		} else {
			sizeMax.cx = INT_MAX;
		}

		if (pheader->m_iRowHeightMax != INT_MAX) {
			sizeMax.cy = pheader->m_iRowHeightMax + marginsPix.m_iTopBottom;
		} else {
			sizeMax.cy = INT_MAX;
		}

		return TRUE;
	}

	/// Does the paint work. You can override it, but remember to call this method in the end, otherwise Transparent ctrls won't be painted.
	/** \param dc The hDC to paint to
	 */
	void AutoSizeDoPaint(WTL::CDCHandle dc)
	{
		T* pT = static_cast<T*>(this);
		ATLVERIFY(pT->HandleTransparentMap(dc));
	}

protected:
	BOOL _CtrlsInitialize(CCtrlGroup *pheader, CMargins *pmargins, LONG lXUnits, LONG lYUnits, BOOL bMain = TRUE)
	{
		if (pheader->m_iSignature != WMH_BEGIN) {
			ATLTRACE(_T("Wrong header format. Aborted.\n"));
			return FALSE;
		}

		int *piHeadCols = pheader->m_iCtrls;

		if (piHeadCols[pheader->m_iNumCols * 3] != WMH_END) {
			ATLTRACE(_T("Wrong header format. Aborted.\n"));
			return FALSE;
		}

		if (*((int*)pheader + pheader->m_iSize) != WM_END) {
			ATLTRACE(_T("Wrong header format. Aborted.\n"));
			return FALSE;
		}

		int *piCell = piHeadCols + pheader->m_iNumCols * 3 + 1;

		if (*piCell != WMR_BEGIN && *piCell != WM_END) {
			ATLTRACE(_T("Malformed endrow in header.\n"));
			return FALSE;
		}

		T* pT = static_cast<T*>(this);

		int iColWidthFixed = 0;
		int iColWidthMin = 0;
		int_no iColWidthMax = 0;
		int iColExpand = 0;
		int iColContract = 0;

		int *piColAuto = (int*)_alloca(pheader->m_iNumCols * sizeof(int));

		if (bMain) {
			iColWidthFixed += pmargins->m_iLeftRight;
			iColWidthMin += pmargins->m_iLeftRight;
			iColWidthMax += pmargins->m_iLeftRight;
		}

		for (int i = 0; i < pheader->m_iNumCols; i++) {
			int iType, iStyle, iVal;
			_TypeVal(piHeadCols[i * 3], iType, iStyle, iVal);

			if (i && !(iStyle & WMSRC_NOGAP)) {
				iColWidthFixed += pmargins->m_iXGap;
				iColWidthMin += pmargins->m_iXGap;
				iColWidthMax += pmargins->m_iXGap;
			}

			if (iType == WMSRC_AUTO) {
				piColAuto[i] = 0;

				piHeadCols[i * 3] = iType|iStyle;

				_SetVariableMinMax(i, piHeadCols + i * 3);
			} else if (iType == WMSRC_EXPANDABLE) {
				piColAuto[i] = INT_MIN;

				if (iVal > 0) {
					iColExpand += iVal;
				} else if (iVal == 0) {
					piHeadCols[i * 3] |= WMSRC_DEFEXPANDABLEVAL;
					iColExpand += WMSRC_DEFEXPANDABLEVAL;
				} else {
					ATLTRACE(_T("Wrong value in column: %d. Ignored.\n"), i);
					piHeadCols[i * 3] = iType|iStyle|WMSRC_DEFEXPANDABLEVAL;
					iColExpand += WMSRC_DEFEXPANDABLEVAL;
				}

				_SetVariableMinMax(i, piHeadCols + i * 3);
			} else if (iType == WMSRC_CONTRACTABLE) {
				piColAuto[i] = INT_MIN;

				if (iVal > 0) {
					iColContract += iVal;
				} else if (iVal == 0) {
					piHeadCols[i * 3] |= WMSRC_DEFCONTRACTABLEVAL;
					iColContract += WMSRC_DEFCONTRACTABLEVAL;
				} else {
					ATLTRACE(_T("Wrong value in column: %d. Ignored.\n"), i);
					piHeadCols[i * 3] = iType|iStyle|WMSRC_DEFCONTRACTABLEVAL;
					iColContract += WMSRC_DEFCONTRACTABLEVAL;
				}

				_SetVariableMinMax(i, piHeadCols + i * 3);
			} else if (iType == WMSRC_EQUAL) {
				if (iVal >= i || iVal < 0) {
					ATLTRACE(_T("Columns can be equal only to preceeding columns. Flag in column: %d ignored. Auto-column set.\n"), i);
					piHeadCols[i * 3] = WMSRC_AUTO|iStyle;
					piColAuto[i] = 0;

					_SetVariableMinMax(i, piHeadCols + i * 3);
				} else {
					int iColDef = iVal;
					while (piColAuto[iColDef] < 0 && piColAuto[iColDef] != INT_MIN) {
						iColDef = -piColAuto[iColDef] - 1;
					}

					int iTypeDef, iStyleDef, iValDef;
					_TypeVal(piHeadCols[iColDef * 3], iTypeDef, iStyleDef, iValDef);

					_CheckEqualMinMax(i, piHeadCols + i * 3);

					piColAuto[i] = -(iColDef + 1);

					if (iTypeDef == WMSRC_AUTO) {
					} else if (iTypeDef == WMSRC_EXPANDABLE) {
						iColExpand += iValDef;
					} else if (iTypeDef == WMSRC_CONTRACTABLE) {
						iColContract += iValDef;
					} else if (iTypeDef == WMSRC_GAP) {
						int iWidth = pmargins->m_iXGap;
						if (iValDef > 0) {
							iWidth += ::MulDiv(iValDef, lXUnits, 4);
						} else {
							iWidth += (-iValDef);
						}
					} else if (iTypeDef == WMSRC_GAPM) {
						int iWidth = pmargins->m_iXGap;
						if (iValDef > 0) {
							iWidth -= ::MulDiv(iValDef, lXUnits, 4);
						} else {
							iWidth -= (-iValDef);
						}

						if (iWidth < 0) {
							iWidth = 0;
						}
					} else {
						int iWidth;
						if (iValDef > 0) {
							iWidth = ::MulDiv(iValDef, lXUnits, 4);
						} else {
							iWidth = -iValDef;
						}
					}
				}
			} else if (iType == WMSRC_GAP) {
				piColAuto[i] = INT_MIN;

				int iWidth = pmargins->m_iXGap;
				if (iVal > 0) {
					iWidth += ::MulDiv(iVal, lXUnits, 4);
				} else {
					iWidth += (-iVal);
				}

				_CheckFixedMinMax(i, piHeadCols + i * 3, lXUnits, 4, iWidth);
			} else if (iType == WMSRC_GAPM) {
				piColAuto[i] = INT_MIN;

				int iWidth = pmargins->m_iXGap;
				if (iVal > 0) {
					iWidth -= ::MulDiv(iVal, lXUnits, 4);
				} else {
					iWidth -= (-iVal);
				}

				if (iWidth < 0) {
					iWidth = 0;
				}

				_CheckFixedMinMax(i, piHeadCols + i * 3, lXUnits, 4, iWidth);
			} else {
				if (iVal > 0) {
					if (iType) {
						ATLTRACE(_T("Wrong flag in column: %d. Ignored. Auto-column set.\n"), i);
						piHeadCols[i * 3] = WMSRC_AUTO|iStyle;
						piColAuto[i] = 0;

						_SetVariableMinMax(i, piHeadCols + i * 3);
					} else {
						piColAuto[i] = INT_MIN;

						int iWidth = ::MulDiv(iVal, lXUnits, 4);

						_CheckFixedMinMax(i, piHeadCols + i * 3, lXUnits, 4, iWidth);
					}
				} else {
					piColAuto[i] = INT_MIN;

					int iWidth = -iVal;

					_CheckFixedMinMax(i, piHeadCols + i * 3, lXUnits, 4, iWidth);
				}
			}
		}

		int iNumCtrls = 0;

		int iRow = 0, iRowDef = 0;
		int iRowHeightFixed = 0;
		int iRowHeightMin = 0;
		int_no iRowHeightMax = 0;
		int iRowExpand = 0;
		int iRowContract = 0;

		if (pheader->m_iNumRows) {
			int *piRowAuto = (int*)_alloca(pheader->m_iNumRows * sizeof(int));
			int **ppiRowBegin = (int**)_alloca(pheader->m_iNumRows * sizeof(int*));

			if (bMain) {
				iRowHeightFixed += pmargins->m_iTopBottom;
				iRowHeightMin += pmargins->m_iTopBottom;
				iRowHeightMax += pmargins->m_iTopBottom;
			}

			while (*piCell != WM_END) {
				ppiRowBegin[iRow] = piCell;

				if (ppiRowBegin[iRow][0] == WMR_BEGIN) {
					int iType, iStyle, iVal;
					_TypeVal(ppiRowBegin[iRow][1], iType, iStyle, iVal);

					if (iRow && !(iStyle & WMSRC_NOGAP)) {
						iRowHeightFixed += pmargins->m_iYGap;
						iRowHeightMin += pmargins->m_iYGap;
						iRowHeightMax += pmargins->m_iYGap;
					}

					if (iType == WMSRC_AUTO) {
						piRowAuto[iRow] = 0;
						iRowDef = iRow;

						ppiRowBegin[iRow][1] = iType|iStyle;

						_SetVariableMinMax(iRow, ppiRowBegin[iRow] + 1);
					} else if (iType == WMSRC_EXPANDABLE) {
						piRowAuto[iRow] = INT_MIN;
						iRowDef = iRow;

						if (iVal > 0) {
							iRowExpand += iVal;
						} else if (iVal == 0) {
							ppiRowBegin[iRow][1] |= WMSRC_DEFEXPANDABLEVAL;
							iRowExpand += WMSRC_DEFEXPANDABLEVAL;
						} else {
							ATLTRACE(_T("Wrong value in row: %d. Ignored.\n"), iRow);
							ppiRowBegin[iRow][1] = iType|iStyle|WMSRC_DEFEXPANDABLEVAL;
							iRowExpand += WMSRC_DEFEXPANDABLEVAL;
						}

						_SetVariableMinMax(iRow, ppiRowBegin[iRow] + 1);
					} else if (iType == WMSRC_CONTRACTABLE) {
						piRowAuto[iRow] = INT_MIN;
						iRowDef = iRow;

						if (iVal > 0) {
							iRowContract += iVal;
						} else if (iVal == 0) {
							ppiRowBegin[iRow][1] |= WMSRC_DEFCONTRACTABLEVAL;
							iRowContract += WMSRC_DEFCONTRACTABLEVAL;
						} else {
							ATLTRACE(_T("Wrong value in row: %d. Ignored.\n"), iRow);
							ppiRowBegin[iRow][1] = iType|iStyle|WMSRC_DEFCONTRACTABLEVAL;
							iRowContract += WMSRC_DEFCONTRACTABLEVAL;
						}

						_SetVariableMinMax(iRow, ppiRowBegin[iRow] + 1);
					} else if (iType == WMSRC_EQUAL) {
						if (iVal >= iRow || iVal < 0) {
							ATLTRACE(_T("Rows can be equal only to preceeding rows. Flag in row: %d ignored. Auto-row set.\n"), iRow);
							ppiRowBegin[iRow][1] = WMSRC_AUTO|iStyle;
							piRowAuto[iRow] = 0;
							iRowDef = iRow;

							_SetVariableMinMax(iRow, ppiRowBegin[iRow] + 1);
						} else {
							iRowDef = iVal;
							while (piRowAuto[iRowDef] < 0 && piRowAuto[iRowDef] != INT_MIN) {
								iRowDef = -piRowAuto[iRowDef] - 1;
							}

							int iTypeDef, iStyleDef, iValDef;
							_TypeVal(ppiRowBegin[iRowDef][1], iTypeDef, iStyleDef, iValDef);

							_CheckEqualMinMax(iRow, ppiRowBegin[iRow] + 1);

							if (iTypeDef == WMSRC_AUTO) {
								piRowAuto[iRow] = -(iRowDef + 1);
							} else if (iTypeDef == WMSRC_EXPANDABLE) {
								piRowAuto[iRow] = INT_MIN;

								iRowExpand += iValDef;
							} else if (iTypeDef == WMSRC_CONTRACTABLE) {
								piRowAuto[iRow] = INT_MIN;

								iRowContract += iValDef;
							} else if (iTypeDef == WMSRC_GAP) {
								piRowAuto[iRow] = INT_MIN;

								int iHeight = pmargins->m_iYGap;
								if (iValDef > 0) {
									iHeight += ::MulDiv(iValDef, lYUnits, 8);
								} else {
									iHeight += (-iValDef);
								}
							} else if (iTypeDef == WMSRC_GAPM) {
								piRowAuto[iRow] = INT_MIN;

								int iHeight = pmargins->m_iYGap;
								if (iValDef > 0) {
									iHeight -= ::MulDiv(iValDef, lYUnits, 8);
								} else {
									iHeight -= (-iValDef);
								}

								if (iHeight < 0) {
									iHeight = 0;
								}
							} else {
								piRowAuto[iRow] = INT_MIN;

								int iHeight;
								if (iValDef > 0) {
									iHeight = ::MulDiv(iValDef, lYUnits, 8);
								} else {
									iHeight = -iValDef;
								}
							}
						}
					} else if (iType == WMSRC_GAP) {
						piRowAuto[iRow] = INT_MIN;
						iRowDef = iRow;

						int iHeight = pmargins->m_iYGap;
						if (iVal > 0) {
							iHeight += ::MulDiv(iVal, lYUnits, 8);
						} else {
							iHeight += (-iVal);
						}

						_CheckFixedMinMax(iRow, ppiRowBegin[iRow] + 1, lYUnits, 8, iHeight);
					} else if (iType == WMSRC_GAPM) {
						piRowAuto[iRow] = INT_MIN;
						iRowDef = iRow;

						int iHeight = pmargins->m_iYGap;
						if (iVal > 0) {
							iHeight -= ::MulDiv(iVal, lYUnits, 8);
						} else {
							iHeight -= (-iVal);
						}

						if (iHeight < 0) {
							iHeight = 0;
						}

						_CheckFixedMinMax(iRow, ppiRowBegin[iRow] + 1, lYUnits, 8, iHeight);
					} else {
						iRowDef = iRow;
						if (iVal > 0) {
							if (iType) {
								ATLTRACE(_T("Wrong flag in row: %d. Ignored. Auto-row set.\n"), iRow);
								ppiRowBegin[iRow][1] = WMSRC_AUTO|iStyle;
								piRowAuto[iRow] = 0;

								_SetVariableMinMax(iRow, ppiRowBegin[iRow] + 1);
							} else {
								piRowAuto[iRow] = INT_MIN;

								int iHeight = ::MulDiv(iVal, lYUnits, 8);

								_CheckFixedMinMax(iRow, ppiRowBegin[iRow] + 1, lYUnits, 8, iHeight);
							}
						} else {
							piRowAuto[iRow] = INT_MIN;

							int iHeight = -iVal;

							_CheckFixedMinMax(iRow, ppiRowBegin[iRow] + 1, lYUnits, 8, iHeight);
						}
					}

					piCell += 4;

					int iCol = 0, iColDef = 0;
					while (*piCell != WMR_END && *piCell != WM_END) {
						if (iCol < pheader->m_iNumCols) {
							iColDef = (piColAuto[iCol] == INT_MIN || piColAuto[iCol] >= 0 ? iCol: -piColAuto[iCol] - 1);

							if (*piCell != WMH_BEGIN && *piCell != WMSCTRL_EMPTY && *piCell != WMSCTRL_EXPDOWN && *piCell != WMSCTRL_EXPRIGHT) {
								iNumCtrls++;
								if (piColAuto[iColDef] != INT_MIN || piRowAuto[iRowDef] != INT_MIN || 
									piHeadCols[iColDef * 3 + 1] & WMSRCMM_NEEDCALC || piHeadCols[iColDef * 3 + 2] & WMSRCMM_NEEDCALC || 
									ppiRowBegin[iRowDef][2] & WMSRCMM_NEEDCALC || ppiRowBegin[iRowDef][3] & WMSRCMM_NEEDCALC) {

									HWND hWnd = pT->GetDlgItem(*piCell);
									if (!hWnd) {
										ATLTRACE(_T("Inexistant ctrl in row: %d, col: %d. Ignored.\n"), iRow, iCol);
									} else {
										const MINMAXINFO mmOrig = {0, 0, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX};
										MINMAXINFO mm = mmOrig;

										::SendMessage(hWnd, WM_GETMINMAXINFO, 0, (LPARAM)&mm);
										if (::memcmp(&mm, &mmOrig, sizeof(MINMAXINFO)) != 0) {
											_CalcAuto(piColAuto[iColDef], mm.ptMinTrackSize.x);
											_CalcMinMaxAuto(piHeadCols + iColDef * 3, mm.ptMinTrackSize.x, mm.ptMaxTrackSize.x);

											_CalcAuto(piRowAuto[iRowDef], mm.ptMinTrackSize.y);
											_CalcMinMaxAuto(ppiRowBegin[iRowDef] + 1, mm.ptMinTrackSize.y, mm.ptMaxTrackSize.y);
										} else {
											RECT rect = {0};
											ATLVERIFY(::GetWindowRect(hWnd, &rect));

											int iWidth = rect.right - rect.left;
											_CalcAuto(piColAuto[iColDef], iWidth);

											int iHeight = rect.bottom - rect.top;
											_CalcAuto(piRowAuto[iRowDef], iHeight);
										}
									}
								}
							} else if (*piCell == WMH_BEGIN) {
								CCtrlGroup *pheaderNew = (CCtrlGroup*)piCell; 
								if (_CtrlsInitialize(pheaderNew, pmargins, lXUnits, lYUnits, FALSE)) {
									_CalcAuto(piColAuto[iColDef], pheaderNew->m_iColWidthMin);
									_CalcMinMaxAuto(piHeadCols + iColDef * 3, pheaderNew->m_iColWidthMin, pheaderNew->m_iColWidthMax);

									_CalcAuto(piRowAuto[iRowDef], pheaderNew->m_iRowHeightMin);
									_CalcMinMaxAuto(ppiRowBegin[iRowDef] + 1, pheaderNew->m_iRowHeightMin, pheaderNew->m_iRowHeightMax);

									iNumCtrls += pheaderNew->m_iNumCtrls;

									piCell += piCell[1];
								} else {
									return FALSE;
								}
							}
						} else {
							ATLTRACE(_T("Too many columns. Row: %d, col: %d ignored.\n"), iRow, iCol);

							if (*piCell == WMH_BEGIN) {
								piCell += piCell[1];
							}
						}

						iCol++;
						piCell++;
					}

					if (*piCell == WM_END) {
						ATLTRACE(_T("Malformed endrow in row: %d. Ignored.\n"), iRow);
					} 
				} else {
					ATLTRACE(_T("Malformed row: %d. Skipped. \n"), iRow);
					while (*piCell != WMR_END && *piCell != WM_END) {
						piCell++;
					}

					piRowAuto[iRow] = INT_MIN;
				}

				iRow++;

				if (*piCell != WM_END) {
					piCell++;
				}
			}

#ifdef TRACEAUTOSIZE
			ATLTRACE("Rows: ");
#endif 
			for (int i = 0; i < iRow; i++) {
				if (piRowAuto[i] != INT_MIN) {
					if (piRowAuto[i] >= 0) {
						_CheckAutoMinMax(ppiRowBegin[i] + 1, piRowAuto[i]);
						int iType = ppiRowBegin[i][1] & WMSRC_TYPEMASK;
						_AddMinMax(i, ppiRowBegin[i] + 1, lYUnits, 8, iRowHeightFixed, iType, iRowHeightMin, iRowHeightMax);
						_SetAuto(ppiRowBegin[i] + 1, lYUnits, 8, piRowAuto[i]);
					} else {
						int iDef = -piRowAuto[i] - 1;
						int iType = ppiRowBegin[iDef][1] & WMSRC_TYPEMASK;
						_AddMinMax(i, ppiRowBegin[iDef] + 1, lYUnits, 8, iRowHeightFixed, iType, iRowHeightMin, iRowHeightMax);
					}
				} else {
					int iType = ppiRowBegin[i][1] & WMSRC_TYPEMASK;
					_AddMinMax(i, ppiRowBegin[i] + 1, lYUnits, 8, iRowHeightFixed, iType, iRowHeightMin, iRowHeightMax);
				}

#ifdef TRACEAUTOSIZE
				int iTypeMin, iTypeMax;
				int iMinPix, iMaxPix;
				_LoadMinMax(ppiRowBegin[i] + 1, lYUnits, 8, iTypeMin, iMinPix, iTypeMax, iMaxPix);
				ATLTRACE("%d-%d ", iMinPix, iMaxPix);
#endif
			}
		}

#ifdef TRACEAUTOSIZE
		ATLTRACE("\nCols: ");
#endif
		for (int i = 0; i < pheader->m_iNumCols; i++) {
			if (piColAuto[i] != INT_MIN) {
				if (piColAuto[i] >= 0) {
					_CheckAutoMinMax(piHeadCols + i * 3, piColAuto[i]);
					int iType = piHeadCols[i * 3] & WMSRC_TYPEMASK;
					_AddMinMax(i, piHeadCols + i * 3, lXUnits, 4, iColWidthFixed, iType, iColWidthMin, iColWidthMax);
					_SetAuto(piHeadCols + i * 3, lXUnits, 4, piColAuto[i]);
				} else {
					int iDef = -piColAuto[i] - 1;
					int iType = piHeadCols[iDef * 3] & WMSRC_TYPEMASK;
					_AddMinMax(i, piHeadCols + iDef * 3, lXUnits, 4, iColWidthFixed, iType, iColWidthMin, iColWidthMax);
				}
			} else {
				int iType = piHeadCols[i * 3] & WMSRC_TYPEMASK;
				_AddMinMax(i, piHeadCols + i * 3, lXUnits, 4, iColWidthFixed, iType, iColWidthMin, iColWidthMax);
			}

#ifdef TRACEAUTOSIZE
			int iTypeMin, iTypeMax;
			int iMinPix, iMaxPix;
			_LoadMinMax(piHeadCols + i * 3, lXUnits, 4, iTypeMin, iMinPix, iTypeMax, iMaxPix);
			ATLTRACE("%d-%d ", iMinPix, iMaxPix);
#endif
		}

#ifdef TRACEAUTOSIZE
		ATLTRACE("\n");
#endif

		if (iRow != pheader->m_iNumRows) {
			ATLTRACE(_T("Number of rows incorrect. Fixed.\n"));
			pheader->m_iNumRows = iRow;
		}

		pheader->m_iNumCtrls = iNumCtrls;
		pheader->m_iColWidthFixed = iColWidthFixed;
		pheader->m_iColWidthMin = iColWidthMin;
		pheader->m_iColWidthMax = iColWidthMax;
		pheader->m_iColExpand = iColExpand;
		pheader->m_iColContract = iColContract;

		pheader->m_iRowHeightFixed = iRowHeightFixed;
		pheader->m_iRowHeightMin = iRowHeightMin;
		pheader->m_iRowHeightMax = iRowHeightMax;
		pheader->m_iRowExpand = iRowExpand;
		pheader->m_iRowContract = iRowContract;

#ifdef TRACEAUTOSIZE
		ATLTRACE("Fixed: %d, %d. Min: %d, %d. Max: %d, %d\n", iColWidthFixed, iRowHeightFixed, iColWidthMin, iRowHeightMin, iColWidthMax, iRowHeightMax);
#endif

		ATLASSERT(*piCell == WM_END);
		return TRUE;
	}

	BOOL _CtrlsArrange(HDWP &hdwp, CCtrlGroup *pheader, CMargins *pmargins, LONG lXUnits, LONG lYUnits, RECT *pRectWindow, BOOL bMain = TRUE)
	{
		if (pheader->m_iSignature != WMH_BEGIN) {
			ATLTRACE(_T("Wrong header format. Aborted.\n"));
			return FALSE;
		}

		int *piHeadCols = pheader->m_iCtrls;

		int *piCell = piHeadCols + pheader->m_iNumCols * 3 + 1;

		T* pT = static_cast<T*>(this);

		if (pheader->m_iNumRows) {
			int iColTrueMin = pheader->m_iColWidthMin;
			int iColTrueMax = pheader->m_iColWidthMax;

			int iRowTrueMin = pheader->m_iRowHeightMin;
			int iRowTrueMax = pheader->m_iRowHeightMax;

			int iWidth = pRectWindow->right - pRectWindow->left;
			int iTotalColContract;

			if (iWidth < iColTrueMin) {
				ATLTRACE(_T("Width too small! Needed %d more pixels.\n"), iColTrueMin - iWidth);
				iTotalColContract = 0;
			} else if (iWidth >= iColTrueMin && iWidth <= iColTrueMax) {
				iTotalColContract = 0;
			} else {
				iTotalColContract = iWidth - iColTrueMax;
				iWidth = iColTrueMax;
			}

			int iHeight = pRectWindow->bottom - pRectWindow->top;
			int iTotalRowContract;

			if (iHeight < iRowTrueMin) {
				ATLTRACE(_T("Height too small! Needed %d more pixels.\n"), iRowTrueMin - iHeight);
				iTotalRowContract = 0;
			} else if (iHeight >= iRowTrueMin && iHeight <= iRowTrueMax) {
				iTotalRowContract = 0;
			} else {
				iTotalRowContract = iHeight - iRowTrueMax;
				iHeight = iRowTrueMax;
			}

#ifdef TRACEAUTOSIZE2
			ATLTRACE("Rect: %d, %d\n", iWidth, iHeight);
#endif

			int iTotalColExpand = (iWidth > iColTrueMin ? iWidth - pheader->m_iColWidthFixed : 0);
			int iColExpand = pheader->m_iColExpand, iColContract = pheader->m_iColContract;

			int iTotalRowExpand = (iHeight > iRowTrueMin ? iHeight - pheader->m_iRowHeightFixed : 0);
			int iRowExpand = pheader->m_iRowExpand, iRowContract = pheader->m_iRowContract;

			int *piWidthCol = (int*)_alloca(pheader->m_iNumCols * sizeof(int));

			int iWidthTotal = pRectWindow->left;
			if (bMain) {
				iWidthTotal += pmargins->m_iLeftRight;
			}

#ifdef TRACEAUTOSIZE2
			ATLTRACE("Cols: ");
#endif

			for (int i = 0; i < pheader->m_iNumCols; i++) {
				int iType, iStyle, iVal;
				_TypeVal(piHeadCols[i * 3], iType, iStyle, iVal);

				if (i && !(iStyle & WMSRC_NOGAP)) {
					iWidthTotal += pmargins->m_iXGap;
				}

				if (iWidthTotal > pRectWindow->right) {
					iWidthTotal = pRectWindow->right;
				}

				int iTypeMin, iTypeMax;
				int iMinPix, iMaxPix;
				_LoadMinMax(piHeadCols + i * 3, lXUnits, 4, iTypeMin, iMinPix, iTypeMax, iMaxPix);

				if (iType == WMSRC_AUTO) {
					piWidthCol[i] = iVal;
				} else if (iType == WMSRC_EXPANDABLE) {
					piWidthCol[i] = ::MulDiv(iTotalColExpand, iVal, iColExpand);
					_NormalizeColRow(piWidthCol[i], iMinPix, iMaxPix);
					iColExpand -= iVal;
					iTotalColExpand = (iTotalColExpand > piWidthCol[i] ? iTotalColExpand - piWidthCol[i] : 0);
				} else if (iType == WMSRC_CONTRACTABLE) {
					piWidthCol[i] = ::MulDiv(iTotalColContract, iVal, iColContract);
					_NormalizeColRow(piWidthCol[i], iMinPix, iMaxPix);
					iColContract -= iVal;
					iTotalColContract = (iTotalColContract > piWidthCol[i] ? iTotalColContract - piWidthCol[i] : 0);
				} else if (iType == WMSRC_EQUAL) {
					int iColDef = iVal;
					int iTypeDef, iStyleDef, iValDef;
					do {
						_TypeVal(piHeadCols[iColDef * 3], iTypeDef, iStyleDef, iValDef);
					} while (iTypeDef == WMSRC_EQUAL && (iColDef = iValDef) == iColDef);

					int iTypeMinDef, iTypeMaxDef;
					int iMinPixDef, iMaxPixDef;
					_LoadMinMax(piHeadCols + iColDef * 3, lXUnits, 4, iTypeMinDef, iMinPixDef, iTypeMaxDef, iMaxPixDef);

					if (iTypeDef == WMSRC_AUTO) {
						piWidthCol[i] = iValDef;
					} else if (iTypeDef == WMSRC_EXPANDABLE) {
						piWidthCol[i] = ::MulDiv(iTotalColExpand, iValDef, iColExpand);
						_NormalizeColRow(piWidthCol[i], iMinPixDef, iMaxPixDef);
						iColExpand -= iValDef;
						iTotalColExpand = (iTotalColExpand > piWidthCol[i] ? iTotalColExpand - piWidthCol[i] : 0);
					} else if (iTypeDef == WMSRC_CONTRACTABLE) {
						piWidthCol[i] = ::MulDiv(iTotalColContract, iValDef, iColContract);
						_NormalizeColRow(piWidthCol[i], iMinPixDef, iMaxPixDef);
						iColContract -= iValDef;
						iTotalColContract = (iTotalColContract > piWidthCol[i] ? iTotalColContract - piWidthCol[i] : 0);
					} else if (iTypeDef == WMSRC_GAP) {
						piWidthCol[i] = iMinPixDef;
					} else if (iTypeDef == WMSRC_GAPM) {
						piWidthCol[i] = iMinPixDef;
					} else {
						piWidthCol[i] = iMinPixDef;
					}		
				} else if (iType == WMSRC_GAP) {
					piWidthCol[i] = iMinPix;
				} else if (iType == WMSRC_GAPM) {
					piWidthCol[i] = iMinPix;
				} else {
					piWidthCol[i] = iMinPix;
				}

				iWidthTotal += piWidthCol[i];
				if (iWidthTotal > pRectWindow->right) {
					piWidthCol[i] -= iWidthTotal - pRectWindow->right;
					iWidthTotal = pRectWindow->right;
				}

#ifdef TRACEAUTOSIZE2
				ATLTRACE("%d ", piWidthCol[i]);
#endif
			}

			int *piHeightRow = (int*)_alloca(pheader->m_iNumRows * sizeof(int));
			int **ppiRowBegin = (int**)_alloca(pheader->m_iNumRows * sizeof(int*));

			HWND *phWnd = (HWND*)_alloca(pheader->m_iNumCols * sizeof(HWND));
			int **ppiCtrl = (int**)_alloca(pheader->m_iNumCols * sizeof(int**));
			RECT *prect = (RECT*)_alloca(pheader->m_iNumCols * sizeof(RECT));
			::memset(phWnd, 0, pheader->m_iNumCols * sizeof(HWND));
			::memset(ppiCtrl, 0, pheader->m_iNumCols * sizeof(int**));

			int iHeightTotal = pRectWindow->top;
			if (bMain) {
				iHeightTotal += pmargins->m_iTopBottom;
			}

			int iRow = 0;

#ifdef TRACEAUTOSIZE2
			ATLTRACE("\nRows: ");
#endif

			while (*piCell != WM_END) {
				ppiRowBegin[iRow] = piCell;

				if (*piCell == WMR_BEGIN) {
					int iType, iStyle, iVal;
					_TypeVal(piCell[1], iType, iStyle, iVal);

					if (iRow && !(iStyle & WMSRC_NOGAP)) {
						iHeightTotal += pmargins->m_iYGap;
					}

					if (iHeightTotal > pRectWindow->bottom) {
						iHeightTotal = pRectWindow->bottom;
					}

					int iTypeMin, iTypeMax;
					int iMinPix, iMaxPix;
					_LoadMinMax(ppiRowBegin[iRow] + 1, lYUnits, 8, iTypeMin, iMinPix, iTypeMax, iMaxPix);

					if (iType == WMSRC_AUTO) {
						piHeightRow[iRow] = iVal;
					} else if (iType == WMSRC_EXPANDABLE) {
						piHeightRow[iRow] = ::MulDiv(iTotalRowExpand, iVal, iRowExpand);
						_NormalizeColRow(piHeightRow[iRow], iMinPix, iMaxPix);
						iRowExpand -= iVal;
						iTotalRowExpand = (iTotalColExpand > piHeightRow[iRow] ? iTotalColExpand - piHeightRow[iRow] : 0);
					} else if (iType == WMSRC_CONTRACTABLE) {
						piHeightRow[iRow] = ::MulDiv(iTotalRowContract, iVal, iRowContract);
						_NormalizeColRow(piHeightRow[iRow], iMinPix, iMaxPix);
						iRowContract -= iVal;
						iTotalRowContract = (iTotalColContract > piHeightRow[iRow] ? iTotalColContract - piHeightRow[iRow] : 0);
					} else if (iType == WMSRC_EQUAL) {
						int iRowDef = iVal;
						int iTypeDef, iStyleDef, iValDef;
						do {
							_TypeVal(ppiRowBegin[iRowDef][1], iTypeDef, iStyleDef, iValDef);
						} while (iTypeDef == WMSRC_EQUAL && (iRowDef = iValDef) == iRowDef);

						int iTypeMinDef, iTypeMaxDef;
						int iMinPixDef, iMaxPixDef;
						_LoadMinMax(ppiRowBegin[iRowDef] + 1, lYUnits, 8, iTypeMinDef, iMinPixDef, iTypeMaxDef, iMaxPixDef);

						if (iTypeDef == WMSRC_AUTO) {
							piHeightRow[iRow] = iValDef;
						} else if (iTypeDef == WMSRC_EXPANDABLE) {
							piHeightRow[iRow] = ::MulDiv(iTotalRowExpand, iValDef, iRowExpand);
							_NormalizeColRow(piHeightRow[iRow], iMinPix, iMaxPix);
							iRowExpand -= iValDef;
							iTotalRowExpand = (iTotalColExpand > piHeightRow[iRow] ? iTotalColExpand - piHeightRow[iRow] : 0);
						} else if (iTypeDef == WMSRC_CONTRACTABLE) {
							piHeightRow[iRow] = ::MulDiv(iTotalRowContract, iValDef, iRowContract);
							_NormalizeColRow(piHeightRow[iRow], iMinPix, iMaxPix);
							iRowContract -= iValDef;
							iTotalRowContract = (iTotalColContract > piHeightRow[iRow] ? iTotalColContract - piHeightRow[iRow] : 0);
						} else if (iTypeDef == WMSRC_GAP) {
							piHeightRow[iRow] = iMinPixDef;
						} else if (iTypeDef == WMSRC_GAPM) {
							piHeightRow[iRow] = iMinPixDef;
						} else {
							piHeightRow[iRow] = iMinPixDef;
						}		
					} else if (iType == WMSRC_GAP) {
						piHeightRow[iRow] = iMinPix;
					} else if (iType == WMSRC_GAPM) {
						piHeightRow[iRow] = iMinPix;
					} else {
						piHeightRow[iRow] = iMinPix;
					}

					if (iHeightTotal + piHeightRow[iRow] > pRectWindow->bottom) {
						piHeightRow[iRow] = pRectWindow->bottom - iHeightTotal;
					}

#ifdef TRACEAUTOSIZE2
					ATLTRACE("%d ", piHeightRow[iRow]);
#endif

					piCell += 4;

					int iCol = 0;
					RECT *pLastRect = NULL;

					int iWidthTotal = pRectWindow->left;
					if (bMain) {
						iWidthTotal += pmargins->m_iLeftRight;
					}

					while (*piCell != WM_END && *piCell != WMR_END) {
						if (iCol < pheader->m_iNumCols) {
							if (iCol && !(piHeadCols[iCol * 3] & WMSRC_NOGAP)) {
								iWidthTotal += pmargins->m_iXGap;
							}

							if (iWidthTotal > pRectWindow->right) {
								iWidthTotal = pRectWindow->right;
							}

							if (*piCell != WMSCTRL_EXPDOWN) {
								if (!_MoveCtrl(iCol, hdwp, phWnd, ppiCtrl, prect, pmargins, lXUnits, lYUnits)) {
									return FALSE;
								}

								if (*piCell != WMSCTRL_EMPTY && *piCell != WMSCTRL_EXPRIGHT) {
									prect[iCol].left = iWidthTotal;
									prect[iCol].top = iHeightTotal;
									prect[iCol].right = piWidthCol[iCol];
									prect[iCol].bottom = piHeightRow[iRow];
									pLastRect = &prect[iCol];

									if (*piCell != WMH_BEGIN) {
										phWnd[iCol] = pT->GetDlgItem(*piCell);
										ppiCtrl[iCol] = NULL;
										if (!phWnd[iCol]) {
											phWnd[iCol] = (HWND)-1;
											ATLTRACE(_T("Inexistant ctrl in row: %d, col: %d. Ignored.\n"), iRow, iCol);
										}
									} else {
										phWnd[iCol] = NULL;
										ppiCtrl[iCol] = piCell;
										piCell += piCell[1];
									}
								} else {
									phWnd[iCol] = NULL;
									ppiCtrl[iCol] = NULL;
									if (*piCell != WMSCTRL_EXPRIGHT) {
										pLastRect = NULL;
									}
								}
							} 

							if (*piCell == WMSCTRL_EXPRIGHT) {
								if (pLastRect) {
									pLastRect->right += piWidthCol[iCol];
									if ((piHeadCols[iCol * 3] & WMSRC_NOGAP) != WMSRC_NOGAP && iCol) {
										pLastRect->right += pmargins->m_iXGap;
										if (pLastRect->left + pLastRect->right > pRectWindow->right) {
											pLastRect->right = pRectWindow->right - pLastRect->left;
										}
									}
								} else {
									ATLTRACE(_T("Wrong continue right flag in row: %d, column: %d. Ignored.\n"), iRow, iCol);
								}
							} else if (*piCell == WMSCTRL_EXPDOWN) {
								if (phWnd[iCol] || ppiCtrl[iCol]) {
									prect[iCol].bottom += piHeightRow[iRow];
									if (iRow && !(iStyle & WMSRC_NOGAP)) {
										prect[iCol].bottom += pmargins->m_iYGap;
										if (prect[iCol].top + prect[iCol].bottom > pRectWindow->bottom) {
											prect[iCol].bottom = pRectWindow->bottom - prect[iCol].top;
										}
									}
								} else {
									ATLTRACE(_T("Wrong continue down flag in row: %d, column: %d. Ignored.\n"), iRow, iCol);
								}
								pLastRect = NULL;
							}

							iWidthTotal += piWidthCol[iCol];
						}

						iCol++;
						piCell++;
					}

					for (int i = iCol; i < pheader->m_iNumCols; i++) {
						if (!_MoveCtrl(i, hdwp, phWnd, ppiCtrl, prect, pmargins, lXUnits, lYUnits)) {
							return FALSE;
						}
					}
				} else {
					ATLTRACE(_T("Malformed row: %d. Skipped. \n"), iRow);
					while (*piCell != WMR_END && *piCell != WM_END) {
						piCell++;
					}

					for (int i = 0; i < pheader->m_iNumCols; i++) {
						if (!_MoveCtrl(i, hdwp, phWnd, ppiCtrl, prect, pmargins, lXUnits, lYUnits)) {
							return FALSE;
						}
					}

					piHeightRow[iRow] = 0;
				}

				iHeightTotal += piHeightRow[iRow];

				if (*piCell != WM_END) {
					piCell++;
				} else {
					ATLTRACE(_T("Malformed endrow in row: %d. Ignored.\n"), iRow);
				}

				iRow++;
			} 			

			for (int i = 0; i < pheader->m_iNumCols; i++) {
				if (!_MoveCtrl(i, hdwp, phWnd, ppiCtrl, prect, pmargins, lXUnits, lYUnits)) {
					return FALSE;
				}
			}
		}

		ATLASSERT(*piCell == WM_END);

		return TRUE;
	}

	BOOL _MoveCtrl(int iCol, HDWP &hdwp, HWND *phWnd, int **ppiCtrl, RECT *prect, CMargins *pmargins, LONG lXUnits, LONG lYUnits)
	{
		if (phWnd[iCol] && phWnd[iCol] != (HWND)-1) {
			POINT pt = {0};
			__if_exists(T::GetScrollOffset) {
				T* pT = static_cast<T*>(this);
				pT->GetScrollOffset(pt);
			}

			UINT uiFlags = ::GetWindowLong(phWnd[iCol], GWL_EXSTYLE) & WS_EX_TRANSPARENT ? SWP_NOCOPYBITS : 0;
			ATLVERIFY((hdwp = ::DeferWindowPos(hdwp, phWnd[iCol], NULL, prect[iCol].left - pt.x, prect[iCol].top - pt.y, prect[iCol].right, prect[iCol].bottom, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER|uiFlags)) != NULL);
			if (!hdwp) {
				return FALSE;
			}
			phWnd[iCol] = NULL;
		} else if (ppiCtrl[iCol]) {
			RECT rect = {prect[iCol].left, prect[iCol].top, prect[iCol].left + prect[iCol].right, prect[iCol].top + prect[iCol].bottom};
			if (!_CtrlsArrange(hdwp, (CCtrlGroup*)ppiCtrl[iCol], pmargins, lXUnits, lYUnits, &rect, FALSE)) {
				return FALSE;
			}

			ppiCtrl[iCol] = NULL;
		}

		return TRUE;
	}

	/// Converts margins and gaps from DLGU or Pixels to Pixels
	static void _CheckMargins(long lXUnits, int iXUnitsBase, long lYUnits, int iYUnitsBase, const CMargins *pmargins, CMargins *pmarginsPix)
	{
		pmarginsPix->m_iLeftRight = (pmargins->m_iLeftRight > 0 ? ::MulDiv(pmargins->m_iLeftRight, lXUnits, iXUnitsBase) : -pmargins->m_iLeftRight);
		pmarginsPix->m_iTopBottom = (pmargins->m_iTopBottom > 0 ? ::MulDiv(pmargins->m_iTopBottom, lYUnits, iYUnitsBase) : -pmargins->m_iTopBottom);
		pmarginsPix->m_iXGap = (pmargins->m_iXGap > 0 ? ::MulDiv(pmargins->m_iXGap, lXUnits, iXUnitsBase) : -pmargins->m_iXGap);
		pmarginsPix->m_iYGap = (pmargins->m_iYGap > 0 ? ::MulDiv(pmargins->m_iYGap, lYUnits, iYUnitsBase) : -pmargins->m_iYGap);
	}

	/// Splits the type of a Row/Col in Type, ExStyle and Val
	static void _TypeVal(int iBase, int &iType, int &iStyle, int &iVal)
	{
		if (iBase >= 0) {
			iType = iBase & WMSRC_TYPEMASK;
			iStyle = iBase & WMSRC_EXSTYLEMASK;
			iVal = iBase & WMSRC_VALMASK;
		} else {
			iBase = 0 - iBase;
			iType = iBase & WMSRC_TYPEMASK;
			iStyle = iBase & WMSRC_EXSTYLEMASK;
			iVal = -(int)(iBase & WMSRC_VALMASK);
		}
	}

	static void _SetVariableMinMax(int iCR, int *piColRow)
	{
		_SetVariableMinMax(iCR, piColRow[1]);
		_SetVariableMinMax(iCR, piColRow[2], WMSRCMM_MAXVAL);
	}

	static void _SetVariableMinMax(int iCR, int &iBase, int iVal = 0)
	{
		iCR;

		BOOL bNeg = iBase < 0;

		if (bNeg) {
			iBase = -iBase;
		}

		if (iBase & WMSRCMM_SIZECALC) {
			iBase = WMSRCMM_SIZECALC|WMSRCMM_NEEDCALC|iVal;
			bNeg = FALSE;
		} else if (iBase & WMSRCMM_NEEDCALC) {
			ATLTRACE(_T("Wrong min-max flag in column (or row): %d. Fixed.\n"), iCR);
			iBase &= ~WMSRCMM_NEEDCALC;
		}

		if (bNeg) {
			iBase = -iBase;
		}
	}

	static void _CheckEqualMinMax(int iCR, int *piColRow)
	{
		_CheckEqualMinMax(iCR, piColRow[1]);
		_CheckEqualMinMax(iCR, piColRow[2]);
	}

	static void _CheckEqualMinMax(int iCR, int &iBase)
	{
		iCR;

		BOOL bNeg = iBase < 0;

		if (bNeg) {
			iBase = -iBase;
		}

		if (!(iBase & WMSRCMM_SIZECALC)) {
			ATLTRACE(_T("Equal columns/rows can't have min/max value. Ignored value in column (or row) %d.\n"), iCR);
		} else if (iBase & WMSRCMM_NEEDCALC) {
			ATLTRACE(_T("Wrong min-max flag in column (or row): %d. Fixed.\n"), iCR);
		}

		iBase = WMSRCMM_SIZECALC;
	}

	static void _CheckFixedMinMax(int iCR, int *piColRow, long lXYUnits, int iXYUnitsBase, int iVal)
	{
		_CheckFixedMinMax(iCR, piColRow[1], lXYUnits, iXYUnitsBase, iVal);
		_CheckFixedMinMax(iCR, piColRow[2], lXYUnits, iXYUnitsBase, iVal);
	}

	static void _CheckFixedMinMax(int iCR, int &iBase, long lXYUnits, int iXYUnitsBase, int iVal)
	{
		iCR;

		BOOL bNeg = iBase < 0;

		if (bNeg) {
			iBase = -iBase;
		}

		if (iBase & WMSRCMM_SIZECALC) {
			iBase = WMSRCMM_SIZECALC|iVal;
		} else {
			if (iBase & WMSRCMM_NEEDCALC) {
				ATLTRACE(_T("Wrong min-max flag in column (or row): %d. Fixed.\n"), iCR);
				iBase &= ~WMSRCMM_NEEDCALC;
			}

			int iBasePix = (iBase > 0 ? ::MulDiv(iBase, lXYUnits, iXYUnitsBase) : -iBase);

			if (iBasePix != iVal) {
				ATLTRACE(_T("Wrong min-max value in column (or row): %d. Fixed.\n"), iCR);
				iBase = iVal;
				bNeg = FALSE;
			}
		}

		if (bNeg) {
			iBase = -iBase;
		}
	}

	static void _CalcAuto(int &iVal, int iMin)
	{
		if (iVal != INT_MIN) {
			if (iMin > iVal) {
				iVal = iMin;
			}
		}
	}

	static void _CheckAutoMinMax(int *piColRow, int iVal)
	{
		{
			BOOL bNeg = piColRow[1] < 0;

			if (bNeg) {
				piColRow[1] = -piColRow[1];
			}

			if (piColRow[1] & WMSRCMM_SIZECALC) {
				piColRow[1] = WMSRCMM_SIZECALC|iVal;
			}

			if (bNeg) {
				piColRow[1] = -piColRow[1];
			}
		}

		{
			BOOL bNeg = piColRow[2] < 0;

			if (bNeg) {
				piColRow[2] = -piColRow[2];
			}
			
			if (piColRow[2] & WMSRCMM_SIZECALC) {
				piColRow[2] = WMSRCMM_SIZECALC|iVal;
			}

			if (bNeg) {
				piColRow[2] = -piColRow[2];
			}
		}
	}

	static void _CalcMinMaxAuto(int *piColRow, int iMin, int iMax)
	{
		{
			BOOL bNeg = piColRow[1] < 0;

			if (bNeg) {
				piColRow[1] = -piColRow[1];
			}

			if (piColRow[1] & WMSRCMM_NEEDCALC) {
				iMin &= WMSRCMM_VALMASK;
				int iMinPix = piColRow[1] & WMSRCMM_VALMASK;
				if (iMin > iMinPix) {
					piColRow[1] = WMSRCMM_SIZECALC|iMin;
				}
			}

			if (bNeg) {
				piColRow[1] = -piColRow[1];
			}
		}

		{
			BOOL bNeg = piColRow[2] < 0;

			if (bNeg) {
				piColRow[2] = -piColRow[2];
			}

			if (piColRow[2] & WMSRCMM_NEEDCALC) {
				iMax &= WMSRCMM_VALMASK;
				int iMaxPix = piColRow[2] & WMSRCMM_VALMASK;
				if (iMax < iMaxPix) {
					piColRow[2] = WMSRCMM_SIZECALC|iMax;
				}
			}

			if (bNeg) {
				piColRow[2] = -piColRow[2];
			}
		}
	}

	static void _SetAuto(int *piColRow, long lXYUnits, int iXYUnitsBase, int iVal)
	{
		int iTypeMin, iTypeMax;
		int iMinPix, iMaxPix;
		_LoadMinMax(piColRow, lXYUnits, iXYUnitsBase, iTypeMin, iMinPix, iTypeMax, iMaxPix);

		_NormalizeColRow(iVal, iMinPix, iMaxPix);

		piColRow[0] &= ~WMSRC_VALMASK;
		piColRow[0] |= iVal & WMSRC_VALMASK;
	}

	static void _AddMinMax(int iCR, int *piColRow, long lXYUnits, int iXYUnitsBase, int &iFixed, int iType, int &iMin, int_no &iMax)
	{
		iCR;
		int iTypeMin, iTypeMax;
		int iMinPix, iMaxPix;
		_LoadMinMax(piColRow, lXYUnits, iXYUnitsBase, iTypeMin, iMinPix, iTypeMax, iMaxPix);

		if (iTypeMin) {
			if (iTypeMax) {
				if (iMinPix > iMaxPix) {
					ATLTRACE(_T("Auto-min-value > auto-max-value in column (or row): %d. Fixed. Now auto-max-value = Auto-min-value.\n"), iCR);
					if (piColRow[2] < 0) {
						piColRow[2] = -piColRow[2];
					}
					piColRow[2] &= WMSRCMM_TYPEMASK;
					piColRow[2] |= iMinPix;
					iMaxPix = iMinPix;
				}
			} else {
				if (iMinPix > iMaxPix) {
					ATLTRACE(_T("Auto-min-value > max-value in column (or row): %d. Fixed. Now auto-min-value = man-value.\n"), iCR);
					if (piColRow[1] < 0) {
						piColRow[1] = -piColRow[1];
					}
					piColRow[1] &= WMSRCMM_TYPEMASK;
					piColRow[1] |= iMaxPix;
					iMinPix = iMaxPix;
				}
			}
		} else if (iTypeMax) {
			if (iMinPix > iMaxPix) {
				ATLTRACE(_T("Min-value > auto-max-value in column (or row): %d. Fixed. Now auto-max-value = min-value.\n"), iCR);
				if (piColRow[2] < 0) {
					piColRow[2] = -piColRow[2];
				}
				piColRow[2] &= WMSRCMM_TYPEMASK;
				piColRow[2] |= iMinPix;
				iMaxPix = iMinPix;
			}
		} else {
			if (iMinPix > iMaxPix) {
				ATLTRACE(_T("Min-value > max-value in column (or row): %d. Fixed. Now max-value = min-value.\n"), iCR);
				piColRow[2] = iMinPix;
			}
		}

		iMin += iMinPix;

		if (iType != WMSRC_CONTRACTABLE) {
			iMax += iMaxPix;
		}

		if (iType != WMSRC_EXPANDABLE && iType != WMSRC_CONTRACTABLE) {
			iFixed += iMinPix;
		}
	}

	static void _LoadMinMax(int *piColRow, long lXYUnits, int iXYUnitsBase, int &iTypeMin, int &iMin, int &iTypeMax, int &iMax)
	{
		if (piColRow[1] >= 0) {
			iTypeMin = piColRow[1] & WMSRCMM_TYPEMASK;
			iMin = piColRow[1] & WMSRCMM_VALMASK;
		} else {
			iTypeMin = (-piColRow[1]) & WMSRCMM_TYPEMASK;
			iMin = -(int)((-piColRow[1]) & WMSRCMM_VALMASK);
		}

		if (!iTypeMin) {
			iMin = (iMin > 0 ? ::MulDiv(iMin, lXYUnits, iXYUnitsBase) : -iMin);
		}

		if (piColRow[2] >= 0) {
			iTypeMax = piColRow[2] & WMSRCMM_TYPEMASK;
			iMax = piColRow[2] & WMSRCMM_VALMASK;
		} else {
			iTypeMax = (-piColRow[2]) & WMSRCMM_TYPEMASK;
			iMax = -(int)((-piColRow[2]) & WMSRCMM_VALMASK);
		}

		if (iMax == WMSRCMM_MAXVAL) {
			iMax = INT_MAX;
		} else if (!iTypeMax) {
			iMax = (iMax > 0 ? ::MulDiv(iMax, lXYUnits, iXYUnitsBase) : -iMax);
		}
	}

	static void _NormalizeColRow(int &iVal, int iMin, int iMax)
	{
		if (iVal < iMin) {
			iVal = iMin;
		} else if (iVal > iMax) {
			iVal = iMax;
		}
	}

	static void _Init(CRowsIndex *&p, CRowsIndex *q)
	{
		q->m_piNum = NULL;
		q->m_pPrev = p;
		p = q;
	}

	static int _DeInit(CRowsIndex *&p, int iRet)
	{
		p = p->m_pPrev;
		return iRet;
	}

	static void _IncRow(CRowsIndex *q)
	{
		(*q->m_piNum)++;
	}

	static int _FirstParam(int a = 0, int b = 0, int c = 0, int d = 0)
	{
		b; c; d;
		return a;
	}

	static int _SecondParam(int a = 0, int b = 0, int c = 0, int d = 0)
	{
		a; c; d;
		return b;
	}

	static int _ThirdParam(int a = 0, int b = 0, int c = 0, int d = 0)
	{
		a; b; d;
		return c;
	}

	static int _FourthParam(int a = 0, int b = 0, int c = 0, int d = 0)
	{
		a; b; c;
		return d;
	}

	//

protected:
	BOOL InitializeTransparentCtrl(int iCtrl)
	{
		T* pT = static_cast<T*>(this);
		HWND hWnd = pT->GetDlgItem(iCtrl);

		if (hWnd) {
			TCHAR szName[256];
			ATLVERIFY(::GetClassName(hWnd, szName, 256));

			WNDPROC wp;
			if (::_tcscmp(_T("Button"), szName) == 0) {
				wp = TransparentWindowProc<1>;
			} else {
				wp = TransparentWindowProc<0>;
			}

			::ShowWindow(hWnd, SW_HIDE);
			WNDPROC wpOld = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)wp);
			ATLVERIFY(::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)wpOld) == 0);
		}

		return TRUE;
	}

	BOOL DrawTransparentCtrl(WTL::CDCHandle dc, int iCtrl)
	{
		T* pT = static_cast<T*>(this);
		HWND hWnd = pT->GetDlgItem(iCtrl);

		if (hWnd) {
			RECT rect;
			::GetWindowRect(hWnd, &rect);
			::MapWindowPoints(NULL, pT->m_hWnd, (LPPOINT)&rect, 1);

			POINT pt;
			dc.SetViewportOrg(rect.left, rect.top, &pt);

			::SendMessage(hWnd, WM_PRINT, (WPARAM)dc.m_hDC, PRF_CHILDREN|PRF_CLIENT|PRF_NONCLIENT|PRF_OWNED);

			dc.SetViewportOrg(pt.x, pt.y);
		}

		return TRUE;
	}

	template <int t_iType>
	static LRESULT CALLBACK TransparentWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		int iOffset, iOffsetTop;

		if (t_iType) {
			iOffsetTop = 5;
			iOffset = 5;
		} else {
			iOffsetTop = 5;
			iOffset = 5;
		}

		if (uMsg == WM_WINDOWPOSCHANGING) {
			WINDOWPOS *pwp = (WINDOWPOS*)lParam;

			if ((pwp->flags & (SWP_NOSIZE|SWP_NOMOVE)) == 0) {
				HWND hWndParent = ::GetParent(hWnd);
				RECT rectOld, rectNew, rectMax;
				::GetWindowRect(hWnd, &rectOld);
				::MapWindowPoints(NULL, hWndParent, (LPPOINT)&rectOld, 2);

				if ((pwp->flags & SWP_NOMOVE) == 0) {
					rectNew.left = pwp->x;
					rectNew.top = pwp->y;
				} else {
					rectNew.left = rectOld.left;
					rectNew.top = rectOld.top;
				}

				if ((pwp->flags & SWP_NOSIZE) == 0) {
					rectNew.right = pwp->cx;
					rectNew.bottom = pwp->cy;
				} else {
					rectNew.right = rectOld.right - rectOld.left;
					rectNew.bottom = rectOld.bottom - rectOld.top;
				}

				rectNew.right += rectNew.left;
				rectNew.bottom += rectNew.top;

				rectMax.left = min(rectOld.left, rectNew.left);
				rectMax.top = min(rectOld.top, rectNew.top);
				rectMax.right = max(rectOld.right, rectNew.right);
				rectMax.bottom = max(rectOld.bottom, rectNew.bottom);

				int iAreaOld = (rectOld.right - rectOld.left) * (rectOld.bottom - rectOld.top);
				int iAreaNew = (rectNew.right - rectNew.left) * (rectNew.bottom - rectNew.top);
				RECT rectInv[4];
				int iArea[] = {0, 0, 0, 0};

				if (rectOld.left < rectNew.left) {
					RECT rect = {rectOld.left, rectMax.top, rectNew.left + iOffset, rectMax.bottom};
					iArea[0] = (rect.right - rect.left) * (rect.bottom - rect.top);
					rectInv[0] = rect;
				} else if (rectOld.left > rectNew.left) {
					RECT rect = {rectNew.left, rectMax.top, rectOld.left + iOffset, rectMax.bottom};
					iArea[0] = (rect.right - rect.left) * (rect.bottom - rect.top);
					rectInv[0] = rect;
				}

				if (rectOld.right < rectNew.right) {
					RECT rect = {rectOld.right - iOffset, rectMax.top, rectNew.right, rectMax.bottom};
					iArea[1] = (rect.right - rect.left) * (rect.bottom - rect.top);
					rectInv[1] = rect;
				} else if (rectOld.right > rectNew.right) {
					RECT rect = {rectNew.right - iOffset, rectMax.top, rectOld.right, rectMax.bottom};
					iArea[1] = (rect.right - rect.left) * (rect.bottom - rect.top);
					rectInv[1] = rect;
				}

				if (rectOld.top != rectNew.top) {
					if (t_iType) {
						HWND hWndParent = ::GetParent(hWnd);
						int iHeight = _GetFontSize(hWnd, hWndParent);

						iOffsetTop += iHeight / 2;
					}

					if (rectOld.top < rectNew.top) {
						RECT rect = {rectMax.left, rectOld.top, rectMax.right, rectNew.top + iOffsetTop};
						iArea[2] = (rect.right - rect.left) * (rect.bottom - rect.top);
						rectInv[2] = rect;
					} else if (rectOld.top > rectNew.top) {
						RECT rect = {rectMax.left, rectNew.top, rectMax.right, rectOld.top + iOffsetTop};
						iArea[2] = (rect.right - rect.left) * (rect.bottom - rect.top);
						rectInv[2] = rect;
					}
				}

				if (rectOld.bottom < rectNew.bottom) {
					RECT rect = {rectMax.left, rectOld.bottom - iOffset, rectMax.right, rectNew.bottom};
					iArea[3] = (rect.right - rect.left) * (rect.bottom - rect.top);
					rectInv[3] = rect;
				} else if (rectOld.bottom > rectNew.bottom) {
					RECT rect = {rectMax.left, rectNew.bottom - iOffset, rectMax.right, rectOld.bottom};
					iArea[3] = (rect.right - rect.left) * (rect.bottom - rect.top);
					rectInv[3] = rect;
				}

				int iAreaTot1 = iAreaOld + iAreaNew;
				int iAreaTot2 = iArea[0] + iArea[1] + iArea[2] + iArea[3];

				if (iAreaTot1 <= iAreaTot2) {
					::InvalidateRect(hWndParent, &rectOld, TRUE);
					::InvalidateRect(hWndParent, &rectNew, TRUE);
				} else {
					for (int i = 0; i < 4; i++) {
						if (iArea[i]) {
							::InvalidateRect(hWndParent, &rectInv[i], TRUE);
						}
					}
				}
			}
		} else if (t_iType && uMsg == WM_UPDATEUISTATE) {
			if (HIWORD(wParam) & UISF_HIDEACCEL) {
				HWND hWndParent = ::GetParent(hWnd);
				int iHeight = _GetFontSize(hWnd, hWndParent);

				RECT rect;
				::GetWindowRect(hWnd, &rect);
				::MapWindowPoints(NULL, hWndParent, (LPPOINT)&rect, 2);
				rect.bottom = rect.top + iHeight + 2;

				::InvalidateRect(hWndParent, &rect, TRUE);
			}
		}

		WNDPROC wp = (WNDPROC)(LONG_PTR)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
		return ::CallWindowProc(wp, hWnd, uMsg, wParam, lParam);
	}

	static int _GetFontSize(HWND hWnd, HWND hWndParent)
	{
		HFONT hFont = (HFONT)::SendMessage(hWnd, WM_GETFONT, 0, 0);

		HDC hDC = ::GetDC(hWndParent);

		HFONT hOldFont = (HFONT)::SelectObject(hDC, (HGDIOBJ)hFont);

		TEXTMETRIC tm;
		::GetTextMetrics(hDC, &tm);

		::SelectObject(hDC, (HGDIOBJ)hOldFont);

		return tm.tmHeight - tm.tmDescent;

//		Not used anymore (better results with GetTextMetrics):
//		LOGFONT lf;
//		::GetObject(hFont, sizeof(LOGFONT), (HGDIOBJ)&lf)
//		return -MulDiv(lf.lfHeight, 72, dc.GetDeviceCaps(LOGPIXELSY));
	}
};

//

} // namespace ATL

//

#endif // __ATLAUTOSIZEDLG_H__