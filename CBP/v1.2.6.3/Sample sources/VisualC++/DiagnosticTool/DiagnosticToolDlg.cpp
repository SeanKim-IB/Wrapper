
// DiagnosticToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DiagnosticTool.h"
#include "DiagnosticToolDlg.h"
#include <io.h>		//access

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDiagnosticToolDlg *m_app;

// CDiagnosticToolDlg dialog
const CBrush   CDiagnosticToolDlg::s_background_brush( RGB(59,56,56) );

CRITICAL_SECTION g_CriticalSection;

CDiagnosticToolDlg::CDiagnosticToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDiagnosticToolDlg::IDD, pParent)
	, m_strIBSUVer(_T(""))
	, m_strWrapperVer(_T(""))
	, m_isRunTests(FALSE)
	, m_nDevHandle(-1)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	InitializeCriticalSection(&g_CriticalSection);

	m_ReportDlg = new CReportDlg();
	m_ReportDlg->Create(IDD_DIALOG_REPORT);
	m_ReportDlg->ShowWindow(SW_HIDE);

	m_app = this;
}

CDiagnosticToolDlg::~CDiagnosticToolDlg()
{
	DeleteCriticalSection(&g_CriticalSection);
}

void CDiagnosticToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDiagnosticToolDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	//ON_MESSAGE(WM_USER_CAPTURE_SEQ_START,			OnMsg_CaptureSeqStart)
	//ON_MESSAGE(WM_USER_CAPTURE_SEQ_NEXT,			OnMsg_CaptureSeqNext)
	ON_MESSAGE(WM_USER_DEVICE_COMMUNICATION_BREAK,	OnMsg_DeviceCommunicationBreak)
	//ON_MESSAGE(WM_USER_DRAW_CLIENT_WINDOW,			OnMsg_DrawClientWindow)
	ON_MESSAGE(WM_USER_UPDATE_DEVICE_LIST,			OnMsg_UpdateDeviceList)
	ON_MESSAGE(WM_USER_INIT_WARNING,				OnMsg_InitWarning)
	ON_MESSAGE(WM_USER_UPDATE_DISPLAY_RESOURCES,	OnMsg_UpdateDisplayResources)
	//ON_MESSAGE(WM_USER_UPDATE_STATUS_MESSAGE,		OnMsg_UpdateStatusMessage)
	ON_MESSAGE(WM_USER_BEEP,						OnMsg_Beep)
	//ON_MESSAGE(WM_USER_DRAW_FINGER_QUALITY,			OnMsg_DrawFingerQuality)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_RUN_TESTS, &CDiagnosticToolDlg::OnBnClickedButtonRunTests)
END_MESSAGE_MAP()


// CDiagnosticToolDlg message handlers

BOOL CDiagnosticToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	// Set background image
	m_BitmapBGND.LoadBitmap(IDB_BITMAP_BGND);
	m_BitmapPASS.LoadBitmap(IDB_BITMAP_PASS);
	m_BitmapFAIL.LoadBitmap(IDB_BITMAP_FAIL);
	m_BitmapEXE.LoadBitmap(IDB_BITMAP_EXE);

	// register callbacks.
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, OnEvent_DeviceCount, this );
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, OnEvent_InitProgress, this );
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE, OnEvent_AsyncOpenDevice, this );

	// Get version of this app
	TCHAR MyName[MAX_PATH];
	GetModuleFileName(AfxGetStaticModuleState()->m_hCurrentInstanceHandle, MyName, MAX_PATH);
	CString FileVersion=_T("");
    DWORD infoSize = 0;
	char *buffer;

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

					FileVersion.Format("Kojak Diagnostic Tool ( Version %d.%d.%d.%d )", majorVer, minorVer, buildNum, revisionNum);
				}
			}
			delete[] buffer;
		}
	}
	
	if(FileVersion.GetLength()==0)
		FileVersion.Format("Kojak Diagnostic Tool ( can't read version )");

	SetWindowText(FileVersion);

	// Get version of this app
	GetCurrentDirectory(MAX_PATH, MyName);
	CString ibsu_filename;
	if(MyName[strlen(MyName)-1] == '\\')
		ibsu_filename.Format("%sIBScanUltimate.dll", MyName);
	else
		ibsu_filename.Format("%s\\IBScanUltimate.dll", MyName);
	sprintf(MyName, "%s", ibsu_filename.GetBuffer());

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

					m_strIBSUVer.Format("%d.%d.%d.%d", majorVer, minorVer, buildNum, revisionNum);
				}
			}
			delete[] buffer;
		}
	}
	
	if(m_strIBSUVer.GetLength()==0)
		m_strIBSUVer.Format("0.0.0.0");

	// get version of cbp wrapper
	GetCurrentDirectory(MAX_PATH, MyName);
	CString wrapper_filename;
	if(MyName[strlen(MyName)-1] == '\\')
		wrapper_filename.Format("%sCBP_FP_Wrapper.dll", MyName);
	else
		wrapper_filename.Format("%s\\CBP_FP_Wrapper.dll", MyName);
	sprintf(MyName, "%s", wrapper_filename.GetBuffer());

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

					m_strWrapperVer.Format("%d.%d.%d.%d", majorVer, minorVer, buildNum, revisionNum);
				}
			}
			delete[] buffer;
		}
	}
	
	if(m_strWrapperVer.GetLength()==0)
		m_strWrapperVer.Format("0.0.0.0");
	
	///////////////////////////////////////////////////////////////////////////////////////////
	// add callbacks
	m_callbacklist.cbp_fp_onCalibrate = &OnEvent_cbp_fp_onCalibrate;
	m_callbacklist.cbp_fp_onCapture = &OnEvent_cbp_fp_onCapture;
	m_callbacklist.cbp_fp_onClose = &OnEvent_cbp_fp_onClose;
	m_callbacklist.cbp_fp_onConfigure = &OnEvent_cbp_fp_onConfigure;
	m_callbacklist.cbp_fp_onEnumDevices = &OnEvent_cbp_fp_onEnumDevices;
	m_callbacklist.cbp_fp_onError = &OnEvent_cbp_fp_onError;
	m_callbacklist.cbp_fp_onGetDirtiness = &OnEvent_cbp_fp_onGetDirtiness;
	m_callbacklist.cbp_fp_onGetLockInfo = &OnEvent_cbp_fp_onGetLockInfo;
	m_callbacklist.cbp_fp_onInitialize = &OnEvent_cbp_fp_onInitialize;
	m_callbacklist.cbp_fp_onLock = &OnEvent_cbp_fp_onLock;
	m_callbacklist.cbp_fp_onOpen = &OnEvent_cbp_fp_onOpen;
	m_callbacklist.cbp_fp_onPowerSave = &OnEvent_cbp_fp_onPowerSave;
	m_callbacklist.cbp_fp_onPreview = &OnEvent_cbp_fp_onPreview;
	m_callbacklist.cbp_fp_onPreviewAnalysis = &OnEvent_cbp_fp_onPreviewAnalysis;
	m_callbacklist.cbp_fp_onStopPreview = &OnEvent_cbp_fp_onStopPreview;
	m_callbacklist.cbp_fp_onUninitialize = &OnEvent_cbp_fp_onUninitialize;
	m_callbacklist.cbp_fp_onUnLock = &OnEvent_cbp_fp_onUnLock;
	m_callbacklist.cbp_fp_onWarning = &OnEvent_cbp_fp_onWarning;
	///////////////////////////////////////////////////////////////////////////////////////////

	PostMessage(WM_USER_UPDATE_DEVICE_LIST);

	m_TestInfo.TestCount = 5;
	for(int i=0; i<m_TestInfo.TestCount; i++)
	{
		m_TestInfo.nID = IDC_STATIC_TEST_1_PASS;
		m_TestInfo.TestProgress[i] = TEST_RESULT_NONE;
	}

	_Init_Layout();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDiagnosticToolDlg::_Init_Layout()
{
	WINDOWPLACEMENT place;
	GetWindowPlacement(&place);

	SetWindowPos(NULL, 0, 0, 976, 367, SWP_NOMOVE);

	CWnd *pWnd;
	
	pWnd = (CWnd*)GetDlgItem(IDC_EDIT_DEVICE_INFO);
	place.rcNormalPosition.left = 0;
	place.rcNormalPosition.top = 25;
	place.rcNormalPosition.right = 976;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+30;
	pWnd->SetWindowPlacement(&place);

	pWnd = (CWnd*)GetDlgItem(IDC_BUTTON_RUN_TESTS);
	place.rcNormalPosition.left = place.rcNormalPosition.right-220;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom + 15;
	place.rcNormalPosition.right = place.rcNormalPosition.left+190;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+50;
	pWnd->SetWindowPlacement(&place);
	//*/

	/*pWnd = (CWnd*)GetDlgItem(IDC_EDIT_DEVICE_INFO);
	place.rcNormalPosition.left = 0;
	place.rcNormalPosition.top = 40;
	place.rcNormalPosition.right = 976;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+30;
	pWnd->SetWindowPlacement(&place);

	pWnd = (CWnd*)GetDlgItem(IDC_BUTTON_RUN_TESTS);
	place.rcNormalPosition.left = 25;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+40;
	place.rcNormalPosition.right = place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+50;
	pWnd->SetWindowPlacement(&place);//*/

	int nIDs_PASS[5] = {IDC_STATIC_TEST_1_PASS, IDC_STATIC_TEST_2_PASS, 
					IDC_STATIC_TEST_3_PASS, IDC_STATIC_TEST_4_PASS, IDC_STATIC_TEST_5_PASS};
	int nIDs_FAIL[5] = {IDC_STATIC_TEST_1_FAIL, IDC_STATIC_TEST_2_FAIL, 
					IDC_STATIC_TEST_3_FAIL, IDC_STATIC_TEST_4_FAIL, IDC_STATIC_TEST_5_FAIL};
	int signal_width = 130, signal_height = 58;
	int sx = 210, sy=191, gap_x=19, gap_y=14;

	for(int i=0; i<5; i++)
	{
		pWnd = (CWnd*)GetDlgItem(nIDs_PASS[i]);
		place.rcNormalPosition.left = sx + (signal_width+gap_x)*i;
		place.rcNormalPosition.top = sy;
		place.rcNormalPosition.right = place.rcNormalPosition.left+signal_width;
		place.rcNormalPosition.bottom = place.rcNormalPosition.top+signal_height;
		pWnd->SetWindowPlacement(&place);

		pWnd = (CWnd*)GetDlgItem(nIDs_FAIL[i]);
		place.rcNormalPosition.top = sy + signal_height+gap_y;
		place.rcNormalPosition.bottom = place.rcNormalPosition.top+signal_height;
		pWnd->SetWindowPlacement(&place);
	}

	_UpdateTestResult();
}

