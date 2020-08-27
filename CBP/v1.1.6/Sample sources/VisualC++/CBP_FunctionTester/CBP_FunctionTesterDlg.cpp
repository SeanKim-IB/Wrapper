
// CBP_FunctionTesterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CBP_FunctionTester.h"
#include "CBP_FunctionTesterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCBP_FunctionTesterDlg dialog

CCBP_FunctionTesterDlg *m_app;


CCBP_FunctionTesterDlg::CCBP_FunctionTesterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCBP_FunctionTesterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_app = this;

	m_Image = new BYTE [2048*2048];
	memset(m_Image, 255, 2048*2048);
	m_disp_buf = new BYTE [2048*2048];
	memset(m_disp_buf, 255, 2048*2048);

	m_ImageInfo = (BITMAPINFO*) new char [1064];
	for(int i=0; i<256; i++)
	{
		m_ImageInfo->bmiColors[i].rgbBlue = 
		m_ImageInfo->bmiColors[i].rgbRed = 
		m_ImageInfo->bmiColors[i].rgbGreen = i;
		m_ImageInfo->bmiColors[i].rgbReserved = 0;
	}
	m_ImageInfo->bmiHeader.biBitCount = 8;
	m_ImageInfo->bmiHeader.biClrImportant = 0;
	m_ImageInfo->bmiHeader.biClrUsed = 0;
	m_ImageInfo->bmiHeader.biCompression = BI_RGB;
	m_ImageInfo->bmiHeader.biHeight = -375;
	m_ImageInfo->bmiHeader.biPlanes = 1;
	m_ImageInfo->bmiHeader.biSize = 40;
	m_ImageInfo->bmiHeader.biSizeImage = 400*375;
	m_ImageInfo->bmiHeader.biWidth = 400;
	m_ImageInfo->bmiHeader.biXPelsPerMeter = 0;
	m_ImageInfo->bmiHeader.biYPelsPerMeter = 0;

	m_ImageWidth = 0;
	m_ImageHeight = 0;
	m_DispWidth = 400;
	m_DispHeight = 375;

	m_OpenMode = 3;

	memset(&m_PreviewAnalysis, 0, sizeof(cbp_fp_previewAnalysis));
}

CCBP_FunctionTesterDlg::~CCBP_FunctionTesterDlg()
{
	delete [] m_Image;
	delete [] m_disp_buf;
	delete m_ImageInfo;
}

void CCBP_FunctionTesterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DEVICES, m_cboDevices);
	DDX_Control(pDX, IDC_LIST_LOG, m_ListLog);
	DDX_Control(pDX, IDC_STATIC_VIEW, m_DisplayView);
	DDX_Control(pDX, IDC_COMBO_PROPERTY, m_cboProperty);
	DDX_Control(pDX, IDC_COMBO_SEQUENCE, m_cboSequence);
	DDX_Control(pDX, IDC_COMBO_COLLECT_TYPE, m_cboCollectType);
}

BEGIN_MESSAGE_MAP(CCBP_FunctionTesterDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_INITIALIZE, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpInitialize)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_ENUMDEVICES, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpEnumdevices)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_OPEN, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpOpen)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_LOCK, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpLock)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_REGISTER_CALLBACKS, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpRegisterCallbacks)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_CALIBRATE, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpCalibrate)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_GET_LOCKINFO, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpGetLockinfo)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_START_IMAGING, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpStartImaging)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_CAPTURE, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpCapture)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_UNLOCK, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpUnlock)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_CLOSE, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpClose)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_UNINITIALIZE, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpUninitialize)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_STOP_IMAGING, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpStopImaging)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_CONFIGURE, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpConfigure)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_GET_DIRTINESS, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpGetDirtiness)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_GET_PROPERTY, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpGetProperty)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_SET_PROPERTY, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpSetProperty)
	ON_BN_CLICKED(IDC_BUTTON_CBP_FP_POWER_SAVE, &CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpPowerSave)
	ON_BN_CLICKED(IDC_RADIO_MODE1, &CCBP_FunctionTesterDlg::OnBnClickedRadioMode1)
	ON_BN_CLICKED(IDC_RADIO_MODE2, &CCBP_FunctionTesterDlg::OnBnClickedRadioMode2)
	ON_BN_CLICKED(IDC_RADIO_MODE3, &CCBP_FunctionTesterDlg::OnBnClickedRadioMode3)
	ON_WM_CLOSE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_MENU_SAVE_IMAGE, &CCBP_FunctionTesterDlg::OnMenuSaveImage)
END_MESSAGE_MAP()


// CCBP_FunctionTesterDlg message handlers

