#pragma once


// CClearWindow ��ȭ �����Դϴ�.

class CClearWindow : public CDialog
{
	DECLARE_DYNAMIC(CClearWindow)

public:
	CClearWindow(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CClearWindow();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG_CLEARWIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CString m_ClearTerm;
	afx_msg void OnEnChangeEditClearterm();
};
