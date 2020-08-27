/////////////////////////////////////////////////////////////////////////////
/*
DESCRIPTION:
	CMainCapture - Class for getting image capture core from Live Scanner
	http://www.integratedbiometrics.com/

NOTES:
	Copyright(C) Integrated Biometrics, 2011

VERSION HISTORY:
	19 April 2011 - First initialize
*/
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainCapture.h"
// Only build this file when running on top of Windows USB driver
// (WINCE does not use this driver)

#include "../IBScanUltimateDLL.h"
#include <math.h>
#ifdef WINCE
#include "WinCEMath.h"
#include <winsock.h>   // For timeval
#endif

////////////////////////////////////////////////////////////////////////////////
// Implementation

// gettimeofday in windows
#ifndef WINCE
int CMainCapture::gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    static int tzflag;

    if (NULL != tv)
    {
        // Get system time
        GetSystemTimeAsFileTime(&ft);

        // Make a unsigned 64 bit
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        // Change to epoch time
        tmpres -= DELTA_EPOCH_IN_MICROSECS;

        // Change from 100nano to 1micro
        tmpres /= 10;

        // Change to sec and micorsec
        tv->tv_sec = (LONG)(tmpres / 1000000UL);
        tv->tv_usec = (tmpres % 1000000UL);
    }

    // Process to timezone
    if (NULL != tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }

    return 0;
}
#endif

UINT CMainCapture::_InitializeThreadCallback(LPVOID pParam)
{
    if (pParam == NULL)
    {
        return -1;
    }

    ThreadParam	*pThreadParam  = reinterpret_cast<ThreadParam *>(pParam);
    CMainCapture	*pOwner = (CMainCapture *)pThreadParam->pMainCapture;
    int				count = 0;

    pOwner->_GoJob_Initialize_ForThread();

    while (TRUE)
    {
        if (pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_INIT_PROGRESS])
        {
            break;
        }

        if (count++ > 50)
        {
            break;
        }

        Sleep(10);
    }

    AfxEndThread(0);

    return 1;
}

UINT CMainCapture::_CaptureThreadCallback(LPVOID pParam)
{
    if (pParam == NULL)
    {
        return -1;
    }

    ThreadParam	*pThreadParam  = reinterpret_cast<ThreadParam *>(pParam);
    CMainCapture	*pOwner = (CMainCapture *)pThreadParam->pMainCapture;
    DWORD			Event = 0;
    BOOL			bUnexpectedExit = FALSE;
    int				nUnexpectedCount = 0;
    int				nRc;
    BOOL			bExitThread = FALSE;

    SetEvent(pThreadParam->threadStarted);
    pOwner->SetIsActiveStateOfCaptureThread(FALSE/*Capture*/, FALSE/*Dummy*/, FALSE/*Idle*/);

    while (1)
    {
        pOwner->m_bAliveCaptureThread = TRUE;

#if defined(__IBSCAN_ULTIMATE_SDK__)
        Event = WaitForMultipleObjects(3, pOwner->m_hCaptureThread_EventArray, FALSE, INFINITE);
#elif defined(__IBSCAN_SDK__)
		Event = WaitForMultipleObjects(7, pOwner->m_hCaptureThread_EventArray, FALSE, INFINITE);
#endif
        switch (Event)
        {
            case 0:
                bExitThread = TRUE;
                break;
            case 1:
                //SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);
                pOwner->SetIsActiveStateOfCaptureThread(TRUE/*Capture*/, FALSE/*Dummy*/, FALSE/*Idle*/);

                nRc = pOwner->_GoJob_PreviewImage_ForThread(pOwner->m_pAlgo->m_ImgFromCIS);
                if (nRc == ACUISITION_ERROR)
                {
                    // 20111104 enzyme add - If cypress is alive, you can try to capture again! (Just for ESD)
                    nUnexpectedCount++;
                    bUnexpectedExit = pOwner->_Go_Job_UnexpectedTermination_fromCaptureThread(nUnexpectedCount);
                    if (bUnexpectedExit)
                    {
                        ResetEvent(pOwner->m_hCaptureThread_CaptureEvent);

                        ResetEvent(pOwner->m_hCaptureThread_DummyEvent);
                        ResetEvent(pOwner->m_hCaptureThread_DestroyEvent);
#if defined(__IBSCAN_SDK__)
            			ResetEvent(pOwner->m_hRawCaptureThread_GetOneFrameImageEvent);
            			ResetEvent(pOwner->m_hRawCaptureThread_TakePreviewImageEvent);
            			ResetEvent(pOwner->m_hRawCaptureThread_TakeResultImageEvent);
#endif
                        pOwner->_PostTraceLogCallback(IBSU_ERR_CHANNEL_IO_UNEXPECTED_FAILED, "Unexpected communiation failed");
                        break;
                    }
                }
                else if (nRc == ACUISITION_NOT_COMPLETE)
                {
                    nUnexpectedCount = 0;
                }
                else
                {
                    // ACUISITION_COMPLETED or ACUISITION_ABORT
                    if (nRc == ACUISITION_ABORT)
                    {
                        pOwner->m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_CLEARD;
                    }
                    else
                    {
                        pOwner->m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_HAS_FINGERS;
                    }

#if defined(__IBSCAN_SDK__)
        			ResetEvent(pOwner->m_hRawCaptureThread_GetOneFrameImageEvent);
        			ResetEvent(pOwner->m_hRawCaptureThread_TakePreviewImageEvent);
        			ResetEvent(pOwner->m_hRawCaptureThread_TakeResultImageEvent);
#endif
                    ResetEvent(pOwner->m_hCaptureThread_CaptureEvent);
                    SetEvent(pOwner->m_hCaptureThread_DummyEvent);
                }
                break;

            case 2:
                //SetThreadExecutionState(ES_CONTINUOUS);
                pOwner->SetIsActiveStateOfCaptureThread(FALSE/*Capture*/, TRUE/*Dummy*/, FALSE/*Idle*/);

                if (pOwner->m_bCaptureThread_DummyStopMessage)
                {
                    pOwner->SetIsActiveStateOfCaptureThread(FALSE/*Capture*/, FALSE/*Dummy*/, FALSE/*Idle*/);
                    ResetEvent(pOwner->m_hCaptureThread_DummyEvent);
                }
                else
                {
                    // If CALLBACK_CLEAR_PLATEN is not registered,
                    // then the check for finger removal prior to new capture is not performed
                    if (pOwner->m_clbkParam[CALLBACK_CLEAR_PLATEN].callback)
                    {
                        nRc = pOwner->_GoJob_DummyCapture_ForThread(pOwner->m_pAlgo->m_ImgFromCIS);
                    }
                    else
                    {
                        nRc = 1;    // does not check clear platen
                    }

                    // enzyme modify 2012-11-30 for Capture_Abort
                    if (nRc > 0 || pOwner->m_clbkProperty.nPlatenState == ENUM_IBSU_PLATEN_CLEARD)
                    {
						
#if defined _POWER_OFF_ANALOG_TOUCH_WHEN_NO_CAPTURE_
						if(pOwner->m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0 && pOwner->m_propertyInfo.bEnableTOF &&
							((pOwner->m_propertyInfo.bEnableTOFforROLL && pOwner->m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER) ||
							pOwner->m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER))
						{
							pOwner->_FPGA_SetRegister(0x30, 0x66);
							pOwner->_FPGA_SetRegister(0x67, 0x00);
							pOwner->_FPGA_SetRegister(0x31, 0x3E);
						}
#endif
                        pOwner->_PostTraceLogCallback(0, "Exit  CheckForNextCapture (%d)", nRc);
                        // Complete dummy capture
                        pOwner->m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_CLEARD;
                        ResetEvent(pOwner->m_hCaptureThread_DummyEvent);

                        // We had a turn off of transformer to increase the life of LE film
                        pOwner->_SetLEOperationMode(ADDRESS_LE_OFF_MODE);

                        if (pOwner->m_propertyInfo.bEnablePowerSaveMode == TRUE/* || pOwner->m_propertyInfo.bEnableHibernateMode == TRUE*/)
                        {
                            pOwner->_SndUsbFwCaptureStop();
                        }

                        pOwner->SetIsActiveStateOfCaptureThread(FALSE/*Capture*/, FALSE/*Dummy*/, FALSE/*Idle*/);
                    }
                }
                break;
#if defined(__IBSCAN_SDK__)
    		case 3:
    			// Get one frame image
    			ResetEvent(pOwner->m_hRawCaptureThread_GetOneFrameImageEvent);
                pOwner->SetIsActiveStateOfCaptureThread(TRUE/*Capture*/, FALSE/*Dummy*/, FALSE/*Idle*/);
    			nRc = pOwner->_GoJob_OneFrameRawImage_ForThread(pOwner->m_pAlgo->m_ImgFromCIS);
    			if( nRc == ACUISITION_ERROR )
    			{
    				// 20111104 enzyme add - If cypress is alive, you can try to capture again! (Just for ESD)
    				nUnexpectedCount++;
    				bUnexpectedExit = pOwner->_Go_Job_UnexpectedTermination_fromCaptureThread(nUnexpectedCount);
    				if( bUnexpectedExit )
    				{
            			ResetEvent(pOwner->m_hCaptureThread_CaptureEvent);
            			ResetEvent(pOwner->m_hRawCaptureThread_GetOneFrameImageEvent);
            			ResetEvent(pOwner->m_hRawCaptureThread_TakePreviewImageEvent);
            			ResetEvent(pOwner->m_hRawCaptureThread_TakeResultImageEvent);
                        ResetEvent(pOwner->m_hCaptureThread_DummyEvent);
                        ResetEvent(pOwner->m_hCaptureThread_DestroyEvent);
    					pOwner->_PostCallback(CALLBACK_NOTIFY_MESSAGE, 0, 0, 0, 0, IBSU_ERR_CHANNEL_IO_UNEXPECTED_FAILED);
    					pOwner->_PostTraceLogCallback(IBSU_ERR_CHANNEL_IO_UNEXPECTED_FAILED, "Unexpected communiation failed");
    					break;
    				}
    				SetEvent(pOwner->m_hRawCaptureThread_GetOneFrameImageEvent);
    			}
    			else if( nRc == ACUISITION_NOT_COMPLETE )
    			{
    				nUnexpectedCount = 0;
    			}
    			else 
    			{
                    if (nRc == ACUISITION_ABORT)
                    {
                        pOwner->m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_CLEARD;
                    }
                    else
                    {
                        pOwner->m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_HAS_FINGERS;
                    }

                    ResetEvent(pOwner->m_hCaptureThread_CaptureEvent);
        			ResetEvent(pOwner->m_hRawCaptureThread_GetOneFrameImageEvent);
        			ResetEvent(pOwner->m_hRawCaptureThread_TakePreviewImageEvent);
        			ResetEvent(pOwner->m_hRawCaptureThread_TakeResultImageEvent);
                    SetEvent(pOwner->m_hCaptureThread_DummyEvent);
    			}
    			break;
    		case 4:
    			// Image processing for previewimage
    			ResetEvent(pOwner->m_hRawCaptureThread_TakePreviewImageEvent);
    			nRc = pOwner->_GoJob_TakePreviewImage_ForThread(pOwner->m_pAlgo->m_GetImgFromApp);
                if (nRc == ACUISITION_ABORT)
                {
                    pOwner->m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_CLEARD;

                    ResetEvent(pOwner->m_hCaptureThread_CaptureEvent);
    			    ResetEvent(pOwner->m_hRawCaptureThread_GetOneFrameImageEvent);
    			    ResetEvent(pOwner->m_hRawCaptureThread_TakePreviewImageEvent);
    			    ResetEvent(pOwner->m_hRawCaptureThread_TakeResultImageEvent);
                    SetEvent(pOwner->m_hCaptureThread_DummyEvent);
                }
    			break;
    		case 5:
    			// Image processing for resultimage
    			ResetEvent(pOwner->m_hRawCaptureThread_TakeResultImageEvent);
    			nRc = pOwner->_GoJob_TakeResultImage_ForThread(pOwner->m_pAlgo->m_GetImgFromApp);
                if (nRc == ACUISITION_ABORT)
                {
                    pOwner->m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_CLEARD;

                    ResetEvent(pOwner->m_hCaptureThread_CaptureEvent);
    			    ResetEvent(pOwner->m_hRawCaptureThread_GetOneFrameImageEvent);
    			    ResetEvent(pOwner->m_hRawCaptureThread_TakePreviewImageEvent);
    			    ResetEvent(pOwner->m_hRawCaptureThread_TakeResultImageEvent);
                    SetEvent(pOwner->m_hCaptureThread_DummyEvent);
                }
    			break;
		    case 6:
			    // m_hRawCaptureThread_AbortEvent
			    ResetEvent(pOwner->m_hRawCaptureThread_GetOneFrameImageEvent);
			    ResetEvent(pOwner->m_hRawCaptureThread_TakePreviewImageEvent);
			    ResetEvent(pOwner->m_hRawCaptureThread_TakeResultImageEvent);
			    ResetEvent(pOwner->m_hRawCaptureThread_AbortEvent);

                pOwner->SetIsActiveStateOfCaptureThread(FALSE/*Capture*/, FALSE/*Dummy*/, FALSE/*Idle*/);
			    break;
#endif
            default:
                // Idle
                pOwner->SetIsActiveStateOfCaptureThread(FALSE/*Capture*/, FALSE/*Dummy*/, TRUE/*Idle*/);
                break;
        }

        if (bExitThread || bUnexpectedExit)
        {
            break;
        }

        Sleep(1);
    }

    while (TRUE)
    {
        if (pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_FINGER_COUNT] &&
                pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_FINGER_QUALITY] &&
                pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_PREVIEW_IMAGE] &&
                pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_TAKING_ACQUISITION] &&
                pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_COMPLETE_ACQUISITION] &&
                pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_RESULT_IMAGE] &&
                pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_CLEAR_PLATEN] &&
                pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_TRACE_LOG] &&
				pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_NOTIFY_MESSAGE] &&
				pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_RESULT_IMAGE_EX] &&
