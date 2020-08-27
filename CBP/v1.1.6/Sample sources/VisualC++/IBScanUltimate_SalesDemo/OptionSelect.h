#pragma once
#include "afxwin.h"


// OptionSelect ��ȭ �����Դϴ�.
class CIBScanUltimate_SalesDemoDlg;

class OptionSelect : public CDialog
{
	DECLARE_DYNAMIC(OptionSelect)

public:
	OptionSelect(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~OptionSelect();

	CIBScanUltimate_SalesDemoDlg *m_pParent;

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_Option };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()

public:

	BOOL m_chkUseBitmap;
	BOOL m_chkUseWsq;
	BOOL m_chkUseIso;

	
	BOOL m_chkUseNFIQ;
	afx_msg void OnBnClickedBtnAccept();
	afx_msg void OnBnClickedButton1();
	virtual BOOL OnInitDialog();
	
	int m_miniNfiq;
	int m_WetDLevel;
	int m_SpoofLevel;
	CListCtrl m_ListCtrl;
	BOOL m_chkRightThumb;
	BOOL m_chkRightIndex;
	BOOL m_chkRightMiddle;
	BOOL m_chkRightRing;
	BOOL m_chkRightLittle;
	BOOL m_chkLeftThumb;
	BOOL m_chkLeftIndex;
	BOOL m_chkLeftMiddle;
	BOOL m_chkLeftRing;
	BOOL m_chkLeftLittle;
	BOOL m_chkDuplication;
	BOOL m_SmearD;
	BOOL m_chkAutoCapture;
	BOOL m_chkViewNFIQ;
	BOOL m_chkViewNFIQ2;
	BOOL m_WetFingerD;
	CComboBox m_cboWetLevel;
	CComboBox m_cboSpoofLevel;
	BOOL m_chkSpoofD;
	afx_msg void OnCbnSelchangeComboWetLevel();
};
