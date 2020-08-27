#include "StdAfx.h"

#include "IBSU_Global_Def.h"

// Do not include this code if not using the WINUSB driver
#if defined(_WINDOWS) && defined(__USE_WINUSB_DRIVER__)

#include "WinUSBInterface_Curve.h"

// Linked libraries - from DDK or custom folder ./WinUSB
#pragma comment (lib , "setupapi.lib" )

//#define __BULK_TIMEOUT__ 3000  // Milliseconds

CCurveUSBInterface::CCurveUSBInterface(GUID guid)
{
    m_deviceHandle = INVALID_HANDLE_VALUE;
    m_guid = guid;
    m_vid = 0;
    m_pid = 0;
}

CCurveUSBInterface::~CCurveUSBInterface(void)
{
    Close();
}

BOOL CCurveUSBInterface::CreateDeviceHandle(int deviceIndex, int *errorNo)
{
    *errorNo = IBSU_STATUS_OK;

    BOOL bResult = TRUE;
    HDEVINFO hDeviceInfo;
    SP_DEVINFO_DATA DeviceInfoData;

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = NULL;

    ULONG requiredLength = 0;

    LPTSTR lpDevicePath = NULL;

    DWORD index = 0;

    // Get information about all the installed devices for the specified
    // device interface class.
    hDeviceInfo = SetupDiGetClassDevs(
                      &m_guid,
                      NULL,
                      NULL,
                      DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hDeviceInfo == INVALID_HANDLE_VALUE)
    {
        // ERROR
        //printf("Error SetupDiGetClassDevs: %d.\n", GetLastError());
        bResult = FALSE;
        goto done;
    }

    //Enumerate all the device interfaces in the device information set.
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (index = 0; SetupDiEnumDeviceInfo(hDeviceInfo, index, &DeviceInfoData); index++)
    {
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

        //Get information about the device interface.
        bResult = SetupDiEnumDeviceInterfaces(
                      hDeviceInfo,
                      &DeviceInfoData,
                      &m_guid,
                      0, // index,  -- First interface only
                      &deviceInterfaceData);

        // Check if last item
        if (GetLastError() == ERROR_NO_MORE_ITEMS)
        {
            break;
        }

        //Check for some other error
        if (!bResult)
        {
            //printf("Error SetupDiEnumDeviceInterfaces: %d.\n", GetLastError());
            goto done;
        }

        //Interface data is returned in SP_DEVICE_INTERFACE_DETAIL_DATA
        //which we need to allocate, so we have to call this function twice.
        //First to get the size so that we know how much to allocate
        //Second, the actual call with the allocated buffer

        bResult = SetupDiGetDeviceInterfaceDetail(
                      hDeviceInfo,
                      &deviceInterfaceData,
                      NULL, 0,
                      &requiredLength,
                      NULL);


        //Check for some other error
        if (!bResult)
        {
            if ((ERROR_INSUFFICIENT_BUFFER == GetLastError()) && (requiredLength > 0))
            {
                //we got the size, allocate buffer
                pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, requiredLength);

                if (!pInterfaceDetailData)
                {
                    // ERROR
                    //printf("Error allocating memory for the device detail buffer.\n");
                    goto done;
                }
            }
            else
            {
                //printf("Error SetupDiEnumDeviceInterfaces: %d.\n", GetLastError());
                goto done;
            }
        }

        //get the interface detailed data
        pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        //Now call it with the correct size and allocated buffer
        bResult = SetupDiGetDeviceInterfaceDetail(
                      hDeviceInfo,
                      &deviceInterfaceData,
                      pInterfaceDetailData,
                      requiredLength,
                      NULL,
                      &DeviceInfoData);

        //Check for some other error
        if (!bResult)
        {
            //printf("Error SetupDiGetDeviceInterfaceDetail: %d.\n", GetLastError());
            goto done;
        }


        //copy device path
        size_t nLength = _tcslen((TCHAR *)pInterfaceDetailData->DevicePath) + 1;
        lpDevicePath = (TCHAR *) LocalAlloc(LPTR, nLength * sizeof(TCHAR));
        _tcsncpy(lpDevicePath, pInterfaceDetailData->DevicePath, nLength);

        lpDevicePath[nLength - 1] = 0;

        //_tprintf(_T("Device path:  %s\n"), lpDevicePath);

        // Extract and save VID and PID
        // enzyme modify 2012-10-27 I don't know why different "Vid/Pid vs vid/pid
        //		TCHAR * vidLoc = _tcsstr(lpDevicePath, _T("vid_"));
        //		TCHAR * pidLoc = _tcsstr(lpDevicePath, _T("pid_"));
        TCHAR *vidLoc = _tcsstr(_tcsupr(lpDevicePath), _T("VID_"));
        TCHAR *pidLoc = _tcsstr(_tcsupr(lpDevicePath), _T("PID_"));

        if (vidLoc)
        {
            TCHAR *endChar;
            m_vid = _tcstol(vidLoc + 4, &endChar, 16);
        }

        if (pidLoc)
        {
            TCHAR *endChar;
            m_pid = _tcstol(pidLoc + 4, &endChar, 16);
        }

        if (index == deviceIndex)
        {
            break;  // Have target device index
        }

    }

    if (!lpDevicePath)
    {
        //Error.
        //printf("Error %d.", GetLastError());
        bResult = FALSE;
        goto done;
    }

    //Open the device
    m_deviceHandle = CreateFile(
                         lpDevicePath,
                         GENERIC_READ | GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_EXISTING,
                         FILE_FLAG_OVERLAPPED,
                         NULL);

    if (m_deviceHandle == INVALID_HANDLE_VALUE)
    {
        //Error.
        //    printf("Error %d.", GetLastError());
        if (GetLastError() == ERROR_ACCESS_DENIED)
        {
            *errorNo = IBSU_ERR_DEVICE_ACTIVE;
            LocalFree(lpDevicePath);
            LocalFree(pInterfaceDetailData);
            if (!SetupDiDestroyDeviceInfoList(hDeviceInfo))
            {
                // Handle destroy error
            }

            return FALSE;
        }

        bResult = FALSE;
        goto done;
    }
    else
    {
        bResult = TRUE;
        LocalFree(lpDevicePath);
        LocalFree(pInterfaceDetailData);
        if (!SetupDiDestroyDeviceInfoList(hDeviceInfo))
        {
            // Handle destroy error
        }

        return TRUE;
    }

