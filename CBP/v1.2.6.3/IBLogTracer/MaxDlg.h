#pragma once


// CMaxDlg 대화 상자입니다.

class CMaxDlg : public CDialog
{
	DECLARE_DYNAMIC(CMaxDlg)

public:
	CMaxDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CMaxDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_MAX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
