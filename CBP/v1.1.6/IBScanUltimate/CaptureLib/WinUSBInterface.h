#pragma once

//#include <winioctl.h>
// Include Windows headers
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

// Include WinUSB headers - Requires headers from Windows DDK or 
// Custom include folder ./WinUSB
#include <winusb.h>
#include <Usb100.h>
#include <Setupapi.h>

class CWinUSBInterface
{
public:
	CWinUSBInterface(GUID guid);
	~CWinUSBInterface(void);
	BOOL Open(int deviceIndex, int *errorNo);
	void Close(void);
	int		GetApiHandle();

	int Vid();
	int Pid();

	BOOL GetSerialNumber(char *pStrSerialNumber);

	int SyncReadPipe (UCHAR  pipeIndex, PUCHAR  Buffer, ULONG  BufferLength, PULONG  LengthTransferred, int timeout);
	int ReadPipe (UCHAR  PipeIndex, PUCHAR  Buffer, ULONG  BufferLength, PULONG  LengthTransferred, int timeout);

	int WritePipe (UCHAR  PipeIndex, PUCHAR  Buffer, ULONG  BufferLength, PULONG  LengthTransferred, int timeout);
	int GetUsbSpeed(UsbSpeed *speed); 

private:
	int m_maxEndpoints;
	UCHAR* m_endpointMap;
	BOOL CreateDeviceHandle(int deviceIndex, int *errorNo);
	BOOL GetWinUSBHandle(int *errorNo);
	BOOL MapEndpoints(int *errorNo);
	WINUSB_INTERFACE_HANDLE	m_hWinUSBHandle;
	HANDLE m_deviceHandle;
	OVERLAPPED m_overlap;
	HANDLE m_hIOEvent;
	GUID m_guid;
	int m_vid;
	int m_pid;



};
