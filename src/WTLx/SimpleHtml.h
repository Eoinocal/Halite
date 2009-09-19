#if !defined(AFX_SIMPLEHTMLCTRL_H__20020223_765E_434E_641B_0080AD509054__INCLUDED_)
#define AFX_SIMPLEHTMLCTRL_H__20020223_765E_434E_641B_0080AD509054__INCLUDED_

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CSimpleHtmlCtrl - A RTF-based HTML Viewer
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2002 Bjarke Viksoe.
//
// Note:
//  This control handles ASCII text only. It
//  will not handle MBCS nor UTF-8 HTML
//  correctly.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is
// not sold for profit without the authors written consent, and
// providing that this notice and the authors name is included.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#ifndef __cplusplus
  #error WTL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
  #error SimpleHtmlCtrl.h requires atlapp.h to be included first
#endif

#ifndef __ATLCTRLS_H__
  #error SimpleHtmlCtrl.h requires atlctrls.h to be included first
#endif

#if _RICHEDIT_VER < 0x0200
  #pragma message("Warning: The SimpleHtmlCtrl is limited without RichEdit Version 2")
#endif



/////////////////////////////////////////////////////////////////////////////
// Convert HTML to RTF

class CHtmlToRtf
{
public:
   typedef struct LINK {
      TCHAR szURL[128];
      int nStart;
      int nEnd;
   };
protected:
   typedef struct CONTEXT
   {
      TCHAR szFaceName[LF_FACESIZE];
      int lFontSize;
      int lTextColor;
      int lBackColor;
      int lCellSpacing;
      int lCellPadding;
      int lBorder;
      int lLeftIndent;
      int lRightIndent;
      int lTopIndent;
      int lBottomIndent;
   };
   CSimpleValArray<LOGFONT> m_aFonts;
   CSimpleValArray<COLORREF> m_aColors;
   CSimpleValArray<LINK> m_aLinks;
   COLORREF m_clrText;
   COLORREF m_clrBack;
   int m_nCharPos;
   bool m_bEndLine;
   bool m_bWasSpace;

public:
   LPCTSTR Convert(LPCTSTR pstrHTML, LOGFONT lf, COLORREF& clrText, COLORREF& clrBack)
   {
      // Create a new output buffer
      // We assume that we can create a corretly sized output
      // buffer before we convert HTML to RTF...
      const int SIZE_FACTOR = 2;
      unsigned int nTextSize = _tcslen(pstrHTML);
      unsigned int nSize = nTextSize * SIZE_FACTOR;
      if( nSize < 1000 ) nSize = 1000;
      LPTSTR pstrBuffer = (LPTSTR) malloc( nSize*sizeof(TCHAR) );
      ATLASSERT(pstrBuffer);

      // Initialize structures
      m_aFonts.RemoveAll();
      m_aFonts.Add(lf);
      m_aColors.RemoveAll();
      m_aColors.Add(clrText);
      m_aColors.Add(clrBack);
      m_aLinks.RemoveAll();

      // Parse HTML and convert it into RTF
      LPTSTR pstrDest = pstrBuffer;
      m_clrBack = clrBack;
      bool bRes = Parse(pstrHTML, pstrDest);
      *pstrDest = _T('\0');
      ATLASSERT(_tcslen(pstrBuffer) < nSize);
      if( !bRes ) {
         free(pstrBuffer);
         return NULL;
      }
      clrBack = m_clrBack;

      // Construct the actual return buffer
      LPTSTR pstrText = (LPTSTR) malloc( nSize*sizeof(TCHAR) );
      ATLASSERT(pstrText);

      // Prepare RTF document...
      pstrDest = pstrText;
      AddOutput(pstrDest, _T("{\\rtf1\\ansi"));
      // Add fonts
      AddOutput(pstrDest, _T("\\deff0{\\fonttbl"));
      for( int i=0; i<m_aFonts.GetSize(); i++ ) {
         LOGFONT lf = m_aFonts[i];
         AddOutputV(pstrDest, _T("{\\f%d\\fnil\\fcharset0 %s;}"), i, lf.lfFaceName);
      }
      AddOutput(pstrDest, _T("}"));
      // Add colors
      AddOutput(pstrDest, _T("{\\colortbl ;"));
      for( i=0; i<m_aColors.GetSize(); i++ ) {
         COLORREF clr = m_aColors[i];
         AddOutputV(pstrDest, _T("\\red%d\\green%d\\blue%d;"),
            GetRValue(clr), GetGValue(clr), GetBValue(clr));
      }
      AddOutput(pstrDest, _T("}"));
      // Some other stuff
      AddOutput(pstrDest, _T("\\pard\\plain\\tx0"));
      AddOutputV(pstrDest, _T("\\fs%d"), GetFontSize(lf.lfHeight));
      // Add converted buffer
      AddOutput(pstrDest, pstrBuffer);
      // Almost done
      AddOutput(pstrDest, _T("}"));
      *pstrDest = _T('\0');

      free(pstrBuffer);
      return pstrText;
   }
   bool GetLink(int iIndex, LINK& link) const
   {
      if( iIndex<0 || iIndex>=m_aLinks.GetSize() ) return false;
      link = m_aLinks[iIndex];
      return true;
   }

