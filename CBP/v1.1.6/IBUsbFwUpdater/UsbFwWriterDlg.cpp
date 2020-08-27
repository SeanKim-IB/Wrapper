
// UsbFwWriterDlg.cpp : implementation file
//

#include "stdafx.h"
#include <io.h>
#include "UsbFwWriter.h"
#include "UsbFwWriterDlg.h"

#include "IBScanUltimateApi.h"
#include "ReservedApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int e_progressiveRange;

#define PRODUCTION_SET_STRING	_T("ibkorea1120!")
#define PARTNER_SET_STRING		_T("#partner1042!")
#define PARTNER_SET_STRING2		_T("#partner1120!")
#define RESERVED_KEY_STRING		_T("ibkorea1120!")
#define	PROG_INI				_T("FirmwareWriterSetting.ini")
#define	FW_FILE_EXT				_T("*.iic")


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
#define WM_USER_IBK_PRODUCTION_MODE				WM_APP + 12
#define WM_USER_IB_PARTNER_MODE					WM_APP + 13


// Beep definitions
const int __BEEP_FAIL__							= 0;
const int __BEEP_SUCCESS__						= 1;
const int __BEEP_OK__							= 2;
const int __BEEP_DEVICE_COMMUNICATION_BREAK__	= 3;


// Use critical section
CRITICAL_SECTION g_CriticalSection;


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



// CUsbFwWriterDlg dialog




CUsbFwWriterDlg::CUsbFwWriterDlg(CWnd* pParent /*=NULL*/)
: CDialog(CUsbFwWriterDlg::IDD, pParent)
, m_FwThread(NULL)
, Property_old_font(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// initialize related members
	m_IBKProductionMode = PRODUCTION_SET_STRING;
	m_IBPartnerMode = PARTNER_SET_STRING;
	m_IBPartnerMode2 = PARTNER_SET_STRING2;
	m_keyIBKProductionMode = _T("");
	m_keyIBPartnerMode = _T("");
	m_bIBKProductionMode = FALSE;
	m_bIBPartnerMode = FALSE;

	InitializeCriticalSection(&g_CriticalSection);
}

CUsbFwWriterDlg::~CUsbFwWriterDlg()
{
	DeleteCriticalSection(&g_CriticalSection);
}

void CUsbFwWriterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_devList);
	DDX_Control(pDX, IDC_LIST_FILE, m_FwFileList);
	DDX_Control(pDX, IDC_BTN_FW_UPDATE, m_btnFirmwareUp);
	DDX_Control(pDX, IDC_BTN_EDIT_PROPERTY, m_editProperty);
	DDX_Control(pDX, IDC_BUTTON1, m_btnChgFirmwareDir);
}

BEGIN_MESSAGE_MAP(CUsbFwWriterDlg, CDialog)
	ON_WM_SYSCOMMAND()
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
	ON_MESSAGE(WM_USER_IBK_PRODUCTION_MODE,			OnMsg_IBKProductionMode)
	ON_MESSAGE(WM_USER_IB_PARTNER_MODE,				OnMsg_IBPartnerMode)

	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_FW_UPDATE, &CUsbFwWriterDlg::OnBnClickedBtnFwUpdate)
	ON_BN_CLICKED(IDC_BTN_EDIT_PROPERTY, &CUsbFwWriterDlg::OnBnClickedBtnEditProperty)
	ON_WM_DESTROY()

//	ON_WM_CTLCOLOR()
ON_WM_CREATE()
//ON_NOTIFY(HDN_ITEMCLICK, 0, &CUsbFwWriterDlg::OnHdnItemclickList1)
//ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST1, &CUsbFwWriterDlg::OnLvnColumnclickList1)
//ON_NOTIFY(HDN_ITEMCLICK, 0, &CUsbFwWriterDlg::OnHdnItemclickList1)
ON_NOTIFY(NM_CLICK, IDC_LIST1, &CUsbFwWriterDlg::OnNMClickList1)
ON_NOTIFY(NM_CLICK, IDC_LIST_FILE, &CUsbFwWriterDlg::OnNMClickListFile)
ON_BN_CLICKED(IDC_BUTTON1, &CUsbFwWriterDlg::OnBnClickedBtnChangeFwDir)
ON_WM_CTLCOLOR()
ON_WM_CLOSE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////////////////
void CUsbFwWriterDlg::_SetStatusBarMessage( LPCTSTR Format, ... )
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

int CUsbFwWriterDlg::_ReleaseDevice(int deviceHandle)
{
	int nRc = IBSU_STATUS_OK;

	if( deviceHandle != -1 )
		nRc = IBSU_CloseDevice( deviceHandle );

	return nRc;
}  

void CUsbFwWriterDlg::_BeepFail()
{
	Beep( 3500, 300 );
	Sleep(150);
	Beep( 3500, 150 );
	Sleep(150);
	Beep( 3500, 150 );
	Sleep(150);
	Beep( 3500, 150 );
}

