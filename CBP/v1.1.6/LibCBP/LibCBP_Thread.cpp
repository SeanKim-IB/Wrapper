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

UINT CLibCBPApp::_THREAD_cbp_fp_calibrate(LPVOID pParam)
{
	ThreadParam_cbp_fp_calibrate *thread_param = (ThreadParam_cbp_fp_calibrate *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(thread_param->handle)) == NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_INVALID_HANDLE, "cbp_fp_calibrate");
		goto done;
	}

	CBPLockInfo lockinfo;
	int nRc = pApp->_FindFromArr_LockInfo(thread_param->handle, (long)GetCurrentProcessId(), &lockinfo);

	if( nRc != CBP_FP_OK )
	{
		pApp->callback_error(CBP_FP_ERROR_NOT_LOCKED, "cbp_fp_calibrate");
		goto done;
	}

	//Sleep(1000);
	//pApp->callback_error(CBP_FP_ERROR_NOT_IMPLEMENTED, "cbp_fp_calibrate");

	if(pApp->m_Callbacks.cbp_fp_onCalibrate)
	{
		pApp->_LogMessage("cbp_fp_onCalibrate", "Enter", "", 2);
		pApp->m_Callbacks.cbp_fp_onCalibrate(thread_param->handle);
		pApp->_LogMessage("cbp_fp_onCalibrate", "Exit", "", 2);
	}

done:
	
	thread_param->pWnd = NULL;

	return TRUE;
}

UINT CLibCBPApp::_THREAD_cbp_fp_capture(LPVOID pParam)
{
	ThreadParam_cbp_fp_capture *thread_param = (ThreadParam_cbp_fp_capture *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	//BOOL					IsActive;
	int						nRc = IBSU_STATUS_OK;
	DWORD					LedType = IBSU_LED_NONE;
	int                     pImageStatus;
	IBSU_ImageData          pImage;
	IBSU_ImageType          pImageType;
	int                     pDetectedFingerCount;
	IBSU_ImageData          pSegmentImageArray[IBSU_MAX_SEGMENT_COUNT];
	IBSU_SegmentPosition    pSegmentPositionArray[IBSU_MAX_SEGMENT_COUNT];
	int                     pSegmentImageArrayCount;
	IBSU_FingerCountState   pFingerCountState;
	IBSU_FingerQualityState pQualityArray[IBSU_MAX_SEGMENT_COUNT];
	int                     pQualityArrayCount;
	BOOL					WetFingerDetected = FALSE;
	BOOL					IsValidGeo = TRUE;
	DWORD					fIndex = 0;
	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(thread_param->handle)) == NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_INVALID_HANDLE, "cbp_fp_capture");
		goto done;
	}

	// to support clear platen, this code should be removed.
	/*nRc = IBSU_IsCaptureActive(thread_param->handle, &IsActive);
	if( nRc == IBSU_STATUS_OK && IsActive )
	{
		pApp->callback_error(CBP_FP_ERROR_NOW_CAPTURING, "cbp_fp_capture");
		goto done;
	}*/

/*	if( pCBPUsbDev->m_bReceivedResultImage == FALSE )
	{
		pApp->callback_error(CBP_FP_ERROR_CAPTURE, "cbp_fp_capture");
		goto done;
	}
*/
	nRc = IBSU_BGetImageEx(thread_param->handle, &pImageStatus, &pImage, &pImageType, &pDetectedFingerCount, pSegmentImageArray, 
				pSegmentPositionArray, &pSegmentImageArrayCount, &pFingerCountState, pQualityArray, &pQualityArrayCount);
	
	if( nRc == IBSU_STATUS_OK )
	{
		cbp_fp_grayScaleCapture grayScaleCapture;
		grayScaleCapture.image = new BYTE [pImage.Width*pImage.Height];
		//memcpy(grayScaleCapture.image, pImage.Buffer, pImage.Width*pImage.Height);
		// flip
		BYTE *pBuf = (BYTE *)pImage.Buffer;
		for(int i=0; i<(int)pImage.Height; i++)
		{
			memcpy(&grayScaleCapture.image[i*pImage.Width], &pBuf[(pImage.Height-1-i)*pImage.Width], pImage.Width);
		}

		grayScaleCapture.width = pImage.Width;
		grayScaleCapture.height = pImage.Height;

		grayScaleCapture.slapType = pCBPUsbDev->m_SlapType;
		grayScaleCapture.collectionType = pCBPUsbDev->m_CollectionType;

		// wet finger detect for segmented images
		for(int i=0; i<pSegmentImageArrayCount; i++)
		{
			nRc = IBSU_CheckWetFinger(thread_param->handle, pSegmentImageArray[i]);

			if(nRc == IBSU_WRN_WET_FINGERS)
			{
				WetFingerDetected = TRUE;
				break;
			}
		}

		if(pApp->m_Callbacks.cbp_fp_onWarning != NULL && WetFingerDetected == TRUE)
		{
			pApp->_LogMessage("cbp_fp_onWarning", "Enter", "Wet finger is detected", 2);
			pApp->m_Callbacks.cbp_fp_onWarning(CBP_FP_WARNING, "Wet finger is detected");
			pApp->_LogMessage("cbp_fp_onWarning", "Exit", "Wet finger is detected", 2);
		}
		
		// Is valid geometry?
		if(IBSU_IsValidFingerGeometryEx(thread_param->handle, pSegmentPositionArray, pSegmentImageArrayCount, fIndex, &IsValidGeo) != IBSU_STATUS_OK)
		{
			IsValidGeo = TRUE;
		}

		if(pApp->m_Callbacks.cbp_fp_onWarning != NULL && IsValidGeo == FALSE)
		{
			pApp->_LogMessage("cbp_fp_onWarning", "Enter", "Wrong slap is detected", 2);
			pApp->m_Callbacks.cbp_fp_onWarning(CBP_FP_WARNING, "Wrong slap is detected");
			pApp->_LogMessage("cbp_fp_onWarning", "Exit", "Wrong slap is detected", 2);
		}

		if(pApp->m_Callbacks.cbp_fp_onCapture != NULL)
		{
			pApp->_LogMessage("cbp_fp_onCapture", "Enter", "", 2);
			pApp->m_Callbacks.cbp_fp_onCapture(thread_param->handle, &grayScaleCapture);
			pApp->_LogMessage("cbp_fp_onCapture", "Exit", "", 2);
		}

		delete [] grayScaleCapture.image;
	}
	else
	{
		//pApp->callback_error(CBP_FP_ERROR_CAPTURE, "cbp_fp_capture");
		// no error 17. just return "blank frame"
		pImage.Width = 1600;
		pImage.Height = 1500;

		cbp_fp_grayScaleCapture grayScaleCapture;
		grayScaleCapture.image = new BYTE [pImage.Width*pImage.Height];
		memset(grayScaleCapture.image, 255, pImage.Width*pImage.Height);

		grayScaleCapture.width = pImage.Width;
		grayScaleCapture.height = pImage.Height;

		grayScaleCapture.slapType = pCBPUsbDev->m_SlapType;
		grayScaleCapture.collectionType = pCBPUsbDev->m_CollectionType;

		if(pApp->m_Callbacks.cbp_fp_onCapture != NULL)
		{
			pApp->_LogMessage("cbp_fp_onCapture", "Enter", "", 2);
			pApp->m_Callbacks.cbp_fp_onCapture(thread_param->handle, &grayScaleCapture);
			pApp->_LogMessage("cbp_fp_onCapture", "Exit", "", 2);
		}

		delete [] grayScaleCapture.image;
	}