void CDiagnosticToolDlg::_UpdateTestResult()
{
	int nIDs_PASS[5] = {IDC_STATIC_TEST_1_PASS, IDC_STATIC_TEST_2_PASS, 
					IDC_STATIC_TEST_3_PASS, IDC_STATIC_TEST_4_PASS, IDC_STATIC_TEST_5_PASS};
	int nIDs_FAIL[5] = {IDC_STATIC_TEST_1_FAIL, IDC_STATIC_TEST_2_FAIL, 
					IDC_STATIC_TEST_3_FAIL, IDC_STATIC_TEST_4_FAIL, IDC_STATIC_TEST_5_FAIL};
	CWnd* pWnd_PASS, *pWnd_FAIL;

	for(int i=0; i<m_TestInfo.TestCount; i++)
	{
		pWnd_PASS = (CWnd*)GetDlgItem(nIDs_PASS[i]);
		pWnd_FAIL = (CWnd*)GetDlgItem(nIDs_FAIL[i]);
		
		if( m_TestInfo.TestProgress[i] == TEST_RESULT_EXE )
		{
			CDC MemDC;
			CClientDC dc(pWnd_PASS);
			CRect rect;
			BITMAP bmap;
			pWnd_PASS->GetClientRect(rect);
			pWnd_PASS->ShowWindow(SW_SHOW);
			pWnd_FAIL->ShowWindow(SW_HIDE);

			MemDC.CreateCompatibleDC(&dc);
			CBitmap *pOldBitmap = MemDC.SelectObject(&m_BitmapEXE);
			m_BitmapEXE.GetBitmap(&bmap);
			dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &MemDC, 0, 0, bmap.bmWidth, bmap.bmHeight, SRCCOPY);
			MemDC.SelectObject(pOldBitmap);
		}
		else if( m_TestInfo.TestProgress[i] == TEST_RESULT_PASS )
		{
			CDC MemDC;
			CClientDC dc(pWnd_PASS);
			CRect rect;
			BITMAP bmap;
			pWnd_PASS->GetClientRect(rect);
			pWnd_PASS->ShowWindow(SW_SHOW);
			pWnd_FAIL->ShowWindow(SW_HIDE);

			MemDC.CreateCompatibleDC(&dc);
			CBitmap *pOldBitmap = MemDC.SelectObject(&m_BitmapPASS);
			m_BitmapPASS.GetBitmap(&bmap);
			dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &MemDC, 0, 0, bmap.bmWidth, bmap.bmHeight, SRCCOPY);
			MemDC.SelectObject(pOldBitmap);
		}
		else if( m_TestInfo.TestProgress[i] == TEST_RESULT_FAIL )
		{
			CDC MemDC;
			CClientDC dc(pWnd_FAIL);
			CRect rect;
			BITMAP bmap;
			pWnd_FAIL->GetClientRect(rect);
			pWnd_FAIL->ShowWindow(SW_SHOW);
			pWnd_PASS->ShowWindow(SW_HIDE);

			MemDC.CreateCompatibleDC(&dc);
			CBitmap *pOldBitmap = MemDC.SelectObject(&m_BitmapFAIL);
			m_BitmapFAIL.GetBitmap(&bmap);
			dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &MemDC, 0, 0, bmap.bmWidth, bmap.bmHeight, SRCCOPY);
			MemDC.SelectObject(pOldBitmap);
		}
		else
		{
			pWnd_FAIL->ShowWindow(SW_HIDE);
			pWnd_PASS->ShowWindow(SW_HIDE);
		}
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
BOOL CDiagnosticToolDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	CBitmap *pOldBitmap = dc.SelectObject(&m_BitmapBGND);

	BITMAP bmap;
	m_BitmapBGND.GetBitmap(&bmap);
	pDC->StretchBlt(0, 0, rect.Width(), rect.Height(),
		&dc, 0, 0, bmap.bmWidth, bmap.bmHeight, SRCCOPY);
	dc.SelectObject(pOldBitmap);

	return TRUE;
}

