// OneFingerDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "IBScanUltimate_SalesDemo.h"
#include "ScanFingerDlg.h"
#include "IBScanUltimate_SalesDemoDlg.h"

#include "IBScanUltimateApi.h"
#include "FingerDisplayManager.h"
#include "ColorButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CScanFingerDlg 대화 상자입니다.
const CBrush   CScanFingerDlg::s_brushRed( RGB(255,0,0) );
const CBrush   CScanFingerDlg::s_brushOrange( RGB(255,128,0) );
const CBrush   CScanFingerDlg::s_brushGreen( RGB(0,128,0) );
const CBrush   CScanFingerDlg::s_brushPlaten( RGB(74,74,74) );


IMPLEMENT_DYNAMIC(CScanFingerDlg, CDialog)

CScanFingerDlg::CScanFingerDlg(CWnd* pParent /*=NULL*/)
: CDialog(CScanFingerDlg::IDD, pParent)
{
	m_CaptureStart =FALSE;
	m_CaptureEnd =FALSE;
	m_FingerDisplayManager = NULL;
	m_StartFingerCapture =FALSE;
	m_SelectFinger = TRUE;
	m_CaptureisNotCompleted =FALSE;
}

CScanFingerDlg::~CScanFingerDlg()
{
	if(m_FingerDisplayManager)
		delete m_FingerDisplayManager;
}

void CScanFingerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FINGER_VIEW, m_FingerView);
	DDX_Control(pDX, IDC_VIEW, m_CapView);
	DDX_Control(pDX, IDC_STRING_FINGER_STATUS, m_strFingerStatus);
	DDX_Control(pDX, IDC_BUTTON1, m_Button1);
	DDX_Control(pDX, IDC_BUTTON2, m_Button2);
	DDX_Control(pDX, IDC_BUTTON3, m_Button3);
	DDX_Control(pDX, IDC_BUTTON4, m_Button4);
	DDX_Control(pDX, IDC_BUTTON5, m_Button5);
	DDX_Control(pDX, IDC_BUTTON6, m_Button6);
	DDX_Control(pDX, IDC_BUTTON7, m_Button7);
	DDX_Control(pDX, IDC_BUTTON8, m_Button8);
	DDX_Control(pDX, IDC_BUTTON9, m_Button9);
	DDX_Control(pDX, IDC_BUTTON91, m_Button10);
	DDX_Control(pDX, IDC_BUTTON92, m_Button11);
	DDX_Control(pDX, IDC_BUTTON93, m_Button12);
	DDX_Control(pDX, IDC_BUTTON94, m_Button13);
	DDX_Control(pDX, IDC_BUTTON95, m_Button14);
	DDX_Control(pDX, IDC_BUTTON96, m_Button15);
	DDX_Control(pDX, IDC_BUTTON97, m_Button16);
	DDX_Control(pDX, IDC_BUTTON98, m_Button17);
	DDX_Control(pDX, IDC_SCAN_10_FINGERS, m_btnscan);
	DDX_Control(pDX, IDC_CLOSE, m_btnclose);
	DDX_Control(pDX, IDC_BUTTON99, m_Button18);
}


BEGIN_MESSAGE_MAP(CScanFingerDlg, CDialog)
	ON_BN_CLICKED(IDC_SCAN_10_FINGERS, &CScanFingerDlg::OnBnClickedScan10Fingers)
	//ON_BN_CLICKED(IDC_SCAN_SELECTED_FINGER, &CScanFingerDlg::OnBnClickedScanSelectedFinger)
	ON_BN_CLICKED(IDC_CLOSE, &CScanFingerDlg::OnBnClickedClose)
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON1, &CScanFingerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CScanFingerDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CScanFingerDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CScanFingerDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CScanFingerDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CScanFingerDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CScanFingerDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &CScanFingerDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &CScanFingerDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON91, &CScanFingerDlg::OnBnClickedButton91)
	ON_BN_CLICKED(IDC_BUTTON92, &CScanFingerDlg::OnBnClickedButton92)
	ON_BN_CLICKED(IDC_BUTTON93, &CScanFingerDlg::OnBnClickedButton93)
	ON_BN_CLICKED(IDC_BUTTON94, &CScanFingerDlg::OnBnClickedButton94)
	ON_BN_CLICKED(IDC_BUTTON95, &CScanFingerDlg::OnBnClickedButton95)
	ON_BN_CLICKED(IDC_BUTTON96, &CScanFingerDlg::OnBnClickedButton96)
	ON_BN_CLICKED(IDC_BUTTON97, &CScanFingerDlg::OnBnClickedButton97)
	ON_BN_CLICKED(IDC_BUTTON98, &CScanFingerDlg::OnBnClickedButton98)
	ON_BN_CLICKED(IDC_BUTTON99, &CScanFingerDlg::OnBnClickedButton99)
END_MESSAGE_MAP()


// CScanFingerDlg 메시지 처리기입니다.
BOOL CScanFingerDlg::PreTranslateMessage(MSG* pMsg)
{
	switch( pMsg->message )
	{
	case WM_KEYDOWN :
		{
			switch( pMsg->wParam )
			{
			case VK_ESCAPE :
			case VK_CONTROL :
			case VK_PAUSE :
			case VK_RETURN :
			case VK_CANCEL :
				return TRUE;
			}
		}
		break;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CScanFingerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	Init_Layout();

	SetTimer(1, 100, NULL);

	return TRUE;
}

void CScanFingerDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CDialog::OnPaint()을(를) 호출하지 마십시오.
	DrawImage();
	UpdateFingerView();
}

void CScanFingerDlg::DrawImage()
{
	m_pParent->m_ScanFingerDlg.SendMessage(WM_COMMAND,IDC_SCAN_10_FINGERS ,0);
}

void CScanFingerDlg::UpdateFingerView()
{
	m_FingerDisplayManager->UpdateDisplayWindow();
}

void CScanFingerDlg::Init_Layout()
{
	m_background.CreateSolidBrush(RGB(70,70,70));

	CButton* btn;
	WINDOWPLACEMENT place;

	GetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_FINGER_VIEW);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = 5;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+150;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_FINGER_STATUS);
	place.rcNormalPosition.left =350;
	place.rcNormalPosition.top = 0;
	place.rcNormalPosition.right = place.rcNormalPosition.left+700;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STATIC_QUALITY_1);
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+2;
	place.rcNormalPosition.left = 350;
	place.rcNormalPosition.right = place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+20;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STATIC_QUALITY_2);
	place.rcNormalPosition.left = place.rcNormalPosition.right+5;
	place.rcNormalPosition.right = place.rcNormalPosition.left+170;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STATIC_QUALITY_3);
	place.rcNormalPosition.left = place.rcNormalPosition.right+5;
	place.rcNormalPosition.right = place.rcNormalPosition.left+170;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STATIC_QUALITY_4);
	place.rcNormalPosition.left = place.rcNormalPosition.right+5;
	place.rcNormalPosition.right = place.rcNormalPosition.left+170;
	btn->SetWindowPlacement(&place);
	//////////////////////// BTN ///////////////////////////////////
	btn = (CButton*)GetDlgItem(IDC_SCAN_10_FINGERS);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = 160;
	place.rcNormalPosition.right = place.rcNormalPosition.left+140;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+40;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_CLOSE);
	place.rcNormalPosition.left = 170;
	place.rcNormalPosition.top = 160;//place.rcNormalPosition.bottom+10;
	place.rcNormalPosition.right = place.rcNormalPosition.left+125;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+40;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON1);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+25;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON2);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON3);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON4);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON5);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON6);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON7);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON8);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON9);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON91);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON92);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON93);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON94);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON95);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON96);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON97);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_BUTTON98);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	

	btn = (CButton*)GetDlgItem(IDC_VIEW);
	place.rcNormalPosition.left = 350;
	place.rcNormalPosition.top = 52;
	place.rcNormalPosition.right = place.rcNormalPosition.left+700;//618IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+603;//IMG_H;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_TXT_STATUS);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = 655;//place.rcNormalPosition.bottom+10;
	place.rcNormalPosition.right = place.rcNormalPosition.left+1060;//+IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	m_FingerDisplayManager = new CFingerDisplayManager(&m_FingerView, 240, 160, ONE_FINGER_MODE);

	m_strFingerStatus.SetTextColor(RGB(255,255,255));
	//m_strFingerStatus.SetBkColor(RGB(255,255,255));

	m_btnscan.LoadBitmaps(IDB_ENABLE_SCAN,IDB_FOCUS_SCAN,NULL,NULL);
	m_btnscan.SizeToContent();

	m_btnclose.LoadBitmaps(IDB_DISABLE_CLOSE,IDB_FOCUS_CLOSE,NULL,NULL);
	m_btnclose.SizeToContent();
	

}
void CScanFingerDlg::Disable10Finger()
{
	
	if(m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;
		m_Button10.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftRing)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
		m_Button9.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
		m_Button8.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		m_Button7.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
		m_Button6.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
		m_Button5.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
		m_Button4.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
		m_Button3.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
		m_Button2.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
		m_Button1.EnableWindow(FALSE);
	}
}
void CScanFingerDlg::Disable1Finger()
{
	if(m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;	
	}
	if(m_pParent->m_OptionDlg.m_chkLeftRing)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
	}
	
	if(m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightThumb && m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
		m_Button1.EnableWindow(FALSE);
	}
}

