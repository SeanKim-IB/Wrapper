// IBScanNFIQ2.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "IBScanNFIQ2.h"
#ifdef _WINDOWS
#include "FileVersionInfo.h"
#endif

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <list>
#include <fstream>


#if defined(_WINDOWS)
const TCHAR *sPipeName = _T("\\\\.\\pipe\\IBTraceLogger");
#endif
const char *sTraceMainCategory = "IBScanNFIQ2";

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CIBScanNFIQ2App

#ifdef _WINDOWS
BEGIN_MESSAGE_MAP(CIBScanNFIQ2App, CWinApp)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////
#elif defined(__linux__)
bool IsBadReadPtr(const void *x, int size)
{
    return (x == NULL);
}

////////////////////////////////////////////////////////////////////////////////////////
#endif


// CIBScanNFIQ2App construction

CIBScanNFIQ2App::CIBScanNFIQ2App()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
    m_bIsInitialized = FALSE;

    // Disable trace log on Linux by default to preserve current behavior
#if defined(__linux__) && !defined(__android__)
    m_traceLogEnabled = FALSE;
#else
    m_traceLogEnabled = TRUE;
#endif

    m_hPipe = INVALID_HANDLE_VALUE;
}


// The one and only CIBScanNFIQ2App object

CIBScanNFIQ2App theApp;


// CIBScanNFIQ2App initialization

// CIBScanUltimateApp initialization
#if defined(_WINDOWS)
BOOL CIBScanNFIQ2App::InitInstance()
{
    CWinApp::InitInstance();

    return TRUE;
}

BOOL CIBScanNFIQ2App::ExitInstance()
{
	CWinApp::ExitInstance();

    return TRUE;
}
#endif

CIBScanNFIQ2App::~CIBScanNFIQ2App()
{
    m_bIsInitialized = FALSE;

#ifdef _WINDOWS
    if (m_hPipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
    }
#endif
}

int CIBScanNFIQ2App::_GetThreadID()
{
#ifdef _WINDOWS
    return (int)GetCurrentThreadId();
#else
    return pthread_self();
#endif
}

