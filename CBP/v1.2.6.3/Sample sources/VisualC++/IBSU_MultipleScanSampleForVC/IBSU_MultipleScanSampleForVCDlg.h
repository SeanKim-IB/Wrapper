
// IBSU_MultipleScanSampleForVCDlg.h : header file
//

#pragma once
#include <vector>
#include "IBScanUltimateApi_defs.h"
#include "afxwin.h"


#define MAX_DEVICE_COUNT	4

/// Parameter set for image acquisition.
typedef struct tagCaptureInfo
{
	CString			PreCaptureMessage;		// to display on fingerprint window
	CString			PostCaptuerMessage;		// to display on fingerprint window
	IBSU_ImageType	ImageType;				// capture mode
	int				NumberOfFinger;			// number of finger count
	CString			fingerName;				// finger name (e.g left thumbs, left index ... )
}CaptureInfo;

struct ThreadParam
{
	HANDLE			threadStarted;
	void*			pParentApp;
	int				devIndex;
};

typedef struct tagDeviceManager
{
	int			index;
	int			handle;
}DeviceManager;


// CIBSU_MultipleScanSampleForVCDlg dialog
class CIBSU_MultipleScanSampleForVCDlg : public CDialog
{
// Construction
public:
	CIBSU_MultipleScanSampleForVCDlg(CWnd* pParent = NULL);	// standard constructor

	virtual ~CIBSU_MultipleScanSampleForVCDlg();

// Dialog Data
	enum { IDD = IDD_IBSU_MULTIPLESCANSAMPLEFORVC_DIALOG };

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

private:
	static UINT		_InitializeDeviceThreadCallback( LPVOID pParam );
	void			_SetStatusBarMessage( int devIndex, LPCTSTR Format, ... );
	void			_SetImageMessage( int devIndex, LPCSTR Format, ... );
	void			_UpdateCaptureSequences();
	int				_ReleaseDevice(int deviceIndex);
	void			_BeepFail();
	void			_BeepSuccess();
	void			_BeepOk();
	void			_BeepDeviceCommunicationBreak();
	void			_SaveBitmapImage( const IBSU_ImageData &image, const CString &fingerName );
	int				_ExecuteInitializeDevice(int nRc, int devHandle, int devIndex);
	int				_GetDeviceIndexFromHandle( const int devHandle );

// Implementation
protected:
	HICON m_hIcon;

	int				m_nSelectedDevIndex;						///< Index of selected device
	int				m_nDevHandle[MAX_DEVICE_COUNT];				///< Device handle
	BOOL			m_bInitializing[MAX_DEVICE_COUNT];			///< Device initialization is in progress
	TCHAR			m_ImgSaveFolder[MAX_PATH + 1];				///< Base folder for image saving
	CString			m_ImgSubFolder;								///< Sub Folder for image sequence
	CString			m_strImageMessage[MAX_DEVICE_COUNT];
	int				m_nDeviceCount;

	std::vector< DeviceManager >	m_vecDeviceManager;
	std::vector< CaptureInfo >	m_vecCaptureSeq[MAX_DEVICE_COUNT];				///< Sequence of capture steps
	int							m_nCurrentCaptureStep[MAX_DEVICE_COUNT];			///< Current capture step

	IBSU_SdkVersion m_verInfo;
	CWinThread	*m_pInitialThread;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	LRESULT OnMsg_CaptureSeqStart(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_CaptureSeqNext(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_DeviceCommunicationBreak(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_UpdateDeviceList(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_InitWarning(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_UpdateDisplayResources(WPARAM wParam = 0, LPARAM lParam = 0);
	LRESULT OnMsg_UpdateStatusMessage(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_Beep(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_AskRecapture(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	CCheckListBox m_listUsbDevices;
	CComboBox m_cboCaptureSeq;
	BOOL m_chkAutoContrast;
	BOOL m_chkAutoCapture;
	BOOL m_chkIgnoreFingerCount;
	BOOL m_chkSaveImages;
	afx_msg void OnBnClickedBtnCaptureStart();
	afx_msg void OnBnClickedBtnCaptureStop();
	afx_msg void OnBnClickedBtnImageFolder();
	CStatic m_frameImage[MAX_DEVICE_COUNT];
	afx_msg void OnCbnSelchangeComboCaptureSeq();
	CSliderCtrl m_sliderContrast;
	afx_msg void OnNMReleasedcaptureSliderContrast(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderContrast(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheckAutoContrast();
	CStatic m_IBLogo;
	afx_msg void OnBnClickedCheckClientWindow();
	afx_msg void OnCheckchangeListDevices();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnClose();
};