#if defined(__IBSCAN_SDK__)
    			pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_ONE_FRAME_IMAGE] &&
    			pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_TAKE_PREVIEW_IMAGE] &&
    			pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_TAKE_RESULT_IMAGE] &&
#endif
				pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_KEYBUTTON])
        {
            break;
        }

        Sleep(10);
    }

    pOwner->SetIsActiveStateOfCaptureThread(FALSE/*Capture*/, FALSE/*Dummy*/, FALSE/*Idle*/);

    if (bUnexpectedExit)
    {
        // Not a planned exit, call notify if active
        char devicePath[MAX_PATH];
        int	 deviceCount = pOwner->m_pDlg->FindAllDeviceCount();
        BOOL bAliveDevice = FALSE;
        for (int i = 0; i < deviceCount; i++)
        {
            if (!pOwner->m_pDlg->_GetDevicePathByIndex(i, devicePath))
            {
                break;
            }
            if (strcmp(devicePath, pOwner->m_szDevicePath) == 0)
            {
                bAliveDevice = TRUE;
                break;
            }
        }

        if (bAliveDevice)
        {
              pOwner->m_pDlgUsbManager->m_bIsCommunicationBreak = TRUE;
//            pOwner->_PostCallback(CALLBACK_COMMUNICATION_BREAK);		// Unknown or FPGA problem

        }
    }

    pOwner->m_bAliveCaptureThread = FALSE;

    //SetThreadExecutionState(ES_CONTINUOUS);

    AfxEndThread(0);

    return 1;
}

UINT CMainCapture::_AsyncInitializeThreadCallback(LPVOID pParam)
{
    if (pParam == NULL)
    {
        return -1;
    }

    ThreadParam	*pThreadParam  = reinterpret_cast<ThreadParam *>(pParam);
    CMainCapture	*pOwner = (CMainCapture *)pThreadParam->pMainCapture;
    CIBUsbManager	*pIBUsbManager = (CIBUsbManager *)pThreadParam->pParentApp;
    int				nRc = IBSU_STATUS_OK;

    SetEvent(pThreadParam->threadStarted);
    nRc = pOwner->_OpenDeviceThread();

    if (nRc == IBSU_STATUS_OK)
    {
        pIBUsbManager->m_bInitialized = TRUE;
        pOwner->SetInitialized(TRUE);
    }
    pIBUsbManager->m_bCompletedOpenDeviceThread = TRUE;

    pOwner->m_clbkProperty.errorCode = nRc;
	pOwner->_PostCallback(CALLBACK_ASYNC_OPEN_DEVICE);

    AfxEndThread(0);

    return 1;
}

UINT CMainCapture::_KeyButtonThreadCallback(LPVOID pParam)
{
    if (pParam == NULL)
    {
        return -1;
    }

    ThreadParam	*pThreadParam  = reinterpret_cast<ThreadParam *>(pParam);
    CMainCapture	*pOwner = (CMainCapture *)pThreadParam->pMainCapture;
	DWORD			Event = 0;
    int				nRc = IBSU_STATUS_OK;
	BOOL			bExitThread=FALSE;
	unsigned char   stat1, stat2;
	unsigned char   keyButtonState = 0;
	unsigned char   saveKeyButtonState = 0xFF;
    int             keyButtonState_samecount = 0;
	unsigned char   prev_keyButtonState = 0;

	SetEvent(pThreadParam->threadStarted);

    while (1)
    {
        pOwner->m_bAliveKeyButtonThread = TRUE;

        Event = WaitForMultipleObjects(2, pOwner->m_hKeyButtonThread_EventArray, FALSE, INFINITE);

        switch (Event)
        {
            case 0:
                bExitThread = TRUE;
                break;
            case 1:
                stat1=0;
		        stat2=0;

		        if ( pOwner->_GetButtonStat_Kojak(&stat1, &stat2) == IBSU_STATUS_OK )
		        {
			        keyButtonState = stat1 + stat2*2;
                    if (keyButtonState > 2)
                    {
                        keyButtonState_samecount = -3;
                        continue;
                    }

                    if (keyButtonState == prev_keyButtonState)
                    {
                        keyButtonState_samecount++;
                    }
                    prev_keyButtonState = keyButtonState;

                    if (pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_KEYBUTTON])
                    {
			            if (keyButtonState != saveKeyButtonState && keyButtonState_samecount > 1)
			            {
				            saveKeyButtonState = keyButtonState;
                            keyButtonState_samecount = 0;
				            if (keyButtonState > 0)
				            {

                                pOwner->_PostCallback(CALLBACK_KEYBUTTON, 0, 0, 0, keyButtonState);
				            }
			            }
                    }
		        }
                break;
            default:
                // Idle
                break;
        }

        if (bExitThread)
        {
            break;
        }

        Sleep(100);
    }

    pOwner->m_bAliveKeyButtonThread = FALSE;

    AfxEndThread(0);

    return 1;
}

