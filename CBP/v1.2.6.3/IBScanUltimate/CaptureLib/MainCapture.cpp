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
#include "../IBScanUltimateDLL.h"
#include <math.h>
#ifdef __linux__
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include "memwatch.h"
#include <sys/time.h>
#include "LinuxPort.h"
#endif
#ifdef WINCE
#include <winsock.h>   // For timeval
#endif

// gon 2011-10-12
unsigned char vig_bright_table[37] = {  0,  40,  45,  50,  55,  58,  64,  67,  73,  76,
                                        81,  87,  90,  96,  99, 102, 107, 110, 116, 119,
                                        124, 127, 133, 135, 141, 144, 147, 152, 156, 161,
                                        162, 167, 170, 176, 178, 185, 255
                                     };//mask=11
float vig_ratio_table[37] = {0.0f, 0.56f, 0.57f, 0.57f, 0.64f, 0.67f, 0.69f, 0.72f, 0.76f, 0.77f,
                             0.81f, 0.84f, 0.85f, 0.88f, 0.89f, 0.92f, 0.93f, 0.95f, 0.96f, 0.97f,
                             1.00f, 1.00f, 1.01f, 1.01f, 1.04f, 1.04f, 1.05f, 1.07f, 1.07f, 1.08f,
                             1.08f, 1.09f, 1.09f, 1.11f, 1.11f, 1.11f, 1.11f
                            };

void CaptureLibDumpOutput( char *szTitle, UCHAR *pBuffer, size_t Count );

extern const char *sTraceMainCategory;

CMainCapture::CMainCapture(CIBScanUltimateApp *dlg, CIBUsbManager *dlgUsbManager)
{
    m_pDlg = dlg;
    m_pDlgUsbManager = dlgUsbManager;
    m_bIsInitialized = FALSE;
    m_nDeviceIndex = -1;

    m_bIsActiveCapture = FALSE;
    //	m_bIsCommunicationBreak = FALSE;
    m_bIsCaptureSetMode = FALSE;
    m_bIsSetTakeResultImageManually = FALSE;
    m_bIsActiveDummy = FALSE;

    memset(&m_clbkParam, 0, sizeof(m_clbkParam));
    for (int i = 0; i < __MAX_CALLBACK_COUNT__; i++)
    {
        //		m_clbkParam[i].bIsReadySend = TRUE;
        dlgUsbManager->m_bIsReadySend[i] = TRUE;
        m_clbkParam[i].funcIndex = (CallbackType)i;
        m_clbkParam[i].pMainCapture = this;
        m_clbkParam[i].pParentApp = m_pDlg;
        m_clbkParam[i].pIBUsbManager = m_pDlgUsbManager;
    }

#if !defined(__USE_LIBUSBX_DRIVER__)
    m_unDeviceNumber = 0;
#endif

    m_propertyInfo.nLEOperationMode = ADDRESS_LE_ON_MODE;
    m_propertyInfo.nContrastValue = __DEFAULT_CONTRAST_VALUE__;
    m_propertyInfo.nVoltageValue = __DEFAULT_VOLTAGE_VALUE__;
    m_propertyInfo.nExposureValue = __WATSON_DEFAULT_EXPOSURE_VALUE__;
    m_propertyInfo.dwActiveLEDs = IBSU_LED_NONE;
    m_propertyInfo.nContrastTres = __DEFAULT_CONTRAST_TRES__;
    m_propertyInfo.nIgnoreFingerTime = 4000;
#ifdef __IBSCAN_ULTIMATE_SDK__
    m_propertyInfo.bEnablePowerSaveMode = FALSE;
#else
    m_propertyInfo.bEnablePowerSaveMode = TRUE;
#endif
    m_propertyInfo.retryCountWrongCommunication = 6;
	m_propertyInfo.nCaptureTimeout = -1;
	m_propertyInfo.nRollMinWidth = 15;
	m_propertyInfo.nRollMode = ROLL_MODE_USE_NOTICE;
	m_propertyInfo.nRollLevel = ROLL_LEVEL_MEDIUM;
	m_propertyInfo.nCaptureAreaThreshold = 6;
	m_propertyInfo.nSINGLE_FLAT_AREA_TRES = SINGLE_FLAT_AREA_TRES + m_propertyInfo.nCaptureAreaThreshold*100;
	m_propertyInfo.bEnableDecimation = TRUE;
	m_propertyInfo.bEnableCaptureOnRelease = FALSE;
	m_propertyInfo.bNoPreviewImage = FALSE;
	m_propertyInfo.bRollImageOverride = FALSE;
	m_propertyInfo.bWarningInvalidArea = FALSE;
	m_propertyInfo.bEnableStartingVoltage = FALSE;
	m_propertyInfo.nStartingVoltage = _DAC_FOR_NORMAL_SINGLE_FINGER_;
	m_propertyInfo.nSharpenValue = 1.0f;
	m_propertyInfo.bWetFingerDetect = FALSE;
	m_propertyInfo.nWetFingerDetectLevel = 3;
	m_propertyInfo.nWetFingerDetectLevelThres[0] = 50;
	m_propertyInfo.nWetFingerDetectLevelThres[1] = 100;
	m_propertyInfo.nWetFingerDetectLevelThres[2] = 150;
	m_propertyInfo.nWetFingerDetectLevelThres[3] = 200;
	m_propertyInfo.nWetFingerDetectLevelThres[4] = 250;
	m_propertyInfo.bEnableTOF = TRUE;
	m_propertyInfo.bEnableEncryption = FALSE;
	m_propertyInfo.bEnableTOFforROLL = TRUE;
	m_propertyInfo.nStartingPositionOfRollArea = 0;
	m_propertyInfo.bStartRollWithoutLock = FALSE;
	m_propertyInfo.bReservedEnhanceResultImage = FALSE;
	m_propertyInfo.bKojakPLwithDPFilm = FALSE;
	m_propertyInfo.nSpoofLevel = 5;
	m_propertyInfo.bEnableSpoof = FALSE;
	m_propertyInfo.bIsSpoofSupported = FALSE;
	m_propertyInfo.bSupportEncryption = FALSE;
	m_propertyInfo.bEnableViewEncryptionImage = FALSE;
	m_propertyInfo.bEnableCBPMode = TRUE;
#if defined(__embedded__)  || defined(WINCE)
	m_propertyInfo.nImageProcessThres = IMAGE_PROCESS_THRES_LOW;
#else
	m_propertyInfo.nImageProcessThres = IMAGE_PROCESS_THRES_HIGH;
#endif
	m_propertyInfo.nDevId = -1;
	m_propertyInfo.ImageType = ENUM_IBSU_TYPE_NONE;
#if defined(__IBSCAN_SDK__)
    m_propertyInfo.bEnableFastFrameMode = FALSE;
    m_propertyInfo.bRawCaptureStart = FALSE;
#endif

    memset(&m_UsbDeviceInfo, 0, sizeof(m_UsbDeviceInfo));
    //	m_UsbDeviceInfo.CisRowStart = 86;		// Default value for 1022
    //	m_UsbDeviceInfo.CisColStart = 208;		// Default value for 1088

    //	memset(&m_cImgAnalysis, 0, sizeof(m_cImgAnalysis));
    m_nGammaLevel = 12;

    m_bCaptureThread_StopMessage = FALSE;
    m_bCaptureThread_DummyStopMessage = FALSE;
	m_bInitializeThread_StopMessage = FALSE;
    m_bFirstSent_clbkClearPlaten = FALSE;
    m_bPowerResume = FALSE;
	m_bNeedtoInitializeCIS_Five0 = TRUE;
    gettimeofday(&m_LastCalibrate_tv, NULL);


    m_pCaptureThread = 0;
    m_pInitializeThread = 0;
    m_pAsyncInitializeThread = 0;
    m_pKeyButtonThread = 0;
	m_pTOFSensorThread = 0;

    memset(&m_DisplayWindow, 0, sizeof(DisplayWindow));
    m_DisplayWindow.rollGuideLine = TRUE;
	m_DisplayWindow.dispInvalidArea = FALSE;				// It was changed since ver1.7.2 due to Tom's request
															// US people has big fingers (especially thumbs), so the capture may not completed with two fingers
    m_DisplayWindow.bkColor = 0x00d8e9ec;		// Button face color of Windows
    m_DisplayWindow.bIsSetDisplayWindow = FALSE;
	m_DisplayWindow.rollGuideLineWidth = 2;
    m_DisplayWindow.keepRedrawLastImage = TRUE;

    //	m_prevZoomout_height = -1;
    //	m_prevzoomout_width = -1;

    m_apiHandle = -1;
    m_pAlgo = NULL;
	m_pMatcherAlg = NULL;

    m_bUniformityMaskPath = FALSE;

    m_DisplayWindow.image = NULL;
    m_bRunningEP6 = FALSE;

    m_MasterValue = 0x0F;
    m_SlaveValue = 0x0F;

	m_Send_CaptureStart_BtnStat = -1;

	m_nAnalogTouch_PlateOrg = -1;
	m_nAnalogTouch_FilmOrg = -1;
	m_nAnalogTouch_Plate = -1;
	m_nAnalogTouch_Film = -1;
	m_nAnalogTouch_Min = -1;
	m_nAnalogTouch_Slope = -1;


#ifdef _WINDOWS
    m_hCaptureThread_DestroyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);		// create a manual event
    ResetEvent(m_hCaptureThread_DestroyEvent);

    m_hCaptureThread_CaptureEvent = CreateEvent(NULL, TRUE, FALSE, NULL);		// create a manual event
    ResetEvent(m_hCaptureThread_CaptureEvent);

    m_hCaptureThread_DummyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);			// create a manual event
    ResetEvent(m_hCaptureThread_DummyEvent);

#if defined(__IBSCAN_SDK__)
    m_hRawCaptureThread_GetOneFrameImageEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ResetEvent(m_hRawCaptureThread_GetOneFrameImageEvent);

    m_hRawCaptureThread_TakePreviewImageEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ResetEvent(m_hRawCaptureThread_TakePreviewImageEvent);

    m_hRawCaptureThread_TakeResultImageEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ResetEvent(m_hRawCaptureThread_TakeResultImageEvent);

    m_hRawCaptureThread_AbortEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ResetEvent(m_hRawCaptureThread_AbortEvent);
#endif
    m_hCaptureThread_EventArray[0] = m_hCaptureThread_DestroyEvent;
    m_hCaptureThread_EventArray[1] = m_hCaptureThread_CaptureEvent;
    m_hCaptureThread_EventArray[2] = m_hCaptureThread_DummyEvent;
#if defined(__IBSCAN_SDK__)
    m_hCaptureThread_EventArray[3] = m_hRawCaptureThread_GetOneFrameImageEvent;
    m_hCaptureThread_EventArray[4] = m_hRawCaptureThread_TakePreviewImageEvent;
    m_hCaptureThread_EventArray[5] = m_hRawCaptureThread_TakeResultImageEvent;
    m_hCaptureThread_EventArray[6] = m_hRawCaptureThread_AbortEvent;
#endif

    m_hKeyButtonThread_DestroyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);		// create a manual event
    ResetEvent(m_hKeyButtonThread_DestroyEvent);

    m_hKeyButtonThread_ReadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);		// create a manual event
    ResetEvent(m_hKeyButtonThread_ReadEvent);

    m_hKeyButtonThread_EventArray[0] = m_hKeyButtonThread_DestroyEvent;
    m_hKeyButtonThread_EventArray[1] = m_hKeyButtonThread_ReadEvent;

    m_hTOFSensorThread_DestroyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);		// create a manual event
    ResetEvent(m_hTOFSensorThread_DestroyEvent);

    m_hTOFSensorThread_ReadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);		// create a manual event
    ResetEvent(m_hTOFSensorThread_ReadEvent);

    m_hTOFSensorThread_EventArray[0] = m_hTOFSensorThread_DestroyEvent;
    m_hTOFSensorThread_EventArray[1] = m_hTOFSensorThread_ReadEvent;

#if defined(_WINDOWS) && defined(__USE_WINUSB_DRIVER__)
    m_pUsbDevice = NULL;
    m_pUsbDevice = new CWinUSBInterface(IBSCAN_DEVICE_INTERFACE_GUID);

    m_pCurveUsbDevice = NULL;
    m_pCurveUsbDevice = new CCurveUSBInterface(IBSCAN_DEVICE_INTERFACE_GUID);

#elif defined(__USE_LIBUSBX_DRIVER__)
	// Setup the CE version of libusb library
	((CIBScanUltimateApp *) AfxGetApp())->DelayedLibUSBInit(); // &&&&
#endif 

#ifdef __libusb_latest__
	m_pDlg->DelayedLibUSBInit(); // &&&&
#endif

    m_BitmapInfo = (IB_BITMAPINFO *)new BYTE[1064];
    for (int i = 0; i < 256; i++)
    {
        m_BitmapInfo->bmiColors[i].rgbRed =
            m_BitmapInfo->bmiColors[i].rgbGreen =
                m_BitmapInfo->bmiColors[i].rgbBlue = i;
        m_BitmapInfo->bmiColors[i].rgbReserved = 0;
    }
    m_BitmapInfo->bmiHeader.biBitCount = 8;
    m_BitmapInfo->bmiHeader.biClrImportant = 0;
    m_BitmapInfo->bmiHeader.biClrUsed = 0;
    m_BitmapInfo->bmiHeader.biCompression = BI_RGB;
    m_BitmapInfo->bmiHeader.biHeight = 750;
    m_BitmapInfo->bmiHeader.biPlanes = 1;
    m_BitmapInfo->bmiHeader.biSize = 40;
    m_BitmapInfo->bmiHeader.biSizeImage = 800 * 750;
    m_BitmapInfo->bmiHeader.biWidth = 800;
    m_BitmapInfo->bmiHeader.biXPelsPerMeter = 0;
    m_BitmapInfo->bmiHeader.biYPelsPerMeter = 0;

    m_EnterSleep = FALSE;

	m_EncryptionMode = ENUM_IBSU_ENCRYPTION_KEY_RANDOM;

#elif defined(__linux__)
    m_bCaptureThread_DestroyEvent = FALSE;
    m_bCaptureThread_CaptureEvent = FALSE;
    m_bCaptureThread_DummyEvent = FALSE;
#if defined(__IBSCAN_SDK__)
    m_bRawCaptureThread_GetOneFrameImageEvent = FALSE;
    m_bRawCaptureThread_TakePreviewImageEvent = FALSE;
    m_bRawCaptureThread_TakeResultImageEvent = FALSE;
    m_bRawCaptureThread_AbortEvent = FALSE;
#endif

    m_bKeyButtonThread_DestroyEvent = FALSE;
    m_bKeyButtonThread_ReadEvent = FALSE;

    m_bTOFSensorThread_DestroyEvent = FALSE;
    m_bTOFSensorThread_ReadEvent = FALSE;

    m_hDriver = NULL;
#endif

#if defined(__ppi__)	
	memset(&m_parallelDevice, 0x00, sizeof(Parallel_device));
    m_pdev = (struct usb_device*)malloc(sizeof(struct usb_device));
    m_nPiHandle = -1;
#endif
}

CMainCapture::~CMainCapture(void)
{
    if (m_DisplayWindow.image)
    {
        delete [] m_DisplayWindow.image;
        m_DisplayWindow.image = NULL;
    }

    _ClearOverlayText();
    _RemoveAllOverlayObject();

    //	Capture_Abort();
    //	Sleep(10);
//    Main_Release(TRUE);
//	m_pDlgUsbManager->_Delete_MainCapture();

#ifdef _WINDOWS
    if (m_hCaptureThread_DestroyEvent)
    {
        CloseHandle(m_hCaptureThread_DestroyEvent);
        m_hCaptureThread_DestroyEvent = NULL;
    }
    if (m_hCaptureThread_CaptureEvent)
    {
        CloseHandle(m_hCaptureThread_CaptureEvent);
        m_hCaptureThread_CaptureEvent = NULL;
    }
    if (m_hCaptureThread_DummyEvent)
    {
        CloseHandle(m_hCaptureThread_DummyEvent);
        m_hCaptureThread_DummyEvent = NULL;
    }
#if defined(__IBSCAN_SDK__)
    if( m_hRawCaptureThread_GetOneFrameImageEvent )
    {
        CloseHandle(m_hRawCaptureThread_GetOneFrameImageEvent);
        m_hRawCaptureThread_GetOneFrameImageEvent = NULL;
    }
    if( m_hRawCaptureThread_TakePreviewImageEvent )
    {
        CloseHandle(m_hRawCaptureThread_TakePreviewImageEvent);
        m_hRawCaptureThread_TakePreviewImageEvent = NULL;
    }
    if( m_hRawCaptureThread_TakeResultImageEvent )
    {
        CloseHandle(m_hRawCaptureThread_TakeResultImageEvent);
        m_hRawCaptureThread_TakeResultImageEvent = NULL;
    }
    if( m_hRawCaptureThread_AbortEvent )
    {
        CloseHandle(m_hRawCaptureThread_AbortEvent);
        m_hRawCaptureThread_AbortEvent = NULL;
    }
#endif

    if (m_hKeyButtonThread_DestroyEvent)
    {
        CloseHandle(m_hKeyButtonThread_DestroyEvent);
        m_hKeyButtonThread_DestroyEvent = NULL;
    }
    if (m_hKeyButtonThread_ReadEvent)
    {
        CloseHandle(m_hKeyButtonThread_ReadEvent);
        m_hKeyButtonThread_ReadEvent = NULL;
    }
	if (m_hTOFSensorThread_DestroyEvent)
    {
        CloseHandle(m_hTOFSensorThread_DestroyEvent);
        m_hTOFSensorThread_DestroyEvent = NULL;
    }
    if (m_hTOFSensorThread_ReadEvent)
    {
        CloseHandle(m_hTOFSensorThread_ReadEvent);
        m_hTOFSensorThread_ReadEvent = NULL;
    }
#endif

#if defined(_WINDOWS) && defined(__USE_WINUSB_DRIVER__)
    if (m_pUsbDevice)
    {
        delete m_pUsbDevice;
        m_pUsbDevice = NULL;
    }

    if (m_pCurveUsbDevice)
    {
        delete m_pCurveUsbDevice;
        m_pCurveUsbDevice = NULL;
    }

    if (m_BitmapInfo)
    {
        delete m_BitmapInfo;
        m_BitmapInfo = NULL;
    }
#endif 

#if defined(_WINDOWS)
	if (m_pAsyncInitializeThread)
    {
        ::WaitForSingleObject(m_pAsyncInitializeThread->m_hThread, INFINITE);
        delete m_pAsyncInitializeThread;
        m_pAsyncInitializeThread = 0;
    }
#endif

#if defined(__linux__)
    m_bCaptureThread_DestroyEvent = FALSE;
    m_bCaptureThread_CaptureEvent = FALSE;
    m_bCaptureThread_DummyEvent = FALSE;
#if defined(__IBSCAN_SDK__)
    m_bRawCaptureThread_GetOneFrameImageEvent = FALSE;
    m_bRawCaptureThread_TakePreviewImageEvent = FALSE;
    m_bRawCaptureThread_TakeResultImageEvent = FALSE;
    m_bRawCaptureThread_AbortEvent = FALSE;
#endif

    m_bKeyButtonThread_DestroyEvent = FALSE;
    m_bKeyButtonThread_ReadEvent = FALSE;

    m_bTOFSensorThread_DestroyEvent = FALSE;
    m_bTOFSensorThread_ReadEvent = FALSE;
#endif

    if (m_pAlgo)
    {
        delete m_pAlgo;
        m_pAlgo = NULL;
    }

	if (m_pMatcherAlg)
    {
        delete m_pMatcherAlg;
        m_pMatcherAlg = NULL;
    }

    m_pDlg = NULL;

#if defined(__ppi__)	
    free(m_pdev);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Implementation

int CMainCapture::Main_Initialize(const int deviceIndex, const BOOL bAsyncOpenDevice, LPCSTR uniformityMaskPath, BOOL useUniformityMask, BOOL bReserved, BOOL bPPIMode)
{
    int nErrorNo;
    int	nRc;
    m_bPPIMode = bPPIMode;
    int nDeviceIndex = deviceIndex;
#if defined(__ppi__)
    if (!bPPIMode)
    {
        nDeviceIndex = deviceIndex - m_pDlg->m_nPPIDeviceCount;
    }
#endif

    if (IsInitialized())
    {
        return IBSU_WRN_ALREADY_INITIALIZED;
    }

    if (!_OpenUSBDevice(nDeviceIndex, &nErrorNo))
    {
        return nErrorNo;
    }

	if ((nRc = _GetProperty(PROPERTY_PRODUCT_ID, (UCHAR *)m_propertyInfo.cProductID)) != IBSU_STATUS_OK)
    {
		// Correction for unstable device
		if (nRc != IBSU_ERR_CHANNEL_IO_READ_TIMEOUT)
        goto done;

		_CloseUSBDevice();
		if (!_OpenUSBDevice(nDeviceIndex, &nErrorNo)) goto done;
		if ((nRc = _GetProperty(PROPERTY_PRODUCT_ID, (UCHAR *)m_propertyInfo.cProductID)) != IBSU_STATUS_OK) goto done;
    }

	if ((nRc = _GetProperty(PROPERTY_SERIAL_NUMBER, (UCHAR *)m_propertyInfo.cSerialNumber)) != IBSU_STATUS_OK)
    {
        goto done;
    }

    if ((nRc = _GetProperty(PROPERTY_FIRMWARE, (UCHAR *)m_propertyInfo.cFirmware)) != IBSU_STATUS_OK)
    {
        goto done;
    }

    if ((nRc = _GetProperty(PROPERTY_REVISION, (UCHAR *)m_propertyInfo.cDevRevision)) != IBSU_STATUS_OK)
    {
        goto done;
    }

    if ((nRc = _GetProperty(PROPERTY_VENDOR_ID, (UCHAR *)m_propertyInfo.cVendorID)) != IBSU_STATUS_OK)
    {
        goto done;
    }

    if ((nRc = _GetProperty(PROPERTY_PRODUCTION_DATE, (UCHAR *)m_propertyInfo.cProductionDate)) != IBSU_STATUS_OK)
    {
        goto done;
    }

    if ((nRc = _GetProperty(PROPERTY_SERVICE_DATE, (UCHAR *)m_propertyInfo.cServiceDate)) != IBSU_STATUS_OK)
    {
        goto done;
    }

    if ((nRc = _GetUsbSpeed(&m_UsbDeviceInfo.nUsbSpeed)) != IBSU_STATUS_OK)
    {
        goto done;
    }

	// check if there is new packet structure. [ SOP(1bytes) | VERSION(2bytes) | TOF | Windowing | PPI | CS | EOP ]
	if((nRc = _Read_Calibration_Data_From_EEPROM()) != IBSU_STATUS_OK)
	{
		goto done;
	}

/*	if( m_UsbDeviceInfo.nUsbSpeed != __USB_HIGH_SPEED__ )
	{
		nRc = IBSU_ERR_USB20_REQUIRED;
		goto done;
	}
*/
#ifdef __android__
    int  busNumber, devAddress;
    char devPath[IBSU_MAX_STR_LEN];

    m_pDlg->GetBusInfoByIndex(deviceIndex, &busNumber, &devAddress, &devPath[0]);
	m_propertyInfo.nDevId = busNumber * 1000 + devAddress;
#endif

    if (!_SetInitializeDeviceDesc(&m_propertyInfo, &m_UsbDeviceInfo))
    {
        nRc = IBSU_ERR_NOT_SUPPORTED;			// Not supported yet!
        goto done;
    }

    if (!useUniformityMask)
    {
        m_UsbDeviceInfo.bNeedMask = FALSE;
    }

#if defined(WINCE)
	if (m_UsbDeviceInfo.pid != __PID_COLUMBO__ && m_UsbDeviceInfo.pid != __PID_COLUMBO_REV1__)
	{
		// It seems Curve device is not stable
		// Does not support 2 or 4-fingerprint scanner due to memeory issue of WindowsCE (Only can use max. 32MBbyte)
		nRc = IBSU_ERR_MEM_ALLOC;
		goto done;
	}
#endif

    if ((nRc = _SndUsbFwInitialize()) != IBSU_STATUS_OK)
    {
        goto done;
    }

    if (m_pInitializeThread)
    {
        nRc = IBSU_ERR_DEVICE_ACTIVE;
        goto done;
    }

    if (m_pCaptureThread)
    {
        nRc = IBSU_ERR_DEVICE_ACTIVE;
        goto done;
    }

    if (m_propertyInfo.bEnablePowerSaveMode == FALSE)
    {
        if ((nRc = _SndUsbFwCaptureStop()) != IBSU_STATUS_OK) goto done;
        if ((nRc = _SndUsbFwCaptureStart()) != IBSU_STATUS_OK) goto done;
        if ((nRc = _InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart,
            m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight)) != IBSU_STATUS_OK) goto done;
        /*
        		Capture_SetLEVoltage(__DEFAULT_VOLTAGE_VALUE__);
        		m_propertyInfo.nVoltageValue = __DEFAULT_VOLTAGE_VALUE__;
        		if( m_UsbDeviceInfo.LeVoltageControl == SUPPORT_LE_VOLTAGE_CONTROL )
        		{
        			Capture_SetLEVoltage(12);
        			Capture_SetLEVoltage(13);
        			Capture_SetLEVoltage(14);
        			Capture_SetLEVoltage(15);
        			Capture_SetLEVoltage(__DEFAULT_VOLTAGE_VALUE__);
        		}
        */

        int nFpgaVersion = _GetFpgaVersion(TRUE);

        if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
        {
            if (nFpgaVersion < ((0 << 24) | (9 << 16) | (0 << 8) | 0x0D) )
            {
                nRc = IBSU_ERR_NOT_SUPPORTED;			// Kojak beta version is not supported
                goto done;
            }
        }

		m_UsbDeviceInfo.bCanUseTOF = FALSE;
		switch (m_UsbDeviceInfo.devType)
		{
		case DEVICE_TYPE_KOJAK:
			if ( nFpgaVersion >= _FPGA_VER_FOR_KOJAK_NEW_BRD_AND_DOUBLE_P_AND_CURRENTSENSOR_ )
			{
				if (m_propertyInfo.sKojakCalibrationInfo.Version > 0)
				{
					m_UsbDeviceInfo.bCanUseTOF = TRUE;
				}
				else
				{
					// Calibration data does not written (Production problem by assembler)
					if(!bReserved)
					{
						nRc = IBSU_ERR_DEVICE_NEED_CALIBRATE_TOF;
						goto done;
					}
					else
						m_UsbDeviceInfo.bCanUseTOF = FALSE;
				}
			}
			break;
		case DEVICE_TYPE_FIVE0:
			if ( nFpgaVersion >= _FPGA_VER_FOR_FIVE0_TOF_BETA_ )
			{
				if ( (((BYTE)m_propertyInfo.cTOFAnalogPlate[0] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[1])) > 0 &&
					 (((BYTE)m_propertyInfo.cTOFAnalogPlate[2] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[3])) > 0)
				{
					m_UsbDeviceInfo.bCanUseTOF = TRUE;
				}
				else
				{
					// Calibration data does not written (Production problem by assembler)
					nRc = IBSU_ERR_DEVICE_NEED_CALIBRATE_TOF;
					goto done;
				}
			}
			break;
		default:
			break;
		}

	    nRc = _IsRequiredSDKVersion(m_propertyInfo.sKojakCalibrationInfo.cRequiredSDK);
	    if (nRc != IBSU_STATUS_OK)
	    {
		    goto done;
	    }
    }

    m_pDlgUsbManager->m_bIsReadySend[CALLBACK_INIT_PROGRESS] = TRUE;
    m_propertyInfo.bAsyncOpenDevice = bAsyncOpenDevice;
    SetDeviceIndex(deviceIndex);		/// For InitProgress callback

    nRc = IBSU_STATUS_OK;

    if (m_pAlgo != NULL)
    {
        delete m_pAlgo;
        m_pAlgo = NULL;
    }

	if (m_pMatcherAlg != NULL)
    {
        delete m_pMatcherAlg;
        m_pMatcherAlg = NULL;
    }

	try
	{
		m_pAlgo = new CIBAlgorithm(&m_UsbDeviceInfo, &m_propertyInfo);
		m_pAlgo->m_nPGoodFrameCount = &m_nGoodFrameCount;
		m_pAlgo->m_PPI_Correction_Horizontal	= m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal;
		m_pAlgo->m_PPI_Correction_Vertical		= m_propertyInfo.sKojakCalibrationInfo.PPIVertical;

		m_pMatcherAlg = new CIBSMAlgorithm;
		m_pMatcherAlg->IBSM_SDKInitialize();
	}
	catch (...)
	{
		nRc = IBSU_ERR_MEM_ALLOC;
		goto done;
	}

	if (m_pAlgo == NULL)
	{
		nRc = IBSU_ERR_MEM_ALLOC;
		goto done;
	}

	if (m_pMatcherAlg == NULL)
	{
		nRc = IBSU_ERR_MEM_ALLOC;
		goto done;
	}

    if (bReserved)
    {
        m_bIsInitialized = TRUE;
        return IBSU_STATUS_OK;
    }

    if (uniformityMaskPath)
    {
        m_bUniformityMaskPath = TRUE;
        m_pAlgo->m_bDistortionMaskPath = TRUE;

        size_t str_length = strlen(uniformityMaskPath);
#ifdef _WINDOWS
        if (uniformityMaskPath[str_length - 1] == '\\')
        {
            sprintf(m_cUniformityMaskPath, "%s%s_%s.um", uniformityMaskPath, m_propertyInfo.cProductID, m_propertyInfo.cSerialNumber);
            sprintf(m_pAlgo->m_cDistortionMaskPath, "%s%s.dt", uniformityMaskPath, m_propertyInfo.cProductID);
        }
        else
        {
            sprintf(m_cUniformityMaskPath, "%s\\%s_%s.um", uniformityMaskPath, m_propertyInfo.cProductID, m_propertyInfo.cSerialNumber);
            sprintf(m_pAlgo->m_cDistortionMaskPath, "%s\\%s.dt", uniformityMaskPath, m_propertyInfo.cProductID);
        }
#else
        if (uniformityMaskPath[str_length - 1] == '/')
        {
            sprintf(m_cUniformityMaskPath, "%s%s_%s.um", uniformityMaskPath, m_propertyInfo.cProductID, m_propertyInfo.cSerialNumber);
            sprintf(m_pAlgo->m_cDistortionMaskPath, "%s%s.dt", uniformityMaskPath, m_propertyInfo.cProductID);
        }
        else
        {
            sprintf(m_cUniformityMaskPath, "%s/%s_%s.um", uniformityMaskPath, m_propertyInfo.cProductID, m_propertyInfo.cSerialNumber);
            sprintf(m_pAlgo->m_cDistortionMaskPath, "%s/%s.dt", uniformityMaskPath, m_propertyInfo.cProductID);
        }
#endif
    }

    m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_CLEARD;

	// LE OFF
	if(m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
		_OnOff_LE_Kojak(0);

    return nRc;

done:
    _SndUsbFwCaptureStop();
    _SndUsbFwRelease();

    _CloseUSBDevice();

    return nRc;
}

int CMainCapture::Main_Release(BOOL bReleaseMark)
{
	if (!IsInitialized())
    {
        return IBSU_ERR_DEVICE_NOT_INITIALIZED;
    }

    return _Main_Release(bReleaseMark);
}

int CMainCapture::_Main_Release(BOOL bReleaseMark, BOOL bLibraryTerminating)
{
    BOOL		bLoopExit;

    // Stop the capture stream
    Capture_Abort();
    int count = 0;

    if (bReleaseMark)
    {
        while (TRUE)
        {
            bLoopExit = TRUE;
            for (int i = 0; i < __MAX_CALLBACK_COUNT__; i++)
            {
                if (m_pDlgUsbManager->m_bIsReadySend[i] == FALSE)
                {
                    bLoopExit = FALSE;
                    break;
                }
            }

            if (bLoopExit)
            {
                break;
            }

#ifdef _WINDOWS
			MSG msg;
			PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
#endif

            Sleep(10);
            if (count++ > 50)
            {
                return IBSU_ERR_RESOURCE_LOCKED;
            }
        }

        if (m_pDlgUsbManager->m_bEnterCallback)
        {
            return IBSU_ERR_RESOURCE_LOCKED;
        }
    }

    // Stop the capture thread
#ifdef _WINDOWS
    if (m_pCaptureThread)
        _DestroyDeviceThread(&m_pCaptureThread, &m_hCaptureThread_DestroyEvent, &m_bAliveCaptureThread, THREAD_CAPTURE);
    if (m_pKeyButtonThread)
        _DestroyDeviceThread(&m_pKeyButtonThread, &m_hKeyButtonThread_DestroyEvent, &m_bAliveKeyButtonThread, THREAD_KEYBUTTON);
	if (m_pTOFSensorThread)
        _DestroyDeviceThread(&m_pTOFSensorThread, &m_hTOFSensorThread_DestroyEvent, &m_bAliveTOFSensorThread, THREAD_TOFSENSOR);
#elif defined(__linux__)
    if (m_pCaptureThread)
        _DestroyDeviceThread(&m_pCaptureThread, &m_bCaptureThread_DestroyEvent, &m_bAliveCaptureThread, THREAD_CAPTURE);
    if (m_pKeyButtonThread)
        _DestroyDeviceThread(&m_pKeyButtonThread, &m_bKeyButtonThread_DestroyEvent, &m_bAliveKeyButtonThread, THREAD_KEYBUTTON);
	if (m_pTOFSensorThread)
        _DestroyDeviceThread(&m_pTOFSensorThread, &m_bTOFSensorThread_DestroyEvent, &m_bAliveTOFSensorThread, THREAD_TOFSENSOR);
#endif

    if (bLibraryTerminating)
        return IBSU_STATUS_OK;

// March 29 2014: Jeff Spencer: On Windows Mobile this function is called during DLL_PROCESS_DETACH. And it is nolonger possible
// to communicate through USB and since there are no timeouts on bulk in/outs the thread hangs and prevents the DLLs
// from properly unloading. The correct fix would be to add a IBSU_Initialize and IBSU_UnInitialize that applications could 
// call prior to DLL_PROCESS_DETACH. For Windows Applications this would be in the WM_CLOSE message handler.
#if !defined(WINCE)
    // 2013-06-25 enzyme add - Selective support LED/Touch for TBN240
    // Curve(TBN240) should turn off the LEDs and turn on the EL manually
    m_propertyInfo.dwActiveLEDs = IBSU_LED_NONE;
	if ((m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE && m_UsbDeviceInfo.curveModel == CURVE_MODEL_TBN240) ||
		(m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE_SETI && m_UsbDeviceInfo.curveModel == CURVE_MODEL_TBN340) ||
		(m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE_SETI && m_UsbDeviceInfo.curveModel == CURVE_MODEL_TBN340_V1))
	{
	    m_propertyInfo.dwActiveLEDs = IBSU_LED_SCAN_CURVE_BLUE;		// This is default LED on TBN240 / TBN340
	}
    _SetActiveLEDs(0, m_propertyInfo.dwActiveLEDs);
    m_propertyInfo.nLEOperationMode = ADDRESS_LE_ON_MODE;
    _SetLEOperationMode((WORD)m_propertyInfo.nLEOperationMode);

    _SndUsbFwCaptureStop();
    _SndUsbFwRelease();

#endif

    _CloseUSBDevice();

    if (bReleaseMark)
    {
        m_bIsInitialized = FALSE;
    }

    return IBSU_STATUS_OK;
}

int CMainCapture::Main_GetDeviceInfo(const int deviceIndex, IBSU_DeviceDescA *deviceInfo, BOOL bPPIMode)
{
    char szProductName[IBSU_MAX_STR_LEN];
    int nErrorNo;
    int nRc;
    m_bPPIMode = bPPIMode;
    int nDeviceIndex = deviceIndex;
#if defined(__ppi__)
    if (!bPPIMode)
    {
        nDeviceIndex = deviceIndex - m_pDlg->m_nPPIDeviceCount;
    }
#endif

    memset(deviceInfo, 0, sizeof(IBSU_DeviceDescA));
    if (IsInitialized())
    {
        return IBSU_ERR_COMMAND_FAILED;
    }

    if (!_OpenUSBDevice(nDeviceIndex, &nErrorNo))
    {
        return nErrorNo;
    }

    SetDeviceIndex(deviceIndex);
    if ((nRc = _GetProperty(PROPERTY_PRODUCT_ID, (UCHAR *)szProductName)) != IBSU_STATUS_OK)
    {
		// Correction for unstable device
		if (nRc != IBSU_ERR_CHANNEL_IO_READ_TIMEOUT)
        goto done;

		_CloseUSBDevice();
		if (!_OpenUSBDevice(nDeviceIndex, &nErrorNo)) goto done;
		if ((nRc = _GetProperty(PROPERTY_PRODUCT_ID, (UCHAR *)szProductName)) != IBSU_STATUS_OK) goto done;
    }
    strcpy(deviceInfo->productName, szProductName);
    if ((nRc = _GetProperty(PROPERTY_SERIAL_NUMBER, (UCHAR *)deviceInfo->serialNumber)) != IBSU_STATUS_OK)
    {
        goto done;
    }
    if ((nRc = _GetProperty(PROPERTY_FIRMWARE, (UCHAR *)deviceInfo->fwVersion)) != IBSU_STATUS_OK)
    {
        goto done;
    }
    if ((nRc = _GetProperty(PROPERTY_REVISION, (UCHAR *)deviceInfo->devRevision)) != IBSU_STATUS_OK)
    {
        goto done;
    }

	// check if there is new packet structure. [ SOP(1bytes) | VERSION(2bytes) | TOF | Windowing | PPI | CS | EOP ]
	if((nRc = _Read_Calibration_Data_From_EEPROM()) != IBSU_STATUS_OK)
	{
		goto done;
	}

//	deviceInfo->spoof = m_propertyInfo.sKojakCalibrationInfo.IsSupportSpoof;
    deviceInfo->handle = -1;
    deviceInfo->IsHandleOpened = FALSE;
    sprintf(deviceInfo->interfaceType, "%s", "USB");
#if defined(__ppi__)	
    if (m_bPPIMode)
    {
        sprintf(deviceInfo->interfaceType, "%s", "PPI");    // Parallel Peripheral Interface
    }
#endif

    _CloseUSBDevice();

    return IBSU_STATUS_OK;

done:
    _CloseUSBDevice();

    if (nRc == IBSU_ERR_CHANNEL_IO_WRITE_FAILED || nRc == IBSU_ERR_CHANNEL_IO_READ_FAILED)
    {
        nRc = IBSU_WRN_CHANNEL_IO_SLEEP_STATUS;
    }

    return nRc;
}

int CMainCapture::Main_GetProperty(const IBSU_PropertyId propertyId, LPSTR propertyValue)
{
    int  nRc;
	char cCmtValue[IBSU_MAX_STR_LEN]={0};
	int FPGAVersion=0;

	switch (propertyId)
    {
        case ENUM_IBSU_PROPERTY_PRODUCT_ID:
            return _GetProperty(PROPERTY_PRODUCT_ID, (UCHAR *)propertyValue);

        case ENUM_IBSU_PROPERTY_SERIAL_NUMBER:
            return _GetProperty(PROPERTY_SERIAL_NUMBER, (UCHAR *)propertyValue);

        case ENUM_IBSU_PROPERTY_VENDOR_ID:
            return _GetProperty(PROPERTY_VENDOR_ID, (UCHAR *)propertyValue);

        case ENUM_IBSU_PROPERTY_IBIA_VENDOR_ID:
            sprintf(propertyValue, "0");
            return IBSU_ERR_NOT_SUPPORTED;			// Not supported

        case ENUM_IBSU_PROPERTY_IBIA_VERSION:
            sprintf(propertyValue, "0");
            return IBSU_ERR_NOT_SUPPORTED;			// Not supported

        case ENUM_IBSU_PROPERTY_IBIA_DEVICE_ID:
            sprintf(propertyValue, "0");
            return IBSU_ERR_NOT_SUPPORTED;			// Not supported

        case ENUM_IBSU_PROPERTY_FIRMWARE:
            return _GetProperty(PROPERTY_FIRMWARE, (UCHAR *)propertyValue);

        case ENUM_IBSU_PROPERTY_REVISION:
            return _GetProperty(PROPERTY_REVISION, (UCHAR *)propertyValue);

        case ENUM_IBSU_PROPERTY_PRODUCTION_DATE:
            return _GetProperty(PROPERTY_PRODUCTION_DATE, (UCHAR *)propertyValue);

        case ENUM_IBSU_PROPERTY_SERVICE_DATE:
            return _GetProperty(PROPERTY_SERVICE_DATE, (UCHAR *)propertyValue);

        case ENUM_IBSU_PROPERTY_IMAGE_WIDTH:
            sprintf(propertyValue, "%d", m_UsbDeviceInfo.ImgWidth);
            break;

        case ENUM_IBSU_PROPERTY_IMAGE_HEIGHT:
            sprintf(propertyValue, "%d", m_UsbDeviceInfo.ImgHeight);
            break;

        case ENUM_IBSU_PROPERTY_RECOMMENDED_LEVEL:
            sprintf(propertyValue, "%d", m_propertyInfo.nContrastTres);
            break;

        case ENUM_IBSU_PROPERTY_IGNORE_FINGER_TIME:
            sprintf(propertyValue, "%d", m_propertyInfo.nIgnoreFingerTime);
            break;

        case ENUM_IBSU_PROPERTY_POLLINGTIME_TO_BGETIMAGE:
            sprintf(propertyValue, "%d", m_UsbDeviceInfo.pollingTimeForGetImg);
            break;

        case ENUM_IBSU_PROPERTY_ENABLE_POWER_SAVE_MODE:
            if (m_propertyInfo.bEnablePowerSaveMode)
            {
                sprintf(propertyValue, "TRUE");
            }
            else
            {
                sprintf(propertyValue, "FALSE");
            }
            break;

        case ENUM_IBSU_PROPERTY_RETRY_WRONG_COMMUNICATION:
            sprintf(propertyValue, "%d", m_propertyInfo.retryCountWrongCommunication);
            break;

		case ENUM_IBSU_PROPERTY_CAPTURE_TIMEOUT:
			sprintf(propertyValue, "%d", m_propertyInfo.nCaptureTimeout);
			break;

		case ENUM_IBSU_PROPERTY_ROLL_MIN_WIDTH:
			if(_CheckForSupportedImageType(ENUM_IBSU_ROLL_SINGLE_FINGER, m_UsbDeviceInfo.devType) == IBSU_STATUS_OK)
			{
			sprintf(propertyValue, "%d", m_propertyInfo.nRollMinWidth);
			}
			else
			{
				sprintf(propertyValue, "0");
				return IBSU_ERR_NOT_SUPPORTED;			// Not supported
			}
			break;

 		case ENUM_IBSU_PROPERTY_ROLL_MODE:
			if(_CheckForSupportedImageType(ENUM_IBSU_ROLL_SINGLE_FINGER, m_UsbDeviceInfo.devType) == IBSU_STATUS_OK)
			{
			sprintf(propertyValue, "%d", m_propertyInfo.nRollMode);
			}
			else
			{
				sprintf(propertyValue, "0");
				return IBSU_ERR_NOT_SUPPORTED;			// Not supported
			}
			break;

		case ENUM_IBSU_PROPERTY_ROLL_LEVEL:
			if(_CheckForSupportedImageType(ENUM_IBSU_ROLL_SINGLE_FINGER, m_UsbDeviceInfo.devType) == IBSU_STATUS_OK)
			{
			sprintf(propertyValue, "%d", m_propertyInfo.nRollLevel);
			}
			else
			{
				sprintf(propertyValue, "0");
				return IBSU_ERR_NOT_SUPPORTED;			// Not supported
			}
			break;

		case ENUM_IBSU_PROPERTY_CAPTURE_AREA_THRESHOLD:
			sprintf(propertyValue, "%d", m_propertyInfo.nCaptureAreaThreshold);
			break;

		case ENUM_IBSU_PROPERTY_ENABLE_DECIMATION:
            if (m_propertyInfo.bEnableDecimation)
            {
                sprintf(propertyValue, "TRUE");
            }
            else
            {
                sprintf(propertyValue, "FALSE");
            }
            break;

		case ENUM_IBSU_PROPERTY_ENABLE_CAPTURE_ON_RELEASE:
            if (m_propertyInfo.bEnableCaptureOnRelease)
            {
                sprintf(propertyValue, "TRUE");
            }
            else
            {
                sprintf(propertyValue, "FALSE");
            }
            break;

		case ENUM_IBSU_PROPERTY_DEVICE_INDEX:
			sprintf(propertyValue, "%d", GetDeviceIndex());
			break;

		case ENUM_IBSU_PROPERTY_DEVICE_ID:
			sprintf(propertyValue, "%d", m_propertyInfo.nDevId);
			break;
		case ENUM_IBSU_PROPERTY_SUPER_DRY_MODE:
			if (m_propertyInfo.nSuperDryMode)
            {
                sprintf(propertyValue, "TRUE");
            }
            else
            {
                sprintf(propertyValue, "FALSE");
            }
			break;
		case ENUM_IBSU_PROPERTY_MIN_CAPTURE_TIME_IN_SUPER_DRY_MODE:
            sprintf(propertyValue, "%d", m_propertyInfo.nWaitTimeForDry);
			break;

        case ENUM_IBSU_PROPERTY_ROLLED_IMAGE_WIDTH:
			if(_CheckForSupportedImageType(ENUM_IBSU_ROLL_SINGLE_FINGER, m_UsbDeviceInfo.devType) == IBSU_STATUS_OK)
			{
                sprintf(propertyValue, "%d", m_UsbDeviceInfo.ImgWidth_Roll);
			}
			else
			{
				sprintf(propertyValue, "0");
				return IBSU_ERR_NOT_SUPPORTED;			// Not supported
			}
            break;

        case ENUM_IBSU_PROPERTY_ROLLED_IMAGE_HEIGHT:
			if(_CheckForSupportedImageType(ENUM_IBSU_ROLL_SINGLE_FINGER, m_UsbDeviceInfo.devType) == IBSU_STATUS_OK)
			{
                sprintf(propertyValue, "%d", m_UsbDeviceInfo.ImgHeight_Roll);
			}
			else
			{
				sprintf(propertyValue, "0");
				return IBSU_ERR_NOT_SUPPORTED;			// Not supported
			}
            break;

		case ENUM_IBSU_PROPERTY_NO_PREVIEW_IMAGE:
			if (m_propertyInfo.bNoPreviewImage)
            {
                sprintf(propertyValue, "TRUE");
            }
            else
            {
                sprintf(propertyValue, "FALSE");
            }
            break;

		case ENUM_IBSU_PROPERTY_WARNING_MESSAGE_INVALID_AREA:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0)
				return IBSU_ERR_NOT_SUPPORTED;			// Not supported

			if (m_propertyInfo.bWarningInvalidArea)
            {
                sprintf(propertyValue, "TRUE");
            }
            else
            {
                sprintf(propertyValue, "FALSE");
            }
            break;

		case ENUM_IBSU_PROPERTY_ROLL_IMAGE_OVERRIDE:
			if (m_UsbDeviceInfo.devType == DEVICE_TYPE_COLUMBO ||
				m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE || 
				m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE_SETI)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			if (m_propertyInfo.bRollImageOverride)
            {
                sprintf(propertyValue, "TRUE");
            }
            else
            {
                sprintf(propertyValue, "FALSE");
            }
            break;

		case ENUM_IBSU_PROPERTY_ENABLE_WET_FINGER_DETECT:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			if (m_propertyInfo.bWetFingerDetect)
            {
				sprintf(propertyValue, "TRUE");
            }
            else
            {
                sprintf(propertyValue, "FALSE");
            }
            break;

		case ENUM_IBSU_PROPERTY_WET_FINGER_DETECT_LEVEL:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			sprintf(propertyValue, "%d", m_propertyInfo.nWetFingerDetectLevel);
            break;

		case ENUM_IBSU_PROPERTY_WET_FINGER_DETECT_LEVEL_THRESHOLD:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0 )
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			sprintf(propertyValue, "%d %d %d %d %d", m_propertyInfo.nWetFingerDetectLevelThres[0], m_propertyInfo.nWetFingerDetectLevelThres[1],
						m_propertyInfo.nWetFingerDetectLevelThres[2], m_propertyInfo.nWetFingerDetectLevelThres[3], m_propertyInfo.nWetFingerDetectLevelThres[4]);
			break;

		case ENUM_IBSU_PROPERTY_ENABLE_TOF:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			if(m_propertyInfo.bEnableTOF)
			{
				sprintf(propertyValue, "TRUE");
			}
			else
			{
				sprintf(propertyValue, "FALSE");
			}
            break;

		case ENUM_IBSU_PROPERTY_START_POSITION_OF_ROLLING_AREA:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			sprintf(propertyValue, "%d", m_propertyInfo.nStartingPositionOfRollArea);
            break;

		case ENUM_IBSU_PROPERTY_START_ROLL_WITHOUT_LOCK:
			if (_CheckForSupportedImageType(ENUM_IBSU_ROLL_SINGLE_FINGER, m_UsbDeviceInfo.devType) != IBSU_STATUS_OK)
				//m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			if(m_propertyInfo.bStartRollWithoutLock)
			{
				sprintf(propertyValue, "TRUE");
			}
			else
			{
				sprintf(propertyValue, "FALSE");
			}
            break;

		case ENUM_IBSU_PROPERTY_ENABLE_ENCRYPTION:	
			if(m_UsbDeviceInfo.devType != DEVICE_TYPE_WATSON_MINI && m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
				return IBSU_ERR_NOT_SUPPORTED;

			if ( (m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON_MINI && m_UsbDeviceInfo.nFpgaVersion < _FPGA_VER_FOR_WATSON_MINI_ENCRYPTION) ||
				 (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK && m_UsbDeviceInfo.nFpgaVersion < _FPGA_VER_FOR_KOJAK_3_0) )
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			if(m_propertyInfo.bEnableEncryption)
			{
				sprintf(propertyValue, "TRUE");
			}
			else
			{
				sprintf(propertyValue, "FALSE");
			}
			break;

		case ENUM_IBSU_PROPERTY_IS_SPOOF_SUPPORTED:
#ifndef _WINDOWS
			return IBSU_ERR_NOT_SUPPORTED;
#endif
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_COLUMBO )
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			if(m_propertyInfo.bIsSpoofSupported)
			{
				sprintf(propertyValue, "TRUE");
			}
			else
			{
				sprintf(propertyValue, "FALSE");
			}
			break;

		case ENUM_IBSU_PROPERTY_ENABLE_SPOOF:
#ifndef _WINDOWS
			return IBSU_ERR_NOT_SUPPORTED;
#endif
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_COLUMBO )
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			if (m_propertyInfo.bIsSpoofSupported == FALSE)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			if(m_propertyInfo.bEnableSpoof)
			{
				sprintf(propertyValue, "TRUE");
			}
			else
			{
				sprintf(propertyValue, "FALSE");
			}
			break;

		case ENUM_IBSU_PROPERTY_SPOOF_LEVEL:
#ifndef _WINDOWS
			return IBSU_ERR_NOT_SUPPORTED;
#endif
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_COLUMBO )
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			if (m_propertyInfo.bIsSpoofSupported == FALSE)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			sprintf(propertyValue, "%d", m_propertyInfo.nSpoofLevel);
			break;

		case ENUM_IBSU_PROPERTY_VIEW_ENCRYPTION_IMAGE_MODE:
			if(m_UsbDeviceInfo.devType != DEVICE_TYPE_WATSON_MINI && m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			if ( (m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON_MINI && m_UsbDeviceInfo.nFpgaVersion < _FPGA_VER_FOR_WATSON_MINI_ENCRYPTION) ||
				 (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK && m_UsbDeviceInfo.nFpgaVersion < _FPGA_VER_FOR_KOJAK_3_0) )
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			if(m_propertyInfo.bEnableViewEncryptionImage)
			{
				sprintf(propertyValue, "TRUE");
			}
			else
			{
				sprintf(propertyValue, "FALSE");
			}
			break;

        case ENUM_IBSU_PROPERTY_RESERVED_1:
			if( (nRc = _GetProperty(PROPERTY_CMT1, (UCHAR*)cCmtValue)) != IBSU_STATUS_OK )
				return nRc;
			memcpy(&propertyValue[0], cCmtValue, 32);
			if( (nRc = _GetProperty(PROPERTY_CMT2, (UCHAR*)cCmtValue)) != IBSU_STATUS_OK )
				return nRc;
			memcpy(&propertyValue[32], cCmtValue, 32);
			if( (nRc = _GetProperty(PROPERTY_CMT3, (UCHAR*)cCmtValue)) != IBSU_STATUS_OK )
				return nRc;
			memcpy(&propertyValue[64], cCmtValue, 32);
			if( (nRc = _GetProperty(PROPERTY_CMT4, (UCHAR*)cCmtValue)) != IBSU_STATUS_OK )
				return nRc;
			memcpy(&propertyValue[96], cCmtValue, 4);
			break;

		case ENUM_IBSU_PROPERTY_RESERVED_IMAGE_PROCESS_THRESHOLD:
			sprintf(propertyValue, "%d", m_propertyInfo.nImageProcessThres);
			break;

		case ENUM_IBSU_PROPERTY_RESERVED_ENABLE_TOF_FOR_ROLL:
			if (m_UsbDeviceInfo.bCanUseTOF == FALSE)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			if (m_propertyInfo.bEnableTOFforROLL)
            {
				sprintf(propertyValue, "TRUE");
            }
            else
            {
                sprintf(propertyValue, "FALSE");
            }
			break;

		case ENUM_IBSU_PROPERTY_RESERVED_CAPTURE_BRIGHTNESS_THRESHOLD_FOR_FLAT:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_WATSON &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_SHERLOCK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_SHERLOCK_ROIC &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_WATSON_MINI &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_COLUMBO &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			sprintf(propertyValue, "%d %d", m_propertyInfo.nCaptureBrightThresFlat[0], m_propertyInfo.nCaptureBrightThresFlat[1]);
			break;

		case ENUM_IBSU_PROPERTY_RESERVED_CAPTURE_BRIGHTNESS_THRESHOLD_FOR_ROLL:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_WATSON &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_SHERLOCK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_SHERLOCK_ROIC &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_WATSON_MINI &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_COLUMBO &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			sprintf(propertyValue, "%d %d", m_propertyInfo.nCaptureBrightThresRoll[0], m_propertyInfo.nCaptureBrightThresRoll[1]);
			break;

		case ENUM_IBSU_PROPERTY_RESERVED_ENHANCED_RESULT_IMAGE:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			if (m_propertyInfo.bReservedEnhanceResultImage)
            {
				sprintf(propertyValue, "TRUE");
            }
            else
            {
                sprintf(propertyValue, "FALSE");
            }
			break;
#if defined(__G_ADJUST_START_VOLTAGE_and_SHARPENING__)
		case ENUM_IBSU_PROPERTY_RESERVED_ENABLE_STARTING_VOLTAGE:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			if (m_propertyInfo.bEnableStartingVoltage)
            {
                sprintf(propertyValue, "TRUE");
            }
            else
            {
                sprintf(propertyValue, "FALSE");
            }
			break;

		case ENUM_IBSU_PROPERTY_RESERVED_STARTING_VOLTAGE_VALUE:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			sprintf(propertyValue, "%d", 0xFF - m_propertyInfo.nStartingVoltage + 1);
			break;

		case ENUM_IBSU_PROPERTY_RESERVED_SHARPENING_VALUE:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			sprintf(propertyValue, "%.1f", m_propertyInfo.nSharpenValue);
			break;
#endif

#if defined(__IBSCAN_SDK__)
    case IBSCAN_PROPERTY_RAW_IMAGE_WIDTH:
        sprintf(propertyValue, "%d", m_UsbDeviceInfo.CisImgWidth);
        break;

    case IBSCAN_PROPERTY_RAW_IMAGE_HEIGHT:
        sprintf(propertyValue, "%d", m_UsbDeviceInfo.CisImgHeight);
        break;

    case IBSCAN_PROPERTY_ENABLE_FAST_FRAME_MODE:
        if( m_propertyInfo.bEnableFastFrameMode )
            sprintf(propertyValue, "TRUE");
        else
            sprintf(propertyValue, "FALSE");
        break;
#endif
        default:
            sprintf(propertyValue, "0");
            return IBSU_ERR_NOT_SUPPORTED;			// Not supported
    }

    return IBSU_STATUS_OK;
}

int CMainCapture::Main_SetProperty(const IBSU_PropertyId propertyId, LPCSTR propertyValue, BOOL bRserved, ReservedAccessLevel nAcessLevel)
{
    size_t		len;
    int			nRc;
	UCHAR		*pValue;
	int			ret, temp_val[5];
	//int			FPGAVersion;

    len = strlen(propertyValue);
    switch (propertyId)
    {
        case ENUM_IBSU_PROPERTY_PRODUCT_ID:
        case ENUM_IBSU_PROPERTY_SERIAL_NUMBER:
        case ENUM_IBSU_PROPERTY_VENDOR_ID:
        case ENUM_IBSU_PROPERTY_IBIA_VENDOR_ID:
        case ENUM_IBSU_PROPERTY_IBIA_VERSION:
        case ENUM_IBSU_PROPERTY_IBIA_DEVICE_ID:
        case ENUM_IBSU_PROPERTY_FIRMWARE:
        case ENUM_IBSU_PROPERTY_REVISION:
        case ENUM_IBSU_PROPERTY_PRODUCTION_DATE:
        case ENUM_IBSU_PROPERTY_SERVICE_DATE:
            if (len <= 0 || len >= 32)
            {
                return IBSU_ERR_INVALID_PARAM_VALUE;
            }
            break;
		case ENUM_IBSU_PROPERTY_RESERVED_1:
            if (len <= 0 || len >= 100)
            {
                return IBSU_ERR_INVALID_PARAM_VALUE;
            }
#if defined(__IBSCAN_SDK__)
        pValue = (UCHAR*)propertyValue;
        if( (nRc = _SetProperty(PROPERTY_CMT1, pValue, len)) != IBSU_STATUS_OK )
            return nRc;
        if( len > 31 )
        {
            Sleep(50);
            pValue += 32;
            if( (nRc = _SetProperty(PROPERTY_CMT2, (UCHAR*)pValue, len-31)) != IBSU_STATUS_OK )
                return nRc;
        }
        if( len > 63 )
        {
            Sleep(50);
            pValue += 32;
            if( (nRc = _SetProperty(PROPERTY_CMT3, (UCHAR*)pValue, len-63)) != IBSU_STATUS_OK )
                return nRc;
        }
        if( len > 95 )
        {
            Sleep(50);
            pValue += 32;
            if( (nRc = _SetProperty(PROPERTY_CMT4, (UCHAR*)pValue, len-95)) != IBSU_STATUS_OK )
                return nRc;
        }

        return IBSU_STATUS_OK;
#endif
            break;
		case ENUM_IBSU_PROPERTY_RESERVED_IMAGE_PROCESS_THRESHOLD:
			if (len < IMAGE_PROCESS_THRES_LOW || len > IMAGE_PROCESS_THRES_HIGH)
			{
				return IBSU_ERR_INVALID_PARAM_VALUE;
			}
			break;
		case ENUM_IBSU_PROPERTY_RESERVED_ENABLE_TOF_FOR_ROLL:
			break;
		case ENUM_IBSU_PROPERTY_RESERVED_CAPTURE_BRIGHTNESS_THRESHOLD_FOR_FLAT:
			break;
		case ENUM_IBSU_PROPERTY_RESERVED_CAPTURE_BRIGHTNESS_THRESHOLD_FOR_ROLL:
			break;
		case ENUM_IBSU_PROPERTY_RESERVED_ENHANCED_RESULT_IMAGE:
			break;
#if defined(__G_ADJUST_START_VOLTAGE_and_SHARPENING__)
		case ENUM_IBSU_PROPERTY_RESERVED_ENABLE_STARTING_VOLTAGE:
			break;
		case ENUM_IBSU_PROPERTY_RESERVED_STARTING_VOLTAGE_VALUE:
			break;
		case ENUM_IBSU_PROPERTY_RESERVED_SHARPENING_VALUE:
			break;
#endif
        case ENUM_IBSU_PROPERTY_RECOMMENDED_LEVEL:
        if (atoi(propertyValue) < -5 || atoi(propertyValue) > 5)
        {
            return IBSU_ERR_INVALID_PARAM_VALUE;
        }

            m_propertyInfo.nContrastTres = atoi(propertyValue);
            return IBSU_STATUS_OK;

        case ENUM_IBSU_PROPERTY_IGNORE_FINGER_TIME:
            m_propertyInfo.nIgnoreFingerTime = atoi(propertyValue);
            return IBSU_STATUS_OK;

        case ENUM_IBSU_PROPERTY_ENABLE_POWER_SAVE_MODE:
            if (strcmp(propertyValue, "TRUE") == 0)
            {
                m_propertyInfo.bEnablePowerSaveMode = TRUE;
				if (m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON)
				{
					// CURVE (TBN240/320) always turn on (Do not need turn off because it is very small current)
                m_propertyInfo.dwActiveLEDs = 0;
				}
                if (GetIsActiveCapture() == FALSE && m_clbkProperty.nPlatenState == ENUM_IBSU_PLATEN_CLEARD)
                {
                    _SndUsbFwCaptureStop();
                    // 2013-04-26 enzyme add -
                    // Bug fixed, LED will be go to turn off automatically when FPGA go to turn off
                    // because they connected to FPGA (That was our mistake of the H/W circuit design)
                    _SetActiveLEDs(0, m_propertyInfo.dwActiveLEDs);
                }
                return IBSU_STATUS_OK;
            }
            else if (strcmp(propertyValue, "FALSE") == 0)
            {
                m_propertyInfo.bEnablePowerSaveMode = FALSE;
                if (IsInitialized() == TRUE && GetIsActiveCapture() == FALSE)
                {
                    _SndUsbFwCaptureStop();
                    _SndUsbFwCaptureStart();
                    _InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
                }
                return IBSU_STATUS_OK;
            }
            else
            {
                return IBSU_ERR_INVALID_PARAM_VALUE;
            }

        case ENUM_IBSU_PROPERTY_RETRY_WRONG_COMMUNICATION:
			if( atoi(propertyValue) < 0 || atoi(propertyValue) > 120 )
            {
                return IBSU_ERR_INVALID_PARAM_VALUE;
            }

            m_propertyInfo.retryCountWrongCommunication = atoi(propertyValue);
            return IBSU_STATUS_OK;

		case ENUM_IBSU_PROPERTY_CAPTURE_TIMEOUT:
			if( atoi(propertyValue) != -1 && (atoi(propertyValue) < 10 || atoi(propertyValue) > 3600) )
				return IBSU_ERR_INVALID_PARAM_VALUE;

			m_propertyInfo.nCaptureTimeout = atoi(propertyValue);
			return IBSU_STATUS_OK;

		case ENUM_IBSU_PROPERTY_ROLL_MIN_WIDTH:
			if(_CheckForSupportedImageType(ENUM_IBSU_ROLL_SINGLE_FINGER, m_UsbDeviceInfo.devType) == IBSU_STATUS_OK)
			{
			if( atoi(propertyValue) < 10 || atoi(propertyValue) > 30 )
				return IBSU_ERR_INVALID_PARAM_VALUE;

			m_propertyInfo.nRollMinWidth = atoi(propertyValue);

			if(m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
			{
				int Threshold_good_roll_x = (int)(m_propertyInfo.nRollMinWidth * (m_pAlgo->CIS_IMG_W_ROLL-30) / 40.64);

				if(m_pAlgo->m_best_roll_width < Threshold_good_roll_x)
				{
					m_pAlgo->m_best_roll_width = 0;
					memset(m_pAlgo->m_capture_rolled_local_best_buffer, 0, m_pAlgo->CIS_IMG_SIZE_ROLL);
					m_pAlgo->g_Rolling_Saved_Complete = FALSE;
					m_pAlgo->g_Rolling_Complete = FALSE;
					m_pAlgo->g_Rolling_BackTrace = FALSE;
					m_pAlgo->m_rollingStatus = 1;
				}
			}

			return IBSU_STATUS_OK;
			}
			else
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
		case ENUM_IBSU_PROPERTY_ROLL_MODE:
			if(_CheckForSupportedImageType(ENUM_IBSU_ROLL_SINGLE_FINGER, m_UsbDeviceInfo.devType) == IBSU_STATUS_OK)
			{
			if( atoi(propertyValue) < ROLL_MODE_NO_USE || atoi(propertyValue) > ROLL_MODE_USE_NOTICE )
				return IBSU_ERR_INVALID_PARAM_VALUE;

			m_propertyInfo.nRollMode = (RollMode)atoi(propertyValue);
			return IBSU_STATUS_OK;
			}
			else
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
		case ENUM_IBSU_PROPERTY_ROLL_LEVEL:
			if(_CheckForSupportedImageType(ENUM_IBSU_ROLL_SINGLE_FINGER, m_UsbDeviceInfo.devType) == IBSU_STATUS_OK)
			{
			if( atoi(propertyValue) < ROLL_LEVEL_LOW || atoi(propertyValue) > ROLL_LEVEL_HIGH )
				return IBSU_ERR_INVALID_PARAM_VALUE;

			m_propertyInfo.nRollLevel = (RollLevel)atoi(propertyValue);
			return IBSU_STATUS_OK;
			}
			else
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
		case ENUM_IBSU_PROPERTY_CAPTURE_AREA_THRESHOLD:
			if( atoi(propertyValue) < 0 || atoi(propertyValue) > 12 )
				return IBSU_ERR_INVALID_PARAM_VALUE;

			m_propertyInfo.nCaptureAreaThreshold = atoi(propertyValue);
			m_propertyInfo.nSINGLE_FLAT_AREA_TRES = SINGLE_FLAT_AREA_TRES + m_propertyInfo.nCaptureAreaThreshold*100;
			return IBSU_STATUS_OK;

		case ENUM_IBSU_PROPERTY_ENABLE_DECIMATION:
			if (m_UsbDeviceInfo.devType == DEVICE_TYPE_COLUMBO)
			{
				if (strcmp(propertyValue, "TRUE") == 0)
				{
					if (m_propertyInfo.bEnableCaptureOnRelease)
					{
						return IBSU_ERR_RESOURCE_LOCKED;
					}
					m_propertyInfo.bEnableDecimation = TRUE;
					return IBSU_STATUS_OK;
				}
				else if (strcmp(propertyValue, "FALSE") == 0)
				{
					m_propertyInfo.bEnableDecimation = FALSE;
					return IBSU_STATUS_OK;
				}
				else
				{
					return IBSU_ERR_INVALID_PARAM_VALUE;
				}
			}
			else if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
			{
				if (strcmp(propertyValue, "TRUE") == 0)
				{
					m_propertyInfo.bEnableDecimation = TRUE;
					return IBSU_STATUS_OK;
				}
				else if (strcmp(propertyValue, "FALSE") == 0)
				{
					m_propertyInfo.bEnableDecimation = FALSE;
					return IBSU_STATUS_OK;
				}
				else
				{
					return IBSU_ERR_INVALID_PARAM_VALUE;
				}
			}
			else
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

		case ENUM_IBSU_PROPERTY_ENABLE_CAPTURE_ON_RELEASE:
			if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
            {
                return IBSU_ERR_NOT_SUPPORTED;
            }

			if (strcmp(propertyValue, "TRUE") == 0)
			{
				m_propertyInfo.bEnableCaptureOnRelease = TRUE;
				m_propertyInfo.bSavedEnableDecimation = m_propertyInfo.bEnableDecimation;
				m_propertyInfo.bEnableDecimation = FALSE;
				return IBSU_STATUS_OK;
			}
			else if (strcmp(propertyValue, "FALSE") == 0)
			{
				m_propertyInfo.bEnableCaptureOnRelease = FALSE;
				m_propertyInfo.bEnableDecimation = m_propertyInfo.bSavedEnableDecimation;
				return IBSU_STATUS_OK;
			}
			else
			{
				return IBSU_ERR_INVALID_PARAM_VALUE;
			}
		case ENUM_IBSU_PROPERTY_SUPER_DRY_MODE:
			if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
			{
				if (strcmp(propertyValue, "TRUE") == 0)
				{
					m_propertyInfo.nSuperDryMode = 1;
				}
				else if (strcmp(propertyValue, "FALSE") == 0)
				{
					m_propertyInfo.nSuperDryMode = 0;
				}
				else
				{
					return IBSU_ERR_INVALID_PARAM_VALUE;
				}
				return IBSU_STATUS_OK;
			}
			else
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
		case ENUM_IBSU_PROPERTY_MIN_CAPTURE_TIME_IN_SUPER_DRY_MODE:
			if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
			{
			    if( atoi(propertyValue) != -1 && (atoi(propertyValue) < 600 || atoi(propertyValue) > 3000) )
				    return IBSU_ERR_INVALID_PARAM_VALUE;

                m_propertyInfo.nWaitTimeForDry = atoi(propertyValue);
				return IBSU_STATUS_OK;
			}
			else
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
		case ENUM_IBSU_PROPERTY_NO_PREVIEW_IMAGE:
//			if (m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK)
			{
				if (strcmp(propertyValue, "TRUE") == 0)
				{
					m_propertyInfo.bNoPreviewImage = 1;
				}
				else if (strcmp(propertyValue, "FALSE") == 0)
				{
					m_propertyInfo.bNoPreviewImage = 0;
				}
				else
				{
					return IBSU_ERR_INVALID_PARAM_VALUE;
				}
				return IBSU_STATUS_OK;
			}
//			else
//			{
//				return IBSU_ERR_NOT_SUPPORTED;
//			}
		case ENUM_IBSU_PROPERTY_WARNING_MESSAGE_INVALID_AREA:
			if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK ||
				m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0)
			{
				if (strcmp(propertyValue, "TRUE") == 0)
				{
					m_propertyInfo.bWarningInvalidArea = 1;
				}
				else if (strcmp(propertyValue, "FALSE") == 0)
				{
					m_propertyInfo.bWarningInvalidArea = 0;
				}
				else
				{
					return IBSU_ERR_INVALID_PARAM_VALUE;
				}
				return IBSU_STATUS_OK;
			}
			else
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
		case ENUM_IBSU_PROPERTY_ROLL_IMAGE_OVERRIDE:
			if (m_UsbDeviceInfo.devType == DEVICE_TYPE_COLUMBO ||
				m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE || 
				m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE_SETI)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			if (strcmp(propertyValue, "TRUE") == 0)
			{
				m_propertyInfo.bRollImageOverride = 1;
			}
			else if (strcmp(propertyValue, "FALSE") == 0)
			{
				m_propertyInfo.bRollImageOverride = 0;
			}
			else
			{
				return IBSU_ERR_INVALID_PARAM_VALUE;
			}
			return IBSU_STATUS_OK;
		case ENUM_IBSU_PROPERTY_ENABLE_WET_FINGER_DETECT:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0 )
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			if (strcmp(propertyValue, "TRUE") == 0)
			{
				m_propertyInfo.bWetFingerDetect = TRUE;
			}
			else if (strcmp(propertyValue, "FALSE") == 0)
			{
				m_propertyInfo.bWetFingerDetect = FALSE;
			}
			else
			{
				return IBSU_ERR_INVALID_PARAM_VALUE;
			}
			return IBSU_STATUS_OK;
		case ENUM_IBSU_PROPERTY_WET_FINGER_DETECT_LEVEL:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0 )
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			if( atoi(propertyValue) < 1 || atoi(propertyValue) > 5 )
			    return IBSU_ERR_INVALID_PARAM_VALUE;

			m_propertyInfo.nWetFingerDetectLevel = atoi(propertyValue);
			
			return IBSU_STATUS_OK;
		case ENUM_IBSU_PROPERTY_WET_FINGER_DETECT_LEVEL_THRESHOLD:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0 )
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			ret = sscanf(propertyValue, "%d %d %d %d %d", &temp_val[0], &temp_val[1], &temp_val[2], &temp_val[3], &temp_val[4]);

			if(ret != 5)
				return IBSU_ERR_INVALID_PARAM_VALUE;

			if( (temp_val[0] > temp_val[1]) ||
				(temp_val[1] > temp_val[2]) ||
				(temp_val[2] > temp_val[3]) ||
				(temp_val[3] > temp_val[4]) )
				return IBSU_ERR_INVALID_PARAM_VALUE;

			for(int i=0; i<5; i++)
			{
				if( temp_val[i] < 10 || temp_val[i] > 1000 )
					return IBSU_ERR_INVALID_PARAM_VALUE;
			}

			m_propertyInfo.nWetFingerDetectLevelThres[0] = temp_val[0];
			m_propertyInfo.nWetFingerDetectLevelThres[1] = temp_val[1];
			m_propertyInfo.nWetFingerDetectLevelThres[2] = temp_val[2];
			m_propertyInfo.nWetFingerDetectLevelThres[3] = temp_val[3];
			m_propertyInfo.nWetFingerDetectLevelThres[4] = temp_val[4];

			return IBSU_STATUS_OK;
		case ENUM_IBSU_PROPERTY_START_POSITION_OF_ROLLING_AREA:
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
				m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			if (m_bIsActiveCapture)
			{
				return IBSU_ERR_COMMAND_FAILED;
			}
			if( atoi(propertyValue) < 0 || atoi(propertyValue) > 9 )
			    return IBSU_ERR_INVALID_PARAM_VALUE;

			m_propertyInfo.nStartingPositionOfRollArea = atoi(propertyValue);
			
			return IBSU_STATUS_OK;
		case ENUM_IBSU_PROPERTY_START_ROLL_WITHOUT_LOCK:
			if(_CheckForSupportedImageType(ENUM_IBSU_ROLL_SINGLE_FINGER, m_UsbDeviceInfo.devType) != IBSU_STATUS_OK)
			//if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			if (strcmp(propertyValue, "TRUE") == 0)
			{
				m_propertyInfo.bStartRollWithoutLock = TRUE;
			}
			else if (strcmp(propertyValue, "FALSE") == 0)
			{
				m_propertyInfo.bStartRollWithoutLock = FALSE;
			}
			else
			{
				return IBSU_ERR_INVALID_PARAM_VALUE;
			}
			return IBSU_STATUS_OK;
		case ENUM_IBSU_PROPERTY_ENABLE_TOF:
			if (m_UsbDeviceInfo.bCanUseTOF == FALSE)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			if (m_bIsActiveCapture)
			{
				return IBSU_ERR_COMMAND_FAILED;
			}

			if (strcmp(propertyValue, "TRUE") == 0)
			{
				if(m_propertyInfo.bEnableTOF == FALSE)
				{
					m_propertyInfo.bEnableTOF = TRUE;
					m_bNeedtoInitializeCIS_Five0 = TRUE;
				}
			}
			else if (strcmp(propertyValue, "FALSE") == 0)
			{
				if(m_propertyInfo.bEnableTOF == TRUE)
				{
					m_propertyInfo.bEnableTOF = FALSE;
					m_bNeedtoInitializeCIS_Five0 = TRUE;
				}
			}
			else
			{
				return IBSU_ERR_INVALID_PARAM_VALUE;
			}

			return IBSU_STATUS_OK;
		case ENUM_IBSU_PROPERTY_ENABLE_ENCRYPTION:	
			if(m_UsbDeviceInfo.devType != DEVICE_TYPE_WATSON_MINI && m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
				return IBSU_ERR_NOT_SUPPORTED;

			if ( (m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON_MINI && m_UsbDeviceInfo.nFpgaVersion < _FPGA_VER_FOR_WATSON_MINI_ENCRYPTION) ||
				 (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK && m_UsbDeviceInfo.nFpgaVersion < _FPGA_VER_FOR_KOJAK_3_0) )
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			if(m_bIsActiveCapture)
			{
				return IBSU_ERR_DEVICE_ACTIVE;
			}

			if (strcmp(propertyValue, "TRUE") == 0)
			{
				if(m_propertyInfo.bEnableEncryption == FALSE)
				{
					if (m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON_MINI)
						nRc = _FPGA_SetRegister( 0x33, 0x01 );
					else if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
						nRc = _FPGA_SetRegister( 0x43, 0x01 );

					if(nRc == IBSU_STATUS_OK)
					{
						m_propertyInfo.bEnableEncryption = TRUE;
					}
					return nRc;
				}
			}
			else if (strcmp(propertyValue, "FALSE") == 0)
			{
				if(m_propertyInfo.bEnableEncryption == TRUE)
				{
					if (m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON_MINI)
						nRc = _FPGA_SetRegister( 0x33, 0x00 );
					else if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
						nRc = _FPGA_SetRegister( 0x43, 0x00 );

					if(nRc == IBSU_STATUS_OK)
					{
						m_propertyInfo.bEnableEncryption = FALSE;
					}
					return nRc;
				}
			}
			else
			{
				return IBSU_ERR_INVALID_PARAM_VALUE;
			}

			return IBSU_STATUS_OK;
		case ENUM_IBSU_PROPERTY_ENABLE_SPOOF:
#ifndef _WINDOWS
			return IBSU_ERR_NOT_SUPPORTED;
#endif
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_COLUMBO)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			
			if (m_propertyInfo.bIsSpoofSupported == FALSE)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			if (strcmp(propertyValue, "TRUE") == 0)
			{
				m_propertyInfo.bEnableSpoof = TRUE;
			}
			else if (strcmp(propertyValue, "FALSE") == 0)
			{
				m_propertyInfo.bEnableSpoof = FALSE;
			}
			else
			{
				return IBSU_ERR_INVALID_PARAM_VALUE;
			}

			return IBSU_STATUS_OK;
		case ENUM_IBSU_PROPERTY_SPOOF_LEVEL:
#ifndef _WINDOWS
			return IBSU_ERR_NOT_SUPPORTED;
#endif
			if (m_UsbDeviceInfo.devType != DEVICE_TYPE_COLUMBO)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}
			
			if (m_propertyInfo.bIsSpoofSupported == FALSE)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			if (atoi(propertyValue) < 0 ||  atoi(propertyValue) > 10)
			{
				return IBSU_ERR_INVALID_PARAM_VALUE;
			}

			m_propertyInfo.nSpoofLevel = atoi(propertyValue);

			return IBSU_STATUS_OK;
		case ENUM_IBSU_PROPERTY_VIEW_ENCRYPTION_IMAGE_MODE:
			if(m_UsbDeviceInfo.devType != DEVICE_TYPE_WATSON_MINI && m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			if ( (m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON_MINI && m_UsbDeviceInfo.nFpgaVersion < _FPGA_VER_FOR_WATSON_MINI_ENCRYPTION) ||
				 (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK && m_UsbDeviceInfo.nFpgaVersion < _FPGA_VER_FOR_KOJAK_3_0) )
			{
				return IBSU_ERR_NOT_SUPPORTED;
			}

			if (strcmp(propertyValue, "TRUE") == 0)
			{
				m_propertyInfo.bEnableViewEncryptionImage = TRUE;
			}
			else if (strcmp(propertyValue, "FALSE") == 0)
			{
				m_propertyInfo.bEnableViewEncryptionImage = FALSE;
			}
			else
			{
				return IBSU_ERR_INVALID_PARAM_VALUE;
			}

			return IBSU_STATUS_OK;
#if defined(__IBSCAN_SDK__)
    case IBSCAN_PROPERTY_ENABLE_FAST_FRAME_MODE:
        if (strcmp(propertyValue, "TRUE") == 0)
        {
            m_propertyInfo.bEnableFastFrameMode = TRUE;
        }
        else if( strcmp(propertyValue, "FALSE") == 0 )
        {
            m_propertyInfo.bEnableFastFrameMode = FALSE;
        }
        else
        {
            return IBSCAN_ERR_INVALID_PARAM_VALUE;
        }
        return IBSU_STATUS_OK;
#endif
        default:
            return IBSU_ERR_NOT_SUPPORTED;			// Not supported
    }

    if (bRserved)
    {
        switch (propertyId)
        {
            case ENUM_IBSU_PROPERTY_PRODUCT_ID:
            case ENUM_IBSU_PROPERTY_VENDOR_ID:
            case ENUM_IBSU_PROPERTY_IBIA_VENDOR_ID:
            case ENUM_IBSU_PROPERTY_IBIA_VERSION:
            case ENUM_IBSU_PROPERTY_IBIA_DEVICE_ID:
            case ENUM_IBSU_PROPERTY_FIRMWARE:
            case ENUM_IBSU_PROPERTY_REVISION:
                break;

            case ENUM_IBSU_PROPERTY_SERIAL_NUMBER:
                if (nAcessLevel < RESERVED_ACCESS_LEVEL_HIGH)
                {
                    return IBSU_ERR_NOT_SUPPORTED;
                }

                if ((nRc = _SetProperty(PROPERTY_SERIAL_NUMBER, (UCHAR *)propertyValue, len)) != IBSU_STATUS_OK)
                {
                    return nRc;
                }
                break;

            case ENUM_IBSU_PROPERTY_PRODUCTION_DATE:
                if (nAcessLevel < RESERVED_ACCESS_LEVEL_HIGH)
                {
                    return IBSU_ERR_NOT_SUPPORTED;
                }

                if ((nRc = _SetProperty(PROPERTY_PRODUCTION_DATE, (UCHAR *)propertyValue, len)) != IBSU_STATUS_OK)
                {
                    return nRc;
                }
                break;

            case ENUM_IBSU_PROPERTY_SERVICE_DATE:
                if (nAcessLevel < RESERVED_ACCESS_LEVEL_HIGH)
                {
                    return IBSU_ERR_NOT_SUPPORTED;
                }

                if ((nRc = _SetProperty(PROPERTY_SERVICE_DATE, (UCHAR *)propertyValue, len)) != IBSU_STATUS_OK)
                {
                    return nRc;
                }
                break;

            case ENUM_IBSU_PROPERTY_RESERVED_1:
                if (nAcessLevel < RESERVED_ACCESS_LEVEL_LOW)
                {
                    return IBSU_ERR_NOT_SUPPORTED;
                }

			    pValue = (UCHAR*)propertyValue;
			    if( (nRc = _SetProperty(PROPERTY_CMT1, pValue, len)) != IBSU_STATUS_OK )
				    return nRc;
			    if( len > 31 )
			    {
				    Sleep(50);
				    pValue += 32;
				    if( (nRc = _SetProperty(PROPERTY_CMT2, (UCHAR*)pValue, len-31)) != IBSU_STATUS_OK )
					    return nRc;
			    }
			    if( len > 63 )
			    {
				    Sleep(50);
				    pValue += 32;
				    if( (nRc = _SetProperty(PROPERTY_CMT3, (UCHAR*)pValue, len-63)) != IBSU_STATUS_OK )
					    return nRc;
			    }
			    if( len > 95 )
			    {
				    Sleep(50);
				    pValue += 32;
				    if( (nRc = _SetProperty(PROPERTY_CMT4, (UCHAR*)pValue, len-95)) != IBSU_STATUS_OK )
					    return nRc;
			    }
				break;

			case ENUM_IBSU_PROPERTY_RESERVED_IMAGE_PROCESS_THRESHOLD:
                if (nAcessLevel < RESERVED_ACCESS_LEVEL_MEDIUM)
                {
                    return IBSU_ERR_NOT_SUPPORTED;
                }
				if (m_UsbDeviceInfo.devType != DEVICE_TYPE_COLUMBO &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_WATSON &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_WATSON_MINI &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_SHERLOCK &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_SHERLOCK_ROIC )
				{
					return IBSU_ERR_NOT_SUPPORTED;
				}
			    m_propertyInfo.nImageProcessThres = (ImageProcessThres)atoi(propertyValue);
				break;

			case ENUM_IBSU_PROPERTY_RESERVED_ENABLE_TOF_FOR_ROLL:
				if (nAcessLevel < RESERVED_ACCESS_LEVEL_MEDIUM)
                {
                    return IBSU_ERR_NOT_SUPPORTED;
                }
				if (m_UsbDeviceInfo.bCanUseTOF == FALSE)
				{
					return IBSU_ERR_NOT_SUPPORTED;
				}
				if (m_bIsActiveCapture)
				{
					return IBSU_ERR_COMMAND_FAILED;
				}
				if (strcmp(propertyValue, "TRUE") == 0)
				{
					if(m_propertyInfo.bEnableTOFforROLL == FALSE)
					{
						m_propertyInfo.bEnableTOFforROLL = TRUE;
						if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
							m_bNeedtoInitializeCIS_Five0 = TRUE;
					}
				}
				else if (strcmp(propertyValue, "FALSE") == 0)
				{
					if(m_propertyInfo.bEnableTOFforROLL == TRUE)
					{
						m_propertyInfo.bEnableTOFforROLL = FALSE;
						if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
							m_bNeedtoInitializeCIS_Five0 = TRUE;
					}
				}
				else
				{
					return IBSU_ERR_INVALID_PARAM_VALUE;
				}
				break;

			case ENUM_IBSU_PROPERTY_RESERVED_CAPTURE_BRIGHTNESS_THRESHOLD_FOR_FLAT:
				if (nAcessLevel < RESERVED_ACCESS_LEVEL_MEDIUM)
                {
                    return IBSU_ERR_NOT_SUPPORTED;
                }

				if (m_UsbDeviceInfo.devType != DEVICE_TYPE_WATSON &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_SHERLOCK &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_SHERLOCK_ROIC &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_WATSON_MINI &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_COLUMBO &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0)
				{
					return IBSU_ERR_NOT_SUPPORTED;
				}

				ret = sscanf(propertyValue, "%d %d", &temp_val[0], &temp_val[1]);

				if(ret != 2)
					return IBSU_ERR_INVALID_PARAM_VALUE;

				if( temp_val[0] > temp_val[1] )
					return IBSU_ERR_INVALID_PARAM_VALUE;

				for(int i=0; i<2; i++)
				{
					if( temp_val[i] < 30 || temp_val[i] > 150 )
						return IBSU_ERR_INVALID_PARAM_VALUE;
				}

				m_propertyInfo.nCaptureBrightThresFlat[0] = temp_val[0];
				m_propertyInfo.nCaptureBrightThresFlat[1] = temp_val[1];
				break;

			case ENUM_IBSU_PROPERTY_RESERVED_CAPTURE_BRIGHTNESS_THRESHOLD_FOR_ROLL:
				if (nAcessLevel < RESERVED_ACCESS_LEVEL_MEDIUM)
                {
                    return IBSU_ERR_NOT_SUPPORTED;
                }

				if (m_UsbDeviceInfo.devType != DEVICE_TYPE_WATSON &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_SHERLOCK &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_SHERLOCK_ROIC &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_WATSON_MINI &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_COLUMBO &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0)
				{
					return IBSU_ERR_NOT_SUPPORTED;
				}

				ret = sscanf(propertyValue, "%d %d", &temp_val[0], &temp_val[1]);

				if(ret != 2)
					return IBSU_ERR_INVALID_PARAM_VALUE;

				if( temp_val[0] > temp_val[1] )
					return IBSU_ERR_INVALID_PARAM_VALUE;

				for(int i=0; i<2; i++)
				{
					if( temp_val[i] < 30 || temp_val[i] > 150 )
						return IBSU_ERR_INVALID_PARAM_VALUE;
				}

				m_propertyInfo.nCaptureBrightThresRoll[0] = temp_val[0];
				m_propertyInfo.nCaptureBrightThresRoll[1] = temp_val[1];
				break;

			case ENUM_IBSU_PROPERTY_RESERVED_ENHANCED_RESULT_IMAGE:
				if (nAcessLevel < RESERVED_ACCESS_LEVEL_MEDIUM)
                {
                    return IBSU_ERR_NOT_SUPPORTED;
                }

				if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
				{
					return IBSU_ERR_NOT_SUPPORTED;
				}

				if (strcmp(propertyValue, "TRUE") == 0)
				{
					m_propertyInfo.bReservedEnhanceResultImage = TRUE;
				}
				else if (strcmp(propertyValue, "FALSE") == 0)
				{
					m_propertyInfo.bReservedEnhanceResultImage = FALSE;
				}
				else
				{
					return IBSU_ERR_INVALID_PARAM_VALUE;
				}
				break;
#if defined(__G_ADJUST_START_VOLTAGE_and_SHARPENING__)
			case ENUM_IBSU_PROPERTY_RESERVED_ENABLE_STARTING_VOLTAGE:
				if (nAcessLevel < RESERVED_ACCESS_LEVEL_MEDIUM)
                {
                    return IBSU_ERR_NOT_SUPPORTED;
                }
				if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0)
				{
					return IBSU_ERR_NOT_SUPPORTED;
				}
				if (strcmp(propertyValue, "TRUE") == 0)
				{
					m_propertyInfo.bEnableStartingVoltage = 1;
				}
				else if (strcmp(propertyValue, "FALSE") == 0)
				{
					m_propertyInfo.bEnableStartingVoltage = 0;
				}
				else
				{
					return IBSU_ERR_INVALID_PARAM_VALUE;
				}
				break;

			case ENUM_IBSU_PROPERTY_RESERVED_STARTING_VOLTAGE_VALUE:
				if (nAcessLevel < RESERVED_ACCESS_LEVEL_MEDIUM)
                {
                    return IBSU_ERR_NOT_SUPPORTED;
                }
				if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0)
				{
					return IBSU_ERR_NOT_SUPPORTED;
				}
				if(atoi(propertyValue) < 1 || atoi(propertyValue) > 96) // 1: 0xFF, 64 : 0xA0
				{
					return IBSU_ERR_INVALID_PARAM_VALUE;
				}
				
				m_propertyInfo.nStartingVoltage = 0xFF - atoi(propertyValue) + 1;
				break;

			case ENUM_IBSU_PROPERTY_RESERVED_SHARPENING_VALUE:
				if (nAcessLevel < RESERVED_ACCESS_LEVEL_MEDIUM)
                {
                    return IBSU_ERR_NOT_SUPPORTED;
                }
				if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK &&
					m_UsbDeviceInfo.devType != DEVICE_TYPE_FIVE0)
				{
					return IBSU_ERR_NOT_SUPPORTED;
				}
				if(atof(propertyValue) < 0.0 || atof(propertyValue) > 5.0)
				{
					return IBSU_ERR_INVALID_PARAM_VALUE;
				}
				
				m_propertyInfo.nSharpenValue = (float)atof(propertyValue);
				break;
#endif

			default:
                break;
        }

        return IBSU_STATUS_OK;
    }

    return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::Capture_Start(const IBSU_ImageType imageType,
                                const IBSU_ImageResolution imageResolution,
                                const DWORD captureOptions,
                                BOOL bReserved,
                                BOOL bRawCapture)
{
    int			nRc;
    int			numberOfObjects;
    DWORD		options;
    int			nWidth, nHeight, nResolutionX, nResolutionY;
    int			tries = 0;

	m_pAlgo->g_Stitch_Dir = 0;
	m_pAlgo->g_RollingWidth = 0;
	m_TOF_MaxVal = 0;
	m_TOF_MaxVal_DAC = 0xFF;
	m_CurrentCaptureDAC = 0x00;
	m_CurrentCaptureDACMax = 0xFF;
	m_CurrentCaptureDACMax_TOFValue = 0x00;
	m_UsbDeviceInfo.bSpoofDetected = FALSE;
#ifdef _KOJAK_ATOF_16B_
	m_Kojak30_OperationMode = 2;
#endif

	if(m_propertyInfo.bEnableCBPMode == TRUE)
		memset(&m_pAlgo->m_CBPPreviewInfo, 0, sizeof(CBPPreviewInfo));
	
	if(m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON || m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON_MINI)
		m_propertyInfo.nExposureValue = __WATSON_DEFAULT_EXPOSURE_VALUE__;
	else if(m_UsbDeviceInfo.devType == DEVICE_TYPE_COLUMBO)
		m_propertyInfo.nExposureValue = __COLUMBO_DEFAULT_EXPOSURE_VALUE__;

	if(m_UsbDeviceInfo.devType == DEVICE_TYPE_COLUMBO)
		memset(m_pAlgo->m_final_image, 0xFF, m_UsbDeviceInfo.CisImgSize);

#ifdef __G_INIT_CAPTURE_TIME__
    gettimeofday(&m_initCaptureStart_tv, NULL);
#endif
    // Deletion previous image when you satart capture on the screen
#ifdef __IBSCAN_ULTIMATE_SDK__
	memset(m_pAlgo->m_OutRollResultImg, 0xFF, m_UsbDeviceInfo.ImgSize);
    memset(m_pAlgo->m_OutResultImg, 0xFF, m_UsbDeviceInfo.ImgSize);
#else
	memset(m_pAlgo->m_OutRollResultImg, 0x00, m_UsbDeviceInfo.ImgSize);
    memset(m_pAlgo->m_OutResultImg, 0x00, m_UsbDeviceInfo.ImgSize);
#endif

	m_pAlgo->m_cImgAnalysis.isDetected = FALSE;
	m_pAlgo->m_cImgAnalysis.isDetectedTOF = FALSE;

	memset(m_pAlgo->m_capture_rolled_buffer, 0, m_UsbDeviceInfo.CisImgSize);
	memset(m_pAlgo->m_capture_rolled_best_buffer, 0, m_UsbDeviceInfo.CisImgSize);
	memset(m_pAlgo->m_capture_rolled_local_best_buffer, 0, m_UsbDeviceInfo.CisImgSize);

	memset(m_pAlgo->m_pUsbDevInfo->CapturedBrightness, 0, sizeof(m_pAlgo->m_pUsbDevInfo->CapturedBrightness));
	memset(m_pAlgo->m_pUsbDevInfo->CapturedWetRatio, 0, sizeof(m_pAlgo->m_pUsbDevInfo->CapturedWetRatio));
    options = captureOptions;
    if ((captureOptions & IBSU_OPTION_AUTO_CAPTURE) == IBSU_OPTION_AUTO_CAPTURE)
    {
        options |= IBSU_OPTION_AUTO_CONTRAST;
    }
/*
    nRc = Capture_SetMode(imageType, imageResolution, options, &nWidth, &nHeight, &nResolutionX, &nResolutionY);
    if (nRc != IBSU_STATUS_OK)
    {
        return nRc;
    }
*/
    switch (imageType)
    {
        case ENUM_IBSU_TYPE_NONE:
            return IBSU_ERR_NOT_SUPPORTED;
        case ENUM_IBSU_FLAT_SINGLE_FINGER:
        case ENUM_IBSU_ROLL_SINGLE_FINGER:
            numberOfObjects = 1;
            break;
        case ENUM_IBSU_FLAT_TWO_FINGERS:
            numberOfObjects = 2;
            break;
		case ENUM_IBSU_FLAT_THREE_FINGERS:
            numberOfObjects = 3;
            break;
        case ENUM_IBSU_FLAT_FOUR_FINGERS:
            numberOfObjects = 4;
            break;
        default:
            return IBSU_ERR_INVALID_PARAM_VALUE;
    }

    if (m_bIsActiveCapture)
    {
        return IBSU_ERR_CAPTURE_STILL_RUNNING;
    }

    // 2013-05-03 enzyme delete - Don't add any USB bulk function during working the dummy/capture Thread in the Curve USB driver(old).
    // Because of the Curve USB driver(old) does not allow to communicate USB bulk at same time.
    // If you add critical section ("CThreadSync Sync;") on the bulk function, you can solve this issue.
    // However, it makes slow the frame rate on the multi-devices environment.
    // So I do not use this way until we change USB driver from Curve driver(old) to WinUsbDriver for Curve device
    // Is alive USB device
    //	if( !_IsAlive_UsbDevice() )
    //		return IBSU_ERR_DEVICE_IO;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // enzyme add 2012-11-05 Must wait for the starting capture until finish dummy image process.
    if (!bReserved)
    {
        // RESERVED_BeginCaptureImage does not use Capture Thread..
        // Commented out because of error on Windows.  Is this even necessary?  Why is it here then?
#if 0
#ifdef _WINDOWS
        ResetEvent(m_hCaptureThread_DummyEvent);
#else
        m_bCaptureThread_DummyEvent = FALSE;
#endif
#endif
        m_bCaptureThread_DummyStopMessage = TRUE;
        do
        {
            Sleep(10);
            // 2013-03-27 enzyme modify - Bug fixed
            // Just wait for finished dummy capture without returning error
            if (tries++ > 100)
            {
                m_bIsActiveDummy = FALSE;
                //				return IBSU_ERR_CAPTURE_STILL_RUNNING;
                break;
            }
        }
        while (m_bIsActiveDummy);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    m_propertyInfo.nNumberOfObjects = numberOfObjects;
     nRc = Capture_SetMode(imageType, imageResolution, options, &nWidth, &nHeight, &nResolutionX, &nResolutionY);
    if (nRc != IBSU_STATUS_OK)
    {
        return nRc;
    }

    // Is alive USB device
    if (!_IsAlive_UsbDevice())
    {
        return IBSU_ERR_DEVICE_IO;
    }

    if (m_propertyInfo.bEnablePowerSaveMode == TRUE ||
		m_UsbDeviceInfo.devType == DEVICE_TYPE_COLUMBO ||
		_IsNeedInitializeCIS() == TRUE ||
		(m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0 && m_bNeedtoInitializeCIS_Five0 == TRUE))
    {
		_SndUsbFwCaptureStop();
		_SndUsbFwCaptureStart();
		_InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
	}

#if defined _POWER_OFF_ANALOG_TOUCH_WHEN_NO_CAPTURE_
	if(m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0)
	{
		if( m_propertyInfo.bEnableTOF)
		{
			if((m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_propertyInfo.bEnableTOFforROLL) ||
				m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER)
			{
				// TOF on
				_FPGA_SetRegister(0x30, 0x62);
				_FPGA_SetRegister(0x67, 0x01);
			
				_CalibrateTOF();
			}
		}
	}
#endif

	m_UsbDeviceInfo.bDecimation = FALSE;
	m_UsbDeviceInfo.nDecimation_Mode = DECIMATION_NONE;
	if (m_propertyInfo.bEnableDecimation)
    {
        if (_CaptureStart_for_Decimation() != IBSU_STATUS_OK)
            return IBSU_ERR_INVALID_PARAM_VALUE;
    }
	else
	{
		if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)		// GON
		{
			if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			{
				m_FullCaptureSetting.forSuperDry_Mode = m_propertyInfo.nSuperDryMode;

				m_FullCaptureSetting.DefaultMinDAC = _DAC_FOR_MINIMUM_;
				m_FullCaptureSetting.DefaultMaxDAC = _DAC_FOR_MAXIMUM_;
				m_FullCaptureSetting.DefaultBitshift = _NO_BIT_BITSHIFT_FOR_FULLFRAME_ROLL_;
				m_FullCaptureSetting.DefaultCutThres = _NO_BIT_CUTTHRES_FOR_FULLFRAME_ROLL_;

				if(m_FullCaptureSetting.forSuperDry_Mode)
				{
					m_FullCaptureSetting.TargetMinBrightness = _TARGET_MIN_BRIGHTNESS_FOR_DRY_;
					m_FullCaptureSetting.TargetMaxBrightness = _TARGET_MAX_BRIGHTNESS_FOR_DRY_;
				}
				else
				{
					m_FullCaptureSetting.TargetMinBrightness = m_propertyInfo.nCaptureBrightThresRoll[0];
					m_FullCaptureSetting.TargetMaxBrightness = m_propertyInfo.nCaptureBrightThresRoll[1];
				}

				m_FullCaptureSetting.TargetFingerCount = 1;
				if(m_propertyInfo.nSuperDryMode)
				{
					m_FullCaptureSetting.DefaultDAC = _DAC_FOR_DRY_ONE_FINGER_;
				}
				else
				{
					if(m_propertyInfo.bEnableStartingVoltage == TRUE)
					{
						m_FullCaptureSetting.DefaultDAC = m_propertyInfo.nStartingVoltage;
					}
					else
					{
						if(m_UsbDeviceInfo.bCanUseTOF &&
							m_propertyInfo.bEnableTOF && 
							m_propertyInfo.bEnableTOFforROLL)
						{
							m_FullCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FINGER_WITH_TOF_;
						}
						else if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_NEW_BRD_AND_DOUBLE_P_ ||
								m_propertyInfo.bKojakPLwithDPFilm == TRUE)
						{
							m_FullCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_OF_DOUBLE_P_;
						}
						else
						{
							m_FullCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_;
						}
					}
				}
				m_FullCaptureSetting.CurrentDAC = m_FullCaptureSetting.DefaultDAC;
				m_FullCaptureSetting.DefaultGain = _NO_BIT_GAIN_FOR_FULLFRAME_;
				m_FullCaptureSetting.CurrentGain = m_FullCaptureSetting.DefaultGain;
				m_FullCaptureSetting.DefaultExposure = _DEFAULT_EXPOSURE_FOR_FULLFRAME_ROLL_;
				m_FullCaptureSetting.CurrentExposure = m_FullCaptureSetting.DefaultExposure;
				m_FullCaptureSetting.CurrentBitshift = m_FullCaptureSetting.DefaultBitshift;
				m_FullCaptureSetting.CurrentCutThres = m_FullCaptureSetting.DefaultCutThres;
				m_FullCaptureSetting.AdditionalFrame = 1;
				m_FullCaptureSetting.RollCaptureMode = TRUE;
				m_FullCaptureSetting.DetectionFrameCount = 0;

				m_DeciCaptureSetting = m_FullCaptureSetting;

				m_UsbDeviceInfo.bDecimation = FALSE;
				_ChangeDecimationModeForKojak(FALSE, &m_FullCaptureSetting, TRUE);

				m_CurrentCaptureDAC = m_FullCaptureSetting.CurrentDAC;
			}
			else if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
					m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
					m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS ||
					m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS )
			{
				m_FullCaptureSetting.forSuperDry_Mode = m_propertyInfo.nSuperDryMode;

				m_FullCaptureSetting.DefaultMinDAC = _DAC_FOR_MINIMUM_;
				m_FullCaptureSetting.DefaultMaxDAC = _DAC_FOR_MAXIMUM_;
				if(m_FullCaptureSetting.forSuperDry_Mode)
				{
					m_FullCaptureSetting.TargetMinBrightness = _TARGET_MIN_BRIGHTNESS_FOR_DRY_;
					m_FullCaptureSetting.TargetMaxBrightness = _TARGET_MAX_BRIGHTNESS_FOR_DRY_;
				}
				else
				{
					m_FullCaptureSetting.TargetMinBrightness = m_propertyInfo.nCaptureBrightThresFlat[0];
					m_FullCaptureSetting.TargetMaxBrightness = m_propertyInfo.nCaptureBrightThresFlat[1];
				}
				m_FullCaptureSetting.TargetFingerCount = m_propertyInfo.nNumberOfObjects;
				if(m_propertyInfo.nSuperDryMode)
				{
					if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER)
						m_FullCaptureSetting.DefaultDAC = _DAC_FOR_DRY_ONE_FINGER_;
					else if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS)
						m_FullCaptureSetting.DefaultDAC = _DAC_FOR_DRY_TWO_FINGER_;
					else
						m_FullCaptureSetting.DefaultDAC = _DAC_FOR_DRY_FOUR_FINGER_;
				}
				else
				{
					if(m_propertyInfo.bEnableStartingVoltage == TRUE)
					{
						m_FullCaptureSetting.DefaultDAC = m_propertyInfo.nStartingVoltage;
					}
					else
					{
						if(m_UsbDeviceInfo.bCanUseTOF &&
							m_propertyInfo.bEnableTOF)
						{
							m_FullCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FINGER_WITH_TOF_;
						}
						else if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_NEW_BRD_AND_DOUBLE_P_ ||
								m_propertyInfo.bKojakPLwithDPFilm == TRUE)
						{
							if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS || 
								m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
								m_FullCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_OF_DOUBLE_P_;
							else
								m_FullCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FOUR_FINGER_OF_DOUBLE_P_;
						}
						else
						{
							if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS || 
								m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
								m_FullCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_;
							else
								m_FullCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FOUR_FINGER_;
						}
					}
				}

				m_FullCaptureSetting.CurrentDAC = m_FullCaptureSetting.DefaultDAC;
				m_FullCaptureSetting.DefaultGain = _DEFAULT_GAIN_FOR_DECI_;
				m_FullCaptureSetting.CurrentGain = m_FullCaptureSetting.DefaultGain;
				m_FullCaptureSetting.DefaultExposure = _NO_BIT_EXPOSURE_FOR_FULLFRAME_;
				m_FullCaptureSetting.CurrentExposure = m_FullCaptureSetting.DefaultExposure;
				m_FullCaptureSetting.DefaultBitshift = _NO_BIT_BITSHIFT_FOR_FULLFRAME_;
				m_FullCaptureSetting.CurrentBitshift = m_FullCaptureSetting.DefaultBitshift;
				m_FullCaptureSetting.DefaultCutThres = _NO_BIT_CUTTHRES_FOR_FULLFRAME_;
				m_FullCaptureSetting.CurrentCutThres = m_FullCaptureSetting.DefaultCutThres;
				m_DeciCaptureSetting.AdditionalFrame = 0;
				m_DeciCaptureSetting.RollCaptureMode = FALSE;
				m_DeciCaptureSetting.DetectionFrameCount = 0;

				m_DeciCaptureSetting = m_FullCaptureSetting;

				m_UsbDeviceInfo.bDecimation = FALSE;
				_ChangeDecimationModeForKojak(FALSE, &m_FullCaptureSetting, TRUE);

				m_CurrentCaptureDAC = m_FullCaptureSetting.CurrentDAC;
			}
		}
	}

	if (m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0)		// GON
	{
		m_propertyInfo.nLEOperationMode = ADDRESS_LE_OFF_MODE;

		if(imageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
			m_FullCaptureSetting.forSuperDry_Mode = m_propertyInfo.nSuperDryMode;
			m_FullCaptureSetting.DACPowerOffThreshold = 1;

			m_FullCaptureSetting.DefaultMinDAC = _DAC_FOR_MINIMUM_FIVE0;
			m_FullCaptureSetting.DefaultMaxDAC = _DAC_FOR_MAXIMUM_FIVE0;
			//m_FullCaptureSetting.DefaultBitshift = _NO_BIT_BITSHIFT_FOR_FULLFRAME_;
			//m_FullCaptureSetting.DefaultCutThres = _NO_BIT_CUTTHRES_FOR_FULLFRAME_;

			if(m_FullCaptureSetting.forSuperDry_Mode)
			{
				m_FullCaptureSetting.TargetMinBrightness = _TARGET_MIN_BRIGHTNESS_FOR_DRY_FIVE0;
				m_FullCaptureSetting.TargetMaxBrightness = _TARGET_MAX_BRIGHTNESS_FOR_DRY_FIVE0;
			}
			else
			{
				m_FullCaptureSetting.TargetMinBrightness = m_propertyInfo.nCaptureBrightThresRoll[0];
				m_FullCaptureSetting.TargetMaxBrightness = m_propertyInfo.nCaptureBrightThresRoll[1];
			}

			m_FullCaptureSetting.TargetFingerCount = 1;
			
			if(m_propertyInfo.bEnableStartingVoltage == TRUE)
			{
				m_FullCaptureSetting.DefaultDAC = m_propertyInfo.nStartingVoltage;
			}
			else
			{
				if(m_propertyInfo.nSuperDryMode)
				{
					m_FullCaptureSetting.DefaultDAC = _DAC_FOR_DRY_ONE_FINGER_FIVE0;
				}
				else
				{
					if(m_propertyInfo.bEnableTOF && m_propertyInfo.bEnableTOFforROLL)
					{
						m_FullCaptureSetting.DefaultDAC = _DAC_FOR_TOF_FIVE0_;
					}
					else
					{
						if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_FIVE0_DOUBLE_P_)
							m_FullCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_FIVE0_DOUBLE_P_;
						else
							m_FullCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_FIVE0;
					}
				}
			}

			m_FullCaptureSetting.CurrentDAC = m_FullCaptureSetting.DefaultDAC;
			//m_FullCaptureSetting.DefaultGain = _NO_BIT_GAIN_FOR_FULLFRAME_;
			//m_FullCaptureSetting.CurrentGain = m_FullCaptureSetting.DefaultGain;
			//m_FullCaptureSetting.DefaultExposure = _DEFAULT_EXPOSURE_FOR_FULLFRAME_ROLL_;
			//m_FullCaptureSetting.CurrentExposure = m_FullCaptureSetting.DefaultExposure;
			//m_FullCaptureSetting.CurrentBitshift = m_FullCaptureSetting.DefaultBitshift;
			//m_FullCaptureSetting.CurrentCutThres = m_FullCaptureSetting.DefaultCutThres;
			m_FullCaptureSetting.AdditionalFrame = 1;
			m_FullCaptureSetting.RollCaptureMode = TRUE;
			m_FullCaptureSetting.DetectionFrameCount = 0;

			m_DeciCaptureSetting = m_FullCaptureSetting;

			m_UsbDeviceInfo.bDecimation = FALSE;
		}
		else if(imageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
				imageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
				imageType == ENUM_IBSU_FLAT_FOUR_FINGERS ||
				imageType == ENUM_IBSU_FLAT_THREE_FINGERS )
		{
			m_DeciCaptureSetting.forSuperDry_Mode = m_propertyInfo.nSuperDryMode;
			m_DeciCaptureSetting.DACPowerOffThreshold = 1;

			m_DeciCaptureSetting.DefaultMinDAC = _DAC_FOR_MINIMUM_FIVE0;
			m_DeciCaptureSetting.DefaultMaxDAC = _DAC_FOR_MAXIMUM_FIVE0;
			if(m_DeciCaptureSetting.forSuperDry_Mode)
			{
				m_DeciCaptureSetting.TargetMinBrightness = _TARGET_MIN_BRIGHTNESS_FOR_DRY_FIVE0;
				m_DeciCaptureSetting.TargetMaxBrightness = _TARGET_MAX_BRIGHTNESS_FOR_DRY_FIVE0;
			}
			else
			{
				m_DeciCaptureSetting.TargetMinBrightness = m_propertyInfo.nCaptureBrightThresFlat[0];
				m_DeciCaptureSetting.TargetMaxBrightness = m_propertyInfo.nCaptureBrightThresFlat[1];
			}
			m_DeciCaptureSetting.TargetFingerCount = numberOfObjects;
			
			if(m_propertyInfo.bEnableStartingVoltage == TRUE)
			{
				m_DeciCaptureSetting.DefaultDAC = m_propertyInfo.nStartingVoltage;
			}
			else
			{
				if(m_propertyInfo.nSuperDryMode)
				{
					if(imageType == ENUM_IBSU_FLAT_SINGLE_FINGER)
						m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_DRY_ONE_FINGER_FIVE0;
					else if(imageType == ENUM_IBSU_FLAT_TWO_FINGERS)
						m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_DRY_TWO_FINGER_FIVE0;
					else
						m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_DRY_FOUR_FINGER_FIVE0;
				}
				else
				{
					if( m_propertyInfo.bEnableTOF &&
                        m_UsbDeviceInfo.bCanUseTOF
					)
					{
						m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_TOF_FIVE0_;
					}
					else
					{
						if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_FIVE0_DOUBLE_P_)
						{
							if(imageType == ENUM_IBSU_FLAT_THREE_FINGERS || 
								imageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
								m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_FIVE0_DOUBLE_P_;
							else
								m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FOUR_FINGER_FIVE0_DOUBLE_P_;
						}
						else
						{
							if(imageType == ENUM_IBSU_FLAT_THREE_FINGERS || 
								imageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
								m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_FIVE0;
							else
								m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FOUR_FINGER_FIVE0;
						}
					}
				}
			}
			m_DeciCaptureSetting.CurrentDAC = m_DeciCaptureSetting.DefaultDAC;
			m_DeciCaptureSetting.DefaultGain = _DEFAULT_GAIN_FOR_DECI_;
			m_DeciCaptureSetting.CurrentGain = m_DeciCaptureSetting.DefaultGain;
			m_DeciCaptureSetting.DefaultExposure = _DEFAULT_EXPOSURE_FOR_DECI_;
			m_DeciCaptureSetting.CurrentExposure = m_DeciCaptureSetting.DefaultExposure;
			m_DeciCaptureSetting.DefaultBitshift = _DEFAULT_BITSHIFT_FOR_DECI_;
			m_DeciCaptureSetting.CurrentBitshift = m_DeciCaptureSetting.DefaultBitshift;
			m_DeciCaptureSetting.DefaultCutThres = _DEFAULT_CUTTHRES_FOR_DECI_;
			m_DeciCaptureSetting.CurrentCutThres = m_DeciCaptureSetting.DefaultCutThres;
			m_DeciCaptureSetting.AdditionalFrame = 1;
			m_DeciCaptureSetting.RollCaptureMode = FALSE;
			m_DeciCaptureSetting.DetectionFrameCount = 0;

			m_FullCaptureSetting = m_DeciCaptureSetting;
			m_FullCaptureSetting.DefaultExposure = _NO_BIT_EXPOSURE_FOR_FULLFRAME_;
			m_FullCaptureSetting.DefaultBitshift = _NO_BIT_BITSHIFT_FOR_FULLFRAME_;
			m_FullCaptureSetting.DefaultCutThres = _NO_BIT_CUTTHRES_FOR_FULLFRAME_;
			m_FullCaptureSetting.CurrentExposure = m_FullCaptureSetting.DefaultExposure;
			m_FullCaptureSetting.CurrentBitshift = m_FullCaptureSetting.DefaultBitshift;
			m_FullCaptureSetting.CurrentCutThres = m_FullCaptureSetting.DefaultCutThres;
		}
	}

    //	if( m_UsbDeviceInfo.bEnableOperationLE == FALSE )
    //		_FPGA_SetRegister(0x25, 0x00);	// LE OFF

	if(m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
	{
/*		Capture_SetLEVoltage(__DEFAULT_VOLTAGE_VALUE_FOR_KOJAK__);
		m_propertyInfo.nVoltageValue = __DEFAULT_VOLTAGE_VALUE_FOR_KOJAK__;
		if (m_UsbDeviceInfo.LeVoltageControl == SUPPORT_LE_VOLTAGE_CONTROL)
		{
			Capture_SetLEVoltage(__DEFAULT_VOLTAGE_VALUE_FOR_KOJAK__);
		}*/
	}
	else
	{
		Capture_SetLEVoltage(__DEFAULT_VOLTAGE_VALUE__);
		m_propertyInfo.nVoltageValue = __DEFAULT_VOLTAGE_VALUE__;
		if (m_UsbDeviceInfo.LeVoltageControl == SUPPORT_LE_VOLTAGE_CONTROL)
		{
			Capture_SetLEVoltage(12);
			Capture_SetLEVoltage(13);
			Capture_SetLEVoltage(14);
			Capture_SetLEVoltage(15);
			Capture_SetLEVoltage(__DEFAULT_VOLTAGE_VALUE__);
		}
	}

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // enzyme add 2012-11-22 To check communication between FPGA and CIS
    if (m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK || m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK_ROIC)
    {
        m_propertyInfo.nLEOperationMode = ADDRESS_LE_OFF_MODE;
		if (m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK && m_UsbDeviceInfo.bNewFPCB)
		{
			ASIC_SetRegister(ASIC_MASTER_ADDRESS, 0x43, m_MasterValue);
			ASIC_SetRegister(ASIC_SLAVE_ADDRESS, 0x43, m_SlaveValue);
		}
        memset(m_pAlgo->m_TFT_MaskImg, 0, m_UsbDeviceInfo.CisImgSize);
    }

    _SetLEOperationMode((WORD)m_propertyInfo.nLEOperationMode);

    if (m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE)
    {
        _PowerCaptureInit_Curve(&m_pAlgo->m_CaptureInfo, &m_propertyInfo);
    }

	if (m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE_SETI)
    {
        _PowerCaptureInit_CurveSETi(&m_pAlgo->m_CaptureInfo, &m_propertyInfo);
    }

    // Used default contrast value for all of capture mode (auto, manual)
    m_propertyInfo.nContrastValue = m_UsbDeviceInfo.nDefaultContrastValue;
    _Set_CIS_GainRegister(m_propertyInfo.nContrastValue, FALSE);

    if (!bReserved)
    {
		if (m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0)
			_FPGA_SetRegister(0x2F, 0x01);

        if (!_GetOneFrameImage(m_pAlgo->m_ImgFromCIS, m_UsbDeviceInfo.CisImgSize))
        {
            if (m_propertyInfo.bEnablePowerSaveMode == TRUE)
            {
                _SndUsbFwCaptureStop();
            }
            return IBSU_ERR_CAPTURE_COMMAND_FAILED;
        }

        if (_CheckForClearPlaten(m_pAlgo->m_ImgFromCIS) == 1)
        {
            m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_CLEARD;
        }
		else
		{
			if (m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0)
			{
//				_FPGA_SetRegister(0x27, 0x00); // LE off from FPGA
                m_propertyInfo.nLEOperationMode = ADDRESS_LE_OFF_MODE;
                _SetLEOperationMode(m_propertyInfo.nLEOperationMode);
				m_nFrameCount = -2;
			}
		}

        // 2013-05-03 enzyme add - If you change the gain register on Curve,
        // the gain register must be changed immediately after getting one frame image.
        // If not, Curve CIS do not understand the setting gain register.
        if (m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE)
        {
            _Set_CIS_GainRegister(m_propertyInfo.nContrastValue, FALSE);
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    _InitializeCaptureVariables();

	// Is it OK?
    //m_bFirstSent_clbkClearPlaten = FALSE;

    if (!bReserved && !bRawCapture)
    {
        // RESERVED_BeginCaptureImage does not use Capture Thread..
#ifdef _WINDOWS
        //	ResetEvent(m_hCaptureThread_DummyEvent);
        SetEvent(m_hCaptureThread_CaptureEvent);
#else
        //	m_bCaptureThread_DummyEvent = FALSE;
        m_bCaptureThread_CaptureEvent = TRUE;
#endif
        tries = 0;
        do
        {
            Sleep(10);
            if (tries++ > 100)
            {
                if (m_propertyInfo.bEnablePowerSaveMode == TRUE)
                {
                    _SndUsbFwCaptureStop();
                }
                return IBSU_ERR_CAPTURE_STILL_RUNNING;
            }
        }
        while (!m_bIsActiveCapture);
    }

    memset(&m_blockProperty, 0, sizeof(m_blockProperty));
    memset(&m_clbkProperty.qualityArray, ENUM_IBSU_QUALITY_GOOD, sizeof(m_clbkProperty.qualityArray));
    // For IBSU_BGetRollingInfo function
    m_pAlgo->m_rollingStatus = 0;
    m_pAlgo->g_LastX = -1;
    m_nGoodFrameCount = 0;
    m_timeAfterBeginCaptureImage = 0;
    m_nRemainedDecimationFrameCount = __DEFAULT_COLUMBO_DECIMATION_COUNT__;
	if(m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
    {
		m_nRemainedDecimationFrameCount = __DEFAULT_KOJAK_DECIMATION_COUNT__;
    }
    m_SavedFingerCountStatus = -1;
	m_bFirstPutFingerOnSensor = FALSE;
    m_bIsActiveCapture = TRUE;

    /* To support ENUM_IBSU_PROPERTY_CAPTURE_TIMEOUT */
    m_pAlgo->m_cImgAnalysis.nAccumulated_Captured_Elapsed = 0;
    gettimeofday(&m_start_tv, NULL);
    m_clbkProperty.imageInfo.FrameTime = m_UsbDeviceInfo.nMIN_Elapsed;

	/* To support ENUM_IBSU_PROPERTY_ENABLE_CAPTURE_ON_RELEASE */
	m_pAlgo->m_BestFrame.score = 0;

    if (bRawCapture)
    {
        m_propertyInfo.bRawCaptureStart = TRUE;
    }
    else
    {
        m_propertyInfo.bRawCaptureStart = FALSE;
    }

	if ( m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK &&
		m_UsbDeviceInfo.bCanUseTOF &&
		m_propertyInfo.bEnableTOF &&
		(m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER ||
		(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_propertyInfo.bEnableTOFforROLL)) )
    {
        memset(m_TOFArr, 0, sizeof(m_TOFArr));
        m_TOFArr_idx=0;

        _Start_TOF();
    }

    return IBSU_STATUS_OK;
}

int CMainCapture::Capture_Abort()
{
    // Already aborted capture
    if (!m_bIsActiveCapture)
    {
        return IBSU_ERR_CAPTURE_NOT_RUNNING;
    }

    if (!IsCaptureSetMode())
    {
        return IBSU_ERR_CAPTURE_INVALID_MODE;
    }

    m_bCaptureThread_StopMessage = TRUE;
    m_bCaptureThread_DummyStopMessage = FALSE;

    if (m_pCaptureThread)
    {
        int tries = 0;

        // Commented out because of error on Windows.  Is this even necessary?  Why is it here then?
#if 0
#ifdef _WINDOWS
        ResetEvent(m_hCaptureThread_CaptureEvent);
        SetEvent(m_hCaptureThread_DummyEvent);
#else
        m_bCaptureThread_CaptureEvent = FALSE;
        m_bCaptureThread_DummyEvent = TRUE;
#endif
#endif

#if defined(__IBSCAN_SDK__)
        if (m_propertyInfo.bRawCaptureStart)
        {
#ifdef _WINDOWS
            SetEvent(m_hRawCaptureThread_AbortEvent);
#else
            m_bRawCaptureThread_AbortEvent = TRUE;
#endif
        }
#endif

        do
        {
            Sleep(10);
            if (tries++ > 100)
            {
                return IBSU_ERR_CAPTURE_STOP;
            }
        }
        while (m_bIsActiveCapture);
    }

    // 2013-05-03 enzyme delete - Don't add any USB bulk function during working the dummy/capture Thread in the Curve USB driver(old).
    // Because of the Curve USB driver(old) does not allow to communicate USB bulk at same time.
    // If you add critical section ("CThreadSync Sync;") on the bulk function, you can solve this issue.
    // However, it makes slow the frame rate on the multi-devices environment.
    // So I do not use this way until we change USB driver from Curve driver(old) to WinUsbDriver for Curve device
    // Is alive USB device
    //	if( !_IsAlive_UsbDevice() )
    //		return IBSU_ERR_DEVICE_IO;

    //	_SetLEOperationMode(ADDRESS_LE_OFF_MODE);
    //	if( m_propertyInfo.dwActiveLEDs == IBSU_LED_NONE )
    //		_SndUsbFwCaptureStop();

    m_bIsActiveCapture = FALSE;
    m_blockProperty.bGetOneFrameImage = FALSE;
    m_blockProperty.bFillResultImage = FALSE;
    m_propertyInfo.bRawCaptureStart = FALSE;

    return IBSU_STATUS_OK;
}

int CMainCapture::Capture_SetMode(const IBSU_ImageType imageType,
                                  const IBSU_ImageResolution imageResolution,
                                  //								const IBSU_ImageOrientation lineOrder,
                                  const DWORD captureOptions,
                                  int *resultWidth, int *resultHeight,
                                  int	*baseResolutionX, int *baseResolutionY)
{
    m_bIsCaptureSetMode = FALSE;

    if (imageResolution != ENUM_IBSU_IMAGE_RESOLUTION_500)
    {
        return IBSU_ERR_CAPTURE_INVALID_MODE;
    }

    if (_CheckForSupportedImageType(imageType, m_UsbDeviceInfo.devType) != IBSU_STATUS_OK)
    {
        return IBSU_ERR_CAPTURE_INVALID_MODE;
    }

#if defined(__IBSCAN_SDK__)
    switch (imageType)
    {
    case IBSCAN_ROLL_SINGLE_FINGER:
        m_propertyInfo.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
        break;
    case IBSCAN_FLAT_SINGLE_FINGER:
    case IBSCAN_SINGLE_FINGER:
        m_propertyInfo.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
        break;
    case IBSCAN_FLAT_TWO_THUMBS:
    case IBSCAN_FLAT_TWO_FINGERS:
    case IBSCAN_TWO_FINGERS:
        m_propertyInfo.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
        break;
    default:
        return IBSU_ERR_CAPTURE_INVALID_MODE;
    }
#elif defined(__IBSCAN_ULTIMATE_SDK__)
    m_propertyInfo.ImageType = imageType;
#else
    m_propertyInfo.ImageType = imageType;
#endif
    m_propertyInfo.nCaptureOptions = (int)captureOptions;

    *resultWidth = m_UsbDeviceInfo.ImgWidth;
    *resultHeight = m_UsbDeviceInfo.ImgHeight;
    *baseResolutionX = m_UsbDeviceInfo.baseResolutionX;
    *baseResolutionY = m_UsbDeviceInfo.baseResolutionY;

    m_bIsCaptureSetMode = TRUE;

    return IBSU_STATUS_OK;
}

#if defined(__IBSCAN_SDK__)
int CMainCapture::Capture_GetMode(IBSU_ImageType *imageType,
                                  IBSU_ImageResolution *imageResolution,
                                  DWORD *captureOptions,
                                  const int numberOfObjects)
{
    if (!m_bIsCaptureSetMode)
    {
        return IBSU_ERR_CAPTURE_INVALID_MODE;
    }

    if (numberOfObjects < 1 || numberOfObjects > 2)
    {
        return IBSU_ERR_INVALID_PARAM_VALUE;
    }

    if (numberOfObjects == 1 &&
        m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER &&
        m_propertyInfo.ImageType != ENUM_IBSU_FLAT_SINGLE_FINGER)
    {
        return IBSU_ERR_CAPTURE_INVALID_MODE;
    }
    else if (numberOfObjects == 2 &&
        m_propertyInfo.ImageType != ENUM_IBSU_FLAT_TWO_FINGERS)
    {
        return IBSU_ERR_CAPTURE_INVALID_MODE;
    }

    *imageType = m_propertyInfo.ImageType;
    *imageResolution = ENUM_IBSU_IMAGE_RESOLUTION_500;
    *captureOptions = m_propertyInfo.nCaptureOptions;

    return IBSU_STATUS_OK;
}
#endif

int CMainCapture::Capture_IsModeAvailable(const IBSU_ImageType imageType,
        const IBSU_ImageResolution imageResolution,
        BOOL *isAvailable)
{
    *isAvailable = FALSE;
    if (imageResolution != ENUM_IBSU_IMAGE_RESOLUTION_500)
    {
        return IBSU_STATUS_OK;
    }

    if (_CheckForSupportedImageType(imageType, m_UsbDeviceInfo.devType) == IBSU_STATUS_OK)
    {
        *isAvailable = TRUE;
    }

    return IBSU_STATUS_OK;
}

int CMainCapture::Capture_IsActive(BOOL *isActive)
{
    //	CThreadSync Sync;
    *isActive = m_bIsActiveCapture;

    return IBSU_STATUS_OK;
}

int CMainCapture::Capture_TakeResultImageManually(BOOL isSet)
{
    if (!m_bIsActiveCapture)
    {
        return IBSU_ERR_CAPTURE_NOT_RUNNING;
    }

    if (!IsCaptureSetMode())
    {
        return IBSU_ERR_CAPTURE_INVALID_MODE;
    }

    SetTakeResultImageManually(isSet);

    return IBSU_STATUS_OK;
}

int	CMainCapture::Capture_GetContrast(int *contrastValue)
{
    //	int		nRc;
    //	DWORD	nValue;

#if defined(__IBSCAN_SDK__)
    if (!IsCaptureSetMode())
    {
        return IBSU_ERR_CAPTURE_INVALID_MODE;
    }
#endif

    // enzyme 2013-02-05 add - To remove code "CThreadSync Sync;" on _GetOneFrameImage_Curve()
    // Then the  _GetOneFrameImage speed of other devices will be improved at multi-devices
    if (_GetPID() == __PID_CURVE__)
    {
        if (m_propertyInfo.bEnablePowerSaveMode == TRUE && GetIsActiveCapture() == FALSE)
        {
            return IBSU_ERR_DEVICE_ENABLED_POWER_SAVE_MODE;
        }

        *contrastValue = m_propertyInfo.nContrastValue;

        return IBSU_STATUS_OK;
    }

    // Is alive USB device
    if (!_IsAlive_UsbDevice())
    {
        return IBSU_ERR_DEVICE_IO;
    }

#if defined(__IBSCAN_ULTIMATE_SDK__)
    if (m_propertyInfo.bEnablePowerSaveMode == TRUE && GetIsActiveCapture() == FALSE)
    {
        return IBSU_ERR_DEVICE_ENABLED_POWER_SAVE_MODE;
    }
#endif

    *contrastValue = m_propertyInfo.nContrastValue;

    return IBSU_STATUS_OK;
}

int	CMainCapture::Capture_SetContrast(int contrastValue)
{
    //	DWORD	nValue;
    int		nRc;

#if defined(__IBSCAN_SDK__)
    if (!IsCaptureSetMode())
    {
        return IBSU_ERR_CAPTURE_INVALID_MODE;
    }
#endif

    if (contrastValue < 0 || contrastValue > IBSU_MAX_CONTRAST_VALUE)
    {
        return IBSU_ERR_INVALID_PARAM_VALUE;
    }

#if defined(__IBSCAN_ULTIMATE_SDK__)
    if (m_propertyInfo.bEnablePowerSaveMode == TRUE && GetIsActiveCapture() == FALSE)
    {
        return IBSU_ERR_DEVICE_ENABLED_POWER_SAVE_MODE;
    }
#endif

    // enzyme 2013-02-05 add - To remove code "CThreadSync Sync;" on _GetOneFrameImage_Curve()
    // Then the  _GetOneFrameImage speed of other devices will be improved at multi-devices
    if (_GetPID() == __PID_CURVE__)
    {
        if (GetIsActiveCapture() == FALSE && GetIsActiveDummy() == FALSE)
        {
            if ((nRc = _Set_CIS_GainRegister(contrastValue, FALSE)) != IBSU_STATUS_OK)
            {
                return nRc;
            }
        }

        m_propertyInfo.nContrastValue = contrastValue;

        return IBSU_STATUS_OK;
    }

    // RGB gain
    //	nValue = _GetContrast_FromGainTable(contrastValue);
    //	if( (nRc = _MT9M_SetRegister(MT9M_GLOBAL_GAIN, (WORD)nValue)) != IBSU_STATUS_OK )
    if ((nRc = _Set_CIS_GainRegister(contrastValue, FALSE, TRUE)) != IBSU_STATUS_OK)
    {
        return nRc;
    }

    m_propertyInfo.nContrastValue = contrastValue;

    return IBSU_STATUS_OK;
}

int	CMainCapture::Capture_SetLEVoltage(int voltageValue)
{
    if (m_UsbDeviceInfo.bEnableELVoltage == FALSE)
    {
        return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
    }

	if (voltageValue < 0 || voltageValue > 15)
	{
		if( !(m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK && 
			m_UsbDeviceInfo.nFpgaVersion >= ((0 << 24) | (9 << 16) | (0 << 8) | 0x0D) ) )			// GON
		return IBSU_ERR_INVALID_PARAM_VALUE;
	}

    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
            return Capture_SetLEVoltage_Watson(voltageValue);
        case DEVICE_TYPE_WATSON_MINI:
            return Capture_SetLEVoltage_WatsonMini(voltageValue);
        case DEVICE_TYPE_SHERLOCK_ROIC:
        case DEVICE_TYPE_SHERLOCK:
            return Capture_SetLEVoltage_Sherlock(voltageValue);
        case DEVICE_TYPE_COLUMBO:
            Capture_SetLEVoltage_Columbo(voltageValue);		// Bug fixed due to the DAC initialize issue
            return Capture_SetLEVoltage_Columbo(voltageValue);
        case DEVICE_TYPE_CURVE:
            return Capture_SetLEVoltage_Curve(voltageValue);
		case DEVICE_TYPE_CURVE_SETI:
            return Capture_SetLEVoltage_CurveSETi(voltageValue);
        case DEVICE_TYPE_HOLMES:
            return Capture_SetLEVoltage_Holmes(voltageValue);
		case DEVICE_TYPE_KOJAK:
            return Capture_SetLEVoltage_Kojak(voltageValue);
        case DEVICE_TYPE_FIVE0:
            return Capture_SetLEVoltage_Five0(voltageValue);
        default:
            break;
    }

    return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
}

int	CMainCapture::Capture_GetLEVoltage(int *voltageValue)
{
    if (m_UsbDeviceInfo.bEnableELVoltage == FALSE)
    {
        return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
    }

    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
            return Capture_GetLEVoltage_Watson(voltageValue);
        case DEVICE_TYPE_WATSON_MINI:
            return Capture_GetLEVoltage_WatsonMini(voltageValue);
        case DEVICE_TYPE_SHERLOCK_ROIC:
        case DEVICE_TYPE_SHERLOCK:
            return Capture_GetLEVoltage_Sherlock(voltageValue);
        case DEVICE_TYPE_COLUMBO:
            return Capture_GetLEVoltage_Columbo(voltageValue);
        case DEVICE_TYPE_CURVE:
            return Capture_GetLEVoltage_Curve(voltageValue);
		case DEVICE_TYPE_CURVE_SETI:
            return Capture_GetLEVoltage_CurveSETi(voltageValue);
        case DEVICE_TYPE_HOLMES:
            return Capture_GetLEVoltage_Holmes(voltageValue);
		case DEVICE_TYPE_KOJAK:
            return Capture_GetLEVoltage_Kojak(voltageValue);
        case DEVICE_TYPE_FIVE0:
            return Capture_GetLEVoltage_Five0(voltageValue);
        default:
            break;
    }

    return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
}

int	CMainCapture::Capture_GetLEOperationMode(IBSU_LEOperationMode *leOperationMode)
{
    //	if( !IsCaptureSetMode() )
    //		return IBSU_ERR_CAPTURE_INVALID_MODE;

    // Is alive USB device
    if (!_IsAlive_UsbDevice())
    {
        return IBSU_ERR_DEVICE_IO;
    }

    /*
    // enzyme modification because cypress has a problem about power management
    	DWORD	nValue;
    	if( !_GetLEOperationMode(NULL, (WORD&)nValue) )
    		return IBSU_ERR_DEVICE_IO;

    	*leOperationMode = (IBSU_LEOperationMode)nValue;
    	m_propertyInfo.nLEOperationMode = nValue;
    */

    if (m_propertyInfo.bEnablePowerSaveMode == TRUE && GetIsActiveCapture() == FALSE)
    {
        return IBSU_ERR_DEVICE_ENABLED_POWER_SAVE_MODE;
    }

    if (m_UsbDeviceInfo.bEnableOperationLE == FALSE)
    {
        return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
    }

    *leOperationMode = (IBSU_LEOperationMode)m_propertyInfo.nLEOperationMode;

    return IBSU_STATUS_OK;
}

int	CMainCapture::Capture_SetLEOperationMode(IBSU_LEOperationMode leOperationMode)
{
    DWORD	nValue;
    int		nRc;

    //	if( !IsCaptureSetMode() )
    //		return IBSU_ERR_CAPTURE_INVALID_MOE;

    // Is alive USB device
    if (!_IsAlive_UsbDevice())
    {
        return IBSU_ERR_DEVICE_IO;
    }

    nValue = (DWORD)leOperationMode;

    if (m_propertyInfo.bEnablePowerSaveMode == TRUE && GetIsActiveCapture() == FALSE)
    {
        return IBSU_ERR_DEVICE_ENABLED_POWER_SAVE_MODE;
    }

    if (m_UsbDeviceInfo.bEnableOperationLE == FALSE)
    {
        return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
    }

    //////////////////////////////////////////////////////////////////
    // enzyme modify 2012-11-26 Bug fixed on Watson Mini
    // 2013-06-12 enzyme modify to support TBN240
    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
            break;
        case DEVICE_TYPE_SHERLOCK:
        case DEVICE_TYPE_SHERLOCK_ROIC:
        case DEVICE_TYPE_WATSON_MINI:
        case DEVICE_TYPE_COLUMBO:
        case DEVICE_TYPE_HOLMES:
		case DEVICE_TYPE_KOJAK:
        case DEVICE_TYPE_FIVE0:
            if (leOperationMode == ENUM_IBSU_LE_OPERATION_AUTO)
            {
                return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;    // Does not have touch sensor
            }
            break;
        case DEVICE_TYPE_CURVE:
            if (m_UsbDeviceInfo.curveModel < CURVE_MODEL_TBN240)
            {
                return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;    // Does not have touch sensor
            }
            else if (m_UsbDeviceInfo.curveModel == CURVE_MODEL_TBN240 && 
                     leOperationMode == ENUM_IBSU_LE_OPERATION_OFF)
            {
                return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;    // Does not support LE power off
            }
            else
            {
                break;
            }
		case DEVICE_TYPE_CURVE_SETI:
            if (m_UsbDeviceInfo.curveModel == CURVE_MODEL_TBN320 || m_UsbDeviceInfo.curveModel == CURVE_MODEL_TBN320_V1)
            {
                return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;    // Does not have touch sensor
            }
            else if ((m_UsbDeviceInfo.curveModel == CURVE_MODEL_TBN340 || m_UsbDeviceInfo.curveModel == CURVE_MODEL_TBN340_V1) &&
                     leOperationMode == ENUM_IBSU_LE_OPERATION_OFF)
            {
                return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;    // Does not support LE power off
            }
            else
            {
                break;
            }
        default:
            return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
    }

    if ((nRc = _SetLEOperationMode((WORD)nValue)) != IBSU_STATUS_OK)
    {
        return nRc;
    }
    //////////////////////////////////////////////////////////////////

    m_propertyInfo.nLEOperationMode = nValue;

    return IBSU_STATUS_OK;
}

int CMainCapture::RegisterCallback(CallbackType clbkIndex, void *callback, void *context)
{
    m_clbkParam[clbkIndex].callback = callback;
    m_clbkParam[clbkIndex].context = context;

    /* To notify deprecated API to users */
    switch( clbkIndex )
    {
    case CALLBACK_RESULT_IMAGE:
		_PostCallback(CALLBACK_NOTIFY_MESSAGE, 0, 0, 0, IBSU_WRN_API_DEPRECATED);
		_PostTraceLogCallback(IBSU_WRN_API_DEPRECATED, "ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE was deprecated. Please use ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX instead");
        break;
    case CALLBACK_KEYBUTTON:
#ifdef _WINDOWS
        SetEvent(m_hKeyButtonThread_ReadEvent);
#else
        m_bKeyButtonThread_ReadEvent = TRUE;
#endif
        break;
	default:
		break;
    }

    return IBSU_STATUS_OK;
}

int	CMainCapture::Control_GetTouchInStatus(int *touchInStatus)
{
    WORD	status;
    int		nRc;

    if (m_UsbDeviceInfo.bEnableTouchIn == FALSE)
    {
        return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
    }

    if ((nRc = _GetTouchInStatus(&status)) != IBSU_STATUS_OK)
    {
        return nRc;
    }

    *touchInStatus = status;

    return IBSU_STATUS_OK;
}

int CMainCapture::Control_GetAvailableLEDs(IBSU_LedType *ledType, int *ledCount, DWORD *availableLEDs)
{
    // Is alive USB device
    if (!_IsAlive_UsbDevice())
    {
        return IBSU_ERR_DEVICE_IO;
    }

    *ledType = m_UsbDeviceInfo.ledType;
    *ledCount = m_UsbDeviceInfo.ledCount;
    *availableLEDs = m_UsbDeviceInfo.availableLEDs;

    return IBSU_STATUS_OK;
}

int	CMainCapture::Control_SetActiveLEDs(DWORD activeLEDs, BOOL useReservedLed)
{
    DWORD	availableLEDs;
    DWORD	nValue;
    int		nRc;

    // Is alive USB device
    if (!_IsAlive_UsbDevice())
    {
        return IBSU_ERR_DEVICE_IO;
    }

    if (m_UsbDeviceInfo.ledType == ENUM_IBSU_LED_TYPE_NONE)
    {
        return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
    }

    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
#if defined(__IBSCAN_SDK__)
        if (m_propertyInfo.bEnablePowerSaveMode == TRUE && GetIsActiveCapture() == FALSE &&
            activeLEDs > IBSCAN_LED_NONE)
        {
            // Turn on the FPGA power
            _SndUsbFwCaptureStop();
            _SndUsbFwCaptureStart();
            _FPGA_SetRegister( 0x00, 0x01 );	// set reset
            Sleep(80);							// for more safe
            _ResetFifo();
            _FPGA_SetRegister( 0x00, 0x00 );	// clear reset
        }
#elif defined(__IBSCAN_ULTIMATE_SDK__)
            // FPGA is controlling LED, so it is impossible to work it on the power save mode
            if (m_propertyInfo.bEnablePowerSaveMode == TRUE && GetIsActiveCapture() == FALSE)
            {
                return IBSU_ERR_DEVICE_ENABLED_POWER_SAVE_MODE;
            }
#endif
            break;
        default: break;
    }

    // 2011-11-11 enzyme modification for IBSCAN_LED_ALL (Because Cypress bug..)
    availableLEDs = (m_UsbDeviceInfo.availableLEDs | IBSU_LED_INIT_BLUE);
    activeLEDs &= availableLEDs;
    // User does not use reserved LEDs by vendor e.g. init_LED
    if (!useReservedLed)
    {
        activeLEDs &= (~IBSU_LED_INIT_BLUE);
    }

    nValue = (DWORD)activeLEDs;

    if ((nRc = _SetActiveLEDs(0, nValue)) != IBSU_STATUS_OK)
    {
        return nRc;
    }

    m_propertyInfo.dwActiveLEDs = nValue;

    return IBSU_STATUS_OK;
}

int	CMainCapture::Control_GetActiveLEDs(DWORD *activeLEDs, BOOL useReservedLed)
{
    DWORD	nValue;

    // Is alive USB device
    if (!_IsAlive_UsbDevice())
    {
        return IBSU_ERR_DEVICE_IO;
    }

    if (m_UsbDeviceInfo.ledType == ENUM_IBSU_LED_TYPE_NONE)
    {
        return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
    }

    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
#if defined(__IBSCAN_ULTIMATE_SDK__)
            // FPGA is controlling LED, so it is impossible to work it on the power save mode
            if (m_propertyInfo.bEnablePowerSaveMode == TRUE && GetIsActiveCapture() == FALSE)
            {
                return IBSU_ERR_DEVICE_ENABLED_POWER_SAVE_MODE;
            }
#endif
            // Cypress bug in the Watson, but it is possible to fix it with the modification of SDK code
            nValue = m_propertyInfo.dwActiveLEDs;
            break;
        case DEVICE_TYPE_CURVE:
            if (m_UsbDeviceInfo.curveModel == CURVE_MODEL_TBN240)
            {
                nValue = m_propertyInfo.dwActiveLEDs;
            }
            else
            {
                return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
            }
            break;
		case DEVICE_TYPE_CURVE_SETI:
            if (m_UsbDeviceInfo.curveModel == CURVE_MODEL_TBN340 || m_UsbDeviceInfo.curveModel == CURVE_MODEL_TBN340_V1)
            {
                nValue = m_propertyInfo.dwActiveLEDs;
            }
            else
            {
                return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
            }
            break;
        case DEVICE_TYPE_KOJAK:
            nValue = m_propertyInfo.dwActiveLEDs;
            break;
        default:
            return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
    }
    /////////////////////////////////////////////////////////////////

    // User does not use reserved LEDs by vendor e.g. init_LED
    if (!useReservedLed)
    {
        nValue &= (~IBSU_LED_INIT_BLUE);
    }

    *activeLEDs = nValue;
    return IBSU_STATUS_OK;
}

int CMainCapture::Control_GetAvailableBeeper(IBSU_BeeperType *pBeeperType)
{
    // Is alive USB device
    if (!_IsAlive_UsbDevice())
    {
        return IBSU_ERR_DEVICE_IO;
    }

    if (m_UsbDeviceInfo.beeperType == ENUM_IBSU_BEEPER_TYPE_NONE)
        return IBSU_ERR_NOT_SUPPORTED;

    *pBeeperType = m_UsbDeviceInfo.beeperType;

    return IBSU_STATUS_OK;
}

int	CMainCapture::Control_SetBeeper(const IBSU_BeepPattern beepPattern, const DWORD soundTone, const DWORD duration, const DWORD reserved_1, const DWORD reserved_2)
{
    int     stoundToneValue, repeatValue = 0;
    int		nRc = IBSU_ERR_INVALID_PARAM_VALUE;

    // Is alive USB device
    if (!_IsAlive_UsbDevice())
    {
        return IBSU_ERR_DEVICE_IO;
    }

	if (m_UsbDeviceInfo.beeperType == ENUM_IBSU_BEEPER_TYPE_NONE)
    {
        return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
    }

	if (beepPattern == ENUM_IBSU_BEEP_PATTERN_GENERIC)
    {
        if (duration < 1 || duration > 200)
        {
            return IBSU_ERR_INVALID_PARAM_VALUE;
        }
    }
    else if (beepPattern == ENUM_IBSU_BEEP_PATTERN_REPEAT)
    {
        if ( (duration < 1 || duration > 7) ||
             (reserved_1 < 1 || reserved_1 > 8) ||
             (reserved_2 < 0 || reserved_2 > 1) )
        {
            return IBSU_ERR_INVALID_PARAM_VALUE;
        }
    }
    else
    {
        return IBSU_ERR_INVALID_PARAM_VALUE;
    }

    // FPGA is controlling beeper, so it is impossible to work it on the power save mode
    if (m_propertyInfo.bEnablePowerSaveMode == TRUE && GetIsActiveCapture() == FALSE)
    {
        return IBSU_ERR_DEVICE_ENABLED_POWER_SAVE_MODE;
    }

    switch (soundTone)
    {
    case 0: stoundToneValue = 0x2C; break;     // Mi
    case 1: stoundToneValue = 0x2D; break;     // Fa
    case 2: stoundToneValue = 0x2E; break;     // Sol
    default: return IBSU_ERR_INVALID_PARAM_VALUE;
    }

	if(beepPattern == ENUM_IBSU_BEEP_PATTERN_GENERIC)
	{
		if ((nRc = _FPGA_SetRegister(0x7C, (UCHAR)stoundToneValue)) != IBSU_STATUS_OK)     // 음계
		{
			return nRc;
		}
		
		if ((nRc = _FPGA_SetRegister(0x71, (UCHAR)duration)) != IBSU_STATUS_OK)     // 음의 길이 1당 25ms ex)0x20(dec 32)는 800ms
		{
			return nRc;
		}

		if ((nRc = _FPGA_SetRegister(0x7B, 0x01)) != IBSU_STATUS_OK)     // Beep Trig
		{
			return nRc;
		}

		if ((nRc = _FPGA_SetRegister(0x7B, 0x00)) != IBSU_STATUS_OK)     // Beep Trigger 초기화
		{
			return nRc;
		}

		Sleep(duration*25);
	}
	else if(beepPattern == ENUM_IBSU_BEEP_PATTERN_REPEAT)
	{
        repeatValue = ( ((UCHAR)reserved_2 << 7) | ((UCHAR)duration << 4) | ((UCHAR)reserved_1 + (UCHAR)duration) );
		if ((nRc = _FPGA_SetRegister(0x7C, (UCHAR)stoundToneValue)) != IBSU_STATUS_OK)     // 음계
		{
			return nRc;
		}

		if ((nRc = _FPGA_SetRegister(0x7F, (UCHAR)repeatValue)) != IBSU_STATUS_OK)     // beep repeater 설정 bit7 : Enable, bit 4~6 : Ontime(1당 25ms), bit 0~3 : 주기(1당 25ms), ex) 0xB7은 175ms 주기 중 75ms Ontime & Enable
		{
			return nRc;
		}
	}

    return nRc;
}

int	CMainCapture::Capture_BGetImage(IBSU_ImageData *pImage, IBSU_ImageType *pImageType,
                                    IBSU_ImageData *pSplitImageArray, int *pSplitImageArrayCount,
                                    IBSU_FingerCountState *pFingerCountState, IBSU_FingerQualityState *pQualityArray,
                                    int *pQualityArrayCount)
{
    if (!m_blockProperty.bGetOneFrameImage)
    {
        return IBSU_WRN_BGET_IMAGE;
    }

    memset(pImage, 0, sizeof(IBSU_ImageData));
    memcpy(pImage, &m_blockProperty.imageInfo, sizeof(IBSU_ImageData));
    *pImageType = m_blockProperty.imageType;
    memset(pSplitImageArray, 0xFF, sizeof(IBSU_ImageData)*m_propertyInfo.nNumberOfObjects);
    *pSplitImageArrayCount = 0;
    if (m_blockProperty.imageInfo.IsFinal)
    {
        memcpy(pSplitImageArray, m_blockProperty.pSplitImageArray, sizeof(IBSU_ImageData)*m_blockProperty.splitImageArrayCount);
        *pSplitImageArrayCount = m_blockProperty.splitImageArrayCount;
        m_blockProperty.imageInfo.IsFinal = FALSE;
        // 20130219 enzyme add - I would like to return warning code "IBSU_WRN_BGET_IMAGE" after making result image
        m_blockProperty.bGetOneFrameImage = FALSE;
    }
    *pFingerCountState = (IBSU_FingerCountState)m_blockProperty.nFingerState;
    memcpy(pQualityArray, m_blockProperty.qualityArray, sizeof(m_blockProperty.qualityArray));
    *pQualityArrayCount = 4;

    return IBSU_STATUS_OK;
}

int	CMainCapture::Capture_BGetImageEx(int *pImageStatus, IBSU_ImageData *pImage,
								IBSU_ImageType *pImageType, int *pDetectedFingerCount,
								IBSU_ImageData *pSegmentImageArray, IBSU_SegmentPosition *pSegmentPositionArray,
								int *pSegmentImageArrayCount, IBSU_FingerCountState *pFingerCountState,
								IBSU_FingerQualityState *pQualityArray, int *pQualityArrayCount)
{
    if (!m_blockProperty.bGetOneFrameImage)
    {
        return IBSU_WRN_BGET_IMAGE;
    }

    memset(pImage, 0, sizeof(IBSU_ImageData));
    memcpy(pImage, &m_blockProperty.imageInfo, sizeof(IBSU_ImageData));
    *pImageType = m_blockProperty.imageType;
//    memset(pSegmentImageArray, 0xFF, sizeof(IBSU_ImageData)*m_propertyInfo.nNumberOfObjects);
    *pSegmentImageArrayCount = 0;
	*pImageStatus = 0;
	*pDetectedFingerCount = 0;

    if (m_blockProperty.imageInfo.IsFinal)
    {
		*pImageStatus = m_blockProperty.nResultImageStatus;
		*pDetectedFingerCount = m_blockProperty.nDetectedFingerCount;
        memcpy(pSegmentImageArray, m_blockProperty.pSegmentArray, sizeof(IBSU_ImageData)*m_blockProperty.segmentArrayCount);
        memcpy(pSegmentPositionArray, m_blockProperty.pSegmentPositionArray, sizeof(IBSU_SegmentPosition)*m_blockProperty.segmentArrayCount);
        *pSegmentImageArrayCount = m_blockProperty.segmentArrayCount;
        m_blockProperty.imageInfo.IsFinal = FALSE;
        // 20130219 enzyme add - I would like to return warning code "IBSU_WRN_BGET_IMAGE" after making result image
        m_blockProperty.bGetOneFrameImage = FALSE;
    }
    *pFingerCountState = (IBSU_FingerCountState)m_blockProperty.nFingerState;
    memcpy(pQualityArray, m_blockProperty.qualityArray, sizeof(m_blockProperty.qualityArray));
    *pQualityArrayCount = 4;

    return IBSU_STATUS_OK;
}

int CMainCapture::Main_BGetInitProgress(BOOL *pIsComplete, int *pHandle, int *pProgressValue)
{
    *pIsComplete = FALSE;
    *pHandle = -1;
    *pProgressValue = m_clbkProperty.nProgressValue;

    if (*pProgressValue == 100)
    {
        if (IsInitialized())
        {
            *pIsComplete = TRUE;
            *pHandle = GetDeviceHandle();
        }
        else
        {
            *pProgressValue = 99;
        }
    }

    return IBSU_STATUS_OK;
}

int	CMainCapture::Capture_BGetClearPlatenAtCapture(IBSU_PlatenState *pPlatenState)
{
    *pPlatenState = (IBSU_PlatenState)m_clbkProperty.nPlatenState;

    return IBSU_STATUS_OK;
}

int	CMainCapture::Capture_BGetRollingInfo(IBSU_RollingState *pRollingState, int *pRollingLineX)
{
    *pRollingState = (IBSU_RollingState)0;
    *pRollingLineX = 0;

    if (m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
    {
        *pRollingState = (IBSU_RollingState)m_pAlgo->m_rollingStatus;
		*pRollingLineX = (int)((m_pAlgo->g_LastX + 1) * m_UsbDeviceInfo.ImgWidth_Roll / m_UsbDeviceInfo.CisImgWidth_Roll);
        return IBSU_STATUS_OK;
    }

    return IBSU_WRN_ROLLING_NOT_RUNNING;
}

int	CMainCapture::Capture_BGetRollingInfoEx(IBSU_RollingState *pRollingState, int *pRollingLineX, int *pRollingDirection, int *pRollingWidth)
{
    *pRollingState = (IBSU_RollingState)0;
    *pRollingLineX = 0;
	*pRollingDirection = 0;
	*pRollingWidth = 0;

    if (m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
    {
        *pRollingState = (IBSU_RollingState)m_pAlgo->m_rollingStatus;
		*pRollingLineX = (int)((m_pAlgo->g_LastX + 1) * m_UsbDeviceInfo.ImgWidth_Roll / m_UsbDeviceInfo.CisImgWidth_Roll);
		
		if(m_pAlgo->g_Stitch_Dir == 0)
			*pRollingDirection = 0;		// 0 : can't determine
		else if(m_pAlgo->g_Stitch_Dir == 1)
			*pRollingDirection = 1;		// 1 : left to right
		else if(m_pAlgo->g_Stitch_Dir == -1)
			*pRollingDirection = 2;		// 2 : right to left

		*pRollingWidth = (int)(m_pAlgo->g_RollingWidth * m_UsbDeviceInfo.ImgWidth_Roll / (m_UsbDeviceInfo.CisImgWidth_Roll-30) * 0.0508f + 0.25f);

		if(*pRollingState == 0)
		{
			*pRollingDirection = 0;
			*pRollingWidth = 0;
		}

        return IBSU_STATUS_OK;
    }

    return IBSU_WRN_ROLLING_NOT_RUNNING;
}

int	CMainCapture::Capture_GetIBSM_ResultImageInfo(IBSM_FingerPosition fingerPosition,
        IBSM_ImageData *pResultImage,
        IBSM_ImageData *pSplitResultImage,
        int            *pSplitResultImageCount)
{
    if (!m_blockProperty.bFillResultImage)
    {
        return IBSU_WRN_EMPTY_IBSM_RESULT_IMAGE;
    }

    // Make pResultImage
    pResultImage->ImageFormat = IBSM_IMG_FORMAT_NO_BIT_PACKING;
    switch (m_blockProperty.imageType)
    {
        case ENUM_IBSU_ROLL_SINGLE_FINGER:
            pResultImage->ImpressionType = IBSM_IMPRESSION_TYPE_LIVE_SCAN_ROLLED;
            break;
        case ENUM_IBSU_FLAT_SINGLE_FINGER:
        case ENUM_IBSU_FLAT_TWO_FINGERS:
        case ENUM_IBSU_FLAT_FOUR_FINGERS:
            pResultImage->ImpressionType = IBSM_IMPRESSION_TYPE_LIVE_SCAN_PLAIN;
            break;
        default:
            pResultImage->ImpressionType = IBSM_IMPRESSION_TYPE_UNKNOWN;
            break;
    }
    pResultImage->FingerPosition = fingerPosition;
    pResultImage->CaptureDeviceTechID = IBSM_CAPTURE_DEVICE_ELECTRO_LUMINESCENT;
    pResultImage->CaptureDeviceVendorID = IBSM_CAPTURE_DEVICE_VENDOR_INTEGRATED_BIOMETRICS;
    pResultImage->CaptureDeviceTypeID = m_UsbDeviceInfo.captureDeviceTypeId;
    pResultImage->ScanSamplingX = m_UsbDeviceInfo.scanResolutionX;
    pResultImage->ScanSamplingY = m_UsbDeviceInfo.scanResolutionY;
    pResultImage->ImageSamplingX = m_UsbDeviceInfo.baseResolutionX;
    pResultImage->ImageSamplingY = m_UsbDeviceInfo.baseResolutionY;
    pResultImage->ImageSizeX = m_UsbDeviceInfo.ImgWidth;
    pResultImage->ImageSizeY = m_UsbDeviceInfo.ImgHeight;
    pResultImage->ScaleUnit = 0x01;						// Inches : 0x01
    pResultImage->BitDepth = 0x08;						// 8 bits
    pResultImage->ImageDataLength = m_UsbDeviceInfo.ImgSize;
    pResultImage->ImageData = m_blockProperty.imageInfo.Buffer;

    // Make pSplitResultImage 16-02-23 modified 
    IBSM_ImageData *img = pSplitResultImage;
 //   unsigned char *buffer = (unsigned char *)m_blockProperty.pSplitImageArray->Buffer;
    for (int i = 0; i < m_blockProperty.splitImageArrayCount; i++)
    {
        memcpy(img, pResultImage, sizeof(IBSM_ImageData));
        img->ImageData = (void *)m_blockProperty.pSplitImageArray[i].Buffer;//buffer;
        img++;
//        buffer += sizeof(IBSU_ImageData);
    }

    // Make pSplitResultImageCount
    *pSplitResultImageCount = m_blockProperty.splitImageArrayCount;

    m_blockProperty.bFillResultImage = FALSE;

    return IBSU_STATUS_OK;
}

int	CMainCapture::Capture_GetNFIQScore(const BYTE *imgBuffer, const DWORD width, const DWORD height, const BYTE bitsPerPixel, int *pScore)
{
    int nfiq_score = 0;

    if (m_pAlgo->_Algo_GetNFIQScore(imgBuffer, width, height,
                                    bitsPerPixel, &nfiq_score) != 0)	// 0 : success
    {
        return IBSU_ERR_NBIS_NFIQ_FAILED;
    }

    *pScore = nfiq_score;

    return IBSU_STATUS_OK;
}

int	CMainCapture::Capture_CheckWetFinger(const IBSU_ImageData inImage)
{
	int WetRatio = 0;
//	int wet_threshold = 100;
	
	if(inImage.Width <= 0 || inImage.Height <= 0)
		return IBSU_ERR_INVALID_PARAM_VALUE;

	if(m_propertyInfo.bWetFingerDetect == FALSE)
		return IBSU_ERR_NOT_SUPPORTED;

	unsigned char *TmpBuffer = new unsigned char[inImage.Width*inImage.Height];

	for(int i=0; i<(int)(inImage.Width*inImage.Height); i++)
	{
		TmpBuffer[i] = 255 - ((unsigned char*)inImage.Buffer)[i];
	}

	int nRc = m_pAlgo->_Algo_AnalysisBlockStd_forWet(TmpBuffer, inImage.Width, inImage.Height, 0, &WetRatio);

	delete [] TmpBuffer;

	return nRc;
}

int	CMainCapture::Capture_GetImageWidth(const BYTE *inImage, const int width, const int height, int *outMM)
{
	int nRc = IBSU_STATUS_OK;

	*outMM = 0;
	
	if(width <= 0 || height <= 0)
		return IBSU_ERR_INVALID_PARAM_VALUE;

	int i, j;
	int left=width, right=0;
	int thres = 10;
	int *mean_h = new int [width];
	memset(mean_h, 0, width*sizeof(int));

	thres = 10;//_NO_BIT_CMOS_NOISE_FOR_FULLFRAME_ROLL-5;//m_SBDAlg->m_Roll_Minus_Val;

	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			mean_h[j] += (255-inImage[i*width+j]);
		}
	}
	
	for(j=0; j<width; j++)
		mean_h[j] /= height;

	for(j=0; j<width; j++)
	{
		if(mean_h[j] > thres)
		{
			left=j;
			break;
		}
	}

	for(j=width-1; j>=0; j--)
	{
		if(mean_h[j] > thres)
		{
			right=j;
			break;
		}
	}

	delete [] mean_h;

	if(left >= right)
		return IBSU_ERR_COMMAND_FAILED;

	*outMM = (int)((right-left) * 50.8 / 1000);

	return nRc;
}

#if defined(__IBSCAN_SDK__)
int CMainCapture::RawCapture_Abort()
{
    return Capture_Abort();
}

int CMainCapture::RawCapture_GetOneFrameImage()
{
    if( !m_bCaptureThread_StopMessage )
    {
#ifdef _WINDOWS
        SetEvent(m_hRawCaptureThread_GetOneFrameImageEvent);
#else
        m_bRawCaptureThread_GetOneFrameImageEvent = TRUE;
#endif
    }

    return IBSU_STATUS_OK;
}

int CMainCapture::RawCapture_TakePreviewImage(const RawImageData rawImage)
{
    memcpy(m_pAlgo->m_GetImgFromApp, (BYTE *)rawImage.Buffer, rawImage.Width*rawImage.Height);

    if( !m_bCaptureThread_StopMessage )
    {
#ifdef _WINDOWS
        SetEvent(m_hRawCaptureThread_TakePreviewImageEvent);
#else
        m_bRawCaptureThread_TakePreviewImageEvent = TRUE;
#endif
    }

    return IBSU_STATUS_OK;
}

int CMainCapture::RawCapture_TakeResultImage(const RawImageData rawImage)
{
    memcpy(m_pAlgo->m_GetImgFromApp, rawImage.Buffer, rawImage.Width*rawImage.Height);

    if( !m_bCaptureThread_StopMessage )
    {
#ifdef _WINDOWS
        SetEvent(m_hRawCaptureThread_TakeResultImageEvent);
#else
        m_bRawCaptureThread_TakeResultImageEvent = TRUE;
#endif
    }

    return IBSU_STATUS_OK;
}

int	CMainCapture::Capture_SetDACRegister(int dacValue)
{
    int		nRc;

    if (!IsCaptureSetMode())
    {
        return IBSCAN_ERR_CHANNEL_INVALID_CAPTURE_MODE;
    }

    if (dacValue < 0 || dacValue > 255)
    {
        return IBSCAN_ERR_DEVICE_WRITE_PARAM;
    }

    if( m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON || 
        m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON_MINI ||
        m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE ||
        m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE_SETI)
    {
        return IBSCAN_ERR_NO_HARDWARE_SUPPORT;
    }

    dacValue = 255 - dacValue;
    if ((nRc = _DAC_SetRegister(dacValue)) != IBSU_STATUS_OK)
    {
        return nRc;
    }

    return IBSU_STATUS_OK;
}
#endif
//////////////////////////////////////////////////////////////////////////////////




void CMainCapture::_InitializeCaptureVariables()
{
    memset(&m_pAlgo->m_cImgAnalysis, 0, sizeof(m_pAlgo->m_cImgAnalysis));
	m_pAlgo->m_CaptureGood = FALSE;
    m_nGammaLevel = 12;

	m_pAlgo->m_cImgAnalysis.is_final = FALSE;
	m_pAlgo->m_cImgAnalysis.detected_frame_count = 0;
	m_pAlgo->m_cImgAnalysis.percent_of_255 = 0;
	m_pAlgo->m_cImgAnalysis.JudgeMeanMode = 0;
	m_pAlgo->m_cImgAnalysis.JudgeMeanModeCount = 0;

    m_bCaptureThread_StopMessage = FALSE;
    m_bCaptureThread_DummyStopMessage = FALSE;

    m_pDlgUsbManager->m_bIsReadySend[CALLBACK_PREVIEW_IMAGE] = TRUE;
    m_pDlgUsbManager->m_bIsReadySend[CALLBACK_FINGER_COUNT] = TRUE;
    m_pDlgUsbManager->m_bIsReadySend[CALLBACK_FINGER_QUALITY] = TRUE;
    m_pDlgUsbManager->m_bIsReadySend[CALLBACK_TAKING_ACQUISITION] = TRUE;
    m_pDlgUsbManager->m_bIsReadySend[CALLBACK_COMPLETE_ACQUISITION] = TRUE;
    m_pDlgUsbManager->m_bIsReadySend[CALLBACK_RESULT_IMAGE] = TRUE;
    m_pDlgUsbManager->m_bIsReadySend[CALLBACK_CLEAR_PLATEN] = TRUE;
    m_pDlgUsbManager->m_bIsReadySend[CALLBACK_NOTIFY_MESSAGE] = TRUE;
	m_pDlgUsbManager->m_bIsReadySend[CALLBACK_RESULT_IMAGE_EX] = TRUE;
	m_pDlgUsbManager->m_bIsReadySend[CALLBACK_KEYBUTTON] = TRUE;

    m_pDlgUsbManager->m_bIsReadySend[CALLBACK_ONE_FRAME_IMAGE] = TRUE;
    m_pDlgUsbManager->m_bIsReadySend[CALLBACK_TAKE_PREVIEW_IMAGE] = TRUE;
    m_pDlgUsbManager->m_bIsReadySend[CALLBACK_TAKE_RESULT_IMAGE] = TRUE;

    //	m_nFrameCount = 0;
    // 2013-05-03 enzyme modify - Some of devices do not need to clear double buffering image
    // Because of they do not have any buffer
    m_nFrameCount = m_UsbDeviceInfo.nDoubleBufferedFrameCount;

    m_clbkProperty.nFingerState = -1;
}

BOOL CMainCapture::_SetInitializeDeviceDesc(PropertyInfo *pPropertyInfo, UsbDeviceInfo *pScanDevDesc)
{
    char *productID = pPropertyInfo->cProductID;
	UCHAR nChipId;

    _SetPID_VID(&m_UsbDeviceInfo, productID);

    // Default value for Usb device
    pScanDevDesc->CisImgWidth = 1088;
    pScanDevDesc->CisImgHeight = 1022;
    pScanDevDesc->CisImgWidth_Roll = pScanDevDesc->CisImgWidth;
    pScanDevDesc->CisImgHeight_Roll = pScanDevDesc->CisImgHeight;
    pScanDevDesc->ImgWidth = 800;
    pScanDevDesc->ImgHeight = 750;
    pScanDevDesc->ImgWidth_Roll = pScanDevDesc->ImgWidth;
    pScanDevDesc->ImgHeight_Roll = pScanDevDesc->ImgHeight;
    pScanDevDesc->ledType = ENUM_IBSU_LED_TYPE_NONE;
    pScanDevDesc->ledCount = 0;
    pScanDevDesc->availableLEDs = 0;
    pScanDevDesc->baseResolutionX = 500;
    pScanDevDesc->baseResolutionY = 500;
    pScanDevDesc->pollingTimeForGetImg = 50;
    pScanDevDesc->CisRowStart = 86;			// Watson's default value is 1022
    pScanDevDesc->CisColStart = 208;		// Watson's default value is 1088
    pScanDevDesc->bEnableTouchIn = FALSE;
    pScanDevDesc->bNeedMask = TRUE;
    pScanDevDesc->bNeedEmbeddedMask = TRUE;
    pScanDevDesc->bNeedDistortionMask = TRUE;
    pScanDevDesc->bEnableOperationLE = TRUE;
    pScanDevDesc->nWaterImageSize = pScanDevDesc->CisImgWidth * pScanDevDesc->CisImgHeight;
    pScanDevDesc->bEnableELVoltage = TRUE;
    pScanDevDesc->nMIN_Elapsed = 50;
    pScanDevDesc->scanResolutionX = 500;
    pScanDevDesc->scanResolutionY = 500;
    pScanDevDesc->captureDeviceTypeId = IBSM_CAPTURE_DEVICE_TYPE_ID_UNKNOWN;
    pScanDevDesc->nDefaultContrastValue = __DEFAULT_CONTRAST_VALUE__;
    pScanDevDesc->bNewFPCB = FALSE;
    pScanDevDesc->nDoubleBufferedFrameCount = -2;
    pScanDevDesc->nZoomOut = 5;
    pScanDevDesc->nFpgaVersion = -1;
    pScanDevDesc->bDecimation = FALSE;
    pScanDevDesc->nDecimation_Mode = DECIMATION_NONE;
	pPropertyInfo->bEnableDecimation = FALSE;
	pScanDevDesc->beeperType = ENUM_IBSU_BEEPER_TYPE_NONE;
    pScanDevDesc->bSmearDetection = TRUE;
    pScanDevDesc->nASICVersion = -1;
	pScanDevDesc->bArrangeMode = 0;
	pPropertyInfo->nCaptureBrightThresFlat[0] = __CAPTURE_MIN_BRIGHT__;
	pPropertyInfo->nCaptureBrightThresFlat[1] = __CAPTURE_MAX_BRIGHT__;
	pPropertyInfo->nCaptureBrightThresRoll[0] = __CAPTURE_MIN_BRIGHT__;
	pPropertyInfo->nCaptureBrightThresRoll[1] = __CAPTURE_MAX_BRIGHT__;
	pPropertyInfo->bColumboBaordforPI = FALSE;

    switch (pScanDevDesc->pid)
    {
        case __PID_WATSON__:
            if ((strcmp(productID, "IBNW11C")			== 0) ||
                    (strcmp(productID, "IBSCAN310LS-W")		== 0) ||
                    (strcmp(productID, "WATSON")				== 0))
            {
                pScanDevDesc->devType = DEVICE_TYPE_WATSON;
                sprintf(pScanDevDesc->productName, "WATSON");
                pScanDevDesc->ledType = ENUM_IBSU_LED_TYPE_TSCAN;
                pScanDevDesc->ledCount = 1;
                pScanDevDesc->availableLEDs = IBSU_LED_SCAN_GREEN;
                pScanDevDesc->bEnableTouchIn = TRUE;
                pScanDevDesc->nWaterImageSize = pScanDevDesc->CisImgWidth * pScanDevDesc->CisImgHeight;
                pScanDevDesc->scanResolutionX = 690;
                pScanDevDesc->scanResolutionY = 690;
                pScanDevDesc->captureDeviceTypeId = IBSM_CAPTURE_DEVICE_TYPE_ID_WATSON;
            }
            // Because we made a sherlock one for just testing on Watson pid
            else if ((strcmp(productID, "SHERLOCK-ONE")	== 0) ||
                     (strcmp(productID, "SHERLOCK")		== 0))
            {
                pScanDevDesc->devType = DEVICE_TYPE_SHERLOCK;
                sprintf(pScanDevDesc->productName, "SHERLOCK");
                pScanDevDesc->CisImgWidth = 800;
                pScanDevDesc->CisImgHeight = 750;
                pScanDevDesc->CisImgWidth_Roll = pScanDevDesc->CisImgWidth;
                pScanDevDesc->CisImgHeight_Roll = pScanDevDesc->CisImgHeight;
//            pScanDevDesc->bNeedMask = FALSE;
                pScanDevDesc->bNeedEmbeddedMask = FALSE;
                pScanDevDesc->bNeedDistortionMask = FALSE;
                pScanDevDesc->bEnableOperationLE = FALSE;
                pScanDevDesc->nWaterImageSize = (int)((pScanDevDesc->CisImgWidth * pScanDevDesc->CisImgHeight + 510) / 511 * 511);
                pScanDevDesc->captureDeviceTypeId = IBSM_CAPTURE_DEVICE_TYPE_ID_SHERLOCK;
            }
            else
            {
                return FALSE;			// Not supported yet!
            }
            break;
        case __PID_WATSON_REV1__:
            // Fixed uniformity issue with adding flash memory on FPGA
            pScanDevDesc->devType = DEVICE_TYPE_WATSON;
            sprintf(pScanDevDesc->productName, "WATSON");
            pScanDevDesc->ledType = ENUM_IBSU_LED_TYPE_TSCAN;
            pScanDevDesc->ledCount = 1;
            pScanDevDesc->availableLEDs = IBSU_LED_SCAN_GREEN;
            pScanDevDesc->bEnableTouchIn = TRUE;
            pScanDevDesc->nWaterImageSize = pScanDevDesc->CisImgWidth * pScanDevDesc->CisImgHeight;
            pScanDevDesc->scanResolutionX = 690;
            pScanDevDesc->scanResolutionY = 690;
            pScanDevDesc->captureDeviceTypeId = IBSM_CAPTURE_DEVICE_TYPE_ID_WATSON;
            break;
        case __PID_SHERLOCK__:
        case __PID_SHERLOCK_REV1__:
            pScanDevDesc->CisImgWidth = 800;
            pScanDevDesc->CisImgHeight = 750;
            pScanDevDesc->CisImgWidth_Roll = pScanDevDesc->CisImgWidth;
            pScanDevDesc->CisImgHeight_Roll = pScanDevDesc->CisImgHeight;
            pScanDevDesc->bNeedEmbeddedMask = FALSE;
            pScanDevDesc->bNeedDistortionMask = FALSE;
            pScanDevDesc->bEnableOperationLE = FALSE;
            pScanDevDesc->nWaterImageSize = (int)((pScanDevDesc->CisImgWidth * pScanDevDesc->CisImgHeight + 510) / 511 * 511);
            // 2013-03-12 enzyme modify - To solve flicker issue
            //		pScanDevDesc->nMIN_Elapsed = 80;
            pScanDevDesc->captureDeviceTypeId = IBSM_CAPTURE_DEVICE_TYPE_ID_SHERLOCK;
            if ((strcmp(productID, "SHERLOCK")	== 0))
            {
                pScanDevDesc->devType = DEVICE_TYPE_SHERLOCK;
                sprintf(pScanDevDesc->productName, "SHERLOCK");
            }
            else if ((strcmp(productID, "SHERLOCK_ROIC")	== 0))
            {
                pScanDevDesc->devType = DEVICE_TYPE_SHERLOCK_ROIC;
                sprintf(pScanDevDesc->productName, "SHERLOCK_ROIC");
            }
            pScanDevDesc->nDefaultContrastValue = __ROIC_VOLTAGE_DEFAULT_VALUE__;
            break;
#if defined(__IBSCAN_ULTIMATE_SDK__)
        case __PID_WATSON_MINI__:
        case __PID_WATSON_MINI_REV1__:
            pScanDevDesc->devType = DEVICE_TYPE_WATSON_MINI;
            sprintf(pScanDevDesc->productName, "WATSON MINI");
            pScanDevDesc->nWaterImageSize = pScanDevDesc->CisImgWidth * pScanDevDesc->CisImgHeight;
            pScanDevDesc->scanResolutionX = 690;
            pScanDevDesc->scanResolutionY = 690;
            pScanDevDesc->captureDeviceTypeId = IBSM_CAPTURE_DEVICE_TYPE_ID_WATSON_MINI;
            break;
        case __PID_COLUMBO__:
        case __PID_COLUMBO_REV1__:
            pScanDevDesc->devType = DEVICE_TYPE_COLUMBO;
            sprintf(pScanDevDesc->productName, "COLUMBO");
            pScanDevDesc->CisImgWidth = 880;
            pScanDevDesc->CisImgHeight = 1100;
            pScanDevDesc->ImgWidth = 400;
            pScanDevDesc->ImgHeight = 500;
            pScanDevDesc->CisImgWidth_Roll = pScanDevDesc->CisImgWidth;
            pScanDevDesc->CisImgHeight_Roll = pScanDevDesc->CisImgHeight;
            pScanDevDesc->ImgWidth_Roll = pScanDevDesc->ImgWidth;
            pScanDevDesc->ImgHeight_Roll = pScanDevDesc->ImgHeight;
            pScanDevDesc->CisRowStart = 0;
            pScanDevDesc->CisColStart = 0;
//        pScanDevDesc->bNeedMask = FALSE;
            //		pScanDevDesc->nWaterImageSize = pScanDevDesc->CisImgWidth*pScanDevDesc->CisImgHeight;
            // Note..
            // CIS_IMG_SIZE = 1100 * 880 = 968,000
            // Possible write sector of flash = 512byte, but 1byte is checksum, so 968,000/511 = 1,895
            // Totall sector of flash = 528byte, so 1,895 * 528 = 1,000,560
            // Each packet size of EP6In = 512byte, so totall transfer is 1,000,560 / 512 = 1,955
            // Finally the water image size is 1,955 *512 = 1,000,960
            pScanDevDesc->nWaterImageSize = 1000960;
            pScanDevDesc->nMIN_Elapsed = 60;
            pScanDevDesc->scanResolutionX = 966;
            pScanDevDesc->scanResolutionY = 966;
            pScanDevDesc->captureDeviceTypeId = IBSM_CAPTURE_DEVICE_TYPE_ID_COLUMBO;
            pScanDevDesc->nDoubleBufferedFrameCount = 0;
            pScanDevDesc->nDefaultContrastValue = __DEFAULT_COLUMBO_CONTRAST_VALUE__;
		 	pPropertyInfo->bEnableDecimation = TRUE;
			
			if(pScanDevDesc->pid == __PID_COLUMBO_REV1__)
				pPropertyInfo->bColumboBaordforPI = TRUE;
#if defined(__ppi__)	
    if (m_bPPIMode)
    {
	    m_propertyInfo.bEnableDecimation = FALSE;
    }
#endif
 			pPropertyInfo->nRollMode = ROLL_MODE_NO_USE;
            pScanDevDesc->bSmearDetection = FALSE;
            break;
        case __PID_CURVE__:
            pScanDevDesc->devType = DEVICE_TYPE_CURVE;
            sprintf(pScanDevDesc->productName, "CURVE");
			pScanDevDesc->nDefaultContrastValue = 34;

			_CurveSETi_GetRegister(0x01, &nChipId);
			
			if(nChipId == 0x12)
			{
				pScanDevDesc->devType = DEVICE_TYPE_CURVE_SETI;
				pScanDevDesc->CisImgWidth = 316;//360;
				pScanDevDesc->CisImgHeight = 386;//432;
    	        pScanDevDesc->CisRowStart = 10;
	            pScanDevDesc->CisColStart = 30;
			}
			else
			{
				pScanDevDesc->CisImgWidth = 360;
				pScanDevDesc->CisImgHeight = 440;
	            pScanDevDesc->CisRowStart = 60;
	            pScanDevDesc->CisColStart = 28;
			}
			pScanDevDesc->ImgWidth = 288;
            pScanDevDesc->ImgHeight = 352;
            pScanDevDesc->CisImgWidth_Roll = pScanDevDesc->CisImgWidth;
            pScanDevDesc->CisImgHeight_Roll = pScanDevDesc->CisImgHeight;
            pScanDevDesc->ImgWidth_Roll = pScanDevDesc->ImgWidth;
            pScanDevDesc->ImgHeight_Roll = pScanDevDesc->ImgHeight;
//        pScanDevDesc->bNeedMask = FALSE;
            pScanDevDesc->nWaterImageSize = 0;
            pScanDevDesc->bEnableELVoltage = FALSE;
            pScanDevDesc->scanResolutionX = 620;
            pScanDevDesc->scanResolutionY = 620;
            pScanDevDesc->captureDeviceTypeId = IBSM_CAPTURE_DEVICE_TYPE_ID_CURVE;
            //	    pScanDevDesc->nMIN_Elapsed = 80;
            pScanDevDesc->curveModel = _GetCurveModel(pPropertyInfo, pScanDevDesc);
            pScanDevDesc->bEnableOperationLE = FALSE;
            if (pScanDevDesc->curveModel == CURVE_MODEL_TBN240 || pScanDevDesc->curveModel == CURVE_MODEL_TBN340 || pScanDevDesc->curveModel == CURVE_MODEL_TBN340_V1)
            {
                pScanDevDesc->ledType = ENUM_IBSU_LED_TYPE_TSCAN;
                pScanDevDesc->ledCount = 3;
                pScanDevDesc->availableLEDs = IBSU_LED_SCAN_CURVE_RED | IBSU_LED_SCAN_CURVE_GREEN | IBSU_LED_SCAN_CURVE_BLUE;
                pScanDevDesc->bEnableOperationLE = TRUE;
				pPropertyInfo->dwActiveLEDs = IBSU_LED_SCAN_CURVE_BLUE;			// This is default LED on TBN240
            }
            pScanDevDesc->nDoubleBufferedFrameCount = 0;
			pPropertyInfo->nRollMode = ROLL_MODE_NO_USE;
            pScanDevDesc->bSmearDetection = FALSE;
            break;
        case __PID_HOLMES__:
            pScanDevDesc->devType = DEVICE_TYPE_HOLMES;
            sprintf(pScanDevDesc->productName, "HOLMES");
            //		pScanDevDesc->CisImgWidth = 800;
            //		pScanDevDesc->CisImgHeight = 750;
            //		pScanDevDesc->ImgWidth = 800;
            //		pScanDevDesc->ImgHeight = 750;
            //		pScanDevDesc->CisRowStart = 0;
            //		pScanDevDesc->CisColStart = 0;
            //		pScanDevDesc->bNeedMask = FALSE;
//            pScanDevDesc->CisImgWidth_Roll = pScanDevDesc->CisImgWidth;
//            pScanDevDesc->CisImgHeight_Roll = pScanDevDesc->CisImgHeight;
//            pScanDevDesc->ImgWidth_Roll = pScanDevDesc->ImgWidth;
//            pScanDevDesc->ImgHeight_Roll = pScanDevDesc->ImgHeight;
            pScanDevDesc->nWaterImageSize = pScanDevDesc->CisImgWidth * pScanDevDesc->CisImgHeight;
            pScanDevDesc->nMIN_Elapsed = 60;
            pScanDevDesc->captureDeviceTypeId = IBSM_CAPTURE_DEVICE_TYPE_ID_HOLMES;
            pScanDevDesc->nZoomOut = 2;
            break;
		case __PID_KOJAK__:
		case __PID_KOJAK_REV1__:
            pScanDevDesc->devType = DEVICE_TYPE_KOJAK;
            sprintf(pScanDevDesc->productName, "KOJAK");
            pScanDevDesc->ledType = (IBSU_LedType)ENUM_IBSU_LED_TYPE_FSCAN;
            pScanDevDesc->ledCount = 14;
            pScanDevDesc->availableLEDs = 
				IBSU_LED_F_LEFT_LITTLE_GREEN |
				IBSU_LED_F_LEFT_LITTLE_RED |
				IBSU_LED_F_LEFT_RING_GREEN |
				IBSU_LED_F_LEFT_RING_RED |
				IBSU_LED_F_LEFT_MIDDLE_GREEN |
				IBSU_LED_F_LEFT_MIDDLE_RED |
				IBSU_LED_F_LEFT_INDEX_GREEN |
				IBSU_LED_F_LEFT_INDEX_RED |
				IBSU_LED_F_LEFT_THUMB_GREEN |
				IBSU_LED_F_LEFT_THUMB_RED |
				IBSU_LED_F_RIGHT_THUMB_GREEN |
				IBSU_LED_F_RIGHT_THUMB_RED |
				IBSU_LED_F_RIGHT_INDEX_GREEN |
				IBSU_LED_F_RIGHT_INDEX_RED |
				IBSU_LED_F_RIGHT_MIDDLE_GREEN |
				IBSU_LED_F_RIGHT_MIDDLE_RED |
				IBSU_LED_F_RIGHT_RING_GREEN |
				IBSU_LED_F_RIGHT_RING_RED |
				IBSU_LED_F_RIGHT_LITTLE_GREEN |
				IBSU_LED_F_RIGHT_LITTLE_RED |
				IBSU_LED_F_BLINK_GREEN |
				IBSU_LED_F_BLINK_RED |
				IBSU_LED_F_PROGRESS_ROLL |
				IBSU_LED_F_PROGRESS_LEFT_HAND |
				IBSU_LED_F_PROGRESS_TWO_THUMB |
				IBSU_LED_F_PROGRESS_RIGHT_HAND;
			
			pScanDevDesc->beeperType = ENUM_IBSU_BEEPER_TYPE_MONOTONE;

			pScanDevDesc->CisImgWidth = 2056;//2052;//2054;
            pScanDevDesc->CisImgHeight = 1928;//1930;
            pScanDevDesc->ImgWidth = 1600;
            pScanDevDesc->ImgHeight = 1500;
			pScanDevDesc->CisImgWidth_Roll = 1088;
            pScanDevDesc->CisImgHeight_Roll = 1022;
            pScanDevDesc->ImgWidth_Roll = 800;////1008;	//800
            pScanDevDesc->ImgHeight_Roll = 750;////944;	//750
            pScanDevDesc->CisRowStart = 0;
            pScanDevDesc->CisColStart = 0;
//        pScanDevDesc->bNeedMask = FALSE;
//            pScanDevDesc->nWaterImageSize = pScanDevDesc->CisImgWidth * pScanDevDesc->CisImgHeight;
            // Note..
            // CIS_IMG_SIZE = 2052 * 1928 = 3,956,256
            // Possible write sector of flash = 512byte, but 1byte is checksum, so 3,956,256/511 = 7,743
            // Totall sector of flash = 528byte, so 7,743 * 528 = 4088304
            // Each packet size of EP6In = 512byte, so totall transfer is 4,088,304 / 512 = 7,985
            // Finally the water image size is 1,955 *512 = 4,088,320
            pScanDevDesc->nWaterImageSize = 4096512;
            pScanDevDesc->nMIN_Elapsed = 60;
            pScanDevDesc->captureDeviceTypeId = IBSM_CAPTURE_DEVICE_TYPE_ID_KOJAK;
            pScanDevDesc->nZoomOut = 5;
			pPropertyInfo->bEnableDecimation = TRUE;
			pPropertyInfo->nSuperDryMode = 0;
            pScanDevDesc->nDoubleBufferedFrameCount = 0;
            pScanDevDesc->bSmearDetection = TRUE;//FALSE;          // Not supported yet!!!
//			pPropertyInfo->nCaptureBrightThresFlat[0] = _TARGET_MIN_BRIGHTNESS_FOR_CAPTURE2_;
//			pPropertyInfo->nCaptureBrightThresFlat[1] = _TARGET_MAX_BRIGHTNESS_FOR_CAPTURE2_;
			pPropertyInfo->nCaptureBrightThresFlat[0] = _TARGET_MIN_BRIGHTNESS_FOR_CAPTURE2_+5;
			pPropertyInfo->nCaptureBrightThresFlat[1] = _TARGET_MAX_BRIGHTNESS_FOR_CAPTURE2_-10;
			pPropertyInfo->nCaptureBrightThresRoll[0] = _TARGET_MIN_BRIGHTNESS_FOR_CAPTURE2_ROLL;
			pPropertyInfo->nCaptureBrightThresRoll[1] = _TARGET_MAX_BRIGHTNESS_FOR_CAPTURE2_ROLL;
            break;
		case __PID_FIVE0__:
		case __PID_FIVE0_REV1__:
		case __PID_FIVE0_DERMALOG__:
            pScanDevDesc->devType = DEVICE_TYPE_FIVE0;
            sprintf(pScanDevDesc->productName, "FIVE-0");
            pScanDevDesc->CisImgWidth = 1600;       // 1600
            pScanDevDesc->CisImgHeight = 1000;      // 1000
            pScanDevDesc->ImgWidth = 1600;          // 1600
            pScanDevDesc->ImgHeight = 1000;         // 1000
			pScanDevDesc->CisImgWidth_Roll = 800;
            pScanDevDesc->CisImgHeight_Roll = 750;
            pScanDevDesc->ImgWidth_Roll = 800;
            pScanDevDesc->ImgHeight_Roll = 750;
//        pScanDevDesc->bNeedMask = FALSE;
            pScanDevDesc->bNeedEmbeddedMask = FALSE;
            pScanDevDesc->bNeedDistortionMask = FALSE;
            pScanDevDesc->bEnableOperationLE = FALSE;
            pScanDevDesc->nWaterImageSize = (int)((pScanDevDesc->CisImgWidth * pScanDevDesc->CisImgHeight + 
													pScanDevDesc->CisImgWidth / 8 * pScanDevDesc->CisImgHeight + 510) / 511 * 528);
            pScanDevDesc->captureDeviceTypeId = IBSM_CAPTURE_DEVICE_TYPE_ID_FIVE0;
			pPropertyInfo->nSuperDryMode = 0;
			pScanDevDesc->nMIN_Elapsed = 50;
			pPropertyInfo->nCaptureBrightThresFlat[0] = _TARGET_MIN_BRIGHTNESS_FOR_CAPTURE2_FIVE0;
			pPropertyInfo->nCaptureBrightThresFlat[1] = _TARGET_MAX_BRIGHTNESS_FOR_CAPTURE2_FIVE0;
			pPropertyInfo->nCaptureBrightThresRoll[0] = _TARGET_MIN_BRIGHTNESS_FOR_CAPTURE2_ROLL_FIVE0;
			pPropertyInfo->nCaptureBrightThresRoll[1] = _TARGET_MAX_BRIGHTNESS_FOR_CAPTURE2_ROLL_FIVE0;
            break;       
#endif
		default:
            return FALSE;
    }

    pScanDevDesc->CisImgSize = pScanDevDesc->CisImgWidth * pScanDevDesc->CisImgHeight;
    pScanDevDesc->ImgSize = pScanDevDesc->ImgWidth * pScanDevDesc->ImgHeight;
    pScanDevDesc->CisImgSize_Roll = pScanDevDesc->CisImgWidth_Roll * pScanDevDesc->CisImgHeight_Roll;
    pScanDevDesc->ImgSize_Roll = pScanDevDesc->ImgWidth_Roll * pScanDevDesc->ImgHeight_Roll;

	if (pPropertyInfo->bEnableCaptureOnRelease)
	{
		pPropertyInfo->bSavedEnableDecimation = pPropertyInfo->bEnableDecimation;
		pPropertyInfo->bEnableDecimation = FALSE;
	}

    return TRUE;
}

BOOL CMainCapture::_GetOneFrameImage(unsigned char *Image, int ImgSize)
{
    int bResult = FALSE;

    if (Image == NULL)
    {
        return FALSE;
    }

    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
        case DEVICE_TYPE_WATSON_MINI:
            bResult = _GetOneFrameImage_Watson(Image, ImgSize);
            break;
        case DEVICE_TYPE_SHERLOCK_ROIC:
        case DEVICE_TYPE_SHERLOCK:
            bResult = _GetOneFrameImage_Sherlock(Image, ImgSize);
            break;
        case DEVICE_TYPE_COLUMBO:
            m_bRunningEP6 = TRUE;
            bResult = _GetOneFrameImage_Columbo(Image, ImgSize);
            break;
        case DEVICE_TYPE_CURVE:
            m_bRunningEP6 = TRUE;
			bResult = _GetOneFrameImage_Curve(Image, ImgSize);
            break;
        case DEVICE_TYPE_CURVE_SETI:
            m_bRunningEP6 = TRUE;
           	bResult = _GetOneFrameImage_CurveSETi(Image, ImgSize);
            break;
        case DEVICE_TYPE_HOLMES:
            bResult = _GetOneFrameImage_Holmes(Image, ImgSize);
            break;
		case DEVICE_TYPE_KOJAK:
			if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
			{
				bResult = _GetOneFrameImage_Kojak(Image, m_UsbDeviceInfo.CisImgSize_Roll);
			}
			else if( m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
					 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
					 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
				     m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
				bResult = _GetOneFrameImage_Kojak(Image, ImgSize);
			else
				bResult = FALSE;
            break;
        case DEVICE_TYPE_FIVE0:
            bResult = _GetOneFrameImage_Five0(Image, ImgSize);
            break;
        default:
            break;
    }
    m_bRunningEP6 = FALSE;

    return bResult;
}

int CMainCapture::_ResetFifo()
{
    if (_GetPID() == __PID_CURVE__)
    {
        UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};

        outBuffer[0] = NA2CTL_RESET_FIFO;

        return _UsbBulkOutIn(EP1OUT, 0x00, outBuffer, 1, -1, NULL, 0, NULL);
    }

    return _UsbBulkOutIn(EP1OUT, CMD_RESET_FIFO, NULL, 0, -1, NULL, 0, NULL);
}

int CMainCapture::_FPGA_SetRegister(UCHAR adr, UCHAR val)
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};

    outBuffer[0] = (UCHAR)adr;
    outBuffer[1] = (UCHAR)1;
    outBuffer[2] = (UCHAR)val;

    return _UsbBulkOutIn(EP1OUT, CMD_WRITE_FPGA_REG, outBuffer, 3, -1, NULL, 0, NULL);
}

int CMainCapture::_FPGA_GetRegister(UCHAR adr, UCHAR *val, int timeout)
{
    UCHAR		bData[MAX_DATA_BYTES + 1];
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};
    int			nRc;

    outBuffer[0] = (UCHAR)adr;
    outBuffer[1] = (UCHAR)1;

    nRc = _UsbBulkOutIn(EP1OUT, CMD_READ_FPGA_REG, outBuffer, 2, EP1IN, &bData[0], 1, NULL, timeout);
    if (nRc == IBSU_STATUS_OK)
    {
        *val = bData[0];
    }
    else
    {
        *val = 0;
    }

    return nRc;
}

int CMainCapture::_MT9M_SetRegister(WORD adr, WORD val)
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};

    outBuffer[0] = (UCHAR)(adr);
    outBuffer[1] = (UCHAR)(adr >> 8);
    outBuffer[2] = (UCHAR)2;
    outBuffer[3] = (UCHAR)(val >> 8);
    outBuffer[4] = (UCHAR)(val);

    return _UsbBulkOutIn(EP1OUT, CMD_WRITE_CIS_REG, outBuffer, 5, -1, NULL, 0, NULL);
}

int CMainCapture::_MT9M_GetRegister(WORD adr, WORD *val)
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};
    UCHAR		inBuffer[MAX_DATA_BYTES + 1] = {0};
    int			nRc;

    outBuffer[0] = (UCHAR)(adr);
    outBuffer[1] = (UCHAR)(adr >> 8);
    outBuffer[2] = (UCHAR)2;

    nRc = _UsbBulkOutIn(EP1OUT, CMD_READ_CIS_REG, outBuffer, 3,
                        EP1IN, (PUCHAR)inBuffer, 2, NULL);
    if (nRc == IBSU_STATUS_OK)
    {
        *val = (((WORD)inBuffer[0]) << 8) + (WORD)inBuffer[1];
    }
    else
    {
        *val = 0;
    }

    return nRc;
}

int CMainCapture::_InitializeForCISRegister(WORD rowStart, WORD colStart, WORD width, WORD height, BOOL bEEPROM_Read)
{
    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
        case DEVICE_TYPE_WATSON_MINI:
            return _InitializeForCISRegister_Watson(rowStart, colStart, width, height);
        case DEVICE_TYPE_SHERLOCK_ROIC:
        case DEVICE_TYPE_SHERLOCK:
            return _InitializeForCISRegister_Sherlock(rowStart, colStart, width, height);
        case DEVICE_TYPE_COLUMBO:
            return _InitializeForCISRegister_Columbo(rowStart, colStart, width, height);
        case DEVICE_TYPE_CURVE:
           	return _InitializeForCISRegister_Curve(rowStart, colStart, width, height);
		case DEVICE_TYPE_CURVE_SETI:
			return _InitializeForCISRegister_CurveSETi(rowStart, colStart, width, height);
        case DEVICE_TYPE_HOLMES:
            return _InitializeForCISRegister_Holmes(rowStart, colStart, width, height);
		case DEVICE_TYPE_KOJAK:
			if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
				return _InitializeForCISRegister_Half_Kojak(rowStart, colStart, m_UsbDeviceInfo.CisImgWidth_Roll, m_UsbDeviceInfo.CisImgHeight_Roll);
			else if( m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
					 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
					 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
				     m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
				return _InitializeForCISRegister_Full_Kojak(rowStart, colStart, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
			else
				return _InitializeForCISRegister_Full_Kojak(rowStart, colStart, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
        case DEVICE_TYPE_FIVE0:
            return _InitializeForCISRegister_Five0(rowStart, colStart, width, height);

        default:
            break;
    }

    return FALSE;
}

int CMainCapture::_SndUsbFwInitialize()
{
    if (_GetPID() == __PID_CURVE__)
    {
        return IBSU_STATUS_OK;
    }

    return _UsbBulkOutIn(EP1OUT, CMD_DEVICE_INITIALIZE, NULL, 0, -1, NULL, 0, NULL);
}

int CMainCapture::_SndUsbFwCaptureStart()
{
    if (_GetPID() == __PID_CURVE__)
    {
        return IBSU_STATUS_OK;
    }

    return _UsbBulkOutIn(EP1OUT, CMD_CAPTURE_START, NULL, 0, -1, NULL, 0, NULL);
}

int CMainCapture::_SndUsbFwCaptureStop()
{
    _SetLEOperationMode(ADDRESS_LE_OFF_MODE);		// Important! Always off LE to prevent electrical shock

    if (_GetPID() == __PID_CURVE__)
    {
        return IBSU_STATUS_OK;
    }
    // enzyme 2013-01-30 Comment Columbo firmware has a bug on ver 0.9.1
    else if ((_GetPID() == __PID_COLUMBO__/* || _GetPID() == __PID_COLUMBO_REV1__*/) &&
              strcmp(m_propertyInfo.cFirmware, "0.9.1") <= 0)
    {
        return IBSU_STATUS_OK;
    }
    else if ((_GetPID() == __PID_COLUMBO__/* || _GetPID() == __PID_COLUMBO_REV1__*/) &&
              strcmp(m_propertyInfo.cFirmware, "1.0.3") <= 0)
    {
        int nRc = _UsbBulkOutIn(EP1OUT, CMD_CAPTURE_END, NULL, 0, -1, NULL, 0, NULL);
        if (nRc == IBSU_STATUS_OK)
        {
            Sleep(500);    // To fix the firmware bug of Columbo
        }

        return nRc;
    }
	// enzyme 2014-07-08 Bug fixed - When Sherlock become to the power save mode, the power of ASIC did not turn off correctly.
	// So we solved this issue with software reset.
	else if ((_GetPID() == __PID_SHERLOCK__ || _GetPID() == __PID_SHERLOCK_REV1__) &&
              strcmp(m_propertyInfo.cProductID, "SHERLOCK") == 0)
	{
		ASIC_SetRegister(ASIC_MASTER_ADDRESS, 0x02, 0x60);
		Sleep(1);
		ASIC_SetRegister(ASIC_SLAVE_ADDRESS, 0x02, 0x60);
		Sleep(10);
	}

    return _UsbBulkOutIn(EP1OUT, CMD_CAPTURE_END, NULL, 0, -1, NULL, 0, NULL);
}

int CMainCapture::_SndUsbFwCaptureStop_atLibraryTerminated()
{
    if (_GetPID() == __PID_CURVE__)
    {
        return IBSU_STATUS_OK;
    }
    // enzyme 2013-01-30 Comment Columbo firmware has a bug on ver 0.9.1
    else if ((_GetPID() == __PID_COLUMBO__ || _GetPID() == __PID_COLUMBO_REV1__) &&
              strcmp(m_propertyInfo.cFirmware, "0.9.1") <= 0)
    {
        return IBSU_STATUS_OK;
    }
    else if ((_GetPID() == __PID_COLUMBO__ || _GetPID() == __PID_COLUMBO_REV1__) &&
              strcmp(m_propertyInfo.cFirmware, "1.0.3") <= 0)
    {
        int nRc = _UsbBulkOut(EP1OUT, CMD_CAPTURE_END, NULL, 0);
        if (nRc == IBSU_STATUS_OK)
        {
            Sleep(500);    // To fix the firmware bug of Columbo
        }

        return nRc;
    }

    return _UsbBulkOut(EP1OUT, CMD_CAPTURE_END, NULL, 0);
}

int CMainCapture::_SndUsbFwRelease()
{
    if (_GetPID() == __PID_CURVE__)
    {
        return IBSU_STATUS_OK;
    }

    return _UsbBulkOutIn(EP1OUT, CMD_DEVICE_RELEASE, NULL, 0, -1, NULL, 0, NULL);
}

int CMainCapture::_SndUsbFwRelease_atLibraryTerminated()
{
    if (_GetPID() == __PID_CURVE__)
    {
        return IBSU_STATUS_OK;
    }

    return _UsbBulkOut(EP1OUT, CMD_DEVICE_RELEASE, NULL, 0);
}

int CMainCapture::_SndUsbFwGotoSleep()
{
    if (_GetPID() == __PID_CURVE__)
    {
        return IBSU_STATUS_OK;
    }

#if defined(__embedded__) || defined(WINCE)
    return _UsbBulkOutIn(EP1OUT, CMD_DEVICE_RELEASE, NULL, 0, -1, NULL, 0, NULL);
#else
    return IBSU_STATUS_OK;
#endif
}

int CMainCapture::_SndUsbFwInitLEVoltage()
{
    return _UsbBulkOutIn(EP1OUT, CMD_INIT_LE_VOLTAGE, NULL, 0, -1, NULL, 0, NULL);
}

int CMainCapture::_GetProperty(DEV_INFO_ADDRESS addr, BYTE *inBuffer)
{
    UCHAR		outBuffer[64] = {0};
    LONG		nInBufferLen = 32;
    int			nRc;
	UCHAR tmpRead[IBSU_MAX_STR_LEN]={0};
	UCHAR tmpRead2[IBSU_MAX_STR_LEN]={0};


	memset(inBuffer, 0, IBSU_MAX_STR_LEN);
	outBuffer[0] = (UCHAR)addr;


    if (_GetPID() == __PID_CURVE__)
    {
		CurveModel curveModel = _GetCurveModel(&m_propertyInfo, &m_UsbDeviceInfo);
		// CURVE_MODEL_TBN320_V1 and CURVE_MODEL_TBN340_V1 can get property data from Cypress.
		// However other models cannot get property data from Cypress
		if (curveModel == CURVE_MODEL_CYTE ||
			curveModel == CURVE_MODEL_CYTE_V1 ||
			curveModel == CURVE_MODEL_CYTE_V2 ||
			curveModel == CURVE_MODEL_TBN240 ||
			curveModel == CURVE_MODEL_TBN320 ||
			curveModel == CURVE_MODEL_TBN340)
		{
			switch (addr)
			{
				case PROPERTY_PRODUCT_ID:
					sprintf((char *)inBuffer, "CURVE");
					break;
				case PROPERTY_SERIAL_NUMBER:
					if (IsInitialized())
					{
						strcpy((char *)inBuffer, m_propertyInfo.cSerialNumber);
						break;
					}
					else
					{
#if defined(_WINDOWS) && defined(__USE_WINUSB_DRIVER__)
						m_pCurveUsbDevice->GetSerialNumber(inBuffer);
						break;
#else
						_GetSerialNumber_Curve(inBuffer);
						break;
#endif
					}
				case PROPERTY_FIRMWARE:
					if (curveModel == CURVE_MODEL_CYTE)
					{
						sprintf((char *)inBuffer, "1.0.0");
					}
					else if (curveModel == CURVE_MODEL_CYTE_V1)
					{
						sprintf((char *)inBuffer, "1.0.1");
					}
					else if (curveModel == CURVE_MODEL_CYTE_V2)
					{
						sprintf((char *)inBuffer, "1.0.2");
					}
					else if (curveModel == CURVE_MODEL_TBN240)
					{
						sprintf((char *)inBuffer, "1.0.3");
					}
					else
					{
						sprintf((char *)inBuffer, "2.0.0");
					}
					break;
				default:
					inBuffer[0] = '\0';
					break;
			}

			return IBSU_STATUS_OK;
		}
		// CURVE_MODEL_TBN320_V1 and CURVE_MODEL_TBN340_V1 can get property data from Cypress.
		else if (curveModel == CURVE_MODEL_TBN320_V1 ||
			curveModel == CURVE_MODEL_TBN340_V1)
		{
			if (addr == PROPERTY_SERIAL_NUMBER)
			{
				if (IsInitialized())
				{
					strcpy((char *)inBuffer, m_propertyInfo.cSerialNumber);
				}
				else
				{
#if defined(_WINDOWS) && defined(__USE_WINUSB_DRIVER__)
					if(_GetCurveVersion() >= ((0x03<<16) | (0x00<<8) | 0x00))
						m_pCurveUsbDevice->GetSerialNumber(inBuffer, TRUE);
					else
						m_pCurveUsbDevice->GetSerialNumber(inBuffer, FALSE);
#else
					_GetSerialNumber_Curve(inBuffer);
#endif
				}
				nRc = IBSU_STATUS_OK;
			}
			else
			{
				outBuffer[0] = (UCHAR)CMD_READ_PROPERTY;
				outBuffer[1] = (UCHAR)addr;
				nRc = _UsbBulkOutIn(EP1OUT, 0x02, outBuffer, 2,
									EP1IN, (PUCHAR)tmpRead2, nInBufferLen, NULL);
				if (nRc == IBSU_STATUS_OK)
				{
					if (addr == PROPERTY_PRODUCT_ID)
					{
						int matchedIndex=-1;
						int count=0;
						for (int i=0; i<32; i++)
						{
							if ((tmpRead2[i] >= '0' && tmpRead2[i] <= '9') ||
								(tmpRead2[i] >= 'A' && tmpRead2[i] <= 'Z') ||
								(tmpRead2[i] >= 'a' && tmpRead2[i] <= 'z'))
							{
								if (count++ >=3)
								{
									matchedIndex = i - 3;
									break;
								}
							}
						}

						if (matchedIndex != 0)
                        {
							matchedIndex = (matchedIndex == -1)? nInBufferLen: matchedIndex;
					        nRc = _UsbBulkOutIn(-1, 0x02, NULL, 0,
											EP1IN, (PUCHAR)tmpRead, matchedIndex, NULL, 100);
TRACE ("PROPERTY_PRODUCT_ID read failed ==> matchedCorrectionIndex = %d\n", matchedIndex);
							return IBSU_ERR_CHANNEL_IO_READ_TIMEOUT;
                        }
					}

					memcpy(inBuffer, tmpRead2, nInBufferLen);
				}
			}

			return nRc;
		}
		else
		{
			return IBSU_ERR_NOT_SUPPORTED;
		}
    }
#ifdef DEBUG // Stick DEADBEEF in the buffer so if we can see if it comes back untouched.
	*(inBuffer+0) = 0xDE;
	*(inBuffer+1) = 0xAD;
	*(inBuffer+2) = 0xBE;
	*(inBuffer+3) = 0xEF;
#endif

    nRc = _UsbBulkOutIn(EP1OUT, CMD_READ_PROPERTY, outBuffer, 1,
                        EP1IN, (PUCHAR)tmpRead2, nInBufferLen, NULL);
	if (nRc == IBSU_STATUS_OK)
	{
		if (addr == PROPERTY_PRODUCT_ID)
		{
			int matchedIndex=-1;
			int count=0;
			for (int i=0; i<32; i++)
			{
				if ((tmpRead2[i] >= '0' && tmpRead2[i] <= '9') ||
					(tmpRead2[i] >= 'A' && tmpRead2[i] <= 'Z') ||
					(tmpRead2[i] >= 'a' && tmpRead2[i] <= 'z'))
            {
					if (count++ >=3)
					{
						matchedIndex = i - 3;
						break;
					}
				}
			}

			if (matchedIndex != 0)
			{
				matchedIndex = (matchedIndex == -1)? nInBufferLen: matchedIndex;
		        nRc = _UsbBulkOutIn(-1, 0x02, NULL, 0,
						        EP1IN, (PUCHAR)tmpRead, matchedIndex, NULL, 100);
TRACE ("PROPERTY_PRODUCT_ID read failed ==> matchedCorrectionIndex = %d\n", matchedIndex);
				return IBSU_ERR_CHANNEL_IO_READ_TIMEOUT;
            }
		}

		memcpy(inBuffer, tmpRead2, nInBufferLen);
	}

#ifdef DEBUG
//	CaptureLibDumpOutput( "CaptureLib:CMainCapture::_GetProperty data returned", inBuffer, min(nDataReturned, 128) );
#endif

    return nRc;
}

#ifdef DEBUG
void CaptureLibDumpOutput( char *szTitle, UCHAR *pBuffer, size_t Count )
{
#ifdef UNICODE
	WCHAR OutBuffer[512];
	int LoopCount = 0;
	while( Count )
	{
		WCHAR *pwchPos = OutBuffer;
		wsprintfW(pwchPos, TEXT("%S:%08x:%02x:"), szTitle, pBuffer,(LoopCount++)*8);
		pwchPos += wcslen(pwchPos);

		for( int i = 0; i < 7 && Count > 1; i++, Count-- )
		{
			wsprintfW(pwchPos, TEXT("%02x,"), *pBuffer++);
			pwchPos += wcslen(pwchPos); 
		}
		if( Count )
		{
			wsprintfW(pwchPos, TEXT("%02x"), *pBuffer++);
			Count--;
			pwchPos += wcslen(pwchPos); 
		}
		TRACE(TEXT("%s\r\n"), OutBuffer); 

	}
#else // matches #ifdef UNICODE
	char OutBuffer[512];
	int LoopCount = 0;
	while( Count )
	{
		char *pchPos = OutBuffer;
		sprintf(pchPos, TEXT("%s:%08x:%02x:"), szTitle, pBuffer,(LoopCount++)*8);
		pchPos += strlen(pchPos);

		for( int i = 0; i < 7 && Count > 1; i++, Count-- )
		{
			sprintf(pchPos, TEXT("%02x,"), *pBuffer++);
			pchPos += strlen(pchPos); 
		}
		if( Count )
		{
			sprintf(pchPos, TEXT("%02x"), *pBuffer++);
			Count--;
			pchPos += strlen(pchPos); 
		}
		TRACE(TEXT("%s\r\n"), OutBuffer); 
	}
#endif 
}

#endif

int CMainCapture::_SetProperty(DEV_INFO_ADDRESS addr, BYTE *Buffer, size_t Count)
{
    UCHAR		outBuffer[64] = {0};
    size_t		nBufferLen = 32;

    if (_GetPID() == __PID_CURVE__)
    {
		CurveModel curveModel = _GetCurveModel(&m_propertyInfo, &m_UsbDeviceInfo);
		// CURVE_MODEL_TBN320_V1 and CURVE_MODEL_TBN340_V1 can get property data from Cypress.
		// However other models cannot get property data from Cypress
		if (curveModel == CURVE_MODEL_TBN320_V1 ||
			curveModel == CURVE_MODEL_TBN340_V1)
		{
			if (addr == PROPERTY_SERIAL_NUMBER)
			{
				return IBSU_ERR_NOT_SUPPORTED;		// will use existing program to write USN to Curve.
			}
			else
			{
				outBuffer[0] = (UCHAR)CMD_READ_PROPERTY;
				outBuffer[1] = (UCHAR)addr;
				outBuffer[2] = (UCHAR)nBufferLen;
	
				if (Count > nBufferLen)
				{
					Count = nBufferLen;
				}
	
				for (size_t i = 0; i < Count; i++)
				{
					outBuffer[i + 3] = Buffer[i];
				}
				for (size_t i = Count; i < (size_t)nBufferLen; i++)
				{
					outBuffer[i + 3] = 0;
				}
			
				return _UsbBulkOutIn(EP1OUT, 0x03 + nBufferLen, outBuffer, 3 + nBufferLen, -1, NULL, 0, NULL);
			}
		}
		else
		{
			return IBSU_ERR_NOT_SUPPORTED;
		}
	}

    outBuffer[0] = (UCHAR)addr;
    outBuffer[1] = (UCHAR)nBufferLen;

    if (Count > nBufferLen)
    {
        Count = nBufferLen;
    }

    for (size_t i = 0; i < Count; i++)
    {
        outBuffer[i + 2] = Buffer[i];
    }
    for (size_t i = Count; i < (size_t)nBufferLen; i++)
    {
        outBuffer[i + 2] = 0;
    }

    return _UsbBulkOutIn(EP1OUT, CMD_WRITE_PROPERTY, outBuffer, 2 + nBufferLen, -1, NULL, 0, NULL);
}

int CMainCapture::_GetLEOperationMode(WORD addr, WORD *val)
{
    UCHAR		inBuffer[MAX_DATA_BYTES + 1] = {0};
    int			nRc;

    nRc = _UsbBulkOutIn(EP1OUT, CMD_READ_LE_MODE, NULL, 0,
                        EP1IN, (PUCHAR)inBuffer, 1, NULL);
    if (nRc == IBSU_STATUS_OK)
    {
        *val = inBuffer[0];
    }
    else
    {
        *val = 0;
    }

    return nRc;
}

int CMainCapture::_SetLEOperationMode(WORD addr)
{
    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
        case DEVICE_TYPE_WATSON_MINI:
            return _SetLEOperationMode_Watson(addr);
        case DEVICE_TYPE_SHERLOCK_ROIC:
        case DEVICE_TYPE_SHERLOCK:
            return _SetLEOperationMode_Sherlock(addr);
        case DEVICE_TYPE_COLUMBO:
            return _SetLEOperationMode_Columbo(addr);
        case DEVICE_TYPE_CURVE:
			return _SetLEOperationMode_Curve(addr);
		case DEVICE_TYPE_CURVE_SETI:
			return _SetLEOperationMode_CurveSETi(addr);
        case DEVICE_TYPE_HOLMES:
            return _SetLEOperationMode_Holmes(addr);
		case DEVICE_TYPE_KOJAK:
            return _SetLEOperationMode_Kojak(addr);
		case DEVICE_TYPE_FIVE0:
            return _SetLEOperationMode_Five0(addr);
        default:
            break;
    }

    return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
}

int CMainCapture::_GetTouchInStatus(WORD *val)
{
    UCHAR		inBuffer[MAX_DATA_BYTES + 1] = {0};
    int			nRc;

    nRc = _UsbBulkOutIn(EP1OUT, CMD_READ_TOUCH_IN_STATUS, NULL, 0,
                        EP1IN, (PUCHAR)inBuffer, 1, NULL);

    if (nRc == IBSU_STATUS_OK)
    {
        *val = inBuffer[0];
    }
    else
    {
        *val = 0;
    }

    return nRc;
}

int CMainCapture::_GetActiveLEDs(DWORD *val)
{
    UCHAR		inBuffer[5] = {0};
    int			nRc;

    nRc = _UsbBulkOutIn(EP1OUT, CMD_READ_LED, NULL, 0,
                        EP1IN, (PUCHAR)inBuffer, 4, NULL);

    *val = atoi((char *)inBuffer);

    return nRc;
}

int CMainCapture::_SetActiveLEDs(WORD addr, DWORD val)
{
    UCHAR		outBuffer[64] = {0};
    char        buf[64] = {0};
    int         len = 4;

    sprintf(buf, "%04u", val);
    if ((m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE && m_UsbDeviceInfo.curveModel == CURVE_MODEL_TBN240) ||
		(m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE_SETI && m_UsbDeviceInfo.curveModel ==  CURVE_MODEL_TBN340) ||
		(m_UsbDeviceInfo.devType == DEVICE_TYPE_CURVE_SETI && m_UsbDeviceInfo.curveModel ==  CURVE_MODEL_TBN340_V1))
    {
        outBuffer[0] = (UCHAR)NA2CTL_LED_CONTROL;
        outBuffer[1] = (UCHAR)0;
        outBuffer[2] = (UCHAR)0;
        outBuffer[3] = (UCHAR)0;
        if ((val & IBSU_LED_SCAN_CURVE_RED) == IBSU_LED_SCAN_CURVE_RED)
        {
            outBuffer[1] = (UCHAR)1;
        }
        if ((val & IBSU_LED_SCAN_CURVE_GREEN) == IBSU_LED_SCAN_CURVE_GREEN)
        {
            outBuffer[2] = (UCHAR)1;
        }
        if ((val & IBSU_LED_SCAN_CURVE_BLUE) == IBSU_LED_SCAN_CURVE_BLUE)
        {
            outBuffer[3] = (UCHAR)1;
        }

        return _UsbBulkOutIn(EP1OUT, 0x03/*BufSize*/, outBuffer, 4, -1, NULL, 0, NULL);
    }
	else if(m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
	{
		return _SetActiveLEDs_Kojak(addr, val);
	}

    outBuffer[0] = (UCHAR)len;
    for (int i = 0; i < len; i++)
    {
        outBuffer[i + 1] = buf[i];
    }

    return _UsbBulkOutIn(EP1OUT, CMD_WRITE_LED, outBuffer, 1 + len, -1, NULL, 0, NULL);
}

int CMainCapture::_GetLEVoltage(int *voltageValue)
{
    if (m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON && strcmp(m_propertyInfo.cFirmware, "0.14.0") < 0)
    {
        return IBSU_ERR_NOT_SUPPORTED;
    }

    UCHAR		inBuffer[MAX_DATA_BYTES + 1] = {0};
    int			nRc;

    nRc = _UsbBulkOutIn(EP1OUT, CMD_READ_LE_VOLTAGE, NULL, 0,
                        EP1IN, (PUCHAR)inBuffer, 1, NULL);

    if (nRc == IBSU_STATUS_OK)
    {
        *voltageValue = inBuffer[0];
    }
    else
    {
        *voltageValue = 0;
    }

    return nRc;
}

int CMainCapture::_SetLEVoltage(int voltageValue)
{
    if (m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON && strcmp(m_propertyInfo.cFirmware, "0.14.0") < 0)
    {
        return IBSU_ERR_NOT_SUPPORTED;
    }

    UCHAR		outBuffer[64] = {0};

    outBuffer[0] = (UCHAR)voltageValue;
	
	if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK )
	{
		if(m_UsbDeviceInfo.nFpgaVersion ==  ( (0 << 16) | (8 << 8) | 0) ||
			m_UsbDeviceInfo.nFpgaVersion ==  ( (0 << 16) | (8 << 8) | 1) ||
			m_UsbDeviceInfo.nFpgaVersion ==  ( (0 << 16) | (8 << 8) | 2) )
		{
			float val =(1.0f/9.0f) * voltageValue*voltageValue - (26.0f/9.0f) * voltageValue + (304.0f/9.0f);
			
			val = ceilf(val);
			outBuffer[1] = (UCHAR)val;         //25 - (UCHAR)voltageValue;
			outBuffer[2] = 0x80;
			return _UsbBulkOutIn(EP1OUT, CMD_WRITE_LE_VOLTAGE, outBuffer, 3, -1, NULL, 0, NULL);
		}
		else if(m_UsbDeviceInfo.nFpgaVersion >=  ((0 << 24) | (9 << 16) | (0 << 8) | 0x0D) )			// GON
		{
			return _UsbBulkOutIn(EP1OUT, CMD_WRITE_LE_VOLTAGE_DAC, outBuffer, 1, -1, NULL, 0, NULL);
		}
		else if(m_UsbDeviceInfo.nFpgaVersion >=  ((0 << 24) | (8 << 16) | (3 << 8) | 0x00) )
		{
			outBuffer[1] = 0x1b;         //25 - (UCHAR)voltageValue;
			outBuffer[2] = 0x80;         //25 - (UCHAR)voltageValue;
			return _UsbBulkOutIn(EP1OUT, CMD_WRITE_LE_VOLTAGE, outBuffer, 3, -1, NULL, 0, NULL);
		}
	}

    return _UsbBulkOutIn(EP1OUT, CMD_WRITE_LE_VOLTAGE, outBuffer, 1, -1, NULL, 0, NULL);
}

void CMainCapture::_PostCallback(CallbackType clbkIndex, BYTE *OutImg, double frameTime, int progressValue, int warningCode, BOOL bIsFinal)
{
    CThreadSync Sync;			// enzyme add 2012-11-08

    if (clbkIndex != CALLBACK_KEYBUTTON && m_bCaptureThread_StopMessage)
        return;

    sprintf(m_clbkParam[clbkIndex].traceLog.category, "IBSU_Device_%s(%s)", m_UsbDeviceInfo.productName, m_UsbDeviceInfo.serialNumber);
    m_clbkParam[clbkIndex].traceLog.error = 0;
    m_clbkParam[clbkIndex].traceLog.tid = _GetThreadID();

    switch (clbkIndex)
    {
        case CALLBACK_INIT_PROGRESS:
            m_clbkProperty.nProgressValue = progressValue;
            if (m_pDlgUsbManager->m_bIsReadySend[clbkIndex])
            {
//                m_clbkProperty.nProgressValue = progressValue;
                if (m_pDlg->m_Clbk_InitProgress)
                {
                    m_pDlgUsbManager->m_bIsReadySend[clbkIndex] = FALSE;
                    m_clbkParam[clbkIndex].deviceHandle = GetDeviceHandle();
                    m_clbkParam[clbkIndex].callback = (void *)m_pDlg->m_Clbk_InitProgress;
                    m_clbkParam[clbkIndex].context = m_pDlg->m_pContext_InitProgress;

                    m_pDlg->SubmitJob((LPVOID)&m_clbkParam[clbkIndex]);
                }
            }
            return;

        case CALLBACK_ASYNC_OPEN_DEVICE:
            if (m_pDlgUsbManager->m_bIsReadySend[clbkIndex])
            {
                if (m_pDlg->m_Clbk_AsyncOpenDevice)
                {
                    m_pDlgUsbManager->m_bIsReadySend[clbkIndex] = FALSE;
                    m_clbkParam[clbkIndex].deviceHandle = -1;
                    m_clbkParam[clbkIndex].callback = (void *)m_pDlg->m_Clbk_AsyncOpenDevice;
                    m_clbkParam[clbkIndex].context = m_pDlg->m_pContext_AsyncOpenDevice;

                    m_pDlg->SubmitJob((LPVOID)&m_clbkParam[clbkIndex]);
                }
            }
            return;

        case CALLBACK_COMMUNICATION_BREAK:
            /*		m_pCaptureThread = 0;
            		m_pDlg->UpdateIndexInList(m_pDlg->m_nDeviceCount);
            		SetIsCommunicationBreak(TRUE);
            #ifdef _WINDOWS
            		SetDeviceIndex(-1);
            #endif
            */
            m_clbkParam[clbkIndex].traceLog.code = LOG_ERROR;
            break;
        case CALLBACK_FINGER_QUALITY:
            memcpy(m_blockProperty.qualityArray, m_clbkProperty.qualityArray, sizeof(m_blockProperty.qualityArray));
            break;
        case CALLBACK_PREVIEW_IMAGE:
            m_clbkProperty.imageInfo.FrameTime = frameTime;
            m_clbkProperty.imageInfo.BitsPerPixel = 8;
            m_clbkProperty.imageInfo.Format = IBSU_IMG_FORMAT_GRAY;
            m_clbkProperty.imageInfo.Height = m_UsbDeviceInfo.ImgHeight;
            m_clbkProperty.imageInfo.Width = m_UsbDeviceInfo.ImgWidth;
			// For decimated images, the resolution of data may be lower than 
			// the resolution of the image.  But we return resolution of image anyway.
            m_clbkProperty.imageInfo.ResolutionX = m_UsbDeviceInfo.baseResolutionX;
            m_clbkProperty.imageInfo.ResolutionY = m_UsbDeviceInfo.baseResolutionY;
            m_clbkProperty.imageInfo.Pitch = -m_UsbDeviceInfo.ImgWidth;
#if defined(__IBSCAN_SDK__)
            m_clbkProperty.imageInfo.IsFinal = bIsFinal;
#elif defined(__IBSCAN_ULTIMATE_SDK__)
            m_clbkProperty.imageInfo.IsFinal = FALSE;
			m_clbkProperty.imageInfo.ProcessThres = m_propertyInfo.nImageProcessThres;
#endif
			if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
			{
				m_clbkProperty.imageInfo.Height = m_UsbDeviceInfo.ImgHeight_Roll;
				m_clbkProperty.imageInfo.Width = m_UsbDeviceInfo.ImgWidth_Roll;
				// For decimated images, the resolution of data may be lower than 
				// the resolution of the image.  But we return resolution of image anyway.
				m_clbkProperty.imageInfo.Pitch = -m_UsbDeviceInfo.ImgWidth_Roll;
				memcpy(m_pAlgo->m_OutResultImg, OutImg, m_UsbDeviceInfo.ImgSize_Roll);
			}
            else
            {
			    memcpy(m_pAlgo->m_OutResultImg, OutImg, m_UsbDeviceInfo.ImgSize);
            }

            m_clbkProperty.imageInfo.Buffer = (BYTE *)m_pAlgo->m_OutResultImg;

            // For blocking funtions (IBSU_BGetImage(), IBSU_BGetInitProgress(), IBSU_BGetClearPlatenAtCapture())
            {
                m_blockProperty.imageType				= m_propertyInfo.ImageType;
                m_blockProperty.pSplitImageArray		= m_clbkProperty.pSplitImageArray;
                m_blockProperty.splitImageArrayCount	= m_clbkProperty.splitImageArrayCount;
                m_blockProperty.nFingerState			= m_clbkProperty.nFingerState;
                memcpy(&m_blockProperty.imageInfo, &m_clbkProperty.imageInfo, sizeof(IBSU_ImageData));
//                memcpy(m_blockProperty.qualityArray, m_clbkProperty.qualityArray, sizeof(m_blockProperty.qualityArray));
                m_blockProperty.bGetOneFrameImage		= TRUE;
            }

			memcpy(m_pAlgo->m_EncryptTemp, m_pAlgo->m_EncryptImage, m_UsbDeviceInfo.CisImgSize);
            break;
        case CALLBACK_RESULT_IMAGE:
            m_clbkProperty.imageInfo.FrameTime = frameTime;
			m_clbkProperty.imageInfo.BitsPerPixel = 8;
			m_clbkProperty.imageInfo.Format = IBSU_IMG_FORMAT_GRAY;
			m_clbkProperty.imageInfo.Height = m_UsbDeviceInfo.ImgHeight;
			m_clbkProperty.imageInfo.Width = m_UsbDeviceInfo.ImgWidth;
			m_clbkProperty.imageInfo.ResolutionX = m_UsbDeviceInfo.baseResolutionX;
			m_clbkProperty.imageInfo.ResolutionY = m_UsbDeviceInfo.baseResolutionY;
			m_clbkProperty.imageInfo.Pitch = -m_UsbDeviceInfo.ImgWidth;
            m_clbkProperty.imageInfo.IsFinal = TRUE;
#if defined(__IBSCAN_ULTIMATE_SDK__)
			m_clbkProperty.imageInfo.ProcessThres = IMAGE_PROCESS_THRES_PERFECT;
#endif
			if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
			{
				m_clbkProperty.imageInfo.Height = m_UsbDeviceInfo.ImgHeight_Roll;
				m_clbkProperty.imageInfo.Width = m_UsbDeviceInfo.ImgWidth_Roll;
				m_clbkProperty.imageInfo.Pitch = -m_UsbDeviceInfo.ImgWidth_Roll;
				memcpy(m_pAlgo->m_OutResultImg, OutImg, m_UsbDeviceInfo.ImgSize_Roll);
			}
			else
			{
				memcpy(m_pAlgo->m_OutResultImg, OutImg, m_UsbDeviceInfo.ImgSize);
			}
			m_clbkProperty.imageInfo.Buffer = (BYTE *)m_pAlgo->m_OutResultImg;

			// 2012-04-03 enzyme -- Adding for split image
            memset(&m_propertyInfo.splitImageInfoArray[0], 0, IBSU_MAX_SEGMENT_COUNT*sizeof(IBSU_ImageData));
            for (int i = 0; i < m_pAlgo->m_segment_arr.SegmentCnt; i++)
            {
                memcpy(&m_propertyInfo.splitImageInfoArray[i], &m_clbkProperty.imageInfo, sizeof(IBSU_ImageData));
                m_propertyInfo.splitImageInfoArray[i].Buffer = (BYTE *)&m_pAlgo->m_OutSplitResultArray[i * m_clbkProperty.imageInfo.Width * m_clbkProperty.imageInfo.Height];
            }
            m_clbkProperty.pSplitImageArray = &m_propertyInfo.splitImageInfoArray[0];
            m_clbkProperty.splitImageArrayCount = m_pAlgo->m_segment_arr.SegmentCnt;

            // For blocking funtions (IBSU_BGetImage(), IBSU_BGetInitProgress(), IBSU_BGetClearPlatenAtCapture())
            {
                m_blockProperty.imageType				= m_propertyInfo.ImageType;
                m_blockProperty.pSplitImageArray		= m_clbkProperty.pSplitImageArray;
                m_blockProperty.splitImageArrayCount	= m_clbkProperty.splitImageArrayCount;
                m_blockProperty.nFingerState			= m_clbkProperty.nFingerState;
                memcpy(&m_blockProperty.imageInfo, &m_clbkProperty.imageInfo, sizeof(IBSU_ImageData));
                memcpy(m_blockProperty.qualityArray, m_clbkProperty.qualityArray, sizeof(m_blockProperty.qualityArray));
                m_blockProperty.bGetOneFrameImage		= TRUE;

                m_blockProperty.bFillResultImage		= TRUE;
            }
            break;
		case CALLBACK_RESULT_IMAGE_EX:
			m_clbkProperty.imageInfo.FrameTime = frameTime;
			m_clbkProperty.imageInfo.BitsPerPixel = 8;
			m_clbkProperty.imageInfo.Format = IBSU_IMG_FORMAT_GRAY;
			m_clbkProperty.imageInfo.Height = m_UsbDeviceInfo.ImgHeight;
			m_clbkProperty.imageInfo.Width = m_UsbDeviceInfo.ImgWidth;
			m_clbkProperty.imageInfo.ResolutionX = m_UsbDeviceInfo.baseResolutionX;
			m_clbkProperty.imageInfo.ResolutionY = m_UsbDeviceInfo.baseResolutionY;
			m_clbkProperty.imageInfo.Pitch = -m_UsbDeviceInfo.ImgWidth;
			m_clbkProperty.imageInfo.IsFinal = TRUE;
#if defined(__IBSCAN_ULTIMATE_SDK__)
			m_clbkProperty.imageInfo.ProcessThres = IMAGE_PROCESS_THRES_PERFECT;
#endif
			m_clbkProperty.nResultImageStatus = IBSU_STATUS_OK;
//			m_clbkProperty.nDetectedFingerCount = m_pAlgo->m_cImgAnalysis.finger_count;
			m_clbkProperty.nDetectedFingerCount = m_pAlgo->m_segment_arr.SegmentCnt;

			if( (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK || m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0) &&
				m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER &&
				m_propertyInfo.bWarningInvalidArea == TRUE)
			{
				if( (m_pAlgo->nInvalidAreaDetected >= IBSU_WRN_QUALITY_INVALID_AREA_HORIZONTALLY)
					&&(m_pAlgo->nInvalidAreaDetected <= (IBSU_WRN_QUALITY_INVALID_AREA_HORIZONTALLY | IBSU_WRN_QUALITY_INVALID_AREA_VERTICALLY)))
				{
					m_clbkProperty.nResultImageStatus = m_pAlgo->nInvalidAreaDetected;
					m_pAlgo->nInvalidAreaDetected = 0;
				}
			}

			if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
			{
				m_clbkProperty.imageInfo.Height = m_UsbDeviceInfo.ImgHeight_Roll;
				m_clbkProperty.imageInfo.Width = m_UsbDeviceInfo.ImgWidth_Roll;
				m_clbkProperty.imageInfo.Pitch = -m_UsbDeviceInfo.ImgWidth_Roll;
				memcpy(m_pAlgo->m_OutResultImg, OutImg, m_UsbDeviceInfo.ImgSize_Roll);
			}
			else
			{
				memcpy(m_pAlgo->m_OutResultImg, OutImg, m_UsbDeviceInfo.ImgSize);
			}

            m_clbkProperty.nDetectedFingerCount = m_pAlgo->m_segment_arr.SegmentCnt;
			m_clbkProperty.imageInfo.Buffer = (BYTE *)m_pAlgo->m_OutResultImg;

			if( !bIsFinal )
				m_clbkProperty.nResultImageStatus = IBSU_ERR_CAPTURE_TIMEOUT;
			else if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
			{
			//	TRACE("BestFrame = %d, bSmearDectected = %d\n", m_pAlgo->bBestFrameSmearDectected, m_pAlgo->bSmearDectected);
				if( m_UsbDeviceInfo.bSmearDetection && m_pAlgo->bBestFrameSmearDectected )
                {
				    m_clbkProperty.nResultImageStatus = IBSU_WRN_ROLLING_SMEAR;

				    if( m_pAlgo->bBestFrameSmearDirectionHor )
					    m_clbkProperty.nResultImageStatus |= IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY;

				    if( m_pAlgo->bBestFrameSmearDirectionVer )
					    m_clbkProperty.nResultImageStatus |= IBSU_WRN_ROLLING_SHIFTED_VERTICALLY;
                }
			}
			else if( m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER )
			{
				if( m_clbkProperty.nDetectedFingerCount == 0 )
					m_clbkProperty.nResultImageStatus = IBSU_WRN_NO_FINGER;
				else if( m_clbkProperty.nDetectedFingerCount != m_propertyInfo.nNumberOfObjects )
					m_clbkProperty.nResultImageStatus = IBSU_WRN_INCORRECT_FINGERS;
			}

			// spoof warning
			if( m_UsbDeviceInfo.devType == DEVICE_TYPE_COLUMBO &&
				m_propertyInfo.bIsSpoofSupported == TRUE &&
				m_UsbDeviceInfo.bSpoofDetected == TRUE)
			{
				m_clbkProperty.nResultImageStatus = IBSU_WRN_SPOOF_DETECTED;
			}

			// is it ok to use for submit image?
			/*if(m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK && 
				(m_pAlgo->m_cImgAnalysis.mean < m_FullCaptureSetting.TargetMinBrightness ||
				m_pAlgo->m_cImgAnalysis.mean > 90))*/
			if( (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK || m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0) && 
				(m_UsbDeviceInfo.CapturedBrightness[0] == IBSU_WRN_WET_FINGERS ||
				 m_UsbDeviceInfo.CapturedBrightness[1] == IBSU_WRN_WET_FINGERS ||
				 m_UsbDeviceInfo.CapturedBrightness[2] == IBSU_WRN_WET_FINGERS ||
				 m_UsbDeviceInfo.CapturedBrightness[3] == IBSU_WRN_WET_FINGERS))
			{
				m_clbkProperty.nResultImageStatus = IBSU_WRN_WET_FINGERS;
			}
#ifdef __G_DEBUG__
			TRACE("m_pAlgo->m_cImgAnalysis.mean : %d\n", m_pAlgo->m_cImgAnalysis.mean);
#endif

//			memcpy(m_pAlgo->m_OutResultImg, OutImg, m_UsbDeviceInfo.ImgSize);
			m_clbkProperty.imageInfo.Buffer = (BYTE*)m_pAlgo->m_OutResultImg;
			// For split images
            memset(&m_propertyInfo.splitImageInfoArray[0], 0, IBSU_MAX_SEGMENT_COUNT*sizeof(IBSU_ImageData));
			for( int i=0; i<m_clbkProperty.nDetectedFingerCount; i++ )
			{
				memcpy(&m_propertyInfo.splitImageInfoArray[i], &m_clbkProperty.imageInfo, sizeof(IBSU_ImageData));
				m_propertyInfo.splitImageInfoArray[i].Buffer = (BYTE*)&m_pAlgo->m_OutSplitResultArray[i*m_clbkProperty.imageInfo.Width*m_clbkProperty.imageInfo.Height];
			}
			m_clbkProperty.pSplitImageArray = &m_propertyInfo.splitImageInfoArray[0];
			m_clbkProperty.splitImageArrayCount = m_clbkProperty.nDetectedFingerCount;

			// For segment images
            memset(&m_propertyInfo.segmentArray[0], 0, IBSU_MAX_SEGMENT_COUNT*sizeof(IBSU_ImageData));
			for( int i=0; i<m_clbkProperty.nDetectedFingerCount; i++ )
			{
				int x1, y1, x2, y2, x3, y3, x4, y4;
				memcpy(&m_propertyInfo.segmentArray[i], &m_clbkProperty.imageInfo, sizeof(IBSU_ImageData));
				m_propertyInfo.segmentArray[i].Width = m_pAlgo->m_OutSplitResultArrayExWidth[i];
				m_propertyInfo.segmentArray[i].Height = m_pAlgo->m_OutSplitResultArrayExHeight[i];
				m_propertyInfo.segmentArray[i].Pitch = -m_pAlgo->m_OutSplitResultArrayExWidth[i];
//				m_propertyInfo.segmentArray[i].Buffer = (BYTE*)&m_pAlgo->m_OutSplitResultArrayEx[i*m_clbkProperty.imageInfo.Width*m_clbkProperty.imageInfo.Height];
				
				if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
				{
					m_propertyInfo.segmentArray[i].Buffer = (BYTE*)&m_pAlgo->m_OutSplitResultArrayEx[i*m_pAlgo->IMG_SIZE_ROLL];//m_clbkProperty.imageInfo.Width*m_clbkProperty.imageInfo.Height];

					x1 = (m_pAlgo->m_segment_arr.Segment[i].P1_X-m_pAlgo->ENLARGESIZE_ZOOM_W_ROLL) * m_pAlgo->IMG_W_ROLL / m_pAlgo->ZOOM_W_ROLL;
					y1 = (m_pAlgo->m_segment_arr.Segment[i].P1_Y-m_pAlgo->ENLARGESIZE_ZOOM_H_ROLL) * m_pAlgo->IMG_H_ROLL / m_pAlgo->ZOOM_H_ROLL;

					x2 = (m_pAlgo->m_segment_arr.Segment[i].P2_X-m_pAlgo->ENLARGESIZE_ZOOM_W_ROLL) * m_pAlgo->IMG_W_ROLL / m_pAlgo->ZOOM_W_ROLL;
					y2 = (m_pAlgo->m_segment_arr.Segment[i].P2_Y-m_pAlgo->ENLARGESIZE_ZOOM_H_ROLL) * m_pAlgo->IMG_H_ROLL / m_pAlgo->ZOOM_H_ROLL;

					x3 = (m_pAlgo->m_segment_arr.Segment[i].P3_X-m_pAlgo->ENLARGESIZE_ZOOM_W_ROLL) * m_pAlgo->IMG_W_ROLL / m_pAlgo->ZOOM_W_ROLL;
					y3 = (m_pAlgo->m_segment_arr.Segment[i].P3_Y-m_pAlgo->ENLARGESIZE_ZOOM_H_ROLL) * m_pAlgo->IMG_H_ROLL / m_pAlgo->ZOOM_H_ROLL;

					x4 = (m_pAlgo->m_segment_arr.Segment[i].P4_X-m_pAlgo->ENLARGESIZE_ZOOM_W_ROLL) * m_pAlgo->IMG_W_ROLL / m_pAlgo->ZOOM_W_ROLL;
					y4 = (m_pAlgo->m_segment_arr.Segment[i].P4_Y-m_pAlgo->ENLARGESIZE_ZOOM_H_ROLL) * m_pAlgo->IMG_H_ROLL / m_pAlgo->ZOOM_H_ROLL;

					m_propertyInfo.segmentPositionArray[i].x1 = x1;
					m_propertyInfo.segmentPositionArray[i].x2 = x2;
					m_propertyInfo.segmentPositionArray[i].x3 = x3;
					m_propertyInfo.segmentPositionArray[i].x4 = x4;
					m_propertyInfo.segmentPositionArray[i].y1 = (m_pAlgo->IMG_H_ROLL - y1);
					m_propertyInfo.segmentPositionArray[i].y2 = (m_pAlgo->IMG_H_ROLL - y2);
					m_propertyInfo.segmentPositionArray[i].y3 = (m_pAlgo->IMG_H_ROLL - y3);
					m_propertyInfo.segmentPositionArray[i].y4 = (m_pAlgo->IMG_H_ROLL - y4);
				}
				else
				{
					m_propertyInfo.segmentArray[i].Buffer = (BYTE*)&m_pAlgo->m_OutSplitResultArrayEx[i*m_pAlgo->IMG_SIZE];//m_clbkProperty.imageInfo.Width*m_clbkProperty.imageInfo.Height];

					x1 = (m_pAlgo->m_segment_arr.Segment[i].P1_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
					y1 = (m_pAlgo->m_segment_arr.Segment[i].P1_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

					x2 = (m_pAlgo->m_segment_arr.Segment[i].P2_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
					y2 = (m_pAlgo->m_segment_arr.Segment[i].P2_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

					x3 = (m_pAlgo->m_segment_arr.Segment[i].P3_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
					y3 = (m_pAlgo->m_segment_arr.Segment[i].P3_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

					x4 = (m_pAlgo->m_segment_arr.Segment[i].P4_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
					y4 = (m_pAlgo->m_segment_arr.Segment[i].P4_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

				
					m_propertyInfo.segmentPositionArray[i].x1 = x1;
					m_propertyInfo.segmentPositionArray[i].x2 = x2;
					m_propertyInfo.segmentPositionArray[i].x3 = x3;
					m_propertyInfo.segmentPositionArray[i].x4 = x4;
					m_propertyInfo.segmentPositionArray[i].y1 = (m_pAlgo->IMG_H - y1);
					m_propertyInfo.segmentPositionArray[i].y2 = (m_pAlgo->IMG_H - y2);
					m_propertyInfo.segmentPositionArray[i].y3 = (m_pAlgo->IMG_H - y3);
					m_propertyInfo.segmentPositionArray[i].y4 = (m_pAlgo->IMG_H - y4);
				}
			}
			m_clbkProperty.pSegmentArray = &m_propertyInfo.segmentArray[0];
			m_clbkProperty.segmentArrayCount = m_clbkProperty.nDetectedFingerCount;

			m_clbkProperty.pSegmentPositionArray = &m_propertyInfo.segmentPositionArray[0];

			// For blocking functions (IBSU_BGetImage(), IBSU_BGetInitProgress(), IBSU_BGetClearPlatenAtCapture())
			{
				m_blockProperty.imageType				= m_propertyInfo.ImageType;
				m_blockProperty.pSplitImageArray		= m_clbkProperty.pSplitImageArray;
				m_blockProperty.splitImageArrayCount	= m_clbkProperty.splitImageArrayCount;
				m_blockProperty.nFingerState			= m_clbkProperty.nFingerState;
				memcpy(&m_blockProperty.imageInfo, &m_clbkProperty.imageInfo, sizeof(IBSU_ImageData));
				memcpy(m_blockProperty.qualityArray, m_clbkProperty.qualityArray, sizeof(m_blockProperty.qualityArray));
				m_blockProperty.bGetOneFrameImage		= TRUE;

				m_blockProperty.bFillResultImage		= TRUE;

				m_blockProperty.nResultImageStatus      = m_clbkProperty.nResultImageStatus;
				m_blockProperty.nDetectedFingerCount    = m_clbkProperty.nDetectedFingerCount;
				m_blockProperty.pSegmentArray           = m_clbkProperty.pSegmentArray;
				m_blockProperty.segmentArrayCount       = m_clbkProperty.segmentArrayCount;
				m_blockProperty.pSegmentPositionArray	= m_clbkProperty.pSegmentPositionArray;
			}
			break;
        case CALLBACK_NOTIFY_MESSAGE:
            m_clbkProperty.warningCode = warningCode;
            break;
		case CALLBACK_KEYBUTTON:
			m_clbkProperty.nButtonIndex = warningCode;
            break;
        case CALLBACK_DEVICE_COUNT:
        case CALLBACK_FINGER_COUNT:
//        case CALLBACK_FINGER_QUALITY:
        case CALLBACK_TAKING_ACQUISITION:
        case CALLBACK_COMPLETE_ACQUISITION:
        case CALLBACK_CLEAR_PLATEN:
            break;
#if defined(__IBSCAN_SDK__)
    case CALLBACK_ONE_FRAME_IMAGE:
        m_clbkProperty.oneFrameImageInfo.FrameTime = frameTime;
        m_clbkProperty.oneFrameImageInfo.BitsPerPixel = 8;
        m_clbkProperty.oneFrameImageInfo.Format = IMG_FORMAT_GRAY;
        m_clbkProperty.oneFrameImageInfo.Height = m_UsbDeviceInfo.CisImgHeight;
        m_clbkProperty.oneFrameImageInfo.Width = m_UsbDeviceInfo.CisImgWidth;
        m_clbkProperty.oneFrameImageInfo.ResolutionX = m_UsbDeviceInfo.baseResolutionX*m_UsbDeviceInfo.CisImgWidth/m_UsbDeviceInfo.ImgWidth;
        m_clbkProperty.oneFrameImageInfo.ResolutionY = m_UsbDeviceInfo.baseResolutionY*m_UsbDeviceInfo.CisImgHeight/m_UsbDeviceInfo.ImgHeight;
        m_clbkProperty.oneFrameImageInfo.Pitch = 1;
        m_clbkProperty.oneFrameImageInfo.IsFinal = bIsFinal;
        memcpy(m_pAlgo->m_capture_rolled_buffer, OutImg, m_UsbDeviceInfo.CisImgSize);
        m_clbkProperty.oneFrameImageInfo.Buffer = (BYTE*)m_pAlgo->m_capture_rolled_buffer;
        break;
    case CALLBACK_TAKE_PREVIEW_IMAGE:
        if(m_propertyInfo.bEnableFastFrameMode && m_propertyInfo.bRawCaptureStart)
        {
            m_clbkProperty.takePreviewImageInfo.FrameTime = frameTime + m_clbkProperty.oneFrameImageInfo.FrameTime;
            m_clbkProperty.imageInfo.FrameTime = m_clbkProperty.takePreviewImageInfo.FrameTime;
            m_clbkProperty.takePreviewImageInfo.BitsPerPixel = 8;
            m_clbkProperty.takePreviewImageInfo.Format = IMG_FORMAT_GRAY;
            m_clbkProperty.takePreviewImageInfo.Height = m_UsbDeviceInfo.ImgHeight/2;
            m_clbkProperty.takePreviewImageInfo.Width = m_UsbDeviceInfo.ImgWidth/2;
            m_clbkProperty.takePreviewImageInfo.ResolutionX = m_UsbDeviceInfo.baseResolutionX/2;
            m_clbkProperty.takePreviewImageInfo.ResolutionY = m_UsbDeviceInfo.baseResolutionY/2;
            m_clbkProperty.takePreviewImageInfo.Pitch = 1;
            m_clbkProperty.takePreviewImageInfo.IsFinal = FALSE;
            memcpy(m_pAlgo->m_OutResultImg, OutImg, m_UsbDeviceInfo.ImgSize/4);
            m_clbkProperty.takePreviewImageInfo.Buffer = (BYTE*)m_pAlgo->m_OutResultImg;
        }
        else
        {
            m_clbkProperty.takePreviewImageInfo.FrameTime = frameTime + m_clbkProperty.oneFrameImageInfo.FrameTime;
            m_clbkProperty.imageInfo.FrameTime = m_clbkProperty.takePreviewImageInfo.FrameTime;
            m_clbkProperty.takePreviewImageInfo.BitsPerPixel = 8;
            m_clbkProperty.takePreviewImageInfo.Format = IMG_FORMAT_GRAY;
            m_clbkProperty.takePreviewImageInfo.Height = m_UsbDeviceInfo.ImgHeight;
            m_clbkProperty.takePreviewImageInfo.Width = m_UsbDeviceInfo.ImgWidth;
            m_clbkProperty.takePreviewImageInfo.ResolutionX = m_UsbDeviceInfo.baseResolutionX;
            m_clbkProperty.takePreviewImageInfo.ResolutionY = m_UsbDeviceInfo.baseResolutionY;
            m_clbkProperty.takePreviewImageInfo.Pitch = 1;
            m_clbkProperty.takePreviewImageInfo.IsFinal = FALSE;
            memcpy(m_pAlgo->m_OutResultImg, OutImg, m_UsbDeviceInfo.ImgSize);
            m_clbkProperty.takePreviewImageInfo.Buffer = (BYTE*)m_pAlgo->m_OutResultImg;
        }
        break;
    case CALLBACK_TAKE_RESULT_IMAGE:
        m_clbkProperty.takeResultImageInfo.FrameTime = frameTime + m_clbkProperty.oneFrameImageInfo.FrameTime;
        m_clbkProperty.takeResultImageInfo.BitsPerPixel = 8;
        m_clbkProperty.takeResultImageInfo.Format = IMG_FORMAT_GRAY;
        m_clbkProperty.takeResultImageInfo.Height = m_UsbDeviceInfo.ImgHeight;
        m_clbkProperty.takeResultImageInfo.Width = m_UsbDeviceInfo.ImgWidth;
        m_clbkProperty.takeResultImageInfo.ResolutionX = m_UsbDeviceInfo.baseResolutionX;
        m_clbkProperty.takeResultImageInfo.ResolutionY = m_UsbDeviceInfo.baseResolutionY;
        m_clbkProperty.takeResultImageInfo.Pitch = 1;
        m_clbkProperty.takeResultImageInfo.IsFinal = TRUE;
        memcpy(m_pAlgo->m_OutResultImg, OutImg, m_UsbDeviceInfo.ImgSize);
        m_clbkProperty.takeResultImageInfo.Buffer = (BYTE*)m_pAlgo->m_OutResultImg;
        break;
#endif
        default:
            return;
    }

    if (m_pDlgUsbManager->m_bIsReadySend[clbkIndex])
    {
        if (m_clbkParam[clbkIndex].callback)
        {
            m_pDlgUsbManager->m_bIsReadySend[clbkIndex] = FALSE;
            m_clbkParam[clbkIndex].deviceHandle = GetDeviceHandle();

            m_pDlg->SubmitJob((LPVOID)&m_clbkParam[clbkIndex]);
        }
    }
}

void CMainCapture::_PostTraceLogCallback(int error, const char *format, ...)
{
    CThreadSync Sync;
    int			waitCount = 0;

    while (TRUE)
    {
        if (m_pDlgUsbManager->m_bIsReadySend[CALLBACK_TRACE_LOG])
        {
            break;
        }

        Sleep(5);
        if (waitCount++ > 10)
        {
            break;
        }
    }

    if (m_pDlgUsbManager->m_bIsReadySend[CALLBACK_TRACE_LOG])
    {
        m_pDlgUsbManager->m_bIsReadySend[CALLBACK_TRACE_LOG] = FALSE;
        m_clbkParam[CALLBACK_TRACE_LOG].deviceHandle = GetDeviceHandle();
        m_clbkParam[CALLBACK_TRACE_LOG].callback = NULL;			// Just add dummy

        sprintf(m_clbkParam[CALLBACK_TRACE_LOG].traceLog.category, "IBSU_Device_%s(%s)", m_UsbDeviceInfo.productName, m_UsbDeviceInfo.serialNumber);
        m_clbkParam[CALLBACK_TRACE_LOG].traceLog.error = error;
        m_clbkParam[CALLBACK_TRACE_LOG].traceLog.tid = _GetThreadID();
        va_list ap;  // for variable args
        va_start(ap, format); // init specifying last non-var arg
        if (error < 0)
        {
            // error occurs
            vsprintf(m_clbkParam[CALLBACK_TRACE_LOG].traceLog.message, format, ap);
        }
        else
        {
            m_clbkParam[CALLBACK_TRACE_LOG].traceLog.message[0] = '\t';
            vsprintf(&m_clbkParam[CALLBACK_TRACE_LOG].traceLog.message[1], format, ap);
        }
        va_end(ap); // end var args

        m_pDlg->SubmitJob((LPVOID)&m_clbkParam[CALLBACK_TRACE_LOG]);
    }
}

#ifdef __USE_DEBUG_WITH_BITMAP__
#ifdef _WINDOWS
int CMainCapture::SaveBitmapImage(char *filename, unsigned char *buffer, int width, int height, int invert)
{
    //	return FALSE;

    FILE *fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        return FALSE;
    }

    int i;
    IB_BITMAPFILEHEADER header;
    IB_BITMAPINFO *Info = (IB_BITMAPINFO *)new unsigned char [1064];

    header.bfOffBits = 1078;
    header.bfReserved1 = 0;
    header.bfReserved2 = 0;
    header.bfSize = 1078 + width * height;
    header.bfType = (unsigned short)(('M' << 8) | 'B');

    for (i = 0; i < 256; i++)
    {
        Info->bmiColors[i].rgbBlue = i;
        Info->bmiColors[i].rgbRed = i;
        Info->bmiColors[i].rgbGreen = i;
        Info->bmiColors[i].rgbReserved = 0;
    }
    Info->bmiHeader.biBitCount = 8;
    Info->bmiHeader.biClrImportant = 0;
    Info->bmiHeader.biClrUsed = 0;
    Info->bmiHeader.biCompression = BI_RGB;
    Info->bmiHeader.biHeight = height;
    Info->bmiHeader.biPlanes = 1;
    Info->bmiHeader.biSize = 40;
    Info->bmiHeader.biSizeImage = width * height;
    Info->bmiHeader.biWidth = width;
    Info->bmiHeader.biXPelsPerMeter = 0;
    Info->bmiHeader.biYPelsPerMeter = 0;

    fwrite(&header, 1, 14, fp);
    fwrite(Info, 1, 1064, fp);

    if (invert == TRUE)
    {
        unsigned char *inv_buffer = new unsigned char [width * height];
        for (i = 0; i < width * height; i++)
        {
            inv_buffer[i] = 255 - buffer[i];
        }
        fwrite(inv_buffer, 1, width * height, fp);
        delete [] inv_buffer;
    }
    else
    {
        fwrite(buffer, 1, width * height, fp);
    }

    fclose(fp);

    delete Info;

    return TRUE;
}
#endif
#endif

int CMainCapture::_CheckForClearPlaten(BYTE *InImg)
{
    int			nRc = 0;

    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
        case DEVICE_TYPE_WATSON_MINI:
            nRc = _GoJob_DummyCapture_ForThread_Watson(InImg);
            break;
        case DEVICE_TYPE_SHERLOCK_ROIC:
        case DEVICE_TYPE_SHERLOCK:
            nRc = _GoJob_DummyCapture_ForThread_Sherlock(InImg);
            break;
        case DEVICE_TYPE_COLUMBO:
            nRc = _GoJob_DummyCapture_ForThread_Columbo(InImg);
            break;
        case DEVICE_TYPE_CURVE:
			nRc = _GoJob_DummyCapture_ForThread_Curve(InImg);
            break;
		case DEVICE_TYPE_CURVE_SETI:
			nRc = _GoJob_DummyCapture_ForThread_CurveSETi(InImg);
            break;
        case DEVICE_TYPE_HOLMES:
            nRc = _GoJob_DummyCapture_ForThread_Holmes(InImg);
            break;
		case DEVICE_TYPE_KOJAK:
#ifdef __DUMMY_DETECT_FINGER_USING_ATOF__
			if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
				nRc = _GoJob_DummyCapture_ForThread_Kojak_ATOF(InImg);
			else
#endif
				nRc = _GoJob_DummyCapture_ForThread_Kojak(InImg);
            break;
		case DEVICE_TYPE_FIVE0:
            nRc = _GoJob_DummyCapture_ForThread_Five0(InImg);
            break;
        default:
            break;
    }

    return nRc;
}

int CMainCapture::_GoJob_DummyCapture_ForThread(BYTE *InImg)
{
    struct 		timeval finish_tv, start_tv;
    double		elapsed;
    int			nRc = 0;
	int			tmpCIS_IMG_SIZE = m_UsbDeviceInfo.CisImgSize;

    gettimeofday(&start_tv, NULL);

	if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK )
			tmpCIS_IMG_SIZE = m_UsbDeviceInfo.CisImgSize_Roll;
	}

    if (!_GetOneFrameImage(InImg, tmpCIS_IMG_SIZE))
    {
        return -1;
    }

    if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK &&
		m_bCaptureThread_StopMessage == FALSE)
    {
		_ApplyCaptureSettings(m_FullCaptureSetting);
    }

    nRc = _CheckForClearPlaten(InImg);

    if (m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
    {
		_Set_CIS_GainRegister(m_propertyInfo.nContrastValue, FALSE);
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
	_DrawClientWindow(m_pAlgo->m_OutResultImg, m_DisplayWindow, &dstOverlayText, &dstOverlayTextEx, &dstOverlayLineEx, &dstOverlayQuadrangleEx, &dstOverlayShapeEx);

    gettimeofday(&finish_tv, NULL);
    elapsed = (finish_tv.tv_sec - start_tv.tv_sec) + (finish_tv.tv_usec - start_tv.tv_usec) / 1000000.0;

    int gab = m_UsbDeviceInfo.nMIN_Elapsed - (int)(elapsed * 1000);
    if (gab > 0)
    {
        Sleep(gab);
    }

    if (nRc)
    {
        return 1;
    }

    return 0;
}

AcuisitionState CMainCapture::_GoJob_PreImageProcessing_ForOneFrame(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage)
{
    double		elapsed;

    *bIsGoodImage = FALSE;

    if (!_GetOneFrameImage(InImg, m_UsbDeviceInfo.CisImgSize))
    {
        return ACUISITION_ERROR;
    }

    if (m_bPowerResume)
    {
        _PostTraceLogCallback(0, "==================> m_bPowerResume is TRUE");
        //        _SndUsbFwCaptureStop();
        //        _SndUsbFwCaptureStart();
        //        _InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
        //        _CaptureStart_for_Decimation();
        Capture_SetLEVoltage(m_propertyInfo.nVoltageValue);
        _SetLEOperationMode((WORD)m_propertyInfo.nLEOperationMode);
        _SetActiveLEDs(0, m_propertyInfo.dwActiveLEDs);
        m_bPowerResume = FALSE;
    }

#ifdef __USE_DEBUG_WITH_BITMAP__
    SaveBitmapImage("c:\\OneFrameImage.bmp", InImg, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight, FALSE);
#endif
    // If mode is AsyncOpenDevice, waiting for m_clbkParam[CALLBACK_ASYNC_OPEN_DEVICE].bIsReadySend to TRUE
    if (!m_pDlgUsbManager->m_bIsReadySend[CALLBACK_ASYNC_OPEN_DEVICE])
    {
        gettimeofday(&m_finish_tv, NULL);
        elapsed = (m_finish_tv.tv_sec - m_start_tv.tv_sec) + (m_finish_tv.tv_usec - m_start_tv.tv_usec) / 1000000.0;

        int gab = m_UsbDeviceInfo.nMIN_Elapsed - (int)(elapsed * 1000);
        if (gab > 0)
        {
            Sleep(gab);
        }

        return ACUISITION_NOT_COMPLETE;
    }

    if (m_nFrameCount == 0)
    {
        _PostCallback(CALLBACK_CLEAR_PLATEN);

        if (m_clbkProperty.nPlatenState == ENUM_IBSU_PLATEN_HAS_FINGERS)
        {
            m_bFirstSent_clbkClearPlaten = TRUE;
        }
    }

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    switch (m_UsbDeviceInfo.devType)
    {
    case DEVICE_TYPE_WATSON:
    case DEVICE_TYPE_WATSON_MINI:
        if (_GoJob_PreImageProcessing_Watson(InImg, m_pAlgo->m_ImgOnProcessing, bIsGoodImage) == ACUISITION_NOT_COMPLETE)
        {
            return ACUISITION_NOT_COMPLETE;
        }
        break;
    case DEVICE_TYPE_SHERLOCK_ROIC:
    case DEVICE_TYPE_SHERLOCK:
        if (_GoJob_PreImageProcessing_Sherlock(InImg, m_pAlgo->m_ImgOnProcessing, bIsGoodImage) == ACUISITION_NOT_COMPLETE)
        {
            return ACUISITION_NOT_COMPLETE;
        }
        break;
    case DEVICE_TYPE_COLUMBO:
        if (_GoJob_PreImageProcessing_Columbo(InImg, m_pAlgo->m_ImgOnProcessing, bIsGoodImage) == ACUISITION_NOT_COMPLETE)
        {
            return ACUISITION_NOT_COMPLETE;
        }
        break;
    case DEVICE_TYPE_CURVE:
        if (_GoJob_PreImageProcessing_Curve(InImg, m_pAlgo->m_ImgOnProcessing, bIsGoodImage) == ACUISITION_NOT_COMPLETE)
        {
            return ACUISITION_NOT_COMPLETE;
        }
        break;
    case DEVICE_TYPE_CURVE_SETI:
        if( _GoJob_PreImageProcessing_CurveSETi(InImg, m_pAlgo->m_ImgOnProcessing, bIsGoodImage) == ACUISITION_NOT_COMPLETE )
        {
            return ACUISITION_NOT_COMPLETE;
        }
        break;
    case DEVICE_TYPE_HOLMES:
        if (_GoJob_PreImageProcessing_Holmes(InImg, m_pAlgo->m_ImgOnProcessing, bIsGoodImage) == ACUISITION_NOT_COMPLETE)
        {
            return ACUISITION_NOT_COMPLETE;
        }
        break;
    case DEVICE_TYPE_KOJAK:
		if(m_UsbDeviceInfo.bCanUseTOF && m_propertyInfo.bEnableTOF &&
			( (m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER) || 
			  (m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_propertyInfo.bEnableTOFforROLL) ) )
		{
			if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
			{
				if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
				{
					if (_GoJob_PreImageProcessing_Kojak_CurrentSensor(InImg, m_pAlgo->m_ImgOnProcessing, bIsGoodImage) == ACUISITION_NOT_COMPLETE)
					{
						return ACUISITION_NOT_COMPLETE;
					}
				}
				else
				{
					if (_GoJob_PreImageProcessing_Kojak_CurrentSensor_and_ATOF(InImg, m_pAlgo->m_ImgOnProcessing, bIsGoodImage) == ACUISITION_NOT_COMPLETE)
					{
						return ACUISITION_NOT_COMPLETE;
					}
				}
			}
			else
			{
				if (_GoJob_PreImageProcessing_Kojak_CurrentSensor(InImg, m_pAlgo->m_ImgOnProcessing, bIsGoodImage) == ACUISITION_NOT_COMPLETE)
				{
					return ACUISITION_NOT_COMPLETE;
				}
			}
		}
		else
		{
			if (_GoJob_PreImageProcessing_Kojak(InImg, m_pAlgo->m_ImgOnProcessing, bIsGoodImage) == ACUISITION_NOT_COMPLETE)
			{
				return ACUISITION_NOT_COMPLETE;
			}
		}
        break;
    case DEVICE_TYPE_FIVE0:
        if (_GoJob_PreImageProcessing_Five0(InImg, m_pAlgo->m_ImgOnProcessing, bIsGoodImage) == ACUISITION_NOT_COMPLETE)
        {
            return ACUISITION_NOT_COMPLETE;
        }
        break;
    default:
        break;
    }

    if (m_propertyInfo.bEnableCaptureOnRelease)
    {
        // Update BestFrame
        m_pAlgo->_Algo_UpdateBestFrame(OutImg, m_pAlgo->m_cImgAnalysis);
        if (m_pAlgo->m_cImgAnalysis.finger_count > 0)
        {
            m_bFirstPutFingerOnSensor = TRUE;
        }
    }

    // enzyme add 2012-11-29 it should be set to FALSE when your finger is not clear on the sensor platen
    if (m_bFirstSent_clbkClearPlaten)
    {
        *bIsGoodImage = FALSE;
    }
#ifdef __G_PERFORMANCE_DEBUG__
    gettimeofday(&m_pd_finish_tv, NULL);
    m_pd_Elapsed = (m_pd_finish_tv.tv_sec - m_pd_start_tv.tv_sec) + (m_pd_finish_tv.tv_usec - m_pd_start_tv.tv_usec) / 1000000.0;
    _PostTraceLogCallback(0, "<===> PreImageProceesing() = %.1f ms, segment count = %d", m_pd_Elapsed*1000, m_pAlgo->m_cImgAnalysis.finger_count);
    gettimeofday(&m_pd_start_tv, NULL);
#endif

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    return ACUISITION_NONE;
}

AcuisitionState CMainCapture::_GoJob_PreviewImage_ForThread(BYTE *InImg)
{
    BYTE		*OutImg = m_pAlgo->m_Inter_Img4;//InImg;
    double		elapsed;
    BOOL		bIsFinal = FALSE;
    BOOL		bIsGoodImage = FALSE;
    AcuisitionState nRc;

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    if (_ClearDoubleBufferedImageOnFPGA(InImg, m_UsbDeviceInfo.nMIN_Elapsed) == ACUISITION_NOT_COMPLETE)
    {
        return ACUISITION_NOT_COMPLETE;
    }

    gettimeofday(&m_start_tv, NULL);
    nRc = _GoJob_PreImageProcessing_ForOneFrame(InImg, m_pAlgo->m_ImgOnProcessing, &bIsGoodImage);
    if (nRc != ACUISITION_NONE)
    {
        return nRc;
    }

    bIsFinal = _GoJob_PostImageProcessing(m_pAlgo->m_ImgOnProcessing, OutImg, bIsGoodImage);

	if( bIsFinal == TRUE &&
		m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK &&
		m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
	{
		m_ATOF_ON_Count = 3;
		m_ATOF_OFF_Count = 0;
		m_pAlgo->m_isEnabledATOF = TRUE;
	}

#ifdef __G_PERFORMANCE_DEBUG__
    gettimeofday(&m_pd_finish_tv, NULL);
    m_pd_Elapsed = (m_pd_finish_tv.tv_sec - m_pd_start_tv.tv_sec) + (m_pd_finish_tv.tv_usec - m_pd_start_tv.tv_usec) / 1000000.0;
	_PostTraceLogCallback(0, "<===> PostImageProceesing() = %.1f ms, bIsFinal = %d", m_pd_Elapsed*1000, bIsFinal);
#endif

#ifdef __G_PERFORMANCE_DEBUG__
    gettimeofday(&m_finish_tv, NULL);
    m_pd_Elapsed = (m_finish_tv.tv_sec - m_start_tv.tv_sec) + (m_finish_tv.tv_usec - m_start_tv.tv_usec) / 1000000.0;
	_PostTraceLogCallback(0, "<===> Total processing time = %.1f ms", m_pd_Elapsed*1000);
#endif

#if defined(__G_PERFORMANCE_DEBUG__) && defined(WINCE)
    RETAILMSG(1, (TEXT("<====> mean=%03d, voltage=%d, contrast=%d, decimation=%d, decimationMode=%d, frame(count=%02d, delay=%02d, good_count=%02d), LE mode=%d, noise_histo=%02d, final=%d, same_count=%d, same_gain_time=%d, Fingercnt=%d, gain=%d, x=%04d, y=%04d, foreCNT=%d, bIsGoodImage=%d\r\n"),
          m_pAlgo->m_cImgAnalysis.mean, m_propertyInfo.nVoltageValue, m_propertyInfo.nContrastValue,
          m_UsbDeviceInfo.bDecimation, m_UsbDeviceInfo.nDecimation_Mode,
          m_pAlgo->m_cImgAnalysis.frame_count, m_pAlgo->m_cImgAnalysis.frame_delay,
          m_pAlgo->m_cImgAnalysis.good_frame_count, m_propertyInfo.nLEOperationMode,
          m_pAlgo->m_cImgAnalysis.noise_histo, bIsFinal, m_pAlgo->m_cImgAnalysis.same_gain_count,
          m_pAlgo->m_cImgAnalysis.same_gain_time, m_pAlgo->m_cImgAnalysis.finger_count,
          m_propertyInfo.nContrastValue, m_pAlgo->m_cImgAnalysis.center_x,
          m_pAlgo->m_cImgAnalysis.center_y, m_pAlgo->m_cImgAnalysis.foreground_count,
          bIsGoodImage));
#endif

#ifdef __G_DEBUG__
	if(m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
	{
		if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		TRACE(_T("<====> mean=%03d(%03d), voltage=%X, contrast=%X, decimation=%d, decimationMode=%d, frame(count=%02d, delay=%02d, good_count=%02d), ") \
          _T("LE mode=%d, noise_histo=%02d, final=%d, same_count=%d, same_gain_time=%d, Fingercnt=%d, ") \
          _T("gain=%d, x=%04d, y=%04d, foreCNT=%d, bIsGoodImage=%d\n"),
		  m_pAlgo->m_cImgAnalysis.mean, m_pAlgo->m_SBDAlg->m_SegmentBright, m_FullCaptureSetting.CurrentDAC, m_FullCaptureSetting.CurrentGain, //m_propertyInfo.nVoltageValue, m_propertyInfo.nContrastValue,
          m_UsbDeviceInfo.bDecimation, m_UsbDeviceInfo.nDecimation_Mode,
          m_pAlgo->m_cImgAnalysis.frame_count, m_pAlgo->m_cImgAnalysis.frame_delay,
          m_pAlgo->m_cImgAnalysis.good_frame_count, m_propertyInfo.nLEOperationMode,
          m_pAlgo->m_cImgAnalysis.noise_histo, bIsFinal, m_pAlgo->m_cImgAnalysis.same_gain_count,
          m_pAlgo->m_cImgAnalysis.same_gain_time, m_pAlgo->m_cImgAnalysis.finger_count,
          m_propertyInfo.nContrastValue, m_pAlgo->m_cImgAnalysis.center_x,
          m_pAlgo->m_cImgAnalysis.center_y, m_pAlgo->m_cImgAnalysis.foreground_count,
          bIsGoodImage);
		else
			TRACE(_T("<====> mean=%03d(%03d), voltage=%X, contrast=%X, decimation=%d, decimationMode=%d, frame(count=%02d, delay=%02d, good_count=%02d), ") \
          _T("LE mode=%d, noise_histo=%02d, final=%d, same_count=%d, same_gain_time=%d, Fingercnt=%d, ") \
          _T("gain=%d, x=%04d, y=%04d, foreCNT=%d(%s), DryMode(%d), bIsGoodImage=%d\n"),
		  m_pAlgo->m_cImgAnalysis.mean, m_pAlgo->m_SBDAlg->m_SegmentBright, m_DeciCaptureSetting.CurrentDAC, m_DeciCaptureSetting.CurrentGain, //m_propertyInfo.nVoltageValue, m_propertyInfo.nContrastValue,
          m_UsbDeviceInfo.bDecimation, m_UsbDeviceInfo.nDecimation_Mode,
          m_pAlgo->m_cImgAnalysis.frame_count, m_pAlgo->m_cImgAnalysis.frame_delay,
          m_pAlgo->m_cImgAnalysis.good_frame_count, m_propertyInfo.nLEOperationMode,
          m_pAlgo->m_cImgAnalysis.noise_histo, bIsFinal, m_pAlgo->m_cImgAnalysis.same_gain_count,
          m_pAlgo->m_cImgAnalysis.same_gain_time, m_pAlgo->m_cImgAnalysis.finger_count,
          m_propertyInfo.nContrastValue, m_pAlgo->m_cImgAnalysis.center_x,
          m_pAlgo->m_cImgAnalysis.center_y, m_pAlgo->m_cImgAnalysis.foreground_count,
		  (m_pAlgo->m_cImgAnalysis.isDetected==1)?"Detected":"NoDetect",
		  (m_pAlgo->m_cImgAnalysis.isChangetoSuperDry>=3)?1:0,
         bIsGoodImage);
	}
	else if(m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0)
	{
		if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		TRACE(_T("<====> brightPixelRate=%d, foreCNT=%d, mean=%03d(%03d), voltage=%X, contrast=%X, decimation=%d, decimationMode=%d, frame(count=%02d, delay=%02d, good_count=%02d), ") \
          _T("LE mode=%d, noise_histo=%02d, final=%d, same_count=%d, same_gain_time=%d, Fingercnt=%d, ") \
          _T("gain=%d, x=%04d, y=%04d, bIsGoodImage=%d\n"),
          (m_pAlgo->m_cImgAnalysis.foreground_count2==0)? 0: (int)(m_pAlgo->m_cImgAnalysis.bright_pixel_count*100)/(m_pAlgo->m_cImgAnalysis.foreground_count2),
          m_pAlgo->m_cImgAnalysis.foreground_count2, m_pAlgo->m_cImgAnalysis.mean,
          m_pAlgo->m_SBDAlg->m_SegmentBright, m_FullCaptureSetting.CurrentDAC, m_FullCaptureSetting.CurrentGain, //m_propertyInfo.nVoltageValue, m_propertyInfo.nContrastValue,
          m_pAlgo->m_cImgAnalysis.frame_count, m_pAlgo->m_cImgAnalysis.frame_delay,
          m_pAlgo->m_cImgAnalysis.good_frame_count, m_propertyInfo.nLEOperationMode,
          m_pAlgo->m_cImgAnalysis.noise_histo, bIsFinal, m_pAlgo->m_cImgAnalysis.same_gain_count,
          m_pAlgo->m_cImgAnalysis.same_gain_time, m_pAlgo->m_cImgAnalysis.finger_count,
          m_propertyInfo.nContrastValue, m_pAlgo->m_cImgAnalysis.center_x,
          m_pAlgo->m_cImgAnalysis.center_y, 
          bIsGoodImage);
		else
			TRACE(_T("<====> brightPixelRate=%d, foreCNT=%d, mean=%03d(%03d), voltage=%X, contrast=%X, frame(count=%02d, delay=%02d, good_count=%02d), ") \
          _T("LE mode=%d, noise_histo=%02d, final=%d, same_count=%d, same_gain_time=%d, Fingercnt=%d, ") \
          _T("gain=%d, x=%04d, y=%04d, DryMode(%d), bIsGoodImage=%d\n"),
          (m_pAlgo->m_cImgAnalysis.foreground_count2==0)? 0: (int)(m_pAlgo->m_cImgAnalysis.bright_pixel_count*100)/(m_pAlgo->m_cImgAnalysis.foreground_count2),
		  m_pAlgo->m_cImgAnalysis.foreground_count2, m_pAlgo->m_cImgAnalysis.mean,
          m_pAlgo->m_SBDAlg->m_SegmentBright, m_FullCaptureSetting.CurrentDAC, m_DeciCaptureSetting.CurrentGain, //m_propertyInfo.nVoltageValue, m_propertyInfo.nContrastValue,
          m_pAlgo->m_cImgAnalysis.frame_count, m_pAlgo->m_cImgAnalysis.frame_delay,
          m_pAlgo->m_cImgAnalysis.good_frame_count, m_propertyInfo.nLEOperationMode,
          m_pAlgo->m_cImgAnalysis.noise_histo, bIsFinal, m_pAlgo->m_cImgAnalysis.same_gain_count,
          m_pAlgo->m_cImgAnalysis.same_gain_time, m_pAlgo->m_cImgAnalysis.finger_count,
          m_propertyInfo.nContrastValue, m_pAlgo->m_cImgAnalysis.center_x,
          m_pAlgo->m_cImgAnalysis.center_y, 
		  (m_pAlgo->m_cImgAnalysis.isDetected==1)?"Detected":"NoDetect",
		  (m_pAlgo->m_cImgAnalysis.isChangetoSuperDry>=3)?1:0,
         bIsGoodImage);
	}
	else
		TRACE(_T("<====> foreCNT=%d, mean=%03d, exporue=%X, voltage=%X, contrast=%X, frame(count=%02d, delay=%02d, good_count=%02d), ") \
          _T("LE mode=%d, noise_histo=%02d, final=%d, same_count=%d, same_gain_time=%d, Fingercnt=%d, ") \
          _T("gain=%d, x=%04d, y=%04d, bIsGoodImage=%d\n"),
          m_pAlgo->m_cImgAnalysis.foreground_count, m_pAlgo->m_cImgAnalysis.mean, m_propertyInfo.nExposureValue, m_propertyInfo.nVoltageValue, m_propertyInfo.nContrastValue,
          m_UsbDeviceInfo.bDecimation, m_UsbDeviceInfo.nDecimation_Mode,
          m_pAlgo->m_cImgAnalysis.frame_count, m_pAlgo->m_cImgAnalysis.frame_delay,
          m_pAlgo->m_cImgAnalysis.good_frame_count, m_propertyInfo.nLEOperationMode,
          m_pAlgo->m_cImgAnalysis.noise_histo, bIsFinal, m_pAlgo->m_cImgAnalysis.same_gain_count,
          m_pAlgo->m_cImgAnalysis.same_gain_time, m_pAlgo->m_cImgAnalysis.finger_count,
          m_propertyInfo.nContrastValue, m_pAlgo->m_cImgAnalysis.center_x,
          m_pAlgo->m_cImgAnalysis.center_y, 
          (m_pAlgo->m_cImgAnalysis.isDetected==1)?"Detected":"NoDetect",
          bIsGoodImage);

/*    int LEFT = m_pAlgo->m_cImgAnalysis.LEFT * m_UsbDeviceInfo.ImgWidth / m_UsbDeviceInfo.CisImgWidth;
    int RIGHT = m_pAlgo->m_cImgAnalysis.RIGHT * m_UsbDeviceInfo.ImgWidth / m_UsbDeviceInfo.CisImgWidth;
    int TOP = m_pAlgo->m_cImgAnalysis.TOP * m_UsbDeviceInfo.ImgHeight / m_UsbDeviceInfo.CisImgHeight;
    int BOTTOM = m_pAlgo->m_cImgAnalysis.BOTTOM * m_UsbDeviceInfo.ImgHeight / m_UsbDeviceInfo.CisImgHeight;

    for (int j = LEFT; j <= RIGHT; j++)
    {
#ifdef __IBSCAN_ULTIMATE_SDK__
        OutImg[TOP * m_UsbDeviceInfo.ImgWidth + j] = 0;
        OutImg[BOTTOM * m_UsbDeviceInfo.ImgWidth + j] = 0;
#else
        OutImg[TOP * m_UsbDeviceInfo.ImgWidth + j] = 255;
        OutImg[BOTTOM * m_UsbDeviceInfo.ImgWidth + j] = 255;
#endif
    }

    for (int i = TOP; i <= BOTTOM; i++)
    {
#ifdef __IBSCAN_ULTIMATE_SDK__
        OutImg[i * m_UsbDeviceInfo.ImgWidth + LEFT] = 0;
        OutImg[i * m_UsbDeviceInfo.ImgWidth + RIGHT] = 0;
#else
        OutImg[i * m_UsbDeviceInfo.ImgWidth + LEFT] = 255;
        OutImg[i * m_UsbDeviceInfo.ImgWidth + RIGHT] = 255;
#endif
    }
*/
#endif

    gettimeofday(&m_finish_tv, NULL);
    elapsed = (m_finish_tv.tv_sec - m_start_tv.tv_sec) + (m_finish_tv.tv_usec - m_start_tv.tv_usec) / 1000000.0;

    int gab = m_UsbDeviceInfo.nMIN_Elapsed - (int)(elapsed * 1000);
    if (gab > 0)
    {
        Sleep(gab);
        elapsed += (double)gab / 1000;
    }

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
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

	if(m_propertyInfo.bEnableEncryption == TRUE && m_propertyInfo.bEnableViewEncryptionImage == TRUE)
		_DrawClientWindow(m_pAlgo->m_EncryptImage500ppi, m_DisplayWindow, &dstOverlayText, &dstOverlayTextEx, &dstOverlayLineEx, &dstOverlayQuadrangleEx, &dstOverlayShapeEx);
	else
		_DrawClientWindow(OutImg, m_DisplayWindow, &dstOverlayText, &dstOverlayTextEx, &dstOverlayLineEx, &dstOverlayQuadrangleEx, &dstOverlayShapeEx);

#ifdef __G_INIT_CAPTURE_TIME__
    if (m_nFrameCount == 0)
	{
        gettimeofday(&m_initCaptureFinish_tv, NULL);
        double elapsedInitCapture = (m_initCaptureFinish_tv.tv_sec - m_initCaptureStart_tv.tv_sec) + (m_initCaptureFinish_tv.tv_usec - m_initCaptureStart_tv.tv_usec) / 1000000.0;
		_PostTraceLogCallback(0, "Initialized capture time %.1f ms", elapsedInitCapture*1000);
	}
#endif

#if defined(__IBSCAN_ULTIMATE_SDK__)
    // 2013-05-23 enzyme add - To support ENUM_IBSU_PROPERTY_CAPTURE_TIMEOUT
    if( m_propertyInfo.nCaptureTimeout != -1 )
    {
        gettimeofday(&m_finish_tv, NULL);
        m_Elapsed = (m_finish_tv.tv_sec - m_start_tv.tv_sec) + (m_finish_tv.tv_usec - m_start_tv.tv_usec) / 1000000.0;
        m_pAlgo->m_cImgAnalysis.nAccumulated_Captured_Elapsed += (int)(m_Elapsed*1000);
        if( (int)(m_pAlgo->m_cImgAnalysis.nAccumulated_Captured_Elapsed/1000) > m_propertyInfo.nCaptureTimeout )
        {
            // Do not support IBSU_CallbackResultImage callback
            _PostCallback(CALLBACK_RESULT_IMAGE_EX, OutImg, elapsed, 0, 0 , FALSE);
		    return ACUISITION_ABORT;
        }
        gettimeofday(&m_start_tv, NULL);
	}

    // enzyme modify 2012-11-30
    if (bIsFinal)
    {
        _PostCallback(CALLBACK_RESULT_IMAGE, OutImg, elapsed, 0, 0 , TRUE);
        _PostCallback(CALLBACK_RESULT_IMAGE_EX, OutImg, elapsed, 0, 0 , TRUE);
        return ACUISITION_COMPLETED;
    }
#endif

    if (m_nFrameCount++ > 200)
    {
        m_nFrameCount = 200;
    }

#if defined(__IBSCAN_ULTIMATE_SDK__)
    _PostCallback(CALLBACK_PREVIEW_IMAGE, OutImg, elapsed);
#else
    _PostCallback(CALLBACK_PREVIEW_IMAGE, OutImg, elapsed, 0, 0, bIsFinal);
#endif

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    return ACUISITION_NOT_COMPLETE;
}

void CMainCapture::_GoJob_Initialize_ForThread()
{
    _PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, 0);
    m_UsbDeviceInfo.LeVoltageControl = NOT_SUPPORT_LE_VOLTAGE_CONTROL;

    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
        case DEVICE_TYPE_WATSON_MINI:
            _GoJob_Initialize_ForThread_Watson();
            break;
        case DEVICE_TYPE_SHERLOCK_ROIC:
        case DEVICE_TYPE_SHERLOCK:
            _GoJob_Initialize_ForThread_Sherlock();
            break;
        case DEVICE_TYPE_COLUMBO:
            _GoJob_Initialize_ForThread_Columbo();
            break;
        case DEVICE_TYPE_CURVE:
           	_GoJob_Initialize_ForThread_Curve();
            break;
        case DEVICE_TYPE_CURVE_SETI:
			_GoJob_Initialize_ForThread_CurveSETi();
            break;
        case DEVICE_TYPE_HOLMES:
            _GoJob_Initialize_ForThread_Holmes();
            break;
		case DEVICE_TYPE_KOJAK:
            _GoJob_Initialize_ForThread_Kojak();
            break;
		case DEVICE_TYPE_FIVE0:
            _GoJob_Initialize_ForThread_Five0();
            break;
        default:
            break;
    }

    _PostTraceLogCallback(IBSU_STATUS_OK, "Internal Chip ver %X", m_UsbDeviceInfo.nFpgaVersion);
	_PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, 100);
}

BOOL CMainCapture::_GoJob_PostImageProcessing_Default(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage)
{
    //	static int		nGoodFrameCount=0;
    //	static double	timeAfterBeginCaptureImage = 0;
    //	static int		SavedFingerCountStatus = ENUM_IBSU_FINGER_COUNT_OK;
    BOOL			bIsFinal = FALSE;
    BOOL			bCompleteAcquisition = FALSE;
    int				nRc = -1;

    if ((m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CAPTURE) == IBSU_OPTION_AUTO_CAPTURE)
    {
        if (m_clbkProperty.nFingerState == ENUM_IBSU_FINGER_COUNT_OK && bIsGoodImage == TRUE)
        {
            bIsFinal = TRUE;
        }
    }

	/*if (GetTakeResultImageManually())
    {
		if(m_pAlgo->m_cImgAnalysis.finger_count > 0)
			bIsFinal = TRUE;
		else
			SetTakeResultImageManually(FALSE);
    }*/
	if (GetTakeResultImageManually())
    {
		if(m_pAlgo->m_cImgAnalysis.finger_count > 0)
		{
			bIsFinal = TRUE;
			nRc = IBSU_STATUS_OK;

			if (m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER &&
				m_pAlgo->m_rollingStatus < 1)
			{
				SetTakeResultImageManually(FALSE);
				m_pAlgo->m_rollingStatus = 1;

				if(m_nGoodFrameCount <= 1)
					m_pAlgo->NewRoll_Init_Rolling();

				m_nGoodFrameCount = 3;

				_PostCallback(CALLBACK_TAKING_ACQUISITION);
                m_pAlgo->m_rollingStatus = 1;

				_PostImageProcessing_ForPreview(InImg, OutImg, bIsGoodImage);
				return FALSE;
			}
		}
		else
		{
			bIsFinal = FALSE;
			m_pAlgo->m_rollingStatus = 0;
			m_nGoodFrameCount = 0;
			SetTakeResultImageManually(FALSE);
		}
    }

    // For IBSU_OPTION_IGNORE_FINGER_COUNT
    if ((m_propertyInfo.nCaptureOptions & IBSU_OPTION_IGNORE_FINGER_COUNT) == IBSU_OPTION_IGNORE_FINGER_COUNT &&
            (m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CAPTURE) == IBSU_OPTION_AUTO_CAPTURE
       )
    {
        if (m_clbkProperty.nFingerState == ENUM_IBSU_TOO_MANY_FINGERS ||
                m_clbkProperty.nFingerState == ENUM_IBSU_TOO_FEW_FINGERS)
        {
            m_timeAfterBeginCaptureImage += m_clbkProperty.imageInfo.FrameTime;
            if ((int)(m_timeAfterBeginCaptureImage * 1000) > m_propertyInfo.nIgnoreFingerTime)
            {
                bIsFinal = TRUE;
            }
        }
        else if ((m_clbkProperty.nFingerState == ENUM_IBSU_TOO_MANY_FINGERS ||
                  m_clbkProperty.nFingerState == ENUM_IBSU_TOO_FEW_FINGERS) &&
                 (m_SavedFingerCountStatus != m_clbkProperty.nFingerState))
        {
            // Initialize for IBSU_OPTION_IGNORE_FINGER_COUNT
            m_timeAfterBeginCaptureImage = 0;
        }
    }

    m_SavedFingerCountStatus = m_clbkProperty.nFingerState;

    if (bIsFinal && m_bIsActiveCapture)
    {
        m_nGoodFrameCount++;
    }

    if (m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
    {
        if (m_nGoodFrameCount > 1)
        {
            if (m_nGoodFrameCount == 2)
            {
                _PostCallback(CALLBACK_TAKING_ACQUISITION);
                m_pAlgo->m_rollingStatus = 1;
            }

			nRc = m_pAlgo->_PostImageProcessing_ForRolling(InImg, m_nGoodFrameCount, &bCompleteAcquisition);
			if (m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK)
            {
				m_pAlgo->m_bRotatedImage = FALSE;
                m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_ROIC_GetBrightWithRawImage(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
            }
			else
            {
				m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_GetBrightWithRawImage(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
            }

			for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
			{
				if( m_clbkProperty.qualityArray[i] == ENUM_IBSU_QUALITY_INVALID_AREA_LEFT	||
					m_clbkProperty.qualityArray[i] == ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT	||
					m_clbkProperty.qualityArray[i] == ENUM_IBSU_QUALITY_INVALID_AREA_TOP	||
					m_clbkProperty.qualityArray[i] == ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM )
				{
					bCompleteAcquisition = FALSE;
					m_nGoodFrameCount = 0;
					nRc = -1;
				}
			}

			if (GetTakeResultImageManually())
			{
				bCompleteAcquisition = TRUE;
				nRc = IBSU_STATUS_OK;
				memcpy(InImg, m_pAlgo->m_capture_rolled_local_best_buffer, m_pAlgo->CIS_IMG_SIZE_ROLL);
			}

            if (bCompleteAcquisition)
            {
				if(m_pAlgo->m_rollingStatus < 2)
					_PostCallback(CALLBACK_COMPLETE_ACQUISITION);
                m_pAlgo->m_rollingStatus = 2;
            }

			if( nRc == IBSU_STATUS_OK &&
                (m_pDlgUsbManager->m_bIsReadySend[CALLBACK_RESULT_IMAGE] || m_pDlgUsbManager->m_bIsReadySend[CALLBACK_RESULT_IMAGE_EX]) )
			{
				m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(m_pAlgo->m_cImgAnalysis.mean);
				_PostCallback(CALLBACK_FINGER_COUNT);

				// fixed wrong finger quality callback
				IBSU_FingerQualityState savedQualityArray[4];
				memcpy(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray));

				if(!m_DisplayWindow.dispInvalidArea || 
					m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
				{
					m_clbkProperty.imageInfo.IsFinal = TRUE;
					m_pAlgo->m_cImgAnalysis.is_final = TRUE;
					m_pAlgo->_Algo_JudgeFingerQuality(&m_pAlgo->m_cImgAnalysis, &bIsGoodImage, &m_propertyInfo, &m_clbkProperty, m_DisplayWindow.dispInvalidArea);
					if (!m_bFirstSent_clbkClearPlaten)
					{
						if (memcmp(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray)) != 0)
						{
							_PostCallback(CALLBACK_FINGER_QUALITY);
						}
					}
				}

                // .... post image processing to make perfect image
                // Take Result image
                _PostImageProcessing_ForResult(InImg, OutImg);
                m_nGoodFrameCount = 0;
                SetTakeResultImageManually(FALSE);
                m_timeAfterBeginCaptureImage = 0;
                m_pAlgo->m_rollingStatus = 3;
                return TRUE;
            }
            else if (nRc != IBSU_STATUS_OK && !bIsFinal && 
				     m_pAlgo->m_cImgAnalysis.finger_count == 0)
            {
                if (m_pAlgo->g_Rolling_Saved_Complete == FALSE)
                {
                    m_nGoodFrameCount = 0;
                }
			}
        }
        else
        {
            // For IBSU_BGetRollingInfo function
            memset(m_pAlgo->m_capture_rolled_local_best_buffer, 0, m_UsbDeviceInfo.CisImgSize);
            m_pAlgo->m_rollingStatus = 0;
            m_pAlgo->g_LastX = -1;
        }
    }
    else if (m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
             m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS)
	{
/*#ifdef __G_DEBUG__
		int diff_mean;

 		diff_mean = abs(m_pAlgo->m_cImgAnalysis.final_mean-m_pAlgo->m_cImgAnalysis.mean);
        TRACE("bIsFinal=%d, gain = %d, mean=%d, diff_mean=%d, fc=%d, prev_fc=%d, diff_fc=%d, x=%d, prev_x=%d, diff_x=%d, y=%d, prev_y=%d, diff_y=%d\n", 
			bIsFinal, m_pAlgo->m_pPropertyInfo->nContrastValue, m_pAlgo->m_cImgAnalysis.mean, diff_mean, m_pAlgo->m_cImgAnalysis.foreground_count, m_pAlgo->m_cImgAnalysis.pre_foreground_count, m_pAlgo->m_cImgAnalysis.diff_foreground_count,
			m_pAlgo->m_cImgAnalysis.center_x, m_pAlgo->m_cImgAnalysis.pre_center_x, m_pAlgo->m_cImgAnalysis.center_x,
			m_pAlgo->m_cImgAnalysis.center_y, m_pAlgo->m_cImgAnalysis.pre_center_y, m_pAlgo->m_cImgAnalysis.diff_center_y);
#endif
*/
		if ((!m_UsbDeviceInfo.bDecimation && m_UsbDeviceInfo.nDecimation_Mode != DECIMATION_NONE) &&
			(m_pAlgo->m_cImgAnalysis.diff_foreground_count > SINGLE_FLAT_DIFF_TRES ||
			 m_pAlgo->m_cImgAnalysis.diff_center_x > 5 ||
			 m_pAlgo->m_cImgAnalysis.diff_center_y > 5))
		{
			if (!m_UsbDeviceInfo.bDecimation && m_clbkProperty.nFingerState == ENUM_IBSU_FINGER_COUNT_OK && bIsGoodImage == TRUE)
			{
			}
			else
			{
				bIsFinal = FALSE;
				m_pAlgo->m_cImgAnalysis.is_final = FALSE;
			}
		}

		if (m_propertyInfo.bEnableCaptureOnRelease && m_UsbDeviceInfo.bDecimation == FALSE)
		{
			if (m_bFirstPutFingerOnSensor &&
				m_SavedFingerState != m_clbkProperty.nFingerState &&
				m_clbkProperty.nFingerState == ENUM_IBSU_NON_FINGER)
			{
				memcpy(&m_pAlgo->m_cImgAnalysis, &m_pAlgo->m_BestFrame.imgAnalysis, sizeof(FrameImgAnalysis));
				InImg = m_pAlgo->m_BestFrame.Buffer;
				bIsFinal = TRUE;
			}
			else if (!GetTakeResultImageManually())
			{
				bIsFinal = FALSE;
			}
		}

       if (bIsFinal)
        {
            // If preview images are decimated, transition to capturing final non-decimated
			// image.  This applies only to Columbo.
            if (m_UsbDeviceInfo.bDecimation == TRUE && m_UsbDeviceInfo.nDecimation_Mode != DECIMATION_NONE)
            {
                bIsFinal = FALSE;
                bIsGoodImage = FALSE;
                m_nRemainedDecimationFrameCount--;
				if (m_nRemainedDecimationFrameCount >= 0 && m_nRemainedDecimationFrameCount < (__DEFAULT_COLUMBO_DECIMATION_COUNT__-1))
				{
					memcpy(OutImg, m_pAlgo->m_OutResultImg, m_UsbDeviceInfo.ImgSize);
				}
				else
				{
					_PostImageProcessing_ForPreview(InImg, OutImg, bIsGoodImage);
				}

/*#ifdef __G_DEBUG__
                TRACE("\n ==> m_nRemainedDecimationFrameCount = %d\n", m_nRemainedDecimationFrameCount);
#endif*/

                if (m_UsbDeviceInfo.nUsbSpeed >= USB_HIGH_SPEED)
                {
                    m_nRemainedDecimationFrameCount = 0;
                }

				// We set the exposure time to lower value, but this will only take effect 
				// at the second image.
                if (m_nRemainedDecimationFrameCount == __DEFAULT_COLUMBO_DECIMATION_COUNT__-1)
                {
                    _Seti_SetRegister(0x30, 0x00);
                    _Seti_SetRegister(0x31, 0x60);
                }
				else if (m_nRemainedDecimationFrameCount <= 0)
                {
                    m_UsbDeviceInfo.bDecimation = FALSE;

                    if (m_UsbDeviceInfo.nUsbSpeed < USB_HIGH_SPEED)
                    {
                        _Seti_SetRegister(0x30, 0x04);
                        _Seti_SetRegister(0x31, 0xFF);
                    }

                    _SetDecimation_Columbo(FALSE);
                }

                return FALSE;
            }

            // .... post image processing to make perfect image
            // Take Result image
			_PostCallback(CALLBACK_COMPLETE_ACQUISITION);
#if defined(WINCE)
			for (int i=0; i<3; i++)
			{
				Sleep(10);
				if ((int)m_pOverlayText.size() > 0 ||
					(int)m_pOverlayTextEx.size() > 0 ||
					(int)m_pOverlayLineEx.size() > 0 ||
					(int)m_pOverlayQuadrangleEx.size() > 0 ||
					(int)m_pOverlayShapeEx.size() > 0)
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
					_DrawClientWindow(m_pAlgo->m_OutResultImg, m_DisplayWindow, &dstOverlayText, &dstOverlayTextEx, &dstOverlayLineEx, &dstOverlayQuadrangleEx, &dstOverlayShapeEx);
					break;
				}
			}
#endif
			// fixed wrong finger quality callback
			if(!m_DisplayWindow.dispInvalidArea || 
				m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			{
				bIsFinal = TRUE;
				bIsGoodImage = TRUE;
				m_clbkProperty.imageInfo.IsFinal = TRUE;
				m_pAlgo->m_cImgAnalysis.is_final = TRUE;
				//BOOL bTmpGoodImage = TRUE;

				IBSU_FingerQualityState savedQualityArray[4];
				memcpy(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray));
				m_pAlgo->_Algo_JudgeFingerQuality(&m_pAlgo->m_cImgAnalysis, &bIsGoodImage, &m_propertyInfo, &m_clbkProperty, m_DisplayWindow.dispInvalidArea);

				if (!m_bFirstSent_clbkClearPlaten)
				{
					if (memcmp(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray)) != 0)
					{
						_PostCallback(CALLBACK_FINGER_QUALITY);
					}
				}
			}

			_PostImageProcessing_ForResult(InImg, OutImg);
            m_nGoodFrameCount = 0;
            SetTakeResultImageManually(FALSE);
            m_timeAfterBeginCaptureImage = 0;

            // Reset parameters for next capture.  This applies only to Columbo.
            m_nRemainedDecimationFrameCount = __DEFAULT_COLUMBO_DECIMATION_COUNT__;
            if (m_UsbDeviceInfo.nDecimation_Mode != DECIMATION_NONE)
            {
                m_UsbDeviceInfo.bDecimation = TRUE;
                _SetDecimation_Columbo(TRUE);
            }

            return TRUE;
        }
					else
					{
            // A bad image was captured after a good image.  Reset parameters to continue
            // capturing normal decimated images.  This applies only to Columbo.
            if (m_nRemainedDecimationFrameCount != __DEFAULT_COLUMBO_DECIMATION_COUNT__)
            {
                m_nRemainedDecimationFrameCount = __DEFAULT_COLUMBO_DECIMATION_COUNT__;

                if (m_UsbDeviceInfo.nDecimation_Mode != DECIMATION_NONE)
                {
                    if (m_UsbDeviceInfo.nUsbSpeed < USB_HIGH_SPEED)
                    {
                        _Seti_SetRegister(0x30, 0x04);
                        _Seti_SetRegister(0x31, 0xFF);
                    }

                    // We just captured a non-decimated image, which turned out to be 
                    // bad.
                    if (m_UsbDeviceInfo.bDecimation == FALSE)
                    {
/*#ifdef __G_DEBUG__
						TRACE("Detected non-decimated image\n");
#endif*/
                        // Need to display previous image (skip current non-decimated image)
                        memcpy(OutImg, m_pAlgo->m_OutResultImg, m_UsbDeviceInfo.ImgSize);

                        m_UsbDeviceInfo.bDecimation = TRUE;
                        _SetDecimation_Columbo(TRUE);
                        return FALSE;
                    }

                    m_UsbDeviceInfo.bDecimation = TRUE;
                    _SetDecimation_Columbo(TRUE);
                }
            }
        }
        }

    _PostImageProcessing_ForPreview(InImg, OutImg, bIsGoodImage);

    return FALSE;
    }

BOOL CMainCapture::_GoJob_PostImageProcessing(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage)
    {
    switch (m_UsbDeviceInfo.devType)
        {
        case DEVICE_TYPE_WATSON:
        case DEVICE_TYPE_WATSON_MINI:
        case DEVICE_TYPE_SHERLOCK_ROIC:
        case DEVICE_TYPE_SHERLOCK:
        case DEVICE_TYPE_COLUMBO:
        case DEVICE_TYPE_CURVE:
        case DEVICE_TYPE_CURVE_SETI:
            return _GoJob_PostImageProcessing_Default(InImg, OutImg, bIsGoodImage);
        case DEVICE_TYPE_HOLMES:
            return _GoJob_PostImageProcessing_Holmes(InImg, OutImg, bIsGoodImage);
		case DEVICE_TYPE_KOJAK:
            return _GoJob_PostImageProcessing_Kojak(InImg, OutImg, bIsGoodImage);
		case DEVICE_TYPE_FIVE0:
            return _GoJob_PostImageProcessing_Five0(InImg, OutImg, bIsGoodImage);
        default:
            break;
    }

    return FALSE;
}

void CMainCapture::_PostImageProcessing_ForResult(BYTE *InImg, BYTE *OutImg)
{
    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
        case DEVICE_TYPE_WATSON_MINI:
            _PostImageProcessing_ForResult_Watson(InImg, OutImg);
            break;
        case DEVICE_TYPE_SHERLOCK_ROIC:
        case DEVICE_TYPE_SHERLOCK:
            _PostImageProcessing_ForResult_Sherlock(InImg, OutImg);
            break;
        case DEVICE_TYPE_COLUMBO:
            _PostImageProcessing_ForResult_Columbo(InImg, OutImg);
            break;
        case DEVICE_TYPE_CURVE:
           	_PostImageProcessing_ForResult_Curve(InImg, OutImg);
            break;
        case DEVICE_TYPE_CURVE_SETI:
			_PostImageProcessing_ForResult_CurveSETi(InImg, OutImg);
            break;
        case DEVICE_TYPE_HOLMES:
            _PostImageProcessing_ForResult_Holmes(InImg, OutImg);
            break;
		case DEVICE_TYPE_KOJAK:
            _PostImageProcessing_ForResult_Kojak(InImg, OutImg);
            break;
		case DEVICE_TYPE_FIVE0:
            _PostImageProcessing_ForResult_Five0(InImg, OutImg);
            break;
        default:
            break;
    }

	int i, detected_count=0;
	for(i=0; i<IBSU_MAX_SEGMENT_COUNT; i++)
	{
		if(m_clbkProperty.qualityArray[i] != ENUM_IBSU_FINGER_NOT_PRESENT)
			detected_count++;
	}

	if(detected_count > m_pAlgo->m_segment_arr.SegmentCnt)
	{
		detected_count = 0;
		for(int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++)
		{
			if(m_clbkProperty.qualityArray[i] != ENUM_IBSU_FINGER_NOT_PRESENT)
				detected_count++;

			if(detected_count > m_pAlgo->m_segment_arr.SegmentCnt)
				m_clbkProperty.qualityArray[i] = ENUM_IBSU_FINGER_NOT_PRESENT;
		}

		_PostCallback(CALLBACK_FINGER_QUALITY);
		Sleep(10);
	}
}

void CMainCapture::_PostImageProcessing_ForPreview(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage)
{
#if defined(__IBSCAN_ULTIMATE_SDK__)
	if(m_propertyInfo.bNoPreviewImage)
	{
		if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
			if( m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK ||
				m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0 )
				memset(OutImg, 255, m_UsbDeviceInfo.ImgSize_Roll);
			else
				memset(OutImg, 255, m_UsbDeviceInfo.ImgSize);
		}
		else
			memset(OutImg, 255, m_UsbDeviceInfo.ImgSize);
		return;
	}
#endif

    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
        case DEVICE_TYPE_WATSON_MINI:
            _PostImageProcessing_ForPreview_Watson(InImg, OutImg, bIsGoodImage);
            break;
        case DEVICE_TYPE_SHERLOCK_ROIC:
        case DEVICE_TYPE_SHERLOCK:
            _PostImageProcessing_ForPreview_Sherlock(InImg, OutImg, bIsGoodImage);
            break;
        case DEVICE_TYPE_COLUMBO:
            _PostImageProcessing_ForPreview_Columbo(InImg, OutImg, bIsGoodImage);
            break;
        case DEVICE_TYPE_CURVE:
           	_PostImageProcessing_ForPreview_Curve(InImg, OutImg, bIsGoodImage);
            break;
        case DEVICE_TYPE_CURVE_SETI:
			_PostImageProcessing_ForPreview_CurveSETi(InImg, OutImg, bIsGoodImage);
            break;
        case DEVICE_TYPE_HOLMES:
            _PostImageProcessing_ForPreview_Holmes(InImg, OutImg, bIsGoodImage);
            break;
		case DEVICE_TYPE_KOJAK:
            _PostImageProcessing_ForPreview_Kojak(InImg, OutImg, bIsGoodImage);
            break;
		case DEVICE_TYPE_FIVE0:
            _PostImageProcessing_ForPreview_Five0(InImg, OutImg, bIsGoodImage);
            break;
        default:
            break;
    }
}

#if defined(__IBSCAN_SDK__)
AcuisitionState CMainCapture::_GoJob_OneFrameRawImage_ForThread(BYTE *InImg)
{
    BYTE		*OutImg = m_pAlgo->m_Inter_Img4;//InImg;
    struct 		timeval finish_tv, start_tv;
    double		elapsed;
    BOOL		bIsFinal = FALSE;
    BOOL		bIsGoodImage = FALSE;
    AcuisitionState nRc;
    int			sum_pixel, sum_start_pos, sum_end_pos;

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    if (_ClearDoubleBufferedImageOnFPGA(InImg, m_UsbDeviceInfo.nMIN_Elapsed) == ACUISITION_NOT_COMPLETE)
    {
        return ACUISITION_NOT_COMPLETE;
    }

    gettimeofday(&start_tv, NULL);

    nRc = _GoJob_PreImageProcessing_ForOneFrame(InImg, m_pAlgo->m_ImgOnProcessing, &bIsGoodImage);
    if (nRc != ACUISITION_NONE)
    {
        return nRc;
    }

    if( (m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CAPTURE) == IBSU_OPTION_AUTO_CAPTURE )
    {
        if( m_clbkProperty.nFingerState == ENUM_IBSU_FINGER_COUNT_OK && bIsGoodImage == TRUE )
            bIsFinal = TRUE;
    }
    else
    {
        if( bIsGoodImage == TRUE )
            bIsFinal = TRUE;
    }

    sum_pixel = 0;
    sum_start_pos = m_UsbDeviceInfo.CisImgHeight*m_UsbDeviceInfo.CisImgWidth/4;
    sum_end_pos = sum_start_pos + m_UsbDeviceInfo.CisImgWidth;
    for( int i=sum_start_pos; i<sum_end_pos; i++ )
        sum_pixel += m_pAlgo->m_ImgOnProcessing[i];

    sum_start_pos = m_UsbDeviceInfo.CisImgHeight*m_UsbDeviceInfo.CisImgWidth/2;
    sum_end_pos = sum_start_pos + m_UsbDeviceInfo.CisImgWidth;
    for( int i=sum_start_pos; i<sum_end_pos; i++ )
        sum_pixel += m_pAlgo->m_ImgOnProcessing[i];

    sum_start_pos = m_UsbDeviceInfo.CisImgHeight*m_UsbDeviceInfo.CisImgWidth*3/4;
    sum_end_pos = sum_start_pos + m_UsbDeviceInfo.CisImgWidth;
    for( int i=sum_start_pos; i<sum_end_pos; i++ )
        sum_pixel += m_pAlgo->m_ImgOnProcessing[i];

    m_pAlgo->m_cImgAnalysis.sum_pixel = sum_pixel;
    gettimeofday(&finish_tv, NULL);
    elapsed = (finish_tv.tv_sec - start_tv.tv_sec) + (finish_tv.tv_usec - start_tv.tv_usec) / 1000000.0;

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    if( m_nFrameCount++ > 100 )
        m_nFrameCount = 100;

    _PostCallback(CALLBACK_ONE_FRAME_IMAGE, m_pAlgo->m_ImgOnProcessing, elapsed, 0, 0, bIsFinal);

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    return ACUISITION_NOT_COMPLETE;
}

AcuisitionState CMainCapture::_GoJob_TakePreviewImage_ForThread(BYTE *InImg)
{
    BYTE		*OutImg = m_pAlgo->m_Inter_Img4;//InImg;
    struct 		timeval finish_tv, start_tv;
    double		elapsed;
    int			sum_pixel, sum_start_pos, sum_end_pos;

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    gettimeofday(&start_tv, NULL);

    sum_pixel = 0;
    sum_start_pos = m_UsbDeviceInfo.CisImgHeight*m_UsbDeviceInfo.CisImgWidth/4;
    sum_end_pos = sum_start_pos + m_UsbDeviceInfo.CisImgWidth;
    for( int i=sum_start_pos; i<sum_end_pos; i++ )
        sum_pixel += InImg[i];

    sum_start_pos = m_UsbDeviceInfo.CisImgHeight*m_UsbDeviceInfo.CisImgWidth/2;
    sum_end_pos = sum_start_pos + m_UsbDeviceInfo.CisImgWidth;
    for( int i=sum_start_pos; i<sum_end_pos; i++ )
        sum_pixel += InImg[i];

    sum_start_pos = m_UsbDeviceInfo.CisImgHeight*m_UsbDeviceInfo.CisImgWidth*3/4;
    sum_end_pos = sum_start_pos + m_UsbDeviceInfo.CisImgWidth;
    for( int i=sum_start_pos; i<sum_end_pos; i++ )
        sum_pixel += InImg[i];

    if( m_pAlgo->m_cImgAnalysis.sum_pixel != sum_pixel )
    {
        m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_GetBrightWithRawImage(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
    }

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    _PostImageProcessing_ForPreview(InImg, OutImg, FALSE);

    gettimeofday(&finish_tv, NULL);
    elapsed = (finish_tv.tv_sec - start_tv.tv_sec) + (finish_tv.tv_usec - start_tv.tv_usec) / 1000000.0;

    int gab = m_UsbDeviceInfo.nMIN_Elapsed - (int)((elapsed+m_clbkProperty.oneFrameImageInfo.FrameTime)*1000);
    if (gab > 0)
    {
        Sleep(gab);
        elapsed += (double)gab / 1000;
    }

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    _PostCallback(CALLBACK_TAKE_PREVIEW_IMAGE, OutImg, elapsed);

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    return ACUISITION_NOT_COMPLETE;
}

AcuisitionState CMainCapture::_GoJob_TakeResultImage_ForThread(BYTE *InImg)
{
    BYTE		*OutImg = m_pAlgo->m_Inter_Img4;//InImg;
    struct 		timeval finish_tv, start_tv;
    double		elapsed;
    int			sum_pixel, sum_start_pos, sum_end_pos;

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    gettimeofday(&start_tv, NULL);

    sum_pixel = 0;
    sum_start_pos = m_UsbDeviceInfo.CisImgHeight*m_UsbDeviceInfo.CisImgWidth/4;
    sum_end_pos = sum_start_pos + m_UsbDeviceInfo.CisImgWidth;
    for( int i=sum_start_pos; i<sum_end_pos; i++ )
        sum_pixel += InImg[i];

    sum_start_pos = m_UsbDeviceInfo.CisImgHeight*m_UsbDeviceInfo.CisImgWidth/2;
    sum_end_pos = sum_start_pos + m_UsbDeviceInfo.CisImgWidth;
    for( int i=sum_start_pos; i<sum_end_pos; i++ )
        sum_pixel += InImg[i];

    sum_start_pos = m_UsbDeviceInfo.CisImgHeight*m_UsbDeviceInfo.CisImgWidth*3/4;
    sum_end_pos = sum_start_pos + m_UsbDeviceInfo.CisImgWidth;
    for( int i=sum_start_pos; i<sum_end_pos; i++ )
        sum_pixel += InImg[i];

    if( m_pAlgo->m_cImgAnalysis.sum_pixel != sum_pixel )
    {
        m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_GetBrightWithRawImage(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
    }

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    _PostImageProcessing_ForResult(InImg, OutImg);

    gettimeofday(&finish_tv, NULL);
    elapsed = (finish_tv.tv_sec - start_tv.tv_sec) + (finish_tv.tv_usec - start_tv.tv_usec) / 1000000.0;

    int gab = m_UsbDeviceInfo.nMIN_Elapsed - (int)((elapsed+m_clbkProperty.oneFrameImageInfo.FrameTime)*1000);
    if (gab > 0)
    {
        Sleep(gab);
        elapsed += (double)gab / 1000;
    }

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    _PostCallback(CALLBACK_TAKE_RESULT_IMAGE, OutImg, elapsed);

    if (m_bCaptureThread_StopMessage)
    {
        return ACUISITION_ABORT;
    }

    return ACUISITION_NOT_COMPLETE;
}
#endif

BOOL CMainCapture::_UM_WAIT_BUSY()
{
    unsigned int timeout_counter = 0;
    unsigned int fail_counter = 0;
    BYTE		 cResult[2] = {0};

    while (TRUE)
    {
        if (!_UM_STATUS_READ(cResult))
        {
            fail_counter++;
        }
        else
        {
            if ((cResult[0] & 0x80) == 0)
            {
                timeout_counter++;
            }
            else
            {
                break;
            }
        }

        if (fail_counter > 1 || timeout_counter > 100)
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL CMainCapture::_UM_STATUS_READ(BYTE *cResult)
{
    unsigned char packet_buf[5], read_buf[2] = {0x00, 0x00};
    LONG /*lSize, */lActualBytes, writecnt = 0, readcnt = 2;
    int nRc;

    cResult[0] = 0x00;

    packet_buf[writecnt++] = 0;
    writecnt++;

    // Out count
    _FPGA_SetRegister(0x20, (UCHAR)(writecnt & 0xFF));
    _FPGA_SetRegister(0x21, (UCHAR)((writecnt >> 8) & 0x03));

    // In count
    _FPGA_SetRegister(0x22, (UCHAR)(readcnt & 0xFF));
    _FPGA_SetRegister(0x23, (UCHAR)((readcnt >> 8) & 0x03));

    // start bit set
    _FPGA_SetRegister(0x24, 0x01);

    nRc = _UsbBulkOutIn(EP2OUT, 0xD7, (UCHAR *)packet_buf, writecnt - 1,
                        EP8IN, (UCHAR *)read_buf, readcnt, &lActualBytes);
    if (nRc != IBSU_STATUS_OK || lActualBytes != readcnt)
    {
        return FALSE;
    }

    cResult[0] = read_buf[0];

    return TRUE;
}

BOOL CMainCapture::_UM_CONT_READ_FLASH(unsigned char *buf, LONG buf_count, unsigned int start_address, unsigned int page)
{
    unsigned char packet_buf[8];
    LONG /*lSize, */lActualBytes, writecnt = 0, readcnt = buf_count;
    int nRc;

    if (_UM_WAIT_BUSY() == FALSE)
    {
        return FALSE;
    }

    //	packet_buf[writecnt++] = 0xE8;
    packet_buf[writecnt++] = (page >> 6) & 0xFF;
    packet_buf[writecnt++] = (UCHAR)(page << 2) | (start_address >> 8);
    packet_buf[writecnt++] = (UCHAR)start_address;
    packet_buf[writecnt++] = 0x00;
    packet_buf[writecnt++] = 0x00;
    packet_buf[writecnt++] = 0x00;
    packet_buf[writecnt++] = 0x00;
    writecnt++;

    // Out count
    _FPGA_SetRegister(0x20, (UCHAR)(writecnt & 0xFF));
    _FPGA_SetRegister(0x21, (UCHAR)((writecnt >> 8) & 0x03));

    // In count
    _FPGA_SetRegister(0x22, (UCHAR)(readcnt & 0xFF));
    _FPGA_SetRegister(0x23, (UCHAR)((readcnt >> 8) & 0x03));

    // start bit set
    _FPGA_SetRegister(0x24, 0x01);

    nRc = _UsbBulkOutIn(EP2OUT, 0xE8, (UCHAR *)packet_buf, writecnt - 1,
                        EP8IN, (UCHAR *)buf, readcnt, &lActualBytes);
    if (nRc != IBSU_STATUS_OK || lActualBytes != readcnt)
    {
        return FALSE;
    }

    return TRUE;
}

BYTE CMainCapture::_UM_MAKE_CHECKSUM(unsigned char *mask_buf, LONG mask_buf_size)
{
    LONG i;
    unsigned char checksum = 0x00;

    for (i = 0; i < mask_buf_size; i++)
    {
        checksum += mask_buf[i];
    }

    return checksum;
}

BOOL CMainCapture::_IsAlive_UsbDevice()
{
//    if (_ResetFifo() != IBSU_STATUS_OK)
    char cSerialNumber[IBSU_MAX_STR_LEN]={0};
	if (_GetProperty(PROPERTY_SERIAL_NUMBER, (UCHAR *)cSerialNumber) != IBSU_STATUS_OK)
    {
        return FALSE;
    }

    return TRUE;
}

void CMainCapture::_ClearOverlayText()
{
	std::vector<OverlayText *>::iterator it = m_pOverlayText.begin();
	while( it != m_pOverlayText.end() )
	{
		if( *it )
		{
			delete *it;
			it = m_pOverlayText.erase(it);
		}
		else
			++it;
	}

    m_pOverlayText.clear();
}

OverlayText *CMainCapture::_FindOverlayTextInList(int x, int y)
{
    OverlayText	*pOverlayText = NULL;

    for (int i = 0; i < (int)m_pOverlayText.size(); i++)
    {
        pOverlayText = m_pOverlayText.at(i);
        if (pOverlayText->x == x && pOverlayText->y == y)
        {
            return pOverlayText;
        }
    }

    return NULL;
}

OverlayTextEx* CMainCapture::_FindOverlayTextExInList(const int overlayHandle)
{
	OverlayTextEx	*pOverlay = NULL;

	for(int i=0; i<(int)m_pOverlayTextEx.size(); i++ )
	{
		pOverlay = m_pOverlayTextEx.at(i);
        if( pOverlay->handle == overlayHandle )
		{
			return pOverlay;
		}
	}

	return NULL;
}

OverlayLineEx* CMainCapture::_FindOverlayLineExInList(const int overlayHandle)
{
	OverlayLineEx	*pOverlay = NULL;

	for(int i=0; i<(int)m_pOverlayLineEx.size(); i++ )
	{
		pOverlay = m_pOverlayLineEx.at(i);
        if( pOverlay->handle == overlayHandle )
		{
			return pOverlay;
		}
	}

	return NULL;
}

OverlayQuadrangleEx* CMainCapture::_FindOverlayQuadrangleExInList(const int overlayHandle)
{
	OverlayQuadrangleEx	*pOverlay = NULL;

	for(int i=0; i<(int)m_pOverlayQuadrangleEx.size(); i++ )
	{
		pOverlay = m_pOverlayQuadrangleEx.at(i);
        if( pOverlay->handle == overlayHandle )
		{
			return pOverlay;
		}
	}

	return NULL;
}

OverlayShapeEx* CMainCapture::_FindOverlayShapeExInList(const int overlayHandle)
{
	OverlayShapeEx	*pOverlay = NULL;

	for(int i=0; i<(int)m_pOverlayShapeEx.size(); i++ )
	{
		pOverlay = m_pOverlayShapeEx.at(i);
        if( pOverlay->handle == overlayHandle )
		{
			return pOverlay;
		}
	}

	return NULL;
}

DWORD CMainCapture::_GetThreadID()
{
#ifdef _WINDOWS
    return GetCurrentThreadId();
#else
    return pthread_self();
#endif
}

int CMainCapture::_OpenDeviceThread()
{
    // Make threads of initialization and capture
    int nRc;

#ifdef _WINDOWS
    if ((nRc = _CreateDeviceThread(&m_pInitializeThread, THREAD_INITIALIZE)) != IBSU_STATUS_OK) return nRc;
    if ((nRc = _CreateDeviceThread(&m_pCaptureThread, THREAD_CAPTURE)) != IBSU_STATUS_OK) return nRc;
    if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
    {
        if ((nRc = _CreateDeviceThread(&m_pKeyButtonThread, THREAD_KEYBUTTON)) != IBSU_STATUS_OK) return nRc;
    }
    if (m_UsbDeviceInfo.bCanUseTOF)
    {	
        if ((nRc = _CreateDeviceThread(&m_pTOFSensorThread, THREAD_TOFSENSOR)) != IBSU_STATUS_OK) return nRc;
        SetEvent(m_hTOFSensorThread_ReadEvent);
    }

#elif defined(__linux__)
    if ((nRc = _CreateDeviceThread(&m_pInitializeThread, THREAD_INITIALIZE)) != IBSU_STATUS_OK) return nRc;
    if ((nRc = _CreateDeviceThread(&m_pCaptureThread, THREAD_CAPTURE)) != IBSU_STATUS_OK) return nRc;
    if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
    {
        if ((nRc = _CreateDeviceThread(&m_pKeyButtonThread, THREAD_KEYBUTTON)) != IBSU_STATUS_OK) return nRc;
    }
	if (m_UsbDeviceInfo.bCanUseTOF)
    {	
        if ((nRc = _CreateDeviceThread(&m_pTOFSensorThread, THREAD_TOFSENSOR)) != IBSU_STATUS_OK) return nRc;
		m_bTOFSensorThread_ReadEvent = TRUE;
    }
#endif

    // If USB device is disconnected while the IntializeThread is running
    if (!_IsAlive_UsbDevice())
    {
    // Stop the capture stream
//    Capture_Abort();
#ifdef _WINDOWS
	    if (m_pCaptureThread)
	        _DestroyDeviceThread(&m_pCaptureThread, &m_hCaptureThread_DestroyEvent, &m_bAliveCaptureThread, THREAD_CAPTURE);
	    if (m_pKeyButtonThread)
	        _DestroyDeviceThread(&m_pKeyButtonThread, &m_hKeyButtonThread_DestroyEvent, &m_bAliveKeyButtonThread, THREAD_KEYBUTTON);
	    if (m_pTOFSensorThread)
	        _DestroyDeviceThread(&m_pTOFSensorThread, &m_hTOFSensorThread_DestroyEvent, &m_bAliveTOFSensorThread, THREAD_TOFSENSOR);
#elif defined(__linux__)
	    if (m_pCaptureThread)
	        _DestroyDeviceThread(&m_pCaptureThread, &m_bCaptureThread_DestroyEvent, &m_bAliveCaptureThread, THREAD_CAPTURE);
	    if (m_pKeyButtonThread)
	        _DestroyDeviceThread(&m_pKeyButtonThread, &m_bKeyButtonThread_DestroyEvent, &m_bAliveKeyButtonThread, THREAD_KEYBUTTON);
	    if (m_pTOFSensorThread)
	        _DestroyDeviceThread(&m_pTOFSensorThread, &m_bTOFSensorThread_DestroyEvent, &m_bAliveTOFSensorThread, THREAD_TOFSENSOR);
#endif

        _SndUsbFwCaptureStop();
        _SndUsbFwRelease();

        _CloseUSBDevice();
        return IBSU_ERR_DEVICE_IO;
    }

    return IBSU_STATUS_OK;
}

#if defined(_WINDOWS)
int CMainCapture::_CreateDeviceThread(CWinThread **thread, ThreadType type)
{
    if (*thread)
    {
        return IBSU_STATUS_OK;
    }

//G_TRACE("Enter _CreateDeviceThread (type - %d)\n", type);
    CWinThread *pThread;
    // Create thread
    ThreadParam param;

    param.pMainCapture = this;
    param.threadStarted = CreateEvent(NULL, FALSE, FALSE, NULL);

    switch (type)
    {
    case THREAD_INITIALIZE:
        pThread = ::AfxBeginThread(_InitializeThreadCallback, &param,
                                               THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);

        if (!pThread)
        {
            return IBSU_ERR_THREAD_CREATE;
        }

        (CWinThread*)*thread = pThread;
        pThread->m_bAutoDelete = FALSE;
        pThread->ResumeThread();
        ::WaitForSingleObject(pThread->m_hThread, INFINITE);
        delete pThread;
        pThread = 0;
        *thread = 0;
        CloseHandle(param.threadStarted);
//G_TRACE("Exit _CreateDeviceThread (type - %d)\n", type);
        return IBSU_STATUS_OK;
    case THREAD_CAPTURE:
        pThread = AfxBeginThread(_CaptureThreadCallback, &param,
                                          THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
        break;
    case THREAD_KEYBUTTON:
         pThread = AfxBeginThread(_KeyButtonThreadCallback, &param,
                                          THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
        break;
    case THREAD_TOFSENSOR:
		pThread = AfxBeginThread(_TOFSensorThreadCallback, &param,
                                          THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
        break;
//   case THREAD_TRACELOG:
//        pThread = AfxBeginThread(_TraceLogThreadCallback, &param,
//                                          THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
//        break;
    default:
        return -1;
    }

    if (!pThread)
    {
        return IBSU_ERR_THREAD_CREATE;
    }

    (CWinThread*)*thread = pThread;
    pThread->m_bAutoDelete = FALSE;
    pThread->ResumeThread();
    WaitForSingleObject(param.threadStarted, INFINITE);
    CloseHandle(param.threadStarted);

//G_TRACE("Exit _CreateDeviceThread (type - %d)\n", type);
    return IBSU_STATUS_OK;
}

#elif defined(__linux__)
int CMainCapture::_CreateDeviceThread(pthread_t *thread, ThreadType type)
{
    if (*thread)
    {
        return IBSU_STATUS_OK;
    }

//G_TRACE("Enter _CreateDeviceThread (type - %d)\n", type);
    pthread_t pThread;
    ThreadParam param;
    int threadRC;
    int tries;

    param.pMainCapture = this;

    pthread_attr_t threadAttr;

    // initialize the thread attribute
    pthread_attr_init(&threadAttr);

    // Set the stack size of the thread
    pthread_attr_setstacksize(&threadAttr, 240 * 1024);

    // Set thread to pthread_join
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

    // Create the threads
    pThread = 0;
    param.threadStarted = FALSE;

    switch (type)
    {
    case THREAD_INITIALIZE:
        // Need for pthread_join
        pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

        threadRC = pthread_create(&pThread, &threadAttr, _InitializeThreadCallback, &param);

        // Destroy the thread attributes
        pthread_attr_destroy(&threadAttr);

        if (threadRC != 0)
        {
            return IBSU_ERR_THREAD_CREATE;
        }

        memcpy(thread, &pThread, sizeof(pthread_t));
        pthread_join(pThread, NULL);
        pThread = 0;
        *thread = 0;
//G_TRACE("Exit _CreateDeviceThread (type - %d)\n", type);
        return IBSU_STATUS_OK;
    case THREAD_CAPTURE:
        threadRC = pthread_create(&pThread, &threadAttr, _CaptureThreadCallback, &param);
        break;
    case THREAD_KEYBUTTON:
        threadRC = pthread_create(&pThread, &threadAttr, _KeyButtonThreadCallback, &param);
        break;
    case THREAD_TOFSENSOR:
        threadRC = pthread_create(&pThread, &threadAttr, _TOFSensorThreadCallback, &param);
        break;
/*   case THREAD_TRACELOG:
        threadRC = pthread_create(&pThread, &threadAttr, _TraceLogThreadCallback, &param);
        break;
*/    default:
        return -1;
    }

    // Destroy the thread attributes
    pthread_attr_destroy(&threadAttr);

    if (threadRC != 0)
    {
        return IBSU_ERR_THREAD_CREATE;
    }

    memcpy(thread, &pThread, sizeof(pthread_t));
    tries = 0;
    do
    {
        Sleep(10);
        if (tries++ > 100)
        {
            break;
        }
    }
    while (!param.threadStarted);

//G_TRACE("Exit _CreateDeviceThread (type - %d)\n", type);
    return IBSU_STATUS_OK;
}
#endif

#if defined(_WINDOWS)
int CMainCapture::_DestroyDeviceThread(CWinThread **thread, HANDLE *destroyEvent, BOOL *pAliveThread, ThreadType type)
{
    CWinThread *pThread = (CWinThread*)*thread;
    int	tries = 0;

    // Stop the thread
    if (pThread)
    {
//G_TRACE("Enter _DestroyDeviceThread (type - %d)\n", type);
       SetEvent(*destroyEvent);
        do
        {
            Sleep(10);
            if (tries++ > 100)
            {
                // 2011-12-16 enzyme add - Forced to kill thread
                //				DWORD nExitCode = NULL;
                //				GetExitCodeThread( pThread->m_hThread, &nExitCode );
                //				TerminateThread( pThread->m_hThread, nExitCode );

                *pAliveThread = FALSE;
                break;
            }
        }
        while (*pAliveThread);

        ::WaitForSingleObject(pThread->m_hThread, INFINITE);
        delete pThread;
        pThread = 0;
        *thread = 0;
    }
    ResetEvent(*destroyEvent);

//G_TRACE("Exit _DestroyDeviceThread (type - %d)\n", type);
    return IBSU_STATUS_OK;
}
#elif defined(__linux__)
int CMainCapture::_DestroyDeviceThread(pthread_t *thread, BOOL *destroyEvent, BOOL *pAliveThread, ThreadType type)
{
    pthread_t pThread = (pthread_t)*thread;
    int tries = 0;

    // Stop the thread
    if (pThread)
    {
//G_TRACE("Enter _DestroyDeviceThread (type - %d)\n", type);
        do
        {
            *destroyEvent = TRUE;
            Sleep(10);
            if (tries++ > 100)
            {
                int rc = 0, status;
#ifdef __android__
                //  ** Android doesn't implement this -> rc = pthread_cancel(pThread); // Forced kill
                if ((status = pthread_kill(pThread, SIGUSR1)) != 0)
                {
                    // printf("Error cancelling thread %d, error = %d (%s)", pthread_id, status, strerror status));
                }
#else
                rc = pthread_cancel(pThread); // Forced kill
#endif
                if (rc == 0 && pthread_join(pThread, (void **)&status) == 0)
                {
#ifdef __G_DEBUG__
                    printf("Forced kill thread = %d\n", status);
#endif
                    *pAliveThread = FALSE;
                    break;
                }
            }

        }
        while (*pAliveThread);

        pThread = 0;
        *thread = 0;
    }

//G_TRACE("Exit _DestroyDeviceThread (type - %d)\n", type);
    return IBSU_STATUS_OK;
}
#endif

int CMainCapture::_AsyncOpenDeviceThread()
{
    // Make threads of Asyncronous OpenDevice thread
#ifdef _WINDOWS
    // 20130219 enzyme add - memory leak bug fixed
    if (m_pAsyncInitializeThread)
    {
        ::WaitForSingleObject(m_pAsyncInitializeThread->m_hThread, INFINITE);
        delete m_pAsyncInitializeThread;
        m_pAsyncInitializeThread = 0;
    }

    ThreadParam param;

    param.pMainCapture = this;
    param.pParentApp = m_pDlgUsbManager;
    param.threadStarted = CreateEvent(NULL, FALSE, FALSE, NULL);

    m_pAsyncInitializeThread = ::AfxBeginThread(_AsyncInitializeThreadCallback, &param,
                               THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);

    if (!m_pAsyncInitializeThread)
    {
        return IBSU_ERR_THREAD_CREATE;
    }

    m_pAsyncInitializeThread->m_bAutoDelete = FALSE;
    m_pAsyncInitializeThread->ResumeThread();
    WaitForSingleObject(param.threadStarted, INFINITE);
    CloseHandle(param.threadStarted);

#elif defined(__linux__)
    ThreadParam param;

    param.pMainCapture = this;
    param.pParentApp = m_pDlgUsbManager;

    pthread_attr_t threadAttr;

    //struct sched_param pThreadParam;  // scheduling priority

    // initialize the thread attribute
    pthread_attr_init(&threadAttr);

    // Set the stack size of the thread
    pthread_attr_setstacksize(&threadAttr, 240 * 1024);

    // Set thread to pthread_join
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

    // Create the threads
    m_pAsyncInitializeThread = 0;
    param.threadStarted = FALSE;
    int threadRC = pthread_create(&m_pAsyncInitializeThread, &threadAttr, _AsyncInitializeThreadCallback, &param);

    // Destroy the thread attributes
    pthread_attr_destroy(&threadAttr);

    if (threadRC != 0)
    {
        return IBSU_ERR_THREAD_CREATE;
    }

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

    return IBSU_STATUS_OK;
}

BOOL CMainCapture::_FPGA_GetVoltage(BYTE *Voltage)
{
    return _FPGA_GetRegister(0xA0, Voltage);
}

BOOL CMainCapture::_FPGA_SetVoltage(BYTE Voltage)
{
    int nRc;

    if ((nRc = _FPGA_SetRegister(0xA0, Voltage)) != IBSU_STATUS_OK)
    {
        return nRc;
    }

    return IBSU_STATUS_OK;
}

BOOL CMainCapture::_Algo_Kojak_GetWaterMaskFromFlashMemory(unsigned char *mask_buf, LONG mask_buf_size)
{
    int nRc = -1;
    BOOL isSameUM = FALSE;
//    int voltageValue = 0;

    if (m_propertyInfo.bEnablePowerSaveMode)
    {
        if (_SndUsbFwCaptureStop() != IBSU_STATUS_OK) goto done;
        if (_SndUsbFwCaptureStart() != IBSU_STATUS_OK) goto done;
        if (_InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart,
            m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight) != IBSU_STATUS_OK) goto done;
    }

/*	Capture_SetLEVoltage(__DEFAULT_VOLTAGE_VALUE_FOR_KOJAK__);
    Capture_GetLEVoltage(&voltageValue);
    if (voltageValue == __DEFAULT_VOLTAGE_VALUE_FOR_KOJAK__)
    {
        m_UsbDeviceInfo.LeVoltageControl = SUPPORT_LE_VOLTAGE_CONTROL;
    }
*/
	m_UsbDeviceInfo.LeVoltageControl = SUPPORT_LE_VOLTAGE_CONTROL;

    if (m_bUniformityMaskPath)
    {
        FILE *fp;

        fp = fopen(m_cUniformityMaskPath, "rb");
        if (fp != NULL)
        {
            const size_t maskSize = m_UsbDeviceInfo.CisImgSize + 8; // 8 is the checksum size
            size_t       readSize;

            /* Size+8 is compatible with original version of Columbo um files. Only Size+4 is needed. */
            readSize = fread(mask_buf, 1, maskSize, fp);
            fclose(fp);

            if (readSize == maskSize)
            {
                m_pAlgo->_Algo_Decrypt_Simple(mask_buf, mask_buf, maskSize);
                if (m_pAlgo->_Algo_ConfirmChecksum((DWORD *)mask_buf, (maskSize / sizeof(DWORD))))
                {
                    isSameUM = TRUE;
                }
            }
        }
    }

    if (isSameUM)
    {
        _PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, 90);

        if (m_propertyInfo.bEnablePowerSaveMode)
        {
            _SndUsbFwCaptureStop();
        }

        return TRUE;
    }

    _PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, 30);
    nRc = _ReadMaskFromEP8IN_Kojak_8M(mask_buf, m_UsbDeviceInfo.CisImgSize, 1);		// Bitshift 1인 놈만 현재는 사용한다.
    if (nRc != IBSU_STATUS_OK)
    {
        goto done;
    }

     _PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, 70);
	if (m_bUniformityMaskPath)
    {
        FILE *fp;
        fp = fopen(m_cUniformityMaskPath, "wb");
        if (fp != NULL)
        {
            const size_t   maskSize = m_UsbDeviceInfo.CisImgSize + 8; // 8 is the checksum size
            unsigned char *tmpMask = NULL;

            /* Size+8 is compatible with original version of Columbo um files. Only Size+4 is needed. */
			tmpMask = new unsigned char [maskSize];
            memcpy(tmpMask, mask_buf, m_UsbDeviceInfo.CisImgSize);
            m_pAlgo->_Algo_MakeChecksum((DWORD *)tmpMask, (maskSize / sizeof(DWORD)));
            m_pAlgo->_Algo_Encrypt_Simple(tmpMask, tmpMask, maskSize);
            (void)fwrite(tmpMask, 1, maskSize, fp);
            fclose(fp);
            delete [] tmpMask;
        }
    }

    _PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, 90);

    if (m_propertyInfo.bEnablePowerSaveMode == TRUE)
    {
        _SndUsbFwCaptureStop();
    }

    return TRUE;

done:
    if (m_propertyInfo.bEnablePowerSaveMode == TRUE)
    {
        _SndUsbFwCaptureStop();
    }
    else
    {
        /* Need to reset CPLD because CPLD was hanged due to unexpected reason */
        _SndUsbFwCaptureStop();
        _SndUsbFwCaptureStart();
        _InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
    }

    return FALSE;
}

BOOL CMainCapture::_Algo_Columbo_GetWaterMaskFromFlashMemory(unsigned char *mask_buf, LONG mask_buf_size)
{
    int nRc = -1;
    BOOL isSameUM = FALSE;
    int voltageValue = 0;

    if (m_propertyInfo.bEnablePowerSaveMode)
    {
        _SndUsbFwCaptureStop();
        _SndUsbFwCaptureStart();
        _InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
    }

    Capture_SetLEVoltage(__DEFAULT_VOLTAGE_VALUE__);
#if defined(__ppi__)
    if (m_bPPIMode)
    {
        voltageValue = __DEFAULT_VOLTAGE_VALUE__;
    }
    else
    {
        Capture_GetLEVoltage(&voltageValue);
    }
#else
    Capture_GetLEVoltage(&voltageValue);
#endif
    if (voltageValue == __DEFAULT_VOLTAGE_VALUE__)
    {
        m_UsbDeviceInfo.LeVoltageControl = SUPPORT_LE_VOLTAGE_CONTROL;
    }

    if (m_bUniformityMaskPath)
    {
        FILE *fp;

        fp = fopen(m_cUniformityMaskPath, "rb");
        if (fp != NULL)
        {
            const size_t maskSize = m_UsbDeviceInfo.CisImgSize + 8; // 8 is the checksum size
            size_t       readSize;

            /* Size+8 is compatible with original version of Columbo um files. Only Size+4 is needed. */
            readSize = fread(mask_buf, 1, maskSize, fp);
            fclose(fp);

            if (readSize == maskSize)
            {
                m_pAlgo->_Algo_Decrypt_Simple(mask_buf, mask_buf, maskSize);
                if (m_pAlgo->_Algo_ConfirmChecksum((DWORD *)mask_buf, (maskSize / sizeof(DWORD))))
                {
                    isSameUM = TRUE;
                }
            }
        }
    }

    if (isSameUM)
    {
        _PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, 90);

        if (m_propertyInfo.bEnablePowerSaveMode)
        {
            _SndUsbFwCaptureStop();
        }

        return TRUE;
    }

    nRc = _ReadMaskFromEP6IN_Columbo(mask_buf, m_UsbDeviceInfo.CisImgSize);
    if (nRc != IBSU_STATUS_OK)
    {
        goto done;
    }

    if (m_bUniformityMaskPath)
    {
        FILE *fp;
        fp = fopen(m_cUniformityMaskPath, "wb");
        if (fp != NULL)
        {
            const size_t   maskSize = m_UsbDeviceInfo.CisImgSize + 8; // 8 is the checksum size
            unsigned char *tmpMask = NULL;

            /* Size+8 is compatible with original version of Columbo um files. Only Size+4 is needed. */
            tmpMask = new unsigned char [maskSize];
            memcpy(tmpMask, mask_buf, m_UsbDeviceInfo.CisImgSize);
            m_pAlgo->_Algo_MakeChecksum((DWORD *)tmpMask, (maskSize / sizeof(DWORD)));
            m_pAlgo->_Algo_Encrypt_Simple(tmpMask, tmpMask, maskSize);
            (void)fwrite(tmpMask, 1, maskSize, fp);
            fclose(fp);
            delete [] tmpMask;
        }
    }

    _PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, 90);

    if (m_propertyInfo.bEnablePowerSaveMode == TRUE)
    {
        _SndUsbFwCaptureStop();
    }

    return TRUE;

done:
    if (m_propertyInfo.bEnablePowerSaveMode == TRUE)
    {
        _SndUsbFwCaptureStop();
    }
    else
    {
        /* Need to reset CPLD because CPLD was hanged due to unexpected reason */
        _SndUsbFwCaptureStop();
        _SndUsbFwCaptureStart();
        _InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
    }

    return FALSE;
}

BOOL CMainCapture::_Algo_GetWaterMaskFromFlashMemory(unsigned char *mask_buf, LONG mask_buf_size)
{
    LONG i, page_count;
    int remained_buf_size;
    LONG /*lSize, */lActualBytes, writecnt = 0;
    unsigned char packet_buf[10];
    unsigned char checksum, read_buf[UM_MAX_PAGE_SIZE];
    int	progressCount = 0, progressValue = 0;
    int voltageValue = 0;
    int nRc;
    BOOL isSameUM = FALSE;

    if (m_propertyInfo.bEnablePowerSaveMode)
    {
        _SndUsbFwCaptureStop();
        _SndUsbFwCaptureStart();
        _InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
    }

    Capture_SetLEVoltage(__DEFAULT_VOLTAGE_VALUE__);
    Capture_GetLEVoltage(&voltageValue);
    if (voltageValue == __DEFAULT_VOLTAGE_VALUE__)
    {
        m_UsbDeviceInfo.LeVoltageControl = SUPPORT_LE_VOLTAGE_CONTROL;
    }

    page_count = mask_buf_size / (UM_ONE_PAGE_SIZE - 1);
    remained_buf_size = mask_buf_size - page_count * (UM_ONE_PAGE_SIZE - 1);

    if (m_bUniformityMaskPath)
    {
        FILE *fp;

        fp = fopen(m_cUniformityMaskPath, "rb");
        if (fp != NULL)
        {
            const size_t maskSize = mask_buf_size + 4; // 4 is the checksum size
            size_t       readSize;

            readSize = fread(mask_buf, 1, maskSize, fp);
            fclose(fp);

            if (readSize == maskSize)
            {
                m_pAlgo->_Algo_Decrypt_Simple(mask_buf, mask_buf, maskSize);
                if (m_pAlgo->_Algo_ConfirmChecksum((DWORD *)mask_buf, maskSize / sizeof(DWORD)))
                {
                    isSameUM = TRUE;
                }
            }
        }

        if (isSameUM)
        {
            // Read sampling mask from device
            for (int j = 0; j < 2; j++)
            {
                if (j == 0)
                {
                    memset(read_buf, 0, sizeof(read_buf));
                    if (_UM_CONT_READ_FLASH(read_buf, UM_MAX_PAGE_SIZE, 0, j) == FALSE)
                    {
                        isSameUM = FALSE;
                        break;
                    }

                    checksum = _UM_MAKE_CHECKSUM(read_buf, UM_ONE_PAGE_SIZE - 1);

                    if (checksum != read_buf[UM_ONE_PAGE_SIZE - 1])
                    {
                        isSameUM = FALSE;
                        break;
                    }

                    if (memcmp(&mask_buf[j * (UM_ONE_PAGE_SIZE - 1)], read_buf, (UM_ONE_PAGE_SIZE - 1)) != 0)
                    {
                        isSameUM = FALSE;
                        break;
                    }
                }
                else
                {
                    writecnt = 0;
                    packet_buf[writecnt++] = (j >> 6) & 0xFF;
                    packet_buf[writecnt++] = (unsigned char)(j << 2);
                    packet_buf[writecnt++] = 0;
                    packet_buf[writecnt++] = 0;
                    packet_buf[writecnt++] = 0;
                    packet_buf[writecnt++] = 0;
                    packet_buf[writecnt++] = 0;

                    // start bit set
                    _FPGA_SetRegister(0x24, 0x01);

                    nRc = _UsbBulkOutIn(EP2OUT, 0xE8, (UCHAR *)packet_buf, writecnt,
                                        EP8IN, (UCHAR *)read_buf, UM_MAX_PAGE_SIZE, &lActualBytes);
                    if (nRc != IBSU_STATUS_OK || lActualBytes != UM_MAX_PAGE_SIZE)
                    {
                        isSameUM = FALSE;
                        break;
                    }

                    checksum = _UM_MAKE_CHECKSUM(read_buf, UM_ONE_PAGE_SIZE - 1);

                    if (checksum != read_buf[UM_ONE_PAGE_SIZE - 1])
                    {
                        if (j != (page_count - 1))
                        {
                            isSameUM = FALSE;
                            break;
                        }
                    }

                    if (memcmp(&mask_buf[j * (UM_ONE_PAGE_SIZE - 1)], read_buf, (UM_ONE_PAGE_SIZE - 1)) != 0)
                    {
                        isSameUM = FALSE;
                        break;
                    }
                }
            }
        }
    }

    if (isSameUM)
    {
        if (m_propertyInfo.bEnablePowerSaveMode == TRUE)
        {
            _SndUsbFwCaptureStop();
        }

        return TRUE;
    }

    for (i = 0; i < page_count; i++)
    {
        if ((i % 200) == 0)
        {
            progressCount++;
            progressValue = 10 + (i * 80) / page_count;// / progressCount*7 + 10;
            _PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, progressValue);
        }

        if (i == 0)
        {
            memset(read_buf, 0, sizeof(read_buf));
            if (_UM_CONT_READ_FLASH(read_buf, UM_MAX_PAGE_SIZE, 0, i) == FALSE)
            {
                goto done;
            }

            checksum = _UM_MAKE_CHECKSUM(read_buf, UM_ONE_PAGE_SIZE - 1);

            if (checksum != read_buf[UM_ONE_PAGE_SIZE - 1])
            {
                goto done;
            }

            memcpy(&mask_buf[i * (UM_ONE_PAGE_SIZE - 1)], read_buf, (UM_ONE_PAGE_SIZE - 1));
        }
        else
        {
            writecnt = 0;
            packet_buf[writecnt++] = (i >> 6) & 0xFF;
            packet_buf[writecnt++] = (unsigned char)(i << 2);
            packet_buf[writecnt++] = 0;
            packet_buf[writecnt++] = 0;
            packet_buf[writecnt++] = 0;
            packet_buf[writecnt++] = 0;
            packet_buf[writecnt++] = 0;

            // start bit set
            _FPGA_SetRegister(0x24, 0x01);

            nRc = _UsbBulkOutIn(EP2OUT, 0xE8, (UCHAR *)packet_buf, writecnt,
                                EP8IN, (UCHAR *)read_buf, UM_MAX_PAGE_SIZE, &lActualBytes);
            if (nRc != IBSU_STATUS_OK || lActualBytes != UM_MAX_PAGE_SIZE)
            {
                goto done;
            }

            checksum = _UM_MAKE_CHECKSUM(read_buf, UM_ONE_PAGE_SIZE - 1);

            if (checksum != read_buf[UM_ONE_PAGE_SIZE - 1])
            {
                if (i != (page_count - 1))
                {
                    goto done;
                }
            }

            memcpy(&mask_buf[i * (UM_ONE_PAGE_SIZE - 1)], read_buf, (UM_ONE_PAGE_SIZE - 1));
        }
    }

    if (remained_buf_size > 0)
    {
        writecnt = 0;
        packet_buf[writecnt++] = (i >> 6) & 0xFF;
        packet_buf[writecnt++] = (unsigned char)(i << 2);
        packet_buf[writecnt++] = 0;
        packet_buf[writecnt++] = 0;
        packet_buf[writecnt++] = 0;
        packet_buf[writecnt++] = 0;
        packet_buf[writecnt++] = 0;

        // start bit set
        _FPGA_SetRegister(0x24, 0x01);

        nRc = _UsbBulkOutIn(EP2OUT, 0xE8, (UCHAR *)packet_buf, writecnt, EP8IN, (UCHAR *)&mask_buf[i * UM_ONE_PAGE_SIZE], UM_MAX_PAGE_SIZE, &lActualBytes);
        if (nRc != IBSU_STATUS_OK || lActualBytes != UM_MAX_PAGE_SIZE)
        {
            goto done;
        }

        checksum = _UM_MAKE_CHECKSUM(read_buf, remained_buf_size);

        if (checksum != read_buf[remained_buf_size])
        {
            goto done;
        }

        memcpy(&mask_buf[i * (UM_ONE_PAGE_SIZE - 1)], read_buf, remained_buf_size);
    }

    if (m_bUniformityMaskPath)
    {
        FILE *fp;
        fp = fopen(m_cUniformityMaskPath, "wb");
        if (fp != NULL)
        {
            unsigned char *tmpMask = new unsigned char [mask_buf_size + 4];
            memcpy(tmpMask, mask_buf, mask_buf_size);
            m_pAlgo->_Algo_MakeChecksum((DWORD *)tmpMask, (mask_buf_size + 4) / sizeof(DWORD));
            m_pAlgo->_Algo_Encrypt_Simple(tmpMask, tmpMask, mask_buf_size + 4);
            fwrite(tmpMask, mask_buf_size + 4, 1, fp);
            fclose(fp);
            delete [] tmpMask;
        }
    }
    if (m_propertyInfo.bEnablePowerSaveMode == TRUE)
    {
        _SndUsbFwCaptureStop();
    }

    return TRUE;


done:
    if (m_propertyInfo.bEnablePowerSaveMode == TRUE)
    {
        _SndUsbFwCaptureStop();
    }

    return FALSE;
}

int CMainCapture::_GetContrast_FromGainTable(int index, int auto_capture_mode)
{
    const USHORT Watson_GainTable[35] =
    {
        0x0049, 0x004B, 0x004D, 0x004F, 0x0051, 0x0053, 0x0055, 0x0057, 0x0059, 0x005B,
        0x005D, 0x005F, 0x0061, 0x0063, 0x0065, 0x0067, 0x0069, 0x006B, 0x006D, 0x006F,
        0x0071, 0x0073, 0x0075, 0x0077, 0x0079, 0x007B, 0x007D, 0x007F, 0x017F, 0x027F,
        0x037F, 0x047F, 0x057F, 0x067F, 0x077F
    };
	const UCHAR Sherlock_GainTable[35] =
    {
        200, 198, 196, 194, 192, 190, 188, 186, 184, 182,
        180, 178, 176, 174, 172, 170, 168, 166, 164, 162,
        160, 158, 156, 154, 152, 150, 148, 146, 144, 142,
        140, 138, 136, 134, 132
    };
    const UCHAR Columbo_GainTable[35] =
    {
        0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F, 0x12, 0x14, 0x16,
        0x18, 0x1A, 0x1C, 0x1E, 0x21, 0x23, 0x25, 0x27, 0x29, 0x2B,
        0x2D, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3F, 0x41,
        0x43, 0x45, 0x47, 0x49, 0x4C
    };
    const UCHAR Curve_GainTable[35] =
    {
        17,  24,  31,  38,  45,  52,  59,  66,  73,  80,
        87,  94,  101, 108, 115, 122, 129, 136, 143, 150,
        157, 164, 171, 178, 185, 192, 199, 206, 213, 220,
        227, 234, 241, 248, 255
    };
    const UCHAR Sherlock_ROIC_GainTable_Manual[35] =
    {
        200, 199, 198, 197, 196, 195, 194, 193, 192, 191,
        190, 189, 188, 187, 186, 185, 184, 183, 182, 181,
        180, 179, 178, 177, 176, 175, 175, 175, 175, 175,
        175, 175, 175, 175, 175
    };
    const UCHAR Sherlock_ROIC_GainTable[35] =
    {
        200, 198, 196, 194, 192, 190, 188, 186, 184, 182,
        180, 178, 176, 174, 172, 170, 168, 166, 164, 162,
        160, 158, 156, 154, 152, 150, 148, 146, 144, 142,
        140, 138, 136, 134, 132
    };
/*    const USHORT Kojak_GainTable[35] =
    {
        0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 
        0x004D, 0x004E, 0x004F, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056,
        0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F, 0x0A5F,
        0x0B5F, 0x0C5F, 0x055F, 0x065F, 0x075F
    };
*/
	const USHORT Kojak_DACTable[35] =
    {
        0xEC, 0xEA, 0xE8, 0xE6, 0xE4, 0xE2, 0xE0, 0xDE, 0xDC, 0xDA, 
        0xD8, 0xD6, 0xD4, 0xD2, 0xD0, 0xCE, 0xCC, 0xCA, 0xC8, 0xC6,
        0xC4, 0xC2, 0xC0, 0xBE, 0xBC, 0xBA, 0xB8, 0xB6, 0xB4, 0xB2,
		0xB0, 0xAE, 0xAC, 0xAA, 0xA8
    };

	const USHORT Kojak_DACTable_forDP[35] =
    {
        0xF0, 0xEE, 0xEC, 0xEA, 0xE8, 0xE6, 0xE4, 0xE2, 0xE0, 0xDE, 
		0xDC, 0xDA, 0xD8, 0xD6, 0xD4, 0xD2, 0xD0, 0xCE, 0xCC, 0xCA, 
		0xC8, 0xC6, 0xC4, 0xC2, 0xC0, 0xBE, 0xBC, 0xBA, 0xB8, 0xB6, 
		0xB4, 0xB2, 0xB0, 0xAE, 0xAC
    };
/*
 * Should use more than 200Vpp in order to light-emitting phospher sufficiently
 * So we use a short LE on time instead of using a low voltage
 */ 
    const UCHAR Sherlock_ASIC_GainTableForBfilm[35] =
    {
        182, 181, 180, 179, 178, 177, 176, 175, 174, 173,
        172, 171, 168, 165, 162, 159, 156, 153, 150, 147,
        144, 141, 138, 135, 132, 129, 126, 123, 120, 117,
        114, 111, 108, 105, 102
    };

	const UCHAR Sherlock_ASIC_GainTableForDPfilm[35] =
    {
        204, 201, 198, 195, 192, 189, 186, 183, 180, 177,
        174, 171, 168, 165, 162, 159, 156, 153, 150, 147,
        144, 141, 138, 135, 132, 129, 126, 123, 120, 117,
        114, 111, 108, 105, 102
    };

    const UCHAR Holmes_GainTable[35] =
    {
        17,  24,  31,  38,  45,  52,  59,  66,  73,  80,
        87,  94,  101, 108, 115, 122, 129, 136, 143, 150,
        157, 164, 171, 178, 185, 192, 199, 206, 213, 220,
        227, 234, 241, 248, 255
    };

	const UCHAR CurveSETi_GainTable[35] =
	{ 
        0x00, 0x03, 0x07, 0x0A, 0x0E, 0x11, 0x15, 0x18, 0x1C, 0x1F,
		0x23, 0x26, 0x2A, 0x2D, 0x31, 0x34, 0x38, 0x3B, 0x3F, 0x42,
		0x46, 0x49, 0x4D, 0x50, 0x54, 0x57, 0x5B, 0x5E, 0x62, 0x65,
		0x69, 0x6C, 0x70, 0x73, 0x77
	};

	const USHORT Five0_DACTable[35] =
    {
        0xEC, 0xEA, 0xE8, 0xE6, 0xE4, 0xE2, 0xE0, 0xDE, 0xDC, 0xDA, 
        0xD8, 0xD6, 0xD4, 0xD2, 0xD0, 0xCE, 0xCC, 0xCA, 0xC8, 0xC6,
        0xC4, 0xC2, 0xC0, 0xBE, 0xBC, 0xBA, 0xB8, 0xB6, 0xB4, 0xB2,
		0xB0, 0xAE, 0xAC, 0xAA, 0xA8
    };

	const USHORT Five0_DACTable_forDP[35] =
    {
        0xF0, 0xEE, 0xEC, 0xEA, 0xE8, 0xE6, 0xE4, 0xE2, 0xE0, 0xDE, 
		0xDC, 0xDA, 0xD8, 0xD6, 0xD4, 0xD2, 0xD0, 0xCE, 0xCC, 0xCA, 
		0xC8, 0xC6, 0xC4, 0xC2, 0xC0, 0xBE, 0xBC, 0xBA, 0xB8, 0xB6, 
		0xB4, 0xB2, 0xB0, 0xAE, 0xAC
    };

	switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
        case DEVICE_TYPE_WATSON_MINI:
			return Watson_GainTable[index];			
        case DEVICE_TYPE_SHERLOCK:
            if (m_UsbDeviceInfo.bNewFPCB == TRUE)
            {
				if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_SHERLOCK_DOUBLE_P_)
				{
					m_CurrentCaptureDAC = Sherlock_ASIC_GainTableForDPfilm[index] + 0x38;
					return (int)(Sherlock_ASIC_GainTableForDPfilm[index] + 0x38);
				}
				else
				{
					m_CurrentCaptureDAC = Sherlock_ASIC_GainTableForBfilm[index];
					return (int)(Sherlock_ASIC_GainTableForBfilm[index]);
				}
            }
            else
            {
                return (int)(Sherlock_GainTable[index] * 0.8);
            }
        case DEVICE_TYPE_SHERLOCK_ROIC:
            if (auto_capture_mode == FALSE)
            {
                return Sherlock_ROIC_GainTable_Manual[index];
            }
            else
            {
                return Sherlock_ROIC_GainTable[index];
            }
        case DEVICE_TYPE_COLUMBO:
			m_CurrentCaptureDAC = Columbo_GainTable[index];
			return Columbo_GainTable[index];
        case DEVICE_TYPE_CURVE:
           	return Curve_GainTable[index];
        case DEVICE_TYPE_CURVE_SETI:
			return CurveSETi_GainTable[index];
        case DEVICE_TYPE_HOLMES:
            return Holmes_GainTable[index];
        case DEVICE_TYPE_KOJAK:
			if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_NEW_BRD_AND_DOUBLE_P_)
				m_CurrentCaptureDAC = Kojak_DACTable_forDP[index];
			else
				m_CurrentCaptureDAC = Kojak_DACTable[index];
			if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_NEW_BRD_AND_DOUBLE_P_)
				return Kojak_DACTable_forDP[index];
			else
				return Kojak_DACTable[index];
        case DEVICE_TYPE_FIVE0:
			if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_FIVE0_DOUBLE_P_)
				m_CurrentCaptureDAC = Five0_DACTable_forDP[index];
			else
				m_CurrentCaptureDAC = Five0_DACTable[index];
			if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_FIVE0_DOUBLE_P_)
				return Five0_DACTable_forDP[index];
			else
				return Five0_DACTable[index];
        default:
            return 0;
    }
}

int CMainCapture::_Set_CIS_GainRegister(int index, int auto_capture_mode, BOOL kojak_dac_mode)
{
    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
        case DEVICE_TYPE_WATSON_MINI:
            return _MT9M_SetRegister(MT9M_GLOBAL_GAIN, _GetContrast_FromGainTable(index, auto_capture_mode));
        case DEVICE_TYPE_SHERLOCK:
        case DEVICE_TYPE_SHERLOCK_ROIC:
#if defined(__IBSCAN_ULTIMATE_SDK__)
			{
				DWORD dwLeOnTime;
				if (index >= __ASIC_VOLTAGE_DEFAULT_VALUE__)
				{
					dwLeOnTime = 480000;
					_FPGA_SetRegister(0x2C, (UCHAR)((dwLeOnTime >> 16) & 0xFF));	// LE on time
					_FPGA_SetRegister(0x2D, (UCHAR)((dwLeOnTime >> 8) & 0xFF));	// LE on time
					_FPGA_SetRegister(0x2E, (UCHAR)(dwLeOnTime & 0xFF));	// LE on time
					return _DAC_SetRegister(_GetContrast_FromGainTable(index, auto_capture_mode));
				}
				else
				{
					if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_SHERLOCK_DOUBLE_P_)
						dwLeOnTime = 100000 + (34545*index);
					else
						dwLeOnTime = 150000 + (30000*index);

					_FPGA_SetRegister(0x2C, (UCHAR)((dwLeOnTime >> 16) & 0xFF));	// LE on time
					_FPGA_SetRegister(0x2D, (UCHAR)((dwLeOnTime >> 8) & 0xFF));	// LE on time
					_FPGA_SetRegister(0x2E, (UCHAR)(dwLeOnTime & 0xFF));	// LE on time
					return IBSU_STATUS_OK;
				}
				return _DAC_SetRegister(_GetContrast_FromGainTable(index, auto_capture_mode));
			}
#elif defined(__IBSCAN_SDK__)
            return _DAC_SetRegister(_GetContrast_FromGainTable(index, auto_capture_mode));
#endif
        case DEVICE_TYPE_COLUMBO:
            return _Seti_SetRegister(0x32, _GetContrast_FromGainTable(index, auto_capture_mode));
        case DEVICE_TYPE_CURVE:
			return _Hynix_SetRegister(0x30, _GetContrast_FromGainTable(index, auto_capture_mode));
        case DEVICE_TYPE_CURVE_SETI:
			return _CurveSETi_SetRegister(0x32, _GetContrast_FromGainTable(index, auto_capture_mode));
        case DEVICE_TYPE_HOLMES:
            return 0;
		case DEVICE_TYPE_KOJAK:
            if (auto_capture_mode == FALSE && kojak_dac_mode)
            {
			    _MT9M_SetRegister(MT9M_GLOBAL_GAIN, 0x005F);
			    return _DAC_SetRegister_for_Kojak(_GetContrast_FromGainTable(index, auto_capture_mode));
                //return _MT9M_SetRegister(MT9M_GLOBAL_GAIN, _GetContrast_FromGainTable(index, auto_capture_mode));
            }
			return 0;
//		    return _DAC_SetRegister_for_Kojak(_GetContrast_FromGainTable(index, auto_capture_mode));
        case DEVICE_TYPE_FIVE0:
		    if (auto_capture_mode == FALSE && kojak_dac_mode)
            {
				return _ApplyCaptureSettings_Five0(_GetContrast_FromGainTable(index, auto_capture_mode));
			}
			return 0;
        default:
            return 0;
    }
}

BOOL CMainCapture::_Go_Job_UnexpectedTermination_fromCaptureThread(int nUnexpectedCount)
{
    if (nUnexpectedCount > m_propertyInfo.retryCountWrongCommunication)
    {
        return TRUE;
    }
    else if ((nUnexpectedCount % 2) == 1)
    {
        if (_ResetFifo() == IBSU_STATUS_OK)
        {
            _SndUsbFwCaptureStop();
            _SndUsbFwCaptureStart();
            _InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
	        if (m_propertyInfo.bEnableDecimation)
            {
                _CaptureStart_for_Decimation();
            }
            // 2013-05-03 enzyme modify - Bug fixed
            // It is important to make initialization of image frame after resetting of CIS camera
            m_nFrameCount = -2;

            Capture_SetLEVoltage(m_propertyInfo.nVoltageValue);
            _SetLEOperationMode((WORD)m_propertyInfo.nLEOperationMode);
            _SetActiveLEDs(0, m_propertyInfo.dwActiveLEDs);

			if (m_UsbDeviceInfo.devType == DEVICE_TYPE_FIVE0)
			{
				_FPGA_SetRegister(0x2F, 0x01);
				m_propertyInfo.nLEOperationMode = ADDRESS_LE_OFF_MODE;
				_SetLEOperationMode(m_propertyInfo.nLEOperationMode);
			}
			if (m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK && m_UsbDeviceInfo.bNewFPCB)
			{
				ASIC_SetRegister(ASIC_MASTER_ADDRESS, 0x43, m_MasterValue);
				ASIC_SetRegister(ASIC_SLAVE_ADDRESS, 0x43, m_SlaveValue);
			}

            _PostCallback(CALLBACK_NOTIFY_MESSAGE, 0, 0, 0, IBSU_WRN_CHANNEL_IO_CAMERA_WRONG);
            _PostTraceLogCallback(IBSU_WRN_CHANNEL_IO_CAMERA_WRONG, "Camera work is wrong. reset is required");
        }
    }
    else
    {
        _PostCallback(CALLBACK_NOTIFY_MESSAGE, 0, 0, 0, IBSU_WRN_CHANNEL_IO_FRAME_MISSING);
        _PostTraceLogCallback(IBSU_WRN_CHANNEL_IO_FRAME_MISSING, "Missing a frame image");
    }

    return FALSE;
}

AcuisitionState CMainCapture::_ClearDoubleBufferedImageOnFPGA(BYTE *InImg, int MIN_Elapsed)
{
    struct 		timeval finish_tv, start_tv;
    double		elapsed;

    // enzyme comment 2012-10-31 I found a problem on Android and fixed it. The following is error with wrong memory
    // "_GetOneFrameImage(m_pAlgo->m_OutResultImg, m_UsbDeviceInfo.ImgSize);"
    // To clear double buffering image stored in FPGA
    for (; m_nFrameCount < 0; m_nFrameCount++)
    {
        gettimeofday(&start_tv, NULL);
        _GetOneFrameImage(InImg, m_UsbDeviceInfo.CisImgSize);
        
        if (_CheckForClearPlaten(InImg) == 1)
        {
            m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_CLEARD;
        }

        gettimeofday(&finish_tv, NULL);
        elapsed = (finish_tv.tv_sec - start_tv.tv_sec) + (finish_tv.tv_usec - start_tv.tv_usec) / 1000000.0;

        int gab = MIN_Elapsed - (int)(elapsed * 1000);
        if (gab > 0)
        {
            Sleep(gab);
        }
    }

    if (m_nFrameCount < 0)
    {
        m_nFrameCount++;

        return ACUISITION_NOT_COMPLETE;
    }

    return ACUISITION_NONE;
}

int CMainCapture::_DAC_SetRegister(WORD val)
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};

    outBuffer[0] = (UCHAR)val;

    return _UsbBulkOutIn(EP1OUT, CMD_DAC_WRITE_ADDR, outBuffer, 1, -1, NULL, 0, NULL);
}

int CMainCapture::_DAC_SetRegister_for_Kojak(WORD val)
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};

    outBuffer[0] = (UCHAR)val;

    return _UsbBulkOutIn(EP1OUT, 0x77, outBuffer, 1, -1, NULL, 0, NULL);
}

int CMainCapture::_CheckForSupportedImageType(const IBSU_ImageType imageType, ScannerDeviceType devType)
{
#if defined(__IBSCAN_SDK__)
    switch (devType)
    {
    case DEVICE_TYPE_WATSON:
    case DEVICE_TYPE_WATSON_MINI:
    case DEVICE_TYPE_SHERLOCK_ROIC:
    case DEVICE_TYPE_SHERLOCK:
        if (imageType == IBSCAN_ROLL_SINGLE_FINGER || 
            imageType == IBSCAN_FLAT_SINGLE_FINGER ||
            imageType == IBSCAN_FLAT_TWO_THUMBS ||
            imageType == IBSCAN_FLAT_TWO_FINGERS ||
            imageType == IBSCAN_SINGLE_FINGER ||
            imageType == IBSCAN_TWO_FINGERS)
        {
            return IBSU_STATUS_OK;
        }
        return IBSCAN_ERR_CHANNEL_INVALID_CAPTURE_MODE;
    default:
        return IBSCAN_ERR_CHANNEL_INVALID_CAPTURE_MODE;
    }
#elif defined(__IBSCAN_ULTIMATE_SDK__)
    if (imageType == ENUM_IBSU_TYPE_NONE)
    {
        return IBSU_ERR_NOT_SUPPORTED;
    }

    switch (devType)
    {
        case DEVICE_TYPE_WATSON:
        case DEVICE_TYPE_WATSON_MINI:
        case DEVICE_TYPE_SHERLOCK_ROIC:
        case DEVICE_TYPE_SHERLOCK:
            if (imageType == ENUM_IBSU_FLAT_THREE_FINGERS || 
				imageType == ENUM_IBSU_FLAT_FOUR_FINGERS)
            {
                return IBSU_ERR_NOT_SUPPORTED;
            }
            break;
        case DEVICE_TYPE_COLUMBO:
        case DEVICE_TYPE_CURVE:
        case DEVICE_TYPE_CURVE_SETI:
            if (imageType == ENUM_IBSU_ROLL_SINGLE_FINGER ||
                    imageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
					imageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
                    imageType == ENUM_IBSU_FLAT_FOUR_FINGERS)
            {
                return IBSU_ERR_NOT_SUPPORTED;
            }
            break;
        case DEVICE_TYPE_HOLMES:
        case DEVICE_TYPE_KOJAK:
        case DEVICE_TYPE_FIVE0:
            break;
        default:
            break;
    }
#endif

    return IBSU_STATUS_OK;
}

// Create overlay handle by assigning next higher int than already used, and insert into overlay handle map
OverlayHandle* CMainCapture::_CreateOverlayHandle()
{
	CThreadSync Sync;
	OverlayHandle	*pOverlayHandle = NULL;
	int maxHandle = -1;

	for(int i=0; i<(int)m_pOverlayHandle.size(); i++ )
	{
		pOverlayHandle = m_pOverlayHandle.at(i);
		if( pOverlayHandle->handle > maxHandle )
		{
			maxHandle = pOverlayHandle->handle;
		}
	}

	maxHandle++;
	OverlayHandle *pOverlay = NULL;
	pOverlay = new OverlayHandle;
    pOverlay->handle = maxHandle;
    pOverlay->show = TRUE;          // Default is TRUE
	m_pOverlayHandle.push_back(pOverlay);

	return pOverlay;
}

BOOL CMainCapture::_FindOverlayHandleInList(int handle)
{
	OverlayHandle *pOvHandle = NULL;

	for(int i=0; i<(int)m_pOverlayHandle.size(); i++ )
	{
		pOvHandle = m_pOverlayHandle.at(i);
		if( pOvHandle->handle == handle )
		{
			return TRUE;
		}
	}

	return FALSE;
}

int CMainCapture::_ShowOverlayObject(int handle, BOOL show)
{
	OverlayHandle *pOvHandle = NULL;

    if( !_FindOverlayHandleInList(handle) )
        return IBSU_ERR_INVALID_OVERLAY_HANDLE;
    
    // Show objects(text, line, quadrangle) from vector which is used argument handle
	for(int i=0; i<(int)m_pOverlayHandle.size(); i++ )
	{
		pOvHandle = m_pOverlayHandle.at(i);
		if( pOvHandle->handle == handle )
            pOvHandle->show = show;
	}

    return IBSU_STATUS_OK;
}

int CMainCapture::_ShowAllOverlayObject(BOOL show)
{
	OverlayHandle *pOvHandle = NULL;

    // Show objects(text, line, quadrangle) from vector which is used argument handle
	for(int i=0; i<(int)m_pOverlayHandle.size(); i++ )
	{
		pOvHandle = m_pOverlayHandle.at(i);
        pOvHandle->show = show;
	}

    return IBSU_STATUS_OK;
}

int CMainCapture::_RemoveOverlayObject(int handle)
{
    if( !_FindOverlayHandleInList(handle) )
        return IBSU_ERR_INVALID_OVERLAY_HANDLE;

    // Remove OverlayTextEx from vector which is used argument handle
	_RemoveOverlayTextEx(handle);

    // Remove OverlayLineEx from vector which is used argument handle
	_RemoveOverlayLineEx(handle);

    // Remove OverlayQuadrangleEx from vector which is used argument handle
	_RemoveOverlayQuadrangleEx(handle);

    // Remove OverlayShapeEx from vector which is used argument handle
	_RemoveOverlayShapeEx(handle);

    // Remove argument handle from vector
	_RemoveOverlayHandle(handle);

    return IBSU_STATUS_OK;
}

int CMainCapture::_RemoveAllOverlayObject()
{
    // Remove OverlayTextEx from vector which is used argument handle
	_RemoveAllOverlayTextEx();

    // Remove OverlayLineEx from vector which is used argument handle
	_RemoveAllOverlayLineEx();

    // Remove OverlayQuadrangleEx from vector which is used argument handle
	_RemoveAllOverlayQuadrangleEx();

    // Remove OverlayShapeEx from vector which is used argument handle
	_RemoveAllOverlayShapeEx();

    // Remove argument handle from vector
	_RemoveAllOverlayHandle();

    return IBSU_STATUS_OK;
}

CurveModel CMainCapture::_GetCurveModel(const PropertyInfo *pPropertyInfo, const UsbDeviceInfo *pScanDevDesc)
{
    //////////////////////////////////////////////////////////////////////////////////////
    // 2013-05-28 enzyme add - To know Curve's version from the dirty firmware
    // Because we need to add LED/Touch feature on SDK
    // Version history
    // CURVE_MODEL_CYTE,           // 1st version : 2004-09-13 (LED - X, Touch - X)
    // CURVE_MODEL_CYTE_V1,        // 2nd version : 2006-08-25 (LED - X, Touch - X)
    // CURVE_MODEL_CYTE_V2,        // 3rd version : 2006-12-27 (LED - X, Touch - X), to support WHQL with SN
    // CURVE_MODEL_TBN240,         // Current version : 2011-04-04 (LED - O, Touch - O), but it has SN bug.
    // CURVE_MODEL_TBN240_V1       // Future version with enhancement F/W : (LED - O, Touch - O).

    BYTE curve_model_cyte[32] =
    {
        0xC2, 0x3F, 0x11, 0x04, 0x10, 0x00, 0x00, 0x40, 0x00, 0x06, 0x00, 0x00, 0x02, 0x0A, 0xB8, 0x02,
        0x07, 0xAE, 0x00, 0x03, 0x00, 0x33, 0x02, 0x09, 0xFC, 0x00, 0x03, 0x00, 0x43, 0x02, 0x0A, 0x00
    };
    BYTE curve_model_cyte_v1[32] =
    {
        0xC2, 0x3F, 0x11, 0x04, 0x10, 0x00, 0x00, 0x40, 0x00, 0x06, 0x00, 0x00, 0x02, 0x0B, 0x0E, 0x02,
        0x07, 0xFD, 0x00, 0x03, 0x00, 0x33, 0x02, 0x0F, 0x4B, 0x00, 0x03, 0x00, 0x43, 0x02, 0x09, 0x00
    };
    BYTE curve_model_cyte_v2[32] =
    {
        0xC2, 0x3F, 0x11, 0x04, 0x10, 0x00, 0x00, 0x40, 0x00, 0x06, 0x00, 0x00, 0x02, 0x0B, 0x63, 0x02,
        0x08, 0x35, 0x00, 0x03, 0x00, 0x33, 0x02, 0x0F, 0x86, 0x00, 0x03, 0x00, 0x43, 0x02, 0x0A, 0x00
    };
    BYTE curve_model_tbn240[32] =
    {
        0xC2, 0x3F, 0x11, 0x04, 0x10, 0x00, 0x00, 0x40, 0x00, 0x06, 0x00, 0x00, 0x02, 0x0B, 0x63, 0x02,
        0x08, 0x55, 0x00, 0x03, 0x00, 0x33, 0x02, 0x0F, 0xBA, 0x00, 0x03, 0x00, 0x43, 0x02, 0x0A, 0x00
    };

	// add new model (SETi CIS : TBN 3xx series)
	BYTE curve_model_tbn320[32] =
    {
		0xC2, 0x3F, 0x11, 0x04, 0x10, 0x00, 0x00, 0x40, 0x00, 0x06, 0x00, 0x00, 0x02, 0x0B, 0x63, 0x02,
		0x08, 0x12, 0x00, 0x03, 0x00, 0x33, 0x02, 0x0F, 0x90, 0x00, 0x03, 0x00, 0x43, 0x02, 0x0A, 0x00
    };
	BYTE curve_model_tbn320_with_USN[32] =
    {
		0xC2, 0x3F, 0x11, 0x04, 0x10, 0x00, 0x00, 0x40, 0x00, 0x06, 0x00, 0x00, 0x02, 0x0B, 0x63, 0x02,
		0x08, 0x50, 0x00, 0x03, 0x00, 0x33, 0x02, 0x0F, 0xCC, 0x00, 0x03, 0x00, 0x43, 0x02, 0x0A, 0x00
    };
	BYTE curve_model_tbn340[32] =
    {
        0xC2, 0x3F, 0x11, 0x04, 0x10, 0x00, 0x00, 0x40, 0x00, 0x06, 0x00, 0x00, 0x02, 0x0A, 0x10, 0x02,
		0x0B, 0xB8, 0x00, 0x03, 0x00, 0x33, 0x02, 0x0E, 0x33, 0x00, 0x03, 0x00, 0x43, 0x02, 0x08, 0x00
    };
	BYTE curve_model_tbn340_with_USN[32] =
    {
        0xC2, 0x3F, 0x11, 0x04, 0x10, 0x00, 0x00, 0x40, 0x00, 0x06, 0x00, 0x00, 0x02, 0x0A, 0x63, 0x02,
		0x0C, 0x0B, 0x00, 0x03, 0x00, 0x33, 0x02, 0x0E, 0x71, 0x00, 0x03, 0x00, 0x43, 0x02, 0x09, 0x00
    };

    CurveModel  cModel = UNKNOWN;
    BYTE        cSnFromRom[256] = {0};

    Reserved_ReadEEPROM(0x0000, cSnFromRom, 32);
    if (memcmp(cSnFromRom, curve_model_cyte, 32) == 0)
    {
        cModel = CURVE_MODEL_CYTE;
    }
    else if (memcmp(cSnFromRom, curve_model_cyte_v1, 32) == 0)
    {
        cModel = CURVE_MODEL_CYTE_V1;
    }
    else if (memcmp(cSnFromRom, curve_model_cyte_v2, 32) == 0)
    {
        cModel = CURVE_MODEL_CYTE_V2;
    }
    else if (memcmp(cSnFromRom, curve_model_tbn240, 32) == 0)
    {
        cModel = CURVE_MODEL_TBN240;
    }
	else if (memcmp(cSnFromRom, curve_model_tbn320, 32) == 0 || memcmp(cSnFromRom, curve_model_tbn320_with_USN, 32) == 0)
    {
        cModel = CURVE_MODEL_TBN320;
    }
	else if (memcmp(cSnFromRom, curve_model_tbn340, 32) == 0 || memcmp(cSnFromRom, curve_model_tbn340_with_USN, 32) == 0)
    {
        cModel = CURVE_MODEL_TBN340;
    }
    else
    {
		UCHAR		outBuffer[64] = {0};
		LONG		nInBufferLen = 32;
		int			nRc;

        cModel = UNKNOWN;
		outBuffer[0] = (UCHAR)CMD_READ_PROPERTY;
		outBuffer[1] = (UCHAR)PROPERTY_CURVE_SETI_MODEL;
		UCHAR tmpRead[IBSU_MAX_STR_LEN]={0};
		nRc = _UsbBulkOutIn(EP1OUT, 0x02, outBuffer, 2,
							EP1IN, (PUCHAR)tmpRead, nInBufferLen, NULL, 500);
		if (nRc == IBSU_STATUS_OK)
		{
			// to bug fixed of firmware
			nRc = _UsbBulkOutIn(EP1OUT, 0x02, outBuffer, 2,
							EP1IN, (PUCHAR)tmpRead, nInBufferLen, NULL, 500);

			if (nRc == IBSU_STATUS_OK)
			{
				if (strcmp((char*)tmpRead, "CURVE_MODEL_TBN320_V1") == 0)
				{
					cModel = CURVE_MODEL_TBN320_V1;
				}
				else if (strcmp((char*)tmpRead, "CURVE_MODEL_TBN340_V1") == 0)
				{
					cModel = CURVE_MODEL_TBN340_V1;
				}
			}
		}
    }

    return cModel;
}

void CMainCapture::_RemoveOverlayHandle(int handle)
{
	std::vector<OverlayHandle *>::iterator it = m_pOverlayHandle.begin();
	while( it != m_pOverlayHandle.end() )
	{
		if( *it && ((*it)->handle ==handle) )
		{
			delete *it;
			it = m_pOverlayHandle.erase(it);
		}
		else
			++it;
	}
}

void CMainCapture::_RemoveOverlayTextEx(int handle)
{
	std::vector<OverlayTextEx *>::iterator it = m_pOverlayTextEx.begin();
	while( it != m_pOverlayTextEx.end() )
	{
		if( *it && ((*it)->handle ==handle) )
		{
			delete *it;
			it = m_pOverlayTextEx.erase(it);
		}
		else
			++it;
	}
}

void CMainCapture::_RemoveOverlayLineEx(int handle)
{
	std::vector<OverlayLineEx *>::iterator it = m_pOverlayLineEx.begin();
	while( it != m_pOverlayLineEx.end() )
	{
		if( *it && ((*it)->handle ==handle) )
		{
			delete *it;
			it = m_pOverlayLineEx.erase(it);
		}
		else
			++it;
	}
}

void CMainCapture::_RemoveOverlayQuadrangleEx(int handle)
{
	std::vector<OverlayQuadrangleEx *>::iterator it = m_pOverlayQuadrangleEx.begin();
	while( it != m_pOverlayQuadrangleEx.end() )
	{
		if( *it && ((*it)->handle ==handle) )
		{
			delete *it;
			it = m_pOverlayQuadrangleEx.erase(it);
		}
		else
			++it;
	}
}

void CMainCapture::_RemoveOverlayShapeEx(int handle)
{
	std::vector<OverlayShapeEx *>::iterator it = m_pOverlayShapeEx.begin();
	while( it != m_pOverlayShapeEx.end() )
	{
		if( *it && ((*it)->handle ==handle) )
		{
			delete *it;
			it = m_pOverlayShapeEx.erase(it);
		}
		else
			++it;
	}
}

void CMainCapture::_RemoveAllOverlayText()
{
	std::vector<OverlayText *>::iterator it = m_pOverlayText.begin();
	while( it != m_pOverlayText.end() )
	{
		if( *it )
		{
			delete *it;
			it = m_pOverlayText.erase(it);
		}
		else
			++it;
	}
	m_pOverlayText.clear();
}

void CMainCapture::_RemoveAllOverlayHandle()
{
	std::vector<OverlayHandle *>::iterator it = m_pOverlayHandle.begin();
	while( it != m_pOverlayHandle.end() )
	{
		if( *it )
		{
			delete *it;
			it = m_pOverlayHandle.erase(it);
		}
		else
			++it;
	}
	m_pOverlayHandle.clear();
}

void CMainCapture::_RemoveAllOverlayTextEx()
{
	std::vector<OverlayTextEx *>::iterator it = m_pOverlayTextEx.begin();
	while( it != m_pOverlayTextEx.end() )
	{
		if( *it )
		{
			delete *it;
			it = m_pOverlayTextEx.erase(it);
		}
		else
			++it;
	}
	m_pOverlayTextEx.clear();
}

void CMainCapture::_RemoveAllOverlayLineEx()
{
	std::vector<OverlayLineEx *>::iterator it = m_pOverlayLineEx.begin();
	while( it != m_pOverlayLineEx.end() )
	{
		if( *it )
		{
			delete *it;
			it = m_pOverlayLineEx.erase(it);
		}
		else
			++it;
	}
	m_pOverlayLineEx.clear();
}

void CMainCapture::_RemoveAllOverlayQuadrangleEx()
{
	std::vector<OverlayQuadrangleEx *>::iterator it = m_pOverlayQuadrangleEx.begin();
	while( it != m_pOverlayQuadrangleEx.end() )
	{
		if( *it )
		{
			delete *it;
			it = m_pOverlayQuadrangleEx.erase(it);
		}
		else
			++it;
	}
	m_pOverlayQuadrangleEx.clear();
}

void CMainCapture::_RemoveAllOverlayShapeEx()
{
	std::vector<OverlayShapeEx *>::iterator it = m_pOverlayShapeEx.begin();
	while( it != m_pOverlayShapeEx.end() )
	{
		if( *it )
		{
			delete *it;
			it = m_pOverlayShapeEx.erase(it);
		}
		else
			++it;
	}
	m_pOverlayShapeEx.clear();
}



/////////////////////////////////////////////////////////////////////////////////////////////
int CMainCapture::Reserved_GetDeviceInfo(const int deviceIndex, RESERVED_DeviceInfo *deviceInfo, BOOL bPPIMode)
{
    int nErrorNo;
    int nRc;
	char cCmtValue[IBSU_MAX_STR_LEN]={0};
    CurveModel curveModel = UNKNOWN;
    m_bPPIMode = bPPIMode;
    int nDeviceIndex = deviceIndex;
#if defined(__ppi__)
    if (!bPPIMode)
    {
        nDeviceIndex = deviceIndex - m_pDlg->m_nPPIDeviceCount;
    }
#endif

    memset(deviceInfo, 0, sizeof(RESERVED_DeviceInfo));
    if (!IsInitialized())
    {
        if (!_OpenUSBDevice(nDeviceIndex, &nErrorNo))
        {
            return nErrorNo;
        }
    }

    if ((nRc = _GetProperty(PROPERTY_PRODUCT_ID, (UCHAR *)deviceInfo->productName)) != IBSU_STATUS_OK)
    {
		// Correction for unstable device
		if (nRc != IBSU_ERR_CHANNEL_IO_READ_TIMEOUT)
        goto done;

		_CloseUSBDevice();
		if (!_OpenUSBDevice(nDeviceIndex, &nErrorNo)) goto done;
		if ((nRc = _GetProperty(PROPERTY_PRODUCT_ID, (UCHAR *)deviceInfo->productName)) != IBSU_STATUS_OK) goto done;
    }
    if ((nRc = _GetProperty(PROPERTY_SERIAL_NUMBER, (UCHAR *)deviceInfo->serialNumber)) != IBSU_STATUS_OK)
    {
        goto done;
    }
    if ((nRc = _GetProperty(PROPERTY_FIRMWARE, (UCHAR *)deviceInfo->fwVersion)) != IBSU_STATUS_OK)
    {
        goto done;
    }
    if ((nRc = _GetProperty(PROPERTY_REVISION, (UCHAR *)deviceInfo->devRevision)) != IBSU_STATUS_OK)
    {
        goto done;
    }
    if ((nRc = _GetProperty(PROPERTY_VENDOR_ID, (UCHAR *)deviceInfo->vendorID)) != IBSU_STATUS_OK)
    {
        goto done;
    }
    if ((nRc = _GetProperty(PROPERTY_PRODUCTION_DATE, (UCHAR *)deviceInfo->productionDate)) != IBSU_STATUS_OK)
    {
        goto done;
    }
    if ((nRc = _GetProperty(PROPERTY_SERVICE_DATE, (UCHAR *)deviceInfo->serviceDate)) != IBSU_STATUS_OK)
    {
        goto done;
    }
#if defined(__IBSCAN_ULTIMATE_SDK__)
	if ((nRc = _GetProperty(PROPERTY_CMT1, (UCHAR*)cCmtValue)) != IBSU_STATUS_OK )
	{
        goto done;
    }
	memcpy(&deviceInfo->reserved1[0], cCmtValue, 32);
	if ((nRc = _GetProperty(PROPERTY_CMT2, (UCHAR*)cCmtValue)) != IBSU_STATUS_OK )
	{
        goto done;
    }
	memcpy(&deviceInfo->reserved1[32], cCmtValue, 32);
	if ((nRc = _GetProperty(PROPERTY_CMT3, (UCHAR*)cCmtValue)) != IBSU_STATUS_OK )
	{
        goto done;
    }
	memcpy(&deviceInfo->reserved1[64], cCmtValue, 32);
	if ((nRc = _GetProperty(PROPERTY_CMT4, (UCHAR*)cCmtValue)) != IBSU_STATUS_OK )
	{
        goto done;
    }
	memcpy(&deviceInfo->reserved1[96], cCmtValue, 4);

    if (_GetPID() == __PID_CURVE__)
    {
	    curveModel = _GetCurveModel(&m_propertyInfo, &m_UsbDeviceInfo);
    }

    switch(curveModel)
    {
    case CURVE_MODEL_CYTE:
    case CURVE_MODEL_CYTE_V1:
    case CURVE_MODEL_CYTE_V2:
        sprintf(&deviceInfo->reserved2[0], "%s", "CURVE-220");
        break;
    case CURVE_MODEL_TBN240:
        sprintf(&deviceInfo->reserved2[0], "%s", "CURVE-240");
        break;
    case CURVE_MODEL_TBN320:
    case CURVE_MODEL_TBN320_V1:
        sprintf(&deviceInfo->reserved2[0], "%s", "CURVE-320");
        break;
    case CURVE_MODEL_TBN340:
    case CURVE_MODEL_TBN340_V1:
        sprintf(&deviceInfo->reserved2[0], "%s", "CURVE-340");
        break;
    default:
        sprintf(&deviceInfo->reserved2[0], "%s", "UNKNOWN");
    }
#endif

    deviceInfo->handle = -1;
    deviceInfo->IsHandleOpened = FALSE;
    sprintf(deviceInfo->interfaceType, "%s", "USB");
#if defined(__ppi__)	
    if (m_bPPIMode)
    {
        sprintf(deviceInfo->interfaceType, "%s", "PPI");    // Parallel Peripheral Interface
    }
#endif

    if (!IsInitialized())
    {
        _CloseUSBDevice();
    }

    return IBSU_STATUS_OK;

done:
    if (!IsInitialized())
    {
        _CloseUSBDevice();
    }

    if (nRc == IBSU_ERR_CHANNEL_IO_WRITE_FAILED || nRc == IBSU_ERR_CHANNEL_IO_READ_FAILED)
    {
        nRc = IBSU_WRN_CHANNEL_IO_SLEEP_STATUS;
    }

    return nRc;
}

int CMainCapture::Reserved_WriteEEPROM(WORD addr, const BYTE *buf, int len)
{
    UCHAR		outBuffer[64]={0};
	int			nRc = IBSU_STATUS_OK;
	
	if( _GetPID() == __PID_CURVE__ )
	{
		return _Hynix_WriteEEPROM(addr, buf, len);
	}

	// Unlock Write Protection
	outBuffer[0] = 0;		// 0 : Unlock,  1 : Lock
	if((nRc = _UsbBulkOutIn(EP1OUT, CMD_EEPROM_WRITEPROTECTION, outBuffer, 1, -1, NULL, 0, NULL)) != IBSU_STATUS_OK)
		return nRc;

	outBuffer[0] = (UCHAR)(addr&0xFF);
	outBuffer[1] = (UCHAR)((addr>>8)&0x1F);
	outBuffer[2] = (UCHAR)len;
	for( int i = 0; i < len; i++ )
	{
		outBuffer[i+3] = buf[i];
	}

	if((nRc = _UsbBulkOutIn( EP1OUT, CMD_WRITE_EEPROM, outBuffer, 3+len, -1, NULL, 0, NULL)) != IBSU_STATUS_OK)
		return nRc;

	// Lock Write Protection
	outBuffer[0] = 1;		// 0 : Unlock,  1 : Lock
	if((nRc = _UsbBulkOutIn(EP1OUT, CMD_EEPROM_WRITEPROTECTION, outBuffer, 1, -1, NULL, 0, NULL)) != IBSU_STATUS_OK)
		return nRc;

	return IBSU_STATUS_OK;
}

int CMainCapture::Reserved_ReadEEPROM(WORD addr, BYTE *buf, int len)
{
    UCHAR		outBuffer[64] = {0};

    if (_GetPID() == __PID_CURVE__)
    {
        return _Hynix_ReadEEPROM(addr, buf, len);
    }

    outBuffer[0] = (UCHAR)(addr & 0xFF);
    outBuffer[1] = (UCHAR)((addr >> 8) & 0x1F);
    outBuffer[2] = (UCHAR)len;

    return _UsbBulkOutIn(EP1OUT, CMD_READ_EEPROM, outBuffer, 3, EP1IN, buf, len, NULL);
}

int CMainCapture::Reserved_GetOneFrameImage(unsigned char *pRawImage, int imageLength)
{
    if (!m_bIsActiveCapture)
    {
        return IBSU_ERR_CAPTURE_NOT_RUNNING;
    }

    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
        case DEVICE_TYPE_WATSON_MINI:
        case DEVICE_TYPE_CURVE:
        case DEVICE_TYPE_CURVE_SETI:
        case DEVICE_TYPE_COLUMBO:
        case DEVICE_TYPE_HOLMES:
            break;
        case DEVICE_TYPE_SHERLOCK_ROIC:
        case DEVICE_TYPE_SHERLOCK:
            m_propertyInfo.nLEOperationMode = ADDRESS_LE_ON_MODE;
            _SetLEOperationMode(m_propertyInfo.nLEOperationMode);
            break;
        default:
            break;
    }

    if (!_GetOneFrameImage(pRawImage, imageLength))
    {
        return IBSU_ERR_DEVICE_IO;
    }

    return IBSU_STATUS_OK;
}

int CMainCapture::Reserved_SetFpgaRegister(UCHAR address, UCHAR value)
{
    //	if( address < 0 || address > HOLMES_FPGA_EL_DELAY_TIME )
    //		return IBSU_ERR_INVALID_PARAM_VALUE;

    return _FPGA_SetRegister(address, value);
}

int CMainCapture::Reserved_GetFpgaRegister(UCHAR address, UCHAR *value)
{
    //	if( address < 0 || address > HOLMES_FPGA_EL_DELAY_TIME )
    //		return IBSU_ERR_INVALID_PARAM_VALUE;

    return _FPGA_GetRegister(address, value);
}

int CMainCapture::Reserved_DrawClientWindow(unsigned char *drawImage)
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
	_DrawClientWindow(drawImage, m_DisplayWindow, &dstOverlayText, &dstOverlayTextEx, &dstOverlayLineEx, &dstOverlayQuadrangleEx, &dstOverlayShapeEx);

    return IBSU_STATUS_OK;
}

int CMainCapture::Reserved_UsbBulkOutIn(int outEp, UCHAR uiCommand, UCHAR *outData, LONG outDataLen,
                                        int inEp, UCHAR *inData, LONG inDataLen, LONG *nBytesRead)
{
    return _UsbBulkOutIn(outEp, uiCommand, outData, outDataLen, inEp, inData, inDataLen, nBytesRead);
}

int CMainCapture::Reserved_InitializeCamera()
{
    if (_InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart,
        m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight) != IBSU_STATUS_OK)
    {
        return IBSU_ERR_NOT_SUPPORTED;
    }

    return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::Reserved_InitializeCameraFlatForKOJAK()
{
    if (_InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart,
        m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight) != IBSU_STATUS_OK)
    {
        return IBSU_ERR_NOT_SUPPORTED;
    }

    return IBSU_ERR_NOT_SUPPORTED;
}

int CMainCapture::Reserved_InitializeCameraRollForKOJAK()
{
    if (_InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart,
        m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight) != IBSU_STATUS_OK)
    {
        return IBSU_ERR_NOT_SUPPORTED;
    }

    return IBSU_ERR_NOT_SUPPORTED;
}

int	CMainCapture::Reserved_GetEnhancedImage(const IBSU_ImageData inImage,
										   IBSU_ImageData *enhancedImage,
										   int *segmentImageArrayCount,
										   IBSU_ImageData *segmentImageArray,
										   IBSU_SegmentPosition *segmentPositionArray)
{
    memset(enhancedImage, 0, sizeof(IBSU_ImageData));
	*segmentImageArrayCount = 0;
    memset(segmentImageArray, 0xFF, sizeof(IBSU_ImageData)*IBSU_MAX_SEGMENT_COUNT);
    memset(segmentPositionArray, 0, sizeof(IBSU_SegmentPosition)*IBSU_MAX_SEGMENT_COUNT);

	if (m_propertyInfo.bEnableDecimation)
		return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;

    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
        case DEVICE_TYPE_WATSON_MINI:
			return Reserved_GetEnhancdImage_Watson(inImage, enhancedImage, segmentImageArrayCount, segmentImageArray, segmentPositionArray);
        case DEVICE_TYPE_SHERLOCK:
        case DEVICE_TYPE_SHERLOCK_ROIC:
			return Reserved_GetEnhancdImage_Sherlock(inImage, enhancedImage, segmentImageArrayCount, segmentImageArray, segmentPositionArray);
        case DEVICE_TYPE_COLUMBO:
			return Reserved_GetEnhancdImage_Columbo(inImage, enhancedImage, segmentImageArrayCount, segmentImageArray, segmentPositionArray);
        case DEVICE_TYPE_HOLMES:
        case DEVICE_TYPE_CURVE:
		case DEVICE_TYPE_CURVE_SETI:
		default:
            return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
    }

	return IBSU_STATUS_OK;
}

int	CMainCapture::Reserved_GetFinalImageByNative(IBSU_ImageData *finalImage)
{
    switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_COLUMBO:
			return Reserved_GetFinalImageByNative_Columbo(finalImage);
		case DEVICE_TYPE_KOJAK:
			return Reserved_GetFinalImageByNative_Kojak(finalImage);
		default:
            return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
    }

	return IBSU_STATUS_OK;
}

int	CMainCapture::Reserved_CheckWetFinger(const IBSU_ImageData inImage)
{
	int WetRatio = 0;
	
	if(inImage.Width <= 0 || inImage.Height <= 0)
		return IBSU_ERR_INVALID_PARAM_VALUE;

	if(m_propertyInfo.bWetFingerDetect == FALSE)
		return IBSU_ERR_NOT_SUPPORTED;

	unsigned char *TmpBuffer = new unsigned char[inImage.Width*inImage.Height];

	for(int i=0; i<(int)(inImage.Width*inImage.Height); i++)
	{
		TmpBuffer[i] = 255 - ((unsigned char*)inImage.Buffer)[i];
	}

	int nRc = m_pAlgo->_Algo_AnalysisBlockStd_forWet(TmpBuffer, inImage.Width, inImage.Height, 0, &WetRatio);

#ifdef __G_DEBUG__
	int wet_threshold = 100;
	if(m_propertyInfo.nWetFingerDetectLevel == 1)
		wet_threshold = 15;
	else if(m_propertyInfo.nWetFingerDetectLevel == 2)
		wet_threshold = 12;
	else if(m_propertyInfo.nWetFingerDetectLevel == 3)
		wet_threshold = 9;
	else if(m_propertyInfo.nWetFingerDetectLevel == 4)
		wet_threshold = 6;
	else if(m_propertyInfo.nWetFingerDetectLevel == 5)
		wet_threshold = 3;
	else
		wet_threshold = 100;

	TRACE("Reserved_CheckWetFinger (nRc : %d) (WetRatio : %d, threshold : %d)\n", nRc, WetRatio, wet_threshold); 
#endif

	delete [] TmpBuffer;

	return nRc;
}

int	CMainCapture::Reserved_GetImageWidth(const BYTE *inImage, const int width, const int height, int *outMM)
{
	int nRc = IBSU_STATUS_OK;

	*outMM = 0;
	
	if(width <= 0 || height <= 0)
		return IBSU_ERR_INVALID_PARAM_VALUE;

	int i, j;
	int left=width, right=0;
	int thres = 10;
	int *mean_h = new int [width];
	memset(mean_h, 0, width*sizeof(int));

	thres = 10;//_NO_BIT_CMOS_NOISE_FOR_FULLFRAME_ROLL-5;//m_SBDAlg->m_Roll_Minus_Val;

	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			mean_h[j] += (255-inImage[i*width+j]);
		}
	}
	
	for(j=0; j<width; j++)
		mean_h[j] /= height;

	for(j=0; j<width; j++)
	{
		if(mean_h[j] > thres)
		{
			left=j;
			break;
		}
	}

	for(j=width-1; j>=0; j--)
	{
		if(mean_h[j] > thres)
		{
			right=j;
			break;
		}
	}

	delete [] mean_h;

	if(left >= right)
		return IBSU_ERR_COMMAND_FAILED;

	*outMM = (int)((right-left) * 50.8 / 1000);

	return nRc;
}

int	CMainCapture::Reserved_WriteMask(const BYTE *MaskData, const int MaskDataLen, int *pProgress)
{
    LONG /*lSize, */ writecnt = 3;
    unsigned char packet_buf[16];
    int			nRc = IBSU_STATUS_OK;
	int			i = 0;
	unsigned char send_data[514];


	int page_count = MaskDataLen / 511;
	
	*pProgress = 0;

	for ( i=0; i<page_count; i++)
	{
		memset(send_data, 0, 514);
        memcpy(&send_data[2], &MaskData[i*511], 511);
		send_data[2+511] = _CalcCheckSum(&send_data[2], 511);

		packet_buf[0] = 0x2B;
		packet_buf[1] = 0x01;
		packet_buf[2] = (i%2);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x29;
		packet_buf[1] = 0x01;
		packet_buf[2] = ((i>>8)&0xFF);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x2A;
		packet_buf[1] = 0x01;
		packet_buf[2] = (i&0xFF);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

        // 20151204 enzyme - bug need to check with Micky
//		nRc = m_pUsbDevice->WritePipe(EP2OUT, send_data, 514, &cbSent, timeout);
        nRc = _UsbBulkOutIn(EP2OUT, 0x00, (UCHAR *)&send_data[1], 514-1, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x27;
		packet_buf[1] = 0x01;
		packet_buf[2] = 0x01;
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x27;
		packet_buf[1] = 0x01;
		packet_buf[2] = (0x00);
//		nRc = m_pUsbDevice->WritePipe(EP1OUT, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

//		TRACE("mask write : %d, check_sum : %d\n", i,send_data[2+511]);
//		Sleep(41);
		Sleep(18);
		
		*pProgress = i * 100 / page_count;
	}

	if(page_count*511 < MaskDataLen)
	{
		int remain_size = MaskDataLen%511;

		memset(send_data, 0, 514);
		memcpy(&send_data[2], &MaskData[i*511], remain_size);
		send_data[2+511] = _CalcCheckSum(&send_data[2], 511);

		packet_buf[0] = 0x2B;
		packet_buf[1] = 0x01;
		packet_buf[2] = (i%2);
//		nRc = m_pUsbDevice->WritePipe(EP1OUT, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x29;
		packet_buf[1] = 0x01;
		packet_buf[2] = ((i>>8)&0xFF);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x2A;
		packet_buf[1] = 0x01;
		packet_buf[2] = (i&0xFF);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

//		nRc = m_pUsbDevice->WritePipe(EP2OUT, send_data, 514, &cbSent, timeout);
        nRc = _UsbBulkOutIn(EP2OUT, 0x00, (UCHAR *)&send_data[1], 514-1, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x27;
		packet_buf[1] = 0x01;
		packet_buf[2] = 0x01;
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x27;
		packet_buf[1] = 0x01;
		packet_buf[2] = (0x00);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

//		TRACE("mask write : %d, check_sum : %d\n", i,send_data[2+511]);
	}

release:
	*pProgress = 100;

	return nRc;
}

int	CMainCapture::Reserved_WriteMask_8M(const BYTE *MaskData, const int MaskDataLen, const int section, int *pProgress)
{
    LONG /*lSize, */writecnt = 3;
    unsigned char packet_buf[16];
    int			nRc = IBSU_STATUS_OK;
	int			i = 0;
	unsigned char send_data[514];

	int page_count = MaskDataLen / 511;
	

	*pProgress = 0;

	// section select
	_FPGA_SetRegister(0x33, section);

	for ( i=0; i<page_count; i++)
	{
		memset(send_data, 0, 514);
		memcpy(&send_data[2], &MaskData[i*511], 511);
		send_data[2+511] = _CalcCheckSum(&send_data[2], 511);

		packet_buf[0] = 0x2B;
		packet_buf[1] = 0x01;
		packet_buf[2] = (i%2);
//		nRc = m_pUsbDevice->WritePipe(EP1OUT, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x29;
		packet_buf[1] = 0x01;
		packet_buf[2] = ((i>>8)&0xFF);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x2A;
		packet_buf[1] = 0x01;
		packet_buf[2] = (i&0xFF);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

//		nRc = m_pUsbDevice->WritePipe(EP2OUT, send_data, 514, &cbSent, timeout);
        nRc = _UsbBulkOutIn(EP2OUT, 0x00, (UCHAR *)&send_data[1], 514-1, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x27;
		packet_buf[1] = 0x01;
		packet_buf[2] = 0x01;
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x27;
		packet_buf[1] = 0x01;
		packet_buf[2] = (0x00);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

//		TRACE("mask write : %d, check_sum : %d\n", i,send_data[2+511]);
//		Sleep(18);
		Sleep(10);
		*pProgress = i * 100 / page_count;
	}

	if(page_count*511 < MaskDataLen)
	{
		int remain_size = MaskDataLen%511;

		memset(send_data, 0, 514);
		memcpy(&send_data[2], &MaskData[i*511], remain_size);
		send_data[2+511] = _CalcCheckSum(&send_data[2], 511);

		packet_buf[0] = 0x2B;
		packet_buf[1] = 0x01;
		packet_buf[2] = (i%2);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x29;
		packet_buf[1] = 0x01;
		packet_buf[2] = ((i>>8)&0xFF);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x2A;
		packet_buf[1] = 0x01;
		packet_buf[2] = (i&0xFF);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

//		nRc = m_pUsbDevice->WritePipe(EP2OUT, send_data, 514, &cbSent, timeout);
        nRc = _UsbBulkOutIn(EP2OUT, 0x00, (UCHAR *)&send_data[1], 514-1, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x27;
		packet_buf[1] = 0x01;
		packet_buf[2] = 0x01;
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		packet_buf[0] = 0x27;
		packet_buf[1] = 0x01;
		packet_buf[2] = (0x00);
        nRc = _UsbBulkOutIn(EP1OUT, 0x06, (UCHAR *)packet_buf, writecnt, -1, NULL, 0, NULL);
		if(nRc != IBSU_STATUS_OK)
			goto release;

//		TRACE("mask write : %d, check_sum : %d\n", i,send_data[2+511]);
	}

release:
	*pProgress = 100;

	return nRc;
}

int	CMainCapture::Reserved_ResetDevice()
{
	_SndUsbFwCaptureStop();
	Sleep(100);
    _SndUsbFwCaptureStart();
	Sleep(100);
    _InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
	return IBSU_STATUS_OK;
}

void CMainCapture::TFT_MasterSlaveDeviation(unsigned char *InImg, double *Bright)
{
    int IMG_TEST_AREA_LEFT	=	0;
    int IMG_TEST_AREA_RIGHT	= m_UsbDeviceInfo.CisImgWidth - 0;	//(IMG_W-0)
    int IMG_TEST_AREA_TOP	=	0;
    int IMG_TEST_AREA_BOTTOM =	m_UsbDeviceInfo.CisImgHeight - 0; //(IMG_H-0)
    int IMG_WHITE_CHARGE	=	10;

    int MASTER_ASIC_END_H	 =	383;
    int SLAVE_ASIC_START_H	=	MASTER_ASIC_END_H + 1;

    int i, j;
    int Sum;
    int LEFT1, RIGHT1, TOP1, BOTTOM1;
    Bright[0] = 0.0;
    Bright[1] = 0.0;

    LEFT1 = IMG_TEST_AREA_LEFT;
    RIGHT1 = IMG_TEST_AREA_RIGHT;
/*	// assuming signed overflow does not occur when assuming that (X - c) <= X is always true
    if (RIGHT1 > m_UsbDeviceInfo.CisImgWidth - IMG_WHITE_CHARGE)
    {
        RIGHT1 = m_UsbDeviceInfo.CisImgWidth - IMG_WHITE_CHARGE;
    }
*/
	RIGHT1 = m_UsbDeviceInfo.CisImgWidth - IMG_WHITE_CHARGE;
	TOP1 = IMG_TEST_AREA_TOP;
    BOTTOM1 = IMG_TEST_AREA_BOTTOM;

    Sum = 0;

    for (i = TOP1; i < BOTTOM1; i++)
    {
        for (j = LEFT1; j < RIGHT1; j++)
        {
            Sum += InImg[i * m_UsbDeviceInfo.CisImgWidth + j];
        }
    }

    // MASTER_ASIC
    for (i = TOP1; i <= MASTER_ASIC_END_H; i++)
    {
        for (j = LEFT1; j < RIGHT1; j++)
        {
            Bright[0] += InImg[i * m_UsbDeviceInfo.CisImgWidth + j];
        }
    }

    // SLAVE_ASIC
    for (i = SLAVE_ASIC_START_H; i < BOTTOM1; i++)
    {
        for (j = LEFT1; j < RIGHT1; j++)
        {
            Bright[1] += InImg[i * m_UsbDeviceInfo.CisImgWidth + j];
        }
    }

    Bright[0] /= (double)(MASTER_ASIC_END_H - TOP1 + 1) * (RIGHT1 - LEFT1);
    Bright[1] /= (double)(BOTTOM1 - SLAVE_ASIC_START_H) * (RIGHT1 - LEFT1);

    return;
}

int CMainCapture::_ReadMaskFromEP6IN_Columbo(unsigned char *mask_buf, int mask_buf_size)
{
    long          page_count;
    int           remained_buf_size;
    unsigned char checksum;
    UCHAR         *readBuffer;
    unsigned char *pMaskImg;

    readBuffer = new UCHAR[m_UsbDeviceInfo.nWaterImageSize];
#ifdef WINCE
	if( readBuffer == NULL ){
		DebugBreak();
	}
#endif
	memset(readBuffer, 0, m_UsbDeviceInfo.nWaterImageSize);

    int   nRc = -1;
    UCHAR bData[MAX_DATA_BYTES + 1] = {0};
    long  i = 0;
    LONG  lActualBytes = 0;

    page_count = mask_buf_size / (UM_ONE_PAGE_SIZE - 1);
    remained_buf_size = mask_buf_size - page_count * (UM_ONE_PAGE_SIZE - 1);

    /* Start reading mask. */
    nRc = _UsbBulkOutIn(EP1OUT, CMD_START_READ_MASK, NULL, 0, -1, NULL, 0, NULL);
    if (nRc != IBSU_STATUS_OK)
    {
        goto error;
    }

    /* Read entire mask. */
//    nRc = _UsbBulkIn(EP6IN, readBuffer, m_UsbDeviceInfo.nWaterImageSize, &lActualBytes);
#if defined(__ppi__)
    if (m_bPPIMode)
    {
        nRc = _PPI_MaskBulkRead(readBuffer, m_UsbDeviceInfo.nWaterImageSize, &lActualBytes);
    }
    else
    {
        nRc = _UsbBulkOutIn(-1, 0x00, NULL, 0, EP6IN, readBuffer, m_UsbDeviceInfo.nWaterImageSize, &lActualBytes);
    }
#else
    nRc = _UsbBulkOutIn(-1, 0x00, NULL, 0, EP6IN, readBuffer, m_UsbDeviceInfo.nWaterImageSize, &lActualBytes);
#endif
    if ((nRc != IBSU_STATUS_OK) || (lActualBytes != m_UsbDeviceInfo.nWaterImageSize))
    {
//        printf("[%d] Error was occurred in _PPI_MaskBulkRead (%d = %d)\n", nRc, lActualBytes, m_UsbDeviceInfo.nWaterImageSize);
        goto error;
    }

    /* Stop reading mask. */
    nRc = _UsbBulkOutIn(EP1OUT, CMD_STOP_READ_MASK, NULL, 0, EP1IN, &bData[0], 1, NULL);
    if (nRc != IBSU_STATUS_OK)
    {
        goto error;
    }

    /* Reset FIFO. */
    nRc = _UsbBulkOutIn(EP1OUT, CMD_RESET_FIFO, NULL, 0, -1, NULL, 0, NULL);
    if (nRc != IBSU_STATUS_OK)
    {
        goto error;
    }

    pMaskImg = &readBuffer[4];		// 4bytes is dummy data, probably it is firmware bug...

	// skip as columbo pi header
	if(m_propertyInfo.bColumboBaordforPI == TRUE)
	{
		pMaskImg = &readBuffer[4 + UM_MAX_PAGE_SIZE];
		page_count = page_count - 1;
	}

    for (i = 0; i < page_count; i++)
    {
        checksum = _UM_MAKE_CHECKSUM(&pMaskImg[i * (UM_MAX_PAGE_SIZE)], (UM_ONE_PAGE_SIZE - 1));
        if (checksum != pMaskImg[(i * (UM_MAX_PAGE_SIZE)) + (UM_ONE_PAGE_SIZE - 1)])
        {
//            printf("Checksum error was occurred (%d/%d)\n", i, page_count);
            goto error;
        }
        memcpy(&mask_buf[i * (UM_ONE_PAGE_SIZE - 1)], &pMaskImg[i * (UM_MAX_PAGE_SIZE)], (UM_ONE_PAGE_SIZE - 1));
    }

    if (remained_buf_size > 0)
    {
#if !defined(__ppi__)
        checksum = _UM_MAKE_CHECKSUM(&pMaskImg[i * (UM_MAX_PAGE_SIZE)], (UM_ONE_PAGE_SIZE - 1));
        if (checksum != pMaskImg[(i * (UM_MAX_PAGE_SIZE)) + (UM_ONE_PAGE_SIZE - 1)])
        {
//            printf("Checksum error was occurred (%d/%d)\n", i, page_count);
            goto error;
        }
#endif
        memcpy(&mask_buf[i * (UM_ONE_PAGE_SIZE - 1)], &pMaskImg[i * (UM_MAX_PAGE_SIZE)], remained_buf_size);
    }

    delete [] readBuffer;
    return IBSU_STATUS_OK;

error:
//    printf("[%d] Error was occurred in _PPI_MaskBulkRead ()\n");
    delete [] readBuffer;
    return IBSU_ERR_COMMAND_FAILED;
}

int CMainCapture::_ReadMaskFromEP8IN_Kojak(unsigned char *mask_buf, int mask_buf_size)
{
#define FLASH_WRITE_START		0x27
#define FLASH_READ_START		0x28
#define FLASH_PAGE_NUM_HIGH		0x29
#define FLASH_PAGE_NUM_LOW		0x2A
#define FLASH_BANK_SELECT		0x2B
#define FLASH_READ_BYTE_LOW		0x2C
#define FLASH_READ_BYTE_MIDDLE	0x2D
#define FLASH_READ_BYTE_HIGH	0x2E
#define FLASH_READ_EP_OFF		0x2F
#define FLASH_PAGE_SIZE			528

    long          page_count;
    int           remained_buf_size;
    unsigned char checksum;
    UCHAR         *readBuffer;
    unsigned char *pMaskImg;

    readBuffer = new UCHAR[m_UsbDeviceInfo.nWaterImageSize];
#ifdef WINCE
	if( readBuffer == NULL ){
		DebugBreak();
	}
#endif
	memset(readBuffer, 0, m_UsbDeviceInfo.nWaterImageSize);

    int   nRc = -1;
    long  i = 0;
    LONG  lActualBytes = 0;

    page_count = mask_buf_size / (UM_ONE_PAGE_SIZE - 1);
    remained_buf_size = mask_buf_size - page_count * (UM_ONE_PAGE_SIZE - 1);
	
	if ((nRc=_FPGA_SetRegister(FLASH_READ_EP_OFF, 1)) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_EP_OFF, 1) : %d\n"), nRc);
#endif
        goto done;
    }
	
	_ResetFifo();

	if ((nRc=_FPGA_SetRegister(FLASH_READ_BYTE_HIGH, ((m_UsbDeviceInfo.nWaterImageSize>>16)&0xFF))) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_BYTE_HIGH) : %d\n"), nRc);
#endif
        goto done;
    }
	
	if ((nRc=_FPGA_SetRegister(FLASH_READ_BYTE_MIDDLE, ((m_UsbDeviceInfo.nWaterImageSize>>8)&0xFF))) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_BYTE_MIDDLE) : %d\n"), nRc);
#endif
        goto done;
    }
	
	if ((nRc=_FPGA_SetRegister(FLASH_READ_BYTE_LOW, (m_UsbDeviceInfo.nWaterImageSize&0xFF))) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_BYTE_LOW) : %d\n"), nRc);
#endif
        goto done;
    }

	if ((nRc=_FPGA_SetRegister(FLASH_READ_START, 1)) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_START) : %d\n"), nRc);
#endif
        goto done;
    }

    if ((nRc = _UsbBulkIn(EP8IN, readBuffer, m_UsbDeviceInfo.nWaterImageSize, &lActualBytes)) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
 	    TRACE(_T("_UsbBulkIn() : lActualBytes %d, nRc %d\n"), lActualBytes, nRc);
#endif
        goto done;
    }
#ifdef __G_DEBUG__
    TRACE(_T("lActualBytes : %d\n"), lActualBytes);
#endif
	
	if ((nRc=_FPGA_SetRegister(FLASH_READ_EP_OFF, 0)) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
 	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_EP_OFF) : %d\n"), nRc);
#endif
        goto done;
    }
	
    pMaskImg = &readBuffer[0];

    for (i = 0; i < page_count; i++)
    {
        checksum = _UM_MAKE_CHECKSUM(&pMaskImg[i * (UM_MAX_PAGE_SIZE)], (UM_ONE_PAGE_SIZE - 1));
        if (checksum != pMaskImg[(i * (UM_MAX_PAGE_SIZE)) + (UM_ONE_PAGE_SIZE - 1)])
        {
			nRc = IBSU_ERR_CHANNEL_IO_READ_FAILED;
            goto done;
        }
        memcpy(&mask_buf[i * (UM_ONE_PAGE_SIZE - 1)], &pMaskImg[i * (UM_MAX_PAGE_SIZE)], (UM_ONE_PAGE_SIZE - 1));
    }

    if (remained_buf_size > 0)
    {
        checksum = _UM_MAKE_CHECKSUM(&pMaskImg[i * (UM_MAX_PAGE_SIZE)], (UM_ONE_PAGE_SIZE - 1));
        if (checksum != pMaskImg[(i * (UM_MAX_PAGE_SIZE)) + (UM_ONE_PAGE_SIZE - 1)])
        {
			nRc = IBSU_ERR_CHANNEL_IO_READ_FAILED;
            goto done;
        }
        memcpy(&mask_buf[i * (UM_ONE_PAGE_SIZE - 1)], &pMaskImg[i * (UM_MAX_PAGE_SIZE)], remained_buf_size);
    }

	delete [] readBuffer;
    return IBSU_STATUS_OK;

done:
	delete [] readBuffer;
    return nRc;
}

int CMainCapture::_ReadMaskFromEP8IN_Kojak_8M(unsigned char *mask_buf, int mask_buf_size, const int section)
{
#define FLASH_WRITE_START		0x27
#define FLASH_READ_START		0x28
#define FLASH_PAGE_NUM_HIGH		0x29
#define FLASH_PAGE_NUM_LOW		0x2A
#define FLASH_BANK_SELECT		0x2B
#define FLASH_READ_BYTE_LOW		0x2C
#define FLASH_READ_BYTE_MIDDLE	0x2D
#define FLASH_READ_BYTE_HIGH	0x2E
#define FLASH_READ_EP_OFF		0x2F
#define FLASH_PAGE_SIZE			528

    long          page_count;
    int           remained_buf_size;
    unsigned char checksum;
    UCHAR         *readBuffer;
    unsigned char *pMaskImg;
	int read_size = m_UsbDeviceInfo.nWaterImageSize;

    readBuffer = new UCHAR[read_size];
#ifdef WINCE
	if( readBuffer == NULL ){
		DebugBreak();
	}
#endif
	memset(readBuffer, 0, read_size);

    int   nRc = -1;
    long  i = 0;
    LONG  lActualBytes = 0;

	// section select
	if ((nRc=_FPGA_SetRegister(0x33, section)) != IBSU_STATUS_OK)
    {
        goto done;
    }

    page_count = mask_buf_size / (UM_ONE_PAGE_SIZE - 1);
    remained_buf_size = mask_buf_size - page_count * (UM_ONE_PAGE_SIZE - 1);
	
	if ((nRc=_FPGA_SetRegister(FLASH_READ_EP_OFF, 1)) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_EP_OFF, 1) : %d\n"), nRc);
#endif
        goto done;
    }
	
	_ResetFifo();

	if ((nRc=_FPGA_SetRegister(FLASH_READ_BYTE_HIGH, ((read_size>>16)&0xFF))) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_BYTE_HIGH) : %d\n"), nRc);
#endif
        goto done;
    }
	
	if ((nRc=_FPGA_SetRegister(FLASH_READ_BYTE_MIDDLE, ((read_size>>8)&0xFF))) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_BYTE_MIDDLE) : %d\n"), nRc);
#endif
        goto done;
    }
	
	if ((nRc=_FPGA_SetRegister(FLASH_READ_BYTE_LOW, (read_size&0xFF))) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_BYTE_LOW) : %d\n"), nRc);
#endif
        goto done;
    }

	if ((nRc=_FPGA_SetRegister(FLASH_READ_START, 1)) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_START) : %d\n"), nRc);
#endif
        goto done;
    }

	if(read_size > 1024*1024)
	{
		int remained_size = read_size;
		for(int i=0; remained_size > 0; i++)
		{
			if(remained_size > 1024*1024)
			{
				if ((nRc = _UsbBulkOutIn(-1, 0x00, NULL, 0, EP8IN, &readBuffer[i*1024*1024], 1024*1024, &lActualBytes)) != IBSU_STATUS_OK)
				{
#ifdef __G_DEBUG__
 					TRACE(_T("_UsbBulkIn() : lActualBytes %d, nRc %d\n"), lActualBytes, nRc);
#endif
					goto done;
				}
			}
			else
			{
				if ((nRc = _UsbBulkOutIn(-1, 0x00, NULL, 0, EP8IN, &readBuffer[i*1024*1024], remained_size, &lActualBytes)) != IBSU_STATUS_OK)
				{
#ifdef __G_DEBUG__
 					TRACE(_T("_UsbBulkIn() : lActualBytes %d, nRc %d\n"), lActualBytes, nRc);
#endif
					goto done;
				}
			}
			remained_size = remained_size - lActualBytes;
		}
#ifdef __G_DEBUG__
	    TRACE(_T("lActualBytes : %d\n"), lActualBytes);
#endif
	}
	else
	{
		if ((nRc = _UsbBulkOutIn(-1, 0x00, NULL, 0, EP8IN, readBuffer, read_size, &lActualBytes)) != IBSU_STATUS_OK)
		{
#ifdef __G_DEBUG__
 			TRACE(_T("_UsbBulkIn() : lActualBytes %d, nRc %d\n"), lActualBytes, nRc);
#endif
			goto done;
		}
#ifdef __G_DEBUG__
	    TRACE(_T("lActualBytes : %d\n"), lActualBytes);
#endif
	}
	
	if ((nRc=_FPGA_SetRegister(FLASH_READ_EP_OFF, 0)) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
 	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_EP_OFF) : %d\n"), nRc);
#endif
        goto done;
    }
	
    pMaskImg = &readBuffer[0];

    for (i = 0; i < page_count; i++)
    {
        checksum = _UM_MAKE_CHECKSUM(&pMaskImg[i * (UM_MAX_PAGE_SIZE)], (UM_ONE_PAGE_SIZE - 1));
        if (checksum != pMaskImg[(i * (UM_MAX_PAGE_SIZE)) + (UM_ONE_PAGE_SIZE - 1)])
        {
			nRc = IBSU_ERR_CHANNEL_IO_READ_FAILED;
            goto done;
        }
        memcpy(&mask_buf[i * (UM_ONE_PAGE_SIZE - 1)], &pMaskImg[i * (UM_MAX_PAGE_SIZE)], (UM_ONE_PAGE_SIZE - 1));
    }

    if (remained_buf_size > 0)
    {
        checksum = _UM_MAKE_CHECKSUM(&pMaskImg[i * (UM_MAX_PAGE_SIZE)], (UM_ONE_PAGE_SIZE - 1));
        if (checksum != pMaskImg[(i * (UM_MAX_PAGE_SIZE)) + (UM_ONE_PAGE_SIZE - 1)])
        {
			nRc = IBSU_ERR_CHANNEL_IO_READ_FAILED;
            goto done;
        }
        memcpy(&mask_buf[i * (UM_ONE_PAGE_SIZE - 1)], &pMaskImg[i * (UM_MAX_PAGE_SIZE)], remained_buf_size);
    }

	delete [] readBuffer;
    return IBSU_STATUS_OK;

done:
	delete [] readBuffer;
    return nRc;
}

unsigned char CMainCapture::_CalcCheckSum(unsigned char *data, int data_count)
{
	unsigned char CS=0;

	for(int i=0; i<data_count; i++)
		CS += data[i];

	return CS;
}

int CMainCapture::_GetFpgaVersion(int Extension)		// if Extension = TRUE, 4 bytes firmware version is returned.
{
    // This can create a long initialization time in Curve
    // So it should be skipped
    if (_GetPID() == __PID_CURVE__)
    {
        return 0;
    }

    UCHAR fpgaVer[4] = {0};
    int	  fpgaVersion = 0;//, eepromVersion = 0;
    int	  nRc;

    nRc = _FPGA_GetRegister(0xF0, &fpgaVer[0], 500);
    if (nRc == IBSU_STATUS_OK)
    {
        nRc = _FPGA_GetRegister(0xF1, &fpgaVer[1]);

        if (nRc == IBSU_STATUS_OK)
        {
            nRc = _FPGA_GetRegister(0xF2, &fpgaVer[2]);

			if (nRc == IBSU_STATUS_OK && Extension)
			{
				nRc = _FPGA_GetRegister(0xF3, &fpgaVer[3]);
			}
        }
    }

    if (nRc == IBSU_STATUS_OK)
    {
		if(Extension)
//			fpgaVersion = ((int)fpgaVer[0] << 24) | ((int)fpgaVer[1] << 16) | ((int)fpgaVer[2] << 8) | ((int)fpgaVer[3]);
			fpgaVersion = (int)((fpgaVer[0] << 24) | (fpgaVer[1] << 16) | (fpgaVer[2] << 8) | (fpgaVer[3]));
		else
//            fpgaVersion = ((int)fpgaVer[0] << 16) | ((int)fpgaVer[1] << 8) | ((int)fpgaVer[2]);
            fpgaVersion = (int)((fpgaVer[0] << 16) | (fpgaVer[1] << 8) | (fpgaVer[2]));
    }
    else
    {
        fpgaVersion = 0;
    }

	/* get FPGA version from eeprom
	if(m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON_MINI &&
		fpgaVersion >= _FPGA_VER_FOR_WATSON_MINI_WRITE_EEPROM_FOR_VER_)
	{
		// for debugging
		_WriteVersionToEEPROM("1.1.3");

		if(_ReadVersionFromEEPROM(&eepromVersion) == IBSU_STATUS_OK)
		{
			fpgaVersion = eepromVersion;
		}
		else
		{
			fpgaVersion = 0;
		}
	}*/

    return fpgaVersion;
}

int CMainCapture::_ReadVersionFromEEPROM(int *eepromVersion)
{
	int nRc = IBSU_STATUS_OK;
	char propertyValue[256];
	EEPROM_Ver eeprom_ver;

	memset(&eeprom_ver, 0, sizeof(EEPROM_Ver));
	memset(propertyValue, 0, 64);

	*eepromVersion = 0;

	// StartFlag(0xF1) / Length(1byte) / StructVer(1bytes) / Data(?bytes) / EndFlag(0xF2)

	nRc = _GetProperty(PROPERTY_IBIA_VERSION, (BYTE *)propertyValue);
		
	if(nRc == IBSU_STATUS_OK)
	{
		eeprom_ver.StartFlag = (unsigned char)propertyValue[0];
		eeprom_ver.Length = (unsigned char)propertyValue[1];
		eeprom_ver.StructVer = (unsigned char)propertyValue[2];
		eeprom_ver.EndFlag = (unsigned char)propertyValue[eeprom_ver.Length+1];
		
		if(eeprom_ver.StartFlag != 0xF1 || eeprom_ver.Length == 0 || eeprom_ver.EndFlag != 0xF2 )
		{
			//TRACE("\n==> Failed to _ReadVersionFromEEPROM()\n");
			return IBSU_ERR_CHANNEL_IO_READ_FAILED;
		}
		
		memcpy(eeprom_ver.Data, (unsigned char*)&propertyValue[3], eeprom_ver.Length-2);

		*eepromVersion = (eeprom_ver.Data[0]<<16) | (eeprom_ver.Data[1]<<8) | (eeprom_ver.Data[2]);

		//TRACE("\n==> _ReadVersionFromEEPROM() : %X\n", *eepromVersion);
	}

	return nRc;
}

int CMainCapture::_WriteVersionToEEPROM(char *FPGAVersion)
{
	EEPROM_Ver eeprom_ver;
	BYTE WriteBuf[64];
	char string_tmp[64];
	int i = 0;
	memset(&eeprom_ver, 0, sizeof(EEPROM_Ver));
	memset(WriteBuf, 0, 64);

	eeprom_ver.StartFlag = 0xF1;
	eeprom_ver.EndFlag = 0xF2;
	eeprom_ver.StructVer = 0x01;

	sprintf(string_tmp, "%s", FPGAVersion);
	char *token = strtok(string_tmp, ".");
	while ( token )
	{
		//TRACE("%d TOKEN %s:\n", i, token);
		eeprom_ver.Data[i] = atoi(token);

		token = strtok(NULL, (const char*)".");
		i++;
	}

	eeprom_ver.Length = i + 2;

	WriteBuf[0] = eeprom_ver.StartFlag;
	WriteBuf[1] = eeprom_ver.Length;
	WriteBuf[2] = eeprom_ver.StructVer;

	for(i=0; i<eeprom_ver.Length-2; i++)
	{
		WriteBuf[i+3] = eeprom_ver.Data[i];
	}
	WriteBuf[i+3] = eeprom_ver.EndFlag;

	if(_SetProperty(PROPERTY_IBIA_VERSION, WriteBuf, eeprom_ver.Length+2) != IBSU_STATUS_OK)
	{
		return IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
	}

	return IBSU_STATUS_OK;
}

void CMainCapture::_UpdateGammaLevel(int maxAutoContrastValue)
{
    int prevGammaLevel;

    prevGammaLevel = m_nGammaLevel;

    if (m_pAlgo->m_cImgAnalysis.mean > 0)
    {
        m_nGammaLevel = 12 - (m_pAlgo->m_cImgAnalysis.mean - 80) / 3 + m_propertyInfo.nContrastTres * 2;
    }
    else
    {
        m_nGammaLevel = 12;
    }

    if (m_propertyInfo.nContrastValue >= maxAutoContrastValue && m_pAlgo->m_cImgAnalysis.max_same_gain_count == 60)
    {
        m_nGammaLevel -= 5;
    }

    if (m_nGammaLevel < 0)
    {
        m_nGammaLevel = 0;
    }
    else if (m_nGammaLevel > 47)
    {
        m_nGammaLevel = 47;
    }

    m_nGammaLevel = (m_nGammaLevel + prevGammaLevel + 1) / 2;
}

int CMainCapture::_IsNeedInitializeCIS()
{
	switch (m_UsbDeviceInfo.devType)
    {
        case DEVICE_TYPE_WATSON:
        case DEVICE_TYPE_WATSON_MINI:
            return _IsNeedInitializeCIS_Watson();
        case DEVICE_TYPE_SHERLOCK_ROIC:
        case DEVICE_TYPE_SHERLOCK:
            return _IsNeedInitializeCIS_Sherlock();
//        case DEVICE_TYPE_COLUMBO:
//            return _IsNeedInitializeCIS_Columbo();
        case DEVICE_TYPE_CURVE:
           	return _IsNeedInitializeCIS_Curve();
        case DEVICE_TYPE_CURVE_SETI:
			return _IsNeedInitializeCIS_CurveSETi();
        case DEVICE_TYPE_HOLMES:
            return _IsNeedInitializeCIS_Holmes();
		case DEVICE_TYPE_KOJAK:
            return _IsNeedInitializeCIS_Kojak();
		case DEVICE_TYPE_FIVE0:
            return _IsNeedInitializeCIS_Five0();
        default:
            return FALSE;
    }

	return FALSE;
}

int CMainCapture::_CaptureStart_for_Decimation()
{
	if (m_UsbDeviceInfo.devType == DEVICE_TYPE_COLUMBO)
	{
		if (m_UsbDeviceInfo.nFpgaVersion >= ((1 << 16) | (1 << 8) | 2))
		{
			m_UsbDeviceInfo.bDecimation = TRUE;

			if (m_UsbDeviceInfo.nUsbSpeed < USB_HIGH_SPEED)
			{
				// USB 1.x --> 1/16 decimation
				m_UsbDeviceInfo.nDecimation_Mode = DECIMATION_4X;
			}
			else
			{
				// USB 2.0 --> 1/4 decimation
				m_UsbDeviceInfo.nDecimation_Mode = DECIMATION_2X;
			}
		}

		_SetDecimation_Columbo(TRUE);
		_SetDecimationMode_Columbo(m_UsbDeviceInfo.nDecimation_Mode);
	}
	else if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)		// GON
	{
        if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
			m_FullCaptureSetting.forSuperDry_Mode = m_propertyInfo.nSuperDryMode;

			if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
			{
				m_FullCaptureSetting.DefaultMinDAC = _KOJAK30_DAC_FOR_MINIMUM_;
				m_FullCaptureSetting.DefaultMaxDAC = _KOJAK30_DAC_FOR_MAXIMUM_;
			}
			else
			{
				m_FullCaptureSetting.DefaultMinDAC = _DAC_FOR_MINIMUM_;
				m_FullCaptureSetting.DefaultMaxDAC = _DAC_FOR_MAXIMUM_;
			}
			m_FullCaptureSetting.DefaultBitshift = _NO_BIT_BITSHIFT_FOR_FULLFRAME_ROLL_;
			m_FullCaptureSetting.DefaultCutThres = _NO_BIT_CUTTHRES_FOR_FULLFRAME_ROLL_;
			m_FullCaptureSetting.Kojak30_PERIOD = _KOJAK30_PERIOD_ROLL_;
			m_FullCaptureSetting.Kojak30_ELON = _KOJAK30_DEFAULT_ELON_FOR_FULLFRAME_ROLL_;

			if(m_FullCaptureSetting.forSuperDry_Mode)
			{
				m_FullCaptureSetting.TargetMinBrightness = _TARGET_MIN_BRIGHTNESS_FOR_DRY_;
				m_FullCaptureSetting.TargetMaxBrightness = _TARGET_MAX_BRIGHTNESS_FOR_DRY_;
			}
			else
			{
				m_FullCaptureSetting.TargetMinBrightness = m_propertyInfo.nCaptureBrightThresRoll[0];
				m_FullCaptureSetting.TargetMaxBrightness = m_propertyInfo.nCaptureBrightThresRoll[1];
			}

			m_FullCaptureSetting.TargetFingerCount = 1;
			if(m_propertyInfo.nSuperDryMode)
			{
				m_FullCaptureSetting.DefaultDAC = _DAC_FOR_DRY_ONE_FINGER_;
			}
			else
			{
				if(m_propertyInfo.bEnableStartingVoltage == TRUE)
				{
					m_FullCaptureSetting.DefaultDAC = m_propertyInfo.nStartingVoltage;
				}
				else
				{
					if(m_UsbDeviceInfo.bCanUseTOF &&
						m_propertyInfo.bEnableTOF && 
						m_propertyInfo.bEnableTOFforROLL)
					{
						if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
							m_FullCaptureSetting.DefaultDAC = _KOJAK30_DAC_FOR_NORMAL_FINGER_WITH_TOF_;
						else
							m_FullCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FINGER_WITH_TOF_;
					}
					else if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_NEW_BRD_AND_DOUBLE_P_ ||
							m_propertyInfo.bKojakPLwithDPFilm == TRUE)
					{
						m_FullCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_OF_DOUBLE_P_;
					}
					else
					{
						m_FullCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_;
					}
				}
			}
			m_FullCaptureSetting.CurrentDAC = m_FullCaptureSetting.DefaultDAC;
			m_FullCaptureSetting.DefaultGain = _NO_BIT_GAIN_FOR_FULLFRAME_;
			m_FullCaptureSetting.CurrentGain = m_FullCaptureSetting.DefaultGain;
			if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
				//m_FullCaptureSetting.DefaultExposure = m_nATOF_EXPOSURE_ROLL;
				m_FullCaptureSetting.DefaultExposure = _KOJAK30_DEFAULT_EXPOSURE_FOR_FULLFRAME_ROLL_;
			else
				m_FullCaptureSetting.DefaultExposure = _DEFAULT_EXPOSURE_FOR_FULLFRAME_ROLL_;
			m_FullCaptureSetting.CurrentExposure = m_FullCaptureSetting.DefaultExposure;
			m_FullCaptureSetting.CurrentBitshift = m_FullCaptureSetting.DefaultBitshift;
			m_FullCaptureSetting.CurrentCutThres = m_FullCaptureSetting.DefaultCutThres;
			m_FullCaptureSetting.AdditionalFrame = 1;
			m_FullCaptureSetting.RollCaptureMode = TRUE;
			m_FullCaptureSetting.DetectionFrameCount = 0;

			m_DeciCaptureSetting = m_FullCaptureSetting;

			m_UsbDeviceInfo.bDecimation = FALSE;
			_ChangeDecimationModeForKojak(FALSE, &m_FullCaptureSetting, TRUE);

			m_CurrentCaptureDAC = m_FullCaptureSetting.CurrentDAC;
		}
		else if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
				m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
				m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS ||
				m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS )
		{
			m_DeciCaptureSetting.forSuperDry_Mode = m_propertyInfo.nSuperDryMode;

			if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
				m_DeciCaptureSetting.DefaultMinDAC = _KOJAK30_DAC_FOR_MINIMUM_;
			else
				m_DeciCaptureSetting.DefaultMinDAC = _DAC_FOR_MINIMUM_;
			m_DeciCaptureSetting.DefaultMaxDAC = _DAC_FOR_MAXIMUM_;
			m_DeciCaptureSetting.Kojak30_PERIOD = _KOJAK30_PERIOD_DECI_;
			m_DeciCaptureSetting.Kojak30_ELON = _KOJAK30_DEFAULT_ELON_DECI_;

			if(m_DeciCaptureSetting.forSuperDry_Mode)
			{
				m_DeciCaptureSetting.TargetMinBrightness = _TARGET_MIN_BRIGHTNESS_FOR_DRY_;
				m_DeciCaptureSetting.TargetMaxBrightness = _TARGET_MAX_BRIGHTNESS_FOR_DRY_;
			}
			else
			{
				m_DeciCaptureSetting.TargetMinBrightness = m_propertyInfo.nCaptureBrightThresFlat[0];
				m_DeciCaptureSetting.TargetMaxBrightness = m_propertyInfo.nCaptureBrightThresFlat[1];
			}
			m_DeciCaptureSetting.TargetFingerCount = m_propertyInfo.nNumberOfObjects;
			if(m_propertyInfo.nSuperDryMode)
			{
				if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER)
					m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_DRY_ONE_FINGER_;
				else if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS)
					m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_DRY_TWO_FINGER_;
				else
					m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_DRY_FOUR_FINGER_;
			}
			else
			{
				if(m_propertyInfo.bEnableStartingVoltage == TRUE)
				{
					m_DeciCaptureSetting.DefaultDAC = m_propertyInfo.nStartingVoltage;
				}
				else
				{
					if(m_UsbDeviceInfo.bCanUseTOF &&
						m_propertyInfo.bEnableTOF)
					{
						if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
							m_DeciCaptureSetting.DefaultDAC = _KOJAK30_DAC_FOR_NORMAL_FINGER_WITH_TOF_;
						else
							m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FINGER_WITH_TOF_;
					}
					else if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_NEW_BRD_AND_DOUBLE_P_ ||
							m_propertyInfo.bKojakPLwithDPFilm == TRUE)
					{
						if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS || 
							m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
							m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_OF_DOUBLE_P_;
						else
							m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FOUR_FINGER_OF_DOUBLE_P_;
					}
					else
					{
						if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS || 
							m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
							m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_;
						else
							m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FOUR_FINGER_;
					}
				}
			}
			m_DeciCaptureSetting.CurrentDAC = m_DeciCaptureSetting.DefaultDAC;
			m_DeciCaptureSetting.DefaultGain = _DEFAULT_GAIN_FOR_DECI_;
			m_DeciCaptureSetting.CurrentGain = m_DeciCaptureSetting.DefaultGain;
			if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
				//m_DeciCaptureSetting.DefaultExposure = m_nATOF_EXPOSURE_DECI;
				m_DeciCaptureSetting.DefaultExposure = _KOJAK30_DEFAULT_EXPOSURE_FOR_DECI_;
			else
				m_DeciCaptureSetting.DefaultExposure = _DEFAULT_EXPOSURE_FOR_DECI_;
			m_DeciCaptureSetting.CurrentExposure = m_DeciCaptureSetting.DefaultExposure;
			m_DeciCaptureSetting.DefaultBitshift = _DEFAULT_BITSHIFT_FOR_DECI_;
			m_DeciCaptureSetting.CurrentBitshift = m_DeciCaptureSetting.DefaultBitshift;
			m_DeciCaptureSetting.DefaultCutThres = _DEFAULT_CUTTHRES_FOR_DECI_;
			m_DeciCaptureSetting.CurrentCutThres = m_DeciCaptureSetting.DefaultCutThres;
			m_DeciCaptureSetting.AdditionalFrame = 1;
			m_DeciCaptureSetting.RollCaptureMode = FALSE;
			m_DeciCaptureSetting.DetectionFrameCount = 0;

			m_FullCaptureSetting = m_DeciCaptureSetting;
			if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
				m_FullCaptureSetting.DefaultExposure = _KOJAK30_NO_BIT_EXPOSURE_FOR_FULLFRAME_;
			else
				m_FullCaptureSetting.DefaultExposure = _NO_BIT_EXPOSURE_FOR_FULLFRAME_;
			m_FullCaptureSetting.DefaultBitshift = _NO_BIT_BITSHIFT_FOR_FULLFRAME_;
			m_FullCaptureSetting.DefaultCutThres = _NO_BIT_CUTTHRES_FOR_FULLFRAME_;
			m_FullCaptureSetting.CurrentExposure = m_FullCaptureSetting.DefaultExposure;
			m_FullCaptureSetting.CurrentBitshift = m_FullCaptureSetting.DefaultBitshift;
			m_FullCaptureSetting.CurrentCutThres = m_FullCaptureSetting.DefaultCutThres;
			m_FullCaptureSetting.Kojak30_PERIOD = _KOJAK30_PERIOD_FOR_FULLFRAME_;
			m_FullCaptureSetting.Kojak30_ELON = _KOJAK30_DEFAULT_ELON_FOR_FULLFRAME_;

			if (m_UsbDeviceInfo.nFpgaVersion >= ((0 << 24) | (9 << 16) | (0 << 8) | 0x0D))
			{
				m_UsbDeviceInfo.bDecimation = TRUE;
				m_UsbDeviceInfo.nDecimation_Mode = DECIMATION_2X;
			}
			_ChangeDecimationModeForKojak(m_UsbDeviceInfo.bDecimation, &m_DeciCaptureSetting, TRUE);

			m_CurrentCaptureDAC = m_FullCaptureSetting.CurrentDAC;
		}
		else
			return IBSU_ERR_INVALID_PARAM_VALUE;
	}

    return IBSU_STATUS_OK;
}

int CMainCapture::_Read_Calibration_Data_From_EEPROM()
{
#define CALIBRATION_DATA_SOP		(0xF1)
#define CALIBRATION_DATA_EOP		(0xF2)

	int nRc = IBSU_STATUS_OK;
	int version, version2;
	int ValidData = FALSE;
	char checksum; 
	int PacketLength = 0, PacketLength2 = 0;
	BOOL isValid = FALSE;;

	memset(&m_propertyInfo.sKojakCalibrationInfo, 0, sizeof(KOJAK_CALIBRATION_INFO));

	// support new type of calbration data 
	// Read EEPROM of Cypress from 0x1FC0 where calibration data is saved.
#if defined(__ppi__)
    if ((nRc = _GetProperty(PROPERTY_CALIBRATON_DATA, (UCHAR *)m_propertyInfo.cTOFAnalogPlate)) != IBSU_STATUS_OK)
    {
        goto done;
    }

#else

	if ((nRc =  Reserved_ReadEEPROM(0x1FC0, (UCHAR *)m_propertyInfo.cTOFAnalogPlate, __MAX_CALIBRATION_DATA_LENGTH__)) != IBSU_STATUS_OK)
	{
		goto done;
	}


	// Validity check
	if ( (BYTE)m_propertyInfo.cTOFAnalogPlate[0] == CALIBRATION_DATA_SOP ) // check SOP
	{
		version = ((BYTE)m_propertyInfo.cTOFAnalogPlate[1] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[2]);
		version2 = (BYTE)m_propertyInfo.cTOFAnalogPlate[19];

		switch(version)
		{
		case 0x05: PacketLength = 17; break;
		case 0x04: PacketLength = 13; break;
		case 0x03: PacketLength = 12; break;
		default : isValid = FALSE;
		}

		if(version2 >= 6)
		{
			PacketLength2	= m_propertyInfo.cTOFAnalogPlate[18];
		}

		if( (BYTE)m_propertyInfo.cTOFAnalogPlate[PacketLength] == CALIBRATION_DATA_EOP ) // check EOP
		{
			checksum = _Calibration_Data_Checksum(&m_propertyInfo.cTOFAnalogPlate[1], PacketLength-2);
			if(m_propertyInfo.cTOFAnalogPlate[PacketLength-1] == checksum)
				isValid = TRUE;
			else
				isValid = FALSE;
		}
		else
			isValid = FALSE;
	}
	else
		isValid = FALSE;

	if(isValid == FALSE)
	{
		if ((nRc = _GetProperty(PROPERTY_IBIA_VERSION, (UCHAR *)m_propertyInfo.cTOFAnalogPlate)) != IBSU_STATUS_OK)
		{
			goto done;
		}
	}
#endif

	// if IBIA VERSION is empty, this is old version.
	if ( (BYTE)m_propertyInfo.cTOFAnalogPlate[0] == CALIBRATION_DATA_SOP ) // check SOP
	{
		int PacketLength = 12;
		BYTE data[4] = {0,};
		version = ((BYTE)m_propertyInfo.cTOFAnalogPlate[1] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[2]);

		if(version >= 0x05)
		{
			PacketLength = 17;

			if( (BYTE)m_propertyInfo.cTOFAnalogPlate[PacketLength] == CALIBRATION_DATA_EOP ) // check EOP
			{
				checksum = _Calibration_Data_Checksum(&m_propertyInfo.cTOFAnalogPlate[1], PacketLength-2);
				if(m_propertyInfo.cTOFAnalogPlate[PacketLength-1] == checksum)
				{
					m_propertyInfo.sKojakCalibrationInfo.Version = ((BYTE)m_propertyInfo.cTOFAnalogPlate[1] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[2]);
					m_propertyInfo.sKojakCalibrationInfo.ACMin = ((BYTE)m_propertyInfo.cTOFAnalogPlate[3] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[4]);
					m_propertyInfo.sKojakCalibrationInfo.ACMax = ((BYTE)m_propertyInfo.cTOFAnalogPlate[5] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[6]);
					m_propertyInfo.sKojakCalibrationInfo.LensPosRow = (char)m_propertyInfo.cTOFAnalogPlate[7];
					m_propertyInfo.sKojakCalibrationInfo.LensPosCol = (char)m_propertyInfo.cTOFAnalogPlate[8];
					m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal = (char)m_propertyInfo.cTOFAnalogPlate[9];
					m_propertyInfo.sKojakCalibrationInfo.PPIVertical = (char)m_propertyInfo.cTOFAnalogPlate[10];
					m_propertyInfo.sKojakCalibrationInfo.IsSupportSpoof = (BYTE)m_propertyInfo.cTOFAnalogPlate[11];

					memcpy(&data[0], &m_propertyInfo.cTOFAnalogPlate[12], 4);
					memset(m_propertyInfo.sKojakCalibrationInfo.cRequiredSDK, 0, IBSU_MAX_STR_LEN);
					sprintf(m_propertyInfo.sKojakCalibrationInfo.cRequiredSDK, "%d.%d.%d.%d", data[0], data[1], data[2], data[3]);

					if(m_propertyInfo.sKojakCalibrationInfo.LensPosRow > 127)
						m_propertyInfo.sKojakCalibrationInfo.LensPosRow = m_propertyInfo.sKojakCalibrationInfo.LensPosRow - 256;
					if(m_propertyInfo.sKojakCalibrationInfo.LensPosCol > 127)
						m_propertyInfo.sKojakCalibrationInfo.LensPosCol = m_propertyInfo.sKojakCalibrationInfo.LensPosCol - 256;
					if(m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal > 127)
						m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal = m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal - 256;
					if(m_propertyInfo.sKojakCalibrationInfo.PPIVertical > 127)
						m_propertyInfo.sKojakCalibrationInfo.PPIVertical = m_propertyInfo.sKojakCalibrationInfo.PPIVertical - 256;
					
					ValidData = TRUE;

					if(version2 >= 6)
					{
						m_UsbDeviceInfo.nFpgaVersion = _GetFpgaVersion(TRUE);

						if((PacketLength2 > 0) /*&&
											   (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK) && (m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0_)*/
											   )
						{
							if((BYTE)m_propertyInfo.cTOFAnalogPlate[PacketLength + PacketLength2] == (BYTE)0xf2)// EOF check
							{
								checksum = _Calibration_Data_Checksum(&m_propertyInfo.cTOFAnalogPlate[PacketLength+1], PacketLength2 - 2);	// checksum
								if((BYTE)m_propertyInfo.cTOFAnalogPlate[PacketLength + PacketLength2 - 1] == (BYTE)checksum)
								{
									m_propertyInfo.sKojakCalibrationInfo.Version	= m_propertyInfo.cTOFAnalogPlate[19];
									m_propertyInfo.sKojakCalibrationInfo.ATOF_sign	= m_propertyInfo.cTOFAnalogPlate[20];
									m_propertyInfo.sKojakCalibrationInfo.ATOF_Diff	= ((BYTE)m_propertyInfo.cTOFAnalogPlate[21] << 8) | (BYTE)m_propertyInfo.cTOFAnalogPlate[22];		// Analog Touch Min Diff
									m_propertyInfo.sKojakCalibrationInfo.ATOF_slope	= ((BYTE)m_propertyInfo.cTOFAnalogPlate[23] << 8) | (BYTE)m_propertyInfo.cTOFAnalogPlate[24];	// Analog Touch Slope
									m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_sign = m_propertyInfo.cTOFAnalogPlate[25];
									m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_Diff	= ((BYTE)m_propertyInfo.cTOFAnalogPlate[26] << 8) | (BYTE)m_propertyInfo.cTOFAnalogPlate[27];		// Analog Touch Min Diff
									m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_slope	= ((BYTE)m_propertyInfo.cTOFAnalogPlate[28] << 8) | (BYTE)m_propertyInfo.cTOFAnalogPlate[29];	// Analog Touch Slope

									_PostTraceLogCallback(0, "Calibration Info New (ver:%2X, min:%d, max:%d, pos_row:%d, pos_col:%d, ppi_hor:%d, ppi_ver:%d, spoof:%d, minSDK:%s, AT_Sign:%d, AT_Diff:%d, AT_Slope:%d, AT_OffSign:%d, AT_OffDiff:%d, AT_OffSlope:%d)",
										m_propertyInfo.sKojakCalibrationInfo.Version,
										m_propertyInfo.sKojakCalibrationInfo.ACMin, m_propertyInfo.sKojakCalibrationInfo.ACMax, 
										m_propertyInfo.sKojakCalibrationInfo.LensPosRow, m_propertyInfo.sKojakCalibrationInfo.LensPosCol,
										m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal, m_propertyInfo.sKojakCalibrationInfo.PPIVertical,
										m_propertyInfo.sKojakCalibrationInfo.IsSupportSpoof, m_propertyInfo.sKojakCalibrationInfo.cRequiredSDK,
										m_propertyInfo.sKojakCalibrationInfo.ATOF_sign, m_propertyInfo.sKojakCalibrationInfo.ATOF_Diff, m_propertyInfo.sKojakCalibrationInfo.ATOF_slope,
										m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_sign, m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_Diff, m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_slope);
								}
							}
							else
							{
								nRc = IBSU_ERR_DEVICE_INVALID_CALIBRATION_DATA;
								goto done;
							}
						}
					}
					else
					{
						_PostTraceLogCallback(0, "Calibration Info New (ver:%2X, min:%d, max:%d, pos_row:%d, pos_col:%d, ppi_hor:%d, ppi_ver:%d, spoof:%d, minSDK:%s)", m_propertyInfo.sKojakCalibrationInfo.Version,
						m_propertyInfo.sKojakCalibrationInfo.ACMin, m_propertyInfo.sKojakCalibrationInfo.ACMax, 
						m_propertyInfo.sKojakCalibrationInfo.LensPosRow, m_propertyInfo.sKojakCalibrationInfo.LensPosCol,
						m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal, m_propertyInfo.sKojakCalibrationInfo.PPIVertical,
						m_propertyInfo.sKojakCalibrationInfo.IsSupportSpoof, m_propertyInfo.sKojakCalibrationInfo.cRequiredSDK);
					}
				}
			}
			else
			{
				nRc = IBSU_ERR_DEVICE_INVALID_CALIBRATION_DATA;
				goto done;
			}
		}
		else if(version >= 0x04)
		{
			PacketLength = 13;

			if( (BYTE)m_propertyInfo.cTOFAnalogPlate[PacketLength] == CALIBRATION_DATA_EOP ) // check EOP
			{
				checksum = _Calibration_Data_Checksum(&m_propertyInfo.cTOFAnalogPlate[1], PacketLength-2);
				if(m_propertyInfo.cTOFAnalogPlate[PacketLength-1] == checksum)
				{
					m_propertyInfo.sKojakCalibrationInfo.Version = ((BYTE)m_propertyInfo.cTOFAnalogPlate[1] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[2]);
					m_propertyInfo.sKojakCalibrationInfo.ACMin = ((BYTE)m_propertyInfo.cTOFAnalogPlate[3] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[4]);
					m_propertyInfo.sKojakCalibrationInfo.ACMax = ((BYTE)m_propertyInfo.cTOFAnalogPlate[5] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[6]);
					m_propertyInfo.sKojakCalibrationInfo.LensPosRow = (char)m_propertyInfo.cTOFAnalogPlate[7];
					m_propertyInfo.sKojakCalibrationInfo.LensPosCol = (char)m_propertyInfo.cTOFAnalogPlate[8];
					m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal = (char)m_propertyInfo.cTOFAnalogPlate[9];
					m_propertyInfo.sKojakCalibrationInfo.PPIVertical = (char)m_propertyInfo.cTOFAnalogPlate[10];
					m_propertyInfo.sKojakCalibrationInfo.IsSupportSpoof = (BYTE)m_propertyInfo.cTOFAnalogPlate[11];

					if(m_propertyInfo.sKojakCalibrationInfo.LensPosRow > 127)
						m_propertyInfo.sKojakCalibrationInfo.LensPosRow = m_propertyInfo.sKojakCalibrationInfo.LensPosRow - 256;
					if(m_propertyInfo.sKojakCalibrationInfo.LensPosCol > 127)
						m_propertyInfo.sKojakCalibrationInfo.LensPosCol = m_propertyInfo.sKojakCalibrationInfo.LensPosCol - 256;
					if(m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal > 127)
						m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal = m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal - 256;
					if(m_propertyInfo.sKojakCalibrationInfo.PPIVertical > 127)
						m_propertyInfo.sKojakCalibrationInfo.PPIVertical = m_propertyInfo.sKojakCalibrationInfo.PPIVertical - 256;

					ValidData = TRUE;

					_PostTraceLogCallback(0, "Calibration Info New (ver:%2X, min:%d, max:%d, pos_row:%d, pos_col:%d, ppi_hor:%d, ppi_ver:%d, spoof:%d)", m_propertyInfo.sKojakCalibrationInfo.Version,
						m_propertyInfo.sKojakCalibrationInfo.ACMin, m_propertyInfo.sKojakCalibrationInfo.ACMax, 
						m_propertyInfo.sKojakCalibrationInfo.LensPosRow, m_propertyInfo.sKojakCalibrationInfo.LensPosCol,
						m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal, m_propertyInfo.sKojakCalibrationInfo.PPIVertical,
						m_propertyInfo.sKojakCalibrationInfo.IsSupportSpoof);
				}
			}
			else
			{
				nRc = IBSU_ERR_DEVICE_INVALID_CALIBRATION_DATA;
				goto done;
			}
		}
		else if(version >= 0x03)
		{
			PacketLength = 12;

			if( (BYTE)m_propertyInfo.cTOFAnalogPlate[PacketLength] == CALIBRATION_DATA_EOP ) // check EOP
			{
				checksum = _Calibration_Data_Checksum(&m_propertyInfo.cTOFAnalogPlate[1], PacketLength-2);
				if(m_propertyInfo.cTOFAnalogPlate[PacketLength-1] == checksum)
				{
					m_propertyInfo.sKojakCalibrationInfo.Version = ((BYTE)m_propertyInfo.cTOFAnalogPlate[1] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[2]);
					m_propertyInfo.sKojakCalibrationInfo.ACMin = ((BYTE)m_propertyInfo.cTOFAnalogPlate[3] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[4]);
					m_propertyInfo.sKojakCalibrationInfo.ACMax = ((BYTE)m_propertyInfo.cTOFAnalogPlate[5] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[6]);
					m_propertyInfo.sKojakCalibrationInfo.LensPosRow = (char)m_propertyInfo.cTOFAnalogPlate[7];
					m_propertyInfo.sKojakCalibrationInfo.LensPosCol = (char)m_propertyInfo.cTOFAnalogPlate[8];
					m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal = (char)m_propertyInfo.cTOFAnalogPlate[9];
					m_propertyInfo.sKojakCalibrationInfo.PPIVertical = (char)m_propertyInfo.cTOFAnalogPlate[9];
					m_propertyInfo.sKojakCalibrationInfo.IsSupportSpoof = 0;

					if(m_propertyInfo.sKojakCalibrationInfo.LensPosRow > 127)
						m_propertyInfo.sKojakCalibrationInfo.LensPosRow = m_propertyInfo.sKojakCalibrationInfo.LensPosRow - 256;
					if(m_propertyInfo.sKojakCalibrationInfo.LensPosCol > 127)
						m_propertyInfo.sKojakCalibrationInfo.LensPosCol = m_propertyInfo.sKojakCalibrationInfo.LensPosCol - 256;
					if(m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal > 127)
						m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal = m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal - 256;
					if(m_propertyInfo.sKojakCalibrationInfo.PPIVertical > 127)
						m_propertyInfo.sKojakCalibrationInfo.PPIVertical = m_propertyInfo.sKojakCalibrationInfo.PPIVertical - 256;

					ValidData = TRUE;

					_PostTraceLogCallback(0, "Kojak Calibration Info New (ver:%2X, min:%d, max:%d, pos_row:%d, pos_col:%d, ppi_hor:%d, ppi_ver:%d)", m_propertyInfo.sKojakCalibrationInfo.Version,
						m_propertyInfo.sKojakCalibrationInfo.ACMin, m_propertyInfo.sKojakCalibrationInfo.ACMax, 
						m_propertyInfo.sKojakCalibrationInfo.LensPosRow, m_propertyInfo.sKojakCalibrationInfo.LensPosCol,
						m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal, m_propertyInfo.sKojakCalibrationInfo.PPIVertical);
				}

			}
			else
			{
				nRc = IBSU_ERR_DEVICE_INVALID_CALIBRATION_DATA;
				goto done;
			}
		}
	}
	else	// old type
	{
		if ((nRc = _GetProperty(PROPERTY_IBIA_VENDOR_ID, (UCHAR *)m_propertyInfo.cTOFAnalogPlate)) != IBSU_STATUS_OK)
		{
			goto done;
		}
	
		version = ((BYTE)m_propertyInfo.cTOFAnalogPlate[0] << 8) + (BYTE)m_propertyInfo.cTOFAnalogPlate[1];
		
		if(version > 0xFF)				// min and slope only for current sensor (prototype version)
		{
			memset(&m_propertyInfo.sKojakCalibrationInfo, 0, sizeof(KOJAK_CALIBRATION_INFO));
			
			ValidData = TRUE;
		}
		else if(version >= 0x03)		// min and slope only for current sensor (higher than cypress 2.0.4) + lens position info
		{
			m_propertyInfo.sKojakCalibrationInfo.Version = ((BYTE)m_propertyInfo.cTOFAnalogPlate[0] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[1]);
			m_propertyInfo.sKojakCalibrationInfo.ACMin = ((BYTE)m_propertyInfo.cTOFAnalogPlate[2] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[3]);
			m_propertyInfo.sKojakCalibrationInfo.ACMax = ((BYTE)m_propertyInfo.cTOFAnalogPlate[4] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[5]);
			m_propertyInfo.sKojakCalibrationInfo.LensPosRow = (char)m_propertyInfo.cTOFAnalogPlate[6];
			m_propertyInfo.sKojakCalibrationInfo.LensPosCol = (char)m_propertyInfo.cTOFAnalogPlate[7];
			m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal = (char)m_propertyInfo.cTOFAnalogPlate[8];
			m_propertyInfo.sKojakCalibrationInfo.PPIVertical = (char)m_propertyInfo.cTOFAnalogPlate[9];
			m_propertyInfo.sKojakCalibrationInfo.IsSupportSpoof	= 0;

			if(m_propertyInfo.sKojakCalibrationInfo.LensPosRow > 127)
				m_propertyInfo.sKojakCalibrationInfo.LensPosRow = m_propertyInfo.sKojakCalibrationInfo.LensPosRow - 256;
			if(m_propertyInfo.sKojakCalibrationInfo.LensPosCol > 127)
				m_propertyInfo.sKojakCalibrationInfo.LensPosCol = m_propertyInfo.sKojakCalibrationInfo.LensPosCol - 256;
			if(m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal > 127)
				m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal = m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal - 256;
			if(m_propertyInfo.sKojakCalibrationInfo.PPIVertical > 127)
				m_propertyInfo.sKojakCalibrationInfo.PPIVertical = m_propertyInfo.sKojakCalibrationInfo.PPIVertical - 256;

			ValidData = TRUE;

			_PostTraceLogCallback(0, "Kojak Calibration Info Old (ver:%2X, min:%d, max:%d, pos_row:%d, pos_col:%d, ppi_hor:%d, ppi_ver:%d)", m_propertyInfo.sKojakCalibrationInfo.Version,
				m_propertyInfo.sKojakCalibrationInfo.ACMin, m_propertyInfo.sKojakCalibrationInfo.ACMax, 
				m_propertyInfo.sKojakCalibrationInfo.LensPosRow, m_propertyInfo.sKojakCalibrationInfo.LensPosCol,
				m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal, m_propertyInfo.sKojakCalibrationInfo.PPIVertical);
		}
		else if(version >= 0x02)		// min and slope only for current sensor (higher than cypress 2.0.4) + lens position info
		{
			m_propertyInfo.sKojakCalibrationInfo.Version = ((BYTE)m_propertyInfo.cTOFAnalogPlate[0] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[1]);
			m_propertyInfo.sKojakCalibrationInfo.ACMin = ((BYTE)m_propertyInfo.cTOFAnalogPlate[2] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[3]);
			m_propertyInfo.sKojakCalibrationInfo.ACMax = ((BYTE)m_propertyInfo.cTOFAnalogPlate[4] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[5]);
			m_propertyInfo.sKojakCalibrationInfo.LensPosRow = (char)m_propertyInfo.cTOFAnalogPlate[6];
			m_propertyInfo.sKojakCalibrationInfo.LensPosCol = (char)m_propertyInfo.cTOFAnalogPlate[7];
			m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal	= 0;
			m_propertyInfo.sKojakCalibrationInfo.PPIVertical	= 0;
			m_propertyInfo.sKojakCalibrationInfo.IsSupportSpoof	= 0;

			if(m_propertyInfo.sKojakCalibrationInfo.LensPosRow > 127)
				m_propertyInfo.sKojakCalibrationInfo.LensPosRow = m_propertyInfo.sKojakCalibrationInfo.LensPosRow - 256;
			if(m_propertyInfo.sKojakCalibrationInfo.LensPosCol > 127)
				m_propertyInfo.sKojakCalibrationInfo.LensPosCol = m_propertyInfo.sKojakCalibrationInfo.LensPosCol - 256;
			
			ValidData = TRUE;

			_PostTraceLogCallback(0, "Kojak Calibration Info Old (ver:%2X, min:%d, max:%d, pos_row:%d, pos_col:%d)", m_propertyInfo.sKojakCalibrationInfo.Version,
				m_propertyInfo.sKojakCalibrationInfo.ACMin, m_propertyInfo.sKojakCalibrationInfo.ACMax, 
				m_propertyInfo.sKojakCalibrationInfo.LensPosRow, m_propertyInfo.sKojakCalibrationInfo.LensPosCol);
		}
		else if(version >= 0x01)		// min and slope only for current sensor (higher than cypress 2.0.4)
		{
			m_propertyInfo.sKojakCalibrationInfo.Version = ((BYTE)m_propertyInfo.cTOFAnalogPlate[0] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[1]);
			m_propertyInfo.sKojakCalibrationInfo.ACMin = ((BYTE)m_propertyInfo.cTOFAnalogPlate[2] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[3]);
			m_propertyInfo.sKojakCalibrationInfo.ACMax = ((BYTE)m_propertyInfo.cTOFAnalogPlate[4] << 8) + ((BYTE)m_propertyInfo.cTOFAnalogPlate[5]);
			m_propertyInfo.sKojakCalibrationInfo.LensPosRow = 0;
			m_propertyInfo.sKojakCalibrationInfo.LensPosCol = 0;
			m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal	= 0;
			m_propertyInfo.sKojakCalibrationInfo.PPIVertical	= 0;
			m_propertyInfo.sKojakCalibrationInfo.IsSupportSpoof	= 0;

			ValidData = TRUE;

			_PostTraceLogCallback(0, "Kojak Calibration Info Old (ver:%2X, min:%d, max:%d)", m_propertyInfo.sKojakCalibrationInfo.Version,
				m_propertyInfo.sKojakCalibrationInfo.ACMin, m_propertyInfo.sKojakCalibrationInfo.ACMax);
		}
	}

done:

	if(ValidData == FALSE)
	{
		// No calibration data
		m_propertyInfo.sKojakCalibrationInfo.Version = -1;
		m_propertyInfo.sKojakCalibrationInfo.ACMin = -1;
		m_propertyInfo.sKojakCalibrationInfo.ACMax = -1;
		m_propertyInfo.sKojakCalibrationInfo.LensPosRow = 0;
		m_propertyInfo.sKojakCalibrationInfo.LensPosCol = 0;
		m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal	= 0;
		m_propertyInfo.sKojakCalibrationInfo.PPIVertical	= 0;
		m_propertyInfo.sKojakCalibrationInfo.IsSupportSpoof	= 0;
		m_propertyInfo.sKojakCalibrationInfo.ATOF_sign = 0;
		m_propertyInfo.sKojakCalibrationInfo.ATOF_Diff = 0;
		m_propertyInfo.sKojakCalibrationInfo.ATOF_slope = 0;
		m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_sign = 0;
		m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_Diff = 0;
		m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_slope = 0;
	}

	m_propertyInfo.bIsSpoofSupported = m_propertyInfo.sKojakCalibrationInfo.IsSupportSpoof;

/*	if( 
		(m_propertyInfo.sKojakCalibrationInfo.LensPosRow < _CIS_WINDOWING_ROW_MIN)				|| 
		(m_propertyInfo.sKojakCalibrationInfo.LensPosRow >  _CIS_WINDOWING_ROW_MAX)				||		// Windowing Row
		(m_propertyInfo.sKojakCalibrationInfo.LensPosCol < _CIS_WINDOWING_COLUMN_MIN)			|| 
		(m_propertyInfo.sKojakCalibrationInfo.LensPosCol > _CIS_WINDOWING_COLUMN_MAX)			||		// Windowing Column
		(m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal	< _PPI_CORRECTION_HORIZONTAL_MIN)	|| 
		(m_propertyInfo.sKojakCalibrationInfo.PPIHorizontal > _PPI_CORRECTION_HORIZONTAL_MAX)	||		// PPI Horizontal
		(m_propertyInfo.sKojakCalibrationInfo.PPIVertical	< _PPI_CORRECTION_VERTICAL_MIN)		|| 
		(m_propertyInfo.sKojakCalibrationInfo.PPIVertical > _PPI_CORRECTION_VERTICAL_MAX) )				// PPI Vertical
	{
		nRc = IBSU_ERR_DEVICE_INVALID_CALIBRATION_DATA;
	}
*/
	return nRc;
}

char CMainCapture::_Calibration_Data_Checksum(char *data, unsigned char length)
{
	unsigned char	count;
	unsigned int	Sum = 0;

	for (count = 0; count < length; count++)
		Sum = Sum + data[count];

	return (char)(Sum & 0xFF);
}

int CMainCapture::_IsRequiredSDKVersion(char *requiredSDKVersion)
{

	int					nRc = IBSU_ERR_DEVICE_HIGHER_SDK_REQUIRED;
	IBSU_SdkVersion		curSDKVersion;
	int					data[4] = {0,}, data2[4] = {0,};
	int					nCurSDKVer = 0, nDeviceSDKVer = 0;

	IBSU_GetSDKVersion(&curSDKVersion);
	sscanf(curSDKVersion.Product, "%d.%d.%d.%d", &data[0], &data[1], &data[2], &data[3]);
	sscanf(requiredSDKVersion, "%d.%d.%d.%d", &data2[0], &data2[1], &data2[2], &data2[3]);

	
	// check if current SDK Version is lower then than the minimum SDK Version of device
	int nCurSDKVersion		=	data[0] * 1000	+
								data[1] * 100	+
								data[2] * 10	+
								data[3] * 1		;

	int nminimumSDKVersion	=	data2[0] * 1000	+
								data2[1] * 100	+
								data2[2] * 10	+
								data2[3] * 1	;

	
	if(nCurSDKVersion >= nminimumSDKVersion)
		nRc = IBSU_STATUS_OK;
	else
		nRc = IBSU_ERR_DEVICE_HIGHER_SDK_REQUIRED;

	return nRc;
}

int CMainCapture::_GetCurveVersion()
{
	int i=0;
	int Version = 0;
	const char s[2] = ".";
	char tmpfwVersion[256]={0};
	char tmpVersion[256]={0};
	char *token;

	if (_GetProperty(PROPERTY_FIRMWARE, (UCHAR *)tmpfwVersion) == IBSU_STATUS_OK)
	{
		// change version string to integer
		strcpy(tmpVersion, tmpfwVersion);

		/* get the first token */
		token = strtok(tmpVersion, s);

		/* walk through other tokens */
		while( token != NULL && i < 3) {
			Version += atoi(token) << (16-i*8);
			token = strtok(NULL, s);
			i++;
		}
	}

	return Version;
}

int CMainCapture::_SetEncryptionKey(const unsigned char *pEncryptionKey, const IBSU_EncryptionMode encMode)
{
	int nRc = IBSU_STATUS_OK;
	unsigned char tempKey[AES256_KEY_LENGTH];

	if (m_propertyInfo.bSupportEncryption == FALSE)
	{
		return IBSU_ERR_NOT_SUPPORTED;
	}
	
	switch(encMode)
	{
	case ENUM_IBSU_ENCRYPTION_KEY_RANDOM:
		m_pAlgo->m_AESEnc->AES_Set_Key_Random();
		m_EncryptionMode = encMode;
		break;

	case ENUM_IBSU_ENCRYPTION_KEY_CUSTOM:
		memcpy(tempKey, (unsigned char*)pEncryptionKey, AES256_KEY_LENGTH);
		m_pAlgo->m_AESEnc->AES_Set_Key(tempKey);
		m_EncryptionMode = encMode;
		break;

	default:
		return IBSU_ERR_INVALID_PARAM_VALUE;
	}


	UCHAR org[256], enc[512];

	memset(org, 0, 256);
	memset(enc, 0, 512);

	memcpy(org, (PUCHAR)AES_KEY_ENCRYPTION, 256);

	CIBEncryption *RSA = new CIBEncryption(org);
	RSA->AES_Key_Encrypt(org, enc);
	delete RSA;

	return _AES_KeyTransfer(enc, 512);
}

int CMainCapture::Reserved_GetEncryptedImage(unsigned char *pEncKey, IBSU_ImageData *pRawEncImage)
{
	if(m_pAlgo->m_AESEnc == NULL)
		return IBSU_ERR_NOT_SUPPORTED;

	memcpy(pEncKey, &m_pAlgo->m_AESEnc->m_rkey_all[0], AES256_KEY_LENGTH);

    pRawEncImage->BitsPerPixel = 8;
    pRawEncImage->Format = IBSU_IMG_FORMAT_GRAY;
    pRawEncImage->Buffer = m_pAlgo->m_EncryptTemp;
    pRawEncImage->Width = m_UsbDeviceInfo.CisImgWidth;
    pRawEncImage->Height = m_UsbDeviceInfo.CisImgHeight;
    pRawEncImage->Pitch = -m_UsbDeviceInfo.CisImgWidth;
    pRawEncImage->ResolutionX = m_UsbDeviceInfo.scanResolutionX;
    pRawEncImage->ResolutionY = m_UsbDeviceInfo.scanResolutionY;

	if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
        pRawEncImage->Width = m_UsbDeviceInfo.CisImgWidth_Roll;
        pRawEncImage->Height = m_UsbDeviceInfo.CisImgHeight_Roll;
        pRawEncImage->Pitch = -m_UsbDeviceInfo.CisImgWidth_Roll;
	}

    return IBSU_STATUS_OK;
}

int	CMainCapture::_AES_KeyTransfer(const unsigned char *pEncryptionKey, int KeyLength)
{
	switch(m_UsbDeviceInfo.devType)
	{
		case DEVICE_TYPE_WATSON_MINI:
			return _AES_KeyTransfer_For_Watson((unsigned char*)pEncryptionKey, KeyLength);

		case DEVICE_TYPE_KOJAK:
			return _AES_KeyTransfer_For_Kojak((unsigned char*)pEncryptionKey, KeyLength);
	}

	return IBSU_ERR_NOT_SUPPORTED;
}

int	CMainCapture::Reserved_CBP_GetPreviewInfo(IBSU_SegmentPosition *segmentPositionArray, IBSU_SegmentPosition *segmentPositionArray_for_geo, int *segmentPositionArrayCount, 
BOOL *IsFingerDetected)
{
	*segmentPositionArrayCount = m_pAlgo->m_CBPPreviewInfo.SegPosArrCount;
	memset(segmentPositionArray, 0, sizeof(IBSU_SegmentPosition)*IBSU_MAX_SEGMENT_COUNT);
	memcpy(segmentPositionArray, &m_pAlgo->m_CBPPreviewInfo.SegPosArr_90, sizeof(IBSU_SegmentPosition)*m_pAlgo->m_CBPPreviewInfo.SegPosArrCount);
	memset(segmentPositionArray_for_geo, 0, sizeof(IBSU_SegmentPosition)*IBSU_MAX_SEGMENT_COUNT);
	memcpy(segmentPositionArray_for_geo, &m_pAlgo->m_CBPPreviewInfo.SegPosArr, sizeof(IBSU_SegmentPosition)*m_pAlgo->m_CBPPreviewInfo.SegPosArrCount);
	*IsFingerDetected = m_pAlgo->m_cImgAnalysis.isDetected;

	return IBSU_STATUS_OK;
}

int	CMainCapture::Reserved_CBP_CleanUp()
{
	m_blockProperty.bGetOneFrameImage = FALSE;

	return IBSU_STATUS_OK;
}

int	CMainCapture::Reserved_CBP_IsFingerOn(BOOL *bIsFingerOn)
{
	*bIsFingerOn = (m_pAlgo->m_cImgAnalysis.finger_count > 0) ? TRUE : FALSE;

	return IBSU_STATUS_OK;
}

