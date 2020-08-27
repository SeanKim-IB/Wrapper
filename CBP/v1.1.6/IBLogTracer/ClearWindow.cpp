// ClearWindow.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LogViewer.h"
#include "ClearWindow.h"


// CClearWindow 대화 상자입니다.

IMPLEMENT_DYNAMIC(CClearWindow, CDialog)

CString e_clearTerm;

CClearWindow::CClearWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CClearWindow::IDD, pParent)
	, m_ClearTerm(e_clearTerm)
{

}

CClearWindow::~CClearWindow()
{
}

void CClearWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CLEARTERM, m_ClearTerm);
}


BEGIN_MESSAGE_MAP(CClearWindow, CDialog)
	ON_BN_CLICKED(IDOK, &CClearWindow::OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT_CLEARTERM, &CClearWindow::OnEnChangeEditClearterm)
END_MESSAGE_MAP()


// CClearWindow 메시지 처리기입니다.

BOOL CClearWindow::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// ¿¹¿Ü: OCX ¼Ó¼º ÆäÀÌÁö´Â FALSE¸¦ ¹ÝÈ¯ÇØ¾ß ÇÕ´Ï´Ù.
}

void CClearWindow::OnBnClickedOk()
{
	OnOK();
}

void CClearWindow::OnEnChangeEditClearterm()
{
	UpdateData(TRUE);
}
