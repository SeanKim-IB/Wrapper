// IBScanNFIQ2.h : main header file for the IBScanNFIQ2 DLL
//

#pragma once

#ifdef _WINDOWS
#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#endif

#include "IBScanNFIQ2Api.h"
#include "IBSU_Global_Def.h"

#include "include/NFIQException.h"
#include "include/Timer.hpp"
#include "include/FingerprintImageData.h"
#include "include/NFIQ2Algorithm.h"

using namespace std;
using namespace NFIQ;

// CIBScanNFIQ2App
// See IBScanNFIQ2.cpp for the implementation of this class
//

class CIBScanNFIQ2App : public CWinApp
{
public:
	CIBScanNFIQ2App();

// Overrides
public:
	virtual ~CIBScanNFIQ2App(void);

#ifdef _WINDOWS
	virtual BOOL		InitInstance();
	virtual BOOL		ExitInstance();
#endif


	int					DLL_GetVersion(char *pVerInfo);
	int					DLL_Initialize();
	int					DLL_IsInitialized();
	int					DLL_ComputeScore(const BYTE *imgBuffer, const DWORD width, const DWORD height,
											const BYTE bitsPerPixel, int *pScore);

    
	int					_GetThreadID();
	void				SendToTraceLog();
   	void				InsertTraceLog(const char *category, int error, int tid, const char *format, ...);
//////////////////////////////////////////////////////////////////////////////////


public: // Members variables
    NFIQ::NFIQ2Algorithm m_cNFIQ2;
    BOOL            m_traceLogEnabled;
	std::vector<TraceLog> m_pListTraceLog;
	HANDLE			m_hPipe;


protected: // Members variables

	BOOL		    m_bIsInitialized;			// Intialize for NFIQ2 module


public: // Inline members

	inline void		SetInitialized(BOOL value);
	inline BOOL		IsInitialized(void) const;

#ifdef _WINDOWS
	DECLARE_MESSAGE_MAP()
#endif
};

////////////////////////////////////////////////////////////////////////////////

inline void CIBScanNFIQ2App::SetInitialized(BOOL value)
	{ m_bIsInitialized = value; return; }

inline BOOL CIBScanNFIQ2App::IsInitialized(void) const
	{ return m_bIsInitialized; }
