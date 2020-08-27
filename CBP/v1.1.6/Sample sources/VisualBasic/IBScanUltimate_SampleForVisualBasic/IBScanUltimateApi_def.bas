Attribute VB_Name = "Module3"
'/*
'****************************************************************************************************
'* IBScanUltimateApi_defs.vb
'*
'* DESCRIPTION:
'*     API structures and constants for IBScanUltimate.
'*     http://www.integratedbiometrics.com
'*
'* NOTES:
'*     Copyright (c) Integrated Biometrics, 2009-2017
'*
'* HISTORY:
'*     2012/04/06  1.0.0  Created.
'*     2012/05/29  1.1.0  Added callback function (IBSU_CallbackAsyncOpenDevice).
'*     2012/09/05  1.3.0  Added enumeration value to IBSU_PropertyId
'*                            (ENUM_IBSU_PROPERTY_ENABLE_POWER_SAVE_MODE)
'*                        Modified IBSU_DeviceDesc structure for Android.
'*     2012/11/06  1.4.1  Added enumeration for RollingState (IBSU_RollingState).
'*     2013/02/01  1.5.0  Added enumeration value to IBSU_ImageType (ENUM_IBSU_FLAT_FOUR_FINGERS)
'*                        Added enumeration value to IBSU_PropertyId
'*                            (ENUM_IBSU_PROPERTY_RETRY_WRONG_COMMUNICATION)
'*                        Added callback function (IBSU_CallbackNotifyMessage)
'*     2013/03/20  1.6.0  Added new enumerations & structures to support IBScanMatcher integration
'*                            (IBSM_ImageFormat, IBSM_ImpressionType, IBSM_FingerPosition,
'*                             IBSM_CaptureDeviceTechID, IBSM_CaptureDeviceTypeID,
'*                             IBSM_CaptureDeviceVendorID, IBSM_ImageData)
'*     2013/07/31  1.6.8  Added new defines to support Curve (TBN240) LEDs (IBSU_LED_SCAN_CURVE_RED,
'*                             IBSU_LED_SCAN_CURVE_GREEN, IBSU_LED_SCAN_CURVE_BLUE)
'*     2013/08/03  1.6.9  Reformatted.
'*     2013/10/14  1.7.0  Added new defines to support image segmentation and bitmap header.
'*                            (IBSU_MAX_SEGMENT_COUNT, IBSU_BMP_GRAY_HEADER_LEN, IBSU_BMP_RGB24_HEADER_LEN,
'*                             IBSU_BMP_RGB32_HEADER_LEN)
'*                        Added a new structure to support image segmentation.
'*                            (IBSU_SegmentPosition)
'*                        Added new items in existing enumerations to support capture timeout,
'*                        roll-figerprint's width, draw arrow, getting the scale of the displayed
'*                        image, getting the left/top margin of the display, showing invalid finger
'*                        position area of top/left/right, and identifying the extended result image
'*                        available callback.
'*                        Added new enumeration to make overlay object on client window.
'*                            (IBSU_OverlayShapePattern)
'*                        Added new callback function (IBSU_CallbackResultImageEx)
'*     2014/02/25  1.7.1  Added enumeration value to IBSU_PropertyId
'*                            (ENUM_IBSU_PROPERTY_ROLL_MODE,ENUM_IBSU_PROPERTY_ROLL_LEVEL )
'*     2014/07/23  1.8.0  Reformatted.
'*                        Added enumeration value to IBSU_ClientWindowPropertyId
'*                            (ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE_WIDTH )
'*     2015/03/04  1.8.3  Reformatted to support UNICODE for WinCE
'*                        Added enumeration value to IBSU_PropertyId
'*                            (ENUM_IBSU_PROPERTY_CAPTURE_AREA_THRESHOLD, ENUM_IBSU_PROPERTY_ENABLE_DECIMATION )
'*                        Added enumeration value to IBSU_ClientWindowPropertyId
'*                            (ENUM_IBSU_WINDOW_PROPERTY_SCALE_FACTOR_EX )
'*                        Added reserved feature on ENUM_IBSU_PROPERTY_RESERVED_1
'*     2015/04/07  1.8.4  Added enumeration value to IBSU_PropertyId
'*                            (ENUM_IBSU_PROPERTY_ENABLE_CAPTURE_ON_RELEASE, ENUM_IBSU_PROPERTY_DEVICE_INDEX,
'*                             ENUM_IBSU_PROPERTY_DEVICE_ID )
'*                        Added reserved enumeration value to IBSU_PropertyId
'*                            (ENUM_IBSU_PROPERTY_RESERVED_IMAGE_PROCESS_THRESHOLD )
'*     2015/08/05  1.8.5  Added enumeration value to IBSU_ClientWindowPropertyId
'*                            (ENUM_IBSU_WINDOW_PROPERTY_KEEP_REDRAW_LAST_IMAGE )
'*                        Added new enumerations to support Image combine
'*                            (IBSU_CombineImageWhichHand )
'*     2015/12/11  1.9.0  Added additional LED definitions for Kojak.
'*                        Added enumeration value to IBSU_ImageType
'*                            (ENUM_IBSU_FLAT_THREE_FINGERS )
'*                        Added enumeration value to IBSU_PropertyId
'*                            (ENUM_IBSU_PROPERTY_SUPER_DRY_MODE )
'*                        Added enumeration value to IBSU_LedType
'*                            (ENUM_IBSU_LED_TYPE_FSCAN )
'*                        Added enumeration for beeper
'*                            (IBSU_BeeperType, IBSU_BeepPattern )
'*                        Added enumeration value to IBSM_CaptureDeviceTypeID
'*                            (IBSM_CAPTURE_DEVICE_TYPE_ID_KOJAK )
'*                        Added new callback function (IBSU_CallbackKeyButtons)
'*     2016/01/21  1.9.2  Added enumeration value to IBSU_PropertyId
'*                            (ENUM_IBSU_PROPERTY_MIN_CAPTURE_TIME_IN_SUPER_DRY_MODE )
'*     2016/04/20  1.9.3  Added enumeration value to IBSU_PropertyId
'*                            (ENUM_IBSU_PROPERTY_ROLLED_IMAGE_WIDTH, ENUM_IBSU_PROPERTY_ROLLED_IMAGE_HEIGHT )
'*     2016/09/22  1.9.4  Added enumeration value to IBSU_PropertyId
'*                            (ENUM_IBSU_PROPERTY_NO_PREVIEW_IMAGE, ENUM_IBSU_PROPERTY_ROLL_IMAGE_OVERRIDE )
'*     2017/04/27  1.9.7  Added enumeration value to IBSU_PropertyId
'*                            (ENUM_IBSU_PROPERTY_WARNING_MESSAGE_INVALID_AREA, ENUM_IBSU_PROPERTY_ENABLE_WET_FINGER_DETECT,
'*                             ENUM_IBSU_PROPERTY_WET_FINGER_DETECT_LEVEL) 
'*                        Added enumeration value to IBSU_FingerQualityState
'*                            (ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM )
'*     2017/06/16  1.9.8  Added enumeration value to IBSU_PropertyId
'*                            (ENUM_IBSU_PROPERTY_WET_FINGER_DETECT_LEVEL_THRESHOLD, 
'*                             ENUM_IBSU_PROPERTY_START_POSITION_OF_ROLLING_AREA,
'*	                           ENUM_IBSU_PROPERTY_START_ROLL_WITHOUT_LOCK,
'*                             ENUM_IBSU_PROPERTY_ENABLE_TOF,
'*                             ENUM_IBSU_PROPERTY_RESERVED_ENABLE_TOF_FOR_ROLL, 
'*                             ENUM_IBSU_PROPERTY_RESERVED_CAPTURE_BRIGHTNESS_THRESHOLD_FOR_FLAT,
'*                             ENUM_IBSU_PROPERTY_RESERVED_CAPTURE_BRIGHTNESS_THRESHOLD_FOR_ROLL,
'*                             ENUM_IBSU_PROPERTY_RESERVED_ENHANCED_RESULT_IMAGE)
'*     2017/12/05  1.10.0  Added enumeration value to IBSU_PropertyId
'*                            (ENUM_IBSU_PROPERTY_ENABLE_ENCRYPTION) 
'*     2018/04/27  2.0.1   Deprecated enumeration
'*                            (IBSM_FingerPosition)
'****************************************************************************************************
'*/

