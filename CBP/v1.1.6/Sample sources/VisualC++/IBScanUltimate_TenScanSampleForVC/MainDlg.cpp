// MainDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "IBScanUltimate_TenScanSampleForVC.h"
#include "MainDlg.h"
#include "IBScanUltimate_TenScanSampleForVCDlg.h"

#include "FingerDisplayManager.h"
#include "IBScanUltimateApi.h"

// CMainDlg 대화 상자입니다.

// CIBScanUltimate_TenScanSampleForVCDlg dialog
int CALLBACK BrowseForFolderCallback(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData)
{
	char szPath[MAX_PATH];

	switch( uMsg )
	{
	case BFFM_INITIALIZED:
		SendMessage( hwnd, BFFM_SETSELECTION, TRUE, pData );
		break;
	case BFFM_SELCHANGED: 
		if( SHGetPathFromIDList( (LPITEMIDLIST)lp ,szPath ) ) 
		{
			SendMessage( hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szPath ); 
		}
		break;
	}

	return 0;
}

IMPLEMENT_DYNAMIC(CMainDlg, CDialog)

CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDlg::IDD, pParent)
{
	m_CaptureMode = NONE_CAPTURE;
	m_CaptureFinger = NONE_FINGER;
	m_CaptureCompleted = FALSE;
	m_UseAutoSplit = FALSE;
	m_Run_AutoSplit = FALSE;

	int i;

	for(i=0; i<IMAGE_BUFFER_COUNT; i++)
	{
		m_ImgBuf[i] = new BYTE [IMG_SIZE];
		memset(m_ImgBuf[i], 0xFF, IMG_SIZE);
		m_ImgBuf_S[i] = new BYTE [S_IMG_SIZE];
		memset(m_ImgBuf_S[i], 0xFF, S_IMG_SIZE);
	}
	
	m_Info = (BITMAPINFO*)new BYTE[1064];
	for(i=0; i<256; i++)
	{
		m_Info->bmiColors[i].rgbBlue = 
		m_Info->bmiColors[i].rgbRed = 
		m_Info->bmiColors[i].rgbGreen = (BYTE)i;
	}
	m_Info->bmiHeader.biBitCount = 8;
	m_Info->bmiHeader.biClrImportant = 0;
	m_Info->bmiHeader.biClrUsed = 0;
	m_Info->bmiHeader.biCompression = BI_RGB;
	m_Info->bmiHeader.biHeight = S_IMG_H;
	m_Info->bmiHeader.biPlanes = 1;
	m_Info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_Info->bmiHeader.biSizeImage = S_IMG_SIZE;
	m_Info->bmiHeader.biWidth = S_IMG_W;
	m_Info->bmiHeader.biXPelsPerMeter = 19700;
	m_Info->bmiHeader.biYPelsPerMeter = 19700;

	memset(m_NFIQBuf, 0, sizeof(m_NFIQBuf));
}

CMainDlg::~CMainDlg()
{
	for(int i=0; i<IMAGE_BUFFER_COUNT; i++)
	{
		delete [] m_ImgBuf[i];
		delete [] m_ImgBuf_S[i];
	}

	delete m_Info;

	delete m_ZoomDlg;
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVICE_LIST, m_DeviceList);
	DDX_Control(pDX, IDC_SEQUENCE_LIST, m_SequenceList);
	DDX_Control(pDX, IDC_START_CAPTURE, m_btnCapture);
	DDX_Control(pDX, IDC_EDIT_CAPTURE, m_btnEdit);
	DDX_Control(pDX, IDC_SAVE_CAPTURE, m_btnSave);
	DDX_Control(pDX, IDC_STRING_VIEW_0, m_strView0);
	DDX_Control(pDX, IDC_STRING_VIEW_1, m_strView1);
	DDX_Control(pDX, IDC_STRING_VIEW_2, m_strView2);
	DDX_Control(pDX, IDC_STRING_VIEW_3, m_strView3);
	DDX_Control(pDX, IDC_STRING_VIEW_4, m_strView4);
	DDX_Control(pDX, IDC_STRING_VIEW_5, m_strView5);
	DDX_Control(pDX, IDC_STRING_VIEW_6, m_strView6);
	DDX_Control(pDX, IDC_STRING_VIEW_7, m_strView7);
	DDX_Control(pDX, IDC_STRING_VIEW_8, m_strView8);
	DDX_Control(pDX, IDC_STRING_VIEW_9, m_strView9);
	DDX_Control(pDX, IDC_STRING_DEVICE_LIST, m_strDeviceList);
	DDX_Control(pDX, IDC_STRING_CAPTURE_SEQUENCES, m_strCaptureSeq);
	DDX_Check(pDX, IDC_USE_AUTO_SPLIT, m_UseAutoSplit);
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_DEVICE_LIST, &CMainDlg::OnCbnSelchangeDeviceList)
	ON_CBN_SELCHANGE(IDC_SEQUENCE_LIST, &CMainDlg::OnCbnSelchangeSequenceList)
	ON_BN_CLICKED(IDC_START_CAPTURE, &CMainDlg::OnBnClickedStartCapture)
	ON_BN_CLICKED(IDC_EDIT_CAPTURE, &CMainDlg::OnBnClickedEditCapture)
	ON_BN_CLICKED(IDC_SAVE_CAPTURE, &CMainDlg::OnBnClickedSaveCapture)
	ON_BN_CLICKED(IDC_USE_AUTO_SPLIT, &CMainDlg::OnBnClickedUseAutoSplit)
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()


