
// IBScanUltimate_SalesDemoDlg.cpp : implementation file
//

#include "stdafx.h"
//#ifdef __FORIBK__
//#endif
#include "IBScanUltimate_SalesDemo.h"
#include "IBScanUltimate_SalesDemoDlg.h"

#include "IBScanUltimateApi.h"
//#include "ReservedApi.h"
#include "FingerDisplayManager.h"
#include "OptionSelect.h"

#include "IBScanNFIQ2Api.h"
#include "nex_sdk.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// LED color definitions
const int __LED_COLOR_NONE__	= 0;
const int __LED_COLOR_GREEN__	= 1;
const int __LED_COLOR_RED__		= 2;
const int __LED_COLOR_YELLOW__	= 3;

// Key button definitions
const int __LEFT_KEY_BUTTON__	= 1;
const int __RIGHT_KEY_BUTTON__	= 2;

// Use critical section
CRITICAL_SECTION g_CriticalSection;

// CIBScanUltimate_SalesDemoDlg dialog
CIBScanUltimate_SalesDemoDlg::CIBScanUltimate_SalesDemoDlg(CWnd* pParent /*=NULL*/)
: CDialog(CIBScanUltimate_SalesDemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	InitializeCriticalSection(&g_CriticalSection);

	m_pWnd = NULL;
	m_CurrentTabIdx = -1;
	m_nDevHandle = -1;
	m_nCurrentCaptureStep = -1;
	m_bInitializing = FALSE;
	m_bNeedClearPlaten = FALSE;
	m_CaptureEnd = FALSE;
	m_Main_Security_Level = 1;
	m_SmearFlag = FALSE;
	m_WrongPlaceFlag = FALSE;
	m_WetFingerFlag = FALSE;
	m_SpoofArr = new unsigned char [SPOOF_BUF_W*SPOOF_BUF_H];
	m_SpoofArrFlip = new unsigned char [SPOOF_BUF_W*SPOOF_BUF_H];

	nex_sdk_initialize();
	nex_sdk_load_model(NEX_SDK_CLASSIFIER_MLP);
}

CIBScanUltimate_SalesDemoDlg::~CIBScanUltimate_SalesDemoDlg()
{
	nex_sdk_destroy_model();
	nex_sdk_destroy();

	delete [] m_SpoofArr;
	delete [] m_SpoofArrFlip;

	DeleteCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SalesDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_VIEW, m_TabCtrl);
}

BEGIN_MESSAGE_MAP(CIBScanUltimate_SalesDemoDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
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

	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_VIEW, &CIBScanUltimate_SalesDemoDlg::OnTcnSelchangeTabView)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_COMMAND(IDD_Option, &CIBScanUltimate_SalesDemoDlg::OnOption)
	ON_COMMAND(ID_EXIT, &CIBScanUltimate_SalesDemoDlg::OnExit)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CIBScanUltimate_SalesDemoDlg message handlers

BOOL CIBScanUltimate_SalesDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	//m_background.CreateSolidBrush(RGB(67,116,217));

	// TODO: Add extra initialization here
	Init_Layout();

	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, OnEvent_DeviceCount, this );
//	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, OnEvent_InitProgress, this );
	PostMessage( WM_USER_UPDATE_DEVICE_LIST );

	TCHAR MyName[MAX_PATH];
	GetModuleFileName(NULL, MyName, MAX_PATH);
	CString FileVersion=_T("");
    DWORD infoSize = 0;
	char *buffer;

	IBSU_GetSDKVersion(&m_verInfo);

	infoSize = GetFileVersionInfoSize(MyName, 0);
	if(infoSize>0)
	{
		buffer = new char[infoSize];
		if(buffer)
		{
			if(GetFileVersionInfo(MyName,0,infoSize, buffer)!=0)
			{
				VS_FIXEDFILEINFO* pFineInfo = NULL;
				UINT bufLen = 0;

				if(VerQueryValue(buffer,"\\",(LPVOID*)&pFineInfo, &bufLen) !=0)
				{    
					WORD majorVer, minorVer, buildNum, revisionNum;
					majorVer = HIWORD(pFineInfo->dwFileVersionMS);
					minorVer = LOWORD(pFineInfo->dwFileVersionMS);
					buildNum = HIWORD(pFineInfo->dwFileVersionLS);
					revisionNum = LOWORD(pFineInfo->dwFileVersionLS);

					FileVersion.Format("CBP Demo Application ( Version %d.%d.%d.%d ) with DLL ver. %s", majorVer, minorVer, buildNum, revisionNum, m_verInfo.Product);
				}
			}
			delete[] buffer;
		}
	}
	
	if(FileVersion.GetLength()==0)
		FileVersion.Format("CBP Demo Application ( can't read version ) with DLL ver. %s", m_verInfo.Product);

	SetWindowText(FileVersion);
	memset(&m_FingerQuality[0], ENUM_IBSU_FINGER_NOT_PRESENT, sizeof(m_FingerQuality));

	IBSU_NFIQ2_Initialize();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIBScanUltimate_SalesDemoDlg::OnPaint()
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
HCURSOR CIBScanUltimate_SalesDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CIBScanUltimate_SalesDemoDlg::Init_Layout()
{
	int i;
	CString strlist_Eng[NUMBER_OF_TAB] = {"Main Tab", "Scan Tab"};

	WINDOWPLACEMENT place;
	GetWindowPlacement(&place);

	int WindowWidth = 1100;//GetSystemMetrics(SM_CXSCREEN)
	int WindowHeight = 740; //GetSystemMetrics(SM_CYSCREEN)

	SetWindowPos(NULL, 0, 0, WindowWidth, WindowHeight, SWP_NOMOVE);
	CenterWindow();

	place.rcNormalPosition.left=0;
	place.rcNormalPosition.top=5;
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
	m_ScanFingerDlg.m_FingerDisplayManager->m_pParent = this;

	m_MainDlg.ShowWindow(SW_SHOW);
	m_pWnd = &m_MainDlg;
}

void CIBScanUltimate_SalesDemoDlg::OnTcnSelchangeTabView(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_CurrentTabIdx = m_TabCtrl.GetCurSel();

	ChangeView(m_CurrentTabIdx);

	*pResult = 0;
}

void CIBScanUltimate_SalesDemoDlg::ChangeView(int view)
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

int CIBScanUltimate_SalesDemoDlg::_ReleaseDevice()
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

void CIBScanUltimate_SalesDemoDlg::_BeepFail()
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

void CIBScanUltimate_SalesDemoDlg::_BeepSuccess()
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

void CIBScanUltimate_SalesDemoDlg::_BeepOk()
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

void CIBScanUltimate_SalesDemoDlg::_BeepDeviceCommunicationBreak()
{
	for( int i=0; i<8; i++ )
	{
		Beep( 3500, 100 );
		Sleep( 100 );
	}
}

void CIBScanUltimate_SalesDemoDlg::_SetImageMessage( LPCSTR Format, ... )
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

	if( m_bNeedClearPlaten )
		cr = RGB(255, 0, 0);

	IBSU_SetClientWindowOverlayText( m_nDevHandle, "Calibri", font_size, TRUE, cMessage, x, y, (DWORD)cr );
}

void CIBScanUltimate_SalesDemoDlg::_SetNFIQMessage(LPCTSTR Format, ...)
{
	TCHAR	cMessage[IBSU_MAX_STR_LEN+1];
	va_list arg_ptr;

	va_start( arg_ptr, Format );
	_vsntprintf( cMessage, IBSU_MAX_STR_LEN, Format, arg_ptr );
	va_end( arg_ptr );

	int			font_size = 6;
	int			x = 5;
	int			y = 5;
	COLORREF	cr = RGB(0, 0, 255);

	IBSU_SetClientWindowOverlayText( m_nDevHandle, "Calibri", font_size, TRUE, cMessage, x, y, (DWORD)cr );
}

void CIBScanUltimate_SalesDemoDlg::_SetStatusBarMessage( LPCTSTR Format, ... )
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