void CScanFingerDlg::Disable4Finger()
{
	if(m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;	
	}
	if(m_pParent->m_OptionDlg.m_chkLeftRing)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
	}
	
	if(m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		m_Button4.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
		m_Button3.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
		m_Button2.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
		m_Button1.EnableWindow(FALSE);
	}
}
void CScanFingerDlg::Disable2Finger()
{
	if(m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;	
	}
	if(m_pParent->m_OptionDlg.m_chkLeftRing)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
	}
	
	if(m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		
	}
	if(m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
		m_Button2.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
		m_Button1.EnableWindow(FALSE);
	}
}
void CScanFingerDlg::Disable3Finger()
{
	if(m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;	
	}
	if(m_pParent->m_OptionDlg.m_chkLeftRing)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
	}
	
	if(m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		
	}
	if(m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkLeftLittle && m_pParent->m_OptionDlg.m_chkLeftRing && m_pParent->m_OptionDlg.m_chkLeftMiddle  && m_pParent->m_OptionDlg.m_chkLeftIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;
		m_Button2.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
		m_Button4.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightLittle && m_pParent->m_OptionDlg.m_chkRightRing && m_pParent->m_OptionDlg.m_chkRightMiddle  && m_pParent->m_OptionDlg.m_chkRightIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
		m_Button1.EnableWindow(FALSE);

	}
	if(m_pParent->m_OptionDlg.m_chkRightThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
		m_Button2.EnableWindow(FALSE);
	}
}

void CScanFingerDlg::Disable13Finger()
{
	if(m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;
		m_Button13.EnableWindow(FALSE);
		
	}
	if(m_pParent->m_OptionDlg.m_chkLeftRing)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
		m_Button12.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
		m_Button11.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		m_Button10.EnableWindow(FALSE);
		
	}
	if(m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
		m_Button9.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
		m_Button8.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
		m_Button7.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
		m_Button6.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
		m_Button5.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
		m_Button4.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftLittle && m_pParent->m_OptionDlg.m_chkLeftRing && m_pParent->m_OptionDlg.m_chkLeftMiddle  && m_pParent->m_OptionDlg.m_chkLeftIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;
		m_Button1.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftThumb && m_pParent->m_OptionDlg.m_chkRightThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
		m_Button2.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightLittle && m_pParent->m_OptionDlg.m_chkRightRing && m_pParent->m_OptionDlg.m_chkRightMiddle  && m_pParent->m_OptionDlg.m_chkRightIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
		m_Button3.EnableWindow(FALSE);
	}
}

void CScanFingerDlg::Disable15Finger()
{
	if(m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;
		m_Button15.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftRing)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
		m_Button14.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
		m_Button13.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		m_Button12.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
		m_Button11.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
		m_Button10.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
		m_Button9.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
		m_Button8.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
		m_Button7.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
		m_Button6.EnableWindow(FALSE);
	}

	if(m_pParent->m_OptionDlg.m_chkRightThumb && m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
		m_Button5.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkLeftRing && m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;
		m_Button4.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex && m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
		m_Button3.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing && m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
		m_Button2.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex && m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
		m_Button1.EnableWindow(FALSE);
	}

}

void CScanFingerDlg::Disable5Finger()
{
	if(m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkLeftRing)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
	}
	
	if(m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
	}

	if(m_pParent->m_OptionDlg.m_chkRightThumb && m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
		m_Button5.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkLeftRing && m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;
		m_Button4.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex && m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
		m_Button3.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing && m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
		m_Button2.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex && m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
		m_Button1.EnableWindow(FALSE);
	}

}
void CScanFingerDlg::Disable6Finger()
{
	if(m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkLeftRing)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
		m_Button6.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
	}
	if(m_pParent->m_OptionDlg.m_chkRightThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
		m_Button5.EnableWindow(FALSE);
	
	}	
	if(m_pParent->m_OptionDlg.m_chkLeftRing && m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;
		m_Button4.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex && m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
		m_Button3.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing && m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
		m_Button2.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex && m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
		m_Button1.EnableWindow(FALSE);
	}
}

void CScanFingerDlg::Disable16Finger()
{
	if(m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;
		m_Button16.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftRing)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
		m_Button15.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
		m_Button14.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		m_Button13.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
		m_Button12.EnableWindow(FALSE);
		m_Button6.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
		m_Button11.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
		m_Button10.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
		m_Button9.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
		m_Button8.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
		m_Button5.EnableWindow(FALSE);
		m_Button7.EnableWindow(FALSE);
	}	
	if(m_pParent->m_OptionDlg.m_chkLeftRing && m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;
		m_Button4.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex && m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
		m_Button3.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing && m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
		m_Button2.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex && m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
		m_Button1.EnableWindow(FALSE);
	}
}

void CScanFingerDlg::Disable17Finger()
{
	if(m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;
		m_Button17.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftRing)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
		m_Button16.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
		m_Button15.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		m_Button14.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] = FALSE;
		m_Button13.EnableWindow(FALSE);
		m_Button7.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
		m_Button12.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
		m_Button11.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
		m_Button10.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
		m_Button9.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightThumb)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] = FALSE;
		m_Button6.EnableWindow(FALSE);
		m_Button8.EnableWindow(FALSE);
	}	
	if(m_pParent->m_OptionDlg.m_chkLeftRing && m_pParent->m_OptionDlg.m_chkLeftLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_RING] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] = FALSE;
		m_Button4.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkLeftIndex && m_pParent->m_OptionDlg.m_chkLeftMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] = FALSE;
		m_Button3.EnableWindow(FALSE);
	}
	
	if(m_pParent->m_OptionDlg.m_chkRightRing && m_pParent->m_OptionDlg.m_chkRightLittle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] = FALSE;
		m_Button2.EnableWindow(FALSE);
	}
	if(m_pParent->m_OptionDlg.m_chkRightIndex && m_pParent->m_OptionDlg.m_chkRightMiddle)
	{
		m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] = FALSE;
		m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] = FALSE;
		m_Button1.EnableWindow(FALSE);
	}
}

