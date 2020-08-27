#pragma once

class CIBScanUltimate_TenScanSampleForVCDlg;
class CFingerDisplayManager;

// CScanFingerDlg ��ȭ �����Դϴ�.

class CScanFingerDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanFingerDlg)

public:
	CScanFingerDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CScanFingerDlg();

	void Init_Layout();
	void DrawImage();
	void UpdateFingerView();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_SCAN_FINGER_DIALOG };

	CIBScanUltimate_TenScanSampleForVCDlg		*m_pParent;
	CFingerDisplayManager						*m_FingerDisplayManager;

protected:
	static const CBrush   s_brushRed;                   ///<  Red background brush for scanner
	static const CBrush   s_brushOrange;                ///<  Orange background brush for scanner
	static const CBrush   s_brushGreen;                 ///<  Green background brush for scanner
	static const CBrush   s_brushPlaten;				///<  Platen brush for scanner

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

public:
	CStatic m_FingerView;
	CStatic m_CapView;
	CColorCtrl<CBoldCtrl <CStatic> > m_strFingerQuality;
	CColorCtrl<CBoldCtrl <CStatic> > m_strFingerStatus;

	afx_msg void OnBnClickedScan10Fingers();
	afx_msg void OnBnClickedScanSelectedFinger();
	afx_msg void OnBnClickedClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