BOOL CCBP_FunctionTesterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_callbacklist.cbp_fp_onCalibrate = &OnEvent_cbp_fp_onCalibrate;
	m_callbacklist.cbp_fp_onCapture = &OnEvent_cbp_fp_onCapture;
	m_callbacklist.cbp_fp_onClose = &OnEvent_cbp_fp_onClose;
	m_callbacklist.cbp_fp_onConfigure = &OnEvent_cbp_fp_onConfigure;
	m_callbacklist.cbp_fp_onEnumDevices = &OnEvent_cbp_fp_onEnumDevices;
	m_callbacklist.cbp_fp_onError = &OnEvent_cbp_fp_onError;
	m_callbacklist.cbp_fp_onGetDirtiness = &OnEvent_cbp_fp_onGetDirtiness;
	m_callbacklist.cbp_fp_onGetLockInfo = &OnEvent_cbp_fp_onGetLockInfo;
	m_callbacklist.cbp_fp_onInitialize = &OnEvent_cbp_fp_onInitialize;
	m_callbacklist.cbp_fp_onLock = &OnEvent_cbp_fp_onLock;
	m_callbacklist.cbp_fp_onOpen = &OnEvent_cbp_fp_onOpen;
	m_callbacklist.cbp_fp_onPowerSave = &OnEvent_cbp_fp_onPowerSave;
	m_callbacklist.cbp_fp_onPreview = &OnEvent_cbp_fp_onPreview;
	m_callbacklist.cbp_fp_onPreviewAnalysis = &OnEvent_cbp_fp_onPreviewAnalysis;
	m_callbacklist.cbp_fp_onStopPreview = &OnEvent_cbp_fp_onStopPreview;
	m_callbacklist.cbp_fp_onUninitialize = &OnEvent_cbp_fp_onUninitialize;
	m_callbacklist.cbp_fp_onUnLock = &OnEvent_cbp_fp_onUnLock;
	m_callbacklist.cbp_fp_onWarning = &OnEvent_cbp_fp_onWarning;

	m_cboDevices.ResetContent();
	m_ListLog.DeleteAllItems();

	CRect rect;
	m_ListLog.GetClientRect(&rect);
	m_ListLog.InsertColumn(0, _T(""), LVCFMT_LEFT, 0);
	m_ListLog.InsertColumn(1, _T("Logs"), LVCFMT_LEFT, 400-10);

	m_cboProperty.InsertString(0, "supportedCollectionTypes");
	m_cboProperty.InsertString(1, "supportedSlaps");
	m_cboProperty.InsertString(2, "make");
	m_cboProperty.InsertString(3, "model");
	m_cboProperty.InsertString(4, "serial");
	m_cboProperty.InsertString(5, "softwareVersion");
	m_cboProperty.InsertString(6, "firmwareVersion");
	m_cboProperty.InsertString(7, "previewPixelHeight");
	m_cboProperty.InsertString(8, "previewPixelWidth");
	m_cboProperty.InsertString(9, "capturePixelHeight");
	m_cboProperty.InsertString(10, "capturePixelWidth");
	m_cboProperty.InsertString(11, "previewDPI");
	m_cboProperty.InsertString(12, "captureDPI");
	m_cboProperty.InsertString(13, "debugLevel");
	m_cboProperty.InsertString(14, "debugFile");
	m_cboProperty.SetCurSel(0);

	m_cboSequence.InsertString(0, "slap_rightHand");
	m_cboSequence.InsertString(1, "slap_leftHand");
	m_cboSequence.InsertString(2, "slap_twoFingers");
	m_cboSequence.InsertString(3, "slap_twoThumbs");
	m_cboSequence.InsertString(4, "slap_rightThumb");
	m_cboSequence.InsertString(5, "slap_rightIndex");
	m_cboSequence.InsertString(6, "slap_rightMiddle");
	m_cboSequence.InsertString(7, "slap_rightRing");
	m_cboSequence.InsertString(8, "slap_rightLittle");
	m_cboSequence.InsertString(9, "slap_leftThumb");
	m_cboSequence.InsertString(10, "slap_leftIndex");
	m_cboSequence.InsertString(11, "slap_leftMiddle");
	m_cboSequence.InsertString(12, "slap_leftRing");
	m_cboSequence.InsertString(13, "slap_leftLittle");
	m_cboSequence.InsertString(14, "slap_twotThumbs");
	m_cboSequence.InsertString(15, "slap_stitchedLeftThumb");
	m_cboSequence.InsertString(16, "slap_stitchedRightThumb");
	m_cboSequence.InsertString(17, "slap_unknown");
	m_cboSequence.SetCurSel(0);

	m_cboCollectType.InsertString(0, "collection_rolled");
	m_cboCollectType.InsertString(1, "collection_flat");
	m_cboCollectType.InsertString(2, "collection_contactless");
	m_cboCollectType.InsertString(3, "collection_unknown");
	m_cboCollectType.SetCurSel(1);

	if(m_OpenMode == 1)
		((CButton*)GetDlgItem(IDC_RADIO_MODE1))->SetCheck(TRUE);
	else if(m_OpenMode == 2)
		((CButton*)GetDlgItem(IDC_RADIO_MODE2))->SetCheck(TRUE);
	else if(m_OpenMode == 3)
		((CButton*)GetDlgItem(IDC_RADIO_MODE3))->SetCheck(TRUE);

	m_DisplayView.SetWindowPos(NULL, 0, 0, 400, 375, SWP_NOMOVE);
	m_ListLog.SetWindowPos(NULL, 0, 0, 400, 180, SWP_NOMOVE);

	SetWindowPos(NULL, 0, 0, 850, 700, SWP_NOMOVE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCBP_FunctionTesterDlg::_ADD_LOG(CString log, BOOL newStart)
{
	if(newStart == TRUE)
		m_ListLog.DeleteAllItems();

	int idx = m_ListLog.GetItemCount();

	m_ListLog.InsertItem(idx, "");
	m_ListLog.SetItemText(idx, 1, log);

	CSize size;
    size.cy=(m_ListLog.GetItemCount()-1)*12;
    size.cx=0;

    if(m_ListLog.Scroll(size))
    {
       m_ListLog.SetItemState(m_ListLog.GetItemCount()-1, LVIS_SELECTED,LVIS_SELECTED);
       m_ListLog.SetFocus();
    }
}

void CCBP_FunctionTesterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();

		_DisplayImage();
	}
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onCalibrate(int handle)
{
	CString remark;
	remark.Format(">>> cbp_fp_onCalibrate() - Successfully calibrated the device (%d)", handle);
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onCapture(int handle, struct cbp_fp_grayScaleCapture *grayScaleCapture)
{
	CString remark;
	remark.Format(">>> cbp_fp_onCapture() - successfully captured the final slap iamge (%d)", handle);
	m_app->_ADD_LOG(remark);
	remark.Format(">>> cbp_fp_onCapture() - final captured image dimension %d x %d", grayScaleCapture->width, grayScaleCapture->height);
	m_app->_ADD_LOG(remark);

	memcpy(m_app->m_Image, grayScaleCapture->image, grayScaleCapture->width*grayScaleCapture->height);
	m_app->m_ImageWidth = grayScaleCapture->width;
	m_app->m_ImageHeight = grayScaleCapture->height;

	//m_app->m_ImageInfo->bmiHeader.biWidth = grayScaleCapture->width;
	//m_app->m_ImageInfo->bmiHeader.biHeight = grayScaleCapture->height;
	//m_app->m_ImageInfo->bmiHeader.biSizeImage = grayScaleCapture->width*grayScaleCapture->height;

	//m_app->_SaveBitmapImage("d:\\final.bmp", m_app->m_Image, m_app->m_ImageWidth, m_app->m_ImageHeight, FALSE);

	m_app->_DisplayImage();
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onClose(int handle)
{
	CString remark;
	remark.Format(">>> cbp_fp_onClose - Successfully closed the device (%d)", handle);
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onConfigure(int handle, bool configurationChange)
{
	CString remark;
	remark.Format(">>> cbp_fp_onConfigure() Successfully configured the device (%d)", handle);
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onEnumDevices(struct cbp_fp_device *deviceList[], int deviceListLen, char *requestID)
{
	CString remark;
	remark.Format(">>> cbp_fp_OnEnumDevices() - Detected %d device(s) connected to the workstation", deviceListLen);
	m_app->_ADD_LOG(remark);
	
	CString str;

	m_app->m_cboDevices.ResetContent();
	m_app->m_cbp_devices.clear();
	for(int i=0; i<deviceListLen; i++)
	{
		str.Format("%s (%s) - %s\n", deviceList[i]->model, deviceList[i]->serialNumber, deviceList[i]->make);
		m_app->m_cboDevices.InsertString(i, str);
		
		cbp_fp_device device;
		sprintf(device.make, deviceList[i]->make);
		sprintf(device.model, deviceList[i]->model);
		sprintf(device.serialNumber, deviceList[i]->serialNumber);
		m_app->m_cbp_devices.push_back(device);
	}

	if(deviceListLen > 0)
	{
		m_app->m_cboDevices.SetCurSel(0);
	}

}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onError(int errorNumber, char *errorDescription)
{
	CString remark;
	remark.Format(">>> cbp_fp_onError (%d) %s", errorNumber, errorDescription);
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onGetDirtiness(int handle , int dirtinessLevel)
{
	CString remark;
	remark.Format(">>> cbp_fp_onGetDirtiness() - Dirtiness level %d", dirtinessLevel);
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onGetLockInfo(int handle, struct cbp_fp_lock_info *lockInfo)
{
	CString remark;
	remark.Format(">>> cbp_fp_onGetLockInfo() handle %d, PID %d, duration %d", lockInfo->handle, lockInfo->lockPID, lockInfo->lockDuration);
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onInitialize(char *requestID)
{
	CString remark;
	remark.Format(">>> cbp_fp_onInitialize() - Successfully initialized the device");
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onLock(int handle, int processID)
{
	CString remark;
	remark.Format(">>> cbp_fp_onLock() - Successfully locked the device. Handle %d PID %d\n", handle, processID);
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onOpen(int handle, struct cbp_fp_scanner_connection *scanner_connection)
{
	CString remark;
	remark.Format(">>> cbp_fp_onOpen() - Successfully opened the device");
	m_app->_ADD_LOG(remark);

	m_app->m_devHandle = handle;
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onPowerSave(int  handle, bool isOnPowerSave)
{
	CString remark;
	remark.Format(">>> cbp_fp_onPowerSave() - Successfully turned the power save mode on");
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onPreview(int handle, struct cbp_fp_grayScalePreview *preview)
{
	CString remark;
	remark.Format(">>> cbp_fp_onPreview()...");
	m_app->_ADD_LOG(remark);

	memcpy(m_app->m_Image, preview->image, preview->width*preview->height);
	m_app->m_ImageWidth = preview->width;
	m_app->m_ImageHeight = preview->height;

	m_app->_DisplayImage();
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onPreviewAnalysis(int handle, struct cbp_fp_previewAnalysis *previewAnalysis)
{
	memcpy(&m_app->m_PreviewAnalysis, previewAnalysis, sizeof(cbp_fp_previewAnalysis));
	
	struct cbp_fp_fingerAttributes fingerAttributes;
	char codeString[100] = { 0 };
	char codeString_slap[100] = { 0 };
	char codeString_collection[100] = { 0 };

	m_app->GetSlapTypeCodeString(previewAnalysis->slapType, codeString_slap);
	m_app->GetCollectionTypeCodeString(previewAnalysis->collectionType, codeString_collection);

	CString remark;
	remark.Format(">>> cbp_fp_onPreviewAnalysis() - Vendor analysis data:");
	m_app->_ADD_LOG(remark);
	remark.Format("    CollectionType: %d ( %s )", previewAnalysis->collectionType, codeString_collection);
	m_app->_ADD_LOG(remark);
	remark.Format("    SlapType: %d ( %s )", previewAnalysis->slapType, codeString_slap);
	m_app->_ADD_LOG(remark);

	for (int i = 0; i < previewAnalysis->fingerAttributeListLen; i++)
	{
		fingerAttributes = previewAnalysis->fingerAttributeList[i];
		for(int j=0; j<fingerAttributes.analysisCodeListLen; j++)
		{
			m_app->GetAnalysisCodeString(fingerAttributes.analysisCodeList[j], codeString);
			remark.Format("    FINGER [%d][%d] %s, score %d\n", i, j, codeString, fingerAttributes.score);
			m_app->_ADD_LOG(remark);
		}
	}

	m_app->_DisplayImage();
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onStopPreview(int handle)
{
	m_app->endtime = clock();

	CString remark;
	remark.Format(">>> cbp_fp_onStopPreview() - Successfully interrupt the previewing process");
	m_app->_ADD_LOG(remark);
	remark.Format(">>> cbp_fp_onStopPreview() - elapsed time (%.1f ms)", m_app->endtime - m_app->starttime);
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onUninitialize(char *requestID)
{
	CString remark;
	remark.Format(">>> cbp_fp_onUninitialize() - Successfully uninitialized the device");
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onUnLock(int handle)
{
	CString remark;
	remark.Format(">>> cbp_fp_onUnLock() - Successfully unlocked the device. Handle %d", handle);
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnEvent_cbp_fp_onWarning(int warningNumber, char *warningDescription)
{
	CString remark;
	remark.Format(">>> cbp_fp_onWarning (%d) %s", warningNumber, warningDescription);
	m_app->_ADD_LOG(remark);
}

///////////////////////////////////////////////////////////////////////////////////////


// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCBP_FunctionTesterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpInitialize()
{
	cbp_fp_initialize("Integrated Biometrics");
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpEnumdevices()
{
	cbp_fp_enumDevices("Integrated Biometrics");
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpOpen()
{
	if(m_OpenMode == 1)
	{
		cbp_fp_open(NULL);
		return;
	}

	if(m_cbp_devices.size() == 0)
	{
		m_app->_ADD_LOG(">>> no devices.");
		return;
	}

	int pos = m_cboDevices.GetCurSel();

	cbp_fp_device device = m_cbp_devices.at(pos);

	if(m_OpenMode == 2)
	{
		sprintf(device.serialNumber, "");
		cbp_fp_open(&device);
	}
	else 
		cbp_fp_open(&device);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpRegisterCallbacks()
{
	int nRc = cbp_fp_registerCallBacks(&m_callbacklist);

	CString remark;
	remark.Format(">>> cbp_fp_registerCallBacks");
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpLock()
{
	cbp_fp_lock(m_devHandle);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpCalibrate()
{
	cbp_fp_calibrate(m_devHandle);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpGetLockinfo()
{
	cbp_fp_getLockInfo(m_devHandle);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpStartImaging()
{
	cbp_fp_slapType slap_type;
	cbp_fp_collectionType collect_type;
	char pString[256];
	
	m_cboSequence.GetLBText(m_cboSequence.GetCurSel(), pString);

	if( strcmp(pString, "slap_rightHand") == 0)					{ slap_type = slap_rightHand; }
	else if( strcmp(pString, "slap_leftHand") == 0)				{ slap_type = slap_leftHand; }
	else if( strcmp(pString, "slap_twoFingers") == 0)			{ slap_type = slap_twoFingers; }
	else if( strcmp(pString, "slap_twoThumbs") == 0)			{ slap_type = slap_twoThumbs; }
	else if( strcmp(pString, "slap_rightThumb") == 0)			{ slap_type = slap_rightThumb; }
	else if( strcmp(pString, "slap_rightIndex") == 0)			{ slap_type = slap_rightIndex; }
	else if( strcmp(pString, "slap_rightMiddle") == 0)			{ slap_type = slap_rightMiddle; }
	else if( strcmp(pString, "slap_rightRing") == 0)			{ slap_type = slap_rightRing; }
	else if( strcmp(pString, "slap_rightLittle") == 0)			{ slap_type = slap_rightLittle; }
	else if( strcmp(pString, "slap_leftThumb") == 0)			{ slap_type = slap_leftThumb; }
	else if( strcmp(pString, "slap_leftIndex") == 0)			{ slap_type = slap_leftIndex; }
	else if( strcmp(pString, "slap_leftMiddle") == 0)			{ slap_type = slap_leftMiddle; }
	else if( strcmp(pString, "slap_leftRing") == 0)				{ slap_type = slap_leftRing; }
	else if( strcmp(pString, "slap_leftLittle") == 0)			{ slap_type = slap_leftLittle; }
	else if( strcmp(pString, "slap_twotThumbs") == 0)			{ slap_type = slap_twotThumbs; }
	else if( strcmp(pString, "slap_stitchedLeftThumb") == 0)	{ slap_type = slap_stitchedLeftThumb; }
	else if( strcmp(pString, "slap_stitchedRightThumb") == 0)	{ slap_type = slap_stitchedRightThumb; }
	else if( strcmp(pString, "slap_unknown") == 0)				{ slap_type = slap_unknown; }
	else return;

	m_cboCollectType.GetLBText(m_cboCollectType.GetCurSel(), pString);

	if( strcmp(pString, "collection_rolled") == 0)				{ collect_type = collection_rolled; }
	else if( strcmp(pString, "collection_flat") == 0)			{ collect_type = collection_flat; }
	else if( strcmp(pString, "collection_contactless") == 0)	{ collect_type = collection_contactless; }
	else if( strcmp(pString, "collection_unknown") == 0)		{ collect_type = collection_unknown; }
	else return;
	
	cbp_fp_startImaging(m_devHandle, slap_type, collect_type);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpStopImaging()
{
	starttime = clock();

	cbp_fp_stopImaging(m_devHandle);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpCapture()
{
	cbp_fp_slapType slap_type;
	cbp_fp_collectionType collect_type;
	char pString[256];
	
	m_cboSequence.GetLBText(m_cboSequence.GetCurSel(), pString);

	if( strcmp(pString, "slap_rightHand") == 0)					{ slap_type = slap_rightHand; }
	else if( strcmp(pString, "slap_leftHand") == 0)				{ slap_type = slap_leftHand; }
	else if( strcmp(pString, "slap_twoFingers") == 0)			{ slap_type = slap_twoFingers; }
	else if( strcmp(pString, "slap_twoThumbs") == 0)			{ slap_type = slap_twoThumbs; }
	else if( strcmp(pString, "slap_rightThumb") == 0)			{ slap_type = slap_rightThumb; }
	else if( strcmp(pString, "slap_rightIndex") == 0)			{ slap_type = slap_rightIndex; }
	else if( strcmp(pString, "slap_rightMiddle") == 0)			{ slap_type = slap_rightMiddle; }
	else if( strcmp(pString, "slap_rightRing") == 0)			{ slap_type = slap_rightRing; }
	else if( strcmp(pString, "slap_rightLittle") == 0)			{ slap_type = slap_rightLittle; }
	else if( strcmp(pString, "slap_leftThumb") == 0)			{ slap_type = slap_leftThumb; }
	else if( strcmp(pString, "slap_leftIndex") == 0)			{ slap_type = slap_leftIndex; }
	else if( strcmp(pString, "slap_leftMiddle") == 0)			{ slap_type = slap_leftMiddle; }
	else if( strcmp(pString, "slap_leftRing") == 0)				{ slap_type = slap_leftRing; }
	else if( strcmp(pString, "slap_leftLittle") == 0)			{ slap_type = slap_leftLittle; }
	else if( strcmp(pString, "slap_twotThumbs") == 0)			{ slap_type = slap_twotThumbs; }
	else if( strcmp(pString, "slap_stitchedLeftThumb") == 0)	{ slap_type = slap_stitchedLeftThumb; }
	else if( strcmp(pString, "slap_stitchedRightThumb") == 0)	{ slap_type = slap_stitchedRightThumb; }
	else if( strcmp(pString, "slap_unknown") == 0)				{ slap_type = slap_unknown; }
	else return;

	m_cboCollectType.GetLBText(m_cboCollectType.GetCurSel(), pString);

	if( strcmp(pString, "collection_rolled") == 0)				{ collect_type = collection_rolled; }
	else if( strcmp(pString, "collection_flat") == 0)			{ collect_type = collection_flat; }
	else if( strcmp(pString, "collection_contactless") == 0)	{ collect_type = collection_contactless; }
	else if( strcmp(pString, "collection_unknown") == 0)		{ collect_type = collection_unknown; }
	else return;

	cbp_fp_capture(m_devHandle, slap_type, collect_type);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpUnlock()
{
	cbp_fp_unlock(m_devHandle);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpClose()
{
	cbp_fp_close(m_devHandle);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpUninitialize()
{
	cbp_fp_uninitialize("Integrated Biometrics");
}

void CCBP_FunctionTesterDlg::_DisplayImage()
{
	int i, j, ii, jj;

	for(i=0; i<m_DispHeight; i++)
	{
		ii = i * m_ImageHeight / m_DispHeight;
		for(j=0; j<m_DispWidth; j++)
		{
			jj = j * m_ImageWidth / m_DispWidth;

			m_disp_buf[i*m_DispWidth+j] = m_Image[ii*m_ImageWidth+jj];
		}
	}
	
	CClientDC dc(&m_DisplayView);

	CDC MemDC;
	MemDC.CreateCompatibleDC(&dc);
	CBitmap bmpMem, *pOldMem;
	bmpMem.CreateCompatibleBitmap(&dc, m_DispWidth, m_DispHeight);
	pOldMem = (CBitmap *)MemDC.SelectObject(&bmpMem); 

	::StretchDIBits(MemDC.m_hDC, 0, 0, m_DispWidth, m_DispHeight, 0, 0, m_DispWidth, m_DispHeight, 
		m_disp_buf, m_ImageInfo, DIB_RGB_COLORS, SRCCOPY);

	CPen pen, *oldpen;
	pen.CreatePen(PS_SOLID, 1, RGB(0, 200, 0));
	oldpen = MemDC.SelectObject(&pen);

	MemDC.SelectObject(GetStockObject(NULL_BRUSH));
	MemDC.SetBkMode(TRANSPARENT);
/*
	int x, y, w, h;
	CString str;
	for(int i=0; i<m_PreviewAnalysis.fingerAttributeListLen; i++)
	{
		CPen pen;
		if(m_PreviewAnalysis.fingerAttributeList[i].analysisCodeListLen > 1)
			pen.CreatePen(PS_SOLID, 1, RGB(200, 200, 50));
		else if(m_PreviewAnalysis.fingerAttributeList[i].score >= 75)
			pen.CreatePen(PS_SOLID, 1, RGB(0, 200, 0));
		else if(m_PreviewAnalysis.fingerAttributeList[i].score >= 50)
			pen.CreatePen(PS_SOLID, 1, RGB(255, 100, 50));
		else
			pen.CreatePen(PS_SOLID, 1, RGB(200, 0, 0));

		MemDC.SelectObject(&pen);

		x = m_PreviewAnalysis.fingerAttributeList[i].boundingBox.x * m_DispWidth / m_ImageWidth;
		y = m_PreviewAnalysis.fingerAttributeList[i].boundingBox.y * m_DispWidth / m_ImageWidth;
		w = m_PreviewAnalysis.fingerAttributeList[i].boundingBox.width * m_DispWidth / m_ImageWidth;
		h = m_PreviewAnalysis.fingerAttributeList[i].boundingBox.height * m_DispWidth / m_ImageWidth;

		MemDC.Rectangle(x, y, x+w, y+h);
		str.Format("%d", i+1);
		MemDC.TextOutA(x, y, str);

		x = m_PreviewAnalysis.fingerAttributeList[i].centroid.x * m_DispWidth / m_ImageWidth;
		y = m_PreviewAnalysis.fingerAttributeList[i].centroid.y * m_DispWidth / m_ImageWidth;
		MemDC.Ellipse(x-2, y-2, x+2, y+2);
	}*/

	MemDC.SelectObject(oldpen);

	BitBlt(dc.m_hDC, 0, 0, m_DispWidth, m_DispHeight, MemDC.m_hDC, 0, 0, SRCCOPY);
	MemDC.SelectObject(pOldMem);
	MemDC.DeleteDC();
	bmpMem.DeleteObject();
}

int CCBP_FunctionTesterDlg::_SaveBitmapImage(char *filename, unsigned char *buffer, 
											 int width, int height, int pitch, int invert)
{
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        return FALSE;
    }

    int i;
    BITMAPFILEHEADER header;
    BITMAPINFO *Info = (BITMAPINFO *)new unsigned char [1064];

    header.bfOffBits = 1078;
    header.bfReserved1 = 0;
    header.bfReserved2 = 0;
    header.bfSize = 1078 + width * height;
    header.bfType = (unsigned short)(('M' << 8) | 'B');

    for (i = 0; i < 256; i++)
    {
        Info->bmiColors[i].rgbBlue = i;
        Info->bmiColors[i].rgbRed = i;
        Info->bmiColors[i].rgbGreen = i;
        Info->bmiColors[i].rgbReserved = 0;
    }
    Info->bmiHeader.biBitCount = 8;
    Info->bmiHeader.biClrImportant = 0;
    Info->bmiHeader.biClrUsed = 0;
    Info->bmiHeader.biCompression = BI_RGB;
    Info->bmiHeader.biHeight = height;
    Info->bmiHeader.biPlanes = 1;
    Info->bmiHeader.biSize = 40;
    Info->bmiHeader.biSizeImage = width * height;
    Info->bmiHeader.biWidth = width;
    Info->bmiHeader.biXPelsPerMeter = 0;
    Info->bmiHeader.biYPelsPerMeter = 0;

    fwrite(&header, 1, 14, fp);
    fwrite(Info, 1, 1064, fp);

    unsigned char *new_buffer = new unsigned char [width * height];
    unsigned char *new_buffer2 = new unsigned char [width * height];

    if (invert == TRUE)
    {
        for (i = 0; i < width * height; i++)
        {
            new_buffer[i] = 255 - buffer[i];
        }
    }
    else
    {
        memcpy(new_buffer, buffer, width * height);
    }

	if(pitch < 0)
	{
		for (i = 0; i < height; i++)
        {
			memcpy(&new_buffer2[(height-1-i) * width], &new_buffer[i * width], width);
		}
	}
	else
	{
		memcpy(new_buffer2, new_buffer, width * height);
	}

	fwrite(new_buffer2, 1, width * height, fp);
    fclose(fp);

	delete [] new_buffer;
  	delete [] new_buffer2;
	delete Info;

    return TRUE;
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpConfigure()
{
	cbp_fp_configure(m_devHandle);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpGetDirtiness()
{
	cbp_fp_getDirtiness(m_devHandle);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpGetProperty()
{
	CString property_name, remark;
	m_cboProperty.GetLBText(m_cboProperty.GetCurSel(), property_name);
	remark.Format(">>> cbp_fp_getProperty");
	m_app->_ADD_LOG(remark);
	remark.Format("    Name: %s", property_name);
	m_app->_ADD_LOG(remark);
	remark.Format("    Value: %s", cbp_fp_getProperty(m_devHandle, (char*)property_name.GetBuffer()));
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpSetProperty()
{
	CString property_name, property_value, remark;
	m_cboProperty.GetLBText(m_cboProperty.GetCurSel(), property_name);

	cbp_fp_property property_Val;
	sprintf(property_Val.name, "%s", property_name);
	GetDlgItem(IDC_EDIT_PROPERTY)->GetWindowText(property_value);
	sprintf(property_Val.value, "%s", property_value);

	cbp_fp_setProperty(m_devHandle, &property_Val);
	remark.Format(">>> cbp_fp_setProperty");
	m_app->_ADD_LOG(remark);
	remark.Format("    Name: %s", property_Val.name);
	m_app->_ADD_LOG(remark);
	remark.Format("    Value: %s", property_Val.value);
	m_app->_ADD_LOG(remark);
}

void CCBP_FunctionTesterDlg::OnBnClickedButtonCbpFpPowerSave()
{
	cbp_fp_powerSave(m_devHandle, TRUE);
}

void CCBP_FunctionTesterDlg::GetAnalysisCodeString(int code, char * codeString)
{
	switch (code)
	{
	case CBP_FP_ANALYSIS_CODE_GOOD_PRINT:
		sprintf(codeString, "Good print");
		break;

	case CBP_FP_ANALYSIS_CODE_NO_PRINT:
		sprintf(codeString, "No print");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_DARK:
		sprintf(codeString, "Too dark");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_LIGHT:
		sprintf(codeString, "Too light");
		break;

	case CBP_FP_ANALYSIS_CODE_BAD_SHAPE:
		sprintf(codeString, "Bad shape");
		break;

	case CBP_FP_ANALYSIS_CODE_WRONG_SLAP:
		sprintf(codeString, "Wrong slap");
		break;

	case CBP_FP_ANALYSIS_CODE_BAD_POSITION:
		sprintf(codeString, "Bad position");
		break;

	case CBP_FP_ANALYSIS_CODE_ROTATE_CLOCKWISE:
		sprintf(codeString, "Rotate clockwise");
		break;

	case CBP_FP_ANALYSIS_CODE_ROTATE_COUNTERCLOCKWISE:
		sprintf(codeString, "Rotate counterclockwise");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_HIGH:
		sprintf(codeString, "Too high");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_LOW:
		sprintf(codeString, "Too low");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_LEFT:
		sprintf(codeString, "Too left");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_RIGHT:
		sprintf(codeString, "Too right");
		break;

	case CBP_FP_ANALYSIS_CODE_SPOOF_DETECTED:
		sprintf(codeString, "Spoof detected");
		break;

	default:
		sprintf(codeString, "Unknown code %d", code);
		break;
	}
}

void CCBP_FunctionTesterDlg::GetSlapTypeCodeString(cbp_fp_slapType code, char * codeString)
{
	switch(code)
	{
	case slap_rightHand: 
		sprintf(codeString, "slap_rightHand"); 
		break;

	case slap_leftHand:
		sprintf(codeString, "slap_leftHand"); 
		break;

	case slap_twoFingers: 
		sprintf(codeString, "slap_twoFingers"); 
		break;

	case slap_twoThumbs: 
		sprintf(codeString, "slap_twoThumbs"); 
		break;

	case slap_rightThumb: 
		sprintf(codeString, "slap_rightThumb"); 
		break;

	case slap_rightIndex: 
		sprintf(codeString, "slap_rightIndex"); 
		break;

	case slap_rightMiddle: 
		sprintf(codeString, "slap_rightMiddle"); 
		break;

	case slap_rightRing: 
		sprintf(codeString, "slap_rightRing"); 
		break;

	case slap_rightLittle: 
		sprintf(codeString, "slap_rightLittle"); 
		break;

	case slap_leftThumb: 
		sprintf(codeString, "slap_leftThumb"); 
		break;

	case slap_leftIndex: 
		sprintf(codeString, "slap_leftIndex"); 
		break;

	case slap_leftMiddle: 
		sprintf(codeString, "slap_leftMiddle"); 
		break;

	case slap_leftRing: 
		sprintf(codeString, "slap_leftRing"); 
		break;

	case slap_leftLittle: 
		sprintf(codeString, "slap_leftLittle"); 
		break;

	case slap_twotThumbs: 
		sprintf(codeString, "slap_twotThumbs"); 
		break;

	case slap_stitchedLeftThumb: 
		sprintf(codeString, "slap_stitchedLeftThumb"); 
		break;

	case slap_stitchedRightThumb: 
		sprintf(codeString, "slap_stitchedRightThumb"); 
		break;

	case slap_unknown: 
	default:
		sprintf(codeString, "slap_unknown"); 
		break;
	}
}

void CCBP_FunctionTesterDlg::GetCollectionTypeCodeString(cbp_fp_collectionType code, char * codeString)
{
	switch(code)
	{
	case collection_rolled: 
		sprintf(codeString, "collection_rolled"); 
		break;

	case collection_flat: 
		sprintf(codeString, "collection_flat"); 
		break;

	case collection_contactless: 
		sprintf(codeString, "collection_contactless"); 
		break;
	
	case collection_unknown:
	default: 
		sprintf(codeString, "collection_unknown");
		break;
	}
}

void CCBP_FunctionTesterDlg::OnBnClickedRadioMode1()
{
	m_OpenMode = 1;
}

void CCBP_FunctionTesterDlg::OnBnClickedRadioMode2()
{
	m_OpenMode = 2;
}

void CCBP_FunctionTesterDlg::OnBnClickedRadioMode3()
{
	m_OpenMode = 3;
}

void CCBP_FunctionTesterDlg::OnClose()
{
	cbp_fp_unlock(m_devHandle);

	CDialog::OnClose();
}

void CCBP_FunctionTesterDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu menu, *pMenu;
	menu.LoadMenu(IDR_MENU1);
	pMenu = menu.GetSubMenu(0);
	pMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
}

void CCBP_FunctionTesterDlg::OnMenuSaveImage()
{
	CFileDialog dlg(FALSE, 0, 0, 6, "Bitmap Image(*.bmp)|*.bmp||");

	if(dlg.DoModal() == IDOK)
	{
		CString filename = dlg.GetPathName();
		filename.MakeLower();

		if(filename.Find(".bmp") > 0)
		{
			filename = dlg.GetPathName();
		}
		else
		{
			filename = dlg.GetPathName() + ".bmp";
		}

		_SaveBitmapImage((char*)filename.GetBuffer(), m_Image, m_ImageWidth, m_ImageHeight, -m_ImageWidth, FALSE);
	}
}

