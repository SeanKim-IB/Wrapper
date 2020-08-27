#pragma once
#include <winioctl.h>
#include <Usb100.h>
#include <Setupapi.h>


#define Ezusb_IOCTL_INDEX		0x0800



#define IOCTL_Ezusb_GET_DEVICE_DESCRIPTOR CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+1,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_Ezusb_RESET                 CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+12,\
                                                   METHOD_IN_DIRECT,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_Ezusb_GET_STRING_DESCRIPTOR CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+17,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

/*
	Perform an IN transfer over the specified bulk or interrupt pipe.

	lpInBuffer			: BULK_TRANSFER_CONTROL stucture specifying the pipe number to read from
	nInBufferSize		: sizeof(BULK_TRANSFER_CONTROL)
	lpOutBuffer			: Buffer to hold data read from the device.  
	nOutputBufferSize	: size of lpOutBuffer.  This parameter determines
							the size of the USB transfer.
	lpBytesReturned		: actual number of bytes read
*/ 
#define IOCTL_EZUSB_BULK_READ             CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+19,\
                                                   METHOD_OUT_DIRECT,  \
                                                   FILE_ANY_ACCESS)

/*
	Perform an OUT transfer over the specified bulk or interrupt pipe.

	lpInBuffer			: BULK_TRANSFER_CONTROL stucture specifying the pipe number to write to
	nInBufferSize		: sizeof(BULK_TRANSFER_CONTROL)
	lpOutBuffer			: Buffer of data to write to the device
	nOutputBufferSize	: size of lpOutBuffer.  This parameter determines
							the size of the USB transfer.
	lpBytesReturned		: actual number of bytes written
*/
#define IOCTL_EZUSB_BULK_WRITE            CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+20,\
                                                   METHOD_IN_DIRECT,  \
                                                   FILE_ANY_ACCESS)


class CCurveUSBInterface
{
public:
	CCurveUSBInterface(GUID guid);
	~CCurveUSBInterface(void);
	BOOL Open(int deviceIndex, int *errorNo);
	void Close(void);

	int Vid();
	int Pid();

	int SyncReadPipe (UCHAR  pipeIndex, PUCHAR  Buffer, ULONG  BufferLength, PULONG  LengthTransferred, int timeout);
	int ReadPipe (UCHAR  PipeIndex, PUCHAR  Buffer, ULONG  BufferLength, PULONG  LengthTransferred, int timeout);

	int WritePipe (UCHAR  PipeIndex, PUCHAR  Buffer, ULONG  BufferLength, PULONG  LengthTransferred, int timeout);
	int ResetUsbPort(); 
	int GetSerialNumber (PUCHAR  Buffer);
	int GetUsbSpeed(UsbSpeed *speed);

private:
	int m_maxEndpoints;
	UCHAR* m_endpointMap;
	BOOL CreateDeviceHandle(int deviceIndex, int *errorNo);
	BOOL GetWinUSBHandle(int *errorNo);
	BOOL MapEndpoints(int *errorNo);
	HANDLE m_deviceHandle;
	GUID m_guid;
	int m_vid;
	int m_pid;



};
