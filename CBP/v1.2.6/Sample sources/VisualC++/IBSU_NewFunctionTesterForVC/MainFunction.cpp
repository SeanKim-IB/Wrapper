// MainFunction.cpp : implementation file
//

#include "stdafx.h"
#include "IBSU_NewFunctionTester.h"
#include "MainFunction.h"
#include "IBSU_NewFunctionTesterDlg.h"


// CMainFunction dialog

IMPLEMENT_DYNAMIC(CMainFunction, CDialog)


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


UINT CMainFunction::_InitializeDeviceThreadCallback( LPVOID pParam )
{   
	if( pParam == NULL )
		return 1;

	CMainFunction* pDlg = (CMainFunction*)pParam;

	int		devIndex = pDlg->GetDlgItemInt(IDC_EDIT_INDEX);
	int		devHandle;
	int		nRc;
	CString temp;
	int		pos=0;

	nRc = IBSU_OpenDevice(devIndex, &devHandle);

	if( nRc >= IBSU_STATUS_OK )
	{
		pDlg->_SetRegisterCallbacksForInitDevice(devHandle);
	}

	pDlg->m_pMainDlg->_SetResultMessageOnListView(pDlg->m_pMainDlg->m_IsAppend, "-- IBSU_OpenDevice -- Return value (%d)", nRc);
	pDlg->m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Device handle : %d", devHandle);

	return 0;
}

UINT CMainFunction::_InitializeDeviceExThreadCallback( LPVOID pParam )
{   
	if( pParam == NULL )
		return 1;

	CMainFunction* pDlg = (CMainFunction*)pParam;

	int		devIndex = pDlg->GetDlgItemInt(IDC_EDIT_INDEX);
	int		devHandle;
	int		nRc;
	CString temp;
	int		pos=0;
	CString	unformityMaskPath;

	pDlg->GetDlgItemText(IDC_EDIT_UMASK_PATH, unformityMaskPath);

	nRc = IBSU_OpenDeviceEx(devIndex, unformityMaskPath, pDlg->m_chkAsyncOpenDeviceEx, &devHandle);

	if( nRc >= IBSU_STATUS_OK )
	{
		pDlg->_SetRegisterCallbacksForInitDevice(devHandle);
	}

	pDlg->m_pMainDlg->_SetResultMessageOnListView(pDlg->m_pMainDlg->m_IsAppend, "-- IBSU_OpenDeviceEx -- Return value (%d)", nRc);
	pDlg->m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Device handle : %d", devHandle);

	return 0;
}

int CMainFunction::_SetRegisterCallbacksForInitDevice( const int devHandle )
{
	// register callback functions
	IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, m_pMainDlg->OnEvent_DeviceCommunicationBreak, m_pMainDlg );//Ok    
	IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, m_pMainDlg->OnEvent_PreviewImage, m_pMainDlg );//Ok
	IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, m_pMainDlg->OnEvent_TakingAcquisition, m_pMainDlg );//Ok  
	IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, m_pMainDlg->OnEvent_CompleteAcquisition, m_pMainDlg );//Ok  

	IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX, m_pMainDlg->OnEvent_ResultImageEx, m_pMainDlg );  //Ok
    IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, m_pMainDlg->OnEvent_FingerCount, m_pMainDlg );  //Ok
	IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY, m_pMainDlg->OnEvent_FingerQuality, m_pMainDlg ); //Ok 
	IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE, m_pMainDlg->OnEvent_ClearPlatenAtCapture, m_pMainDlg );
	IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_NOTIFY_MESSAGE, m_pMainDlg->OnEvent_NotifyMessage, m_pMainDlg );  
	IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_KEYBUTTON, m_pMainDlg->OnEvent_PressedKeyButtons, m_pMainDlg );

	return 0;
}

CMainFunction::CMainFunction(CWnd* pParent /*=NULL*/)
	: CDialog(CMainFunction::IDD, pParent)
	, m_chkAsyncOpenDeviceEx(FALSE)
	, m_chkEnableTraceLog(TRUE)
{
}

CMainFunction::~CMainFunction()
{
}

void CMainFunction::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_PROPERTY, m_cbProperty);
	//DDX_Control(pDX, IDC_COMBO2, m_cbSetLEOperationMode);
	DDX_Check(pDX, IDC_CHECK_ASYNC_OPENDEVICE_EX, m_chkAsyncOpenDeviceEx);
	DDX_Check(pDX, IDC_CHECK_IBSU_ENABLETRACELOG, m_chkEnableTraceLog);
}


