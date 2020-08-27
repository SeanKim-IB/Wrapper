// MaxDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LogViewer.h"
#include "MaxDlg.h"


// CMaxDlg 대화 상자입니다.

CString e_maxSize;

IMPLEMENT_DYNAMIC(CMaxDlg, CDialog)

CMaxDlg::CMaxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMaxDlg::IDD, pParent)
{

}

CMaxDlg::~CMaxDlg()
{
}

void CMaxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMaxDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CMaxDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CMaxDlg 메시지 처리기입니다.

BOOL CMaxDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ¿©±â¿¡ Ãß°¡ ÃÊ±âÈ­ ÀÛ¾÷À» Ãß°¡ÇÕ´Ï´Ù.
	GetDlgItem(IDC_EDIT_MAXSIZE)->SetWindowText("20000");

	return TRUE;  // return TRUE unless you set the focus to a control
	// ¿¹¿Ü: OCX ¼Ó¼º ÆäÀÌÁö´Â FALSE¸¦ ¹ÝÈ¯ÇØ¾ß ÇÕ´Ï´Ù.
}

void CMaxDlg::OnBnClickedOk()
{
	// TODO: ¿©±â¿¡ ÄÁÆ®·Ñ ¾Ë¸² Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.
	GetDlgItem(IDC_EDIT_MAXSIZE)->GetWindowText(e_maxSize);

	OnOK();
}
