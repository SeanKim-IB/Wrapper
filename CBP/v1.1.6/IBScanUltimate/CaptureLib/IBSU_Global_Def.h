#ifndef IBSU_GLOBAL_H
#define IBSU_GLOBAL_H

#define __IBSCAN_ULTIMATE_SDK__
//#define __IBSCAN_SDK__

#if defined(__IBSCAN_ULTIMATE_SDK__)
#include "IBScanUltimateApi.h"
#include "ReservedApi.h"
#elif defined(__IBSCAN_SDK__)
#include "IBScanApi.h"
#include "ReservedApi.h"
#include "ReDefine.h"
#endif

// Matcher
#include "../MatcherLib/ISO_19794_define.h"

#if defined(_WINDOWS) && defined(_DEBUG)
//#define __G_DEBUG__
//#define __USE_DEBUG_WITH_BITMAP__
//#define __G_INIT_CAPTURE_TIME__
#define __G_PRINT_DAC_VALUE__
#endif

//#define __G_PERFORMANCE_DEBUG__
//#define __G_ADJUST_START_VOLTAGE_and_SHARPENING__

#define __MORPHO_TRAK__
//#define __MORPHO_TRAK_VERIFICATION_TEST__

//#define _POWER_OFF_ANALOG_TOUCH_WHEN_NO_CAPTURE_

//#define __embedded__
#ifdef WINCE
#define __USE_LIBUSBX_DRIVER__
#else
#define __USE_WINUSB_DRIVER__
#endif 

#ifdef __android__
// --------------------------------------------------------------
// Android Debug code
#include <android/log.h> 
#define LOG_DEBUG_TAG "IBSCAN" 


// End Android Debug code
// ------------------------------------------------------------#endif
#endif

#if defined(_WINDOWS)
    #define G_TRACE(...) TRACE(__VA_ARGS__)
#elif defined(__android__)
    #define G_TRACE(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_DEBUG_TAG, __VA_ARGS__)
#elif defined(__linux__)
    #define G_TRACE(...) printf(__VA_ARGS__)
#endif

#if defined(__IBSCAN_ULTIMATE_SDK__)
extern "C" int comp_nfiq(int *onfiq, float *oconf, unsigned char *idata, const int iw, const int ih, const int id, const int ippi, int *optflag);
extern "C" int wsq_encode_mem(unsigned char **odata, int *olen, const float r_bitrate, unsigned char *idata, const int w, const int h,
                                 const int d, const int ppi, char *comment_text);
extern "C" int wsq_decode_mem(unsigned char **odata, int *ow, int *oh, int *od, int *oppi, int *lossyflag, unsigned char *idata, const int ilen);
#endif

///////////////////////////////////////////////////////////////////////////////////
// Main definitions for DLL
#define PARTNER_SET_STRING2					"#partner1120!"     // Reserved Access Level 3
#define PARTNER_SET_STRING					"#partner1042!"     // Reserved Access Level 2
#define RESERVED_KEY_STRING					"ibkorea1120!"      // Reserved Access Level 1

#define __VID_IB__							(0x113f)

#define __PID_CURVE__						(0x1004)
#define __PID_WATSON__						(0x1005)
#define __PID_WATSON_REV1__					(0x1006)
#define __PID_SHERLOCK__					(0x1010)
#define __PID_SHERLOCK_REV1__				(0x1011)
#define __PID_WATSON_MINI__					(0x1020)
#define __PID_WATSON_MINI_REV1__			(0x1021)
#define __PID_COLUMBO__						(0x1100)
#define __PID_COLUMBO_REV1__				(0x1101)
#define __PID_HOLMES__						(0x1200)
#define __PID_KOJAK__						(0x1300)
#define __PID_KOJAK_REV1__					(0x1301)
#define __PID_FIVE0__						(0x1500)
#define __PID_FIVE0_REV1__					(0x1501)

#define __MAX_CALLBACK_COUNT__				64          // Set to enough number to prevent enexpected error

#define __MAX_PIPE_DATA__					4096
#define __MAX_TRACE_MESSAGE_SIZE__			(__MAX_PIPE_DATA__-128)
#define __PIPE_WAIT_TIME__					100
#define __BULK_TIMEOUT__					5000  // Milliseconds

#define MAX_DATA_BYTES						1024		// For USB Bulk communication

#define __MAX_SUPPORTED_DEVICES_COUNT__		14
static const int __G_SUPPORTED_DEVICES__[__MAX_SUPPORTED_DEVICES_COUNT__] = {
	__PID_WATSON__,
	__PID_WATSON_REV1__,
	__PID_WATSON_MINI__,
	__PID_WATSON_MINI_REV1__,
	__PID_SHERLOCK__,
	__PID_SHERLOCK_REV1__,
	__PID_COLUMBO__,
	__PID_COLUMBO_REV1__,
	__PID_CURVE__,
	__PID_HOLMES__,
	__PID_KOJAK__,
	__PID_KOJAK_REV1__,
	__PID_FIVE0__,
	__PID_FIVE0_REV1__
};

#define __MAX_IMG_WIDTH__					2048
#define __MAX_IMG_HEIGHT__					2048
#define __MAX_QUALITY_COUNT__               4
///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
// For CURVE
#define INIT_PREV   100000000
#define LESS_THAN1	20
#define LESS_THAN2	100
#define LESS_THAN3	180
#define LESS_THAN4	255

#define	CUT_NOISE			55
#define SIDE_W				20
#define UPPER_H				72

#define __DEFAULT_CURVE_CONTRAST_VALUE__		34		//255
#define __AUTO_CURVE_CONTRAST_MAX_VALUE__		34
#define __DEFAULT_CURVE_INTEGRATION_VALUE__		6
#define __AUTO_CURVE_INTEGRATION_MAX_VALUE__	10
#define __CURVE_SETI_MAX_EXPOSURE_VALUE			(0x0880)   //08A0
//#define __CURVE_SETI_DEFAULT_EXPOSURE_VALUE	    1600//1791   //06FF
#define __CURVE_SETI_DEFAULT_EXPOSURE_VALUE	    (0x0480)
#define __CURVE_SETI_MAX_PACKET_SIZE__			(8*1024)
#define __CURVE_MAX_PACKET_SIZE__				(8*1024)
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
// For Watson Mini
#define _FPGA_VER_FOR_WATSON_MINI_ENCRYPTION					( (0<<24) | (1<<16) | (1<<8) | (0) )	// RSA + AES ENCRYPTION
#define _FPGA_VER_FOR_WATSON_MINI_DAC							( (0<<24) | (1<<16) | (1<<8) | (1) )	// New DAC
//#define _FPGA_VER_FOR_WATSON_MINI_DAC_DP_FILM					( (0<<24) | (1<<16) | (1<<8) | (2) )	// New DAC + DP film
//#define _FPGA_VER_FOR_WATSON_MINI_WRITE_EEPROM_FOR_VER_			( (0<<24) | (1<<16) | (1<<8) | (3) )	// write version on EEPROM not FPGA
///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
// For Sherlock ASIC
#define _ASIC_11_2_FRAME_			0x01
#define _ASIC_12_0_FRAME_			0x02
#define _ASIC_FRAMERATE_			_ASIC_11_2_FRAME_ 
#define _FPGA_VER_FOR_SHERLOCK_DOUBLE_P_						( (0<<24) | (1<<16) | (0<<8) | (5) )		// DP film
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// capture logic for KOJAK
#define MIN_GAIN			(0x0041)
#define MAX_GAIN			(0x005F)

//#define _TARGET_MIN_BRIGHTNESS_				60//30
//#define _TARGET_MAX_BRIGHTNESS_				95

#define _TARGET_MIN_BRIGHTNESS_FOR_CAPTURE2_	(45)//(55)
#define _TARGET_MAX_BRIGHTNESS_FOR_CAPTURE2_	(80)

#define _TARGET_MIN_BRIGHTNESS_FOR_CAPTURE2_ROLL	(55)//(45)
#define _TARGET_MAX_BRIGHTNESS_FOR_CAPTURE2_ROLL	(70)

#define _TARGET_MIN_BRIGHTNESS_FOR_DRY_				(60)
#define _TARGET_MAX_BRIGHTNESS_FOR_DRY_				(100)

#define _DAC_FOR_MINIMUM_					0xFF//0xFF
//#define _DAC_FOR_MAXIMUM_					0x66//(246V)	//0x03 (334V)
//#define _DAC_FOR_MAXIMUM_TOF_				0xA5
#define _DAC_FOR_MAXIMUM_					0x40//(300V)
#define _DAC_FOR_MAXIMUM_TOF_				0x66

#define _DAC_FOR_NORMAL_SINGLE_FINGER_		0xC9//0xC0
#define _DAC_FOR_NORMAL_FOUR_FINGER_		0xC9

#define _DAC_FOR_NORMAL_SINGLE_FINGER_OF_DOUBLE_P_		0xE0
#define _DAC_FOR_NORMAL_FOUR_FINGER_OF_DOUBLE_P_		0xE0

#define _DAC_FOR_NORMAL_FINGER_WITH_TOF_	0xFF

#define _DAC_FOR_DRY_ONE_FINGER_			0xA5//0x81
#define _DAC_FOR_DRY_TWO_FINGER_			0xA5//0x93
#define _DAC_FOR_DRY_FOUR_FINGER_			0xA5

#define _THRES_255_80_MEAN_CAPTURE_			100
#define _THRES_255_80_MEAN_ALGO_			100//30//15

#define _NO_BIT_BITSHIFT_FOR_FULLFRAME_ROLL_	0
#define _NO_BIT_CUTTHRES_FOR_FULLFRAME_ROLL_	5
#define _DEFAULT_EXPOSURE_FOR_FULLFRAME_ROLL_	0x0BB8
#define _NO_BIT_CMOS_NOISE_FOR_FULLFRAME_ROLL	15

#define _NO_BIT_BITSHIFT_FOR_FULLFRAME_		1
#define _NO_BIT_CUTTHRES_FOR_FULLFRAME_		15
#define _NO_BIT_GAIN_FOR_FULLFRAME_			0x005F
#define _NO_BIT_EXPOSURE_FOR_FULLFRAME_		0x0BB8
#define _NO_BIT_CMOS_NOISE_FOR_FULLFRAME_	20

#define _DEFAULT_BITSHIFT_FOR_DECI_			1//2
#define _DEFAULT_CUTTHRES_FOR_DECI_			15//40
#define _DEFAULT_GAIN_FOR_DECI_				0x005F
#define _DEFAULT_EXPOSURE_FOR_DECI_			0x1000
//#define _DEFAULT_EXPOSURE_FOR_DECI_			0x0D00
#define _DEFAULT_CMOS_NOISE_FOR_DECI_		20

#define _FPGA_VER_FOR_KOJAK_PL_WITH_DOUBLE_P_							( (1<<24) | (0<<16) | (1<<8) | (0) )	// Kojak PL + DP film
#define _FPGA_VER_FOR_KOJAK_NEW_BRD_AND_DOUBLE_P_						( (2<<24) | (0<<16) | (0<<8) | (0) )	// new board + DP film
#define _FPGA_VER_FOR_KOJAK_NEW_BRD_AND_DOUBLE_P_AND_CURRENTSENSOR_		( (2<<24) | (0<<16) | (1<<8) | (0) )	// new board + DP film + CurrentSensor

