// NonCallbackFunctions.cpp : implementation file
//

#include "stdafx.h"
#include "IBSU_NewFunctionTester.h"
#include "NonCallbackFunctions.h"
#include "IBSU_NewFunctionTesterDlg.h"


// CNonCallbackFunctions dialog

IMPLEMENT_DYNAMIC(CNonCallbackFunctions, CDialog)

CNonCallbackFunctions::CNonCallbackFunctions(CWnd* pParent /*=NULL*/)
	: CDialog(CNonCallbackFunctions::IDD, pParent)
	, m_chkAutoContrast(FALSE)
	, m_chkAutoCapture(FALSE)
	, m_chkIgnoreNumberofFinger(FALSE)
{

}

CNonCallbackFunctions::~CNonCallbackFunctions()
{
}

void CNonCallbackFunctions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_IMAGETYPE, m_cbImageType);
	DDX_Check(pDX, IDC_CHECK_AUTOCONTRAST, m_chkAutoContrast);
	DDX_Check(pDX, IDC_CHECK_AUTOCAPTURE, m_chkAutoCapture);
	DDX_Check(pDX, IDC_CHECK_TRIGGER, m_chkIgnoreNumberofFinger);
}


BEGIN_MESSAGE_MAP(CNonCallbackFunctions, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ASYNCOPENDEVICE, &CNonCallbackFunctions::OnBnClickedButtonAsyncopendevice)
	ON_BN_CLICKED(IDC_BUTTON_BGETINITPROGRESS, &CNonCallbackFunctions::OnBnClickedButtonBgetinitprogress)
	ON_BN_CLICKED(IDC_BUTTON_CLOSEDEVICE2, &CNonCallbackFunctions::OnBnClickedButtonClosedevice2)
	ON_BN_CLICKED(IDC_BUTTON_CLOSEALLDEVICE, &CNonCallbackFunctions::OnBnClickedButtonClosealldevice)
	ON_BN_CLICKED(IDC_BUTTON_BEGINCAPTURE, &CNonCallbackFunctions::OnBnClickedButtonBegincapture)
	ON_BN_CLICKED(IDC_BUTTON_CANCELCAPTURE, &CNonCallbackFunctions::OnBnClickedButtonCancelcapture)
	ON_BN_CLICKED(IDC_BUTTON_TAKERESULTIMAGE, &CNonCallbackFunctions::OnBnClickedButtonTakeresultimage)
	ON_BN_CLICKED(IDC_BUTTON_BGETIMAGE, &CNonCallbackFunctions::OnBnClickedButtonBgetimage)
	ON_BN_CLICKED(IDC_BUTTON_BGETCLEARPLATENATCAPTURE, &CNonCallbackFunctions::OnBnClickedButtonBgetclearplatenatcapture)
	ON_BN_CLICKED(IDC_BUTTON_GETCONTRAST, &CNonCallbackFunctions::OnBnClickedButtonGetcontrast)
	ON_BN_CLICKED(IDC_BUTTON_SETCONTRAST, &CNonCallbackFunctions::OnBnClickedButtonSetcontrast)
	ON_BN_CLICKED(IDC_CHECK_AUTOCONTRAST, &CNonCallbackFunctions::OnBnClickedCheckAutocontrast)
	ON_BN_CLICKED(IDC_CHECK_AUTOCAPTURE, &CNonCallbackFunctions::OnBnClickedCheckAutocapture)
	ON_BN_CLICKED(IDC_CHECK_TRIGGER, &CNonCallbackFunctions::OnBnClickedCheckTrigger)
END_MESSAGE_MAP()


// CNonCallbackFunctions message handlers

int CNonCallbackFunctions::_GetCurrentDeviceHandle()
{
	int deviceHandle = GetDlgItemInt(IDC_EDIT_DEVICEHANDLE);
	return deviceHandle;
}




