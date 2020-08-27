// ClientWindowFunctions.cpp : implementation file
//

#include "stdafx.h"
#include "IBSU_NewFunctionTester.h"
#include "ClientWindowFunctions.h"
#include "IBSU_NewFunctionTesterDlg.h"


// CClientWindowFunctions dialog

IMPLEMENT_DYNAMIC(CClientWindowFunctions, CDialog)

CClientWindowFunctions::CClientWindowFunctions(CWnd* pParent /*=NULL*/)
	: CDialog(CClientWindowFunctions::IDD, pParent)
	, m_chkUseClientWindow(FALSE)
	, m_chkGuideLineValue(FALSE)
	, m_chkShowOverlayObject(TRUE)
	, m_chkShowAllOverlayObject(TRUE)
	, m_chkInvalidArea(FALSE)
{

}

CClientWindowFunctions::~CClientWindowFunctions()
{
}

void CClientWindowFunctions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_CLIENT_WINDOW, m_chkUseClientWindow);
	DDX_Check(pDX, IDC_CHECK_GUIDELINE, m_chkGuideLineValue);
	DDX_Check(pDX, IDC_CHECK_SHOWOVERLAYOBJECT, m_chkShowOverlayObject);
	DDX_Check(pDX, IDC_CHECK_SHOWALLOVERLAYOBJECT, m_chkShowAllOverlayObject);
	DDX_Check(pDX, IDC_CHECK_INVALIDAREA, m_chkInvalidArea);
	DDX_Control(pDX, IDC_COMBO_ROLL_GUIDE_LINE_WIDTH, m_cbRollGuideLineWidth);
}


BEGIN_MESSAGE_MAP(CClientWindowFunctions, CDialog)
ON_BN_CLICKED(IDC_BUTTON_SETBACKGROUNDCOLOR, &CClientWindowFunctions::OnBnClickedButtonSetbackgroundcolor)
ON_BN_CLICKED(IDC_BUTTON_SETCLIENTWINDOW, &CClientWindowFunctions::OnBnClickedButtonSetclientwindow)
ON_BN_CLICKED(IDC_CHECK_CLIENT_WINDOW, &CClientWindowFunctions::OnBnClickedCheckClientWindow)
ON_BN_CLICKED(IDC_CHECK_GUIDELINE, &CClientWindowFunctions::OnBnClickedCheckGuideline)
//ON_BN_CLICKED(IDC_BUTTON_SETCLIENTWINDOWOVERLAYTEXT, &CClientWindowFunctions::OnBnClickedButtonSetclientwindowoverlaytext)
ON_BN_CLICKED(IDC_BUTTON_ADDOVERLAYTEXT, &CClientWindowFunctions::OnBnClickedButtonAddoverlaytext)
//ON_BN_CLICKED(IDC_BUTTON_SHOWOVERLAYOBJECT, &CClientWindowFunctions::OnBnClickedButtonShowoverlayobject)
ON_BN_CLICKED(IDC_CHECK_SHOWOVERLAYOBJECT, &CClientWindowFunctions::OnBnClickedCheckShowoverlayobject)
ON_BN_CLICKED(IDC_CHECK_SHOWALLOVERLAYOBJECT, &CClientWindowFunctions::OnBnClickedCheckShowalloverlayobject)
ON_BN_CLICKED(IDC_BUTTON_REMOVEOVERLAYOBJECT, &CClientWindowFunctions::OnBnClickedButtonRemoveoverlayobject)
ON_BN_CLICKED(IDC_BUTTON_REMOVEALLOVERLAYOBJECT, &CClientWindowFunctions::OnBnClickedButtonRemovealloverlayobject)
ON_BN_CLICKED(IDC_BUTTON_MODIFYOVERLAYTEXT, &CClientWindowFunctions::OnBnClickedButtonModifyoverlaytext)
ON_BN_CLICKED(IDC_BUTTON_ADDOVERLAYLINE, &CClientWindowFunctions::OnBnClickedButtonAddoverlayline)
ON_BN_CLICKED(IDC_BUTTON_MODIFYOVERLAYLINE, &CClientWindowFunctions::OnBnClickedButtonModifyoverlayline)
ON_BN_CLICKED(IDC_BUTTON_ADDOVERLAYQUADRANGLE, &CClientWindowFunctions::OnBnClickedButtonAddoverlayquadrangle)
ON_BN_CLICKED(IDC_BUTTON_MODIFYOVERLAYQUADRANGLE, &CClientWindowFunctions::OnBnClickedButtonModifyoverlayquadrangle)
//ON_BN_CLICKED(IDC_BUTTON_ADDOVERLAYSHAPE, &CClientWindowFunctions::OnBnClickedButtonAddoverlayshape)
ON_BN_CLICKED(IDC_CHECK_INVALIDAREA, &CClientWindowFunctions::OnBnClickedCheckInvalidarea)
ON_CBN_SELCHANGE(IDC_COMBO_ROLL_GUIDE_LINE_WIDTH, &CClientWindowFunctions::OnCbnSelchangeComboRollGuideLineWidth)
END_MESSAGE_MAP()


