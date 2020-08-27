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

// Only include this code when running on top of Windows USB driver
// (WINCE does not use this driver)
#ifdef __USE_LIBUSBX_DRIVER__

#include "../IBScanUltimateDLL.h"
#include <math.h>
#include <winsock.h> // For timeval struct

////////////////////////////////////////////////////////////////////////////////
// Implementation

// WinCE doesn't have this function
static void GetSystemTimeAsFileTime(FILETIME* pSystemAsFileTime)
{
    SYSTEMTIME st;
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, pSystemAsFileTime);
}

// gettimeofday in windows
int CMainCapture::gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    static int tzflag;
	static _TIME_ZONE_INFORMATION timezoneinfo;

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
			GetTimeZoneInformation( &timezoneinfo );
            tzflag++;
        }
        tz->tz_minuteswest = timezoneinfo.Bias / 60;
        tz->tz_dsttime = 0; // always return timezone not used. TODO: Replace with correct code.
    }

    return 0;
}

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

        Event = WaitForMultipleObjects(3, pOwner->m_hCaptureThread_EventArray, FALSE, INFINITE);

        switch (Event)
        {
            case 0:
                bExitThread = TRUE;
                break;
            case 1:
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

                    ResetEvent(pOwner->m_hCaptureThread_CaptureEvent);
                    SetEvent(pOwner->m_hCaptureThread_DummyEvent);
                }
                break;

            case 2:
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
                pOwner->m_pDlgUsbManager->m_bIsReadySend[CALLBACK_NOTIFY_MESSAGE])
        {
            break;
        }

        Sleep(10);
    }

    pOwner->SetIsActiveStateOfCaptureThread(FALSE/*Capture*/, FALSE/*Dummy*/, FALSE/*Idle*/);

    pOwner->m_bAliveCaptureThread = FALSE;

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
            pOwner->_PostCallback(CALLBACK_COMMUNICATION_BREAK);		// Unknown or FPGA problem
        }
    }

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
    int				nRc = IBSU_STATUS_OK;

    SetEvent(pThreadParam->threadStarted);

    nRc = pOwner->_OpenDeviceThread();
    if (nRc == IBSU_STATUS_OK)
    {
        pOwner->SetInitialized(TRUE);
    }

    pOwner->m_clbkProperty.errorCode = nRc;
    pOwner->_PostCallback(CALLBACK_ASYNC_OPEN_DEVICE);

    AfxEndThread(0);

    return 1;
}


/////////////////////////////////////////////////////////////////////////////
// Static members

/////////////////////////////////////////////////////////////////////////////


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
            TRACE(TEXT("Unexpected _UsbBulkOutIn() failed = %d\n"), m_bRunningEP6);