BOOL CNonCallbackFunctions::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_pMainDlg = (CIBSU_NewFunctionTesterDlg*)AfxGetMainWnd();

	GetDlgItem( IDC_EDIT_INDEX )->SetWindowText("0");
	GetDlgItem( IDC_EDIT_DEVICEHANDLE )->SetWindowText("0");

	m_cbImageType.AddString("ENUM_IBSU_TYPE_NONE");
	m_cbImageType.AddString("ENUM_IBSU_ROLL_SINGLE_FINGER");
	m_cbImageType.AddString("ENUM_IBSU_FLAT_SINGLE_FINGER");
	m_cbImageType.AddString("ENUM_IBSU_FLAT_TWO_FINGERS");
	m_cbImageType.AddString("ENUM_IBSU_FLAT_FOUR_FINGERS");
	m_cbImageType.SetCurSel(2);

	GetDlgItem(IDC_EDIT_BCONTRAST)->SetWindowText("21");

	IBSU_ReleaseCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT );
	IBSU_ReleaseCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS );
	IBSU_ReleaseCallbacks( NULL, ENUM_IBSU_ESSENTIAL_EVENT_ASYNC_OPEN_DEVICE );

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CNonCallbackFunctions::OnBnClickedButtonAsyncopendevice()
{
	int nRc = -1;
	int pos = 0;
	int devIndex = GetDlgItemInt(IDC_EDIT_INDEX);

	m_pMainDlg->m_bUseNonCallback = TRUE;

	nRc = IBSU_AsyncOpenDevice(devIndex);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_AsyncOpenDevice -- Return value (%d)", nRc);
}

void CNonCallbackFunctions::OnBnClickedButtonBgetinitprogress()
{
	int		devIndex = GetDlgItemInt(IDC_EDIT_INDEX);
	int		pos=0;
	BOOL	isComplete;
	int		devHandle;
	int		progressValue;
	CString strValue;


	int nRc = IBSU_BGetInitProgress(devIndex, &isComplete, &devHandle, &progressValue);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_BGetInitProgress -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		strValue.Format("%d", progressValue);
		GetDlgItem(IDC_EDIT_BGETINITPREOGRESS)->SetWindowText(strValue);

		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "isComplete : %d", isComplete);
		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Handle : %d", devHandle);
		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Progress Value : %d", progressValue);
	}
}

void CNonCallbackFunctions::OnBnClickedButtonClosedevice2()
{
	int nRc = IBSU_CloseDevice(_GetCurrentDeviceHandle());

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_CloseDevice -- Return value (%d)", nRc);
}

void CNonCallbackFunctions::OnBnClickedButtonClosealldevice()
{
	int nRc = -1;
	nRc = IBSU_CloseAllDevice();

	m_pMainDlg->GetDlgItem(IDC_EDIT_ASYNCOPENDEVICE)->SetWindowTextA("");
	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_CloseAllDevice -- Return value (%d)", nRc);
}

void CNonCallbackFunctions::OnBnClickedButtonBegincapture()
{
	const IBSU_ImageType imgType = (IBSU_ImageType)m_cbImageType.GetCurSel();
	const IBSU_ImageResolution imgRes = ENUM_IBSU_IMAGE_RESOLUTION_500;		// Currently we only support 500ppi.
	DWORD captureOptions = 0;


	if( m_chkAutoContrast )
		captureOptions |= IBSU_OPTION_AUTO_CONTRAST;
	if( m_chkAutoCapture )
		captureOptions |= IBSU_OPTION_AUTO_CAPTURE;
	if( m_chkIgnoreNumberofFinger )
		captureOptions |= IBSU_OPTION_IGNORE_FINGER_COUNT;

	int nRc = IBSU_BeginCaptureImage(_GetCurrentDeviceHandle(), imgType, imgRes, captureOptions);
	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_BeginCaptureImage -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		GetDlgItem( IDC_BUTTON_TAKERESULTIMAGE )->EnableWindow( TRUE );
	}
}

void CNonCallbackFunctions::OnBnClickedButtonCancelcapture()
{
	int nRc = IBSU_CancelCaptureImage( _GetCurrentDeviceHandle() );
	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_CancelCaptureImage -- Return value (%d)", nRc);
}