// CMainDlg 메시지 처리기입니다.
BOOL CMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	Init_Layout();

	m_nCurrWidth = IMG_W;
	m_nCurrHeight = IMG_H;

	for(int i=0;i<IMAGE_BUFFER_COUNT;i++)
	{
		m_nCurrentWidthForSplit[i] = IMG_W;
		m_nCurrentHeightForSplit[i] = IMG_H;
	}

	m_ZoomDlg = new CZoomDlg();
	m_ZoomDlg->Create(IDD_ZOOM_DIALOG);

	return TRUE;
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
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

void CMainDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CDialog::OnPaint()을(를) 호출하지 마십시오.

	DrawImage();
}

void CMainDlg::DrawImage()
{
	int i;
	CString str;

	if( m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE || 
		m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE ||
		m_CaptureMode == NONE_CAPTURE ||
		( m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE && m_Run_AutoSplit == TRUE ) 
		)
	{
		for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
		{
			CClientDC dc((CStatic *)GetDlgItem(IDC_VIEW_0+i));
			StretchDIBits(dc.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[i], m_Info, DIB_RGB_COLORS, SRCCOPY);
			if(m_NFIQBuf[i]>0)
			{
				str.Format("%d", m_NFIQBuf[i]);
				dc.SetBkMode(TRANSPARENT);
				dc.TextOut(5, 5, str);
			}
		}
	}
	else if( m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE )
	{
		for(i=LEFT_RING_LITTLE; i<=RIGHT_RING_LITTLE; i++)
		{
			CClientDC dc((CStatic *)GetDlgItem(IDC_VIEW_0+i-LEFT_RING_LITTLE));
			StretchDIBits(dc.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[i], m_Info, DIB_RGB_COLORS, SRCCOPY);
			if(m_NFIQBuf[i]>0)
			{
				str.Format("%d", m_NFIQBuf[i]);
				dc.SetBkMode(TRANSPARENT);
				dc.TextOut(5, 5, str);
			}
		}
	}
}

