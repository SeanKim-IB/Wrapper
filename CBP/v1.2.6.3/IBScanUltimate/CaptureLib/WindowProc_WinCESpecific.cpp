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
// (WINCE uses this driver)
#ifdef __USE_LIBUSBX_DRIVER__

#include "../IBScanUltimateDLL.h"
#include <math.h>
#include <winsock.h> // For timeval struct

////////////////////////////////////////////////////////////////////////////////
// Implementation

// WinCE doesn't have this Win32 function so implement it here
static void GetSystemTimeAsFileTime(FILETIME* pSystemAsFileTime)
{
    SYSTEMTIME st;
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, pSystemAsFileTime);
}

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

int CMainCapture::_UsbBulkOutIn(int outEp, UCHAR uiCommand, UCHAR *outData, LONG outDataLen,
                                int inEp, UCHAR *inData, LONG inDataLen, LONG *nBytesRead, int timeout)
{
    /////////////////////////////////////////////////////////////////////////////////////
    // 2013-06-13 enzyme add - add code to avoid the slower frame speed
    // on the multi-devices environment because Columbo and Curve has a different logic to get a frame
    // call EP1OUT(GET_ONE_FRAME) then get frame from EP6IN
    // So previous code used the "CThreadSync Sync;" to avoid the dead lock,
    // but it makes the slower frame speed on the multi-devices environment


	#ifdef __G_DEBUG__
	DEBUGMSG(1, (TEXT("CaptureLib:CMainCapture::_UsbBulkOutIn Entry (outEp=%d, uiCommand=0x%x, inEp=0x%x)\n"), outEp, uiCommand, inEp));
	#endif

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
            DEBUGMSG(1, (TEXT("Unexpected _UsbBulkOutIn() failed = IBSU_ERR_CHANNEL_IO_COMMAND_FAILED")));
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
#ifdef __G_DEBUG__
            DEBUGMSG(1, (TEXT("Unexpected _UsbBulkOutIn() failed = IBSU_ERR_CHANNEL_IO_INVALID_HANDLE\n")));
#endif

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
#ifdef __G_DEBUG__
            DEBUGMSG(1, (TEXT("Unexpected _UsbBulkOutIn() failed = IBSU_ERR_CHANNEL_IO_WRITE_FAILED\n")));
#endif
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
#ifdef __G_DEBUG__
            DEBUGMSG(1, (TEXT("Unexpected _UsbBulkOutIn() failed = IBSU_ERR_CHANNEL_IO_READ_FAILED\n")));
#endif
			return IBSU_ERR_CHANNEL_IO_READ_FAILED;
        }
    }
	#ifdef __G_DEBUG__
	if (inEp >= 0)
	{
		if( nActualLength >= 4 )
		{
			DEBUGMSG(1, (TEXT("CaptureLib:CMainCapture::_UsbBulkOutIn Exit IBSU_STATUS_OK (Bytes Read=0x%x, first four bytes %02x,%02x,%02x,%02x)\n"), 
				nActualLength, *inData, *(inData+1), *(inData+2), *(inData+3)));
		}
		else
		{
			DEBUGMSG(1, (TEXT("CaptureLib:CMainCapture::_UsbBulkOutIn Exit IBSU_STATUS_OK (Bytes Read=0x%x)\n"), 
				nActualLength ));
		}
	}
	else
	{
		DEBUGMSG(1, (TEXT("CaptureLib:CMainCapture::_UsbBulkOutIn Exit IBSU_STATUS_OK (No Bytes Expected or Read\n")));
	}
	#endif

    return IBSU_STATUS_OK;
}

int CMainCapture::_UsbBulkIn(int ep, UCHAR *data, LONG dataLen, LONG *nBytesRead, int timeout)
{
    int		nResult;
	int		nActualLength = 0;

#ifdef __G_DEBUG__
	DEBUGMSG(1,(TEXT("CaptureLib:CMainCapture::_UsbBulkIn Entry (ep=%d)\n"), ep));
#endif

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

	#ifdef __G_DEBUG__
	DEBUGMSG(1,(TEXT("CaptureLib:CMainCapture::_UsbBulkIn Exit( ep=%d) bytes read = 0x%x\n"), ep, nActualLength));
	#endif

    return IBSU_STATUS_OK;
}


