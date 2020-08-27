// IBScanUltimate.h : main header file for the IBScanUltimate DLL
//

#pragma once

#ifdef _WINDOWS
    #ifndef __AFXWIN_H__
	     #error "include 'stdafx.h' before including this file for PCH"
    #endif

    #include "resource.h"		// main symbols
    #include <Dbt.h>
    #ifndef WINCE
        #include <setupapi.h>
    #endif 

	#ifdef WINCE
		#include <msgqueue.h>
		#include <pnp.h>
	#endif
#endif
#include <map>
#include <vector>
#if defined(__IBSCAN_ULTIMATE_SDK__)
#include "IBScanUltimateApi.h"
#elif defined(__IBSCAN_SDK__)
#include "IBScanApi.h"
#endif
#include "ReservedApi.h"
#include "CaptureLib/IBSU_Global_Def.h"
#include "CaptureLib/MultiThreadSync.h"
#include "CaptureLib/MainCapture.h"
// Thread pool includes 
#include "ThreadPool/Job.h"
#include "ThreadPool/ThreadPoolMgr.h"

//Matcher
#include "MatcherLib/IBSMAlgorithm.h"

using namespace std;

#ifdef WINCE
typedef union {
	DEVDETAIL d;
	char pad[sizeof(DEVDETAIL) + MAX_DEVCLASS_NAMELEN];
} MYDEV;
#endif

/*
****************************************************************************************************
* tag_IBSM_Point
*
* DESCRIPTION:
*     CPoint stucture for Linux
*
* ARGUMENTS:
*     x : x coordinate
*     y : x coordinate
****************************************************************************************************
*/
typedef struct tag_IBSM_Point
{
    int x;
	int y;
}
IBSM_Point;

// CIBScanUltimateApp
// See IBScanUltimate.cpp for the implementation of this class
//
class CIBUsbManager : public CMultiThreadSync<CIBUsbManager>
{
public:
	CIBUsbManager();

// Overrides
	virtual ~CIBUsbManager(void);

	int 			_Delete_MainCapture(BOOL bReleasMark = TRUE, BOOL bDeleteClass = TRUE);

	CMainCapture*	m_pMainCapture;
	int				m_nUsbHandle;
	int				m_nUsbIndex;
	char			m_szDevicePath[MAX_PATH];	// Device path
	BOOL			m_bIsCommunicationBreak;
	IBSU_DeviceDescA m_DeviceDesc;
	BOOL			m_bIsReadySend[__MAX_CALLBACK_COUNT__];
	BOOL			m_bInitialized;
	BOOL			m_bCompletedOpenDeviceThread;
    BOOL            m_bEnterCallback;
};