   bool Parse(LPCTSTR pszStart, LPTSTR& pstrDest)
   {
      // This method builds the RTF document based on the input HTML.
      // A lot of it is basic conversion, but a few tags require special
      // attention. We also keep track of the assumed character-position while
      // generating the RTF. This is particular difficult since this is specific
      // to the Microsoft RTF-specification implementation. We need to do this
      // because we later on must change the text formatting of links (anchor tags).
      CSimpleArray<CONTEXT> aContexts;
      CONTEXT Context = { 0 };
      _tcscpy( Context.szFaceName, m_aFonts[0].lfFaceName );
      Context.lFontSize = GetFontSize( m_aFonts[0].lfHeight );
      Context.lTextColor = 0;
      Context.lBackColor = 1;
      Context.lLeftIndent = Context.lRightIndent = Context.lTopIndent = Context.lBottomIndent = 0;

      int nCellPos = 0;
      long lTwipFactor = (1L * 1440L) / ::GetDeviceCaps(::GetDC(NULL), LOGPIXELSX);
      m_bEndLine = false;
      m_bWasSpace = true;
      m_nCharPos = 0;

      LINK Link;
      TCHAR szAppendScope[1024];             // Attribute string currently being built
      TCHAR szValue[128];                    // Tag value buffer

      while( *pszStart ) {
         // Add text before next tag
         AddText(pstrDest, pszStart);
         if( *pszStart == _T('\0') ) return true; // No more tags present?!
         ATLASSERT(*pszStart == _T('<'));
         pszStart++;

         // Extract the complete start-tag
         TCHAR szTag[128];
         LPTSTR pszTag = szTag;
         while( *pszStart && *pszStart!=_T('>') ) *pszTag++ = *pszStart++;
         ATLASSERT(*pszStart == _T('>'));
         *pszTag = _T('\0');
         pszStart++;
         ::CharLowerBuff(szTag, _tcslen(szTag));

         // Extract the tag-name
         TCHAR szTagName[16];
         LPTSTR pszTagName = szTagName;
         pszTag = szTag;
         while( *pszTag && (*pszTag == _T('/') || _istalnum(*pszTag)) ) *pszTagName++ = *pszTag++;
         *pszTagName = _T('\0');

         // Tag attributes starts where tag-name ends
         pszTag = szTag + _tcslen(szTagName);
         while( _istspace(*pszTag) ) *pszTag++;

         // Parse tag...
         CONTEXT OldContext = Context;          // Store old context (for comparison later)
         LPTSTR pszAppendScope = szAppendScope;
         bool bNewContext = false;              // New Scope defined?
         bool bRestoreContext = false;          // Closed scope?
         bool bHadEndLine = m_bEndLine;         // Keep track of EOF for space trimming.
         m_bEndLine = false;

         if( szTagName[0]!=_T('/') ) {
            //
            // Open tags
            //
            if( _tcscmp(szTagName, _T("b")) == 0 || _tcscmp(szTagName, _T("strong")) == 0 ) {
               AddOutput(pszAppendScope, _T("\\b1 "));
            }
            else if( _tcscmp(szTagName, _T("i")) == 0 ) {
               AddOutput(pszAppendScope, _T("\\i1 "));
            }
            else if( _tcscmp(szTagName, _T("u")) == 0 ) {
               AddOutput(pszAppendScope, _T("\\ul1 "));
            }
            else if( _tcscmp(szTagName, _T("br")) == 0 ) {
               AddOutput(pszAppendScope, _T("\\line "));
               m_nCharPos++;
               m_bEndLine = true;
               m_bWasSpace = true;
            }
            else if( _tcscmp(szTagName, _T("p")) == 0 ) {
               if( !bHadEndLine ) {
                  AddOutput(pszAppendScope, _T("\\par"));
                  m_nCharPos++;
               }
               AddOutput(pszAppendScope, _T("\\par "));
               m_nCharPos++;
               m_bEndLine = true;
               m_bWasSpace = true;
            }
            else if( _tcscmp(szTagName, _T("body")) == 0 ) {
               if( GetAttribute(pszTag, _T("bgcolor"), szValue) ) {
                  Context.lBackColor = GetColor(szValue);
                  // Assign this color to the control's background
                  m_clrBack = m_aColors[Context.lBackColor];
               }
               Context.lLeftIndent =
               Context.lRightIndent =
               Context.lTopIndent =
               Context.lBottomIndent = 8;
               if( GetAttribute(pszTag, _T("leftmargin"), szValue) ) {
                  Context.lLeftIndent = GetInteger(szValue);
               }
               if( GetAttribute(pszTag, _T("rightmargin"), szValue) ) {
                  Context.lRightIndent = GetInteger(szValue);
               }
               if( GetAttribute(pszTag, _T("topmargin"), szValue) ) {
                  Context.lTopIndent = GetInteger(szValue);
               }
               if( GetAttribute(pszTag, _T("bottommargin"), szValue) ) {
                  Context.lBottomIndent = GetInteger(szValue);
               }
               AddOutputV(pszAppendScope, _T("\\margt%d"), Context.lTopIndent * lTwipFactor);
               AddOutputV(pszAppendScope, _T("\\margb%d"), Context.lBottomIndent * lTwipFactor);
               AddOutputV(pszAppendScope, _T("\\li%d"), Context.lLeftIndent * lTwipFactor);
               AddOutputV(pszAppendScope, _T("\\ri%d"), Context.lRightIndent * lTwipFactor);
               bNewContext = true;
            }
            else if( _tcscmp(szTagName, _T("font")) == 0 ) {
               if( GetAttribute(pszTag, _T("color"), szValue) ) {
                  Context.lTextColor = GetColor(szValue);
               }
               if( GetAttribute(pszTag, _T("size"), szValue) ) {
                  Context.lFontSize = GetFontSize(szValue);
               }
               if( GetAttribute(pszTag, _T("face"), szValue) ) {
                  _tcscpy( Context.szFaceName, szValue );
               }
               bNewContext = true;
            }
            else if( _tcscmp(szTagName, _T("a")) == 0 ) {
               if( GetAttribute(pszTag, _T("href"), szValue) ) {
                  // Add the actual URL as a hidden test
                  Link.nStart = m_nCharPos;
                  _tcscpy( Link.szURL, szValue );
               }
               bNewContext = true;
            }
            else if( _tcscmp(szTagName, _T("span")) == 0 ) {
               bNewContext = true;
            }
            else if( _tcscmp(szTagName, _T("div")) == 0 ) {
               if( !bHadEndLine ) {
                  AddOutput(pszAppendScope, _T("\\par "));
                  m_nCharPos++;
                  m_bEndLine = true;
                  m_bWasSpace = true;
               }
               if( GetAttribute(pszTag, _T("align"), szValue) ) {
                  if( _tcscmp(szValue, _T("center")) == 0 ) AddOutput(pszAppendScope, _T("\\qc "));
                  else if( _tcscmp(szValue, _T("left")) == 0 ) AddOutput(pszAppendScope, _T("\\ql "));
                  else if( _tcscmp(szValue, _T("right")) == 0 ) AddOutput(pszAppendScope, _T("\\qr "));
                  else if( _tcscmp(szValue, _T("justify")) == 0 ) AddOutput(pszAppendScope, _T("\\qj "));
               }
               if( GetAttribute(pszTag, _T("nowrap"), szValue) ) {
                  AddOutput(pszAppendScope, _T("\\nowwrap "));
               }
               bNewContext = true;
            }
            else if( _tcscmp(szTagName, _T("table")) == 0 ) {
               Context.lLeftIndent = 0;
               Context.lRightIndent = 0;
               int lBorder = 0;
               if( GetAttribute(pszTag, _T("border"), szValue) ) {
                  lBorder = GetInteger(szValue);
               }
               if( GetStyleAttribute(pszTag, _T("table-layout"), szValue) ) {
                  ATLASSERT(_tcscmp(szValue, _T("fixed")) == 0);
               }
               AddOutput(pszAppendScope, _T("\\par\\trowd \\trgaph108\\trleft-108"));
               AddOutputV(pszAppendScope, _T("\\trbrdrt\\brdrs\\brdrw%d "), lBorder);
               AddOutputV(pszAppendScope, _T("\\trbrdrl\\brdrs\\brdrw%d "), lBorder);
               AddOutputV(pszAppendScope, _T("\\trbrdrb\\brdrs\\brdrw%d "), lBorder);
               AddOutputV(pszAppendScope, _T("\\trbrdrr\\brdrs\\brdrw%d "), lBorder);
               nCellPos = 0;
               m_nCharPos++;
               m_bWasSpace = true;
               bNewContext = true;
            }
            else if( _tcscmp(szTagName, _T("col")) == 0 ) {
               int lWidth = 0;
               if( GetAttribute(pszTag, _T("width"), szValue) ) {
                  lWidth = GetInteger(szValue);
               }
               ATLASSERT(lWidth>0);
               nCellPos += lWidth * lTwipFactor;
               AddOutput(pszAppendScope, _T("\\clbrdrt\\brdrw15\\brdrs\\clbrdrl\\brdrw15\\brdrs\\clbrdrb\\brdrw15\\brdrs\\clbrdrr\\brdrw15\\brdrs"));
               AddOutputV(pszAppendScope, _T("\\cellx%ld"), nCellPos);
               m_bWasSpace = true;
            }
            else if( _tcscmp(szTagName, _T("tr")) == 0 ) {
               AddOutput(pszAppendScope, _T("\\intbl "));
               m_bWasSpace = true;
            }
            else if( _tcscmp(szTagName, _T("td")) == 0 ) {
               AddOutput(pszAppendScope, _T("\\par "));
               m_nCharPos++;
               m_bWasSpace = true;
            }

            // Containers may also include these styles...
            if( bNewContext ) {
               if( _tcsstr(pszTag, _T("style=")) != NULL ) {
                  if( GetStyleAttribute(pszTag, _T("color"), szValue) ) {
                     Context.lTextColor = GetColor(szValue);
                  }
                  if( GetStyleAttribute(pszTag, _T("background-color"), szValue) ) {
                     Context.lBackColor = GetColor(szValue);
                  }
               }
            }
         }
         else {
            //
            // Closing tags
            //
            if( _tcscmp(szTagName, _T("/b")) == 0 || _tcscmp(szTagName, _T("/strong")) == 0) {
               AddOutput(pszAppendScope, _T("\\b0 "));
            }
            else if( _tcscmp(szTagName, _T("/i")) == 0 ) {
               AddOutput(pszAppendScope, _T("\\i0 "));
            }
            else if( _tcscmp(szTagName, _T("/u")) == 0 ) {
               AddOutput(pszAppendScope, _T("\\ulnone "));
            }
            else if( _tcscmp(szTagName, _T("/tr")) == 0 ) {
               AddOutput(pszAppendScope, _T("\\row "));
               m_nCharPos++;
               m_bWasSpace = true;
            }
            else if( _tcscmp(szTagName, _T("/td")) == 0 ) {
               AddOutput(pszAppendScope, _T("\\cell "));
               m_nCharPos++;
               m_bWasSpace = true;
            }
            else if( _tcscmp(szTagName, _T("/body")) == 0 ) {
               bRestoreContext = true;
            }
            else if( _tcscmp(szTagName, _T("/font")) == 0 ) {
               bRestoreContext = true;
            }
            else if( _tcscmp(szTagName, _T("/a")) == 0 ) {
               Link.nEnd = m_nCharPos;
               m_aLinks.Add(Link);
               //ATLTRACE("Link: %d - %d\n", Link.nStart, Link.nEnd);
               bRestoreContext = true;
            }
            else if( _tcscmp(szTagName, _T("/span")) == 0 ) {
               bRestoreContext = true;
            }
            else if( _tcscmp(szTagName, _T("/div")) == 0 ) {
               AddOutput(pszAppendScope, _T("\\par "));
               m_nCharPos++;
               bRestoreContext = true;
               m_bEndLine = true;
               m_bWasSpace = true;
            }
            else if( _tcscmp(szTagName, _T("/table")) == 0 ) {
               bRestoreContext = true;
               m_bEndLine = true;
               m_bWasSpace = true;
            }
         }

         // Appended all we wanted?
         *pszAppendScope = _T('\0');

         // Keep track of current rendering context
         if( bNewContext ) {
            // Push new context on stack
            aContexts.Add(OldContext);
            // Append attributes to output...
            AddOutput(pstrDest, _T("{"));
            AddOutput(pstrDest, szAppendScope);
         }
         else if( bRestoreContext ) {
            // Pop context from stack
            OldContext = Context;
            int lLast = aContexts.GetSize()-1;
            ATLASSERT(aContexts.GetSize()>0); // unmatched open/close tags
            if( aContexts.GetSize() == 0 ) return false;
            Context = aContexts[lLast];
            aContexts.RemoveAt(lLast);
            // Append attributes to output...
            AddOutput(pstrDest, szAppendScope);
            AddOutput(pstrDest, _T("}\r\n"));
         }
         else {
            // No context change, just append the
            // attributes...
            AddOutput(pstrDest, szAppendScope);
         }
         // If context change, we match any changes in rendering and
         // and RTF comamnds...
         if( bNewContext ) {
            bool bChanged = false;
            if( OldContext.lTextColor != Context.lTextColor ) {
               AddOutputV(pstrDest, _T("\\cf%d"), Context.lTextColor + 1);
               bChanged = true;
            }
            if( OldContext.lBackColor != Context.lBackColor ) {
               AddOutputV(pstrDest, _T("\\highlight%d"), Context.lBackColor + 1);
               bChanged = true;
            }
            if( OldContext.lFontSize != Context.lFontSize ) {
               AddOutputV(pstrDest, _T("\\fs%d"), Context.lFontSize);
               bChanged = true;
            }
            if( _tcscmp(OldContext.szFaceName, Context.szFaceName)!=0 ) {
               AddOutputV(pstrDest, _T("\\f%d"), GetFontFace(Context.szFaceName));
               bChanged = true;
            }
            if( bChanged ) AddOutput(pstrDest, _T(" "));
         }

      }
      return true;
   }