void CScanFingerDlg::Btn17Layout_Each_Both_Roll()
{
	int i;

	CColorButton *btnarr[17] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	CString btnstr[17] = {"Right Index-Middle", "Right Ring-Little", "Left Index-Middle","Left Ring-Little","Both Thumbs","Right Thumb","Left Thumb",
		"Right Thumb roll", "Right Index roll","Right Middle roll","Right Ring roll","Right Little roll",
		"Left Thumb roll", "Left Index roll","Left Middle roll","Left Ring roll","Left Little roll"};
	for(i=0; i<17; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
}

void CScanFingerDlg::Btn16Layout_Each_Roll()
{
	int i;

	CColorButton *btnarr[17] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	CString btnstr[17] = {"Right Index-Middle", "Right Ring-Little", "Left Index-Middle","Left Ring-Little","Right Thumb","Left Thumb",
		"Right Thumb roll", "Right Index roll","Right Middle roll","Right Ring roll","Right Little roll",
		"Left Thumb roll", "Left Index roll","Left Middle roll","Left Ring roll","Left Little roll",""};
	for(i=0; i<16; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
	btnarr[16]->ShowWindow(SW_HIDE);
}
void CScanFingerDlg::Btn15Layout_Flat()
{

	int i;

	CColorButton *btnarr[17] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	CString btnstr[17] = {"Right Index-Middle", "Right Ring-Little", "Left Index-Middle","Left Ring-Little","Both Thumbs",
		"Right Thumb", "Right Index","Right Middle","Right Ring","Right Little",
		"Left Thumb", "Left Index","Left Middle","Left Ring","Left Little","",""};
	for(i=0; i<15; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
	btnarr[15]->ShowWindow(SW_HIDE);
	btnarr[16]->ShowWindow(SW_HIDE);
}

void CScanFingerDlg::Btn15Layout_Roll()
{
	int i;

	CColorButton *btnarr[17] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	CString btnstr[17] = {"Right Index-Middle", "Right Ring-Little", "Left Index-Middle","Left Ring-Little","Both Thumbs",
		"Right Thumb Roll", "Right Index Roll","Right Middle Roll","Right Ring Roll","Right Little Roll",
		"Left Thumb Roll", "Left Index Roll","Left Middle Roll","Left Ring Roll","Left Little Roll","",""};
	for(i=0; i<15; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
	btnarr[15]->ShowWindow(SW_HIDE);
	btnarr[16]->ShowWindow(SW_HIDE);

}

void CScanFingerDlg::Btn13Layout()
{
	int i;

	CColorButton *btnarr[17] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	CString btnstr[17] = {"Right Four","Left Four","Both Thumbs",
		"Right Thumb roll", "Right Index roll","Right Middle roll","Right Ring roll","Right Little roll",
		"Left Thumb roll", "Left Index roll","Left Middle roll","Left Ring roll","Left Little roll","14","15","16","17"};
	for(i=0; i<13; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
	btnarr[13]->ShowWindow(SW_HIDE);
	btnarr[14]->ShowWindow(SW_HIDE);
	btnarr[15]->ShowWindow(SW_HIDE);
	btnarr[16]->ShowWindow(SW_HIDE);
}

void CScanFingerDlg::Btn14Layout()
{
	int i;

	CColorButton *btnarr[17] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	CString btnstr[17] = {"Right Four", "Right Thumb", "Left Four", "Left Thumb",
		"Right Thumb roll", "Right Index roll","Right Middle roll","Right Ring roll","Right Little roll",
		"Left Thumb roll", "Left Index roll","Left Middle roll","Left Ring roll","Left Little roll","15","16","17"};
	for(i=0; i<14; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
	btnarr[14]->ShowWindow(SW_HIDE);
	btnarr[15]->ShowWindow(SW_HIDE);
	btnarr[16]->ShowWindow(SW_HIDE);
}

void CScanFingerDlg::BtnSingleLayout()
{
	int i;

	CColorButton *btnarr[17] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	CString btnstr[17] = {"Right Thumb", "Right Index","Right Middle","Right Ring","Right Little",
		"Left Thumb", "Left Index","Left Middle","Left Ring","Left Little","11","12","13","14","15","16","17"};
	for(i=0; i<10; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
	for(i=10; i<17; i++)
	{
		btnarr[i]->ShowWindow(SW_HIDE);
	}
}

void CScanFingerDlg::BtnSingleRollLayout()
{
	int i;

	CColorButton *btnarr[17] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	CString btnstr[17] = {"Right Thumb roll", "Right Index roll","Right Middle roll","Right Ring roll","Right Little roll",
		"Left Thumb roll", "Left Index roll","Left Middle roll","Left Ring roll","Left Little roll","11","12","13","14","15","16","17"};
	for(i=0; i<10; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
	for(i=10; i<17; i++)
	{
		btnarr[i]->ShowWindow(SW_HIDE);
	}
}

void CScanFingerDlg::BtnTwoFingerLayout()
{
	int i;

	CColorButton *btnarr[17] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	CString btnstr[17] = {"Right Index-Middle", "Right Ring-Little", "Left Index-Middle","Left Ring-Little", "Both Thumbs",
		"6", "7","8","9","10","11","12","13","14","15","16","17"};
	for(i=0; i<5; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
	for(i=5; i<17; i++)
	{
		btnarr[i]->ShowWindow(SW_HIDE);
		
	}
}
void CScanFingerDlg::BtnTwoFingerLayout_Each()
{
	int i;

	CColorButton *btnarr[17] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	CString btnstr[17] = {"Right Index-Middle", "Right Ring-Little", "Left Index-Middle","Left Ring-Little", "Right Thumb",
		"Left Thumb", "7","8","9","10","11","12","13","14","15","16","17"};
	for(i=0; i<6; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
	for(i=6; i<17; i++)
	{
		btnarr[i]->ShowWindow(SW_HIDE);
		
	}
}
void CScanFingerDlg::Btn1Layout()
{
	int i;

	CColorButton *btnarr[17] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	CString btnstr[17] = {"Both Thumbs", "2", "3","4", "5",
		"6", "7","8","9","10","11","12","13","14","15","16","17"};
	for(i=0; i<1; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
	for(i=1; i<17; i++)
	{
		btnarr[i]->ShowWindow(SW_HIDE);
		
	}

}
void CScanFingerDlg::Btn2Layout()
{
	int i;

	CColorButton *btnarr[17] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	CString btnstr[17] = {"Right Thumb", "Left Thumb", "3","4", "5",
		"6", "7","8","9","10","11","12","13","14","15","16","17"};
	for(i=0; i<2; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
	for(i=2; i<17; i++)
	{
		btnarr[i]->ShowWindow(SW_HIDE);
		
	}
}
void CScanFingerDlg::Btn3Layout()
{
	int i;

	CColorButton *btnarr[17] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	CString btnstr[17] = {"Right Four", "Left Four", "Both thumbs","4", "5",
		"6", "7","8","9","10","11","12","13","14","15","16","17"};
	for(i=0; i<3; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
	for(i=3; i<17; i++)
	{
		btnarr[i]->ShowWindow(SW_HIDE);
		
	}
}
void CScanFingerDlg::Btn4Layout()
{
	int i;

	CColorButton *btnarr[17] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	CString btnstr[17] = {"Right Thumb", "Right Index", "Left Thumb","Left Index", "5",
		"6", "7","8","9","10","11","12","13","14","15","16","17"};
	for(i=0; i<4; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
	for(i=4; i<17; i++)
	{
		btnarr[i]->ShowWindow(SW_HIDE);
		
	}

}
void CScanFingerDlg::Btn5Layout()
{
	int i;

	CColorButton *btnarr[18] = {&m_Button1, &m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6,&m_Button7, &m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17, &m_Button18};
	CString btnstr[18] = {"Right Four", "Right Thumb", "Left Four", "Left Thumb","5",
		"6", "7","8","9","10","11","12","13","14","15","16","17", "18"};
	for(i=0; i<4; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(255,255,255),RGB(70,70,70));
	}
	for(i=4; i<18; i++)
	{
		btnarr[i]->ShowWindow(SW_HIDE);
		
	}
}

void CScanFingerDlg::OnBnClickedScan10Fingers()
{
	if( m_StartFingerCapture == TRUE )
	{
		if( m_pParent->m_OptionDlg.m_chkAutoCapture == FALSE )
		{
			BOOL IsActive;
			int nRc;
			nRc = IBSU_IsCaptureActive(m_pParent->m_nDevHandle, &IsActive);
			if( nRc == IBSU_STATUS_OK && IsActive )
			{
				IBSU_TakeResultImageManually(m_pParent->m_nDevHandle);
			}
		}
		return;
	}

	m_CaptureStart = TRUE;
	m_CaptureEnd = FALSE;

	if( m_pParent->m_bInitializing )
		return;

	int devIndex = m_pParent->m_MainDlg.m_DeviceList.GetCurSel() - 1;
	if( devIndex < 0 )
		return;

	int seqIndex = m_pParent->m_MainDlg.m_SequenceList.GetCurSel() - 1;
	if( seqIndex < 0 )
		return;

	//if( m_pParent->m_nCurrentCaptureStep != -1 )
	//{
	//	BOOL IsActive;
	//	int nRc;
	//	nRc = IBSU_IsCaptureActive(m_pParent->m_nDevHandle, &IsActive);
	//	if( nRc == IBSU_STATUS_OK && IsActive )
	//	{
	//		IBSU_TakeResultImageManually(m_pParent->m_nDevHandle);
	//	}

	//	return;
	//}

	////////////////////////////////////////
	// select capture mode
	m_pParent->m_MainDlg.m_CaptureFinger = NONE_FINGER;

	if( m_pParent->m_nDevHandle == -1 )
	{
/*		m_pParent->m_bInitializing = true;
		VERIFY( ::AfxBeginThread( m_pParent->m_MainDlg._InitializeDeviceThreadCallback, m_pParent, THREAD_PRIORITY_NORMAL,
			0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
*/
        m_pParent->m_bPreInitialization = FALSE;
            m_pParent->m_MainDlg.SetTimer(0, 500, NULL);
	}
	else
	{
		// device already initialized
		m_pParent->PostMessage(WM_USER_CAPTURE_SEQ_START);
	}
	m_StartFingerCapture=TRUE;
	m_pParent->OnMsg_UpdateDisplayResources();
}

void CScanFingerDlg::OnBnClickedClose()
{
	m_pParent->m_MainDlg.m_PressedBtn = FALSE;

	if(m_CaptureEnd == FALSE)
	{
		if(MessageBox("Capture is not completed,Do you want close?","CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
		{		
			m_StartFingerCapture=FALSE;
			m_CaptureStart =FALSE;
			m_pParent->_CaptureStop();
	
			m_pParent->ChangeView(0);
			m_pParent->m_MainDlg.m_btnEdit.EnableWindow(FALSE);
			m_pParent->m_MainDlg.m_btnSave.EnableWindow(FALSE);

			m_pParent->m_SmearFlag = FALSE;
			m_pParent->m_WrongPlaceFlag = FALSE;
			m_pParent->m_WetFingerFlag = FALSE;
			memset(m_pParent->m_ShiftedFlag, 0, sizeof(m_pParent->m_ShiftedFlag));
			m_pParent->_SetLEDs(m_pParent->m_nDevHandle, ENUM_IBSU_TYPE_NONE, NONE_FINGER, 0, FALSE, m_pParent->m_ShiftedFlag);
		}
		return;
	}
	else
	{
		m_StartFingerCapture=FALSE;
		m_CaptureStart =FALSE;
		m_pParent->_CaptureStop();
		m_pParent->ChangeView(0);
		m_pParent->m_MainDlg.m_btnEdit.EnableWindow(TRUE);
		m_pParent->m_MainDlg.m_btnSave.EnableWindow(TRUE);
	}
}

void CScanFingerDlg::OnTimer(UINT_PTR nIDEvent)
{
	UpdateFingerView();

	CDialog::OnTimer(nIDEvent);
}

void CScanFingerDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(m_SelectFinger ==TRUE)
		return;

	WINDOWPLACEMENT place;
	m_FingerView.GetWindowPlacement(&place);

	CRect rect(place.rcNormalPosition);

	if(rect.PtInRect(point) == TRUE)
	{
		point.x -= place.rcNormalPosition.left;
		point.y -= place.rcNormalPosition.top;

		m_FingerDisplayManager->SelectFinger(point);
	}

	CDialog::OnLButtonDown(nFlags, point);
}

void CScanFingerDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	if(m_CaptureStart == TRUE)
		return;

	WINDOWPLACEMENT place;
	m_FingerView.GetWindowPlacement(&place);

	CRect rect(place.rcNormalPosition);

	if(rect.PtInRect(point) == TRUE)
	{
		point.x -= place.rcNormalPosition.left;
		point.y -= place.rcNormalPosition.top;

		m_FingerDisplayManager->ToggleEnableFinger(point);
		
		if(m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] == FALSE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button10.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button15.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button16.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button17.EnableWindow(FALSE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[LEFT_RING] == FALSE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button9.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button14.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button15.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button16.EnableWindow(FALSE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] == FALSE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button8.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button13.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button14.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button15.EnableWindow(FALSE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] == FALSE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button7.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button12.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button13.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button14.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
			{
				m_Button4.EnableWindow(FALSE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] == FALSE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button6.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button11.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button6.EnableWindow(FALSE);
				m_Button12.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button13.EnableWindow(FALSE);
				m_Button7.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
			{
				m_Button3.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB)
			{
				m_Button2.EnableWindow(FALSE);
			}
		}
		//RIGHT
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] == FALSE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button5.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button10.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button11.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button12.EnableWindow(FALSE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] == FALSE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button4.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button9.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button10.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button11.EnableWindow(FALSE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] == FALSE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button3.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button8.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button9.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button10.EnableWindow(FALSE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] == FALSE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button2.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button7.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button8.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button9.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
			{
				m_Button2.EnableWindow(FALSE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] == FALSE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button1.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button6.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button5.EnableWindow(FALSE);
				m_Button7.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button6.EnableWindow(FALSE);
				m_Button8.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
			{
				m_Button1.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB)
			{
				m_Button1.EnableWindow(FALSE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] == FALSE &&
			m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] == FALSE	)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH)
			{
				m_Button5.EnableWindow(FALSE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TWO_FINGER_FLAT_CAPTURE)
			{
				m_Button1.EnableWindow(FALSE);
			}
			
		}
		if(m_FingerDisplayManager->m_FingerEnable[LEFT_RING] == FALSE &&
			m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] == FALSE	)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH)
			{
				m_Button4.EnableWindow(FALSE);
			}
			
		}
		if(m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] == FALSE &&
			m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] == FALSE	)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH)
			{
				m_Button3.EnableWindow(FALSE);
			}
			
		}
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] == FALSE &&
			m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] == FALSE	)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH)
			{
				m_Button2.EnableWindow(FALSE);
			}
			
		}
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] == FALSE &&
			m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] == FALSE	)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH)
			{
				m_Button1.EnableWindow(FALSE);
			}
			
		}


	