void CUsbFwWriterDlg::_BeepSuccess()
{
	Beep( 3500, 100 );
	Sleep(50);
	Beep( 3500, 100 );
}

void CUsbFwWriterDlg::_BeepOk()
{
	Beep( 3500, 100 );
}

void CUsbFwWriterDlg::_BeepDeviceCommunicationBreak()
{
	for( int i=0; i<8; i++ )
	{
		Beep( 3500, 100 );
		Sleep( 100 );
	}
}

void CUsbFwWriterDlg::_UpdateScreenElements()
{
	BOOL	deviceSelected = FALSE;

	if( _GetCheckedDevCount() > 0 )
		deviceSelected = TRUE;

	GetDlgItem( IDC_BTN_FW_UPDATE )->EnableWindow( deviceSelected );
	GetDlgItem( IDC_BTN_EDIT_PROPERTY )->EnableWindow( deviceSelected );
}

int CUsbFwWriterDlg::_GetCheckedFwFileCount()
{
	int number = 0;
	
	for( int i=0; i<m_FwFileList.GetItemCount(); i++ )
	{
		if( m_FwFileList.GetCheck(i) )
		{
			number++;
		}
	}
	
	return number;
}

int CUsbFwWriterDlg::_GetCheckedDevCount()
{
	int number = 0;
	
	for( int i=0; i<m_devList.GetItemCount(); i++ )
	{
		if( m_devList.GetCheck(i) )
		{
			number++;
		}
	}
	
	return number;
}




////////////////////////////////////////////////////////////////////////////////////////////
void CUsbFwWriterDlg::OnEvent_DeviceCommunicationBreak(
						 const int deviceHandle,
						 void*     pContext
					   )
{
	if( pContext == NULL )
		return;

	CUsbFwWriterDlg *pDlg = reinterpret_cast<CUsbFwWriterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->PostMessage( WM_USER_DEVICE_COMMUNICATION_BREAK, deviceHandle );
	LeaveCriticalSection(&g_CriticalSection);
}

void CUsbFwWriterDlg::OnEvent_PreviewImage(
						 const int deviceHandle,
						 void*     pContext,
						 const	   IBSU_ImageData image
					   )
{
	if( pContext == NULL )
		return;

	CUsbFwWriterDlg *pDlg = reinterpret_cast<CUsbFwWriterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	LeaveCriticalSection(&g_CriticalSection);
}

void CUsbFwWriterDlg::OnEvent_FingerCount(
						   const int                   deviceHandle,
						   void*                       pContext,
						   const IBSU_FingerCountState fingerCountState
						 )
{
	if( pContext == NULL )
		return;

	CUsbFwWriterDlg *pDlg = reinterpret_cast<CUsbFwWriterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	LeaveCriticalSection(&g_CriticalSection);
}

void CUsbFwWriterDlg::OnEvent_FingerQuality(
                          const int                     deviceHandle,   
                          void                          *pContext,       
                          const IBSU_FingerQualityState *pQualityArray, 
                          const int                     qualityArrayCount    
						 )
{
	if( pContext == NULL )
		return;

	CUsbFwWriterDlg *pDlg = reinterpret_cast<CUsbFwWriterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	LeaveCriticalSection(&g_CriticalSection);
}

void CUsbFwWriterDlg::OnEvent_DeviceCount(
						 const int detectedDevices,
						 void      *pContext
					   )
{
	if( pContext == NULL )
		return;

	CUsbFwWriterDlg *pDlg = reinterpret_cast<CUsbFwWriterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->PostMessage( WM_USER_UPDATE_DEVICE_LIST );
	LeaveCriticalSection(&g_CriticalSection);
}

void CUsbFwWriterDlg::OnEvent_InitProgress(
						 const int deviceIndex,
						 void      *pContext,
						 const int progressValue
					   )
{
	if( pContext == NULL )
		return;

	CUsbFwWriterDlg *pDlg = reinterpret_cast<CUsbFwWriterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->_SetStatusBarMessage(_T("Initializing device... %d%%"), progressValue);
	LeaveCriticalSection(&g_CriticalSection);
}

void CUsbFwWriterDlg::OnEvent_TakingAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   )
{
	if( pContext == NULL )
		return;

	CUsbFwWriterDlg *pDlg = reinterpret_cast<CUsbFwWriterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	LeaveCriticalSection(&g_CriticalSection);
}

void CUsbFwWriterDlg::OnEvent_CompleteAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   )
{
	if( pContext == NULL )
		return;

	CUsbFwWriterDlg *pDlg = reinterpret_cast<CUsbFwWriterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	LeaveCriticalSection(&g_CriticalSection);
}

/****
 ** This IBSU_CallbackResultImage callback was deprecated since 1.7.0
 ** Please use IBSU_CallbackResultImageEx instead
*/
void CUsbFwWriterDlg::OnEvent_ResultImageEx(
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

	CUsbFwWriterDlg *pDlg = reinterpret_cast<CUsbFwWriterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	LeaveCriticalSection(&g_CriticalSection);
}

