
// LogViewer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CLogViewerApp:
// See LogViewer.cpp for the implementation of this class
//

class CLogViewerApp : public CWinAppEx
{
public:
	CLogViewerApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CLogViewerApp theApp;