////////////////////////////////////////////////////////////////////////////
		if(m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] == TRUE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button10.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button15.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button16.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button17.EnableWindow(TRUE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[LEFT_RING] == TRUE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button9.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button14.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button15.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button16.EnableWindow(TRUE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] == TRUE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button8.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button13.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button14.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button15.EnableWindow(TRUE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] == TRUE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button7.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button12.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button13.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button14.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
			{
				m_Button4.EnableWindow(TRUE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] == TRUE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button6.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button11.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button6.EnableWindow(TRUE);
				m_Button12.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button13.EnableWindow(TRUE);
				m_Button7.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
			{
				m_Button3.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB)
			{
				m_Button2.EnableWindow(TRUE);
			}
		}
		//RIGHT
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] == TRUE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button5.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button10.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button11.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button12.EnableWindow(TRUE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] == TRUE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button4.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button9.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button10.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button11.EnableWindow(TRUE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] == TRUE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button3.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button8.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button9.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button10.EnableWindow(TRUE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] == TRUE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button2.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button7.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button8.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button9.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
			{
				m_Button2.EnableWindow(TRUE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] == TRUE)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
			{
				m_Button1.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
			{
				m_Button6.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
			{
				m_Button5.EnableWindow(TRUE);
				m_Button7.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
			{
				m_Button6.EnableWindow(TRUE);
				m_Button8.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
			{
				m_Button1.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB)
			{
				m_Button1.EnableWindow(TRUE);
			}
		}
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_THUMB] == TRUE ||
			m_FingerDisplayManager->m_FingerEnable[LEFT_THUMB] == TRUE	)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH)
			{
				m_Button5.EnableWindow(TRUE);
			}
			if(m_pParent->m_MainDlg.m_CaptureMode == TWO_FINGER_FLAT_CAPTURE)
			{
				m_Button1.EnableWindow(TRUE);
			}
			
		}
		if(m_FingerDisplayManager->m_FingerEnable[LEFT_RING] == TRUE ||
			m_FingerDisplayManager->m_FingerEnable[LEFT_LITTLE] == TRUE	)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH)
			{
				m_Button4.EnableWindow(TRUE);
			}
			
		}
		if(m_FingerDisplayManager->m_FingerEnable[LEFT_MIDDLE] == TRUE ||
			m_FingerDisplayManager->m_FingerEnable[LEFT_INDEX] == TRUE	)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH)
			{
				m_Button3.EnableWindow(TRUE);
			}
			
		}
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_RING] == TRUE ||
			m_FingerDisplayManager->m_FingerEnable[RIGHT_LITTLE] == TRUE	)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH)
			{
				m_Button2.EnableWindow(TRUE);
			}
			
		}
		if(m_FingerDisplayManager->m_FingerEnable[RIGHT_INDEX] == TRUE ||
			m_FingerDisplayManager->m_FingerEnable[RIGHT_MIDDLE] == TRUE	)
		{
			if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN ||
				m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH)
			{
				m_Button1.EnableWindow(TRUE);
			}
			
		}


/////////////////////////////////////////////////////////////////////////////
	}

	

	CDialog::OnRButtonDown(nFlags, point);
}

HBRUSH CScanFingerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	//CWnd *wnd;
	hbr =(HBRUSH)m_background;

//	KOJAK_LED KojakLed;

