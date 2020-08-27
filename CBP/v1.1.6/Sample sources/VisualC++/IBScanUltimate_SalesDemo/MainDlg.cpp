// MainDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#ifdef __FORIBK__
#endif
#include "IBScanUltimate_SalesDemo.h"
#include "MainDlg.h"
#include "IBScanUltimate_SalesDemoDlg.h"

#include "ScanFingerDlg.h"
#include "FingerDisplayManager.h"
#include "IBScanUltimateApi.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainDlg 대화 상자입니다.
const int __LED_COLOR_GREEN__	= 0;
const int __LED_COLOR_RED__		= 1;
const int __LED_COLOR_YELLOW__	= 2;
// CIBScanUltimate_SalesDemoDlg dialog
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

IMPLEMENT_DYNAMIC(CMainDlg, CDialog)

CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
: CDialog(CMainDlg::IDD, pParent)
, m_SeqName(_T(""))
{
	m_CaptureMode = NONE_CAPTURE;
	m_CaptureFinger = NONE_FINGER;
	m_CaptureBtnFinger =0;
	m_CaptureCompleted = FALSE;
	m_PressedBtn = FALSE;
	//m_UseAutoSplit = FALSE;
	int i;

	for(i=0; i<IMAGE_BUFFER_COUNT; i++)
	{
		m_ImgBuf[i] = new BYTE [MAX_IMG_SIZE];
		memset(m_ImgBuf[i], 0xFF, MAX_IMG_SIZE);

		m_ImgBuf_S[i] = new BYTE [MAX_IMG_SIZE];
		memset(m_ImgBuf_S[i], 0xFF, MAX_IMG_SIZE);
	}

	//
	m_Info = (BITMAPINFO*)new BYTE[1064];

	for(i=0; i<256; i++)
	{
		m_Info->bmiColors[i].rgbBlue = 
			m_Info->bmiColors[i].rgbRed = 
			m_Info->bmiColors[i].rgbGreen = (BYTE)i;
	}
	m_Info->bmiHeader.biBitCount = 8;
	m_Info->bmiHeader.biClrImportant = 0;
	m_Info->bmiHeader.biClrUsed = 0;
	m_Info->bmiHeader.biCompression = BI_RGB;
	m_Info->bmiHeader.biHeight = S_IMG_H;
	m_Info->bmiHeader.biPlanes = 1;
	m_Info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_Info->bmiHeader.biSizeImage = S_IMG_SIZE;
	m_Info->bmiHeader.biWidth = S_IMG_W;
	m_Info->bmiHeader.biXPelsPerMeter = 19700;
	m_Info->bmiHeader.biYPelsPerMeter = 19700;

	//
	m_Info_1 = (BITMAPINFO*)new BYTE[1064];

	for(i=0; i<256; i++)
	{
		m_Info_1->bmiColors[i].rgbBlue = 
			m_Info_1->bmiColors[i].rgbRed = 
			m_Info_1->bmiColors[i].rgbGreen = (BYTE)i;
	}
	m_Info_1->bmiHeader.biBitCount = 8;
	m_Info_1->bmiHeader.biClrImportant = 0;
	m_Info_1->bmiHeader.biClrUsed = 0;
	m_Info_1->bmiHeader.biCompression = BI_RGB;
	m_Info_1->bmiHeader.biHeight = S_IMG_1_H;
	m_Info_1->bmiHeader.biPlanes = 1;
	m_Info_1->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_Info_1->bmiHeader.biSizeImage = S_IMG_1_SIZE;
	m_Info_1->bmiHeader.biWidth = S_IMG_1_W;
	m_Info_1->bmiHeader.biXPelsPerMeter = 19700;
	m_Info_1->bmiHeader.biYPelsPerMeter = 19700;

	//
	m_Info_2 = (BITMAPINFO*)new BYTE[1064];

	for(i=0; i<256; i++)
	{
		m_Info_2->bmiColors[i].rgbBlue = 
			m_Info_2->bmiColors[i].rgbRed = 
			m_Info_2->bmiColors[i].rgbGreen = (BYTE)i;
	}
	m_Info_2->bmiHeader.biBitCount = 8;
	m_Info_2->bmiHeader.biClrImportant = 0;
	m_Info_2->bmiHeader.biClrUsed = 0;
	m_Info_2->bmiHeader.biCompression = BI_RGB;
	m_Info_2->bmiHeader.biHeight = S_IMG_2_H;
	m_Info_2->bmiHeader.biPlanes = 1;
	m_Info_2->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_Info_2->bmiHeader.biSizeImage = S_IMG_2_SIZE;
	m_Info_2->bmiHeader.biWidth = S_IMG_2_W;
	m_Info_2->bmiHeader.biXPelsPerMeter = 19700;
	m_Info_2->bmiHeader.biYPelsPerMeter = 19700;

	//3 image
	m_Info_3 = (BITMAPINFO*)new BYTE[1064];

	for(i=0; i<256; i++)
	{
		m_Info_3->bmiColors[i].rgbBlue = 
			m_Info_3->bmiColors[i].rgbRed = 
			m_Info_3->bmiColors[i].rgbGreen = (BYTE)i;
	}
	m_Info_3->bmiHeader.biBitCount = 8;
	m_Info_3->bmiHeader.biClrImportant = 0;
	m_Info_3->bmiHeader.biClrUsed = 0;
	m_Info_3->bmiHeader.biCompression = BI_RGB;
	m_Info_3->bmiHeader.biHeight = S_IMG_3_H;
	m_Info_3->bmiHeader.biPlanes = 1;
	m_Info_3->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_Info_3->bmiHeader.biSizeImage = S_IMG_3_SIZE;
	m_Info_3->bmiHeader.biWidth = S_IMG_3_W;
	m_Info_3->bmiHeader.biXPelsPerMeter = 19700;
	m_Info_3->bmiHeader.biYPelsPerMeter = 19700;

	//1 image
	m_Info_4 = (BITMAPINFO*)new BYTE[1064];

	for(i=0; i<256; i++)
	{
		m_Info_4->bmiColors[i].rgbBlue = 
			m_Info_4->bmiColors[i].rgbRed = 
			m_Info_4->bmiColors[i].rgbGreen = (BYTE)i;
	}
	m_Info_4->bmiHeader.biBitCount = 8;
	m_Info_4->bmiHeader.biClrImportant = 0;
	m_Info_4->bmiHeader.biClrUsed = 0;
	m_Info_4->bmiHeader.biCompression = BI_RGB;
	m_Info_4->bmiHeader.biHeight = S_IMG_4_H;
	m_Info_4->bmiHeader.biPlanes = 1;
	m_Info_4->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_Info_4->bmiHeader.biSizeImage = S_IMG_4_SIZE;
	m_Info_4->bmiHeader.biWidth = S_IMG_4_W;
	m_Info_4->bmiHeader.biXPelsPerMeter = 19700;
	m_Info_4->bmiHeader.biYPelsPerMeter = 19700;

	memset(m_NFIQBuf , 0, sizeof(m_NFIQBuf));
	memset(m_NFIQ2Buf , 0, sizeof(m_NFIQ2Buf));
	memset(m_SpoofBuf , 0, sizeof(m_SpoofBuf));

	memset(m_NFIQBuf1 , 0, sizeof(m_NFIQBuf1));
	memset(m_NFIQBuf2 , 0, sizeof(m_NFIQBuf2));
	memset(m_NFIQBuf3, 0, sizeof(m_NFIQBuf3));
	memset(m_NFIQBuf4 , 0, sizeof(m_NFIQBuf4));

	memset(m_NFIQ2Buf1 , 0, sizeof(m_NFIQ2Buf1));
	memset(m_NFIQ2Buf2 , 0, sizeof(m_NFIQ2Buf2));
	memset(m_NFIQ2Buf3, 0, sizeof(m_NFIQ2Buf3));
	memset(m_NFIQ2Buf4 , 0, sizeof(m_NFIQ2Buf4));

	memset(m_SpoofBuf1 , 0, sizeof(m_SpoofBuf1));
	memset(m_SpoofBuf2 , 0, sizeof(m_SpoofBuf2));
	memset(m_SpoofBuf3, 0, sizeof(m_SpoofBuf3));
	memset(m_SpoofBuf4 , 0, sizeof(m_SpoofBuf4));

	memset(m_NFIQTwoBuf1 , 0, sizeof(m_NFIQTwoBuf1));
	memset(m_NFIQTwoBuf2 , 0, sizeof(m_NFIQTwoBuf2));

	memset(m_NFIQ2TwoBuf1 , 0, sizeof(m_NFIQ2TwoBuf1));
	memset(m_NFIQ2TwoBuf2 , 0, sizeof(m_NFIQ2TwoBuf2));

	memset(m_SpoofTwoBuf1 , 0, sizeof(m_SpoofTwoBuf1));
	memset(m_SpoofTwoBuf2 , 0, sizeof(m_SpoofTwoBuf2));

	m_ZoomDlg = NULL;
}

CMainDlg::~CMainDlg()
{
	for(int i=0; i<IMAGE_BUFFER_COUNT; i++)
	{
		delete [] m_ImgBuf[i];
		delete [] m_ImgBuf_S[i];
	}

	delete m_Info;
	delete m_Info_1;
	delete m_Info_2;
	delete m_Info_3;
	delete m_Info_4;

	if(m_ZoomDlg)
		delete m_ZoomDlg;
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVICE_LIST, m_DeviceList);
	DDX_Control(pDX, IDC_SEQUENCE_LIST, m_SequenceList);
	DDX_Control(pDX, IDC_START_CAPTURE, m_btnCapture);
	DDX_Control(pDX, IDC_EDIT_CAPTURE, m_btnEdit);
	DDX_Control(pDX, IDC_SAVE_CAPTURE, m_btnSave);

	DDX_Control(pDX, IDC_STRING_DEVICE_LIST, m_strDeviceList);
	DDX_Control(pDX, IDC_STRING_CAPTURE_SEQUENCES, m_strCaptureSeq);
	//DDX_Check(pDX, IDC_USE_AUTO_SPLIT, m_UseAutoSplit);
	DDX_Control(pDX, IDC_VIEW_0, m_strView0);
	DDX_Control(pDX, IDC_VIEW_1, m_strView1);
	DDX_Control(pDX, IDC_VIEW_2, m_strView2);
	DDX_Control(pDX, IDC_VIEW_3, m_strView3);
	DDX_Control(pDX, IDC_VIEW_4, m_strView4);
	DDX_Control(pDX, IDC_VIEW_5, m_strView5);
	DDX_Control(pDX, IDC_VIEW_6, m_strView6);
	DDX_Control(pDX, IDC_VIEW_7, m_strView7);
	DDX_Control(pDX, IDC_VIEW_8, m_strView8);
	DDX_Control(pDX, IDC_VIEW_9, m_strView9);
	DDX_Control(pDX, IDC_VIEW_91, m_strView91);
	DDX_Control(pDX, IDC_VIEW_92, m_strView92);
	DDX_Control(pDX, IDC_VIEW_93, m_strView93);
	DDX_Control(pDX, IDC_VIEW_94, m_strView94);
	DDX_Control(pDX, IDC_VIEW_95, m_strView95);
	DDX_Control(pDX, IDC_VIEW_96, m_strView96);
	DDX_Control(pDX, IDC_VIEW_97, m_strView97);
	DDX_Control(pDX, IDC_VIEW_98, m_strView98);
	DDX_Control(pDX, IDC_VIEW_99, m_strView99);
	DDX_Control(pDX, IDC_VIEW_990, m_strView990);
	DDX_Control(pDX, IDC_VIEW_991, m_strView991);
	DDX_Control(pDX, IDC_VIEW_161, m_strView161);
	DDX_Control(pDX, IDC_VIEW_162, m_strView162);
	DDX_Control(pDX, IDC_VIEW_163, m_strView163);
	DDX_Control(pDX, IDC_VIEW_164, m_strView164);

	DDX_Control(pDX, IDC_SEQ_PREVIEW, m_seq_preview);
	DDX_Control(pDX, IDC_PROGRESS_SAVE, m_ProgressSave);
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_DEVICE_LIST, &CMainDlg::OnCbnSelchangeDeviceList)
	ON_CBN_SELCHANGE(IDC_SEQUENCE_LIST, &CMainDlg::OnCbnSelchangeSequenceList)
	ON_BN_CLICKED(IDC_START_CAPTURE, &CMainDlg::OnBnClickedStartCapture)
	ON_BN_CLICKED(IDC_EDIT_CAPTURE, &CMainDlg::OnBnClickedEditCapture)
	ON_BN_CLICKED(IDC_SAVE_CAPTURE, &CMainDlg::OnBnClickedSaveCapture)
	//ON_BN_CLICKED(IDC_USE_AUTO_SPLIT, &CMainDlg::OnBnClickedUseAutoSplit)

	ON_WM_RBUTTONDOWN()
	ON_WM_CTLCOLOR()
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CMainDlg 메시지 처리기입니다.
BOOL CMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	Init_Layout();

	m_nCurrWidth = IMG_W;
	m_nCurrHeight = IMG_H;

	m_nCurrWidth_3  = MAX_IMG_W;
	m_nCurrHeight_3 = MAX_IMG_H;

	m_nCurrWidth_1  = IMG_1_W;
	m_nCurrHeight_1 = IMG_1_H;

	for(int i=0;i<IMAGE_BUFFER_COUNT;i++)
	{
		m_nCurrentWidthForSplit[i] = IMG_W;
		m_nCurrentHeightForSplit[i] = IMG_H;
	}

	m_ZoomDlg = new CZoomDlg();
	m_ZoomDlg->Create(IDD_ZOOM);
	m_ZoomDlg->m_MainDlg = this;

	return TRUE;
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
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

void CMainDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CDialog::OnPaint()을(를) 호출하지 마십시오.

	DrawImage();
}

