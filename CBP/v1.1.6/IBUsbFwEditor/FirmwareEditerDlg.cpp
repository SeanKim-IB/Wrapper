
// FirmwareEditerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "FirmwareEditer.h"
#include "FirmwareEditerDlg.h"
#include "IBScanUltimateApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFirmwareEditerDlg 대화 상자




CFirmwareEditerDlg::CFirmwareEditerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFirmwareEditerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFirmwareEditerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VIEW, m_logo);
	DDX_Control(pDX, IDC_STATIC_PID, m_staticPid);
	DDX_Control(pDX, IDC_STATIC_FIRMWARE, m_staticFirmware);
	DDX_Control(pDX, IDC_STATIC_REVISON, m_staticRevison);
	DDX_Control(pDX, IDC_STATIC_RESERVE, m_staticReserve);
	DDX_Control(pDX, IDC_EDIT_PID, m_editPid);
	DDX_Control(pDX, IDC_EDIT_FIRMWARE, m_editFirmware);
	DDX_Control(pDX, IDC_EDIT_REVISON, m_editRevison);
	DDX_Control(pDX, IDC_EDIT_RESERVE, m_editReserve);
	DDX_Control(pDX, IDC_BUTTON_READ, m_btnRead);
	DDX_Control(pDX, IDC_BUTTON_WRITE, m_btnWrite);
	DDX_Control(pDX, IDC_STATIC_VENDER, m_staticVender);
	DDX_Control(pDX, IDC_EDIT_VENDER, m_editVender);
	DDX_Control(pDX, IDC_COMBO_PID, m_cbPid);
}

BEGIN_MESSAGE_MAP(CFirmwareEditerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_READ, &CFirmwareEditerDlg::OnBnClickedButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, &CFirmwareEditerDlg::OnBnClickedButtonWrite)
END_MESSAGE_MAP()


// CFirmwareEditerDlg 메시지 처리기

