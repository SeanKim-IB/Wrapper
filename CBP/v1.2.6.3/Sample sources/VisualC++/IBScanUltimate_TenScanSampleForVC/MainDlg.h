#pragma once

#include "ZoomDlg.h"

class CIBScanUltimate_TenScanSampleForVCDlg;

// CMainDlg 대화 상자입니다.

class CMainDlg : public CDialog
{
	DECLARE_DYNAMIC(CMainDlg)

public:
	CMainDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CMainDlg();

	void Init_Layout();
	void OneFinger_Layout();
	void TwoFinger_Layout();

	void DrawImage();
	void _UpdateCaptureSequences();
	static UINT _InitializeDeviceThreadCallback( LPVOID pParam );

	int SaveWSQ(CString filename, unsigned char *buffer, int width, int height, int pitch);

// 대화 상자 데이터입니다.
	enum { IDD = IDD_MAIN_DIALOG };

	CIBScanUltimate_TenScanSampleForVCDlg	*m_pParent;
	UINT				m_CaptureMode;
	UINT				m_CaptureFinger;
	BOOL				m_CaptureCompleted;
	BOOL				m_Run_AutoSplit;
	
	BYTE				*m_ImgBuf[IMAGE_BUFFER_COUNT];
	BYTE				*m_ImgBuf_S[IMAGE_BUFFER_COUNT];
	BITMAPINFO			*m_Info;
	int					m_nCurrWidth;
	int					m_nCurrHeight;
	int					m_nCurrentWidthForSplit[IMAGE_BUFFER_COUNT];
	int					m_nCurrentHeightForSplit[IMAGE_BUFFER_COUNT];
	BYTE				m_NFIQBuf[IMAGE_BUFFER_COUNT];
	CZoomDlg			*m_ZoomDlg;

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

public:
	CComboBox m_DeviceList;
	CComboBox m_SequenceList;
	CButtonST m_btnCapture;
	CButtonST m_btnEdit;
	CButtonST m_btnSave;
	BOOL m_UseAutoSplit;
	CColorCtrl<CBoldCtrl <CStatic> > m_strDeviceList;
	CColorCtrl<CBoldCtrl <CStatic> > m_strCaptureSeq;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView0;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView1;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView2;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView3;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView4;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView5;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView6;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView7;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView8;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView9;

	afx_msg void OnCbnSelchangeDeviceList();
	afx_msg void OnCbnSelchangeSequenceList();
	afx_msg void OnBnClickedStartCapture();
	afx_msg void OnBnClickedEditCapture();
	afx_msg void OnBnClickedSaveCapture();
	afx_msg void OnBnClickedUseAutoSplit();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};
