
// IBSDKFunctionTesterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IBSDKFunctionTester.h"
#include "IBSDKFunctionTesterDlg.h"

#include "IBScanUltimateApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CRITICAL_SECTION g_CriticalSection;
// CIBSDKFunctionTesterDlg dialog




CIBSDKFunctionTesterDlg::CIBSDKFunctionTesterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIBSDKFunctionTesterDlg::IDD, pParent)
	, m_chkUseClientWindow(FALSE)
	, m_chkAutoContrast(FALSE)
	, m_chkAutoCapture(FALSE)
	, m_chkGuideLineValue(FALSE)
	, m_chkIgnoreNumberofFinger(FALSE)
	, m_chkAppendFunctionResult(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_indexForproperty = -1;
	m_StartAgingTest = FALSE;
	m_IsAppend = FALSE;

	InitializeCriticalSection(&g_CriticalSection);
}

void CIBSDKFunctionTesterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MAIN, m_listMain);
	DDX_Control(pDX, IDC_BUTTON_DEVICECLOSE, m_btnCloseDevice);
	DDX_Control(pDX, IDC_BUTTON_ISDEVICEOPEN, m_btnIsDeviceOpened);
	DDX_Control(pDX, IDC_COMBO_PROPERTY, m_cbProperty);
	DDX_Check(pDX, IDC_CHECK_CLIENT_WINDOW, m_chkUseClientWindow);
	DDX_Check(pDX, IDC_CHECK__AUTO_CONTRAST, m_chkAutoContrast);
	DDX_Check(pDX, IDC_CHECK_AUTO_CAPTURE, m_chkAutoCapture);
	DDX_Control(pDX, IDC_COMBO_ISCAPTUREAVAILABLE, m_cbImageType);
	DDX_Control(pDX, IDC_COMBO_SETLEOPERATIONMODE, m_cbSetLEOperationMode);
	//DDX_Control(pDX, IDC_COMBO_SETLEDS, m_cbSetLEDs);
	DDX_Control(pDX, IDC_FRAME_IMAGE, m_staticView);
	DDX_Check(pDX, IDC_CHECK_GUIDELINE, m_chkGuideLineValue);
	DDX_Check(pDX, IDC_CHECK_FINGERCOUNT, m_chkIgnoreNumberofFinger);
	DDX_Control(pDX, IDC_LIST_ETC, m_listEtc);
	DDX_Check(pDX, IDC_CHECK_APPENDRESULT, m_chkAppendFunctionResult);
	DDX_Control(pDX, IDC_ICON_PREVIEWIMAGE_OFF, m_iconPreview);
	DDX_Control(pDX, IDC_ICON_INITPROGRESS_OFF, m_iconInitProgress);
	DDX_Control(pDX, IDC_ICON_TAKINGACQUISITION_OFF, m_iconTakingAcquisition);
	DDX_Control(pDX, IDC_ICON_COMPLETEACQUISITION_OFF, m_iconCompleteAcquisition);
	DDX_Control(pDX, IDC_ICON_RESULTIMAGE_OFF, m_iconResultImageEx);
	DDX_Control(pDX, IDC_ICON_FINGERCOUNT_OFF, m_iconFingerCount);
	DDX_Control(pDX, IDC_ICON_FINGERQUALITY_OFF, m_iconFingerQuality);
	DDX_Control(pDX, IDC_ICON_DCOMBRE_OFF, m_iconDeviceCommunicationBreak);
	DDX_Control(pDX, IDC_ICON_DEVICECOUNT_OFF, m_iconDeviceCount);
	DDX_Control(pDX, IDC_ICON_CLEARPLATE_OFF, m_iconClearPlatenAtCapture);
}

BEGIN_MESSAGE_MAP(CIBSDKFunctionTesterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	//}}AFX_MSG_MAP
ON_BN_CLICKED(IDC_BUTTON_SDKVERSION, &CIBSDKFunctionTesterDlg::OnBnClickedButtonSdkversion)
ON_BN_CLICKED(IDC_BUTTON_GETDEVICECOUNT, &CIBSDKFunctionTesterDlg::OnBnClickedButtonGetdevicecount)
ON_BN_CLICKED(IDC_BUTTON_DEVICEDESCRIPTION, &CIBSDKFunctionTesterDlg::OnBnClickedButtonDevicedescription)
ON_BN_CLICKED(IDC_BUTTON_OPENDEVICE, &CIBSDKFunctionTesterDlg::OnBnClickedButtonOpendevice)
ON_BN_CLICKED(IDC_BUTTON_DEVICECLOSE, &CIBSDKFunctionTesterDlg::OnBnClickedButtonDeviceclose)
ON_BN_CLICKED(IDC_BUTTON_ISDEVICEOPEN, &CIBSDKFunctionTesterDlg::OnBnClickedButtonIsdeviceopen)
ON_BN_CLICKED(IDC_BUTTON_GETPROPERTY, &CIBSDKFunctionTesterDlg::OnBnClickedButtonGetproperty)
ON_BN_CLICKED(IDC_BUTTON_SETPROPERTY, &CIBSDKFunctionTesterDlg::OnBnClickedButtonSetproperty)
ON_BN_CLICKED(IDC_BUTTON_START, &CIBSDKFunctionTesterDlg::OnBnClickedButtonStart)
ON_BN_CLICKED(IDC_BUTTON_STOP, &CIBSDKFunctionTesterDlg::OnBnClickedButtonStop)
ON_BN_CLICKED(IDC_BUTTON_ISCAPTUREAVAILABLE, &CIBSDKFunctionTesterDlg::OnBnClickedButtonIscaptureavailable)
ON_BN_CLICKED(IDC_BUTTON_ISCAPTUREACTIVE, &CIBSDKFunctionTesterDlg::OnBnClickedButtonIscaptureactive)
ON_BN_CLICKED(IDC_BUTTON_GETCONTRAST, &CIBSDKFunctionTesterDlg::OnBnClickedButtonGetcontrast)
ON_BN_CLICKED(IDC_BUTTON_SETCONTRAST, &CIBSDKFunctionTesterDlg::OnBnClickedButtonSetcontrast)
ON_BN_CLICKED(IDC_BUTTON_GETLEOPERATIONMODE, &CIBSDKFunctionTesterDlg::OnBnClickedButtonGetleoperationmode)
ON_BN_CLICKED(IDC_BUTTON_SETLEOPERATION, &CIBSDKFunctionTesterDlg::OnBnClickedButtonSetleoperation)
ON_BN_CLICKED(IDC_BUTTON_ISTOUCHFINGER, &CIBSDKFunctionTesterDlg::OnBnClickedButtonIstouchfinger)
ON_BN_CLICKED(IDC_BUTTON_GETOPERABLELEDS, &CIBSDKFunctionTesterDlg::OnBnClickedButtonGetoperableleds)
ON_BN_CLICKED(IDC_BUTTON_GETLEDS, &CIBSDKFunctionTesterDlg::OnBnClickedButtonGetleds)
ON_BN_CLICKED(IDC_BUTTON_SETLEDS, &CIBSDKFunctionTesterDlg::OnBnClickedButtonSetleds)
ON_BN_CLICKED(IDC_CHECK_CLIENT_WINDOW, &CIBSDKFunctionTesterDlg::OnBnClickedCheckDisplayWindow)
ON_BN_CLICKED(IDC_CHECK_GUIDELINE, &CIBSDKFunctionTesterDlg::OnBnClickedCheckGuideline)
ON_BN_CLICKED(IDC_CHECK_FINGERCOUNT, &CIBSDKFunctionTesterDlg::OnBnClickedCheckFingercount)
ON_BN_CLICKED(IDC_CHECK_AUTO_CAPTURE, &CIBSDKFunctionTesterDlg::OnBnClickedCheckAutoCapture)
ON_BN_CLICKED(IDC_CHECK__AUTO_CONTRAST, &CIBSDKFunctionTesterDlg::OnBnClickedCheck)
ON_BN_CLICKED(IDC_BUTTON_MANUALCAPTURE, &CIBSDKFunctionTesterDlg::OnBnClickedButtonManualcapture)
ON_BN_CLICKED(IDC_CHECK_APPENDRESULT, &CIBSDKFunctionTesterDlg::OnBnClickedCheckAppendresult)
ON_BN_CLICKED(IDC_BUTTON_BACKCL, &CIBSDKFunctionTesterDlg::OnBnClickedButtonBackcl)
ON_BN_CLICKED(IDC_BUTTON_SET_DISPLAY_OVERLAY_TEXT, &CIBSDKFunctionTesterDlg::OnBnClickedButtonSetDisplayOverlayText)
ON_WM_DESTROY()
ON_WM_CLOSE()
ON_BN_CLICKED(IDC_BUTTON_AGING_TEST, &CIBSDKFunctionTesterDlg::OnBnClickedButtonAgingTest)
END_MESSAGE_MAP()


// CIBSDKFunctionTesterDlg message handlers

UINT CIBSDKFunctionTesterDlg::_InitializeDeviceThreadCallback( LPVOID pParam )
{   
	if( pParam == NULL )
		return 1;

	CIBSDKFunctionTesterDlg* pDlg = (CIBSDKFunctionTesterDlg*)pParam;

	int		devIndex = pDlg->GetDlgItemInt(IDC_EDIT_DEVICE);
	int		devHandle;
	int		nRc;
	CString temp;
	int		pos=0;

	nRc = IBSU_OpenDevice(devIndex, &devHandle);

	if( nRc >= IBSU_STATUS_OK )
	{
		// register callback functions
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, OnEvent_DeviceCommunicationBreak, pDlg );//Ok    
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, OnEvent_PreviewImage, pDlg );//Ok
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, OnEvent_TakingAcquisition, pDlg );//Ok  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, OnEvent_CompleteAcquisition, pDlg );//Ok  
/****
 ** This IBSU_CallbackResultImage callback was deprecated since 1.7.0
 ** Please consider using IBSU_CallbackResultImageEx instead
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE, OnEvent_ResultImage, pDlg );  //Ok
*/
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX, OnEvent_ResultImageEx, pDlg );  //Ok
        IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, OnEvent_FingerCount, pDlg );  //Ok
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY, OnEvent_FingerQuality, pDlg ); //Ok 
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE, OnEvent_ClearPlatenAtCapture, pDlg );
	}

	pDlg->_SetResultMessageOnListView(pDlg->m_IsAppend, "-- IBSU_OpenDevice -- Return value (%d)", nRc);
	pDlg->_SetAdditionalMessageOnListView(pos++, "Device handle : %d", devHandle);

	return 0;
}

