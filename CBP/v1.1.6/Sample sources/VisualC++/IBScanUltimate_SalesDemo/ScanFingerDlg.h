#pragma once
#include "afxwin.h"
#include "ColorButton.h"

class CIBScanUltimate_SalesDemoDlg;
class CFingerDisplayManager;

// CScanFingerDlg 대화 상자입니다.

class CScanFingerDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanFingerDlg)

public:
	CScanFingerDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CScanFingerDlg();


	void Init_Layout();
	void DrawImage();
	void UpdateFingerView();

	void BtnSingleLayout();				//10
	void BtnSingleRollLayout();			//10 roll
	void BtnTwoFingerLayout();			//5
	void BtnTwoFingerLayout_Each();		//6
	void Btn15Layout_Flat();			//15
	void Btn15Layout_Roll();			//15
	void Btn16Layout_Each_Roll();		//16
	void Btn17Layout_Each_Both_Roll();	//17
	void Btn1Layout();					//1
	void Btn2Layout();
	void Btn4Layout();
	void Btn3Layout();//442
	void Btn13Layout();//442androll
	void Btn5Layout();//4141
	void Btn14Layout();//4141androll

	void Disable1Finger();
	void Disable2Finger();
	void Disable3Finger();//442
	void Disable13Finger();//442androll
	void Disable4Finger();
	void Disable5Finger();
	void Disable6Finger();
	void Disable10Finger();
	void Disable15Finger();
	void Disable16Finger();
	void Disable17Finger();
	void Disable18Finger();//4141
	


	void blinkBtn();
	

// 대화 상자 데이터입니다.
	enum { IDD = IDD_SCAN_FINGER_DIALOG };

	CIBScanUltimate_SalesDemoDlg		*m_pParent;
	CFingerDisplayManager				*m_FingerDisplayManager;

protected:
	static const CBrush   s_brushRed;                   ///<  Red background brush for scanner
	static const CBrush   s_brushOrange;                ///<  Orange background brush for scanner
	static const CBrush   s_brushGreen;                 ///<  Green background brush for scanner
	static const CBrush   s_brushPlaten;				///<  Platen brush for scanner
	CBrush m_background;

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

public:
	BOOL	m_CaptureStart;
	BOOL	m_CaptureEnd;
	BOOL	m_StartFingerCapture;
	BOOL    m_SelectFinger;
	BOOL	m_CaptureisNotCompleted;


	CStatic m_FingerView;
	CStatic m_CapView;
	CColorCtrl<CBoldCtrl <CStatic> > m_strFingerStatus;

	afx_msg void OnBnClickedScan10Fingers();
	afx_msg void OnBnClickedClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CColorButton m_Button1;
	CColorButton m_Button2;
	CColorButton m_Button3;
	CColorButton m_Button4;
	CColorButton m_Button5;
	CColorButton m_Button6;
	CColorButton m_Button7;
	CColorButton m_Button8;
	CColorButton m_Button9;
	CColorButton m_Button10;
	CColorButton m_Button11;
	CColorButton m_Button12;
	CColorButton m_Button13;
	CColorButton m_Button14;
	CColorButton m_Button15;
	CColorButton m_Button16;
	CColorButton m_Button17;

	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton91();
	afx_msg void OnBnClickedButton92();
	afx_msg void OnBnClickedButton93();
	afx_msg void OnBnClickedButton94();
	afx_msg void OnBnClickedButton95();
	afx_msg void OnBnClickedButton96();
	afx_msg void OnBnClickedButton97();
	afx_msg void OnBnClickedButton98();
	CBitmapButton m_btnscan;
	CBitmapButton m_btnclose;

	afx_msg void OnBnClickedButton99();
	CColorButton m_Button18;
};
