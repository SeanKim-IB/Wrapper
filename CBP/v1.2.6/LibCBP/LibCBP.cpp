// LibCBP.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "LibCBP.h"
#include "IBScanUltimateApi.h"
#include <math.h>

#ifdef _WINDOWS
#include "FileVersionInfo.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

#if defined(_WINDOWS)
// Shared memory for lock info
typedef struct TMemoryMappedDLLData 
{ 
	CBPLockInfo m_LockInfoArr[MAX_LOCK_INFO_COUNT];
	int			m_LockInfoArrCount;
	int			m_AccessedProcessCount;
} TSharedData; 

static const char   *g_MMFileName = "CBPAPI_MEMORY_MAPPED"; 
static HANDLE       g_MapHandle; 
static TSharedData  *g_pSharedData = NULL; 

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
		g_pSharedData->m_LockInfoArrCount = 0;
		for(int i=0; i<MAX_LOCK_INFO_COUNT; i++)
		{
			g_pSharedData->m_LockInfoArr[i].m_LockInfo.handle = -1;
			g_pSharedData->m_LockInfoArr[i].m_LockInfo.lockPID = 0;
			g_pSharedData->m_LockInfoArr[i].m_LockInfo.lockDuration = 0;
		}
		g_pSharedData->m_AccessedProcessCount = 0;
    }

	// increase count for the accessed process
	g_pSharedData->m_AccessedProcessCount++;
}

void CloseSharedData() 
{
	if (g_pSharedData)
	{
		g_pSharedData->m_AccessedProcessCount--;

		// release memory map if access process is empty
		if(g_pSharedData->m_AccessedProcessCount <= 0)
		{
			UnmapViewOfFile(g_pSharedData);
		}
	    
		if (g_MapHandle)
		{
			CloseHandle(g_MapHandle);
		}
	}
} 
#endif

BEGIN_MESSAGE_MAP(CLibCBPApp, CWinApp)
END_MESSAGE_MAP()

CLibCBPApp::CLibCBPApp()
{
	memset(&m_Callbacks, 0, sizeof(cbp_fp_callBacks));
	m_CBPUsbDevices.clear();

	// allocation
	pParam_calibrate = new ThreadParam_cbp_fp_calibrate;
	pParam_calibrate->pWnd = NULL;

	pParam_capture = new ThreadParam_cbp_fp_capture;
	pParam_capture->pWnd = NULL;

	pParam_close = new ThreadParam_cbp_fp_close;
	pParam_close->pWnd = NULL;
	
	pParam_configure = new ThreadParam_cbp_fp_configure;
	pParam_configure->pWnd = NULL;
	
	pParam_enumDevices = new ThreadParam_cbp_fp_enumDevices;
	pParam_enumDevices->pWnd = NULL;

	pParam_getDirtiness = new ThreadParam_cbp_fp_getDirtiness;
	pParam_getDirtiness->pWnd = NULL;

	pParam_getLockInfo = new ThreadParam_cbp_fp_getLockInfo;
	pParam_getLockInfo->pWnd = NULL;
	//pParam_getLockInfo->ThreadCount = 0;

	pParam_initialize = new ThreadParam_cbp_fp_initialize;
	pParam_initialize->pWnd = NULL;

	pParam_lock = new ThreadParam_cbp_fp_lock;
	pParam_lock->pWnd = NULL;

	pParam_open = new ThreadParam_cbp_fp_open;
	pParam_open->device = new cbp_fp_device;
	pParam_open->pWnd = NULL;

	pParam_powerSave = new ThreadParam_cbp_fp_powerSave;
	pParam_powerSave->pWnd = NULL;

	pParam_startImaging = new ThreadParam_cbp_fp_startImaging;
	pParam_startImaging->pWnd = NULL;

	pParam_stopImaging = new ThreadParam_cbp_fp_stopImaging;
	pParam_stopImaging->pWnd = NULL;

	pParam_uninitialize = new ThreadParam_cbp_fp_uninitialize;
	pParam_uninitialize->pWnd = NULL;

	pParam_unlock = new ThreadParam_cbp_fp_unlock;
	pParam_unlock->pWnd = NULL;

	// 20180905 change ini path
	m_DebugLevel = 0;
	m_DebugFileSizeLimit = MAX_LOG_SIZE;
	m_DebugFileCount = DEF_LOG_COUNT_INDEX;
	sprintf(m_DebugFileName, "");

	// Refer ini located in same directory as the wrapper.
	CString inifile;
	TCHAR pf[MAX_PATH];
	sprintf(pf, "%s", _Get_DLL_PATH().GetBuffer());

	if(pf[strlen(pf)-1] == '\\')
		inifile.Format("%s%s", pf, INI_FILE_NAME);
	else
		inifile.Format("%s\\%s", pf, INI_FILE_NAME);

	m_INIMgr.SetFilename(inifile);

	if(m_INIMgr.ISExistINI() == TRUE)
	{
		_Read_INI_Info();
	}

	for(int i=0; i<m_DebugFileCount+1; i++)
	{
		m_VFoundLogFiles[i] = "";
	}
	m_VFoundLogFilesCount = 0;

	// Log Manage fuction call
	_LogManage();
}

CString CLibCBPApp::_Get_DLL_PATH()
{
	HMODULE hModule = (HMODULE)&__ImageBase;
	char szPath[1024];
	ZeroMemory( szPath, 1024 );
	GetModuleFileName(hModule, szPath, 1024);
	CString strPath = szPath;
	return strPath.Left(strPath.ReverseFind('\\')+1);
}

CString CLibCBPApp::_Get_EXE_PATH()
{
	CString strPath;
	GetModuleFileName(NULL, strPath.GetBuffer(MAX_PATH), MAX_PATH);
	strPath.ReleaseBuffer();
	return strPath.Left(strPath.ReverseFind('\\')+1);
}

CLibCBPApp::~CLibCBPApp()
{
	// to be safe : first try to memory de-allocation of IBScanUltimate.
	IBSU_UnloadLibrary();

	std::vector<CBPUsbDevice *>::iterator it = m_CBPUsbDevices.begin();
	while( it != m_CBPUsbDevices.end() )
	{
		if( *it )
		{
			delete *it;
			it = m_CBPUsbDevices.erase(it);
		}
		else
            ++it;
	}
	m_CBPUsbDevices.clear();

	//IBSU_CloseAllDevice();

	// de-allocation
	if(pParam_calibrate)	delete pParam_calibrate;
	if(pParam_capture)	delete pParam_capture;
	if(pParam_close)	delete pParam_close;
	if(pParam_configure)	delete pParam_configure;
	if(pParam_enumDevices)	delete pParam_enumDevices;
	if(pParam_getDirtiness)	delete pParam_getDirtiness;
	if(pParam_getLockInfo)	delete pParam_getLockInfo;
	if(pParam_initialize)	delete pParam_initialize;
	if(pParam_lock)	delete pParam_lock;
	if(pParam_open->device)	delete pParam_open->device;
	if(pParam_open)	delete pParam_open;
	if(pParam_powerSave)	delete pParam_powerSave;
	if(pParam_startImaging)	delete pParam_startImaging;
	if(pParam_stopImaging)	delete pParam_stopImaging;
	if(pParam_uninitialize)	delete pParam_uninitialize;
	if(pParam_unlock)	delete pParam_unlock;
}

CLibCBPApp theApp;

BOOL CLibCBPApp::InitInstance()
{
	CWinApp::InitInstance();

#if defined(_WINDOWS)
    CreateSharedData();
#endif

	return TRUE;
}

BOOL CLibCBPApp::ExitInstance()
{
	CWinApp::ExitInstance();

#if defined(_WINDOWS)
    CloseSharedData();
#endif

    return TRUE;
}

CBPUsbDevice* CLibCBPApp::_GetUsbDevicesFromHandle(int handle)
{
	CBPUsbDevice *tmpCBPUsbDev = NULL;

	for(unsigned int i=0; i<m_CBPUsbDevices.size(); i++)
	{
		tmpCBPUsbDev = m_CBPUsbDevices.at(i);
		if(tmpCBPUsbDev->m_nDevHandle == handle)
		{
			return tmpCBPUsbDev;
		}
	}

	return NULL;
}

CBPUsbDevice* CLibCBPApp::_GetUsbDevicesFromRequestID(char *requestID)
{
	CBPUsbDevice *tmpCBPUsbDev = NULL;

	for(unsigned int i=0; i<m_CBPUsbDevices.size(); i++)
	{
		tmpCBPUsbDev = m_CBPUsbDevices.at(i);
		if(strcmp(tmpCBPUsbDev->m_sRequestID, requestID) == 0)
		{
			return tmpCBPUsbDev;
		}
	}

	return NULL;
}

CBPUsbDevice* CLibCBPApp::_GetUsbDevicesFromNotOpened()
{
	CBPUsbDevice *tmpCBPUsbDev = NULL;

	for(int i=m_CBPUsbDevices.size()-1; i>=0; i--)
	{
		tmpCBPUsbDev = m_CBPUsbDevices.at(i);

		// initialized and device not opened
		if( tmpCBPUsbDev->m_bInitialized == TRUE && 
			tmpCBPUsbDev->m_nDevHandle == -1 )
		{
			return tmpCBPUsbDev;
		}
	}

	return NULL;
}

BOOL CLibCBPApp::_DeleteUsbDeviceFromRequestID(char *requestID)
{
	CBPUsbDevice *tmpCBPUsbDev = NULL;
	int number_index=-1;
	int DeleteCount=0;

	for(unsigned int i=0; i<m_CBPUsbDevices.size(); i++)
	{
		tmpCBPUsbDev = m_CBPUsbDevices.at(i);
		if(strcmp(tmpCBPUsbDev->m_sRequestID, requestID) == 0)
		{
			break;
		}
	}

	std::vector<CBPUsbDevice *>::iterator it = m_CBPUsbDevices.begin();
	while( it != m_CBPUsbDevices.end() )
	{
		if( *it == tmpCBPUsbDev )
		{
			delete *it;
			it = m_CBPUsbDevices.erase(it);
			DeleteCount++;
		}
		else
			++it;
	}

	if(DeleteCount > 0)
		return TRUE;

	return FALSE;
}

BOOL CLibCBPApp::_DeleteUsbDeviceFromHandle(int handle)
{
	CBPUsbDevice *tmpCBPUsbDev = NULL;
	int number_index=-1;
	int DeleteCount=0;

	for(unsigned int i=0; i<m_CBPUsbDevices.size(); i++)
	{
		tmpCBPUsbDev = m_CBPUsbDevices.at(i);
		if(tmpCBPUsbDev->m_nDevHandle == handle)
		{
			break;
		}
	}

	std::vector<CBPUsbDevice *>::iterator it = m_CBPUsbDevices.begin();
	while( it != m_CBPUsbDevices.end() )
	{
		if( *it == tmpCBPUsbDev )
		{
			delete *it;
			it = m_CBPUsbDevices.erase(it);
			DeleteCount++;
		}
		else
			++it;
	}
	
	if(DeleteCount > 0)
		return TRUE;

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
void CLibCBPApp::OnEvent_DeviceCommunicationBreak(const int deviceHandle, void* pContext)
{
	if( pContext == NULL )
		return;

	CLibCBPApp *pApp = reinterpret_cast<CLibCBPApp*>(pContext);
	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(deviceHandle)) == NULL)
		return;

	pApp->callback_error(CBP_FP_ERROR_SCANNER_DISCONNECTED, "Check the USB Cable");

	cbp_fp_close(deviceHandle);
}

