// CaptureFunctions.cpp : implementation file
//

#include "stdafx.h"
#include "IBSU_NewFunctionTester.h"
#include "CaptureFunctions.h"
#include "MainFunction.h"
#include "IBSU_NewFunctionTesterDlg.h"
#include "ImageDlg.h"

// CCaptureFunctions dialog

IMPLEMENT_DYNAMIC(CCaptureFunctions, CDialog)

int e_devHandle;

CCaptureFunctions::CCaptureFunctions(CWnd* pParent /*=NULL*/)
	: CDialog(CCaptureFunctions::IDD, pParent)
	, m_chkAutoContrast(FALSE)
	, m_chkAutoCapture(FALSE)
	, m_chkIgnoreNumberofFinger(FALSE)
{

}

CCaptureFunctions::~CCaptureFunctions()
{
}

void CCaptureFunctions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_IMAGETYPE, m_cbImageType);
	DDX_Check(pDX, IDC_CHECK_AUTOCONTRAST, m_chkAutoContrast);
	DDX_Check(pDX, IDC_CHECK_AUTOCAPTURE, m_chkAutoCapture);
	DDX_Check(pDX, IDC_CHECK_TRIGGER, m_chkIgnoreNumberofFinger);
}


BEGIN_MESSAGE_MAP(CCaptureFunctions, CDialog)
//	ON_CBN_SELCHANGE(IDC_COMBO_IMAGETYPE, &CCaptureFunctions::OnCbnSelchangeComboImagetype)
ON_BN_CLICKED(IDC_BUTTON_BEGINCAPTURE, &CCaptureFunctions::OnBnClickedButtonBegincapture)
ON_BN_CLICKED(IDC_BUTTON_CANCELCAPTURE, &CCaptureFunctions::OnBnClickedButtonCancelcapture)
ON_BN_CLICKED(IDC_BUTTON_TAKERESULTIMAGE, &CCaptureFunctions::OnBnClickedButtonTakeresultimage)
ON_BN_CLICKED(IDC_BUTTON_ISCAPTUREAVAILABLE, &CCaptureFunctions::OnBnClickedButtonIscaptureavailable)
ON_BN_CLICKED(IDC_BUTTON_ISCAPTUREACTIVE, &CCaptureFunctions::OnBnClickedButtonIscaptureactive)
ON_BN_CLICKED(IDC_BUTTON_ISTOUCHFINGER, &CCaptureFunctions::OnBnClickedButtonIstouchfinger)
ON_BN_CLICKED(IDC_CHECK_AUTOCONTRAST, &CCaptureFunctions::OnBnClickedCheckAutocontrast)
ON_BN_CLICKED(IDC_CHECK_AUTOCAPTURE, &CCaptureFunctions::OnBnClickedCheckAutocapture)
ON_BN_CLICKED(IDC_CHECK_TRIGGER, &CCaptureFunctions::OnBnClickedCheckTrigger)
ON_BN_CLICKED(IDC_BUTTON_GETCONTRAST, &CCaptureFunctions::OnBnClickedButtonGetcontrast)
ON_BN_CLICKED(IDC_BUTTON_SETCONTRAST, &CCaptureFunctions::OnBnClickedButtonSetcontrast)
ON_WM_DESTROY()
END_MESSAGE_MAP()


// CCaptureFunctions message handlers

BOOL CCaptureFunctions::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_pMainDlg = (CIBSU_NewFunctionTesterDlg*)AfxGetMainWnd();

	m_cbImageType.AddString("ENUM_IBSU_TYPE_NONE");
	m_cbImageType.AddString("ENUM_IBSU_ROLL_SINGLE_FINGER");
	m_cbImageType.AddString("ENUM_IBSU_FLAT_SINGLE_FINGER");
	m_cbImageType.AddString("ENUM_IBSU_FLAT_TWO_FINGERS");
	m_cbImageType.AddString("ENUM_IBSU_FLAT_FOUR_FINGERS");
	m_cbImageType.AddString("ENUM_IBSU_FLAT_THREE_FINGERS");
	m_cbImageType.SetCurSel(2);

	GetDlgItem(IDC_EDIT_DEVICEHANDLE)->SetWindowText("0");
	GetDlgItem(IDC_EDIT_CONTRAST)->SetWindowText("21");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

