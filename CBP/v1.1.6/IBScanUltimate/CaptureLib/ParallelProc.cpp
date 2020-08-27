////////////////////////////////////////////////////////////////////////////
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
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef __libusb_latest__
#include <libusb.h>
#else
#include <usb.h>
#endif
#include "IBSU_Global_Def.h"
#include "MainCapture.h"
#ifdef __android__
#include "IBScanUltimateApi.h"
#include "IBScanUltimateDLL.h"
#else
#include "../../include/IBScanUltimateApi.h"
#include "../IBScanUltimateDLL.h"
#endif
#include <math.h>
#include "memwatch.h"
#include <sys/time.h>

#include "Parallel.h"
#include "device_parallelApi.h"
#include "device_parallelApi_def.h"
#include "device_parallelApi_err.h"


//#define __BULK_TIMEOUT__ 3000  // Milliseconds

/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Implementation

void *CMainCapture::_InitializeThreadCallback(LPVOID pParam)
{
    if (pParam == NULL)
    {
        return NULL;
    }

    ThreadParam	*pThreadParam  = reinterpret_cast<ThreadParam *>(pParam);
    CMainCapture	*pOwner = (CMainCapture *)pThreadParam->pMainCapture;
    int				count = 0;

    //	SetEvent( pThreadParam->threadStarted );

    pOwner->_GoJob_Initialize_ForThread();

    while (TRUE)
    {
        //		if( pOwner->m_clbkParam[CALLBACK_INIT_PROGRESS].bIsReadySend )
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

    pThreadParam->threadStarted = TRUE;

    //	return NULL;
    pthread_exit((void *) 0);
}

void *CMainCapture::_CaptureThreadCallback(LPVOID pParam)
{
    if (pParam == NULL)
    {
        return NULL;
    }

    ThreadParam	*pThreadParam  = reinterpret_cast<ThreadParam *>(pParam);
    CMainCapture	*pOwner = (CMainCapture *)pThreadParam->pMainCapture;
    BOOL			bUnexpectedExit = FALSE;
    int				nUnexpectedCount = 0;
    int				nRc;
    BOOL			bExitThread = FALSE;

    pThreadParam->threadStarted = TRUE;
    pOwner->SetIsActiveStateOfCaptureThread(FALSE/*Capture*/, FALSE/*Dummy*/, FALSE/*Idle*/);

    while (1)
    {
        pOwner->m_bAliveCaptureThread = TRUE;

        if (pOwner->m_bCaptureThread_DestroyEvent)
        {
            break;
        }
        else if (pOwner->m_bCaptureThread_CaptureEvent)
        {
            pOwner->SetIsActiveStateOfCaptureThread(TRUE/*Capture*/, FALSE/*Dummy*/, FALSE/*Idle*/);

            nRc = pOwner->_GoJob_PreviewImage_ForThread(pOwner->m_pAlgo->m_ImgFromCIS);
            if (nRc == ACUISITION_ERROR)
            {
                // 20111104 enzyme add - If cypress is alive, you can try to capture again! (Just for ESD)
                nUnexpectedCount++;
                bUnexpectedExit = pOwner->_Go_Job_UnexpectedTermination_fromCaptureThread(nUnexpectedCount);
                if (bUnexpectedExit)
                {
                    pOwner->m_bCaptureThread_CaptureEvent = FALSE;
#if defined(__IBSCAN_SDK__)
        			pOwner->m_bRawCaptureThread_GetOneFrameImageEvent = FALSE;
        			pOwner->m_bRawCaptureThread_TakePreviewImageEvent = FALSE;
        			pOwner->m_bRawCaptureThread_TakeResultImageEvent = FALSE;
#endif
                    pOwner->m_bCaptureThread_DummyEvent = FALSE;
                    pOwner->m_bCaptureThread_DestroyEvent = FALSE;
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
                pOwner->m_bCaptureThread_CaptureEvent = FALSE;
#if defined(__IBSCAN_SDK__)
    			pOwner->m_bRawCaptureThread_GetOneFrameImageEvent = FALSE;
    			pOwner->m_bRawCaptureThread_TakePreviewImageEvent = FALSE;
    			pOwner->m_bRawCaptureThread_TakeResultImageEvent = FALSE;
#endif
                pOwner->m_bCaptureThread_DummyEvent = TRUE;
            }
        }
        else if (pOwner->m_bCaptureThread_DummyEvent)
        {
            pOwner->SetIsActiveStateOfCaptureThread(FALSE/*Capture*/, TRUE/*Dummy*/, FALSE/*Idle*/);

            if (pOwner->m_bCaptureThread_DummyStopMessage)
            {
                pOwner->SetIsActiveStateOfCaptureThread(FALSE/*Capture*/, FALSE/*Dummy*/, FALSE/*Idle*/);
                pOwner->m_bCaptureThread_DummyEvent = FALSE;
            }
            else
            {
                // If CALLBACK_CLEAR_PLATEN is not registerd,
                // then the check for finger removal prior to new capture is not performed
                if (pOwner->m_clbkParam[CALLBACK_CLEAR_PLATEN].callback)
                {
                    nRc = pOwner->_GoJob_DummyCapture_ForThread(pOwner->m_pAlgo->m_ImgFromCIS);
                }
                else
                {
                    nRc = 1;    // does not check clear platen
                }

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
                    pOwner->m_bCaptureThread_DummyEvent = FALSE;

                    // We had a turn off of transformer to increase the life of LE film
                    pOwner->_SetLEOperationMode(ADDRESS_LE_OFF_MODE);

                    if (pOwner->m_propertyInfo.bEnablePowerSaveMode == TRUE)
                    {
                        pOwner->_SndUsbFwCaptureStop();
                    }

                    pOwner->SetIsActiveStateOfCaptureThread(FALSE/*Capture*/, FALSE/*Dummy*/, FALSE/*Idle*/);
                }
            }
        }
#if defined(__IBSCAN_SDK__)
		else if( pOwner->m_bRawCaptureThread_GetOneFrameImageEvent )
		{
			// Get one frame image
			pOwner->m_bRawCaptureThread_GetOneFrameImageEvent = FALSE;
            pOwner->SetIsActiveStateOfCaptureThread(TRUE/*Capture*/, FALSE/*Dummy*/, FALSE/*Idle*/);
			nRc = pOwner->_GoJob_OneFrameRawImage_ForThread(pOwner->m_pAlgo->m_ImgFromCIS);
			if( nRc == ACUISITION_ERROR )
			{
				// 20111104 enzyme add - If cypress is alive, you can try to capture again! (Just for ESD)
				nUnexpectedCount++;
				bUnexpectedExit = pOwner->_Go_Job_UnexpectedTermination_fromCaptureThread(nUnexpectedCount);
				if( bUnexpectedExit )
				{
					pOwner->m_bCaptureThread_CaptureEvent = FALSE;
        			pOwner->m_bRawCaptureThread_GetOneFrameImageEvent = FALSE;
        			pOwner->m_bRawCaptureThread_TakePreviewImageEvent = FALSE;
        			pOwner->m_bRawCaptureThread_TakeResultImageEvent = FALSE;
                    pOwner->m_bCaptureThread_DummyEvent = FALSE;
					pOwner->m_bCaptureThread_DestroyEvent = FALSE;
					pOwner->_PostCallback(CALLBACK_NOTIFY_MESSAGE, 0, 0, 0, 0, IBSU_ERR_CHANNEL_IO_UNEXPECTED_FAILED);
                    pOwner->_PostTraceLogCallback(IBSU_ERR_CHANNEL_IO_UNEXPECTED_FAILED, "Unexpected communiation failed");
					break;
				}
			}
			else if( nRc == ACUISITION_NOT_COMPLETE )
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
                pOwner->m_bCaptureThread_CaptureEvent = FALSE;
    			pOwner->m_bRawCaptureThread_GetOneFrameImageEvent = FALSE;
    			pOwner->m_bRawCaptureThread_TakePreviewImageEvent = FALSE;
    			pOwner->m_bRawCaptureThread_TakeResultImageEvent = FALSE;
                pOwner->m_bCaptureThread_DummyEvent = TRUE;
			}
		}
		else if( pOwner->m_bRawCaptureThread_TakePreviewImageEvent )
		{
			// Image processing for previewimage
			pOwner->m_bRawCaptureThread_TakePreviewImageEvent = FALSE;
			nRc = pOwner->_GoJob_TakePreviewImage_ForThread(pOwner->m_pAlgo->m_GetImgFromApp);
            if (nRc == ACUISITION_ABORT)
            {
                pOwner->m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_CLEARD;

                pOwner->m_bCaptureThread_CaptureEvent = FALSE;
    			pOwner->m_bRawCaptureThread_GetOneFrameImageEvent = FALSE;
    			pOwner->m_bRawCaptureThread_TakePreviewImageEvent = FALSE;
    			pOwner->m_bRawCaptureThread_TakeResultImageEvent = FALSE;
                pOwner->m_bCaptureThread_DummyEvent = TRUE;
            }
        }
		else if( pOwner->m_bRawCaptureThread_TakeResultImageEvent )
		{
			// Image processing for resultimage
			pOwner->m_bRawCaptureThread_TakeResultImageEvent = FALSE;
			nRc = pOwner->_GoJob_TakeResultImage_ForThread(pOwner->m_pAlgo->m_GetImgFromApp);
            if (nRc == ACUISITION_ABORT)
            {
                pOwner->m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_CLEARD;

                pOwner->m_bCaptureThread_CaptureEvent = FALSE;
    			pOwner->m_bRawCaptureThread_GetOneFrameImageEvent = FALSE;
    			pOwner->m_bRawCaptureThread_TakePreviewImageEvent = FALSE;
    			pOwner->m_bRawCaptureThread_TakeResultImageEvent = FALSE;
                pOwner->m_bCaptureThread_DummyEvent = TRUE;
            }
		}
		else if( pOwner->m_bRawCaptureThread_AbortEvent )
		{
			pOwner->m_bRawCaptureThread_GetOneFrameImageEvent = FALSE;
			pOwner->m_bRawCaptureThread_TakePreviewImageEvent = FALSE;
			pOwner->m_bRawCaptureThread_TakeResultImageEvent = FALSE;
			pOwner->m_bRawCaptureThread_AbortEvent = FALSE;
			
			pOwner->SetIsActiveStateOfCaptureThread(FALSE/*Capture*/, FALSE/*Dummy*/, FALSE/*Idle*/);
		}
#endif
        else
        {
            pOwner->SetIsActiveStateOfCaptureThread(FALSE/*Capture*/, FALSE/*Dummy*/, TRUE/*Idle*/);
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
#ifdef __G_DEBUG__
        printf("Unexpected communication break\n");
#endif
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

    pthread_exit((void *) 0);
}

void *CMainCapture::_AsyncInitializeThreadCallback(LPVOID pParam)
{
    if (pParam == NULL)
    {
        return NULL;
    }

    ThreadParam	*pThreadParam  = reinterpret_cast<ThreadParam *>(pParam);
    CMainCapture	*pOwner = (CMainCapture *)pThreadParam->pMainCapture;
    CIBUsbManager	*pIBUsbManager = (CIBUsbManager *)pThreadParam->pParentApp;
    int				nRc = IBSU_STATUS_OK;

    pThreadParam->threadStarted = TRUE;

    nRc = pOwner->_OpenDeviceThread();
    if (nRc == IBSU_STATUS_OK)
    {
        pIBUsbManager->m_bInitialized = TRUE;
        pOwner->SetInitialized(TRUE);
    }
    pIBUsbManager->m_bCompletedOpenDeviceThread = TRUE;

    pOwner->m_clbkProperty.errorCode = nRc;
    pOwner->_PostCallback(CALLBACK_ASYNC_OPEN_DEVICE);

    pthread_exit((void *) 0);
}

void *CMainCapture::_KeyButtonThreadCallback(LPVOID pParam)
{
    if (pParam == NULL)
    {
        return NULL;
    }

    ThreadParam	*pThreadParam  = reinterpret_cast<ThreadParam *>(pParam);
    CMainCapture	*pOwner = (CMainCapture *)pThreadParam->pMainCapture;
    BOOL			bExitThread = FALSE;
	unsigned char   stat1, stat2;
	unsigned char   keyButtonState = 0;
	unsigned char   saveKeyButtonState = 0xFF;

    pThreadParam->threadStarted = TRUE;

    while (1)
    {
        pOwner->m_bAliveKeyButtonThread = TRUE;

        if (pOwner->m_bKeyButtonThread_DestroyEvent)
        {
            break;
        }
        else if (pOwner->m_bKeyButtonThread_ReadEvent)
        {
            stat1=0;
		    stat2=0;

		    if ( pOwner->_GetButtonStat_Kojak(&stat1, &stat2) == IBSU_STATUS_OK )
		    {
			    keyButtonState = stat1 + stat2*2;
/*			    if (keyButtonState > 0)
			    {
				    pOwner->_ClearButtonStat_Kojak(1);
				    pOwner->_ClearButtonStat_Kojak(2);
			    }
*/
			    if (keyButtonState != saveKeyButtonState)
			    {
				    saveKeyButtonState = keyButtonState;
				    if (keyButtonState > 0)
				    {
//                        pOwner->Control_SetBeeper(ENUM_IBSU_BEEP_PATTERN_GENERIC, 7, 1);
					    pOwner->_PostCallback(CALLBACK_KEYBUTTON, 0, 0, 0, keyButtonState);
				    }
			    }
		    }
        }
        if (bExitThread)
        {
            break;
        }

        Sleep(100);
    }

    pOwner->m_bAliveKeyButtonThread = FALSE;

    pthread_exit((void *) 0);
}

void *CMainCapture::_TOFSensorThreadCallback(LPVOID pParam)
{
    if (pParam == NULL)
    {
        return NULL;
    }

    ThreadParam	*pThreadParam  = reinterpret_cast<ThreadParam *>(pParam);
    CMainCapture	*pOwner = (CMainCapture *)pThreadParam->pMainCapture;
    BOOL			bExitThread = FALSE;
//    BOOL            bTOFCalibrated=FALSE;
    struct timeval  current_tv;
    double          lastCalibrateTime;
	int AnalogTouchArr[10] = {-100,-100,-100,-100,-100,-100,-100,-100,-100,-100};
	int AnalogTouchArrPtr = 0;


    pThreadParam->threadStarted = TRUE;

    while (1)
    {
        pOwner->m_bAliveTOFSensorThread = TRUE;

        if (pOwner->m_bTOFSensorThread_DestroyEvent)
        {
            break;
        }
        else if (pOwner->m_bTOFSensorThread_ReadEvent)
        {
/*				BYTE value1, value2, value3;
			pOwner->_FPGA_GetRegister(0x67, &value1);
			pOwner->_FPGA_GetRegister(0x31, &value2);
			TRACE("Analog Touch state : %s EnalbeTof:%d, EnableRoll:%d, NeedtoInitCIS:%d\n", 
				(value1 == 0x00 && value2 == 0x3E)?"OFF":"ON",
				pOwner->m_propertyInfo.bEnableTOF,
				pOwner->m_propertyInfo.bEnableTOFforROLL,
				pOwner->m_bNeedtoInitializeCIS_Five0);
*/
            if (pOwner->m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0 && pOwner->m_propertyInfo.bEnableTOF == TRUE &&
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

#if defined(__linux__) || defined(WINCE)
            gettimeofday(&current_tv, NULL);
#else
            pOwner->gettimeofday(&current_tv, NULL);
#endif
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

    pthread_exit((void *) 0);
}

#ifndef __libusb_latest__
int CMainCapture::_DeviceCount()
{

    CThreadSync Sync;

    DWORD nDevices = 0;

    struct usb_bus *bus;
    struct usb_device *dev;

    m_pUSBDevice.clear();
    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == __VID_IB__)
            {
                if (m_pDlg->_FindSupportedDevices(dev->descriptor.idProduct))
                {
                    nDevices++;
                    m_pUSBDevice.push_back(dev);
                }
            }
        }
    }

    int nPPI_Devices = _DeviceCount_PPI();

    return nDevices + nPPI_Devices;
}

BOOL CMainCapture::_Open(int deviceIndex, int *errorNo)
{
    int nRc;
    *errorNo = IBSU_STATUS_OK;

    struct usb_device *dev = m_pUSBDevice.at(deviceIndex);
    m_hDriver = usb_open(dev);

    if (!m_hDriver)
    {
        *errorNo = IBSU_ERR_DEVICE_NOT_MATCHED;
        return FALSE;
    }

    // enzyme add 2011-11-14 For RaptorOne (Do not use USB hub)
#ifdef __android__
    //	usb_reset(m_hDriver);
//    usb_clear_halt(m_hDriver, EP2OUT);		// I think it is FPGA bug on Watson, Watson Mini, ...
//    usb_set_configuration(m_hDriver, dev->config->bConfigurationValue);
#endif

    if ((nRc = usb_claim_interface(m_hDriver, USB_INTERFACE)) < 0)
    {
        if (nRc == -16)
        {
            *errorNo = IBSU_ERR_DEVICE_ACTIVE;
        }
        else
        {
            *errorNo = IBSU_ERR_DEVICE_NOT_MATCHED;
        }

        usb_close(m_hDriver);
        m_hDriver = NULL;

        return FALSE;
    }

    return (m_hDriver != NULL);
}
#endif

#ifdef __libusb_latest__
BOOL CMainCapture::_OpenUSBDevice(int deviceIndex, int *errorNo)
{
    if (m_bPPIMode)
    {
        return _OpenUSBDevice_PPI(deviceIndex, errorNo);
    }

	libusb_device **dev_list = NULL;
	libusb_device *dev = NULL;
	struct libusb_device_descriptor dev_descriptor;
	int dev_list_len = 0;
	int i;
	int r;
	int nDevices = 0;
	BOOL ret = FALSE;
    int nRc;
    *errorNo = IBSU_STATUS_OK;
	BOOL found = FALSE;
	int openretval;

	if( m_hDriver != NULL)
	{
        *errorNo = IBSU_ERR_DEVICE_ACTIVE;
        return FALSE;
    }
	
	libusb_context *ctx = m_pDlg->m_LibUSBContext; // &&&&
	r = libusb_get_device_list(ctx, &dev_list);
	if (r < 0)
    {
        *errorNo = IBSU_ERR_DEVICE_NOT_MATCHED;
        return FALSE;
	}

	/* Iterate over the device list, finding the desired device */
	dev_list_len = r;
	for (i = 0; i < dev_list_len; i++)
    {
		dev = dev_list[i];
		libusb_get_device_descriptor(dev,&dev_descriptor);

		if( dev_descriptor.idVendor == __VID_IB__ )
		{
            if (m_pDlg->_FindSupportedDevices(dev_descriptor.idProduct))
            {
                if (nDevices == deviceIndex)
                {
					found = TRUE;                 
					break;
                }			
				nDevices++;
            }
		}	
	}

	if( !found )
	{
        *errorNo = IBSU_ERR_DEVICE_NOT_MATCHED;
		goto cleanup;
	}

    openretval = libusb_open(dev, &m_hDriver);

	if( openretval != 0 || !m_hDriver )
	{
        *errorNo = IBSU_ERR_DEVICE_NOT_MATCHED;
        goto cleanup;
    }

    if ((nRc = libusb_claim_interface(m_hDriver, USB_INTERFACE)) < 0)
    {
        if (nRc == LIBUSB_ERROR_BUSY)
        {
            *errorNo = IBSU_ERR_DEVICE_ACTIVE;
        }
		else if( nRc == LIBUSB_ERROR_NO_DEVICE ) // Is device disconnected?
		{
			*errorNo = IBSU_ERR_DEVICE_NOT_FOUND;
		}
        else
        {
            *errorNo = IBSU_ERR_DEVICE_NOT_MATCHED;
        }

        libusb_close(m_hDriver);
        m_hDriver = NULL;
		goto cleanup;
    }

    // Set device index
    m_unDeviceNumber = deviceIndex;

    ret = TRUE;

cleanup:
	// Free the entire list, relying on the refcount incremented by the Open to keep the open device from being freed.
	libusb_free_device_list(dev_list, 1); 
	
	return ret;
}
#else
BOOL CMainCapture::_OpenUSBDevice(int deviceIndex, int *errorNo)
{
    if (m_bPPIMode)
    {
        return _OpenUSBDevice_PPI(deviceIndex, errorNo);
    }

    *errorNo = IBSU_STATUS_OK;

    if (_DeviceCount() <= deviceIndex)
    {
        *errorNo = IBSU_ERR_DEVICE_NOT_FOUND;
        return FALSE;
    }

    // Set device index
    m_unDeviceNumber = deviceIndex;

    // connect to the device
    if (!_Open(deviceIndex, errorNo))
    {
        return FALSE;
    }

    return TRUE;
}
#endif

#ifdef __libusb_latest__
void CMainCapture::_CloseUSBDevice()
{
    if (m_bPPIMode)
    {
        return _CloseUSBDevice_PPI();
    }

    if (m_hDriver != NULL)
    {
        libusb_release_interface(m_hDriver, USB_INTERFACE);
        libusb_close(m_hDriver);
        m_hDriver = NULL;
    }
}
#else
void CMainCapture::_CloseUSBDevice()
{
    if (m_bPPIMode)
    {
        return _CloseUSBDevice_PPI();
    }

    if (m_hDriver != NULL)
    {
        usb_release_interface(m_hDriver, USB_INTERFACE);
        usb_close(m_hDriver);
        m_hDriver = NULL;
    }
}
#endif

int CMainCapture::_UsbBulkOutIn(int outEp, UCHAR uiCommand, UCHAR *outData, LONG outDataLen,
                                int inEp, UCHAR *inData, LONG inDataLen, LONG *nBytesRead, int timeout)
{
    if (m_bPPIMode)
    {
        return _UsbBulkOutIn_PPI(outEp, uiCommand, outData, outDataLen,
                                 inEp, inData, inDataLen, nBytesRead, timeout);
    }

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
    CThreadSync Sync;		// 2011-12-18 enzyme add
    IO_PACKET	io_packet;
    int			nResult;
#ifdef __libusb_latest__
    int			lByteCount;
	int			transferred=0;
#else
    LONG		lByteCount;
#endif

    if (!m_hDriver)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

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

#ifdef __libusb_latest__
	    nResult = libusb_bulk_transfer(m_hDriver, outEp, (unsigned char *)(&io_packet), lByteCount, &transferred, timeout);
#else
        nResult = usb_bulk_write(m_hDriver, outEp, (char *)(&io_packet), lByteCount, timeout);
#endif
        if (nResult < 0)
        {
            return IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
        }
    }

    if (inEp >= 0)
    {
#ifdef __libusb_latest__
        nResult = libusb_bulk_transfer(m_hDriver, inEp, (unsigned char *)inData, inDataLen, &transferred, timeout);
#else
        nResult = usb_bulk_read(m_hDriver, inEp, (char *)inData, inDataLen, timeout);
#endif

        if (nBytesRead != NULL)
        {
#ifdef __libusb_latest__
            *nBytesRead = transferred;
#else
            *nBytesRead = nResult;
#endif
        }

        if (nResult < 0)
        {
            return IBSU_ERR_CHANNEL_IO_READ_FAILED;
        }
    }

    return IBSU_STATUS_OK;
}

int CMainCapture::_UsbBulkIn(int ep, UCHAR *data, LONG dataLen, LONG *nBytesRead, int timeout)
{
    if (m_bPPIMode)
    {
        return _UsbBulkIn_PPI(ep, data, dataLen, nBytesRead, timeout);
    }

    int			nResult;
#ifdef __libusb_latest__
	int			transferred=0;
#endif

    if (!m_hDriver)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

#ifdef __libusb_latest__
    nResult = libusb_bulk_transfer(m_hDriver, ep, (unsigned char *)data, dataLen, &transferred, timeout);
#else
    nResult = usb_bulk_read(m_hDriver, ep, (char *)data, dataLen, timeout);
#endif

    if (nBytesRead != NULL)
    {
#ifdef __libusb_latest__
        *nBytesRead = transferred;
#else
        *nBytesRead = nResult;
#endif
    }

    if (nResult  < 0)
    {
        return IBSU_ERR_CHANNEL_IO_READ_FAILED;
    }

    return IBSU_STATUS_OK;
}

int CMainCapture::_UsbBulkOut(int ep, UCHAR uiCommand, UCHAR *data, LONG dataLen, int timeout)
{
    if (m_bPPIMode)
    {
        return _UsbBulkOut_PPI(ep, uiCommand, data, dataLen, timeout);
    }

    CThreadSync Sync;

    return _UsbBulkOut_Low(ep, uiCommand, data, dataLen, timeout);
}

int CMainCapture::_UsbBulkOut_Low(int ep, UCHAR uiCommand, UCHAR *data, LONG dataLen, int timeout)
{
    IO_PACKET	io_packet;
    int			nResult;
#ifdef __libusb_latest__
    int			lByteCount;
	int			transferred=0;
#else
    LONG		lByteCount;
#endif

    if (!m_hDriver)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    // \C3?\EB \B0\A1\B4\C9\C7\D1 \B5\A5\C0\CC\C5\CD \BC\F6\B7\CE \C1\A6\C7\D1
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

#ifdef __libusb_latest__
    nResult = libusb_bulk_transfer(m_hDriver, ep, (unsigned char *)(&io_packet), lByteCount, &transferred, timeout);
#else
    nResult = usb_bulk_write(m_hDriver, ep, (char *)(&io_packet), lByteCount, timeout);
#endif
    if (nResult < 0)
    {
        return IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
    }

    return IBSU_STATUS_OK;
}

#ifdef __libusb_latest__
void CMainCapture::_SetPID_VID(UsbDeviceInfo *pScanDevDesc, char *productID)
{
    if (m_bPPIMode)
    {
        return _SetPID_VID_PPI(pScanDevDesc, productID);
    }

    if (m_hDriver != NULL && pScanDevDesc != NULL)
	{
		libusb_device *dev = libusb_get_device(m_hDriver);
		if (dev != NULL)
		{
			struct libusb_device_descriptor desc;
			if (libusb_get_device_descriptor(dev, &desc) == 0)
			{
			    pScanDevDesc->vid = desc.idVendor;
		        pScanDevDesc->pid = desc.idProduct;
			}
		}
	}
}
#else
void CMainCapture::_SetPID_VID(UsbDeviceInfo *pScanDevDesc, char *productID)
{
    if (m_bPPIMode)
    {
        return _SetPID_VID_PPI(pScanDevDesc, productID);
    }

    struct usb_device *dev = m_pUSBDevice.at(m_unDeviceNumber);

    pScanDevDesc->vid = dev->descriptor.idVendor;
    pScanDevDesc->pid = dev->descriptor.idProduct;
}
#endif

#ifdef __libusb_latest__
int CMainCapture::_GetPID()
{
    if (m_bPPIMode)
    {
        return _GetPID_PPI();
    }

    if( m_hDriver != NULL )
	{
		libusb_device *dev = libusb_get_device(m_hDriver);
		if (dev != NULL)
		{
			struct libusb_device_descriptor desc;
			if (libusb_get_device_descriptor(dev, &desc) == 0)
			{
		        return desc.idProduct;
			}
		}
	}
    
	return -1;
}
#else
int CMainCapture::_GetPID()
{
    if (m_bPPIMode)
    {
        return _GetPID_PPI();
    }

    struct usb_device *dev = m_pUSBDevice.at(m_unDeviceNumber);

    return dev->descriptor.idProduct;
}
#endif

int CMainCapture::ClientWindow_Create(const IBSU_HWND hWindow, const DWORD left, const DWORD top, const DWORD right, const DWORD bottom, BOOL bReserved, DWORD imgWidth, DWORD imgHeight)
{
    return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::ClientWindow_Destroy(const BOOL clearExistingInfo)
{
    return IBSU_ERR_NOT_SUPPORTED;
}

int	CMainCapture::ClientWindow_GetProperty(const IBSU_ClientWindowPropertyId propertyId, LPSTR propertyValue)
{
    return IBSU_ERR_NOT_SUPPORTED;
}

int	CMainCapture::ClientWindow_SetProperty(const IBSU_ClientWindowPropertyId propertyId, LPCSTR propertyValue)
{
    return IBSU_ERR_NOT_SUPPORTED;
}

int	CMainCapture::ClientWindow_SetOverlayText(const char *fontName, const int fontSize, const BOOL fontBold,
        const char *text, const int posX, const int posY, const DWORD textColor)
{
    return IBSU_ERR_NOT_SUPPORTED;
}
int CMainCapture::ClientWindow_ShowOverlayObject(const int overlayHandle, const BOOL show)
{
	return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::ClientWindow_ShowAllOverlayObject(const BOOL show)
{
	return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::ClientWindow_RemoveOverlayObject(const int overlayHandle)
{
	return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::ClientWindow_RemoveAllOverlayObject()
{
	return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::ClientWindow_AddOverlayText(int *pOverlayHandle, const char *fontName, const int fontSize, const BOOL fontBold,
										 const char *text, const int posX, const int posY, const DWORD color)
{
	return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::ClientWindow_ModifyOverlayText(const int overlayHandle, const char *fontName, const int fontSize, const BOOL fontBold,
										 const char *text, const int posX, const int posY, const DWORD color)
{
	return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::ClientWindow_AddOverlayLine(int *pOverlayHandle, const int x1, const int y1,
                                         const int x2, const int y2, const int width, const DWORD color)
{
	return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::ClientWindow_ModifyOverlayLine(const int overlayHandle, const int x1, const int y1,
                                         const int x2, const int y2, const int width, const DWORD color)
{
	return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::ClientWindow_AddOverlayQuadrangle(int *pOverlayHandle, const int x1, const int y1,
                                         const int x2, const int y2, const int x3, const int y3, 
                                         const int x4, const int y4, const int width, const DWORD color)
{
	return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::ClientWindow_ModifyOverlayQuadrangle(const int overlayHandle, const int x1, const int y1,
                                         const int x2, const int y2, const int x3, const int y3, 
                                         const int x4, const int y4, const int width, const DWORD color)
{
	return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::ClientWindow_AddOverlayShape(int *pOverlayHandle, const IBSU_OverlayShapePattern shapePattern,
						const int x1, const int y1, const int x2, const int y2, const int width, const DWORD color,
						const int reserved_1, const int reserved_2)
{
	return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::ClientWindow_ModifyOverlayShape(const int overlayHandle, const IBSU_OverlayShapePattern shapePattern,
						const int x1, const int y1, const int x2, const int y2, const int width, const DWORD color,
						const int reserved_1, const int reserved_2)
{
	return IBSU_ERR_NOT_SUPPORTED;
}

int	CMainCapture::ClientWindow_Redraw(const DWORD flags)
{
	return IBSU_ERR_NOT_SUPPORTED;
}

void CMainCapture::_DrawClientWindow(BYTE *InImg, DisplayWindow dispWindow,
									 std::vector<OverlayText *>*overlayText,
									 std::vector<OverlayTextEx *>*overlayTextEx,
									 std::vector<OverlayLineEx *>*overlayLineEx,
									 std::vector<OverlayQuadrangleEx *>*overlayQuadrangleEx,
									 std::vector<OverlayShapeEx *>*overlayShapeEx,
									 const DWORD flags)
{
    return;
}

void CMainCapture::_ReDrawClientWindow_Overlays(BYTE *InImg, DisplayWindow dispWindow)
{
	return;
}

#ifdef __libusb_latest__
int CMainCapture::_GetSerialNumber_Curve(PUCHAR  Buffer)
{
    if (m_bPPIMode)
    {
        return _GetSerialNumber_Curve_PPI(Buffer);
    }

    unsigned char		strSerial[64];
    int					nResult;
    int					i, pos;

    if (m_hDriver == NULL)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

	libusb_device *dev = libusb_get_device(m_hDriver);  // Should never fail
	if (dev == NULL) 
	{
		return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
	}

	struct libusb_device_descriptor desc;
	if (libusb_get_device_descriptor(dev, &desc) != 0)
	{
		return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
	}

    nResult = libusb_get_string_descriptor_ascii(m_hDriver, desc.iSerialNumber, strSerial, sizeof(strSerial));

    if (nResult  < 0)
    {
        return IBSU_ERR_CHANNEL_IO_READ_FAILED;
    }

    int nCharacter = (int)(nResult / 2 - 1);

    if (nCharacter > 9)
    {
        nCharacter = 9;
    }

    for (i = 0, pos = 0; i < nCharacter; i++, pos++)
    {
		if (pos == 4)
		{
			Buffer[pos++] = '-';
		}
        Buffer[pos] = strSerial[i * 2 + 2];
    }

    Buffer[pos] = 0;

    return IBSU_STATUS_OK;
}

int CMainCapture::_GetUsbSpeed(UsbSpeed *speed)
{
    if (m_bPPIMode)
    {
        return _GetUsbSpeed_PPI(speed);
    }

    if (m_hDriver == NULL)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    struct libusb_device *dev = libusb_get_device(m_hDriver);
	if( dev == NULL ) // This shouldn't be possible.
	{
		return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
	}

	int LibUsbSpeed = libusb_get_device_speed(dev);
    if (LibUsbSpeed == LIBUSB_SPEED_HIGH)
    {
        *speed = USB_HIGH_SPEED;
    }
    else
    {
        *speed = USB_FULL_SPEED;
    }
		
    return IBSU_STATUS_OK;
}
#else
int CMainCapture::_GetSerialNumber_Curve(PUCHAR  Buffer)
{
    if (m_bPPIMode)
    {
        return _GetSerialNumber_Curve_PPI(Buffer);
    }

    struct usb_device *dev = m_pUSBDevice.at(m_unDeviceNumber);
    char				strSerial[64];
    int					nResult;
    int					i, pos;

    if (!m_hDriver)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    nResult = usb_get_string(m_hDriver, dev->descriptor.iSerialNumber, 0x03, strSerial, sizeof(strSerial));

    if (nResult  < 0)
    {
        return IBSU_ERR_CHANNEL_IO_READ_FAILED;
    }

    int nCharacter = (int)(nResult / 2 - 1);

    if (nCharacter > 9)
    {
        nCharacter = 9;
    }

    for (i = 0, pos = 0; i < nCharacter; i++, pos++)
    {
		if (pos == 4)
		{
			Buffer[pos++] = '-';
		}
        Buffer[pos] = strSerial[i * 2 + 2];
    }

    Buffer[pos] = 0;

    return IBSU_STATUS_OK;
}

int CMainCapture::_GetUsbSpeed(UsbSpeed *speed)
{
    if (m_bPPIMode)
    {
        return _GetUsbSpeed_PPI(speed);
    }

    struct usb_device *dev = m_pUSBDevice.at(m_unDeviceNumber);

    if (!m_hDriver)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    if (dev->descriptor.bcdUSB == 0x0200)
    {
        *speed = USB_HIGH_SPEED;
    }
    else
    {
        *speed = USB_FULL_SPEED;
    }

    return IBSU_STATUS_OK;
}
#endif




























////////////////////////////////////////////////////////////////////////////////////////
// It is for PPI functions
void _ParallelToUsbDeviceInfo(struct usb_device *pDst, Parallel_device *pSrc)
{
	pDst->descriptor.idVendor = pSrc->idVendor;
	pDst->descriptor.idProduct = pSrc->idProduct;
	pDst->descriptor.bcdUSB = 0x0200;

//	printf("Exit _ParallelToUsbDeviceInfo idVendor[%X] idProduct[%X]\n", pDst->descriptor.idVendor, pDst->descriptor.idProduct);
}


int CMainCapture::_DeviceCount_PPI()
{
    CThreadSync Sync;

	int nDevices = 0;
	int nRc = device_count(&nDevices);

    if (nRc != IBSU_STATUS_OK)
        return 0;

    return nDevices;
}

#define _I2C_CMD_SIZE_				(1)
#define	_I2C_INIT_BUFF_SIZE_		(1)
#define _SPI_CMD_SIZE_				(1)
#define	_DEVICE_INFO_SIZE_			(528)
#define	_SPI_DUMMY_SIZE_			(7)
#define _FPCB_STATE_PIN_NUM_		(33)
#define	_MASK_IMG_SIZE_				(1000960)
#define _MASK_READ_BUFFER_SIZE_		(_DEVICE_INFO_SIZE_+_MASK_IMG_SIZE_)



BOOL CMainCapture::_Open_PPI(int deviceIndex, int *errorNo)
{
	CThreadSync Sync;

	int nDevices = 0;
	struct usb_device *dev;
    int nRc = IBSU_STATUS_OK;

	memset(&dev, 0x00, sizeof(dev));
	
    *errorNo = IBSU_STATUS_OK;
	if ((nRc = device_control_init(deviceIndex)) != IBSU_STATUS_OK)
	{
		printf("device_control_init[%d]\n", nRc);
		*errorNo = IBSU_ERR_FILE_OPEN;
		return FALSE;
	}	

	if ((nRc = device_control_reg_write(deviceIndex, MAIN_POWER, 1)) != IBSU_STATUS_OK)
	{
		printf("Error %d GPIO writing for main power\n", nRc);
		*errorNo = IBSU_ERR_DEVICE_IO;
		return FALSE;
	}

	if ((nRc = device_control_reg_write(deviceIndex, LE_POWER, 1)) != IBSU_STATUS_OK)
	{
		printf("Error %d GPIO writing for LE power\n", nRc);
		*errorNo = IBSU_ERR_DEVICE_IO;
		return FALSE;
	}

	if ((nRc = device_control_reg_write(deviceIndex, CIS_RESET, 1)) != IBSU_STATUS_OK)
	{
		printf("Error %d GPIO writing for CIS reset\n", nRc);
		*errorNo = IBSU_ERR_DEVICE_IO;
		return FALSE;
	}

	if ((nRc = device_control_reg_write(deviceIndex, LE_ENABLE, 1)) != IBSU_STATUS_OK)
	{
		printf("Error %d GPIO writing for LE enable\n", nRc);
		*errorNo = IBSU_ERR_DEVICE_IO;
		return FALSE;
	}

    /* Get the number of attached scanners. */
    nRc = device_count(&nDevices);
	if (nRc != IBSU_STATUS_OK || nDevices <= 0)
	{
		printf("No IB Scan devices attached... exiting\n");
		*errorNo = IBSU_ERR_DEVICE_NOT_FOUND;
		return FALSE;
	}

    /* Intialize device */
	if ((nRc = device_init()) != IBSU_STATUS_OK)
    {
        printf("Error %d device initialize\n", nRc);
        *errorNo = IBSU_ERR_FILE_OPEN;
        return FALSE;
    }
	
    /* Open device */
    nRc = device_open(&m_nPiHandle);
    if (nRc != IBSU_STATUS_OK)
    {
		printf("device_open[%d]\n", nRc);
        *errorNo = IBSU_ERR_FILE_OPEN;
        return FALSE;
    }

	m_apiHandle = m_nPiHandle;
	
	if (device_i2c_init(m_nPiHandle) != IBSU_STATUS_OK ||
		device_spi_init(m_nPiHandle) != IBSU_STATUS_OK || 				
		device_parallel_init(m_nPiHandle) != IBSU_STATUS_OK)
	{
		printf("the initialization of serial peripheral interfaces was failed\n");
		*errorNo = IBSU_ERR_FILE_OPEN;
		return FALSE;
	}

    /* Get device description from flash memory. */
    unsigned char cmd[16]={0}, dummy[16]={0}, buffer[16]={0};
    unsigned char devDesc[UM_MAX_PAGE_SIZE]={0};
    int transferred;

    cmd[0] = 0xE8;
	if ((nRc = device_spi_read(m_nPiHandle, cmd, 4, dummy, 4, devDesc, UM_MAX_PAGE_SIZE, &transferred, __BULK_TIMEOUT__)) != IBSU_STATUS_OK)
	{
		printf("Device description read failed [%d]\n", nRc);
		*errorNo = IBSU_ERR_CHANNEL_IO_READ_FAILED;
		return FALSE;
	}

	if (transferred != UM_MAX_PAGE_SIZE)
	{
		printf("Device description read failed[transferred = %d]\n", transferred);
		*errorNo = IBSU_ERR_CHANNEL_IO_READ_FAILED;
		return FALSE;
	}

	memcpy(&m_parallelDevice, devDesc, sizeof(Parallel_device));
/*    __android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "m_parallelDevice.cProductID = %s, \n" \
            "m_parallelDevice.cSerialNumber = %s, \n" \
            "m_parallelDevice.cVendorID = %s, \n" \
            "m_parallelDevice.cIBIA_VendorID = %s, \n" \
            "m_parallelDevice.cIBIA_Version = %s, \n" \
            "m_parallelDevice.cIBIA_DeviceID = %s, \n" \
            "m_parallelDevice.cFirmware = %s, \n" \
            "m_parallelDevice.cDevRevision = %s, \n" \
            "m_parallelDevice.cProductionDate = %s, \n" \
            "m_parallelDevice.cServiceDate = %s, \n" \
            "m_parallelDevice.cFPGA = %s, \n" \
            "m_parallelDevice.cReserved_1 = %s, \n" \
            "m_parallelDevice.idVendor = %x, \n" \
            "m_parallelDevice.idProduct = %x \n",
            m_parallelDevice.cProductID,
            m_parallelDevice.cSerialNumber,
            m_parallelDevice.cVendorID,
            m_parallelDevice.cIBIA_VendorID,
            m_parallelDevice.cIBIA_Version,
            m_parallelDevice.cIBIA_DeviceID,
            m_parallelDevice.cFirmware,
            m_parallelDevice.cDevRevision,
            m_parallelDevice.cProductionDate,
            m_parallelDevice.cServiceDate,
            m_parallelDevice.cFPGA,
            m_parallelDevice.cReserved_1,
            m_parallelDevice.idVendor,
            m_parallelDevice.idProduct);
*/
	_ParallelToUsbDeviceInfo(m_pdev, &m_parallelDevice);
	m_pUSBDevice.clear();
	m_pUSBDevice.push_back(m_pdev);
	
    /* EL driver initialization */
	if ((nRc = device_control_reg_write(deviceIndex, LE_ENABLE, 0)) != IBSU_STATUS_OK ||
        (nRc = device_control_reg_write(deviceIndex, LE_POWER, 1)) != IBSU_STATUS_OK)
	{
		printf("Error %d GPIO writing for LE driver initialization\n", nRc);
		*errorNo = IBSU_ERR_DEVICE_IO;
		return FALSE;
	}
	
	cmd[0] = 0x00;	// write el driver voltage value
	buffer[0] = 0xFF;	// initial voltage value

	if (device_i2c_write(m_nPiHandle, CMD_DAC_WRITE_ADDR, cmd, 1, buffer, 1) != IBSU_STATUS_OK)
	{
		printf("Error %d i2c writing for LE driver initialization\n", nRc);
		*errorNo = IBSU_ERR_DEVICE_IO;
		return FALSE;
	}

	if ((nRc = device_control_reg_write(deviceIndex, CIS_RESET, 0)) != IBSU_STATUS_OK)
	{
		printf("Error %d GPIO writing for CIS reset\n", nRc);
		*errorNo = IBSU_ERR_DEVICE_IO;
		return FALSE;
	}

	return TRUE;
}

BOOL CMainCapture::_OpenUSBDevice_PPI(int deviceIndex, int *errorNo)
{
    *errorNo = IBSU_STATUS_OK;

    // connect to the device
    if (!_Open_PPI(deviceIndex, errorNo))
    {
        return FALSE;
    }

    // Set device index
    m_unDeviceNumber = deviceIndex;

    return TRUE;
}

void CMainCapture::_CloseUSBDevice_PPI()
{
    if (m_hDriver != NULL)
    {
		device_control_reg_write(m_unDeviceNumber, CIS_RESET, 1);
		device_control_reg_write(m_unDeviceNumber, LE_ENABLE, 0);
		device_control_reg_write(m_unDeviceNumber, LE_POWER, 0);
		device_control_reg_write(m_unDeviceNumber, MAIN_POWER, 0);

        device_parallel_close(m_nPiHandle);
        device_i2c_close(m_nPiHandle);
        device_spi_close(m_nPiHandle);

        device_close(m_nPiHandle);
        m_hDriver = NULL;

        device_control_close(m_unDeviceNumber);
    }
}

int CMainCapture::_GoJob_ParseUSBBulkOut(UCHAR uiCommand, UCHAR *outData, LONG outDataLen)
{
    int nResult = 0;
    UCHAR cmd[4]={0};
    UCHAR data[4]={0};

	switch(uiCommand)
	{
		// SPI
		case CMD_WRITE_PROPERTY :
		case CMD_START_READ_MASK :
			//nResult = device_spi_write(m_nPiHandle, &uiCommand, _SPI_CMD_SIZE_, _SPI_DUMMY_SIZE_, outData, outDataLen, &nTransferredLen, timeout);
			break;

		// PPI
		case CMD_GET_ONE_FRAME :
			break;

		// GPIO
		case CMD_FORCE_POWER_CONTROL :
		case CMD_CAPTURE_START :
		case CMD_CAPTURE_END :
			break;

		// I2C
		case CMD_WRITE_CIS_REG :
			nResult = device_i2c_write(m_nPiHandle, __I2C_WRITE_ADDRESS_COLUMBO_CIS__, &outData[0], 2, &outData[2], 2);
			break;
        case CMD_WRITE_SETI_REG :
			nResult = device_i2c_write(m_nPiHandle, __I2C_WRITE_ADDRESS_COLUMBO_CIS__, &outData[0], 1, &outData[1], 1);
			break;
		case CMD_WRITE_LE_VOLTAGE :
//			nResult = device_i2c_write(m_nPiHandle, __I2C_WRITE_ADDRESS_COLUMBO_EL__, &uiCommand, 1, outData, outDataLen);
			cmd[0] = 0x00;
            nResult = device_i2c_write(m_nPiHandle, __I2C_WRITE_ADDRESS_COLUMBO_EL__, cmd, 1, outData, 1);
            if (nResult == IBSU_STATUS_OK)
            {
                m_nLEVoltage = outData[0];
            }
			break;
		case CMD_INIT_LE_VOLTAGE :
//			nResult = device_i2c_write(m_nPiHandle, __I2C_WRITE_ADDRESS_COLUMBO_EL__, &uiCommand, 1, outData, outDataLen);
			cmd[0] = 0x00;
            data[0] = 208;
			nResult = device_i2c_write(m_nPiHandle, __I2C_WRITE_ADDRESS_COLUMBO_EL__, cmd, 1, data, 1);
			break;

		// I2C, GPIO, SPI
		case CMD_DEVICE_INITIALIZE :
//			nResult += device_i2c_write(m_nPiHandle, __I2C_WRITE_ADDRESS_COLUMBO_CIS__, &uiCommand, 1, outData, outDataLen);
//			nResult += device_i2c_write(m_nPiHandle, __I2C_WRITE_ADDRESS_COLUMBO_EL__, &uiCommand, 1, outData, outDataLen);
			break;

		// I2C, GPIO
		case CMD_DEVICE_RELEASE :
//			nResult += device_i2c_write(m_nPiHandle, __I2C_WRITE_ADDRESS_COLUMBO_CIS__, &uiCommand, 1, outData, outDataLen);
//			nResult += device_i2c_write(m_nPiHandle, __I2C_WRITE_ADDRESS_COLUMBO_EL__, &uiCommand, 1, outData, outDataLen);
			break;

		default :
			break;
	}

    return nResult;
}

int CMainCapture::_GoJob_ParseUSBBulkIn(UCHAR uiCommand, UCHAR *outData, LONG outDataLen, UCHAR *inData, LONG inDataLen, LONG *nBytesRead)
{
    int nResult = 0;
	pParallel_device pDeviceInfo = &m_parallelDevice;	

	switch(uiCommand)
	{
		// SPI
		case CMD_READ_PROPERTY :
		{
			int i = outData[0];
			switch(i)
			{
				case PROPERTY_PRODUCT_ID :
					memcpy(inData, pDeviceInfo->cProductID, PARALLEL_MAX_STR_LEN);
					break;

				case PROPERTY_SERIAL_NUMBER :
					memcpy(inData, pDeviceInfo->cSerialNumber, PARALLEL_MAX_STR_LEN);
					break;

				case PROPERTY_VENDOR_ID :
					memcpy(inData, pDeviceInfo->cVendorID, PARALLEL_MAX_STR_LEN);
					break;

				case PROPERTY_IBIA_VENDOR_ID :
					memcpy(inData, pDeviceInfo->cIBIA_VendorID, PARALLEL_MAX_STR_LEN);
					break;

				case PROPERTY_IBIA_VERSION :
					memcpy(inData, pDeviceInfo->cIBIA_Version, PARALLEL_MAX_STR_LEN);
					break;

				case PROPERTY_IBIA_DEVICE_ID :
					memcpy(inData, pDeviceInfo->cIBIA_DeviceID, PARALLEL_MAX_STR_LEN);
					break;

				case PROPERTY_FIRMWARE :
					memcpy(inData, pDeviceInfo->cFirmware, PARALLEL_MAX_STR_LEN);
					break;

				case PROPERTY_REVISION :
					memcpy(inData, pDeviceInfo->cDevRevision, PARALLEL_MAX_STR_LEN);
					break;

				case PROPERTY_PRODUCTION_DATE :
					memcpy(inData, pDeviceInfo->cProductionDate, PARALLEL_MAX_STR_LEN);
					break;

				case PROPERTY_SERVICE_DATE :
					memcpy(inData, pDeviceInfo->cServiceDate, PARALLEL_MAX_STR_LEN);
					break;

				case PROPERTY_CMT1 :
					memcpy(inData, pDeviceInfo->cReserved_1, PARALLEL_MAX_STR_LEN);
					break;
					
				case PROPERTY_CMT2 :
				case PROPERTY_CMT3 :
				case PROPERTY_CMT4 :
				case PROPERTY_CURVE_SETI_MODEL :
				default :
					return IBSU_ERR_INVALID_PARAM_VALUE;
			}

			if(nBytesRead)
			{
				*nBytesRead = PARALLEL_MAX_STR_LEN;
			}
			break;
		}

		case CMD_READ_FPGA_REG :
			switch( outData[0] )
			{
				case 0xF0 :
					inData[0] = (UCHAR)pDeviceInfo->cFPGA[0];
					break;

				case 0xF1 :
					inData[0] = (UCHAR)pDeviceInfo->cFPGA[1];
					break;

				case 0xF2 :
					inData[0] = (UCHAR)pDeviceInfo->cFPGA[2];
					break;

				case 0xF3 :
					inData[0] = (UCHAR)pDeviceInfo->cFPGA[3];
					break;

				default :
					return IBSU_ERR_INVALID_PARAM_VALUE;
			}

			break;
			
		case CMD_START_READ_MASK :
/*			szBuffer = new unsigned char[_MASK_READ_BUFFER_SIZE_+1];
			memset(szBuffer, 0x00, _MASK_READ_BUFFER_SIZE_);
			nResult = device_spi_read(m_nPiHandle, &uiCommand, 1, _SPI_DUMMY_SIZE_, szBuffer, _MASK_READ_BUFFER_SIZE_, &nTransferredLen, timeout);
			memcpy(inData, szBuffer+_DEVICE_INFO_SIZE_, _MASK_IMG_SIZE_);

			if(nBytesRead)	*nBytesRead = _MASK_IMG_SIZE_;
*/
			nResult = device_control_reg_write(m_unDeviceNumber, LE_ENABLE, 1);

//			delete [] szBuffer;
			break;

		// PPI
		case CMD_GET_ONE_FRAME :
			break;

		// GPIO
		case CMD_FORCE_POWER_CONTROL :
		case CMD_CAPTURE_START :
		case CMD_CAPTURE_END :
			break;

		// I2C
		case CMD_READ_CIS_REG :
			nResult = device_i2c_read(m_nPiHandle, __I2C_READ_ADDRESS_COLUMBO_CIS__, &outData[0], 1, inData, inDataLen);
			break;
			
        case CMD_READ_SETI_REG :
			nResult = device_i2c_read(m_nPiHandle, __I2C_WRITE_ADDRESS_COLUMBO_CIS__, &outData[0], 1, inData, inDataLen);
			break;
			
		case CMD_READ_LE_VOLTAGE :
//			nResult = device_i2c_read(m_nPiHandle, __I2C_READ_ADDRESS_COLUMBO_EL__, &uiCommand, 1, inData, inDataLen);
			inData[0] = (UCHAR)m_nLEVoltage;
            *nBytesRead = 1;
            nResult = IBSU_STATUS_OK;
			break;

		// I2C, GPIO, SPI

		default :
			break;
	}

    return nResult;
}

int CMainCapture::_UsbBulkOutIn_PPI(int outEp, UCHAR uiCommand, UCHAR *outData, LONG outDataLen,
                                int inEp, UCHAR *inData, LONG inDataLen, LONG *nBytesRead, int timeout)
{
#if 0
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
#endif
    /////////////////////////////////////////////////////////////////////////////////////
    CThreadSync Sync;		// 2011-12-18 enzyme add  // PPI에서는 삭제?? 확인 필요??
    //IO_PACKET	io_packet;
    int			nResult = 0;
	
    if (m_nPiHandle == -1)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    if (outEp >= 0)
    {
#if 0		
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

#ifdef __libusb_latest__
	    nResult = libusb_bulk_transfer(m_hDriver, outEp, (unsigned char *)(&io_packet), lByteCount, &transferred, timeout);
#else
        nResult = usb_bulk_write(m_hDriver, outEp, (char *)(&io_packet), lByteCount, timeout);
#endif
#endif

        nResult = _GoJob_ParseUSBBulkOut(uiCommand, outData, outDataLen);

        if (nResult != IBSU_STATUS_OK)
        {
            return IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
        }
    }

    if (inEp >= 0)
    {
#if 0
#ifdef __libusb_latest__
        nResult = libusb_bulk_transfer(m_hDriver, inEp, (unsigned char *)inData, inDataLen, &transferred, timeout);
#else
        nResult = usb_bulk_read(m_hDriver, inEp, (char *)inData, inDataLen, timeout);
#endif
#endif
        nResult = _GoJob_ParseUSBBulkIn(uiCommand, outData, outDataLen, inData, inDataLen, nBytesRead);

#if 0		
        if (nBytesRead != NULL)
        {
#ifdef __libusb_latest__
            *nBytesRead = transferred;
#else
            *nBytesRead = nResult;
#endif
        }
#endif

        if (nResult != 0)
        {
            return IBSU_ERR_CHANNEL_IO_READ_FAILED;
        }
    }

    return IBSU_STATUS_OK;
}

int CMainCapture::_PPI_OneFrameBulkRead(UCHAR *data, LONG dataLen, LONG *nBytesRead, int timeout)
{
    int			nResult;

    if (m_nPiHandle == -1)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

	nResult = device_parallel_bulk_read(m_nPiHandle, data, dataLen, nBytesRead, timeout);

    if (nResult  != IBSU_STATUS_OK || nBytesRead <= 0)
    {
		printf("Enter %s - %s - %d [%d][%d]\n", __FILE__, __func__, __LINE__, *nBytesRead, nResult);
        return IBSU_ERR_CHANNEL_IO_READ_FAILED;
    }

    return IBSU_STATUS_OK;
}

int CMainCapture::_PPI_MaskBulkRead(UCHAR *data, LONG dataLen, LONG *nBytesRead, int timeout)
{
    int			nResult;
    int 		    nTransferredLen = 0;
    int i, nRc;
    unsigned char cmd[16]={0}, dummy[16]={0};
    int transferred;
    
    if (m_nPiHandle == -1)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

//	nResult = device_parallel_bulk_read(m_nPiHandle, data, dataLen, nBytesRead, timeout);
    int page_count = dataLen / UM_MAX_PAGE_SIZE;
    int remained_size = dataLen - (page_count * UM_MAX_PAGE_SIZE);
    int remained_idx = page_count+1;
    
    for (i=1; i<page_count; i++)
    {
		cmd[0] = 0xE8;
		cmd[1] = (i>>6)&0x3F;
		cmd[2] = ((i&0x3F)<<2);
		cmd[3] = 0;

		if ((nRc = device_spi_read(m_nPiHandle, cmd, 4, dummy, 0, (unsigned char*)&data[(i-1) * (UM_MAX_PAGE_SIZE)], UM_MAX_PAGE_SIZE, &transferred, timeout)) != STATUS_OK)
        {
            goto error;
        }
        nTransferredLen += transferred;
    }

/*
 * Columbo PI does not enough space to verify the checksum, so the remained data not be used
    if (remained_size > 0)
    {
		cmd[0] = 0xE8;
		cmd[1] = (remained_idx>>6)&0x3F;
		cmd[2] = ((remained_idx&0x3F)<<2);
		cmd[3] = 0;

		if ((nRc = device_spi_read(m_nPiHandle, cmd, 4, dummy, 0, (unsigned char*)&data[(remained_idx-1) * (UM_MAX_PAGE_SIZE)], remained_size, &transferred, timeout)) != STATUS_OK)
        {
            goto error;
        }
        nTransferredLen += transferred;
    }
*/
    memset((unsigned char*)&data[(remained_idx-1) * (UM_MAX_PAGE_SIZE)], 0, remained_size);
    nTransferredLen += transferred;

	*nBytesRead = nTransferredLen;

	nResult = device_control_reg_write(m_unDeviceNumber, LE_ENABLE, 1);

    if (nResult  != IBSU_STATUS_OK || nBytesRead <= 0)
    {
		printf("Enter %s - %s - %d [%d][%d]\n", __FILE__, __func__, __LINE__, *nBytesRead, nResult);
        return IBSU_ERR_CHANNEL_IO_READ_FAILED;
    }

    return IBSU_STATUS_OK;

error:
	printf("Error was occurred in _PPI_MaskBulkRead()\n");
    return IBSU_ERR_COMMAND_FAILED;
}


int CMainCapture::_UsbBulkIn_PPI(int ep, UCHAR *data, LONG dataLen, LONG *nBytesRead, int timeout)
{
    return IBSU_STATUS_OK;
}

int CMainCapture::_UsbBulkOut_PPI(int ep, UCHAR uiCommand, UCHAR *data, LONG dataLen, int timeout)
{
    CThreadSync Sync;
    //IO_PACKET	io_packet;
    int			nResult = 0;
	//unsigned char		szBuffer[_MASK_READ_BUFFER_SIZE_+1];
	int 		nTransferredLen = 0;
	//pParallel_device pDeviceInfo = &m_parallelDevice;

    if (m_nPiHandle == -1)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }
#if 0
    // \C3?\EB \B0\A1\B4\C9\C7\D1 \B5\A5\C0\CC\C5\CD \BC\F6\B7\CE \C1\A6\C7\D1
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

#ifdef __libusb_latest__
    nResult = libusb_bulk_transfer(m_hDriver, ep, (unsigned char *)(&io_packet), lByteCount, &transferred, timeout);
#else
    nResult = usb_bulk_write(m_hDriver, ep, (char *)(&io_packet), lByteCount, timeout);
#endif
#endif
	// Parsing based on command (spi, i2c, gpio, ppi)
    nResult = _GoJob_ParseUSBBulkOut(uiCommand, data, dataLen);
	
    if (nResult != 0)
    {
        return IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
    }

    return IBSU_STATUS_OK;
}

void CMainCapture::_SetPID_VID_PPI(UsbDeviceInfo *pScanDevDesc, char *productID)
{
    struct usb_device *dev = m_pUSBDevice.at(m_unDeviceNumber);

    pScanDevDesc->vid = dev->descriptor.idVendor;
    pScanDevDesc->pid = dev->descriptor.idProduct;
}

int CMainCapture::_GetPID_PPI()
{
    struct usb_device *dev = m_pUSBDevice.at(m_unDeviceNumber);

    return dev->descriptor.idProduct;
}

int CMainCapture::_GetSerialNumber_Curve_PPI(PUCHAR  Buffer)
{
    Buffer[0] = 0;

    return IBSU_STATUS_OK;
}

int CMainCapture::_GetUsbSpeed_PPI(UsbSpeed *speed)
{
    *speed = USB_HIGH_SPEED;

    return IBSU_STATUS_OK;
}