done:
	
	// turn off all LEDs
	IBSU_SetLEDs(thread_param->handle, IBSU_LED_NONE);
	
	thread_param->pWnd = NULL;

	return TRUE;
}

UINT CLibCBPApp::_THREAD_cbp_fp_close(LPVOID pParam)
{
	ThreadParam_cbp_fp_close *thread_param = (ThreadParam_cbp_fp_close *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(thread_param->handle)) == NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_INVALID_HANDLE, "cbp_fp_close");
		goto done;
	}

	IBSU_SetLEDs(thread_param->handle, IBSU_LED_NONE);

	if(IBSU_CloseDevice(thread_param->handle) == IBSU_STATUS_OK)
	{
		pCBPUsbDev->m_nDevHandle = -1;
		if(pApp->m_Callbacks.cbp_fp_onClose)
		{
			pApp->_LogMessage("cbp_fp_onClose", "Enter", "", 2);
			pApp->m_Callbacks.cbp_fp_onClose(thread_param->handle);
			pApp->_LogMessage("cbp_fp_onClose", "Exit", "", 2);
		}
	}
	else
	{
		pApp->callback_error(CBP_FP_ERROR_NOT_CLOSED, "cbp_fp_close");
	}

done:
	
	thread_param->pWnd = NULL;

	return TRUE;
}

UINT CLibCBPApp::_THREAD_cbp_fp_configure(LPVOID pParam)
{
	ThreadParam_cbp_fp_configure *thread_param = (ThreadParam_cbp_fp_configure *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(thread_param->handle)) == NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_INVALID_HANDLE, "cbp_fp_configure");
		goto done;
	}

	//pApp->callback_error(CBP_FP_ERROR_NOT_IMPLEMENTED, "cbp_fp_configure");

	if(pApp->m_Callbacks.cbp_fp_onConfigure)
	{
		pApp->_LogMessage("cbp_fp_onConfigure", "Enter", "", 2);
		pApp->m_Callbacks.cbp_fp_onConfigure(thread_param->handle, false);
		pApp->_LogMessage("cbp_fp_onConfigure", "Exit", "", 2);
	}

done:
	
	thread_param->pWnd = NULL;

	return TRUE;
}

UINT CLibCBPApp::_THREAD_cbp_fp_enumDevices(LPVOID pParam)
{
	ThreadParam_cbp_fp_enumDevices *thread_param = (ThreadParam_cbp_fp_enumDevices *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	int device_cnt = 0;
	int nRc = IBSU_STATUS_OK;
	char DeviceCount[MAX_PATH];
	
	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromRequestID(thread_param->pRequestID)) == NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_NOT_INITIALIZED, "cbp_fp_enumDevices");
		goto done;
	}

	nRc = IBSU_GetDeviceCount(&device_cnt);

	if(device_cnt > 64)
		device_cnt = 64;

	if(nRc == IBSU_STATUS_OK)
	{
		if(device_cnt > 0)
		{
			// new try 03
			cbp_fp_device **cbp_fp_devices = (cbp_fp_device **)malloc(sizeof(cbp_fp_device)*device_cnt);

			for(int i=0; i<device_cnt; i++)
			{
				IBSU_DeviceDesc desc;
				nRc = IBSU_GetDeviceDescription(i, &desc);
				if(nRc == IBSU_STATUS_OK)
				{
					//cbp_fp_device *pDevice = (cbp_fp_device *)malloc(sizeof(cbp_fp_device));
					cbp_fp_devices[i] =  (cbp_fp_device *)malloc(sizeof(cbp_fp_device));
					sprintf(cbp_fp_devices[i]->make, STRING_SCANNER_MAKE);
					//sprintf(cbp_fp_devices[i]->model, "Kojak");
					//sprintf(cbp_fp_devices[i]->serialNumber, "1830-00005");
					sprintf(cbp_fp_devices[i]->model, "%s", desc.productName);
					sprintf(cbp_fp_devices[i]->serialNumber, "%s", desc.serialNumber);
				}
				else
				{
					sprintf(cbp_fp_devices[i]->make, STRING_SCANNER_MAKE);
					sprintf(cbp_fp_devices[i]->model, "");
					sprintf(cbp_fp_devices[i]->serialNumber, "");
				}
			}

			if(pApp->m_Callbacks.cbp_fp_onEnumDevices)
			{
				pApp->_LogMessage("cbp_fp_onEnumDevices", "Enter", thread_param->pRequestID, 2);

				sprintf(DeviceCount, "%d", device_cnt);
				pApp->_LogMessage("device_cnt", ">>>", DeviceCount, 2);
				for(int i=0; i<device_cnt; i++)
				{
					sprintf(DeviceCount, "%d) Make(%s), Model(%s), SN(%s)", i,
						cbp_fp_devices[i]->make, cbp_fp_devices[i]->model, cbp_fp_devices[i]->serialNumber);
					pApp->_LogMessage(DeviceCount, ">>>", "", 2);
				}

				pApp->m_Callbacks.cbp_fp_onEnumDevices(cbp_fp_devices, device_cnt, thread_param->pRequestID);
				pApp->_LogMessage("cbp_fp_onEnumDevices", "Exit", "", 2);
			}
			
			for(int i=0; i<device_cnt; i++)
			{
				free(cbp_fp_devices[i]);
			}
			free(cbp_fp_devices);
			
			goto done;
		}
	}

	if(pApp->m_Callbacks.cbp_fp_onEnumDevices)
	{
		pApp->_LogMessage("cbp_fp_onEnumDevices", "Enter", thread_param->pRequestID, 2);
		pApp->m_Callbacks.cbp_fp_onEnumDevices(NULL, 0, thread_param->pRequestID);
		pApp->_LogMessage("cbp_fp_onEnumDevices", "Exit", "", 2);
	}

