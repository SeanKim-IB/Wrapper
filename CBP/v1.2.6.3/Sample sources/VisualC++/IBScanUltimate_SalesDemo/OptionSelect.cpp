// OptionSelect.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "IBScanUltimate_SalesDemo.h"
#include "OptionSelect.h"
#include "IBScanUltimate_SalesDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// OptionSelect 대화 상자입니다.

IMPLEMENT_DYNAMIC(OptionSelect, CDialog)

OptionSelect::OptionSelect(CWnd* pParent /*=NULL*/)
	: CDialog(OptionSelect::IDD, pParent)
	, m_chkUseBitmap(TRUE)
	, m_chkUseWsq(TRUE)
	, m_chkUseIso(TRUE)
	, m_chkUseNFIQ(FALSE)
	, m_miniNfiq(1)
	, m_chkRightThumb(FALSE)
	, m_chkRightIndex(FALSE)
	, m_chkRightMiddle(FALSE)
	, m_chkRightRing(FALSE)
	, m_chkRightLittle(FALSE)
	, m_chkLeftThumb(FALSE)
	, m_chkLeftIndex(FALSE)
	, m_chkLeftMiddle(FALSE)
	, m_chkLeftRing(FALSE)
	, m_chkLeftLittle(FALSE)
	, m_chkDuplication(FALSE)
	, m_SmearD(FALSE)
	, m_chkAutoCapture(TRUE)
	, m_chkViewNFIQ(TRUE)
	, m_chkViewNFIQ2(FALSE)
	, m_WetFingerD(FALSE)
	, m_WetDLevel(3)
	, m_chkSpoofD(FALSE)
	, m_SpoofLevel(5)
{

}

OptionSelect::~OptionSelect()
{
}

void OptionSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHK_BITMAP ,m_chkUseBitmap);
	DDX_Check(pDX, IDC_CHK_WSQ ,m_chkUseWsq);
	DDX_Check(pDX, IDC_CHK_ISO ,m_chkUseIso);
	DDX_Check(pDX, IDC_CHECK_NFIQ ,m_chkUseNFIQ);
	DDX_Text(pDX, IDC_EDIT1, m_miniNfiq);
	DDX_Check(pDX, IDC_CHK_A1, m_chkRightThumb);
	DDX_Check(pDX, IDC_CHK_A2, m_chkRightIndex);
	DDX_Check(pDX, IDC_CHK_A3, m_chkRightMiddle);
	DDX_Check(pDX, IDC_CHK_A4, m_chkRightRing);
	DDX_Check(pDX, IDC_CHK_A5, m_chkRightLittle);

	DDX_Check(pDX, IDC_CHK_A6, m_chkLeftThumb);
	DDX_Check(pDX, IDC_CHK_A7, m_chkLeftIndex);
	DDX_Check(pDX, IDC_CHK_A8, m_chkLeftMiddle);
	DDX_Check(pDX, IDC_CHK_A9, m_chkLeftRing);
	DDX_Check(pDX, IDC_CHK_A10, m_chkLeftLittle);
	DDX_Check(pDX, IDC_CHECK2, m_chkDuplication);
	DDX_Check(pDX, IDC_CHECK3, m_SmearD);
	DDX_Check(pDX, IDC_CHECK_AUTO_CAPTURE, m_chkAutoCapture);
	DDX_Check(pDX, IDC_CHECK1, m_chkViewNFIQ);
	DDX_Check(pDX, IDC_CHECK5, m_chkViewNFIQ2);
	DDX_Check(pDX, IDC_CHECK_WET_DETECT, m_WetFingerD);
	DDX_Control(pDX, IDC_COMBO_WET_LEVEL, m_cboWetLevel);
	DDX_Control(pDX, IDC_COMBO_SPOOF_LEVEL, m_cboSpoofLevel);
	DDX_Check(pDX, IDC_CHECK_SPOOF_DETECT, m_chkSpoofD);
}


BEGIN_MESSAGE_MAP(OptionSelect, CDialog)
	ON_BN_CLICKED(IDC_BTN_Accept, &OptionSelect::OnBnClickedBtnAccept)
	ON_BN_CLICKED(IDC_BUTTON1, &OptionSelect::OnBnClickedButton1)
	ON_CBN_SELCHANGE(IDC_COMBO_WET_LEVEL, &OptionSelect::OnCbnSelchangeComboWetLevel)
END_MESSAGE_MAP()

BOOL OptionSelect::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_cboWetLevel.InsertString(0, "1");
	m_cboWetLevel.InsertString(1, "2");
	m_cboWetLevel.InsertString(2, "3");
	m_cboWetLevel.InsertString(3, "4");
	m_cboWetLevel.InsertString(4, "5");
	m_cboWetLevel.SetCurSel(m_WetDLevel-1);

	m_cboSpoofLevel.InsertString(0, "0");
	m_cboSpoofLevel.InsertString(1, "1");
	m_cboSpoofLevel.InsertString(2, "2");
	m_cboSpoofLevel.InsertString(3, "3");
	m_cboSpoofLevel.InsertString(4, "4");
	m_cboSpoofLevel.InsertString(5, "5");
	m_cboSpoofLevel.InsertString(6, "6");
	m_cboSpoofLevel.InsertString(7, "7");
	m_cboSpoofLevel.InsertString(8, "8");
	m_cboSpoofLevel.InsertString(9, "9");
	m_cboSpoofLevel.InsertString(10, "10");
	m_cboSpoofLevel.SetCurSel(m_SpoofLevel);	// Level * 100 = SpoofThres

	return TRUE;
}

void OptionSelect::OnBnClickedBtnAccept()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if(m_pParent->m_OptionDlg.m_chkAutoCapture == TRUE)
	{
		m_pParent->m_ScanFingerDlg.m_btnscan.LoadBitmaps(IDB_ENABLE_SCAN,IDB_FOCUS_SCAN,NULL,NULL);
		m_pParent->m_ScanFingerDlg.m_btnscan.SizeToContent();
		m_pParent->m_ScanFingerDlg.m_btnscan.Invalidate();
	}
	else
	{
		m_pParent->m_ScanFingerDlg.m_btnscan.LoadBitmaps(IDB_ENABLE_MANUAL_CAPTURE,IDB_FOCUS_MANUAL_CAPTURE,NULL,NULL);
		m_pParent->m_ScanFingerDlg.m_btnscan.SizeToContent();
		m_pParent->m_ScanFingerDlg.m_btnscan.Invalidate();
	}

	m_pParent->m_MainDlg.DrawImage();

	m_pParent->m_OptionDlg.m_WetDLevel = m_pParent->m_OptionDlg.m_cboWetLevel.GetCurSel()+1;

	//OnOK();
	DestroyWindow();
	
}

void OptionSelect::OnBnClickedButton1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	DestroyWindow();
}

// OptionSelect 메시지 처리기입니다.

void OptionSelect::OnCbnSelchangeComboWetLevel()
{
	m_WetDLevel = m_cboWetLevel.GetCurSel()+1;
}