void CUsbFwWriterDlg::OnEvent_ClearPlatenAtCapture(
                          const int                   deviceHandle,
                          void                        *pContext,
                          const IBSU_PlatenState      platenState
                        )
{
	if( pContext == NULL )
		return;

	CUsbFwWriterDlg *pDlg = reinterpret_cast<CUsbFwWriterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	LeaveCriticalSection(&g_CriticalSection);
}

void CUsbFwWriterDlg::OnEvent_AsyncOpenDevice(
                          const int                   deviceIndex,
                          void                        *pContext,
                          const int                   deviceHandle,
                          const int                   errorCode
						  )
{
	if( pContext == NULL )
		return;

	CUsbFwWriterDlg *pDlg = reinterpret_cast<CUsbFwWriterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	LeaveCriticalSection(&g_CriticalSection);
}






////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CUsbFwWriterDlg::OnMsg_CaptureSeqStart(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CUsbFwWriterDlg::OnMsg_CaptureSeqNext(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CUsbFwWriterDlg::OnMsg_DeviceCommunicationBreak(WPARAM wParam, LPARAM lParam)
{
	int nRc;
	int deviceHandle = (int)wParam;

	if( deviceHandle == -1 )
		return 0L;

	_SetStatusBarMessage( _T( "Device communication was broken" ) );

	nRc = _ReleaseDevice(deviceHandle);
	if( nRc == IBSU_ERR_RESOURCE_LOCKED )
	{
		// retry to release device
		PostMessage( WM_USER_DEVICE_COMMUNICATION_BREAK, deviceHandle );
	}
	else
	{
		PostMessage( WM_USER_BEEP, __BEEP_DEVICE_COMMUNICATION_BREAK__ );
		PostMessage( WM_USER_UPDATE_DEVICE_LIST );
	}

	return 0L;
}