Option Explicit


Public Type IBSU_RECT
    Left As Long
    Top As Long
    Right As Long
    Bottom As Long
End Type
    
'/*
'****************************************************************************************************
'* GLOBAL DEFINES
'****************************************************************************************************
'*/
'/* Required length of buffer for return string parameters. */
Public Const IBSU_MAX_STR_LEN = 128
    '/* Minimum value of contrast. */
Public Const IBSU_MIN_CONTRAST_VALUE = 0
    '/* Maximum value of contrast. */
Public Const IBSU_MAX_CONTRAST_VALUE = 34

    '/* Required buffer length for segment parameters. */
Public Const IBSU_MAX_SEGMENT_COUNT = 4
    '/* Required buffer length for the 8bit bitmap header. */
Public Const IBSU_BMP_GRAY_HEADER_LEN = 1078
    '/* Required buffer length for the 24bit bitmap header. */
Public Const IBSU_BMP_RGB24_HEADER_LEN = 54
    '/* Required buffer length for the 32bit bitmap header. */
Public Const IBSU_BMP_RGB32_HEADER_LEN = 54

    '/* Capture options.  For more information, see IBSU_BeginCaptureImage(). */
Public Const IBSU_OPTION_AUTO_CONTRAST = 1
Public Const IBSU_OPTION_AUTO_CAPTURE = 2
Public Const IBSU_OPTION_IGNORE_FINGER_COUNT = 4

'/* LED bit defines. */
'/* All LEDs off. */
Public Const IBSU_LED_NONE As Long = &H0&
'/* All LEDs on. */
Public Const IBSU_LED_ALL As Long = &HFFFFFFFF
'/* Reserved for vendor (user cannot control it). */
Public Const IBSU_LED_INIT_BLUE As Long = &H1&
'/* OK key - green blink phase 1. */
Public Const IBSU_LED_SCAN_GREEN As Long = &H2&
'/* Red LED for Curve (TBN240). */
Public Const IBSU_LED_SCAN_CURVE_RED As Long = &H10&
'/* Green LED for Curve (TBN240). */
Public Const IBSU_LED_SCAN_CURVE_GREEN As Long = &H20&
'/* Blue LED for Curve (TBN240). */
Public Const IBSU_LED_SCAN_CURVE_BLUE As Long = &H40&

'/* Specific LED bit defines with LED type = ENUM_IBSU_LED_TYPE_FSCAN (e.g four finger scanner, Kojak). */
' All Green LEDs blink.
Public Const IBSU_LED_F_BLINK_GREEN As Long = &H10000000
' All Red LEDs blink.
Public Const IBSU_LED_F_BLINK_RED As Long = &H20000000
' Green LED for left little finger.
Public Const IBSU_LED_F_LEFT_LITTLE_GREEN As Long = &H1000000
' Red LED for left little finger.
Public Const IBSU_LED_F_LEFT_LITTLE_RED As Long = &H2000000
' Green LED for left ring finger.
Public Const IBSU_LED_F_LEFT_RING_GREEN As Long = &H4000000
' Red LED for left ring finger.
Public Const IBSU_LED_F_LEFT_RING_RED As Long = &H8000000
' Green LED for left middle finger.
Public Const IBSU_LED_F_LEFT_MIDDLE_GREEN As Long = &H100000
' Red LED for left middle finger.
Public Const IBSU_LED_F_LEFT_MIDDLE_RED As Long = &H200000
' Green LED for left index finger.
Public Const IBSU_LED_F_LEFT_INDEX_GREEN As Long = &H400000
' Red LED for left index finger.
Public Const IBSU_LED_F_LEFT_INDEX_RED As Long = &H800000
' Green LED for left thumb finger.
Public Const IBSU_LED_F_LEFT_THUMB_GREEN As Long = &H10000
' Red LED for left thumb finger.
Public Const IBSU_LED_F_LEFT_THUMB_RED As Long = &H20000
' Green LED for right thumb finger.
Public Const IBSU_LED_F_RIGHT_THUMB_GREEN As Long = &H40000
' Red LED for right thumb finger.
Public Const IBSU_LED_F_RIGHT_THUMB_RED As Long = &H80000
' Green LED for right index finger.
Public Const IBSU_LED_F_RIGHT_INDEX_GREEN As Long = &H1000
' Red LED for right index finger.
Public Const IBSU_LED_F_RIGHT_INDEX_RED As Long = &H2000
' Green LED for right middle finger.
Public Const IBSU_LED_F_RIGHT_MIDDLE_GREEN As Long = &H4000
' Red LED for right middle finger.
'/* 0x00008000 cannot be used at VB6 (Negative value) */
'Public Const IBSU_LED_F_RIGHT_MIDDLE_RED As Long = &H8000
Public Const IBSU_LED_F_RIGHT_MIDDLE_RED As Long = &H40000000
' Green LED for right ring finger.
Public Const IBSU_LED_F_RIGHT_RING_GREEN As Long = &H100
' Red LED for right ring finger.
Public Const IBSU_LED_F_RIGHT_RING_RED As Long = &H200
' Green LED for right little finger.
Public Const IBSU_LED_F_RIGHT_LITTLE_GREEN As Long = &H400
' Red LED for right little finger.
Public Const IBSU_LED_F_RIGHT_LITTLE_RED As Long = &H800
' Green LED for right ring finger.
Public Const IBSU_LED_F_PROGRESS_ROLL As Long = &H10
' Red LED for right ring finger.
Public Const IBSU_LED_F_PROGRESS_LEFT_HAND As Long = &H20
' Green LED for right little finger.
Public Const IBSU_LED_F_PROGRESS_TWO_THUMB As Long = &H40
' Red LED for right little finger.
Public Const IBSU_LED_F_PROGRESS_RIGHT_HAND As Long = &H80