int CIBSDKFunctionTesterDlg::_GetCurrDeviceHandle()
{
	int	devHandle = GetDlgItemInt(IDC_EDIT_HANDLE);

	return devHandle;
}

void CIBSDKFunctionTesterDlg::_SetResultMessageOnListView(BOOL bAppend, LPCTSTR Format, ...)
{
	m_listEtc.DeleteAllItems();

	if( !bAppend )
	{
		m_listMain.DeleteAllItems();
		m_nRowOnListView = 0;
	}

	TCHAR cMessage[IBSU_MAX_STR_LEN+1];
	va_list arg_ptr;

	va_start( arg_ptr, Format );
	_vsntprintf( cMessage, IBSU_MAX_STR_LEN, Format, arg_ptr );
	va_end( arg_ptr );  

	CString strMessage;
	strMessage.Format(_T("%s"), cMessage);
	m_listMain.InsertItem(m_nRowOnListView, strMessage);
	m_nRowOnListView++;
}

void CIBSDKFunctionTesterDlg::_SetAdditionalMessageOnListView(int row, LPCTSTR Format, ...)
{
	if( row == 0 )
		m_listEtc.DeleteAllItems();

	TCHAR cMessage[IBSU_MAX_STR_LEN+1];
	va_list arg_ptr;

	va_start( arg_ptr, Format );
	_vsntprintf( cMessage, IBSU_MAX_STR_LEN, Format, arg_ptr );
	va_end( arg_ptr );  

	CString strMessage;
	strMessage.Format(_T("%s"), cMessage);
	m_listEtc.InsertItem(row, strMessage);
}

void CIBSDKFunctionTesterDlg::_SetCoordinateControl(WINDOWPLACEMENT *place, int nID, int left, int top, int width, int height)
{
	CWnd *pWnd = GetDlgItem(nID);

	place->rcNormalPosition.left = left;
	place->rcNormalPosition.top = top;
	place->rcNormalPosition.right = left + width;
	place->rcNormalPosition.bottom = top + height;

	pWnd->SetWindowPlacement(place);
}






BOOL CIBSDKFunctionTesterDlg::OnInitDialog()
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

	// TODO: Add extra initialization here
	m_nRowOnListView = 0;


	m_ToolTip.Create(this);
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_OPENDEVICE), "It is open the Device.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	
	Init_Layout();

	ToolTip();

	GetDlgItem( IDC_BUTTON_BACKCL )->EnableWindow( FALSE );
	GetDlgItem( IDC_CHECK_GUIDELINE )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_SET_DISPLAY_OVERLAY_TEXT )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_BKRANGE )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_POSX )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_POSX )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_POSY )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_POSY )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_TEXTCOLOR )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_TEXTCOLOR )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_RED )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_GREEN )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_BLACK )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_GREEN )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_BLACK )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_ADDTEXT )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_ADDTEXT )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_BKRED )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_BKGREEN )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_BKBLUE )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_BKRED )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_BKGREEN )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_BKBLUE )->EnableWindow( FALSE );

	GetDlgItem( IDC_BUTTON_MANUALCAPTURE )->EnableWindow( FALSE );
	//GetDlgItem( IDC_BUTTON_STOP )->EnableWindow( FALSE );

	//Input value to EditBox
	GetDlgItem( IDC_EDIT_DEVICE )->SetWindowText("0");
	GetDlgItem( IDC_EDIT_HANDLE )->SetWindowText("0");
	GetDlgItem( IDC_EDIT_SETLED )->SetWindowText("FFFFFFFF");
	GetDlgItem( IDC_EDIT_POSX )->SetWindowText("10");
	GetDlgItem( IDC_EDIT_POSY )->SetWindowText("10");
	GetDlgItem( IDC_EDIT_SETCONTRAST )->SetWindowText("21");
	GetDlgItem( IDC_EDIT_TEXTCOLOR )->SetWindowText("0");
	GetDlgItem( IDC_EDIT_GREEN )->SetWindowText("0");
	GetDlgItem( IDC_EDIT_BLACK )->SetWindowText("255");
	GetDlgItem( IDC_EDIT_BKRED )->SetWindowText("128");
	GetDlgItem( IDC_EDIT_BKGREEN )->SetWindowText("128");
	GetDlgItem( IDC_EDIT_BKBLUE )->SetWindowText("128");
	GetDlgItem( IDC_EDIT_BKBLUE )->SetWindowText("128");
	GetDlgItem( IDC_EDIT_ADDTEXT )->SetWindowText("Default Text");
	
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, OnEvent_DeviceCount, this );
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, OnEvent_InitProgress, this );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIBSDKFunctionTesterDlg::Init_Layout()
{
	m_hIcon_On = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON_ON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	m_hIcon_Off = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON_OFF), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	WINDOWPLACEMENT		place;

	SetWindowPos(NULL, 0,0, 1020, 730, SWP_NOMOVE);
	GetWindowPlacement(&place);        

	//LOGO
	_SetCoordinateControl(&place, IDC_STATIC_LOGO, 0, 0, 1024, 75);


	int		left = 10, top = 90;
	const int	basicWidth = 140, basicHeight = 20;
	const int	wideWidth = 205;
	const int	smallWidth = 75;
	const int	groupWidth = 320;
	const int	iconWidth = 16, iconHeight = 16;
	RECT	*rc = &place.rcNormalPosition;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// 1st column

	//GetSDKVersion Button
	_SetCoordinateControl(&place, IDC_BUTTON_SDKVERSION, left+15, top, basicWidth, basicHeight);
	//GetDeviceCount Button
	_SetCoordinateControl(&place, IDC_BUTTON_GETDEVICECOUNT, rc->right+15, rc->top, basicWidth, basicHeight);


	//////////////////////////////////////////////////////////////////////////////////////////////////
	//IDC_STATIC_MAININTERFACE (Group box control)
	_SetCoordinateControl(&place, IDC_STATIC_MAININTERFACE, left, rc->bottom+10, groupWidth, 215);

	//static Device index
	_SetCoordinateControl(&place, IDC_STATIC_DEVICE, left+15, rc->top+20, 70, basicHeight);
	//Device index EditBox
	_SetCoordinateControl(&place, IDC_EDIT_DEVICE, rc->right, rc->top, 70, basicHeight);
	//GetDeviceDescriptionButton
	_SetCoordinateControl(&place, IDC_BUTTON_DEVICEDESCRIPTION, rc->right+15, rc->top, basicWidth, basicHeight);

	//DeviceOpen Button
	_SetCoordinateControl(&place, IDC_BUTTON_OPENDEVICE, left+15+basicWidth+15, rc->bottom+5, basicWidth, basicHeight);

	//static Device handle
	_SetCoordinateControl(&place, IDC_STATIC_HANDLE, left+15, rc->bottom+15, 70, basicHeight);
	//Device handle EditBox
	_SetCoordinateControl(&place, IDC_EDIT_HANDLE, rc->right, rc->top, 70, basicHeight);
	//CloseDevice Button
	_SetCoordinateControl(&place, IDC_BUTTON_DEVICECLOSE, rc->right+15, rc->top, basicWidth, basicHeight);
//	int bottomForDeviceDescBtn = place.rcNormalPosition.bottom = place.rcNormalPosition.top + 30;

	//IsDeviceOpenedButton
	_SetCoordinateControl(&place, IDC_BUTTON_ISDEVICEOPEN, left+15+basicWidth+15, rc->bottom+5, basicWidth, basicHeight);

	//PropertyCBox	
	_SetCoordinateControl(&place, IDC_COMBO_PROPERTY, left+15, rc->bottom+10, basicWidth*2+15, basicHeight);

	//GetPropertyButton
	_SetCoordinateControl(&place, IDC_BUTTON_GETPROPERTY, left+15, rc->bottom+5, basicWidth, basicHeight);

	//SetPropertyButton
	_SetCoordinateControl(&place, IDC_BUTTON_SETPROPERTY, left+15, rc->bottom+5, basicWidth, basicHeight);
	//Proderty Value Edit
	_SetCoordinateControl(&place, IDC_EDIT_SETPROPERTY, rc->right+15, rc->top, basicWidth, basicHeight);

	//Static Property Value
