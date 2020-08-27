
// FirmwareEditerDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include "btnst.h"
#define		STRINGLENGTH 32


typedef struct _PRODUCTPROPERTY{
	char product_ID[STRINGLENGTH];
	char firmware_version[STRINGLENGTH];
	char vender_ID[STRINGLENGTH];
	char revison[STRINGLENGTH];
	char reserve[STRINGLENGTH];
} PRODUCTPROPERTY;
// CFirmwareEditerDlg 대화 상자
class CFirmwareEditerDlg : public CDialog
{
// 생성입니다.
public:
	CFirmwareEditerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_FIRMWAREEDITER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;
	CString		m_filepath;
	CString		m_targetfilepath;
	BOOL		flag;
	void	Init_Layout();
	void		WriteFirmwareFile();

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_logo;
	CStatic m_staticPid;
	CStatic m_staticFirmware;
	CStatic m_staticRevison;
	CStatic m_staticReserve;
	CEdit m_editPid;
	CEdit m_editFirmware;
	CEdit m_editRevison;
	CEdit m_editReserve;
	CButtonST m_btnRead;
	CButtonST m_btnWrite;
	afx_msg void OnBnClickedButtonRead();
	afx_msg void OnBnClickedButtonWrite();
	CStatic m_staticVender;
	CEdit m_editVender;
	CComboBox m_cbPid;
};
