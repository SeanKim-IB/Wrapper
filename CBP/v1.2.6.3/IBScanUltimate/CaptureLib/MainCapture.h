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

#ifndef __CAPTURECORE_H__
#define __CAPTURECORE_H__

#if defined( _MSC_VER ) && ( _MSC_VER >= 1020 )
	#pragma once
#endif

#if defined(__IBSCAN_ULTIMATE_SDK__)
#include "IBScanUltimateApi.h"
#elif defined(__IBSCAN_SDK__)
#include "IBScanApi.h"
#endif
#include "ReservedApi.h"
#include "MultiThreadSync.h"
#include "IBSU_Global_Def.h"
#if defined(_WINDOWS) && defined(__USE_WINUSB_DRIVER__)
#include "WinUSBInterface.h"
#include "WinUSBInterface_Curve.h"
#endif
#include "IBAlgorithm.h"
#include "../MatcherLib/IBSMAlgorithm.h"

#ifdef _WINDOWS
#include "nex_sdk.h"
#endif

#if defined(__ppi__)
#include "Parallel.h"
#endif

#ifdef __linux__
#include <vector>
#ifdef __libusb_latest__
#include <libusb.h>
#else
#include <usb.h>
#endif
#endif

#ifdef __USE_LIBUSBX_DRIVER__
/* Disable: warning C4200: nonstandard extension used : zero-sized array in struct/union */
#pragma warning(disable:4200)
#include <..\..\Driver\WM\LibUSBx\libusb\libusb.h>
#endif

#include <map>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <time.h>

#ifdef _WINDOWS
// Constants to change epoch time
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

// for timezone
struct timezone
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};
#endif

#ifndef		ASSERT_RETURN
	#define ASSERT_RETURN( x )	{ /*ASSERT( 0 ); */return x; }
#endif

/////////////////////////////////////////////////////////////////////////////
//class CMainCapture;

class CIBScanUltimateApp;
class CIBUsbManager;

class CMainCapture : public CMultiThreadSync<CMainCapture>
{
public:	// Construction/destruction:

	CMainCapture(CIBScanUltimateApp *dlg, CIBUsbManager *dlgUsbManager);
	virtual			~CMainCapture(void);

public:	// Implementation:

	int				SaveBitmapImage(char *filename, unsigned char *buffer, int width, int height, int invert);

	int				Main_Initialize(const int deviceIndex, const BOOL bAsyncOpenDevice, LPCSTR uniformityMaskPath, BOOL useUniformityMask=TRUE, BOOL bReserved=FALSE, BOOL bPPIMode=FALSE);
	int				Main_Release(BOOL bReleaseMark);
	int				Main_GetDeviceInfo(const int deviceIndex, IBSU_DeviceDescA *deviceInfo, BOOL bPPIMode=FALSE);
	int				Main_GetProperty(const IBSU_PropertyId propertyId, LPSTR propertyValue);
	int				Main_SetProperty(const IBSU_PropertyId propertyId, LPCSTR propertyValue, BOOL bRserved=FALSE, ReservedAccessLevel nAcessLevel=RESERVED_ACCESS_NO);
	int				Capture_Start(const IBSU_ImageType imageType, const IBSU_ImageResolution imageResolution, const DWORD captureOptions, BOOL bReserved=FALSE, BOOL bRawCapture=FALSE);
	int				Capture_Abort();
	int				Capture_SetMode(const IBSU_ImageType imageType, const IBSU_ImageResolution imageResolution,
									/*const IBSU_ImageOrientation lineOrder, */const DWORD captureOptions,
									int *resultWidth, int *resultHeight,
									int	*baseResolutionX, int *baseResolutionY);

	int				Capture_IsModeAvailable(const IBSU_ImageType imageType, const IBSU_ImageResolution imageResolution, BOOL *isAvailable);
	int				Capture_IsActive(BOOL *isActive);
	int				Capture_TakeResultImageManually(BOOL isSet);

	int				Capture_GetContrast(int *contrastValue);
	int				Capture_SetContrast(int contrastValue);
	int				Capture_SetLEVoltage(int voltageValue);
	int				Capture_SetLEVoltage_Watson(int voltageValue);
	int				Capture_SetLEVoltage_WatsonMini(int voltageValue);
	int				Capture_SetLEVoltage_Sherlock(int voltageValue);
	int				Capture_SetLEVoltage_Columbo(int voltageValue);
	int				Capture_SetLEVoltage_Curve(int voltageValue);
	int				Capture_SetLEVoltage_CurveSETi(int voltageValue);
	int				Capture_SetLEVoltage_Holmes(int voltageValue);
	int				Capture_SetLEVoltage_Kojak(int voltageValue);
	int				Capture_SetLEVoltage_Five0(int voltageValue);
	int				Capture_GetLEVoltage(int *voltageValue);
	int				Capture_GetLEVoltage_Watson(int *voltageValue);
	int				Capture_GetLEVoltage_WatsonMini(int *voltageValue);
	int				Capture_GetLEVoltage_Sherlock(int *voltageValue);
	int				Capture_GetLEVoltage_Columbo(int *voltageValue);
	int				Capture_GetLEVoltage_Curve(int *voltageValue);
	int				Capture_GetLEVoltage_CurveSETi(int *voltageValue);
	int				Capture_GetLEVoltage_Holmes(int *voltageValue);
	int				Capture_GetLEVoltage_Kojak(int *voltageValue);
	int				Capture_GetLEVoltage_Five0(int *voltageValue);

	int				Capture_GetLEOperationMode(IBSU_LEOperationMode *leOperationMode);
	int				Capture_SetLEOperationMode(IBSU_LEOperationMode leOperationMode);

	int				Reserved_GetEnhancedImage(const IBSU_ImageData inImage, IBSU_ImageData *enhancedImage,
									 int *segmentImageArrayCount, IBSU_ImageData *segmentImageArray, IBSU_SegmentPosition *segmentPositionArray);
	int				Reserved_GetFinalImageByNative(IBSU_ImageData *finalImage);
	int				Reserved_CheckWetFinger(const IBSU_ImageData inImage);
	int				Reserved_GetImageWidth(const BYTE *inImage, const int width, const int height, int *outMM);
	
	//	for CBP
	int				Reserved_CBP_GetPreviewInfo(IBSU_SegmentPosition *segmentPositionArray, IBSU_SegmentPosition *segmentPositionArray_for_geo, int *segmentPositionArrayCount, BOOL *IsFingerDetected);
	int				Reserved_CBP_CleanUp();
	int				Reserved_CBP_IsFingerOn(BOOL *bIsFingerOn);

	int				RegisterCallback(CallbackType clbkIndex, void *callback, void *context);

	int				Control_GetTouchInStatus(int *touchInStatus);
	int				Control_GetAvailableLEDs(IBSU_LedType *ledType, int *ledCount, DWORD *availableLEDs);
	int				Control_GetActiveLEDs(DWORD *activeLEDs, BOOL useReservedLed);
	int				Control_SetActiveLEDs(DWORD activeLEDs, BOOL useReservedLed);
	int				Control_GetAvailableBeeper(IBSU_BeeperType *pBeeperType);
	int				Control_SetBeeper(const IBSU_BeepPattern beepPattern, const DWORD soundTone, const DWORD duration, const DWORD reserved_1, const DWORD reserved_2);

