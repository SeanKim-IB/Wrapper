#pragma once
#include "afxwin.h"




// CMainFunction dialog

class CIBSU_NewFunctionTesterDlg;



class CMainFunction : public CDialog
{
	DECLARE_DYNAMIC(CMainFunction)

public:
	CMainFunction(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMainFunction();

private:
	static UINT		_InitializeDeviceThreadCallback( LPVOID pParam );
	static UINT		_InitializeDeviceExThreadCallback( LPVOID pParam );
	int				_SetRegisterCallbacksForInitDevice( const int devHandle );

public:
	int		_GetCurrentDeviceHandle();

public:
	CIBSU_NewFunctionTesterDlg *m_pMainDlg;

// Dialog Data
	enum { IDD = IDD_DIALOG_MFUNCTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButtonGetdevicecount();
//	afx_msg void OnCbnSelchangeCombo1();
//	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButtonGetsdkversion();
	afx_msg void OnBnClickedButtonGetdevicedescription();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonOpendevice();
	afx_msg void OnBnClickedButtonClosedevice();
	afx_msg void OnBnClickedButtonIsdeviceopened();
	CComboBox m_cbProperty;
//	CComboBox m_cbSetLEOperationMode;
	afx_msg void OnBnClickedButtonGetproperty();
	afx_msg void OnBnClickedButtonSetproperty();
//	afx_msg void OnBnClickedButtonGetleoperationmode();
//	afx_msg void OnBnClickedButtonSetleoperationmode();
//	afx_msg void OnBnClickedButtonGetoperableleds();
//	afx_msg void OnBnClickedButtonGetleds();
//	afx_msg void OnBnClickedButtonSetleds();
	afx_msg void OnBnClickedButtonAsyncopendevice();
	afx_msg void OnBnClickedButtonClosealldevice();
	afx_msg void OnBnClickedButtonBgetinitprogress();
	afx_msg void OnBnClickedButtonOpendeviceEx();
	BOOL m_chkAsyncOpenDeviceEx;
	afx_msg void OnBnClickedButtonImageUmaskpath();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
//	afx_msg void OnBnClickedButtonEnabletracelog();
	afx_msg void OnBnClickedCheckIbsuEnabletracelog();
	BOOL m_chkEnableTraceLog;
};
