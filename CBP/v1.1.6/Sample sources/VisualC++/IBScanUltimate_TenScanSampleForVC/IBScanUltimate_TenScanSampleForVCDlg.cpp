
// IBScanUltimate_TenScanSampleForVCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IBScanUltimate_TenScanSampleForVC.h"
#include "IBScanUltimate_TenScanSampleForVCDlg.h"

#include "IBScanUltimateApi.h"
#include "FingerDisplayManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Use critical section
CRITICAL_SECTION g_CriticalSection;

// CIBScanUltimate_TenScanSampleForVCDlg dialog
CIBScanUltimate_TenScanSampleForVCDlg::CIBScanUltimate_TenScanSampleForVCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIBScanUltimate_TenScanSampleForVCDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	m_pWnd = NULL;
	m_CurrentTabIdx = -1;
	m_nDevHandle = -1;
	m_nCurrentCaptureStep = -1;
	m_bInitializing = FALSE;
	m_bNeedClearPlaten = FALSE;

	InitializeCriticalSection(&g_CriticalSection);
}

CIBScanUltimate_TenScanSampleForVCDlg::~CIBScanUltimate_TenScanSampleForVCDlg()
{
	DeleteCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_TenScanSampleForVCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_VIEW, m_TabCtrl);
}

BEGIN_MESSAGE_MAP(CIBScanUltimate_TenScanSampleForVCDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	
	// user window messages
	ON_MESSAGE(WM_USER_CAPTURE_SEQ_START,			OnMsg_CaptureSeqStart)
	ON_MESSAGE(WM_USER_CAPTURE_SEQ_NEXT,			OnMsg_CaptureSeqNext)
	ON_MESSAGE(WM_USER_DEVICE_COMMUNICATION_BREAK,	OnMsg_DeviceCommunicationBreak)
	ON_MESSAGE(WM_USER_DRAW_CLIENT_WINDOW,			OnMsg_DrawClientWindow)
	ON_MESSAGE(WM_USER_UPDATE_DEVICE_LIST,			OnMsg_UpdateDeviceList)
	ON_MESSAGE(WM_USER_INIT_WARNING,				OnMsg_InitWarning)
	ON_MESSAGE(WM_USER_UPDATE_DISPLAY_RESOURCES,	OnMsg_UpdateDisplayResources)
	ON_MESSAGE(WM_USER_UPDATE_STATUS_MESSAGE,		OnMsg_UpdateStatusMessage)
	ON_MESSAGE(WM_USER_BEEP,						OnMsg_Beep)
	ON_MESSAGE(WM_USER_DRAW_FINGER_QUALITY,			OnMsg_DrawFingerQuality)
	ON_MESSAGE(WM_USER_ASK_RECAPTURE,				OnMsg_AskRecapture)

	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_VIEW, &CIBScanUltimate_TenScanSampleForVCDlg::OnTcnSelchangeTabView)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CIBScanUltimate_TenScanSampleForVCDlg message handlers

BOOL CIBScanUltimate_TenScanSampleForVCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	Init_Layout();

	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, OnEvent_DeviceCount, this );
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, OnEvent_InitProgress, this );
	PostMessage( WM_USER_UPDATE_DEVICE_LIST );

	IBSU_GetSDKVersion(&m_verInfo);
	CString titleName;
	titleName.Format("10 Prints Scan Sample for VC++");
	SetWindowText(titleName);
	memset(&m_FingerQuality[0], 0, sizeof(m_FingerQuality));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIBScanUltimate_TenScanSampleForVCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIBScanUltimate_TenScanSampleForVCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CIBScanUltimate_TenScanSampleForVCDlg::Init_Layout()
{
	int i;
	CString strlist_Eng[NUMBER_OF_TAB] = {"Main Tab", "Scan Tab"};

	WINDOWPLACEMENT place;
	GetWindowPlacement(&place);

	int WindowWidth = 875;
	int WindowHeight = 749;

	SetWindowPos(NULL, 0, 0, WindowWidth, WindowHeight, SWP_NOMOVE);
	CenterWindow();

	CButton *btn;
	btn = (CButton*)GetDlgItem(IDC_STATIC_LOGO);
	place.rcNormalPosition.left=0;
	place.rcNormalPosition.top=0;
	place.rcNormalPosition.right=place.rcNormalPosition.left+WindowWidth;
	place.rcNormalPosition.bottom=place.rcNormalPosition.top+56;
	btn->SetWindowPlacement(&place);

	place.rcNormalPosition.left=0;
	place.rcNormalPosition.top=56+20;
	place.rcNormalPosition.right=place.rcNormalPosition.left+WindowWidth;
	place.rcNormalPosition.bottom=place.rcNormalPosition.top+WindowHeight;
	m_TabCtrl.SetWindowPlacement(&place);

	for(i=0; i<NUMBER_OF_TAB; i++)
		m_TabCtrl.InsertItem(i, strlist_Eng[i]);

	m_MainDlg.Create(IDD_MAIN_DIALOG, &m_TabCtrl);
	m_MainDlg.SetWindowPos(NULL, 5, 0, WindowWidth-10, WindowHeight, SWP_NOZORDER);
	m_MainDlg.m_pParent = this;

	m_ScanFingerDlg.Create(IDD_SCAN_FINGER_DIALOG, &m_TabCtrl);
	m_ScanFingerDlg.SetWindowPos(NULL, 5, 0, WindowWidth-10, WindowHeight, SWP_NOZORDER);
	m_ScanFingerDlg.m_pParent = this;

	m_MainDlg.ShowWindow(SW_SHOW);
	m_pWnd = &m_MainDlg;
}

