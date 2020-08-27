// EditProperty.cpp : implementation file
//

#include "stdafx.h"
#include "UsbFwWriter.h"
#include "EditProperty.h"
#include "UsbFwWriterDlg.h"


#include "IBScanUltimateApi.h"
#include "ReservedApi.h"


#define RESERVED_KEY_STRING		_T("ibkorea1120!")

// CEditProperty 대화 상자입니다.

IMPLEMENT_DYNAMIC(CEditProperty, CDialog)

CEditProperty::CEditProperty(CWnd* pParent /*=NULL*/)
	: CDialog(CEditProperty::IDD, pParent)
{
	m_nDevHandle = -1;
}

CEditProperty::~CEditProperty()
{
}

void CEditProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_VID, m_editVID);
	DDX_Control(pDX, IDC_EDIT_PRODUCTION_DATE, m_editProDate);
	DDX_Control(pDX, IDC_EDIT_SERVICE_DATE, m_editSerDate);
	DDX_Control(pDX, IDC_BTN_SET_PROPERTY, m_btnSetProperty);
	DDX_Control(pDX, IDC_BTN_CANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CEditProperty, CDialog)
	ON_BN_CLICKED(IDC_BTN_SET_PROPERTY, &CEditProperty::OnBnClickedBtnSetProperty)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CEditProperty::OnBnClickedBtnCancel)
END_MESSAGE_MAP()


// CEditProperty 메시지 처리기입니다.

void CEditProperty::SetParentDlg(CUsbFwWriterDlg *pDlg)
{
	m_pParent = pDlg;
}

void CEditProperty::SetDeviceHandle(int devHandle)
{
	m_nDevHandle = devHandle;
}

void CEditProperty::_GetDeviceProperty()
{
	if( m_nDevHandle < 0 )
		return;

	IBSU_GetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_PRODUCT_ID, m_propertyInfo.cProductID);
	IBSU_GetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_SERIAL_NUMBER, m_propertyInfo.cSerialNumber);
	IBSU_GetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_VENDOR_ID, m_propertyInfo.cVendorID);
	IBSU_GetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_IBIA_VENDOR_ID, m_propertyInfo.cIBIA_VendorID);
	IBSU_GetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_IBIA_VERSION, m_propertyInfo.cIBIA_Version);
	IBSU_GetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_IBIA_DEVICE_ID, m_propertyInfo.cIBIA_DeviceID);
	IBSU_GetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_FIRMWARE, m_propertyInfo.cFirmware);
	IBSU_GetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_REVISION, m_propertyInfo.cDevRevision);
	IBSU_GetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_PRODUCTION_DATE, m_propertyInfo.cProductionDate);
	IBSU_GetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_SERVICE_DATE, m_propertyInfo.cServiceDate);
	IBSU_GetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_RESERVED_1, m_propertyInfo.cReserved_1);


	SetDlgItemText(IDC_EDIT_PRODUCT_ID, m_propertyInfo.cProductID);
	SetDlgItemText(IDC_EDIT_SERIAL_NUMBER, m_propertyInfo.cSerialNumber);
	SetDlgItemText(IDC_EDIT_IBIA_VENDOR_ID, m_propertyInfo.cIBIA_VendorID);
	SetDlgItemText(IDC_EDIT_IBIA_VERSION, m_propertyInfo.cIBIA_Version);
	SetDlgItemText(IDC_EDIT_IBIA_DEVICE_ID, m_propertyInfo.cIBIA_DeviceID);
	SetDlgItemText(IDC_EDIT_FIRMWARE, m_propertyInfo.cFirmware);
	SetDlgItemText(IDC_EDIT_REVISION, m_propertyInfo.cDevRevision);
	SetDlgItemText(IDC_EDIT_PRODUCTION_DATE, m_propertyInfo.cProductionDate);
	SetDlgItemText(IDC_EDIT_SERVICE_DATE, m_propertyInfo.cServiceDate);
	SetDlgItemText(IDC_EDIT_RESERVED_1, m_propertyInfo.cReserved_1);
}