void CMainDlg::DrawImage()
{
	char tmp_str[256];
	int pos_idx;
	CString strCaptureSeq;
	m_SequenceList.GetLBText(m_SequenceList.GetCurSel(), tmp_str);
	strCaptureSeq.Format("%s", tmp_str);

	CClientDC dc1(this);
	CRect rect;
	CBrush brush;
	//, *oldbrush;
	brush.CreateSolidBrush(RGB(50,50,50));
	GetClientRect(&rect);
	rect.top += 70;
	dc1.FillRect(&rect, &brush);
	brush.DeleteObject();


	CClientDC dc(&m_seq_preview);
	CDC MemDC;
	MemDC.CreateCompatibleDC(&dc);

	CBitmap bitmap, *oldbitmap;
	BITMAP bmpInfo;

	if( strCaptureSeq == PRINT_10_SINGLE_FLAT )
	{
		bitmap.LoadBitmap(IDB_SEQ_10_FLAT);
	}
	else if( strCaptureSeq == PRINT_10_SINGLE_ROLLS )
	{
		bitmap.LoadBitmap(IDB_SEQ_10_ROLLED);
	}
	else if( strCaptureSeq == PRINT_FOUR_FLAT )
	{
		bitmap.LoadBitmap(IDB_SEQ_442);
	}
	else if( strCaptureSeq == PRINT_FOUR_FLAT2 )
	{
		bitmap.LoadBitmap(IDB_SEQ_4141);
	}
	else if( strCaptureSeq == PRINT_FOUR_FLAT_AND_ROLLS )
	{
		bitmap.LoadBitmap(IDB_SEQ_442_Rolled);
	}
	else if( strCaptureSeq == PRINT_FOUR_FLAT2_AND_ROLLS )
	{
		bitmap.LoadBitmap(IDB_SEQ_4141_ROLLED);
	}
	///////////////////////////////////////////////////////////////////////////////////////
	else if( strCaptureSeq == PRINT_14_TWO_FLAT_BOTH_AND_ROLLS )
	{
		bitmap.LoadBitmap(IDB_SEQ_14_Rolled);
	}
	else if( strCaptureSeq == PRINT_14_TWO_FLAT_EACH_AND_ROLLS )
	{
		bitmap.LoadBitmap(IDB_SEQ_14_Rolled2);
	}
	else if( strCaptureSeq == PRINT_14_TWO_FLAT_AND_SINGLE_FLAT )
	{
		bitmap.LoadBitmap(IDB_SEQ_14_Single);
	}
	else if( strCaptureSeq == PRINT_14_FLAT_THUMB )
	{
		bitmap.LoadBitmap(IDB_SEQ_14_2flat);
	}
	else if( strCaptureSeq == PRINT_14_FLAT_THUMB_AND_INDEX )
	{
		bitmap.LoadBitmap(IDB_SEQ_14_4flat);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	else if( strCaptureSeq == PRINT_3_TWO_FLAT_BOTH )
	{
		bitmap.LoadBitmap(IDB_SEQ_3_both);
	}
	else if( strCaptureSeq == PRINT_3_TWO_FLAT_EACH )
	{
		bitmap.LoadBitmap(IDB_SEQ_3_2);
	}
	else if( strCaptureSeq == PRINT_19_TWO_FLAT_DOUBLE_AND_ROLLS )
	{
		bitmap.LoadBitmap(IDB_SEQ_19);
	}
	else if( strCaptureSeq == PRINT_1_TWO_FLAT_BOTH )
	{
		bitmap.LoadBitmap(IDB_SEQ_1);
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////
	else if( strCaptureSeq == PRINT_16_TWO_FLAT_EACH_AND_ROLLS )
	{
		bitmap.LoadBitmap(IDB_SEQ_16);
	}
	else
		bitmap.LoadBitmap(IDB_SEQ_DEFAULT);

	bitmap.GetBitmap(&bmpInfo);
	oldbitmap = MemDC.SelectObject(&bitmap);
	dc.BitBlt(0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, &MemDC, 0, 0, SRCCOPY);
	MemDC.SelectObject(oldbitmap);

	DWORD TextColor = RGB(0, 128, 0);
	int TextMode = OPAQUE;//TRANSPARENT;

	if( m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE || 
		m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
	{
		//10 image
		CString str;

		CClientDC dc1 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_4));
		StretchDIBits(dc1.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc2 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_5));
		StretchDIBits(dc2.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc3 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_6));
		StretchDIBits(dc3.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc4 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_7));
		StretchDIBits(dc4.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc5 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_8));
		StretchDIBits(dc5.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc6 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_9));
		StretchDIBits(dc6.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc7 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_91));
		StretchDIBits(dc7.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc8 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_92));
		StretchDIBits(dc8.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc9 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_93));
		StretchDIBits(dc9.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc10 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_94));
		StretchDIBits(dc10.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC *dcarr[10]={&dc1,&dc2,&dc3,&dc4,&dc5,&dc6,&dc7,&dc8,&dc9,&dc10};
		int finarr[10]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE};

		for(int i=0;i<10;i++)
		{
			if(m_NFIQBuf[finarr[i]]>0)
			{
				dcarr[i]->SetTextColor(TextColor);
				dcarr[i]->SetBkMode(TextMode);

				pos_idx = 0;
				str.Format("NFIQ ( %d )", m_NFIQBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("NFIQ2 ( %d )", m_NFIQ2Buf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("Spoof ( %d )", m_SpoofBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkSpoofD)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);
			}
		}
	}
	else if( m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE || 
		m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
	{
		//14 image
		CString str;

		CClientDC dc1 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_4));
		StretchDIBits(dc1.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc2 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_5));
		StretchDIBits(dc2.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc3 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_6));
		StretchDIBits(dc3.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc4 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_7));
		StretchDIBits(dc4.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc5 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_8));
		StretchDIBits(dc5.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc6 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_9));
		StretchDIBits(dc6.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc7 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_91));
		StretchDIBits(dc7.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc8 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_92));
		StretchDIBits(dc8.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc9 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_93));
		StretchDIBits(dc9.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc10 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_94));
		StretchDIBits(dc10.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc11 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_95));
		StretchDIBits(dc11.m_hDC ,0 ,0 ,S_IMG_1_W ,S_IMG_1_H ,0, 0 ,S_IMG_1_W ,S_IMG_1_H , m_ImgBuf_S[LEFT_HAND] ,m_Info_1 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[LEFT_LITTLE]>0||m_NFIQBuf2[LEFT_RING]>0||m_NFIQBuf3[LEFT_MIDDLE]>0||m_NFIQBuf4[LEFT_INDEX]>0)
		{
			dc11.SetTextColor(TextColor);
			dc11.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[LEFT_LITTLE],m_NFIQBuf2[LEFT_RING],m_NFIQBuf3[LEFT_MIDDLE],m_NFIQBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc11.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[LEFT_LITTLE],m_NFIQ2Buf2[LEFT_RING],m_NFIQ2Buf3[LEFT_MIDDLE],m_NFIQ2Buf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc11.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[LEFT_LITTLE],m_SpoofBuf2[LEFT_RING],m_SpoofBuf3[LEFT_MIDDLE],m_SpoofBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc11.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC dc12 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_96));
		StretchDIBits(dc12.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[BOTH_LEFT_THUMB] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc13 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_97));
		StretchDIBits(dc13.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[BOTH_RIGHT_THUMB] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc14 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_98));
		StretchDIBits(dc14.m_hDC ,0 ,0 ,S_IMG_1_W ,S_IMG_1_H ,0, 0 ,S_IMG_1_W ,S_IMG_1_H , m_ImgBuf_S[RIGHT_HAND] ,m_Info_1 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[RIGHT_INDEX]>0||m_NFIQBuf2[RIGHT_MIDDLE]>0||m_NFIQBuf3[RIGHT_RING]>0||m_NFIQBuf4[RIGHT_LITTLE]>0)
		{
			dc14.SetTextColor(TextColor);
			dc14.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[RIGHT_INDEX],m_NFIQBuf2[RIGHT_MIDDLE],m_NFIQBuf3[RIGHT_RING],m_NFIQBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc14.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[RIGHT_INDEX],m_NFIQ2Buf2[RIGHT_MIDDLE],m_NFIQ2Buf3[RIGHT_RING],m_NFIQ2Buf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc14.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[RIGHT_INDEX],m_SpoofBuf2[RIGHT_MIDDLE],m_SpoofBuf3[RIGHT_RING],m_SpoofBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc14.TextOut(5, 5 + 20*(pos_idx++), str);
		}
		CClientDC *dcarr[12]={&dc1,&dc2,&dc3,&dc4,&dc5,&dc6,&dc7,&dc8,&dc9,&dc10,&dc12,&dc13};
		int finarr[12]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE,BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB};

		for(int i=0;i<12;i++)
		{
			if(m_NFIQBuf[finarr[i]]>0)
			{
				dcarr[i]->SetTextColor(TextColor);
				dcarr[i]->SetBkMode(TextMode);

				pos_idx = 0;
				str.Format("NFIQ (%d)", m_NFIQBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("NFIQ2 (%d)", m_NFIQ2Buf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("Spoof (%d)", m_SpoofBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkSpoofD)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);
			}
		}
	}
	else if(m_CaptureMode == TEN_FINGER_FLAT_THUMB	||
		m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
	{
		//14 image
		CString str;

		CClientDC dc1 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_4));
		StretchDIBits(dc1.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc2 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_5));
		StretchDIBits(dc2.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc3 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_6));
		StretchDIBits(dc3.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc4 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_7));
		StretchDIBits(dc4.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc5 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_8));
		StretchDIBits(dc5.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc6 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_9));
		StretchDIBits(dc6.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc7 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_91));
		StretchDIBits(dc7.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc8 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_92));
		StretchDIBits(dc8.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc9 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_93));
		StretchDIBits(dc9.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc10 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_94));
		StretchDIBits(dc10.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc11 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_95));
		StretchDIBits(dc11.m_hDC ,0 ,0 ,S_IMG_1_W ,S_IMG_1_H ,0, 0 ,S_IMG_1_W ,S_IMG_1_H , m_ImgBuf_S[LEFT_HAND] ,m_Info_1 , DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc12 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_96));
		StretchDIBits(dc12.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[BOTH_LEFT_THUMB] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc13 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_97));
		StretchDIBits(dc13.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[BOTH_RIGHT_THUMB] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc14 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_98));
		StretchDIBits(dc14.m_hDC ,0 ,0 ,S_IMG_1_W ,S_IMG_1_H ,0, 0 ,S_IMG_1_W ,S_IMG_1_H , m_ImgBuf_S[RIGHT_HAND] ,m_Info_1 , DIB_RGB_COLORS, SRCCOPY);

		CClientDC *dcarr[10]={&dc1,&dc2,&dc3,&dc4,&dc5,&dc6,&dc7,&dc8,&dc9,&dc10};
		int finarr[10]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE};

		for(int i=0;i<10;i++)
		{
			if(m_NFIQBuf[finarr[i]]>0)
			{
				dcarr[i]->SetTextColor(TextColor);
				dcarr[i]->SetBkMode(TextMode);

				pos_idx = 0;
				str.Format("NFIQ (%d)", m_NFIQBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("NFIQ2 (%d)", m_NFIQ2Buf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("Spoof (%d)", m_SpoofBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkSpoofD)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);
			}
		}
	}
	else if( m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH)
	{
		//14 image
		CString str;

		CClientDC dc1 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_4));
		StretchDIBits(dc1.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc2 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_5));
		StretchDIBits(dc2.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc3 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_6));
		StretchDIBits(dc3.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc4 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_7));
		StretchDIBits(dc4.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc5 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_8));
		StretchDIBits(dc5.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc6 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_9));
		StretchDIBits(dc6.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc7 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_91));
		StretchDIBits(dc7.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc8 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_92));
		StretchDIBits(dc8.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc9 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_93));
		StretchDIBits(dc9.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc10 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_94));
		StretchDIBits(dc10.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc11 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_95));
		StretchDIBits(dc11.m_hDC ,0 ,0 ,S_IMG_1_W ,S_IMG_1_H ,0, 0 ,S_IMG_1_W ,S_IMG_1_H , m_ImgBuf_S[LEFT_HAND] ,m_Info_1 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[LEFT_LITTLE]>0||m_NFIQBuf2[LEFT_RING]>0||m_NFIQBuf3[LEFT_MIDDLE]>0||m_NFIQBuf4[LEFT_INDEX]>0)
		{
			dc11.SetTextColor(TextColor);
			dc11.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[LEFT_LITTLE],m_NFIQBuf2[LEFT_RING],m_NFIQBuf3[LEFT_MIDDLE],m_NFIQBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc11.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[LEFT_LITTLE],m_NFIQ2Buf2[LEFT_RING],m_NFIQ2Buf3[LEFT_MIDDLE],m_NFIQ2Buf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc11.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[LEFT_LITTLE],m_SpoofBuf2[LEFT_RING],m_SpoofBuf3[LEFT_MIDDLE],m_SpoofBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc11.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC dc12 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_96));
		StretchDIBits(dc12.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[BOTH_LEFT_THUMB] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc13 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_97));
		StretchDIBits(dc13.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[BOTH_RIGHT_THUMB] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc14 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_98));
		StretchDIBits(dc14.m_hDC ,0 ,0 ,S_IMG_1_W ,S_IMG_1_H ,0, 0 ,S_IMG_1_W ,S_IMG_1_H , m_ImgBuf_S[RIGHT_HAND] ,m_Info_1 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[RIGHT_INDEX]>0||m_NFIQBuf2[RIGHT_MIDDLE]>0||m_NFIQBuf3[RIGHT_RING]>0||m_NFIQBuf4[RIGHT_LITTLE]>0)
		{
			dc14.SetTextColor(TextColor);
			dc14.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[RIGHT_INDEX],m_NFIQBuf2[RIGHT_MIDDLE],m_NFIQBuf3[RIGHT_RING],m_NFIQBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc14.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[RIGHT_INDEX],m_NFIQ2Buf2[RIGHT_MIDDLE],m_NFIQ2Buf3[RIGHT_RING],m_NFIQ2Buf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc14.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[RIGHT_INDEX],m_SpoofBuf2[RIGHT_MIDDLE],m_SpoofBuf3[RIGHT_RING],m_SpoofBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc14.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC *dcarr[12]={&dc1,&dc2,&dc3,&dc4,&dc5,&dc6,&dc7,&dc8,&dc9,&dc10,&dc12,&dc13};
		int finarr[12]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE,
			BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB};

		for(int i=0;i<12;i++)
		{
			if(m_NFIQBuf[finarr[i]]>0)
			{
				dcarr[i]->SetTextColor(TextColor);
				dcarr[i]->SetBkMode(TextMode);

				pos_idx = 0;
				str.Format("NFIQ (%d)", m_NFIQBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("NFIQ2 (%d)", m_NFIQ2Buf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("Spoof (%d)", m_SpoofBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkSpoofD)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);
			}
		}
	}

	else if(m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		//16 image
		CString str;

		CClientDC dc1 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_4));
		StretchDIBits(dc1.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc2 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_5));
		StretchDIBits(dc2.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc3 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_6));
		StretchDIBits(dc3.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc4 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_7));
		StretchDIBits(dc4.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc5 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_8));
		StretchDIBits(dc5.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc6 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_9));
		StretchDIBits(dc6.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc7 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_91));
		StretchDIBits(dc7.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc8 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_92));
		StretchDIBits(dc8.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc9 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_93));
		StretchDIBits(dc9.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc10 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_94));
		StretchDIBits(dc10.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc11 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_161));
		StretchDIBits(dc11.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[LEFT_RING_LITTLE] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[LEFT_LITTLE]>0||m_NFIQTwoBuf2[LEFT_RING]>0)
		{
			dc11.SetTextColor(TextColor);
			dc11.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[LEFT_LITTLE],m_NFIQTwoBuf2[LEFT_RING]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc11.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[LEFT_LITTLE],m_NFIQ2TwoBuf2[LEFT_RING]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc11.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[LEFT_LITTLE],m_SpoofTwoBuf2[LEFT_RING]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc11.TextOut(5, 5 + 20*(pos_idx++), str);
		}
		CClientDC dc12 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_162));
		StretchDIBits(dc12.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[LEFT_INDEX_MIDDLE] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[LEFT_MIDDLE]>0||m_NFIQTwoBuf2[LEFT_INDEX]>0)
		{
			dc12.SetTextColor(TextColor);
			dc12.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[LEFT_MIDDLE],m_NFIQTwoBuf2[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc12.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[LEFT_MIDDLE],m_NFIQ2TwoBuf2[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc12.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[LEFT_MIDDLE],m_SpoofTwoBuf2[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc12.TextOut(5, 5 + 20*(pos_idx++), str);
		}
		CClientDC dc13 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_96));
		StretchDIBits(dc13.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[BOTH_LEFT_THUMB] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc14 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_97));
		StretchDIBits(dc14.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[BOTH_RIGHT_THUMB] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc15 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_163));
		StretchDIBits(dc15.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[RIGHT_INDEX_MIDDLE] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[RIGHT_INDEX]>0||m_NFIQTwoBuf2[RIGHT_MIDDLE]>0)
		{
			dc15.SetTextColor(TextColor);
			dc15.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[RIGHT_INDEX],m_NFIQTwoBuf2[RIGHT_MIDDLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc15.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[RIGHT_INDEX],m_NFIQ2TwoBuf2[RIGHT_MIDDLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc15.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[RIGHT_INDEX],m_SpoofTwoBuf2[RIGHT_MIDDLE]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc15.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC dc16 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_164));
		StretchDIBits(dc16.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[RIGHT_RING_LITTLE] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[RIGHT_RING]>0||m_NFIQTwoBuf2[RIGHT_LITTLE]>0)
		{
			dc16.SetTextColor(TextColor);
			dc16.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[RIGHT_RING],m_NFIQTwoBuf2[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc16.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[RIGHT_RING],m_NFIQ2TwoBuf2[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc16.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[RIGHT_RING],m_SpoofTwoBuf2[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc16.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC *dcarr[12]={&dc1,&dc2,&dc3,&dc4,&dc5,&dc6,&dc7,&dc8,&dc9,&dc10,&dc13,&dc14};
		int finarr[12]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE,
			BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB};

		for(int i=0;i<12;i++)
		{
			if(m_NFIQBuf[finarr[i]]>0)
			{
				dcarr[i]->SetTextColor(TextColor);
				dcarr[i]->SetBkMode(TextMode);

				pos_idx = 0;
				str.Format("NFIQ (%d)", m_NFIQBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("NFIQ2 (%d)", m_NFIQ2Buf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("Spoof (%d)", m_SpoofBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkSpoofD)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);
			}
		}
	}
	else if(m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		//19 image
		CString str;

		CClientDC dc1 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_4));
		StretchDIBits(dc1.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc2 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_5));
		StretchDIBits(dc2.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc3 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_6));
		StretchDIBits(dc3.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc4 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_7));
		StretchDIBits(dc4.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc5 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_8));
		StretchDIBits(dc5.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc6 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_9));
		StretchDIBits(dc6.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc7 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_91));
		StretchDIBits(dc7.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc8 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_92));
		StretchDIBits(dc8.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc9 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_93));
		StretchDIBits(dc9.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc10 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_94));
		StretchDIBits(dc10.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc11 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_161));
		StretchDIBits(dc11.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[LEFT_RING_LITTLE] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[LEFT_LITTLE]>0||m_NFIQTwoBuf2[LEFT_RING]>0)
		{
			dc11.SetTextColor(TextColor);
			dc11.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[LEFT_LITTLE],m_NFIQTwoBuf2[LEFT_RING]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc11.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[LEFT_LITTLE],m_NFIQ2TwoBuf2[LEFT_RING]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc11.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[RIGHT_RING],m_SpoofTwoBuf2[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc11.TextOut(5, 5 + 20*(pos_idx++), str);
		}
		CClientDC dc12 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_162));
		StretchDIBits(dc12.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[LEFT_INDEX_MIDDLE] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[LEFT_MIDDLE]>0||m_NFIQTwoBuf2[LEFT_INDEX]>0)
		{
			dc12.SetTextColor(TextColor);
			dc12.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[LEFT_MIDDLE],m_NFIQTwoBuf2[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc12.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[LEFT_MIDDLE],m_NFIQ2TwoBuf2[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc12.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[LEFT_MIDDLE],m_SpoofTwoBuf2[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc12.TextOut(5, 5 + 20*(pos_idx++), str);
		}
		CClientDC dc13 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_96));
		StretchDIBits(dc13.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[BOTH_LEFT_THUMB] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc14 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_97));
		StretchDIBits(dc14.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[BOTH_RIGHT_THUMB] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc15 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_163));
		StretchDIBits(dc15.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[RIGHT_INDEX_MIDDLE] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[RIGHT_INDEX]>0||m_NFIQTwoBuf2[RIGHT_MIDDLE]>0)
		{
			dc15.SetTextColor(TextColor);
			dc15.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[RIGHT_INDEX],m_NFIQTwoBuf2[RIGHT_MIDDLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc15.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[RIGHT_INDEX],m_NFIQ2TwoBuf2[RIGHT_MIDDLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc15.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[RIGHT_INDEX],m_SpoofTwoBuf2[RIGHT_MIDDLE]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc15.TextOut(5, 5 + 20*(pos_idx++), str);
		}
		CClientDC dc16 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_164));
		StretchDIBits(dc16.m_hDC ,0 ,0 ,S_IMG_2_W ,S_IMG_2_H ,0, 0 ,S_IMG_2_W ,S_IMG_2_H , m_ImgBuf_S[RIGHT_RING_LITTLE] ,m_Info_2 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[RIGHT_RING]>0||m_NFIQTwoBuf2[RIGHT_LITTLE]>0)
		{
			dc16.SetTextColor(TextColor);
			dc16.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[RIGHT_RING],m_NFIQTwoBuf2[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc16.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[RIGHT_RING],m_NFIQ2TwoBuf2[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc16.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[RIGHT_RING],m_SpoofTwoBuf2[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc16.TextOut(5, 5 + 20*(pos_idx++), str);
		}
		CClientDC dc17 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_99));
		StretchDIBits(dc17.m_hDC ,0 ,0 ,S_IMG_1_W ,S_IMG_1_H ,0, 0 ,S_IMG_1_W ,S_IMG_1_H , m_ImgBuf_S[LEFT_HAND] ,m_Info_1 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[LEFT_LITTLE]>0||m_NFIQBuf2[LEFT_RING]>0||m_NFIQBuf3[LEFT_MIDDLE]>0||m_NFIQBuf4[LEFT_INDEX]>0)
		{
			dc17.SetTextColor(TextColor);
			dc17.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[LEFT_LITTLE],m_NFIQBuf2[LEFT_RING],m_NFIQBuf3[LEFT_MIDDLE],m_NFIQBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc17.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[LEFT_LITTLE],m_NFIQ2Buf2[LEFT_RING],m_NFIQ2Buf3[LEFT_MIDDLE],m_NFIQ2Buf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc17.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[LEFT_LITTLE],m_SpoofBuf2[LEFT_RING],m_SpoofBuf3[LEFT_MIDDLE],m_SpoofBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc17.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC dc18 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_990));
		StretchDIBits(dc18.m_hDC ,0 ,0 ,S_IMG_1_W ,S_IMG_1_H ,0, 0 ,S_IMG_1_W ,S_IMG_1_H , m_ImgBuf_S[BOTH_THUMBS] ,m_Info_1 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[LEFT_THUMB]>0||m_NFIQTwoBuf2[RIGHT_THUMB]>0)
		{
			dc18.SetTextColor(TextColor);
			dc18.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[LEFT_THUMB],m_NFIQTwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc18.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[LEFT_THUMB],m_NFIQ2TwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc18.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[LEFT_THUMB],m_SpoofTwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc18.TextOut(5, 5 + 20*(pos_idx++), str);
		}
		CClientDC dc19 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_991));
		StretchDIBits(dc19.m_hDC ,0 ,0 ,S_IMG_1_W ,S_IMG_1_H ,0, 0 ,S_IMG_1_W ,S_IMG_1_H , m_ImgBuf_S[RIGHT_HAND] ,m_Info_1 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[RIGHT_INDEX]>0||m_NFIQBuf2[RIGHT_MIDDLE]>0||m_NFIQBuf3[RIGHT_RING]>0||m_NFIQBuf4[RIGHT_LITTLE]>0)
		{
			dc19.SetTextColor(TextColor);
			dc19.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[RIGHT_INDEX],m_NFIQBuf2[RIGHT_MIDDLE],m_NFIQBuf3[RIGHT_RING],m_NFIQBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc19.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[RIGHT_INDEX],m_NFIQ2Buf2[RIGHT_MIDDLE],m_NFIQ2Buf3[RIGHT_RING],m_NFIQ2Buf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc19.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[RIGHT_INDEX],m_SpoofBuf2[RIGHT_MIDDLE],m_SpoofBuf3[RIGHT_RING],m_SpoofBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc19.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC *dcarr[12]={&dc1,&dc2,&dc3,&dc4,&dc5,&dc6,&dc7,&dc8,&dc9,&dc10,&dc13,&dc14};
		int finarr[12]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE,
			BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB};

		for(int i=0;i<12;i++)
		{
			if(m_NFIQBuf[finarr[i]]>0)
			{
				dcarr[i]->SetTextColor(TextColor);
				dcarr[i]->SetBkMode(TextMode);

				pos_idx = 0;
				str.Format("NFIQ (%d)", m_NFIQBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("NFIQ2 (%d)", m_NFIQ2Buf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("Spoof (%d)", m_SpoofBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkSpoofD)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);
			}
		}
	}
	else if(m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH)
	{
		CString str;

		CClientDC dc4 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_4));
		StretchDIBits(dc4.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc5 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_5));
		StretchDIBits(dc5.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc6 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_6));
		StretchDIBits(dc6.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc7 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_7));
		StretchDIBits(dc7.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc8 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_8));
		StretchDIBits(dc8.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc9 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_9));
		StretchDIBits(dc9.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc10 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_91));
		StretchDIBits(dc10.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc11 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_92));
		StretchDIBits(dc11.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc12 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_93));
		StretchDIBits(dc12.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc13 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_94));
		StretchDIBits(dc13.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		//3 image
		CClientDC dc1 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_FOUR_1));
		StretchDIBits(dc1.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[LEFT_HAND] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[LEFT_LITTLE]>0||m_NFIQBuf2[LEFT_RING]>0||m_NFIQBuf3[LEFT_MIDDLE]>0||m_NFIQBuf4[LEFT_INDEX]>0)
		{
			dc1.SetTextColor(TextColor);
			dc1.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[LEFT_LITTLE],m_NFIQBuf2[LEFT_RING],m_NFIQBuf3[LEFT_MIDDLE],m_NFIQBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[LEFT_LITTLE],m_NFIQ2Buf2[LEFT_RING],m_NFIQ2Buf3[LEFT_MIDDLE],m_NFIQ2Buf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[LEFT_LITTLE],m_SpoofBuf2[LEFT_RING],m_SpoofBuf3[LEFT_MIDDLE],m_SpoofBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC dc2 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_FOUR_2));
		StretchDIBits(dc2.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[BOTH_THUMBS] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[LEFT_THUMB]>0||m_NFIQTwoBuf2[RIGHT_THUMB]>0)
		{
			dc2.SetTextColor(TextColor);
			dc2.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[LEFT_THUMB],m_NFIQTwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[LEFT_THUMB],m_NFIQ2TwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[LEFT_THUMB],m_SpoofTwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC dc3 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_FOUR_3));
		StretchDIBits(dc3.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[RIGHT_HAND] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[RIGHT_INDEX]>0||m_NFIQBuf2[RIGHT_MIDDLE]>0||m_NFIQBuf3[RIGHT_RING]>0||m_NFIQBuf4[RIGHT_LITTLE]>0)
		{
			dc3.SetTextColor(TextColor);
			dc3.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[RIGHT_INDEX],m_NFIQBuf2[RIGHT_MIDDLE],m_NFIQBuf3[RIGHT_RING],m_NFIQBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[RIGHT_INDEX],m_NFIQ2Buf2[RIGHT_MIDDLE],m_NFIQ2Buf3[RIGHT_RING],m_NFIQ2Buf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[RIGHT_INDEX],m_SpoofBuf2[RIGHT_MIDDLE],m_SpoofBuf3[RIGHT_RING],m_SpoofBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC *dcarr[10]={&dc4,&dc5,&dc6,&dc7,&dc8,&dc9,&dc10,&dc11,&dc12,&dc13};
		int finarr[10]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE};

		for(int i=0;i<10;i++)
		{
			if(m_NFIQBuf[finarr[i]]>0)
			{
				pos_idx = 0;
				dcarr[i]->SetTextColor(TextColor);
				dcarr[i]->SetBkMode(TextMode);

				str.Format("NFIQ (%d)", m_NFIQBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("NFIQ2 (%d)", m_NFIQ2Buf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("Spoof (%d)", m_SpoofBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkSpoofD)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);
			}
		}
	}
	else if(m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE ||
			m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		CString str;

		CClientDC dc4 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_4));
		StretchDIBits(dc4.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc5 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_5));
		StretchDIBits(dc5.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc6 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_6));
		StretchDIBits(dc6.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc7 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_7));
		StretchDIBits(dc7.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc8 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_8));
		StretchDIBits(dc8.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc9 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_9));
		StretchDIBits(dc9.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc10 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_91));
		StretchDIBits(dc10.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc11 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_92));
		StretchDIBits(dc11.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc12 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_93));
		StretchDIBits(dc12.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc13 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_94));
		StretchDIBits(dc13.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		//3 image
		CClientDC dc1 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_FOUR_1));
		StretchDIBits(dc1.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[LEFT_HAND] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[LEFT_LITTLE]>0||m_NFIQBuf2[LEFT_RING]>0||m_NFIQBuf3[LEFT_MIDDLE]>0||m_NFIQBuf4[LEFT_INDEX]>0)
		{
			dc1.SetTextColor(TextColor);
			dc1.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[LEFT_LITTLE],m_NFIQBuf2[LEFT_RING],m_NFIQBuf3[LEFT_MIDDLE],m_NFIQBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[LEFT_LITTLE],m_NFIQ2Buf2[LEFT_RING],m_NFIQ2Buf3[LEFT_MIDDLE],m_NFIQ2Buf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[LEFT_LITTLE],m_SpoofBuf2[LEFT_RING],m_SpoofBuf3[LEFT_MIDDLE],m_SpoofBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC dc2 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_FOUR_2));
		StretchDIBits(dc2.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[BOTH_THUMBS] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[LEFT_THUMB]>0||m_NFIQTwoBuf2[RIGHT_THUMB]>0)
		{
			dc2.SetTextColor(TextColor);
			dc2.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[LEFT_THUMB],m_NFIQTwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[LEFT_THUMB],m_NFIQ2TwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[LEFT_THUMB],m_SpoofTwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC dc3 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_FOUR_3));
		StretchDIBits(dc3.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[RIGHT_HAND] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[RIGHT_INDEX]>0||m_NFIQBuf2[RIGHT_MIDDLE]>0||m_NFIQBuf3[RIGHT_RING]>0||m_NFIQBuf4[RIGHT_LITTLE]>0)
		{
			dc3.SetTextColor(TextColor);
			dc3.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[RIGHT_INDEX],m_NFIQBuf2[RIGHT_MIDDLE],m_NFIQBuf3[RIGHT_RING],m_NFIQBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[RIGHT_INDEX],m_NFIQ2Buf2[RIGHT_MIDDLE],m_NFIQ2Buf3[RIGHT_RING],m_NFIQ2Buf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[RIGHT_INDEX],m_SpoofBuf2[RIGHT_MIDDLE],m_SpoofBuf3[RIGHT_RING],m_SpoofBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC *dcarr[10]={&dc4,&dc5,&dc6,&dc7,&dc8,&dc9,&dc10,&dc11,&dc12,&dc13};
		int finarr[10]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE};

		for(int i=0;i<10;i++)
		{
			if(m_NFIQBuf[finarr[i]]>0)
			{
				dcarr[i]->SetTextColor(TextColor);
				dcarr[i]->SetBkMode(TextMode);

				pos_idx = 0;
				str.Format("NFIQ (%d)", m_NFIQBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("NFIQ2 (%d)", m_NFIQ2Buf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);

				str.Format("Spoof (%d)", m_SpoofBuf[finarr[i]]);
				if(m_pParent->m_OptionDlg.m_chkSpoofD)
					dcarr[i]->TextOut(5, 5 + 20*(pos_idx++), str);
			}
		}
	}
	else if(m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH)
	{
		CString str;

		//3 image
		CClientDC dc1 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_1));
		StretchDIBits(dc1.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[LEFT_HAND] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[LEFT_LITTLE]>0||m_NFIQBuf2[LEFT_RING]>0||m_NFIQBuf3[LEFT_MIDDLE]>0||m_NFIQBuf4[LEFT_INDEX]>0)
		{
			dc1.SetTextColor(TextColor);
			dc1.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[LEFT_LITTLE],m_NFIQBuf2[LEFT_RING],m_NFIQBuf3[LEFT_MIDDLE],m_NFIQBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[LEFT_LITTLE],m_NFIQ2Buf2[LEFT_RING],m_NFIQ2Buf3[LEFT_MIDDLE],m_NFIQ2Buf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[LEFT_LITTLE],m_SpoofBuf2[LEFT_RING],m_SpoofBuf3[LEFT_MIDDLE],m_SpoofBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC dc2 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_2));
		StretchDIBits(dc2.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[BOTH_THUMBS] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[LEFT_THUMB]>0||m_NFIQTwoBuf2[RIGHT_THUMB]>0)
		{
			dc2.SetTextColor(TextColor);
			dc2.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[LEFT_THUMB],m_NFIQTwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[LEFT_THUMB],m_NFIQ2TwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[LEFT_THUMB],m_SpoofTwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC dc3 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_3));
		StretchDIBits(dc3.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[RIGHT_HAND] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[RIGHT_INDEX]>0||m_NFIQBuf2[RIGHT_MIDDLE]>0||m_NFIQBuf3[RIGHT_RING]>0||m_NFIQBuf4[RIGHT_LITTLE]>0)
		{
			dc3.SetTextColor(TextColor);
			dc3.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[RIGHT_INDEX],m_NFIQBuf2[RIGHT_MIDDLE],m_NFIQBuf3[RIGHT_RING],m_NFIQBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[RIGHT_INDEX],m_NFIQ2Buf2[RIGHT_MIDDLE],m_NFIQ2Buf3[RIGHT_RING],m_NFIQ2Buf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[RIGHT_INDEX],m_SpoofBuf2[RIGHT_MIDDLE],m_SpoofBuf3[RIGHT_RING],m_SpoofBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);
		}
	}

	else if(m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH)
	{
		CString str;

		//3 image
		CClientDC dc1 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_1));
		StretchDIBits(dc1.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[LEFT_HAND] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[LEFT_LITTLE]>0||m_NFIQBuf2[LEFT_RING]>0||m_NFIQBuf3[LEFT_MIDDLE]>0||m_NFIQBuf4[LEFT_INDEX]>0)
		{
			dc1.SetTextColor(TextColor);
			dc1.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[LEFT_LITTLE],m_NFIQBuf2[LEFT_RING],m_NFIQBuf3[LEFT_MIDDLE],m_NFIQBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[LEFT_LITTLE],m_NFIQ2Buf2[LEFT_RING],m_NFIQ2Buf3[LEFT_MIDDLE],m_NFIQ2Buf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[LEFT_LITTLE],m_SpoofBuf2[LEFT_RING],m_SpoofBuf3[LEFT_MIDDLE],m_SpoofBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC dc2 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_2));
		StretchDIBits(dc2.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[THUMB_SUM] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[LEFT_THUMB]>0||m_NFIQTwoBuf2[RIGHT_THUMB]>0)
		{
			dc2.SetTextColor(TextColor);
			dc2.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[LEFT_THUMB],m_NFIQTwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[LEFT_THUMB],m_NFIQ2TwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[LEFT_THUMB],m_SpoofTwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC dc3 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_3));
		StretchDIBits(dc3.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[RIGHT_HAND] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[RIGHT_INDEX]>0||m_NFIQBuf2[RIGHT_MIDDLE]>0||m_NFIQBuf3[RIGHT_RING]>0||m_NFIQBuf4[RIGHT_LITTLE]>0)
		{
			dc3.SetTextColor(TextColor);
			dc3.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[RIGHT_INDEX],m_NFIQBuf2[RIGHT_MIDDLE],m_NFIQBuf3[RIGHT_RING],m_NFIQBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[RIGHT_INDEX],m_NFIQ2Buf2[RIGHT_MIDDLE],m_NFIQ2Buf3[RIGHT_RING],m_NFIQ2Buf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[RIGHT_INDEX],m_SpoofBuf2[RIGHT_MIDDLE],m_SpoofBuf3[RIGHT_RING],m_SpoofBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);
		}
	}
	else if(m_CaptureMode == TWO_FINGER_FLAT_CAPTURE )
	{
		//1 image
		CString str;

		CClientDC dc((CStatic *)GetDlgItem(IDC_VIEW_0));
		StretchDIBits(dc.m_hDC ,0 ,0 ,S_IMG_4_W ,S_IMG_4_H ,0, 0 ,S_IMG_4_W ,S_IMG_4_H , m_ImgBuf_S[BOTH_THUMBS] ,m_Info_4 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[LEFT_THUMB]>0||m_NFIQTwoBuf2[RIGHT_THUMB]>0)
		{
			dc.SetTextColor(TextColor);
			dc.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[LEFT_THUMB],m_NFIQTwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[LEFT_THUMB],m_NFIQ2TwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[LEFT_THUMB],m_SpoofTwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc.TextOut(5, 5 + 20*(pos_idx++), str);
		}
	}
	else
	{
		CString str;

		CClientDC dc4 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_4));
		StretchDIBits(dc4.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc5 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_5));
		StretchDIBits(dc5.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc6 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_6));
		StretchDIBits(dc6.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc7 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_7));
		StretchDIBits(dc7.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc8 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_8));
		StretchDIBits(dc8.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[LEFT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc9 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_9));
		StretchDIBits(dc9.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_THUMB], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc10 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_91));
		StretchDIBits(dc10.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_INDEX], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc11 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_92));
		StretchDIBits(dc11.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_MIDDLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc12 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_93));
		StretchDIBits(dc12.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_RING], m_Info, DIB_RGB_COLORS, SRCCOPY);

		CClientDC dc13 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_94));
		StretchDIBits(dc13.m_hDC, 0, 0, S_IMG_W, S_IMG_H, 0, 0, S_IMG_W, S_IMG_H, m_ImgBuf_S[RIGHT_LITTLE], m_Info, DIB_RGB_COLORS, SRCCOPY);

		//3 image
		CClientDC dc1 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_FOUR_1));
		StretchDIBits(dc1.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[LEFT_HAND] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[LEFT_LITTLE]>0||m_NFIQBuf2[LEFT_RING]>0||m_NFIQBuf3[LEFT_MIDDLE]>0||m_NFIQBuf4[LEFT_INDEX]>0)
		{
			dc1.SetTextColor(TextColor);
			dc1.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[LEFT_LITTLE],m_NFIQBuf2[LEFT_RING],m_NFIQBuf3[LEFT_MIDDLE],m_NFIQBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[LEFT_LITTLE],m_NFIQ2Buf2[LEFT_RING],m_NFIQ2Buf3[LEFT_MIDDLE],m_NFIQ2Buf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[LEFT_LITTLE],m_SpoofBuf2[LEFT_RING],m_SpoofBuf3[LEFT_MIDDLE],m_SpoofBuf4[LEFT_INDEX]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc1.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC dc2 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_FOUR_2));
		StretchDIBits(dc2.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[BOTH_THUMBS] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQTwoBuf1[LEFT_THUMB]>0||m_NFIQTwoBuf2[RIGHT_THUMB]>0)
		{
			dc2.SetTextColor(TextColor);
			dc2.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d)", m_NFIQTwoBuf1[LEFT_THUMB],m_NFIQTwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d)", m_NFIQ2TwoBuf1[LEFT_THUMB],m_NFIQ2TwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d)", m_SpoofTwoBuf1[LEFT_THUMB],m_SpoofTwoBuf2[RIGHT_THUMB]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc2.TextOut(5, 5 + 20*(pos_idx++), str);
		}

		CClientDC dc3 =CClientDC((CStatic *)GetDlgItem(IDC_VIEW_FOUR_3));
		StretchDIBits(dc3.m_hDC ,0 ,0 ,S_IMG_3_W ,S_IMG_3_H ,0, 0 ,S_IMG_3_W ,S_IMG_3_H , m_ImgBuf_S[RIGHT_HAND] ,m_Info_3 , DIB_RGB_COLORS, SRCCOPY);
		if(m_NFIQBuf1[RIGHT_INDEX]>0||m_NFIQBuf2[RIGHT_MIDDLE]>0||m_NFIQBuf3[RIGHT_RING]>0||m_NFIQBuf4[RIGHT_LITTLE]>0)
		{
			dc3.SetTextColor(TextColor);
			dc3.SetBkMode(TextMode);

			pos_idx = 0;
			str.Format("NFIQ (%d-%d-%d-%d)", m_NFIQBuf1[RIGHT_INDEX],m_NFIQBuf2[RIGHT_MIDDLE],m_NFIQBuf3[RIGHT_RING],m_NFIQBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("NFIQ2 (%d-%d-%d-%d)", m_NFIQ2Buf1[RIGHT_INDEX],m_NFIQ2Buf2[RIGHT_MIDDLE],m_NFIQ2Buf3[RIGHT_RING],m_NFIQ2Buf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkViewNFIQ2)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);

			str.Format("Spoof (%d-%d-%d-%d)", m_SpoofBuf1[RIGHT_INDEX],m_SpoofBuf2[RIGHT_MIDDLE],m_SpoofBuf3[RIGHT_RING],m_SpoofBuf4[RIGHT_LITTLE]);
			if(m_pParent->m_OptionDlg.m_chkSpoofD)
				dc3.TextOut(5, 5 + 20*(pos_idx++), str);
		}
	}
}

void CMainDlg::Init_Layout()
{
	CButton* btn;
	WINDOWPLACEMENT place;

	GetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STATIC_LOGO);

	place.rcNormalPosition.right = place.rcNormalPosition.left+172;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+66;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_SEQ_PREVIEW);
	place.rcNormalPosition.left = 4;
	place.rcNormalPosition.top = 180;
	place.rcNormalPosition.right = place.rcNormalPosition.left+162;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+284;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_DEVICE_LIST);
	place.rcNormalPosition.left = 178;//235;
	place.rcNormalPosition.top = 5;
	place.rcNormalPosition.right = place.rcNormalPosition.left+220;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_DEVICE_LIST);
	place.rcNormalPosition.left = 178;//235;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+220;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+200;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_STRING_CAPTURE_SEQUENCES);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = 5;
	place.rcNormalPosition.right = place.rcNormalPosition.left+320;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+25;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_SEQUENCE_LIST);
	place.rcNormalPosition.left = 408;
	place.rcNormalPosition.top = place.rcNormalPosition.bottom;
	place.rcNormalPosition.right = place.rcNormalPosition.left+320;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+200;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_START_CAPTURE);
	place.rcNormalPosition.left = place.rcNormalPosition.right;
	place.rcNormalPosition.top = 0;
	place.rcNormalPosition.right = place.rcNormalPosition.left+120;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+61;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_EDIT_CAPTURE);
	place.rcNormalPosition.left = place.rcNormalPosition.right;
	place.rcNormalPosition.right = place.rcNormalPosition.left+120;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+61;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_SAVE_CAPTURE);
	place.rcNormalPosition.left = place.rcNormalPosition.right;
	place.rcNormalPosition.right = place.rcNormalPosition.left+120;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+55;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_PROGRESS_SAVE);
	place.rcNormalPosition.top = place.rcNormalPosition.bottom+5;
	place.rcNormalPosition.left = 970;
	place.rcNormalPosition.right = place.rcNormalPosition.left+120;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+8;
	btn->SetWindowPlacement(&place);

	//////VIEW 1print/////////////
	btn = (CButton*)GetDlgItem(IDC_VIEW_0);
	place.rcNormalPosition.left = 408;
	place.rcNormalPosition.top = 150;
	place.rcNormalPosition.right = place.rcNormalPosition.left+400;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+370;
	btn->SetWindowPlacement(&place);
	//////////3 print ///////////
	btn = (CButton*)GetDlgItem(IDC_VIEW_1);
	place.rcNormalPosition.left = 173;
	place.rcNormalPosition.top = 180;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+282;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_2);
	place.rcNormalPosition.left = place.rcNormalPosition.right+5;
	place.rcNormalPosition.top = 180;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+282;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_3);
	place.rcNormalPosition.left = place.rcNormalPosition.right+5;
	place.rcNormalPosition.top = 180;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+282;
	btn->SetWindowPlacement(&place);
	//////////////10 print///////////////////
	btn = (CButton*)GetDlgItem(IDC_VIEW_4);
	place.rcNormalPosition.left = 193;
	place.rcNormalPosition.top = 70;
	place.rcNormalPosition.right = place.rcNormalPosition.left+160;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+150;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_5);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = 70;
	place.rcNormalPosition.right = place.rcNormalPosition.left+160;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+150;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_6);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = 70;
	place.rcNormalPosition.right = place.rcNormalPosition.left+160;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+150;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_7);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = 70;
	place.rcNormalPosition.right = place.rcNormalPosition.left+160;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+150;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_8);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = 70;
	place.rcNormalPosition.right = place.rcNormalPosition.left+160;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+150;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_9);
	place.rcNormalPosition.left = 193;
	place.rcNormalPosition.top = 230;
	place.rcNormalPosition.right = place.rcNormalPosition.left+160;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+150;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_91);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = 230;
	place.rcNormalPosition.right = place.rcNormalPosition.left+160;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+150;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_92);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = 230;
	place.rcNormalPosition.right = place.rcNormalPosition.left+160;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+150;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_93);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = 230;
	place.rcNormalPosition.right = place.rcNormalPosition.left+160;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+150;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_94);
	place.rcNormalPosition.left = place.rcNormalPosition.right+10;
	place.rcNormalPosition.top = 230;
	place.rcNormalPosition.right = place.rcNormalPosition.left+160;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+150;
	btn->SetWindowPlacement(&place);

	///////////////14print (4print)///////////////////////////////

	btn = (CButton*)GetDlgItem(IDC_VIEW_95);
	place.rcNormalPosition.left = 193;
	place.rcNormalPosition.top = 390;
	place.rcNormalPosition.right = place.rcNormalPosition.left+280;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+140;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_96);
	place.rcNormalPosition.left = place.rcNormalPosition.right;
	place.rcNormalPosition.top = 390;
	place.rcNormalPosition.right = place.rcNormalPosition.left+140;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+140;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_97);
	place.rcNormalPosition.left = place.rcNormalPosition.right;
	place.rcNormalPosition.top = 390;
	place.rcNormalPosition.right = place.rcNormalPosition.left+140;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+140;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_98);
	place.rcNormalPosition.left = place.rcNormalPosition.right;
	place.rcNormalPosition.top = 390;
	place.rcNormalPosition.right = place.rcNormalPosition.left+280;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+140;
	btn->SetWindowPlacement(&place);

	///////////////16print (6print)///////////////////////////////
	btn = (CButton*)GetDlgItem(IDC_VIEW_161);
	place.rcNormalPosition.left = 193;
	place.rcNormalPosition.top = 390;
	place.rcNormalPosition.right = place.rcNormalPosition.left+140;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+140;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_162);
	place.rcNormalPosition.left = place.rcNormalPosition.right;
	place.rcNormalPosition.top = 390;
	place.rcNormalPosition.right = place.rcNormalPosition.left+140;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+140;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_163);
	place.rcNormalPosition.left = 753;
	place.rcNormalPosition.top = 390;
	place.rcNormalPosition.right = place.rcNormalPosition.left+140;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+140;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_164);
	place.rcNormalPosition.left = place.rcNormalPosition.right;
	place.rcNormalPosition.top = 390;
	place.rcNormalPosition.right = place.rcNormalPosition.left+140;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+140;
	btn->SetWindowPlacement(&place);
	///////////////////19print/////////////////////////////////////

	btn = (CButton*)GetDlgItem(IDC_VIEW_99);
	place.rcNormalPosition.left = 193;
	place.rcNormalPosition.top = 540;
	place.rcNormalPosition.right = place.rcNormalPosition.left+280;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+140;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_990);
	place.rcNormalPosition.left = place.rcNormalPosition.right;
	place.rcNormalPosition.top = 540;
	place.rcNormalPosition.right = place.rcNormalPosition.left+280;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+140;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_991);
	place.rcNormalPosition.left = place.rcNormalPosition.right;
	place.rcNormalPosition.top = 540;
	place.rcNormalPosition.right = place.rcNormalPosition.left+280;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+140;
	btn->SetWindowPlacement(&place);


	/////KOJAK_4-4-2////
	btn = (CButton*)GetDlgItem(IDC_VIEW_FOUR_1);
	place.rcNormalPosition.left = 173;
	place.rcNormalPosition.top = 390;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+282;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_FOUR_2);
	place.rcNormalPosition.left = place.rcNormalPosition.right+5;
	place.rcNormalPosition.top = 390;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+282;
	btn->SetWindowPlacement(&place);

	btn = (CButton*)GetDlgItem(IDC_VIEW_FOUR_3);
	place.rcNormalPosition.left = place.rcNormalPosition.right+5;
	place.rcNormalPosition.top = 390;
	place.rcNormalPosition.right = place.rcNormalPosition.left+300;
	place.rcNormalPosition.bottom = place.rcNormalPosition.top+282;
	btn->SetWindowPlacement(&place);


	m_strDeviceList.SetTextColor(RGB(200,200,200));
	m_strDeviceList.SetBkColor(RGB(50,50,50));

	m_strCaptureSeq.SetTextColor(RGB(200,200,200));
	m_strCaptureSeq.SetBkColor(RGB(50,50,50));

	m_btnCapture.LoadBitmaps(IDB_ENABLE_CAPTURE ,IDB_FOCUS_CAPTURE,IDB_FOCUS_CAPTURE,IDB_DISABLE_CAPTURE);
	m_btnCapture.SizeToContent();

	m_btnEdit.LoadBitmaps(IDB_ENABLE_EDIT, IDB_FOCUS_EDIT,IDB_FOCUS_EDIT,IDB_DISABLE_EDIT);
	m_btnEdit.SizeToContent();

	m_btnSave.LoadBitmaps(IDB_ENABLE_SAVE ,IDB_FOCUS_SAVE,IDB_ENABLE_SAVE,IDB_DISABLE_SAVE);
	m_btnSave.SizeToContent();

	doubleFourFlat_Layout();
}

void CMainDlg::doubleFourFlat_Layout()
{
	int i,j;

	CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_0);
	view->ShowWindow(SW_HIDE);

	for(i =1; i<4; i++ )
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_HIDE);
	}

	for(i= 4; i<14; i++)
	{
		CStatic *view = (CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_SHOW);
	}
	for( i =14;i<18;i++)
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_HIDE);
	}
	CStatic *viewarray[7] ={&m_strView99,&m_strView990,&m_strView991,&m_strView161,&m_strView162,&m_strView163,&m_strView164};

	for(j=0; j<7; j++)
	{
		viewarray[j]->ShowWindow(SW_HIDE);
	}

	for(i =0; i<3; i++ )
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_FOUR_1+i);
		view->ShowWindow(SW_SHOW);
	}
}

void CMainDlg::TwoFlatAndSingleFlat_Layout()
{
	int i,j,k;

	CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_0);
	view->ShowWindow(SW_HIDE);

	for(i =1; i<4; i++ )
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_SHOW);
	}

	for( i=4;i<14;i++ )
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_HIDE);
	}
	for( i =14;i<18;i++)
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_HIDE);
	}

	CStatic *viewarray[7] ={&m_strView99,&m_strView990,&m_strView991,&m_strView161,&m_strView162,&m_strView163,&m_strView164};

	for(j=0; j<7; j++)
	{
		viewarray[j]->ShowWindow(SW_HIDE);
	}

	for(k =0; k<3; k++ )
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_FOUR_1+k);
		view->ShowWindow(SW_HIDE);
	}
}

void CMainDlg::TwoFlatAndSingleFlatAndRoll_Layout()
{
	int i,j,k;

	CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_0);
	view->ShowWindow(SW_HIDE);

	for(i =1; i<4; i++ )
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_HIDE);
	}

	for(i= 4; i<18; i++)
	{
		CStatic *view = (CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_SHOW);
	}

	CStatic *viewarray[7] ={&m_strView99,&m_strView990,&m_strView991,&m_strView161,&m_strView162,&m_strView163,&m_strView164};

	for(j=0; j<7; j++)
	{
		viewarray[j]->ShowWindow(SW_HIDE);
	}

	for(k =0; k<3; k++ )
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_FOUR_1+k);
		view->ShowWindow(SW_HIDE);
	}
}

void CMainDlg::OnlySingleFlat_Layout()
{
	int i,j,k;

	CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_0);
	view->ShowWindow(SW_HIDE);

	for(i =1; i<4; i++ )
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_HIDE);
	}

	for(i= 4; i<14; i++)
	{
		CStatic *view = (CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_SHOW);
	}
	for( i =14;i<18;i++)
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_HIDE);
	}
	CStatic *viewarray[7] ={&m_strView99,&m_strView990,&m_strView991,&m_strView161,&m_strView162,&m_strView163,&m_strView164};

	for(j=0; j<7; j++)
	{
		viewarray[j]->ShowWindow(SW_HIDE);
	}

	for(k =0; k<3; k++ )
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_FOUR_1+k);
		view->ShowWindow(SW_HIDE);
	}
}

void CMainDlg::OnlyTwoFlat_Layout()
{
	int i,j,k;

	CStatic *view = (CStatic *)GetDlgItem(IDC_VIEW_0);
	view->ShowWindow(SW_SHOW);

	for(i =1;i<18;i++)
	{
		CStatic *view = (CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_HIDE);
	}

	CStatic *viewarray[7] ={&m_strView99,&m_strView990,&m_strView991,&m_strView161,&m_strView162,&m_strView163,&m_strView164};

	for(j=0; j<7; j++)
	{
		viewarray[j]->ShowWindow(SW_HIDE);
	}

	for(k =0; k<3; k++ )
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_FOUR_1+k);
		view->ShowWindow(SW_HIDE);
	}
}

