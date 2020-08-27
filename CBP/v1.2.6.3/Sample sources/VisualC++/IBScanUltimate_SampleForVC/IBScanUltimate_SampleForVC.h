
// IBScanUltimate_SampleForVC.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CIBScanUltimate_SampleForVCApp:
// See IBScanUltimate_SampleForVC.cpp for the implementation of this class
//

class CIBScanUltimate_SampleForVCApp : public CWinApp
{
public:
	CIBScanUltimate_SampleForVCApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CIBScanUltimate_SampleForVCApp theApp;