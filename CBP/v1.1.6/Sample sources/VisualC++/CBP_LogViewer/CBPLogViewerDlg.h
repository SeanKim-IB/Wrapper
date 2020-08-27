
// CBPLogViewerDlg.h : header file
//

#pragma once
#include "afxwin.h"
//#include "ColorListBox.h"
#include "afxcmn.h"
#include <vector>

typedef struct {
	unsigned char error_flag;
	CString text;
} CBPLogs;

// CCBPLogViewerDlg dialog
class CCBPLogViewerDlg : public CDialog
{
// Construction
public:
	CCBPLogViewerDlg(CWnd* pParent = NULL);	// standard constructor

	void _AalysisLog(FILE *fp);
	void _AalysisLog_rev1(FILE *fp);
	void _AalysisLog_rev2(FILE *fp);  // add time gap from score75 to stopimaging and from capture and onCapture

	void _AddLog(char *str, BOOL newline, int error = 0);

	long _GetTimeStampToInt(char *str);

	int m_EndofList;
	CString m_FileVersion;
// Dialog Data
	enum { IDD = IDD_CBPLOGVIEWER_DIALOG };

	std::vector<CBPLogs> m_database;
	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonOpenLog();
	//CColorListBox m_listFilteredLog;
	CListCtrl m_listFilteredLog;
	CString m_Summary;
	afx_msg void OnBnClickedButtonSaveBriefLog();
	CProgressCtrl m_progressLog;
};