' Bit-pattern of finger index for IBSU_RemoveFingerImage, IBSU_AddFingerImage, IBSU_IsFingerDuplicated and IBSU_IsValidFingerGeometry
Public Const IBSU_FINGER_NONE				     As Long = &H0
Public Const IBSU_FINGER_LEFT_LITTLE             As Long = &H1
Public Const IBSU_FINGER_LEFT_RING               As Long = &H2
Public Const IBSU_FINGER_LEFT_MIDDLE             As Long = &H4
Public Const IBSU_FINGER_LEFT_INDEX              As Long = &H8
Public Const IBSU_FINGER_LEFT_THUMB              As Long = &H10
Public Const IBSU_FINGER_RIGHT_THUMB             As Long = &H20
Public Const IBSU_FINGER_RIGHT_INDEX             As Long = &H40
Public Const IBSU_FINGER_RIGHT_MIDDLE            As Long = &H80
Public Const IBSU_FINGER_RIGHT_RING              As Long = &H100
Public Const IBSU_FINGER_RIGHT_LITTLE            As Long = &H200
Public Const IBSU_FINGER_LEFT_HAND               As Long = (IBSU_FINGER_LEFT_INDEX  Or IBSU_FINGER_LEFT_MIDDLE  Or IBSU_FINGER_LEFT_RING  Or IBSU_FINGER_LEFT_LITTLE)
Public Const IBSU_FINGER_RIGHT_HAND              As Long = (IBSU_FINGER_RIGHT_INDEX Or IBSU_FINGER_RIGHT_MIDDLE Or IBSU_FINGER_RIGHT_RING Or IBSU_FINGER_RIGHT_LITTLE)
Public Const IBSU_FINGER_BOTH_THUMBS			 As Long = (IBSU_FINGER_RIGHT_THUMB Or IBSU_FINGER_LEFT_THUMB)
Public Const IBSU_FINGER_ALL                     As Long = (IBSU_FINGER_LEFT_HAND   Or IBSU_FINGER_RIGHT_HAND   Or IBSU_FINGER_BOTH_THUMBS)
Public Const IBSU_FINGER_LEFT_LITTLE_RING		 As Long = (IBSU_FINGER_LEFT_LITTLE Or IBSU_FINGER_LEFT_RING)
Public Const IBSU_FINGER_LEFT_MIDDLE_INDEX		 As Long = (IBSU_FINGER_LEFT_MIDDLE Or IBSU_FINGER_LEFT_INDEX)
Public Const IBSU_FINGER_RIGHT_INDEX_MIDDLE		 As Long = (IBSU_FINGER_RIGHT_INDEX Or IBSU_FINGER_RIGHT_MIDDLE)
Public Const IBSU_FINGER_RIGHT_RING_LITTLE		 As Long = (IBSU_FINGER_RIGHT_RING  Or IBSU_FINGER_RIGHT_LITTLE)

'/*
'****************************************************************************************************
'* GLOBAL TYPES
'****************************************************************************************************
'*/

'/*
'****************************************************************************************************
'* IBSU_SdkVersion
'*
'* DESCRIPTION:
'*     Container to hold SDK version.
'****************************************************************************************************
'*/
Public Type IBSU_SdkVersion
    Product(0 To IBSU_MAX_STR_LEN - 1) As Byte                  '///< Product version string
    File(0 To IBSU_MAX_STR_LEN - 1) As Byte                     '///< File version string
End Type

'/*
'****************************************************************************************************
'* IBSU_DeviceDesc
'*
'* DESCRIPTION:
'*     Basic device description.
'****************************************************************************************************
'*/
Public Type IBSU_DeviceDesc
    serialNumber(0 To IBSU_MAX_STR_LEN - 1) As Byte               '///< Device serial number
    productName(0 To IBSU_MAX_STR_LEN - 1) As Byte                '///< Device product name
    interfaceType(0 To IBSU_MAX_STR_LEN - 1) As Byte              '///< Device interface type (USB, Firewire)
    fwVersion(0 To IBSU_MAX_STR_LEN - 1) As Byte                  '///< Device firmware version
    devRevision(0 To IBSU_MAX_STR_LEN - 1) As Byte                '///< Device revision
    handle As Long                                       '///< Return device handle
    IsHandleOpened As Long                               '///< Check if device handle is opened
End Type

'/*
'****************************************************************************************************
'* IBSU_SegmentPosition
'*
'* DESCRIPTION:
'*     Container to hold segment information.
'****************************************************************************************************
'*/
Public Type IBSU_SegmentPosition
    x1 As Integer                                           '///< X coordinate of starting point of the finger segment
    y1 As Integer                                           '///< Y coordinate of starting point of the finger segment
    x2 As Integer                                           '///< X coordinate of 1st corner of the finger segment
    y2 As Integer                                           '///< Y coordinate of 1st corner of the finger segment
    x3 As Integer                                           '///< X coordinate of 2nd corner of the finger segment
    y3 As Integer                                           '///< Y coordinate of 2nd corner of the finger segment
    x4 As Integer                                           '///< X coordinate of 3rd corner of the finger segment
    y4 As Integer                                           '///< Y coordinate of 3rd corner of the finger segment
