
// IBSU_MultipleScanSampleForVC.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CIBSU_MultipleScanSampleForVCApp:
// See IBSU_MultipleScanSampleForVC.cpp for the implementation of this class
//

class CIBSU_MultipleScanSampleForVCApp : public CWinApp
{
public:
	CIBSU_MultipleScanSampleForVCApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CIBSU_MultipleScanSampleForVCApp theApp;