void CIBScanUltimate_TenScanSampleForVCDlg::OnTcnSelchangeTabView(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_CurrentTabIdx = m_TabCtrl.GetCurSel();
	
	ChangeView(m_CurrentTabIdx);

	*pResult = 0;
}

void CIBScanUltimate_TenScanSampleForVCDlg::ChangeView(int view)
{
	if(m_pWnd != NULL)
	{
		m_pWnd->ShowWindow(SW_HIDE);
		m_pWnd = NULL;
	}

	m_CurrentTabIdx = view;

	switch(m_CurrentTabIdx)
	{
	case 0:
		m_MainDlg.ShowWindow(SW_SHOW);
		m_pWnd = &m_MainDlg;
		break;
	case 1:
		m_ScanFingerDlg.ShowWindow(SW_SHOW);
		m_pWnd = &m_ScanFingerDlg;
		break;
	default:
		return;
	}
}

int CIBScanUltimate_TenScanSampleForVCDlg::_ReleaseDevice()
{
	int nRc = IBSU_STATUS_OK;
	
	if( m_nDevHandle != -1 )
		nRc = IBSU_CloseDevice( m_nDevHandle );

	if( nRc >= IBSU_STATUS_OK )
	{
		m_nDevHandle = -1;
		m_nCurrentCaptureStep = -1;
		m_bInitializing = false;
	}

	return nRc;
}  

void CIBScanUltimate_TenScanSampleForVCDlg::_BeepFail()
{
	Beep( 3500, 300 );
	Sleep(150);
	Beep( 3500, 150 );
	Sleep(150);
	Beep( 3500, 150 );
	Sleep(150);
	Beep( 3500, 150 );
}

void CIBScanUltimate_TenScanSampleForVCDlg::_BeepSuccess()
{
	Beep( 3500, 100 );
	Sleep(50);
	Beep( 3500, 100 );
}

void CIBScanUltimate_TenScanSampleForVCDlg::_BeepOk()
{
	Beep( 3500, 100 );
}

void CIBScanUltimate_TenScanSampleForVCDlg::_BeepDeviceCommunicationBreak()
{
	for( int i=0; i<8; i++ )
	{
		Beep( 3500, 100 );
		Sleep( 100 );
	}
}


void CIBScanUltimate_TenScanSampleForVCDlg::_SetImageMessage( LPCSTR Format, ... )
{
	TCHAR cMessage[IBSU_MAX_STR_LEN+1];
	va_list arg_ptr;

	va_start( arg_ptr, Format );
	_vsntprintf( cMessage, IBSU_MAX_STR_LEN, Format, arg_ptr );
	va_end( arg_ptr );

	int			font_size = 16;
	int			x = 10;
	int			y = 10;
	COLORREF	cr = RGB(0, 0, 255);

	IBSU_SetClientWindowOverlayText( m_nDevHandle, "Calibri", font_size, TRUE/*bold*/, cMessage, x, y, (DWORD)cr );
}

void CIBScanUltimate_TenScanSampleForVCDlg::_SetStatusBarMessage( LPCTSTR Format, ... )
{
	TCHAR cMessage[IBSU_MAX_STR_LEN+1];
	va_list arg_ptr;

	va_start( arg_ptr, Format );
	_vsntprintf( cMessage, IBSU_MAX_STR_LEN, Format, arg_ptr );
	va_end( arg_ptr );  

	CString strMessage;
	strMessage.Format(_T("%s"), cMessage);

	m_ScanFingerDlg.SetDlgItemText( IDC_TXT_STATUS, strMessage);
}

