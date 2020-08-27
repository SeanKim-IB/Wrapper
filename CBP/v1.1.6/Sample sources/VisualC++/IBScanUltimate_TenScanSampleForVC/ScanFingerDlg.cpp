// OneFingerDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "IBScanUltimate_TenScanSampleForVC.h"
#include "ScanFingerDlg.h"
#include "IBScanUltimate_TenScanSampleForVCDlg.h"

#include "IBScanUltimateApi.h"
#include "FingerDisplayManager.h"

// CScanFingerDlg 대화 상자입니다.
const CBrush   CScanFingerDlg::s_brushRed( RGB(255,0,0) );
const CBrush   CScanFingerDlg::s_brushOrange( RGB(255,128,0) );
const CBrush   CScanFingerDlg::s_brushGreen( RGB(0,128,0) );
const CBrush   CScanFingerDlg::s_brushPlaten( RGB(74,74,74) );


IMPLEMENT_DYNAMIC(CScanFingerDlg, CDialog)

CScanFingerDlg::CScanFingerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanFingerDlg::IDD, pParent)
{
	m_FingerDisplayManager = NULL;
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
	DDX_Control(pDX, IDC_STRING_FINGER_QUALITY, m_strFingerQuality);
	DDX_Control(pDX, IDC_STRING_FINGER_STATUS, m_strFingerStatus);
}


BEGIN_MESSAGE_MAP(CScanFingerDlg, CDialog)
	ON_BN_CLICKED(IDC_SCAN_10_FINGERS, &CScanFingerDlg::OnBnClickedScan10Fingers)
	ON_BN_CLICKED(IDC_SCAN_SELECTED_FINGER, &CScanFingerDlg::OnBnClickedScanSelectedFinger)
	ON_BN_CLICKED(IDC_CLOSE, &CScanFingerDlg::OnBnClickedClose)
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_CTLCOLOR()
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
	
}

void CScanFingerDlg::UpdateFingerView()
{
	m_FingerDisplayManager->UpdateDisplayWindow();
}

void CScanFingerDlg::Init_Layout()
{
	CButton* btn;
	WINDOWPLACEMENT place;

	GetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_FINGER_STATUS);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = 10;
	place.rcNormalPosition.right = place.rcNormalPosition.left+200;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_FINGER_VIEW);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+200;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+138;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_SCAN_10_FINGERS);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+10;
	place.rcNormalPosition.right = place.rcNormalPosition.left+200;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+50;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_SCAN_SELECTED_FINGER);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right = place.rcNormalPosition.left+200;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+50;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_CLOSE);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right = place.rcNormalPosition.left+200;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+50;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_FINGER_QUALITY);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+20;
	place.rcNormalPosition.right = place.rcNormalPosition.left+200;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_FINGER_QUALITY);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+200;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+200;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STATIC_QUALITY_1);
	place.rcNormalPosition.left = 37;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom-150;
	place.rcNormalPosition.right = place.rcNormalPosition.left+35;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+120;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STATIC_QUALITY_2);
	place.rcNormalPosition.left = place.rcNormalPosition.right+5;
	place.rcNormalPosition.right = place.rcNormalPosition.left+35;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STATIC_QUALITY_3);
	place.rcNormalPosition.left = place.rcNormalPosition.right+5;
	place.rcNormalPosition.right = place.rcNormalPosition.left+35;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STATIC_QUALITY_4);
	place.rcNormalPosition.left = place.rcNormalPosition.right+5;
	place.rcNormalPosition.right = place.rcNormalPosition.left+35;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW);
	place.rcNormalPosition.left = place.rcNormalPosition.right+35;
	place.rcNormalPosition.top = 10;
	place.rcNormalPosition.right = place.rcNormalPosition.left+618;//IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+580;//IMG_H;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_TXT_STATUS);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+10;
	place.rcNormalPosition.right = place.rcNormalPosition.left+200+12+618;//+IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	m_FingerDisplayManager = new CFingerDisplayManager(&m_FingerView, 200, 138, ONE_FINGER_MODE);

	m_strFingerStatus.SetTextColor(RGB(200,200,200));
	m_strFingerStatus.SetBkColor(RGB(50,50,50));

	m_strFingerQuality.SetTextColor(RGB(200,200,200));
	m_strFingerQuality.SetBkColor(RGB(50,50,50));
}