void CMainDlg::TwoFlatAndRoll_1_Layout()
{
	int i,j,k;

	CStatic *view[16] ={&m_strView4,&m_strView5,&m_strView6,&m_strView7,&m_strView8,&m_strView9,
		&m_strView91,&m_strView92,&m_strView93,&m_strView94,&m_strView161,
		&m_strView162,&m_strView96,&m_strView97,&m_strView163,&m_strView164};

	for(j=0; j<16; j++)
	{
		view[j]->ShowWindow(SW_SHOW);
	}
	for(i=0;i<4;i++)
	{
		CStatic *view = (CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_HIDE);
	}

	CStatic *view1 =(CStatic *)GetDlgItem(IDC_VIEW_95);
	view1->ShowWindow(SW_HIDE);
	CStatic *view2 =(CStatic *)GetDlgItem(IDC_VIEW_98);
	view2->ShowWindow(SW_HIDE);
	CStatic *view3 =(CStatic *)GetDlgItem(IDC_VIEW_99);
	view3->ShowWindow(SW_HIDE);
	CStatic *view4 =(CStatic *)GetDlgItem(IDC_VIEW_990);
	view4->ShowWindow(SW_HIDE);
	CStatic *view5 =(CStatic *)GetDlgItem(IDC_VIEW_991);
	view5->ShowWindow(SW_HIDE);

	for(k =0; k<3; k++ )
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_FOUR_1+k);
		view->ShowWindow(SW_HIDE);
	}
}

void CMainDlg::TwoFlatAndRoll_2_Layout()
{
	int i,j,k;
	for(i=0;i<4;i++)
	{
		CStatic *view = (CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_HIDE);
	}
	for(i=4;i<14;i++)
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_0+i);
		view->ShowWindow(SW_SHOW);
	}

	CStatic *viewarray[6] ={&m_strView161,&m_strView162,&m_strView96,&m_strView97,&m_strView163,&m_strView164};

	for(j=0; j<6; j++)
	{
		viewarray[j]->ShowWindow(SW_SHOW);
	}

	CStatic *view4 =(CStatic *)GetDlgItem(IDC_VIEW_95);
	view4->ShowWindow(SW_HIDE);

	CStatic *view5 =(CStatic *)GetDlgItem(IDC_VIEW_98);
	view5->ShowWindow(SW_HIDE);

	CStatic *view1 =(CStatic *)GetDlgItem(IDC_VIEW_99);
	view1->ShowWindow(SW_SHOW);
	CStatic *view2 =(CStatic *)GetDlgItem(IDC_VIEW_990);
	view2->ShowWindow(SW_SHOW);
	CStatic *view3 =(CStatic *)GetDlgItem(IDC_VIEW_991);
	view3->ShowWindow(SW_SHOW);

	for(k =0; k<3; k++ )
	{
		CStatic *view =(CStatic *)GetDlgItem(IDC_VIEW_FOUR_1+k);
		view->ShowWindow(SW_HIDE);
	}
}

UINT CMainDlg::_InitializeDeviceThreadCallback( LPVOID pParam )
{   
	if( pParam == NULL )
		return 1;

	CIBScanUltimate_SalesDemoDlg*	pDlg = (CIBScanUltimate_SalesDemoDlg*)pParam;
	const int	devIndex = pDlg->m_MainDlg.m_DeviceList.GetCurSel() - 1;
	int			devHandle;
	int			nRc = IBSU_STATUS_OK;
	CWnd		*disWnd;
	RECT		clientRect;
    int         ledCount;
    DWORD       operableLEDs;

    pDlg->m_MainDlg.BeginWaitCursor();

	pDlg->m_bInitializing = true;

	nRc = IBSU_OpenDevice( devIndex, &devHandle );

	pDlg->m_bInitializing = false;

	if( nRc >= IBSU_STATUS_OK )
	{
		pDlg->m_nDevHandle = devHandle;

        IBSU_GetOperableLEDs(devHandle, &pDlg->m_LedType, &ledCount, &operableLEDs);

		disWnd = pDlg->m_ScanFingerDlg.GetDlgItem( IDC_VIEW );

		disWnd->GetClientRect( &clientRect );

		IBSU_CreateClientWindow( devHandle, disWnd->m_hWnd, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);

		///For ResultImageEx
		IBSU_AddOverlayQuadrangle(devHandle, &pDlg->m_nOvClearPlatenHandle, 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);
		IBSU_AddOverlayText(devHandle, &pDlg->m_nOvImageTextHandle, "Arial", 10, TRUE/*bold*/, "", 10, 10, (DWORD)0);
		IBSU_AddOverlayText(devHandle, &pDlg->m_nOvCaptureTimeTextHandle, "Arial", 10, TRUE/*bold*/, "", 10, 10, (DWORD)0);
		for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
			IBSU_AddOverlayQuadrangle(devHandle, &pDlg->m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);
		for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
			IBSU_AddOverlayText(devHandle, &pDlg->m_nOvQualityTextHandle[i], "Arial", 10, TRUE/*bold*/, "", 10, 10, (DWORD)0);
		///

		// register callback functions
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, pDlg->OnEvent_DeviceCommunicationBreak, pDlg );    
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, pDlg->OnEvent_PreviewImage, pDlg );
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, pDlg->OnEvent_TakingAcquisition, pDlg );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, pDlg->OnEvent_CompleteAcquisition, pDlg );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX, pDlg->OnEvent_ResultImageEx, pDlg );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, pDlg->OnEvent_FingerCount, pDlg );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY, pDlg->OnEvent_FingerQuality, pDlg );  
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_KEYBUTTON, pDlg->OnEvent_PressedKeyButtons, pDlg ); 
		IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE, pDlg->OnEvent_ClearPlatenAtCapture, pDlg );  
	}

	// status notification and sequence start
	if( nRc == IBSU_STATUS_OK )
	{
        if (!pDlg->m_bPreInitialization)
        {
		    pDlg->PostMessage( WM_USER_CAPTURE_SEQ_START );
            pDlg->m_bPreInitialization = FALSE;
        }

        pDlg->m_MainDlg.EndWaitCursor();

		return 0;
	}

	if( nRc > IBSU_STATUS_OK )
		pDlg->PostMessage( WM_USER_INIT_WARNING, nRc );
	else 
	{
		switch (nRc)
		{
		case IBSU_ERR_DEVICE_ACTIVE:
			pDlg->_SetStatusBarMessage( _T( "[Error code = %d] Device initialization failed because in use by another thread/process." ), nRc );
			break;
		case IBSU_ERR_USB20_REQUIRED:
			pDlg->_SetStatusBarMessage( _T( "[Error code = %d] Device initialization failed because SDK only works with USB 2.0." ), nRc );
			break;
		default:
			pDlg->_SetStatusBarMessage( _T( "[Error code = %d] Device initialization failed" ), nRc );
			break;
		}
	}

	pDlg->PostMessage( WM_USER_UPDATE_DISPLAY_RESOURCES );

    pDlg->m_MainDlg.EndWaitCursor();

	return 0;
}

void CMainDlg::OnCbnSelchangeDeviceList()
{
	if( m_DeviceList.GetCurSel() == m_pParent->m_nSelectedDevIndex )
		return;

	AfxGetApp()->DoWaitCursor( 1 );

	m_pParent->m_nSelectedDevIndex = m_DeviceList.GetCurSel(); 
	if( m_pParent->m_nDevHandle != -1 )
	{
		m_pParent->_CaptureStop();
		m_pParent->_ReleaseDevice();
	}
	_UpdateCaptureSequences();

	AfxGetApp()->DoWaitCursor( -1 );
}

void CMainDlg::_UpdateCaptureSequences()
{
	// store currently selected sequence
	CString strSelectedText;
	int selectedSeq = m_SequenceList.GetCurSel();
	if( selectedSeq > -1 )
		m_SequenceList.GetLBText( selectedSeq, strSelectedText );

	// populate combo box
	m_SequenceList.ResetContent();
	m_SequenceList.AddString( _T( "- Please select -" ) );

	const int devIndex = m_DeviceList.GetCurSel() - 1;
	IBSU_DeviceDesc devDesc;
	devDesc.productName[0] = 0;  
	if( devIndex > -1 )
		IBSU_GetDeviceDescription( devIndex, &devDesc );

	if( ( _stricmp( devDesc.productName, "WATSON" )			== 0) ||
		( _stricmp( devDesc.productName, "WATSON MINI" )	== 0) ||
		( _stricmp( devDesc.productName, "SHERLOCK_ROIC" )	== 0) ||
		( _stricmp( devDesc.productName, "SHERLOCK" )		== 0) )
	{
		m_SequenceList.AddString( PRINT_1_TWO_FLAT_BOTH );

		m_SequenceList.AddString( PRINT_3_TWO_FLAT_BOTH );
		m_SequenceList.AddString( PRINT_3_TWO_FLAT_EACH );
		m_SequenceList.AddString( PRINT_10_SINGLE_FLAT );
		m_SequenceList.AddString( PRINT_10_SINGLE_ROLLS );

		m_SequenceList.AddString( PRINT_14_TWO_FLAT_AND_SINGLE_FLAT );
		m_SequenceList.AddString( PRINT_14_TWO_FLAT_BOTH_AND_ROLLS );
		m_SequenceList.AddString( PRINT_14_TWO_FLAT_EACH_AND_ROLLS );
		m_SequenceList.AddString( PRINT_14_FLAT_THUMB );
		m_SequenceList.AddString( PRINT_14_FLAT_THUMB_AND_INDEX );
		m_SequenceList.AddString( PRINT_16_TWO_FLAT_EACH_AND_ROLLS );
		m_SequenceList.AddString( PRINT_19_TWO_FLAT_DOUBLE_AND_ROLLS );	
	}
	else if( ( _stricmp( devDesc.productName, "COLUMBO" )		== 0) ||
		( _stricmp( devDesc.productName, "CURVE" )			== 0) )
	{
		m_SequenceList.AddString( PRINT_10_SINGLE_FLAT );
		m_SequenceList.AddString( PRINT_14_FLAT_THUMB );
		m_SequenceList.AddString( PRINT_14_FLAT_THUMB_AND_INDEX );
	}
	else if( ( _stricmp( devDesc.productName, "KOJAK" )		== 0) ||
		( _stricmp( devDesc.productName, "FIVE-0" )			== 0) )
	{
#if defined(__FORIBK__)
		m_SequenceList.AddString( PRINT_10_SINGLE_FLAT );
		m_SequenceList.AddString( PRINT_10_SINGLE_ROLLS );
		m_SequenceList.AddString( PRINT_FOUR_FLAT );
#else
		m_SequenceList.AddString( PRINT_FOUR_FLAT2 );
		m_SequenceList.AddString( PRINT_FOUR_FLAT2_AND_ROLLS );
		m_SequenceList.AddString( PRINT_10_SINGLE_FLAT );
		m_SequenceList.AddString( PRINT_10_SINGLE_ROLLS );

		m_SequenceList.AddString( PRINT_FOUR_FLAT );
		m_SequenceList.AddString( PRINT_FOUR_FLAT_AND_ROLLS );
#endif
	} 

	// select previously selected sequence
	if( selectedSeq > -1 )
		selectedSeq = m_SequenceList.FindString( 0, strSelectedText );
	m_SequenceList.SetCurSel( selectedSeq == -1 ? 0 : selectedSeq );

	m_pParent->PostMessage(WM_USER_UPDATE_DISPLAY_RESOURCES);
}

void CMainDlg::OnCbnSelchangeSequenceList()
{
	DrawImage();
	m_pParent->PostMessage(WM_USER_UPDATE_DISPLAY_RESOURCES);
}

void CMainDlg::OnBnClickedStartCapture()
{
	if(m_CaptureCompleted == TRUE)
	{
		if(MessageBox("Data will be lost. Do you want to continue?","CBP Demo", MB_YESNO | MB_ICONINFORMATION) == IDNO)
			return;
	}

	m_PressedBtn = TRUE;
	CString strCaptureSeq;
	int nSelectedSeq;
	nSelectedSeq = m_SequenceList.GetCurSel();
	if( nSelectedSeq > -1 )
		m_SequenceList.GetLBText( nSelectedSeq, strCaptureSeq );

	if(m_pParent->m_OptionDlg.m_chkAutoCapture == TRUE)
	{
		m_pParent->m_ScanFingerDlg.m_btnscan.LoadBitmaps(IDB_ENABLE_SCAN,IDB_FOCUS_SCAN,NULL,NULL);
		m_pParent->m_ScanFingerDlg.m_btnscan.SizeToContent();
	}
	else
	{
		m_pParent->m_ScanFingerDlg.m_btnscan.LoadBitmaps(IDB_ENABLE_MANUAL_CAPTURE,IDB_FOCUS_MANUAL_CAPTURE,NULL,NULL);
		m_pParent->m_ScanFingerDlg.m_btnscan.SizeToContent();
	}

	m_pParent->m_ScanFingerDlg.m_Button1.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button2.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button3.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button4.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button5.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button6.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button7.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button8.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button9.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button10.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button11.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button12.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button13.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button14.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button15.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button16.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button17.EnableWindow(TRUE);
	m_pParent->m_ScanFingerDlg.m_Button18.EnableWindow(TRUE);

	memset(m_NFIQBuf , 0, sizeof(m_NFIQBuf));
	memset(m_NFIQ2Buf , 0, sizeof(m_NFIQ2Buf));
	memset(m_SpoofBuf , 0, sizeof(m_SpoofBuf));

	memset(m_NFIQBuf1 , 0, sizeof(m_NFIQBuf1));
	memset(m_NFIQBuf2 , 0, sizeof(m_NFIQBuf2));
	memset(m_NFIQBuf3, 0, sizeof(m_NFIQBuf3));
	memset(m_NFIQBuf4 , 0, sizeof(m_NFIQBuf4));

	memset(m_NFIQ2Buf1 , 0, sizeof(m_NFIQ2Buf1));
	memset(m_NFIQ2Buf2 , 0, sizeof(m_NFIQ2Buf2));
	memset(m_NFIQ2Buf3, 0, sizeof(m_NFIQ2Buf3));
	memset(m_NFIQ2Buf4 , 0, sizeof(m_NFIQ2Buf4));

	memset(m_SpoofBuf1 , 0, sizeof(m_SpoofBuf1));
	memset(m_SpoofBuf2 , 0, sizeof(m_SpoofBuf2));
	memset(m_SpoofBuf3, 0, sizeof(m_SpoofBuf3));
	memset(m_SpoofBuf4 , 0, sizeof(m_SpoofBuf4));

	memset(m_NFIQTwoBuf1 , 0, sizeof(m_NFIQTwoBuf1));
	memset(m_NFIQTwoBuf2 , 0, sizeof(m_NFIQTwoBuf2));

	memset(m_NFIQ2TwoBuf1 , 0, sizeof(m_NFIQ2TwoBuf1));
	memset(m_NFIQ2TwoBuf2 , 0, sizeof(m_NFIQ2TwoBuf2));

	memset(m_SpoofTwoBuf1 , 0, sizeof(m_SpoofTwoBuf1));
	memset(m_SpoofTwoBuf2 , 0, sizeof(m_SpoofTwoBuf2));

	if(m_pParent != NULL)
		IBSU_RemoveFingerImage(m_pParent->m_nDevHandle, IBSU_FINGER_ALL);


	const int	devIndex = m_DeviceList.GetCurSel() - 1;
	int			nRc = IBSU_STATUS_OK;
    IBSU_DeviceDesc deviceDesc;

    m_pParent->m_bPreInitialization = FALSE;
    
    nRc = IBSU_GetDeviceDescription( devIndex, &deviceDesc);
    if (nRc == IBSU_STATUS_OK && strcmp(deviceDesc.productName, "KOJAK") == 0)
    {
	    if( m_pParent->m_nDevHandle == -1 )
	    {
    		//m_pParent->m_bInitializing = true;
           m_pParent->m_bPreInitialization = TRUE;

            SetTimer(0, 100, NULL);
	    }
    }

	m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->InitializeVariables();

	if( strCaptureSeq == PRINT_10_SINGLE_FLAT )
	{
		m_CaptureMode = TEN_FINGER_ONE_FLAT_CAPTURE;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(ONE_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.BtnSingleLayout();
		OnlySingleFlat_Layout();//10
		m_pParent->m_ScanFingerDlg.Disable10Finger();
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_10_SINGLE_FLAT);
	}
	else if( strCaptureSeq == PRINT_10_SINGLE_ROLLS )
	{
		m_CaptureMode = TEN_FINGER_ONE_ROLL_CAPTURE;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(ONE_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.BtnSingleRollLayout();
		OnlySingleFlat_Layout();//10
		m_pParent->m_ScanFingerDlg.Disable10Finger();
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_10_SINGLE_ROLLS);
	}
	else if( strCaptureSeq == PRINT_FOUR_FLAT )
	{
		//for KOJAK
		m_CaptureMode = TEN_FINGER_FOUR_FOUR_TWO;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(FOUR_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.Btn3Layout();
		doubleFourFlat_Layout();
		m_pParent->m_ScanFingerDlg.Disable3Finger();
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_FOUR_FLAT);
	}
	else if( strCaptureSeq == PRINT_FOUR_FLAT2 )
	{
		//for KOJAK
		m_CaptureMode = TEN_FINGER_FOUR_ONE_FOUR_ONE;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(FOUR_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.Btn5Layout();
		doubleFourFlat_Layout();
		m_pParent->m_ScanFingerDlg.Disable3Finger();
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_FOUR_FLAT2);
	}
	else if( strCaptureSeq == PRINT_FOUR_FLAT_AND_ROLLS )
	{
		//for KOJAK
		m_CaptureMode = TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(FOUR_ROLL_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.Btn13Layout();
		doubleFourFlat_Layout();
		m_pParent->m_ScanFingerDlg.Disable13Finger();
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_FOUR_FLAT_AND_ROLLS);
	}
	else if( strCaptureSeq == PRINT_FOUR_FLAT2_AND_ROLLS )
	{
		//for KOJAK
		m_CaptureMode = TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(FOUR_ROLL_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.Btn14Layout();
		doubleFourFlat_Layout();
		m_pParent->m_ScanFingerDlg.Disable13Finger();
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_FOUR_FLAT2_AND_ROLLS);
	}

	///////////////////////////////////////////////////////////////////////////////////////
	else if( strCaptureSeq == PRINT_14_TWO_FLAT_BOTH_AND_ROLLS )
	{
		m_CaptureMode = TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(ALL_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_14_TWO_FLAT_BOTH_AND_ROLLS);
		TwoFlatAndSingleFlatAndRoll_Layout();//14
		m_pParent->m_ScanFingerDlg.Disable15Finger();
		m_pParent->m_ScanFingerDlg.Btn15Layout_Roll();
	}
	else if( strCaptureSeq == PRINT_14_TWO_FLAT_EACH_AND_ROLLS )
	{
		m_CaptureMode = TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(ALL_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_14_TWO_FLAT_EACH_AND_ROLLS);
		TwoFlatAndSingleFlatAndRoll_Layout();//14
		m_pParent->m_ScanFingerDlg.Btn16Layout_Each_Roll();
		m_pParent->m_ScanFingerDlg.Disable16Finger();
	}
	else if( strCaptureSeq == PRINT_14_TWO_FLAT_AND_SINGLE_FLAT )
	{
		m_CaptureMode = TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(ALL_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_14_TWO_FLAT_AND_SINGLE_FLAT);
		TwoFlatAndSingleFlatAndRoll_Layout();//14
		m_pParent->m_ScanFingerDlg.Disable15Finger();
		m_pParent->m_ScanFingerDlg.Btn15Layout_Flat();
	}
	else if( strCaptureSeq == PRINT_14_FLAT_THUMB )
	{
		m_CaptureMode = TEN_FINGER_FLAT_THUMB;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(ONE_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_14_FLAT_THUMB);
		TwoFlatAndSingleFlatAndRoll_Layout();//14
		m_pParent->m_ScanFingerDlg.Btn2Layout();
		m_pParent->m_ScanFingerDlg.Disable2Finger();
	}
	else if( strCaptureSeq == PRINT_14_FLAT_THUMB_AND_INDEX )
	{
		m_CaptureMode = TEN_FINGER_FLAT_THUMB_INDEX;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(ONE_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_14_FLAT_THUMB_AND_INDEX);
		TwoFlatAndSingleFlatAndRoll_Layout();//14
		m_pParent->m_ScanFingerDlg.Btn4Layout();
		m_pParent->m_ScanFingerDlg.Disable4Finger();
	}
	else if( strCaptureSeq == PRINT_3_TWO_FLAT_BOTH )
	{
		m_CaptureMode = TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(ALL_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_3_TWO_FLAT_BOTH);
		TwoFlatAndSingleFlat_Layout();//14
		m_pParent->m_ScanFingerDlg.BtnTwoFingerLayout();
		m_pParent->m_ScanFingerDlg.Disable5Finger();
	}
	else if( strCaptureSeq == PRINT_3_TWO_FLAT_EACH )
	{
		m_CaptureMode = TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH	;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(ALL_FINGER_MODE);		
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_3_TWO_FLAT_EACH);
		TwoFlatAndSingleFlat_Layout();//14
		m_pParent->m_ScanFingerDlg.BtnTwoFingerLayout_Each();
		m_pParent->m_ScanFingerDlg.Disable6Finger();
	}
	else if( strCaptureSeq == PRINT_19_TWO_FLAT_DOUBLE_AND_ROLLS )
	{//Two Flat(5)+Thumb(2)+Roll(10)
		//Button 17
		m_CaptureMode = TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(ALL_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_19_TWO_FLAT_DOUBLE_AND_ROLLS);
		TwoFlatAndRoll_2_Layout();
		m_pParent->m_ScanFingerDlg.Btn17Layout_Each_Both_Roll();
		m_pParent->m_ScanFingerDlg.Disable17Finger();
	}

	else if( strCaptureSeq == PRINT_1_TWO_FLAT_BOTH )
	{
		m_CaptureMode = TWO_FINGER_FLAT_CAPTURE;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(TWO_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_1_TWO_FLAT_BOTH);
		OnlyTwoFlat_Layout();//1
		m_pParent->m_ScanFingerDlg.Btn1Layout();
		m_pParent->m_ScanFingerDlg.Disable1Finger();
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////
	else if( strCaptureSeq == PRINT_16_TWO_FLAT_EACH_AND_ROLLS )
	{//button 5
		m_CaptureMode = TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN;
		m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->SetCaptureMode(ALL_FINGER_MODE);
		m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_STRING_FINGER_STATUS)->SetWindowTextA(PRINT_16_TWO_FLAT_EACH_AND_ROLLS);
		TwoFlatAndRoll_1_Layout();
		m_pParent->m_ScanFingerDlg.Btn16Layout_Each_Roll();
		m_pParent->m_ScanFingerDlg.Disable16Finger();
	}
	else
	{
		MessageBox("None Selected.","CBP Demo", MB_OK | MB_ICONSTOP);
		return;
	}

	// initialize fingerprint memory buffer
	for(int i=0; i<IMAGE_BUFFER_COUNT; i++)
	{
		memset(m_ImgBuf[i], 0xFF, MAX_IMG_SIZE);
		memset(m_ImgBuf_S[i], 0xFF, MAX_IMG_SIZE);
	}
	memset(m_nCurrentWidthForSplit ,0,sizeof(m_nCurrentWidthForSplit));
	memset(m_nCurrentHeightForSplit,0,sizeof(m_nCurrentHeightForSplit));

	m_nCurrWidth =0;
	m_nCurrHeight =0;

	m_nCurrWidth_3 =0;
	m_nCurrHeight_3 =0;

	// go to tab2
	m_pParent->ChangeView(1);
}

void CMainDlg::OnBnClickedEditCapture()
{
	if(m_pParent->m_ScanFingerDlg.m_CaptureEnd == FALSE)
		return;
	if(m_pParent->m_nDevHandle == -1)
		return;

	m_pParent->_SetImageMessage("");
	m_pParent->_SetStatusBarMessage("");

	for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
		IBSU_ModifyOverlayQuadrangle(m_pParent->m_nDevHandle, m_pParent->m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);
	for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
		IBSU_ModifyOverlayText(m_pParent->m_nDevHandle, m_pParent->m_nOvQualityTextHandle[i], "Arial", 10, TRUE/*bold*/, "", 10, 10, (DWORD)0);

	m_pParent->m_ScanFingerDlg.m_CaptureStart =TRUE;

//	memset(m_TemplateDB , 0, sizeof(m_TemplateDB));
//	memset(m_TemplateCheck, 0, sizeof(m_TemplateCheck));
//	memset(m_TemplateThumb, 0, sizeof(m_TemplateThumb));
//	memset(m_TemplateFour, 0, sizeof(m_TemplateFour));

	// go to tab2
	m_pParent->ChangeView(1);
}