void CIBScanUltimate_SalesDemoDlg::_GoJob_442_Process(const int imageStatus,
			const IBSU_ImageData        image,
			const IBSU_ImageType        imageType,
			const int                   detectedFingerCount,
			const int                   segmentImageArrayCount,
			const IBSU_ImageData        *pSegmentImageArray,
			const IBSU_SegmentPosition  *pSegmentPositionArray,
					BOOL				m_HighNFIQ

)
{
	BOOL			b_ReCapture = FALSE;
	CString			askMsg = "";
	DWORD			nRc[4] = {IBSU_FINGER_NONE,IBSU_FINGER_NONE,IBSU_FINGER_NONE,IBSU_FINGER_NONE};
	UINT			pos = 0, match_pos = 0;
	BOOL			isValid = FALSE;
	const int       SecurityLevel = m_Main_Security_Level;

	m_SmearFlag = FALSE;
	m_SmearFingerIndex = 0;
	m_WrongPlaceFlag = FALSE;
	m_WrongPlaceFingerIndex = 0;
	m_WetFingerFlag = FALSE;
	m_WetFingerIndex = 0;

	if( (imageStatus == IBSU_WRN_ROLLING_SMEAR ||
		 imageStatus == IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY ||
		 imageStatus == IBSU_WRN_ROLLING_SHIFTED_VERTICALLY ||
		 imageStatus == (IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY | IBSU_WRN_ROLLING_SHIFTED_VERTICALLY)) && 
		 m_OptionDlg.m_SmearD == TRUE )
	{
		askMsg.Format("[Warning = %s]",_GetwarningMessage(imageStatus));
		b_ReCapture = TRUE;

		m_SmearFlag = TRUE;
		m_SmearFingerIndex = m_MainDlg.m_CaptureFinger;
	}

	if( imageStatus == IBSU_WRN_WET_FINGERS && m_OptionDlg.m_WetFingerD == TRUE )
	{
		askMsg.Format("[Warning = %s]",_GetwarningMessage(imageStatus));
		b_ReCapture = TRUE;

		m_WetFingerFlag = TRUE;
		m_WetFingerIndex = m_MainDlg.m_CaptureFinger;
	}

	if(m_HighNFIQ)
	{
		askMsg +="\nPoor Quality";
		b_ReCapture = TRUE;
	}

	if((m_MainDlg.m_CaptureFinger >= LEFT_LITTLE ) && (m_MainDlg.m_CaptureFinger <= RIGHT_LITTLE ))
	{
		if (m_OptionDlg.m_chkDuplication)
		{
			switch(m_MainDlg.m_CaptureFinger)
			{
				case LEFT_LITTLE:	pos = IBSU_FINGER_LEFT_LITTLE;	break;
				case LEFT_RING:		pos = IBSU_FINGER_LEFT_RING;	break;
				case LEFT_MIDDLE:	pos = IBSU_FINGER_LEFT_MIDDLE;	break;
				case LEFT_INDEX:	pos = IBSU_FINGER_LEFT_INDEX;	break;
				case LEFT_THUMB:	pos = IBSU_FINGER_LEFT_THUMB;	break;
				case RIGHT_THUMB:	pos = IBSU_FINGER_RIGHT_THUMB;	break;
				case RIGHT_INDEX:	pos = IBSU_FINGER_RIGHT_INDEX;	break;
				case RIGHT_MIDDLE:	pos = IBSU_FINGER_RIGHT_MIDDLE;	break;
				case RIGHT_RING:	pos = IBSU_FINGER_RIGHT_RING;	break;
				case RIGHT_LITTLE:	pos = IBSU_FINGER_RIGHT_LITTLE;	break;
			}

			if(imageType == ENUM_IBSU_FLAT_SINGLE_FINGER)
			{
				match_pos = IBSU_FINGER_LEFT_LITTLE | IBSU_FINGER_LEFT_RING | 
					IBSU_FINGER_LEFT_MIDDLE | IBSU_FINGER_LEFT_INDEX | 
					IBSU_FINGER_LEFT_THUMB | IBSU_FINGER_RIGHT_THUMB | 
					IBSU_FINGER_RIGHT_INDEX | IBSU_FINGER_RIGHT_MIDDLE | 
					IBSU_FINGER_RIGHT_RING | IBSU_FINGER_RIGHT_LITTLE;
				match_pos ^= pos;
				
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], match_pos, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] != IBSU_FINGER_NONE)
				{
					askMsg +="\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
				else
				{
					if(imageType == ENUM_IBSU_FLAT_SINGLE_FINGER)
						IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], pos, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
				}
			}
			else
			{
				match_pos = pos;
			
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], match_pos, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] != pos)
				{
					askMsg +="\nWrong finger are on the platen.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
				else
				{
					if(imageType == ENUM_IBSU_FLAT_SINGLE_FINGER)
						IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], pos, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
				}
			}
		}

		if (b_ReCapture)
		{
			askMsg += "\nDo you want a Recapture?";

			if( m_SmearFlag == TRUE || m_WrongPlaceFlag == TRUE || m_WetFingerFlag == TRUE )
				SetTimer(305, 100, NULL);

			if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{
				if( m_SmearFlag == TRUE || m_WrongPlaceFlag == TRUE || m_WetFingerFlag == TRUE )
					KillTimer(305);
				goto done;
			}
			
			if( m_SmearFlag == TRUE || m_WrongPlaceFlag == TRUE || m_WetFingerFlag == TRUE )
				KillTimer(305);

			memset(m_ShiftedFlag, 0, sizeof(m_ShiftedFlag));

			if(m_SmearFlag == TRUE)
				_SetLEDs(m_nDevHandle, imageType, m_SmearFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
			else if(m_WrongPlaceFlag == TRUE)
				_SetLEDs(m_nDevHandle, imageType, m_WrongPlaceFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
			else if(m_WetFingerFlag == TRUE)
				_SetLEDs(m_nDevHandle, imageType, m_WetFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
		}
	}
	else if(m_MainDlg.m_CaptureFinger == BOTH_THUMBS )
	{
		if (m_OptionDlg.m_chkDuplication)
		{
			match_pos = IBSU_FINGER_LEFT_LITTLE | IBSU_FINGER_LEFT_RING | 
				IBSU_FINGER_LEFT_MIDDLE | IBSU_FINGER_LEFT_INDEX | 
				IBSU_FINGER_LEFT_THUMB | IBSU_FINGER_RIGHT_THUMB | 
				IBSU_FINGER_RIGHT_INDEX | IBSU_FINGER_RIGHT_MIDDLE | 
				IBSU_FINGER_RIGHT_RING | IBSU_FINGER_RIGHT_LITTLE;
			match_pos ^= (IBSU_FINGER_LEFT_THUMB | IBSU_FINGER_RIGHT_THUMB);
			
			IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], match_pos, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
			IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], match_pos, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

			if (nRc[0] != IBSU_FINGER_NONE ||
				nRc[1] != IBSU_FINGER_NONE )
			{
				askMsg +="\nThe scanner detected a duplicate finger.";
				b_ReCapture = TRUE;
				
				m_WrongPlaceFlag = TRUE;
				m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
			}
			else
			{
				if(imageType == ENUM_IBSU_FLAT_TWO_FINGERS)
				{
					IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,  ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
					IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
				}
			}
		}

		if (b_ReCapture)
		{
			askMsg += "\nDo you want a Recapture?";

			if( m_WrongPlaceFlag == TRUE )
				SetTimer(305, 100, NULL);

			if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{
				if( m_WrongPlaceFlag == TRUE )
					KillTimer(305);
				goto done;
			}

			if( m_WrongPlaceFlag == TRUE )
				KillTimer(305);

			memset(m_ShiftedFlag, 0, sizeof(m_ShiftedFlag));

			if(m_SmearFlag == TRUE)
				_SetLEDs(m_nDevHandle, imageType, m_SmearFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
			else if(m_WrongPlaceFlag == TRUE)
				_SetLEDs(m_nDevHandle, imageType, m_WrongPlaceFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
			else if(m_WetFingerFlag == TRUE)
				_SetLEDs(m_nDevHandle, imageType, m_WetFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
		}
	}
	else if(m_MainDlg.m_CaptureFinger == RIGHT_HAND)
	{
		IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_RIGHT_HAND, imageType, &isValid);
		if(!isValid)
		{
			askMsg +="\nWrong finger are on the platen.";
			b_ReCapture = TRUE;
			
			m_WrongPlaceFlag = TRUE;
			m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
		}
		else
		{
			if (m_OptionDlg.m_chkDuplication)
			{
				match_pos = IBSU_FINGER_LEFT_LITTLE | IBSU_FINGER_LEFT_RING | 
					IBSU_FINGER_LEFT_MIDDLE | IBSU_FINGER_LEFT_INDEX | 
					IBSU_FINGER_LEFT_THUMB | IBSU_FINGER_RIGHT_THUMB | 
					IBSU_FINGER_RIGHT_INDEX | IBSU_FINGER_RIGHT_MIDDLE | 
					IBSU_FINGER_RIGHT_RING | IBSU_FINGER_RIGHT_LITTLE;
				match_pos ^= (IBSU_FINGER_RIGHT_INDEX | IBSU_FINGER_RIGHT_MIDDLE | 
							  IBSU_FINGER_RIGHT_RING | IBSU_FINGER_RIGHT_LITTLE);

				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], match_pos,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], match_pos,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[2], match_pos,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[2]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[3], match_pos,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[3]);

				if (nRc[0] != IBSU_FINGER_NONE || 
					nRc[1] != IBSU_FINGER_NONE ||
					nRc[2] != IBSU_FINGER_NONE || 
					nRc[3] != IBSU_FINGER_NONE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
					
					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
				else
				{
					if(imageType == ENUM_IBSU_FLAT_FOUR_FINGERS)
					{
						IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
						IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
						IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[2], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
						IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[3], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
					}
				}
			}
		}

		if (b_ReCapture)
		{
			askMsg += "\nDo you want a Recapture?";

			if( m_SmearFlag == TRUE || m_WrongPlaceFlag == TRUE || m_WetFingerFlag == TRUE )
				SetTimer(305, 100, NULL);

			if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{
				if( m_SmearFlag == TRUE || m_WrongPlaceFlag == TRUE || m_WetFingerFlag == TRUE )
					KillTimer(305);
				goto done;
			}

			if( m_SmearFlag == TRUE || m_WrongPlaceFlag == TRUE || m_WetFingerFlag == TRUE )
				KillTimer(305);

			memset(m_ShiftedFlag, 0, sizeof(m_ShiftedFlag));

			if(m_SmearFlag == TRUE)
				_SetLEDs(m_nDevHandle, imageType, m_SmearFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
			else if(m_WrongPlaceFlag == TRUE)
				_SetLEDs(m_nDevHandle, imageType, m_WrongPlaceFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
			else if(m_WetFingerFlag == TRUE)
				_SetLEDs(m_nDevHandle, imageType, m_WetFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
		}
	}
	else if(m_MainDlg.m_CaptureFinger == LEFT_HAND )
	{
		IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_LEFT_HAND, imageType, &isValid);
		if(!isValid)
		{
			askMsg +="\nWrong finger are on the platen.";
			b_ReCapture = TRUE;
			
			m_WrongPlaceFlag = TRUE;
			m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
		}
		else
		{
			if (m_OptionDlg.m_chkDuplication)
			{
				match_pos = IBSU_FINGER_LEFT_LITTLE | IBSU_FINGER_LEFT_RING | 
					IBSU_FINGER_LEFT_MIDDLE | IBSU_FINGER_LEFT_INDEX | 
					IBSU_FINGER_LEFT_THUMB | IBSU_FINGER_RIGHT_THUMB | 
					IBSU_FINGER_RIGHT_INDEX | IBSU_FINGER_RIGHT_MIDDLE | 
					IBSU_FINGER_RIGHT_RING | IBSU_FINGER_RIGHT_LITTLE;
				match_pos ^= (IBSU_FINGER_LEFT_LITTLE | IBSU_FINGER_LEFT_RING | 
							  IBSU_FINGER_LEFT_MIDDLE | IBSU_FINGER_LEFT_INDEX);

				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], match_pos,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], match_pos,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[2], match_pos,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[2]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[3], match_pos,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[3]);

				if (nRc[0] != IBSU_FINGER_NONE || 
					nRc[1] != IBSU_FINGER_NONE ||
					nRc[2] != IBSU_FINGER_NONE || 
					nRc[3] != IBSU_FINGER_NONE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
					
					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
				else
				{
					if(imageType == ENUM_IBSU_FLAT_FOUR_FINGERS)
					{
						IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
						IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_RING,		ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
						IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[2], IBSU_FINGER_LEFT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
						IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[3], IBSU_FINGER_LEFT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
					}
				}
			}
		}

		if (b_ReCapture)
		{
			askMsg += "\nDo you want a Recapture?";

			if( m_SmearFlag == TRUE || m_WrongPlaceFlag == TRUE || m_WetFingerFlag == TRUE )
				SetTimer(305, 100, NULL);

			if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{
				if( m_SmearFlag == TRUE || m_WrongPlaceFlag == TRUE || m_WetFingerFlag == TRUE )
					KillTimer(305);
				goto done;
			}

			if( m_SmearFlag == TRUE || m_WrongPlaceFlag == TRUE || m_WetFingerFlag == TRUE )
				KillTimer(305);

			memset(m_ShiftedFlag, 0, sizeof(m_ShiftedFlag));

			if(m_SmearFlag == TRUE)
				_SetLEDs(m_nDevHandle, imageType, m_SmearFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
			else if(m_WrongPlaceFlag == TRUE)
				_SetLEDs(m_nDevHandle, imageType, m_WrongPlaceFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
			else if(m_WetFingerFlag == TRUE)
				_SetLEDs(m_nDevHandle, imageType, m_WetFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
		}
	}

	return;

done:
	m_nCurrentCaptureStep--;
}

void CIBScanUltimate_SalesDemoDlg::_GoJob_Single_Process(const int imageStatus,
			const IBSU_ImageData        image,
			const IBSU_ImageType        imageType,
			const int                   detectedFingerCount,
			const int                   segmentImageArrayCount,
			const IBSU_ImageData        *pSegmentImageArray,
			const IBSU_SegmentPosition  *pSegmentPositionArray,
			BOOL m_HighNFIQ
			)
{
	BOOL		b_ReCapture = FALSE;
	CString		askMsg = "";
	DWORD		nRc[4] = {IBSU_FINGER_NONE,IBSU_FINGER_NONE,IBSU_FINGER_NONE,IBSU_FINGER_NONE};
	DWORD		pos = IBSU_FINGER_NONE, match_pos = IBSU_FINGER_NONE;
	int			nRc2 = IBSU_STATUS_OK;
	const int       SecurityLevel = m_Main_Security_Level;

	m_SmearFlag = FALSE;
	m_SmearFingerIndex = 0;
	m_WrongPlaceFlag = FALSE;
	m_WrongPlaceFingerIndex = 0;
	m_WetFingerFlag = FALSE;
	m_WetFingerIndex = 0;

	if( (imageStatus == IBSU_WRN_ROLLING_SMEAR ||
		 imageStatus == IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY ||
		 imageStatus == IBSU_WRN_ROLLING_SHIFTED_VERTICALLY ||
		 imageStatus == (IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY | IBSU_WRN_ROLLING_SHIFTED_VERTICALLY)) && 
		 m_OptionDlg.m_SmearD == TRUE )
	{
		askMsg.Format("[Warning = %s]",_GetwarningMessage(imageStatus));
		b_ReCapture = TRUE;

		m_SmearFlag = TRUE;
		m_SmearFingerIndex = m_MainDlg.m_CaptureFinger;
	}

	if( imageStatus == IBSU_WRN_WET_FINGERS && m_OptionDlg.m_WetFingerD == TRUE )
	{
		askMsg.Format("[Warning = %s]",_GetwarningMessage(imageStatus));
		b_ReCapture = TRUE;

		m_WetFingerFlag = TRUE;
		m_WetFingerIndex = m_MainDlg.m_CaptureFinger;
	}
	
	if(m_HighNFIQ)
	{
		askMsg +="\nPoor Quality";
		b_ReCapture = TRUE;
	}

	if((m_MainDlg.m_CaptureFinger >= LEFT_LITTLE ) && (m_MainDlg.m_CaptureFinger <= RIGHT_LITTLE ))
	{
		if(m_OptionDlg.m_chkDuplication)
		{
			switch(m_MainDlg.m_CaptureFinger)
			{
				case LEFT_LITTLE:	pos = IBSU_FINGER_LEFT_LITTLE;	break;
				case LEFT_RING:		pos = IBSU_FINGER_LEFT_RING;	break;
				case LEFT_MIDDLE:	pos = IBSU_FINGER_LEFT_MIDDLE;	break;
				case LEFT_INDEX:	pos = IBSU_FINGER_LEFT_INDEX;	break;
				case LEFT_THUMB:	pos = IBSU_FINGER_LEFT_THUMB;	break;
				case RIGHT_THUMB:	pos = IBSU_FINGER_RIGHT_THUMB;	break;
				case RIGHT_INDEX:	pos = IBSU_FINGER_RIGHT_INDEX;	break;
				case RIGHT_MIDDLE:	pos = IBSU_FINGER_RIGHT_MIDDLE;	break;
				case RIGHT_RING:	pos = IBSU_FINGER_RIGHT_RING;	break;
				case RIGHT_LITTLE:	pos = IBSU_FINGER_RIGHT_LITTLE;	break;
			}

			match_pos = IBSU_FINGER_LEFT_LITTLE | IBSU_FINGER_LEFT_RING | 
				IBSU_FINGER_LEFT_MIDDLE | IBSU_FINGER_LEFT_INDEX | 
				IBSU_FINGER_LEFT_THUMB | IBSU_FINGER_RIGHT_THUMB | 
				IBSU_FINGER_RIGHT_INDEX | IBSU_FINGER_RIGHT_MIDDLE | 
				IBSU_FINGER_RIGHT_RING | IBSU_FINGER_RIGHT_LITTLE;

			match_pos ^= pos;
			
			IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], match_pos, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

			if (nRc[0] != IBSU_FINGER_NONE)
			{
				askMsg +="\nThe scanner detected a duplicate finger.";
				b_ReCapture = TRUE;
				
				m_WrongPlaceFlag = TRUE;
				m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
			}
			else 
			{
				if(imageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
					(imageType == ENUM_IBSU_ROLL_SINGLE_FINGER &&
					m_MainDlg.m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE))
				{
					IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], pos, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
				}
			}
		}
	}

	if (b_ReCapture)
	{
		askMsg += "\nDo you want a Recapture?";

		if( m_SmearFlag == TRUE || m_WrongPlaceFlag == TRUE || m_WetFingerFlag == TRUE )
			SetTimer(305, 100, NULL);

		if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
		{
			if( m_SmearFlag == TRUE || m_WrongPlaceFlag == TRUE || m_WetFingerFlag == TRUE )
				KillTimer(305);
			goto done;
		}
		
		if( m_SmearFlag == TRUE || m_WrongPlaceFlag == TRUE || m_WetFingerFlag == TRUE )
			KillTimer(305);

		memset(m_ShiftedFlag, 0, sizeof(m_ShiftedFlag));

		if(m_SmearFlag == TRUE)
			_SetLEDs(m_nDevHandle, imageType, m_SmearFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
		else if(m_WrongPlaceFlag == TRUE)
			_SetLEDs(m_nDevHandle, imageType, m_WrongPlaceFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
		else if(m_WetFingerFlag == TRUE)
			_SetLEDs(m_nDevHandle, imageType, m_WetFingerIndex, __LED_COLOR_GREEN__, FALSE, m_ShiftedFlag);
	}

	return;

done:
	m_nCurrentCaptureStep--;
}

void CIBScanUltimate_SalesDemoDlg::_GoJob_TwoBasic_Process(const int imageStatus,
			const IBSU_ImageData        image,
			const IBSU_ImageType        imageType,
			const int                   detectedFingerCount,
			const int                   segmentImageArrayCount,
			const IBSU_ImageData        *pSegmentImageArray,
			const IBSU_SegmentPosition  *pSegmentPositionArray,
			BOOL m_HighNFIQ
)
{
	BOOL			b_ReCapture = FALSE;
	BOOL			isValid = FALSE;
	const int       SecurityLevel = m_Main_Security_Level;
	int				m_FirstFingerIdx = 0;
	int				m_SecondFingerIdx = 0;

	m_SmearFlag = FALSE;
	m_SmearFingerIndex = 0;
	m_WrongPlaceFlag = FALSE;
	m_WrongPlaceFingerIndex = 0;

	CString askMsg = "";
	DWORD			nRc[4] = {IBSU_FINGER_NONE,IBSU_FINGER_NONE,IBSU_FINGER_NONE,IBSU_FINGER_NONE};

	if( (imageStatus == IBSU_WRN_ROLLING_SMEAR ||
		 imageStatus == IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY ||
		 imageStatus == IBSU_WRN_ROLLING_SHIFTED_VERTICALLY ||
		 imageStatus == (IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY | IBSU_WRN_ROLLING_SHIFTED_VERTICALLY)) && 
		 m_OptionDlg.m_SmearD == TRUE )
	{
		askMsg.Format("[Warning = %s]",_GetwarningMessage(imageStatus));
		b_ReCapture = TRUE;

		m_SmearFlag = TRUE;
		m_SmearFingerIndex = m_MainDlg.m_CaptureFinger;
	}

	if( imageStatus == IBSU_WRN_WET_FINGERS && m_OptionDlg.m_WetFingerD == TRUE )
	{
		askMsg.Format("[Warning = %s]",_GetwarningMessage(imageStatus));
		b_ReCapture = TRUE;

		m_WetFingerFlag = TRUE;
		m_WetFingerIndex = m_MainDlg.m_CaptureFinger;
	}

	if(m_HighNFIQ)
	{
		askMsg +="\nPoor Quality";
		b_ReCapture = TRUE;
	}

	if(m_MainDlg.m_CaptureFinger == LEFT_THUMB )
	{
		if(m_OptionDlg.m_chkDuplication)
		{
			IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,  ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

			if (nRc[0] == IBSU_FINGER_LEFT_THUMB)
			{
				askMsg +="\nThe scanner detected a duplicate finger.";
				b_ReCapture = TRUE;

				m_WrongPlaceFlag = TRUE;
				m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
			}
		}

		if (b_ReCapture)
		{
			askMsg += "\nDo you want a Recapture?";
			if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{		
				goto done;
			}
		}
			
		IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,  ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
	}
	else if(m_MainDlg.m_CaptureFinger == RIGHT_THUMB )
	{
		if(m_OptionDlg.m_chkDuplication)
		{
			IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

			if (nRc[0] == IBSU_FINGER_RIGHT_THUMB)
			{
				askMsg +="\nThe scanner detected a duplicate finger.";
				b_ReCapture = TRUE;

				m_WrongPlaceFlag = TRUE;
				m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
			}
		}
		if (b_ReCapture)
		{
			askMsg += "\nDo you want a Recapture?";
			if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{		
				goto done;
			}
		}

		IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
	}
	else if(m_MainDlg.m_CaptureFinger == BOTH_THUMBS )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE ))
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,  ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_LEFT_THUMB || 
					nRc[1] == IBSU_FINGER_RIGHT_THUMB)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,  ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_THUMB)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,  ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_THUMB)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,  ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}
	else if(m_MainDlg.m_CaptureFinger == LEFT_INDEX_MIDDLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_LEFT_MIDDLE_INDEX, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;

				m_WrongPlaceFlag = TRUE;
				m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
			}

			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_LEFT_MIDDLE || 
					nRc[1] == IBSU_FINGER_LEFT_INDEX)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{ 
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_INDEX,  ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_INDEX)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_INDEX,  ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,  ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_MIDDLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,  ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}
	else if(m_MainDlg.m_CaptureFinger == LEFT_RING_LITTLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_LEFT_LITTLE_RING, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;

				m_WrongPlaceFlag = TRUE;
				m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
			}

			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_RING,		ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_LEFT_LITTLE || 
					nRc[1] == IBSU_FINGER_LEFT_RING)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_RING,		ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
		{
			if (m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_RING)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_RING,		ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_LITTLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}
	else if(m_MainDlg.m_CaptureFinger == RIGHT_INDEX_MIDDLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_RIGHT_INDEX_MIDDLE, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;

				m_WrongPlaceFlag = TRUE;
				m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
			}

			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);
				
				if (nRc[0] == IBSU_FINGER_RIGHT_INDEX || 
					nRc[1] == IBSU_FINGER_RIGHT_MIDDLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_INDEX)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				
				if (nRc[0] == IBSU_FINGER_RIGHT_MIDDLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}
	else if(m_MainDlg.m_CaptureFinger == RIGHT_RING_LITTLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_RIGHT_RING_LITTLE, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;

				m_WrongPlaceFlag = TRUE;
				m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
			}

			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_RIGHT_RING || 
					nRc[1] == IBSU_FINGER_RIGHT_LITTLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_RING)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_LITTLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;

					m_WrongPlaceFlag = TRUE;
					m_WrongPlaceFingerIndex = m_MainDlg.m_CaptureFinger;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}
return;

done:
	m_nCurrentCaptureStep--;
}

void CIBScanUltimate_SalesDemoDlg::_GoJob_TwoOther1_Process(const int imageStatus,
			const IBSU_ImageData        image,
			const IBSU_ImageType        imageType,
			const int                   detectedFingerCount,
			const int                   segmentImageArrayCount,
			const IBSU_ImageData        *pSegmentImageArray,
			const IBSU_SegmentPosition  *pSegmentPositionArray,
			BOOL m_HighNFIQ
			)
{
	BOOL			b_ReCapture = FALSE;
	CString			askMsg = "";
	DWORD			nRc[4] = {IBSU_FINGER_NONE,IBSU_FINGER_NONE,IBSU_FINGER_NONE,IBSU_FINGER_NONE};
	UINT			pos = 0;
	BOOL			isValid = FALSE;
	const int       SecurityLevel = m_Main_Security_Level;

	m_SmearFlag = FALSE;
	m_SmearFingerIndex = 0;
	m_WrongPlaceFlag = FALSE;
	m_WrongPlaceFingerIndex = 0;
	m_WetFingerFlag = FALSE;
	m_WetFingerIndex = 0;

	if( (imageStatus == IBSU_WRN_ROLLING_SMEAR ||
		 imageStatus == IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY ||
		 imageStatus == IBSU_WRN_ROLLING_SHIFTED_VERTICALLY ||
		 imageStatus == (IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY | IBSU_WRN_ROLLING_SHIFTED_VERTICALLY)) && 
		 m_OptionDlg.m_SmearD == TRUE )
	{
		askMsg.Format("[Warning = %s]",_GetwarningMessage(imageStatus));
		b_ReCapture = TRUE;

		m_SmearFlag = TRUE;
		m_SmearFingerIndex = m_MainDlg.m_CaptureFinger;
	}

	if( imageStatus == IBSU_WRN_WET_FINGERS && m_OptionDlg.m_WetFingerD == TRUE )
	{
		askMsg.Format("[Warning = %s]",_GetwarningMessage(imageStatus));
		b_ReCapture = TRUE;

		m_WetFingerFlag = TRUE;
		m_WetFingerIndex = m_MainDlg.m_CaptureFinger;
	}

	if(m_HighNFIQ)
	{
		askMsg +="\nPoor Quality";
		b_ReCapture = TRUE;
	}
	
	if((m_MainDlg.m_CaptureFinger >= LEFT_LITTLE ) && (m_MainDlg.m_CaptureFinger <= RIGHT_LITTLE ))
	{
		if(m_OptionDlg.m_chkDuplication)
		{
			switch(m_MainDlg.m_CaptureFinger)
			{
				case LEFT_LITTLE:	pos = IBSU_FINGER_LEFT_LITTLE;	break;
				case LEFT_RING:		pos = IBSU_FINGER_LEFT_RING;	break;
				case LEFT_MIDDLE:	pos = IBSU_FINGER_LEFT_MIDDLE;	break;
				case LEFT_INDEX:	pos = IBSU_FINGER_LEFT_INDEX;	break;
				case LEFT_THUMB:	pos = IBSU_FINGER_LEFT_THUMB;	break;
				case RIGHT_THUMB:	pos = IBSU_FINGER_RIGHT_THUMB;	break;
				case RIGHT_INDEX:	pos = IBSU_FINGER_RIGHT_INDEX;	break;
				case RIGHT_MIDDLE:	pos = IBSU_FINGER_RIGHT_MIDDLE;	break;
				case RIGHT_RING:	pos = IBSU_FINGER_RIGHT_RING;	break;
				case RIGHT_LITTLE:	pos = IBSU_FINGER_RIGHT_LITTLE;	break;
			}
			
			IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], pos, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

			if (nRc[0] != pos)
			{
				askMsg += "\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}
		}
		if (b_ReCapture)
		{
			askMsg += "\nDo you want a Recapture?";
			if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{		
				goto done;
			}
		}
		
		IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], pos, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
	}
	else if(m_MainDlg.m_CaptureFinger == BOTH_LEFT_THUMB )
	{
		if(m_OptionDlg.m_chkDuplication)
		{
			IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

			if (nRc[0] == IBSU_FINGER_LEFT_THUMB)
			{
				askMsg += "\nThe scanner detected a duplicate finger.";
				b_ReCapture = TRUE;
			}
		}
		if (b_ReCapture)
		{
			askMsg += "\nDo you want a Recapture?";
			if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{		
				goto done;
			}
		}

		IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		
	}else if(m_MainDlg.m_CaptureFinger == BOTH_RIGHT_THUMB )
	{
		if(m_OptionDlg.m_chkDuplication)
		{
			IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

			if (nRc[0] == IBSU_FINGER_RIGHT_THUMB)
			{
				askMsg += "\nThe scanner detected a duplicate finger.";
				b_ReCapture = TRUE;
			}
		}
		if (b_ReCapture)
		{
			askMsg += "\nDo you want a Recapture?";
			if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{		
				goto done;
			}
		}
		
		IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

	}else if(m_MainDlg.m_CaptureFinger == BOTH_THUMBS)
	{
		if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB))
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_LEFT_THUMB || 
					nRc[1] == IBSU_FINGER_RIGHT_THUMB)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB))
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_THUMB)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB))
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_THUMB)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}
	else if(m_MainDlg.m_CaptureFinger == LEFT_INDEX_MIDDLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_LEFT_MIDDLE_INDEX, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}

			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_LEFT_MIDDLE || 
					nRc[1] == IBSU_FINGER_LEFT_INDEX)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}

			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_INDEX)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_MIDDLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}else if(m_MainDlg.m_CaptureFinger == LEFT_RING_LITTLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_LEFT_LITTLE_RING, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}

			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_LEFT_LITTLE || 
					nRc[1] == IBSU_FINGER_LEFT_RING)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
		{
			if (m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_RING)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_LITTLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}else if(m_MainDlg.m_CaptureFinger == RIGHT_INDEX_MIDDLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_RIGHT_INDEX_MIDDLE, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}

			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);
				
				if (nRc[0] == IBSU_FINGER_RIGHT_INDEX || 
					nRc[1] == IBSU_FINGER_RIGHT_MIDDLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_INDEX)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_MIDDLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}else if(m_MainDlg.m_CaptureFinger == RIGHT_RING_LITTLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_RIGHT_RING_LITTLE, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}

			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_RIGHT_RING || 
					nRc[1] == IBSU_FINGER_RIGHT_LITTLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_RING)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_LITTLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}
