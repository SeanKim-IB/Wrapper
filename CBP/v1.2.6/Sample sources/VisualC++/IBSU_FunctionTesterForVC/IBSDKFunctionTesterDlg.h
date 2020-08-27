
// IBSDKFunctionTesterDlg.h : header file
//

#pragma once
#include "afxcmn.h"
//#include "FormMain.h"
//#include "FormImage.h"
//#include "FormCallBack.h"
#include <vector>
#include "IBScanUltimateApi_defs.h"
#include "afxwin.h"
//#include "btnst.h"




// CIBSDKFunctionTesterDlg dialog
class CIBSDKFunctionTesterDlg : public CDialog
{
// Construction
public:
	CIBSDKFunctionTesterDlg(CWnd* pParent = NULL);	// standard constructor



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
 ** Please consider using IBSU_CallbackResultImageEx instead
    static void CALLBACK OnEvent_ResultImage(
                          const int                   deviceHandle,
                          void                        *pContext,
                          const IBSU_ImageData        image,
						  const IBSU_ImageType        imageType,
                          const IBSU_ImageData        *pSplitImageArray,
                          const int                   splitImageArrayCount
					   );
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

	CWnd*		m_pwndShow;		//tab 컨트롤을 관리

// Dialog Data
	enum { IDD = IDD_IBSDKFUNCTIONTESTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
public:
	HICON			m_hIcon_On;
	HICON			m_hIcon_Off;

protected:
	HICON m_hIcon;

	IBSU_SdkVersion	m_verInfo;
	int				m_nSelectedDevIndex;
	int				m_indexForproperty;
	BOOL			m_bBlank;
	int				m_imageType;
	int				m_leOperationMode;
	int				m_setLEDs;
	DWORD			m_bkRed;
	DWORD			m_bkGreen;
	DWORD			m_bkBlue;
	BOOL			m_guideLine;
	int				m_posX;
	int				m_posY;
	DWORD			m_bkTextRed;
	DWORD			m_bkTextGreen;
	DWORD			m_bkTextBlue;
	int				m_nRc;
	BOOL			m_bNeedClearPlaten;

    int             m_nOverlayHandle;
    int             m_nOverlayQuadrangleHandle;
	
	void			Init_Layout();

	void			_SetEditValue(int value);
	void			_SetEditFingerCount(IBSU_FingerCountState fingerCountState);



	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private: 
	int				_GetCurrDeviceHandle();
	void			_SetResultMessageOnListView(BOOL bAppend, LPCTSTR Format, ...);
	void			_SetAdditionalMessageOnListView(int row, LPCTSTR Format, ...);
	void			_SetCoordinateControl(WINDOWPLACEMENT *place, int nID, int left, int top, int width, int height);
	static UINT		_InitializeDeviceThreadCallback( LPVOID pParam );
	void			ToolTip();


private:
	int				m_nRowOnListView;
	CToolTipCtrl	m_ToolTip;
	BOOL			m_IsAppend;
public:
	CListCtrl m_listMain;
	afx_msg void OnBnClickedButtonSdkversion();
	afx_msg void OnBnClickedButtonGetdevicecount();
	afx_msg void OnBnClickedButtonDevicedescription();
	afx_msg void OnBnClickedButtonOpendevice();
	CButton m_btnCloseDevice;
	afx_msg void OnBnClickedButtonDeviceclose();
	afx_msg void OnBnClickedButtonIsdeviceopen();
	CButton m_btnIsDeviceOpened;
	afx_msg void OnBnClickedButtonGetproperty();
	CComboBox m_cbProperty;
	afx_msg void OnBnClickedButtonSetproperty();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	BOOL m_chkUseClientWindow;
	BOOL m_chkAutoContrast;
	BOOL m_chkAutoCapture;
	afx_msg void OnBnClickedButtonIscaptureavailable();
	CComboBox m_cbImageType;
	afx_msg void OnBnClickedButtonIscaptureactive();
	afx_msg void OnBnClickedButtonGetcontrast();
	afx_msg void OnBnClickedButtonSetcontrast();
	afx_msg void OnBnClickedButtonGetleoperationmode();
	afx_msg void OnBnClickedButtonSetleoperation();
	CComboBox m_cbSetLEOperationMode;
	afx_msg void OnBnClickedButtonIstouchfinger();
	afx_msg void OnBnClickedButtonGetoperableleds();
	afx_msg void OnBnClickedButtonGetleds();
	//CComboBox m_cbSetLEDs;
	afx_msg void OnBnClickedButtonSetleds();
	CStatic m_staticView;
	CEdit m_editBackColor;
	CButton m_chkBold;
	afx_msg void OnBnClickedCheckDisplayWindow();
	BOOL m_chkGuideLineValue;
	afx_msg void OnBnClickedCheckGuideline();
	BOOL m_chkIgnoreNumberofFinger;
	afx_msg void OnBnClickedCheckClearplate();
	afx_msg void OnBnClickedCheckFingercount();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedCheckAutoCapture();
	afx_msg void OnBnClickedCheck();
	CListCtrl m_listEtc;
	afx_msg void OnBnClickedButtonManualcapture();
	BOOL m_chkAppendFunctionResult;
	afx_msg void OnBnClickedCheckAppendresult();
	afx_msg void OnBnClickedButtonBackcl();
	afx_msg void OnBnClickedButtonSetDisplayOverlayText();
	afx_msg void OnClose();
	CStatic m_iconPreview;
	CStatic m_iconInitProgress;
	CStatic m_iconTakingAcquisition;
	CStatic m_iconCompleteAcquisition;
	CStatic m_iconResultImageEx;
	CStatic m_iconFingerCount;
	CStatic m_iconFingerQuality;
	CStatic m_iconDeviceCommunicationBreak;
	CStatic m_iconDeviceCount;
	CStatic m_iconClearPlatenAtCapture;
	IBSU_FingerCountState m_fingerCountState;
};
