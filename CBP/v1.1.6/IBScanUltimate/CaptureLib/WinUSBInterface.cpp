#include "StdAfx.h"

#include "IBSU_Global_Def.h"

// Do not include this code if not using the WINUSB driver
#if defined(_WINDOWS) && defined(__USE_WINUSB_DRIVER__)

#include "WinUSBInterface.h"

// Linked libraries - from DDK or custom folder ./WinUSB
#pragma comment (lib , "setupapi.lib" )
//
//	BFD - 11/6/12 - compiler logic for 64 & 32 bit
//
#ifdef __64BITSYSTEM
#pragma comment (lib , "winusb64.lib" )
#else
#pragma comment (lib , "winusb32.lib" )
#endif


//#define __BULK_TIMEOUT__ 3000  // Milliseconds

CWinUSBInterface::CWinUSBInterface(GUID guid)
{
    m_hWinUSBHandle = INVALID_HANDLE_VALUE;
    m_deviceHandle = INVALID_HANDLE_VALUE;
    m_hIOEvent = INVALID_HANDLE_VALUE;
    memset(&m_overlap, 0, sizeof(m_overlap));
    m_guid = guid;
    m_endpointMap = 0;
    m_maxEndpoints = 0;
    m_vid = 0;
    m_pid = 0;
}

CWinUSBInterface::~CWinUSBInterface(void)
{
    Close();
}

BOOL CWinUSBInterface::CreateDeviceHandle(int deviceIndex, int *errorNo)
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

    m_hIOEvent = CreateEvent(
                     NULL,    // default security attribute
                     TRUE,    // manual-reset event
                     TRUE,    // initial state = signaled
                     NULL);   // unnamed event object

    if (m_hIOEvent == NULL)
    {
        //printf("CreateEvent failed with %d.\n", GetLastError());
        bResult = FALSE;
        m_hIOEvent = INVALID_HANDLE_VALUE;
        goto done;
    }

    m_overlap.hEvent = m_hIOEvent;

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