#endif
            return IBSU_ERR_CHANNEL_IO_COMMAND_FAILED;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////
    CThreadSync Sync;		// 2011-12-18 enzyme add
    IO_PACKET	io_packet;
    int			nResult;
    int			nByteCount;
	int			nActualLength;

    if (!m_hLibUsbDriver)
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

        nByteCount = (int)outDataLen + 1;
		nResult = libusb_bulk_transfer(m_hLibUsbDriver, outEp, (UCHAR *)(&io_packet), nByteCount, &nActualLength, timeout);

	    if (nResult < 0 || nByteCount != nActualLength)
		{
            return IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
        }
    }

    if (inEp >= 0)
    {
        nResult = libusb_bulk_transfer(m_hLibUsbDriver, inEp, (UCHAR *)inData, inDataLen, &nActualLength, timeout);

        if (nBytesRead != NULL)
        {
            *nBytesRead = nActualLength;
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
    int		nResult;
	int		nActualLength = 0;

    if (!m_hLibUsbDriver)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

	nResult = libusb_bulk_transfer(m_hLibUsbDriver, ep & 0xFF, data, dataLen,
		&nActualLength, timeout);


    if (nBytesRead != NULL)
    {
        *nBytesRead = nActualLength;
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
    IO_PACKET	io_packet;
    int			nResult;
    int			nByteCount;
	int			nActualLength;

    if (m_hLibUsbDriver == NULL)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    if (dataLen > MAX_DATA_BYTES)
    {
        dataLen = MAX_DATA_BYTES;
    }

    io_packet.cmd.Ioctl     = (UCHAR)uiCommand;
    if (dataLen > 0)
    {
        memcpy(io_packet.cmd.Buffer, data, dataLen);
    }

    nByteCount = (int)dataLen + 1;

	nResult = libusb_bulk_transfer(m_hLibUsbDriver, ep, (UCHAR *)(&io_packet), nByteCount, &nActualLength, timeout);

    if (nResult < 0 || nByteCount != nActualLength)
    {
        return IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
    }

    return IBSU_STATUS_OK;
}




BOOL CMainCapture::_OpenUSBDevice(int deviceIndex, int *errorNo)
{
    CThreadSync Sync;

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

	r = libusb_get_device_list(NULL, &dev_list);
	if (r < 0) {
//		usbi_err("get_device_list failed with error %d", r);
        *errorNo = IBSU_ERR_DEVICE_NOT_MATCHED;
        return FALSE;
	}

	/* Iterate over the device list, finding the desired device */
	dev_list_len = r;
	for (i = 0; i < dev_list_len; i++) {
		libusb_device *dev = dev_list[i];
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

    openretval = libusb_open(dev, &m_hLibUsbDriver);

	if( openretval != 0 || !m_hLibUsbDriver )
	{
	// HACKHACK: Add logging here
        *errorNo = IBSU_ERR_DEVICE_NOT_MATCHED;
        goto cleanup;
    }

    if ((nRc = libusb_claim_interface(m_hLibUsbDriver, USB_INTERFACE)) < 0)
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

        libusb_close(m_hLibUsbDriver);
        m_hLibUsbDriver = NULL;
		goto cleanup;
    }

	ret = TRUE;

cleanup:
	// Free the entire list, relying on the refcount incremented by the Open to keep the open device from being freed.
	libusb_free_device_list(dev_list, 1); 
	return ret;

}


void CMainCapture::_CloseUSBDevice()
{
    if (m_hLibUsbDriver != NULL)
    {
        libusb_release_interface(m_hLibUsbDriver, USB_INTERFACE);
        libusb_close(m_hLibUsbDriver);
        m_hLibUsbDriver = NULL;
    }
}

void CMainCapture::_SetPID_VID(UsbDeviceInfo *pScanDevDesc)
{
    if( m_hLibUsbDriver != NULL && pScanDevDesc != NULL)
	{
		libusb_device *dev = libusb_get_device( m_hLibUsbDriver );
		if( dev != NULL )
		{
			struct libusb_device_descriptor desc;
			if( libusb_get_device_descriptor(dev, &desc) == 0)
			{
			    pScanDevDesc->vid = desc.idVendor;
		        pScanDevDesc->pid = desc.idProduct;
			}
		}
	}
}

int CMainCapture::_GetPID()
{
    if( m_hLibUsbDriver != NULL )
	{
		libusb_device *dev = libusb_get_device( m_hLibUsbDriver );
		if( dev != NULL )
		{
			struct libusb_device_descriptor desc;
			if( libusb_get_device_descriptor(dev, &desc) == 0)
			{
		        return desc.idProduct;
			}
		}
	}
    
	return -1;
}

int CMainCapture::_GetUsbSpeed(UsbSpeed *speed)
{
    if( m_hLibUsbDriver == NULL )
	{
		return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
	}

	libusb_device *dev = libusb_get_device( m_hLibUsbDriver );
	if( dev == NULL ) // This shouldn't be possible.
	{
		ASSERT(dev != NULL); 
		return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
	}

	struct libusb_device_descriptor desc;
	if( libusb_get_device_descriptor(dev, &desc) == 0)
	{
		if (desc.bcdUSB == 0x0200)
		{
			*speed = USB_HIGH_SPEED;
		}
		else
		{
			*speed = USB_FULL_SPEED;
		}
	}
  
    return IBSU_STATUS_OK;
}


int CMainCapture::ClientWindow_Create(const IBSU_HWND hWindow, const DWORD left, const DWORD top, const DWORD right, const DWORD bottom, BOOL bReserved, DWORD imgWidth, DWORD imgHeight)
{
    CThreadSync Sync;			// enzyme add 2013-02-04 - To remove wired image during run _DrawClientWindow() from Capture Thread.

    if (left < 0 || top < 0 || (right - left) < 0 || (bottom - top) < 0)
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
    }

    m_hWindow = hWindow;
    m_rectWindow.left = left;
    m_rectWindow.top = top;
    m_rectWindow.right = right;
    m_rectWindow.bottom = bottom;

    ////////////////////////////////////////////////////////////////////////////////////////
    // enzyme add 2012-12-20 We have to redraw image viewer.
    // Watson & Watson Mini & Sherlock is 800*750 pixels, Curve is 288*352 pixels, Columbo is 400*500 pixels
    RECT rt;
    GetClientRect(m_hWindow, &rt);
    InvalidateRect(m_hWindow, &rt, TRUE);
    UpdateWindow(m_hWindow);
    ////////////////////////////////////////////////////////////////////////////////////////

    // enzyme 2013-01-31 bug fixed when you use big viewer
    DWORD outWidth = m_rectWindow.right - m_rectWindow.left;
    DWORD outHeight = m_rectWindow.bottom - m_rectWindow.top;
    int tmp_width = outWidth;
    int tmp_height = outHeight;

    if (outWidth > imgWidth)
    {
        tmp_width = imgWidth;
    }
    if (outHeight > imgHeight)
    {
        tmp_height = imgHeight;
    }

    float ratio_width = (float)tmp_width / (float)imgWidth;
    float ratio_height = (float)tmp_height / (float)imgHeight;

    m_DisplayWindow.Width = outWidth;
    m_DisplayWindow.Height = outHeight;

    if (ratio_width >= ratio_height)
    {
        m_DisplayWindow.imgWidth = tmp_height * imgWidth / imgHeight;
        m_DisplayWindow.imgHeight = tmp_height;
        m_DisplayWindow.x = (outWidth - m_DisplayWindow.imgWidth) / 2;
        m_DisplayWindow.y = (outHeight - m_DisplayWindow.imgHeight) / 2;
    }
    else
    {
        m_DisplayWindow.imgWidth = tmp_width;
        m_DisplayWindow.imgHeight = tmp_width * imgHeight / imgWidth;
        m_DisplayWindow.x = (outWidth - m_DisplayWindow.imgWidth) / 2;
        m_DisplayWindow.y = (outHeight - m_DisplayWindow.imgHeight) / 2;
    }

    if (m_DisplayWindow.x < 0)
    {
        m_DisplayWindow.x = 0;
    }
    if (m_DisplayWindow.y < 0)
    {
        m_DisplayWindow.y = 0;
    }

    m_DisplayWindow.imgWidth -= (m_DisplayWindow.imgWidth % 4);

    // enzyme delete 2013-02-05 - we used same code to _DrawClientWindow() to remove "CThreadSync Sync;" on this function
    // Then we can save capture time from each devices.
    //	m_BitmapInfo->bmiHeader.biHeight = m_DisplayWindow.imgHeight;
    //	m_BitmapInfo->bmiHeader.biSizeImage = m_DisplayWindow.imgWidth*m_DisplayWindow.imgHeight;
    //	m_BitmapInfo->bmiHeader.biWidth = m_DisplayWindow.imgWidth;

    /////////////////////////////////////////////////////////////////////////////////////////////
    // enzyme 2013-01-02 Modify potential bug fixed
    /*	if( m_DisplayWindow.image )
    		delete [] m_DisplayWindow.image;

    	m_DisplayWindow.image = new BYTE[m_DisplayWindow.imgWidth*m_DisplayWindow.imgHeight];
    */
    if (!m_DisplayWindow.image)
    {
        m_DisplayWindow.image = new BYTE[__MAX_IMG_WIDTH__ * __MAX_IMG_HEIGHT__];
    }
    /////////////////////////////////////////////////////////////////////////////////////////////

    m_DisplayWindow.bIsSetDisplayWindow = TRUE;

    return IBSU_STATUS_OK;
}

int CMainCapture::ClientWindow_Destroy(const BOOL clearExistingInfo)
{
    CThreadSync Sync;			// enzyme add 2013-02-05

    if (!m_DisplayWindow.bIsSetDisplayWindow)
    {
        return IBSU_ERR_CLIENT_WINDOW_NOT_CREATE;
    }

    if (clearExistingInfo)
    {
        // Reset display window information.  The image buffer still needs to be freed.
        m_DisplayWindow.rollGuideLine = TRUE;
        m_DisplayWindow.bkColor = 0x00d8e9ec;		// Button face color of Windows

        _ClearOverlayText();
    }

    m_DisplayWindow.bIsSetDisplayWindow = FALSE;

    _DrawClientWindow(m_pAlgo->m_OutResultImg, m_DisplayWindow);

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

        default:
            return IBSU_ERR_NOT_SUPPORTED;
    }

    return IBSU_STATUS_OK;
}