	int				ClientWindow_Create(const IBSU_HWND hWindow, const DWORD left, const DWORD top, const DWORD right, const DWORD bottom, BOOL bReserved=FALSE, DWORD imgWidth=0, DWORD imgHeight=0);
    int             _SetDisplayWindowParam(int imgWidth, int imgHeight, int *displayImgX, int *displayImgY, int *displayImgWidht, int *displayImgHeight);
	int				ClientWindow_Destroy(const BOOL clearExistingInfo);
	int				ClientWindow_GetProperty(const IBSU_ClientWindowPropertyId propertyId, LPSTR propertyValue);
	int				ClientWindow_SetProperty(const IBSU_ClientWindowPropertyId propertyId, LPCSTR propertyValue);
	int				ClientWindow_SetOverlayText(const char *fontName, const int fontSize, const BOOL fontBold,
										 const char *text, const int posX, const int posY, const DWORD textColor);
#if defined(__IBSCAN_SDK__)
	int				Capture_GetMode(IBSU_ImageType *imageType, IBSU_ImageResolution *imageResolution, DWORD *captureOptions, const int numberOfObjects);
	int				RawCapture_Abort();
	int				RawCapture_GetOneFrameImage();
	int				RawCapture_TakePreviewImage(const RawImageData rawImage);
	int				RawCapture_TakeResultImage(const RawImageData rawImage);
	int				Capture_SetDACRegister(int dacValue);
#endif
	int				Capture_BGetImage(IBSU_ImageData *pImage, IBSU_ImageType *pImageType,
									IBSU_ImageData *pSplitImageArray, int *pSplitImageArrayCount,
									IBSU_FingerCountState *pFingerCountState, IBSU_FingerQualityState *pQualityArray,
									int *pQualityArrayCount);
	int				Capture_BGetImageEx(int *pImageStatus, IBSU_ImageData *pImage,
									IBSU_ImageType *pImageType, int *pDetectedFingerCount,
									IBSU_ImageData *pSegmentImageArray, IBSU_SegmentPosition *pSegmentPositionArray,
									int *pSegmentImageArrayCount, IBSU_FingerCountState *pFingerCountState,
									IBSU_FingerQualityState *pQualityArray, int *pQualityArrayCount);
	int				Main_BGetInitProgress(BOOL *pIsComplete, int *pHandle, int *pProgressValue);
	int				Capture_BGetClearPlatenAtCapture(IBSU_PlatenState *pPlatenState);
	int				Capture_BGetRollingInfo(IBSU_RollingState *pRollingState, int *pRollingLineX);
	int				Capture_BGetRollingInfoEx(IBSU_RollingState *pRollingState, int *pRollingLineX, int *pRollingDirection, int *pRollingWidth);
	int				Capture_GetIBSM_ResultImageInfo(IBSM_FingerPosition fingerPosition,
													IBSM_ImageData *pResultImage,
													IBSM_ImageData *pSplitResultImage,
													int            *pSplitResultImageCount);
	int				Capture_CheckWetFinger(const IBSU_ImageData inImage);
	int				Capture_GetImageWidth(const BYTE *inImage, const int width, const int height, int *Width_MM);
	int				Capture_GetNFIQScore(const BYTE *imgBuffer,const DWORD width, const DWORD height, const BYTE BitsPerPixel, int *pScore);
    int             ClientWindow_ShowOverlayObject(const int overlayHandle, const BOOL show);
    int             ClientWindow_ShowAllOverlayObject(const BOOL show);
    int             ClientWindow_RemoveOverlayObject(const int overlayHandle);
    int             ClientWindow_RemoveAllOverlayObject();
    int             ClientWindow_AddOverlayText(int *pOverlayHandle, const char *fontName, const int fontSize, const BOOL fontBold,
										 const char *text, const int posX, const int posY, const DWORD color);
    int             ClientWindow_ModifyOverlayText(const int overlayHandle, const char *fontName, const int fontSize, const BOOL fontBold,
										 const char *text, const int posX, const int posY, const DWORD color);
    int             ClientWindow_AddOverlayLine(int *pOverlayHandle, const int x1, const int y1,
                                         const int x2, const int y2, const int width, const DWORD color);
    int             ClientWindow_ModifyOverlayLine(const int overlayHandle, const int x1, const int y1,
                                         const int x2, const int y2, const int width, const DWORD color);
    int             ClientWindow_AddOverlayQuadrangle(int *pOverlayHandle, const int x1, const int y1,
                                         const int x2, const int y2, const int x3, const int y3, 
                                         const int x4, const int y4, const int width, const DWORD color);
    int             ClientWindow_ModifyOverlayQuadrangle(const int overlayHandle, const int x1, const int y1,
                                         const int x2, const int y2, const int x3, const int y3, 
                                         const int x4, const int y4, const int width, const DWORD color);
	int				ClientWindow_AddOverlayShape(int *pOverlayHandle, const IBSU_OverlayShapePattern shapePattern,
							const int x1, const int y1, const int x2, const int y2, const int width, const DWORD color,
							const int reserved_1, const int reserved_2);
	int				ClientWindow_ModifyOverlayShape(const int overlayHandle, const IBSU_OverlayShapePattern shapePattern,
							const int x1, const int y1, const int x2, const int y2, const int width, const DWORD color,
							const int reserved_1, const int reserved_2);
	int				ClientWindow_Redraw(const DWORD flags);

	// For Windows
	int				_Main_Release(BOOL bReleaseMark, BOOL bLibraryTerminating = FALSE);
	void			_InitializeCaptureVariables();
	BOOL			_SetInitializeDeviceDesc(PropertyInfo *pPropertyInfo, UsbDeviceInfo *pScanDevDesc);

	BOOL			_GetOneFrameImage(unsigned char *Image, int ImgSize);
	BOOL			_GetOneFrameImage_Watson(unsigned char *Image, int ImgSize);
	BOOL			_GetOneFrameImage_Sherlock(unsigned char *Image, int ImgSize);
	BOOL			_GetOneFrameImage_Columbo(unsigned char *Image, int ImgSize);
	BOOL			_GetOneFrameImage_Curve(unsigned char *Image, int ImgSize);
	BOOL			_GetOneFrameImage_CurveSETi(unsigned char *Image, int ImgSize);
	BOOL			_GetOneFrameImage_Holmes(unsigned char *Image, int ImgSize);
	BOOL			_GetOneFrameImage_Kojak(unsigned char *Image, int ImgSize);
	BOOL			_GetOneFrameImage_Five0(unsigned char *Image, int ImgSize);