// CClientWindowFunctions message handlers

BOOL CClientWindowFunctions::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_pMainDlg = (CIBSU_NewFunctionTesterDlg*)AfxGetMainWnd();

	GetDlgItem( IDC_BUTTON_SETBACKGROUNDCOLOR )->EnableWindow( FALSE );
	GetDlgItem( IDC_CHECK_GUIDELINE )->EnableWindow( FALSE );
	GetDlgItem( IDC_CHECK_INVALIDAREA )->EnableWindow( FALSE );
	GetDlgItem( IDC_CHECK_SHOWOVERLAYOBJECT )->EnableWindow( FALSE );
	GetDlgItem( IDC_CHECK_SHOWALLOVERLAYOBJECT )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_SETCLIENTWINDOWOVERLAYTEXT )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_OVERLAYHANDLE )->EnableWindow( FALSE );

	GetDlgItem( IDC_STATIC_LINEWIDTH )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_LINEWIDTH )->EnableWindow( FALSE );

	GetDlgItem( IDC_STATIC_POSX )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_POSX )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_POSY )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_POSY )->EnableWindow( FALSE );
	
	GetDlgItem( IDC_STATIC_POSX2 )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_POSX2 )->EnableWindow(FALSE);
	GetDlgItem( IDC_STATIC_POSY2 )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_POSY2 )->EnableWindow(FALSE);
	
	GetDlgItem( IDC_STATIC_POSX3 )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_POSX3 )->EnableWindow(FALSE);
	GetDlgItem( IDC_STATIC_POSY3 )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_POSY3 )->EnableWindow(FALSE);
	
	GetDlgItem( IDC_STATIC_POSX4 )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_POSX4 )->EnableWindow(FALSE);
	GetDlgItem( IDC_STATIC_POSY4 )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_POSY4 )->EnableWindow(FALSE);
	
	GetDlgItem( IDC_STATIC_TEXTCOLOR )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_RED )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_GREEN )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_BLUE )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_TEXTRED )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_TEXTGREEN )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_TEXTBLUE )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_TEXTRANGE )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_ADDTEXT )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_ADDTEXT )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_BKRED )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_BKGREEN )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATIC_BKBLUE )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_BKRED )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_BKGREEN )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_BKBLUE )->EnableWindow( FALSE );

	GetDlgItem( IDC_BUTTON_REMOVEOVERLAYOBJECT )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_ADDOVERLAYTEXT )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_REMOVEALLOVERLAYOBJECT )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_MODIFYOVERLAYTEXT )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_ADDOVERLAYLINE )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_ADDOVERLAYQUADRANGLE )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_MODIFYOVERLAYQUADRANGLE )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_ADDOVERLAYSHAPE )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_MODIFYOVERLAYSHAPE )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_MODIFYOVERLAYLINE )->EnableWindow( FALSE );

	//Input value to EditBox
	GetDlgItem( IDC_EDIT_DEVICEHANDLE )->SetWindowText("0");
	GetDlgItem( IDC_EDIT_SETLED )->SetWindowText("FFFFFFFF");
	GetDlgItem( IDC_EDIT_OVERLAYHANDLE )->SetWindowText("0");
	GetDlgItem( IDC_EDIT_LINEWIDTH )->SetWindowText("10");
	GetDlgItem( IDC_EDIT_POSX )->SetWindowText("10");
	GetDlgItem( IDC_EDIT_POSY )->SetWindowText("10");
	GetDlgItem( IDC_EDIT_POSX2 )->SetWindowText("100");
	GetDlgItem( IDC_EDIT_POSY2 )->SetWindowText("10");
	GetDlgItem( IDC_EDIT_POSX3 )->SetWindowText("100");
	GetDlgItem( IDC_EDIT_POSY3 )->SetWindowText("100");
	GetDlgItem( IDC_EDIT_POSX4 )->SetWindowText("10");
	GetDlgItem( IDC_EDIT_POSY4 )->SetWindowText("100");
	GetDlgItem( IDC_EDIT_TEXTRED )->SetWindowText("0");
	GetDlgItem( IDC_EDIT_TEXTGREEN )->SetWindowText("0");
	GetDlgItem( IDC_EDIT_TEXTBLUE )->SetWindowText("255");
	GetDlgItem( IDC_EDIT_BKRED )->SetWindowText("128");
	GetDlgItem( IDC_EDIT_BKGREEN )->SetWindowText("128");
	GetDlgItem( IDC_EDIT_BKBLUE )->SetWindowText("128");
	GetDlgItem( IDC_EDIT_BKBLUE )->SetWindowText("128");
	GetDlgItem( IDC_EDIT_ADDTEXT )->SetWindowText("Default Text");

	m_cbRollGuideLineWidth.AddString("1 pixel");
	m_cbRollGuideLineWidth.AddString("2 pixels");
	m_cbRollGuideLineWidth.AddString("3 pixels");
	m_cbRollGuideLineWidth.AddString("4 pixels");
	m_cbRollGuideLineWidth.AddString("5 pixels");
	m_cbRollGuideLineWidth.AddString("6 pixels");
	m_cbRollGuideLineWidth.SetCurSel(1);


	m_nOverlayHandle = -1;

	return TRUE;  // return TRUE unless you set the focus to a control
}

