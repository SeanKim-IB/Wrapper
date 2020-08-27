
// IBScanUltimate_SimpleScanWM.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resourceppc.h"		// main symbols


// CIBScanUltimate_SimpleScanWMApp:
// See IBScanUltimate_SimpleScanWM.cpp for the implementation of this class
//

class CIBScanUltimate_SimpleScanWMApp : public CWinApp
{
public:
	CIBScanUltimate_SimpleScanWMApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CIBScanUltimate_SimpleScanWMApp theApp;