//	IBSU_GetKojakLED(m_pParent->m_nDevHandle, &KojakLed);
	
	//KojakLed.LeftIndex = 3 | 4 ;   1 : g   2 : r 

	


	// TODO:  Change any attributes of the DC here
	const int WndID = pWnd->GetDlgCtrlID();
	for( int i = 0; i < 4; i++ )
	{
		//IDC_STATIC_QUALITY_1+i
		
		if( WndID == IDC_STATIC_QUALITY_1+i )
		{
			if( m_pParent->m_bNeedClearPlaten )
			{
				if( m_pParent->m_bBlank )
					hbr = static_cast<HBRUSH>(s_brushRed);
				else
					hbr = static_cast<HBRUSH>(s_brushPlaten);
			}
			else
			{
				//wnd = GetDlgItem(WndID);

				switch( m_pParent->m_FingerQuality[i] )
				{ 
				case ENUM_IBSU_QUALITY_GOOD:
					hbr = static_cast<HBRUSH>(s_brushGreen);
					
					break;          
				case ENUM_IBSU_QUALITY_FAIR :
					hbr = static_cast<HBRUSH>(s_brushOrange);
					//wnd->SetWindowText("FAIR");
					break;          
				case ENUM_IBSU_QUALITY_POOR :
					hbr = static_cast<HBRUSH>(s_brushRed);
					//wnd->SetWindowText("POOR");
					break;          
				case ENUM_IBSU_FINGER_NOT_PRESENT:
					hbr = static_cast<HBRUSH>(s_brushPlaten);
					//wnd->SetWindowText("");
					break;
				case ENUM_IBSU_QUALITY_INVALID_AREA_TOP:
					hbr = static_cast<HBRUSH>(s_brushRed);
					//wnd->SetWindowText("");
					break;
				case ENUM_IBSU_QUALITY_INVALID_AREA_LEFT:
					hbr = static_cast<HBRUSH>(s_brushRed);
					//wnd->SetWindowText("");
					break;
				case ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT:
					hbr = static_cast<HBRUSH>(s_brushRed);
					//wnd->SetWindowText("");
					break;
				case ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM:
					hbr = static_cast<HBRUSH>(s_brushRed);
					//wnd->SetWindowText("");
					break;
				default:
					ASSERT( FALSE );
				}
			}
		}
	}

//	IBSU_SetKojakLED(m_pParent->m_nDevHandle, KojakLed);

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

void CScanFingerDlg::OnBnClickedButton1()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_THUMB);
			info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
			info.fingerName = _T("SFF_Right_Thumb");
			info.capture_finger_idx = RIGHT_THUMB;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TWO_FINGER_FLAT_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_BOTH_THUMBS);
			info.PreCaptureMessage = _T("Please put both thumbs on the sensor!");
			info.fingerName = _T("TFF_Both_Thumbs");
			info.capture_finger_idx = BOTH_THUMBS;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_THUMB);
			info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
			info.fingerName = _T("SRF_Right_Thumb");
			info.capture_finger_idx = RIGHT_THUMB;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX_MIDDLE);
			info.PreCaptureMessage = _T("Please put right index and middle on the sensor!");
			info.fingerName = _T("TFF_Right_Index_Middle");
			info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right index on the sensor!");
			info.fingerName = _T("SFF_Right_Index");
			info.capture_finger_idx = RIGHT_INDEX;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}else if(m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_MIDDLE);
			info.PreCaptureMessage = _T("Please put right middle on the sensor!");
			info.fingerName = _T("SFF_Right_Index_Middle");
			info.capture_finger_idx = RIGHT_MIDDLE;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	///
	if( m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO || 
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right four fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Four_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
			info.NumberOfFinger = 4;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		//RIGHT_THREE
		else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
			info.NumberOfFinger = 3;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//RIGHT_THREE
		else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
			info.NumberOfFinger = 3;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//RIGHT_THREE
		else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
			info.NumberOfFinger = 3;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//RIGHT_THREE
		else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
			info.NumberOfFinger = 3;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//RIGHT_TWO
		else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//RIGHT_TWO
		else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//RIGHT_TWO
		else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//RIGHT_TWO
		else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//RIGHT_TWO
		else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//RIGHT_TWO
		else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		//RIGHT_SINGLE
		else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		//RIGHT_SINGLE
		else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		//RIGHT_SINGLE
		else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		//RIGHT_SINGLE
		else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_HAND);
			info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
			info.fingerName = _T("TFF_Right_Fingers");
			info.capture_finger_idx = RIGHT_HAND;
			info.capture_finger_btn =1;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;

	}
	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);

	CColorButton *btnarr[16] = {&m_Button2,&m_Button3, &m_Button4, &m_Button5, &m_Button6, &m_Button7, &m_Button8, 
		&m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}

	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );	
}

