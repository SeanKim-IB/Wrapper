#pragma once
//#include "UsbFwWriterDlg.h"
#include "afxwin.h"
#include "btnst.h"
#include "ShadeButtonST.h"
#include "IBScanUltimateApi_defs.h"

typedef struct tagPropertyInfo
{
	char			cProductID[IBSU_MAX_STR_LEN];
	char			cSerialNumber[IBSU_MAX_STR_LEN];
	char			cVendorID[IBSU_MAX_STR_LEN];
	char			cIBIA_VendorID[IBSU_MAX_STR_LEN];
	char			cIBIA_Version[IBSU_MAX_STR_LEN];
	char			cIBIA_DeviceID[IBSU_MAX_STR_LEN];
	char			cFirmware[IBSU_MAX_STR_LEN];
	char			cDevRevision[IBSU_MAX_STR_LEN];
	char			cProductionDate[IBSU_MAX_STR_LEN];
	char			cServiceDate[IBSU_MAX_STR_LEN];
	char			cReserved_1[IBSU_MAX_STR_LEN];
} PROPERTY_INFO;

// CEditProperty ��ȭ �����Դϴ�.

class CUsbFwWriterDlg;

class CEditProperty : public CDialog
{
	DECLARE_DYNAMIC(CEditProperty)

public:
	CEditProperty(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CEditProperty();
	
	void SetParentDlg(CUsbFwWriterDlg *pDlg);
	void SetDeviceHandle(int devHandle);
	void _GetDeviceProperty();
	void _SetDeviceProperty();
	

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG_EDITPRO };

	CUsbFwWriterDlg		*m_pParent;
	int					m_nDevHandle;
	PROPERTY_INFO		m_propertyInfo;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CEdit m_editVID;
	CEdit m_editRevison;
	CEdit m_editProDate;
	CEdit m_editSerDate;
	CButtonST m_btnSetProperty;
	CButtonST m_btnCancel;
	afx_msg void OnBnClickedBtnSetProperty();
	afx_msg void OnBnClickedBtnCancel();
};