BOOL CFirmwareEditerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	m_btnRead.SetIcon(IDI_ICON_FOLDER);
	m_btnRead.SetFlat(FALSE);

	m_btnWrite.SetIcon(IDI_ICON_WRITE);
	m_btnWrite.SetFlat(FALSE);

	//GetDlgItem(IDC_BUTTON_WRITE)->EnableWindow(FALSE);

	//SetDlgItemText(IDC_STATIC_

	SetDlgItemText(IDC_EDIT_VENDER, "Integrated Biometrics");

	flag = false;

	
	/*m_cbPid.AddString("IBNW11C");
	m_cbPid.AddString("Mini");
	m_cbPid.AddString("Thin");*/

	//((CComboBox*)GetDlgItem(IDC_COMBO_PID))->AddString("DK");

	Init_Layout();

	m_cbPid.InsertString(0,"IBNW11C");
	m_cbPid.InsertString(1,"WATSON MINI");
	m_cbPid.InsertString(2,"SHERLOCK");
	m_cbPid.InsertString(3,"SHERLOCK_ROIC");
	m_cbPid.InsertString(4,"COLUMBO");
	m_cbPid.InsertString(5,"CURVE");
	m_cbPid.InsertString(6,"CURVE-320");
	m_cbPid.InsertString(7,"CURVE-340");
	m_cbPid.InsertString(8,"HOLMES");
	m_cbPid.InsertString(9,"KOJAK");
	m_cbPid.InsertString(10,"FIVE-0");
	m_cbPid.SetCurSel(0);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CFirmwareEditerDlg::Init_Layout()
{
	WINDOWPLACEMENT place;

	SetWindowPos(NULL,0,0, 437, 260, SWP_NOMOVE);
	GetWindowPlacement(&place);

	//int left = 10;

	//left = left;
	place.rcNormalPosition.left= 0;
	place.rcNormalPosition.top = 0;
	place.rcNormalPosition.right = 437;
	place.rcNormalPosition.bottom = 47;
	m_logo.SetWindowPlacement(&place);

	int left = 10;
	//int right = 10;

	place.rcNormalPosition.left = left;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom + 10;
	place.rcNormalPosition.right = place.rcNormalPosition.left + 120;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top + 20;
	m_staticPid.SetWindowPlacement(&place);

	place.rcNormalPosition.left = left;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom + 5;
	place.rcNormalPosition.right = place.rcNormalPosition.left + 120;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top + 20;
	m_staticFirmware.SetWindowPlacement(&place);
///////////////////
	place.rcNormalPosition.left = left;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom + 5;
	place.rcNormalPosition.right = place.rcNormalPosition.left + 110;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top + 20;
	//m_staticFirmware.SetWindowPlacement(&place);
	m_staticVender.SetWindowPlacement(&place);

	place.rcNormalPosition.left = left;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom + 5;
	place.rcNormalPosition.right = place.rcNormalPosition.left + 120;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top +20;
	m_staticRevison.SetWindowPlacement(&place);

	place.rcNormalPosition.left = left;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom + 5;
	place.rcNormalPosition.right = place.rcNormalPosition.left + 120;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top + 20;
	m_staticReserve.SetWindowPlacement(&place);

	//editBox
	left = place.rcNormalPosition.left;
	//place.rcNormalPosition.left = place.rcNormalPosition.left + 80;
	place.rcNormalPosition.left = left + 110;
	place.rcNormalPosition.top = 55;
	place.rcNormalPosition.right = place.rcNormalPosition.left + 300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top + 120;
	//m_editPid.SetWindowPlacement(&place);
	m_cbPid.SetWindowPlacement(&place);

	place.rcNormalPosition.left = left + 110;
	place.rcNormalPosition.top = 55 + 25 + 5;
	place.rcNormalPosition.right = place.rcNormalPosition.left +300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top + 20;
	m_editFirmware.SetWindowPlacement(&place);
	////////////////////////////////

	place.rcNormalPosition.left = left + 110;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom + 5;
	place.rcNormalPosition.right = place.rcNormalPosition.left +300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top + 20;
	//m_editFirmware.SetWindowPlacement(&place);
	m_editVender.SetWindowPlacement(&place);

	place.rcNormalPosition.left = left + 110;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom + 5;
	place.rcNormalPosition.right = place.rcNormalPosition.left +300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top + 20;
	m_editRevison.SetWindowPlacement(&place);

	place.rcNormalPosition.left = left + 110;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom + 5;
	place.rcNormalPosition.right = place.rcNormalPosition.left + 300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top + 20;
	m_editReserve.SetWindowPlacement(&place);

	//button
	place.rcNormalPosition.left = left + 110;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom +5;
	place.rcNormalPosition.right = place.rcNormalPosition.left + 130;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top + 35;
	m_btnRead.SetWindowPlacement(&place);

	place.rcNormalPosition.left = place.rcNormalPosition.left + 170;
	place.rcNormalPosition.top = place.rcNormalPosition.top;
	place.rcNormalPosition.right = place.rcNormalPosition.left + 130;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top +35;
	m_btnWrite.SetWindowPlacement(&place);
	
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CFirmwareEditerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CFirmwareEditerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CFirmwareEditerDlg::OnBnClickedButtonRead()
{
	// TODO: ¿©±â¿¡ ÄÁÆ®·Ñ ¾Ë¸² Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.

	CFileDialog dlg(TRUE, 0,0,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "iic files(*.iic)|*.iic||");

	if(dlg.DoModal() == IDOK)
	{
		//m_filepath = dlg.GetFileName();
		m_filepath =dlg.GetPathName();
	//	GetDlgItem(IDC_BUTTON_WRITE)->EnableWindow(true);
	}

	

	
}

void CFirmwareEditerDlg::WriteFirmwareFile()
{
	PRODUCTPROPERTY p;
	FILE *out;

	unsigned char temp[8192];
	unsigned int crc = 0;

	CString pid;
	CString firmversion;
	CString revison;
	CString reserve;

	//out = fopen(m_targetfilepath, "wb");
	//fclose(out);

	//원본파일을 새로만들 파일로 복사한다. 
	m_targetfilepath+=".iic";

	CopyFile(m_filepath, m_targetfilepath, false);

	out = fopen(m_targetfilepath, "ab");


	//out = fopen((char*)(LPCTSTR)m_filepath, "ab");
	memset(&p, 0, sizeof(PRODUCTPROPERTY));
	//SetDlgItemText(IDC_EDIT_PID, firmware);
	GetDlgItemText(IDC_EDIT_PID, pid);
	GetDlgItemText(IDC_EDIT_FIRMWARE, firmversion);
	GetDlgItemText(IDC_EDIT_REVISON, revison);
	GetDlgItemText(IDC_EDIT_RESERVE, reserve);

	int index = m_cbPid.GetCurSel();
	m_cbPid.GetLBText(index, pid);

	strcpy(p.product_ID, pid);
	strcpy(p.firmware_version, firmversion);
	strcpy(p.vender_ID, "Integrated Biometrics");
	strcpy(p.revison, revison);
	strcpy(p.reserve, reserve);

	fseek(out, 0, SEEK_END);

	if(NULL != out)
	{
		fwrite(&p, 1, sizeof(PRODUCTPROPERTY), out);
		fclose(out);
	}

	out = fopen(m_targetfilepath, "rb");
	fseek(out, 0, SEEK_END);
	int size = ftell(out);

	fseek(out, 0, SEEK_SET);
	fread(temp, 1, size, out);

	fclose(out);

	for(int i=0;i<size;i++)
	{
		crc += temp[i];
	}

	out = fopen(m_targetfilepath, "ab");
	fwrite(&crc, 1,4,out);

	fclose(out);

	AfxMessageBox("Finish Write File");
}

void CFirmwareEditerDlg::OnBnClickedButtonWrite()
{
	// TODO: ¿©±â¿¡ ÄÁÆ®·Ñ ¾Ë¸² Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.

	CFileDialog dlg(FALSE, 0,0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "iic files(*.iic)|*.iic||");

	if(dlg.DoModal() == IDOK)
	{
		//m_filepath =dlg.GetPathName();
		//WriterFirmwareFile();

	    if(IBSU_IsWritableDirectory(dlg.GetFolderPath(), TRUE) != IBSU_STATUS_OK)
		{
			AfxMessageBox("You don't have writing permission on this folder\r\nPlease select another folder (e.g. desktop folder)");
		}

		m_targetfilepath = dlg.GetPathName();
		WriteFirmwareFile();
	}
	
}