BEGIN_MESSAGE_MAP(CMainFunction, CDialog)
	//ON_BN_CLICKED(IDC_BUTTON2, &CMainFunction::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON_GETDEVICECOUNT, &CMainFunction::OnBnClickedButtonGetdevicecount)
//	ON_CBN_SELCHANGE(IDC_COMBO1, &CMainFunction::OnCbnSelchangeCombo1)
//ON_BN_CLICKED(IDC_BUTTON1, &CMainFunction::OnBnClickedButton1)
ON_BN_CLICKED(IDC_BUTTON_GETSDKVERSION, &CMainFunction::OnBnClickedButtonGetsdkversion)
ON_BN_CLICKED(IDC_BUTTON_GETDEVICEDESCRIPTION, &CMainFunction::OnBnClickedButtonGetdevicedescription)
ON_BN_CLICKED(IDC_BUTTON_OPENDEVICE, &CMainFunction::OnBnClickedButtonOpendevice)
ON_BN_CLICKED(IDC_BUTTON_CLOSEDEVICE, &CMainFunction::OnBnClickedButtonClosedevice)
ON_BN_CLICKED(IDC_BUTTON_ISDEVICEOPENED, &CMainFunction::OnBnClickedButtonIsdeviceopened)
ON_BN_CLICKED(IDC_BUTTON_GETPROPERTY, &CMainFunction::OnBnClickedButtonGetproperty)
ON_BN_CLICKED(IDC_BUTTON_SETPROPERTY, &CMainFunction::OnBnClickedButtonSetproperty)
//ON_BN_CLICKED(IDC_BUTTON_GETLEOPERATIONMODE, &CMainFunction::OnBnClickedButtonGetleoperationmode)
//ON_BN_CLICKED(IDC_BUTTON_SETLEOPERATIONMODE, &CMainFunction::OnBnClickedButtonSetleoperationmode)
//ON_BN_CLICKED(IDC_BUTTON_GETOPERABLELEDS, &CMainFunction::OnBnClickedButtonGetoperableleds)
//ON_BN_CLICKED(IDC_BUTTON_GETLEDS, &CMainFunction::OnBnClickedButtonGetleds)
//ON_BN_CLICKED(IDC_BUTTON_SETLEDS, &CMainFunction::OnBnClickedButtonSetleds)
ON_BN_CLICKED(IDC_BUTTON_ASYNCOPENDEVICE, &CMainFunction::OnBnClickedButtonAsyncopendevice)
ON_BN_CLICKED(IDC_BUTTON_CLOSEALLDEVICE, &CMainFunction::OnBnClickedButtonClosealldevice)
ON_BN_CLICKED(IDC_BUTTON_BGETINITPROGRESS, &CMainFunction::OnBnClickedButtonBgetinitprogress)
ON_BN_CLICKED(IDC_BUTTON_OPENDEVICE_EX, &CMainFunction::OnBnClickedButtonOpendeviceEx)
ON_BN_CLICKED(IDC_BUTTON_IMAGE_UMASKPATH, &CMainFunction::OnBnClickedButtonImageUmaskpath)
//ON_BN_CLICKED(IDC_BUTTON_ENABLETRACELOG, &CMainFunction::OnBnClickedButtonEnabletracelog)
ON_BN_CLICKED(IDC_CHECK_IBSU_ENABLETRACELOG, &CMainFunction::OnBnClickedCheckIbsuEnabletracelog)
END_MESSAGE_MAP()


// CMainFunction message handlers

/*void CMainFunction::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
}*/

void CMainFunction::OnBnClickedButtonGetdevicecount()
{
	// TODO: Add your control notification handler code here

	//CIBSU_NewFunctionTesterDlg* pDlg = reinterpret_cast<CIBSU_NewFunctionTesterDlg*>(m_pMainDlg);

	//m_pMainDlg = (CIBSU_NewFunctionTesterDlg*)AfxGetMainWnd();


	//CIBSU_NewFunctionTesterDlg* pDlg = (CIBSU_NewFunctionTesterDlg*)GetParent();


	int pos = 0;
	int devices = 0;
	//CString test = "dd";

	int nRc = IBSU_GetDeviceCount(&devices);
//	pDlg->Test_Result(test);
//	pDlg->Test_Result(devices);

//	pDlg->_SetResultMessageOnListView(true, "-- IBSU_GetDeviceDescription -- Return value (%d)", nRc);
//	pDlg->_SetAdditionalMessageOnListView(1, "Number of Device(s) : %d", devices);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_GetDeviceDescription -- Return value (%d)", nRc);
	m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Number of Device(s) : %d", devices);

}