   void AddText(LPTSTR& pstrDest, LPCTSTR& pszHTML)
   {
      while( *pszHTML && *pszHTML!=_T('<') ) {
         switch( *pszHTML ) {
         case _T('&'):
            {
               static LPCTSTR szPseudo[] =
               {
                  _T("&nbsp;"), _T("\\~"),
                  _T("&quot;"), _T("\""),
                  _T("&amp;"), _T("&"),
                  _T("&lt;"), _T("<"),
                  _T("&gt;"), _T(">"),
                  NULL, NULL
               };
               LPCTSTR* ppstrPseudo = szPseudo;
               while( *ppstrPseudo ) {
                  if( _tcsnicmp(pszHTML, *ppstrPseudo, ::lstrlen(*ppstrPseudo)) == 0 ) {
                     pszHTML += ::lstrlen(*ppstrPseudo);
                     AddOutput(pstrDest, *(ppstrPseudo + 1));
                     m_nCharPos++;
                     m_bWasSpace = false;
                     break;
                  }
                  ppstrPseudo += 2;
               }
               ATLASSERT(*ppstrPseudo); // Found a replacement?
            }
            break;
         case _T('\\'):
            *pstrDest++ = _T('\\');
            *pstrDest++ = _T('\\');
            pszHTML++;
            m_nCharPos++;
            m_bWasSpace = false;
            break;
         case _T('{'):
            *pstrDest++ = _T('\\');
            *pstrDest++ = _T('{');
            pszHTML++;
            m_nCharPos++;
            m_bWasSpace = false;
            break;
         case _T('}'):
            *pstrDest++ = _T('\\');
            *pstrDest++ = _T('}');
            pszHTML++;
            m_nCharPos++;
            m_bWasSpace = false;
            break;
         case _T('\t'):
         case _T(' '):
            // Ignore spaces as long as this is the start of a new line
            // Not exactly HTML compatible, but at good try...
            if( !m_bEndLine && !m_bWasSpace) {
               *pstrDest++ = _T(' ');
               m_nCharPos++;
            }
            pszHTML++;
            m_bWasSpace = true;
            break;
         case _T('\r'):
            pszHTML++;
            break;
         case _T('\n'):
            if( !m_bWasSpace ) {
               *pstrDest++ = _T(' ');
               m_nCharPos++;
            }
            pszHTML++;
            break;
         default:
            *pstrDest++ = *pszHTML++;
            m_nCharPos++;
            m_bEndLine = false;
            m_bWasSpace = false;
         }
      }
   }
   inline void AddOutput(LPTSTR& pstrDest, LPCTSTR pszSrc) const
   {
      while( *pszSrc ) *pstrDest++ = *pszSrc++;
   }
   void AddOutputV(LPTSTR& pstrDest, LPCTSTR pszFormat, ...) const
   {
      va_list argptr;
      va_start( argptr, pszFormat );
      int nCount = ::wvsprintf( pstrDest, pszFormat, argptr );
      va_end( argptr );
      pstrDest += nCount;
   }

