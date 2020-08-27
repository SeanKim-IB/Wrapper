
// IBScanUltimate_SimpleScanWMDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IBSU_SimpleScanWM.h"
#include "IBSU_SimpleScanWMDlg.h"
#include "IBSU_SimpleScanWMSplashDlg.h"

#include "IBScanUltimateApi.h"
//#include "shlobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_STATUS_FINGER_QUALITY			1
#define TIMER_STATUS_DELAY					500

// Capture sequence definitions
const TCHAR CAPTURE_SEQ_FLAT_SINGLE_FINGER[]		= _T("Single flat finger");
const TCHAR CAPTURE_SEQ_ROLL_SINGLE_FINGER[]		= _T("Single rolled finger");
const TCHAR CAPTURE_SEQ_2_FLAT_FINGERS[]			= _T("2 flat fingers");
const TCHAR CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS[]	= _T("10 single flat fingers");
const TCHAR CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS[]	= _T("10 single rolled fingers");
const TCHAR CAPTURE_SEQ_4_FLAT_FINGERS[]			= _T("4 flat fingers");
const TCHAR CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER[]	= _T("10 flat fingers with 4-finger scanner");


// Use user window messages
#define WM_USER_CAPTURE_SEQ_START				WM_APP + 1
#define WM_USER_CAPTURE_SEQ_NEXT				WM_APP + 2
#define WM_USER_DEVICE_COMMUNICATION_BREAK		WM_APP + 3
#define WM_USER_DRAW_CLIENT_WINDOW				WM_APP + 4
#define WM_USER_UPDATE_DEVICE_LIST				WM_APP + 5
#define WM_USER_INIT_WARNING					WM_APP + 6
#define WM_USER_UPDATE_DISPLAY_RESOURCES		WM_APP + 7
#define WM_USER_UPDATE_STATUS_MESSAGE			WM_APP + 8
#define WM_USER_BEEP							WM_APP + 9
#define WM_USER_DRAW_FINGER_QUALITY				WM_APP + 10
#define WM_USER_ASYNC_OPEN_DEVICE				WM_APP + 11


// Beep definitions
const int __BEEP_FAIL__							= 0;
const int __BEEP_SUCCESS__						= 1;
const int __BEEP_OK__							= 2;
const int __BEEP_DEVICE_COMMUNICATION_BREAK__	= 3;

const CBrush   CIBScanUltimate_SimpleScanWMDlg::s_brushRed( RGB(255,0,0) );
const CBrush   CIBScanUltimate_SimpleScanWMDlg::s_brushOrange( RGB(255,128,0) );
const CBrush   CIBScanUltimate_SimpleScanWMDlg::s_brushGreen( RGB(0,128,0) );
const CBrush   CIBScanUltimate_SimpleScanWMDlg::s_brushPlaten( RGB(255,255,255) );


// Use critical section
CRITICAL_SECTION g_CriticalSection;


CIBScanUltimate_SimpleScanWMDlg::CIBScanUltimate_SimpleScanWMDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIBScanUltimate_SimpleScanWMDlg::IDD, pParent)
	, m_chkSaveImages(TRUE)
	, m_chkAsyncOpenDevice(FALSE)
	, m_chkOpenDeviceEx(TRUE)
	, m_chkUseNFIQ(FALSE)
	, m_chkDrawSegmentImage(TRUE)
	, m_chkUseClearPlaten(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_nSelectedDevIndex = -1;

	for(int i=0; i<4; i++)
		m_FingerQuality[i] = ENUM_IBSU_FINGER_NOT_PRESENT;

	InitializeCriticalSection(&g_CriticalSection);
}

CIBScanUltimate_SimpleScanWMDlg::~CIBScanUltimate_SimpleScanWMDlg()
{
	DeleteCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SimpleScanWMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DEVICES, m_cboUsbDevices);
	DDX_Control(pDX, IDC_FRAME_IMAGE, m_frameImage);
}

BEGIN_MESSAGE_MAP(CIBScanUltimate_SimpleScanWMDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

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
	ON_MESSAGE(WM_USER_ASYNC_OPEN_DEVICE,			OnMsg_AsyncOpenDevice)

	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_CAPTURE_START, &CIBScanUltimate_SimpleScanWMDlg::OnBnClickedBtnCaptureStart)
	ON_BN_CLICKED(IDC_BTN_CAPTURE_STOP, &CIBScanUltimate_SimpleScanWMDlg::OnBnClickedBtnCaptureStop)
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICES, &CIBScanUltimate_SimpleScanWMDlg::OnCbnSelchangeComboDevices)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CIBScanUltimate_SimpleScanWMDlg message handlers

BOOL CIBScanUltimate_SimpleScanWMDlg::OnInitDialog()
{
	RECT	rc;
	RECT	rect;
	RECT rectWorkArea;
	RECT rectTaskBar;
	CWnd *TaskBar;
	CWnd *MenuBar;
	RECT rectMenu;

	GetWindowRect( &rect );
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWorkArea, 0);



	CDialog::OnInitDialog();

	GetWindowRect( &rect );
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWorkArea, 0);


// Find task bar
	TaskBar = FindWindowW(TEXT("HHTaskBar"), TEXT("") );
	TaskBar->GetWindowRect( &rectTaskBar );

// Find size of Menubar
	HWND hMenu = SHFindMenuBar( m_hWnd );
    MenuBar = new CWnd();
	MenuBar->Attach(hMenu);
	MenuBar->GetWindowRect(&rectMenu);

