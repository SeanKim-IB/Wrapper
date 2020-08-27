
// CBP_SampleForVCDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "CBP_FP.h"
#include <vector>

typedef struct tag_CaptureSeq
{
	cbp_fp_slapType			slap_type;
	cbp_fp_collectionType	collect_type;
} CaptureSeq;

// CCBP_SampleForVCDlg dialog
class CCBP_SampleForVCDlg : public CDialog
{
// Construction
public:
	CCBP_SampleForVCDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CCBP_SampleForVCDlg();

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

	void _DisplayImage();

// Dialog Data
	enum { IDD = IDD_CBP_SAMPLEFORVC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	std::vector <cbp_fp_device>	m_cbp_devices;
	std::vector <CaptureSeq>	m_cbp_seq;
	int m_nCurrentCaptureStep;
	int m_devHandle;
	cbp_fp_callBacks m_callbacklist;
	cbp_fp_previewAnalysis m_PreviewAnalysis;
	cbp_fp_slapType m_slaptype;
	cbp_fp_collectionType m_collectiontype;

	BYTE *m_Image;
	BYTE *m_disp_buf;
	BITMAPINFO *m_ImageInfo;
	int m_ImageWidth;
	int m_ImageHeight;
	int m_DispWidth;
	int m_DispHeight;
	BOOL m_ProgramDestroyed;

// Implementation
protected:
	HICON m_hIcon;

	void _ADD_LOG(CString log, CString remark, BOOL newStart = FALSE);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_frameImage;
	CComboBox m_cboDevices;
	CComboBox m_cboSequence;
	CComboBox m_cboCollectType;
	afx_msg void OnBnClickedButtonRefresh();
	CListCtrl m_ListLog;
	afx_msg void OnBnClickedButtonStartImaging();
	afx_msg void OnBnClickedButtonStopImaging();
	afx_msg void OnBnClickedButtonCaptureImage();
	afx_msg void OnBnClickedButtonStartScenario();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	BOOL m_chkAutoCapture;
	afx_msg void OnBnClickedCheckAutomaticCapture();
};