////////////////////////////////////////////////////////////////////////////////////////////
void CIBScanUltimate_TenScanSampleForVCDlg::OnEvent_DeviceCommunicationBreak(
						 const int deviceHandle,
						 void*     pContext
					   )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_TenScanSampleForVCDlg *pDlg = reinterpret_cast<CIBScanUltimate_TenScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->PostMessage( WM_USER_DEVICE_COMMUNICATION_BREAK );
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_TenScanSampleForVCDlg::OnEvent_PreviewImage(
						 const int deviceHandle,
						 void*     pContext,
						 const	   IBSU_ImageData image
					   )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_TenScanSampleForVCDlg *pDlg = reinterpret_cast<CIBScanUltimate_TenScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	memcpy(pDlg->m_MainDlg.m_ImgBuf[pDlg->m_MainDlg.m_CaptureFinger], image.Buffer, image.Width*image.Height);
	IBSU_GenerateZoomOutImage(image, pDlg->m_MainDlg.m_ImgBuf_S[pDlg->m_MainDlg.m_CaptureFinger], S_IMG_W, S_IMG_H, 128);

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_TenScanSampleForVCDlg::OnEvent_FingerCount(
						   const int                   deviceHandle,
						   void*                       pContext,
						   const IBSU_FingerCountState fingerCountState
						 )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_TenScanSampleForVCDlg *pDlg = reinterpret_cast<CIBScanUltimate_TenScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	if( deviceHandle != pDlg->m_nDevHandle )
		ASSERT( FALSE );

	char *fingerState;  
	if( fingerCountState == ENUM_IBSU_FINGER_COUNT_OK )
		fingerState = "FINGER_COUNT_OK";
	else if( fingerCountState == ENUM_IBSU_TOO_MANY_FINGERS )
		fingerState = "TOO_MANY_FINGERS";
	else if( fingerCountState == ENUM_IBSU_TOO_FEW_FINGERS )
		fingerState = "TOO_FEW_FINGERS";
	else if( fingerCountState == ENUM_IBSU_NON_FINGER )
		fingerState = "NON-FINGER";
	else
		fingerState = "UNKNOWN";

	pDlg->_SetStatusBarMessage( _T("-- Finger count changed --\tDevice= %d, State= %s"),  
							deviceHandle, fingerState );
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_TenScanSampleForVCDlg::OnEvent_FingerQuality(
                          const int                     deviceHandle,   
                          void                          *pContext,       
                          const IBSU_FingerQualityState *pQualityArray, 
                          const int                     qualityArrayCount    
						 )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_TenScanSampleForVCDlg *pDlg = reinterpret_cast<CIBScanUltimate_TenScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	memcpy(pDlg->m_FingerQuality, pQualityArray, sizeof(IBSU_FingerQualityState)*qualityArrayCount);
	pDlg->PostMessage( WM_USER_DRAW_FINGER_QUALITY );

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_TenScanSampleForVCDlg::OnEvent_DeviceCount(
						 const int detectedDevices,
						 void      *pContext
					   )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_TenScanSampleForVCDlg *pDlg = reinterpret_cast<CIBScanUltimate_TenScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->PostMessage( WM_USER_UPDATE_DEVICE_LIST );
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_TenScanSampleForVCDlg::OnEvent_InitProgress(
						 const int deviceIndex,
						 void      *pContext,
						 const int progressValue
					   )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_TenScanSampleForVCDlg *pDlg = reinterpret_cast<CIBScanUltimate_TenScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->_SetStatusBarMessage(_T("Initializing device... %d%%"), progressValue);
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_TenScanSampleForVCDlg::OnEvent_TakingAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_TenScanSampleForVCDlg *pDlg = reinterpret_cast<CIBScanUltimate_TenScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	CString m_strImageMessage;

	if( imageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		pDlg->PostMessage( WM_USER_BEEP, __BEEP_OK__ );
		m_strImageMessage.Format(_T("When done remove finger from sensor"));
		pDlg->_SetImageMessage(m_strImageMessage);
		pDlg->_SetStatusBarMessage(m_strImageMessage);
	}

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_TenScanSampleForVCDlg::OnEvent_CompleteAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_TenScanSampleForVCDlg *pDlg = reinterpret_cast<CIBScanUltimate_TenScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	if( imageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
		pDlg->PostMessage( WM_USER_BEEP, __BEEP_OK__ );

	LeaveCriticalSection(&g_CriticalSection);
}

/****
 ** This IBSU_CallbackResultImage callback was deprecated since 1.7.0
 ** Please use IBSU_CallbackResultImageEx instead
 */