void CMainDlg::OnBnClickedSaveCapture()
{
	if(m_pParent->m_ScanFingerDlg.m_CaptureEnd == FALSE)
		return;

	LPITEMIDLIST	pidlSelected;
	BROWSEINFO		bi = {0};
	LPMALLOC		pMalloc;
	TCHAR			m_ImgSaveFolder[MAX_PATH + 1];				///< Base folder for image saving
	int				i;
	SHGetMalloc( &pMalloc );

	// show 'Browse For Folder' dialog:
	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = _T("Please select a folder to store captured images!");
	bi.ulFlags = BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseForFolderCallback;
	bi.lParam = (LPARAM)&m_ImgSaveFolder[0];

	pidlSelected = SHBrowseForFolder(&bi);

	/*CString strFolder;
	strFolder.Format( _T("%s\\%s"), m_ImgSaveFolder,m_ImgSubFolder);
	CreateDirectory(strFolder,NULL);*/


	if( pidlSelected )
	{
		SHGetPathFromIDList( pidlSelected, m_ImgSaveFolder );
		pMalloc->Free(pidlSelected);

		CString filepath = m_ImgSaveFolder;

		if(filepath.GetAt(filepath.GetLength()-1) == '\\')
			filepath.Delete(filepath.GetLength()-1, 1);

		sprintf(m_ImgSaveFolder, "%s", filepath);

		if (IBSU_IsWritableDirectory(m_ImgSaveFolder, FALSE) != IBSU_STATUS_OK)
		{
			AfxMessageBox("You don't have writing permission on this folder\r\nPlease select another folder (e.g. desktop folder)");
			return;
		}

		// Save
		CString filename[IMAGE_BUFFER_COUNT], full_filename;
		CString filename_bitmap[IMAGE_BUFFER_COUNT];
		CString filename_wsq[IMAGE_BUFFER_COUNT];
		CString filename_iso[IMAGE_BUFFER_COUNT];

        BeginWaitCursor();


		if( m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE)
		{
			int FingerName[IMAGE_BUFFER_COUNT] = {LEFT_LITTLE,LEFT_RING,LEFT_MIDDLE,LEFT_INDEX,LEFT_THUMB,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE};
			CString FName[10]={"SFF_Left_Little","SFF_Left_Ring","SFF_Left_Middle","SFF_Left_Index","SFF_Left_Thumb","SFF_Right_Thumb","SFF_Right_Index","SFF_Right_Middle","SFF_Right_Ring","SFF_Right_Little"};

			for(i=0;i<10;i++)
			{
				filename_bitmap[i] =_T(FName[i]+".bmp");
				filename_wsq[i] = _T(FName[i]+".wsq");
				filename_iso[i] = _T(FName[i]+".fir");
			}
			m_ProgressSave.SetRange(0, 9);


			BOOL bkColor = TRUE;		// background is white
			for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
			{
				m_ProgressSave.SetPos(i);
				if(m_pParent->m_OptionDlg.m_chkUseBitmap)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[i]);

					if( IBSU_SaveBitmapImage( full_filename,(BYTE*)m_ImgBuf[i],m_nCurrWidth, m_nCurrHeight , -m_nCurrWidth,500,500) != IBSU_STATUS_OK )
					{					
						MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
					}
				}

				if(m_pParent->m_OptionDlg.m_chkUseWsq)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[i]);

					if( SaveWSQ(full_filename, m_ImgBuf[i], m_nCurrWidth,m_nCurrHeight) == 0)
					{
						MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
					}
				}

				if(m_pParent->m_OptionDlg.m_chkUseIso)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[i]);

					if( SaveISOFIR(full_filename, m_ImgBuf[i], m_nCurrWidth,m_nCurrHeight) == 0)
					{
						MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
					}
				}
			}

			m_ProgressSave.SetPos(0);
			MessageBox( _T("Done!"),"CBP Demo");
		}
		//1
		else if( m_CaptureMode == TWO_FINGER_FLAT_CAPTURE )
		{

			filename_bitmap[BOTH_THUMBS] =_T("SFF_BOTH_Thumbs.bmp");
			filename_wsq[BOTH_THUMBS]=_T("SFF_BOTH_Thumbs.wsq");
			filename_iso[BOTH_THUMBS]=_T("SFF_BOTH_Thumbs.fir");

			if(m_pParent->m_OptionDlg.m_chkUseBitmap)
			{
				full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[BOTH_THUMBS]);

				if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[BOTH_THUMBS], 
					m_nCurrWidth, m_nCurrHeight, -m_nCurrWidth, 500, 500 ) != IBSU_STATUS_OK )
				{
					MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");	
				}
			}

			if(m_pParent->m_OptionDlg.m_chkUseWsq)
			{
				full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[BOTH_THUMBS]);
				if( SaveWSQ(full_filename, m_ImgBuf[BOTH_THUMBS], m_nCurrWidth,m_nCurrHeight) == 0)
				{
					MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
				}
			}

			if(m_pParent->m_OptionDlg.m_chkUseIso)
			{
				full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[BOTH_THUMBS]);
				if( SaveISOFIR(full_filename, m_ImgBuf[BOTH_THUMBS], m_nCurrWidth,m_nCurrHeight) == 0)
				{
					MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
				}
			}
			MessageBox( _T("Done!"),"CBP Demo");
		}

		//10
		else if( m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE )
		{

			int FingerName[IMAGE_BUFFER_COUNT] = {LEFT_LITTLE,LEFT_RING,LEFT_MIDDLE,LEFT_INDEX,LEFT_THUMB,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE};
			CString FName[10]={"SRF_Left_Little","SRF_Left_Ring","SRF_Left_Middle","SRF_Left_Index","SRF_Left_Thumb","SRF_Right_Thumb","SRF_Right_Index","SRF_Right_Middle","SRF_Right_Ring","SRF_Right_Little"};

			for(i=0;i<10;i++)
			{
				filename_bitmap[i] =_T(FName[i]+".bmp");
				filename_wsq[i] = _T(FName[i]+".wsq");
				filename_iso[i] = _T(FName[i]+".fir");
			}
			BOOL bkColor = TRUE;		// background is white

			m_ProgressSave.SetRange(0, 9);

			for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
			{
				m_ProgressSave.SetPos(i);

				if(m_pParent->m_OptionDlg.m_chkUseBitmap)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[i]);

					if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[i], 
						m_nCurrWidth, m_nCurrHeight, -m_nCurrWidth, 500, 500 ) != IBSU_STATUS_OK )
					{
						MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						break;
					}

				}
				if(m_pParent->m_OptionDlg.m_chkUseWsq)
				{	
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[i]);

					if( SaveWSQ(full_filename, m_ImgBuf[i], m_nCurrWidth,m_nCurrHeight) == 0)
					{
						MessageBox( _T("Failed to save wsq image!"),"CBP Demo");

					}

				}
				if(m_pParent->m_OptionDlg.m_chkUseIso)
				{	
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[i]);

					if( SaveISOFIR(full_filename, m_ImgBuf[i], m_nCurrWidth,m_nCurrHeight) == 0)
					{
						MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");

					}

				}

			}
			m_ProgressSave.SetPos(0);
			MessageBox( _T("Done!"),"CBP Demo");
		}
		//KOJAK (4-4-2)
		//KOJAK (4-1-4-1)
		else if( m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
				 m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE)
		{
			filename_bitmap[LEFT_HAND] =_T("SFF_LEFT_Four.bmp");
			filename_bitmap[BOTH_THUMBS] = _T("STF_Both_Thumbs.bmp");
			filename_bitmap[RIGHT_HAND] =_T("SFF_RIGHT_Four.bmp");

			filename_bitmap[LEFT_LITTLE] = _T("SFF_Left_Little.bmp");
			filename_bitmap[LEFT_RING] = _T("SFF_Left_Ring.bmp");
			filename_bitmap[LEFT_MIDDLE] = _T("SFF_Left_Middle.bmp");
			filename_bitmap[LEFT_INDEX] = _T("SFF_Left_Index.bmp");
			filename_bitmap[LEFT_THUMB] = _T("SFF_Left_Thumb.bmp");
			filename_bitmap[RIGHT_THUMB] = _T("SFF_Right_Thumb.bmp");
			filename_bitmap[RIGHT_INDEX] = _T("SFF_Right_Index.bmp");
			filename_bitmap[RIGHT_MIDDLE] = _T("SFF_Right_Middle.bmp");
			filename_bitmap[RIGHT_RING] = _T("SFF_Right_Ring.bmp");
			filename_bitmap[RIGHT_LITTLE] = _T("SFF_Right_Little.bmp");

			filename_wsq[LEFT_HAND] =_T("SFF_LEFT_Four.wsq");
			filename_wsq[BOTH_THUMBS] = _T("STF_Both_Thumbs.wsq");
			filename_wsq[RIGHT_HAND] =_T("SFF_RIGHT_Four.wsq");

			filename_wsq[LEFT_LITTLE] = _T("SFF_Left_Little.wsq");
			filename_wsq[LEFT_RING] = _T("SFF_Left_Ring.wsq");
			filename_wsq[LEFT_MIDDLE] = _T("SFF_Left_Middle.wsq");
			filename_wsq[LEFT_INDEX] = _T("SFF_Left_Index.wsq");
			filename_wsq[LEFT_THUMB] = _T("SFF_Left_Thumb.wsq");
			filename_wsq[RIGHT_THUMB] = _T("SFF_Right_Thumb.wsq");
			filename_wsq[RIGHT_INDEX] = _T("SFF_Right_Index.wsq");
			filename_wsq[RIGHT_MIDDLE] = _T("SFF_Right_Middle.wsq");
			filename_wsq[RIGHT_RING] = _T("SFF_Right_Ring.wsq");
			filename_wsq[RIGHT_LITTLE] = _T("SFF_Right_Little.wsq");

			filename_iso[LEFT_HAND] =_T("SFF_LEFT_Four.fir");
			filename_iso[BOTH_THUMBS] = _T("STF_Both_Thumbs.fir");
			filename_iso[RIGHT_HAND] =_T("SFF_RIGHT_Four.fir");

			filename_iso[LEFT_LITTLE] = _T("SFF_Left_Little.fir");
			filename_iso[LEFT_RING] = _T("SFF_Left_Ring.fir");
			filename_iso[LEFT_MIDDLE] = _T("SFF_Left_Middle.fir");
			filename_iso[LEFT_INDEX] = _T("SFF_Left_Index.fir");
			filename_iso[LEFT_THUMB] = _T("SFF_Left_Thumb.fir");
			filename_iso[RIGHT_THUMB] = _T("SFF_Right_Thumb.fir");
			filename_iso[RIGHT_INDEX] = _T("SFF_Right_Index.fir");
			filename_iso[RIGHT_MIDDLE] = _T("SFF_Right_Middle.fir");
			filename_iso[RIGHT_RING] = _T("SFF_Right_Ring.fir");
			filename_iso[RIGHT_LITTLE] = _T("SFF_Right_Little.fir");

			BOOL bkColor = TRUE;		// background is white

			int Fingerarray[3]={LEFT_HAND,BOTH_THUMBS,RIGHT_HAND};
			m_ProgressSave.SetRange(0, 12);

			for(i=0; i<=2; i++)
			{
				m_ProgressSave.SetPos(i);

				if(m_pParent->m_OptionDlg.m_chkUseBitmap)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
					if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
						m_nCurrWidth_3/*1600*/, m_nCurrHeight_3, -m_nCurrWidth_3, 500, 500 ) != IBSU_STATUS_OK )
					{
						MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
					}
				}
				if(m_pParent->m_OptionDlg.m_chkUseWsq)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
					if( SaveWSQ(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3,m_nCurrHeight_3) == 0)
					{
						MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
					}
				}
				if(m_pParent->m_OptionDlg.m_chkUseIso)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
					if( SaveISOFIR(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3,m_nCurrHeight_3) == 0)
					{
						MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
					}
				}
			}

			for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
			{
				m_ProgressSave.SetPos(i);
				if(m_pParent->m_OptionDlg.m_chkUseBitmap)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[i]);

					if( IBSU_SaveBitmapImage( full_filename,(BYTE*)m_ImgBuf[i],m_nCurrentWidthForSplit[i], m_nCurrentHeightForSplit[i] ,-m_nCurrentWidthForSplit[i],500,500) != IBSU_STATUS_OK )
					{					
						MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
					}
				}

				if(m_pParent->m_OptionDlg.m_chkUseWsq)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[i]);

					if( SaveWSQ(full_filename, m_ImgBuf[i], m_nCurrentWidthForSplit[i], m_nCurrentHeightForSplit[i]) == 0)
					{
						MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
					}
				}

				if(m_pParent->m_OptionDlg.m_chkUseIso)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[i]);

					if( SaveISOFIR(full_filename, m_ImgBuf[i], m_nCurrentWidthForSplit[i], m_nCurrentHeightForSplit[i]) == 0)
					{
						MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
					}
				}
			}
			m_ProgressSave.SetPos(0);
			MessageBox( _T("Done!"),"CBP Demo");
		}

		//kojak 2nd 442roll
		else if( m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
				 m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH )
		{
			filename_bitmap[LEFT_HAND] =_T("SFF_LEFT_Four.bmp");
			filename_bitmap[BOTH_THUMBS] = _T("STF_Both_Thumbs.bmp");
			filename_bitmap[RIGHT_HAND] =_T("SFF_RIGHT_Four.bmp");

			filename_bitmap[LEFT_LITTLE] = _T("SRF_Left_Little.bmp");
			filename_bitmap[LEFT_RING] = _T("SRF_Left_Ring.bmp");
			filename_bitmap[LEFT_MIDDLE] = _T("SRF_Left_Middle.bmp");
			filename_bitmap[LEFT_INDEX] = _T("SRF_Left_Index.bmp");
			filename_bitmap[LEFT_THUMB] = _T("SRF_Left_Thumb.bmp");
			filename_bitmap[RIGHT_THUMB] = _T("SRF_Right_Thumb.bmp");
			filename_bitmap[RIGHT_INDEX] = _T("SRF_Right_Index.bmp");
			filename_bitmap[RIGHT_MIDDLE] = _T("SRF_Right_Middle.bmp");
			filename_bitmap[RIGHT_RING] = _T("SRF_Right_Ring.bmp");
			filename_bitmap[RIGHT_LITTLE] = _T("SRF_Right_Little.bmp");

			filename_wsq[LEFT_HAND] =_T("SFF_LEFT_Four.wsq");
			filename_wsq[BOTH_THUMBS] = _T("STF_Both_Thumbs.wsq");
			filename_wsq[RIGHT_HAND] =_T("SFF_RIGHT_Four.wsq");

			filename_wsq[LEFT_LITTLE] = _T("SRF_Left_Little.wsq");
			filename_wsq[LEFT_RING] = _T("SRF_Left_Ring.wsq");
			filename_wsq[LEFT_MIDDLE] = _T("SRF_Left_Middle.wsq");
			filename_wsq[LEFT_INDEX] = _T("SRF_Left_Index.wsq");
			filename_wsq[LEFT_THUMB] = _T("SRF_Left_Thumb.wsq");
			filename_wsq[RIGHT_THUMB] = _T("SRF_Right_Thumb.wsq");
			filename_wsq[RIGHT_INDEX] = _T("SRF_Right_Index.wsq");
			filename_wsq[RIGHT_MIDDLE] = _T("SRF_Right_Middle.wsq");
			filename_wsq[RIGHT_RING] = _T("SRF_Right_Ring.wsq");
			filename_wsq[RIGHT_LITTLE] = _T("SRF_Right_Little.wsq");

			filename_iso[LEFT_HAND] =_T("SFF_LEFT_Four.fir");
			filename_iso[BOTH_THUMBS] = _T("STF_Both_Thumbs.fir");
			filename_iso[RIGHT_HAND] =_T("SFF_RIGHT_Four.fir");

			filename_iso[LEFT_LITTLE] = _T("SRF_Left_Little.fir");
			filename_iso[LEFT_RING] = _T("SRF_Left_Ring.fir");
			filename_iso[LEFT_MIDDLE] = _T("SRF_Left_Middle.fir");
			filename_iso[LEFT_INDEX] = _T("SRF_Left_Index.fir");
			filename_iso[LEFT_THUMB] = _T("SRF_Left_Thumb.fir");
			filename_iso[RIGHT_THUMB] = _T("SRF_Right_Thumb.fir");
			filename_iso[RIGHT_INDEX] = _T("SRF_Right_Index.fir");
			filename_iso[RIGHT_MIDDLE] = _T("SRF_Right_Middle.fir");
			filename_iso[RIGHT_RING] = _T("SRF_Right_Ring.fir");
			filename_iso[RIGHT_LITTLE] = _T("SRF_Right_Little.fir");

			BOOL bkColor = TRUE;		// background is white

			int Fingerarray[3]={LEFT_HAND,BOTH_THUMBS,RIGHT_HAND};
			m_ProgressSave.SetRange(0, 12);

			for(i=0; i<=2; i++)
			{
				m_ProgressSave.SetPos(i);

				if(m_pParent->m_OptionDlg.m_chkUseBitmap)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
					if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
						m_nCurrWidth_3/*1600*/, m_nCurrHeight_3, -m_nCurrWidth_3, 500, 500 ) != IBSU_STATUS_OK )
					{
						MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
					}	

				}
				if(m_pParent->m_OptionDlg.m_chkUseWsq)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
					if( SaveWSQ(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3,m_nCurrHeight_3) == 0)
					{
						MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
					}
				}
				if(m_pParent->m_OptionDlg.m_chkUseIso)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
					if( SaveISOFIR(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3,m_nCurrHeight_3) == 0)
					{
						MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
					}
				}
			}

			for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
			{
				m_nCurrWidth = IMG_W;
				m_nCurrHeight =IMG_H;
				m_ProgressSave.SetPos(i);
				if(m_pParent->m_OptionDlg.m_chkUseBitmap)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[i]);

					if( IBSU_SaveBitmapImage( full_filename,(BYTE*)m_ImgBuf[i],m_nCurrWidth, m_nCurrHeight ,-m_nCurrWidth,500,500) != IBSU_STATUS_OK )
					{					
						MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
					}
				}

				if(m_pParent->m_OptionDlg.m_chkUseWsq)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[i]);

					if( SaveWSQ(full_filename, m_ImgBuf[i], m_nCurrWidth,m_nCurrHeight) == 0)
					{
						MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
					}
				}

				if(m_pParent->m_OptionDlg.m_chkUseIso)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[i]);

					if( SaveISOFIR(full_filename, m_ImgBuf[i], m_nCurrWidth,m_nCurrHeight) == 0)
					{
						MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
					}
				}
			}
			m_ProgressSave.SetPos(0);
			MessageBox( _T("Done!"),"CBP Demo");
		}
		//3
		else if( m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH )
		{
			filename_bitmap[LEFT_HAND] =_T("SFF_LEFT_Four.bmp");
			filename_bitmap[BOTH_THUMBS] = _T("STF_Both_Thumbs.bmp");
			filename_bitmap[RIGHT_HAND] =_T("SFF_RIGHT_Four.bmp");

			filename_wsq[LEFT_HAND] =_T("SFF_LEFT_Four.wsq");
			filename_wsq[BOTH_THUMBS] = _T("STF_Both_Thumbs.wsq");
			filename_wsq[RIGHT_HAND] =_T("SFF_RIGHT_Four.wsq");

			filename_iso[LEFT_HAND] =_T("SFF_LEFT_Four.fir");
			filename_iso[BOTH_THUMBS] = _T("STF_Both_Thumbs.fir");
			filename_iso[RIGHT_HAND] =_T("SFF_RIGHT_Four.fir");

			BOOL bkColor = TRUE;		// background is white

			int Fingerarray[3]={LEFT_HAND,BOTH_THUMBS,RIGHT_HAND};
			m_ProgressSave.SetRange(0, 2);

			for(i=0; i<=2; i++)
			{
				m_ProgressSave.SetPos(i);

				if(i==0 || i==2)
				{
					if(m_pParent->m_OptionDlg.m_chkUseBitmap)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
						if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
							m_nCurrWidth_3/*1600*/, m_nCurrHeight_3, -m_nCurrWidth_3, 500, 500 ) != IBSU_STATUS_OK )
						{
							MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseWsq)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
						if( SaveWSQ(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3,m_nCurrHeight_3) == 0)
						{
							MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseIso)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
						if( SaveISOFIR(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3,m_nCurrHeight_3) == 0)
						{
							MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
						}
					}
				}else if(i==1)
				{
					if(m_pParent->m_OptionDlg.m_chkUseBitmap)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
						if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
							m_nCurrWidth_3/*1600*/, m_nCurrHeight_3, -m_nCurrWidth_3, 500, 500 ) != IBSU_STATUS_OK )
						{
							MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						}	
					}
					if(m_pParent->m_OptionDlg.m_chkUseWsq)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
						if( SaveWSQ(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3,m_nCurrHeight_3) == 0)
						{
							MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseIso)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
						if( SaveISOFIR(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3,m_nCurrHeight_3) == 0)
						{
							MessageBox( _T("Failed to save 19794-4 image!"),"CBP Demo");
						}
					}
				}
			}
			m_ProgressSave.SetPos(0);
			MessageBox( _T("Done!"),"CBP Demo");
		}
		//3
		else if( m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH )
		{
			filename_bitmap[LEFT_HAND] =_T("SFF_LEFT_Four.bmp");
			filename_bitmap[THUMB_SUM] = _T("STF_Thumbs.bmp");
			filename_bitmap[RIGHT_HAND] =_T("SFF_RIGHT_Four.bmp");

			filename_wsq[LEFT_HAND] =_T("SFF_LEFT_Four.wsq");
			filename_wsq[THUMB_SUM] = _T("STF_Thumbs.wsq");
			filename_wsq[RIGHT_HAND] =_T("SFF_RIGHT_Four.wsq");

			filename_iso[LEFT_HAND] =_T("SFF_LEFT_Four.fir");
			filename_iso[THUMB_SUM] = _T("STF_Thumbs.fir");
			filename_iso[RIGHT_HAND] =_T("SFF_RIGHT_Four.fir");

			BOOL bkColor = TRUE;		// background is white

			int Fingerarray[3]={LEFT_HAND,THUMB_SUM,RIGHT_HAND};
			m_ProgressSave.SetRange(0,2);
			for(i=0; i<=2; i++)
			{
				m_ProgressSave.SetPos(i);
				if(m_pParent->m_OptionDlg.m_chkUseBitmap)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);

					if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
						m_nCurrWidth_3/*1600*/, m_nCurrHeight_3, -m_nCurrWidth_3, 500, 500 ) != IBSU_STATUS_OK )
					{
						MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						break;
					}
				}
				if(m_pParent->m_OptionDlg.m_chkUseWsq)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
					if( SaveWSQ(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3,m_nCurrHeight_3) == 0)
					{
						MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
					}
				}
				if(m_pParent->m_OptionDlg.m_chkUseIso)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
					if( SaveISOFIR(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3,m_nCurrHeight_3) == 0)
					{
						MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
					}
				}
			}
			m_ProgressSave.SetPos(0);
			MessageBox( _T("Done!"),"CBP Demo");
		}
		//14 single
		else if( m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE)
		{
			filename_bitmap[LEFT_HAND] =_T("SFF_LEFT_Four.bmp");
			filename_bitmap[BOTH_LEFT_THUMB] = _T("STF_Both_Left_Thumbs.bmp");
			filename_bitmap[BOTH_RIGHT_THUMB] = _T("STF_Both_Right_Thumbs.bmp");
			filename_bitmap[RIGHT_HAND] =_T("SFF_RIGHT_Four.bmp");
			filename_bitmap[LEFT_LITTLE] = _T("SFF_Left_Little.bmp");
			filename_bitmap[LEFT_RING] = _T("SFF_Left_Ring.bmp");
			filename_bitmap[LEFT_MIDDLE] = _T("SFF_Left_Middle.bmp");
			filename_bitmap[LEFT_INDEX] = _T("SFF_Left_Index.bmp");
			filename_bitmap[LEFT_THUMB] = _T("SFF_Left_Thumb.bmp");
			filename_bitmap[RIGHT_THUMB] = _T("SFF_Right_Thumb.bmp");
			filename_bitmap[RIGHT_INDEX] = _T("SFF_Right_Index.bmp");
			filename_bitmap[RIGHT_MIDDLE] = _T("SFF_Right_Middle.bmp");
			filename_bitmap[RIGHT_RING] = _T("SFF_Right_Ring.bmp");
			filename_bitmap[RIGHT_LITTLE] = _T("SFF_Right_Little.bmp");

			filename_wsq[LEFT_HAND] =_T("SFF_LEFT_Four.wsq");
			filename_wsq[BOTH_LEFT_THUMB] = _T("STF_Both_Left_Thumbs.wsq");
			filename_wsq[BOTH_RIGHT_THUMB] = _T("STF_Both_Right_Thumbs.wsq");
			filename_wsq[RIGHT_HAND] =_T("SFF_RIGHT_Four.wsq");
			filename_wsq[LEFT_LITTLE] = _T("SFF_Left_Little.wsq");
			filename_wsq[LEFT_RING] = _T("SFF_Left_Ring.wsq");
			filename_wsq[LEFT_MIDDLE] = _T("SFF_Left_Middle.wsq");
			filename_wsq[LEFT_INDEX] = _T("SFF_Left_Index.wsq");
			filename_wsq[LEFT_THUMB] = _T("SFF_Left_Thumb.wsq");
			filename_wsq[RIGHT_THUMB] = _T("SFF_Right_Thumb.wsq");
			filename_wsq[RIGHT_INDEX] = _T("SFF_Right_Index.wsq");
			filename_wsq[RIGHT_MIDDLE] = _T("SFF_Right_Middle.wsq");
			filename_wsq[RIGHT_RING] = _T("SFF_Right_Ring.wsq");
			filename_wsq[RIGHT_LITTLE] = _T("SFF_Right_Little.wsq");

			filename_iso[LEFT_HAND] =_T("SFF_LEFT_Four.fir");
			filename_iso[BOTH_LEFT_THUMB] = _T("STF_Both_Left_Thumbs.fir");
			filename_iso[BOTH_RIGHT_THUMB] = _T("STF_Both_Right_Thumbs.fir");
			filename_iso[RIGHT_HAND] =_T("SFF_RIGHT_Four.fir");
			filename_iso[LEFT_LITTLE] = _T("SFF_Left_Little.fir");
			filename_iso[LEFT_RING] = _T("SFF_Left_Ring.fir");
			filename_iso[LEFT_MIDDLE] = _T("SFF_Left_Middle.fir");
			filename_iso[LEFT_INDEX] = _T("SFF_Left_Index.fir");
			filename_iso[LEFT_THUMB] = _T("SFF_Left_Thumb.fir");
			filename_iso[RIGHT_THUMB] = _T("SFF_Right_Thumb.fir");
			filename_iso[RIGHT_INDEX] = _T("SFF_Right_Index.fir");
			filename_iso[RIGHT_MIDDLE] = _T("SFF_Right_Middle.fir");
			filename_iso[RIGHT_RING] = _T("SFF_Right_Ring.fir");
			filename_iso[RIGHT_LITTLE] = _T("SFF_Right_Little.fir");

			BOOL bkColor = TRUE;		// background is white
			int Fingerarray[14]={LEFT_HAND, BOTH_LEFT_THUMB, BOTH_RIGHT_THUMB ,RIGHT_HAND,
				LEFT_LITTLE,LEFT_RING,LEFT_MIDDLE,LEFT_INDEX,LEFT_THUMB,
				RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE};

			m_ProgressSave.SetRange(0,13);

			for(i=0; i<=13; i++)
			{
				m_ProgressSave.SetPos(i);
				if(i==0 || i== 3)
				{
					if(m_pParent->m_OptionDlg.m_chkUseBitmap)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
						if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
							m_nCurrWidth_3, m_nCurrHeight_3, -m_nCurrWidth_3, 500, 500 ) != IBSU_STATUS_OK )
						{
							MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseWsq)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
						if( SaveWSQ(full_filename,(BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3,m_nCurrHeight_3) == 0)
						{
							MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseIso)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
						if( SaveISOFIR(full_filename,(BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3,m_nCurrHeight_3) == 0)
						{
							MessageBox( _T("Failed to save ISO 19794-4  image!"),"CBP Demo");
						}
					}
				}	
				else if(i ==1 )
				{
					//pitchForSplit =0-m_nCurrentWidthForSplit[BOTH_LEFT_THUMB];
					if(m_pParent->m_OptionDlg.m_chkUseBitmap)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
						if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[BOTH_LEFT_THUMB], 
							m_nCurrentWidthForSplit[BOTH_LEFT_THUMB], m_nCurrentHeightForSplit[BOTH_LEFT_THUMB], -m_nCurrentWidthForSplit[BOTH_LEFT_THUMB], 500, 500 ) != IBSU_STATUS_OK )
						{
							MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseWsq)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
						if( SaveWSQ(full_filename, m_ImgBuf[BOTH_LEFT_THUMB], m_nCurrentWidthForSplit[BOTH_LEFT_THUMB], m_nCurrentHeightForSplit[BOTH_LEFT_THUMB]) == 0)
						{
							MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
						}

					}
					if(m_pParent->m_OptionDlg.m_chkUseIso)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
						if( SaveISOFIR(full_filename, m_ImgBuf[BOTH_LEFT_THUMB], m_nCurrentWidthForSplit[BOTH_LEFT_THUMB], m_nCurrentHeightForSplit[BOTH_LEFT_THUMB]) == 0)
						{
							MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
						}
					}
				}
				else if(i==2)
				{
					//pitchForSplit =0-m_nCurrentWidthForSplit[BOTH_RIGHT_THUMB];
					if(m_pParent->m_OptionDlg.m_chkUseBitmap)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
						if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[BOTH_RIGHT_THUMB], 
							m_nCurrentWidthForSplit[BOTH_RIGHT_THUMB], m_nCurrentHeightForSplit[BOTH_RIGHT_THUMB], -m_nCurrentWidthForSplit[BOTH_RIGHT_THUMB], 500, 500 ) != IBSU_STATUS_OK )
						{
							MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						}

					}
					if(m_pParent->m_OptionDlg.m_chkUseWsq)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
						if( SaveWSQ(full_filename, m_ImgBuf[BOTH_RIGHT_THUMB], m_nCurrentWidthForSplit[BOTH_RIGHT_THUMB], m_nCurrentHeightForSplit[BOTH_RIGHT_THUMB]) == 0)
						{
							MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
						}

					}
					if(m_pParent->m_OptionDlg.m_chkUseIso)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
						if( SaveISOFIR(full_filename, m_ImgBuf[BOTH_RIGHT_THUMB], m_nCurrentWidthForSplit[BOTH_RIGHT_THUMB], m_nCurrentHeightForSplit[BOTH_RIGHT_THUMB]) == 0)
						{
							MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
						}
					}
				}
				else if(i>3)
				{
					if(m_pParent->m_OptionDlg.m_chkUseBitmap)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
						if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
							m_nCurrWidth, m_nCurrHeight, -m_nCurrWidth, 500, 500 ) != IBSU_STATUS_OK )
						{
							MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseWsq)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
						if( SaveWSQ( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]],m_nCurrWidth, m_nCurrHeight) == 0 )
						{
							MessageBox( _T( "Failed to save wsq image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseIso)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
						if( SaveISOFIR( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]],m_nCurrWidth, m_nCurrHeight) == 0 )
						{
							MessageBox( _T( "Failed to save ISO 19794-4 image!" ),"CBP Demo");
						}
					}
				}
			}
			m_ProgressSave.SetPos(0);
			MessageBox( _T("Done!"),"CBP Demo");

		}
		//14 roll each
		else if( m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH)
		{
			filename_bitmap[LEFT_HAND]			=_T("SFF_LEFT_Four.bmp");
			filename_bitmap[BOTH_LEFT_THUMB]	=_T("SFF_LEft_Thumbs.bmp");
			filename_bitmap[BOTH_RIGHT_THUMB]	=_T("SFF_Right_Thumbs.bmp");
			filename_bitmap[RIGHT_HAND]			=_T("SFF_RIGHT_Four.bmp");
			filename_bitmap[LEFT_LITTLE]		=_T("SRF_Left_Little.bmp");
			filename_bitmap[LEFT_RING]			=_T("SRF_Left_Ring.bmp");
			filename_bitmap[LEFT_MIDDLE]		=_T("SRF_Left_Middle.bmp");
			filename_bitmap[LEFT_INDEX]			=_T("SRF_Left_Index.bmp");
			filename_bitmap[LEFT_THUMB]			=_T("SRF_Left_Thumb.bmp");
			filename_bitmap[RIGHT_THUMB]		=_T("SRF_Right_Thumb.bmp");
			filename_bitmap[RIGHT_INDEX]		=_T("SRF_Right_Index.bmp");
			filename_bitmap[RIGHT_MIDDLE]		=_T("SRF_Right_Middle.bmp");
			filename_bitmap[RIGHT_RING]			=_T("SRF_Right_Ring.bmp");
			filename_bitmap[RIGHT_LITTLE]		=_T("SRF_Right_Little.bmp");

			filename_wsq[LEFT_HAND]			=_T("SFF_LEFT_Four.wsq");
			filename_wsq[BOTH_LEFT_THUMB]	=_T("SFF_LEft_Thumbs.wsq");
			filename_wsq[BOTH_RIGHT_THUMB]	=_T("SFF_Right_Thumbs.wsq");
			filename_wsq[RIGHT_HAND]		=_T("SFF_RIGHT_Four.wsq");
			filename_wsq[LEFT_LITTLE]		=_T("SRF_Left_Little.wsq");
			filename_wsq[LEFT_RING]			=_T("SRF_Left_Ring.wsq");
			filename_wsq[LEFT_MIDDLE]		=_T("SRF_Left_Middle.wsq");
			filename_wsq[LEFT_INDEX]		=_T("SRF_Left_Index.wsq");
			filename_wsq[LEFT_THUMB]		=_T("SRF_Left_Thumb.wsq");
			filename_wsq[RIGHT_THUMB]		=_T("SRF_Right_Thumb.wsq");
			filename_wsq[RIGHT_INDEX]		=_T("SRF_Right_Index.wsq");
			filename_wsq[RIGHT_MIDDLE]		=_T("SRF_Right_Middle.wsq");
			filename_wsq[RIGHT_RING]		=_T("SRF_Right_Ring.wsq");
			filename_wsq[RIGHT_LITTLE]		=_T("SRF_Right_Little.wsq");

			filename_iso[LEFT_HAND]			=_T("SFF_LEFT_Four.fir");
			filename_iso[BOTH_LEFT_THUMB]	=_T("SFF_LEft_Thumbs.fir");
			filename_iso[BOTH_RIGHT_THUMB]	=_T("SFF_Right_Thumbs.fir");
			filename_iso[RIGHT_HAND]		=_T("SFF_RIGHT_Four.fir");
			filename_iso[LEFT_LITTLE]		=_T("SRF_Left_Little.fir");
			filename_iso[LEFT_RING]			=_T("SRF_Left_Ring.fir");
			filename_iso[LEFT_MIDDLE]		=_T("SRF_Left_Middle.fir");
			filename_iso[LEFT_INDEX]		=_T("SRF_Left_Index.fir");
			filename_iso[LEFT_THUMB]		=_T("SRF_Left_Thumb.fir");
			filename_iso[RIGHT_THUMB]		=_T("SRF_Right_Thumb.fir");
			filename_iso[RIGHT_INDEX]		=_T("SRF_Right_Index.fir");
			filename_iso[RIGHT_MIDDLE]		=_T("SRF_Right_Middle.fir");
			filename_iso[RIGHT_RING]		=_T("SRF_Right_Ring.fir");
			filename_iso[RIGHT_LITTLE]		=_T("SRF_Right_Little.fir");

			BOOL bkColor = TRUE;		// background is white
			int Fingerarray[14]={LEFT_HAND,BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB,RIGHT_HAND,
				LEFT_LITTLE,LEFT_RING,LEFT_MIDDLE,LEFT_INDEX,
				LEFT_THUMB,RIGHT_THUMB,RIGHT_INDEX,
				RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE};

			m_ProgressSave.SetRange(0,13);

			for(i=0; i<=13; i++)
			{
				m_ProgressSave.SetPos(i);
				if(i==0 || i== 3)
				{
					if(m_pParent->m_OptionDlg.m_chkUseBitmap)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
						if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
							m_nCurrWidth_3, m_nCurrHeight_3, -m_nCurrWidth_3, 500, 500 ) != IBSU_STATUS_OK )
						{
							MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseWsq)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
						if( SaveWSQ( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3, m_nCurrHeight_3) == 0 )
						{
							MessageBox( _T( "Failed to save wsq image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseIso)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
						if( SaveISOFIR( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3, m_nCurrHeight_3) == 0 )
						{
							MessageBox( _T( "Failed to save 19794-4 image!" ),"CBP Demo");
						}
					}
				}
				else
				{
					if(m_pParent->m_OptionDlg.m_chkUseBitmap)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
						if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
							m_nCurrWidth, m_nCurrHeight, -m_nCurrWidth, 500, 500 ) != IBSU_STATUS_OK )
						{
							MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseWsq)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
						if( SaveWSQ( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth, m_nCurrHeight) == 0 )
						{
							MessageBox( _T( "Failed to save wsq image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseIso)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
						if( SaveISOFIR( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth, m_nCurrHeight) == 0 )
						{
							MessageBox( _T( "Failed to save ISO 19794-4 image!" ),"CBP Demo");
						}
					}
				}
			}
			m_ProgressSave.SetPos(0);
			MessageBox( _T("Done!"),"CBP Demo");
		}
		//14 roll
		else if( m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH)
		{
			filename_bitmap[LEFT_HAND]			=_T("SFF_LEFT_Four.bmp");
			filename_bitmap[BOTH_LEFT_THUMB]	=_T("SFF_LEft_Thumbs.bmp");
			filename_bitmap[BOTH_RIGHT_THUMB]	=_T("SFF_Right_Thumbs.bmp");
			filename_bitmap[RIGHT_HAND]			=_T("SFF_RIGHT_Four.bmp");
			filename_bitmap[LEFT_LITTLE]		=_T("SRF_Left_Little.bmp");
			filename_bitmap[LEFT_RING]			=_T("SRF_Left_Ring.bmp");
			filename_bitmap[LEFT_MIDDLE]		=_T("SRF_Left_Middle.bmp");
			filename_bitmap[LEFT_INDEX]			=_T("SRF_Left_Index.bmp");
			filename_bitmap[LEFT_THUMB]			=_T("SRF_Left_Thumb.bmp");
			filename_bitmap[RIGHT_THUMB]		=_T("SRF_Right_Thumb.bmp");
			filename_bitmap[RIGHT_INDEX]		=_T("SRF_Right_Index.bmp");
			filename_bitmap[RIGHT_MIDDLE]		=_T("SRF_Right_Middle.bmp");
			filename_bitmap[RIGHT_RING]			=_T("SRF_Right_Ring.bmp");
			filename_bitmap[RIGHT_LITTLE]		=_T("SRF_Right_Little.bmp");

			filename_wsq[LEFT_HAND]			=_T("SFF_LEFT_Four.wsq");
			filename_wsq[BOTH_LEFT_THUMB]	=_T("SFF_LEft_Thumbs.wsq");
			filename_wsq[BOTH_RIGHT_THUMB]	=_T("SFF_Right_Thumbs.wsq");
			filename_wsq[RIGHT_HAND]		=_T("SFF_RIGHT_Four.wsq");
			filename_wsq[LEFT_LITTLE]		=_T("SRF_Left_Little.wsq");
			filename_wsq[LEFT_RING]			=_T("SRF_Left_Ring.wsq");
			filename_wsq[LEFT_MIDDLE]		=_T("SRF_Left_Middle.wsq");
			filename_wsq[LEFT_INDEX]		=_T("SRF_Left_Index.wsq");
			filename_wsq[LEFT_THUMB]		=_T("SRF_Left_Thumb.wsq");
			filename_wsq[RIGHT_THUMB]		=_T("SRF_Right_Thumb.wsq");
			filename_wsq[RIGHT_INDEX]		=_T("SRF_Right_Index.wsq");
			filename_wsq[RIGHT_MIDDLE]		=_T("SRF_Right_Middle.wsq");
			filename_wsq[RIGHT_RING]		=_T("SRF_Right_Ring.wsq");
			filename_wsq[RIGHT_LITTLE]		=_T("SRF_Right_Little.wsq");

			filename_iso[LEFT_HAND]			=_T("SFF_LEFT_Four.fir");
			filename_iso[BOTH_LEFT_THUMB]	=_T("SFF_LEft_Thumbs.fir");
			filename_iso[BOTH_RIGHT_THUMB]	=_T("SFF_Right_Thumbs.fir");
			filename_iso[RIGHT_HAND]		=_T("SFF_RIGHT_Four.fir");
			filename_iso[LEFT_LITTLE]		=_T("SRF_Left_Little.fir");
			filename_iso[LEFT_RING]			=_T("SRF_Left_Ring.fir");
			filename_iso[LEFT_MIDDLE]		=_T("SRF_Left_Middle.fir");
			filename_iso[LEFT_INDEX]		=_T("SRF_Left_Index.fir");
			filename_iso[LEFT_THUMB]		=_T("SRF_Left_Thumb.fir");
			filename_iso[RIGHT_THUMB]		=_T("SRF_Right_Thumb.fir");
			filename_iso[RIGHT_INDEX]		=_T("SRF_Right_Index.fir");
			filename_iso[RIGHT_MIDDLE]		=_T("SRF_Right_Middle.fir");
			filename_iso[RIGHT_RING]		=_T("SRF_Right_Ring.fir");
			filename_iso[RIGHT_LITTLE]		=_T("SRF_Right_Little.fir");

			BOOL bkColor = TRUE;		// background is white
			int Fingerarray[14]={LEFT_HAND, BOTH_LEFT_THUMB, BOTH_RIGHT_THUMB ,RIGHT_HAND,
				LEFT_LITTLE,LEFT_RING,LEFT_MIDDLE,LEFT_INDEX,LEFT_THUMB,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE};

			m_ProgressSave.SetRange(0, 13);

			for(i=0; i<13; i++)
			{
				m_ProgressSave.SetPos(i);
				if(i==0 || i== 3)
				{
					if(m_pParent->m_OptionDlg.m_chkUseBitmap)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
						if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
							m_nCurrWidth_3, m_nCurrHeight_3, -m_nCurrWidth_3, 500, 500 ) != IBSU_STATUS_OK )
						{
							MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseWsq)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
						if( SaveWSQ( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3, m_nCurrHeight_3) == 0 )
						{
							MessageBox( _T( "Failed to save wsq image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseIso)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
						if( SaveISOFIR( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3, m_nCurrHeight_3) == 0 )
						{
							MessageBox( _T( "Failed to save ISO 19794-4 image!" ),"CBP Demo");
						}
					}
				}
				else if(i ==1 )
				{
					//pitchForSplit =0-m_nCurrentWidthForSplit[BOTH_LEFT_THUMB];
					if(m_pParent->m_OptionDlg.m_chkUseBitmap)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
						if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[BOTH_LEFT_THUMB], 
							m_nCurrentWidthForSplit[BOTH_LEFT_THUMB], m_nCurrentHeightForSplit[BOTH_LEFT_THUMB], -m_nCurrentWidthForSplit[BOTH_LEFT_THUMB], 500, 500 ) != IBSU_STATUS_OK )
						{
							MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseWsq)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
						if( SaveWSQ(full_filename, m_ImgBuf[BOTH_LEFT_THUMB], m_nCurrentWidthForSplit[BOTH_LEFT_THUMB], m_nCurrentHeightForSplit[BOTH_LEFT_THUMB]) == 0)
						{
							MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseIso)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
						if( SaveISOFIR(full_filename, m_ImgBuf[BOTH_LEFT_THUMB], m_nCurrentWidthForSplit[BOTH_LEFT_THUMB], m_nCurrentHeightForSplit[BOTH_LEFT_THUMB]) == 0)
						{
							MessageBox( _T("Failed to save 19794-4 image!"),"CBP Demo");
						}
					}
				}
				else if(i==2)
				{
					if(m_pParent->m_OptionDlg.m_chkUseBitmap)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
						if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[BOTH_RIGHT_THUMB], 
							m_nCurrentWidthForSplit[BOTH_RIGHT_THUMB], m_nCurrentHeightForSplit[BOTH_RIGHT_THUMB], -m_nCurrentWidthForSplit[BOTH_RIGHT_THUMB], 500, 500 ) != IBSU_STATUS_OK )
						{
							MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						}

					}
					if(m_pParent->m_OptionDlg.m_chkUseWsq)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
						if( SaveWSQ(full_filename, m_ImgBuf[BOTH_RIGHT_THUMB], m_nCurrentWidthForSplit[BOTH_RIGHT_THUMB], m_nCurrentHeightForSplit[BOTH_RIGHT_THUMB]) == 0)
						{
							MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseIso)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
						if( SaveISOFIR(full_filename, m_ImgBuf[BOTH_RIGHT_THUMB], m_nCurrentWidthForSplit[BOTH_RIGHT_THUMB], m_nCurrentHeightForSplit[BOTH_RIGHT_THUMB]) == 0)
						{
							MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
						}
					}
				}
				else if(i>3)
				{
					if(m_pParent->m_OptionDlg.m_chkUseBitmap)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
						if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
							m_nCurrWidth, m_nCurrHeight, -m_nCurrWidth, 500, 500 ) != IBSU_STATUS_OK )
						{
							MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseWsq)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
						if( SaveWSQ(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth, m_nCurrHeight) == 0)
						{
							MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseIso)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
						if( SaveISOFIR(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth, m_nCurrHeight) == 0)
						{
							MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
						}
					}
				}
			}

			m_ProgressSave.SetPos(0);
			MessageBox( _T("Done!"),"CBP Demo");
		}
		else if(m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
		{
			filename_bitmap[LEFT_THUMB]	=_T("SFF_Left_Thumbs.bmp");
			filename_bitmap[RIGHT_THUMB]	=_T("SFF_Right_Thumbs.bmp");
			filename_bitmap[LEFT_INDEX]			=_T("SFF_Left_Index.bmp");
			filename_bitmap[RIGHT_INDEX]		=_T("SFF_Right_Index.bmp");

			filename_wsq[LEFT_THUMB]	=_T("SFF_Left_Thumbs.wsq");
			filename_wsq[RIGHT_THUMB]	=_T("SFF_Right_Thumbs.wsq");
			filename_wsq[LEFT_INDEX]		=_T("SFF_Left_Index.wsq");
			filename_wsq[RIGHT_INDEX]		=_T("SFF_Right_Index.wsq");

			filename_iso[LEFT_THUMB]	=_T("SFF_Left_Thumbs.fir");
			filename_iso[RIGHT_THUMB]	=_T("SFF_Right_Thumbs.fir");
			filename_iso[LEFT_INDEX]		=_T("SFF_Left_Index.fir");
			filename_iso[RIGHT_INDEX]		=_T("SFF_Right_Index.fir");

			BOOL bkColor = TRUE;		// background is white
			int Fingerarray[4]={LEFT_INDEX,LEFT_THUMB,RIGHT_THUMB,RIGHT_INDEX};

			m_ProgressSave.SetRange(0, 4);

			for(i=0; i<4; i++)
			{
				m_ProgressSave.SetPos(i);

				if(m_pParent->m_OptionDlg.m_chkUseBitmap)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
					if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
						m_nCurrWidth, m_nCurrHeight, -m_nCurrWidth, 500, 500 ) != IBSU_STATUS_OK )
					{
						MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
					}
				}
				if(m_pParent->m_OptionDlg.m_chkUseWsq)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
					if( SaveWSQ(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth, m_nCurrHeight) == 0)
					{
						MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
					}
				}
				if(m_pParent->m_OptionDlg.m_chkUseIso)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
					if( SaveISOFIR(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth, m_nCurrHeight) == 0)
					{
						MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
					}
				}
			}

			m_ProgressSave.SetPos(0);
			MessageBox( _T("Done!"),"CBP Demo");
		}
		else if(m_CaptureMode == TEN_FINGER_FLAT_THUMB)
		{
			filename_bitmap[LEFT_THUMB]	=_T("SFF_Left_Thumbs.bmp");
			filename_bitmap[RIGHT_THUMB]=_T("SFF_Right_Thumbs.bmp");	

			filename_wsq[LEFT_THUMB]	=_T("SFF_Left_Thumbs.wsq");
			filename_wsq[RIGHT_THUMB]	=_T("SFF_Right_Thumbs.wsq");

			filename_iso[LEFT_THUMB]	=_T("SFF_Left_Thumbs.fir");
			filename_iso[RIGHT_THUMB]	=_T("SFF_Right_Thumbs.fir");
		
			BOOL bkColor = TRUE;		// background is white
			int Fingerarray[2]={LEFT_THUMB,RIGHT_THUMB};

			m_ProgressSave.SetRange(0, 2);

			for(i=0; i<2; i++)
			{
				m_ProgressSave.SetPos(i);

				if(m_pParent->m_OptionDlg.m_chkUseBitmap)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
					if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
						m_nCurrWidth, m_nCurrHeight, -m_nCurrWidth, 500, 500 ) != IBSU_STATUS_OK )
					{
						MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
					}
				}
				if(m_pParent->m_OptionDlg.m_chkUseWsq)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
					if( SaveWSQ(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth, m_nCurrHeight) == 0)
					{
						MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
					}
				}
				if(m_pParent->m_OptionDlg.m_chkUseIso)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
					if( SaveISOFIR(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth, m_nCurrHeight) == 0)
					{
						MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
					}
				}
			}

			m_ProgressSave.SetPos(0);
			MessageBox( _T("Done!"),"CBP Demo");
		}
		//16
		else if( m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
		{
			filename_bitmap[LEFT_RING_LITTLE] =_T("SFF_Left_Ring_Little.bmp");
			filename_bitmap[LEFT_INDEX_MIDDLE] =_T("SFF_Left_Index_Middle.bmp");
			filename_bitmap[BOTH_LEFT_THUMB] = _T("STF_Both_Left_Thumb.bmp");
			filename_bitmap[BOTH_RIGHT_THUMB] = _T("STF_Both_Right_Thumb.bmp");
			filename_bitmap[RIGHT_INDEX_MIDDLE] =_T("SFF_Right_Index_Middle.bmp");
			filename_bitmap[RIGHT_RING_LITTLE] =_T("SFF_Right_Ring_Little.bmp");
			filename_bitmap[LEFT_LITTLE] = _T("SRF_Left_Little.bmp");
			filename_bitmap[LEFT_RING] = _T("SRF_Left_Ring.bmp");
			filename_bitmap[LEFT_MIDDLE] = _T("SRF_Left_Middle.bmp");
			filename_bitmap[LEFT_INDEX] = _T("SRF_Left_Index.bmp");
			filename_bitmap[LEFT_THUMB] = _T("SRF_Left_Thumb.bmp");
			filename_bitmap[RIGHT_THUMB] = _T("SRF_Right_Thumb.bmp");
			filename_bitmap[RIGHT_INDEX] = _T("SRF_Right_Index.bmp");
			filename_bitmap[RIGHT_MIDDLE] = _T("SRF_Right_Middle.bmp");
			filename_bitmap[RIGHT_RING] = _T("SRF_Right_Ring.bmp");
			filename_bitmap[RIGHT_LITTLE] = _T("SRF_Right_Little.bmp");

			filename_wsq[LEFT_RING_LITTLE] =_T("SFF_Left_Ring_Little.wsq");
			filename_wsq[LEFT_INDEX_MIDDLE] =_T("SFF_Left_Index_Middle.wsq");
			filename_wsq[BOTH_LEFT_THUMB] = _T("STF_Both_Left_Thumb.wsq");
			filename_wsq[BOTH_RIGHT_THUMB] = _T("STF_Both_Right_Thumb.wsq");
			filename_wsq[RIGHT_INDEX_MIDDLE] =_T("SFF_Right_Index_Middle.wsq");
			filename_wsq[RIGHT_RING_LITTLE] =_T("SFF_Right_Ring_Little.wsq");
			filename_wsq[LEFT_LITTLE] = _T("SRF_Left_Little.wsq");
			filename_wsq[LEFT_RING] = _T("SRF_Left_Ring.wsq");
			filename_wsq[LEFT_MIDDLE] = _T("SRF_Left_Middle.wsq");
			filename_wsq[LEFT_INDEX] = _T("SRF_Left_Index.wsq");
			filename_wsq[LEFT_THUMB] = _T("SRF_Left_Thumb.wsq");
			filename_wsq[RIGHT_THUMB] = _T("SRF_Right_Thumb.wsq");
			filename_wsq[RIGHT_INDEX] = _T("SRF_Right_Index.wsq");
			filename_wsq[RIGHT_MIDDLE] = _T("SRF_Right_Middle.wsq");
			filename_wsq[RIGHT_RING] = _T("SRF_Right_Ring.wsq");
			filename_wsq[RIGHT_LITTLE] = _T("SRF_Right_Little.wsq");

			filename_iso[LEFT_RING_LITTLE] =_T("SFF_Left_Ring_Little.fir");
			filename_iso[LEFT_INDEX_MIDDLE] =_T("SFF_Left_Index_Middle.fir");
			filename_iso[BOTH_LEFT_THUMB] = _T("STF_Both_Left_Thumb.fir");
			filename_iso[BOTH_RIGHT_THUMB] = _T("STF_Both_Right_Thumb.fir");
			filename_iso[RIGHT_INDEX_MIDDLE] =_T("SFF_Right_Index_Middle.fir");
			filename_iso[RIGHT_RING_LITTLE] =_T("SFF_Right_Ring_Little.fir");
			filename_iso[LEFT_LITTLE] = _T("SRF_Left_Little.fir");
			filename_iso[LEFT_RING] = _T("SRF_Left_Ring.fir");
			filename_iso[LEFT_MIDDLE] = _T("SRF_Left_Middle.fir");
			filename_iso[LEFT_INDEX] = _T("SRF_Left_Index.fir");
			filename_iso[LEFT_THUMB] = _T("SRF_Left_Thumb.fir");
			filename_iso[RIGHT_THUMB] = _T("SRF_Right_Thumb.fir");
			filename_iso[RIGHT_INDEX] = _T("SRF_Right_Index.fir");
			filename_iso[RIGHT_MIDDLE] = _T("SRF_Right_Middle.fir");
			filename_iso[RIGHT_RING] = _T("SRF_Right_Ring.fir");
			filename_iso[RIGHT_LITTLE] = _T("SRF_Right_Little.fir");

			BOOL bkColor = TRUE;		// background is white
			int Fingerarray[16]={LEFT_LITTLE,LEFT_RING,LEFT_MIDDLE,LEFT_INDEX,LEFT_THUMB,
				RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE,
				LEFT_RING_LITTLE,LEFT_INDEX_MIDDLE,
				BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB,
				RIGHT_INDEX_MIDDLE,RIGHT_RING_LITTLE};

			m_ProgressSave.SetRange(0, 15);

			for(i=0; i<16; i++)
			{
				m_ProgressSave.SetPos(i);
				if(m_pParent->m_OptionDlg.m_chkUseBitmap)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
					if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
						m_nCurrWidth, m_nCurrHeight, -m_nCurrWidth, 500, 500 ) != IBSU_STATUS_OK )
					{
						MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
					}
				}
				if(m_pParent->m_OptionDlg.m_chkUseWsq)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
					if( SaveWSQ(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth, m_nCurrHeight) == 0)
					{
						MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
					}
				}
				if(m_pParent->m_OptionDlg.m_chkUseIso)
				{
					full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
					if( SaveISOFIR(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth, m_nCurrHeight) == 0)
					{
						MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
					}
				}
			}
			m_ProgressSave.SetPos(0);
			MessageBox( _T("Done!"),"CBP Demo");
		}
		//19
		else if( m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
		{
			filename_bitmap[LEFT_LITTLE] = _T("SFF_Left_Little.bmp");
			filename_bitmap[LEFT_RING] = _T("SFF_Left_Ring.bmp");
			filename_bitmap[LEFT_MIDDLE] = _T("SFF_Left_Middle.bmp");
			filename_bitmap[LEFT_INDEX] = _T("SFF_Left_Index.bmp");
			filename_bitmap[LEFT_THUMB] = _T("SFF_Left_Thumb.bmp");
			filename_bitmap[RIGHT_THUMB] = _T("SFF_Right_Thumb.bmp");
			filename_bitmap[RIGHT_INDEX] = _T("SFF_Right_Index.bmp");
			filename_bitmap[RIGHT_MIDDLE] = _T("SFF_Right_Middle.bmp");
			filename_bitmap[RIGHT_RING] = _T("SFF_Right_Ring.bmp");
			filename_bitmap[RIGHT_LITTLE] = _T("SFF_Right_Little.bmp");

			filename_bitmap[LEFT_RING_LITTLE] =_T("SFF_Left_Ring_Little.bmp");
			filename_bitmap[LEFT_INDEX_MIDDLE] =_T("SFF_Left_Index_Middle.bmp");
			filename_bitmap[BOTH_LEFT_THUMB] = _T("STF_Both_Left_Thumbs.bmp");
			filename_bitmap[BOTH_RIGHT_THUMB] = _T("STF_Both_Right_Thumbs.bmp");
			filename_bitmap[RIGHT_INDEX_MIDDLE] =_T("SFF_Right_Index_Middle.bmp");
			filename_bitmap[RIGHT_RING_LITTLE] =_T("SFF_Right_Ring_Little.bmp");

			filename_bitmap[LEFT_HAND] =_T("SFF_LEFT_Four.bmp");
			filename_bitmap[BOTH_THUMBS] = _T("STF_Both_Thumbs.bmp");
			filename_bitmap[RIGHT_HAND] =_T("SFF_Right_Four.bmp");

			filename_wsq[LEFT_LITTLE] = _T("SFF_Left_Little.wsq");
			filename_wsq[LEFT_RING] = _T("SFF_Left_Ring.wsq");
			filename_wsq[LEFT_MIDDLE] = _T("SFF_Left_Middle.wsq");
			filename_wsq[LEFT_INDEX] = _T("SFF_Left_Index.wsq");
			filename_wsq[LEFT_THUMB] = _T("SFF_Left_Thumb.wsq");
			filename_wsq[RIGHT_THUMB] = _T("SFF_Right_Thumb.wsq");
			filename_wsq[RIGHT_INDEX] = _T("SFF_Right_Index.wsq");
			filename_wsq[RIGHT_MIDDLE] = _T("SFF_Right_Middle.wsq");
			filename_wsq[RIGHT_RING] = _T("SFF_Right_Ring.wsq");
			filename_wsq[RIGHT_LITTLE] = _T("SFF_Right_Little.wsq");

			filename_wsq[LEFT_RING_LITTLE] =_T("SFF_Left_Ring_Little.wsq");
			filename_wsq[LEFT_INDEX_MIDDLE] =_T("SFF_Left_Index_Middle.wsq");
			filename_wsq[BOTH_LEFT_THUMB] = _T("STF_Both_Left_Thumbs.wsq");
			filename_wsq[BOTH_RIGHT_THUMB] = _T("STF_Both_Right_Thumbs.wsq");
			filename_wsq[RIGHT_INDEX_MIDDLE] =_T("SFF_Right_Index_Middle.wsq");
			filename_wsq[RIGHT_RING_LITTLE] =_T("SFF_Right_Ring_Little.wsq");

			filename_wsq[LEFT_HAND] =_T("SFF_Left_Four.wsq");
			filename_wsq[BOTH_THUMBS] = _T("STF_Both_Thumbs.wsq");
			filename_wsq[RIGHT_HAND] =_T("SFF_Right_Four.wsq");

			filename_iso[LEFT_LITTLE] = _T("SFF_Left_Little.fir");
			filename_iso[LEFT_RING] = _T("SFF_Left_Ring.fir");
			filename_iso[LEFT_MIDDLE] = _T("SFF_Left_Middle.fir");
			filename_iso[LEFT_INDEX] = _T("SFF_Left_Index.fir");
			filename_iso[LEFT_THUMB] = _T("SFF_Left_Thumb.fir");
			filename_iso[RIGHT_THUMB] = _T("SFF_Right_Thumb.fir");
			filename_iso[RIGHT_INDEX] = _T("SFF_Right_Index.fir");
			filename_iso[RIGHT_MIDDLE] = _T("SFF_Right_Middle.fir");
			filename_iso[RIGHT_RING] = _T("SFF_Right_Ring.fir");
			filename_iso[RIGHT_LITTLE] = _T("SFF_Right_Little.fir");

			filename_iso[LEFT_RING_LITTLE] =_T("SFF_Left_Ring_Little.fir");
			filename_iso[LEFT_INDEX_MIDDLE] =_T("SFF_Left_Index_Middle.fir");
			filename_iso[BOTH_LEFT_THUMB] = _T("STF_Both_Left_Thumbs.fir");
			filename_iso[BOTH_RIGHT_THUMB] = _T("STF_Both_Right_Thumbs.fir");
			filename_iso[RIGHT_INDEX_MIDDLE] =_T("SFF_Right_Index_Middle.fir");
			filename_iso[RIGHT_RING_LITTLE] =_T("SFF_Right_Ring_Little.fir");

			filename_iso[LEFT_HAND] =_T("SFF_Left_Four.fir");
			filename_iso[BOTH_THUMBS] = _T("STF_Both_Thumbs.fir");
			filename_iso[RIGHT_HAND] =_T("SFF_Right_Four.fir");

			BOOL bkColor = TRUE;		// background is white

			int Fingerarray[19]={LEFT_LITTLE,LEFT_RING,LEFT_MIDDLE,LEFT_INDEX,LEFT_THUMB,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE,
				LEFT_RING_LITTLE,LEFT_INDEX_MIDDLE,BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB,RIGHT_INDEX_MIDDLE,RIGHT_RING_LITTLE,
				LEFT_HAND,BOTH_THUMBS,RIGHT_HAND};

			m_ProgressSave.SetRange(0,18);

			for(i=0; i<=18; i++)
			{
				m_ProgressSave.SetPos(i);

				if(i ==16 || i==18)
				{
					if(m_pParent->m_OptionDlg.m_chkUseBitmap)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
						if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
							m_nCurrWidth_3, m_nCurrHeight_3, -m_nCurrWidth_3, 500, 500 ) != IBSU_STATUS_OK )
						{
							MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseWsq)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
						if( SaveWSQ(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3, m_nCurrHeight_3) == 0)
						{
							MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseIso)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
						if( SaveISOFIR(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth_3, m_nCurrHeight_3) == 0)
						{
							MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
						}
					}
				}
				else if(i<16 || i==17)
				{
					if(m_pParent->m_OptionDlg.m_chkUseBitmap)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_bitmap[Fingerarray[i]]);
						if( IBSU_SaveBitmapImage( full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], 
							m_nCurrWidth, m_nCurrHeight, -m_nCurrWidth, 500, 500 ) != IBSU_STATUS_OK )
						{
							MessageBox( _T( "Failed to save bitmap image!" ),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseWsq)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_wsq[Fingerarray[i]]);
						if( SaveWSQ(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth, m_nCurrHeight) == 0)
						{
							MessageBox( _T("Failed to save wsq image!"),"CBP Demo");
						}
					}
					if(m_pParent->m_OptionDlg.m_chkUseIso)
					{
						full_filename.Format("%s\\%s", m_ImgSaveFolder, filename_iso[Fingerarray[i]]);
						if( SaveISOFIR(full_filename, (BYTE*)m_ImgBuf[Fingerarray[i]], m_nCurrWidth, m_nCurrHeight) == 0)
						{
							MessageBox( _T("Failed to save ISO 19794-4 image!"),"CBP Demo");
						}
					}
				}
			}
			m_ProgressSave.SetPos(0);
			MessageBox( _T("Done!"),"CBP Demo");
		}

		EndWaitCursor();
	}
	pMalloc->Release();
}

