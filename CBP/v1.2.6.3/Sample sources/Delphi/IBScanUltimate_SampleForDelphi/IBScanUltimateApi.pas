unit IBScanUltimateApi;

(*
****************************************************************************************************
* IBScanUltimateApi.h
*
* DESCRIPTION:
*     API functions for IBScanUltimate.
*     http://www.integratedbiometrics.com
*
* NOTES:
*     Copyright (c) Integrated Biometrics, 2009-2017
*
* HISTORY:
*     2012/04/06  1.0.0  Created.
*     2012/05/29  1.1.0  Added blocking API functions (IBSU_AsyncOpenDevice(), IBSU_BGetImage(), 
*                            IBSU_BGetInitProgress(), IBSU_BGetClearPlatenAtCapture()).
*     2012/11/06  1.4.1  Added rolling and extended open API functions (IBSU_BGetRollingInfo(),
*                            IBSU_OpenDeviceEx()).
*     2013/03/20  1.6.0  Added API function to support IBScanMatcher integration 
*                            (IBSU_GetIBSM_ResultImageInfo(), IBSU_GetNFIQScore()).
*     2013/04/05  1.6.2  Added API function to enable or disable trace log at run-time 
*                            (IBSU_EnableTraceLog()).
*     2013/08/03  1.6.9  Reformatted.
*     2013/10/14  1.7.0  Added API functions to acquire an image from a device (blocking for resultEx),
*                        deregister a callback function, show (or remove) an overlay object, 
*                        show (or remove) all overlay objects, add an overlay text, modify an existing
*                        overlay text, add an overlay line, modify an existing line, add an overlay
*                        quadrangle, modify an existing quadrangle, add an overlay shape, modify an
*                        overlay shape, save image to bitmap memory
*                        (IBSU_BGetImageEx(), IBSU_ReleaseCallbacks(), IBSU_ShowOverlayObject,
*                         IBSU_ShowAllOverlayObject(), IBSU_RemoveOverlayObject(), IBSU_RemoveAllOverlayObject(),
*                         IBSU_AddOverlayText(), IBSU_ModifyOverlayText(), IBSU_AddOverlayLine(),
*                         IBSU_ModifyOverlayLine(), IBSU_AddOverlayQuadrangle(), IBSU_ModifyOverlayQuadrangle(),
*                         IBSU_AddOverlayShape(), IBSU_ModifyOverlayShape(), IBSU_SaveBitmapMem())
*     2014/07/23  1.8.0  Reformatted.
*                        Added API functions are relelated to WSQ
*                        (IBSU_WSQEncodeMem, IBSU_WSQEncodeToFile, IBSU_WSQDecodeMem,
*                         IBSU_WSQDecodeFromFile, IBSU_FreeMemory)
*     2014/09/17  1.8.1  Added API functions are relelated to JPEG2000 and PNG
*                        (IBSU_SavePngImage, IBSU_SaveJP2Image)
*     2015/03/04  1.8.3  Reformatted to support UNICODE for WinCE
*                        Added API function is relelated to ClientWindow
*                        (IBSU_RedrawClientWindow)
*                        Bug Fixed, Added new parameter (pitch) to WSQ functions
*                        (IBSU_WSQEncodeMem, IBSU_WSQEncodeToFile, IBSU_WSQDecodeMem,
*                         IBSU_WSQDecodeFromFile)
*     2015/04/07  1.8.4  Added API function to unload the library manually
*                        (IBSU_UnloadLibrary)

*     2015/08/05  1.8.5  Added API function to combine two image into one
*                        (IBSU_CombineImage)
*     2015/12/11  1.9.0  Added API function to support Kojak device
*                        (IBSU_GetOperableBeeper, IBSU_SetBeeper)
*     2017/04/27  1.9.7  Added API function to support improved feature for CombineImage
*                        (IBSU_CombineImageEx)
*     2017/06/17  1.9.8  Added API function to support improved feature for CombineImage
*                        (IBSU_CheckWetFinger, IBSU_BGetRollingInfoEx, IBSU_GetImageWidth,
*                         IBSU_IsWritableDirectory)
*     2017/08/22  1.9.9  Added API function to get final image by native for Columbo
*                        (RESERVED_GetFinalImageByNative)
*     2018/03/06  2.0.0  Added API function to improve dispaly speed on Embedded System
*                        (IBSU_GenerateDisplayImage)
*     2018/04/27  2.0.1  Added API function to improve dispaly speed on Embedded System
*                        (IBSU_RemoveFingerImage, IBSU_AddFingerImage, IBSU_IsFingerDuplicated,
*                         IBSU_IsValidFingerGeometry)
*                        Deprecated API function about IBScanMater(IBSM)
*                        (IBSU_GetIBSM_ResultImageInfo)
****************************************************************************************************
*)

interface