//	_SetCoordinateControl(&place, IDC_STATIC_PROPERTYVALUE, left+15, rc->bottom+5, smallWidth, basicHeight);


	//////////////////////////////////////////////////////////////////////////////////////////////////
	//LEOperation GroupBox
	_SetCoordinateControl(&place, IDC_STATIC_LEOPERATION, left, rc->bottom+20, groupWidth, 75);

	//GetLEOperationMode Button
	_SetCoordinateControl(&place, IDC_BUTTON_GETLEOPERATIONMODE, left+15, rc->top+20, basicWidth, basicHeight);

	//SetLEOperationMode Button
	_SetCoordinateControl(&place, IDC_BUTTON_SETLEOPERATION, left+15, rc->bottom+5, basicWidth, basicHeight);
	//SetLEOperationMode CBox
	_SetCoordinateControl(&place, IDC_COMBO_SETLEOPERATIONMODE, rc->right+15, rc->top, basicWidth, basicHeight);


	//////////////////////////////////////////////////////////////////////////////////////////////////
	//LED Operation Group Box
	_SetCoordinateControl(&place, IDC_STATIC_LED, left, rc->bottom+20, groupWidth, 75);

	//GetOperableLEDs Button
	_SetCoordinateControl(&place, IDC_BUTTON_GETOPERABLELEDS, left+15, rc->top+20, basicWidth, basicHeight);
	//GetLEDs Button
	_SetCoordinateControl(&place, IDC_BUTTON_GETLEDS, rc->right+15, rc->top, basicWidth, basicHeight);

	//SetLEDs Button
	_SetCoordinateControl(&place, IDC_BUTTON_SETLEDS, left+15, rc->bottom+5, basicWidth, basicHeight);
	//SetLEDs EditBox
	_SetCoordinateControl(&place, IDC_EDIT_SETLED, rc->right+15, rc->top, 80, basicHeight);
	//SetLEDs Static Box
	_SetCoordinateControl(&place, IDC_STATIC_HEX, rc->right, rc->top, 60, basicHeight);


	//////////////////////////////////////////////////////////////////////////////////////////////////
	//User defined visualization Group Box
	_SetCoordinateControl(&place, IDC_STATIC_DISPLAY, left, rc->bottom+20, groupWidth, 175);

	//User define Window CheckBox
	_SetCoordinateControl(&place, IDC_CHECK_CLIENT_WINDOW, left+15, rc->top+20, basicWidth, basicHeight);

	//Back color Button
	_SetCoordinateControl(&place, IDC_BUTTON_BACKCL, left+15, rc->bottom+5, basicWidth, basicHeight);
	//Static Red
	_SetCoordinateControl(&place, IDC_STATIC_BKRED, rc->right+15, rc->top, 10, basicHeight);
	//Edit BKRed
	_SetCoordinateControl(&place, IDC_EDIT_BKRED, rc->right, rc->top, 30, basicHeight);
	//Static Green
	_SetCoordinateControl(&place, IDC_STATIC_BKGREEN, rc->right+5, rc->top, 10, basicHeight);
	//Edit BKGreen
	_SetCoordinateControl(&place, IDC_EDIT_BKGREEN, rc->right, rc->top, 30, basicHeight);
	//Static Blue
	_SetCoordinateControl(&place, IDC_STATIC_BKBLUE, rc->right+5, rc->top, 10, basicHeight);
	//Edit BKBlue
	_SetCoordinateControl(&place, IDC_EDIT_BKBLUE, rc->right, rc->top, 30, basicHeight);

	//UseGuidLine Chk
	_SetCoordinateControl(&place, IDC_CHECK_GUIDELINE, left+15, rc->bottom+5, basicWidth, basicHeight);
	//static BK Range
	_SetCoordinateControl(&place, IDC_STATIC_BKRANGE, rc->right+110, rc->top, 40, basicHeight);

	//Set display overlay text button
	_SetCoordinateControl(&place, IDC_BUTTON_SET_DISPLAY_OVERLAY_TEXT, left+15, rc->bottom+5, basicWidth, basicHeight);
	//static PosX
	_SetCoordinateControl(&place, IDC_STATIC_POSX, rc->right+15, rc->top, 10, basicHeight);
	//edit PosX
	_SetCoordinateControl(&place, IDC_EDIT_POSX, rc->right, rc->top, 30, basicHeight);
	//static PosY
	_SetCoordinateControl(&place, IDC_STATIC_POSY, rc->right+5, rc->top, 10, basicHeight);
	//edit PosY
	_SetCoordinateControl(&place, IDC_EDIT_POSY, rc->right, rc->top, 30, basicHeight);

	//static Fore Color
	_SetCoordinateControl(&place, IDC_STATIC_TEXTCOLOR, left+15, rc->bottom+5, basicWidth, basicHeight);
	//Static Red
	_SetCoordinateControl(&place, IDC_STATIC_RED, rc->right+15, rc->top, 10, basicHeight);
	//edit Color
	_SetCoordinateControl(&place, IDC_EDIT_TEXTCOLOR, rc->right, rc->top, 30, basicHeight);
	//Static Green
	_SetCoordinateControl(&place, IDC_STATIC_GREEN, rc->right+5, rc->top, 10, basicHeight);
	//edit Color
	_SetCoordinateControl(&place, IDC_EDIT_GREEN, rc->right, rc->top, 30, basicHeight);
	//Static Black
	_SetCoordinateControl(&place, IDC_STATIC_BLACK, rc->right+5, rc->top, 10, basicHeight);
	//edit Color
	_SetCoordinateControl(&place, IDC_EDIT_BLACK, rc->right, rc->top, 30, basicHeight);

	//Static AddText 
	_SetCoordinateControl(&place, IDC_STATIC_ADDTEXT, left+15, rc->bottom+5, smallWidth, basicHeight);
	//EditBox AddText
	_SetCoordinateControl(&place, IDC_EDIT_ADDTEXT, rc->right+15, rc->top, wideWidth, basicHeight);


	//////////////////////////////////////////////////////////////////////////////////////////////////
	// 2nd column
	left = groupWidth+20;
	//Image Capture GroupBox
	_SetCoordinateControl(&place, IDC_STATIC_IMAGE, left, top, groupWidth, 165);

	//static Type
	_SetCoordinateControl(&place, IDC_STATIC_IMAGETYPE, left+15, rc->top+20, smallWidth, basicHeight);
	//CaptureType ComboBox
	_SetCoordinateControl(&place, IDC_COMBO_ISCAPTUREAVAILABLE, rc->right+15, rc->top, wideWidth, basicHeight);

	//AutoContrast Chk
	_SetCoordinateControl(&place, IDC_CHECK__AUTO_CONTRAST, left+15, rc->bottom, basicWidth*2, basicHeight);

	//AutoCapture Chk
	_SetCoordinateControl(&place, IDC_CHECK_AUTO_CAPTURE, left+15, rc->bottom, basicWidth*2, basicHeight);

	//IgnoreNumberofFinger CheckBox
	_SetCoordinateControl(&place, IDC_CHECK_FINGERCOUNT, left+15, rc->bottom, basicWidth*2, basicHeight);

	//CaptureStart Button
	_SetCoordinateControl(&place, IDC_BUTTON_START, left+15, rc->bottom+10, basicWidth, basicHeight);
	//CaptureStop Button
	_SetCoordinateControl(&place, IDC_BUTTON_STOP, rc->right+15, rc->top, basicWidth, basicHeight);

	//CaptureManually Button
	_SetCoordinateControl(&place, IDC_BUTTON_MANUALCAPTURE, left+basicWidth, rc->bottom+5, basicWidth+30, basicHeight);


	//////////////////////////////////////////////////////////////////////////////////////////////////
	//Check Capture status group box
	_SetCoordinateControl(&place, IDC_STATIC_CAPTURE, left, rc->bottom+20, groupWidth, 75);

	//IsCaptureAbailable Button
	_SetCoordinateControl(&place, IDC_BUTTON_ISCAPTUREAVAILABLE, left+15, rc->top+20, basicWidth, basicHeight);
	//IsCaptureActive Button
	_SetCoordinateControl(&place, IDC_BUTTON_ISCAPTUREACTIVE, rc->right+15, rc->top, basicWidth, basicHeight);

	//IsTouchFinger Button
	_SetCoordinateControl(&place, IDC_BUTTON_ISTOUCHFINGER, left+15+basicWidth+15, rc->bottom+5, basicWidth, basicHeight);


	//////////////////////////////////////////////////////////////////////////////////////////////////
	//Contrast static group box
	_SetCoordinateControl(&place, IDC_STATIC_CONTRAST, left, rc->bottom+20, groupWidth, 75);

	//GetContrast Button
	_SetCoordinateControl(&place, IDC_BUTTON_GETCONTRAST, left+15, rc->top+20, basicWidth, basicHeight);

	//SetContrast Button
	_SetCoordinateControl(&place, IDC_BUTTON_SETCONTRAST, left+15, rc->bottom+5, basicWidth, basicHeight);
	//Setcontrast Edit
	_SetCoordinateControl(&place, IDC_EDIT_SETCONTRAST, rc->right+15, rc->top, 80, basicHeight);
	//Setcontrast Range
	_SetCoordinateControl(&place, IDC_STATIC_SETCONTRAST, rc->right, rc->top, 60, basicHeight);


	//////////////////////////////////////////////////////////////////////////////////////////////////
	//Capture Screen
	_SetCoordinateControl(&place, IDC_FRAME_IMAGE, left, rc->bottom+20, groupWidth, 250);


	//////////////////////////////////////////////////////////////////////////////////////////////////
	// 3rd column
	left = groupWidth+20 + groupWidth+10;
	//Callback fired GroupBox
	_SetCoordinateControl(&place, IDC_STATIC_CALLBACK, left, top, groupWidth+15, 248);

	//InitProgress On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_INITPROGRESS_OFF, left+10, rc->top+20, iconWidth, iconHeight);
	//InitProgress Static
	_SetCoordinateControl(&place, IDC_STATIC_INITPROGRESS, rc->right+5, rc->top, basicWidth, basicHeight);
	//InitProgress edit Box
	_SetCoordinateControl(&place, IDC_EDIT_INITPROGRESS, rc->right, rc->top, basicWidth+15, basicHeight);

	//PreviewImage On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_PREVIEWIMAGE_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//PreviewImage Static
	_SetCoordinateControl(&place, IDC_STATIC_PREVIEWIMAGE, rc->right+5, rc->top, basicWidth, basicHeight);
	//PreviewImage edit Box
	_SetCoordinateControl(&place, IDC_EDIT_PREVIEWIMAGE, rc->right, rc->top, basicWidth+15, basicHeight);

	//TakingAcquition On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_TAKINGACQUISITION_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//TakingAcquition Static
	_SetCoordinateControl(&place, IDC_STATIC_TAKINGACQUISITION, rc->right+5, rc->top, basicWidth, basicHeight);

	//CompletAcquition On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_COMPLETEACQUISITION_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//CompletAcquition Static
	_SetCoordinateControl(&place, IDC_STATIC_COMPLETEACQUISITION, rc->right+5, rc->top, basicWidth, basicHeight);

	//ResultImage On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_RESULTIMAGE_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//ResultImage Static
	_SetCoordinateControl(&place, IDC_STATIC_RESULTIMAGE, rc->right+5, rc->top, basicWidth, basicHeight);
	//ResultImage edit Box
	_SetCoordinateControl(&place, IDC_EDIT_RESULTIMAGE, rc->right, rc->top, basicWidth+15, basicHeight);

	//FingerCount On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_FINGERCOUNT_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//FingerCount Static
	_SetCoordinateControl(&place, IDC_STATIC_FINGERCOUNT, rc->right+5, rc->top, basicWidth, basicHeight);
	//FingerCount edit Box
	_SetCoordinateControl(&place, IDC_EDIT_FINGERCOUNT, rc->right, rc->top, basicWidth+15, basicHeight);

	//FingerQuality On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_FINGERQUALITY_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//FingerQuality Static
	_SetCoordinateControl(&place, IDC_STATIC_FINGERQUALITY, rc->right+5, rc->top, basicWidth, basicHeight);
	//FingerQuality edit Box
	_SetCoordinateControl(&place, IDC_EDIT_FINGERQUALITY, rc->right, rc->top, basicWidth+15, basicHeight);

	//DeviceCommunicationBreak On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_DCOMBRE_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//DeviceCommunicationBreak Static
	_SetCoordinateControl(&place, IDC_STATIC_DCOMBRE, rc->right+5, rc->top, basicWidth, basicHeight);

	//DeviceCount On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_DEVICECOUNT_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//DeviceCount Static
	_SetCoordinateControl(&place, IDC_STATIC_DEVICECOUNT, rc->right+5, rc->top, basicWidth, basicHeight);
	//DeviceCount edit Box
	_SetCoordinateControl(&place, IDC_EDIT_DEVICECOUNT, rc->right, rc->top, basicWidth+15, basicHeight);

	//ClearPlaten On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_CLEARPLATE_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//ClearPlaten Static
	_SetCoordinateControl(&place, IDC_STATIC_CLEARPLATE, rc->right+5, rc->top, basicWidth, basicHeight);
	//ClearPlaten edit Box
	_SetCoordinateControl(&place, IDC_EDIT_CLEARPLATE, rc->right, rc->top, basicWidth+15, basicHeight);

	//AppendResult ChkBox
	_SetCoordinateControl(&place, IDC_CHECK_APPENDRESULT, left, rc->bottom+20, groupWidth+15, basicHeight);

	//m_listMain
	m_listMain.InsertColumn(0, "Function Result", LVCFMT_LEFT, groupWidth+10);
	_SetCoordinateControl(&place, IDC_LIST_MAIN, left, rc->bottom+5, groupWidth+15, 155);

	//m_listEtc
	m_listEtc.InsertColumn(0, "Additional information", LVCFMT_LEFT, groupWidth+10);
	_SetCoordinateControl(&place, IDC_LIST_ETC, left, rc->bottom+5, groupWidth+15, 155);
	
	//Property CBox
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_PRODUCT_ID");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_SERIAL_NUMBER");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_VENDOR_ID");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_IBIA_VENDOR_ID");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_IBIA_VERSION");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_IBIA_DEVICE_ID");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_FIRMWARE");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_REVISION");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_PRODUCTION_DATE");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_SERVICE_DATE");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_IMAGE_WIDTH");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_IMAGE_HEIGHT");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_IGNORE_FINGER_TIME");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_RECOMMENDED_LEVEL");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_POLLINGTIME_TO_BGETIMAGE");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_ENABLE_POWER_SAVE_MODE");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_RETRY_WRONG_COMMUNICATION");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_CAPTURE_TIMEOUT");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_ROLL_MIN_WIDTH");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_ROLL_MODE");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_ROLL_LEVEL");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_CAPTURE_AREA_THRESHOLD");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_ENABLE_DECIMATION");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_ENABLE_CAPTURE_ON_RELEASE");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_DEVICE_INDEX");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_DEVICE_ID");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_SUPER_DRY_MODE");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_MIN_CAPTURE_TIME_IN_SUPER_DRY_MODE");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_ROLLED_IMAGE_WIDTH");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_ROLLED_IMAGE_HEIGHT");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_NO_PREVIEW_IMAGE");
    m_cbProperty.AddString("ENUM_IBSU_PROPERTY_ROLL_IMAGE_OVERRIDE");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_WARNING_MESSAGE_INVALID_AREA");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_ENABLE_WET_FINGER_DETECT");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_WET_FINGER_DETECT_LEVEL");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_WET_FINGER_DETECT_LEVEL_THRESHOLD");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_START_POSITION_OF_ROLLING_AREA");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_START_ROLL_WITHOUT_LOCK");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_ENABLE_TOF");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_ENABLE_ENCRYPTION");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_RESERVED_ENABLE_TOF_FOR_ROLL");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_RESERVED_CAPTURE_BRIGHTNESS_THRESHOLD_FOR_FLAT");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_RESERVED_CAPTURE_BRIGHTNESS_THRESHOLD_FOR_ROLL");
	m_cbProperty.AddString("ENUM_IBSU_PROPERTY_RESERVED_ENHANCED_RESULT_IMAGE");
	m_cbProperty.SetCurSel(0);


	//ImageType CBox
	m_cbImageType.AddString("ENUM_IBSU_TYPE_NONE");
	m_cbImageType.AddString("ENUM_IBSU_ROLL_SINGLE_FINGER");
	m_cbImageType.AddString("ENUM_IBSU_FLAT_SINGLE_FINGER");
	m_cbImageType.AddString("ENUM_IBSU_FLAT_TWO_FINGERS");
	m_cbImageType.AddString("ENUM_IBSU_FLAT_FOUR_FINGERS");
	m_cbImageType.AddString("ENUM_IBSU_FLAT_THREE_FINGERS");
	m_cbImageType.SetCurSel(2);

	//SetLEOperationMode CBox
	m_cbSetLEOperationMode.AddString("LE_OPERATION_AUTO");
	m_cbSetLEOperationMode.AddString("LE_OPERATION_ON");
	m_cbSetLEOperationMode.AddString("LE_OPERATION_OFF");
	m_cbSetLEOperationMode.SetCurSel(0);
	
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIBSDKFunctionTesterDlg::OnPaint()
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
		CPaintDC dc(this);

		CPen NewPen(PS_SOLID, 3, RGB(192,0,0));

		CPen* pOldPen = dc.SelectObject(&NewPen);

		CBrush NewBrush(RGB(192,192,192));
		CBrush* pOldBrush = dc.SelectObject(&NewBrush);

		dc.Rectangle(120, 10, 220,110);

		dc.SelectObject(pOldPen);

		dc.SelectObject(pOldBrush);

	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIBSDKFunctionTesterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CIBSDKFunctionTesterDlg::OnBnClickedButtonSdkversion()
{
	int		pos=0;
	IBSU_SdkVersion verInfo;

	int nRc = IBSU_GetSDKVersion(&verInfo);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_GetSDKVersion -- Return value (%d)", nRc);

	_SetAdditionalMessageOnListView(pos++, "Product : %s", verInfo.Product);
	_SetAdditionalMessageOnListView(pos++, "File : %s", verInfo.File);
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonGetdevicecount()
{
	int		pos=0;
	int		devices = 0;

	int	nRc = IBSU_GetDeviceCount(&devices);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_GetDeviceDescription -- Return value (%d)", nRc);

	_SetAdditionalMessageOnListView(pos++, "Number of Device(s) : %d", devices);
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonDevicedescription()
{
	int		pos=0;
	int		devIndex = GetDlgItemInt(IDC_EDIT_DEVICE);
	IBSU_DeviceDesc devDesc;

	int nRc = IBSU_GetDeviceDescription(devIndex, &devDesc);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_GetDeviceDescription -- Return value (%d)", nRc);

	_SetAdditionalMessageOnListView(pos++, "Serial Number : %s", devDesc.serialNumber);
	_SetAdditionalMessageOnListView(pos++, "Product Name : %s", devDesc.productName);
	_SetAdditionalMessageOnListView(pos++, "Interface Type : %s", devDesc.interfaceType);
	_SetAdditionalMessageOnListView(pos++, "Firmware Version : %s", devDesc.fwVersion);
	_SetAdditionalMessageOnListView(pos++, "Device Revision : %s", devDesc.devRevision);
	_SetAdditionalMessageOnListView(pos++, "Device Handle : %d", devDesc.handle);
	_SetAdditionalMessageOnListView(pos++, "Handle Opened : %d", devDesc.IsHandleOpened);
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonOpendevice()
{
	VERIFY( ::AfxBeginThread( _InitializeDeviceThreadCallback, this, THREAD_PRIORITY_NORMAL,
							  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonDeviceclose()
{
	int devHandle = _GetCurrDeviceHandle();

	int nRc = IBSU_CloseDevice(devHandle);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_CloseDevice -- Return value (%d)", nRc);
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonIsdeviceopen()
{
	int nRc = IBSU_IsDeviceOpened(_GetCurrDeviceHandle());

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_IsDeviceOpened -- Return value (%d)", nRc);
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonGetproperty()
{
	int		pos=0;
	CString	propertyName;
	char	propertyValue[IBSU_MAX_STR_LEN];
	int		propertyIndex;
	int nRc;

	UpdateData(FALSE);
	propertyIndex = m_cbProperty.GetCurSel();
	GetDlgItemText(IDC_COMBO_PROPERTY, propertyName);

	if(propertyName.Find("RESERVED") > -1)
		nRc = IBSU_GetProperty(_GetCurrDeviceHandle(), (IBSU_PropertyId)(propertyIndex+400-ENUM_IBSU_PROPERTY_ENABLE_TOF), propertyValue);
	else
		nRc = IBSU_GetProperty(_GetCurrDeviceHandle(), (IBSU_PropertyId)propertyIndex, propertyValue);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_GetProperty -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		_SetAdditionalMessageOnListView(pos++, "Property value : %s", propertyValue);
	}
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonSetproperty()
{
	int		pos=0;
	CString	propertyValue, propertyName;
	int		propertyIndex;
	int		nRc;

	UpdateData(FALSE);
	propertyIndex = m_cbProperty.GetCurSel();
	GetDlgItemText(IDC_EDIT_SETPROPERTY, propertyValue);
	GetDlgItemText(IDC_COMBO_PROPERTY, propertyName);

	if(propertyName.Find("RESERVED") > -1)
		nRc = RESERVED_SetProperty(_GetCurrDeviceHandle(), "#partner1042!", (IBSU_PropertyId)(propertyIndex+400-ENUM_IBSU_PROPERTY_ENABLE_TOF), propertyValue);
	else
		nRc = IBSU_SetProperty(_GetCurrDeviceHandle(), (IBSU_PropertyId)propertyIndex, propertyValue);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_SetProperty -- Return value (%d)", nRc);
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonStart()
{
	UpdateData(FALSE);

	const IBSU_ImageType imgType = (IBSU_ImageType)m_cbImageType.GetCurSel();
	const IBSU_ImageResolution imgRes = ENUM_IBSU_IMAGE_RESOLUTION_500;		// Currently we only support 500ppi.
	DWORD captureOptions = 0;


	if( m_chkAutoContrast )
		captureOptions |= IBSU_OPTION_AUTO_CONTRAST;
	if( m_chkAutoCapture )
		captureOptions |= IBSU_OPTION_AUTO_CAPTURE;
	if( m_chkIgnoreNumberofFinger )
		captureOptions |= IBSU_OPTION_IGNORE_FINGER_COUNT;

	int nRc = IBSU_BeginCaptureImage(_GetCurrDeviceHandle(), imgType, imgRes, captureOptions);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_BeginCaptureImage -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
		GetDlgItem( IDC_BUTTON_MANUALCAPTURE )->EnableWindow( TRUE );
}

void CIBSDKFunctionTesterDlg::OnEvent_DeviceCount(
						 const int detectedDevices,
						 void      *pContext
					   )
{
	if( pContext == NULL )
		return;

	CIBSDKFunctionTesterDlg *pDlg = reinterpret_cast<CIBSDKFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	CString result;
	result.Format("%d", detectedDevices);
	pDlg->GetDlgItem(IDC_EDIT_DEVICECOUNT)->SetWindowText(result);

	pDlg->m_iconDeviceCount.SetIcon(pDlg->m_hIcon_On);
	Sleep(300);
	pDlg->m_iconDeviceCount.SetIcon(pDlg->m_hIcon_Off);

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSDKFunctionTesterDlg::OnEvent_FingerQuality(
                          const int                     deviceHandle,   
                          void                          *pContext,       
                          const IBSU_FingerQualityState *pQualityArray, 
                          const int                     qualityArrayCount    
						 )
{
	if( pContext == NULL )
		return;

	CIBSDKFunctionTesterDlg *pDlg = reinterpret_cast<CIBSDKFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	CString temp, result;
	pDlg->m_iconFingerQuality.SetIcon(pDlg->m_hIcon_On);
	Sleep(25);

	for(int i=0; i<qualityArrayCount; i++)
	{
		switch( pQualityArray[i])
		{ 
			case ENUM_IBSU_QUALITY_GOOD:
				result += "(Good)";
				break;          
			case ENUM_IBSU_QUALITY_FAIR :
				result += "(Fair)";
				break;          
			case ENUM_IBSU_QUALITY_POOR :
				result += "(Poor)";
				break;          
			case ENUM_IBSU_QUALITY_INVALID_AREA_TOP :
				result += "(Inv-Top)";
				break;          
			case ENUM_IBSU_QUALITY_INVALID_AREA_LEFT :
				result += "(Inv-Left)";
				break;          
			case ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT :
				result += "(Inv-Right)";
				break;          
			case ENUM_IBSU_FINGER_NOT_PRESENT:
				result += "(Non)";
				break;
			default:
				result += "(Non)";
				break;
		}
	}

	pDlg->GetDlgItem(IDC_EDIT_FINGERQUALITY)->SetWindowText(result);

	pDlg->m_iconFingerQuality.SetIcon(pDlg->m_hIcon_Off);

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSDKFunctionTesterDlg::OnEvent_FingerCount(
						   const int                   deviceHandle,
						   void*                       pContext,
						   const IBSU_FingerCountState fingerCountState
						 )
{
	if( pContext == NULL )
		return;

	CIBSDKFunctionTesterDlg *pDlg = reinterpret_cast<CIBSDKFunctionTesterDlg*>(pContext);
	if( deviceHandle != pDlg->_GetCurrDeviceHandle() )
		ASSERT( FALSE );

	EnterCriticalSection(&g_CriticalSection);
	
	pDlg->m_iconFingerCount.SetIcon(pDlg->m_hIcon_On);

	Sleep(50);
	
	CString fingerState;
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

	pDlg->GetDlgItem(IDC_EDIT_FINGERCOUNT)->SetWindowText(fingerState);

	pDlg->m_iconFingerCount.SetIcon(pDlg->m_hIcon_Off);

	LeaveCriticalSection(&g_CriticalSection);

}

void CIBSDKFunctionTesterDlg::OnEvent_InitProgress(
						 const int deviceIndex,
						 void      *pContext,
						 const int progressValue
					   )
{
	if( pContext == NULL )
		return;

	CIBSDKFunctionTesterDlg *pDlg = reinterpret_cast<CIBSDKFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	{
		// Init Progress callback is very fast, so you have to return directly to get next callback message.
		// Or to reduce your processing time in this callback function
		pDlg->_SetEditValue(progressValue);

		if( progressValue == 100 )
		{
			// Led OFF
			pDlg->m_iconInitProgress.SetIcon(pDlg->m_hIcon_Off);
		}
		else
		{
			// Led ON
			pDlg->m_iconInitProgress.SetIcon(pDlg->m_hIcon_On);
		}
	}
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSDKFunctionTesterDlg::OnEvent_PreviewImage(
						 const int deviceHandle,
						 void*     pContext,
						 const	   IBSU_ImageData image
					   )
{
	if( pContext == NULL )
		return;

	CIBSDKFunctionTesterDlg *pDlg = reinterpret_cast<CIBSDKFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	{
		CString result;
		result.Format("%0.3f ms", image.FrameTime*1000);
		pDlg->GetDlgItem(IDC_EDIT_PREVIEWIMAGE)->SetWindowText(result);

		pDlg->m_iconPreview.SetIcon(pDlg->m_hIcon_On);
		Sleep(30);
		pDlg->m_iconPreview.SetIcon(pDlg->m_hIcon_Off);
	}
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSDKFunctionTesterDlg::OnEvent_TakingAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   )
{
	if( pContext == NULL )
		return;

	CIBSDKFunctionTesterDlg *pDlg = reinterpret_cast<CIBSDKFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	if( imageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		pDlg->m_iconTakingAcquisition.SetIcon(pDlg->m_hIcon_On);
		Sleep(1000);
		pDlg->m_iconTakingAcquisition.SetIcon(pDlg->m_hIcon_Off);

	}

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSDKFunctionTesterDlg::OnEvent_CompleteAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   )
{
	if( pContext == NULL )
		return;

	CIBSDKFunctionTesterDlg *pDlg = reinterpret_cast<CIBSDKFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	pDlg->m_iconCompleteAcquisition.SetIcon(pDlg->m_hIcon_On);
	Sleep(1000);
	pDlg->m_iconCompleteAcquisition.SetIcon(pDlg->m_hIcon_Off);

	LeaveCriticalSection(&g_CriticalSection);
}

/****
 ** This IBSU_CallbackResultImage callback was deprecated since 1.7.0
 ** Please consider using IBSU_CallbackResultImageEx instead
 */
void CIBSDKFunctionTesterDlg::OnEvent_ResultImageEx(
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

	CIBSDKFunctionTesterDlg *pDlg = reinterpret_cast<CIBSDKFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	pDlg->GetDlgItem( IDC_BUTTON_START )->EnableWindow( TRUE );

	CString result;
    if( imageStatus == IBSU_STATUS_OK )
	    result.Format("STATUS_OK");
    else if( imageStatus > IBSU_STATUS_OK )
        result.Format("WARNING : %d", imageStatus);
    else
        result.Format("ERROR : %d", imageStatus);

	pDlg->GetDlgItem(IDC_EDIT_RESULTIMAGE)->SetWindowText(result);

	pDlg->m_iconResultImageEx.SetIcon(pDlg->m_hIcon_On);
	Sleep(1000);
	pDlg->m_iconResultImageEx.SetIcon(pDlg->m_hIcon_Off);

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSDKFunctionTesterDlg::OnEvent_DeviceCommunicationBreak(
						 const int deviceHandle,
						 void*     pContext
					   )
{
	if( pContext == NULL )
		return;

	CIBSDKFunctionTesterDlg *pDlg = reinterpret_cast<CIBSDKFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	pDlg->m_iconDeviceCommunicationBreak.SetIcon(pDlg->m_hIcon_On);
	Sleep(100);
	pDlg->m_iconDeviceCommunicationBreak.SetIcon(pDlg->m_hIcon_Off);

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSDKFunctionTesterDlg::OnEvent_ClearPlatenAtCapture(
                          const int                   deviceHandle,
                          void                        *pContext,
                          const IBSU_PlatenState      platenState
                        )
{
	if( pContext == NULL )
		return;

	CIBSDKFunctionTesterDlg *pDlg = reinterpret_cast<CIBSDKFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	{
		if( platenState == ENUM_IBSU_PLATEN_HAS_FINGERS )
		{
			pDlg->m_iconClearPlatenAtCapture.SetIcon(pDlg->m_hIcon_On);

			pDlg->GetDlgItem(IDC_EDIT_CLEARPLATE)->SetWindowText("PLATEN_HAS_FINGERS");
		}
		else
		{
			pDlg->m_iconClearPlatenAtCapture.SetIcon(pDlg->m_hIcon_Off);

			pDlg->GetDlgItem(IDC_EDIT_CLEARPLATE)->SetWindowText("PLATEN_CLEARD");
		}
	}
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonStop()
{
	int nRc = IBSU_CancelCaptureImage( _GetCurrDeviceHandle() );

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_CancelCaptureImage -- Return value (%d)", nRc);
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonIscaptureavailable()
{
	UpdateData(FALSE);

	int		pos=0;
	const	IBSU_ImageType imgType = (IBSU_ImageType)m_cbImageType.GetCurSel();
	const	IBSU_ImageResolution imgRes = ENUM_IBSU_IMAGE_RESOLUTION_500;		// Currently we only support 500ppi.
	BOOL	modeAvailable;
	char	cValue[64];

	int nRc = IBSU_IsCaptureAvailable(_GetCurrDeviceHandle(), imgType, imgRes, &modeAvailable);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_IsCaptureAvailable -- Return value (%d) ", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		switch(imgType)
		{
		case ENUM_IBSU_TYPE_NONE:
			sprintf(cValue, "ENUM_IBSU_TYPE_NONE"); break;
		case ENUM_IBSU_ROLL_SINGLE_FINGER:
			sprintf(cValue, "ENUM_IBSU_ROLL_SINGLE_FINGER"); break;
		case ENUM_IBSU_FLAT_SINGLE_FINGER:
			sprintf(cValue, "ENUM_IBSU_FLAT_SINGLE_FINGER"); break;
		case ENUM_IBSU_FLAT_TWO_FINGERS:
			sprintf(cValue, "ENUM_IBSU_FLAT_TWO_FINGERS"); break;
		case ENUM_IBSU_FLAT_FOUR_FINGERS:
			sprintf(cValue, "ENUM_IBSU_FLAT_FOUR_FINGERS"); break;
		case ENUM_IBSU_FLAT_THREE_FINGERS:
			sprintf(cValue, "ENUM_IBSU_FLAT_THREE_FINGERS"); break;
		default:
			_SetAdditionalMessageOnListView(pos++, "Please select ImageType.");
			return;
		}

		if( !modeAvailable )
			_SetAdditionalMessageOnListView(pos++, "%s mode is not available", cValue);
		else
			_SetAdditionalMessageOnListView(pos++, "%s mode is available", cValue);
	}
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonIscaptureactive()
{
	int		pos=0;
	BOOL	IsActive;

	int nRc = IBSU_IsCaptureActive(_GetCurrDeviceHandle(), &IsActive);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_IsCaptureActive -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		if( IsActive )
			_SetAdditionalMessageOnListView(pos++, "Capturing...");
		else
			_SetAdditionalMessageOnListView(pos++, "Not Capturing...");
	}
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonGetcontrast()
{
	int		pos=0;
	int		contrastValue;


	int nRc = IBSU_GetContrast(_GetCurrDeviceHandle(), &contrastValue);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_GetContrast -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		_SetAdditionalMessageOnListView(pos++, "Contrast : %d", contrastValue);
	}
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonSetcontrast()
{
	UpdateData(FALSE);

	int		contrastValue;
	CString temp;

	GetDlgItemText(IDC_EDIT_SETCONTRAST, temp);
	contrastValue = _ttoi(temp);
	int nRc = IBSU_SetContrast(_GetCurrDeviceHandle(), contrastValue);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_SetContrast -- Return value (%d)", nRc);
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonGetleoperationmode()
{
	UpdateData(FALSE);

	int		pos=0;
	int		leOperationMode = m_cbSetLEOperationMode.GetCurSel();
	char	cValue[32];

	int nRc = IBSU_GetLEOperationMode(_GetCurrDeviceHandle(), (IBSU_LEOperationMode*)&leOperationMode);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_GetLEOperationMode -- Return value (%d)", nRc);

	if(nRc == IBSU_STATUS_OK)
	{
		if( leOperationMode == ENUM_IBSU_LE_OPERATION_AUTO )
			sprintf(cValue, "AUTO");
		else if( leOperationMode == ENUM_IBSU_LE_OPERATION_ON )
			sprintf(cValue, "ON");
		else if( leOperationMode == ENUM_IBSU_LE_OPERATION_OFF )
			sprintf(cValue, "OFF");
		else
			sprintf(cValue, "Unknown");

		_SetAdditionalMessageOnListView(pos++, "LE Mode : %s", cValue);
	}
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonSetleoperation()
{
	UpdateData(FALSE);

	int		pos=0;
	int		leOperationMode = m_cbSetLEOperationMode.GetCurSel();
	
	int nRc = IBSU_SetLEOperationMode(_GetCurrDeviceHandle(), (IBSU_LEOperationMode)leOperationMode);
	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_SetLEOperationMode -- Return value (%d)", nRc);
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonIstouchfinger()
{
	int		pos=0;
	int		touchInStatus;

	int nRc = IBSU_IsTouchedFinger(_GetCurrDeviceHandle(), &touchInStatus);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_IsTouchedFinger -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		if( touchInStatus )
			_SetAdditionalMessageOnListView(pos++, "Detected finger.");
		else
			_SetAdditionalMessageOnListView(pos++, "Not detected finger.");
	}
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonGetoperableleds()
{
	int		pos=0;
	int		ledCount;
	DWORD	operableLEDs;
	IBSU_LedType ledType;


	int nRc = IBSU_GetOperableLEDs(_GetCurrDeviceHandle(), &ledType, &ledCount, &operableLEDs);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_GetOperableLEDs -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		_SetAdditionalMessageOnListView(pos++, "LedType : %d, LedCount : %d, Operable LEDs : 0x%08X",
				ledType, ledCount, operableLEDs);
	}
}



void CIBSDKFunctionTesterDlg::OnBnClickedButtonGetleds()
{
	int		pos=0;
	DWORD	activeLEDs = -1;


	int nRc = IBSU_GetLEDs(_GetCurrDeviceHandle(), &activeLEDs);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_GetLEDs -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		_SetAdditionalMessageOnListView(pos++, "LED status : 0x%08X", activeLEDs);
	}
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonSetleds()
{
	UpdateData(FALSE);
	CString	temp;
	DWORD	activeLEDs;


	GetDlgItemText(IDC_EDIT_SETLED, temp);
	sscanf(temp, "%x", &activeLEDs);

	int nRc = IBSU_SetLEDs(_GetCurrDeviceHandle(), activeLEDs);
	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_SetLEDs -- Return value (%d)", nRc);
}

void CIBSDKFunctionTesterDlg::OnBnClickedCheckDisplayWindow()
{
	UpdateData(TRUE);
	int k = m_chkUseClientWindow;
	if( m_chkUseClientWindow )
	{
		GetDlgItem( IDC_BUTTON_BACKCL )->EnableWindow( TRUE );
		GetDlgItem( IDC_CHECK_GUIDELINE )->EnableWindow( TRUE );
		GetDlgItem( IDC_BUTTON_SET_DISPLAY_OVERLAY_TEXT )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_BKRANGE )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_POSX )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_POSX )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_POSY )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_POSY )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_TEXTCOLOR )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_TEXTCOLOR )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_RED )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_GREEN )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_BLACK )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_GREEN )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_BLACK )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_ADDTEXT )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_ADDTEXT )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_BKRED )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_BKGREEN )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_BKBLUE )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_BKRED )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_BKGREEN )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_BKBLUE )->EnableWindow( TRUE );
		
		CWnd *disWnd = GetDlgItem( IDC_FRAME_IMAGE );
		RECT clientRect;
		disWnd->GetClientRect( &clientRect );

		int nRc = IBSU_CreateClientWindow(_GetCurrDeviceHandle(), disWnd->m_hWnd, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
		_SetResultMessageOnListView(m_IsAppend, "-- IBSU_CreateClientWindow -- Return value (%d)", nRc);
	}
	else
	{
		GetDlgItem( IDC_BUTTON_BACKCL )->EnableWindow( FALSE );
		GetDlgItem( IDC_CHECK_GUIDELINE )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_SET_DISPLAY_OVERLAY_TEXT )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_BKRANGE )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_POSX )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_POSX )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_POSY )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_POSY )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_TEXTCOLOR )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_TEXTCOLOR )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_RED )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_GREEN )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_BLACK )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_GREEN )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_BLACK )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_ADDTEXT )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_ADDTEXT )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_BKRED )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_BKGREEN )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_BKBLUE )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_BKRED )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_BKGREEN )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_BKBLUE )->EnableWindow( FALSE );

		int nRc = IBSU_DestroyClientWindow(_GetCurrDeviceHandle(), TRUE);
		_SetResultMessageOnListView(m_IsAppend, "-- IBSU_DestroyClientWindow -- Return value (%d)", nRc);
	}
}