BOOL CWinUSBInterface::GetSerialNumber(char *pStrSerialNumber)
{


#define String_index 3
#define Language_ID 0x0409

    struct _buff
    {
        USB_STRING_DESCRIPTOR stringDescriptor;
        char x[100]; // Space for string storage - bString
    } buff;


    ULONG cbSent = 0;
    if (WinUsb_GetDescriptor(m_hWinUSBHandle,
                             USB_STRING_DESCRIPTOR_TYPE,
                             String_index, Language_ID,
                             (PUCHAR)&buff,
                             sizeof(buff),
                             &cbSent))
    {

        // Success
        // Convert from Unicode into strValue
        // WinUsb_GetDescriptor always returns as Unicode - char * might be either
        WCHAR *pSource = buff.stringDescriptor.bString;
        char *pDest = pStrSerialNumber;
        int nChar = (buff.stringDescriptor.bLength - 2) / sizeof(WCHAR);  // Subtract first 2 structure elements from string length
        for (int i = 0; i < nChar; i++)
        {
            *pDest = (char)(*pSource);
            pDest++;
            pSource++;
        }
        *pDest = 0; // Nul terminator

        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

BOOL CWinUSBInterface::GetWinUSBHandle(int *errorNo)
{
    *errorNo = IBSU_STATUS_OK;

    if (m_deviceHandle == INVALID_HANDLE_VALUE)
    {
        *errorNo = IBSU_ERR_DEVICE_NOT_MATCHED;
        return FALSE;
    }

    BOOL bResult = WinUsb_Initialize(m_deviceHandle, &m_hWinUSBHandle);
    if (!bResult)
    {
        //Error.
        //    TRACE("WinUsb_Initialize Error %d.", GetLastError());
        *errorNo = IBSU_ERR_DEVICE_NOT_MATCHED;
        return FALSE;
    }

    return bResult;
}

int CWinUSBInterface::Vid()
{
    return m_vid;

}

int CWinUSBInterface::Pid()
{
    return m_pid;
}


// Create lookup table to map endpoints to pipe IDs
BOOL CWinUSBInterface::MapEndpoints(int *errorNo)
{
    *errorNo = IBSU_STATUS_OK;

    BOOL	bResult = TRUE;

    USB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
    ZeroMemory(&InterfaceDescriptor, sizeof(USB_INTERFACE_DESCRIPTOR));

    WINUSB_PIPE_INFORMATION  Pipe;
    ZeroMemory(&Pipe, sizeof(WINUSB_PIPE_INFORMATION));

    bResult = WinUsb_QueryInterfaceSettings(m_hWinUSBHandle, 0, &InterfaceDescriptor);

    if (bResult)
    {
        m_maxEndpoints = InterfaceDescriptor.bNumEndpoints;
        m_endpointMap = new UCHAR[m_maxEndpoints];
        for (int index = 0; index < InterfaceDescriptor.bNumEndpoints; index++)
        {
            bResult = WinUsb_QueryPipe(m_hWinUSBHandle, 0, index, &Pipe);

            if (bResult)
            {
                m_endpointMap[index] = Pipe.PipeId;

                // enzyme 2012-11-01 add To prevent hangup when ESD test because at time FPGA die
                int valPolicy = __BULK_TIMEOUT__;
				BOOL bRet = FALSE;
                if (Pipe.PipeId == 0x86)
                {
                    bRet = WinUsb_SetPipePolicy(m_hWinUSBHandle, Pipe.PipeId, PIPE_TRANSFER_TIMEOUT, sizeof(ULONG), &valPolicy);
                }

                if (Pipe.PipeId == 0x81 || Pipe.PipeId == 0x86 || Pipe.PipeId == 0x88)
                {
					valPolicy = 1;
                    bRet = WinUsb_SetPipePolicy(m_hWinUSBHandle, Pipe.PipeId, RESET_PIPE_ON_RESUME, sizeof(ULONG), &valPolicy);
					valPolicy = 1;
                    bRet = WinUsb_SetPipePolicy(m_hWinUSBHandle, Pipe.PipeId, AUTO_CLEAR_STALL, sizeof(ULONG), &valPolicy);
					valPolicy = 1;
                    bRet = WinUsb_SetPipePolicy(m_hWinUSBHandle, Pipe.PipeId, IGNORE_SHORT_PACKETS, sizeof(ULONG), &valPolicy);
					valPolicy = 1;
                    bRet = WinUsb_SetPipePolicy(m_hWinUSBHandle, Pipe.PipeId, AUTO_FLUSH, sizeof(ULONG), &valPolicy);
                }
                /*
                if (Pipe.PipeType == UsbdPipeTypeControl)
                {
                printf("Endpoint index: %d Pipe type: Control Pipe ID: %d.\n", index, Pipe.PipeId);
                }
                if (Pipe.PipeType == UsbdPipeTypeIsochronous)
                {
                printf("Endpoint index: %d Pipe type: Isochronous Pipe ID: %d.\n", index, Pipe.PipeId);
                }
                if (Pipe.PipeType == UsbdPipeTypeBulk)
                {
                if (USB_ENDPOINT_DIRECTION_IN(Pipe.PipeId))
                {
                printf("Endpoint index: %d Pipe type: Bulk In Pipe ID: %d.\n", index, printPipeID);
                pipeid->PipeInId = Pipe.PipeId;
                }
                if (USB_ENDPOINT_DIRECTION_OUT(Pipe.PipeId))
                {
                printf("Endpoint index: %d Pipe type: Bulk Out Pipe ID: %d.\n", index, printPipeID);
                pipeid->PipeOutId = Pipe.PipeId;
                }
                }
                if (Pipe.PipeType == UsbdPipeTypeInterrupt)
                {
                printf("Endpoint index: %d Pipe type: Interrupt Pipe ID: %d.\n", index, Pipe.PipeId);
                }
                */
            }
            else
            {
                continue;
            }
        }
    }
    else
    {
        *errorNo = IBSU_ERR_DEVICE_NOT_MATCHED;
    }

    return bResult;
}


BOOL CWinUSBInterface::Open(int deviceIndex, int *errorNo)
{
    *errorNo = IBSU_STATUS_OK;

    BOOL bResult = CreateDeviceHandle(deviceIndex, errorNo);
    if (bResult)
    {
        bResult = GetWinUSBHandle(errorNo);
    }

    if (bResult)
    {
        bResult = MapEndpoints(errorNo);
    }

    if (!bResult)
    {
        Close();
    }
    return bResult;
}



void CWinUSBInterface::Close(void)
{
    if (m_deviceHandle != INVALID_HANDLE_VALUE)
    {
		CloseHandle(m_deviceHandle);
		m_deviceHandle = INVALID_HANDLE_VALUE;
    }
    if (m_hWinUSBHandle != INVALID_HANDLE_VALUE)
    {
        WinUsb_Free(m_hWinUSBHandle);
        m_hWinUSBHandle = INVALID_HANDLE_VALUE;
    }
    if (m_hIOEvent != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hIOEvent);
        m_hIOEvent = INVALID_HANDLE_VALUE;
    }
    m_overlap.hEvent = INVALID_HANDLE_VALUE;
    if (m_endpointMap)
    {
        delete[] m_endpointMap;
    }
    m_endpointMap = 0;
    m_maxEndpoints = 0;
    m_vid = 0;
    m_pid = 0;
}

int CWinUSBInterface::SyncReadPipe(UCHAR  pipeIndex, PUCHAR  Buffer, ULONG  BufferLength, PULONG  LengthTransferred, int timeout)
{
    if (m_deviceHandle == INVALID_HANDLE_VALUE || m_hWinUSBHandle == INVALID_HANDLE_VALUE)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    if (pipeIndex >= m_maxEndpoints)
    {
        return IBSU_ERR_CHANNEL_IO_WRONG_PIPE_INDEX;
    }

    BOOL bResult = WinUsb_ReadPipe(m_hWinUSBHandle, m_endpointMap[pipeIndex], Buffer, BufferLength, LengthTransferred, 0);

    if (bResult)
    {
        return IBSU_STATUS_OK;
    }
    else
    {
        if (GetLastError() == ERROR_SEM_TIMEOUT)
        {
            return IBSU_ERR_CHANNEL_IO_READ_TIMEOUT;
        }
        else
        {
            return IBSU_ERR_CHANNEL_IO_READ_FAILED;
        }
    }
}

int CWinUSBInterface::ReadPipe(UCHAR  pipeIndex, PUCHAR  Buffer, ULONG  BufferLength, PULONG  LengthTransferred, int timeout)
{
    int lastError;

    if (m_deviceHandle == INVALID_HANDLE_VALUE || m_hWinUSBHandle == INVALID_HANDLE_VALUE)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    if (pipeIndex >= m_maxEndpoints)
    {
        return IBSU_ERR_CHANNEL_IO_WRONG_PIPE_INDEX;
    }

    ResetEvent(m_hIOEvent);
    BOOL bResult = WinUsb_ReadPipe(m_hWinUSBHandle, m_endpointMap[pipeIndex], Buffer, BufferLength, NULL, &m_overlap);

    if (!bResult)
    {
        // Error or IO still pending
        lastError = GetLastError();
        if (lastError == ERROR_IO_PENDING)
        {
            // Wait for result or timeout
            if (WaitForSingleObject(m_hIOEvent, timeout) == WAIT_OBJECT_0)
            {
                if (WinUsb_GetOverlappedResult(m_hWinUSBHandle, &m_overlap, LengthTransferred, TRUE))
                {
                    // Success
                    return IBSU_STATUS_OK;
                }
            }
            else
            {
                return IBSU_ERR_CHANNEL_IO_READ_TIMEOUT;
            }
        }
    }

    //  BOOL bResult = WinUsb_ReadPipe(m_hWinUSBHandle, m_endpointMap[pipeIndex], Buffer, BufferLength, LengthTransferred, 0);

    if (bResult)
    {
        return IBSU_STATUS_OK;
    }
    else
    {
        return IBSU_ERR_CHANNEL_IO_READ_FAILED;
    }
}

int CWinUSBInterface::WritePipe(UCHAR  pipeIndex, PUCHAR  Buffer, ULONG  BufferLength, PULONG  LengthTransferred, int timeout)
{
    int lastError;

    if (m_deviceHandle == INVALID_HANDLE_VALUE || m_hWinUSBHandle == INVALID_HANDLE_VALUE)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    if (pipeIndex >= m_maxEndpoints)
    {
        return IBSU_ERR_CHANNEL_IO_WRONG_PIPE_INDEX;
    }

    ResetEvent(m_hIOEvent);
    BOOL bResult = WinUsb_WritePipe(m_hWinUSBHandle, m_endpointMap[pipeIndex], Buffer, BufferLength, NULL, &m_overlap);
    if (!bResult)
    {
        // Error or IO still pending
        lastError = GetLastError();
        if (lastError == ERROR_IO_PENDING)
        {
            // Wait for result or timeout
            if (WaitForSingleObject(m_hIOEvent, timeout) == WAIT_OBJECT_0)
            {
                if (WinUsb_GetOverlappedResult(m_hWinUSBHandle, &m_overlap, LengthTransferred, TRUE))
                {
                    // Success
                    return IBSU_STATUS_OK;
                }
            }
            else
            {
                return IBSU_ERR_CHANNEL_IO_WRITE_TIMEOUT;
            }
        }
    }

    //  BOOL bResult = WinUsb_WritePipe(m_hWinUSBHandle, m_endpointMap[pipeIndex], Buffer, BufferLength, LengthTransferred, 0);

    if (bResult)
    {
        return IBSU_STATUS_OK;
    }
    else
    {
        return IBSU_ERR_CHANNEL_IO_WRITE_FAILED;
    }
}

int CWinUSBInterface::GetUsbSpeed(UsbSpeed *speed)
{
    ULONG	length;
    UCHAR	deviceSpeed;

    if (m_deviceHandle == INVALID_HANDLE_VALUE || m_hWinUSBHandle == INVALID_HANDLE_VALUE)
    {
        return IBSU_ERR_CHANNEL_IO_INVALID_HANDLE;
    }

    length = sizeof(UCHAR);
    BOOL bResult = WinUsb_QueryDeviceInformation(m_hWinUSBHandle, DEVICE_SPEED, &length, &deviceSpeed);
    if (bResult)
    {
        *speed = (UsbSpeed)deviceSpeed;
        return IBSU_STATUS_OK;
    }

    return IBSU_ERR_CHANNEL_IO_COMMAND_FAILED;
}

#endif // #if defined(_WINDOWS) && defined(__USE_WINUSB_DRIVER__)