done:
	
	thread_param->pWnd = NULL;

	return TRUE;
}

UINT CLibCBPApp::_THREAD_cbp_fp_getDirtiness(LPVOID pParam)
{
	ThreadParam_cbp_fp_getDirtiness *thread_param = (ThreadParam_cbp_fp_getDirtiness *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(thread_param->handle)) == NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_INVALID_HANDLE, "cbp_fp_getDirtiness");
		goto done;
	}

	//pApp->callback_error(CBP_FP_ERROR_NOT_IMPLEMENTED, "cbp_fp_getDirtiness");
	
	if(pApp->m_Callbacks.cbp_fp_onGetDirtiness)
	{
		pApp->_LogMessage("cbp_fp_onGetDirtiness", "Enter", "", 2);
		pApp->m_Callbacks.cbp_fp_onGetDirtiness(thread_param->handle, 0); // spotless
		pApp->_LogMessage("cbp_fp_onGetDirtiness", "Exit", "", 2);
	}

done:
	
	thread_param->pWnd = NULL;

	return TRUE;
}

UINT CLibCBPApp::_THREAD_cbp_fp_getLockInfo(LPVOID pParam)
{
	ThreadParam_cbp_fp_getLockInfo *thread_param = (ThreadParam_cbp_fp_getLockInfo *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(thread_param->handle)) == NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_INVALID_HANDLE, "cbp_fp_getLockInfo");
		goto done;
	}

	CBPLockInfo lockinfo;
	int nRc = pApp->_FindFromArr_LockInfo(thread_param->handle, (long)GetCurrentProcessId(), &lockinfo);

	if( nRc == CBP_FP_OK )
	{
		// calculate lock duration
		lockinfo.m_LockInfo.lockDuration = (long)clock() - lockinfo.m_LockCreateTime;
	}
	else
	{
		//pApp->callback_error(CBP_FP_ERROR_NOT_LOCKED, "cbp_fp_getLockInfo");
		// calculate lock duration
		lockinfo.m_LockInfo.handle = -1;
		lockinfo.m_LockInfo.lockDuration = -1;
		lockinfo.m_LockInfo.lockPID = -1;
	}
		
	if(pApp->m_Callbacks.cbp_fp_onGetLockInfo)
	{
		char strLockInfo[MAX_PATH];
		
		pApp->_LogMessage("cbp_fp_onGetLockInfo", "Enter", "", 2);
		
		sprintf(strLockInfo, "handle:%d, Duration:%d, PID:%d", 
			lockinfo.m_LockInfo.handle, lockinfo.m_LockInfo.lockDuration, lockinfo.m_LockInfo.lockPID);
		pApp->_LogMessage(strLockInfo, ">>>", "", 2);
		pApp->m_Callbacks.cbp_fp_onGetLockInfo(thread_param->handle, &lockinfo.m_LockInfo);
		pApp->_LogMessage("cbp_fp_onGetLockInfo", "Exit", "", 2);
	}

done:
	
	/*thread_param->ThreadCount--;
	if(thread_param->ThreadCount == 0)
	{
		thread_param->pWnd = NULL;
	}*/

	return TRUE;
}

//static UINT _THREAD_cbp_fp_getProerty(LPVOID pParam);

//static UINT _THREAD_cbp_fp_setProperty(LPVOID pParam);
/*
UINT CLibCBPApp::_THREAD_cbp_fp_initialize(LPVOID pParam)
{
	ThreadParam_cbp_fp_initialize *thread_param = (ThreadParam_cbp_fp_initialize *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	CBPUsbDevice *pCBPUsbDev = NULL;

	// check requestID is duplicated.
	if((pCBPUsbDev = pApp->_GetUsbDevicesFromRequestID(thread_param->pRequestID)) != NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_ALREADY_INITIALIZED, "cbp_fp_initialize");
		goto done;
	}

	pCBPUsbDev = new CBPUsbDevice;

	// register callbacks of IBScanUltimate SDK
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, pApp->OnEvent_DeviceCount, pApp );
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, pApp->OnEvent_InitProgress, pApp );
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE, pApp->OnEvent_AsyncOpenDevice, pApp );

	// initialize vaiables
	pCBPUsbDev->m_bInitialized = TRUE;
	pCBPUsbDev->m_bLocked = FALSE;
	pCBPUsbDev->m_bReceivedResultImage = FALSE;
	memset(&pCBPUsbDev->m_FingerQualityState, ENUM_IBSU_FINGER_NOT_PRESENT, sizeof(IBSU_FingerQualityState)*IBSU_MAX_SEGMENT_COUNT);

	pCBPUsbDev->m_nDevHandle = -1;
	pCBPUsbDev->m_nFingerCount = 0;
	pCBPUsbDev->m_SlapType = slap_unknown;
	pCBPUsbDev->m_CollectionType = collection_unknown;
	sprintf(pCBPUsbDev->m_sRequestID, "%s", thread_param->pRequestID);

	// add request to list
	pApp->m_CBPUsbDevices.push_back(pCBPUsbDev);

	if(pApp->m_Callbacks.cbp_fp_onInitialize)
	{
		pApp->_LogMessage("cbp_fp_onInitialize", "Enter", pCBPUsbDev->m_sRequestID, 2);
		//pApp->m_bisNotInitializeYet = TRUE;
		pApp->m_Callbacks.cbp_fp_onInitialize(pCBPUsbDev->m_sRequestID);
		//pApp->m_bisNotInitializeYet = FALSE;
		pApp->_LogMessage("cbp_fp_onInitialize", "Exit", "", 2);
	}

done:

	thread_param->pWnd = NULL;
	
	return TRUE;
}*/

