/*
****************************************************************************************************
* IBScanUltimateApi.cs
*
* DESCRIPTION:
*     API functions for IBScanUltimate.
*     http://www.integratedbiometrics.com
*
* NOTES:
*     Copyright (c) Integrated Biometrics, 2009-2014
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
*     2014/02/25  1.7.1  Changed datatype from string to StringBuilder for IBSU_SetClientDisplayProperty
*     2014/07/23  1.8.0  Reformatted.
*                        Added API functions are relelated to WSQ
*                        (IBSU_WSQEncodeMem, IBSU_WSQEncodeToFile, IBSU_WSQDecodeMem,
*                         IBSU_WSQDecodeFromFile, IBSU_FreeMemory)
*     2014/09/17  1.8.1  Changed datatype from IBSM_ImageData to IntPtr for IBSU_GetIBSM_ResultImageInfo.
*                        Added API functions are relelated to JPEG2000 and PNG
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
****************************************************************************************************
*/

using System;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Text;

namespace IBscanUltimate
{
    public enum PlaySoundFlags : int
    {
        SND_SYNC = 0x0,     // play synchronously (default)
        SND_ASYNC = 0x1,    // play asynchronously
        SND_NODEFAULT = 0x2,    // silence (!default) if sound not found
        SND_MEMORY = 0x4,       // pszSound points to a memory file
        SND_LOOP = 0x8,     // loop the sound until next sndPlaySound
        SND_NOSTOP = 0x10,      // don't stop any currently playing sound
        SND_NOWAIT = 0x2000,    // don't wait if the driver is busy
        SND_ALIAS = 0x10000,    // name is a registry alias
        SND_ALIAS_ID = 0x110000,// alias is a predefined ID
        SND_FILENAME = 0x20000, // name is file name
        SND_RESOURCE = 0x40004, // name is resource name or atom
    }

    public class Win32
    {
        [DllImport("coredll.dll")]
        public static extern int GetClientRect(IntPtr hWnd, ref DLL.IBSU_RECT lpRect);

        [DllImport("coredll.dll")]
        public static extern void MessageBeep(int code);

        [DllImport("coredll.dll")]
        public static extern int SetFocus(IntPtr hWnd);

        [DllImport("coredll.dll")]
        //public static extern bool PlaySound(String Filename, int Mod, int Flags);  
        public static extern int PlaySound(string szSound,IntPtr hModule,int flags);

        [DllImport("shell32.dll")]
        public static extern bool SHGetSpecialFolderPath(IntPtr hwndOwner, StringBuilder lpszPath, int nFolder, bool fCreate);
        public enum CSIDL
        {
            CSIDL_DESKTOP = 0x0000, // 
            CSIDL_INTERNET = 0x0001, // Internet Explorer (icon on desktop)
            CSIDL_PROGRAMS = 0x0002, // Start Menu\Programs
            CSIDL_CONTROLS = 0x0003, // My Computer\Control Panel
            CSIDL_PRINTERS = 0x0004, // My Computer\Printers
            CSIDL_PERSONAL = 0x0005, // My Documents
            CSIDL_FAVORITES = 0x0006, // \Favorites
            CSIDL_STARTUP = 0x0007, // Start Menu\Programs\Startup
            CSIDL_RECENT = 0x0008, // \Recent
            CSIDL_SENDTO = 0x0009, // \SendTo
            CSIDL_BITBUCKET = 0x000a, // \Recycle Bin
            CSIDL_STARTMENU = 0x000b, // \Start Menu
            CSIDL_MYDOCUMENTS = 0x000c, // logical "My Documents" desktop icon
            CSIDL_MYMUSIC = 0x000d, // "My Music" folder
            CSIDL_MYVIDEO = 0x000e, // "My Videos" folder
            CSIDL_DESKTOPDIRECTORY = 0x0010, // \Desktop
            CSIDL_DRIVES = 0x0011, // My Computer
            CSIDL_NETWORK = 0x0012, // Network Neighborhood (My Network Places)
            CSIDL_NETHOOD = 0x0013, // \nethood
            CSIDL_FONTS = 0x0014, // windows\fonts
            CSIDL_TEMPLATES = 0x0015,
            CSIDL_COMMON_STARTMENU = 0x0016, // All Users\Start Menu
            CSIDL_COMMON_PROGRAMS = 0X0017, // All Users\Start Menu\Programs
            CSIDL_COMMON_STARTUP = 0x0018, // All Users\Startup
            CSIDL_COMMON_DESKTOPDIRECTORY = 0x0019, // All Users\Desktop
            CSIDL_APPDATA = 0x001a, // \Application Data
            CSIDL_PRINTHOOD = 0x001b, // \PrintHood

            CSIDL_LOCAL_APPDATA = 0x001c, // \Local Settings\Applicaiton Data (non roaming)

            CSIDL_ALTSTARTUP = 0x001d, // non localized startup
            CSIDL_COMMON_ALTSTARTUP = 0x001e, // non localized common startup
            CSIDL_COMMON_FAVORITES = 0x001f,

            CSIDL_INTERNET_CACHE = 0x0020,
            CSIDL_COOKIES = 0x0021,
            CSIDL_HISTORY = 0x0022,
            CSIDL_COMMON_APPDATA = 0x0023, // All Users\Application Data
            CSIDL_WINDOWS = 0x0024, // GetWindowsDirectory()
            CSIDL_SYSTEM = 0x0025, // GetSystemDirectory()
            CSIDL_PROGRAM_FILES = 0x0026, // C:\Program Files
            CSIDL_MYPICTURES = 0x0027, // C:\Program Files\My Pictures

            CSIDL_PROFILE = 0x0028, // USERPROFILE
            CSIDL_SYSTEMX86 = 0x0029, // x86 system directory on RISC
            CSIDL_PROGRAM_FILESX86 = 0x002a, // x86 C:\Program Files on RISC