void CLibCBPApp::OnEvent_PreviewImage(const int deviceHandle, void* pContext, const IBSU_ImageData image)
{
	if( pContext == NULL )
		return;

	CLibCBPApp *pApp = reinterpret_cast<CLibCBPApp*>(pContext);
	CBPUsbDevice *pCBPUsbDev = NULL;
	BOOL IsValidGeo = TRUE;
	BOOL IsActive;
	int nRc;
	char log_str[MAX_PATH];
	struct cbp_fp_fingerAttributes fingerAttributes;
	char codeString[100] = { 0 };

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(deviceHandle)) == NULL)
		return;

	// prevent invoke after on_stoppreview
	if(pCBPUsbDev->m_StopPreview == TRUE)
		return;

	if( pCBPUsbDev->m_bReceivedResultImage == FALSE ) // during startimaging
	{
		// get CBPPreviewInfo
		IBSU_SegmentPosition  segmentPositionArray[IBSU_MAX_SEGMENT_COUNT];
		IBSU_SegmentPosition  segmentPositionArray_for_geo[IBSU_MAX_SEGMENT_COUNT];
		int segmentPositionArrayCount = 0;
		BOOL IsFingerDetected = FALSE;

		memset(segmentPositionArray, 0, sizeof(IBSU_SegmentPosition) * IBSU_MAX_SEGMENT_COUNT);
		if(RESERVED_CBP_GetPreviewInfo(deviceHandle, "ibkorea1120!", segmentPositionArray, segmentPositionArray_for_geo, &segmentPositionArrayCount, &IsFingerDetected) != IBSU_STATUS_OK)
		{
			pApp->callback_error(CBP_FP_ERROR_PREVIEW, "cbp_fp_onPreview");
			return;
		}

		cbp_fp_grayScalePreview preview;
		preview.image = new BYTE [image.Width*image.Height];
		//memcpy(preview.image, image.Buffer, image.Width*image.Height);
		// flip
		BYTE *pBuf = (BYTE *)image.Buffer;
		for(int i=0; i<(int)image.Height; i++)
		{
			memcpy(&preview.image[i*image.Width], &pBuf[(image.Height-1-i)*image.Width], image.Width);
		}
	
		/*
		// Draw Segment on image
		for(int i=0; i<segmentPositionArrayCount; i++)
		{
			int x1, x2, x3, x4, y1, y2, y3, y4;
			x1 = segmentPositionArray_for_geo[i].x1;
			x2 = segmentPositionArray_for_geo[i].x2;
			x3 = segmentPositionArray_for_geo[i].x3;
			x4 = segmentPositionArray_for_geo[i].x4;

			y1 = segmentPositionArray_for_geo[i].y1;
			y2 = segmentPositionArray_for_geo[i].y2;
			y3 = segmentPositionArray_for_geo[i].y3;
			y4 = segmentPositionArray_for_geo[i].y4;

			pApp->_DrawLine(preview.image, x1, y1, x2, y2, image.Width, image.Height, 0);
			pApp->_DrawLine(preview.image, x2, y2, x3, y3, image.Width, image.Height, 0);
			pApp->_DrawLine(preview.image, x3, y3, x4, y4, image.Width, image.Height, 0);
			pApp->_DrawLine(preview.image, x4, y4, x1, y1, image.Width, image.Height, 0);
		}*/

		preview.width = image.Width;
		preview.height = image.Height;

		preview.slapType = pCBPUsbDev->m_SlapType;
		preview.collectionType = pCBPUsbDev->m_CollectionType;

		cbp_fp_previewAnalysis preview_analysis;
		
		preview_analysis.slapType = pCBPUsbDev->m_SlapType;
		preview_analysis.collectionType = pCBPUsbDev->m_CollectionType;
		preview_analysis.fingerAttributeListLen = segmentPositionArrayCount;
		memset(preview_analysis.fingerAttributeList, 0, sizeof(cbp_fp_fingerAttributes)*CBP_FP_MAX_FINGER);

		DWORD fIndex = 0;
		if(pCBPUsbDev->m_SlapType == slap_rightHand)
			fIndex = IBSU_FINGER_RIGHT_HAND;
		else if(pCBPUsbDev->m_SlapType == slap_leftHand)
			fIndex = IBSU_FINGER_LEFT_HAND;
		else 
			fIndex = 0;

		if(segmentPositionArrayCount == 4)
		{
			if(IBSU_IsValidFingerGeometryEx(deviceHandle, segmentPositionArray_for_geo, segmentPositionArrayCount, fIndex, &IsValidGeo) != IBSU_STATUS_OK)
				IsValidGeo = TRUE;
		}
		
		if(IsValidGeo == TRUE)
		{
			if(segmentPositionArrayCount < 4)
				pApp->m_IsValidGeoCnt = 0;
			else
			{
				pApp->m_IsValidGeoCnt--;
				if(pApp->m_IsValidGeoCnt < 0)
					pApp->m_IsValidGeoCnt = 0;
			}
		}

#if defined(_DEBUG)
		TRACE("Valid Count : %d\n", pApp->m_IsValidGeoCnt);
#endif

		int best_score = 0;
		int is_score_100 = FALSE;
		for(int i=0; i<segmentPositionArrayCount; i++)
		{
			preview_analysis.fingerAttributeList[i].analysisCodeListLen = 0;
			memset(preview_analysis.fingerAttributeList[i].analysisCodeList, 0, sizeof(int)*CBP_FP_MAX_FP_ANAYSIS_CODES);

			int found_count = 0, found_idx = -1;
			for(int j=0; j<IBSU_MAX_SEGMENT_COUNT; j++)
			{
				if(pCBPUsbDev->m_FingerQualityState[j] != ENUM_IBSU_FINGER_NOT_PRESENT)
					found_count++;
				
				if(found_count == i+1)
				{
					found_idx = j;
					break;
				}
			}

			if(found_idx != -1)
			{
				if(pCBPUsbDev->m_FingerQualityState[found_idx] == ENUM_IBSU_QUALITY_POOR)
				{
					//preview_analysis.fingerAttributeList[i].score = 25;
					preview_analysis.fingerAttributeList[i].score = 50;
					preview_analysis.fingerAttributeList[i].analysisCodeList[preview_analysis.fingerAttributeList[i].analysisCodeListLen] = CBP_FP_ANALYSIS_CODE_TOO_LIGHT;
				}
				else if(pCBPUsbDev->m_FingerQualityState[found_idx] == ENUM_IBSU_QUALITY_FAIR)
				{
					//preview_analysis.fingerAttributeList[i].score = 50;
					preview_analysis.fingerAttributeList[i].score = 75;
					preview_analysis.fingerAttributeList[i].analysisCodeList[preview_analysis.fingerAttributeList[i].analysisCodeListLen] = CBP_FP_ANALYSIS_CODE_TOO_LIGHT;
				}
				else if(pCBPUsbDev->m_FingerQualityState[found_idx] == ENUM_IBSU_QUALITY_GOOD)
				{
					is_score_100 = TRUE;
					//preview_analysis.fingerAttributeList[i].score = 75;
					if(pApp->m_Score100Count < 2)
					{
						preview_analysis.fingerAttributeList[i].score = 75;
						preview_analysis.fingerAttributeList[i].analysisCodeList[preview_analysis.fingerAttributeList[i].analysisCodeListLen] = CBP_FP_ANALYSIS_CODE_TOO_LIGHT;
					}
					else
					{
						pApp->m_Score100Count = 2;
						preview_analysis.fingerAttributeList[i].score = 100;
						preview_analysis.fingerAttributeList[i].analysisCodeList[preview_analysis.fingerAttributeList[i].analysisCodeListLen] = CBP_FP_ANALYSIS_CODE_GOOD_PRINT;
					}
				}
				else if(pCBPUsbDev->m_FingerQualityState[found_idx] == ENUM_IBSU_QUALITY_INVALID_AREA_TOP)
				{
					preview_analysis.fingerAttributeList[i].score = 0;
					preview_analysis.fingerAttributeList[i].analysisCodeList[preview_analysis.fingerAttributeList[i].analysisCodeListLen] = CBP_FP_ANALYSIS_CODE_TOO_HIGH;
				}
				else if(pCBPUsbDev->m_FingerQualityState[found_idx] == ENUM_IBSU_QUALITY_INVALID_AREA_LEFT)
				{
					preview_analysis.fingerAttributeList[i].score = 0;
					preview_analysis.fingerAttributeList[i].analysisCodeList[preview_analysis.fingerAttributeList[i].analysisCodeListLen] = CBP_FP_ANALYSIS_CODE_TOO_LEFT;
				}
				else if(pCBPUsbDev->m_FingerQualityState[found_idx] == ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT)
				{
					preview_analysis.fingerAttributeList[i].score = 0;
					preview_analysis.fingerAttributeList[i].analysisCodeList[preview_analysis.fingerAttributeList[i].analysisCodeListLen] = CBP_FP_ANALYSIS_CODE_TOO_RIGHT;
				}
				else if(pCBPUsbDev->m_FingerQualityState[found_idx] == ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM)
				{
					preview_analysis.fingerAttributeList[i].score = 0;
					preview_analysis.fingerAttributeList[i].analysisCodeList[preview_analysis.fingerAttributeList[i].analysisCodeListLen] = CBP_FP_ANALYSIS_CODE_TOO_LOW;
				}
				else
				{
					preview_analysis.fingerAttributeList[i].score = 0;
					preview_analysis.fingerAttributeList[i].analysisCodeList[preview_analysis.fingerAttributeList[i].analysisCodeListLen] = CBP_FP_ANALYSIS_CODE_NO_PRINT;
				}

				if(best_score < preview_analysis.fingerAttributeList[i].score)
					best_score = preview_analysis.fingerAttributeList[i].score;
				
				if(pCBPUsbDev->m_ClearPlaten == TRUE)
				{
					/*if(preview_analysis.fingerAttributeList[i].score > 25)
					{
						preview_analysis.fingerAttributeList[i].score = 25;
						preview_analysis.fingerAttributeList[i].analysisCodeList[preview_analysis.fingerAttributeList[i].analysisCodeListLen] = CBP_FP_ANALYSIS_CODE_TOO_LIGHT;
					}*/
					if(preview_analysis.fingerAttributeList[i].score > 50)
					{
						preview_analysis.fingerAttributeList[i].score = 50;
						preview_analysis.fingerAttributeList[i].analysisCodeList[preview_analysis.fingerAttributeList[i].analysisCodeListLen] = CBP_FP_ANALYSIS_CODE_TOO_LIGHT;
					}
				}

				preview_analysis.fingerAttributeList[i].analysisCodeListLen++;

				//if(IsValidGeo == FALSE && (best_score > 25 || best_score == 0))
				if(IsValidGeo == FALSE)// && (best_score > 50 || best_score == 0))
				{
					if(i == 3)
					{
						pApp->m_IsValidGeoCnt++;

						if(pApp->m_IsValidGeoCnt > 2)
						{
							pApp->m_IsValidGeoCnt = 3;
							preview_analysis.fingerAttributeList[i].analysisCodeList[preview_analysis.fingerAttributeList[i].analysisCodeListLen] = 
							preview_analysis.fingerAttributeList[i].analysisCodeList[preview_analysis.fingerAttributeList[i].analysisCodeListLen-1];
							preview_analysis.fingerAttributeList[i].analysisCodeList[preview_analysis.fingerAttributeList[i].analysisCodeListLen-1] = CBP_FP_ANALYSIS_CODE_WRONG_SLAP;
							preview_analysis.fingerAttributeList[i].analysisCodeListLen++;
						}
					}
				}
			}

			// calculate centroid position
			preview_analysis.fingerAttributeList[i].centroid.x = (segmentPositionArray[i].x1 + segmentPositionArray[i].x2 + 
																  segmentPositionArray[i].x3 + segmentPositionArray[i].x4) / 4;
			preview_analysis.fingerAttributeList[i].centroid.y = (segmentPositionArray[i].y1 + segmentPositionArray[i].y2 + 
																  segmentPositionArray[i].y3 + segmentPositionArray[i].y4) / 4;

			// calculate bounding box
			int minx, maxx, miny, maxy;
			minx = maxx = preview_analysis.fingerAttributeList[i].centroid.x;
			if(minx > segmentPositionArray[i].x1) minx = segmentPositionArray[i].x1;
			if(minx > segmentPositionArray[i].x2) minx = segmentPositionArray[i].x2;
			if(minx > segmentPositionArray[i].x3) minx = segmentPositionArray[i].x3;
			if(minx > segmentPositionArray[i].x4) minx = segmentPositionArray[i].x4;
			if(maxx < segmentPositionArray[i].x1) maxx = segmentPositionArray[i].x1;
			if(maxx < segmentPositionArray[i].x2) maxx = segmentPositionArray[i].x2;
			if(maxx < segmentPositionArray[i].x3) maxx = segmentPositionArray[i].x3;
			if(maxx < segmentPositionArray[i].x4) maxx = segmentPositionArray[i].x4;

			miny = maxy = preview_analysis.fingerAttributeList[i].centroid.y;
			if(miny > segmentPositionArray[i].y1) miny = segmentPositionArray[i].y1;
			if(miny > segmentPositionArray[i].y2) miny = segmentPositionArray[i].y2;
			if(miny > segmentPositionArray[i].y3) miny = segmentPositionArray[i].y3;
			if(miny > segmentPositionArray[i].y4) miny = segmentPositionArray[i].y4;
			if(maxy < segmentPositionArray[i].y1) maxy = segmentPositionArray[i].y1;
			if(maxy < segmentPositionArray[i].y2) maxy = segmentPositionArray[i].y2;
			if(maxy < segmentPositionArray[i].y3) maxy = segmentPositionArray[i].y3;
			if(maxy < segmentPositionArray[i].y4) maxy = segmentPositionArray[i].y4;

			if(minx < 1) minx = 1;
			else if(minx > (int)(image.Width-1)) minx = (int)(image.Width-1);
			if(maxx < 1) maxx = 1;
			else if(maxx > (int)(image.Width-1)) maxx = (int)(image.Width-1);

			if(miny < 1) miny = 1;
			else if(miny > (int)(image.Height-1)) miny = (int)(image.Height-1);
			if(maxy  < 1) maxy  = 1;
			else if(maxy  > (int)(image.Height-1)) maxy = (int)(image.Height-1);

			preview_analysis.fingerAttributeList[i].boundingBox.x = minx;
			preview_analysis.fingerAttributeList[i].boundingBox.y = miny;
			preview_analysis.fingerAttributeList[i].boundingBox.width = (maxx-minx);
			preview_analysis.fingerAttributeList[i].boundingBox.height = (maxy-miny);

			preview_analysis.fingerAttributeList[i].centroid.x = minx + (maxx-minx)/2;
			preview_analysis.fingerAttributeList[i].centroid.y = miny + (maxy-miny)/2;
		}

		if(is_score_100 == TRUE)
		{
			pApp->m_Score100Count++;
		}
		else
		{
			pApp->m_Score100Count = 0;
		}

		//TRACE("m_Score100Count : %d, m_IsValidGeoCnt : %d\n", pApp->m_Score100Count, pApp->m_IsValidGeoCnt);

		if(segmentPositionArrayCount > 0)
			pApp->_ControlLEDforKojak(deviceHandle, pCBPUsbDev->m_FingerQualityState, segmentPositionArrayCount, __LED_COLOR_YELLOW__,
										pCBPUsbDev->m_SlapType, pCBPUsbDev->m_CollectionType);
		else
			pApp->_ControlLEDforKojak(deviceHandle, pCBPUsbDev->m_FingerQualityState, segmentPositionArrayCount, __LED_COLOR_RED__,
										pCBPUsbDev->m_SlapType, pCBPUsbDev->m_CollectionType);
	
		nRc = IBSU_IsCaptureActive(deviceHandle, &IsActive);
		if( nRc == IBSU_STATUS_OK && !IsActive )
		{
			delete [] preview.image;
			return;
		}

		if(pApp->m_Callbacks.cbp_fp_onPreview != NULL)
		{
			pApp->_LogMessage("cbp_fp_onPreview", "Enter", "", 2);
			pApp->m_Callbacks.cbp_fp_onPreview(deviceHandle, &preview);
			pApp->_LogMessage("cbp_fp_onPreview", "Exit", "", 2);
		}

		if(pApp->m_Callbacks.cbp_fp_onPreviewAnalysis != NULL)
		{
			TCHAR str_slapType[MAX_PATH];
			TCHAR str_collectionType[MAX_PATH];

			pApp->_GetCollectionTypeCodeString(preview_analysis.collectionType, str_collectionType);
			pApp->_GetSlapTypeCodeString(preview_analysis.slapType, str_slapType);

			pApp->_LogMessage("cbp_fp_onPreviewAnalysis", "Enter", "", 2);

			/*if(pCBPUsbDev->m_ClearPlaten == TRUE)
			{
				pApp->_LogMessage(log_str, ">>> Clear Platen", "", 1);
			}*/

			sprintf(log_str, "%d ( %s )", preview_analysis.collectionType, str_collectionType);
			pApp->_LogMessage(log_str, ">>> CollectionType", "", 2);
			sprintf(log_str, "%d ( %s )", preview_analysis.slapType, str_slapType);
			pApp->_LogMessage(log_str, ">>> SlapType", "", 2);
			
			for (int i = 0; i < preview_analysis.fingerAttributeListLen; i++)
			{
				fingerAttributes = preview_analysis.fingerAttributeList[i];
				
				for(int j=0; j<fingerAttributes.analysisCodeListLen; j++)
				{
					pApp->_GetAnalysisCodeString(fingerAttributes.analysisCodeList[j], codeString);
					sprintf(log_str, "FINGER [%d] %s, score %d", i, codeString, fingerAttributes.score);
					pApp->_LogMessage(log_str, ">>> fingerAttribute", "", 2);
				}
			}

			/*
			//if(WrongSlapFlag)
			{
				pApp->_LogMessage(">>> SegInfo", "", "", 2);
				for(int i = 0; i < segmentPositionArrayCount; i++)
				{
					sprintf(log_str, "#%d ] (%d,%d) (%d,%d) (%d,%d) (%d,%d)", i+1, segmentPositionArray_for_geo[i].x1, segmentPositionArray_for_geo[i].y1,
														segmentPositionArray_for_geo[i].x2, segmentPositionArray_for_geo[i].y2,
														segmentPositionArray_for_geo[i].x3, segmentPositionArray_for_geo[i].y3,
														segmentPositionArray_for_geo[i].x4, segmentPositionArray_for_geo[i].y4);
					pApp->_LogMessage(log_str, "", "", 2);
				}
			}*/

			pApp->m_Callbacks.cbp_fp_onPreviewAnalysis(deviceHandle, &preview_analysis);
			pApp->_LogMessage("cbp_fp_onPreviewAnalysis", "Exit", "", 2);
		}

		delete [] preview.image;
	}
}

