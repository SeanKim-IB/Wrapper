
// IBSU_MultipleScanSampleForVCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IBSU_MultipleScanSampleForVC.h"
#include "IBSU_MultipleScanSampleForVCDlg.h"

#include "IBScanUltimateApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define TIMER_STATUS_DELAY					300

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
#define WM_USER_ASK_RECAPTURE					WM_APP + 12


// Beep definitions
const int __BEEP_FAIL__							= 0;
const int __BEEP_SUCCESS__						= 1;
const int __BEEP_OK__							= 2;
const int __BEEP_DEVICE_COMMUNICATION_BREAK__	= 3;


// Use critical section
CRITICAL_SECTION g_CriticalSection;


// CIBScanUltimate_SampleForVCDlg dialog
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

// CIBSU_MultipleScanSampleForVCDlg dialog




CIBSU_MultipleScanSampleForVCDlg::CIBSU_MultipleScanSampleForVCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIBSU_MultipleScanSampleForVCDlg::IDD, pParent)
	, m_chkAutoContrast(TRUE)
	, m_chkAutoCapture(TRUE)
	, m_chkIgnoreFingerCount(FALSE)
	, m_chkSaveImages(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_nSelectedDevIndex = -1;

	InitializeCriticalSection(&g_CriticalSection);
}

CIBSU_MultipleScanSampleForVCDlg::~CIBSU_MultipleScanSampleForVCDlg()
{
	DeleteCriticalSection(&g_CriticalSection);
}

void CIBSU_MultipleScanSampleForVCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CAPTURE_SEQ, m_cboCaptureSeq);
	DDX_Check(pDX, IDC_CHECK_AUTO_CONTRAST, m_chkAutoContrast);
	DDX_Check(pDX, IDC_CHECK_AUTO_CAPTURE, m_chkAutoCapture);
	DDX_Check(pDX, IDC_CHECK_IGNORE_FINGER_COUNT, m_chkIgnoreFingerCount);
	DDX_Check(pDX, IDC_CHECK_SAVE_IMAGES, m_chkSaveImages);
	DDX_Control(pDX, IDC_FRAME_IMAGE_1, m_frameImage[0]);
	DDX_Control(pDX, IDC_FRAME_IMAGE_2, m_frameImage[1]);
	DDX_Control(pDX, IDC_FRAME_IMAGE_3, m_frameImage[2]);
	DDX_Control(pDX, IDC_FRAME_IMAGE_4, m_frameImage[3]);
	DDX_Control(pDX, IDC_SLIDER_CONTRAST, m_sliderContrast);
	DDX_Control(pDX, IDC_PIC_IB_LOGO, m_IBLogo);
	DDX_Control(pDX, IDC_LIST_DEVICES, m_listUsbDevices);
}

BEGIN_MESSAGE_MAP(CIBSU_MultipleScanSampleForVCDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	// user window messages
	ON_MESSAGE(WM_USER_CAPTURE_SEQ_START,			OnMsg_CaptureSeqStart)
	ON_MESSAGE(WM_USER_CAPTURE_SEQ_NEXT,			OnMsg_CaptureSeqNext)
	ON_MESSAGE(WM_USER_DEVICE_COMMUNICATION_BREAK,	OnMsg_DeviceCommunicationBreak)
	ON_MESSAGE(WM_USER_UPDATE_DEVICE_LIST,			OnMsg_UpdateDeviceList)
	ON_MESSAGE(WM_USER_INIT_WARNING,				OnMsg_InitWarning)
	ON_MESSAGE(WM_USER_UPDATE_DISPLAY_RESOURCES,	OnMsg_UpdateDisplayResources)
	ON_MESSAGE(WM_USER_UPDATE_STATUS_MESSAGE,		OnMsg_UpdateStatusMessage)
	ON_MESSAGE(WM_USER_BEEP,						OnMsg_Beep)
	ON_MESSAGE(WM_USER_ASK_RECAPTURE,				OnMsg_AskRecapture)

	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_CAPTURE_START, &CIBSU_MultipleScanSampleForVCDlg::OnBnClickedBtnCaptureStart)
	ON_BN_CLICKED(IDC_BTN_CAPTURE_STOP, &CIBSU_MultipleScanSampleForVCDlg::OnBnClickedBtnCaptureStop)
	ON_BN_CLICKED(IDC_BTN_IMAGE_FOLDER, &CIBSU_MultipleScanSampleForVCDlg::OnBnClickedBtnImageFolder)
//	ON_CBN_SELCHANGE(IDC_LIST_DEVICES, &CIBSU_MultipleScanSampleForVCDlg::OnCbnSelchangeComboDevices)
	ON_CBN_SELCHANGE(IDC_COMBO_CAPTURE_SEQ, &CIBSU_MultipleScanSampleForVCDlg::OnCbnSelchangeComboCaptureSeq)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_CONTRAST, &CIBSU_MultipleScanSampleForVCDlg::OnNMReleasedcaptureSliderContrast)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CONTRAST, &CIBSU_MultipleScanSampleForVCDlg::OnNMCustomdrawSliderContrast)
	ON_BN_CLICKED(IDC_CHECK_AUTO_CONTRAST, &CIBSU_MultipleScanSampleForVCDlg::OnBnClickedCheckAutoContrast)
	ON_WM_TIMER()
	ON_CLBN_CHKCHANGE(IDC_LIST_DEVICES, OnCheckchangeListDevices)
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CIBSU_MultipleScanSampleForVCDlg message handlers