   bool GetAttribute(LPCTSTR pszTag, LPCTSTR pszName, LPTSTR pszValue) const
   {
      *pszValue = _T('\0');
      LPCTSTR p = _tcsstr(pszTag, pszName);
      if( p==NULL ) return false;
      p += _tcslen(pszName);
      while( _istspace(*p) ) p++;
      if( *p!=_T('=') ) return true;
      p++;
      while( _istspace(*p) ) p++;
      TCHAR c = *p;
      switch( c ) {
      case _T('\"'):
      case _T('\''):
         p++;
         while( *p && *p!=c ) *pszValue++ = *p++;
         break;
      default:
         while( *p && (_istalnum(*p) || *p == _T('-')) ) *pszValue++ = *p++;
         break;
      }
      *pszValue = _T('\0');
      return true;
   }
   bool GetStyleAttribute(LPCTSTR pszTag, LPCTSTR pszName, LPTSTR pszValue) const
   {
      // NOTE: The handling of the STYLE attribute is a bit simplistic
      //       and will only work for well-behaved and simple HTML.
      *pszValue = _T('\0');
      LPCTSTR p = _tcsstr(pszTag, pszName);
      if( p==NULL ) return false;
      p += _tcslen(pszName);
      while( _istspace(*p) ) p++;
      if( *p!=_T(':') ) return false;
      ATLASSERT(_tcsstr(pszTag, _T("style="))!=NULL);
      p++;
      while( _istspace(*p) ) p++;
      TCHAR c = *p;
      switch( c ) {
      case _T('\"'):
      case _T('\''):
         p++;
         while( *p && *p!=c ) *pszValue++ = *p++;
         break;
      default:
         while( *p && (_istalnum(*p) || *p == _T('-')) ) *pszValue++ = *p++;
         break;
      }
      *pszValue = _T('\0');
      return true;
   }
   int GetColor(LPCTSTR pszValue)
   {
      COLORREF clr;
      if( *pszValue == _T('#') ) {
         pszValue++;
         LPTSTR pstrStop;
         DWORD BGR = _tcstol(pszValue, &pstrStop, 16); // convert hex value
         clr = RGB( (BGR & 0xFF0000) >> 16, (BGR & 0xFF00) >> 8, BGR & 0xFF);
      }
      else {
         // TODO: Implement more colors-by-name
         if( _tcscmp(pszValue, _T("window")) == 0 ) clr = ::GetSysColor(COLOR_WINDOW);
         else if( _tcscmp(pszValue, _T("windowtext")) == 0 ) clr = ::GetSysColor(COLOR_WINDOWTEXT);
         else if( _tcscmp(pszValue, _T("buttonface")) == 0 ) clr = ::GetSysColor(COLOR_BTNFACE);
         else if( _tcscmp(pszValue, _T("buttontext")) == 0 ) clr = ::GetSysColor(COLOR_BTNTEXT);
         else clr = RGB(0,0,0);
      }
      int iIndex = m_aColors.Find(clr);
      if( iIndex>=0 ) return iIndex;
      m_aColors.Add(clr);
      return m_aColors.GetSize() - 1;
   }
   int GetFontFace(LPCTSTR pszValue)
   {
      for( int i = 0; i < m_aFonts.GetSize(); i++ ) {
         if( _tcsicmp(m_aFonts[i].lfFaceName, pszValue) == 0 ) return i;
      }
      LOGFONT lf = { 0 };
      _tcscpy( lf.lfFaceName, pszValue );
      m_aFonts.Add(lf);
      return m_aColors.GetSize() - 1;
   }
   int GetFontSize(LPCTSTR pszValue) const
   {
      int lFontSize = GetInteger(pszValue);
      switch( lFontSize ) {
      case 1: return 15; // 7.5
      case 2: return 20; // 10
      case 3: return 24; // 12
      case 4: return 27; // 13.5
      case 5: return 36; // 18
      case 6: return 48; // 24
      case 7: return 72; // 36
      default: return 24;
      }
   }
   int GetFontSize(int lFontSize) const
   {
      // Ok, RTF is in half-points, so this is obviously an x2 algorithm.
      // But I'll leave it here if you need to customize...
      if( lFontSize>=36 ) return 72;
      if( lFontSize>=24 ) return 48;
      if( lFontSize>=18 ) return 36;
      if( lFontSize>=16 ) return 32;
      if( lFontSize>=14 ) return 28;
      if( lFontSize>=12 ) return 24;
      if( lFontSize>=10 ) return 20;
      if( lFontSize>=8 ) return 18;
      return 24; // default
   }
   int GetInteger(LPCTSTR pszValue) const
   {
      return _ttol(pszValue);
   }
};