void CNonCallbackFunctions::OnBnClickedButtonBgetimage()
{
	int				pos=0;
	IBSU_ImageData	imgData;
	IBSU_ImageType	imgType;
	IBSU_ImageData	segmentImageArray[IBSU_MAX_SEGMENT_COUNT];
	IBSU_SegmentPosition segmentPositionArray[IBSU_MAX_SEGMENT_COUNT];
	int				segmentImageArrayCount;
	IBSU_FingerCountState	fingerCountState;
	IBSU_FingerQualityState	fingerQualityState[IBSU_MAX_SEGMENT_COUNT];
	int				qualityArrayCount;
	CString			strIsFinal, strFingerCount, strQuality;
	int				imageStatus = 0;
	int				detectedFingerCount = 0;

	int nRc = IBSU_BGetImageEx(_GetCurrentDeviceHandle(), &imageStatus, &imgData, &imgType, &detectedFingerCount, &segmentImageArray[0], 
		&segmentPositionArray[0], &segmentImageArrayCount, &fingerCountState, &fingerQualityState[0], &qualityArrayCount);


	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_BGetImage -- Return value (%d)", nRc);


	if(nRc == IBSU_STATUS_OK)
	{
		
		if( imgData.IsFinal ) strIsFinal = _T("RESULT");
		else strIsFinal = _T("PREVIEW");
		GetDlgItem(IDC_EDIT_BFLAG)->SetWindowText(strIsFinal);

		if( fingerCountState == ENUM_IBSU_FINGER_COUNT_OK )
			strFingerCount = "FINGER_COUNT_OK";
		else if( fingerCountState == ENUM_IBSU_TOO_MANY_FINGERS )
			strFingerCount = "TOO_MANY_FINGERS";
		else if( fingerCountState == ENUM_IBSU_TOO_FEW_FINGERS )
			strFingerCount = "TOO_FEW_FINGERS";
		else if( fingerCountState == ENUM_IBSU_NON_FINGER )
			strFingerCount = "NON-FINGER";
		else
			strFingerCount = "UNKNOWN";
		GetDlgItem(IDC_EDIT_BFINGERCOUNT)->SetWindowText(strFingerCount);

		for(int i=0; i<qualityArrayCount; i++)
		{
			switch( fingerQualityState[i])
			{ 
			case ENUM_IBSU_QUALITY_GOOD:
				strQuality += "(Good)";
				break;          
			case ENUM_IBSU_QUALITY_FAIR :
				strQuality += "(Fair)";
				break;          
			case ENUM_IBSU_QUALITY_POOR :
				strQuality += "(Poor)";
				break;          
			case ENUM_IBSU_QUALITY_INVALID_AREA_TOP :
				strQuality += "(Inv-Top)";
				break;          
			case ENUM_IBSU_QUALITY_INVALID_AREA_LEFT :
				strQuality += "(Inv-Left)";
				break;          
			case ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT :
				strQuality += "(Inv-Right)";
				break;          
			case ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM :
				strQuality += "(Inv-Bottom)";
				break;          
			case ENUM_IBSU_FINGER_NOT_PRESENT:
				strQuality += "(Non)";
				break;
			default:
				strQuality += "(Non)";
				break;
			}
		}
		GetDlgItem(IDC_EDIT_BFINGERQUALITY)->SetWindowText(strQuality);

		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "IsFinal : %d", imgData.IsFinal);
		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "FingerCountState : " + strFingerCount);
		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "FingerQualityState : " + strQuality);
	}
	else{}
}

void CNonCallbackFunctions::OnBnClickedButtonBgetclearplatenatcapture()
{
	int					pos=0;
	IBSU_PlatenState	platenState;
	CString				strValue;

	int nRc = IBSU_BGetClearPlatenAtCapture(_GetCurrentDeviceHandle(), &platenState);

	if( platenState == ENUM_IBSU_PLATEN_CLEARD )
		strValue = _T("PLATEN_CLEARD");
	else
		strValue = _T("PLATEN_HAS_FINGERS");
	GetDlgItem(IDC_EDIT_BCLEARPLATENATCAPTURE)->SetWindowText(strValue);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_BGetClearPlatenAtCapture -- Return value (%d)", nRc);

	m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "PlatenState : " + strValue);
}

void CNonCallbackFunctions::OnBnClickedButtonTakeresultimage()
{
	int nRc = IBSU_TakeResultImageManually(_GetCurrentDeviceHandle());
	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_TakeResultImageManually -- Return value (%d)", nRc);
}

void CNonCallbackFunctions::OnBnClickedButtonGetcontrast()
{
	int		pos=0;
	int		contrastValue;


	int nRc = IBSU_GetContrast(_GetCurrentDeviceHandle(), &contrastValue);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_GetContrast -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Contrast : %d", contrastValue);
	}
}

void CNonCallbackFunctions::OnBnClickedButtonSetcontrast()
{
	UpdateData(FALSE);

	int		contrastValue;
	CString temp;

	GetDlgItemText(IDC_EDIT_BCONTRAST, temp);
	contrastValue = _ttoi(temp);
	int nRc = IBSU_SetContrast(_GetCurrentDeviceHandle(), contrastValue);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_SetContrast -- Return value (%d)", nRc);
}

void CNonCallbackFunctions::OnBnClickedCheckAutocontrast()
{
	UpdateData(TRUE);
}

void CNonCallbackFunctions::OnBnClickedCheckAutocapture()
{
	UpdateData(TRUE);
}

void CNonCallbackFunctions::OnBnClickedCheckTrigger()
{
	UpdateData(TRUE);
}

BOOL CNonCallbackFunctions::PreTranslateMessage(MSG* pMsg)
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