            CSIDL_PROGRAM_FILES_COMMON = 0x002b, // C:\Program Files\Common

            CSIDL_PROGRAM_FILES_COMMONX86 = 0x002c, // x86 Program Files\Common on RISC
            CSIDL_COMMON_TEMPLATES = 0x002d, // All Users\Templates

            CSIDL_COMMON_DOCUMENTS = 0x002e, // All Users\Documents
            CSIDL_COMMON_ADMINTOOLS = 0x002f, // All Users\Start Menu\Programs\Administrative Tools
            CSIDL_ADMINTOOLS = 0x0030, // \Start Menu\Programs\Administrative Tools

            CSIDL_CONNECTIONS = 0x0031, // Network and Dial-up Connections
            CSIDL_COMMON_MUSIC = 0x0035, // All Users\My Music
            CSIDL_COMMON_PICTURES = 0x0036, // All Users\My Pictures
            CSIDL_COMMON_VIDEO = 0x0037, // All Users\My Video

            CSIDL_CDBURN_AREA = 0x003b // USERPROFILE\Local Settings\Application Data\Microsoft\CD Burning
        }
    }

    public partial class DLL
    {
        /*
        ****************************************************************************************************
        * CALLBACK FUNCTION TYPES
        ****************************************************************************************************
        */

        /*
        ****************************************************************************************************
        * IBSU_Callback()
        *
        * DESCRIPTION:
        *     Callback for ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, called when communication with a
        *     device is interrupted.
        *
        * ARGUMENTS:
        *     deviceHandle  Device handle.
        *     pContext      User context.
        ****************************************************************************************************
        */
        public delegate void IBSU_Callback(
            int         deviceHandle,
            IntPtr      pContext
            );

        /*
        ****************************************************************************************************
        * IBSU_CallbackPreviewImage()
        *
        * DESCRIPTION:
        *     Callback for ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, called when a preview image is available.
        *
        * ARGUMENTS:
        *     deviceHandle  Device handle.
        *     pContext      User context.
        *     image         Preview image data.  This structure, including the buffer, is valid only 
        *                   within the function context.  If required for later use, any data must be 
        *                   copied to another structure.
        ****************************************************************************************************
        */
        public delegate void IBSU_CallbackPreviewImage(
            int             deviceHandle,
            IntPtr          pContext,
            IBSU_ImageData  image
            );

        /*
        ****************************************************************************************************
        * IBSU_CallbackFingerCount()
        *
        * DESCRIPTION:
        *     Callback for ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, called when the finger count changes.
        *
        * ARGUMENTS:
        *     deviceHandle      Device handle.
        *     pContext          User context.
        *     fingerCountState  Finger count state.
        ****************************************************************************************************
        */
        public delegate void IBSU_CallbackFingerCount(
            int                     deviceHandle,
            IntPtr                  pContext,
            IBSU_FingerCountState   fingerCountState
            );

        /*
        ****************************************************************************************************
        * IBSU_CallbackFingerQuality()
        *
        * DESCRIPTION:
        *     Callback for ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY, called when a finger quality changes.
        *
        * ARGUMENTS:
        *     deviceHandle       Device handle.
        *     pContext           User context.
        *     pQualityArray      Array of finger qualities.
        *     qualityArrayCount  Number of qualities in array.
        ****************************************************************************************************
        */
        public delegate void IBSU_CallbackFingerQuality(
            int         deviceHandle,
            IntPtr      pContext,
            IntPtr      pQualityArray,
            int         qualityArrayCount
            );

        /*
        ****************************************************************************************************
        * IBSU_CallbackDeviceCount()
        *
        * DESCRIPTION:
        *     Callback for ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, called when the number of detected
        *     devices changes.
        *
        * ARGUMENTS:
        *     detectedDevices  Number of detected devices.
        *     pContext         User context.
        ****************************************************************************************************
        */
        public delegate void IBSU_CallbackDeviceCount(
            int         detectedDevices,
            IntPtr      pContext
            );

        /*
        ****************************************************************************************************
        * IBSU_CallbackInitProgress()
        *
        * DESCRIPTION:
        *     Callback for ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, called when the initialization progress
        *     changes for a device.
        *
        * ARGUMENTS:
        *     deviceIndex    Zero-based index of device.
        *     pContext       User context.
        *     progressValue  Initialization progress, as a percent, between 0 and 100, inclusive.
        ****************************************************************************************************
        */
        public delegate void IBSU_CallbackInitProgress(
            int         deviceIndex,
            IntPtr  pContext,
            int         progressValue
            );

        /*
        ****************************************************************************************************
        * IBSU_CallbackTakingAcquisition()
        *
        * DESCRIPTION:
        *     Callback for ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, called for a rolled print
        *     acquisition when the rolling should begin.
        *
        * ARGUMENTS:
        *     deviceHandle  Device handle.
        *     pContext      User context.
        *     imageType     Type of image being acquired.
        ****************************************************************************************************
        */
        public delegate void IBSU_CallbackTakingAcquisition(
            int             deviceHandle,
            IntPtr          pContext,
            IBSU_ImageType  imageType
            );

        /*
        ****************************************************************************************************
        * IBSU_CallbackCompleteAcquisition()
        *
        * DESCRIPTION:
        *     Callback for ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, called for a rolled print
        *     acquisition when the rolling capture has completed.
        *
        * ARGUMENTS:
        *     deviceHandle  Device handle.
        *     pContext      User context.
        *     imageType     Type of image being acquired.
        ****************************************************************************************************
        */
        public delegate void IBSU_CallbackCompleteAcquisition(
            int             deviceHandle,
            IntPtr          pContext,
            IBSU_ImageType  imageType
            );

