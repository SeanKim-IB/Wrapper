#pragma once

#define MAX_STR_LEN		512

// CReportDlg dialog

class CReportDlg : public CDialog
{
	DECLARE_DYNAMIC(CReportDlg)

public:
	CReportDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CReportDlg();

	void		_Init_Report();
	void		_Add_Report( LPCTSTR Format, ... );

// Dialog Data
	enum { IDD = IDD_DIALOG_REPORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnInitDialog();
public:
	CString m_strReport;
	afx_msg void OnBnClickedButtonSaveFile();
};