#ifdef _WINDOWS
class CIBScanUltimateApp : public CWinApp, public CMultiThreadSync<CIBScanUltimateApp>
#elif defined(__linux__)
class CIBScanUltimateApp : public CMultiThreadSync<CIBScanUltimateApp>
#endif
{
public:
	CIBScanUltimateApp();

// Overrides
public:
	virtual ~CIBScanUltimateApp(void);

#ifdef _WINDOWS
	virtual BOOL		InitInstance();
	virtual BOOL		ExitInstance();

	BOOL				CreateMsgWnd(HINSTANCE hInst, HWND *phWnd);
	LRESULT				OnDeviceChange(WPARAM wParam, LPARAM lParam);
	LRESULT				OnPaintClientWindow(WPARAM wParam, LPARAM lParam);
	LRESULT				OnPowerBroadCast(WPARAM wParam, LPARAM lParam);
  #ifndef WINCE
    BOOL				IsIBScanDevice(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam, char *outNameBuf);
  #endif 
#endif


//////////////////////////////////////////////////////////////////////////////////
#ifndef WINCE
	int					DLL_GetSDKVersion(IBSU_SdkVersion *pVerInfo);
#else
	int					DLL_GetSDKVersionW(IBSU_SdkVersionW *pVerInfo);
#endif
	int					DLL_GetDeviceCount(int *pDeviceCount);
	int					DLL_GetDeviceDescription(const int deviceIndex, IBSU_DeviceDescA *pDeviceDesc);
#ifdef WINCE
	int					DLL_GetDeviceDescriptionW(const int deviceIndex, IBSU_DeviceDescW *pDeviceDesc);
#endif
	int					DLL_RegisterCallbacks(const int	handle, const IBSU_Events events, void *pEventName, void *pContext);
	int					DLL_OpenDevice(const int deviceIndex, const BOOL useUniformityMask, int *pHandle);
	int					DLL_CloseDevice(const int handle);
	int					DLL_CloseAllDevice(const BOOL appTerminate = FALSE);
	int					DLL_IsDeviceOpened(const int handle);
	int					DLL_GetProperty(const int handle, const IBSU_PropertyId propertyId, LPSTR propertyValue);
	int					DLL_GetPropertyW(const int handle, const IBSU_PropertyId propertyId, LPWSTR propertyValue);
	int					DLL_SetProperty(const int handle,const IBSU_PropertyId propertyId, LPCSTR propertyValue);
	int					DLL_SetPropertyW(const int handle,const IBSU_PropertyId propertyId, LPCWSTR propertyValue);
	int					DLL_EnableTraceLog(const BOOL on);
	int					DLL_IsCaptureAvailable(const int handle, const IBSU_ImageType imageType,
											const IBSU_ImageResolution imageResolution, BOOL *pIsAvailable);
	int					DLL_BeginCaptureImage(const int handle, const IBSU_ImageType imageType,
											const IBSU_ImageResolution imageResolution, const DWORD captureOptions);
	int					DLL_CancelCaptureImage(const int handle);
	int					DLL_IsCaptureActive(const int handle, BOOL *pIsActive);
	int					DLL_TakeResultImageManually(const int handle);
	int					DLL_GetContrast(const int handle, int *pContrastValue);
	int					DLL_SetContrast(const int handle, const int contrastValue);
	int					DLL_GetLEOperationMode(const int handle, IBSU_LEOperationMode *leOperationMode);
	int					DLL_SetLEOperationMode(const int handle, const IBSU_LEOperationMode leOperationMode);
	int					DLL_IsTouchedFinger(const int handle, int *touchInValue);
	int					DLL_GetOperableLEDs(const int handle, IBSU_LedType *pLedType, int *pLedCount, DWORD *pOperableLEDs);
	int					DLL_GetLEDs(const int handle, DWORD	*pActiveLEDs);
	int					DLL_SetLEDs(const int handle, const DWORD activeLEDs);
	int					DLL_CreateClientWindow(const int handle, const IBSU_HWND hWindow, const DWORD left,
											const DWORD top, const DWORD right, const DWORD bottom);
	int					DLL_DestroyClientWindow(const int handle, const BOOL clearExistingInfo);
	int					DLL_GetClientWindowProperty(const int handle, const IBSU_ClientWindowPropertyId propertyId,
											LPSTR propertyValue);
	int					DLL_GetClientWindowPropertyW(const int handle, const IBSU_ClientWindowPropertyId propertyId,
											LPWSTR propertyValue);
	int					DLL_SetClientDisplayProperty(const int handle, const IBSU_ClientWindowPropertyId propertyId,
											LPCSTR propertyValue);
	int					DLL_SetClientDisplayPropertyW(const int handle, const IBSU_ClientWindowPropertyId propertyId,
											LPCWSTR propertyValue);
	int					DLL_SetClientWindowOverlayText(const int handle, const char *fontName, const int fontSize,
											const BOOL fontBold, const char *text, const int posX, const int posY,
											const DWORD textColor);
	int					DLL_SetClientWindowOverlayTextW(const int handle, const WCHAR *fontName, const int fontSize,
											const BOOL fontBold, const WCHAR *text, const int posX, const int posY,
											const DWORD textColor);
	int					DLL_GenerateZoomOutImage(const IBSU_ImageData inImage, BYTE *outImage, const int outWidth,
											const int outHeight, const BYTE bkColor);
	int					DLL_SaveBitmapImage(LPCSTR filePath, const BYTE *imgBuffer, const DWORD width, const DWORD height,
											const int pitch, const double resX, const double resY);
	int					DLL_SaveBitmapImageW(LPCWSTR filePath, const BYTE *imgBuffer, const DWORD width, const DWORD height,
											const int pitch, const double resX, const double resY);
	int					DLL_AsyncOpenDevice(const int deviceIndex, BOOL useUniformityMask);
	int					DLL_BGetImage(const int	handle, IBSU_ImageData *pImage, IBSU_ImageType *pImageType,
											IBSU_ImageData *pSplitImageArray, int *pSplitImageArrayCount,
											IBSU_FingerCountState *pFingerCountState, IBSU_FingerQualityState *pQualityArray,
											int *pQualityArrayCount);
	int					DLL_BGetImageEx(const int handle, int *pImageStatus, IBSU_ImageData *pImage,
											IBSU_ImageType *pImageType, int *pDetectedFingerCount,
											IBSU_ImageData *pSegmentImageArray, IBSU_SegmentPosition *pSegmentPositionArray,
											int *pSegmentImageArrayCount, IBSU_FingerCountState *pFingerCountState,
											IBSU_FingerQualityState *pQualityArray, int *pQualityArrayCount);
	int					DLL_BGetInitProgress(const int deviceIndex, BOOL *pIsComplete, int *pHandle, int *pProgressValue);
	int					DLL_BGetClearPlatenAtCapture(const int handle, IBSU_PlatenState *pPlatenState);
	int					DLL_BGetRollingInfo(const int handle, IBSU_RollingState *pRollingState, int *pRollingLineX);
	int					DLL_BGetRollingInfoEx(const int handle, IBSU_RollingState *pRollingState, int *pRollingLineX, int *pRollingDirection, int *pRollingWidth);
	int					DLL_OpenDeviceEx(const int deviceIndex, LPCSTR uniformityMaskPath, const BOOL useUniformityMask, const BOOL asyncOpen, int *pHandle);
	int					DLL_OpenDeviceExW(const int deviceIndex, LPCWSTR uniformityMaskPath, const BOOL useUniformityMask, const BOOL asyncOpen, int *pHandle);
	int					DLL_GetIBSM_ResultImageInfo(const int handle, IBSM_FingerPosition fingerPosition, IBSM_ImageData *pResultImage,
											IBSM_ImageData *pSplitResultImage, int *pSplitResultImageCount);
	int					DLL_GetNFIQScore(const int handle, const BYTE *imgBuffer, const DWORD width, const DWORD height,
											const BYTE bitsPerPixel, int *pScore);
	int					DLL_GenerateZoomOutImageEx(const BYTE *pInImage, const int inWidth, const int inHeight,
											BYTE *outImage, const int outWidth, const int outHeight, const BYTE bkColor);
	int					DLL_ReleaseCallbacks(const int handle, const IBSU_Events events);
	int					DLL_SaveBitmapMem(const BYTE *inImage, const DWORD inWidth, const DWORD inHeight, const int inPitch,
											const double inResX, const double inResY, BYTE *outBitmapBuffer,
											const IBSU_ImageFormat outImageFormat, const DWORD outWidth, const DWORD outHeight,
											const BYTE bkColor);
	int					DLL_ShowOverlayObject(const int handle, const int overlayHandle, const BOOL show);
	int					DLL_ShowAllOverlayObject(const int handle, const BOOL show);
	int					DLL_RemoveOverlayObject(const int handle, const int overlayHandle);
	int					DLL_RemoveAllOverlayObject(const int handle);
	int					DLL_AddOverlayText(const int handle, int *pOverlayHandle, const char *fontName, const int fontSize,
											const BOOL fontBold, const char *text, const int posX, const int posY, const DWORD textColor);
	int					DLL_AddOverlayTextW(const int handle, int *pOverlayHandle, const wchar_t *fontName, const int fontSize,
											const BOOL fontBold, const wchar_t *text, const int posX, const int posY, const DWORD textColor);
	int					DLL_ModifyOverlayText(const int handle, const int overlayHandle, const char *fontName, int fontSize,
											const BOOL fontBold, const char *text, const int posX, const int posY, const DWORD textColor);
	int					DLL_ModifyOverlayTextW(const int handle, const int overlayHandle, const wchar_t *fontName, int fontSize,
											const BOOL fontBold, const wchar_t *text, const int posX, const int posY, const DWORD textColor);
	int					DLL_AddOverlayLine(const int handle, int *pOverlayHandle, const int x1, const int y1, const int x2, const int y2,
											const int lineWidth, const DWORD lineColor);
	int					DLL_ModifyOverlayLine(const int handle, const int overlayHandle, const int x1, const int y1,
											const int x2, const int y2, const int lineWidth, const DWORD lineColor);
	int					DLL_AddOverlayQuadrangle(const int handle, int *pOverlayHandle, const int x1, const int y1,
											const int x2, const int y2, const int x3, const int y3,
											const int x4, const int y4, const int lineWidth, const DWORD lineColor);
	int					DLL_ModifyOverlayQuadrangle(const int handle, int overlayHandle, const int x1, const int y1,
											const int x2, const int y2, const int x3, const int y3,
											const int x4, const int y4, const int lineWidth, const DWORD lineColor);
	int					DLL_AddOverlayShape(const int handle, int *pOverlayHandle, const IBSU_OverlayShapePattern shapePattern,
											const int x1, const int y1, const int x2, const int y2,
											const int lineWidth, const DWORD lineColor, const int reserved_1, const int reserved_2);
	int					DLL_ModifyOverlayShape(const int handle, int overlayHandle, const IBSU_OverlayShapePattern shapePattern,
											const int x1, const int y1, const int x2, const int y2,
											const int lineWidth, const DWORD lineColor, const int reserved_1, const int reserved_2);
	int					DLL_WSQEncodeMem(const BYTE *image, const int width, const int height, const int pitch,
										const BYTE bitsPerPixel, const int pixelPerInch, const double bitRate,
										const char *commentText, unsigned char **compressedData, int *compressedLength);
	int					DLL_WSQEncodeToFile(LPCSTR filePath, const BYTE *image, const int width, const int height, const int pitch,
										const BYTE bitsPerPixel, const int pixelPerInch, const double bitRate, const char *commentText);
	int					DLL_WSQEncodeToFileW(LPCWSTR filePath, const BYTE *image, const int width, const int height, const int pitch,
										const BYTE bitsPerPixel, const int pixelPerInch, const double bitRate, const WCHAR *commentText);
	int					DLL_WSQDecodeMem(const BYTE *compressedImage, const int compressedLength,
										unsigned char **decompressedImage, int *outWidth, int *outHeight, int *outPitch, int *outBitsPerPixel, int *outPixelPerInch);
	int					DLL_WSQDecodeFromFile(LPCSTR filePath, unsigned char **decompressedImage, int *outWidth, int *outHeight, int *outPitch,
										int *outBitsPerPixel, int *outPixelPerInch);
	int					DLL_WSQDecodeFromFileW(LPCWSTR filePath, unsigned char **decompressedImage, int *outWidth, int *outHeight, int *outPitch,
										int *outBitsPerPixel, int *outPixelPerInch);
	int					DLL_FreeMemory(void *memblock);
	int					DLL_SavePngImage(LPCSTR filePath, const BYTE *image, const DWORD width, const DWORD height, const int pitch, const double resX, const double resY);
	int					DLL_SavePngImageW(LPCWSTR filePath, const BYTE *image, const DWORD width, const DWORD height, const int pitch, const double resX, const double resY);
	int					DLL_SaveJP2Image(LPCSTR filePath, const BYTE *image, const DWORD width, const DWORD height, const int pitch, const double resX, const double resY, const int fQuality);
	int					DLL_SaveJP2ImageW(LPCWSTR filePath, const BYTE *image, const DWORD width, const DWORD height, const int pitch, const double resX, const double resY, const int fQuality);
	int					DLL_RedrawClientWindow(const int handle, const DWORD flags);
	int					DLL_UnloadLibrary();
	int					DLL_CombineImage(const IBSU_ImageData InImage1, const IBSU_ImageData InImage2, IBSU_CombineImageWhichHand WhichHand, IBSU_ImageData *OutImage);
	int					DLL_GetOperableBeeper(const int handle, IBSU_BeeperType *pBeeperType);
	int					DLL_SetBeeper(const int handle, const IBSU_BeepPattern beepPattern, const DWORD soundTone, const DWORD duration, const DWORD reserved_1, const DWORD reserved_2);
#if defined(__IBSCAN_SDK__)
	int					DLL_Capture_SetMode(const int handle, const IBScanImageType imageType, const IBScanImageResolution imageResolution, const DWORD captureOptions,
										int *resultWidth, int *resultHeight, int *baseResolutionX, int *baseResolutionY);
	int					DLL_Capture_Start(const int handle, const int numberOfObjects);
    int                 DLL_GetDeviceHandleInfo(const int deviceIndex, int *handle, BOOL *isHandleOpened);
    int                 DLL_RawCapture_Start(const int handle, const int numberOfObjects);
    int                 DLL_RawCapture_Abort(const int handle);
    int                 DLL_RawCapture_GetOneFrameImage(const int handle);
    int                 DLL_RawCapture_TakePreviewImage(const int handle, const RawImageData rawImage);
    int                 DLL_RawCapture_TakeResultImage(const int handle, const RawImageData rawImage);
    int                 DLL_SetDACRegister(const int handle, const int dacValue);
#endif
	int					DLL_CombineImageEx(const IBSU_ImageData InImage1, const IBSU_ImageData InImage2, IBSU_CombineImageWhichHand WhichHand, IBSU_ImageData *OutImage,
											IBSU_ImageData  *pSegmentImageArray, IBSU_SegmentPosition *pSegmentPositionArray, int *pSegmentImageArrayCount);
	int					DLL_CheckWetFinger(const int handle, const IBSU_ImageData inImage);
	int					DLL_GetImageWidth(const int handle, const IBSU_ImageData inImage, int *Width_MM);
	
	int					DLL_IsWritableDirectory(LPCSTR dirpath, const BOOL needCreateSubFolder);
	int					DLL_IsWritableDirectoryW(LPCWSTR dirpath, const BOOL needCreateSubFolder);
	int					DLL_GenerateDisplayImage(const BYTE *pInImage, const int inWidth, const int inHeight,
											BYTE *outImage, const int outWidth, const int outHeight, const BYTE bkColor,
											const IBSU_ImageFormat outFormat, const int outQualityLevel, const BOOL verticalFlip);

	int					RES_OpenDevice(const int deviceIndex, const char *pReservedkey, const BOOL useUniformityMask, int *pHandle);
	int					RES_GetDeviceInfo(const int deviceIndex, const char *pReservedkey, RESERVED_DeviceInfo *pDeviceInfo);
	int					RES_WriteEEPROM(const int handle, const char *pReservedkey, const WORD addr, const BYTE *pData, const int len);
	int					RES_SetProperty(const int handle, const char *pReservedkey, const IBSU_PropertyId propertyId, LPCSTR propertyValue);
	int					RES_SetLEVoltage(const int handle, const char *pReservedkey, const int voltageValue);
	int					RES_GetLEVoltage(const int handle, const char *pReservedkey, int *pVoltageValue);
	int					RES_BeginCaptureImage(const int handle, const char *pReservedkey, const IBSU_ImageType imageType,
											const IBSU_ImageResolution imageResolution, const DWORD captureOptions);
	int					RES_GetOneFrameImage(const int handle, const char *pReservedkey, unsigned char *pRawImage, const int imageLength);
	int					RES_SetFpgaRegister(const int handle, const char *pReservedkey, const unsigned char address, const unsigned char value);
	int					RES_GetFpgaRegister(const int handle, const char *pReservedkey, const unsigned char address, unsigned char *pValue);
	int					RES_CreateClientWindow(const int handle, const char *pReservedkey, const IBSU_HWND hWindow,
											const DWORD left, const DWORD top, const DWORD right, const DWORD bottom,
											const DWORD rawImgWidth, const DWORD rawImgHeight);
	int					RES_DrawClientWindow(const int handle, const char *pReservedkey, unsigned char *drawImage);
	int					RES_UsbBulkOutIn(const int handle, const char *pReservedkey, const int outEp, const unsigned char uiCommand,
											unsigned char *outData, const int outDataLen, const int inEp, unsigned char *inData,
											const int inDataLen, int *nBytesRead);
	int					RES_InitializeCamera(const int handle, const char *pReservedkey);
	int					RES_ReadEEPROM(const int handle, const char *pReservedkey, const WORD addr, BYTE *pData, const int len);
	int					RES_GetEnhancedImage(const int handle, const char *pReservedkey, const IBSU_ImageData inImage, IBSU_ImageData *enhancedImage,
										 int *segmentImageArrayCount, IBSU_ImageData *segmentImageArray, IBSU_SegmentPosition *segmentPositionArray);
	int					RES_GetFinalImageByNative(const int handle, const char *pReservedkey, IBSU_ImageData *finalImage);
#ifdef _WINDOWS
	int					RES_GetSpoofScore(const char *pReservedKey, RESERVED_CaptureDeviceID  deviceID, const BYTE *pImage, const int Width, const int Height, const int Pitch, int *pScore);
#endif
    int                 RES_GetEncryptedImage(const int handle, const char *pReservedKey, unsigned char *pEncKey, IBSU_ImageData *pRawEncImage);
    int                 RES_ConvertToDecryptImage(const char *pReservedKey, unsigned char *pEncKey, IBSU_ImageData rawEncImage, IBSU_ImageData *pRawDecImage);
	int					RES_CBP_GetPreviewInfo(const int handle, const char *pReservedkey, IBSU_SegmentPosition *segmentPositionArray, IBSU_SegmentPosition *segmentPositionArray_for_geo, int *segmentPositionArrayCount, BOOL *IsFingerDetected);
	int					RES_CBP_CleanUp(const int handle);
	int					RES_CBP_IsFingerOn(const int handle, BOOL *bIsFingerOn);

	// Matcher 
	int					DLL_RemoveFingerImage(const int handle, const DWORD fingerPosition);
	int					DLL_AddFingerImage(const int handle, const IBSU_ImageData image, const DWORD fingerPosition, const IBSU_ImageType fingerType, const BOOL flagForce);
	int					DLL_IsFingerDuplicated(const int handle, const IBSU_ImageData image, const DWORD fingerPosition, const IBSU_ImageType fingerType, const int securityLevel, DWORD *pMatchedPosition);
	int					DLL_IsValidFingerGeometry(const int handle, const IBSU_ImageData image, const DWORD fingerPosition, const IBSU_ImageType imageType, BOOL *pValid);
	int					DLL_IsValidFingerGeometryEx(const int handle, IBSU_SegmentPosition *segmentPositionArray, int segmentPositionArrayCount, const DWORD fIndex, BOOL *pValid);
	int					DLL_GetSecurityScore(const int securityLevel, int *pSecurityScore);
	int					DLL_GetRequiredSDKVersion(const int deviceIndex, char *minSDKVersion);
	int					DLL_SetEncryptionKey(const int handle, const unsigned char* pEncyptionKey, const IBSU_EncryptionMode encMode);
/*	int					RES_ReadMask(const int handle, const char *pReservedkey, unsigned char *pWaterImage, int WaterImageSize);
	int					RES_ReadMask_8M(const int handle, const char *pReservedkey, unsigned char *pWaterImage, int WaterImageSize, const int Sector);
	int					RES_WriteMask(const int handle, const char *pReservedKey, const BYTE *MaskData, const int MaskDataLen, int *pProgress);
	int					RES_WriteMask_8M(const int handle, const char *pReservedKey, const BYTE *MaskData, const int MaskDataLen, const int Sector, int *pProgress);
	int					RES_ResetDevice(const int handle, const char *pReservedKey);

	int					RES_InitializeCameraFlatForKOJAK(const int handle, const char *pReservedkey);
	int					RES_InitializeCameraRollForKOJAK(const int handle, const char *pReservedkey);
*/
//////////////////////////////////////////////////////////////////////////////////


	BOOL				Close(BOOL bTerminate= FALSE);

	void				DeviceCountThreadCallback(LPVOID pParameter);
	void				WindowsMessageThreadCallback(LPVOID pParameter);

#if defined(WINCE)
	BOOL				EnumerateDevices (HANDLE h, MYDEV *detail);
#endif
	CIBUsbManager*		FindUsbDeviceByIndexInList(int deviceIndex);
	CIBUsbManager*		FindUsbDeviceByIndexInTempList(int deviceIndex);
	CIBUsbManager*		FindUsbDeviceInList(int handle);
	AllocatedMemory*    FindMemblockInList(void *memblock);
	CIBUsbManager*		FindUsbDeviceByDevicePath(char *devicePath);
	CIBUsbManager*		FindUsbDeviceByInvalidDevIndex();
	void				DeleteUsbDeviceInList(int handle);
	void				DeleteUsbManagerInTempList(int deviceIndex);
    void                DeleteUsbDeviceInTempList(int deviceIndex);
//	void				DeleteUsbHandleInList(int handle);
//	void				DeleteClassMainCapture(CIBUsbManager *pIBUsbManager);
	void				DeleteMemblockInList(void *memblock);

	void				RemoveAllCommuncationBreakDevice();
	BOOL				FindAllDeviceCount();
	BOOL 				GetBusInfoByIndex(int deviceIndex, int *bus_number, int *device_address, char *device_path);
//	BOOL 				GetSysPathFromBus(int bus_number, int device_address, char *device_path);
	void				UpdateIndexInList(int dwTotalDevices);
	int					CreateAPIHandle();

	int					_GetThreadID();
	void				SendToTraceLog();
	void				InsertTraceLog(const char *category, int error, int tid, const char *format, ...);

	void				SubmitJob(LPVOID pParameter);

	static DWORD WINAPI JobProc( LPVOID p_pJobParam );
	static DWORD WINAPI JobNotifier( LPVOID p_pJobParam );

	int					Main_RegisterCallbackDeviceCount(void *callback, void *context);
	int					Main_RegisterCallbackInitProgress(void *callback, void *context);
	int					Main_RegisterCallbackAsyncOpenDevice(void *callback, void *context);
    int                 _RegisterCallbacks(CIBUsbManager *pIBUsbManager, IBSU_Events events, void *pEventName, void *pContext);
	int					Main_GenerateZoomOutImage(const IBSU_ImageData inImage, BYTE *OutImg,
								int outWidth, int outHeight, const BYTE bkColor);
    void                _flip_vertically(unsigned char *pixels, const int width, const int height, const int bytes_per_pixel);
	int					Main_GenerateDisplayImage(const IBSU_ImageData inImage, BYTE *OutImg,
								int outWidth, int outHeight, const BYTE bkColor, const IBSU_ImageFormat outFormat,
								const int outQualityLevel, const BOOL verticalFlip);
	void				_ZoomOut_with_Bilinear(unsigned char *InImg, int imgWidth, int imgHeight, unsigned char *OutImg, int outWidth, int outHeight);
	int					Main_SaveBitmapImage(LPCSTR filePath, const BYTE *imgBuffer, const DWORD width, const DWORD height,
								const int pitch, const double resX, const double resY);
#if defined(_WINDOWS) && !defined(WINCE)
	BOOL				_GetEnumeratedIndexFromSortedIndex(DWORD sortedIndex, DWORD *enumeratedIndex, int *pVid, int *pPid);
#elif defined (__linux__)
	BOOL				_GetEnumeratedIndexFromSortedIndex(DWORD sortedIndex, DWORD *enumeratedIndex);
#endif
	BOOL				_GetDevicePathByIndex(int devIndex, char *outNameBuf);
	BOOL				_GetProductNameFromEnumeration(int devIndex, char *outNameBuf);
	int					_IsValidUsbHandle(CIBUsbManager *pIBUsbManager);
	void				_RunDeviceCountThread();
	void				_RunWindowsMessageThread();
	BOOL				_FindSupportedDevices(int pid);
	BOOL				_GetDeviceNameFromPID(int pid, char *outNameBuf);
	int					_OpenDevice(int deviceIndex, LPCSTR uniformityMaskPath, BOOL useUniformityMask, BOOL asyncOpen, int *pHandle, BOOL bReserved=FALSE);
    int                 _SaveBitmapMem(BYTE *InImg, BYTE *OutImg, const DWORD width, const DWORD height,
								        const int pitch, const double resX, const double resY, const BYTE BitsPerPixel);
    int                 Main_SaveBitmapMem(const IBSU_ImageData inImage, BYTE *outBitmapBuffer,
                                           const int outWidth, const int outHeight, const BYTE bkColor, int factor);
	int					_RemoveAllVector_UsbManager();

	void				_RemoveAllVector_WorkerJob();
	void				_RemoveWorkerJobInList(CJob *pJob);
#if defined(__USE_LIBUSBX_DRIVER__)
	int					DelayedLibUSBInit( void );
#endif
	void				_ImageFlipVertically(unsigned char *pixels_buffer, const int width, const int height);
#ifdef __libusb_latest__
	int					DelayedLibUSBInit( void );
#endif

	////////////////////////////////////////////////////////////////////////////////////////
	// Reserved functions
	int					_IsValidReservedKey(const char *pReservedkey);
	int					_IsValidPartnerReservedKey(const char *pPartnerReservedkey);
    int                 _GetReservedKeyAccessLevel(const char *pReservedkey, ReservedAccessLevel *accessLevel);
	////////////////////////////////////////////////////////////////////////////////////////


public: // Inline members

	inline int		GetDeviceCount(void) const;

public:


public: // Members variables

//	HANDLE			m_hDeviceCountThread;

	HANDLE			m_hDeviceCountThreadDestroyEvent;
	HANDLE			m_hDeviceCountThreadArrivalEvent;
	HANDLE			m_hDeviceCountThreadRemoveEvent;
	HANDLE			m_hDeviceCountThreadPaintEvent;
	HANDLE			m_hDeviceCountThreadPowerSleepEvent;
	HANDLE			m_hDeviceCountThreadPowerResumeEvent;
	HANDLE			m_hDeviceCountEventArray[6];

	HANDLE			m_hWindowsMessageThreadDestroyEvent;

    BOOL            m_traceLogEnabled;

	HWND			m_hMsgWnd;
	int				m_nDeviceCount;				    // Detected USB and PPI device count
	int				m_nPPIDeviceCount;				// Detected PPI device count
//	std::vector<CMainCapture*> m_pListDevice;
//	std::map< int, CIBUsbManager* > m_handleToCaptureMap;  // Mapping from API 'handle' to CMainCapture instance

	std::vector<CIBUsbManager*> m_pListUsbManager;
	std::vector<CIBUsbManager*> m_pTempUsbManager;
//	std::vector<IBUsbHandle> m_pListUsbHandle;
#if defined(__USE_LIBUSBX_DRIVER__)
	libusb_context * m_LibUSBContext;
#endif

	std::vector<TraceLog> m_pListTraceLog;
	std::vector<CJob*> m_pListWorkerJob;
	std::vector<AllocatedMemory*> m_pListAllocatedMemory;
#ifdef _WINDOWS
	HANDLE			m_hDeviceCountThread;
	HWND			m_hClientWnd;
	HANDLE			m_hWindowsMessageThread;
#elif defined(__linux__)
	// enzyme 2013-01-30 modify To solve bug on x64-Linux (definition pthread_t for DeviceCount thread)
	pthread_t		m_hDeviceCountThread;
#ifdef __libusb_latest__
//	std::vector<struct libusb_device *> m_pUSBDevice;  // List of Watson devices found in bus scan
	libusb_context * m_LibUSBContext;
#else
//	std::vector<struct usb_device *> m_pUSBDevice;  // List of Watson devices found in bus scan
#endif
	bool			m_applicationShutdown;
#endif

	char			m_szChangedDevicePath[MAX_PATH];

	CThreadPoolMgr	*m_pThreadPoolMgr;
	ThreadParam		m_tpCommBreak;

#if defined (__IBSCAN_ULTIMATE_SDK__)
	IBSU_CallbackDeviceCount	m_Clbk_DeviceCount;
	void*						m_pContext_DeviceCount;
	IBSU_CallbackInitProgress	m_Clbk_InitProgress;
	void*						m_pContext_InitProgress;
	IBSU_CallbackAsyncOpenDevice m_Clbk_AsyncOpenDevice;
	void*						m_pContext_AsyncOpenDevice;
#elif defined (__IBSCAN_SDK__)
	IBSCAN_CallbackDeviceCount	m_Clbk_DeviceCount;
	void*						m_pContext_DeviceCount;
	IBSCAN_CallbackInitProgress	m_Clbk_InitProgress;
	void*						m_pContext_InitProgress;
	IBSU_CallbackAsyncOpenDevice m_Clbk_AsyncOpenDevice;
	void*						m_pContext_AsyncOpenDevice;
#endif
	HANDLE			m_hPipe;

	int				m_prevInput_height;
	int				m_prevInput_width;
	int				m_prevZoomout_height;
	int				m_prevzoomout_width;

	int				m_refy1_arr[__MAX_IMG_WIDTH__];
	int				m_refy2_arr[__MAX_IMG_WIDTH__];
	short			m_p_arr[__MAX_IMG_WIDTH__];
	int				m_refx1_arr[__MAX_IMG_HEIGHT__];
	int				m_refx2_arr[__MAX_IMG_HEIGHT__];
	short			m_q_arr[__MAX_IMG_HEIGHT__];

	// bilinear interpolation
	BYTE			*m_pZoomOut_InImg;
	BYTE			*m_pZoomOut_TmpImg;

	// combine image
	BYTE			*m_pCombine_Image;
	BYTE			*m_pDecryptImage;

	BOOL			m_bAliveDeviceCountThread;
    BOOL            m_bAliveWindowsMessageThread;

	BOOL			m_bOnloadLibrary;

	IBSU_ImageData			pSegmentArr[IBSU_MAX_SEGMENT_COUNT];
	IBSU_SegmentPosition	pSegmentPositionArr[IBSU_MAX_SEGMENT_COUNT];
	int						segmentArrayCount;

	IBSU_TemplateDB m_TemplateDB[IMAGE_BUFFER_COUNT];

protected: // Members variables


public:

#ifdef _WINDOWS
	DECLARE_MESSAGE_MAP()
#endif
};

