// ImageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IBSU_NewFunctionTester.h"
#include "ImageDlg.h"
#include "CaptureFunctions.h"


// CImageDlg dialog

extern int e_devHandle;

IMPLEMENT_DYNAMIC(CImageDlg, CDialog)

CImageDlg::CImageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImageDlg::IDD, pParent)
{


}

CImageDlg::~CImageDlg()
{
}

void CImageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VIEW, m_view);
}


BEGIN_MESSAGE_MAP(CImageDlg, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CImageDlg message handlers

void CImageDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if(m_view.GetSafeHwnd()!=NULL)
	{
		//m_view.SetWindowPos(NULL, 10,10, cx-20, cy-20, SWP_NOZORDER);
/*
		IBSU_DeviceDesc devDesc;
		int devWidth = 800, devHeight = 750;

		devDesc.productName[0] = 0;

		IBSU_GetDeviceDescription(e_devHandle, &devDesc);

		if( ( _stricmp( devDesc.productName, "WATSON" )			== 0) ||
			( _stricmp( devDesc.productName, "WATSON MINI" )	== 0) ||
			( _stricmp( devDesc.productName, "SHERLOCK_ROIC" )	== 0) ||
			( _stricmp( devDesc.productName, "SHERLOCK" )		== 0) )
		{
			devWidth = 800;
			devHeight = 750;
		}
		else if( ( _stricmp( devDesc.productName, "COLUMBO" )		== 0) )
		{
			devWidth = 400;
			devHeight = 500;
		}
		else if( ( _stricmp( devDesc.productName, "CURVE" )			== 0) )
		{
			devWidth = 288;
			devHeight = 352;
		}

		if( devWidth < cx )
			cx = devWidth;
		if( devHeight < cy )
			cy = devHeight;
		if(352 > cx)
			cx = 352;
		if(288 > cy)
			cy = 288;
		
*/

		//m_view.SetWindowPos(NULL, 10,10, cx-20, cy-20, SWP_NOREPOSITION);
		m_view.SetWindowPos(NULL, 0,0, cx, cy, SWP_NOREPOSITION);

		CWnd *disWnd = GetDlgItem(IDC_STATIC_VIEW);
		RECT clientRect;
		disWnd->GetClientRect( &clientRect );

		IBSU_CreateClientWindow(e_devHandle, disWnd->m_hWnd, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);

		/*
		CWnd *disWnd = m_imageDlg->GetDlgItem( IDC_STATIC_VIEW );
		RECT clientRect;
		disWnd->GetClientRect( &clientRect );

		int nRc = IBSU_CreateClientWindow(_GetCrrentDeviceHandleInCaptureFunction(), disWnd->m_hWnd, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
		*/
	}
	// TODO: Add your message handler code here
}

BOOL CImageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	//SetWindowPos(NULL, 0,0, 300, 320, SWP_NOMOVE);

	m_view.SetWindowPos(NULL, 0,0,288,352,SWP_NOMOVE);

//	m_functionsDlg = new CCaptureFunctions(this);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