HBRUSH CDiagnosticToolDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	int nCtrlId = pWnd->GetDlgCtrlID();

	switch(nCtrlId)
	{
		case IDC_EDIT_DEVICE_INFO:
			pDC->SetTextColor(RGB(217,217,217));
			pDC->SetBkMode(TRANSPARENT);
			return static_cast<HBRUSH>(s_background_brush);
	}

	return hbr;//(HBRUSH)GetStockObject(NULL_BRUSH);
}

void CDiagnosticToolDlg::OnPaint()
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
HCURSOR CDiagnosticToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDiagnosticToolDlg::OnEvent_DeviceCommunicationBreak(
	const int deviceHandle,
	void*     pContext
	)
{
	if( pContext == NULL )
		return;

	CDiagnosticToolDlg *pDlg = reinterpret_cast<CDiagnosticToolDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->PostMessage( WM_USER_DEVICE_COMMUNICATION_BREAK );
	LeaveCriticalSection(&g_CriticalSection);
}

void CDiagnosticToolDlg::OnEvent_PreviewImage(
	 const int deviceHandle,
	 void*     pContext,
	 const	   IBSU_ImageData image
   )
{
	if( pContext == NULL )
		return;

	CDiagnosticToolDlg *pDlg = reinterpret_cast<CDiagnosticToolDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	LeaveCriticalSection(&g_CriticalSection);
}

void CDiagnosticToolDlg::OnEvent_FingerCount(
	   const int                   deviceHandle,
	   void*                       pContext,
	   const IBSU_FingerCountState fingerCountState
	 )
{
	if( pContext == NULL )
		return;

	CDiagnosticToolDlg *pDlg = reinterpret_cast<CDiagnosticToolDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	LeaveCriticalSection(&g_CriticalSection);
}

void CDiagnosticToolDlg::OnEvent_FingerQuality(
      const int                     deviceHandle,   
      void                          *pContext,       
      const IBSU_FingerQualityState *pQualityArray, 
      const int                     qualityArrayCount    
	 )
{
	if( pContext == NULL )
		return;

	CDiagnosticToolDlg *pDlg = reinterpret_cast<CDiagnosticToolDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	LeaveCriticalSection(&g_CriticalSection);
}

void CDiagnosticToolDlg::OnEvent_DeviceCount(
	 const int detectedDevices,
	 void      *pContext
   )
{
	if( pContext == NULL )
		return;

	CDiagnosticToolDlg *pDlg = reinterpret_cast<CDiagnosticToolDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);
	pDlg->PostMessage( WM_USER_UPDATE_DEVICE_LIST );
	LeaveCriticalSection(&g_CriticalSection);
}

void CDiagnosticToolDlg::OnEvent_InitProgress(
	 const int deviceIndex,
	 void      *pContext,
	 const int progressValue
   )
{
	if( pContext == NULL )
		return;

	CDiagnosticToolDlg *pDlg = reinterpret_cast<CDiagnosticToolDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	LeaveCriticalSection(&g_CriticalSection);
}

void CDiagnosticToolDlg::OnEvent_TakingAcquisition(
	 const int				deviceHandle,
	 void					*pContext,
	 const IBSU_ImageType	imageType
   )
{
	if( pContext == NULL )
		return;

	CDiagnosticToolDlg *pDlg = reinterpret_cast<CDiagnosticToolDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	LeaveCriticalSection(&g_CriticalSection);
}

void CDiagnosticToolDlg::OnEvent_CompleteAcquisition(
	 const int				deviceHandle,
	 void					*pContext,
	 const IBSU_ImageType	imageType
   )
{
	if( pContext == NULL )
		return;

	CDiagnosticToolDlg *pDlg = reinterpret_cast<CDiagnosticToolDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	LeaveCriticalSection(&g_CriticalSection);
}

void CDiagnosticToolDlg::OnEvent_ResultImageEx(
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

	CDiagnosticToolDlg *pDlg = reinterpret_cast<CDiagnosticToolDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	LeaveCriticalSection(&g_CriticalSection);
}

void CDiagnosticToolDlg::OnEvent_ClearPlatenAtCapture(
      const int                   deviceHandle,
      void                        *pContext,
      const IBSU_PlatenState      platenState
    )
{
	if( pContext == NULL )
		return;

	CDiagnosticToolDlg *pDlg = reinterpret_cast<CDiagnosticToolDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	LeaveCriticalSection(&g_CriticalSection);
}

void CDiagnosticToolDlg::OnEvent_AsyncOpenDevice(
      const int                   deviceIndex,
      void                        *pContext,
      const int                   deviceHandle,
      const int                   errorCode
	  )
{
	if( pContext == NULL )
		return;

	CDiagnosticToolDlg *pDlg = reinterpret_cast<CDiagnosticToolDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	LeaveCriticalSection(&g_CriticalSection);
}

void CDiagnosticToolDlg::OnEvent_PressedKeyButtons(
      const int                deviceHandle,
      void                     *pContext,
      const int                pressedKeyButtons
    )
{
	if( pContext == NULL )
		return;

	CDiagnosticToolDlg *pDlg = reinterpret_cast<CDiagnosticToolDlg*>(pContext);
	EnterCriticalSection(&g_CriticalSection);

	LeaveCriticalSection(&g_CriticalSection);
}

LRESULT CDiagnosticToolDlg::OnMsg_UpdateDeviceList(WPARAM wParam, LPARAM lParam)
{
	// populate combo box
	CString strDevice;
	int devices = 0;
	IBSU_GetDeviceCount( &devices );

	if(devices == 0)
	{
		strDevice.Format( _T("NO DEVICE. IBSU DLL:<%s>, WEBWR Ver:<%s>" ), m_strIBSUVer, m_strWrapperVer);
		GetDlgItem(IDC_EDIT_DEVICE_INFO)->SetWindowText(strDevice);

		m_strProductName.Format(_T("NO DEVICE"));
		m_strSerialNumber.Format(_T(""));
		return 0L;
	}

	IBSU_DeviceDesc devDesc;
	if( IBSU_GetDeviceDescription( 0, &devDesc ) < IBSU_STATUS_OK )
	{
		m_strProductName.Format(_T("NO DEVICE"));
		m_strSerialNumber.Format(_T(""));
		return 0L;
	}

	if( devDesc.productName[0] == 0 )
		strDevice = _T( "unknown device" );
	else
	{
		strDevice.Format( _T("DETECTED %s.%s; IBSU DLL:<%s>, WEBWR Ver:<%s>" ), 
			devDesc.productName, devDesc.serialNumber, m_strIBSUVer, m_strWrapperVer);

		m_strProductName.Format(_T("%s"), devDesc.productName);
		m_strSerialNumber.Format(_T("%s"), devDesc.serialNumber);
	}

	GetDlgItem(IDC_EDIT_DEVICE_INFO)->SetWindowText(strDevice);
	return 0L;
}

LRESULT CDiagnosticToolDlg::OnMsg_InitWarning(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CDiagnosticToolDlg::OnMsg_UpdateDisplayResources(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CDiagnosticToolDlg::OnMsg_DeviceCommunicationBreak(WPARAM wParam, LPARAM lParam)
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

LRESULT CDiagnosticToolDlg::OnMsg_Beep(WPARAM wParam, LPARAM lParam)
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

void CDiagnosticToolDlg::_BeepFail()
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

void CDiagnosticToolDlg::_BeepSuccess()
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
	    Sleep(50);
   	    IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2/*Sol*/, 4/*100ms = 4*25ms*/, 0, 0);
    }
}