BOOL CIBSU_MultipleScanSampleForVCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	RECT	rect;

	GetDlgItem(IDC_PIC_IB_LOGO)->GetWindowRect( &rect );
	GetDlgItem(IDC_PIC_IB_LOGO)->SetWindowPos(NULL, 0, 0, (rect.right-rect.left), (rect.bottom-rect.top), SWP_NOSIZE);

	for( int i=0; i<MAX_DEVICE_COUNT; i++ )
	{
		m_nDevHandle[i] = -1;
		m_nCurrentCaptureStep[i] = -1;
		m_bInitializing[i] = FALSE;
		m_strImageMessage[i] = _T( "" );

//		m_bNeedClearPlaten[i] = FALSE;
	}

	m_ImgSaveFolder[0] = 0;
	SHGetSpecialFolderPath( NULL, m_ImgSaveFolder, CSIDL_MYPICTURES, TRUE );

	m_sliderContrast.SetRange(0, 34);
	m_sliderContrast.SetPos(0);
	m_sliderContrast.SetTicFreq(5);
	GetDlgItem(IDC_EDIT_CONTRAST)->SetWindowText("0");

	IBSU_GetSDKVersion(&m_verInfo);
	CString titleName;
	titleName.Format("IntegrationSample for VC++");
	SetWindowText(titleName);

	CString strDrawString;
	strDrawString.Format("VC++ sample with DLL ver. %s", m_verInfo.Product);
	GetDlgItem(IDC_STATIC_DLL_VER)->SetWindowText(strDrawString);

	strDrawString = _T("Copyright (c) Integrated Biometrics");
	GetDlgItem(IDC_STATIC_COPYRIGHT)->SetWindowText(strDrawString);

	m_nDeviceCount = 0;
	IBSU_GetDeviceCount(&m_nDeviceCount);

	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, OnEvent_DeviceCount, this );
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, OnEvent_InitProgress, this );


	m_pInitialThread = NULL;

	PostMessage( WM_USER_UPDATE_DEVICE_LIST );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIBSU_MultipleScanSampleForVCDlg::OnPaint()
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
HCURSOR CIBSU_MultipleScanSampleForVCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



////////////////////////////////////////////////////////////////////////////////////////////
// User defined functions

UINT CIBSU_MultipleScanSampleForVCDlg::_InitializeDeviceThreadCallback( LPVOID pParam )
{   
	if( pParam == NULL )
		return 1;

	ThreadParam*	pThreadParam = reinterpret_cast<ThreadParam*>( pParam );
	CIBSU_MultipleScanSampleForVCDlg*	pDlg = (CIBSU_MultipleScanSampleForVCDlg*)pThreadParam->pParentApp;
	const int		devIndex = pThreadParam->devIndex;
	int				devHandle;
	int				nRc = IBSU_STATUS_OK;


	SetEvent(pThreadParam->threadStarted);

	pDlg->m_bInitializing[devIndex] = true;
	nRc = IBSU_OpenDevice( devIndex, &devHandle );
	pDlg->m_bInitializing[devIndex] = false;

	if( nRc >= IBSU_STATUS_OK )
		pDlg->m_nDevHandle[devIndex] = devHandle;
	return pDlg->_ExecuteInitializeDevice(nRc, devHandle, devIndex);
}

void CIBSU_MultipleScanSampleForVCDlg::_SetStatusBarMessage( int devIndex, LPCTSTR Format, ... )
{
	TCHAR cMessage[IBSU_MAX_STR_LEN+1];
	va_list arg_ptr;

	va_start( arg_ptr, Format );
	_vsntprintf( cMessage, IBSU_MAX_STR_LEN, Format, arg_ptr );
	va_end( arg_ptr );  

	CString strMessage;
	strMessage.Format(_T("%s"), cMessage);
	
	if( devIndex == 0 )
		SetDlgItemText( IDC_TXT_STATUS_1, strMessage);
	else if( devIndex == 1 )
		SetDlgItemText( IDC_TXT_STATUS_2, strMessage);
	else if( devIndex == 2 )
		SetDlgItemText( IDC_TXT_STATUS_3, strMessage);
	else if( devIndex == 3 )
		SetDlgItemText( IDC_TXT_STATUS_4, strMessage);
}

void CIBSU_MultipleScanSampleForVCDlg::_SetImageMessage( int devIndex, LPCSTR Format, ... )
{
	TCHAR cMessage[IBSU_MAX_STR_LEN+1];
	va_list arg_ptr;

	va_start( arg_ptr, Format );
	_vsntprintf( cMessage, IBSU_MAX_STR_LEN, Format, arg_ptr );
	va_end( arg_ptr );

	int			font_size = 8;
	int			x = 0;
	int			y = 0;
	COLORREF	cr = RGB(0, 0, 255);

	IBSU_SetClientWindowOverlayText( m_nDevHandle[devIndex], "Arial", font_size, FALSE/*bold*/, cMessage, x, y, (DWORD)cr );
}

void CIBSU_MultipleScanSampleForVCDlg::_UpdateCaptureSequences()
{
	// store currently selected sequence
	CString strSelectedText;
	int selectedSeq = m_cboCaptureSeq.GetCurSel();
	if( selectedSeq > -1 )
		m_cboCaptureSeq.GetLBText( selectedSeq, strSelectedText );

	// populate combo box
	m_cboCaptureSeq.ResetContent();
	m_cboCaptureSeq.AddString( _T( "- Please select -" ) );

//	const int devIndex = m_cboUsbDevices.GetCurSel() - 1;

	IBSU_DeviceDesc devDesc;
	devDesc.productName[0] = 0;  
//	if( devIndex > -1 )
//		IBSU_GetDeviceDescription( devIndex, &devDesc );
	IBSU_GetDeviceDescription( 0, &devDesc );

	if( ( _stricmp( devDesc.productName, "WATSON" )			== 0) ||
		( _stricmp( devDesc.productName, "WATSON MINI" )	== 0) ||
		( _stricmp( devDesc.productName, "SHERLOCK_ROIC" )	== 0) ||
		( _stricmp( devDesc.productName, "SHERLOCK" )		== 0) )
	{
		m_cboCaptureSeq.AddString( CAPTURE_SEQ_FLAT_SINGLE_FINGER );
		m_cboCaptureSeq.AddString( CAPTURE_SEQ_ROLL_SINGLE_FINGER );
		m_cboCaptureSeq.AddString( CAPTURE_SEQ_2_FLAT_FINGERS );
		m_cboCaptureSeq.AddString( CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS );
		m_cboCaptureSeq.AddString( CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS );
	}
	else if( ( _stricmp( devDesc.productName, "COLUMBO" )		== 0) ||
			 ( _stricmp( devDesc.productName, "CURVE" )			== 0) )
	{
		m_cboCaptureSeq.AddString( CAPTURE_SEQ_FLAT_SINGLE_FINGER );
		m_cboCaptureSeq.AddString( CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS );
	}
	else if( ( _stricmp( devDesc.productName, "HOLMES" )		== 0) )
	{
		m_cboCaptureSeq.AddString( CAPTURE_SEQ_FLAT_SINGLE_FINGER );
		m_cboCaptureSeq.AddString( CAPTURE_SEQ_ROLL_SINGLE_FINGER );
		m_cboCaptureSeq.AddString( CAPTURE_SEQ_2_FLAT_FINGERS );
		m_cboCaptureSeq.AddString( CAPTURE_SEQ_4_FLAT_FINGERS );
		m_cboCaptureSeq.AddString( CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS );
		m_cboCaptureSeq.AddString( CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS );
		m_cboCaptureSeq.AddString( CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER );
	}


	// select previously selected sequence
	if( selectedSeq > -1 )
		selectedSeq = m_cboCaptureSeq.FindString( 0, strSelectedText );
	m_cboCaptureSeq.SetCurSel( selectedSeq == -1 ? 0 : selectedSeq );

	OnMsg_UpdateDisplayResources();
}

