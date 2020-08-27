// TwoFingerDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "IBScanUltimate_SalesDemo.h"
#include "TwoFingerDlg.h"
#include "IBScanUltimate_SalesDemoDlg.h"

#include "IBScanUltimateApi.h"
#include "FingerDisplayManager.h"

// CTwoFingerDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CTwoFingerDlg, CDialog)

CTwoFingerDlg::CTwoFingerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTwoFingerDlg::IDD, pParent)
{
	m_FingerDisplayManager = NULL;
}

CTwoFingerDlg::~CTwoFingerDlg()
{
	if(m_FingerDisplayManager)
		delete m_FingerDisplayManager;
}

void CTwoFingerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FINGER_VIEW, m_FingerView);
	DDX_Control(pDX, IDC_VIEW, m_CapView);
	DDX_Control(pDX, IDC_STRING_FINGER_QUALITY, m_strFingerQuality);
	DDX_Control(pDX, IDC_STRING_FINGER_STATUS, m_strFingerStatus);
}


BEGIN_MESSAGE_MAP(CTwoFingerDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(IDC_SCAN_10_FINGERS, &CTwoFingerDlg::OnBnClickedScan10Fingers)
	ON_BN_CLICKED(IDC_SCAN_SELECTED_FINGER, &CTwoFingerDlg::OnBnClickedScanSelectedFinger)
	ON_BN_CLICKED(IDC_CLOSE, &CTwoFingerDlg::OnBnClickedClose)
END_MESSAGE_MAP()


// CTwoFingerDlg 메시지 처리기입니다.
BOOL CTwoFingerDlg::PreTranslateMessage(MSG* pMsg)
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

BOOL CTwoFingerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	Init_Layout();

	SetTimer(1, 100, NULL);

	return TRUE;
}

void CTwoFingerDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CDialog::OnPaint()을(를) 호출하지 마십시오.
	DrawImage();
	UpdateFingerView();
}

void CTwoFingerDlg::DrawImage()
{
	
}

void CTwoFingerDlg::UpdateFingerView()
{
	m_FingerDisplayManager->UpdateDisplayWindow();
}

void CTwoFingerDlg::Init_Layout()
{
	CButton* btn;
	WINDOWPLACEMENT place;

	GetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_FINGER_STATUS);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = 10;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_FINGER_VIEW);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+205;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_SCAN_10_FINGERS);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+10;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+50;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_SCAN_SELECTED_FINGER);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+50;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_CLOSE);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+50;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_FINGER_QUALITY);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+20;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_FINGER_QUALITY);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+300;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW);
	place.rcNormalPosition.left = place.rcNormalPosition.right+15;
	place.rcNormalPosition.top = 10;
	place.rcNormalPosition.right = place.rcNormalPosition.left+IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+IMG_H;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_TXT_STATUS);
	place.rcNormalPosition.left = 15;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+10;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300+IMG_W+15;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	m_FingerDisplayManager = new CFingerDisplayManager(&m_FingerView, 300, 205, TWO_FINGER_MODE);

	m_strFingerStatus.SetTextColor(RGB(200,200,200));
	m_strFingerStatus.SetBkColor(RGB(50,50,50));

	m_strFingerQuality.SetTextColor(RGB(200,200,200));
	m_strFingerQuality.SetBkColor(RGB(50,50,50));
}

void CTwoFingerDlg::OnBnClickedClose()
{
	m_pParent->_CaptureStop();
	m_pParent->ChangeView(0);
}

void CTwoFingerDlg::OnTimer(UINT_PTR nIDEvent)
{
	UpdateFingerView();

	CDialog::OnTimer(nIDEvent);
}

void CTwoFingerDlg::OnLButtonDown(UINT nFlags, CPoint point)
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

void CTwoFingerDlg::OnRButtonDown(UINT nFlags, CPoint point)
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

void CTwoFingerDlg::OnBnClickedScan10Fingers()
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

void CTwoFingerDlg::OnBnClickedScanSelectedFinger()
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