int CClientWindowFunctions::_GetCrrentDeviceHandle()
{ 
	int deviceHandle = GetDlgItemInt(IDC_EDIT_DEVICEHANDLE);
	
	return deviceHandle;
}

void CClientWindowFunctions::OnBnClickedButtonSetbackgroundcolor()
{
	// TODO: Add your control notification handler code here
	DWORD bkRed = GetDlgItemInt(IDC_EDIT_BKRED);
	DWORD bkGreen = GetDlgItemInt(IDC_EDIT_BKGREEN);
	DWORD bkBlue = GetDlgItemInt(IDC_EDIT_BKBLUE);

	COLORREF	cr = RGB(bkRed, bkGreen, bkBlue);

	CString temp;
	temp.Format("%d",cr);

	int nRc = IBSU_SetClientDisplayProperty(_GetCrrentDeviceHandle(), ENUM_IBSU_WINDOW_PROPERTY_BK_COLOR, temp);
	 

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_SetClientDisplayProperty -- Return value (%d)", nRc);

}

void CClientWindowFunctions::OnBnClickedCheckClientWindow()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if( m_chkUseClientWindow )
	{
		GetDlgItem( IDC_BUTTON_SETBACKGROUNDCOLOR )->EnableWindow( TRUE );
		GetDlgItem( IDC_CHECK_GUIDELINE )->EnableWindow( TRUE );
		GetDlgItem( IDC_CHECK_INVALIDAREA )->EnableWindow( TRUE );
		GetDlgItem( IDC_BUTTON_SETCLIENTWINDOW )->EnableWindow( TRUE );
		//GetDlgItem( IDC_STATIC_BKRANGE )->EnableWindow( TRUE );
		GetDlgItem( IDC_CHECK_SHOWOVERLAYOBJECT )->EnableWindow( TRUE );
		GetDlgItem( IDC_CHECK_SHOWALLOVERLAYOBJECT )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_POSX )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_POSX )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_POSY )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_POSY )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_POSX2 )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_POSX2 )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_POSY2 )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_POSY2 )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_POSX3 )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_POSX3 )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_POSY3 )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_POSY3 )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_POSX4 )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_POSX4 )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_POSY4 )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_POSY4 )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_OVERLAYHANDLE )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_OVERLAYHANDLE )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_LINEWIDTH )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_LINEWIDTH )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_TEXTRED )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_RED )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_GREEN )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_BLUE )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_TEXTGREEN )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_TEXTBLUE )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_ADDTEXT )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_ADDTEXT )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_BKRED )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_BKGREEN )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATIC_BKBLUE )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_BKRED )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_BKGREEN )->EnableWindow( TRUE );
		GetDlgItem( IDC_EDIT_BKBLUE )->EnableWindow( TRUE );
		GetDlgItem( IDC_BUTTON_REMOVEOVERLAYOBJECT )->EnableWindow( TRUE );
		GetDlgItem( IDC_BUTTON_ADDOVERLAYTEXT )->EnableWindow( TRUE );
		GetDlgItem( IDC_BUTTON_REMOVEALLOVERLAYOBJECT )->EnableWindow( TRUE );
		GetDlgItem( IDC_BUTTON_MODIFYOVERLAYTEXT )->EnableWindow( TRUE );
		GetDlgItem( IDC_BUTTON_ADDOVERLAYLINE )->EnableWindow( TRUE );
		GetDlgItem( IDC_BUTTON_ADDOVERLAYQUADRANGLE )->EnableWindow( TRUE );
		GetDlgItem( IDC_BUTTON_MODIFYOVERLAYQUADRANGLE )->EnableWindow( TRUE );
		GetDlgItem( IDC_BUTTON_ADDOVERLAYSHAPE )->EnableWindow( TRUE );
		GetDlgItem( IDC_BUTTON_MODIFYOVERLAYSHAPE )->EnableWindow( TRUE );
		GetDlgItem( IDC_BUTTON_MODIFYOVERLAYLINE )->EnableWindow( TRUE );
		
		CWnd *disWnd = m_pMainDlg->m_imageDlg->GetDlgItem( IDC_STATIC_VIEW );
		RECT clientRect;
		disWnd->GetClientRect( &clientRect );

		int nRc = IBSU_CreateClientWindow(_GetCrrentDeviceHandle(), disWnd->m_hWnd, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
		m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_CreateClientWindow -- Return value (%d)", nRc);
	}
	else
	{
		GetDlgItem( IDC_BUTTON_SETBACKGROUNDCOLOR )->EnableWindow( FALSE );
		GetDlgItem( IDC_CHECK_GUIDELINE )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_SETCLIENTWINDOW )->EnableWindow( FALSE );
		GetDlgItem( IDC_CHECK_INVALIDAREA )->EnableWindow( FALSE );
		GetDlgItem( IDC_CHECK_SHOWOVERLAYOBJECT )->EnableWindow( FALSE );
		GetDlgItem( IDC_CHECK_SHOWALLOVERLAYOBJECT )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_POSX )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_POSX )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_POSY )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_POSY )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_POSX2 )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_POSX2 )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_POSY2 )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_POSY2 )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_POSX3 )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_POSX3 )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_POSY3 )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_POSY3 )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_POSX4 )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_POSX4 )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_POSY4 )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_POSY4 )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_OVERLAYHANDLE )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_OVERLAYHANDLE )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_LINEWIDTH )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_LINEWIDTH )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_TEXTCOLOR )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_TEXTRED )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_RED )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_GREEN )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_BLUE )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_TEXTGREEN )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_TEXTBLUE )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_ADDTEXT )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_ADDTEXT )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_BKRED )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_BKGREEN )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATIC_BKBLUE )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_BKRED )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_BKGREEN )->EnableWindow( FALSE );
		GetDlgItem( IDC_EDIT_BKBLUE )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_REMOVEOVERLAYOBJECT )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_ADDOVERLAYTEXT )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_REMOVEALLOVERLAYOBJECT )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_MODIFYOVERLAYTEXT )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_ADDOVERLAYLINE )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_ADDOVERLAYQUADRANGLE )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_MODIFYOVERLAYQUADRANGLE )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_ADDOVERLAYSHAPE )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_MODIFYOVERLAYSHAPE )->EnableWindow( FALSE );
		GetDlgItem( IDC_BUTTON_MODIFYOVERLAYLINE )->EnableWindow( FALSE );

		int nRc = IBSU_DestroyClientWindow(_GetCrrentDeviceHandle(), FALSE);
		m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_DestroyClientWindow -- Return value (%d)", nRc);
	}

}

