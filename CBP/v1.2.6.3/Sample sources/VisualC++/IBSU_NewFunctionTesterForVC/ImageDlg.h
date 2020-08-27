#pragma once
#include "afxwin.h"


// CImageDlg dialog

class CCaptureFunctions;

class CImageDlg : public CDialog
{
	DECLARE_DYNAMIC(CImageDlg)

public:
	CImageDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CImageDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_IMAGE };


public: 
	CCaptureFunctions					*m_functionsDlg;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CStatic m_view;
	virtual BOOL OnInitDialog();
};