void CDiagnosticToolDlg::_BeepOk()
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

void CDiagnosticToolDlg::_BeepDeviceCommunicationBreak()
{
    for( int i=0; i<8; i++ )
    {
	    Beep( 3500, 100 );
	    Sleep( 100 );
    }
}

int CDiagnosticToolDlg::_ReleaseDevice()
{
	int nRc = IBSU_STATUS_OK;

	if( m_nDevHandle != -1 )
		nRc = IBSU_CloseDevice( m_nDevHandle );

	if( nRc >= IBSU_STATUS_OK )
	{
		m_nDevHandle = -1;
	}

	return nRc;
}  

void CDiagnosticToolDlg::OnBnClickedButtonRunTests()
{
	m_isRunTests = !m_isRunTests;

	if(m_isRunTests == TRUE)
	{
		memset(&m_TestInfo.TestProgress, TEST_RESULT_NONE, sizeof(unsigned char)*MAX_TEST_COUNT);
		GetDlgItem(IDC_BUTTON_RUN_TESTS)->SetWindowText("CANCEL");
		AfxBeginThread(_threadRunTests, this);
		AfxBeginThread(_threadUpdateResource, this);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_RUN_TESTS)->SetWindowText("RUN TESTS");
	}
}

UINT CDiagnosticToolDlg::_threadUpdateResource(LPVOID pParam)
{
	CDiagnosticToolDlg *pDlg = (CDiagnosticToolDlg*) pParam;

	while(pDlg->m_isRunTests)
	{
		pDlg->_UpdateTestResult();
		Sleep(50);
	}

	return TRUE;
}

UINT CDiagnosticToolDlg::_threadRunTests(LPVOID pParam)
{
	CDiagnosticToolDlg *pDlg = (CDiagnosticToolDlg*) pParam;

	CString strDevice;
	strDevice.Format( _T("TEST REPORT\r\n\r\n%s.%s; IBSU DLL:<%s>, WEBWR Ver:<%s>\r\n\r\n" ), 
			pDlg->m_strProductName, pDlg->m_strSerialNumber, pDlg->m_strIBSUVer, pDlg->m_strWrapperVer);
	pDlg->m_ReportDlg->_Add_Report(strDevice);

	//////////////////////////////////////////////////////////////////////////////////////
	// TEST1
	pDlg->_RunTest1();
	
	while(pDlg->m_isRunTests)
	{
		if(	pDlg->m_TestInfo.TestProgress[pDlg->m_TestIndex] != TEST_RESULT_EXE)
			break;
		Sleep(100);
	}
	//////////////////////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////////////////
	// TEST2
	pDlg->_RunTest2();
	
	while(pDlg->m_isRunTests)
	{
		if(	pDlg->m_TestInfo.TestProgress[pDlg->m_TestIndex] != TEST_RESULT_EXE)
			break;
		Sleep(100);
	}
	//////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////
	// TEST3
	pDlg->_RunTest3();
	
	while(pDlg->m_isRunTests)
	{
		if(	pDlg->m_TestInfo.TestProgress[pDlg->m_TestIndex] != TEST_RESULT_EXE)
			break;
		Sleep(100);
	}
	//////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////
	// TEST4
	pDlg->_RunTest4();
	
	while(pDlg->m_isRunTests)
	{
		if(	pDlg->m_TestInfo.TestProgress[pDlg->m_TestIndex] != TEST_RESULT_EXE)
			break;
		Sleep(100);
	}
	//////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////
	// TEST5
	pDlg->_RunTest5();
	
	while(pDlg->m_isRunTests)
	{
		if(	pDlg->m_TestInfo.TestProgress[pDlg->m_TestIndex] != TEST_RESULT_EXE)
			break;
		Sleep(100);
	}
	//////////////////////////////////////////////////////////////////////////////////////

	if( pDlg->m_isRunTests == TRUE )
	{
		pDlg->GetDlgItem(IDC_BUTTON_RUN_TESTS)->SetWindowTextA("DONE !");
		Sleep(1000);

		pDlg->m_ReportDlg->ShowWindow(SW_SHOW);
		pDlg->m_ReportDlg->SetForegroundWindow();

		pDlg->m_ReportDlg->_Add_Report("\r\n\r\nDONE !\r\n\r\n");

		pDlg->PostMessage(WM_COMMAND, IDC_BUTTON_RUN_TESTS);
	}

	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, pDlg->OnEvent_DeviceCount, pDlg );
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, pDlg->OnEvent_InitProgress, pDlg );
	IBSU_RegisterCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE, pDlg->OnEvent_AsyncOpenDevice, pDlg);

	return TRUE;
}