void CMainDlg::Init_Layout()
{
	CButton* btn;
	WINDOWPLACEMENT place;

	GetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_DEVICE_LIST);
	place.rcNormalPosition.left = 20;
	place.rcNormalPosition.top = 10;
	place.rcNormalPosition.right = place.rcNormalPosition.left+250;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_DEVICE_LIST);
	place.rcNormalPosition.left = 20;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+250;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+200;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_CAPTURE_SEQUENCES);
	place.rcNormalPosition.left = 20;
	place.rcNormalPosition.top = 100;
	place.rcNormalPosition.right = place.rcNormalPosition.left+250;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_SEQUENCE_LIST);
	place.rcNormalPosition.left = 20;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+250;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+200;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_START_CAPTURE);
	place.rcNormalPosition.left = place.rcNormalPosition.right+30;
	place.rcNormalPosition.top = 30;
	place.rcNormalPosition.right = place.rcNormalPosition.left+160;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+100;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_EDIT_CAPTURE);
	place.rcNormalPosition.left = place.rcNormalPosition.right+30;
	place.rcNormalPosition.right = place.rcNormalPosition.left+160;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+100;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_SAVE_CAPTURE);
	place.rcNormalPosition.left = place.rcNormalPosition.right+30;
	place.rcNormalPosition.right = place.rcNormalPosition.left+160;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+100;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_USE_AUTO_SPLIT);
	place.rcNormalPosition.left = 300;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+10;
	place.rcNormalPosition.right = place.rcNormalPosition.left+800;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+20;
	btn->SetWindowPlacement(&place);

	int top = place.rcNormalPosition.bottom+50;

	btn = (CButton*)GetDlgItem(IDC_STRING_VIEW_0);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = top;
	place.rcNormalPosition.right = place.rcNormalPosition.left+S_IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_0);
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+S_IMG_H;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_VIEW_1);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = top;
	place.rcNormalPosition.right = place.rcNormalPosition.left+S_IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_1);
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+S_IMG_H;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_VIEW_2);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = top;
	place.rcNormalPosition.right = place.rcNormalPosition.left+S_IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_2);
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+S_IMG_H;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_VIEW_3);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = top;
	place.rcNormalPosition.right = place.rcNormalPosition.left+S_IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_3);
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+S_IMG_H;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_VIEW_4);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = top;
	place.rcNormalPosition.right = place.rcNormalPosition.left+S_IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_4);
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+S_IMG_H;
	btn->SetWindowPlacement(&place);

	top = place.rcNormalPosition.bottom+45;

	btn = (CButton*)GetDlgItem(IDC_STRING_VIEW_5);
	place.rcNormalPosition.left = 10;
	place.rcNormalPosition.top = top;
	place.rcNormalPosition.right = place.rcNormalPosition.left+S_IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_5);
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+S_IMG_H;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_VIEW_6);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = top;
	place.rcNormalPosition.right = place.rcNormalPosition.left+S_IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_6);
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+S_IMG_H;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_VIEW_7);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = top;
	place.rcNormalPosition.right = place.rcNormalPosition.left+S_IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_7);
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+S_IMG_H;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_VIEW_8);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = top;
	place.rcNormalPosition.right = place.rcNormalPosition.left+S_IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_8);
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+S_IMG_H;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_VIEW_9);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = top;
	place.rcNormalPosition.right = place.rcNormalPosition.left+S_IMG_W;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_9);
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+S_IMG_H;
	btn->SetWindowPlacement(&place);

	m_btnCapture.SetIcon(IDI_START_CAPTURE);
	m_btnCapture.SetFlat(FALSE);

	m_btnEdit.SetIcon(IDI_EDIT);
	m_btnEdit.SetFlat(FALSE);

	m_btnSave.SetIcon(IDI_SAVE);
	m_btnSave.SetFlat(FALSE);

	m_strDeviceList.SetTextColor(RGB(200,200,200));
	m_strDeviceList.SetBkColor(RGB(50,50,50));

	m_strCaptureSeq.SetTextColor(RGB(200,200,200));
	m_strCaptureSeq.SetBkColor(RGB(50,50,50));

	m_strView0.SetTextColor(RGB(200,200,200));
	m_strView0.SetBkColor(RGB(50,50,50));

	m_strView1.SetTextColor(RGB(200,200,200));
	m_strView1.SetBkColor(RGB(50,50,50));

	m_strView2.SetTextColor(RGB(200,200,200));
	m_strView2.SetBkColor(RGB(50,50,50));

	m_strView3.SetTextColor(RGB(200,200,200));
	m_strView3.SetBkColor(RGB(50,50,50));

	m_strView4.SetTextColor(RGB(200,200,200));
	m_strView4.SetBkColor(RGB(50,50,50));

	m_strView5.SetTextColor(RGB(200,200,200));
	m_strView5.SetBkColor(RGB(50,50,50));

	m_strView6.SetTextColor(RGB(200,200,200));
	m_strView6.SetBkColor(RGB(50,50,50));

	m_strView7.SetTextColor(RGB(200,200,200));
	m_strView7.SetBkColor(RGB(50,50,50));

	m_strView8.SetTextColor(RGB(200,200,200));
	m_strView8.SetBkColor(RGB(50,50,50));

	m_strView9.SetTextColor(RGB(200,200,200));
	m_strView9.SetBkColor(RGB(50,50,50));

	OneFinger_Layout();
}

