#pragma once
#include "afxwin.h"
#include "ColorButton.h"

class CMainDlg;
// CZoomDlg 대화 상자입니다.

class CZoomDlg : public CDialog
{
	DECLARE_DYNAMIC(CZoomDlg)

public:
	CZoomDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CZoomDlg();

	void DrawImage();
	void ChangeZoomWindow(int zoomratio);
	void InitLayout();
	void Zoom_1_Img();
	void Zoom_3_Img();
	void Zoom_10_Img();
	void Zoom_14_Img();
	void Zoom_16_Img();
	void Zoom_19_Img();
	//Kojak
	void Zoom_13_Img();

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



// 대화 상자 데이터입니다.
	enum { IDD = IDD_ZOOM };

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	CStatic m_ZoomView;
	CMainDlg *m_MainDlg;
	

	CColorButton m_ZButton1;
	CColorButton m_ZButton2;
	CColorButton m_ZButton3;
	CColorButton m_ZButton4;
	CColorButton m_ZButton5;
	CColorButton m_ZButton6;
	CColorButton m_ZButton7;
	CColorButton m_ZButton8;
	CColorButton m_ZButton9;
	CColorButton m_ZButton10;
	CColorButton m_ZButton11;
	CColorButton m_ZButton12;
	CColorButton m_ZButton13;
	CColorButton m_ZButton14;
	CColorButton m_ZButton15;
	CColorButton m_ZButton16;
	CColorButton m_ZButton17;
	CColorButton m_ZButton18;
	CColorButton m_ZButton19;
	afx_msg void OnBnClickedZoomBtn1();
	afx_msg void OnBnClickedZoomBtn2();
	afx_msg void OnBnClickedZoomBtn3();
	afx_msg void OnBnClickedZoomBtn4();
	afx_msg void OnBnClickedZoomBtn5();
	afx_msg void OnBnClickedZoomBtn6();
	afx_msg void OnBnClickedZoomBtn7();
	afx_msg void OnBnClickedZoomBtn8();
	afx_msg void OnBnClickedZoomBtn9();
	afx_msg void OnBnClickedZoomBtn10();
	afx_msg void OnBnClickedZoomBtn11();
	afx_msg void OnBnClickedZoomBtn12();
	afx_msg void OnBnClickedZoomBtn13();
	afx_msg void OnBnClickedZoomBtn14();
	afx_msg void OnBnClickedZoomBtn15();
	afx_msg void OnBnClickedZoomBtn16();
	afx_msg void OnBnClickedZoomBtn17();
	afx_msg void OnBnClickedZoomBtn18();
	afx_msg void OnBnClickedZoomBtn19();
};