#define _CIS_WINDOWING_COLUMN_MAX		 100
#define _CIS_WINDOWING_COLUMN_MIN		-100
#define _CIS_WINDOWING_ROW_MAX			 40
#define _CIS_WINDOWING_ROW_MIN			-40

#define _PPI_CORRECTION_HORIZONTAL_MAX		 50
#define _PPI_CORRECTION_HORIZONTAL_MIN		-50
#define _PPI_CORRECTION_VERTICAL_MAX		 50
#define _PPI_CORRECTION_VERTICAL_MIN		-50
///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
// capture logic for FIVE0
#define _TARGET_MIN_BRIGHTNESS_FOR_CAPTURE2_FIVE0		(65)//(55)
#define _TARGET_MAX_BRIGHTNESS_FOR_CAPTURE2_FIVE0		(80)

#define _TARGET_MIN_BRIGHTNESS_FOR_CAPTURE2_ROLL_FIVE0	(65)//(55)
#define _TARGET_MAX_BRIGHTNESS_FOR_CAPTURE2_ROLL_FIVE0	(80)

#define _TARGET_MIN_BRIGHTNESS_FOR_DRY_FIVE0			(60)
#define _TARGET_MAX_BRIGHTNESS_FOR_DRY_FIVE0			(100)

#define _DAC_FOR_MINIMUM_FIVE0							0xFF//0xFF
#define _DAC_FOR_MAXIMUM_FIVE0							0x40//(246V)	//0x03 (334V)

#define _DAC_FOR_TOF_FIVE0_								0xFF//0xEA

#define _DAC_FOR_NORMAL_SINGLE_FINGER_FIVE0				0xAF//0xAF//0xC0
#define _DAC_FOR_NORMAL_FOUR_FINGER_FIVE0				0xAF//0xAF

#define _DAC_FOR_NORMAL_SINGLE_FINGER_FIVE0_DOUBLE_P_	0xC7//0xAF//0xC0
#define _DAC_FOR_NORMAL_FOUR_FINGER_FIVE0_DOUBLE_P_		0xC7//0xAF

#define _DAC_FOR_DRY_ONE_FINGER_FIVE0					0xA5//0x81
#define _DAC_FOR_DRY_TWO_FINGER_FIVE0					0xA5//0x93
#define _DAC_FOR_DRY_FOUR_FINGER_FIVE0					0xA5

#define _THRES_255_80_MEAN_CAPTURE_FIVE0				100
#define _THRES_255_80_MEAN_ALGO_FIVE0					100//30//15

#define _FPGA_VER_FOR_FIVE0_DOUBLE_P_					( (1<<24) | (0<<16) | (0<<8) | (0) )	// DP film
#define _FPGA_VER_FOR_FIVE0_TOF_BETA_					( (1<<24) | (0<<16) | (0<<8) | (1) )	// TOF beta
#define _FPGA_VER_FOR_FIVE0_TOF_RELEASE_				( (1<<24) | (0<<16) | (0<<8) | (2) )	// TOF release

#define _TFT_NOISE_FOR_FIVE0_							10
#define _TFT_LE_NOISE_HEIGHT_FOR_FIVE0_					0//30

// for ASIC Linearity
#define	LINEARITY_MAPPED_DEFAULT	16
#define EEPROM_PACKET_DATA_AREA_ADDRESS_START	4
#define EEPROM_HEADER_SIZE						3
#define PACKET_DATA_SIZE						4
#define EEPROM_STRUCTURE_VERSION	0x0100		// 1.0

typedef struct tag_EEPRom_Packet
{
	UCHAR	address_type;
	UCHAR	address_high;
	UCHAR	address_low;
	UCHAR	data;

} EEPRom_Packet;
///////////////////////////////////////////////////////////////////////////////////


typedef struct tagIBSU_DeviceDescA
{
    char serialNumber[IBSU_MAX_STR_LEN];               /* Device serial number. */
    char productName[IBSU_MAX_STR_LEN];                /* Device product name. */
    char interfaceType[IBSU_MAX_STR_LEN];              /* Device interface type (USB, Firewire). */
    char fwVersion[IBSU_MAX_STR_LEN];                  /* Device firmware version. */
    char devRevision[IBSU_MAX_STR_LEN];                /* Device revision. */
    int  handle;                                       /* Return device handle. */
    BOOL IsHandleOpened;                               /* Check if device handle is opened. */
#ifdef __android__
    int  devID;                                        /* Device ID. */
#endif
    int spoof;										   /* Check if device supports spoofing. */
}
IBSU_DeviceDescA;

#if defined(_WINDOWS) && defined(__USE_WINUSB_DRIVER__)
//#include "IBSCAN_WinUsbDriver_Def.h"
// Constant for {E470DBC6-040A-4C88-9CFF-C008D6B58E89}
static const GUID IBSCAN_DEVICE_INTERFACE_GUID = 
//{ 0xE470DBC6, 0x040A, 0x4C88, { 0x9C, 0xFF, 0xC0, 0x08, 0xD6, 0xB5, 0x8E, 0x89 } };
{ 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };
typedef BITMAPFILEHEADER   IB_BITMAPFILEHEADER;
typedef BITMAPINFOHEADER   IB_BITMAPINFOHEADER;
typedef RGBQUAD            IB_RGBQUAD;
typedef BITMAPINFO         IB_BITMAPINFO;
#else
#define pszLinuxFileVersion					"2.0.1.2"
#define pszLinuxProdutVersion				"2.0.1.2"

// {AE18AA60-7F6A-11d4-97DD-00010229B959}
//static GUID IBSCAN_DEVICE_INTERFACE_GUID = {0xae18aa60, 0x7f6a, 0x11d4, {0x97, 0xdd, 0x0, 0x1, 0x2, 0x29, 0xb9, 0x59} };
///////////////////////////////////////////////////////////////////////////
// Bitmap structure for Linux
#pragma pack(push, 1)
typedef struct tagIB_BITMAPFILEHEADER { 
  WORD    bfType; 
  DWORD   bfSize; 
  WORD    bfReserved1; 
  WORD    bfReserved2; 
  DWORD   bfOffBits; 
} IB_BITMAPFILEHEADER;

typedef struct tagIB_BITMAPINFOHEADER{ 
  DWORD  biSize; 
  LONG   biWidth; 
  LONG   biHeight; 
  WORD   biPlanes; 
  WORD   biBitCount; 
  DWORD  biCompression; 
  DWORD  biSizeImage; 
  LONG   biXPelsPerMeter; 
  LONG   biYPelsPerMeter; 
  DWORD  biClrUsed; 
  DWORD  biClrImportant; 
} IB_BITMAPINFOHEADER; 

typedef struct tagIB_RGBQUAD { 
  BYTE    rgbBlue; 
  BYTE    rgbGreen; 
  BYTE    rgbRed; 
  BYTE    rgbReserved; 
} IB_RGBQUAD; 

typedef struct tagIB_BITMAPINFO {
   IB_BITMAPINFOHEADER bmiHeader;
   IB_RGBQUAD bmiColors[1];
} IB_BITMAPINFO;
#pragma pack(pop)

#ifndef BI_RGB
#define BI_RGB				0L
#endif

#endif

struct ThreadParam
{
  int				deviceHandle;
  HANDLE			threadStarted;
  void*				pParentApp;
  void*				pMainCapture;
  void*				callback;
  void*				context;
  int				funcIndex;
};

//-------------------------------------
//	Command packet
//-------------------------------------
typedef union _IO_PACKET {
	struct _CMD_PACKET {
		BYTE Ioctl;
		BYTE Buffer[MAX_DATA_BYTES-1];
	} cmd;
	struct _STAT_PACKET {
		BYTE Data[MAX_DATA_BYTES];
	} stat;
} IO_PACKET, *PIO_PACKET;

typedef struct _BULK_CONTROL
{
	ULONG pipeNum;
} BULK_CONTROL, *PBULK_CONTROL;

typedef enum
{
	ACUISITION_NONE,
	ACUISITION_ERROR,
	ACUISITION_ABORT,
	ACUISITION_NOT_COMPLETE,
	ACUISITION_COMPLETED,
    ACUISITION_POWER_RESUME
} AcuisitionState;

typedef enum
{
	CALLBACK_DEVICE_COUNT,
	CALLBACK_INIT_PROGRESS,
	CALLBACK_COMMUNICATION_BREAK,
	CALLBACK_PREVIEW_IMAGE,
	CALLBACK_FINGER_COUNT,
	CALLBACK_FINGER_QUALITY,
	CALLBACK_TAKING_ACQUISITION,
	CALLBACK_COMPLETE_ACQUISITION,
	CALLBACK_RESULT_IMAGE,
	CALLBACK_CLEAR_PLATEN,
	CALLBACK_TRACE_LOG,
	CALLBACK_ASYNC_OPEN_DEVICE,
	CALLBACK_NOTIFY_MESSAGE,
    CALLBACK_RESULT_IMAGE_EX,
	CALLBACK_KEYBUTTON,
	CALLBACK_ONE_FRAME_IMAGE=33,        // Index for IBScanSDK
	CALLBACK_TAKE_PREVIEW_IMAGE,
	CALLBACK_TAKE_RESULT_IMAGE
} CallbackType;

typedef enum
{
	DEVICE_TYPE_UNKNOWN,
	DEVICE_TYPE_WATSON,
	DEVICE_TYPE_SHERLOCK,
	DEVICE_TYPE_SHERLOCK_ROIC,
	DEVICE_TYPE_WATSON_MINI,
	DEVICE_TYPE_COLUMBO,
	DEVICE_TYPE_CURVE,
	DEVICE_TYPE_CURVE_SETI,
	DEVICE_TYPE_HOLMES,
	DEVICE_TYPE_KOJAK,
	DEVICE_TYPE_FIVE0
} ScannerDeviceType;

typedef enum
{
	CMD_TYPE_UNKNOWN,
	CMD_TYPE_SET_GAIN,
	CMD_TYPE_GET_GAIN,
	CMD_TYPE_SET_LE,
	CMD_TYPE_GET_LE,
	CMD_TYPE_SET_SERIAL,
	CMD_TYPE_GET_SERIAL,
	CMD_TYPE_SET_LED,
	CMD_TYPE_GET_LED
} CmdType;

typedef enum
{
	NOT_SUPPORT_LE_VOLTAGE_CONTROL,
	SUPPORT_LE_VOLTAGE_CONTROL
} LEVoltageControl;

typedef enum
{
	LOG_ERROR,
	LOG_WARNING,
	LOG_INFORMATION
} LogCode;

typedef enum
{
	UNKNOWN,
    CURVE_MODEL_CYTE,           // 1st version : 2004-09-13 (LED - X, Touch - X)
    CURVE_MODEL_CYTE_V1,        // 2nd version : 2006-08-25 (LED - X, Touch - X)
    CURVE_MODEL_CYTE_V2,        // 3rd version : 2006-12-27 (LED - X, Touch - X), to support WHQL with SN
    CURVE_MODEL_TBN240,         // Current version : 2010-10-27 (LED - O, Touch - O), but it has SN bug.
    CURVE_MODEL_TBN320,
    CURVE_MODEL_TBN340,
    CURVE_MODEL_TBN320_V1,      // Future version with enhancement F/W : (LED - X, Touch - X).
    CURVE_MODEL_TBN340_V1       // Future version with enhancement F/W : (LED - O, Touch - O).
} CurveModel;