	// For the FPGA driver control in Windows
//	BOOL			_GetDeviceNameByIndex(LPGUID pGuid, DWORD dwDeviceNumber, char *outNameBuf);
	int 			_UsbBulkOutIn(int outEp, UCHAR uiCommand, UCHAR *outData, LONG outDataLen,
							  int inEp, UCHAR *inData, LONG inDataLen, LONG *nBytesRead, int timeout = __BULK_TIMEOUT__);
#if defined(__ppi__)	
	int				_PPI_OneFrameBulkRead(UCHAR *data, LONG dataLen, LONG *nBytesRead, int timeout = __BULK_TIMEOUT__);
	int				_PPI_MaskBulkRead(UCHAR *data, LONG dataLen, LONG *nBytesRead, int timeout = __BULK_TIMEOUT__);
#endif
	int				_UsbBulkIn(int ep, UCHAR *data, LONG dataLen, LONG *nBytesRead, int timeout = __BULK_TIMEOUT__);
	int			    _UsbBulkOut(int ep, UCHAR uiCommand, UCHAR *data, LONG dataLen, int timeout = __BULK_TIMEOUT__);
	int			    _UsbBulkOut_Low(int ep, UCHAR uiCommand, UCHAR *data, LONG dataLen, int timeout = __BULK_TIMEOUT__);
	int				_ResetFifo();
	int				_Seti_SetRegister(UCHAR adr, UCHAR val);
	int				_CurveSETi_SetRegister(UCHAR adr, UCHAR val);
	int				_CurveSETi_GetRegister(UCHAR adr, UCHAR *val);
	int				_Hynix_SetRegister(UCHAR adr, UCHAR val);
	int				_Hynix_GetRegister(UCHAR adr, UCHAR *val);
	int				_Hynix_SetImageSize(int x, int y, int width, int height);
	int				_Hynix_ReadDeviceStatus(UCHAR *val);
	int				_Hynix_ChangeOperationMode(UCHAR mode);
	int				_Hynix_ResetFifo();
	int				_Hynix_WriteEEPROM(WORD addr, const BYTE *buf, int len);
	int				_Hynix_ReadEEPROM(WORD addr, BYTE *buf, int len);

	int				_FPGA_SetRegister(UCHAR adr, UCHAR val);
	int				_FPGA_GetRegister(UCHAR adr, UCHAR *val, int timeout = __BULK_TIMEOUT__ );
	int				_MT9M_SetRegister(WORD adr, WORD val);
	int				_MT9M_GetRegister(WORD adr, WORD *val);
	int 			_InitializeForCISRegister(WORD rowStart, WORD colStart, WORD width, WORD height, BOOL bEEPROM_Read = TRUE);
	int			    _InitializeForCISRegister_Watson(WORD rowStart, WORD colStart, WORD width, WORD height);
	int		    	_InitializeForCISRegister_Sherlock(WORD rowStart, WORD colStart, WORD width, WORD height);
	int		    	_InitializeForCISRegister_Columbo(WORD rowStart, WORD colStart, WORD width, WORD height);
	int		    	_InitializeForCISRegister_Curve(WORD rowStart, WORD colStart, WORD width, WORD height);
	int		    	_InitializeForCISRegister_CurveSETi(WORD rowStart, WORD colStart, WORD width, WORD height);
	int		    	_InitializeForCISRegister_Holmes(WORD rowStart, WORD colStart, WORD width, WORD height);
	int		    	_InitializeForCISRegister_Full_Kojak(WORD rowStart, WORD colStart, WORD width, WORD height);
	int		    	_InitializeForCISRegister_Half_Kojak(WORD rowStart, WORD colStart, WORD width, WORD height);
	int		    	_InitializeForCISRegister_Five0(WORD rowStart, WORD colStart, WORD width, WORD height);
	int				_SndUsbFwInitialize();
	int				_SndUsbFwCaptureStart();
	int				_SndUsbFwCaptureStop();
	int				_SndUsbFwRelease();
	int				_SndUsbFwCaptureStop_atLibraryTerminated();
	int				_SndUsbFwRelease_atLibraryTerminated();
	int 			_SndUsbFwGotoSleep();
	int				_SndUsbFwInitLEVoltage();
	int				_GetProperty( DEV_INFO_ADDRESS addr, BYTE *inBuffer);
	int				_SetProperty( DEV_INFO_ADDRESS addr, BYTE *Buffer, size_t Count);
	int				_GetLEOperationMode( WORD addr, WORD *val);
	int				_SetLEOperationMode( WORD addr);
	int				_SetLEOperationMode_Watson( WORD addr);
	int				_SetLEOperationMode_Sherlock( WORD addr);
	int				_SetLEOperationMode_Columbo( WORD addr);
	int				_SetLEOperationMode_Curve( WORD addr);
	int				_SetLEOperationMode_CurveSETi( WORD addr);
	int				_SetLEOperationMode_Holmes( WORD addr);
	int				_SetLEOperationMode_Kojak( WORD addr);
	int				_SetLEOperationMode_Five0( WORD addr);
	int				_GetTouchInStatus(WORD *val);
	int				_GetActiveLEDs(DWORD *val);
	int				_SetActiveLEDs( WORD addr, DWORD val);
	int				_SetActiveLEDs_Kojak( WORD addr, DWORD val);
	int				_GetLEVoltage( int *voltageValue);
	int				_SetLEVoltage( int voltageValue);

	void			_PostCallback(CallbackType clbkIndex, BYTE *OutImg = NULL, double frameTime = 0, int progressValue = 0, int warningCode = 0, BOOL bIsFinal = FALSE);
	void			_PostTraceLogCallback(int error, const char* format, ...);

	int				_LoadLinearity();
	BYTE			_CalcCheckSum(unsigned char *data, int data_count);
	int				_ReadByteFromEEPROM(UCHAR regAddr, UCHAR *regVal);

//	BOOL			_Process_AutoCaptureByGain(BYTE *InImg, int maxBright, int minBright);
//	BOOL			_Process_AutoCaptureByVoltage(BYTE *InImg, int maxBright, int minBright);
//	void			_Process_AnalysisImage(unsigned char *InImg);

    int             _CheckForClearPlaten(BYTE *InImg);
	int				_GoJob_DummyCapture_ForThread(BYTE *InImg);
	int				_GoJob_DummyCapture_ForThread_Watson(BYTE *InImg);
	int				_GoJob_DummyCapture_ForThread_Sherlock(BYTE *InImg);
	int				_GoJob_DummyCapture_ForThread_Columbo(BYTE *InImg);
	int				_GoJob_DummyCapture_ForThread_Curve(BYTE *InImg);
	int				_GoJob_DummyCapture_ForThread_CurveSETi(BYTE *InImg);
	int				_GoJob_DummyCapture_ForThread_Holmes(BYTE *InImg);
	int				_GoJob_DummyCapture_ForThread_Kojak(BYTE *InImg);
#ifdef __DUMMY_DETECT_FINGER_USING_ATOF__
	int				_GoJob_DummyCapture_ForThread_Kojak_ATOF(BYTE *InImg);
#endif
	int				_GoJob_DummyCapture_ForThread_Five0(BYTE *InImg);

    AcuisitionState _GoJob_PreImageProcessing_ForOneFrame(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage);