void CScanFingerDlg::OnBnClickedButton2()
{
	if(m_CaptureEnd == FALSE )
		return;

	//memset(m_pParent->m_MainDlg.m_TemplateDB , 0, sizeof(m_pParent->m_MainDlg.m_TemplateDB));
	//memset(m_pParent->m_MainDlg.m_TemplateCheck, 0, sizeof(m_pParent->m_MainDlg.m_TemplateCheck));
	//memset(m_pParent->m_MainDlg.m_TemplateThumb, 0, sizeof(m_pParent->m_MainDlg.m_TemplateThumb));
	//memset(m_pParent->m_MainDlg.m_TemplateFour, 0, sizeof(m_pParent->m_MainDlg.m_TemplateFour));

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right index on the sensor!");
			info.fingerName = _T("SFF_Right_Index");
			info.capture_finger_idx = RIGHT_INDEX;
			info.capture_finger_btn =2;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_THUMB);
			info.PreCaptureMessage = _T("Please put Left thumb on the sensor!");
			info.fingerName = _T("TFF_Left_Thumbs");
			info.capture_finger_idx = LEFT_THUMB;
			info.capture_finger_btn =2;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right index on the sensor!");
			info.fingerName = _T("SRF_Right_Index");
			info.capture_finger_idx = RIGHT_INDEX;
			info.capture_finger_btn =2;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{

		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right ring and little on the sensor!");
			info.fingerName = _T("TFF_Right_Ring_Little");
			info.capture_finger_idx = RIGHT_RING_LITTLE;
			info.capture_finger_btn =2;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_RING);
			info.PreCaptureMessage = _T("Please put right ring on the sensor!");
			info.fingerName = _T("TFF_Right_Ring");
			info.capture_finger_idx = RIGHT_RING;
			info.capture_finger_btn =2;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_LITTLE);
			info.PreCaptureMessage = _T("Please put right little on the sensor!");
			info.fingerName = _T("TFF_Right_Little");
			info.capture_finger_idx = RIGHT_LITTLE;
			info.capture_finger_btn =2;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO || 
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH)
	{
		// LEFT_FOUR
			if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left four fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
				info.NumberOfFinger = 4;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}
			
			//LEFT_THREE
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
				info.NumberOfFinger = 3;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}
			
			//LEFT_TWO
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}

			
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
			info.fingerName = _T("SFF_Right_Thumb");
			info.capture_finger_idx = RIGHT_THUMB;
			info.capture_finger_btn =2;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button3, &m_Button4, &m_Button5, &m_Button6, &m_Button7, &m_Button8, 
		&m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}

	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton3()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE)
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_MIDDLE);
			info.PreCaptureMessage = _T("Please put right middle on the sensor!");
			info.fingerName = _T("SFF_Right_Middle");
			info.capture_finger_idx = RIGHT_MIDDLE;
			info.capture_finger_btn =3;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_THUMB);
			info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
			info.fingerName = _T("SFF_Left_Thumb");
			info.capture_finger_idx = LEFT_THUMB;
			info.capture_finger_btn =3;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_MIDDLE);
			info.PreCaptureMessage = _T("Please put right middle on the sensor!");
			info.fingerName = _T("SRF_Right_Middle");
			info.capture_finger_idx = RIGHT_MIDDLE;
			info.capture_finger_btn =3;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_MIDDLE_INDEX);
			info.PreCaptureMessage = _T("Please put left index and middle on the sensor!");
			info.fingerName = _T("TFF_Left_Index_Middle");
			info.capture_finger_idx = LEFT_INDEX_MIDDLE;
			info.capture_finger_btn =3;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_INDEX);
			info.PreCaptureMessage = _T("Please put left index on the sensor!");
			info.fingerName = _T("TFF_Left_Index");
			info.capture_finger_idx = LEFT_INDEX;
			info.capture_finger_btn =3;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}else if(m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_MIDDLE);
			info.PreCaptureMessage = _T("Please put left middle on the sensor!");
			info.fingerName = _T("TFF_Left_Middle");
			info.capture_finger_idx = LEFT_MIDDLE;
			info.capture_finger_btn =3;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if( m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO || 
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH)
	{
		// BOTH_THUMBS
			if( m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE &&
				m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_BOTH_THUMBS);
				info.PreCaptureMessage = _T("Please put both thumbs on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}
			else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_BOTH_THUMBS);
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}
			else if( m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_BOTH_THUMBS);
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_pParent->m_vecCaptureSeq.push_back(info);
			}
			m_CaptureEnd = FALSE;
	}
	
	if( m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		// LEFT_FOUR
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left four fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
			info.NumberOfFinger = 4;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		
		//LEFT_THREE
		else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
			info.NumberOfFinger = 3;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//LEFT_THREE
		else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
			info.NumberOfFinger = 3;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//LEFT_THREE
		else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
			info.NumberOfFinger = 3;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//LEFT_THREE
		else if( m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
			info.NumberOfFinger = 3;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		
		//LEFT_TWO
		else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//LEFT_TWO
		else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//LEFT_TWO
		else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//LEFT_TWO
		else if( m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//LEFT_TWO
		else if( m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}

		//LEFT_TWO
		else if( m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		//LEFT_SINGLE
		else if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		//LEFT_SINGLE
		else if( m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		//LEFT_SINGLE
		else if( m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		//LEFT_SINGLE
		else if( m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_HAND);
			info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
			info.fingerName = _T("TFF_Left_Fingers");
			info.capture_finger_idx = LEFT_HAND;
			info.capture_finger_btn = 3;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button4, &m_Button5, &m_Button6, &m_Button7, &m_Button8, 
		&m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton4()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE)
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_RING);
			info.PreCaptureMessage = _T("Please put right ring on the sensor!");
			info.fingerName = _T("SFF_Right_Ring");
			info.capture_finger_idx = RIGHT_RING;
			info.capture_finger_btn =4;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_INDEX);
			info.PreCaptureMessage = _T("Please put left index on the sensor!");
			info.fingerName = _T("SFF_Left_Index");
			info.capture_finger_idx = LEFT_INDEX;
			info.capture_finger_btn =4;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_RING);
			info.PreCaptureMessage = _T("Please put right ring on the sensor!");
			info.fingerName = _T("SRF_Right_Ring");
			info.capture_finger_idx = RIGHT_RING;
			info.capture_finger_btn =4;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH )
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_THUMB);
			info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
			info.fingerName = _T("SRF_Right_Thumb");
			info.capture_finger_idx = RIGHT_THUMB;
			info.capture_finger_btn =4;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_LITTLE_RING);
			info.PreCaptureMessage = _T("Please put left ring and little on the sensor!");
			info.fingerName = _T("TFF_Left_Ring_Little");
			info.capture_finger_idx = LEFT_RING_LITTLE;
			info.capture_finger_btn =4;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}else if( m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_RING);
			info.PreCaptureMessage = _T("Please put left ring on the sensor!");
			info.fingerName = _T("TFF_Left_Ring");
			info.capture_finger_idx = LEFT_RING;
			info.capture_finger_btn =4;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}else if( m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_LITTLE);
			info.PreCaptureMessage = _T("Please put left little on the sensor!");
			info.fingerName = _T("TFF_Left_Little");
			info.capture_finger_idx = LEFT_LITTLE;
			info.capture_finger_btn =4;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if( m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
			info.fingerName = _T("SFF_Left_Thumb");
			info.capture_finger_idx = LEFT_THUMB;
			info.capture_finger_btn = 4;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button3, &m_Button5, &m_Button6, &m_Button7, &m_Button8, 
		&m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton5()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE)
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_LITTLE);
			info.PreCaptureMessage = _T("Please put right little on the sensor!");
			info.fingerName = _T("SFF_Right_Little");
			info.capture_finger_idx = RIGHT_LITTLE;
			info.capture_finger_btn =5;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_LITTLE);
			info.PreCaptureMessage = _T("Please put right little on the sensor!");
			info.fingerName = _T("SRF_Right_Little");
			info.capture_finger_idx = RIGHT_LITTLE;
			info.capture_finger_btn =5;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(	m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_THUMB);
			info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
			info.fingerName = _T("TFF_Right_Thumb");
			info.capture_finger_idx = RIGHT_THUMB;
			info.capture_finger_btn =5;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(	m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_THUMB);
			info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
			info.fingerName = _T("TFF_Right_Thumb");
			info.capture_finger_idx = BOTH_RIGHT_THUMB;
			info.capture_finger_btn =5;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH )
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right index on the sensor!");
			info.fingerName = _T("SRF_Right_Index");
			info.capture_finger_idx = RIGHT_INDEX;
			info.capture_finger_btn =5;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE &&
			m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_BOTH_THUMBS);
			info.PreCaptureMessage = _T("Please put both thumbs on the sensor!");
			info.fingerName = _T("TFF_Both_Thumbs");
			info.capture_finger_idx = BOTH_THUMBS;
			info.capture_finger_btn =5;
			info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}else if( m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_THUMB);
			info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
			info.fingerName = _T("SFF_Right_Thumb");
			info.capture_finger_idx = RIGHT_THUMB;
			info.capture_finger_btn =5;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}else if( m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_THUMB);
			info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
			info.fingerName = _T("SFF_Left_Thumb");
			info.capture_finger_idx = LEFT_THUMB;
			info.capture_finger_btn =5;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if( m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
			info.fingerName = _T("SFF_Right_Thumb");
			info.capture_finger_idx = RIGHT_THUMB;
			info.capture_finger_btn = 5;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button3, &m_Button4, &m_Button6, &m_Button7, &m_Button8, 
		&m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton6()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE)
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_THUMB);
			info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
			info.fingerName = _T("SFF_Left_Thumb");
			info.capture_finger_idx = LEFT_THUMB;
			info.capture_finger_btn =6;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_THUMB);
			info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
			info.fingerName = _T("SRF_Left_Thumb");
			info.capture_finger_idx = LEFT_THUMB;
			info.capture_finger_btn =6;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(	m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_THUMB);
			info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
			info.fingerName = _T("TFF_Left_Thumb");
			info.capture_finger_idx = LEFT_THUMB;
			info.capture_finger_btn =6;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(	m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_THUMB);
			info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
			info.fingerName = _T("TFF_Left_Thumb");
			info.capture_finger_idx = BOTH_LEFT_THUMB;
			info.capture_finger_btn =6;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 2;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH )
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_MIDDLE);
			info.PreCaptureMessage = _T("Please put right middle on the sensor!");
			info.fingerName = _T("SRF_Right_Middle");
			info.capture_finger_idx = RIGHT_MIDDLE;
			info.capture_finger_btn =6;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_THUMB);
			info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
			info.fingerName = _T("TFF_Right_Thumbs");
			info.capture_finger_idx = RIGHT_THUMB;
			info.capture_finger_btn =6;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_THUMB);
			info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
			info.fingerName = _T("TFF_Right_Thumb");
			info.capture_finger_idx = RIGHT_THUMB;
			info.capture_finger_btn =6;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}	
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_THUMB);
			info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
			info.fingerName = _T("TFF_Right_Thumb");
			info.capture_finger_idx = RIGHT_THUMB;
			info.capture_finger_btn =6;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if( m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right index on the sensor!");
			info.fingerName = _T("SFF_Right_Index");
			info.capture_finger_idx = RIGHT_INDEX;
			info.capture_finger_btn = 6;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button7, &m_Button8, 
		&m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton7()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE)
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_INDEX);
			info.PreCaptureMessage = _T("Please put left index on the sensor!");
			info.fingerName = _T("SFF_Left_Index");
			info.capture_finger_idx = LEFT_INDEX;
			info.capture_finger_btn =7;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_INDEX);
			info.PreCaptureMessage = _T("Please put left index on the sensor!");
			info.fingerName = _T("SRF_Left_Index");
			info.capture_finger_idx = LEFT_INDEX;
			info.capture_finger_btn =7;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(	m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_THUMB);
			info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
			info.fingerName = _T("SRF_Right_Thumb");
			info.capture_finger_idx = RIGHT_THUMB;
			info.capture_finger_btn =7;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_THUMB);
			info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
			info.fingerName = _T("SFF_LEFT_Thumb");
			info.capture_finger_idx = LEFT_THUMB;
			info.capture_finger_btn =7;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH )
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_RING);
			info.PreCaptureMessage = _T("Please put right ring on the sensor!");
			info.fingerName = _T("SRF_Right_Ring");
			info.capture_finger_idx = RIGHT_RING;
			info.capture_finger_btn =7;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right index on the sensor!");
			info.fingerName = _T("TFF_Right_Index");
			info.capture_finger_idx = RIGHT_INDEX;
			info.capture_finger_btn =7;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right index on the sensor!");
			info.fingerName = _T("TFF_Right_Index");
			info.capture_finger_idx = RIGHT_INDEX;
			info.capture_finger_btn =7;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if( m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right middle on the sensor!");
			info.fingerName = _T("SFF_Right_Middle");
			info.capture_finger_idx = RIGHT_MIDDLE;
			info.capture_finger_btn = 7;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6, &m_Button8, 
		&m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton8()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE)
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_MIDDLE);
			info.PreCaptureMessage = _T("Please put left middle on the sensor!");
			info.fingerName = _T("SFF_Left_Middle");
			info.capture_finger_idx = LEFT_MIDDLE;
			info.capture_finger_btn =8;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_MIDDLE);
			info.PreCaptureMessage = _T("Please put left middle on the sensor!");
			info.fingerName = _T("SRF_Left_Middle");
			info.capture_finger_idx = LEFT_MIDDLE;
			info.capture_finger_btn =8;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(	m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right index on the sensor!");
			info.fingerName = _T("SRF_Right_Index");
			info.capture_finger_idx = RIGHT_INDEX;
			info.capture_finger_btn =8;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_THUMB);
			info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
			info.fingerName = _T("SRF_Right_Thumb");
			info.capture_finger_idx = RIGHT_THUMB;
			info.capture_finger_btn =8;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH )
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_LITTLE);
			info.PreCaptureMessage = _T("Please put right little on the sensor!");
			info.fingerName = _T("SRF_Right_Little");
			info.capture_finger_idx = RIGHT_LITTLE;
			info.capture_finger_btn =8;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_MIDDLE);
			info.PreCaptureMessage = _T("Please put right middle on the sensor!");
			info.fingerName = _T("TFF_Right_Middle");
			info.capture_finger_idx = RIGHT_MIDDLE;
			info.capture_finger_btn =8;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_MIDDLE);
			info.PreCaptureMessage = _T("Please put right middle on the sensor!");
			info.fingerName = _T("TFF_Right_Middle");
			info.capture_finger_idx = RIGHT_MIDDLE;
			info.capture_finger_btn =8;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if( m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right ring on the sensor!");
			info.fingerName = _T("SFF_Right_Ring");
			info.capture_finger_idx = RIGHT_RING;
			info.capture_finger_btn = 8;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6, &m_Button7, 
		&m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton9()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE)
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_RING);
			info.PreCaptureMessage = _T("Please put left ring on the sensor!");
			info.fingerName = _T("SFF_Left_Ring");
			info.capture_finger_idx = LEFT_RING;
			info.capture_finger_btn =9;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_RING);
			info.PreCaptureMessage = _T("Please put left ring on the sensor!");
			info.fingerName = _T("SRF_Left_Ring");
			info.capture_finger_idx = LEFT_RING;
			info.capture_finger_btn =9;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(	m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_MIDDLE);
			info.PreCaptureMessage = _T("Please put right middle on the sensor!");
			info.fingerName = _T("SRF_Right_Middle");
			info.capture_finger_idx = RIGHT_MIDDLE;
			info.capture_finger_btn =9;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right index on the sensor!");
			info.fingerName = _T("SRF_Right_Index");
			info.capture_finger_idx = RIGHT_INDEX;
			info.capture_finger_btn =9;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_RING);
			info.PreCaptureMessage = _T("Please put right ring on the sensor!");
			info.fingerName = _T("TFF_Right_Ring");
			info.capture_finger_idx = RIGHT_RING;
			info.capture_finger_btn =9;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH )
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_THUMB);
			info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
			info.fingerName = _T("SRF_Left_Thumb");
			info.capture_finger_idx = LEFT_THUMB;
			info.capture_finger_btn =9;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_RING);
			info.PreCaptureMessage = _T("Please put right ring on the sensor!");
			info.fingerName = _T("TFF_Right_Ring");
			info.capture_finger_idx = RIGHT_RING;
			info.capture_finger_btn =9;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}	
		m_CaptureEnd = FALSE;
	}

	if( m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put right little on the sensor!");
			info.fingerName = _T("SFF_Right_Little");
			info.capture_finger_idx = RIGHT_LITTLE;
			info.capture_finger_btn = 9;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6, &m_Button7, 
		&m_Button8, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton91()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE)
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_LITTLE);
			info.PreCaptureMessage = _T("Please put left little on the sensor!");
			info.fingerName = _T("SFF_Left_Little");
			info.capture_finger_idx = LEFT_LITTLE;
			info.capture_finger_btn =10;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_LITTLE);
			info.PreCaptureMessage = _T("Please put left little on the sensor!");
			info.fingerName = _T("SRF_Left_Little");
			info.capture_finger_idx = LEFT_LITTLE;
			info.capture_finger_btn =10;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(	m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_RING);
			info.PreCaptureMessage = _T("Please put right ring on the sensor!");
			info.fingerName = _T("SRF_Right_Ring");
			info.capture_finger_idx = RIGHT_RING;
			info.capture_finger_btn =10;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH )
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_INDEX);
			info.PreCaptureMessage = _T("Please put left index on the sensor!");
			info.fingerName = _T("SRF_Left_Index");
			info.capture_finger_idx = LEFT_INDEX;
			info.capture_finger_btn =10;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_MIDDLE);
			info.PreCaptureMessage = _T("Please put right middle on the sensor!");
			info.fingerName = _T("SRF_Right_Middle");
			info.capture_finger_idx = RIGHT_MIDDLE;
			info.capture_finger_btn =10;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
	{
		if(m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_LITTLE);
			info.PreCaptureMessage = _T("Please put right little on the sensor!");
			info.fingerName = _T("TFF_Right_Little");
			info.capture_finger_idx = RIGHT_LITTLE;
			info.capture_finger_btn =10;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_LITTLE);
			info.PreCaptureMessage = _T("Please put right little on the sensor!");
			info.fingerName = _T("TFF_Right_Little");
			info.capture_finger_idx = RIGHT_LITTLE;
			info.capture_finger_btn =10;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if( m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
			info.fingerName = _T("SFF_Left_Thumb");
			info.capture_finger_idx = LEFT_THUMB;
			info.capture_finger_btn = 10;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6, &m_Button7, 
		&m_Button8, &m_Button9, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton92()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(	m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_LITTLE);
			info.PreCaptureMessage = _T("Please put right little on the sensor!");
			info.fingerName = _T("SRF_Right_Little");
			info.capture_finger_idx = RIGHT_LITTLE;
			info.capture_finger_btn =11;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_RING);
			info.PreCaptureMessage = _T("Please put right ring on the sensor!");
			info.fingerName = _T("SRF_Right_Ring");
			info.capture_finger_idx = RIGHT_RING;
			info.capture_finger_btn =11;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_THUMB);
			info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
			info.fingerName = _T("TFF_Left_Thumb");
			info.capture_finger_idx = LEFT_THUMB;
			info.capture_finger_btn =11;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH )
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_MIDDLE);
			info.PreCaptureMessage = _T("Please put left middle on the sensor!");
			info.fingerName = _T("SRF_Left_Middle");
			info.capture_finger_idx = LEFT_MIDDLE;
			info.capture_finger_btn =11;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_THUMB);
			info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
			info.fingerName = _T("SFF_Left_Thumb");
			info.capture_finger_idx = LEFT_THUMB;
			info.capture_finger_btn =11;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if( m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put left index on the sensor!");
			info.fingerName = _T("SFF_Left_Index");
			info.capture_finger_idx = LEFT_INDEX;
			info.capture_finger_btn = 11;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6, &m_Button7, 
		&m_Button8, &m_Button9, &m_Button10, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton93()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(	m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_THUMB);
			info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
			info.fingerName = _T("SRF_Left_Thumb");
			info.capture_finger_idx = LEFT_THUMB;
			info.capture_finger_btn =12;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		if( m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_LITTLE);
			info.PreCaptureMessage = _T("Please put right little on the sensor!");
			info.fingerName = _T("SRF_Right_Little");
			info.capture_finger_idx = RIGHT_LITTLE;
			info.capture_finger_btn =12;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_INDEX);
			info.PreCaptureMessage = _T("Please put left index on the sensor!");
			info.fingerName = _T("TFF_Left_Index");
			info.capture_finger_idx = LEFT_INDEX;
			info.capture_finger_btn =12;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH )
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_RING);
			info.PreCaptureMessage = _T("Please put left ring on the sensor!");
			info.fingerName = _T("SRF_Left_Ring");
			info.capture_finger_idx = LEFT_RING;
			info.capture_finger_btn =12;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_INDEX);
			info.PreCaptureMessage = _T("Please put left index on the sensor!");
			info.fingerName = _T("SFF_Left_Index");
			info.capture_finger_idx = LEFT_INDEX;
			info.capture_finger_btn =12;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if( m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put left middle on the sensor!");
			info.fingerName = _T("SFF_Left_Middle");
			info.capture_finger_idx = LEFT_MIDDLE;
			info.capture_finger_btn = 12;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6, &m_Button7, 
		&m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton94()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(	m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_INDEX);
			info.PreCaptureMessage = _T("Please put left index on the sensor!");
			info.fingerName = _T("SRF_Left_Index");
			info.capture_finger_idx = LEFT_INDEX;
			info.capture_finger_btn =13;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_THUMB);
			info.PreCaptureMessage = _T("Please put left thumb e on the sensor!");
			info.fingerName = _T("SRF_Left_Thumb");
			info.capture_finger_idx = LEFT_THUMB;
			info.capture_finger_btn =13;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_MIDDLE);
			info.PreCaptureMessage = _T("Please put left middle on the sensor!");
			info.fingerName = _T("TFF_Left_Middle");
			info.capture_finger_idx = LEFT_MIDDLE;
			info.capture_finger_btn =13;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH )
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_LITTLE);
			info.PreCaptureMessage = _T("Please put left little on the sensor!");
			info.fingerName = _T("SRF_Left_Little");
			info.capture_finger_idx = LEFT_LITTLE;
			info.capture_finger_btn =13;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_MIDDLE);
			info.PreCaptureMessage = _T("Please put left middle on the sensor!");
			info.fingerName = _T("SFF_Left_Middle");
			info.capture_finger_idx = LEFT_MIDDLE;
			info.capture_finger_btn =13;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}	
		m_CaptureEnd = FALSE;
	}

	if( m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put left ring on the sensor!");
			info.fingerName = _T("SFF_Left_Ring");
			info.capture_finger_idx = LEFT_RING;
			info.capture_finger_btn = 13;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6, &m_Button7, 
		&m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton95()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(	m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_MIDDLE);
			info.PreCaptureMessage = _T("Please put left middle on the sensor!");
			info.fingerName = _T("SRF_Left_Middle");
			info.capture_finger_idx = LEFT_MIDDLE;
			info.capture_finger_btn =14;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_INDEX);
			info.PreCaptureMessage = _T("Please put left index e on the sensor!");
			info.fingerName = _T("SRF_Left_Index");
			info.capture_finger_idx = LEFT_INDEX;
			info.capture_finger_btn =14;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_RING);
			info.PreCaptureMessage = _T("Please put left ring on the sensor!");
			info.fingerName = _T("TFF_Left_Ring");
			info.capture_finger_idx = LEFT_RING;
			info.capture_finger_btn =14;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_RING);
			info.PreCaptureMessage = _T("Please put left ring on the sensor!");
			info.fingerName = _T("SFF_Left_Ring");
			info.capture_finger_idx = LEFT_RING;
			info.capture_finger_btn =14;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	if( m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_RIGHT_INDEX);
			info.PreCaptureMessage = _T("Please put left little on the sensor!");
			info.fingerName = _T("SFF_Left_Little");
			info.capture_finger_idx = LEFT_LITTLE;
			info.capture_finger_btn = 14;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6, &m_Button7, 
		&m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton96()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(	m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_RING);
			info.PreCaptureMessage = _T("Please put left ring on the sensor!");
			info.fingerName = _T("SRF_Left_Ring");
			info.capture_finger_idx = LEFT_RING;
			info.capture_finger_btn =15;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_MIDDLE);
			info.PreCaptureMessage = _T("Please put left middle e on the sensor!");
			info.fingerName = _T("SRF_Left_Middle");
			info.capture_finger_idx = LEFT_MIDDLE;
			info.capture_finger_btn =15;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
	{
		if(m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_LITTLE);
			info.PreCaptureMessage = _T("Please put left little on the sensor!");
			info.fingerName = _T("TFF_Left_Little");
			info.capture_finger_idx = LEFT_LITTLE;
			info.capture_finger_btn =15;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;

	}

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_LITTLE);
			info.PreCaptureMessage = _T("Please put left little on the sensor!");
			info.fingerName = _T("SFF_Left_Little");
			info.capture_finger_idx = LEFT_LITTLE;
			info.capture_finger_btn =15;
			info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6, &m_Button7, 
		&m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button16, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton97()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(	m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_LITTLE);
			info.PreCaptureMessage = _T("Please put left little on the sensor!");
			info.fingerName = _T("SRF_Left_Little");
			info.capture_finger_idx = LEFT_LITTLE;
			info.capture_finger_btn =16;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}
	
	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_RING);
			info.PreCaptureMessage = _T("Please put left ring e on the sensor!");
			info.fingerName = _T("SRF_Left_Ring");
			info.capture_finger_idx = LEFT_RING;
			info.capture_finger_btn =16;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6, &m_Button7, 
		&m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button17};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