// USB Speed.  These values are the same as WinUsb.
typedef enum 
{
    USB_UNKNOWN_SPEED       = 0x00,
    USB_LOW_SPEED           = 0x01,
    USB_FULL_SPEED          = 0x02,
    USB_HIGH_SPEED          = 0x03,
    USB_SUPER_SPEED         = 0x04,
    USB_SUPER_SPEED_10_GBPS = 0x05
} UsbSpeed;

// Decimation mode.  These values are the same as transmitted to the Columbo.
typedef enum
{
    DECIMATION_2X   = 0x00,
    DECIMATION_4X   = 0x01,
    DECIMATION_NONE = 0xFF
} DecimationMode;

// Enumeration of rolling level.
typedef enum enum_IBSU_RollLevel
{
	ROLL_LEVEL_LOW,
	ROLL_LEVEL_MEDIUM,
	ROLL_LEVEL_HIGH
} RollLevel;

// Enumeration of rolling mode.
typedef enum enum_IBSU_RollMode
{
	ROLL_MODE_NO_USE,
	ROLL_MODE_USE_NOTICE
} RollMode;

// Enumeration of the image processing threshold.
typedef enum
{
	IMAGE_PROCESS_THRES_LOW,
	IMAGE_PROCESS_THRES_MEDIUM,
	IMAGE_PROCESS_THRES_HIGH,
	IMAGE_PROCESS_THRES_PERFECT,
} ImageProcessThres;

// Enumeration of the capture processing.
typedef enum
{
	ENUM_IBSM_CAPTURE_PROGRESS_NONE,
	ENUM_IBSM_CAPTURE_PROGRESS_TOUCH,
	ENUM_IBSM_CAPTURE_PROGRESS_TOF,
	ENUM_IBSM_CAPTURE_PROGRESS_CAPTURING
} CaptureProgressStatus;

typedef enum
{
	THREAD_INITIALIZE,
	THREAD_CAPTURE,
	THREAD_KEYBUTTON,
	THREAD_TOFSENSOR,
} ThreadType;

typedef enum
{
	RESERVED_ACCESS_NO,
	RESERVED_ACCESS_LEVEL_LOW,
	RESERVED_ACCESS_LEVEL_MEDIUM,
	RESERVED_ACCESS_LEVEL_HIGH,
} ReservedAccessLevel;

typedef enum
{
	TOUCH_SENSOR_MODE_NONE,
	TOUCH_SENSOR_MODE_DIGITAL,
	TOUCH_SENSOR_MODE_ANALOG,
}TouchSensorMode;

typedef struct tagUsbDeviceInfo
{
	USHORT				vid;
	USHORT				pid;
	ScannerDeviceType	devType;
	char				productName[32];
	char				serialNumber[32];
	int					CisImgWidth;
	int					CisImgHeight;
	int					CisImgSize;
	int					CisImgWidth_Roll;
	int					CisImgHeight_Roll;
	int					CisImgSize_Roll;
	int					CisRowStart;
	int					CisColStart;
	int					ImgWidth;
	int					ImgHeight;
	int					ImgSize;
	int					ImgWidth_Roll;
	int					ImgHeight_Roll;
	int					ImgSize_Roll;
	IBSU_LedType		ledType; 
	int					ledCount;
	DWORD				availableLEDs;
	int					baseResolutionX;
	int					baseResolutionY;
	LEVoltageControl	LeVoltageControl;
	BOOL				bEnableTouchIn;
	BOOL				bEnableOperationLE;
	BOOL				bEnableELVoltage;
	///////////////////////////////////
	int					pollingTimeForGetImg;
	BOOL				bNeedMask;
	BOOL				bNeedEmbeddedMask;
	BOOL				bNeedDistortionMask;
	int					nThresholdGoodRollX;
	int					nThresholdGoodRollArea;
	int					nWaterImageSize;
	int					nMIN_Elapsed;
	int					scanResolutionX;
	int					scanResolutionY;
	int					captureDeviceTypeId;
    CurveModel          curveModel;
	int					nDefaultContrastValue;
	BOOL				bNewFPCB;
	int					nDoubleBufferedFrameCount;
	int					nZoomOut;
	int					nFpgaVersion;
	UsbSpeed			nUsbSpeed;
	BOOL				bDecimation;
	DecimationMode	    nDecimation_Mode;
	IBSU_BeeperType		beeperType;
    BOOL                bSmearDetection;
	int					nInvalidAreaDetection;
	int					nASICVersion;
	BOOL				bArrangeMode;
	BOOL				bWindowsMode;
	int					CapturedBrightness[4];
	int					CapturedWetRatio[4];
	BOOL				bCanUseTOF;
	BOOL				bSpoofDetected;
} UsbDeviceInfo;

typedef struct tagRollingInfo
{
	// stitching
	int		g_StartX, g_StartY;			// 처음 위치
	int		g_EndX, g_EndY;				// 가장 끝 위치
	int		g_LastX, g_LastY;			// 최근 위치
	int		g_Stitch_Dir;				// 이동 방향 1 : -->   2 : <-- 
	BOOL	g_Rolling_Complete;			// 충분한 지문이 들어왔는지 체크
	BOOL	g_Rolling_Saved_Complete;	// 충분한 지문이 들어왔는지 체크
	BOOL	g_Rolling_BackTrace;
	int		TotalCount1, TotalCount2;

	// check rolling start condition 
	int		prev_count;
	int		prev_bright;
	int		prev_pos_x;
	int		prev_pos_y;
	int		isPassedCount;
	BOOL	bSmearDectected;
} RollingInfo;

typedef struct tagSendUsbBulkData
{
	BOOL				lsSend;
	CmdType				cmdType;
	DWORD				value;
} SendUsbBulkData;

typedef struct tagFrameImgAnalysis{
	int noise_histo;
	int noise_histo2;
	int good_frame_count;
	int frame_delay;
	BOOL is_final;
	BOOL final_adjust_gain;
	int saved_finger_count;
	int finger_count;
	int pre_foreground_count;
	int foreground_count;
	int foreground_count2;
	int center_x;
	int center_y;
	int pre_center_x;
	int pre_center_y;
	int mean;
	int gain_step;
	int changed_gain_step_count;
	int same_gain_count;
	int max_same_gain_count;
	int pre_gain;
	int	pre_DAC;
	int frame_count;
	int LEFT;
	int RIGHT;
	int TOP;
	int BOTTOM;
	int sum_pixel;
	int same_gain_time;
	BOOL is_complete_voltage_control;
	BOOL is_complete_exposure_control;
    int nAccumulated_Captured_Elapsed;
	int diff_foreground_count;
	int diff_center_x;
	int diff_center_y;
	int final_mean;
	BOOL bFinger_detected_on_Sherlock;
	BOOL bLowLeOnClock_on_Sherock;
	int detected_frame_count;
	int percent_of_255;
	int JudgeMeanMode;
	int JudgeMeanModeCount;
	int sum_mean;
	BOOL bChangedMIN_BRIGHT;
	int isDetected;
	int isDetectedTOF;
	int isChangetoSuperDry;
    int bright_pixel_count;
	int pre_exp;
}FrameImgAnalysis;

typedef struct tag_CaptureSetting
{
	int DefaultGain;
	int DefaultExposure;
	int DefaultDAC;
	int DefaultMinDAC;
	int DefaultMaxDAC;
	int DefaultBitshift;
	int DefaultCutThres;
	int TargetMinBrightness;
	int TargetMaxBrightness;
	int TargetFingerCount;
	int CurrentDAC;
	int CurrentGain;
	int CurrentExposure;
	int CurrentBitshift;
	int CurrentCutThres;
	int AdditionalFrame;
	int forSuperDry_Mode;
	int RollCaptureMode;
	int DetectionFrameCount;
	int DACPowerOff;
	int PrevDAC;
	int DACPowerOffThreshold;
}CaptureSetting;

typedef struct tagOverlayText
{
	char  familyName[IBSU_MAX_STR_LEN];
	int	  size;
	BOOL  bold;
	char text[IBSU_MAX_STR_LEN];
	int  x;
	int  y;
	DWORD color;
} OverlayText;

typedef struct tagOverlayHandle
{
    int  handle;
    BOOL show;
} OverlayHandle;

typedef struct tagOverlayTextEx
{
    int  handle;
	char  familyName[IBSU_MAX_STR_LEN];
	int	  size;
	BOOL  bold;
	char text[IBSU_MAX_STR_LEN];
	int  x;
	int  y;
	DWORD color;
    BOOL *pShow;
} OverlayTextEx;

typedef struct tagOverlayLineEx
{
    int  handle;
	int  x1;
	int  y1;
	int  x2;
	int  y2;
	int  width;
	DWORD color;
    BOOL *pShow;
} OverlayLineEx;

typedef struct tagOverlayQuadrangleEx
{
    int  handle;
	int  x1;
	int  y1;
	int  x2;
	int  y2;
	int  x3;
	int  y3;
	int  x4;
	int  y4;
	int  width;
	DWORD color;
    BOOL *pShow;
} OverlayQuadrangleEx;

typedef struct tagOverlayShapeEx
{
    int  handle;
	IBSU_OverlayShapePattern  shapePattern;
	int  x1;
	int  y1;
	int  x2;
	int  y2;
	int  width;
	DWORD color;
	int  reserved_1;
	int  reserved_2;
    BOOL *pShow;
} OverlayShapeEx;

typedef struct tagDisplayWindow
{
	BYTE	*image;
	int		x;
	int		y;
	int		Width;
	int		Height;
	int		imgX;
	int		imgY;
	int		imgWidth;
	int		imgHeight;
	int		imgX_Roll;
	int		imgY_Roll;
	int		imgWidth_Roll;
	int		imgHeight_Roll;
	DWORD	bkColor;
	BOOL	rollGuideLine;
	BOOL	bIsSetDisplayWindow;
	BOOL    dispInvalidArea;
	int     rollGuideLineWidth;
    BOOL    keepRedrawLastImage;
} DisplayWindow;

typedef struct tagFingerQuality
{
	int  x;
	int  y;
	int  pos_in_area;
	int	 mean;
	int  segment_count;
	int  quality_score;
} FingerQuality;

typedef struct tagTraceLog
{
	char	timestamp[24];
	char	category[64];
	LogCode	code;
	int		error;
	int		tid;
	char	message[__MAX_TRACE_MESSAGE_SIZE__];
} TraceLog;
/*
typedef struct tagIBUsbHandle
{
	int	 handle;
	BOOL complete;
} IBUsbHandle;
*/
typedef struct tagAllocatedMemory
{
	void *memblock;
	int  memblock_size;
} AllocatedMemory;

typedef struct tagCallbackParam
{
	int					deviceHandle;
	void*				pParentApp;
	void*				pMainCapture;
	void*				callback;
	void*				context;
	void*				pIBUsbManager;
	CallbackType		funcIndex;
//	BOOL				bIsReadySend;
	TraceLog			traceLog;
} CallbackParam;

