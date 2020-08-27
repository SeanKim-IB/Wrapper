
// IBScanUltimate_SampleForVCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IBSU_NonCallbackSampleForVC.h"
#include "IBSU_NonCallbackSampleForVCDlg.h"

#include "IBScanUltimateApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_POOLING_DEVICE				1
#define TIMER_POOLING_DELAY					50

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


// Beep definitions
const int __BEEP_FAIL__							= 0;
const int __BEEP_SUCCESS__						= 1;
const int __BEEP_OK__							= 2;
const int __BEEP_DEVICE_COMMUNICATION_BREAK__	= 3;

// LED color definitions
const int __LED_COLOR_NONE__	= 0;
const int __LED_COLOR_GREEN__	= 1;
const int __LED_COLOR_RED__		= 2;
const int __LED_COLOR_YELLOW__	= 3;

// Key button definitions
const int __LEFT_KEY_BUTTON__	= 1;
const int __RIGHT_KEY_BUTTON__	= 2;

const CBrush   CIBSU_NonCallbackSampleForVCDlg::s_brushRed( RGB(255,0,0) );
const CBrush   CIBSU_NonCallbackSampleForVCDlg::s_brushOrange( RGB(255,128,0) );
const CBrush   CIBSU_NonCallbackSampleForVCDlg::s_brushGreen( RGB(0,128,0) );
const CBrush   CIBSU_NonCallbackSampleForVCDlg::s_brushPlaten( RGB(78,78,78) );


// Use critical section
CRITICAL_SECTION g_CriticalSection;


// CIBSU_NonCallbackSampleForVCDlg dialog
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

CIBSU_NonCallbackSampleForVCDlg::CIBSU_NonCallbackSampleForVCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIBSU_NonCallbackSampleForVCDlg::IDD, pParent)
	, m_chkAutoContrast(TRUE)
	, m_chkAutoCapture(TRUE)
	, m_chkIgnoreFingerCount(FALSE)
	, m_chkSaveImages(FALSE)
	, m_chkUseClearPlaten(TRUE)
	, m_chkUseNFIQ(TRUE)
	, m_chkDrawSegmentImage(TRUE)
	, m_chkInvalidArea(FALSE)
	, m_chkDetectSmear(TRUE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_nSelectedDevIndex = -1;

	m_BpollingTimeGetImage = 0;

	for(int i=0; i<4; i++)
		m_FingerQuality[i] = ENUM_IBSU_FINGER_NOT_PRESENT;

	InitializeCriticalSection(&g_CriticalSection);
}

CIBSU_NonCallbackSampleForVCDlg::~CIBSU_NonCallbackSampleForVCDlg()
{
	DeleteCriticalSection(&g_CriticalSection);
}

void CIBSU_NonCallbackSampleForVCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DEVICES, m_cboUsbDevices);
	DDX_Control(pDX, IDC_COMBO_CAPTURE_SEQ, m_cboCaptureSeq);
	DDX_Check(pDX, IDC_CHECK_AUTO_CONTRAST, m_chkAutoContrast);
	DDX_Check(pDX, IDC_CHECK_AUTO_CAPTURE, m_chkAutoCapture);
	DDX_Check(pDX, IDC_CHECK_IGNORE_FINGER_COUNT, m_chkIgnoreFingerCount);
	DDX_Check(pDX, IDC_CHECK_SAVE_IMAGES, m_chkSaveImages);
	DDX_Control(pDX, IDC_FRAME_IMAGE, m_frameImage);
	DDX_Control(pDX, IDC_SLIDER_CONTRAST, m_sliderContrast);
	DDX_Control(pDX, IDC_PIC_SCANNER, m_QualityView);
	DDX_Check(pDX, IDC_CHECK_CLEAR_PLATEN, m_chkUseClearPlaten);
	DDX_Control(pDX, IDC_PIC_IB_LOGO, m_IBLogo);
	DDX_Check(pDX, IDC_CHECK_NFIQ, m_chkUseNFIQ);
	DDX_Check(pDX, IDC_CHECK_DRAW_SEGMENT_IMAGE, m_chkDrawSegmentImage);
	DDX_Check(pDX, IDC_CHECK_INVALID_AREA, m_chkInvalidArea);
	DDX_Check(pDX, IDC_CHECK_SMEAR, m_chkDetectSmear);
	DDX_Control(pDX, IDC_COMBO_SMEAR_LEVEL, m_cboSmearLevel);
}

BEGIN_MESSAGE_MAP(CIBSU_NonCallbackSampleForVCDlg, CDialog)
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
	ON_MESSAGE(WM_USER_DRAW_FINGER_QUALITY,			OnMsg_DrawFingerQuality)

	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_CAPTURE_START, &CIBSU_NonCallbackSampleForVCDlg::OnBnClickedBtnCaptureStart)
	ON_BN_CLICKED(IDC_BTN_CAPTURE_STOP, &CIBSU_NonCallbackSampleForVCDlg::OnBnClickedBtnCaptureStop)
	ON_BN_CLICKED(IDC_BTN_IMAGE_FOLDER, &CIBSU_NonCallbackSampleForVCDlg::OnBnClickedBtnImageFolder)
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICES, &CIBSU_NonCallbackSampleForVCDlg::OnCbnSelchangeComboDevices)
	ON_CBN_SELCHANGE(IDC_COMBO_CAPTURE_SEQ, &CIBSU_NonCallbackSampleForVCDlg::OnCbnSelchangeComboCaptureSeq)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_CONTRAST, &CIBSU_NonCallbackSampleForVCDlg::OnNMReleasedcaptureSliderContrast)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CONTRAST, &CIBSU_NonCallbackSampleForVCDlg::OnNMCustomdrawSliderContrast)
	ON_BN_CLICKED(IDC_CHECK_AUTO_CONTRAST, &CIBSU_NonCallbackSampleForVCDlg::OnBnClickedCheckAutoContrast)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHECK_CLEAR_PLATEN, &CIBSU_NonCallbackSampleForVCDlg::OnBnClickedCheckClearPlaten)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CIBSU_NonCallbackSampleForVCDlg message handlers

