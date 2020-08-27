
// IBScanUltimate_SampleForVCDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <vector>
#include "IBScanUltimateApi_defs.h"
#include "afxcmn.h"

typedef int (_stdcall *pIBSU_OpenDevice)(const int deviceIndex, int *pHandle);
typedef int (_stdcall *pIBSU_OpenDeviceEx)(const int  deviceIndex, LPCSTR uniformityMaskPath, const BOOL asyncOpen, int *pHandle);
typedef int (_stdcall *pIBSU_CloseDevice)(const int handle);
typedef int (_stdcall *pIBSU_GetSDKVersion)(IBSU_SdkVersion *pVerinfo);
typedef int (_stdcall *pIBSU_GetDeviceCount)(int *pDeviceCount);
typedef int (_stdcall *pIBSU_GetDeviceDescription)(const int deviceIndex, IBSU_DeviceDesc *pDeviceDesc);
typedef int (_stdcall *pIBSU_RegisterCallbacks)(const int handle, const IBSU_Events event, void *pCallbackFunction, void *pContext);
typedef int (_stdcall *pIBSU_GetProperty)(const int handle, const IBSU_PropertyId propertyId, LPSTR propertyValue);
typedef int (_stdcall *pIBSU_ReleaseCallbacks)(const int handle, const IBSU_Events events);
typedef int (_stdcall *pIBSU_SetLEDs)(const int handle, const DWORD activeLEDs);
typedef int (_stdcall *pIBSU_SetProperty)(const int handle, const IBSU_PropertyId propertyId, LPCSTR propertyValue);
typedef int (_stdcall *pIBSU_BeginCaptureImage)(const int handle, const IBSU_ImageType imageType, const IBSU_ImageResolution imageResolution, const DWORD captureOptions);
typedef int (_stdcall *pIBSU_IsCaptureActive)(const int handle, BOOL *pIsActive);
typedef int (_stdcall *pIBSU_CancelCaptureImage)(const int handle);
typedef int (_stdcall *pIBSU_UnloadLibrary)(void);
typedef int (_stdcall *pIBSU_CreateClientWindow)(const int handle, const IBSU_HWND hWindow, const DWORD left, const DWORD top, const DWORD right, const DWORD bottom);
typedef int (_stdcall *pIBSU_DestroyClientWindow)(const int handle, BOOL clearExistingInfo);
typedef int (_stdcall *pIBSU_ModifyOverlayText)(const int handle, const int overlayHandle, const char *fontName, const int fontSize, const BOOL fontBold, const char *text, const int posX, const int posY, const DWORD textColor);
typedef int (_stdcall *pIBSU_GetOperableBeeper)(const int handle, IBSU_BeeperType *pBeeperType);
typedef int (_stdcall *pIBSU_SetBeeper)(const int handle, const IBSU_BeepPattern beepPattern, const DWORD soundTone, const DWORD duration, const DWORD reserved_1, const DWORD reserved_2);
typedef int (_stdcall *pIBSU_SaveBitmapImage)(LPCSTR filePath, const BYTE *imgBuffer, const DWORD width, const DWORD height, const int pitch, const double resX, const double resY);
typedef int (_stdcall *pIBSU_WSQEncodeToFile)(LPCSTR filePath, const BYTE *image, const int width, const int height, const int pitch, const int bitsPerPixel, const int pixelPerInch, const double bitRate, const char *commentText);    
typedef int (_stdcall *pIBSU_SavePngImage)(LPCSTR filePath, const BYTE *image, const DWORD width, const DWORD height, const int pitch, const double resX, const double resY);
typedef int (_stdcall *pIBSU_SaveJP2Image)(LPCSTR filePath, const BYTE *image, const DWORD width, const DWORD height, const int pitch, const double resX, const double resY, const int fQuality);
typedef int (_stdcall *pIBSU_GetOperableLEDs)(const int handle, IBSU_LedType *pLedType, int *pLedCount, DWORD *pOperableLEDs);
typedef int (_stdcall *pIBSU_AddOverlayQuadrangle)(const int handle, int *pOverlayHandle, const int x1, const int y1, const int x2, const int y2, const int x3, const int y3, const int x4, const int y4, const int lineWidth, const DWORD lineColor);
typedef int (_stdcall *pIBSU_ModifyOverlayQuadrangle)(const int handle, const int overlayHandle, const int x1, const int y1, const int x2, const int y2, const int x3, const int y3, const int x4, const int y4, const int lineWidth, const DWORD lineColor);
typedef int (_stdcall *pIBSU_AddOverlayText)(const int handle, int *pOverlayHandle, const char *fontName, const int fontSize, const BOOL fontBold, const char *text, const int posX, const int posY, const DWORD textColor);
typedef int (_stdcall *pIBSU_GetClientWindowProperty)(const int handle, const IBSU_ClientWindowPropertyId propertyId, LPSTR propertyValue);
typedef int (_stdcall *pIBSU_SetClientDisplayProperty)(const int handle, const IBSU_ClientWindowPropertyId propertyId, LPCSTR propertyValue);
typedef int (_stdcall *pIBSU_GetNFIQScore)(const int handle, const BYTE *imgBuffer, const DWORD width, const DWORD height, const BYTE  bitsPerPixel, int *pScore);
typedef int (_stdcall *pIBSU_IsCaptureAvailable)(const int handle, const IBSU_ImageType imageType, const IBSU_ImageResolution imageResolution, BOOL *pIsAvailable);
typedef int (_stdcall *pIBSU_TakeResultImageManually)(const int handle);
typedef int (_stdcall *pIBSU_AsyncOpenDevice)(const int deviceIndex);
typedef int (_stdcall *pIBSU_RedrawClientWindow)(const int handle);
typedef int (_stdcall *pIBSU_SetContrast)(const int handle, const int contrastValue);
typedef int (_stdcall *pIBSU_IsWritableDirectory)(const char *dirpath, const BOOL needCreateSubFolder);