typedef struct tagCallbackProperty
{
	int						nProgressValue;
	int						nFingerState;
	int						nPlatenState;
	IBSU_FingerQualityState	qualityArray[__MAX_QUALITY_COUNT__];
	IBSU_ImageData			imageInfo;
	IBSU_ImageData			*pSplitImageArray;
	IBSU_ImageData			oneFrameImageInfo;            // It is only for IBScanSDK
	IBSU_ImageData			takePreviewImageInfo;         // It is only for IBScanSDK
	IBSU_ImageData			takeResultImageInfo;          // It is only for IBScanSDK
	int						splitImageArrayCount;
	int						errorCode;
	int						warningCode;
    int                     nResultImageStatus;
    int                     nDetectedFingerCount;
	IBSU_ImageData			*pSegmentArray;
	int						segmentArrayCount;
	IBSU_SegmentPosition	*pSegmentPositionArray;
	int						nButtonIndex;
} CallbackProperty;

typedef struct tagBlockProperty
{
	BOOL					bGetOneFrameImage;
	int						nFingerState;
	IBSU_FingerQualityState	qualityArray[__MAX_QUALITY_COUNT__];
	IBSU_ImageData			imageInfo;
	IBSU_ImageType			imageType;
	IBSU_ImageData			*pSplitImageArray;
	int						splitImageArrayCount;
	int						errorCode;
	BOOL					bFillResultImage;
    int                     nResultImageStatus;
    int                     nDetectedFingerCount;
	IBSU_ImageData			*pSegmentArray;
	int						segmentArrayCount;
	IBSU_SegmentPosition	*pSegmentPositionArray;
} BlockProperty;

typedef struct tag_KOJAK_CALIBRATION_INFO
{
	short Version;
	short ACMin;
	short ACMax;
	short LensPosRow;
	short LensPosCol;
	short PPIHorizontal;
	short PPIVertical;
	short IsSupportSpoof;
}
KOJAK_CALIBRATION_INFO;

typedef struct tagPropertyInfo
{
	IBSU_ImageType	ImageType;
	int				nCaptureOptions;
	int				nNumberOfObjects;
	BOOL			bAsyncOpenDevice;
	int				nLEOperationMode;
	int				nContrastValue;
	int				nVoltageValue;
	int				nIgnoreFingerTime;
	DWORD			dwActiveLEDs;
	char			cFirmware[IBSU_MAX_STR_LEN];
	int				nContrastTres;
	BOOL			bIsCompleteVoltageControl;
	IBSU_ImageData	splitImageInfoArray[IBSU_MAX_SEGMENT_COUNT];
	char			cProductID[IBSU_MAX_STR_LEN];
	char			cSerialNumber[IBSU_MAX_STR_LEN];
	char			cDevRevision[IBSU_MAX_STR_LEN];
	char			cVendorID[IBSU_MAX_STR_LEN];
	char			cProductionDate[IBSU_MAX_STR_LEN];
	char			cServiceDate[IBSU_MAX_STR_LEN];
	BOOL			bEnablePowerSaveMode;
	BOOL			bEnableFastFrameMode;       // It is only for IBScanSDK
	BOOL			bRawCaptureStart;           // It is only for IBScanSDK
	int				nIntegrationValue;
	int				retryCountWrongCommunication;
    int             nCaptureTimeout;
    int             nRollMinWidth;
    RollMode		nRollMode;
    RollLevel		nRollLevel;
	int				nCaptureAreaThreshold;
	int				nSINGLE_FLAT_AREA_TRES;
	IBSU_ImageData	segmentArray[IBSU_MAX_SEGMENT_COUNT];
	IBSU_SegmentPosition	segmentPositionArray[IBSU_MAX_SEGMENT_COUNT];
	BOOL			bEnableDecimation;
	BOOL			bSavedEnableDecimation;
	BOOL			bEnableCaptureOnRelease;
	ImageProcessThres	nImageProcessThres;
	int				nDevId;
	int				nSuperDryMode;
	int				nWaitTimeForDry;
	BOOL			bNoPreviewImage;
	BOOL			bRollImageOverride;
	BOOL			bWarningInvalidArea;
	BOOL			bEnableStartingVoltage;
	int				nStartingVoltage;
	float			nSharpenValue;
	BOOL			bWetFingerDetect;
	int				nWetFingerDetectLevel;
	int				nWetFingerDetectLevelThres[5];
	BOOL			bEnableTOF;
	BOOL			bEnableTOFforROLL;
    char            cTOFAnalogPlate[IBSU_MAX_STR_LEN];
	int				nStartingPositionOfRollArea;
	BOOL			bStartRollWithoutLock;
	int				nCaptureBrightThresFlat[2];
	int				nCaptureBrightThresRoll[2];
	BOOL			bReservedEnhanceResultImage;
    KOJAK_CALIBRATION_INFO sKojakCalibrationInfo;
	BOOL			bKojakPLwithDPFilm;
	BOOL			bEnableEncryption;
	int				nExposureValue;
	BOOL			bColumboBaordforPI;
	BOOL			bIsSpoofSupported;
	BOOL			bEnableSpoof;
	int				nSpoofLevel;
	BOOL			bEnableCBPMode;
} PropertyInfo;

typedef struct{
	int lessTHAN2;
	int lessTHAN3;
	int lessTHAN4;
	int prev_lessTHAN2;
	int prev_lessTHAN3;
	int prev_lessTHAN4;
	int noise_lessTHAN2;
	int noise_lessTHAN3;
	int noise_lessTHAN4;
	int noise_histo;
	int prev_histo;
	int current_histo;
	int	real_histo;
	int detected_count;
	int NOISE_DETECT;
	int condition_stay;
	int STOP_DETECT_COUNT;
	int GRAP_NULL_FRAME;
	int dry_count;
	int isDetectGain;
	int tot_frame;
	int integration_count;
	int skip_count;
	int same_gain_count;
} GoodCaptureInfo;

typedef struct{
	IBSU_SegmentPosition	SegPosArr[IBSU_MAX_SEGMENT_COUNT];
	IBSU_SegmentPosition	SegPosArr_90[IBSU_MAX_SEGMENT_COUNT];
	int						SegPosArrCount;
} CBPPreviewInfo;

typedef struct tagBestFrameInfo
{
	BYTE					*Buffer;
	int						score;
	FrameImgAnalysis		imgAnalysis;
} BestFrameInfo;

typedef struct tag_KOJAK_LED
{
	unsigned char LeftLittle : 4;
	unsigned char LeftRing : 4;
	unsigned char LeftMiddle : 4;
	unsigned char LeftIndex : 4;
	unsigned char LeftThumb : 4;
	unsigned char RightThumb : 4;
	unsigned char RightIndex : 4;
	unsigned char RightMiddle : 4;
	unsigned char RightRing : 4;
	unsigned char RightLittle : 4;
	unsigned char Roll : 1;
	unsigned char LeftProgress : 1;
	unsigned char ThumbProgress : 1;
	unsigned char RightProgress : 1;
	unsigned char Reserved : 4;
} 
KOJAK_LED;

typedef struct tag_EEPROM_Ver
{
	unsigned char StartFlag;
	unsigned char Length;
	unsigned char StructVer;	// default : 0x01
	unsigned char Data[28];		// 32 - 4
	unsigned char EndFlag;
} EEPROM_Ver;

// USB ID numbers
#define __DEFAULT_CONTRAST_VALUE__			21
#define __AUTO_CONTRAST_MAX_VALUE__			27
#define __DEFAULT_CONTRAST_TRES__			0
#define __DEFAULT_VOLTAGE_VALUE__			11
#define __AUTO_VOLTAGE_MAX_VALUE__			15

#define __WATSON_DEFAULT_VOLTAGE_VALUE__	9
#define __WATSON_DEFAULT_EXPOSURE_VALUE__	0x0BB8
#define __WATSON_MINIMUM_EXPOSURE_VALUE__	0x0500

#define __COLUMBO_DEFAULT_EXPOSURE_VALUE__	0x04FF
#define __COLUMBO_MINIMUM_EXPOSURE_VALUE__	0x00FF

#define __DEFAULT_VOLTAGE_VALUE_FOR_KOJAK__	0xFF

#define __ROIC_VOLTAGE_MIN_VALUE__			0			// 216v
#define __ROIC_VOLTAGE_DEFAULT_VALUE__		18			// 248v			(21 is best for dragon)
#define __ASIC_VOLTAGE_DEFAULT_VALUE__		11
#define __ROIC_VOLTAGE_MAX_VALUE__			34			// 282v

//#define __CAPTURE_MIN_BRIGHT__				45
//#define __CAPTURE_MAX_BRIGHT__				70
#define __CAPTURE_MIN_BRIGHT__				40
#define __CAPTURE_MAX_BRIGHT__				85

#define __WATSON_CIS_IMG_QUATER_SIZE__		277984

#define __DEFAULT_COLUMBO_CONTRAST_VALUE__			21		// Changed Columbo's voltage, so it need to change value 
#define __AUTO_COLUMBO_CONTRAST_MAX_VALUE__			34
#if defined(WINCE)
#define __DEFAULT_COLUMBO_DECIMATION_COUNT__		2
#define __DEFAULT_KOJAK_DECIMATION_COUNT__			1
#else
#define __DEFAULT_COLUMBO_DECIMATION_COUNT__		3
#define __DEFAULT_KOJAK_DECIMATION_COUNT__			1
#endif

// auto gain
//#define D_NOISE_MINVALUE	10
//#define SINGLE_FLAT_AREA_TRES	1500
#define SINGLE_FLAT_AREA_TRES	500
#define SINGLE_FLAT_DIFF_TRES	200//1000

//slap finger segmentation 
#define THRESHOLD_DIFFERENT_ANGLE_1			45
#define THRESHOLD_DIFFERENT_ANGLE_2			15//20
//#define MIN_FOREGROUND_AREA					(1500*25/(D_ZOOM_OUT*D_ZOOM_OUT))
//#define MIN_MAIN_FOREGROUND_AREA			(500*25/(D_ZOOM_OUT*D_ZOOM_OUT))
//#define MIN_FOREGROUND_AREA					(1500)
#define MIN_FOREGROUND_AREA					(SINGLE_FLAT_AREA_TRES)
#define MIN_MAIN_FOREGROUND_AREA			(500)
#define MAX_SEGMENT_COUNT					100

#define FALSE_SEGMENT_ANGLE_THRESHOLD		45

typedef struct{
	int		X;
	int		Y;
	int		Ang;
	int		Label;
	int		Area;
} SEGMENT;

typedef struct{
	int		SegmentCnt;
	SEGMENT	Segment[MAX_SEGMENT_COUNT];
} SEGMENT_ARRAY;

typedef struct{
	short	X;
	short	Y;
	unsigned short	Ang;
	unsigned short	Label;
	unsigned short	Area;
	short	TipX;
	short	TipY;
	short	KnuckleX;
	short	KnuckleY;
	short	LeftX;
	short	LeftY;
	short	RightX;
	short	RightY;
	short	CenterX;
	short	CenterY;
	short	P1_X;
	short	P1_Y;
	short	P2_X;
	short	P2_Y;
	short	P3_X;
	short	P3_Y;
	short	P4_X;
	short	P4_Y;
} NEW_SEGMENT;

typedef struct{
	unsigned short	SegmentCnt;
	NEW_SEGMENT		VCenterSegment;
	NEW_SEGMENT		Segment[MAX_SEGMENT_COUNT];
} NEW_SEGMENT_ARRAY;

// For uniformity mask
#define UM_MAX_PACKET_SIZE		1024
#define UM_ONE_PAGE_SIZE		512
#define UM_MAX_PAGE_SIZE		528