(* The "C2PTypes.pas" unit declares external windows data types for the conversion purposes. It's created
automatically by the CtoPas converter and saved under "\[Application Path]\CtoPas Projects\P_Files" folder.
Consult the help file for more information about "C2PTypes.pas" unit and it's data type declarations *)

uses
	Windows, Messages, SysUtils, Classes, IBScanUltimateApi_defs, IBScanUltimateApi_err, IBScanUltimate;

type
    ppbyte = ^pbyte;

///@name Main Interface Functions
///@{

(*
****************************************************************************************************
* IBSU_GetSDKVersion()
* 
* DESCRIPTION:
*     Obtain product and software version information.
*
* ARGUMENTS:
*     pVerinfo  Pointer to structure that will receive SDK version information.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
{$EXTERNALSYM IBSU_GetSDKVersion}
function IBSU_GetSDKVersion(
	pVerinfo: pIBSU_SdkVersion                    
                                                 
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GetDeviceCount()
* 
* DESCRIPTION:
*     Retrieve count of connected IB USB scanner devices.
*
* ARGUMENTS:
*     pDeviceCount  Pointer to variable that will receive count of connected IB USB scanner devices.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
{$EXTERNALSYM IBSU_GetDeviceCount}
function IBSU_GetDeviceCount(
	pDeviceCount: pinteger                          
                                                  
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GetDeviceDescription()
* 
* DESCRIPTION:
*     Retrieve detailed device information about a particular scanner by its logical index.
*
* ARGUMENTS:
*     deviceIndex  Zero-based index of the scanner.
*     pDeviceDesc  Pointer to structure that will receive description of device.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
{$EXTERNALSYM IBSU_GetDeviceDescription}
function IBSU_GetDeviceDescription(
	deviceIndex: integer;                         
	pDeviceDesc: pIBSU_DeviceDesc           
                                            
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_RegisterCallbacks()
* 
* DESCRIPTION:
*     Register a callback function for an event type.  These asynchronous notifications enable 
*     event-driven management of scanner processes.  For more information about a particular 
*     event or the signature of its callback, see the definition of 'IBSU_Events'.
*
* ARGUMENTS:
*     handle             Handle for device associated with this event (if appropriate).
*     event              Event for which the callback is being registered.
*     pCallbackFunction  Pointer to the callback function.
*     pContext           Pointer to user context that will be passed to callback function.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)

function IBSU_RegisterCallbacks(
	handle: integer;                           
    events: IBSU_Events;
    pEventName: pointer;
    pContext: pointer
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_OpenDevice()
* 
* DESCRIPTION:
*     Initialize a device and obtain a handle for subsequent function calls.  Any initialized device
*     must be released with IBSU_CloseDevice() or IBSU_CloseAllDevice() before shutting down the 
*     application.
*
* ARGUMENTS:
*     deviceIndex  Zero-based index of the scanner.
*     pHandle      Pointer to variable that will receive device handle for subsequent function calls.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
{$EXTERNALSYM IBSU_OpenDevice}
function IBSU_OpenDevice(
	deviceIndex: integer;                           
	handle: pinteger
                                               
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_CloseDevice()
* 
* DESCRIPTION:
*     Release all resources for a device.
*
* ARGUMENTS:
*     handle  Device handle.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
*         IBSU_ERR_RESOURCE_LOCKED: A callback is still active.
*         IBSU_ERR_DEVICE_NOT_INITIALIZED: Device has already been released or is unknown.
****************************************************************************************************
*)
function IBSU_CloseDevice(
	handle: integer                              
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_CloseAllDevice()
* 
* DESCRIPTION:
*     Release all resources for all open devices.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
*         IBSU_ERR_RESOURCE_LOCKED: A callback is still active.
****************************************************************************************************
*)
{$EXTERNALSYM IBSU_CloseAllDevice}
function IBSU_CloseAllDevice(
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_IsDeviceOpened()
* 
* DESCRIPTION:
*     Check whether a device is open/initialized.
*
* ARGUMENTS:
*     handle  Device handle.
*
* RETURNS:
*     IBSU_STATUS_OK, if device is open/initialized.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
*         IBSU_ERR_INVALID_PARAM_VALUE: Handle value is out of range.
*         IBSU_ERR_DEVICE_NOT_INITIALIZED: Device has not been initialized.
*         IBSU_ERR_DEVICE_IO: Device has been initialized, but there was a communication problem.
****************************************************************************************************
*)
function IBSU_IsDeviceOpened(
	handle: integer                                  
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_SetProperty()
* 
* DESCRIPTION:
*     Set the value of a property for a device.  For descriptions of properties and values, see 
*     definition of 'IBSU_PropertyId'.
*
* ARGUMENTS:
*     handle         Device handle.
*     propertyId     Property for which value will be set.
*     propertyValue  Value of property to set.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_SetProperty(
	handle: integer;                              
    propertyId: IBSU_PropertyId;
    propertyValue: PAnsiChar
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GetProperty()
* 
* DESCRIPTION:
*     Get the value of a property for a device.  For descriptions of properties and values, see 
*     definition of 'IBSU_PropertyId'.
*
* ARGUMENTS:
*     handle          Device handle.
*     propertyId      Property for which value will be set.
*     propertyValue   Buffer in which value of property will be stored.  This buffer should be 
*                     able to hold IBSU_MAX_STR_LEN characters.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_GetProperty(
	handle: integer;                                 
    propertyId: IBSU_PropertyId;
    propertyValue: PAnsiChar
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_EnableTraceLog()
* 
* DESCRIPTION:
*     Enable or disable trace log at run-time.  The trace log is enabled by default on Windows and
*     Android and disabled by default on Linux.
*
* ARGUMENTS:
*     on  Indicates whether trace log should be turned on or off.  TRUE to turn log on; FALSE to 
*         turn log off.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_EnableTraceLog(
    onoff: BOOL
): integer; stdcall;

///@}


///@name Image Acquisition Related Interface Functions
///@{

(*
****************************************************************************************************
* IBSU_IsCaptureAvailable()
* 
* DESCRIPTION:
*     Check whether capture mode is supported by a device.
*
* ARGUMENTS:
*     handle           Device handle.
*     imageType        Type of capture.
*     imageResolution  Resolution of capture.
*     pIsAvailable     Pointer to variable that will receive indicator of support.  Will be set to
*                      TRUE if mode is supported.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_IsCaptureAvailable(
	handle: integer;                                 
    imageType: IBSU_ImageType;
    imageResolution: IBSU_ImageResolution;
    pIsAvailable: pBOOL
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_BeginCaptureImage()
* 
* DESCRIPTION:
*     Begin acquiring an image from a device.
*
* ARGUMENTS:
*     handle           Device handle.
*     imageType        Type of capture.
*     imageResolution  Resolution of capture.
*     captureOptions   Bit-wise OR of capture options:
*                          IBSU_OPTION_AUTO_CONTRAST - automatically adjust contrast to optimal value
*                          IBSU_OPTION_AUTO_CAPTURE - complete capture automatically when a good-
*                              quality image is available
*                          IBSU_OPTION_IGNORE_FINGER_COUNT - ignore finger count when deciding to 
*                              complete capture
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
*         IBSU_ERR_CAPTURE_STILL_RUNNING - An acquisition is currently executing and must complete 
*             before another capture can be started.
*         IBSU_ERR_CAPTURE_INVALID_MODE - Capture mode is not supported by this device. 
****************************************************************************************************
*)
function IBSU_BeginCaptureImage(
	handle: integer;                                  
    imageType: IBSU_ImageType;
    imageResolution: IBSU_ImageResolution;
    captureOptions: DWORD
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_CancelCaptureImage()
* 
* DESCRIPTION:
*     Abort acquisition on a device.
*
* ARGUMENTS:
*     handle  Device handle.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
*         IBSU_ERR_CAPTURE_NOT_RUNNING - Acquisition is not active. 
****************************************************************************************************
*)
function IBSU_CancelCaptureImage(
	handle: integer                                  
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_IsCaptureActive()
* 
* DESCRIPTION:
*     Determine whether acquisition is active for a device.
*
* ARGUMENTS:
*     handle     Device handle.
*     pIsActive  Pointer to variable that will receive indicator of activity.  TRUE if a acquisition
*                is in progress.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_IsCaptureActive(
	handle: integer;
    pIsActive: pBOOL
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_TakeResultImageManually()
* 
* DESCRIPTION:
*     Begin acquiring an image from a device with image gain manually set.
*
* ARGUMENTS:
*     handle  Device handle.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_TakeResultImageManually(
	handle: integer                                 
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GetContrast()
* 
* DESCRIPTION:
*     Get the contrast value for a device.
*
* ARGUMENTS:
*     handle          Device handle.
*     pContrastValue  Pointer to variable that will receive contrast value.  Value will be between
*                     IBSU_MIN_CONTRAST_VALUE and IBSU_MAX_CONTRAST_VALUE, inclusive.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_GetContrast(
	handle: integer;                              
    pContrastValue: pinteger
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_SetContrast()
* 
* DESCRIPTION:
*     Set the contrast value for a device.
*
* ARGUMENTS:
*     handle         Device handle.
*     contrastValue  Contrast value.  Value must be between IBSU_MIN_CONTRAST_VALUE and 
*                    IBSU_MAX_CONTRAST_VALUE, inclusive.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_SetContrast(
	handle: integer;                                
    contrastValue: integer
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_SetLEOperationMode()
* 
* DESCRIPTION:
*     Set the light-emitting (LE) film operation mode for a device.
*
* ARGUMENTS:
*     handle           Device handle.
*     leOperationMode  LE film operation mode.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_SetLEOperationMode(
	handle: integer;                                
    leOperationMode: IBSU_LEOperationMode
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GetLEOperationMode()
* 
* DESCRIPTION:
*     Get the light-emitting (LE) film operation mode for a device.
*
* ARGUMENTS:
*     handle            Device handle.
*     pLeOperationMode  Pointer to variable that will receive LE film operation mode.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_GetLEOperationMode(
	handle: integer;                              
    leOperationMode: pIBSU_LEOperationMode
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_IsTouchedFinger()
* 
* DESCRIPTION:
*     Determine whether a finger is on a scanner's surface.  This function queries the proximity 
*     detector only integrated into some sensors. 
*
* ARGUMENTS:
*     handle         Device handle.
*     pTouchInValue  Pointer to variable that will receive touch input value.  0 if no finger is 
*                    touching the surface, 1 if one or more fingers are touching the surface.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_IsTouchedFinger(
	handle: integer;                               
    touchInValue: pinteger
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GetOperableLEDs()
* 
* DESCRIPTION:
*     Get characteristics of operable LEDs on a device. 
*
* ARGUMENTS:
*     handle         Device handle.
*     pLedType       Pointer to variable that will receive type of LEDs.
*     pLedCount      Pointer to variable that will receive count of LEDs.
*     pOperableLEDs  Pointer to variable that will receive bit-pattern of operable LEDs.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_GetOperableLEDs(
	handle: integer;
    pLedType: pIBSU_LedType;
    pLedCount: pinteger;
    pOperableLEDs: pDWORD
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GetLEDs()
* 
* DESCRIPTION:
*     Get the value of LEDs on a device. 
*
* ARGUMENTS:
*     handle       Device handle.
*     pActiveLEDs  Pointer to variable that will receive bit-pattern of LED values.  Set bits 
*                  indicate LEDs that are on; clear bits indicate LEDs that are off.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_GetLEDs(
	handle: integer;                              
    pActiveLEDs: pDWORD
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_SetLEDs()
* 
* DESCRIPTION:
*     Set the value of LEDs on a device. 
*
* ARGUMENTS:
*     handle      Device handle.
*     activeLEDs  Bit-pattern of LED values.  Set bits indicate LEDs to turn on; clear bits indicate 
*                 LEDs to turn off.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_SetLEDs(
	handle: integer;                                
    activeLEDs: DWORD
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_CreateClientWindow()
* 
* DESCRIPTION:
*     Create client window associated with device.  (Available only on Windows.)
*
* ARGUMENTS:
*     handle   Device handle.
*     hWindow  Windows handle to draw.
*     left     Coordinate of left edge of rectangle.
*     top      Coordinate of top edge of rectangle.
*     right    Coordinate of right edge of rectangle.
*     bottom   Coordinate of bottom edge of rectangle.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_CreateClientWindow(
	handle: integer;                                 
    hWindow: IBSU_HWND;
	left: DWORD;                                      
	top: DWORD;                                      
	right: DWORD;                                    
	bottom: DWORD                                     
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_DestroyClientWindow()
* 
* DESCRIPTION:
*     Destroy client window associated with device.  (Available only on Windows.)
*
* ARGUMENTS:
*     handle             Device handle.
*     clearExistingInfo  Indicates whether the existing display information, including display
*                        properties and overlay text, will be cleared.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_DestroyClientWindow(
    handle: integer;
	clearExistingInfo: BOOL                           
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GetClientWindowProperty()
* 
* DESCRIPTION:
*     Get the value of a property for the client window associated with a device.  For descriptions 
*     of properties and values, see definition of 'IBSU_ClientWindowPropertyId'.  (Available only on
*     Windows.)
*
* ARGUMENTS:
*     handle          Device handle.
*     propertyId      Property for which value will be set.
*     propertyValue   Buffer in which value of property will be stored.  This buffer should be 
*                     able to hold IBSU_MAX_STR_LEN characters.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_GetClientWindowProperty(
	handle: integer;                                 
    propertyId: IBSU_DisplayPropertyId;
    propertyValue: PAnsiChar
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_SetClientDisplayProperty()
* 
* DESCRIPTION:
*     Set the value of a property for the client window associated with a device.  For descriptions 
*     of properties and values, see definition of 'IBSU_ClientWindowPropertyId'.  (Available only on
*     Windows.)
*
* ARGUMENTS:
*     handle          Device handle.
*     propertyId      Property for which value will be set.
*     propertyValue   Value of property to set.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_SetClientDisplayProperty(
	handle: integer;                                 
    propertyId: IBSU_DisplayPropertyId;
    propertyValue: PAnsiChar
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_SetClientWindowOverlayText()
* 
* DESCRIPTION:
*     Set the overlay text for the client window associated with a device.  (Available only on
*     Windows.)
*
* ARGUMENTS:
*     handle     Device handle.
*     fontName   Font name.
*     fontSize   Font size.
*     fontBold   Indicates whether font will be bold.
*     text       Text to display.
*     posX       X-coordinate of text.
*     poxY       Y-coordinate of text.
*     textColor  Color of text.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_SetClientWindowOverlayText(
	handle: integer;                                
    fontName: string;
	fontSize: integer;                                
	fontBold: BOOL;                               
	text: string;                                  
	posX: integer;                                    
	posY: integer;                                    
	textColor: DWORD                                
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GenerateZoomOutImage()
* 
* DESCRIPTION:
*     Generate scaled version of image.
*
* ARGUMENTS:
*     inImage     Original image.
*     outImage    Pointer to buffer that will receive output image.  This buffer must hold at least
*                 'outWidth' x 'outHeight' bytes.
*     outWidth    Width of output image.
*     outHeight   Height of output image.
*     bkColor     Background color of output image.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_GenerateZoomOutImage(
	inImage: IBSU_ImageData;
    outImage: pbyte;
	outWidth: integer;
	outHeight: integer;
    bkColor: byte
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_SaveBitmapImage()
* 
* DESCRIPTION:
*     Save image to bitmap file.
*
* ARGUMENTS:
*     filePath   Path of file for output image.
*     imgBuffer  Pointer to image buffer.
*     width      Image width (in pixels).
*     height     Image height (in pixels).
*     pitch      Image line pitch (in bytes).  A positive value indicates top-down line order; a
*                negative value indicates bottom-up line order.
*     resX       Horizontal image resolution (in pixels/inch).
*     resY       Vertical image resolution (in pixels/inch).
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_SaveBitmapImage(
    filePath: string;
	imgBuffer: pbyte;
    width: DWORD;
    height: DWORD;
    pitch: integer;
    resX: double;
    resY: double
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_AsyncOpenDevice()
* 
* DESCRIPTION:
*     Initialize a device asynchronously.  The handle will be delivered to the application with a
*     ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE callback or with IBSU_BGetInitProgress().  Any 
*     initialized device must be released with IBSU_CloseDevice() or IBSU_CloseAllDevice() before 
*     shutting down the application.
*
* ARGUMENTS:
*     deviceIndex  Zero-based index of the scanner.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_AsyncOpenDevice(
    deviceIndex: integer
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_BGetImage()
* 
* DESCRIPTION:
*     Acquire an image from a device, blocking for result.  The split image array will only be 
*     populated if the image is a result image, i.e., if the 'IsFinal' member of 'pImage' is set to
*     TRUE.
*
* ARGUMENTS:
*     handle                 Device handle.
*     pImage                 Pointer to structure that will receive data of preview or result image.   
*                            The buffer in this structure points to an internal image buffer; the 
*                            data should be copied to an application buffer if desired for future 
*                            processing.
*     pImageType             Pointer to variable that will receive image type.
*     pSplitImageArray       Pointer to array of four structures that will receive individual finger 
*                            images split from result image.  The buffers in these structures point
*                            to internal image buffers; the data should be copied to application 
*                            buffers if desired for future processing.
*     pSplitImageArrayCount  Pointer to variable that will receive number of finger images split 
*                            from result images.
*     pFingerCountState      Pointer to variable that will receive finger count state.
*     pQualityArray          Pointer to array of four variables that will receive quality state for
*                            finger image.
*     pQualityArrayCount     Pointer to variable that will receive number of finger qualities.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_BGetImage(
    handle: integer;
	pImage: pIBSU_ImageData;
    pImageType: pIBSU_ImageType;
    pSplitImageArray: pIBSU_ImageData;
    pSplitImageArrayCount: pinteger;
    pFingerCountState: pIBSU_FingerCountState;
    pQualityArray: pIBSU_FingerQualityState;
    pQualityArrayCount: pinteger
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_BGetImageEx()
* 
* DESCRIPTION:
*     Acquire an image from a device, blocking for resultEx.  The segment image array and  will only be 
*     populated if the image is a result image, i.e., if the 'IsFinal' member of 'pImage' is set to
*     TRUE.
*
* ARGUMENTS:
*     handle                    Device handle.
*     pImageStatus              Pointer to variable that will receive image status from result images.
*     pImage                    Pointer to structure that will receive data of preview or result image.   
*                               The buffer in this structure points to an internal image buffer; the 
*                               data should be copied to an application buffer if desired for future 
*                               processing.
*     pImageType                Pointer to variable that will receive image type.
*     pDetectedFingerCount      Pointer to variable that will receive detected finger count.
*     pSegmentImageArray        Pointer to array of four structures that will receive individual finger 
*                               images segment from result image.  The buffers in these structures point
*                               to internal image buffers; the data should be copied to application 
*                               buffers if desired for future processing.
*     pSegmentPositionArray	    Pointer to array of four structures that will receive individual finger 
*								images segment coordinates from result image.  The buffers in these structures point
*                               to internal image buffers; the data should be copied to application 
*                               buffers if desired for future processing.
*     pSegmentImageArrayCount   Pointer to variable that will receive number of finger images split 
*                               from result images.
*     pFingerCountState         Pointer to variable that will receive finger count state.
*     pQualityArray             Pointer to array of four variables that will receive quality state for
*                               finger image.
*     pQualityArrayCount        Pointer to variable that will receive number of finger qualities.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_BGetImageEx(
    handle: integer;
    pImageStatus: pinteger;
	pImage: pIBSU_ImageData;
    pImageType: pIBSU_ImageType;
    pDetectedFingerCount: pinteger;
    pSegmentImageArray: pIBSU_ImageData;
    pSegmentPositionArray: pIBSU_SegmentPosition;
    pSegmentImageArrayCount: pinteger;
    pFingerCountState: pIBSU_FingerCountState;
    pQualityArray: pIBSU_FingerQualityState;
    pQualityArrayCount: pinteger
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_BGetInitProgress()
* 
* DESCRIPTION:
*     Get initialization progress of a device.  If initialization is complete, the handle for
*     subsequent function calls will be returned to the application.
*
* ARGUMENTS:
*     deviceIndex     Zero-based index of the scanner.
*     pIsComplete     Pointer to variable that will receive indicator of initialization completion.
*     pHandle         Pointer to variable that will receive device handle for subsequent function 
*                     calls, if 'pIsComplete' receives the value TRUE.
*     pProgressValue  Pointer to variable that will receive initialize progress, as a percentage
*                     between 0 and 100, inclusive.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_BGetInitProgress(
    deviceIndex : integer;
	pIsComplete: pBOOL;
    pHandle: pinteger;
    pProgressValue: pinteger
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_BGetClearPlatenAtCapture()
* 
* DESCRIPTION:
*     Determine whether the platen was clear when capture was started or has since become clear. 
*
* ARGUMENTS:
*     handle        Device handle.
*     pPlatenState  Pointer to variable that will receive platen state.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_BGetClearPlatenAtCapture(
    handle: integer;
	pPlatenState: pIBSU_PlatenState               
): integer; stdcall;

///@}

(*
****************************************************************************************************
* IBSU_BGetRollingInfo()
* 
* DESCRIPTION:
*     Get information about the status of the rolled print capture for a device. 
*
* ARGUMENTS:
*     handle         Device handle.
*     pRollingState  Pointer to variable that will receive rolling state.
*     pRollingLineX  Pointer to variable that will receive x-coordinate of current "rolling line" 
*                    for display as a guide.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_BGetRollingInfo(
    handle: integer;
    pRollingState: pIBSU_RollingState;
	pRollingLineX: pinteger                         
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_BGetRollingInfoEx()
* 
* DESCRIPTION:
*     Get information about the status of the rolled print capture for a device. 
*
* ARGUMENTS:
*     handle         Device handle.
*     pRollingState  Pointer to variable that will receive rolling state.
*     pRollingLineX  Pointer to variable that will receive x-coordinate of current "rolling line" 
*                    for display as a guide.
*     pRollDirection Pointer to variable that will receive rolling direction
*                    0 : can't determin yet
*                    1 : left to right  --->
*                    2 : right to left  <---
*     pRollWidth     Pointer to vairable that will receive rolling width (mm)
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
*)
function IBSU_BGetRollingInfoEx(
    handle: integer;
    pRollingState: pIBSU_RollingState;
	  pRollingLineX: pinteger;
	  pRollingDirection: pinteger;
	  pRollingWidth: pinteger
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_OpenDeviceEx()
* 
* DESCRIPTION:
*     Initialize a device and obtain a handle for subsequent function calls.  The uniformity mask
*     will be loaded from a file to speed up initialization.  Any initialized device must be 
*     released with IBSU_CloseDevice() or IBSU_CloseAllDevice() before shutting down the 
*     application.
*
* ARGUMENTS:
*     deviceIndex         Zero-based index of the scanner.
*     uniformityMaskPath  Path at which uniformity mask file is located.  If file does not exist,
*                         it will be created to accelerate subsequent initializations.
*     ayncnOpen           Indicates whether initialization will be performed synchronously or 
*                         asynchronously.
*     pHandle             Pointer to variable that will receive device handle for subsequent 
*                         function calls, if 'asyncOpen' is FALSE.  Otherwise, handle will be 
*                         delivered to the application with a ENUM_IBSU_ESSENTIAL_EVEN_ASYNC_OPEN_DEVICE 
*                         callback or with IBSU_BGetInitProgress().
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_OpenDeviceEx(
    deviceIndex: integer;
	uniformityMaskPath: string;
	asyncOpen: BOOL;
    pHandle: pinteger
                                                 
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GetIBSM_ResultImageInfo() (deprecated)
* 
* DESCRIPTION:
*     Get result image information.
*
* ARGUMENTS:
*     handle                  Device handle.
*     fingerPosition          Finger position.
*     pResultImage            Pointer to structure that will receive data of preview or result image.   
*                             The buffer in this structure points to an internal image buffer; the 
*                             data should be copied to an application buffer if desired for future 
*                             processing.
*     pSplitResultImage       Pointer to array of four structures that will receive individual finger 
*                             images split from result image.  The buffers in these structures point
*                             to internal image buffers; the data should be copied to application 
*                             buffers if desired for future processing.
*     pSplitResultImageCount  Pointer to variable that will receive number of finger images split 
*                             from result image.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_GetIBSM_ResultImageInfo(
    handle: integer;
    fingerPosition: IBSM_FingerPosition;
    pResultImage: pIBSM_ImageData;
	pSplitResultImage: pIBSM_ImageData;
	pSplitResultImageCount: pinteger
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GetNFIQScore()
* 
* DESCRIPTION:
*     Calculate NFIQ score for image.
*
* ARGUMENTS:
*     handle        Device handle.
*     imgBuffer     Pointer to image buffer.
*     width         Image width (in pixels).
*     height        Image height (in pixels).
*     bitsPerPixel  Bits per pixel.
*     pScore        Pointer to variable that will receive NFIQ score.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)

function IBSU_GetNFIQScore(
    handle: integer;
	imgBuffer: pbyte;
    width: DWORD;
    height: DWORD;
    bitsPerPixel: byte;
    pScore: pinteger
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GenerateZoomOutImageEx()
* 
* DESCRIPTION:
*     Generate scaled version of image.
*
* ARGUMENTS:
*     inImage     Original image data.
*     inWidth     Width of input image.
*     in Height   Width of input image.
*     outImage    Pointer to buffer that will receive output image.  This buffer must hold at least
*                 'outWidth' x 'outHeight' bytes.
*     outWidth    Width of output image.
*     outHeight   Height of output image.
*     bkColor     Background color of output image.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_GenerateZoomOutImageEx(
    pInImage: pbyte;
    inWidth: integer;
    inHeight: integer;
    outImage: pbyte;
	outWidth: integer;
	outHeight: integer;
    bkColor: byte
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_ReleaseCallbacks()
* 
* DESCRIPTION:
*     Unregister a callback function for an event type.  These asynchronous notifications enable 
*     event-driven management of scanner processes.  For more information about a particular 
*     event or the signature of its callback, see the definition of 'IBSU_Events'.
*
* ARGUMENTS:
*     handle             Handle for device associated with this event (if appropriate).
*     event              Event for which the callback is being unregistered.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_ReleaseCallbacks(
    handle: integer;
    events: IBSU_Events
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_SaveBitmapMem()
* 
* DESCRIPTION:
*     Save image to bitmap memory.
*
* ARGUMENTS:
*     inImage           Point to image data (Gray scale image).
*     inWidth           Image width (in pixels).
*     inHeight          Image height (in pixels).
*     inPitch           Image line pitch (in bytes).  A positive value indicates top-down line order; a
*                       negative value indicates bottom-up line order.
*     inResX            Horizontal image resolution (in pixels/inch).
*     inResY            Vertical image resolution (in pixels/inch).
*     outBitmapBuffer   Pointer to output image data buffer which is set image format and zoom-out factor; a
*                       Memory must be provided by caller
*                       Required memory buffer size with argument @ref outImageFormat, 
*                       @ref IBSU_IMG_FORMAT_GRAY, @ref IBSU_BMP_GRAY_HEADER_LEN + outWidth * outHeight bytes
*                       @ref IBSU_IMG_FORMAT_RGB24, @ref IBSU_BMP_RGB24_HEADER_LEN + 3 * outWidth * outHeight bytes
*                       @ref IBSU_IMG_FORMAT_RGB32,  @ref IBSU_BMP_RGB32_HEADER_LEN + 4 * outWidth * outHeight bytes
*     outImageFormat    Set Image color format for output image
*     outWidth          Width for zoom-out image
*     outHeight         Height for zoom-out image
*     bkColor           Background color for remain area from zoom-out image
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_SaveBitmapMem(
    inImage: pbyte;
    inWidth: DWORD;
    inHeight: DWORD;
    inPitch: integer;
    inResX: double;
    inResY: double;
    outBitmapBuffer: pbyte;
    outImageFormat: IBSU_ImageFormat;
    outWidth: DWORD;
    outHeight: DWORD;
    bkColor: byte
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_ShowOverlayObject()
* 
* DESCRIPTION:
*     Show an overlay objects which is used with overlay handle.
*
* ARGUMENTS:
*     handle            Handle for device associated with this event (if appropriate).
*     overlayHandle	    Overlay handle obtained by overlay functions.
*     show	            If TRUE, the overlay will be shown on client window.
*                       If FALSE, the overlay will be hidden on client window.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_ShowOverlayObject(
    handle: integer;
    overlayHandle: integer;
    show: BOOL
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_ShowAllOverlayObject()
* 
* DESCRIPTION:
*     Show all of overlay objects.
*
* ARGUMENTS:
*     handle            Handle for device associated with this event (if appropriate).
*     show	            If TRUE, the overlay will be shown on client window.
*                       If FALSE, the overlay will be hidden on client window.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_ShowAllOverlayObject(
    handle: integer;
    show: BOOL
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_RemoveOverlayObject()
* 
* DESCRIPTION:
*     Remove an overlay objects which is used with overlay handle.
*
* ARGUMENTS:
*     handle            Handle for device associated with this event (if appropriate).
*     overlayHandle	    Overlay handle obtained by overlay functions.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_RemoveOverlayObject(
    handle: integer;
    overlayHandle: integer
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_RemoveAllOverlayObject()
* 
* DESCRIPTION:
*     Remove all of overlay objects.
* 
* ARGUMENTS:
*     handle            Handle for device associated with this event (if appropriate).
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_RemoveAllOverlayObject(
    handle: integer
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_AddOverlayText()
* 
* DESCRIPTION:
*	  Add an overlay text for display on window    
* 
* ARGUMENTS:
*     handle            Handle for device associated with this event (if appropriate).
*	  pOverlayHandle	Function returns overlay handle to be used for client windows functions call
*     fontName			used kind of font
*	  fontSize			used font size	
*	  fontBold			if font is bold or not
*	  text				text for display on window
*	  posX				X coordinate of text for display on window
*	  posY				Y coordinate of text for display on window
*	  textColor			text color
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_AddOverlayText(
    handle: integer;
    pOverlayHandle: pinteger;
    fontName: string;
	fontSize: integer;                          
	fontBold: BOOL;                          
	text: string;                           
	posX: integer;                              
	posY: integer;                           
	textColor: DWORD                         
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_ModifyOverlayText()
* 
* DESCRIPTION:
*	  Modify an existing overlay text for display on window    
* 
* ARGUMENTS:
*     handle            Handle for device associated with this event (if appropriate).
*	  OverlayHandle		Overlay handle to be modified
*     fontName			used kind of font
*	  fontSize			used font size	
*	  fontBold			if font is bold or not
*	  text				text for display on window
*	  posX				X coordinate of text for display on window
*	  posY				Y coordinate of text for display on window
*	  textColor			text color
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_ModifyOverlayText(
    handle: integer;
    overlayHandle: integer;
    fontName: string;
	fontSize: integer;                          
	fontBold: BOOL;                          
	text: string;                           
	posX: integer;                              
	posY: integer;                               
	textColor: DWORD                         
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_AddOverlayLine()
* 
* DESCRIPTION:
*	  Add an overlay line for display on window    
* 
* ARGUMENTS:
*     handle            Handle for device associated with this event (if appropriate).
*	  pOverlayHandle    Function returns overlay handle to be used for client windows functions calls
*     x1				X coordinate of start point of line
*	  y1				Y coordinate of start point of line
*	  x2				X coordinate of end point of line
*	  y2				Y coordinate of end point of line
*	  lineWidth		    line width
*	  lineColor			line color
*	 
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_AddOverlayLine(
    handle: integer;
    pOverlayHandle: pinteger;
    x1: integer;
	y1: integer;                                 
	x2: integer;                                
	y2: integer;                                
	lineWidth: integer;                      
	lineColor: DWORD                        
): integer; stdcall;


(*
****************************************************************************************************
* IBSU_ModifyOverlayLine()
* 
* DESCRIPTION:
*	  Modify an existing line for display on window    
* 
* ARGUMENTS:
*     handle            Handle for device associated with this event (if appropriate).
*	  OverlayHandle     OverlayHandle
*     x1				X coordinate of start point of line
*	  y1				Y coordinate of start point of line
*	  x2				X coordinate of end point of line
*	  y2				Y coordinate of end point of line
*	  lineWidth		    line width
*	  lineColor			line color
*	 
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_ModifyOverlayLine(
    handle: integer;
    overlayHandle: integer;
    x1: integer;
	y1: integer;                                 
	x2: integer;                                
	y2: integer;                                
	lineWidth: integer;                          
	lineColor: DWORD                          
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_AddOverlayQuadrangle()
* 
* DESCRIPTION:
*	  Add an overlay quadrangle for display on window    
* 
* ARGUMENTS:
*     handle            Handle for device associated with this event (if appropriate).
*	  pOverlayHandle    Function returns overlay handle to be used for client windows functions calls
*     x1				X coordinate of start point of line
*	  y1				Y coordinate of start point of line
*	  x2				X coordinate of 1st corner of quadrangle
*	  y2				Y coordinate of 1st corner of quadrangle
*     x3				X coordinate of 2nd corner of quadrangle
*	  y3				Y coordinate of 2nd corner of quadrangle
*	  x4				X coordinate of 3rd corner of quadrangle
*	  y4				Y coordinate of 2rd corner of quadrangle
*	  lineWidth		    line width
*	  lineColor			line color
*	 
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_AddOverlayQuadrangle(
    handle: integer;
    pOverlayHandle: pinteger;
    x1: integer;
	y1: integer;                                 
	x2: integer;                                
	y2: integer;                                
	x3: integer;                               
	y3: integer;                                
	x4: integer;                               
	y4: integer;                             
	lineWidth: integer;                          
	lineColor: DWORD                         
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_ModifyOverlayQuadrangle()
* 
* DESCRIPTION:
*	  Modify an existing quadrangle for display on window    
* 
* ARGUMENTS:
*     handle            Handle for device associated with this event (if appropriate).
*	  OverlayHandle     Overlay handle to be modified
*     x1				X coordinate of start point of line
*	  y1				Y coordinate of start point of line
*	  x2				X coordinate of 1st corner of quadrangle
*	  y2				Y coordinate of 1st corner of quadrangle
*     x3				X coordinate of 2nd corner of quadrangle
*	  y3				Y coordinate of 2nd corner of quadrangle
*	  x4				X coordinate of 3rd corner of quadrangle
*	  y4				Y coordinate of 2rd corner of quadrangle
*	  lineWidth		    line width
*	  lineColor			line color
*	 
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_ModifyOverlayQuadrangle(
    handle: integer;
    overlayHandle: integer;
    x1: integer;
	y1: integer;                                 
	x2: integer;                                
	y2: integer;                                
	x3: integer;                             
	y3: integer;                              
	x4: integer;                                
	y4: integer;                                 
	lineWidth: integer;                         
	lineColor: DWORD                         
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_AddOverlayShape()
* 
* DESCRIPTION:
*	  Add an overlay shape for display on window    
* 
* ARGUMENTS:
*     handle            Handle for device associated with this event (if appropriate).
*	  pOverlayHandle    Function returns overlay handle to be used for client windows functions calls
*     shapePattern		Predefined overlay shape		
*	  x1				X coordinate of start point of overlay shape
*	  y1				Y coordinate of start point of overlay shape
*     x2				X coordinate of end point of overlay shape
*	  y2				Y coordinate of end point of overlay shape
*	  lineWidth		    line width
*	  lineColor			line color
*	  reserved_1		X Reserved
*	  reserved_2		Y Reserved
*
*						If you set shapePattern to ENUM_IBSU_OVERLAY_SHAPE_ARROW
*						* reserved_1 can use the width(in pixels) of the full base of the arrowhead
*						* reserved_2 can use the angle(in radians) at the arrow tip between the two sides of the arrowhead
*	 
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_AddOverlayShape(
    handle: integer;
    pOverlayHandle: pinteger;

    shapePattern: IBSU_OverlayShapePattern;
	x1: integer;
	y1: integer;
	x2: integer;
	y2: integer;
	lineWidth: integer;
	lineColor: integer;
	reserved_1: integer;
	reserved_2: integer
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_ModifyOverlayShape()
* 
* DESCRIPTION:
*	  Modify an overlay shape for display on window    
* 
* ARGUMENTS:
*     handle            Handle for device associated with this event (if appropriate).
*	  OverlayHandle		Overlay handle to be modified
*     shapePattern				
*	  x1				X coordinate of start point of overlay shape
*	  y1				Y coordinate of start point of overlay shape
*     x2				X coordinate of end point of overlay shape
*	  y2				Y coordinate of end point of overlay shape
*	  lineWidth		    line width
*	  lineColor			line color
*	  reserved_1		X Reserved
*	  reserved_2		Y Reserved
*
*						If you set shapePattern to ENUM_IBSU_OVERLAY_SHAPE_ARROW
*						* reserved_1 can use the width(in pixels) of the full base of the arrowhead
*						* reserved_2 can use the angle(in radians) at the arrow tip between the two sides of the arrowhead
*	 
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_ModifyOverlayShape(
    handle: integer;
    overlayHandle: integer;
    shapePattern: IBSU_OverlayShapePattern;
	x1: integer;
	y1: integer;
	x2: integer;
	y2: integer;
	lineWidth: integer;
	lineColor: integer;
	reserved_1: integer;
	reserved_2: integer
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_WSQEncodeMem()
* 
* DESCRIPTION:
*     WSQ compresses grayscale fingerprint image.
*
* ARGUMENTS:
*     image             Original image.
*     width             Width of original image (in pixels).
*     height            Height of original image (in pixels).
*     pitch             Image line pitch (in bytes).  A positive value indicates top-down line order; a
*                       negative value indicates bottom-up line order.
*     bitsPerPixel      Bits per pixel of original image.
*     pixelPerInch      Pixel per inch of original image.
*     bitRate           Determines the amount of lossy compression.
                        Suggested settings:
                        bitRate = 2.25 yields around 5:1 compression
                        bitRate = 0.75 yields around 15:1 compression
*     commentText       Comment to write compressed data.
*     compressedData    Pointer of image which is compressed from original image by WSQ compression.
*                       This pointer is deallocated by IBSU_FreeMemory() after using it.
*     compressedLength  Length of image which is compressed from original image by WSQ compression.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_WSQEncodeMem(
    image: pbyte;
    width: integer;
    height: integer;
    pitch: integer;
	bitPerPixel: integer;
	pixelPerInch: integer;
	bitRate: double;
	commentText: string;
	compressedData: ppbyte;
	compressedLength: pinteger
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_WSQEncodeToFile()
* 
* DESCRIPTION:
*     Save WSQ compresses grayscale fingerprint image to specific file path.
*
* ARGUMENTS:
*     filePath          File path to save image which is compressed from original image by WSQ compression.
*     image             Original image.
*     width             Width of original image (in pixels).
*     height            Height of original image (in pixels).
*     pitch             Image line pitch (in bytes).  A positive value indicates top-down line order; a
*                       negative value indicates bottom-up line order.
*     bitsPerPixel      Bits per pixel of original image.
*     pixelPerInch      Pixel per inch of original image.
*     bitRate           Determines the amount of lossy compression.
                        Suggested settings:
                        bitRate = 2.25 yields around 5:1 compression
                        bitRate = 0.75 yields around 15:1 compression
*     commentText       Comment to write compressed data.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_WSQEncodeToFile(
    filePath: string;
    image: pbyte;
    width: integer;
    height: integer;
    pitch: integer;
	bitPerPixel: integer;
	pixelPerInch: integer;
	bitRate: double;
	commentText: string
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_WSQDecodeMem()
* 
* DESCRIPTION:
*     Decompress a WSQ-encoded grayscale fingerprint image.
*
* ARGUMENTS:
*     compressedImage   WSQ-encoded image.
*     compressedLength  Length of WSQ-encoded image.
*     decompressedImage Pointer of image which is decompressed from WSQ-encoded image.
*                       This pointer is deallocated by IBSU_FreeMemory() after using it.
*     outWidth          Width of decompressed image (in pixels).
*     outHeight         Height of decompressed image (in pixels).
*     outPitch          Image line pitch (in bytes).  A positive value indicates top-down line order; a
*                       negative value indicates bottom-up line order.
*     outBitsPerPixel   Bits per pixel of decompressed image.
*     outPixelPerInch   Pixel per inch of decompressed image.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_WSQDecodeMem(
    compressedImage: pbyte;
    compressedLength: integer;
    decompressedImage: ppbyte;
    outWidth: pinteger;
    outHeight: pinteger;
    outPitch: pinteger;
	outBitPerPixel: pinteger;
	outPixelPerInch: pinteger
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_WSQDecodeFromFile()
* 
* DESCRIPTION:
*     Decompress a WSQ-encoded grayscale fingerprint image from specific file path.
*
* ARGUMENTS:
*     filePath          File path of WSQ-encoded image.
*     decompressedImage Pointer of image which is decompressed from WSQ-encoded image.
*                       This pointer is deallocated by IBSU_FreeMemory() after using it.
*     outWidth          Width of decompressed image (in pixels).
*     outHeight         Height of decompressed image (in pixels).
*     outPitch          Image line pitch (in bytes).  A positive value indicates top-down line order; a
*                       negative value indicates bottom-up line order.
*     outBitsPerPixel   Bits per pixel of decompressed image.
*     outPixelPerInch   Pixel per inch of decompressed image.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_WSQDecodeFromFile(
    filePath: string;
    decompressedImage: ppbyte;
    outWidth: pinteger;
    outHeight: pinteger;
    outPitch: pinteger;
	outBitPerPixel: pinteger;
	outPixelPerInch: pinteger
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_FreeMemory()
* 
* DESCRIPTION:
*     Release the allocated memory block on the internal heap of library.
*     This is obtained by IBSU_WSQEncodeMem(), IBSU_WSQDecodeMem, IBSU_WSQDecodeFromFile() and other API functions.
*
* ARGUMENTS:
*     memblock          Previously allocated memory block to be freed.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_FreeMemory(
    memblock: pointer
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_SavePngImage()
*
* DESCRIPTION:
*     Save image to PNG file.
*
* ARGUMENTS:
*     filePath   Path of file for output image.
*     imgBuffer  Pointer to image buffer.
*     width      Image width (in pixels).
*     height     Image height (in pixels).
*     pitch      Image line pitch (in bytes).  A positive value indicates top-down line order; a
*                negative value indicates bottom-up line order.
*     resX       Horizontal image resolution (in pixels/inch).
*     resY       Vertical image resolution (in pixels/inch).
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_SavePngImage(
    filePath: string;
	imgBuffer: pbyte;
    width: DWORD;
    height: DWORD;
    pitch: integer;
    resX: double;
    resY: double
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_SaveJP2Image()
* 
* DESCRIPTION:
*     Save image to JPEG-2000 file.
*
* ARGUMENTS:
*     filePath   Path of file for output image.
*     imgBuffer  Pointer to image buffer.
*     width      Image width (in pixels).
*     height     Image height (in pixels).
*     pitch      Image line pitch (in bytes).  A positive value indicates top-down line order; a
*                negative value indicates bottom-up line order.
*     resX       Horizontal image resolution (in pixels/inch).
*     resY       Vertical image resolution (in pixels/inch).
*     fQuality   Quality level for JPEG2000, he valid range is between 0 and 100
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_SaveJP2Image(
    filePath: string;
	imgBuffer: pbyte;
    width: DWORD;
    height: DWORD;
    pitch: integer;
    resX: double;
    resY: double;
    fQuality: integer
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_RedrawClientWindow()
* 
* DESCRIPTION:
*     Update the specified client window which is defined by IBSU_CreateClientWindow().  (Available only on Windows.)
*
* ARGUMENTS:
*     handle          Device handle.
*     flags           Bit-pattern of redraw flags.  See flag codes in 'IBScanUltimateApi_def
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_RedrawClientWindow(
    handle: integer
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_UnloadLibrary()
* 
* DESCRIPTION:
*     The library is unmapped from the address space manually, and the library is no longer valid
*     So APIs will not be worked correctly after calling 
*     Some platform SDKs (Windows Mobile, Android)
*     can be needed to call IBSU_UnloadLibrary() before shutting down the application.
*
* ARGUMENTS:
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_UnloadLibrary(): integer; stdcall;

(*
****************************************************************************************************
* IBSU_CombineImage()
*
* DESCRIPTION:
*     Combine two images (2 flat fingers) into a single image (left/right hands)
*
* ARGUMENTS:
*     inImage1		  Pointer to IBSU_ImageData ( index and middle finger )
*     inImage2		  Pointer to IBSU_ImageData ( ring and little finger )
*	  whichHand		  Information of left or right hand
*     outImage		  Pointer to IBSU_ImageData ( 1600 x 1500 fixed size image )
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_CombineImage(
    inImage1: IBSU_ImageData;
    inImage2: IBSU_ImageData;
	whichHand: IBSU_CombineImageWhichHand;
	outImage: pIBSU_ImageData
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GetOperableBeeper()
*
* DESCRIPTION:
*     Get characteristics of operable Beeper on a device.
*
* ARGUMENTS:
*     handle         Device handle.
*     pBeeperType    Pointer to variable that will receive type of Beeper.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_GetOperableBeeper(
    handle: integer;
    pBeeperType: pIBSU_BeeperType
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_SetBeeper()
* 
* DESCRIPTION:
*     Set the value of Beeper on a device. 
*
* ARGUMENTS:
*     handle          Device handle.
*     beepPattern     Pattern of beep.
*     soundTone       The frequency of the sound, in specific value. The parameter must be
*                     in the range 0 through 2.
*     duration        The duration of the sound, in 25 miliseconds. The parameter must be
*                     in the range 1 through 200 at ENUM_IBSU_BEEP_PATTERN_GENERIC,
*                     in the range 1 through 7 at ENUM_IBSU_BEEP_PATTERN_REPEAT.
*     reserved_1      Reserved
*     reserved_2      Reserved
*                     If you set beepPattern to ENUM_IBSU_BEEP_PATTERN_REPEAT
*                     reserved_1 can use the sleep time after duration of the sound, in 25 miliseconds.
*                     The parameter must be in the range 1 through 8
*                     reserved_2 can use the operation(start/stop of pattern repeat), 1 to start; 0 to stop 
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_SetBeeper(
    handle: integer;
    beepPattern: IBSU_BeepPattern;
    soundTone: DWORD;
    duration: DWORD;
    reserved_1: DWORD;
    reserved_2: DWORD
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_CombineImageEx()
* 
* DESCRIPTION:
*     Combine two images (2 flat fingers) into a single image (left/right hands)
*     and return segment information as well
*
* ARGUMENTS:
*     inImage1					Pointer to IBSU_ImageData ( index and middle finger )
*     inImage2					Pointer to IBSU_ImageData ( ring and little finger )
*	    whichHand					Information of left or right hand
*     outImage					Pointer to IBSU_ImageData ( 1600 x 1500 fixed size image )
*     pSegmentImageArray        Pointer to array of four structures that will receive individual finger 
*                               image segments from output image.  The buffers in these structures point
*                               to internal image buffers; the data should be copied to application 
*                               buffers if desired for future processing.
*     pSegmentPositionArray     Pointer to array of four structures that will receive position data for 
*                               individual fingers split from output image.
*     pSegmentImageArrayCount   Pointer to variable that will receive number of finger images split 
*                               from output image.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_CombineImageEx(
    inImage1: IBSU_ImageData;
    inImage2: IBSU_ImageData;
    whichHand: IBSU_CombineImageWhichHand;
    outImage: pIBSU_ImageData;
    pSegmentImageArray: pIBSU_SegmentPosition;
    pSegmentImageArrayCount: pinteger
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_CheckWetFinger()
* 
* DESCRIPTION:
*     Check if the image is wet or not.
*
* ARGUMENTS:
*     handle                 Device handle.
*     inImage                Input image data which is returned from result callback.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See warning codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_CheckWetFinger(
    handle: integer;
    inImage: IBSU_ImageData
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GetImageWidth()
*
* DESCRIPTION:
*     Get the image width of input image by millimeter(mm).
*
* ARGUMENTS:
*     handle                 Device handle.
*     inImage                Input image data which is returned from result callback.
*     Width_MM				 Output millimeter (width) of Input image.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See warning codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_GetImageWidth(
    handle: integer;
    inImage: IBSU_ImageData;
    Width_MM: pinteger
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_IsWritableDirectory()
* 
* DESCRIPTION:
*     Check whether a directory is writable.
*
* ARGUMENTS:
*     dirpath                Directory path.
*     needCreateSubFolder	 Check whether need to create subfolder into the directory path.
*
* RETURNS:
*     IBSU_STATUS_OK, if a directory is writable.
*     Error code < 0, otherwise.  See warning codes in 'IBScanUltimateApi_err'.
*         IBSU_ERR_CHANNEL_IO_WRITE_FAILED: Directory does not writable.
****************************************************************************************************
*)
function IBSU_IsWritableDirectory(
  dirpath: string;
  needCreateSubFolder: BOOL
): integer; stdcall;

(*
****************************************************************************************************
* RESERVED_GetFinalImageByNative()
* 
* DESCRIPTION:
*     get a native image for the final capture.
*
* ARGUMENTS:
*     handle                 Device handle.
*     pReservedKey           Key to unlock reserved functionality.
*     finalImage             Pointer to structure that will receive data of final image by native
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See warning codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function RESERVED_GetFinalImageByNative(
  handle: integer;
  pReservedKey: string;
  finalImage: IBSU_ImageData
): integer; stdcall;

(*
****************************************************************************************************
* IBSU_GenerateDisplayImage()
* 
* DESCRIPTION:
*     Generate scaled image in various formats for fast image display on canvas.
*     You can use instead of IBSU_GenerateZoomOutImageEx()
*
* ARGUMENTS:
*     inImage     Original grayscale image data.
*     inWidth     Width of input image.
*     in Height   Height of input image.
*     outImage    Pointer to buffer that will receive output image.  This buffer must hold at least
*                 'outWidth' x 'outHeight' x 'bitsPerPixel' bytes.
*     outWidth    Width of output image.
*     outHeight   Height of output image.
*     outBkColor     Background color of output image.
*     outFormat   IBSU_ImageFormat of output image.
*     outQualityLevel  Image quality of output image. The parameter must be in the range 0 through 2
*     outVerticalFlip  Enable/disable vertical flip of output image.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*)
function IBSU_GenerateDisplayImage(
	pInImage: pbyte;
	inWidth: integer;
	inHeight: integer;
  outImage: pbyte;
	outWidth: integer;
	outHeight: integer;
	outBkColor: byte;
	outFormat: IBSU_ImageFormat;
	outQualityLevel: integer;
	outVerticalFlip: BOOL
): integer; stdcall;

/*
****************************************************************************************************
* IBSU_RemoveFingerImages()
* 
* DESCRIPTION:
*     Remove finger images.
* 
* ARGUMENTS:
*     handle              Handle for device associated with this event (if appropriate).
*     fIndex              Bit-pattern of finger index of input image. 
*                         ex) IBSU_FINGER_LEFT_LITTLE | IBSU_FINGER_LEFT_RING in IBScanUltimateApi_defs.h
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*/
function IBSU_RemoveFingerImage(
	handle: integer;
	fIndex: DWORD
): integer; stdcall;


/*
****************************************************************************************************
* IBSU_AddFingerImage()
* 
* DESCRIPTION:
*     Add an finger image for the fingerprint duplicate check and roll to slap comparison.
*     It can have only ten prints 
* 
* ARGUMENTS:
*     handle              Handle for device associated with this event (if appropriate).
*     image               input image data.
*     fIndex              Bit-pattern of finger index of input image. 
*                         ex) IBSU_FINGER_LEFT_LITTLE | IBSU_FINGER_LEFT_RING in IBScanUltimateApi_defs.h
*     imageType           Image type of input image.
*     flagForce           Indicates whether input image should be saved even if another image is already stord or not.  TRUE to be stored force; FALSE to 
*                         be not stored force.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*/

function IBSU_AddFingerImage(
	handle: integer;
  image: IBSU_ImageData;
  fIndex:DWORD;
  imageType: IBSU_ImageType;
  flagForce: BOOL
): integer; stdcall;


/*
****************************************************************************************************
* IBSU_IsFingerDuplicated()
* 
* DESCRIPTION:
*     Checks for the fingerprint duplicate from the stored prints by IBSU_AddFingerImage(). 
* 
* ARGUMENTS:
*     handle              Handle for device associated with this event (if appropriate).
*     image               input image data for the fingerprint duplicate check.
*     fIndex              Bit-pattern of finger index of input image. 
*                         ex) IBSU_FINGER_LEFT_LITTLE | IBSU_FINGER_LEFT_RING in IBScanUltimateApi_defs.h
*     imageType           Image type of input image.
*     securityLevel       security level for the duplicate checks.
*     pMatchedPosition    Pointer to variable that will receive result of duplicate.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*/

function IBSU_IsFingerDuplicated(
	handle: integer;
  image: IBSU_ImageData;
  fIndex: DWORD;
  imageType: IBSU_ImageType;
  securityLevel: integer;
  pMatchedPosition: pinteger
): integer; stdcall;


/*
****************************************************************************************************
* IBSU_IsValidFingerGeometry()
* 
* DESCRIPTION:
*     Check for hand and finger geometry whether it is correct or not. 
* 
* ARGUMENTS:
*     handle              Handle for device associated with this event (if appropriate).
*     image               input image data for roll to slap comparison.
*     fIndex              Bit-pattern of finger index of input image. 
*                         ex) IBSU_FINGER_LEFT_LITTLE | IBSU_FINGER_LEFT_RING in IBScanUltimateApi_defs.h
*     imageType           Image type of input image.
*     pValid              Pointer to variable that will receive whether it is valid or not.  TRUE to valid; FALSE to invalid.
*
* RETURNS:
*     IBSU_STATUS_OK, if successful.
*     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
****************************************************************************************************
*/

function IBSU_IsValidFingerGeometry(
	handle: integer;
  image: IBSU_ImageData;
  fIndex: DWORD;
  imageType: IBSU_ImageType;
  pValid: pBOOL
): integer; stdcall;
     
     
implementation

(* ** DllName constant, change "DllName.dll" to name of the dll,
  ** to import API functions.
  ** e.g. if API functions imported from kernel32.dll
  ** change "DllName.dll" to "kernel32.dll" without quotes!!! *)
const
	DllName	= 'IBScanUltimate.dll';

function IBSU_GetSDKVersion; external DllName name 'IBSU_GetSDKVersion';
function IBSU_GetDeviceCount; external DllName name 'IBSU_GetDeviceCount';
function IBSU_GetDeviceDescription; external DllName name 'IBSU_GetDeviceDescription';
function IBSU_RegisterCallbacks; external DllName name 'IBSU_RegisterCallbacks';
function IBSU_OpenDevice; external DllName name 'IBSU_OpenDevice';
function IBSU_CloseDevice; external DllName name 'IBSU_CloseDevice';
function IBSU_CloseAllDevice; external DllName name 'IBSU_CloseAllDevice';
function IBSU_IsDeviceOpened; external DllName name 'IBSU_IsDeviceOpened';
function IBSU_SetProperty; external DllName name 'IBSU_SetProperty';
function IBSU_GetProperty; external DllName name 'IBSU_GetProperty';
function IBSU_EnableTraceLog; external DllName name 'IBSU_EnableTraceLog';
function IBSU_IsCaptureAvailable; external DllName name 'IBSU_IsCaptureAvailable';
function IBSU_BeginCaptureImage; external DllName name 'IBSU_BeginCaptureImage';
function IBSU_CancelCaptureImage; external DllName name 'IBSU_CancelCaptureImage';
function IBSU_IsCaptureActive; external DllName name 'IBSU_IsCaptureActive';
function IBSU_TakeResultImageManually; external DllName name 'IBSU_TakeResultImageManually';
function IBSU_GetContrast; external DllName name 'IBSU_GetContrast';
function IBSU_SetContrast; external DllName name 'IBSU_SetContrast';
function IBSU_SetLEOperationMode; external DllName name 'IBSU_SetLEOperationMode';
function IBSU_GetLEOperationMode; external DllName name 'IBSU_GetLEOperationMode';
function IBSU_IsTouchedFinger; external DllName name 'IBSU_IsTouchedFinger';
function IBSU_GetOperableLEDs; external DllName name 'IBSU_GetOperableLEDs';
function IBSU_GetLEDs; external DllName name 'IBSU_GetLEDs';
function IBSU_SetLEDs; external DllName name 'IBSU_SetLEDs';
function IBSU_CreateClientWindow; external DllName name 'IBSU_CreateClientWindow';
function IBSU_DestroyClientWindow; external DllName name 'IBSU_DestroyClientWindow';
function IBSU_GetClientWindowProperty; external DllName name 'IBSU_GetClientWindowProperty';
function IBSU_SetClientDisplayProperty; external DllName name 'IBSU_SetClientDisplayProperty';
function IBSU_SetClientWindowOverlayText; external DllName name 'IBSU_SetClientWindowOverlayText';
function IBSU_GenerateZoomOutImage; external DllName name 'IBSU_GenerateZoomOutImage';
function IBSU_SaveBitmapImage; external DllName name 'IBSU_SaveBitmapImage';
function IBSU_AsyncOpenDevice; external DllName name 'IBSU_AsyncOpenDevice';
function IBSU_BGetImage; external DllName name 'IBSU_BGetImage';
function IBSU_BGetImageEx; external DllName name 'IBSU_BGetImageEx';
function IBSU_BGetInitProgress; external DllName name 'IBSU_BGetInitProgress';
function IBSU_BGetClearPlatenAtCapture; external DllName name 'IBSU_BGetClearPlatenAtCapture';
function IBSU_BGetRollingInfo; external DllName name 'IBSU_BGetRollingInfo';
function IBSU_OpenDeviceEx; external DllName name 'IBSU_OpenDeviceEx';
function IBSU_GetIBSM_ResultImageInfo; external DllName name 'IBSU_GetIBSM_ResultImageInfo';
function IBSU_GetNFIQScore; external DllName name 'IBSU_GetNFIQScore';
function IBSU_GenerateZoomOutImageEx; external DllName name 'IBSU_GenerateZoomOutImageEx';
function IBSU_ReleaseCallbacks; external DllName name 'IBSU_ReleaseCallbacks';
function IBSU_SaveBitmapMem; external DllName name 'IBSU_SaveBitmapMem';
function IBSU_ShowOverlayObject; external DllName name 'IBSU_ShowOverlayObject';
function IBSU_ShowAllOverlayObject; external DllName name 'IBSU_ShowAllOverlayObject';
function IBSU_RemoveOverlayObject; external DllName name 'IBSU_RemoveOverlayObject';
function IBSU_RemoveAllOverlayObject; external DllName name 'IBSU_RemoveAllOverlayObject';
function IBSU_AddOverlayText; external DllName name 'IBSU_AddOverlayText';
function IBSU_ModifyOverlayText; external DllName name 'IBSU_ModifyOverlayText';
function IBSU_AddOverlayLine; external DllName name 'IBSU_AddOverlayLine';
function IBSU_ModifyOverlayLine; external DllName name 'IBSU_ModifyOverlayLine';
function IBSU_AddOverlayQuadrangle; external DllName name 'IBSU_AddOverlayQuadrangle';
function IBSU_ModifyOverlayQuadrangle; external DllName name 'IBSU_ModifyOverlayQuadrangle';
function IBSU_AddOverlayShape; external DllName name 'IBSU_AddOverlayShape';
function IBSU_ModifyOverlayShape; external DllName name 'IBSU_ModifyOverlayShape';
function IBSU_WSQEncodeMem; external DllName name 'IBSU_WSQEncodeMem';
function IBSU_WSQEncodeToFile; external DllName name 'IBSU_WSQEncodeToFile';
function IBSU_WSQDecodeMem; external DllName name 'IBSU_WSQDecodeMem';
function IBSU_WSQDecodeFromFile; external DllName name 'IBSU_WSQDecodeFromFile';
function IBSU_FreeMemory; external DllName name 'IBSU_FreeMemory';
function IBSU_SavePngImage; external DllName name 'IBSU_SavePngImage';
function IBSU_SaveJP2Image; external DllName name 'IBSU_SaveJP2Image';
function IBSU_RedrawClientWindow; external DllName name 'IBSU_RedrawClientWindow';
function IBSU_UnloadLibrary; external DllName name 'IBSU_UnloadLibrary';
function IBSU_CombineImage; external DllName name 'IBSU_CombineImage';
function IBSU_GetOperableBeeper; external DllName name 'IBSU_GetOperableBeeper';
function IBSU_SetBeeper; external DllName name 'IBSU_SetBeeper';
function IBSU_CombineImageEx; external DllName name 'IBSU_CombineImageEx';
function IBSU_BGetRollingInfoEx; external DllName name 'IBSU_BGetRollingInfoEx';
function IBSU_CheckWetFinger; external DllName name 'IBSU_CheckWetFinger';
function IBSU_GetImageWidth; external DllName name 'IBSU_GetImageWidth';
function IBSU_IsWritableDirectory; external DllName name 'IBSU_IsWritableDirectory';
function RESERVED_GetFinalImageByNative; external DllName name 'RESERVED_GetFinalImageByNative';
function IBSU_GenerateDisplayImage; external DllName name 'IBSU_GenerateDisplayImage';
function IBSU_RemoveFingerImage; external DllName name 'IBSU_RemoveFingerImage';
function IBSU_AddFingerImage; external DllName name 'IBSU_AddFingerImage';
function IBSU_IsFingerDuplicated; external DllName name 'IBSU_IsFingerDuplicated';
function IBSU_IsValidFingerGeometry; external DllName name 'IBSU_IsValidFingerGeometry';

end.

