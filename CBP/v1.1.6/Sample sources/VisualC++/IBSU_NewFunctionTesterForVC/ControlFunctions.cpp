// ControlFunctions.cpp : implementation file
//

#include "stdafx.h"
#include "IBSU_NewFunctionTester.h"
#include "ControlFunctions.h"
#include "IBSU_NewFunctionTesterDlg.h"

// CControlFunctions dialog

IMPLEMENT_DYNAMIC(CControlFunctions, CDialog)

CControlFunctions::CControlFunctions(CWnd* pParent /*=NULL*/)
	: CDialog(CControlFunctions::IDD, pParent)
{

}

CControlFunctions::~CControlFunctions()
{
}

void CControlFunctions::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_OPERATIONMODE, m_cbOperationMode);
    DDX_Control(pDX, IDC_COMBO_BEEPPATTERN, m_cbBeepPattern);
}


BEGIN_MESSAGE_MAP(CControlFunctions, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GETLEOPERATIONMODE, &CControlFunctions::OnBnClickedButtonGetleoperationmode)
	ON_BN_CLICKED(IDC_BUTTON_SETLEOPERATIONMODE, &CControlFunctions::OnBnClickedButtonSetleoperationmode)
	ON_BN_CLICKED(IDC_BUTTON_GETOPERABLELEDS, &CControlFunctions::OnBnClickedButtonGetoperableleds)
	ON_BN_CLICKED(IDC_BUTTON_GETLEDS, &CControlFunctions::OnBnClickedButtonGetleds)
	ON_BN_CLICKED(IDC_BUTTON_SETLEDS, &CControlFunctions::OnBnClickedButtonSetleds)
    ON_BN_CLICKED(IDC_BUTTON_GETOPERABLEBEEPER, &CControlFunctions::OnBnClickedButtonGetoperablebeeper)
    ON_BN_CLICKED(IDC_BUTTON_SETBEEPER, &CControlFunctions::OnBnClickedButtonSetbeeper)
END_MESSAGE_MAP()


// CControlFunctions message handlers

BOOL CControlFunctions::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	m_pMainDlg = (CIBSU_NewFunctionTesterDlg*)AfxGetMainWnd();

	m_cbOperationMode.AddString("ENUM_IBSU_LE_OPERATION_AUTO");
	m_cbOperationMode.AddString("ENUM_IBSU_LE_OPERATION_ON");
	m_cbOperationMode.AddString("ENUM_IBSU_LE_OPERATION_OFF");
	m_cbOperationMode.SetCurSel(0);

	m_cbBeepPattern.AddString("ENUM_IBSU_BEEP_PATTERN_GENERIC");
	m_cbBeepPattern.AddString("ENUM_IBSU_BEEP_PATTERN_REPEAT");
	m_cbBeepPattern.SetCurSel(0);

    GetDlgItem(IDC_EDIT_DEVICEHANDLE)->SetWindowText("0");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

int CControlFunctions::_GetCurrentDeviceHandleInControlFunctions()
{
	int deviceHandle = GetDlgItemInt(IDC_EDIT_DEVICEHANDLE);
	return deviceHandle;
}

void CControlFunctions::OnBnClickedButtonGetleoperationmode()
{
	// TODO: Add your control notification handler code here
	UpdateData(FALSE);

	int		pos=0;
	int		leOperationMode = m_cbOperationMode.GetCurSel();
	char	cValue[32];

	int nRc = IBSU_GetLEOperationMode(_GetCurrentDeviceHandleInControlFunctions(), (IBSU_LEOperationMode*)&leOperationMode);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_GetLEOperationMode -- Return value (%d)", nRc);

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

		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "LE Mode : %s", cValue);
	}
}

void CControlFunctions::OnBnClickedButtonSetleoperationmode()
{
	// TODO: Add your control notification handler code here
	UpdateData(FALSE);

	int		pos=0;
	int		leOperationMode = m_cbOperationMode.GetCurSel();
	
	int nRc = IBSU_SetLEOperationMode(_GetCurrentDeviceHandleInControlFunctions(), (IBSU_LEOperationMode)leOperationMode);
	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_SetLEOperationMode -- Return value (%d)", nRc);
}

void CControlFunctions::OnBnClickedButtonGetoperableleds()
{
	// TODO: Add your control notification handler code here
	int		pos=0;
	int		ledCount;
	DWORD	operableLEDs;
	IBSU_LedType ledType;


	int nRc = IBSU_GetOperableLEDs(_GetCurrentDeviceHandleInControlFunctions(), &ledType, &ledCount, &operableLEDs);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_GetOperableLEDs -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "LedType : %d, LedCount : %d, Operable LEDs : 0x%08X",
				ledType, ledCount, operableLEDs);
	}
}

void CControlFunctions::OnBnClickedButtonGetleds()
{
	// TODO: Add your control notification handler code here
	int		pos=0;
	DWORD	activeLEDs = -1;


	int nRc = IBSU_GetLEDs(_GetCurrentDeviceHandleInControlFunctions(), &activeLEDs);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_GetLEDs -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "LED status : 0x%08X", activeLEDs);
	}
}

void CControlFunctions::OnBnClickedButtonSetleds()
{
	// TODO: Add your control notification handler code here
	UpdateData(FALSE);
	CString	temp;
	DWORD	activeLEDs;


	GetDlgItemText(IDC_EDIT_SETLED, temp);
	sscanf(temp, "%x", &activeLEDs);

	int nRc = IBSU_SetLEDs(_GetCurrentDeviceHandleInControlFunctions(), activeLEDs);
	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_SetLEDs -- Return value (%d)", nRc);
}

BOOL CControlFunctions::PreTranslateMessage(MSG* pMsg)
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

void CControlFunctions::OnBnClickedButtonGetoperablebeeper()
{
	// TODO: Add your control notification handler code here
	int		pos=0;
	IBSU_BeeperType beeperType;
	char	cValue[32];


	int nRc = IBSU_GetOperableBeeper(_GetCurrentDeviceHandleInControlFunctions(), &beeperType);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_GetOperableBeeper -- Return value (%d)", nRc);

	if( nRc == IBSU_STATUS_OK )
	{
		if( beeperType == ENUM_IBSU_BEEPER_TYPE_NONE )
			sprintf(cValue, "None");
		else if( beeperType == ENUM_IBSU_BEEPER_TYPE_MONOTONE )
			sprintf(cValue, "Monotone");
		else
			sprintf(cValue, "Unknown");

		m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "beeperType : %s", cValue);
	}
}

void CControlFunctions::OnBnClickedButtonSetbeeper()
{
    // TODO: Add your control notification handler code here
	UpdateData(FALSE);

	int		beepPattern = m_cbBeepPattern.GetCurSel();
	CString	temp;
	DWORD	soundTone, duration, reserved_1, reserved_2;
	
	GetDlgItemText(IDC_EDIT_SETBEEPER1, temp);
	sscanf(temp, "%d", &soundTone);
	GetDlgItemText(IDC_EDIT_SETBEEPER2, temp);
	sscanf(temp, "%d", &duration);
	GetDlgItemText(IDC_EDIT_SETBEEPER3, temp);
	sscanf(temp, "%d", &reserved_1);
	GetDlgItemText(IDC_EDIT_SETBEEPER4, temp);
	sscanf(temp, "%d", &reserved_2);

	int nRc = IBSU_SetBeeper(_GetCurrentDeviceHandleInControlFunctions(), (IBSU_BeepPattern)beepPattern, soundTone, duration, reserved_1, reserved_2);
	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_SetBeeper -- Return value (%d)", nRc);
}