	AcuisitionState	_GoJob_PreviewImage_ForThread(BYTE *InImg);
	AcuisitionState	_GoJob_PreImageProcessing_Watson(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage);
	AcuisitionState	_GoJob_PreImageProcessing_Sherlock(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage);
	AcuisitionState	_GoJob_PreImageProcessing_Columbo(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage);
	AcuisitionState	_GoJob_PreImageProcessing_Curve(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage);
	AcuisitionState	_GoJob_PreImageProcessing_CurveSETi(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage);
	AcuisitionState	_GoJob_PreImageProcessing_Holmes(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage);
	AcuisitionState	_GoJob_PreImageProcessing_Kojak(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage);
	AcuisitionState	_GoJob_PreImageProcessing_Kojak_CurrentSensor(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage);
	AcuisitionState	_GoJob_PreImageProcessing_Kojak_CurrentSensor_and_ATOF(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage);
	AcuisitionState	_GoJob_PreImageProcessing_Five0(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage);

#if defined(__IBSCAN_SDK__)
	AcuisitionState	_GoJob_OneFrameRawImage_ForThread(BYTE *InImg);
	AcuisitionState	_GoJob_TakePreviewImage_ForThread(BYTE *InImg);
	AcuisitionState	_GoJob_TakeResultImage_ForThread(BYTE *InImg);
#endif
	void			_GoJob_Initialize_ForThread();
	void			_GoJob_Initialize_ForThread_Watson();
	void			_GoJob_Initialize_ForThread_Sherlock();
	void			_GoJob_Initialize_ForThread_Columbo();
	void			_GoJob_Initialize_ForThread_Curve();
	void			_GoJob_Initialize_ForThread_CurveSETi();
	void			_GoJob_Initialize_ForThread_Holmes();
	void			_GoJob_Initialize_ForThread_Kojak();
	void			_GoJob_Initialize_ForThread_Five0();
	BOOL			_GoJob_PostImageProcessing(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage);
	BOOL			_GoJob_PostImageProcessing_Default(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage);
	BOOL			_GoJob_PostImageProcessing_Holmes(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage);
	BOOL			_GoJob_PostImageProcessing_Kojak(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage);
	BOOL			_GoJob_PostImageProcessing_Five0(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage);

	void			_PostImageProcessing_ForResult(BYTE *InImg, BYTE *OutImg);
	void			_PostImageProcessing_ForResult_Watson(BYTE *InImg, BYTE *OutImg);
	void			_PostImageProcessing_ForResult_Sherlock(BYTE *InImg, BYTE *OutImg);
	void			_PostImageProcessing_ForResult_Columbo(BYTE *InImg, BYTE *OutImg);
	void			_PostImageProcessing_ForResult_Curve(BYTE *InImg, BYTE *OutImg);
	void			_PostImageProcessing_ForResult_CurveSETi(BYTE *InImg, BYTE *OutImg);
	void			_PostImageProcessing_ForResult_Holmes(BYTE *InImg, BYTE *OutImg);
	void			_PostImageProcessing_ForResult_Kojak(BYTE *InImg, BYTE *OutImg);
	void			_PostImageProcessing_ForResult_Five0(BYTE *InImg, BYTE *OutImg);

	void			_PostImageProcessing_ForPreview(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage);
	void			_PostImageProcessing_ForPreview_Watson(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage);
	void			_PostImageProcessing_ForPreview_Sherlock(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage);
	void			_PostImageProcessing_ForPreview_Columbo(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage);
	void			_PostImageProcessing_ForPreview_Curve(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage);
	void			_PostImageProcessing_ForPreview_CurveSETi(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage);
	void			_PostImageProcessing_ForPreview_Holmes(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage);
	void			_PostImageProcessing_ForPreview_Kojak(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage);
	void			_PostImageProcessing_ForPreview_Five0(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage);

#if defined(__IBSCAN_SDK__)
	int				_PostImageProcessing_ForRolling(unsigned char *InImg, int frameCount, BOOL *isComplete);
#endif

	int				Reserved_GetEnhancdImage_Watson(const IBSU_ImageData inImage,
												   IBSU_ImageData *enhancedImage,
												   int *segmentImageArrayCount,
												   IBSU_ImageData *segmentImageArray,
												   IBSU_SegmentPosition *segmentPositionArray);
	int				Reserved_GetEnhancdImage_Sherlock(const IBSU_ImageData inImage,
												   IBSU_ImageData *enhancedImage,
												   int *segmentImageArrayCount,
												   IBSU_ImageData *segmentImageArray,
												   IBSU_SegmentPosition *segmentPositionArray);
	int				Reserved_GetEnhancdImage_Columbo(const IBSU_ImageData inImage,
												   IBSU_ImageData *enhancedImage,
												   int *segmentImageArrayCount,
												   IBSU_ImageData *segmentImageArray,
												   IBSU_SegmentPosition *segmentPositionArray);

	int				Reserved_GetFinalImageByNative_Columbo(IBSU_ImageData *finalImage);
	int				Reserved_GetFinalImageByNative_Kojak(IBSU_ImageData *finalImage);
	BOOL			_UM_WAIT_BUSY();
	BOOL			_UM_STATUS_READ(BYTE *data);
	BOOL			_UM_CONT_READ_FLASH(unsigned char *buf, LONG buf_count, unsigned int start_address, unsigned int page);
	BYTE			_UM_MAKE_CHECKSUM(unsigned char *mask_buf, LONG mask_buf_size);
	BOOL			_IsAlive_UsbDevice();


	BOOL			_OpenUSBDevice(int deviceIndex, int *errorNo);
	void			_CloseUSBDevice();

	// support new types of calibration data
	int				_Read_Calibration_Data_From_EEPROM();
	char			_Calibration_Data_Checksum(char *data, unsigned char length);

#if defined(__ppi__)
    int             _GoJob_ParseUSBBulkOut(UCHAR uiCommand, UCHAR *outData, LONG outDataLen);
    int             _GoJob_ParseUSBBulkIn(UCHAR uiCommand, UCHAR *outData, LONG outDataLen, UCHAR *inData, LONG inDataLen, LONG *nBytesRead);
#endif

    void			_SetPID_VID(UsbDeviceInfo *pScanDevDesc, char *productID);
	int				_GetPID();