return;

done:
	m_nCurrentCaptureStep--;
}

void CIBScanUltimate_SalesDemoDlg::_GoJob_TwoOther2_Process(const int imageStatus,
			const IBSU_ImageData        image,
			const IBSU_ImageType        imageType,
			const int                   detectedFingerCount,
			const int                   segmentImageArrayCount,
			const IBSU_ImageData        *pSegmentImageArray,
			const IBSU_SegmentPosition  *pSegmentPositionArray,
			BOOL m_HighNFIQ
			)
{
	BOOL			b_ReCapture = FALSE;
	CString			askMsg = "";
	DWORD			nRc[4] = {IBSU_FINGER_NONE,IBSU_FINGER_NONE,IBSU_FINGER_NONE,IBSU_FINGER_NONE};
	UINT			pos = 0;
	BOOL			isValid = FALSE;
	const int       SecurityLevel = m_Main_Security_Level;
	
	m_SmearFlag = FALSE;
	m_SmearFingerIndex = 0;
	m_WrongPlaceFlag = FALSE;
	m_WrongPlaceFingerIndex = 0;
	m_WetFingerFlag = FALSE;
	m_WetFingerIndex = 0;

	if( (imageStatus == IBSU_WRN_ROLLING_SMEAR ||
		 imageStatus == IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY ||
		 imageStatus == IBSU_WRN_ROLLING_SHIFTED_VERTICALLY ||
		 imageStatus == (IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY | IBSU_WRN_ROLLING_SHIFTED_VERTICALLY)) && 
		 m_OptionDlg.m_SmearD == TRUE )
	{
		askMsg.Format("[Warning = %s]",_GetwarningMessage(imageStatus));
		b_ReCapture = TRUE;

		m_SmearFlag = TRUE;
		m_SmearFingerIndex = m_MainDlg.m_CaptureFinger;
	}

	if( imageStatus == IBSU_WRN_WET_FINGERS && m_OptionDlg.m_WetFingerD == TRUE )
	{
		askMsg.Format("[Warning = %s]",_GetwarningMessage(imageStatus));
		b_ReCapture = TRUE;

		m_WetFingerFlag = TRUE;
		m_WetFingerIndex = m_MainDlg.m_CaptureFinger;
	}

	if(m_HighNFIQ)
	{
		askMsg +="\nPoor Quality";
		b_ReCapture = TRUE;
	}

	if((m_MainDlg.m_CaptureFinger >= LEFT_LITTLE ) && (m_MainDlg.m_CaptureFinger <= RIGHT_LITTLE ))
	{
		if(m_OptionDlg.m_chkDuplication)
		{
			switch(m_MainDlg.m_CaptureFinger)
			{
				case LEFT_LITTLE:	pos = IBSU_FINGER_LEFT_LITTLE;	break;
				case LEFT_RING:		pos = IBSU_FINGER_LEFT_RING;	break;
				case LEFT_MIDDLE:	pos = IBSU_FINGER_LEFT_MIDDLE;	break;
				case LEFT_INDEX:	pos = IBSU_FINGER_LEFT_INDEX;	break;
				case LEFT_THUMB:	pos = IBSU_FINGER_LEFT_THUMB;	break;
				case RIGHT_THUMB:	pos = IBSU_FINGER_RIGHT_THUMB;	break;
				case RIGHT_INDEX:	pos = IBSU_FINGER_RIGHT_INDEX;	break;
				case RIGHT_MIDDLE:	pos = IBSU_FINGER_RIGHT_MIDDLE;	break;
				case RIGHT_RING:	pos = IBSU_FINGER_RIGHT_RING;	break;
				case RIGHT_LITTLE:	pos = IBSU_FINGER_RIGHT_LITTLE;	break;
			}
			
			IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], pos, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

			if (nRc[0] != pos)
			{
				askMsg += "\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}
		}
		if (b_ReCapture)
		{
			askMsg += "\nDo you want a Recapture?";
			if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{		
				goto done;
			}
		}
		
		IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], pos, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
	}
	else if(m_MainDlg.m_CaptureFinger == BOTH_LEFT_THUMB )
	{
		if(m_OptionDlg.m_chkDuplication)
		{
			IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

			if (nRc[0] == IBSU_FINGER_LEFT_THUMB)
			{
				askMsg += "\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}
		}
		if (b_ReCapture)
		{
			askMsg += "\nDo you want a Recapture?";
			if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{		
				goto done;
			}
		}
		
		IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

	}else if(m_MainDlg.m_CaptureFinger == BOTH_RIGHT_THUMB )
	{
		if(m_OptionDlg.m_chkDuplication)
		{
			IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

			if (nRc[0] == IBSU_FINGER_RIGHT_THUMB)
			{
				askMsg += "\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}
		}
		if (b_ReCapture)
		{
			askMsg += "\nDo you want a Recapture?";
			if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{		
				goto done;
			}
		}

		IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

	}else if(m_MainDlg.m_CaptureFinger == BOTH_THUMBS)
	{
		if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB))
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_LEFT_THUMB || 
					nRc[1] == IBSU_FINGER_RIGHT_THUMB)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB))
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_THUMB)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB))
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_THUMB)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_THUMB,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}
	else if(m_MainDlg.m_CaptureFinger == LEFT_INDEX_MIDDLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_LEFT_MIDDLE_INDEX, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_LEFT_MIDDLE || 
					nRc[1] == IBSU_FINGER_LEFT_INDEX)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}

			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_INDEX)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				
				if (nRc[0] == IBSU_FINGER_LEFT_MIDDLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}else if(m_MainDlg.m_CaptureFinger == LEFT_RING_LITTLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_LEFT_LITTLE_RING, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}

			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_LEFT_LITTLE || 
					nRc[1] == IBSU_FINGER_LEFT_RING)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
		{
			if (m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_RING)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_LITTLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}else if(m_MainDlg.m_CaptureFinger == RIGHT_INDEX_MIDDLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_RIGHT_INDEX_MIDDLE, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);
				
				if (nRc[0] == IBSU_FINGER_RIGHT_INDEX || 
					nRc[1] == IBSU_FINGER_RIGHT_MIDDLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_INDEX)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_MIDDLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}else if(m_MainDlg.m_CaptureFinger == RIGHT_RING_LITTLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_RIGHT_RING_LITTLE, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}

			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_RIGHT_RING || 
					nRc[1] == IBSU_FINGER_RIGHT_LITTLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_RING)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_LITTLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{		
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}
	return;

done:
	m_nCurrentCaptureStep--;

}

void CIBScanUltimate_SalesDemoDlg::_GoJob_TwoOther3_Process(const int imageStatus,
			const IBSU_ImageData        image,
			const IBSU_ImageType        imageType,
			const int                   detectedFingerCount,
			const int                   segmentImageArrayCount,
			const IBSU_ImageData        *pSegmentImageArray,
			const IBSU_SegmentPosition  *pSegmentPositionArray,
			BOOL m_HighNFIQ
			)
{
	BOOL			b_ReCapture = FALSE;
	CString			askMsg = "";
	DWORD			nRc[4] = {IBSU_FINGER_NONE,IBSU_FINGER_NONE,IBSU_FINGER_NONE,IBSU_FINGER_NONE};
	UINT			pos = 0;
	BOOL			isValid = FALSE;
	const int       SecurityLevel = m_Main_Security_Level;

	m_SmearFlag = FALSE;
	m_SmearFingerIndex = 0;
	m_WrongPlaceFlag = FALSE;
	m_WrongPlaceFingerIndex = 0;
	m_WetFingerFlag = FALSE;
	m_WetFingerIndex = 0;

	if( (imageStatus == IBSU_WRN_ROLLING_SMEAR ||
		 imageStatus == IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY ||
		 imageStatus == IBSU_WRN_ROLLING_SHIFTED_VERTICALLY ||
		 imageStatus == (IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY | IBSU_WRN_ROLLING_SHIFTED_VERTICALLY)) && 
		 m_OptionDlg.m_SmearD == TRUE )
	{
		askMsg.Format("[Warning = %s]",_GetwarningMessage(imageStatus));
		b_ReCapture = TRUE;

		m_SmearFlag = TRUE;
		m_SmearFingerIndex = m_MainDlg.m_CaptureFinger;
	}

	if( imageStatus == IBSU_WRN_WET_FINGERS && m_OptionDlg.m_WetFingerD == TRUE )
	{
		askMsg.Format("[Warning = %s]",_GetwarningMessage(imageStatus));
		b_ReCapture = TRUE;

		m_WetFingerFlag = TRUE;
		m_WetFingerIndex = m_MainDlg.m_CaptureFinger;
	}

	if(m_HighNFIQ)
	{
		askMsg +="\nPoor Quality";
		b_ReCapture = TRUE;
	}

	if((m_MainDlg.m_CaptureFinger >= LEFT_LITTLE ) && (m_MainDlg.m_CaptureFinger <= RIGHT_LITTLE ))
	{
		if(m_OptionDlg.m_chkDuplication)
		{
			switch(m_MainDlg.m_CaptureFinger)
			{
				case LEFT_LITTLE:	pos = IBSU_FINGER_LEFT_LITTLE;	break;
				case LEFT_RING:		pos = IBSU_FINGER_LEFT_RING;	break;
				case LEFT_MIDDLE:	pos = IBSU_FINGER_LEFT_MIDDLE;	break;
				case LEFT_INDEX:	pos = IBSU_FINGER_LEFT_INDEX;	break;
				case LEFT_THUMB:	pos = IBSU_FINGER_LEFT_THUMB;	break;
				case RIGHT_THUMB:	pos = IBSU_FINGER_RIGHT_THUMB;	break;
				case RIGHT_INDEX:	pos = IBSU_FINGER_RIGHT_INDEX;	break;
				case RIGHT_MIDDLE:	pos = IBSU_FINGER_RIGHT_MIDDLE;	break;
				case RIGHT_RING:	pos = IBSU_FINGER_RIGHT_RING;	break;
				case RIGHT_LITTLE:	pos = IBSU_FINGER_RIGHT_LITTLE;	break;
			}
			
			IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], pos, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
			
			if (nRc[0] != pos)
			{
				askMsg += "\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}
		}
		if (b_ReCapture)
		{
			askMsg += "\nDo you want a Recapture?";
			if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{
				goto done;
			}
		}

		IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], pos, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

	}else if(m_MainDlg.m_CaptureFinger == BOTH_THUMBS)
	{
		if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB))
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,  ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_LEFT_THUMB || 
					nRc[1] == IBSU_FINGER_RIGHT_THUMB)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,  ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB))
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,  ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_THUMB)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_THUMB,  ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB))
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_THUMB)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_THUMB, ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}
	else if(m_MainDlg.m_CaptureFinger == LEFT_INDEX_MIDDLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_LEFT_MIDDLE_INDEX, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_LEFT_MIDDLE || 
					nRc[1] == IBSU_FINGER_LEFT_INDEX)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_INDEX,  ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_INDEX)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_INDEX,  ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,  ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_MIDDLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_MIDDLE,  ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}else if(m_MainDlg.m_CaptureFinger == LEFT_RING_LITTLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_LEFT_LITTLE_RING, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}

			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_RING,		ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_LEFT_LITTLE || 
					nRc[1] == IBSU_FINGER_LEFT_RING)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_LEFT_RING,		ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
		{
			if (m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_RING)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_RING,		ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_LEFT_LITTLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_LEFT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}else if(m_MainDlg.m_CaptureFinger == RIGHT_INDEX_MIDDLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_RIGHT_INDEX_MIDDLE, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);
				
				if (nRc[0] == IBSU_FINGER_RIGHT_INDEX || 
					nRc[1] == IBSU_FINGER_RIGHT_MIDDLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_INDEX)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}
			
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_INDEX,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_MIDDLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_MIDDLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}else if(m_MainDlg.m_CaptureFinger == RIGHT_RING_LITTLE )
	{
		if((m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE && 
			m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE ))
		{
			IBSU_IsValidFingerGeometry(m_nDevHandle, image, IBSU_FINGER_RIGHT_RING_LITTLE, imageType, &isValid);
			if(!isValid)
			{
				askMsg +="\nWrong finger are on the platen.";
				b_ReCapture = TRUE;
			}

			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[1]);

				if (nRc[0] == IBSU_FINGER_RIGHT_RING || 
					nRc[1] == IBSU_FINGER_RIGHT_LITTLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[1], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
		else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_RING)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_RING,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);

		}else if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
		{
			if(m_OptionDlg.m_chkDuplication)
			{
				IBSU_IsFingerDuplicated(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, SecurityLevel, &nRc[0]);

				if (nRc[0] == IBSU_FINGER_RIGHT_LITTLE)
				{
					askMsg += "\nThe scanner detected a duplicate finger.";
					b_ReCapture = TRUE;
				}
			}
			if (b_ReCapture)
			{
				askMsg += "\nDo you want a Recapture?";
				if (MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
				{
					goto done;
				}
			}

			IBSU_AddFingerImage(m_nDevHandle, pSegmentImageArray[0], IBSU_FINGER_RIGHT_LITTLE,	ENUM_IBSU_FLAT_SINGLE_FINGER, FALSE);
		}
	}
return;

done:
	m_nCurrentCaptureStep--;
}

