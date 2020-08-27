
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

//#include <afxcontrolbars.h>     // MFC support for ribbons and control bars




#define NUMBER_OF_TAB		2
#define IMG_W				800
#define IMG_H				750
#define IMG_SIZE			(IMG_W*IMG_H)
#define S_IMG_W				160
#define S_IMG_H				150
#define S_IMG_SIZE			(S_IMG_W*S_IMG_H)

#define IMAGE_BUFFER_COUNT	15

// capture mode
#define NONE_CAPTURE					0x00
#define ONE_FINGER_FLAT_CAPTURE			0x01
#define ROLL_FINGER_FLAT_CAPTURE		0x02
#define TWO_FINGER_FLAT_CAPTURE			0x03
#define TEN_FINGER_ONE_FLAT_CAPTURE		0x04
#define TEN_FINGER_ONE_ROLL_CAPTURE		0x05
#define TEN_FINGER_TWO_FLAT_CAPTURE		0x06
#define TEN_FINGER_FOUR_FLAT_CAPTURE	0x07

// capture fingerprint index
#define NONE_FINGER						0xFF
#define LEFT_LITTLE						0x00
#define LEFT_RING						0x01
#define LEFT_MIDDLE						0x02
#define LEFT_INDEX						0x03
#define LEFT_THUMB						0x04
#define RIGHT_THUMB						0x05
#define RIGHT_INDEX						0x06
#define RIGHT_MIDDLE					0x07
#define RIGHT_RING						0x08
#define RIGHT_LITTLE					0x09
#define LEFT_RING_LITTLE				0x0A
#define LEFT_INDEX_MIDDLE				0x0B
#define BOTH_THUMBS						0x0C
#define RIGHT_INDEX_MIDDLE				0x0D
#define RIGHT_RING_LITTLE				0x0E

// Use user window messages
#define WM_USER_CAPTURE_SEQ_START				WM_APP + 1
#define WM_USER_CAPTURE_SEQ_NEXT				WM_APP + 2
#define WM_USER_DEVICE_COMMUNICATION_BREAK		WM_APP + 3
#define WM_USER_DRAW_CLIENT_WINDOW				WM_APP + 4
#define WM_USER_UPDATE_DEVICE_LIST				WM_APP + 5
#define WM_USER_INIT_WARNING					WM_APP + 6
#define WM_USER_UPDATE_DISPLAY_RESOURCES		WM_APP + 7
#define WM_USER_UPDATE_STATUS_MESSAGE			WM_APP + 8
#define WM_USER_BEEP							WM_APP + 9
#define WM_USER_DRAW_FINGER_QUALITY				WM_APP + 10
#define WM_USER_ASK_RECAPTURE					WM_APP + 11

// Capture sequence definitions
#define CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS		_T("10 prints using one flat finger")
#define CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS	_T("10 prints using one rolled finger")
#define CAPTURE_SEQ_10_DOUBLE_FLAT_FINGERS		_T("10 prints using two flat fingers")
#define CAPTURE_SEQ_10_FOUR_FLAT_FINGERS		_T("10 prints using four flat fingers")

#define __BEEP_FAIL__							0
#define __BEEP_SUCCESS__						1
#define __BEEP_OK__								2
#define __BEEP_DEVICE_COMMUNICATION_BREAK__		3

#include "Layout/btnst.h"
#include "Layout/ColorCtrl.h"
#include "Layout/FontCtrl.h"


#pragma warning(disable:4390)		// I don't like Window's sequre version function.
#pragma warning(disable:4995)		// I don't like Window's sequre version function.
#pragma warning(disable:4996)		// I don't like Window's sequre version function.


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