void CMainDlg::OnBnClickedUseAutoSplit()
{
	UpdateData(TRUE);
}

int CMainDlg::SaveWSQ(CString filename, unsigned char *buffer, int width, int height, int pitch)
{
	pitch = -width;
	if( IBSU_WSQEncodeToFile(filename, buffer, width, height, pitch,
		8, 500, 0.75, "") < IBSU_STATUS_OK)
		return 0;

	return 1;
}

int CMainDlg::SaveISOFIR(CString filename, unsigned char *buffer, int width, int height)
{
	int handle =-1;
	//ISO_FIR iso_fir;
	IBSM_ImageData tmpImage;

	tmpImage.ImageFormat = IBSM_IMG_FORMAT_NO_BIT_PACKING;
	tmpImage.ImpressionType = IBSM_IMPRESSION_TYPE_UNKNOWN;
	tmpImage.FingerPosition = IBSM_FINGER_POSITION_UNKNOWN;
	tmpImage.CaptureDeviceTechID = IBSM_CAPTURE_DEVICE_UNKNOWN_OR_UNSPECIFIED;
	tmpImage.CaptureDeviceVendorID = IBSM_CAPTURE_DEVICE_VENDOR_ID_UNREPORTED;
	tmpImage.CaptureDeviceTypeID = IBSM_CAPTURE_DEVICE_TYPE_ID_UNKNOWN;
	tmpImage.ScanSamplingX = 500;
	tmpImage.ScanSamplingY = 500;
	tmpImage.ImageSamplingX = 500;
	tmpImage.ImageSamplingY = 500;
	tmpImage.ImageSizeX = (unsigned short)width;
	tmpImage.ImageSizeY = (unsigned short)height;
	tmpImage.ScaleUnit = 0x01;
	tmpImage.BitDepth = 8;
	tmpImage.ImageData = buffer;
	tmpImage.ImageDataLength = tmpImage.ImageSizeX*tmpImage.ImageSizeY;

	return 1;
}

void CMainDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	WINDOWPLACEMENT place;
	CWnd *pWnd;
	CRect rect;
	int i,j;

	if( m_CaptureMode == TWO_FINGER_FLAT_CAPTURE)
	{
		m_ZoomDlg->Zoom_1_Img();
		pWnd = GetDlgItem(IDC_VIEW_0);
		pWnd->GetWindowPlacement(&place);
		rect = place.rcNormalPosition;

		if(rect.PtInRect(point) == TRUE)
		{
			m_ZoomDlg->m_InImgWidth  = m_nCurrWidth ;
			m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
			m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
			m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
			m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
			m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
			memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[BOTH_THUMBS], m_ZoomDlg->m_InImgSize);
			memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[BOTH_THUMBS], m_ZoomDlg->m_InImgSize);
			m_ZoomDlg->m_ZoomRatio = 2;
			m_ZoomDlg->m_ZoomStartX = 0;
			m_ZoomDlg->m_ZoomStartY = 0;
			m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
			m_ZoomDlg->ShowWindow(SW_SHOW);
			m_ZoomDlg->CenterWindow();
			m_ZoomDlg->SetFocus();
			m_ZoomDlg->m_ZButton1.SetColor(RGB(255,255,255),RGB(0,128,0));
		}
	}

	/////////////////////////////KOJAK//////////////////////////////////////////////////
	if( m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE )
	{
		m_ZoomDlg->Zoom_13_Img();

		int finarr2[3]={LEFT_HAND,BOTH_THUMBS,RIGHT_HAND};

		for(i=0; i<3; i++)
		{
			pWnd = GetDlgItem(IDC_VIEW_FOUR_1+i);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;

			if(rect.PtInRect(point) == TRUE)
			{
				if(i==1)
				{
					m_ZoomDlg->m_InImgWidth  = MAX_IMG_W ;
					m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton2.SetColor(RGB(255,255,255),RGB(0,128,0));

				}else if(i==0)
				{
					m_ZoomDlg->m_InImgWidth  = MAX_IMG_W ;
					m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton1.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==2)
				{
					m_ZoomDlg->m_InImgWidth  = MAX_IMG_W ;
					m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton3.SetColor(RGB(255,255,255),RGB(0,128,0));
				}

				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[finarr2[i]], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[finarr2[i]], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}
		}

		int finarr[10]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,
			RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE};
		for(j=0; j<10; j++)
		{
			pWnd = GetDlgItem(IDC_VIEW_4+j);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				if(j==0)
				{
					m_ZoomDlg->m_ZButton4.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==1)
				{
					m_ZoomDlg->m_ZButton5.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==2)
				{
					m_ZoomDlg->m_ZButton6.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==3)
				{
					m_ZoomDlg->m_ZButton7.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==4)
				{
					m_ZoomDlg->m_ZButton8.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==5)
				{
					m_ZoomDlg->m_ZButton9.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==6)
				{
					m_ZoomDlg->m_ZButton10.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==7)
				{
					m_ZoomDlg->m_ZButton11.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==8)
				{
					m_ZoomDlg->m_ZButton12.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==9)
				{
					m_ZoomDlg->m_ZButton13.SetColor(RGB(255,255,255),RGB(0,128,0));
				}
				m_ZoomDlg->m_InImgWidth = m_nCurrentWidthForSplit[finarr[j]];
				m_ZoomDlg->m_InImgHeight = m_nCurrentHeightForSplit[finarr[j]];
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;

				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[finarr[j]], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[finarr[j]], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}
		}
	}

	/////////////////////////////kojak 2nd///////////////////////////////////////////////
	if( m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
		m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		m_ZoomDlg->Zoom_13_Img();

		int finarr2[3]={LEFT_HAND,BOTH_THUMBS,RIGHT_HAND};


		for(i=0; i<3; i++)
		{
			pWnd = GetDlgItem(IDC_VIEW_FOUR_1+i);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;

			if(rect.PtInRect(point) == TRUE)
			{

				if(i==1)
				{
					m_ZoomDlg->m_InImgWidth  = MAX_IMG_W ;
					m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton2.SetColor(RGB(255,255,255),RGB(0,128,0));

				}else if(i==0)
				{
					m_ZoomDlg->m_InImgWidth  = MAX_IMG_W ;
					m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton1.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==2)
				{
					m_ZoomDlg->m_InImgWidth  = MAX_IMG_W ;
					m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton3.SetColor(RGB(255,255,255),RGB(0,128,0));
				}

				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[finarr2[i]], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[finarr2[i]], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}

		}

		int finarr[10]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,
			RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE};
		for(j=0; j<10; j++)
		{
			pWnd = GetDlgItem(IDC_VIEW_4+j);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				if(j==0)
				{
					m_ZoomDlg->m_ZButton4.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==1)
				{
					m_ZoomDlg->m_ZButton5.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==2)
				{
					m_ZoomDlg->m_ZButton6.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==3)
				{
					m_ZoomDlg->m_ZButton7.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==4)
				{
					m_ZoomDlg->m_ZButton8.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==5)
				{
					m_ZoomDlg->m_ZButton9.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==6)
				{
					m_ZoomDlg->m_ZButton10.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==7)
				{
					m_ZoomDlg->m_ZButton11.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==8)
				{
					m_ZoomDlg->m_ZButton12.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(j==9)
				{
					m_ZoomDlg->m_ZButton13.SetColor(RGB(255,255,255),RGB(0,128,0));
				}
				//m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				//m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgWidth = IMG_W;
				m_ZoomDlg->m_InImgHeight = IMG_H;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;

				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[finarr[j]], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[finarr[j]], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////

	if( m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH)
	{
		m_ZoomDlg->Zoom_3_Img();

		int finarr[3]={LEFT_HAND,BOTH_THUMBS,RIGHT_HAND};


		for(i=0; i<3; i++)
		{
			pWnd = GetDlgItem(IDC_VIEW_1+i);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;

			if(rect.PtInRect(point) == TRUE)
			{

				if(i==1)
				{
					m_ZoomDlg->m_InImgWidth  = MAX_IMG_W ;
					m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton2.SetColor(RGB(255,255,255),RGB(0,128,0));

				}else if(i==0)
				{
					m_ZoomDlg->m_InImgWidth  = MAX_IMG_W ;
					m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton1.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==2)
				{
					m_ZoomDlg->m_InImgWidth  = MAX_IMG_W ;
					m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton3.SetColor(RGB(255,255,255),RGB(0,128,0));
				}

				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[finarr[i]], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[finarr[i]], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}

		}

	}

	if( m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH)
	{
		m_ZoomDlg->Zoom_3_Img();
		int finarr[3]={LEFT_HAND,THUMB_SUM,RIGHT_HAND};
		for(i=0; i<3; i++)
		{
			pWnd = GetDlgItem(IDC_VIEW_1+i);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				if(i==0)
				{
					m_ZoomDlg->m_ZButton1.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==1)
				{
					m_ZoomDlg->m_ZButton2.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==2)
				{
					m_ZoomDlg->m_ZButton3.SetColor(RGB(255,255,255),RGB(0,128,0));
				}
				m_ZoomDlg->m_InImgWidth  = MAX_IMG_W ;
				m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[finarr[i]], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[finarr[i]], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}


		}

	}

	if( m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE || m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
	{

		m_ZoomDlg->Zoom_10_Img();

		int finarr[10]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,
			RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE};
		for(i=0; i<10; i++)
		{
			pWnd = GetDlgItem(IDC_VIEW_4+i);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				if(i==0)
				{
					m_ZoomDlg->m_ZButton1.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==1)
				{
					m_ZoomDlg->m_ZButton2.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==2)
				{
					m_ZoomDlg->m_ZButton3.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==3)
				{
					m_ZoomDlg->m_ZButton4.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==4)
				{
					m_ZoomDlg->m_ZButton5.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==5)
				{
					m_ZoomDlg->m_ZButton6.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==6)
				{
					m_ZoomDlg->m_ZButton7.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==7)
				{
					m_ZoomDlg->m_ZButton8.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==8)
				{
					m_ZoomDlg->m_ZButton9.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==9)
				{
					m_ZoomDlg->m_ZButton10.SetColor(RGB(255,255,255),RGB(0,128,0));
				}
				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;

				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[finarr[i]], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[finarr[i]], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}

		}

	}
	if( m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE || m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH ||
		m_CaptureMode == TEN_FINGER_FLAT_THUMB || m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
	{

		m_ZoomDlg->Zoom_14_Img();

		int finarr[14]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE,
			LEFT_HAND,BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB,RIGHT_HAND};
		for(i=0; i<14; i++)
		{
			pWnd = GetDlgItem(IDC_VIEW_4+i);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				if(i==11)
				{
					m_ZoomDlg->m_InImgWidth = m_nCurrentWidthForSplit[BOTH_LEFT_THUMB];
					m_ZoomDlg->m_InImgHeight = m_nCurrentHeightForSplit[BOTH_LEFT_THUMB];
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton12.SetColor(RGB(255,255,255),RGB(0,128,0));

				}else if(i==12)
				{
					m_ZoomDlg->m_InImgWidth = m_nCurrentWidthForSplit[BOTH_RIGHT_THUMB];
					m_ZoomDlg->m_InImgHeight = m_nCurrentHeightForSplit[BOTH_RIGHT_THUMB];
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton13.SetColor(RGB(255,255,255),RGB(0,128,0));


				}else if(i==10 )
				{
					m_ZoomDlg->m_InImgWidth = MAX_IMG_W;
					m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton11.SetColor(RGB(255,255,255),RGB(0,128,0));

				}else if(i==13)
				{
					m_ZoomDlg->m_InImgWidth = MAX_IMG_W;
					m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton14.SetColor(RGB(255,255,255),RGB(0,128,0));

				}
				else
				{
					if(i==0)
					{
						m_ZoomDlg->m_ZButton1.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==1)
					{
						m_ZoomDlg->m_ZButton2.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==2)
					{
						m_ZoomDlg->m_ZButton3.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==3)
					{
						m_ZoomDlg->m_ZButton4.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==4)
					{
						m_ZoomDlg->m_ZButton5.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==5)
					{
						m_ZoomDlg->m_ZButton6.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==6)
					{
						m_ZoomDlg->m_ZButton7.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==7)
					{
						m_ZoomDlg->m_ZButton8.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==8)
					{
						m_ZoomDlg->m_ZButton9.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==9)
					{
						m_ZoomDlg->m_ZButton10.SetColor(RGB(255,255,255),RGB(0,128,0));
					}

					m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
					m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				}


				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[finarr[i]], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[finarr[i]], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}
		}

	}
	if( m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH)
	{
		m_ZoomDlg->Zoom_14_Img();

		int finarr[14]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE,
			LEFT_HAND,BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB,RIGHT_HAND};
		for(i=0; i<14; i++)
		{
			pWnd = GetDlgItem(IDC_VIEW_4+i);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				if(i==10)
				{
					m_ZoomDlg->m_InImgWidth = MAX_IMG_W;
					m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton11.SetColor(RGB(255,255,255),RGB(0,128,0));

				}else if(i==13)
				{
					m_ZoomDlg->m_InImgWidth = MAX_IMG_W;
					m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
					m_ZoomDlg->m_ZButton14.SetColor(RGB(255,255,255),RGB(0,128,0));

				}
				else
				{
					if(i==0)
					{
						m_ZoomDlg->m_ZButton1.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==1)
					{
						m_ZoomDlg->m_ZButton2.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==2)
					{
						m_ZoomDlg->m_ZButton3.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==3)
					{
						m_ZoomDlg->m_ZButton4.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==4)
					{
						m_ZoomDlg->m_ZButton5.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==5)
					{
						m_ZoomDlg->m_ZButton6.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==6)
					{
						m_ZoomDlg->m_ZButton7.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==7)
					{
						m_ZoomDlg->m_ZButton8.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==8)
					{
						m_ZoomDlg->m_ZButton9.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==9)
					{
						m_ZoomDlg->m_ZButton10.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==11)
					{
						m_ZoomDlg->m_ZButton12.SetColor(RGB(255,255,255),RGB(0,128,0));
					}else if(i==12)
					{
						m_ZoomDlg->m_ZButton13.SetColor(RGB(255,255,255),RGB(0,128,0));
					}
					m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
					m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
					m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				}


				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[finarr[i]], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[finarr[i]], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}
		}

	}
	if( m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN)
	{
		m_ZoomDlg->Zoom_16_Img();

		int finarr[10]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE};
		for(i=0; i<10; i++)
		{
			pWnd = GetDlgItem(IDC_VIEW_4+i);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				if(i==0)
				{
					m_ZoomDlg->m_ZButton1.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==1)
				{
					m_ZoomDlg->m_ZButton2.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==2)
				{
					m_ZoomDlg->m_ZButton3.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==3)
				{
					m_ZoomDlg->m_ZButton4.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==4)
				{
					m_ZoomDlg->m_ZButton5.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==5)
				{
					m_ZoomDlg->m_ZButton6.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==6)
				{
					m_ZoomDlg->m_ZButton7.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==7)
				{
					m_ZoomDlg->m_ZButton8.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==8)
				{
					m_ZoomDlg->m_ZButton9.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==9)
				{
					m_ZoomDlg->m_ZButton10.SetColor(RGB(255,255,255),RGB(0,128,0));
				}

				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[finarr[i]], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[finarr[i]], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}
		}

		{
			//int finarr[16]={LEFT_RING_LITTLE,LEFT_INDEX_MIDDLE,BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB,RIGHT_INDEX_MIDDLE,RIGHT_RING_LITTLE}
			pWnd = GetDlgItem(IDC_VIEW_161);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				m_ZoomDlg->m_ZButton11.SetColor(RGB(255,255,255),RGB(0,128,0));

				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[LEFT_RING_LITTLE], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[LEFT_RING_LITTLE], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}

		}
		//
		{
			//int finarr[16]={LEFT_RING_LITTLE,LEFT_INDEX_MIDDLE,BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB,RIGHT_INDEX_MIDDLE,RIGHT_RING_LITTLE}
			pWnd = GetDlgItem(IDC_VIEW_162);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				m_ZoomDlg->m_ZButton12.SetColor(RGB(255,255,255),RGB(0,128,0));

				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[LEFT_INDEX_MIDDLE], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[LEFT_INDEX_MIDDLE], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}

		}
		//
		{
			//int finarr[16]={LEFT_RING_LITTLE,LEFT_INDEX_MIDDLE,BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB,RIGHT_INDEX_MIDDLE,RIGHT_RING_LITTLE}
			pWnd = GetDlgItem(IDC_VIEW_163);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				m_ZoomDlg->m_ZButton15.SetColor(RGB(255,255,255),RGB(0,128,0));
				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[RIGHT_INDEX_MIDDLE], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[RIGHT_INDEX_MIDDLE], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}

		}
		//
		{
			//int finarr[16]={LEFT_RING_LITTLE,LEFT_INDEX_MIDDLE,BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB,RIGHT_INDEX_MIDDLE,RIGHT_RING_LITTLE}
			pWnd = GetDlgItem(IDC_VIEW_164);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{

				m_ZoomDlg->m_ZButton16.SetColor(RGB(255,255,255),RGB(0,128,0));
				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[RIGHT_RING_LITTLE], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[RIGHT_RING_LITTLE], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}

		}
		{
			//int finarr[16]={LEFT_RING_LITTLE,LEFT_INDEX_MIDDLE,BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB,RIGHT_INDEX_MIDDLE,RIGHT_RING_LITTLE}
			pWnd = GetDlgItem(IDC_VIEW_96);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{

				m_ZoomDlg->m_ZButton13.SetColor(RGB(255,255,255),RGB(0,128,0));
				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[BOTH_LEFT_THUMB], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[BOTH_LEFT_THUMB], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}

		}
		//
		{	
			pWnd = GetDlgItem(IDC_VIEW_97);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				m_ZoomDlg->m_ZButton14.SetColor(RGB(255,255,255),RGB(0,128,0));
				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[BOTH_RIGHT_THUMB], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[BOTH_RIGHT_THUMB], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}
		}

	}
	if( m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		m_ZoomDlg->Zoom_19_Img();

		int finarr[10]={LEFT_THUMB,LEFT_INDEX,LEFT_MIDDLE,LEFT_RING,LEFT_LITTLE,RIGHT_THUMB,RIGHT_INDEX,RIGHT_MIDDLE,RIGHT_RING,RIGHT_LITTLE};
		for(i=0; i<10; i++)
		{
			pWnd = GetDlgItem(IDC_VIEW_4+i);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				if(i==0)
				{
					m_ZoomDlg->m_ZButton1.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==1)
				{
					m_ZoomDlg->m_ZButton2.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==2)
				{
					m_ZoomDlg->m_ZButton3.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==3)
				{
					m_ZoomDlg->m_ZButton4.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==4)
				{
					m_ZoomDlg->m_ZButton5.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==5)
				{
					m_ZoomDlg->m_ZButton6.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==6)
				{
					m_ZoomDlg->m_ZButton7.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==7)
				{
					m_ZoomDlg->m_ZButton8.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==8)
				{
					m_ZoomDlg->m_ZButton9.SetColor(RGB(255,255,255),RGB(0,128,0));
				}else if(i==9)
				{
					m_ZoomDlg->m_ZButton10.SetColor(RGB(255,255,255),RGB(0,128,0));
				}

				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[finarr[i]], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[finarr[i]], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}
		}

		{
			//int finarr[16]={LEFT_RING_LITTLE,LEFT_INDEX_MIDDLE,BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB,RIGHT_INDEX_MIDDLE,RIGHT_RING_LITTLE}
			pWnd = GetDlgItem(IDC_VIEW_161);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				m_ZoomDlg->m_ZButton11.SetColor(RGB(255,255,255),RGB(0,128,0));

				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[LEFT_RING_LITTLE], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[LEFT_RING_LITTLE], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}

		}
		//
		{
			//int finarr[16]={LEFT_RING_LITTLE,LEFT_INDEX_MIDDLE,BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB,RIGHT_INDEX_MIDDLE,RIGHT_RING_LITTLE}
			pWnd = GetDlgItem(IDC_VIEW_162);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				m_ZoomDlg->m_ZButton12.SetColor(RGB(255,255,255),RGB(0,128,0));
				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[LEFT_INDEX_MIDDLE], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[LEFT_INDEX_MIDDLE], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}

		}
		{
			pWnd = GetDlgItem(IDC_VIEW_96);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				m_ZoomDlg->m_ZButton13.SetColor(RGB(255,255,255),RGB(0,128,0));
				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[BOTH_LEFT_THUMB], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[BOTH_LEFT_THUMB], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}

		}

		{	
			pWnd = GetDlgItem(IDC_VIEW_97);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				m_ZoomDlg->m_ZButton14.SetColor(RGB(255,255,255),RGB(0,128,0));

				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[BOTH_RIGHT_THUMB], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[BOTH_RIGHT_THUMB], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}
		}
		//
		{
			//int finarr[16]={LEFT_RING_LITTLE,LEFT_INDEX_MIDDLE,BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB,RIGHT_INDEX_MIDDLE,RIGHT_RING_LITTLE}
			pWnd = GetDlgItem(IDC_VIEW_163);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				m_ZoomDlg->m_ZButton15.SetColor(RGB(255,255,255),RGB(0,128,0));

				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[RIGHT_INDEX_MIDDLE], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[RIGHT_INDEX_MIDDLE], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}

		}
		//
		{
			//int finarr[16]={LEFT_RING_LITTLE,LEFT_INDEX_MIDDLE,BOTH_LEFT_THUMB,BOTH_RIGHT_THUMB,RIGHT_INDEX_MIDDLE,RIGHT_RING_LITTLE}
			pWnd = GetDlgItem(IDC_VIEW_164);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{

				m_ZoomDlg->m_ZButton16.SetColor(RGB(255,255,255),RGB(0,128,0));
				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[RIGHT_RING_LITTLE], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[RIGHT_RING_LITTLE], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}

		}
		{	
			pWnd = GetDlgItem(IDC_VIEW_99);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				m_ZoomDlg->m_ZButton17.SetColor(RGB(255,255,255),RGB(0,128,0));
				m_ZoomDlg->m_InImgWidth = MAX_IMG_W;
				m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[LEFT_HAND], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[LEFT_HAND], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}
		}
		{	
			pWnd = GetDlgItem(IDC_VIEW_990);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				m_ZoomDlg->m_ZButton18.SetColor(RGB(255,255,255),RGB(0,128,0));
				m_ZoomDlg->m_InImgWidth = m_nCurrWidth;
				m_ZoomDlg->m_InImgHeight = m_nCurrHeight;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[BOTH_THUMBS], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[BOTH_THUMBS], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}
		}
		{	
			pWnd = GetDlgItem(IDC_VIEW_991);
			pWnd->GetWindowPlacement(&place);
			rect = place.rcNormalPosition;
			if(rect.PtInRect(point) == TRUE)
			{
				m_ZoomDlg->m_ZButton19.SetColor(RGB(255,255,255),RGB(0,128,0));
				m_ZoomDlg->m_InImgWidth = MAX_IMG_W;
				m_ZoomDlg->m_InImgHeight = MAX_IMG_H;
				m_ZoomDlg->m_InImgSize = m_ZoomDlg->m_InImgWidth*m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgWidth;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgHeight;
				m_ZoomDlg->m_OutImgWidth = m_ZoomDlg->m_InImgSize;
				memcpy(m_ZoomDlg->m_InImg, m_ImgBuf[RIGHT_HAND], m_ZoomDlg->m_InImgSize);
				memcpy(m_ZoomDlg->m_OutImg, m_ImgBuf[RIGHT_HAND], m_ZoomDlg->m_InImgSize);
				m_ZoomDlg->m_ZoomRatio = 2;
				m_ZoomDlg->m_ZoomStartX = 0;
				m_ZoomDlg->m_ZoomStartY = 0;
				m_ZoomDlg->ChangeZoomWindow(m_ZoomDlg->m_ZoomRatio);
				m_ZoomDlg->ShowWindow(SW_SHOW);
				m_ZoomDlg->CenterWindow();
				m_ZoomDlg->SetFocus();
			}
		}



	}




	CDialog::OnRButtonDown(nFlags, point);
}

