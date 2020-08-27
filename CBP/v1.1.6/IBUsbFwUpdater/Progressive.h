#pragma once
#include "afxcmn.h"
#include "btnst.h"
#include "ShadeButtonST.h"

// CProgressive 대화 상자입니다.

class CUsbFwWriterDlg;

class CProgressive : public CDialog
{
	DECLARE_DYNAMIC(CProgressive)

public:
	CProgressive(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CProgressive();

	CUsbFwWriterDlg *pParentWnd;

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_PRO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	//int		WriteFirmware();


	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CProgressCtrl m_progressiveWatson;
	CProgressCtrl m_progressiveTotal;
	int		WriteFirmware();
	int		m_chknumberofprogress;
	void	Test();
	void	InitStaticText();
	CFont*	GetFont(LONG IfWidth, LONG IfHeight, LONG IfWeight);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CButtonST m_OK;
	CButtonST m_Cancel;
//	virtual BOOL DestroyWindow();
};