// Move window to use the available space
	SetRect(&rc, 0, rectTaskBar.bottom+1, GetSystemMetrics(SM_CXSCREEN), rectMenu.top-1 );
	MoveWindow(&rc, TRUE);

// Set the system working area. Since WINCE isn't for some reason.
	SystemParametersInfo(SPI_SETWORKAREA, 0, &rc, SPIF_SENDCHANGE);	


	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_nDevHandle = -1;
	m_nCurrentCaptureStep = -1;
	m_bInitializing = FALSE;
	m_strImageMessage = _T( "" );
	m_bNeedClearPlaten = FALSE;

	m_ImgSaveFolder[0] = 0;
	SHGetSpecialFolderPath( NULL, m_ImgSaveFolder, CSIDL_MYPICTURES, TRUE );

	IBSU_GetSDKVersion(&m_verInfo);
	CString titleName;
	titleName.Format(TEXT("SimpleScan WM - %s"), m_verInfo.Product);
	SetWindowText(titleName);

	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, OnEvent_DeviceCount, this );
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, OnEvent_InitProgress, this );
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE, OnEvent_AsyncOpenDevice, this );


	this->GetWindowRect(&m_defaultWndPlace.rcNormalPosition);
	CWnd *disWnd = GetDlgItem( IDC_FRAME_IMAGE );
	disWnd->GetWindowRect(&m_defaultViewPlace.rcNormalPosition);
	disWnd = GetDlgItem( IDC_TXT_STATUS );
	disWnd->GetWindowRect(&m_defaultStatusPlace.rcNormalPosition);

	// Display the splash screen
	CIBScanUltimate_SimpleScanWMSplashDlg SplashDlg;
	SplashDlg.DoModal();

	SHGetSpecialFolderPath( NULL, m_UMFolder, CSIDL_MYPICTURES, TRUE );

	SetTimer( TIMER_STATUS_FINGER_QUALITY, TIMER_STATUS_DELAY, NULL );
	PostMessage( WM_USER_UPDATE_DEVICE_LIST );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIBScanUltimate_SimpleScanWMDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

//      This message isn't in WINCE
//		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
HCURSOR CIBScanUltimate_SimpleScanWMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



////////////////////////////////////////////////////////////////////////////////////////////
// User defined functions

UINT CIBScanUltimate_SimpleScanWMDlg::_InitializeDeviceThreadCallback( LPVOID pParam )
{   
	if( pParam == NULL )
		return 1;

	CIBScanUltimate_SimpleScanWMDlg*	pDlg = (CIBScanUltimate_SimpleScanWMDlg*)pParam;
	const int		devIndex = pDlg->m_cboUsbDevices.GetCurSel() - 1;
	int				devHandle;
	int				nRc = IBSU_STATUS_OK;


	pDlg->m_bInitializing = true;

	if( pDlg->m_chkOpenDeviceEx )
	{
		nRc = IBSU_OpenDeviceEx( devIndex, pDlg->m_UMFolder, FALSE, &devHandle );
	}
	else
	{
		nRc = IBSU_OpenDevice( devIndex, &devHandle );
	}

	pDlg->m_bInitializing = false;

	return pDlg->_ExecuteInitializeDevice(nRc, devHandle);
}

void CIBScanUltimate_SimpleScanWMDlg::_SetStatusBarMessage( LPCTSTR Format, ... )
{
	TCHAR cMessage[IBSU_MAX_STR_LEN+1];
	va_list arg_ptr;

	va_start( arg_ptr, Format );
	_vsntprintf( cMessage, IBSU_MAX_STR_LEN, Format, arg_ptr );
	va_end( arg_ptr );  

	CString strMessage;
	strMessage.Format(_T("%s"), cMessage);
	SetDlgItemText( IDC_TXT_STATUS, strMessage);
}

void CIBScanUltimate_SimpleScanWMDlg::_SetImageMessage( LPCTSTR Format, ... )
{
	TCHAR cMessage[IBSU_MAX_STR_LEN+1];
	va_list arg_ptr;

	va_start( arg_ptr, Format );
	_vsntprintf( cMessage, IBSU_MAX_STR_LEN, Format, arg_ptr );
	va_end( arg_ptr );

	int			font_size = 8;
	int			x = 10;
	int			y = 10;
	COLORREF	cr = RGB(0, 0, 255);

    if( m_bNeedClearPlaten )
        cr = RGB(255, 0, 0);

    // IBSU_SetClientWindowOverlayText was deprecated since 1.7.0
    // Please use the function IBSU_AddOverlayText and IBSU_ModifyOverlayText instead
	IBSU_ModifyOverlayText( m_nDevHandle, m_nOvImageTextHandle, TEXT("Arial"), font_size, TRUE/*bold*/, cMessage, x, y, (DWORD)cr );
}

int CIBScanUltimate_SimpleScanWMDlg::_ReleaseDevice()
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

void CIBScanUltimate_SimpleScanWMDlg::_BeepFail()
{
	MessageBeep( -1 );
	Sleep(150);
	MessageBeep( -1 );
	Sleep(150);
	MessageBeep( -1 );
	Sleep(150);
	MessageBeep( -1 );
}

void CIBScanUltimate_SimpleScanWMDlg::_BeepSuccess()
{
	MessageBeep( -1 );
	Sleep(50);
	MessageBeep( -1 );
}

void CIBScanUltimate_SimpleScanWMDlg::_BeepOk()
{
	MessageBeep( -1 );
}

