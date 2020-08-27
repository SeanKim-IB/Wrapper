
// IBSU_NewFunctionTesterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IBSU_NewFunctionTester.h"
#include "IBSU_NewFunctionTesterDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CRITICAL_SECTION g_CriticalSection;
// CIBSU_NewFunctionTesterDlg dialog

// Key button definitions
const int __LEFT_KEY_BUTTON__	= 1;
const int __RIGHT_KEY_BUTTON__	= 2;



CIBSU_NewFunctionTesterDlg::CIBSU_NewFunctionTesterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIBSU_NewFunctionTesterDlg::IDD, pParent)
	, m_chkAppendFunctionResult(FALSE)
	, m_chkUseImageViewer(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pwndShow = NULL;
	m_nRowOnListView = 0;
	m_IsAppend = FALSE;

	InitializeCriticalSection(&g_CriticalSection);
}

void CIBSU_NewFunctionTesterDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_MAIN, m_listMain);
    //DDX_Control(pDX, IDC_LIST2, m_listEtc);
    DDX_Control(pDX, IDC_TEST_TAB, m_tab);
    DDX_Control(pDX, IDC_LIST_ETC, m_listEtc);
    DDX_Check(pDX, IDC_CHECK_APPENDRESULT, m_chkAppendFunctionResult);
    DDX_Check(pDX, IDC_CHECK_IMAGE_VIEWER, m_chkUseImageViewer);
    DDX_Control(pDX, IDC_ICON_INITPROGRESS_OFF, m_iconInitProgress);
    DDX_Control(pDX, IDC_ICON_PREVIEWIMAGE_OFF, m_iconPreviewImage);
    DDX_Control(pDX, IDC_ICON_TAKINGACQUISITION_OFF, m_iconTakingAcquisition);
    DDX_Control(pDX, IDC_ICON_COMPLETEACQUISITION_OFF, m_iconCompleteAcquisition);
    DDX_Control(pDX, IDC_ICON_RESULTIMAGE_OFF, m_iconResultImageEx);
    DDX_Control(pDX, IDC_ICON_FINGERCOUNT_OFF, m_iconFingerCount);
    DDX_Control(pDX, IDC_ICON_FINGERQUALITY_OFF, m_iconFingerQuality);
    DDX_Control(pDX, IDC_ICON_DEVICECOMMUNICATIONBREAK_OFF, m_iconDeviceCommunicationBreak);
    DDX_Control(pDX, IDC_ICON_DEVICECOUNT_OFF, m_iconDeviceCount);
    DDX_Control(pDX, IDC_ICON_CLEARPLATENATCAPTURE_OFF, m_iconClearPlatenAtCapture);
    DDX_Control(pDX, IDC_ICON_ASYNCOPENDEVICE_OFF, m_iconAsyncOpenDevice);
    DDX_Control(pDX, IDC_ICON_NOTIFYMESSAGE_OFF, m_iconNotifyMessage);
    DDX_Control(pDX, IDC_ICON_KEYBUTTON_OFF, m_iconKeyButton);
}

BEGIN_MESSAGE_MAP(CIBSU_NewFunctionTesterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_NOTIFY(TCN_SELCHANGE, IDC_TEST_TAB, &CIBSU_NewFunctionTesterDlg::OnTcnSelchangeTestTab)
	ON_BN_CLICKED(IDC_CHECK_APPENDRESULT, &CIBSU_NewFunctionTesterDlg::OnBnClickedCheckAppendresult)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_IMAGE_VIEWER, &CIBSU_NewFunctionTesterDlg::OnBnClickedCheckImageViewer)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CIBSU_NewFunctionTesterDlg message handlers

BOOL CIBSU_NewFunctionTesterDlg::OnInitDialog()
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