        /*
        ****************************************************************************************************
        * IBSU_CallbackResultImage()
        *
        * DESCRIPTION:
        *     Callback for ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE, called when the result image is 
        *     available.
        *
        * ARGUMENTS:
        *     deviceHandle          Device handle.
        *     pContext              User context.
        *     image                 Data of preview or result image.  The buffer in this structure points to 
        *                           an internal image buffer; the data should be copied to an application 
        *                           buffer if desired for future processing.
        *     imageType             Image type.
        *     pSplitImageArray      Array of four structures with data of individual finger images split
        *                           from result image.  The buffers in these structures point to internal
        *                           image buffers; the data should be copied to application buffers if
        *                           desired for future processing.
        *     splitImageArrayCount  Number of finger images split from result images.
        ****************************************************************************************************
        */
        public delegate void IBSU_CallbackResultImage(
            int             deviceHandle,
            IntPtr          pContext,
            IBSU_ImageData  image,
            IBSU_ImageType  imageType,
            IntPtr          pSplitImageArray,
            int             splitImageArrayCount
            );

        /*
        ****************************************************************************************************
        * IBSU_CallbackResultImageEx()
        *
        * DESCRIPTION:
        *     Callback for ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX, called when the result image is 
        *     available, with extended information.
        *
        * ARGUMENTS:
        *     deviceHandle            Device handle.
        *     pContext                User context.
        *     imageStatus             Status from result image acquisition.  See error codes in 
        *                             'IBScanUltimateApi_err'.
        *     image                   Data of preview or result image.  The buffer in this structure points to 
        *                             an internal image buffer; the data should be copied to an application 
        *                             buffer if desired for future processing.
        *     imageType               Image type.
        *     detectedFingerCount     Number of detected fingers.
        *     segmentImageArrayCount  Number of finger images split from result images.
        *     pSegmentImageArray      Array of structures with data of individual finger images split from
        *                             result image.  The buffers in these structures point to internal image 
        *                             buffers; the data should be copied to application buffers if desired 
        *                             for future processing.
        *     pSegmentPositionArray   Array of structures with position data for individual fingers split 
        *                             from result image.
        ****************************************************************************************************
        */
        public delegate void IBSU_CallbackResultImageEx(
            int             deviceHandle,
            IntPtr          pContext,
            int             imageStatus,
            IBSU_ImageData  image,
		    IBSU_ImageType  imageType,
		    int             detectedFingerCount,
            int             segmentImageArrayCount,
            IntPtr          pSegmentImageArray,
		    IntPtr          SegmentPositionArray
            );

        /*
        ****************************************************************************************************
        * IBSU_CallbackClearPlatenAtCapture()
        *
        * DESCRIPTION:
        *     Callback for ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE, called when the platen was not 
        *     clear when capture started or has since become clear.
        *
        * ARGUMENTS:
        *     deviceHandle  Device handle.
        *     pContext      User context.
        *     platenState   Platen state.
        ****************************************************************************************************
        */
        public delegate void IBSU_CallbackClearPlatenAtCapture(
            int                 deviceHandle,
            IntPtr              pContext,
            IBSU_PlatenState    platenState
            );

        /*
        ****************************************************************************************************
        * IBSU_CallbackAsyncOpenDevice()
        *
        * DESCRIPTION:
        *     Callback for ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE, called asynchronous device 
        *     initialization completes
        *
        * ARGUMENTS:
        *     deviceIndex   Zero-based index of device.
        *     pContext      User context.
        *     deviceHandle  Handle for subsequent function calls.
        *     errorCode     Error that prevented initialization from completing.
        ****************************************************************************************************
        */
        public delegate void IBSU_CallbackAsyncOpenDevice(
            int     deviceIndex,
            IntPtr  pContext,
            int     deviceHandle,
            int     errorCode
            );

        /*
        ****************************************************************************************************
        * IBSU_CallbackNotifyMessage()
        *
        * DESCRIPTION:
        *     Callback for ENUM_IBSU_OPTIONAL_EVENT_NOTIFY_MESSAGE, called when a warning message is 
        *     generated.
        *
        * ARGUMENTS:
        *     deviceHandle   Device handle.
        *     pContext       User context.
        *     notifyMessage  Status code as defined in IBScanUltimateApi_err.cs.
        ****************************************************************************************************
        */
        public delegate void IBSU_CallbackNotifyMessage(
            int     deviceHandle,
            IntPtr  pContext,
            int     notifyMessage
            );

        /*
        ****************************************************************************************************
        * IBSU_CallbackKeyButtons()
        *
        * DESCRIPTION:
        *     Callback for ENUM_IBSU_ESSENTIAL_EVENT_KEYBUTTON, called when the key button of device was chicked.
        *
        * ARGUMENTS:
        *     deviceHandle       Device handle.
        *     pContext           User context.
        *     pressedKeyButtons  The key button index which is pressed.
        ****************************************************************************************************
        */
        public delegate void IBSU_CallbackKeyButtons(
            int     deviceHandle,
            IntPtr  pContext,
            int     pressedKeyButtons
            );