void CClientWindowFunctions::OnBnClickedButtonSetclientwindow()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	int posX = GetDlgItemInt(IDC_EDIT_POSX);
	int posY = GetDlgItemInt(IDC_EDIT_POSY);
	int bkTextRed = GetDlgItemInt(IDC_EDIT_TEXTRED);
	int bkTextGreen = GetDlgItemInt(IDC_EDIT_TEXTGREEN);
	int bkTextBlue = GetDlgItemInt(IDC_EDIT_TEXTBLUE);

	int			font_size = 10;
	COLORREF	cr = RGB(bkTextRed, bkTextGreen, bkTextBlue);
	CString		temp;

	GetDlgItemText(IDC_EDIT_ADDTEXT, temp);

	//int nRc = IBSU_SetClientWindowOverlayText(_GetCrrentDeviceHandle(), "Arial", font_size, FALSE, temp.GetBuffer(0), posX, posY, (DWORD)cr );

	int overlayHandle = 0;
	//IBSU_AddOverlayText(m_nDevHandle, &m_nOvImageTextHandle, "Arial", 10, TRUE/*bold*/, "", 10, 10, (DWORD)0);
	int nRc = IBSU_AddOverlayText(_GetCrrentDeviceHandle(), &overlayHandle, "Arial", 10, TRUE, "TEST",10, 10, (DWORD)0);

	IBSU_ShowOverlayObject(_GetCrrentDeviceHandle(), overlayHandle, TRUE);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_SetClientWindowOverlayText -- Return value (%d)", nRc);
}