// For Auto segmentation
#define D_BLOCK_SIZE	20
#define SKIP_BLOCK		5
#define RE_CIS_IMG_H	1030
#define RE_CIS_IMG_W	1100
#define ENBLOCK_IMG_H	(RE_CIS_IMG_H+D_BLOCK_SIZE*2)
#define ENBLOCK_IMG_W	(RE_CIS_IMG_W+D_BLOCK_SIZE*2)


// Rolling : Stitching Range
//#define PIXEL_ASSIGN_RANGE_LARGE	16
//#define PIXEL_ASSIGN_RANGE_NORMAL	8
//#define REMOVAL_SMALL_AREA			5000

#define REMOVAL_SMALL_AREA			20000	
#define PIXEL_ASSIGN_RANGE			16
#define PIXEL_MAX_RANGE				48
#define PIXEL_INCREASE_RANGE		4
#define THRESHOLD_DIST_BTW_FINGERS	180

#define FOREGROUND_BRIGHT_VALUE		15
#define THRESHOLD_DIFF_BRIGHT		5
#define THRESHOLD_POSITION_X		5
#define THRESHOLD_POSITION_Y		5
#define THRESHOLD_GOOD_ROLL_X		120				//((CIS_IMG_W*3)>>3)
#define THRESHOLD_GOOD_ROLL_AREA	100000			//((CIS_IMG_SIZE*4)>>5)

// ERROR_MESSAGE
#define SUCCESS					0xFF
#define NO_FINGER				0x00
#define FINGER_DETECTED			0x01
#define ROLL_COMPLETE			0x02
#define ROLL_COMPLETE_BACKTRACE	0x03






#if defined(_WINDOWS) && !defined(WINCE)
	#define		EP1OUT				0
	#define		EP1IN				1
	#define		EP2OUT				2
	#define		EP4OUT				3
//	#define		EP6IN				2			// Endpoint 6in
	#define		EP6IN				4			// Endpoint 6in index
	#define		EP8IN				5

	#define		EP6IN_CURVE			2
#elif defined(__linux__) || defined(WINCE)
	#define		EP1OUT				0x01
	#define		EP1IN				0x81
	#define		EP2OUT				0x02
	#define		EP4OUT				0x04
	#define		EP6IN				0x86
	#define		EP8IN				0x88
	#define		EP6IN_CURVE			0x86
	#define		USB_INTERFACE		0
#endif

typedef enum
{
	PROPERTY_PRODUCT_ID=0,
	PROPERTY_SERIAL_NUMBER,
	PROPERTY_VENDOR_ID,
	PROPERTY_IBIA_VENDOR_ID,		// TOF info for KOJAK and FIVE-0
	PROPERTY_IBIA_VERSION,			// EEPROM version instead of FPGA or CPLD
	PROPERTY_IBIA_DEVICE_ID,
	PROPERTY_FIRMWARE,
	PROPERTY_REVISION,
	PROPERTY_PRODUCTION_DATE,
	PROPERTY_SERVICE_DATE,
	PROPERTY_CMT1,
	PROPERTY_CMT2,
	PROPERTY_CMT3,
	PROPERTY_CMT4,
	PROPERTY_CURVE_SETI_MODEL
}DEV_INFO_ADDRESS;

typedef enum
{
	ADDRESS_LE_AUTO_MODE=0,
	ADDRESS_LE_ON_MODE,
	ADDRESS_LE_OFF_MODE
}LE_OPERATION_ADDRESS;

typedef enum
{
	ENUM_FPGA_PWR_ON=0,
	ENUM_FPGA_PWR_OFF,
	ENUM_TOUCH_PWR_ON,
	ENUM_TOUCH_PWR_OFF
}FORCE_POWER_CONTROL_ADDRESS;

#define LED_ALL_OFF					0
#define INIT_LED_PWR_ON				1
#define LED_PWR_ON					2

#define CMD_NOP						0x00
#define CMD_READ_STATUS				0x01
#define CMD_RESET_FIFO				0x02
#define CMD_READ_EEPROM				0x03
#define CMD_WRITE_EEPROM			0x04
#define CMD_READ_FPGA_REG			0x05
#define CMD_WRITE_FPGA_REG			0x06
#define CMD_READ_CIS_REG			0x07
#define CMD_WRITE_CIS_REG			0x08
#define CMD_FORCE_POWER_CONTROL		0x09
#define	CMD_SUSPEND					0x0A

// new command
#define	CMD_DEVICE_INITIALIZE		0x0B		// wakeup cypress only
#define	CMD_DEVICE_RELEASE			0x0C		// enter sleep
#define	CMD_CAPTURE_START			0x0D
#define	CMD_CAPTURE_END				0x0E
#define	CMD_READ_PROPERTY			0x0F
#define	CMD_WRITE_PROPERTY			0x10
#define CMD_READ_LED				0x11
#define CMD_WRITE_LED				0x12
#define CMD_READ_LE_MODE			0x13
#define CMD_WRITE_LE_MODE			0x14
#define CMD_READ_TOUCH_IN_STATUS	0x15
#define CMD_WRITE_LE_VOLTAGE		0x16
#define CMD_INIT_LE_VOLTAGE			0x17
#define CMD_READ_LE_VOLTAGE			0x18

#define CMD_GET_ONE_FRAME			0x19
#define CMD_READ_SETI_REG			0x1A
#define CMD_WRITE_SETI_REG			0x1B

#define CMD_READ_ASIC_REG			0x20
#define CMD_WRITE_ASIC_REG			0x21
#define CMD_ASIC_RESET				0x22
#define CMD_ANALOG_POWER_ON			0x23

#define CMD_START_READ_MASK         0X33 //For Columbo read Mask by Sean 2013/06/05
#define CMD_STOP_READ_MASK			0x34

#define CMD_DAC_WRITE_ADDR  		0xC0
#define CMD_DAC_READ_ADDR  			0xC1

#define ASIC_MASTER_ADDRESS			0x84
#define ASIC_SLAVE_ADDRESS			0x86

#define CMD_WRITE_SETI_REG_CURVE	0x67
#define CMD_READ_SETI_REG_CURVE		0x68

#define CMD_WRITE_LE_VOLTAGE_DAC	0x77
#define CMD_READ_LE_VOLTAGE_DAC		0x18

#define FIVE0_ASIC_GLOBAL_ADDRESS	0x92
#define FIVE0_ASIC_MASTER_ADDRESS	0xA6
#define FIVE0_ASIC_SLAVE_ADDRESS	0xAA

#define FIVE0_WRITE_ASIC_EEPROM		0x24
#define FIVE0_READ_ASIC_EEPROM		0x25

#define CMD_EEPROM_WRITEPROTECTION_ENABLE	0xFA

#define CMD_WRITE_ADC_THRES			0x79

////////////////////////////////////////////////////////////////
// Hynix command
#define NET_IO_CONTROL_BUFFER_SIZE      62

////////////////////////////////////////////////////////////////
// commands for KOJAK
#define CMD_KOJAK_LE_OFF			0x1C
#define CMD_KOJAK_LE_ON				0x1D
#define CMD_KOJAK_WRITE_LED			0x19
#define CMD_KOJAK_READ_BUTTON_STAT	0x1A
#define CMD_KOJAK_CLEAR_BUTTON_STAT	0x1B
#define CMD_KOJAK_MASK_RELAY_OFF	0x20
#define CMD_KOJAK_READ_LED			0x21
#define CMD_KOJAK_POWER_OFF_BUTTON	0x23

#define CMD_KOJAK_TOUCH_ON			0x1C
#define CMD_KOJAK_TOUCH_OFF			0x1D
#define CMD_KOJAK_TOUCH_READ		0x1E
#define CMD_KOJAK_WRITE_TOF			0x28
#define CMD_KOJAK_READ_TOF			0x29
#define CMD_KOJAK_TOUCH_SET_STATE	0x2A
#define CMD_KOJAK_TOUCH_GET_STATE	0x2B

typedef struct _NET_IO_CONTROL
{
    unsigned char BufferSize;
	unsigned char Ioctl;
	unsigned char Buffer[NET_IO_CONTROL_BUFFER_SIZE];
} NET_IO_CONTROL, *PNET_IO_CONTROL;


typedef struct _BULK_TRANSFER_CONTROL
{
   ULONG pipeNum;
} BULK_TRANSFER_CONTROL, *PBULK_TRANSFER_CONTROL;


typedef struct _GET_STRING_DESCRIPTOR_IN
{
   UCHAR    Index;
   USHORT   LanguageId;
} GET_STRING_DESCRIPTOR_IN, *PGET_STRING_DESCRIPTOR_IN;


#define NACTL_NO_OPERATION				0
#define NACTL_CONFIG_CIS_REGISTER		1
#define NACTL_READ_CIS_REGISTER			2
#define NACTL_START_IMAGE_STREAM		3
#define NACTL_STOP_IMAGE_STREAM			4
#define NACTL_SYNC_START_IMAGE			5
#define NACTL_SYNC_STOP_IMAGE			6
#define NACTL_INITIAL_FLAG				7
#define NACTL_CHECK_START_FRAME			8
#define NACTL_CHECK_END_FRAME			9
#define NACTL_ELPOWER_3V				10
#define NACTL_ELPOWER_4V				11
#define NACTL_WAITFOR_FRAMESYNC			12

#define NACTL_WRITE_BYTE_REGISTER		20
#define NACTL_READ_BYTE_REGISTER		21
#define NACTL_WRITE_GAIN_REGISTER		22

#define NACTL_READ_ID_CODE				30
#define NACTL_WRITE_ID_CODE				40  // private command

#define NA2CTL_SELECT_CIS_CLOCK			100
#define NA2CTL_SELECT_EL_VOLTAGE		101
#define NA2CTL_READ_STATUS				102
#define NA2CTL_WRITE_CIS_REGISTER		103
#define NA2CTL_READ_CIS_REGISTER		104
#define NA2CTL_GET_ONE_FRAME			105
#define NA2CTL_WRITE_EEPROM				130
#define NA2CTL_WRITE_IDCODE				131
#define NA2CTL_READ_EEPROM				140
#define NA2CTL_READ_IDCODE				141
#define NA2CTL_SET_EL_POWER_STATE       106
#define NA2CTL_WRITE_WIN_POS			107
#define NA2CTL_READ_WIN_POS				108
#define NA2CTL_RESET_FIFO				109
#define NA2CTL_SET_NORMAL_OPERATION		110
#define NA2CTL_SET_IDLE					111
#define NA2CTL_LED_CONTROL				112
#define NA2CTL_TOUCH_CONTROL			113
////////////////////////////////////////////////////////////////

