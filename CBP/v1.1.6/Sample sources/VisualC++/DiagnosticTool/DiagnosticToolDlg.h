
// DiagnosticToolDlg.h : header file
//

#pragma once

#include "IBScanUltimateAPI.h"
#include "CBP_FP.h"
#include "ReportDlg.h"

#define MAX_TEST_COUNT		20
#define TEST_RESULT_NONE	0
#define TEST_RESULT_EXE		1
#define TEST_RESULT_PASS	2
#define TEST_RESULT_FAIL	3

#define __RUN_TEST_1__		0
#define __RUN_TEST_2__		1
#define __RUN_TEST_3__		2
#define __RUN_TEST_4__		3
#define __RUN_TEST_5__		4

typedef struct tagTestItem{
	short nID;
	unsigned char TestCount;
	unsigned char TestProgress[MAX_TEST_COUNT];
}CBPTestInfo;

// CDiagnosticToolDlg dialog
class CDiagnosticToolDlg : public CDialog
{
// Construction
public:
	CDiagnosticToolDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CDiagnosticToolDlg();

// Dialog Data
	enum { IDD = IDD_DIAGNOSTICTOOL_DIALOG };

	static const CBrush   s_background_brush;                   ///<  Red background brush for scanner
	static const CBrush   s_runtests_blue;                   ///<  Red background brush for scanner

	CBitmap m_BitmapBGND;
	CBitmap m_BitmapPASS;
	CBitmap m_BitmapFAIL;
	CBitmap m_BitmapEXE;

	CReportDlg	*m_ReportDlg;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

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
	static void CALLBACK OnEvent_PressedKeyButtons(
                          const int                deviceHandle,
                          void                     *pContext,
                          const int                pressedKeyButtons
                        );

	static UINT		_threadRunTests(LPVOID pParam);
	static UINT		_threadUpdateResource(LPVOID pParam);
	void			_RunTest1();
	void			_RunTest2();
	void			_RunTest3();
	void			_RunTest4();
	void			_RunTest5();

	void			_Init_Layout();
	void			_UpdateTestResult();

	void			_BeepFail();
	void			_BeepSuccess();
	void			_BeepOk();
	void			_BeepDeviceCommunicationBreak();
	int				_ReleaseDevice();

	CBPTestInfo		m_TestInfo;
	BOOL			m_isRunTests;

	///////////////////////////////////////////////////////////////////////////////
	// variables related to tests
	cbp_fp_callBacks m_callbacklist;
	int				m_TestIndex;
	
	// callbacks of CBP
	static void OnEvent_cbp_fp_onCalibrate(int handle);
	static void OnEvent_cbp_fp_onCapture(int handle, struct cbp_fp_grayScaleCapture *grayScaleCapture);
	static void OnEvent_cbp_fp_onClose(int handle);
	static void OnEvent_cbp_fp_onConfigure(int handle, bool configurationChange);
	static void OnEvent_cbp_fp_onEnumDevices(struct cbp_fp_device *deviceList[], int deviceListLen, char *requestID);
	static void OnEvent_cbp_fp_onError(int errorNumber, char *errorDescription);
	static void OnEvent_cbp_fp_onGetDirtiness(int handle , int dirtinessLevel);
	static void OnEvent_cbp_fp_onGetLockInfo(int handle, struct cbp_fp_lock_info *lockInfo);
	static void OnEvent_cbp_fp_onInitialize(char *requestID);
	static void OnEvent_cbp_fp_onLock(int handle, int processID);
	static void OnEvent_cbp_fp_onOpen(int handle, struct cbp_fp_scanner_connection *scanner_connection);
	static void OnEvent_cbp_fp_onPowerSave(int  handle, bool isOnPowerSave);
	static void OnEvent_cbp_fp_onPreview(int handle, struct cbp_fp_grayScalePreview *preview);
	static void OnEvent_cbp_fp_onPreviewAnalysis(int handle, struct cbp_fp_previewAnalysis *previewAnalysis);
	static void OnEvent_cbp_fp_onStopPreview(int handle);
	static void OnEvent_cbp_fp_onUninitialize(char *requestID);
	static void OnEvent_cbp_fp_onUnLock(int handle);
	static void OnEvent_cbp_fp_onWarning(int warningNumber, char *warningDescription);
	///////////////////////////////////////////////////////////////////////////////

	void GetSlapTypeCodeString(cbp_fp_slapType code, char * codeString);
	void GetCollectionTypeCodeString(cbp_fp_collectionType code, char * codeString);
	void GetCurrentSystemInfo(char *OSInfo);

// Implementation
protected:
	HICON		m_hIcon;

	CString		m_strIBSUVer;
	CString		m_strWrapperVer;
	CString		m_strProductName;
	CString		m_strSerialNumber;
	int			m_nDevHandle;

	BOOL OnEraseBkgnd(CDC* pDC);
	HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
	LRESULT OnMsg_UpdateDeviceList(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_InitWarning(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_UpdateDisplayResources(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_DeviceCommunicationBreak(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_Beep(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonRunTests();
};