End Type

'/*
'****************************************************************************************************
'* IBSU_ImageType
'*
'* DESCRIPTION:
'*     Enumeration of image types.  The image type is passed as a parameter to API functions
'*     IBSU_BeginCaptureImage() and IBSU_IsCaptureAvailable().
'****************************************************************************************************
'*/
Public Enum IBSU_ImageType
    '/* Unspecified type. */
    ENUM_IBSU_TYPE_NONE

    '/* One-finger rolled fingerprint. */
    ENUM_IBSU_ROLL_SINGLE_FINGER

    '/* One-finger flat fingerprint. */
    ENUM_IBSU_FLAT_SINGLE_FINGER

    '/* Two-finger flat fingerprint. */
    ENUM_IBSU_FLAT_TWO_FINGERS

    '/* Four-finger flat fingerprint. */
    ENUM_IBSU_FLAT_FOUR_FINGERS

    '/* Three-finger flat fingerprint. */
    ENUM_IBSU_FLAT_THREE_FINGERS
End Enum

'/*
'****************************************************************************************************
'* IBSU_ImageResolution
'*
'* DESCRIPTION:
'*     Enumeration of image resolutions.  The image resolution is passed as a parameter to API
'*     functions IBSU_BeginCaptureImage() and IBSU_IsCaptureAvailable().
'****************************************************************************************************
'*/
Public Enum IBSU_ImageResolution
    '/* 500 pixels/inch. */
    ENUM_IBSU_IMAGE_RESOLUTION_500 = 500

    '/* 1000 pixels/inch. */
    ENUM_IBSU_IMAGE_RESOLUTION_1000 = 1000
End Enum

