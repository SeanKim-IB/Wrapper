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
            if (dev->descriptor.idVendor == __VID_IB__ || dev->descriptor.idVendor == __VID_DERMALOG__)
            {
                if (m_pDlg->_FindSupportedDevices(dev->descriptor.idProduct))
                {
                    nDevices++;
                    m_pUSBDevice.push_back(dev);
                }
            }
        }
    }

    return nDevices;
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

		if( dev_descriptor.idVendor == __VID_IB__ || dev_descriptor.idVendor == __VID_DERMALOG__)
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
    struct usb_device *dev = m_pUSBDevice.at(m_unDeviceNumber);

    pScanDevDesc->vid = dev->descriptor.idVendor;
    pScanDevDesc->pid = dev->descriptor.idProduct;
}
#endif

#ifdef __libusb_latest__
int CMainCapture::_GetPID()
{
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