/*
typedef struct _FX2_STATUS {
	BOOL	bHighSpeedMode;
	BOOL	bEp2Stall;
	BOOL	bEp2Empty;
	BOOL	bEp2Full;
	UCHAR	cEp2PacketNum;
	BOOL	bEp4Stall;
	BOOL	bEp4Empty;
	BOOL	bEp4Full;
	UCHAR	cEp4PacketNum;
	BOOL	bEp6Stall;
	BOOL	bEp6Empty;
	BOOL	bEp6Full;
	UCHAR	cEp6PacketNum;
	BOOL	bEp8Stall;
	BOOL	bEp8Empty;
	BOOL	bEp8Full;
	UCHAR	cEp8PacketNum;
	USHORT	nEp2FifoCount;
	USHORT	nEp4FifoCount;
	USHORT	nEp6FifoCount;
	USHORT	nEp8FifoCount;
} FX2_STATUS, *PFX2_STATUS;
//----------------------------------------------------------
#define	MT9P001_SHUTTER_WIDTH_UPPER		0x08
#define	MT9P001_SHUTTER_WIDTH_LOWER		0x09
#define MT9P001_GLOBAL_GAIN				0x35

#define	MT9E001_MODE_SELECT		0x0100
#define	MT9E001_SOFTWARE_RESET	0x0103

#define	MT9E_DATA_PEDESTAL				0x0008, 2
#define	MT9E_MODE_SELECT				0x0100, 1
#define	MT9E_IMAGE_ORIENTATION			0x0101, 1
#define	MT9E_SOFTWARE_RESET				0x0103, 1
#define	MT9E_GROUPED_PARAMETER_HOLD		0x0104, 1
#define	MT9E_MASK_CORRUPTED_FRAME		0x0105, 1
#define	MT9E_CCP_DATA_FORMAT			0x0112, 2
#define	MT9E_FINE_INTEGRATION_TIME		0x0200, 2
#define	MT9E_COARSE_INTEGRATION_TIME	0x0202, 2
#define	MT9E_ANALOGUE_GAIN_CODE_GLOBAL	0x0204, 2
#define	MT9E_ANALOGUE_GAIN_CODE_GREENR	0x0206, 2
#define	MT9E_ANALOGUE_GAIN_CODE_RED		0x0208, 2
#define	MT9E_ANALOGUE_GAIN_CODE_BLUE	0x020A, 2
#define	MT9E_ANALOGUE_GAIN_CODE_GREENB	0x020C, 2
#define	MT9E_DIGITAL_GAIN_GREENR		0x020E, 2
#define	MT9E_DIGITAL_GAIN_RED			0x0210, 2
#define	MT9E_DIGITAL_GAIN_BLUE			0x0212, 2
#define	MT9E_DIGITAL_GAIN_GREENB		0x0214, 2
#define	MT9E_VT_PIX_CLK_DIV				0x0300, 2
#define	MT9E_VT_SYS_CLK_DIV				0x0302, 2
#define	MT9E_PRE_PLL_CLK_DIV			0x0304, 2
#define	MT9E_PLL_MULTIPLIER				0x0306, 2
#define	MT9E_OP_PIX_CLK_DIV				0x0308, 2
#define	MT9E_OP_SYS_CLK_DIV				0x030A, 2
#define	MT9E_FRAME_LENGTH_LINE			0x0340, 2
#define	MT9E_LINE_LENGTH_PCK			0x0342, 2
#define	MT9E_X_ADDR_START				0x0344, 2
#define	MT9E_Y_ADDR_START				0x0346, 2
#define	MT9E_X_ADDR_END					0x0348, 2
#define	MT9E_Y_ADDR_END					0x034A, 2
#define	MT9E_X_OUTPUT_SIZE				0x034C, 2
#define	MT9E_Y_OUTPUT_SIZE				0x034E, 2
#define	MT9E_X_EVEN_INC					0x0380, 2
#define	MT9E_X_ODD_INC					0x0382, 2
#define	MT9E_Y_EVEN_INC					0x0384, 2
#define	MT9E_Y_ODD_INC					0x0386, 2
#define	MT9E_SCALING_MODE				0x0400, 2
#define	MT9E_SPATIAL_SAMPLING			0x0402, 2
#define	MT9E_SCALE_M					0x0404, 2
#define	MT9E_SCALE_N					0x0406, 2
#define	MT9E_COMPRESSION_MODE			0x0500, 2
#define	MT9E_TEST_PATTERN_MODE			0x0600, 2
#define	MT9E_TEST_DATA_RED				0x0602, 2
#define	MT9E_TEST_DATA_GREENR			0x0604, 2
#define	MT9E_TEST_DATA_BLUE				0x0606, 2
#define	MT9E_TEST_DATA_GREENB			0x0608, 2
#define	MT9E_HORIZONTAL_CURSOR_WIDTH	0x060A, 2
#define	MT9E_HORIZONTAL_CURSOR_POSITION	0x060C, 2
#define	MT9E_VERTICAL_CURSOR_WIDTH		0x060E, 2
#define	MT9E_VERTICAL_CURSOR_POSTION	0x0610, 2

#define	MT9E_FINE_CORRECTION			0x3010, 2
#define	MT9E_ROW_SPEED					0x3016, 2
#define	MT9E_EXTRA_DELAY				0x3018, 2
#define	MT9E_RESET_REGISTER				0x301A, 2
#define	MT9E_GPI_STATUS					0x3026, 2
#define	MT9E_FRAME_STATUS				0x303C, 2
#define	MT9E_READ_MODE					0x3040, 2
#define	MT9E_FLASH						0x3046, 2
#define	MT9E_FLASH_COUNT				0x3048, 2
#define	MT9E_GREEN1_GAIN				0x3056, 2
#define	MT9E_BLUE_GAIN					0x3058, 2
#define	MT9E_RED_GAIN					0x305A, 2
#define	MT9E_GREEN2_GAIN				0x305C, 2
#define	MT9E_GLOBAL_GAIN				0x305E, 2
#define	MT9E_DATAPATH_STATUS			0x306A, 2
#define	MT9E_DATAPATH_SELECT			0x306E, 2
#define	MT9E_CALIB_GREEN1_ASC1			0x30A8, 2
#define	MT9E_CALIB_BLUE_ASC1			0x30AA, 2
#define	MT9E_CALIB_RED_ASC1				0x30AC, 2
#define	MT9E_CALIB_GREEN2_ASC1			0x30AE, 2

//----------------------------------------------------------
#define	RES_3264X2448_8BIT		0x00		//	0 : 3264 x 2448		--> 1440 x 1080
#define	RES_3264X2448_16BIT		0x10
#define RES_1632X1224_8BIT		0x02		//  2 : 1632 x 1224		--> 720 x 540
#define	RES_1632X1224_16BIT		0x12
#define	RES_816X612_8BIT		0x03		//	3 : 816 x 612		--> 360 x 270
#define	RES_816X612_16BIT		0x13
#define RES_UNKNOWN				0xff

#define	RES_1440X1080_8BIT		0x00
#define	RES_1440X1080_16BIT		0x10
#define	RES_720X540_8BIT		0x02
#define	RES_720X540_16BIT		0x12
#define	RES_360X270_8BIT		0x03
#define	RES_360X270_16BIT		0x13
#define	RES_1024X1024_8BIT		0x04
*/

//----------------------------------------------------------
#define	MT9M_CHIP_VERSION				0x00
#define	MT9M_ROW_START					0x01
#define	MT9M_COLUMN_START				0x02
#define	MT9M_ROW_SIZE					0x03
#define	MT9M_COLUMN_SIZE				0x04
#define	MT9M_HORIZONTAL_BLANK			0x05
#define	MT9M_VERTICAL_BLANK				0x06
#define	MT9M_OUTPUT_CONTROL				0x07
#define	MT9M_SHUTTER_WIDTH_UPPER		0x08
#define	MT9M_SHUTTER_WIDTH_LOWER		0x09
#define	MT9M_PIXEL_CLOCK_CONTROL		0x0A
#define	MT9M_RESTART					0x0B
#define	MT9M_SHUTTER_DELAY				0x0C
#define	MT9M_RESET						0x0D
#define	MT9M_PLL_CONTROL				0x10
#define	MT9M_PLL_CONFIG_1				0x11
#define	MT9M_PLL_CONFIG_2				0x12
#define	MT9M_READ_MODE_1				0x1E
#define MT9M_READ_MODE_2				0x20
#define	MT9M_GREEN1_GAIN				0x2B
#define	MT9M_BLUE_GAIN					0x2C
#define	MT9M_RED_GAIN					0x2D
#define	MT9M_GREEN2_GAIN				0x2E
#define	MT9M_GLOBAL_GAIN				0x35
#define	MT9M_CLAMP_VOLTAGE				0x41
#define	MT9M_ADC_REF_LO_VOLTAGE			0x42
#define	MT9M_ADC_REF_HI_VOLTAGE			0x43
#define	MT9M_ROW_BLACK_TARGET			0x49
#define	MT9M_ROW_BLACK_DEFAULT_OFFSET	0x4B
#define	MT9M_BLC_SAMPLE_SIZE			0x5B
#define	MT9M_BLC_TUNE_1					0x5C
#define	MT9M_BLC_DELTA_THRESHOLDS		0x5D
#define	MT9M_BLC_TUNE_2					0x5E
#define	MT9M_BLC_TARGET_THRESHOLDS		0x5F
#define	MT9M_GREEN1_OFFSET				0x60
#define	MT9M_GREEN2_OFFSET				0x61
#define	MT9M_BLACK_LEVEL_CALIBRATION	0x62
#define	MT9M_RED_OFFSET					0x63
#define	MT9M_BLUE_OFFSET				0x64
#define	MT9M_RD_EN_VLN_SH_TIMING		0x7F
#define	MT9M_RESERVED_R148				0x94
#define	MT9M_FORMATTER_0				0x9D
#define	MT9M_FORMATTER_1				0x9E
#define	MT9M_FORMATTER_2				0x9F
#define	MT9M_CHIP_VERSION_ALT			0xFF

//---------------------------------------------------------------------------
#define IMAGE_WIDTH		288
#define IMAGE_HEIGHT	352

#define		ENLARGE_CELL_H		16
#define		ENLARGE_CELL_W		16

#define		ENLARGE_IMG_H		(IMAGE_WIDTH+ENLARGE_CELL_H*2)
#define		ENLARGE_IMG_W		(IMAGE_HEIGHT+ENLARGE_CELL_W*2)

#define		ENLARGE_HALF_H		(ENLARGE_IMG_H/2)
#define		ENLARGE_HALF_W		(ENLARGE_IMG_W/2)

#define		ENLARGE_QUARTER_H	(ENLARGE_IMG_H>>2)
#define		ENLARGE_QUARTER_W	(ENLARGE_IMG_W>>2)

#define D_IMAGE_HEIGHT				352
#define D_IMAGE_WIDTH				288
#define	D_ENLARGE_BAND_H			16
#define	D_ENLARGE_BAND_W			16
#define	D_ENLARGE_H					(D_IMAGE_HEIGHT+D_ENLARGE_BAND_H*2)
#define	D_ENLARGE_W					(D_IMAGE_WIDTH+D_ENLARGE_BAND_W*2)
#define	D_ENLARGE_HALF_H			(D_ENLARGE_H/2)
#define	D_ENLARGE_HALF_W			(D_ENLARGE_W/2)
#define	D_ENLARGE_QUARTER_H			(D_ENLARGE_H>>2)
#define	D_ENLARGE_QUARTER_W			(D_ENLARGE_W>>2)



//----------------------------------------------------------
// Holes FPGA Registers
#define	HOLMES_FPGA_PRODUCT_ID_MSB				0x00
#define	HOLMES_FPGA_PRODUCT_ID_LSB				0x01
#define	HOLMES_FPGA_CHIP_ENABLE					0x02
#define	HOLMES_FPGA_POWER_DOWN					0x03		// Not used
#define	HOLMES_FPGA_SOFT_RESET					0x04
#define	HOLMES_FPGA_PIXCLK						0x05		// Not used
#define	HOLMES_FPGA_ANALOG_GAIN					0x06
#define	HOLMES_FPGA_DIGITAL_GAIN				0x07
#define	HOLMES_FPGA_EXPOSURE_CONTROL			0x08
#define	HOLMES_FPGA_AUTO_BRIGHTNESS_CONTROL		0x09
#define	HOLMES_FPGA_DECIMATION					0x0A