'/*
'****************************************************************************************************
'* IBSU_PropertyId
'*
'* DESCRIPTION:
'*     Enumeration of property IDs.  Properties can be gotten with IBSU_GetProperty(); some properties
'*     can also be set with IBSU_SetProperty().
'****************************************************************************************************
'*/
Public Enum IBSU_PropertyId
    '/* Product name string (e.g., "Watson").  [Get only.] */
    ENUM_IBSU_PROPERTY_PRODUCT_ID

    '/* Serial number string.  [Get only.] */
    ENUM_IBSU_PROPERTY_SERIAL_NUMBER

    '/* Device manufacturer identifier.  [Get only.] */
    ENUM_IBSU_PROPERTY_VENDOR_ID

    '/* IBIA vendor ID.  [Get only.] */
    ENUM_IBSU_PROPERTY_IBIA_VENDOR_ID

    '/* IBIA version information.  [Get only.] */
    ENUM_IBSU_PROPERTY_IBIA_VERSION

    '/* IBIA device ID.  [Get only.] */
    ENUM_IBSU_PROPERTY_IBIA_DEVICE_ID

    '/* Firmware version string.  [Get only.] */
    ENUM_IBSU_PROPERTY_FIRMWARE

    '/* Device revision string.  [Get only.] */
    ENUM_IBSU_PROPERTY_REVISION

    '/* Production date string.  [Get only.] */
    ENUM_IBSU_PROPERTY_PRODUCTION_DATE

    '/* Last service date string.  [Get only.] */
    ENUM_IBSU_PROPERTY_SERVICE_DATE

    '/* Image width value.  [Get only.] */
    ENUM_IBSU_PROPERTY_IMAGE_WIDTH

    '/* Image height value.  [Get only.] */
    ENUM_IBSU_PROPERTY_IMAGE_HEIGHT

    '/* Time to acquire fingerprint in the auto capture regardless of number of fingers.  The option
    ' * IBSU_OPTION_AUTO_CAPTURE must be used.  The valid range is between 2000- and 4000-ms,
    ' * inclusive, with the default of 4000-ms. [Get and set.] */
    ENUM_IBSU_PROPERTY_IGNORE_FINGER_TIME

    '/* Auto contrast level value. [Get and set.] */
    ENUM_IBSU_PROPERTY_RECOMMENDED_LEVEL

    '/* Polling time for IBSU_BGetImage(). [Get only.] */
    ENUM_IBSU_PROPERTY_POLLINGTIME_TO_BGETIMAGE

    '/* Enable power save mode (TRUE to enable or FALSE to disable). [Get and set.] */
    ENUM_IBSU_PROPERTY_ENABLE_POWER_SAVE_MODE

    '/* Retry count for communication error.  The valid range is between 0 and 120, inclusive, with
    ' * the default of 6. [Get and set.] */
    ENUM_IBSU_PROPERTY_RETRY_WRONG_COMMUNICATION

    '/* The maximum wait time for image capture, in seconds.  Must use IBSU_CallbackResultImageEx instead
    ' * of IBSU_CallbackResultImage.  If -1, the timeout is infinite.  Otherwise, the valid
    ' * range is between 10- and 3600-seconds, inclusive.  The default is -1.  [Get and set.] */
    ENUM_IBSU_PROPERTY_CAPTURE_TIMEOUT

    '/* Minimum distance of rolled fingerprint, in millimeters.  The valid range is between 10-
    ' * and 30-mm, inclusive.  The default is 15-mm.  [Get and set.] */
    ENUM_IBSU_PROPERTY_ROLL_MIN_WIDTH

    '/* roll mode. The valid range is between 0 ~ 1.  The default is 1.  [Get and set.]
    ' * 0 : no use smear
    '   1 : use notice */
    ENUM_IBSU_PROPERTY_ROLL_MODE

    '/* roll level. The valid range is between 0 ~ 2.  The default is 1.  [Get and set.]
    ' * 0 : low level
    '   1 : medium level
    '   2 : high level */
    ENUM_IBSU_PROPERTY_ROLL_LEVEL

    '/* The area threshold for image capture for flat fingers and
    ' * The area threshold for beginning rolled finger.
    ' * The valid range is between 0 and 12, inclusive, with the default of 6. [Get and set.] */
    ENUM_IBSU_PROPERTY_CAPTURE_AREA_THRESHOLD

    '/* Enable decimation mode (TRUE to enable or FALSE to disable).
    ' * Some of devices (or firmware version) does not support this feature. [Get and set.]*/
    ENUM_IBSU_PROPERTY_ENABLE_DECIMATION

    '/* Enable capture on release (TRUE to enable or FALSE to disable). The default is FALSE.  [Get and set.]
    ' * TRUE  : the result callback will be called when user release the finger from the sensor.
    ' * FALSE : the result callback will be called when the quality of finger become good */
    ENUM_IBSU_PROPERTY_ENABLE_CAPTURE_ON_RELEASE

    '/* The device index. [Get only.] */
    ENUM_IBSU_PROPERTY_DEVICE_INDEX

    '/* The device ID which has same information with UsbDevice class of Android. [Get only.] */
    ENUM_IBSU_PROPERTY_DEVICE_ID
    
    '/* It can be used for dry finger
    ' * Some of devices (or firmware version) does not support this feature.
    ' * The default is FALSE. [Get and set.]
    ' * TRUE  : Enable dry mode.
    ' * FALSE : Disable dry mode */
    ENUM_IBSU_PROPERTY_SUPER_DRY_MODE

    '/* It is a minimum capture time when the dry mode is enabled with the property ENUM_IBSU_PROPERTY_SUPER_DRY_MODE
    ' * Some of devices (or firmware version) does not support this feature.
    ' * The valid range is between 600- and 3000-ms,
    ' * inclusive, with the default of 2000-ms. [Get and set.]*/
    ENUM_IBSU_PROPERTY_MIN_CAPTURE_TIME_IN_SUPER_DRY_MODE

    '/* Rolled image width value.  [Get only.] */
    ENUM_IBSU_PROPERTY_ROLLED_IMAGE_WIDTH           

    '/* Rolled image height value.  [Get only.] */
    ENUM_IBSU_PROPERTY_ROLLED_IMAGE_HEIGHT              

	'/* Enable the drawing for preview image (TRUE to enable or FALSE to disable). 
    ' * The default is TRUE.  [Get and set.] */
	ENUM_IBSU_PROPERTY_NO_PREVIEW_IMAGE

	'/* Enable to override roll image (TRUE to enable or FALSE to disable). 
    ' * The default is FALSE.  [Get and set.] */
	ENUM_IBSU_PROPERTY_ROLL_IMAGE_OVERRIDE

    '/* Enable the warning message for invalid area for result image (TRUE to enable or FALSE to disable). 
    ' * The default is FALSE.  [Get and set.] */
    ENUM_IBSU_PROPERTY_WARNING_MESSAGE_INVALID_AREA

    '/* Enable wet detect function.
    ' * The default is FALSE.  [Get and set.] */
    ENUM_IBSU_PROPERTY_ENABLE_WET_FINGER_DETECT

    '/* Change wet detect level.
    ' * The valid range is between 1 and 5. The default is 3.  [Get and set.]
    ' * 1 : Lowest level for detect wet finger : less sensitive
    ' * 5 : Highest level for detect wet finger : more sensitive */
    ENUM_IBSU_PROPERTY_WET_FINGER_DETECT_LEVEL
    
		'/* Change threshold for each wet detect level.
    ' * The valid range is between 10 and 1000. The default is "50 100 150 200 250"  [Get and set.]
    ' * 50 : Threshold of lowest level for detect wet finger
    ' * 250 : Threshold of highest level for detect wet finger */
    ENUM_IBSU_PROPERTY_WET_FINGER_DETECT_LEVEL_THRESHOLD

    '/* Control rolling area vertically.
    ' * The valid range is between 0 and 9. The default is 0.  [Get and set.]
    ' * 0 : minimum position
    ' * 9 : maximum position */
    ENUM_IBSU_PROPERTY_START_POSITION_OF_ROLLING_AREA

    '/* Enable rolling without lock.
    ' * The default is FALSE.  [Get and set.] */
    ENUM_IBSU_PROPERTY_START_ROLL_WITHOUT_LOCK

    '/* Enable TOF function.
    ' * The default is set depending on the devices.  [Get and set.] */
    ENUM_IBSU_PROPERTY_ENABLE_TOF
    
    '/* Enable Encryption for capture images
    '* The default is FALSE.  [Get and set.] */
	  ENUM_IBSU_PROPERTY_ENABLE_ENCRYPTION

    '/* Reserved for manufacturer strings. [Need a reserve code]*/
    ENUM_IBSU_PROPERTY_RESERVED_1 = 200
    ENUM_IBSU_PROPERTY_RESERVED_2
    ENUM_IBSU_PROPERTY_RESERVED_100

    '/* The previmage processing threshold. [Need a partner or reserve code]
    ' * The valid range is between 0 and 2, inclusive,
    ' * with the default of 0 on embedded processor (ARM, Android and Windows Mobile),
    ' * and with the default of 2 on PC. [Get and set.]
    ' * 0  : IMAGE_PROCESS_LOW
    ' * 1  : IMAGE_PROCESS_MEDIUM
    ' * 2  : IMAGE_PROCESS_HIGH */
    ENUM_IBSU_PROPERTY_RESERVED_IMAGE_PROCESS_THRESHOLD = 400
    
    '/* Enable TOF for roll capture
    ' * The default is FALSE.  [Get and set.] */
    ENUM_IBSU_PROPERTY_RESERVED_ENABLE_TOF_FOR_ROLL

    '/* Change brightness threshold for flat capture
    ' * The default values are depending on the scanner.  [Get and set.] */
    ENUM_IBSU_PROPERTY_RESERVED_CAPTURE_BRIGHTNESS_THRESHOLD_FOR_FLAT

    '/* Change brightness threshold for roll capture
    ' * The default values are depending on the scanner.  [Get and set.] */
    ENUM_IBSU_PROPERTY_RESERVED_CAPTURE_BRIGHTNESS_THRESHOLD_FOR_ROLL
  
    '/* Change result image to be enhanced
    ' * The default values are FALSE.  [Get and set.] */
    ENUM_IBSU_PROPERTY_RESERVED_ENHANCED_RESULT_IMAGE
End Enum