/// Parameter set for image acquisition.
typedef struct tagCaptureInfo
{
	CString			PreCaptureMessage;		// to display on fingerprint window
	CString			PostCaptuerMessage;		// to display on fingerprint window
	IBSU_ImageType	ImageType;				// capture mode
	int				NumberOfFinger;			// number of finger count
	CString			fingerName;				// finger name (e.g left thumbs, left index ... )
	IBSM_FingerPosition fingerPosition;		// Finger position. e.g Right Thumb, Right Index finger
}CaptureInfo;

struct ThreadParam
{
	HANDLE			threadStarted;
	void*			pParentApp;
};


// CIBScanUltimate_SampleForVCDlg dialog
class CIBScanUltimate_SampleForVCDlg : public CDialog
{
// Construction
public:
	CIBScanUltimate_SampleForVCDlg(CWnd* pParent = NULL);	// standard constructor

	virtual ~CIBScanUltimate_SampleForVCDlg();

	void InitializeLibrary();

// Dialog Data
	enum { IDD = IDD_IBSCANULTIMATE_SAMPLEFORVC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	/// Callback functions.
	/// Please refer to IBScanUltimateApi_defs.h in include folder.
	static void CALLBACK OnEvent_DeviceCommunicationBreak(
						 const int deviceHandle,
						 void*     pContext
					   );

	static void CALLBACK OnEvent_PreviewImage(
						 const int deviceHandle,
						 void*     pContext,
						 const	   IBSU_ImageData image
					   );

	static void CALLBACK OnEvent_FingerCount(
						   const int                   deviceHandle,
						   void*                       pContext,
						   const IBSU_FingerCountState fingerCountState
						 );

	static void CALLBACK OnEvent_FingerQuality(
                          const int                     deviceHandle,   
                          void                          *pContext,       
                          const IBSU_FingerQualityState *pQualityArray, 
                          const int                     qualityArrayCount    
						 );

	static void CALLBACK OnEvent_DeviceCount(
						 const int detectedDevices,
						 void      *pContext
					   );

	static void CALLBACK OnEvent_InitProgress(
						 const int deviceIndex,
						 void      *pContext,
						 const int progressValue
					   );

	static void CALLBACK OnEvent_TakingAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   );

	static void CALLBACK OnEvent_CompleteAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   );

/****
 ** This IBSU_CallbackResultImage callback was deprecated since 1.7.0
 ** Please use IBSU_CallbackResultImageEx instead
*/
    static void CALLBACK OnEvent_ResultImageEx(
                          const int                   deviceHandle,
                          void                        *pContext,
                          const int                   imageStatus,
                          const IBSU_ImageData        image,
						  const IBSU_ImageType        imageType,
						  const int                   detectedFingerCount,
                          const int                   segmentImageArrayCount,
                          const IBSU_ImageData        *pSegmentImageArray,
						  const IBSU_SegmentPosition  *pSegmentPositionArray
                        );

	static void CALLBACK OnEvent_ClearPlatenAtCapture(
                          const int                   deviceHandle,
                          void                        *pContext,
                          const IBSU_PlatenState      platenState
                        );

	static void CALLBACK OnEvent_AsyncOpenDevice(
                          const int                   deviceIndex,
                          void                        *pContext,
                          const int                   deviceHandle,
                          const int                   errorCode
                        );

	static void CALLBACK OnEvent_NotifyMessage(
                          const int                deviceHandle,
                          void                     *pContext,
                          const int                message
                        );

	static void CALLBACK OnEvent_PressedKeyButtons(
                          const int                deviceHandle,
                          void                     *pContext,
                          const int                pressedKeyButtons
                        );