BOOL CIBSU_NonCallbackSampleForVCDlg::OnInitDialog()
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

	m_nDevHandle = -1;
	m_nCurrentCaptureStep = -1;
	m_bInitializing = FALSE;
	m_strImageMessage = _T( "" );
	m_bNeedClearPlaten = FALSE;

	m_ImgSaveFolder[0] = 0;
	SHGetSpecialFolderPath( NULL, m_ImgSaveFolder, CSIDL_MYPICTURES, TRUE );

	m_sliderContrast.SetRange(0, 34);
	m_sliderContrast.SetPos(0);
	m_sliderContrast.SetTicFreq(5);
	GetDlgItem(IDC_EDIT_CONTRAST)->SetWindowText("0");

	IBSU_GetSDKVersion(&m_verInfo);
	CString titleName;
	titleName.Format("NonCallbackSample for VC++");
	SetWindowText(titleName);

	CString strDrawString;
	strDrawString.Format("VC++ non-callback sample with DLL ver. %s", m_verInfo.Product);
	GetDlgItem(IDC_STATIC_DLL_VER)->SetWindowText(strDrawString);

	strDrawString = _T("Copyright (c) Integrated Biometrics");
	GetDlgItem(IDC_STATIC_COPYRIGHT)->SetWindowText(strDrawString);

	m_cboSmearLevel.ResetContent();
	m_cboSmearLevel.AddString( _T( "LOW" ) );
	m_cboSmearLevel.AddString( _T( "MEDIUM" ) );
	m_cboSmearLevel.AddString( _T( "HIGH" ) );
	m_cboSmearLevel.SetCurSel(1);

	IBSU_GetDeviceCount(&m_nDeviceCount);
	m_nPollingCount = 0;

	SetTimer( TIMER_POOLING_DEVICE, TIMER_POOLING_DELAY, NULL );
	PostMessage( WM_USER_UPDATE_DEVICE_LIST );


	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIBSU_NonCallbackSampleForVCDlg::OnPaint()
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
HCURSOR CIBSU_NonCallbackSampleForVCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


////////////////////////////////////////////////////////////////////////////////////////////
// User defined functions

void CIBSU_NonCallbackSampleForVCDlg::_SetStatusBarMessage( LPCTSTR Format, ... )
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

void CIBSU_NonCallbackSampleForVCDlg::_SetImageMessage( LPCSTR Format, ... )
{
	TCHAR cMessage[IBSU_MAX_STR_LEN+1];
	va_list arg_ptr;

	va_start( arg_ptr, Format );
	_vsntprintf( cMessage, IBSU_MAX_STR_LEN, Format, arg_ptr );
	va_end( arg_ptr );

	int			font_size = 10;
	int			x = 10;
	int			y = 10;
	COLORREF	cr = RGB(0, 0, 255);

    if( m_bNeedClearPlaten )
        cr = RGB(255, 0, 0);

    // IBSU_SetClientWindowOverlayText was deprecated since 1.7.0
    // Please use the function IBSU_AddOverlayText and IBSU_ModifyOverlayText instead
	IBSU_ModifyOverlayText( m_nDevHandle, m_nOvImageTextHandle, "Arial", font_size, TRUE/*bold*/, cMessage, x, y, (DWORD)cr );
}

void CIBSU_NonCallbackSampleForVCDlg::_UpdateCaptureSequences()
{
	// store currently selected sequence
	CString strSelectedText;
	int selectedSeq = m_cboCaptureSeq.GetCurSel();
	if( selectedSeq > -1 )
		m_cboCaptureSeq.GetLBText( selectedSeq, strSelectedText );

	// populate combo box
	m_cboCaptureSeq.ResetContent();
	m_cboCaptureSeq.AddString( _T( "- Please select -" ) );

	const int devIndex = m_cboUsbDevices.GetCurSel() - 1;
	IBSU_DeviceDesc devDesc;
	devDesc.productName[0] = 0;  
	if( devIndex > -1 )
		IBSU_GetDeviceDescription( devIndex, &devDesc );

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
	else if( ( _stricmp( devDesc.productName, "HOLMES" )		== 0) ||
		     ( _stricmp( devDesc.productName, "KOJAK" )	== 0) ||
			 ( _stricmp( devDesc.productName, "FIVE-0" )			== 0) )
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

int CIBSU_NonCallbackSampleForVCDlg::_ReleaseDevice()
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

void CIBSU_NonCallbackSampleForVCDlg::_BeepFail()
{
	IBSU_BeeperType beeperType;
    if (IBSU_GetOperableBeeper(m_nDevHandle, &beeperType) != IBSU_STATUS_OK)
    {
		Beep( 3500, 300 );
		Sleep(150);
		Beep( 3500, 150 );
		Sleep(150);
		Beep( 3500, 150 );
		Sleep(150);
		Beep( 3500, 150 );
	}
	else
    {
   	    IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2/*Sol*/, 12/*300ms = 12*25ms*/, 0, 0);
	    Sleep(150);
   	    IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2/*Sol*/, 6/*150ms = 6*25ms*/, 0, 0);
	    Sleep(150);
   	    IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2/*Sol*/, 6/*150ms = 6*25ms*/, 0, 0);
	    Sleep(150);
   	    IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2/*Sol*/, 6/*150ms = 6*25ms*/, 0, 0);
    }
}

void CIBSU_NonCallbackSampleForVCDlg::_BeepSuccess()
{
	IBSU_BeeperType beeperType;
    if (IBSU_GetOperableBeeper(m_nDevHandle, &beeperType) != IBSU_STATUS_OK)
    {
		Beep( 3500, 100 );
		Sleep(50);
		Beep( 3500, 100 );
	}
	else
    {
   	    IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2/*Sol*/, 4/*100ms = 4*25ms*/, 0, 0);
	    Sleep(150);
   	    IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2/*Sol*/, 4/*100ms = 4*25ms*/, 0, 0);
    }
}