//void CMainFunction::OnCbnSelchangeCombo1()
//{
//	// TODO: Add your control notification handler code here
//}

//void CMainFunction::OnBnClickedButton1()
//{
//	// TODO: Add your control notification handler code here
//}

void CMainFunction::OnBnClickedButtonGetsdkversion()
{
	// TODO: Add your control notification handler code here

	//m_pMainDlg = (CIBSU_NewFunctionTesterDlg*)AfxGetMainWnd();

	int		pos=0;
	IBSU_SdkVersion verInfo;

	int nRc = IBSU_GetSDKVersion(&verInfo);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_GetSDKVersion -- Return value (%d)", nRc);

	m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Product : %s", verInfo.Product);
	m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "File : %s", verInfo.File);
}

void CMainFunction::OnBnClickedButtonGetdevicedescription()
{
	// TODO: Add your control notification handler code here
	int		pos=0;
	int		devIndex = GetDlgItemInt(IDC_EDIT_INDEX);
	IBSU_DeviceDesc devDesc;

	int nRc = IBSU_GetDeviceDescription(devIndex, &devDesc);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_GetDeviceDescription -- Return value (%d)", nRc);

	m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Serial Number : %s", devDesc.serialNumber);
	m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Product Name : %s", devDesc.productName);
	m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Interface Type : %s", devDesc.interfaceType);
	m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Firmware Version : %s", devDesc.fwVersion);
	m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Device Revision : %s", devDesc.devRevision);
	m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Device Handle : %d", devDesc.handle);
	m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Handle Opened : %d", devDesc.IsHandleOpened);
}

BOOL CMainFunction::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	m_pMainDlg = (CIBSU_NewFunctionTesterDlg*)AfxGetMainWnd();

	GetDlgItem( IDC_EDIT_INDEX )->SetWindowText("0");
	GetDlgItem( IDC_EDIT_DEVICEHANDLE )->SetWindowText("0");

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
	m_cbProperty.SetCurSel(0);

    TCHAR	unformityMaskPath[MAX_PATH + 1];				///< Base folder for uniformity mask path
    SHGetSpecialFolderPath( NULL, unformityMaskPath, CSIDL_MYPICTURES, TRUE );
	GetDlgItem( IDC_EDIT_UMASK_PATH )->SetWindowText(unformityMaskPath);

/*	m_cbSetLEOperationMode.AddString("LE_OPERATION_AUTO");
	m_cbSetLEOperationMode.AddString("LE_OPERATION_ON");
	m_cbSetLEOperationMode.AddString("LE_OPERATION_OFF");
	m_cbSetLEOperationMode.SetCurSel(0);*/

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMainFunction::OnBnClickedButtonOpendevice()
{
	// TODO: Add your control notification handler code here
	VERIFY(::AfxBeginThread(_InitializeDeviceThreadCallback, this, THREAD_PRIORITY_NORMAL, 
							 0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );

}

int CMainFunction::_GetCurrentDeviceHandle()
{
	int devHandle = GetDlgItemInt(IDC_EDIT_DEVICEHANDLE); 
	
	return devHandle;
}

void CMainFunction::OnBnClickedButtonClosedevice()
{
	int nRc = IBSU_CloseDevice(_GetCurrentDeviceHandle());

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_CloseDevice -- Return value (%d)", nRc);
}

void CMainFunction::OnBnClickedButtonIsdeviceopened()
{
	// TODO: Add your control notification handler code here
	int nRc = IBSU_IsDeviceOpened(_GetCurrentDeviceHandle());

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_IsDeviceOpened -- Return value (%d)", nRc);
}

void CMainFunction::OnBnClickedButtonGetproperty()
{
	// TODO: Add your control notification handler code here
	int		pos=0;
	CString	propertyName;
	char	propertyValue[IBSU_MAX_STR_LEN];
	int		propertyIndex;
	int nRc;

	UpdateData(FALSE);
	propertyIndex = m_cbProperty.GetCurSel();
	GetDlgItemText(IDC_COMBO_PROPERTY, propertyName);

	nRc = IBSU_GetProperty(_GetCurrentDeviceHandle(), (IBSU_PropertyId)propertyIndex, propertyValue);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_GetProperty -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Property value : %s", propertyValue);
	}
}