done:
    *errorNo = IBSU_ERR_DEVICE_NOT_MATCHED;
    LocalFree(lpDevicePath);
    LocalFree(pInterfaceDetailData);
    if (!SetupDiDestroyDeviceInfoList(hDeviceInfo))
    {
        // Handle destroy error
    }

    return bResult;
}

int CCurveUSBInterface::Vid()
{
    return m_vid;

}

int CCurveUSBInterface::Pid()
{
    return m_pid;
}

BOOL CCurveUSBInterface::Open(int deviceIndex, int *errorNo)
{
    *errorNo = IBSU_STATUS_OK;

    BOOL bResult = CreateDeviceHandle(deviceIndex, errorNo);
    if (!bResult)
    {
        Close();
    }
    return bResult;
}

void CCurveUSBInterface::Close(void)
{
    if (m_deviceHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_deviceHandle);
        m_deviceHandle = INVALID_HANDLE_VALUE;
    }
    m_vid = 0;
    m_pid = 0;
}

int CCurveUSBInterface::SyncReadPipe(UCHAR  pipeIndex, PUCHAR  Buffer, ULONG  BufferLength, PULONG  LengthTransferred, int timeout)
{
    if (m_deviceHandle == INVALID_HANDLE_VALUE)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    DWORD					nByte;
    BULK_TRANSFER_CONTROL	tBulkCtl;

    tBulkCtl.pipeNum = pipeIndex;

    BOOL bResult = DeviceIoControl(m_deviceHandle,
                                   IOCTL_EZUSB_BULK_READ,
                                   &tBulkCtl, sizeof(BULK_TRANSFER_CONTROL),
                                   Buffer, BufferLength,
                                   &nByte, NULL);
    if (LengthTransferred != NULL)
    {
        *LengthTransferred = nByte;
    }

    if (bResult)
    {
        return IBSU_STATUS_OK;
    }
    else
    {
        return IBSU_ERR_CHANNEL_IO_READ_FAILED;
    }
}