void CIBScanUltimate_TenScanSampleForVCDlg::OnEvent_ResultImageEx(
                          const int                   deviceHandle,
                          void                        *pContext,
                          const int                   imageStatus,
                          const IBSU_ImageData        image,
						  const IBSU_ImageType        imageType,
						  const int                   detectedFingerCount,
                          const int                   segmentImageArrayCount,
                          const IBSU_ImageData        *pSegmentImageArray,
						  const IBSU_SegmentPosition  *pSegmentPositionArray
                        )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_TenScanSampleForVCDlg *pDlg = reinterpret_cast<CIBScanUltimate_TenScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	if( deviceHandle != pDlg->m_nDevHandle )
		ASSERT( FALSE );

    if( imageStatus >= IBSU_STATUS_OK )
	{
        pDlg->PostMessage( WM_USER_BEEP, __BEEP_SUCCESS__ );
	}
	else
	{
        pDlg->PostMessage( WM_USER_BEEP, __BEEP_FAIL__ );
	}

	// added 2012-11-30
	if( pDlg->m_bNeedClearPlaten )
	{
		pDlg->m_bNeedClearPlaten = FALSE;
		pDlg->PostMessage( WM_USER_DRAW_FINGER_QUALITY );
	}

    char imgTypeName[IBSU_MAX_STR_LEN]={0};
    switch( imageType )
    {
    case ENUM_IBSU_ROLL_SINGLE_FINGER:
	    strcpy(imgTypeName, "-- Rolling single finger --"); break;
    case ENUM_IBSU_FLAT_SINGLE_FINGER:
	    strcpy(imgTypeName, "-- Flat single finger --"); break;
    case ENUM_IBSU_FLAT_TWO_FINGERS:
	    strcpy(imgTypeName, "-- Flat two fingers --"); break;
    case ENUM_IBSU_FLAT_FOUR_FINGERS:
	    strcpy(imgTypeName, "-- Flat 4 fingers --"); break;
    default:
	    strcpy(imgTypeName, "-- Unknown --"); break;
    }

	int nfiq_score[IBSU_MAX_SEGMENT_COUNT];

	char propertyValue[32];
	IBSU_GetClientWindowProperty(deviceHandle, ENUM_IBSU_WINDOW_PROPERTY_SCALE_FACTOR, propertyValue);
	double scaleFactor = atof(propertyValue);
	IBSU_GetClientWindowProperty(deviceHandle, ENUM_IBSU_WINDOW_PROPERTY_LEFT_MARGIN, propertyValue);
	int leftMargin = atoi(propertyValue);
	IBSU_GetClientWindowProperty(deviceHandle, ENUM_IBSU_WINDOW_PROPERTY_TOP_MARGIN, propertyValue);
	int topMargin = atoi(propertyValue);
	for( int i=0; i<segmentImageArrayCount; i++ )
	{
		COLORREF cr = RGB(0, 128, 0);
		int x1, x2, x3, x4, y1, y2, y3, y4;
		x1 = leftMargin + (int)((pSegmentPositionArray+i)->x1*scaleFactor);
		x2 = leftMargin + (int)((pSegmentPositionArray+i)->x2*scaleFactor);
		x3 = leftMargin + (int)((pSegmentPositionArray+i)->x3*scaleFactor);
		x4 = leftMargin + (int)((pSegmentPositionArray+i)->x4*scaleFactor);
		y1 = topMargin +  (int)((pSegmentPositionArray+i)->y1*scaleFactor);
		y2 = topMargin +  (int)((pSegmentPositionArray+i)->y2*scaleFactor);
		y3 = topMargin +  (int)((pSegmentPositionArray+i)->y3*scaleFactor);
		y4 = topMargin +  (int)((pSegmentPositionArray+i)->y4*scaleFactor);

		IBSU_ModifyOverlayQuadrangle(deviceHandle, pDlg->m_nOvSegmentHandle[i],
			x1, y1, x2, y2, x3, y3, x4, y4, 1, (DWORD)cr);
	}


	CString m_strImageMessage;
	if( imageStatus >= IBSU_STATUS_OK )
	{
		int	i, score=0, nRc, segment_pos=0;
		memset(&nfiq_score, 0, sizeof(nfiq_score));
		for( i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
		{
			if( pDlg->m_FingerQuality[i] == ENUM_IBSU_FINGER_NOT_PRESENT )
				continue;

			nRc = IBSU_GetNFIQScore(deviceHandle, (const BYTE*)(pSegmentImageArray+segment_pos)->Buffer,
				(pSegmentImageArray+segment_pos)->Width, (pSegmentImageArray+segment_pos)->Height,
				(pSegmentImageArray+segment_pos)->BitsPerPixel, &score);
			if( nRc == IBSU_STATUS_OK )
			{
				nfiq_score[segment_pos] = score;
				segment_pos++;
			}
		}

		if( pDlg->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE ||
			pDlg->m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE )
		{
			memcpy(pDlg->m_MainDlg.m_ImgBuf[pDlg->m_MainDlg.m_CaptureFinger], image.Buffer, image.Width*image.Height);
			IBSU_GenerateZoomOutImage(pSegmentImageArray[0],  pDlg->m_MainDlg.m_ImgBuf_S[pDlg->m_MainDlg.m_CaptureFinger], S_IMG_W, S_IMG_H, 255);
			pDlg->m_MainDlg.m_NFIQBuf[pDlg->m_MainDlg.m_CaptureFinger] = nfiq_score[0];
		}
		else if( pDlg->m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE )
		{
			if( pDlg->m_MainDlg.m_Run_AutoSplit == TRUE )
			{
				int index = (pDlg->m_MainDlg.m_CaptureFinger-LEFT_RING_LITTLE)*2;

				if(imageType == ENUM_IBSU_FLAT_SINGLE_FINGER)
				{
					if(pDlg->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(index) == TRUE)
					{
						memcpy(pDlg->m_MainDlg.m_ImgBuf[index], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
						pDlg->m_MainDlg.m_nCurrentWidthForSplit[index] = pSegmentImageArray[0].Width;
						pDlg->m_MainDlg.m_nCurrentHeightForSplit[index] = pSegmentImageArray[0].Height;
						IBSU_GenerateZoomOutImage(pSegmentImageArray[0], pDlg->m_MainDlg.m_ImgBuf_S[index], S_IMG_W, S_IMG_H, 255);
						pDlg->m_MainDlg.m_NFIQBuf[index] = nfiq_score[0];
					}
					else if(pDlg->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(index+1) == TRUE)
					{
						memcpy(pDlg->m_MainDlg.m_ImgBuf[index+1], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
						pDlg->m_MainDlg.m_nCurrentWidthForSplit[index+1] = pSegmentImageArray[0].Width;
						pDlg->m_MainDlg.m_nCurrentHeightForSplit[index+1] = pSegmentImageArray[0].Height;
						IBSU_GenerateZoomOutImage(pSegmentImageArray[0], pDlg->m_MainDlg.m_ImgBuf_S[index+1], S_IMG_W, S_IMG_H, 255);
						pDlg->m_MainDlg.m_NFIQBuf[index+1] = nfiq_score[0];
					}
				}
				else
				{
					memcpy(pDlg->m_MainDlg.m_ImgBuf[index], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
					pDlg->m_MainDlg.m_nCurrentWidthForSplit[index] = pSegmentImageArray[0].Width;
					pDlg->m_MainDlg.m_nCurrentHeightForSplit[index] = pSegmentImageArray[0].Height;
					IBSU_GenerateZoomOutImage(pSegmentImageArray[0], pDlg->m_MainDlg.m_ImgBuf_S[index], S_IMG_W, S_IMG_H, 255);
					pDlg->m_MainDlg.m_NFIQBuf[index] = nfiq_score[0];

					memcpy(pDlg->m_MainDlg.m_ImgBuf[index+1], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
					pDlg->m_MainDlg.m_nCurrentWidthForSplit[index+1] = pSegmentImageArray[1].Width;
					pDlg->m_MainDlg.m_nCurrentHeightForSplit[index+1] = pSegmentImageArray[1].Height;
					IBSU_GenerateZoomOutImage(pSegmentImageArray[1], pDlg->m_MainDlg.m_ImgBuf_S[index+1], S_IMG_W, S_IMG_H, 255);
					pDlg->m_MainDlg.m_NFIQBuf[index+1] = nfiq_score[1];
				}
			}
			else
			{
				memcpy(pDlg->m_MainDlg.m_ImgBuf[pDlg->m_MainDlg.m_CaptureFinger], image.Buffer, image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image, pDlg->m_MainDlg.m_ImgBuf_S[pDlg->m_MainDlg.m_CaptureFinger], S_IMG_W, S_IMG_H, 255);
				pDlg->m_MainDlg.m_NFIQBuf[pDlg->m_MainDlg.m_CaptureFinger] = nfiq_score[0];
			}
		}

		if( imageStatus == IBSU_STATUS_OK )
		{
			m_strImageMessage.Format(_T("%s acquisition completed successfully"), imgTypeName);
	        pDlg->_SetImageMessage(m_strImageMessage);
	        pDlg->_SetStatusBarMessage(m_strImageMessage);
		}
		else
		{
			// > IBSU_STATUS_OK
			m_strImageMessage.Format(_T("%s Warning(%s)"), imgTypeName, pDlg->_GetWarningMessageShort(imageStatus));
			pDlg->_SetImageMessage(m_strImageMessage);
			pDlg->_SetStatusBarMessage(m_strImageMessage);

			pDlg->PostMessage( WM_USER_ASK_RECAPTURE );
			pDlg->m_ScanFingerDlg.m_FingerDisplayManager->SetFingerCompleted(pDlg->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
			pDlg->m_MainDlg.m_CaptureCompleted = TRUE;

			pDlg->m_MainDlg.m_nCurrWidth = image.Width;
			pDlg->m_MainDlg.m_nCurrHeight = image.Height;

			LeaveCriticalSection(&g_CriticalSection);
			return;
		}
	}
	else
	{
		// < IBSU_STATUS_OK
	    m_strImageMessage.Format(_T("%s acquisition failed (Error code = %d)"), imgTypeName, imageStatus);
	    pDlg->_SetImageMessage(m_strImageMessage);
	    pDlg->_SetStatusBarMessage(m_strImageMessage);

        // Stop all of acquisition
        pDlg->m_nCurrentCaptureStep = (int)pDlg->m_vecCaptureSeq.size();
	}

    pDlg->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );

	pDlg->m_ScanFingerDlg.m_FingerDisplayManager->SetFingerCompleted(pDlg->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
	pDlg->m_MainDlg.m_CaptureCompleted = TRUE;

	pDlg->m_MainDlg.m_nCurrWidth = image.Width;
	pDlg->m_MainDlg.m_nCurrHeight = image.Height;

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_TenScanSampleForVCDlg::OnEvent_ClearPlatenAtCapture(
                          const int                   deviceHandle,
                          void                        *pContext,
                          const IBSU_PlatenState      platenState
                        )
{
	if( pContext == NULL )
		return;

	CString m_strImageMessage;

	CIBScanUltimate_TenScanSampleForVCDlg *pDlg = reinterpret_cast<CIBScanUltimate_TenScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	if( platenState == ENUM_IBSU_PLATEN_HAS_FINGERS )
		pDlg->m_bNeedClearPlaten = TRUE;
	else
		pDlg->m_bNeedClearPlaten = FALSE;

	if( pDlg->m_bNeedClearPlaten )
	{
		
		m_strImageMessage.Format(_T("Please remove your fingers on the platen first!"));
		pDlg->_SetImageMessage(m_strImageMessage);
		pDlg->_SetStatusBarMessage(m_strImageMessage);
	}
	else
	{
		CaptureInfo info = pDlg->m_vecCaptureSeq[pDlg->m_nCurrentCaptureStep];

		// Display message for image acuisition again
		CString strMessage;
		strMessage = info.PreCaptureMessage;

		pDlg->_SetStatusBarMessage(_T("%s"), strMessage);
		pDlg->_SetImageMessage(_T("%s"), strMessage);
		m_strImageMessage = strMessage;
	}

	pDlg->PostMessage( WM_USER_DRAW_FINGER_QUALITY );

	LeaveCriticalSection(&g_CriticalSection);
}
////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CIBScanUltimate_TenScanSampleForVCDlg::OnMsg_CaptureSeqStart(WPARAM wParam, LPARAM lParam)
{
	if( m_nDevHandle == -1 )
	{
		OnMsg_UpdateDisplayResources();
		return 0L;
	}
	m_vecCaptureSeq.clear();
	CaptureInfo info;

	if( m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE )
	{
		info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
		info.NumberOfFinger = 1;
		info.PostCaptuerMessage = _T("Keep fingers on the sensor!");
		
		if( m_MainDlg.m_CaptureFinger == NONE_FINGER )
		{
			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SFF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SFF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SFF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SFF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SFF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SFF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SFF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SFF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SFF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				m_vecCaptureSeq.push_back(info);
			}
		}
		else
		{
			if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SFF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SFF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SFF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SFF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SFF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SFF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SFF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SFF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SFF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
			}
			m_vecCaptureSeq.push_back(info);
		}
	}
	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE )
	{
		info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
		info.PostCaptuerMessage = _T("Roll finger!");
		info.NumberOfFinger = 1;
		
		if( m_MainDlg.m_CaptureFinger == NONE_FINGER )
		{
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				m_vecCaptureSeq.push_back(info);
			}
		}
		else
		{
			if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
			}
			m_vecCaptureSeq.push_back(info);
		}
	}
	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE )
	{
		info.PostCaptuerMessage = _T("Keep finger on the sensor!");
		
		if( m_MainDlg.m_CaptureFinger == NONE_FINGER )
		{
			// RIGHT_INDEX_MIDDLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index and middle on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			// RIGHT_RING_LITTLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring and little on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			// LEFT_INDEX_MIDDLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index and middle on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			// LEFT_RING_LITTLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring and little on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			// BOTH_THUMBS
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put both thumbs on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
		}
		else
		{
			if( m_MainDlg.m_CaptureFinger == RIGHT_INDEX_MIDDLE )
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right index and middle on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right index on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right middle on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_RING_LITTLE )
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right ring and little on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right ring on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right little on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_INDEX_MIDDLE )
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left index and middle on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left index on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left middle on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_RING_LITTLE )
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left ring and little on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left ring on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left little on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if( m_MainDlg.m_CaptureFinger == BOTH_THUMBS )
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put both thumbs on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
		}
	}

	PostMessage( WM_USER_CAPTURE_SEQ_NEXT );

	return 0L;
}