void CClientWindowFunctions::OnBnClickedCheckGuideline()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	int nRc;

	if( m_chkGuideLineValue )
	{
		nRc = IBSU_SetClientDisplayProperty( _GetCrrentDeviceHandle(), ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE, "TRUE" );
	}
	else
	{
		nRc = IBSU_SetClientDisplayProperty( _GetCrrentDeviceHandle(), ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE, "FALSE" );
	}

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_SetClientDisplayProperty -- Return value (%d)", nRc);
}

/*void CClientWindowFunctions::OnBnClickedButtonSetclientwindowoverlaytext()
{
	// TODO: Add your control notification handler code here
}*/

void CClientWindowFunctions::OnBnClickedButtonAddoverlaytext()
{
	// TODO: Add your control notification handler code here
	int posX = GetDlgItemInt(IDC_EDIT_POSX);
	int posY = GetDlgItemInt(IDC_EDIT_POSY);
	int bkTextRed = GetDlgItemInt(IDC_EDIT_TEXTRED);
	int bkTextGreen = GetDlgItemInt(IDC_EDIT_TEXTGREEN);
	int bkTextBlue = GetDlgItemInt(IDC_EDIT_TEXTBLUE);

	int			font_size = 10;
	COLORREF	cr = RGB(bkTextRed, bkTextGreen, bkTextBlue);
	CString		temp;

	int pos = 0;

	GetDlgItemText(IDC_EDIT_ADDTEXT, temp);

	int nRc = IBSU_AddOverlayText(_GetCrrentDeviceHandle(), &m_nOverlayHandle, "Arial", font_size, FALSE, temp.GetBuffer(0) ,posX, posY, (DWORD)cr);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_AddOverlayText -- Return value (%d)", nRc);
	m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "OverlayHandle handle : %d", m_nOverlayHandle);

}