void CMainDlg::OneFinger_Layout()
{
	int i;
	CWnd *wnd;
	CString str_table[10] = {"Left Little", "Left Ring", "Left Middle", "Left Index", "Left Thumb", 
							"Right Thumb", "Right Index", "Right Middle", "Right Ring", "Right Little"};

	for(i=0; i<10; i++)
	{
		CStatic *view = (CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_SHOW);
	
		wnd = (CStatic *)GetDlgItem(IDC_STRING_VIEW_0+i);
		wnd->SetWindowText(str_table[i]);
		wnd->ShowWindow(SW_SHOW);
	}
}

void CMainDlg::TwoFinger_Layout()
{
	int i;
	CWnd *wnd;
	CString str_table[5] = {"Left Ring && Little", "Left Index && Middle", "Both Thumbs", "Right Index && Middle", "Right Ring && Little"}; 

	for(i=0; i<5; i++)
	{
		CStatic *view = (CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_SHOW);
	
		wnd = (CStatic *)GetDlgItem(IDC_STRING_VIEW_0+i);
		wnd->SetWindowText(str_table[i]);
		wnd->ShowWindow(SW_SHOW);
	}

	for(i=5; i<10; i++)
	{
		CStatic *view = (CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_HIDE);
		wnd = (CStatic *)GetDlgItem(IDC_STRING_VIEW_0+i);
		wnd->ShowWindow(SW_HIDE);
	}
}

UINT CMainDlg::_InitializeDeviceThreadCallback( LPVOID pParam )
{   
	if( pParam == NULL )
		return 1;

	CIBScanUltimate_TenScanSampleForVCDlg*	pDlg = (CIBScanUltimate_TenScanSampleForVCDlg*)pParam;
	const int	devIndex = pDlg->m_MainDlg.m_DeviceList.GetCurSel() - 1;
	int			devHandle;
	int			nRc = IBSU_STATUS_OK;
	CWnd		*disWnd;
	RECT		clientRect;

	pDlg->m_bInitializing = true;
	
    nRc = IBSU_OpenDevice( devIndex, &devHandle );

	pDlg->m_bInitializing = false;

	if( nRc >= IBSU_STATUS_OK )
	{
		pDlg->m_nDevHandle = devHandle;
		disWnd = pDlg->m_ScanFingerDlg.GetDlgItem( IDC_VIEW );
		
		disWnd->GetClientRect( &clientRect );

		IBSU_CreateClientWindow( devHandle, disWnd->m_hWnd, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);

		///For ResultImageEx
		IBSU_AddOverlayQuadrangle(devHandle, &pDlg->m_nOvClearPlatenHandle, 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);
		IBSU_AddOverlayText(devHandle, &pDlg->m_nOvImageTextHandle, "Arial", 10, TRUE/*bold*/, "", 10, 10, (DWORD)0);
		for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
			IBSU_AddOverlayQuadrangle(devHandle, &pDlg->m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);
		///

		// register callback functions
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, pDlg->OnEvent_DeviceCommunicationBreak, pDlg );    
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, pDlg->OnEvent_PreviewImage, pDlg );
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, pDlg->OnEvent_TakingAcquisition, pDlg );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, pDlg->OnEvent_CompleteAcquisition, pDlg );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX, pDlg->OnEvent_ResultImageEx, pDlg );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, pDlg->OnEvent_FingerCount, pDlg );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY, pDlg->OnEvent_FingerQuality, pDlg );  
		if( pDlg->m_chkUseClearPlaten )
			IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE, pDlg->OnEvent_ClearPlatenAtCapture, pDlg );  
	}

	// status notification and sequence start
	if( nRc == IBSU_STATUS_OK )
	{
		pDlg->PostMessage( WM_USER_CAPTURE_SEQ_START );
		return 0;
	}

	if( nRc > IBSU_STATUS_OK )
		pDlg->PostMessage( WM_USER_INIT_WARNING, nRc );
	else 
	{
		switch (nRc)
		{
		case IBSU_ERR_DEVICE_ACTIVE:
			TRACE( _T( "[Error code = %d] Device initialization failed because in use by another thread/process." ), nRc );
			break;
		case IBSU_ERR_USB20_REQUIRED:
			TRACE( _T( "[Error code = %d] Device initialization failed because SDK only works with USB 2.0." ), nRc );
			break;
		default:
			TRACE( _T( "[Error code = %d] Device initialization failed" ), nRc );
			break;
		}
	}

	pDlg->PostMessage( WM_USER_UPDATE_DISPLAY_RESOURCES );
	return 0;
}

