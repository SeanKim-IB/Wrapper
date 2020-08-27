#pragma once
#include "afxwin.h"


class CIBSU_NewFunctionTesterDlg;
// CNonCallbackFunctions dialog

class CNonCallbackFunctions : public CDialog
{
	DECLARE_DYNAMIC(CNonCallbackFunctions)

public:
	CNonCallbackFunctions(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNonCallbackFunctions();

	int			_GetCurrentDeviceHandle();

// Dialog Data
	enum { IDD = IDD_DIALOG_NONCALLBACK };

public:
	CIBSU_NewFunctionTesterDlg	*m_pMainDlg;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAsyncopendevice();
	CComboBox m_cbImageType;
	afx_msg void OnBnClickedButtonBgetinitprogress();
	afx_msg void OnBnClickedButtonClosedevice2();
	afx_msg void OnBnClickedButtonClosealldevice();
	BOOL m_chkAutoContrast;
	BOOL m_chkAutoCapture;
	BOOL m_chkIgnoreNumberofFinger;
	afx_msg void OnBnClickedButtonBegincapture();
	afx_msg void OnBnClickedButtonCancelcapture();
	afx_msg void OnBnClickedButtonTakeresultimage();
	afx_msg void OnBnClickedButtonBgetimage();
	afx_msg void OnBnClickedButtonBgetclearplatenatcapture();
	afx_msg void OnBnClickedButtonGetcontrast();
	afx_msg void OnBnClickedButtonSetcontrast();
	afx_msg void OnBnClickedCheckAutocontrast();
	afx_msg void OnBnClickedCheckAutocapture();
	afx_msg void OnBnClickedCheckTrigger();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
