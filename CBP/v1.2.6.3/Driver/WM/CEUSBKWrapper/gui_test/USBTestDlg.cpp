// USBTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "USBTest.h"
#include "USBTestDlg.h"
#include "DlgPrint.h"
//#include stdlib.h" // for mbtowc
//#include <Stringapiset.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CUSBTestDlg dialog

CUSBTestDlg::CUSBTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUSBTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUSBTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CUSBTestDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_INPUTEDIT, &CUSBTestDlg::OnEnChangeInputedit)
	ON_WM_CREATE()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CUSBTestDlg message handlers

BOOL CUSBTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CUSBTestDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_USBTEST_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_USBTEST_DIALOG));
	}
}
#endif


void CUSBTestDlg::OnEnChangeEdit2()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CUSBTestDlg::OnEnChangeInputedit()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CString Str, CommandStr;
	CEdit * pDisplay = (CEdit *)this->GetDlgItem( IDC_DISPLAYEDIT);
	CEdit * pInput = (CEdit *) this->GetDlgItem(IDC_INPUTEDIT);
	USES_CONVERSION;

	pInput->GetWindowTextW(Str);

	//WDlgPrintf( L"%s\r\n", L"This is a test");
	//DlgPrintf("%s\r\n", "This is too");

	// CE by default won't take the enter key into a edit box. So we're using the '.' key to indicate the end of a command.
	int Pos = Str.Find('.');
	//DlgPrintf("Pos=%i\r\n", Pos );
	if( Pos > 0 )
	{
		CommandStr = Str.Mid(0,Pos);
		WDlgPrintf(L"%s\r\n", CommandStr);
		CString StrRemainder = Str.Mid(Pos+1);
		pInput->SetWindowTextW(StrRemainder);

		// Do the work
		BOOL running = handleCommand(W2A(CommandStr));
		checkDeviceNotificationQueue(hTestQueue);
		printMenu();

	}
	//pDisplay->SetWindowTextW( Str );
	// Use ReplaceSel and SetSel and GetWindowTextLength
	
}


int WDlgPrintf(const WCHAR *format, ... )
{
	WCHAR Buf[512] ={0};

	CWnd* pMainWnd = AfxGetApp()->GetMainWnd();
    CEdit* pDisplayEdit = (CEdit *)pMainWnd->GetDlgItem(IDC_DISPLAYEDIT);

	va_list args;
	va_start(args, format);
    int ret = _vsnwprintf( Buf, sizeof(Buf)/sizeof(*Buf), format, args);
	int Length = pDisplayEdit->GetWindowTextLengthW();
	pDisplayEdit->SetSel(-1,-1);
	pDisplayEdit->ReplaceSel(Buf);

	
	return ret;
}
int DlgPrintf(const char *format, ... )
{
	char Buf[512] ={0};
	WCHAR WBuf[512] = {0};
	USES_CONVERSION;


	CWnd* pMainWnd = AfxGetApp()->GetMainWnd();
    CEdit* pDisplayEdit = (CEdit *)pMainWnd->GetDlgItem(IDC_DISPLAYEDIT);

	va_list args;
	va_start(args, format);
    int ret = _vsnprintf( Buf, sizeof(Buf)/sizeof(*Buf), format, args);
	pDisplayEdit->SetSel(-1,-1);
	pDisplayEdit->ReplaceSel(A2W(Buf));


	return ret;
}
int CUSBTestDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	// Create message queue for device notifications
	createDeviceNotificationQueue(hTestQueue, hTestNotification);

	return 0;
}

void CUSBTestDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	destroyDeviceNotificationQueue(hTestQueue, hTestNotification);

	CDialog::OnClose();
}
