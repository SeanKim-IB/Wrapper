#pragma once


// CMaxDlg ��ȭ �����Դϴ�.

class CMaxDlg : public CDialog
{
	DECLARE_DYNAMIC(CMaxDlg)

public:
	CMaxDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CMaxDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG_MAX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