void CScanFingerDlg::OnBnClickedScan10Fingers()
{
	if( m_pParent->m_bInitializing )
		return;

	int devIndex = m_pParent->m_MainDlg.m_DeviceList.GetCurSel() - 1;
	if( devIndex < 0 )
		return;

	int seqIndex = m_pParent->m_MainDlg.m_SequenceList.GetCurSel() - 1;
	if( seqIndex < 0 )
		return;

	if( m_pParent->m_nCurrentCaptureStep != -1 )
	{
		BOOL IsActive;
		int nRc;
		nRc = IBSU_IsCaptureActive(m_pParent->m_nDevHandle, &IsActive);
		if( nRc == IBSU_STATUS_OK && IsActive )
		{
			IBSU_TakeResultImageManually(m_pParent->m_nDevHandle);
		}

		return;
	}

	// select capture mode
	m_pParent->m_MainDlg.m_CaptureFinger = NONE_FINGER;
	
	if( m_pParent->m_nDevHandle == -1 )
	{
		m_pParent->m_bInitializing = true;
		VERIFY( ::AfxBeginThread( m_pParent->m_MainDlg._InitializeDeviceThreadCallback, m_pParent, THREAD_PRIORITY_NORMAL,
								  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
	}
	else
	{
		// device already initialized
		m_pParent->PostMessage(WM_USER_CAPTURE_SEQ_START);
	}

	m_pParent->OnMsg_UpdateDisplayResources();
}

void CScanFingerDlg::OnBnClickedScanSelectedFinger()
{
	if( m_pParent->m_bInitializing )
		return;

	int devIndex = m_pParent->m_MainDlg.m_DeviceList.GetCurSel() - 1;
	if( devIndex < 0 )
		return;

	int seqIndex = m_pParent->m_MainDlg.m_SequenceList.GetCurSel() - 1;
	if( seqIndex < 0 )
		return;

	if( m_pParent->m_nCurrentCaptureStep != -1 )
	{
		BOOL IsActive;
		int nRc;
		nRc = IBSU_IsCaptureActive(m_pParent->m_nDevHandle, &IsActive);
		if( nRc == IBSU_STATUS_OK && IsActive )
		{
			IBSU_TakeResultImageManually(m_pParent->m_nDevHandle);
		}

		return;
	}

	// select capture mode
	m_pParent->m_MainDlg.m_CaptureFinger = m_FingerDisplayManager->GetSelectedFingerIndex();

	if(m_pParent->m_MainDlg.m_CaptureFinger == NONE_FINGER)
	{
		MessageBox("Please select finger", "Scan Selected Finger", MB_OK | MB_ICONSTOP);
		return;
	}

	if( m_pParent->m_nDevHandle == -1 )
	{
		m_pParent->m_bInitializing = true;
		VERIFY( ::AfxBeginThread( m_pParent->m_MainDlg._InitializeDeviceThreadCallback, m_pParent, THREAD_PRIORITY_NORMAL,
								  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
	}
	else
	{
		// device already initialized
		m_pParent->PostMessage(WM_USER_CAPTURE_SEQ_START);
	}

	m_pParent->OnMsg_UpdateDisplayResources();
}

void CScanFingerDlg::OnBnClickedClose()
{
	m_pParent->_CaptureStop();
	m_pParent->ChangeView(0);
}

void CScanFingerDlg::OnTimer(UINT_PTR nIDEvent)
{
	UpdateFingerView();

	CDialog::OnTimer(nIDEvent);
}

void CScanFingerDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
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
	WINDOWPLACEMENT place;
	m_FingerView.GetWindowPlacement(&place);

	CRect rect(place.rcNormalPosition);
	
	if(rect.PtInRect(point) == TRUE)
	{
		point.x -= place.rcNormalPosition.left;
		point.y -= place.rcNormalPosition.top;

		m_FingerDisplayManager->ToggleEnableFinger(point);
	}

	CDialog::OnRButtonDown(nFlags, point);
}

HBRUSH CScanFingerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	const int WndID = pWnd->GetDlgCtrlID();
	for( int i = 0; i < 4; i++ )
	{
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
				switch( m_pParent->m_FingerQuality[i] )
				{ 
				case ENUM_IBSU_QUALITY_GOOD:
					hbr = static_cast<HBRUSH>(s_brushGreen);
					break;          
				case ENUM_IBSU_QUALITY_FAIR :
					hbr = static_cast<HBRUSH>(s_brushOrange);
					break;          
				case ENUM_IBSU_QUALITY_POOR :
					hbr = static_cast<HBRUSH>(s_brushRed);
					break;          
				case ENUM_IBSU_FINGER_NOT_PRESENT:
					hbr = static_cast<HBRUSH>(s_brushPlaten);
					break;
				case ENUM_IBSU_QUALITY_INVALID_AREA_TOP:
					hbr = static_cast<HBRUSH>(s_brushRed);
					break;
				case ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM:
					hbr = static_cast<HBRUSH>(s_brushRed);
					break;
				case ENUM_IBSU_QUALITY_INVALID_AREA_LEFT:
					hbr = static_cast<HBRUSH>(s_brushRed);
					break;
				case ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT:
					hbr = static_cast<HBRUSH>(s_brushRed);
					break;
				default:
					ASSERT( FALSE );
				}
			}
		}
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}