//void CClientWindowFunctions::OnBnClickedButtonShowoverlayobject()
//{
//	// TODO: Add your control notification handler code here
//	int m_nOverlayHandle = GetDlgItemInt(IDC_EDIT_OVERLAYHANDLE);
//
//	int nRc = IBSU_ShowOverlayObject(_GetCrrentDeviceHandle(), overlayHandle, TRUE);
//}

void CClientWindowFunctions::OnBnClickedCheckShowoverlayobject()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	m_nOverlayHandle = GetDlgItemInt(IDC_EDIT_OVERLAYHANDLE);
	int nRc = -1;

	if( m_chkShowOverlayObject )
	{
		nRc = IBSU_ShowOverlayObject(_GetCrrentDeviceHandle(), m_nOverlayHandle, TRUE);
	}
	else
	{
		nRc = IBSU_ShowOverlayObject(_GetCrrentDeviceHandle(), m_nOverlayHandle, FALSE);
	}

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_ShowOverlayObject -- Return value (%d)", nRc);

}

void CClientWindowFunctions::OnBnClickedCheckShowalloverlayobject()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	int nRc = -1;

	if( m_chkShowAllOverlayObject )
	{
		nRc = IBSU_ShowAllOverlayObject(_GetCrrentDeviceHandle(), TRUE);
	}
	else
	{
		nRc = IBSU_ShowAllOverlayObject(_GetCrrentDeviceHandle(), FALSE);
	}

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_ShowAllOverlayObject -- Return value (%d)", nRc);
}

void CClientWindowFunctions::OnBnClickedButtonRemoveoverlayobject()
{
	// TODO: Add your control notification handler code here
	int nRc = -1;
	m_nOverlayHandle = GetDlgItemInt(IDC_EDIT_OVERLAYHANDLE);

	nRc = IBSU_RemoveOverlayObject(_GetCrrentDeviceHandle(), m_nOverlayHandle);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_RemoveOverlayObject -- Return value (%d)", nRc);
}

void CClientWindowFunctions::OnBnClickedButtonRemovealloverlayobject()
{
	// TODO: Add your control notification handler code here
	int nRc = -1;

	nRc = IBSU_RemoveAllOverlayObject(_GetCrrentDeviceHandle());

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_RemoveAllOverlayObject -- Return value (%d)", nRc);
}

void CClientWindowFunctions::OnBnClickedButtonModifyoverlaytext()
{
	// TODO: Add your control notification handler code here
	m_nOverlayHandle = GetDlgItemInt(IDC_EDIT_OVERLAYHANDLE);

	int posX = GetDlgItemInt(IDC_EDIT_POSX);
	int posY = GetDlgItemInt(IDC_EDIT_POSY);
	int bkTextRed = GetDlgItemInt(IDC_EDIT_TEXTRED);
	int bkTextGreen = GetDlgItemInt(IDC_EDIT_TEXTGREEN);
	int bkTextBlue = GetDlgItemInt(IDC_EDIT_TEXTBLUE);

	int			font_size = 10;
	COLORREF	cr = RGB(bkTextRed, bkTextGreen, bkTextBlue);
	CString		temp;

	GetDlgItemText(IDC_EDIT_ADDTEXT, temp);


	int nRc = IBSU_ModifyOverlayText(_GetCrrentDeviceHandle(), m_nOverlayHandle, "bold", font_size, FALSE, temp.GetBuffer(0) ,posX, posY, (DWORD)cr);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_ModifyOverlayText -- Return value (%d)", nRc);
}