	void			_ClearOverlayText();
	OverlayText*	_FindOverlayTextInList(int x, int y);
	OverlayTextEx*	_FindOverlayTextExInList(const int overlayHandle);
	OverlayLineEx*	_FindOverlayLineExInList(const int overlayHandle);
	OverlayQuadrangleEx*	_FindOverlayQuadrangleExInList(const int overlayHandle);
	OverlayShapeEx* _FindOverlayShapeExInList(const int overlayHandle);
	void			_DrawClientWindow(BYTE *InImg, DisplayWindow dispWindow,
									std::vector<OverlayText *>*overlayText,
									std::vector<OverlayTextEx *>*overlayTextEx,
									std::vector<OverlayLineEx *>*overlayLineEx,
									std::vector<OverlayQuadrangleEx *>*overlayQuadrangleEx,
									std::vector<OverlayShapeEx *>*overlayShapeEx,
									const DWORD flags = 0x00000000);
	void			_ReDrawClientWindow_Overlays(BYTE *InImg, DisplayWindow dispWindow);
#ifdef _WINDOWS
    void            _DrawClientWindow_Text(HDC hMemDc, DisplayWindow dispWindow, std::vector<OverlayText *>*overlayObjects);
    void            _DrawClientWindow_TextEx(HDC hMemDc, DisplayWindow dispWindow, std::vector<OverlayTextEx *>*overlayObjects);
    void            _DrawClientWindow_LineEx(HDC hMemDc, DisplayWindow dispWindow, std::vector<OverlayLineEx *>*overlayObjects);
    void            _DrawClientWindow_QuadrangleEx(HDC hMemDc, DisplayWindow dispWindow, std::vector<OverlayQuadrangleEx *>*overlayObjects);
    void            _DrawClientWindow_ShapeEx(HDC hMemDc, DisplayWindow dispWindow, std::vector<OverlayShapeEx *>*overlayObjects);

	void			_DrawClientWindow_DispInvalidArea(HDC hMemDc, DisplayWindow dispWindow);
	void			_DrawClientWindow_RollGuideLine(HDC hMemDc, DisplayWindow dispWindow);
#endif
	DWORD			_GetThreadID();
	int				_OpenDeviceThread();
#if defined(_WINDOWS)
	int				_DestroyDeviceThread(CWinThread **thread, HANDLE *destroyEvent, BOOL *pAliveThread, ThreadType type);
    int             _CreateDeviceThread(CWinThread **thread, ThreadType type);
#elif defined(__linux__)
	int				_DestroyDeviceThread(pthread_t *thread, BOOL *destroyEvent, BOOL *pAliveThread, ThreadType type);
    int             _CreateDeviceThread(pthread_t *thread, ThreadType type);
#endif
    int				_AsyncOpenDeviceThread();
	BOOL			_FPGA_GetVoltage(BYTE *Voltage);
	BOOL			_FPGA_SetVoltage(BYTE Voltage);
	int				_GetContrast_FromGainTable(int index, int auto_capture_mode);
	int				_Set_CIS_GainRegister(int index, int auto_capture_mode, BOOL kojak_dac_mode=FALSE);

	int				_ROIC_SetRegister(WORD adr, WORD val);
	BOOL			_Go_Job_UnexpectedTermination_fromCaptureThread(int nUnexpectedCount);
	AcuisitionState	_ClearDoubleBufferedImageOnFPGA(BYTE *InImg, int MIN_Elapsed);
	int				_DAC_SetRegister(WORD val);
	int				_DAC_SetRegister_for_Kojak(WORD val);
	int				_CheckForSupportedImageType(const IBSU_ImageType imageType, ScannerDeviceType devType);
    OverlayHandle*	_CreateOverlayHandle();
    BOOL			_FindOverlayHandleInList(int handle);
    int             _ShowOverlayObject(int handle, BOOL show);
    int             _ShowAllOverlayObject(BOOL show);
    int             _RemoveOverlayObject(int handle);
    int             _RemoveAllOverlayObject();
    CurveModel      _GetCurveModel(const PropertyInfo *pPropertyInfo, const UsbDeviceInfo *pScanDevDesc);

	int				_IsNeedInitializeCIS();
	int				_IsNeedInitializeCIS_Watson();
	int				_IsNeedInitializeCIS_Sherlock();
	int				_IsNeedInitializeCIS_Columbo();
	int				_IsNeedInitializeCIS_Curve();
	int				_IsNeedInitializeCIS_CurveSETi();
	int				_IsNeedInitializeCIS_Holmes();
	int				_IsNeedInitializeCIS_Kojak();
	int				_IsNeedInitializeCIS_Five0();

    int             _CaptureStart_for_Decimation();

	int				_SetPlateTouchOn();
	int				_SetPlateTouchOff();
	int				_SetPlateTouchStatus(BYTE state);
	int				_GetPlateTouchStatus(BYTE &state);
	int				_SetRelayStatus(BYTE addr, BYTE val);

	int				_LE_OFF();
	int				_WriteCurrentSensor(BYTE addr, WORD Value);
	int				_ReadCurrentSensorPrototype(WORD &state);
	int				_ReadCurrentSensor(WORD &state);
	int				_ReadAnalogTOFFromKojak(WORD &c_state, WORD &a_state, WORD &a_state2);
	WORD			_CalculateCurrentLimitPrototype();
	WORD			_CalculateCurrentLimit();

	int				_Start_PlateTouch();
	BOOL			_Check_PlateTouch();
	int				_Start_TOF();
	BOOL			_Check_TOF(int *CurTOF, int *ThresTOF);
	BOOL			_ReadTOFStatus(BYTE *GroundCheck, BYTE *Idle);

	int				_SetADCThres(unsigned char ADCThres);

	int				m_TouchCount;
	int				m_TouchFrameCount;
	int				m_TOF_ON_Count;
	int				m_TOF_OFF_Count;
	int				m_TOFArr[3];
	int				m_TOFArr_idx;
	int				m_TOF_CurVal;
	int				m_TOF_Thres;
	int				m_TOF_Analog;
	int				m_TOF_Analog2;
	int				m_TOF_Analog_Thres;	
	int				m_TOF_DAC;
	int				m_Brightness;
	int				m_TOF_MaxVal;
	int				m_TOF_MaxVal_DAC;

	int				m_ATOF_ON_Count;
	int				m_ATOF_OFF_Count;

	CaptureProgressStatus	m_CaptureProgressStatus;

// for KOJAK
	int				_OnOff_LE_Kojak(unsigned char on_off);
//	int				_ClearButtonStat_Kojak(unsigned char btn_idx);
	int				_GetButtonStat_Kojak(unsigned char *btn1_stat, unsigned char *btn2_stat);
//	int				_PowerOffButton_Kojak(unsigned char btn_idx);
	void			_SetDryCapture_Kojak();
	
	int				ASIC_GetRegister(unsigned char main_addr, unsigned char reg_addr, unsigned char *reg_value);
	int				ASIC_SetRegister(unsigned char main_addr, unsigned char reg_addr, unsigned char reg_value);
	int				ASIC_SetRegister_Five0(unsigned char main_addr, WORD reg_addr, unsigned char reg_value);
	int				ASIC_GetRegister_Five0(unsigned char main_addr, WORD reg_addr, unsigned char *reg_value);
	int				ASIC_Reset();
	int				ASIC_AnalogPower();
	int				ASIC_Initialize();
	int				ASIC_Analog_Power_On();

