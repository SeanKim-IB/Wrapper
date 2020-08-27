// ZoomDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IBScanUltimate_TenScanSampleForVC.h"
#include "ZoomDlg.h"

#include "IBScanUltimateApi.h"

// CZoomDlg dialog
IMPLEMENT_DYNAMIC(CZoomDlg, CDialog)

CZoomDlg::CZoomDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CZoomDlg::IDD, pParent)
{
	m_InImg = new BYTE [IMG_SIZE];
	m_OutImg = new BYTE [IMG_SIZE*25];
		
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
	m_Info->bmiHeader.biHeight = IMG_H;
	m_Info->bmiHeader.biPlanes = 1;
	m_Info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_Info->bmiHeader.biSizeImage = IMG_SIZE;
	m_Info->bmiHeader.biWidth = IMG_W;
	m_Info->bmiHeader.biXPelsPerMeter = 19700;
	m_Info->bmiHeader.biYPelsPerMeter = 19700;

	m_ZoomRatio = 2;

	m_InImgWidth = IMG_W;
	m_InImgHeight = IMG_H;
	m_InImgSize = IMG_SIZE;
	m_OutImgWidth = IMG_W;
	m_OutImgHeight = IMG_H;
	m_OutImgSize = IMG_SIZE;

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
}


BEGIN_MESSAGE_MAP(CZoomDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


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

	SetWindowPos(NULL, 0, 0, IMG_W+8, IMG_H+36, SWP_NOMOVE);

	return TRUE;
}

void CZoomDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	DrawImage();
}

void CZoomDlg::DrawImage()
{
	CClientDC dc(this);
	CRect rect;
	GetClientRect(&rect);

	int startx_view, startx_img;
	int starty_view, starty_img;
	int view_width, view_height;
	int img_width, img_height;

	if(m_Info->bmiHeader.biWidth <= rect.Width())
	{
		startx_view = abs(m_Info->bmiHeader.biWidth-rect.Width())/2;
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
	::StretchDIBits(dc.m_hDC, startx_view, starty_view, view_width, view_height, startx_img, starty_img, img_width, img_height, 
						m_OutImg, m_Info, DIB_RGB_COLORS, SRCCOPY);
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
	int ref_x[IMG_W*6], ref_y[IMG_H*6];
	int p[IMG_W*6], q[IMG_H*6];
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
