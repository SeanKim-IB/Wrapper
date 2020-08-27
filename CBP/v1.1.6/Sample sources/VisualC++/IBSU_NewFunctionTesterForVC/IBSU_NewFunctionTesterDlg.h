
// IBSU_NewFunctionTesterDlg.h : header file
//

#pragma once
#include "afxcmn.h"
//#include "FormMain.h"
//#include "FormImage.h"
//#include "FormCallBack.h"
#include <vector>
//#include "IBScanUltimateApi_defs.h"
//#include "IBScanUltimateApi.h"
#include "afxwin.h"

#include "ImageDlg.h"
//Tab Control Dialog
#include "MainFunction.h"
#include "CaptureFunctions.h"
#include "ControlFunctions.h"
#include "ClientWindowFunctions.h"
#include "NonCallbackFunctions.h"


#define NUMBER_OF_TAB			5


// CIBSU_NewFunctionTesterDlg dialog
class CIBSU_NewFunctionTesterDlg : public CDialog
{
// Construction
public:
	CIBSU_NewFunctionTesterDlg(CWnd* pParent = NULL);	// standard constructor

	CImageDlg				*m_imageDlg;

	//Tab Control class 
	CMainFunction			m_mainFunction;
	CCaptureFunctions		m_captureFunctions;
	CControlFunctions		m_controlFunctions;
	CClientWindowFunctions	m_clientWindowFunctions;
	CNonCallbackFunctions	m_nonCallbackFunctions;

	CWnd*				m_pwndShow;

// Dialog Data
	enum { IDD = IDD_IBSU_NEWFUNCTIONTESTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
	void		_SetCoordinateControl(WINDOWPLACEMENT *place, int nID, int left, int top, int width, int height);

public:
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

	int			_GetCurrDeviceHandle();


	void		Init_Layout();
	void		_SetResultMessageOnListView(BOOL bAppend, LPCTSTR Format, ...);
	void		_SetAdditionalMessageOnListView(int row, LPCSTR Format, ...);
	void		_SetEditValue(int value);

private:
	int			m_nRowOnListView;

public: 
	bool		m_IsAppend;
	bool		m_bUseNonCallback;
	HICON		m_hIcon_On;
	HICON		m_hIcon_Off;


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_listMain;
	//CListCtrl m_listEtc;
	CTabCtrl m_tab;
	afx_msg void OnTcnSelchangeTestTab(NMHDR *pNMHDR, LRESULT *pResult);
	CListCtrl m_listEtc;
	BOOL m_chkAppendFunctionResult;
	afx_msg void OnBnClickedCheckAppendresult();
	afx_msg void OnBnClickedCheckImageViewer();
	BOOL m_chkUseImageViewer;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	CStatic m_iconInitProgress;
	CStatic m_iconPreviewImage;
	CStatic m_iconTakingAcquisition;
	CStatic m_iconCompleteAcquisition;
	CStatic m_iconResultImageEx;
	CStatic m_iconFingerCount;
	CStatic m_iconFingerQuality;
	CStatic m_iconDeviceCommunicationBreak;
	CStatic m_iconDeviceCount;
	CStatic m_iconClearPlatenAtCapture;
	CStatic m_iconAsyncOpenDevice;
	CStatic m_iconNotifyMessage;
    CStatic m_iconKeyButton;
};