void CIBSU_NonCallbackSampleForVCDlg::_BeepOk()
{
	IBSU_BeeperType beeperType;
    if (IBSU_GetOperableBeeper(m_nDevHandle, &beeperType) != IBSU_STATUS_OK)
    {
		Beep( 3500, 100 );
	}
	else
    {
   	    IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2/*Sol*/, 4/*100ms = 4*25ms*/, 0, 0);
    }
}

void CIBSU_NonCallbackSampleForVCDlg::_BeepDeviceCommunicationBreak()
{
	for( int i=0; i<8; i++ )
	{
		Beep( 3500, 100 );
		Sleep( 100 );
	}

	//IBSU_ImageData &image;
}

void CIBSU_NonCallbackSampleForVCDlg::_SaveBitmapImage( const IBSU_ImageData &image, const CString &fingerName ) 
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

	int countZ = sizeof(IBSU_ImageData);
	if( IBSU_SaveBitmapImage( strFileName, (BYTE*)image.Buffer, 
							  image.Width, image.Height, image.Pitch, 
							  image.ResolutionX, image.ResolutionY ) != IBSU_STATUS_OK )
	{
		AfxMessageBox( _T( "Failed to save bitmap image!" ));
	}
}

void CIBSU_NonCallbackSampleForVCDlg::_SaveWsqImage( const IBSU_ImageData &image, const CString &fingerName ) 
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

	// Bug Fixed, WSQ image was flipped vertically.
	// Pitch parameter is required to fix bug.
	if( IBSU_WSQEncodeToFile( strFileName, (BYTE*)image.Buffer,
							  image.Width, image.Height, image.Pitch, image.BitsPerPixel,
							  (int)image.ResolutionX, (double)0.75, "" ) != IBSU_STATUS_OK )
	{
		AfxMessageBox( _T( "Failed to save WSQ image!" ));
	}
}

void CIBSU_NonCallbackSampleForVCDlg::_SavePngImage( const IBSU_ImageData &image, const CString &fingerName ) 
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
	strFileName.Format( _T( "%s\\Image_%02d_%s.png" ), strFolder, m_nCurrentCaptureStep, fingerName );

	if( IBSU_SavePngImage( strFileName, (BYTE*)image.Buffer, 
						   image.Width, image.Height, image.Pitch, 
						   image.ResolutionX, image.ResolutionY ) != IBSU_STATUS_OK )
	{
		AfxMessageBox( _T( "Failed to save png image!" ));
	}

}

void CIBSU_NonCallbackSampleForVCDlg::_SaveJP2Image( const IBSU_ImageData &image, const CString &fingerName ) 
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
	strFileName.Format( _T( "%s\\Image_%02d_%s.jp2" ), strFolder, m_nCurrentCaptureStep, fingerName );

	if( IBSU_SaveJP2Image( strFileName, (BYTE*)image.Buffer, 
						   image.Width, image.Height, image.Pitch, 
						   image.ResolutionX, image.ResolutionY, 80 ) != IBSU_STATUS_OK )
	{
		AfxMessageBox( _T( "Failed to save jpeg-2000 image!" ));
	}

}

int CIBSU_NonCallbackSampleForVCDlg::_WaitingForFinishInitDevice( const int devIndex )
{
	int		nRc;
	BOOL	IsComplete;
	int		devHandle;
	int		progressValue;
    int     ledCount;
    DWORD   operableLEDs;

	while( TRUE )
	{
		nRc = IBSU_BGetInitProgress(devIndex, &IsComplete, &devHandle, &progressValue);
		if( nRc != IBSU_STATUS_OK || IsComplete )
			break;
		_SetStatusBarMessage(_T("Initializing device... %d%%"), progressValue);
		Sleep(50);
	}

	if( nRc == IBSU_STATUS_OK )
	{
		m_nDevHandle = devHandle;

        IBSU_GetOperableLEDs(devHandle, &m_LedType, &ledCount, &operableLEDs);

		char	propertyValue[128];

		IBSU_GetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_POLLINGTIME_TO_BGETIMAGE, propertyValue);
		m_BpollingTimeGetImage = atoi(propertyValue);
		    
		CWnd *disWnd = GetDlgItem( IDC_FRAME_IMAGE );
		RECT clientRect;
		disWnd->GetClientRect( &clientRect );

		// create display window
		IBSU_CreateClientWindow( devHandle, disWnd->m_hWnd, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom );

		IBSU_AddOverlayQuadrangle(m_nDevHandle, &m_nOvClearPlatenHandle, 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);
		IBSU_AddOverlayText(m_nDevHandle, &m_nOvImageTextHandle, "Arial", 10, TRUE/*bold*/, "", 10, 10, (DWORD)0);
		for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
			IBSU_AddOverlayQuadrangle(m_nDevHandle, &m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);


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

void CIBSU_NonCallbackSampleForVCDlg::_SetLEDs(int deviceHandle, CaptureInfo info, int ledColor, BOOL bBlink)
{
	DWORD setLEDs = 0;

    if (m_LedType == ENUM_IBSU_LED_TYPE_FSCAN)
    {
	    if( bBlink )
	    {
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_BLINK_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_BLINK_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_BLINK_GREEN;
			    setLEDs |= IBSU_LED_F_BLINK_RED;
		    }
	    }

        if( info.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
        {
            setLEDs |= IBSU_LED_F_PROGRESS_ROLL;
        }

	    if( (_tcsicmp(info.fingerName, "SFF_Right_Thumb")			== 0) ||
		    (_tcsicmp(info.fingerName, "SRF_Right_Thumb")			== 0) )
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_THUMB_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_THUMB_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_THUMB_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_THUMB_RED;
		    }
	    }
	    else if( (_tcsicmp(info.fingerName, "SFF_Left_Thumb")		== 0) ||
		    (_tcsicmp(info.fingerName, "SRF_Left_Thumb")			== 0) )
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_THUMB_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_THUMB_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_THUMB_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_THUMB_RED;
		    }
	    }
	    else if( (_tcsicmp(info.fingerName, "TFF_2_Thumbs")			== 0) )
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_THUMB_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_THUMB_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_THUMB_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_THUMB_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_THUMB_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_THUMB_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_THUMB_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_THUMB_RED;
		    }
	    }
	    ///////////////////LEFT HAND////////////////////
	    else if( (_tcsicmp(info.fingerName, "SFF_Left_Index")		== 0) ||
		    (_tcsicmp(info.fingerName, "SRF_Left_Index")			== 0) )
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
		    }
	    }
	    else if( (_tcsicmp(info.fingerName, "SFF_Left_Middle")		== 0) ||
		    (_tcsicmp(info.fingerName, "SRF_Left_Middle")			== 0) )
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
		    }
	    }
	    else if( (_tcsicmp(info.fingerName, "SFF_Left_Ring")		== 0) ||
		    (_tcsicmp(info.fingerName, "SRF_Left_Ring")				== 0) )
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_RING_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_RING_RED;
		    }
	    }
	    else if( (_tcsicmp(info.fingerName, "SFF_Left_Little")		== 0) ||
		    (_tcsicmp(info.fingerName, "SRF_Left_Little")			== 0) )
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
		    }
	    }
	    else if( (_tcsicmp(info.fingerName, "4FF_Left_4_Fingers")	== 0) )
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
			    setLEDs |= IBSU_LED_F_LEFT_RING_RED;
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
			    setLEDs |= IBSU_LED_F_LEFT_RING_RED;
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
		    }
	    }
	    ///////////RIGHT HAND /////////////////////////
	    else if( (_tcsicmp(info.fingerName, "SFF_Right_Index")		== 0) ||
		    (_tcsicmp(info.fingerName, "SRF_Right_Index")			== 0) )
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
		    }
	    }
	    else if( (_tcsicmp(info.fingerName, "SFF_Right_Middle")		== 0) ||
		    (_tcsicmp(info.fingerName, "SRF_Right_Middle")			== 0) )
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
		    }
	    }
	    else if( (_tcsicmp(info.fingerName, "SFF_Right_Ring")		== 0) ||
		    (_tcsicmp(info.fingerName, "SRF_Right_Ring")			== 0) )
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
		    }
	    }
	    else if( (_tcsicmp(info.fingerName, "SFF_Right_Little")		== 0) ||
		    (_tcsicmp(info.fingerName, "SRF_Right_Little")			== 0) )
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
		    }
	    }
	    else if( (_tcsicmp(info.fingerName, "4FF_Right_4_Fingers")	== 0) )
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
		    }
	    }

        if (ledColor == __LED_COLOR_NONE__)
        {
            setLEDs = 0;
        }

	    IBSU_SetLEDs(deviceHandle, setLEDs);
    }
}





