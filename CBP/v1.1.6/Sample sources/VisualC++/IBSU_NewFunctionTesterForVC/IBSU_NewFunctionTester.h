
// IBSU_NewFunctionTester.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#include "IBScanUltimateApi_defs.h"
#include "IBScanUltimateApi.h"


// CIBSU_NewFunctionTesterApp:
// See IBSU_NewFunctionTester.cpp for the implementation of this class
//

class CIBSU_NewFunctionTesterApp : public CWinApp
{
public:
	CIBSU_NewFunctionTesterApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CIBSU_NewFunctionTesterApp theApp;