void CIBSDKFunctionTesterDlg::_SetEditValue(int value)
{
	CString temp, result;
	result.Format("%d", value); 
	temp = "%";
	result += temp;


	GetDlgItem(IDC_EDIT_INITPROGRESS)->SetWindowText(result);
}
void CIBSDKFunctionTesterDlg::OnBnClickedCheckGuideline()
{
	UpdateData(TRUE);
	int nRc;

	if( m_chkGuideLineValue )
	{
		nRc = IBSU_SetClientDisplayProperty( _GetCurrDeviceHandle(), ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE, "TRUE" );
	}
	else
	{
		nRc = IBSU_SetClientDisplayProperty( _GetCurrDeviceHandle(), ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE, "FALSE" );
	}

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_SetClientDisplayProperty -- Return value (%d)", nRc);
}

void CIBSDKFunctionTesterDlg::OnBnClickedCheckClearplate()
{
	UpdateData(TRUE);
}

void CIBSDKFunctionTesterDlg::OnBnClickedCheckFingercount()
{
	UpdateData(TRUE);
}

BOOL CIBSDKFunctionTesterDlg::PreTranslateMessage(MSG* pMsg)
{
	m_ToolTip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

void CIBSDKFunctionTesterDlg::OnBnClickedCheckAutoCapture()
{
	UpdateData(TRUE);
}

void CIBSDKFunctionTesterDlg::OnBnClickedCheck()
{
	UpdateData(TRUE);
}

void CIBSDKFunctionTesterDlg::ToolTip()
{

	//DeviceClose Button

	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_DEVICECLOSE), "It is close the Device.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//SdkVersion Button

	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_SDKVERSION), "You can see information about SDK");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//Device description Button

	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_DEVICEDESCRIPTION), "You can see information about device (You have to OpenDevie before run it)");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//Isdevice Open Button
	
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_ISDEVICEOPEN), "You can check if device is opend or not");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//GetProperty Button
	
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_GETPROPERTY), "You can see specific information about device (You have to OpenDevie before run it)");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//SetProperty  Button
	
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_SETPROPERTY), "You can modify specific information in device (You can only modify ""ENUM_IBSU_PROPERTY_IGNORE_FINGER_TIME"".)");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//IsCaptureAvailable  Button
	
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_ISCAPTUREAVAILABLE), "You can check what kind of fingerprint is able to be captured with this device");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//IsCaptureActive  Button
	
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_ISCAPTUREACTIVE), "Check if a particular device is actively scanning for image acquisition.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//IsTouchFinger  Button
	
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_ISTOUCHFINGER), "Queries a particular scanner to determine if a finger is currently on the detector.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//GetOperableLEDs  Button
	
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_GETOPERABLELEDS), "Get operable status LED's.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//SetLEOperation  Button
	
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_SETLEOPERATION), "Sets the touch operation mode (On, Off, or Auto) for a selected scanner.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//GetLEOperation Button
	
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_GETLEOPERATIONMODE), "Gets the touch operation mode (On, Off, or Auto) for a selected scanner.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//SETLEDS Button
	
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_SETLEDS), "Set active status LED's on device.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//GETLEDS Button
	
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_GETLEDS), "Get active status LED's on device.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//Start Button
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_START), "You can start capture image or take result image (You have to choose image type).");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//Stop Button
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_STOP), "You can stop capture image.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//SetContrast Button
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_SETCONTRAST), "You can set contrast value.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//GetContrast Button
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_GETCONTRAST), "You can get contrast value.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//////////////////////////////////////////////////////

	//Ignore NumberOfFinger ChkBox
	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK_FINGERCOUNT), "It can ignore number of finger (Ex. You cannot capture one finger image with 'two finger capture' mode without this option) ");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//IDC_CHECK__AUTO_CONTRAST ChkBox
	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK__AUTO_CONTRAST), "Optimal contrast value is fixed automatically.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//IDC_CHECK_AUTO_CAPTURE ChkBox
	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK_AUTO_CAPTURE), "Image is captured automatically.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

	//Capture manually button
	m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON_MANUALCAPTURE), "You can capture image manually.");
	m_ToolTip.SetTipBkColor(RGB(0,0,0));
	m_ToolTip.SetTipTextColor(RGB(255,255,255));

}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonManualcapture()
{
	int nRc = IBSU_TakeResultImageManually(_GetCurrDeviceHandle());

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_TakeResultImageManually -- Return value (%d)", nRc);
}