void CClientWindowFunctions::OnBnClickedButtonAddoverlayline()
{
	// TODO: Add your control notification handler code here
	

	int posX = GetDlgItemInt(IDC_EDIT_POSX);
	int posY = GetDlgItemInt(IDC_EDIT_POSY);
	int posX2 = GetDlgItemInt(IDC_EDIT_POSX2);
	int posY2 = GetDlgItemInt(IDC_EDIT_POSY2);
	int lineWidth = GetDlgItemInt(IDC_EDIT_LINEWIDTH);

	int bkTextRed = GetDlgItemInt(IDC_EDIT_TEXTRED);
	int bkTextGreen = GetDlgItemInt(IDC_EDIT_TEXTGREEN);
	int bkTextBlue = GetDlgItemInt(IDC_EDIT_TEXTBLUE);

	COLORREF cr = RGB(bkTextRed, bkTextGreen, bkTextBlue);

	int pos = 0;

	int nRc = IBSU_AddOverlayLine(_GetCrrentDeviceHandle(), &m_nOverlayHandle, posX, posY, posX2, posY2, lineWidth, (DWORD)cr);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_AddOverlayLine -- Return value (%d)", nRc);
	m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "OverlayHandle handle : %d", m_nOverlayHandle);

}

void CClientWindowFunctions::OnBnClickedButtonModifyoverlayline()
{
	// TODO: Add your control notification handler code here
	m_nOverlayHandle = GetDlgItemInt(IDC_EDIT_OVERLAYHANDLE);

	int posX = GetDlgItemInt(IDC_EDIT_POSX);
	int posY = GetDlgItemInt(IDC_EDIT_POSY);
	int posX2 = GetDlgItemInt(IDC_EDIT_POSX2);
	int posY2 = GetDlgItemInt(IDC_EDIT_POSY2);
	int lineWidth = GetDlgItemInt(IDC_EDIT_LINEWIDTH);

	int bkTextRed = GetDlgItemInt(IDC_EDIT_TEXTRED);
	int bkTextGreen = GetDlgItemInt(IDC_EDIT_TEXTGREEN);
	int bkTextBlue = GetDlgItemInt(IDC_EDIT_TEXTBLUE);

	COLORREF cr = RGB(bkTextRed, bkTextGreen, bkTextBlue);

	int nRc = IBSU_ModifyOverlayLine(_GetCrrentDeviceHandle(), m_nOverlayHandle, posX, posY, posX2, posY2, lineWidth, (DWORD)cr);

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_ModifyOverlayLine -- Return value (%d)", nRc);

}

void CClientWindowFunctions::OnBnClickedButtonAddoverlayquadrangle()
{
	// TODO: Add your control notification handler code here
	int posX = GetDlgItemInt(IDC_EDIT_POSX);
	int posY = GetDlgItemInt(IDC_EDIT_POSY);
	int posX2 = GetDlgItemInt(IDC_EDIT_POSX2);
	int posY2 = GetDlgItemInt(IDC_EDIT_POSY2);
	int posX3 = GetDlgItemInt(IDC_EDIT_POSX3);
	int posY3 = GetDlgItemInt(IDC_EDIT_POSY3);
	int posX4 = GetDlgItemInt(IDC_EDIT_POSX4);
	int posY4 = GetDlgItemInt(IDC_EDIT_POSY4);

	int lineWidth = GetDlgItemInt(IDC_EDIT_LINEWIDTH);

	int bkTextRed = GetDlgItemInt(IDC_EDIT_TEXTRED);
	int bkTextGreen = GetDlgItemInt(IDC_EDIT_TEXTGREEN);
	int bkTextBlue = GetDlgItemInt(IDC_EDIT_TEXTBLUE);

	COLORREF cr = RGB(bkTextRed, bkTextGreen, bkTextBlue);

	int pos = 0;

	int nRc = IBSU_AddOverlayQuadrangle(_GetCrrentDeviceHandle(), &m_nOverlayHandle, posX, posY, posX2, posY2, posX3, posY3, posX4, posY4,
		lineWidth, (DWORD)cr);



	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_AddOverlayQuadrangle -- Return value (%d)", nRc);
	m_pMainDlg->_SetAdditionalMessageOnListView(pos++, "OverlayHandle handle : %d", m_nOverlayHandle);

}

