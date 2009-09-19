/*
Module : NTray.h
Purpose: Interface for a MFC class to encapsulate Shell_NotifyIcon
Created: PJN / 14-05-1997
History: None

Copyright (c) 1997 - 2006 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////////// Macros / Defines ///////////////////////////

#ifndef _NTRAY_H__
#define _NTRAY_H__

#ifndef CTRAYNOTIFYICON_EXT_CLASS
#define CTRAYNOTIFYICON_EXT_CLASS
#endif

#ifndef CTRAYNOTIFYICON_EXT_API
#define CTRAYNOTIFYICON_EXT_API
#endif

#ifndef __ATLWIN_H__
#pragma message("CTrayNotifyIcon as of v1.51 requires ATL support to implement its functionality. If your project is MFC only, then you need to update it to include ATL support")
#endif

#ifndef _AFX
#include <atlmisc.h> //If you do want to use CTrayNotifyIcon independent of MFC, then you need to download and install WTL from http://sourceforge.net/projects/wtl
#define CTrayNotifyIconString WTL::CString
#else
#define CTrayNotifyIconString CString
#endif


/////////////////////////// Classes ///////////////////////////////////////////

//forward declaration
class CTrayNotifyIcon;

//Class which handles subclassing the top level window and handles the timer
//required for tray icon animation and taskbar creation message
class CTRAYNOTIFYICON_EXT_CLASS CTrayIconHooker : public ATL::CWindowImpl<CTrayIconHooker>
{
public:
//Constructors / Destructors
  CTrayIconHooker();
  ~CTrayIconHooker();

//Methods
#ifdef _AFX
  BOOL Init(CTrayNotifyIcon* pTrayIcon, CWnd* pNotifyWnd);
#else
  BOOL Init(CTrayNotifyIcon* pTrayIcon, ATL::CWindow* pNotifyWnd);
#endif  
  void StartUsingAnimation(HICON* phIcons, int nNumIcons, DWORD dwDelay);
  void StopUsingAnimation();
  BOOL UsingAnimatedIcon() const;
  HICON GetCurrentIcon() const;

protected:
//Methods
  virtual BOOL ProcessWindowMessage(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID);
  void OnTimer(UINT_PTR nIDEvent);
  LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);

//Member variables
  CTrayNotifyIcon* m_pTrayIcon;
  HICON*           m_phIcons;
  int              m_nNumIcons;
  UINT_PTR         m_nTimerID;
  int              m_nCurrentIconIndex;
};

//the actual tray notification class wrapper
class CTRAYNOTIFYICON_EXT_CLASS CTrayNotifyIcon
{
public:
//Enums / Typedefs
#ifndef CTRAYNOTIFYICON_NOWIN2K
  enum BalloonStyle
  {
    Warning,
    Error,
    Info,
    None,
    User
  };
#endif

//We use our own definitions of the NOTIFYICONDATA structs so that
//we can use all the functionality without requiring client code to 
//define _WIN32_IE >= 0x500
  typedef struct _NOTIFYICONDATA_1 //The version of the structure supported by Shell v4
  {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    TCHAR szTip[64];
  } NOTIFYICONDATA_1;

  typedef struct _NOTIFYICONDATA_2 //The version of the structure supported by Shell v5
  {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    TCHAR szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    TCHAR szInfo[256];
    union 
    {
      UINT uTimeout;
      UINT uVersion;
    } DUMMYUNIONNAME;
    TCHAR szInfoTitle[64];
    DWORD dwInfoFlags;
  } NOTIFYICONDATA_2;

  typedef struct _NOTIFYICONDATA_3 //The version of the structure supported by Shell v6
  {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    TCHAR szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    TCHAR szInfo[256];
    union 
    {
      UINT uTimeout;
      UINT uVersion;
    } DUMMYUNIONNAME;
    TCHAR szInfoTitle[64];
    DWORD dwInfoFlags;
    GUID guidItem;
  } NOTIFYICONDATA_3;

//Constructors / Destructors
  CTrayNotifyIcon();
  virtual ~CTrayNotifyIcon();

//Create the tray icon
#ifdef _AFX
  BOOL Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, HICON hIcon, UINT nNotifyMessage, UINT uMenuID = 0);
  BOOL Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, WTL::CBitmap* pBitmap, UINT nNotifyMessage, UINT uMenuID = 0);
  BOOL Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, HICON* phIcons, int nNumIcons, DWORD dwDelay, UINT nNotifyMessage, UINT uMenuID = 0);
  BOOL Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, HICON hIcon, UINT nNotifyMessage, UINT uMenuID = 0, BOOL bNoSound = FALSE);                  
  BOOL Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, WTL::CBitmap* pBitmap, UINT nNotifyMessage, UINT uMenuID = 0, BOOL bNoSound = FALSE);
  BOOL Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, HICON* phIcons, int nNumIcons, DWORD dwDelay, UINT nNotifyMessage, UINT uMenuID = 0, BOOL bNoSound = FALSE);
#else
  BOOL Create(ATL::CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, HICON hIcon, UINT nNotifyMessage, UINT uMenuID = 0);
  BOOL Create(ATL::CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, WTL::CBitmap* pBitmap, UINT nNotifyMessage, UINT uMenuID = 0);
  BOOL Create(ATL::CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, HICON* phIcons, int nNumIcons, DWORD dwDelay, UINT nNotifyMessage, UINT uMenuID = 0);
  BOOL Create(ATL::CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, HICON hIcon, UINT nNotifyMessage, UINT uMenuID = 0, BOOL bNoSound = FALSE);                  
  BOOL Create(ATL::CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, WTL::CBitmap* pBitmap, UINT nNotifyMessage, UINT uMenuID = 0, BOOL bNoSound = FALSE);
  BOOL Create(ATL::CWindow* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, UINT nTimeout, BalloonStyle style, HICON* phIcons, int nNumIcons, DWORD dwDelay, UINT nNotifyMessage, UINT uMenuID = 0, BOOL bNoSound = FALSE);
#endif  

//Sets or gets the Tooltip text
  BOOL                  SetTooltipText(LPCTSTR pszTooltipText);
  BOOL                  SetTooltipText(UINT nID);
  CTrayNotifyIconString GetTooltipText() const;

//Sets or gets the icon displayed
  BOOL SetIcon(HICON hIcon);
  BOOL SetIcon(WTL::CBitmap* pBitmap);
  BOOL SetIcon(LPCTSTR lpIconName);
  BOOL SetIcon(UINT nIDResource);
  BOOL SetIcon(HICON* phIcons, int nNumIcons, DWORD dwDelay);
  BOOL SetStandardIcon(LPCTSTR lpIconName);
  BOOL SetStandardIcon(UINT nIDResource);
  HICON GetIcon() const;
  BOOL UsingAnimatedIcon() const;

//Sets or gets the window to send notification messages to
#ifdef _AFX
  BOOL SetNotificationWnd(CWnd* pNotifyWnd);
  CWnd* GetNotificationWnd() const;
#else
  BOOL SetNotificationWnd(ATL::CWindow* pNotifyWnd);
  ATL::CWindow* GetNotificationWnd() const;
#endif

//Modification of the tray icons
  BOOL Delete();
  BOOL Create();
  BOOL Hide();
  BOOL Show();

//Dynamic tray icon support
  HICON BitmapToIcon(WTL::CBitmap* pBitmap);
  static BOOL GetDynamicDCAndBitmap(WTL::CDC* pDC, WTL::CBitmap* pBitmap);

//Modification of the menu to use as the context menu
  void SetMenu(HMENU hMenu);
  WTL::CMenu& GetMenu();
  void SetDefaultMenuItem(UINT uItem, BOOL fByPos);
  void GetDefaultMenuItem(UINT& uItem, BOOL& fByPos) { uItem = m_nDefaultMenuItem; fByPos = m_bDefaultMenuItemByPos; };

//Default handler for tray notification message
  virtual LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);

//Status information
  BOOL IsShowing() const { return !IsHidden(); };
  BOOL IsHidden() const { return m_bHidden; };

//Sets or gets the Balloon style tooltip settings
  BOOL                  SetBalloonDetails(LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, BalloonStyle style, UINT nTimeout, HICON hUserIcon=NULL, BOOL bNoSound=FALSE);
  CTrayNotifyIconString GetBalloonText() const;
  CTrayNotifyIconString GetBalloonCaption() const;
  BalloonStyle          GetBalloonStyle() const;
  UINT                  GetBalloonTimeout() const;

//Other functionality
  BOOL SetVersion(UINT uVersion);
  BOOL SetFocus();
  static DWORD GetShellVersion();

//Helper functions to load tray icon from resources
  static HICON LoadIconResource(LPCTSTR lpIconName);
  static HICON LoadIconResource(UINT nIDResource);

protected:
//Member variables
  NOTIFYICONDATA_3 m_NotifyIconData;
  BOOL             m_bCreated;
  BOOL             m_bHidden;
#ifdef _AFX  
  CWnd*            m_pNotificationWnd;
#else
  ATL::CWindow*         m_pNotificationWnd;
#endif  
  CTrayIconHooker  m_HookWnd;
  WTL::CMenu       m_Menu;
  UINT             m_nDefaultMenuItem;
  BOOL             m_bDefaultMenuItemByPos;
  static DWORD     sm_dwShellVersion;
  HICON            m_hDynamicIcon; //Our cached copy of the last icon created with BitmapToIcon

//Methods
  static DWORD GetNOTIFYICONDATASizeForOS();

  friend class CTrayIconHooker;
};

#endif //_NTRAY_H__