UINT CLibCBPApp::_THREAD_cbp_fp_initialize(LPVOID pParam)
{
	ThreadParam_cbp_fp_initialize *thread_param = (ThreadParam_cbp_fp_initialize *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	CBPUsbDevice *pCBPUsbDev = NULL;
	BOOL isNew = TRUE;

	// check requestID is duplicated.
	if((pCBPUsbDev = pApp->_GetUsbDevicesFromRequestID(thread_param->pRequestID)) != NULL)
	{
		//pApp->callback_error(CBP_FP_ERROR_ALREADY_INITIALIZED, "cbp_fp_initialize");
		pApp->m_Callbacks.cbp_fp_onWarning(CBP_FP_WRN_ALREADY_INITIALIZED, "cbp_fp_initialize");
		isNew = FALSE;
		//goto done;

		// register callbacks of IBScanUltimate SDK
		IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, pApp->OnEvent_DeviceCount, pApp );
		IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, pApp->OnEvent_InitProgress, pApp );
		IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE, pApp->OnEvent_AsyncOpenDevice, pApp );

		// initialize vaiables
		pCBPUsbDev->m_bInitialized = TRUE;

		if(pCBPUsbDev->m_nDevHandle != -1)
		{
			// clean up
			IBSU_CancelCaptureImage(pCBPUsbDev->m_nDevHandle);
			RESERVED_CleanUpCBP(pCBPUsbDev->m_nDevHandle);

			// delete lock
			pApp->_DeleteFromArr_LockInfo(pCBPUsbDev->m_nDevHandle, (long)GetCurrentProcessId());
			pCBPUsbDev->m_bLocked = FALSE;
			pCBPUsbDev->m_bReceivedResultImage = FALSE;
			memset(&pCBPUsbDev->m_FingerQualityState, ENUM_IBSU_FINGER_NOT_PRESENT, sizeof(IBSU_FingerQualityState)*IBSU_MAX_SEGMENT_COUNT);

			IBSU_CloseDevice(pCBPUsbDev->m_nDevHandle);

			pCBPUsbDev->m_nDevHandle = -1;
			pCBPUsbDev->m_nFingerCount = 0;
			pCBPUsbDev->m_SlapType = slap_unknown;
			pCBPUsbDev->m_CollectionType = collection_unknown;
			sprintf(pCBPUsbDev->m_sRequestID, "%s", thread_param->pRequestID);
		}
	}
	else
	{
		pCBPUsbDev = new CBPUsbDevice;

		// register callbacks of IBScanUltimate SDK
		IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, pApp->OnEvent_DeviceCount, pApp );
		IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, pApp->OnEvent_InitProgress, pApp );
		IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE, pApp->OnEvent_AsyncOpenDevice, pApp );

		// initialize vaiables
		pCBPUsbDev->m_bInitialized = TRUE;
		pCBPUsbDev->m_bLocked = FALSE;
		pCBPUsbDev->m_bReceivedResultImage = FALSE;
		memset(&pCBPUsbDev->m_FingerQualityState, ENUM_IBSU_FINGER_NOT_PRESENT, sizeof(IBSU_FingerQualityState)*IBSU_MAX_SEGMENT_COUNT);

		pCBPUsbDev->m_nDevHandle = -1;
		pCBPUsbDev->m_nFingerCount = 0;
		pCBPUsbDev->m_SlapType = slap_unknown;
		pCBPUsbDev->m_CollectionType = collection_unknown;
		sprintf(pCBPUsbDev->m_sRequestID, "%s", thread_param->pRequestID);

		// add request to list
		pApp->m_CBPUsbDevices.push_back(pCBPUsbDev);
	}

	if(pApp->m_Callbacks.cbp_fp_onInitialize)
	{
		pApp->_LogMessage("cbp_fp_onInitialize", "Enter", pCBPUsbDev->m_sRequestID, 2);
		//pApp->m_bisNotInitializeYet = TRUE;
		pApp->m_Callbacks.cbp_fp_onInitialize(pCBPUsbDev->m_sRequestID);
		//pApp->m_bisNotInitializeYet = FALSE;
		pApp->_LogMessage("cbp_fp_onInitialize", "Exit", "", 2);
	}

	thread_param->pWnd = NULL;
	
	return TRUE;
}

UINT CLibCBPApp::_THREAD_cbp_fp_lock(LPVOID pParam)
{
	ThreadParam_cbp_fp_lock *thread_param = (ThreadParam_cbp_fp_lock *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(thread_param->handle)) == NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_INVALID_HANDLE, "cbp_fp_lock");
		goto done;
	}

	int pid = GetCurrentProcessId();
	int nRc = pApp->_AddToArr_LockInfo(thread_param->handle, pid);
	
	if(nRc != CBP_FP_OK)
	{
		pApp->callback_error(nRc, "cbp_fp_lock");
		goto done;
	}
	
	if(pApp->m_Callbacks.cbp_fp_onLock)
	{
		pApp->_LogMessage("cbp_fp_onLock", "Enter", "", 2);
		pApp->m_Callbacks.cbp_fp_onLock(thread_param->handle, pid);
		pApp->_LogMessage("cbp_fp_onLock", "Exit", "", 2);
	}

done:
	
	thread_param->pWnd = NULL;

	return TRUE;
}