void CIBScanUltimate_SimpleScanWMDlg::_BeepDeviceCommunicationBreak()
{
	for( int i=0; i<5; i++ )
	{
		MessageBeep( -1 );
		Sleep( 500 );
	}
}

void CIBScanUltimate_SimpleScanWMDlg::_SaveBitmapImage( const IBSU_ImageData &image, const CString &fingerName ) 
{
	if( ( m_ImgSaveFolder[0] == 0 ) || ( m_ImgSubFolder.GetLength() == 0 ) )
	{
		return;
	}
  
	CString strFolder;
	strFolder.Format( _T( "%s\\%s" ), m_ImgSaveFolder, m_ImgSubFolder );
	CreateDirectory( strFolder, NULL );

	CString strFileName;
	strFileName.Format( _T( "%s\\Image_%02d_%s.bmp" ), strFolder, m_nCurrentCaptureStep, fingerName );

	if( IBSU_SaveBitmapImage( strFileName, (BYTE*)image.Buffer, 
							  image.Width, image.Height, image.Pitch, 
							  image.ResolutionX, image.ResolutionY ) != IBSU_STATUS_OK )
	{
		AfxMessageBox( _T( "Failed to save bitmap image!" ));
	}
}

void CIBScanUltimate_SimpleScanWMDlg::_SaveWsqImage( const IBSU_ImageData &image, const CString &fingerName ) 
{
	if( ( m_ImgSaveFolder[0] == 0 ) || ( m_ImgSubFolder.GetLength() == 0 ) )
	{
		return;
	}
  
	CString strFolder;
	strFolder.Format( _T( "%s\\%s" ), m_ImgSaveFolder, m_ImgSubFolder );
	CreateDirectory( strFolder, NULL );

	CString strFileName;
	strFileName.Format( _T( "%s\\Image_%02d_%s.wsq" ), strFolder, m_nCurrentCaptureStep, fingerName );

	if( IBSU_WSQEncodeToFile( strFileName, (BYTE*)image.Buffer, 
							  image.Width, image.Height, image.Pitch, image.BitsPerPixel, 
							  (int)image.ResolutionX, (double)0.75, _T("") ) != IBSU_STATUS_OK )
	{
		AfxMessageBox( _T( "Failed to save WSQ image!" ));
	}
}

int CIBScanUltimate_SimpleScanWMDlg::_ExecuteInitializeDevice(int nRc, int devHandle)
{
	if( nRc >= IBSU_STATUS_OK )
	{
		m_nDevHandle = devHandle;
		    
		CWnd *disWnd = GetDlgItem( IDC_FRAME_IMAGE );
		RECT clientRect;
		disWnd->GetClientRect( &clientRect );

		// Enable power save mode
		IBSU_SetProperty(devHandle, ENUM_IBSU_PROPERTY_ENABLE_POWER_SAVE_MODE, _T("TRUE"));

		// create display window
		IBSU_CreateClientWindow( devHandle, disWnd->m_hWnd, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom );

		IBSU_AddOverlayQuadrangle(devHandle, &m_nOvClearPlatenHandle, 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);
		IBSU_AddOverlayText(devHandle, &m_nOvImageTextHandle, TEXT("Arial"), 10, TRUE/*bold*/, TEXT(""), 10, 10, (DWORD)0);
		for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
			IBSU_AddOverlayQuadrangle(devHandle, &m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);

/** Default value is 0x00d8e9ec( COLOR_BTNFACE ) for enumeration ENUM_IBSU_WINDOW_PROPERTY_BK_COLOR
 ** You can change the background color as using method below
			char cValue[16];
			DWORD bkColor = GetSysColor( COLOR_BTNFACE );
			sprintf(cValue, "%u", bkColor);
			IBSU_SetClientDisplayProperty( devHandle, ENUM_IBSU_WINDOW_PROPERTY_BK_COLOR, cValue );
*/

/** Default value is TRUE for enumeration ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE
 ** You can remove the guide line of rolling as using method below
			char cValue[16];
			sprintf(cValue, "%s", "FALSE");
			IBSU_SetClientDisplayProperty( devHandle, ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE, cValue );
*/

		// register callback functions
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, OnEvent_DeviceCommunicationBreak, this );    
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, OnEvent_PreviewImage, this );
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, OnEvent_TakingAcquisition, this );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, OnEvent_CompleteAcquisition, this );  
        IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX, OnEvent_ResultImageEx, this );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, OnEvent_FingerCount, this );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY, OnEvent_FingerQuality, this );  
		if( m_chkUseClearPlaten )
			IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE, OnEvent_ClearPlatenAtCapture, this );  
	}

	// status notification and sequence start
	if( nRc == IBSU_STATUS_OK )
	{
		PostMessage( WM_USER_CAPTURE_SEQ_START );
		return 0;
	}

	if( nRc > IBSU_STATUS_OK )
		PostMessage( WM_USER_INIT_WARNING, nRc );
	else 
	{
		switch (nRc)
		{
		case IBSU_ERR_DEVICE_ACTIVE:
			_SetStatusBarMessage( _T( "[Error code = %d] Device initialization failed because in use by another thread/process." ), nRc );
			break;
		case IBSU_ERR_USB20_REQUIRED:
			_SetStatusBarMessage( _T( "[Error code = %d] Device initialization failed because SDK only works with USB 2.0." ), nRc );
			break;
		default:
			_SetStatusBarMessage( _T( "[Error code = %d] Device initialization failed" ), nRc );
			break;
		}
	}

	PostMessage( WM_USER_UPDATE_DISPLAY_RESOURCES );
	return 0;
}

