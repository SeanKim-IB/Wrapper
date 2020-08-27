// ZoomDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "IBScanUltimate_SalesDemo.h"
#include "ZoomDlg.h"
#include "MainDlg.h"

#include "IBScanUltimateApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CZoomDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CZoomDlg, CDialog)

CZoomDlg::CZoomDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CZoomDlg::IDD, pParent)
{
	//m_InImg = new BYTE [IMG_SIZE];
	m_InImg = new BYTE [MAX_IMG_SIZE];
	m_OutImg = new BYTE [MAX_IMG_SIZE*25];
		
	m_Info = (BITMAPINFO*)new BYTE[1064];
	for(int i=0; i<256; i++)
	{
		m_Info->bmiColors[i].rgbBlue = 
		m_Info->bmiColors[i].rgbRed = 
		m_Info->bmiColors[i].rgbGreen = (BYTE)i;
	}
	m_Info->bmiHeader.biBitCount = 8;
	m_Info->bmiHeader.biClrImportant = 0;
	m_Info->bmiHeader.biClrUsed = 0;
	m_Info->bmiHeader.biCompression = BI_RGB;
	m_Info->bmiHeader.biHeight = MAX_IMG_H;
	m_Info->bmiHeader.biPlanes = 1;
	m_Info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_Info->bmiHeader.biSizeImage = MAX_IMG_SIZE;
	m_Info->bmiHeader.biWidth = MAX_IMG_W;
	m_Info->bmiHeader.biXPelsPerMeter = 19700;
	m_Info->bmiHeader.biYPelsPerMeter = 19700;

	m_ZoomRatio = 2;

	m_InImgWidth = MAX_IMG_W;
	m_InImgHeight = MAX_IMG_H;
	m_InImgSize = MAX_IMG_SIZE;
	m_OutImgWidth = MAX_IMG_W;
	m_OutImgHeight = MAX_IMG_H;
	m_OutImgSize = MAX_IMG_SIZE;

	oldpoint = CPoint(0,0);

}

CZoomDlg::~CZoomDlg()
{
	delete [] m_InImg;
	delete [] m_OutImg;

	delete m_Info;
}

void CZoomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_ZOOM, m_ZoomView);
	DDX_Control(pDX, IDC_ZOOM_BTN_1, m_ZButton1);
	DDX_Control(pDX, IDC_ZOOM_BTN_2, m_ZButton2);
	DDX_Control(pDX, IDC_ZOOM_BTN_3, m_ZButton3);
	DDX_Control(pDX, IDC_ZOOM_BTN_4, m_ZButton4);
	DDX_Control(pDX, IDC_ZOOM_BTN_5, m_ZButton5);
	DDX_Control(pDX, IDC_ZOOM_BTN_6, m_ZButton6);
	DDX_Control(pDX, IDC_ZOOM_BTN_7, m_ZButton7);
	DDX_Control(pDX, IDC_ZOOM_BTN_8, m_ZButton8);
	DDX_Control(pDX, IDC_ZOOM_BTN_9, m_ZButton9);
	DDX_Control(pDX, IDC_ZOOM_BTN_10, m_ZButton10);
	DDX_Control(pDX, IDC_ZOOM_BTN_11, m_ZButton11);
	DDX_Control(pDX, IDC_ZOOM_BTN_12, m_ZButton12);
	DDX_Control(pDX, IDC_ZOOM_BTN_13, m_ZButton13);
	DDX_Control(pDX, IDC_ZOOM_BTN_14, m_ZButton14);
	DDX_Control(pDX, IDC_ZOOM_BTN_15, m_ZButton15);
	DDX_Control(pDX, IDC_ZOOM_BTN_16, m_ZButton16);
	DDX_Control(pDX, IDC_ZOOM_BTN_17, m_ZButton17);
	DDX_Control(pDX, IDC_ZOOM_BTN_18, m_ZButton18);
	DDX_Control(pDX, IDC_ZOOM_BTN_19, m_ZButton19);
}