void CLibCBPApp::OnEvent_FingerCount(const int deviceHandle, void* pContext, const IBSU_FingerCountState fingerCountState)
{
	if( pContext == NULL )
		return;
}

void CLibCBPApp::OnEvent_FingerQuality(const int deviceHandle, void *pContext, const IBSU_FingerQualityState *pQualityArray, const int qualityArrayCount)
{
	if( pContext == NULL )
		return;

	CLibCBPApp *pApp = reinterpret_cast<CLibCBPApp*>(pContext);
	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(deviceHandle)) == NULL)
		return;

	memcpy(pCBPUsbDev->m_FingerQualityState, pQualityArray, sizeof(IBSU_FingerQualityState)*IBSU_MAX_SEGMENT_COUNT);
}

void CLibCBPApp::OnEvent_DeviceCount(const int detectedDevices, void *pContext)
{
	if( pContext == NULL )
		return;
}

void CLibCBPApp::OnEvent_InitProgress(const int deviceIndex, void *pContext, const int progressValue)
{
	if( pContext == NULL )
		return;
}

void CLibCBPApp::OnEvent_TakingAcquisition(const int deviceHandle, void *pContext, const IBSU_ImageType imageType)
{
	if( pContext == NULL )
		return;
}

void CLibCBPApp::OnEvent_CompleteAcquisition(const int deviceHandle, void *pContext, const IBSU_ImageType imageType)
{
	if( pContext == NULL )
		return;
}

void CLibCBPApp::OnEvent_ResultImageEx(const int deviceHandle, void *pContext, const int imageStatus, const IBSU_ImageData image, 
												const IBSU_ImageType imageType, const int detectedFingerCount, const int segmentImageArrayCount, 
												const IBSU_ImageData *pSegmentImageArray, const IBSU_SegmentPosition *pSegmentPositionArray)
{
	if( pContext == NULL )
		return;

	CLibCBPApp *pApp = reinterpret_cast<CLibCBPApp*>(pContext);
	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(deviceHandle)) == NULL)
		return;

//	if(pApp->m_collectionType == collection_rolled)
	{
		pCBPUsbDev->m_bReceivedResultImage = TRUE;
	}
}

void CLibCBPApp::OnEvent_ClearPlatenAtCapture(const int deviceHandle, void *pContext, const IBSU_PlatenState platenState)
{
	if( pContext == NULL )
		return;

	CLibCBPApp *pApp = reinterpret_cast<CLibCBPApp*>(pContext);
	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(deviceHandle)) == NULL)
		return;

	if(platenState == ENUM_IBSU_PLATEN_HAS_FINGERS)
		pCBPUsbDev->m_ClearPlaten = TRUE;
	else
		pCBPUsbDev->m_ClearPlaten = FALSE;
}

void CLibCBPApp::OnEvent_AsyncOpenDevice(const int deviceIndex, void *pContext, const int deviceHandle, const int errorCode)
{
	if( pContext == NULL )
		return;
}

void CLibCBPApp::OnEvent_NotifyMessage(const int deviceHandle, void *pContext, const int message)
{
	if( pContext == NULL )
		return;
}

void CLibCBPApp::OnEvent_PressedKeyButtons(const int deviceHandle, void *pContext, const int pressedKeyButtons)
{
	if( pContext == NULL )
		return;
}