void CEditProperty::_SetDeviceProperty()
{
	if( m_nDevHandle < 0 )
		return;

	int		nRc;
	int		ErrorCnt = 0;
	CString temp;

//	GetDlgItem(IDC_EDIT_PRODUCT_ID)->GetWindowText(temp);
//	strcpy(m_propertyInfo.cProductID, temp.GetBuffer(0));

	GetDlgItem(IDC_EDIT_SERIAL_NUMBER)->GetWindowText(temp);
	strcpy(m_propertyInfo.cSerialNumber, temp.GetBuffer(0));

//	GetDlgItem(IDC_EDIT_IBIA_VENDOR_ID)->GetWindowText(temp);
//	strcpy(m_propertyInfo.cIBIA_VendorID, temp.GetBuffer(0));

//	GetDlgItem(IDC_EDIT_IBIA_VERSION)->GetWindowText(temp);
//	strcpy(m_propertyInfo.cIBIA_Version, temp.GetBuffer(0));

//	GetDlgItem(IDC_EDIT_IBIA_DEVICE_ID)->GetWindowText(temp);
//	strcpy(m_propertyInfo.cIBIA_DeviceID, temp.GetBuffer(0));

//	GetDlgItem(IDC_EDIT_FIRMWARE)->GetWindowText(temp);
//	strcpy(m_propertyInfo.cFirmware, temp.GetBuffer(0));

//	GetDlgItem(IDC_EDIT_REVISION)->GetWindowText(temp);
//	strcpy(m_propertyInfo.cDevRevision, temp.GetBuffer(0));

	GetDlgItem(IDC_EDIT_PRODUCTION_DATE)->GetWindowText(temp);
	strcpy(m_propertyInfo.cProductionDate, temp.GetBuffer(0));

	GetDlgItem(IDC_EDIT_SERVICE_DATE)->GetWindowText(temp);
	strcpy(m_propertyInfo.cServiceDate, temp.GetBuffer(0));

	GetDlgItem(IDC_EDIT_RESERVED_1)->GetWindowText(temp);
	strcpy(m_propertyInfo.cReserved_1, temp.GetBuffer(0));


//	nRc = RESERVED_SetProperty(m_nDevHandle, RESERVED_KEY_STRING, ENUM_IBSU_PROPERTY_PRODUCT_ID, m_propertyInfo.cProductID);
	if( strlen(m_propertyInfo.cSerialNumber) == 0 &&
		strlen(m_propertyInfo.cProductionDate) == 0 &&
		strlen(m_propertyInfo.cServiceDate) == 0 &&
		strlen(m_propertyInfo.cReserved_1) == 0 )
	{
		AfxMessageBox("Please write property values!");
		return;
	}

	if( strlen(m_propertyInfo.cSerialNumber) > 0 )
	{

		nRc = RESERVED_SetProperty(m_nDevHandle, RESERVED_KEY_STRING, ENUM_IBSU_PROPERTY_SERIAL_NUMBER, m_propertyInfo.cSerialNumber);
		if( nRc < IBSU_STATUS_OK ) ErrorCnt++;
	}
//	nRc = RESERVED_SetProperty(m_nDevHandle, RESERVED_KEY_STRING, ENUM_IBSU_PROPERTY_VENDOR_ID, m_propertyInfo.cVendorID);
//	nRc = RESERVED_SetProperty(m_nDevHandle, RESERVED_KEY_STRING, ENUM_IBSU_PROPERTY_IBIA_VENDOR_ID, m_propertyInfo.cIBIA_VendorID);
//	nRc = RESERVED_SetProperty(m_nDevHandle, RESERVED_KEY_STRING, ENUM_IBSU_PROPERTY_IBIA_VERSION, m_propertyInfo.cIBIA_Version);
//	nRc = RESERVED_SetProperty(m_nDevHandle, RESERVED_KEY_STRING, ENUM_IBSU_PROPERTY_IBIA_DEVICE_ID, m_propertyInfo.cIBIA_DeviceID);
//	nRc = RESERVED_SetProperty(m_nDevHandle, RESERVED_KEY_STRING, ENUM_IBSU_PROPERTY_FIRMWARE, m_propertyInfo.cFirmware);
//	nRc = RESERVED_SetProperty(m_nDevHandle, RESERVED_KEY_STRING, ENUM_IBSU_PROPERTY_REVISION, m_propertyInfo.cDevRevision);
	if( strlen(m_propertyInfo.cProductionDate) > 0 )
	{
		nRc = RESERVED_SetProperty(m_nDevHandle, RESERVED_KEY_STRING, ENUM_IBSU_PROPERTY_PRODUCTION_DATE, m_propertyInfo.cProductionDate);
		if( nRc < IBSU_STATUS_OK ) ErrorCnt++;
	}
	if( strlen(m_propertyInfo.cServiceDate) > 0 )
	{
		nRc = RESERVED_SetProperty(m_nDevHandle, RESERVED_KEY_STRING, ENUM_IBSU_PROPERTY_SERVICE_DATE, m_propertyInfo.cServiceDate);
		if( nRc < IBSU_STATUS_OK ) ErrorCnt++;
	}
	if( strlen(m_propertyInfo.cReserved_1) > 0 )
	{
		nRc = RESERVED_SetProperty(m_nDevHandle, RESERVED_KEY_STRING, ENUM_IBSU_PROPERTY_RESERVED_1, m_propertyInfo.cReserved_1);
		if( nRc < IBSU_STATUS_OK ) ErrorCnt++;
	}

	if(	ErrorCnt > 0 )
	{
		CString msg;
		msg.Format("Set Property - %d times failed", ErrorCnt);
		AfxMessageBox(msg);
	}
	else
	{
		AfxMessageBox("Set Property sucessfully!");
	}
}