void CClientWindowFunctions::OnBnClickedButtonModifyoverlayquadrangle()
{
	// TODO: Add your control notification handler code here
	m_nOverlayHandle = GetDlgItemInt(IDC_EDIT_OVERLAYHANDLE);

	int posX = GetDlgItemInt(IDC_EDIT_POSX);
	int posY = GetDlgItemInt(IDC_EDIT_POSY);
	int posX2 = GetDlgItemInt(IDC_EDIT_POSX2);
	int posY2 = GetDlgItemInt(IDC_EDIT_POSY2);
	int posX3 = GetDlgItemInt(IDC_EDIT_POSX3);
	int posY3 = GetDlgItemInt(IDC_EDIT_POSY3);
	int posX4 = GetDlgItemInt(IDC_EDIT_POSX4);
	int posY4 = GetDlgItemInt(IDC_EDIT_POSY4);

	int lineWidth = GetDlgItemInt(IDC_EDIT_LINEWIDTH);

	int bkTextRed = GetDlgItemInt(IDC_EDIT_TEXTRED);
	int bkTextGreen = GetDlgItemInt(IDC_EDIT_TEXTGREEN);
	int bkTextBlue = GetDlgItemInt(IDC_EDIT_TEXTBLUE);

	COLORREF cr = RGB(bkTextRed, bkTextGreen, bkTextBlue);

	int nRc = IBSU_ModifyOverlayQuadrangle(_GetCrrentDeviceHandle(), m_nOverlayHandle, posX, posY, posX2, posY2, posX3, posY3, posX4, posY4,
		lineWidth, (DWORD)cr);




	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_ModifyOverlayQuadrangle -- Return value (%d)", nRc);

}


BOOL CClientWindowFunctions::PreTranslateMessage(MSG* pMsg)
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

//void CClientWindowFunctions::OnBnClickedButtonAddoverlayshape()
//{
	// TODO: Add your control notification handler code here
//}

void CClientWindowFunctions::OnBnClickedCheckInvalidarea()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	int nRc;

	if( m_chkInvalidArea )
	{
		nRc = IBSU_SetClientDisplayProperty(_GetCrrentDeviceHandle(), ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA, "TRUE");

	}
	else
	{
		nRc = IBSU_SetClientDisplayProperty(_GetCrrentDeviceHandle(), ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA, "FALSE");
	}

	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_SetClientDisplayProperty -- Return value (%d)", nRc);
}

void CClientWindowFunctions::OnCbnSelchangeComboRollGuideLineWidth()
{
	UpdateData(FALSE);

	int		pos=0;
	int		rollGuideLineWidth = m_cbRollGuideLineWidth.GetCurSel() + 1;
	CString	propertyValue;
	
	propertyValue.Format("%d", rollGuideLineWidth);
	int nRc = IBSU_SetClientDisplayProperty(_GetCrrentDeviceHandle(), ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE_WIDTH, propertyValue);
	m_pMainDlg->_SetResultMessageOnListView(m_pMainDlg->m_IsAppend, "-- IBSU_SetClientDisplayProperty -- Return value (%d)", nRc);
}