void CMainDlg::OnCbnSelchangeDeviceList()
{
	if( m_DeviceList.GetCurSel() == m_pParent->m_nSelectedDevIndex )
		return;

	AfxGetApp()->DoWaitCursor( 1 );

	m_pParent->m_nSelectedDevIndex = m_DeviceList.GetCurSel(); 
	if( m_pParent->m_nDevHandle != -1 )
	{
		m_pParent->_CaptureStop();
		m_pParent->_ReleaseDevice();
	}

	_UpdateCaptureSequences();

	AfxGetApp()->DoWaitCursor( -1 );
}

void CMainDlg::_UpdateCaptureSequences()
{
	// store currently selected sequence
	CString strSelectedText;
	int selectedSeq = m_SequenceList.GetCurSel();
	if( selectedSeq > -1 )
		m_SequenceList.GetLBText( selectedSeq, strSelectedText );

	// populate combo box
	m_SequenceList.ResetContent();
	m_SequenceList.AddString( _T( "- Please select -" ) );

	const int devIndex = m_DeviceList.GetCurSel() - 1;
	IBSU_DeviceDesc devDesc;
	devDesc.productName[0] = 0;  
	if( devIndex > -1 )
		IBSU_GetDeviceDescription( devIndex, &devDesc );

	if( ( _stricmp( devDesc.productName, "WATSON" )			== 0) ||
		( _stricmp( devDesc.productName, "WATSON MINI" )	== 0) ||
		( _stricmp( devDesc.productName, "SHERLOCK_ROIC" )	== 0) ||
		( _stricmp( devDesc.productName, "SHERLOCK" )		== 0) )
	{
		m_SequenceList.AddString( CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS );
		m_SequenceList.AddString( CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS );
		m_SequenceList.AddString( CAPTURE_SEQ_10_DOUBLE_FLAT_FINGERS );
	}
	else if( ( _stricmp( devDesc.productName, "COLUMBO" )		== 0) ||
			 ( _stricmp( devDesc.productName, "CURVE" )			== 0) )
	{
		m_SequenceList.AddString( CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS );
	}

	// select previously selected sequence
	if( selectedSeq > -1 )
		selectedSeq = m_SequenceList.FindString( 0, strSelectedText );
	m_SequenceList.SetCurSel( selectedSeq == -1 ? 0 : selectedSeq );

	m_pParent->PostMessage(WM_USER_UPDATE_DISPLAY_RESOURCES);
}

void CMainDlg::OnCbnSelchangeSequenceList()
{
	m_pParent->PostMessage(WM_USER_UPDATE_DISPLAY_RESOURCES);
}

void CMainDlg::OnBnClickedStartCapture()
{
	CString strCaptureSeq;
	int nSelectedSeq;
	nSelectedSeq = m_SequenceList.GetCurSel();
	if( nSelectedSeq > -1 )
	m_SequenceList.GetLBText( nSelectedSeq, strCaptureSeq );

	if(m_CaptureCompleted == TRUE)
	{
		if(AfxMessageBox("Data will be lost. Do you want to continue?", MB_YESNO | MB_ICONINFORMATION) == IDNO)
			return;

		CWnd *disWnd;
		RECT		clientRect;
		IBSU_DestroyClientWindow(m_pParent->m_nDevHandle, TRUE);
		disWnd = m_pParent->m_ScanFingerDlg.GetDlgItem( IDC_VIEW );
		
		disWnd->GetClientRect( &clientRect );

		IBSU_CreateClientWindow( m_pParent->m_nDevHandle, disWnd->m_hWnd, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);

		///For ResultImageEx
		IBSU_AddOverlayQuadrangle(m_pParent->m_nDevHandle, &m_pParent->m_nOvClearPlatenHandle, 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);
		IBSU_AddOverlayText(m_pParent->m_nDevHandle, &m_pParent->m_nOvImageTextHandle, "Arial", 10, TRUE/*bold*/, "", 10, 10, (DWORD)0);
		for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
			IBSU_AddOverlayQuadrangle(m_pParent->m_nDevHandle, &m_pParent->m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);
	}

	if( strCaptureSeq == CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS )
	{
		m_CaptureMode = TEN_FINGER_ONE_FLAT_CAPTURE;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(ONE_FINGER_MODE);
		m_pParent->m_MainDlg.OneFinger_Layout();
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS);
	}
	else if( strCaptureSeq == CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS )
	{
		m_CaptureMode = TEN_FINGER_ONE_ROLL_CAPTURE;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(ONE_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS);
		m_pParent->m_MainDlg.OneFinger_Layout();
	}
	else if( strCaptureSeq == CAPTURE_SEQ_10_DOUBLE_FLAT_FINGERS )
	{
		m_CaptureMode = TEN_FINGER_TWO_FLAT_CAPTURE;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(TWO_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(CAPTURE_SEQ_10_DOUBLE_FLAT_FINGERS);
		m_pParent->m_MainDlg.TwoFinger_Layout();

		if(m_pParent->m_MainDlg.m_UseAutoSplit == TRUE)
		{
			m_pParent->m_MainDlg.m_Run_AutoSplit = TRUE;
			m_pParent->m_MainDlg.OneFinger_Layout();
		}
		else
		{
			m_pParent->m_MainDlg.m_Run_AutoSplit = FALSE;
			m_pParent->m_MainDlg.TwoFinger_Layout();
		}
	}
	else
	{
		AfxMessageBox("None Selected.", MB_OK | MB_ICONSTOP);
		return;
	}

	// initialize fingerprint memory buffer
	for(int i=0; i<IMAGE_BUFFER_COUNT; i++)
	{
		memset(m_ImgBuf[i], 0xFF, IMG_SIZE);
		memset(m_ImgBuf_S[i], 0xFF, S_IMG_SIZE);
	}
	memset(m_NFIQBuf, 0, sizeof(m_NFIQBuf));
	memset(m_nCurrentWidthForSplit, 0, sizeof(m_nCurrentWidthForSplit));
	memset(m_nCurrentHeightForSplit, 0, sizeof(m_nCurrentHeightForSplit));
	m_nCurrWidth = 0;
	m_nCurrHeight = 0;

	// go to tab2
	m_pParent->ChangeView(1);
	m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->InitializeVariables();
}