////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CIBSU_NonCallbackSampleForVCDlg::OnMsg_CaptureSeqStart(WPARAM wParam, LPARAM lParam)
{
	if( m_nDevHandle == -1 )
	{
		OnMsg_UpdateDisplayResources();
		return 0L;
	}

	CString strCaptureSeq;
	int nSelectedSeq;
	nSelectedSeq = m_cboCaptureSeq.GetCurSel();
	if( nSelectedSeq > -1 )
		m_cboCaptureSeq.GetLBText( nSelectedSeq, strCaptureSeq );

	m_vecCaptureSeq.clear();
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
		info.fingerPosition = IBSM_FINGER_POSITION_UNKNOWN;
		m_vecCaptureSeq.push_back(info);
	}

	if( strCaptureSeq == CAPTURE_SEQ_ROLL_SINGLE_FINGER )
	{
		info.PreCaptureMessage = _T("Please put a single finger on the sensor!");
		info.PostCaptuerMessage = _T("Roll finger!");
		info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
		info.NumberOfFinger = 1;
		info.fingerName = _T("SRF_Unknown");
		info.fingerPosition = IBSM_FINGER_POSITION_UNKNOWN;
		m_vecCaptureSeq.push_back(info);
	}

	if( strCaptureSeq == CAPTURE_SEQ_2_FLAT_FINGERS )
	{
		info.PreCaptureMessage = _T("Please put two fingers on the sensor!");
		info.PostCaptuerMessage = _T("Keep fingers on the sensor!");
		info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
		info.NumberOfFinger = 2;
		info.fingerName = _T("TFF_Unknown");
		info.fingerPosition = IBSM_FINGER_POSITION_UNKNOWN;
		m_vecCaptureSeq.push_back(info);
	}

	if( strCaptureSeq == CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS )
	{
		info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
		info.fingerName = _T("SFF_Right_Thumb");
		info.PostCaptuerMessage = _T("Keep fingers on the sensor!");
		info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
		info.NumberOfFinger = 1;
		info.fingerPosition = IBSM_FINGER_POSITION_RIGHT_THUMB;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put right index on the sensor!");
		info.fingerName = _T("SFF_Right_Index");
		info.fingerPosition = IBSM_FINGER_POSITION_RIGHT_INDEX_FINGER;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put right middle on the sensor!");
		info.fingerName = _T("SFF_Right_Middle");
		info.fingerPosition = IBSM_FINGER_POSITION_RIGHT_MIDDLE_FINGER;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put right ring on the sensor!");
		info.fingerName = _T("SFF_Right_Ring");
		info.fingerPosition = IBSM_FINGER_POSITION_RIGHT_RING_FINGER;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put right little on the sensor!");
		info.fingerName = _T("SFF_Right_Little");
		info.fingerPosition = IBSM_FINGER_POSITION_RIGHT_LITTLE_FINGER;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
		info.fingerName = _T("SFF_Left_Thumb");
		info.fingerPosition = IBSM_FINGER_POSITION_LEFT_THUMB;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put left index on the sensor!");
		info.fingerName = _T("SFF_Left_Index");
		info.fingerPosition = IBSM_FINGER_POSITION_LEFT_INDEX_FINGER;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put left middle on the sensor!");
		info.fingerName = _T("SFF_Left_Middle");
		info.fingerPosition = IBSM_FINGER_POSITION_LEFT_MIDDLE_FINGER;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put left ring on the sensor!");
		info.fingerName = _T("SFF_Left_Ring");
		info.fingerPosition = IBSM_FINGER_POSITION_LEFT_RING_FINGER;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put left little on the sensor!");
		info.fingerName = _T("SFF_Left_Little");
		info.fingerPosition = IBSM_FINGER_POSITION_LEFT_LITTLE_FINGER;
		m_vecCaptureSeq.push_back(info);
	}

	if( strCaptureSeq == CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS )
	{
		info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
		info.PostCaptuerMessage = _T("Roll finger!");
		info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
		info.NumberOfFinger = 1;
		info.fingerName = _T("SRF_Right_Thumb");
		info.fingerPosition = IBSM_FINGER_POSITION_RIGHT_THUMB;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put right index on the sensor!");
		info.fingerName = _T("SRF_Right_Index");
		info.fingerPosition = IBSM_FINGER_POSITION_RIGHT_INDEX_FINGER;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put right middle on the sensor!");
		info.fingerName = _T("SRF_Right_Middle");
		info.fingerPosition = IBSM_FINGER_POSITION_RIGHT_MIDDLE_FINGER;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put right ring on the sensor!");
		info.fingerName = _T("SRF_Right_Ring");
		info.fingerPosition = IBSM_FINGER_POSITION_RIGHT_RING_FINGER;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put right little on the sensor!");
		info.fingerName = _T("SRF_Right_Little");
		info.fingerPosition = IBSM_FINGER_POSITION_RIGHT_LITTLE_FINGER;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
		info.fingerName = _T("SRF_Left_Thumb");
		info.fingerPosition = IBSM_FINGER_POSITION_LEFT_THUMB;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put left index on the sensor!");
		info.fingerName = _T("SRF_Left_Index");
		info.fingerPosition = IBSM_FINGER_POSITION_LEFT_INDEX_FINGER;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put left middle on the sensor!");
		info.fingerName = _T("SRF_Left_Middle");
		info.fingerPosition = IBSM_FINGER_POSITION_LEFT_MIDDLE_FINGER;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put left ring on the sensor!");
		info.fingerName = _T("SRF_Left_Ring");
		info.fingerPosition = IBSM_FINGER_POSITION_LEFT_RING_FINGER;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put left little on the sensor!");
		info.fingerName = _T("SRF_Left_Little");
		info.fingerPosition = IBSM_FINGER_POSITION_LEFT_LITTLE_FINGER;
		m_vecCaptureSeq.push_back(info);
	}

	if( strCaptureSeq == CAPTURE_SEQ_4_FLAT_FINGERS )
	{
		info.PreCaptureMessage = _T("Please put 4 fingers on the sensor!");
		info.PostCaptuerMessage = _T("Keep fingers on the sensor!");
		info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
		info.NumberOfFinger = 4;
		info.fingerName = _T("4FF_Unknown");
		info.fingerPosition = IBSM_FINGER_POSITION_UNKNOWN;
		m_vecCaptureSeq.push_back(info);
	}

	if( strCaptureSeq == CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER )
	{
		info.PreCaptureMessage = _T("Please put right 4-fingers on the sensor!");
		info.fingerName = _T("4FF_Right_4_Fingers");
		info.PostCaptuerMessage = _T("Keep fingers on the sensor!");
		info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
		info.NumberOfFinger = 4;
		info.fingerPosition = IBSM_FINGER_POSITION_PLAIN_RIGHT_FOUR_FINGERS;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put left 4-fingers on the sensor!");
		info.fingerName = _T("4FF_Left_4_Fingers");
		info.fingerPosition = IBSM_FINGER_POSITION_PLAIN_LEFT_FOUR_FINGERS;
		m_vecCaptureSeq.push_back(info);

		info.PreCaptureMessage = _T("Please put 2-thumbs on the sensor!");
		info.fingerName = _T("TFF_2_Thumbs");
		info.fingerPosition = IBSM_FINGER_POSITION_PLAIN_THUMBS;
		info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
		info.NumberOfFinger = 2;
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

LRESULT CIBSU_NonCallbackSampleForVCDlg::OnMsg_CaptureSeqNext(WPARAM wParam, LPARAM lParam)
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
        CaptureInfo tmpInfo;
		_SetLEDs(m_nDevHandle, tmpInfo, __LED_COLOR_NONE__, FALSE);
		m_nCurrentCaptureStep = -1;
		m_ImgSubFolder = _T("");

		OnMsg_UpdateDisplayResources();
		GetDlgItem(IDC_BTN_CAPTURE_START)->SetFocus();
		return 0L;
	}

	if( m_chkInvalidArea )
	{
		IBSU_SetClientDisplayProperty(m_nDevHandle, ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA, "TRUE");
	}
	else
	{
		IBSU_SetClientDisplayProperty(m_nDevHandle, ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA, "FALSE");
	}


	if( m_chkDetectSmear )
	{
		IBSU_SetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_ROLL_MODE, "1");
		CString strValue;
		strValue.Format("%d", m_cboSmearLevel.GetCurSel());
		IBSU_SetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_ROLL_LEVEL, strValue);
	}
	else
	{
		IBSU_SetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_ROLL_MODE, "0");
	}

	for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
		IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);

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
	if( m_chkAutoContrast )
		captureOptions |= IBSU_OPTION_AUTO_CONTRAST;
	if( m_chkAutoCapture )
		captureOptions |= IBSU_OPTION_AUTO_CAPTURE;
	if( m_chkIgnoreFingerCount )
		captureOptions |= IBSU_OPTION_IGNORE_FINGER_COUNT;

	nRc = IBSU_BeginCaptureImage(m_nDevHandle, info.ImageType, imgRes, captureOptions);
	if( nRc >= IBSU_STATUS_OK )
	{
		// Display message for image acuisition
		CString strMessage;
		strMessage = info.PreCaptureMessage;

		_SetStatusBarMessage(_T("%s"), strMessage);
		if( !m_chkAutoCapture )
			strMessage += _T("\r\nPress button 'Take Result Image' when image is good!");

		_SetImageMessage(_T("%s"), strMessage);
		m_strImageMessage = strMessage;

        _SetLEDs(m_nDevHandle, info, __LED_COLOR_RED__, TRUE);
	}
	else
	{
		_SetStatusBarMessage(_T("Failed to execute IBSU_BeginCaptureImage()"));
	}

	OnMsg_UpdateDisplayResources();

	return 0L;
}