int	CMainCapture::ClientWindow_SetOverlayText(const wchar_t *fontName, const int fontSize, const BOOL fontBold,
        const wchar_t *text, const int posX, const int posY, const DWORD textColor)
{
    CThreadSync Sync;			// enzyme add 2013-02-05

    OverlayText textInfo;

    memset(&textInfo, 0, sizeof(OverlayText));

    wcscpy(textInfo.familyName, fontName);
    wcscpy(textInfo.text, text);
    textInfo.size = fontSize;
    textInfo.bold = fontBold;
    textInfo.x = posX;
    textInfo.y = posY;
    textInfo.color = textColor;

    if (textInfo.x < 0 || textInfo.y < 0)
    {
        return IBSU_ERR_INVALID_PARAM_VALUE;
    }

    if (m_DisplayWindow.bIsSetDisplayWindow)
    {
        OverlayText *pOverlayText = NULL;
        if ((pOverlayText = _FindOverlayTextInList(textInfo.x, textInfo.y)) != NULL)
        {
            memcpy(pOverlayText, &textInfo, sizeof(OverlayText));
        }
        else
        {
            pOverlayText = new OverlayText();
            memcpy(pOverlayText, &textInfo, sizeof(OverlayText));
            m_pOverlayText.push_back(pOverlayText);
        }

        _DrawClientWindow(m_pAlgo->m_OutResultImg, m_DisplayWindow);

        return IBSU_STATUS_OK;
    }

    return IBSU_ERR_CLIENT_WINDOW;
}