BEGIN_MESSAGE_MAP(CZoomDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_ZOOM_BTN_1, &CZoomDlg::OnBnClickedZoomBtn1)
	ON_BN_CLICKED(IDC_ZOOM_BTN_2, &CZoomDlg::OnBnClickedZoomBtn2)
	ON_BN_CLICKED(IDC_ZOOM_BTN_3, &CZoomDlg::OnBnClickedZoomBtn3)
	ON_BN_CLICKED(IDC_ZOOM_BTN_4, &CZoomDlg::OnBnClickedZoomBtn4)
	ON_BN_CLICKED(IDC_ZOOM_BTN_5, &CZoomDlg::OnBnClickedZoomBtn5)
	ON_BN_CLICKED(IDC_ZOOM_BTN_6, &CZoomDlg::OnBnClickedZoomBtn6)
	ON_BN_CLICKED(IDC_ZOOM_BTN_7, &CZoomDlg::OnBnClickedZoomBtn7)
	ON_BN_CLICKED(IDC_ZOOM_BTN_8, &CZoomDlg::OnBnClickedZoomBtn8)
	ON_BN_CLICKED(IDC_ZOOM_BTN_9, &CZoomDlg::OnBnClickedZoomBtn9)
	ON_BN_CLICKED(IDC_ZOOM_BTN_10, &CZoomDlg::OnBnClickedZoomBtn10)
	ON_BN_CLICKED(IDC_ZOOM_BTN_11, &CZoomDlg::OnBnClickedZoomBtn11)
	ON_BN_CLICKED(IDC_ZOOM_BTN_12, &CZoomDlg::OnBnClickedZoomBtn12)
	ON_BN_CLICKED(IDC_ZOOM_BTN_13, &CZoomDlg::OnBnClickedZoomBtn13)
	ON_BN_CLICKED(IDC_ZOOM_BTN_14, &CZoomDlg::OnBnClickedZoomBtn14)
	ON_BN_CLICKED(IDC_ZOOM_BTN_15, &CZoomDlg::OnBnClickedZoomBtn15)
	ON_BN_CLICKED(IDC_ZOOM_BTN_16, &CZoomDlg::OnBnClickedZoomBtn16)
	ON_BN_CLICKED(IDC_ZOOM_BTN_17, &CZoomDlg::OnBnClickedZoomBtn17)
	ON_BN_CLICKED(IDC_ZOOM_BTN_18, &CZoomDlg::OnBnClickedZoomBtn18)
	ON_BN_CLICKED(IDC_ZOOM_BTN_19, &CZoomDlg::OnBnClickedZoomBtn19)
END_MESSAGE_MAP()


