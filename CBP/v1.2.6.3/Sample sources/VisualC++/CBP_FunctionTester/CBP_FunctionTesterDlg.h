
// CBP_FunctionTesterDlg.h : header file
//

#pragma once

#include "CBP_FP.h"
#include "afxwin.h"
#include <vector>
#include "afxcmn.h"

// CCBP_FunctionTesterDlg dialog
class CCBP_FunctionTesterDlg : public CDialog
{
// Construction
public:
	CCBP_FunctionTesterDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CCBP_FunctionTesterDlg();

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
	int CCBP_FunctionTesterDlg::_SaveBitmapImage(char *filename, unsigned char *buffer, int width, int height, int pitch, int invert);

// Dialog Data
	enum { IDD = IDD_CBP_FUNCTIONTESTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	std::vector <cbp_fp_device>	m_cbp_devices;
	int m_devHandle;
	cbp_fp_callBacks m_callbacklist;
	cbp_fp_previewAnalysis m_PreviewAnalysis;

	BYTE *m_Image;
	BYTE *m_disp_buf;
	BITMAPINFO *m_ImageInfo;
	int m_ImageWidth;
	int m_ImageHeight;
	int m_DispWidth;
	int m_DispHeight;
	int m_OpenMode;

	double starttime;
	double endtime;

// Implementation
protected:
	HICON m_hIcon;

	void _ADD_LOG(CString remark, BOOL newStart = FALSE);
	void GetAnalysisCodeString(int code, char * codeString);
	void GetSlapTypeCodeString(cbp_fp_slapType code, char * codeString);
	void GetCollectionTypeCodeString(cbp_fp_collectionType code, char * codeString);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonCbpFpInitialize();
	afx_msg void OnBnClickedButtonCbpFpEnumdevices();
	afx_msg void OnBnClickedButtonCbpFpOpen();
	CComboBox m_cboDevices;
	afx_msg void OnBnClickedButtonCbpFpLock();
	afx_msg void OnBnClickedButtonCbpFpRegisterCallbacks();
	afx_msg void OnBnClickedButtonCbpFpCalibrate();
	afx_msg void OnBnClickedButtonCbpFpGetLockinfo();
	afx_msg void OnBnClickedButtonCbpFpStartImaging();
	afx_msg void OnBnClickedButtonCbpFpCapture();
	afx_msg void OnBnClickedButtonCbpFpUnlock();
	afx_msg void OnBnClickedButtonCbpFpClose();
	afx_msg void OnBnClickedButtonCbpFpUninitialize();
	CListCtrl m_ListLog;
	afx_msg void OnBnClickedButtonCbpFpStopImaging();
	CStatic m_DisplayView;
	afx_msg void OnBnClickedButtonCbpFpConfigure();
	afx_msg void OnBnClickedButtonCbpFpGetDirtiness();
	afx_msg void OnBnClickedButtonCbpFpGetProperty();
	afx_msg void OnBnClickedButtonCbpFpSetProperty();
	CComboBox m_cboProperty;
	afx_msg void OnBnClickedButtonCbpFpPowerSave();
	CComboBox m_cboSequence;
	CComboBox m_cboCollectType;
	afx_msg void OnBnClickedRadioMode1();
	afx_msg void OnBnClickedRadioMode2();
	afx_msg void OnBnClickedRadioMode3();
	afx_msg void OnClose();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnMenuSaveImage();
};