int CCurveUSBInterface::ReadPipe(UCHAR  pipeIndex, PUCHAR  Buffer, ULONG  BufferLength, PULONG  LengthTransferred, int timeout)
{
    if (m_deviceHandle == INVALID_HANDLE_VALUE)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    DWORD					nByte;
    BULK_TRANSFER_CONTROL	tBulkCtl;

    tBulkCtl.pipeNum = pipeIndex;

    BOOL bResult = DeviceIoControl(m_deviceHandle,
                                   IOCTL_EZUSB_BULK_READ,
                                   &tBulkCtl, sizeof(BULK_TRANSFER_CONTROL),
                                   Buffer, BufferLength,
                                   &nByte, NULL);
    if (LengthTransferred != NULL)
    {
        *LengthTransferred = nByte;
    }

    if (bResult)
    {
        return IBSU_STATUS_OK;
    }
    else
    {
        return IBSU_ERR_CHANNEL_IO_READ_FAILED;
    }
}

int CCurveUSBInterface::WritePipe(UCHAR  pipeIndex, PUCHAR  Buffer, ULONG  BufferLength, PULONG  LengthTransferred, int timeout)
{
    if (m_deviceHandle == INVALID_HANDLE_VALUE)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    DWORD					nByte;
    BULK_TRANSFER_CONTROL	tBulkCtl;

    tBulkCtl.pipeNum = pipeIndex;

    BOOL bResult = DeviceIoControl(m_deviceHandle,
                                   IOCTL_EZUSB_BULK_WRITE,
                                   &tBulkCtl, sizeof(BULK_TRANSFER_CONTROL),
                                   Buffer,  BufferLength,
                                   &nByte, NULL);

    if (bResult)
    {
        return IBSU_STATUS_OK;
    }
    else
    {
        return IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
    }
}

int CCurveUSBInterface::ResetUsbPort()
{
    DWORD nByte;

    BOOL bResult = DeviceIoControl(m_deviceHandle,
                                   IOCTL_Ezusb_RESET,
                                   NULL, 0,
                                   NULL, 0,
                                   &nByte,	NULL);

    if (bResult)
    {
        return IBSU_STATUS_OK;
    }
    else
    {
        return IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
    }
}

int CCurveUSBInterface::GetSerialNumber(PUCHAR  Buffer)
{
    if (m_deviceHandle == INVALID_HANDLE_VALUE)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    GET_STRING_DESCRIPTOR_IN	GetStringDescriptorIn;
    DWORD					nByte;
    char					strSerial[64];
    int						i, pos;

    GetStringDescriptorIn.Index = 3;
    GetStringDescriptorIn.LanguageId = 0x0409;

    BOOL bResult = DeviceIoControl(m_deviceHandle,
                                   IOCTL_Ezusb_GET_STRING_DESCRIPTOR,
                                   &GetStringDescriptorIn,
                                   sizeof(GET_STRING_DESCRIPTOR_IN),
                                   strSerial, 64,
                                   &nByte, NULL);

    if (!bResult)
    {
        return IBSU_ERR_CHANNEL_IO_READ_FAILED;
    }

    int nCharacter = (int)(nByte / 2 - 1);

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

int CCurveUSBInterface::GetUsbSpeed(UsbSpeed *speed)
{
    if (m_deviceHandle == INVALID_HANDLE_VALUE)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    UCHAR	buffer[64] = {0};
    ULONG	cbSent = 0, cbRead = 0;
    int		nRc;

    *speed = USB_UNKNOWN_SPEED;
    buffer[0] = 0;
    buffer[1] = (UCHAR)NA2CTL_READ_STATUS;
    nRc = WritePipe(EP1OUT, buffer, 2, &cbSent, __BULK_TIMEOUT__);
    if (nRc != IBSU_STATUS_OK)
    {
        return nRc;
    }

    nRc = ReadPipe(EP1IN, buffer, 3, &cbRead, __BULK_TIMEOUT__);
    if (nRc != IBSU_STATUS_OK)
    {
        return nRc;
    }

    if (buffer[0] == 1)
    {
        *speed = USB_HIGH_SPEED;
    }
    else
    {
        *speed = USB_FULL_SPEED;
    }

    return nRc;
}

#endif // #if defined(_WINDOWS) && defined(__USE_WINUSB_DRIVER__)