int CIBScanUltimate_SimpleScanWMDlg::_ModifyOverlayForWarningOfClearPlaten(BOOL bVisible)
{
	if( m_nDevHandle == -1 )
		return -1;

	CWnd *disWnd = GetDlgItem( IDC_FRAME_IMAGE );
	RECT clientRect;

    int      nRc = IBSU_STATUS_OK;
    COLORREF cr = RGB(255, 0, 0);
    int      left, top, right, bottom;

	disWnd->GetClientRect( &clientRect );
    left = 0; top = 0; right = clientRect.right-clientRect.left; bottom = clientRect.bottom-clientRect.top;
    if( bVisible )
	{
		nRc = IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvClearPlatenHandle,
			left, top, right, top, right, bottom, left, bottom, 20, (DWORD)cr);
	}
	else
	{
		nRc = IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvClearPlatenHandle,
			0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);
	}

    return nRc;
}




////////////////////////////////////////////////////////////////////////////////////////////
void CIBScanUltimate_SimpleScanWMDlg::OnEvent_DeviceCommunicationBreak(
						 const int deviceHandle,
						 void*     pContext
					   )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SimpleScanWMDlg *pDlg = reinterpret_cast<CIBScanUltimate_SimpleScanWMDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->PostMessage( WM_USER_DEVICE_COMMUNICATION_BREAK );
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SimpleScanWMDlg::OnEvent_PreviewImage(
						 const int deviceHandle,
						 void*     pContext,
						 const	   IBSU_ImageData image
					   )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SimpleScanWMDlg *pDlg = reinterpret_cast<CIBScanUltimate_SimpleScanWMDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	CString message;
	message.Format(_T("%s (%.1f fps)"), pDlg->m_strImageMessage, 1000/(image.FrameTime*1000));
	pDlg->_SetImageMessage(message);

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SimpleScanWMDlg::OnEvent_FingerCount(
						   const int                   deviceHandle,
						   void*                       pContext,
						   const IBSU_FingerCountState fingerCountState
						 )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SimpleScanWMDlg *pDlg = reinterpret_cast<CIBScanUltimate_SimpleScanWMDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