LRESULT CUsbFwWriterDlg::OnMsg_DrawClientWindow(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CUsbFwWriterDlg::OnMsg_UpdateDeviceList(WPARAM wParam, LPARAM lParam)
{
	_UpdateScreenElements();

	m_devList.DeleteAllItems();

	int devices = 0;
	IBSU_GetDeviceCount( &devices );

	for( int i = 0; i < devices; i++ )
	{
		RESERVED_DeviceInfo devInfo;
		
		if( RESERVED_GetDeviceInfo( i, RESERVED_KEY_STRING, &devInfo ) < IBSU_STATUS_OK )
			continue;

		CString strDevice;
		if( devInfo.productName[0] == 0 )
			strDevice = _T( "unknown device" );
		else
			strDevice.Format( _T( "%s" ), devInfo.productName );

        if (strcmp(devInfo.productName, "CURVE") == 0)
        {
            strcpy(devInfo.productName, devInfo.reserved2);
        }
		m_devList.InsertItem(i,devInfo.productName);
		m_devList.SetItemText(i,1,devInfo.serialNumber);
		m_devList.SetItemText(i,2,devInfo.fwVersion);
		m_devList.SetItemText(i,3,devInfo.vendorID);
		m_devList.SetItemText(i,4,devInfo.devRevision);
		m_devList.SetItemText(i,5,devInfo.productionDate);
		m_devList.SetItemText(i,6,devInfo.serviceDate);
	}

	return 0L;
}

LRESULT CUsbFwWriterDlg::OnMsg_InitWarning(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CUsbFwWriterDlg::OnMsg_UpdateDisplayResources(WPARAM wParam, LPARAM lParam)
{
/*	BOOL selectedDev = m_cboUsbDevices.GetCurSel() > 0;
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
*/
	return 0;
}

LRESULT CUsbFwWriterDlg::OnMsg_UpdateStatusMessage(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CUsbFwWriterDlg::OnMsg_Beep(WPARAM wParam, LPARAM lParam)
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

LRESULT CUsbFwWriterDlg::OnMsg_DrawFingerQuality(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CUsbFwWriterDlg::OnMsg_IBKProductionMode(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_bIBKProductionMode = TRUE;

	AfxMessageBox("Enabled IB's production mode!");
	return 0;
}

LRESULT CUsbFwWriterDlg::OnMsg_IBPartnerMode(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_bIBPartnerMode = TRUE;

	AfxMessageBox("Enabled IB's partner mode!");
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////


// CUsbFwWriterDlg message handlers

BOOL CUsbFwWriterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

//	WarningpDlg.Create(Warning::IDD, this);                
//  WarningpDlg.CenterWindow();


/*	memset(&Property_logfont,0,sizeof(Property_logfont));
	Property_logfont.lfHeight=-13;
	
	memcpy(&Property_logfont.lfFaceName,"MS Shell Dlg 2",15);
	Property_logfont.lfWeight=FW_BOLD;
	Property_new_font.CreateFontIndirectA(&Property_logfont);
	Property_old_font=GetDlgItem( IDC_PROPERTY_VALUE_1)->GetFont();
*/

	m_devList.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	m_FwFileList.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	m_btnFirmwareUp.SetIcon(IDI_ICON_TOOL, (int)BTNST_AUTO_GRAY);
	m_btnFirmwareUp.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 30);
	m_btnFirmwareUp.DrawTransparent(true);

	m_editProperty.SetIcon(IDI_ICON_EDIT, (int)BTNST_AUTO_GRAY);
	m_editProperty.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 30);
	m_editProperty.DrawTransparent(true);

	m_btnChgFirmwareDir.SetIcon(IDI_ICON_DIR, (int)BTNST_AUTO_GRAY);
	m_btnChgFirmwareDir.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 30);
	m_btnChgFirmwareDir.DrawTransparent(true);



	// Device List
	m_devList.InsertColumn(0, "Product Name", LVCFMT_LEFT, 140);
	m_devList.InsertColumn(1, "Serial Number", LVCFMT_LEFT, 140);
	m_devList.InsertColumn(2, "Firmware", LVCFMT_LEFT, 90);
	m_devList.InsertColumn(3, "VenderID", LVCFMT_LEFT, 140);
	m_devList.InsertColumn(4, "Revison", LVCFMT_LEFT, 90);
	m_devList.InsertColumn(5, "Production Date", LVCFMT_LEFT, 100);
	m_devList.InsertColumn(6, "Service Date", LVCFMT_LEFT, 100);

	// Firmware Files
	m_FwFileList.InsertColumn(0, "File Name", LVCFMT_LEFT, 140);
	m_FwFileList.InsertColumn(1, "Product Name", LVCFMT_LEFT, 140);
	m_FwFileList.InsertColumn(2, "FirmWare", LVCFMT_LEFT, 90);
	m_FwFileList.InsertColumn(3, "VenderID", LVCFMT_LEFT, 140);
	m_FwFileList.InsertColumn(4, "Revison", LVCFMT_LEFT, 90);
	

	ReadIniFile();

	m_FwFileList.DeleteAllItems();
	LoadFwFileToList(m_cFwFilePath, FW_FILE_EXT);

	m_chknumberofprogress = 0;

	mpDlg = NULL;

	IBSU_GetSDKVersion(&m_verInfo);
	CString titleName;
	titleName.Format("IBUsbFwUpdater");
	SetWindowText(titleName);

	CString strDrawString;
	strDrawString.Format("IBUsbFwUpdater with DLL ver. %s", m_verInfo.Product);
	GetDlgItem(IDC_STATIC_DLL_VER)->SetWindowText(strDrawString);

	strDrawString = _T("Copyright (c) Integrated Biometrics");
	GetDlgItem(IDC_STATIC_COPYRIGHT)->SetWindowText(strDrawString);

	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, OnEvent_DeviceCount, this );

	_UpdateScreenElements();
	PostMessage( WM_USER_UPDATE_DEVICE_LIST );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUsbFwWriterDlg::WriteIniFile(char *saveDir)
{
	sprintf(m_cFwFilePath, "%s\\", saveDir);

	if( _access(m_cIniFileName, NULL) ==0 )
	{
		::WritePrivateProfileString("SETTING", "Directory", m_cFwFilePath, m_cIniFileName);
	}
	else
	{
		sprintf(m_cIniFileName, "%s\\%s", m_cFwFilePath, PROG_INI);
		::WritePrivateProfileString("SETTING", "Directory", m_cFwFilePath, m_cIniFileName);
	}
}

void CUsbFwWriterDlg::ReadIniFile()
{
	char	iniFile[512];
	char	curDir[MAX_PATH];


	GetCurrentDirectory(MAX_PATH, curDir);
	sprintf(iniFile, "%s\\%s", curDir, PROG_INI);

	if( _access(iniFile, NULL) ==0 )
	{
		strcpy(m_cIniFileName, iniFile);
		::GetPrivateProfileString("SETTING", "Directory","C:\\Program Files\\Integrated Biometrics\\IBScanUltimateSDK\\Firmware\\",m_cFwFilePath, MAX_PATH, m_cIniFileName);
	}
	else
	{
		sprintf(m_cFwFilePath, "%s", "C:\\Program Files\\Integrated Biometrics\\IBScanUltimateSDK\\Firmware\\");
		sprintf(m_cIniFileName, "%s\\%s", m_cFwFilePath, PROG_INI);
		::WritePrivateProfileString("SETTING", "Directory", m_cFwFilePath, m_cIniFileName);
	}
}

BOOL CUsbFwWriterDlg::LoadFwFileToList(CString path, CString ext)
{
	CFileFind finder;
	BOOL bWorking = finder.FindFile(path+ext);
	int pos =0;
	PRODUCTPROPERTY productProperty;
	FILE *fp;
	int fp_pos;

	while(bWorking)
	{
		bWorking = finder.FindNextFile();
		m_FwFileList.InsertItem(pos,finder.GetFileName().Left ( finder.GetFileName().GetLength() - 4 ));
		fp = fopen(path+finder.GetFileName().Left ( finder.GetFileName().GetLength() - 4 )+".iic","rb");


		fp_pos = 0 - (sizeof(PRODUCTPROPERTY)+4);
		fseek(fp, fp_pos, SEEK_END);

		if(NULL != fp)
		{
			fread(&productProperty, 1,sizeof(PRODUCTPROPERTY), fp);
			fclose(fp);
		}

		//파일을 읽어들일때 구조체가 추가된 버전인지 확인한다.
		if(strcmp(productProperty.vender_ID, "Integrated Biometrics")==0)
		{
			if( ( strcmp( productProperty.product_ID, "IBNW11C" )		== 0) ||
				( strcmp( productProperty.product_ID, "IBSCAN310LS-W" ) == 0) )
			{
				strcpy(productProperty.product_ID, "WATSON");
			}
			m_FwFileList.SetItemText(pos, 1, productProperty.product_ID);
			m_FwFileList.SetItemText(pos, 2, productProperty.firmware_version);
			m_FwFileList.SetItemText(pos, 3, productProperty.vender_ID);
			m_FwFileList.SetItemText(pos, 4, productProperty.revison);
		}
		else
		{
			m_FwFileList.SetItemText(pos, 1, "Unknown");
			m_FwFileList.SetItemText(pos, 2, "Unknown");
			m_FwFileList.SetItemText(pos, 3, "Unknown");
			m_FwFileList.SetItemText(pos, 4, "Unknown");
		}
		pos++;
	}
	return 1;
	
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CUsbFwWriterDlg::OnPaint()
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
HCURSOR CUsbFwWriterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CUsbFwWriterDlg::OnBnClickedBtnFwUpdate()
{
	CString textvalue;
	CString temp;
	unsigned int	crc = 0;
	FILE			*fp;
	int				fp_pos;

	CString chk;

	mpDlg = new CProgressive();

	m_chknumberofprogress = 0;


	e_progressiveRange = _GetCheckedDevCount();

	if( _GetCheckedFwFileCount() > 1 )
	{
		AfxMessageBox("You have to check one firmware.");
		return;
	}
	
	for( int i=0; i<m_FwFileList.GetItemCount(); i++ )
	{
		if( m_FwFileList.GetCheck(i) )
		{
			textvalue = m_FwFileList.GetItemText(i,0);
			temp = m_cFwFilePath + textvalue +".iic";
		}
	}

	strcpy(m_fwFileInfo.pathName, temp.GetBuffer(0));
	strcpy(m_fwFileInfo.fileName, textvalue.GetBuffer(0));

	//읽어온 파일에서 CRC를 계산
	fp = fopen(m_fwFileInfo.pathName, "rb");
	if( fp == NULL )
	{
		AfxMessageBox("Firmware file is not opened.");
		return;
	}

	fseek(fp, 0, SEEK_END);
	m_fwFileInfo.fileLength = ftell(fp);
	
	fseek(fp, 0, SEEK_SET);
	fread(m_fwFileInfo.fwData, 1, m_fwFileInfo.fileLength, fp);

	fp_pos = 0 - (sizeof(PRODUCTPROPERTY)+4);
	fseek(fp, fp_pos, SEEK_END);
	fread(&m_fwFileInfo.product_ID[0], 1, sizeof(PRODUCTPROPERTY), fp);

	fseek(fp, -4, SEEK_END);
	fread(&m_fwFileInfo.crc, 1, 4, fp);

	fclose(fp);

	for(int i=0; i<(m_fwFileInfo.fileLength-4); i++)
	{
		crc += m_fwFileInfo.fwData[i];
	}

	if( ( strcmp( m_fwFileInfo.product_ID, "IBNW11C" )			== 0) ||
		( strcmp( m_fwFileInfo.product_ID, "IBSCAN310LS-W" )   == 0) )
	{
		strcpy(m_fwFileInfo.product_ID, "WATSON");
	}

	//구조체는 파일끝에 있으므로 파일끝에서 구조체 크기만큼 이동하면 구조체의 시작임
	//파일에서 crc값을 읽어와서 위에서 구해진 값과 비교한다. 

	//firmware가 구조체 추가 이전 버전인지 확인한다. 
	if( strcmp(m_fwFileInfo.vender_ID, "Integrated Biometrics") == 0 )
	{
		//CRC 값이 일치하면 다음 루틴에 들어간다.
		//파일에서 crc값을 읽어와서 위에서 구해진 값과 비교한다. 
		if( crc != m_fwFileInfo.crc )
		{
			AfxMessageBox("There is unknown problem with firmware file.");
			return;
		}

		// Gon Add : Property 구조체 부분은 EEPROM에 writing 하지 않도록 fileLength를 구조체 크기만큼 줄인다.
		m_fwFileInfo.fileLength = m_fwFileInfo.fileLength - sizeof(PRODUCTPROPERTY);
	}

	mpDlg->Create(IDD_DIALOG_PRO);
	mpDlg->ShowWindow(SW_SHOW);

	m_FwThread = AfxBeginThread(FwProgressThread, this);
}

void CUsbFwWriterDlg::OnBnClickedBtnEditProperty()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	CEditProperty eDlg;
	int nRc = IBSU_STATUS_OK;
	CString msg;
	int devHandle;


	if( _GetCheckedDevCount() == 1 )
	{
		for(int i=0;i<m_devList.GetItemCount();i++)
		{
			if( m_devList.GetCheck(i) )
			{
				RESERVED_DeviceInfo devInfo;
				nRc = RESERVED_GetDeviceInfo(i, RESERVED_KEY_STRING, &devInfo);
				if( nRc == IBSU_STATUS_OK && strcmp(devInfo.productName, "CURVE") == 0 )
				{
					msg.Format("%s is not support this feature!", devInfo.productName);
					AfxMessageBox(msg);
					return;
				}

				nRc = RESERVED_OpenDevice(i, RESERVED_KEY_STRING, &devHandle);
				if( nRc >= IBSU_STATUS_OK )
				{
					IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, OnEvent_DeviceCommunicationBreak, this );
				}

				if( IBSU_IsDeviceOpened(devHandle) != IBSU_STATUS_OK )
				{
					msg.Format("%s Device isn't intialized", devInfo.productName);
					AfxMessageBox(msg);
					IBSU_CloseDevice(devHandle);
					return;
				}

				////////////////////////////
				eDlg.SetParentDlg(this);
				eDlg.SetDeviceHandle(devHandle);
				INT_PTR nResult = eDlg.DoModal();

				IBSU_CloseDevice(devHandle);
			}
		}

		if( m_bIBKProductionMode )
		{
			PostMessage( WM_USER_UPDATE_DEVICE_LIST );
		}
	}
	else
	{
		AfxMessageBox("You have to select the one device.");
	}
}