void CIBScanUltimate_SalesDemoDlg::OnEvent_DeviceCommunicationBreak(
	const int deviceHandle,
	void*     pContext
	)
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SalesDemoDlg *pDlg = reinterpret_cast<CIBScanUltimate_SalesDemoDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->PostMessage( WM_USER_DEVICE_COMMUNICATION_BREAK );
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SalesDemoDlg::OnEvent_PreviewImage(
	const int deviceHandle,
	void*     pContext,
	const	   IBSU_ImageData image
	)
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SalesDemoDlg *pDlg = reinterpret_cast<CIBScanUltimate_SalesDemoDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	//BlinkButton Display
	pDlg->m_ScanFingerDlg.blinkBtn();

	// get CBPPreviewInfo
	pDlg->m_segmentPositionArrayCount = 0;
	memset(pDlg->m_segmentPositionArray, 0, sizeof(IBSU_SegmentPosition) * IBSU_MAX_SEGMENT_COUNT);
	pDlg->m_IsFingerDetected = FALSE;

	for(int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++)
	{
		IBSU_ModifyOverlayText(pDlg->m_nDevHandle, pDlg->m_nOvQualityTextHandle[i], "Arial", 10, TRUE, "", 0, 0, (DWORD)0);
		IBSU_ModifyOverlayQuadrangle(pDlg->m_nDevHandle, pDlg->m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 1, (DWORD)0);
	}

	int window_height;
	CRect rect;
	pDlg->m_ScanFingerDlg.m_CapView.GetClientRect(&rect);
	window_height = rect.Height();

	if(RESERVED_GetCBPPreviewInfo(deviceHandle, "ibkorea1120!", pDlg->m_segmentPositionArray, pDlg->m_segmentPositionArray_for_geo, &pDlg->m_segmentPositionArrayCount, &pDlg->m_IsFingerDetected) == IBSU_STATUS_OK)
	{
		char propertyValue[32];
		IBSU_GetClientWindowProperty(deviceHandle, ENUM_IBSU_WINDOW_PROPERTY_SCALE_FACTOR, propertyValue);
		double scaleFactor = atof(propertyValue);
		IBSU_GetClientWindowProperty(deviceHandle, ENUM_IBSU_WINDOW_PROPERTY_LEFT_MARGIN, propertyValue);
		int leftMargin = atoi(propertyValue);
		IBSU_GetClientWindowProperty(deviceHandle, ENUM_IBSU_WINDOW_PROPERTY_TOP_MARGIN, propertyValue);
		int topMargin = atoi(propertyValue);

		int finger_idx = 0;
		for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
		{
			if(pDlg->m_FingerQuality[i] == ENUM_IBSU_FINGER_NOT_PRESENT)
				continue;

			COLORREF cr = RGB(0, 128, 0);
			int x1, x2, x3, x4, y1, y2, y3, y4;

			int minx=10000, maxx=0, miny=10000, maxy=0;
			if(minx > pDlg->m_segmentPositionArray[finger_idx].x1) minx = pDlg->m_segmentPositionArray[finger_idx].x1;
			if(minx > pDlg->m_segmentPositionArray[finger_idx].x2) minx = pDlg->m_segmentPositionArray[finger_idx].x2;
			if(minx > pDlg->m_segmentPositionArray[finger_idx].x3) minx = pDlg->m_segmentPositionArray[finger_idx].x3;
			if(minx > pDlg->m_segmentPositionArray[finger_idx].x4) minx = pDlg->m_segmentPositionArray[finger_idx].x4;
			if(maxx < pDlg->m_segmentPositionArray[finger_idx].x1) maxx = pDlg->m_segmentPositionArray[finger_idx].x1;
			if(maxx < pDlg->m_segmentPositionArray[finger_idx].x2) maxx = pDlg->m_segmentPositionArray[finger_idx].x2;
			if(maxx < pDlg->m_segmentPositionArray[finger_idx].x3) maxx = pDlg->m_segmentPositionArray[finger_idx].x3;
			if(maxx < pDlg->m_segmentPositionArray[finger_idx].x4) maxx = pDlg->m_segmentPositionArray[finger_idx].x4;

			if(miny > pDlg->m_segmentPositionArray[finger_idx].y1) miny = pDlg->m_segmentPositionArray[finger_idx].y1;
			if(miny > pDlg->m_segmentPositionArray[finger_idx].y2) miny = pDlg->m_segmentPositionArray[finger_idx].y2;
			if(miny > pDlg->m_segmentPositionArray[finger_idx].y3) miny = pDlg->m_segmentPositionArray[finger_idx].y3;
			if(miny > pDlg->m_segmentPositionArray[finger_idx].y4) miny = pDlg->m_segmentPositionArray[finger_idx].y4;
			if(maxy < pDlg->m_segmentPositionArray[finger_idx].y1) maxy = pDlg->m_segmentPositionArray[finger_idx].y1;
			if(maxy < pDlg->m_segmentPositionArray[finger_idx].y2) maxy = pDlg->m_segmentPositionArray[finger_idx].y2;
			if(maxy < pDlg->m_segmentPositionArray[finger_idx].y3) maxy = pDlg->m_segmentPositionArray[finger_idx].y3;
			if(maxy < pDlg->m_segmentPositionArray[finger_idx].y4) maxy = pDlg->m_segmentPositionArray[finger_idx].y4;

			x1 = leftMargin + (int)(minx*scaleFactor);
			x2 = leftMargin + (int)(maxx*scaleFactor);
			x3 = leftMargin + (int)(maxx*scaleFactor);
			x4 = leftMargin + (int)(minx*scaleFactor);
			y1 = topMargin +  (int)(miny*scaleFactor);
			y2 = topMargin +  (int)(miny*scaleFactor);
			y3 = topMargin +  (int)(maxy*scaleFactor);
			y4 = topMargin +  (int)(maxy*scaleFactor);

/*
			x1 = leftMargin + (int)((pDlg->m_segmentPositionArray+i)->x1*scaleFactor);
			x2 = leftMargin + (int)((pDlg->m_segmentPositionArray+i)->x2*scaleFactor);
			x3 = leftMargin + (int)((pDlg->m_segmentPositionArray+i)->x3*scaleFactor);
			x4 = leftMargin + (int)((pDlg->m_segmentPositionArray+i)->x4*scaleFactor);
			y1 = topMargin +  (int)((pDlg->m_segmentPositionArray+i)->y1*scaleFactor);
			y2 = topMargin +  (int)((pDlg->m_segmentPositionArray+i)->y2*scaleFactor);
			y3 = topMargin +  (int)((pDlg->m_segmentPositionArray+i)->y3*scaleFactor);
			y4 = topMargin +  (int)((pDlg->m_segmentPositionArray+i)->y4*scaleFactor);
*/
/*			IBSU_ModifyOverlayQuadrangle(deviceHandle, pDlg->m_nOvSegmentHandle[i],
				x1, y1, x2, y2, x3, y3, x4, y4, 1, (DWORD)cr);

			if(pDlg->m_FingerQuality[i] == ENUM_IBSU_QUALITY_POOR)
				IBSU_ModifyOverlayText(pDlg->m_nDevHandle, pDlg->m_nOvQualityTextHandle[i], "Arial", 11, TRUE, "25", x1, y1, (DWORD)RGB(200,0,0));
			else if(pDlg->m_FingerQuality[i] == ENUM_IBSU_QUALITY_FAIR)
				IBSU_ModifyOverlayText(pDlg->m_nDevHandle, pDlg->m_nOvQualityTextHandle[i], "Arial", 11, TRUE, "50", x1, y1, (DWORD)RGB(255,128,0));
			else if(pDlg->m_FingerQuality[i] == ENUM_IBSU_QUALITY_GOOD)
				IBSU_ModifyOverlayText(pDlg->m_nDevHandle, pDlg->m_nOvQualityTextHandle[i], "Arial", 11, TRUE, "75", x1, y1, (DWORD)RGB(0,200,0));
			//else
			//	IBSU_ModifyOverlayText(pDlg->m_nDevHandle, pDlg->m_nOvQualityTextHandle[i], "Arial", 11, TRUE, "", 0, 0, (DWORD)0);
*/
			finger_idx++;
		}

		memset(pDlg->m_ShiftedFlag, 0, sizeof(pDlg->m_ShiftedFlag));
		for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
		{
			if( pDlg->m_FingerQuality[i] == ENUM_IBSU_QUALITY_INVALID_AREA_TOP ||
				pDlg->m_FingerQuality[i] == ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM )
				pDlg->m_ShiftedFlag[i] |= 1;
			else if( pDlg->m_FingerQuality[i] == ENUM_IBSU_QUALITY_INVALID_AREA_LEFT ||
				pDlg->m_FingerQuality[i] == ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT )
				pDlg->m_ShiftedFlag[i] |= 2;
			else if( pDlg->m_FingerQuality[i] != ENUM_IBSU_FINGER_NOT_PRESENT )
				pDlg->m_ShiftedFlag[i] = 0xFF;
		}
	}
	else
	{
		pDlg->m_segmentPositionArrayCount = 0;
		memset(pDlg->m_segmentPositionArray, 0, sizeof(IBSU_SegmentPosition) * IBSU_MAX_SEGMENT_COUNT);
		pDlg->m_IsFingerDetected = FALSE;
	}

	if( pDlg->m_IsFingerDetected )
	{
		CaptureInfo info = pDlg->m_vecCaptureSeq[pDlg->m_nCurrentCaptureStep];
        pDlg->_SetLEDs(deviceHandle, info.ImageType, pDlg->m_MainDlg.m_CaptureFinger, __LED_COLOR_YELLOW__, FALSE, pDlg->m_ShiftedFlag, pDlg->m_segmentPositionArrayCount);
	}
	else
	{
		CaptureInfo info = pDlg->m_vecCaptureSeq[pDlg->m_nCurrentCaptureStep];
        pDlg->_SetLEDs(deviceHandle, info.ImageType, pDlg->m_MainDlg.m_CaptureFinger, __LED_COLOR_RED__, FALSE, pDlg->m_ShiftedFlag);

		pDlg->m_FirstTimeToCapture = TRUE;
		IBSU_ModifyOverlayText(deviceHandle, pDlg->m_nOvCaptureTimeTextHandle, "Arial", 12, TRUE, "", 10, window_height-80, (DWORD)0 );
	}

	if( pDlg->m_IsFingerDetected && pDlg->m_FirstTimeToCapture )
	{
		pDlg->m_FirstTimeToCapture = FALSE;
		pDlg->m_StartTime = clock();
		IBSU_ModifyOverlayText(deviceHandle, pDlg->m_nOvCaptureTimeTextHandle, "Arial", 12, TRUE, "", 10, window_height-80, (DWORD)0 );
	}

	CString str;
	str.Format("Num of Fingers ( %d )", pDlg->m_segmentPositionArrayCount);
	IBSU_ModifyOverlayText(deviceHandle, pDlg->m_nOvImageTextHandle, "Arial", 12, TRUE, str, 10, window_height-50, (DWORD)0 );

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SalesDemoDlg::OnEvent_FingerCount(
	const int                   deviceHandle,
	void*                       pContext,
	const IBSU_FingerCountState fingerCountState
	)
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SalesDemoDlg *pDlg = reinterpret_cast<CIBScanUltimate_SalesDemoDlg*>(pContext);
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

void CIBScanUltimate_SalesDemoDlg::OnEvent_FingerQuality(
	const int                     deviceHandle,   
	void                          *pContext,       
	const IBSU_FingerQualityState *pQualityArray, 
	const int                     qualityArrayCount    
	)
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SalesDemoDlg *pDlg = reinterpret_cast<CIBScanUltimate_SalesDemoDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	memcpy(pDlg->m_FingerQuality, pQualityArray, sizeof(IBSU_FingerQualityState)*qualityArrayCount);
	pDlg->PostMessage( WM_USER_DRAW_FINGER_QUALITY );

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SalesDemoDlg::OnEvent_DeviceCount(
	const int detectedDevices,
	void      *pContext
	)
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SalesDemoDlg *pDlg = reinterpret_cast<CIBScanUltimate_SalesDemoDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->PostMessage( WM_USER_UPDATE_DEVICE_LIST );
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SalesDemoDlg::OnEvent_InitProgress(
	const int deviceIndex,
	void      *pContext,
	const int progressValue
	)
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SalesDemoDlg *pDlg = reinterpret_cast<CIBScanUltimate_SalesDemoDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
//    if(pDlg->m_bPreInitialization)
	    pDlg->_SetStatusBarMessage(_T("Initializing device... %d%%"), 100);
 //   else
 //       pDlg->_SetStatusBarMessage(_T("Initializing device... %d%%"), progressValue);
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SalesDemoDlg::OnEvent_TakingAcquisition(
	const int				deviceHandle,
	void					*pContext,
	const IBSU_ImageType	imageType
	)
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SalesDemoDlg *pDlg = reinterpret_cast<CIBScanUltimate_SalesDemoDlg*>(pContext);
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