void CIBSDKFunctionTesterDlg::OnBnClickedCheckAppendresult()
{
	UpdateData(TRUE);

	if(m_chkAppendFunctionResult)
	{
		m_IsAppend = TRUE;
		
	}
	else
	{
		m_IsAppend = FALSE;
	}
	
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonBackcl()
{
	m_bkRed = GetDlgItemInt(IDC_EDIT_BKRED);
	m_bkGreen = GetDlgItemInt(IDC_EDIT_BKGREEN);
	m_bkBlue = GetDlgItemInt(IDC_EDIT_BKBLUE);

	COLORREF	cr = RGB(m_bkRed, m_bkGreen, m_bkBlue);

	CString temp;
	temp.Format("%d",cr);

	int nRc = IBSU_SetClientDisplayProperty(_GetCurrDeviceHandle(), ENUM_IBSU_WINDOW_PROPERTY_BK_COLOR, temp);

	_SetResultMessageOnListView(m_IsAppend, "-- IBSU_SetClientDisplayProperty -- Return value (%d)", nRc);
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonSetDisplayOverlayText()
{
	UpdateData(TRUE);

	m_posX = GetDlgItemInt(IDC_EDIT_POSX);
	m_posY = GetDlgItemInt(IDC_EDIT_POSY);
	m_bkTextRed = GetDlgItemInt(IDC_EDIT_TEXTCOLOR);
	m_bkTextGreen = GetDlgItemInt(IDC_EDIT_GREEN);
	m_bkTextBlue = GetDlgItemInt(IDC_EDIT_BLACK);

	int			font_size = 10;
	COLORREF	cr = RGB(m_bkTextRed, m_bkTextGreen, m_bkTextBlue);
	CString		temp;

	GetDlgItemText(IDC_EDIT_ADDTEXT, temp);

//	int nRc = IBSU_SetClientWindowOverlayText( _GetCurrDeviceHandle(), "Arial", font_size, FALSE, temp.GetBuffer(0), m_posX, m_posY, (DWORD)cr );
	int nRc = IBSU_AddOverlayText( _GetCurrDeviceHandle(), &m_nOverlayHandle, "Arial", font_size, FALSE, temp.GetBuffer(0), m_posX, m_posY, (DWORD)cr );

    _SetResultMessageOnListView(m_IsAppend, "-- IBSU_SetClientWindowOverlayText -- Return value (%d)", nRc);
}

void CIBSDKFunctionTesterDlg::OnClose()
{
	int nRc;

    m_StartAgingTest = FALSE;
    Sleep(1000);
	for( int i=0; i<10; i++ )
	{
		nRc = IBSU_CloseAllDevice();
		if( nRc != IBSU_ERR_RESOURCE_LOCKED )
			break;
		Sleep(100);
	}

	CDialog::OnClose();
}

BOOL CIBSDKFunctionTesterDlg::_WriteErrorLog(char *filepath, char *err, int attatch)
{
	FILE *fp;
	
	if(attatch)
		fp = fopen(filepath, "at");
	else
		fp = fopen(filepath, "wt");

	if(fp == NULL)
		return FALSE;

	fprintf(fp, "%s", err);
	fclose(fp);

	TRACE(err);

	return TRUE;
}

UINT CIBSDKFunctionTesterDlg::_ThreadAgingTest(LPVOID pParam)
{
	if( pParam == NULL )
		return 1;

	CIBSDKFunctionTesterDlg* pDlg = (CIBSDKFunctionTesterDlg*)pParam;

	int		devIndex = pDlg->GetDlgItemInt(IDC_EDIT_DEVICE);
	int		devHandle;
	int		nRc;
	CString err;
	int		pos=0;
	int		TotalTestCount=0;
	int		TotalErrorCount=0;
	TCHAR	path[512];
	CString filename, msg;
	int scenario_idx;

	BOOL IsActive;
	char str_value[256];
	
	IBSU_SdkVersion sdk_version;
	int dev_count;
	IBSU_DeviceDesc dev_desc;

	GetCurrentDirectory(512, path);
	filename.Format("%s\\AgingTest_OpenDevice_CloseDevice.txt", path);

	pDlg->_WriteErrorLog((char*)filename.GetBuffer(0), "Start Aging Test\n", 0);

	while(pDlg->m_StartAgingTest)
	{
		err = "";

		scenario_idx = rand() % 3;

		switch(scenario_idx)
		{
			case 0:
				nRc = IBSU_OpenDevice(devIndex, &devHandle);

				//IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, OnEvent_DeviceCommunicationBreak, pDlg );//Ok    
				//IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, OnEvent_PreviewImage, pDlg );//Ok
				//IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, OnEvent_TakingAcquisition, pDlg );//Ok  
				//IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, OnEvent_CompleteAcquisition, pDlg );//Ok  

				//IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX, OnEvent_ResultImageEx, pDlg );  //Ok
				//IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, OnEvent_FingerCount, pDlg );  //Ok
				//IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY, OnEvent_FingerQuality, pDlg ); //Ok 
				//IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE, OnEvent_ClearPlatenAtCapture, pDlg );

				if( nRc == IBSU_STATUS_OK )
				{
	                for( int i=0; i<10; i++ )
	                {
		                nRc = IBSU_CloseDevice(devHandle);
		                if( nRc != IBSU_ERR_RESOURCE_LOCKED )
			                break;
		                Sleep(100);
	                }

					if( nRc < IBSU_STATUS_OK )
					{
						err.Format("IBSU_CloseDevice() : %d\n", nRc);
						TotalErrorCount++;
					}
				}
				else
				{
					err.Format("IBSU_OpenDevice() : %d\n", nRc);
					TotalErrorCount++;
				}
				break;

			case 1:
				IBSU_GetSDKVersion(&sdk_version);
				IBSU_GetDeviceCount(&dev_count);
				IBSU_GetDeviceDescription(0, &dev_desc);
				IBSU_RegisterCallbacks(NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, pDlg->OnEvent_DeviceCount, pDlg);

				nRc = IBSU_OpenDevice(devIndex, &devHandle);

				if( nRc == IBSU_STATUS_OK )
				{
					IBSU_GetProperty( devHandle, ENUM_IBSU_PROPERTY_IMAGE_WIDTH, str_value);
					IBSU_GetProperty( devHandle, ENUM_IBSU_PROPERTY_IMAGE_HEIGHT, str_value);

	                for( int i=0; i<10; i++ )
	                {
		                nRc = IBSU_CloseDevice(devHandle);
		                if( nRc != IBSU_ERR_RESOURCE_LOCKED )
			                break;
		                Sleep(100);
	                }

					IBSU_ReleaseCallbacks(NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT);

					if( nRc < IBSU_STATUS_OK )
					{
						err.Format("IBSU_CloseDevice() : %d\n", nRc);
						TotalErrorCount++;
					}
				}
				else
				{
					err.Format("IBSU_OpenDevice() : %d\n", nRc);
					TotalErrorCount++;
				}
				break;

			case 2:
				IBSU_GetSDKVersion(&sdk_version);
				IBSU_GetDeviceCount(&dev_count);
				IBSU_GetDeviceDescription(0, &dev_desc);
				IBSU_RegisterCallbacks(NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, pDlg->OnEvent_DeviceCount, pDlg);

				nRc = IBSU_OpenDevice(devIndex, &devHandle);

				if( nRc == IBSU_STATUS_OK )
				{
					IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, pDlg->OnEvent_PreviewImage, pDlg );//Ok
					IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX, pDlg->OnEvent_ResultImageEx, pDlg );  //Ok
					IBSU_SetLEDs (devHandle, 0x00005500);
					IBSU_SetProperty( devHandle, ENUM_IBSU_PROPERTY_CAPTURE_TIMEOUT, "-1");
					IBSU_BeginCaptureImage( devHandle, ENUM_IBSU_ROLL_SINGLE_FINGER, ENUM_IBSU_IMAGE_RESOLUTION_500, 0x05);
					Sleep(600);
					IBSU_IsCaptureActive(devHandle, &IsActive);
					IBSU_CancelCaptureImage(devHandle);
					IBSU_ReleaseCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE );
					IBSU_ReleaseCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX );

	                for( int i=0; i<10; i++ )
	                {
		                nRc = IBSU_CloseDevice(devHandle);
		                if( nRc != IBSU_ERR_RESOURCE_LOCKED )
			                break;
		                Sleep(100);
	                }

					IBSU_ReleaseCallbacks(NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT);

					if( nRc < IBSU_STATUS_OK )
					{
						err.Format("IBSU_CloseDevice() : %d\n", nRc);
						TotalErrorCount++;
					}
				}
				else
				{
					err.Format("IBSU_OpenDevice() : %d\n", nRc);
					TotalErrorCount++;
				}

				break;		
		}
		

		TotalTestCount++;

		SYSTEMTIME time;
		GetLocalTime(&time);
		msg.Format("[%04d%02d%02d:%02d%02d%02d] : TestCount(%5d), TestError(%5d) : %s\n", 
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, TotalTestCount, TotalErrorCount, err);
		pDlg->_WriteErrorLog((char*)filename.GetBuffer(0), (char*)msg.GetBuffer(0), 1);
		pDlg->SetWindowText(msg);
	}

	return 0;
}

void CIBSDKFunctionTesterDlg::OnBnClickedButtonAgingTest()
{
	m_StartAgingTest = !m_StartAgingTest;

	if(m_StartAgingTest == TRUE)
	{
		VERIFY( ::AfxBeginThread( _ThreadAgingTest, this, THREAD_PRIORITY_NORMAL,
							  0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
		GetDlgItem(IDC_BUTTON_AGING_TEST)->SetWindowTextA("Stop Aging Test");
	}
	else
	{
		GetDlgItem(IDC_BUTTON_AGING_TEST)->SetWindowTextA("Start Aging Test");
	}
}