void CMainFunction::OnBnClickedButtonSetproperty()
{
	int		pos=0;
	CString	propertyValue, propertyName;
	int		propertyIndex;
	int		nRc;

	propertyIndex = m_cbProperty.GetCurSel();
	GetDlgItemText(IDC_EDIT_SETPROPERTY, propertyValue);
	GetDlgItemText(IDC_COMBO_PROPERTY, propertyName);

	nRc = IBSU_SetProperty(_GetCurrentDeviceHandle(), (IBSU_PropertyId)propertyIndex, propertyValue);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_SetProperty -- Return value (%d)", nRc);
}

void CMainFunction::OnBnClickedButtonAsyncopendevice()
{
	int nRc = -1;
	int pos = 0;
	int devIndex = GetDlgItemInt(IDC_EDIT_INDEX);

	nRc = IBSU_AsyncOpenDevice(devIndex);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_AsyncOpenDevice -- Return value (%d)", nRc);
}

void CMainFunction::OnBnClickedButtonClosealldevice()
{
	int nRc = -1;
	nRc = IBSU_CloseAllDevice();

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_CloseAllDevice -- Return value (%d)", nRc);
}

void CMainFunction::OnBnClickedButtonBgetinitprogress()
{
	int		devIndex = GetDlgItemInt(IDC_EDIT_INDEX);
	int		pos=0;
	BOOL	isComplete;
	int		devHandle;
	int		progressValue;


	int nRc = IBSU_BGetInitProgress(devIndex, &isComplete, &devHandle, &progressValue);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_BGetInitProgress -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "isComplete : %d", isComplete);
		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Handle : %d", devHandle);
		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Progress Value : %d", progressValue);
	}
}

void CMainFunction::OnBnClickedButtonOpendeviceEx()
{
	int		pos=0;
	int		devIndex = GetDlgItemInt(IDC_EDIT_INDEX);
	CString	unformityMaskPath;
	int		devHandle;


	UpdateData();
	GetDlgItemText(IDC_EDIT_UMASK_PATH, unformityMaskPath);

	if( m_chkAsyncOpenDeviceEx )
	{
		int nRc = IBSU_OpenDeviceEx(devIndex, unformityMaskPath, m_chkAsyncOpenDeviceEx, &devHandle);

		m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_OpenDeviceEx -- Return value (%d)", nRc);

		if( nRc == IBSU_STATUS_OK )
		{
			m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Handle : %d", devHandle);
		}
	}
	else
	{
		VERIFY(::AfxBeginThread(_InitializeDeviceExThreadCallback, this, THREAD_PRIORITY_NORMAL, 
								 0x100000, STACK_SIZE_PARAM_IS_A_RESERVATION ) != NULL );
	}
}

void CMainFunction::OnBnClickedButtonImageUmaskpath()
{
	TCHAR		  uniformityMaskPath[MAX_PATH + 1];
	LPITEMIDLIST  pidlSelected;
	BROWSEINFO    bi = {0};
	LPMALLOC      pMalloc;

	SHGetMalloc( &pMalloc );

	// show 'Browse For Folder' dialog:
	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = _T("Please select a folder to use unformity mask!");
	bi.ulFlags = BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseForFolderCallback;
	bi.lParam = (LPARAM)&uniformityMaskPath[0];

	pidlSelected = SHBrowseForFolder(&bi);

	if( pidlSelected )
	{
		SHGetPathFromIDList( pidlSelected, uniformityMaskPath );
		pMalloc->Free(pidlSelected);

	    if (IBSU_IsWritableDirectory(uniformityMaskPath, TRUE) != IBSU_STATUS_OK)
	    {
		    AfxMessageBox("You don't have writing permission on this folder\r\nPlease select another folder (e.g. desktop folder)");
	    }
        else
        {
		GetDlgItem( IDC_EDIT_UMASK_PATH )->SetWindowText(uniformityMaskPath);
        }
	}
	pMalloc->Release();
}

BOOL CMainFunction::PreTranslateMessage(MSG* pMsg)
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



void CMainFunction::OnBnClickedCheckIbsuEnabletracelog()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	int nRc = -1;

	if( m_chkEnableTraceLog )
	{
		nRc = IBSU_EnableTraceLog(TRUE);
	}
	else
	{
		nRc = IBSU_EnableTraceLog(FALSE);
	}

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_EnableTraceLog -- Return value (%d)", nRc);
	
}
