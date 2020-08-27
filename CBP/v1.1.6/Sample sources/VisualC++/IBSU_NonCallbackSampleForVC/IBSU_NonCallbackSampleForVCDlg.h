
// IBScanUltimate_SampleForVCDlg.h : header file
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


// CIBSU_NonCallbackSampleForVCDlg dialog
class CIBSU_NonCallbackSampleForVCDlg : public CDialog
{
// Construction
public:
	CIBSU_NonCallbackSampleForVCDlg(CWnd* pParent = NULL);	// standard constructor

	virtual ~CIBSU_NonCallbackSampleForVCDlg();

// Dialog Data
	enum { IDD = IDD_IBSU_NONCALLBACKSAMPLEFORVC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	int				m_BpollingTimeGetImage;
	int				m_nPollingCount;
	IBSU_ImageData	m_image;
	IBSU_ImageType	m_imageType;
	IBSU_ImageData	m_segmentImageArray[IBSU_MAX_SEGMENT_COUNT];
	IBSU_SegmentPosition	m_segmentPositionArray[IBSU_MAX_SEGMENT_COUNT];
	int				m_segmentImageArrayCount;
	IBSU_FingerCountState	m_fingerCountState;
	int				m_qualityArrayCount;
	IBSU_FingerCountState	m_saveFingerCountState;


private:
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
	int				_WaitingForFinishInitDevice( const int devIndex );
	void			_SetLEDs(int deviceHandle, CaptureInfo info, int ledColor, BOOL bBlink);

// Implementation
protected:
	HICON m_hIcon;

	int				m_nSelectedDevIndex;						///< Index of selected device
	int				m_nDevHandle;								///< Device handle
	BOOL			m_bInitializing;							///< Device initialization is in progress
	TCHAR			m_ImgSaveFolder[MAX_PATH + 1];				///< Base folder for image saving
	CString			m_ImgSubFolder;								///< Sub Folder for image sequence
	CString			m_strImageMessage;
	BOOL			m_bNeedClearPlaten;
	BOOL			m_bBlank;
	int				m_nDeviceCount;

    int             m_nOvImageTextHandle;
    int             m_nOvClearPlatenHandle;
	int				m_nOvSegmentHandle[IBSU_MAX_SEGMENT_COUNT];

	std::vector< CaptureInfo >	m_vecCaptureSeq;				///< Sequence of capture steps
	int							m_nCurrentCaptureStep;			///< Current capture step

	IBSU_SdkVersion m_verInfo;
	IBSU_FingerQualityState	m_FingerQuality[IBSU_MAX_SEGMENT_COUNT];
    IBSU_LedType    m_LedType;

	IBSU_RollingState	m_rollingState;
	int					m_rollingLineX;

	static const CBrush   s_brushRed;                   ///<  Red background brush for scanner
	static const CBrush   s_brushOrange;                ///<  Orange background brush for scanner
	static const CBrush   s_brushGreen;                 ///<  Green background brush for scanner
	static const CBrush   s_brushPlaten;				///<  Platen brush for scanner

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
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	BOOL m_chkUseNFIQ;
	afx_msg void OnBnClickedCheckClearPlaten();
	afx_msg void OnClose();
	BOOL m_chkDrawSegmentImage;
	BOOL m_chkInvalidArea;
	BOOL m_chkDetectSmear;
	CComboBox m_cboSmearLevel;
};