// CZoomDlg 메시지 처리기입니다.
// CZoomDlg message handlers
BOOL CZoomDlg::PreTranslateMessage(MSG* pMsg)
{
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

BOOL CZoomDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//SetWindowPos(NULL, 0, 0, IMG_W+8, IMG_H+36, SWP_NOMOVE);
	InitLayout();

	return TRUE;
}
void CZoomDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CDialog::OnPaint()을(를) 호출하지 마십시오.
	DrawImage();
}
void CZoomDlg::DrawImage()
{
	//CClientDC dc(this);
	CClientDC dc(&m_ZoomView);// =CClientDC((CStatic *)GetDlgItem(IDC_STATIC_ZOOM));
	CRect rect;
	m_ZoomView.GetClientRect(&rect);

	int startx_view, startx_img;
	int starty_view, starty_img;
	int view_width, view_height;
	int img_width, img_height;

	if(m_Info->bmiHeader.biWidth <= rect.Width())
	{
		startx_view = abs(m_Info->bmiHeader.biWidth-rect.Width())/2;
		//startx_view = 0;
		startx_img = 0;
		view_width = m_Info->bmiHeader.biWidth;
		img_width = m_Info->bmiHeader.biWidth;
	}
	else
	{
		startx_view = 0;
		startx_img = abs(m_Info->bmiHeader.biWidth-rect.Width())/2 + m_ZoomStartX;
		view_width = rect.Width();
		img_width = rect.Width();
	}

	if(m_Info->bmiHeader.biHeight <= rect.Height())
	{
		starty_view = abs(m_Info->bmiHeader.biHeight-rect.Height())/2;
		starty_img = 0;
		view_height = m_Info->bmiHeader.biHeight;
		img_height = m_Info->bmiHeader.biHeight;
	}
	else
	{
		starty_view = 0;
		starty_img = abs(m_Info->bmiHeader.biHeight-rect.Height())/2 + m_ZoomStartY;
		view_height = rect.Height();
		img_height = rect.Height();
	}

	CBrush brush;
	brush.CreateSolidBrush(RGB(200,200,200));
	dc.FillRect(&rect, &brush);
	brush.DeleteObject();
	::StretchDIBits(dc.m_hDC, startx_view, starty_view, view_width, view_height, startx_img, starty_img, img_width, img_height,//img_width, img_height, 
						m_OutImg, m_Info, DIB_RGB_COLORS, SRCCOPY);
}
void CZoomDlg::InitLayout()
{
	CButton* btn;
	WINDOWPLACEMENT place;

	GetWindowPlacement(&place);

	SetWindowPos(NULL,0,0,900,670,SWP_NOMOVE);

	btn =(CButton*)GetDlgItem(IDC_STATIC_ZOOM);
	place.rcNormalPosition.left =195;
	place.rcNormalPosition.top =0;
	place.rcNormalPosition.right=place.rcNormalPosition.left+700;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+620;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_1);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);
	
	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_2);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_3);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_4);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_5);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_6);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_7);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_8);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_9);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_10);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_11);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_12);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_13);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_14);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_15);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_16);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_17);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_18);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_ZOOM_BTN_19);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.right=place.rcNormalPosition.left+170;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+30;
	btn->SetWindowPlacement(&place);


	btn =(CButton*)GetDlgItem(IDC_STATIC_Tip1);
	place.rcNormalPosition.left =5;
	place.rcNormalPosition.top =620;
	place.rcNormalPosition.right=place.rcNormalPosition.left+450;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+40;
	btn->SetWindowPlacement(&place);

	btn =(CButton*)GetDlgItem(IDC_STATIC_Tip2);
	place.rcNormalPosition.left =455;
	place.rcNormalPosition.top =620;
	place.rcNormalPosition.right=place.rcNormalPosition.left+445;
	place.rcNormalPosition.bottom =place.rcNormalPosition.top+40;
	btn->SetWindowPlacement(&place);

	


}
void CZoomDlg::Zoom_1_Img()
{
	int i;
	
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->ShowWindow(SW_HIDE);
	}
	CColorButton *btn1 =(CColorButton *)GetDlgItem(IDC_ZOOM_BTN_1);
	btn1->ShowWindow(SW_SHOW);
	btn1->SetWindowTextA("thumbs");
	btn1->SetColor(RGB(0,0,128),RGB(255,255,255));

}
void CZoomDlg::Zoom_3_Img()
{
	int i;
	CColorButton *btn1 =(CColorButton *)GetDlgItem(IDC_ZOOM_BTN_1);
	btn1->ShowWindow(SW_SHOW);
	btn1->SetWindowTextA("Left Four");
	btn1->SetColor(RGB(0,0,128),RGB(255,255,255));

	CColorButton *btn2 =(CColorButton *)GetDlgItem(IDC_ZOOM_BTN_2);
	btn2->ShowWindow(SW_SHOW);
	btn2->SetWindowTextA("Thumbs");
	btn2->SetColor(RGB(0,0,128),RGB(255,255,255));

	CColorButton *btn3 =(CColorButton *)GetDlgItem(IDC_ZOOM_BTN_3);
	btn3->ShowWindow(SW_SHOW);
	btn3->SetWindowTextA("Right Four");
	btn3->SetColor(RGB(0,0,128),RGB(255,255,255));

	CColorButton *btndisable[16] = {&m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<16;i++)
	{
		btndisable[i]->ShowWindow(SW_HIDE);
	}

}