        /*
        ****************************************************************************************************
        * GLOBAL FUNCTIONS
        ****************************************************************************************************
        */

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_GetSDKVersion(
            ref IBSU_SdkVersion pVerinfo                        
                                                                
            );
        public static int _IBSU_GetSDKVersion(
            ref IBSU_SdkVersion pVerinfo
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_GetSDKVersion(ref pVerinfo);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_GetSDKVersion : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_GetDeviceCount(
            ref int pDeviceCount                            
                                                               
            );
        public static int _IBSU_GetDeviceCount(
            ref int pDeviceCount
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_GetDeviceCount(ref pDeviceCount);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_GetDeviceCount : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_GetDeviceDescription(
            int                    deviceIndex,               
            ref IBSU_DeviceDesc    pDeviceDesc                
                                                              
            );
        public static int _IBSU_GetDeviceDescription(
            int                    deviceIndex,
            ref IBSU_DeviceDesc    pDeviceDesc
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_GetDeviceDescription(deviceIndex, ref pDeviceDesc);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_GetDeviceDescription : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_RegisterCallbacks(
            int                 handle,                      
            IBSU_Events         events,                       
            Delegate            pEventName,                  
            IntPtr              pContext                     
            );
        public static int _IBSU_RegisterCallbacks(
            int                 handle,
            IBSU_Events         events,
            Delegate            pEventName,
            IntPtr              pContext
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_RegisterCallbacks(handle, events, pEventName, pContext);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_RegisterCallbacks : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_OpenDevice(
            int         deviceIndex,                            
            ref int     handle                                  
                                                                
            );
        public static int _IBSU_OpenDevice(
            int         deviceIndex,
            ref int     handle
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_OpenDevice(deviceIndex, ref handle);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_OpenDevice : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_CloseDevice(
            int  handle                                 
            );
        public static int _IBSU_CloseDevice(
            int handle
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_CloseDevice(handle);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_CloseDevice : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_CloseAllDevice(
            );
        public static int _IBSU_CloseAllDevice(
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_CloseAllDevice();
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_CloseAllDevice : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_IsDeviceOpened(
            int  handle                                 
            );
        public static int _IBSU_IsDeviceOpened(
            int handle
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_IsDeviceOpened(handle);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_IsDeviceOpened : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_SetProperty(
            int             handle,                     
            IBSU_PropertyId propertyId,                 
            StringBuilder   propertyValue               
            );
        public static int _IBSU_SetProperty(
            int             handle,
            IBSU_PropertyId propertyId,
            StringBuilder   propertyValue
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_SetProperty(handle, propertyId, propertyValue);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_SetProperty : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_GetProperty(
            int             handle,                     
            IBSU_PropertyId propertyId,                 
            StringBuilder   propertyValue               
            );
        public static int _IBSU_GetProperty(
            int             handle,
            IBSU_PropertyId propertyId,
            StringBuilder   propertyValue
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_GetProperty(handle, propertyId, propertyValue);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_GetProperty : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_EnableTraceLog(
            bool on               
        );
        public static int _IBSU_EnableTraceLog(
            bool on
            )
        {
            int nRc = IBSU_STATUS_OK;
            
            try
            {
                nRc = IBSU_EnableTraceLog(on);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_EnableTraceLog : " + except.Message);            
            }
            
            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_IsCaptureAvailable(
            int                     handle,               
            IBSU_ImageType          imageType,            
            IBSU_ImageResolution    imageResolution,      
            ref bool                pIsAvailable          
                                                          
            );
        public static int _IBSU_IsCaptureAvailable(
            int                     handle,
            IBSU_ImageType          imageType,
            IBSU_ImageResolution    imageResolution,
            ref bool                pIsAvailable
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_IsCaptureAvailable(handle, imageType, imageResolution, ref pIsAvailable);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_IsCaptureAvailable : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_BeginCaptureImage(
            int                     handle,               
            IBSU_ImageType          imageType,            
            IBSU_ImageResolution    imageResolution,      
            uint                    captureOptions        
            );
        public static int _IBSU_BeginCaptureImage(
            int                     handle,
            IBSU_ImageType          imageType,
            IBSU_ImageResolution    imageResolution,
            uint                    captureOptions
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_BeginCaptureImage(handle, imageType, imageResolution, captureOptions);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_BeginCaptureImage : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_CancelCaptureImage(
            int handle               
            );
        public static int _IBSU_CancelCaptureImage(
            int handle
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_CancelCaptureImage(handle);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_CancelCaptureImage : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_IsCaptureActive(
            int         handle,                                 
            ref bool    pIsActive                                                                                 
            );
        public static int _IBSU_IsCaptureActive(
            int         handle,
            ref bool    pIsActive
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_IsCaptureActive(handle, ref pIsActive);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_IsCaptureActive : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_TakeResultImageManually(
            int handle                                 
            );
        public static int _IBSU_TakeResultImageManually(
            int handle
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_TakeResultImageManually(handle);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_TakeResultImageManually : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_GetContrast(
            int         handle,                                 
            ref int     pContrastValue                          
                                                                
            );
        public static int _IBSU_GetContrast(
            int         handle,
            ref int     pContrastValue
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_GetContrast(handle, ref pContrastValue);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_GetContrast : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_SetContrast(
            int handle,                                 
            int contrastValue                          
            );
        public static int _IBSU_SetContrast(
            int handle,
            int contrastValue
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_SetContrast(handle, contrastValue);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_SetContrast : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_SetLEOperationMode(
            int                     handle,                
            IBSU_LEOperationMode    leOperationMode        
            );
        public static int _IBSU_SetLEOperationMode(
            int                     handle,
            IBSU_LEOperationMode    leOperationMode
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_SetLEOperationMode(handle, leOperationMode);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_SetLEOperationMode : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_GetLEOperationMode(
            int                         handle,                 
            ref IBSU_LEOperationMode    leOperationMode         
            );
        public static int _IBSU_GetLEOperationMode(
            int                         handle,
            ref IBSU_LEOperationMode    leOperationMode
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_GetLEOperationMode(handle, ref leOperationMode);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_GetLEOperationMode : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_IsTouchedFinger(
            int     handle,                                 
            ref int touchInValue                            
                                                            
            );
        public static int _IBSU_IsTouchedFinger(
            int handle,
            ref int touchInValue
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_IsTouchedFinger(handle, ref touchInValue);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_IsTouchedFinger : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_GetOperableLEDs(
            int                 handle,                         
            ref IBSU_LedType    pLedType,                       
                                                                
            ref int             pLedCount,                      
                                                                
            ref uint            pOperableLEDs                   
         );
        public static int _IBSU_GetOperableLEDs(
            int                 handle,
            ref IBSU_LedType    pLedType,
            ref int             pLedCount,
            ref uint            pOperableLEDs
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_GetOperableLEDs(handle, ref pLedType, ref pLedCount, ref pOperableLEDs);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_GetOperableLEDs : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_GetLEDs(
            int         handle,                             
            ref uint    pActiveLEDs                         
                                                            
            );
        public static int _IBSU_GetLEDs(
            int         handle,
            ref uint    pActiveLEDs
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_GetLEDs(handle, ref pActiveLEDs);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_GetLEDs : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_SetLEDs(
            int     handle,                              
            uint    activeLEDs                           
            );
        public static int _IBSU_SetLEDs(
            int     handle,
            uint    activeLEDs
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_SetLEDs(handle, activeLEDs);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_SetLEDs : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_CreateClientWindow(
            int         handle,                          
            IntPtr      hWindow,                         
            int         left,                            
            int         top,                             
            int         right,                           
            int         bottom                           
            );
        public static int _IBSU_CreateClientWindow(
            int         handle,
            IntPtr      hWindow,
            int         left,
            int         top,
            int         right,
            int         bottom
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_CreateClientWindow(handle, hWindow, left, top, right, bottom);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_CreateClientWindow : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_DestroyClientWindow(
            int handle,                          ///< [in] Device handle obtained by IBSU_OpenDevice()
            bool clearExistingInfo                ///< [in] Clear the existing display information about display property, overlay text.
            );
        public static int _IBSU_DestroyClientWindow(
            int handle,
            bool clearExistingInfo
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_DestroyClientWindow(handle, clearExistingInfo);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_DestroyClientWindow : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_GetClientWindowProperty(
            int                     handle,                 ///< [in]  Device handle obtained by IBSU_OpenDevice()
            IBSU_ClientWindowPropertyId  propertyId,             ///< [in]  Property identifier to set value for
            StringBuilder              propertyValue           ///< [out] String returning property value \n
                                                            ///<       Memory must be provided by caller
                                                            ///<       (needs to be able to hold @ref IBSU_MAX_STR_LEN characters)
            );
        public static int _IBSU_GetClientWindowProperty(
            int handle,
            IBSU_ClientWindowPropertyId propertyId,
            StringBuilder propertyValue
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_GetClientWindowProperty(handle, propertyId, propertyValue);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_GetClientWindowProperty : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_SetClientDisplayProperty(
            int                     handle,                 
            IBSU_ClientWindowPropertyId  propertyId,             
            StringBuilder                propertyValue           
            );
        public static int _IBSU_SetClientDisplayProperty(
            int                     handle,
            IBSU_ClientWindowPropertyId  propertyId,
            StringBuilder                propertyValue
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_SetClientDisplayProperty(handle, propertyId, propertyValue);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_SetClientDisplayProperty : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_SetClientWindowOverlayText(
            int         handle,                            
            string      fontName,                         
            int         fontSize,                          
            bool        fontBold,                          
            string      text,                             
            int         posX,                              
            int         posY,                              
            uint        textColor                          
            );
        public static int _IBSU_SetClientWindowOverlayText(
            int         handle,
            string      fontName,
            int         fontSize,
            bool        fontBold,
            string      text,
            int         posX,
            int         posY,
            uint        textColor
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_SetClientWindowOverlayText(handle, fontName, fontSize, fontBold, text, posX, posY, textColor);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_SetClientWindowOverlayText : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_GenerateZoomOutImage(
            IBSU_ImageData  inImage,                       
            IntPtr          outImage,                     
                                                               
            int             outWidth,                      
            int             outHeight,                     
            byte            bkColor                        
            );
        public static int _IBSU_GenerateZoomOutImage(
            IBSU_ImageData  inImage,
            IntPtr          outImage,
            int             outWidth,
            int             outHeight,
            byte            bkColor
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_GenerateZoomOutImage(inImage, outImage, outWidth, outHeight, bkColor);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_GenerateZoomOutImage : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_SaveBitmapImage(
            string      filePath,                       
            IntPtr      imgBuffer,                      
            uint        width,                          
            uint        height,                         
            int         pitch,                          
                                                        
                                                        
            double      resX,                           
            double      resY                            
            );
        public static int _IBSU_SaveBitmapImage(
            string      filePath,
            IntPtr      imgBuffer,
            uint        width,
            uint        height,
            int         pitch,
            double      resX,
            double      resY
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_SaveBitmapImage(filePath, imgBuffer, width, height, pitch, resX, resY);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_SaveBitmapImage : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_AsyncOpenDevice(
            int deviceIndex                            
            );
        public static int _IBSU_AsyncOpenDevice(
            int deviceIndex
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_AsyncOpenDevice(deviceIndex);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_AsyncOpenDevice : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_OpenDeviceEx(
            int	        deviceIndex,                            
            string      uniformityMaskPath,                     
                                                                
            bool        asyncOpen,                              
            ref int     pHandle                                 
                                                                
            );
        public static int _IBSU_OpenDeviceEx(
            int         deviceIndex,
            string      uniformityMaskPath,
            bool        asyncOpen,
            ref int     pHandle
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_OpenDeviceEx(deviceIndex, uniformityMaskPath, asyncOpen, ref pHandle);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_OpenDeviceEx : " + except.Message);
            }

            return nRc;
        }

        /*
        ****************************************************************************************************
        * IBSU_GetIBSM_ResultImageInfo()
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_GetIBSM_ResultImageInfo(
            int		            handle,                         
            IBSM_FingerPosition fingerPosition,                 
            IntPtr              pResultImage,
            IntPtr              pSplitResultImage,
            ref int             pSplitResultImageCount         
            );
        public static int _IBSU_GetIBSM_ResultImageInfo(
            int                 handle,
            IBSM_FingerPosition fingerPosition,
            IntPtr              pResultImage,
            IntPtr              pSplitResultImage,
            ref int             pSplitResultImageCount
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_GetIBSM_ResultImageInfo(handle, fingerPosition, pResultImage, pSplitResultImage, ref pSplitResultImageCount);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_GetIBSM_ResultImageInfo : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_GetNFIQScore(
            int         handle,                         ///< [in]  Device handle
            IntPtr      imgBuffer,                      ///< [in]  Point to image data
            uint        width,                          ///< [in]  Image width
            uint        height,                         ///< [in]  Image height
            byte        bitsPerPixel,                   ///< [in]  Number of Bits per pixel
            ref int     pScore                          ///< [out] NFIQ score
            );
        public static int _IBSU_GetNFIQScore(
            int         handle,
            IntPtr      imgBuffer,
            uint        width,
            uint        height,
            byte        bitsPerPixel,
            ref int     pScore
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_GetNFIQScore(handle, imgBuffer, width, height, bitsPerPixel, ref pScore);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_GetNFIQScore : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_GenerateZoomOutImageEx(
            IntPtr          pInImage,                      
            int             inWidth,                       
            int             inHeight,                      
            IntPtr          outImage,                      
                                                               
            int             outWidth,                      
            int             outHeight,                     
            byte            bkColor                        
            );
        public static int _IBSU_GenerateZoomOutImageEx(
            IntPtr          pInImage,
            int             inWidth,
            int             inHeight,
            IntPtr          outImage,
            int             outWidth,
            int             outHeight,
            byte            bkColor
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_GenerateZoomOutImageEx(pInImage, inWidth, inHeight, outImage, outWidth, outHeight, bkColor);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_GenerateZoomOutImageEx : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_ReleaseCallbacks(
            int                 handle,                      
            IBSU_Events         events                        
            );
        public static int _IBSU_ReleaseCallbacks(
            int                 handle,                      
            IBSU_Events         events                        
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_ReleaseCallbacks(handle, events);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_ReleaseCallbacks : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_SaveBitmapMem(
            IntPtr              inImage,                    
            uint                width,                      
            uint                height,                     
            int                 pitch,                      
                                                            
                                                            
            double              resX,                       
            double              resY,                       
            IntPtr              outBitmapBuffer,            
                                                       
            IBSU_ImageFormat    outImageFormat,             
            uint                outWidth,                   
            uint                outHeight,                  
            byte                bkColor                     
            );
        public static int _IBSU_SaveBitmapMem(
            IntPtr              inImage,                    
            uint                width,                      
            uint                height,                     
            int                 pitch,                      
                                                            
            double              resX,                       
            double              resY,                       
            IntPtr              outBitmapBuffer,            
                                                            
            IBSU_ImageFormat    outImageFormat,             
            uint                outWidth,                   
            uint                outHeight,                  
            byte                bkColor                     
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_SaveBitmapMem(inImage, width, height, pitch, resX, resY, outBitmapBuffer, outImageFormat, outWidth, outHeight, bkColor);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_SaveBitmapMem : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_ShowOverlayObject(
            int         handle,                             
            int         overlayHandle,                      
            bool        show                                
            );
        public static int _IBSU_ShowOverlayObject(
            int         handle,                             
            int         overlayHandle,                      
            bool        show                                
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_ShowOverlayObject(handle, overlayHandle, show);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_ShowOverlayObject : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_ShowAllOverlayObject(
            int         handle,                             
            bool        show                                
            );
        public static int _IBSU_ShowAllOverlayObject(
            int         handle,                             
            bool        show                                
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_ShowAllOverlayObject(handle, show);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_ShowAllOverlayObject : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_RemoveOverlayObject(
            int         handle,                             
            int         overlayHandle                      
            );
        public static int _IBSU_RemoveOverlayObject(
            int         handle,                             
            int         overlayHandle                      
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_RemoveOverlayObject(handle, overlayHandle);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_RemoveOverlayObject : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_RemoveAllOverlayObject(
            int handle                             
            );
        public static int _IBSU_RemoveAllOverlayObject(
            int handle                             
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_RemoveAllOverlayObject(handle);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_RemoveAllOverlayObject : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_AddOverlayText(
            int         handle,                            
            ref int     pOverlayHandle,                    
                                                           
            string      fontName,                          
	        int         fontSize,                           
	        bool        fontBold,                           
	        string      text,                              
	        int         posX,                               
	        int         posY,                               
	        uint        textColor                           
            );
        public static int _IBSU_AddOverlayText(
            int handle,                             
            ref int pOverlayHandle,                    
            
            string fontName,                          
            int fontSize,                           
            bool fontBold,                           
            string text,                              
            int posX,                               
            int posY,                               
            uint textColor                           
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_AddOverlayText(handle, ref pOverlayHandle, fontName, fontSize, fontBold, text, posX, posY, textColor);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_AddOverlayText : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_ModifyOverlayText(
            int handle,                             
            int overlayHandle,                    
            string fontName,                          
            int fontSize,                           
            bool fontBold,                      
            string text,                          
            int posX,                            
            int posY,                           
            uint textColor                  
            );
        public static int _IBSU_ModifyOverlayText(
            int handle,                            
            int overlayHandle,                    
            string fontName,                          
            int fontSize,                           
            bool fontBold,                         
            string text,                             
            int posX,                            
            int posY,                              
            uint textColor                         
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_ModifyOverlayText(handle, overlayHandle, fontName, fontSize, fontBold, text, posX, posY, textColor);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_ModifyOverlayText : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_AddOverlayLine(
            int handle,                           
            ref int pOverlayHandle,                   
           
            int         x1,                                
	        int         y1,                                 
	        int         x2,                              
	        int         y2,                             
	        int         lineWidth,                         
	        uint        lineColor                         
            );
        public static int _IBSU_AddOverlayLine(
            int handle,                            
            ref int pOverlayHandle,                    
          
            int x1,                                
            int y1,                                
            int x2,                               
            int y2,                                 
            int lineWidth,                        
            uint lineColor                       
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_AddOverlayLine(handle, ref pOverlayHandle, x1, y1, x2, y2, lineWidth, lineColor);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_AddOverlayLine : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_ModifyOverlayLine(
            int handle,                             
            int overlayHandle,                      
            int x1,                                 
            int y1,                                 
            int x2,                                 
            int y2,                                 
            int lineWidth,                          
            uint lineColor                          
            );
        public static int _IBSU_ModifyOverlayLine(
            int handle,                             
            int overlayHandle,                      ///< [in]  Overlay handle to be modified
            int x1,                                 ///< [in]  X coordinate of starting point of line
            int y1,                                 ///< [in]  Y coordinate of starting point of line
            int x2,                                 ///< [in]  X coordinate of ending point of line
            int y2,                                 ///< [in]  Y coordinate of ending point of line
            int lineWidth,                          ///< [in]  line width
            uint lineColor                          ///< [in]  line color
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_ModifyOverlayLine(handle, overlayHandle, x1, y1, x2, y2, lineWidth, lineColor);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_ModifyOverlayLine : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_AddOverlayQuadrangle(
            int handle,                             
            ref int pOverlayHandle,                    
            
            int         x1,                                 
            int         y1,                                 
            int         x2,                                 
            int         y2,                                 
            int         x3,                                 
            int         y3,                                 
            int         x4,                                 
            int         y4,                                 
            int         lineWidth,                          
            uint        lineColor                           
            );
        public static int _IBSU_AddOverlayQuadrangle(
            int handle,                             
            ref int pOverlayHandle,                  
           
            int x1,                                 
            int y1,                                 
            int x2,                                 
            int y2,                                 
            int x3,                                 
            int y3,                                 
            int x4,                                 
            int y4,                                 
            int lineWidth,                          
            uint lineColor                           
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_AddOverlayQuadrangle(handle, ref pOverlayHandle, x1, y1, x2, y2, x3, y3, x4, y4, lineWidth, lineColor);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_AddOverlayQuadrangle : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_ModifyOverlayQuadrangle(
            int handle,                             
            int overlayHandle,                      
            int x1,                                 
            int y1,                                 
            int x2,                                 
            int y2,                                 
            int x3,                                 
            int y3,                                 
            int x4,                                 
            int y4,                                 
            int lineWidth,                          
            uint lineColor                           
            );
        public static int _IBSU_ModifyOverlayQuadrangle(
            int handle,                             
            int overlayHandle,                      
            int x1,                                 
            int y1,                                 
            int x2,                                 
            int y2,                                 
            int x3,                                 
            int y3,                                 
            int x4,                                 
            int y4,                                 
            int lineWidth,                          
            uint lineColor                           
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_ModifyOverlayQuadrangle(handle, overlayHandle, x1, y1, x2, y2, x3, y3, x4, y4, lineWidth, lineColor);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_ModifyOverlayQuadrangle : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_AddOverlayShape(
            int handle,                             
            ref int pOverlayHandle,                    
            IBSU_OverlayShapePattern shapePattern,          
	        int						 x1,                    
	        int						 y1,                    
	        int						 x2,                    
	        int						 y2,                    
	        int						 lineWidth,             
	        uint					 lineColor,             
	        int						 reserved_1,            
	        int						 reserved_2            
            );
        public static int _IBSU_AddOverlayShape(
            int handle,                             
            ref int pOverlayHandle,                    
            IBSU_OverlayShapePattern shapePattern,         
            int x1,                   
            int y1,                    
            int x2,                    
            int y2,                    
            int lineWidth,             
            uint lineColor,            
            int reserved_1,            
            int reserved_2             
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_AddOverlayShape(handle, ref pOverlayHandle, shapePattern, x1, y1, x2, y2, lineWidth, lineColor, reserved_1, reserved_2);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_AddOverlayShape : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_ModifyOverlayShape(
            int handle,                             
            int overlayHandle,                      
            IBSU_OverlayShapePattern shapePattern,          
            int x1,                    
            int y1,                    
            int x2,                    
            int y2,                    
            int lineWidth,             
            uint lineColor,             
            int reserved_1,            
            int reserved_2             
            );
        public static int _IBSU_ModifyOverlayShape(
            int handle,                             
            int overlayHandle,                      
            IBSU_OverlayShapePattern shapePattern,          
            int x1,                    
            int y1,                    
            int x2,                    
            int y2,                    
            int lineWidth,             
            uint lineColor,             
            int reserved_1,            
            int reserved_2             
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_ModifyOverlayShape(handle, overlayHandle, shapePattern, x1, y1, x2, y2, lineWidth, lineColor, reserved_1, reserved_2);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_ModifyOverlayShape : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_WSQEncodeMem(
            IntPtr image,
            int width,
            int height,
            int pitch,
            int bitsPerPixel,
	        int pixelPerInch,
	        double bitRate,
	        string commentText,
	        IntPtr compressedData,
	        ref int compressedLength
            );
        public static int _IBSU_WSQEncodeMem(
            IntPtr image,
            int width,
            int height,
            int pitch,
            int bitsPerPixel,
	        int pixelPerInch,
	        double bitRate,
	        string commentText,
	        IntPtr compressedData,
	        ref int compressedLength
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_WSQEncodeMem(image, width, height, pitch, bitsPerPixel, pixelPerInch,
                    bitRate, commentText, compressedData, ref compressedLength);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_WSQEncodeMem : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_WSQEncodeToFile(
            string filePath,
            IntPtr image,
            int width,
            int height,
            int pitch,
            int bitsPerPixel,
	        int pixelPerInch,
	        double bitRate,
	        string commentText
            );
        public static int _IBSU_WSQEncodeToFile(
            string filePath,
            IntPtr image,
            int width,
            int height,
            int pitch,
            int bitsPerPixel,
	        int pixelPerInch,
	        double bitRate,
	        string commentText
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_WSQEncodeToFile(filePath, image, width, height, pitch, bitsPerPixel, pixelPerInch,
                    bitRate, commentText);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_WSQEncodeToFile : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_WSQDecodeMem(
            IntPtr compressedImage,
            int compressedLength,
            IntPtr decompressedImage,
            ref int outWidth,
            ref int outHeight,
            ref int outPitch,
            ref int outBitsPerPixel,
	        ref int outPixelPerInch
            );
        public static int _IBSU_WSQDecodeMem(
            IntPtr compressedImage,
            int compressedLength,
            IntPtr decompressedImage,
            ref int outWidth,
            ref int outHeight,
            ref int outPitch,
            ref int outBitsPerPixel,
	        ref int outPixelPerInch
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_WSQDecodeMem(compressedImage, compressedLength, decompressedImage,
                    ref outWidth, ref outHeight, ref outPitch, ref outBitsPerPixel, ref outPixelPerInch);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_WSQDecodeMem : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_WSQDecodeFromFile(
            string filePath,
            IntPtr decompressedImage,
            ref int outWidth,
            ref int outHeight,
            ref int outPitch,
            ref int outBitsPerPixel,
	        ref int outPixelPerInch
            );
        public static int _IBSU_WSQDecodeFromFile(
            string filePath,
            IntPtr decompressedImage,
            ref int outWidth,
            ref int outHeight,
            ref int outPitch,
            ref int outBitsPerPixel,
	        ref int outPixelPerInch
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_WSQDecodeFromFile(filePath, decompressedImage,
                    ref outWidth, ref outHeight, ref outPitch, ref outBitsPerPixel, ref outPixelPerInch);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_WSQDecodeFromFile : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_FreeMemory(
            IntPtr memblock
            );
        public static int _IBSU_FreeMemory(
            IntPtr memblock
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_FreeMemory(memblock);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_FreeMemory : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_SavePngImage(
            string filePath,
            IntPtr imgBuffer,
            uint width,
            uint height,
            int pitch,
            double resX,
            double resY
            );
        public static int _IBSU_SavePngImage(
            string filePath,
            IntPtr imgBuffer,
            uint width,
            uint height,
            int pitch,
            double resX,
            double resY
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_SavePngImage(filePath, imgBuffer, width, height, pitch, resX, resY);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_SavePngImage : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_SaveJP2Image(
            string filePath,
            IntPtr imgBuffer,
            uint width,
            uint height,
            int pitch,
            double resX,
            double resY,
            int fQuality
            );
        public static int _IBSU_SaveJP2Image(
            string filePath,
            IntPtr imgBuffer,
            uint width,
            uint height,
            int pitch,
            double resX,
            double resY,
            int fQuality
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_SaveJP2Image(filePath, imgBuffer, width, height, pitch, resX, resY, fQuality);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_SaveJP2Image : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_RedrawClientWindow(
            int handle
            );
        public static int _IBSU_RedrawClientWindow(
            int handle
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_RedrawClientWindow(handle);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_RedrawClientWindow : " + except.Message);
            }

            return nRc;
        }

        /*
        ****************************************************************************************************
        * IBSU_UnloadLibrary()
        * 
        * DESCRIPTION:
        *     The library is unmapped from the address space manually, and the library is no longer valid
        *
        * ARGUMENTS:
        *
        * RETURNS:
        *     IBSU_STATUS_OK, if successful.
        *     Error code < 0, otherwise.  See error codes in 'IBScanUltimateApi_err'.
        ****************************************************************************************************
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_UnloadLibrary();
        public static int _IBSU_UnloadLibrary()
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_UnloadLibrary();
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_UnloadLibrary : " + except.Message);
            }

            return nRc;
        }
        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_GetOperableBeeper(
            int                    handle,
            ref IBSU_BeeperType    pBeeperType
            );
        public static int _IBSU_GetOperableBeeper(
            int                    handle,
            ref IBSU_BeeperType    pBeeperType
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_GetOperableBeeper(handle, ref pBeeperType);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_GetOperableBeeper : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_SetBeeper(
            int                 handle,
            IBSU_BeepPattern    beepPattern,
            uint                soundTone,
            uint                duration,
            uint                reserved_1,
            uint                reserved_2
            );
        public static int _IBSU_SetBeeper(
            int                 handle,
            IBSU_BeepPattern    beepPattern,
            uint                soundTone,
            uint                duration,
            uint                reserved_1,
            uint                reserved_2
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_SetBeeper(handle, beepPattern, soundTone, duration, reserved_1, reserved_2);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_SetBeeper : " + except.Message);
            }

            return nRc;
        }

        /*
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
        */
        [DllImport("IBScanUltimate_WM6.dll", CharSet = CharSet.Auto, SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        private static extern int IBSU_CombineImage(
            IBSU_ImageData inImage1,
            IBSU_ImageData inImage2,
            IBSU_CombineImageWhichHand whichHand,
            ref IBSU_ImageData outImage
            );
        public static int _IBSU_CombineImage(
            IBSU_ImageData inImage1,
            IBSU_ImageData inImage2,
            IBSU_CombineImageWhichHand whichHand,
            ref IBSU_ImageData outImage
            )
        {
            int nRc = IBSU_STATUS_OK;
            try
            {
                nRc = IBSU_CombineImage(inImage1, inImage2, whichHand, ref outImage);
            }
            catch (Exception except)
            {
                IBScanUlitmateWM.include.LogManager.WriteToLog("IBSU_CombineImage : " + except.Message);
            }

            return nRc;
        }
    }
}