HBRUSH CMainDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  여기서 DC의 특성을 변경합니다.
	//	hbr =(HBRUSH)m_background;

	// TODO:  기본값이 적당하지 않으면 다른 브러시를 반환합니다.
	return hbr;
}

void CMainDlg::	_SingleDevice(UINT m_CaptureMode, int imageStatus, int nfiq_score[IBSU_MAX_SEGMENT_COUNT], int nfiq2_score[IBSU_MAX_SEGMENT_COUNT], int spoof_score[IBSU_MAX_SEGMENT_COUNT], 
							  IBSU_ImageData image, const IBSU_ImageData *pSegmentImageArray, const IBSU_SegmentPosition *pSegmentPositionArray)
{
	if(m_CaptureMode == TEN_FINGER_ONE_ROLL_CAPTURE)
	{
		memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
		IBSU_GenerateZoomOutImage(pSegmentImageArray[0],  m_ImgBuf_S[m_CaptureFinger], S_IMG_W, S_IMG_H, 255);
		m_NFIQBuf[m_CaptureFinger] = nfiq_score[0];
		m_NFIQ2Buf[m_CaptureFinger] = nfiq2_score[0];
		m_SpoofBuf[m_CaptureFinger] = spoof_score[0];
	}
	else if(m_CaptureMode == TEN_FINGER_ONE_FLAT_CAPTURE)
	{
		memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
		IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_W, S_IMG_H, 255);
		m_NFIQBuf[m_CaptureFinger] = nfiq_score[0];
		m_NFIQ2Buf[m_CaptureFinger] = nfiq2_score[0];
		m_SpoofBuf[m_CaptureFinger] = spoof_score[0];
	}
	else if(m_CaptureMode == TEN_FINGER_FLAT_THUMB||m_CaptureMode == TEN_FINGER_FLAT_THUMB_INDEX)
	{
		if(m_CaptureFinger)
		{
			memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer, image.Width*image.Height);
			IBSU_GenerateZoomOutImage(image, m_ImgBuf_S[m_CaptureFinger], S_IMG_W, S_IMG_H, 255);
			m_NFIQBuf[m_CaptureFinger] = nfiq_score[0];
			m_NFIQ2Buf[m_CaptureFinger] = nfiq2_score[0];
			m_SpoofBuf[m_CaptureFinger] = spoof_score[0];
		}
	}
}