void CScanFingerDlg::OnBnClickedButton98()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;

	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_LITTLE);
			info.PreCaptureMessage = _T("Please put left little e on the sensor!");
			info.fingerName = _T("SRF_Left_Little");
			info.capture_finger_idx = LEFT_LITTLE;
			info.capture_finger_btn =17;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}	
		m_CaptureEnd = FALSE;
	}

	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
			CColorButton *btnarr[16] = {&m_Button1,&m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6, &m_Button7, 
			&m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16};
	for(int i=0; i<16; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}
void CScanFingerDlg::blinkBtn()
{
	static int i=0;

	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 1){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button1.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button1.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 2){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button2.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button2.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 3){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button3.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button3.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 4){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button4.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button4.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 5){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button5.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button5.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 6){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button6.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button6.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 7){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button7.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button7.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 8){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button8.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button8.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 9){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button9.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button9.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 10){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button10.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button10.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 11){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button11.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button11.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 12){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button12.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button12.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 13){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button13.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button13.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 14){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button14.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button14.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 15){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button15.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button15.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 16){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button16.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button16.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}	
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 17){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button17.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button17.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
	if(m_pParent->m_MainDlg.m_CaptureBtnFinger == 18){
		if(((i++)/3) % 2 == 0)
		{
			m_pParent->m_ScanFingerDlg.m_Button18.SetColor(RGB(70,70,70),RGB(0,128,0));
		}
		else
		{
			m_pParent->m_ScanFingerDlg.m_Button18.SetColor(RGB(0,128,0),RGB(70,70,70));
		}
	}
}