#define	HOLMES_FPGA_WINDOW_SIZE_ROW_MSB			0x0B
#define	HOLMES_FPGA_WINDOW_SIZE_ROW_LSB			0x0C
#define	HOLMES_FPGA_WINDOW_SIZE_COL_MSB			0x0D
#define	HOLMES_FPGA_WINDOW_SIZE_COL_LSB			0x0E

#define	HOLMES_FPGA_WINDOW_POS_ROW_MSB			0x0F
#define	HOLMES_FPGA_WINDOW_POS_ROW_LSB			0x10
#define	HOLMES_FPGA_WINDOW_POS_COL_MSB			0x11
#define	HOLMES_FPGA_WINDOW_POS_COL_LSB			0x12

#define	HOLMES_FPGA_BLACK_LEVEL_THRESH			0x13
#define	HOLMES_FPGA_ADC_OFFSET					0x14
#define	HOLMES_FPGA_MIRROR_CONTROL				0x15
#define	HOLMES_FPGA_FLIP_CONTROL				0x16
#define	HOLMES_FPGA_CAPTURE_CONTROL				0x17
#define	HOLMES_FPGA_EL_TIME						0x18
#define	HOLMES_FPGA_EL_DELAY_TIME				0x19





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			Matcher
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum enumIBSU_FingerIndex
{
    ENUM_IBSU_FINGER_LEFT_LITTLE             = 0,
	ENUM_IBSU_FINGER_LEFT_RING               = 1,
	ENUM_IBSU_FINGER_LEFT_MIDDLE             = 2,
	ENUM_IBSU_FINGER_LEFT_INDEX              = 3,
	ENUM_IBSU_FINGER_LEFT_THUMB              = 4,
	ENUM_IBSU_FINGER_RIGHT_THUMB             = 5,
	ENUM_IBSU_FINGER_RIGHT_INDEX             = 6,
	ENUM_IBSU_FINGER_RIGHT_MIDDLE            = 7,
	ENUM_IBSU_FINGER_RIGHT_RING              = 8,
	ENUM_IBSU_FINGER_RIGHT_LITTLE            = 9
}IBSU_FingerIndex;

// 제품 변경시 __SCT_PRODUCT_TYPE__ 을 설정한다. 반드시 !!!
#define __SCT_PRODUCT_ALGO_TEST__   1
#define __SCT_PRODUCT_SDK__         2
#define __SCT_PRODUCT_MODULE__      3
#define __SCT_PRODUCT_TnA__         4

#define __SCT_PRODUCT_TYPE__        __SCT_PRODUCT_SDK__

#define IMAGE_BUFFER_COUNT						10

#define MAX_IMAGE_W                 2048
#define MAX_IMAGE_H                 2048
#define MAX_IMAGE_SIZE              (MAX_IMAGE_W*MAX_IMAGE_W)
#define MAX_DIAGONAL_LENGTH         2900

#define D_ENLARGE_BAND              16

#define MAX_MATH_SIZE               1024
#define QUANTIZED_DIRECTION         256

// Histogram Stretch
#define IBSM_D_CUT_MINTRES               0.3//0.01
#define IBSM_D_CUT_MAXTRES               0.05//0.01

//Normalization Definition
#define D_CONTRAST_MIN              10
#define D_CONTRAST_MAX              245

//Frequency Definition
#define D_FREQUENCY_BOX_WIDTH       40
#define D_FREQUENCY_BOX_HEIGHT      20
#define D_FREQUENCY_BLOCK_SIZE      16
#define D_FREQUENCY_MIN             4
#define D_FREQUENCY_MAX             16
#define D_FREQUENCY_WIDTH           (D_ENLARGE_W/D_FREQUENCY_BLOCK_SIZE)
#define D_FREQUENCY_HEIGHT          (D_ENLARGE_H/D_FREQUENCY_BLOCK_SIZE)
#define D_FREQUENCY_AREA_SIZE       30

// Gabor
#define D_GABOR_QUANTIZE            10000
#define D_GABOR_HALF_WINDOW         7
#define D_GABOR_DELTA               4
#define D_GABOR_SQURE_DELTA         (D_GABOR_DELTA*D_GABOR_DELTA)
#define D_GABOR_ROW_WINDOW          (D_GABOR_HALF_WINDOW*2+1)
#define D_GABOR_COLS_WINDOW         (D_GABOR_HALF_WINDOW*2+1)
#define D_GABOR_ROWCOLS_WINDOW      (D_GABOR_ROW_WINDOW*D_GABOR_COLS_WINDOW)
#define D_GABOR_ANGLEROWCOLS_WINDOW (64*D_GABOR_ROW_WINDOW*D_GABOR_COLS_WINDOW)
#define D_GABOR_TABLE_SIZE          ((D_FREQUENCY_MAX-D_FREQUENCY_MIN+1)*64*D_GABOR_ROW_WINDOW*D_GABOR_COLS_WINDOW)

// Thinning
#define THIN_ONE                    0x01
#define THIN_TWO                    0x02
#define THIN_THREE                  0x04
#define THIN_FOUR                   0x08
#define THIN_FIVE                   0x10
#define THIN_SIX                    0x20
#define THIN_SEVEN                  0x40
#define THIN_EIGHT                  0x80

#define ANGLE_TRACE_LENGTH          13

// minutia
#define MAX_MINUTIAE                255
#define MAX_MINUTIAE_FOR_IBISDK     99
#define TEMPLATE_SIZE_INT           (MAX_MINUTIAE+2)

// core and delta
#define MAX_SINGULAR_CNT            16
#define MAX_TRACING_CNT             3000
#define MAX_TRACING_LINE_CNT        16
#define THRES_CLOSE_DISTANCE        30

#define D_FP_UNKNOWN_TYPE           0x00
#define D_FP_TENTED_ARCH_TYPE       0x01
#define D_FP_WHORL_TYPE             0x02
#define D_FP_LEFT_LOOP_TYPE         0x03
#define D_FP_RIGHT_LOOP_TYPE        0x04
#define D_FP_ARCH_TYPE              0x05
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
// defines for matching

//#define SAVE_MATCHING_INFO
//#define __ROC_TEST__
#define IBSM_MAX_MINUTIAE_SIZE      (255+2)

#define __SCT_USING_FASTMATCHING__              //신뢰성 있으면서 빠른 매칭
#define __SCT_HIGH_FASTMATCHING_DEVEL_MODE__
#define __SCT_THINNING_DEVEL_MODE__
#define __SCT_ANGLESORT_DEVEL_MODE__

#define WAS_186
#define MATCHING_USING_TABLE
//#define FVC_PENALTY
#define GLOBAL_MATCHING

#define D_NEIGHBOR  8
#ifdef MATCHING_USING_TABLE
    #define NMSmax 102400           //100*1024
    #define RP_THETTA   1121        //(1.0952380)*1024
#else
    #define NMSmax 100
    #define RP_THETTA   1.0952380
#endif

#define MAX_ANGLE 360
#define INT_FOR_ONE_NEIGHBOUR 1
#define INT_FOR_ONLY_MINUTIA 1
#define INT_FOR_ONE_MINUTIA_WOUT_NUMBERS (INT_FOR_ONLY_MINUTIA+INT_FOR_ONE_NEIGHBOUR*D_NEIGHBOR)
#define NUMBER_OF_MINUTIA_IN_INT 4
#define POINTS_OF_CYCLE 16
#define POINTS_OF_CYCLE2 16
#define INT_FOR_CYCLE (POINTS_OF_CYCLE/4+POINTS_OF_CYCLE2/4)
#define INT_FOR_NEIGBOURS_NUMBERS ((D_NEIGHBOR-1)/(NUMBER_OF_MINUTIA_IN_INT)+1)
#define INT_FOR_ONE_MINUTIA (INT_FOR_ONLY_MINUTIA+INT_FOR_ONE_NEIGHBOUR*D_NEIGHBOR+INT_FOR_NEIGBOURS_NUMBERS + INT_FOR_CYCLE)
#define BITS_FOR_NUMBER (32/NUMBER_OF_MINUTIA_IN_INT)
#define INT_FOR_MINUTIA_WOUT_CYCLES (INT_FOR_ONLY_MINUTIA+INT_FOR_ONE_NEIGHBOUR*D_NEIGHBOR+INT_FOR_NEIGBOURS_NUMBERS)
#define INT_FOR_FIRST_CYCLE (POINTS_OF_CYCLE/4)

#if (__SCT_PRODUCT_TYPE__ == __SCT_PRODUCT_ALGO_TEST__ && defined(__SCT_USING_AUTOMATCHING_TEST__))
#define D_WIDTH D_IMAGE_WIDTH
#define D_HEIGHT    D_IMAGE_HEIGHT
#define D_ANAL_SIZE 950
#define D_ROTATION_LIM  30      //default 30
#define FEATURE_SIZE     (9456)
#else
#define CUT_DISTANCE_FORCANDIDATE   30
#define D_WIDTH D_IMAGE_WIDTH
#define D_HEIGHT    D_IMAGE_HEIGHT
#define D_ANAL_SIZE 950
#define D_ROTATION_LIM  30              //default 30
#define D_NEIGHBOR_PASS_SCORE   147     // default 180
#define D_JOIN_TREENMS_LIM  D_NEIGHBOR_PASS_SCORE       //default 180
#define D_JOIN_ONENMS_LIM   70          //  default 70
#define D_NEIGHBOR_PASS_SCORE2  128     // default 180/140
#define FEATURE_SIZE     (9456)
#define D_DIRECT_MATCHING_SCORE 5000
#define D_DISTANCE_PENALTY  20
#define D_WAS               186
#endif

// FVC Matching
#define rMin    6                                       //default 6
#define rInc    (9/128.)                                //(9/128.)
#define angleMin    5                                   //defualt 5
#define pi  3.141592
#define angleMax    21                                  //defualt 21
#define Wr  36                                          //defualt 36....56이젤로조아///////////////////////////////////////////////////////
#define P_rMax  NMSmax*((100-Wr)/100.)/3
#define TETTA_LIM   14                                  //default 14
#define D_JOIN_TRACENMS_LIM 70      //default 65
////////////////////////////////////////////////////////////////////////////////////////////////


//#define __MAX_CALLBACK_COUNT__              12
#define __MAX_PIPE_DATA__                   4096
#define __MAX_TRACE_MESSAGE_SIZE__          (__MAX_PIPE_DATA__-128)

typedef struct tag_IB_POINT
{
    unsigned short x;
    unsigned short y;
} IB_POINT;

typedef struct tag_IB_RECT
{
    unsigned short left;
    unsigned short right;
    unsigned short top;
    unsigned short bottom;
} IB_RECT;

typedef struct tag_IB_DIRECTION
{
    unsigned short a[4];        // 0 : core-delta 방향, 1 : 검->회, 2 : 회->검, 3 : core 흐름
} IB_DIRECTION;

typedef struct tag_IB_LENGTH
{
    unsigned short l[3];
} IB_LENGTH;