void CMainDlg::_FourFingerDevice(UINT m_CaptureMode, IBSU_ImageType imageType, int segmentImageArrayCount, 
								 int nfiq_score[IBSU_MAX_SEGMENT_COUNT], int nfiq2_score[IBSU_MAX_SEGMENT_COUNT], int spoof_score[IBSU_MAX_SEGMENT_COUNT], 
								 IBSU_ImageData image, const IBSU_ImageData *pSegmentImageArray, const IBSU_SegmentPosition *pSegmentPositionArray)
{
	if( m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO ||
		m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE )
	{
		if(m_CaptureFinger == LEFT_THUMB)
		{
			if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				BYTE *tmpBuffer0 = (BYTE*)pSegmentImageArray[0].Buffer;
				int width, height;
				int i, j, ii, jj;

				width = pSegmentImageArray[0].Width;
				height = pSegmentImageArray[0].Height;
				for(i=0; i<height; i++)
				{
					for(j=0; j<width; j++)
					{
						m_ImgBuf[BOTH_THUMBS][(i-height/2+MAX_IMG_H/2)*MAX_IMG_W+(j-width/2+MAX_IMG_W/4)] = tmpBuffer0[i*width+j];
					}
				}

				for(i=0; i<S_IMG_3_H; i++)
				{
					ii = i * MAX_IMG_H / S_IMG_3_H;

					for(j=0; j<S_IMG_3_W; j++)
					{
						jj = j * MAX_IMG_W / S_IMG_3_W;

						m_ImgBuf_S[BOTH_THUMBS][i*S_IMG_3_W+j] = m_ImgBuf[BOTH_THUMBS][ii*MAX_IMG_W+jj];
					}
				}

				memcpy(m_ImgBuf[LEFT_THUMB], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_THUMB] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_THUMB]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_THUMB], S_IMG_W, S_IMG_H, 255);

				m_NFIQTwoBuf1[LEFT_THUMB] = nfiq_score[0];
				m_NFIQ2TwoBuf1[LEFT_THUMB] = nfiq2_score[0];
				m_SpoofTwoBuf1[LEFT_THUMB] = spoof_score[0];
			}
		}
		else if(m_CaptureFinger == RIGHT_THUMB)
		{
			if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
			{
				BYTE *tmpBuffer0 = (BYTE*)pSegmentImageArray[0].Buffer;
				int width, height;
				int i, j, ii, jj;

				width = pSegmentImageArray[0].Width;
				height = pSegmentImageArray[0].Height;
				for(i=0; i<height; i++)
				{
					for(j=0; j<width; j++)
					{
						m_ImgBuf[BOTH_THUMBS][(i-height/2+MAX_IMG_H/2)*MAX_IMG_W+(j-width/2+MAX_IMG_W*3/4)] = tmpBuffer0[i*width+j];
					}
				}

				for(i=0; i<S_IMG_3_H; i++)
				{
					ii = i * MAX_IMG_H / S_IMG_3_H;

					for(j=0; j<S_IMG_3_W; j++)
					{
						jj = j * MAX_IMG_W / S_IMG_3_W;

						m_ImgBuf_S[BOTH_THUMBS][i*S_IMG_3_W+j] = m_ImgBuf[BOTH_THUMBS][ii*MAX_IMG_W+jj];
					}
				}

				memcpy(m_ImgBuf[RIGHT_THUMB], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_THUMB] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_THUMB]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_THUMB], S_IMG_W, S_IMG_H, 255);

				m_NFIQTwoBuf2[RIGHT_THUMB] = nfiq_score[0];
				m_NFIQ2TwoBuf2[RIGHT_THUMB] = nfiq2_score[0];
				m_SpoofTwoBuf2[RIGHT_THUMB] = spoof_score[0];
			}
		}
		else if(m_CaptureFinger == BOTH_THUMBS)
		{
			if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE ))
			{
				memcpy(m_ImgBuf[BOTH_THUMBS], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				memcpy(m_ImgBuf[LEFT_THUMB], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_THUMB] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_THUMB]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_THUMB], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_THUMB], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[RIGHT_THUMB] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[RIGHT_THUMB]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[RIGHT_THUMB], S_IMG_W, S_IMG_H, 255);

				m_NFIQTwoBuf1[LEFT_THUMB] = nfiq_score[0];
				m_NFIQTwoBuf2[RIGHT_THUMB] = nfiq_score[1];

				m_NFIQ2TwoBuf1[LEFT_THUMB] = nfiq2_score[0];
				m_NFIQ2TwoBuf2[RIGHT_THUMB] = nfiq2_score[1];

				m_SpoofTwoBuf1[LEFT_THUMB] = spoof_score[0];
				m_SpoofTwoBuf2[RIGHT_THUMB] = spoof_score[1];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				memcpy(m_ImgBuf[BOTH_THUMBS], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				memcpy(m_ImgBuf[LEFT_THUMB], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_THUMB] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_THUMB]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_THUMB], S_IMG_W, S_IMG_H, 255);

				m_NFIQTwoBuf1[LEFT_THUMB] = nfiq_score[0];
				m_NFIQ2TwoBuf1[LEFT_THUMB] = nfiq2_score[0];
				m_SpoofTwoBuf1[LEFT_THUMB] = spoof_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
			{
				memcpy(m_ImgBuf[BOTH_THUMBS], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				memcpy(m_ImgBuf[RIGHT_THUMB], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_THUMB] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_THUMB]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_THUMB], S_IMG_W, S_IMG_H, 255);

				m_NFIQTwoBuf2[RIGHT_THUMB] = nfiq_score[0];
				m_NFIQ2TwoBuf2[RIGHT_THUMB] = nfiq2_score[0];
				m_SpoofTwoBuf2[RIGHT_THUMB] = spoof_score[0];
			}
		}
		else if(m_CaptureFinger == LEFT_HAND)
		{
			if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf2[LEFT_RING] = nfiq_score[1];
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[2];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[3];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[1];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[2];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[3];

				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				m_SpoofBuf2[LEFT_RING] = spoof_score[1];
				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[2];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[3];

				memcpy(m_ImgBuf[LEFT_LITTLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_LITTLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_LITTLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_LITTLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_RING], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[LEFT_RING] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[LEFT_RING]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[LEFT_RING], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_MIDDLE], pSegmentImageArray[2].Buffer, pSegmentImageArray[2].Width*pSegmentImageArray[2].Height);
				m_nCurrentWidthForSplit[LEFT_MIDDLE] =pSegmentImageArray[2].Width;
				m_nCurrentHeightForSplit[LEFT_MIDDLE]=pSegmentImageArray[2].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[2], m_ImgBuf_S[LEFT_MIDDLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_INDEX], pSegmentImageArray[3].Buffer, pSegmentImageArray[3].Width*pSegmentImageArray[3].Height);
				m_nCurrentWidthForSplit[LEFT_INDEX] =pSegmentImageArray[3].Width;
				m_nCurrentHeightForSplit[LEFT_INDEX]=pSegmentImageArray[3].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[3], m_ImgBuf_S[LEFT_INDEX], S_IMG_W, S_IMG_H, 255);

			}
			//Three Fingers
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[LEFT_RING] = nfiq_score[0];
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[1];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[2];

				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[0];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[1];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[2];

				m_SpoofBuf2[LEFT_RING] = spoof_score[0];
				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[1];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[2];

				memcpy(m_ImgBuf[LEFT_RING], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_RING] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_RING]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_RING], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_MIDDLE], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[LEFT_MIDDLE] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[LEFT_MIDDLE]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[LEFT_MIDDLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_INDEX], pSegmentImageArray[2].Buffer, pSegmentImageArray[2].Width*pSegmentImageArray[2].Height);
				m_nCurrentWidthForSplit[LEFT_INDEX] =pSegmentImageArray[2].Width;
				m_nCurrentHeightForSplit[LEFT_INDEX]=pSegmentImageArray[2].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[2], m_ImgBuf_S[LEFT_INDEX], S_IMG_W, S_IMG_H, 255);

			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[1];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[2];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[1];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[2];

				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[1];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[2];

				memcpy(m_ImgBuf[LEFT_LITTLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_LITTLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_LITTLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_LITTLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_MIDDLE], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[LEFT_MIDDLE] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[LEFT_MIDDLE]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[LEFT_MIDDLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_INDEX], pSegmentImageArray[2].Buffer, pSegmentImageArray[2].Width*pSegmentImageArray[2].Height);
				m_nCurrentWidthForSplit[LEFT_INDEX] =pSegmentImageArray[2].Width;
				m_nCurrentHeightForSplit[LEFT_INDEX]=pSegmentImageArray[2].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[2], m_ImgBuf_S[LEFT_INDEX], S_IMG_W, S_IMG_H, 255);

			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf2[LEFT_RING] = nfiq_score[1];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[2];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[1];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[2];

				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				m_SpoofBuf2[LEFT_RING] = spoof_score[1];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[2];

				memcpy(m_ImgBuf[LEFT_LITTLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_LITTLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_LITTLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_LITTLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_RING], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[LEFT_RING] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[LEFT_RING]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[LEFT_RING], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_INDEX], pSegmentImageArray[2].Buffer, pSegmentImageArray[2].Width*pSegmentImageArray[2].Height);
				m_nCurrentWidthForSplit[LEFT_INDEX] =pSegmentImageArray[2].Width;
				m_nCurrentHeightForSplit[LEFT_INDEX]=pSegmentImageArray[2].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[2], m_ImgBuf_S[LEFT_INDEX], S_IMG_W, S_IMG_H, 255);

			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf2[LEFT_RING] = nfiq_score[1];
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[2];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[1];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[2];

				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				m_SpoofBuf2[LEFT_RING] = spoof_score[1];
				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[2];

				memcpy(m_ImgBuf[LEFT_LITTLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_LITTLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_LITTLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_LITTLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_RING], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[LEFT_RING] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[LEFT_RING]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[LEFT_RING], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_MIDDLE], pSegmentImageArray[2].Buffer, pSegmentImageArray[2].Width*pSegmentImageArray[2].Height);
				m_nCurrentWidthForSplit[LEFT_MIDDLE] =pSegmentImageArray[2].Width;
				m_nCurrentHeightForSplit[LEFT_MIDDLE]=pSegmentImageArray[2].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[2], m_ImgBuf_S[LEFT_MIDDLE], S_IMG_W, S_IMG_H, 255);

			}
			//Two Fingers
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE ))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[0];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[1];

				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[0];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[1];

				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[0];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[1];

				memcpy(m_ImgBuf[LEFT_MIDDLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_MIDDLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_MIDDLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_MIDDLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_INDEX], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[LEFT_INDEX] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[LEFT_INDEX]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[LEFT_INDEX], S_IMG_W, S_IMG_H, 255);

			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE ))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[LEFT_RING] = nfiq_score[0];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[1];

				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[0];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[1];

				m_SpoofBuf2[LEFT_RING] = spoof_score[0];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[1];

				memcpy(m_ImgBuf[LEFT_RING], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_RING] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_RING]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_RING], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_INDEX], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[LEFT_INDEX] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[LEFT_INDEX]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[LEFT_INDEX], S_IMG_W, S_IMG_H, 255);
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[1];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[1];

				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[1];

				memcpy(m_ImgBuf[LEFT_LITTLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_LITTLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_LITTLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_LITTLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_INDEX], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[LEFT_INDEX] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[LEFT_INDEX]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[LEFT_INDEX], S_IMG_W, S_IMG_H, 255);

			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[LEFT_RING] = nfiq_score[0];
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[1];

				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[0];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[1];

				m_SpoofBuf2[LEFT_RING] = spoof_score[0];
				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[1];

				memcpy(m_ImgBuf[LEFT_RING], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_RING] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_RING]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_RING], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_MIDDLE], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[LEFT_MIDDLE] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[LEFT_MIDDLE]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[LEFT_MIDDLE], S_IMG_W, S_IMG_H, 255);

			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[1];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[1];

				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[1];

				memcpy(m_ImgBuf[LEFT_LITTLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_LITTLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_LITTLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_LITTLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_MIDDLE], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[LEFT_MIDDLE] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[LEFT_MIDDLE]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[LEFT_MIDDLE], S_IMG_W, S_IMG_H, 255);

			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf2[LEFT_RING] = nfiq_score[1];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[1];

				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				m_SpoofBuf2[LEFT_RING] = spoof_score[1];

				memcpy(m_ImgBuf[LEFT_LITTLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_LITTLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_LITTLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_LITTLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[LEFT_RING], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[LEFT_RING] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[LEFT_RING]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[LEFT_RING], S_IMG_W, S_IMG_H, 255);

			}
			//Single Finger
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[0];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[0];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[0];

				memcpy(m_ImgBuf[LEFT_INDEX], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_INDEX] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_INDEX]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_INDEX], S_IMG_W, S_IMG_H, 255);

			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[0];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[0];
				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[0];

				memcpy(m_ImgBuf[LEFT_MIDDLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_MIDDLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_MIDDLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_MIDDLE], S_IMG_W, S_IMG_H, 255);

			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[LEFT_RING] = nfiq_score[0];
				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[0];
				m_SpoofBuf2[LEFT_RING] = spoof_score[0];

				memcpy(m_ImgBuf[LEFT_RING], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_RING] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_RING]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_RING], S_IMG_W, S_IMG_H, 255);

			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];

				memcpy(m_ImgBuf[LEFT_LITTLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[LEFT_LITTLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[LEFT_LITTLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_LITTLE], S_IMG_W, S_IMG_H, 255);
			}
		}
		else if(m_CaptureFinger == RIGHT_HAND)
		{
			if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[1];
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[2];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[3];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[1];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[2];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[3];

				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[1];
				m_SpoofBuf3[RIGHT_RING] = spoof_score[2];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[3];

				memcpy(m_ImgBuf[RIGHT_INDEX], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_INDEX] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_INDEX]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_INDEX], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_MIDDLE], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[RIGHT_MIDDLE] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[RIGHT_MIDDLE]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[RIGHT_MIDDLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_RING], pSegmentImageArray[2].Buffer, pSegmentImageArray[2].Width*pSegmentImageArray[2].Height);
				m_nCurrentWidthForSplit[RIGHT_RING] =pSegmentImageArray[2].Width;
				m_nCurrentHeightForSplit[RIGHT_RING]=pSegmentImageArray[2].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[2], m_ImgBuf_S[RIGHT_RING], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_LITTLE], pSegmentImageArray[3].Buffer, pSegmentImageArray[3].Width*pSegmentImageArray[3].Height);
				m_nCurrentWidthForSplit[RIGHT_LITTLE] =pSegmentImageArray[3].Width;
				m_nCurrentHeightForSplit[RIGHT_LITTLE]=pSegmentImageArray[3].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[3], m_ImgBuf_S[RIGHT_LITTLE], S_IMG_W, S_IMG_H, 255);
			}
			//Three Fingers
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);
				
				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[1];
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[2];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[1];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[2];

				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[1];
				m_SpoofBuf3[RIGHT_RING] = spoof_score[2];

				memcpy(m_ImgBuf[RIGHT_INDEX], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_INDEX] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_INDEX]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_INDEX], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_MIDDLE], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[RIGHT_MIDDLE] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[RIGHT_MIDDLE]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[RIGHT_MIDDLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_RING], pSegmentImageArray[2].Buffer, pSegmentImageArray[2].Width*pSegmentImageArray[2].Height);
				m_nCurrentWidthForSplit[RIGHT_RING] =pSegmentImageArray[2].Width;
				m_nCurrentHeightForSplit[RIGHT_RING]=pSegmentImageArray[2].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[2], m_ImgBuf_S[RIGHT_RING], S_IMG_W, S_IMG_H, 255);
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);
				
				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[1];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[2];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[1];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[2];

				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[1];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[2];

				memcpy(m_ImgBuf[RIGHT_INDEX], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_INDEX] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_INDEX]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_INDEX], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_MIDDLE], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[RIGHT_MIDDLE] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[RIGHT_MIDDLE]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[RIGHT_MIDDLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_LITTLE], pSegmentImageArray[2].Buffer, pSegmentImageArray[2].Width*pSegmentImageArray[2].Height);
				m_nCurrentWidthForSplit[RIGHT_LITTLE] =pSegmentImageArray[2].Width;
				m_nCurrentHeightForSplit[RIGHT_LITTLE]=pSegmentImageArray[2].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[2], m_ImgBuf_S[RIGHT_LITTLE], S_IMG_W, S_IMG_H, 255);

			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[0];
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[1];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[2];

				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[0];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[1];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[2];

				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[0];
				m_SpoofBuf3[RIGHT_RING] = spoof_score[1];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[2];

				memcpy(m_ImgBuf[RIGHT_MIDDLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_MIDDLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_MIDDLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_MIDDLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_RING], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[RIGHT_RING] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[RIGHT_RING]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[RIGHT_RING], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_LITTLE], pSegmentImageArray[2].Buffer, pSegmentImageArray[2].Width*pSegmentImageArray[2].Height);
				m_nCurrentWidthForSplit[RIGHT_LITTLE] =pSegmentImageArray[2].Width;
				m_nCurrentHeightForSplit[RIGHT_LITTLE]=pSegmentImageArray[2].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[2], m_ImgBuf_S[RIGHT_LITTLE], S_IMG_W, S_IMG_H, 255);

			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[1];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[2];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[1];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[2];

				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				m_SpoofBuf3[RIGHT_RING] = spoof_score[1];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[2];

				memcpy(m_ImgBuf[RIGHT_INDEX], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_INDEX] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_INDEX]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_INDEX], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_RING], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[RIGHT_RING] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[RIGHT_RING]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[RIGHT_RING], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_LITTLE], pSegmentImageArray[2].Buffer, pSegmentImageArray[2].Width*pSegmentImageArray[2].Height);
				m_nCurrentWidthForSplit[RIGHT_LITTLE] =pSegmentImageArray[2].Width;
				m_nCurrentHeightForSplit[RIGHT_LITTLE]=pSegmentImageArray[2].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[2], m_ImgBuf_S[RIGHT_LITTLE], S_IMG_W, S_IMG_H, 255);

			}
			//Two Fingers
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE ))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[1];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[1];

				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[1];

				memcpy(m_ImgBuf[RIGHT_INDEX], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_INDEX] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_INDEX]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_INDEX], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_MIDDLE], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[RIGHT_MIDDLE] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[RIGHT_MIDDLE]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[RIGHT_MIDDLE], S_IMG_W, S_IMG_H, 255);
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE ))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[1];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[1];

				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				m_SpoofBuf3[RIGHT_RING] = spoof_score[1];

				memcpy(m_ImgBuf[RIGHT_INDEX], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_INDEX] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_INDEX]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_INDEX], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_RING], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[RIGHT_RING] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[RIGHT_RING]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[RIGHT_RING], S_IMG_W, S_IMG_H, 255);
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[1];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[1];

				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[1];

				memcpy(m_ImgBuf[RIGHT_INDEX], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_INDEX] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_INDEX]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_INDEX], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_LITTLE], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[RIGHT_LITTLE] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[RIGHT_LITTLE]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[RIGHT_LITTLE], S_IMG_W, S_IMG_H, 255);
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE ))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[0];
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[1];

				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[0];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[1];

				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[0];
				m_SpoofBuf3[RIGHT_RING] = spoof_score[1];

				memcpy(m_ImgBuf[RIGHT_MIDDLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_MIDDLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_MIDDLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_MIDDLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_RING], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[RIGHT_RING] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[RIGHT_RING]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[RIGHT_RING], S_IMG_W, S_IMG_H, 255);

			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE ))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[0];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[1];

				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[0];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[1];

				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[0];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[1];

				memcpy(m_ImgBuf[RIGHT_MIDDLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_MIDDLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_MIDDLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_MIDDLE], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_LITTLE], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[RIGHT_LITTLE] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[RIGHT_LITTLE]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[RIGHT_LITTLE], S_IMG_W, S_IMG_H, 255);
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE ))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf3[RIGHT_RING] = nfiq_score[0];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[1];

				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[0];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[1];

				m_SpoofBuf3[RIGHT_RING] = spoof_score[0];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[1];

				memcpy(m_ImgBuf[RIGHT_RING], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_RING] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_RING]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_RING], S_IMG_W, S_IMG_H, 255);

				memcpy(m_ImgBuf[RIGHT_LITTLE], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
				m_nCurrentWidthForSplit[RIGHT_LITTLE] =pSegmentImageArray[1].Width;
				m_nCurrentHeightForSplit[RIGHT_LITTLE]=pSegmentImageArray[1].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[RIGHT_LITTLE], S_IMG_W, S_IMG_H, 255);
			}
			//1 finger
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];

				memcpy(m_ImgBuf[RIGHT_INDEX], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_INDEX] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_INDEX]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_INDEX], S_IMG_W, S_IMG_H, 255);
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[0];
				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[0];
				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[0];

				memcpy(m_ImgBuf[RIGHT_MIDDLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_MIDDLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_MIDDLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_MIDDLE], S_IMG_W, S_IMG_H, 255);

			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf3[RIGHT_RING] = nfiq_score[0];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[0];
				m_SpoofBuf3[RIGHT_RING] = spoof_score[0];

				memcpy(m_ImgBuf[RIGHT_RING], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_RING] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_RING]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_RING], S_IMG_W, S_IMG_H, 255);
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[0];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[0];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[0];

				memcpy(m_ImgBuf[RIGHT_LITTLE], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
				m_nCurrentWidthForSplit[RIGHT_LITTLE] =pSegmentImageArray[0].Width;
				m_nCurrentHeightForSplit[RIGHT_LITTLE]=pSegmentImageArray[0].Height;
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_LITTLE], S_IMG_W, S_IMG_H, 255);
			}
		}
	}
	else if( m_CaptureMode == TEN_FINGER_FOUR_FOUR_TWO_ROLL_CAPTURE_EACH ||
			  m_CaptureMode == TEN_FINGER_FOUR_ONE_FOUR_ONE_ROLL_CAPTURE_EACH)
	{
		//4-1-4-1
		if(m_CaptureFinger == LEFT_THUMB)
		{
			if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				if(imageType == ENUM_IBSU_FLAT_SINGLE_FINGER)
				{
					BYTE *tmpBuffer0 = (BYTE*)pSegmentImageArray[0].Buffer;
					int width, height;
					int i, j, ii, jj;

					width = pSegmentImageArray[0].Width;
					height = pSegmentImageArray[0].Height;
					for(i=0; i<height; i++)
					{
						for(j=0; j<width; j++)
						{
							m_ImgBuf[BOTH_THUMBS][(i-height/2+MAX_IMG_H/2)*MAX_IMG_W+(j-width/2+MAX_IMG_W/4)] = tmpBuffer0[i*width+j];
						}
					}

					for(i=0; i<S_IMG_3_H; i++)
					{
						ii = i * MAX_IMG_H / S_IMG_3_H;

						for(j=0; j<S_IMG_3_W; j++)
						{
							jj = j * MAX_IMG_W / S_IMG_3_W;

							m_ImgBuf_S[BOTH_THUMBS][i*S_IMG_3_W+j] = m_ImgBuf[BOTH_THUMBS][ii*MAX_IMG_W+jj];
						}
					}
					m_NFIQTwoBuf1[LEFT_THUMB] = nfiq_score[0];
					m_NFIQ2TwoBuf1[LEFT_THUMB] = nfiq2_score[0];
					m_SpoofTwoBuf1[LEFT_THUMB] = spoof_score[0];
				}
				else
				{
					memcpy(m_ImgBuf[LEFT_THUMB], image.Buffer, image.Width*image.Height);
					IBSU_GenerateZoomOutImage(image, m_ImgBuf_S[LEFT_THUMB], S_IMG_W, S_IMG_H, 255);

					m_NFIQBuf[LEFT_THUMB] = nfiq_score[0];
					m_NFIQ2Buf[LEFT_THUMB] = nfiq2_score[0];
					m_SpoofBuf[LEFT_THUMB] = spoof_score[0];
				}
			}
		}
		else if(m_CaptureFinger == RIGHT_THUMB)
		{
			if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
			{
				if(imageType == ENUM_IBSU_FLAT_SINGLE_FINGER)
				{
					BYTE *tmpBuffer0 = (BYTE*)pSegmentImageArray[0].Buffer;
					int width, height;
					int i, j, ii, jj;

					width = pSegmentImageArray[0].Width;
					height = pSegmentImageArray[0].Height;
					for(i=0; i<height; i++)
					{
						for(j=0; j<width; j++)
						{
							m_ImgBuf[BOTH_THUMBS][(i-height/2+MAX_IMG_H/2)*MAX_IMG_W+(j-width/2+MAX_IMG_W*3/4)] = tmpBuffer0[i*width+j];
						}
					}

					for(i=0; i<S_IMG_3_H; i++)
					{
						ii = i * MAX_IMG_H / S_IMG_3_H;

						for(j=0; j<S_IMG_3_W; j++)
						{
							jj = j * MAX_IMG_W / S_IMG_3_W;

							m_ImgBuf_S[BOTH_THUMBS][i*S_IMG_3_W+j] = m_ImgBuf[BOTH_THUMBS][ii*MAX_IMG_W+jj];
						}
					}
					m_NFIQTwoBuf2[RIGHT_THUMB] = nfiq_score[0];
					m_NFIQ2TwoBuf2[RIGHT_THUMB] = nfiq2_score[0];
					m_SpoofTwoBuf2[RIGHT_THUMB] = spoof_score[0];
				}
				else
				{
					memcpy(m_ImgBuf[RIGHT_THUMB], image.Buffer, image.Width*image.Height);
					IBSU_GenerateZoomOutImage(image, m_ImgBuf_S[RIGHT_THUMB], S_IMG_W, S_IMG_H, 255);

					m_NFIQBuf[RIGHT_THUMB] = nfiq_score[0];
					m_NFIQ2Buf[RIGHT_THUMB] = nfiq2_score[0];
					m_SpoofBuf[RIGHT_THUMB] = spoof_score[0];
				}
			}
		}
		//442
		else if(m_CaptureFinger == BOTH_THUMBS)
		{
			if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE ))
			{
				memcpy(m_ImgBuf[BOTH_THUMBS], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQTwoBuf1[LEFT_THUMB] = nfiq_score[0];
				m_NFIQTwoBuf2[RIGHT_THUMB] = nfiq_score[1];

				m_NFIQ2TwoBuf1[LEFT_THUMB] = nfiq2_score[0];
				m_NFIQ2TwoBuf2[RIGHT_THUMB] = nfiq2_score[1];

				m_SpoofTwoBuf1[LEFT_THUMB] = spoof_score[0];
				m_SpoofTwoBuf2[RIGHT_THUMB] = spoof_score[1];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				memcpy(m_ImgBuf[BOTH_THUMBS], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQTwoBuf1[LEFT_THUMB] = nfiq_score[0];
				m_NFIQ2TwoBuf1[LEFT_THUMB] = nfiq2_score[0];
				m_SpoofTwoBuf1[LEFT_THUMB] = spoof_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
			{
				memcpy(m_ImgBuf[BOTH_THUMBS], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQTwoBuf2[RIGHT_THUMB] = nfiq_score[0];
				m_NFIQ2TwoBuf2[RIGHT_THUMB] = nfiq2_score[0];
				m_SpoofTwoBuf2[RIGHT_THUMB] = spoof_score[0];
			}
		}
		else if(m_CaptureFinger == LEFT_HAND)
		{
			if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf2[LEFT_RING] = nfiq_score[1];
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[2];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[3];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[1];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[2];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[3];

				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				m_SpoofBuf2[LEFT_RING] = spoof_score[1];
				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[2];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[3];
			}
			//Three Fingers
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[LEFT_RING] = nfiq_score[0];
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[1];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[2];

				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[0];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[1];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[2];

				m_SpoofBuf2[LEFT_RING] = spoof_score[0];
				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[1];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[2];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[1];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[2];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[1];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[2];

				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[1];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[2];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf2[LEFT_RING] = nfiq_score[1];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[2];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[1];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[2];

				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				m_SpoofBuf2[LEFT_RING] = spoof_score[1];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[2];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf2[LEFT_RING] = nfiq_score[1];
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[2];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[1];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[2];

				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				m_SpoofBuf2[LEFT_RING] = spoof_score[1];
				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[2];
			}
			//Two Fingers
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE ))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[0];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[1];

				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[0];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[1];

				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[0];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[1];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE ))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[LEFT_RING] = nfiq_score[0];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[1];

				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[0];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[1];

				m_SpoofBuf2[LEFT_RING] = spoof_score[0];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[1];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[1];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[1];

				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[1];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[LEFT_RING] = nfiq_score[0];
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[1];

				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[0];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[1];

				m_SpoofBuf2[LEFT_RING] = spoof_score[0];
				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[1];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[1];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[1];

				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[1];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf2[LEFT_RING] = nfiq_score[1];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[1];

				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				m_SpoofBuf2[LEFT_RING] = spoof_score[1];
			}
			//Single Finger
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[0];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[0];
				m_SpoofBuf4[LEFT_INDEX] = spoof_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[0];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[0];
				m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[LEFT_RING] = nfiq_score[0];
				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[0];
				m_SpoofBuf2[LEFT_RING] = spoof_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
			}
		}
		else if(m_CaptureFinger == RIGHT_HAND)
		{
			if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[1];
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[2];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[3];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[1];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[2];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[3];

				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[1];
				m_SpoofBuf3[RIGHT_RING] = spoof_score[2];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[3];
			}
			//Three Fingers
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);
				
				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[1];
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[2];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[1];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[2];

				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[1];
				m_SpoofBuf3[RIGHT_RING] = spoof_score[2];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE	))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);
				
				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[1];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[2];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[1];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[2];

				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[1];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[2];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[0];
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[1];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[2];

				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[0];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[1];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[2];

				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[0];
				m_SpoofBuf3[RIGHT_RING] = spoof_score[1];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[2];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[1];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[2];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[1];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[2];

				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				m_SpoofBuf3[RIGHT_RING] = spoof_score[1];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[2];
			}
			//Two Fingers
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE ))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[1];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[1];

				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[1];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE ))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[1];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[1];

				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				m_SpoofBuf3[RIGHT_RING] = spoof_score[1];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE &&
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[1];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[1];

				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[1];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE ))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[0];
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[1];

				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[0];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[1];

				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[0];
				m_SpoofBuf3[RIGHT_RING] = spoof_score[1];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE ))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[0];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[1];

				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[0];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[1];

				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[0];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[1];
			}
			else if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE ))
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf3[RIGHT_RING] = nfiq_score[0];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[1];

				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[0];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[1];
				
				m_SpoofBuf3[RIGHT_RING] = spoof_score[0];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[1];
			}
			//1 finger
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE )
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[0];
				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[0];
				m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE )
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf3[RIGHT_RING] = nfiq_score[0];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[0];
				m_SpoofBuf3[RIGHT_RING] = spoof_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_3_W, S_IMG_3_H, 255);

				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[0];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[0];
				m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[0];
			}
		}
		else if(m_CaptureFinger < LEFT_RING_LITTLE )
		{
			memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer, image.Width*image.Height);
			IBSU_GenerateZoomOutImage(image, m_ImgBuf_S[m_CaptureFinger], S_IMG_W, S_IMG_H, 255);

			m_NFIQBuf[m_CaptureFinger] = nfiq_score[0];
			m_NFIQ2Buf[m_CaptureFinger] = nfiq2_score[0];
			m_SpoofBuf[m_CaptureFinger] = spoof_score[0];
		}
	}
}
void CMainDlg::_TwoBasicDevice(UINT m_CaptureMode, int imageStatus, int segmentImageArrayCount, int nfiq_score[IBSU_MAX_SEGMENT_COUNT], int nfiq2_score[IBSU_MAX_SEGMENT_COUNT], 
							   int spoof_score[IBSU_MAX_SEGMENT_COUNT], IBSU_ImageData image, const IBSU_ImageData *pSegmentImageArray, const IBSU_SegmentPosition *pSegmentPositionArray)
{
	if( m_CaptureMode == TWO_FINGER_FLAT_CAPTURE )
	{
		memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer, image.Width*image.Height);
		IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_4_W, S_IMG_4_H, 255);
		if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE && 
			m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE )
		{		
			m_NFIQTwoBuf1[LEFT_THUMB] = nfiq_score[0];
			m_NFIQTwoBuf2[RIGHT_THUMB] = nfiq_score[1];

			m_NFIQ2TwoBuf1[LEFT_THUMB] = nfiq2_score[0];
			m_NFIQ2TwoBuf2[RIGHT_THUMB] = nfiq2_score[1];
		}
		else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
		{
			m_NFIQTwoBuf1[LEFT_THUMB] = nfiq_score[0];
			m_NFIQ2TwoBuf1[LEFT_THUMB] = nfiq2_score[0];
		}
		else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
		{
			m_NFIQTwoBuf2[RIGHT_THUMB] = nfiq_score[0];
			m_NFIQ2TwoBuf2[RIGHT_THUMB] = nfiq2_score[0];
		}
	}
	else if(m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_BOTH )
	{
		BYTE *tmpBuf1 = new BYTE [S_IMG_3_W*S_IMG_3_H/2];
		BYTE *tmpBuffer = (BYTE*)image.Buffer;

		if(m_CaptureFinger == BOTH_THUMBS)
		{
			if((m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE ))
			{
				memset(m_ImgBuf[BOTH_THUMBS],0xFF,MAX_IMG_SIZE);
				memset(m_ImgBuf_S[BOTH_THUMBS],0xFF,MAX_IMG_SIZE);

				BYTE *tmpBuffer0 = (BYTE*)pSegmentImageArray[0].Buffer;
				BYTE *tmpBuffer1 = (BYTE*)pSegmentImageArray[1].Buffer;
				int width, height;
				int i, j, ii, jj;

				width = pSegmentImageArray[0].Width;
				height = pSegmentImageArray[0].Height;
				for(i=0; i<height; i++)
				{
					for(j=0; j<width; j++)
					{
						m_ImgBuf[BOTH_THUMBS][(i-height/2+MAX_IMG_H/2)*MAX_IMG_W+(j-width/2+MAX_IMG_W/4)] = tmpBuffer0[i*width+j];
					}
				}

				width = pSegmentImageArray[1].Width;
				height = pSegmentImageArray[1].Height;
				for(i=0; i<height; i++)
				{
					for(j=0; j<width; j++)
					{
						m_ImgBuf[BOTH_THUMBS][(i-height/2+MAX_IMG_H/2)*MAX_IMG_W+(j-width/2+MAX_IMG_W*3/4)] = tmpBuffer1[i*width+j];
					}
				}

				for(i=0; i<S_IMG_3_H; i++)
				{
					ii = i * MAX_IMG_H / S_IMG_3_H;

					for(j=0; j<S_IMG_3_W; j++)
					{
						jj = j * MAX_IMG_W / S_IMG_3_W;

						m_ImgBuf_S[BOTH_THUMBS][i*S_IMG_3_W+j] = m_ImgBuf[BOTH_THUMBS][ii*MAX_IMG_W+jj];
					}
				}

				m_NFIQTwoBuf1[LEFT_THUMB] = nfiq_score[0];
				m_NFIQTwoBuf2[RIGHT_THUMB] = nfiq_score[1];

				m_NFIQ2TwoBuf1[LEFT_THUMB] = nfiq2_score[0];
				m_NFIQ2TwoBuf2[RIGHT_THUMB] = nfiq2_score[1];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
			{
				memset(m_ImgBuf[BOTH_THUMBS],0xFF,MAX_IMG_SIZE);
				memset(m_ImgBuf_S[BOTH_THUMBS],0xFF,MAX_IMG_SIZE);

				BYTE *tmpBuffer0 = (BYTE*)pSegmentImageArray[0].Buffer;
				int width, height;
				int i, j, ii, jj;

				width = pSegmentImageArray[0].Width;
				height = pSegmentImageArray[0].Height;
				for(i=0; i<height; i++)
				{
					for(j=0; j<width; j++)
					{
						m_ImgBuf[BOTH_THUMBS][(i-height/2+MAX_IMG_H/2)*MAX_IMG_W+(j-width/2+MAX_IMG_W/4)] = tmpBuffer0[i*width+j];
					}
				}

				for(i=0; i<S_IMG_3_H; i++)
				{
					ii = i * MAX_IMG_H / S_IMG_3_H;

					for(j=0; j<S_IMG_3_W; j++)
					{
						jj = j * MAX_IMG_W / S_IMG_3_W;

						m_ImgBuf_S[BOTH_THUMBS][i*S_IMG_3_W+j] = m_ImgBuf[BOTH_THUMBS][ii*MAX_IMG_W+jj];
					}
				}

				m_NFIQTwoBuf1[LEFT_THUMB] = nfiq_score[0];
				m_NFIQ2TwoBuf1[LEFT_THUMB] = nfiq2_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
			{
				memset(m_ImgBuf[BOTH_THUMBS],0xFF,MAX_IMG_SIZE);
				memset(m_ImgBuf_S[BOTH_THUMBS],0xFF,MAX_IMG_SIZE);

				BYTE *tmpBuffer0 = (BYTE*)pSegmentImageArray[0].Buffer;
				int width, height;
				int i, j, ii, jj;

				width = pSegmentImageArray[0].Width;
				height = pSegmentImageArray[0].Height;
				for(i=0; i<height; i++)
				{
					for(j=0; j<width; j++)
					{
						m_ImgBuf[BOTH_THUMBS][(i-height/2+MAX_IMG_H/2)*MAX_IMG_W+(j-width/2+MAX_IMG_W*3/4)] = tmpBuffer0[i*width+j];
					}
				}

				for(i=0; i<S_IMG_3_H; i++)
				{
					ii = i * MAX_IMG_H / S_IMG_3_H;

					for(j=0; j<S_IMG_3_W; j++)
					{
						jj = j * MAX_IMG_W / S_IMG_3_W;

						m_ImgBuf_S[BOTH_THUMBS][i*S_IMG_3_W+j] = m_ImgBuf[BOTH_THUMBS][ii*MAX_IMG_W+jj];
					}
				}

				m_NFIQTwoBuf2[RIGHT_THUMB] = nfiq_score[0];
				m_NFIQ2TwoBuf2[RIGHT_THUMB] = nfiq2_score[0];
			}
		}
		IBSU_GenerateZoomOutImage(image, tmpBuf1, S_IMG_3_W/2, S_IMG_3_H, 255);

		if(m_CaptureFinger == LEFT_INDEX_MIDDLE)
		{
			for(int i=0; i<S_IMG_3_H; i++)
			{
				for(int j=0; j<S_IMG_3_W/2; j++)
				{
					m_ImgBuf_S[LEFT_HAND][i*S_IMG_3_W+j+S_IMG_3_W/2] = tmpBuf1[i*S_IMG_3_W/2+j];
				}
			}
			for(int i=0; i<IMG_H; i++)
			{
				for(int j=0; j<IMG_W; j++)
				{
					m_ImgBuf[LEFT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j+MAX_IMG_W/2] = tmpBuffer[i*IMG_W+j];
				}
			}

			if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[0];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[1];

				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[0];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[1];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
			{
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[0];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[0];
				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[0];
			}
		}
		else if(m_CaptureFinger == LEFT_RING_LITTLE)
		{
			for(int i=0; i<S_IMG_3_H; i++)
			{
				for(int j=0; j<S_IMG_3_W/2; j++)
				{
					m_ImgBuf_S[LEFT_HAND][i*S_IMG_3_W+j] = tmpBuf1[i*S_IMG_3_W/2+j];
				}
			}
			for(int i=0; i<IMG_H; i++)
			{
				for(int j=0; j<IMG_W; j++)
				{
					m_ImgBuf[LEFT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j] = tmpBuffer[i*IMG_W+j];
				}
			}
			if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf2[LEFT_RING] = nfiq_score[1];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[1];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				m_NFIQBuf2[LEFT_RING] = nfiq_score[0];
				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
			}
		}

		IBSU_GenerateZoomOutImage(image, tmpBuf1, S_IMG_3_W/2, S_IMG_3_H, 255);

		if(m_CaptureFinger == RIGHT_RING_LITTLE )
		{
			for(int i=0; i<S_IMG_3_H; i++)
			{
				for(int j=0; j<S_IMG_3_W/2; j++)
				{
					m_ImgBuf_S[RIGHT_HAND][i*S_IMG_3_W+j+S_IMG_3_W/2] = tmpBuf1[i*S_IMG_3_W/2+j];
				}
			}
			for(int i=0; i<IMG_H; i++)
			{
				for(int j=0; j<IMG_W; j++)
				{
					m_ImgBuf[RIGHT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j+MAX_IMG_W/2] = tmpBuffer[i*IMG_W+j];
				}
			}

			if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[0];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[1];

				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[0];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[1];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[0];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[0];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[0];
			}
		}
		else if(m_CaptureFinger == RIGHT_INDEX_MIDDLE)
		{
			for(int i=0; i<S_IMG_3_H; i++)
			{
				for(int j=0; j<S_IMG_3_W/2; j++)
				{
					m_ImgBuf_S[RIGHT_HAND][i*S_IMG_3_W+j] = tmpBuf1[i*S_IMG_3_W/2+j];
				}
			}
			for(int i=0; i<IMG_H; i++)
			{
				for(int j=0; j<IMG_W; j++)
				{
					m_ImgBuf[RIGHT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j] = tmpBuffer[i*IMG_W+j];
				}
			}
			if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[1];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[1];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
			{
				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
			{
				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[0];
				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[0];
			}
		}
		delete [] tmpBuf1;
	}
	else if(m_CaptureMode == TEN_FINGER_TWO_FLAT_CAPTURE_3IMAGE_EACH)
	{
		BYTE *tmpBuf1 = new BYTE [S_IMG_3_W*S_IMG_3_H/2];
		BYTE *tmpBuffer = (BYTE*)image.Buffer;
		IBSU_GenerateZoomOutImage(image, tmpBuf1, S_IMG_3_W/2, S_IMG_3_H, 255);

		if(m_CaptureFinger == RIGHT_THUMB)
		{
			for(int i=0; i<S_IMG_3_H; i++)
			{
				for(int j=0; j<S_IMG_3_W/2; j++)
				{
					m_ImgBuf_S[THUMB_SUM][i*S_IMG_3_W+j+S_IMG_3_W/2] = tmpBuf1[i*S_IMG_3_W/2+j];
				}
			}
			for(int i=0; i<IMG_H; i++)
			{
				for(int j=0; j<IMG_W; j++)
				{
					m_ImgBuf[THUMB_SUM][(i+MAX_IMG_H/4)*MAX_IMG_W+j+MAX_IMG_W/2] = tmpBuffer[i*IMG_W+j];
				}
			}
			m_NFIQTwoBuf2[RIGHT_THUMB] = nfiq_score[0];
			m_NFIQ2TwoBuf2[RIGHT_THUMB] = nfiq2_score[0];
		}
		else if(m_CaptureFinger ==LEFT_THUMB )
		{
			for(int i=0; i<S_IMG_3_H; i++)
			{
				for(int j=0; j<S_IMG_3_W/2; j++)
				{
					m_ImgBuf_S[THUMB_SUM][i*S_IMG_3_W+j] = tmpBuf1[i*S_IMG_3_W/2+j];
				}
			}
			for(int i=0; i<IMG_H; i++)
			{
				for(int j=0; j<IMG_W; j++)
				{
					m_ImgBuf[THUMB_SUM][(i+MAX_IMG_H/4)*MAX_IMG_W+j] = tmpBuffer[i*IMG_W+j];
				}
			}
			m_NFIQTwoBuf1[LEFT_THUMB] = nfiq_score[0];
			m_NFIQ2TwoBuf1[LEFT_THUMB] = nfiq2_score[0];
		}

		IBSU_GenerateZoomOutImage(image, tmpBuf1, S_IMG_3_W/2, S_IMG_3_H, 255);

		if(m_CaptureFinger == LEFT_INDEX_MIDDLE)
		{
			for(int i=0; i<S_IMG_3_H; i++)
			{
				for(int j=0; j<S_IMG_3_W/2; j++)
				{
					m_ImgBuf_S[LEFT_HAND][i*S_IMG_3_W+j+S_IMG_3_W/2] = tmpBuf1[i*S_IMG_3_W/2+j];
				}
			}
			for(int i=0; i<IMG_H; i++)
			{
				for(int j=0; j<IMG_W; j++)
				{
					m_ImgBuf[LEFT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j+MAX_IMG_W/2] = tmpBuffer[i*IMG_W+j];
				}
			}
			if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE )
			{
				m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[0];
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[1];

				m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[0];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[1];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) == TRUE)
			{
				m_NFIQBuf4[LEFT_INDEX] = nfiq_score[0];
				m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE) == TRUE)
			{
				m_NFIQBuf3[LEFT_MIDDLE]= nfiq_score[0];
				m_NFIQ2Buf3[LEFT_MIDDLE]= nfiq2_score[0];
			}
		}
		else if(m_CaptureFinger == LEFT_RING_LITTLE)
		{
			for(int i=0; i<S_IMG_3_H; i++)
			{
				for(int j=0; j<S_IMG_3_W/2; j++)
				{
					m_ImgBuf_S[LEFT_HAND][i*S_IMG_3_W+j] = tmpBuf1[i*S_IMG_3_W/2+j];
				}
			}
			for(int i=0; i<IMG_H; i++)
			{
				for(int j=0; j<IMG_W; j++)
				{
					m_ImgBuf[LEFT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j] = tmpBuffer[i*IMG_W+j];
				}
			}

			if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE )
			{
				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQBuf2[LEFT_RING] = nfiq_score[1];

				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[1];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) == TRUE)
			{
				m_NFIQBuf2[LEFT_RING] = nfiq_score[0];
				m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE) == TRUE)
			{
				m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
				m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
			}
		}

		IBSU_GenerateZoomOutImage(image, tmpBuf1, S_IMG_3_W/2, S_IMG_3_H, 255);

		if(m_CaptureFinger == RIGHT_RING_LITTLE )
		{
			for(int i=0; i<S_IMG_3_H; i++)
			{
				for(int j=0; j<S_IMG_3_W/2; j++)
				{
					m_ImgBuf_S[RIGHT_HAND][i*S_IMG_3_W+j+S_IMG_3_W/2] = tmpBuf1[i*S_IMG_3_W/2+j];
				}
			}
			for(int i=0; i<IMG_H; i++)
			{
				for(int j=0; j<IMG_W; j++)
				{
					m_ImgBuf[RIGHT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j+MAX_IMG_W/2] = tmpBuffer[i*IMG_W+j];
				}
			}

			if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE )
			{
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[0];
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[1];

				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[0];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[1];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) == TRUE)
			{
				m_NFIQBuf3[RIGHT_RING] = nfiq_score[0];
				m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE) == TRUE)
			{
				m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[0];
				m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[0];
			}
		}
		else if(m_CaptureFinger == RIGHT_INDEX_MIDDLE)
		{
			for(int i=0; i<S_IMG_3_H; i++)
			{
				for(int j=0; j<S_IMG_3_W/2; j++)
				{
					m_ImgBuf_S[RIGHT_HAND][i*S_IMG_3_W+j] = tmpBuf1[i*S_IMG_3_W/2+j];
				}
			}
			for(int i=0; i<IMG_H; i++)
			{
				for(int j=0; j<IMG_W; j++)
				{
					m_ImgBuf[RIGHT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j] = tmpBuffer[i*IMG_W+j];
				}
			}

			if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE && 
				m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE )
			{
				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[1];

				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[1];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) == TRUE)
			{
				m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
				m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
			}
			else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE) == TRUE)
			{
				m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[0];
				m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[0];
			}
		}
		delete [] tmpBuf1;
	}
}