'/*
'****************************************************************************************************
'* IBSU_ClientWindowPropertyId
'*
'* DESCRIPTION:
'*     Enumeration of client window property IDs.  Properties can be gotten with IBSU_GetClientWindowProperty();
'*     some properties can also be set with IBSU_SetClientWindowProperty().
'****************************************************************************************************
'*/
Public Enum IBSU_ClientWindowPropertyId
    '/* Background color of display window.  The valid range is between 0x00000000 and 0xFFFFFFFF,
    ' * inclusive, with the default of 0x00D8E9EC (the button face color on Windows). [Get and set.] */
    ENUM_IBSU_WINDOW_PROPERTY_BK_COLOR

    '/* Indicates whether guide line should be drawn for rolling print capture (TRUE or FALSE).  The
    ' * default is TRUE.  [Get and set.] */
    ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE

    '/* Draw arrow to display invalid area (TRUE or FALSE).  The default is FALSE.  [Get and set.] */
    ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA

    '/* Get the scale of the display image on client window, as a floating point value.  */
    ENUM_IBSU_WINDOW_PROPERTY_SCALE_FACTOR

    '/* Get the left margin of the displayed image in relation to the client window, as an integer. */
    ENUM_IBSU_WINDOW_PROPERTY_LEFT_MARGIN

    '/* Get the top margin of the displayed image in relation to the client window, as an integer. */
    ENUM_IBSU_WINDOW_PROPERTY_TOP_MARGIN

    '/* Thickness of ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE The valid range is between 1 and 6 pixels,
    ' * inclusive, with the default of 2 pixels.  [Get and set.] */
    ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE_WIDTH
    
    '/* Get the extended scale of the display image on client window, as a integer value.  */
    ENUM_IBSU_WINDOW_PROPERTY_SCALE_FACTOR_EX
    
    '* Keep the last image for redrawing of the display image when calling IBSU_RedrawClientWindow.
    '(TRUE to enable or FALSE to disable).  The default is TRUE.  [Get and set.]  */
    ENUM_IBSU_WINDOW_PROPERTY_KEEP_REDRAW_LAST_IMAGE
End Enum

'/*
'****************************************************************************************************
'* IBSU_FingerCountState
'*
'* DESCRIPTION:
'*     Enumeration of finger count states.
'****************************************************************************************************
'*/
Public Enum IBSU_FingerCountState
    ENUM_IBSU_FINGER_COUNT_OK
    ENUM_IBSU_TOO_MANY_FINGERS
    ENUM_IBSU_TOO_FEW_FINGERS
    ENUM_IBSU_NON_FINGER
End Enum

'/*
'****************************************************************************************************
'* IBSU_FingerQualityState
'*
'* DESCRIPTION:
'*     Enumeration of finger quality states.
'****************************************************************************************************
'*/
Public Enum IBSU_FingerQualityState
    ENUM_IBSU_FINGER_NOT_PRESENT
    ENUM_IBSU_QUALITY_GOOD
    ENUM_IBSU_QUALITY_FAIR
    ENUM_IBSU_QUALITY_POOR
    ENUM_IBSU_QUALITY_INVALID_AREA_TOP                      '///< Finger position is not valid area of top side
    ENUM_IBSU_QUALITY_INVALID_AREA_LEFT                 '///< Finger position is not valid area of left side
    ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT                    '///< Finger position is not valid area of right side
    ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM                    '///< Finger position is not valid area of bottom side
End Enum

'/*
'****************************************************************************************************
'* IBSU_LEOperationMode
'*
'* DESCRIPTION:
'*     Enumeration of light emitting (LE) file operation modes.
'****************************************************************************************************
'*/
Public Enum IBSU_LEOperationMode
    ENUM_IBSU_LE_OPERATION_AUTO
    ENUM_IBSU_LE_OPERATION_ON
    ENUM_IBSU_LE_OPERATION_OFF
End Enum

'/*
'****************************************************************************************************
'* IBSU_PlatenState
'*
'* DESCRIPTION:
'*     Enumeration of platen states.
'****************************************************************************************************
'*/
Public Enum IBSU_PlatenState
    ENUM_IBSU_PLATEN_CLEARD
    ENUM_IBSU_PLATEN_HAS_FINGERS
End Enum

'/*
'****************************************************************************************************
'* IBSU_Events
'*
'* DESCRIPTION:
'*     Enumeration of events that can trigger callbacks.
'****************************************************************************************************
'*/
Public Enum IBSU_Events
    '/* Callback when device count changes. */
    ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT

    '/* Callback when communication with a device is interrupted. */
    ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK

    '/* Callback when a new preview image is available from a device. */
    ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE

    '/* Callback for rolled print acquisition when rolling should begin. */
    ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION

    '/* Callback for rolled print acquisition when rolling completes. */
    ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION

    '/* Callback when result image is available for a capture (deprecated). */
    ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE

    '/* Callback when a finger quality changes. */
    ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY

    '/* Callback when the finger count changes. */
    ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT

    '/* Callback when initialization progress changes for a device. */
    ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS

    '/* Callback when the platen was not clear when capture started or has since become clear. */
    ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE

    '/* Callback when asynchronous device initialization completes. */
    ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE

    '/* Callback when a warning message is generated. */
    ENUM_IBSU_OPTIONAL_EVENT_NOTIFY_MESSAGE

    '/* Callback when result image is available for a capture (with extended information). */
    ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX

    '/* Callback when key buttons are pressed */
    ENUM_IBSU_ESSENTIAL_EVENT_KEYBUTTON
End Enum

'/*
'****************************************************************************************************
'* IBSU_LedType
'*
'* DESCRIPTION:
'*     Enumeration of LED types.
'****************************************************************************************************
'*/
Public Enum IBSU_LedType
    '/* No LED field. */
    ENUM_IBSU_LED_TYPE_NONE

    '/* Two-scanner type (e.g., Watson). */
    ENUM_IBSU_LED_TYPE_TSCAN

    '/* four-scanner type (e.g., Kojak). */
    ENUM_IBSU_LED_TYPE_FSCAN
End Enum

'/*
'****************************************************************************************************
'* IBSU_RollingState
'*
'* DESCRIPTION:
'*     Enumeration of rolling print acquisition states.
'****************************************************************************************************
'*/
Public Enum IBSU_RollingState
    ENUM_IBSU_ROLLING_NOT_PRESENT
    ENUM_IBSU_ROLLING_TAKE_ACQUISITION
    ENUM_IBSU_ROLLING_COMPLETE_ACQUISITION
    ENUM_IBSU_ROLLING_RESULT_IMAGE
End Enum

'/*
'****************************************************************************************************
'* IBSU_OverlayShapePattern
'*
'* DESCRIPTION:
'*     Enumeration of the shape pattern to use for the overlay on client window.
'****************************************************************************************************
'*/
Public Enum IBSU_OverlayShapePattern
    ENUM_IBSU_OVERLAY_SHAPE_RECTANGLE
    ENUM_IBSU_OVERLAY_SHAPE_ELLIPSE
    ENUM_IBSU_OVERLAY_SHAPE_CROSS
    ENUM_IBSU_OVERLAY_SHAPE_ARROW