BOOL CEditProperty::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnSetProperty.SetIcon(IDI_ICON_OK, (int)BTNST_AUTO_GRAY);
	m_btnSetProperty.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 30);
	m_btnSetProperty.DrawTransparent(true);

	m_btnCancel.SetIcon(IDI_ICON_NO, (int)BTNST_AUTO_GRAY);
	m_btnCancel.OffsetColor(CButtonST::BTNST_COLOR_BK_IN, 30);
	m_btnCancel.DrawTransparent(true);

	_GetDeviceProperty();

	if( m_pParent->m_bIBKProductionMode )
	{
		GetDlgItem(IDC_EDIT_SERIAL_NUMBER)->EnableWindow( TRUE );
		GetDlgItem(IDC_EDIT_PRODUCTION_DATE)->EnableWindow( TRUE );
		GetDlgItem(IDC_EDIT_SERVICE_DATE)->EnableWindow( TRUE );
		GetDlgItem(IDC_EDIT_RESERVED_1)->EnableWindow( TRUE );

		GetDlgItem(IDC_BTN_SET_PROPERTY)->EnableWindow( TRUE );
	}
	else if( m_pParent->m_bIBPartnerMode )
	{
		GetDlgItem(IDC_EDIT_SERIAL_NUMBER)->EnableWindow( FALSE );
		GetDlgItem(IDC_EDIT_PRODUCTION_DATE)->EnableWindow( FALSE );
		GetDlgItem(IDC_EDIT_SERVICE_DATE)->EnableWindow( FALSE );
		GetDlgItem(IDC_EDIT_RESERVED_1)->EnableWindow( TRUE );

		GetDlgItem(IDC_BTN_SET_PROPERTY)->EnableWindow( TRUE );
	}
	else
	{
		GetDlgItem(IDC_EDIT_SERIAL_NUMBER)->EnableWindow( FALSE );
		GetDlgItem(IDC_EDIT_PRODUCTION_DATE)->EnableWindow( FALSE );
		GetDlgItem(IDC_EDIT_SERVICE_DATE)->EnableWindow( FALSE );
		GetDlgItem(IDC_EDIT_RESERVED_1)->EnableWindow( FALSE );

		GetDlgItem(IDC_BTN_SET_PROPERTY)->EnableWindow( FALSE );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CEditProperty::OnBnClickedBtnSetProperty()
{
	if( m_pParent->m_bIBKProductionMode || m_pParent->m_bIBPartnerMode )
	{
		_SetDeviceProperty();
	}

	OnOK();
}

void CEditProperty::OnBnClickedBtnCancel()
{
	OnCancel();
}