//	SetWindowPos(NULL, 0, 0, 1020, 730, SWP_NOMOVE);

	CRect Rect;
	m_tab.GetClientRect(&Rect);

	m_mainFunction.Create(IDD_DIALOG_MFUNCTION, &m_tab);
	m_mainFunction.SetWindowPos(NULL, 5, 25, Rect.Width() - 10, Rect.Height() - 30, SWP_SHOWWINDOW | SWP_NOZORDER);
	m_pwndShow = &m_mainFunction;

	m_captureFunctions.Create(IDD_DIALOG_CAPTUREFUNCTIONS, &m_tab);
	m_captureFunctions.SetWindowPos(NULL, 5, 25, Rect.Width() - 10, Rect.Height() - 30, SWP_NOZORDER);

	m_controlFunctions.Create(IDD_DIALOG_CONTROL, &m_tab);
	m_controlFunctions.SetWindowPos(NULL, 5, 25, Rect.Width() - 10, Rect.Height() - 30, SWP_NOZORDER);

	m_clientWindowFunctions.Create(IDD_DIALOG_CLIENTWINDOW, &m_tab);
	m_clientWindowFunctions.SetWindowPos(NULL, 5, 25, Rect.Width() - 10, Rect.Height() - 30, SWP_NOZORDER);

	m_nonCallbackFunctions.Create(IDD_DIALOG_NONCALLBACK, &m_tab);
	m_nonCallbackFunctions.SetWindowPos(NULL, 5, 25, Rect.Width() - 10, Rect.Height() - 30, SWP_NOZORDER);

	int k=0;

	m_nRowOnListView = 0;

	Init_Layout();

	m_imageDlg = new CImageDlg(this);
	m_imageDlg->Create(IDD_DIALOG_IMAGE);
	m_imageDlg->ShowWindow(SW_HIDE);

	////////////////////////////////////////////////////////////////////////////////////////
	// If you want to use Non-Callback method without callback functions,
	// then you does not use following functions.
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, OnEvent_DeviceCount, this );
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, OnEvent_InitProgress, this );
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE, OnEvent_AsyncOpenDevice, this );
	////////////////////////////////////////////////////////////////////////////////////////

	m_bUseNonCallback = FALSE;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIBSU_NewFunctionTesterDlg::OnPaint()
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
HCURSOR CIBSU_NewFunctionTesterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CIBSU_NewFunctionTesterDlg::OnEvent_DeviceCommunicationBreak(
							const int deviceHandle,
							void* pContext
							)
{
	if( pContext == NULL )
		return;

	CIBSU_NewFunctionTesterDlg *pDlg = reinterpret_cast<CIBSU_NewFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	pDlg->m_iconDeviceCommunicationBreak.SetIcon(pDlg->m_hIcon_On);
	Sleep(100);
	pDlg->m_iconDeviceCommunicationBreak.SetIcon(pDlg->m_hIcon_Off);

	LeaveCriticalSection(&g_CriticalSection);
}