End Enum

'/*
'****************************************************************************************************
'* IBSU_CombineImageWhichHand
'*
'* DESCRIPTION:
'*     Enumeration of hand to use for combining two images into one.
'****************************************************************************************************
'*/
Public Enum IBSU_CombineImageWhichHand
    ENUM_IBSU_COMBINE_IMAGE_LEFT_HAND
    ENUM_IBSU_COMBINE_IMAGE_RIGHT_HAND
End Enum
'/*
'****************************************************************************************************
'* IBSU_BeeperType
'*
'* DESCRIPTION:
'*     Enumeration of Beeper types.
'****************************************************************************************************
'*/
Public Enum IBSU_BeeperType
    ' No Beeper field.
    ENUM_IBSU_BEEPER_TYPE_NONE

    ' Monotone type.
    ENUM_IBSU_BEEPER_TYPE_MONOTONE
End Enum

'/*
'****************************************************************************************************
'* IBSU_BeepPattern
'*
'* DESCRIPTION:
'*     Enumeration of the beep pattern.
'****************************************************************************************************
'*/
Public Enum IBSU_BeepPattern
    ENUM_IBSU_BEEP_PATTERN_GENERIC
    ENUM_IBSU_BEEP_PATTERN_REPEAT
End Enum

'/*
'****************************************************************************************************
'* IBSM_ImageFormat
'*
'* DESCRIPTION:
'*     Enumeration of image formats to support IBScanMatcher integration.
'****************************************************************************************************
'*/
Public Enum IBSM_ImageFormat
    IBSM_IMG_FORMAT_NO_BIT_PACKING = 0
    IBSM_IMG_FORMAT_BIT_PACKED
    IBSM_IMG_FORMAT_WSQ
    IBSM_IMG_FORMAT_JPEG_LOSSY
    IBSM_IMG_FORMAT_JPEG2000_LOSSY
    IBSM_IMG_FORMAT_JPEG2000_LOSSLESS
    IBSM_IMG_FORMAT_PNG
    IBSM_IMG_FORMAT_UNKNOWN
End Enum

'/*
'****************************************************************************************************
'* IBSM_ImpressionType
'*
'* DESCRIPTION:
'*     Enumeration of image impression types to support IBScanMatcher integration.
'****************************************************************************************************
'*/
Public Enum IBSM_ImpressionType
    IBSM_IMPRESSION_TYPE_LIVE_SCAN_PLAIN = 0
    IBSM_IMPRESSION_TYPE_LIVE_SCAN_ROLLED
    IBSM_IMPRESSION_TYPE_NONLIVE_SCAN_PLAIN
    IBSM_IMPRESSION_TYPE_NONLIVE_SCAN_ROLLED
    IBSM_IMPRESSION_TYPE_LATENT_IMPRESSION
    IBSM_IMPRESSION_TYPE_LATENT_TRACING
    IBSM_IMPRESSION_TYPE_LATENT_PHOTO
    IBSM_IMPRESSION_TYPE_LATENT_LIFT
    IBSM_IMPRESSION_TYPE_LIVE_SCAN_SWIPE
    IBSM_IMPRESSION_TYPE_LIVE_SCAN_VERTICAL_ROLL
    IBSM_IMPRESSION_TYPE_LIVE_SCAN_PALM
    IBSM_IMPRESSION_TYPE_NONLIVE_SCAN_PALM
    IBSM_IMPRESSION_TYPE_LATENT_PALM_IMPRESSION
    IBSM_IMPRESSION_TYPE_LATENT_PALM_TRACING
    IBSM_IMPRESSION_TYPE_LATENT_PALM_PHOTO
    IBSM_IMPRESSION_TYPE_LATENT_PALM_LIFT
    IBSM_IMPRESSION_TYPE_LIVE_SCAN_OPTICAL_CONTRCTLESS_PLAIN = 24
    IBSM_IMPRESSION_TYPE_OTHER = 28
    IBSM_IMPRESSION_TYPE_UNKNOWN = 29
End Enum

'/*
'****************************************************************************************************
'* IBSM_FingerPosition (deprecated)
'*
'* DESCRIPTION:
'*     Enumeration of finger positions to support IBScanMatcher integration.
'****************************************************************************************************
'*/
Public Enum IBSM_FingerPosition
    IBSM_FINGER_POSITION_UNKNOWN = 0
    IBSM_FINGER_POSITION_RIGHT_THUMB
    IBSM_FINGER_POSITION_RIGHT_INDEX_FINGER
    IBSM_FINGER_POSITION_RIGHT_MIDDLE_FINGER
    IBSM_FINGER_POSITION_RIGHT_RING_FINGER
    IBSM_FINGER_POSITION_RIGHT_LITTLE_FINGER
    IBSM_FINGER_POSITION_LEFT_THUMB
    IBSM_FINGER_POSITION_LEFT_INDEX_FINGER
    IBSM_FINGER_POSITION_LEFT_MIDDLE_FINGER
    IBSM_FINGER_POSITION_LEFT_RING_FINGER
    IBSM_FINGER_POSITION_LEFT_LITTLE_FINGER
    IBSM_FINGER_POSITION_PLAIN_RIGHT_FOUR_FINGERS = 13
    IBSM_FINGER_POSITION_PLAIN_LEFT_FOUR_FINGERS
    IBSM_FINGER_POSITION_PLAIN_THUMBS
    IBSM_FINGER_POSITION_UNKNOWN_PALM = 20
    IBSM_FINGER_POSITION_RIGHT_FULL_PALM
    IBSM_FINGER_POSITION_RIGHT_WRITERS_PALM
    IBSM_FINGER_POSITION_LEFT_FULL_PALM
    IBSM_FINGER_POSITION_LEFT_WRITERS_PALM
    IBSM_FINGER_POSITION_RIGHT_LOWER_PALM
    IBSM_FINGER_POSITION_RIGHT_UPPER_PALM
    IBSM_FINGER_POSITION_LEFT_LOWER_PALM
    IBSM_FINGER_POSITION_LEFT_UPPER_PALM
    IBSM_FINGER_POSITION_RIGHT_OTHER
    IBSM_FINGER_POSITION_LEFT_OTHER
    IBSM_FINGER_POSITION_RIGHT_INTERDIGITAL
    IBSM_FINGER_POSITION_RIGHT_THENAR
    IBSM_FINGER_POSITION_RIGHT_HYPOTHENAR
    IBSM_FINGER_POSITION_LEFT_INTERDIGITAL
    IBSM_FINGER_POSITION_LEFT_THENAR
    IBSM_FINGER_POSITION_LEFT_HYPOTHENAR
    IBSM_FINGER_POSITION_RIGHT_INDEX_AND_MIDDLE = 40
    IBSM_FINGER_POSITION_RIGHT_MIDDLE_AND_RING
    IBSM_FINGER_POSITION_RIGHT_RING_AND_LITTLE
    IBSM_FINGER_POSITION_LEFT_INDEX_AND_MIDDLE
    IBSM_FINGER_POSITION_LEFT_MIDDLE_AND_RING
    IBSM_FINGER_POSITION_LEFT_RING_AND_LITTLE
    IBSM_FINGER_POSITION_RIGHT_INDEX_AND_LEFT_INDEX
    IBSM_FINGER_POSITION_RIGHT_INDEX_AND_MIDDLE_AND_RING
    IBSM_FINGER_POSITION_RIGHT_MIDDLE_AND_RING_AND_LITTLE
    IBSM_FINGER_POSITION_LEFT_INDEX_AND_MIDDLE_AND_RING
    IBSM_FINGER_POSITION_LEFT_MIDDLE_AND_RING_AND_LITTLE
