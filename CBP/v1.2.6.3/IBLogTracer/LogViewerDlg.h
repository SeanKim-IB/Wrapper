
// LogViewerDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include <vector>
#include "afxwin.h"
using namespace std;
#define  LIMITE	50000
//#define WM_TRAYICON_MSG	WM_USER+1
#define MYWM_NOTIFYICON	WM_USER+1

class CLogData
{
public:
	CLogData(CString str_log)
	{
		int iCur=0;
		CString seperator = _T("^");
		m_COLUMN_DATE = str_log.Tokenize(seperator, iCur);
		m_COLUMN_CATEGORY = str_log.Tokenize(seperator, iCur);
		m_COLUMN_LOG_CODE = str_log.Tokenize(seperator, iCur);
		m_COLUMN_ERROR = str_log.Tokenize(seperator, iCur);
		m_COLUMN_THREAD = str_log.Tokenize(seperator, iCur);
		m_COLUMN_MESSAGE = str_log.Tokenize(seperator, iCur);
	}

	CString m_COLUMN_DATE;
	CString m_COLUMN_CATEGORY;
	CString m_COLUMN_LOG_CODE;
	CString m_COLUMN_ERROR;
	CString m_COLUMN_THREAD;
	CString m_COLUMN_MESSAGE;
};

// CLogViewerDlg dialog
class CLogViewerDlg : public CDialog
{
// Construction
public:
	CLogViewerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LOGVIEWER_DIALOG };

	void _InsertLog(CString str_log);
	void _RefreshLog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	static void LogListenerThreadProc(LPVOID /*data*/);

	HANDLE	hThread;

	//This is the database we using
	vector<CLogData>	m_database;
	UINT				m_Old_Log_Count;

// Implementation
protected:
	HICON m_hIcon;

	//bool		m_bTrayStatus;



	void		TraySetting(void);
	//LRESULT		TrayIconMessage(WPARAM wParam, LPARAM lParam);
	LRESULT		onTrayNotify(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOdfinditemList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOdcachehintList(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

private:
	int			m_count;
	CString		m_filePath;
	FILE		*m_fp;
	int			m_temp;
	CMenu		m_TrayMenu;
	int			m_clearCount;
	CString		m_fileName;
	int			m_test;
	int			m_DBLogCount;
	TCHAR		CurPath[MAX_PATH];


	void		GetFileName();
	void		InitLayout();
	void		MakePipe();


public:
	CListCtrl m_LogList;
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnMenuExit();
	afx_msg void OnMenuShow();
	BOOL m_chkStatusBar;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CButton m_btnClearWindow;
	afx_msg void OnMenuClearwindow();
	afx_msg void OnMenuOpenfilefolder();
	afx_msg void OnMenuShownumberofitem();
	afx_msg void OnMenuLogfiles();
	afx_msg void OnMenuOptionforclearwindow();
	afx_msg void OnUpdateMenuLogfiles(CCmdUI *pCmdUI);
};