/*
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
*/
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SimpleScanWMDlg::OnEvent_FingerQuality(
                          const int                     deviceHandle,   
                          void                          *pContext,       
                          const IBSU_FingerQualityState *pQualityArray, 
                          const int                     qualityArrayCount    
						 )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SimpleScanWMDlg *pDlg = reinterpret_cast<CIBScanUltimate_SimpleScanWMDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	memcpy(pDlg->m_FingerQuality, pQualityArray, sizeof(IBSU_FingerQualityState)*qualityArrayCount);
	pDlg->PostMessage( WM_USER_DRAW_FINGER_QUALITY );

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SimpleScanWMDlg::OnEvent_DeviceCount(
						 const int detectedDevices,
						 void      *pContext
					   )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SimpleScanWMDlg *pDlg = reinterpret_cast<CIBScanUltimate_SimpleScanWMDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->PostMessage( WM_USER_UPDATE_DEVICE_LIST );
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SimpleScanWMDlg::OnEvent_InitProgress(
						 const int deviceIndex,
						 void      *pContext,
						 const int progressValue
					   )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SimpleScanWMDlg *pDlg = reinterpret_cast<CIBScanUltimate_SimpleScanWMDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->_SetStatusBarMessage(_T("Initializing device... %d%%"), progressValue);
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SimpleScanWMDlg::OnEvent_TakingAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SimpleScanWMDlg *pDlg = reinterpret_cast<CIBScanUltimate_SimpleScanWMDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	if( imageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		pDlg->PostMessage( WM_USER_BEEP, __BEEP_OK__ );
		pDlg->m_strImageMessage.Format(_T("When done remove finger from sensor"));
		pDlg->_SetImageMessage(pDlg->m_strImageMessage);
		pDlg->_SetStatusBarMessage(pDlg->m_strImageMessage);
	}

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SimpleScanWMDlg::OnEvent_CompleteAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SimpleScanWMDlg *pDlg = reinterpret_cast<CIBScanUltimate_SimpleScanWMDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	if( imageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		pDlg->PostMessage( WM_USER_BEEP, __BEEP_OK__ );
	}
	else
	{
		pDlg->m_strImageMessage.Format(_T("Remove fingers from sensor"));
		pDlg->_SetImageMessage(pDlg->m_strImageMessage);
		pDlg->_SetStatusBarMessage(_T("Acquisition completed, postprocessing.."));
	}

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SimpleScanWMDlg::OnEvent_ResultImageEx(
                          const int                     deviceHandle,
                          void                          *pContext,
                          const int                     imageStatus,
                          const IBSU_ImageData          image,
						  const IBSU_ImageType          imageType,
						  const int                     detectedFingerCount,
                          const int                     segmentImageArrayCount,
                          const IBSU_ImageData          *pSegmentImageArray,
						  const IBSU_SegmentPosition    *pSegmentPositionArray
                        )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SimpleScanWMDlg *pDlg = reinterpret_cast<CIBScanUltimate_SimpleScanWMDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	if( deviceHandle != pDlg->m_nDevHandle )
		ASSERT( FALSE );

	// added 2012-11-30
	if( pDlg->m_bNeedClearPlaten )
	{
		pDlg->m_bNeedClearPlaten = FALSE;
		pDlg->PostMessage( WM_USER_DRAW_FINGER_QUALITY );
	}

    if( imageStatus >= IBSU_STATUS_OK )
    {
		if( pDlg->m_chkSaveImages )
		{
			pDlg->_SetStatusBarMessage( _T( "Saving image..." ) );
			CaptureInfo info = pDlg->m_vecCaptureSeq[pDlg->m_nCurrentCaptureStep];
			pDlg->_SaveBitmapImage(image, info.fingerName);
			// pDlg->_SaveWsqImage(image, info.fingerName);	// It takes several seconds
				if( segmentImageArrayCount > 0 )
			{
					CString			segmentName;
					for( int i=0; i<segmentImageArrayCount; i++ )
				{
						segmentName.Format(TEXT("%s_Segment_%02d"), info.fingerName, i);
						pDlg->_SaveBitmapImage(*(pSegmentImageArray+i), segmentName);
						//pDlg->_SaveWsqImage(*(pSegmentImageArray+i), segmentName);	// It takes several seconds
				}
			}
		}

		if( pDlg->m_chkDrawSegmentImage )
		{
			// Draw quadrangle for the segment image
			TCHAR propertyValue[32];
			IBSU_GetClientWindowProperty(deviceHandle, ENUM_IBSU_WINDOW_PROPERTY_SCALE_FACTOR_EX, propertyValue);
			double scaleFactor = (double)_ttoi(propertyValue)/1000000;
			IBSU_GetClientWindowProperty(deviceHandle, ENUM_IBSU_WINDOW_PROPERTY_LEFT_MARGIN, propertyValue);
			int leftMargin = _ttoi(propertyValue);
			IBSU_GetClientWindowProperty(deviceHandle, ENUM_IBSU_WINDOW_PROPERTY_TOP_MARGIN, propertyValue);
			int topMargin = _ttoi(propertyValue);
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
		}

//	CaptureInfo info = pDlg->m_vecCaptureSeq[pDlg->m_nCurrentCaptureStep];
//	IBSM_ImageData resultImage;
//	IBSM_ImageData splitResultImage[4];
//	int splitResultImageCount;
//	pDlg->_SetStatusBarMessage( _T( "Get IBSM_ResultImageInfo..." ) );
//	IBSU_GetIBSM_ResultImageInfo(deviceHandle, info.fingerPosition, &resultImage, &splitResultImage[0], &splitResultImageCount);

		if( pDlg->m_chkUseNFIQ )
		{
			int nfiq_score[IBSU_MAX_SEGMENT_COUNT];
			int	score=0, nRc, segment_pos=0;
			memset(&nfiq_score, 0, sizeof(nfiq_score));
			CString strValue = _T("Err");
			pDlg->_SetStatusBarMessage( _T( "Get NFIQ score..." ) );
			for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
			{
				if( pDlg->m_FingerQuality[i] == ENUM_IBSU_FINGER_NOT_PRESENT )
					continue;
				nRc = IBSU_GetNFIQScore(deviceHandle, (const BYTE*)(pSegmentImageArray+segment_pos)->Buffer,
					(pSegmentImageArray+segment_pos)->Width, (pSegmentImageArray+segment_pos)->Height,
					(pSegmentImageArray+segment_pos)->BitsPerPixel, &score);
				if( nRc == IBSU_STATUS_OK )
					nfiq_score[i] = score;
				else
					nfiq_score[i] = -1;

				strValue.Format(TEXT("%d"), nfiq_score[i]);
				pDlg->GetDlgItem( IDC_STATIC_QUALITY_1+i )->SetWindowText(strValue);
				segment_pos++;
			}
		}

		if( imageStatus == IBSU_STATUS_OK )
        {
			pDlg->m_strImageMessage.Format(_T("Acquisition completed successfully"));
			pDlg->_SetImageMessage(pDlg->m_strImageMessage);
			pDlg->_SetStatusBarMessage(pDlg->m_strImageMessage);

			pDlg->PostMessage( WM_USER_BEEP, __BEEP_SUCCESS__ );
        }
        else
        {
	        pDlg->m_strImageMessage.Format(_T("Acquisition Warning (code = %d)"), imageStatus);
	        pDlg->_SetImageMessage(pDlg->m_strImageMessage);
	        pDlg->_SetStatusBarMessage(pDlg->m_strImageMessage);
            CString askMsg;
            askMsg.Format(TEXT("[Warning = %d] Do you want a recapture?"), imageStatus);
       		if( AfxMessageBox(askMsg, MB_YESNO | MB_ICONINFORMATION) == IDYES )
            {
                pDlg->m_nCurrentCaptureStep--;
            }
            else
            {
    	        pDlg->PostMessage( WM_USER_BEEP, __BEEP_SUCCESS__ );
            }
        }
    }
    else
    {
	    pDlg->m_strImageMessage.Format(_T("Acquisition failed (code = %d)"), imageStatus);
	    pDlg->_SetImageMessage(pDlg->m_strImageMessage);
	    pDlg->_SetStatusBarMessage(pDlg->m_strImageMessage);

        pDlg->PostMessage( WM_USER_BEEP, __BEEP_FAIL__ );
        // Stop all of acquisition
        pDlg->m_nCurrentCaptureStep = (int)pDlg->m_vecCaptureSeq.size();
    }

	pDlg->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SimpleScanWMDlg::OnEvent_ClearPlatenAtCapture(
                          const int                   deviceHandle,
                          void                        *pContext,
                          const IBSU_PlatenState      platenState
                        )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SimpleScanWMDlg *pDlg = reinterpret_cast<CIBScanUltimate_SimpleScanWMDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	if( platenState == ENUM_IBSU_PLATEN_HAS_FINGERS )
		pDlg->m_bNeedClearPlaten = TRUE;
	else
		pDlg->m_bNeedClearPlaten = FALSE;

	if( pDlg->m_bNeedClearPlaten )
	{
		pDlg->m_strImageMessage.Format(_T("Please remove your fingers on the platen first!"));
		pDlg->_SetImageMessage(pDlg->m_strImageMessage);
		pDlg->_SetStatusBarMessage(pDlg->m_strImageMessage);
	}
	else
	{
		CaptureInfo info = pDlg->m_vecCaptureSeq[pDlg->m_nCurrentCaptureStep];

		// Display message for image acuisition again
		CString strMessage;
		strMessage = info.PreCaptureMessage;

		pDlg->_SetStatusBarMessage(_T("%s"), strMessage);
		pDlg->_SetImageMessage(_T("%s"), strMessage);
		pDlg->m_strImageMessage = strMessage;
	}

	pDlg->PostMessage( WM_USER_DRAW_FINGER_QUALITY );

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SimpleScanWMDlg::OnEvent_AsyncOpenDevice(
                          const int                   deviceIndex,
                          void                        *pContext,
                          const int                   deviceHandle,
                          const int                   errorCode
						  )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SimpleScanWMDlg *pDlg = reinterpret_cast<CIBScanUltimate_SimpleScanWMDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	pDlg->m_bInitializing = false;
	pDlg->PostMessage( WM_USER_ASYNC_OPEN_DEVICE, errorCode, deviceHandle );

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SimpleScanWMDlg::OnEvent_NotifyMessage(
                          const int                deviceHandle,
                          void                     *pContext,
                          const int                message
						  )
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SimpleScanWMDlg *pDlg = reinterpret_cast<CIBScanUltimate_SimpleScanWMDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	TRACE(TEXT("OnEvent_NotifyMessage = %d\r\n"), message);

	LeaveCriticalSection(&g_CriticalSection);
}