void CIBScanUltimate_SalesDemoDlg::OnEvent_CompleteAcquisition(
	const int				deviceHandle,
	void					*pContext,
	const IBSU_ImageType	imageType
	)
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SalesDemoDlg *pDlg = reinterpret_cast<CIBScanUltimate_SalesDemoDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	if( imageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
		pDlg->PostMessage( WM_USER_BEEP, __BEEP_OK__ );
	else
	{
		pDlg->PostMessage( WM_USER_BEEP, __BEEP_SUCCESS__ );
		pDlg->_SetImageMessage(_T("Remove fingers from sensor"));
		pDlg->_SetStatusBarMessage(_T("Acquisition completed, postprocessing.."));
		IBSU_RedrawClientWindow(deviceHandle); 
	}
	
	pDlg->m_EndTime = clock();

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SalesDemoDlg::OnEvent_AsyncOpenDevice(
	const int                   deviceIndex,
	void                        *pContext,
	const int                   deviceHandle,
	const int                   errorCode
	)
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SalesDemoDlg *pDlg = reinterpret_cast<CIBScanUltimate_SalesDemoDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	pDlg->m_bInitializing = false;
	pDlg->PostMessage( WM_USER_ASYNC_OPEN_DEVICE, errorCode, deviceHandle );

	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SalesDemoDlg::OnEvent_PressedKeyButtons(
	const int                deviceHandle,
	void                     *pContext,
	const int                pressedKeyButtons
	)
{
	if( pContext == NULL )
		return;

	CIBScanUltimate_SalesDemoDlg *pDlg = reinterpret_cast<CIBScanUltimate_SalesDemoDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	TRACE("OnEvent_PressedKeyButtons = %d\n", pressedKeyButtons);

	if(pDlg->m_MainDlg.m_PressedBtn == TRUE)
	{
		if( pressedKeyButtons == __LEFT_KEY_BUTTON__ )
		{
	        IBSU_SetBeeper(deviceHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2/*Sol*/, 4/*100ms = 4*25ms*/,0, 0);
			pDlg->m_ScanFingerDlg.PostMessage(WM_COMMAND, IDC_SCAN_10_FINGERS);
		}
	}
	LeaveCriticalSection(&g_CriticalSection);
}


/****
** This IBSU_CallbackResultImage callback was deprecated since 1.7.0
** Please use IBSU_CallbackResultImageEx instead
*/
void CIBScanUltimate_SalesDemoDlg::OnEvent_ResultImageEx(
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

	CIBScanUltimate_SalesDemoDlg *pDlg = reinterpret_cast<CIBScanUltimate_SalesDemoDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	if( deviceHandle != pDlg->m_nDevHandle )
		ASSERT( FALSE );

	if( pDlg->m_bNeedClearPlaten )
	{
		pDlg->m_bNeedClearPlaten = FALSE;
		pDlg->PostMessage( WM_USER_DRAW_FINGER_QUALITY );
	}

	int window_height;
	CRect rect;
	pDlg->m_ScanFingerDlg.m_CapView.GetClientRect(&rect);
	window_height = rect.Height();

	CString str;
	str.Format("Capture time ( %.1f sec )", (pDlg->m_EndTime-pDlg->m_StartTime)/1000.f);
	IBSU_ModifyOverlayText(deviceHandle, pDlg->m_nOvCaptureTimeTextHandle, "Arial", 12, TRUE, str, 10, window_height-80, (DWORD)0 );

	char imgTypeName[IBSU_MAX_STR_LEN]={0};
	switch( imageType )
	{
	case ENUM_IBSU_ROLL_SINGLE_FINGER:
		strcpy(imgTypeName, "-- Rolling single finger --"); break;
	case ENUM_IBSU_FLAT_SINGLE_FINGER:
		strcpy(imgTypeName, "-- Flat single finger --"); break;
	case ENUM_IBSU_FLAT_TWO_FINGERS:
		strcpy(imgTypeName, "-- Flat two fingers --"); break;
	case ENUM_IBSU_FLAT_THREE_FINGERS:
		strcpy(imgTypeName, "-- Flat three fingers --"); break;
	case ENUM_IBSU_FLAT_FOUR_FINGERS:
		strcpy(imgTypeName, "-- Flat 4 fingers --"); break;
	default:
		strcpy(imgTypeName, "-- Unknown --"); break;
	}

	for(int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++)
	{
		IBSU_ModifyOverlayText(pDlg->m_nDevHandle, pDlg->m_nOvQualityTextHandle[i], "Arial", 10, TRUE, "", 0, 0, (DWORD)0);
		IBSU_ModifyOverlayQuadrangle(pDlg->m_nDevHandle, pDlg->m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 1, (DWORD)0);
	}

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

	//
	int nfiq_score[IBSU_MAX_SEGMENT_COUNT];
	int nfiq2_score[IBSU_MAX_SEGMENT_COUNT];
	int spoof_score[IBSU_MAX_SEGMENT_COUNT];

	CString m_strImageMessage;
	int minimumNfiq=0;
	minimumNfiq = pDlg->m_OptionDlg.m_miniNfiq;

	BOOL m_CaptureisNotCompleted = FALSE;
	BOOL m_HighNFIQ = FALSE;
	/*int m_ErrNfiq		= 1;
	int m_ErrDuplicate = 1;
	int m_ErrPosition  = 1;
	int m_ErrImgStatus = 1;

	CString askMsgNFIQ;
	CString askMsgDuplicate;
	CString askMsgPosition;
	CString askMsgImgStatus;*/

	BOOL bAskRecapture = TRUE;

	if( imageStatus >= IBSU_STATUS_OK )
	{
		CaptureInfo info = pDlg->m_vecCaptureSeq[pDlg->m_nCurrentCaptureStep];
		memset(pDlg->m_ShiftedFlag, 0, sizeof(pDlg->m_ShiftedFlag));
        pDlg->_SetLEDs(deviceHandle, info.ImageType, pDlg->m_MainDlg.m_CaptureFinger, __LED_COLOR_GREEN__, FALSE, pDlg->m_ShiftedFlag, pDlg->m_SmearFlag);

		int nRc, segment_pos=0;
		int score;

		memset(&nfiq_score,0,sizeof(nfiq_score));
		memset(&nfiq2_score,0,sizeof(nfiq2_score));
		memset(&spoof_score,0,sizeof(spoof_score));

        int nfiqtrue=0;
		for(int nfiq=0; nfiq<IBSU_MAX_SEGMENT_COUNT; nfiq++)
		{
			if(pDlg->m_FingerQuality[nfiq] == ENUM_IBSU_FINGER_NOT_PRESENT)
			{
				continue;
			}else
			{
				nRc = IBSU_GetNFIQScore(deviceHandle,(const BYTE*)(pSegmentImageArray+segment_pos)->Buffer,
										(pSegmentImageArray+segment_pos)->Width, (pSegmentImageArray+segment_pos)->Height,
										(pSegmentImageArray+segment_pos)->BitsPerPixel, &score);

				if(nRc == IBSU_STATUS_OK)
					nfiq_score[nfiqtrue] = score;
				else
					nfiq_score[nfiqtrue]=-1;

				if(pDlg ->m_OptionDlg.m_chkUseNFIQ)
				{
					if(score > pDlg->m_OptionDlg.m_miniNfiq)
					{
						m_HighNFIQ = TRUE;
					}
				}

				nRc = IBSU_NFIQ2_ComputeScore((const BYTE*)(pSegmentImageArray+segment_pos)->Buffer,
										(pSegmentImageArray+segment_pos)->Width, (pSegmentImageArray+segment_pos)->Height,
										(pSegmentImageArray+segment_pos)->BitsPerPixel, &score);

				if(nRc == IBSU_STATUS_OK)
					nfiq2_score[nfiqtrue] = score;
				else
					nfiq2_score[nfiqtrue]=-1;

				int x1, x2, x3, x4, y1, y2, y3, y4;
				x1 = leftMargin + (int)((pSegmentPositionArray+segment_pos)->x1*scaleFactor);
				x2 = leftMargin + (int)((pSegmentPositionArray+segment_pos)->x2*scaleFactor);
				x3 = leftMargin + (int)((pSegmentPositionArray+segment_pos)->x3*scaleFactor);
				x4 = leftMargin + (int)((pSegmentPositionArray+segment_pos)->x4*scaleFactor);
				y1 = topMargin +  (int)((pSegmentPositionArray+segment_pos)->y1*scaleFactor);
				y2 = topMargin +  (int)((pSegmentPositionArray+segment_pos)->y2*scaleFactor);
				y3 = topMargin +  (int)((pSegmentPositionArray+segment_pos)->y3*scaleFactor);
				y4 = topMargin +  (int)((pSegmentPositionArray+segment_pos)->y4*scaleFactor);
				CString str;

				int dst_x, dst_y, spoof_x, spoof_y, spoof_width, spoof_height;
				if ((pSegmentImageArray+segment_pos)->Width <= SPOOF_BUF_W)
                {
                    spoof_x = 0;
					dst_x = (SPOOF_BUF_W-(pSegmentImageArray+segment_pos)->Width) / 2;
                    spoof_width = (pSegmentImageArray+segment_pos)->Width;
                }
                else
                {
                    spoof_x = ((pSegmentImageArray+segment_pos)->Width - SPOOF_BUF_W) / 2;
					dst_x = 0;
                    spoof_width = SPOOF_BUF_W;
                }

                if ((pSegmentImageArray+segment_pos)->Height <= SPOOF_BUF_H)
                {
                    spoof_y = 0;
					dst_y = (SPOOF_BUF_H-(pSegmentImageArray+segment_pos)->Height) / 2;
                    spoof_height = (pSegmentImageArray+segment_pos)->Height;
                }
                else
                {
                    spoof_y = ((pSegmentImageArray+segment_pos)->Height - SPOOF_BUF_H) / 2;
					dst_y = 0;
                    spoof_height = SPOOF_BUF_H;
                }

				memset(pDlg->m_SpoofArr, 0xFF, SPOOF_BUF_W*SPOOF_BUF_H);
                unsigned char *ptr = (unsigned char *)(pSegmentImageArray+segment_pos)->Buffer;
                for (int y=0, pos=0; y<spoof_height; y++, pos++)
                {
                    memcpy(&pDlg->m_SpoofArr[((pos+dst_y)*SPOOF_BUF_W)+dst_x], &ptr[(y+spoof_y)*(pSegmentImageArray+segment_pos)->Width+spoof_x], spoof_width);
                }
				for (int y=0; y<SPOOF_BUF_H; y++)
				{
					memcpy(&pDlg->m_SpoofArrFlip[y*SPOOF_BUF_W], &pDlg->m_SpoofArr[(SPOOF_BUF_H-1-y)*SPOOF_BUF_W], SPOOF_BUF_W);
				}

				//IBSU_SaveBitmapImage("d:\\Spoof.bmp", pDlg->m_SpoofArr, SPOOF_BUF_W, SPOOF_BUF_H, -SPOOF_BUF_W, 500, 500);

				nex_sdk_load_image_bytes(pDlg->m_SpoofArrFlip, SPOOF_BUF_H, SPOOF_BUF_W);
				spoof_score[nfiqtrue] = nex_sdk_get_score(NEX_SDK_CLASSIFIER_MLP);

				char strtmp[256] = "";
				
				if( pDlg->m_OptionDlg.m_chkViewNFIQ )
					sprintf(strtmp, "%sNFIQ ( %d ) ", strtmp, nfiq_score[nfiqtrue]);
				if( pDlg->m_OptionDlg.m_chkViewNFIQ2 )
					sprintf(strtmp, "%sNFIQ2 ( %d ) ", strtmp, nfiq2_score[nfiqtrue]);
				if( pDlg->m_OptionDlg.m_chkSpoofD )
					sprintf(strtmp, "%sSpoof ( %d ) ", strtmp, spoof_score[nfiqtrue]);

				str.Format("%s", strtmp);

				IBSU_ModifyOverlayText(pDlg->m_nDevHandle, pDlg->m_nOvQualityTextHandle[nfiqtrue], "Arial", 12, TRUE, str, x2, y2, (DWORD)RGB(0,150,0));

				segment_pos++;
				nfiqtrue++;
			}
		}

		//finger draw
		switch(pDlg->m_MainDlg.m_CaptureMode)
		{
		case TEN_FINGER_ONE_FLAT_CAPTURE:
		case TEN_FINGER_ONE_ROLL_CAPTURE:
		case TEN_FINGER_FLAT_THUMB:
		case TEN_FINGER_FLAT_THUMB_INDEX:
			bAskRecapture = FALSE;
			pDlg->m_MainDlg._SingleDevice(pDlg->m_MainDlg.m_CaptureMode ,imageStatus, nfiq_score, nfiq2_score, spoof_score, image,pSegmentImageArray,pSegmentPositionArray);
			pDlg->_GoJob_Single_Process(imageStatus, image, imageType, detectedFingerCount, segmentImageArrayCount, pSegmentImageArray, pSegmentPositionArray,m_HighNFIQ);	
			break;
		case TEN_FINGER_FOUR_FOUR_TWO:
		case TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH:
		case TEN_FINGER_FOUR_ONE_FOUR_ONE:
		case TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH:
			bAskRecapture = FALSE;
			pDlg->m_MainDlg._FourFingerDevice(pDlg->m_MainDlg.m_CaptureMode ,imageType,segmentImageArrayCount, nfiq_score, nfiq2_score, spoof_score, 
				image,pSegmentImageArray,pSegmentPositionArray);
			pDlg->_GoJob_442_Process(imageStatus, image, imageType, detectedFingerCount, segmentImageArrayCount, pSegmentImageArray, pSegmentPositionArray,m_HighNFIQ);
			break;
		case TWO_FINGER_FLAT_CAPTURE:
		case TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH:
		case TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH:
			bAskRecapture = FALSE;
			pDlg->m_MainDlg._TwoBasicDevice(pDlg->m_MainDlg.m_CaptureMode, imageStatus, segmentImageArrayCount, nfiq_score, nfiq2_score, spoof_score, image, pSegmentImageArray, pSegmentPositionArray);
			pDlg->_GoJob_TwoBasic_Process(imageStatus, image, imageType, detectedFingerCount, segmentImageArrayCount, pSegmentImageArray, pSegmentPositionArray,m_HighNFIQ);
			break;
		case TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH:
		case TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN:
			bAskRecapture = FALSE;
			pDlg->m_MainDlg._TowOtherDevice(pDlg->m_MainDlg.m_CaptureMode, imageStatus, segmentImageArrayCount, nfiq_score, nfiq2_score, spoof_score, image, pSegmentImageArray, pSegmentPositionArray);
			pDlg->_GoJob_TwoOther1_Process(imageStatus, image, imageType, detectedFingerCount, segmentImageArrayCount, pSegmentImageArray, pSegmentPositionArray,m_HighNFIQ);
			break;
		case TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL:
			bAskRecapture = FALSE;
			pDlg->m_MainDlg._TowOtherDevice(pDlg->m_MainDlg.m_CaptureMode, imageStatus, segmentImageArrayCount, nfiq_score, nfiq2_score, spoof_score, image, pSegmentImageArray, pSegmentPositionArray);
			pDlg->_GoJob_TwoOther2_Process(imageStatus, image, imageType, detectedFingerCount, segmentImageArrayCount, pSegmentImageArray, pSegmentPositionArray,m_HighNFIQ);
			break;
		case TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE:
		case TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH:
			bAskRecapture = FALSE;
			pDlg->m_MainDlg._TowOtherDevice(pDlg->m_MainDlg.m_CaptureMode, imageStatus, segmentImageArrayCount, nfiq_score, nfiq2_score, spoof_score, image, pSegmentImageArray, pSegmentPositionArray);
			pDlg->_GoJob_TwoOther3_Process(imageStatus, image, imageType, detectedFingerCount, segmentImageArrayCount, pSegmentImageArray, pSegmentPositionArray,m_HighNFIQ);
			break;
		default:
			break;
		}

		if( imageStatus == IBSU_STATUS_OK )
		{
			m_strImageMessage.Format(_T("%s acquisition completed successfully"), imgTypeName);
			pDlg->_SetImageMessage(m_strImageMessage);
			pDlg->_SetStatusBarMessage(m_strImageMessage);
			if(imageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			pDlg->PostMessage( WM_USER_BEEP, __BEEP_SUCCESS__ );
		}
		else // > IBSU_STATUS_OK
		{
			if(pDlg->m_OptionDlg.m_SmearD ==TRUE)
			{
				m_strImageMessage.Format(_T("%s acquisition Warning (Warning code = %d)"), imgTypeName, imageStatus);
				pDlg->_SetImageMessage(m_strImageMessage);
				pDlg->_SetStatusBarMessage(m_strImageMessage);

				//CString askMsg;
				//askMsgImgStatus.Format("[Warning = %s]",pDlg->_GetwarningMessage(imageStatus));
				//m_ErrImgStatus =0;
			}else
			{
				if(imageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
					pDlg->PostMessage( WM_USER_BEEP, __BEEP_SUCCESS__ );
			}
		}
	}
	else
	{
		m_strImageMessage.Format(_T("%s acquisition failed (Error code = %d)"), imgTypeName, imageStatus);
		pDlg->_SetImageMessage(m_strImageMessage);
		pDlg->_SetStatusBarMessage(m_strImageMessage);
		pDlg->PostMessage( WM_USER_BEEP, __BEEP_FAIL__ );
		// Stop all of acquisition
		pDlg->m_nCurrentCaptureStep = (int)pDlg->m_vecCaptureSeq.size();
	}

	if (bAskRecapture)
	{
		m_CaptureisNotCompleted = TRUE;
	}
	else
	{
		m_CaptureisNotCompleted = FALSE;
	}

	pDlg->PostMessage( WM_USER_CAPTURE_SEQ_NEXT );

	if(m_CaptureisNotCompleted == FALSE){

		pDlg->m_ScanFingerDlg.m_FingerDisplayManager->SetFingerCompleted(pDlg->m_MainDlg.m_CaptureFinger, STATE_COMPLETED);
		pDlg->m_MainDlg.m_CaptureCompleted = TRUE;

		pDlg->m_MainDlg.m_nCurrWidth = image.Width;
		pDlg->m_MainDlg.m_nCurrHeight = image.Height;

		pDlg->m_MainDlg.m_nCurrWidth_1 = 560;
		pDlg->m_MainDlg.m_nCurrHeight_1 = 280;

		pDlg->m_MainDlg.m_nCurrWidth_3 = 1600;
		pDlg->m_MainDlg.m_nCurrHeight_3 = 1500;	
	}
	LeaveCriticalSection(&g_CriticalSection);
}

void CIBScanUltimate_SalesDemoDlg::OnEvent_ClearPlatenAtCapture(
	const int                   deviceHandle,
	void                        *pContext,
	const IBSU_PlatenState      platenState
	)
{
	if( pContext == NULL )
		return;

	CString m_strImageMessage;

	CIBScanUltimate_SalesDemoDlg *pDlg = reinterpret_cast<CIBScanUltimate_SalesDemoDlg*>(pContext);
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

LRESULT CIBScanUltimate_SalesDemoDlg::OnMsg_CaptureSeqStart(WPARAM wParam, LPARAM lParam)
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
				info.capture_finger_btn =1;

				m_vecCaptureSeq.push_back(info);

			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SFF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =2;

				m_vecCaptureSeq.push_back(info);

			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SFF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.capture_finger_btn =3;

				m_vecCaptureSeq.push_back(info);

			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SFF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.capture_finger_btn =4;

				m_vecCaptureSeq.push_back(info);


			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SFF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.capture_finger_btn =5;

				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =6;

				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SFF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =7;

				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SFF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.capture_finger_btn =8;

				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SFF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.capture_finger_btn =9;

				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SFF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.capture_finger_btn =10;

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
				info.capture_finger_btn =1;

			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SFF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =2;


			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SFF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.capture_finger_btn =3;


			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SFF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.capture_finger_btn =4;


			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SFF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.capture_finger_btn =5;


			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =6;


			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SFF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =7;


			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SFF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.capture_finger_btn =8;


			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SFF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.capture_finger_btn =9;


			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SFF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.capture_finger_btn =10;
			}

			m_vecCaptureSeq.push_back(info);
		}
	}
	///////////////////////
	else if( m_MainDlg.m_CaptureMode == TWO_FINGER_FLAT_CAPTURE )
	{
		info.PostCaptuerMessage = _T("Keep fingers on the sensor!");

		if( m_MainDlg.m_CaptureFinger == NONE_FINGER )
		{
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put both thumbs on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =1;

				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =1;

				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
		}
		else
		{
			if( m_MainDlg.m_CaptureFinger == BOTH_THUMBS )
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put both thumbs on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =1;


					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =1;

					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}

			m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd =TRUE;
	}
	//////////////////////////
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
				info.capture_finger_btn =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =2;


				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.capture_finger_btn =3;


				m_vecCaptureSeq.push_back(info);
			}


			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.capture_finger_btn =4;


				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.capture_finger_btn =5;


				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =6;


				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =7;


				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.capture_finger_btn =8;


				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.capture_finger_btn =9;
				m_vecCaptureSeq.push_back(info);

			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.capture_finger_btn =10;


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
				info.capture_finger_btn =1;


			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =2;


			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.capture_finger_btn =3;


			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.capture_finger_btn =4;


			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.capture_finger_btn =5;


			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =6;


			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =7;


			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.capture_finger_btn =8;


			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.capture_finger_btn =9;


			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.capture_finger_btn =10;


			}

			m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd =TRUE;
	}
	/////////////////////////////////

	if( m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB )
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
				info.capture_finger_btn =1;
				m_vecCaptureSeq.push_back(info);

			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =2;
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
				info.capture_finger_btn =1;
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =2;
			}

			m_vecCaptureSeq.push_back(info);
		}
		m_CaptureEnd =TRUE;
	}
	////////////////////////////////
	if( m_MainDlg.m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX )
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
				info.capture_finger_btn =1;


				m_vecCaptureSeq.push_back(info);

			}
			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SFF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =2;


				m_vecCaptureSeq.push_back(info);

			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =3;
				m_vecCaptureSeq.push_back(info);

			}
			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left Index on the sensor!");
				info.fingerName = _T("SFF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =4;
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
				info.capture_finger_btn =1;


			}

			else if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SFF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =2;


			}

			else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =1;


			}

			else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SFF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =2;

			}

			m_vecCaptureSeq.push_back(info);


		}
		m_CaptureEnd =TRUE;
	}
	/////////////////////////////////////////////////////////////////////KOJAK/////////////////////////////////////////////////////////////////////////////////////
	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO )
	{
		info.PostCaptuerMessage = _T("Keep finger on the sensor!");

		if( m_MainDlg.m_CaptureFinger == NONE_FINGER )
		{
			// RIGHT_FOUR
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right four fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Four_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
				info.NumberOfFinger = 4;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			// LEFT_FOUR
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left four fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
				info.NumberOfFinger = 4;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
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
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
		}
		else
		{
			if(m_MainDlg.m_CaptureFinger == RIGHT_HAND)
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right four fingers on the sensor!");
					info.fingerName = _T("TFF_Right_Four_Fingers");
					info.capture_finger_idx = RIGHT_HAND;
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
					info.NumberOfFinger = 4;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if(m_MainDlg.m_CaptureFinger == LEFT_HAND)
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
				{
					info.PreCaptureMessage = _T("Please put left four fingers on the sensor!");
					info.fingerName = _T("TFF_Left_Fingers");
					info.capture_finger_idx = LEFT_HAND;
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
					info.NumberOfFinger = 4;
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
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
		}
		m_CaptureEnd =TRUE;
	}

	////Kojak 2nd
	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH )
	{
		info.PostCaptuerMessage = _T("Keep finger on the sensor!");

		if( m_MainDlg.m_CaptureFinger == NONE_FINGER )
		{
			// RIGHT_FOUR
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{

				info.PreCaptureMessage = _T("Please put right four fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Four_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
				info.NumberOfFinger = 4;
				m_vecCaptureSeq.push_back(info);
			}


			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}


			// LEFT_FOUR
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left four fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
				info.NumberOfFinger = 4;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =2;
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
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//end 442 
			//start 10 rolled
			//Roll
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.capture_finger_btn =6;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}


			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.capture_finger_btn =7;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.capture_finger_btn =8;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =9;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =10;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.capture_finger_btn =11;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.capture_finger_btn =12;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.capture_finger_btn =13;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

		}
		else
		{
			if(m_MainDlg.m_CaptureFinger == RIGHT_HAND)
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right four fingers on the sensor!");
					info.fingerName = _T("TFF_Right_Four_Fingers");
					info.capture_finger_idx = RIGHT_HAND;
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
					info.NumberOfFinger = 4;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if(m_MainDlg.m_CaptureFinger == LEFT_HAND)
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
				{
					info.PreCaptureMessage = _T("Please put left four fingers on the sensor!");
					info.fingerName = _T("TFF_Left_Fingers");
					info.capture_finger_idx = LEFT_HAND;
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
					info.NumberOfFinger = 4;
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
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			//end 442
			//start rolled
			//Roll
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.capture_finger_btn =6;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}


			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.capture_finger_btn =7;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.capture_finger_btn =8;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =9;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =10;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.capture_finger_btn =11;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.capture_finger_btn =12;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.capture_finger_btn =13;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

		}
		m_CaptureEnd =TRUE;
	}

	////Kojak 3nd 4-1-4-1
	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE )
	{
		info.PostCaptuerMessage = _T("Keep finger on the sensor!");

		if( m_MainDlg.m_CaptureFinger == NONE_FINGER )
		{
			// RIGHT_FOUR
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{

				info.PreCaptureMessage = _T("Please put right four fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Four_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
				info.NumberOfFinger = 4;
				m_vecCaptureSeq.push_back(info);
			}


			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			// RIGHT_THUMB
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("TFF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}


			// LEFT_FOUR
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left four fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
				info.NumberOfFinger = 4;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			// LEFT_THUMB
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("TFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
		}
		else
		{
			if(m_MainDlg.m_CaptureFinger == RIGHT_HAND)
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right four fingers on the sensor!");
					info.fingerName = _T("TFF_Right_Four_Fingers");
					info.capture_finger_idx = RIGHT_HAND;
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
					info.NumberOfFinger = 4;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB )
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
					info.fingerName = _T("TFF_Right_Thumb");
					info.capture_finger_idx = RIGHT_THUMB;
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if(m_MainDlg.m_CaptureFinger == LEFT_HAND)
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
				{
					info.PreCaptureMessage = _T("Please put left four fingers on the sensor!");
					info.fingerName = _T("TFF_Left_Fingers");
					info.capture_finger_idx = LEFT_HAND;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
					info.NumberOfFinger = 4;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB )
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
					info.fingerName = _T("TFF_Left_Thumb");
					info.capture_finger_idx = LEFT_THUMB;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
		}
		m_CaptureEnd =TRUE;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////Kojak 2nd
	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		info.PostCaptuerMessage = _T("Keep finger on the sensor!");

		if( m_MainDlg.m_CaptureFinger == NONE_FINGER )
		{
			// RIGHT_FOUR
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{

				info.PreCaptureMessage = _T("Please put right four fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Four_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
				info.NumberOfFinger = 4;
				m_vecCaptureSeq.push_back(info);
			}


			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right fingers on the sensor!");
				info.fingerName = _T("TFF_Right_Fingers");
				info.capture_finger_idx = RIGHT_HAND;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT THUMB
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("TFF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			// LEFT_FOUR
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left four fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
				info.NumberOfFinger = 4;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THREE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_THREE_FINGERS;
				info.NumberOfFinger = 3;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_TWO
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_SINGLE
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left fingers on the sensor!");
				info.fingerName = _T("TFF_Left_Fingers");
				info.capture_finger_idx = LEFT_HAND;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			// LEFT THUMB
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("TFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn = 4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//end 4141 
			//start 10 rolled
			//Roll
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =6;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.capture_finger_btn =7;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}


			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.capture_finger_btn =8;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.capture_finger_btn =9;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =10;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =11;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.capture_finger_btn =12;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.capture_finger_btn =13;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.capture_finger_btn =14;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}
		}
		else
		{
			if(m_MainDlg.m_CaptureFinger == RIGHT_HAND)
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right four fingers on the sensor!");
					info.fingerName = _T("TFF_Right_Four_Fingers");
					info.capture_finger_idx = RIGHT_HAND;
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
					info.NumberOfFinger = 4;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB )
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
				{
					info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
					info.fingerName = _T("TFF_Right_Thumb");
					info.capture_finger_idx = RIGHT_THUMB;
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if(m_MainDlg.m_CaptureFinger == LEFT_HAND)
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
					m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
				{
					info.PreCaptureMessage = _T("Please put left four fingers on the sensor!");
					info.fingerName = _T("TFF_Left_Fingers");
					info.capture_finger_idx = LEFT_HAND;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS;
					info.NumberOfFinger = 4;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB )
			{
				if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
				{
					info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
					info.fingerName = _T("TFF_Left_Thumb");
					info.capture_finger_idx = LEFT_THUMB;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			//end 442
			//start rolled
			//Roll
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =6;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.capture_finger_btn =7;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.capture_finger_btn =8;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.capture_finger_btn =9;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =10;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =11;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.capture_finger_btn =12;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.capture_finger_btn =13;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.capture_finger_btn =14;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}
		}
		m_CaptureEnd =TRUE;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH )
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
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
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
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
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
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
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
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
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
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =5;
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
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right index on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right middle on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.capture_finger_btn =1;
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
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right ring on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right little on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
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
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left index on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left middle on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
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
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left ring on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left little on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
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
					info.capture_finger_btn =5;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =5;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =5;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
		}
		m_CaptureEnd =TRUE;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH )
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
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
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
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
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
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
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
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THUMB
			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SFF_Right_Thumb");
				info.capture_finger_btn =5;
				info.capture_finger_idx = RIGHT_THUMB;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THUMB
			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =6;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;
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
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right index on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right middle on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.capture_finger_btn =1;
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
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right ring on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right little on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
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
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left index on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left middle on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
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
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left ring on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left little on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB )
			{
				//RIGHT_THUMB

				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SFF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THUMB
			else if(m_MainDlg.m_CaptureFinger == LEFT_THUMB )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =6;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}
		}
		m_CaptureEnd =TRUE;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH )
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
				info.capture_finger_btn =1;

				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}

			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
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
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;

				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
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
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;

				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
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
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;

				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THUMB
			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SFF_Right_Thumb");
				info.capture_finger_idx = BOTH_RIGHT_THUMB;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THUMB
			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = BOTH_LEFT_THUMB;
				info.capture_finger_btn =6;

				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			//Roll
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.capture_finger_btn =7;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =8;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.capture_finger_btn =9;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}


			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.capture_finger_btn =10;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.capture_finger_btn =11;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =12;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =13;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.capture_finger_btn =14;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.capture_finger_btn =15;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.capture_finger_btn =16;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
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
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;

					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right index on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right middle on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.capture_finger_btn =1;
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
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;

					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right ring on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right little on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
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
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;

					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left index on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left middle on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
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
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;

					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left ring on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left little on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB )
			{
				//RIGHT_THUMB

				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SFF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THUMB
			else if(m_MainDlg.m_CaptureFinger == LEFT_THUMB )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =6;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			//Roll
			if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.capture_finger_btn =7;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =8;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.capture_finger_btn =9;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.capture_finger_btn =10;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.capture_finger_btn =11;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =12;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =13;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.capture_finger_btn =14;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.capture_finger_btn =15;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.capture_finger_btn =16;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			m_vecCaptureSeq.push_back(info);

		}
		m_CaptureEnd =TRUE;
	}

	///////////////////////////////////
	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN )
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
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;

				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
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
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;

				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
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
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;

				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
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
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;

				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THUMB
			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SFF_Right_Thumb");
				info.capture_finger_idx = BOTH_RIGHT_THUMB;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THUMB
			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = BOTH_LEFT_THUMB;
				info.capture_finger_btn =6;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			//ROll

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.capture_finger_btn =7;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =8;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.capture_finger_btn =9;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.capture_finger_btn =10;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.capture_finger_btn =11;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =12;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =13;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}


			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.capture_finger_btn =14;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.capture_finger_btn =15;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}

			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.capture_finger_btn =16;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
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
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;

					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right index on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right middle on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.capture_finger_btn =1;
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
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;

					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right ring on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right little on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
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
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;

					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left index on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left middle on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
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
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
					info.NumberOfFinger = 2;

					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left ring on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left little on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB )
			{
				//RIGHT_THUMB

				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SFF_Right_Thumb");
				info.capture_finger_idx = BOTH_RIGHT_THUMB;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THUMB
			else if(m_MainDlg.m_CaptureFinger == LEFT_THUMB )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = BOTH_LEFT_THUMB;
				info.capture_finger_btn =6;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			//Roll
			if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.capture_finger_btn =7;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =8;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.capture_finger_btn =9;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.capture_finger_btn =10;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.capture_finger_btn =11;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =12;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =13;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.capture_finger_btn =14;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.capture_finger_btn =15;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}
			else if( m_MainDlg.m_CaptureFinger == LEFT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.capture_finger_btn =16;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);

			}

		}
		m_CaptureEnd =TRUE;
	}
	//////////////////////////////////
	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE )
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
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;

				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;

				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
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
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;

				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;

				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
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
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;

				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;

				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
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
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;

				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;

				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
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
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;

				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;

				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;

				m_vecCaptureSeq.push_back(info);
			}


			//Single flat
			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SFF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.capture_finger_btn =6;
				info.ImageType =ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SFF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.capture_finger_btn =7;
				info.ImageType =ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SFF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.capture_finger_btn =8;
				info.ImageType =ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}


			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SFF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.capture_finger_btn =9;
				info.ImageType =ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SFF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.capture_finger_btn =10;
				info.ImageType =ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =11;
				info.ImageType =ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SFF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.capture_finger_btn =12;
				info.ImageType =ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SFF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.capture_finger_btn =13;
				info.ImageType =ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SFF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.capture_finger_btn =14;
				info.ImageType =ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}

			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SFF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.capture_finger_btn =15;
				info.ImageType =ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;

				m_vecCaptureSeq.push_back(info);
			}
			/////
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
					info.capture_finger_btn =1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right index on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right middle on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.capture_finger_btn =1;
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
					info.capture_finger_btn =2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right ring on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right little on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
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
					info.capture_finger_btn =3;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left index on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left middle on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
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
					info.capture_finger_btn =4;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left ring on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left little on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
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
					info.capture_finger_btn =5;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =5;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =5;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				////////////////////////
				else if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
					info.fingerName = _T("SRF_Right_Thumb");
					info.capture_finger_idx = RIGHT_THUMB;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =6;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_MainDlg.m_CaptureFinger == RIGHT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right index on the sensor!");
					info.fingerName = _T("SRF_Right_Index");
					info.capture_finger_idx = RIGHT_INDEX;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =7;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_MainDlg.m_CaptureFinger == RIGHT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right middle on the sensor!");
					info.fingerName = _T("SRF_Right_Middle");
					info.capture_finger_idx = RIGHT_MIDDLE;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =8;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_MainDlg.m_CaptureFinger == RIGHT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right ring on the sensor!");
					info.fingerName = _T("SRF_Right_Ring");
					info.capture_finger_idx = RIGHT_RING;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =9;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_MainDlg.m_CaptureFinger == RIGHT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right little on the sensor!");
					info.fingerName = _T("SRF_Right_Little");
					info.capture_finger_idx = RIGHT_LITTLE;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =10;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
					info.fingerName = _T("SRF_Left_Thumb");
					info.capture_finger_idx = LEFT_THUMB;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =11;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_MainDlg.m_CaptureFinger == LEFT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left index on the sensor!");
					info.fingerName = _T("SRF_Left_Index");
					info.capture_finger_idx = LEFT_INDEX;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =12;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_MainDlg.m_CaptureFinger == LEFT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left middle on the sensor!");
					info.fingerName = _T("SRF_Left_Middle");
					info.capture_finger_idx = LEFT_MIDDLE;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =13;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_MainDlg.m_CaptureFinger == LEFT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left ring on the sensor!");
					info.fingerName = _T("SRF_Left_Ring");
					info.capture_finger_idx = LEFT_RING;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =14;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_MainDlg.m_CaptureFinger == LEFT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left little on the sensor!");
					info.fingerName = _T("SRF_Left_Little");
					info.capture_finger_idx = LEFT_LITTLE;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =15;
					m_vecCaptureSeq.push_back(info);
				}

			}
		}
		m_CaptureEnd =TRUE;
	}

	//////////////////////////////////
	//jiae19
	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL )
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
				info.capture_finger_btn =1;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
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
				info.capture_finger_btn =2;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
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
				info.capture_finger_btn =3;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
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
				info.capture_finger_btn =4;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//Both
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE &&
				m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put both thumbs on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.NumberOfFinger = 2;
				info.capture_finger_btn =5;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_THUMB
			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SFF_Right_Thumb");
				info.capture_finger_idx = BOTH_RIGHT_THUMB;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =6;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THUMB
			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = BOTH_LEFT_THUMB;
				info.capture_finger_btn =7;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}


			//Roll
			//RIGHT_THUMB
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =8;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_INDEX
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =9;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_MIDDLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =10;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_RING
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =11;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_LITTLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =12;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_THUMB
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =13;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_INDEX
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =14;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_MIDDLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =15;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_RING
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =16;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_LITTLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =17;
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
					info.capture_finger_btn =1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right index on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					info.capture_finger_btn =1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right middle on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.capture_finger_btn =1;
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
					info.capture_finger_btn =2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right ring on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right little on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
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
					info.capture_finger_btn =3;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left index on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left middle on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
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
					info.capture_finger_btn =4;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left ring on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left little on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			//BOth
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
					info.capture_finger_btn =5;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =5;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =5;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB )
			{
				//RIGHT_THUMB

				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SFF_Right_Thumb");
				info.capture_finger_idx = BOTH_RIGHT_THUMB;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =6;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THUMB
			else if(m_MainDlg.m_CaptureFinger == LEFT_THUMB )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = BOTH_LEFT_THUMB;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =7;
				m_vecCaptureSeq.push_back(info);
			}

			//Roll
			//RIGHT_THUMB
			if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =8;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_INDEX
			else if( m_MainDlg.m_CaptureFinger == RIGHT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =9;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_MIDDLE
			else if( m_MainDlg.m_CaptureFinger == RIGHT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =10;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_RING
			else if( m_MainDlg.m_CaptureFinger == RIGHT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =11;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_LITTLE
			else if( m_MainDlg.m_CaptureFinger == RIGHT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =12;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_THUMB
			else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =13;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_INDEX
			else if( m_MainDlg.m_CaptureFinger == LEFT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =14;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_MIDDLE
			else if( m_MainDlg.m_CaptureFinger == LEFT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =15;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_RING
			else if( m_MainDlg.m_CaptureFinger == LEFT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =16;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_LITTLE
			else if( m_MainDlg.m_CaptureFinger == LEFT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =17;
				m_vecCaptureSeq.push_back(info);
			}

		}
		m_CaptureEnd =TRUE;
	}
	////////////////////////////////////



	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH )
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
				info.capture_finger_btn =1;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
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
				info.capture_finger_btn =2;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
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
				info.capture_finger_btn =3;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
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
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS;
				info.capture_finger_btn =4;
				info.NumberOfFinger = 2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_THUMB
			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SFF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =5;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THUMB
			if(m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.capture_finger_btn =6;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;
				m_vecCaptureSeq.push_back(info);
			}
			//Roll
			//RIGHT_THUMB
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =7;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_INDEX
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =8;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_MIDDLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =9;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_RING
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =10;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_LITTLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =11;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THUMB
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =12;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_INDEX
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =13;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_MIDDLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =14;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_RING
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =15;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_LITTLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =16;
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
					info.capture_finger_btn =1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right index on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right middle on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.capture_finger_btn =1;
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
					info.capture_finger_btn =2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right ring on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right little on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
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
					info.capture_finger_btn =3;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left index on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left middle on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
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
					info.capture_finger_btn =4;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left ring on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left little on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			else if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB )
			{
				//RIGHT_THUMB

				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SFF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =5;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THUMB
			else if(m_MainDlg.m_CaptureFinger == LEFT_THUMB )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SFF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =6;
				m_vecCaptureSeq.push_back(info);
			}

			//Roll
			//RIGHT_THUMB
			if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =7;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_INDEX
			else if( m_MainDlg.m_CaptureFinger == RIGHT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =8;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_MIDDLE
			else if( m_MainDlg.m_CaptureFinger == RIGHT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =9;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_RING
			else if( m_MainDlg.m_CaptureFinger == RIGHT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =10;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_LITTLE
			else if( m_MainDlg.m_CaptureFinger == RIGHT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =11;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_THUMB
			else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =12;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_INDEX
			else if( m_MainDlg.m_CaptureFinger == LEFT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =13;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_MIDDLE
			else if( m_MainDlg.m_CaptureFinger == LEFT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =14;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_RING
			else if( m_MainDlg.m_CaptureFinger == LEFT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =15;
				m_vecCaptureSeq.push_back(info);
			}
			//LEFT_LITTLE
			else if( m_MainDlg.m_CaptureFinger == LEFT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =16;
				m_vecCaptureSeq.push_back(info);
			}

		}
		m_CaptureEnd =TRUE;
	}
	//////////////////////////////////

	else if( m_MainDlg.m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH )
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
				info.capture_finger_btn =1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("TFF_Right_Index_Middle");
				info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
				info.capture_finger_btn =1;
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
				info.capture_finger_btn =2;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("TFF_Right_Ring_Little");
				info.capture_finger_idx = RIGHT_RING_LITTLE;
				info.capture_finger_btn =2;
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
				info.capture_finger_btn =3;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("TFF_Left_Index_Middle");
				info.capture_finger_idx = LEFT_INDEX_MIDDLE;
				info.capture_finger_btn =3;
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
				info.capture_finger_btn =4;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("TFF_Left_Ring_Little");
				info.capture_finger_idx = LEFT_RING_LITTLE;
				info.capture_finger_btn =4;
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
				info.capture_finger_btn =5;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}
			else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("TFF_Both_Thumbs");
				info.capture_finger_idx = BOTH_THUMBS;
				info.capture_finger_btn =5;
				info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
				info.NumberOfFinger = 1;
				m_vecCaptureSeq.push_back(info);
			}

			//Roll
			//RIGHT_THUMB
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
				info.fingerName = _T("SRF_Right_Thumb");
				info.capture_finger_idx = RIGHT_THUMB;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =6;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_INDEX
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right index on the sensor!");
				info.fingerName = _T("SRF_Right_Index");
				info.capture_finger_idx = RIGHT_INDEX;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =7;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_MIDDLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right middle on the sensor!");
				info.fingerName = _T("SRF_Right_Middle");
				info.capture_finger_idx = RIGHT_MIDDLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =8;
				m_vecCaptureSeq.push_back(info);
			}
			//RIGHT_RING
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right ring on the sensor!");
				info.fingerName = _T("SRF_Right_Ring");
				info.capture_finger_idx = RIGHT_RING;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =9;
				m_vecCaptureSeq.push_back(info);
			}

			//RIGHT_LITTLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put right little on the sensor!");
				info.fingerName = _T("SRF_Right_Little");
				info.capture_finger_idx = RIGHT_LITTLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =10;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_THUMB
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
				info.fingerName = _T("SRF_Left_Thumb");
				info.capture_finger_idx = LEFT_THUMB;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =11;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_INDEX
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left index on the sensor!");
				info.fingerName = _T("SRF_Left_Index");
				info.capture_finger_idx = LEFT_INDEX;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =12;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_MIDDLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left middle on the sensor!");
				info.fingerName = _T("SRF_Left_Middle");
				info.capture_finger_idx = LEFT_MIDDLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =13;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_RING
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left ring on the sensor!");
				info.fingerName = _T("SRF_Left_Ring");
				info.capture_finger_idx = LEFT_RING;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =14;
				m_vecCaptureSeq.push_back(info);
			}

			//LEFT_LITTLE
			if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				info.PreCaptureMessage = _T("Please put left little on the sensor!");
				info.fingerName = _T("SRF_Left_Little");
				info.capture_finger_idx = LEFT_LITTLE;
				info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
				info.NumberOfFinger =1;
				info.capture_finger_btn =15;
				m_vecCaptureSeq.push_back(info);
			}

		}
		else
		{
			//RIGHT_INDEX_MIDDLE
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
					info.capture_finger_btn =1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right index on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.capture_finger_btn =1;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right middle on the sensor!");
					info.fingerName = _T("TFF_Right_Index_Middle");
					info.capture_finger_btn =1;
					info.capture_finger_idx = RIGHT_INDEX_MIDDLE;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			//RIGHT_RING_LITTLE
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
					info.capture_finger_btn =2;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right ring on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right little on the sensor!");
					info.fingerName = _T("TFF_Right_Ring_Little");
					info.capture_finger_idx = RIGHT_RING_LITTLE;
					info.capture_finger_btn =2;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			//LEFT_INDEX_MIDDLE
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
					info.capture_finger_btn =3;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left index on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left middle on the sensor!");
					info.fingerName = _T("TFF_Left_Index_Middle");
					info.capture_finger_idx = LEFT_INDEX_MIDDLE;
					info.capture_finger_btn =3;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			//LEFT_RING_LITTLE
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
					info.capture_finger_btn =4;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left ring on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left little on the sensor!");
					info.fingerName = _T("TFF_Left_Ring_Little");
					info.capture_finger_idx = LEFT_RING_LITTLE;
					info.capture_finger_btn =4;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
			}
			//BOTH_THUMBS
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
					info.capture_finger_btn =5;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =5;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				else if( m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
					info.fingerName = _T("TFF_Both_Thumbs");
					info.capture_finger_idx = BOTH_THUMBS;
					info.capture_finger_btn =5;
					info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER;
					info.NumberOfFinger = 1;
					m_vecCaptureSeq.push_back(info);
				}
				//Roll
				//RIGHT_THUMB
				if( m_MainDlg.m_CaptureFinger == RIGHT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right thumb on the sensor!");
					info.fingerName = _T("SRF_Right_Thumb");
					info.capture_finger_idx = RIGHT_THUMB;
					info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =6;
					m_vecCaptureSeq.push_back(info);
				}
				//RIGHT_INDEX
				else if( m_MainDlg.m_CaptureFinger == RIGHT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right index on the sensor!");
					info.fingerName = _T("SRF_Right_Index");
					info.capture_finger_idx = RIGHT_INDEX;
					info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =7;
					m_vecCaptureSeq.push_back(info);
				}
				//RIGHT_MIDDLE
				else if( m_MainDlg.m_CaptureFinger == RIGHT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right middle on the sensor!");
					info.fingerName = _T("SRF_Right_Middle");
					info.capture_finger_idx = RIGHT_MIDDLE;
					info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =8;
					m_vecCaptureSeq.push_back(info);
				}
				//RIGHT_RING
				else if( m_MainDlg.m_CaptureFinger == RIGHT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right ring on the sensor!");
					info.fingerName = _T("SRF_Right_Ring");
					info.capture_finger_idx = RIGHT_RING;
					info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =9;
					m_vecCaptureSeq.push_back(info);
				}
				//RIGHT_LITTLE
				else if( m_MainDlg.m_CaptureFinger == RIGHT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put right little on the sensor!");
					info.fingerName = _T("SRF_Right_Little");
					info.capture_finger_idx = RIGHT_LITTLE;
					info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =10;
					m_vecCaptureSeq.push_back(info);
				}
				//LEFT_THUMB
				else if( m_MainDlg.m_CaptureFinger == LEFT_THUMB && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left thumb on the sensor!");
					info.fingerName = _T("SRF_Left_Thumb");
					info.capture_finger_idx = LEFT_THUMB;
					info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =11;
					m_vecCaptureSeq.push_back(info);
				}
				//LEFT_INDEX
				else if( m_MainDlg.m_CaptureFinger == LEFT_INDEX && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left index on the sensor!");
					info.fingerName = _T("SRF_Left_Index");
					info.capture_finger_idx = LEFT_INDEX;
					info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =12;
					m_vecCaptureSeq.push_back(info);
				}
				//LEFT_MIDDLE
				else if( m_MainDlg.m_CaptureFinger == LEFT_MIDDLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left middle on the sensor!");
					info.fingerName = _T("SRF_Left_Middle");
					info.capture_finger_idx = LEFT_MIDDLE;
					info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =13;
					m_vecCaptureSeq.push_back(info);
				}
				//LEFT_RING
				else if( m_MainDlg.m_CaptureFinger == LEFT_RING && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left ring on the sensor!");
					info.fingerName = _T("SRF_Left_Ring");
					info.capture_finger_idx = LEFT_RING;
					info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =14;
					m_vecCaptureSeq.push_back(info);
				}
				//LEFT_LITTLE
				else if( m_MainDlg.m_CaptureFinger == LEFT_LITTLE && m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
				{
					info.PreCaptureMessage = _T("Please put left little on the sensor!");
					info.fingerName = _T("SRF_Left_Little");
					info.capture_finger_idx = LEFT_LITTLE;
					info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER;
					info.NumberOfFinger =1;
					info.capture_finger_btn =15;
					m_vecCaptureSeq.push_back(info);
				}
			}
		}
		m_CaptureEnd =TRUE;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PostMessage( WM_USER_CAPTURE_SEQ_NEXT );

	return 0L;
}

