
// CBP_SampleForVCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CBP_SampleForVC.h"
#include "CBP_SampleForVCDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCBP_SampleForVCDlg *m_app;

// CCBP_SampleForVCDlg dialog
CCBP_SampleForVCDlg::CCBP_SampleForVCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCBP_SampleForVCDlg::IDD, pParent)
	, m_ProgramDestroyed(FALSE)
	, m_chkAutoCapture(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_app = this;

	m_Image = new BYTE[2048*2048];
	m_disp_buf = new BYTE [2048*2048];
	memset(m_Image, 255, 2048*2048);
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
	m_ImageInfo->bmiHeader.biHeight = 375;
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

	m_devHandle = -1;

	// register callbacks for CBP library
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
}

CCBP_SampleForVCDlg::~CCBP_SampleForVCDlg()
{
	delete [] m_Image;
	delete [] m_disp_buf;
	delete m_ImageInfo;
}

void CCBP_SampleForVCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FRAME_IMAGE, m_frameImage);
	DDX_Control(pDX, IDC_COMBO_DEVICES, m_cboDevices);
	DDX_Control(pDX, IDC_COMBO_CAPTURE_SEQ, m_cboSequence);
	DDX_Control(pDX, IDC_COMBO_COLLECTION_TYPE, m_cboCollectType);
	DDX_Control(pDX, IDC_LIST_LOG, m_ListLog);
	DDX_Check(pDX, IDC_CHECK_AUTOMATIC_CAPTURE, m_chkAutoCapture);
}

BEGIN_MESSAGE_MAP(CCBP_SampleForVCDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CCBP_SampleForVCDlg::OnBnClickedButtonRefresh)
	ON_BN_CLICKED(IDC_BUTTON_START_IMAGING, &CCBP_SampleForVCDlg::OnBnClickedButtonStartImaging)
	ON_BN_CLICKED(IDC_BUTTON_STOP_IMAGING, &CCBP_SampleForVCDlg::OnBnClickedButtonStopImaging)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE_IMAGE, &CCBP_SampleForVCDlg::OnBnClickedButtonCaptureImage)
	ON_BN_CLICKED(IDC_BUTTON_START_SCENARIO, &CCBP_SampleForVCDlg::OnBnClickedButtonStartScenario)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECK_AUTOMATIC_CAPTURE, &CCBP_SampleForVCDlg::OnBnClickedCheckAutomaticCapture)
END_MESSAGE_MAP()