BOOL CUsbFwWriterDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
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
	case WM_CHAR :
		// Routine for matching of key sequece for IBK's production mode
		if ( pMsg->wParam == (WPARAM)m_IBKProductionMode.GetAt( m_keyIBKProductionMode.GetLength() ) )
		{
			m_keyIBKProductionMode += ( char ) pMsg->wParam ;

			if ( m_IBKProductionMode == m_keyIBKProductionMode )
			{
				m_keyIBKProductionMode = _T("");
				PostMessage(WM_USER_IBK_PRODUCTION_MODE, 0, 0);
			}

			return FALSE;	// key sequence 입력 중에는 소리 안나게 한다.
		}
		else if ( pMsg->wParam == (WPARAM)m_IBPartnerMode.GetAt( m_keyIBPartnerMode.GetLength() ) ||
            pMsg->wParam == (WPARAM)m_IBPartnerMode2.GetAt( m_keyIBPartnerMode.GetLength() ))
		{
			m_keyIBPartnerMode += ( char ) pMsg->wParam ;

			if ( m_IBPartnerMode == m_keyIBPartnerMode ||
                 m_IBPartnerMode2 == m_keyIBPartnerMode)
			{
				m_keyIBPartnerMode = _T("");
				PostMessage(WM_USER_IB_PARTNER_MODE, 0, 0);
			}

			return FALSE;	// key sequence 입력 중에는 소리 안나게 한다.
		}
		else
		{
			m_keyIBKProductionMode = _T("");
		}
		break;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