LRESULT CIBScanUltimate_TenScanSampleForVCDlg::OnMsg_CaptureSeqNext(WPARAM wParam, LPARAM lParam)
{
	int nRc;
	if( m_nDevHandle == -1 )
		return 0L;

	m_bBlank = FALSE;
	memset(&m_FingerQuality[0], 0, sizeof(m_FingerQuality));

	m_nCurrentCaptureStep++;
	if( m_nCurrentCaptureStep >= (int)m_vecCaptureSeq.size() )
	{
		// All of capture sequence completely
		m_nCurrentCaptureStep = -1;

		if( m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE || 
			m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE )
		{
			m_ScanFingerDlg.m_FingerDisplayManager->SelectFinger(NONE_FINGER);
		}
		else if( m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE )
		{
			m_ScanFingerDlg.m_FingerDisplayManager->SelectFinger(NONE_FINGER);
		}
		
		OnMsg_UpdateDisplayResources();
		return 0L;
	}

	//For REsultImageEx
	for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
		IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);
	//

	// Make capture delay for display result image on multi capture mode (500 ms)
	if( m_nCurrentCaptureStep > 0 )
	{
		Sleep(500);
	}

	CaptureInfo info = m_vecCaptureSeq[m_nCurrentCaptureStep];

	IBSU_ImageResolution imgRes = ENUM_IBSU_IMAGE_RESOLUTION_500;
	BOOL bAvailable = FALSE;
	nRc = IBSU_IsCaptureAvailable(m_nDevHandle, info.ImageType, imgRes, &bAvailable);
	if( nRc != IBSU_STATUS_OK || !bAvailable )
	{
		m_nCurrentCaptureStep = -1;
		return 0L;
	}

	// Start capture
	DWORD captureOptions = 0;
	captureOptions |= IBSU_OPTION_AUTO_CONTRAST;
	captureOptions |= IBSU_OPTION_AUTO_CAPTURE;

	m_MainDlg.m_CaptureFinger = info.capture_finger_idx;

	if( m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE || 
		m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE )
	{
		m_ScanFingerDlg.m_FingerDisplayManager->SelectFinger(m_MainDlg.m_CaptureFinger); 
	}
	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE )
	{
		m_ScanFingerDlg.m_FingerDisplayManager->SelectFinger(m_MainDlg.m_CaptureFinger);
	}

	nRc = IBSU_BeginCaptureImage(m_nDevHandle, info.ImageType, imgRes, captureOptions);

	return 0L;
}