	void			TFT_MasterSlaveDeviation(unsigned char *InImg, double *Bright);

#if  defined(__linux__) || defined(__USE_LIBUSBX_DRIVER__)
#ifdef __libusb_latest__
	int				_DeviceCount(libusb_device **devs);
	BOOL			_Open(int deviceIndex, int *errorNo, libusb_device **devs);
#else
	int				_DeviceCount();
	int				_GetSerialNumber_Curve(PUCHAR  Buffer);
#endif
#endif
#if defined(__linux__)
#ifdef __libusb_latest__
	BOOL			_Open(int deviceIndex, int *errorNo);
	int				_GetSerialNumber_Curve(PUCHAR  Buffer);
#else
	BOOL			_Open(int deviceIndex, int *errorNo);
#endif
#endif 

#if defined(__ppi__)
    int             _DeviceCount_PPI();
    BOOL            _OpenUSBDevice_PPI(int deviceIndex, int *errorNo);
    void            _CloseUSBDevice_PPI();
    BOOL            _Open_PPI(int deviceIndex, int *errorNo);
	int 			_UsbBulkOutIn_PPI(int outEp, UCHAR uiCommand, UCHAR *outData, LONG outDataLen,
	                                int inEp, UCHAR *inData, LONG inDataLen, LONG *nBytesRead, int timeout);
    int             _UsbBulkIn_PPI(int ep, UCHAR *data, LONG dataLen, LONG *nBytesRead, int timeout);
    int             _UsbBulkOut_PPI(int ep, UCHAR uiCommand, UCHAR *data, LONG dataLen, int timeout);
    void            _SetPID_VID_PPI(UsbDeviceInfo *pScanDevDesc, char *productID);
    int             _GetPID_PPI();
    int             _GetSerialNumber_Curve_PPI(PUCHAR  Buffer);
    int             _GetUsbSpeed_PPI(UsbSpeed *speed);
#endif

	BOOL			_Algo_GetWaterMaskFromFlashMemory(unsigned char *mask_buf, LONG mask_buf_size);
	BOOL			_Algo_Columbo_GetWaterMaskFromFlashMemory(unsigned char *mask_buf, LONG mask_buf_size);
	BOOL			_Algo_Kojak_GetWaterMaskFromFlashMemory(unsigned char *mask_buf, LONG mask_buf_size);
	BOOL			_Algo_Sherlock_GetWaterMaskFromFlashMemory(unsigned char *mask_buf, LONG mask_buf_size);
	BOOL			_Algo_Five0_GetWaterMaskFromFlashMemory(unsigned char *mask_buf, LONG mask_buf_size);

	int				_ReadTFT_DefectMaskFromFlashMemory(unsigned char *mask_buf, long mask_buf_size);
	BOOL			_TFT_DefectMaskDecompress(unsigned char *compress_buf, unsigned char *mask_buf, int maskBufSize);
	BOOL			_TFT_DefectMaskCompress(unsigned char *mask_buf, unsigned char *compress_buf, int maskBufSize);

    int             _PowerCaptureInit_Curve(GoodCaptureInfo *capinfo, PropertyInfo *propertyInfo);
    void            _OninitCISRegister_Curve(int option, PropertyInfo *propertyInfo);
	BOOL			_Algo_Curve_AutoCapture(GoodCaptureInfo *capinfo, PropertyInfo *propertyInfo, int frameTime);
	BOOL			_Algo_Curve_ManualCapture(int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime);
	BOOL			_Algo_CurveSeti_AutoCapture(GoodCaptureInfo *capinfo, PropertyInfo *propertyInfo, int frameTime);
	BOOL			_Algo_CurveSeti_ManualCapture(int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime);
	int				_ReadMaskFromEP6IN_Columbo(unsigned char *mask_buf, int mask_buf_size); // for Read Columbo Mask Add by Sean 2013/06/05
	int				_ReadMaskFromEP8IN_Kojak(unsigned char *mask_buf, int mask_buf_size); // for Read Columbo Mask Add by Sean 2013/06/05
	int				_ReadMaskFromEP8IN_Kojak_8M(unsigned char *mask_buf, int mask_buf_size, const int section);
	int				_ReadMaskFromEP8IN_Five0(unsigned char *mask_buf, int mask_buf_size, const int section); // for Read Columbo Mask Add by Sean 2013/06/05
//	int				_WriteMask_Five0(const BYTE *MaskData, const int MaskDataLen, const int section, const int start_page);
	int				_GetFpgaVersion(int Extension = FALSE);
	void			_UpdateGammaLevel(int maxAutoContrastValue);
	int				_GetCurveVersion();

	int				_GetUsbSpeed(UsbSpeed *speed);
	int				_SetDecimation_Columbo(BOOL enable);
	int				_SetDecimationMode_Columbo(DecimationMode mode);

	void			_RemoveOverlayHandle(int handle);
	void			_RemoveOverlayTextEx(int handle);
	void			_RemoveOverlayLineEx(int handle);
	void			_RemoveOverlayQuadrangleEx(int handle);
	void			_RemoveOverlayShapeEx(int handle);

	void			_RemoveAllOverlayText();
	void			_RemoveAllOverlayHandle();
	void			_RemoveAllOverlayTextEx();
	void			_RemoveAllOverlayLineEx();
	void			_RemoveAllOverlayQuadrangleEx();
	void			_RemoveAllOverlayShapeEx();

	int				_OninitCISRegister_CurveSETi();
	int				_PowerCaptureInit_CurveSETi(GoodCaptureInfo *capinfo, PropertyInfo *propertyInfo);


	void			_ChangeDecimationModeForKojak(BOOL DecimationMode, CaptureSetting *CurrentCaptureSetting, BOOL ApplyChangeSetting);
	void			_ApplyCaptureSettings(CaptureSetting Settings);

	int				_ApplyCaptureSettings_Five0(unsigned char DAC_Value);
	int				_InitializeTOFSensor_Five0(BOOL useDigitalTouch, BOOL useAnalogTouch);
	int				_CalibrateTOF();
	int				_ReadTOFSensorDigital_Five0(int *plate, int *film);
	int				_ReadTOFSensorAnalog_Five0(int *plate_org, int *film_org, int *plate, int *film);

	int				_AES_KeyTransfer_For_Watson(unsigned char* key, int size);
	int				_AES_KeyTransfer_For_Kojak(unsigned char* key, int size);
#ifdef _KOJAK_ATOF_16B_
	int				_Kojak30_InitializeOperationMode(int Mode);
#endif

	int				_IsRequiredSDKVersion(char *requiredSDKVersion);

	int				_SetEncryptionKey(const unsigned char *pEncryptionKey, const IBSU_EncryptionMode encMode);
	int				_AES_KeyTransfer(const unsigned char *pEncryptionKey, int KeyLength);
	int				_Initialize_Encryption_For_Watson();
	int				_Initialize_Encryption_For_Kojak();