UINT CUsbFwWriterDlg::FwProgressThread(LPVOID pParam)
{
	CUsbFwWriterDlg* pDlg =(CUsbFwWriterDlg*) pParam ;

	RESERVED_DeviceInfo devInfo;
	int		nNumberOfDevList = pDlg->m_devList.GetItemCount();
	int		progress = 0;
	int		pos = 0;
	int		nNumberOfCheckedDevices = 0;
	int		nFailedCount = 0;
	int		devHandle;
	int		nRc = IBSU_STATUS_OK;


	nNumberOfCheckedDevices = pDlg->_GetCheckedDevCount();

	for( int i=0; i<nNumberOfDevList; i++ )
	{
		if( pDlg->m_devList.GetCheck(i) )
		{
			progress++;

			nRc = RESERVED_OpenDevice(i, RESERVED_KEY_STRING, &devHandle);
			if( nRc < IBSU_STATUS_OK )
			{
				nFailedCount++;
				continue;
			}

			IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, OnEvent_DeviceCommunicationBreak, pDlg);
			/////////////////////////////////

			RESERVED_GetDeviceInfo( i, RESERVED_KEY_STRING, &devInfo );

            if (strcmp(devInfo.productName, "CURVE") == 0)
            {
                strcpy(devInfo.productName, devInfo.reserved2);
            }

			if( pDlg->m_bIBKProductionMode )
			{
				if( strcmp(devInfo.productName, pDlg->m_fwFileInfo.product_ID) != 0 ||
					strcmp(devInfo.fwVersion, pDlg->m_fwFileInfo.firmware_version) >= 0 )
				{
					if( AfxMessageBox("This firmware is different with selected device. Do you need to update by force?", MB_OKCANCEL) == IDCANCEL )
					{
						IBSU_CloseAllDevice();
						::PostMessage(pDlg->mpDlg->m_hWnd, WM_CLOSE, 0, 0);
						return 0;
					}
				}
			}
			else
			{
				if( strcmp(devInfo.productName, pDlg->m_fwFileInfo.product_ID) != 0 )
				{
					AfxMessageBox("This firmware could not used for your device");
					::PostMessage(pDlg->mpDlg->m_hWnd, WM_CLOSE, 0, 0);
					IBSU_CloseAllDevice();
					return 0;
				}

				if( strcmp(devInfo.fwVersion, pDlg->m_fwFileInfo.firmware_version) >= 0 )
				{
					if( progress == 1 )
					{
						if( AfxMessageBox("Your firmversion is same or higher than firmware file. Do you need to update?", MB_OKCANCEL) == IDCANCEL )
						{
							IBSU_CloseAllDevice();
							::PostMessage(pDlg->mpDlg->m_hWnd, WM_CLOSE, 0, 0);
							return 0;
						}
					}
				}
			}

			pDlg->GetDlgItem(IDC_BTN_FW_UPDATE)->EnableWindow(FALSE);
			pDlg->GetDlgItem(IDC_BTN_EDIT_PROPERTY)->EnableWindow(FALSE);

			pDlg->Control_WriteUsbFirmware(devHandle, pDlg->m_fwFileInfo.fwData, pDlg->m_fwFileInfo.fileLength);

			pDlg->EnableWindow();
		}
	}

	if( nFailedCount == 0 )
	{
		AfxMessageBox("All firmwares successfully Updated.");
	}
	else
	{
		AfxMessageBox("One of devices could not be updated. Please check your devices status.");
	}

	IBSU_CloseAllDevice();
	::PostMessage(pDlg->mpDlg->m_hWnd, WM_CLOSE, 0, 0);

	pDlg->PostMessage( WM_USER_UPDATE_DEVICE_LIST );

	return 0;
}