void CMainDlg::_TowOtherDevice(UINT m_CaptureMode, int imageStatus, int segmentImageArrayCount, int nfiq_score[IBSU_MAX_SEGMENT_COUNT], int nfiq2_score[IBSU_MAX_SEGMENT_COUNT],
							   int spoof_score[IBSU_MAX_SEGMENT_COUNT], IBSU_ImageData image, const IBSU_ImageData *pSegmentImageArray, const IBSU_SegmentPosition *pSegmentPositionArray)
{
	if( m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_EACH )
	{
		BYTE *tmpBuf1 = new BYTE [S_IMG_1_W*S_IMG_1_H/2];
		BYTE *tmpBuffer = (BYTE*)image.Buffer;

		if(m_CaptureFinger < LEFT_RING_LITTLE)
		{
			memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer, image.Width*image.Height);
			IBSU_GenerateZoomOutImage(image, m_ImgBuf_S[m_CaptureFinger], S_IMG_W, S_IMG_H, 255);
			m_NFIQBuf[m_CaptureFinger] = nfiq_score[0];
			m_NFIQ2Buf[m_CaptureFinger] = nfiq2_score[0];
			m_SpoofBuf[m_CaptureFinger] = spoof_score[0];
		}
		else
		{
			if(m_CaptureFinger == BOTH_LEFT_THUMB )
			{
				memcpy(m_ImgBuf[BOTH_LEFT_THUMB], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[BOTH_LEFT_THUMB], S_IMG_2_W, S_IMG_2_H, 255);
				m_NFIQBuf[BOTH_LEFT_THUMB] = nfiq_score[0];
				m_NFIQ2Buf[BOTH_LEFT_THUMB] = nfiq2_score[0];
				m_SpoofBuf[BOTH_LEFT_THUMB] = spoof_score[0];
			}
			else if(m_CaptureFinger == BOTH_RIGHT_THUMB)
			{
				memcpy(m_ImgBuf[BOTH_RIGHT_THUMB], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[BOTH_RIGHT_THUMB], S_IMG_2_W, S_IMG_2_H, 255);
				m_NFIQBuf[BOTH_RIGHT_THUMB] = nfiq_score[0];
				m_NFIQ2Buf[BOTH_RIGHT_THUMB] = nfiq2_score[0];
				m_SpoofBuf[BOTH_RIGHT_THUMB] = spoof_score[0];
			}

			IBSU_GenerateZoomOutImage(image, tmpBuf1, S_IMG_1_W/2, S_IMG_1_H, 255);
			if(m_CaptureFinger == LEFT_INDEX_MIDDLE)
			{
				for(int i=0; i<S_IMG_1_H; i++)
				{
					for(int j=0; j<S_IMG_1_W/2; j++)
					{
						m_ImgBuf_S[LEFT_HAND][i*S_IMG_1_W+j+S_IMG_1_W/2] = tmpBuf1[i*S_IMG_1_W/2+j];
					}
				}
				for(int i=0; i<IMG_H; i++)
				{
					for(int j=0; j<IMG_W; j++)
					{
						m_ImgBuf[LEFT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j+MAX_IMG_W/2] = tmpBuffer[i*IMG_W+j];
					}
				}
				if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) && 
					m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE))
				{
					m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[0];
					m_NFIQBuf4[LEFT_INDEX] = nfiq_score[1];

					m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[0];
					m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[1];

					m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[0];
					m_SpoofBuf4[LEFT_INDEX] = spoof_score[1];
				}
				else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX))
				{
					m_NFIQBuf4[LEFT_INDEX] = nfiq_score[0];
					m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[0];
					m_SpoofBuf4[LEFT_INDEX] = spoof_score[0];
				}
				else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE))
				{
					m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[0];
					m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[0];
					m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[0];
				}
			}
			else if(m_CaptureFinger == LEFT_RING_LITTLE)
			{
				for(int i=0; i<S_IMG_1_H; i++)
				{
					for(int j=0; j<S_IMG_1_W/2; j++)
					{
						m_ImgBuf_S[LEFT_HAND][i*S_IMG_1_W+j] = tmpBuf1[i*S_IMG_1_W/2+j];
					}
				}
				for(int i=0; i<IMG_H; i++)
				{
					for(int j=0; j<IMG_W; j++)
					{
						m_ImgBuf[LEFT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j] = tmpBuffer[i*IMG_W+j];
					}
				}

				if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) && 
					m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE))
				{
					m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
					m_NFIQBuf2[LEFT_RING] = nfiq_score[1];

					m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
					m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[1];

					m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
					m_SpoofBuf2[LEFT_RING] = spoof_score[1];
				}
				else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING))
				{
					m_NFIQBuf2[LEFT_RING] = nfiq_score[0];
					m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[0];
					m_SpoofBuf2[LEFT_RING] = spoof_score[0];
				}
				else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE))
				{
					m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
					m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
					m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
				}
			}

			if(m_CaptureFinger == RIGHT_RING_LITTLE )
			{
				for(int i=0; i<S_IMG_1_H; i++)
				{
					for(int j=0; j<S_IMG_1_W/2; j++)
					{
						m_ImgBuf_S[RIGHT_HAND][i*S_IMG_1_W+j+S_IMG_1_W/2] = tmpBuf1[i*S_IMG_1_W/2+j];
					}
				}
				for(int i=0; i<IMG_H; i++)
				{
					for(int j=0; j<IMG_W; j++)
					{
						m_ImgBuf[RIGHT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j+MAX_IMG_W/2] = tmpBuffer[i*IMG_W+j];
					}
				}

				if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) && 
					m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE))
				{
					m_NFIQBuf3[RIGHT_RING] = nfiq_score[0];
					m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[1];

					m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[0];
					m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[1];

					m_SpoofBuf3[RIGHT_RING] = spoof_score[0];
					m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[1];
				}
				else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING))
				{
					m_NFIQBuf3[RIGHT_RING] = nfiq_score[0];
					m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[0];
					m_SpoofBuf3[RIGHT_RING] = spoof_score[0];
				}
				else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE))
				{
					m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[0];
					m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[0];
					m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[0];
				}
			}
			else if(m_CaptureFinger == RIGHT_INDEX_MIDDLE)
			{
				for(int i=0; i<S_IMG_1_H; i++)
				{
					for(int j=0; j<S_IMG_1_W/2; j++)
					{
						m_ImgBuf_S[RIGHT_HAND][i*S_IMG_1_W+j] = tmpBuf1[i*S_IMG_1_W/2+j];
					}
				}
				for(int i=0; i<IMG_H; i++)
				{
					for(int j=0; j<IMG_W; j++)
					{
						m_ImgBuf[RIGHT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j] = tmpBuffer[i*IMG_W+j];
					}
				}
				if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) && 
					m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE))
				{
					m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
					m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[1];

					m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
					m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[1];

					m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
					m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[1];
				}
				else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX))
				{
					m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
					m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
					m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
				}
				else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE))
				{
					m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[0];
					m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[0];
					m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[0];
				}
			}				
		}

		delete [] tmpBuf1;
	}

	else if( m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_SIXTEEN )
	{
		if(m_CaptureFinger <LEFT_RING_LITTLE)
		{
			memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer, image.Width*image.Height);
			IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_W, S_IMG_H, 255);
			m_NFIQBuf[m_CaptureFinger] = nfiq_score[0];
			m_NFIQ2Buf[m_CaptureFinger] = nfiq2_score[0];
			m_SpoofBuf[m_CaptureFinger] = spoof_score[0];
		}
		else
		{
			if(m_CaptureFinger == BOTH_LEFT_THUMB)
			{
				memcpy(m_ImgBuf[BOTH_LEFT_THUMB], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[BOTH_LEFT_THUMB], S_IMG_2_W, S_IMG_2_H, 255);
				m_NFIQBuf[BOTH_LEFT_THUMB] = nfiq_score[0];
				m_NFIQ2Buf[BOTH_LEFT_THUMB] = nfiq2_score[0];
				m_SpoofBuf[BOTH_LEFT_THUMB] = spoof_score[0];
			}
			else if(m_CaptureFinger == BOTH_RIGHT_THUMB)
			{
				memcpy(m_ImgBuf[BOTH_RIGHT_THUMB], image.Buffer,image.Width*image.Height);
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[BOTH_RIGHT_THUMB], S_IMG_2_W, S_IMG_2_H, 255);
				m_NFIQBuf[BOTH_RIGHT_THUMB] = nfiq_score[0];
				m_NFIQ2Buf[BOTH_RIGHT_THUMB] = nfiq2_score[0];
				m_SpoofBuf[BOTH_RIGHT_THUMB] = spoof_score[0];
			}
			else
			{
				memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer, image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[m_CaptureFinger], S_IMG_2_W, S_IMG_2_H, 255);

				if(m_CaptureFinger == LEFT_INDEX_MIDDLE)
				{
					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE))
					{
						m_NFIQTwoBuf1[LEFT_MIDDLE] = nfiq_score[0];
						m_NFIQTwoBuf2[LEFT_INDEX] = nfiq_score[1];

						m_NFIQ2TwoBuf1[LEFT_MIDDLE] = nfiq2_score[0];
						m_NFIQ2TwoBuf2[LEFT_INDEX] = nfiq2_score[1];

						m_SpoofTwoBuf1[LEFT_MIDDLE] = spoof_score[0];
						m_SpoofTwoBuf2[LEFT_INDEX] = spoof_score[1];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX))
					{
						m_NFIQTwoBuf2[LEFT_INDEX] = nfiq_score[0];
						m_NFIQ2TwoBuf2[LEFT_INDEX] = nfiq2_score[0];
						m_SpoofTwoBuf2[LEFT_INDEX] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE))
					{
						m_NFIQTwoBuf1[LEFT_MIDDLE] = nfiq_score[0];
						m_NFIQ2TwoBuf1[LEFT_MIDDLE] = nfiq2_score[0];
						m_SpoofTwoBuf1[LEFT_MIDDLE] = spoof_score[0];
					}
				}
				else if(m_CaptureFinger == LEFT_RING_LITTLE)
				{
					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE))
					{
						m_NFIQTwoBuf2[LEFT_RING] = nfiq_score[1];
						m_NFIQTwoBuf1[LEFT_LITTLE] = nfiq_score[0];

						m_NFIQ2TwoBuf2[LEFT_RING] = nfiq2_score[1];
						m_NFIQ2TwoBuf1[LEFT_LITTLE] = nfiq2_score[0];

						m_SpoofTwoBuf2[LEFT_RING] = spoof_score[1];
						m_SpoofTwoBuf1[LEFT_LITTLE] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING))
					{
						m_NFIQTwoBuf2[LEFT_RING] = nfiq_score[0];
						m_NFIQ2TwoBuf2[LEFT_RING] = nfiq2_score[0];
						m_SpoofTwoBuf2[LEFT_RING] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE))
					{
						m_NFIQTwoBuf1[LEFT_LITTLE] = nfiq_score[0];
						m_NFIQ2TwoBuf1[LEFT_LITTLE] = nfiq2_score[0];
						m_SpoofTwoBuf1[LEFT_LITTLE] = spoof_score[0];
					}
				}
				else if(m_CaptureFinger == RIGHT_INDEX_MIDDLE)
				{
					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE))
					{
						m_NFIQTwoBuf1[RIGHT_INDEX] = nfiq_score[0];
						m_NFIQTwoBuf2[RIGHT_MIDDLE] = nfiq_score[1];

						m_NFIQ2TwoBuf1[RIGHT_INDEX] = nfiq2_score[0];
						m_NFIQ2TwoBuf2[RIGHT_MIDDLE] = nfiq2_score[1];

						m_SpoofTwoBuf1[RIGHT_INDEX] = spoof_score[0];
						m_SpoofTwoBuf2[RIGHT_MIDDLE] = spoof_score[1];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX))
					{
						m_NFIQTwoBuf1[RIGHT_INDEX] = nfiq_score[0];
						m_NFIQ2TwoBuf1[RIGHT_INDEX] = nfiq2_score[0];
						m_SpoofTwoBuf1[RIGHT_INDEX] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE))
					{
						m_NFIQTwoBuf2[RIGHT_MIDDLE] = nfiq_score[0];
						m_NFIQ2TwoBuf2[RIGHT_MIDDLE] = nfiq2_score[0];
						m_SpoofTwoBuf2[RIGHT_MIDDLE] = spoof_score[0];
					}
				}
				else if(m_CaptureFinger == RIGHT_RING_LITTLE)
				{
					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE))
					{
						m_NFIQTwoBuf1[RIGHT_RING] = nfiq_score[0];
						m_NFIQTwoBuf2[RIGHT_LITTLE] = nfiq_score[1];

						m_NFIQ2TwoBuf1[RIGHT_RING] = nfiq2_score[0];
						m_NFIQ2TwoBuf2[RIGHT_LITTLE] = nfiq2_score[1];

						m_SpoofTwoBuf1[RIGHT_RING] = spoof_score[0];
						m_SpoofTwoBuf2[RIGHT_LITTLE] = spoof_score[1];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING))
					{
						m_NFIQTwoBuf1[RIGHT_RING] = nfiq_score[0];
						m_NFIQ2TwoBuf1[RIGHT_RING] = nfiq2_score[0];
						m_SpoofTwoBuf1[RIGHT_RING] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE))
					{
						m_NFIQTwoBuf2[RIGHT_LITTLE] = nfiq_score[0];
						m_NFIQ2TwoBuf2[RIGHT_LITTLE] = nfiq2_score[0];
						m_SpoofTwoBuf2[RIGHT_LITTLE] = spoof_score[0];						
					}
				}
			}
		}
	}

	else if(m_CaptureMode == TEN_FINGER_TWO_FLAT_DOUBLE_THUMB_ROLL)
	{
		BYTE *tmpBuf1 = new BYTE [S_IMG_1_W*S_IMG_1_H/2];
		BYTE *tmpBuffer = (BYTE*)image.Buffer;

		if(m_CaptureFinger <LEFT_RING_LITTLE)
		{
			memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer, image.Width*image.Height);
			IBSU_GenerateZoomOutImage(pSegmentImageArray[0],  m_ImgBuf_S[m_CaptureFinger], S_IMG_W, S_IMG_H, 255);
			m_NFIQBuf[m_CaptureFinger] = nfiq_score[0];
			m_NFIQ2Buf[m_CaptureFinger] = nfiq2_score[0];
			m_SpoofBuf[m_CaptureFinger] = spoof_score[0];
		}
		else
		{
			if(m_CaptureFinger == BOTH_LEFT_THUMB)
			{
				memcpy(m_ImgBuf[BOTH_LEFT_THUMB], image.Buffer, image.Width*image.Height);
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[BOTH_LEFT_THUMB], S_IMG_2_W, S_IMG_2_H, 255);
				m_NFIQBuf[BOTH_LEFT_THUMB] = nfiq_score[0];
				m_NFIQ2Buf[BOTH_LEFT_THUMB] = nfiq2_score[0];
				m_SpoofBuf[BOTH_LEFT_THUMB] = spoof_score[0];
			}
			if(m_CaptureFinger == BOTH_RIGHT_THUMB)
			{
				memcpy(m_ImgBuf[BOTH_RIGHT_THUMB], image.Buffer, image.Width*image.Height);
				IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[BOTH_RIGHT_THUMB], S_IMG_2_W, S_IMG_2_H, 255);
				m_NFIQBuf[BOTH_RIGHT_THUMB] = nfiq_score[0];
				m_NFIQ2Buf[BOTH_RIGHT_THUMB] = nfiq2_score[0];
				m_SpoofBuf[BOTH_RIGHT_THUMB] = spoof_score[0];
			}
			else if(m_CaptureFinger == BOTH_THUMBS)
			{
				memcpy(m_ImgBuf[BOTH_THUMBS], image.Buffer, image.Width*image.Height);
				IBSU_GenerateZoomOutImage(image, m_ImgBuf_S[BOTH_THUMBS], S_IMG_1_W, S_IMG_1_H, 255);

				if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) && m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB))
				{
					m_NFIQTwoBuf1[LEFT_THUMB] = nfiq_score[0];
					m_NFIQTwoBuf2[RIGHT_THUMB] = nfiq_score[1];

					m_NFIQ2TwoBuf1[LEFT_THUMB] = nfiq2_score[0];
					m_NFIQ2TwoBuf2[RIGHT_THUMB] = nfiq2_score[1];

					m_SpoofTwoBuf1[LEFT_THUMB] = spoof_score[0];
					m_SpoofTwoBuf2[RIGHT_THUMB] = spoof_score[1];
				}
				else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB))
				{
					memcpy(m_ImgBuf[LEFT_THUMB], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
					m_nCurrentWidthForSplit[LEFT_THUMB] =pSegmentImageArray[0].Width;
					m_nCurrentHeightForSplit[LEFT_THUMB]=pSegmentImageArray[0].Height;
					IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[LEFT_THUMB], S_IMG_2_W, S_IMG_2_H, 255);
					m_NFIQTwoBuf1[LEFT_THUMB] = nfiq_score[0];
					m_NFIQ2TwoBuf1[LEFT_THUMB] = nfiq2_score[0];
					m_SpoofTwoBuf1[LEFT_THUMB] = spoof_score[0];
				}
				else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB))
				{
					memcpy(m_ImgBuf[RIGHT_THUMB], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
					m_nCurrentWidthForSplit[RIGHT_THUMB] =pSegmentImageArray[0].Width;
					m_nCurrentHeightForSplit[RIGHT_THUMB]=pSegmentImageArray[0].Height;
					IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[RIGHT_THUMB], S_IMG_2_W, S_IMG_2_H, 255);
					m_NFIQTwoBuf2[RIGHT_THUMB] = nfiq_score[0];
					m_NFIQ2TwoBuf2[RIGHT_THUMB] = nfiq2_score[0];
					m_SpoofTwoBuf2[RIGHT_THUMB] = spoof_score[0];
				}
			}
			else
			{
				///////////////////////////////////////////////LEFT HAND////////////////////////////////////////////////////////////
				IBSU_GenerateZoomOutImage(image, tmpBuf1, S_IMG_1_W/2, S_IMG_1_H, 255);

				if(m_CaptureFinger == LEFT_INDEX_MIDDLE)
				{
					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE))
					{
						m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[0];
						m_NFIQBuf4[LEFT_INDEX] = nfiq_score[1];

						m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[0];
						m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[1];

						m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[0];
						m_SpoofBuf4[LEFT_INDEX] = spoof_score[1];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX))
					{
						m_NFIQBuf4[LEFT_INDEX] = nfiq_score[0];
						m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[0];
						m_SpoofBuf4[LEFT_INDEX] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE))
					{
						m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[0];
						m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[0];
						m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[0];
					}
					for(int i=0; i<S_IMG_1_H; i++)
					{
						for(int j=0; j<S_IMG_1_W/2; j++)
						{
							m_ImgBuf_S[LEFT_HAND][i*S_IMG_1_W+j+S_IMG_1_W/2] = tmpBuf1[i*S_IMG_1_W/2+j];
						}
					}
					for(int i=0; i<IMG_H; i++)
					{
						for(int j=0; j<IMG_W; j++)
						{
							m_ImgBuf[LEFT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j+MAX_IMG_W/2] = tmpBuffer[i*IMG_W+j];
						}
					}
				}
				else if(m_CaptureFinger == LEFT_RING_LITTLE)
				{
					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE))
					{
						m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
						m_NFIQBuf2[LEFT_RING] = nfiq_score[1];

						m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
						m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[1];

						m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
						m_SpoofBuf2[LEFT_RING] = spoof_score[1];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE))
					{
						m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
						m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
						m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING))
					{
						m_NFIQBuf2[LEFT_RING] = nfiq_score[0];
						m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[0];
						m_SpoofBuf2[LEFT_RING] = spoof_score[0];
					}

					for(int i=0; i<S_IMG_1_H; i++)
					{
						for(int j=0; j<S_IMG_1_W/2; j++)
						{
							m_ImgBuf_S[LEFT_HAND][i*S_IMG_1_W+j] = tmpBuf1[i*S_IMG_1_W/2+j];
						}
					}
					for(int i=0; i<IMG_H; i++)
					{
						for(int j=0; j<IMG_W; j++)
						{
							m_ImgBuf[LEFT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j] = tmpBuffer[i*IMG_W+j];
						}
					}
				}
				if(m_CaptureFinger == RIGHT_RING_LITTLE )
				{
					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE))
					{
						m_NFIQBuf3[RIGHT_RING] = nfiq_score[0];
						m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[1];

						m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[0];
						m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[1];

						m_SpoofBuf3[RIGHT_RING] = spoof_score[0];
						m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[1];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING))
					{
						m_NFIQBuf3[RIGHT_RING] = nfiq_score[0];
						m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[0];
						m_SpoofBuf3[RIGHT_RING] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE))
					{
						m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[0];
						m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[0];
						m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[0];
					}

					for(int i=0; i<S_IMG_1_H; i++)
					{
						for(int j=0; j<S_IMG_1_W/2; j++)
						{
							m_ImgBuf_S[RIGHT_HAND][i*S_IMG_1_W+j+S_IMG_1_W/2] = tmpBuf1[i*S_IMG_1_W/2+j];
						}
					}
					for(int i=0; i<IMG_H; i++)
					{
						for(int j=0; j<IMG_W; j++)
						{
							m_ImgBuf[RIGHT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j+MAX_IMG_W/2] = tmpBuffer[i*IMG_W+j];
						}
					}
				}
				else if(m_CaptureFinger == RIGHT_INDEX_MIDDLE)
				{
					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE))
					{
						m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
						m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[1];

						m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
						m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[1];

						m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
						m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[1];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX))
					{
						m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
						m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
						m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE))
					{
						m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[0];
						m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[0];
						m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[0];
					}

					for(int i=0; i<S_IMG_1_H; i++)
					{
						for(int j=0; j<S_IMG_1_W/2; j++)
						{
							m_ImgBuf_S[RIGHT_HAND][i*S_IMG_1_W+j] = tmpBuf1[i*S_IMG_1_W/2+j];
						}
					}
					for(int i=0; i<IMG_H; i++)
					{
						for(int j=0; j<IMG_W; j++)
						{
							m_ImgBuf[RIGHT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j] = tmpBuffer[i*IMG_W+j];
						}
					}
				}

				if(m_CaptureFinger == LEFT_RING_LITTLE)
				{
					memcpy(m_ImgBuf[LEFT_RING_LITTLE], image.Buffer, image.Width*image.Height);
					IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[LEFT_RING_LITTLE], S_IMG_2_W, S_IMG_2_H, 255);

					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE))
					{
						m_NFIQTwoBuf1[LEFT_LITTLE] = nfiq_score[0];
						m_NFIQTwoBuf2[LEFT_RING] = nfiq_score[1];

						m_NFIQ2TwoBuf1[LEFT_LITTLE] = nfiq2_score[0];
						m_NFIQ2TwoBuf2[LEFT_RING] = nfiq2_score[1];

						m_SpoofTwoBuf1[LEFT_LITTLE] = spoof_score[0];
						m_SpoofTwoBuf2[LEFT_RING] = spoof_score[1];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING))
					{
						m_NFIQTwoBuf2[LEFT_RING] = nfiq_score[0];
						m_NFIQ2TwoBuf2[LEFT_RING] = nfiq2_score[0];
						m_SpoofTwoBuf2[LEFT_RING] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE))
					{
						m_NFIQTwoBuf1[LEFT_LITTLE] = nfiq_score[0];
						m_NFIQ2TwoBuf1[LEFT_LITTLE] = nfiq2_score[0];
						m_SpoofTwoBuf1[LEFT_LITTLE] = spoof_score[0];
					}
				}
				if(m_CaptureFinger == LEFT_INDEX_MIDDLE)
				{
					memcpy(m_ImgBuf[LEFT_INDEX_MIDDLE], image.Buffer, image.Width*image.Height);
					IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[LEFT_INDEX_MIDDLE], S_IMG_2_W, S_IMG_2_H, 255);

					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE))
					{
						m_NFIQTwoBuf1[LEFT_MIDDLE] = nfiq_score[0];
						m_NFIQTwoBuf2[LEFT_INDEX] = nfiq_score[1];

						m_NFIQ2TwoBuf1[LEFT_MIDDLE] = nfiq2_score[0];
						m_NFIQ2TwoBuf2[LEFT_INDEX] = nfiq2_score[1];

						m_SpoofTwoBuf1[LEFT_MIDDLE] = spoof_score[0];
						m_SpoofTwoBuf2[LEFT_INDEX] = spoof_score[1];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE))
					{
						m_NFIQTwoBuf1[LEFT_MIDDLE] = nfiq_score[0];
						m_NFIQ2TwoBuf1[LEFT_MIDDLE] = nfiq2_score[0];
						m_SpoofTwoBuf1[LEFT_MIDDLE] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX))
					{
						m_NFIQTwoBuf2[LEFT_INDEX] = nfiq_score[0];
						m_NFIQ2TwoBuf2[LEFT_INDEX] = nfiq2_score[0];
						m_SpoofTwoBuf2[LEFT_INDEX] = spoof_score[0];
					}
				}
				if(m_CaptureFinger == RIGHT_RING_LITTLE)
				{
					memcpy(m_ImgBuf[RIGHT_RING_LITTLE], image.Buffer, image.Width*image.Height);
					IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[RIGHT_RING_LITTLE], S_IMG_2_W, S_IMG_2_H, 255);

					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE))
					{
						m_NFIQTwoBuf1[RIGHT_RING] = nfiq_score[0];
						m_NFIQTwoBuf2[RIGHT_LITTLE] = nfiq_score[1];

						m_NFIQ2TwoBuf1[RIGHT_RING] = nfiq2_score[0];
						m_NFIQ2TwoBuf2[RIGHT_LITTLE] = nfiq2_score[1];

						m_SpoofTwoBuf1[RIGHT_RING] = spoof_score[0];
						m_SpoofTwoBuf2[RIGHT_LITTLE] = spoof_score[1];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING))
					{
						m_NFIQTwoBuf1[RIGHT_RING] = nfiq_score[0];
						m_NFIQ2TwoBuf1[RIGHT_RING] = nfiq2_score[0];
						m_SpoofTwoBuf1[RIGHT_RING] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE))
					{
						m_NFIQTwoBuf2[RIGHT_LITTLE] = nfiq_score[0];
						m_NFIQ2TwoBuf2[RIGHT_LITTLE] = nfiq2_score[0];
						m_SpoofTwoBuf2[RIGHT_LITTLE] = spoof_score[0];
					}
				}
				if(m_CaptureFinger == RIGHT_INDEX_MIDDLE)
				{
					memcpy(m_ImgBuf[RIGHT_INDEX_MIDDLE], image.Buffer, image.Width*image.Height);
					IBSU_GenerateZoomOutImage(image,  m_ImgBuf_S[RIGHT_INDEX_MIDDLE], S_IMG_2_W, S_IMG_2_H, 255);

					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE))
					{
						m_NFIQTwoBuf1[RIGHT_INDEX] = nfiq_score[0];
						m_NFIQTwoBuf2[RIGHT_MIDDLE] = nfiq_score[1];

						m_NFIQ2TwoBuf1[RIGHT_INDEX] = nfiq2_score[0];
						m_NFIQ2TwoBuf2[RIGHT_MIDDLE] = nfiq2_score[1];

						m_SpoofTwoBuf1[RIGHT_INDEX] = spoof_score[0];
						m_SpoofTwoBuf2[RIGHT_MIDDLE] = spoof_score[1];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX))
					{
						m_NFIQTwoBuf1[RIGHT_INDEX] = nfiq_score[0];
						m_NFIQ2TwoBuf1[RIGHT_INDEX] = nfiq2_score[0];
						m_SpoofTwoBuf1[RIGHT_INDEX] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE))
					{
						m_NFIQTwoBuf2[RIGHT_MIDDLE] = nfiq_score[0];
						m_NFIQ2TwoBuf2[RIGHT_MIDDLE] = nfiq2_score[0];
						m_SpoofTwoBuf2[RIGHT_MIDDLE] = spoof_score[0];
					}
				}
			}
		}

		delete [] tmpBuf1;
	}
	else if( m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_SINGLE_FLAT_CAPTURE ||m_CaptureMode == TEN_FINGER_TWO_FLAT_AND_ROLL_CAPTURE_BOTH )
	{
		BYTE *tmpBuf1 = new BYTE [S_IMG_1_W*S_IMG_1_H/2];
		BYTE *tmpBuffer = (BYTE*)image.Buffer;

		if(m_CaptureFinger < LEFT_RING_LITTLE )
		{
			memcpy(m_ImgBuf[m_CaptureFinger], image.Buffer, image.Width*image.Height);
			IBSU_GenerateZoomOutImage(image, m_ImgBuf_S[m_CaptureFinger], S_IMG_W, S_IMG_H, 255);
			m_NFIQBuf[m_CaptureFinger] = nfiq_score[0];
			m_NFIQ2Buf[m_CaptureFinger] = nfiq2_score[0];
			m_SpoofBuf[m_CaptureFinger] = spoof_score[0];
		}
		else
		{
			if(m_CaptureFinger == BOTH_THUMBS)
			{
				if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE && 
					m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
				{
					memcpy(m_ImgBuf[BOTH_LEFT_THUMB], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
					m_nCurrentWidthForSplit[BOTH_LEFT_THUMB] =pSegmentImageArray[0].Width;
					m_nCurrentHeightForSplit[BOTH_LEFT_THUMB]=pSegmentImageArray[0].Height;
					IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[BOTH_LEFT_THUMB], S_IMG_2_W, S_IMG_2_H, 255);
					m_NFIQBuf[BOTH_LEFT_THUMB] = nfiq_score[0];
					m_NFIQ2Buf[BOTH_LEFT_THUMB] = nfiq2_score[0];
					m_SpoofBuf[BOTH_LEFT_THUMB] = spoof_score[0];

					memcpy(m_ImgBuf[BOTH_RIGHT_THUMB], pSegmentImageArray[1].Buffer, pSegmentImageArray[1].Width*pSegmentImageArray[1].Height);
					m_nCurrentWidthForSplit[BOTH_RIGHT_THUMB] =pSegmentImageArray[1].Width;
					m_nCurrentHeightForSplit[BOTH_RIGHT_THUMB]=pSegmentImageArray[1].Height;
					IBSU_GenerateZoomOutImage(pSegmentImageArray[1], m_ImgBuf_S[BOTH_RIGHT_THUMB], S_IMG_2_W, S_IMG_2_H, 255);
					m_NFIQBuf[BOTH_RIGHT_THUMB] = nfiq_score[1];
					m_NFIQ2Buf[BOTH_RIGHT_THUMB] = nfiq2_score[1];
					m_SpoofBuf[BOTH_RIGHT_THUMB] = spoof_score[1];
				}
				else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_THUMB) == TRUE)
				{
					memcpy(m_ImgBuf[BOTH_LEFT_THUMB], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
					m_nCurrentWidthForSplit[BOTH_LEFT_THUMB] =pSegmentImageArray[0].Width;
					m_nCurrentHeightForSplit[BOTH_LEFT_THUMB]=pSegmentImageArray[0].Height;
					IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[BOTH_LEFT_THUMB], S_IMG_2_W, S_IMG_2_H, 255);
					m_NFIQBuf[BOTH_LEFT_THUMB] = nfiq_score[0];
					m_NFIQ2Buf[BOTH_LEFT_THUMB] = nfiq2_score[0];
					m_SpoofBuf[BOTH_LEFT_THUMB] = spoof_score[0];
				}
				else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_THUMB) == TRUE)
				{
					memcpy(m_ImgBuf[BOTH_RIGHT_THUMB], pSegmentImageArray[0].Buffer, pSegmentImageArray[0].Width*pSegmentImageArray[0].Height);
					m_nCurrentWidthForSplit[BOTH_RIGHT_THUMB] =pSegmentImageArray[0].Width;
					m_nCurrentHeightForSplit[BOTH_RIGHT_THUMB]=pSegmentImageArray[0].Height;
					IBSU_GenerateZoomOutImage(pSegmentImageArray[0], m_ImgBuf_S[BOTH_RIGHT_THUMB], S_IMG_2_W, S_IMG_2_H, 255);
					m_NFIQBuf[BOTH_RIGHT_THUMB] = nfiq_score[0];
					m_NFIQ2Buf[BOTH_RIGHT_THUMB] = nfiq2_score[0];
					m_SpoofBuf[BOTH_RIGHT_THUMB] = spoof_score[0];
				}
			}
			else
			{
				IBSU_GenerateZoomOutImage(image, tmpBuf1, S_IMG_1_W/2, S_IMG_1_H, 255);
				if(m_CaptureFinger == LEFT_INDEX_MIDDLE)
				{
					for(int i=0; i<S_IMG_1_H; i++)
					{
						for(int j=0; j<S_IMG_1_W/2; j++)
						{
							m_ImgBuf_S[LEFT_HAND][i*S_IMG_1_W+j+S_IMG_1_W/2] = tmpBuf1[i*S_IMG_1_W/2+j];
						}
					}
					for(int i=0; i<IMG_H; i++)
					{
						for(int j=0; j<IMG_W; j++)
						{
							m_ImgBuf[LEFT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j+MAX_IMG_W/2] = tmpBuffer[i*IMG_W+j];
						}
					}

					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE))
					{
						m_NFIQBuf4[LEFT_INDEX] = nfiq_score[1];
						m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[0];

						m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[1];
						m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[0];

						m_SpoofBuf4[LEFT_INDEX] = spoof_score[1];
						m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_INDEX))
					{
						m_NFIQBuf4[LEFT_INDEX] = nfiq_score[0];
						m_NFIQ2Buf4[LEFT_INDEX] = nfiq2_score[0];
						m_SpoofBuf4[LEFT_INDEX] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_MIDDLE))
					{
						m_NFIQBuf3[LEFT_MIDDLE] = nfiq_score[0];
						m_NFIQ2Buf3[LEFT_MIDDLE] = nfiq2_score[0];
						m_SpoofBuf3[LEFT_MIDDLE] = spoof_score[0];
					}
				}
				else if(m_CaptureFinger == LEFT_RING_LITTLE)
				{
					for(int i=0; i<S_IMG_1_H; i++)
					{
						for(int j=0; j<S_IMG_1_W/2; j++)
						{
							m_ImgBuf_S[LEFT_HAND][i*S_IMG_1_W+j] = tmpBuf1[i*S_IMG_1_W/2+j];
						}
					}
					for(int i=0; i<IMG_H; i++)
					{
						for(int j=0; j<IMG_W; j++)
						{
							m_ImgBuf[LEFT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j] = tmpBuffer[i*IMG_W+j];
						}
					}

					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE))
					{
						m_NFIQBuf2[LEFT_RING] = nfiq_score[1];
						m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];

						m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[1];
						m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];

						m_SpoofBuf2[LEFT_RING] = spoof_score[1];
						m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_RING))
					{
						m_NFIQBuf2[LEFT_RING] = nfiq_score[0];
						m_NFIQ2Buf2[LEFT_RING] = nfiq2_score[0];
						m_SpoofBuf2[LEFT_RING] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(LEFT_LITTLE))
					{
						m_NFIQBuf1[LEFT_LITTLE] = nfiq_score[0];
						m_NFIQ2Buf1[LEFT_LITTLE] = nfiq2_score[0];
						m_SpoofBuf1[LEFT_LITTLE] = spoof_score[0];
					}
				}

				if(m_CaptureFinger == RIGHT_RING_LITTLE )
				{
					for(int i=0; i<S_IMG_1_H; i++)
					{
						for(int j=0; j<S_IMG_1_W/2; j++)
						{
							m_ImgBuf_S[RIGHT_HAND][i*S_IMG_1_W+j+S_IMG_1_W/2] = tmpBuf1[i*S_IMG_1_W/2+j];
						}
					}
					for(int i=0; i<IMG_H; i++)
					{
						for(int j=0; j<IMG_W; j++)
						{
							m_ImgBuf[RIGHT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j+MAX_IMG_W/2] = tmpBuffer[i*IMG_W+j];
						}
					}

					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE))
					{
						m_NFIQBuf3[RIGHT_RING] = nfiq_score[0];
						m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[1];

						m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[0];
						m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[1];

						m_SpoofBuf3[RIGHT_RING] = spoof_score[0];
						m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[1];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_RING))
					{
						m_NFIQBuf3[RIGHT_RING] = nfiq_score[0];
						m_NFIQ2Buf3[RIGHT_RING] = nfiq2_score[0];
						m_SpoofBuf3[RIGHT_RING] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_LITTLE))
					{
						m_NFIQBuf4[RIGHT_LITTLE] = nfiq_score[1];
						m_NFIQ2Buf4[RIGHT_LITTLE] = nfiq2_score[1];
						m_SpoofBuf4[RIGHT_LITTLE] = spoof_score[1];
					}
				}
				else if(m_CaptureFinger == RIGHT_INDEX_MIDDLE)
				{
					for(int i=0; i<S_IMG_1_H; i++)
					{
						for(int j=0; j<S_IMG_1_W/2; j++)
						{
							m_ImgBuf_S[RIGHT_HAND][i*S_IMG_1_W+j] = tmpBuf1[i*S_IMG_1_W/2+j];
						}
					}
					for(int i=0; i<IMG_H; i++)
					{
						for(int j=0; j<IMG_W; j++)
						{
							m_ImgBuf[RIGHT_HAND][(i+MAX_IMG_H/4)*MAX_IMG_W+j] = tmpBuffer[i*IMG_W+j];
						}
					}

					if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX) && 
						m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE))
					{
						m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
						m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[1];

						m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
						m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[1];

						m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
						m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[1];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_INDEX))
					{
						m_NFIQBuf1[RIGHT_INDEX] = nfiq_score[0];
						m_NFIQ2Buf1[RIGHT_INDEX] = nfiq2_score[0];
						m_SpoofBuf1[RIGHT_INDEX] = spoof_score[0];
					}
					else if(m_pParent->m_ScanFingerDlg.m_FingerDisplayManager->IsEnableFinger(RIGHT_MIDDLE))
					{
						m_NFIQBuf2[RIGHT_MIDDLE] = nfiq_score[0];
						m_NFIQ2Buf2[RIGHT_MIDDLE] = nfiq2_score[0];
						m_SpoofBuf2[RIGHT_MIDDLE] = spoof_score[0];
					}
				}
			}
		}

		delete [] tmpBuf1;
	}
}

void CMainDlg::OnTimer(UINT_PTR nIDEvent)
{
    const int	devIndex = m_DeviceList.GetCurSel() - 1;
	int			nRc = IBSU_STATUS_OK;

    KillTimer(0);

    BeginWaitCursor();

	CString open_time;
	
	double start_clock = clock();

    m_pParent->m_bInitializing = true;

	// Asynchronously Open Device
	// Function IBSU_OpenDevice() will not be blocked while initializing the algorithm (4 - 7seconds).
	// And you can start the capture after getting callback funcation IBSU_CallbackAsyncOpenDevice().		
    nRc = IBSU_AsyncOpenDevice( devIndex );
	m_pParent->m_bInitializing = false;

	if( nRc >= IBSU_STATUS_OK )
	{
		_WaitingForFinishInitDevice( devIndex );
	}

    EndWaitCursor();

	double end_clock = clock();

	open_time.Format("Initializing Time : %.1f seconds", (end_clock - start_clock) / 1000.0f);
	m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_TXT_STATUS)->SetWindowTextA(open_time);

    CDialog::OnTimer(nIDEvent);
}

int CMainDlg::_WaitingForFinishInitDevice( const int devIndex )
{
	int		nRc;
	BOOL	IsComplete;
	int		devHandle;
	int		progressValue;
	CWnd		*disWnd;
	RECT		clientRect;
    CString str;
    int     ledCount;
    DWORD   operableLEDs;

	while( TRUE )
	{
		nRc = IBSU_BGetInitProgress(devIndex, &IsComplete, &devHandle, &progressValue);
		if( nRc != IBSU_STATUS_OK || IsComplete )
			break;
		str.Format("Initializing device... %d%%", progressValue);
        m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_TXT_STATUS)->SetWindowTextA(str);
		Sleep(50);
	}
	str.Format("Initializing device... 100%%");
    m_pParent->m_ScanFingerDlg.GetDlgItem(IDC_TXT_STATUS)->SetWindowTextA(str);

    if( nRc >= IBSU_STATUS_OK )
    {
	    m_pParent->m_nDevHandle = devHandle;

        IBSU_GetOperableLEDs(devHandle, &m_pParent->m_LedType, &ledCount, &operableLEDs);

        disWnd = m_pParent->m_ScanFingerDlg.GetDlgItem( IDC_VIEW );

	    disWnd->GetClientRect( &clientRect );

	    IBSU_CreateClientWindow( devHandle, disWnd->m_hWnd, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);

	    ///For ResultImageEx
	    IBSU_AddOverlayQuadrangle(devHandle, &m_pParent->m_nOvClearPlatenHandle, 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);
	    IBSU_AddOverlayText(devHandle, &m_pParent->m_nOvImageTextHandle, "Arial", 10, TRUE/*bold*/, "", 10, 10, (DWORD)0);
	    IBSU_AddOverlayText(devHandle, &m_pParent->m_nOvCaptureTimeTextHandle, "Arial", 10, TRUE/*bold*/, "", 10, 10, (DWORD)0);
	    for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
		    IBSU_AddOverlayQuadrangle(devHandle, &m_pParent->m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 0, (DWORD)0);
	    for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
		    IBSU_AddOverlayText(devHandle, &m_pParent->m_nOvQualityTextHandle[i], "Arial", 10, TRUE/*bold*/, "", 10, 10, (DWORD)0);
	    ///

		IBSU_SetClientDisplayProperty(devHandle, ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA, "TRUE");

	    // register callback functions
	    IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, m_pParent->OnEvent_DeviceCommunicationBreak, m_pParent );    
	    IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, m_pParent->OnEvent_PreviewImage, m_pParent );
	    IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, m_pParent->OnEvent_TakingAcquisition, m_pParent );  
	    IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, m_pParent->OnEvent_CompleteAcquisition, m_pParent );  
	    IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX, m_pParent->OnEvent_ResultImageEx, m_pParent );  
	    IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, m_pParent->OnEvent_FingerCount, m_pParent );  
	    IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY, m_pParent->OnEvent_FingerQuality, m_pParent );
	    IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_ESSENTIAL_EVENT_KEYBUTTON, m_pParent->OnEvent_PressedKeyButtons, m_pParent ); 
	    IBSU_RegisterCallbacks( devHandle, ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE, m_pParent->OnEvent_ClearPlatenAtCapture, m_pParent );  

        if( !m_pParent->m_bPreInitialization )
        {
            m_pParent->m_bPreInitialization = TRUE;
            m_pParent->PostMessage( WM_USER_CAPTURE_SEQ_START );
        }

		memset(m_pParent->m_ShiftedFlag, 0, sizeof(m_pParent->m_ShiftedFlag));
		m_pParent->_SetLEDs(m_pParent->m_nDevHandle, ENUM_IBSU_TYPE_NONE, NONE_FINGER, 0, FALSE, m_pParent->m_ShiftedFlag);
    }

    return 0;
}