	////////////////////////////////////////////////////////////////////////////////////////
	// Reserved functions
	int				Reserved_GetDeviceInfo(const int deviceIndex, RESERVED_DeviceInfo *deviceInfo, BOOL bPPIMode=false);
	int				Reserved_WriteEEPROM( WORD addr, const BYTE *buf, int len);
	int				Reserved_ReadEEPROM( WORD addr, BYTE *buf, int len);
	int				Reserved_GetOneFrameImage(unsigned char *pRawImage, int imageLength);
	int				Reserved_SetFpgaRegister(UCHAR address, UCHAR value);
	int				Reserved_GetFpgaRegister(UCHAR address, UCHAR *value);
	int				Reserved_DrawClientWindow(unsigned char *drawImage);
	int				Reserved_UsbBulkOutIn(int outEp, UCHAR uiCommand, UCHAR *outData, LONG outDataLen,
								 int inEp, UCHAR *inData, LONG inDataLen, LONG *nBytesRead);
	int				Reserved_InitializeCamera();
	int				Reserved_WriteMask(const BYTE *MaskData, const int MaskDataLen, int *pProgress);
	int				Reserved_WriteMask_8M(const BYTE *MaskData, const int MaskDataLen, const int sector, int *pProgress);
	int				Reserved_ResetDevice();
	int				Reserved_InitializeCameraFlatForKOJAK();
	int				Reserved_InitializeCameraRollForKOJAK();
	int				Reserved_GetEncryptedImage(unsigned char *pEncKey, IBSU_ImageData *pRawEncImage);
	////////////////////////////////////////////////////////////////////////////////////////

	int				_ReadVersionFromEEPROM(int *eepromVersion);
	int				_WriteVersionToEEPROM(char *FPGAVersion);

private:

#ifdef WINCE
#define THREADCALLBACKRET UINT
#elif defined(_WINDOWS)
#define THREADCALLBACKRET UINT
#elif defined(__linux__)
#define THREADCALLBACKRET void*
#endif 

/// Image quality infield test execution thread.
/// Image quality infield test execution thread.
static THREADCALLBACKRET _InitializeThreadCallback(LPVOID pParam);
static THREADCALLBACKRET _CaptureThreadCallback(LPVOID pParam);
static THREADCALLBACKRET _AsyncInitializeThreadCallback(LPVOID pParam);
static THREADCALLBACKRET _KeyButtonThreadCallback(LPVOID pParam);
static THREADCALLBACKRET _TOFSensorThreadCallback(LPVOID pParam);

#ifdef _WINDOWS
	int			gettimeofday(struct timeval *tv, struct timezone *tz);
#endif

public: // Static members:


public: // Inline members

	inline void		SetInitialized(BOOL value);
	inline BOOL		IsInitialized(void) const;
	inline void		SetDeviceIndex(int index);
	inline int		GetDeviceIndex(void) const;
	inline void		SetDeviceHandle(int handle);
	inline int		GetDeviceHandle(void) const;
//	inline void		SetIsCommunicationBreak(BOOL value);
//	inline BOOL		GetIsCommunicationBreak(void) const;
	inline BOOL		IsCaptureSetMode(void) const;
	inline void		SetIsActiveCapture(BOOL isActive);
	inline BOOL		GetIsActiveCapture(void) const;
	inline void		SetTakeResultImageManually(BOOL isSet);
	inline BOOL		GetTakeResultImageManually(void) const;
	inline void		SetIsActiveDummy(BOOL isActiveDummy);
	inline BOOL		GetIsActiveDummy(void) const;
	inline void     SetIsActiveStateOfCaptureThread(BOOL isActiveCapture, BOOL isActiveDummy, BOOL isActiveIdle);

protected:


public:	// Members variables
	CIBScanUltimateApp	*m_pDlg;
	CIBUsbManager		*m_pDlgUsbManager;

	PropertyInfo		m_propertyInfo;


	CallbackParam		m_clbkParam[__MAX_CALLBACK_COUNT__];
	CallbackProperty	m_clbkProperty;
	BlockProperty		m_blockProperty;

	DisplayWindow		m_DisplayWindow;
	UsbDeviceInfo		m_UsbDeviceInfo;

	int					m_nGammaLevel;

	int					m_nFrameCount;
	BOOL				m_bFirstSent_clbkClearPlaten;

	BOOL				m_bAliveCaptureThread;
	BOOL				m_bAliveKeyButtonThread;
	BOOL				m_bAliveTOFSensorThread;
	char				m_szDevicePath[MAX_PATH];

	CIBAlgorithm*		m_pAlgo;
	CIBSMAlgorithm*		m_pMatcherAlg;
	BOOL				m_bUniformityMaskPath;
	char				m_cUniformityMaskPath[1024];

	int					m_nGoodFrameCount;
	double				m_timeAfterBeginCaptureImage;
	int					m_SavedFingerCountStatus;
	BOOL				m_bFirstPutFingerOnSensor;

	TouchSensorMode		m_Five0_TouchSensorMode;

   	struct timeval      m_finish_tv, m_start_tv;
#ifdef __G_INIT_CAPTURE_TIME__
   	struct timeval      m_initCaptureFinish_tv, m_initCaptureStart_tv;
#endif
    double              m_Elapsed;

#ifdef __G_PERFORMANCE_DEBUG__
   	struct timeval      m_pd_finish_tv, m_pd_start_tv;
    double              m_pd_Elapsed;
#endif

    BOOL                m_bRunningEP6;
	int					m_nRemainedDecimationFrameCount;

	int					m_Send_CaptureStart_BtnStat;


	CaptureSetting		m_DeciCaptureSetting;
	CaptureSetting		m_FullCaptureSetting;

	int					m_nDigitalTouch_Plate;
	int					m_nDigitalTouch_Film;
	int					m_nAnalogTouch_Plate;
	int					m_nAnalogTouch_Film;
	int					m_nAnalogTouch_PlateOrg;
	int					m_nAnalogTouch_FilmOrg;
	int					m_nAnalogTouch_Min;
	float				m_nAnalogTouch_Slope;
	int					m_TOF_SUCCESS_COUNT;
   	struct timeval      m_LastCalibrate_tv;
	int					m_bTOFCalibrated;

	IBSU_EncryptionMode m_EncryptionMode;

#ifdef _WINDOWS
	HANDLE		m_hCaptureThread_DestroyEvent;
	HANDLE		m_hCaptureThread_CaptureEvent;
	HANDLE		m_hCaptureThread_DummyEvent;
#if defined(__IBSCAN_ULTIMATE_SDK__)
	HANDLE		m_hCaptureThread_EventArray[3];
#elif defined(__IBSCAN_SDK__)
	HANDLE		m_hRawCaptureThread_GetOneFrameImageEvent;
	HANDLE		m_hRawCaptureThread_TakePreviewImageEvent;
	HANDLE		m_hRawCaptureThread_TakeResultImageEvent;
	HANDLE		m_hRawCaptureThread_AbortEvent;
	HANDLE		m_hCaptureThread_EventArray[7];
#endif

	HANDLE		m_hKeyButtonThread_DestroyEvent;
	HANDLE		m_hKeyButtonThread_ReadEvent;
	HANDLE		m_hKeyButtonThread_EventArray[2];

