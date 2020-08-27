
// CBP_SampleForVC.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CCBP_SampleForVCApp:
// See CBP_SampleForVC.cpp for the implementation of this class
//

class CCBP_SampleForVCApp : public CWinAppEx
{
public:
	CCBP_SampleForVCApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CCBP_SampleForVCApp theApp;