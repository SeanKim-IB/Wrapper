#pragma once
#include "afxwin.h"


// CCaptureFunctions dialog
class CIBSU_NewFunctionTesterDlg;
class CMainFunction;

class CCaptureFunctions : public CDialog
{
	DECLARE_DYNAMIC(CCaptureFunctions)

public:
	CCaptureFunctions(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCaptureFunctions();

public:
	int _GetCrrentDeviceHandleInCaptureFunction();


public:
	CIBSU_NewFunctionTesterDlg	*m_pMainDlg;


// Dialog Data
	enum { IDD = IDD_DIALOG_CAPTUREFUNCTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_cbImageType;
	virtual BOOL OnInitDialog();
//	afx_msg void OnCbnSelchangeComboImagetype();
	BOOL m_chkAutoContrast;
	BOOL m_chkAutoCapture;
	BOOL m_chkIgnoreNumberofFinger;
	afx_msg void OnBnClickedButtonBegincapture();
	afx_msg void OnBnClickedButtonCancelcapture();
	afx_msg void OnBnClickedButtonTakeresultimage();
	afx_msg void OnBnClickedButtonIscaptureavailable();
	afx_msg void OnBnClickedButtonIscaptureactive();
	afx_msg void OnBnClickedButtonIstouchfinger();
	afx_msg void OnBnClickedCheckAutocontrast();
	afx_msg void OnBnClickedCheckAutocapture();
	afx_msg void OnBnClickedCheckTrigger();
	afx_msg void OnBnClickedButtonGetcontrast();
	afx_msg void OnBnClickedButtonSetcontrast();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