LRESULT CIBScanUltimate_SalesDemoDlg::OnMsg_CaptureSeqNext(WPARAM wParam, LPARAM lParam)
{
	int nRc;
	int btnNow;
	if( m_nDevHandle == -1 )
		return 0L;

	m_bBlank = FALSE;
	m_FirstTimeToCapture = TRUE;
	memset(&m_FingerQuality[0], ENUM_IBSU_FINGER_NOT_PRESENT, sizeof(m_FingerQuality));

	m_nCurrentCaptureStep++;

	CColorButton *btnarr[18] = {&m_ScanFingerDlg.m_Button1, &m_ScanFingerDlg.m_Button2, 
			&m_ScanFingerDlg.m_Button3, &m_ScanFingerDlg.m_Button4, &m_ScanFingerDlg.m_Button5, 
			&m_ScanFingerDlg.m_Button6, &m_ScanFingerDlg.m_Button7, &m_ScanFingerDlg.m_Button8, 
			&m_ScanFingerDlg.m_Button9, &m_ScanFingerDlg.m_Button10,&m_ScanFingerDlg.m_Button11,
			&m_ScanFingerDlg.m_Button12, &m_ScanFingerDlg.m_Button13, &m_ScanFingerDlg.m_Button14,
			&m_ScanFingerDlg.m_Button15 ,&m_ScanFingerDlg.m_Button16,&m_ScanFingerDlg.m_Button17,&m_ScanFingerDlg.m_Button18};

	if( m_nCurrentCaptureStep >= (int)m_vecCaptureSeq.size() )
	{
		m_ScanFingerDlg.m_CaptureEnd = TRUE;
		memset(m_ShiftedFlag, 0, sizeof(m_ShiftedFlag));
	    _SetLEDs(m_nDevHandle, ENUM_IBSU_TYPE_NONE, NONE_FINGER, __LED_COLOR_NONE__, FALSE, m_ShiftedFlag);
		// All of capture sequence completely
		m_nCurrentCaptureStep = -1;

		int i;
		int btnArray;

		switch(m_MainDlg.m_CaptureMode)
		{
		case TWO_FINGER_FLAT_CAPTURE:
			btnArray =1;
			break;
		case TEN_FINGER_FLAT_THUMB:
			btnArray=2;
			break;
		case TEN_FINGER_FOUR_FOUR_TWO:
			btnArray =3;
			break;
		case TEN_FINGER_FLAT_THUMB_INDEX:
			btnArray=4;
			break;
		case TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH:
			btnArray =5;
			break;
		case TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH:
			btnArray=6;
			break;
		case TEN_FINGER_ONE_FLAT_CAPTURE:
		case TEN_FINGER_ONE_ROLL_CAPTURE:
			btnArray =10;
			break;
		case TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH:
			btnArray =13;
			break;
		case TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH:
		case TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE:		
			btnArray=15;
			break;
		case TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH:
		case TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN:
			btnArray =16;
			break;
		case TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL:
			btnArray =17;
			m_ScanFingerDlg.m_FingerDisplayManager->SelectFinger(NONE_FINGER);
			break;
		case TEN_FINGER_FOUR_ONE_FOUR_ONE:
			btnArray = 4;
			break;
		case TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH:
			btnArray = 14;
			break;
		default:
			break;
		}
		for(i=0; i<btnArray; i++)
		{
			btnarr[i]->SetColor(RGB(146,208,80), RGB(70,70,70));
		}
		m_ScanFingerDlg.m_FingerDisplayManager->SelectFinger(NONE_FINGER);

		OnMsg_UpdateDisplayResources();
		return 0L;
	}

	//For ResultImageEx
	for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
		IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);

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

	m_ScanFingerDlg.m_FingerDisplayManager->SelectFinger(NONE_FINGER);

	// Start capture
	DWORD captureOptions = 0;
	captureOptions |= IBSU_OPTION_AUTO_CONTRAST;

	if(m_OptionDlg.m_chkAutoCapture)
	{
		captureOptions |= IBSU_OPTION_AUTO_CAPTURE;
	}

	m_MainDlg.m_CaptureFinger = info.capture_finger_idx;
	m_MainDlg.m_CaptureBtnFinger =info.capture_finger_btn;

	for(btnNow=0; btnNow<m_MainDlg.m_CaptureBtnFinger; btnNow++)
	{
		btnarr[btnNow]->SetColor(RGB(146,208,80), RGB(70,70,70));
	}
	btnarr[m_MainDlg.m_CaptureBtnFinger-1]->SetColor(RGB(255,205,18), RGB(70,70,70));
	
	m_ScanFingerDlg.m_FingerDisplayManager->SelectFinger(m_MainDlg.m_CaptureFinger);

	IBSU_SetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_ENABLE_WET_FINGER_DETECT, "TRUE");
	IBSU_SetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_WET_FINGER_DETECT_LEVEL_THRESHOLD, "200 400 600 800 1000");
	CString str;
	str.Format("%d", m_OptionDlg.m_WetDLevel);
	IBSU_SetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_WET_FINGER_DETECT_LEVEL, str);

	nRc = IBSU_BeginCaptureImage(m_nDevHandle, info.ImageType, imgRes, captureOptions);

    _SetLEDs(m_nDevHandle, info.ImageType, m_MainDlg.m_CaptureFinger, __LED_COLOR_RED__, FALSE, m_ShiftedFlag);

	return 0L;
}