void CMainCapture::_DrawClientWindow(BYTE *InImg, DisplayWindow dispWindow)
{
    //	CThreadSync Sync;		// enzyme delete 2013-02-05

    // Overlay text for display on window with double buffering
    HDC		hdc = ::GetDC(m_hWindow);
    HDC		hMemDc;
    HBITMAP hBmpBuffer;
    HBRUSH	brush, old_brush;

    hMemDc = ::CreateCompatibleDC(hdc);
    hBmpBuffer = ::CreateCompatibleBitmap(hdc, dispWindow.Width, dispWindow.Height);
    HGDIOBJ hOldBmp = ::SelectObject(hMemDc, hBmpBuffer);

    brush = ::CreateSolidBrush((COLORREF)dispWindow.bkColor);
    old_brush = (HBRUSH)::SelectObject(hMemDc, brush);

    RECT rect;
    rect.left = 0;
    rect.right = dispWindow.Width;
    rect.top = 0;
    rect.bottom = dispWindow.Height;

    ::FillRect(hMemDc, &rect, brush);
    ::SelectObject(hMemDc, old_brush);
    ::DeleteObject(brush);

    ////////////////////////////////////////////////////////////////////////////////////////////
    // enzyme delete 2013-02-05 - we used same code to _DrawClientWindow() to remove "CThreadSync Sync;" on this function
    // Then we can save capture time from each devices.
    m_BitmapInfo->bmiHeader.biHeight = dispWindow.imgHeight;
    m_BitmapInfo->bmiHeader.biSizeImage = dispWindow.imgWidth * dispWindow.imgHeight;
    m_BitmapInfo->bmiHeader.biWidth = dispWindow.imgWidth;
    ////////////////////////////////////////////////////////////////////////////////////////////

    if (dispWindow.bIsSetDisplayWindow)
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

        ::StretchDIBits(hMemDc, dispWindow.x, dispWindow.y, dispWindow.imgWidth, dispWindow.imgHeight, 0, 0,
                        dispWindow.imgWidth, dispWindow.imgHeight, dispWindow.image, m_BitmapInfo, DIB_RGB_COLORS, SRCCOPY);

        OverlayText *pOverlayText;
        for (int i = 0; i < (int)m_pOverlayText.size(); i++)
        {
            pOverlayText = m_pOverlayText.at(i);

            // drawing
            HFONT	font, old_font;
            int		bold = 0;
            RECT	fontRect;

            fontRect.left = pOverlayText->x;
            fontRect.right = dispWindow.Width;
            fontRect.top = pOverlayText->y;
            fontRect.bottom = dispWindow.Height;

            if (pOverlayText->bold)
            {
                bold = FW_BOLD;
            }

			int font_size = -::MulDiv(pOverlayText->size, GetDeviceCaps(hMemDc, LOGPIXELSY), 72);

#ifdef WINCE
			font = ::CreateFont(font_size, 0, 0, 0, bold, 0, 0, 0, ANSI_CHARSET, 3, 2, 1,
                                VARIABLE_PITCH | FF_ROMAN, pOverlayText->familyName);
#else
			font = ::CreateFontW(font_size, 0, 0, 0, bold, 0, 0, 0, ANSI_CHARSET, 3, 2, 1,
                                VARIABLE_PITCH | FF_ROMAN, pOverlayText->familyName);
#endif
            old_font = (HFONT)::SelectObject(hMemDc, font);

            ::SetTextColor(hMemDc, (COLORREF)pOverlayText->color);
            ::SetBkMode(hMemDc, TRANSPARENT);

			
			::DrawTextW(hMemDc, pOverlayText->text, -1, &fontRect, DT_LEFT | DT_NOCLIP);
            ::SelectObject(hMemDc, old_font);
            ::DeleteObject(font);
        }


#ifdef __IBSCAN_ULTIMATE_SDK__
        // is set to TRUE of enumeration of ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE
        if (dispWindow.rollGuideLine)
        {
            if (m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER &&
                    (m_pAlgo->m_cImgAnalysis.finger_count > 0 || m_pAlgo->g_Rolling_Saved_Complete == TRUE) &&
                    (m_pAlgo->m_rollingStatus == 1 || m_pAlgo->m_rollingStatus == 2))
            {
                HPEN	pen, old_pen;

                // Guide line for rolling
                if (m_pAlgo->m_rollingStatus == 1)
                {
                    pen = ::CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
                }
                else if (m_pAlgo->m_rollingStatus == 2)
                {
                    pen = ::CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
                }
                old_pen = (HPEN)::SelectObject(hMemDc, pen);

                if (m_pAlgo->g_LastX > -1)
                {
                    int startx = (dispWindow.Width - dispWindow.imgWidth) / 2;
                    ::MoveToEx(hMemDc, m_pAlgo->g_LastX * dispWindow.imgWidth / m_UsbDeviceInfo.CisImgWidth + startx, 1, NULL);
                    ::LineTo(hMemDc, m_pAlgo->g_LastX * dispWindow.imgWidth / m_UsbDeviceInfo.CisImgWidth + startx, dispWindow.imgHeight - 1);
                }

                ::SelectObject(hMemDc, old_pen);
                ::DeleteObject(pen);
            }
            else if (m_pAlgo->m_cImgAnalysis.finger_count == 0)
            {
                if (m_pAlgo->g_Rolling_Saved_Complete == FALSE)
                {
                    m_pAlgo->m_rollingStatus = 0;
                }
            }
        }
#endif
    }
    else
    {
#ifdef __IBSCAN_ULTIMATE_SDK__
        memset(dispWindow.image, 0xFF, dispWindow.imgWidth * dispWindow.imgHeight);
#else
        memset(dispWindow.image, 0x00, dispWindow.imgWidth * dispWindow.imgHeight);
#endif
        ::StretchDIBits(hMemDc, dispWindow.x, dispWindow.y, dispWindow.imgWidth, dispWindow.imgHeight, 0, 0,
                        dispWindow.imgWidth, dispWindow.imgHeight, dispWindow.image, m_BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
    }

    ::BitBlt(hdc, 0, 0, dispWindow.Width, dispWindow.Height, hMemDc, 0, 0, SRCCOPY);
    // Always free the memory DC first, then the bitmap to avoid common resource leaks.
    ::SelectObject(hMemDc, hOldBmp);
    ::DeleteObject(hMemDc);
    ::DeleteObject(hBmpBuffer);
    ::ReleaseDC(m_hWindow, hdc);
}

int CMainCapture::_GetSerialNumber_Curve(PUCHAR  Buffer)
{
    UCHAR				strSerial[64];
    int					nResult;
    int					i;

    if (m_hLibUsbDriver == NULL)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

	libusb_device *dev = libusb_get_device( m_hLibUsbDriver );  // Should never fail
	if( dev == NULL ) 
	{
		return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
	}
	
	struct libusb_device_descriptor desc;
	if( libusb_get_device_descriptor(dev, &desc) != 0)
	{
		return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
	}

    nResult = libusb_get_string_descriptor(m_hLibUsbDriver, desc.iSerialNumber, 0x03, strSerial, sizeof(strSerial));

    if (nResult  < 0)
    {
        return IBSU_ERR_CHANNEL_IO_READ_FAILED;
    }

    int nCharacter = (int)(nResult / 2 - 1);

    if (nCharacter > 9)
    {
        nCharacter = 9;
    }

    for (i = 0; i < nCharacter; i++)
    {
        Buffer[i] = strSerial[i * 2 + 2];
    }

    Buffer[i] = 0;

    return IBSU_STATUS_OK;
}


#endif //ifdef __USE_LIBUSBX_DRIVER__