int CIBSU_MultipleScanSampleForVCDlg::_ReleaseDevice(int deviceIndex)
{
	int nRc = IBSU_STATUS_OK;

	if( m_nDevHandle[deviceIndex] != -1 )
		nRc = IBSU_CloseDevice( m_nDevHandle[deviceIndex] );

	if( nRc >= IBSU_STATUS_OK )
	{
		m_nDevHandle[deviceIndex] = -1;
		m_nCurrentCaptureStep[deviceIndex] = -1;
		m_bInitializing[deviceIndex] = false;
	}

	return nRc;
}  

void CIBSU_MultipleScanSampleForVCDlg::_BeepFail()
{
	Beep( 3500, 300 );
	Sleep(150);
	Beep( 3500, 150 );
	Sleep(150);
	Beep( 3500, 150 );
	Sleep(150);
	Beep( 3500, 150 );
}

void CIBSU_MultipleScanSampleForVCDlg::_BeepSuccess()
{
	Beep( 3500, 100 );
	Sleep(50);
	Beep( 3500, 100 );
}

void CIBSU_MultipleScanSampleForVCDlg::_BeepOk()
{
	Beep( 3500, 100 );
}

void CIBSU_MultipleScanSampleForVCDlg::_BeepDeviceCommunicationBreak()
{
	for( int i=0; i<8; i++ )
	{
		Beep( 3500, 100 );
		Sleep( 100 );
	}
}

void CIBSU_MultipleScanSampleForVCDlg::_SaveBitmapImage( const IBSU_ImageData &image, const CString &fingerName ) 
{
	if( ( m_ImgSaveFolder[0] == 0 ) || ( m_ImgSubFolder.GetLength() == 0 ) )
	{
		return;
	}
  
	CString strFolder;
	strFolder.Format( _T( "%s\\%s" ), m_ImgSaveFolder, m_ImgSubFolder );
	CreateDirectory( strFolder, NULL );

	CString strFileName;
//	CaptureInfo info = m_vecCaptureSeq[m_nCurrentCaptureStep];
	strFileName.Format( _T( "%s\\Image_%02d_%s.bmp" ), strFolder, m_nCurrentCaptureStep, fingerName );

	if( IBSU_SaveBitmapImage( strFileName, (BYTE*)image.Buffer, 
							  image.Width, image.Height, image.Pitch, 
							  image.ResolutionX, image.ResolutionY ) != IBSU_STATUS_OK )
	{
		AfxMessageBox( _T( "Failed to save bitmap image!" ));
	}
}

int CIBSU_MultipleScanSampleForVCDlg::_ExecuteInitializeDevice(int nRc, int devHandle, int devIndex)
{
	if( nRc >= IBSU_STATUS_OK )
	{
		CWnd *disWnd;

		if( devIndex == 0 )
			disWnd = GetDlgItem( IDC_FRAME_IMAGE_1 );
		else if( devIndex == 1 )
			disWnd = GetDlgItem( IDC_FRAME_IMAGE_2 );
		else if( devIndex == 2 )
			disWnd = GetDlgItem( IDC_FRAME_IMAGE_3 );
		else if( devIndex == 3 )
			disWnd = GetDlgItem( IDC_FRAME_IMAGE_4 );

		RECT clientRect;
		disWnd->GetClientRect( &clientRect );

		// create display window
		IBSU_CreateClientWindow( devHandle, disWnd->m_hWnd, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom );

		// register callback functions
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, OnEvent_DeviceCommunicationBreak, this );    
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, OnEvent_PreviewImage, this );
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, OnEvent_TakingAcquisition, this );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, OnEvent_CompleteAcquisition, this );  

		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX, OnEvent_ResultImageEx, this );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, OnEvent_FingerCount, this );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY, OnEvent_FingerQuality, this );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE, OnEvent_ClearPlatenAtCapture, this );  
	}

	// status notification and sequence start
	if( nRc == IBSU_STATUS_OK )
	{
		PostMessage( WM_USER_CAPTURE_SEQ_START, _GetDeviceIndexFromHandle(devHandle) );
		return 0;
	}

	if( nRc > IBSU_STATUS_OK )
		PostMessage( WM_USER_INIT_WARNING, nRc );
	else 
	{
		switch (nRc)
		{
		case IBSU_ERR_DEVICE_ACTIVE:
			_SetStatusBarMessage( devIndex, _T( "[Error code = %d] Device initialization failed because in use by another thread/process." ), nRc );
			break;
		case IBSU_ERR_USB20_REQUIRED:
			_SetStatusBarMessage( devIndex, _T( "[Error code = %d] Device initialization failed because SDK only works with USB 2.0." ), nRc );
			break;
		default:
			_SetStatusBarMessage( devIndex, _T( "[Error code = %d] Device initialization failed" ), nRc );
			break;
		}
	}

	PostMessage( WM_USER_UPDATE_DISPLAY_RESOURCES );
	return 0;
}

int CIBSU_MultipleScanSampleForVCDlg::_GetDeviceIndexFromHandle( const int devHandle ) 
{
	for( int devIndex=0; devIndex<MAX_DEVICE_COUNT; devIndex++ )
	{
		if( m_nDevHandle[devIndex] == devHandle )
			return devIndex;
	}

	return -1;
}