void CZoomDlg::Zoom_13_Img()
{
	int i;
	CColorButton *btn1 =(CColorButton *)GetDlgItem(IDC_ZOOM_BTN_1);
	btn1->ShowWindow(SW_SHOW);
	btn1->SetWindowTextA("Left Four");
	btn1->SetColor(RGB(0,0,128),RGB(255,255,255));

	CColorButton *btn2 =(CColorButton *)GetDlgItem(IDC_ZOOM_BTN_2);
	btn2->ShowWindow(SW_SHOW);
	btn2->SetWindowTextA("Thumbs");
	btn2->SetColor(RGB(0,0,128),RGB(255,255,255));

	CColorButton *btn3 =(CColorButton *)GetDlgItem(IDC_ZOOM_BTN_3);
	btn3->ShowWindow(SW_SHOW);
	btn3->SetWindowTextA("Right Four");
	btn3->SetColor(RGB(0,0,128),RGB(255,255,255));

	CColorButton *btnarr[10] = {&m_ZButton4, &m_ZButton5, &m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12, &m_ZButton13};
	CString btnstr[10] = {"Left Thumb","Left Index", "Left Middle","Left Ring","Left Little",
		"Right Thumb", "Right Index", "Right Middle","Right Ring","Right Little"};

	for(i=0; i<10; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}

	CColorButton *btndisable[6] = {&m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<6;i++)
	{
		btndisable[i]->ShowWindow(SW_HIDE);
	}

}
void CZoomDlg::Zoom_10_Img()
{
	int i;
	CColorButton *btnarr[10] = {&m_ZButton1, &m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, &m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10};
	CString btnstr[10] = {"Left Thumb","Left Index", "Left Middle","Left Ring","Left Little",
		"Right Thumb", "Right Index", "Right Middle","Right Ring","Right Little"};

	CColorButton *btndisable[9] = {&m_ZButton11, &m_ZButton12, &m_ZButton13, &m_ZButton14, &m_ZButton15, &m_ZButton16, &m_ZButton17, &m_ZButton18, &m_ZButton19};	

	for(i=0; i<10; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}

	for(i=0;i<9;i++)
	{
		btndisable[i]->ShowWindow(SW_HIDE);
	}
}
void CZoomDlg::Zoom_14_Img()
{
	int i;
	CColorButton *btnarr[14] = {&m_ZButton1, &m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12, &m_ZButton13, &m_ZButton14};
	CString btnstr[14] = {"Left Thumb", "Left Index", "Left Middle","Left Ring","Left Little",
		"Right Thumb", "Right Index", "Right Middle","Right Ring","Right Little",
		"Left Four","Left Thumb","Right Thumb","Right Four"};

	CColorButton *btndisable[5] = {&m_ZButton15, &m_ZButton16, &m_ZButton17, &m_ZButton18, &m_ZButton19};	

	for(i=0; i<14; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	for(i=0;i<5;i++)
	{
		btndisable[i]->ShowWindow(SW_HIDE);
	}
}
void CZoomDlg::Zoom_16_Img()
{
	int i;
	CColorButton *btnarr[16] = {&m_ZButton1, &m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16};
	CString btnstr[16] = {"Left Thumb", "Left Index", "Left Middle","Left Ring","Left Little",
		"Right Thumb", "Right Index", "Right Middle","Right Ring","Right Little",
		"Left Ring-Little","Left Index-Middle","Left Thumb","Right Thumb","Right Index-Middle",
		"Right Ring-Little"};

	CColorButton *btndisable[3] = {&m_ZButton17, &m_ZButton18, &m_ZButton19};	

	for(i=0; i<16; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	for(i=0;i<3;i++)
	{
		btndisable[i]->ShowWindow(SW_HIDE);
	}

}
void CZoomDlg::Zoom_19_Img()
{
	int i;
	CColorButton *btnarr[19] = {&m_ZButton1, &m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	CString btnstr[19] =  {"Left Thumb", "Left Index", "Left Middle","Left Ring","Left Little",
		"Right Thumb", "Right Index", "Right Middle","Right Ring","Right Little",
		"Left Ring-Little","Left Index-Middle","Left Thumb","Right Thumb","Right Index-Middle",
		"Right Ring-Little","Left Four","Thumbs","Right Four"};

	for(i=0; i<19; i++)
	{
		btnarr[i]->ShowWindow(SW_SHOW);
		btnarr[i]->SetWindowTextA(btnstr[i]);
		btnarr[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
}

void CZoomDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	oldpoint = point;

	CDialog::OnLButtonDown(nFlags, point);
}

void CZoomDlg::ChangeZoomWindow(int zoomratio)
{
	float ratio[11] = {0.5, 0.75, 1.0, 1.25, 1.5, 1.75, 2.0, 2.5, 3.0, 4.0, 5.0};
	CString str;
	
	str.Format("Zoom %d", (int)(ratio[zoomratio]*100));
	SetWindowText(str + "%");
	m_OutImgWidth = (int)(m_InImgWidth*ratio[zoomratio]);
	m_OutImgHeight = (int)(m_InImgHeight*ratio[zoomratio]);
	
	if(m_OutImgWidth%4 != 0)
		m_OutImgWidth = m_OutImgWidth+(4-m_OutImgWidth%4);

	m_OutImgSize = m_OutImgWidth*m_OutImgHeight;
	
	int i, j, ii, jj;
	int ref_x[MAX_IMG_W*6], ref_y[MAX_IMG_H*6];
	int p[MAX_IMG_W*6], q[MAX_IMG_H*6];
	float x, y;
	for(i=0; i<m_OutImgWidth; i++)
	{
		x = (float)i * m_InImgWidth / m_OutImgWidth;
		ref_x[i] = (int)x;
		p[i] = 32-(int)((x - (int)x) * 32);

		if(ref_x[i] >= m_InImgWidth-2) 
			ref_x[i] = m_InImgWidth-2;
	}

	for(i=0; i<m_OutImgHeight; i++)
	{
		y = (float)i * m_InImgHeight / m_OutImgHeight;
		ref_y[i] = (int)y;
		q[i] = 32-(int)((y - (int)y) * 32);

		if(ref_y[i] >= m_InImgHeight-2) 
			ref_y[i] = m_InImgHeight-2;
	}

	memset(m_OutImg, 255, IMG_SIZE*25);

	int ps, qs, pos, value;
    for(i=0; i<m_OutImgHeight; i++)
	{
		ii = ref_y[i];
		qs = q[i];

		for(j=0; j<m_OutImgWidth; j++)
		{
            jj = ref_x[j];
			ps = p[j];

            pos = ii * m_InImgWidth + jj;

            value = (ps * (qs * m_InImg[pos] + (32 - qs) * m_InImg[pos + m_InImgWidth]) + 
					(32 - ps) * (qs * m_InImg[pos + 1] + (32 - qs) * m_InImg[pos + m_InImgWidth + 1])) >> 10;

			m_OutImg[i*m_OutImgWidth+j] = (unsigned char)value;
        }
    }

	m_Info->bmiHeader.biWidth = m_OutImgWidth;
	m_Info->bmiHeader.biHeight = m_OutImgHeight;
	m_Info->bmiHeader.biSizeImage = m_OutImgWidth * m_OutImgHeight;
	DrawImage();
}

BOOL CZoomDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if(zDelta > 0)
	{
		m_ZoomRatio++;
		if(m_ZoomRatio > 10)
			m_ZoomRatio = 10;

		ChangeZoomWindow(m_ZoomRatio);
	}
	else
	{
		m_ZoomRatio--;
		if(m_ZoomRatio < 0)
			m_ZoomRatio = 0;

		ChangeZoomWindow(m_ZoomRatio);
	}

	SetFocus();

	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CZoomDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if(nFlags & MK_LBUTTON)
	{
		m_ZoomStartX += oldpoint.x - point.x;
		m_ZoomStartY -= oldpoint.y - point.y;
		oldpoint = point;
		DrawImage();

	}

	CDialog::OnMouseMove(nFlags, point);
}


void CZoomDlg::OnBnClickedZoomBtn1()
{
	int i;
	m_ZButton1.SetColor(RGB(255,255,255),RGB(0,128,0));
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	if( m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		m_InImgWidth  = MAX_IMG_W;
		m_InImgHeight = MAX_IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;

		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_HAND], m_InImgSize);
//		memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[LEFT_HAND], m_OutImgSize);
	}else
	{
		m_InImgWidth  = m_MainDlg->m_nCurrWidth;
		m_InImgHeight = m_MainDlg->m_nCurrHeight;
		m_InImgSize = m_InImgWidth*m_InImgHeight;

		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_THUMB], m_InImgSize);
//		memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[LEFT_THUMB], m_OutImgSize);

	}