///////////////////////////////////////////////////////////////////////////////////////////
void CIBSU_NewFunctionTesterDlg::OnEvent_FingerQuality(
                          const int                     deviceHandle,   
                          void                          *pContext,       
                          const IBSU_FingerQualityState *pQualityArray, 
                          const int                     qualityArrayCount    
						 )
{
	if( pContext == NULL )
		return;

	CIBSU_NewFunctionTesterDlg *pDlg = reinterpret_cast<CIBSU_NewFunctionTesterDlg*>(pContext);
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
			case ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM :
				result += "(Inv-Bottom)";
				break;          
			case ENUM_IBSU_FINGER_NOT_PRESENT:
				result += "(Non)";
				break;
			default:
				return;
		}
	}

	pDlg->GetDlgItem(IDC_EDIT_FINGERQUALITY)->SetWindowText(result);

	pDlg->m_iconFingerQuality.SetIcon(pDlg->m_hIcon_Off);

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_NewFunctionTesterDlg::OnEvent_DeviceCount(
						 const int detectedDevices,
						 void      *pContext
					   )
{
	if( pContext == NULL )
		return;

	CIBSU_NewFunctionTesterDlg *pDlg = reinterpret_cast<CIBSU_NewFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	CString result;
	result.Format("%d", detectedDevices);
	pDlg->GetDlgItem(IDC_EDIT_DEVICECOUNT)->SetWindowText(result);

	pDlg->m_iconDeviceCount.SetIcon(pDlg->m_hIcon_On);
	Sleep(300);
	pDlg->m_iconDeviceCount.SetIcon(pDlg->m_hIcon_Off);

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_NewFunctionTesterDlg::OnEvent_FingerCount(
						   const int                   deviceHandle,
						   void*                       pContext,
						   const IBSU_FingerCountState fingerCountState
						 )
{
	if( pContext == NULL )
		return;

	CIBSU_NewFunctionTesterDlg *pDlg = reinterpret_cast<CIBSU_NewFunctionTesterDlg*>(pContext);
	/*if( deviceHandle != pDlg->_GetCurrDeviceHandle() )
		ASSERT( FALSE );*/

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

void CIBSU_NewFunctionTesterDlg::OnEvent_InitProgress(
						 const int deviceIndex,
						 void      *pContext,
						 const int progressValue
					   )
{
	if( pContext == NULL )
		return;

	CIBSU_NewFunctionTesterDlg *pDlg = reinterpret_cast<CIBSU_NewFunctionTesterDlg*>(pContext);
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

void CIBSU_NewFunctionTesterDlg::OnEvent_PreviewImage(
						 const int deviceHandle,
						 void*     pContext,
						 const	   IBSU_ImageData image
					   )
{
	if( pContext == NULL )
		return;

	CIBSU_NewFunctionTesterDlg *pDlg = reinterpret_cast<CIBSU_NewFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	{
		CString result;
		result.Format("%0.3f ms", image.FrameTime*1000);
		pDlg->GetDlgItem(IDC_EDIT_PREVIEWIMAGE)->SetWindowText(result);

		pDlg->m_iconPreviewImage.SetIcon(pDlg->m_hIcon_On);
		Sleep(30);
		pDlg->m_iconPreviewImage.SetIcon(pDlg->m_hIcon_Off);
	}
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_NewFunctionTesterDlg::OnEvent_TakingAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   )
{
	if( pContext == NULL )
		return;

	CIBSU_NewFunctionTesterDlg *pDlg = reinterpret_cast<CIBSU_NewFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	if( imageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		pDlg->m_iconTakingAcquisition.SetIcon(pDlg->m_hIcon_On);
		Sleep(1000);
		pDlg->m_iconTakingAcquisition.SetIcon(pDlg->m_hIcon_Off);
	}

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_NewFunctionTesterDlg::OnEvent_CompleteAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   )
{
	if( pContext == NULL )
		return;

	CIBSU_NewFunctionTesterDlg *pDlg = reinterpret_cast<CIBSU_NewFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	pDlg->m_iconCompleteAcquisition.SetIcon(pDlg->m_hIcon_On);
	Sleep(1000);
	pDlg->m_iconCompleteAcquisition.SetIcon(pDlg->m_hIcon_Off);

	LeaveCriticalSection(&g_CriticalSection);
}

/****
 ** This IBSU_CallbackResultImage callback was deprecated since 1.7.0
 ** Please consider using IBSU_CallbackResultImageEx instead
****/
void CIBSU_NewFunctionTesterDlg::OnEvent_ResultImageEx(
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

	CIBSU_NewFunctionTesterDlg *pDlg = reinterpret_cast<CIBSU_NewFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	//pDlg->GetDlgItem( IDC_BUTTON_START )->EnableWindow( TRUE );

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

void CIBSU_NewFunctionTesterDlg::OnEvent_ClearPlatenAtCapture(
                          const int                   deviceHandle,
                          void                        *pContext,
                          const IBSU_PlatenState      platenState
                        )
{
	if( pContext == NULL )
		return;

	CIBSU_NewFunctionTesterDlg *pDlg = reinterpret_cast<CIBSU_NewFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	{
		if( platenState == ENUM_IBSU_PLATEN_HAS_FINGERS )
		{
			pDlg->m_iconClearPlatenAtCapture.SetIcon(pDlg->m_hIcon_On);

			pDlg->GetDlgItem(IDC_EDIT_CLEARPLATENATCAPTURE)->SetWindowText("PLATEN_HAS_FINGERS");
		}
		else
		{
			pDlg->m_iconClearPlatenAtCapture.SetIcon(pDlg->m_hIcon_On);

			pDlg->GetDlgItem(IDC_EDIT_CLEARPLATENATCAPTURE)->SetWindowText("PLATEN_CLEARD");

			Sleep(300);
			pDlg->m_iconClearPlatenAtCapture.SetIcon(pDlg->m_hIcon_Off);
		}
	}
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_NewFunctionTesterDlg::OnEvent_AsyncOpenDevice(
                          const int                   deviceIndex,
                          void                        *pContext,
                          const int                   deviceHandle,
                          const int                   errorCode
						  )
{
	if( pContext == NULL )
		return;

	CIBSU_NewFunctionTesterDlg *pDlg = reinterpret_cast<CIBSU_NewFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	CString result;
	if( deviceHandle >= 0 )
	{
		result.Format("%d", deviceHandle);
		pDlg->GetDlgItem(IDC_EDIT_ASYNCOPENDEVICE)->SetWindowText(result);
	}
	else
	{
		result.Format("%d", errorCode);
		pDlg->GetDlgItem(IDC_EDIT_ASYNCOPENDEVICE)->SetWindowText(result);
	}

	pDlg->m_iconAsyncOpenDevice.SetIcon(pDlg->m_hIcon_On);
	Sleep(300);
	pDlg->m_iconAsyncOpenDevice.SetIcon(pDlg->m_hIcon_Off);

	if( deviceHandle >= 0 && !pDlg->m_bUseNonCallback )
	{
		// register callback functions
		IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, pDlg->OnEvent_DeviceCommunicationBreak, pDlg );    
		IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, pDlg->OnEvent_PreviewImage, pDlg );
		IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, pDlg->OnEvent_TakingAcquisition, pDlg );
		IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, pDlg->OnEvent_CompleteAcquisition, pDlg );

		IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX, pDlg->OnEvent_ResultImageEx, pDlg );
        IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, pDlg->OnEvent_FingerCount, pDlg );
		IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY, pDlg->OnEvent_FingerQuality, pDlg );
		IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE, pDlg->OnEvent_ClearPlatenAtCapture, pDlg );
		IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_OPTIONAL_EVENT_NOTIFY_MESSAGE, pDlg->OnEvent_NotifyMessage, pDlg );
		IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_ESSENTIAL_EVENT_KEYBUTTON, pDlg->OnEvent_PressedKeyButtons, pDlg );
	}

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_NewFunctionTesterDlg::OnEvent_NotifyMessage(
                          const int                deviceHandle,
                          void                     *pContext,
                          const int                message
						  )
{
	if( pContext == NULL )
		return;

	CIBSU_NewFunctionTesterDlg *pDlg = reinterpret_cast<CIBSU_NewFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	CString result;
	result.Format("%d", message);
	pDlg->GetDlgItem(IDC_EDIT_NOTIFYMESSAGE)->SetWindowText(result);

	pDlg->m_iconNotifyMessage.SetIcon(pDlg->m_hIcon_On);
	Sleep(300);
	pDlg->m_iconNotifyMessage.SetIcon(pDlg->m_hIcon_Off);

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_NewFunctionTesterDlg::OnEvent_PressedKeyButtons(
                          const int                deviceHandle,
                          void                     *pContext,
                          const int                pressedKeyButtons
						  )
{
	if( pContext == NULL )
		return;

	CIBSU_NewFunctionTesterDlg *pDlg = reinterpret_cast<CIBSU_NewFunctionTesterDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	CString result;
    if( pressedKeyButtons == __LEFT_KEY_BUTTON__ )
    {
        result.Format("%s", "Pressed Left key");
	    pDlg->GetDlgItem(IDC_EDIT_KEYBUTTON)->SetWindowText(result);
    }
    else if( pressedKeyButtons == __RIGHT_KEY_BUTTON__ )
    {
        result.Format("%s", "Pressed Right key");
	    pDlg->GetDlgItem(IDC_EDIT_KEYBUTTON)->SetWindowText(result);
    }

	pDlg->m_iconKeyButton.SetIcon(pDlg->m_hIcon_On);
	Sleep(300);
	pDlg->m_iconKeyButton.SetIcon(pDlg->m_hIcon_Off);

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBSU_NewFunctionTesterDlg::_SetEditValue(int value)
{
	CString temp, result;
	result.Format("%d", value); 
	temp = "%";
	result += temp;


	GetDlgItem(IDC_EDIT_INITPROGRESS)->SetWindowText(result);
}

void CIBSU_NewFunctionTesterDlg::_SetCoordinateControl(WINDOWPLACEMENT *place, int nID, int left, int top, int width, int height)
{
	CWnd *pWnd = GetDlgItem(nID);

	place->rcNormalPosition.left = left;
	place->rcNormalPosition.top = top;
	place->rcNormalPosition.right = left + width;
	place->rcNormalPosition.bottom = top + height;

	pWnd->SetWindowPlacement(place);
}

void CIBSU_NewFunctionTesterDlg::Init_Layout()
{
	CString strTabList[NUMBER_OF_TAB] = {_T("      Main      "), _T("    Capture     "), _T("    Control     "), _T("  ClientWindow  "), _T("Non-Callback Method")};
	WINDOWPLACEMENT place;

	m_hIcon_On = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON_ON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	m_hIcon_Off = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON_OFF), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	SetWindowPos(NULL, 0, 0, 1030, 730, SWP_NOMOVE);
	GetWindowPlacement(&place);

	//int left = 10, top = 90;
	int left = 685, top  = 90;
	const int basicWidth = 140, basicHeight = 20;
	const int wideWidth = 205, smallWidth = 75;
	const int groupWidth = 325;
	const int iconWidth = 16, iconHeight = 16;
	RECT	*rc = &place.rcNormalPosition;

	for(int i=0;i<NUMBER_OF_TAB;i++)
	{
		m_tab.InsertItem(i, strTabList[i]);
	}
	
	//Callback GroupBox
	//_SetCoordinateControl(&place, IDC_STATIC_CALLBACK, left, 80, groupWidth, 370);
	_SetCoordinateControl(&place, IDC_STATIC_CALLBACK, left, rc->top + 80 , groupWidth, 315);

	//1st icon 
	_SetCoordinateControl(&place, IDC_ICON_INITPROGRESS_OFF, left+10, rc->top + 20, iconWidth, iconHeight);
	//1st static
	_SetCoordinateControl(&place, IDC_STATIC_INITPROGRESS, rc->right+5, rc->top, basicWidth, basicHeight);
	//1st edit box
	_SetCoordinateControl(&place, IDC_EDIT_INITPROGRESS, rc->right, rc->top, basicWidth, basicHeight);

	//2nd icon
	_SetCoordinateControl(&place, IDC_ICON_PREVIEWIMAGE_OFF, left+10, rc->bottom + 2, iconWidth, iconHeight);
	//2nd static
	_SetCoordinateControl(&place, IDC_STATIC_PREVIEWIMAGE, rc->right+5, rc->top, basicWidth, basicHeight);
	//2nd edit box
	_SetCoordinateControl(&place, IDC_EDIT_PREVIEWIMAGE, rc->right, rc->top, basicWidth, basicHeight);

	//3rd icon
	_SetCoordinateControl(&place, IDC_ICON_TAKINGACQUISITION_OFF, left+10, rc->bottom + 2, iconWidth, iconHeight);
	//3rd static
	_SetCoordinateControl(&place, IDC_STATIC_TAKINGACQUISITION, rc->right+5, rc->top, basicWidth, basicHeight);

	//4th icon
	_SetCoordinateControl(&place, IDC_ICON_COMPLETEACQUISITION_OFF, left+10, rc->bottom + 2, iconWidth, iconHeight);
	//4th static
	_SetCoordinateControl(&place, IDC_STATIC_COMPLETEACQUISITION, rc->right+5, rc->top, basicWidth, basicHeight);
	
	//5th icon
	_SetCoordinateControl(&place, IDC_ICON_RESULTIMAGE_OFF, left+10, rc->bottom + 2, iconWidth, iconHeight);
	//5th static
	_SetCoordinateControl(&place, IDC_STATIC_RESULTIMAGE, rc->right+5, rc->top, basicWidth, basicHeight);
	//5th edit Box
	_SetCoordinateControl(&place, IDC_EDIT_RESULTIMAGE, rc->right, rc->top, basicWidth, basicHeight);

	//6th Icon
	_SetCoordinateControl(&place, IDC_ICON_FINGERCOUNT_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//6th Static
	_SetCoordinateControl(&place, IDC_STATIC_FINGERCOUNT, rc->right+5, rc->top, basicWidth, basicHeight);
	//6th edit Box
	_SetCoordinateControl(&place, IDC_EDIT_FINGERCOUNT, rc->right, rc->top, basicWidth, basicHeight);

	//7th On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_FINGERQUALITY_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//7th Static
	_SetCoordinateControl(&place, IDC_STATIC_FINGERQUALITY, rc->right+5, rc->top, basicWidth, basicHeight);
	//7th edit Box
	_SetCoordinateControl(&place, IDC_EDIT_FINGERQUALITY, rc->right, rc->top, basicWidth, basicHeight);

	//8th Icon
	_SetCoordinateControl(&place, IDC_ICON_DEVICECOMMUNICATIONBREAK_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//8th Static
	_SetCoordinateControl(&place, IDC_STATIC_DEVICECOMMUNICATIONBREAK, rc->right+5, rc->top, basicWidth, basicHeight);

	//9th Icon
	_SetCoordinateControl(&place, IDC_ICON_DEVICECOUNT_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//9th Static
	_SetCoordinateControl(&place, IDC_STATIC_DEVICECOUNT, rc->right+5, rc->top, basicWidth, basicHeight);
	//9th edit Box
	_SetCoordinateControl(&place, IDC_EDIT_DEVICECOUNT, rc->right, rc->top, basicWidth, basicHeight);

	//10th On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_CLEARPLATENATCAPTURE_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//10th Static
	_SetCoordinateControl(&place, IDC_STATIC_CLEARPLATENATCAPTURE, rc->right+5, rc->top, basicWidth, basicHeight);
	//10th edit Box
	_SetCoordinateControl(&place, IDC_EDIT_CLEARPLATENATCAPTURE, rc->right, rc->top, basicWidth, basicHeight);

	//11th On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_ASYNCOPENDEVICE_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//11th Static
	_SetCoordinateControl(&place, IDC_STATIC_ASYNCOPENDEVICE, rc->right+5, rc->top, basicWidth, basicHeight);
	//11th edit Box
	_SetCoordinateControl(&place, IDC_EDIT_ASYNCOPENDEVICE, rc->right, rc->top, basicWidth, basicHeight);

	//12th On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_NOTIFYMESSAGE_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//12th Static
	_SetCoordinateControl(&place, IDC_STATIC_NOTIFYMESSAGE, rc->right+5, rc->top, basicWidth, basicHeight);
	//12th edit Box
	_SetCoordinateControl(&place, IDC_EDIT_NOTIFYMESSAGE, rc->right, rc->top, basicWidth, basicHeight);

	//13th On/Off Icon
	_SetCoordinateControl(&place, IDC_ICON_KEYBUTTON_OFF, left+10, rc->bottom+2, iconWidth, iconHeight);
	//12th Static
	_SetCoordinateControl(&place, IDC_STATIC_KEYBUTTON, rc->right+5, rc->top, basicWidth, basicHeight);
	//12th edit Box
	_SetCoordinateControl(&place, IDC_EDIT_KEYBUTTON, rc->right, rc->top, basicWidth, basicHeight);

	//AppendResult ChkBox
	_SetCoordinateControl(&place, IDC_CHECK_APPENDRESULT, left+5, rc->bottom+30, groupWidth, basicHeight);

	//MainList Control
	m_listMain.InsertColumn(0, "Function Result", LVCFMT_LEFT, groupWidth-5);
	_SetCoordinateControl(&place, IDC_LIST_MAIN, left, rc->bottom+5, groupWidth, 80);

	//m_listEtc Control
	m_listEtc.InsertColumn(0, "Additional information", LVCFMT_LEFT, groupWidth-5);
	_SetCoordinateControl(&place, IDC_LIST_ETC, left, rc->bottom+10, groupWidth, 130);
	

}

void CIBSU_NewFunctionTesterDlg::OnTcnSelchangeTestTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	if(m_pwndShow != NULL)
	{
		m_pwndShow->ShowWindow(SW_HIDE);
		m_pwndShow = NULL;
	}

	int nIndex = m_tab.GetCurSel();

	switch(nIndex)
	{
	case 0:
		m_mainFunction.ShowWindow(SW_SHOW);
		m_pwndShow = &m_mainFunction;
		break;

	case 1:
		m_captureFunctions.ShowWindow(SW_SHOW);
		m_pwndShow = &m_captureFunctions;
		break;

	case 2:
		m_controlFunctions.ShowWindow(SW_SHOW);
		m_pwndShow = &m_controlFunctions;
		break;

	case 3:
		m_clientWindowFunctions.ShowWindow(SW_SHOW);
		m_pwndShow = &m_clientWindowFunctions;
		break;

	case 4:
		m_nonCallbackFunctions.ShowWindow(SW_SHOW);
		m_pwndShow = &m_nonCallbackFunctions;
		break;

	}

	*pResult = 0;
}

void CIBSU_NewFunctionTesterDlg::_SetResultMessageOnListView(BOOL bAppend, LPCTSTR Format, ...)
{
	//m_list
	m_listEtc.DeleteAllItems();

	if(!bAppend)
	{
		m_listMain.DeleteAllItems();
		m_nRowOnListView = 0;
	}

	TCHAR cMessage[IBSU_MAX_STR_LEN+1];
	va_list arg_ptr;

	va_start( arg_ptr, Format);
	_vsntprintf( cMessage, IBSU_MAX_STR_LEN, Format, arg_ptr);
	va_end( arg_ptr);

	CString strMessage;
	strMessage.Format(_T("%s"), cMessage);
	m_listMain.InsertItem(m_nRowOnListView, strMessage);
	m_nRowOnListView++;
}

void CIBSU_NewFunctionTesterDlg::_SetAdditionalMessageOnListView(int row, LPCTSTR Format, ...)
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

void CIBSU_NewFunctionTesterDlg::OnBnClickedCheckAppendresult()
{
	// TODO: Add your control notification handler code here
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

int CIBSU_NewFunctionTesterDlg::_GetCurrDeviceHandle()
{
	return 0;
}

void CIBSU_NewFunctionTesterDlg::OnBnClickedCheckImageViewer()
{
	UpdateData();

	if( m_chkUseImageViewer )
		m_imageDlg->ShowWindow(SW_SHOW);
	else
		m_imageDlg->ShowWindow(SW_HIDE);
}

BOOL CIBSU_NewFunctionTesterDlg::PreTranslateMessage(MSG* pMsg)
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
	}


	return CDialog::PreTranslateMessage(pMsg);
}

void CIBSU_NewFunctionTesterDlg::OnClose()
{
	if( m_imageDlg )
		delete m_imageDlg;

	int nRc;
	for( int i=0; i<10; i++ )
	{
		nRc = IBSU_CloseAllDevice();
		if( nRc != IBSU_ERR_RESOURCE_LOCKED )
			break;
		Sleep(100);
	}

	CDialog::OnClose();
}