void CLibCBPApp::_LogMessage(char *msg, char *flag, char *result, int requiredDebugLevel)
{
	if(m_DebugLevel == 0)
		return;

	if(m_DebugLevel >= requiredDebugLevel)
	{
		// open log file
		FILE *m_logFile = fopen(m_DebugFileName, "at");
		if(m_logFile == NULL)
			return;

		fseek(m_logFile, 0, SEEK_END);
		int size = ftell(m_logFile);

		SYSTEMTIME systime;
		GetLocalTime(&systime);

		if(size > m_DebugFileSizeLimit)
		{
			fclose(m_logFile);

			// Rename latest log for rollover
			CString NewFilename_tmp = m_DebugFileName;
			CString NewFilename;
			BOOL CopySuccess = false;
			BOOL DeleteSuccess = false;
	
			NewFilename_tmp.Delete(NewFilename_tmp.GetLength()-4, 4);
			NewFilename.Format("%s_%4ld-%02ld-%02ld-%02ld-%02ld-%02ld.log", NewFilename_tmp,
				systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

			CopySuccess = rename(m_DebugFileName, NewFilename);
			if (!CopySuccess) 
			{
				// Put New file in Log File Vector 
				m_VFoundLogFiles[m_VFoundLogFilesCount] = NewFilename;
				m_VFoundLogFilesCount++;
			}

			if (m_VFoundLogFilesCount > m_DebugFileCount)
			{
				DeleteSuccess = DeleteFile(m_VFoundLogFiles[0]);
				if (DeleteSuccess)
				{
					for(int i=1; i<m_VFoundLogFilesCount; i++)
					{
						m_VFoundLogFiles[i-1] = m_VFoundLogFiles[i];
					}
					m_VFoundLogFiles[m_VFoundLogFilesCount-1] = "";
					m_VFoundLogFilesCount--;
				}
			}

			_LogManage();

			/*CopySuccess = CopyFile(m_DebugFileName, NewFilename, FALSE);
			if (CopySuccess) 
			{
				// Put New file in Log File Vector 
				m_VFoundLogFiles[m_VFoundLogFilesCount] = NewFilename;
				m_VFoundLogFilesCount++;
			}*/

			

			m_logFile = fopen(m_DebugFileName, "wt");
			if(m_logFile == NULL)
				return;
		}

		fprintf(m_logFile, "[%4ld-%02ld-%02ld %02ld:%02ld:%02ld.%03ld]\t%s\t%s\t%s\n", systime.wYear, systime.wMonth, systime.wDay, 
			systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds, flag, msg, result);
		fclose(m_logFile);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
/// brief Performs calibration on the scanner.  A lock is required.
/// param[in] handle handle to the scanner
/// see struct cbp_fp_callBacks.cbp_fp_onCalibrate
CBP_FP_DEVICE_API void STDCALL cbp_fp_calibrate(int handle)
{
	sprintf(theApp.m_Message, "handle (%d)", handle);
	theApp._LogMessage("cbp_fp_calibrate", "Enter", theApp.m_Message, 2);

    theApp.DLL_calibrate(handle);

	theApp._LogMessage("cbp_fp_calibrate", "Exit", "", 2);
}

///	\brief Captures a ten print slap image.  This image is typically of higher resolution than the image.
/// \param[in] handle handle to the scanner
/// \param[in] slapType indicates the type of slap to be processed.  The scanner may display guidance information based on the slap requested.  In addition the analysis information will be affected by the indicated slap type.
/// \param[in] collectionType indicates whether a flat or rolled slap collection type method will be used
/// \see CBP_FP_PROPERTY_CAPTURE_PIXEL_HEIGHT
/// \see CBP_FP_PROPERTY_CAPTURE_PIXEL_WIDTH
/// \see struct cbp_fp_callBacks.cbp_fp_onCapture
CBP_FP_DEVICE_API void STDCALL cbp_fp_capture(int handle, cbp_fp_slapType slapType, cbp_fp_collectionType collectionType)
{
	TCHAR str_slapType[MAX_PATH];
	TCHAR str_collectionType[MAX_PATH];

	theApp._GetSlapTypeCodeString(slapType, str_slapType);
	theApp._GetCollectionTypeCodeString(collectionType, str_collectionType);

	sprintf(theApp.m_Message, "handle (%d), slapType (%d,%s), collectionType (%d,%s)", 
								handle, slapType, str_slapType, collectionType, str_collectionType);
	theApp._LogMessage("cbp_fp_capture", "Enter", theApp.m_Message, 2);

	theApp.DLL_capture(handle, slapType, collectionType);

	theApp._LogMessage("cbp_fp_capture", "Exit", "", 2);
}

/// \brief Closes a connection to a ten print scanner and releases any memory associated with that connection.
/// \param[in] handle handle to the scanner to be closed.
/// \see struct cbp_fp_callBacks.cbp_fp_onClose
CBP_FP_DEVICE_API void STDCALL cbp_fp_close(int handle)
{
	sprintf(theApp.m_Message, "handle (%d)", handle);
	theApp._LogMessage("cbp_fp_close", "Enter", theApp.m_Message, 2);

	theApp.DLL_close(handle);

	theApp._LogMessage("cbp_fp_close", "Exit", "", 2);
}

/// \brief Pops up a modal dialog that manages proprietary settings of the scanner.  The settings are persistantly cached.
/// It is expected that a restart of the application will be required freeing up all existing connections to the scanner.
/// This API may be optionally implimented.
/// \param[in] handle handle to the scanner
/// \see struct cbp_fp_callBacks.cbp_fp_onConfigure
CBP_FP_DEVICE_API void STDCALL cbp_fp_configure(int handle) // lock required
{
	sprintf(theApp.m_Message, "handle (%d)", handle);
	theApp._LogMessage("cbp_fp_configure", "Enter", theApp.m_Message, 2);

    theApp.DLL_configure(handle);

	theApp._LogMessage("cbp_fp_configure", "Exit", "", 2);
}

/// \brief Enumerates the devices recognized for the implimented wrapping layer. The devices may reflect which devices have been installed
/// or perhaps only a subset of those devices which are connected. Results in a call back to \c cbp_fp_onEnumDevices
/// \param[in] requestID a string which is used to pass back to the caller, the identifier of the request
/// \see struct cbp_fp_callBacks.cbp_fp_OnEnumDevices
CBP_FP_DEVICE_API void STDCALL cbp_fp_enumDevices(char *requestID)
{
	sprintf(theApp.m_Message, "requestID (%s)", requestID);
	theApp._LogMessage("cbp_fp_enumDevices", "Enter", theApp.m_Message, 2);

	theApp.DLL_enumDevices(requestID);

	theApp._LogMessage("cbp_fp_enumDevices", "Exit", "", 2);
}

/// \brief Gets the dirtiness level of the scanner.
/// \param[in] handle handle to the scanner
/// \see struct cbp_fp_callBacks.cbp_fp_onGetDirtiness
CBP_FP_DEVICE_API void STDCALL cbp_fp_getDirtiness(int handle) // lock required
{
	sprintf(theApp.m_Message, "handle (%d)", handle);
	theApp._LogMessage("cbp_fp_getDirtiness", "Enter", theApp.m_Message, 2);

	theApp.DLL_getDirtiness(handle);

	theApp._LogMessage("cbp_fp_getDirtiness", "Exit", "", 2);
}

/// \brief gets the lock information of the scanner.
/// \param[in] handle handle to the scanner
/// \see struct cbp_fp_callBacks.cbp_fp_onGetLockInfo
/// \see cbp_fp_lock_info
CBP_FP_DEVICE_API void STDCALL cbp_fp_getLockInfo(int handle)
{
	sprintf(theApp.m_Message, "handle (%d)", handle);
	theApp._LogMessage("cbp_fp_getLockInfo", "Enter", theApp.m_Message, 2);

	theApp.DLL_getLockInfo(handle);
	
	theApp._LogMessage("cbp_fp_getLockInfo", "Exit", "", 2);
}

/// \brief looks up a property in a connection session and returns the value.
/// \param[in] handle handle to the scanner
/// \param[in] name name of the property to look up
/// \return the value of the property is returned. If the property is not found in the connection session, a \c NULL is returned.  Memory returned is allocated from the session and should not be modified.
CBP_FP_DEVICE_API char * STDCALL cbp_fp_getProperty(int handle, char *name)  // no callback
{
	sprintf(theApp.m_Message, "handle (%d), property name (%s)", handle, name);
	theApp._LogMessage("cbp_fp_getProperty", "Enter", theApp.m_Message, 2);
    
	char *result = theApp.DLL_getProperty(handle, name);

	sprintf(theApp.m_Message, "property value (%s)", result);
	theApp._LogMessage("cbp_fp_getProperty", "Exit", theApp.m_Message, 2);

	return result;
}

/// \brief Initialized global memory structures and established connections that apply to all finger print scanner(s) associated with the implimented wrapper.
/// The expectation is that this call will only happen once during application initialization.  For example, if your implimentation requires a connecting to a service,
/// the initialization of that service would occur within the implimentation of this call.  Call results in a chain to \c cbp_fp_onInitialize
/// \param[in] requestID a string which is used to pass back to the caller, the identifier of the request
/// \return \c CBP_FP_OK if everything OK, else another error code.
/// \see struct cbp_fp_callBacks.cbp_fp_onInitialize
CBP_FP_DEVICE_API void STDCALL cbp_fp_initialize(char *requestID)
{
	sprintf(theApp.m_Message, "requestID (%s)", requestID);
	theApp._LogMessage("cbp_fp_initialize", "Enter", theApp.m_Message, 2);

	theApp.DLL_initialize(requestID);

	theApp._LogMessage("cbp_fp_initialize", "Exit", "", 2);
}

/// \brief Locks a scanner giving exclusive access.  The scanner must be unlocked for anyone else to access it.  Typically applications will lock a scanner on focus, and unlock the scanner on unfocus.
/// \param[in] handle handle to the scanner
/// \see struct cbp_fp_callBacks.cbp_fp_onLock
/// \see cbp_fp_getLockInfo
CBP_FP_DEVICE_API void STDCALL cbp_fp_lock(int handle)
{
	sprintf(theApp.m_Message, "handle (%d)", handle);
	theApp._LogMessage("cbp_fp_lock", "Enter", theApp.m_Message, 2);

    theApp.DLL_lock(handle);

	theApp._LogMessage("cbp_fp_lock", "Exit", "", 2);
}

/// \brief Establishes communications with a ten print scanner.  Some implimentations will not support multiple concurrent connections using their SDK.  In this case
/// \c cbp_fp_onError should be chained with an error of \c CBP_FP_ERROR_NOT_IMPLEMENTED.
/// A ten print scanner can be open in multiple ways with this API:
/// \ul
/// \li \c NULL - open will discover any ten print device implimented by the wrapping layer.  The discovered ten print device opened, shall be the first discovered.
/// \li \c cbp_fp_device->model - if this is defined, then a specific model will be discovered.
/// \li \c cbp_fp_device->serial - model must be defined, then if this is defined, a specific model and serial number will be discovered.
/// \ul
/// \param[in] device the scanner to be opened
/// \see struct cbp_fp_device
/// \see struct cbp_fp_callBacks.cbp_fp_onOpen
CBP_FP_DEVICE_API void STDCALL cbp_fp_open(struct cbp_fp_device *device)
{
	if(device == NULL)
		sprintf(theApp.m_Message, "device is NULL");
	else
		sprintf(theApp.m_Message, "make (%s), model (%s), serialNumber (%s)", device->make, device->model, device->serialNumber);
	theApp._LogMessage("cbp_fp_open", "Enter", theApp.m_Message, 2);

    theApp.DLL_open(device);

	theApp._LogMessage("cbp_fp_open", "Exit", "", 2);
}

/// \brief Sets the powersave mode of the scanner.  During powersave mode, the scanner will power down energy consumming components such as the LED.
/// It is the responsiblity of the client to restore the powersave mode to \c false before requesting any ten print processing from the scanner.
/// \param[in] handle handle to the scanner
/// \param[in] powerSaveOn \c CBP_FP_TRUE if power save mode on, \c CBP_FP_FALSE if power save mode off
/// \see struct cbp_fp_callBacks.cbp_fp_onPowerSave
CBP_FP_DEVICE_API void STDCALL cbp_fp_powerSave(int handle, bool powerSaveOn) // lock required
{
	sprintf(theApp.m_Message, "handle (%d), powerSaveOn (%d)", handle, powerSaveOn);
	theApp._LogMessage("cbp_fp_powerSave", "Enter", theApp.m_Message, 2);

    theApp.DLL_powerSave(handle, powerSaveOn);

	theApp._LogMessage("cbp_fp_powerSave", "Exit", "", 2);
}

/// \brief Registers the call backs implimented by the client.  The implimented wrapping layer will call these call backs to indicate events and pass data back to the
/// client.  Each call back in the structure should be initialized to point to a call back function.
/// Implimentations should validate that all call backs are implimented.  This function has no call back to it.
/// \param[in] callBacks structure containing the function pointers to the call backs.  All call backs must be implimented or stubbed. \see struct cbp_fp_callBacks
/// \return \c CBP_FP_OK - if all is well.  \c CBP_FP_ERROR_NOT_IMPLEMENTED - if one or more of the call backs is not implimented.
CBP_FP_DEVICE_API int STDCALL cbp_fp_registerCallBacks(struct cbp_fp_callBacks *callBacks) // no callback
{
	int nRc = CBP_FP_OK;
	sprintf(theApp.m_Message, "");
	theApp._LogMessage("cbp_fp_registerCallBacks", "Enter", theApp.m_Message, 2);

    nRc = theApp.DLL_registerCallBacks(callBacks);

	sprintf(theApp.m_Message, "Ret (%d)", nRc);
	theApp._LogMessage("cbp_fp_registerCallBacks", "Exit", theApp.m_Message, 2);

	return nRc;
}

/// \brief sets a property value.  Each connection contains a session which cache's the property values.  Some properties may have their settings changed.  While others are read only.
/// If an attempt is made to alter a property value on a read only property, \c cbp_tp_onError is called with a \c CBP_FP_ERROR_PROPERTY_NOT_SETABLE
/// \param[in] handle handle to the device
/// \param[in] property the property name and value to be set in the session property cache
CBP_FP_DEVICE_API void STDCALL cbp_fp_setProperty(int handle, struct cbp_fp_property *property) // no callback
{
	sprintf(theApp.m_Message, "property name (%s)", property->name);
	theApp._LogMessage("cbp_fp_setProperty", "Enter", theApp.m_Message, 2);
    
	theApp.DLL_setProperty(handle, property);

	sprintf(theApp.m_Message, "property value (%s)", property->name);
	theApp._LogMessage("cbp_fp_setProperty", "Exit", theApp.m_Message, 2);
}

/// \brief Engages the scanner into imaging mode. Imaging and analysis will be returned to the client until a \c cbp_fp_stopImaging is requested.
/// \param[in] handle handle to the scanner
/// \param[in] slapType indicates the type of slap to be processed.  The scanner may display guidance information based on the slap requested.  In addition the analysis information will be affected by the indicated slap type.
/// \param[in] collectionType indicates whether a flat or rolled slap collection type method will be used
/// \see cbp_fp_slapType
/// \see cbp_fp_collectionType
/// \see struct cbp_fp_callBacks.cbp_fp_onPreview
/// \see struct cbp_fp_callBacks.cbp_fp_onPreviewAnalysis
/// \see cbp_fp_capture
/// \see cbp_fp_stopImaging
CBP_FP_DEVICE_API void STDCALL cbp_fp_startImaging(int handle, cbp_fp_slapType slapType, cbp_fp_collectionType collectionType)
{
	TCHAR str_slapType[MAX_PATH];
	TCHAR str_collectionType[MAX_PATH];

	switch(slapType)
	{
	case slap_rightHand: sprintf(str_slapType, "slap_rightHand"); break;
	case slap_leftHand: sprintf(str_slapType, "slap_leftHand"); break;
	case slap_twoFingers: sprintf(str_slapType, "slap_twoFingers"); break;
	case slap_twoThumbs: sprintf(str_slapType, "slap_twoThumbs"); break;
	case slap_rightThumb: sprintf(str_slapType, "slap_rightThumb"); break;
	case slap_rightIndex: sprintf(str_slapType, "slap_rightIndex"); break;
	case slap_rightMiddle: sprintf(str_slapType, "slap_rightMiddle"); break;
	case slap_rightRing: sprintf(str_slapType, "slap_rightRing"); break;
	case slap_rightLittle: sprintf(str_slapType, "slap_rightLittle"); break;
	case slap_leftThumb: sprintf(str_slapType, "slap_leftThumb"); break;
	case slap_leftIndex: sprintf(str_slapType, "slap_leftIndex"); break;
	case slap_leftMiddle: sprintf(str_slapType, "slap_leftMiddle"); break;
	case slap_leftRing: sprintf(str_slapType, "slap_leftRing"); break;
	case slap_leftLittle: sprintf(str_slapType, "slap_leftLittle"); break;
	case slap_twotThumbs: sprintf(str_slapType, "slap_twotThumbs"); break;
	case slap_stitchedLeftThumb: sprintf(str_slapType, "slap_stitchedLeftThumb"); break;
	case slap_stitchedRightThumb: sprintf(str_slapType, "slap_stitchedRightThumb"); break;
	case slap_unknown: 
	default: sprintf(str_slapType, "slap_unknown"); break;
	}

	switch(collectionType)
	{
	case collection_rolled: sprintf(str_collectionType, "collection_rolled"); break;
	case collection_flat: sprintf(str_collectionType, "collection_flat"); break;
	case collection_contactless: sprintf(str_collectionType, "collection_contactless"); break;
	case collection_unknown:
	default: sprintf(str_slapType, "collection_unknown"); break;
	}

	sprintf(theApp.m_Message, "handle (%d), slapType (%d,%s), collectionType (%d,%s)", 
								handle, slapType, str_slapType, collectionType, str_collectionType);
	theApp._LogMessage("cbp_fp_startImaging", "Enter", theApp.m_Message, 2);
    
    theApp.DLL_startImaging(handle, slapType, collectionType);

	theApp._LogMessage("cbp_fp_startImaging", "Exit", "", 2);
}

/// \brief Disengages the scanner from imaging mode.  The client typically calls this after quality analysis yields sufficient quality for a capture or a time out has occurred.
/// \param[in] handle handle to the scanner
/// \see cbp_fp_stopImaging
CBP_FP_DEVICE_API void STDCALL cbp_fp_stopImaging(int handle)
{
	sprintf(theApp.m_Message, "handle (%d)", handle);
	theApp._LogMessage("cbp_fp_stopImaging", "Enter", theApp.m_Message, 2);
    
    theApp.DLL_stopImaging(handle);

	theApp._LogMessage("cbp_fp_stopImaging", "Exit", "", 2);
}

/// \brief Uninitialized global memory structures and established connections that apply to all ten print scanner(s) associated with the implimented wrapper.
/// The expectation is that this call will only happen once during application uninitialization.  For example, if your implimentation requires a connecting to a service,
/// the uninitialization of that service would occur within the implimentation of this call.  Call results in a chain to \c struct \c cbp_fp_callBacks.cbp_fp_onUninitialize
/// \param[in] requestID a string which is used to pass back to the caller, the identifier of the request
/// \return \c CBP_FP_OK if everything OK, else another error code.
/// \see struct cbp_fp_callBacks.cbp_fp_onUninitialize
CBP_FP_DEVICE_API void STDCALL cbp_fp_uninitialize(char *requestID)
{
	sprintf(theApp.m_Message, "requestID (%s)", requestID);
	theApp._LogMessage("cbp_fp_uninitialize", "Enter", theApp.m_Message, 2);
    
    theApp.DLL_uninitialize(requestID);

	theApp._LogMessage("cbp_fp_uninitialize", "Exit", "", 2);
}

/// \brief Unlocks a scanner allowing for exclusive access of the scanner by another client or service.
/// \param[in] handle handle to the scanner
/// \see struct cbp_fp_callBacks.cbp_fp_onUnLock
CBP_FP_DEVICE_API void STDCALL cbp_fp_unlock(int handle)
{
	int nRc = CBP_FP_OK;
	sprintf(theApp.m_Message, "handle (%d)", handle);
	theApp._LogMessage("cbp_fp_unlock", "Enter", theApp.m_Message, 2);

	theApp.DLL_unlock(handle);
	
	sprintf(theApp.m_Message, "Ret (%d)", nRc);
	theApp._LogMessage("cbp_fp_unlock", "Exit", theApp.m_Message, 2);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
void CLibCBPApp::callback_warning(int warn_code, char *warn_location)
{
	if(m_Callbacks.cbp_fp_onWarning)
	{
		sprintf(theApp.m_Message, "warn_code (%d)", warn_code);
		theApp._LogMessage("callback_warning", "Enter", theApp.m_Message, 1);
		theApp._LogMessage(warn_location, ">>> this warning code occured at", "", 1);

		char warn_msg[256];
		switch(warn_code)
		{
		case CBP_FP_WARNING_CONFIGURE:
			sprintf(warn_msg, "an error occurred during configuration"); break;
		case CBP_FP_WARNING_GET_DIRTINESS:
			sprintf(warn_msg, "an error occurred when checking for dirtiness"); break;
		case CBP_FP_WARNING_POWER_SAVE:
			sprintf(warn_msg, "an error occurred during power save"); break;
		case CBP_FP_WARNING_ALREADY_LOCKED:
			sprintf(warn_msg, "the connected scanner is already locked"); break;
		case CBP_FP_WARNING_NOT_LOCKED:
			sprintf(warn_msg, "the connected scanner is not locked"); break;
		case CBP_FP_WARNING_PROPERTY_NOT_SETABLE:
			sprintf(warn_msg, "the property is not setable"); break;
		case CBP_FP_WARNING_PROPERTY:
			sprintf(warn_msg, "an error occurred during a cbp_fp_getProperty call"); break;
		case CBP_FP_WARNING_CALIBRATE:
			sprintf(warn_msg, "an error occurred during the calibrate"); break;
		case CBP_FP_ERROR_INVALID_PARAM:
			sprintf(warn_msg, "the param is not valid"); break;
		case CBP_FP_ERROR_PREVIEW:
			sprintf(warn_msg, "an error occurred during preview or previewanalysis"); break;
		case CBP_FP_ERROR_UNSUPPORTED_COLLECTION_TYPE:
			sprintf(warn_msg, "an unsupported collection type has been requested"); break;
		case CBP_FP_WARNING:
		default:
			sprintf(warn_msg, "a general error has occurred and no details are available"); break;
		}

		sprintf(theApp.m_Message, "err_msg (%s)", warn_msg);
		theApp._LogMessage("callback_warning", "Exit", theApp.m_Message, 1);

		m_Callbacks.cbp_fp_onWarning(warn_code, warn_msg);
	}
}

void CLibCBPApp::callback_error(int err_code, char *error_location)
{
	if(m_Callbacks.cbp_fp_onError)
	{
		sprintf(theApp.m_Message, "err_code (%d)", err_code);
		theApp._LogMessage("callback_error", "Enter", theApp.m_Message, 1);
		theApp._LogMessage(error_location, ">>> this error occured at", "", 1);

		char error_msg[256];
		switch(err_code)
		{
		case CBP_FP_ERROR:
			sprintf(error_msg, "a general error has occurred and no details are available"); break;
		case CBP_FP_ERROR_NOT_IMPLEMENTED:
			sprintf(error_msg, "the function has not been implimented"); break;
		case CBP_FP_ERROR_NOT_OPENED:
			sprintf(error_msg, "the handle is not open"); break;
		case CBP_FP_ERROR_INVALID_HANDLE:
			sprintf(error_msg, "the handle is no longer valid"); break;
		case CBP_FP_ERROR_SCANNER_NOT_FOUND:
			sprintf(error_msg, "the scanner indicated has not been found "); break;
		case CBP_FP_ERROR_INVALID_SCANNER_SERIAL_NUMBER:
			sprintf(error_msg, "the scanner serial number is invalid"); break;
		case CBP_FP_ERROR_INVALID_SCANNER_NAME:
			sprintf(error_msg, "a scanner name is improperly formatted or not found"); break;
		case CBP_FP_ERROR_IO_ERROR:
			sprintf(error_msg, "the connected scanner experienced an IO error"); break;
		case CBP_FP_ERROR_UNSUPPORTED_SLAP:
			sprintf(error_msg, "an unsupported slap has been requested"); break;
		case CBP_FP_ERROR_BUSY:
			sprintf(error_msg, "the connected scanner is busy"); break;
		case CBP_FP_ERROR_NOT_INITIALIZED:
			sprintf(error_msg, "the library is not initialized"); break;
		case CBP_FP_ERROR_NOT_CLOSED:
			sprintf(error_msg, "the scanner is not closed"); break;
		case CBP_FP_ERROR_CALLBACK_REGISTRATION:
			sprintf(error_msg, "an error occurred during the call back registration"); break;
		case CBP_FP_ERROR_CAPTURE:
			sprintf(error_msg, "an error occurred during a capture of a slap"); break;
		case CBP_FP_ERROR_IMAGING:
			sprintf(error_msg, "an error occurred during a imaging of a slap"); break;
		case CBP_FP_ERROR_LOCK:
			sprintf(error_msg, "an error occurred during a cbp_fp_lock call"); break;
		case CBP_FP_ERROR_UNLOCK:
			sprintf(error_msg, "an error occurred during a cbp_fp_unLock call"); break;
		case CBP_FP_ERROR_UNITIALIZE:
			sprintf(error_msg, "an error occurred during unitialization"); break;
		case CBP_FP_ERROR_NOT_CONNECTED:
			sprintf(error_msg, "the scanner is disconnected"); break;
		case CBP_FP_ERROR_INVALID_PARAM:
			sprintf(error_msg, "the param is not valid"); break;
		case CBP_FP_ERROR_PREVIEW:
			sprintf(error_msg, "an error occurred during preview or previewanalysis"); break;
		case CBP_FP_ERROR_UNSUPPORTED_COLLECTION_TYPE:
			sprintf(error_msg, "an unsupported collection type has been requested"); break;
		case CBP_FP_ERROR_SCANNER_DISCONNECTED:
			sprintf(error_msg, "Check the USB Cable"); break;
		default:
			sprintf(error_msg, "a general error has occurred and no details are available"); break;
		}

		sprintf(theApp.m_Message, "err_msg (%s)", error_msg);
		theApp._LogMessage("callback_error", "Exit", theApp.m_Message, 1);

		m_Callbacks.cbp_fp_onError(err_code, error_msg);
	}
}

void CLibCBPApp::DLL_calibrate(int handle)
{
	if(pParam_calibrate->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_calibrate");
		return;
	}

	pParam_calibrate->pWnd = (CWnd*)this;
	pParam_calibrate->handle = handle;

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_calibrate, (LPVOID)pParam_calibrate, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

void CLibCBPApp::DLL_capture(int handle, cbp_fp_slapType slapType, cbp_fp_collectionType collectionType)
{
	if(pParam_capture->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_capture");
		return;
	}

	pParam_capture->pWnd = (CWnd*)this;
	pParam_capture->handle = handle;
	pParam_capture->slapType = slapType;
	pParam_capture->collectionType = collectionType;

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_capture, (LPVOID)pParam_capture, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

void CLibCBPApp::DLL_close(int handle)
{
	if(pParam_close->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_close");
		return;
	}

	pParam_close->pWnd = (CWnd*)this;
	pParam_close->handle = handle;

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_close, (LPVOID)pParam_close, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

void CLibCBPApp::DLL_configure(int handle) // lock required
{
	if(pParam_configure->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_configure");
		return;
	}

	pParam_configure->pWnd = (CWnd*)this;
	pParam_configure->handle = handle;

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_configure, (LPVOID)pParam_configure, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

void CLibCBPApp::DLL_enumDevices(char *requestID)
{
	if(pParam_enumDevices->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_enumDevices");
		return;
	}

	pParam_enumDevices->pWnd = (CWnd*)this;
	pParam_enumDevices->pRequestID = requestID;

	/*int i=0;
	while(m_bisNotInitializeYet == TRUE)
	{
		_LogMessage("cbp_fp_enumDevices", "Waiting 100 ms", "", 2);
		Sleep(100);
		i++;

		if(i == 10)
			break;
	}*/

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_enumDevices, (LPVOID)pParam_enumDevices, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

void CLibCBPApp::DLL_getDirtiness(int handle)  // lock required
{
	if(pParam_getDirtiness->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_getDirtiness");
		return;
	}

	pParam_getDirtiness->pWnd = (CWnd*)this;
	pParam_getDirtiness->handle = handle;

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_getDirtiness, (LPVOID)pParam_getDirtiness, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

void CLibCBPApp::DLL_getLockInfo(int handle)
{
	/*if(pParam_getLockInfo->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_getLockInfo");
		return;
	}*/

	pParam_getLockInfo->pWnd = (CWnd*)this;
	//pParam_getLockInfo->ThreadCount++;
	pParam_getLockInfo->handle = handle;

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_getLockInfo, (LPVOID)pParam_getLockInfo, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

char* CLibCBPApp::DLL_getProperty(int handle, char *name)  // no callback
{
	char cValue[128];

	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = _GetUsbDevicesFromHandle(handle)) == NULL)
	{
		callback_error(CBP_FP_ERROR_NOT_INITIALIZED, "cbp_fp_getProperty");
		return "";
	}

	if(pCBPUsbDev->m_bInitialized == FALSE)
	{
		callback_error(CBP_FP_ERROR_NOT_INITIALIZED, "cbp_fp_getProperty");
		return "";
	}

	if(pCBPUsbDev->m_nDevHandle != handle)
	{
		callback_error(CBP_FP_ERROR_INVALID_HANDLE, "cbp_fp_getProperty");
		return "";
	}

	if(!strcmp(name, CBP_FP_PROPERTY_SUPPORTED_COLLECTION_TYPES))
	{
		sprintf(m_strRet, "collection_rolled,collection_flat");
	}
	else if(!strcmp(name, CBP_FP_PROPERTY_SUPPORTED_SLAPS))
	{
		sprintf(m_strRet, "slap_rightHand,slap_leftHand,slap_twoFingers,slap_twoThumbs,slap_rightThumb,slap_rightIndex,slap_rightMiddle,slap_rightRing,\
						slap_rightLittle,slap_leftThumb,slap_leftIndex,slap_leftMiddle,slap_leftRing,slap_leftLittle,slap_twotThumbs,slap_unknown");
	}
	else if(!strcmp(name, CBP_FP_PROPERTY_MAKE))
	{
		sprintf(m_strRet, STRING_SCANNER_MAKE);
	}
	else if(!strcmp(name, CBP_FP_PROPERTY_MODEL))
	{
		if(IBSU_GetProperty(handle, ENUM_IBSU_PROPERTY_PRODUCT_ID, cValue) == IBSU_STATUS_OK)
			sprintf(m_strRet, "%s", cValue);
	}
	else if(!strcmp(name, CBP_FP_PROPERTY_SERIAL))
	{
		if(IBSU_GetProperty(handle, ENUM_IBSU_PROPERTY_SERIAL_NUMBER, cValue) == IBSU_STATUS_OK)
			sprintf(m_strRet, "%s", cValue);
	}
	else if(!strcmp(name, CBP_FP_PROPERTY_SOFTWARE_VERSION))
	{
#ifdef _WINDOWS
		CFileVersionInfo verInfo;
		if (verInfo.Open(m_hInstance))
		{
			sprintf(m_strRet, "%d.%d.%d.%d", verInfo.GetFileVersionMajor(),
					verInfo.GetFileVersionMinor(),
					verInfo.GetFileVersionBuild(),
					verInfo.GetFileVersionQFE());

			//sprintf(pVerInfo->Product, "%d.%d.%d.%d", verInfo.GetProductVersionMajor(),
			//		verInfo.GetProductVersionMinor(),
			//		verInfo.GetProductVersionBuild(),
			//		verInfo.GetProductVersionQFE());
		}
		else
		{
			sprintf(m_strRet, "?.?.?.?");
		}
#elif defined(__linux__)
		strcpy(m_strRet, pszLinuxFileVersion);
#endif
	}
	else if(!strcmp(name, CBP_FP_PROPERTY_FIRMWARE_VERSION))
	{
		if(IBSU_GetProperty(handle, ENUM_IBSU_PROPERTY_FIRMWARE, cValue) == IBSU_STATUS_OK)
			sprintf(m_strRet, "%s", cValue);
	}
	else if(!strcmp(name, CBP_FP_PROPERTY_PREVIEW_PIXEL_HEIGHT))
	{
		if(IBSU_GetProperty(handle, ENUM_IBSU_PROPERTY_IMAGE_HEIGHT, cValue) == IBSU_STATUS_OK)
			sprintf(m_strRet, "%s", cValue);
	}
	else if(!strcmp(name, CBP_FP_PROPERTY_PREVIEW_PIXEL_WIDTH))
	{
		if(IBSU_GetProperty(handle, ENUM_IBSU_PROPERTY_IMAGE_WIDTH, cValue) == IBSU_STATUS_OK)
			sprintf(m_strRet, "%s", cValue);
	}
	else if(!strcmp(name, CBP_FP_PROPERTY_CAPTURE_PIXEL_HEIGHT))
	{
		if(IBSU_GetProperty(handle, ENUM_IBSU_PROPERTY_IMAGE_HEIGHT, cValue) == IBSU_STATUS_OK)
			sprintf(m_strRet, "%s", cValue);
	}
	else if(!strcmp(name, CBP_FP_PROPERTY_CAPTURE_PIXEL_WIDTH))
	{
		if(IBSU_GetProperty(handle, ENUM_IBSU_PROPERTY_IMAGE_WIDTH, cValue) == IBSU_STATUS_OK)
			sprintf(m_strRet, "%s", cValue);
	}
	else if(!strcmp(name, CBP_FP_PROPERTY_PREVIEW_DPI))
	{
		sprintf(m_strRet, "500");
	}
	else if(!strcmp(name, CBP_FP_PROPERTY_CAPTURE_DPI))
	{
		sprintf(m_strRet, "500");
	}
	else if(!strcmp(name, CBP_FP_PROPERTY_DEBUG_LEVEL))
	{
		sprintf(m_strRet, "%d", m_DebugLevel);
	}
	else if(!strcmp(name, CBP_FP_PROPERTY_DEBUG_FILE_NAME))
	{
		sprintf(m_strRet, "%s", m_DebugFileName);
	}
	else
	{
		sprintf(m_strRet, "");
		callback_warning(CBP_FP_WARNING_PROPERTY, "cbp_fp_getProperty");
	}

	return m_strRet;
}

// no callback, but if error, then call cbp_fp_error(CBP_FP_ERROR_PROPERTY_NOT_SETABLE)
void CLibCBPApp::DLL_setProperty(int handle, struct cbp_fp_property *property)
{
	int iValue;

	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = _GetUsbDevicesFromHandle(handle)) == NULL)
	{
		callback_error(CBP_FP_ERROR_NOT_INITIALIZED, "cbp_fp_setProperty");
		return;
	}

	if(pCBPUsbDev->m_bInitialized == FALSE)
	{
		callback_error(CBP_FP_ERROR_NOT_INITIALIZED, "cbp_fp_setProperty");
		return;
	}

	if(pCBPUsbDev->m_nDevHandle != handle)
	{
		callback_error(CBP_FP_ERROR_INVALID_HANDLE, "cbp_fp_setProperty");
		return;
	}

	if( !strcmp(property->name, CBP_FP_PROPERTY_SUPPORTED_COLLECTION_TYPES) ||
		!strcmp(property->name, CBP_FP_PROPERTY_SUPPORTED_SLAPS) ||
		!strcmp(property->name, CBP_FP_PROPERTY_MAKE) ||
		!strcmp(property->name, CBP_FP_PROPERTY_MODEL) ||
		!strcmp(property->name, CBP_FP_PROPERTY_SERIAL) ||
		!strcmp(property->name, CBP_FP_PROPERTY_SOFTWARE_VERSION) ||
		!strcmp(property->name, CBP_FP_PROPERTY_FIRMWARE_VERSION) ||
		!strcmp(property->name, CBP_FP_PROPERTY_PREVIEW_PIXEL_HEIGHT) ||
		!strcmp(property->name, CBP_FP_PROPERTY_PREVIEW_PIXEL_WIDTH) ||
		!strcmp(property->name, CBP_FP_PROPERTY_CAPTURE_PIXEL_HEIGHT) ||
		!strcmp(property->name, CBP_FP_PROPERTY_CAPTURE_PIXEL_WIDTH) ||
		!strcmp(property->name, CBP_FP_PROPERTY_PREVIEW_DPI) ||
		!strcmp(property->name, CBP_FP_PROPERTY_CAPTURE_DPI))
	{
		callback_warning(CBP_FP_WARNING_PROPERTY_NOT_SETABLE, "cbp_fp_setProperty");
	}
	else if(!strcmp(property->name, CBP_FP_PROPERTY_DEBUG_LEVEL))
	{
		iValue = atoi(property->value);
		if(iValue >= 0 && iValue <= 2)
		{
			m_DebugLevel = iValue;
			m_INIMgr.WriteProperty("DebugInfo", "Level", m_DebugLevel);
		}
		else
			callback_warning(CBP_FP_WARNING_SET_PROPERTY, "cbp_fp_setProperty");
	}
	else if(!strcmp(property->name, CBP_FP_PROPERTY_DEBUG_FILE_NAME))
	{
		sprintf(m_DebugFileName, "%s", property->value);
		m_INIMgr.WriteProperty("DebugInfo", "Filename", m_DebugFileName);
	}
	else
	{
		callback_warning(CBP_FP_WARNING_SET_PROPERTY, "cbp_fp_setProperty");
	}
}

void CLibCBPApp::DLL_initialize(char *requestID)
{
	if(pParam_initialize->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_initialize");
		return;
	}

	pParam_initialize->pWnd = (CWnd*)this;
	pParam_initialize->pRequestID = requestID;
	//m_bisNotInitializeYet = TRUE;

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_initialize, (LPVOID)pParam_initialize, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

void CLibCBPApp::DLL_lock(int handle)
{
	if(pParam_lock->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_lock");
		return;
	}

	pParam_lock->pWnd = (CWnd*)this;
	pParam_lock->handle = handle;

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_lock, (LPVOID)pParam_lock, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

void CLibCBPApp::DLL_open(struct cbp_fp_device *device)
{
	if(pParam_open->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_open");
		return;
	}

	pParam_open->pWnd = (CWnd*)this;
	if(device == NULL)
	{
		sprintf(pParam_open->device->make, "");
		sprintf(pParam_open->device->model, "");
		sprintf(pParam_open->device->serialNumber, "");
	}
	else
	{
		sprintf(pParam_open->device->make, "%s", device->make);
		sprintf(pParam_open->device->model, "%s", device->model);
		sprintf(pParam_open->device->serialNumber, "%s", device->serialNumber);
	}

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_open, (LPVOID)pParam_open, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

void CLibCBPApp::DLL_powerSave(int handle, bool powerSaveOn) // lock required
{
	if(pParam_powerSave->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_powerSave");
		return;
	}

	pParam_powerSave->pWnd = (CWnd*)this;
	pParam_powerSave->handle = handle;
	pParam_powerSave->powerSaveOn = powerSaveOn;

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_powerSave, (LPVOID)pParam_powerSave, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

int CLibCBPApp::DLL_registerCallBacks(struct cbp_fp_callBacks *callBacks) // no callback
{
	if(::IsBadReadPtr(callBacks, sizeof(cbp_fp_callBacks)))
	{
		callback_error(CBP_FP_ERROR_INVALID_PARAM, "cbp_fp_registerCallBacks");
		return CBP_FP_ERROR_INVALID_PARAM;
	}
	m_Callbacks.cbp_fp_onCalibrate = callBacks->cbp_fp_onCalibrate;
	m_Callbacks.cbp_fp_onCapture = callBacks->cbp_fp_onCapture;
	m_Callbacks.cbp_fp_onClose = callBacks->cbp_fp_onClose;
	m_Callbacks.cbp_fp_onConfigure = callBacks->cbp_fp_onConfigure;
	m_Callbacks.cbp_fp_onEnumDevices = callBacks->cbp_fp_onEnumDevices;
	m_Callbacks.cbp_fp_onError = callBacks->cbp_fp_onError;
	m_Callbacks.cbp_fp_onGetDirtiness = callBacks->cbp_fp_onGetDirtiness;
	m_Callbacks.cbp_fp_onGetLockInfo = callBacks->cbp_fp_onGetLockInfo;
	m_Callbacks.cbp_fp_onInitialize = callBacks->cbp_fp_onInitialize;
	m_Callbacks.cbp_fp_onLock = callBacks->cbp_fp_onLock;
	m_Callbacks.cbp_fp_onOpen = callBacks->cbp_fp_onOpen;
	m_Callbacks.cbp_fp_onPowerSave = callBacks->cbp_fp_onPowerSave;
	m_Callbacks.cbp_fp_onPreview = callBacks->cbp_fp_onPreview;
	m_Callbacks.cbp_fp_onPreviewAnalysis = callBacks->cbp_fp_onPreviewAnalysis;
	m_Callbacks.cbp_fp_onStopPreview = callBacks->cbp_fp_onStopPreview;
	m_Callbacks.cbp_fp_onUninitialize = callBacks->cbp_fp_onUninitialize;
	m_Callbacks.cbp_fp_onUnLock = callBacks->cbp_fp_onUnLock;
	m_Callbacks.cbp_fp_onWarning = callBacks->cbp_fp_onWarning;

	return CBP_FP_OK;
}

void CLibCBPApp::DLL_startImaging(int handle, cbp_fp_slapType slapType, cbp_fp_collectionType collectionType)
{
	if(pParam_startImaging->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_startImaging");
		return;
	}

	pParam_startImaging->pWnd = (CWnd*)this;
	pParam_startImaging->handle = handle;
	pParam_startImaging->slapType = slapType;
	pParam_startImaging->collectionType = collectionType;

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_startImaging, (LPVOID)pParam_startImaging, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

void CLibCBPApp::DLL_stopImaging(int handle)
{
	if(pParam_stopImaging->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_stopImaging");
		return;
	}

	pParam_stopImaging->pWnd = (CWnd*)this;
	pParam_stopImaging->handle = handle;

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_stopImaging, (LPVOID)pParam_stopImaging, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

void CLibCBPApp::DLL_uninitialize(char *requestID)
{
	if(pParam_uninitialize->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_uninitialize");
		return;
	}

	pParam_uninitialize->pWnd = (CWnd*)this;
	pParam_uninitialize->pRequestID = requestID;

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_uninitialize, (LPVOID)pParam_uninitialize, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

void CLibCBPApp::DLL_unlock(int handle)
{
	if(pParam_unlock->pWnd != NULL)
	{
		callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_unlock");
		return;
	}

	pParam_unlock->pWnd = (CWnd*)this;
	pParam_unlock->handle = handle;

	VERIFY( ::AfxBeginThread( _THREAD_cbp_fp_unlock, (LPVOID)pParam_unlock, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

void CLibCBPApp::_ControlLEDforKojak(int handle, IBSU_FingerQualityState *fingerQualityState, int finger_count, int ledColor,
									 cbp_fp_slapType slapType, cbp_fp_collectionType collectionType)
{
	DWORD setLEDs = IBSU_LED_NONE;
	IBSU_ImageType imageType;
	unsigned char ShiftedFlag[IBSU_MAX_SEGMENT_COUNT];
	int i, finger_index = NONE_FINGER;
	
	memset(ShiftedFlag, 0, sizeof(ShiftedFlag));

	for( i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
	{
		if( fingerQualityState[i] == ENUM_IBSU_QUALITY_INVALID_AREA_TOP ||
			fingerQualityState[i] == ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM )
			ShiftedFlag[i] |= 1;
		else if( fingerQualityState[i] == ENUM_IBSU_QUALITY_INVALID_AREA_LEFT ||
			fingerQualityState[i] == ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT )
			ShiftedFlag[i] |= 2;
		else if( fingerQualityState[i] != ENUM_IBSU_FINGER_NOT_PRESENT )
			ShiftedFlag[i] = 0xFF;
	}

	if(collectionType == collection_rolled)	
		imageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
	else
		imageType = ENUM_IBSU_FLAT_SINGLE_FINGER;

	switch(slapType)
	{
	case slap_rightHand:
		finger_index = RIGHT_HAND;
		imageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
		break;
	case slap_leftHand:
		finger_index = LEFT_HAND;
		imageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
		break;
	case slap_twoFingers:
	case slap_twoThumbs:
	case slap_twotThumbs:
		finger_index = BOTH_THUMBS;
		imageType = ENUM_IBSU_FLAT_TWO_FINGERS;
		break;
	case slap_rightThumb:
		finger_index = RIGHT_THUMB;
		break;
	case slap_rightIndex:
		finger_index = RIGHT_INDEX;
		break;
	case slap_rightMiddle:
		finger_index = RIGHT_MIDDLE;
		break;
	case slap_rightRing:
		finger_index = RIGHT_RING;
		break;
	case slap_rightLittle:
		finger_index = RIGHT_LITTLE;
		break;
	case slap_leftThumb:
		finger_index = LEFT_THUMB;
		break;
	case slap_leftIndex:
		finger_index = LEFT_INDEX;
		break;
	case slap_leftMiddle:
		finger_index = LEFT_MIDDLE;
		break;
	case slap_leftRing:
		finger_index = LEFT_RING;
		break;
	case slap_leftLittle:
		finger_index = LEFT_LITTLE;
		break;
	case slap_stitchedLeftThumb:
	case slap_stitchedRightThumb:
	case slap_unknown:
	default:
		finger_index = NONE_FINGER;
		imageType = ENUM_IBSU_TYPE_NONE;
		break;
	}

	if(ShiftedFlag[0] > 0 || ShiftedFlag[1] > 0 || ShiftedFlag[2] > 0 || ShiftedFlag[3] > 0)
	{
		if(imageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_ROLL;
	    }

		DWORD LEDlist[IBSU_MAX_SEGMENT_COUNT] = {IBSU_LED_NONE, IBSU_LED_NONE, IBSU_LED_NONE, IBSU_LED_NONE};
		unsigned char si[IBSU_MAX_SEGMENT_COUNT] = {-1,-1,-1,-1}, idx=0;
		for(int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++)
		{
			if(ShiftedFlag[i] > 0)
			{
				si[idx++] = i;
			}
		}

		if((m_AddBlinkIndex++) % 2 == 0)
		{
			if(finger_index == LEFT_HAND)
			{
				setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
				LEDlist[0] = (ShiftedFlag[0] == 0x01 || ShiftedFlag[0] == 0x03) ? IBSU_LED_F_LEFT_LITTLE_RED :
							 (ShiftedFlag[0] == 0x02 || ShiftedFlag[0] == 0xFF) ? (IBSU_LED_F_LEFT_LITTLE_RED | IBSU_LED_F_LEFT_LITTLE_GREEN) : IBSU_LED_NONE;
				LEDlist[1] = (ShiftedFlag[1] == 0x01 || ShiftedFlag[1] == 0x03) ? IBSU_LED_F_LEFT_RING_RED :
							 (ShiftedFlag[1] == 0x02 || ShiftedFlag[1] == 0xFF) ? (IBSU_LED_F_LEFT_RING_RED | IBSU_LED_F_LEFT_RING_GREEN) : IBSU_LED_NONE;
				LEDlist[2] = (ShiftedFlag[2] == 0x01 || ShiftedFlag[2] == 0x03) ? IBSU_LED_F_LEFT_MIDDLE_RED :
							 (ShiftedFlag[2] == 0x02 || ShiftedFlag[2] == 0xFF) ? (IBSU_LED_F_LEFT_MIDDLE_RED | IBSU_LED_F_LEFT_MIDDLE_GREEN) : IBSU_LED_NONE;
				LEDlist[3] = (ShiftedFlag[3] == 0x01 || ShiftedFlag[3] == 0x03) ? IBSU_LED_F_LEFT_INDEX_RED :
							 (ShiftedFlag[3] == 0x02 || ShiftedFlag[3] == 0xFF) ? (IBSU_LED_F_LEFT_INDEX_RED | IBSU_LED_F_LEFT_INDEX_GREEN) : IBSU_LED_NONE;
			}
			else if(finger_index == RIGHT_HAND)
			{
				setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
				LEDlist[0] = (ShiftedFlag[0] == 0x01 || ShiftedFlag[0] == 0x03) ? IBSU_LED_F_RIGHT_INDEX_RED :
							 (ShiftedFlag[0] == 0x02 || ShiftedFlag[0] == 0xFF) ? (IBSU_LED_F_RIGHT_INDEX_RED | IBSU_LED_F_RIGHT_INDEX_GREEN) : IBSU_LED_NONE;
				LEDlist[1] = (ShiftedFlag[1] == 0x01 || ShiftedFlag[1] == 0x03) ? IBSU_LED_F_RIGHT_MIDDLE_RED :
							 (ShiftedFlag[1] == 0x02 || ShiftedFlag[1] == 0xFF) ? (IBSU_LED_F_RIGHT_MIDDLE_RED | IBSU_LED_F_RIGHT_MIDDLE_GREEN) : IBSU_LED_NONE;
				LEDlist[2] = (ShiftedFlag[2] == 0x01 || ShiftedFlag[2] == 0x03) ? IBSU_LED_F_RIGHT_RING_RED :
							 (ShiftedFlag[2] == 0x02 || ShiftedFlag[2] == 0xFF) ? (IBSU_LED_F_RIGHT_RING_RED | IBSU_LED_F_RIGHT_RING_GREEN) : IBSU_LED_NONE;
				LEDlist[3] = (ShiftedFlag[3] == 0x01 || ShiftedFlag[3] == 0x03) ? IBSU_LED_F_RIGHT_LITTLE_RED :
							 (ShiftedFlag[3] == 0x02 || ShiftedFlag[3] == 0xFF) ? (IBSU_LED_F_RIGHT_LITTLE_RED | IBSU_LED_F_RIGHT_LITTLE_GREEN) : IBSU_LED_NONE;
			}
			else if(finger_index == BOTH_THUMBS)
			{
				setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
				if(finger_count >= 2)
				{
					if(si[0] != -1)
					{
						LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_LEFT_THUMB_RED :
									 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE;
					}
					if(si[1] != -1)
					{
						LEDlist[1] = (ShiftedFlag[si[1]] == 0x01 || ShiftedFlag[si[1]] == 0x03) ? IBSU_LED_F_RIGHT_THUMB_RED :
									 (ShiftedFlag[si[1]] == 0x02 || ShiftedFlag[si[1]] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE;
					}
				}
				else if(finger_count >= 1)
				{
					if(si[0] != -1)
					{
						LEDlist[0] |= (ShiftedFlag[0] == 0x01 || ShiftedFlag[0] == 0x03) ? IBSU_LED_F_LEFT_THUMB_RED :
									  (ShiftedFlag[0] == 0x02 || ShiftedFlag[0] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE;
						LEDlist[0] |= (ShiftedFlag[1] == 0x01 || ShiftedFlag[1] == 0x03) ? IBSU_LED_F_LEFT_THUMB_RED :
									  (ShiftedFlag[1] == 0x02 || ShiftedFlag[1] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE;
					}
					if(si[1] != -1)
					{
						LEDlist[1] |= (ShiftedFlag[2] == 0x01 || ShiftedFlag[2] == 0x03) ? IBSU_LED_F_RIGHT_THUMB_RED :
									  (ShiftedFlag[2] == 0x02 || ShiftedFlag[2] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE;
						LEDlist[1] |= (ShiftedFlag[3] == 0x01 || ShiftedFlag[3] == 0x03) ? IBSU_LED_F_RIGHT_THUMB_RED :
									  (ShiftedFlag[3] == 0x02 || ShiftedFlag[3] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE;
					}
				}
				else
				{
					LEDlist[0] = (ShiftedFlag[0] == 0xFF || ShiftedFlag[1] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE;
					LEDlist[1] = (ShiftedFlag[2] == 0xFF || ShiftedFlag[3] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE;
				}
			}
			else if(finger_index >= LEFT_LITTLE && finger_index <= RIGHT_LITTLE)
			{
				switch(finger_index)
				{
				case LEFT_LITTLE: 	
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_LEFT_LITTLE_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_LITTLE_RED | IBSU_LED_F_LEFT_LITTLE_GREEN) : IBSU_LED_NONE; break;
				case LEFT_RING:
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_LEFT_RING_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_RING_RED | IBSU_LED_F_LEFT_RING_GREEN) : IBSU_LED_NONE; break;
				case LEFT_MIDDLE:
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_LEFT_MIDDLE_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_MIDDLE_RED | IBSU_LED_F_LEFT_MIDDLE_GREEN) : IBSU_LED_NONE; break;
				case LEFT_INDEX:
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_LEFT_INDEX_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_INDEX_RED | IBSU_LED_F_LEFT_INDEX_GREEN) : IBSU_LED_NONE; break;
				case LEFT_THUMB:
					setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_LEFT_THUMB_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_THUMB:
					setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_RIGHT_THUMB_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_INDEX:
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_RIGHT_INDEX_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_INDEX_RED | IBSU_LED_F_RIGHT_INDEX_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_MIDDLE:
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_RIGHT_MIDDLE_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_MIDDLE_RED | IBSU_LED_F_RIGHT_MIDDLE_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_RING:
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_RIGHT_RING_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_RING_RED | IBSU_LED_F_RIGHT_RING_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_LITTLE:				
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_RIGHT_LITTLE_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_LITTLE_RED | IBSU_LED_F_RIGHT_LITTLE_GREEN) : IBSU_LED_NONE; break;
				}
			}
		}
		else
		{
			if(finger_index == LEFT_HAND)
			{
				setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
				LEDlist[0] = (ShiftedFlag[0] == 0xFF) ? (IBSU_LED_F_LEFT_LITTLE_RED | IBSU_LED_F_LEFT_LITTLE_GREEN) : IBSU_LED_NONE;
				LEDlist[1] = (ShiftedFlag[1] == 0xFF) ? (IBSU_LED_F_LEFT_RING_RED | IBSU_LED_F_LEFT_RING_GREEN) : IBSU_LED_NONE;
				LEDlist[2] = (ShiftedFlag[2] == 0xFF) ? (IBSU_LED_F_LEFT_MIDDLE_RED | IBSU_LED_F_LEFT_MIDDLE_GREEN) : IBSU_LED_NONE;
				LEDlist[3] = (ShiftedFlag[3] == 0xFF) ? (IBSU_LED_F_LEFT_INDEX_RED | IBSU_LED_F_LEFT_INDEX_GREEN) : IBSU_LED_NONE;
			}
			else if(finger_index == RIGHT_HAND)
			{
				setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
				LEDlist[0] = (ShiftedFlag[0] == 0xFF) ? (IBSU_LED_F_RIGHT_INDEX_RED | IBSU_LED_F_RIGHT_INDEX_GREEN) : IBSU_LED_NONE;
				LEDlist[1] = (ShiftedFlag[1] == 0xFF) ? (IBSU_LED_F_RIGHT_MIDDLE_RED | IBSU_LED_F_RIGHT_MIDDLE_GREEN) : IBSU_LED_NONE;
				LEDlist[2] = (ShiftedFlag[2] == 0xFF) ? (IBSU_LED_F_RIGHT_RING_RED | IBSU_LED_F_RIGHT_RING_GREEN) : IBSU_LED_NONE;
				LEDlist[3] = (ShiftedFlag[3] == 0xFF) ? (IBSU_LED_F_RIGHT_LITTLE_RED | IBSU_LED_F_RIGHT_LITTLE_GREEN) : IBSU_LED_NONE;
			}
			else if(finger_index == BOTH_THUMBS)
			{
				setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
				if(finger_count >= 2)
				{
					if(si[0] != -1)
					{
						LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE;
					}
					if(si[1] != -1)
					{
						LEDlist[1] = (ShiftedFlag[si[1]] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE;
					}
				}
				else
				{
					LEDlist[0] = (ShiftedFlag[0] == 0xFF || ShiftedFlag[1] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE;
					LEDlist[1] = (ShiftedFlag[2] == 0xFF || ShiftedFlag[3] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE;
				}
			}
			else if(finger_index >= LEFT_LITTLE && finger_index <= RIGHT_LITTLE)
			{
				switch(finger_index)
				{
				case LEFT_LITTLE: 	
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_LITTLE_RED | IBSU_LED_F_LEFT_LITTLE_GREEN) : IBSU_LED_NONE; break;
				case LEFT_RING:
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_RING_RED | IBSU_LED_F_LEFT_RING_GREEN) : IBSU_LED_NONE; break;
				case LEFT_MIDDLE:
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_MIDDLE_RED | IBSU_LED_F_LEFT_MIDDLE_GREEN) : IBSU_LED_NONE; break;
				case LEFT_INDEX:
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_INDEX_RED | IBSU_LED_F_LEFT_INDEX_GREEN) : IBSU_LED_NONE; break;
				case LEFT_THUMB:
					setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_THUMB:
					setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_INDEX:
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_INDEX_RED | IBSU_LED_F_RIGHT_INDEX_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_MIDDLE:
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_MIDDLE_RED | IBSU_LED_F_RIGHT_MIDDLE_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_RING:
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_RING_RED | IBSU_LED_F_RIGHT_RING_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_LITTLE:				
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_LITTLE_RED | IBSU_LED_F_RIGHT_LITTLE_GREEN) : IBSU_LED_NONE; break;
				}
			}
		}

		for(int i=0; i<4; i++)
			setLEDs |= LEDlist[i];
	}
	else
    {
	    if(imageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_ROLL;
	    }

	    if(finger_index == RIGHT_THUMB || finger_index == BOTH_RIGHT_THUMB)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_THUMB_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_THUMB_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_THUMB_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_THUMB_RED;
		    }
	    }
	    else if(finger_index == LEFT_THUMB || finger_index == BOTH_LEFT_THUMB)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_THUMB_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_THUMB_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_THUMB_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_THUMB_RED;
		    }
	    }
	    else if(finger_index == BOTH_THUMBS)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_THUMB_GREEN;
				setLEDs |= IBSU_LED_F_RIGHT_THUMB_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_THUMB_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_THUMB_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
				if(finger_count > 1)
				{
					setLEDs |= IBSU_LED_F_RIGHT_THUMB_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_THUMB_RED;
					setLEDs |= IBSU_LED_F_LEFT_THUMB_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_THUMB_RED;
				}
			    else if(finger_count > 0)
				{
					setLEDs |= IBSU_LED_F_RIGHT_THUMB_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_THUMB_RED;
					setLEDs |= IBSU_LED_F_LEFT_THUMB_RED;
				}
				else
				{
					setLEDs |= IBSU_LED_F_LEFT_THUMB_RED;
					setLEDs |= IBSU_LED_F_RIGHT_THUMB_RED;
				}
		    }
	    }
	    ///////////////////LEFT HAND////////////////////
	    else if(finger_index == LEFT_INDEX)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
		    }
	    }
	    else if(finger_index == LEFT_MIDDLE)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
		    }
	    }
	    else if(finger_index == LEFT_INDEX_MIDDLE)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    if(finger_count > 1)
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
				}
			    else if(finger_count > 0)
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
				}
				else
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
				}
		    }
	    }
	    else if(finger_index == LEFT_RING)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_RING_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_RING_RED;
		    }
	    }
	    else if(finger_index == LEFT_LITTLE)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
		    }
	    }
	    else if(finger_index == LEFT_RING_LITTLE)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_RING_RED;
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    if(finger_count > 1)
				{
					setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
			    else if(finger_count > 0)
				{
					setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
				else
				{
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
		    }
	    }
	    else if(finger_index == LEFT_HAND)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
			    setLEDs |= IBSU_LED_F_LEFT_RING_RED;
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    if(finger_count > 3)
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
				else if(finger_count > 2)
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
			    else if(finger_count > 1)
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
			    else if(finger_count > 0)
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
				else
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
		    }
	    }
	    ///////////RIGHT HAND /////////////////////////
	    else if(finger_index == RIGHT_INDEX)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
		    }
	    }
	    else if(finger_index == RIGHT_MIDDLE)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
		    }
	    }
	    else if(finger_index == RIGHT_INDEX_MIDDLE)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    if(finger_count > 1)
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
				}
			    else if(finger_count > 0)
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
				}
				else
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
				}
		    }
	    }
	    else if(finger_index == RIGHT_RING)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
		    }
	    }
	    else if(finger_index == RIGHT_LITTLE)
	    {	
		    setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
		    }
	    }
	    else if(finger_index == RIGHT_RING_LITTLE)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    if(finger_count > 1)
				{
					setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
			    else if(finger_count > 0)
				{
					setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
				else
				{
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
		    }
	    }
	    else if(finger_index == RIGHT_HAND)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    if(finger_count > 3)
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
				else if(finger_count > 2)
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
				else if(finger_count > 1)
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
				else if(finger_count > 0)
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
				else
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
		    }
	    }
    }

	IBSU_SetLEDs(handle, setLEDs);
}