UINT CLibCBPApp::_THREAD_cbp_fp_open(LPVOID pParam)
{
	ThreadParam_cbp_fp_open *thread_param = (ThreadParam_cbp_fp_open *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	int device_cnt = 0;
	int dev_handle = -1;
	int not_found = 0;
	int nRc = IBSU_STATUS_OK;
	
	CBPUsbDevice *pCBPUsbDev = NULL;

	// empty device
	if(pApp->m_CBPUsbDevices.size() == 0)
	{
		pApp->callback_error(CBP_FP_ERROR_NOT_INITIALIZED, "cbp_fp_open");
		goto done;
	}

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromNotOpened()) == NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_BUSY, "cbp_fp_open");
		goto done;
	}

	nRc = IBSU_GetDeviceCount(&device_cnt);

	if(nRc == IBSU_STATUS_OK && device_cnt > 0)
	{
		cbp_fp_scanner_connection scanner_connection;
		not_found = 0;
	
		IBSU_SdkVersion sdkver;
		IBSU_GetSDKVersion(&sdkver);

		for(int i=0; i<device_cnt; i++)
		{
			BOOL try_open;
			IBSU_DeviceDesc desc;
			nRc = IBSU_GetDeviceDescription(i, &desc);

			if(nRc == IBSU_STATUS_OK)
			{
				try_open = FALSE;
				// open in multiple ways ( case 3 : model is defined and serial is defined )
				if( //strcmp(thread_param->device->make, __SCANNER_MAKE__) == 0 &&
					strcmp(thread_param->device->model, desc.productName) == 0 &&
					strcmp(thread_param->device->serialNumber, desc.serialNumber) == 0 )
				{
					try_open = TRUE;
				}
				// open in multiple ways ( case 2 : model is defined )
				else if( //strcmp(thread_param->make, __SCANNER_MAKE__) == 0 &&
						 strcmp(thread_param->device->model, desc.productName) == 0 &&
						 strcmp(thread_param->device->serialNumber, "") == 0)
				{
					try_open = TRUE;
				}
				// open in multiple ways ( case 1 : NULL )
				else if( strcmp(thread_param->device->make, "") == 0 &&
						 strcmp(thread_param->device->model, "") == 0 &&
						 strcmp(thread_param->device->serialNumber, "") == 0) 
				{
					try_open = TRUE;
				}

				if(try_open == TRUE)
				{
					nRc = IBSU_OpenDevice(i, &dev_handle);
					if(nRc == IBSU_STATUS_OK)
					{
						pCBPUsbDev->m_nDevHandle = dev_handle;
						IBSU_RegisterCallbacks( dev_handle, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, pApp->OnEvent_DeviceCommunicationBreak, pApp );    
						IBSU_RegisterCallbacks( dev_handle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, pApp->OnEvent_PreviewImage, pApp );
						IBSU_RegisterCallbacks( dev_handle, ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, pApp->OnEvent_TakingAcquisition, pApp );  
						IBSU_RegisterCallbacks( dev_handle, ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, pApp->OnEvent_CompleteAcquisition, pApp );  
						IBSU_RegisterCallbacks( dev_handle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX, pApp->OnEvent_ResultImageEx, pApp );  
						IBSU_RegisterCallbacks( dev_handle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, pApp->OnEvent_FingerCount, pApp );  
						IBSU_RegisterCallbacks( dev_handle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY, pApp->OnEvent_FingerQuality, pApp );  
						//if( m_chkUseClearPlaten )
							IBSU_RegisterCallbacks( dev_handle, ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE, OnEvent_ClearPlatenAtCapture, pApp );
						IBSU_RegisterCallbacks( dev_handle, ENUM_IBSU_OPTIONAL_EVENT_NOTIFY_MESSAGE, pApp->OnEvent_NotifyMessage, pApp );  
						IBSU_RegisterCallbacks( dev_handle, ENUM_IBSU_ESSENTIAL_EVENT_KEYBUTTON, pApp->OnEvent_PressedKeyButtons, pApp );
						IBSU_SetClientDisplayProperty(dev_handle, ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA, "TRUE");

						IBSU_SetLEDs(dev_handle, IBSU_LED_NONE);

						scanner_connection.device = new cbp_fp_device;
						sprintf(scanner_connection.device->make, STRING_SCANNER_MAKE);
						sprintf(scanner_connection.device->model, "%s", desc.productName);
						sprintf(scanner_connection.device->serialNumber, "%s", desc.serialNumber);

						for(int i=0; i<CBP_FP_MAX_PROPERTY_MAX; i++)
							scanner_connection.propertyList[i] = new cbp_fp_property;
						scanner_connection.propertyListLen = 0;

						// add properties
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_SUPPORTED_COLLECTION_TYPES);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, "flat,rolled");
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_SUPPORTED_SLAPS);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, "rightHand,leftHand,rightThumb,leftThumb");
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_MAKE);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, STRING_SCANNER_MAKE);
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_MODEL);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, "%s", desc.productName);
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_SERIAL);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, "%s", desc.serialNumber);
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_SOFTWARE_VERSION);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, "%s", sdkver.Product);
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_FIRMWARE_VERSION);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, "%s", desc.fwVersion);
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_PREVIEW_PIXEL_HEIGHT);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, "1500");
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_PREVIEW_PIXEL_WIDTH);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, "1600");
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_CAPTURE_PIXEL_HEIGHT);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, "1500");
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_CAPTURE_PIXEL_WIDTH);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, "1600");
						scanner_connection.propertyListLen++;
						
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_PREVIEW_DPI);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, "500");
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_CAPTURE_DPI);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, "500");
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_DEBUG_LEVEL);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, "%d", pApp->m_DebugLevel);
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_PROPERTY_DEBUG_FILE_NAME);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, "%s", pApp->m_DebugFileName);
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_ANALYSIS_HAS_SEGMENTATION);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, CBP_FP_TRUE);
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_ANALYSIS_HAS_CENTROID);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, CBP_FP_TRUE);
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_ANALYSIS_HAS_SPOOF);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, CBP_FP_FALSE);
						scanner_connection.propertyListLen++;

						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->name, CBP_FP_ANALYSIS_HAS_PREVIEW);
						sprintf(scanner_connection.propertyList[scanner_connection.propertyListLen]->value, CBP_FP_TRUE);
						scanner_connection.propertyListLen++;
						
						if(pApp->m_Callbacks.cbp_fp_onOpen)
						{
							pApp->_LogMessage("cbp_fp_onOpen", "Enter", "", 2);
							pApp->m_Callbacks.cbp_fp_onOpen(dev_handle, &scanner_connection);
							pApp->_LogMessage("cbp_fp_onOpen", "Exit", "", 2);
						}

						delete scanner_connection.device;
						for(int i=0; i<CBP_FP_MAX_PROPERTY_MAX; i++)
							delete scanner_connection.propertyList[i];
						not_found++;
						break;
					}
				}
			}
		}

		if(not_found == 0)
		{
			pApp->callback_error(CBP_FP_ERROR_SCANNER_NOT_FOUND, "cbp_fp_open");
		}
	}
	else
	{
		pApp->callback_error(CBP_FP_ERROR_SCANNER_NOT_FOUND, "cbp_fp_open");
	}

done:
	
	thread_param->pWnd = NULL;

	return TRUE;
}

UINT CLibCBPApp::_THREAD_cbp_fp_powerSave(LPVOID pParam)
{
	ThreadParam_cbp_fp_powerSave *thread_param = (ThreadParam_cbp_fp_powerSave *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(thread_param->handle)) == NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_INVALID_HANDLE, "cbp_fp_powerSave");
		goto done;
	}

	//pApp->callback_error(CBP_FP_ERROR_NOT_IMPLEMENTED, "cbp_fp_powerSave");
	
	if(pApp->m_Callbacks.cbp_fp_onPowerSave)
	{
		pApp->_LogMessage("cbp_fp_onPowerSave", "Enter", "", 2);
		pApp->m_Callbacks.cbp_fp_onPowerSave(thread_param->handle, thread_param->powerSaveOn);
		pApp->_LogMessage("cbp_fp_onPowerSave", "Exit", "", 2);
	}