void CMainDlg::OnBnClickedEditCapture()
{
	// go to tab2
	m_pParent->ChangeView(1);
}

void CMainDlg::OnBnClickedSaveCapture()
{
	LPITEMIDLIST	pidlSelected;
	BROWSEINFO		bi = {0};
	LPMALLOC		pMalloc;
	TCHAR			m_ImgSaveFolder[MAX_PATH + 1];				///< Base folder for image saving
	int				i;

	SHGetMalloc( &pMalloc );

	// show 'Browse For Folder' dialog:
	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = _T("Please select a folder to store captured images!");
	bi.ulFlags = BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseForFolderCallback;
	bi.lParam = (LPARAM)&m_ImgSaveFolder[0];

	pidlSelected = SHBrowseForFolder(&bi);

	if( pidlSelected )
	{
		SHGetPathFromIDList( pidlSelected, m_ImgSaveFolder );
		pMalloc->Free(pidlSelected);

        if(IBSU_IsWritableDirectory(m_ImgSaveFolder, TRUE) != IBSU_STATUS_OK)
		{
			AfxMessageBox("You don't have writing permission on this folder\r\nPlease select another folder (e.g. desktop folder)");
			return;
		}

		// Save
		CString filename_bmp[19], filename_wsq[19], full_filename;
		int pitch = 0 - m_nCurrWidth;		// reverse row order
		int pitchForSplit = 0;

		if( m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
			( m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE && m_Run_AutoSplit == TRUE ) )
		{
			filename_bmp[LEFT_LITTLE] = _T("SFF_Left_Little.bmp");
			filename_bmp[LEFT_RING] = _T("SFF_Left_Ring.bmp");
			filename_bmp[LEFT_MIDDLE] = _T("SFF_Left_Middle.bmp");
			filename_bmp[LEFT_INDEX] = _T("SFF_Left_Index.bmp");
			filename_bmp[LEFT_THUMB] = _T("SFF_Left_Thumb.bmp");
			filename_bmp[RIGHT_THUMB] = _T("SFF_Right_Thumb.bmp");
			filename_bmp[RIGHT_INDEX] = _T("SFF_Right_Index.bmp");
			filename_bmp[RIGHT_MIDDLE] = _T("SFF_Right_Middle.bmp");
			filename_bmp[RIGHT_RING] = _T("SFF_Right_Ring.bmp");
			filename_bmp[RIGHT_LITTLE] = _T("SFF_Right_Little.bmp");

			filename_wsq[LEFT_LITTLE] = _T("SFF_Left_Little.wsq");
			filename_wsq[LEFT_RING] = _T("SFF_Left_Ring.wsq");
			filename_wsq[LEFT_MIDDLE] = _T("SFF_Left_Middle.wsq");
			filename_wsq[LEFT_INDEX] = _T("SFF_Left_Index.wsq");
			filename_wsq[LEFT_THUMB] = _T("SFF_Left_Thumb.wsq");
			filename_wsq[RIGHT_THUMB] = _T("SFF_Right_Thumb.wsq");
			filename_wsq[RIGHT_INDEX] = _T("SFF_Right_Index.wsq");
			filename_wsq[RIGHT_MIDDLE] = _T("SFF_Right_Middle.wsq");
			filename_wsq[RIGHT_RING] = _T("SFF_Right_Ring.wsq");
			filename_wsq[RIGHT_LITTLE] = _T("SFF_Right_Little.wsq");

			for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
			{
				if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(i) == FALSE)
					continue;

				full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bmp[i]);
				if( m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE && m_Run_AutoSplit == TRUE )
				{
					pitchForSplit = 0 - m_nCurrentWidthForSplit[i];
					if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[i], 
						m_nCurrentWidthForSplit[i], m_nCurrentHeightForSplit[i], pitchForSplit, 500, 500 ) != IBSU_STATUS_OK )
					{
						AfxMessageBox( _T( "Failed to save bitmap image!" ));
						break;
					}
				}
				else
				{
					if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[i], 
						m_nCurrWidth, m_nCurrHeight, pitch, 500, 500 ) != IBSU_STATUS_OK )
					{
						AfxMessageBox( _T( "Failed to save bitmap image!" ));
						break;
					}
				}

				full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[i]);
				if( m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE && m_Run_AutoSplit == TRUE )
				{
					if( SaveWSQ(full_filename, m_ImgBuf[i], m_nCurrentWidthForSplit[i], m_nCurrentHeightForSplit[i], pitchForSplit) == 0 )
					{
						AfxMessageBox( _T( "Failed to save wsq image!" ));
						break;
					}
				}
				else
				{
					if( SaveWSQ(full_filename, m_ImgBuf[i], m_nCurrWidth, m_nCurrHeight, pitch) == 0 )
					{
						AfxMessageBox( _T( "Failed to save wsq image!" ));
						break;
					}
				}
			}
		}
		else if( m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE )
		{
			filename_bmp[LEFT_LITTLE] = _T("SRF_Left_Little.bmp");
			filename_bmp[LEFT_RING] = _T("SRF_Left_Ring.bmp");
			filename_bmp[LEFT_MIDDLE] = _T("SRF_Left_Middle.bmp");
			filename_bmp[LEFT_INDEX] = _T("SRF_Left_Index.bmp");
			filename_bmp[LEFT_THUMB] = _T("SRF_Left_Thumb.bmp");
			filename_bmp[RIGHT_THUMB] = _T("SRF_Right_Thumb.bmp");
			filename_bmp[RIGHT_INDEX] = _T("SRF_Right_Index.bmp");
			filename_bmp[RIGHT_MIDDLE] = _T("SRF_Right_Middle.bmp");
			filename_bmp[RIGHT_RING] = _T("SRF_Right_Ring.bmp");
			filename_bmp[RIGHT_LITTLE] = _T("SRF_Right_Little.bmp");

			filename_wsq[LEFT_LITTLE] = _T("SRF_Left_Little.wsq");
			filename_wsq[LEFT_RING] = _T("SRF_Left_Ring.wsq");
			filename_wsq[LEFT_MIDDLE] = _T("SRF_Left_Middle.wsq");
			filename_wsq[LEFT_INDEX] = _T("SRF_Left_Index.wsq");
			filename_wsq[LEFT_THUMB] = _T("SRF_Left_Thumb.wsq");
			filename_wsq[RIGHT_THUMB] = _T("SRF_Right_Thumb.wsq");
			filename_wsq[RIGHT_INDEX] = _T("SRF_Right_Index.wsq");
			filename_wsq[RIGHT_MIDDLE] = _T("SRF_Right_Middle.wsq");
			filename_wsq[RIGHT_RING] = _T("SRF_Right_Ring.wsq");
			filename_wsq[RIGHT_LITTLE] = _T("SRF_Right_Little.wsq");

			for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
			{
				if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(i) == FALSE)
					continue;

				full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bmp[i]);
				if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[i], 
										  m_nCurrWidth, m_nCurrHeight, pitch, 500, 500 ) != IBSU_STATUS_OK )
				{
					AfxMessageBox( _T( "Failed to save bitmap image!" ));
					break;
				}

				full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[i]);
				if( SaveWSQ(full_filename, m_ImgBuf[i], m_nCurrWidth, m_nCurrHeight, pitch) == 0 )
				{
					AfxMessageBox( _T( "Failed to save wsq image!" ));
					break;
				}
			}
		}
		else if( m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE )
		{
			filename_bmp[LEFT_RING_LITTLE] = _T("STF_Left_Ring_Little.bmp");
			filename_bmp[LEFT_INDEX_MIDDLE] = _T("STF_Left_Index_Middle.bmp");
			filename_bmp[BOTH_THUMBS] = _T("STF_Both_Thumbs.bmp");
			filename_bmp[RIGHT_INDEX_MIDDLE] = _T("STF_Right_Index_Middle.bmp");
			filename_bmp[RIGHT_RING_LITTLE] = _T("STF_Right_Ring_Little.bmp");

			filename_wsq[LEFT_RING_LITTLE] = _T("STF_Left_Ring_Little.wsq");
			filename_wsq[LEFT_INDEX_MIDDLE] = _T("STF_Left_Index_Middle.wsq");
			filename_wsq[BOTH_THUMBS] = _T("STF_Both_Thumbs.wsq");
			filename_wsq[RIGHT_INDEX_MIDDLE] = _T("STF_Right_Index_Middle.wsq");
			filename_wsq[RIGHT_RING_LITTLE] = _T("STF_Right_Ring_Little.wsq");

			for(i=LEFT_RING_LITTLE; i<=RIGHT_RING_LITTLE; i++)
			{
				int index = (i - LEFT_RING_LITTLE)*2;
				if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(index) == FALSE && 
					m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(index+1) == FALSE)
					continue;

				full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bmp[i]);		
				if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[i], 
										  m_nCurrWidth, m_nCurrHeight, pitch, 500, 500 ) != IBSU_STATUS_OK )
				{
					AfxMessageBox( _T( "Failed to save bitmap image!" ));
					break;
				}

				full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[i]);
				if( SaveWSQ(full_filename, m_ImgBuf[i], m_nCurrWidth, m_nCurrHeight, pitch) == 0 )
				{
					AfxMessageBox( _T( "Failed to save wsq image!" ));
					break;
				}
			}
		}
	}
	pMalloc->Release();
}