BOOL CLibCBPApp::_CheckDuplicate_LockInfo(int handle, long processID)
{
	for(int i=0; i<g_pSharedData->m_LockInfoArrCount; i++)
	{
		if( g_pSharedData->m_LockInfoArr[i].m_LockInfo.handle == handle &&
			g_pSharedData->m_LockInfoArr[i].m_LockInfo.lockPID == processID )
		{
			return TRUE;
		}
	}

	return FALSE;
}

int CLibCBPApp::_AddToArr_LockInfo(int handle, long processID)
{
	int lock_cnt = g_pSharedData->m_LockInfoArrCount;

	// check duplicate lock : already locked
	if(_CheckDuplicate_LockInfo(handle, processID) == TRUE)
		return CBP_FP_WARNING_ALREADY_LOCKED;

	g_pSharedData->m_LockInfoArr[lock_cnt].m_LockInfo.handle = handle;
	g_pSharedData->m_LockInfoArr[lock_cnt].m_LockInfo.lockPID = processID;
	g_pSharedData->m_LockInfoArr[lock_cnt].m_LockInfo.lockDuration = 0;
	g_pSharedData->m_LockInfoArr[lock_cnt].m_LockCreateTime = (long)clock();
	g_pSharedData->m_LockInfoArrCount++;

	return CBP_FP_OK;
}