//	m_ZoomRatio = 2;
//	m_ZoomStartX = 0;
//	m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
//	ShowWindow(SW_SHOW);
//	CenterWindow();
//	SetFocus();
}
void CZoomDlg::OnBnClickedZoomBtn2()
{
	m_ZButton2.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton1, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	if( m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH || 
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO || 
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		m_InImgWidth  = MAX_IMG_W;
		m_InImgHeight = MAX_IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[BOTH_THUMBS], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[BOTH_THUMBS], m_OutImgSize);

	}else if(m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH)
	{
		m_InImgWidth  = MAX_IMG_W;
		m_InImgHeight = MAX_IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[THUMB_SUM], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[THUMB_SUM], m_OutImgSize);
	}else
	{
		m_InImgWidth  = m_MainDlg->m_nCurrWidth;
		m_InImgHeight = m_MainDlg->m_nCurrHeight;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_INDEX], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[LEFT_INDEX], m_OutImgSize);

	}
	//m_ZoomRatio = 2;
//	m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn3()
{
	m_ZButton3.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton1, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	if( m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
	{
		m_InImgWidth  = MAX_IMG_W;
		m_InImgHeight = MAX_IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_HAND], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[RIGHT_HAND], m_OutImgSize);
	}else
	{
		m_InImgWidth  = m_MainDlg->m_nCurrWidth;
		m_InImgHeight = m_MainDlg->m_nCurrHeight;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_MIDDLE], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[LEFT_MIDDLE], m_OutImgSize);

	}
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn4()
{
	m_ZButton4.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton1, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}

	if( m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE )
	{
		m_InImgWidth  = m_MainDlg->m_nCurrentWidthForSplit[LEFT_THUMB];
		m_InImgHeight = m_MainDlg->m_nCurrentHeightForSplit[LEFT_THUMB];
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_THUMB], m_InImgSize);

	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		m_InImgWidth  = IMG_W;
		m_InImgHeight = IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_THUMB], m_InImgSize);

	}else
	{
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_RING], m_InImgSize);
	}

	//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[LEFT_RING], m_OutImgSize);
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn5()
{
	m_ZButton5.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton1, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	m_InImgWidth  = m_MainDlg->m_nCurrWidth;
	m_InImgHeight = m_MainDlg->m_nCurrHeight;
	m_InImgSize = m_InImgWidth*m_InImgHeight;

	if( m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE )
	{
		m_InImgWidth  = m_MainDlg->m_nCurrentWidthForSplit[LEFT_INDEX];
		m_InImgHeight = m_MainDlg->m_nCurrentHeightForSplit[LEFT_INDEX];
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_INDEX], m_InImgSize);

	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		m_InImgWidth  = IMG_W;
		m_InImgHeight = IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_INDEX], m_InImgSize);

	}else
	{
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_LITTLE], m_InImgSize);
	}

	//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[LEFT_LITTLE], m_OutImgSize);
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn6()
{
	m_ZButton6.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton1, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	m_InImgWidth  = m_MainDlg->m_nCurrWidth;
	m_InImgHeight = m_MainDlg->m_nCurrHeight;
	m_InImgSize = m_InImgWidth*m_InImgHeight;

	if( m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE )
	{
		m_InImgWidth  = m_MainDlg->m_nCurrentWidthForSplit[LEFT_MIDDLE];
		m_InImgHeight = m_MainDlg->m_nCurrentHeightForSplit[LEFT_MIDDLE];
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_MIDDLE], m_InImgSize);

	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		m_InImgWidth  = IMG_W;
		m_InImgHeight = IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_MIDDLE], m_InImgSize);

	}else
	{
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_THUMB], m_InImgSize);
	}
	
	
	//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[RIGHT_THUMB], m_OutImgSize);
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn7()
{
	m_ZButton7.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton1, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	m_InImgWidth  = m_MainDlg->m_nCurrWidth;
	m_InImgHeight = m_MainDlg->m_nCurrHeight;
	m_InImgSize = m_InImgWidth*m_InImgHeight;

	if( m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE )
	{
		m_InImgWidth  = m_MainDlg->m_nCurrentWidthForSplit[LEFT_RING];
		m_InImgHeight = m_MainDlg->m_nCurrentHeightForSplit[LEFT_RING];
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_RING], m_InImgSize);
	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		m_InImgWidth  = IMG_W;
		m_InImgHeight = IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_RING], m_InImgSize);

	}else
	{
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_INDEX], m_InImgSize);
	}

	
	//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[RIGHT_INDEX], m_OutImgSize);
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn8()
{
	m_ZButton8.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton1, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	m_InImgWidth  = m_MainDlg->m_nCurrWidth;
	m_InImgHeight = m_MainDlg->m_nCurrHeight;
	m_InImgSize = m_InImgWidth*m_InImgHeight;

	if( m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE )
	{
		m_InImgWidth  = m_MainDlg->m_nCurrentWidthForSplit[LEFT_LITTLE];
		m_InImgHeight = m_MainDlg->m_nCurrentHeightForSplit[LEFT_LITTLE];
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_LITTLE], m_InImgSize);
	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		m_InImgWidth  = IMG_W;
		m_InImgHeight = IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_LITTLE], m_InImgSize);

	}else
	{
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_MIDDLE], m_InImgSize);
	}

	
	//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[RIGHT_MIDDLE], m_OutImgSize);
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

}
void CZoomDlg::OnBnClickedZoomBtn9()
{
	m_ZButton9.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton1, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	m_InImgWidth  = m_MainDlg->m_nCurrWidth;
	m_InImgHeight = m_MainDlg->m_nCurrHeight;
	m_InImgSize = m_InImgWidth*m_InImgHeight;

	if( m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE )
	{
		m_InImgWidth  = m_MainDlg->m_nCurrentWidthForSplit[RIGHT_THUMB];
		m_InImgHeight = m_MainDlg->m_nCurrentHeightForSplit[RIGHT_THUMB];
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_THUMB], m_InImgSize);
	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		m_InImgWidth  = IMG_W;
		m_InImgHeight = IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_THUMB], m_InImgSize);

	}else
	{
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_RING], m_InImgSize);
	}

	
	//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[RIGHT_RING], m_OutImgSize);
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn10()
{
	m_ZButton10.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton1, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	m_InImgWidth  = m_MainDlg->m_nCurrWidth;
	m_InImgHeight = m_MainDlg->m_nCurrHeight;
	m_InImgSize = m_InImgWidth*m_InImgHeight;

	if( m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE )
	{
		m_InImgWidth  = m_MainDlg->m_nCurrentWidthForSplit[RIGHT_INDEX];
		m_InImgHeight = m_MainDlg->m_nCurrentHeightForSplit[RIGHT_INDEX];
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_INDEX], m_InImgSize);

	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		m_InImgWidth  = IMG_W;
		m_InImgHeight = IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_INDEX], m_InImgSize);

	}else
	{
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_LITTLE], m_InImgSize);
	}

	//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[RIGHT_LITTLE], m_OutImgSize);
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn11()
{
	m_ZButton11.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton1, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	
	if( m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE)
	{
		m_InImgWidth  = MAX_IMG_W;
		m_InImgHeight = MAX_IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_HAND], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[LEFT_HAND], m_OutImgSize);
		
	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		m_InImgWidth  = m_MainDlg->m_nCurrWidth;
		m_InImgHeight = m_MainDlg->m_nCurrHeight;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_RING_LITTLE], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[LEFT_RING_LITTLE], m_OutImgSize);

	}
	else if( m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE )
	{
		m_InImgWidth  = m_MainDlg->m_nCurrentWidthForSplit[RIGHT_MIDDLE];
		m_InImgHeight = m_MainDlg->m_nCurrentHeightForSplit[RIGHT_MIDDLE];
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_MIDDLE], m_InImgSize);

	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		m_InImgWidth  = IMG_W;
		m_InImgHeight = IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_MIDDLE], m_InImgSize);

	}
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn12()
{
	m_ZButton12.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton1,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	if(m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE)
	{
		m_InImgWidth = m_MainDlg->m_nCurrentWidthForSplit[BOTH_LEFT_THUMB];
		m_InImgHeight = m_MainDlg->m_nCurrentHeightForSplit[BOTH_LEFT_THUMB];
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[BOTH_LEFT_THUMB], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[BOTH_LEFT_THUMB], m_OutImgSize);
		
	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH)
	{
		m_InImgWidth  = m_MainDlg->m_nCurrWidth;
		m_InImgHeight = m_MainDlg->m_nCurrHeight;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[BOTH_LEFT_THUMB], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[BOTH_LEFT_THUMB], m_OutImgSize);

	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		m_InImgWidth  = m_MainDlg->m_nCurrWidth;
		m_InImgHeight = m_MainDlg->m_nCurrHeight;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_INDEX_MIDDLE], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[LEFT_INDEX_MIDDLE], m_OutImgSize);

	}
	else if( m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE )
	{
		m_InImgWidth  = m_MainDlg->m_nCurrentWidthForSplit[RIGHT_RING];
		m_InImgHeight = m_MainDlg->m_nCurrentHeightForSplit[RIGHT_RING];
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_RING], m_InImgSize);

	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		m_InImgWidth  = IMG_W;
		m_InImgHeight = IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_RING], m_InImgSize);

	}
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn13()
{
	m_ZButton13.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton1, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	if(m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE)
	{

		m_InImgWidth = m_MainDlg->m_nCurrentWidthForSplit[BOTH_RIGHT_THUMB];
		m_InImgHeight = m_MainDlg->m_nCurrentHeightForSplit[BOTH_RIGHT_THUMB];
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[BOTH_RIGHT_THUMB], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[BOTH_RIGHT_THUMB], m_OutImgSize);
		
	}else if(m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH)
	{
		m_InImgWidth  = m_MainDlg->m_nCurrWidth;
		m_InImgHeight = m_MainDlg->m_nCurrHeight;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[BOTH_RIGHT_THUMB], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[BOTH_RIGHT_THUMB], m_OutImgSize);

	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		m_InImgWidth  = m_MainDlg->m_nCurrWidth;
		m_InImgHeight = m_MainDlg->m_nCurrHeight;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[BOTH_LEFT_THUMB], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[BOTH_LEFT_THUMB], m_OutImgSize);

	}
	else if( m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE )
	{
		m_InImgWidth  = m_MainDlg->m_nCurrentWidthForSplit[RIGHT_LITTLE];
		m_InImgHeight = m_MainDlg->m_nCurrentHeightForSplit[RIGHT_LITTLE];
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_LITTLE], m_InImgSize);

	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		m_InImgWidth  = IMG_W;
		m_InImgHeight = IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_LITTLE], m_InImgSize);

	}
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn14()
{
	m_ZButton14.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton1,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	if( m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE )
	{
		m_InImgWidth  = MAX_IMG_W;
		m_InImgHeight = MAX_IMG_H;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_HAND], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[RIGHT_HAND], m_OutImgSize);
		
	}
	else if(m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL ||
		m_MainDlg->m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		m_InImgWidth  = m_MainDlg->m_nCurrWidth;
		m_InImgHeight = m_MainDlg->m_nCurrHeight;
		m_InImgSize = m_InImgWidth*m_InImgHeight;
		memcpy(m_InImg ,m_MainDlg->m_ImgBuf[BOTH_RIGHT_THUMB], m_InImgSize);
		//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[BOTH_RIGHT_THUMB], m_OutImgSize);

	}
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn15()
{
	m_ZButton15.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton1, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	m_InImgWidth  = m_MainDlg->m_nCurrWidth;
	m_InImgHeight = m_MainDlg->m_nCurrHeight;
	m_InImgSize = m_InImgWidth*m_InImgHeight;
	memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_INDEX_MIDDLE], m_InImgSize);
	//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[RIGHT_INDEX_MIDDLE], m_OutImgSize);
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn16()
{
	m_ZButton16.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton1, &m_ZButton17,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	m_InImgWidth  = m_MainDlg->m_nCurrWidth;
	m_InImgHeight = m_MainDlg->m_nCurrHeight;
	m_InImgSize = m_InImgWidth*m_InImgHeight;
	memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_RING_LITTLE], m_InImgSize);
	//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[RIGHT_RING_LITTLE], m_OutImgSize);
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn17()
{
	m_ZButton17.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton1,&m_ZButton18, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	m_InImgWidth  = MAX_IMG_W;
	m_InImgHeight = MAX_IMG_H;
	m_InImgSize = m_InImgWidth*m_InImgHeight;
	memcpy(m_InImg ,m_MainDlg->m_ImgBuf[LEFT_HAND], m_InImgSize);
	//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[LEFT_HAND], m_OutImgSize);
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn18()
{
	m_ZButton18.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton1, &m_ZButton19};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}

	m_InImgWidth  = m_MainDlg->m_nCurrWidth;
	m_InImgHeight = m_MainDlg->m_nCurrHeight;
	m_InImgSize = m_InImgWidth*m_InImgHeight;
	memcpy(m_InImg ,m_MainDlg->m_ImgBuf[BOTH_THUMBS], m_InImgSize);
	//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[BOTH_THUMBS], m_OutImgSize);
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
void CZoomDlg::OnBnClickedZoomBtn19()
{
	m_ZButton19.SetColor(RGB(255,255,255),RGB(0,128,0));
	int i;
	CColorButton *btndisable[18] = {&m_ZButton2, &m_ZButton3, &m_ZButton4, &m_ZButton5, 
		&m_ZButton6, &m_ZButton7, &m_ZButton8, &m_ZButton9, &m_ZButton10, &m_ZButton11, &m_ZButton12,
		&m_ZButton13, &m_ZButton14,&m_ZButton15, &m_ZButton16, &m_ZButton17,&m_ZButton18, &m_ZButton1};
	for(i=0;i<18;i++)
	{
		btndisable[i]->SetColor(RGB(0,0,128),RGB(255,255,255));
	}
	m_InImgWidth  = MAX_IMG_W;
	m_InImgHeight = MAX_IMG_H;
	m_InImgSize = m_InImgWidth*m_InImgHeight;
	memcpy(m_InImg ,m_MainDlg->m_ImgBuf[RIGHT_HAND], m_InImgSize);
	//memcpy(m_OutImg ,m_MainDlg->m_ImgBuf[RIGHT_HAND], m_OutImgSize);
	//m_ZoomRatio = 2;
	//m_ZoomStartX = 0;
	//m_ZoomStartY = 0;
	ChangeZoomWindow(m_ZoomRatio);
	//ShowWindow(SW_SHOW);
	//CenterWindow();
	//SetFocus();
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}