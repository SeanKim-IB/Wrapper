#pragma once


// CIBScanUltimate_SimpleScanWMSplashDlg dialog

class CIBScanUltimate_SimpleScanWMSplashDlg : public CDialog
{
	DECLARE_DYNAMIC(CIBScanUltimate_SimpleScanWMSplashDlg)

public:
	CIBScanUltimate_SimpleScanWMSplashDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CIBScanUltimate_SimpleScanWMSplashDlg();

// Dialog Data
	enum { IDD = IDD_IBSCANULTIMATE_SIMPLESCANWM_SPLASH_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