LRESULT CIBScanUltimate_TenScanSampleForVCDlg::OnMsg_DeviceCommunicationBreak(WPARAM wParam, LPARAM lParam)
{
	int nRc;

	if( m_nDevHandle == -1 )
		return 0L;

	nRc = _ReleaseDevice();
	if( nRc == IBSU_ERR_RESOURCE_LOCKED )
	{
		// retry to release device
		PostMessage( WM_USER_DEVICE_COMMUNICATION_BREAK );
	}
	else
	{
		PostMessage( WM_USER_BEEP, __BEEP_DEVICE_COMMUNICATION_BREAK__ );
		PostMessage( WM_USER_UPDATE_DEVICE_LIST );
	}

	return 0L;
}

LRESULT CIBScanUltimate_TenScanSampleForVCDlg::OnMsg_DrawClientWindow(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CIBScanUltimate_TenScanSampleForVCDlg::OnMsg_UpdateDeviceList(WPARAM wParam, LPARAM lParam)
{
	const bool idle = !m_bInitializing && ( m_nCurrentCaptureStep == -1 );

	// store currently selected device
	CString strSelectedText;
	int selectedDev = m_MainDlg.m_DeviceList.GetCurSel();
	if( selectedDev > -1 )
	m_MainDlg.m_DeviceList.GetLBText( selectedDev, strSelectedText );

	m_MainDlg.m_DeviceList.ResetContent();
	m_MainDlg.m_DeviceList.AddString( _T( "- Please select -" ) );

	// populate combo box
	int devices = 0;
	IBSU_GetDeviceCount( &devices );

	selectedDev = 0;
	for( int i = 0; i < devices; i++ )
	{
		IBSU_DeviceDesc devDesc;
		if( IBSU_GetDeviceDescription( i, &devDesc ) < IBSU_STATUS_OK )
			continue;

		CString strDevice;
		if( devDesc.productName[0] == 0 )
			strDevice = _T( "unknown device" );
		else
			strDevice.Format( _T( "%s_v%s (%s)" ), devDesc.productName, devDesc.fwVersion, devDesc.serialNumber );

		m_MainDlg.m_DeviceList.AddString( strDevice );
		if( strDevice == strSelectedText )
			selectedDev = i + 1;
	}

	if( ( selectedDev == 0 ) && ( m_MainDlg.m_DeviceList.GetCount() == 2 ) )
		selectedDev = 1;

	m_MainDlg.m_DeviceList.SetCurSel( selectedDev );

	if( idle )
	{
		m_MainDlg.OnCbnSelchangeDeviceList();
		m_MainDlg._UpdateCaptureSequences();
	}

	return 0L;
}

LRESULT CIBScanUltimate_TenScanSampleForVCDlg::OnMsg_InitWarning(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CIBScanUltimate_TenScanSampleForVCDlg::OnMsg_UpdateDisplayResources(WPARAM wParam, LPARAM lParam)
{
	BOOL selectedDev = m_MainDlg.m_DeviceList.GetCurSel() > 0;
	BOOL captureSeq = m_MainDlg.m_SequenceList.GetCurSel() > 0;
	BOOL idle = !m_bInitializing && ( m_nCurrentCaptureStep == -1 );
	BOOL active = !m_bInitializing && (m_nCurrentCaptureStep != -1 );
	BOOL uninitializedDev = selectedDev && ( m_nDevHandle == -1 );

	m_MainDlg.m_DeviceList.EnableWindow( idle );  
	m_MainDlg.m_SequenceList.EnableWindow( selectedDev && idle );

	m_MainDlg.GetDlgItem( IDC_START_CAPTURE )->EnableWindow( captureSeq );
	m_MainDlg.GetDlgItem( IDC_EDIT_CAPTURE )->EnableWindow( captureSeq && m_MainDlg.m_CaptureCompleted );
	m_MainDlg.GetDlgItem( IDC_SAVE_CAPTURE )->EnableWindow( captureSeq && m_MainDlg.m_CaptureCompleted);
	m_MainDlg.GetDlgItem( IDC_USE_AUTO_SPLIT )->EnableWindow( m_MainDlg.m_SequenceList.GetCurSel() == 3);

	return 0L;
}

LRESULT CIBScanUltimate_TenScanSampleForVCDlg::OnMsg_UpdateStatusMessage(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CIBScanUltimate_TenScanSampleForVCDlg::OnMsg_Beep(WPARAM wParam, LPARAM lParam)
{
	int beep = (int)wParam;

	if( beep == __BEEP_FAIL__ )
		_BeepFail();
	else if( beep == __BEEP_SUCCESS__ )
		_BeepSuccess();
	else if( beep == __BEEP_OK__ )
		_BeepOk();
	else if( beep == __BEEP_DEVICE_COMMUNICATION_BREAK__ )
		_BeepDeviceCommunicationBreak();

	return 0L;
}

LRESULT CIBScanUltimate_TenScanSampleForVCDlg::OnMsg_DrawFingerQuality(WPARAM wParam, LPARAM lParam)
{
	for( int i = 0; i < 4; i++ )
	{
		m_ScanFingerDlg.GetDlgItem(IDC_STATIC_QUALITY_1+i)->RedrawWindow();
	}

	return 0L;
}

LRESULT CIBScanUltimate_TenScanSampleForVCDlg::OnMsg_AskRecapture(WPARAM wParam, LPARAM lParam)
{
	int imageStatus = (int)wParam;
    CString askMsg;

	askMsg.Format("%s\nDo you want a recapture?", _GetWarningMessage(imageStatus));
	if( AfxMessageBox(askMsg, MB_YESNO | MB_ICONINFORMATION) == IDYES )
	{
		// To recapture current finger position
		m_nCurrentCaptureStep--;
	}

    PostMessage( WM_USER_CAPTURE_SEQ_NEXT );

	return 0L;
}

void CIBScanUltimate_TenScanSampleForVCDlg::_CaptureStop()
{
	if( m_nDevHandle == -1 )
		return;

	IBSU_CancelCaptureImage( m_nDevHandle );
	m_nCurrentCaptureStep = -1;
	m_bNeedClearPlaten = FALSE;
	m_bBlank = FALSE;

	OnMsg_UpdateDisplayResources();
}

void CIBScanUltimate_TenScanSampleForVCDlg::OnClose()
{
	int nRc;
	for( int i=0; i<10; i++ )
	{
		nRc = _ReleaseDevice();
		if( nRc != IBSU_ERR_RESOURCE_LOCKED )
			break;
		Sleep(100);
	}

	CDialog::OnClose();
}

char* CIBScanUltimate_TenScanSampleForVCDlg::_GetWarningMessage(int warning_msg)
{
	switch(warning_msg)
	{
		case IBSU_WRN_CHANNEL_IO_FRAME_MISSING:
			return "Channel io frame missing";
		case IBSU_WRN_CHANNEL_IO_CAMERA_WRONG:
			return "Channel io camera wrong";
		case IBSU_WRN_CHANNEL_IO_SLEEP_STATUS:
			return "Channel io sleep status";
		case IBSU_WRN_OUTDATED_FIRMWARE:
			return "Outdated firmware";
		case IBSU_WRN_ALREADY_INITIALIZED:
			return "Already initialized";
		case IBSU_WRN_API_DEPRECATED:
			return "API deprecated";
		case IBSU_WRN_BGET_IMAGE:
			return "Bget image";
		case IBSU_WRN_ROLLING_NOT_RUNNING:
			return "Rolling not running";
		case IBSU_WRN_NO_FINGER:
			return "No finger";
		case IBSU_WRN_INCORRECT_FINGERS:
			return "Incorrect fingers";
		case IBSU_WRN_ROLLING_SMEAR:
			return "Smear detected";
		case IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY:
			return "Smear detected horizontally";
		case IBSU_WRN_ROLLING_SHIFTED_VERTICALLY:
			return "Smear detected (vertically)";
		case IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY|IBSU_WRN_ROLLING_SHIFTED_VERTICALLY:
			return "Smear detected (horizontally & vertically)";
		case IBSU_WRN_EMPTY_IBSM_RESULT_IMAGE:
			return "Empty result image";
	}

	return "";
}

char* CIBScanUltimate_TenScanSampleForVCDlg::_GetWarningMessageShort(int warning_msg)
{
	switch(warning_msg)
	{
		case IBSU_WRN_CHANNEL_IO_FRAME_MISSING:
			return "frame missing";
		case IBSU_WRN_CHANNEL_IO_CAMERA_WRONG:
			return "camera wrong";
		case IBSU_WRN_CHANNEL_IO_SLEEP_STATUS:
			return "sleep status";
		case IBSU_WRN_OUTDATED_FIRMWARE:
			return "Outdated f/w";
		case IBSU_WRN_ALREADY_INITIALIZED:
			return "Already init";
		case IBSU_WRN_API_DEPRECATED:
			return "API deprecated";
		case IBSU_WRN_BGET_IMAGE:
			return "Bget image";
		case IBSU_WRN_ROLLING_NOT_RUNNING:
			return "Not running";
		case IBSU_WRN_NO_FINGER:
			return "No finger";
		case IBSU_WRN_INCORRECT_FINGERS:
			return "Incorrect fingers";
		case IBSU_WRN_ROLLING_SMEAR:
		case IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY:
		case IBSU_WRN_ROLLING_SHIFTED_VERTICALLY:
		case IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY|IBSU_WRN_ROLLING_SHIFTED_VERTICALLY:
			return "Smear detected";
		case IBSU_WRN_EMPTY_IBSM_RESULT_IMAGE:
			return "Empty result image";
	}

	return "";
}