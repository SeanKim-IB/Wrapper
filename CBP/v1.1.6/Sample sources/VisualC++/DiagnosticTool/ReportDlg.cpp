// ReportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DiagnosticTool.h"
#include "ReportDlg.h"


// CReportDlg dialog

IMPLEMENT_DYNAMIC(CReportDlg, CDialog)

CReportDlg::CReportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReportDlg::IDD, pParent)
	, m_strReport(_T(""))
{

}

CReportDlg::~CReportDlg()
{
}

void CReportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_REPORT, m_strReport);
}


BEGIN_MESSAGE_MAP(CReportDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_FILE, &CReportDlg::OnBnClickedButtonSaveFile)
END_MESSAGE_MAP()


// CReportDlg message handlers
BOOL CReportDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	_Init_Report();

	return TRUE;
}

void CReportDlg::_Init_Report()
{
	m_strReport = "";
	GetDlgItem(IDC_EDIT_REPORT)->SetWindowText(m_strReport);
}

void CReportDlg::_Add_Report( LPCTSTR Format, ... )
{
	GetDlgItem(IDC_EDIT_REPORT)->GetWindowText(m_strReport);

	TCHAR cMessage[MAX_STR_LEN+1];
	va_list arg_ptr;

	va_start( arg_ptr, Format );
	_vsntprintf( cMessage, MAX_STR_LEN, Format, arg_ptr );
	va_end( arg_ptr );  

	CString strMessage;
	strMessage.Format(_T("%s"), cMessage);
	m_strReport += strMessage;

	GetDlgItem(IDC_EDIT_REPORT)->SetWindowText(m_strReport);
}

void CReportDlg::OnBnClickedButtonSaveFile()
{
	CFileDialog dlg(FALSE, 0, 0 , 4|2, "Test Report(*.txt)|*.txt||");

	if(dlg.DoModal() == IDOK)
	{
		FILE *fp = fopen((char*)dlg.GetPathName().GetBuffer(), "wt");
		if(fp == NULL)
		{
			MessageBox("Failed to save a file", "Error message", MB_ICONERROR | MB_OK);
			return;
		}

		fprintf(fp, "%s", m_strReport);
		fclose(fp);
	}
}