int CLibCBPApp::_DeleteFromArr_LockInfo(int handle, long processID)
{
	int lock_cnt = g_pSharedData->m_LockInfoArrCount;
	int DeleteFlag = 0;
	for(int i=0; i<lock_cnt; i++)
	{
		if( g_pSharedData->m_LockInfoArr[i].m_LockInfo.handle == handle &&
			g_pSharedData->m_LockInfoArr[i].m_LockInfo.lockPID == processID )
		{
			g_pSharedData->m_LockInfoArr[i].m_LockInfo.handle = -1;
			g_pSharedData->m_LockInfoArr[i].m_LockInfo.lockPID = 0;
			g_pSharedData->m_LockInfoArr[i].m_LockInfo.lockDuration = 0;

			for(int j=i; j<lock_cnt-1; j++)
			{
				memcpy(&g_pSharedData->m_LockInfoArr[j], &g_pSharedData->m_LockInfoArr[j+1], sizeof(cbp_fp_lock_info));
			}
			g_pSharedData->m_LockInfoArrCount--;
			DeleteFlag = 1;
			break;
		}
	}

	if(DeleteFlag == 1)
		return CBP_FP_OK;

	return CBP_FP_WARNING_NOT_LOCKED;
}

int CLibCBPApp::_FindFromArr_LockInfo(int handle, long processID, CBPLockInfo *lockinfo)
{
	int lock_cnt = g_pSharedData->m_LockInfoArrCount;
	int DeleteFlag = 0;
	for(int i=0; i<lock_cnt; i++)
	{
		if( g_pSharedData->m_LockInfoArr[i].m_LockInfo.handle == handle &&
			g_pSharedData->m_LockInfoArr[i].m_LockInfo.lockPID == processID )
		{
			lockinfo->m_LockInfo.handle = g_pSharedData->m_LockInfoArr[i].m_LockInfo.handle;
			lockinfo->m_LockInfo.lockPID = g_pSharedData->m_LockInfoArr[i].m_LockInfo.lockPID;
			lockinfo->m_LockInfo.lockDuration = g_pSharedData->m_LockInfoArr[i].m_LockInfo.lockDuration;
			lockinfo->m_LockCreateTime = g_pSharedData->m_LockInfoArr[i].m_LockCreateTime;
			return CBP_FP_OK;
		}
	}

	return CBP_FP_WARNING_NOT_LOCKED;
}