int CUsbFwWriterDlg::Control_WriteUsbFirmware(int handle, UCHAR *fwBuffer, int bufSize)//unsigned char* fwImage, int imageLen)
{

#define BLOCK_SIZE	32

	int		nRc = IBSU_STATUS_OK, i;
	int		nBlock, nRemainder;
	UCHAR	tempBuf[BLOCK_SIZE];
	int progress_state=0;

	unsigned char outbuffer[64], inBuffer[64];
	memset(outbuffer, 0, 64);
	memset(inBuffer, 0, 64);

	int a=0; 
	int b=0; 
	int c=0;

	//m_chknumberofprogress++;

	//  0x1E00 ~ 0x1FFF 까지는 개별 Data 영역이므로 보호한다.
	if( bufSize < 0 || bufSize > 0x1e00 )
		return IBSU_ERR_INVALID_PARAM_VALUE;

	nBlock     = bufSize / BLOCK_SIZE;
	nRemainder = bufSize % BLOCK_SIZE;

	//  Firmware updata 중에 발생하는 예외상황에 대비하기 위하여
	//  EEPROM의 DATA(첫번째 block)를 무효화 시키다.
	//  (EZUSB가 Firmware를 인식하지 못하게 한다)

	for( i=0; i<BLOCK_SIZE; i++ )
		tempBuf[i] = 0xff;
	progress_state=(32*1000)/(bufSize+32);
	/////////////////

	mpDlg->m_progressiveTotal.SetPos(m_chknumberofprogress*1000);
	
	//////

	// Disable EEPROM Writing Protection
	outbuffer[0] = 0;
	nRc = RESERVED_UsbBulkOutIn(handle, "ibkorea1120!", 0/*EP1OUT*/, 0xFA/*CMD_EEPROM_WRITEPROTECTION_ENABLE*/, outbuffer, 1, -1, NULL, 0, NULL );
	if( nRc != IBSU_STATUS_OK)
		return nRc;

	Sleep(50);

	if(m_chknumberofprogress==0) 
	{
		mpDlg->m_progressiveTotal.OffsetPos(+(progress_state));
	}
	else
	{
		mpDlg->m_progressiveTotal.OffsetPos(+(progress_state));
	}

	mpDlg->m_progressiveWatson.SetPos(progress_state);

	nRc = RESERVED_WriteEEPROM( handle, RESERVED_KEY_STRING, 0x0000, tempBuf, BLOCK_SIZE );
    



	////////////////////
	//result = true;
	///////////////////
	if( nRc != IBSU_STATUS_OK ) 
		goto done;
	Sleep(50);

	
	for( i=1; i<nBlock; i++ )
	{
		progress_state=((32*(i+1))*1000)/(bufSize+32) ;	

		mpDlg->m_progressiveTotal.SetPos(m_chknumberofprogress*1000);
		if(m_chknumberofprogress==0) 
		{
			mpDlg->m_progressiveTotal.OffsetPos(+(progress_state));

		}else
		{
			mpDlg->m_progressiveTotal.OffsetPos(+(progress_state));
		}

		mpDlg->m_progressiveWatson.SetPos(progress_state);

		nRc = RESERVED_WriteEEPROM( handle, RESERVED_KEY_STRING, 0x0000 + i * BLOCK_SIZE, &fwBuffer[i*BLOCK_SIZE], BLOCK_SIZE );


		
		
		if( nRc != IBSU_STATUS_OK ) 
			goto done;
		Sleep(50);
		
		
	
	}

	if( nRemainder )
	{
		progress_state=(((32*(i+1))+nRemainder)/(bufSize+32)) * 1000;

		mpDlg->m_progressiveTotal.SetPos(m_chknumberofprogress*1000);

		if(m_chknumberofprogress==0) 
		{
			mpDlg->m_progressiveTotal.OffsetPos(+(progress_state));
		}else
		{
			mpDlg->m_progressiveTotal.OffsetPos(+(progress_state));
		}

		mpDlg->m_progressiveWatson.SetPos(progress_state);
		
		nRc = RESERVED_WriteEEPROM( handle, RESERVED_KEY_STRING, 0x0000 + i * BLOCK_SIZE, &fwBuffer[i*BLOCK_SIZE], nRemainder );



	
		if( nRc != IBSU_STATUS_OK ) 
			goto done;
	
		Sleep(50);
		
		
	}

	//  Firmware의 모든 block이 update되면 첫번째 block을 맨 마지막으로
	//  Write 한다.
	progress_state=1000;
	mpDlg->m_progressiveWatson.SetPos(progress_state);
	////////////////////////////////////////////////////////


	if( nBlock ) 
		nRc = RESERVED_WriteEEPROM( handle, RESERVED_KEY_STRING, 0x0000, &fwBuffer[0], BLOCK_SIZE );
	else int test =1;
		nRc = RESERVED_WriteEEPROM( handle, RESERVED_KEY_STRING, 0x0000, &fwBuffer[0], nRemainder );
	
	Sleep(50);

done:

	// Enable EEPROM Writing Protection
	outbuffer[0] = 1;
	nRc = RESERVED_UsbBulkOutIn(handle, "ibkorea1120!", 0/*EP1OUT*/, 0xFA/*CMD_EEPROM_WRITEPROTECTION_ENABLE*/, outbuffer, 1, -1, NULL, 0, NULL );

	Sleep(50);

	m_chknumberofprogress++;

	return nRc;

}