LRESULT CIBScanUltimate_SalesDemoDlg::OnMsg_DeviceCommunicationBreak(WPARAM wParam, LPARAM lParam)
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

LRESULT CIBScanUltimate_SalesDemoDlg::OnMsg_DrawClientWindow(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CIBScanUltimate_SalesDemoDlg::OnMsg_UpdateDeviceList(WPARAM wParam, LPARAM lParam)
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

LRESULT CIBScanUltimate_SalesDemoDlg::OnMsg_InitWarning(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CIBScanUltimate_SalesDemoDlg::OnMsg_UpdateDisplayResources(WPARAM wParam, LPARAM lParam)
{
	BOOL selectedDev = m_MainDlg.m_DeviceList.GetCurSel() > 0;
	BOOL captureSeq = m_MainDlg.m_SequenceList.GetCurSel() > 0;
	BOOL idle = !m_bInitializing && ( m_nCurrentCaptureStep == -1 );
	BOOL active = !m_bInitializing && (m_nCurrentCaptureStep != -1 );
	BOOL uninitializedDev = selectedDev && ( m_nDevHandle == -1 );

	m_MainDlg.m_DeviceList.EnableWindow( idle );  
	m_MainDlg.m_SequenceList.EnableWindow( selectedDev && idle );

	m_MainDlg.GetDlgItem( IDC_START_CAPTURE )->EnableWindow( captureSeq );
	m_MainDlg.GetDlgItem( IDC_EDIT_CAPTURE )->EnableWindow( captureSeq && !uninitializedDev && m_MainDlg.m_CaptureCompleted );
	m_MainDlg.GetDlgItem( IDC_SAVE_CAPTURE )->EnableWindow( captureSeq && !uninitializedDev && m_MainDlg.m_CaptureCompleted );


	return 0L;
}

LRESULT CIBScanUltimate_SalesDemoDlg::OnMsg_UpdateStatusMessage(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CIBScanUltimate_SalesDemoDlg::OnMsg_Beep(WPARAM wParam, LPARAM lParam)
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

LRESULT CIBScanUltimate_SalesDemoDlg::OnMsg_DrawFingerQuality(WPARAM wParam, LPARAM lParam)
{
	for( int i = 0; i < 4; i++ )
	{
		m_ScanFingerDlg.GetDlgItem(IDC_STATIC_QUALITY_1+i)->RedrawWindow();
	}

	return 0L;
}

void CIBScanUltimate_SalesDemoDlg::_CaptureStop()
{
	if( m_nDevHandle == -1 )
		return;

	IBSU_CancelCaptureImage( m_nDevHandle );
	m_nCurrentCaptureStep = -1;
	m_bNeedClearPlaten = FALSE;
	m_bBlank = FALSE;

	OnMsg_UpdateDisplayResources();
}

void CIBScanUltimate_SalesDemoDlg::OnClose()
{
	if(MessageBox("Do you want to exit this program?", "Exit", MB_YESNO | MB_ICONINFORMATION) == IDYES)
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
}

void CIBScanUltimate_SalesDemoDlg::OnOption()
{
	if(m_OptionDlg.GetSafeHwnd()==NULL)
	{
		m_OptionDlg.Create(IDD_Option, this);
		m_OptionDlg.m_pParent = this;
		m_OptionDlg.ShowWindow(SW_SHOW);
	}
	Invalidate();
}

void CIBScanUltimate_SalesDemoDlg::OnExit()
{
	if(MessageBox("Do you want to exit this program?", "Exit", MB_YESNO | MB_ICONINFORMATION) == IDYES)
		OnOK();
}

char* CIBScanUltimate_SalesDemoDlg::_GetwarningMessage(int warning_msg)
{
	switch(warning_msg)
	{
	case IBSU_WRN_ROLLING_SMEAR:
		return "Smear detected";
	case IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY:
		return "Smear detected ";
	case IBSU_WRN_ROLLING_SHIFTED_VERTICALLY:
		return "Smear detected ";
	case IBSU_WRN_ROLLING_SHIFTED_HORIZONTALLY|IBSU_WRN_ROLLING_SHIFTED_VERTICALLY:
		return "Smear detected ";
	case IBSU_WRN_WET_FINGERS:
		return "Wet finger detected";
	}
	return "";
}

int CIBScanUltimate_SalesDemoDlg::ErrorMsg(
	int m_ErrNfiq		,
	int m_ErrDuplicate ,
	int m_ErrPosition  ,
	int m_ErrImgStatus ,
	CString askMsgNFIQ,
	CString askMsgDuplicate,
	CString askMsgPosition,
	CString askMsgImgStatus)
{

	CString askMsg="";

	if(askMsgNFIQ.GetLength() > 3)
		askMsg += askMsgNFIQ + "\n";
	if(askMsgDuplicate.GetLength() > 3)
		askMsg += askMsgDuplicate + "\n";
	if(askMsgPosition.GetLength() > 3)
		askMsg += askMsgPosition + "\n";
	if(askMsgImgStatus.GetLength() > 3)
		askMsg += askMsgImgStatus + "\n";

	if(askMsg.GetLength() > 3)
		askMsg += "\nDo you want a Recapture?";

	if(m_ErrDuplicate == 0 || m_ErrPosition == 0 || m_ErrImgStatus ==0 || m_ErrNfiq ==0)
	{

		if(MessageBox(askMsg,"CBP Demo",MB_YESNO | MB_ICONINFORMATION) == IDYES)
		{		
			m_nCurrentCaptureStep--;
			return 0;

		}
		goto error_code;
	}
error_code:
	return 1;
}

HBRUSH CIBScanUltimate_SalesDemoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  여기서 DC의 특성을 변경합니다.
	hbr =(HBRUSH)m_background;

	// TODO:  기본값이 적당하지 않으면 다른 브러시를 반환합니다.
	return hbr;
}

void CIBScanUltimate_SalesDemoDlg::_ClearLEDs_forCBP(int deviceHandle)
{
	DWORD setLEDs = IBSU_LED_NONE;

	IBSU_SetLEDs(deviceHandle, setLEDs);
}

void CIBScanUltimate_SalesDemoDlg::_SetLEDs(int deviceHandle, IBSU_ImageType imageType, int finger_index, int ledColor, 
											BOOL bBlink, int ShiftedFlag[4], int finger_count)
{
	DWORD setLEDs = IBSU_LED_NONE;

	if(ShiftedFlag[0] > 0 || ShiftedFlag[1] > 0 || ShiftedFlag[2] > 0 || ShiftedFlag[3] > 0)
	{
		if(imageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_ROLL;
	    }

		DWORD LEDlist[4] = {IBSU_LED_NONE, IBSU_LED_NONE, IBSU_LED_NONE, IBSU_LED_NONE};
		unsigned char si[4] = {-1,-1,-1,-1}, idx=0;
		for(int i=0; i<4; i++)
		{
			if(ShiftedFlag[i] > 0)
			{
				si[idx++] = i;
			}
		}

		if((m_AddBlinkIndex++) % 2 == 0)
		{
			if(finger_index == LEFT_HAND)
			{
				setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
				LEDlist[0] = (ShiftedFlag[0] == 0x01 || ShiftedFlag[0] == 0x03) ? IBSU_LED_F_LEFT_LITTLE_RED :
							 (ShiftedFlag[0] == 0x02 || ShiftedFlag[0] == 0xFF) ? (IBSU_LED_F_LEFT_LITTLE_RED | IBSU_LED_F_LEFT_LITTLE_GREEN) : IBSU_LED_NONE;
				LEDlist[1] = (ShiftedFlag[1] == 0x01 || ShiftedFlag[1] == 0x03) ? IBSU_LED_F_LEFT_RING_RED :
							 (ShiftedFlag[1] == 0x02 || ShiftedFlag[1] == 0xFF) ? (IBSU_LED_F_LEFT_RING_RED | IBSU_LED_F_LEFT_RING_GREEN) : IBSU_LED_NONE;
				LEDlist[2] = (ShiftedFlag[2] == 0x01 || ShiftedFlag[2] == 0x03) ? IBSU_LED_F_LEFT_MIDDLE_RED :
							 (ShiftedFlag[2] == 0x02 || ShiftedFlag[2] == 0xFF) ? (IBSU_LED_F_LEFT_MIDDLE_RED | IBSU_LED_F_LEFT_MIDDLE_GREEN) : IBSU_LED_NONE;
				LEDlist[3] = (ShiftedFlag[3] == 0x01 || ShiftedFlag[3] == 0x03) ? IBSU_LED_F_LEFT_INDEX_RED :
							 (ShiftedFlag[3] == 0x02 || ShiftedFlag[3] == 0xFF) ? (IBSU_LED_F_LEFT_INDEX_RED | IBSU_LED_F_LEFT_INDEX_GREEN) : IBSU_LED_NONE;
			}
			else if(finger_index == RIGHT_HAND)
			{
				setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
				LEDlist[0] = (ShiftedFlag[0] == 0x01 || ShiftedFlag[0] == 0x03) ? IBSU_LED_F_RIGHT_INDEX_RED :
							 (ShiftedFlag[0] == 0x02 || ShiftedFlag[0] == 0xFF) ? (IBSU_LED_F_RIGHT_INDEX_RED | IBSU_LED_F_RIGHT_INDEX_GREEN) : IBSU_LED_NONE;
				LEDlist[1] = (ShiftedFlag[1] == 0x01 || ShiftedFlag[1] == 0x03) ? IBSU_LED_F_RIGHT_MIDDLE_RED :
							 (ShiftedFlag[1] == 0x02 || ShiftedFlag[1] == 0xFF) ? (IBSU_LED_F_RIGHT_MIDDLE_RED | IBSU_LED_F_RIGHT_MIDDLE_GREEN) : IBSU_LED_NONE;
				LEDlist[2] = (ShiftedFlag[2] == 0x01 || ShiftedFlag[2] == 0x03) ? IBSU_LED_F_RIGHT_RING_RED :
							 (ShiftedFlag[2] == 0x02 || ShiftedFlag[2] == 0xFF) ? (IBSU_LED_F_RIGHT_RING_RED | IBSU_LED_F_RIGHT_RING_GREEN) : IBSU_LED_NONE;
				LEDlist[3] = (ShiftedFlag[3] == 0x01 || ShiftedFlag[3] == 0x03) ? IBSU_LED_F_RIGHT_LITTLE_RED :
							 (ShiftedFlag[3] == 0x02 || ShiftedFlag[3] == 0xFF) ? (IBSU_LED_F_RIGHT_LITTLE_RED | IBSU_LED_F_RIGHT_LITTLE_GREEN) : IBSU_LED_NONE;
			}
			else if(finger_index == BOTH_THUMBS)
			{
				setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
				if(finger_count >= 2)
				{
					if(si[0] != -1)
					{
						LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_LEFT_THUMB_RED :
									 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE;
					}
					if(si[1] != -1)
					{
						LEDlist[1] = (ShiftedFlag[si[1]] == 0x01 || ShiftedFlag[si[1]] == 0x03) ? IBSU_LED_F_RIGHT_THUMB_RED :
									 (ShiftedFlag[si[1]] == 0x02 || ShiftedFlag[si[1]] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE;
					}
				}
				else if(finger_count >= 1)
				{
					if(si[0] != -1)
					{
						LEDlist[0] |= (ShiftedFlag[0] == 0x01 || ShiftedFlag[0] == 0x03) ? IBSU_LED_F_LEFT_THUMB_RED :
									  (ShiftedFlag[0] == 0x02 || ShiftedFlag[0] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE;
						LEDlist[0] |= (ShiftedFlag[1] == 0x01 || ShiftedFlag[1] == 0x03) ? IBSU_LED_F_LEFT_THUMB_RED :
									  (ShiftedFlag[1] == 0x02 || ShiftedFlag[1] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE;
					}
					if(si[1] != -1)
					{
						LEDlist[1] |= (ShiftedFlag[2] == 0x01 || ShiftedFlag[2] == 0x03) ? IBSU_LED_F_RIGHT_THUMB_RED :
									  (ShiftedFlag[2] == 0x02 || ShiftedFlag[2] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE;
						LEDlist[1] |= (ShiftedFlag[3] == 0x01 || ShiftedFlag[3] == 0x03) ? IBSU_LED_F_RIGHT_THUMB_RED :
									  (ShiftedFlag[3] == 0x02 || ShiftedFlag[3] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE;
					}
				}
				else
				{
					LEDlist[0] = (ShiftedFlag[0] == 0xFF || ShiftedFlag[1] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE;
					LEDlist[1] = (ShiftedFlag[2] == 0xFF || ShiftedFlag[3] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE;
				}
			}
			else if(finger_index >= LEFT_LITTLE && finger_index <= RIGHT_LITTLE)
			{
				switch(finger_index)
				{
				case LEFT_LITTLE: 	
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_LEFT_LITTLE_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_LITTLE_RED | IBSU_LED_F_LEFT_LITTLE_GREEN) : IBSU_LED_NONE; break;
				case LEFT_RING:
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_LEFT_RING_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_RING_RED | IBSU_LED_F_LEFT_RING_GREEN) : IBSU_LED_NONE; break;
				case LEFT_MIDDLE:
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_LEFT_MIDDLE_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_MIDDLE_RED | IBSU_LED_F_LEFT_MIDDLE_GREEN) : IBSU_LED_NONE; break;
				case LEFT_INDEX:
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_LEFT_INDEX_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_INDEX_RED | IBSU_LED_F_LEFT_INDEX_GREEN) : IBSU_LED_NONE; break;
				case LEFT_THUMB:
					setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_LEFT_THUMB_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_THUMB:
					setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_RIGHT_THUMB_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_INDEX:
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_RIGHT_INDEX_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_INDEX_RED | IBSU_LED_F_RIGHT_INDEX_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_MIDDLE:
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_RIGHT_MIDDLE_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_MIDDLE_RED | IBSU_LED_F_RIGHT_MIDDLE_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_RING:
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_RIGHT_RING_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_RING_RED | IBSU_LED_F_RIGHT_RING_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_LITTLE:				
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0x01 || ShiftedFlag[si[0]] == 0x03) ? IBSU_LED_F_RIGHT_LITTLE_RED :
								 (ShiftedFlag[si[0]] == 0x02 || ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_LITTLE_RED | IBSU_LED_F_RIGHT_LITTLE_GREEN) : IBSU_LED_NONE; break;
				}
			}
		}
		else
		{
			if(finger_index == LEFT_HAND)
			{
				setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
				LEDlist[0] = (ShiftedFlag[0] == 0xFF) ? (IBSU_LED_F_LEFT_LITTLE_RED | IBSU_LED_F_LEFT_LITTLE_GREEN) : IBSU_LED_NONE;
				LEDlist[1] = (ShiftedFlag[1] == 0xFF) ? (IBSU_LED_F_LEFT_RING_RED | IBSU_LED_F_LEFT_RING_GREEN) : IBSU_LED_NONE;
				LEDlist[2] = (ShiftedFlag[2] == 0xFF) ? (IBSU_LED_F_LEFT_MIDDLE_RED | IBSU_LED_F_LEFT_MIDDLE_GREEN) : IBSU_LED_NONE;
				LEDlist[3] = (ShiftedFlag[3] == 0xFF) ? (IBSU_LED_F_LEFT_INDEX_RED | IBSU_LED_F_LEFT_INDEX_GREEN) : IBSU_LED_NONE;
			}
			else if(finger_index == RIGHT_HAND)
			{
				setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
				LEDlist[0] = (ShiftedFlag[0] == 0xFF) ? (IBSU_LED_F_RIGHT_INDEX_RED | IBSU_LED_F_RIGHT_INDEX_GREEN) : IBSU_LED_NONE;
				LEDlist[1] = (ShiftedFlag[1] == 0xFF) ? (IBSU_LED_F_RIGHT_MIDDLE_RED | IBSU_LED_F_RIGHT_MIDDLE_GREEN) : IBSU_LED_NONE;
				LEDlist[2] = (ShiftedFlag[2] == 0xFF) ? (IBSU_LED_F_RIGHT_RING_RED | IBSU_LED_F_RIGHT_RING_GREEN) : IBSU_LED_NONE;
				LEDlist[3] = (ShiftedFlag[3] == 0xFF) ? (IBSU_LED_F_RIGHT_LITTLE_RED | IBSU_LED_F_RIGHT_LITTLE_GREEN) : IBSU_LED_NONE;
			}
			else if(finger_index == BOTH_THUMBS)
			{
				setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
				if(finger_count >= 2)
				{
					if(si[0] != -1)
					{
						LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE;
					}
					if(si[1] != -1)
					{
						LEDlist[1] = (ShiftedFlag[si[1]] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE;
					}
				}
				else
				{
					LEDlist[0] = (ShiftedFlag[0] == 0xFF || ShiftedFlag[1] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE;
					LEDlist[1] = (ShiftedFlag[2] == 0xFF || ShiftedFlag[3] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE;
				}
			}
			else if(finger_index >= LEFT_LITTLE && finger_index <= RIGHT_LITTLE)
			{
				switch(finger_index)
				{
				case LEFT_LITTLE: 	
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_LITTLE_RED | IBSU_LED_F_LEFT_LITTLE_GREEN) : IBSU_LED_NONE; break;
				case LEFT_RING:
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_RING_RED | IBSU_LED_F_LEFT_RING_GREEN) : IBSU_LED_NONE; break;
				case LEFT_MIDDLE:
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_MIDDLE_RED | IBSU_LED_F_LEFT_MIDDLE_GREEN) : IBSU_LED_NONE; break;
				case LEFT_INDEX:
					setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_INDEX_RED | IBSU_LED_F_LEFT_INDEX_GREEN) : IBSU_LED_NONE; break;
				case LEFT_THUMB:
					setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_LEFT_THUMB_RED | IBSU_LED_F_LEFT_THUMB_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_THUMB:
					setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_THUMB_RED | IBSU_LED_F_RIGHT_THUMB_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_INDEX:
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_INDEX_RED | IBSU_LED_F_RIGHT_INDEX_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_MIDDLE:
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_MIDDLE_RED | IBSU_LED_F_RIGHT_MIDDLE_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_RING:
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_RING_RED | IBSU_LED_F_RIGHT_RING_GREEN) : IBSU_LED_NONE; break;
				case RIGHT_LITTLE:				
					setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
					LEDlist[0] = (ShiftedFlag[si[0]] == 0xFF) ? (IBSU_LED_F_RIGHT_LITTLE_RED | IBSU_LED_F_RIGHT_LITTLE_GREEN) : IBSU_LED_NONE; break;
				}
			}
		}

		for(int i=0; i<4; i++)
			setLEDs |= LEDlist[i];
		IBSU_SetLEDs(deviceHandle, setLEDs);

		return;
	}

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

	    if(imageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_ROLL;
	    }

	    if(finger_index == RIGHT_THUMB || finger_index == BOTH_RIGHT_THUMB)
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
	    else if(finger_index == LEFT_THUMB || finger_index == BOTH_LEFT_THUMB)
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
	    else if(finger_index == BOTH_THUMBS)
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
				if(finger_count > 1)
				{
					setLEDs |= IBSU_LED_F_RIGHT_THUMB_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_THUMB_RED;
					setLEDs |= IBSU_LED_F_LEFT_THUMB_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_THUMB_RED;
				}
			    else if(finger_count > 0)
				{
					setLEDs |= IBSU_LED_F_RIGHT_THUMB_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_THUMB_RED;
					setLEDs |= IBSU_LED_F_LEFT_THUMB_RED;
				}
				else
				{
					setLEDs |= IBSU_LED_F_LEFT_THUMB_RED;
					setLEDs |= IBSU_LED_F_RIGHT_THUMB_RED;
				}
		    }
	    }
	    ///////////////////LEFT HAND////////////////////
	    else if(finger_index == LEFT_INDEX)
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
	    else if(finger_index == LEFT_MIDDLE)
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
	    else if(finger_index == LEFT_INDEX_MIDDLE)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
			    setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    if(finger_count > 1)
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
				}
			    else if(finger_count > 0)
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
				}
				else
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
				}
		    }
	    }
	    else if(finger_index == LEFT_RING)
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
	    else if(finger_index == LEFT_LITTLE)
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
	    else if(finger_index == LEFT_RING_LITTLE)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_LEFT_RING_RED;
			    setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    if(finger_count > 1)
				{
					setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
			    else if(finger_count > 0)
				{
					setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
				else
				{
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
		    }
	    }
	    else if(finger_index == LEFT_HAND)
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
			    if(finger_count > 3)
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
				else if(finger_count > 2)
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_LEFT_RING_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
			    else if(finger_count > 1)
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
			    else if(finger_count > 0)
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
				else
				{
					setLEDs |= IBSU_LED_F_LEFT_INDEX_RED;
					setLEDs |= IBSU_LED_F_LEFT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_LEFT_RING_RED;
					setLEDs |= IBSU_LED_F_LEFT_LITTLE_RED;
				}
		    }
	    }
	    ///////////RIGHT HAND /////////////////////////
	    else if(finger_index == RIGHT_INDEX)
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
	    else if(finger_index == RIGHT_MIDDLE)
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
	    else if(finger_index == RIGHT_INDEX_MIDDLE)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    if(finger_count > 1)
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
				}
			    else if(finger_count > 0)
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
				}
				else
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
				}
		    }
	    }
	    else if(finger_index == RIGHT_RING)
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
	    else if(finger_index == RIGHT_LITTLE)
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
	    else if(finger_index == RIGHT_RING_LITTLE)
	    {
		    setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND;
		    if( ledColor == __LED_COLOR_GREEN__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_GREEN;
		    }
		    else if( ledColor == __LED_COLOR_RED__ )
		    {
			    setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
			    setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
		    }
		    else if( ledColor == __LED_COLOR_YELLOW__ )
		    {
			    if(finger_count > 1)
				{
					setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
			    else if(finger_count > 0)
				{
					setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
				else
				{
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
		    }
	    }
	    else if(finger_index == RIGHT_HAND)
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
			    if(finger_count > 3)
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
				else if(finger_count > 2)
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_RIGHT_RING_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
				else if(finger_count > 1)
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
				else if(finger_count > 0)
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_GREEN;
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
				else
				{
					setLEDs |= IBSU_LED_F_RIGHT_INDEX_RED;
					setLEDs |= IBSU_LED_F_RIGHT_MIDDLE_RED;
					setLEDs |= IBSU_LED_F_RIGHT_RING_RED;
					setLEDs |= IBSU_LED_F_RIGHT_LITTLE_RED;
				}
		    }
	    }

        IBSU_SetLEDs(deviceHandle, setLEDs);
    }
}

