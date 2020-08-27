// IBScanUltimate_SimpleScanWMSplashDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IBSU_SimpleScanWM.h"
#include "IBSU_SimpleScanWMSplashDlg.h"

#include "IBScanUltimateApi.h"

#define TIMER_SPLASH_SCREEN_ID			2
#define TIMER_SPLASH_SCREEN_DELAY		5000 // 5 Seconds


// CIBScanUltimate_SimpleScanWMSplashDlg dialog

IMPLEMENT_DYNAMIC(CIBScanUltimate_SimpleScanWMSplashDlg, CDialog)

CIBScanUltimate_SimpleScanWMSplashDlg::CIBScanUltimate_SimpleScanWMSplashDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIBScanUltimate_SimpleScanWMSplashDlg::IDD, pParent)
{

}

CIBScanUltimate_SimpleScanWMSplashDlg::~CIBScanUltimate_SimpleScanWMSplashDlg()
{
}

void CIBScanUltimate_SimpleScanWMSplashDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CIBScanUltimate_SimpleScanWMSplashDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()



BOOL CIBScanUltimate_SimpleScanWMSplashDlg::OnInitDialog()
{
	IBSU_SdkVersion verInfo;
	RECT rectWorkArea;

	CDialog::OnInitDialog();

	IBSU_GetSDKVersion(&verInfo);

	CString strDrawString;
	strDrawString.Format(TEXT("SDK Version %s"), verInfo.Product);
	GetDlgItem(IDC_STATIC_DLL_VER)->SetWindowText(strDrawString);

	strDrawString = _T("Copyright (c) Integrated Biometrics");
	GetDlgItem(IDC_STATIC_COPYRIGHT)->SetWindowText(strDrawString);

	// Fill the entire working area
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWorkArea, 0);
	SetWindowPos(NULL, rectWorkArea.left, rectWorkArea.top, rectWorkArea.right-rectWorkArea.left,rectWorkArea.bottom-rectWorkArea.top, SWP_SHOWWINDOW);
	
	SetTimer( TIMER_SPLASH_SCREEN_ID, TIMER_SPLASH_SCREEN_DELAY, NULL );
	return TRUE;  
	
}

void CIBScanUltimate_SimpleScanWMSplashDlg::OnTimer(UINT_PTR nIDEvent)
{
	if( nIDEvent == TIMER_SPLASH_SCREEN_ID )
	{
		this->EndDialog(TRUE);
	}
	else
	{
		CDialog::OnTimer(nIDEvent);
	}
}