End Enum

'/*
'****************************************************************************************************
'* IBSM_CaptureDeviceTechID
'*
'* DESCRIPTION:
'*     Enumeration of capture device technology IDs to support IBScanMatcher integration.
'****************************************************************************************************
'*/
Public Enum IBSM_CaptureDeviceTechID
    IBSM_CAPTURE_DEVICE_UNKNOWN_OR_UNSPECIFIED = 0
    IBSM_CAPTURE_DEVICE_WHITE_LIGHT_OPTICAL_TIR
    IBSM_CAPTURE_DEVICE_WHITE_LIGHT_OPTICAL_DIRECT_VIEW_ON_PLATEN
    IBSM_CAPTURE_DEVICE_WHITE_LIGHT_OPTICAL_TOUCHLESS
    IBSM_CAPTURE_DEVICE_MONOCHROMATIC_VISIBLE_OPTICAL_TIR
    IBSM_CAPTURE_DEVICE_MONOCHROMATIC_VISIBLE_OPTICAL_DIRECT_VIEW_ON_PLATEN
    IBSM_CAPTURE_DEVICE_MONOCHROMATIC_VISIBLE_OPTICAL_TOUCHLESS
    IBSM_CAPTURE_DEVICE_MONOCHROMATIC_IR_OPTICAL_TIR
    IBSM_CAPTURE_DEVICE_MONOCHROMATIC_IR_OPTICAL_DIRECT_VIEW_ON_PLATEN
    IBSM_CAPTURE_DEVICE_MONOCHROMATIC_IR_OPTICAL_TOUCHLESS
    IBSM_CAPTURE_DEVICE_MULTISPECTRAL_OPTICAL_TIR
    IBSM_CAPTURE_DEVICE_MULTISPECTRAL_OPTICAL_DIRECT_VIEW_ON_PLATEN
    IBSM_CAPTURE_DEVICE_MULTISPECTRAL_OPTICAL_TOUCHLESS
    IBSM_CAPTURE_DEVICE_ELECTRO_LUMINESCENT
    IBSM_CAPTURE_DEVICE_SEMICONDUCTOR_CAPACITIVE
    IBSM_CAPTURE_DEVICE_SEMICONDUCTOR_RF
    IBSM_CAPTURE_DEVICE_SEMICONDUCTOR_THEMAL
    IBSM_CAPTURE_DEVICE_PRESSURE_SENSITIVE
    IBSM_CAPTURE_DEVICE_ULTRASOUND
    IBSM_CAPTURE_DEVICE_MECHANICAL
    IBSM_CAPTURE_DEVICE_GLASS_FIBER
End Enum

'/*
'****************************************************************************************************
'* IBSM_CaptureDeviceTypeID
'*
'* DESCRIPTION:
'*     Enumeration of capture device type IDs to support IBScanMatcher integration.
'****************************************************************************************************
'*/
Public Enum IBSM_CaptureDeviceTypeID
    IBSM_CAPTURE_DEVICE_TYPE_ID_UNKNOWN = &H0
    IBSM_CAPTURE_DEVICE_TYPE_ID_CURVE = &H1004
    IBSM_CAPTURE_DEVICE_TYPE_ID_WATSON = &H1005
    IBSM_CAPTURE_DEVICE_TYPE_ID_SHERLOCK = &H1010
    IBSM_CAPTURE_DEVICE_TYPE_ID_WATSON_MINI = &H1020
    IBSM_CAPTURE_DEVICE_TYPE_ID_COLUMBO = &H1100
    IBSM_CAPTURE_DEVICE_TYPE_ID_HOLMES = &H1200
    IBSM_CAPTURE_DEVICE_TYPE_ID_KOJAK = &H1300
    IBSM_CAPTURE_DEVICE_TYPE_ID_FIVE0 = &H1500
End Enum

'/*
'****************************************************************************************************
'* IBSM_CaptureDeviceVendorID
'*
'* DESCRIPTION:
'*     Enumeration of capture device vendor IDs to support IBScanMatcher integration.
'****************************************************************************************************
'*/
Public Enum IBSM_CaptureDeviceVendorID
    IBSM_CAPTURE_DEVICE_VENDOR_ID_UNREPORTED = &H0
    IBSM_CAPTURE_DEVICE_VENDOR_INTEGRATED_BIOMETRICS = &H113F
End Enum
    
'/*
'****************************************************************************************************
'* IBSM_ImageData
'*
'* DESCRIPTION:
'*     Container for image information to support IBScanMatcher integration.
'****************************************************************************************************
'*/
Public Type IBSM_ImageData
    ImageFormat As IBSM_ImageFormat
    ImpressionType As IBSM_ImpressionType
    FingerPosition As IBSM_FingerPosition
    CaptureDeviceTechID As IBSM_CaptureDeviceTechID
    CaptureDeviceVendorID As Integer
    CaptureDeviceTypeID As Integer
    ScanSamplingX As Integer
    ScanSamplingY As Integer
    ImageSamplingX As Integer
    ImageSamplingY As Integer
    ImageSizeX As Integer
    ImageSizeY As Integer
    ScaleUnit As Byte
    BitDepth As Byte
    ImageDataLength As Long
    ImageData As Long
End Type
