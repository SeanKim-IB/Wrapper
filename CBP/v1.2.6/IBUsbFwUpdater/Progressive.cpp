// Progressive.cpp : implementation file
//

#include "stdafx.h"
#include "UsbFwWriter.h"
#include "Progressive.h"
#include "UsbFwWriterDlg.h"

//extern int bufSize;

// CProgressive 대화 상자입니다.
extern int e_progressiveRange;

IMPLEMENT_DYNAMIC(CProgressive, CDialog)

CProgressive::CProgressive(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressive::IDD, pParent)
{
	pParentWnd = (CUsbFwWriterDlg*)pParent;
}

CProgressive::~CProgressive()
{
}

void CProgressive::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_WATSON, m_progressiveWatson);
	DDX_Control(pDX, IDC_PROGRESS_TOTAL, m_progressiveTotal);
	DDX_Control(pDX, IDOK, m_OK);
	DDX_Control(pDX, IDCANCEL, m_Cancel);
}


BEGIN_MESSAGE_MAP(CProgressive, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDOK, &CProgressive::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CProgressive::OnBnClickedCancel)
END_MESSAGE_MAP()


// CProgressive 메시지 처리기입니다.

BOOL CProgressive::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_chknumberofprogress =0;
	m_progressiveWatson.SetRange(0,1000);
	m_progressiveTotal.SetRange(0,1000*e_progressiveRange);
	
	m_OK.SetIcon(IDI_ICON_OK, (int)BTNST_AUTO_GRAY);
	m_OK.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 30);
	m_OK.DrawTransparent(true);

	m_Cancel.SetIcon(IDI_ICON_NO, (int)BTNST_AUTO_GRAY);
	m_Cancel.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 30);
	m_Cancel.DrawTransparent(true);
	//WriteFirmware();
	//Test();

	return TRUE;  // return TRUE unless you set the focus to a control
}

/*void CProgressive::InitStaticText()
{
	  CFont* pFont;
        LONG lfWidth = 50, lfHeight = 200, lfWeight = FW_BOLD;

        pFont = GetFont(lfWidth, lfHeight, lfWeight);
        GetDlgItem(IDC_STATIC)->SetFont(pFont);

        pFont->Detach();
        delete pFont; 

}

CFont* CProgressive::GetFont(LONG IfWidth, LONG IfHeight, LONG IfWeight)
{
	  CFont* pFont;
        LOGFONT logFont;

        font = new CFont();
        memset(&logFont, 0, sizeof(LOGFONT));
		logFont.lfWidth = IfWidth;
        logFont.lfHeight = IfHeight;
        logFont.lfWeight = IfWeight;
        _tcscpy( logFont.lfFaceName, _T("사용자글자") );
        //pFont->CreateFontIndirectW(&logFont);
		pFont->CreateFontIndirectA(&logFont);

        return pFont;   
}*/



HBRUSH CProgressive::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if(pWnd->GetDlgCtrlID() == IDC_STATIC)
	{
		CFont font;

		font.CreateFont(18,0,0,0, FW_SEMIBOLD, FALSE, FALSE, 0,0,0,0,0,0, "Arial");

		CFont *Oldfont = (CFont*) pDC->SelectObject(&font);
	}

	if(pWnd->GetDlgCtrlID() == IDC_STATIC_MESSAGE)
	{
		pDC->SetTextColor(RGB(255,0,0));
		CFont font;

		font.CreateFont(18,0,0,0, FW_SEMIBOLD, FALSE, FALSE, 0,0,0,0,0,0, "Arial");

		CFont *Oldfont = (CFont*) pDC->SelectObject(&font);
	}

	return hbr;
}

void CProgressive::OnBnClickedOk()
{
	//DestroyWindow();
	OnOK();
}

void CProgressive::OnBnClickedCancel()
{
	OnCancel();
}

//BOOL CProgressive::DestroyWindow()
//{
//	// TODO: ¿©±â¿¡ Æ¯¼öÈ­µÈ ÄÚµå¸¦ Ãß°¡ ¹×/¶Ç´Â ±âº» Å¬·¡½º¸¦ È£ÃâÇÕ´Ï´Ù.
//
//
//	return CDialog::DestroyWindow();
//}