////////////////////////////////////////////////////////////////////////////////

inline BOOL CIBScanUltimateApp::GetDeviceCount(void) const
	{ return m_nDeviceCount; }


/////////////////////////////////////////////////////////////////////////////

static const GUID GUID_DEVINTERFACE_LIST[] =
{
	// GUID_DEVINTERFACE_USB_DEVICE
	{ 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } },
/*
	// GUID_DEVINTERFACE_DISK
	{ 0x53f56307, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },

	// GUID_DEVINTERFACE_HID,
	{ 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } },

	// GUID_NDIS_LAN_CLASS
	{ 0xad498944, 0x762f, 0x11d0, { 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c } }

	//// GUID_DEVINTERFACE_COMPORT
	{ 0x86e0d1e0, 0x8089, 0x11d0, { 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 } },

	//// GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
	{ 0x4D36E978, 0xE325, 0x11CE, { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } },

	//// GUID_DEVINTERFACE_PARALLEL
	{ 0x97F76EF0, 0xF883, 0x11D0, { 0xAF, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x84, 0x5C } },

	//// GUID_DEVINTERFACE_PARCLASS
	{ 0x811FC6A5, 0xF728, 0x11D0, { 0xA5, 0x37, 0x00, 0x00, 0xF8, 0x75, 0x3E, 0xD1 } }
*/
};