done:
	
	thread_param->pWnd = NULL;

	return TRUE;
}

//static UINT _THREAD_cbp_fp_registerCallBacks(LPVOID pParam);
UINT CLibCBPApp::_THREAD_cbp_fp_startImaging(LPVOID pParam)
{
	ThreadParam_cbp_fp_startImaging *thread_param = (ThreadParam_cbp_fp_startImaging *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	IBSU_ImageType       imageType;
	IBSU_ImageResolution imageResolution = ENUM_IBSU_IMAGE_RESOLUTION_500;
    DWORD                captureOptions = IBSU_OPTION_AUTO_CONTRAST;// | IBSU_OPTION_AUTO_CAPTURE | IBSU_OPTION_IGNORE_FINGER_COUNT;
	int nRc = IBSU_STATUS_OK;
	DWORD LedType = IBSU_LED_NONE;

	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(thread_param->handle)) == NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_INVALID_HANDLE, "cbp_fp_startImaging");
		goto done;
	}

	switch(thread_param->slapType)
	{
	case slap_rightHand:
	case slap_leftHand:
		imageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
		break;
	case slap_twoFingers:
	case slap_twoThumbs:
	case slap_twotThumbs:
		imageType = ENUM_IBSU_FLAT_TWO_FINGERS;
		break;
	case slap_rightThumb:
	case slap_rightIndex:
	case slap_rightMiddle:
	case slap_rightRing:
	case slap_rightLittle:
	case slap_leftThumb:
	case slap_leftIndex:
	case slap_leftMiddle:
	case slap_leftRing:
	case slap_leftLittle:
		if(thread_param->collectionType == collection_rolled)
		{
			imageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			captureOptions = IBSU_OPTION_AUTO_CONTRAST | IBSU_OPTION_AUTO_CAPTURE;
			LedType |= IBSU_LED_F_PROGRESS_ROLL;
		}
		else
			imageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
		break;
	case slap_stitchedLeftThumb:
	case slap_stitchedRightThumb:
	case slap_unknown:
	default:
		pApp->callback_error(CBP_FP_ERROR_UNSUPPORTED_SLAP, "cbp_fp_startImaging");
		goto done;
	}

	switch(thread_param->collectionType)
	{
	case collection_contactless:
	case collection_unknown:
		pApp->callback_error(CBP_FP_ERROR_UNSUPPORTED_COLLECTION_TYPE, "cbp_fp_startImaging");
		goto done;
	}

	if( (imageType == ENUM_IBSU_FLAT_FOUR_FINGERS ||
		imageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
		imageType == ENUM_IBSU_FLAT_SINGLE_FINGER) &&
		thread_param->collectionType == collection_rolled )
	{
		pApp->callback_error(CBP_FP_ERROR_INVALID_PARAM, "cbp_fp_startImaging");
		goto done;
	}
	else if( imageType == ENUM_IBSU_ROLL_SINGLE_FINGER &&
			 thread_param->collectionType == collection_flat )
	{
		pApp->callback_error(CBP_FP_ERROR_INVALID_PARAM, "cbp_fp_startImaging");
		goto done;
	}

	BOOL IsActive;
	nRc = IBSU_IsCaptureActive(thread_param->handle, &IsActive);
	if( nRc == IBSU_STATUS_OK && IsActive )
	{
		pApp->callback_error(CBP_FP_ERROR_ALREADY_CAPTURING, "cbp_fp_startImaging");
		goto done;
	}

	/*nRc = IBSU_SetProperty(handle, ENUM_IBSU_PROPERTY_ENABLE_DECIMATION, "FALSE");
	if(nRc != IBSU_STATUS_OK)
	{
		callback_error(CBP_FP_ERROR_IMAGING);
		return;
	}*/

	nRc = IBSU_SetProperty(thread_param->handle, ENUM_IBSU_PROPERTY_ENABLE_WET_FINGER_DETECT, "TRUE");
	if(nRc != IBSU_STATUS_OK)
	{
		pApp->callback_error(CBP_FP_ERROR_IMAGING, "cbp_fp_startImaging");
		goto done;
	}

	nRc = IBSU_BeginCaptureImage(thread_param->handle, imageType, imageResolution, captureOptions);
	if(nRc != IBSU_STATUS_OK)
	{
		pApp->callback_error(CBP_FP_ERROR_IMAGING, "cbp_fp_startImaging");
		goto done;
	}

	pCBPUsbDev->m_StopPreview = FALSE;

	pCBPUsbDev->m_bReceivedResultImage = FALSE;
	pCBPUsbDev->m_SlapType = thread_param->slapType;
	pCBPUsbDev->m_CollectionType = thread_param->collectionType;

	pApp->m_IsValidGeoCnt = 0;

	switch(thread_param->slapType)
	{
	case slap_rightHand:
		LedType |=  IBSU_LED_F_RIGHT_INDEX_RED |// IBSU_LED_F_RIGHT_INDEX_GREEN | 
					IBSU_LED_F_RIGHT_MIDDLE_RED |// IBSU_LED_F_RIGHT_MIDDLE_GREEN | 
					IBSU_LED_F_RIGHT_RING_RED |// IBSU_LED_F_RIGHT_RING_GREEN | 
					IBSU_LED_F_RIGHT_LITTLE_RED |// IBSU_LED_F_RIGHT_LITTLE_GREEN | 
					IBSU_LED_F_PROGRESS_RIGHT_HAND;
		break;
	case slap_leftHand:
		LedType |=  IBSU_LED_F_LEFT_INDEX_RED |// IBSU_LED_F_LEFT_INDEX_GREEN | 
					IBSU_LED_F_LEFT_MIDDLE_RED |// IBSU_LED_F_LEFT_MIDDLE_GREEN | 
					IBSU_LED_F_LEFT_RING_RED |// IBSU_LED_F_LEFT_RING_GREEN | 
					IBSU_LED_F_LEFT_LITTLE_RED |// IBSU_LED_F_LEFT_LITTLE_GREEN | 
					IBSU_LED_F_PROGRESS_LEFT_HAND;
		break;
	case slap_twoFingers:
		break;
	case slap_twoThumbs:
	case slap_twotThumbs:
		LedType |= IBSU_LED_F_LEFT_THUMB_RED | /*IBSU_LED_F_LEFT_THUMB_GREEN | */
					IBSU_LED_F_RIGHT_THUMB_RED | /*IBSU_LED_F_RIGHT_THUMB_GREEN | */IBSU_LED_F_PROGRESS_TWO_THUMB;
		break;
	case slap_rightThumb:
		LedType |= IBSU_LED_F_RIGHT_THUMB_RED | /*IBSU_LED_F_RIGHT_THUMB_GREEN | */IBSU_LED_F_PROGRESS_TWO_THUMB;
		break;
	case slap_rightIndex:
		LedType |= IBSU_LED_F_RIGHT_INDEX_RED | /*IBSU_LED_F_RIGHT_INDEX_GREEN | */IBSU_LED_F_PROGRESS_RIGHT_HAND;
		break;
	case slap_rightMiddle:
		LedType |= IBSU_LED_F_RIGHT_MIDDLE_RED | /*IBSU_LED_F_RIGHT_MIDDLE_GREEN | */IBSU_LED_F_PROGRESS_RIGHT_HAND;
		break;
	case slap_rightRing:
		LedType |= IBSU_LED_F_RIGHT_RING_RED | /*IBSU_LED_F_RIGHT_RING_GREEN | */IBSU_LED_F_PROGRESS_RIGHT_HAND;
		break;
	case slap_rightLittle:
		LedType |= IBSU_LED_F_RIGHT_LITTLE_RED | /*IBSU_LED_F_RIGHT_LITTLE_GREEN | */IBSU_LED_F_PROGRESS_RIGHT_HAND;
		break;
	case slap_leftThumb:
		LedType |= IBSU_LED_F_LEFT_THUMB_RED | /*IBSU_LED_F_LEFT_THUMB_GREEN | */IBSU_LED_F_PROGRESS_TWO_THUMB;
		break;
	case slap_leftIndex:
		LedType |= IBSU_LED_F_LEFT_INDEX_RED | /*IBSU_LED_F_LEFT_INDEX_GREEN | */IBSU_LED_F_PROGRESS_LEFT_HAND;
		break;
	case slap_leftMiddle:
		LedType |= IBSU_LED_F_LEFT_MIDDLE_RED | /*IBSU_LED_F_LEFT_MIDDLE_GREEN | */IBSU_LED_F_PROGRESS_LEFT_HAND;
		break;
	case slap_leftRing:
		LedType |= IBSU_LED_F_LEFT_RING_RED | /*IBSU_LED_F_LEFT_RING_GREEN | */IBSU_LED_F_PROGRESS_LEFT_HAND;
		break;
	case slap_leftLittle:
		LedType |= IBSU_LED_F_LEFT_LITTLE_RED | /*IBSU_LED_F_LEFT_LITTLE_GREEN | */IBSU_LED_F_PROGRESS_LEFT_HAND;
		break;
	default:
		goto done;
	}

	IBSU_SetLEDs(thread_param->handle, LedType);