// CCBP_SampleForVCDlg message handlers
HBRUSH CCBP_SampleForVCDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	const int WndID = pWnd->GetDlgCtrlID();
	switch( WndID )
	{
	case IDC_STATIC_DLL_VER:
	case IDC_STATIC_COPYRIGHT:
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(255, 255, 255));
		hbr = (HBRUSH)GetStockObject( NULL_BRUSH );
		if( WndID == IDC_STATIC_DLL_VER )
		{
			CRect	rect;
			CFont	fFont, *fOldFont;
			int		font_size;
			CString szText;

			pWnd->GetWindowText(szText);
			pWnd->GetClientRect(rect);

			fFont.Detach();
			font_size = -MulDiv(14, GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
			if( fFont.CreateFont(font_size, 0, 0, 0, FW_NORMAL, 0, 0, 0,DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, ANTIALIASED_QUALITY,
				DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif") != NULL )
			{
				fOldFont = pDC->SelectObject(&fFont);
				pDC->DrawText(szText, rect, DT_LEFT);
			}
		}
		break;
	}

	return hbr;
}

BOOL CCBP_SampleForVCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CString titleName;
	titleName.Format("CBP Demo App with CBP 10-print API");
	SetWindowText(titleName);

	CString strDrawString;
	strDrawString.Format("CBP Demo App with CBP 10-print API ver. 2.0.1");
	GetDlgItem(IDC_STATIC_DLL_VER)->SetWindowText(strDrawString);

	strDrawString = _T("Copyright (c) Integrated Biometrics");
	GetDlgItem(IDC_STATIC_COPYRIGHT)->SetWindowText(strDrawString);


	m_cboDevices.ResetContent();
	m_ListLog.DeleteAllItems();

	int idx = 0;
	m_cboSequence.InsertString(idx++, "slap_rightHand");
	m_cboSequence.InsertString(idx++, "slap_leftHand");
	m_cboSequence.InsertString(idx++, "slap_twoFingers");
	m_cboSequence.InsertString(idx++, "slap_twoThumbs");
	m_cboSequence.InsertString(idx++, "slap_twotThumbs");
	m_cboSequence.InsertString(idx++, "slap_rightThumb");
	m_cboSequence.InsertString(idx++, "slap_rightIndex");
	m_cboSequence.InsertString(idx++, "slap_rightMiddle");
	m_cboSequence.InsertString(idx++, "slap_rightRing");
	m_cboSequence.InsertString(idx++, "slap_rightLittle");
	m_cboSequence.InsertString(idx++, "slap_leftThumb");
	m_cboSequence.InsertString(idx++, "slap_leftIndex");
	m_cboSequence.InsertString(idx++, "slap_leftMiddle");
	m_cboSequence.InsertString(idx++, "slap_leftRing");
	m_cboSequence.InsertString(idx++, "slap_leftLittle");
	m_cboSequence.InsertString(idx++, "slap_stitchedLeftThumb");
	m_cboSequence.InsertString(idx++, "slap_stitchedRightThumb");
	m_cboSequence.InsertString(idx++, "slap_unknown");

	idx = 0;
	m_cboCollectType.InsertString(idx++, "collection_rolled");
	m_cboCollectType.InsertString(idx++, "collection_flat");
	m_cboCollectType.InsertString(idx++, "collection_contactless");
	m_cboCollectType.InsertString(idx++, "collection_unknown");

	CRect rect;
	m_ListLog.GetClientRect(&rect);
	m_ListLog.InsertColumn(0, _T(""), LVCFMT_LEFT, 0);
	m_ListLog.InsertColumn(1, _T("logs"), LVCFMT_LEFT, rect.Width()-10);

	int nRc = cbp_fp_registerCallBacks(&m_callbacklist);

	CString remark;
	remark.Format("%d", nRc);
	m_app->_ADD_LOG("cbp_fp_registerCallBacks", remark);

	cbp_fp_initialize("Integrated Biometrics");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCBP_SampleForVCDlg::OnPaint()
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
	}
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onCalibrate(int handle)
{
	CString remark;
	remark.Format("%d", handle);
	m_app->_ADD_LOG("cbp_fp_onCalibrate", remark);
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onCapture(int handle, struct cbp_fp_grayScaleCapture *grayScaleCapture)
{
	CString remark;
	remark.Format("%d", handle);
	m_app->_ADD_LOG("cbp_fp_onCapture", remark);

	memcpy(m_app->m_Image, grayScaleCapture->image, grayScaleCapture->width*grayScaleCapture->height);
	m_app->m_ImageWidth = grayScaleCapture->width;
	m_app->m_ImageHeight = grayScaleCapture->height;

	m_app->_DisplayImage();

	if(m_app->m_cbp_seq.size() > 0)
	{
		if(m_app->m_nCurrentCaptureStep >= (int)m_app->m_cbp_seq.size() || m_app->m_nCurrentCaptureStep == -1)
		{
			m_app->m_cbp_seq.clear();
			m_app->m_nCurrentCaptureStep = -1;
			return;
		}

		CaptureSeq seq = m_app->m_cbp_seq[m_app->m_nCurrentCaptureStep];
		m_app->m_nCurrentCaptureStep++;
		m_app->m_slaptype = seq.slap_type;
		m_app->m_collectiontype = seq.collect_type;

		// start next capture
		cbp_fp_startImaging(handle, seq.slap_type, seq.collect_type);
	}
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onClose(int handle)
{
	CString remark;
	remark.Format("%d", handle);
	m_app->_ADD_LOG("cbp_fp_onClose", remark);
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onConfigure(int handle, bool configurationChange)
{
	CString remark;
	remark.Format("%d", handle);
	m_app->_ADD_LOG("cbp_fp_onConfigure", remark);
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onEnumDevices(struct cbp_fp_device *deviceList[], int deviceListLen, char *requestID)
{
	CString remark;
	remark.Format("%d", deviceListLen);
	m_app->_ADD_LOG("cbp_fp_onEnumDevices", remark);

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
		cbp_fp_open(&m_app->m_cbp_devices.at(0));
	}
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onError(int errorNumber, char *errorDescription)
{
	CString remark;
	remark.Format("%d %s", errorNumber, errorDescription);
	m_app->_ADD_LOG("cbp_fp_onError", remark);
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onGetDirtiness(int handle , int dirtinessLevel)
{
	CString remark;
	remark.Format("%d %d", handle, dirtinessLevel);
	m_app->_ADD_LOG("cbp_fp_onGetDirtiness", remark);
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onGetLockInfo(int handle, struct cbp_fp_lock_info *lockInfo)
{
	CString remark;
	remark.Format("%d", handle);
	m_app->_ADD_LOG("cbp_fp_onGetLockInfo", remark);
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onInitialize(char *requestID)
{
	CString remark;
	remark.Format("%s", requestID);
	m_app->_ADD_LOG("cbp_fp_onInitialize", remark);
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onLock(int handle, int processID)
{
	CString remark;
	remark.Format("%d %d", handle, processID);
	m_app->_ADD_LOG("cbp_fp_onLock", remark);
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onOpen(int handle, struct cbp_fp_scanner_connection *scanner_connection)
{
	CString remark;
	remark.Format("%d", handle);
	m_app->_ADD_LOG("cbp_fp_onOpen", remark);

	m_app->m_devHandle = handle;

	// register callbacks
	//cbp_fp_calibrate(handle);
	//cbp_fp_lock(handle);
	//cbp_fp_configure(handle);
	//cbp_fp_powerSave(handle, FALSE);
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onPowerSave(int  handle, bool isOnPowerSave)
{
	CString remark;
	remark.Format("%d %d", handle, isOnPowerSave);
	m_app->_ADD_LOG("cbp_fp_onPowerSave", remark);
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onPreview(int handle, struct cbp_fp_grayScalePreview *preview)
{
	CString remark;
	remark.Format("%d", handle);
	m_app->_ADD_LOG("cbp_fp_onPreview", remark);

	memcpy(m_app->m_Image, preview->image, preview->width*preview->height);
	m_app->m_ImageWidth = preview->width;
	m_app->m_ImageHeight = preview->height;
	
	m_app->_DisplayImage();
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onPreviewAnalysis(int handle, struct cbp_fp_previewAnalysis *previewAnalysis)
{
	memcpy(&m_app->m_PreviewAnalysis, previewAnalysis, sizeof(cbp_fp_previewAnalysis));
	
	CString remark;
	remark.Format("(%d,%d,%d,%d)", 
		m_app->m_PreviewAnalysis.fingerAttributeList[0].score,
		m_app->m_PreviewAnalysis.fingerAttributeList[1].score,
		m_app->m_PreviewAnalysis.fingerAttributeList[2].score,
		m_app->m_PreviewAnalysis.fingerAttributeList[3].score);
	m_app->_ADD_LOG("cbp_fp_onPreviewAnalysis", remark);

	m_app->_DisplayImage();

	if(m_app->m_cbp_seq.size() > 0 && m_app->m_chkAutoCapture)
	{
		int TrueCount=0;
		for(int i=0; i<m_app->m_PreviewAnalysis.fingerAttributeListLen; i++)
		{
			BOOL CaptureFinish = FALSE;
			for(int j=0; j<m_app->m_PreviewAnalysis.fingerAttributeList[i].analysisCodeListLen; j++)
			{
				if(m_app->m_PreviewAnalysis.fingerAttributeList[i].analysisCodeList[j] == CBP_FP_ANALYSIS_CODE_GOOD_PRINT)
				{
					CaptureFinish = TRUE;
					break;
				}
			}
			if(CaptureFinish == TRUE)
			{
				TrueCount++;
			}
		}

		if(m_app->m_PreviewAnalysis.fingerAttributeListLen == TrueCount && TrueCount > 0)
		{
			cbp_fp_capture(handle, previewAnalysis->slapType, previewAnalysis->collectionType);
		}
	}
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onStopPreview(int handle)
{
	CString remark;
	remark.Format("%d", handle);
	m_app->_ADD_LOG("cbp_fp_onStopPreview", remark);
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onUninitialize(char *requestID)
{
	CString remark;
	remark.Format("%s", requestID);
	m_app->_ADD_LOG("cbp_fp_onUninitialize", remark);
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onUnLock(int handle)
{
	CString remark;
	remark.Format("%d", handle);
	m_app->_ADD_LOG("cbp_fp_onUnLock", remark);
}

void CCBP_SampleForVCDlg::OnEvent_cbp_fp_onWarning(int warningNumber, char *warningDescription)
{
	CString remark;
	remark.Format("%d %s", warningNumber, warningDescription);
	m_app->_ADD_LOG("cbp_fp_onWarning", remark);
}

///////////////////////////////////////////////////////////////////////////////////////

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCBP_SampleForVCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCBP_SampleForVCDlg::_DisplayImage()
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

	CClientDC dc(&m_frameImage);
	
	CDC MemDC;
	MemDC.CreateCompatibleDC(&dc);
	CBitmap bmpMem, *pOldMem;
	bmpMem.CreateCompatibleBitmap(&dc, m_DispWidth, m_DispHeight);
	pOldMem = (CBitmap *)MemDC.SelectObject(&bmpMem); 

	::StretchDIBits(MemDC.m_hDC, 0, 0, m_DispWidth, m_DispHeight, 0, 0, m_DispWidth, m_DispHeight, 
		m_disp_buf, m_ImageInfo, DIB_RGB_COLORS, SRCCOPY);

	/*int x, y, w, h;

	CPen pen, *oldpen;
	pen.CreatePen(PS_SOLID, 1, RGB(0, 200, 0));
	oldpen = MemDC.SelectObject(&pen);

	MemDC.SelectObject(GetStockObject(NULL_BRUSH));
	MemDC.SetBkMode(TRANSPARENT);

	CString str;
	for(int i=0; i<m_PreviewAnalysis.fingerAttributeListLen; i++)
	{
		x = m_PreviewAnalysis.fingerAttributeList[i].boundingBox.x * m_DispWidth / m_ImageWidth;
		y = m_PreviewAnalysis.fingerAttributeList[i].boundingBox.y * m_DispWidth / m_ImageWidth;
		w = m_PreviewAnalysis.fingerAttributeList[i].boundingBox.width * m_DispWidth / m_ImageWidth;
		h = m_PreviewAnalysis.fingerAttributeList[i].boundingBox.height * m_DispWidth / m_ImageWidth;

		MemDC.Rectangle(x, y, x+w, y+h);
		str.Format("%d", i+1);
		MemDC.TextOutA(x, y, str);
	}

	MemDC.SelectObject(oldpen);*/

	BitBlt(dc.m_hDC, 0, 0, m_DispWidth, m_DispHeight, MemDC.m_hDC, 0, 0, SRCCOPY);
	MemDC.SelectObject(pOldMem);
	MemDC.DeleteDC();
	bmpMem.DeleteObject();
}

void CCBP_SampleForVCDlg::_ADD_LOG(CString log, CString remark, BOOL newStart)
{
	if(m_ProgramDestroyed == TRUE)
		return;

	if(newStart == TRUE)
		m_ListLog.DeleteAllItems();

	int idx = m_ListLog.GetItemCount();

	m_ListLog.InsertItem(idx, "");
	m_ListLog.SetItemText(idx, 1, log + "(" + remark + ")");
	//m_ListLog.SetItemText(idx, 2, remark);

	CSize size;
    size.cy=(m_ListLog.GetItemCount()-1)*12;
    size.cx=0;

    if(m_ListLog.Scroll(size))
    {
       m_ListLog.SetItemState(m_ListLog.GetItemCount()-1, LVIS_SELECTED,LVIS_SELECTED);
       m_ListLog.SetFocus();
    }
}

void CCBP_SampleForVCDlg::OnBnClickedButtonRefresh()
{
	cbp_fp_enumDevices("Integrated Biometrics");
}

void CCBP_SampleForVCDlg::OnBnClickedButtonStartImaging()
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

	m_cbp_seq.clear();
	CaptureSeq seq;
	seq.slap_type = slap_type;
	seq.collect_type = collect_type;
	m_cbp_seq.push_back(seq);
	m_nCurrentCaptureStep = 1;

	// start streaming
	cbp_fp_startImaging(m_devHandle, slap_type, collect_type);
}

void CCBP_SampleForVCDlg::OnBnClickedButtonStopImaging()
{
	cbp_fp_stopImaging(m_devHandle);
}

void CCBP_SampleForVCDlg::OnBnClickedButtonCaptureImage()
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

void CCBP_SampleForVCDlg::OnBnClickedButtonStartScenario()
{
	if(m_chkAutoCapture == FALSE)
	{
		AfxMessageBox("Please enable \"Automatic Fingerprint Capture\" checkbox first.", MB_OK);
		return;
	}
	
	char pString[256];
	
	m_cboSequence.GetLBText(m_cboSequence.GetCurSel(), pString);
	m_cbp_seq.clear();

	CaptureSeq seq;

	seq.slap_type = slap_rightHand;
	seq.collect_type = collection_flat;
	m_cbp_seq.push_back(seq);

	seq.slap_type = slap_rightThumb;
	seq.collect_type = collection_flat;
	m_cbp_seq.push_back(seq);

	seq.slap_type = slap_leftHand;
	seq.collect_type = collection_flat;
	m_cbp_seq.push_back(seq);

	seq.slap_type = slap_leftThumb;
	seq.collect_type = collection_flat;
	m_cbp_seq.push_back(seq);

	m_nCurrentCaptureStep = 0;

	seq = m_cbp_seq[m_nCurrentCaptureStep];
	m_nCurrentCaptureStep++;

	cbp_fp_startImaging(m_devHandle, seq.slap_type, seq.collect_type); 
}

void CCBP_SampleForVCDlg::OnDestroy()
{
	m_ProgramDestroyed = TRUE;

	if(m_devHandle != -1)
		cbp_fp_close(m_devHandle);

	cbp_fp_uninitialize("Integrated Biometrics");

	CDialog::OnDestroy();
}

void CCBP_SampleForVCDlg::OnClose()
{
	m_ProgramDestroyed = TRUE;

	CDialog::OnClose();
}

void CCBP_SampleForVCDlg::OnBnClickedCheckAutomaticCapture()
{
	UpdateData(TRUE);
}