void CDiagnosticToolDlg::_RunTest1()
{
	m_TestIndex = __RUN_TEST_1__;
	m_TestInfo.TestProgress[m_TestIndex] = TEST_RESULT_EXE;
	m_ReportDlg->_Add_Report("\r\n\r\n================================================================\r\n");
	m_ReportDlg->_Add_Report("< TEST 1 >\r\n\r\n");

	// Search the folder program installed
	BOOL isFolderExist = FALSE;
	BOOL isSearchResult = TRUE;
	char OSVer[128];

	GetCurrentSystemInfo(OSVer);
	m_ReportDlg->_Add_Report("Operating System : %s\r\n", OSVer);

	Sleep(100);

	TCHAR pf[MAX_PATH];
	if(SHGetSpecialFolderPath(0, pf, CSIDL_PROGRAM_FILESX86 , FALSE) == TRUE)
	{
		CString inifile;

		if(pf[strlen(pf)-1] == '\\')
			inifile.Format("%sIntegrated Biometrics\\CBP Wrapper\\Bin\\CBP_FP_Wrapper.ini", pf);
		else
			inifile.Format("%s\\Integrated Biometrics\\CBP Wrapper\\Bin\\CBP_FP_Wrapper.ini", pf);

		if( access( inifile, 0) == 0)
		{
			isFolderExist = TRUE;
		}
		else
		{
			isFolderExist = FALSE;
		}
	}

	if(isFolderExist == FALSE)
	{
		m_TestInfo.TestProgress[m_TestIndex] = TEST_RESULT_FAIL;
		m_ReportDlg->_Add_Report(">>> Search installed folder [ FAIL ]\r\n\r\n");
		m_ReportDlg->_Add_Report("\r\n< TEST 1 : FAIL >");
		m_ReportDlg->_Add_Report("\r\n================================================================\r\n\r\n");
		return;
	}
	m_ReportDlg->_Add_Report(">>> Search installed folder [ PASS ]\r\n\r\n");

	////////////////////////////////////////////////////////////////////////////////
	// Search Driver folder
	CString path;
	if(pf[strlen(pf)-1] == '\\')
		path.Format("%sIntegrated Biometrics\\CBP Wrapper\\", pf);
	else
		path.Format("%s\\Integrated Biometrics\\CBP Wrapper\\", pf);

	CFileFind find;
	CString filename;
	CString answer_string;
	POSITION pos, search_pos;
	BOOL loop;

	CStringList answer_driver, search_driver;
	answer_driver.AddTail("IBScanUsbDriver(x86)_Installer.msi");
	answer_driver.AddTail("IBScanUsbDriver(x64)_Installer.msi");
	answer_driver.AddTail("IBScanUsbDriver_Installer.bat");
	answer_driver.AddTail("IBScanUsbDriver_Uninstaller.bat");

	loop = find.FindFile(path + "Driver\\*.*");
	m_ReportDlg->_Add_Report("%sDriver\r\n", path);
	m_ReportDlg->_Add_Report("   |\r\n");

	while(loop)
	{
		loop = find.FindNextFile();

		if(find.IsDots()) continue;
		if(find.IsDirectory()) continue;
		
		filename = find.GetFileName();
		search_driver.AddTail(filename);
	}

	// comapre to answer
	pos = answer_driver.GetHeadPosition();
	while(pos != NULL){
		answer_string = answer_driver.GetNext(pos);
		search_pos = search_driver.Find(answer_string);
		if(search_pos != NULL)
			m_ReportDlg->_Add_Report("   L %s [PASS]\r\n", answer_string);
		else
		{
			m_ReportDlg->_Add_Report("   L %s [FAIL]\r\n", answer_string);
			isSearchResult = FALSE;
		}
	}
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	// Search DLLs folder
	CStringList answer_bin, search_bin;
	answer_bin.AddTail("CBP_FP_Wrapper.dll");
	answer_bin.AddTail("CBP_FP_Wrapper.ini");
	answer_bin.AddTail("IBScanUltimate.dll");

	loop = find.FindFile(path + "Bin\\*.*");
	m_ReportDlg->_Add_Report("\r\n%sBin\r\n", path);
	m_ReportDlg->_Add_Report("   |\r\n");

	while(loop)
	{
		loop = find.FindNextFile();

		if(find.IsDots()) continue;
		if(find.IsDirectory()) continue;
		
		filename = find.GetFileName();
		search_bin.AddTail(filename);
	}
	// comapre to answer
    pos = answer_bin.GetHeadPosition();
	while(pos != NULL){
		answer_string = answer_bin.GetNext(pos);
		search_pos = search_bin.Find(answer_string);
		if(search_pos != NULL)
			m_ReportDlg->_Add_Report("   L %s [PASS]\r\n", answer_string);
		else
		{
			m_ReportDlg->_Add_Report("   L %s [FAIL]\r\n", answer_string);
			isSearchResult = FALSE;
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////////////////////////
	// Search Documents folder
	CStringList answer_docuemtns, search_documents;
	answer_docuemtns.AddTail("MSI-based IB USB Driver Deployment Guide.pdf");
	answer_docuemtns.AddTail("CBPWrapper Version History.pdf");

	loop = find.FindFile(path + "Documents\\*.*");
	m_ReportDlg->_Add_Report("\r\n%sDocuments\r\n", path);
	m_ReportDlg->_Add_Report("   |\r\n");

	while(loop)
	{
		loop = find.FindNextFile();

		if(find.IsDots()) continue;
		if(find.IsDirectory()) continue;
		
		filename = find.GetFileName();
		search_documents.AddTail(filename);
	}
	// comapre to answer
    pos = answer_docuemtns.GetHeadPosition();
	while(pos != NULL){
		answer_string = answer_docuemtns.GetNext(pos);
		search_pos = search_documents.Find(answer_string);
		if(search_pos != NULL)
			m_ReportDlg->_Add_Report("   L %s [PASS]\r\n", answer_string);
		else
		{
			m_ReportDlg->_Add_Report("   L %s [FAIL]\r\n", answer_string);
			isSearchResult = FALSE;
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	if(isSearchResult == TRUE)
	{
		m_TestInfo.TestProgress[m_TestIndex] = TEST_RESULT_PASS;
		m_ReportDlg->_Add_Report("\r\n< TEST 1 : PASS >");
		m_ReportDlg->_Add_Report("\r\n================================================================\r\n\r\n");
	}
	else
	{
		m_TestInfo.TestProgress[m_TestIndex] = TEST_RESULT_FAIL;
		m_ReportDlg->_Add_Report("\r\n< TEST 1 : FAIL >");
		m_ReportDlg->_Add_Report("\r\n================================================================\r\n\r\n");
	}

	Sleep(1000);
}

void CDiagnosticToolDlg::_RunTest2()
{
	m_TestIndex = __RUN_TEST_2__;
	m_TestInfo.TestProgress[m_TestIndex] = TEST_RESULT_EXE;
	m_ReportDlg->_Add_Report("\r\n\r\n================================================================\r\n");
	m_ReportDlg->_Add_Report("< TEST 2 >\r\n\r\n");

	if( cbp_fp_registerCallBacks(&m_callbacklist) != CBP_FP_OK )
	{
		m_ReportDlg->_Add_Report(">>> cbp_fp_registerCallBacks\t [ FAIL ]\r\n");
		m_TestInfo.TestProgress[m_TestIndex] = TEST_RESULT_FAIL;
		m_ReportDlg->_Add_Report("\r\n< TEST 2 : FAIL >");
		m_ReportDlg->_Add_Report("\r\n================================================================\r\n\r\n");
	}
	else
	{
		Sleep(100);
		cbp_fp_initialize("Diagnostic");
	}
}

void CDiagnosticToolDlg::_RunTest3()
{
	m_TestIndex = __RUN_TEST_3__;
	m_TestInfo.TestProgress[m_TestIndex] = TEST_RESULT_EXE;
	m_ReportDlg->_Add_Report("\r\n\r\n================================================================\r\n");
	m_ReportDlg->_Add_Report("< TEST 3 >\r\n\r\n");

	CString str_Temp;

	if( strcmp("Integrated Biometrics", cbp_fp_getProperty(m_nDevHandle, CBP_FP_PROPERTY_MAKE) ) != 0)
	{
		m_ReportDlg->_Add_Report(">>> cbp_fp_getProperty(CBP_FP_PROPERTY_MAKE) [ FAIL ]\r\n");
		m_TestInfo.TestProgress[m_TestIndex] = TEST_RESULT_FAIL;
		m_ReportDlg->_Add_Report("\r\n< TEST 3 : FAIL >");
		m_ReportDlg->_Add_Report("\r\n================================================================\r\n\r\n");
		return;
	}

	m_ReportDlg->_Add_Report(">>> cbp_fp_getProperty(CBP_FP_PROPERTY_MAKE) [ PASS ] - Integrated Biometrics\r\n");

	if( strcmp("KOJAK", cbp_fp_getProperty(m_nDevHandle, CBP_FP_PROPERTY_MODEL) ) != 0)
	{
		m_ReportDlg->_Add_Report(">>> cbp_fp_getProperty(CBP_FP_PROPERTY_MODEL) [ FAIL ]\r\n");
		m_TestInfo.TestProgress[m_TestIndex] = TEST_RESULT_FAIL;
		m_ReportDlg->_Add_Report("\r\n< TEST 3 : FAIL >");
		m_ReportDlg->_Add_Report("\r\n================================================================\r\n\r\n");
		return;
	}

	m_ReportDlg->_Add_Report(">>> cbp_fp_getProperty(CBP_FP_PROPERTY_MODEL) [ PASS ] - KOJAK\r\n");
	m_ReportDlg->_Add_Report(">>> cbp_fp_getProperty(CBP_FP_PROPERTY_SERIAL) [ PASS ] - %s\r\n", 
						cbp_fp_getProperty(m_nDevHandle, CBP_FP_PROPERTY_SERIAL));
	m_ReportDlg->_Add_Report(">>> cbp_fp_getProperty(CBP_FP_PROPERTY_SOFTWARE_VERSION) [ PASS ] - %s\r\n", 
						cbp_fp_getProperty(m_nDevHandle, CBP_FP_PROPERTY_SOFTWARE_VERSION));
	m_ReportDlg->_Add_Report(">>> cbp_fp_getProperty(CBP_FP_PROPERTY_FIRMWARE_VERSION) [ PASS ] - %s\r\n", 
						cbp_fp_getProperty(m_nDevHandle, CBP_FP_PROPERTY_FIRMWARE_VERSION));

	/*
	cbp_fp_getProperty(m_nDevHandle, CBP_FP_PROPERTY_SERIAL);
	str_Temp.Format(">>> cbp_fp_getProperty(CBP_FP_PROPERTY_SERIAL) [ PASS ]\r\n");
	m_strTestResult += str_Temp;

	cbp_fp_getProperty(m_nDevHandle, CBP_FP_PROPERTY_SOFTWARE_VERSION);
	str_Temp.Format(">>> cbp_fp_getProperty(CBP_FP_PROPERTY_SOFTWARE_VERSION) [ PASS ]\r\n");
	m_strTestResult += str_Temp;

	cbp_fp_getProperty(m_nDevHandle, CBP_FP_PROPERTY_FIRMWARE_VERSION);
	str_Temp.Format(">>> cbp_fp_getProperty(CBP_FP_PROPERTY_FIRMWARE_VERSION) [ PASS ]\r\n");
	m_strTestResult += str_Temp;
	*/

	cbp_fp_lock(m_nDevHandle);
}

void CDiagnosticToolDlg::_RunTest4()
{
	m_TestIndex = __RUN_TEST_4__;
	m_TestInfo.TestProgress[m_TestIndex] = TEST_RESULT_EXE;
	m_ReportDlg->_Add_Report("\r\n\r\n================================================================\r\n");
	m_ReportDlg->_Add_Report("< TEST 4 >\r\n\r\n");

	cbp_fp_startImaging(m_nDevHandle, slap_rightThumb, collection_flat);
}

void CDiagnosticToolDlg::_RunTest5()
{
	m_TestIndex = __RUN_TEST_5__;
	m_TestInfo.TestProgress[m_TestIndex] = TEST_RESULT_EXE;
	m_ReportDlg->_Add_Report("\r\n\r\n================================================================\r\n");
	m_ReportDlg->_Add_Report("< TEST 5 >\r\n\r\n");

	cbp_fp_close(m_nDevHandle);
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void CDiagnosticToolDlg::OnEvent_cbp_fp_onCalibrate(int handle)
{
	if( m_app->m_TestIndex == __RUN_TEST_3__ )
	{
		m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_calibrate() [ PASS ]\r\n");
		Sleep(100);
		cbp_fp_lock(handle);
	}

	/*CString remark;
	remark.Format(">>> cbp_fp_onCalibrate() - Successfully calibrated the device (%d)", handle);
	m_app->_ADD_LOG(remark);*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onCapture(int handle, struct cbp_fp_grayScaleCapture *grayScaleCapture)
{
	/*CString remark;
	remark.Format(">>> cbp_fp_onCapture() - successfully captured the final slap iamge (%d)", handle);
	m_app->_ADD_LOG(remark);
	remark.Format(">>> cbp_fp_onCapture() - final captured image dimension %d x %d", grayScaleCapture->width, grayScaleCapture->height);
	m_app->_ADD_LOG(remark);

	memcpy(m_app->m_Image, grayScaleCapture->image, grayScaleCapture->width*grayScaleCapture->height);
	m_app->m_ImageWidth = grayScaleCapture->width;
	m_app->m_ImageHeight = grayScaleCapture->height;

	m_app->_DisplayImage();*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onClose(int handle)
{
	if( m_app->m_TestIndex == __RUN_TEST_5__ )
	{
		m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_close() [ PASS ] Handle %d\r\n", handle);
		Sleep(100);
		m_app->m_nDevHandle = -1;
		cbp_fp_uninitialize("Diagnostic");
	}

	/*CString remark;
	remark.Format(">>> cbp_fp_onClose - Successfully closed the device (%d)", handle);
	m_app->_ADD_LOG(remark);*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onConfigure(int handle, bool configurationChange)
{
	if( m_app->m_TestIndex == __RUN_TEST_3__ )
	{
		m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_configure() [ PASS ] Handle %d\r\n", handle);
	}

	/*CString remark;
	remark.Format(">>> cbp_fp_onConfigure() Successfully configured the device (%d)", handle);
	m_app->_ADD_LOG(remark);*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onEnumDevices(struct cbp_fp_device *deviceList[], int deviceListLen, char *requestID)
{
	if( m_app->m_TestIndex == __RUN_TEST_2__ )
	{
		if(deviceListLen <= 0)
		{
			m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_enumDevices() [ FAIL ] - No device detected\r\n");
			m_app->m_TestInfo.TestProgress[m_app->m_TestIndex] = TEST_RESULT_FAIL;
			m_app->m_ReportDlg->_Add_Report("\r\n< TEST 2 : FAIL >");
			m_app->m_ReportDlg->_Add_Report("\r\n================================================================\r\n\r\n");
		}
		else
		{
			m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_enumDevices() [ PASS ]\r\n");
			Sleep(100);
			cbp_fp_open(deviceList[0]);
		}
	}

	/*CString remark;
	remark.Format(">>> cbp_fp_OnEnumDevices() - Detected %d device(s) connected to the workstation", deviceListLen);
	m_app->_ADD_LOG(remark);
	
	CString str;

	m_app->m_cboDevices.ResetContent();
	m_app->m_cbp_devices.clear();
	for(int i=0; i<deviceListLen; i++)
	{
		str.Format("%s (%s) - %s\n", deviceList[i]->model, deviceList[i]->serialNumber, deviceList[i]->make);
		m_app->m_cboDevices.InsertString(i, str);
		
		cbp_fp_device device;
		sprintf(device.make, deviceList[i]->make);
		sprintf(device.model, deviceList[i]->model);
		sprintf(device.serialNumber, deviceList[i]->serialNumber);
		m_app->m_cbp_devices.push_back(device);
	}

	if(deviceListLen > 0)
	{
		m_app->m_cboDevices.SetCurSel(0);
	}*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onError(int errorNumber, char *errorDescription)
{
	m_app->m_ReportDlg->_Add_Report("errorNumber %d, errorDesc %s\r\n", errorNumber, errorDescription);
	m_app->m_TestInfo.TestProgress[m_app->m_TestIndex] = TEST_RESULT_FAIL;
	m_app->m_ReportDlg->_Add_Report("\r\n< TEST %d : FAIL >", m_app->m_TestIndex+1);
	m_app->m_ReportDlg->_Add_Report("\r\n================================================================\r\n\r\n");

	if(m_app->m_TestIndex == __RUN_TEST_5__)
		cbp_fp_uninitialize("Diagnostic");

	/*CString remark;
	remark.Format(">>> cbp_fp_onError (%d) %s", errorNumber, errorDescription);
	m_app->_ADD_LOG(remark);*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onGetDirtiness(int handle , int dirtinessLevel)
{
	if( m_app->m_TestIndex == __RUN_TEST_3__ )
	{
		m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_getDirtiness() [ PASS ] Handle %d, dirtinessLevel %d\r\n", handle, dirtinessLevel);
	}

	/*CString remark;
	remark.Format(">>> cbp_fp_onGetDirtiness() - Dirtiness level %d", dirtinessLevel);
	m_app->_ADD_LOG(remark);*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onGetLockInfo(int handle, struct cbp_fp_lock_info *lockInfo)
{
	if( m_app->m_TestIndex == __RUN_TEST_3__ )
	{
		m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_getLockInfo() [ PASS ] handle %d, PID %d, duration %d\r\n", lockInfo->handle, lockInfo->lockPID, lockInfo->lockDuration);
		Sleep(100);
		cbp_fp_unlock(handle);
	}

	/*CString remark;
	remark.Format(">>> cbp_fp_onGetLockInfo() handle %d, PID %d, duration %d", lockInfo->handle, lockInfo->lockPID, lockInfo->lockDuration);
	m_app->_ADD_LOG(remark);*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onInitialize(char *requestID)
{
	/*CString remark;
	remark.Format(">>> cbp_fp_onInitialize() - Successfully initialized the device");
	m_app->_ADD_LOG(remark);*/

	if( m_app->m_TestIndex == __RUN_TEST_2__ )
	{
		m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_initialize() [ PASS ]\r\n");
		Sleep(100);
		cbp_fp_enumDevices(requestID);
	}
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onLock(int handle, int processID)
{
	if( m_app->m_TestIndex == __RUN_TEST_3__ )
	{
		m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_lock() [ PASS ] Handle %d PID %d\r\n", handle, processID);
		Sleep(100);
		cbp_fp_getLockInfo(handle);
	}

	/*CString remark;
	remark.Format(">>> cbp_fp_onLock() - Successfully locked the device. Handle %d PID %d\n", handle, processID);
	m_app->_ADD_LOG(remark);*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onOpen(int handle, struct cbp_fp_scanner_connection *scanner_connection)
{
	if( m_app->m_TestIndex == __RUN_TEST_2__ )
	{
		m_app->m_nDevHandle = handle;

		m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_open() [ PASS ] Handle %d\r\n", handle);
		m_app->m_TestInfo.TestProgress[m_app->m_TestIndex] = TEST_RESULT_PASS;
		m_app->m_ReportDlg->_Add_Report("\r\n< TEST 2 : PASS >");
		m_app->m_ReportDlg->_Add_Report("\r\n================================================================\r\n\r\n");
	}

	/*CString remark;
	remark.Format(">>> cbp_fp_onOpen() - Successfully opened the device");
	m_app->_ADD_LOG(remark);

	m_app->m_devHandle = handle;*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onPowerSave(int  handle, bool isOnPowerSave)
{
	if( m_app->m_TestIndex == __RUN_TEST_3__ )
	{
		m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_powerSave() [ PASS ] Handle %d isOnPowerSave %d\r\n", handle, isOnPowerSave);
		m_app->m_TestInfo.TestProgress[m_app->m_TestIndex] = TEST_RESULT_PASS;
		m_app->m_ReportDlg->_Add_Report("\r\n< TEST 3 : PASS >");
		m_app->m_ReportDlg->_Add_Report("\r\n================================================================\r\n\r\n");
	}

	/*CString remark;
	remark.Format(">>> cbp_fp_onPowerSave() - Successfully turned the power save mode on");
	m_app->_ADD_LOG(remark);*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onPreview(int handle, struct cbp_fp_grayScalePreview *preview)
{
	if( m_app->m_TestIndex == __RUN_TEST_4__ )
	{
		CString result;
		char str_slap[128]="", str_collect[128]="";

		m_app->GetSlapTypeCodeString(preview->slapType, str_slap), 
		m_app->GetCollectionTypeCodeString(preview->collectionType, str_collect);

		m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_startImaging() [ PASS ] Handle %d image info(W %d, H %d %s %s)\r\n", handle, 
			preview->width, preview->height, str_slap, str_collect);
		Sleep(100);
		cbp_fp_stopImaging(handle);
	}

	/*CString remark;
	remark.Format(">>> cbp_fp_onPreview()...");
	m_app->_ADD_LOG(remark);

	memcpy(m_app->m_Image, preview->image, preview->width*preview->height);
	m_app->m_ImageWidth = preview->width;
	m_app->m_ImageHeight = preview->height;

	m_app->_DisplayImage();*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onPreviewAnalysis(int handle, struct cbp_fp_previewAnalysis *previewAnalysis)
{
	/*memcpy(&m_app->m_PreviewAnalysis, previewAnalysis, sizeof(cbp_fp_previewAnalysis));
	
	struct cbp_fp_fingerAttributes fingerAttributes;
	char codeString[100] = { 0 };
	char codeString_slap[100] = { 0 };
	char codeString_collection[100] = { 0 };

	m_app->GetSlapTypeCodeString(previewAnalysis->slapType, codeString_slap);
	m_app->GetCollectionTypeCodeString(previewAnalysis->collectionType, codeString_collection);

	CString remark;
	remark.Format(">>> cbp_fp_onPreviewAnalysis() - Vendor analysis data:");
	m_app->_ADD_LOG(remark);
	remark.Format("    CollectionType: %d ( %s )", previewAnalysis->collectionType, codeString_collection);
	m_app->_ADD_LOG(remark);
	remark.Format("    SlapType: %d ( %s )", previewAnalysis->slapType, codeString_slap);
	m_app->_ADD_LOG(remark);

	for (int i = 0; i < previewAnalysis->fingerAttributeListLen; i++)
	{
		fingerAttributes = previewAnalysis->fingerAttributeList[i];
		for(int j=0; j<fingerAttributes.analysisCodeListLen; j++)
		{
			m_app->GetAnalysisCodeString(fingerAttributes.analysisCodeList[j], codeString);
			remark.Format("    FINGER [%d][%d] %s, score %d\n", i, j, codeString, fingerAttributes.score);
			m_app->_ADD_LOG(remark);
		}
	}

	m_app->_DisplayImage();*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onStopPreview(int handle)
{
	if( m_app->m_TestIndex == __RUN_TEST_4__ )
	{
		m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_stopImaging() [ PASS ] Handle %d\r\n", handle);
		m_app->m_TestInfo.TestProgress[m_app->m_TestIndex] = TEST_RESULT_PASS;
		m_app->m_ReportDlg->_Add_Report("\r\n< TEST 4 : PASS >");
		m_app->m_ReportDlg->_Add_Report("\r\n================================================================\r\n\r\n");
	}

	/*m_app->endtime = clock();

	CString remark;
	remark.Format(">>> cbp_fp_onStopPreview() - Successfully interrupt the previewing process");
	m_app->_ADD_LOG(remark);
	remark.Format(">>> cbp_fp_onStopPreview() - elapsed time (%.1f ms)", m_app->endtime - m_app->starttime);
	m_app->_ADD_LOG(remark);*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onUninitialize(char *requestID)
{
	if( m_app->m_TestIndex == __RUN_TEST_5__ )
	{
		if( m_app->m_TestInfo.TestProgress[m_app->m_TestIndex] == TEST_RESULT_EXE)
		{
			m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_uninitialize() [ PASS ] requestID %s\r\n", requestID);
			m_app->m_TestInfo.TestProgress[m_app->m_TestIndex] = TEST_RESULT_PASS;
			m_app->m_ReportDlg->_Add_Report("\r\n< TEST 5 : PASS >");
			m_app->m_ReportDlg->_Add_Report("\r\n================================================================\r\n\r\n");
		}
	}

	/*CString remark;
	remark.Format(">>> cbp_fp_onUninitialize() - Successfully uninitialized the device");
	m_app->_ADD_LOG(remark);*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onUnLock(int handle)
{
	if( m_app->m_TestIndex == __RUN_TEST_3__ )
	{
		m_app->m_ReportDlg->_Add_Report(">>> cbp_fp_unlock() [ PASS ] Handle %d\r\n", handle);
		m_app->m_TestInfo.TestProgress[m_app->m_TestIndex] = TEST_RESULT_PASS;
		m_app->m_ReportDlg->_Add_Report("\r\n< TEST 3 : PASS >");
		m_app->m_ReportDlg->_Add_Report("\r\n================================================================\r\n\r\n");
	}

	/*CString remark;
	remark.Format(">>> cbp_fp_onUnLock() - Successfully unlocked the device. Handle %d", handle);
	m_app->_ADD_LOG(remark);*/
}

void CDiagnosticToolDlg::OnEvent_cbp_fp_onWarning(int warningNumber, char *warningDescription)
{
	/*CString remark;
	remark.Format(">>> cbp_fp_onWarning (%d) %s", warningNumber, warningDescription);
	m_app->_ADD_LOG(remark);*/
}

///////////////////////////////////////////////////////////////////////////////////////
void CDiagnosticToolDlg::GetSlapTypeCodeString(cbp_fp_slapType code, char * codeString)
{
	switch(code)
	{
	case slap_rightHand: 
		sprintf(codeString, "slap_rightHand"); 
		break;

	case slap_leftHand:
		sprintf(codeString, "slap_leftHand"); 
		break;

	case slap_twoFingers: 
		sprintf(codeString, "slap_twoFingers"); 
		break;

	case slap_twoThumbs: 
		sprintf(codeString, "slap_twoThumbs"); 
		break;

	case slap_rightThumb: 
		sprintf(codeString, "slap_rightThumb"); 
		break;

	case slap_rightIndex: 
		sprintf(codeString, "slap_rightIndex"); 
		break;

	case slap_rightMiddle: 
		sprintf(codeString, "slap_rightMiddle"); 
		break;

	case slap_rightRing: 
		sprintf(codeString, "slap_rightRing"); 
		break;

	case slap_rightLittle: 
		sprintf(codeString, "slap_rightLittle"); 
		break;

	case slap_leftIndex: 
		sprintf(codeString, "slap_leftIndex"); 
		break;

	case slap_leftMiddle: 
		sprintf(codeString, "slap_leftMiddle"); 
		break;

	case slap_leftRing: 
		sprintf(codeString, "slap_leftRing"); 
		break;

	case slap_leftLittle: 
		sprintf(codeString, "slap_leftLittle"); 
		break;

	case slap_twotThumbs: 
		sprintf(codeString, "slap_twotThumbs"); 
		break;

	case slap_stitchedLeftThumb: 
		sprintf(codeString, "slap_stitchedLeftThumb"); 
		break;

	case slap_stitchedRightThumb: 
		sprintf(codeString, "slap_stitchedRightThumb"); 
		break;

	case slap_unknown: 
	default:
		sprintf(codeString, "slap_unknown"); 
		break;
	}
}

void CDiagnosticToolDlg::GetCollectionTypeCodeString(cbp_fp_collectionType code, char * codeString)
{
	switch(code)
	{
	case collection_rolled: 
		sprintf(codeString, "collection_rolled"); 
		break;

	case collection_flat: 
		sprintf(codeString, "collection_flat"); 
		break;

	case collection_contactless: 
		sprintf(codeString, "collection_contactless"); 
		break;
	
	case collection_unknown:
	default: 
		sprintf(codeString, "collection_unknown");
		break;
	}
}

void CDiagnosticToolDlg::GetCurrentSystemInfo(char *OSInfo)
{
#define VER_SUITE_WH_SERVER 0x8000

	OSVERSIONINFOEX osVersionInfoEx;

	ZeroMemory(&osVersionInfoEx, sizeof(OSVERSIONINFOEX));
	osVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	GetVersionEx((LPOSVERSIONINFO)&osVersionInfoEx);

	switch (osVersionInfoEx.dwMajorVersion)
	{
		case 5:
		{
			switch (osVersionInfoEx.dwMinorVersion)
			{
			case 1:
				sprintf(OSInfo, "Windows XP");
				break;

			case 2:
				if (osVersionInfoEx.wProductType == VER_NT_WORKSTATION)
				{
					sprintf(OSInfo, "Windows XP Professional x64 Edition");
					break;
				}
				else if (GetSystemMetrics(SM_SERVERR2) == 0)
				{
					sprintf(OSInfo, "Windows Server 2003");
					break;
				}
				else if (osVersionInfoEx.wSuiteMask & VER_SUITE_WH_SERVER)
				{
					sprintf(OSInfo, "Windows Home Server");
					break;
				}
				else if (GetSystemMetrics(SM_SERVERR2) != 0)
				{
					sprintf(OSInfo, "Windows Server 2003 R2");
					break;
				}
				break;

			default:
				sprintf(OSInfo, "Can't find version name of dwMiniorVersion: %d", osVersionInfoEx.dwMinorVersion);
				break;
			}
			break;
		}
		case 6:
		{
			switch (osVersionInfoEx.dwMinorVersion)
			{
			case 0:
				if (osVersionInfoEx.wProductType == VER_NT_WORKSTATION)
				{
					sprintf(OSInfo, "Windows Vista");
					break;
				}
				else if (osVersionInfoEx.wProductType != VER_NT_WORKSTATION)
				{
					sprintf(OSInfo, "Windows Server 2008");
					break;
				}
				break;
			case 1:
				if (osVersionInfoEx.wProductType != VER_NT_WORKSTATION)
				{
					sprintf(OSInfo, "Windows Server 2008 R2");
					break;
				}
				else if (osVersionInfoEx.wProductType == VER_NT_WORKSTATION)
				{
					sprintf(OSInfo, "Windows 7");
					break;
				}
				break;
			case 2:
				if (osVersionInfoEx.wProductType != VER_NT_WORKSTATION)
				{
					sprintf(OSInfo, "Windows Server 2012");
					break;
				}
				else if (osVersionInfoEx.wProductType == VER_NT_WORKSTATION)
				{
					sprintf(OSInfo, "Windows 8");
					break;
				}
				break;
			case 3:
				if (osVersionInfoEx.wProductType != VER_NT_WORKSTATION)
				{
					sprintf(OSInfo, "Windows Server 2012 R2");
					break;
				}
				else if (osVersionInfoEx.wProductType == VER_NT_WORKSTATION)
				{
					sprintf(OSInfo, "Windows 8.1");
					break;
				}
				break;
			default:
				sprintf(OSInfo, "Can't find version name of dwMiniorVersion: %d", osVersionInfoEx.dwMinorVersion);
				break;
			}
			break;
		}
		case 10:
		{
			switch (osVersionInfoEx.dwMinorVersion)
			{
			case 0:
				if (osVersionInfoEx.wProductType != VER_NT_WORKSTATION)
				{
					sprintf(OSInfo, "Windows 10");
					break;
				}
				else if (osVersionInfoEx.wProductType != VER_NT_WORKSTATION)
				{
					sprintf(OSInfo, "Windows Server 2016 Technical Preview");
					break;
				}
				break;
			default:
				sprintf(OSInfo, "Can't find version name of dwMiniorVersion: %d", osVersionInfoEx.dwMinorVersion);
				break;
			}
		}
		default:
			break;
	}

	Sleep(100);
}
