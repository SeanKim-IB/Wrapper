/////////////////////////////////////////////////////////////////////////////
/* 
DESCRIPTION:
    ReservedApi.h - Reserved API definition.
    http://www.integratedbiometrics.com/

NOTES:
    Copyright(C) Integrated Biometrics, 2011-2013
    
VERSION HISTORY:
 	February 1 2013     - 1.5.0 initialize
*/
/////////////////////////////////////////////////////////////////////////////


#pragma once

#include "IBScanUltimateApi_defs.h"
#include "IBScanUltimateApi_err.h"
#include "IBScanUltimate.h"

#ifdef __cplusplus
extern "C" { 
#endif


#define RESERVED_MIN_LE_VOLTAGE_VALUE			0      ///< Lowest LE voltage value
#define RESERVED_MAX_LE_VOLTAGE_VALUE			15     ///< Highest LE voltage value

/// Basic device info structure.
typedef struct tagRESERVED_DeviceInfo
{
	char serialNumber[IBSU_MAX_STR_LEN];               ///< Device serial number
    char productName[IBSU_MAX_STR_LEN];                ///< Device product name
    char interfaceType[IBSU_MAX_STR_LEN];              ///< Device interface type (USB, Firewire)
    char fwVersion[IBSU_MAX_STR_LEN];                  ///< Device firmware version
    char devRevision[IBSU_MAX_STR_LEN];                ///< Device revision
    int  handle;                                       ///< Return device handle
    BOOL IsHandleOpened;                               ///< Check if device handle is opened
    char vendorID[IBSU_MAX_STR_LEN];                   ///< Device manufacturer identifier
    char productionDate[IBSU_MAX_STR_LEN];             ///< Production date
	char serviceDate[IBSU_MAX_STR_LEN];                ///< Last service date
}
RESERVED_DeviceInfo;

///@name Reserved API Functions
///@{

// Modified IBSU_OpenDevice function
int WINAPI RESERVED_OpenDevice(
      const int  deviceIndex,                         ///< [in]  Zero-based device index for device to init
      const char *pReservedkey,                       ///< [in]  Reserved key which is made manufacturer
      int        *pHandle                             ///< [out] Function returns device handle to be used for subsequent function calls \n
                                                      ///<       Memory must be provided by caller
  );

// Modified IBSU_GetDeviceDescription function
int WINAPI RESERVED_GetDeviceInfo(
      const int              deviceIndex,               ///< [in]  Device handle obtained by IBSU_DeviceOpen()
      const char             *pReservedkey,             ///< [in]  Reserved key which is made manufacturer
      RESERVED_DeviceInfo    *pDeviceInfo               ///< [out] Basic device information \n
                                                        ///<       Memory must be provided by caller
    );

// WritteEEPROM function to write cypress firmware
int WINAPI RESERVED_WriteEEPROM(
      const int     handle,                             ///< [in]  Device handle obtained by IBSU_DeviceOpen()
      const char    *pReservedkey,                      ///< [in]  Reserved key which is made manufacturer
	  const WORD    addr,                               ///< [in]  Address of EEPROM
	  const BYTE    *pData,                             ///< [in]  Data buffer
      const int     len                                 ///< [in]  Length of data buffer
    );

// Modified IBSU_SetProperty function to write some property values for production
// (SerialNumber, ProductionDate, ServiceDate)
int WINAPI RESERVED_SetProperty(
      const int             handle,                     ///< [in]  Device handle obtained by IBSU_DeviceOpen()
      const char            *pReservedkey,              ///< [in]  Reserved key which is made manufacturer
      const IBSU_PropertyId propertyId,                 ///< [in]  Property identifier to set value for
      LPCSTR                propertyValue               ///< [in]  String containing property value
    );

// RESERVED_SetLEVoltage function to control LE voltage with Transformer
int WINAPI RESERVED_SetLEVoltage(
      const int     handle,                             ///< [in]  Device handle obtained by IBSU_DeviceOpen()
      const char    *pReservedkey,                      ///< [in]  Reserved key which is made manufacturer
      const int	    voltageValue                        ///< [in]  Contrast value (range: 0 <= value <= @ref RESERVED_MAX_LE_VOLTAGE_VALUE)
    );

// RESERVED_GetLEVoltage function to control LE voltage with Transformer
int WINAPI RESERVED_GetLEVoltage(
      const int     handle,                             ///< [in]  Device handle obtained by IBSU_DeviceOpen()
      const char    *pReservedkey,                      ///< [in]  Reserved key which is made manufacturer
      int	        *voltageValue                       ///< [out] LE voltage value (range: RESERVED_MIN_LE_VOLTAGE_VALUE <= value <= @ref RESERVED_MAX_LE_VOLTAGE_VALUE) \n
                                                        ///<       Memory must be provided by caller
    );

// Modified IBSU_BeginCaptureImage function without using Capture Thread
int WINAPI RESERVED_BeginCaptureImage(
      const int                   handle,               ///< [in]  Device handle obtained by IBSU_OpenDevice()
      const char                  *pReservedkey,        ///< [in]  Reserved key which is made manufacturer
      const IBSU_ImageType        imageType,            ///< [in]  Image type to capture
      const IBSU_ImageResolution  imageResolution,      ///< [in]  Requested capture resolution
      const DWORD                 captureOptions        ///< [in]  Bit coded capture options to use (see @ref CaptureOptions)
  );

// RESERVED_GetOneFrameImage function without using Capture Thread
int WINAPI RESERVED_GetOneFrameImage(
      const int     handle,                             ///< [in]  Device handle obtained by IBSU_DeviceOpen()
      const char    *pReservedkey,                      ///< [in]  Reserved key which is made manufacturer
      unsigned char *pRawImage,                         ///< [out] Raw capture image from camera (CIS, ROIC, ASIC and some on)
                                                        ///<       Memory must be provided by caller
      const int     imageLength                         ///< [in]  Image length of Raw capture image
  );

// RESERVED_SetFpgaRegister function to control FPGA register
int WINAPI RESERVED_SetFpgaRegister(
      const int              handle,                    ///< [in]  Device handle obtained by IBSU_DeviceOpen()
      const char             *pReservedkey,             ///< [in]  Reserved key which is made manufacturer
      const unsigned char    address,                   ///< [in]  Address of FPGA register (range: 0 <= value <= 0xFF)
      const unsigned char    value                      ///< [in]  Value of FPGA register (range: 0 <= value <= 0xFF)
    );

// RESERVED_GetFpgaRegister function to control FPGA register
int WINAPI RESERVED_GetFpgaRegister(
      const int              handle,                    ///< [in]  Device handle obtained by IBSU_DeviceOpen()
      const char             *pReservedkey,             ///< [in]  Reserved key which is made manufacturer
      const unsigned char    address,                   ///< [in]  Address of FPGA register (range: 0 <= value <= 0xFF)
      unsigned char          *pValue                    ///< [out] Value of FPGA register (range: 0 <= value <= 0xFF) \n
                                                        ///<       Memory must be provided by caller
    );

// Modified IBSU_CreateClientWindow function
int WINAPI RESERVED_CreateClientWindow(
      const int         handle,                          ///< [in] Device handle obtained by IBSU_OpenDevice()
      const char        *pReservedkey,                   ///< [in] Reserved key which is made manufacturer
      const IBSU_HWND   hWindow,                         ///< [in] Windows handle to draw
	  const DWORD		left,                            ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
	  const DWORD		top,                             ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
	  const DWORD		right,                           ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
	  const DWORD		bottom,                          ///< [in] Rectangle coordinates to draw(top, bottom, left, right)
	  const DWORD       rawImgWidth,                     ///< [in] Width of raw image
	  const DWORD       rawImgHeight                     ///< [in] height of raw image
    );

// RESERVED_DrawClientWindow function
int WINAPI RESERVED_DrawClientWindow(
      const int         handle,                          ///< [in]  Device handle obtained by IBSU_OpenDevice()
      const char        *pReservedkey,                   ///< [in]  Reserved key which is made manufacturer
      unsigned char     *drawImage                       ///< [in]  image buffer to draw on Client window.
    );

// RESERVED_UsbBulkOutIn function
int WINAPI RESERVED_UsbBulkOutIn(
      const int             handle,                      ///< [in]  Device handle obtained by IBSU_OpenDevice()
      const char            *pReservedkey,               ///< [in]  Reserved key which is made manufacturer
      const int             outEp,                       ///< [in]  BulkOut endpoint
      const unsigned char   uiCommand,
	  unsigned char         *outData,                    ///< [in]  send data (You can use 'NULL')
	  const int             outDataLen,
	  const int             inEp,                        ///< [in]  BulkIn endpoint
	  unsigned char         *inData,                     ///< [in]  receive data (You can use 'NULL')
	  const int             inDataLen,
	  int                   *nBytesRead                  ///< [out] Received data count (You can use 'NULL')
    );

// RESERVED_InitializeCamera function
int WINAPI RESERVED_InitializeCamera(
      const int             handle,                      ///< [in]  Device handle obtained by IBSU_OpenDevice()
      const char            *pReservedkey                ///< [in]  Reserved key which is made manufacturer
    );


#ifdef __cplusplus
} // extern "C"
#endif
