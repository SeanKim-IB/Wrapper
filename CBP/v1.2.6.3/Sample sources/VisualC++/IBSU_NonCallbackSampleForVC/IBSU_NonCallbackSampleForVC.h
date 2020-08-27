
// IBSU_NonCallbackSampleForVC.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CIBSU_NonCallbackSampleForVCApp:
// See IBSU_NonCallbackSampleForVC.cpp for the implementation of this class
//

class CIBSU_NonCallbackSampleForVCApp : public CWinApp
{
public:
	CIBSU_NonCallbackSampleForVCApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CIBSU_NonCallbackSampleForVCApp theApp;