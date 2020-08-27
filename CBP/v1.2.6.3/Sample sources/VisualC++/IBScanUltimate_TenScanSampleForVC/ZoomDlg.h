#pragma once


// CZoomDlg dialog

class CZoomDlg : public CDialog
{
	DECLARE_DYNAMIC(CZoomDlg)

public:
	CZoomDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CZoomDlg();
	
	void DrawImage();
	void ChangeZoomWindow(int zoomratio);

	unsigned char	*m_InImg;
	unsigned char	*m_OutImg;
	BITMAPINFO		*m_Info;
	int				m_InImgWidth;
	int				m_InImgHeight;
	int				m_InImgSize;
	int				m_OutImgWidth;
	int				m_OutImgHeight;
	int				m_OutImgSize;
	int				m_ZoomRatio;
	int				m_ZoomStartX;
	int				m_ZoomStartY;
	CPoint			oldpoint;

// Dialog Data
	enum { IDD = IDD_ZOOM_DIALOG };

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
