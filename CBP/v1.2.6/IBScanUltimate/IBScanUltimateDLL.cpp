// IBScanUltimate.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#ifdef __linux__
#ifndef __android__
int debug=0;  // To satisfy NBIS library
#endif
#endif
#include "IBScanUltimateDLL.h"
#ifdef _WINDOWS
#include "FileVersionInfo.h"
#elif defined(__linux__)
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef __libusb_latest__
#include <libusb.h>
#else
#include <usb.h>
#endif
//#ifndef __android__
//#include <libudev.h>
//#endif
#include "LinuxPort.h"
#include "memwatch.h"
#include <sys/time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include <time.h>

#if !defined(WINCE) && defined(__IBSCAN_ULTIMATE_SDK__)
#include "ximage.h"
#endif

#include "CaptureLib/IBAlgorithm.h"

#ifdef _WINDOWS
#include "nex_sdk.h"
#endif 

#if defined(__ppi__)	
#include "device_parallelApi.h"
#endif

#if defined(_WINDOWS) && !defined(WINCE)
const TCHAR *sPipeName = _T("\\\\.\\pipe\\IBTraceLogger");
#endif
const char *sTraceMainCategory = "IBScanUltimate";

#if defined(_WINDOWS) && defined(__IBSCAN_ULTIMATE_SDK__)
#pragma comment (lib , "cximage.lib" )
#pragma comment (lib , "jasper.lib" )
#pragma comment (lib , "png.lib" )
#pragma comment (lib , "zlib.lib" )
#endif

#if defined(_WINDOWS) && !defined(WINCE)
// Shared data of MemoryMap
typedef struct TMemoryMappedDLLData 
{ 
    IBSU_DeviceDescA devDesc[16];
    RESERVED_DeviceInfo RES_devDesc[16];
} TSharedData; 

static const char   *g_MMFileName = "IBSU_MEMORY_MAPPED"; 
static HANDLE       g_MapHandle; 
static TSharedData  *g_pSharedData = NULL; 
#endif

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

// CIBScanUltimateApp

// The one and only CIBScanUltimateApp object

CIBScanUltimateApp theApp;


#ifdef _WINDOWS
BEGIN_MESSAGE_MAP(CIBScanUltimateApp, CWinApp)
END_MESSAGE_MAP()

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg)
    {
        case WM_DEVICECHANGE:
            theApp.OnDeviceChange(wParam, lParam);
            break;
#if defined(_WINDOWS) && !defined(WINCE)
        case WM_POWERBROADCAST:
            theApp.OnPowerBroadCast(wParam, lParam);
            break;
#endif
    }

    return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

WNDPROC g_oldClientWindowProc;
LRESULT CALLBACK ClientWindowSubProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch( Msg )
	{
	case WM_PAINT:
		theApp.OnPaintClientWindow(wParam, lParam);
		break;
	}

	return CallWindowProc(g_oldClientWindowProc, hWnd, Msg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////////////

#if defined(_WINDOWS) && !defined(WINCE)
void CreateSharedData() 
{ 
    BOOL fInit;
    int size = sizeof(TSharedData);

    g_MapHandle = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, g_MMFileName); 

    if (g_MapHandle == NULL) 
    { 
        return;
    }

    // The first process to attach initializes memory
    fInit = (GetLastError() != ERROR_ALREADY_EXISTS);

    // Get a pointer to the file-mapped shared memory
    g_pSharedData = (TSharedData *)MapViewOfFile(g_MapHandle, FILE_MAP_ALL_ACCESS, 0, 0, size); 

    if (g_pSharedData == NULL) 
    { 
        CloseHandle(g_MapHandle); 
        return;
    }

    if (fInit)
    {
        memset(g_pSharedData, '\0', size);
    }
} 

void CloseSharedData() 
{
    if (g_pSharedData)
    {
        UnmapViewOfFile(g_pSharedData);
    }

    if (g_MapHandle)
    {
        CloseHandle(g_MapHandle);
    }
} 
#endif

// DeviceCountThreadCallback function
DWORD WINAPI DeviceCountThreadCallback(LPVOID pParameter)
{
    ThreadParam *pThreadParam  = reinterpret_cast<ThreadParam *>(pParameter);
    CIBScanUltimateApp  *pOwner = (CIBScanUltimateApp *)pThreadParam->pParentApp;
    pOwner->DeviceCountThreadCallback(pParameter);

    return 0;
}

// WindowsMessageThreadCallback function
DWORD WINAPI WindowsMessageThreadCallback(LPVOID pParameter)
{
    ThreadParam *pThreadParam  = reinterpret_cast<ThreadParam *>(pParameter);
    CIBScanUltimateApp  *pOwner = (CIBScanUltimateApp *)pThreadParam->pParentApp;
    pOwner->WindowsMessageThreadCallback(pParameter);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
#elif defined(__linux__)
bool IsBadReadPtr(const void *x, int size)
{
    return (x == NULL);
}

////////////////////////////////////////////////////////////////////////////////////////

// DeviceCountThreadCallback function
void *WINAPI DeviceCountThreadCallback(LPVOID pParameter)
{
    ThreadParam *pThreadParam  = reinterpret_cast<ThreadParam *>(pParameter);
    CIBScanUltimateApp  *pOwner = (CIBScanUltimateApp *)pThreadParam->pParentApp;
    pOwner->DeviceCountThreadCallback(pParameter);

    pthread_exit((void *) 0);
}
////////////////////////////////////////////////////////////////////////////////////////
#endif



CIBUsbManager::CIBUsbManager()
{
    m_nUsbHandle = -1;
    m_nUsbIndex = -1;
    m_bIsCommunicationBreak = FALSE;
    m_pMainCapture = NULL;
	m_bInitialized = FALSE;
	m_bCompletedOpenDeviceThread = FALSE;
    m_bEnterCallback = FALSE;
}

CIBUsbManager::~CIBUsbManager()
{
    m_nUsbHandle = -1;
    m_nUsbIndex = -1;
    m_bIsCommunicationBreak = FALSE;
    m_pMainCapture = NULL;
	m_bInitialized = FALSE;
	m_bCompletedOpenDeviceThread = FALSE;
    m_bEnterCallback = FALSE;
}

int CIBUsbManager::_Delete_MainCapture(BOOL bReleasMark, BOOL bDeleteClass)
{
	CThreadSync Sync;
	int nRc = IBSU_STATUS_OK;

	if (m_pMainCapture)
	{
		nRc = m_pMainCapture->Main_Release(bReleasMark);

		if (bDeleteClass)
		{
			delete m_pMainCapture;
			m_pMainCapture = NULL;
		}
	}

	return nRc;
}








// CIBScanUltimateApp construction

CIBScanUltimateApp::CIBScanUltimateApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
    m_hDeviceCountThread = 0;
    m_hDeviceCountThreadDestroyEvent = 0;
    m_hDeviceCountThreadArrivalEvent = 0;
    m_hDeviceCountThreadRemoveEvent = 0;
	m_hDeviceCountThreadPaintEvent = 0;

    m_hDeviceCountThreadPowerSleepEvent = 0;
    m_hDeviceCountThreadPowerResumeEvent = 0;

    // Disable trace log on Linux by default to preserve current behavior
#if defined(__linux__) && !defined(__android__)
    m_traceLogEnabled = FALSE;
#else
    m_traceLogEnabled = TRUE;
#endif

    m_hMsgWnd = 0;
    m_nDeviceCount = 0;
    m_nPPIDeviceCount = 0;
    m_pThreadPoolMgr = NULL;

    m_Clbk_DeviceCount = NULL;
    m_pContext_DeviceCount = NULL;
    m_Clbk_InitProgress = NULL;
    m_pContext_InitProgress = NULL;
    m_Clbk_AsyncOpenDevice = NULL;
    m_pContext_AsyncOpenDevice = NULL;
    m_hPipe = INVALID_HANDLE_VALUE;

    m_prevInput_height = -1;
    m_prevInput_width = -1;
    m_prevZoomout_height = -1;
    m_prevzoomout_width = -1;

    m_bAliveDeviceCountThread = FALSE;
    m_bAliveWindowsMessageThread = FALSE;

    m_pZoomOut_InImg = NULL;
    m_pZoomOut_TmpImg = NULL;
	
    m_pCombine_Image = NULL;
    m_pDecryptImage = NULL;

	pSegmentArr[0].Buffer = new unsigned char [800*750];
	pSegmentArr[1].Buffer = new unsigned char [800*750];
	pSegmentArr[2].Buffer = new unsigned char [800*750];
	pSegmentArr[3].Buffer = new unsigned char [800*750];

#ifdef __linux__
    m_applicationShutdown = false;

    // Set up the Linux USB library
#ifdef __libusb_latest__
    libusb_init(NULL);	// initialize a library session

//	libusb_set_debug(NULL, 3); // set verbosity level to 3, as suggested in the documentation 
#else
    usb_init();

    // Rescan USB bus
    usb_find_busses();
    usb_find_devices();
#endif
/* Calling libusb_init here on Windows Mobile doesn't work because it's called by DllMainCRTStartup which is during DLLMain execution 
 * and libusb_init will try to wait for a thread to start running. But it won't start running until it load DLL's and call other DLLMains.
 * so it's a deadlock. Which is why the delayed initializtion mechanism was created.
#elif defined(__USE_LIBUSBX_DRIVER__)
	// Setup the CE version of libusb library
	libusb_init(NULL);              //&&&Init
*/
#endif 

#ifdef _WINDOWS
	m_hClientWnd = NULL;
    m_hWindowsMessageThread = 0;
    m_hWindowsMessageThreadDestroyEvent = 0;
#endif

	m_bOnloadLibrary = TRUE;

#ifdef _WINDOWS
	// intialize nex_sdk
	nex_sdk_initialize();
	nex_sdk_load_model(NEX_SDK_CLASSIFIER_MLP);
#endif
}


// CIBScanUltimateApp initialization
#ifdef _WINDOWS
BOOL CIBScanUltimateApp::InitInstance()
{
    CWinApp::InitInstance();

//    CreateMsgWnd(m_hInstance, &m_hMsgWnd);

#if defined(_WINDOWS) && !defined(WINCE)
    CreateSharedData();
#endif

    return TRUE;
}

BOOL CIBScanUltimateApp::ExitInstance()
{
	CWinApp::ExitInstance();

#if defined(_WINDOWS) && !defined(WINCE)
    CloseSharedData();
    //SetThreadExecutionState(ES_CONTINUOUS);
#endif

    return TRUE;
}
#endif

CIBScanUltimateApp::~CIBScanUltimateApp()
{
	delete [] (unsigned char*)pSegmentArr[0].Buffer;
	delete [] (unsigned char*)pSegmentArr[1].Buffer;
	delete [] (unsigned char*)pSegmentArr[2].Buffer;
	delete [] (unsigned char*)pSegmentArr[3].Buffer;

	if (m_bOnloadLibrary)
	{
    	Close(TRUE);
	}
#ifdef _WINDOWS
	// un-intialize nex_sdk
	nex_sdk_destroy_model();
	nex_sdk_destroy();
#endif
}

BOOL CIBScanUltimateApp::Close(BOOL bTerminate)
{
// Important notice !!!
// If you use CThreadSync in this funtion,
// then we will have crash during FreeLibrary() function call
    int nRc;

	std::vector<CIBUsbManager *>::iterator it = m_pTempUsbManager.begin();
	while( it != m_pTempUsbManager.end() )
	{
		if( *it )
		{
            nRc = IBSU_STATUS_OK;
			if( (*it)->m_pMainCapture )
			{
                nRc = (*it)->m_pMainCapture->_Main_Release(FALSE, bTerminate);
                if (nRc == IBSU_STATUS_OK)
                {
					delete (*it)->m_pMainCapture;
					(*it)->m_pMainCapture = NULL;
                }
			}

            if (nRc == IBSU_STATUS_OK)
            {
			    delete *it;
			    it = m_pTempUsbManager.erase(it);
            }
		}
		else
			++it;
	}

    it = m_pListUsbManager.begin();
	while( it != m_pListUsbManager.end() )
	{
		if( *it )
		{
			if( (*it)->m_bCompletedOpenDeviceThread )
			{
                nRc = IBSU_STATUS_OK;
				if( (*it)->m_pMainCapture )
				{
                    nRc = (*it)->m_pMainCapture->_Main_Release(FALSE, bTerminate);
                    if (nRc == IBSU_STATUS_OK)
                    {
                        (*it)->m_pMainCapture->_SndUsbFwCaptureStop_atLibraryTerminated();
                        (*it)->m_pMainCapture->_SndUsbFwRelease_atLibraryTerminated();

                        delete (*it)->m_pMainCapture;
    					(*it)->m_pMainCapture = NULL;
                    }
				}

                if (nRc == IBSU_STATUS_OK)
                {
				    delete *it;
				    it = m_pListUsbManager.erase(it);
                }
			}
			else
			{
				++it;
			}
		}
		else
			++it;
	}

/*    RemoveAllCommuncationBreakDevice();
	if ((int)m_pListUsbManager.size() != 0)
	{
		DLL_CloseAllDevice(TRUE);
	}
*/
    if (m_pZoomOut_InImg)
    {
        delete [] m_pZoomOut_InImg;
		m_pZoomOut_InImg = NULL;
    }
    if (m_pZoomOut_TmpImg)
    {
        delete [] m_pZoomOut_TmpImg;
		m_pZoomOut_TmpImg = NULL;
    }
    if (m_pCombine_Image)
    {
        delete [] m_pCombine_Image;
		m_pCombine_Image = NULL;
    }
    if (m_pDecryptImage)
    {
        delete [] m_pDecryptImage;
		m_pDecryptImage = NULL;
    }

    // DestroyWindow for Window event
#ifdef _WINDOWS
    if (m_hPipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
    }

	if( m_hClientWnd )
	{
#ifdef __64BITSYSTEM
		SetWindowLong(m_hClientWnd, GWLP_WNDPROC, (LONG_PTR)g_oldClientWindowProc);
#else
		SetWindowLong(m_hClientWnd, GWL_WNDPROC, (LONG)g_oldClientWindowProc);
#endif
	}

    if (m_hMsgWnd)
    {
        // Send message with timeout, otherwise this call may block indefinitely, thereby
        // preventing the library unloading.  250-milliseconds should be enough for the
        // other task to receive this message.
#ifdef WINCE
        SendMessageTimeout(m_hMsgWnd, WM_DESTROY, 0, 0, SMTO_NORMAL, 250, NULL);
#else
        SendMessageTimeout(m_hMsgWnd, WM_DESTROY, 0, 0, SMTO_ABORTIFHUNG | SMTO_ERRORONEXIT, 250, NULL);
#endif 
        DestroyWindow(m_hMsgWnd);
    }
    m_hMsgWnd = NULL;

    // Destroy event for WindowsMessageThread
    if (m_hWindowsMessageThread)
    {
        SetEvent(m_hWindowsMessageThreadDestroyEvent);
        //		WaitForSingleObject(m_hWindowsMessageThread, INFINITE);
        DWORD nRc;
        while (TRUE)
        {
            nRc = ::WaitForSingleObject(m_hWindowsMessageThread, 1000);
            if (nRc == WAIT_OBJECT_0)
            {
                break;
            }

            if (nRc == WAIT_TIMEOUT && m_bAliveWindowsMessageThread == FALSE)
            {
                // 2012-09-17 enzyme add - Forced to kill thread due to Visual Basic
                DWORD nExitCode = NULL;
                GetExitCodeThread(m_hWindowsMessageThread, &nExitCode);
                TerminateThread(m_hWindowsMessageThread, nExitCode);
                break;
            }
        }

        CloseHandle(m_hWindowsMessageThread);
        m_hWindowsMessageThread = NULL;
    }

    if (m_hWindowsMessageThreadDestroyEvent)
    {
        CloseHandle(m_hWindowsMessageThreadDestroyEvent);
        m_hWindowsMessageThreadDestroyEvent = NULL;
    }

    // Destroy event for DeviceCountThread
    if (m_hDeviceCountThread)
    {
        SetEvent(m_hDeviceCountThreadDestroyEvent);
        //		WaitForSingleObject(m_hDeviceCountThread, INFINITE);
        DWORD nRc;
        while (TRUE)
        {
            nRc = ::WaitForSingleObject(m_hDeviceCountThread, 1000);
            if (nRc == WAIT_OBJECT_0)
            {
                break;
            }

            if (nRc == WAIT_TIMEOUT && m_bAliveDeviceCountThread == FALSE)
            {
                // 2012-09-17 enzyme add - Forced to kill thread due to Visual Basic
                DWORD nExitCode = NULL;
                GetExitCodeThread(m_hDeviceCountThread, &nExitCode);
                TerminateThread(m_hDeviceCountThread, nExitCode);
                break;
            }
        }

        CloseHandle(m_hDeviceCountThread);
        m_hDeviceCountThread = NULL;
    }

    if (m_hDeviceCountThreadDestroyEvent)
    {
        CloseHandle(m_hDeviceCountThreadDestroyEvent);
        m_hDeviceCountThreadDestroyEvent = NULL;
    }

    if (m_hDeviceCountThreadArrivalEvent)
    {
        CloseHandle(m_hDeviceCountThreadArrivalEvent);
        m_hDeviceCountThreadArrivalEvent = NULL;
    }

    if (m_hDeviceCountThreadRemoveEvent)
    {
        CloseHandle(m_hDeviceCountThreadRemoveEvent);
        m_hDeviceCountThreadRemoveEvent = NULL;
    }

	if( m_hDeviceCountThreadPaintEvent )
	{
		CloseHandle(m_hDeviceCountThreadPaintEvent);
		m_hDeviceCountThreadPaintEvent = NULL;
	}

    if (m_hDeviceCountThreadPowerSleepEvent)
    {
        CloseHandle(m_hDeviceCountThreadPowerSleepEvent);
        m_hDeviceCountThreadPowerSleepEvent = NULL;
    }

    if (m_hDeviceCountThreadPowerResumeEvent)
    {
        CloseHandle(m_hDeviceCountThreadPowerResumeEvent);
        m_hDeviceCountThreadPowerResumeEvent = NULL;
    }

#elif defined(__linux__)
    if (m_hDeviceCountThread)
    {
        m_applicationShutdown = true;
        pthread_join(m_hDeviceCountThread, NULL);
        /*		int count=0;
        		while( TRUE )
        		{
        			if( m_bAliveDeviceCountThread == FALSE )
        				break;
        			Sleep(100);
        			if( count++ >= 5 )
        				break;
        		}
#ifdef __android__
        		pthread_join(m_hDeviceCountThread, NULL);
#else
        		int rc, status;
        		rc = pthread_cancel(m_hDeviceCountThread); // Forced kill
        		if( rc == 0 && pthread_join(m_hDeviceCountThread, (void **)&status) == 0 )
        		{
        //			printf("Forced kill thread (%d)\n", status);
        		}

#endif
        */
        m_hDeviceCountThread = 0;
    }
#endif
/*
    CIBUsbManager *pIBUsbManager = NULL;

    for (int i = 0; i < (int)m_pListUsbManager.size(); i++)
    {
        pIBUsbManager = m_pListUsbManager.at(i);
        if (pIBUsbManager->m_pMainCapture)
        {
//            pIBUsbManager->m_pMainCapture->Main_Release(TRUE);
			pIBUsbManager->_Delete_MainCapture(TRUE, FALSE);
        }
        //		Sleep(10);
        //		delete (CMainCapture*)pMainCapture;
        //		pMainCapture = NULL;
    }

    if (m_pThreadPoolMgr != NULL)
    {
        m_pThreadPoolMgr->ClearThreadPool();
        delete theApp.m_pThreadPoolMgr;
        theApp.m_pThreadPoolMgr = NULL;
    }

	std::vector<CIBUsbManager *>::iterator it = m_pListUsbManager.begin();
	while( it != m_pListUsbManager.end() )
	{
		if( *it )
		{
			if( (*it)->m_pMainCapture )
			{
				delete (*it)->m_pMainCapture;
				(*it)->m_pMainCapture = NULL;
			}
			delete *it;
			it = m_pListUsbManager.erase(it);
		}
		else
			++it;
	}
*/

//	m_pListUsbHandle.clear();

    if (m_pThreadPoolMgr != NULL)
    {
        m_pThreadPoolMgr->ClearThreadPool();
        delete theApp.m_pThreadPoolMgr;
        theApp.m_pThreadPoolMgr = NULL;
    }

	_RemoveAllVector_WorkerJob();

	std::vector<AllocatedMemory *>::iterator it2 = m_pListAllocatedMemory.begin();
	while( it2 != m_pListAllocatedMemory.end() )
	{
		if( *it2 )
		{
			if( (*it2)->memblock )
			{
				delete (unsigned char*)(*it2)->memblock;
				(*it2)->memblock = NULL;
			}
			delete *it2;
			it2 = m_pListAllocatedMemory.erase(it2);
		}
		else
			++it2;
	}

	m_pListAllocatedMemory.clear();

#if defined(WINCE)
	libusb_exit(m_LibUSBContext);
#endif

#ifdef __libusb_latest__
	libusb_exit(m_LibUSBContext);
#endif

    // Try to remove CIBUsbManager again
	it = m_pListUsbManager.begin();
	while( it != m_pListUsbManager.end() )
	{
		if( *it )
		{
//			if( (*it)->m_bCompletedOpenDeviceThread )
			{
                nRc = IBSU_STATUS_OK;
				if( (*it)->m_pMainCapture )
				{
                    if ((*it)->m_pMainCapture->m_pInitializeThread)
                    {
#if defined(_WINDOWS)
                        ::WaitForSingleObject((*it)->m_pMainCapture->m_pInitializeThread->m_hThread, 1000);
                        delete (*it)->m_pMainCapture->m_pInitializeThread;
#elif defined(__linux__)
        				pthread_join((*it)->m_pMainCapture->m_pInitializeThread, NULL);
#endif
                        (*it)->m_pMainCapture->m_pInitializeThread = 0;
                    }

                    if ((*it)->m_pMainCapture->m_pAsyncInitializeThread)
                    {
#if defined(_WINDOWS)
                        ::WaitForSingleObject((*it)->m_pMainCapture->m_pAsyncInitializeThread->m_hThread, 1000);
                        delete (*it)->m_pMainCapture->m_pAsyncInitializeThread;
#elif defined(__linux__)
        				pthread_join((*it)->m_pMainCapture->m_pAsyncInitializeThread, NULL);
#endif
                        (*it)->m_pMainCapture->m_pAsyncInitializeThread = 0;
                    }

                    nRc = (*it)->m_pMainCapture->_Main_Release(FALSE, bTerminate);
                    if (nRc == IBSU_STATUS_OK)
                    {
                        (*it)->m_pMainCapture->_SndUsbFwCaptureStop_atLibraryTerminated();
                        (*it)->m_pMainCapture->_SndUsbFwRelease_atLibraryTerminated();

    					delete (*it)->m_pMainCapture;
    					(*it)->m_pMainCapture = NULL;
                    }
				}

                if (nRc == IBSU_STATUS_OK)
                {
				    delete *it;
				    it = m_pListUsbManager.erase(it);
                }
			}
//			else
//			{
//				++it;
//			}
		}
		else
			++it;
	}
	m_pListUsbManager.clear();

	m_bOnloadLibrary = FALSE;

//    CThreadSync Sync;

    return TRUE;
}

#ifdef _WINDOWS
BOOL CIBScanUltimateApp::CreateMsgWnd(HINSTANCE hInst, HWND *phWnd)
{
    WNDCLASS	stWndClass;

    stWndClass.cbClsExtra = 0;
    stWndClass.cbWndExtra = 0;
    stWndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    stWndClass.hCursor = LoadCursor(IDC_ARROW);
#ifdef WINCE
	stWndClass.hIcon = NULL;
#else
    stWndClass.hIcon = LoadIcon(IDI_APPLICATION);
    int			i;
#endif 
    stWndClass.hInstance = hInst;
    stWndClass.lpfnWndProc = (WNDPROC)WndProc;
    stWndClass.lpszClassName = _T("MsgWnd");
    stWndClass.lpszMenuName = NULL;
    stWndClass.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClass(&stWndClass);

    *phWnd = CreateWindow(_T("MsgWnd"), NULL, WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                          NULL, (HMENU)NULL, hInst, NULL);

    if (*phWnd == NULL)
    {
        return FALSE;
    }

#if defined(WINCE) 
	HANDLE hDevNotify;
//#define DEVCLASS_CEUSBKWRAPPER_GUID { 0x5a4e0f69, 0x48bf, 0x46a6, { 0x81, 0x24, 0x72, 0x17, 0x4b, 0xfb, 0x52, 0xd4 } }
//#define DEVCLASS_CEUSBKWRAPPER_NAME_PREFIX L"ceusbkwrapper"
//static const GUID ceusbkwrapper_guid = DEVCLASS_CEUSBKWRAPPER_GUID;
	GUID guid = { 0x5a4e0f69, 0x48bf, 0x46a6, { 0x81, 0x24, 0x72, 0x17, 0x4b, 0xfb, 0x52, 0xd4 } };
    hDevNotify = RequestDeviceNotifications(&guid,*phWnd, TRUE);
    if (!hDevNotify)
    {
        //			TRACE("Can't register device notification");
        return FALSE;
    }
#else
    HDEVNOTIFY hDevNotify;
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
    ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    for (i = 0; i < sizeof(GUID_DEVINTERFACE_LIST) / sizeof(GUID); i++)
    {
        NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];
        hDevNotify = RegisterDeviceNotification(*phWnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
        if (!hDevNotify)
        {
            //			TRACE("Can't register device notification");
            return FALSE;
        }
    }
#endif

    return TRUE;
}

LRESULT CIBScanUltimateApp::OnDeviceChange(WPARAM wParam, LPARAM lParam)
{
#if defined(WINCE)
	return 0;
#else
    if (DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam)
    {
        PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
        PDEV_BROADCAST_DEVICEINTERFACE pDevInf;
        switch (pHdr->dbch_devicetype)
        {
            case DBT_DEVTYP_DEVICEINTERFACE:
                pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;
                if (IsIBScanDevice(pDevInf, wParam, m_szChangedDevicePath))
                {
                    if (DBT_DEVICEARRIVAL == wParam)
                    {
                        SetEvent(m_hDeviceCountThreadArrivalEvent);
                    }
                    else
                    {
                        // Device remove completed
                        SetEvent(m_hDeviceCountThreadRemoveEvent);
                    }
                }
                break;
        }
    }
    return 0;
#endif
}

LRESULT CIBScanUltimateApp::OnPowerBroadCast(WPARAM wParam, LPARAM lParam)
{
#if defined(WINCE)
	return 0;
#else
    if (PBT_APMRESUMEAUTOMATIC == wParam)
    {
	    SetEvent(m_hDeviceCountThreadPowerResumeEvent);
    }
    else if (PBT_APMSUSPEND == wParam)
    {
	    SetEvent(m_hDeviceCountThreadPowerSleepEvent);
    }
    return 0;
#endif
}

LRESULT CIBScanUltimateApp::OnPaintClientWindow(WPARAM wParam, LPARAM lParam)
{
	SetEvent(m_hDeviceCountThreadPaintEvent);

	return 0;
}

#if defined(WINCE)
#else
BOOL CIBScanUltimateApp::IsIBScanDevice(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam, char *outNameBuf)
{
    // pDevInf->dbcc_name:
    // \\?\USB#Vid_04e8&Pid_503b#0002F9A9828E0F06#{a5dcbf10-6530-11d2-901f-00c04fb951ed}
    // szDevId: USB\Vid_04e8&Pid_503b\0002F9A9828E0F06
    // szClass: USB
    ASSERT(lstrlen(pDevInf->dbcc_name) > 4);
    CString szDevId = pDevInf->dbcc_name + 4;
    int idx = szDevId.ReverseFind(_T('#'));
    ASSERT(-1 != idx);
    szDevId.Truncate(idx);
    szDevId.Replace(_T('#'), _T('\\'));
    szDevId.MakeUpper();

    char szDeviceName[MAX_PATH];
#ifdef UNICODE
	int nBufferLength = wcstombs( NULL, szDevId.GetBuffer(0), MAX_PATH );
    wcstombs(szDeviceName, szDevId.GetBuffer(0), nBufferLength+1);
	*(szDeviceName+MAX_PATH-1) = '\0'; // Ensure string is always null terminated
#else
    strcpy(szDeviceName, szDevId.GetBuffer(0));
#endif

    // enzyme add new code 2012-10-27
    LPTSTR	lpDevicePath = NULL;
    int		pid, vid;
    size_t nLength = _tcslen((TCHAR *)pDevInf->dbcc_name) + 1;
    lpDevicePath = (TCHAR *) LocalAlloc(LPTR, nLength * sizeof(TCHAR));
    _tcsncpy(lpDevicePath, pDevInf->dbcc_name, nLength);

    lpDevicePath[nLength - 1] = 0;

    // enzyme modify 2012-10-27 I don't know why different "Vid/Pid vs vid/pid
    //	TCHAR * vidLoc = _tcsstr(lpDevicePath, _T("VID_"));
    //	TCHAR * pidLoc = _tcsstr(lpDevicePath, _T("POD_"));
    TCHAR *vidLoc = _tcsstr(_tcsupr(lpDevicePath), _T("VID_"));
    TCHAR *pidLoc = _tcsstr(_tcsupr(lpDevicePath), _T("PID_"));
    if (vidLoc)
    {
        TCHAR *endChar;
        vid = _tcstol(vidLoc + 4, &endChar, 16);
    }

    if (pidLoc)
    {
        TCHAR *endChar;
        pid = _tcstol(pidLoc + 4, &endChar, 16);
    }
    LocalFree(lpDevicePath);

    if (vid == __VID_IB__ || vid == __VID_DERMALOG__)
    {
        if (_FindSupportedDevices(pid))
        {
            strcpy(outNameBuf, szDeviceName);

            return TRUE;
        }
    }

    return FALSE;
}
#endif
#endif

#if defined(_WINDOWS)
void CIBScanUltimateApp::WindowsMessageThreadCallback(LPVOID pParameter)
{
    ThreadParam *pThreadParam  = reinterpret_cast<ThreadParam *>(pParameter);
    CIBScanUltimateApp  *pOwner = (CIBScanUltimateApp *)pThreadParam->pParentApp;
    int		count = 0;
    DWORD	Event = 0;
    CIBUsbManager	*pIBUsbManager = NULL;
	CMainCapture  	*pMainCapture = NULL;
	BOOL			isFirsttimeToCallback = TRUE;

    SetEvent(pThreadParam->threadStarted);

    m_bAliveWindowsMessageThread = TRUE;
    CreateMsgWnd(m_hInstance, &m_hMsgWnd);

    MSG msg;

    while (TRUE)
    {
#if !defined(WINCE) 
        if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            // Get message from message queue
            GetMessage(&msg, NULL, 0, 0);
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
#endif
		Event = WaitForSingleObject(m_hWindowsMessageThreadDestroyEvent, 10);
        switch (Event)
        {
            case 0:
                // Shut down event
                m_bAliveWindowsMessageThread = FALSE;
                return;
        }

        Sleep(1);
    }

    m_bAliveWindowsMessageThread = FALSE;
}
#endif

#if defined(_WINDOWS) && !defined(WINCE)
void CIBScanUltimateApp::DeviceCountThreadCallback(LPVOID pParameter)
{
    ThreadParam *pThreadParam  = reinterpret_cast<ThreadParam *>(pParameter);
    CIBScanUltimateApp  *pOwner = (CIBScanUltimateApp *)pThreadParam->pParentApp;
    //	IBSU_CallbackDeviceCount callback = (IBSU_CallbackDeviceCount)pThreadParam->callback;
    //	void*	context = pThreadParam->context;
    int		count = 0;
    DWORD	Event = 0;
    CIBUsbManager	*pIBUsbManager = NULL;
	CMainCapture  	*pMainCapture = NULL;
    int				devHandle;
	BOOL			isFirsttimeToCallback = TRUE;

    SetEvent(pThreadParam->threadStarted);

    while (TRUE)
    {
        m_bAliveDeviceCountThread = TRUE;

		Event = WaitForMultipleObjects(6, m_hDeviceCountEventArray, FALSE, INFINITE);
        switch (Event)
        {
            case 0:
                // Shut down event
                m_bAliveDeviceCountThread = FALSE;
                return;
            case 1:
                // Device Arrival
                FindAllDeviceCount();
                UpdateIndexInList(m_nDeviceCount);
                if (m_Clbk_DeviceCount && m_pContext_DeviceCount)
                {
                    InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(),
                                   "Enter CallbackDeviceCountChanged(%d, Device arrival)", m_nDeviceCount);
                    m_Clbk_DeviceCount(m_nDeviceCount, m_pContext_DeviceCount);
                    InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(),
                                   "Exit CallbackDeviceCountChanged", m_nDeviceCount);
                }
                break;
            case 2:
                // Device Removed complete
                FindAllDeviceCount();
                UpdateIndexInList(m_nDeviceCount);

                pIBUsbManager = FindUsbDeviceByDevicePath(m_szChangedDevicePath);
                if (pIBUsbManager && pIBUsbManager->m_pMainCapture)
                {
                    // Wait for termination capture thread itself if this thread is capturing
                    while (pIBUsbManager->m_pMainCapture->GetIsActiveCapture())
                    {
                        Sleep(10);
                    }

                    devHandle = pIBUsbManager->m_pMainCapture->GetDeviceHandle();		// Saved device handle
//                    pIBUsbManager->m_pMainCapture->Main_Release(FALSE);
//					pIBUsbManager->_Delete_MainCapture(FALSE, FALSE);
                    pIBUsbManager->m_bIsCommunicationBreak = TRUE;
                    pIBUsbManager->m_nUsbIndex = -1;
                    pIBUsbManager->m_pMainCapture->SetDeviceHandle(devHandle);			// Restore device handle
                    memset(pIBUsbManager->m_szDevicePath, 0, sizeof(pIBUsbManager->m_szDevicePath));
                    memset(&pIBUsbManager->m_DeviceDesc, 0, sizeof(pIBUsbManager->m_DeviceDesc));

					if (pIBUsbManager->m_bInitialized)
					{
                        //				pIBUsbManager->m_pMainCapture->_PostCallback(CALLBACK_COMMUNICATION_BREAK);
                        InsertTraceLog(sTraceMainCategory, LOG_ERROR, _GetThreadID(),
                                       "Enter CallbackCommunicationBreak (%d)", devHandle);

#if defined(__IBSCAN_ULTIMATE_SDK__)
                        IBSU_Callback	callback = (IBSU_Callback)pIBUsbManager->m_pMainCapture->m_clbkParam[CALLBACK_COMMUNICATION_BREAK].callback;
#elif defined(__IBSCAN_SDK__)
                        IBSCAN_Callback	callback = (IBSCAN_Callback)pIBUsbManager->m_pMainCapture->m_clbkParam[CALLBACK_COMMUNICATION_BREAK].callback;
#endif
                        void			*context = pIBUsbManager->m_pMainCapture->m_clbkParam[CALLBACK_COMMUNICATION_BREAK].context;
 //                       DeleteClassMainCapture(pIBUsbManager);
                        if (callback)		// enzyme add 2012-11-14 bug fixed
                        {
                            callback(devHandle, context);
                        }

                        InsertTraceLog(sTraceMainCategory, LOG_ERROR, _GetThreadID(),
                                       "Exit CallbackCommunicationBreak (%d)", devHandle);
					}
                }

                if (m_Clbk_DeviceCount && m_pContext_DeviceCount)
                {
                    InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(),
                                   "Enter CallbackDeviceCountChanged(%d, Device removed complete)", m_nDeviceCount);
                    m_Clbk_DeviceCount(m_nDeviceCount, m_pContext_DeviceCount);
                    InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(),
                                   "Exit CallbackDeviceCountChanged", m_nDeviceCount);
                }
                break;
			case 3:
				// Paint for ClientWindow event
				pMainCapture = NULL;

				for(int i=0; i<(int)m_pListUsbManager.size(); i++ )
				{
					pMainCapture = m_pListUsbManager.at(i)->m_pMainCapture;
					if( pMainCapture )
					{
							std::vector<OverlayText *>dstOverlayText;
							std::vector<OverlayTextEx *>dstOverlayTextEx;
							std::vector<OverlayLineEx *>dstOverlayLineEx;
							std::vector<OverlayQuadrangleEx *>dstOverlayQuadrangleEx;
							std::vector<OverlayShapeEx *>dstOverlayShapeEx;
							std::copy(pMainCapture->m_pOverlayText.begin(), pMainCapture->m_pOverlayText.end(), std::back_inserter(dstOverlayText));
							std::copy(pMainCapture->m_pOverlayTextEx.begin(), pMainCapture->m_pOverlayTextEx.end(), std::back_inserter(dstOverlayTextEx));
							std::copy(pMainCapture->m_pOverlayLineEx.begin(), pMainCapture->m_pOverlayLineEx.end(), std::back_inserter(dstOverlayLineEx));
							std::copy(pMainCapture->m_pOverlayQuadrangleEx.begin(), pMainCapture->m_pOverlayQuadrangleEx.end(), std::back_inserter(dstOverlayQuadrangleEx));
							std::copy(pMainCapture->m_pOverlayShapeEx.begin(), pMainCapture->m_pOverlayShapeEx.end(), std::back_inserter(dstOverlayShapeEx));
                            if (pMainCapture->m_DisplayWindow.keepRedrawLastImage == FALSE)
                            {
                                memset(pMainCapture->m_DisplayWindow.image, 0xFF, pMainCapture->m_DisplayWindow.imgWidth*pMainCapture->m_DisplayWindow.imgHeight);
                            }
							pMainCapture->_DrawClientWindow(pMainCapture->m_pAlgo->m_OutResultImg, pMainCapture->m_DisplayWindow, &dstOverlayText, &dstOverlayTextEx, &dstOverlayLineEx, &dstOverlayQuadrangleEx, &dstOverlayShapeEx);
					}
				}
				break;
			case 4:
				// Power resume message from O.S
				pMainCapture = NULL;

				for(int i=0; i<(int)m_pListUsbManager.size(); i++ )
				{
					pMainCapture = m_pListUsbManager.at(i)->m_pMainCapture;
					if( pMainCapture )
					{
						pMainCapture->m_EnterSleep = FALSE;
                        pMainCapture->m_bPowerResume = TRUE;
                        InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(), "%s_v%s (%s) is resumed from standby mode",
                            pMainCapture->m_propertyInfo.cProductID, pMainCapture->m_propertyInfo.cFirmware, pMainCapture->m_propertyInfo.cSerialNumber);
//                        if (pMainCapture->GetIsActiveCapture())
//                        {
////InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(), "_SndUsbFwCaptureStop");
//                            pMainCapture->_SndUsbFwCaptureStop();
////InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(), "_SndUsbFwCaptureStart");
//                            pMainCapture->_SndUsbFwCaptureStart();
////InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(), "_InitializeForCISRegister");
//                            pMainCapture->_InitializeForCISRegister(pMainCapture->m_UsbDeviceInfo.CisRowStart, pMainCapture->m_UsbDeviceInfo.CisColStart,
//                                pMainCapture->m_UsbDeviceInfo.CisImgWidth, pMainCapture->m_UsbDeviceInfo.CisImgHeight);
//
////InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(), "_CaptureStart_for_Decimation");
//                            pMainCapture->_CaptureStart_for_Decimation();
////InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(), "Capture_SetLEVoltage");
//                            pMainCapture->Capture_SetLEVoltage(pMainCapture->m_propertyInfo.nVoltageValue);
////InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(), "_SetLEOperationMode");
//                            pMainCapture->_SetLEOperationMode((WORD)pMainCapture->m_propertyInfo.nLEOperationMode);
////InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(), "_SetActiveLEDs");
//                            pMainCapture->_SetActiveLEDs(0, pMainCapture->m_propertyInfo.dwActiveLEDs);
//                        }
//                        else
//                        {
//                            pMainCapture->m_bPowerResume = TRUE;
//                        }
					}
                }
				break;
			case 5:
				// Power resume message from O.S
				pMainCapture = NULL;

				for(int i=0; i<(int)m_pListUsbManager.size(); i++ )
				{
					pMainCapture = m_pListUsbManager.at(i)->m_pMainCapture;
					if( pMainCapture )
					{
						pMainCapture->m_EnterSleep = TRUE;
                        InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(), "%s_v%s (%s) is enter into sleep mode",
                            pMainCapture->m_propertyInfo.cProductID, pMainCapture->m_propertyInfo.cFirmware, pMainCapture->m_propertyInfo.cSerialNumber);
					}
				}
				break;
            default:
				int existingDeviceCount = GetDeviceCount();
				FindAllDeviceCount();
				int newDeviceCount = GetDeviceCount();
				if (existingDeviceCount != newDeviceCount)
				{
//					if (isFirsttimeToCallback)
//					{
//						isFirsttimeToCallback = FALSE;
//					}
//					else
					{
						// Device count changed
						UpdateIndexInList(newDeviceCount);
						if (newDeviceCount > existingDeviceCount)
						{
							// Device Arrival
							SetEvent(m_hDeviceCountThreadArrivalEvent);
						}
						else
						{
							// Device Removed complete
							SetEvent(m_hDeviceCountThreadRemoveEvent);
						}
					}
				}
				break;
        }

        Sleep(1);
    }

    m_bAliveDeviceCountThread = FALSE;
}
#elif defined(WINCE)
BOOL CIBScanUltimateApp::EnumerateDevices (HANDLE h, MYDEV *detail)
{
  DWORD flags;
  DWORD size;
  BOOL  bRet = FALSE;

  SetLastError(0);

  while (ReadMsgQueue(h, detail, sizeof(MYDEV), &size, 1, &flags) == TRUE)
  {
#if defined(__G_PERFORMANCE_DEBUG__) && defined(WINCE)
    RETAILMSG(1, (TEXT("Device notification: %s %s, f=0x%x\n"), detail->d.szName, detail->d.fAttached ? _T("appeared") : _T("was removed"), flags));
#endif
	bRet = TRUE;
  }

#if defined(__G_PERFORMANCE_DEBUG__) && defined(WINCE)
    RETAILMSG(1, (TEXT("Leaving Enumerate, error = %d\n"), GetLastError()));
#endif
	return bRet;
}

void CIBScanUltimateApp::DeviceCountThreadCallback(LPVOID pParameter)
{
    ThreadParam *pThreadParam  = reinterpret_cast<ThreadParam *>(pParameter);
    CIBScanUltimateApp  *pOwner = (CIBScanUltimateApp *)pThreadParam->pParentApp;
    //	IBSU_CallbackDeviceCount callback = (IBSU_CallbackDeviceCount)pThreadParam->callback;
    //	void*	context = pThreadParam->context;
    int		count = 0;
    DWORD	Event = 0;
    CIBUsbManager	*pIBUsbManager = NULL;
	CMainCapture  	*pMainCapture = NULL;
    int				devHandle;
//	MSGQUEUEOPTIONS writeOptions;
	GUID guid = { 0x5a4e0f69, 0x48bf, 0x46a6, { 0x81, 0x24, 0x72, 0x17, 0x4b, 0xfb, 0x52, 0xd4 } };		// DEVCLASS_CEUSBKWRAPPER_GUID
	HANDLE hq, hn;
	MSGQUEUEOPTIONS msgopts;
	MYDEV detail;
//	BOOL  bRet;

	msgopts.dwSize = sizeof(msgopts);
	msgopts.dwFlags = 0;
	msgopts.dwMaxMessages = 0;
	msgopts.cbMaxMessage = sizeof(MYDEV);
	msgopts.bReadAccess = TRUE;

	hq = CreateMsgQueue(NULL, &msgopts);
	if (hq == 0) return;

	hn = RequestDeviceNotifications(&guid, hq, TRUE);
	EnumerateDevices(hq, &detail);

	m_hDeviceCountEventArray[1] = hq;

    SetEvent(pThreadParam->threadStarted);

    while (TRUE)
    {
        m_bAliveDeviceCountThread = TRUE;

		Event = WaitForMultipleObjects(4, m_hDeviceCountEventArray, FALSE, INFINITE);
        switch (Event)
        {
            case 0:
                // Shut down event
				EnumerateDevices(hq, &detail);
				StopDeviceNotifications(hn);
				CloseMsgQueue(hq);
                m_bAliveDeviceCountThread = FALSE;
                return;
            case 1:
                // Device Notification Message
				if (!EnumerateDevices(hq, &detail))
				{
					break;
				}

				if (detail.d.fAttached)
				{
					// Device Arrival
					FindAllDeviceCount();
					UpdateIndexInList(m_nDeviceCount);
					if (m_Clbk_DeviceCount && m_pContext_DeviceCount)
					{
						InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(),
									   "Enter CallbackDeviceCountChanged(%d, Device arrival)", m_nDeviceCount);
						m_Clbk_DeviceCount(m_nDeviceCount, m_pContext_DeviceCount);
						InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(),
									   "Exit CallbackDeviceCountChanged", m_nDeviceCount);
					}
				}
				else
				{
					FindAllDeviceCount();
					UpdateIndexInList(m_nDeviceCount);

					LPCWSTR devicePath = detail.d.szName;
					int nBufferLength = wcstombs( NULL, devicePath, 4096 );
					wcstombs(m_szChangedDevicePath, devicePath, nBufferLength+1);
					pIBUsbManager = FindUsbDeviceByDevicePath(m_szChangedDevicePath);
					if (pIBUsbManager && pIBUsbManager->m_pMainCapture)
					{
						// Wait for termination capture thread itself if this thread is capturing
	                    while (pIBUsbManager->m_pMainCapture->GetIsActiveCapture())
	                    {
	                        Sleep(10);
	                    }

						devHandle = pIBUsbManager->m_pMainCapture->GetDeviceHandle();		// Saved device handle
	//                    pIBUsbManager->m_pMainCapture->Main_Release(FALSE);
//						pIBUsbManager->_Delete_MainCapture(FALSE, FALSE);
						pIBUsbManager->m_bIsCommunicationBreak = TRUE;
						pIBUsbManager->m_nUsbIndex = -1;
						pIBUsbManager->m_pMainCapture->SetDeviceHandle(devHandle);			// Restore device handle
						memset(pIBUsbManager->m_szDevicePath, 0, sizeof(pIBUsbManager->m_szDevicePath));
						memset(&pIBUsbManager->m_DeviceDesc, 0, sizeof(pIBUsbManager->m_DeviceDesc));

						if (pIBUsbManager->m_bInitialized)
						{
						//				pIBUsbManager->m_pMainCapture->_PostCallback(CALLBACK_COMMUNICATION_BREAK);
						InsertTraceLog(sTraceMainCategory, LOG_ERROR, _GetThreadID(),
									   "Enter CallbackCommunicationBreak (%d)", devHandle);

#if defined(__IBSCAN_ULTIMATE_SDK__)
						IBSU_Callback	callback = (IBSU_Callback)pIBUsbManager->m_pMainCapture->m_clbkParam[CALLBACK_COMMUNICATION_BREAK].callback;
#elif defined(__IBSCAN_SDK__)
						IBSCAN_Callback	callback = (IBSCAN_Callback)pIBUsbManager->m_pMainCapture->m_clbkParam[CALLBACK_COMMUNICATION_BREAK].callback;
#endif
						void			*context = pIBUsbManager->m_pMainCapture->m_clbkParam[CALLBACK_COMMUNICATION_BREAK].context;
//						DeleteClassMainCapture(pIBUsbManager);
						if (callback)		// enzyme add 2012-11-14 bug fixed
						{
							callback(devHandle, context);
						}

						InsertTraceLog(sTraceMainCategory, LOG_ERROR, _GetThreadID(),
									   "Exit CallbackCommunicationBreak (%d)", devHandle);
						}
					}

					if (m_Clbk_DeviceCount && m_pContext_DeviceCount)
					{
						InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(),
									   "Enter CallbackDeviceCountChanged(%d, Device removed complete)", m_nDeviceCount);
						m_Clbk_DeviceCount(m_nDeviceCount, m_pContext_DeviceCount);
						InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(),
									   "Exit CallbackDeviceCountChanged", m_nDeviceCount);
					}
				}
                break;
            case 2:
                // Device Removed complete
/*                FindAllDeviceCount();
                UpdateIndexInList(m_nDeviceCount);

                pIBUsbManager = FindUsbDeviceByDevicePath(m_szChangedDevicePath);
                if (pIBUsbManager && pIBUsbManager->m_pMainCapture)
                {
                    // Wait for termination capture thread itself if this thread is capturing
                    while (TRUE)
                    {
                        if (!pIBUsbManager->m_pMainCapture->GetIsActiveCapture())
                        {
                            break;
                        }

                        Sleep(10);
                    }

                    devHandle = pIBUsbManager->m_pMainCapture->GetDeviceHandle();		// Saved device handle
//                    pIBUsbManager->m_pMainCapture->Main_Release(FALSE);
//					pIBUsbManager->_Delete_MainCapture(FALSE, FALSE);
                    pIBUsbManager->m_bIsCommunicationBreak = TRUE;
                    pIBUsbManager->m_nUsbIndex = -1;
                    pIBUsbManager->m_pMainCapture->SetDeviceHandle(devHandle);			// Restore device handle
                    memset(pIBUsbManager->m_szDevicePath, 0, sizeof(pIBUsbManager->m_szDevicePath));
                    memset(&pIBUsbManager->m_DeviceDesc, 0, sizeof(pIBUsbManager->m_DeviceDesc));

					if (pIBUsbManager->m_bInitialized)
					{
                        //				pIBUsbManager->m_pMainCapture->_PostCallback(CALLBACK_COMMUNICATION_BREAK);
                        InsertTraceLog(sTraceMainCategory, LOG_ERROR, _GetThreadID(),
                                       "Enter CallbackCommunicationBreak (%d)", devHandle);

#if defined(__IBSCAN_ULTIMATE_SDK__)
                        IBSU_Callback	callback = (IBSU_Callback)pIBUsbManager->m_pMainCapture->m_clbkParam[CALLBACK_COMMUNICATION_BREAK].callback;
#elif defined(__IBSCAN_SDK__)
                        IBSCAN_Callback	callback = (IBSCAN_Callback)pIBUsbManager->m_pMainCapture->m_clbkParam[CALLBACK_COMMUNICATION_BREAK].callback;
#endif
                        void			*context = pIBUsbManager->m_pMainCapture->m_clbkParam[CALLBACK_COMMUNICATION_BREAK].context;
//                        DeleteClassMainCapture(pIBUsbManager);
                        if (callback)		// enzyme add 2012-11-14 bug fixed
                        {
                            callback(devHandle, context);
                        }

                        InsertTraceLog(sTraceMainCategory, LOG_ERROR, _GetThreadID(),
                                       "Exit CallbackCommunicationBreak (%d)", devHandle);
					}
                }

                if (m_Clbk_DeviceCount && m_pContext_DeviceCount)
                {
                    InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(),
                                   "Enter CallbackDeviceCountChanged(%d, Device removed complete)", m_nDeviceCount);
                    m_Clbk_DeviceCount(m_nDeviceCount, m_pContext_DeviceCount);
                    InsertTraceLog(sTraceMainCategory, LOG_INFORMATION, _GetThreadID(),
                                   "Exit CallbackDeviceCountChanged", m_nDeviceCount);
                }
*/                break;
			case 3:
				// Paint for ClientWindow event
				pMainCapture = NULL;

				for(int i=0; i<(int)m_pListUsbManager.size(); i++ )
				{
					pMainCapture = m_pListUsbManager.at(i)->m_pMainCapture;
					if( pMainCapture )
					{
						std::vector<OverlayText *>dstOverlayText;
						std::vector<OverlayTextEx *>dstOverlayTextEx;
						std::vector<OverlayLineEx *>dstOverlayLineEx;
						std::vector<OverlayQuadrangleEx *>dstOverlayQuadrangleEx;
						std::vector<OverlayShapeEx *>dstOverlayShapeEx;
						std::copy(pMainCapture->m_pOverlayText.begin(), pMainCapture->m_pOverlayText.end(), std::back_inserter(dstOverlayText));
						std::copy(pMainCapture->m_pOverlayTextEx.begin(), pMainCapture->m_pOverlayTextEx.end(), std::back_inserter(dstOverlayTextEx));
						std::copy(pMainCapture->m_pOverlayLineEx.begin(), pMainCapture->m_pOverlayLineEx.end(), std::back_inserter(dstOverlayLineEx));
						std::copy(pMainCapture->m_pOverlayQuadrangleEx.begin(), pMainCapture->m_pOverlayQuadrangleEx.end(), std::back_inserter(dstOverlayQuadrangleEx));
						std::copy(pMainCapture->m_pOverlayShapeEx.begin(), pMainCapture->m_pOverlayShapeEx.end(), std::back_inserter(dstOverlayShapeEx));
						pMainCapture->_DrawClientWindow(pMainCapture->m_pAlgo->m_OutResultImg, pMainCapture->m_DisplayWindow, &dstOverlayText, &dstOverlayTextEx, &dstOverlayLineEx, &dstOverlayQuadrangleEx, &dstOverlayShapeEx);
					}
				}
				break;
        }

        Sleep(1);
    }

    m_bAliveDeviceCountThread = FALSE;
}
#elif defined(__android__) || defined(__linux__)
void CIBScanUltimateApp::DeviceCountThreadCallback(LPVOID pParameter)
{
    ThreadParam *pThreadParam  = reinterpret_cast<ThreadParam *>(pParameter);
//    CIBScanUltimateApp  *pOwner = (CIBScanUltimateApp *)pThreadParam->pParentApp;
    //	IBSU_CallbackDeviceCount callback = (IBSU_CallbackDeviceCount)pThreadParam->callback;
    //	void*	context = pThreadParam->context;
//    int		count = 0;
//    DWORD	Event = 0;
    int existingDeviceCount = -1;
    BOOL bDeiveArrival = FALSE;

    pThreadParam->threadStarted = TRUE;
    // disable udev scanning for Android - not available

    while (TRUE)
    {
        m_bAliveDeviceCountThread = TRUE;

        if (m_applicationShutdown)
        {
            m_bAliveDeviceCountThread = FALSE;
            break;
        }

#ifndef __libusb_latest__
        // Rescan USB bus
        usb_find_busses();
        usb_find_devices();
#endif
        theApp.FindAllDeviceCount();
        int newDeviceCount = theApp.GetDeviceCount();
        if (existingDeviceCount == -1)
        {
            existingDeviceCount = newDeviceCount;
        }

        if (existingDeviceCount != newDeviceCount)
        {
            // Device count changed
            theApp.UpdateIndexInList(newDeviceCount);
            bDeiveArrival = FALSE;
            if (newDeviceCount > existingDeviceCount)
            {
                bDeiveArrival = TRUE;
            }
            existingDeviceCount = newDeviceCount;

            if (bDeiveArrival)
            {
                // Device Arrival
                if (m_Clbk_DeviceCount && m_pContext_DeviceCount)
                {
                    m_Clbk_DeviceCount(newDeviceCount, m_pContext_DeviceCount);
                }
            }
            else
            {
                CIBUsbManager	*pIBUsbManager = NULL;
                int				devHandle;

                // Device Removed complete
                pIBUsbManager = FindUsbDeviceByInvalidDevIndex();
                if (pIBUsbManager && pIBUsbManager->m_pMainCapture)
                {
                    // Wait for termination capture thread itself if this thread is capturing
                    while (pIBUsbManager->m_pMainCapture->GetIsActiveCapture())
                    {
                        Sleep(10);
                    }

                    devHandle = pIBUsbManager->m_pMainCapture->GetDeviceHandle();		// Saved device handle
//                    pIBUsbManager->m_pMainCapture->Main_Release(FALSE);
//					pIBUsbManager->_Delete_MainCapture(FALSE, FALSE);
                    pIBUsbManager->m_bIsCommunicationBreak = TRUE;
                    pIBUsbManager->m_nUsbIndex = -1;
                    pIBUsbManager->m_pMainCapture->SetDeviceHandle(devHandle);			// Restore device handle

					if (pIBUsbManager->m_bInitialized)
					{
                        InsertTraceLog(sTraceMainCategory, LOG_ERROR, _GetThreadID(),
                                       "Enter CallbackCommunicationBreak (%d)", devHandle);

#if defined(__IBSCAN_ULTIMATE_SDK__)
                        IBSU_Callback	callback = (IBSU_Callback)pIBUsbManager->m_pMainCapture->m_clbkParam[CALLBACK_COMMUNICATION_BREAK].callback;
#elif defined(__IBSCAN_SDK__)
                        IBSCAN_Callback	callback = (IBSCAN_Callback)pIBUsbManager->m_pMainCapture->m_clbkParam[CALLBACK_COMMUNICATION_BREAK].callback;
#endif
                        void			*context = pIBUsbManager->m_pMainCapture->m_clbkParam[CALLBACK_COMMUNICATION_BREAK].context;
//                        DeleteClassMainCapture(pIBUsbManager);
                        if (callback)
                        {
                            callback(devHandle, context);
                        }

                        InsertTraceLog(sTraceMainCategory, LOG_ERROR, _GetThreadID(),
                                       "Exit CallbackCommunicationBreak (%d)", devHandle);
                    }
                }

                if (m_Clbk_DeviceCount && m_pContext_DeviceCount)
                {
                    m_Clbk_DeviceCount(newDeviceCount, m_pContext_DeviceCount);
                }
            }
        }
        Sleep(250);
    }

    m_bAliveDeviceCountThread = FALSE;
}
#endif

int CIBScanUltimateApp::_GetThreadID()
{
#ifdef _WINDOWS
    return (int)GetCurrentThreadId();
#else
    return pthread_self();
#endif
}

void CIBScanUltimateApp::SendToTraceLog()
{
#if defined(_WINDOWS) && !defined(WINCE)
    CThreadSync Sync;

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
        __android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[%s] %d-%s\r\n", tLog.timestamp, tLog.error, tLog.message);
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

void CIBScanUltimateApp::InsertTraceLog(const char *category, int error, int tid, const char *format, ...)
{
    CThreadSync Sync;

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

void CIBScanUltimateApp::SubmitJob(LPVOID pParameter)
{
    //	CThreadSync Sync;
    // Create a new job object on the HEAP.
    // Do not create on the stack, else by the time the job is run,
    // the object would have become stack garbage
    // NOTE:	We are not deleting the pointer here, it is deleted in the notifier
    CJob *pJob = new CJob;

    pJob->m_pJobParam	= (LPVOID)(pParameter);  // The parameters to the job function
    pJob->m_pJobFunc	= JobProc; // The actual job function
    pJob->m_pNotifier	= JobNotifier; // The notifier to be called after the job completes
	pJob->m_pParentApp	= this;

	{
		CThreadSync Sync;

		m_pListWorkerJob.push_back(pJob);
	}

    if (m_pThreadPoolMgr)
    {
        m_pThreadPoolMgr->SubmitJob(pJob);    // Submit the job
    }
}

DWORD CIBScanUltimateApp::JobProc(LPVOID p_pJobParam)
{
    //	CThreadSync Sync;
    // Just update the progress bar which has been passed as a parameter
    CallbackParam		*pCallbackParam  = reinterpret_cast<CallbackParam *>(p_pJobParam);
    CIBScanUltimateApp	*pOwner = (CIBScanUltimateApp *)pCallbackParam->pParentApp;
    CIBUsbManager		*pIBUsbManager = (CIBUsbManager *)pCallbackParam->pIBUsbManager;
    CMainCapture		*pMainCapture = (CMainCapture *)pCallbackParam->pMainCapture;
    void				*context = pCallbackParam->context;
    int					funcIndex = (int)pCallbackParam->funcIndex;
    TraceLog			traceLog;


    if ((funcIndex < 0 && funcIndex >= __MAX_CALLBACK_COUNT__) ||
        !pIBUsbManager || !pMainCapture || /*!pCallbackParam->callback || */pMainCapture->m_bCaptureThread_StopMessage)
    {
        return 0;
    }

    pIBUsbManager->m_bEnterCallback = TRUE;

    int					devHandle = pMainCapture->GetDeviceHandle();

    try
    {
        memcpy(&traceLog, &pCallbackParam->traceLog, sizeof(TraceLog));

        switch (funcIndex)
        {
            case CALLBACK_COMMUNICATION_BREAK:
            {
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Enter CallbackCommunicationBreak (%d)", devHandle);

#if defined(__IBSCAN_ULTIMATE_SDK__)
                IBSU_Callback callback = (IBSU_Callback)pCallbackParam->callback;
#elif defined(__IBSCAN_SDK__)
                IBSCAN_Callback callback = (IBSCAN_Callback)pCallbackParam->callback;
#endif

				if(pCallbackParam->callback)
					callback(devHandle, context);

                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Exit CallbackCommunicationBreak (%d)", devHandle);
            }
            break;
            case CALLBACK_INIT_PROGRESS:
            {
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Enter CallbackInitProgress (%d %d%%)", pMainCapture->GetDeviceIndex(), pMainCapture->m_clbkProperty.nProgressValue);

#if defined(__IBSCAN_ULTIMATE_SDK__)
                IBSU_CallbackInitProgress callback = (IBSU_CallbackInitProgress)pCallbackParam->callback;
#elif defined(__IBSCAN_SDK__)
                IBSCAN_CallbackInitProgress callback = (IBSCAN_CallbackInitProgress)pCallbackParam->callback;
#endif
				
				if(pCallbackParam->callback)
					callback(pMainCapture->GetDeviceIndex(), context, pMainCapture->m_clbkProperty.nProgressValue);

                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Exit CallbackInitProgress (%d)", pMainCapture->GetDeviceIndex());
            }
            break;
            case CALLBACK_PREVIEW_IMAGE:
            {
				if(pMainCapture->m_propertyInfo.bEnableTOF == TRUE)
				{
					if(pMainCapture->m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0)
						pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
						"Enter CallbackPreviewImage (%d, DAC:%2X, Min:%d,Slope:%.3f,AT:%d=>%d)", devHandle, 
										   pMainCapture->m_CurrentCaptureDAC, pMainCapture->m_nAnalogTouch_Min, pMainCapture->m_nAnalogTouch_Slope, 
										   pMainCapture->m_nAnalogTouch_PlateOrg, pMainCapture->m_nAnalogTouch_Plate);
					else if(pMainCapture->m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
					{
						if(pMainCapture->m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
						{
							pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
								"Enter CallbackPreviewImage (%d, ver:%2X, DAC:%2X, CS:%X, Thres:%X, AT1:%d, AT2:%d, Thres:%d)", devHandle, pMainCapture->m_propertyInfo.sKojakCalibrationInfo.Version, 
											pMainCapture->m_CurrentCaptureDAC, pMainCapture->m_TOF_CurVal, pMainCapture->m_TOF_Thres,
											pMainCapture->m_TOF_Analog, pMainCapture->m_TOF_Analog2, pMainCapture->m_TOF_Analog_Thres);
						}
						else
						{
							pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
								"Enter CallbackPreviewImage (%d, ver:%2X, DAC:%2X, CS:%X, Thres:%X)", devHandle, pMainCapture->m_propertyInfo.sKojakCalibrationInfo.Version, 
											pMainCapture->m_CurrentCaptureDAC, pMainCapture->m_TOF_CurVal, pMainCapture->m_TOF_Thres);
						}
					}
				    else
                    {
					    pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
						                       "Enter CallbackPreviewImage (%d)", devHandle);
                    }
				}
				else
                {
					pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
						                   "Enter CallbackPreviewImage (%d)", devHandle);
                }

#if defined(__IBSCAN_ULTIMATE_SDK__)
                IBSU_CallbackPreviewImage callback = (IBSU_CallbackPreviewImage)pCallbackParam->callback;
#elif defined(__IBSCAN_SDK__)
                IBSCAN_CallbackPreviewImage callback = (IBSCAN_CallbackPreviewImage)pCallbackParam->callback;
#endif

				if(pCallbackParam->callback)
#if defined(__IBSCAN_ULTIMATE_SDK__)
					callback(pMainCapture->GetDeviceHandle(), context, pMainCapture->m_clbkProperty.imageInfo);
#elif defined(__IBSCAN_SDK__)
					callback(pMainCapture->GetDeviceHandle(), context, (const ImageData*)&pMainCapture->m_clbkProperty.imageInfo);
#endif

                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Exit CallbackPreviewImage (%d)", devHandle);
            }
            break;
            case CALLBACK_FINGER_COUNT:
            {
#if defined(__IBSCAN_ULTIMATE_SDK__)
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Enter CallbackFingerCountChanged (%d %d)", devHandle, pMainCapture->m_clbkProperty.nFingerState);

                IBSU_CallbackFingerCount callback = (IBSU_CallbackFingerCount)pCallbackParam->callback;

				if(pCallbackParam->callback)
					callback(pMainCapture->GetDeviceHandle(), context, (IBSU_FingerCountState)pMainCapture->m_clbkProperty.nFingerState);

                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Exit CallbackFingerCountChanged (%d)", devHandle);
#elif defined(__IBSCAN_SDK__)
				pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Enter CallbackObjectCountChanged (%d %d)", devHandle, pMainCapture->m_clbkProperty.nFingerState);

                IBSCAN_CallbackObjectCount callback = (IBSCAN_CallbackObjectCount)pCallbackParam->callback;

				if(pCallbackParam->callback)
					callback(pMainCapture->GetDeviceHandle(), context, (IBSU_FingerCountState)pMainCapture->m_clbkProperty.nFingerState);

                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Exit CallbackObjectCountChanged (%d)", devHandle);
#endif
            }
            break;
            case CALLBACK_FINGER_QUALITY:
            {
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Enter CallbackFingerQualityChanged (%d %d - %d - %d - %d)", devHandle,
                                       pMainCapture->m_clbkProperty.qualityArray[0], pMainCapture->m_clbkProperty.qualityArray[1],
                                       pMainCapture->m_clbkProperty.qualityArray[2], pMainCapture->m_clbkProperty.qualityArray[3]);

#if defined(__IBSCAN_ULTIMATE_SDK__)
                IBSU_CallbackFingerQuality callback = (IBSU_CallbackFingerQuality)pCallbackParam->callback;

				if(pCallbackParam->callback)
					callback(pMainCapture->GetDeviceHandle(), context,
                         (IBSU_FingerQualityState *)pMainCapture->m_clbkProperty.qualityArray, 4);
#endif
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Exit CallbackFingerQualityChanged (%d)", devHandle);
            }
            break;
            case CALLBACK_TAKING_ACQUISITION:
            {
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Enter CallbackTakingAcquisition (%d %d)", devHandle, pMainCapture->m_propertyInfo.ImageType);

#if defined(__IBSCAN_ULTIMATE_SDK__)
                IBSU_CallbackTakingAcquisition callback = (IBSU_CallbackTakingAcquisition)pCallbackParam->callback;

				if(pCallbackParam->callback)
					callback(pMainCapture->GetDeviceHandle(), context, (IBSU_ImageType)pMainCapture->m_propertyInfo.ImageType);
#endif
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Exit CallbackTakingAcquisition (%d)", devHandle);
            }
            break;
            case CALLBACK_COMPLETE_ACQUISITION:
            {
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Enter CallbackCompleteAcquisition (%d %d)", devHandle, pMainCapture->m_propertyInfo.ImageType);

#if defined(__IBSCAN_ULTIMATE_SDK__)
                IBSU_CallbackCompleteAcquisition callback = (IBSU_CallbackCompleteAcquisition)pCallbackParam->callback;

				if(pCallbackParam->callback)
					callback(pMainCapture->GetDeviceHandle(), context, (IBSU_ImageType)pMainCapture->m_propertyInfo.ImageType);
#endif
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Exit CallbackCompleteAcquisition (%d)", devHandle);
            }
            break;
            case CALLBACK_RESULT_IMAGE:
            {
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Enter CallbackResultImage (%d %d - %dx%d - %f ms)", devHandle, pMainCapture->m_propertyInfo.ImageType,
                                       pMainCapture->m_clbkProperty.imageInfo.Width, pMainCapture->m_clbkProperty.imageInfo.Height,
                                       (double)pMainCapture->m_clbkProperty.imageInfo.FrameTime * 1000);

#if defined(__IBSCAN_ULTIMATE_SDK__)
                IBSU_CallbackResultImage callback = (IBSU_CallbackResultImage)pCallbackParam->callback;

				if(pCallbackParam->callback)
					callback(pMainCapture->GetDeviceHandle(), context,
                         pMainCapture->m_clbkProperty.imageInfo, (IBSU_ImageType)pMainCapture->m_propertyInfo.ImageType,
                         pMainCapture->m_clbkProperty.pSplitImageArray, pMainCapture->m_clbkProperty.splitImageArrayCount);
#endif
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Exit CallbackResultImage (%d)", devHandle);
            }
            break;
            case CALLBACK_CLEAR_PLATEN:
            {
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Enter CallbackClearPlaten (%d %d)", devHandle, pMainCapture->m_clbkProperty.nPlatenState);

#if defined(__IBSCAN_ULTIMATE_SDK__)
                IBSU_CallbackClearPlatenAtCapture callback = (IBSU_CallbackClearPlatenAtCapture)pCallbackParam->callback;

				if(pCallbackParam->callback)
					callback(pMainCapture->GetDeviceHandle(), context, (IBSU_PlatenState)pMainCapture->m_clbkProperty.nPlatenState);
#endif
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Exit CallbackClearPlaten (%d)", devHandle);
            }
            break;
#if defined(__IBSCAN_SDK__)
    		case CALLBACK_ONE_FRAME_IMAGE:
			{
				pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
					"Enter CallbackOneFrameImage (%d)", devHandle);

				IBSCAN_CallbackRawImage callback = (IBSCAN_CallbackRawImage)pCallbackParam->callback;
				callback(devHandle, context, &pMainCapture->m_clbkProperty.oneFrameImageInfo);

				pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
					"Exit CallbackOneFrameImage (%d)", devHandle);
			}
			break;
    		case CALLBACK_TAKE_PREVIEW_IMAGE:
			{
				pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
					"Enter CallbackTakePreviewImage (%d)", devHandle);

				IBSCAN_CallbackPreviewImage callback = (IBSCAN_CallbackPreviewImage)pCallbackParam->callback;
				callback(devHandle, context, &pMainCapture->m_clbkProperty.takePreviewImageInfo);

				pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
					"Exit CallbackTakePreviewImage (%d)", devHandle);
			}
			break;
    		case CALLBACK_TAKE_RESULT_IMAGE:
			{
				pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
					"Enter CallbackTakeResultImage (%d)", devHandle);

				IBSCAN_CallbackPreviewImage callback = (IBSCAN_CallbackPreviewImage)pCallbackParam->callback;
				callback(devHandle, context, &pMainCapture->m_clbkProperty.takeResultImageInfo);

				pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
					"Exit CallbackTakeResultImage (%d)", devHandle);
			}
			break;
#endif
            case CALLBACK_TRACE_LOG:
            {
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid, traceLog.message);
            }
            break;
            case CALLBACK_ASYNC_OPEN_DEVICE:
            {
                // 20130219 enzyme add - memory leak bug fixed
                if (pMainCapture->m_pAsyncInitializeThread)
                {
#if defined(_WINDOWS)
	                    ::WaitForSingleObject(pMainCapture->m_pAsyncInitializeThread->m_hThread, INFINITE);
	                    delete pMainCapture->m_pAsyncInitializeThread;
#elif defined(__linux__)
						int status=0;
					
#if defined(__android__)
						{
							if ((status = pthread_kill(pMainCapture->m_pAsyncInitializeThread, SIGUSR1)) != 0)
							{
							    pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                					"[MT] JobProc/CALLBACK_ASYNC_OPEN_DEVICE - Error killing pthread");
							}
						}
#else // __linux__
                        pthread_cancel(pMainCapture->m_pAsyncInitializeThread);
#endif
        				pthread_join(pMainCapture->m_pAsyncInitializeThread, NULL);
#endif
                    pMainCapture->m_pAsyncInitializeThread = 0;
                }

                if (pMainCapture->m_clbkProperty.errorCode < 0)
                {
                    devHandle = -1;
                }
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Enter CallbackAsyncOpenDevice (%d %d %d)", pMainCapture->GetDeviceIndex(), devHandle, pMainCapture->m_clbkProperty.errorCode);

#if defined(__IBSCAN_ULTIMATE_SDK__)
                IBSU_CallbackAsyncOpenDevice callback = (IBSU_CallbackAsyncOpenDevice)pCallbackParam->callback;

				if(pCallbackParam->callback)
					callback(pMainCapture->GetDeviceIndex(), context, devHandle, pMainCapture->m_clbkProperty.errorCode);
#endif
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Exit CallbackAsyncOpenDevice (%d)", pMainCapture->GetDeviceIndex());
            }
            break;
            case CALLBACK_NOTIFY_MESSAGE:
            {
                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Enter CallbackNotifyMessage (%d %d)", devHandle, pMainCapture->m_clbkProperty.warningCode);

#if defined(__IBSCAN_ULTIMATE_SDK__)
                IBSU_CallbackNotifyMessage callback = (IBSU_CallbackNotifyMessage)pCallbackParam->callback;
#elif defined(__IBSCAN_SDK__)
                IBSCAN_CallbackNotifyMessage callback = (IBSCAN_CallbackNotifyMessage)pCallbackParam->callback;
#endif

				if(pCallbackParam->callback)
					callback(pMainCapture->GetDeviceHandle(), context, pMainCapture->m_clbkProperty.warningCode);

                pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                                       "Exit CallbackNotifyMessage (%d)", devHandle);
            }
            break;
			case CALLBACK_RESULT_IMAGE_EX:
			{
				pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
					"Enter CallbackResultImageEx (%d %d - %dx%d - %f ms)", devHandle, pMainCapture->m_propertyInfo.ImageType,
					pMainCapture->m_clbkProperty.imageInfo.Width, pMainCapture->m_clbkProperty.imageInfo.Height,
					(double)pMainCapture->m_clbkProperty.imageInfo.FrameTime*1000);

#if defined(__IBSCAN_ULTIMATE_SDK__)
				IBSU_CallbackResultImageEx callback = (IBSU_CallbackResultImageEx)pCallbackParam->callback;

				if(pCallbackParam->callback)
					callback(pMainCapture->GetDeviceHandle(), context, pMainCapture->m_clbkProperty.nResultImageStatus,
						pMainCapture->m_clbkProperty.imageInfo, (IBSU_ImageType)pMainCapture->m_propertyInfo.ImageType,
						pMainCapture->m_clbkProperty.nDetectedFingerCount,
						pMainCapture->m_clbkProperty.segmentArrayCount, pMainCapture->m_clbkProperty.pSegmentArray, pMainCapture->m_clbkProperty.pSegmentPositionArray);
#endif
				pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
					"Exit CallbackResultImageEx (%d)", devHandle);
			}
			break;
			case CALLBACK_KEYBUTTON:
			{
				pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
					"Enter CallbackKeyButtons (%d %d)", devHandle, pMainCapture->m_clbkProperty.nButtonIndex);

#if defined(__IBSCAN_ULTIMATE_SDK__)
				IBSU_CallbackKeyButtons callback = (IBSU_CallbackKeyButtons)pCallbackParam->callback;

				if(pCallbackParam->callback)
					callback(pMainCapture->GetDeviceHandle(), context, pMainCapture->m_clbkProperty.nButtonIndex);
#endif
				pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
					"Exit CallbackKeyButtons (%d)", devHandle);
			}
			break;
			default:
                return 0;
        }

        //		pMainCapture->m_clbkParam[funcIndex].bIsReadySend = TRUE;
    }
    catch (...)
    {
#ifdef _WINDOWS
        pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                               "An unexpected error has occurred on Callback, GetLastError(%d)", GetLastError());
#else
        pOwner->InsertTraceLog(traceLog.category, traceLog.error, traceLog.tid,
                               "An unexpected error has occurred on Callback");
#endif
    }

    if (pIBUsbManager)
    {
        pIBUsbManager->m_bEnterCallback = FALSE;
    }

    return 0;
}
/////////////////////////////////////////////////////////////////////
// The notifier func, which is called by the worker thread after the main job
// has been completed.
DWORD CIBScanUltimateApp::JobNotifier(LPVOID p_pJobParam)
{
    //	CThreadSync Sync;
    // Delete the job pointer, to prevent a  leak.
    CJob *pJob = (CJob *)p_pJobParam;
	CIBScanUltimateApp *pOwner  = reinterpret_cast<CIBScanUltimateApp *>(pJob->m_pParentApp);

	{
		CThreadSync Sync;
		CallbackParam		*pCallbackParam  = reinterpret_cast<CallbackParam *>(pJob->m_pJobParam);
		CIBUsbManager		*pIBUsbManager = (CIBUsbManager *)pCallbackParam->pIBUsbManager;
		int					funcIndex = (int)pCallbackParam->funcIndex;
		
        if (pIBUsbManager)
        {
		    pIBUsbManager->m_bIsReadySend[funcIndex] = TRUE;
        }
	}

	pOwner->_RemoveWorkerJobInList(pJob);

    if (pJob)
    {
        delete pJob;
    }

    return 0;
}

#if defined(_WINDOWS) && !defined(WINCE)
BOOL CIBScanUltimateApp::FindAllDeviceCount()
{
    LPGUID pGuid = (LPGUID)&IBSCAN_DEVICE_INTERFACE_GUID;
    DWORD nDevices = 0;
    DWORD nSortedDevices = 0;

    BOOL bResult = TRUE;
    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = NULL;
    SP_DEVINFO_DATA DeviceInfoData;
    ULONG requiredLength = 0;
    LPTSTR lpDevicePath = NULL;
    int vid = 0;
    int pid = 0;

    HDEVINFO hdInfo = SetupDiGetClassDevs(pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hdInfo == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);


    for (nDevices = 0; ; nDevices++)
    {
        SP_INTERFACE_DEVICE_DATA ifData;
        ifData.cbSize = sizeof(ifData);

        /*##############################################*/
        SetupDiEnumDeviceInfo(hdInfo, nDevices, &DeviceInfoData);

        //Reset for this iteration
        if (lpDevicePath)
        {
            LocalFree(lpDevicePath);
            lpDevicePath = NULL;
        }
        if (pInterfaceDetailData)
        {
            LocalFree(pInterfaceDetailData);
            pInterfaceDetailData = NULL;
        }


        deviceInterfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

        bResult = SetupDiEnumDeviceInterfaces(hdInfo, &DeviceInfoData, pGuid, 0, &deviceInterfaceData);
        // Check if last item
        if (GetLastError() == ERROR_NO_MORE_ITEMS)
        {
            break;
        }

        //Check for some other error
        if (!bResult)
        {
            goto done;
        }

        bResult = SetupDiGetDeviceInterfaceDetail(hdInfo, &deviceInterfaceData, NULL, 0, &requiredLength, NULL);
        //Check for some other error
        if (!bResult)
        {
            if ((ERROR_INSUFFICIENT_BUFFER == GetLastError()) && (requiredLength > 0))
            {
                //we got the size, allocate buffer
                pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, requiredLength);

                if (!pInterfaceDetailData)
                {
                    goto done;
                }
            }
            else
            {
                goto done;
            }
        }

        pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        bResult = SetupDiGetDeviceInterfaceDetail(hdInfo, &deviceInterfaceData, pInterfaceDetailData, requiredLength, NULL, &DeviceInfoData);
        //Check for some other error
        if (!bResult)
        {
            goto done;
        }

        size_t nLength = _tcslen((TCHAR *)pInterfaceDetailData->DevicePath) + 1;
        lpDevicePath = (TCHAR *) LocalAlloc(LPTR, nLength * sizeof(TCHAR));
        _tcsncpy(lpDevicePath, pInterfaceDetailData->DevicePath, nLength);

        lpDevicePath[nLength - 1] = 0;

        // enzyme modify 2012-10-27 I don't know why different "Vid/Pid vs vid/pid
        //		TCHAR * vidLoc = _tcsstr(lpDevicePath, _T("vid_"));
        //		TCHAR * pidLoc = _tcsstr(lpDevicePath, _T("pid_"));
        TCHAR *vidLoc = _tcsstr(_tcsupr(lpDevicePath), _T("VID_"));
        TCHAR *pidLoc = _tcsstr(_tcsupr(lpDevicePath), _T("PID_"));
        if (vidLoc)
        {
            TCHAR *endChar;
            vid = _tcstol(vidLoc + 4, &endChar, 16);
        }

        if (pidLoc)
        {
            TCHAR *endChar;
            pid = _tcstol(pidLoc + 4, &endChar, 16);
        }

        if (vid == __VID_IB__ || vid == __VID_DERMALOG__)
        {
            if (_FindSupportedDevices(pid))
            {
                if (!SetupDiEnumInterfaceDevice(hdInfo, NULL, pGuid, nDevices, &ifData))
                {
                    if (GetLastError() == ERROR_NO_MORE_ITEMS)
                    {
                        break;
                    }
                    else
                    {
                        goto done;
                    }
                }

                nSortedDevices++;
            }
            else
            {
                if (!SetupDiEnumInterfaceDevice(hdInfo, NULL, pGuid, nDevices, &ifData))
                {
                    break;
                }
                continue;
            }
        }
        else
        {
            // enzyme delete 2012-10-27 Remove bug code when we use other Generic MS device
            // Bug fixed on 0.16.1
            //			break;
            if (!SetupDiEnumInterfaceDevice(hdInfo, NULL, pGuid, nDevices, &ifData))
            {
                break;
            }
            continue;
        }
    }

    if (lpDevicePath)
    {
        LocalFree(lpDevicePath);
    }
    if (pInterfaceDetailData)
    {
        LocalFree(pInterfaceDetailData);
    }
    SetupDiDestroyDeviceInfoList(hdInfo);

    m_nDeviceCount = nSortedDevices;

    return TRUE;

done:
    LocalFree(lpDevicePath);
    LocalFree(pInterfaceDetailData);
    SetupDiDestroyDeviceInfoList(hdInfo);
    m_nDeviceCount = 0;

    return FALSE;
}

#elif defined(_WINDOWS) && defined(WINCE)
BOOL CIBScanUltimateApp::FindAllDeviceCount()
{
	libusb_device **dev_list = NULL;
	struct libusb_device_descriptor dev_descriptor;
	int dev_list_len = 0;
	int i;
	int r;
	int nDevices = 0;

	r = DelayedLibUSBInit();
	if( r != IBSU_STATUS_OK )
	{
		DEBUGMSG(1, (TEXT("DelayedLibUSBInit failed\r\n")));
		return FALSE;
	}

	r = libusb_get_device_list(m_LibUSBContext, &dev_list);
	if (r < 0) {
//		usbi_err("get_device_list failed with error %d", r);
		m_nDeviceCount = 0;
		return FALSE;
	}

	if (r == 0) {
		libusb_free_device_list(dev_list, 1);
		/* no devices */
		m_nDeviceCount = 0;
		return TRUE;
	}

	/* Iterate over the device list, finding compatible devices and counting them. */

	dev_list_len = r;
	for (i = 0; i < dev_list_len; i++) {
		libusb_device *dev = dev_list[i];
		libusb_get_device_descriptor(dev,&dev_descriptor);

		if( dev_descriptor.idVendor == __VID_IB__ || 
			dev_descriptor.idVendor == __VID_DERMALOG__ )
		{
            if (_FindSupportedDevices(dev_descriptor.idProduct))
            {
                nDevices++;
//                m_pUSBDevice.push_back(dev);
            }
		}	
	}
	m_nDeviceCount = nDevices;

	libusb_free_device_list(dev_list, 1);
	return TRUE;
}
#elif defined(__linux__)
#ifdef __libusb_latest__
int CIBScanUltimateApp::DelayedLibUSBInit()
{
	libusb_context *ctx;

	if( m_LibUSBContext == NULL)
	{
		int ret = libusb_init( &ctx );
		if( ret != LIBUSB_SUCCESS )
		{
			return IBSU_ERR_DEVICE_NOT_INITIALIZED;
		}
		m_LibUSBContext = ctx;
	}
	return IBSU_STATUS_OK;
}

BOOL CIBScanUltimateApp::FindAllDeviceCount()
{
    DWORD nDevices = 0;
#if defined(__ppi__)

    if(device_count((int*)&nDevices) < 0)
		m_nPPIDeviceCount = 0;
	else
		m_nPPIDeviceCount = nDevices;
#endif
	libusb_device **dev_list = NULL;
	struct libusb_device_descriptor dev_descriptor;
	int dev_list_len = 0;
    int i, r;
//    DWORD nDevices = 0;
    nDevices = 0;
    m_nDeviceCount = 0;
    r = DelayedLibUSBInit();
    if (r != IBSU_STATUS_OK)
    {
        return FALSE;
    }

    r = libusb_get_device_list(m_LibUSBContext, &dev_list);
    if (r < 0)
    {
        return FALSE;
    }

    /* Iterate over the device list, finding compatible devices and counting them. */
	dev_list_len = r;
	for (i = 0; i < dev_list_len; i++) {
		libusb_device *dev = dev_list[i];
		libusb_get_device_descriptor(dev,&dev_descriptor);

		if( dev_descriptor.idVendor == __VID_IB__ || 
			dev_descriptor.idVendor == __VID_DERMALOG__ )
		{
            if (_FindSupportedDevices(dev_descriptor.idProduct))
            {
                nDevices++;
            }
		}	
	}
    m_nDeviceCount = nDevices + m_nPPIDeviceCount;

	libusb_free_device_list(dev_list, 1);

    return TRUE;
}

BOOL CIBScanUltimateApp::GetBusInfoByIndex(int deviceIndex, int *bus_number, int *device_address, char *device_path)
{
	libusb_device **devs;
	libusb_device *dev;
	int i=0;

    int  nDevices = 0;

	libusb_get_device_list(NULL, &devs);
	while ((dev = devs[i++]) != NULL)
	{
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r<0) return FALSE;

        if (desc.idVendor == __VID_IB__ || 
			desc.idVendor == __VID_DERMALOG__ )
        {
            if (_FindSupportedDevices(desc.idProduct))
            {
                // bus->location (bus->dirname) : bus number
                // dev->devnum (dev->filename) : device address
                if (nDevices == deviceIndex)
                {
                    *bus_number = libusb_get_bus_number(dev);
                    *device_address = libusb_get_device_address(dev);
					device_path[0] = '\0';
                    //__android_log_print(ANDROID_LOG_DEBUG, LOG_DEBUG_TAG, "DeviceIndex : %d, PID : %04X, Bus number : %d, Device address : %d, Device path : %s\r\n", deviceIndex, dev->descriptor.idProduct, *bus_number, *device_address, device_path);
                    return TRUE;
                }

                nDevices++;
			}
        }
	}
	libusb_free_device_list(devs, 1);

    return FALSE;
}
#else
BOOL CIBScanUltimateApp::FindAllDeviceCount()
{
    DWORD nDevices = 0;
#if defined(__ppi__)

    if(device_count((int*)&nDevices) < 0)
		m_nPPIDeviceCount = 0;
	else
		m_nPPIDeviceCount = nDevices;
#endif
    struct usb_bus *bus;
    struct usb_device *dev;

//    DWORD nDevices = 0;
//    m_pUSBDevice.clear();
    nDevices = 0;
    usb_find_busses();
    usb_find_devices();
    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == __VID_IB__ || 
				dev->descriptor.idVendor == __VID_DERMALOG__)
            {
                if (_FindSupportedDevices(dev->descriptor.idProduct))
                {
                    nDevices++;
//                    m_pUSBDevice.push_back(dev);
                }
            }
        }
    }


    m_nDeviceCount = nDevices + m_nPPIDeviceCount;

    return TRUE;
}

BOOL CIBScanUltimateApp::GetBusInfoByIndex(int deviceIndex, int *bus_number, int *device_address, char *device_path)
{
    struct usb_bus *bus;
    struct usb_device *dev;

    int  nDevices = 0;


    usb_find_busses();
    usb_find_devices();
    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == __VID_IB__ ||
				dev->descriptor.idVendor == __VID_DERMALOG__)
            {
                if (_FindSupportedDevices(dev->descriptor.idProduct))
                {
                    // bus->location (bus->dirname) : bus number
                    // dev->devnum (dev->filename) : device address
                    if (nDevices == deviceIndex)
                    {
//                        *bus_number = atoi(bus->dirname);
                        *bus_number = bus->location;
                        *device_address = dev->devnum;
//                        GetSysPathFromBus(*bus_number, *device_address, device_path);
                        device_path[0] = '\0';
                        //__android_log_print(ANDROID_LOG_DEBUG, LOG_DEBUG_TAG, "DeviceIndex : %d, PID : %04X, Bus number : %d, Device address : %d, Device path : %s\r\n", deviceIndex, dev->descriptor.idProduct, *bus_number, *device_address, device_path);
                        return TRUE;
                    }

                    nDevices++;
                }
            }
        }
    }

    return FALSE;
}
#endif
/*
BOOL CIBScanUltimateApp::GetSysPathFromBus(int bus_number, int device_address, char *device_path)
{
    BOOL bResult = FALSE;
    DIR	*dir = opendir("/sys/bus/usb/devices");
    DIR *subdir;
    struct dirent *entry;
    char devPath[IBSU_MAX_STR_LEN];
    char devInfoName[IBSU_MAX_STR_LEN];
    int fd;
    char cValue[IBSU_MAX_STR_LEN];
    int readCount;

    device_path[0] = '\0';
    while (!bResult && (entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
        {
            continue;
        }
        sprintf(devPath, "/sys/bus/usb/devices/%s", entry->d_name);
        if ((subdir = opendir(devPath)) != NULL)
        {
            closedir(subdir);
            sprintf(devInfoName, "%s/busnum", devPath);
            fd = open(devInfoName, O_RDONLY);
            if (fd == -1)
            {
                continue;
            }
            readCount = read(fd, cValue, 4);
            close(fd);
            if (readCount < 0)
            {
                continue;
            }

            if (atoi(cValue) == bus_number)
            {
                sprintf(devInfoName, "%s/devnum", devPath);
                fd = open(devInfoName, O_RDONLY);
                if (fd == -1)
                {
                    continue;
                }
                readCount = read(fd, cValue, 4);
                close(fd);
                if (readCount < 0)
                {
                    continue;
                }
                if (atoi(cValue) == device_address)
                {
                    bResult = TRUE;
                    strcpy(device_path, devPath);
                    break;
                }
            }
        }
    }

    closedir(dir);

    return bResult;
}
*/

#endif

CIBUsbManager *CIBScanUltimateApp::FindUsbDeviceByIndexInList(int deviceIndex)
{
    CIBUsbManager	*pIBUsbManager = NULL;

    for (int i = 0; i < (int)m_pListUsbManager.size(); i++)
    {
        pIBUsbManager = m_pListUsbManager.at(i);
        if ((int)pIBUsbManager->m_nUsbIndex == deviceIndex)
        {
            return pIBUsbManager;
        }
    }

    return NULL;
}

CIBUsbManager *CIBScanUltimateApp::FindUsbDeviceByIndexInTempList(int deviceIndex)
{
    CIBUsbManager	*pIBUsbManager = NULL;

    for (int i = 0; i < (int)m_pTempUsbManager.size(); i++)
    {
        pIBUsbManager = m_pTempUsbManager.at(i);
        if ((int)pIBUsbManager->m_nUsbIndex == deviceIndex)
        {
            return pIBUsbManager;
        }
    }

    return NULL;
}

CIBUsbManager *CIBScanUltimateApp::FindUsbDeviceInList(int handle)
{
    CIBUsbManager	*pIBUsbManager = NULL;

    for (int i = 0; i < (int)m_pListUsbManager.size(); i++)
    {
        pIBUsbManager = m_pListUsbManager.at(i);
        if ((int)pIBUsbManager->m_nUsbHandle == handle)
        {
            return pIBUsbManager;
        }
    }

    return NULL;
}

AllocatedMemory *CIBScanUltimateApp::FindMemblockInList(void *memblock)
{
    AllocatedMemory	*pAllocatedMemory = NULL;

    for (int i = 0; i < (int)m_pListAllocatedMemory.size(); i++)
    {
        pAllocatedMemory = m_pListAllocatedMemory.at(i);
		if (pAllocatedMemory->memblock == memblock)
        {
            return pAllocatedMemory;
        }
    }

    return NULL;
}

CIBUsbManager *CIBScanUltimateApp::FindUsbDeviceByDevicePath(char *devicePath)
{
    CIBUsbManager *pIBUsbManager = NULL;

    for (int i = 0; i < (int)m_pListUsbManager.size(); i++)
    {
        pIBUsbManager = m_pListUsbManager.at(i);
        if (strcmp(pIBUsbManager->m_szDevicePath, devicePath) == 0)
        {
            //			m_pListUsbManager.RemoveAt(pos);

            return pIBUsbManager;
        }
    }

    return NULL;
}

CIBUsbManager *CIBScanUltimateApp::FindUsbDeviceByInvalidDevIndex()
{
    CIBUsbManager *pIBUsbManager = NULL;

    for (int i = 0; i < (int)m_pListUsbManager.size(); i++)
    {
        pIBUsbManager = m_pListUsbManager.at(i);
        if (pIBUsbManager->m_nUsbIndex == -1 && pIBUsbManager->m_bIsCommunicationBreak == FALSE)
        {
            //			m_pListUsbManager.RemoveAt(pos);

            return pIBUsbManager;
        }
    }

    return NULL;
}

void CIBScanUltimateApp::DeleteUsbDeviceInList(int handle)
{
	CThreadSync Sync;
	std::vector<CIBUsbManager *>::iterator it = m_pListUsbManager.begin();
	while( it != m_pListUsbManager.end() )
	{
		if( *it && (*it)->m_nUsbHandle == handle )
		{
			if( (*it)->m_pMainCapture )
			{
//				(*it)->m_pMainCapture->Main_Release(TRUE);
//				delete (*it)->m_pMainCapture;
//				(*it)->m_pMainCapture = NULL;
				delete (*it)->m_pMainCapture;
				(*it)->m_pMainCapture = NULL;
			}
			delete *it;
			it = m_pListUsbManager.erase(it);
		}
		else
			++it;
	}
}

void CIBScanUltimateApp::DeleteUsbManagerInTempList(int deviceIndex)
{
	CThreadSync Sync;
	std::vector<CIBUsbManager *>::iterator it = m_pTempUsbManager.begin();
	while( it != m_pTempUsbManager.end() )
	{
        if( *it && (*it)->m_nUsbIndex == deviceIndex )
		{
/*			if( (*it)->m_pMainCapture )
			{
				delete (*it)->m_pMainCapture;
				(*it)->m_pMainCapture = NULL;
			}
			delete *it;
*/			it = m_pTempUsbManager.erase(it);
		}
		else
			++it;
	}
}

void CIBScanUltimateApp::DeleteUsbDeviceInTempList(int deviceIndex)
{
	CThreadSync Sync;

    int nRc;
    std::vector<CIBUsbManager *>::iterator it = m_pTempUsbManager.begin();
    while( it != m_pTempUsbManager.end() )
    {
        if( *it && (*it)->m_nUsbIndex == deviceIndex )
        {
            nRc = IBSU_STATUS_OK;
            if( (*it)->m_pMainCapture )
            {
                nRc = (*it)->m_pMainCapture->_Main_Release(TRUE, TRUE);
                if (nRc == IBSU_STATUS_OK)
                {
		            delete (*it)->m_pMainCapture;
		            (*it)->m_pMainCapture = NULL;
                }
            }

            if (nRc == IBSU_STATUS_OK)
            {
                delete *it;
                it = m_pTempUsbManager.erase(it);
            }
            break;
        }
        else
            ++it;
    }
}

/*
void CIBScanUltimateApp::DeleteUsbHandleInList(int handle)
{
	CThreadSync Sync;
	std::vector<IBUsbHandle>::iterator it = m_pListUsbHandle.begin();
	while( it != m_pListUsbHandle.end() )
	{
		if( (it)->handle == handle )
		{
			it = m_pListUsbHandle.erase(it);
		}
		else
			++it;
	}
}
*/
void CIBScanUltimateApp::DeleteMemblockInList(void *memblock)
{
	CThreadSync Sync;
	std::vector<AllocatedMemory *>::iterator it = m_pListAllocatedMemory.begin();
	while( it != m_pListAllocatedMemory.end() )
	{
		if( *it && (*it)->memblock == memblock )
		{
			delete (unsigned char*)(*it)->memblock;
			(*it)->memblock = NULL;
			delete *it;
			it = m_pListAllocatedMemory.erase(it);
		}
		else
			++it;
	}
}

void CIBScanUltimateApp::RemoveAllCommuncationBreakDevice()
{
	CThreadSync Sync;
    int nRc;

	std::vector<CIBUsbManager *>::iterator it = m_pListUsbManager.begin();
	while( it != m_pListUsbManager.end() )
	{
		if( *it )
		{
			if( (*it)->m_bIsCommunicationBreak &&
                (*it)->m_bCompletedOpenDeviceThread &&
                !(*it)->m_bEnterCallback)
			{
                nRc = IBSU_STATUS_OK;
				if( (*it)->m_pMainCapture )
				{
                    nRc = (*it)->m_pMainCapture->_Main_Release(TRUE);
                    if (nRc == IBSU_STATUS_OK)
                    {
    					delete (*it)->m_pMainCapture;
    					(*it)->m_pMainCapture = NULL;
                    }
				}

                if (nRc == IBSU_STATUS_OK)
                {
				    delete *it;
				    it = m_pListUsbManager.erase(it);
                }
			}
			else
			{
				++it;
			}
		}
		else
			++it;
	}
}

// Create handle by assigning next higher int than already used, and insert into handle map
int CIBScanUltimateApp::CreateAPIHandle()
{
    CThreadSync Sync;
    CIBUsbManager	*pIBUsbManager = NULL;
//	IBUsbHandle usbHandle;
    int maxHandle = -1;
    
    for (int i = 0; i < (int)m_pListUsbManager.size(); i++)
    {
        pIBUsbManager = m_pListUsbManager.at(i);
        if (pIBUsbManager->m_nUsbHandle > maxHandle)
        {
            maxHandle = pIBUsbManager->m_nUsbHandle;
        }
    }

/*    for (int i = 0; i < (int)m_pListUsbHandle.size(); i++)
    {
        usbHandle = m_pListUsbHandle.at(i);
		if (usbHandle.handle > maxHandle)
        {
            maxHandle = usbHandle.handle;
        }
    }
*/
    maxHandle++;
    return maxHandle;
}

void CIBScanUltimateApp::UpdateIndexInList(int dwTotalDevices)
{
    CThreadSync Sync;
    CIBUsbManager *pIBUsbManager = NULL;
    int				i, j;
    char			szDeviceName[MAX_PATH];
    BOOL			bIsSetIndex;

    for (i = 0; i < (int)m_pListUsbManager.size(); i++)
    {
        //#ifdef _WINDOWS
        pIBUsbManager = m_pListUsbManager.at(i);
        //		pMainCapture->SetDeviceIndex(-1);
        bIsSetIndex = FALSE;
        for (j = 0; j < dwTotalDevices; j++)
        {
            _GetDevicePathByIndex(j, szDeviceName);
            if (strcmp(pIBUsbManager->m_szDevicePath, szDeviceName) == 0)
            {
#if defined(_WINDOWS) && !defined(WINCE)
                memcpy(&g_pSharedData->devDesc[j], &g_pSharedData->devDesc[pIBUsbManager->m_nUsbIndex], sizeof(IBSU_DeviceDescA));
                memcpy(&g_pSharedData->RES_devDesc[j], &g_pSharedData->RES_devDesc[pIBUsbManager->m_nUsbIndex], sizeof(RESERVED_DeviceInfo));
#endif
                pIBUsbManager->m_nUsbIndex = j;
                bIsSetIndex = TRUE;
                break;
            }
        }

        if (!bIsSetIndex)
        {
            pIBUsbManager->m_nUsbIndex = -1;
        }
        //#elif defined(__linux__)
        //		pIBUsbManager = m_pListUsbManager.at(i);
        //		pIBUsbManager->m_nUsbIndex = i;
        //#endif
    }

    return;
}
/*
void CIBScanUltimateApp::DeleteClassMainCapture(CIBUsbManager *pIBUsbManager)
{
    CThreadSync Sync;

    if (pIBUsbManager->m_pMainCapture)
    {
        delete pIBUsbManager->m_pMainCapture;
        pIBUsbManager->m_pMainCapture = NULL;
    }
}
*/
int CIBScanUltimateApp::Main_RegisterCallbackDeviceCount(void *callback, void *context)
{
#if defined(__IBSCAN_ULTIMATE_SDK__)
    m_Clbk_DeviceCount = (IBSU_CallbackDeviceCount)callback;
#elif defined(__IBSCAN_SDK__)
    m_Clbk_DeviceCount = (IBSCAN_CallbackDeviceCount)callback;
#endif
    m_pContext_DeviceCount = context;

    _RunDeviceCountThread();
    _RunWindowsMessageThread();

    return IBSU_STATUS_OK;
}

int CIBScanUltimateApp::Main_RegisterCallbackInitProgress(void *callback, void *context)
{
#if defined(__IBSCAN_ULTIMATE_SDK__)
    m_Clbk_InitProgress = (IBSU_CallbackInitProgress)callback;
#elif defined(__IBSCAN_SDK__)
    m_Clbk_InitProgress = (IBSCAN_CallbackInitProgress)callback;
#endif
    m_pContext_InitProgress = context;

    return IBSU_STATUS_OK;
}

int CIBScanUltimateApp::Main_RegisterCallbackAsyncOpenDevice(void *callback, void *context)
{
    m_Clbk_AsyncOpenDevice = (IBSU_CallbackAsyncOpenDevice)callback;
    m_pContext_AsyncOpenDevice = context;

    return IBSU_STATUS_OK;
}

int CIBScanUltimateApp::_RegisterCallbacks(CIBUsbManager *pIBUsbManager, IBSU_Events events, void *pEventName, void *pContext)
{
    int         nRc;

	switch( events)
	{
	case ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT:
		nRc = Main_RegisterCallbackDeviceCount(pEventName, pContext);
		break;
	case ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS:
		nRc = Main_RegisterCallbackInitProgress(pEventName, pContext);
		break;
	case ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK:
		nRc = pIBUsbManager->m_pMainCapture->RegisterCallback(CALLBACK_COMMUNICATION_BREAK, pEventName, pContext);
		break;
	case ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE:
		nRc = pIBUsbManager->m_pMainCapture->RegisterCallback(CALLBACK_PREVIEW_IMAGE, pEventName, pContext);
		break;
	case ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT:
		nRc = pIBUsbManager->m_pMainCapture->RegisterCallback(CALLBACK_FINGER_COUNT, pEventName, pContext);
		break;
	case ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY:
		nRc = pIBUsbManager->m_pMainCapture->RegisterCallback(CALLBACK_FINGER_QUALITY, pEventName, pContext);
		break;
	case ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION:
		nRc = pIBUsbManager->m_pMainCapture->RegisterCallback(CALLBACK_TAKING_ACQUISITION, pEventName, pContext);
		break;
	case ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION:
		nRc = pIBUsbManager->m_pMainCapture->RegisterCallback(CALLBACK_COMPLETE_ACQUISITION, pEventName, pContext);
		break;
	case ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE:
		nRc = pIBUsbManager->m_pMainCapture->RegisterCallback(CALLBACK_RESULT_IMAGE, pEventName, pContext);
		break;
	case ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE:
		nRc = pIBUsbManager->m_pMainCapture->RegisterCallback(CALLBACK_CLEAR_PLATEN, pEventName, pContext);
		break;
	case ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE:
		nRc = theApp.Main_RegisterCallbackAsyncOpenDevice(pEventName, pContext);
		break;
	case ENUM_IBSU_OPTIONAL_EVENT_NOTIFY_MESSAGE:
		nRc = pIBUsbManager->m_pMainCapture->RegisterCallback(CALLBACK_NOTIFY_MESSAGE, pEventName, pContext);
		break;
	case ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX:
		nRc = pIBUsbManager->m_pMainCapture->RegisterCallback(CALLBACK_RESULT_IMAGE_EX, pEventName, pContext);
		break;
	case ENUM_IBSU_ESSENTIAL_EVENT_KEYBUTTON:
		nRc = pIBUsbManager->m_pMainCapture->RegisterCallback(CALLBACK_KEYBUTTON, pEventName, pContext);
		break;
#if defined(__IBSCAN_SDK__)
	case ENUM_IBSU_ESSENTIAL_EVENT_ONE_FRAME_IMAGE:
		nRc = pIBUsbManager->m_pMainCapture->RegisterCallback(CALLBACK_ONE_FRAME_IMAGE, pEventName, pContext);
		break;
	case ENUM_IBSU_ESSENTIAL_EVENT_TAKE_PREVIEW_IMAGE:
		nRc = pIBUsbManager->m_pMainCapture->RegisterCallback(CALLBACK_TAKE_PREVIEW_IMAGE, pEventName, pContext);
		break;
	case ENUM_IBSU_ESSENTIAL_EVENT_TAKE_RESULT_IMAGE:
		nRc = pIBUsbManager->m_pMainCapture->RegisterCallback(CALLBACK_TAKE_RESULT_IMAGE, pEventName, pContext);
		break;
#endif
	default:
		nRc = IBSU_ERR_NOT_SUPPORTED;
		break;
	}

    return nRc;
}

void CIBScanUltimateApp::_flip_vertically(unsigned char *pixels, const int width, const int height, const int bytes_per_pixel)
{
    const int stride = width * bytes_per_pixel;
    unsigned char *row = new unsigned char [stride];
    unsigned char *low = pixels;
    unsigned char *high = &pixels[(height - 1) * stride];

    for (; low < high; low += stride, high -= stride)
    {
        memcpy(row, low, stride);
        memcpy(low, high, stride);
        memcpy(high, row, stride);
    }
    delete [] row;
}

int CIBScanUltimateApp::Main_GenerateDisplayImage(const IBSU_ImageData inImage, BYTE *OutImg,
        int outWidth, int outHeight, const BYTE bkColor, const IBSU_ImageFormat outFormat,
		const int outQualityLevel, const BOOL verticalFlip)
{
    CThreadSync Sync;

    // enzyme 2013-01-31 bug fixed when you use big viewer
    if (outWidth > __MAX_IMG_WIDTH__ || outHeight > __MAX_IMG_HEIGHT__)
    {
        return IBSU_ERR_INVALID_PARAM_VALUE;
    }

    int tmp_width = outWidth;
    int tmp_height = outHeight;

    if (outWidth > (int)inImage.Width)
    {
        tmp_width = inImage.Width;
    }
    if (outHeight > (int)inImage.Height)
    {
        tmp_height = inImage.Height;
    }

    float ratio_width = (float)tmp_width / (float)inImage.Width;
    float ratio_height = (float)tmp_height / (float)inImage.Height;
    int i;
    int target_x, target_y, target_width, target_height;

    if (ratio_width >= ratio_height)
    {
        target_width = tmp_height * inImage.Width / inImage.Height;
        target_height = tmp_height;
        target_x = (outWidth - target_width) / 2;
        target_y = (outHeight - target_height) / 2;
    }
    else
    {
        target_width = tmp_width;
        target_height = tmp_width * inImage.Height / inImage.Width;
        target_x = (outWidth - target_width) / 2;
        target_y = (outHeight - target_height) / 2;
    }

    if (target_x < 0)
    {
        target_x = 0;
    }
    if (target_y < 0)
    {
        target_y = 0;
    }

    target_width -= (target_width % 4);

    // bilinear interpolation
    //	BYTE *m_pZoomOut_InImg = new BYTE [inImage.Width*inImage.Height];
    //	BYTE *m_pZoomOut_TmpImg = new BYTE [target_width*target_height];
//    if (m_pZoomOut_InImg == NULL)
//    {
//        m_pZoomOut_InImg = new BYTE [__MAX_IMG_WIDTH__ * __MAX_IMG_HEIGHT__];
//    }
	if (outQualityLevel == 2)
	{
		if (m_pZoomOut_TmpImg == NULL)
		{
			m_pZoomOut_TmpImg = new BYTE [__MAX_IMG_WIDTH__ * __MAX_IMG_HEIGHT__];
		}
	}

//    memcpy(m_pZoomOut_InImg, inImage.Buffer, inImage.Width * inImage.Height);
	int x1, y1;
	switch (outFormat)
	{
	case IBSU_IMG_FORMAT_GRAY:
		{
			BYTE *pOutImg = (BYTE*)OutImg;
			BYTE *pInImg = (BYTE*)inImage.Buffer;
			memset(OutImg, bkColor, outWidth * outHeight);
			switch (outQualityLevel)
			{
			case 0:
				for (int y=0; y<target_height; y++)
				{
					y1 = y * inImage.Height / target_height;

					for (int x=0; x<target_width; x++)
					{
						x1 = x * inImage.Width/ target_width;

						pOutImg[(target_height+target_y-y-1)*outWidth+target_x+x] = pInImg[y1*inImage.Width+x1];
					}
				}
				break;
			case 1:
				for (int y=0; y<target_height-1; y++)
				{
					y1 = y * inImage.Height / target_height;

					for (int x=0; x<target_width-1; x++)
					{
						x1 = x * inImage.Width/ target_width;

						pOutImg[(target_height+target_y-y-1)*outWidth+target_x+x] = (pInImg[y1*inImage.Width+x1]+
													pInImg[y1*inImage.Width+x1+1]+
													pInImg[(y1+1)*inImage.Width+x1]+
													pInImg[(y1+1)*inImage.Width+x1+1]) >> 2;
					}
				}
				break;
			case 2:
				_ZoomOut_with_Bilinear((unsigned char*)inImage.Buffer, inImage.Width, inImage.Height, m_pZoomOut_TmpImg, target_width, target_height);
				for (i = 0; i < target_height; i++)
				{
					memcpy(&OutImg[(target_height+target_y-i-1)*outWidth+target_x], &m_pZoomOut_TmpImg[i * target_width], target_width);
				}
				break;
			}
		}

        if (!verticalFlip)
        {
            _flip_vertically(OutImg, outWidth, outHeight, 1);
        }
		break;

	case IBSU_IMG_FORMAT_RGB24:
		{
			BYTE *pOutImg = (BYTE*)OutImg;
			BYTE *pInImg = (BYTE*)inImage.Buffer;
			memset(OutImg, bkColor, outWidth * outHeight * 3);
			switch (outQualityLevel)
			{
			case 0:
				for (int y=0; y<target_height; y++)
				{
					y1 = y * inImage.Height / target_height;

					for (int x=0; x<target_width; x++)
					{
						x1 = x * inImage.Width/ target_width;
						BYTE value = pInImg[y1*inImage.Width+x1];
                        //if (value == 0xFF)
                        //    continue;

						pOutImg[((target_height+target_y-y-1)*outWidth+target_x+x)*3] = 
                        pOutImg[((target_height+target_y-y-1)*outWidth+target_x+x)*3+1] = 
                        pOutImg[((target_height+target_y-y-1)*outWidth+target_x+x)*3+2] = value;
					}
				}
				break;
			case 1:
				for (int y=0; y<target_height-1; y++)
				{
					y1 = y * inImage.Height / target_height;

					for (int x=0; x<target_width-1; x++)
					{
						x1 = x * inImage.Width/ target_width;
						BYTE value = (  pInImg[y1*inImage.Width+x1]+
									    pInImg[y1*inImage.Width+x1+1]+
									    pInImg[(y1+1)*inImage.Width+x1]+
									    pInImg[(y1+1)*inImage.Width+x1+1]) >> 2;

                        //if (value == 0xFF)
                        //    continue;

						pOutImg[((target_height+target_y-y-1)*outWidth+target_x+x)*3] = 
                        pOutImg[((target_height+target_y-y-1)*outWidth+target_x+x)*3+1] = 
                        pOutImg[((target_height+target_y-y-1)*outWidth+target_x+x)*3+2] = value;
					}
				}
				break;
			case 2:
				_ZoomOut_with_Bilinear((unsigned char*)inImage.Buffer, inImage.Width, inImage.Height, m_pZoomOut_TmpImg, target_width, target_height);
				for (int y=0; y<target_height; y++)
				{
					for (int x=0; x<target_width; x++)
					{
						BYTE value = m_pZoomOut_TmpImg[y*target_width+x];

                        //if (value == 0xFF)
                        //    continue;

						pOutImg[((target_height+target_y-y-1)*outWidth+target_x+x)*3] = 
                        pOutImg[((target_height+target_y-y-1)*outWidth+target_x+x)*3+1] = 
                        pOutImg[((target_height+target_y-y-1)*outWidth+target_x+x)*3+2] = value;
					}
				}
				break;
			}
		}

        if (!verticalFlip)
        {
            _flip_vertically(OutImg, outWidth, outHeight, 3);
        }
		break;

	case IBSU_IMG_FORMAT_RGB32:
		{
			unsigned int *pOutImg = (unsigned int*)OutImg;
			BYTE *pInImg = (BYTE*)inImage.Buffer;
			memset(OutImg, bkColor, outWidth * outHeight * 4);
			switch (outQualityLevel)
			{
			case 0:
				for (int y=0; y<target_height; y++)
				{
					y1 = y * inImage.Height / target_height;

					for (int x=0; x<target_width; x++)
					{
						x1 = x * inImage.Width/ target_width;
						BYTE value = pInImg[y1*inImage.Width+x1];
                        //if (value == 0xFF)
                        //    continue;

						pOutImg[(target_height+target_y-y-1)*outWidth+target_x+x] = (0xFF << 24) | (value << 16) | (value << 8) | value;
					}
				}
				break;
			case 1:
				for (int y=0; y<target_height-1; y++)
				{
					y1 = y * inImage.Height / target_height;

					for (int x=0; x<target_width-1; x++)
					{
						x1 = x * inImage.Width/ target_width;
						BYTE value = (  pInImg[y1*inImage.Width+x1]+
										pInImg[y1*inImage.Width+x1+1]+
										pInImg[(y1+1)*inImage.Width+x1]+
										pInImg[(y1+1)*inImage.Width+x1+1]) >> 2;

                        //if (value == 0xFF)
                        //    continue;

                        pOutImg[(target_height+target_y-y-1)*outWidth+target_x+x] = (0xFF << 24) | (value << 16) | (value << 8) | value;
					}
				}
				break;
			case 2:
				_ZoomOut_with_Bilinear((unsigned char*)inImage.Buffer, inImage.Width, inImage.Height, m_pZoomOut_TmpImg, target_width, target_height);
				for (int y=0; y<target_height; y++)
				{
					for (int x=0; x<target_width; x++)
					{
						BYTE value = m_pZoomOut_TmpImg[y*target_width+x];

                        //if (value == 0xFF)
                        //    continue;

                        pOutImg[(target_height+target_y-y-1)*outWidth+target_x+x] = (0xFF << 24) | (value << 16) | (value << 8) | value;
					}
				}
				break;
			}
		}

        if (!verticalFlip)
        {
            _flip_vertically(OutImg, outWidth, outHeight, 4);
        }
		break;

	case IBSU_IMG_FORMAT_UNKNOWN:
    default:
        break;
	}


    return IBSU_STATUS_OK;
}

int CIBScanUltimateApp::Main_GenerateZoomOutImage(const IBSU_ImageData inImage, BYTE *OutImg,
        int outWidth, int outHeight, const BYTE bkColor)
{
    CThreadSync Sync;

    // enzyme 2013-01-31 bug fixed when you use big viewer
    if (outWidth > __MAX_IMG_WIDTH__ || outHeight > __MAX_IMG_HEIGHT__)
    {
        return IBSU_ERR_INVALID_PARAM_VALUE;
    }

    int tmp_width = outWidth;
    int tmp_height = outHeight;

    if (outWidth > (int)inImage.Width)
    {
        tmp_width = inImage.Width;
    }
    if (outHeight > (int)inImage.Height)
    {
        tmp_height = inImage.Height;
    }

    float ratio_width = (float)tmp_width / (float)inImage.Width;
    float ratio_height = (float)tmp_height / (float)inImage.Height;
    int i;
    int target_x, target_y, target_width, target_height;

    if (ratio_width >= ratio_height)
    {
        target_width = tmp_height * inImage.Width / inImage.Height;
        target_height = tmp_height;
        target_x = (outWidth - target_width) / 2;
        target_y = (outHeight - target_height) / 2;
    }
    else
    {
        target_width = tmp_width;
        target_height = tmp_width * inImage.Height / inImage.Width;
        target_x = (outWidth - target_width) / 2;
        target_y = (outHeight - target_height) / 2;
    }

    if (target_x < 0)
    {
        target_x = 0;
    }
    if (target_y < 0)
    {
        target_y = 0;
    }

    target_width -= (target_width % 4);

    // bilinear interpolation
    //	BYTE *m_pZoomOut_InImg = new BYTE [inImage.Width*inImage.Height];
    //	BYTE *m_pZoomOut_TmpImg = new BYTE [target_width*target_height];
    if (m_pZoomOut_InImg == NULL)
    {
        m_pZoomOut_InImg = new BYTE [__MAX_IMG_WIDTH__ * __MAX_IMG_HEIGHT__];
    }
    if (m_pZoomOut_TmpImg == NULL)
    {
        m_pZoomOut_TmpImg = new BYTE [__MAX_IMG_WIDTH__ * __MAX_IMG_HEIGHT__];
    }

    memcpy(m_pZoomOut_InImg, inImage.Buffer, inImage.Width * inImage.Height);
    _ZoomOut_with_Bilinear(m_pZoomOut_InImg, inImage.Width, inImage.Height, m_pZoomOut_TmpImg, target_width, target_height);

    memset(OutImg, bkColor, outWidth * outHeight);
    for (i = 0; i < target_height; i++)
    {
        memcpy(&OutImg[(target_y + i)*outWidth + target_x], &m_pZoomOut_TmpImg[i * target_width], target_width);
    }
    //	delete [] m_pZoomOut_InImg;
    //	delete [] m_pZoomOut_TmpImg;

    return IBSU_STATUS_OK;
}

void CIBScanUltimateApp::_ZoomOut_with_Bilinear(unsigned char *InImg, int imgWidth, int imgHeight, unsigned char *OutImg, int outWidth, int outHeight)
{
    // bilinear interpolation
    int i, j, p, q, refy1, refy2, refx1, refx2;
    int value;
    float temp_i;
    //	int *refy1_arr = new int [outHeight];
    //	int *refy2_arr = new int [outHeight];
    //	short *p_arr = new short [outHeight];
    //	int *refx1_arr = new int [outWidth];
    //	int *refx2_arr = new int [outWidth];
    //	short *q_arr = new short [outWidth];
    int IMG_H = imgHeight;
    int IMG_W = imgWidth;

    // enzyme 2013-01-30 bug fixed
    if (imgHeight != m_prevInput_height ||
            imgWidth != m_prevInput_width ||
            outHeight != m_prevZoomout_height ||
            outWidth != m_prevzoomout_width)
    {
        m_prevInput_height = imgHeight;
        m_prevInput_width = imgWidth;
        m_prevZoomout_height = outHeight;
        m_prevzoomout_width = outWidth;

        for (i = 0; i < outHeight; i++)
        {
            temp_i = (float)i * IMG_H / outHeight;
            m_p_arr[i] = (short)((temp_i - (short)temp_i) * 128);
            m_refy1_arr[i] = (int)temp_i;
            m_refy2_arr[i] = (int)temp_i + 1;
            if (m_refy1_arr[i] > IMG_H - 1)
            {
                m_refy1_arr[i] = IMG_H - 1;
            }
            if (m_refy2_arr[i] > IMG_H - 1)
            {
                m_refy2_arr[i] = IMG_H - 1;
            }

            m_refy1_arr[i] = m_refy1_arr[i] * IMG_W;
            m_refy2_arr[i] = m_refy2_arr[i] * IMG_W;
        }
        for (i = 0; i < outWidth; i++)
        {
            temp_i = (float)i * IMG_W / outWidth;
            m_q_arr[i] = (short)((temp_i - (short)temp_i) * 128);
            m_refx1_arr[i] = (int)temp_i;
            m_refx2_arr[i] = (int)temp_i + 1;
            if (m_refx1_arr[i] > IMG_W - 1)
            {
                m_refx1_arr[i] = IMG_W - 1;
            }
            if (m_refx2_arr[i] > IMG_W - 1)
            {
                m_refx2_arr[i] = IMG_W - 1;
            }
        }
    }

    for (i = 0; i < outHeight; i++)
    {
        refy1 = m_refy1_arr[i];
        refy2 = m_refy2_arr[i];
        p = m_p_arr[i];

        for (j = 0; j < outWidth; j++)
        {
            refx1 = m_refx1_arr[j];
            refx2 = m_refx2_arr[j];
            q = m_q_arr[j];

            value = ((128 - p) * ((128 - q) * InImg[refy1 + refx1] + q * InImg[refy1 + refx2]) +
                     p * ((128 - q) * InImg[refy2 + refx1] + q * InImg[refy2 + refx2])) >> 14;

            OutImg[i * outWidth + j] = (unsigned char)value;
        }
    }

    //	delete [] refy1_arr;
    //	delete [] refy2_arr;
    //	delete [] p_arr;
    //	delete [] refx1_arr;
    //	delete [] refx2_arr;
    //	delete [] q_arr;
}

int CIBScanUltimateApp::Main_SaveBitmapImage(LPCSTR filePath, const BYTE *imgBuffer, const DWORD width, const DWORD height,
        const int pitch, const double resX, const double resY)
{
    CThreadSync Sync;

    IB_BITMAPINFO	*pBmpInfo = NULL;
    FILE		*hFile = NULL;
    BYTE		*pImageBuffer;

    pImageBuffer = new BYTE [width * height];
    memcpy(pImageBuffer, imgBuffer, width * height);

    try
    {
        IB_BITMAPFILEHEADER      m_BmpFileHeader;

        // prepare BMP Info header:
        pBmpInfo = reinterpret_cast <IB_BITMAPINFO *>
                   (new UCHAR [sizeof(IB_BITMAPINFOHEADER) + 256 * sizeof(IB_RGBQUAD)]);

        pBmpInfo->bmiHeader.biSize          = sizeof(IB_BITMAPINFOHEADER);
        pBmpInfo->bmiHeader.biWidth         = 0;
        pBmpInfo->bmiHeader.biHeight        = 0;
        pBmpInfo->bmiHeader.biPlanes        = 1;
        pBmpInfo->bmiHeader.biBitCount      = 8;
        pBmpInfo->bmiHeader.biCompression   = BI_RGB;
        pBmpInfo->bmiHeader.biSizeImage     = 0;			// requires BI_RGB
        pBmpInfo->bmiHeader.biXPelsPerMeter = 0;
        pBmpInfo->bmiHeader.biYPelsPerMeter = 0;
        pBmpInfo->bmiHeader.biClrUsed       = 0;
        pBmpInfo->bmiHeader.biClrImportant  = 0;

        for (int i = 0; i <= 255; i++)
        {
            ASSERT(i == (BYTE) i);
            pBmpInfo->bmiColors[i].rgbBlue     = (BYTE)i;
            pBmpInfo->bmiColors[i].rgbGreen    = (BYTE)i;
            pBmpInfo->bmiColors[i].rgbRed      = (BYTE)i;
            pBmpInfo->bmiColors[i].rgbReserved = 0;
        }

        // prepare BMP file header:
        m_BmpFileHeader.bfType = (unsigned short)(('M' << 8) | 'B');
        m_BmpFileHeader.bfReserved1 = 0;
        m_BmpFileHeader.bfReserved2 = 0;
        m_BmpFileHeader.bfOffBits = sizeof(IB_BITMAPFILEHEADER) + sizeof(IB_BITMAPINFOHEADER) +
                                    256 * sizeof(IB_RGBQUAD);

        // Create file
        hFile = fopen(filePath, "wb");
        if (!hFile)
        {
            if (hFile)
            {
                fclose(hFile);
            }
            delete [] pBmpInfo;
            delete [] pImageBuffer;
            return IBSU_ERR_COMMAND_FAILED;
        }

        size_t bytesToWrite;
        size_t bytesWritten;

        // Write file header
        m_BmpFileHeader.bfSize = sizeof(IB_BITMAPFILEHEADER) + sizeof(IB_BITMAPINFOHEADER) +
                                 256 * sizeof(IB_RGBQUAD) + (width * height);

        bytesToWrite = sizeof(IB_BITMAPFILEHEADER);
        bytesWritten = fwrite((char *)&m_BmpFileHeader, 1, bytesToWrite, hFile);
        if (bytesWritten != bytesToWrite)
        {
            if (hFile)
            {
                fclose(hFile);
            }
            delete [] pBmpInfo;
            delete [] pImageBuffer;
            return IBSU_ERR_COMMAND_FAILED;
        }

        // Write BITMAPINFO structure
        // normally images are stored bottom up
        pBmpInfo->bmiHeader.biWidth  = width;
        pBmpInfo->bmiHeader.biHeight = (pitch > 0 ? -1 : 1) * height;
        pBmpInfo->bmiHeader.biXPelsPerMeter = LONG(resX * 10000 / 254);
        pBmpInfo->bmiHeader.biYPelsPerMeter = LONG(resY * 10000 / 254);
        bytesToWrite = sizeof(IB_BITMAPINFOHEADER) + 256 * sizeof(IB_RGBQUAD);
        bytesWritten = fwrite((char *)pBmpInfo, 1, bytesToWrite, hFile);
        if (bytesWritten != bytesToWrite)
        {
            if (hFile)
            {
                fclose(hFile);
            }
            delete [] pBmpInfo;
            delete [] pImageBuffer;
            return IBSU_ERR_COMMAND_FAILED;
        }

        // Write image
        BYTE *ptr = const_cast<BYTE *>(pImageBuffer);
        bytesToWrite = width;
        for (DWORD y = 0; y < height; y++)
        {
            bytesWritten = fwrite((char *)ptr, 1, bytesToWrite, hFile);
            if (bytesWritten != bytesToWrite)
            {
                if (hFile)
                {
                    fclose(hFile);
                }
                delete [] pBmpInfo;
                delete [] pImageBuffer;
                return IBSU_ERR_COMMAND_FAILED;
            }
            ptr += abs(pitch);
        }
    }
    catch (...)
    {
        if (hFile)
        {
            fclose(hFile);
        }
        delete [] pBmpInfo;
        delete [] pImageBuffer;
        return IBSU_ERR_COMMAND_FAILED;
    }

    fclose(hFile);
    delete [] pBmpInfo;
    delete [] pImageBuffer;

    return IBSU_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
#if defined(_WINDOWS) && !defined(WINCE)
BOOL CIBScanUltimateApp::_GetEnumeratedIndexFromSortedIndex(DWORD sortedIndex, DWORD *enumeratedIndex, int *pVid, int *pPid)
{
    LPGUID pGuid = (LPGUID)&IBSCAN_DEVICE_INTERFACE_GUID;
    DWORD nDevices = 0;
    DWORD nIndex = 0;

    BOOL bResult = TRUE;
    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = NULL;
    SP_DEVINFO_DATA DeviceInfoData;
    ULONG requiredLength = 0;
    LPTSTR lpDevicePath = NULL;
    int vid = 0;
    int pid = 0;

    HDEVINFO hdInfo = SetupDiGetClassDevs(pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hdInfo == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);


    for (nDevices = 0; ; nDevices++)
    {
        SP_INTERFACE_DEVICE_DATA ifData;
        ifData.cbSize = sizeof(ifData);

        /*##############################################*/
        SetupDiEnumDeviceInfo(hdInfo, nDevices, &DeviceInfoData);

        //Reset for this iteration
        if (lpDevicePath)
        {
            LocalFree(lpDevicePath);
            lpDevicePath = NULL;
        }
        if (pInterfaceDetailData)
        {
            LocalFree(pInterfaceDetailData);
            pInterfaceDetailData = NULL;
        }


        deviceInterfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

        bResult = SetupDiEnumDeviceInterfaces(hdInfo, &DeviceInfoData, pGuid, 0, &deviceInterfaceData);
        // Check if last item
        if (GetLastError() == ERROR_NO_MORE_ITEMS)
        {
            break;
        }

        //Check for some other error
        if (!bResult)
        {
            goto done;
        }

        bResult = SetupDiGetDeviceInterfaceDetail(hdInfo, &deviceInterfaceData, NULL, 0, &requiredLength, NULL);
        //Check for some other error
        if (!bResult)
        {
            if ((ERROR_INSUFFICIENT_BUFFER == GetLastError()) && (requiredLength > 0))
            {
                //we got the size, allocate buffer
                pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, requiredLength);

                if (!pInterfaceDetailData)
                {
                    goto done;
                }
            }
            else
            {
                goto done;
            }
        }

        pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        bResult = SetupDiGetDeviceInterfaceDetail(hdInfo, &deviceInterfaceData, pInterfaceDetailData, requiredLength, NULL, &DeviceInfoData);
        //Check for some other error
        if (!bResult)
        {
            goto done;
        }

        size_t nLength = _tcslen((TCHAR *)pInterfaceDetailData->DevicePath) + 1;
        lpDevicePath = (TCHAR *) LocalAlloc(LPTR, nLength * sizeof(TCHAR));
        _tcsncpy(lpDevicePath, pInterfaceDetailData->DevicePath, nLength);

        lpDevicePath[nLength - 1] = 0;

        // enzyme modify 2012-10-27 I don't know why different "Vid/Pid vs vid/pid
        //		TCHAR * vidLoc = _tcsstr(lpDevicePath, _T("vid_"));
        //		TCHAR * pidLoc = _tcsstr(lpDevicePath, _T("pid_"));
        TCHAR *vidLoc = _tcsstr(_tcsupr(lpDevicePath), _T("VID_"));
        TCHAR *pidLoc = _tcsstr(_tcsupr(lpDevicePath), _T("PID_"));
        if (vidLoc)
        {
            TCHAR *endChar;
            vid = _tcstol(vidLoc + 4, &endChar, 16);
        }

        if (pidLoc)
        {
            TCHAR *endChar;
            pid = _tcstol(pidLoc + 4, &endChar, 16);
        }

        if (vid == __VID_IB__ || vid == __VID_DERMALOG__)
        {
            if (_FindSupportedDevices(pid))
            {
                if (!SetupDiEnumInterfaceDevice(hdInfo, NULL, pGuid, nDevices, &ifData))
                {
                    if (GetLastError() == ERROR_NO_MORE_ITEMS)
                    {
                        break;
                    }
                    else
                    {
                        goto done;
                    }
                }

                if (nIndex++ == sortedIndex)
                {
                    LocalFree(lpDevicePath);
                    LocalFree(pInterfaceDetailData);
                    *enumeratedIndex = nDevices;
                    SetupDiDestroyDeviceInfoList(hdInfo);
                    *pVid = vid;
                    *pPid = pid;
                    return TRUE;
                }
            }
            else
            {
                if (!SetupDiEnumInterfaceDevice(hdInfo, NULL, pGuid, nDevices, &ifData))
                {
                    break;
                }
                continue;
            }
        }
        else
        {
            // enzyme delete 2012-10-27 Remove bug code when we use other Generic MS device
            // Bug fixed on 0.16.1
            //			break;
            if (!SetupDiEnumInterfaceDevice(hdInfo, NULL, pGuid, nDevices, &ifData))
            {
                break;
            }
            continue;
        }
    }

    if (lpDevicePath)
    {
        LocalFree(lpDevicePath);
    }
    if (pInterfaceDetailData)
    {
        LocalFree(pInterfaceDetailData);
    }
    SetupDiDestroyDeviceInfoList(hdInfo);

    return FALSE;

done:
    LocalFree(lpDevicePath);
    LocalFree(pInterfaceDetailData);
    SetupDiDestroyDeviceInfoList(hdInfo);

    return FALSE;
}

BOOL CIBScanUltimateApp::_GetDevicePathByIndex(int devIndex, char *outNameBuf)
{
    LPGUID pGuid = (LPGUID)&IBSCAN_DEVICE_INTERFACE_GUID;

    // Find enumerated index from sorted index
    DWORD enumeratedIndex;
    int vid, pid;
    enumeratedIndex = devIndex;
    if (_GetEnumeratedIndexFromSortedIndex(devIndex, &enumeratedIndex, &vid, &pid) == FALSE)
    {
        return FALSE;
    }

    // see if there are of these attached to the system
    HDEVINFO hdInfo = SetupDiGetClassDevs(pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hdInfo == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    SP_INTERFACE_DEVICE_DATA ifData;
    ifData.cbSize = sizeof(ifData);

    // see if the device with the corresponding DeviceNumber is present
    if (!SetupDiEnumInterfaceDevice(hdInfo, NULL, pGuid, enumeratedIndex, &ifData))
    {
        SetupDiDestroyDeviceInfoList(hdInfo);
        return FALSE;
    }

    DWORD dwNameLength = 0;

    // find out how many bytes to malloc for the DeviceName.
    SetupDiGetInterfaceDeviceDetail(hdInfo, &ifData, NULL, 0, &dwNameLength, NULL);

    // we need to account for the pipe name so add to the length here
    dwNameLength += 32;

    PSP_INTERFACE_DEVICE_DETAIL_DATA detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(dwNameLength);

    if (!detail)
    {
        SetupDiDestroyDeviceInfoList(hdInfo);
        return FALSE;
    }

    detail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

    // get the DeviceName
    if (!SetupDiGetInterfaceDeviceDetail(hdInfo, &ifData, detail, dwNameLength, NULL, NULL))
    {
        //		NOISY(("Failed to OpenDeviceHandle\n", GetLastError()));
        free((PVOID)detail);
        SetupDiDestroyDeviceInfoList(hdInfo);
        return FALSE;
    }

    CString szDevId = detail->DevicePath + 4;
    int idx = szDevId.ReverseFind(_T('#'));
    ASSERT(-1 != idx);
    szDevId.Truncate(idx);
    szDevId.Replace(_T('#'), _T('\\'));
    szDevId.MakeUpper();

    char szDeviceName[MAX_PATH];
#ifdef UNICODE
	int nBufferLength = wcstombs( NULL, szDevId.GetBuffer(0), MAX_PATH );
	wcstombs(szDeviceName, szDevId.GetBuffer(0), nBufferLength+1);
	*(szDeviceName+MAX_PATH-1) = '\0'; // Ensure string is always null terminated
#else
	strcpy(szDeviceName, szDevId.GetBuffer(0));
#endif
	strcpy(outNameBuf, szDeviceName);
    //	strncpy(szDeviceName, detail->DevicePath, sizeof(szDeviceName));
    //	strcpy(outNameBuf, szDeviceName+8);

    free((PVOID) detail);
    SetupDiDestroyDeviceInfoList(hdInfo);

    // return handle
    return TRUE;
}

BOOL CIBScanUltimateApp::_GetProductNameFromEnumeration(int devIndex, char *outNameBuf)
{
    CThreadSync Sync;

    LPGUID pGuid = (LPGUID)&IBSCAN_DEVICE_INTERFACE_GUID;

    // Find enumerated index from sorted index
    DWORD enumeratedIndex;
    int vid, pid;
    enumeratedIndex = devIndex;
    if (_GetEnumeratedIndexFromSortedIndex(devIndex, &enumeratedIndex, &vid, &pid) == FALSE)
    {
        return FALSE;
    }

    // see if there are of these attached to the system
    HDEVINFO hdInfo = SetupDiGetClassDevs(pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hdInfo == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    SP_INTERFACE_DEVICE_DATA ifData;
    ifData.cbSize = sizeof(ifData);

    // see if the device with the corresponding DeviceNumber is present
    if (!SetupDiEnumInterfaceDevice(hdInfo, NULL, pGuid, enumeratedIndex, &ifData))
    {
        SetupDiDestroyDeviceInfoList(hdInfo);
        return FALSE;
    }

    DWORD dwNameLength = 0;

    // find out how many bytes to malloc for the DeviceName.
    SetupDiGetInterfaceDeviceDetail(hdInfo, &ifData, NULL, 0, &dwNameLength, NULL);

    // we need to account for the pipe name so add to the length here
    dwNameLength += 32;

    PSP_INTERFACE_DEVICE_DETAIL_DATA detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(dwNameLength);

    if (!detail)
    {
        SetupDiDestroyDeviceInfoList(hdInfo);
        return FALSE;
    }

    detail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

    // get the DeviceName
    if (!SetupDiGetInterfaceDeviceDetail(hdInfo, &ifData, detail, dwNameLength, NULL, NULL))
    {
        //		NOISY(("Failed to OpenDeviceHandle\n", GetLastError()));
        free((PVOID)detail);
        SetupDiDestroyDeviceInfoList(hdInfo);
        return FALSE;
    }

    size_t nLength = _tcslen((TCHAR *)detail->DevicePath) + 1;
    LPTSTR lpDevicePath = NULL;
    lpDevicePath = (TCHAR *) LocalAlloc(LPTR, nLength * sizeof(TCHAR));
    _tcsncpy(lpDevicePath, detail->DevicePath, nLength);

    lpDevicePath[nLength - 1] = 0;

    free((PVOID) detail);
    SetupDiDestroyDeviceInfoList(hdInfo);

    // enzyme modify 2012-10-27 I don't know why different "Vid/Pid vs vid/pid
    //	TCHAR * vidLoc = _tcsstr(lpDevicePath, _T("vid_"));
    //	TCHAR * pidLoc = _tcsstr(lpDevicePath, _T("pid_"));
    TCHAR *vidLoc = _tcsstr(_tcsupr(lpDevicePath), _T("VID_"));
    TCHAR *pidLoc = _tcsstr(_tcsupr(lpDevicePath), _T("PID_"));
    //	int pid, vid;
    if (vidLoc)
    {
        TCHAR *endChar;
        vid = _tcstol(vidLoc + 4, &endChar, 16);
    }

    if (pidLoc)
    {
        TCHAR *endChar;
        pid = _tcstol(pidLoc + 4, &endChar, 16);
    }
    LocalFree(lpDevicePath);

    if (vid == __VID_IB__ || vid == __VID_DERMALOG__)
    {
        if (_FindSupportedDevices(pid))
        {
            if (_GetDeviceNameFromPID(pid, outNameBuf))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

#elif defined(WINCE)
BOOL CIBScanUltimateApp::_GetDevicePathByIndex(int devIndex, char *outNameBuf)
{
    CThreadSync Sync;

	libusb_device **dev_list = NULL;
	struct libusb_device_descriptor dev_descriptor;
	int dev_list_len = 0;
	int i;
	int r;
	int nDevices = 0;
	BOOL ret = FALSE;

	r = DelayedLibUSBInit();
	if( r != IBSU_STATUS_OK )
	{
		DEBUGMSG(1, (TEXT("DelayedLibUSBInit failed\r\n")));
		return FALSE;
	}

	r = libusb_get_device_list(m_LibUSBContext, &dev_list);
	if (r < 0) {
//		usbi_err("get_device_list failed with error %d", r);
		return FALSE;
	}

	/* Iterate over the device list, finding the desired device */
	dev_list_len = r;
	for (i = 0; i < dev_list_len; i++) {
		libusb_device *dev = dev_list[i];
		libusb_get_device_descriptor(dev,&dev_descriptor);

		if( dev_descriptor.idVendor == __VID_IB__ ||
			dev_descriptor.idVendor == __VID_DERMALOG__)
		{
            if (_FindSupportedDevices(dev_descriptor.idProduct))
            {
                if (nDevices == devIndex)
                {
					uint8_t bus_num = libusb_get_bus_number(dev);
					uint8_t devnum = libusb_get_device_address(dev);

                    sprintf(outNameBuf, "ceusbkwrapper_%03d_%03d", bus_num, devnum);
					ret = TRUE;
                    break;
                }			
				nDevices++;
            }
		}	
	}

	libusb_free_device_list(dev_list, 1);
	return ret;

}

BOOL CIBScanUltimateApp::_GetProductNameFromEnumeration(int devIndex, char *outNameBuf)
{
    CThreadSync Sync;

	libusb_device **dev_list = NULL;
	struct libusb_device_descriptor dev_descriptor;
	int dev_list_len = 0;
	int i;
	int r;
	int nDevices = 0;
	BOOL ret = FALSE;

	r = DelayedLibUSBInit();
	if( r != IBSU_STATUS_OK )
	{
		DEBUGMSG(1, (TEXT("DelayedLibUSBInit failed\r\n")));
		return FALSE;
	}

	r = libusb_get_device_list(m_LibUSBContext, &dev_list);
	if (r < 0) {
//		usbi_err("get_device_list failed with error %d", r);
		return FALSE;
	}

	/* Iterate over the device list, finding the desired device */
	dev_list_len = r;
	for (i = 0; i < dev_list_len; i++) {
		libusb_device *dev = dev_list[i];
		libusb_get_device_descriptor(dev,&dev_descriptor);

		if( dev_descriptor.idVendor == __VID_IB__ ||
			dev_descriptor.idVendor == __VID_DERMALOG__)
		{
            if (_FindSupportedDevices(dev_descriptor.idProduct))
            {
                if (nDevices == devIndex)
                {
                    if (_GetDeviceNameFromPID(dev_descriptor.idProduct, outNameBuf))
                    {
						ret = TRUE;
						break;
					}
                }			
				nDevices++;
            }
		}	
	}

	libusb_free_device_list(dev_list, 1);
	return ret;

}
#else
#ifdef __libusb_latest__
BOOL CIBScanUltimateApp::_GetDevicePathByIndex(int devIndex, char *outNameBuf)
{
    CThreadSync Sync;

	libusb_device **dev_list = NULL;
	struct libusb_device_descriptor dev_descriptor;
	int dev_list_len = 0;
	int i;
	int r;
	int nDevices = 0;
	BOOL ret = FALSE;

	r = DelayedLibUSBInit();
	if (r != IBSU_STATUS_OK)
	{
		return FALSE;
	}

	r = libusb_get_device_list(m_LibUSBContext, &dev_list);
	if (r < 0)
    {
		return FALSE;
	}

	/* Iterate over the device list, finding the desired device */
	dev_list_len = r;
	for (i = 0; i < dev_list_len; i++)
    {
		libusb_device *dev = dev_list[i];
		libusb_get_device_descriptor(dev,&dev_descriptor);

		if( dev_descriptor.idVendor == __VID_IB__ ||
			dev_descriptor.idVendor == __VID_DERMALOG__)
		{
            if (_FindSupportedDevices(dev_descriptor.idProduct))
            {
                if (nDevices == devIndex)
                {
					uint8_t bus_num = libusb_get_bus_number(dev);
					uint8_t devnum = libusb_get_device_address(dev);

                    sprintf(outNameBuf, "%d_%d", bus_num, devnum);
					ret = TRUE;
                    break;
                }			
				nDevices++;
            }
		}	
	}
	libusb_free_device_list(dev_list, 1);

    return ret;
}

BOOL CIBScanUltimateApp::_GetProductNameFromEnumeration(int devIndex, char *outNameBuf)
{
    CThreadSync Sync;

	libusb_device **dev_list = NULL;
	struct libusb_device_descriptor dev_descriptor;
	int dev_list_len = 0;
	int i;
	int r;
	int nDevices = 0;
	BOOL ret = FALSE;

	r = DelayedLibUSBInit();
	if( r != IBSU_STATUS_OK )
	{
		return FALSE;
	}

	r = libusb_get_device_list(m_LibUSBContext, &dev_list);
	if (r < 0) {
//		usbi_err("get_device_list failed with error %d", r);
		return FALSE;
	}

	/* Iterate over the device list, finding the desired device */
	dev_list_len = r;
	for (i = 0; i < dev_list_len; i++) {
		libusb_device *dev = dev_list[i];
		libusb_get_device_descriptor(dev,&dev_descriptor);

		if( dev_descriptor.idVendor == __VID_IB__ ||
			dev_descriptor.idVendor == __VID_DERMALOG__)
		{
            if (_FindSupportedDevices(dev_descriptor.idProduct))
            {
                if (nDevices == devIndex)
                {
                    if (_GetDeviceNameFromPID(dev_descriptor.idProduct, outNameBuf))
                    {
						ret = TRUE;
						break;
					}
                }			
				nDevices++;
            }
		}	
	}

	libusb_free_device_list(dev_list, 1);
	return ret;
}
#else
BOOL CIBScanUltimateApp::_GetDevicePathByIndex(int devIndex, char *outNameBuf)
{
    CThreadSync Sync;

    struct usb_bus *bus;
    struct usb_device *dev;

    int  nDevices = 0;

    usb_find_busses();
    usb_find_devices();
    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == __VID_IB__ ||
				dev->descriptor.idVendor == __VID_DERMALOG__)
            {
                if (_FindSupportedDevices(dev->descriptor.idProduct))
                {
                    if (nDevices == devIndex)
                    {
                        sprintf(outNameBuf, "%s_%d", bus->dirname, dev->devnum);
                        return TRUE;
                    }

                    nDevices++;
                }
            }
        }
    }

    return FALSE;
}

BOOL CIBScanUltimateApp::_GetProductNameFromEnumeration(int devIndex, char *outNameBuf)
{
    CThreadSync Sync;

    struct usb_bus *bus;
    struct usb_device *dev;

    int  nDevices = 0;

    usb_find_busses();
    usb_find_devices();
    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == __VID_IB__ ||
				dev->descriptor.idVendor == __VID_DERMALOG__)
            {
                if (_FindSupportedDevices(dev->descriptor.idProduct))
                {
                    if (nDevices == devIndex)
                    {
                        if (_GetDeviceNameFromPID(dev->descriptor.idProduct, outNameBuf))
                        {
                            return TRUE;
                        }
                    }

                    nDevices++;
                }
            }
        }
    }

    return FALSE;
}
#endif

#endif

int CIBScanUltimateApp::_IsValidUsbHandle(CIBUsbManager *pIBUsbManager)
{
    CThreadSync Sync;
    int				nRc = IBSU_STATUS_OK;

    if (pIBUsbManager && pIBUsbManager->m_bIsCommunicationBreak)
    {
        nRc = IBSU_ERR_DEVICE_INVALID_STATE;
    }
    else if (pIBUsbManager == NULL ||
             pIBUsbManager->m_pMainCapture == NULL ||
             pIBUsbManager->m_pMainCapture->IsInitialized() == FALSE)
    {
        nRc = IBSU_ERR_DEVICE_NOT_INITIALIZED;
    }

    return nRc;
}

void CIBScanUltimateApp::_RunWindowsMessageThread()
{
#if defined(_WINDOWS)
    if (!m_hWindowsMessageThread)
    {
        // Create thread to create Windows Message Procedure
        ThreadParam param;

        param.pParentApp = &theApp;
        param.threadStarted = CreateEvent(NULL, FALSE, FALSE, NULL);

        // Create control event for WindowsMessageThread
        m_hWindowsMessageThreadDestroyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        m_hWindowsMessageThread = ::CreateThread(NULL, 0, ::WindowsMessageThreadCallback, &param, 0, NULL);
        if (m_hWindowsMessageThread)
        {
            WaitForSingleObject(param.threadStarted, INFINITE);
        }
        CloseHandle(param.threadStarted);
    }
#endif
}

void CIBScanUltimateApp::_RunDeviceCountThread()
{
    if (!m_hDeviceCountThread)
    {
        // Create thread to detect the changing of the device count
#ifdef _WINDOWS
        ThreadParam param;

        param.pParentApp = &theApp;
        param.threadStarted = CreateEvent(NULL, FALSE, FALSE, NULL);

        // Create control event for DeviceCountThread
        m_hDeviceCountThreadDestroyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        m_hDeviceCountThreadArrivalEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        m_hDeviceCountThreadRemoveEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hDeviceCountThreadPaintEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        m_hDeviceCountThreadPowerSleepEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        m_hDeviceCountThreadPowerResumeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        // Initialize the event objects
        m_hDeviceCountEventArray[0] = m_hDeviceCountThreadDestroyEvent;
        m_hDeviceCountEventArray[1] = m_hDeviceCountThreadArrivalEvent;
        m_hDeviceCountEventArray[2] = m_hDeviceCountThreadRemoveEvent;
		m_hDeviceCountEventArray[3] = m_hDeviceCountThreadPaintEvent;
        m_hDeviceCountEventArray[4] = m_hDeviceCountThreadPowerResumeEvent;
        m_hDeviceCountEventArray[5] = m_hDeviceCountThreadPowerSleepEvent;

        m_hDeviceCountThread = ::CreateThread(NULL, 0, ::DeviceCountThreadCallback, &param, 0, NULL);
        if (m_hDeviceCountThread)
        {
            WaitForSingleObject(param.threadStarted, INFINITE);
        }
        CloseHandle(param.threadStarted);
#elif defined(__linux__)
        ThreadParam param;

        param.pParentApp = &theApp;

        pthread_attr_t threadAttr;

        // initialize the thread attribute
        pthread_attr_init(&threadAttr);

        // Set the stack size of the thread
        pthread_attr_setstacksize(&threadAttr, 240 * 1024);

        // Set thread to detached state. No need for pthread_join
        pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
        /*
#ifdef __android__
        		pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
#else
        		pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
#endif
        */
        param.threadStarted = false;
        // Create the thread
        int threadRC = pthread_create(&m_hDeviceCountThread, &threadAttr,
                                      ::DeviceCountThreadCallback, &param);

        // Destroy the thread attributes
        pthread_attr_destroy(&threadAttr);

        if (threadRC != 0)
        {
            //			In production code, you MUST handle failures, and pass
            //			it on according to whatever error handling mechanisms you follow
            m_hDeviceCountThread = 0;
            return;
        }
        // Wait for the thread to initialize and get ready ...
        int tries = 0;
        do
        {
            Sleep(10);
            if (tries++ > 100)
            {
                break;
            }
        }
        while (!param.threadStarted);
#endif
    }
}

BOOL CIBScanUltimateApp::_FindSupportedDevices(int pid)
{
    for (int i = 0; i < __MAX_SUPPORTED_DEVICES_COUNT__; i++)
    {
        if (__G_SUPPORTED_DEVICES__[i] == pid)
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL CIBScanUltimateApp::_GetDeviceNameFromPID(int pid, char *outNameBuf)
{
    switch (pid)
    {
        case __PID_WATSON__:
        case __PID_WATSON_REV1__:
#if defined(__IBSCAN_SDK__)
		sprintf(outNameBuf, "IBNW11C");
#else
		sprintf(outNameBuf, "WATSON");
#endif
            break;
        case __PID_WATSON_MINI__:
        case __PID_WATSON_MINI_REV1__:
            sprintf(outNameBuf, "WATSON MINI");
            break;
        case __PID_SHERLOCK__:
        case __PID_SHERLOCK_REV1__:
            sprintf(outNameBuf, "SHERLOCK");
            break;
        case __PID_COLUMBO__:
        case __PID_COLUMBO_REV1__:
            sprintf(outNameBuf, "COLUMBO");
            break;
        case __PID_CURVE__:
            sprintf(outNameBuf, "CURVE");
            break;
        case __PID_KOJAK__:
        case __PID_KOJAK_REV1__:
            sprintf(outNameBuf, "KOJAK");
            break;
        case __PID_FIVE0__:
        case __PID_FIVE0_REV1__:
        case __PID_FIVE0_DERMALOG__:
            sprintf(outNameBuf, "FIVE-0");
            break;
        default:
            return FALSE;
    }

    return TRUE;
}

int CIBScanUltimateApp::_OpenDevice(int deviceIndex, LPCSTR uniformityMaskPath, BOOL useUniformityMask, BOOL asyncOpen, int *pHandle, BOOL bReserved)
{
    int				nRc = IBSU_STATUS_OK;
    CIBUsbManager	*pIBUsbManager = NULL;
    int				threadPoolCount = 3;
    BOOL            bHasManagerClass = FALSE;

#if defined(__embedded__)  || defined(WINCE)
    threadPoolCount = 3;
#endif

    if (m_pThreadPoolMgr == NULL)
    {
        _RunDeviceCountThread();
        _RunWindowsMessageThread();
        m_pThreadPoolMgr = new CThreadPoolMgr(threadPoolCount);
    }

    RemoveAllCommuncationBreakDevice();
    if ((pIBUsbManager = FindUsbDeviceByIndexInTempList(deviceIndex)) != NULL)
    {
        return IBSU_ERR_DEVICE_BUSY;
    }

    if ((pIBUsbManager = FindUsbDeviceByIndexInList(deviceIndex)) != NULL)
    {
        bHasManagerClass = TRUE;
        if (pIBUsbManager->m_pMainCapture != NULL)
        {
            if (pIBUsbManager->m_pMainCapture->IsInitialized())
            {
                *pHandle = pIBUsbManager->m_pMainCapture->GetDeviceHandle();
                return IBSU_WRN_ALREADY_INITIALIZED;
            }

            if (!pIBUsbManager->m_bCompletedOpenDeviceThread)
            {
                return IBSU_ERR_DEVICE_BUSY;
            }
        }
    }

    if (!bHasManagerClass)
    {
        pIBUsbManager = new CIBUsbManager();
    }

    if (pIBUsbManager->m_pMainCapture == NULL)
    {
        pIBUsbManager->m_pMainCapture = new CMainCapture(&theApp, pIBUsbManager);
    }

    if (!bHasManagerClass)
    {
        pIBUsbManager->m_nUsbIndex = deviceIndex;
        m_pTempUsbManager.push_back(pIBUsbManager);
    }

    BOOL bPPIMode = FALSE;
#if defined(__ppi__)
    if (deviceIndex < m_nPPIDeviceCount)
        bPPIMode = TRUE;
#endif

    if (bReserved)
    {
        nRc = pIBUsbManager->m_pMainCapture->Main_Initialize(deviceIndex, asyncOpen, uniformityMaskPath, useUniformityMask, TRUE, bPPIMode);
        if (nRc == IBSU_STATUS_OK)
        {
            pIBUsbManager->m_bInitialized = TRUE;
            pIBUsbManager->m_pMainCapture->SetInitialized(TRUE);
            pIBUsbManager->m_bCompletedOpenDeviceThread = TRUE;
        }
    }
    else
    {
        nRc = pIBUsbManager->m_pMainCapture->Main_Initialize(deviceIndex, asyncOpen, uniformityMaskPath, useUniformityMask, FALSE, bPPIMode);
        if (nRc == IBSU_STATUS_OK)
        {
/*			nRc = pIBUsbManager->m_pMainCapture->_IsRequiredSDKVersion(pIBUsbManager->m_pMainCapture->m_propertyInfo.sKojakCalibrationInfo.cRequiredSDK);
			if (nRc != IBSU_STATUS_OK)
			{
				goto done;
			}
*/
//            Sync.Leave();
            if (!asyncOpen)
            {
                // Make threads of initialization and capture
                nRc = pIBUsbManager->m_pMainCapture->_OpenDeviceThread();
                if (nRc == IBSU_STATUS_OK)
                {
                    pIBUsbManager->m_bInitialized = TRUE;
                    pIBUsbManager->m_pMainCapture->SetInitialized(TRUE);
                    pIBUsbManager->m_bCompletedOpenDeviceThread = TRUE;
                }
            }
            else
            {
                // Make thread of asyncronous initialization
                nRc = pIBUsbManager->m_pMainCapture->_AsyncOpenDeviceThread();
            }
//            Sync.Enter();
        }
    }

//    CThreadSync Sync;
    if (nRc == IBSU_STATUS_OK)
    {
/*#if defined(__ppi__)
        if (deviceIndex < m_nPPIDeviceCount)
        {
            if(device_open(pHandle) == 0)
		    {
			    bHasManagerClass = FALSE;
		    }
        }
        else
        {
    		*pHandle = theApp.CreateAPIHandle();
        }
#else
*/
		*pHandle = theApp.CreateAPIHandle();
//#endif
        pIBUsbManager->m_pMainCapture->SetDeviceIndex(deviceIndex);
        pIBUsbManager->m_pMainCapture->SetDeviceHandle(*pHandle);
        pIBUsbManager->m_nUsbIndex = deviceIndex;
		pIBUsbManager->m_nUsbHandle = *pHandle;
        _GetDevicePathByIndex(deviceIndex, pIBUsbManager->m_szDevicePath);

        strcpy(pIBUsbManager->m_DeviceDesc.productName, pIBUsbManager->m_pMainCapture->m_propertyInfo.cProductID);
        strcpy(pIBUsbManager->m_DeviceDesc.serialNumber, pIBUsbManager->m_pMainCapture->m_propertyInfo.cSerialNumber);
        strcpy(pIBUsbManager->m_DeviceDesc.fwVersion, pIBUsbManager->m_pMainCapture->m_propertyInfo.cFirmware);
        strcpy(pIBUsbManager->m_DeviceDesc.devRevision, pIBUsbManager->m_pMainCapture->m_propertyInfo.cDevRevision);

        strcpy(pIBUsbManager->m_pMainCapture->m_szDevicePath, pIBUsbManager->m_szDevicePath);

#if defined(_WINDOWS) && !defined(WINCE)
        if (g_pSharedData != NULL)
        {
            strcpy(&g_pSharedData->devDesc[deviceIndex].productName[0], pIBUsbManager->m_pMainCapture->m_propertyInfo.cProductID);
            strcpy(&g_pSharedData->devDesc[deviceIndex].serialNumber[0], pIBUsbManager->m_pMainCapture->m_propertyInfo.cSerialNumber);
            strcpy(&g_pSharedData->devDesc[deviceIndex].fwVersion[0], pIBUsbManager->m_pMainCapture->m_propertyInfo.cFirmware);
            strcpy(&g_pSharedData->devDesc[deviceIndex].devRevision[0], pIBUsbManager->m_pMainCapture->m_propertyInfo.cDevRevision);

            strcpy(&g_pSharedData->RES_devDesc[deviceIndex].productName[0], pIBUsbManager->m_pMainCapture->m_propertyInfo.cProductID);
            strcpy(&g_pSharedData->RES_devDesc[deviceIndex].serialNumber[0], pIBUsbManager->m_pMainCapture->m_propertyInfo.cSerialNumber);
            strcpy(&g_pSharedData->RES_devDesc[deviceIndex].fwVersion[0], pIBUsbManager->m_pMainCapture->m_propertyInfo.cFirmware);
            strcpy(&g_pSharedData->RES_devDesc[deviceIndex].devRevision[0], pIBUsbManager->m_pMainCapture->m_propertyInfo.cDevRevision);
            strcpy(&g_pSharedData->RES_devDesc[deviceIndex].vendorID[0], pIBUsbManager->m_pMainCapture->m_propertyInfo.cVendorID);
            strcpy(&g_pSharedData->RES_devDesc[deviceIndex].productionDate[0], pIBUsbManager->m_pMainCapture->m_propertyInfo.cProductionDate);
            strcpy(&g_pSharedData->RES_devDesc[deviceIndex].serviceDate[0], pIBUsbManager->m_pMainCapture->m_propertyInfo.cServiceDate);
        }
#endif
        if (!bHasManagerClass)
        {
            DeleteUsbManagerInTempList(deviceIndex);
            m_pListUsbManager.push_back(pIBUsbManager);
        }
    }
	else
	{
//done:
        if (!bHasManagerClass)
        {
            DeleteUsbDeviceInTempList(deviceIndex);
        }

        *pHandle = -1;
	}

    return nRc;
}

int CIBScanUltimateApp::_IsValidReservedKey(const char *pReservedkey)
{
    CThreadSync Sync;
    int				nRc = IBSU_STATUS_OK;

    if (strcmp(pReservedkey, RESERVED_KEY_STRING) != 0)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
    }

    return nRc;
}

int CIBScanUltimateApp::_IsValidPartnerReservedKey(const char *pPartnerReservedkey)
{
    CThreadSync Sync;
    int				nRc = IBSU_STATUS_OK;

    if (strcmp(pPartnerReservedkey, PARTNER_SET_STRING) != 0 &&
		strcmp(pPartnerReservedkey, PARTNER_SET_STRING2) != 0)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
    }

    return nRc;
}

int CIBScanUltimateApp::_GetReservedKeyAccessLevel(const char *pReservedkey, ReservedAccessLevel *accessLevel)
{
    CThreadSync Sync;
    int				nRc = IBSU_STATUS_OK;

    if (strcmp(pReservedkey, RESERVED_KEY_STRING) == 0)
    {
        *accessLevel = RESERVED_ACCESS_LEVEL_HIGH;   // High Level
    }
    else if (strcmp(pReservedkey, PARTNER_SET_STRING) == 0)
    {
        *accessLevel = RESERVED_ACCESS_LEVEL_MEDIUM;   // Medium Level
    }
    else if (strcmp(pReservedkey, PARTNER_SET_STRING2) == 0)
    {
        *accessLevel = RESERVED_ACCESS_LEVEL_LOW;   // Low Level
    }
    else
    {
        *accessLevel = RESERVED_ACCESS_NO;
        nRc = IBSU_ERR_NOT_SUPPORTED;
    }

    return nRc;
}

int CIBScanUltimateApp::_SaveBitmapMem(BYTE *InImg, BYTE *OutImg, const DWORD width, const DWORD height,
								const int pitch, const double resX, const double resY, const BYTE BitsPerPixel)
{
	CThreadSync Sync;

	IB_BITMAPINFO* pBmpInfo = NULL;
    int         rgbBit = BitsPerPixel/8;

	try
	{
		IB_BITMAPFILEHEADER      m_BmpFileHeader;

		// prepare BMP Info header:
		pBmpInfo = reinterpret_cast <IB_BITMAPINFO *>
										  (new UCHAR [sizeof (IB_BITMAPINFOHEADER) + 256 * sizeof (IB_RGBQUAD)]);

		pBmpInfo->bmiHeader.biSize          = sizeof (IB_BITMAPINFOHEADER);
		pBmpInfo->bmiHeader.biWidth         = 0;
		pBmpInfo->bmiHeader.biHeight        = 0;
		pBmpInfo->bmiHeader.biPlanes        = 1;
		pBmpInfo->bmiHeader.biBitCount      = BitsPerPixel;
		pBmpInfo->bmiHeader.biCompression   = BI_RGB;
		pBmpInfo->bmiHeader.biSizeImage     = 0;			// requires BI_RGB
		pBmpInfo->bmiHeader.biXPelsPerMeter = 0;
		pBmpInfo->bmiHeader.biYPelsPerMeter = 0;
		pBmpInfo->bmiHeader.biClrUsed       = 0;
		pBmpInfo->bmiHeader.biClrImportant  = 0;

        if( BitsPerPixel <= 8 )
        {
            // IBSU_IMG_FORMAT_GRAY
		    for(int i = 0; i <= 255; i++)
		    {
			    ASSERT (i == (BYTE) i);
			    pBmpInfo->bmiColors[i].rgbBlue     = (BYTE)i;
			    pBmpInfo->bmiColors[i].rgbGreen    = (BYTE)i;
			    pBmpInfo->bmiColors[i].rgbRed      = (BYTE)i;
			    pBmpInfo->bmiColors[i].rgbReserved = 0;
		    }
        }
  	
		// prepare BMP file header:
		m_BmpFileHeader.bfType = (unsigned short)(('M'<<8) | 'B');
		m_BmpFileHeader.bfReserved1 = 0;
		m_BmpFileHeader.bfReserved2 = 0;
        if( BitsPerPixel <= 8 )
        {
            // IBSU_IMG_FORMAT_GRAY
		    m_BmpFileHeader.bfOffBits = sizeof(IB_BITMAPFILEHEADER) + sizeof(IB_BITMAPINFOHEADER) +
									    256 * sizeof(IB_RGBQUAD);
		    m_BmpFileHeader.bfSize = sizeof(IB_BITMAPFILEHEADER) + sizeof(IB_BITMAPINFOHEADER) +
								      256 * sizeof(IB_RGBQUAD) + ( width * height );
        }
        else
        {
		    m_BmpFileHeader.bfOffBits = sizeof(IB_BITMAPFILEHEADER) + sizeof(IB_BITMAPINFOHEADER);
		    m_BmpFileHeader.bfSize = sizeof(IB_BITMAPFILEHEADER) + sizeof(IB_BITMAPINFOHEADER) +
								      ( rgbBit * width * height );
        }
    
		// Make BITMAPINFO structure
		// normally images are stored bottom up
		pBmpInfo->bmiHeader.biWidth  = width;
		pBmpInfo->bmiHeader.biHeight = ( pitch > 0 ? -1 : 1 ) * height;
		pBmpInfo->bmiHeader.biXPelsPerMeter = LONG( resX * 10000 / 254 );
		pBmpInfo->bmiHeader.biYPelsPerMeter = LONG( resY * 10000 / 254 );
      
		// Make bitmap buffer
        BYTE *ptrOutImg = OutImg;
        DWORD bytesWritten;
        bytesWritten = sizeof(IB_BITMAPFILEHEADER);
        memcpy(ptrOutImg, &m_BmpFileHeader, bytesWritten);
        ptrOutImg += bytesWritten;

        switch( BitsPerPixel )
        {
        default:
        	break;
        case 8:
            bytesWritten = sizeof(IB_BITMAPINFOHEADER) + 256 * sizeof(IB_RGBQUAD);
            memcpy(ptrOutImg, pBmpInfo, bytesWritten);
            ptrOutImg += bytesWritten;
            memcpy(ptrOutImg, InImg, width*height);
            break;
        case 24:
            bytesWritten = sizeof(IB_BITMAPINFOHEADER);
            memcpy(ptrOutImg, pBmpInfo, bytesWritten);
            ptrOutImg += bytesWritten;
            for( DWORD y = 0; y < height; y++ )
            {
                for( DWORD x = 0; x < width; x++ )
                {
		            ptrOutImg[(y*width+x)*3] =
			        ptrOutImg[(y*width+x)*3+1] =
			        ptrOutImg[(y*width+x)*3+2] = InImg[y*width+x];
                }
            }
            break;
        case 32:
            bytesWritten = sizeof(IB_BITMAPINFOHEADER);
            memcpy(ptrOutImg, pBmpInfo, bytesWritten);
            ptrOutImg += bytesWritten;
            for( DWORD y = 0; y < height; y++ )
            {
                for( DWORD x = 0; x < width; x++ )
                {
		            ptrOutImg[(y*width+x)*4] =
			        ptrOutImg[(y*width+x)*4+1] =
			        ptrOutImg[(y*width+x)*4+2] = InImg[y*width+x];
		            ptrOutImg[(y*width+x)*4+3] = -1;//0xff, that's the alpha. 
                }
            }
            break;
        }
	}
	catch(...)
	{
		delete [] pBmpInfo;
		return IBSU_ERR_COMMAND_FAILED;	
	}

	delete [] pBmpInfo;

	return IBSU_STATUS_OK;
}

int CIBScanUltimateApp::Main_SaveBitmapMem(const IBSU_ImageData inImage, BYTE *outBitmapBuffer,
                                           const int outWidth, const int outHeight, const BYTE bkColor, 
                                           int factor)
{
	int     nRc = IBSU_STATUS_OK;
    BYTE    *pZoomoutBuffer = NULL;
    double  resX, resY;

    pZoomoutBuffer = new BYTE[outWidth*outHeight];

    nRc = Main_GenerateZoomOutImage(inImage, pZoomoutBuffer, outWidth, outHeight, bkColor);
    if( nRc != IBSU_STATUS_OK )
    {
        delete [] pZoomoutBuffer;
        return nRc;
    }

    resX = (double)inImage.ResolutionX*inImage.Width/outWidth;
    resY = (double)inImage.ResolutionY*inImage.Height/outHeight;
    nRc = _SaveBitmapMem(pZoomoutBuffer, outBitmapBuffer, outWidth, outHeight, inImage.Pitch, resX, resY, factor*8);

    delete [] pZoomoutBuffer;

    return nRc;
}

int CIBScanUltimateApp::_RemoveAllVector_UsbManager()
{
	int nRc = IBSU_STATUS_OK;

	std::vector<CIBUsbManager *>::iterator it = m_pListUsbManager.begin();
	while( it != m_pListUsbManager.end() )
	{
		if( *it )
		{
			if( (*it)->m_pMainCapture && !(*it)->m_bIsCommunicationBreak )
			{
//				if( (*it)->m_pMainCapture->Main_Release(TRUE) == IBSU_ERR_RESOURCE_LOCKED )
				if( (*it)->_Delete_MainCapture(TRUE, FALSE) == IBSU_ERR_RESOURCE_LOCKED )
				{
					nRc = IBSU_ERR_RESOURCE_LOCKED;
					++it;
					continue;
				}
				else
				{
					delete (*it)->m_pMainCapture;
					(*it)->m_pMainCapture = NULL;
				}
			}
			delete *it;
			it = m_pListUsbManager.erase(it);
		}
		else
			++it;
	}

	if( nRc != IBSU_ERR_RESOURCE_LOCKED )
		m_pListUsbManager.clear();

	return nRc;
}

void CIBScanUltimateApp::_RemoveAllVector_WorkerJob()
{
	std::vector<CJob *>::iterator it = m_pListWorkerJob.begin();
	while( it != m_pListWorkerJob.end() )
	{
		if( *it )
		{
			delete *it;
			it = m_pListWorkerJob.erase(it);
		}
		else
			++it;
	}
	m_pListWorkerJob.clear();
}

void CIBScanUltimateApp::_RemoveWorkerJobInList(CJob *pJob)
{
	CThreadSync Sync;

	std::vector<CJob *>::iterator it = m_pListWorkerJob.begin();
	while( it != m_pListWorkerJob.end() )
	{
		if( *it && (*it) == pJob )
		{
//			delete *it;
			it = m_pListWorkerJob.erase(it);
		}
		else
			++it;
	}
}

void CIBScanUltimateApp::_ImageFlipVertically(unsigned char *pixels_buffer, const int width, const int height)
{
	const int rows = height / 2;		// Iterate only half the buffer to get a full flip
	const int row_stride = width;
    unsigned char* temp_row = (unsigned char*)malloc(row_stride);

    int source_offset, target_offset;

    for (int rowIndex = 0; rowIndex < rows; rowIndex++)
    {
        source_offset = rowIndex * row_stride;
        target_offset = (height - rowIndex - 1) * row_stride;

        memcpy(temp_row, pixels_buffer + source_offset, row_stride);
        memcpy(pixels_buffer + source_offset, pixels_buffer + target_offset, row_stride);
        memcpy(pixels_buffer + target_offset, temp_row, row_stride);
    }

    free(temp_row);
    temp_row = NULL;
}









//////////////////////////////////////////////////////////////////////////////////
#ifndef WINCE
int CIBScanUltimateApp::DLL_GetSDKVersion(
    IBSU_SdkVersion *pVerInfo                       ///< [out] API version information \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    int		nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetSDKVersion ()");

    if (::IsBadReadPtr(pVerInfo, sizeof(IBSU_SdkVersion)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetSDKVersion ()");
        return nRc;
    }

    if (nRc == IBSU_STATUS_OK)
    {
#ifdef _WINDOWS
        CFileVersionInfo verInfo;
        memset(pVerInfo, 0, sizeof(IBSU_SdkVersion));
        if (verInfo.Open(m_hInstance))
        {
            sprintf(pVerInfo->File, "%d.%d.%d.%d", verInfo.GetFileVersionMajor(),
                    verInfo.GetFileVersionMinor(),
                    verInfo.GetFileVersionBuild(),
                    verInfo.GetFileVersionQFE());

            sprintf(pVerInfo->Product, "%d.%d.%d.%d", verInfo.GetProductVersionMajor(),
                    verInfo.GetProductVersionMinor(),
                    verInfo.GetProductVersionBuild(),
                    verInfo.GetProductVersionQFE());
        }
        else
        {
            nRc = IBSU_ERR_NOT_SUPPORTED;
        }
#elif defined(__linux__)
        strcpy(pVerInfo->File, pszLinuxFileVersion);
        strcpy(pVerInfo->Product, pszLinuxProdutVersion);
#endif
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetSDKVersion (pVerInfo->Product=%s, pVerInfo->File=%s)", pVerInfo->Product, pVerInfo->File);

    return nRc;
}

#else
int CIBScanUltimateApp::DLL_GetSDKVersionW(
    IBSU_SdkVersionW *pVerInfo                       ///< [out] API version information \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    int		nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetSDKVersionW ()");

    if (::IsBadReadPtr(pVerInfo, sizeof(IBSU_SdkVersionW)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetSDKVersionW ()");
        return nRc;
    }

    if (nRc == IBSU_STATUS_OK)
    {
#ifdef _WINDOWS
        CFileVersionInfo verInfo;
        memset(pVerInfo, 0, sizeof(IBSU_SdkVersionW));
        if (verInfo.Open(m_hInstance))
        {
            wsprintfW(pVerInfo->File, L"%d.%d.%d.%d", verInfo.GetFileVersionMajor(),
                    verInfo.GetFileVersionMinor(),
                    verInfo.GetFileVersionBuild(),
                    verInfo.GetFileVersionQFE());

            wsprintfW(pVerInfo->Product, L"%d.%d.%d.%d", verInfo.GetProductVersionMajor(),
                    verInfo.GetProductVersionMinor(),
                    verInfo.GetProductVersionBuild(),
                    verInfo.GetProductVersionQFE());
        }
        else
        {
            nRc = IBSU_ERR_NOT_SUPPORTED;
        }
#elif defined(__linux__)
		// Convert Multibyte to Unicode
		wchar_t wcsLinuxFileVersion[IBSU_MAX_STR_LEN];
		wchar_t wcsLinuxProductVersion[IBSU_MAX_STR_LEN];

		mbstowcs( wcsLinuxFileVersion, pszLinuxFileVersion, IBSU_MAX_STR_LEN );
		*(wcsLinuxFileVersion+IBSU_MAX_STR_LEN-1) = L'\0'; // Ensure string is always null terminated
		mbstowcs( wcsLinuxProductVersion, pszLinuxProdutVersion, IBSU_MAX_STR_LEN );
		*(wcsLinuxProductVersion+IBSU_MAX_STR_LEN-1) = L'\0'; // Ensure string is always null terminated

		wcscpy(pVerInfo->File, wcsLinuxFileVersion);
        wcscpy(pVerInfo->Product, wcsLinuxProductVersion);
#endif
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetSDKVersionW (pVerInfo->Product=%s, pVerInfo->File=%s)", pVerInfo->Product, pVerInfo->File);

    return nRc;
}
#endif

int CIBScanUltimateApp::DLL_GetDeviceCount(
    int *pDeviceCount                               ///< [out] Number of connected devices \n
    ///  Memory must be provided by caller
)
{
//	CThreadSync Sync;

    int		nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetDeviceCount ()");

    if (::IsBadReadPtr(pDeviceCount, sizeof(int)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetDeviceCount ()");
        return nRc;
    }

    *pDeviceCount = 0;
    if (nRc == IBSU_STATUS_OK)
    {
        FindAllDeviceCount();
        *pDeviceCount = GetDeviceCount();
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetDeviceCount (pDeviceCount=%d)", *pDeviceCount);

    return nRc;
}

int CIBScanUltimateApp::DLL_GetDeviceDescription(
    const int          deviceIndex,                   ///< [in] Zero-based device index for device to lookup
    IBSU_DeviceDescA   *pDeviceDesc                   ///< [out] Basic device description \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    int				nRc = IBSU_STATUS_OK;
    CIBUsbManager *pIBUsbManager = NULL;
    CMainCapture	*pMainCapture = NULL;
    IBSU_DeviceDescA devDesc;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetDeviceDescription (deviceIndex=%d)", deviceIndex);

    if (::IsBadReadPtr(pDeviceDesc, sizeof(IBSU_DeviceDescA)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetDeviceDescription (deviceIndex=%d)", deviceIndex);
        return nRc;
    }

    memset(pDeviceDesc, 0, sizeof(IBSU_DeviceDescA));
    pDeviceDesc->handle = -1;
    memset(&devDesc, 0, sizeof(IBSU_DeviceDescA));

    FindAllDeviceCount();
    if (GetDeviceCount() <= deviceIndex)
    {
        nRc = IBSU_ERR_DEVICE_NOT_FOUND;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetDeviceDescription (deviceIndex=%d)", deviceIndex);
        return nRc;
    }

    if ((pIBUsbManager = FindUsbDeviceByIndexInList(deviceIndex)) == NULL ||
            pIBUsbManager->m_pMainCapture == NULL)
    {
        if (pIBUsbManager == NULL)
        {
            pIBUsbManager = new CIBUsbManager();
        }
        pMainCapture = new CMainCapture(&theApp, pIBUsbManager);

    BOOL bPPIMode = FALSE;
#if defined(__ppi__)
    if (deviceIndex < m_nPPIDeviceCount)
        bPPIMode = TRUE;
#endif
        nRc = pMainCapture->Main_GetDeviceInfo(deviceIndex, &devDesc, bPPIMode);
        Sleep(10);
        delete(CMainCapture *)pMainCapture;
        if (pIBUsbManager)
        {
            delete(CIBUsbManager *)pIBUsbManager;
        }

        // [IBSU_WRN_CHANNEL_IO_SLEEP_STATUS] Communction failed because status of USB chip is sleep mode
        // [IBSU_ERR_DEVICE_ACTIVE] Initialization failed because in use by another thread/process
        if (nRc == IBSU_WRN_CHANNEL_IO_SLEEP_STATUS || nRc == IBSU_ERR_DEVICE_ACTIVE)
        {
            if (!_GetProductNameFromEnumeration(deviceIndex, devDesc.productName))
            {
                return IBSU_ERR_CHANNEL_IO_COMMAND_FAILED;
            }
#if defined(_WINDOWS) && !defined(WINCE)
            if (g_pSharedData != NULL && strcmp(g_pSharedData->devDesc[deviceIndex].productName, devDesc.productName) == 0)
            {
                memcpy(&devDesc, &g_pSharedData->devDesc[deviceIndex], sizeof(IBSU_DeviceDescA));
            }
            else
            {
                sprintf(devDesc.serialNumber, "?");
                sprintf(devDesc.fwVersion, "?");
                sprintf(devDesc.devRevision, "?");
            }
#else
            sprintf(devDesc.serialNumber, "?");
            sprintf(devDesc.fwVersion, "?");
            sprintf(devDesc.devRevision, "?");
#endif
            sprintf(devDesc.interfaceType, "%s", "USB");
#if defined(__ppi__)
            if (deviceIndex >= (m_nDeviceCount - m_nPPIDeviceCount))
            {
                sprintf(devDesc.interfaceType, "%s", "PPI");
            }
#endif
            devDesc.handle = -1;
            devDesc.IsHandleOpened = FALSE;

            if (nRc == IBSU_ERR_DEVICE_ACTIVE)
            {
                nRc = IBSU_STATUS_OK;    // Remove error information to display only
            }
        }
        else
        {
#if defined(_WINDOWS) && !defined(WINCE)
            if (g_pSharedData != NULL)
            {
//                memcpy(&g_pSharedData->devDesc[deviceIndex], &devDesc, sizeof(IBSU_DeviceDescA));
            }
#endif
        }
    }
    else
    {
        //		pMainCapture = pIBUsbManager->m_pMainCapture;
        //		nRc = pMainCapture->Main_GetDeviceInfo(deviceIndex, &devDesc);
        strcpy(devDesc.productName, pIBUsbManager->m_DeviceDesc.productName);
        strcpy(devDesc.serialNumber, pIBUsbManager->m_DeviceDesc.serialNumber);
        strcpy(devDesc.fwVersion, pIBUsbManager->m_DeviceDesc.fwVersion);
        strcpy(devDesc.devRevision, pIBUsbManager->m_DeviceDesc.devRevision);
        sprintf(devDesc.interfaceType, "%s", "USB");
#if defined(__ppi__)	
        if (deviceIndex >= (m_nDeviceCount - m_nPPIDeviceCount))
        {
            sprintf(devDesc.interfaceType, "%s", "PPI");
        }
#endif
        devDesc.handle = pIBUsbManager->m_pMainCapture->GetDeviceHandle();
        devDesc.IsHandleOpened = TRUE;
    }

#ifdef __android__
    // 2013-03-11 Brian modify -
    //	GetBusInfoByIndex(deviceIndex, &devDesc.busNumber, &devDesc.devAddress, &devDesc.devPath[0]);
    {
        int  busNumber, devAddress;
        char devPath[IBSU_MAX_STR_LEN];

        GetBusInfoByIndex(deviceIndex, &busNumber, &devAddress, &devPath[0]);
		devDesc.devID = busNumber * 1000 + devAddress;
    }
#endif

    memcpy(pDeviceDesc, &devDesc, sizeof(IBSU_DeviceDescA));
#if defined(__IBSCAN_ULTIMATE_SDK__)
    if ((strcmp(pDeviceDesc->productName, "IBNW11C")			== 0) ||
            (strcmp(pDeviceDesc->productName, "IBSCAN310LS-W")   == 0))
    {
        strcpy(pDeviceDesc->productName, "WATSON");
    }
#endif

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetDeviceDescription (deviceIndex=%d, %s_v%s(%s)", deviceIndex,
                          pDeviceDesc->productName, pDeviceDesc->fwVersion, pDeviceDesc->serialNumber);

    return nRc;
}

#ifdef WINCE
int CIBScanUltimateApp::DLL_GetDeviceDescriptionW(
    const int          deviceIndex,                   ///< [in] Zero-based device index for device to lookup
    IBSU_DeviceDescW    *pDeviceDescW                   ///< [out] Basic device description \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    int				nRc = IBSU_STATUS_OK;

    IBSU_DeviceDescA devDescA;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetDeviceDescriptionW (deviceIndex=%d)", deviceIndex);

    if (::IsBadReadPtr(pDeviceDescW, sizeof(IBSU_DeviceDescW)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetDeviceDescriptionW (deviceIndex=%d)", deviceIndex);
        return nRc;
    }

    memset(pDeviceDescW, 0, sizeof(IBSU_DeviceDescW));
	memset(&devDescA, 0, sizeof(IBSU_DeviceDescA));

	// Call The ASCII implementation
	nRc = DLL_GetDeviceDescription( deviceIndex, &devDescA );
	if( nRc == IBSU_STATUS_OK )
	{
		// Convert char to wchar_t
		mbstowcs( pDeviceDescW->serialNumber, devDescA.serialNumber, IBSU_MAX_STR_LEN );
		mbstowcs( pDeviceDescW->productName, devDescA.productName, IBSU_MAX_STR_LEN );
		mbstowcs( pDeviceDescW->interfaceType, devDescA.interfaceType, IBSU_MAX_STR_LEN );
		mbstowcs( pDeviceDescW->fwVersion, devDescA.fwVersion, IBSU_MAX_STR_LEN );
		mbstowcs( pDeviceDescW->devRevision, devDescA.devRevision, IBSU_MAX_STR_LEN );
	}

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetDeviceDescriptionW (deviceIndex=%d, %S_v%S(%S)", deviceIndex,
                          pDeviceDescW->productName, pDeviceDescW->fwVersion, pDeviceDescW->serialNumber);

	return nRc;
}
#endif

int CIBScanUltimateApp::DLL_RegisterCallbacks(
    const int		       handle,                      ///< [in] Device handle obtained by IBSU_DeviceOpen()
    const IBSU_Events    events,                      ///< [in] Enum value to the notification function
    void                 *pEventName,                 ///< [in] Pointer to the notification function
    void                 *pContext                    ///< [in] Pointer to user context; this value is used as parameter for callback
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_RegisterCallbacks (handle=%d, events=%d)", handle, events);

    // Check only pEventName pointer.  We do not use pContext.
    if (::IsBadReadPtr(pEventName, sizeof(pEventName)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_RegisterCallbacks (handle=%d, events=%d)", handle, events);
        return nRc;
    }

    if (events != ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT &&
            events != ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE &&
            events != ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS)
    {
        pIBUsbManager = FindUsbDeviceInList(handle);
        nRc = _IsValidUsbHandle(pIBUsbManager);
        if (nRc != IBSU_STATUS_OK)
        {
            InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                                  "Exit  IBSU_RegisterCallbacks (handle=%d, events=%d, pEventName=0x%x, pContext=0x%x)",
                                  handle, events, pEventName, pContext);
            return nRc;
        }
    }

    nRc = _RegisterCallbacks(pIBUsbManager, events, pEventName, pContext);

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_RegisterCallbacks (handle=%d, events=%d, pEventName=0x%x, pContext=0x%x)",
                          handle, events, pEventName, pContext);

    return nRc;
}

int CIBScanUltimateApp::DLL_OpenDevice(
    const int  deviceIndex,                         ///< [in]  Zero-based device index for device to init
    const BOOL useUniformityMask,					///< [in]  Set device in uniformity mask mode (if set to TRUE)
    int        *pHandle                             ///< [out] Function returns device handle to be used for subsequent function calls \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    int	nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_OpenDevice (deviceIndex=%d)", deviceIndex);

    if (::IsBadReadPtr(pHandle, sizeof(int)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_OpenDevice (deviceIndex=%d)", deviceIndex);
        return nRc;
    }

    *pHandle = -1;

    FindAllDeviceCount();
    if (GetDeviceCount() <= deviceIndex)
    {
        nRc = IBSU_ERR_DEVICE_NOT_FOUND;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_OpenDevice (deviceIndex=%d)", deviceIndex);
        return nRc;
    }

    nRc = _OpenDevice(deviceIndex, NULL, useUniformityMask, FALSE, pHandle);

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_OpenDevice (deviceIndex=%d, pHandle=%d)", deviceIndex, *pHandle);

    return nRc;
}

int CIBScanUltimateApp::DLL_CloseDevice(
    const int  handle                              ///< [in] Device handle obtained by IBSU_OpenDevice()
    //      const BOOL sendToStandby                     ///< [in] Set device in standby mode (if set to TRUE)
)
{
//	CThreadSync Sync;

    int				nRc = IBSU_STATUS_OK;
    CIBUsbManager	*pIBUsbManager = NULL;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_CloseDevice (handle=%d)", handle);

    pIBUsbManager = FindUsbDeviceInList(handle);
    if (pIBUsbManager == NULL)
    {
        nRc = IBSU_ERR_DEVICE_NOT_INITIALIZED;
    }
    else if (pIBUsbManager->m_pMainCapture && 
        (!pIBUsbManager->m_bInitialized && !pIBUsbManager->m_bCompletedOpenDeviceThread))
    {
        // The device is initializing
        nRc = IBSU_ERR_RESOURCE_LOCKED;
    }
    else if (pIBUsbManager->m_pMainCapture &&
        !pIBUsbManager->m_bInitialized && pIBUsbManager->m_bIsCommunicationBreak)
    {
        // Uninitialized and communication break
        nRc = pIBUsbManager->m_pMainCapture->_Main_Release(TRUE);
        if (nRc == IBSU_STATUS_OK)
        {
		    DeleteUsbDeviceInList(handle);
        }
        nRc = IBSU_ERR_DEVICE_NOT_INITIALIZED;
    }
    else if (pIBUsbManager->m_pMainCapture)
    {
        nRc = pIBUsbManager->m_pMainCapture->_Main_Release(TRUE);
    }

    if (nRc == IBSU_STATUS_OK)
    {
        Sleep(10);

		DeleteUsbDeviceInList(handle);

#ifndef __android__
        if ((int)m_pListUsbManager.size() == 0 && m_pThreadPoolMgr != NULL)
        {
            m_pThreadPoolMgr->ClearThreadPool();
            delete theApp.m_pThreadPoolMgr;
            theApp.m_pThreadPoolMgr = NULL;
        }
#endif
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_CloseDevice (handle=%d)", handle);

    return nRc;
}

int CIBScanUltimateApp::DLL_CloseAllDevice(
    const BOOL appTerminate
)
{
//	CThreadSync Sync;

    int				nRc = IBSU_STATUS_OK;

	if (!appTerminate)
	{
		InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
							  "Enter IBSU_CloseAllDevice ()");
	}

    if ((int)m_pListUsbManager.size() == 0)
    {
        nRc = IBSU_ERR_DEVICE_NOT_INITIALIZED;
    }

//	nRc = _RemoveAllVector_UsbManager();
	std::vector<CIBUsbManager *>::iterator it = m_pListUsbManager.begin();
	while( it != m_pListUsbManager.end() )
	{
		if( *it )
		{
			if (!(*it)->m_bCompletedOpenDeviceThread)
			{
                nRc = IBSU_ERR_RESOURCE_LOCKED;
				++it;
				continue;
            }

            if ( (*it)->m_pMainCapture )
			{
                nRc = (*it)->m_pMainCapture->_Main_Release(TRUE);
                if (nRc == IBSU_STATUS_OK)
                {
					delete (*it)->m_pMainCapture;
					(*it)->m_pMainCapture = NULL;
                }
			}

            if (nRc == IBSU_STATUS_OK)
            {
			    delete *it;
			    it = m_pListUsbManager.erase(it);
            }
		}
		else
		{
			++it;
		}
	}

	if( nRc == IBSU_STATUS_OK )
	{
		m_pListUsbManager.clear();

#ifndef __android__
        if ((int)m_pListUsbManager.size() == 0 && m_pThreadPoolMgr != NULL)
        {
            m_pThreadPoolMgr->ClearThreadPool();
            delete theApp.m_pThreadPoolMgr;
            theApp.m_pThreadPoolMgr = NULL;
        }
#endif
	}

	if (!appTerminate)
	{
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
							  "Exit  IBSU_CloseAllDevice ()");
	}

    return nRc;
}

int CIBScanUltimateApp::DLL_IsDeviceOpened(
    const int  handle                               ///< [in] Device handle obtained by IBSU_OpenDevice()
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_IsDeviceOpened (handle=%d)", handle);

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_IsDeviceOpened (handle=%d)", handle);

    return nRc;
}

int CIBScanUltimateApp::DLL_GetProperty(
    const int             handle,                   ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const IBSU_PropertyId propertyId,               ///< [in]  Property identifier to get value for
    LPSTR                 propertyValue             ///< [out] String returning property value \n
    ///<       Memory must be provided by caller
    ///<       (needs to be able to hold @ref IBSU_MAX_STR_LEN characters)
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetProperty (handle=%d, propertyId=%d)", handle, propertyId);

    if (::IsBadReadPtr(propertyValue, sizeof(propertyValue)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetProperty (handle=%d, propertyId=%d)", handle, propertyId);
        return nRc;
    }

    propertyValue[0] = '\0';

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Main_GetProperty(propertyId, propertyValue);
    }

#if defined(__IBSCAN_ULTIMATE_SDK__)
    // 2013-02-27 Brian modify - It is unlikely to execute wrongly, but the code is wrong
    //	if( propertyId == ENUM_IBSU_PROPERTY_PRODUCT_ID &&
    //		( strcmp( propertyValue, "IBNW11C" )			== 0) ||
    //		( strcmp( propertyValue, "IBSCAN310LS-W" )   == 0) )
    if (propertyId == ENUM_IBSU_PROPERTY_PRODUCT_ID &&
            ((strcmp(propertyValue, "IBNW11C")			== 0) ||
             (strcmp(propertyValue, "IBSCAN310LS-W")   == 0)))
    {
        strcpy(propertyValue, "WATSON");
    }
#endif

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetProperty (handle=%d, propertyId=%d, propertyValue=%s)",
                          handle, propertyId, propertyValue);

    return nRc;
}

int CIBScanUltimateApp::DLL_GetPropertyW(
    const int             handle,                   ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const IBSU_PropertyId propertyId,               ///< [in]  Property identifier to get value for
    LPWSTR                propertyValue             ///< [out] String returning property value \n
    ///<       Memory must be provided by caller
    ///<       (needs to be able to hold @ref IBSU_MAX_STR_LEN characters)
)
{
//	CThreadSync Sync;
#ifdef WINCE
	char			szProperty[IBSU_MAX_STR_LEN];
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetPropertyW");

    if (::IsBadReadPtr(propertyValue, sizeof(propertyValue)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetPropertyW");
        return nRc;
    }

	nRc = DLL_GetProperty(handle, propertyId, szProperty);

	// Convert char to wchar_t
	mbstowcs( propertyValue, szProperty, IBSU_MAX_STR_LEN );

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetPropertyW");

    return nRc;
#else
	return IBSU_ERR_NOT_SUPPORTED;
#endif
}

int CIBScanUltimateApp::DLL_SetProperty(
    const int             handle,                   ///< [in] Device handle obtained by IBSU_OpenDevice()
    const IBSU_PropertyId propertyId,               ///< [in] Property identifier to set value for
    LPCSTR                propertyValue             ///< [in] String containing property value
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SetProperty (handle=%d, propertyId=%d)",
                          handle, propertyId);

    if (::IsBadReadPtr(propertyValue, sizeof(propertyValue)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_SetProperty (handle=%d, propertyId=%d)",
                              handle, propertyId);
        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Main_SetProperty(propertyId, propertyValue);

        Sleep(50);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SetProperty (handle=%d, propertyId=%d, propertyValue=%s)",
                          handle, propertyId, propertyValue);

    return nRc;
}

int CIBScanUltimateApp::DLL_SetPropertyW(
    const int             handle,                   ///< [in] Device handle obtained by IBSU_OpenDevice()
    const IBSU_PropertyId propertyId,               ///< [in] Property identifier to set value for
    LPCWSTR               propertyValue             ///< [in] String containing property value
)
{
//	CThreadSync Sync;

#ifdef WINCE
    int				nRc = IBSU_STATUS_OK;
	char			szPropertyValue[IBSU_MAX_STR_LEN];

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SetPropertyW");

    if (::IsBadReadPtr(propertyValue, sizeof(propertyValue)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_SetPropertyW");
        return nRc;
    }

	int nBufferLength = wcstombs( NULL, propertyValue, IBSU_MAX_STR_LEN );
	wcstombs( szPropertyValue, propertyValue, nBufferLength+1 );
	*(szPropertyValue+IBSU_MAX_STR_LEN-1) = '\0'; // Ensure string is always null terminated

	nRc = DLL_SetProperty(handle, propertyId, szPropertyValue);

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SetPropertyW");

    return nRc;
#else
	return IBSU_ERR_NOT_SUPPORTED;
#endif
}

int CIBScanUltimateApp::DLL_EnableTraceLog(
    const BOOL on  ///< [in] TRUE to turn trace log on; FALSE to turn trace log off
)
{
//	CThreadSync Sync;

    int nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_EnableTraceLog (on=%d)", on);

    m_traceLogEnabled = on;

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_EnableTraceLog (on-%d)", on);

    return nRc;
}

int CIBScanUltimateApp::DLL_IsCaptureAvailable(
    const int                   handle,             ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const IBSU_ImageType        imageType,          ///< [in]  Image type to verify
    const IBSU_ImageResolution  imageResolution,    ///< [in]  Requested capture resolution
    BOOL                        *pIsAvailable        ///< [out] Returns TRUE if mode is available \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_IsCaptureAvailable (handle=%d, imageType=%d, imageResolutin=%d)",
                          handle, imageType, imageResolution);
    if (::IsBadReadPtr(pIsAvailable, sizeof(pIsAvailable)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_IsCaptureAvailable (handle=%d, imageType=%d, imageResolutin=%d)",
                              handle, imageType, imageResolution);
        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    *pIsAvailable = FALSE;
    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_IsModeAvailable(imageType, imageResolution, pIsAvailable);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_IsCaptureAvailable (handle=%d, imageType=%d, imageResolutin=%d, pIsAvailable=%d)",
                          handle, imageType, imageResolution, *pIsAvailable);

    return nRc;
}

int CIBScanUltimateApp::DLL_BeginCaptureImage(
    const int	                  handle,             ///< [in] Device handle obtained by IBSU_OpenDevice()
    const IBSU_ImageType        imageType,          ///< [in]  Image type to capture
    const IBSU_ImageResolution  imageResolution,    ///< [in]  Requested capture resolution
    const DWORD                 captureOptions      ///< [in]  Bit coded capture options to use (see @ref CaptureOptions)
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_BeginCaptureImage (handle=%d, imageType=%d, imageResolutin=%d, captureOptions=0x%x)",
                          handle, imageType, imageResolution, captureOptions);

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_Start(imageType, imageResolution, captureOptions);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_BeginCaptureImage (handle=%d, imageType=%d, imageResolutin=%d, captureOptions=0x%x)",
                          handle, imageType, imageResolution, captureOptions);

    return nRc;
}

int CIBScanUltimateApp::DLL_CancelCaptureImage(
    const int handle                                ///< [in] Device handle obtained by IBSU_OpenDevice()
)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_CancelCaptureImage (handle=%d)", handle);

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_Abort();
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_CancelCaptureImage (handle=%d)", handle);

    return nRc;
}

int CIBScanUltimateApp::DLL_IsCaptureActive(
    const int handle,                               ///< [in]  Device handle obtained by IBSU_OpenDevice()
    BOOL      *pIsActive                            ///< [out] Returns TRUE if acquisition is in progress
    ///<       (preview or result image acquisition) \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_IsCaptureActive (handle=%d)", handle);

    if (::IsBadReadPtr(pIsActive, sizeof(pIsActive)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_IsCaptureActive (handle=%d)", handle);
        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    *pIsActive = FALSE;
    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_IsActive(pIsActive);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_IsCaptureActive (handle=%d, pIsActive=%d)", handle, *pIsActive);

    return nRc;
}

int CIBScanUltimateApp::DLL_TakeResultImageManually(
    const int handle                               ///< [in]  Device handle obtained by IBSU_OpenDevice()
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_TakeResultImageManually (handle=%d)", handle);

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_TakeResultImageManually(TRUE);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_TakeResultImageManually (handle=%d)", handle);

    return nRc;
}

int CIBScanUltimateApp::DLL_GetContrast(
    const int handle,                               ///< [in]  Device handle obtained by IBSU_OpenDevice()
    int       *pContrastValue                       ///< [out] Contrast value (range: 0 <= value <= @ref IBSU_MAX_CONTRAST_VALUE) \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetContrast (handle=%d)", handle);

    if (::IsBadReadPtr(pContrastValue, sizeof(int *)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetContrast (handle=%d)", handle);
        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    *pContrastValue = -1;
    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_GetContrast(pContrastValue);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetContrast (handle=%d, pContrastValue=%d)", handle, *pContrastValue);

    return nRc;
}

int CIBScanUltimateApp::DLL_SetContrast(
    const int handle,                               ///< [in] Device handle obtained by IBSU_OpenDevice()
    const int contrastValue                         ///< [in] Contrast value (range: 0 <= value <= @ref IBSU_MAX_CONTRAST_VALUE)
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SetContrast (handle=%d, contrastValue=%d)", handle, contrastValue);

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_SetContrast(contrastValue);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SetContrast (handle=%d, contrastValue=%d)", handle, contrastValue);

    return nRc;
}

int CIBScanUltimateApp::DLL_GetLEOperationMode(
    const int			   handle,                               ///< [in] Device handle obtained by IBSU_OpenDevice()
    IBSU_LEOperationMode *leOperationMode								  ///< [out] Touch sensor operation mode
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetLEOperationMode (handle=%d)", handle);

    if (::IsBadReadPtr(leOperationMode, sizeof(leOperationMode)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetLEOperationMode (handle=%d)", handle);
        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    *leOperationMode = ENUM_IBSU_LE_OPERATION_OFF;
    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_GetLEOperationMode(leOperationMode);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetLEOperationMode (handle=%d, leOperationMode=%d)", handle, *leOperationMode);

    return nRc;
}

int CIBScanUltimateApp::DLL_SetLEOperationMode(
    const int					 handle,                               ///< [in] Device handle obtained by IBSU_OpenDevice()
    const IBSU_LEOperationMode leOperationMode						  ///< [in] Touch sensor operation mode
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SetLEOperationMode (handle=%d, leOperationMode=%d)", handle, leOperationMode);

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_SetLEOperationMode(leOperationMode);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SetLEOperationMode (handle=%d, leOperationMode=%d)", handle, leOperationMode);

    return nRc;
}

int CIBScanUltimateApp::DLL_IsTouchedFinger(
    const int	handle,                               ///< [in] Device handle obtained by IBSU_OpenDevice()
    int       *touchInValue                         ///< [out] touchInValue value (0 : touch off, 1 : touch on) \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_IsTouchedFinger (handle=%d)", handle);

    if (::IsBadReadPtr(touchInValue, sizeof(touchInValue)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_IsTouchedFinger (handle=%d)", handle);
        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    *touchInValue = -1;
    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Control_GetTouchInStatus(touchInValue);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_IsTouchedFinger (handle=%d, touchInValue=%d)", handle, *touchInValue);

    return nRc;
}

int CIBScanUltimateApp::DLL_GetOperableLEDs(
    const int     handle,                            ///< [in]  Device handle obtained by IBSU_OpenDevice()
    IBSU_LedType  *pLedType,                         ///< [out] Type of LED's \n
    ///<       Memory must be provided by caller
    int           *pLedCount,                        ///< [out] Number of LED's \n
    ///<       Memory must be provided by caller
    DWORD         *pOperableLEDs                     ///< [out] Bit pattern of operable LED's \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetOperableLEDs (handle=%d)", handle);

    if (::IsBadReadPtr(pLedType, sizeof(pLedType)) ||
            ::IsBadReadPtr(pLedCount, sizeof(pLedCount)) ||
            ::IsBadReadPtr(pOperableLEDs, sizeof(pOperableLEDs)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetOperableLEDs (handle=%d)", handle);
        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    *pLedType = ENUM_IBSU_LED_TYPE_NONE;
    *pLedCount = -1;
    *pOperableLEDs = 0;
    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Control_GetAvailableLEDs(pLedType, pLedCount, pOperableLEDs);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetOperableLEDs (handle=%d, pLedType=%d, pLedCount=%d, pOperableLEDs=0x%08x)",
                          handle, *pLedType, *pLedCount, *pOperableLEDs);

    return nRc;
}

int CIBScanUltimateApp::DLL_GetLEDs(
    const int		handle,                               ///< [in]  Device handle obtained by IBSU_OpenDevice()
    DWORD			*pActiveLEDs                          ///< [out] get active LEDs
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetLEDs (handle=%d)", handle);

    if (::IsBadReadPtr(pActiveLEDs, sizeof(pActiveLEDs)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetLEDs (handle=%d)", handle);
        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    *pActiveLEDs = 0;
    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Control_GetActiveLEDs(pActiveLEDs, FALSE);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetLEDs (handle=%d, pActiveLEDs=0x%08x)", handle, *pActiveLEDs);

    return nRc;
}

int CIBScanUltimateApp::DLL_SetLEDs(
    const int		handle,                               ///< [in] Device handle obtained by IBSU_OpenDevice()
    const DWORD	activeLEDs							  ///< [in] set active LEDs
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SetLEDs (handle=%d, activeLEDs=0x%08x)", handle, activeLEDs);

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Control_SetActiveLEDs(activeLEDs, FALSE);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SetLEDs (handle=%d, activeLEDs=0x%08x)", handle, activeLEDs);

    return nRc;
}

int CIBScanUltimateApp::DLL_CreateClientWindow(
    const int         handle,                          ///< [in] Device handle obtained by IBSU_OpenDevice()
    const IBSU_HWND   hWindow,                         ///< [in] Windows handle to draw
    const DWORD		left,                            ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
    const DWORD		top,                             ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
    const DWORD		right,                           ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
    const DWORD		bottom                           ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_CreateClientWindow (handle=%d, hWindow=%x, left=%d, top=%d, right=%d, bottom=%d)",
                          handle, hWindow, left, top, right, bottom);

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->ClientWindow_Create(hWindow, left, top, right, bottom);
    }

#ifdef _WINDOWS
	if( nRc == IBSU_STATUS_OK )
	{
		if( m_hClientWnd == NULL )
		{
			g_oldClientWindowProc = (WNDPROC)SetWindowLong(hWindow, GWLP_WNDPROC, (LONG)ClientWindowSubProc);
			m_hClientWnd = hWindow;
		}
		else if( m_hClientWnd != hWindow )
		{
#ifdef __64BITSYSTEM
			SetWindowLong(m_hClientWnd, GWLP_WNDPROC, (LONG_PTR)g_oldClientWindowProc);
#else
			SetWindowLong(m_hClientWnd, GWL_WNDPROC, (LONG)g_oldClientWindowProc);
#endif
			g_oldClientWindowProc = (WNDPROC)SetWindowLong(hWindow, GWLP_WNDPROC, (LONG)ClientWindowSubProc);
			m_hClientWnd = hWindow;
		}
	}
#endif

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_CreateClientWindow (handle=%d, hWindow=%x, left=%d, top=%d, right=%d, bottom=%d)",
                          handle, hWindow, left, top, right, bottom);

    return nRc;
}

int CIBScanUltimateApp::DLL_DestroyClientWindow(
    const int         handle,                          ///< [in] Device handle obtained by IBSU_OpenDevice()
    const BOOL		clearExistingInfo                ///< [in] clear the existing display information about display property, overlay text.
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_DestroyClientWindow (handle=%d, clearExistingInfo=%d)", handle, clearExistingInfo);

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->ClientWindow_Destroy(clearExistingInfo);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_DestroyClientWindow (handle=%d, clearExistingInfo=%d)", handle, clearExistingInfo);
    return nRc;
}

int CIBScanUltimateApp::DLL_GetClientWindowProperty(
    const int                         handle,                 ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const IBSU_ClientWindowPropertyId propertyId,             ///< [in]  Property identifier to set value for
    LPSTR                             propertyValue           ///< [out] String returning property value \n
    ///<       Memory must be provided by caller
    ///<       (needs to be able to hold @ref IBSU_MAX_STR_LEN characters)
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetClientWindowProperty (handle=%d, propertyId=%d)", handle, propertyId);

    if (::IsBadReadPtr(propertyValue, sizeof(propertyValue)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetClientWindowProperty (handle=%d, propertyId=%d)", handle, propertyId);
        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);
    propertyValue[0] = '\0';

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->ClientWindow_GetProperty(propertyId, propertyValue);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetClientWindowProperty (handle=%d, propertyId=%d, propertyValue=%s)",
                          handle, propertyId, propertyValue);

    return nRc;
}

int CIBScanUltimateApp::DLL_GetClientWindowPropertyW(
    const int                         handle,                 ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const IBSU_ClientWindowPropertyId propertyId,             ///< [in]  Property identifier to set value for
    LPWSTR                            propertyValue           ///< [out] String returning property value \n
    ///<       Memory must be provided by caller
    ///<       (needs to be able to hold @ref IBSU_MAX_STR_LEN characters)
)
{
//	CThreadSync Sync;

#ifdef WINCE
    int				nRc = IBSU_STATUS_OK;
	char			szProperty[IBSU_MAX_STR_LEN];

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetClientWindowPropertyW");

    if (::IsBadReadPtr(propertyValue, sizeof(propertyValue)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetClientWindowPropertyW");
        return nRc;
    }

	nRc = DLL_GetClientWindowProperty(handle, propertyId, szProperty);

	// Convert char to wchar_t
	mbstowcs( propertyValue, szProperty, IBSU_MAX_STR_LEN );

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetClientWindowPropertyW");

    return nRc;
#else
	return IBSU_ERR_NOT_SUPPORTED;
#endif
}

int CIBScanUltimateApp::DLL_SetClientDisplayProperty(
    const int                         handle,                 ///< [in] Device handle obtained by IBSU_OpenDevice()
    const IBSU_ClientWindowPropertyId propertyId,             ///< [in] Property identifier to set value for
    LPCSTR                            propertyValue           ///< [in] String containing property value
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SetClientDisplayProperty (handle=%d, propertyId=%d, propertyValue=%s)",
                          handle, propertyId, propertyValue);

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->ClientWindow_SetProperty(propertyId, propertyValue);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SetClientDisplayProperty (handle=%d, propertyId=%d, propertyValue=%s)",
                          handle, propertyId, propertyValue);

    return nRc;
}

int CIBScanUltimateApp::DLL_SetClientDisplayPropertyW(
    const int                         handle,                 ///< [in] Device handle obtained by IBSU_OpenDevice()
    const IBSU_ClientWindowPropertyId propertyId,             ///< [in] Property identifier to set value for
    LPCWSTR                           propertyValue           ///< [in] String containing property value
)
{
//	CThreadSync Sync;

#ifdef WINCE
    int				nRc = IBSU_STATUS_OK;
	char			szPropertyValue[IBSU_MAX_STR_LEN];

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SetClientDisplayPropertyW");

	int nBufferLength = wcstombs( NULL, propertyValue, IBSU_MAX_STR_LEN );
	wcstombs( szPropertyValue, propertyValue, nBufferLength+1 );
	*(szPropertyValue+IBSU_MAX_STR_LEN-1) = '\0'; // Ensure string is always null terminated

	nRc = DLL_SetClientDisplayProperty(handle, propertyId, szPropertyValue);

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SetClientDisplayPropertyW");

    return nRc;
#else
	return IBSU_ERR_NOT_SUPPORTED;
#endif
}

int CIBScanUltimateApp::DLL_SetClientWindowOverlayText(
    const int         handle,                            ///< [in] Device handle obtained by IBSU_OpenDevice()
    const char        *fontName,                         ///< [in] font name for display on window
    const int         fontSize,                          ///< [in] font size for display on window
    const BOOL        fontBold,                          ///< [in] font bold for display on window
    const char        *text,                             ///< [in] text for display on window
    const int         posX,                              ///< [in] X coordinate of text for display on window
    const int         posY,                              ///< [in] Y coordinate of text for display on window
    const DWORD       textColor                          ///< [in] text color for display on window
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SetClientWindowOverlayText (handle=%d, fontSize=%d, fontBold=%d, posX=%d, posY=%d, textColor=%d)",
                          handle, fontSize, fontBold, posX, posY, textColor);

    if (::IsBadReadPtr(fontName, sizeof(char)) ||
            ::IsBadReadPtr(text, sizeof(char)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit IBSU_SetClientWindowOverlayText (handle=%d, fontSize=%d, fontBold=%d, posX=%d, posY=%d, textColor=%d)",
                              handle, fontSize, fontBold, posX, posY, textColor);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->ClientWindow_SetOverlayText(fontName, fontSize, fontBold, text, posX, posY, textColor);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SetClientWindowOverlayText (handle=%d, fontName=%s, fontSize=%d, fontBold=%d, " \
                          "text=%s, posX=%d, posY=%d, textColor=%d)",
                          handle, fontName, fontSize, fontBold, text, posX, posY, textColor);

    return nRc;
}

int CIBScanUltimateApp::DLL_SetClientWindowOverlayTextW(
    const int         handle,                            ///< [in] Device handle obtained by IBSU_OpenDevice()
    const WCHAR       *fontName,                         ///< [in] font name for display on window
    const int         fontSize,                          ///< [in] font size for display on window
    const BOOL        fontBold,                          ///< [in] font bold for display on window
    const WCHAR       *text,                             ///< [in] text for display on window
    const int         posX,                              ///< [in] X coordinate of text for display on window
    const int         posY,                              ///< [in] Y coordinate of text for display on window
    const DWORD       textColor                          ///< [in] text color for display on window
)
{
//	CThreadSync Sync;

#ifdef WINCE
    int				nRc = IBSU_STATUS_OK;
	char            szFontName[IBSU_MAX_STR_LEN];
	char            szText[IBSU_MAX_STR_LEN];
	int				nBufferLength;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SetClientWindowOverlayTextW");

    if (::IsBadReadPtr(fontName, sizeof(char)) ||
            ::IsBadReadPtr(text, sizeof(char)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit IBSU_SetClientWindowOverlayTextW");

        return nRc;
    }

	nBufferLength = wcstombs( NULL, fontName, IBSU_MAX_STR_LEN );
	wcstombs( szFontName, fontName, nBufferLength+1 );
	*(szFontName+IBSU_MAX_STR_LEN-1) = '\0'; // Ensure string is always null terminated
	nBufferLength = wcstombs( NULL, text, IBSU_MAX_STR_LEN );
	wcstombs( szText, text, nBufferLength+1 );
	*(szText+IBSU_MAX_STR_LEN-1) = '\0'; // Ensure string is always null terminated

	nRc = DLL_SetClientWindowOverlayText(handle, szFontName, fontSize, fontBold, szText, posX, posY, textColor);

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SetClientWindowOverlayTextW");

    return nRc;
#else
	return IBSU_ERR_NOT_SUPPORTED;
#endif
}

int CIBScanUltimateApp::DLL_GenerateZoomOutImage(
    const IBSU_ImageData  inImage,                       ///< [in] Original image
    BYTE                  *outImage,                     ///< [out] Pointer to zoom-out image data buffer \n
    ///<       Memory must be provided by caller
    const int             outWidth,                      ///< [in]  Width for zoom-out image
    const int             outHeight,                     ///< [in]  Height for zoom-out image
    const BYTE            bkColor                        ///< [in]  Background color for remain area from zoom-out image
)
{
//	CThreadSync Sync;

    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GenerateZoomOutImage (inImage=%x, outWidth=%d, outHeight=%d, bkColor=%d)",
                          inImage, outWidth, outHeight, bkColor);

    if (::IsBadReadPtr(inImage.Buffer, sizeof(inImage.Buffer)) ||
            ::IsBadReadPtr(outImage, sizeof(outWidth * outHeight)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit IBSU_GenerateZoomOutImage (inImage=%x, outWidth=%d, outHeight=%d, bkColor=%d)",
                              &inImage, outWidth, outHeight, bkColor);

        return nRc;
    }

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = Main_GenerateZoomOutImage(inImage, outImage, outWidth, outHeight, bkColor);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GenerateZoomOutImage (inImage=%x, outImage=%x, outWidth=%d, outHeight=%d, bkColor=%d)",
                          &inImage, outImage, outWidth, outHeight, bkColor);

    return nRc;
}

int CIBScanUltimateApp::DLL_SaveBitmapImage(
    LPCSTR                filePath,                      ///< [in]  File path to save bitmap
    const BYTE            *imgBuffer,                    ///< [in]  Point to image data
    const DWORD           width,                         ///< [in]  Image width
    const DWORD           height,                        ///< [in]  Image height
    const int             pitch,                         ///< [in]  Image line pitch (in Bytes).\n
    ///        Positive values indicate top down line order,
    ///        Negative values mean bottom up line order
    const double          resX,                          ///< [in]  Image horizontal resolution (in PPI)
    const double          resY                          ///< [in]  Image vertical resolution (in PPI)
)
{
//	CThreadSync Sync;

    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SaveBitmapImage (width=%d, height=%d, pitch=%d, resX=%0.1f, resY=%0.1f)",
                          width, height, pitch, resX, resY);

    if (::IsBadReadPtr(filePath, sizeof(*filePath)) ||
            ::IsBadReadPtr(imgBuffer, width * height))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_SaveBitmapImage (width=%d, height=%d, pitch=%d, resX=%0.1f, resY=%0.1f)",
                              width, height, pitch, resX, resY);

        return nRc;
    }

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = Main_SaveBitmapImage(filePath, imgBuffer, width, height, pitch, resX, resY);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SaveBitmapImage (filePath=%s, imgBuffer=%x, width=%d, height=%d, pitch=%d, resX=%0.1f, resY=%0.1f)",
                          filePath, imgBuffer, width, height, pitch, resX, resY);

    return nRc;
}

int CIBScanUltimateApp::DLL_SaveBitmapImageW(
    LPCWSTR               filePath,                      ///< [in]  File path to save bitmap
    const BYTE            *imgBuffer,                    ///< [in]  Point to image data
    const DWORD           width,                         ///< [in]  Image width
    const DWORD           height,                        ///< [in]  Image height
    const int             pitch,                         ///< [in]  Image line pitch (in Bytes).\n
    ///        Positive values indicate top down line order,
    ///        Negative values mean bottom up line order
    const double          resX,                          ///< [in]  Image horizontal resolution (in PPI)
    const double          resY                          ///< [in]  Image vertical resolution (in PPI)
)
{
#ifdef WINCE
	int		nRc = IBSU_STATUS_OK;
	char    szFilePath[MAX_PATH]={0};

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SaveBitmapImageW");

    if (::IsBadReadPtr(filePath, sizeof(*filePath)) ||
            ::IsBadReadPtr(imgBuffer, width * height))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_SaveBitmapImageW");

        return nRc;
    }

	int nBufferLength = wcstombs( NULL, filePath, MAX_PATH );
	wcstombs( szFilePath, filePath, nBufferLength+1 );
	*(szFilePath+MAX_PATH-1) = '\0'; // Ensure string is always null terminated

	nRc = DLL_SaveBitmapImage( szFilePath, imgBuffer, width, height, pitch, resX, resY );

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SaveBitmapImageW");

	return nRc;
#else
	return IBSU_ERR_NOT_SUPPORTED;
#endif
}

int CIBScanUltimateApp::DLL_AsyncOpenDevice(
    const int  deviceIndex,                               ///< [in]  Zero-based device index for device to init
    const BOOL useUniformityMask 					      ///< [in]  Set device in uniformity mask mode (if set to TRUE)
)
{
//	CThreadSync Sync;

    int	nRc = IBSU_STATUS_OK;
    int	devHandle;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_AsyncOpenDevice (deviceIndex=%d)", deviceIndex);

    FindAllDeviceCount();
    if (GetDeviceCount() <= deviceIndex)
    {
        nRc = IBSU_ERR_DEVICE_NOT_FOUND;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_AsyncOpenDevice (deviceIndex=%d)", deviceIndex);
        return nRc;
    }

    nRc = _OpenDevice(deviceIndex, NULL, useUniformityMask, TRUE, &devHandle);

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_AsyncOpenDevice (deviceIndex=%d)", deviceIndex);

    return nRc;
}

int CIBScanUltimateApp::DLL_BGetImage(
    const int	                  handle,                  ///< [in]  Device handle
    IBSU_ImageData              *pImage,                 ///< [out] Image data of preview image or result image
    IBSU_ImageType              *pImageType,             ///< [out] Image type
    IBSU_ImageData              *pSplitImageArray,       ///< [out] Finger array to be splited from result image (two-fingers, four-fingers)
    int                         *pSplitImageArrayCount,  ///< [out] Array count to be splited from result image (two-fingers, four-fingers)
    IBSU_FingerCountState       *pFingerCountState,      ///< [out] Finger count state
    IBSU_FingerQualityState     *pQualityArray,          ///< [out] Finger quality state
    int                         *pQualityArrayCount      ///< [out] Finger quality count
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_BGetImage (handle=%d)", handle);

    if (::IsBadReadPtr(pImage, sizeof(*pImage)) ||
            ::IsBadReadPtr(pImageType, sizeof(*pImageType)) ||
            ::IsBadReadPtr(pSplitImageArray, sizeof(*pSplitImageArray)) ||
            ::IsBadReadPtr(pSplitImageArrayCount, sizeof(*pSplitImageArrayCount)) ||
            ::IsBadReadPtr(pFingerCountState, sizeof(*pFingerCountState)) ||
            ::IsBadReadPtr(pQualityArray, sizeof(*pQualityArray)) ||
            ::IsBadReadPtr(pQualityArrayCount, sizeof(*pQualityArrayCount)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_BGetImage (handle=%d)", handle);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_BGetImage(pImage, pImageType, pSplitImageArray, pSplitImageArrayCount,
                pFingerCountState, pQualityArray, pQualityArrayCount);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_BGetImage (handle=%d, pImage=%x, pImageType=%d, pSplitImageArray=%x, " \
                          "pSplitImageArrayCount=%d, pFingerCountState=%d, pQualityArray=%x, pQualityArrayCount=%d)",
                          handle, pImage, *pImageType, pSplitImageArray, *pSplitImageArrayCount,
                          *pFingerCountState, pQualityArray, *pQualityArrayCount);

    return nRc;
}

int CIBScanUltimateApp::DLL_BGetImageEx(
	const int                handle,
	int                     *pImageStatus,
	IBSU_ImageData          *pImage,
	IBSU_ImageType          *pImageType,
	int                     *pDetectedFingerCount,
	IBSU_ImageData          *pSegmentImageArray,
	IBSU_SegmentPosition    *pSegmentPositionArray,
	int                     *pSegmentImageArrayCount,
	IBSU_FingerCountState   *pFingerCountState,
	IBSU_FingerQualityState *pQualityArray,
	int                     *pQualityArrayCount
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_BGetImageEx (handle=%d)", handle);

    if (::IsBadReadPtr(pImageStatus, sizeof(*pImageStatus)) ||
            ::IsBadReadPtr(pImage, sizeof(*pImage)) ||
            ::IsBadReadPtr(pImageType, sizeof(*pImageType)) ||
            ::IsBadReadPtr(pDetectedFingerCount, sizeof(*pDetectedFingerCount)) ||
            ::IsBadReadPtr(pSegmentImageArray, sizeof(*pSegmentImageArray)) ||
            ::IsBadReadPtr(pSegmentPositionArray, sizeof(*pSegmentPositionArray)) ||
            ::IsBadReadPtr(pSegmentImageArrayCount, sizeof(*pSegmentImageArrayCount)) ||
            ::IsBadReadPtr(pFingerCountState, sizeof(*pFingerCountState)) ||
            ::IsBadReadPtr(pQualityArray, sizeof(*pQualityArray)) ||
            ::IsBadReadPtr(pQualityArrayCount, sizeof(*pQualityArrayCount)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_BGetImageEx (handle=%d)", handle);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_BGetImageEx(pImageStatus, pImage, pImageType, pDetectedFingerCount,
				pSegmentImageArray, pSegmentPositionArray, pSegmentImageArrayCount,
                pFingerCountState, pQualityArray, pQualityArrayCount);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_BGetImageEx (handle=%d, pImageStatus=%d, pImage=%x, pImageType=%d, pDetectedFingerCount=%d, " \
                          "pSegmentImageArray=%x, pSegmentPositionArray=%x, pSegmentImageArrayCount=%d, " \
						  "pFingerCountState=%d, pQualityArray=%x, pQualityArrayCount=%d)",
                          handle, *pImageStatus, pImage, *pImageType, *pDetectedFingerCount, 
						  pSegmentImageArray, pSegmentPositionArray, *pSegmentImageArrayCount,
                          *pFingerCountState, pQualityArray, *pQualityArrayCount);

    return nRc;
}

int CIBScanUltimateApp::DLL_BGetInitProgress(
    const int	  deviceIndex,                             ///< [in]  Device index
    BOOL        *pIsComplete,                            ///< [out] Is that complete the initialize device
    int         *pHandle,                                ///< [out] Device handle
    int         *pProgressValue                          ///< [out] progress value of initialize device
)
{
//	CThreadSync Sync;

    CIBUsbManager *pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_BGetInitProgress (deviceIndex=%d)", deviceIndex);

    if (::IsBadReadPtr(pIsComplete, sizeof(*pIsComplete)) ||
            ::IsBadReadPtr(pHandle, sizeof(*pHandle)) ||
            ::IsBadReadPtr(pProgressValue, sizeof(*pProgressValue)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_BGetInitProgress (deviceIndex=%d)", deviceIndex);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceByIndexInList(deviceIndex);

    if (pIBUsbManager && pIBUsbManager->m_bIsCommunicationBreak)
    {
        nRc = IBSU_ERR_DEVICE_INVALID_STATE;
    }
    else if (pIBUsbManager == NULL ||
             pIBUsbManager->m_pMainCapture == NULL)
    {
        nRc = IBSU_ERR_DEVICE_NOT_INITIALIZED;
    }

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Main_BGetInitProgress(pIsComplete, pHandle, pProgressValue);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_BGetInitProgress (deviceIndex=%d, pIsComplete=%d, pHandle=%d, pProgressValue=%d)",
                          deviceIndex, *pIsComplete, *pHandle, *pProgressValue);

    return nRc;
}

int CIBScanUltimateApp::DLL_BGetClearPlatenAtCapture(
    const int	            handle,                        ///< [in]  Device handle
    IBSU_PlatenState      *pPlatenState                  ///< [out] Platen state
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_BGetClearPlatenAtCapture (handle=%d)", handle);

    if (::IsBadReadPtr(pPlatenState, sizeof(*pPlatenState)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_BGetClearPlatenAtCapture (handle=%d)", handle);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_BGetClearPlatenAtCapture(pPlatenState);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_BGetClearPlatenAtCapture (handle=%d, pPlatenState=%d)", handle, *pPlatenState);

    return nRc;
}

int CIBScanUltimateApp::DLL_BGetRollingInfo(
    const int		    handle,                             ///< [in]  Device handle
    IBSU_RollingState *pRollingState,                     ///< [out] Rolling state
    int               *pRollingLineX                      ///< [out] x-coordinate of Rolling line for drawing
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_BGetRollingInfo (handle=%d)", handle);

    if (::IsBadReadPtr(pRollingState, sizeof(*pRollingState)) ||
            ::IsBadReadPtr(pRollingLineX, sizeof(*pRollingLineX)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_BGetRollingInfo (handle=%d)", handle);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_BGetRollingInfo(pRollingState, pRollingLineX);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_BGetRollingState (handle=%d, pRollingState=%d, pRollingLineX=%d)", handle, *pRollingState, *pRollingLineX);

    return nRc;
}

int CIBScanUltimateApp::DLL_BGetRollingInfoEx(
    const int		    handle,                             ///< [in]  Device handle
    IBSU_RollingState *pRollingState,                     ///< [out] Rolling state
    int               *pRollingLineX,                      ///< [out] x-coordinate of Rolling line for drawing
	int               *pRollingDirection,                      ///< [out] Rolling direction
	int               *pRollingWidth                      ///< [out] Rolling image width (mm)
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_BGetRollingInfoEx (handle=%d)", handle);

    if (::IsBadReadPtr(pRollingState, sizeof(*pRollingState)) ||
        ::IsBadReadPtr(pRollingLineX, sizeof(*pRollingLineX)) ||
		::IsBadReadPtr(pRollingDirection, sizeof(*pRollingDirection)) ||
		::IsBadReadPtr(pRollingWidth, sizeof(*pRollingWidth)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_BGetRollingInfoEx (handle=%d)", handle);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_BGetRollingInfoEx(pRollingState, pRollingLineX, pRollingDirection, pRollingWidth);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_BGetRollingInfoEx (handle=%d, pRollingState=%d, pRollingLineX=%d, pRollingDirection=%d, pRollingWidth=%d)", 
						  handle, *pRollingState, *pRollingLineX, *pRollingDirection, *pRollingWidth);

    return nRc;
}

int CIBScanUltimateApp::DLL_OpenDeviceEx(
    const int	    deviceIndex,                           ///< [in]  Zero-based device index for device to init
    LPCSTR        uniformityMaskPath,                    ///< [in]  Uniformatity mask path in your computer
    ///<       If the file does not exist or different in path, the DLL makes a new file in path.
    const BOOL	  useUniformityMask,					 ///< [in]  Set device in uniformity mask mode (if set to TRUE)
    const BOOL    asyncOpen,                             ///< [in]  async open device(TRUE) or sync open device(FALSE)
    int           *pHandle                               ///< [out] Function returns device handle to be used for subsequent function calls \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    int	nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_OpenDeviceEx (deviceIndex=%d, asyncOpen=%d)", deviceIndex, asyncOpen);

    if (::IsBadReadPtr(uniformityMaskPath, sizeof(*uniformityMaskPath)) ||
            ::IsBadReadPtr(pHandle, sizeof(int)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_OpenDeviceEx (deviceIndex=%d, asyncOpen=%d)", deviceIndex, asyncOpen);
        return nRc;
    }

    *pHandle = -1;

    FindAllDeviceCount();
    if (GetDeviceCount() <= deviceIndex)
    {
        nRc = IBSU_ERR_DEVICE_NOT_FOUND;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_OpenDeviceEx (deviceIndex=%d, asyncOpen=%d)", deviceIndex, asyncOpen);
        return nRc;
    }

    nRc = _OpenDevice(deviceIndex, uniformityMaskPath, useUniformityMask, asyncOpen, pHandle);

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_OpenDevice (deviceIndex=%d, uniformityMaskPath=%s, asyncOpen=%d, pHandle=%d)", deviceIndex, uniformityMaskPath, asyncOpen, *pHandle);

    return nRc;
}

int CIBScanUltimateApp::DLL_OpenDeviceExW(
    const int	    deviceIndex,                           ///< [in]  Zero-based device index for device to init
    LPCWSTR        uniformityMaskPath,                    ///< [in]  Uniformatity mask path in your computer
    ///<       If the file does not exist or different in path, the DLL makes a new file in path.
    const BOOL useUniformityMask,						 ///< [in]  Set device in uniformity mask mode (if set to TRUE)
    const BOOL    asyncOpen,                             ///< [in]  async open device(TRUE) or sync open device(FALSE)
    int           *pHandle                               ///< [out] Function returns device handle to be used for subsequent function calls \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

#ifdef WINCE
    int	nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_OpenDeviceExW");

    if (::IsBadReadPtr(uniformityMaskPath, sizeof(*uniformityMaskPath)) ||
            ::IsBadReadPtr(pHandle, sizeof(int)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_OpenDeviceExW");
        return nRc;
    }

    *pHandle = -1;

    FindAllDeviceCount();
    if (GetDeviceCount() <= deviceIndex)
    {
        nRc = IBSU_ERR_DEVICE_NOT_FOUND;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_OpenDeviceExW (deviceIndex=%d, asyncOpen=%d)", deviceIndex, asyncOpen);
        return nRc;
    }

	char szUniformityMaskPath[MAX_PATH];

	int nBufferLength = wcstombs( NULL, uniformityMaskPath, MAX_PATH );
	wcstombs( szUniformityMaskPath, uniformityMaskPath, nBufferLength+1 );
	*(szUniformityMaskPath+MAX_PATH-1) = '\0'; // Ensure string is always null terminated

    nRc = _OpenDevice(deviceIndex, szUniformityMaskPath, useUniformityMask, asyncOpen, pHandle);

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_OpenDeviceExW");

    return nRc;
#else
	return IBSU_ERR_NOT_SUPPORTED;
#endif
}

int CIBScanUltimateApp::DLL_GetIBSM_ResultImageInfo(
    const int		        handle,                         ///< [in]  Device handle
    IBSM_FingerPosition   fingerPosition,                 ///< [in]  Finger position. e.g Right Thumb, Right Index finger
    IBSM_ImageData        *pResultImage,                  ///< [out] Result image
    IBSM_ImageData        *pSplitResultImage,             ///< [out] Splited image from Result image
    int                   *pSplitResultImageCount         ///< [out] Splited image count (e.g Two-fingers is 2)
)
{
//	CThreadSync Sync;
	
    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetIBSM_ResultImageInfo (handle=%d)", handle);

    if (::IsBadReadPtr(pResultImage, sizeof(*pResultImage)) ||
            ::IsBadReadPtr(pSplitResultImage, sizeof(*pSplitResultImage)) ||
            ::IsBadReadPtr(pSplitResultImageCount, sizeof(*pSplitResultImageCount)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetIBSM_ResultImageInfo (handle=%d)", handle);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_GetIBSM_ResultImageInfo(fingerPosition, pResultImage, pSplitResultImage, pSplitResultImageCount);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetIBSM_ResultImageInfo (handle=%d, fingerPosition=%d, pResultImage=0x%x, pSplitResultImage=0x%x, pSplitResultImageCount=%d)",
                          handle, fingerPosition, pSplitResultImage, pSplitResultImage, *pSplitResultImageCount);

    return nRc;
}

int CIBScanUltimateApp::DLL_GetNFIQScore(
    const int		        handle,                         ///< [in]  Device handle
    const BYTE            *imgBuffer,                     ///< [in]  Point to image data
    const DWORD           width,                          ///< [in]  Image width
    const DWORD           height,                         ///< [in]  Image height
    const BYTE            bitsPerPixel,                   ///< [in]  Number of Bits per pixel
    int                   *pScore                         ///< [out] NFIQ score
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetNFIQScore (handle=%d)", handle);

    if (::IsBadReadPtr(pScore, sizeof(*pScore)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetNFIQScore (handle=%d)", handle);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    *pScore = -1;
    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_GetNFIQScore(imgBuffer, width, height, bitsPerPixel, pScore);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetNFIQScore (handle=%d, *pScore=%d)",
                          handle, *pScore);

    return nRc;
}

int CIBScanUltimateApp::DLL_GenerateZoomOutImageEx(
    const BYTE            *pInImage,                     ///< [in]  Original image
    const int             inWidth,                       ///< [in]  Width of original image
    const int             inHeight,                      ///< [in]  Height of original image
    BYTE                  *outImage,                     ///< [out] Pointer to zoom-out image data buffer \n
    ///<       Memory must be provided by caller
    const int             outWidth,                      ///< [in]  Width for zoom-out image
    const int             outHeight,                     ///< [in]  Height for zoom-out image
    const BYTE            bkColor                        ///< [in]  Background color for remain area from zoom-out image
)
{
//	CThreadSync Sync;

    int				nRc = IBSU_STATUS_OK;
    IBSU_ImageData	inImage;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GenerateZoomOutImageEx (pInImage=%x, inWidth=%d, inHeight=%d, outWidth=%d, outHeight=%d, bkColor=%d)",
                          pInImage, inWidth, inHeight, outWidth, outHeight, bkColor);

    if (::IsBadReadPtr(pInImage, sizeof(pInImage)) ||
            ::IsBadReadPtr(outImage, sizeof(outWidth * outHeight)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit IBSU_GenerateZoomOutImageEx (pInImage=%x, inWidth=%d, inHeight=%d,, outWidth=%d, outHeight=%d, bkColor=%d)",
                              pInImage, inWidth, inHeight, outWidth, outHeight, bkColor);

        return nRc;
    }

    inImage.Buffer = (void *)pInImage;
    inImage.Width = inWidth;
    inImage.Height = inHeight;
    if (nRc == IBSU_STATUS_OK)
    {
        nRc = Main_GenerateZoomOutImage(inImage, outImage, outWidth, outHeight, bkColor);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GenerateZoomOutImageEx (pInImage=%x, inWidth=%d, inHeight=%d, outImage=%x, outWidth=%d, outHeight=%d, bkColor=%d)",
                          pInImage, inWidth, inHeight, outImage, outWidth, outHeight, bkColor);

    return nRc;
}

int CIBScanUltimateApp::DLL_ReleaseCallbacks(
      const int		       handle,                      ///< [in] Device handle obtained by IBSU_DeviceOpen()
      const IBSU_Events    events                       ///< [in] Enum value to the notification function 
    )
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_ReleaseCallbacks (handle=%d, events=%d)", handle, events);

	if( events != ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT &&
		events != ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE &&
		events != ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS )
	{
		pIBUsbManager = FindUsbDeviceInList(handle);
		nRc = _IsValidUsbHandle(pIBUsbManager);
		if( nRc != IBSU_STATUS_OK )
		{
			InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
				"Exit  IBSU_ReleaseCallbacks (handle=%d, events=%d)", handle, events);
			return nRc;
		}
	}

    nRc = _RegisterCallbacks(pIBUsbManager, events, NULL, NULL);

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_ReleaseCallbacks (handle=%d, events=%d)", handle, events);

	return nRc;
}

int CIBScanUltimateApp::DLL_SaveBitmapMem(
	  const BYTE                *inImage,                  ///< [in]  Point to image data (Gray scale image)
      const DWORD               inWidth,                   ///< [in]  Image width
      const DWORD               inHeight,                  ///< [in]  Image height
      const int                 inPitch,                   ///< [in]  Image line pitch (in Bytes).\n
                                                           ///<       Positive values indicate top down line order,
                                                           ///<       Negative values mean bottom up line order
      const double              inResX,                    ///< [in]  Image horizontal resolution (in PPI)
      const double              inResY,                    ///< [in]  Image vertical resolution (in PPI)
      BYTE                      *outBitmapBuffer,          ///< [out] Pointer to output image data buffer which is set image format and zoom-out factor \n
                                                           ///<       Memory must be provided by caller
      const IBSU_ImageFormat    outImageFormat,            ///< [in]  Set Image color format for output image
	  const DWORD               outWidth,                  ///< [in]  Width for zoom-out image
	  const DWORD               outHeight,                 ///< [in]  Height for zoom-out image
      const BYTE                bkColor                    ///< [in]  Background color for remain area from zoom-out image
    )
{
//	CThreadSync Sync;

	int				nRc = IBSU_STATUS_OK;
	IBSU_ImageData	inImageData;
    int             factor;
    int             expectedBufferLen;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_SaveBitmapMem (inImage=%x, inWidth=%d, inHeight=%d, inPitch=%d, " \
        "inResX=%0.1f, inResY=%0.1f, outImageFormat=%d, outWidth=%d, outHeight=%d, bkColor=%d)",
		inImage, inWidth, inHeight, inPitch, inResX, inResY, outImageFormat, outWidth, outHeight, bkColor);

    // @ref IBSU_IMG_FORMAT_GRAY, @ref IBSU_BMP_GRAY_HEADER_LEN + outWidth * outHeight bytes
    // @ref IBSU_IMG_FORMAT_RGB24, @ref IBSU_BMP_RGB24_HEADER_LEN + 3 * outWidth * outHeight bytes
    // @ref IBSU_IMG_FORMAT_RGB32,  @ref IBSU_BMP_RGB32_HEADER_LEN + 4 * outWidth * outHeight bytes
    switch( outImageFormat )
    {
    case IBSU_IMG_FORMAT_GRAY:
        factor = 1;
        expectedBufferLen = IBSU_BMP_GRAY_HEADER_LEN + outWidth * outHeight;
        break;
    case IBSU_IMG_FORMAT_RGB24:
        factor = 3;
        expectedBufferLen = IBSU_BMP_RGB24_HEADER_LEN + factor * outWidth * outHeight;
        break;
    case IBSU_IMG_FORMAT_RGB32:
        factor = 4;
        expectedBufferLen = IBSU_BMP_RGB32_HEADER_LEN + factor * outWidth * outHeight;
        break;
    default:
        nRc = IBSU_ERR_INVALID_PARAM_VALUE;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		    "Exit IBSU_SaveBitmapMem (inImage=%x, inWidth=%d, inHeight=%d, inPitch=%d, " \
            "inResX=%0.1f, inResY=%0.1f, outImageFormat=%d, outWidth=%d, outHeight=%d, bkColor=%d)",
		    inImage, inWidth, inHeight, inPitch, inResX, inResY, outImageFormat, outWidth, outHeight, bkColor);

		return nRc;
    }

    if( ::IsBadReadPtr( inImage, sizeof(inImage)) ||
		::IsBadReadPtr( outBitmapBuffer, expectedBufferLen) )
	{
		nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		    "Exit IBSU_SaveBitmapMem (inImage=%x, inWidth=%d, inHeight=%d, inPitch=%d, " \
            "inResX=%0.1f, inResY=%0.1f, outImageFormat=%d, outWidth=%d, outHeight=%d, bkColor=%d)",
		    inImage, inWidth, inHeight, inPitch, inResX, inResY, outImageFormat, outWidth, outHeight, bkColor);

		return nRc;
	}

	inImageData.Buffer = (void*)inImage;
	inImageData.Width = inWidth;
	inImageData.Height = inHeight;
    inImageData.Pitch = inPitch;
    inImageData.ResolutionX = inResX;
    inImageData.ResolutionY = inResY;

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = Main_SaveBitmapMem(inImageData, outBitmapBuffer, outWidth, outHeight, bkColor, factor);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
	    "Exit IBSU_SaveBitmapMem (inImage=%x, inWidth=%d, inHeight=%d, inPitch=%d, " \
        "inResX=%0.1f, inResY=%0.1f, outImageFormat=%d, outWidth=%d, outHeight=%d, bkColor=%d)",
	    inImage, inWidth, inHeight, inPitch, inResX, inResY, outImageFormat, outWidth, outHeight, bkColor);

	return nRc;
}

int CIBScanUltimateApp::DLL_ShowOverlayObject(
      const int         handle,                            ///< [in] Device handle obtained by IBSU_OpenDevice()
      const int         overlayHandle,                     ///< [in] Overlay handle obtained by overlay functions
      const BOOL        show                               ///< [in] If TRUE, the overlay will be shown on client window
                                                           ///       If FALSE, the overlay will be hidden on client window
    )
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_ShowOverlayObject (handle=%d, overlayHandle=%d, show=%d)", handle, overlayHandle, show);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_ShowOverlayObject(overlayHandle, show);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_ShowOverlayObject (handle=%d, overlayHandle=%d, show=%d)", handle, overlayHandle, show);

	return nRc;
}

int CIBScanUltimateApp::DLL_ShowAllOverlayObject(
      const int         handle,                            ///< [in] Device handle obtained by IBSU_OpenDevice()
      const BOOL        show                               ///< [in] If TRUE, the overlay will be shown on client window
                                                           ///       If FALSE, the overlay will be hidden on client window
    )
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_ShowAllOverlayObject (handle=%d, show=%d)", handle, show);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_ShowAllOverlayObject(show);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_ShowAllOverlayObject (handle=%d, show=%d)", handle, show);

	return nRc;
}

int CIBScanUltimateApp::DLL_RemoveOverlayObject(
      const int         handle,                            ///< [in] Device handle obtained by IBSU_OpenDevice()
      const int         overlayHandle                      ///< [in] Overlay handle obtained by overlay functions
    )
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_RemoveOverlayObject (handle=%d, overlayHandle=%d)", handle, overlayHandle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_RemoveOverlayObject(overlayHandle);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_RemoveOverlayObject (handle=%d, overlayHandle=%d)", handle, overlayHandle);

	return nRc;
}

int CIBScanUltimateApp::DLL_RemoveAllOverlayObject(
      const int         handle                             ///< [in] Device handle obtained by IBSU_OpenDevice()
    )
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_RemoveAllOverlayObject (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_RemoveAllOverlayObject();
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_RemoveAllOverlayObject (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_AddOverlayText(
      const int         handle,                            ///< [in]  Device handle obtained by IBSU_OpenDevice()
      int               *pOverlayHandle,                   ///< [out] Function returns overlay handle to be used for client windows functions calls \n
                                                           ///<       Memory must be provided by caller
      const char        *fontName,                         ///< [in]  font name for display on window
	  const int         fontSize,                          ///< [in]  font size for display on window
	  const BOOL        fontBold,                          ///< [in]  font bold for display on window
	  const char        *text,                             ///< [in]  text for display on window
	  const int         posX,                              ///< [in]  X coordinate of text for display on window
	  const int         posY,                              ///< [in]  Y coordinate of text for display on window
	  const DWORD       textColor                          ///< [in]  text color for display on window
    )
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_AddOverlayText (handle=%d)", handle);

	if( ::IsBadReadPtr( fontName, sizeof(*fontName)) ||
		::IsBadReadPtr( text, sizeof(*text)) )
	{
		nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit  IBSU_AddOverlayText (handle=%d)", handle);

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_AddOverlayText(pOverlayHandle, fontName, fontSize, 
            fontBold, text, posX, posY, textColor);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_AddOverlayText (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_AddOverlayTextW(
      const int         handle,                            ///< [in]  Device handle obtained by IBSU_OpenDevice()
      int               *pOverlayHandle,                   ///< [out] Function returns overlay handle to be used for client windows functions calls \n
                                                           ///<       Memory must be provided by caller
      const wchar_t     *fontName,                         ///< [in]  font name for display on window
	  const int         fontSize,                          ///< [in]  font size for display on window
	  const BOOL        fontBold,                          ///< [in]  font bold for display on window
	  const wchar_t     *text,                             ///< [in]  text for display on window
	  const int         posX,                              ///< [in]  X coordinate of text for display on window
	  const int         posY,                              ///< [in]  Y coordinate of text for display on window
	  const DWORD       textColor                          ///< [in]  text color for display on window
    )
{
//	CThreadSync Sync;

#ifdef WINCE
	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;
	char            szFontName[IBSU_MAX_STR_LEN];
	char            szText[IBSU_MAX_STR_LEN];
	int				nBufferLength;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_AddOverlayTextW");

	if( ::IsBadReadPtr( fontName, sizeof(*fontName)) ||
		::IsBadReadPtr( text, sizeof(*text)) )
	{
		nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit  IBSU_AddOverlayTextW");

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nBufferLength = wcstombs( NULL, fontName, IBSU_MAX_STR_LEN );
		wcstombs( szFontName, fontName, nBufferLength+1 );
		*(szFontName+IBSU_MAX_STR_LEN-1) = '\0'; // Ensure string is always null terminated
		nBufferLength = wcstombs( NULL, text, IBSU_MAX_STR_LEN );
		wcstombs( szText, text, nBufferLength+1 );
		*(szText+IBSU_MAX_STR_LEN-1) = '\0'; // Ensure string is always null terminated

		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_AddOverlayText(pOverlayHandle, szFontName, fontSize, 
            fontBold, szText, posX, posY, textColor);
	}
	
	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_AddOverlayTextW");

	return nRc;
#else
	return IBSU_ERR_NOT_SUPPORTED;
#endif
}

int CIBScanUltimateApp::DLL_ModifyOverlayText(
      const int         handle,                             ///< [in]  Device handle obtained by IBSU_OpenDevice()
      const int         overlayHandle,                      ///< [in]  Overlay handle to be modified
      const char        *fontName,                          ///< [in]  font name for display on window
	  const int         fontSize,                           ///< [in]  font size for display on window
	  const BOOL        fontBold,                           ///< [in]  font bold for display on window
	  const char        *text,                              ///< [in]  text for display on window
	  const int         posX,                               ///< [in]  X coordinate of text for display on window
	  const int         posY,                               ///< [in]  Y coordinate of text for display on window
	  const DWORD       textColor                           ///< [in]  text color for display on window
    )
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_ModifyOverlayText (handle=%d)", handle);

	if( ::IsBadReadPtr( fontName, sizeof(*fontName)) ||
		::IsBadReadPtr( text, sizeof(*text)) )
	{
		nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit  IBSU_ModifyOverlayText (handle=%d)", handle);

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_ModifyOverlayText(overlayHandle, fontName, fontSize, 
            fontBold, text, posX, posY, textColor);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_ModifyOverlayText (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_ModifyOverlayTextW(
      const int         handle,                             ///< [in]  Device handle obtained by IBSU_OpenDevice()
      const int         overlayHandle,                      ///< [in]  Overlay handle to be modified
      const wchar_t     *fontName,                          ///< [in]  font name for display on window
	  const int         fontSize,                           ///< [in]  font size for display on window
	  const BOOL        fontBold,                           ///< [in]  font bold for display on window
	  const wchar_t     *text,                              ///< [in]  text for display on window
	  const int         posX,                               ///< [in]  X coordinate of text for display on window
	  const int         posY,                               ///< [in]  Y coordinate of text for display on window
	  const DWORD       textColor                           ///< [in]  text color for display on window
    )
{
//	CThreadSync Sync;
#ifdef WINCE
	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;
	char            szFontName[IBSU_MAX_STR_LEN];
	char            szText[IBSU_MAX_STR_LEN];
	int				nBufferLength;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_ModifyOverlayTextW");

	if( ::IsBadReadPtr( fontName, sizeof(*fontName)) ||
		::IsBadReadPtr( text, sizeof(*text)) )
	{
		nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit  IBSU_ModifyOverlayTextW");

		return nRc;
	}
	
	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nBufferLength = wcstombs( NULL, fontName, IBSU_MAX_STR_LEN );
		wcstombs( szFontName, fontName, nBufferLength+1 );
		*(szFontName+IBSU_MAX_STR_LEN-1) = '\0'; // Ensure string is always null terminated
		nBufferLength = wcstombs( NULL, text, IBSU_MAX_STR_LEN );
		wcstombs( szText, text, nBufferLength+1 );
		*(szText+IBSU_MAX_STR_LEN-1) = '\0'; // Ensure string is always null terminated

		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_ModifyOverlayText(overlayHandle, szFontName, fontSize, 
            fontBold, szText, posX, posY, textColor);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_ModifyOverlayTextW");

	return nRc;
#else
	return IBSU_ERR_NOT_SUPPORTED;
#endif
}

int CIBScanUltimateApp::DLL_AddOverlayLine(
      const int         handle,                            ///< [in]  Device handle obtained by IBSU_OpenDevice()
      int               *pOverlayHandle,                   ///< [out] Function returns overlay handle to be used for client windows functions calls \n
                                                           ///<       Memory must be provided by caller
      const int         x1,                                ///< [in]  X coordinate of starting point of line
	  const int         y1,                                ///< [in]  Y coordinate of starting point of line
	  const int         x2,                                ///< [in]  X coordinate of ending point of line
	  const int         y2,                                ///< [in]  Y coordinate of ending point of line
	  const int         lineWidth,                         ///< [in]  line width
	  const DWORD       lineColor                          ///< [in]  line color
    )
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_AddOverlayLine (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_AddOverlayLine(pOverlayHandle, x1, y1,
            x2, y2, lineWidth, lineColor);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_AddOverlayLine (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_ModifyOverlayLine(
      const int         handle,                             ///< [in]  Device handle obtained by IBSU_OpenDevice()
      const int         overlayHandle,                      ///< [in]  Overlay handle to be modified
      const int         x1,                                 ///< [in]  X coordinate of starting point of line
	  const int         y1,                                 ///< [in]  Y coordinate of starting point of line
	  const int         x2,                                 ///< [in]  X coordinate of ending point of line
	  const int         y2,                                 ///< [in]  Y coordinate of ending point of line
	  const int         lineWidth,                          ///< [in]  line width
	  const DWORD       lineColor                           ///< [in]  line color
    )
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_ModifyOverlayLine (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_ModifyOverlayLine(overlayHandle, x1, y1,
            x2, y2, lineWidth, lineColor);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_ModifyOverlayLine (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_AddOverlayQuadrangle(
      const int         handle,                            ///< [in]  Device handle obtained by IBSU_OpenDevice()
      int               *pOverlayHandle,                   ///< [out] Function returns overlay handle to be used for client windows functions calls \n
                                                           ///<       Memory must be provided by caller
      const int         x1,                                ///< [in]  X coordinate of starting point of quadrangle
	  const int         y1,                                ///< [in]  Y coordinate of starting point of quadrangle
	  const int         x2,                                ///< [in]  X coordinate of 1st corner of quadrangle
	  const int         y2,                                ///< [in]  Y coordinate of 1st corner of quadrangle
	  const int         x3,                                ///< [in]  X coordinate of 2nd corner of quadrangle
	  const int         y3,                                ///< [in]  Y coordinate of 2nd corner of quadrangle
	  const int         x4,                                ///< [in]  X coordinate of 3rd corner of quadrangle
	  const int         y4,                                ///< [in]  Y coordinate of 3rd corner of quadrangle
	  const int         lineWidth,                         ///< [in]  line width of quadrangle
	  const DWORD       lineColor                          ///< [in]  line color of quadrangle
    )
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_AddOverlayQuadrangle (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_AddOverlayQuadrangle(pOverlayHandle, x1, y1,
            x2, y2, x3, y3, x4, y4, lineWidth, lineColor);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_AddOverlayQuadrangle (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_ModifyOverlayQuadrangle(
      const int         handle,                             ///< [in]  Device handle obtained by IBSU_OpenDevice()
      const int         overlayHandle,                      ///< [in]  Overlay handle to be modified
      const int         x1,                                 ///< [in]  X coordinate of starting point of quadrangle
	  const int         y1,                                 ///< [in]  Y coordinate of starting point of quadrangle
	  const int         x2,                                 ///< [in]  X coordinate of 1st corner of quadrangle
	  const int         y2,                                 ///< [in]  Y coordinate of 1st corner of quadrangle
	  const int         x3,                                 ///< [in]  X coordinate of 2nd corner of quadrangle
	  const int         y3,                                 ///< [in]  Y coordinate of 2nd corner of quadrangle
	  const int         x4,                                 ///< [in]  X coordinate of 3rd corner of quadrangle
	  const int         y4,                                 ///< [in]  Y coordinate of 3rd corner of quadrangle
	  const int         lineWidth,                          ///< [in]  line width of quadrangle
	  const DWORD       lineColor                           ///< [in]  line color of quadrangle
    )
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_ModifyOverlayQuadrangle (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_ModifyOverlayQuadrangle(overlayHandle, x1, y1,
            x2, y2, x3, y3, x4, y4, lineWidth, lineColor);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_ModifyOverlayQuadrangle (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_AddOverlayShape(
      const int						 handle,                ///< [in]  Device handle obtained by IBSU_OpenDevice()
      int							 *pOverlayHandle,       ///< [out] Function returns overlay handle to be used for client windows functions calls \n
                                                            ///<       Memory must be provided by caller
      const IBSU_OverlayShapePattern shapePattern,          ///< [in]  X coordinate of starting point of the overlay shape
	  const int						 x1,                    ///< [in]  Y coordinate of starting point of the overlay shape
	  const int						 y1,                    ///< [in]  Y coordinate of starting point of the overlay shape
	  const int						 x2,                    ///< [in]  X coordinate of ending point of the overlay shape
	  const int						 y2,                    ///< [in]  Y coordinate of ending point of the overlay shape
	  const int						 lineWidth,             ///< [in]  line width of the overlay shape
	  const DWORD					 lineColor,             ///< [in]  line color of the overlay shape
	  const int						 reserved_1,            ///< [in]  Reserved
	  const int						 reserved_2             ///< [in]  Reserved
	                                                        ///<       If you set shapePattern to ENUM_IBSU_OVERLAY_SHAPE_ARROW
															///<           * reserved_1 can use the width(in pixels) of the full base of the arrowhead
															///<           * reserved_2 can use the angle(in radians) at the arrow tip between the two sides of the arrowhead
    )
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_AddOverlayShape (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_AddOverlayShape(pOverlayHandle, shapePattern,
			x1, y1, x2, y2, lineWidth, lineColor, reserved_1, reserved_2);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_AddOverlayShape (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_ModifyOverlayShape(
      const int						 handle,                ///< [in]  Device handle obtained by IBSU_OpenDevice()
      const int						 overlayHandle,         ///< [in]  Overlay handle to be modified
      const IBSU_OverlayShapePattern shapePattern,          ///< [in]  X coordinate of starting point of the overlay shape
	  const int						 x1,                    ///< [in]  Y coordinate of starting point of the overlay shape
	  const int						 y1,                    ///< [in]  Y coordinate of starting point of the overlay shape
	  const int						 x2,                    ///< [in]  X coordinate of ending point of the overlay shape
	  const int						 y2,                    ///< [in]  Y coordinate of ending point of the overlay shape
	  const int						 lineWidth,             ///< [in]  line width of the overlay shape
	  const DWORD					 lineColor,             ///< [in]  line color of the overlay shape
	  const int						 reserved_1,            ///< [in]  Reserved
	  const int						 reserved_2             ///< [in]  Reserved
	                                                        ///<       If you set shapePattern to ENUM_IBSU_OVERLAY_SHAPE_ARROW
															///<           * reserved_1 can use the width(in pixels) of the full base of the arrowhead
															///<           * reserved_2 can use the angle(in radians) at the arrow tip between the two sides of the arrowhead
    )
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_ModifyOverlayShape (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_ModifyOverlayShape(overlayHandle, shapePattern,
			x1, y1, x2, y2, lineWidth, lineColor, reserved_1, reserved_2);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_ModifyOverlayShape (handle=%d)", handle);

	return nRc;
}

#if defined(__IBSCAN_ULTIMATE_SDK__)
int CIBScanUltimateApp::DLL_WSQEncodeMem(
    const BYTE            *image,
    const int             width,
    const int             height,
    const int             pitch,
    const BYTE            bitsPerPixel,
	const int             pixelPerInch,
	const double          bitRate,
	const char            *commentText,
    unsigned char         **compressedData,
	int                   *compressedLength
)
{
//	CThreadSync Sync;

    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_WSQEncodeMem");

    if (::IsBadReadPtr(image, width * height) ||
//		::IsBadReadPtr(commentText, sizeof(*commentText)) ||
//		::IsBadReadPtr(compressedData, sizeof(*compressedData)) ||
		::IsBadReadPtr(compressedLength, sizeof(*compressedLength)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_WSQEncodeMem");

        return nRc;
    }

	unsigned char *pixels_buffer = new unsigned char [width*height];
	memcpy(pixels_buffer, image, width*height);

	if (pitch < 0)
	{
		_ImageFlipVertically((unsigned char*)pixels_buffer, width, height);
	}

	unsigned char *pCompressedData;
	int rc = wsq_encode_mem(&pCompressedData, compressedLength, (const float)bitRate,
							(unsigned char*)pixels_buffer, width, height, bitsPerPixel, pixelPerInch, (char*)commentText);
	delete [] pixels_buffer;

	if (rc == 0)
	{
		AllocatedMemory *pAllocatedMemory = new AllocatedMemory;
		*compressedData = new unsigned char [*compressedLength];
		memcpy(*compressedData, pCompressedData, *compressedLength);
		free(pCompressedData);

		pAllocatedMemory->memblock	= *compressedData;
		pAllocatedMemory->memblock_size	= *compressedLength;
		{
			CThreadSync Sync;

			m_pListAllocatedMemory.push_back(pAllocatedMemory);
		}
	}
	else
	{
		nRc = IBSU_ERR_NBIS_WSQ_ENCODE_FAILED;
	}

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_WSQEncodeMem (*compressedLength=%d)",
                          *compressedLength);

    return nRc;
}

int CIBScanUltimateApp::DLL_WSQEncodeToFile(
    LPCSTR                filePath,
	const BYTE            *image,
    const int             width,
    const int             height,
    const int             pitch,
    const BYTE            bitsPerPixel,
	const int             pixelPerInch,
	const double          bitRate,
	const char            *commentText
)
{
//	CThreadSync Sync;

    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_WSQEncodeToFile");

    if (::IsBadReadPtr(filePath, sizeof(*filePath)) ||
		::IsBadReadPtr(image, width * height))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_WSQEncodeToFile");

        return nRc;
    }

	unsigned char *pixels_buffer = new unsigned char [width*height];
	memcpy(pixels_buffer, image, width*height);

	if (pitch < 0)
	{
		_ImageFlipVertically(pixels_buffer, width, height);
	}

	unsigned char *pCompressedData;
	int compressedLength;
	int rc = wsq_encode_mem(&pCompressedData, &compressedLength, (const float)bitRate,
							(unsigned char*)pixels_buffer, width, height, bitsPerPixel, pixelPerInch, (char*)commentText);
	delete [] pixels_buffer;

	if (rc == 0)
	{
		FILE *fp;
		fp = fopen(filePath, "wb");
		if (fp != NULL)
		{
			fwrite(pCompressedData, 1, compressedLength, fp);
			fclose(fp);
		}
		else
		{
			nRc = IBSU_ERR_FILE_OPEN;
		}

		free(pCompressedData);
	}
	else
	{
		nRc = IBSU_ERR_NBIS_WSQ_ENCODE_FAILED;
	}

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_WSQEncodeToFile");

    return nRc;
}

int CIBScanUltimateApp::DLL_WSQEncodeToFileW(
    LPCWSTR               filePath,
	const BYTE            *image,
    const int             width,
    const int             height,
    const int             pitch,
    const BYTE            bitsPerPixel,
	const int             pixelPerInch,
	const double          bitRate,
	const WCHAR           *commentText
)
{
//	CThreadSync Sync;

#ifdef WINCE
    int				nRc = IBSU_STATUS_OK;
	char			szFilePath[MAX_PATH];
	char            szCommentText[IBSU_MAX_STR_LEN];
	int nBufferLength;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_WSQEncodeToFileW");

    if (::IsBadReadPtr(filePath, sizeof(*filePath)) ||
		::IsBadReadPtr(image, width * height))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_WSQEncodeToFileW");

        return nRc;
    }

	nBufferLength = wcstombs( NULL, filePath, MAX_PATH );
	wcstombs( szFilePath, filePath, nBufferLength+1 );
	*(szFilePath+MAX_PATH-1) = '\0'; // Ensure string is always null terminated
	nBufferLength = wcstombs( NULL, commentText, IBSU_MAX_STR_LEN );
	wcstombs( szCommentText, commentText, nBufferLength+1 );
	*(szCommentText+IBSU_MAX_STR_LEN-1) = '\0'; // Ensure string is always null terminated

	nRc = DLL_WSQEncodeToFile( szFilePath, image, width, height, pitch, bitsPerPixel, pixelPerInch, bitRate, szCommentText);

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_WSQEncodeToFileW");

	return nRc;
#else
	return IBSU_ERR_NOT_SUPPORTED;
#endif
}

int CIBScanUltimateApp::DLL_WSQDecodeMem(
    const BYTE            *compressedImage,
	const int             compressedLength,
    unsigned char         **decompressedImage,
	int                   *outWidth,
	int                   *outHeight,
	int                   *outPitch,
	int                   *outBitsPerPixel,
	int                   *outPixelPerInch
)
{
//	CThreadSync Sync;

    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_WSQDecodeMem");

    if (::IsBadReadPtr(compressedImage, compressedLength) ||
//		::IsBadReadPtr(decompressedImage, sizeof(*decompressedImage)) ||
		::IsBadReadPtr(outWidth, sizeof(*outWidth)) ||
		::IsBadReadPtr(outHeight, sizeof(*outHeight)) ||
		::IsBadReadPtr(outPitch, sizeof(*outPitch)) ||
		::IsBadReadPtr(outBitsPerPixel, sizeof(*outBitsPerPixel)) ||
		::IsBadReadPtr(outPixelPerInch, sizeof(*outPixelPerInch)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_WSQDecodeMem");

        return nRc;
    }

	unsigned char *pDecompressData;
	int decompressSize = 0;
    int lossyFlag;
    int rc = wsq_decode_mem(&pDecompressData, outWidth, outHeight, outBitsPerPixel, outPixelPerInch,
                            &lossyFlag, (unsigned char *)compressedImage, compressedLength);
	if (rc == 0)
	{
		AllocatedMemory *pAllocatedMemory = new AllocatedMemory;
		decompressSize = *outWidth * *outHeight;
        *decompressedImage = new unsigned char [decompressSize];
        memcpy(*decompressedImage, pDecompressData, decompressSize);
		// Image flip vertically
		_ImageFlipVertically(*decompressedImage, *outWidth, *outHeight);
		*outPitch = -*outWidth;
		free(pDecompressData);

		pAllocatedMemory->memblock	= *decompressedImage;
		pAllocatedMemory->memblock_size	= decompressSize;
		{
			CThreadSync Sync;

			m_pListAllocatedMemory.push_back(pAllocatedMemory);
		}
	}
	else
	{
		nRc = IBSU_ERR_NBIS_WSQ_DECODE_FAILED;
	}

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_WSQDecodeMem (*outWidth=%d, *outHeight=%d, *outBitsPerPixel=%d, *outPixelPerInch=%d)",
                          *outWidth, *outHeight, *outBitsPerPixel, *outPixelPerInch);

    return nRc;
}

int CIBScanUltimateApp::DLL_WSQDecodeFromFile(
    LPCSTR                filePath,
    unsigned char         **decompressedImage,
	int                   *outWidth,
	int                   *outHeight,
	int                   *outPitch,
	int                   *outBitsPerPixel,
	int                   *outPixelPerInch
)
{
//	CThreadSync Sync;

    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_WSQDecodeFromFile");

    if (::IsBadReadPtr(filePath, sizeof(*filePath)) ||
//		::IsBadReadPtr(decompressedImage, sizeof(*decompressedImage)) ||
		::IsBadReadPtr(outWidth, sizeof(*outWidth)) ||
		::IsBadReadPtr(outHeight, sizeof(*outHeight)) ||
		::IsBadReadPtr(outPitch, sizeof(*outPitch)) ||
		::IsBadReadPtr(outBitsPerPixel, sizeof(*outBitsPerPixel)) ||
		::IsBadReadPtr(outPixelPerInch, sizeof(*outPixelPerInch)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_WSQDecodeFromFile");

        return nRc;
    }

	BYTE *compressedImage;
	int compressedLength;
	FILE *fp;
	fp = fopen(filePath, "rb");
    if (fp != NULL)
    {
		// Get file size
		fseek(fp, 0, SEEK_END);
		compressedLength = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		compressedImage = new BYTE[compressedLength];
        size_t readSize = fread(compressedImage, 1, compressedLength, fp);
        fclose(fp);
		if ((int)readSize != compressedLength)
		{
			nRc = IBSU_ERR_FILE_OPEN;
		}
    }
	else
	{
		nRc = IBSU_ERR_FILE_OPEN;
	}

	if (nRc != IBSU_STATUS_OK)
	{
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
							  "Exit  IBSU_WSQDecodeFromFile");
		return nRc;
	}

	unsigned char *pDecompressData;
	int decompressSize = 0;
    int lossyFlag;
    int rc = wsq_decode_mem(&pDecompressData, outWidth, outHeight, outBitsPerPixel, outPixelPerInch,
                            &lossyFlag, (unsigned char *)compressedImage, compressedLength);
	delete compressedImage;

	if (rc == 0)
	{
		AllocatedMemory *pAllocatedMemory = new AllocatedMemory;
		decompressSize = *outWidth * *outHeight;
        *decompressedImage = new unsigned char [decompressSize];
        memcpy(*decompressedImage, pDecompressData, decompressSize);
		// Image flip vertically
		_ImageFlipVertically(*decompressedImage, *outWidth, *outHeight);
		*outPitch = -*outWidth;
		free(pDecompressData);

		pAllocatedMemory->memblock	= *decompressedImage;
		pAllocatedMemory->memblock_size	= decompressSize;
		{
			CThreadSync Sync;

			m_pListAllocatedMemory.push_back(pAllocatedMemory);
		}
	}
	else
	{
		nRc = IBSU_ERR_NBIS_WSQ_DECODE_FAILED;
	}

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_WSQDecodeFromFile (*outWidth=%d, *outHeight=%d, *outBitsPerPixel=%d, *outPixelPerInch=%d)",
                          *outWidth, *outHeight, *outBitsPerPixel, *outPixelPerInch);

    return nRc;
}

int CIBScanUltimateApp::DLL_WSQDecodeFromFileW(
    LPCWSTR               filePath,
    unsigned char         **decompressedImage,
	int                   *outWidth,
	int                   *outHeight,
	int                   *outPitch,
	int                   *outBitsPerPixel,
	int                   *outPixelPerInch
)
{
//	CThreadSync Sync;

#ifdef WINCE
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_WSQDecodeFromFileW");

    if (::IsBadReadPtr(filePath, sizeof(*filePath)) ||
//		::IsBadReadPtr(decompressedImage, sizeof(*decompressedImage)) ||
		::IsBadReadPtr(outWidth, sizeof(*outWidth)) ||
		::IsBadReadPtr(outHeight, sizeof(*outHeight)) ||
		::IsBadReadPtr(outPitch, sizeof(*outPitch)) ||
		::IsBadReadPtr(outBitsPerPixel, sizeof(*outBitsPerPixel)) ||
		::IsBadReadPtr(outPixelPerInch, sizeof(*outPixelPerInch)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_WSQDecodeFromFileW");

        return nRc;
    }

	char szFilePath[MAX_PATH];

	int nBufferLength = wcstombs( NULL, filePath, MAX_PATH );
	wcstombs( szFilePath, filePath, nBufferLength+1 );
	*(szFilePath+MAX_PATH-1) = '\0'; // Ensure string is always null terminated

	nRc = DLL_WSQDecodeFromFile(szFilePath, decompressedImage, outWidth, outHeight, outPitch, outBitsPerPixel, outPixelPerInch);

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_WSQDecodeFromFileW");

    return nRc;
#else
	return IBSU_ERR_NOT_SUPPORTED;
#endif
}
#endif

int CIBScanUltimateApp::DLL_FreeMemory(
    void		          *memblock
)
{
    AllocatedMemory	*pAllocatedMemory = NULL;
    int nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_FreeMemory ()");

    if (::IsBadReadPtr(memblock, sizeof(int)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_FreeMemory ()");

        return nRc;
    }

	pAllocatedMemory = FindMemblockInList(memblock);
	if (pAllocatedMemory == NULL)
	{
		nRc = IBSU_ERR_MEM_ALLOC;
	}
	else
	{
		DeleteMemblockInList(memblock);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_FreeMemory ()");

    return nRc;
}

#if defined(__IBSCAN_ULTIMATE_SDK__)
int CIBScanUltimateApp::DLL_SavePngImage
    (LPCSTR       filePath,
     const BYTE  *image,
     const DWORD  width,
     const DWORD  height,
     const int    pitch,
     const double resX,
     const double resY
)
{
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SavePngImage");

#if defined(WINCE)
    nRc = IBSU_ERR_NOT_SUPPORTED;
    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SavePngImage");
    return nRc;
#else
    if (::IsBadReadPtr(filePath, sizeof(*filePath)) ||
		::IsBadReadPtr(image, width * height))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_SavePngImage");

        return nRc;
    }

	unsigned char *tmpImage = new unsigned char [width*height+1078];
    nRc = _SaveBitmapMem((BYTE*)image, tmpImage, width, height, pitch, resX, resY, 8/*8bit grayscale*/);

	if (nRc == IBSU_STATUS_OK)
	{
		CxImage image2((BYTE*)tmpImage, width*height+1078, CXIMAGE_FORMAT_BMP);
		if (image2.IsValid())
		{
			TCHAR szFilePath[MAX_PATH];
#ifdef UNICODE
			mbstowcs(szFilePath, filePath, MAX_PATH);
#else
			strcpy(szFilePath, filePath);
#endif
			int bRet = image2.Save(filePath, CXIMAGE_FORMAT_PNG);
			if (bRet)
			{
				nRc = IBSU_STATUS_OK;
			}
		}

		image2.Destroy();
	}

	delete [] tmpImage;

	if (nRc != IBSU_STATUS_OK)
	{
		nRc = IBSU_ERR_NBIS_PNG_ENCODE_FAILED;
	}

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SavePngImage");

    return nRc;
#endif
}

int CIBScanUltimateApp::DLL_SavePngImageW
    (LPCWSTR       filePath,
     const BYTE  *image,
     const DWORD  width,
     const DWORD  height,
     const int    pitch,
     const double resX,
     const double resY
)
{
#ifdef WINCE
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SavePngImageW");

#if defined(WINCE)
    nRc = IBSU_ERR_NOT_SUPPORTED;
    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SavePngImageW");
#else
	char            szFilePath[MAX_PATH];
    if (::IsBadReadPtr(filePath, sizeof(*filePath)) ||
		::IsBadReadPtr(image, width * height))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_SavePngImageW");

        return nRc;
    }

	int nBufferLength = wcstombs( NULL, filePath, MAX_PATH );
	wcstombs( szFilePath, filePath, nBufferLength+1 );
	*(szFilePath+MAX_PATH-1) = '\0'; // Ensure string is always null terminated

	nRc = DLL_SavePngImage( szFilePath, image, width, height, pitch, resX, resY );

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SavePngImageW");
#endif
	
	return nRc;
#else
	return IBSU_ERR_NOT_SUPPORTED;
#endif
}

int CIBScanUltimateApp::DLL_SaveJP2Image
    (LPCSTR                filePath,
     const BYTE           *image,
     const DWORD           width,
     const DWORD           height,
     const int             pitch,
     const double          resX,
     const double          resY,
	 const int             fQuality
)
{
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SaveJP2Image");

#if defined(WINCE)
    nRc = IBSU_ERR_NOT_SUPPORTED;
    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SaveJP2Image");
    return nRc;
#else
    if (::IsBadReadPtr(filePath, sizeof(*filePath)) ||
		::IsBadReadPtr(image, width * height))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_SaveJP2Image");

        return nRc;
    }

	if (fQuality < 0 || fQuality > 100)
	{
		nRc = IBSU_ERR_INVALID_PARAM_VALUE;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_SaveJP2Image");

        return nRc;
	}

	unsigned char *tmpImage = new unsigned char [width*height+1078];
    nRc = _SaveBitmapMem((BYTE*)image, tmpImage, width, height, pitch, resX, resY, 8/*8bit grayscale*/);

	if (nRc == IBSU_STATUS_OK)
	{
		CxImage image2((BYTE*)tmpImage, width*height+1078, CXIMAGE_FORMAT_BMP);
		if (image2.IsValid())
		{
			TCHAR szFilePath[MAX_PATH];
			image2.SetJpegQuality(fQuality);
#ifdef UNICODE
			mbstowcs(szFilePath, filePath, MAX_PATH);
#else
			strcpy(szFilePath, filePath);
#endif
			image2.SetJpegQuality(fQuality);
			int bRet = image2.Save(filePath, CXIMAGE_FORMAT_JP2);
			if (bRet)
			{
				nRc = IBSU_STATUS_OK;
			}
		}
		image2.Destroy();
	}

	delete [] tmpImage;

	if (nRc != IBSU_STATUS_OK)
	{
		nRc = IBSU_ERR_NBIS_JP2_ENCODE_FAILED;
	}

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SaveJP2Image");

    return nRc;
#endif
}

int CIBScanUltimateApp::DLL_SaveJP2ImageW
    (LPCWSTR               filePath,
     const BYTE           *image,
     const DWORD           width,
     const DWORD           height,
     const int             pitch,
     const double          resX,
     const double          resY,
	 const int             fQuality
)
{
#ifdef WINCE
    int				nRc = IBSU_STATUS_OK;

#if defined(WINCE)
    nRc = IBSU_ERR_NOT_SUPPORTED;
    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SaveJP2ImageW");
#else
	char            szFilePath[MAX_PATH];
    if (::IsBadReadPtr(filePath, sizeof(*filePath)) ||
		::IsBadReadPtr(image, width * height))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_SaveJP2ImageW");

        return nRc;
    }

	int nBufferLength = wcstombs( NULL, filePath, MAX_PATH );
	wcstombs( szFilePath, filePath, nBufferLength+1 );
	*(szFilePath+MAX_PATH-1) = '\0'; // Ensure string is always null terminated

	nRc = DLL_SaveJP2Image( szFilePath, image, width, height, pitch, resX, resY, fQuality );

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SaveJP2ImageW");
#endif

    return nRc;
#else
	return IBSU_ERR_NOT_SUPPORTED;
#endif
}
#endif

int CIBScanUltimateApp::DLL_RedrawClientWindow(
    const int   handle,
	const DWORD flags
)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_RedrawClientWindow (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->ClientWindow_Redraw(flags);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_RedrawClientWindow (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_UnloadLibrary()
{
//	CThreadSync Sync;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_UnloadLibrary ()");

	if (m_bOnloadLibrary)
	{
		Close();
	}
	else
	{
		nRc = IBSU_ERR_LIBRARY_UNLOAD_FAILED;
	}

//	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
//		"Exit  IBSU_UnloadLibrary ()");

	return nRc;
}

int	CIBScanUltimateApp::DLL_CombineImage(
	const IBSU_ImageData InImage1,
	const IBSU_ImageData InImage2,
	IBSU_CombineImageWhichHand WhichHand, 
	IBSU_ImageData *OutImage)
{
//	CThreadSync Sync;

	int	nRc = IBSU_STATUS_OK;
    unsigned int i, j;
	const IBSU_ImageData *Buffer1 = NULL;
	const IBSU_ImageData *Buffer2 = NULL;
	unsigned char *pBuffer1 = NULL, *pBuffer2 = NULL;
	int plus_height1=0, plus_height2=0;

	if( InImage1.Width != 800 || InImage1.Height != 750 || 
		InImage2.Width != 800 || InImage2.Height != 750 )
		return IBSU_ERR_INVALID_PARAM_VALUE;

	if(WhichHand == ENUM_IBSU_COMBINE_IMAGE_LEFT_HAND)
	{
		Buffer1 = &InImage2;
		Buffer2 = &InImage1;
		plus_height1 = 1500/8;
		plus_height2 = 1500/4;
	}
	else if(WhichHand == ENUM_IBSU_COMBINE_IMAGE_RIGHT_HAND)
	{
		Buffer1 = &InImage1;
		Buffer2 = &InImage2;
		plus_height1 = 1500/4;
		plus_height2 = 1500/8;
	}
	else
		return IBSU_ERR_INVALID_PARAM_VALUE;

	pBuffer1 = (unsigned char*)Buffer1->Buffer;
	pBuffer2 = (unsigned char*)Buffer2->Buffer;

	OutImage->BitsPerPixel = Buffer1->BitsPerPixel;
	OutImage->Format = Buffer1->Format; 
	OutImage->FrameTime = Buffer1->FrameTime;
	OutImage->IsFinal = Buffer1->IsFinal;
	OutImage->Pitch = Buffer1->Pitch + Buffer2->Pitch;
	OutImage->ResolutionX = Buffer1->ResolutionX;
	OutImage->ResolutionY = Buffer1->ResolutionY;
	OutImage->Width = 1600;//Buffer1->Width + Buffer2->Width;
	OutImage->Height = 1500;//Buffer1->Height;

    if (!m_pCombine_Image)
    {
        m_pCombine_Image = new BYTE [__MAX_IMG_WIDTH__ * __MAX_IMG_HEIGHT__];
    }

	OutImage->Buffer = m_pCombine_Image;

    memset(m_pCombine_Image, 255, __MAX_IMG_WIDTH__*__MAX_IMG_HEIGHT__);

	for(i=0; i<Buffer1->Height; i++)
	{
		for(j=0; j<Buffer1->Width; j++)
		{
			m_pCombine_Image[(i+plus_height1)*OutImage->Width+j] = pBuffer1[i*Buffer1->Width+j];
		}
	} 

	for(i=0; i<Buffer2->Height; i++)
	{
		for(j=0; j<Buffer2->Width; j++)
		{
			m_pCombine_Image[(i+plus_height2)*OutImage->Width+j+Buffer1->Width] = pBuffer2[i*Buffer2->Width+j];
		}
	}

	return nRc;
}

int	CIBScanUltimateApp::DLL_CombineImageEx(
	const IBSU_ImageData InImage1,
	const IBSU_ImageData InImage2,
	IBSU_CombineImageWhichHand WhichHand, 
	IBSU_ImageData *OutImage,
	IBSU_ImageData  *pSegmentImageArray,
	IBSU_SegmentPosition *pSegmentPositionArray,
	int *pSegmentImageArrayCount)
{
//	CThreadSync Sync;

	int	i, imageidx, nRc = IBSU_STATUS_OK;
	int total_segment_cnt = 0;
	int x1, y1, x2, y2, x3, y3, x4, y4;
   	int offset_x=0;
	int offset_y=0;

	*pSegmentImageArrayCount = 0;

	nRc = DLL_CombineImage(InImage1, InImage2, WhichHand, OutImage);

	if(nRc != IBSU_STATUS_OK)
		return nRc;

	PropertyInfo		m_propertyInfo;
	UsbDeviceInfo		m_UsbDeviceInfo;

	memset(&m_propertyInfo, 0, sizeof(PropertyInfo));
	memset(&m_UsbDeviceInfo, 0, sizeof(UsbDeviceInfo));

	m_UsbDeviceInfo.devType = DEVICE_TYPE_WATSON;
	m_UsbDeviceInfo.ImgWidth = 800;
	m_UsbDeviceInfo.ImgHeight = 750;
	m_UsbDeviceInfo.ImgSize = m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight;
	m_UsbDeviceInfo.ImgWidth_Roll = 800;
	m_UsbDeviceInfo.ImgHeight_Roll = 750;
	m_UsbDeviceInfo.nZoomOut = 5;
	m_UsbDeviceInfo.ImgSize_Roll = m_UsbDeviceInfo.ImgWidth_Roll*m_UsbDeviceInfo.ImgHeight_Roll;
	
	m_propertyInfo.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
	m_propertyInfo.nNumberOfObjects = 2;

	CIBAlgorithm *m_pAlgo = new CIBAlgorithm(&m_UsbDeviceInfo, &m_propertyInfo);
	
	unsigned char *Buffer1 = new unsigned char [m_UsbDeviceInfo.ImgSize];
	unsigned char *Buffer2 = new unsigned char [m_UsbDeviceInfo.ImgSize];
	unsigned char *pBuffer = Buffer1;

	// invert image
	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite((unsigned char*)InImage1.Buffer, Buffer1, m_UsbDeviceInfo.ImgSize);
	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite((unsigned char*)InImage2.Buffer, Buffer2, m_UsbDeviceInfo.ImgSize);

	offset_x = 0;
	offset_y = 0;
	for(imageidx=0; imageidx<2; imageidx++)
	{
		if(imageidx == 0)
		{
			if(WhichHand == ENUM_IBSU_COMBINE_IMAGE_LEFT_HAND)
			{
				pBuffer = Buffer2;
				offset_x = 0;
				offset_y = (750 - 1500/8);//563;
			}
			else
			{
				pBuffer = Buffer1;
				offset_x = 0;
				offset_y = (750 - 1500/4);//1500/4;
			}
		}
		else
		{
			if(WhichHand == ENUM_IBSU_COMBINE_IMAGE_LEFT_HAND)
			{
				pBuffer = Buffer1;
				offset_x = 800;
				offset_y = (750 - 1500/4);//1500/4;
			}
			else
			{
				pBuffer = Buffer2;
				offset_x = 800;
				offset_y = (750 - 1500/8);//563;//1500/2;
			}
		}

		m_pAlgo->_Algo_SegmentFinger(pBuffer);
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitResultArray, m_pAlgo->m_OutSplitResultArray, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize);
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitResultArrayEx, m_pAlgo->m_OutSplitResultArrayEx, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize);

		for(i=0; i<m_pAlgo->m_segment_arr.SegmentCnt; i++)
		{
			memcpy((unsigned char*)pSegmentArr[total_segment_cnt].Buffer, &m_pAlgo->m_OutSplitResultArrayEx[i*m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight], 
					m_pAlgo->m_OutSplitResultArrayExWidth[i]*m_pAlgo->m_OutSplitResultArrayExHeight[i]);

			pSegmentImageArray[total_segment_cnt].BitsPerPixel = 8;
			pSegmentImageArray[total_segment_cnt].Format = IBSU_IMG_FORMAT_GRAY;
			pSegmentImageArray[total_segment_cnt].FrameTime = 0;
			pSegmentImageArray[total_segment_cnt].Height = m_pAlgo->m_OutSplitResultArrayExHeight[i];
			pSegmentImageArray[total_segment_cnt].IsFinal = TRUE;
			pSegmentImageArray[total_segment_cnt].Pitch = -m_pAlgo->m_OutSplitResultArrayExWidth[i];
			pSegmentImageArray[total_segment_cnt].ProcessThres = 0;
			pSegmentImageArray[total_segment_cnt].ResolutionX = 500;
			pSegmentImageArray[total_segment_cnt].ResolutionY = 500;
			pSegmentImageArray[total_segment_cnt].Width = m_pAlgo->m_OutSplitResultArrayExWidth[i];
			pSegmentImageArray[total_segment_cnt].Buffer = pSegmentArr[total_segment_cnt].Buffer;

			x1 = (m_pAlgo->m_segment_arr.Segment[i].P1_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
			y1 = (m_pAlgo->m_segment_arr.Segment[i].P1_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

			x2 = (m_pAlgo->m_segment_arr.Segment[i].P2_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
			y2 = (m_pAlgo->m_segment_arr.Segment[i].P2_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

			x3 = (m_pAlgo->m_segment_arr.Segment[i].P3_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
			y3 = (m_pAlgo->m_segment_arr.Segment[i].P3_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

			x4 = (m_pAlgo->m_segment_arr.Segment[i].P4_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
			y4 = (m_pAlgo->m_segment_arr.Segment[i].P4_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;
		
			pSegmentPositionArray[total_segment_cnt].x1 = x1 + offset_x;
			pSegmentPositionArray[total_segment_cnt].x2 = x2 + offset_x;
			pSegmentPositionArray[total_segment_cnt].x3 = x3 + offset_x;
			pSegmentPositionArray[total_segment_cnt].x4 = x4 + offset_x;
			pSegmentPositionArray[total_segment_cnt].y1 = (m_pAlgo->IMG_H - y1) + offset_y;
			pSegmentPositionArray[total_segment_cnt].y2 = (m_pAlgo->IMG_H - y2) + offset_y;
			pSegmentPositionArray[total_segment_cnt].y3 = (m_pAlgo->IMG_H - y3) + offset_y;
			pSegmentPositionArray[total_segment_cnt].y4 = (m_pAlgo->IMG_H - y4) + offset_y;

			total_segment_cnt++;
		}	
	}

	*pSegmentImageArrayCount = total_segment_cnt;

	delete [] Buffer1;
	delete [] Buffer2;
	delete m_pAlgo;

	return nRc;
}

int CIBScanUltimateApp::DLL_GetOperableBeeper(
	const int     handle,
    IBSU_BeeperType *pBeeperType
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetOperableBeeper (handle=%d)", handle);

    if (::IsBadReadPtr(pBeeperType, sizeof(pBeeperType)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetOperableBeeper (handle=%d)", handle);
        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    *pBeeperType = ENUM_IBSU_BEEPER_TYPE_NONE;
    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Control_GetAvailableBeeper(pBeeperType);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_GetOperableBeeper (handle=%d, pLedType=%d)",
                          handle, *pBeeperType);

    return nRc;
}

int CIBScanUltimateApp::DLL_SetBeeper(
	const int              handle,
    const IBSU_BeepPattern beepPattern,
	const DWORD            soundTone,
	const DWORD            duration,
    const DWORD            reserved_1,
    const DWORD            reserved_2
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_SetBeeper (handle=%d, beepPattern=%d, soundTone=%d, duration=%d, reserved_1=%d, reserved_2=%d)",
						  handle, beepPattern, soundTone, duration, reserved_1, reserved_2);

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Control_SetBeeper(beepPattern, soundTone, duration, reserved_1, reserved_2);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_SetBeeper (handle=%d, beepPattern=%d, soundTone=%d, duration=%d, reserved_1=%d, reserved_2=%d)",
						  handle, beepPattern, soundTone, duration, reserved_1, reserved_2);

    return nRc;
}

int CIBScanUltimateApp::DLL_CheckWetFinger
    (const int              handle,
	 const IBSU_ImageData   inImage)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(), "Enter IBSU_CheckWetFinger (handle=%d)", handle);

    if (::IsBadReadPtr(inImage.Buffer, sizeof(inImage.Buffer)))
	{
		nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Exit  IBSU_CheckWetFinger (handle=%d)", handle);

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->Capture_CheckWetFinger(inImage);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Exit  IBSU_CheckWetFinger (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_GetImageWidth
    (const int              handle,
	 const IBSU_ImageData  inImage,
	 int				   *Width_MM)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(), "Enter IBSU_GetImageWidth (handle=%d)", handle);

    if (::IsBadReadPtr(inImage.Buffer, sizeof(inImage.Buffer)))
	{
		nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Exit  IBSU_GetImageWidth (handle=%d)", handle);

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->Capture_GetImageWidth((BYTE*)inImage.Buffer, inImage.Width, inImage.Height, Width_MM);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Exit  IBSU_GetImageWidth (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_IsWritableDirectory
	(LPCSTR	           dirpath,
	 const BOOL		   needCreateSubFolder)
{

	int				nRc = IBSU_STATUS_OK;
	char			random_path[MAX_PATH];
	char			random_file[MAX_PATH];

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(), "Enter IBSU_IsWritableDirectory ()");

	if (::IsBadReadPtr(dirpath, sizeof(*dirpath)))
	{
		nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Exit  IBSU_IsWritableDirectory ()");

		return nRc;
	}

    size_t str_length = strlen(dirpath);
#ifdef _WINDOWS
	TCHAR           szDirPath[MAX_PATH];
	TCHAR           szFilePath[MAX_PATH];
    SYSTEMTIME currentTime;
    ::GetLocalTime(&currentTime);
    if (needCreateSubFolder)
    {
        if (dirpath[str_length - 1] == '\\')
        {
	        sprintf(random_path, "%s%04d%02d%02d-%02d%02d%02d", dirpath, 
			        currentTime.wYear, currentTime.wMonth, currentTime.wDay, currentTime.wHour, currentTime.wMinute, currentTime.wSecond);
        }
        else
        {
	        sprintf(random_path, "%s\\%04d%02d%02d-%02d%02d%02d", dirpath, 
			        currentTime.wYear, currentTime.wMonth, currentTime.wDay, currentTime.wHour, currentTime.wMinute, currentTime.wSecond);
        }

#ifdef UNICODE
			mbstowcs(szDirPath, random_path, MAX_PATH);
#else
			strcpy(szDirPath, random_path);
#endif
        CreateDirectory(szDirPath, NULL);
        DWORD dwAttrib = GetFileAttributes(szDirPath);

//        if (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
        if (dwAttrib == FILE_ATTRIBUTE_DIRECTORY)
        {
            nRc = IBSU_STATUS_OK;
        }
        else
        {
            nRc = IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
        }
    }
    else
    {
        if (dirpath[str_length - 1] == '\\')
        {
	        sprintf(random_path, "%s", dirpath);
        }
        else
        {
	        sprintf(random_path, "%s\\", dirpath);
        }
    }

    if (nRc == IBSU_STATUS_OK)
    {
        sprintf(random_file, "%s\\%04d%02d%02d-%02d%02d%02d.txt", random_path, 
			        currentTime.wYear, currentTime.wMonth, currentTime.wDay, currentTime.wHour, currentTime.wMinute, currentTime.wSecond);

#ifdef UNICODE
			mbstowcs(szFilePath, random_file, MAX_PATH);
#else
			strcpy(szFilePath, random_file);
#endif

        FILE *fp = fopen(random_file, "w");
        if (fp == NULL)
        {
            nRc = IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
        }
        else
        {
            fclose(fp);
            DeleteFile(szFilePath);
        }
    }

    if (needCreateSubFolder)
    {
        RemoveDirectory(szDirPath);
    }
#else
    struct timeval timeval;
    gettimeofday(&timeval, 0);
    struct tm *currentTime;
    currentTime = localtime(&timeval.tv_sec);
    if (needCreateSubFolder)
    {
        if (dirpath[str_length - 1] == '/')
        {
            sprintf(random_path, "%s%04d%02d%02d-%02d%02d%02d", dirpath, 
                    currentTime->tm_year + 1900, currentTime->tm_mon + 1, currentTime->tm_mday,
                    currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec);
        }
        else
        {
            sprintf(random_path, "%s/%04d%02d%02d-%02d%02d%02d", dirpath, 
                    currentTime->tm_year + 1900, currentTime->tm_mon + 1, currentTime->tm_mday,
                    currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec);
        }

        mkdir(random_path, 777);
        DIR* dir = opendir(random_path);
        if (dir)
        {
            nRc = IBSU_STATUS_OK;
        }
        else
        {
            nRc = IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
        }
    }
    else
    {
        if (dirpath[str_length - 1] == '\\')
        {
	        sprintf(random_path, "%s", dirpath);
        }
        else
        {
	        sprintf(random_path, "%s\\", dirpath);
        }
    }

    if (nRc == IBSU_STATUS_OK)
    {
        sprintf(random_file, "%s\\%04d%02d%02d-%02d%02d%02d.txt", random_path, 
                currentTime->tm_year + 1900, currentTime->tm_mon + 1, currentTime->tm_mday,
                currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec);

        FILE *fp = fopen(random_file, "w");
        if (fp == NULL)
        {
            nRc = IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
        }
        else
        {
            fclose(fp);
            remove(random_file);
        }
    }

    if (needCreateSubFolder)
    {
        rmdir(random_path);
    }
#endif

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Exit  IBSU_IsWritableDirectory (%s - %d)", random_file, needCreateSubFolder);

	return nRc;
}

int CIBScanUltimateApp::DLL_IsWritableDirectoryW
	(LPCWSTR           dirpath,
	 const BOOL		   needCreateSubFolder)
{
#ifdef WINCE
    int				nRc = IBSU_STATUS_OK;
	char			szDirPath[IBSU_MAX_STR_LEN];

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_IsWritableDirectoryW");

    if (::IsBadReadPtr(dirpath, sizeof(dirpath)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_IsWritableDirectoryW");
        return nRc;
    }

	int nBufferLength = wcstombs( NULL, dirpath, MAX_PATH );
	wcstombs( szDirPath, dirpath, nBufferLength+1 );
	*(szDirPath+MAX_PATH-1) = '\0'; // Ensure string is always null terminated

	nRc = DLL_IsWritableDirectory( szDirPath, needCreateSubFolder );

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  IBSU_IsWritableDirectoryW");

	return nRc;
#else
	return IBSU_ERR_NOT_SUPPORTED;
#endif
}

int CIBScanUltimateApp::DLL_GenerateDisplayImage(
    const BYTE            *pInImage,
    const int             inWidth,
    const int             inHeight,
    BYTE                  *outImage,
    const int             outWidth,
    const int             outHeight,
    const BYTE            bkColor,
	const IBSU_ImageFormat outFormat,
	const int             outQualityLevel,
	const BOOL            verticalFlip
)
{
//	CThreadSync Sync;

    int				nRc = IBSU_STATUS_OK;
    IBSU_ImageData	inImage;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GenerateDisplayImage (pInImage=%x, inWidth=%d, inHeight=%d, outWidth=%d, outHeight=%d, bkColor=%d, outFormat=%d, outQualityLevel=%d, verticalFlip=%d)",
                          pInImage, inWidth, inHeight, outWidth, outHeight, bkColor, outFormat, outQualityLevel, verticalFlip);

    if (::IsBadReadPtr(pInImage, sizeof(pInImage)) ||
            ::IsBadReadPtr(outImage, sizeof(outWidth * outHeight)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
							  "Exit IBSU_GenerateDisplayImage (pInImage=%x, inWidth=%d, inHeight=%d, outWidth=%d, outHeight=%d, bkColor=%d, outFormat=%d, outQualityLevel=%d, verticalFlip=%d)",
							  pInImage, inWidth, inHeight, outWidth, outHeight, bkColor, outFormat, outQualityLevel, verticalFlip);

        return nRc;
    }

	if (outQualityLevel < 0 || outQualityLevel > 2)
	{
		return IBSU_ERR_INVALID_PARAM_VALUE;
	}

    inImage.Buffer = (void *)pInImage;
    inImage.Width = inWidth;
    inImage.Height = inHeight;
    if (nRc == IBSU_STATUS_OK)
    {
		nRc = Main_GenerateDisplayImage(inImage, outImage, outWidth, outHeight, bkColor,
            outFormat, outQualityLevel, verticalFlip);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
						  "Exit IBSU_GenerateDisplayImage (pInImage=%x, inWidth=%d, inHeight=%d, outWidth=%d, outHeight=%d, bkColor=%d, outFormat=%d, outQualityLevel=%d, verticalFlip=%d)",
						  pInImage, inWidth, inHeight, outWidth, outHeight, bkColor, outFormat, outQualityLevel, verticalFlip);

    return nRc;
}

#if defined(__IBSCAN_SDK__)
int CIBScanUltimateApp::DLL_Capture_SetMode(
	const int						handle,
	const IBScanImageType			imageType,
	const IBScanImageResolution	imageResolution,
	const DWORD					captureOptions,
	int							*resultWidth,
	int							*resultHeight,
	int							*baseResolutionX,
	int							*baseResolutionY
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
						"Enter IBSCAN_Capture_SetMode (handle=%d, imageType=%d, imageResolution=%d, captureOptions=0x%08x)",
						handle, imageType, imageResolution, captureOptions);

	if (::IsBadReadPtr( resultWidth, sizeof(*resultWidth)) ||
		::IsBadReadPtr( resultHeight, sizeof(*resultHeight)) ||
		::IsBadReadPtr( baseResolutionX, sizeof(*baseResolutionX)) ||
		::IsBadReadPtr( baseResolutionY, sizeof(*baseResolutionY)))
	{
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
						"Exit  IBSCAN_Capture_SetMode (handle=%d, imageType=%d, imageResolution=%d, captureOptions=0x%08x)",
						handle, imageType, imageResolution, captureOptions);
		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_SetMode(imageType, imageResolution, captureOptions,
								resultWidth, resultHeight, baseResolutionX, baseResolutionY);
    }

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
						"Exit  IBSCAN_Capture_SetMode (handle=%d, imageType=%d, imageResolution=%d, captureOptions=0x%08x, " \
						"resultWidth=%d, resultHeight=%d, baseResolutionX=%d, baseResolutionY=%d)",
						handle, imageType, imageResolution, captureOptions,
						*resultWidth, *resultHeight, *baseResolutionX, *baseResolutionY);

	return nRc;
}

int CIBScanUltimateApp::DLL_Capture_Start(
	const int  handle,
	const int  numberOfObjects
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
	IBSU_ImageType	imageType;
    IBSU_ImageResolution  imageResolution;
    DWORD                 captureOptions;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
						"Enter IBSCAN_Capture_Start (handle=%d numberOfObjects=%d)", handle, numberOfObjects);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if (nRc == IBSU_STATUS_OK)
	{
		nRc = pIBUsbManager->m_pMainCapture->Capture_GetMode(&imageType, &imageResolution, &captureOptions, numberOfObjects); 
		if (nRc == IBSU_STATUS_OK)
		{
			nRc = pIBUsbManager->m_pMainCapture->Capture_Start(imageType, imageResolution, captureOptions, FALSE);
		}
	}

	InsertTraceLog(sTraceMainCategory, nRc, theApp._GetThreadID(),
						"Exit  IBSCAN_Capture_Start (handle=%d numberOfObjects=%d)", handle, numberOfObjects);

	return nRc;
}

int CIBScanUltimateApp::DLL_GetDeviceHandleInfo(
    const int	      deviceIndex,
    int               *handle,
    BOOL              *isHandleOpened
)
{
	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		                "Enter IBSCAN_Main_GetDeviceHandleInfo (deviceIndex=%d)", deviceIndex);

	if (::IsBadReadPtr( handle, sizeof(handle)) ||
		::IsBadReadPtr( isHandleOpened, sizeof(isHandleOpened)))
	{
		nRc = IBSU_ERR_INVALID_PARAM_VALUE;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			            "Exit  IBSCAN_Main_GetDeviceHandleInfo (deviceIndex=%d)", deviceIndex);
		return nRc;
	}

	pIBUsbManager = FindUsbDeviceByIndexInList(deviceIndex);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	*handle = -1;
	*isHandleOpened = FALSE;
	if (nRc == IBSU_STATUS_OK)
	{
		if (pIBUsbManager->m_pMainCapture->IsInitialized() == FALSE)
		{
			nRc = IBSU_ERR_DEVICE_NOT_INITIALIZED;
		}
		else
		{
			*handle = pIBUsbManager->m_pMainCapture->GetDeviceHandle();
			*isHandleOpened = TRUE;
		}
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		                "Exit  IBSCAN_Main_GetDeviceHandleInfo (deviceIndex=%d, handle=%d, isHandleOpened=%d)",
		                deviceIndex, *handle, *isHandleOpened);

	return nRc;
}


int CIBScanUltimateApp::DLL_RawCapture_Start(
    const int   handle,
    const int   numberOfObjects
)
{
	CIBUsbManager	*pIBUsbManager = NULL;
	IBSU_ImageType	imageType;
    IBSU_ImageResolution  imageResolution;
    DWORD                 captureOptions;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		                "Enter IBSCAN_RawCapture_Start (handle=%d, numberOfObjects=%d)", handle, numberOfObjects);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if (nRc == IBSU_STATUS_OK)
	{
		nRc = pIBUsbManager->m_pMainCapture->Capture_GetMode(&imageType, &imageResolution, &captureOptions, numberOfObjects); 
		if (nRc == IBSU_STATUS_OK)
		{
			nRc = pIBUsbManager->m_pMainCapture->Capture_Start(imageType, imageResolution, captureOptions, FALSE, TRUE);
		}
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		                "Exit  IBSCAN_RawCapture_Start (handle=%d, numberOfObjects=%d)", handle, numberOfObjects);

	return nRc;
}

int CIBScanUltimateApp::DLL_RawCapture_Abort(
    const int handle
)
{
	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		                "Enter IBSCAN_RawCapture_Abort (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if (nRc == IBSU_STATUS_OK)
	{
		nRc = pIBUsbManager->m_pMainCapture->RawCapture_Abort();
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		                "Exit  IBSCAN_RawCapture_Abort (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_RawCapture_GetOneFrameImage(
    const int handle
)
{
	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		                "Enter IBSCAN_RawCapture_GetOneFrameImage (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if (nRc == IBSU_STATUS_OK)
	{
		nRc = pIBUsbManager->m_pMainCapture->RawCapture_GetOneFrameImage();
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		                "Exit  IBSCAN_RawCapture_GetOneFrameImage (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_RawCapture_TakePreviewImage(
    const int          handle,
    const RawImageData rawImage
)
{
	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		                "Enter IBSCAN_RawCapture_TakePreviewImage (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if (nRc == IBSU_STATUS_OK)
	{
		nRc = pIBUsbManager->m_pMainCapture->RawCapture_TakePreviewImage(rawImage);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		                "Exit  IBSCAN_RawCapture_TakePreviewImage (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_RawCapture_TakeResultImage(
    const int          handle,
    const RawImageData rawImage
)
{
	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		                "Enter IBSCAN_RawCapture_TakeResultImage (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->RawCapture_TakeResultImage(rawImage);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		                "Exit  IBSCAN_RawCapture_TakeResultImage (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::DLL_SetDACRegister(
    const int handle,
    const int dacValue
)
{
	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		                "Enter IBSCAN_Capture_SetDACRegister (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->Capture_SetDACRegister(dacValue);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		                "Exit  IBSCAN_Capture_SetDACRegister (handle=%d, dacValue=%d)", handle, dacValue);

	return nRc;
}
#endif

int CIBScanUltimateApp::DLL_SetEncryptionKey(
     const int                  handle,
     const unsigned char*	    pEncyptionKey,
	 const IBSU_EncryptionMode  encMode
)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

    if (::IsBadReadPtr(pEncyptionKey, AES256_KEY_LENGTH))
    {
		if(encMode != ENUM_IBSU_ENCRYPTION_KEY_RANDOM)
		{
			nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
			InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
								  "Exit  IBSU_SetEncryptionKey (handle=%d)", handle);
			return nRc;
		}
    }

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter IBSU_SetEncryptionKey (handle=%d, mode=%d)", handle, encMode);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->_SetEncryptionKey(pEncyptionKey, encMode);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_SetEncryptionKey (handle=%d, mode=%d)", handle, encMode);

	return nRc;

}

int CIBScanUltimateApp::RES_OpenDevice(
    const int  deviceIndex,                         ///< [in]  Zero-based device index for device to init
    const char *pReservedkey,                       ///< [in]  Reserved key which is made manufacturer
	const BOOL useUniformityMask,					///< [in]  Set device in uniformity mask mode (if set to TRUE)
    int        *pHandle                             ///< [out] Function returns device handle to be used for subsequent function calls \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    int	nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RESERVED_OpenDevice (deviceIndex=%d)", deviceIndex);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)) ||
            ::IsBadReadPtr(pHandle, sizeof(int)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_OpenDevice (deviceIndex=%d)", deviceIndex);
        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_OpenDevice (deviceIndex=%d,", deviceIndex);

        return nRc;
    }

    *pHandle = -1;

    FindAllDeviceCount();
    if (GetDeviceCount() <= deviceIndex)
    {
        nRc = IBSU_ERR_DEVICE_NOT_FOUND;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_OpenDevice (deviceIndex=%d)", deviceIndex);
        return nRc;
    }

    nRc = _OpenDevice(deviceIndex, NULL, useUniformityMask, FALSE, pHandle, TRUE);

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  RESERVED_OpenDevice (deviceIndex=%d, pHandle=%d)", deviceIndex, *pHandle);

    return nRc;
}

int CIBScanUltimateApp::RES_GetDeviceInfo(
    const int              deviceIndex,               ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char             *pReservedkey,             ///< [in]  Reserved key which is made manufacturer
    RESERVED_DeviceInfo    *pDeviceInfo               ///< [out] Basic device information \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    int				nRc = IBSU_STATUS_OK;
    CIBUsbManager *pIBUsbManager = NULL;
    CMainCapture	*pMainCapture = NULL;
    RESERVED_DeviceInfo devInfo;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RESERVED_GetDeviceInfo (deviceIndex=%d)", deviceIndex);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)) ||
            ::IsBadReadPtr(pDeviceInfo, sizeof(RESERVED_DeviceInfo)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_GetDeviceInfo (deviceIndex=%d)", deviceIndex);
        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK &&
		_IsValidPartnerReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_GetDeviceInfo (deviceIndex=%d,", deviceIndex);

        return nRc;
    }

    memset(pDeviceInfo, 0, sizeof(RESERVED_DeviceInfo));
    pDeviceInfo->handle = -1;
    memset(&devInfo, 0, sizeof(RESERVED_DeviceInfo));

    FindAllDeviceCount();
    if (GetDeviceCount() <= deviceIndex)
    {
        nRc = IBSU_ERR_DEVICE_NOT_FOUND;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_GetDeviceInfo (deviceIndex=%d)", deviceIndex);
        return nRc;
    }

    BOOL bPPIMode = FALSE;
#if defined(__ppi__)
    if (deviceIndex < m_nPPIDeviceCount)
        bPPIMode = TRUE;
#endif

    if ((pIBUsbManager = FindUsbDeviceByIndexInList(deviceIndex)) == NULL ||
            pIBUsbManager->m_pMainCapture == NULL)
    {
        if (pIBUsbManager == NULL)
        {
            pIBUsbManager = new CIBUsbManager();
        }
        pMainCapture = new CMainCapture(&theApp, pIBUsbManager);
        nRc = pMainCapture->Reserved_GetDeviceInfo(deviceIndex, &devInfo, bPPIMode);
        Sleep(10);
        delete(CMainCapture *)pMainCapture;
        if (pIBUsbManager)
        {
            delete(CIBUsbManager *)pIBUsbManager;
        }

        // [IBSU_WRN_CHANNEL_IO_SLEEP_STATUS] Communction failed because status of USB chip is sleep mode
        // [IBSU_ERR_DEVICE_ACTIVE] Initialization failed because in use by another thread/process
        if (nRc == IBSU_WRN_CHANNEL_IO_SLEEP_STATUS || nRc == IBSU_ERR_DEVICE_ACTIVE)
        {
            if (!_GetProductNameFromEnumeration(deviceIndex, devInfo.productName))
            {
                return IBSU_ERR_CHANNEL_IO_COMMAND_FAILED;
            }
#if defined(_WINDOWS) && !defined(WINCE)
            if (g_pSharedData != NULL && strcmp(g_pSharedData->RES_devDesc[deviceIndex].productName, devInfo.productName) == 0)
            {
                memcpy(&devInfo, &g_pSharedData->RES_devDesc[deviceIndex], sizeof(RESERVED_DeviceInfo));
            }
            else
            {
                sprintf(devInfo.serialNumber, "?");
                sprintf(devInfo.fwVersion, "?");
                sprintf(devInfo.vendorID, "?");
                sprintf(devInfo.devRevision, "?");
                sprintf(devInfo.productionDate, "?");
                sprintf(devInfo.serviceDate, "?");
            }
#else
            sprintf(devInfo.serialNumber, "?");
            sprintf(devInfo.fwVersion, "?");
            sprintf(devInfo.vendorID, "?");
            sprintf(devInfo.devRevision, "?");
            sprintf(devInfo.productionDate, "?");
            sprintf(devInfo.serviceDate, "?");
#endif
            sprintf(devInfo.interfaceType, "%s", "USB");
            devInfo.handle = -1;
            devInfo.IsHandleOpened = FALSE;
           
            if (nRc == IBSU_ERR_DEVICE_ACTIVE)
            {
                nRc = IBSU_STATUS_OK;    // Remove error information to display only
            }
        }
        else
        {
#if defined(_WINDOWS) && !defined(WINCE)
            if (g_pSharedData != NULL)
            {
//                memcpy(&g_pSharedData->RES_devDesc[deviceIndex], &devInfo, sizeof(RESERVED_DeviceInfo));
            }
#endif
        }
    }
    else
    {
        pMainCapture = pIBUsbManager->m_pMainCapture;
        nRc = pMainCapture->Reserved_GetDeviceInfo(deviceIndex, &devInfo, bPPIMode);
        devInfo.handle = pIBUsbManager->m_pMainCapture->GetDeviceHandle();
        devInfo.IsHandleOpened = TRUE;
    }

    memcpy(pDeviceInfo, &devInfo, sizeof(RESERVED_DeviceInfo));
    if ((strcmp(pDeviceInfo->productName, "IBNW11C")			== 0) ||
            (strcmp(pDeviceInfo->productName, "IBSCAN310LS-W")   == 0))
    {
        strcpy(pDeviceInfo->productName, "WATSON");
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  RESERVED_GetDeviceInfo (deviceIndex=%d, %s_v%s(%s))", deviceIndex,
                          pDeviceInfo->productName, pDeviceInfo->fwVersion, pDeviceInfo->serialNumber);

    return nRc;
}

int CIBScanUltimateApp::RES_WriteEEPROM(
    const int     handle,                             ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char    *pReservedkey,                      ///< [in]  Reserved key which is made manufacturer
    const WORD    addr,                               ///< [in]  Address of EEPROM
    const BYTE    *pData,                             ///< [in]  Data buffer
    const int     len                                 ///< [in]  Length of data buffer
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RESERVED_WriteEEPROM (handle=%d)", handle);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)) ||
            ::IsBadReadPtr(pData, sizeof(*pData)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_WriteEEPROM (handle=%d)", handle);

        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_WriteEEPROM (handle=%d, addr=0x%04x, pData=%x, len=%d)",
                              handle, addr, pData, len);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Reserved_WriteEEPROM(addr, pData, len);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  RESERVED_WriteEEPROM (handle=%d, addr=0x%04x, pData=%x, len=%d)",
                          handle, addr, pData, len);

    return nRc;
}

int CIBScanUltimateApp::RES_SetProperty(
    const int             handle,                     ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char            *pReservedkey,              ///< [in]  Reserved key which is made manufacturer
    const IBSU_PropertyId propertyId,                 ///< [in] Property identifier to set value for
    LPCSTR                propertyValue               ///< [in] String containing property value
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;
    ReservedAccessLevel nAccessLevel = RESERVED_ACCESS_NO;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RESERVED_SetProperty (handle=%d, propertyId=%d)",
                          handle, propertyId);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)) ||
            ::IsBadReadPtr(propertyValue, sizeof(propertyValue)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_SetProperty (handle=%d, propertyId=%d)",
                              handle, propertyId);
        return nRc;
    }

    if (_GetReservedKeyAccessLevel(pReservedkey, &nAccessLevel) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_SetProperty (handle=%d, propertyId=%d, propertyValue=%s)",
                              handle, propertyId, propertyValue);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Main_SetProperty(propertyId, propertyValue, TRUE, nAccessLevel);

        Sleep(50);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  RESERVED_SetProperty (handle=%d, propertyId=%d, propertyValue=%s)",
                          handle, propertyId, propertyValue);

    return nRc;
}

int CIBScanUltimateApp::RES_SetLEVoltage(
    const int     handle,                             ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char    *pReservedkey,                      ///< [in]  Reserved key which is made manufacturer
    const int	    voltageValue                        ///< [in]  Contrast value (range: 0 <= value <= @ref RESERVED_MAX_LE_VOLTAGE_VALUE)
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RESERVED_SetLEVoltage (handle=%d, voltageValue=%d)", handle, voltageValue);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_SetLEVoltage (handle=%d, voltageValue=%d)",
                              handle, voltageValue);
        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_SetLEVoltage (handle=%d, voltageValue=%d)",
                              handle, voltageValue);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_SetLEVoltage(voltageValue);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  RESERVED_SetLEVoltage (handle=%d, voltageValue=%d)",
                          handle, voltageValue);

    return nRc;
}

int CIBScanUltimateApp::RES_GetLEVoltage(
    const int     handle,                             ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char    *pReservedkey,                      ///< [in]  Reserved key which is made manufacturer
    int	        *pVoltageValue                      ///< [out] LE voltage value (range: RESERVED_MIN_LE_VOLTAGE_VALUE <= value <= @ref RESERVED_MAX_LE_VOLTAGE_VALUE) \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RESERVED_GetLEVoltage (handle=%d)", handle);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)) ||
            ::IsBadReadPtr(pVoltageValue, sizeof(pVoltageValue)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_GetLEVoltage (handle=%d)", handle);
        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_GetLEVoltage (handle=%d)",
                              handle);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    *pVoltageValue = -1;
    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_GetLEVoltage(pVoltageValue);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  RESERVED_GetLEVoltage (handle=%d, pVoltageValue=%d)",
                          handle, *pVoltageValue);

    return nRc;
}

int CIBScanUltimateApp::RES_BeginCaptureImage(
    const int                   handle,               ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const char                  *pReservedkey,        ///< [in]  Reserved key which is made manufacturer
    const IBSU_ImageType        imageType,            ///< [in]  Image type to capture
    const IBSU_ImageResolution  imageResolution,      ///< [in]  Requested capture resolution
    const DWORD                 captureOptions        ///< [in]  Bit coded capture options to use (see @ref CaptureOptions)
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RESERVED_BeginCaptureImage (handle=%d, imageType=%d, imageResolutin=%d, captureOptions=0x%x)",
                          handle, imageType, imageResolution, captureOptions);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_BeginCaptureImage (handle=%d, imageType=%d, imageResolutin=%d, captureOptions=0x%x)",
                              handle, imageType, imageResolution, captureOptions);
        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_BeginCaptureImage (handle=%d, imageType=%d, imageResolutin=%d, captureOptions=0x%x)",
                              handle, imageType, imageResolution, captureOptions);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Capture_Start(imageType, imageResolution, captureOptions, TRUE);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  RESERVED_BeginCaptureImage (handle=%d, imageType=%d, imageResolutin=%d, captureOptions=0x%x)",
                          handle, imageType, imageResolution, captureOptions);

    return nRc;
}

int CIBScanUltimateApp::RES_GetOneFrameImage(
    const int     handle,                             ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char    *pReservedkey,                      ///< [in]  Reserved key which is made manufacturer
    unsigned char *pRawImage,                         ///< [out] Raw capture image from camera (CIS, ROIC, ASIC and some on)
    ///<       Memory must be provided by caller
    const int     imageLength                         ///< [in]  Image length of Raw capture image
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RESERVED_GetOneFrameImage (handle=%d, imageLength=%d)", handle, imageLength);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)) ||
            ::IsBadReadPtr(pRawImage, sizeof(pRawImage)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_GetOneFrameImage (handle=%d, imageLength=%d)", handle, imageLength);
        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_GetOneFrameImage (handle=%d, imageLength=%d)",
                              handle, imageLength);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    // 2013-02-27 enzyme delete
    //	*pRawImage = NULL;
    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Reserved_GetOneFrameImage(pRawImage, imageLength);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  RESERVED_GetOneFrameImage (handle=%d, pRawImage=0x%x, imageLength=%d)",
                          handle, pRawImage, imageLength);

    return nRc;
}

int CIBScanUltimateApp::RES_SetFpgaRegister(
    const int              handle,                    ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char             *pReservedkey,             ///< [in]  Reserved key which is made manufacturer
    const unsigned char    address,                   ///< [in]  Address of FPGA register (range: 0 <= value <= 0xFF)
    const unsigned char    value                      ///< [in]  Value of FPGA register (range: 0 <= value <= 0xFF)
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RESERVED_SetFpgaRegister (handle=%d, value=0x%x)", handle, value);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_SetFpgaRegister (handle=%d, value=0x%x)",
                              handle, value);
        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_SetFpgaRegister (handle=%d, value=0x%x)",
                              handle, value);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Reserved_SetFpgaRegister(address, value);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  RESERVED_SetFpgaRegister (handle=%d, value=0x%x)",
                          handle, value);

    return nRc;
}

int CIBScanUltimateApp::RES_GetFpgaRegister(
    const int              handle,                    ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char             *pReservedkey,             ///< [in]  Reserved key which is made manufacturer
    const unsigned char    address,                   ///< [in]  Address of FPGA register (range: 0 <= value <= 0xFF)
    unsigned char          *pValue                    ///< [out] Value of FPGA register (range: 0 <= value <= 0xFF) \n
    ///<       Memory must be provided by caller
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RESERVED_GetFpgaRegister (handle=%d)", handle);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)) ||
            ::IsBadReadPtr(pValue, sizeof(pValue)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_GetFpgaRegister (handle=%d)", handle);
        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_GetFpgaRegister (handle=%d)",
                              handle);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    *pValue = -1;
    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Reserved_GetFpgaRegister(address, pValue);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  RESERVED_GetFpgaRegister (handle=%d, pValue=%d)",
                          handle, *pValue);

    return nRc;
}

int CIBScanUltimateApp::RES_CreateClientWindow(
    const int         handle,                          ///< [in] Device handle obtained by IBSU_OpenDevice()
    const char        *pReservedkey,                   ///< [in]  Reserved key which is made manufacturer
    const IBSU_HWND   hWindow,                         ///< [in] Windows handle to draw
    const DWORD		left,                            ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
    const DWORD		top,                             ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
    const DWORD		right,                           ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
    const DWORD		bottom,                          ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
    const DWORD       rawImgWidth,                     ///< [in] Width of raw image
    const DWORD       rawImgHeight                     ///< [in] height of raw image
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RESERVED_CreateClientWindow (handle=%d, hWindow=%x, left=%d, top=%d, right=%d, bottom=%d, rawImgWidth=%d, rawImgHeight=%d)",
                          handle, hWindow, left, top, right, bottom, rawImgWidth, rawImgHeight);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_CreateClientWindow (handle=%d, hWindow=%x, left=%d, top=%d, right=%d, bottom=%d, rawImgWidth=%d, rawImgHeight=%d)",
                              handle, hWindow, left, top, right, bottom, rawImgWidth, rawImgHeight);
        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_CreateClientWindow (handle=%d, hWindow=%x, left=%d, top=%d, right=%d, bottom=%d, rawImgWidth=%d, rawImgHeight=%d)",
                              handle, hWindow, left, top, right, bottom, rawImgWidth, rawImgHeight);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->ClientWindow_Create(hWindow, left, top, right, bottom, TRUE, rawImgWidth, rawImgHeight);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  RESERVED_CreateClientWindow (handle=%d, hWindow=%x, left=%d, top=%d, right=%d, bottom=%d, rawImgWidth=%d, rawImgHeight=%d)",
                          handle, hWindow, left, top, right, bottom, rawImgWidth, rawImgHeight);

    return nRc;
}

int CIBScanUltimateApp::RES_DrawClientWindow(
    const int         handle,                          ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const char        *pReservedkey,                   ///< [in]  Reserved key which is made manufacturer
    unsigned char     *drawImage                       ///< [in]  image buffer to draw on Client window.
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RESERVED_DrawClientWindow (handle=%d)",
                          handle);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)) ||
            ::IsBadReadPtr(drawImage, sizeof(*drawImage)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_DrawClientWindow (handle=%d)",
                              handle);
        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  RESERVED_DrawClientWindow (handle=%d)",
                              handle);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Reserved_DrawClientWindow(drawImage);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit  RESERVED_DrawClientWindow (handle=%d, drawImage=0x%x)",
                          handle, drawImage);

    return nRc;
}

int CIBScanUltimateApp::RES_UsbBulkOutIn(
    const int             handle,                      ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const char            *pReservedkey,               ///< [in]  Reserved key which is made manufacturer
    const int             outEp,                       ///< [in]  BulkOut endpoint
    const unsigned char   uiCommand,
    unsigned char         *outData,                    ///< [in]  send data (You can use 'NULL')
    const int             outDataLen,
    const int             inEp,                        ///< [in]  BulkIn endpoint
    unsigned char         *inData,                     ///< [in]  receive data (You can use 'NULL')
    const int             inDataLen,
    int                   *nBytesRead                  ///< [out] Received data count (You can use 'NULL')
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RESERVED_UsbBulkOutIn (handle=%d, outEp=%d, uiCommand=0x%x, outDataLen=%d, inEp=%d, inDataLen=%d)",
                          handle, outEp, uiCommand, outDataLen, inEp, inDataLen);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit RESERVED_UsbBulkOutIn (handle=%d, outEp=%d, uiCommand=0x%x, outDataLen=%d, inEp=%d, inDataLen=%d)",
                              handle, outEp, uiCommand, outDataLen, inEp, inDataLen);
        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit RESERVED_UsbBulkOutIn (handle=%d, outEp=%d, uiCommand=0x%x, outData=0x%x, outDataLen=%d, inEp=%d, inData=0x%x, inDataLen=%d)",
                              handle, outEp, uiCommand, outData, outDataLen, inEp, inData, inDataLen);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Reserved_UsbBulkOutIn(outEp, uiCommand, outData, outDataLen, inEp, inData, inDataLen, (LONG *)nBytesRead);
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit RESERVED_UsbBulkOutIn (handle=%d, outEp=%d, uiCommand=0x%x, outData=0x%x, outDataLen=%d, inEp=%d, inData=0x%x, inDataLen=%d)",
                          handle, outEp, uiCommand, outData, outDataLen, inEp, inData, inDataLen);

    return nRc;
}

int CIBScanUltimateApp::RES_InitializeCamera(
    const int             handle,                      ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const char            *pReservedkey                ///< [in]  Reserved key which is made manufacturer
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RESERVED_InitializeCamera (handle=%d)",
                          handle);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit RESERVED_InitializeCamera (handle=%d",
                              handle);
        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit RESERVED_InitializeCamera (handle=%d)",
                              handle);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
        nRc = pIBUsbManager->m_pMainCapture->Reserved_InitializeCamera();
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit RESERVED_InitializeCamera (handle=%d)",
                          handle);

    return nRc;
}

int CIBScanUltimateApp::RES_ReadEEPROM(
      const int     handle,                             ///< [in]  Device handle obtained by IBSU_DeviceOpen()
      const char    *pReservedkey,                      ///< [in]  Reserved key which is made manufacturer
	  const WORD    addr,                               ///< [in]  Address of EEPROM
	  BYTE          *pData,                             ///< [out] Data buffer
      const int     len                                 ///< [in]  Length of data buffer
    )
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter RESERVED_ReadEEPROM (handle=%d)", handle);

	if( ::IsBadReadPtr( pReservedkey, sizeof(*pReservedkey)) ||
		::IsBadReadPtr( pData, sizeof(*pData)) )
	{
		nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit RESERVED_ReadEEPROM (handle=%d)", handle);

		return nRc;
	}

	if( _IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK )
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit RESERVED_ReadEEPROM (handle=%d, addr=0x%04x, pData=%x, len=%d)",
			handle, addr, pData, len);

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->Reserved_ReadEEPROM(addr, pData, len);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit RESERVED_ReadEEPROM (handle=%d, addr=0x%04x, pData=%x, len=%d)",
		handle, addr, pData, len);

	return nRc;
}

int CIBScanUltimateApp::RES_GetEnhancedImage
    (const int              handle,
     const char            *pReservedkey,
     const IBSU_ImageData   inImage,
     IBSU_ImageData        *enhancedImage,
     int                   *segmentImageArrayCount,
     IBSU_ImageData        *segmentImageArray,
     IBSU_SegmentPosition  *segmentPositionArray)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter RESERVED_GetEnhancedImage (handle=%d)", handle);

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK &&
		_IsValidPartnerReservedKey(pReservedkey) != IBSU_STATUS_OK)
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit RESERVED_GetEnhancedImage (handle=%d)", handle);

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->Reserved_GetEnhancedImage(inImage, enhancedImage, segmentImageArrayCount, segmentImageArray, segmentPositionArray);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit RESERVED_GetEnhancedImage (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::RES_GetFinalImageByNative
    (const int              handle,
     const char            *pReservedkey,
     IBSU_ImageData	       *finalImage)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter RES_GetFinalImageByNative (handle=%d)", handle);

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK &&
		_IsValidPartnerReservedKey(pReservedkey) != IBSU_STATUS_OK)
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit RES_GetFinalImageByNative (handle=%d)", handle);

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->Reserved_GetFinalImageByNative(finalImage);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit RES_GetFinalImageByNative (handle=%d)", handle);

	return nRc;
}

#ifdef _WINDOWS
int CIBScanUltimateApp::RES_GetSpoofScore
    (const char            *pReservedkey,
	 RESERVED_CaptureDeviceID  deviceID,
     const BYTE            *pImage,
     const int             Width,
	 const int             Height,
	 const int             Pitch,
	 int                   *pScore)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;
	BYTE			*invImage;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter RES_GetSpoofScore (deviceID=%d, width=%d, height=%d, Pitch=%d)", deviceID, Width, Height, Pitch);

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK &&
		_IsValidPartnerReservedKey(pReservedkey) != IBSU_STATUS_OK)
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit RES_GetSpoofScore (deviceID=%d)", deviceID);

		return nRc;
	}

	if (deviceID != RESERVED_CAPTURE_DEVICE_ID_COLUMBO)
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit RES_GetSpoofScore (deviceID=%d)", deviceID);

		return nRc;
	}

	if (Width != 400 || Height != 500)
	{
		nRc = IBSU_ERR_INVALID_PARAM_VALUE;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit RES_GetSpoofScore (deviceID=%d)", deviceID);

		return nRc;
	}

	// Check Pitch has Posive or Negative value
	// Negative value meaning image flipped vertically
	if (Pitch < 0) // Verical flipped image
	{
		invImage = new BYTE [Width*Height];
		// Scanned image Vertical flip
		for(int i=0; i<Height; i++)
		{
			memcpy(&invImage[i*Width], &pImage[(Height-1-i)*Width], Width);
		}
		nex_sdk_load_image_bytes(invImage, Height, Width);
		delete [] invImage;
	}
	else // Normal image
	{
		nex_sdk_load_image_bytes(pImage, Height, Width);
	}
	
	*pScore = nex_sdk_get_score(NEX_SDK_CLASSIFIER_MLP);
	
	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit RES_GetSpoofScore (deviceID=%d, Score=%d)", deviceID, *pScore);

	return nRc;
}
#endif

int CIBScanUltimateApp::RES_GetEncryptedImage
    (const int              handle,
     const char         *pReservedkey,
     unsigned char      *pEncKey,
     IBSU_ImageData     *pRawEncImage)

{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter RES_GetEncryptedImage (handle=%d)", handle);

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK &&
		_IsValidPartnerReservedKey(pReservedkey) != IBSU_STATUS_OK)
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit RES_GetEncryptedImage (handle=%d)", handle);

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->Reserved_GetEncryptedImage(pEncKey, pRawEncImage);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit RES_GetEncryptedImage (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::RES_ConvertToDecryptImage
    (const char         *pReservedkey,
     unsigned char      *pEncKey,
     IBSU_ImageData     rawEncImage,
     IBSU_ImageData     *pRawDecImage)

{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter RES_ConvertToDecryptImage ()");

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK &&
		_IsValidPartnerReservedKey(pReservedkey) != IBSU_STATUS_OK)
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit RES_ConvertToDecryptImage ()");

		return nRc;
	}

    if (!m_pDecryptImage)
    {
        m_pDecryptImage = new BYTE [__MAX_IMG_WIDTH__ * __MAX_IMG_HEIGHT__];
    }

    unsigned char *ptr = (unsigned char*)rawEncImage.Buffer;
	CIBEncryption *m_AESEnc = new CIBEncryption(AES_KEY_ENCRYPTION);
	m_AESEnc->AES_Set_Key((unsigned char*)pEncKey);
	m_AESEnc->AES_Decrypt_opt(ptr, (unsigned int)(rawEncImage.Width*rawEncImage.Height), &m_pDecryptImage[2]);
    pRawDecImage->BitsPerPixel = rawEncImage.BitsPerPixel;
    pRawDecImage->Format = rawEncImage.Format;
    pRawDecImage->Buffer = m_pDecryptImage;
    pRawDecImage->Width = rawEncImage.Width;
    pRawDecImage->Height = rawEncImage.Height;
    pRawDecImage->ResolutionX = rawEncImage.ResolutionX;
    pRawDecImage->ResolutionY = rawEncImage.ResolutionY;
    pRawDecImage->Pitch = rawEncImage.Pitch;
	delete m_AESEnc;

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit RES_ConvertToDecryptImage ()");

	return nRc;
}

/*
int CIBScanUltimateApp::RES_ReadMask
    (const int              handle,
     const char            *pReservedkey,
     unsigned char		   *pWaterImage,
     int				   WaterImageSize)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter RES_ReadMask (handle=%d)", handle);

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK &&
		_IsValidPartnerReservedKey(pReservedkey) != IBSU_STATUS_OK)
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit  RES_ReadMask (handle=%d)", handle);

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->_ReadMaskFromEP8IN_Kojak(pWaterImage, WaterImageSize);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  RES_GetEnhancedImage (handle=%d)", handle);

	return nRc;
}


int CIBScanUltimateApp::RES_ReadMask_8M
    (const int              handle,
     const char            *pReservedkey,
     unsigned char		   *pWaterImage,
     int				   WaterImageSize,
	 const int			   Sector)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter RES_ReadMask (handle=%d)", handle);

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK &&
		_IsValidPartnerReservedKey(pReservedkey) != IBSU_STATUS_OK)
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit  RES_ReadMask (handle=%d)", handle);

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->_ReadMaskFromEP8IN_Kojak_8M(pWaterImage, WaterImageSize, Sector);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  RES_GetEnhancedImage (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::RES_WriteMask
    (const int              handle,
     const char            *pReservedKey,
     const BYTE			   *MaskData,
	 const int			   MaskDataLen,
	 int				   *pProgress)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter RES_WriteMask (handle=%d)", handle);

    if (_IsValidReservedKey(pReservedKey) != IBSU_STATUS_OK)
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit  RES_WriteMask (handle=%d)", handle);

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->Reserved_WriteMask(MaskData, MaskDataLen, pProgress);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  RES_WriteMask (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::RES_WriteMask_8M
    (const int              handle,
     const char            *pReservedKey,
     const BYTE			   *MaskData,
	 const int			   MaskDataLen,
	 const int			   Sector,
	 int				   *pProgress)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter RES_WriteMask_8M (handle=%d)", handle);

    if (_IsValidReservedKey(pReservedKey) != IBSU_STATUS_OK)
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit  RES_WriteMask_8M (handle=%d)", handle);

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->Reserved_WriteMask_8M(MaskData, MaskDataLen, Sector, pProgress);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  RES_WriteMask_8M (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::RES_ResetDevice
    (const int              handle,
     const char            *pReservedKey)
{
//	CThreadSync Sync;
	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter RES_ResetDevice (handle=%d)", handle);

    if (_IsValidReservedKey(pReservedKey) != IBSU_STATUS_OK)
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit  RES_ResetDevice (handle=%d)", handle);

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->Reserved_ResetDevice();
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  RES_ResetDevice (handle=%d)", handle);

	return nRc;
}

int CIBScanUltimateApp::RES_InitializeCameraFlatForKOJAK(
    const int             handle,                      ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const char            *pReservedkey                ///< [in]  Reserved key which is made manufacturer
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RES_InitializeCameraFlatForKOJAK (handle=%d)",
                          handle);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit RES_InitializeCameraFlatForKOJAK (handle=%d",
                              handle);
        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit RES_InitializeCameraFlatForKOJAK (handle=%d)",
                              handle);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
		pIBUsbManager->m_pMainCapture->m_propertyInfo.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
        nRc = pIBUsbManager->m_pMainCapture->Reserved_InitializeCameraFlatForKOJAK();
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit RES_InitializeCameraFlatForKOJAK (handle=%d)",
                          handle);

    return nRc;
}

int CIBScanUltimateApp::RES_InitializeCameraRollForKOJAK(
    const int             handle,                      ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const char            *pReservedkey                ///< [in]  Reserved key which is made manufacturer
)
{
//	CThreadSync Sync;

    CIBUsbManager	*pIBUsbManager = NULL;
    int				nRc = IBSU_STATUS_OK;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter RES_InitializeCameraRollForKOJAK (handle=%d)",
                          handle);

    if (::IsBadReadPtr(pReservedkey, sizeof(*pReservedkey)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit RES_InitializeCameraRollForKOJAK (handle=%d",
                              handle);
        return nRc;
    }

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK)
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit RES_InitializeCameraRollForKOJAK (handle=%d)",
                              handle);

        return nRc;
    }

    pIBUsbManager = FindUsbDeviceInList(handle);
    nRc = _IsValidUsbHandle(pIBUsbManager);

    if (nRc == IBSU_STATUS_OK)
    {
		pIBUsbManager->m_pMainCapture->m_propertyInfo.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
        nRc = pIBUsbManager->m_pMainCapture->Reserved_InitializeCameraRollForKOJAK();
    }

    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                          "Exit RES_InitializeCameraRollForKOJAK (handle=%d)",
                          handle);

    return nRc;
}
*/



//////////////////////////////////////////////////////////////////////////////////


#if defined(__IBSCAN_ULTIMATE_SDK__)
#ifndef WINCE
int WINAPI IBSU_GetSDKVersion(
    IBSU_SdkVersion *pVerInfo                       ///< [out] API version information \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetSDKVersion(pVerInfo);
}

#else
int WINAPI IBSU_GetSDKVersionW(
    IBSU_SdkVersionW *pVerInfo                       ///< [out] API version information \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetSDKVersionW(pVerInfo);
}
#endif

int WINAPI IBSU_GetDeviceCount(
    int *pDeviceCount                               ///< [out] Number of connected devices \n
    ///  Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetDeviceCount(pDeviceCount);
}

#ifndef WINCE
int WINAPI IBSU_GetDeviceDescription(
    const int          deviceIndex,                   ///< [in] Zero-based device index for device to lookup
    IBSU_DeviceDesc    *pDeviceDescA                   ///< [out] Basic device description \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetDeviceDescription(deviceIndex, (IBSU_DeviceDescA*)pDeviceDescA);
}

#else
int WINAPI IBSU_GetDeviceDescriptionW(
    const int          deviceIndex,                   ///< [in] Zero-based device index for device to lookup
    IBSU_DeviceDescW    *pDeviceDescW                   ///< [out] Basic device description \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetDeviceDescriptionW(deviceIndex, pDeviceDescW);
}
#endif

int WINAPI IBSU_RegisterCallbacks(
    const int		       handle,                      ///< [in] Device handle obtained by IBSU_DeviceOpen()
    const IBSU_Events    events,                      ///< [in] Enum value to the notification function
    void                 *pEventName,                 ///< [in] Pointer to the notification function
    void                 *pContext                    ///< [in] Pointer to user context; this value is used as parameter for callback
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_RegisterCallbacks(handle, events, pEventName, pContext);
}

int WINAPI IBSU_OpenDevice(
    const int  deviceIndex,                         ///< [in]  Zero-based device index for device to init
    int        *pHandle                             ///< [out] Function returns device handle to be used for subsequent function calls \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_OpenDevice(deviceIndex, TRUE, pHandle);
}

int WINAPI IBSU_CloseDevice(
    const int  handle                              ///< [in] Device handle obtained by IBSU_OpenDevice()
    //      const BOOL sendToStandby                     ///< [in] Set device in standby mode (if set to TRUE)
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_CloseDevice(handle);
}

int WINAPI IBSU_CloseAllDevice(
    //      const BOOL sendToStandby                        ///< [in] If TRUE then set devices in standby mode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_CloseAllDevice();
}

int WINAPI IBSU_IsDeviceOpened(
    const int  handle                               ///< [in] Device handle obtained by IBSU_OpenDevice()
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_IsDeviceOpened(handle);
}

#ifndef WINCE
int WINAPI IBSU_GetProperty(
    const int             handle,                   ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const IBSU_PropertyId propertyId,               ///< [in]  Property identifier to get value for
    LPSTR                 propertyValue             ///< [out] String returning property value \n
    ///<       Memory must be provided by caller
    ///<       (needs to be able to hold @ref IBSU_MAX_STR_LEN characters)
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetProperty(handle, propertyId, propertyValue);
}

#else
int WINAPI IBSU_GetPropertyW(
    const int             handle,                   ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const IBSU_PropertyId propertyId,               ///< [in]  Property identifier to get value for
    LPWSTR                propertyValue             ///< [out] String returning property value \n
    ///<       Memory must be provided by caller
    ///<       (needs to be able to hold @ref IBSU_MAX_STR_LEN characters)
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetPropertyW(handle, propertyId, propertyValue);
}
#endif

#ifndef WINCE
int WINAPI IBSU_SetProperty(
    const int             handle,                   ///< [in] Device handle obtained by IBSU_OpenDevice()
    const IBSU_PropertyId propertyId,               ///< [in] Property identifier to set value for
    LPCSTR                propertyValue             ///< [in] String containing property value
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetProperty(handle, propertyId, propertyValue);
}

#else
int WINAPI IBSU_SetPropertyW(
    const int             handle,                   ///< [in] Device handle obtained by IBSU_OpenDevice()
    const IBSU_PropertyId propertyId,               ///< [in] Property identifier to set value for
    LPCWSTR               propertyValue             ///< [in] String containing property value
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetPropertyW(handle, propertyId, propertyValue);
}
#endif

int WINAPI IBSU_EnableTraceLog(
    const BOOL on  ///< [in] TRUE to turn trace log on; FALSE to turn trace log off
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_EnableTraceLog(on);
}

int WINAPI IBSU_IsCaptureAvailable(
    const int                   handle,             ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const IBSU_ImageType        imageType,          ///< [in]  Image type to verify
    const IBSU_ImageResolution  imageResolution,    ///< [in]  Requested capture resolution
    BOOL                        *pIsAvailable        ///< [out] Returns TRUE if mode is available \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_IsCaptureAvailable(handle, imageType, imageResolution, pIsAvailable);
}

int WINAPI IBSU_BeginCaptureImage(
    const int	                  handle,             ///< [in] Device handle obtained by IBSU_OpenDevice()
    const IBSU_ImageType        imageType,          ///< [in]  Image type to capture
    const IBSU_ImageResolution  imageResolution,    ///< [in]  Requested capture resolution
    const DWORD                 captureOptions      ///< [in]  Bit coded capture options to use (see @ref CaptureOptions)
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_BeginCaptureImage(handle, imageType, imageResolution, captureOptions);
}

int WINAPI IBSU_CancelCaptureImage(
    const int handle                                ///< [in] Device handle obtained by IBSU_OpenDevice()
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_CancelCaptureImage(handle);
}

int WINAPI IBSU_IsCaptureActive(
    const int handle,                               ///< [in]  Device handle obtained by IBSU_OpenDevice()
    BOOL      *pIsActive                            ///< [out] Returns TRUE if acquisition is in progress
    ///<       (preview or result image acquisition) \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_IsCaptureActive(handle, pIsActive);
}

int WINAPI IBSU_TakeResultImageManually(
    const int handle                               ///< [in]  Device handle obtained by IBSU_OpenDevice()
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_TakeResultImageManually(handle);
}

int WINAPI IBSU_GetContrast(
    const int handle,                               ///< [in]  Device handle obtained by IBSU_OpenDevice()
    int       *pContrastValue                       ///< [out] Contrast value (range: 0 <= value <= @ref IBSU_MAX_CONTRAST_VALUE) \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetContrast(handle, pContrastValue);
}

int WINAPI IBSU_SetContrast(
    const int handle,                               ///< [in] Device handle obtained by IBSU_OpenDevice()
    const int contrastValue                         ///< [in] Contrast value (range: 0 <= value <= @ref IBSU_MAX_CONTRAST_VALUE)
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetContrast(handle, contrastValue);
}

int WINAPI IBSU_GetLEOperationMode(
    const int			   handle,                               ///< [in] Device handle obtained by IBSU_OpenDevice()
    IBSU_LEOperationMode *leOperationMode								  ///< [out] Touch sensor operation mode
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetLEOperationMode(handle, leOperationMode);
}

int WINAPI IBSU_SetLEOperationMode(
    const int					 handle,                               ///< [in] Device handle obtained by IBSU_OpenDevice()
    const IBSU_LEOperationMode leOperationMode						  ///< [in] Touch sensor operation mode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetLEOperationMode(handle, leOperationMode);
}

int WINAPI IBSU_IsTouchedFinger(
    const int	handle,                               ///< [in] Device handle obtained by IBSU_OpenDevice()
    int       *touchInValue                         ///< [out] touchInValue value (0 : touch off, 1 : touch on) \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_IsTouchedFinger(handle, touchInValue);
}

int WINAPI IBSU_GetOperableLEDs(
    const int     handle,                            ///< [in]  Device handle obtained by IBSU_OpenDevice()
    IBSU_LedType  *pLedType,                         ///< [out] Type of LED's \n
    ///<       Memory must be provided by caller
    int           *pLedCount,                        ///< [out] Number of LED's \n
    ///<       Memory must be provided by caller
    DWORD         *pOperableLEDs                     ///< [out] Bit pattern of operable LED's \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetOperableLEDs(handle, pLedType, pLedCount, pOperableLEDs);
}

int WINAPI IBSU_GetLEDs(
    const int		handle,                               ///< [in]  Device handle obtained by IBSU_OpenDevice()
    DWORD			*pActiveLEDs                          ///< [out] get active LEDs
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetLEDs(handle, pActiveLEDs);
}

int WINAPI IBSU_SetLEDs(
    const int		handle,                               ///< [in] Device handle obtained by IBSU_OpenDevice()
    const DWORD	activeLEDs							  ///< [in] set active LEDs
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetLEDs(handle, activeLEDs);
}

int WINAPI IBSU_CreateClientWindow(
    const int         handle,                          ///< [in] Device handle obtained by IBSU_OpenDevice()
    const IBSU_HWND   hWindow,                         ///< [in] Windows handle to draw
    const DWORD		left,                            ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
    const DWORD		top,                             ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
    const DWORD		right,                           ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
    const DWORD		bottom                           ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_CreateClientWindow(handle, hWindow, left, top, right, bottom);
}

int WINAPI IBSU_DestroyClientWindow(
    const int         handle,                          ///< [in] Device handle obtained by IBSU_OpenDevice()
    const BOOL		clearExistingInfo                ///< [in] clear the existing display information about display property, overlay text.
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_DestroyClientWindow(handle, clearExistingInfo);
}

#ifndef WINCE
int WINAPI IBSU_GetClientWindowProperty(
    const int                         handle,                 ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const IBSU_ClientWindowPropertyId propertyId,             ///< [in]  Property identifier to set value for
    LPSTR                             propertyValue           ///< [out] String returning property value \n
    ///<       Memory must be provided by caller
    ///<       (needs to be able to hold @ref IBSU_MAX_STR_LEN characters)
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetClientWindowProperty(handle, propertyId, propertyValue);
}

#else
int WINAPI IBSU_GetClientWindowPropertyW(
    const int                         handle,                 ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const IBSU_ClientWindowPropertyId propertyId,             ///< [in]  Property identifier to set value for
    LPWSTR                            propertyValue           ///< [out] String returning property value \n
    ///<       Memory must be provided by caller
    ///<       (needs to be able to hold @ref IBSU_MAX_STR_LEN characters)
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetClientWindowPropertyW(handle, propertyId, propertyValue);
}
#endif

#ifndef WINCE
int WINAPI IBSU_SetClientDisplayProperty(
    const int                         handle,                 ///< [in] Device handle obtained by IBSU_OpenDevice()
    const IBSU_ClientWindowPropertyId propertyId,             ///< [in] Property identifier to set value for
    LPCSTR                            propertyValue           ///< [in] String containing property value
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetClientDisplayProperty(handle, propertyId, propertyValue);
}

#else
int WINAPI IBSU_SetClientDisplayPropertyW(
    const int                         handle,                 ///< [in] Device handle obtained by IBSU_OpenDevice()
    const IBSU_ClientWindowPropertyId propertyId,             ///< [in] Property identifier to set value for
    LPCWSTR                           propertyValue           ///< [in] String containing property value
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetClientDisplayPropertyW(handle, propertyId, propertyValue);
}
#endif

#ifndef WINCE
int WINAPI IBSU_SetClientWindowOverlayText(
    const int         handle,                            ///< [in] Device handle obtained by IBSU_OpenDevice()
    const char        *fontName,                         ///< [in] font name for display on window
    const int         fontSize,                          ///< [in] font size for display on window
    const BOOL        fontBold,                          ///< [in] font bold for display on window
    const char        *text,                             ///< [in] text for display on window
    const int         posX,                              ///< [in] X coordinate of text for display on window
    const int         posY,                              ///< [in] Y coordinate of text for display on window
    const DWORD       textColor                          ///< [in] text color for display on window
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetClientWindowOverlayText(handle, fontName, fontSize, fontBold, text, posX, posY, textColor);
}

#else
int WINAPI IBSU_SetClientWindowOverlayTextW(
    const int         handle,                            ///< [in] Device handle obtained by IBSU_OpenDevice()
    const WCHAR       *fontName,                         ///< [in] font name for display on window
    const int         fontSize,                          ///< [in] font size for display on window
    const BOOL        fontBold,                          ///< [in] font bold for display on window
    const WCHAR       *text,                             ///< [in] text for display on window
    const int         posX,                              ///< [in] X coordinate of text for display on window
    const int         posY,                              ///< [in] Y coordinate of text for display on window
    const DWORD       textColor                          ///< [in] text color for display on window
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetClientWindowOverlayTextW(handle, fontName, fontSize, fontBold, text, posX, posY, textColor);
}
#endif

int WINAPI IBSU_GenerateZoomOutImage(
    const IBSU_ImageData  inImage,                       ///< [in] Original image
    BYTE                  *outImage,                     ///< [out] Pointer to zoom-out image data buffer \n
    ///<       Memory must be provided by caller
    const int             outWidth,                      ///< [in]  Width for zoom-out image
    const int             outHeight,                     ///< [in]  Height for zoom-out image
    const BYTE            bkColor                        ///< [in]  Background color for remain area from zoom-out image
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GenerateZoomOutImage(inImage, outImage, outWidth, outHeight, bkColor);
}

#ifndef WINCE
int WINAPI IBSU_SaveBitmapImage(
    LPCSTR                filePath,                      ///< [in]  File path to save bitmap
    const BYTE            *imgBuffer,                    ///< [in]  Point to image data
    const DWORD           width,                         ///< [in]  Image width
    const DWORD           height,                        ///< [in]  Image height
    const int             pitch,                         ///< [in]  Image line pitch (in Bytes).\n
    ///        Positive values indicate top down line order,
    ///        Negative values mean bottom up line order
    const double          resX,                          ///< [in]  Image horizontal resolution (in PPI)
    const double          resY                          ///< [in]  Image vertical resolution (in PPI)
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SaveBitmapImage(filePath, imgBuffer, width, height, pitch, resX, resY);
}

#else
int WINAPI IBSU_SaveBitmapImageW(
    LPCWSTR               filePath,                      ///< [in]  File path to save bitmap
    const BYTE            *imgBuffer,                    ///< [in]  Point to image data
    const DWORD           width,                         ///< [in]  Image width
    const DWORD           height,                        ///< [in]  Image height
    const int             pitch,                         ///< [in]  Image line pitch (in Bytes).\n
    ///        Positive values indicate top down line order,
    ///        Negative values mean bottom up line order
    const double          resX,                          ///< [in]  Image horizontal resolution (in PPI)
    const double          resY                          ///< [in]  Image vertical resolution (in PPI)
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SaveBitmapImageW(filePath, imgBuffer, width, height, pitch, resX, resY);
}
#endif

int WINAPI IBSU_AsyncOpenDevice(
    const int  deviceIndex                               ///< [in]  Zero-based device index for device to init
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_AsyncOpenDevice(deviceIndex, TRUE);
}

int WINAPI IBSU_BGetImage(
    const int	                  handle,                  ///< [in]  Device handle
    IBSU_ImageData              *pImage,                 ///< [out] Image data of preview image or result image
    IBSU_ImageType              *pImageType,             ///< [out] Image type
    IBSU_ImageData              *pSplitImageArray,       ///< [out] Finger array to be splited from result image (two-fingers, four-fingers)
    int                         *pSplitImageArrayCount,  ///< [out] Array count to be splited from result image (two-fingers, four-fingers)
    IBSU_FingerCountState       *pFingerCountState,      ///< [out] Finger count state
    IBSU_FingerQualityState     *pQualityArray,          ///< [out] Finger quality state
    int                         *pQualityArrayCount      ///< [out] Finger quality count
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_BGetImage(handle, pImage, pImageType, pSplitImageArray, pSplitImageArrayCount,
		pFingerCountState, pQualityArray, pQualityArrayCount);
}

int WINAPI IBSU_BGetImageEx(
	const int                handle,
	int                     *pImageStatus,
	IBSU_ImageData          *pImage,
	IBSU_ImageType          *pImageType,
	int                     *pDetectedFingerCount,
	IBSU_ImageData          *pSegmentImageArray,
	IBSU_SegmentPosition    *pSegmentPositionArray,
	int                     *pSegmentImageArrayCount,
	IBSU_FingerCountState   *pFingerCountState,
	IBSU_FingerQualityState *pQualityArray,
	int                     *pQualityArrayCount
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_BGetImageEx(handle, pImageStatus, pImage, pImageType, pDetectedFingerCount,
		pSegmentImageArray, pSegmentPositionArray, pSegmentImageArrayCount,
		pFingerCountState, pQualityArray, pQualityArrayCount);
}

int WINAPI IBSU_BGetInitProgress(
    const int	  deviceIndex,                             ///< [in]  Device index
    BOOL        *pIsComplete,                            ///< [out] Is that complete the initialize device
    int         *pHandle,                                ///< [out] Device handle
    int         *pProgressValue                          ///< [out] progress value of initialize device
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_BGetInitProgress(deviceIndex, pIsComplete, pHandle, pProgressValue);
}

int WINAPI IBSU_BGetClearPlatenAtCapture(
    const int	            handle,                        ///< [in]  Device handle
    IBSU_PlatenState      *pPlatenState                  ///< [out] Platen state
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_BGetClearPlatenAtCapture(handle, pPlatenState);
}

int WINAPI IBSU_BGetRollingInfo(
    const int		    handle,                             ///< [in]  Device handle
    IBSU_RollingState *pRollingState,                     ///< [out] Rolling state
    int               *pRollingLineX                      ///< [out] x-coordinate of Rolling line for drawing
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_BGetRollingInfo(handle, pRollingState, pRollingLineX);
}

int WINAPI IBSU_BGetRollingInfoEx
    (const int          handle,
     IBSU_RollingState *pRollingState,
     int               *pRollingLineX,
	 int               *pRollingDirection,
	 int               *pRollingWidth)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_BGetRollingInfoEx(handle, pRollingState, pRollingLineX, pRollingDirection, pRollingWidth);
}

#ifndef WINCE
int WINAPI IBSU_OpenDeviceEx(
    const int	    deviceIndex,                           ///< [in]  Zero-based device index for device to init
    LPCSTR        uniformityMaskPath,                    ///< [in]  Uniformatity mask path in your computer
    ///<       If the file does not exist or different in path, the DLL makes a new file in path.
    const BOOL    asyncOpen,                             ///< [in]  async open device(TRUE) or sync open device(FALSE)
    int           *pHandle                               ///< [out] Function returns device handle to be used for subsequent function calls \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_OpenDeviceEx(deviceIndex, uniformityMaskPath, TRUE, asyncOpen, pHandle);
}

#else
int WINAPI IBSU_OpenDeviceExW(
    const int	    deviceIndex,                           ///< [in]  Zero-based device index for device to init
    LPCWSTR        uniformityMaskPath,                    ///< [in]  Uniformatity mask path in your computer
    ///<       If the file does not exist or different in path, the DLL makes a new file in path.
    const BOOL    asyncOpen,                             ///< [in]  async open device(TRUE) or sync open device(FALSE)
    int           *pHandle                               ///< [out] Function returns device handle to be used for subsequent function calls \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_OpenDeviceExW(deviceIndex, uniformityMaskPath, TRUE, asyncOpen, pHandle);
}
#endif

int WINAPI IBSU_GetIBSM_ResultImageInfo(
    const int		        handle,                         ///< [in]  Device handle
    IBSM_FingerPosition   fingerPosition,                 ///< [in]  Finger position. e.g Right Thumb, Right Index finger
    IBSM_ImageData        *pResultImage,                  ///< [out] Result image
    IBSM_ImageData        *pSplitResultImage,             ///< [out] Splited image from Result image
    int                   *pSplitResultImageCount         ///< [out] Splited image count (e.g Two-fingers is 2)
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetIBSM_ResultImageInfo(handle, fingerPosition, pResultImage, pSplitResultImage, pSplitResultImageCount);
}

int WINAPI IBSU_GetNFIQScore(
    const int		        handle,                         ///< [in]  Device handle
    const BYTE            *imgBuffer,                     ///< [in]  Point to image data
    const DWORD           width,                          ///< [in]  Image width
    const DWORD           height,                         ///< [in]  Image height
    const BYTE            bitsPerPixel,                   ///< [in]  Number of Bits per pixel
    int                   *pScore                         ///< [out] NFIQ score
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetNFIQScore(handle, imgBuffer, width, height, bitsPerPixel, pScore);
}

int WINAPI IBSU_GenerateZoomOutImageEx(
    const BYTE            *pInImage,                     ///< [in]  Original image
    const int             inWidth,                       ///< [in]  Width of original image
    const int             inHeight,                      ///< [in]  Height of original image
    BYTE                  *outImage,                     ///< [out] Pointer to zoom-out image data buffer \n
    ///<       Memory must be provided by caller
    const int             outWidth,                      ///< [in]  Width for zoom-out image
    const int             outHeight,                     ///< [in]  Height for zoom-out image
    const BYTE            bkColor                        ///< [in]  Background color for remain area from zoom-out image
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GenerateZoomOutImageEx(pInImage, inWidth, inHeight, outImage, outWidth, outHeight, bkColor);
}

int WINAPI IBSU_ReleaseCallbacks(
      const int		       handle,                      ///< [in] Device handle obtained by IBSU_DeviceOpen()
      const IBSU_Events    events                       ///< [in] Enum value to the notification function 
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_ReleaseCallbacks(handle, events);
}

int WINAPI IBSU_SaveBitmapMem(
	  const BYTE                *inImage,                  ///< [in]  Point to image data (Gray scale image)
      const DWORD               inWidth,                   ///< [in]  Image width
      const DWORD               inHeight,                  ///< [in]  Image height
      const int                 inPitch,                   ///< [in]  Image line pitch (in Bytes).\n
                                                           ///<       Positive values indicate top down line order,
                                                           ///<       Negative values mean bottom up line order
      const double              inResX,                    ///< [in]  Image horizontal resolution (in PPI)
      const double              inResY,                    ///< [in]  Image vertical resolution (in PPI)
      BYTE                      *outBitmapBuffer,          ///< [out] Pointer to output image data buffer which is set image format and zoom-out factor \n
                                                           ///<       Memory must be provided by caller
      const IBSU_ImageFormat    outImageFormat,            ///< [in]  Set Image color format for output image
	  const DWORD               outWidth,                  ///< [in]  Width for zoom-out image
	  const DWORD               outHeight,                 ///< [in]  Height for zoom-out image
      const BYTE                bkColor                    ///< [in]  Background color for remain area from zoom-out image
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SaveBitmapMem(inImage, inWidth, inHeight, inPitch, inResX, inResY,
		outBitmapBuffer, outImageFormat, outWidth, outHeight, bkColor);
}

int WINAPI IBSU_ShowOverlayObject(
      const int         handle,                            ///< [in] Device handle obtained by IBSU_OpenDevice()
      const int         overlayHandle,                     ///< [in] Overlay handle obtained by overlay functions
      const BOOL        show                               ///< [in] If TRUE, the overlay will be shown on client window
                                                           ///       If FALSE, the overlay will be hidden on client window
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_ShowOverlayObject(handle, overlayHandle, show);
}

int WINAPI IBSU_ShowAllOverlayObject(
      const int         handle,                            ///< [in] Device handle obtained by IBSU_OpenDevice()
      const BOOL        show                               ///< [in] If TRUE, the overlay will be shown on client window
                                                           ///       If FALSE, the overlay will be hidden on client window
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_ShowAllOverlayObject(handle, show);
}

int WINAPI IBSU_RemoveOverlayObject(
      const int         handle,                            ///< [in] Device handle obtained by IBSU_OpenDevice()
      const int         overlayHandle                      ///< [in] Overlay handle obtained by overlay functions
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_RemoveOverlayObject(handle, overlayHandle);
}

int WINAPI IBSU_RemoveAllOverlayObject(
      const int         handle                             ///< [in] Device handle obtained by IBSU_OpenDevice()
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_RemoveAllOverlayObject(handle);
}

#ifndef WINCE
int WINAPI IBSU_AddOverlayText(
      const int         handle,                            ///< [in]  Device handle obtained by IBSU_OpenDevice()
      int               *pOverlayHandle,                   ///< [out] Function returns overlay handle to be used for client windows functions calls \n
                                                           ///<       Memory must be provided by caller
      const char        *fontName,                         ///< [in]  font name for display on window
	  const int         fontSize,                          ///< [in]  font size for display on window
	  const BOOL        fontBold,                          ///< [in]  font bold for display on window
	  const char        *text,                             ///< [in]  text for display on window
	  const int         posX,                              ///< [in]  X coordinate of text for display on window
	  const int         posY,                              ///< [in]  Y coordinate of text for display on window
	  const DWORD       textColor                          ///< [in]  text color for display on window
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_AddOverlayText(handle, pOverlayHandle, fontName, fontSize, fontBold,
		text, posX, posY, textColor);
}

#else
int WINAPI IBSU_AddOverlayTextW(
      const int         handle,                            ///< [in]  Device handle obtained by IBSU_OpenDevice()
      int               *pOverlayHandle,                   ///< [out] Function returns overlay handle to be used for client windows functions calls \n
                                                           ///<       Memory must be provided by caller
      const wchar_t     *fontName,                         ///< [in]  font name for display on window
	  const int         fontSize,                          ///< [in]  font size for display on window
	  const BOOL        fontBold,                          ///< [in]  font bold for display on window
	  const wchar_t     *text,                             ///< [in]  text for display on window
	  const int         posX,                              ///< [in]  X coordinate of text for display on window
	  const int         posY,                              ///< [in]  Y coordinate of text for display on window
	  const DWORD       textColor                          ///< [in]  text color for display on window
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_AddOverlayTextW(handle, pOverlayHandle, fontName, fontSize, fontBold,
		text, posX, posY, textColor);
}
#endif

#ifndef WINCE
int WINAPI IBSU_ModifyOverlayText(
      const int         handle,                             ///< [in]  Device handle obtained by IBSU_OpenDevice()
      const int         overlayHandle,                      ///< [in]  Overlay handle to be modified
      const char        *fontName,                          ///< [in]  font name for display on window
	  const int         fontSize,                           ///< [in]  font size for display on window
	  const BOOL        fontBold,                           ///< [in]  font bold for display on window
	  const char        *text,                              ///< [in]  text for display on window
	  const int         posX,                               ///< [in]  X coordinate of text for display on window
	  const int         posY,                               ///< [in]  Y coordinate of text for display on window
	  const DWORD       textColor                           ///< [in]  text color for display on window
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_ModifyOverlayText(handle, overlayHandle, fontName, fontSize, fontBold,
		text, posX, posY, textColor);
}

#else
int WINAPI IBSU_ModifyOverlayTextW(
      const int         handle,                             ///< [in]  Device handle obtained by IBSU_OpenDevice()
      const int         overlayHandle,                      ///< [in]  Overlay handle to be modified
      const wchar_t     *fontName,                          ///< [in]  font name for display on window
	  const int         fontSize,                           ///< [in]  font size for display on window
	  const BOOL        fontBold,                           ///< [in]  font bold for display on window
	  const wchar_t     *text,                              ///< [in]  text for display on window
	  const int         posX,                               ///< [in]  X coordinate of text for display on window
	  const int         posY,                               ///< [in]  Y coordinate of text for display on window
	  const DWORD       textColor                           ///< [in]  text color for display on window
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_ModifyOverlayTextW(handle, overlayHandle, fontName, fontSize, fontBold,
		text, posX, posY, textColor);
}
#endif

int WINAPI IBSU_AddOverlayLine(
      const int         handle,                            ///< [in]  Device handle obtained by IBSU_OpenDevice()
      int               *pOverlayHandle,                   ///< [out] Function returns overlay handle to be used for client windows functions calls \n
                                                           ///<       Memory must be provided by caller
      const int         x1,                                ///< [in]  X coordinate of starting point of line
	  const int         y1,                                ///< [in]  Y coordinate of starting point of line
	  const int         x2,                                ///< [in]  X coordinate of ending point of line
	  const int         y2,                                ///< [in]  Y coordinate of ending point of line
	  const int         lineWidth,                         ///< [in]  line width
	  const DWORD       lineColor                          ///< [in]  line color
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_AddOverlayLine(handle, pOverlayHandle, x1, y1, x2, y2, lineWidth, lineColor);
}

int WINAPI IBSU_ModifyOverlayLine(
      const int         handle,                             ///< [in]  Device handle obtained by IBSU_OpenDevice()
      const int         overlayHandle,                      ///< [in]  Overlay handle to be modified
      const int         x1,                                 ///< [in]  X coordinate of starting point of line
	  const int         y1,                                 ///< [in]  Y coordinate of starting point of line
	  const int         x2,                                 ///< [in]  X coordinate of ending point of line
	  const int         y2,                                 ///< [in]  Y coordinate of ending point of line
	  const int         lineWidth,                          ///< [in]  line width
	  const DWORD       lineColor                           ///< [in]  line color
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_ModifyOverlayLine(handle, overlayHandle, x1, y1, x2, y2, lineWidth, lineColor);
}

int WINAPI IBSU_AddOverlayQuadrangle(
      const int         handle,                            ///< [in]  Device handle obtained by IBSU_OpenDevice()
      int               *pOverlayHandle,                   ///< [out] Function returns overlay handle to be used for client windows functions calls \n
                                                           ///<       Memory must be provided by caller
      const int         x1,                                ///< [in]  X coordinate of starting point of quadrangle
	  const int         y1,                                ///< [in]  Y coordinate of starting point of quadrangle
	  const int         x2,                                ///< [in]  X coordinate of 1st corner of quadrangle
	  const int         y2,                                ///< [in]  Y coordinate of 1st corner of quadrangle
	  const int         x3,                                ///< [in]  X coordinate of 2nd corner of quadrangle
	  const int         y3,                                ///< [in]  Y coordinate of 2nd corner of quadrangle
	  const int         x4,                                ///< [in]  X coordinate of 3rd corner of quadrangle
	  const int         y4,                                ///< [in]  Y coordinate of 3rd corner of quadrangle
	  const int         lineWidth,                         ///< [in]  line width of quadrangle
	  const DWORD       lineColor                          ///< [in]  line color of quadrangle
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_AddOverlayQuadrangle(handle, pOverlayHandle, x1, y1, x2, y2, x3, y3, x4, y4, lineWidth, lineColor);
}

int WINAPI IBSU_ModifyOverlayQuadrangle(
      const int         handle,                             ///< [in]  Device handle obtained by IBSU_OpenDevice()
      const int         overlayHandle,                      ///< [in]  Overlay handle to be modified
      const int         x1,                                 ///< [in]  X coordinate of starting point of quadrangle
	  const int         y1,                                 ///< [in]  Y coordinate of starting point of quadrangle
	  const int         x2,                                 ///< [in]  X coordinate of 1st corner of quadrangle
	  const int         y2,                                 ///< [in]  Y coordinate of 1st corner of quadrangle
	  const int         x3,                                 ///< [in]  X coordinate of 2nd corner of quadrangle
	  const int         y3,                                 ///< [in]  Y coordinate of 2nd corner of quadrangle
	  const int         x4,                                 ///< [in]  X coordinate of 3rd corner of quadrangle
	  const int         y4,                                 ///< [in]  Y coordinate of 3rd corner of quadrangle
	  const int         lineWidth,                          ///< [in]  line width of quadrangle
	  const DWORD       lineColor                           ///< [in]  line color of quadrangle
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_ModifyOverlayQuadrangle(handle, overlayHandle, x1, y1, x2, y2, x3, y3, x4, y4, lineWidth, lineColor);
}

int WINAPI IBSU_AddOverlayShape(
      const int						 handle,                ///< [in]  Device handle obtained by IBSU_OpenDevice()
      int							 *pOverlayHandle,       ///< [out] Function returns overlay handle to be used for client windows functions calls \n
                                                            ///<       Memory must be provided by caller
      const IBSU_OverlayShapePattern shapePattern,          ///< [in]  X coordinate of starting point of the overlay shape
	  const int						 x1,                    ///< [in]  Y coordinate of starting point of the overlay shape
	  const int						 y1,                    ///< [in]  Y coordinate of starting point of the overlay shape
	  const int						 x2,                    ///< [in]  X coordinate of ending point of the overlay shape
	  const int						 y2,                    ///< [in]  Y coordinate of ending point of the overlay shape
	  const int						 lineWidth,             ///< [in]  line width of the overlay shape
	  const DWORD					 lineColor,             ///< [in]  line color of the overlay shape
	  const int						 reserved_1,            ///< [in]  Reserved
	  const int						 reserved_2             ///< [in]  Reserved
	                                                        ///<       If you set shapePattern to ENUM_IBSU_OVERLAY_SHAPE_ARROW
															///<           * reserved_1 can use the width(in pixels) of the full base of the arrowhead
															///<           * reserved_2 can use the angle(in radians) at the arrow tip between the two sides of the arrowhead
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_AddOverlayShape(handle, pOverlayHandle, shapePattern, x1, y1, x2, y2,
		lineWidth, lineColor, reserved_1, reserved_2);
}

int WINAPI IBSU_ModifyOverlayShape(
      const int						 handle,                ///< [in]  Device handle obtained by IBSU_OpenDevice()
      const int						 overlayHandle,         ///< [in]  Overlay handle to be modified
      const IBSU_OverlayShapePattern shapePattern,          ///< [in]  X coordinate of starting point of the overlay shape
	  const int						 x1,                    ///< [in]  Y coordinate of starting point of the overlay shape
	  const int						 y1,                    ///< [in]  Y coordinate of starting point of the overlay shape
	  const int						 x2,                    ///< [in]  X coordinate of ending point of the overlay shape
	  const int						 y2,                    ///< [in]  Y coordinate of ending point of the overlay shape
	  const int						 lineWidth,             ///< [in]  line width of the overlay shape
	  const DWORD					 lineColor,             ///< [in]  line color of the overlay shape
	  const int						 reserved_1,            ///< [in]  Reserved
	  const int						 reserved_2             ///< [in]  Reserved
	                                                        ///<       If you set shapePattern to ENUM_IBSU_OVERLAY_SHAPE_ARROW
															///<           * reserved_1 can use the width(in pixels) of the full base of the arrowhead
															///<           * reserved_2 can use the angle(in radians) at the arrow tip between the two sides of the arrowhead
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_ModifyOverlayShape(handle, overlayHandle, shapePattern, x1, y1, x2, y2,
		lineWidth, lineColor, reserved_1, reserved_2);
}

int WINAPI IBSU_WSQEncodeMem(
    const BYTE     *image,
    const int       width,
    const int       height,
    const int       pitch,
    const int       bitsPerPixel,
	const int       pixelPerInch,
	const double    bitRate,
	const char     *commentText,
	BYTE          **compressedData,
	int            *compressedLength
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_WSQEncodeMem(image, width, height, pitch, (BYTE)bitsPerPixel, pixelPerInch, bitRate,
		commentText, compressedData, compressedLength);
}

#ifndef WINCE
int WINAPI IBSU_WSQEncodeToFile(
    LPCSTR         filePath,
	const BYTE     *image,
    const int       width,
    const int       height,
    const int       pitch,
    const int       bitsPerPixel,
	const int       pixelPerInch,
	const double    bitRate,
	const char     *commentText
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_WSQEncodeToFile(filePath, image, width, height, pitch, (BYTE)bitsPerPixel, pixelPerInch, bitRate,
		commentText);
}

#else
int WINAPI IBSU_WSQEncodeToFileW(
    LPCWSTR         filePath,
	const BYTE     *image,
    const int       width,
    const int       height,
    const int       pitch,
    const int       bitsPerPixel,
	const int       pixelPerInch,
	const double    bitRate,
	const WCHAR    *commentText
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_WSQEncodeToFileW(filePath, image, width, height, pitch, (BYTE)bitsPerPixel, pixelPerInch, bitRate,
		commentText);
}
#endif

int WINAPI IBSU_WSQDecodeMem(
    const BYTE     *compressedImage,
    const int       compressedLength,
    BYTE          **decompressedImage,
    int            *outWidth,
    int            *outHeight,
    int            *outPitch,
    int            *outBitsPerPixel,
	int            *outPixelPerInch
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_WSQDecodeMem(compressedImage, compressedLength,
		decompressedImage, outWidth, outHeight, outPitch, outBitsPerPixel, outPixelPerInch);
}

#ifndef WINCE
int WINAPI IBSU_WSQDecodeFromFile(
    LPCSTR         filePath,
    BYTE          **decompressedImage,
    int            *outWidth,
    int            *outHeight,
    int            *outPitch,
    int            *outBitsPerPixel,
	int            *outPixelPerInch
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_WSQDecodeFromFile(filePath, decompressedImage, outWidth, outHeight, outPitch,
		outBitsPerPixel, outPixelPerInch);
}

#else
int WINAPI IBSU_WSQDecodeFromFileW(
    LPCWSTR        filePath,
    BYTE          **decompressedImage,
    int            *outWidth,
    int            *outHeight,
    int            *outPitch,
    int            *outBitsPerPixel,
	int            *outPixelPerInch
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_WSQDecodeFromFileW(filePath, decompressedImage, outWidth, outHeight, outPitch,
		outBitsPerPixel, outPixelPerInch);
}
#endif

int WINAPI IBSU_FreeMemory(
    void           *memblock
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_FreeMemory(memblock);
}

#ifndef WINCE
int WINAPI IBSU_SavePngImage
    (LPCSTR       filePath,
     const BYTE  *image,
     const DWORD  width,
     const DWORD  height,
     const int    pitch,
     const double resX,
     const double resY
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SavePngImage(filePath, image, width, height, pitch, resX, resY);
}

#else
int WINAPI IBSU_SavePngImageW
    (LPCWSTR      filePath,
     const BYTE  *image,
     const DWORD  width,
     const DWORD  height,
     const int    pitch,
     const double resX,
     const double resY
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SavePngImageW(filePath, image, width, height, pitch, resX, resY);
}
#endif

#ifndef WINCE
int WINAPI IBSU_SaveJP2Image
    (LPCSTR        filePath,
     const BYTE   *image,
     const DWORD   width,
     const DWORD   height,
     const int     pitch,
     const double  resX,
     const double  resY,
	 const int     fQuality
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SaveJP2Image(filePath, image, width, height, pitch, resX, resY, fQuality);
}

#else
int WINAPI IBSU_SaveJP2ImageW
    (LPCWSTR       filePath,
     const BYTE   *image,
     const DWORD   width,
     const DWORD   height,
     const int     pitch,
     const double  resX,
     const double  resY,
	 const int     fQuality
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SaveJP2ImageW(filePath, image, width, height, pitch, resX, resY, fQuality);
}
#endif

int WINAPI IBSU_RedrawClientWindow
    (const int   handle/*,
     const DWORD flags*/
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_RedrawClientWindow(handle, 0xFFFFFFFF);
}

int WINAPI IBSU_UnloadLibrary()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_UnloadLibrary();
}


int WINAPI IBSU_CombineImage
    (const IBSU_ImageData InImage1,
	const IBSU_ImageData InImage2,
	IBSU_CombineImageWhichHand WhichHand,
	IBSU_ImageData *OutImage
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_CombineImage(InImage1, InImage2, WhichHand, OutImage);
}

int WINAPI IBSU_GetOperableBeeper
    (const int     handle,
     IBSU_BeeperType *pBeeperType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetOperableBeeper(handle, pBeeperType);
}

int WINAPI IBSU_SetBeeper
    (const int              handle,
     const IBSU_BeepPattern beepPattern,
	 const DWORD            soundTone,
	 const DWORD            duration,
     const DWORD            reserved_1,
     const DWORD            reserved_2
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetBeeper(handle, beepPattern, soundTone, duration, reserved_1, reserved_2);
}

int WINAPI IBSU_CheckWetFinger
	(const int				handle,
	 const IBSU_ImageData   inImage
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_CheckWetFinger(handle, inImage);
}

int WINAPI IBSU_GetImageWidth
	(const int				handle,
	 const IBSU_ImageData  inImage,
	 int				   *Width_MM
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetImageWidth(handle, inImage, Width_MM);
}

#ifndef WINCE
int WINAPI IBSU_IsWritableDirectory
	(LPCSTR	            dirpath,
	 const BOOL			needCreateSubFolder
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_IsWritableDirectory(dirpath, needCreateSubFolder);
}
#else
int WINAPI IBSU_IsWritableDirectory
	(LPCWSTR	        dirpath,
	 const BOOL			needCreateSubFolder
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_IsWritableDirectoryW(dirpath, needCreateSubFolder);
}
#endif

int WINAPI IBSU_GenerateDisplayImage(
    const BYTE            *pInImage,
    const int             inWidth,
    const int             inHeight,
    BYTE                  *outImage,
    const int             outWidth,
    const int             outHeight,
    const BYTE            outBkColor,
	const IBSU_ImageFormat outFormat,
	const int             outQualityLevel,
    const BOOL            outVerticalFlip
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GenerateDisplayImage(pInImage, inWidth, inHeight,
        outImage, outWidth, outHeight, outBkColor, outFormat, outQualityLevel, outVerticalFlip);
}

#endif




//////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(__IBSCAN_SDK__)
int WINAPI IBSCAN_Main_GetAPIVersion(
		IBScanApiVersion *info							///< [out] API version information \n
														///<       Memory must be provided by caller
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetSDKVersion(info);
}

int WINAPI IBSCAN_Main_GetDeviceCount(
		int *deviceCount								///< [out] Number of connected devices \n
														///  Memory must be provided by caller
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetDeviceCount(deviceCount);
}

int WINAPI IBSCAN_Main_GetDeviceInfo(
		const int			deviceIndex,				///< [in] Zero-based device index for device to lookup
		IBScanDeviceInfo	*deviceInfo					///< [out] Basic device information \n
														///<       Memory must be provided by caller
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    IBSU_DeviceDescA deviceDescA;
    int nRc;

    nRc = theApp.DLL_GetDeviceDescription(deviceIndex, &deviceDescA);
    memcpy(deviceInfo, &deviceDescA, sizeof(IBScanDeviceInfo));

    return nRc;
}

int WINAPI IBSCAN_Main_RegisterCallbackInitProgress(
      IBSCAN_CallbackInitProgress callback,           ///< [in] Pointer to the notification function 
      void                   *context                 ///< [in] Pointer to user context; this value is used as parameter for callback
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return theApp.DLL_RegisterCallbacks(0, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, (void*)callback, context);
}

int WINAPI IBSCAN_Main_RegisterCallbackDeviceCount(
		IBSCAN_CallbackDeviceCount	callback,
		void						*context
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return theApp.DLL_RegisterCallbacks(0, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, (void*)callback, context);
}

int WINAPI IBSCAN_Main_Initialize(
		const int	deviceIndex,						///< [in]  Zero-based device index for device to init
		const BOOL	useUniformityMask,					///< [in]  Set device in uniformity mask mode (if set to TRUE)
		int			*handle								///< [out] Function returns device handle to be used for subsequent function calls \n
														///<       Memory must be provided by caller
  )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_OpenDevice(deviceIndex, useUniformityMask, handle);
}

int WINAPI IBSCAN_Main_Release(
		const int	handle								///< [in] Device handle obtained by IBSCAN_Main_Initialize()
//		const BOOL	sendToStandby						///< [in] Set device in standby mode (if set to TRUE)
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_CloseDevice(handle);
}

int WINAPI IBSCAN_Main_ReleaseAll(
//		const BOOL sendToStandby						///< [in] If TRUE then set devices in standby mode
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_CloseAllDevice();
}

int WINAPI IBSCAN_Main_IsInitialized(
		const int  handle								///< [in] Device handle obtained by IBSCAN_Main_Initialize()
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_IsDeviceOpened(handle);
}

int WINAPI IBSCAN_Main_GetProperty(
		const int				handle,					///< [in]  Device handle obtained by IBSCAN_Main_Initialize()
		const IBScanPropertyId	propertyId,				///< [in]  Property identifier to get value for
		LPSTR					propertyValue			///< [out] String returning property value \n
														///<       Memory must be provided by caller
														///<       (needs to be able to hold @ref IBSCAN_MAX_STR_LEN characters)
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetProperty(handle, propertyId, propertyValue);
}

int WINAPI IBSCAN_Main_SetProperty(
		const int				handle,					///< [in] Device handle obtained by IBSCAN_Main_Initialize()
		const IBScanPropertyId	propertyId,				///< [in] Property identifier to set value for
		LPCSTR					propertyValue			///< [in] String containing property value
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetProperty(handle, propertyId, propertyValue);
}

int WINAPI IBSCAN_Main_RegisterCallbackCommunicationBreak(
      const int      handle,                          ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
      IBSCAN_Callback callback,                        ///< [in] Pointer to the notification function
      void           *context                         ///< [in] Pointer to user context; this value is used as parameter for callback
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_RegisterCallbacks(0, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, (void*)callback, context);
}

int WINAPI IBSCAN_Main_EnableTraceLog(
      const BOOL on  ///< [in] TRUE to turn trace log on; FALSE to turn trace log off
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    return theApp.DLL_EnableTraceLog(on);
}



/////////////////////////////////////////////////////////////////////////////
// Image Acquisition Related Interface Functions
/////////////////////////////////////////////////////////////////////////////

int WINAPI IBSCAN_Capture_IsModeAvailable(
		const int					handle,				///< [in]  Device handle obtained by IBSCAN_Main_Initialize()
		const IBScanImageType		imageType,			///< [in]  Image type to verify
		const IBScanImageResolution	imageResolution,	///< [in]  Requested capture resolution
		BOOL						*isAvailable		///< [out] Returns TRUE if mode is available \n
														///<       Memory must be provided by caller
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_IsCaptureAvailable(handle, imageType, imageResolution, isAvailable);
}

int WINAPI IBSCAN_Capture_SetMode(
		const int						handle,			///< [in]  Device handle obtained by IBSCAN_Main_Initialize()
		const IBScanImageType			imageType,		///< [in]  Image type to capture
		const IBScanImageResolution	imageResolution,	///< [in]  Requested capture resolution
//		const IBScanImageOrientation	lineOrder,		///< [in]  Required result image line order
		const DWORD					captureOptions,		///< [in]  Bit coded capture options to use (see @ref CaptureOptions)
		int							*resultWidth,		///< [out] Returns width of captured result image in pixels \n
														///<       Memory must be provided by caller
		int							*resultHeight,		///< [out] Returns height of captured result image in pixels \n
														///<       Memory must be provided by caller
		int							*baseResolutionX,	///< [out] Returns image horizontal base resolution before processing \n
														///<       Memory must be provided by caller
		int							*baseResolutionY	///< [out] Returns image vertical base resolution before processing \n
														///<       Memory must be provided by caller
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_Capture_SetMode(handle, imageType, imageResolution, 
        captureOptions, resultWidth, resultHeight, baseResolutionX, baseResolutionY);
}

int WINAPI IBSCAN_Capture_Start(
		const int  handle,								///< [in] Device handle obtained by IBSCAN_Main_Initialize()
		const int  numberOfObjects						///< [in] Number of expected objects (fingertips + palm areas) in the image.
														///<      It is used if option @ref IBSCAN_OPTION_AUTO_CAPTURE is set
														///<      at @ref IBSCAN_Capture_SetMode(). Please refer to the image types
														///<      @ref enumLScanImageType for details about valid values.
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return theApp.DLL_Capture_Start(handle, numberOfObjects);
}

int WINAPI IBSCAN_Capture_Abort(
		const int handle								///< [in] Device handle obtained by IBSCAN_Main_Initialize()
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_CancelCaptureImage(handle);
}

int WINAPI IBSCAN_Capture_IsActive(
		const int handle,								///< [in]  Device handle obtained by IBSCAN_Main_Initialize()
		BOOL      *isActive								///< [out] Returns TRUE if acquisition is in progress
														///<       (preview or result image acquisition) \n
														///<       Memory must be provided by caller
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_IsCaptureActive(handle, isActive);
}

int WINAPI IBSCAN_Capture_GetContrast(
      const int handle,                               ///< [in]  Device handle obtained by IBSCAN_Main_Initialize()
      int       *contrastValue                        ///< [out] Contrast value (range: 0 <= value <= @ref IBSCAN_MAX_CONTRAST_VALUE) \n
                                                      ///<       Memory must be provided by caller
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetContrast(handle, contrastValue);
}

int WINAPI IBSCAN_Capture_SetContrast(
      const int handle,                               ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
      const int contrastValue                         ///< [in] Contrast value (range: 0 <= value <= @ref IBSCAN_MAX_CONTRAST_VALUE)
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetContrast(handle, contrastValue);
}

int WINAPI IBSCAN_Capture_GetLEOperationMode(
      const int							handle,                               ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
      IBScanLEOperationMode				*leOperationMode							  ///< [out] Touch sensor operation mode
																			  ///<       Memory must be provided by caller
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetLEOperationMode(handle, leOperationMode);
}

int WINAPI IBSCAN_Capture_SetLEOperationMode(
      const int							handle,                               ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
      const IBScanLEOperationMode		leOperationMode                    ///< [in] Touch sensor operation mode
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetLEOperationMode(handle, leOperationMode);
}

int WINAPI IBSCAN_Capture_RegisterCallbackPreviewImage(
		const int					handle,				///< [in] Device handle obtained by IBSCAN_Main_Initialize()
		IBSCAN_CallbackPreviewImage callback,			///< [in] Pointer to the notification function
		void						*context			///< [in] Pointer to user context; this value is used as parameter for callback
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return theApp.DLL_RegisterCallbacks(handle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, (void*)callback, context);
}

int WINAPI IBSCAN_Capture_RegisterCallbackObjectCount(
		const int					handle,				///< [in] Device handle obtained by IBSCAN_Main_Initialize()
		IBSCAN_CallbackObjectCount callback,			///< [in] Pointer to the notification function 
		void						*context			///< [in] Pointer to user context; this value is used as parameter for callback
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return theApp.DLL_RegisterCallbacks(handle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, (void*)callback, context);
}

int WINAPI IBSCAN_Control_GetTouchInStatus(
      const int	handle,                               ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
	  int       *touchInValue                         ///< [out] touchInValue value (0 : touch off, 1 : touch on) \n
                                                      ///<       Memory must be provided by caller
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_IsTouchedFinger(handle, touchInValue);
}

int WINAPI IBSCAN_Control_GetAvailableLEDs(
      const int     handle,                            ///< [in]  Device handle obtained by LSCAN_Main_Initialize()
      IBScanLedType *ledType,                          ///< [out] Type of LED's \n
                                                       ///<       Memory must be provided by caller
      int           *ledCount,                         ///< [out] Number of LED's \n
                                                       ///<       Memory must be provided by caller
      DWORD         *availableLEDs                     ///< [out] Bit pattern of available LED's \n
                                                       ///<       Memory must be provided by caller
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetOperableLEDs(handle, ledType, ledCount, availableLEDs);
}

int WINAPI IBSCAN_Control_SetActiveLEDs(
      const int							handle,                               ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
      const DWORD						activeLEDs							  ///< [in] set active LEDs
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetLEDs(handle, activeLEDs);
}

int WINAPI IBSCAN_Control_GetActiveLEDs(
      const int							handle,                               ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
      DWORD								*activeLEDs							  ///< [out] get active LEDs
																			  ///<       Memory must be provided by caller
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetLEDs(handle, activeLEDs);
}

/// Return of device information given by device index.
/// @return status code as defined in IBScanApi_err.h
/// @return @ref IBSU_STATUS_OK -> device is ready to be used
int WINAPI IBSCAN_Main_GetDeviceHandleInfo(
		const int	      deviceIndex,						                 ///< [in]  Zero-based device index to get the device handle
        int               *handle,                                           ///< [out] Return device handle
                                                                             ///<       Memory must be provided by caller
        BOOL              *isHandleOpened                                    ///< [out] Check if device handle is opened
                                                                             ///<       Memory must be provided by caller
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_GetDeviceHandleInfo(deviceIndex, handle, isHandleOpened);
}

int WINAPI IBSCAN_RawCapture_Start(
      const int               handle,                     ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
      const int               numberOfObjects             ///< [in] Number of expected objects (fingertips) in the image.
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_RawCapture_Start(handle, numberOfObjects);
}

int WINAPI IBSCAN_RawCapture_Abort(
      const int handle                                ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_RawCapture_Abort(handle);
}

int WINAPI IBSCAN_RawCapture_GetOneFrameImage(
      const int handle                                ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_RawCapture_GetOneFrameImage(handle);
}

int WINAPI IBSCAN_RawCapture_TakePreviewImage(
      const int          handle,                        ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
      const RawImageData rawImage                       ///< [in] raw image obtained by IBSCAN_RawCapture_RegisterCallbackOneFrameImage()
                                                        ///<      or raw merge image of rolling
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_RawCapture_TakePreviewImage(handle, rawImage);
}

int WINAPI IBSCAN_RawCapture_TakeResultImage(
      const int          handle,                        ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
      const RawImageData rawImage                       ///< [in] raw image obtained by IBSCAN_RawCapture_RegisterCallbackOneFrameImage()
                                                        ///<      or raw merge image of rolling
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_RawCapture_TakeResultImage(handle, rawImage);
}

int WINAPI IBSCAN_RawCapture_RegisterCallbackOneFrameImage(
      const int                   handle,               ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
      IBSCAN_CallbackRawImage     callback,             ///< [in] Pointer to the notification function
      void                        *context              ///< [in] Pointer to user context; this value is used as parameter for callback
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return theApp.DLL_RegisterCallbacks(handle, ENUM_IBSU_ESSENTIAL_EVENT_ONE_FRAME_IMAGE, (void*)callback, context);
}

int WINAPI IBSCAN_RawCapture_RegisterCallbackTakePreviewImage(
      const int                   handle,               ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
      IBSCAN_CallbackPreviewImage callback,             ///< [in] Pointer to the notification function
      void                        *context              ///< [in] Pointer to user context; this value is used as parameter for callback
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return theApp.DLL_RegisterCallbacks(handle, ENUM_IBSU_ESSENTIAL_EVENT_TAKE_PREVIEW_IMAGE, (void*)callback, context);
}

int WINAPI IBSCAN_RawCapture_RegisterCallbackTakeResultImage(
      const int                   handle,               ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
      IBSCAN_CallbackPreviewImage callback,             ///< [in] Pointer to the notification function
      void                        *context              ///< [in] Pointer to user context; this value is used as parameter for callback
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return theApp.DLL_RegisterCallbacks(handle, ENUM_IBSU_ESSENTIAL_EVENT_TAKE_RESULT_IMAGE, (void*)callback, context);
}

int WINAPI IBSCAN_Main_InitializeEx(
      const int  deviceIndex,                         ///< [in]  Zero-based device index for device to init
      LPCSTR     uniformityMaskPath,                  ///< [in]  Uniformatity mask path in your computer
                                                      ///<       If the file does not exist or different in path, the DLL makes a new file in path.
      const BOOL useUniformityMask,					  ///< [in]  Uniformity mask that is stored in the flash memory is used
                                                      ///        (When user set this value as a true)
      int        *handle                              ///< [out] Function returns device handle to be used for subsequent function calls \n
                                                      ///<       Memory must be provided by caller
  )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_OpenDeviceEx(deviceIndex, uniformityMaskPath, useUniformityMask, FALSE, handle);
}

int WINAPI IBSCAN_Capture_SetDACRegister(
      const int handle,                               ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
      const int dacValue                              ///< [in] DAC value (range: 0 <= value <= 255)
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return theApp.DLL_SetDACRegister(handle, dacValue);
}

int WINAPI IBSCAN_Main_RegisterCallbackNotifyMessage(
      const int                       handle,               ///< [in] Device handle obtained by IBSCAN_Main_Initialize()
      IBSCAN_CallbackNotifyMessage    callback,             ///< [in] Pointer to the notification function
      void                            *context              ///< [in] Pointer to user context; this value is used as parameter for callback
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return theApp.DLL_RegisterCallbacks(handle, ENUM_IBSU_OPTIONAL_EVENT_NOTIFY_MESSAGE, (void*)callback, context);
}
#endif




int WINAPI IBSU_CombineImageEx
    (const IBSU_ImageData InImage1,
	const IBSU_ImageData InImage2,
	IBSU_CombineImageWhichHand WhichHand,
	IBSU_ImageData *OutImage,
	IBSU_ImageData  *pSegmentImageArray,
	IBSU_SegmentPosition *pSegmentPositionArray,
	int *pSegmentImageArrayCount
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_CombineImageEx(InImage1, InImage2, WhichHand, OutImage, 
									pSegmentImageArray, pSegmentPositionArray, pSegmentImageArrayCount);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
int WINAPI RESERVED_OpenDevice(
    const int  deviceIndex,                         ///< [in]  Zero-based device index for device to init
    const char *pReservedkey,                       ///< [in]  Reserved key which is made manufacturer
    int        *pHandle                             ///< [out] Function returns device handle to be used for subsequent function calls \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_OpenDevice(deviceIndex, pReservedkey, TRUE, pHandle);
}

int WINAPI RESERVED_GetDeviceInfo(
    const int              deviceIndex,               ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char             *pReservedkey,             ///< [in]  Reserved key which is made manufacturer
    RESERVED_DeviceInfo    *pDeviceInfo               ///< [out] Basic device information \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_GetDeviceInfo(deviceIndex, pReservedkey, pDeviceInfo);
}

int WINAPI RESERVED_WriteEEPROM(
    const int     handle,                             ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char    *pReservedkey,                      ///< [in]  Reserved key which is made manufacturer
    const WORD    addr,                               ///< [in]  Address of EEPROM
    const BYTE    *pData,                             ///< [in]  Data buffer
    const int     len                                 ///< [in]  Length of data buffer
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_WriteEEPROM(handle, pReservedkey, addr, pData, len);
}

// Modified IBSU_SetProperty function to write some property values for production
// (SerialNumber, ProductionDate, ServiceDate)
int WINAPI RESERVED_SetProperty(
    const int             handle,                     ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char            *pReservedkey,              ///< [in]  Reserved key which is made manufacturer
    const IBSU_PropertyId propertyId,                 ///< [in] Property identifier to set value for
    LPCSTR                propertyValue               ///< [in] String containing property value
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_SetProperty(handle, pReservedkey, propertyId, propertyValue);
}

int WINAPI RESERVED_SetLEVoltage(
    const int     handle,                             ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char    *pReservedkey,                      ///< [in]  Reserved key which is made manufacturer
    const int	    voltageValue                        ///< [in]  Contrast value (range: 0 <= value <= @ref RESERVED_MAX_LE_VOLTAGE_VALUE)
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_SetLEVoltage(handle, pReservedkey, voltageValue);
}

int WINAPI RESERVED_GetLEVoltage(
    const int     handle,                             ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char    *pReservedkey,                      ///< [in]  Reserved key which is made manufacturer
    int	        *pVoltageValue                      ///< [out] LE voltage value (range: RESERVED_MIN_LE_VOLTAGE_VALUE <= value <= @ref RESERVED_MAX_LE_VOLTAGE_VALUE) \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_GetLEVoltage(handle, pReservedkey, pVoltageValue);
}

int WINAPI RESERVED_BeginCaptureImage(
    const int                   handle,               ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const char                  *pReservedkey,        ///< [in]  Reserved key which is made manufacturer
    const IBSU_ImageType        imageType,            ///< [in]  Image type to capture
    const IBSU_ImageResolution  imageResolution,      ///< [in]  Requested capture resolution
    const DWORD                 captureOptions        ///< [in]  Bit coded capture options to use (see @ref CaptureOptions)
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_BeginCaptureImage(handle, pReservedkey, imageType, imageResolution, captureOptions);
}

int WINAPI RESERVED_GetOneFrameImage(
    const int     handle,                             ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char    *pReservedkey,                      ///< [in]  Reserved key which is made manufacturer
    unsigned char *pRawImage,                         ///< [out] Raw capture image from camera (CIS, ROIC, ASIC and some on)
    ///<       Memory must be provided by caller
    const int     imageLength                         ///< [in]  Image length of Raw capture image
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_GetOneFrameImage(handle, pReservedkey, pRawImage, imageLength);
}

int WINAPI RESERVED_SetFpgaRegister(
    const int              handle,                    ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char             *pReservedkey,             ///< [in]  Reserved key which is made manufacturer
    const unsigned char    address,                   ///< [in]  Address of FPGA register (range: 0 <= value <= 0xFF)
    const unsigned char    value                      ///< [in]  Value of FPGA register (range: 0 <= value <= 0xFF)
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_SetFpgaRegister(handle, pReservedkey, address, value);
}

int WINAPI RESERVED_GetFpgaRegister(
    const int              handle,                    ///< [in]  Device handle obtained by IBSU_DeviceOpen()
    const char             *pReservedkey,             ///< [in]  Reserved key which is made manufacturer
    const unsigned char    address,                   ///< [in]  Address of FPGA register (range: 0 <= value <= 0xFF)
    unsigned char          *pValue                    ///< [out] Value of FPGA register (range: 0 <= value <= 0xFF) \n
    ///<       Memory must be provided by caller
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_GetFpgaRegister(handle, pReservedkey, address, pValue);
}

int WINAPI RESERVED_CreateClientWindow(
    const int         handle,                          ///< [in] Device handle obtained by IBSU_OpenDevice()
    const char        *pReservedkey,                   ///< [in]  Reserved key which is made manufacturer
    const IBSU_HWND   hWindow,                         ///< [in] Windows handle to draw
    const DWORD		left,                            ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
    const DWORD		top,                             ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
    const DWORD		right,                           ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
    const DWORD		bottom,                          ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
    const DWORD       rawImgWidth,                     ///< [in] Width of raw image
    const DWORD       rawImgHeight                     ///< [in] height of raw image
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_CreateClientWindow(handle, pReservedkey, hWindow, left, top, right, bottom, rawImgWidth, rawImgHeight);
}

int WINAPI RESERVED_DrawClientWindow(
    const int         handle,                          ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const char        *pReservedkey,                   ///< [in]  Reserved key which is made manufacturer
    unsigned char     *drawImage                       ///< [in]  image buffer to draw on Client window.
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_DrawClientWindow(handle, pReservedkey, drawImage);
}

int WINAPI RESERVED_UsbBulkOutIn(
    const int             handle,                      ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const char            *pReservedkey,               ///< [in]  Reserved key which is made manufacturer
    const int             outEp,                       ///< [in]  BulkOut endpoint
    const unsigned char   uiCommand,
    unsigned char         *outData,                    ///< [in]  send data (You can use 'NULL')
    const int             outDataLen,
    const int             inEp,                        ///< [in]  BulkIn endpoint
    unsigned char         *inData,                     ///< [in]  receive data (You can use 'NULL')
    const int             inDataLen,
    int                   *nBytesRead                  ///< [out] Received data count (You can use 'NULL')
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_UsbBulkOutIn(handle, pReservedkey, outEp, uiCommand, outData, outDataLen,
		inEp, inData, inDataLen, nBytesRead);
}

int WINAPI RESERVED_InitializeCamera(
    const int             handle,                      ///< [in]  Device handle obtained by IBSU_OpenDevice()
    const char            *pReservedkey                ///< [in]  Reserved key which is made manufacturer
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_InitializeCamera(handle, pReservedkey);
}

int WINAPI RESERVED_ReadEEPROM(
      const int     handle,                             ///< [in]  Device handle obtained by IBSU_DeviceOpen()
      const char    *pReservedkey,                      ///< [in]  Reserved key which is made manufacturer
	  const WORD    addr,                               ///< [in]  Address of EEPROM
	  BYTE          *pData,                             ///< [out] Data buffer
      const int     len                                 ///< [in]  Length of data buffer
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_ReadEEPROM(handle, pReservedkey, addr, pData, len);
}

int WINAPI RESERVED_GetEnhancedImage(
     const int              handle,
     const char            *pReservedkey,
     const IBSU_ImageData   inImage,
     IBSU_ImageData        *enhancedImage,
     int                   *segmentImageArrayCount,
     IBSU_ImageData        *segmentImageArray,
     IBSU_SegmentPosition  *segmentPositionArray)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_GetEnhancedImage(handle, pReservedkey, inImage, enhancedImage, segmentImageArrayCount, segmentImageArray, segmentPositionArray);
}

int WINAPI RESERVED_GetFinalImageByNative(
     const int              handle,
     const char            *pReservedkey,
     IBSU_ImageData	       *finalImage)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_GetFinalImageByNative(handle, pReservedkey, finalImage);
}

#ifdef _WINDOWS
int WINAPI RESERVED_GetSpoofScore(
     const char            *pReservedKey,
     RESERVED_CaptureDeviceID  deviceID,
     const BYTE            *pImage,
     const int             Width,
	 const int             Height,
	 const int             Pitch,
	 int                   *pScore)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_GetSpoofScore(pReservedKey, deviceID, pImage, Width, Height, Pitch, pScore);
}
#endif

int WINAPI RESERVED_GetEncryptedImage(
     const int          handle,
     const char         *pReservedKey,
     unsigned char      *pEncKey,
     IBSU_ImageData     *pRawEncImage)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_GetEncryptedImage(handle, pReservedKey, pEncKey, pRawEncImage);
}

int WINAPI RESERVED_ConvertToDecryptImage(
     const char         *pReservedKey,
     unsigned char      *pEncKey,
     IBSU_ImageData     rawEncImage,
     IBSU_ImageData     *pRawDecImage)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_ConvertToDecryptImage(pReservedKey, pEncKey, rawEncImage, pRawDecImage);
}

/*
int WINAPI RESERVED_ReadMask(
     const int              handle,
     const char            *pReservedkey,
     unsigned char		   *pWaterImage,
	 int				   WaterImageSize)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_ReadMask(handle, pReservedkey, pWaterImage, WaterImageSize);
}

int WINAPI RESERVED_ReadMask_8M(
     const int              handle,
     const char            *pReservedkey,
     unsigned char		   *pWaterImage,
	 int				   WaterImageSize,
	 const int			   Section)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_ReadMask_8M(handle, pReservedkey, pWaterImage, WaterImageSize, Section);
}

int WINAPI RESERVED_WriteMask
    (const int              handle,
     const char            *pReservedKey,
     const BYTE			   *MaskData,
	 const int			   MaskDataLen,
	 int				   *pProgress)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_WriteMask(handle, pReservedKey, MaskData, MaskDataLen, pProgress);
}

int WINAPI RESERVED_WriteMask_8M
    (const int              handle,
     const char            *pReservedKey,
     const BYTE			   *MaskData,
	 const int			   MaskDataLen,
	 const int			   Sector,
	 int				   *pProgress)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_WriteMask_8M(handle, pReservedKey, MaskData, MaskDataLen, Sector, pProgress);
}

int WINAPI RESERVED_ResetDevice
    (const int              handle,
     const char            *pReservedKey)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_ResetDevice(handle, pReservedKey);
}
*/




int CIBScanUltimateApp::DLL_RemoveFingerImage
    (const int                 handle,
     const DWORD               fIndex)
{
	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Enter  IBSU_RemoveFingerImage (handle=%d), fIndex(0x%04X)", handle, fIndex);

	if( nRc == IBSU_STATUS_OK )
	{
		for(DWORD mask = IBSU_FINGER_LEFT_LITTLE ; mask <= IBSU_FINGER_RIGHT_LITTLE ; mask <<= 1)
		{
			switch(fIndex & mask)
			{
			case IBSU_FINGER_LEFT_LITTLE:	memset(&m_TemplateDB[ENUM_IBSU_FINGER_LEFT_LITTLE],		0, sizeof(IBSU_TemplateDB))	; break;
			case IBSU_FINGER_LEFT_RING:		memset(&m_TemplateDB[ENUM_IBSU_FINGER_LEFT_RING],		0, sizeof(IBSU_TemplateDB))	; break;
			case IBSU_FINGER_LEFT_MIDDLE:	memset(&m_TemplateDB[ENUM_IBSU_FINGER_LEFT_MIDDLE],		0, sizeof(IBSU_TemplateDB))	; break;
			case IBSU_FINGER_LEFT_INDEX:	memset(&m_TemplateDB[ENUM_IBSU_FINGER_LEFT_INDEX],		0, sizeof(IBSU_TemplateDB))	; break;
			case IBSU_FINGER_LEFT_THUMB:	memset(&m_TemplateDB[ENUM_IBSU_FINGER_LEFT_THUMB],		0, sizeof(IBSU_TemplateDB))	; break;
			case IBSU_FINGER_RIGHT_THUMB:	memset(&m_TemplateDB[ENUM_IBSU_FINGER_RIGHT_THUMB],		0, sizeof(IBSU_TemplateDB))	; break;
			case IBSU_FINGER_RIGHT_INDEX:	memset(&m_TemplateDB[ENUM_IBSU_FINGER_RIGHT_INDEX],		0, sizeof(IBSU_TemplateDB))	; break;
			case IBSU_FINGER_RIGHT_MIDDLE:	memset(&m_TemplateDB[ENUM_IBSU_FINGER_RIGHT_MIDDLE],	0, sizeof(IBSU_TemplateDB))	; break;
			case IBSU_FINGER_RIGHT_RING:	memset(&m_TemplateDB[ENUM_IBSU_FINGER_RIGHT_RING],		0, sizeof(IBSU_TemplateDB))	; break;
			case IBSU_FINGER_RIGHT_LITTLE:	memset(&m_TemplateDB[ENUM_IBSU_FINGER_RIGHT_LITTLE],	0, sizeof(IBSU_TemplateDB))	; break;
			default: /*nRc = IBSU_ERR_INVALID_PARAM_VALUE; */break;
			}
		}
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Exit  IBSU_RemoveFingerImage");

	return nRc;
}

int CIBScanUltimateApp::DLL_AddFingerImage
    (const int                 handle,
     const IBSU_ImageData      image,
     const DWORD               fIndex,
     const IBSU_ImageType      imageType,
     const BOOL                flagForce)
{
	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;
	IBSM_Template	out_template;
	int				finger_index;

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Enter  IBSU_AddFingerImage (handle=%d), fIndex(0x%04X), image(w=%d,h=%d)", handle, fIndex, image.Width, image.Height);

	if( imageType != ENUM_IBSU_FLAT_SINGLE_FINGER &&
		imageType != ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		goto done;
	}

	if( nRc == IBSU_STATUS_OK )
	{
		memset(out_template.Minutiae, 0, sizeof(unsigned int)*IBSM_MAX_MINUTIAE_SIZE);

		memcpy(pIBUsbManager->m_pMainCapture->m_pAlgo->m_Inter_Img, (unsigned char *)image.Buffer, image.Width*image.Height);
		pIBUsbManager->m_pMainCapture->m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(pIBUsbManager->m_pMainCapture->m_pAlgo->m_Inter_Img, pIBUsbManager->m_pMainCapture->m_pAlgo->m_Inter_Img2, image.Width*image.Height);
		nRc = pIBUsbManager->m_pMainCapture->m_pMatcherAlg->IBSM_ExtractTemplate((const unsigned char*)pIBUsbManager->m_pMainCapture->m_pAlgo->m_Inter_Img2,
																				 (unsigned char*)out_template.Minutiae,
																				 image.Width,
																				 image.Height);

		if(nRc == IBSU_STATUS_OK)
		{
			out_template.Version				= IBSM_TEMPLATE_VERSION_NEW_0;
			out_template.FingerPosition			= fIndex;
			out_template.ImpressionType			= IBSM_IMPRESSION_TYPE_LIVE_SCAN_PLAIN;
			out_template.CaptureDeviceTechID	= IBSM_CAPTURE_DEVICE_ELECTRO_LUMINESCENT;
			out_template.CaptureDeviceTypeID	= pIBUsbManager->m_pMainCapture->m_UsbDeviceInfo.captureDeviceTypeId;
			out_template.CaptureDeviceVendorID	= pIBUsbManager->m_pMainCapture->m_UsbDeviceInfo.vid;
			out_template.ImageSamplingX			= 500;
			out_template.ImageSamplingY			= 500;
			out_template.ImageSizeX				= (unsigned short)image.Width;
			out_template.ImageSizeY				= (unsigned short)image.Height;
			out_template.Reserved				= 0;

//			for(DWORD mask = IBSU_FINGER_LEFT_LITTLE ; mask <= IBSU_FINGER_RIGHT_LITTLE ; mask <<= 1)
			{
				switch(fIndex)// & mask)
				{
				case IBSU_FINGER_LEFT_LITTLE:	finger_index = ENUM_IBSU_FINGER_LEFT_LITTLE; break;
				case IBSU_FINGER_LEFT_RING:		finger_index = ENUM_IBSU_FINGER_LEFT_RING; break;
				case IBSU_FINGER_LEFT_MIDDLE:	finger_index = ENUM_IBSU_FINGER_LEFT_MIDDLE; break;
				case IBSU_FINGER_LEFT_INDEX:	finger_index = ENUM_IBSU_FINGER_LEFT_INDEX; break;
				case IBSU_FINGER_LEFT_THUMB:	finger_index = ENUM_IBSU_FINGER_LEFT_THUMB; break;
				case IBSU_FINGER_RIGHT_THUMB:	finger_index = ENUM_IBSU_FINGER_RIGHT_THUMB; break;
				case IBSU_FINGER_RIGHT_INDEX:	finger_index = ENUM_IBSU_FINGER_RIGHT_INDEX; break;
				case IBSU_FINGER_RIGHT_MIDDLE:	finger_index = ENUM_IBSU_FINGER_RIGHT_MIDDLE; break;
				case IBSU_FINGER_RIGHT_RING:	finger_index = ENUM_IBSU_FINGER_RIGHT_RING; break;
				case IBSU_FINGER_RIGHT_LITTLE:	finger_index = ENUM_IBSU_FINGER_RIGHT_LITTLE; break;
				default:						finger_index = -1; break;
				}

				if(finger_index == -1)
				{
					nRc = IBSU_ERR_INVALID_PARAM_VALUE;
					goto done;
				}
				else 
				{
					if( (flagForce == FALSE) && (m_TemplateDB[finger_index].isUsed == TRUE) )
					{
						nRc = IBSU_ERR_DUPLICATE_ALREADY_USED; 
						goto done;
					}
					memcpy(&m_TemplateDB[finger_index]._template, &out_template, sizeof(IBSM_Template));
					m_TemplateDB[finger_index].isUsed = TRUE;
					m_TemplateDB[finger_index].imageType = imageType;
//					break;
				}
			}
		}
	}

done:

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Exit  IBSU_AddFingerImage");

	return nRc;
}

int CIBScanUltimateApp::DLL_IsFingerDuplicated
    (const int                 handle,
     const IBSU_ImageData      image,
     const DWORD               fIndex,
     const IBSU_ImageType      imageType,
     const int                 securityLevel,
     DWORD                     *pMatchedPosition)
{
	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;
	int				matching_score = 0;
	int				matching_threshold = 0;
	BOOL			flagValid = FALSE;

	IBSM_Template	out_template;

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Enter  IBSU_IsFingerDuplicated (handle=%d), fIndex(0x%04X), imageType(%d)", handle, fIndex, imageType);

	*pMatchedPosition = IBSU_FINGER_NONE;

	if( imageType != ENUM_IBSU_FLAT_SINGLE_FINGER &&
		imageType != ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		goto done;
	}

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = DLL_GetSecurityScore(securityLevel, &matching_threshold);

		if( nRc != IBSU_STATUS_OK)
		{
			nRc = IBSU_ERR_INVALID_PARAM_VALUE;
			goto done;
		}

		memset(out_template.Minutiae, 0, sizeof(unsigned int)*IBSM_MAX_MINUTIAE_SIZE);

		nRc = pIBUsbManager->m_pMainCapture->m_pMatcherAlg->IBSM_ExtractTemplate((const unsigned char*)image.Buffer,
																				 (unsigned char*)out_template.Minutiae,
																				 image.Width,
																				 image.Height);

		if(nRc == IBSU_STATUS_OK)
		{
			if(pIBUsbManager->m_pMainCapture->m_pMatcherAlg->_Algo_SetMatchingLevel(securityLevel) == -1)
			{
				InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Exit  IBSU_IsFingerDuplicated");
				return IBSU_ERR_INVALID_PARAM_VALUE;
			}

			for(DWORD mask = IBSU_FINGER_LEFT_LITTLE ; mask <= IBSU_FINGER_RIGHT_LITTLE ; mask <<= 1)
			{
				flagValid = TRUE;

				switch(fIndex & mask)
				{
				case IBSU_FINGER_LEFT_LITTLE:	
					nRc = pIBUsbManager->m_pMainCapture->m_pMatcherAlg->IBSM_MatchingTemplate((const unsigned char*)m_TemplateDB[ENUM_IBSU_FINGER_LEFT_LITTLE]._template.Minutiae,	(const unsigned char*)out_template.Minutiae, &matching_score); 
					break;
				case IBSU_FINGER_LEFT_RING:		
					nRc = pIBUsbManager->m_pMainCapture->m_pMatcherAlg->IBSM_MatchingTemplate((const unsigned char*)m_TemplateDB[ENUM_IBSU_FINGER_LEFT_RING]._template.Minutiae,	(const unsigned char*)out_template.Minutiae, &matching_score); 
					break;
				case IBSU_FINGER_LEFT_MIDDLE:	
					nRc = pIBUsbManager->m_pMainCapture->m_pMatcherAlg->IBSM_MatchingTemplate((const unsigned char*)m_TemplateDB[ENUM_IBSU_FINGER_LEFT_MIDDLE]._template.Minutiae,	(const unsigned char*)out_template.Minutiae, &matching_score); 
					break;
				case IBSU_FINGER_LEFT_INDEX:	
					nRc = pIBUsbManager->m_pMainCapture->m_pMatcherAlg->IBSM_MatchingTemplate((const unsigned char*)m_TemplateDB[ENUM_IBSU_FINGER_LEFT_INDEX]._template.Minutiae,	(const unsigned char*)out_template.Minutiae, &matching_score); 
					break;
				case IBSU_FINGER_LEFT_THUMB:	
					nRc = pIBUsbManager->m_pMainCapture->m_pMatcherAlg->IBSM_MatchingTemplate((const unsigned char*)m_TemplateDB[ENUM_IBSU_FINGER_LEFT_THUMB]._template.Minutiae,	(const unsigned char*)out_template.Minutiae, &matching_score); 
					break;
				case IBSU_FINGER_RIGHT_THUMB:	
					nRc = pIBUsbManager->m_pMainCapture->m_pMatcherAlg->IBSM_MatchingTemplate((const unsigned char*)m_TemplateDB[ENUM_IBSU_FINGER_RIGHT_THUMB]._template.Minutiae,	(const unsigned char*)out_template.Minutiae, &matching_score); 
					break;
				case IBSU_FINGER_RIGHT_INDEX:	
					nRc = pIBUsbManager->m_pMainCapture->m_pMatcherAlg->IBSM_MatchingTemplate((const unsigned char*)m_TemplateDB[ENUM_IBSU_FINGER_RIGHT_INDEX]._template.Minutiae,	(const unsigned char*)out_template.Minutiae, &matching_score); 
					break;
				case IBSU_FINGER_RIGHT_MIDDLE:	
					nRc = pIBUsbManager->m_pMainCapture->m_pMatcherAlg->IBSM_MatchingTemplate((const unsigned char*)m_TemplateDB[ENUM_IBSU_FINGER_RIGHT_MIDDLE]._template.Minutiae,	(const unsigned char*)out_template.Minutiae, &matching_score); 
					break;
				case IBSU_FINGER_RIGHT_RING:	
					nRc = pIBUsbManager->m_pMainCapture->m_pMatcherAlg->IBSM_MatchingTemplate((const unsigned char*)m_TemplateDB[ENUM_IBSU_FINGER_RIGHT_RING]._template.Minutiae,	(const unsigned char*)out_template.Minutiae, &matching_score); 
					break;
				case IBSU_FINGER_RIGHT_LITTLE:	
					nRc = pIBUsbManager->m_pMainCapture->m_pMatcherAlg->IBSM_MatchingTemplate((const unsigned char*)m_TemplateDB[ENUM_IBSU_FINGER_RIGHT_LITTLE]._template.Minutiae,	(const unsigned char*)out_template.Minutiae, &matching_score); 
					break;
				default: 
					flagValid = FALSE; 
					break;
				}

				if(nRc == IBSU_STATUS_OK)
				{
					if(flagValid && matching_score >= matching_threshold)
						*pMatchedPosition |= (fIndex & mask);
				}
				else
				{
					break;
				}
			}
		}
	}

done:

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Exit  IBSU_IsFingerDuplicated (%d/%d, 0x%04X)", matching_score, matching_threshold, *pMatchedPosition);

	return nRc;
}

int CIBScanUltimateApp::DLL_IsValidFingerGeometry
    (const int                 handle,
     const IBSU_ImageData      image,
     const DWORD               fIndex,
     const IBSU_ImageType      imageType,
     BOOL                     *pValid)
{
	CIBUsbManager			*pIBUsbManager = NULL;
	int						nRc = IBSU_STATUS_OK;
	int						segmentImageArrayCount;
	IBSU_SegmentPosition	tempSeg[4];
	int						MainDir;
	int						i, cx, cy, dx, dy, sum_dx, sum_dy, sum_dxdy;
	double					angle_cos, angle_sin;

	memset(tempSeg, 0, sizeof(IBSU_SegmentPosition)*IBSU_MAX_SEGMENT_COUNT);

	*pValid = FALSE;

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Enter  IBSU_IsValidFingerGeometry (handle=%d), fIndex(0x%04X), imageType(%d)", handle, fIndex, imageType);

	if( imageType != ENUM_IBSU_FLAT_TWO_FINGERS &&
        imageType != ENUM_IBSU_FLAT_FOUR_FINGERS )
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		goto done;
	}

	if( nRc == IBSU_STATUS_OK )
	{
		memcpy(pIBUsbManager->m_pMainCapture->m_pAlgo->m_Inter_Img, (unsigned char *)image.Buffer, image.Width*image.Height);
		pIBUsbManager->m_pMainCapture->m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(pIBUsbManager->m_pMainCapture->m_pAlgo->m_Inter_Img, pIBUsbManager->m_pMainCapture->m_pAlgo->m_Inter_Img2, image.Width*image.Height);
		
		pIBUsbManager->m_pMainCapture->m_pAlgo->m_CalledByValidGeo = TRUE;
		segmentImageArrayCount = pIBUsbManager->m_pMainCapture->m_pAlgo->_Algo_SegmentFinger_Matcher(pIBUsbManager->m_pMainCapture->m_pAlgo->m_Inter_Img2);
		pIBUsbManager->m_pMainCapture->m_pAlgo->m_CalledByValidGeo = FALSE;

		if(segmentImageArrayCount < 2)
		{
			nRc = IBSU_ERR_DUPLICATE_SEGMENTATION_FAILED;
			goto done;
		}

		IBSM_Point *Points = new IBSM_Point[segmentImageArrayCount];
		IBSM_Point *NewPoints1 = new IBSM_Point[segmentImageArrayCount];
		IBSM_Point *NewPoints2 = new IBSM_Point[segmentImageArrayCount];
		IBSM_Point *NewPoints3 = new IBSM_Point[segmentImageArrayCount];
		IBSM_Point *NewPoints4 = new IBSM_Point[segmentImageArrayCount];
		int *Points_Dir = new int [segmentImageArrayCount];
		
		for(i=0; i<segmentImageArrayCount; i++)
		{
			tempSeg[i].x1 = (pIBUsbManager->m_pMainCapture->m_pAlgo->m_segment_arr_matcher.Segment[i].P1_X-pIBUsbManager->m_pMainCapture->m_pAlgo->ENLARGESIZE_ZOOM_W) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W / pIBUsbManager->m_pMainCapture->m_pAlgo->ZOOM_W;
			tempSeg[i].y1 = (pIBUsbManager->m_pMainCapture->m_pAlgo->m_segment_arr_matcher.Segment[i].P1_Y-pIBUsbManager->m_pMainCapture->m_pAlgo->ENLARGESIZE_ZOOM_H) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H / pIBUsbManager->m_pMainCapture->m_pAlgo->ZOOM_H;

			tempSeg[i].x2 = (pIBUsbManager->m_pMainCapture->m_pAlgo->m_segment_arr_matcher.Segment[i].P2_X-pIBUsbManager->m_pMainCapture->m_pAlgo->ENLARGESIZE_ZOOM_W) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W / pIBUsbManager->m_pMainCapture->m_pAlgo->ZOOM_W;
			tempSeg[i].y2 = (pIBUsbManager->m_pMainCapture->m_pAlgo->m_segment_arr_matcher.Segment[i].P2_Y-pIBUsbManager->m_pMainCapture->m_pAlgo->ENLARGESIZE_ZOOM_H) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H / pIBUsbManager->m_pMainCapture->m_pAlgo->ZOOM_H;

			tempSeg[i].x3 = (pIBUsbManager->m_pMainCapture->m_pAlgo->m_segment_arr_matcher.Segment[i].P3_X-pIBUsbManager->m_pMainCapture->m_pAlgo->ENLARGESIZE_ZOOM_W) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W / pIBUsbManager->m_pMainCapture->m_pAlgo->ZOOM_W;
			tempSeg[i].y3 = (pIBUsbManager->m_pMainCapture->m_pAlgo->m_segment_arr_matcher.Segment[i].P3_Y-pIBUsbManager->m_pMainCapture->m_pAlgo->ENLARGESIZE_ZOOM_H) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H / pIBUsbManager->m_pMainCapture->m_pAlgo->ZOOM_H;

			tempSeg[i].x4 = (pIBUsbManager->m_pMainCapture->m_pAlgo->m_segment_arr_matcher.Segment[i].P4_X-pIBUsbManager->m_pMainCapture->m_pAlgo->ENLARGESIZE_ZOOM_W) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W / pIBUsbManager->m_pMainCapture->m_pAlgo->ZOOM_W;
			tempSeg[i].y4 = (pIBUsbManager->m_pMainCapture->m_pAlgo->m_segment_arr_matcher.Segment[i].P4_Y-pIBUsbManager->m_pMainCapture->m_pAlgo->ENLARGESIZE_ZOOM_H) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H / pIBUsbManager->m_pMainCapture->m_pAlgo->ZOOM_H;

			tempSeg[i].y1 = (pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H - tempSeg[i].y1);
			tempSeg[i].y2 = (pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H - tempSeg[i].y2);
			tempSeg[i].y3 = (pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H - tempSeg[i].y3);
			tempSeg[i].y4 = (pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H - tempSeg[i].y4);

			cx = cy = 0;

			cx += tempSeg[i].x1;
			cx += tempSeg[i].x2;
			cx += tempSeg[i].x3;
			cx += tempSeg[i].x4;
			cy += tempSeg[i].y1;
			cy += tempSeg[i].y2;
			cy += tempSeg[i].y3;
			cy += tempSeg[i].y4;
			
			Points[i].x = cx/4;
			Points[i].y = cy/4;

			dx = tempSeg[i].x1 - Points[i].x;
			dy = tempSeg[i].y1 - Points[i].y;
			sum_dx = dx*dx;
			sum_dy = dy*dy;
			sum_dxdy = dx*dy;

			dx = tempSeg[i].x2 - Points[i].x;
			dy = tempSeg[i].y2 - Points[i].y;
			sum_dx += dx*dx;
			sum_dy += dy*dy;
			sum_dxdy += dx*dy;

			dx = tempSeg[i].x3 - Points[i].x;
			dy = tempSeg[i].y3 - Points[i].y;
			sum_dx += dx*dx;
			sum_dy += dy*dy;
			sum_dxdy += dx*dy;

			dx = tempSeg[i].x4 - Points[i].x;
			dy = tempSeg[i].y4 - Points[i].y;
			sum_dx += dx*dx;
			sum_dy += dy*dy;
			sum_dxdy += dx*dy;

			Points_Dir[i] = pIBUsbManager->m_pMainCapture->m_pAlgo->_Algo_ATAN2_FULL(2*sum_dxdy, (sum_dx - sum_dy)) * 180 / 512;
		}

		cx = 0;
		cy = 0;
		angle_cos = 0.0;
		angle_sin = 0.0;
		for(i=0; i<segmentImageArrayCount; i++)
		{
			cx += Points[i].x;
			cy += Points[i].y;

			angle_cos += cos(Points_Dir[i]*3.14159265358979/90.0) * 100;
			angle_sin += sin(Points_Dir[i]*3.14159265358979/90.0) * 100;
		}

		MainDir = pIBUsbManager->m_pMainCapture->m_pAlgo->_Algo_ATAN2_FULL((int)angle_sin, (int)angle_cos) * 180 / 512;

		cx = cx / segmentImageArrayCount;
		cy = cy / segmentImageArrayCount;

		double cs_radian = cos((MainDir-90)*3.14159265358979/180.0);
		double sn_radian = sin((MainDir-90)*3.14159265358979/180.0);

		for(i=0; i<segmentImageArrayCount; i++)
		{
			NewPoints1[i].x = (int) ( (double)(tempSeg[i].x1-cx)*cs_radian + (double)(tempSeg[i].y1-cy)*sn_radian ) + cx;
			NewPoints1[i].y = (int) ( -(double)(tempSeg[i].x1-cx)*sn_radian + (double)(tempSeg[i].y1-cy)*cs_radian ) + cy;

			NewPoints2[i].x = (int) ( (double)(tempSeg[i].x2-cx)*cs_radian + (double)(tempSeg[i].y2-cy)*sn_radian ) + cx;
			NewPoints2[i].y = (int) ( -(double)(tempSeg[i].x2-cx)*sn_radian + (double)(tempSeg[i].y2-cy)*cs_radian ) + cy;

			NewPoints3[i].x = (int) ( (double)(tempSeg[i].x3-cx)*cs_radian + (double)(tempSeg[i].y3-cy)*sn_radian ) + cx;
			NewPoints3[i].y = (int) ( -(double)(tempSeg[i].x3-cx)*sn_radian + (double)(tempSeg[i].y3-cy)*cs_radian ) + cy;

			NewPoints4[i].x = (int) ( (double)(tempSeg[i].x4-cx)*cs_radian + (double)(tempSeg[i].y4-cy)*sn_radian ) + cx;
			NewPoints4[i].y = (int) ( -(double)(tempSeg[i].x4-cx)*sn_radian + (double)(tempSeg[i].y4-cy)*cs_radian ) + cy;
		}

		*pValid = TRUE;

		// ¿ÞÂÊÀÌ ³·Àº °æ¿ì
		if( fIndex == IBSU_FINGER_LEFT_LITTLE_RING || 
			fIndex == IBSU_FINGER_RIGHT_INDEX_MIDDLE )
		{
			if( (NewPoints1[0].y + NewPoints2[0].y) < (NewPoints1[1].y + NewPoints2[1].y) )
				*pValid = FALSE;
		}
		else if (fIndex == IBSU_FINGER_LEFT_HAND)
		{
			if( (NewPoints1[1].y + NewPoints2[1].y) < (NewPoints1[2].y + NewPoints2[2].y) )
				*pValid = FALSE;
		}
		// ¿À¸¥ÂÊÀÌ ³·Àº °æ¿ì
		else if( fIndex == IBSU_FINGER_LEFT_MIDDLE_INDEX || 
			fIndex == IBSU_FINGER_RIGHT_RING_LITTLE )
		{
			if( (NewPoints1[0].y + NewPoints2[0].y) > (NewPoints1[1].y + NewPoints2[1].y) )
				*pValid = FALSE;
		}
		else if (fIndex == IBSU_FINGER_RIGHT_HAND)
		{
			if( (NewPoints1[1].y + NewPoints2[1].y) > (NewPoints1[2].y + NewPoints2[2].y) )
				*pValid = FALSE;
		}

		delete [] Points;
		delete [] NewPoints1;
		delete [] NewPoints2;
		delete [] NewPoints3;
		delete [] NewPoints4;
		delete [] Points_Dir;

		nRc = IBSU_STATUS_OK;
	}

done:

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Exit  IBSU_IsValidFingerGeometry");

	return nRc;
}

int CIBScanUltimateApp::DLL_IsValidFingerGeometryEx
    (const int                 handle,
     IBSU_SegmentPosition	  *segmentPositionArray,
	 int                      segmentPositionArrayCount,
     const DWORD               fIndex,
	 BOOL                     *pValid)
{
	CIBUsbManager			*pIBUsbManager = NULL;
	int						nRc = IBSU_STATUS_OK;
	IBSU_SegmentPosition	tempSeg[4];
	int						MainDir;
	int						i, cx, cy, dx, dy, sum_dx, sum_dy, sum_dxdy;
	double					angle_cos, angle_sin;
	int						WrongSlapTolerance = 50;
	//int						MaximumHeight[4] = {0,0,0,0};

	memset(tempSeg, 0, sizeof(IBSU_SegmentPosition)*IBSU_MAX_SEGMENT_COUNT);

	*pValid = TRUE;

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Enter  DLL_IsValidFingerGeometryEx (handle=%d), fIndex(0x%04X)", handle, fIndex);

	if( nRc == IBSU_STATUS_OK )
	{
		if(segmentPositionArrayCount < 2)
		{
			nRc = IBSU_ERR_DUPLICATE_SEGMENTATION_FAILED;
			goto done;
		}

		IBSM_Point *Points = new IBSM_Point[segmentPositionArrayCount];
		IBSM_Point *NewPoints1 = new IBSM_Point[segmentPositionArrayCount];
		IBSM_Point *NewPoints2 = new IBSM_Point[segmentPositionArrayCount];
		IBSM_Point *NewPoints3 = new IBSM_Point[segmentPositionArrayCount];
		IBSM_Point *NewPoints4 = new IBSM_Point[segmentPositionArrayCount];
		int *Points_Dir = new int [segmentPositionArrayCount];
		
		for(i=0; i<segmentPositionArrayCount; i++)
		{
			tempSeg[i].x1 = segmentPositionArray[i].x1;
			tempSeg[i].y1 = segmentPositionArray[i].y1;

			tempSeg[i].x2 = segmentPositionArray[i].x2;
			tempSeg[i].y2 = segmentPositionArray[i].y2;

			tempSeg[i].x3 = segmentPositionArray[i].x3;
			tempSeg[i].y3 = segmentPositionArray[i].y3;

			tempSeg[i].x4 = segmentPositionArray[i].x4;
			tempSeg[i].y4 = segmentPositionArray[i].y4;

			/*tempSeg[i].y1 = (pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H - tempSeg[i].y1);
			tempSeg[i].y2 = (pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H - tempSeg[i].y2);
			tempSeg[i].y3 = (pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H - tempSeg[i].y3);
			tempSeg[i].y4 = (pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H - tempSeg[i].y4);*/

			cx = cy = 0;

			cx += tempSeg[i].x1;
			cx += tempSeg[i].x2;
			cx += tempSeg[i].x3;
			cx += tempSeg[i].x4;
			cy += tempSeg[i].y1;
			cy += tempSeg[i].y2;
			cy += tempSeg[i].y3;
			cy += tempSeg[i].y4;
			
			Points[i].x = cx/4;
			Points[i].y = cy/4;

			dx = tempSeg[i].x1 - Points[i].x;
			dy = tempSeg[i].y1 - Points[i].y;
			sum_dx = dx*dx;
			sum_dy = dy*dy;
			sum_dxdy = dx*dy;

			dx = tempSeg[i].x2 - Points[i].x;
			dy = tempSeg[i].y2 - Points[i].y;
			sum_dx += dx*dx;
			sum_dy += dy*dy;
			sum_dxdy += dx*dy;

			dx = tempSeg[i].x3 - Points[i].x;
			dy = tempSeg[i].y3 - Points[i].y;
			sum_dx += dx*dx;
			sum_dy += dy*dy;
			sum_dxdy += dx*dy;

			dx = tempSeg[i].x4 - Points[i].x;
			dy = tempSeg[i].y4 - Points[i].y;
			sum_dx += dx*dx;
			sum_dy += dy*dy;
			sum_dxdy += dx*dy;

			Points_Dir[i] = pIBUsbManager->m_pMainCapture->m_pAlgo->_Algo_ATAN2_FULL(2*sum_dxdy, (sum_dx - sum_dy)) * 180 / 512;
		}

		cx = 0;
		cy = 0;
		angle_cos = 0.0;
		angle_sin = 0.0;
		for(i=0; i<segmentPositionArrayCount; i++)
		{
			cx += Points[i].x;
			cy += Points[i].y;

			angle_cos += cos(Points_Dir[i]*3.14159265358979/90.0) * 100;
			angle_sin += sin(Points_Dir[i]*3.14159265358979/90.0) * 100;
		}

		MainDir = pIBUsbManager->m_pMainCapture->m_pAlgo->_Algo_ATAN2_FULL((int)angle_sin, (int)angle_cos) * 180 / 512;

		cx = cx / segmentPositionArrayCount;
		cy = cy / segmentPositionArrayCount;

		double cs_radian = cos((MainDir-90)*3.14159265358979/180.0);
		double sn_radian = sin((MainDir-90)*3.14159265358979/180.0);

		for(i=0; i<segmentPositionArrayCount; i++)
		{
			NewPoints1[i].x = (int) ( (double)(tempSeg[i].x1-cx)*cs_radian + (double)(tempSeg[i].y1-cy)*sn_radian ) + cx;
			NewPoints1[i].y = (int) ( -(double)(tempSeg[i].x1-cx)*sn_radian + (double)(tempSeg[i].y1-cy)*cs_radian ) + cy;
			//MaximumHeight[i] = NewPoints1[i].y;

			NewPoints2[i].x = (int) ( (double)(tempSeg[i].x2-cx)*cs_radian + (double)(tempSeg[i].y2-cy)*sn_radian ) + cx;
			NewPoints2[i].y = (int) ( -(double)(tempSeg[i].x2-cx)*sn_radian + (double)(tempSeg[i].y2-cy)*cs_radian ) + cy;
			//if(MaximumHeight[i] < NewPoints2[i].y)
			//	MaximumHeight[i] = NewPoints2[i].y;

			NewPoints3[i].x = (int) ( (double)(tempSeg[i].x3-cx)*cs_radian + (double)(tempSeg[i].y3-cy)*sn_radian ) + cx;
			NewPoints3[i].y = (int) ( -(double)(tempSeg[i].x3-cx)*sn_radian + (double)(tempSeg[i].y3-cy)*cs_radian ) + cy;
			//if(MaximumHeight[i] < NewPoints3[i].y)
			//	MaximumHeight[i] = NewPoints3[i].y;

			NewPoints4[i].x = (int) ( (double)(tempSeg[i].x4-cx)*cs_radian + (double)(tempSeg[i].y4-cy)*sn_radian ) + cx;
			NewPoints4[i].y = (int) ( -(double)(tempSeg[i].x4-cx)*sn_radian + (double)(tempSeg[i].y4-cy)*cs_radian ) + cy;
			//if(MaximumHeight[i] < NewPoints4[i].y)
			//	MaximumHeight[i] = NewPoints4[i].y;
		}

		/*// for saving debug image
		unsigned char *temp_org = new unsigned char [pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_SIZE/4];
		unsigned char *temp_rot = new unsigned char [pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_SIZE/4];
		memset(temp_org, 0, pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_SIZE/4);
		memset(temp_rot, 0, pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_SIZE/4);

		for(i=0; i<segmentPositionArrayCount; i++)
		{
			if(tempSeg[i].x1 >= 0 && tempSeg[i].x1 <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W-1 &&
				tempSeg[i].y1 >= 0 && tempSeg[i].y1 <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1)
			{
				temp_org[((pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1-tempSeg[i].y1)/2) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W/2 + tempSeg[i].x1/2] = 100+i*50;
			}
			if(tempSeg[i].x2 >= 0 && tempSeg[i].x2 <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W-1 &&
				tempSeg[i].y2 >= 0 && tempSeg[i].y2 <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1)
			{
				temp_org[((pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1-tempSeg[i].y2)/2) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W/2 + tempSeg[i].x2/2] = 100+i*50;
			}
			if(tempSeg[i].x3 >= 0 && tempSeg[i].x3 <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W-1 &&
				tempSeg[i].y3 >= 0 && tempSeg[i].y3 <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1)
			{
				temp_org[((pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1-tempSeg[i].y3)/2) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W/2 + tempSeg[i].x3/2] = 100+i*50;
			}
			if(tempSeg[i].x4 >= 0 && tempSeg[i].x4 <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W-1 &&
				tempSeg[i].y4 >= 0 && tempSeg[i].y4 <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1)
			{
				temp_org[((pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1-tempSeg[i].y4)/2) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W/2 + tempSeg[i].x4/2] = 100+i*50;
			}

			if(NewPoints1[i].x >= 0 && NewPoints1[i].x <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W-1 &&
				NewPoints1[i].y >= 0 && NewPoints1[i].y <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1)
			{
				temp_rot[((pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1-NewPoints1[i].y)/2) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W/2 + NewPoints1[i].x/2] = 100+i*50;
			}
			if(NewPoints2[i].x >= 0 && NewPoints2[i].x <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W-1 &&
				NewPoints2[i].y >= 0 && NewPoints2[i].y <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1)
			{
				temp_rot[((pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1-NewPoints2[i].y)/2) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W/2 + NewPoints2[i].x/2] = 100+i*50;
			}
			if(NewPoints3[i].x >= 0 && NewPoints3[i].x <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W-1 &&
				NewPoints3[i].y >= 0 && NewPoints3[i].y <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1)
			{
				temp_rot[((pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1-NewPoints3[i].y)/2) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W/2 + NewPoints3[i].x/2] = 100+i*50;
			}
			if(NewPoints4[i].x >= 0 && NewPoints4[i].x <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W-1 &&
				NewPoints4[i].y >= 0 && NewPoints4[i].y <= pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1)
			{
				temp_rot[((pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H-1-NewPoints4[i].y)/2) * pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W/2 + NewPoints4[i].x/2] = 100+i*50;
			}
		}

		pIBUsbManager->m_pMainCapture->m_pAlgo->m_SBDAlg->SaveBitmapImage("d:\\temp_org.bmp", temp_org,
			pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W/2, pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H/2);
		pIBUsbManager->m_pMainCapture->m_pAlgo->m_SBDAlg->SaveBitmapImage("d:\\temp_rot.bmp", temp_rot,
			pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_W/2, pIBUsbManager->m_pMainCapture->m_pAlgo->IMG_H/2);

		delete [] temp_org;
		delete [] temp_rot;*/

		// ¿ÞÂÊÀÌ ³·Àº °æ¿ì
		if( fIndex == IBSU_FINGER_LEFT_LITTLE_RING || 
			fIndex == IBSU_FINGER_RIGHT_INDEX_MIDDLE )
		{
			if( (NewPoints1[0].y + NewPoints2[0].y) < (NewPoints1[1].y + NewPoints2[1].y) )
				*pValid = FALSE;
		}
		else if (fIndex == IBSU_FINGER_LEFT_HAND)
		{
			// original
			//if( (NewPoints1[1].y + NewPoints2[1].y) < (NewPoints1[2].y + NewPoints2[2].y) - WrongSlapTolerance )
			//	*pValid = FALSE;
			
			// new
			int left_height = (NewPoints1[0].y + NewPoints2[0].y) + (NewPoints1[1].y + NewPoints2[1].y);
			int right_height = (NewPoints1[2].y + NewPoints2[2].y) + (NewPoints1[3].y + NewPoints2[3].y);
			if( left_height < right_height - WrongSlapTolerance )
				*pValid = FALSE;

#if defined(_DEBUG)
			TRACE("Diff : %d, ", left_height - right_height);
#endif
		}
		// ¿À¸¥ÂÊÀÌ ³·Àº °æ¿ìIBSU_IsValidFingerGeometryEx
		else if( fIndex == IBSU_FINGER_LEFT_MIDDLE_INDEX || 
			fIndex == IBSU_FINGER_RIGHT_RING_LITTLE )
		{
			if( (NewPoints1[0].y + NewPoints2[0].y) > (NewPoints1[1].y + NewPoints2[1].y) )
				*pValid = FALSE;
		}
		else if (fIndex == IBSU_FINGER_RIGHT_HAND)
		{
			// original
			//if( (NewPoints1[1].y + NewPoints2[1].y) - WrongSlapTolerance > (NewPoints1[2].y + NewPoints2[2].y) )
			//	*pValid = FALSE;

			// new
			int left_height = (NewPoints1[0].y + NewPoints2[0].y) + (NewPoints1[1].y + NewPoints2[1].y);
			int right_height = (NewPoints1[2].y + NewPoints2[2].y) + (NewPoints1[3].y + NewPoints2[3].y);
			if( left_height - WrongSlapTolerance > right_height )
				*pValid = FALSE;

#if defined(_DEBUG)
			TRACE("Diff : %d, ", left_height - right_height);
#endif
		}

		delete [] Points;
		delete [] NewPoints1;
		delete [] NewPoints2;
		delete [] NewPoints3;
		delete [] NewPoints4;
		delete [] Points_Dir;

		nRc = IBSU_STATUS_OK;
	}

done:

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(), "Exit  DLL_IsValidFingerGeometryEx");

	return nRc;
}

int WINAPI IBSU_IsValidFingerGeometryEx
    (const int                 handle,
     IBSU_SegmentPosition	  *segmentPositionArray,
	 int                      segmentPositionArrayCount,
     const DWORD               fIndex,
	 BOOL                     *pValid)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_IsValidFingerGeometryEx(handle, segmentPositionArray, segmentPositionArrayCount, fIndex, pValid);
}

int	CIBScanUltimateApp::RES_CBP_GetPreviewInfo
    (const int              handle,
     const char            *pReservedkey,
     IBSU_SegmentPosition  *segmentPositionArray,
	 IBSU_SegmentPosition  *segmentPositionArray_for_geo,
	 int                   *segmentPositionArrayCount,
	 BOOL				   *IsFingerDetected)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter RES_CBP_GetPreviewInfo (handle=%d)", handle);

    if (_IsValidReservedKey(pReservedkey) != IBSU_STATUS_OK &&
		_IsValidPartnerReservedKey(pReservedkey) != IBSU_STATUS_OK)
	{
		nRc = IBSU_ERR_NOT_SUPPORTED;
		InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
			"Exit RES_CBP_GetPreviewInfo (handle=%d)", handle);

		return nRc;
	}

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->Reserved_CBP_GetPreviewInfo(segmentPositionArray, segmentPositionArray_for_geo, segmentPositionArrayCount, IsFingerDetected);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit RES_CBP_GetPreviewInfo (handle=%d)", handle);

	return nRc;
}

int	CIBScanUltimateApp::RES_CBP_CleanUp
    (const int              handle)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter RES_CBP_CleanUp (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->Reserved_CBP_CleanUp();
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit RES_CBP_CleanUp (handle=%d)", handle);

	return nRc;
}

int	CIBScanUltimateApp::RES_CBP_IsFingerOn
    (const int              handle,
	BOOL					*bIsFingerOn)
{
//	CThreadSync Sync;

	CIBUsbManager	*pIBUsbManager = NULL;
	int				nRc = IBSU_STATUS_OK;

	InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
		"Enter RES_CBP_IsFingerOn (handle=%d)", handle);

	pIBUsbManager = FindUsbDeviceInList(handle);
	nRc = _IsValidUsbHandle(pIBUsbManager);

	*bIsFingerOn = TRUE;

	if( nRc == IBSU_STATUS_OK )
	{
		nRc = pIBUsbManager->m_pMainCapture->Reserved_CBP_IsFingerOn(bIsFingerOn);
	}

	InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit RES_CBP_IsFingerOn (handle=%d)", handle);

	return nRc;
}

int WINAPI RESERVED_CBP_GetPreviewInfo
    (const int              handle,
	const char            *pReservedkey,
	IBSU_SegmentPosition  *segmentPositionArray,
	IBSU_SegmentPosition  *segmentPositionArray_for_geo,
	int                   *segmentPositionArrayCount,
	BOOL				  *IsFingerDetected)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_CBP_GetPreviewInfo(handle, pReservedkey, segmentPositionArray, segmentPositionArray_for_geo, segmentPositionArrayCount, IsFingerDetected);
}

int WINAPI RESERVED_CBP_CleanUp
    (const int              handle)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_CBP_CleanUp(handle);
}

int WINAPI RESERVED_CBP_IsFingerOn
    (const int              handle,
	BOOL					*IsNoFinger)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.RES_CBP_IsFingerOn(handle, IsNoFinger);
}

int	CIBScanUltimateApp::DLL_GetSecurityScore
	(const int securityLevel,
	int *pSecurityScore)
{

	 switch(securityLevel)
    {
        case 1: *pSecurityScore = 1600; break;
        case 2: *pSecurityScore = 2000; break;
        case 3: *pSecurityScore = 2400; break;
        case 4: *pSecurityScore = 2800; break;
        case 5: *pSecurityScore = 3200; break;
        case 6: *pSecurityScore = 3600; break;
        case 7: *pSecurityScore = 4000; break;
        default: return IBSU_ERR_INVALID_PARAM_VALUE;
    }

	return IBSU_STATUS_OK;
}

int WINAPI IBSU_RemoveFingerImage(
      const int                 handle,
      const DWORD               fingerPosition)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_RemoveFingerImage(handle, fingerPosition);
}

int WINAPI IBSU_AddFingerImage
    (const int                 handle,
     const IBSU_ImageData      image,
     const DWORD               fingerPosition,
     const IBSU_ImageType      imageType,
     const BOOL                flagForce)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_AddFingerImage(handle, image, fingerPosition, imageType, flagForce);
}

int WINAPI IBSU_IsFingerDuplicated
    (const int                 handle,
     const IBSU_ImageData      image,
     const DWORD               fingerPosition,
     const IBSU_ImageType      imageType,
     const int                 securityLevel,
     DWORD                     *pMatchedPosition)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_IsFingerDuplicated(handle, image, fingerPosition, imageType, securityLevel, pMatchedPosition);
}

int WINAPI IBSU_IsValidFingerGeometry
    (const int                 handle,
     const IBSU_ImageData      image,
     const DWORD               fingerPosition,
     const IBSU_ImageType      imageType,
     BOOL                     *pValid)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_IsValidFingerGeometry(handle, image, fingerPosition, imageType, pValid);
}

int WINAPI IBSU_GetRequiredSDKVersion(
    const int			deviceIndex,
    char			   *minSDKVersion
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	//theApp.DLL_GetDeviceDescription(deviceIndex, pDeviceDescA);
	return theApp.DLL_GetRequiredSDKVersion(deviceIndex, minSDKVersion);
}

int WINAPI IBSU_SetEncryptionKey
    (const int                 handle,
     const unsigned char*	   pEncyptionKey,
	 const IBSU_EncryptionMode encMode)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return theApp.DLL_SetEncryptionKey(handle, pEncyptionKey, encMode);
}

int CIBScanUltimateApp::DLL_GetRequiredSDKVersion(
    const int          deviceIndex,
    char			   *minSDKVersion
)
{
//	CThreadSync Sync;

    int				nRc = IBSU_STATUS_OK;
    CIBUsbManager	*pIBUsbManager = NULL;
    CMainCapture	*pMainCapture = NULL;
	char			version[MAX_PATH];;
	IBSU_DeviceDescA devDesc;

    InsertTraceLog(sTraceMainCategory, 0, _GetThreadID(),
                          "Enter IBSU_GetRequiredSDKVersion (deviceIndex=%d)", deviceIndex);

    if (::IsBadReadPtr(minSDKVersion, sizeof(MAX_PATH)))
    {
        nRc = IBSU_ERR_INVALID_ACCESS_POINTER;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetRequiredSDKVersion (deviceIndex=%d)", deviceIndex);
        return nRc;
    }

	memset(minSDKVersion,	0, sizeof(MAX_PATH));
    memset(&version,		0, sizeof(MAX_PATH));
	memset(&devDesc,		0, sizeof(IBSU_DeviceDesc));

    FindAllDeviceCount();
    if (GetDeviceCount() <= deviceIndex)
    {
        nRc = IBSU_ERR_DEVICE_NOT_FOUND;
        InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
                              "Exit  IBSU_GetRequiredSDKVersion (deviceIndex=%d)", deviceIndex);
        return nRc;
    }

    if ((pIBUsbManager = FindUsbDeviceByIndexInList(deviceIndex)) == NULL ||
            pIBUsbManager->m_pMainCapture == NULL)
    {
        if (pIBUsbManager == NULL)
        {
            pIBUsbManager = new CIBUsbManager();
        }
        pMainCapture = new CMainCapture(&theApp, pIBUsbManager);

    BOOL bPPIMode = FALSE;
#if defined(__ppi__)
    if (deviceIndex < m_nPPIDeviceCount)
        bPPIMode = TRUE;
#endif
        nRc = pMainCapture->Main_GetDeviceInfo(deviceIndex, &devDesc, bPPIMode);
        Sleep(10);
		strcpy(minSDKVersion, pMainCapture->m_propertyInfo.sKojakCalibrationInfo.cRequiredSDK);

        delete(CMainCapture *)pMainCapture;
        if (pIBUsbManager)
        {
            delete(CIBUsbManager *)pIBUsbManager;
        }
    }
    else
    {
        strcpy(minSDKVersion, pIBUsbManager->m_pMainCapture->m_propertyInfo.sKojakCalibrationInfo.cRequiredSDK);
    }

#ifdef __android__
    // 2013-03-11 Brian modify -
    //	GetBusInfoByIndex(deviceIndex, &devDesc.busNumber, &devDesc.devAddress, &devDesc.devPath[0]);
    {
        int  busNumber, devAddress;
        char devPath[IBSU_MAX_STR_LEN];

        GetBusInfoByIndex(deviceIndex, &busNumber, &devAddress, &devPath[0]);
		devDesc.devID = busNumber * 1000 + devAddress;
    }
#endif


    InsertTraceLog(sTraceMainCategory, nRc, _GetThreadID(),
		"Exit  IBSU_GetRequiredSDKVersion (deviceIndex=%d, Required SDK Version=%s)", deviceIndex, minSDKVersion);

    return nRc;
}