LRESULT CIBSU_NonCallbackSampleForVCDlg::OnMsg_DeviceCommunicationBreak(WPARAM wParam, LPARAM lParam)
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

LRESULT CIBSU_NonCallbackSampleForVCDlg::OnMsg_DrawClientWindow(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CIBSU_NonCallbackSampleForVCDlg::OnMsg_UpdateDeviceList(WPARAM wParam, LPARAM lParam)
{
	const bool idle = !m_bInitializing && ( m_nCurrentCaptureStep == -1 );
	if( idle )
	{
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
		_UpdateCaptureSequences();
	}

	return 0L;
}

LRESULT CIBSU_NonCallbackSampleForVCDlg::OnMsg_InitWarning(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CIBSU_NonCallbackSampleForVCDlg::OnMsg_UpdateDisplayResources(WPARAM wParam, LPARAM lParam)
{
	BOOL selectedDev = m_cboUsbDevices.GetCurSel() > 0;
	BOOL captureSeq = m_cboCaptureSeq.GetCurSel() > 0;
	BOOL idle = !m_bInitializing && ( m_nCurrentCaptureStep == -1 );
	BOOL active = !m_bInitializing && (m_nCurrentCaptureStep != -1 );
	BOOL uninitializedDev = selectedDev && ( m_nDevHandle == -1 );


	m_cboUsbDevices.EnableWindow( idle );  
	m_cboCaptureSeq.EnableWindow( selectedDev && idle );

	GetDlgItem( IDC_BTN_CAPTURE_START )->EnableWindow( captureSeq );
	GetDlgItem( IDC_BTN_CAPTURE_STOP )->EnableWindow( active );

	GetDlgItem( IDC_CHECK_AUTO_CONTRAST )->EnableWindow( selectedDev && idle );
	GetDlgItem( IDC_CHECK_AUTO_CAPTURE )->EnableWindow( selectedDev && idle );
	GetDlgItem( IDC_CHECK_IGNORE_FINGER_COUNT )->EnableWindow( selectedDev && idle );
	GetDlgItem( IDC_CHECK_SAVE_IMAGES )->EnableWindow( selectedDev && idle );
	GetDlgItem( IDC_BTN_IMAGE_FOLDER )->EnableWindow( selectedDev && idle );

	GetDlgItem( IDC_CHECK_CLEAR_PLATEN )->EnableWindow( uninitializedDev );
	CString strCaption = _T( "" );
	if( active )
		strCaption = _T( "Take Result Image" );  
	else if( !active && !m_bInitializing )
		strCaption = "Start";  

	SetDlgItemText( IDC_BTN_CAPTURE_START, strCaption );
	return 0;
}

LRESULT CIBSU_NonCallbackSampleForVCDlg::OnMsg_UpdateStatusMessage(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CIBSU_NonCallbackSampleForVCDlg::OnMsg_Beep(WPARAM wParam, LPARAM lParam)
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

LRESULT CIBSU_NonCallbackSampleForVCDlg::OnMsg_DrawFingerQuality(WPARAM wParam, LPARAM lParam)
{
	for( int i = 0; i < 4; i++ )
	{
	  GetDlgItem(IDC_STATIC_QUALITY_1+i)->RedrawWindow();
	}

	return 0L;
}










void CIBSU_NonCallbackSampleForVCDlg::OnBnClickedBtnCaptureStart()
{
	int		nRc;
	BOOL	bColumoSamples = FALSE;

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
			IBSU_TakeResultImageManually(m_nDevHandle);
		}

		return;
	}

	if( m_nDevHandle == -1 )
	{
		m_bInitializing = true;
		// Asynchronously Open Device
		// Function IBSU_OpenDevice() will not be blocked while initializing the algorithm (4 - 7seconds).
		// And you can start the capture after getting callback funcation IBSU_CallbackAsyncOpenDevice().		
        nRc = IBSU_AsyncOpenDevice( devIndex );
		m_bInitializing = false;

		if( nRc >= IBSU_STATUS_OK )
		{
			_WaitingForFinishInitDevice( devIndex );
			return;
		}
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
	else
	{
		// device already initialized
		PostMessage(WM_USER_CAPTURE_SEQ_START);
	}


	OnMsg_UpdateDisplayResources();
}

void CIBSU_NonCallbackSampleForVCDlg::OnBnClickedBtnCaptureStop()
{
	if( m_nDevHandle == -1 )
		return;

	IBSU_CancelCaptureImage( m_nDevHandle );
    CaptureInfo tmpInfo;
	_SetLEDs(m_nDevHandle, tmpInfo, __LED_COLOR_NONE__, FALSE);
	m_nCurrentCaptureStep = -1;
	m_bNeedClearPlaten = FALSE;
	m_bBlank = FALSE;

	_SetStatusBarMessage( _T( "Capture Sequence aborted" ) );
	m_strImageMessage = _T( "" );
	_SetImageMessage(_T( "" ));
	OnMsg_UpdateDisplayResources();
}

void CIBSU_NonCallbackSampleForVCDlg::OnBnClickedBtnImageFolder()
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

void CIBSU_NonCallbackSampleForVCDlg::OnCbnSelchangeComboDevices()
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

void CIBSU_NonCallbackSampleForVCDlg::OnCbnSelchangeComboCaptureSeq()
{
	OnMsg_UpdateDisplayResources();
}

void CIBSU_NonCallbackSampleForVCDlg::OnNMReleasedcaptureSliderContrast(NMHDR *pNMHDR, LRESULT *pResult)
{
	if( m_nDevHandle == -1 || m_chkAutoContrast )
		return;

	int pos = m_sliderContrast.GetPos();
	char cValue[5];
	sprintf(cValue, "%d", pos);
	GetDlgItem(IDC_EDIT_CONTRAST)->SetWindowText(cValue);
	IBSU_SetContrast(m_nDevHandle, pos);

	*pResult = 0;
}

void CIBSU_NonCallbackSampleForVCDlg::OnNMCustomdrawSliderContrast(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	if( m_nDevHandle == -1 || m_chkAutoContrast )
		return;

	int pos = m_sliderContrast.GetPos();
	char cValue[5];
	sprintf(cValue, "%d", pos);
	GetDlgItem(IDC_EDIT_CONTRAST)->SetWindowText(cValue);
	IBSU_SetContrast(m_nDevHandle, pos);

	*pResult = 0;
}

void CIBSU_NonCallbackSampleForVCDlg::OnBnClickedCheckAutoContrast()
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

void CIBSU_NonCallbackSampleForVCDlg::OnTimer(UINT_PTR nIDEvent)
{
	if( nIDEvent == TIMER_POOLING_DEVICE )
	{
		// Detected device count
		int		devCount;
		int		nRc = -1;
		int		imageStatus, detectedFingerCount;

		m_nPollingCount++;

		if( m_nPollingCount > 5 )
		{
			m_nPollingCount = 0;
			IBSU_GetDeviceCount(&devCount);
			if( m_nDeviceCount != devCount )
			{
				m_nDeviceCount = devCount;
				PostMessage( WM_USER_UPDATE_DEVICE_LIST );
			}

			if( m_nDevHandle != -1 )
				nRc = IBSU_IsDeviceOpened(m_nDevHandle);

			// Is check that device's communication is breaked
			if( nRc == IBSU_ERR_DEVICE_INVALID_STATE )
			{
				PostMessage( WM_USER_DEVICE_COMMUNICATION_BREAK );
			}

			// Drawing finger qualtiy
			for( int i = 0; i < IBSU_MAX_SEGMENT_COUNT; i++ )
			{
			  GetDlgItem(IDC_STATIC_QUALITY_1+i)->RedrawWindow();
			}
			if( m_bNeedClearPlaten )
				m_bBlank = !m_bBlank;
		}

		if( m_nDevHandle != -1 && m_nCurrentCaptureStep != -1 )
		{
			nRc = IBSU_BGetImageEx(m_nDevHandle, &imageStatus, &m_image, &m_imageType, &detectedFingerCount,
								&m_segmentImageArray[0], &m_segmentPositionArray[0], &m_segmentImageArrayCount,
								 &m_fingerCountState, &m_FingerQuality[0], &m_qualityArrayCount);

			int savedRollingState = m_rollingState;
			int nRc2 = IBSU_BGetRollingInfo(m_nDevHandle, &m_rollingState, &m_rollingLineX);

			if( nRc == IBSU_STATUS_OK )
			{
				if( m_saveFingerCountState != m_fingerCountState )
				{
					m_saveFingerCountState = m_fingerCountState;

					CaptureInfo info = m_vecCaptureSeq[m_nCurrentCaptureStep];
					if(m_fingerCountState == ENUM_IBSU_NON_FINGER)
					{
						_SetLEDs(m_nDevHandle, info, __LED_COLOR_RED__, TRUE);
					}
					else
					{
						_SetLEDs(m_nDevHandle, info, __LED_COLOR_YELLOW__, TRUE);
					}
				}

				if( nRc2 == IBSU_STATUS_OK &&
					( m_rollingState == ENUM_IBSU_ROLLING_TAKE_ACQUISITION ||
					  m_rollingState == ENUM_IBSU_ROLLING_COMPLETE_ACQUISITION ) &&
					savedRollingState != m_rollingState )
				{
					PostMessage( WM_USER_BEEP, __BEEP_OK__ );
					if( m_rollingState == ENUM_IBSU_ROLLING_TAKE_ACQUISITION )
					{
						m_strImageMessage.Format(_T("When done remove finger from sensor"));
						_SetImageMessage(m_strImageMessage);
						_SetStatusBarMessage(m_strImageMessage);
					}
				}

                if( m_image.IsFinal )
				{
            		CaptureInfo info = m_vecCaptureSeq[m_nCurrentCaptureStep];
            		_SetLEDs(m_nDevHandle, info, __LED_COLOR_GREEN__, FALSE);

					if( imageStatus >= IBSU_STATUS_OK )
					{
						PostMessage( WM_USER_BEEP, __BEEP_SUCCESS__ );
					}
					else
					{
						PostMessage( WM_USER_BEEP, __BEEP_FAIL__ );
					}

					// added 2012-11-30
					if( m_bNeedClearPlaten )
					{
						m_bNeedClearPlaten = FALSE;
					}

					// Image acquisition successful
					// Drawing finger qualtiy
					for( int i = 0; i < 4; i++ )
					{
					  GetDlgItem(IDC_STATIC_QUALITY_1+i)->RedrawWindow();
					}

					char imgTypeName[IBSU_MAX_STR_LEN]={0};
					switch( m_imageType )
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

					if( imageStatus >= IBSU_STATUS_OK )
					{
						// Image acquisition successful
						if( m_chkSaveImages )
						{
							_SetStatusBarMessage( _T( "Saving image..." ) );
							CaptureInfo info = m_vecCaptureSeq[m_nCurrentCaptureStep];
							_SaveBitmapImage(m_image, info.fingerName);
							_SaveWsqImage(m_image, info.fingerName);
							_SavePngImage(m_image, info.fingerName);
							_SaveJP2Image(m_image, info.fingerName);
							if( m_segmentImageArrayCount > 0 )
							{
								CString			segmentName;
								for( int i=0; i<m_segmentImageArrayCount; i++ )
								{
									segmentName.Format("%s_Segment_%02d", info.fingerName, i);
									_SaveBitmapImage(*(m_segmentImageArray+i), segmentName);
									_SaveWsqImage(*(m_segmentImageArray+i), segmentName);
									_SavePngImage(*(m_segmentImageArray+i), segmentName);
									_SaveJP2Image(*(m_segmentImageArray+i), segmentName);
								}
							}
						}

						if( m_chkDrawSegmentImage )
						{
							// Draw quadrangle for the segmented image
							char propertyValue[32];
							IBSU_GetClientWindowProperty(m_nDevHandle, ENUM_IBSU_WINDOW_PROPERTY_SCALE_FACTOR, propertyValue);
							double scaleFactor = atof(propertyValue);
							IBSU_GetClientWindowProperty(m_nDevHandle, ENUM_IBSU_WINDOW_PROPERTY_LEFT_MARGIN, propertyValue);
							int leftMargin = atoi(propertyValue);
							IBSU_GetClientWindowProperty(m_nDevHandle, ENUM_IBSU_WINDOW_PROPERTY_TOP_MARGIN, propertyValue);
							int topMargin = atoi(propertyValue);
							for( int i=0; i<m_segmentImageArrayCount; i++ )
							{
								COLORREF cr = RGB(0, 128, 0);
								int x1, x2, x3, x4, y1, y2, y3, y4;
								x1 = leftMargin + (int)((m_segmentPositionArray+i)->x1*scaleFactor);
								x2 = leftMargin + (int)((m_segmentPositionArray+i)->x2*scaleFactor);
								x3 = leftMargin + (int)((m_segmentPositionArray+i)->x3*scaleFactor);
								x4 = leftMargin + (int)((m_segmentPositionArray+i)->x4*scaleFactor);
								y1 = topMargin +  (int)((m_segmentPositionArray+i)->y1*scaleFactor);
								y2 = topMargin +  (int)((m_segmentPositionArray+i)->y2*scaleFactor);
								y3 = topMargin +  (int)((m_segmentPositionArray+i)->y3*scaleFactor);
								y4 = topMargin +  (int)((m_segmentPositionArray+i)->y4*scaleFactor);

								IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvSegmentHandle[i],
									x1, y1, x2, y2, x3, y3, x4, y4, 1, (DWORD)cr);
							}
						}

						//	CaptureInfo info = pDlg->m_vecCaptureSeq[pDlg->m_nCurrentCaptureStep];
						//	IBSM_ImageData resultImage;
						//	IBSM_ImageData splitResultImage[4];
						//	int splitResultImageCount;
						//	pDlg->_SetStatusBarMessage( _T( "Get IBSM_ResultImageInfo..." ) );
						//	IBSU_GetIBSM_ResultImageInfo(m_nDevHandle, info.fingerPosition, &resultImage, &splitResultImage[0], &splitResultImageCount);

						if( m_chkUseNFIQ )
						{
							int nfiq_score[IBSU_MAX_SEGMENT_COUNT];
							int	score=0, nRc, segment_pos=0;
							memset(&nfiq_score, 0, sizeof(nfiq_score));
							CString strValue = _T("Err");
							_SetStatusBarMessage( _T( "Get NFIQ score..." ) );
							for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
							{
								if( m_FingerQuality[i] == ENUM_IBSU_FINGER_NOT_PRESENT )
									continue;

								nRc = IBSU_GetNFIQScore(m_nDevHandle, (const BYTE*)(m_segmentImageArray+segment_pos)->Buffer,
									(m_segmentImageArray+segment_pos)->Width, (m_segmentImageArray+segment_pos)->Height,
									(m_segmentImageArray+segment_pos)->BitsPerPixel, &score);
								if( nRc == IBSU_STATUS_OK )
									nfiq_score[i] = score;
								else
									nfiq_score[i] = -1;

								segment_pos++;
							}
							strValue.Format("%d-%d-%d-%d", nfiq_score[0], nfiq_score[1], nfiq_score[2], nfiq_score[3]);
							GetDlgItem( IDC_EDIT_NFIQ )->SetWindowText(strValue);
						}

						if( imageStatus == IBSU_STATUS_OK )
						{
							m_strImageMessage.Format(_T("%s acquisition completed successfully"), imgTypeName);
							_SetImageMessage(m_strImageMessage);
							_SetStatusBarMessage(m_strImageMessage);
						}
						else
						{
							// > IBSU_STATUS_OK
							m_strImageMessage.Format(_T("%s acquisition Warning (Warning code = %d)"), imgTypeName, imageStatus);
							_SetImageMessage(m_strImageMessage);
							_SetStatusBarMessage(m_strImageMessage);
				            
							CString askMsg;
							askMsg.Format("[Warning = %d] Do you want a recapture?", imageStatus);
       						if( AfxMessageBox(askMsg, MB_YESNO | MB_ICONINFORMATION) == IDYES )
							{
								// To recapture current finger position
								m_nCurrentCaptureStep--;
							}
						}
					}
					else
					{
						// < IBSU_STATUS_OK
						m_strImageMessage.Format(_T("%s acquisition failed (Error code = %d)"), imgTypeName, imageStatus);
						_SetImageMessage(m_strImageMessage);
						_SetStatusBarMessage(m_strImageMessage);

						// Stop all of acquisition
						m_nCurrentCaptureStep = (int)m_vecCaptureSeq.size();
					}

					PostMessage( WM_USER_CAPTURE_SEQ_NEXT );
				}
			}
		}
	}

	CDialog::OnTimer(nIDEvent);
}

HBRUSH CIBSU_NonCallbackSampleForVCDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
			font_size = -MulDiv(11, GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
			if( fFont.CreateFont(font_size, 0, 0, 0, FW_NORMAL, 0, 0, 0,DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, ANTIALIASED_QUALITY,
				DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif") != NULL )
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

	return hbr;
}

void CIBSU_NonCallbackSampleForVCDlg::OnBnClickedCheckClearPlaten()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CIBSU_NonCallbackSampleForVCDlg::OnClose()
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