////////////////////////////////////////////////////////////////////////////////////////////
void CIBSU_MultipleScanSampleForVCDlg::OnEvent_DeviceCommunicationBreak(
						 const int deviceHandle,
						 void*     pContext
					   )
{
	if( pContext == NULL )
		return;

	CIBSU_MultipleScanSampleForVCDlg *pDlg = reinterpret_cast<CIBSU_MultipleScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->PostMessage( WM_USER_DEVICE_COMMUNICATION_BREAK, deviceHandle );
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_MultipleScanSampleForVCDlg::OnEvent_PreviewImage(
						 const int deviceHandle,
						 void*     pContext,
						 const	   IBSU_ImageData image
					   )
{
	if( pContext == NULL )
		return;

	CIBSU_MultipleScanSampleForVCDlg *pDlg = reinterpret_cast<CIBSU_MultipleScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	int devIndex = pDlg->_GetDeviceIndexFromHandle(deviceHandle);
	CString message;
	message.Format(_T("%s (%.1f fps)"), pDlg->m_strImageMessage[devIndex], 1000/(image.FrameTime*1000));
	pDlg->_SetImageMessage(devIndex, message);

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_MultipleScanSampleForVCDlg::OnEvent_FingerCount(
						   const int                   deviceHandle,
						   void*                       pContext,
						   const IBSU_FingerCountState fingerCountState
						 )
{
	if( pContext == NULL )
		return;

	CIBSU_MultipleScanSampleForVCDlg *pDlg = reinterpret_cast<CIBSU_MultipleScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_MultipleScanSampleForVCDlg::OnEvent_FingerQuality(
                          const int                     deviceHandle,   
                          void                          *pContext,       
                          const IBSU_FingerQualityState *pQualityArray, 
                          const int                     qualityArrayCount    
						 )
{
	if( pContext == NULL )
		return;

	CIBSU_MultipleScanSampleForVCDlg *pDlg = reinterpret_cast<CIBSU_MultipleScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_MultipleScanSampleForVCDlg::OnEvent_DeviceCount(
						 const int detectedDevices,
						 void      *pContext
					   )
{
	if( pContext == NULL )
		return;

	CIBSU_MultipleScanSampleForVCDlg *pDlg = reinterpret_cast<CIBSU_MultipleScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->PostMessage( WM_USER_UPDATE_DEVICE_LIST );
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_MultipleScanSampleForVCDlg::OnEvent_InitProgress(
						 const int deviceIndex,
						 void      *pContext,
						 const int progressValue
					   )
{
	if( pContext == NULL )
		return;

	CIBSU_MultipleScanSampleForVCDlg *pDlg = reinterpret_cast<CIBSU_MultipleScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->_SetStatusBarMessage(deviceIndex, _T("Initializing device... %d%%"), progressValue);
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_MultipleScanSampleForVCDlg::OnEvent_TakingAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   )
{
	if( pContext == NULL )
		return;

	CIBSU_MultipleScanSampleForVCDlg *pDlg = reinterpret_cast<CIBSU_MultipleScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	if( imageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		int devIndex = pDlg->_GetDeviceIndexFromHandle(deviceHandle);
		pDlg->PostMessage( WM_USER_BEEP, __BEEP_OK__ );
		pDlg->m_strImageMessage[devIndex].Format(_T("When done remove finger from sensor"));
		pDlg->_SetImageMessage(devIndex, pDlg->m_strImageMessage[devIndex]);
		pDlg->_SetStatusBarMessage(devIndex, pDlg->m_strImageMessage[devIndex]);
	}

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_MultipleScanSampleForVCDlg::OnEvent_CompleteAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   )
{
	if( pContext == NULL )
		return;

	CIBSU_MultipleScanSampleForVCDlg *pDlg = reinterpret_cast<CIBSU_MultipleScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	if( imageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
		pDlg->PostMessage( WM_USER_BEEP, __BEEP_OK__ );

	LeaveCriticalSection(&g_CriticalSection);
}

/****
 ** This IBSU_CallbackResultImage callback was deprecated since 1.7.0
 ** Please use IBSU_CallbackResultImageEx instead
****/

void CIBSU_MultipleScanSampleForVCDlg::OnEvent_ResultImageEx(
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

	CIBSU_MultipleScanSampleForVCDlg *pDlg = reinterpret_cast<CIBSU_MultipleScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

    if( imageStatus >= IBSU_STATUS_OK )
	{
        pDlg->PostMessage( WM_USER_BEEP, __BEEP_SUCCESS__ );
	}
	else
	{
        pDlg->PostMessage( WM_USER_BEEP, __BEEP_FAIL__ );
	}

	// added 2012-11-30
//	if( pDlg->m_bNeedClearPlaten )
//	{
//		pDlg->m_bNeedClearPlaten = FALSE;
//		pDlg->PostMessage( WM_USER_DRAW_FINGER_QUALITY );
//	}

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

	int devIndex = pDlg->_GetDeviceIndexFromHandle(deviceHandle);

    if( imageStatus >= IBSU_STATUS_OK )
    {
	    // Image acquisition successful
        if( imageStatus == IBSU_STATUS_OK )
        {
	        pDlg->m_strImageMessage[devIndex].Format(_T("%s acquisition completed successfully"), imgTypeName);
	        pDlg->_SetImageMessage(devIndex, pDlg->m_strImageMessage[devIndex]);
	        pDlg->_SetStatusBarMessage(devIndex, pDlg->m_strImageMessage[devIndex]);
        }
        else
        {
            // > IBSU_STATUS_OK
	        pDlg->m_strImageMessage[devIndex].Format(_T("%s acquisition Warning (Warning code = %d)"), imgTypeName, imageStatus);
	        pDlg->_SetImageMessage(devIndex, pDlg->m_strImageMessage[devIndex]);
	        pDlg->_SetStatusBarMessage(devIndex, pDlg->m_strImageMessage[devIndex]);

			pDlg->PostMessage( WM_USER_ASK_RECAPTURE, imageStatus, devIndex );
			LeaveCriticalSection(&g_CriticalSection);
			return;
        }
    }
    else
    {
        // < IBSU_STATUS_OK
	    pDlg->m_strImageMessage[devIndex].Format(_T("%s acquisition failed (Error code = %d)"), imgTypeName, imageStatus);
	    pDlg->_SetImageMessage(devIndex, pDlg->m_strImageMessage[devIndex]);
	    pDlg->_SetStatusBarMessage(devIndex, pDlg->m_strImageMessage[devIndex]);

        // Stop all of acquisition
        pDlg->m_nCurrentCaptureStep[devIndex] = (int)pDlg->m_vecCaptureSeq[devIndex].size();
    }

    pDlg->PostMessage( WM_USER_CAPTURE_SEQ_NEXT, devIndex );

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_MultipleScanSampleForVCDlg::OnEvent_ClearPlatenAtCapture(
                          const int                   deviceHandle,
                          void                        *pContext,
                          const IBSU_PlatenState      platenState
                        )
{
	if( pContext == NULL )
		return;

	CIBSU_MultipleScanSampleForVCDlg *pDlg = reinterpret_cast<CIBSU_MultipleScanSampleForVCDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

/*	if( platenState == ENUM_IBSU_PLATEN_HAS_FINGERS )
		pDlg->m_bNeedClearPlaten = TRUE;
	else
		pDlg->m_bNeedClearPlaten = FALSE;
*/
	int devIndex = pDlg->_GetDeviceIndexFromHandle(deviceHandle);
/*	if( pDlg->m_bNeedClearPlaten )
	{
		pDlg->m_strImageMessage[devIndex].Format(_T("Please remove your fingers on the platen first!"));
		pDlg->_SetImageMessage(devIndex, pDlg->m_strImageMessage[devIndex]);
		pDlg->_SetStatusBarMessage(devIndex, pDlg->m_strImageMessage[devIndex]);
	}
	else
*/	{
		if( pDlg->m_nCurrentCaptureStep[devIndex] > 0 )
		{
			CaptureInfo info = pDlg->m_vecCaptureSeq[devIndex][pDlg->m_nCurrentCaptureStep[devIndex]];

			// Display message for image acuisition again
			CString strMessage;
			strMessage = info.PreCaptureMessage;

			pDlg->_SetStatusBarMessage(devIndex, _T("%s"), strMessage);
			if( !pDlg->m_chkAutoCapture )
				strMessage += _T("\r\nPress button 'Take Result Image' when image is good!");

			pDlg->_SetImageMessage(devIndex, _T("%s"), strMessage);
			pDlg->m_strImageMessage[devIndex] = strMessage;
		}
	}

	pDlg->PostMessage( WM_USER_DRAW_FINGER_QUALITY );

	LeaveCriticalSection(&g_CriticalSection);
}

////////////////////////////////////////////////////////////////////////////////////////////







LRESULT CIBSU_MultipleScanSampleForVCDlg::OnMsg_CaptureSeqStart(WPARAM wParam, LPARAM lParam)
{
	int devIndex = wParam;

	if( m_nDevHandle[devIndex] == -1 )
	{
		OnMsg_UpdateDisplayResources();
		return 0L;
	}

	CString strCaptureSeq;
	int nSelectedSeq;
	nSelectedSeq = m_cboCaptureSeq.GetCurSel();
	if( nSelectedSeq > -1 )
		m_cboCaptureSeq.GetLBText( nSelectedSeq, strCaptureSeq );

	m_vecCaptureSeq[devIndex].clear();
	CaptureInfo info;

/** Please refer to definition below
const TCHAR CAPTURE_SEQ_FLAT_SINGLE_FINGER[]		= _T("Single flat finger");
const TCHAR CAPTURE_SEQ_ROLL_SINGLE_FINGER[]		= _T("Single rolled finger");
const TCHAR CAPTURE_SEQ_2_FLAT_FINGERS[]			= _T("2 flat fingers");
const TCHAR CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS[]	= _T("10 single flat fingers");
const TCHAR CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS[]	= _T("10 single rolled fingers");
*/
	if( strCaptureSeq == CAPTURE_SEQ_FLAT_SINGLE_FINGER )
	{
		info.PreCaptureMessage = _T("Please put a single finger on the sensor!");
		info.PostCaptuerMessage = _T("Keep finger on the sensor!");
		info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
		info.NumberOfFinger = 1;
		info.fingerName = _T("SFF_Unknown");
		m_vecCaptureSeq[devIndex].push_back(info);
	}

	if( strCaptureSeq == CAPTURE_SEQ_ROLL_SINGLE_FINGER )
	{
		info.PreCaptureMessage = _T("Please put a single finger on the sensor!");
		info.PostCaptuerMessage = _T("Roll finger!");
		info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
		info.NumberOfFinger = 1;
		info.fingerName = _T("SRF_Unknown");
		m_vecCaptureSeq[devIndex].push_back(info);
	}

	if( strCaptureSeq == CAPTURE_SEQ_2_FLAT_FINGERS )
	{
		info.PreCaptureMessage = _T("Please put two fingers on the sensor!");
		info.PostCaptuerMessage = _T("Keep fingers on the sensor!");
		info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
		info.NumberOfFinger = 2;
		info.fingerName = _T("TFF_Unknown");
		m_vecCaptureSeq[devIndex].push_back(info);
	}

	if( strCaptureSeq == CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS )
	{
		info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
		info.fingerName = _T("SFF_Right_Thumb");
		info.PostCaptuerMessage = _T("Keep fingers on the sensor!");
		info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
		info.NumberOfFinger = 1;
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put right index on the sensor!");
		info.fingerName = _T("SFF_Right_Index");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put right middle on the sensor!");
		info.fingerName = _T("SFF_Right_Middle");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put right ring on the sensor!");
		info.fingerName = _T("SFF_Right_Ring");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put right little on the sensor!");
		info.fingerName = _T("SFF_Right_Little");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
		info.fingerName = _T("SFF_Left_Thumb");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put left index on the sensor!");
		info.fingerName = _T("SFF_Left_Index");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put left middle on the sensor!");
		info.fingerName = _T("SFF_Left_Middle");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put left ring on the sensor!");
		info.fingerName = _T("SFF_Left_Ring");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put left little on the sensor!");
		info.fingerName = _T("SFF_Left_Little");
		m_vecCaptureSeq[devIndex].push_back(info);
	}

	if( strCaptureSeq == CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS )
	{
		info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
		info.PostCaptuerMessage = _T("Roll finger!");
		info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
		info.NumberOfFinger = 1;
		info.fingerName = _T("SRF_Right_Thumb");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put right index on the sensor!");
		info.fingerName = _T("SRF_Right_Index");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put right middle on the sensor!");
		info.fingerName = _T("SRF_Right_Middle");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put right ring on the sensor!");
		info.fingerName = _T("SRF_Right_Ring");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put right little on the sensor!");
		info.fingerName = _T("SRF_Right_Little");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
		info.fingerName = _T("SRF_Left_Thumb");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put left index on the sensor!");
		info.fingerName = _T("SRF_Left_Index");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put left middle on the sensor!");
		info.fingerName = _T("SRF_Left_Middle");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put left ring on the sensor!");
		info.fingerName = _T("SRF_Left_Ring");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put left little on the sensor!");
		info.fingerName = _T("SRF_Left_Little");
		m_vecCaptureSeq[devIndex].push_back(info);
	}

	if( strCaptureSeq == CAPTURE_SEQ_4_FLAT_FINGERS )
	{
		info.PreCaptureMessage = _T("Please put 4 fingers on the sensor!");
		info.PostCaptuerMessage = _T("Keep fingers on the sensor!");
		info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
		info.NumberOfFinger = 4;
		info.fingerName = _T("4FF_Unknown");
		m_vecCaptureSeq[devIndex].push_back(info);
	}

	if( strCaptureSeq == CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER )
	{
		info.PreCaptureMessage = _T("Please put right 4-fingers on the sensor!");
		info.fingerName = _T("4FF_Right_4_Fingers");
		info.PostCaptuerMessage = _T("Keep fingers on the sensor!");
		info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
		info.NumberOfFinger = 4;
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put left 4-fingers on the sensor!");
		info.fingerName = _T("4FF_Right_4_Fingers");
		m_vecCaptureSeq[devIndex].push_back(info);

		info.PreCaptureMessage = _T("Please put 2-thumbs on the sensor!");
		info.fingerName = _T("TFF_2_Thumbs");
		info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
		info.NumberOfFinger = 2;
		m_vecCaptureSeq[devIndex].push_back(info);
	}

	// Make subfolder name
	SYSTEMTIME localTime;
	GetLocalTime(&localTime);
	m_ImgSubFolder.Format( _T( "%04d-%02d-%02d %02d%02d%02d" ),
                          localTime.wYear, localTime.wMonth, localTime.wDay,
                          localTime.wHour, localTime.wMinute, localTime.wSecond );
  
	PostMessage( WM_USER_CAPTURE_SEQ_NEXT, devIndex );

	return 0L;
}

LRESULT CIBSU_MultipleScanSampleForVCDlg::OnMsg_CaptureSeqNext(WPARAM wParam, LPARAM lParam)
{
	int nRc, devIndex;

	devIndex = wParam;
	if( m_nDevHandle[devIndex] == -1 )
		return 0L;

	m_nCurrentCaptureStep[devIndex]++;
	if( m_nCurrentCaptureStep[devIndex] >= (int)m_vecCaptureSeq[devIndex].size() )
	{
		// All of capture sequence completely
		m_nCurrentCaptureStep[devIndex] = -1;
		m_ImgSubFolder = _T("");

		OnMsg_UpdateDisplayResources();
		GetDlgItem(IDC_BTN_CAPTURE_START)->SetFocus();
		return 0L;
	}

	// Make capture delay for display result image on multi capture mode (500 ms)
	if( m_nCurrentCaptureStep[devIndex] > 0 )
	{
		Sleep(500);
		m_strImageMessage[devIndex] = _T( "" );
	}

	CaptureInfo info = m_vecCaptureSeq[devIndex][m_nCurrentCaptureStep[devIndex]];

	IBSU_ImageResolution imgRes = ENUM_IBSU_IMAGE_RESOLUTION_500;
	BOOL bAvailable = FALSE;
	nRc = IBSU_IsCaptureAvailable(m_nDevHandle[devIndex], info.ImageType, imgRes, &bAvailable);
	if( nRc != IBSU_STATUS_OK || !bAvailable )
	{
		_SetStatusBarMessage(devIndex, _T("The capture mode (%d) is not available"), info.ImageType);
		m_nCurrentCaptureStep[devIndex] = -1;
		OnMsg_UpdateDisplayResources();
		return 0L;
	}

	// Start capture
	DWORD captureOptions = 0;
	if( m_chkAutoContrast )
		captureOptions |= IBSU_OPTION_AUTO_CONTRAST;
	if( m_chkAutoCapture )
		captureOptions |= IBSU_OPTION_AUTO_CAPTURE;
	if( m_chkIgnoreFingerCount )
		captureOptions |= IBSU_OPTION_IGNORE_FINGER_COUNT;

	nRc = IBSU_BeginCaptureImage(m_nDevHandle[devIndex], info.ImageType, imgRes, captureOptions);
	if( nRc >= IBSU_STATUS_OK )
	{
		// Display message for image acuisition
		CString strMessage;
		strMessage = info.PreCaptureMessage;

		_SetStatusBarMessage(devIndex, _T("%s"), strMessage);
		if( !m_chkAutoCapture )
			strMessage += _T("\r\nPress button 'Take Result Image' when image is good!");

		_SetImageMessage(devIndex, _T("%s"), strMessage);
		m_strImageMessage[devIndex] = strMessage;
	}
	else
	{
		_SetStatusBarMessage(devIndex, _T("Failed to execute IBSU_BeginCaptureImage()"));
	}

	OnMsg_UpdateDisplayResources();

	return 0L;
}

LRESULT CIBSU_MultipleScanSampleForVCDlg::OnMsg_DeviceCommunicationBreak(WPARAM wParam, LPARAM lParam)
{
	int nRc;
	int devIndex = _GetDeviceIndexFromHandle((int)wParam);

	if( m_nDevHandle[devIndex] == -1 )
		return 0L;

//	_SetStatusBarMessage( devIndex, _T( "Device communication was broken" ) );

	nRc = _ReleaseDevice(devIndex);
	if( nRc == IBSU_ERR_RESOURCE_LOCKED )
	{
		// retry to release device
		PostMessage( WM_USER_DEVICE_COMMUNICATION_BREAK, devIndex );
	}
	else
	{
		PostMessage( WM_USER_BEEP, __BEEP_DEVICE_COMMUNICATION_BREAK__ );
		PostMessage( WM_USER_UPDATE_DEVICE_LIST );
		m_nDevHandle[devIndex] = -1;
	}

	return 0L;
}

LRESULT CIBSU_MultipleScanSampleForVCDlg::OnMsg_UpdateDeviceList(WPARAM wParam, LPARAM lParam)
{
	int nPrevListCount;
	int checkedPos = -1;
	CString strDevice;

	nPrevListCount = m_listUsbDevices.GetCount();

	UpdateData(FALSE);
	m_listUsbDevices.ResetContent();

	for( int i=0; i<nPrevListCount; i++ )
	{
		if( m_listUsbDevices.GetCheck(i) )
		{
			checkedPos = i;
			break;
		}
	}
	bool idle = true;
	if( checkedPos >= 0 )
		idle = !m_bInitializing[checkedPos] && ( m_nCurrentCaptureStep[checkedPos] == -1 );
	if( idle )
	{
		GetDlgItem( IDC_BTN_CAPTURE_STOP )->EnableWindow( FALSE );
		GetDlgItem( IDC_BTN_CAPTURE_START )->EnableWindow( FALSE );
	}

	int devices = 0;
	IBSU_GetDeviceCount( &devices );
//	devices = ( devices > MAX_DEVICE_COUNT )? MAX_DEVICE_COUNT: devices;
	devices = MAX_DEVICE_COUNT;
	for( int i=0; i<devices; i++ )
	{
		m_nDevHandle[i] = -1;
		IBSU_DeviceDesc devDesc;
		int nRc = IBSU_GetDeviceDescription( i, &devDesc );
		if( nRc == IBSU_ERR_DEVICE_NOT_FOUND )
		{
			_SetStatusBarMessage( i, _T( "" ) );
			_SetImageMessage(i, _T( "" ));
			continue;
		}
		else if( nRc < IBSU_STATUS_OK )
			strDevice = _T( "unknown device" );

		strDevice.Format( _T( "%s_v%s (%s)" ), devDesc.productName, devDesc.fwVersion, devDesc.serialNumber );

		m_listUsbDevices.AddString( strDevice );
		if( devDesc.IsHandleOpened )
		{
			m_nDevHandle[i] = devDesc.handle;
			m_listUsbDevices.SetCheck(i, 1);

			CWnd *disWnd;

			if( i == 0 )
				disWnd = GetDlgItem( IDC_FRAME_IMAGE_1 );
			else if( i == 1 )
				disWnd = GetDlgItem( IDC_FRAME_IMAGE_2 );
			else if( i == 2 )
				disWnd = GetDlgItem( IDC_FRAME_IMAGE_3 );
			else if( i == 3 )
				disWnd = GetDlgItem( IDC_FRAME_IMAGE_4 );

			RECT clientRect;
		    disWnd->GetClientRect( &clientRect );

			// create display window
			IBSU_CreateClientWindow( m_nDevHandle[i], disWnd->m_hWnd, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom );
		}
		else
		{
			m_nDevHandle[i] = -1;
			m_nCurrentCaptureStep[i] = -1;
			m_bInitializing[i] = false;

			_SetStatusBarMessage( i, _T( "" ) );
			_SetImageMessage(i, _T( "" ));
		}
	}
/*
	for( int i=devices; i<nPrevListCount; i++ )
	{
		m_nDevHandle[i] = -1;
		m_nCurrentCaptureStep[i] = -1;
		m_bInitializing[i] = false;
	}
*/
	if( idle )
	{
//		OnCbnSelchangeComboDevices();
		_UpdateCaptureSequences();
	}

	return 0L;
}

LRESULT CIBSU_MultipleScanSampleForVCDlg::OnMsg_InitWarning(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CIBSU_MultipleScanSampleForVCDlg::OnMsg_UpdateDisplayResources(WPARAM wParam, LPARAM lParam)
{
	int devIndex = 0;

	for( int i=0; i<m_nDeviceCount; i++ )
	{
		if( m_listUsbDevices.GetCheck(i) > 0 )
		{
			devIndex = i;
			break;
		}
	}

	BOOL selectedDev = m_listUsbDevices.GetCheck(devIndex) > 0;
	BOOL captureSeq = m_cboCaptureSeq.GetCurSel() > 0;
	BOOL idle = !m_bInitializing[devIndex] && ( m_nCurrentCaptureStep[devIndex] == -1 );
	BOOL active = !m_bInitializing[devIndex] && (m_nCurrentCaptureStep[devIndex] != -1 );
	BOOL uninitializedDev = selectedDev && ( m_nDevHandle[devIndex] == -1 );


//	m_cboUsbDevices.EnableWindow( idle );  
	m_cboCaptureSeq.EnableWindow( selectedDev && idle );

	GetDlgItem( IDC_BTN_CAPTURE_START )->EnableWindow( captureSeq );
	GetDlgItem( IDC_BTN_CAPTURE_STOP )->EnableWindow( active );

	GetDlgItem( IDC_CHECK_AUTO_CONTRAST )->EnableWindow( selectedDev && idle );
	GetDlgItem( IDC_CHECK_AUTO_CAPTURE )->EnableWindow( selectedDev && idle );
	GetDlgItem( IDC_CHECK_IGNORE_FINGER_COUNT )->EnableWindow( selectedDev && idle );
	GetDlgItem( IDC_CHECK_SAVE_IMAGES )->EnableWindow( selectedDev && idle );
	GetDlgItem( IDC_BTN_IMAGE_FOLDER )->EnableWindow( selectedDev && idle );

//	GetDlgItem( IDC_CHECK_CLEAR_PLATEN )->EnableWindow( uninitializedDev );

	CString strCaption = _T( "" );
	if( active )
		strCaption = _T( "Take Result Image" );  
	else if( !active && !m_bInitializing[devIndex] )
		strCaption = "Start";  

	SetDlgItemText( IDC_BTN_CAPTURE_START, strCaption );
	return 0;
}

LRESULT CIBSU_MultipleScanSampleForVCDlg::OnMsg_UpdateStatusMessage(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CIBSU_MultipleScanSampleForVCDlg::OnMsg_Beep(WPARAM wParam, LPARAM lParam)
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

LRESULT CIBSU_MultipleScanSampleForVCDlg::OnMsg_AskRecapture(WPARAM wParam, LPARAM lParam)
{
	int imageStatus = (int)wParam;
	int devIndex = (int)lParam;
    CString askMsg;

    askMsg.Format("[Warning = %d] Do you want a recapture?", imageStatus);
	if( AfxMessageBox(askMsg, MB_YESNO | MB_ICONINFORMATION) == IDYES )
    {
        // To recapture current finger position
        m_nCurrentCaptureStep[devIndex]--;
    }

    PostMessage( WM_USER_CAPTURE_SEQ_NEXT );

	return 0L;
}










void CIBSU_MultipleScanSampleForVCDlg::OnBnClickedBtnCaptureStart()
{
	UpdateData();

	int deviceCount = 0;
	BOOL bCapturing = FALSE;

//	IBSU_GetDeviceCount(&deviceCount);
//	if( deviceCount > MAX_DEVICE_COUNT )
		deviceCount = MAX_DEVICE_COUNT;

	for( int devIndex=0; devIndex<deviceCount; devIndex++ )
	{
		if( m_bInitializing[devIndex] )
			continue;

		if( m_listUsbDevices.GetCheck(devIndex) )
		{
			if( m_nCurrentCaptureStep[devIndex] != -1 )
			{
				BOOL IsActive;
				int nRc;
				nRc = IBSU_IsCaptureActive(m_nDevHandle[devIndex], &IsActive);
				if( nRc == IBSU_STATUS_OK && IsActive )
				{
					IBSU_TakeResultImageManually(m_nDevHandle[devIndex]);
				}

				continue;
			}

			if( m_nDevHandle[devIndex] == -1 )
			{
				m_bInitializing[devIndex] = true;

				ThreadParam param;
				param.threadStarted = CreateEvent( NULL, FALSE, FALSE, NULL );
				param.devIndex = devIndex;
				param.pParentApp = this;
				m_pInitialThread = ::AfxBeginThread( _InitializeDeviceThreadCallback, &param, THREAD_PRIORITY_NORMAL,
										  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION );

				if( !m_pInitialThread )
					return;

				m_pInitialThread->m_bAutoDelete = TRUE;
				WaitForSingleObject( param.threadStarted, 1000 );
			}
			else
			{
				// device already initialized
				PostMessage(WM_USER_CAPTURE_SEQ_START, devIndex);
			}
		}
	}

	OnMsg_UpdateDisplayResources();
}

void CIBSU_MultipleScanSampleForVCDlg::OnBnClickedBtnCaptureStop()
{
	int deviceCount = 0;
//	IBSU_GetDeviceCount(&deviceCount);
//	if( deviceCount > MAX_DEVICE_COUNT )
		deviceCount = MAX_DEVICE_COUNT;

	for( int devIndex=0; devIndex<deviceCount; devIndex++ )
	{
		if( m_nDevHandle[devIndex] == -1 )
			continue;

		IBSU_CancelCaptureImage( m_nDevHandle[devIndex] );
		m_nCurrentCaptureStep[devIndex] = -1;

		_SetStatusBarMessage( devIndex, _T( "Capture Sequence aborted" ) );
		m_strImageMessage[devIndex] = _T( "" );
		_SetImageMessage(devIndex, _T( "" ));
	}

	OnMsg_UpdateDisplayResources();
}

void CIBSU_MultipleScanSampleForVCDlg::OnBnClickedBtnImageFolder()
{
	LPITEMIDLIST  pidlSelected;
	BROWSEINFO    bi = {0};
	LPMALLOC      pMalloc;

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
	}
	pMalloc->Release();

    if(IBSU_IsWritableDirectory(m_ImgSaveFolder, TRUE) != IBSU_STATUS_OK)
	{
		AfxMessageBox("You don't have writing permission on this folder\r\nPlease select another folder (e.g. desktop folder)");
	}
}
/*
void CIBSU_MultipleScanSampleForVCDlg::OnCbnSelchangeComboDevices()
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

	_UpdateCaptureSequences();

	AfxGetApp()->DoWaitCursor( -1 );
}
*/
void CIBSU_MultipleScanSampleForVCDlg::OnCbnSelchangeComboCaptureSeq()
{
	OnMsg_UpdateDisplayResources();
}

void CIBSU_MultipleScanSampleForVCDlg::OnNMReleasedcaptureSliderContrast(NMHDR *pNMHDR, LRESULT *pResult)
{
	if( m_chkAutoContrast )
		return;

	int deviceCount;
	IBSU_GetDeviceCount(&deviceCount);

	for( int i=0; i<deviceCount; i++ )
	{
		if( m_nDevHandle[i] == -1 )
			continue;

		int pos = m_sliderContrast.GetPos();
		char cValue[5];
		sprintf(cValue, "%d", pos);
		GetDlgItem(IDC_EDIT_CONTRAST)->SetWindowText(cValue);
		IBSU_SetContrast(m_nDevHandle[i], pos);
	}

	*pResult = 0;
}

void CIBSU_MultipleScanSampleForVCDlg::OnNMCustomdrawSliderContrast(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	if( m_chkAutoContrast )
		return;

	int deviceCount;
	IBSU_GetDeviceCount(&deviceCount);

	for( int i=0; i<deviceCount; i++ )
	{
		if( m_nDevHandle[i] == -1 )
			continue;

		int pos = m_sliderContrast.GetPos();
		char cValue[5];
		sprintf(cValue, "%d", pos);
		GetDlgItem(IDC_EDIT_CONTRAST)->SetWindowText(cValue);
		IBSU_SetContrast(m_nDevHandle[i], pos);
	}

	*pResult = 0;
}

void CIBSU_MultipleScanSampleForVCDlg::OnBnClickedCheckAutoContrast()
{
	UpdateData(TRUE);

	if( m_chkAutoContrast )
	{
		GetDlgItem(IDC_SLIDER_CONTRAST)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_CONTRAST)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_CONTRAST)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_SLIDER_CONTRAST)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_CONTRAST)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_CONTRAST)->EnableWindow(TRUE);
	}
}
/*
void CIBSU_MultipleScanSampleForVCDlg::OnBnClickedCheckClientWindow()
{
	if( m_nDevHandle == -1 )
		return;

	UpdateData(TRUE);
	if( m_chkUseClientWindow )
	{
		CWnd *disWnd = GetDlgItem( IDC_FRAME_IMAGE );
		RECT clientRect;
		disWnd->GetClientRect( &clientRect );
		IBSU_CreateClientWindow(m_nDevHandle, disWnd->m_hWnd, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
	}
	else
	{
		IBSU_DestroyClientWindow(m_nDevHandle, FALSE);
	}
}
*/

void CIBSU_MultipleScanSampleForVCDlg::OnCheckchangeListDevices() 
{
	_UpdateCaptureSequences();
}

HBRUSH CIBSU_MultipleScanSampleForVCDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

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
			font_size = -MulDiv(14, GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
			if( fFont.CreateFont(font_size, 0, 0, 0, FW_NORMAL, 0, 0, 0,DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, ANTIALIASED_QUALITY,
				DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif") != NULL )
			{
				fOldFont = pDC->SelectObject(&fFont);
				pDC->DrawText(szText, rect, DT_LEFT);
			}
		}
		break;
	}

	return hbr;
}

void CIBSU_MultipleScanSampleForVCDlg::OnClose()
{
	int nRc;

	for( int j=0; j<10; j++ )
	{
		nRc = IBSU_CloseAllDevice();
		if( nRc != IBSU_ERR_RESOURCE_LOCKED )
			break;
		Sleep(100);
	}

	CDialog::OnClose();
}