void CIBScanUltimate_SalesDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 305)
	{
		DWORD setLEDs = IBSU_LED_NONE;

		if( m_SmearFlag == TRUE || m_WrongPlaceFlag == TRUE || m_WetFingerFlag == TRUE )
		{
			if(m_SmearFlag == TRUE)
			{
				setLEDs = IBSU_LED_F_PROGRESS_ROLL;

				if((m_AddBlinkIndex++) % 2 == 0)
				{
					switch(m_SmearFingerIndex)
					{
					case RIGHT_THUMB: setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB | IBSU_LED_F_RIGHT_THUMB_GREEN; break;
					case RIGHT_INDEX: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_INDEX_GREEN; break;
					case RIGHT_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_MIDDLE_GREEN; break;
					case RIGHT_RING: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_RING_GREEN; break;
					case RIGHT_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_LITTLE_GREEN; break;
					case LEFT_THUMB: setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB | IBSU_LED_F_LEFT_THUMB_GREEN; break;
					case LEFT_INDEX: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_INDEX_GREEN; break;
					case LEFT_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_MIDDLE_GREEN; break;
					case LEFT_RING: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_RING_GREEN; break;
					case LEFT_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_LITTLE_GREEN; break;
					}
				}
				else
				{
					switch(m_SmearFingerIndex)
					{
					case RIGHT_THUMB: setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB; break;
					case RIGHT_INDEX: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case RIGHT_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case RIGHT_RING: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case RIGHT_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case LEFT_THUMB: setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB; break;
					case LEFT_INDEX: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case LEFT_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case LEFT_RING: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case LEFT_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					}
				}
			}
			else if(m_WrongPlaceFlag == TRUE)
			{
				if((m_AddBlinkIndex++) % 2 == 0)
				{
					switch(m_WrongPlaceFingerIndex)
					{
					case RIGHT_THUMB: setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB | IBSU_LED_F_RIGHT_THUMB_GREEN; break;
					case RIGHT_INDEX: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_INDEX_GREEN; break;
					case RIGHT_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_MIDDLE_GREEN; break;
					case RIGHT_RING: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_RING_GREEN; break;
					case RIGHT_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_LITTLE_GREEN; break;
					case LEFT_THUMB: setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB | IBSU_LED_F_LEFT_THUMB_GREEN; break;
					case LEFT_INDEX: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_INDEX_GREEN; break;
					case LEFT_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_MIDDLE_GREEN; break;
					case LEFT_RING: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_RING_GREEN; break;
					case LEFT_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_LITTLE_GREEN; break;
					case LEFT_RING_LITTLE: setLEDs |= (IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_RING_GREEN | IBSU_LED_F_LEFT_LITTLE_GREEN); break;
					case LEFT_INDEX_MIDDLE: setLEDs |= (IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_INDEX_GREEN | IBSU_LED_F_LEFT_MIDDLE_GREEN); break;
					case RIGHT_INDEX_MIDDLE: setLEDs |= (IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_INDEX_GREEN | IBSU_LED_F_RIGHT_MIDDLE_GREEN); break;
					case RIGHT_RING_LITTLE: setLEDs |= (IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_RING_GREEN | IBSU_LED_F_RIGHT_LITTLE_GREEN); break;
					case BOTH_THUMBS: setLEDs |= (IBSU_LED_F_PROGRESS_TWO_THUMB | IBSU_LED_F_RIGHT_THUMB_GREEN | IBSU_LED_F_LEFT_THUMB_GREEN); break;
					case LEFT_HAND: setLEDs |= (IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_INDEX_GREEN | IBSU_LED_F_LEFT_MIDDLE_GREEN |
												IBSU_LED_F_LEFT_RING_GREEN | IBSU_LED_F_LEFT_LITTLE_GREEN); break;
					case RIGHT_HAND: setLEDs |= (IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_LITTLE_GREEN | IBSU_LED_F_RIGHT_RING_GREEN |
												IBSU_LED_F_RIGHT_MIDDLE_GREEN | IBSU_LED_F_RIGHT_INDEX_GREEN); break;
					}
				}
				else
				{
					switch(m_WrongPlaceFingerIndex)
					{
					case RIGHT_THUMB: setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB; break;
					case RIGHT_INDEX: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case RIGHT_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case RIGHT_RING: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case RIGHT_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case LEFT_THUMB: setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB; break;
					case LEFT_INDEX: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case LEFT_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case LEFT_RING: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case LEFT_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case LEFT_RING_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case LEFT_INDEX_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case RIGHT_INDEX_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case RIGHT_RING_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case BOTH_THUMBS: setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB; break;
					case LEFT_HAND: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case RIGHT_HAND: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					}
				}
			}
			else if(m_WetFingerFlag == TRUE)
			{
				if((m_AddBlinkIndex++) % 2 == 0)
				{
					switch(m_WetFingerIndex)
					{
					case RIGHT_THUMB: setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB | IBSU_LED_F_RIGHT_THUMB_GREEN; break;
					case RIGHT_INDEX: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_INDEX_GREEN; break;
					case RIGHT_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_MIDDLE_GREEN; break;
					case RIGHT_RING: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_RING_GREEN; break;
					case RIGHT_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_LITTLE_GREEN; break;
					case LEFT_THUMB: setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB | IBSU_LED_F_LEFT_THUMB_GREEN; break;
					case LEFT_INDEX: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_INDEX_GREEN; break;
					case LEFT_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_MIDDLE_GREEN; break;
					case LEFT_RING: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_RING_GREEN; break;
					case LEFT_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_LITTLE_GREEN; break;
					case LEFT_RING_LITTLE: setLEDs |= (IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_RING_GREEN | IBSU_LED_F_LEFT_LITTLE_GREEN); break;
					case LEFT_INDEX_MIDDLE: setLEDs |= (IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_INDEX_GREEN | IBSU_LED_F_LEFT_MIDDLE_GREEN); break;
					case RIGHT_INDEX_MIDDLE: setLEDs |= (IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_INDEX_GREEN | IBSU_LED_F_RIGHT_MIDDLE_GREEN); break;
					case RIGHT_RING_LITTLE: setLEDs |= (IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_RING_GREEN | IBSU_LED_F_RIGHT_LITTLE_GREEN); break;
					case BOTH_THUMBS: setLEDs |= (IBSU_LED_F_PROGRESS_TWO_THUMB | IBSU_LED_F_RIGHT_THUMB_GREEN | IBSU_LED_F_LEFT_THUMB_GREEN); break;
					case LEFT_HAND: setLEDs |= (IBSU_LED_F_PROGRESS_LEFT_HAND | IBSU_LED_F_LEFT_INDEX_GREEN | IBSU_LED_F_LEFT_MIDDLE_GREEN |
												IBSU_LED_F_LEFT_RING_GREEN | IBSU_LED_F_LEFT_LITTLE_GREEN); break;
					case RIGHT_HAND: setLEDs |= (IBSU_LED_F_PROGRESS_RIGHT_HAND | IBSU_LED_F_RIGHT_LITTLE_GREEN | IBSU_LED_F_RIGHT_RING_GREEN |
												IBSU_LED_F_RIGHT_MIDDLE_GREEN | IBSU_LED_F_RIGHT_INDEX_GREEN); break;
					}
				}
				else
				{
					switch(m_WetFingerIndex)
					{
					case RIGHT_THUMB: setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB; break;
					case RIGHT_INDEX: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case RIGHT_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case RIGHT_RING: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case RIGHT_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case LEFT_THUMB: setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB; break;
					case LEFT_INDEX: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case LEFT_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case LEFT_RING: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case LEFT_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case LEFT_RING_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case LEFT_INDEX_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case RIGHT_INDEX_MIDDLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case RIGHT_RING_LITTLE: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					case BOTH_THUMBS: setLEDs |= IBSU_LED_F_PROGRESS_TWO_THUMB; break;
					case LEFT_HAND: setLEDs |= IBSU_LED_F_PROGRESS_LEFT_HAND; break;
					case RIGHT_HAND: setLEDs |= IBSU_LED_F_PROGRESS_RIGHT_HAND; break;
					}
				}
			}
			IBSU_SetLEDs(m_nDevHandle, setLEDs);

			return;
		}
	}

    CDialog::OnTimer(nIDEvent);
}