UINT CMainCapture::_TOFSensorThreadCallback(LPVOID pParam)
{
    if (pParam == NULL)
    {
        return -1;
    }

    ThreadParam	*pThreadParam  = reinterpret_cast<ThreadParam *>(pParam);
    CMainCapture	*pOwner = (CMainCapture *)pThreadParam->pMainCapture;
	DWORD			Event = 0;
    int				nRc = IBSU_STATUS_OK;
	BOOL			bExitThread=FALSE;
//    BOOL            bTOFCalibrated=FALSE;
    struct timeval  current_tv;
    double          lastCalibrateTime;
	int AnalogTouchArr[10] = {-100,-100,-100,-100,-100,-100,-100,-100,-100,-100};
	int AnalogTouchArrPtr = 0;


	SetEvent(pThreadParam->threadStarted);

    while (1)
    {
        pOwner->m_bAliveTOFSensorThread = TRUE;

        Event = WaitForMultipleObjects(2, pOwner->m_hTOFSensorThread_EventArray, FALSE, INFINITE);

        switch (Event)
        {
            case 0:
                bExitThread = TRUE;
                break;
            case 1:
/*				BYTE value1, value2, value3;
				pOwner->_FPGA_GetRegister(0x67, &value1);
				pOwner->_FPGA_GetRegister(0x31, &value2);
				TRACE("Analog Touch state : %s EnalbeTof:%d, EnableRoll:%d, NeedtoInitCIS:%d\n", 
					(value1 == 0x00 && value2 == 0x3E)?"OFF":"ON",
					pOwner->m_propertyInfo.bEnableTOF,
					pOwner->m_propertyInfo.bEnableTOFforROLL,
					pOwner->m_bNeedtoInitializeCIS_Five0);
*/
                if (pOwner->m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0 && 
					pOwner->m_propertyInfo.bEnableTOF == TRUE &&
					pOwner->m_UsbDeviceInfo.bCanUseTOF)
	            {
                    if (pOwner->m_bTOFCalibrated && pOwner->GetIsActiveCapture())
                    {
    	                pOwner->_ReadTOFSensorAnalog_Five0(&pOwner->m_nAnalogTouch_PlateOrg, &pOwner->m_nAnalogTouch_FilmOrg,
															&pOwner->m_nAnalogTouch_Plate, &pOwner->m_nAnalogTouch_Film);

//						TRACE("plate_org(%d) ==> plate(%d)\n", pOwner->m_nAnalogTouch_PlateOrg, pOwner->m_nAnalogTouch_Plate);

						AnalogTouchArr[AnalogTouchArrPtr++] = pOwner->m_nAnalogTouch_PlateOrg;
						if(AnalogTouchArrPtr == 10)
							AnalogTouchArrPtr = 0;

#ifdef __G_DEBUG__
						TRACE("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", 
									AnalogTouchArr[0], AnalogTouchArr[1], AnalogTouchArr[2], AnalogTouchArr[3], AnalogTouchArr[4],
									AnalogTouchArr[5], AnalogTouchArr[6], AnalogTouchArr[7], AnalogTouchArr[8], AnalogTouchArr[9]);
#endif
                    }
	            }
				else
				{
					pOwner->m_nAnalogTouch_Plate = -1;
					pOwner->m_nAnalogTouch_Film = -1;
					pOwner->m_nAnalogTouch_PlateOrg = -1;
					pOwner->m_nAnalogTouch_FilmOrg = -1;
				}

                Sleep(100);
                break;
            default:
                // Idle
                break;
        }

        if (bExitThread)
        {
            break;
        }

        Sleep(100);

		if (pOwner->m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0 && 
			pOwner->m_propertyInfo.bEnableTOF == TRUE &&
			pOwner->m_UsbDeviceInfo.bCanUseTOF &&
			((pOwner->m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && pOwner->m_propertyInfo.bEnableTOFforROLL) ||
			pOwner->m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER) &&
			pOwner->GetIsActiveCapture())
		{
			BOOL bNeedtoCalibration = FALSE;

            pOwner->gettimeofday(&current_tv, NULL);
            lastCalibrateTime = (current_tv.tv_sec - pOwner->m_LastCalibrate_tv.tv_sec);
			if(lastCalibrateTime >= 5*60)		// Whenever 5 minutes
			{
#ifdef __G_DEBUG__
                TRACE("--------------- Calibration TOF (time limit) ---------------\n");
#endif

				bNeedtoCalibration = TRUE;
			}
			
			// 5개가 모두 같은 값이 될 경우 Re-calibration
			else if(AnalogTouchArr[0] == AnalogTouchArr[1] &&
					AnalogTouchArr[0] == AnalogTouchArr[2] &&
					AnalogTouchArr[0] == AnalogTouchArr[3] &&
					AnalogTouchArr[0] == AnalogTouchArr[4] &&
					AnalogTouchArr[0] == AnalogTouchArr[5] &&
					AnalogTouchArr[0] == AnalogTouchArr[6] &&
					AnalogTouchArr[0] == AnalogTouchArr[7] &&
					AnalogTouchArr[0] == AnalogTouchArr[8] &&
					AnalogTouchArr[0] == AnalogTouchArr[9] &&
					AnalogTouchArr[0] != -100 &&
					AnalogTouchArr[1] != -100 &&
					AnalogTouchArr[2] != -100 &&
					AnalogTouchArr[3] != -100 &&
					AnalogTouchArr[4] != -100 &&
					AnalogTouchArr[5] != -100 &&
					AnalogTouchArr[6] != -100 &&
					AnalogTouchArr[7] != -100 &&
					AnalogTouchArr[8] != -100 &&
					AnalogTouchArr[9] != -100)
			{
#ifdef __G_DEBUG__
				TRACE("--------------- Calibration TOF (same analog value) ---------------\n");
				TRACE("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", 
									AnalogTouchArr[0], AnalogTouchArr[1], AnalogTouchArr[2], AnalogTouchArr[3], AnalogTouchArr[4],
									AnalogTouchArr[5], AnalogTouchArr[6], AnalogTouchArr[7], AnalogTouchArr[8], AnalogTouchArr[9]);
#endif
				pOwner->_PostTraceLogCallback(0, "--------------- Calibration TOF (same analog value) ---------------");

				bNeedtoCalibration = TRUE;
			}
			
			if(bNeedtoCalibration == TRUE)
			{
				if (pOwner->_CalibrateTOF() == TRUE)
				{
					pOwner->m_bTOFCalibrated = TRUE;
				}
				else
				{
					pOwner->m_bTOFCalibrated = FALSE;
				}

				AnalogTouchArr[0] = AnalogTouchArr[1] =
				AnalogTouchArr[2] = AnalogTouchArr[3] =
				AnalogTouchArr[4] = AnalogTouchArr[5] =
				AnalogTouchArr[6] = AnalogTouchArr[7] =
				AnalogTouchArr[8] = AnalogTouchArr[9] = -100;
			}
		}
    }

    pOwner->m_bAliveTOFSensorThread = FALSE;

    AfxEndThread(0);

    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// Static members

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// These USB functions are specific to Windows and aren't built on WinCE
/////////////////////////////////////////////////////////////////////////////

#if defined(_WINDOWS) && defined(__USE_WINUSB_DRIVER__)

int CMainCapture::_UsbBulkOutIn(int outEp, UCHAR uiCommand, UCHAR *outData, LONG outDataLen,
                                int inEp, UCHAR *inData, LONG inDataLen, LONG *nBytesRead, int timeout)
{
    /////////////////////////////////////////////////////////////////////////////////////
    // 2013-06-13 enzyme add - add code to avoid the slower frame speed
    // on the multi-devices environment because Columbo and Curve has a different logic to get a frame
    // call EP1OUT(GET_ONE_FRAME) then get frame from EP6IN
    // So previous code used the "CThreadSync Sync;" to avoid the dead lock,
    // but it makes the slower frame speed on the multi-devices environment
    BOOL bExit = TRUE;
    if (m_bRunningEP6)
    {
        for (int i = 0; i < 500; i++)
        {
            if (!m_bRunningEP6)
            {
                bExit = FALSE;
                break;
            }
            Sleep(1);
        }
        if (bExit)
        {
#ifdef __G_DEBUG__
            TRACE("Unexpected _UsbBulkOutIn() failed = %d\n", m_bRunningEP6);
#endif
            return IBSU_ERR_CHANNEL_IO_COMMAND_FAILED;
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////
#if defined(__MORPHO_TRAK__)
    if (inEp >= 0)
    {
        if (m_EnterSleep)
	    {
		    _PostTraceLogCallback(0, "Asking _UsbBulkOutIn, but Sleeping\n");
	    }
        while (m_EnterSleep)
	    {
		    Sleep(10);
	    }
    }
#endif

    CThreadSync Sync;		// 2011-12-18 enzyme add
    IO_PACKET	io_packet;
    int			nRc = IBSU_ERR_CHANNEL_IO_COMMAND_FAILED;
    LONG		lByteCount;

    if (outEp >= 0)
    {
        // 최대 가능한 데이터 수로 제한
        if (outDataLen > MAX_DATA_BYTES)
        {
            outDataLen = MAX_DATA_BYTES;
        }

        io_packet.cmd.Ioctl     = (UCHAR)uiCommand;
        if (outDataLen > 0)
        {
            memcpy(io_packet.cmd.Buffer, outData, outDataLen);
        }

        lByteCount = outDataLen + 1;

        //		if( outDataLen > 0 )
        //			TRACE("[ START-UsbBulkOut ] = %x %d\n", io_packet.cmd.Ioctl, io_packet.cmd.Buffer[0]);
        //		else
        //			TRACE("[ START-UsbBulkOut ] = %x - \n", io_packet.cmd.Ioctl);

        ULONG cbSent = 0;
        if (_GetPID() == __PID_CURVE__)
        {
            nRc = m_pCurveUsbDevice->WritePipe(outEp, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
        }
        else
        {
            nRc = m_pUsbDevice->WritePipe(outEp, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
        }
        //		TRACE("[ END-UsbBulkOut ] = %d %x\n", nRc, io_packet.cmd.Ioctl);

        if (nRc != IBSU_STATUS_OK)
        {
            // It may make the problem in the Communication Break routine, so please do not enable this comment.
            //			_PostTraceLogCallback(nRc, "Usb bulk-out failed(%02x %02x), GetLastError(%ld)",
            //				outEp, uiCommand, GetLastError());
            return nRc;
        }
    }

    if (inEp >= 0)
    {
        //		TRACE("[ START-UsbBulkIn ] = %d\n", inEp);
        ULONG cbRead = 0;
        if (_GetPID() == __PID_CURVE__)
        {
            nRc = m_pCurveUsbDevice->ReadPipe(inEp, inData, inDataLen, &cbRead, timeout);
        }
        else
        {
            nRc = m_pUsbDevice->ReadPipe(inEp, inData, inDataLen, &cbRead, timeout);
        }

        if (nBytesRead != NULL)
        {
            *nBytesRead = cbRead;
        }
       //		TRACE("[ END-UsbBulkIn ] = %d %d\n", nRc, inEp);

        if (nRc != IBSU_STATUS_OK)
        {
            // It may make the problem in the Communication Break routine, so please do not enable this comment.
            //			_PostTraceLogCallback(nRc, "Usb bulk-in failed(%02x), GetLastError(%ld)",
            //				inEp, GetLastError());
            return nRc;
        }
    }

    return nRc;
}

int CMainCapture::_UsbBulkIn(int ep, UCHAR *data, LONG dataLen, LONG *nBytesRead, int timeout)
{
    //	CThreadSync Sync;		// 2012-10-31 enzyme delete
    int			nRc = IBSU_ERR_CHANNEL_IO_COMMAND_FAILED;

    ULONG cbRead = 0;
    //TRACE("[ START-UsbBulkIn ] = %d\n", ep);
#if defined(__MORPHO_TRAK__)
    int nSuspendCount = 0;
	if (m_EnterSleep)
	{
		_PostTraceLogCallback(0, "Asking _UsbBulkIn, but Sleeping\n");
        nSuspendCount = 1;
	}
	
    while(m_EnterSleep)
	{
        nSuspendCount = 1;
		Sleep(10);
	}

    if (nSuspendCount > 0)
    {
		_PostTraceLogCallback(0, "_UsbBulkIn failed because of Sleeping\n");
        return IBSU_ERR_CHANNEL_IO_UNEXPECTED_FAILED;
    }
#endif

    if (_GetPID() == __PID_CURVE__)    {
        nRc = m_pCurveUsbDevice->SyncReadPipe(ep, data, dataLen, &cbRead, timeout);
    }
    else
    {
        nRc = m_pUsbDevice->SyncReadPipe(ep, data, dataLen, &cbRead, timeout);
    }
    //TRACE("[ END-UsbBulkIn ] = %d %d\n", nRc, ep);

    if (nBytesRead != NULL)
    {
        *nBytesRead = cbRead;
    }

    return nRc;
}

int CMainCapture::_UsbBulkOut(int ep, UCHAR uiCommand, UCHAR *data, LONG dataLen, int timeout)
{
    CThreadSync Sync;

    return _UsbBulkOut_Low(ep, uiCommand, data, dataLen, timeout);
}

int CMainCapture::_UsbBulkOut_Low(int ep, UCHAR uiCommand, UCHAR *data, LONG dataLen, int timeout)
{
    IO_PACKET	io_packet;
    int			nRc = IBSU_ERR_CHANNEL_IO_COMMAND_FAILED;
    LONG		lByteCount;

    // 최대 가능한 데이터 수로 제한
    if (dataLen > MAX_DATA_BYTES)
    {
        dataLen = MAX_DATA_BYTES;
    }

    io_packet.cmd.Ioctl     = (UCHAR)uiCommand;
    if (dataLen > 0)
    {
        memcpy(io_packet.cmd.Buffer, data, dataLen);
    }

    lByteCount = dataLen + 1;

    ULONG cbSent = 0;
    if (_GetPID() == __PID_CURVE__)
    {
        nRc = m_pCurveUsbDevice->WritePipe(ep, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
    }
    else
    {
        nRc = m_pUsbDevice->WritePipe(ep, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
    }

    return nRc;
}

BOOL CMainCapture::_OpenUSBDevice(int deviceIndex, int *errorNo)
{
    int enumeratedIndex;
    int vid, pid;

    *errorNo = IBSU_STATUS_OK;

    if (!m_pDlg->_GetEnumeratedIndexFromSortedIndex(deviceIndex, (DWORD *)&enumeratedIndex, &vid, &pid))
    {
        *errorNo = IBSU_ERR_DEVICE_NOT_FOUND;
        return FALSE;
    }

    // Set device index
    m_unDeviceNumber = deviceIndex;

    if (pid == __PID_CURVE__)
    {
        if (m_pCurveUsbDevice->Open(enumeratedIndex, errorNo) == FALSE)
        {
            return FALSE;
        }

        return TRUE;
    }

    if (m_pUsbDevice->Open(enumeratedIndex, errorNo) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

void CMainCapture::_CloseUSBDevice()
{
    if (_GetPID() == __PID_CURVE__)
    {
        m_pCurveUsbDevice->Close();
        return;
    }

    m_pUsbDevice->Close();
}

void CMainCapture::_SetPID_VID(UsbDeviceInfo *pScanDevDesc, char *productID)
{
    if (m_pCurveUsbDevice->Vid() > 0 && m_pCurveUsbDevice->Pid() > 0)
    {
        pScanDevDesc->vid = m_pCurveUsbDevice->Vid();
        pScanDevDesc->pid = m_pCurveUsbDevice->Pid();
        return;
    }

    pScanDevDesc->vid = m_pUsbDevice->Vid();
    pScanDevDesc->pid = m_pUsbDevice->Pid();
}

int CMainCapture::_GetPID()
{
    if (m_pCurveUsbDevice->Pid() > 0)
    {
        return m_pCurveUsbDevice->Pid();
    }

    return m_pUsbDevice->Pid();
}

int CMainCapture::_GetUsbSpeed(UsbSpeed *speed)
{
    if (_GetPID() == __PID_CURVE__)
    {
        CThreadSync Sync;		// Important!
        return m_pCurveUsbDevice->GetUsbSpeed(speed);
    }

    return m_pUsbDevice->GetUsbSpeed(speed);
}

#endif // #if defined(_WINDOWS) && defined(__USE_WINUSB_DRIVER__)

int CMainCapture::ClientWindow_Create(const IBSU_HWND hWindow, const DWORD left, const DWORD top, const DWORD right, const DWORD bottom, BOOL bReserved, DWORD imgWidth, DWORD imgHeight)
{
    int imgWidth_Roll, imgHeight_Roll;
    RECT rt;
    GetClientRect(hWindow, &rt);

	if (left < 0 || top < 0 || right > (DWORD)rt.right || bottom > (DWORD)rt.bottom ||
		(right - left) < 0 || (bottom - top) < 0)
    {
        return IBSU_ERR_INVALID_PARAM_VALUE;
    }

    if ((right - left) > __MAX_IMG_WIDTH__ || (bottom - top) > __MAX_IMG_HEIGHT__)
    {
        return IBSU_ERR_INVALID_PARAM_VALUE;
    }


    if (!bReserved)
    {
        imgWidth = m_UsbDeviceInfo.ImgWidth;
        imgHeight = m_UsbDeviceInfo.ImgHeight;
        imgWidth_Roll = m_UsbDeviceInfo.ImgWidth_Roll;
        imgHeight_Roll = m_UsbDeviceInfo.ImgHeight_Roll;
    }

    m_hWindow = hWindow;
    m_rectWindow.left = left;
    m_rectWindow.top = top;
    m_rectWindow.right = right;
    m_rectWindow.bottom = bottom;

    ////////////////////////////////////////////////////////////////////////////////////////
    // enzyme add 2012-12-20 We have to redraw image viewer.
    // Watson & Watson Mini & Sherlock is 800*750 pixels, Curve is 288*352 pixels, Columbo is 400*500 pixels
    InvalidateRect(m_hWindow, &rt, TRUE);
    UpdateWindow(m_hWindow);
    ////////////////////////////////////////////////////////////////////////////////////////

	m_DisplayWindow.x = rt.left;
	m_DisplayWindow.y = rt.top;
	m_DisplayWindow.Width = (rt.right - rt.left);
	m_DisplayWindow.Height = (rt.bottom - rt.top);

    _SetDisplayWindowParam(imgWidth, imgHeight, &m_DisplayWindow.imgX, &m_DisplayWindow.imgY, &m_DisplayWindow.imgWidth, &m_DisplayWindow.imgHeight);
    _SetDisplayWindowParam(imgWidth_Roll, imgHeight_Roll, &m_DisplayWindow.imgX_Roll, &m_DisplayWindow.imgY_Roll, &m_DisplayWindow.imgWidth_Roll, &m_DisplayWindow.imgHeight_Roll);

    if (!m_DisplayWindow.image)
    {
        m_DisplayWindow.image = new BYTE[__MAX_IMG_WIDTH__ * __MAX_IMG_HEIGHT__];
    }
    /////////////////////////////////////////////////////////////////////////////////////////////

    m_DisplayWindow.bIsSetDisplayWindow = TRUE;

    return IBSU_STATUS_OK;
}

int CMainCapture::_SetDisplayWindowParam(int imgWidth, int imgHeight, int *displayImgX, int *displayImgY, int *displayImgWidht, int *displayImgHeight)
{
    // enzyme 2013-01-31 bug fixed when you use big viewer
    DWORD outWidth = m_rectWindow.right - m_rectWindow.left;
    DWORD outHeight = m_rectWindow.bottom - m_rectWindow.top;
    int tmp_width = outWidth;
    int tmp_height = outHeight;

    if (outWidth > (DWORD)imgWidth)
    {
        tmp_width = imgWidth;
    }
    if (outHeight > (DWORD)imgHeight)
    {
        tmp_height = imgHeight;
    }

    float ratio_width = (float)tmp_width / (float)imgWidth;
    float ratio_height = (float)tmp_height / (float)imgHeight;

    if (ratio_width >= ratio_height)
    {
        *displayImgWidht = tmp_height * imgWidth / imgHeight;
		*displayImgWidht -= (*displayImgWidht % 4);
        *displayImgHeight = tmp_height;
        *displayImgX = (tmp_width - *displayImgWidht) / 2 + m_rectWindow.left;
        *displayImgY = m_rectWindow.top;
    }
    else
    {
        *displayImgWidht = tmp_width;
		*displayImgWidht -= (*displayImgWidht % 4);
        *displayImgHeight = tmp_width * imgHeight / imgWidth;
        *displayImgX = m_rectWindow.left;
        *displayImgY = (tmp_height - *displayImgHeight) / 2 + m_rectWindow.top;
    }

    if (*displayImgX < 0)
    {
        *displayImgX = 0;
    }
    if (*displayImgY < 0)
    {
        *displayImgY = 0;
    }

    return IBSU_STATUS_OK;
}

int CMainCapture::ClientWindow_Destroy(const BOOL clearExistingInfo)
{
    if (!m_DisplayWindow.bIsSetDisplayWindow)
    {
        return IBSU_ERR_CLIENT_WINDOW_NOT_CREATE;
    }

    if (clearExistingInfo)
    {
        // Reset display window information.  The image buffer still needs to be freed.
        m_DisplayWindow.rollGuideLine = TRUE;
		m_DisplayWindow.dispInvalidArea = FALSE;
        m_DisplayWindow.bkColor = 0x00d8e9ec;		// Button face color of Windows
		m_DisplayWindow.rollGuideLineWidth = 2;

        _ClearOverlayText();
        _RemoveAllOverlayObject();
    }

    m_DisplayWindow.bIsSetDisplayWindow = FALSE;

//    _DrawClientWindow(m_pAlgo->m_OutResultImg, m_DisplayWindow);

    return IBSU_STATUS_OK;
}

int	CMainCapture::ClientWindow_GetProperty(const IBSU_ClientWindowPropertyId propertyId, LPSTR propertyValue)
{
    switch (propertyId)
    {
        case ENUM_IBSU_WINDOW_PROPERTY_BK_COLOR:
            sprintf(propertyValue, "%u", m_DisplayWindow.bkColor);
            break;

        case ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE:
            if (m_DisplayWindow.rollGuideLine)
            {
                sprintf(propertyValue, "%s", "TRUE");
            }
            else
            {
                sprintf(propertyValue, "%s", "FALSE");
            }
            break;

		case ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA:
			if( m_DisplayWindow.dispInvalidArea )
				sprintf(propertyValue, "%s", "TRUE");
			else
				sprintf(propertyValue, "%s", "FALSE");
			break;

		case ENUM_IBSU_WINDOW_PROPERTY_SCALE_FACTOR:
			{
                double scaleFactor;
                if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
                {
                    scaleFactor = (double)m_DisplayWindow.imgWidth_Roll / m_UsbDeviceInfo.ImgWidth_Roll;
                }
                else
                {
				    scaleFactor = (double)m_DisplayWindow.imgWidth / m_UsbDeviceInfo.ImgWidth;
                }

				sprintf(propertyValue, "%f", scaleFactor);
			}
			break;

		case ENUM_IBSU_WINDOW_PROPERTY_LEFT_MARGIN:
			{
				int leftMargin = m_DisplayWindow.imgX;
                if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
                {
                    leftMargin = m_DisplayWindow.imgX_Roll;
                }
				sprintf(propertyValue, "%d", leftMargin);
			}
			break;

		case ENUM_IBSU_WINDOW_PROPERTY_TOP_MARGIN:
			{
				int topMargin = m_DisplayWindow.imgY;
                if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
                {
                    topMargin = m_DisplayWindow.imgY_Roll;
                }
				sprintf(propertyValue, "%d", topMargin);
			}
			break;

		case ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE_WIDTH:
            sprintf(propertyValue, "%d", m_DisplayWindow.rollGuideLineWidth);
            break;

		case ENUM_IBSU_WINDOW_PROPERTY_SCALE_FACTOR_EX:
			{
				double scaleFactor;
                if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
                {
                    scaleFactor = (double)m_DisplayWindow.imgWidth_Roll / m_UsbDeviceInfo.ImgWidth_Roll;
                }
                else
                {
                    scaleFactor = (double)m_DisplayWindow.imgWidth / m_UsbDeviceInfo.ImgWidth;
                }
				
				sprintf(propertyValue, "%d", (int)(scaleFactor*1000000));
			}
			break;

        case ENUM_IBSU_WINDOW_PROPERTY_KEEP_REDRAW_LAST_IMAGE:
			if( m_DisplayWindow.keepRedrawLastImage )
				sprintf(propertyValue, "%s", "TRUE");
			else
				sprintf(propertyValue, "%s", "FALSE");
			break;

        default:
            return IBSU_ERR_NOT_SUPPORTED;
    }

    return IBSU_STATUS_OK;
}

int	CMainCapture::ClientWindow_SetProperty(const IBSU_ClientWindowPropertyId propertyId, LPCSTR propertyValue)
{
    switch (propertyId)
    {
        case ENUM_IBSU_WINDOW_PROPERTY_BK_COLOR:
            m_DisplayWindow.bkColor = atoi(propertyValue);
            break;

        case ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE:
            if (strcmp(propertyValue, "TRUE") == 0)
            {
                m_DisplayWindow.rollGuideLine = TRUE;
            }
            else if (strcmp(propertyValue, "FALSE") == 0)
            {
                m_DisplayWindow.rollGuideLine = FALSE;
            }
            else
            {
                return IBSU_ERR_INVALID_PARAM_VALUE;
            }
            break;

		case ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA:
			if( strcmp(propertyValue, "TRUE") == 0 )
				m_DisplayWindow.dispInvalidArea = TRUE;
			else if( strcmp(propertyValue, "FALSE") == 0 )
				m_DisplayWindow.dispInvalidArea = FALSE;
			else
				return IBSU_ERR_INVALID_PARAM_VALUE;
			break;

		case ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE_WIDTH:
			{
				int temp = atoi(propertyValue);
				if (temp < 1 || temp > 6)
					return IBSU_ERR_INVALID_PARAM_VALUE;

				m_DisplayWindow.rollGuideLineWidth = temp;
			}
			break;

        case ENUM_IBSU_WINDOW_PROPERTY_KEEP_REDRAW_LAST_IMAGE:
			if( strcmp(propertyValue, "TRUE") == 0 )
				m_DisplayWindow.keepRedrawLastImage = TRUE;
			else if( strcmp(propertyValue, "FALSE") == 0 )
				m_DisplayWindow.keepRedrawLastImage = FALSE;
			else
				return IBSU_ERR_INVALID_PARAM_VALUE;
			break;

        default:
            return IBSU_ERR_NOT_SUPPORTED;
    }

    return IBSU_STATUS_OK;
}

int	CMainCapture::ClientWindow_SetOverlayText(const char *fontName, const int fontSize, const BOOL fontBold,
        const char *text, const int posX, const int posY, const DWORD textColor)
{
    OverlayText textInfo;

    memset(&textInfo, 0, sizeof(OverlayText));

    strcpy(textInfo.familyName, fontName);
    textInfo.size = fontSize;
    textInfo.bold = fontBold;
    strcpy(textInfo.text, text);
    textInfo.x = posX;
    textInfo.y = posY;
    textInfo.color = textColor;

    /* To notify deprecated API to users */
	_PostCallback(CALLBACK_NOTIFY_MESSAGE, 0, 0, 0, IBSU_WRN_API_DEPRECATED);
	_PostTraceLogCallback(IBSU_WRN_API_DEPRECATED, "IBSU_SetClientWindowOverlayText was deprecated. Please use IBSU_AddOverlayText instead");

    if (m_DisplayWindow.bIsSetDisplayWindow)
    {
        OverlayText *pOverlayText = NULL;
        if ((pOverlayText = _FindOverlayTextInList(textInfo.x, textInfo.y)) != NULL)
        {
            memcpy(pOverlayText, &textInfo, sizeof(OverlayText));
        }
        else
        {
            pOverlayText = new OverlayText;
            memcpy(pOverlayText, &textInfo, sizeof(OverlayText));
            m_pOverlayText.push_back(pOverlayText);
        }

		_ReDrawClientWindow_Overlays(m_pAlgo->m_OutResultImg, m_DisplayWindow);

        return IBSU_STATUS_OK;
    }

    return IBSU_ERR_CLIENT_WINDOW;
}

void CMainCapture::_DrawClientWindow(BYTE *InImg, DisplayWindow dispWindow,
									 std::vector<OverlayText *>*overlayText,
									 std::vector<OverlayTextEx *>*overlayTextEx,
									 std::vector<OverlayLineEx *>*overlayLineEx,
									 std::vector<OverlayQuadrangleEx *>*overlayQuadrangleEx,
									 std::vector<OverlayShapeEx *>*overlayShapeEx,
									 const DWORD flags)
{
	if( !dispWindow.bIsSetDisplayWindow ||
        m_propertyInfo.ImageType == ENUM_IBSU_TYPE_NONE)
		return;

    // Overlay text for display on window with double buffering
    HDC		hdc = ::GetDC(m_hWindow);
    HDC		hMemDc;
    HBITMAP hBmpBuffer;
    HBRUSH	brush, old_brush;
	RECT	rt;
    int     srcWidth=0, srcHeight=0;

	::GetClientRect(m_hWindow, &rt);

    hMemDc = ::CreateCompatibleDC(hdc);
    hBmpBuffer = ::CreateCompatibleBitmap(hdc, dispWindow.Width, dispWindow.Height);
    HGDIOBJ hOldBmp = ::SelectObject(hMemDc, hBmpBuffer);

    brush = ::CreateSolidBrush((COLORREF)dispWindow.bkColor);
    old_brush = (HBRUSH)::SelectObject(hMemDc, brush);

    RECT rect;
	rect.left = dispWindow.x;
    rect.right = dispWindow.Width;
	rect.top = dispWindow.y;
    rect.bottom = dispWindow.Height;

    ::FillRect(hMemDc, &rect, brush);
    ::SelectObject(hMemDc, old_brush);
    ::DeleteObject(brush);

    ////////////////////////////////////////////////////////////////////////////////////////////
    // enzyme delete 2013-02-05 - we used same code to _DrawClientWindow() to remove "CThreadSync Sync;" on this function
    // Then we can save capture time from each devices.
    if ( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
    {
        m_BitmapInfo->bmiHeader.biHeight = dispWindow.imgHeight_Roll;
        m_BitmapInfo->bmiHeader.biSizeImage = dispWindow.imgWidth_Roll * dispWindow.imgHeight_Roll;
        m_BitmapInfo->bmiHeader.biWidth = dispWindow.imgWidth_Roll;
    }
    else
    {
        m_BitmapInfo->bmiHeader.biHeight = dispWindow.imgHeight;
        m_BitmapInfo->bmiHeader.biSizeImage = dispWindow.imgWidth * dispWindow.imgHeight;
        m_BitmapInfo->bmiHeader.biWidth = dispWindow.imgWidth;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////

    if (dispWindow.bIsSetDisplayWindow)
    {
		if (flags == 0x00000000)
		{
			if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
			{
				if (m_UsbDeviceInfo.ImgWidth_Roll == dispWindow.imgWidth_Roll &&
					m_UsbDeviceInfo.ImgHeight_Roll == dispWindow.imgHeight_Roll)
				{
					memcpy(dispWindow.image, InImg, m_UsbDeviceInfo.ImgWidth_Roll * m_UsbDeviceInfo.ImgHeight_Roll);
				}
				else
				{
					m_pAlgo->_Algo_ZoomOut_with_Bilinear(InImg, m_UsbDeviceInfo.ImgWidth_Roll, m_UsbDeviceInfo.ImgHeight_Roll,
														 dispWindow.image, dispWindow.imgWidth_Roll, dispWindow.imgHeight_Roll);
				}
			}
			else
			{
				if (m_UsbDeviceInfo.ImgWidth == dispWindow.imgWidth &&
						m_UsbDeviceInfo.ImgHeight == dispWindow.imgHeight)
				{
					memcpy(dispWindow.image, InImg, m_UsbDeviceInfo.ImgWidth * m_UsbDeviceInfo.ImgHeight);
				}
				else
				{
					m_pAlgo->_Algo_ZoomOut_with_Bilinear(InImg, m_UsbDeviceInfo.ImgWidth, m_UsbDeviceInfo.ImgHeight,
														 dispWindow.image, dispWindow.imgWidth, dispWindow.imgHeight);
				}
			}
		}

		if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
        {
            ::StretchDIBits(hMemDc, dispWindow.imgX_Roll - dispWindow.x, dispWindow.imgY_Roll - dispWindow.y, dispWindow.imgWidth_Roll, dispWindow.imgHeight_Roll, 0, 0,
                            dispWindow.imgWidth_Roll, dispWindow.imgHeight_Roll, dispWindow.image, (BITMAPINFO*)m_BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        }
        else
        {
            ::StretchDIBits(hMemDc, dispWindow.imgX - dispWindow.x, dispWindow.imgY - dispWindow.y, dispWindow.imgWidth, dispWindow.imgHeight, 0, 0,
                            dispWindow.imgWidth, dispWindow.imgHeight, dispWindow.image, (BITMAPINFO*)m_BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        }

        _DrawClientWindow_Text(hMemDc, dispWindow, overlayText);
        _DrawClientWindow_TextEx(hMemDc, dispWindow, overlayTextEx);
        _DrawClientWindow_LineEx(hMemDc, dispWindow, overlayLineEx);
        _DrawClientWindow_QuadrangleEx(hMemDc, dispWindow, overlayQuadrangleEx);
        _DrawClientWindow_ShapeEx(hMemDc, dispWindow, overlayShapeEx);

	    _DrawClientWindow_DispInvalidArea(hMemDc, dispWindow);

#ifdef __IBSCAN_ULTIMATE_SDK__
	    _DrawClientWindow_RollGuideLine(hMemDc, dispWindow);
#endif
	}
	else
	{
		if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
        {
		    memset(dispWindow.image, 0xFF, dispWindow.imgWidth_Roll*dispWindow.imgHeight_Roll);
		    ::StretchDIBits(hMemDc, dispWindow.imgX_Roll - dispWindow.x, dispWindow.imgY_Roll - dispWindow.y, dispWindow.imgWidth_Roll, dispWindow.imgHeight_Roll, 0, 0, 
			    dispWindow.imgWidth_Roll, dispWindow.imgHeight_Roll, dispWindow.image, (BITMAPINFO*)m_BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        }
        else
        {
		    memset(dispWindow.image, 0xFF, dispWindow.imgWidth*dispWindow.imgHeight);
		    ::StretchDIBits(hMemDc, dispWindow.imgX - dispWindow.x, dispWindow.imgY - dispWindow.y, dispWindow.imgWidth, dispWindow.imgHeight, 0, 0, 
			    dispWindow.imgWidth, dispWindow.imgHeight, dispWindow.image, (BITMAPINFO*)m_BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        }
	}

    ::BitBlt(hdc, dispWindow.x, dispWindow.y, dispWindow.Width, dispWindow.Height, hMemDc, 0, 0, SRCCOPY);
	// Always free the memory DC first, then the bitmap to avoid common resource leaks.
	::SelectObject(hMemDc, hOldBmp);
	::DeleteObject(hMemDc);
	::DeleteObject(hBmpBuffer);
	::ReleaseDC(m_hWindow, hdc);
}

int CMainCapture::ClientWindow_ShowOverlayObject(const int overlayHandle, const BOOL show)
{
	return _ShowOverlayObject(overlayHandle, show);
}

int CMainCapture::ClientWindow_ShowAllOverlayObject(const BOOL show)
{
	return _ShowAllOverlayObject(show);
}

int CMainCapture::ClientWindow_RemoveOverlayObject(const int overlayHandle)
{
	return _RemoveOverlayObject(overlayHandle);
}

int CMainCapture::ClientWindow_RemoveAllOverlayObject()
{
	return _RemoveAllOverlayObject();
}

int CMainCapture::ClientWindow_AddOverlayText(int *pOverlayHandle, const char *fontName, const int fontSize, const BOOL fontBold,
									 const char *text, const int posX, const int posY, const DWORD color)
{
	OverlayHandle *pOvHandle = NULL;
	OverlayTextEx info;

	*pOverlayHandle = -1;
	memset(&info, 0, sizeof(OverlayTextEx));

	strcpy(info.familyName, fontName);
	info.size = fontSize;
	info.bold = fontBold;
	strcpy(info.text, text);
	info.x = posX;
	info.y = posY;
	info.color = color;

//	if( posX < 0 || posY < 0 )
//		return IBSU_ERR_INVALID_PARAM_VALUE;

    if( m_DisplayWindow.bIsSetDisplayWindow )
	{
		if( (pOvHandle = _CreateOverlayHandle()) == NULL )
			return IBSU_ERR_INVALID_OVERLAY_HANDLE;

		info.handle = pOvHandle->handle;
		info.pShow = &pOvHandle->show;
		*pOverlayHandle = pOvHandle->handle;

	    OverlayTextEx *pOverlay = NULL;
	    pOverlay = new OverlayTextEx;
	    memcpy(pOverlay, &info, sizeof(OverlayTextEx));
	    m_pOverlayTextEx.push_back(pOverlay);

		_ReDrawClientWindow_Overlays(m_pAlgo->m_OutResultImg, m_DisplayWindow);

		return IBSU_STATUS_OK;
	}
	
	return IBSU_ERR_CLIENT_WINDOW;
}

int CMainCapture::ClientWindow_ModifyOverlayText(const int overlayHandle, const char *fontName, const int fontSize, const BOOL fontBold,
									 const char *text, const int posX, const int posY, const DWORD color)
{
    if( !_FindOverlayHandleInList(overlayHandle) )
        return IBSU_ERR_INVALID_OVERLAY_HANDLE;

//	if( posX < 0 || posY < 0 )
//		return IBSU_ERR_INVALID_PARAM_VALUE;

    if( m_DisplayWindow.bIsSetDisplayWindow )
	{
	    OverlayTextEx *pOverlay = NULL;
		if( (pOverlay = _FindOverlayTextExInList(overlayHandle)) == NULL )
			return IBSU_ERR_INVALID_OVERLAY_HANDLE;

		strcpy(pOverlay->familyName, fontName);
		pOverlay->size = fontSize;
		pOverlay->bold = fontBold;
		strcpy(pOverlay->text, text);
		pOverlay->x = posX;
		pOverlay->y = posY;
		pOverlay->color = color;

		_ReDrawClientWindow_Overlays(m_pAlgo->m_OutResultImg, m_DisplayWindow);

		return IBSU_STATUS_OK;
	}
	
	return IBSU_ERR_CLIENT_WINDOW;
}

int CMainCapture::ClientWindow_AddOverlayLine(int *pOverlayHandle, const int x1, const int y1,
                                     const int x2, const int y2, const int width, const DWORD color)
{
	OverlayHandle *pOvHandle = NULL;
	OverlayLineEx info;

	*pOverlayHandle = -1;
	memset(&info, 0, sizeof(OverlayLineEx));

    info.x1 = x1;
    info.y1 = y1;
    info.x2 = x2;
    info.y2 = y2;
    info.width = width;
	info.color = color;

//	if( x1 < 0 || y1 < 0 || x2 < 0 )
//		return IBSU_ERR_INVALID_PARAM_VALUE;

	if( m_DisplayWindow.bIsSetDisplayWindow )
	{
		if( (pOvHandle = _CreateOverlayHandle()) == NULL )
			return IBSU_ERR_INVALID_OVERLAY_HANDLE;

		info.handle = pOvHandle->handle;
		info.pShow = &pOvHandle->show;
		*pOverlayHandle = pOvHandle->handle;

		OverlayLineEx *pOverlay = NULL;
		pOverlay = new OverlayLineEx;
		memcpy(pOverlay, &info, sizeof(OverlayLineEx));
		m_pOverlayLineEx.push_back(pOverlay);

		_ReDrawClientWindow_Overlays(m_pAlgo->m_OutResultImg, m_DisplayWindow);

		return IBSU_STATUS_OK;
	}
	
	return IBSU_ERR_CLIENT_WINDOW;
}

int CMainCapture::ClientWindow_ModifyOverlayLine(const int overlayHandle, const int x1, const int y1,
                                     const int x2, const int y2, const int width, const DWORD color)
{
    if( !_FindOverlayHandleInList(overlayHandle) )
        return IBSU_ERR_INVALID_OVERLAY_HANDLE;

//	if( x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0 )
//		return IBSU_ERR_INVALID_PARAM_VALUE;

	if( m_DisplayWindow.bIsSetDisplayWindow )
	{
		OverlayLineEx *pOverlay = NULL;
		if( (pOverlay = _FindOverlayLineExInList(overlayHandle)) == NULL )
			return IBSU_ERR_INVALID_OVERLAY_HANDLE;

		pOverlay->x1 = x1;
		pOverlay->y1 = y1;
		pOverlay->x2 = x2;
		pOverlay->y2 = y2;
		pOverlay->width = width;
		pOverlay->color = color;

		_ReDrawClientWindow_Overlays(m_pAlgo->m_OutResultImg, m_DisplayWindow);

		return IBSU_STATUS_OK;
	}
	
	return IBSU_ERR_CLIENT_WINDOW;
}

int CMainCapture::ClientWindow_AddOverlayQuadrangle(int *pOverlayHandle, const int x1, const int y1,
                                     const int x2, const int y2, const int x3, const int y3, 
                                     const int x4, const int y4, const int width, const DWORD color)
{
	OverlayHandle *pOvHandle = NULL;
	OverlayQuadrangleEx info;

	*pOverlayHandle = -1;
	memset(&info, 0, sizeof(OverlayQuadrangleEx));

    info.x1 = x1;
    info.y1 = y1;
    info.x2 = x2;
    info.y2 = y2;
    info.x3 = x3;
    info.y3 = y3;
    info.x4 = x4;
    info.y4 = y4;
    info.width = width;
	info.color = color;

//	if( x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0 || x3 < 0 || y3 < 0 || x4 < 0 || y4 < 0 )
//		return IBSU_ERR_INVALID_PARAM_VALUE;

	if( m_DisplayWindow.bIsSetDisplayWindow )
	{
		if( (pOvHandle = _CreateOverlayHandle()) == NULL )
			return IBSU_ERR_INVALID_OVERLAY_HANDLE;

		info.handle = pOvHandle->handle;
		info.pShow = &pOvHandle->show;
		*pOverlayHandle = pOvHandle->handle;

		OverlayQuadrangleEx *pOverlay = NULL;
		pOverlay = new OverlayQuadrangleEx;
		memcpy(pOverlay, &info, sizeof(OverlayQuadrangleEx));
		m_pOverlayQuadrangleEx.push_back(pOverlay);

		_ReDrawClientWindow_Overlays(m_pAlgo->m_OutResultImg, m_DisplayWindow);

		return IBSU_STATUS_OK;
	}
	
	return IBSU_ERR_CLIENT_WINDOW;
}

int CMainCapture::ClientWindow_ModifyOverlayQuadrangle(const int overlayHandle, const int x1, const int y1,
                                     const int x2, const int y2, const int x3, const int y3, 
                                     const int x4, const int y4, const int width, const DWORD color)
{
    if( !_FindOverlayHandleInList(overlayHandle) )
        return IBSU_ERR_INVALID_OVERLAY_HANDLE;

//	if( x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0 || x3 < 0 || y3 < 0 || x4 < 0 || y4 < 0 )
//		return IBSU_ERR_INVALID_PARAM_VALUE;

	if( m_DisplayWindow.bIsSetDisplayWindow )
	{
		OverlayQuadrangleEx *pOverlay = NULL;
		if( (pOverlay = _FindOverlayQuadrangleExInList(overlayHandle)) == NULL )
			return IBSU_ERR_INVALID_OVERLAY_HANDLE;

		pOverlay->x1 = x1;
		pOverlay->y1 = y1;
		pOverlay->x2 = x2;
		pOverlay->y2 = y2;
		pOverlay->x3 = x3;
		pOverlay->y3 = y3;
		pOverlay->x4 = x4;
		pOverlay->y4 = y4;
		pOverlay->width = width;
		pOverlay->color = color;

		_ReDrawClientWindow_Overlays(m_pAlgo->m_OutResultImg, m_DisplayWindow);

		return IBSU_STATUS_OK;
	}
	
	return IBSU_ERR_CLIENT_WINDOW;
}

int CMainCapture::ClientWindow_AddOverlayShape(int *pOverlayHandle, const IBSU_OverlayShapePattern shapePattern,
							const int x1, const int y1, const int x2, const int y2, const int width, const DWORD color,
							const int reserved_1, const int reserved_2)
{
	OverlayHandle *pOvHandle = NULL;
	OverlayShapeEx info;

	*pOverlayHandle = -1;
	memset(&info, 0, sizeof(OverlayShapeEx));

	info.shapePattern = shapePattern;
    info.x1 = x1;
    info.y1 = y1;
    info.x2 = x2;
    info.y2 = y2;
    info.width = width;
	info.color = color;
	info.reserved_1 = reserved_1;
	info.reserved_2 = reserved_2;

//	if( x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0 )
//		return IBSU_ERR_INVALID_PARAM_VALUE;

	if( m_DisplayWindow.bIsSetDisplayWindow )
	{
		if( (pOvHandle = _CreateOverlayHandle()) == NULL )
			return IBSU_ERR_INVALID_OVERLAY_HANDLE;

		info.handle = pOvHandle->handle;
		info.pShow = &pOvHandle->show;
		*pOverlayHandle = pOvHandle->handle;

		OverlayShapeEx *pOverlay = NULL;
		pOverlay = new OverlayShapeEx;
		memcpy(pOverlay, &info, sizeof(OverlayShapeEx));
		m_pOverlayShapeEx.push_back(pOverlay);

		_ReDrawClientWindow_Overlays(m_pAlgo->m_OutResultImg, m_DisplayWindow);

		return IBSU_STATUS_OK;
	}
	
	return IBSU_ERR_CLIENT_WINDOW;
}

int CMainCapture::ClientWindow_ModifyOverlayShape(const int overlayHandle, const IBSU_OverlayShapePattern shapePattern, 
									 const int x1, const int y1, const int x2, const int y2, const int width, const DWORD color,
									 const int reserved_1, const int reserved_2)
{
    if( !_FindOverlayHandleInList(overlayHandle) )
        return IBSU_ERR_INVALID_OVERLAY_HANDLE;

//	if( x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0  )
//		return IBSU_ERR_INVALID_PARAM_VALUE;

	if( m_DisplayWindow.bIsSetDisplayWindow )
	{
		OverlayShapeEx *pOverlay = NULL;
		if( (pOverlay = _FindOverlayShapeExInList(overlayHandle)) == NULL )
			return IBSU_ERR_INVALID_OVERLAY_HANDLE;

		pOverlay->shapePattern = shapePattern;
		pOverlay->x1 = x1;
		pOverlay->y1 = y1;
		pOverlay->x2 = x2;
		pOverlay->y2 = y2;
		pOverlay->width = width;
		pOverlay->color = color;
		pOverlay->reserved_1 = reserved_1;
		pOverlay->reserved_2 = reserved_2;

		_ReDrawClientWindow_Overlays(m_pAlgo->m_OutResultImg, m_DisplayWindow);

		return IBSU_STATUS_OK;
	}
	
	return IBSU_ERR_CLIENT_WINDOW;
}

int CMainCapture::ClientWindow_Redraw(const DWORD flags)
{
    if (!m_DisplayWindow.bIsSetDisplayWindow)
    {
        return IBSU_ERR_CLIENT_WINDOW_NOT_CREATE;
    }

	std::vector<OverlayText *>dstOverlayText;
	std::vector<OverlayTextEx *>dstOverlayTextEx;
	std::vector<OverlayLineEx *>dstOverlayLineEx;
	std::vector<OverlayQuadrangleEx *>dstOverlayQuadrangleEx;
	std::vector<OverlayShapeEx *>dstOverlayShapeEx;
    std::copy(m_pOverlayText.begin(), m_pOverlayText.end(), std::back_inserter(dstOverlayText));
    std::copy(m_pOverlayTextEx.begin(), m_pOverlayTextEx.end(), std::back_inserter(dstOverlayTextEx));
    std::copy(m_pOverlayLineEx.begin(), m_pOverlayLineEx.end(), std::back_inserter(dstOverlayLineEx));
    std::copy(m_pOverlayQuadrangleEx.begin(), m_pOverlayQuadrangleEx.end(), std::back_inserter(dstOverlayQuadrangleEx));
    std::copy(m_pOverlayShapeEx.begin(), m_pOverlayShapeEx.end(), std::back_inserter(dstOverlayShapeEx));
    if (m_DisplayWindow.keepRedrawLastImage == FALSE)
    {
        memset(m_DisplayWindow.image, 0xFF, m_DisplayWindow.imgWidth*m_DisplayWindow.imgHeight);
    }

	if(m_propertyInfo.bEnableEncryption == TRUE && m_propertyInfo.bEnableViewEncryptionImage == TRUE)
		_DrawClientWindow(m_pAlgo->m_EncryptImage500ppi, m_DisplayWindow, &dstOverlayText, &dstOverlayTextEx, &dstOverlayLineEx, &dstOverlayQuadrangleEx, &dstOverlayShapeEx, flags);
	else
		_DrawClientWindow(m_pAlgo->m_OutResultImg, m_DisplayWindow, &dstOverlayText, &dstOverlayTextEx, &dstOverlayLineEx, &dstOverlayQuadrangleEx, &dstOverlayShapeEx, flags);

	return IBSU_STATUS_OK;
}

void CMainCapture::_DrawClientWindow_Text(HDC hMemDc, DisplayWindow dispWindow, std::vector<OverlayText *>*overlayObjects)
{
	OverlayText *pOverlayText;
	for(int i=0; i<(int)overlayObjects->size(); i++ )
	{
		pOverlayText = overlayObjects->at(i);

		// drawing
		HFONT	font, old_font;
		int		bold = 0;
		RECT	fontRect;

		fontRect.left = pOverlayText->x;
		fontRect.right = dispWindow.Width;
		fontRect.top = pOverlayText->y;
		fontRect.bottom = dispWindow.Height;

		if( pOverlayText->bold )
			bold = FW_BOLD;

		int font_size = -::MulDiv(pOverlayText->size, GetDeviceCaps(hMemDc, LOGPIXELSY), 72);

#ifdef UNICODE
		// Convert Multibyte to Unicode
		wchar_t wcsFont[IBSU_MAX_STR_LEN];
		wchar_t wcsText[IBSU_MAX_STR_LEN];

		mbstowcs( wcsFont, pOverlayText->familyName, IBSU_MAX_STR_LEN );
		*(wcsFont+IBSU_MAX_STR_LEN-1) = L'\0'; // Ensure string is always null terminated
		mbstowcs( wcsText, pOverlayText->text, IBSU_MAX_STR_LEN );
		*(wcsText+IBSU_MAX_STR_LEN-1) = L'\0'; // Ensure string is always null terminated
#if defined(WINCE)
		font = ::CreateFont(font_size, 0, 0, 0, bold, 0, 0, 0,ANSI_CHARSET, 3, 2, 1,
			VARIABLE_PITCH | FF_ROMAN, wcsFont);
#else
		font = ::CreateFontW(font_size, 0, 0, 0, bold, 0, 0, 0,ANSI_CHARSET, 3, 2, 1,
			VARIABLE_PITCH | FF_ROMAN, wcsFont);
#endif

#else
		font = ::CreateFont(font_size, 0, 0, 0, bold, 0, 0, 0,ANSI_CHARSET, 3, 2, 1,
			VARIABLE_PITCH | FF_ROMAN, pOverlayText->familyName);
#endif
		old_font = (HFONT)::SelectObject(hMemDc, font);

		::SetTextColor(hMemDc, (COLORREF)pOverlayText->color);
		::SetBkMode(hMemDc, TRANSPARENT);
#ifdef UNICODE
		::DrawTextW(hMemDc, wcsText, -1, &fontRect, DT_LEFT | DT_NOCLIP);
#else
		::DrawText(hMemDc, pOverlayText->text, -1, &fontRect, DT_LEFT | DT_NOCLIP);
#endif

		::SelectObject(hMemDc, old_font);
		::DeleteObject(font);
	}
}

void CMainCapture::_DrawClientWindow_TextEx(HDC hMemDc, DisplayWindow dispWindow, std::vector<OverlayTextEx*>*overlayObjects)
{
	OverlayTextEx *pOverlay;
	for(int i=0; i<(int)overlayObjects->size(); i++ )
	{
		pOverlay = overlayObjects->at(i);
        if( *(pOverlay->pShow) == FALSE )
            continue;

		// draw TextEx to the memory DC
		HFONT	font, old_font;
		int		bold = 0;
		RECT	fontRect;

		fontRect.left = pOverlay->x;
		fontRect.right = dispWindow.Width;
		fontRect.top = pOverlay->y;
		fontRect.bottom = dispWindow.Height;

		if( pOverlay->bold )
			bold = FW_BOLD;

		int font_size = -::MulDiv(pOverlay->size, GetDeviceCaps(hMemDc, LOGPIXELSY), 72);

#ifdef UNICODE
		// Convert Multibyte to Unicode
		wchar_t wcsFont[IBSU_MAX_STR_LEN];
		wchar_t wcsText[IBSU_MAX_STR_LEN];

		mbstowcs( wcsFont, pOverlay->familyName, IBSU_MAX_STR_LEN );
		*(wcsFont+IBSU_MAX_STR_LEN-1) = L'\0'; // Ensure string is always null terminated
		mbstowcs( wcsText, pOverlay->text, IBSU_MAX_STR_LEN );
		*(wcsText+IBSU_MAX_STR_LEN-1) = L'\0'; // Ensure string is always null terminated

#if defined(WINCE)
		font = ::CreateFont(font_size, 0, 0, 0, bold, 0, 0, 0,ANSI_CHARSET, 3, 2, 1,
			VARIABLE_PITCH | FF_ROMAN, wcsFont);
#else
		font = ::CreateFontW(font_size, 0, 0, 0, bold, 0, 0, 0,ANSI_CHARSET, 3, 2, 1,
			VARIABLE_PITCH | FF_ROMAN, wcsFont);
#endif

#else
		font = ::CreateFont(font_size, 0, 0, 0, bold, 0, 0, 0,ANSI_CHARSET, 3, 2, 1,
			VARIABLE_PITCH | FF_ROMAN, pOverlay->familyName);
#endif
		old_font = (HFONT)::SelectObject(hMemDc, font);

		::SetTextColor(hMemDc, (COLORREF)pOverlay->color);
		::SetBkMode(hMemDc, TRANSPARENT);

#ifdef UNICODE
		::DrawTextW(hMemDc, wcsText, -1, &fontRect, DT_LEFT | DT_NOCLIP);
#else
#ifdef __G_PRINT_DAC_VALUE__
		if(	m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0 ||
			m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK)
		{
			char Text[256];
			if(strlen(pOverlay->text) > 0)
				sprintf(Text, "%s\nDAC(0x%X), ", pOverlay->text, m_CurrentCaptureDAC);
			else
				sprintf(Text, "%s, ", pOverlay->text);

/*			sprintf(Text, "%s\nWet Ratio [ %d, %d, %d, %d ] Wet Thres [ %s ]\n", Text, 
				m_UsbDeviceInfo.CapturedWetRatio[0], 
				m_UsbDeviceInfo.CapturedWetRatio[1], 
				m_UsbDeviceInfo.CapturedWetRatio[2], 
				m_UsbDeviceInfo.CapturedWetRatio[3],
				(m_propertyInfo.nWetFingerDetectLevel == 1)?"15":
				(m_propertyInfo.nWetFingerDetectLevel == 2)?"12":
				(m_propertyInfo.nWetFingerDetectLevel == 3)?"9":
				(m_propertyInfo.nWetFingerDetectLevel == 4)?"6":
				(m_propertyInfo.nWetFingerDetectLevel == 5)?"3":"100");
*/
			sprintf(Text, "%s  Analog ( PlateOrg : %d ==> Plate : %d, FilmOrg : %d ==> Film : %d )", Text,
							m_nAnalogTouch_PlateOrg, m_nAnalogTouch_Plate, 
							m_nAnalogTouch_FilmOrg, m_nAnalogTouch_Film);

			::DrawText(hMemDc, Text, -1, &fontRect, DT_LEFT | DT_NOCLIP);
		}
		else if(m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
		{
			char Text[256];
			if(strlen(pOverlay->text) > 0)
				sprintf(Text, "%s\nDAC(0x%X), ", pOverlay->text, m_CurrentCaptureDAC);
			else
				sprintf(Text, "%s, ", pOverlay->text);

/*			sprintf(Text, "%s\nWet Ratio [ %d, %d, %d, %d ] Wet Thres [ %s ]\n", Text, 
				m_UsbDeviceInfo.CapturedWetRatio[0], 
				m_UsbDeviceInfo.CapturedWetRatio[1], 
				m_UsbDeviceInfo.CapturedWetRatio[2], 
				m_UsbDeviceInfo.CapturedWetRatio[3],
				(m_propertyInfo.nWetFingerDetectLevel == 1)?"15":
				(m_propertyInfo.nWetFingerDetectLevel == 2)?"12":
				(m_propertyInfo.nWetFingerDetectLevel == 3)?"9":
				(m_propertyInfo.nWetFingerDetectLevel == 4)?"6":
				(m_propertyInfo.nWetFingerDetectLevel == 5)?"3":"100");
*/
//			sprintf(Text, "%s  Bright ( %d [%d] ), Current ( %d ), Max ( 0x%X,%d )", Text, m_Brightness, 
//				m_pAlgo->m_SBDAlg->m_SegmentBright, m_TOF_CurVal, m_TOF_MaxVal_DAC, m_TOF_MaxVal );
			//sprintf(Text, "%s  Current (%x = %d), Analog (%x = %d)", Text, m_TOF_CurVal, m_TOF_CurVal, m_TOF_Analog, m_TOF_Analog);
			sprintf(Text, "%s  Current (%x = %d)\n\r, Analog (%x = %d), Analog2 (%x = %d)\n\r", Text, 
				m_TOF_CurVal, m_TOF_CurVal, m_TOF_Analog, m_TOF_Analog, m_TOF_Analog2, m_TOF_Analog2);

			::DrawText(hMemDc, Text, -1, &fontRect, DT_LEFT | DT_NOCLIP);
		}
		else
			::DrawText(hMemDc, pOverlay->text, -1, &fontRect, DT_LEFT | DT_NOCLIP);
#else
		::DrawText(hMemDc, pOverlay->text, -1, &fontRect, DT_LEFT | DT_NOCLIP);
#endif
#endif

		::SelectObject(hMemDc, old_font);
		::DeleteObject(font);
	}
}

void CMainCapture::_DrawClientWindow_LineEx(HDC hMemDc, DisplayWindow dispWindow, std::vector<OverlayLineEx*>*overlayObjects)
{
	OverlayLineEx *pOverlay;
	for(int i=0; i<(int)overlayObjects->size(); i++ )
	{
		pOverlay = overlayObjects->at(i);
        if( *(pOverlay->pShow) == FALSE )
            continue;

		// draw LineEx to the memory DC
		HPEN	pen, old_pen;
        LOGPEN  logpen;
        POINT   point;

        point.x = pOverlay->width;
        logpen.lopnStyle = PS_SOLID;
        logpen.lopnWidth = point;
        logpen.lopnColor = (COLORREF)pOverlay->color;
		pen = ::CreatePenIndirect(&logpen);
		old_pen = (HPEN)::SelectObject(hMemDc, pen);

		::SetBkMode(hMemDc, TRANSPARENT);
        ::MoveToEx(hMemDc, pOverlay->x1, pOverlay->y1, NULL);
		::LineTo(hMemDc, pOverlay->x2, pOverlay->y2);

		::SelectObject(hMemDc, old_pen);
		::DeleteObject(pen);
	}
}

void CMainCapture::_DrawClientWindow_QuadrangleEx(HDC hMemDc, DisplayWindow dispWindow, std::vector<OverlayQuadrangleEx*>*overlayObjects)
{
	OverlayQuadrangleEx *pOverlay;
/*    int x1,x2,x3,x4,y1,y2,y3,y4;
    double scaleFactor = (double)m_DisplayWindow.imgWidth / m_UsbDeviceInfo.ImgWidth;
    int leftMargin = m_DisplayWindow.imgX;
    int topMargin = m_DisplayWindow.imgY;

    for(int i=0; i<(int)m_pAlgo->m_segment_arr.SegmentCnt; i++ )
	{
		// draw QuadrangleEx to the memory DC
		HPEN	pen, old_pen;
        LOGPEN  logpen;
        POINT   point;

        point.x = 1;
        logpen.lopnStyle = PS_SOLID;
        logpen.lopnWidth = point;
        logpen.lopnColor = (COLORREF)RGB(0, 0, 255);
		pen = ::CreatePenIndirect(&logpen);
		old_pen = (HPEN)::SelectObject(hMemDc, pen);

		x1 = (m_pAlgo->m_segment_arr.Segment[i].P1_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
		y1 = (m_pAlgo->m_segment_arr.Segment[i].P1_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

		x2 = (m_pAlgo->m_segment_arr.Segment[i].P2_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
		y2 = (m_pAlgo->m_segment_arr.Segment[i].P2_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

		x3 = (m_pAlgo->m_segment_arr.Segment[i].P3_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
		y3 = (m_pAlgo->m_segment_arr.Segment[i].P3_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

		x4 = (m_pAlgo->m_segment_arr.Segment[i].P4_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
		y4 = (m_pAlgo->m_segment_arr.Segment[i].P4_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

		y1 = (m_pAlgo->IMG_H - y1);
		y2 = (m_pAlgo->IMG_H - y2);
		y3 = (m_pAlgo->IMG_H - y3);
		y4 = (m_pAlgo->IMG_H - y4);

		x1 = leftMargin + (int)(x1*scaleFactor);
		x2 = leftMargin + (int)(x2*scaleFactor);
		x3 = leftMargin + (int)(x3*scaleFactor);
		x4 = leftMargin + (int)(x4*scaleFactor);
		y1 = topMargin +  (int)(y1*scaleFactor);
		y2 = topMargin +  (int)(y2*scaleFactor);
		y3 = topMargin +  (int)(y3*scaleFactor);
		y4 = topMargin +  (int)(y4*scaleFactor);

        ::SetBkMode(hMemDc, TRANSPARENT);
        ::MoveToEx(hMemDc, x1, y1, NULL);
		::LineTo(hMemDc, x2, y2);

        ::MoveToEx(hMemDc, x2, y2, NULL);
		::LineTo(hMemDc, x3, y3);

        ::MoveToEx(hMemDc, x3, y3, NULL);
		::LineTo(hMemDc, x4, y4);

        ::MoveToEx(hMemDc, x4, y4, NULL);
		::LineTo(hMemDc, x1, y1);

		::SelectObject(hMemDc, old_pen);
		::DeleteObject(pen);
	}
*/
    for(int i=0; i<(int)overlayObjects->size(); i++ )
	{
		pOverlay = overlayObjects->at(i);
        if( *(pOverlay->pShow) == FALSE )
            continue;

		// draw QuadrangleEx to the memory DC
		HPEN	pen, old_pen;
        LOGPEN  logpen;
        POINT   point;

        point.x = pOverlay->width;
        logpen.lopnStyle = PS_SOLID;
        logpen.lopnWidth = point;
        logpen.lopnColor = (COLORREF)pOverlay->color;
		pen = ::CreatePenIndirect(&logpen);
		old_pen = (HPEN)::SelectObject(hMemDc, pen);

		::SetBkMode(hMemDc, TRANSPARENT);
        ::MoveToEx(hMemDc, pOverlay->x1, pOverlay->y1, NULL);
		::LineTo(hMemDc, pOverlay->x2, pOverlay->y2);

        ::MoveToEx(hMemDc, pOverlay->x2, pOverlay->y2, NULL);
		::LineTo(hMemDc, pOverlay->x3, pOverlay->y3);

        ::MoveToEx(hMemDc, pOverlay->x3, pOverlay->y3, NULL);
		::LineTo(hMemDc, pOverlay->x4, pOverlay->y4);

        ::MoveToEx(hMemDc, pOverlay->x4, pOverlay->y4, NULL);
		::LineTo(hMemDc, pOverlay->x1, pOverlay->y1);

		::SelectObject(hMemDc, old_pen);
		::DeleteObject(pen);
	}
}

void CMainCapture::_DrawClientWindow_ShapeEx(HDC hMemDc, DisplayWindow dispWindow, std::vector<OverlayShapeEx*>*overlayObjects)
{
	OverlayShapeEx *pOverlay;
	for(int i=0; i<(int)overlayObjects->size(); i++ )
	{
		pOverlay = overlayObjects->at(i);
        if( *(pOverlay->pShow) == FALSE )
            continue;

		// draw Shape to the memory DC
		HPEN	pen, old_pen;
        LOGPEN  logpen;
        POINT   point;
		HBRUSH	old_brush;

        point.x = pOverlay->width;
        logpen.lopnStyle = PS_SOLID;
        logpen.lopnWidth = point;
        logpen.lopnColor = (COLORREF)pOverlay->color;
		pen = ::CreatePenIndirect(&logpen);
		old_pen = (HPEN)::SelectObject(hMemDc, pen);

		::SetBkMode(hMemDc, TRANSPARENT);

		switch( pOverlay->shapePattern )
		{
		case ENUM_IBSU_OVERLAY_SHAPE_RECTANGLE:
			old_brush = (HBRUSH)::SelectObject(hMemDc, GetStockObject(NULL_BRUSH));
			::Rectangle(hMemDc, pOverlay->x1, pOverlay->y1, pOverlay->x2, pOverlay->y2);
			::SelectObject(hMemDc, old_brush);
			break;
		case ENUM_IBSU_OVERLAY_SHAPE_ELLIPSE:
			old_brush = (HBRUSH)::SelectObject(hMemDc, GetStockObject(NULL_BRUSH));
			::Ellipse(hMemDc, pOverlay->x1, pOverlay->y1, pOverlay->x2, pOverlay->y2);
			::SelectObject(hMemDc, old_brush);
			break;
		case ENUM_IBSU_OVERLAY_SHAPE_CROSS:
			{
				int width = abs(pOverlay->x2 - pOverlay->x1);
				int height = abs(pOverlay->y2 - pOverlay->y1);
				::MoveToEx(hMemDc, pOverlay->x1, height/2, NULL);
				::LineTo(hMemDc, pOverlay->x2, height/2);

				::MoveToEx(hMemDc, width/2, pOverlay->y1, NULL);
				::LineTo(hMemDc, width/2, pOverlay->y2);
			}
			break;
		case ENUM_IBSU_OVERLAY_SHAPE_ARROW:
			{
				HBRUSH brush;
				POINT pFrom;
				POINT pBase;
				POINT aptPoly[3];
				float vecLine[2];
				float vecLeft[2];
				float fLength;
				float th;
				float ta;
				float theta;
				int arrow_width;

				if( pOverlay->x1 == pOverlay->x2 && pOverlay->y1 == pOverlay->y2 )
					break;

				brush = ::CreateSolidBrush((COLORREF)pOverlay->color);
				old_brush = (HBRUSH)::SelectObject(hMemDc, brush);

				// Set to point of the line start
				pFrom.x = pOverlay->x1;
				pFrom.y = pOverlay->y1;

				// Set to point of the line end
				aptPoly[0].x = pOverlay->x2;
				aptPoly[0].y = pOverlay->y2;

				// Build the line vector
				vecLine[0] = (float) aptPoly[0].x - pFrom.x;
				vecLine[1] = (float) aptPoly[0].y - pFrom.y;

				// Build the arrow base vector - normal to the line
				vecLeft[0] = -vecLine[1];
				vecLeft[1] = vecLine[0];

				// Setup length parameters
				fLength = (float) sqrt(vecLine[0] * vecLine[0] + vecLine[1] * vecLine[1]);
				theta = (float)pOverlay->reserved_2/100.0f;
				arrow_width = pOverlay->reserved_1;
				th = arrow_width / (2.0f * fLength);
				ta = arrow_width / (2.0f * (tanf(theta) / 2.0f) * fLength);

				// find the base of the arrow
				pBase.x = (int) (aptPoly[0].x + -ta * vecLine[0]);
				pBase.y = (int) (aptPoly[0].y + -ta * vecLine[1]);

				// build the points on the sides of the arrow
				aptPoly[1].x = (int) (pBase.x + th * vecLeft[0]);
				aptPoly[1].y = (int) (pBase.y + th * vecLeft[1]);
				aptPoly[2].x = (int) (pBase.x + -th * vecLeft[0]);
				aptPoly[2].y = (int) (pBase.y + -th * vecLeft[1]);

				::MoveToEx(hMemDc, pFrom.x, pFrom.y, NULL);
				::LineTo(hMemDc, aptPoly[0].x, aptPoly[0].y);
				::Polygon(hMemDc, aptPoly, 3);
				
				::SelectObject(hMemDc, old_brush);
				::DeleteObject(brush);
			}
			break;
		default:
			break;
		}

		::SelectObject(hMemDc, old_pen);
		::DeleteObject(pen);
	}
}

void CMainCapture::_DrawClientWindow_DispInvalidArea(HDC hMemDc, DisplayWindow dispWindow)
{
	if( dispWindow.dispInvalidArea )
	{
		HPEN	pen, old_pen;
        LOGPEN  logpen;
        POINT   point;
		int		invalid_area_bottom_pos_x1 = 0, invalid_area_bottom_pos_x2 = 0, invalid_area_bottom_pos_y1 = 0, invalid_area_bottom_pos_y2 = 0;
		int		cis_img_w = m_pAlgo->CIS_IMG_W;
		int		cis_img_h = m_pAlgo->CIS_IMG_H;
		int		invalid_rate = 0;

		const int arrow_width = 20;
		const int arrow_thickness = 10;
		const int arrow_pos = 30;
		const int arrow_gab = 5;

        point.x = 1;
        logpen.lopnStyle = PS_SOLID;
        logpen.lopnWidth = point;
        logpen.lopnColor = (COLORREF)RGB(255,0,0);
		pen = ::CreatePenIndirect(&logpen);
		old_pen = (HPEN)::SelectObject(hMemDc, pen);

		::SetBkMode(hMemDc, TRANSPARENT);

		for( int i=0; i<4; i++ )
		{
			if( m_clbkProperty.qualityArray[i] == ENUM_IBSU_QUALITY_INVALID_AREA_TOP )
			{
				HBRUSH brush, old_brush;
				POINT aptPoly[4];

				brush = ::CreateSolidBrush((COLORREF)RGB(255,0,0));
				old_brush = (HBRUSH)::SelectObject(hMemDc, brush);

				aptPoly[0].x = dispWindow.Width/2;
				aptPoly[0].y = arrow_pos;
				aptPoly[1].x = aptPoly[0].x + arrow_width;
				aptPoly[1].y = aptPoly[0].y - arrow_width;
				aptPoly[2].x = aptPoly[0].x;
				aptPoly[2].y = aptPoly[0].y - arrow_thickness;
				aptPoly[3].x = aptPoly[0].x - arrow_width;
				aptPoly[3].y = aptPoly[0].y - arrow_width;
				::Polygon(hMemDc, aptPoly, 4);
				
				aptPoly[0].x = dispWindow.Width/2;
				aptPoly[0].y = arrow_pos+arrow_thickness+arrow_gab;
				aptPoly[1].x = aptPoly[0].x + arrow_width;
				aptPoly[1].y = aptPoly[0].y - arrow_width;
				aptPoly[2].x = aptPoly[0].x;
				aptPoly[2].y = aptPoly[0].y - arrow_thickness;
				aptPoly[3].x = aptPoly[0].x - arrow_width;
				aptPoly[3].y = aptPoly[0].y - arrow_width;
				::Polygon(hMemDc, aptPoly, 4);

				::SelectObject(hMemDc, old_brush);
				::DeleteObject(brush);
			}

			if( m_clbkProperty.qualityArray[i] == ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM )
			{
				HBRUSH brush, old_brush;
				POINT aptPoly[4];

				brush = ::CreateSolidBrush((COLORREF)RGB(255,0,0));
				old_brush = (HBRUSH)::SelectObject(hMemDc, brush);

				aptPoly[0].x = dispWindow.Width/2;
				aptPoly[0].y = dispWindow.Height - arrow_pos;
				aptPoly[1].x = aptPoly[0].x - arrow_width;
				aptPoly[1].y = aptPoly[0].y + arrow_width;
				aptPoly[2].x = aptPoly[0].x;
				aptPoly[2].y = aptPoly[0].y + arrow_thickness;
				aptPoly[3].x = aptPoly[0].x + arrow_width;
				aptPoly[3].y = aptPoly[0].y + arrow_width;
				::Polygon(hMemDc, aptPoly, 4);
				
				aptPoly[0].x = dispWindow.Width/2;
				aptPoly[0].y = dispWindow.Height - arrow_thickness - arrow_gab - arrow_pos;//arrow_pos+arrow_thickness+arrow_gab;
				aptPoly[1].x = aptPoly[0].x - arrow_width;
				aptPoly[1].y = aptPoly[0].y + arrow_width;
				aptPoly[2].x = aptPoly[0].x;
				aptPoly[2].y = aptPoly[0].y + arrow_thickness;
				aptPoly[3].x = aptPoly[0].x + arrow_width;
				aptPoly[3].y = aptPoly[0].y + arrow_width;
				::Polygon(hMemDc, aptPoly, 4);

				::SelectObject(hMemDc, old_brush);
				::DeleteObject(brush);
			}

			if( m_clbkProperty.qualityArray[i] == ENUM_IBSU_QUALITY_INVALID_AREA_LEFT )
			{
				HBRUSH brush, old_brush;
				POINT aptPoly[4];

				brush = ::CreateSolidBrush((COLORREF)RGB(255,0,0));
				old_brush = (HBRUSH)::SelectObject(hMemDc, brush);

				aptPoly[0].x = arrow_pos;
				aptPoly[0].y = dispWindow.Height/2;
				aptPoly[1].x = aptPoly[0].x - arrow_width;
				aptPoly[1].y = aptPoly[0].y + arrow_width;
				aptPoly[2].x = aptPoly[0].x - arrow_thickness;
				aptPoly[2].y = aptPoly[0].y;
				aptPoly[3].x = aptPoly[0].x - arrow_width;
				aptPoly[3].y = aptPoly[0].y - arrow_width;
				::Polygon(hMemDc, aptPoly, 4);
				
				aptPoly[0].x = arrow_pos+arrow_thickness+arrow_gab;
				aptPoly[0].y = dispWindow.Height/2;
				aptPoly[1].x = aptPoly[0].x - arrow_width;
				aptPoly[1].y = aptPoly[0].y + arrow_width;
				aptPoly[2].x = aptPoly[0].x - arrow_thickness;
				aptPoly[2].y = aptPoly[0].y;
				aptPoly[3].x = aptPoly[0].x - arrow_width;
				aptPoly[3].y = aptPoly[0].y - arrow_width;
				::Polygon(hMemDc, aptPoly, 4);

				::SelectObject(hMemDc, old_brush);
				::DeleteObject(brush);
			}

			if( m_clbkProperty.qualityArray[i] == ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT )
			{
				HBRUSH brush, old_brush;
				POINT aptPoly[4];

				brush = ::CreateSolidBrush((COLORREF)RGB(255,0,0));
				old_brush = (HBRUSH)::SelectObject(hMemDc, brush);

				aptPoly[0].x = dispWindow.Width - arrow_pos;
				aptPoly[0].y = dispWindow.Height/2;
				aptPoly[1].x = aptPoly[0].x + arrow_width;
				aptPoly[1].y = aptPoly[0].y + arrow_width;
				aptPoly[2].x = aptPoly[0].x + arrow_thickness;
				aptPoly[2].y = aptPoly[0].y;
				aptPoly[3].x = aptPoly[0].x + arrow_width;
				aptPoly[3].y = aptPoly[0].y - arrow_width;
				::Polygon(hMemDc, aptPoly, 4);
				
				aptPoly[0].x = dispWindow.Width - arrow_pos-arrow_thickness-arrow_gab;
				aptPoly[0].y = dispWindow.Height/2;
				aptPoly[1].x = aptPoly[0].x + arrow_width;
				aptPoly[1].y = aptPoly[0].y + arrow_width;
				aptPoly[2].x = aptPoly[0].x + arrow_thickness;
				aptPoly[2].y = aptPoly[0].y;
				aptPoly[3].x = aptPoly[0].x + arrow_width;
				aptPoly[3].y = aptPoly[0].y - arrow_width;
				::Polygon(hMemDc, aptPoly, 4);

				::SelectObject(hMemDc, old_brush);
				::DeleteObject(brush);
			}
		}

		::SelectObject(hMemDc, old_pen);
		::DeleteObject(pen);
	}
}

void CMainCapture::_DrawClientWindow_RollGuideLine(HDC hMemDc, DisplayWindow dispWindow)
{
	if( dispWindow.rollGuideLine )
	{
		if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && 
			(m_pAlgo->m_cImgAnalysis.finger_count > 0 || m_pAlgo->g_Rolling_Saved_Complete == TRUE ) &&
			(m_pAlgo->m_rollingStatus == 1 || m_pAlgo->m_rollingStatus == 2) )
		{
			HPEN	pen, old_pen;

			// Guide line for rolling
			if( m_pAlgo->m_rollingStatus == 1 )
				pen = ::CreatePen(PS_SOLID, m_DisplayWindow.rollGuideLineWidth, RGB(255, 0, 0));
			else if( m_pAlgo->m_rollingStatus == 2 )
				pen = ::CreatePen(PS_SOLID, m_DisplayWindow.rollGuideLineWidth, RGB(0, 255, 0));
			old_pen = (HPEN)::SelectObject(hMemDc, pen);

			if(m_pAlgo->g_LastX > -1)
			{
//				int startx = (dispWindow.Width-dispWindow.imgWidth_Roll)/2;
//				int starty = (dispWindow.Height-dispWindow.imgHeight_Roll)/2;
                int startx = dispWindow.imgX_Roll;
                int starty = dispWindow.imgY_Roll;
				if (starty < 1) starty = 1;

				::MoveToEx(hMemDc, m_pAlgo->g_LastX*dispWindow.imgWidth_Roll/m_UsbDeviceInfo.CisImgWidth_Roll + startx, starty, NULL);
				::LineTo(hMemDc, m_pAlgo->g_LastX*dispWindow.imgWidth_Roll/m_UsbDeviceInfo.CisImgWidth_Roll + startx, starty+dispWindow.imgHeight_Roll-1);
			}

			::SelectObject(hMemDc, old_pen);
			::DeleteObject(pen);
		}
		else if( m_pAlgo->m_cImgAnalysis.finger_count == 0 )
		{
			if(m_pAlgo->g_Rolling_Saved_Complete == FALSE)
				m_pAlgo->m_rollingStatus = 0;
		}
	}
}

void CMainCapture::_ReDrawClientWindow_Overlays(BYTE *InImg, DisplayWindow dispWindow)
{/*
#ifdef __G_DEBUG__
		TRACE("Execute _ReDrawClientWindow_Overlays() = %d\n", GetIsActiveCapture());
#endif*/
	if( !GetIsActiveCapture() )
	{
		std::vector<OverlayText *>dstOverlayText;
		std::vector<OverlayTextEx *>dstOverlayTextEx;
		std::vector<OverlayLineEx *>dstOverlayLineEx;
		std::vector<OverlayQuadrangleEx *>dstOverlayQuadrangleEx;
		std::vector<OverlayShapeEx *>dstOverlayShapeEx;
		std::copy(m_pOverlayText.begin(), m_pOverlayText.end(), std::back_inserter(dstOverlayText));
		std::copy(m_pOverlayTextEx.begin(), m_pOverlayTextEx.end(), std::back_inserter(dstOverlayTextEx));
		std::copy(m_pOverlayLineEx.begin(), m_pOverlayLineEx.end(), std::back_inserter(dstOverlayLineEx));
		std::copy(m_pOverlayQuadrangleEx.begin(), m_pOverlayQuadrangleEx.end(), std::back_inserter(dstOverlayQuadrangleEx));
		std::copy(m_pOverlayShapeEx.begin(), m_pOverlayShapeEx.end(), std::back_inserter(dstOverlayShapeEx));
		_DrawClientWindow(InImg, dispWindow, &dstOverlayText, &dstOverlayTextEx, &dstOverlayLineEx, &dstOverlayQuadrangleEx, &dstOverlayShapeEx, 0xFFFFFFFF);
	}
}