////////////////////////////////////////////////////////////////////
// GetAnalysisCodeString()
////////////////////////////////////////////////////////////////////
void CLibCBPApp::_GetAnalysisCodeString(int code, char * codeString)
{
	switch (code)
	{
	case CBP_FP_ANALYSIS_CODE_GOOD_PRINT:
		sprintf(codeString, "Good print");
		break;

	case CBP_FP_ANALYSIS_CODE_NO_PRINT:
		sprintf(codeString, "No print");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_DARK:
		sprintf(codeString, "Too dark");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_LIGHT:
		sprintf(codeString, "Too light");
		break;

	case CBP_FP_ANALYSIS_CODE_BAD_SHAPE:
		sprintf(codeString, "Bad shape");
		break;

	case CBP_FP_ANALYSIS_CODE_WRONG_SLAP:
		sprintf(codeString, "Wrong slap");
		break;

	case CBP_FP_ANALYSIS_CODE_BAD_POSITION:
		sprintf(codeString, "Bad position");
		break;

	case CBP_FP_ANALYSIS_CODE_ROTATE_CLOCKWISE:
		sprintf(codeString, "Rotate clockwise");
		break;

	case CBP_FP_ANALYSIS_CODE_ROTATE_COUNTERCLOCKWISE:
		sprintf(codeString, "Rotate counterclockwise");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_HIGH:
		sprintf(codeString, "Too high");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_LOW:
		sprintf(codeString, "Too low");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_LEFT:
		sprintf(codeString, "Too left");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_RIGHT:
		sprintf(codeString, "Too right");
		break;

	case CBP_FP_ANALYSIS_CODE_SPOOF_DETECTED:
		sprintf(codeString, "Spoof detected");
		break;

	default:
		sprintf(codeString, "Unknown code %d", code);
		break;
	}
}