////////////////////////////////////////////////////////////////////////////////////////////







LRESULT CIBScanUltimate_SimpleScanWMDlg::OnMsg_CaptureSeqStart(WPARAM wParam, LPARAM lParam)
{
	if( m_nDevHandle == -1 )
	{
		OnMsg_UpdateDisplayResources();
		return 0L;
	}

	m_vecCaptureSeq.clear();
	CaptureInfo info;

//	if( strCaptureSeq == CAPTURE_SEQ_FLAT_SINGLE_FINGER )
	{
		info.PreCaptureMessage = _T("Put finger on sensor.");
		info.PostCaptuerMessage = _T("Keep finger on the sensor!");
		info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
		info.NumberOfFinger = 1;
		info.fingerName = _T("SFF_Unknown");
		info.fingerPosition = IBSM_FINGER_POSITION_UNKNOWN;
		m_vecCaptureSeq.push_back(info);
	}

	// Make subfolder name
	SYSTEMTIME localTime;
	GetLocalTime(&localTime);
	m_ImgSubFolder.Format( _T( "%04d-%02d-%02d %02d%02d%02d" ),
                          localTime.wYear, localTime.wMonth, localTime.wDay,
                          localTime.wHour, localTime.wMinute, localTime.wSecond );
  
	PostMessage( WM_USER_CAPTURE_SEQ_NEXT );

	return 0L;
}

LRESULT CIBScanUltimate_SimpleScanWMDlg::OnMsg_CaptureSeqNext(WPARAM wParam, LPARAM lParam)
{
	int nRc;

	if( m_nDevHandle == -1 )
		return 0L;

	m_bBlank = FALSE;
	memset(&m_FingerQuality[0], ENUM_IBSU_FINGER_NOT_PRESENT, sizeof(m_FingerQuality));

	m_nCurrentCaptureStep++;
	if( m_nCurrentCaptureStep >= (int)m_vecCaptureSeq.size() )
	{
		// All of capture sequence completely
		m_nCurrentCaptureStep = -1;
		m_ImgSubFolder = _T("");

		OnMsg_UpdateDisplayResources();
		GetDlgItem(IDC_BTN_CAPTURE_START)->SetFocus();
		return 0L;
	}

	for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
		IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);

	GetDlgItem( IDC_STATIC_QUALITY_1 )->SetWindowText(_T(""));
	GetDlgItem( IDC_STATIC_QUALITY_2 )->SetWindowText(_T(""));
	GetDlgItem( IDC_STATIC_QUALITY_3 )->SetWindowText(_T(""));
	GetDlgItem( IDC_STATIC_QUALITY_4 )->SetWindowText(_T(""));

	// Make capture delay for display result image on multi capture mode (500 ms)
	if( m_nCurrentCaptureStep > 0 )
	{
		Sleep(500);
		m_strImageMessage = _T( "" );
	}

	CaptureInfo info = m_vecCaptureSeq[m_nCurrentCaptureStep];

	IBSU_ImageResolution imgRes = ENUM_IBSU_IMAGE_RESOLUTION_500;
	BOOL bAvailable = FALSE;
	nRc = IBSU_IsCaptureAvailable(m_nDevHandle, info.ImageType, imgRes, &bAvailable);
	if( nRc != IBSU_STATUS_OK || !bAvailable )
	{
		_SetStatusBarMessage(_T("The capture mode (%d) is not available"), info.ImageType);
		m_nCurrentCaptureStep = -1;
		OnMsg_UpdateDisplayResources();
		return 0L;
	}

	// Start capture
	DWORD captureOptions = 0;
		captureOptions |= IBSU_OPTION_AUTO_CONTRAST;
		captureOptions |= IBSU_OPTION_AUTO_CAPTURE;
		captureOptions |= IBSU_OPTION_IGNORE_FINGER_COUNT;

	nRc = IBSU_BeginCaptureImage(m_nDevHandle, info.ImageType, imgRes, captureOptions);
	if( nRc >= IBSU_STATUS_OK )
	{
		// Display message for image acuisition
		CString strMessage;
		strMessage = info.PreCaptureMessage;

		_SetStatusBarMessage(_T("%s"), strMessage);
		_SetImageMessage(_T("%s"), strMessage);
		m_strImageMessage = strMessage;
	}
	else
	{
		_SetStatusBarMessage(_T("Failed to execute IBSU_BeginCaptureImage()"));
		m_nCurrentCaptureStep = -1;
	}

	OnMsg_UpdateDisplayResources();

	return 0L;
}

