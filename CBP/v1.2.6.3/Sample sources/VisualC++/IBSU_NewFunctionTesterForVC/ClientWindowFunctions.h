#pragma once
#include "afxwin.h"


// CClientWindowFunctions dialog
class CIBSU_NewFunctionTesterDlg;

class CClientWindowFunctions : public CDialog
{
	DECLARE_DYNAMIC(CClientWindowFunctions)

public:
	CClientWindowFunctions(CWnd* pParent = NULL);   // standard constructor
	virtual ~CClientWindowFunctions();

	virtual BOOL OnInitDialog();

public:
	int _GetCrrentDeviceHandle();



	int			m_nOverlayHandle;

public:
	CIBSU_NewFunctionTesterDlg	*m_pMainDlg;

// Dialog Data
	enum { IDD = IDD_DIALOG_CLIENTWINDOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

public:
	BOOL m_chkUseClientWindow;
	BOOL m_chkGuideLineValue;

	afx_msg void OnBnClickedButtonSetbackgroundcolor();
	afx_msg void OnBnClickedButtonSetclientwindow();
	afx_msg void OnBnClickedCheckClientWindow();
	afx_msg void OnBnClickedCheckGuideline();
	//afx_msg void OnBnClickedButtonSetclientwindowoverlaytext();
	afx_msg void OnBnClickedButtonAddoverlaytext();
//	afx_msg void OnBnClickedButtonShowoverlayobject();
	afx_msg void OnBnClickedCheckShowoverlayobject();
	//BOOL m_chkRemoveOverlayObject;
	//BOOL m_chkRemoveAllOverlayObject;

	BOOL m_chkShowOverlayObject;
	BOOL m_chkShowAllOverlayObject;

	afx_msg void OnBnClickedCheckShowalloverlayobject();
	afx_msg void OnBnClickedButtonRemoveoverlayobject();
	afx_msg void OnBnClickedButtonRemovealloverlayobject();
	afx_msg void OnBnClickedButtonModifyoverlaytext();
	afx_msg void OnBnClickedButtonAddoverlayline();
	afx_msg void OnBnClickedButtonModifyoverlayline();
	afx_msg void OnBnClickedButtonAddoverlayquadrangle();
	afx_msg void OnBnClickedButtonModifyoverlayquadrangle();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//afx_msg void OnBnClickedButtonAddoverlayshape();
	BOOL m_chkInvalidArea;
	afx_msg void OnBnClickedCheckInvalidarea();
	CComboBox m_cbRollGuideLineWidth;
	afx_msg void OnCbnSelchangeComboRollGuideLineWidth();
};