done:
	
	thread_param->pWnd = NULL;

	return TRUE;
}

UINT CLibCBPApp::_THREAD_cbp_fp_stopImaging(LPVOID pParam)
{
	ThreadParam_cbp_fp_stopImaging *thread_param = (ThreadParam_cbp_fp_stopImaging *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	BOOL IsActive;
	int nRc = IBSU_STATUS_OK;
	DWORD LedType = IBSU_LED_NONE;
	DWORD fIndex=0;

	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(thread_param->handle)) == NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_INVALID_HANDLE, "cbp_fp_stopImaging");
		goto done;
	}

	nRc = IBSU_IsCaptureActive(thread_param->handle, &IsActive);
	if( nRc == IBSU_STATUS_OK && !IsActive )
	{
		pApp->callback_error(CBP_FP_ERROR_NOT_CAPTURING, "cbp_fp_stopImaging");
		goto done;
	}

	pCBPUsbDev->m_StopPreview = TRUE;

	nRc = IBSU_TakeResultImageManually(thread_param->handle);
	
	if( nRc == IBSU_STATUS_OK )
	{
		// LED green
		switch(pCBPUsbDev->m_SlapType)
		{
		case slap_rightHand:
			LedType |=  IBSU_LED_F_RIGHT_INDEX_GREEN | 
						IBSU_LED_F_RIGHT_MIDDLE_GREEN | 
						IBSU_LED_F_RIGHT_RING_GREEN | 
						IBSU_LED_F_RIGHT_LITTLE_GREEN | 
						IBSU_LED_F_PROGRESS_RIGHT_HAND;
			fIndex = IBSU_FINGER_RIGHT_HAND;
			break;
		case slap_leftHand:
			LedType |=  IBSU_LED_F_LEFT_INDEX_GREEN | 
						IBSU_LED_F_LEFT_MIDDLE_GREEN | 
						IBSU_LED_F_LEFT_RING_GREEN | 
						IBSU_LED_F_LEFT_LITTLE_GREEN | 
						IBSU_LED_F_PROGRESS_LEFT_HAND;
			fIndex = IBSU_FINGER_LEFT_HAND;
			break;
		case slap_twoFingers:
			break;
		case slap_twoThumbs:
		case slap_twotThumbs:
			LedType |= IBSU_LED_F_LEFT_THUMB_GREEN | IBSU_LED_F_RIGHT_THUMB_GREEN | IBSU_LED_F_PROGRESS_TWO_THUMB;
			break;
		case slap_rightThumb:
			LedType |= IBSU_LED_F_RIGHT_THUMB_GREEN | IBSU_LED_F_PROGRESS_TWO_THUMB;
			break;
		case slap_rightIndex:
			LedType |= IBSU_LED_F_RIGHT_INDEX_GREEN | IBSU_LED_F_PROGRESS_RIGHT_HAND;
			break;
		case slap_rightMiddle:
			LedType |= IBSU_LED_F_RIGHT_MIDDLE_GREEN | IBSU_LED_F_PROGRESS_RIGHT_HAND;
			break;
		case slap_rightRing:
			LedType |= IBSU_LED_F_RIGHT_RING_GREEN | IBSU_LED_F_PROGRESS_RIGHT_HAND;
			break;
		case slap_rightLittle:
			LedType |= IBSU_LED_F_RIGHT_LITTLE_GREEN | IBSU_LED_F_PROGRESS_RIGHT_HAND;
			break;
		case slap_leftThumb:
			LedType |= IBSU_LED_F_LEFT_THUMB_GREEN | IBSU_LED_F_PROGRESS_TWO_THUMB;
			break;
		case slap_leftIndex:
			LedType |= IBSU_LED_F_LEFT_INDEX_GREEN | IBSU_LED_F_PROGRESS_LEFT_HAND;
			break;
		case slap_leftMiddle:
			LedType |= IBSU_LED_F_LEFT_MIDDLE_GREEN | IBSU_LED_F_PROGRESS_LEFT_HAND;
			break;
		case slap_leftRing:
			LedType |= IBSU_LED_F_LEFT_RING_GREEN | IBSU_LED_F_PROGRESS_LEFT_HAND;
			break;
		case slap_leftLittle:
			LedType |= IBSU_LED_F_LEFT_LITTLE_GREEN | IBSU_LED_F_PROGRESS_LEFT_HAND;
			break;
		}
		// change LEDs as capture finished.
		IBSU_SetLEDs(thread_param->handle, LedType);

		// wait maximum 3.0 seconds
		//for(int i=0; i<30; i++)

		// optimize onStopImaging()
		// wait maximum 2.0 seconds
		for(int i=0; i<20; i++)
		{
			if(pCBPUsbDev->m_bReceivedResultImage == TRUE)
			{
				break;
			}

			Sleep(100);
		}
	}
	
	if(pCBPUsbDev->m_bReceivedResultImage == FALSE)
	{
		pApp->_LogMessage("StopPreview - Failed to received Final image", ">>>", "", 2);
		IBSU_CancelCaptureImage(thread_param->handle);
	}
	else
	{
		pApp->_LogMessage("StopPreview - Received Final image", ">>>", "", 2);
	}

	IBSU_SetLEDs(thread_param->handle, IBSU_LED_NONE);

	if(pApp->m_Callbacks.cbp_fp_onStopPreview != NULL)
	{
		pApp->_LogMessage("cbp_fp_onStopPreview", "Enter", "", 2);
		pApp->m_Callbacks.cbp_fp_onStopPreview(thread_param->handle);
		pApp->_LogMessage("cbp_fp_onStopPreview", "Exit", "", 2);
	}