void CIBScanNFIQ2App::SendToTraceLog()
{
#if defined(_WINDOWS) && !defined(WINCE)
//    CThreadSync Sync;

    DWORD			dwWritten;
    TraceLog		tLog;
    TCHAR			szLogBuffer[__MAX_PIPE_DATA__] = {0};
    TCHAR			logPipeName[64] = {0};


    if (m_hPipe == INVALID_HANDLE_VALUE)
    {
        if (WaitNamedPipe(sPipeName, 1000))
        {
            m_hPipe = CreateFile(sPipeName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            _stprintf(szLogBuffer, _T("createlog^IBScanU"));
            BOOL bRc = WriteFile(m_hPipe, szLogBuffer, (DWORD)_tcslen(szLogBuffer), &dwWritten, NULL);
            if (bRc == FALSE || dwWritten == 0)
            {
                CloseHandle(m_hPipe);
                m_hPipe = INVALID_HANDLE_VALUE;
                m_pListTraceLog.clear();
            }
            else
            {
                // Make client pipe
                CloseHandle(m_hPipe);
                m_hPipe = INVALID_HANDLE_VALUE;
                _stprintf(logPipeName, _T("\\\\.\\pipe\\IBScanU.log"));
                if (WaitNamedPipe(logPipeName, 1000))
                {
                    m_hPipe = CreateFile(logPipeName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                }
            }
        }
        else
        {
            m_pListTraceLog.clear();
        }
    }

    if (m_hPipe != INVALID_HANDLE_VALUE)
    {
        while ((int)m_pListTraceLog.size())
        {
            tLog = (TraceLog)m_pListTraceLog.at(0);
            _stprintf(szLogBuffer, _T("%s^%s^%d^%d^%d^%s"), tLog.timestamp, tLog.category,
                    tLog.code, tLog.error, tLog.tid, tLog.message);

            BOOL bRc = WriteFile(m_hPipe, szLogBuffer, (DWORD)_tcslen(szLogBuffer), &dwWritten, NULL);            if (bRc == FALSE || dwWritten == 0)
            {
                CloseHandle(m_hPipe);
                m_hPipe = INVALID_HANDLE_VALUE;
                m_pListTraceLog.clear();
                break;
            }
            m_pListTraceLog.erase(m_pListTraceLog.begin());
        }
    }
#elif defined(WINCE) 
	CThreadSync Sync;		
	WCHAR wstrMessage[__MAX_TRACE_MESSAGE_SIZE__+1];  // +1 prevents the string from ever being truncated assuming input is null terminated
	WCHAR wstrTime[25];
	TraceLog		tLog;

    while ((int)m_pListTraceLog.size())
    {
        tLog = (TraceLog)m_pListTraceLog.at(0);  // Get first message
        mbstowcs( wstrMessage, tLog.message, __MAX_TRACE_MESSAGE_SIZE__+1 );
		mbstowcs( wstrTime, tLog.timestamp, 25 );

		RETAILMSG(1, (TEXT("%d %s:%s\r\n"), tLog.error, wstrTime, wstrMessage));
        m_pListTraceLog.erase(m_pListTraceLog.begin()); // remove first message (note: CSync in caller prevents new messages from being added until this function exits)
    }
#elif __android__
    TraceLog		tLog;
    while ((int)m_pListTraceLog.size())
    {
        tLog = (TraceLog)m_pListTraceLog.at(0);
        __android_log_print(ANDROID_LOG_DEBUG, LOG_DEBUG_TAG, "[%s] %d-%s\r\n", tLog.timestamp, tLog.error, tLog.message);
        m_pListTraceLog.erase(m_pListTraceLog.begin());
    }
#else
    TraceLog		tLog;
    while ((int)m_pListTraceLog.size())
    {
        tLog = (TraceLog)m_pListTraceLog.at(0);
        printf("[%s] %d-%s\r\n", tLog.timestamp, tLog.error, tLog.message);
        m_pListTraceLog.erase(m_pListTraceLog.begin());
    }
#endif
}

void CIBScanNFIQ2App::InsertTraceLog(const char *category, int error, int tid, const char *format, ...)
{
//    CThreadSync Sync;

    // Only insert trace log if user has not disable trace log.
    if (m_traceLogEnabled)
    {
        TraceLog traceLog;

        memset(&traceLog, 0, sizeof(TraceLog));

        try
        {
            va_list ap;  // for variable args

            va_start(ap, format); // init specifying last non-var arg
            vsprintf(traceLog.message, format, ap);
            va_end(ap); // end var args
        }
        catch (...)
        {
            return;
        }

        sprintf(traceLog.category, "%s", category);
        if (error == 0)
        {
            traceLog.code = LOG_INFORMATION;
        }
        else if (error < 0)
        {
            traceLog.code = LOG_ERROR;
        }
        else
        {
            traceLog.code = LOG_WARNING;
        }
        traceLog.error = error;
        traceLog.tid = tid;

#ifdef _WINDOWS
        SYSTEMTIME currentTime;
        ::GetLocalTime(&currentTime);
        sprintf(traceLog.timestamp, "%4ld-%02ld-%02ld %02ld:%02ld:%02ld.%03ld",
                currentTime.wYear, currentTime.wMonth, currentTime.wDay,
                currentTime.wHour, currentTime.wMinute, currentTime.wSecond, currentTime.wMilliseconds);
#else
        struct timeval timeval;
        gettimeofday(&timeval, 0);
        struct tm *currentTime;
        currentTime = localtime(&timeval.tv_sec);
        sprintf(traceLog.timestamp, "%4d-%02d-%02d %02d:%02d:%02d.%03d",
                currentTime->tm_year + 1900, currentTime->tm_mon + 1, currentTime->tm_mday,
                currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec, (int)timeval.tv_usec / 1000);
#endif

        m_pListTraceLog.push_back(traceLog);

        SendToTraceLog();
    }
}





//////////////////////////////////////////////////////////////////////////////////
int CIBScanNFIQ2App::DLL_GetVersion(
    char *pVerInfo                       ///< [out] API version information \n
    ///<       Memory must be provided by caller
)
{
    int		nRc = IBSU_NFIQ2_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_NFIQ2_GetVersion ()");

    if (::IsBadReadPtr(pVerInfo, sizeof(char)))
    {
        nRc = IBSU_ERR_NFIQ2_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_NFIQ2_GetVersion ()");
        return nRc;
    }

    if (nRc == IBSU_NFIQ2_STATUS_OK)
    {
#ifdef _WINDOWS
        CFileVersionInfo verInfo;
        memset(pVerInfo, 0, sizeof(char));
        if (verInfo.Open(m_hInstance))
        {
            sprintf(pVerInfo, "%d.%d.%d.%d", verInfo.GetProductVersionMajor(),
                    verInfo.GetProductVersionMinor(),
                    verInfo.GetProductVersionBuild(),
                    verInfo.GetProductVersionQFE());
        }
        else
        {
            nRc = IBSU_ERR_NFIQ2_NOT_SUPPORTED;
        }
#elif defined(__linux__)
        strcpy(pVerInfo, pszLinuxProdutVersion);
#endif
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_NFIQ2_GetVersion (pVerInfo=%s)", pVerInfo);

    return nRc;
}

int CIBScanNFIQ2App::DLL_Initialize()
{
    int		nRc = IBSU_NFIQ2_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_NFIQ2_Initialize ()");

    if (IsInitialized())
    {
        nRc = IBSU_WRN_NFIQ2_ALREADY_INITIALIZED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_NFIQ2_Initialize ()");

        return nRc;
    }

    if (nRc == IBSU_NFIQ2_STATUS_OK)
    {
	    try
	    {
		    // do initialization
		    m_cNFIQ2.initializeNFIQ2Algorithm();
            SetInitialized(TRUE);
	    }
	    catch (NFIQException& ex)
	    {
		    // exceptions may occur e.g. if fingerprint image cannot be read or parsed
		    std::cerr << "ERROR => Return code [" << ex.getReturnCode() << "]: " << ex.getErrorMessage() << std::endl;
            SetInitialized(FALSE);
		    nRc = IBSU_ERR_NFIQ2_FAILED;
	    }
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_NFIQ2_Initialize ()");

    return nRc;
}

int CIBScanNFIQ2App::DLL_IsInitialized()
{
    int		nRc = IBSU_NFIQ2_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_NFIQ2_IsInitialized ()");

    if (IsInitialized() == FALSE)
    {
        nRc = IBSU_ERR_NFIQ2_NOT_INITIALIZED;
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_NFIQ2_IsInitialized ()");

    return nRc;
}

int CIBScanNFIQ2App::DLL_ComputeScore(
    const BYTE            *imgBuffer,                     ///< [in]  Point to image data
    const DWORD           width,                          ///< [in]  Image width
    const DWORD           height,                         ///< [in]  Image height
    const BYTE            bitsPerPixel,                   ///< [in]  Number of Bits per pixel
    int                   *pScore                         ///< [out] NFIQ2 score
)
{
    int				nRc = IBSU_NFIQ2_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_NFIQ2_ComputeScore ()");

    if (::IsBadReadPtr(pScore, sizeof(*pScore)))
    {
        nRc = IBSU_ERR_NFIQ2_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_NFIQ2_ComputeScore ()");

        return nRc;
    }

    if (IsInitialized() == FALSE)
    {
        nRc = IBSU_ERR_NFIQ2_NOT_INITIALIZED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_NFIQ2_ComputeScore ()");

        return nRc;
    }

    *pScore = -1;
    if (nRc == IBSU_NFIQ2_STATUS_OK)
    {
	    try
	    {
            NFIQ::FingerprintImageData rawImage;

	        // except for finger code
	        rawImage.m_FingerCode = 0;

	        rawImage.m_ImageDPI = e_ImageResolution_500dpi;
	        rawImage.m_ImageHeight = height;
	        rawImage.m_ImageWidth = width;
	        rawImage.assign(imgBuffer, height * width);

            std::list<NFIQ::ActionableQualityFeedback> actionableQuality;

            // compute quality now
		    // call wrapper class with fingerprint image to get score
		    // input is always raw image with set image parameters
		    std::list<NFIQ::QualityFeatureData> featureVector;
		    std::list<NFIQ::QualityFeatureSpeed> featureTimings;
		    unsigned int qualityScore = m_cNFIQ2.computeQualityScore(
			    rawImage, 
			    true, actionableQuality, // always return actionable quality
			    false, featureVector,
			    false, featureTimings);
            *pScore = qualityScore;
        }
	    catch (NFIQException& ex)
	    {
		    // exceptions may occur e.g. if fingerprint image cannot be read or parsed
//		    std::cerr << "ERROR => Return code [" << ex.getReturnCode() << "]: " << ex.getErrorMessage() << std::endl;
		    return IBSU_ERR_NFIQ2_FAILED;
	    }
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_NFIQ2_ComputeScore (*pScore=%d)",
                          *pScore);

    return nRc;
}



//////////////////////////////////////////////////////////////////////////////////



int WINAPI IBSU_NFIQ2_GetVersion(
    char *pVerInfo                       ///< [out] API version information \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetVersion(pVerInfo);
}

int WINAPI IBSU_NFIQ2_Initialize()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_Initialize();
}

int WINAPI IBSU_NFIQ2_IsInitialized()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_IsInitialized();
}

int WINAPI IBSU_NFIQ2_ComputeScore(
    const BYTE            *imgBuffer,                     ///< [in]  Point to image data
    const DWORD           width,                          ///< [in]  Image width
    const DWORD           height,                         ///< [in]  Image height
    const BYTE            bitsPerPixel,                   ///< [in]  Number of Bits per pixel
    int                   *pScore                         ///< [out] NFIQ2 score
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_ComputeScore(imgBuffer, width, height, bitsPerPixel, pScore);
}