private:
	static UINT		_InitializeDeviceThreadCallback( LPVOID pParam );
	void			_SetStatusBarMessage( LPCTSTR Format, ... );
	void			_SetImageMessage( LPCSTR Format, ... );
	void			_UpdateCaptureSequences();
	int				_ReleaseDevice();
	void			_BeepFail();
	void			_BeepSuccess();
	void			_BeepOk();
	void			_BeepDeviceCommunicationBreak();
	void			_SaveBitmapImage( const IBSU_ImageData &image, const CString &fingerName );
	void			_SaveWsqImage( const IBSU_ImageData &image, const CString &fingerName );
	void			_SavePngImage( const IBSU_ImageData &image, const CString &fingerName );
	void			_SaveJP2Image( const IBSU_ImageData &image, const CString &fingerName );
	int				_ExecuteInitializeDevice(int nRc, int devHandle);
    int             _ModifyOverlayForWarningOfClearPlaten(BOOL bVisible);
	void			_SetLEDs(int deviceHandle, CaptureInfo info, int ledColor, BOOL bBlink);

// Implementation
protected:
	HICON m_hIcon;

	HINSTANCE		hDll; 

	int				m_nSelectedDevIndex;						///< Index of selected device
	int				m_nDevHandle;								///< Device handle
	BOOL			m_bInitializing;							///< Device initialization is in progress
	TCHAR			m_ImgSaveFolder[MAX_PATH + 1];				///< Base folder for image saving
	CString			m_ImgSubFolder;								///< Sub Folder for image sequence
	CString			m_strImageMessage;
	BOOL			m_bNeedClearPlaten;
	BOOL			m_bBlank;
	BOOL			m_bSaveWarningOfClearPlaten;

    int             m_nOvImageTextHandle;
    int             m_nOvClearPlatenHandle;
	int				m_nOvSegmentHandle[IBSU_MAX_SEGMENT_COUNT];

	std::vector< CaptureInfo >	m_vecCaptureSeq;				///< Sequence of capture steps
	int							m_nCurrentCaptureStep;			///< Current capture step

	IBSU_SdkVersion m_verInfo;
	IBSU_FingerQualityState	m_FingerQuality[IBSU_MAX_SEGMENT_COUNT];
    IBSU_LedType    m_LedType;

	static const CBrush   s_brushRed;                   ///<  Red background brush for scanner
	static const CBrush   s_brushOrange;                ///<  Orange background brush for scanner
	static const CBrush   s_brushGreen;                 ///<  Green background brush for scanner
	static const CBrush   s_brushPlaten;				///<  Platen brush for scanner

	WINDOWPLACEMENT	m_defaultWndPlace;
	WINDOWPLACEMENT m_defaultViewPlace;
	WINDOWPLACEMENT m_defaultStatusPlace;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	LRESULT OnMsg_CaptureSeqStart(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_CaptureSeqNext(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_DeviceCommunicationBreak(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_DrawClientWindow(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_UpdateDeviceList(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_InitWarning(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_UpdateDisplayResources(WPARAM wParam = 0, LPARAM lParam = 0);
	LRESULT OnMsg_UpdateStatusMessage(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_Beep(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_DrawFingerQuality(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_AsyncOpenDevice(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_AskRecapture(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	CComboBox m_cboUsbDevices;
	CComboBox m_cboCaptureSeq;
	BOOL m_chkAutoContrast;
	BOOL m_chkAutoCapture;
	BOOL m_chkIgnoreFingerCount;
	BOOL m_chkSaveImages;
	afx_msg void OnBnClickedBtnCaptureStart();
	afx_msg void OnBnClickedBtnCaptureStop();
	afx_msg void OnBnClickedBtnImageFolder();
	CStatic m_frameImage;
	afx_msg void OnCbnSelchangeComboDevices();
	afx_msg void OnCbnSelchangeComboCaptureSeq();
	CSliderCtrl m_sliderContrast;
	afx_msg void OnNMReleasedcaptureSliderContrast(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderContrast(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheckAutoContrast();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_QualityView;
	BOOL m_chkUseClearPlaten;
	CStatic m_IBLogo;
	BOOL m_chkAsyncOpenDevice;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	BOOL m_chkUseActualWindow;
	afx_msg void OnBnClickedCheckActualwindow();
	BOOL m_chkOpenDeviceEx;
	BOOL m_chkUseNFIQ;
	afx_msg void OnClose();
	BOOL m_chkDrawSegmentImage;
	BOOL m_chkInvalidArea;
	BOOL m_chkDetectSmear;
	CComboBox m_cboSmearLevel;
	BOOL m_chkUseSuperDryMode;
};