void CMainDlg::OnBnClickedUseAutoSplit()
{
	UpdateData(TRUE);
}

int CMainDlg::SaveWSQ(CString filename, unsigned char *buffer, int width, int height, int pitch)
{
	// Bug Fixed, WSQ image was flipped vertically.
	// Pitch parameter is required to fix bug.
	if( IBSU_WSQEncodeToFile(filename, buffer, width, height, pitch,
		8, 500, 0.75, "") < IBSU_STATUS_OK)
		return 0;

	return 1;
}

void CMainDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	WINDOWPLACEMENT place;
	CWnd *pWnd;
	CRect rect;
	int i;

	if( m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE || 
		m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE ||
		(m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE && m_Run_AutoSplit == TRUE) )
	{
		for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
		{
			pWnd = GetDlgItem(IDC_VIEW_0+i-LEFT_LITTLE);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			
			if(rect.PtInRect(point) == TRUE)
			{
				if(m_Run_AutoSplit == TRUE)
				{
					m_ZoomDlg->m_InImgWidth = m_nCurrentWidthForSplit[i];
					m_ZoomDlg->m_InImgHeight = m_nCurrentHeightForSplit[i];
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				}
				else
				{
					m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
					m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				}

				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[i], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[i], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}
		}
	}
	else if( m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE )
	{
		for(i=LEFT_RING_LITTLE; i<=RIGHT_RING_LITTLE; i++)
		{
			pWnd = GetDlgItem(IDC_VIEW_0+i-LEFT_RING_LITTLE);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			
			if(rect.PtInRect(point) == TRUE)
			{
				if(m_Run_AutoSplit == TRUE)
				{
					m_ZoomDlg->m_InImgWidth = m_nCurrentWidthForSplit[i];
					m_ZoomDlg->m_InImgHeight = m_nCurrentHeightForSplit[i];
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				}
				else
				{
					m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
					m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				}

				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[i], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[i], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}
		}
	}

	CDialog::OnRButtonDown(nFlags, point);
}