void CScanFingerDlg::OnBnClickedButton99()
{
	if(m_CaptureEnd == FALSE)
		return;

	m_pParent->m_vecCaptureSeq.clear();
	CaptureInfo info;

	m_pParent->m_nCurrentCaptureStep=-1;


	if(m_pParent->m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE)
	{
		if( m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
		{
			//IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_LEFT_LITTLE);
			info.PreCaptureMessage = _T("Please put left little e on the sensor!");
			info.fingerName = _T("SRF_Left_Little");
			info.capture_finger_idx = LEFT_LITTLE;
			info.capture_finger_btn =17;
			info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
			info.NumberOfFinger = 1;
			m_pParent->m_vecCaptureSeq.push_back(info);
		}	
		m_CaptureEnd = FALSE;
	}
	m_FingerDisplayManager->SetFingerCompleted(m_pParent->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
			CColorButton *btnarr[17] = {&m_Button1,&m_Button2, &m_Button3, &m_Button4, &m_Button5, &m_Button6, &m_Button7, 
			&m_Button8, &m_Button9, &m_Button10, &m_Button11, &m_Button12, &m_Button13, &m_Button14, &m_Button15, &m_Button16, &m_Button17};
	for(int i=0; i<17; i++)
	{
		btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));//SetColor(RGB(0,128,0),RGB(70,70,70));
	}
	m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
}