typedef struct tag_IB_MEET
{
    unsigned short m[10];
} IB_MEET;

typedef struct tag_TRACING_PATTERN
{
    unsigned short p[12];
} IB_TRACING_PATTERN;

typedef struct tag_IB_CORE
{
    IB_POINT            Pos;
    IB_DIRECTION        Dir;
    IB_LENGTH           Len;
    IB_MEET             Meet;
    IB_TRACING_PATTERN  Pattern;
} IB_CORE;

typedef struct tag_IB_DELTA
{
    IB_POINT        Pos;
    IB_DIRECTION    Dir;
    IB_LENGTH       Len;
} IB_DELTA;

typedef struct tag_IBSM_SINGULAR
{
    unsigned char   NumCore;                    //  number of core
    unsigned char   NumDelta;                   //  number of delta
    unsigned char   DistCoreToDelta;            //  distance from core to delta
    unsigned char   FingerType;
    IB_CORE         Core[MAX_SINGULAR_CNT];         //  core quality
    IB_DELTA        Delta[MAX_SINGULAR_CNT];        //  delta quality
} IBSM_SINGULAR;

typedef struct tag_IB_TRACING
{
    unsigned char   NumPoints;                  //  number of core
    IB_POINT        Points[MAX_TRACING_CNT];    //  core quality
} IB_TRACING;


typedef struct tag_IBSM_MINUTIA
{
    unsigned int angle : 9;
    unsigned int y : 11;
    unsigned int x : 11;
    unsigned int type : 1;
} IBSM_MINUTIA;

typedef struct tag_IBISDK_MINUTIA
{
    unsigned int angle : 9;
    unsigned int y : 9;
    unsigned int x : 9;
    unsigned int reserve : 5;
} IBISDK_MINUTIA;

typedef struct tag_IB_EXTENDED_DATA
{
    unsigned char quality;
    unsigned char fingertype;
    unsigned char avg_pitch;
    unsigned char distance;
} IB_EXTENDED_DATA;

typedef struct tag_IBISDK_EXTENDED_DATA
{
    unsigned char       version : 4;
    unsigned char       quality : 4;
    unsigned char       fingertype;
    unsigned char       avg_pitch;
    unsigned char       distance;
} IBISDK_EXTENDED_DATA;

typedef struct tag_IBISDK_TEMPLATE
{
    unsigned int            NumberOfMinutiae;
    IBISDK_MINUTIA          Minutiae[99];
    IBISDK_EXTENDED_DATA    ExtData;

} IBISDK_TEMPLATE;

typedef struct tagNeighbor
{
    short dist[D_NEIGHBOR];
    short angle[D_NEIGHBOR];
    short thetta[D_NEIGHBOR];
    short minutiae_num[D_NEIGHBOR];
}Neighbor;

typedef struct tagFeatureVector
{
    int num;
    short x[MAX_MINUTIAE];
    short y[MAX_MINUTIAE];
    short angle[MAX_MINUTIAE];
    unsigned char type[MAX_MINUTIAE];
    Neighbor neighbor_info[MAX_MINUTIAE];
    unsigned char feature_ver;
    unsigned char core_type;
    unsigned char pitch;
    unsigned char quality;
    unsigned char core_dist;
}FeatureVector;

typedef struct tagMatchedNeighbor_Pair
{
    short Ref_Neighbor[D_NEIGHBOR];
    short Inp_Neighbor[D_NEIGHBOR];
    int Score[D_NEIGHBOR];
    int NMS;
    int MatchedCount;
}MatchedNeighbor_Pair;

typedef struct tagMatchedNeighbor
{
    MatchedNeighbor_Pair Matched_Neighbor[MAX_MINUTIAE];
}MatchedNeighbor;

typedef struct tagCANDIDATE_Pair
{
    short Ref_Minutiae[MAX_MINUTIAE];
    short Inp_Minutiae[MAX_MINUTIAE];
    int Pair_Count;
}CANDIDATE_Pair;

typedef struct tag_IB_TEMPLATE
{
    unsigned char       Version;                    // 0x10
    unsigned short      CaptureDeviceTypeID;        // watson id
    unsigned short      ImageSamplingX;             // watson --> 500 dpi
    unsigned short      ImageSamplingY;             // watson --> 500 dpi
    unsigned short      ImageSizeX;                 // watson --> 800
    unsigned short      ImageSizeY;                 // watson --> 750
    unsigned char       QualityScore;               // aware ?
    unsigned char       QualityAlgVendorID;         // aware ?
    unsigned char       QualityAlgID;               // aware ?
    unsigned char       FingerPosition;
    unsigned char       ImpressionType;
    unsigned char       NumberOfMinutiae;           // 0 ~ 255
    IBSM_MINUTIA            Minutiae[MAX_MINUTIAE];
    IB_EXTENDED_DATA    ExtData;
    unsigned char       Reserved1;                  // not used
} IB_TEMPLATE;


typedef struct tag_IBSM_IMAGE
{
    unsigned char       Version;                    // 0x10
    unsigned short      CaptureDeviceTypeID;        // watson id
    unsigned short      ScanSamplingX;              // watson --> 680 dpi
    unsigned short      ScanSamplingY;              // watson --> 680 dpi
    unsigned short      ImageSamplingX;             // watson --> 500 dpi
    unsigned short      ImageSamplingY;             // watson --> 500 dpi
    unsigned short      ImageSizeX;                 // watson --> 800
    unsigned short      ImageSizeY;                 // watson --> 750
    unsigned char       ScaleUnit;                  // inches --> 0x01
    unsigned char       BitDepth;
    unsigned char       FingerPosition;
    unsigned char       ImpressionType;
    unsigned char       ImageCompressionAlg;
    unsigned int        ImageDataLength;
    unsigned char       ImageData[MAX_IMAGE_SIZE];
    unsigned char       Reserved1;                  // not used
} IBSM_IMAGE;

typedef struct tag_IBSM_MATCHING_INFO
{
    unsigned char       MatchedCount;
    IBSM_MINUTIA            MatchedMinutiae[MAX_MINUTIAE];
    unsigned short      MatchedScore[MAX_MINUTIAE];
}IBSM_MATCHING_INFO;

// Raw pixel format
#define RPIX_IDENTIFIER "RPIX"

#define RPIX_HEADERLENGTH 30

#define RPIX_MAJOR_VERSION 1
#define RPIX_MINOR_VERSION 0

#define RPIX_COMPRESSION_UNDEFINED 0
#define RPIX_COMPRESSION_NONE 1
#define RPIX_COMPRESSION_CCITT_FAX_G3 2
#define RPIX_COMPRESSION_CCITT_FAX_G4 3
#define RPIX_COMPRESSION_DEFAULT RPIX_COMPRESSION_NONE

#define RPIX_PIXEL_ORDER_UNDEFINED 0
#define RPIX_PIXEL_ORDER_NORMAL 1
#define RPIX_PIXEL_ORDER_REVERSE 2
#define RPIX_PIXEL_ORDER_DEFAULT RPIX_PIXEL_ORDER_NORMAL

#define RPIX_SCANLINE_ORDER_UNDEFINED 0
#define RPIX_SCANLINE_ORDER_NORMAL 1
#define RPIX_SCANLINE_ORDER_INVERSE 2
#define RPIX_SCANLINE_ORDER_DEFAULT RPIX_SCANLINE_ORDER_NORMAL

#define RPIX_INTERLEAVING_UNDEFINED 0
#define RPIX_INTERLEAVING_BIP 1
#define RPIX_INTERLEAVING_BIL 2
#define RPIX_INTERLEAVING_BSQ 3
#define RPIX_INTERLEAVING_DEFAULT RPIX_INTERLEAVING_BIP

#define RPIX_CHANNEL_UNDEFINED 0

typedef struct tag_RawPixelHeader
{
    unsigned char identifier[4]; /* Always "RPIX" */

    unsigned long   hdrlength; /* Length of this header in bytes */
    /* Including the hdrlength field */
    /* Not including the identifier field */
    /* &k.hdrlength + k.hdrlength = pixels */

    unsigned char majorversion; /* Major revision # of RPIX format */
    unsigned char minorversion; /* Minor revision # of RPIX format */

    unsigned long width;  /* Image width in pixels */
    unsigned long height; /* Image height in pixels */
    unsigned char comptype;   /* Compression (none, FAXG3, FAXG4, ... ) */
    unsigned char pixelorder; /* Pixel order */
    unsigned char scnlorder;  /* Scanline order */
    unsigned char interleave; /* Interleaving (BIP/BIL/BSQ) */

    unsigned char numbands; /* Number of bands in image (1-255) */
    unsigned char rchannel; /* Default red channel assignment */
    unsigned char gchannel; /* Default green channel assignment */
    unsigned char bchannel; /* Default blue channel assignment */
    /* Grayscale images are encoded in R */
    /* The first band is 1, not 0 */
    /* A value of 0 means "no band" */

    unsigned char reserved[8]; /* For later use */
}RawPixelHeader;

typedef enum enum_IBSM_ProcessedImageType
{
    ENUM_IBSM_PROCESSED_IMAGE_TYPE_GABOR=0x00,
    ENUM_IBSM_PROCESSED_IMAGE_TYPE_BINARY,
    ENUM_IBSM_PROCESSED_IMAGE_TYPE_THIN
}
IBSM_ProcessedImageType;

typedef enum enum_IBSM_ScaleUnit
{
    IBSM_SCALE_UNIT_INCH=0x01,
    IBSM_SCALE_UNIT_CENTIMETER=0x02
}
IBSM_ScaleUnit;

typedef enum enum_IBSM_TemplateVersion
{
    IBSM_TEMPLATE_VERSION_IBISDK_0=0x00,    // TEMPLATE_OLD_VERSION
    IBSM_TEMPLATE_VERSION_IBISDK_1,         // TEMPLATE_MIX_VERSION
    IBSM_TEMPLATE_VERSION_IBISDK_2,         // TEMPLATE_FAST_VERSION
    IBSM_TEMPLATE_VERSION_IBISDK_3,         // Secuest 1nd Algorithm
    IBSM_TEMPLATE_VERSION_NEW_0=0x10        // IBSM_NEW_TEMPLATE
}
IBSM_TemplateVersion;

typedef struct tag_IBSM_Template
{
    IBSM_TemplateVersion        Version;
    DWORD				        FingerPosition;
    IBSM_ImpressionType         ImpressionType;
    IBSM_CaptureDeviceTechID    CaptureDeviceTechID;
    unsigned short              CaptureDeviceVendorID;  // IBIA에 등록하면 받는다.
    unsigned short              CaptureDeviceTypeID;    // WATSON : 0x0001, SHERLOCK : 0x0010, WATSON_MINI : 0x0020, COLUMBO : 0x0030, HOLMES : 0x0040
    unsigned short              ImageSamplingX;         // 500 dpi
    unsigned short              ImageSamplingY;         // 500 dpi
    unsigned short              ImageSizeX;             // 352
    unsigned short              ImageSizeY;             // 288
    unsigned int                Minutiae[IBSM_MAX_MINUTIAE_SIZE];
    unsigned int                Reserved;
}
IBSM_Template;

typedef struct tagIBSU_TemplateDB
{
    BOOL			isUsed;
    IBSU_ImageType  imageType;
    IBSM_Template	_template;
}
IBSU_TemplateDB;
#endif