	HANDLE		m_hTOFSensorThread_DestroyEvent;
	HANDLE		m_hTOFSensorThread_ReadEvent;
	HANDLE		m_hTOFSensorThread_EventArray[2];
#endif 

#if defined(_WINDOWS) // WINCE also defines _WINDOWS
	CWinThread	*m_pInitializeThread;
	CWinThread	*m_pCaptureThread;
	CWinThread	*m_pAsyncInitializeThread;
	CWinThread	*m_pKeyButtonThread;
	CWinThread	*m_pTOFSensorThread;
#elif defined(__linux__)
	pthread_t   m_pInitializeThread;
	pthread_t   m_pCaptureThread;
	pthread_t   m_pAsyncInitializeThread;
	pthread_t   m_pKeyButtonThread;
	pthread_t   m_pTOFSensorThread;
#endif

#ifdef _WINDOWS
	HWND		m_hWindow;
	RECT		m_rectWindow;
	IB_BITMAPINFO	*m_BitmapInfo;
#endif 

#if defined(_WINDOWS) && defined(__USE_WINUSB_DRIVER__)
	CWinUSBInterface* m_pUsbDevice;
	CCurveUSBInterface* m_pCurveUsbDevice;

#elif defined(__linux__)
#ifdef __libusb_latest__
	std::vector<struct libusb_device *> m_pUSBDevice;  // List of Watson devices found in bus scan
#else
	std::vector<struct usb_device *> m_pUSBDevice;  // List of Watson devices found in bus scan
#endif
	BOOL		m_bCaptureThread_DestroyEvent;
	BOOL		m_bCaptureThread_CaptureEvent;
	BOOL		m_bCaptureThread_DummyEvent;
#if defined(__IBSCAN_SDK__)
	BOOL		m_bRawCaptureThread_GetOneFrameImageEvent;
	BOOL		m_bRawCaptureThread_TakePreviewImageEvent;
	BOOL		m_bRawCaptureThread_TakeResultImageEvent;
	BOOL		m_bRawCaptureThread_AbortEvent;
#endif

	BOOL		m_bKeyButtonThread_DestroyEvent;
	BOOL		m_bKeyButtonThread_ReadEvent;

    BOOL        m_bTOFSensorThread_DestroyEvent;
    BOOL        m_bTOFSensorThread_ReadEvent;
#endif

#if defined(__ppi__)
    struct usb_device *m_pdev;
    int         m_nLEVoltage;
#endif
	std::vector<OverlayText *> m_pOverlayText;  // List of OverlayText for display on window
	std::vector<OverlayHandle *> m_pOverlayHandle;    // List of OverlayHandle
	std::vector<OverlayTextEx *> m_pOverlayTextEx;  // List of OverlayTextEx with overlay handle for display on window
	std::vector<OverlayLineEx *> m_pOverlayLineEx;  // List of OverlayLineEx with overlay handle for display on window
	std::vector<OverlayQuadrangleEx *> m_pOverlayQuadrangleEx;  // List of OverlayQuadrangleEx with overlay handle for display on window
	std::vector<OverlayShapeEx *> m_pOverlayShapeEx;  // List of OverlayShapeEx with overlay handle for display on window
	BOOL		m_bCaptureThread_StopMessage;
	BOOL		m_bCaptureThread_DummyStopMessage;

	BOOL		m_bInitializeThread_StopMessage;

	int			m_SavedFingerState;
	BOOL		m_EnterSleep;
    BOOL        m_bPowerResume;
	BOOL		m_bNeedtoInitializeCIS_Five0;

	int			m_CurrentCaptureDAC;
	int			m_CurrentCaptureDACMax;
	int			m_CurrentCaptureDACMax_TOFValue;
#ifdef _KOJAK_ATOF_16B_
	int			m_Kojak30_OperationMode;
#endif
protected: // Members variables

	BOOL		m_bIsInitialized;			// Intialize for USB device
	int			m_nDeviceIndex;				// Device index
	BOOL		m_bIsActiveCapture;			// Status is capturing
//	BOOL		m_bIsCommunicationBreak;	// Status is communication break
	BOOL		m_bIsCaptureSetMode;		// Is executed Capture_SetMode()
	BOOL		m_bIsSetTakeResultImageManually;

#if !defined(__USE_LIBUSBX_DRIVER__)
	UINT		m_unDeviceNumber;						// device number selected
#endif
	BOOL		m_bIsActiveDummy;
	BOOL		m_bIsActiveIdle;

	int			m_apiHandle;

	int			m_MasterValue; 
	int			m_SlaveValue;

    int         m_bPPIMode;

#if defined(__linux__) 
#ifdef __libusb_latest__
	libusb_device_handle * m_hDriver;   // Linux
#else
	usb_dev_handle * m_hDriver;   // Linux
#endif
#elif defined(__USE_LIBUSBX_DRIVER__)
	libusb_device_handle * m_hLibUsbDriver;
#endif

#if defined(__ppi__)	
	int		m_nPiHandle;
 	Parallel_device	m_parallelDevice;
#endif

protected: // Const variables
};

////////////////////////////////////////////////////////////////////////////////

inline void CMainCapture::SetInitialized(BOOL value)
	{ m_bIsInitialized = value; return; }

inline BOOL CMainCapture::IsInitialized(void) const
	{ return m_bIsInitialized; }

inline void CMainCapture::SetDeviceIndex(int index)
	{ m_nDeviceIndex = index; return; }

inline int CMainCapture::GetDeviceIndex(void) const
	{ return m_nDeviceIndex; }

inline int CMainCapture::GetDeviceHandle(void) const
	{ return m_apiHandle; }

inline void CMainCapture::SetDeviceHandle(int handle)
	{ m_apiHandle = handle; }

/*inline void CMainCapture::SetIsCommunicationBreak(BOOL value)
	{ m_bIsCommunicationBreak = value; return; }

inline BOOL CMainCapture::GetIsCommunicationBreak(void) const
	{ return m_bIsCommunicationBreak; }
*/
inline BOOL CMainCapture::IsCaptureSetMode(void) const
	{ return m_bIsCaptureSetMode; }

inline void CMainCapture::SetIsActiveCapture(BOOL isActive)
	{ m_bIsActiveCapture = isActive; return; }

inline BOOL CMainCapture::GetIsActiveCapture(void) const
	{ return m_bIsActiveCapture; }

inline void CMainCapture::SetTakeResultImageManually(BOOL isSet)
	{ m_bIsSetTakeResultImageManually = isSet; return; }

inline BOOL CMainCapture::GetTakeResultImageManually(void) const
	{ return m_bIsSetTakeResultImageManually; }

inline void CMainCapture::SetIsActiveDummy(BOOL isActiveDummy)
	{ m_bIsActiveDummy = isActiveDummy; return; }

inline BOOL CMainCapture::GetIsActiveDummy(void) const
	{ return m_bIsActiveDummy; }

inline void CMainCapture::SetIsActiveStateOfCaptureThread(BOOL isActiveCapture, BOOL isActiveDummy, BOOL isActiveIdle)
	{ m_bIsActiveCapture = isActiveCapture; m_bIsActiveDummy = isActiveDummy; m_bIsActiveIdle = isActiveIdle; return; }


/////////////////////////////////////////////////////////////////////////////
#endif //__CAPTURECORE_H__
/////////////////////////////////////////////////////////////////////////////