done:
	
	thread_param->pWnd = NULL;

	return TRUE;
}

UINT CLibCBPApp::_THREAD_cbp_fp_uninitialize(LPVOID pParam)
{
	ThreadParam_cbp_fp_uninitialize *thread_param = (ThreadParam_cbp_fp_uninitialize *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromRequestID(thread_param->pRequestID)) == NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_NOT_INITIALIZED, "cbp_fp_uninitialize");
		goto done;
	}

	if(IBSU_ReleaseCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT ) != IBSU_STATUS_OK)
	{
		pApp->callback_error(CBP_FP_ERROR_UNITIALIZE, "cbp_fp_uninitialize");
		goto done;
	}

	if(IBSU_ReleaseCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS ) != IBSU_STATUS_OK)
	{
		pApp->callback_error(CBP_FP_ERROR_UNITIALIZE, "cbp_fp_uninitialize");
		goto done;
	}

	if(IBSU_ReleaseCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE ) != IBSU_STATUS_OK)
	{
		pApp->callback_error(CBP_FP_ERROR_UNITIALIZE, "cbp_fp_uninitialize");
		goto done;
	}

	if(pCBPUsbDev->m_nDevHandle != -1)
	{
		IBSU_CloseDevice(pCBPUsbDev->m_nDevHandle);
		pCBPUsbDev->m_nDevHandle = -1;
	}

	pCBPUsbDev->m_bInitialized = FALSE;

	pApp->_DeleteUsbDeviceFromRequestID(thread_param->pRequestID);
	
	if(pApp->m_Callbacks.cbp_fp_onUninitialize)
	{
		pApp->_LogMessage("cbp_fp_onUninitialize", "Enter", thread_param->pRequestID, 2);
		pApp->m_Callbacks.cbp_fp_onUninitialize(thread_param->pRequestID);
		pApp->_LogMessage("cbp_fp_onUninitialize", "Exit", "", 2);
	}

done:
	
	thread_param->pWnd = NULL;

	return TRUE;
}

UINT CLibCBPApp::_THREAD_cbp_fp_unlock(LPVOID pParam)
{
	ThreadParam_cbp_fp_unlock *thread_param = (ThreadParam_cbp_fp_unlock *)pParam;
	CLibCBPApp *pApp = (CLibCBPApp *)thread_param->pWnd;

	CBPUsbDevice *pCBPUsbDev = NULL;

	if((pCBPUsbDev = pApp->_GetUsbDevicesFromHandle(thread_param->handle)) == NULL)
	{
		pApp->callback_error(CBP_FP_ERROR_INVALID_HANDLE, "cbp_fp_unlock");
		goto done;
	}

	/*int                     pImageStatus;
	IBSU_ImageData          pImage;
	IBSU_ImageType          pImageType;
	int                     pDetectedFingerCount;
	IBSU_ImageData          pSegmentImageArray[IBSU_MAX_SEGMENT_COUNT];
	IBSU_SegmentPosition    pSegmentPositionArray[IBSU_MAX_SEGMENT_COUNT];
	int                     pSegmentImageArrayCount;
	IBSU_FingerCountState   pFingerCountState;
	IBSU_FingerQualityState pQualityArray[IBSU_MAX_SEGMENT_COUNT];
	int                     pQualityArrayCount;*/
	
	// clean up
	IBSU_CancelCaptureImage(thread_param->handle);
	RESERVED_CleanUpCBP(thread_param->handle);

	// delete lock
	int nRc = pApp->_DeleteFromArr_LockInfo(thread_param->handle, (long)GetCurrentProcessId());

	if(nRc != CBP_FP_OK)
	{
		pApp->callback_error(CBP_FP_ERROR_UNLOCK, "cbp_fp_unlock");
		goto done;
	}

	if(pApp->m_Callbacks.cbp_fp_onUnLock)
	{
		pApp->_LogMessage("cbp_fp_onUnLock", "Enter", "", 2);
		pApp->m_Callbacks.cbp_fp_onUnLock(thread_param->handle);
		pApp->_LogMessage("cbp_fp_onUnLock", "Exit", "", 2);
	}

done:
	
	thread_param->pWnd = NULL;

	return TRUE;
}
