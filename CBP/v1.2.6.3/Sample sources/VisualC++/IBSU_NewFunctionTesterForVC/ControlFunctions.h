#pragma once
#include "afxwin.h"


// CControlFunctions dialog

class CIBSU_NewFunctionTesterDlg;

class CControlFunctions : public CDialog
{
	DECLARE_DYNAMIC(CControlFunctions)

public:
	CControlFunctions(CWnd* pParent = NULL);   // standard constructor
	virtual ~CControlFunctions();

public:
	int	_GetCurrentDeviceHandleInControlFunctions();

public:
	CIBSU_NewFunctionTesterDlg		*m_pMainDlg;

// Dialog Data
	enum { IDD = IDD_DIALOG_CONTROL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CComboBox m_cbOperationMode;
	afx_msg void OnBnClickedButtonGetleoperationmode();
	afx_msg void OnBnClickedButtonSetleoperationmode();
	afx_msg void OnBnClickedButtonGetoperableleds();
	afx_msg void OnBnClickedButtonGetleds();
	afx_msg void OnBnClickedButtonSetleds();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnBnClickedButtonGetoperablebeeper();
    afx_msg void OnBnClickedButtonSetbeeper();
    CComboBox m_cbBeepPattern;
};