int CMainCapture::_UsbBulkOut(int ep, UCHAR uiCommand, UCHAR *data, LONG dataLen, int timeout)
{
    CThreadSync Sync;
    IO_PACKET	io_packet;
    int			nResult;
    int			nByteCount;
	int			nActualLength;

#ifdef __G_DEBUG__
	DEBUGMSG(1,(TEXT("CaptureLib:CMainCapture::_UsbBulkOut Entry (ep=%d, uiCommand=0x%x)\n"), ep, uiCommand));
#endif

	
	
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
		#ifdef __G_DEBUG__
			DEBUGMSG(1,(TEXT("CaptureLib:CMainCapture::_UsbBulkOut Exit IBSU_ERR_CHANNEL_IO_WRITE_FAILED\n")));
		#endif
		return IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
    }

#ifdef __G_DEBUG__
	DEBUGMSG(1,(TEXT("CaptureLib:CMainCapture::_UsbBulkOut Exit IBSU_STATUS_OK\n")));
#endif

    return IBSU_STATUS_OK;
}


// Initialized libusb if it hasn't yet been initialized
// Note: libusb can't be initialized when this DLL is loaded because it waits for
// a thread it creates to start executing which won't happen while DLLMain is still 
// running. 

int CIBScanUltimateApp::DelayedLibUSBInit( void )
{
	CThreadSync Sync;
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

	if( m_hLibUsbDriver != NULL)
	{
		// HACKHACK: Add logging here possibly _POstTraceLogCallback?
		DEBUGMSG(1, (TEXT("_OpenUSBDevice called when device is already open\r\n")));
        *errorNo = IBSU_ERR_DEVICE_ACTIVE;
        return FALSE;
    }
	
	libusb_context *ctx = ((CIBScanUltimateApp *) AfxGetApp())->m_LibUSBContext; // &&&&
	r = libusb_get_device_list(ctx, &dev_list);
	if (r < 0) {
//		usbi_err("get_device_list failed with error %d", r);
        *errorNo = IBSU_ERR_DEVICE_NOT_MATCHED;
        return FALSE;
	}

	/* Iterate over the device list, finding the desired device */
	dev_list_len = r;
	for (i = 0; i < dev_list_len; i++) {
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

    openretval = libusb_open(dev, &m_hLibUsbDriver);

	if( openretval != 0 || !m_hLibUsbDriver )
	{
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

void CMainCapture::_SetPID_VID(UsbDeviceInfo *pScanDevDesc, char *productID)
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

	int LibUsbSpeed = libusb_get_device_speed(dev);
	switch( LibUsbSpeed )
	{
		case LIBUSB_SPEED_LOW: 
			*speed = USB_LOW_SPEED;
			break;
		case LIBUSB_SPEED_FULL:
			*speed = USB_FULL_SPEED;
			break;
		case LIBUSB_SPEED_HIGH:
			*speed = USB_HIGH_SPEED;
			break;
		case LIBUSB_SPEED_SUPER:
			*speed = USB_SUPER_SPEED;
			break;
		case LIBUSB_SPEED_UNKNOWN:
			struct libusb_device_descriptor desc;
			if( libusb_get_device_descriptor(dev, &desc) == 0)
			{
				if (desc.bcdUSB == 0x0200)
				{
// The Intermec CN70 series uses Full speed even though it's a USB 2.0 interface
// So this code reports that fact.
#if defined(WINCE) && defined(WINCE_FORCE_FULLSPEEDUSB)
					*speed = USB_FULL_SPEED;
#else
					*speed = USB_HIGH_SPEED;
#endif 
				}
				else
				{
					*speed = USB_FULL_SPEED;
				}
			}
	}		
    return IBSU_STATUS_OK;
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