#pragma once


// CClearWindow 대화 상자입니다.

class CClearWindow : public CDialog
{
	DECLARE_DYNAMIC(CClearWindow)

public:
	CClearWindow(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CClearWindow();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_CLEARWIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CString m_ClearTerm;
	afx_msg void OnEnChangeEditClearterm();
};
