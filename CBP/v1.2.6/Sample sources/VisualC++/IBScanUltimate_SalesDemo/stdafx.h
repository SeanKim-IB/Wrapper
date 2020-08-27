
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

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars




#define NUMBER_OF_TAB			2

#define IMG_W					800
#define IMG_H					750
#define IMG_SIZE				(IMG_W*IMG_H)

#define IMG_1_W					560
#define IMG_1_H					280
#define IMG_1_SIZE				(IMG_1_W*IMG_1_H) 

#define S_IMG_W					160
#define S_IMG_H					150
#define S_IMG_SIZE				(S_IMG_W*S_IMG_H)

#define S_IMG_1_W				280
#define S_IMG_1_H				140
#define S_IMG_1_SIZE			(S_IMG_1_W*S_IMG_1_H) 

#define S_IMG_2_W				140
#define S_IMG_2_H				140
#define S_IMG_2_SIZE			(S_IMG_2_W*S_IMG_2_H)

#define S_IMG_3_W				300
#define S_IMG_3_H				282
#define S_IMG_3_SIZE			(S_IMG_3_W*S_IMG_3_H) 

#define S_IMG_4_W				400
#define S_IMG_4_H				370
#define S_IMG_4_SIZE			(S_IMG_4_W*S_IMG_4_H)		


#define MAX_IMG_W				1600
#define MAX_IMG_H				1500
#define MAX_IMG_SIZE			(MAX_IMG_W*MAX_IMG_H) 


#define IMAGE_BUFFER_COUNT		20

#define RES_KEY					"ibkorea1120!"

#define SPOOF_BUF_W				400
#define SPOOF_BUF_H				500

// capture mode
#define NONE_CAPTURE										0x00
#define ONE_FINGER_FLAT_CAPTURE								0x01
#define ROLL_FINGER_FLAT_CAPTURE							0x02
#define TWO_FINGER_FLAT_CAPTURE								0x03	//1image

#define TEN_FINGER_ONE_FLAT_CAPTURE							0x04	//10 image
#define TEN_FINGER_ONE_ROLL_CAPTURE							0x05	//10 image
#define TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH				0x06	//3 image	
#define TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH				0x07	//3image

#define TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH			0x08	//14image
#define TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH			0x09	//14image
#define TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE			0x0A	//14image

#define TEN_FINGER_FLAT_THUMB								0x0B	//14image
#define TEN_FINGER_FLAT_THUMB_INDEX							0x0C	//14image

#define TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL				0x0D	//19image

#define TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN				0x0E	//16image

#define TEN_FINGER_FOUR_FOUR_TWO							0x0F	//442 image
#define TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH			0x10	//442 image_roll

#define TEN_FINGER_FOUR_ONE_FOUR_ONE						0x11	//4141 image
#define TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH		0x12	//4141 image_roll

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
#define LEFT_HAND						0x0F
#define THUMB_SUM						0x10
#define BOTH_LEFT_THUMB					0x11
#define BOTH_RIGHT_THUMB				0x12
#define RIGHT_HAND						0x13




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
#define WM_USER_ASYNC_OPEN_DEVICE				WM_APP + 11

// Capture sequence definitions
/*#define CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS		_T("10 prints using one flat finger")
#define CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS	_T("10 prints using one rolled finger")
#define CAPTURE_SEQ_10_DOUBLE_FLAT_FINGERS		_T("10 prints using two flat fingers")
#define CAPTURE_SEQ_10_FOUR_FLAT_FINGERS		_T("10 prints using four flat fingers")*/

#define PRINT_3_TWO_FLAT_BOTH						_T("10 prints (2-2-2-2-2)")
#define PRINT_3_TWO_FLAT_EACH						_T("10 prints (2-2-2-2-1-1)")

#define PRINT_14_TWO_FLAT_BOTH_AND_ROLLS			_T("20 prints (2-2-2-2-2-rolled) +scan 14 images")
#define PRINT_14_TWO_FLAT_EACH_AND_ROLLS			_T("20 prints (2-2-2-2-1-1-rolled)")
#define PRINT_14_TWO_FLAT_AND_SINGLE_FLAT			_T("20 prints (2-2-2-2-2-flats)")
#define PRINT_14_FLAT_THUMB							_T("2 prints(thumbs)")
#define PRINT_14_FLAT_THUMB_AND_INDEX				_T("4 prints(thumbs+indexes)")


#define PRINT_10_SINGLE_FLAT						_T("10 prints using one flat finger")
#define PRINT_10_SINGLE_ROLLS						_T("10 prints using one rolled finger")

#define PRINT_16_TWO_FLAT_EACH_AND_ROLLS			_T("20 prints (2-2-2-2-1-1-rolled)+scan 16 images")

#define PRINT_19_TWO_FLAT_DOUBLE_AND_ROLLS			_T("20 prints (2-2-2-2-2-1-1-rolled)")
#define PRINT_1_TWO_FLAT_BOTH						_T("1 prints(both thumbs)")

#define PRINT_FOUR_FLAT								_T("Slaps Finger (4-4-2)")
#define PRINT_FOUR_FLAT_AND_ROLLS					_T("Slaps Finger (4-4-2-rolled)")

#define PRINT_FOUR_FLAT2							_T("Slaps Finger (4-1-4-1)")
#define PRINT_FOUR_FLAT2_AND_ROLLS					_T("Slaps Finger (4-1-4-1-rolled)")


#define __BEEP_FAIL__							0
#define __BEEP_SUCCESS__						1
#define __BEEP_OK__								2
#define __BEEP_DEVICE_COMMUNICATION_BREAK__		3

#include "Layout/btnst.h"
#include "Layout/ColorCtrl.h"
#include "Layout/FontCtrl.h"
#include <afxdhtml.h>


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