LRESULT CIBScanUltimate_SimpleScanWMDlg::OnMsg_DeviceCommunicationBreak(WPARAM wParam, LPARAM lParam)
{
	int nRc;

	if( m_nDevHandle == -1 )
		return 0L;

	_SetStatusBarMessage( _T( "Device communication was broken" ) );

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

LRESULT CIBScanUltimate_SimpleScanWMDlg::OnMsg_DrawClientWindow(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CIBScanUltimate_SimpleScanWMDlg::OnMsg_UpdateDeviceList(WPARAM wParam, LPARAM lParam)
{
	const bool idle = !m_bInitializing && ( m_nCurrentCaptureStep == -1 );
	if( idle )
	{
// HACK HACK
//	GetDlgItem( IDC_BTN_CAPTURE_STOP )->EnableWindow( TRUE );
		GetDlgItem( IDC_BTN_CAPTURE_STOP )->EnableWindow( FALSE );
		GetDlgItem( IDC_BTN_CAPTURE_START )->EnableWindow( FALSE );
	}

	// store currently selected device
	CString strSelectedText;
	int selectedDev = m_cboUsbDevices.GetCurSel();
	if( selectedDev > -1 )
	m_cboUsbDevices.GetLBText( selectedDev, strSelectedText );

	m_cboUsbDevices.ResetContent();
	m_cboUsbDevices.AddString( _T( "- Please select -" ) );

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

		m_cboUsbDevices.AddString( strDevice );
		if( strDevice == strSelectedText )
			selectedDev = i + 1;
	}

	if( ( selectedDev == 0 ) && ( m_cboUsbDevices.GetCount() == 2 ) )
		selectedDev = 1;

	m_cboUsbDevices.SetCurSel( selectedDev );

	if( idle )
	{
		OnCbnSelchangeComboDevices();
		OnMsg_UpdateDisplayResources();
	}

	return 0L;
}

LRESULT CIBScanUltimate_SimpleScanWMDlg::OnMsg_InitWarning(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CIBScanUltimate_SimpleScanWMDlg::OnMsg_UpdateDisplayResources(WPARAM wParam, LPARAM lParam)
{
	BOOL selectedDev = m_cboUsbDevices.GetCurSel() > 0;
	BOOL idle = !m_bInitializing && ( m_nCurrentCaptureStep == -1 );
	BOOL active = !m_bInitializing && (m_nCurrentCaptureStep != -1 );
	BOOL uninitializedDev = selectedDev && ( m_nDevHandle == -1 );


	m_cboUsbDevices.EnableWindow( idle );  

	GetDlgItem( IDC_BTN_CAPTURE_START )->EnableWindow( TRUE );
	GetDlgItem( IDC_BTN_CAPTURE_STOP )->EnableWindow( active );
	
	CString strCaption = _T( "" );
	if( active )
		strCaption = _T( "Take Result" );  
	else if( !active && !m_bInitializing )
		strCaption = _T("Start");  

	SetDlgItemText( IDC_BTN_CAPTURE_START, strCaption );
	return 0;
}

LRESULT CIBScanUltimate_SimpleScanWMDlg::OnMsg_UpdateStatusMessage(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CIBScanUltimate_SimpleScanWMDlg::OnMsg_Beep(WPARAM wParam, LPARAM lParam)
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


LRESULT CIBScanUltimate_SimpleScanWMDlg::OnMsg_AsyncOpenDevice(WPARAM wParam, LPARAM lParam)
{
	_ExecuteInitializeDevice((int)wParam, (int)lParam);

	return 0L;
}










void CIBScanUltimate_SimpleScanWMDlg::OnBnClickedBtnCaptureStart()
{
	
	UpdateData();

	if( m_bInitializing )
		return;

	int devIndex = m_cboUsbDevices.GetCurSel() -1;
	if( devIndex < 0 )
		return;

	if( m_nCurrentCaptureStep != -1 )
	{
		BOOL IsActive;
		int nRc;
		nRc = IBSU_IsCaptureActive(m_nDevHandle, &IsActive);
		if( nRc == IBSU_STATUS_OK && IsActive )
		{
			nRc = IBSU_TakeResultImageManually(m_nDevHandle);
			if( nRc != IBSU_STATUS_OK )
				DebugBreak();
		}

		return;
	}

	if( m_nDevHandle == -1 )
	{
		m_bInitializing = true;
		if( m_chkOpenDeviceEx )
		{

		}
		if( m_chkAsyncOpenDevice )
		{
			// Asynchronously Open Device
			// Function IBSU_OpenDevice() will not be blocked while initializing the algorithm (4 - 7seconds).
			// And you can start the capture after getting callback funcation IBSU_CallbackAsyncOpenDevice().
			int nRc, tmpDevHandle;
			if( m_chkOpenDeviceEx )
				nRc = IBSU_OpenDeviceEx( devIndex, m_UMFolder, TRUE, &tmpDevHandle/* Always vaule is "-1" when async mode"*/ );
			else
				nRc = IBSU_AsyncOpenDevice( devIndex );

			if( nRc != IBSU_STATUS_OK )
			{
				if( nRc > IBSU_STATUS_OK )
					PostMessage( WM_USER_INIT_WARNING, nRc );
				else 
				{
					switch (nRc)
					{
					case IBSU_ERR_DEVICE_ACTIVE:
						_SetStatusBarMessage( _T( "[Error code = %d] Device initialization failed because in use by another thread/process." ), nRc );
						break;
					case IBSU_ERR_USB20_REQUIRED:
						_SetStatusBarMessage( _T( "[Error code = %d] Device initialization failed because SDK only works with USB 2.0." ), nRc );
						break;
					default:
						_SetStatusBarMessage( _T( "[Error code = %d] Device initialization failed" ), nRc );
						break;
					}
				}
			}
		}
		else
		{
			VERIFY( ::AfxBeginThread( _InitializeDeviceThreadCallback, this, THREAD_PRIORITY_NORMAL,
									  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
		}
	}
	else
	{
		// device already initialized
		PostMessage(WM_USER_CAPTURE_SEQ_START);
	}

	OnMsg_UpdateDisplayResources();
}

void CIBScanUltimate_SimpleScanWMDlg::OnBnClickedBtnCaptureStop()
{	

	if( m_nDevHandle == -1 )
		return;

	IBSU_CancelCaptureImage( m_nDevHandle );
	m_nCurrentCaptureStep = -1;
	m_bNeedClearPlaten = FALSE;
	m_bBlank = FALSE;

	_SetStatusBarMessage( _T( "Capture Sequence aborted" ) );
	m_strImageMessage = _T( "" );
	_SetImageMessage(_T( "" ));
	OnMsg_UpdateDisplayResources();
}


void CIBScanUltimate_SimpleScanWMDlg::OnCbnSelchangeComboDevices()
{
	if( m_cboUsbDevices.GetCurSel() == m_nSelectedDevIndex )
		return;

	AfxGetApp()->DoWaitCursor( 1 );

	m_nSelectedDevIndex = m_cboUsbDevices.GetCurSel(); 
	if( m_nDevHandle != -1 )
	{
		OnBnClickedBtnCaptureStop();
		_ReleaseDevice();
	}

	OnMsg_UpdateDisplayResources();
	
	AfxGetApp()->DoWaitCursor( -1 );
}





void CIBScanUltimate_SimpleScanWMDlg::OnTimer(UINT_PTR nIDEvent)
{
	if( nIDEvent == TIMER_STATUS_FINGER_QUALITY )
	{
        BOOL idle = !m_bInitializing && ( m_nCurrentCaptureStep == -1 );
/*        if( !idle )
        {
            if( m_bNeedClearPlaten && m_bBlank )
			{
                _ModifyOverlayForWarningOfClearPlaten(TRUE);
			}
            else
                _ModifyOverlayForWarningOfClearPlaten(FALSE);
        }
*/
		for( int i = 0; i < 4; i++ )
		{
		  GetDlgItem(IDC_STATIC_QUALITY_1+i)->RedrawWindow();
		}
		if( m_bNeedClearPlaten )
			m_bBlank = !m_bBlank;
	}

	CDialog::OnTimer(nIDEvent);
}

HBRUSH CIBScanUltimate_SimpleScanWMDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	const int WndID = pWnd->GetDlgCtrlID();
	switch( WndID )
	{
	case IDC_STATIC_DLL_VER:
	case IDC_STATIC_COPYRIGHT:
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(255, 255, 255));
		hbr = (HBRUSH)GetStockObject( NULL_BRUSH );
		if( WndID == IDC_STATIC_DLL_VER )
		{
			CRect	rect;
			CFont	fFont, *fOldFont;
			int		font_size;
			CString szText;

			pWnd->GetWindowText(szText);
			pWnd->GetClientRect(rect);

			fFont.Detach();
			font_size = -::MulDiv(14, GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
			if( fFont.CreateFont(font_size, 0, 0, 0, FW_NORMAL, 0, 0, 0,DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS, ANTIALIASED_QUALITY,
				DEFAULT_PITCH | FF_DONTCARE, TEXT("MS Sans Serif")) != NULL )
			{
				fOldFont = pDC->SelectObject(&fFont);
				pDC->DrawText(szText, rect, DT_LEFT);
			}
		}
		break;
	}

	for( int i = 0; i < 4; i++ )
	{
		if( WndID == IDC_STATIC_QUALITY_1+i )
		{
			if( m_bNeedClearPlaten )
			{
				if( m_bBlank )
					hbr = static_cast<HBRUSH>(s_brushRed);
				else
					hbr = static_cast<HBRUSH>(s_brushPlaten);
			}
			else
			{
				switch( m_FingerQuality[i] )
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

	return hbr;
}



void CIBScanUltimate_SimpleScanWMDlg::OnClose()
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

