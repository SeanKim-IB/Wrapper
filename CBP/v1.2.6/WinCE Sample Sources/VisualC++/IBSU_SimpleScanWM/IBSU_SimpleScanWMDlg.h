
// IBScanUltimate_SimpleScanWMDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <vector>
#include "IBScanUltimateApi_defs.h"
#include "afxcmn.h"



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




// CIBScanUltimate_SimpleScanWMDlg dialog
class CIBScanUltimate_SimpleScanWMDlg : public CDialog
{
// Construction
public:
	CIBScanUltimate_SimpleScanWMDlg(CWnd* pParent = NULL);	// standard constructor

	virtual ~CIBScanUltimate_SimpleScanWMDlg();

// Dialog Data
	enum { IDD = IDD_IBSCANULTIMATE_SIMPLESCANWM_DIALOG };

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

private:
	static UINT		_InitializeDeviceThreadCallback( LPVOID pParam );
	void			_SetStatusBarMessage( LPCTSTR Format, ... );
	void			_SetImageMessage( LPCTSTR Format, ... );
	int				_ReleaseDevice();
	void			_BeepFail();
	void			_BeepSuccess();
	void			_BeepOk();
	void			_BeepDeviceCommunicationBreak();
	void			_SaveBitmapImage( const IBSU_ImageData &image, const CString &fingerName );
	void			_SaveWsqImage( const IBSU_ImageData &image, const CString &fingerName );
	int				_ExecuteInitializeDevice(int nRc, int devHandle);
    int             _ModifyOverlayForWarningOfClearPlaten(BOOL bVisible);

// Implementation
protected:
	HICON m_hIcon;

	CRect m_rect;

	int				m_nSelectedDevIndex;						///< Index of selected device
	int				m_nDevHandle;								///< Device handle
	BOOL			m_bInitializing;							///< Device initialization is in progress
	TCHAR			m_ImgSaveFolder[MAX_PATH + 1];				///< Base folder for image saving
	CString			m_ImgSubFolder;								///< Sub Folder for image sequence
	CString			m_strImageMessage;
	BOOL			m_bNeedClearPlaten;
	BOOL			m_bBlank;
	TCHAR			m_UMFolder[MAX_PATH + 1];				    ///< Base folder for UnfiformityMask saving

    int             m_nOvImageTextHandle;
    int             m_nOvClearPlatenHandle;
	int				m_nOvSegmentHandle[IBSU_MAX_SEGMENT_COUNT];
	std::vector< CaptureInfo >	m_vecCaptureSeq;				///< Sequence of capture steps
	int							m_nCurrentCaptureStep;			///< Current capture step

	IBSU_SdkVersion m_verInfo;
	IBSU_FingerQualityState	m_FingerQuality[IBSU_MAX_SEGMENT_COUNT];


	static const CBrush   s_brushRed;                   ///<  Red background brush for scanner
	static const CBrush   s_brushOrange;                ///<  Orange background brush for scanner
	static const CBrush   s_brushGreen;                 ///<  Green background brush for scanner
	static const CBrush   s_brushPlaten;				///<  Platen brush for scanner

#ifdef WINCE
	// Since WINCE doesn't support this struct build a subset of it for use on WINCE/Windows Mobile 
	struct _WINDOWPLACEMENT {
		RECT rcNormalPosition;
	};
	typedef _WINDOWPLACEMENT WINDOWPLACEMENT;
#endif
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
	LRESULT OnMsg_AsyncOpenDevice(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	CComboBox m_cboUsbDevices;
	BOOL m_chkSaveImages;
	afx_msg void OnBnClickedBtnCaptureStart();
	afx_msg void OnBnClickedBtnCaptureStop();
	CStatic m_frameImage;
	afx_msg void OnCbnSelchangeComboDevices();
	afx_msg void OnCbnSelchangeComboCaptureSeq();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	BOOL m_chkAsyncOpenDevice;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	BOOL m_chkOpenDeviceEx;
	BOOL m_chkUseNFIQ;
	afx_msg void OnClose();
	BOOL m_chkDrawSegmentImage;
	BOOL m_chkUseClearPlaten;
};