int CCaptureFunctions::_GetCrrentDeviceHandleInCaptureFunction()
{ 
	int deviceHandle = GetDlgItemInt(IDC_EDIT_DEVICEHANDLE);
	//for image Size
	e_devHandle = deviceHandle;
	
	return deviceHandle;
}

void CCaptureFunctions::OnBnClickedButtonBegincapture()
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

	int nRc = IBSU_BeginCaptureImage(_GetCrrentDeviceHandleInCaptureFunction(), imgType, imgRes, captureOptions);
	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_BeginCaptureImage -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		GetDlgItem( IDC_BUTTON_TAKERESULTIMAGE )->EnableWindow( TRUE );
	}
}

void CCaptureFunctions::OnBnClickedButtonCancelcapture()
{
	int nRc = IBSU_CancelCaptureImage( _GetCrrentDeviceHandleInCaptureFunction() );
	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_CancelCaptureImage -- Return value (%d)", nRc);
}

void CCaptureFunctions::OnBnClickedButtonTakeresultimage()
{
	int nRc = IBSU_TakeResultImageManually(_GetCrrentDeviceHandleInCaptureFunction());
	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_TakeResultImageManually -- Return value (%d)", nRc);
}

void CCaptureFunctions::OnBnClickedButtonIscaptureavailable()
{
	UpdateData(FALSE);

	int		pos=0;
	const	IBSU_ImageType imgType = (IBSU_ImageType)m_cbImageType.GetCurSel();
	const	IBSU_ImageResolution imgRes = ENUM_IBSU_IMAGE_RESOLUTION_500;		// Currently we only support 500ppi.
	BOOL	modeAvailable;
	char	cValue[64];

	int nRc = IBSU_IsCaptureAvailable(_GetCrrentDeviceHandleInCaptureFunction(), imgType, imgRes, &modeAvailable);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_IsCaptureAvailable -- Return value (%d) ", nRc);

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
			m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Please select ImageType.");
			return;
		}

		if( !modeAvailable )
			m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "%s mode is not available", cValue);
		else
			m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "%s mode is available", cValue);
	}
}

void CCaptureFunctions::OnBnClickedButtonIscaptureactive()
{
	// TODO: Add your control notification handler code here
	int		pos=0;
	BOOL	IsActive;

	int nRc = IBSU_IsCaptureActive(_GetCrrentDeviceHandleInCaptureFunction(), &IsActive);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_IsCaptureActive -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		if( IsActive )
			m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Capturing...");
		else
			m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Not Capturing...");
	}

}

void CCaptureFunctions::OnBnClickedButtonIstouchfinger()
{
	// TODO: Add your control notification handler code here
	int		pos=0;
	int		touchInStatus;

	int nRc = IBSU_IsTouchedFinger(_GetCrrentDeviceHandleInCaptureFunction(), &touchInStatus);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_IsTouchedFinger -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		if( touchInStatus )
			m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Detected finger.");
		else
			m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Not detected finger.");
	}
}


void CCaptureFunctions::OnBnClickedCheckAutocontrast()
{
	UpdateData(TRUE);
}

void CCaptureFunctions::OnBnClickedCheckAutocapture()
{
	UpdateData(TRUE);
}

void CCaptureFunctions::OnBnClickedCheckTrigger()
{
	UpdateData(TRUE);
}

void CCaptureFunctions::OnBnClickedButtonGetcontrast()
{
	int		pos=0;
	int		contrastValue;


	int nRc = IBSU_GetContrast(_GetCrrentDeviceHandleInCaptureFunction(), &contrastValue);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_GetContrast -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "Contrast : %d", contrastValue);
	}
}

void CCaptureFunctions::OnBnClickedButtonSetcontrast()
{
	UpdateData(FALSE);

	int		contrastValue;
	CString temp;

	GetDlgItemText(IDC_EDIT_CONTRAST, temp);
	contrastValue = _ttoi(temp);
	int nRc = IBSU_SetContrast(_GetCrrentDeviceHandleInCaptureFunction(), contrastValue);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_SetContrast -- Return value (%d)", nRc);
}

BOOL CCaptureFunctions::PreTranslateMessage(MSG* pMsg)
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