/////////////////////////////////////////////////////////////////////////////
// CSimpleHtmlCtrl - a subclassed RichEdit control

template< class T, class TBase = CRichEditCtrl, class TWinTraits = CControlWinTraits >
class CSimpleHtmlImpl : public CWindowImpl< T, TBase, TWinTraits >
{
public:
   DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

   typedef struct RtfStream
   {
      LPCSTR pstr;
      DWORD pos;
   };
   CHtmlToRtf convert;
   COLORREF m_clrText;
   COLORREF m_clrBack;

   // Operations

   BOOL SubclassWindow(HWND hWnd)
   {
      ATLASSERT(m_hWnd == NULL);
      ATLASSERT(::IsWindow(hWnd));
#ifdef _DEBUG
      TCHAR szBuffer[20];
      if( ::GetClassName(hWnd, szBuffer, (sizeof(szBuffer)/sizeof(TCHAR))-1) ) {
         ATLASSERT(::lstrcmpi(szBuffer, TBase::GetWndClassName()) == 0);
      }
#endif
      BOOL bRet = CWindowImpl< T, TBase, TWinTraits >::SubclassWindow(hWnd);
      if(bRet) _Init();
      return bRet;
   }

   BOOL Load(LPCTSTR pszHTML)
   {
      ATLASSERT(!::IsBadStringPtr(pszHTML,-1));

      // Extract current font, text color and background color...
      HFONT hFont = GetFont();
      if( hFont==NULL ) hFont = (HFONT) ::GetStockObject(ANSI_VAR_FONT);
      LOGFONT lf = { 0 };
      ::GetObject(hFont, sizeof(LOGFONT), &lf);
      CWindowDC dc(m_hWnd);
      m_clrText = dc.GetTextColor();
      m_clrBack = dc.GetBkColor();

      // Convert HTML to RTF
      LPTSTR pstrText = (LPTSTR) convert.Convert(pszHTML, lf, m_clrText, m_clrBack);
      if( pstrText==NULL ) return FALSE;

      // Change the background color
      SetBackgroundColor(m_clrBack);

      // Stream RTF into control
      USES_CONVERSION;
      RtfStream st = { T2CA(pstrText), 0 }; // TODO: Don't use inline conversion for large texts
      EDITSTREAM es = { 0 };
      es.dwCookie = (DWORD) &st;
      es.dwError = 0;
      es.pfnCallback = _StreamReadCallback;
      // NOTE: If execption handling is enabled for the app, the little ATLTRY macro
      //       might catch catastrophic errors (GPFs) from the RichEdit control
      //       when streaming in malformed RTF.
      ATLTRY( StreamIn(SF_RTF, es) );
      // All done...
      free( pstrText );

      _CreateLinks(convert);
      return TRUE;
   }
   BOOL ExtractLink(CHARRANGE chrng, LPTSTR szUrl, UINT cchMax)
   {
      szUrl[0] = _T('\0');
      CHtmlToRtf::LINK link = { 0 };
      for( int i = 0; convert.GetLink(i, link); i++ ) {
         if( chrng.cpMin >= link.nStart && chrng.cpMax <= link.nEnd ) {
            _tcsncpy( szUrl, link.szURL, cchMax);
            return TRUE;
         }
      }
      return FALSE;
   }