BOOL CUsbFwWriterDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
//	WarningpDlg.DestroyWindow();
	
	return CDialog::DestroyWindow();
}

void CUsbFwWriterDlg::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	int index = pNMItemActivate->iItem;

	if( index >= 0 && index < m_devList.GetItemCount())
	{
		if( m_devList.GetCheck(index) )
		{
			m_devList.SetCheck(index,0);
		}
		else
		{
			m_devList.SetCheck(index,1);
		}
	}
	_UpdateScreenElements();

	*pResult = 0;
}

void CUsbFwWriterDlg::OnNMClickListFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	int index = pNMItemActivate->iItem;

	int flag=0;

	if( index >= 0 && index < m_FwFileList.GetItemCount() )
	{
		if( m_FwFileList.GetCheck(index) )
		{
			m_FwFileList.SetCheck(index, 0);
		}
		else
		{
			for(int i=0;i<m_FwFileList.GetItemCount();i++)
			{
				if( m_FwFileList.GetCheck(i) ) m_FwFileList.SetCheck(i,false);
			}
			m_FwFileList.SetCheck(index, 1);
		}
	}
	_UpdateScreenElements();
	*pResult = 0;
}

void CUsbFwWriterDlg::OnBnClickedBtnChangeFwDir()
{
	TCHAR m_ImgSaveFolder[261];
	LPITEMIDLIST pidSelected;
	BROWSEINFO	bi = {0};
	LPMALLOC	pMalloc;

	SHGetMalloc(&pMalloc);

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.lpszTitle = _T("Please select a folder to read a firmware file");
	bi.ulFlags = BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseForFolderCallback;
	bi.lParam = (LPARAM)&m_ImgSaveFolder[261];

	pidSelected = SHBrowseForFolder(&bi);
	
	if(pidSelected)
	{
		SHGetPathFromIDList( pidSelected, m_ImgSaveFolder );
		pMalloc->Free(pidSelected);
	}
	pMalloc->Release();

	WriteIniFile(m_ImgSaveFolder);
	m_FwFileList.DeleteAllItems();
	LoadFwFileToList(m_cFwFilePath, FW_FILE_EXT);
}

HBRUSH CUsbFwWriterDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

void CUsbFwWriterDlg::OnClose()
{
	int nRc;
	for( int i=0; i<10; i++ )
	{
		nRc = IBSU_CloseAllDevice();
		if( nRc != IBSU_ERR_RESOURCE_LOCKED )
			break;
		Sleep(100);
	}

	if( mpDlg )
		delete mpDlg;

	CDialog::OnClose();
}
