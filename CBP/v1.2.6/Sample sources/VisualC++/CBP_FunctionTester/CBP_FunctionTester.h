
// CBP_FunctionTester.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CCBP_FunctionTesterApp:
// See CBP_FunctionTester.cpp for the implementation of this class
//

class CCBP_FunctionTesterApp : public CWinAppEx
{
public:
	CCBP_FunctionTesterApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CCBP_FunctionTesterApp theApp;