
// IBScanUltimate_TenScanSampleForVCDlg.h : header file
//

#pragma once

#include "ScanFingerDlg.h"
#include "MainDlg.h"

#include <vector>
#include "IBScanUltimateApi_defs.h"

/// Parameter set for image acquisition.
typedef struct tagCaptureInfo
{
	CString			PreCaptureMessage;		// to display on fingerprint window
	CString			PostCaptuerMessage;		// to display on fingerprint window
	IBSU_ImageType	ImageType;				// capture mode
	int				NumberOfFinger;			// number of finger count
	CString			fingerName;				// finger name (e.g left thumbs, left index ... )
	int				capture_finger_idx;		// capture finger index
}CaptureInfo;

typedef struct tagDisplayWindow
{
	HWND	hWindow;
	BYTE	*image;
//	int		x;
//	int		y;
//	int		Width;
//	int		Height;
	int		imgWidth;
	int		imgHeight;
//	DWORD	bkColor;
}
DisplayWindow;

typedef struct tagOverlayText
{
	char  familyName[IBSU_MAX_STR_LEN];
	int	  size;
	BOOL  bold;
	char text[IBSU_MAX_STR_LEN];
	int  x;
	int  y;
	DWORD color;
}
OverlayText;

struct ThreadParam
{
	HANDLE			threadStarted;
	void*			pParentApp;
};

// CIBScanUltimate_TenScanSampleForVCDlg dialog
class CIBScanUltimate_TenScanSampleForVCDlg : public CDialog
{
// Construction
public:
	CIBScanUltimate_TenScanSampleForVCDlg(CWnd* pParent = NULL);	// standard constructor
	~CIBScanUltimate_TenScanSampleForVCDlg();

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
	LRESULT OnMsg_AskRecapture(WPARAM wParam, LPARAM lParam);
	
	static UINT		_InitializeDeviceThreadCallback( LPVOID pParam );
	int				_ReleaseDevice();
	void			_BeepFail();
	void			_BeepSuccess();
	void			_BeepOk();
	void			_BeepDeviceCommunicationBreak();
	void			_CaptureStop();
	void			_SetImageMessage( LPCSTR Format, ... );
	void			_SetStatusBarMessage( LPCTSTR Format, ... );
	char*			_GetWarningMessage(int warning_msg);
	char*			_GetWarningMessageShort(int warning_msg);

	void Init_Layout();
	void ChangeView(int view);

	CMainDlg		m_MainDlg;
	CScanFingerDlg	m_ScanFingerDlg;
	CWnd			*m_pWnd;
	UINT			m_CurrentTabIdx;
	int				m_FingerQuality[4];
	IBSU_SdkVersion m_verInfo;

	int				m_nSelectedDevIndex;						///< Index of selected device
	int				m_nDevHandle;								///< Device handle
	BOOL			m_bInitializing;							///< Device initialization is in progress
	BOOL			m_chkUseClearPlaten;
	BOOL			m_bNeedClearPlaten;

	std::vector< CaptureInfo >	m_vecCaptureSeq;				///< Sequence of capture steps
	int				m_nCurrentCaptureStep;			///< Current capture step
	
	BOOL			m_bBlank;

	//For resultEX
	int									m_nOvImageTextHandle;
    int									m_nOvClearPlatenHandle;
	int				m_nOvSegmentHandle[IBSU_MAX_SEGMENT_COUNT];	
	////

// Dialog Data
	enum { IDD = IDD_IBSCANULTIMATE_TENSCANSAMPLEFORVC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_TabCtrl;
	afx_msg void OnTcnSelchangeTabView(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClose();
};