   // Implementation

   void _Init()
   {
      // Forgot to load RTF library?
      ATLASSERT(::GetModuleHandle(CRichEditCtrl::GetLibraryName())!=NULL);
      // Some style that should be set...
      ATLASSERT(GetStyle() & (ES_MULTILINE|ES_AUTOVSCROLL)==(ES_MULTILINE|ES_AUTOVSCROLL));
      // Default background color
      m_clrBack = ::GetSysColor(COLOR_WINDOW);
      // Turn on word-wrapping...
      ModifyStyle(ES_AUTOHSCROLL, 0);
      SetTargetDevice(NULL, 0);
      // We want to have nice URL behaviour
      SetEventMask(ENM_SELCHANGE | ENM_LINK);
   }
   void _CreateLinks(CHtmlToRtf& conv)
   {
      conv;
#if _RICHEDIT_VER >= 0x0200
      // Traverse the collection of links to assign the new display
      // state. Links seems to be handled internally in the RichEdit
      // control and not by some proprietary RTF tag (must be a first for
      // Microsoft...) so we need to manually assign the CFE_LINK attribute.
      CHtmlToRtf::LINK link = { 0 };
      for( int i = 0; conv.GetLink(i, link); i++ ) {
         SetSel(link.nStart, link.nEnd);
         CHARFORMAT2 cf;
         cf.cbSize = sizeof(cf);
         cf.dwMask = CFM_STYLE | CFM_UNDERLINETYPE | CFM_LINK | CFM_UNDERLINE;
         cf.dwEffects = CFE_LINK | CFE_UNDERLINE;
         cf.bUnderlineType = CFU_UNDERLINE;
         SetSelectionCharFormat(cf);
      }
      SetSel(0,0);
#endif
   }

   static DWORD CALLBACK _StreamReadCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR *pcb)
   {
      RtfStream *pS = reinterpret_cast<RtfStream *>(dwCookie);
      ATLASSERT(pS);
      LPCSTR pstr = pS->pstr + pS->pos;
      ATLASSERT(!::IsBadStringPtrA(pstr, -1));
      LONG len = ::lstrlenA(pstr);
      if( cb>len ) cb = len;
      ::CopyMemory(pbBuff, pstr, cb);
      pS->pos += cb;
      *pcb = cb;
      return 0;
   }

   // Message map and handlers

   BEGIN_MSG_MAP_EX(CSimpleHtmlCtrl)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
   END_MSG_MAP()

   LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      // First let original control initialize everything
      LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
      _Init();
      return lRet;
   }
};


class CSimpleHtmlCtrl : public CSimpleHtmlImpl<CSimpleHtmlCtrl>
{
public:
   DECLARE_WND_SUPERCLASS(_T("WTL_SimpleHtmlCtrl"), GetWndClassName())
};


#endif // !defined(AFX_SIMPLEHTMLCTRL_H__20020223_765E_434E_641B_0080AD509054__INCLUDED_)