void CLibCBPApp::_GetSlapTypeCodeString(cbp_fp_slapType code, char * codeString)
{
	switch(code)
	{
	case slap_rightHand: 
		sprintf(codeString, "slap_rightHand"); 
		break;

	case slap_leftHand:
		sprintf(codeString, "slap_leftHand"); 
		break;

	case slap_twoFingers: 
		sprintf(codeString, "slap_twoFingers"); 
		break;

	case slap_twoThumbs: 
		sprintf(codeString, "slap_twoThumbs"); 
		break;

	case slap_rightThumb: 
		sprintf(codeString, "slap_rightThumb"); 
		break;

	case slap_rightIndex: 
		sprintf(codeString, "slap_rightIndex"); 
		break;

	case slap_rightMiddle: 
		sprintf(codeString, "slap_rightMiddle"); 
		break;

	case slap_rightRing: 
		sprintf(codeString, "slap_rightRing"); 
		break;

	case slap_rightLittle: 
		sprintf(codeString, "slap_rightLittle"); 
		break;

	case slap_leftThumb: 
		sprintf(codeString, "slap_leftThumb"); 
		break;

	case slap_leftIndex: 
		sprintf(codeString, "slap_leftIndex"); 
		break;

	case slap_leftMiddle: 
		sprintf(codeString, "slap_leftMiddle"); 
		break;

	case slap_leftRing: 
		sprintf(codeString, "slap_leftRing"); 
		break;

	case slap_leftLittle: 
		sprintf(codeString, "slap_leftLittle"); 
		break;

	case slap_twotThumbs: 
		sprintf(codeString, "slap_twotThumbs"); 
		break;

	case slap_stitchedLeftThumb: 
		sprintf(codeString, "slap_stitchedLeftThumb"); 
		break;

	case slap_stitchedRightThumb: 
		sprintf(codeString, "slap_stitchedRightThumb"); 
		break;

	case slap_unknown: 
	default:
		sprintf(codeString, "slap_unknown"); 
		break;
	}
}

void CLibCBPApp::_GetCollectionTypeCodeString(cbp_fp_collectionType code, char * codeString)
{
	switch(code)
	{
	case collection_rolled: 
		sprintf(codeString, "collection_rolled"); 
		break;

	case collection_flat: 
		sprintf(codeString, "collection_flat"); 
		break;

	case collection_contactless: 
		sprintf(codeString, "collection_contactless"); 
		break;
	
	case collection_unknown:
	default: 
		sprintf(codeString, "collection_unknown");
		break;
	}
}

void CLibCBPApp::_Read_INI_Info()
{
	CString DebugFile;

	m_INIMgr.ReadProperty("DebugInfo", "Level", m_DebugLevel);
	m_INIMgr.ReadProperty("DebugInfo", "Filename", DebugFile);
	m_INIMgr.ReadProperty("DebugInfo", "FileSizeLimit", m_DebugFileSizeLimit);
	m_INIMgr.ReadProperty("DebugInfo", "LogFileNumber", m_DebugFileCount);

	if(m_DebugFileCount < MIN_LOG_COUNT_INDEX)
	{
		m_DebugFileCount = MIN_LOG_COUNT_INDEX;
		//m_INIMgr.WriteProperty("DebugInfo", "LogFileNumber", m_DebugFileCount);
	}
	else if(m_DebugFileCount > MAX_LOG_COUNT_INDEX)
	{
		m_DebugFileCount = MAX_LOG_COUNT_INDEX;
		//m_INIMgr.WriteProperty("DebugInfo", "LogFileNumber", m_DebugFileCount);
	}

	sprintf(m_DebugFileName, "%s", (char*)DebugFile.GetBuffer());
}

void CLibCBPApp::_LogManage()
{
	std::vector<CString> m_LogFiles;
	m_LogFiles.clear();

	// Find exist Log files via CFileFind
	// Rename latest log for rollover
	CString NewFilename_tmp = m_DebugFileName;
	CFileFind finder;
	CString find_filename;
	int i, vector_size, cnt_found = 0;

	// Get Log file name from INI 
	NewFilename_tmp.Delete(NewFilename_tmp.GetLength()-4, 4);

	// Find <Defined Log file name>*.log file in work directory
	// Count number of log files in work directory
	find_filename = NewFilename_tmp;
	find_filename.Append(_T("_*.log"));

	BOOL bWorking = finder.FindFile(find_filename);
	while(bWorking)
	{
		cnt_found++;
		bWorking = finder.FindNextFile();
		if(finder.IsDots()) continue;
		if(finder.IsDirectory()) continue;

		// Add fonnd files in Vector<String> 
		m_LogFiles.push_back(finder.GetFileName());
	}

	vector_size = m_LogFiles.size();

	// Remove target files (Oldest files)
	for (i=0; i < vector_size-m_DebugFileCount; i++)
	{
		DeleteFile(m_LogFiles.at(i));
	}

	std::vector<CString>::iterator it = m_LogFiles.begin();
	i=0;
	while( it != m_LogFiles.end() )
	{
		if(i >= vector_size-m_DebugFileCount) 
			break;

		i++;

		it = m_LogFiles.erase(it);
	}

	vector_size = m_LogFiles.size();

	m_VFoundLogFilesCount = 0;
	for (i=0; i < vector_size; i++)
	{
		m_VFoundLogFiles[i] = m_LogFiles.at(i);
		m_VFoundLogFilesCount++;
	}

	std::vector<CString>::iterator it2 = m_LogFiles.begin();
	while( it2 != m_LogFiles.end() )
	{
		it2 = m_LogFiles.erase(it2);
	}
	m_LogFiles.clear();
}

void CLibCBPApp::_DrawLine(unsigned char *Img, int sx, int sy, int ex, int ey, int width, int height, unsigned char color)
{
    int dx = abs(ex - sx); // 시작 점과 끝 점의 각 x 좌표의 거리
    int dy = abs(ey - sy); // 시작 점과 끝 점의 각 y 좌표의 거리
     
    if (dy <= dx)
    {
        int p = 2 * (dy - dx);
        int y = sy;

        int inc_x = 1;
        if (ex < sx)
        {
            inc_x = -1;
        }
        int inc_y = 1;
        if (ey < sy)
        {
            inc_y = -1;
        }
        for (int x = sx; (sx <= ex ? x <= ex : x >= ex); x += inc_x)
        {
            if (0 >= p)
            {
                p += 2 * dy;
            }
            else
            {
                p += 2 * (dy - dx);
                y += inc_y;
            }
			
			if(y >= 0 && y <= height-1 && x >= 0 && x <= width-1)
				Img[y*width+x] = color;
        }
    }
    else
    {
        int p = 2 * (dx - dy);
        int x = sx;

        int inc_x = 1;
        if (ex < sx)
        {
            inc_x = -1;
        }
        int inc_y = 1;
        if (ey < sy)
        {
            inc_y = -1;
        }
        for (int y = sy; (sy <= ey ? y <= ey : y >= ey); y += inc_y)
        {
            if (0 >= p)
            {
                p += 2 * dx;
            }
            else
            {
                p += 2 * (dx - dy);
                x += inc_x;
            }

			if(y >= 0 && y <= height-1 && x >= 0 && x <= width-1)
			  Img[y*width+x] = color;
        }
	}
}
