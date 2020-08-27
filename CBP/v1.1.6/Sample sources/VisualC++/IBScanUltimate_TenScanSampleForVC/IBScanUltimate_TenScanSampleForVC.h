
// IBScanUltimate_TenScanSampleForVC.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CIBScanUltimate_TenScanSampleForVCApp:
// See IBScanUltimate_TenScanSampleForVC.cpp for the implementation of this class
//

class CIBScanUltimate_TenScanSampleForVCApp : public CWinApp
{
public:
	CIBScanUltimate_TenScanSampleForVCApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CIBScanUltimate_TenScanSampleForVCApp theApp;