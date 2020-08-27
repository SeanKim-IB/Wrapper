
// CBPLogViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CBPLogViewer.h"
#include "CBPLogViewerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCBPLogViewerDlg dialog
const TCHAR *lpszHeader1[] = {	_T("Logs"),		// 0
								NULL};	

CCBPLogViewerDlg::CCBPLogViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCBPLogViewerDlg::IDD, pParent)
	, m_Summary(_T(""))
	, m_EndofList(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_database.clear();
}

void CCBPLogViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FILTERED_LOG, m_listFilteredLog);
	DDX_Text(pDX, IDC_EDIT_SUMMARY, m_Summary);
	DDX_Control(pDX, IDC_PROGRESS_LOG, m_progressLog);
}

BEGIN_MESSAGE_MAP(CCBPLogViewerDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_FILTERED_LOG, &CCBPLogViewerDlg::OnGetdispinfoList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_FILTERED_LOG, &CCBPLogViewerDlg::OnCustomdrawList)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_LOG, &CCBPLogViewerDlg::OnBnClickedButtonOpenLog)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_BRIEF_LOG, &CCBPLogViewerDlg::OnBnClickedButtonSaveBriefLog)
END_MESSAGE_MAP()


// CCBPLogViewerDlg message handlers

BOOL CCBPLogViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	// Get version of this app
	TCHAR MyName[MAX_PATH];
	GetModuleFileName(AfxGetStaticModuleState()->m_hCurrentInstanceHandle, MyName, MAX_PATH);
	CString FileVersion=_T("");
    DWORD infoSize = 0;
	char *buffer;

	infoSize = GetFileVersionInfoSize(MyName, 0);
	if(infoSize>0)
	{
		buffer = new char[infoSize];
		if(buffer)
		{
			if(GetFileVersionInfo(MyName,0,infoSize, buffer)!=0)
			{
				VS_FIXEDFILEINFO* pFineInfo = NULL;
				UINT bufLen = 0;

				if(VerQueryValue(buffer,"\\",(LPVOID*)&pFineInfo, &bufLen) !=0)
				{    
					WORD majorVer, minorVer, buildNum, revisionNum;
					majorVer = HIWORD(pFineInfo->dwFileVersionMS);
					minorVer = LOWORD(pFineInfo->dwFileVersionMS);
					buildNum = HIWORD(pFineInfo->dwFileVersionLS);
					revisionNum = LOWORD(pFineInfo->dwFileVersionLS);

					m_FileVersion.Format("CBP Log Viewer ( Version %d.%d.%d.%d )", majorVer, minorVer, buildNum, revisionNum);
				}
			}
			delete[] buffer;
		}
	}
	
	if(m_FileVersion.GetLength()==0)
		m_FileVersion.Format("CBP Log Viewer ( can't read version )");

	SetWindowText(m_FileVersion);

	m_progressLog.SetRange(0,100);
	m_progressLog.SetPos(0);

	CRect rect;
	m_listFilteredLog.GetWindowRect(&rect);
	m_listFilteredLog.InsertColumn(0, _T("ERR"), LVCFMT_CENTER, 40);
	m_listFilteredLog.InsertColumn(1, _T("           Logs"), LVCFMT_LEFT, rect.Width()-60);
	m_listFilteredLog.SetExtendedStyle(m_listFilteredLog.GetExtendedStyle() | LVS_EX_FULLROWSELECT );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCBPLogViewerDlg::OnPaint()
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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCBPLogViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCBPLogViewerDlg::OnBnClickedButtonOpenLog()
{
	GetDlgItem(IDC_EDIT_SUMMARY)->SetWindowText("");

	CFileDialog dlg(TRUE, 0, 0, 6, "log file(*.log)|*.log||");

	if(dlg.DoModal() == IDOK)
	{
		FILE *fp = fopen(dlg.GetPathName(), "rt");
		if(fp == NULL)
			return;

		m_database.clear();
		m_listFilteredLog.DeleteAllItems();

		char filename[512];
		sprintf(filename, "%s - %s", (char*)m_FileVersion.GetBuffer(), dlg.GetFileName());
		SetWindowText(filename);

		_AalysisLog_rev2(fp);

		fclose(fp);
	}
}

void CCBPLogViewerDlg::_AalysisLog(FILE *fp)
{
	int StartImagingCount = 0;
	int TokenCount=0;
	char strlineorg[512], strline[512], *pch;
	char save_strTimeStamp[512], save_EnterExit[512];
	char save_strTimeStamp_Start[512], save_strTimeStamp_Detect[512], save_strTimeStampScore75[512];
	char save_strTimeStamp_Capture[512], save_strTimeStampError[512], save_strError[512], strmax[2048];
	char delimit[]=" \t\r\n\v\f"; 
	char result[512];
	BOOL isFound_Start = FALSE, isFound_Stop, isFound_unLock, isFound_Detect;
	BOOL isReadyFound75, isAlreadyFound75, isFound_Score75, isFound_Capture, isFound_Error;

	long time_start, time_detect, time_score75, time_capture, time_error;
	long time_diff_detect_to_score75;
	long time_diff_detect_to_capture;
	long min_time_diff_detect_to_score75, max_time_diff_detect_to_score75;
	long min_time_diff_detect_to_capture, max_time_diff_detect_to_capture;
	long avg_time_diff_detect_to_score75, avg_time_diff_detect_to_capture;
	long total_capture_count, total_capture_count_score75, total_capture_count_capture;

	total_capture_count = 0;
	total_capture_count_score75 = 0;
	total_capture_count_capture = 0;
	avg_time_diff_detect_to_score75 = 0;
	avg_time_diff_detect_to_capture = 0;
	min_time_diff_detect_to_score75 = 100000;
	max_time_diff_detect_to_score75 = -100000;
	min_time_diff_detect_to_capture = 100000;
	max_time_diff_detect_to_capture = -100000;

start_again:

	// Find StartImaing
	isFound_Start = FALSE;
	sprintf(save_strTimeStamp_Start, "");
	sprintf(save_strTimeStamp_Detect, "");
	sprintf(save_strTimeStampScore75, "");
	sprintf(save_strTimeStamp_Capture, "");
	sprintf(save_strTimeStampError, "");
	sprintf(save_strError, "");
	sprintf(strmax, "");

	while(fgets ( strlineorg, 512, fp ) != NULL)
	{
		TokenCount = 0;

		sprintf(strline, "%s", strlineorg);
		pch = strtok (strline, delimit);
		while (pch != NULL)
		{
			if(TokenCount == 1)
			{
				sprintf(save_strTimeStamp, "%s", pch);
				//strncat(save_strTimeStamp, pch, 12);
				save_strTimeStamp[strlen(save_strTimeStamp)-1]='\0';
			}
			else if(TokenCount == 3 && strcmp(pch, "cbp_fp_startImaging") == 0)
			{
				isFound_Start = TRUE;
				sprintf(save_strTimeStamp_Start, "%s", save_strTimeStamp);
				break;
			}

			pch = strtok (NULL, delimit);

			TokenCount++;
		}

		if(isFound_Start == TRUE)
		{
			//TRACE(strlineorg);
			_AddLog(strlineorg, FALSE);
			break;
		}
	}

	if(isFound_Start == FALSE)
		goto done;

	// Find Stop or UnLock
	isFound_Stop = FALSE;
	isFound_unLock = FALSE;
	isFound_Detect = FALSE;
	isReadyFound75 = FALSE;
	isFound_Score75 = FALSE;
	isAlreadyFound75 = FALSE;
	while(fgets ( strlineorg, 512, fp ) != NULL)
	{
		TokenCount = 0;

		sprintf(strline, "%s", strlineorg);
		sprintf(save_EnterExit, "");
		pch = strtok (strline, delimit);
		while (pch != NULL)
		{
			if(TokenCount == 1)
			{
				sprintf(save_strTimeStamp, "%s", pch);
				//strncat(save_strTimeStamp, pch, 12);
				save_strTimeStamp[strlen(save_strTimeStamp)-1]='\0';
			}
			else if(TokenCount == 3)
			{
				if(strcmp(pch, "cbp_fp_stopImaging") == 0 && isFound_Stop == FALSE)
				{
					isFound_Stop = TRUE;
					break;
				}
				if(strcmp(pch, "cbp_fp_unlock") == 0 && isFound_unLock == FALSE)
				{
					isFound_unLock = TRUE;
					break;
				}
			}
			else if(TokenCount == 9 && strcmp(pch, "75") != 0 && isFound_Detect == FALSE && isFound_Score75 == FALSE)
			{
				isFound_Detect = TRUE;
				sprintf(save_strTimeStamp_Detect, "%s", save_strTimeStamp);
				break;
			}
			else if(TokenCount == 9 && strcmp(pch, "75") == 0 && isReadyFound75 == TRUE && isFound_Score75 == FALSE)
			{
				isFound_Score75 = TRUE;
				sprintf(save_strTimeStampScore75, "%s", save_strTimeStamp);
				break;
			}
			
			pch = strtok (NULL, delimit);

			TokenCount++;
		}

		if(isFound_Stop == TRUE)
		{
			//TRACE(strlineorg);
			_AddLog(strlineorg, FALSE);
			sprintf(strlineorg, "");
			break;
		}
		else if(isFound_unLock == TRUE)
		{
			//TRACE(strlineorg);
			_AddLog(strlineorg, FALSE, 1);
			break;
		}
		else if(isFound_Detect == TRUE && isReadyFound75 == FALSE)
		{
			//TRACE(strlineorg);
			_AddLog(strlineorg, FALSE);
			sprintf(strlineorg, "");
			isReadyFound75 = TRUE;
		}
		else if(isFound_Score75 == TRUE && isAlreadyFound75 == FALSE)
		{
			//TRACE(strlineorg);
			_AddLog(strlineorg, FALSE);
			sprintf(strlineorg, "");
			isAlreadyFound75 = TRUE;
		}
	}

	if(isFound_Stop)
	{
		isFound_Capture = FALSE;
		isFound_Error = FALSE;
		// Find capture
		while(fgets ( strlineorg, 512, fp ) != NULL)
		{
			TokenCount = 0;

			sprintf(strline, "%s", strlineorg);
			sprintf(save_EnterExit, "");
			pch = strtok (strline, delimit);
			while (pch != NULL)
			{
				if(TokenCount == 1)
				{
					sprintf(save_strTimeStamp, "%s", pch);
					//strncat(save_strTimeStamp, pch, 12);
					save_strTimeStamp[strlen(save_strTimeStamp)-1]='\0';
				}
				else if(TokenCount == 2)
				{
					sprintf(save_EnterExit, "%s", pch);
				}
				else if(TokenCount == 3)
				{
					if(strcmp(pch, "cbp_fp_onCapture") == 0 && isFound_Capture == FALSE)
					{
						isFound_Capture = TRUE;
						sprintf(save_strTimeStamp_Capture, "%s", save_strTimeStamp);
						break;
					}
					else if(strcmp(pch, "callback_error") == 0 && isFound_Error == FALSE)
					{
						isFound_Error = TRUE;
						sprintf(save_strTimeStampError, "%s", save_strTimeStamp);
						break;
					}
				}

				pch = strtok (NULL, delimit);

				TokenCount++;
			}

			if(isFound_Capture == TRUE)
			{
				//TRACE("%s\n", strlineorg);
				_AddLog(strlineorg, FALSE);
				break;
			}
			else if(isFound_Error == TRUE)
			{
				_AddLog(strlineorg, FALSE, 2);
				break;
			}
		}
	}

	time_start = _GetTimeStampToInt(save_strTimeStamp_Start);
	time_detect = _GetTimeStampToInt(save_strTimeStamp_Detect);
	time_score75 = _GetTimeStampToInt(save_strTimeStampScore75);
	time_capture = _GetTimeStampToInt(save_strTimeStamp_Capture);
	time_error = _GetTimeStampToInt(save_strTimeStampError);

	if(time_score75 != 0 && time_detect != 0)
		time_diff_detect_to_score75 = time_score75 - time_detect;
	else
		time_diff_detect_to_score75 = 0;

	if(time_capture != 0 && time_detect != 0)
		time_diff_detect_to_capture = time_capture - time_detect;
	else
		time_diff_detect_to_capture = 0;

	total_capture_count++;

	if(time_diff_detect_to_score75 != 0)
	{
		total_capture_count_score75++;
		avg_time_diff_detect_to_score75 += time_diff_detect_to_score75;

		if(min_time_diff_detect_to_score75 > time_diff_detect_to_score75)
			min_time_diff_detect_to_score75 = time_diff_detect_to_score75;
		if(max_time_diff_detect_to_score75 < time_diff_detect_to_score75)
			max_time_diff_detect_to_score75 = time_diff_detect_to_score75;
	}

	if(time_diff_detect_to_capture != 0)
	{
		total_capture_count_capture++;
		avg_time_diff_detect_to_capture += time_diff_detect_to_capture;

		if(min_time_diff_detect_to_capture > time_diff_detect_to_capture)
			min_time_diff_detect_to_capture = time_diff_detect_to_capture;
		if(max_time_diff_detect_to_capture < time_diff_detect_to_capture)
			max_time_diff_detect_to_capture = time_diff_detect_to_capture;
	}

	sprintf(result, ">>> Detect -> Score75 (%.1f sec), Detect -> onCapture (%.1f sec)", 
		 time_diff_detect_to_score75/1000.0f, time_diff_detect_to_capture/1000.0f);

	if(isFound_Error == TRUE || isFound_unLock == TRUE)
		sprintf(result, "");

	if(isFound_unLock == TRUE)
		_AddLog(result, FALSE);
	else
		_AddLog(result, TRUE);

	if(strlen(strlineorg) > 10)
	{
		sprintf(strlineorg, "");
		goto start_again;
	}

done:

	if(total_capture_count_score75 > 0)
	{
		avg_time_diff_detect_to_score75 /= total_capture_count_score75;
	}
	else
	{
		avg_time_diff_detect_to_score75 = 0;
		min_time_diff_detect_to_score75 = 0;
		max_time_diff_detect_to_score75 = 0;
	}
	
	if(total_capture_count_capture > 0)
	{
		avg_time_diff_detect_to_capture /= total_capture_count_capture;
	}
	else
	{
		avg_time_diff_detect_to_capture = 0;
		min_time_diff_detect_to_capture = 0;
		max_time_diff_detect_to_capture = 0;
	}

	sprintf(result, "Total StartImaging #(%d), Score75 #(%d), Capture #(%d)\r\n\
avgScore75(%.1f sec), minScore75(%.1f sec), maxScore75(%.1f sec)\r\n\
avgCapture(%.1f sec), minCapture(%.1f sec), maxCapture(%.1f sec)",
				total_capture_count, total_capture_count_score75, total_capture_count_capture,
				avg_time_diff_detect_to_score75/1000.0f, min_time_diff_detect_to_score75/1000.0f, max_time_diff_detect_to_score75/1000.0f,
				avg_time_diff_detect_to_capture/1000.0f, min_time_diff_detect_to_capture/1000.0f, max_time_diff_detect_to_capture/1000.0f); 
	GetDlgItem(IDC_EDIT_SUMMARY)->SetWindowText(result);
}

void CCBPLogViewerDlg::_AalysisLog_rev1(FILE *fp)
{
	int StartImagingCount = 0;
	int TokenCount=0;
	char strlineorg[512], strline[512], *pch;
	char save_strTimeStamp[512], save_EnterExit[512];
	char save_strTimeStamp_Start[512], save_strTimeStamp_Detect[512], save_strTimeStampScore75[512];
	char save_strTimeStamp_Capture[512], save_strTimeStampError[512], save_strError[512], strmax[2048];
	char delimit[]=" \t\r\n\v\f"; 
	char result[512];
	BOOL isFound_Start = FALSE, isFound_Stop, isFound_unLock, isFound_Detect;
	BOOL isReadyFound75, isAlreadyFound75, isFound_Score75, isFound_Capture, isFound_Error;

	long time_start, time_detect, time_score75, time_capture, time_error;
	long time_diff_detect_to_score75;
	long time_diff_detect_to_capture;
	long min_time_diff_detect_to_score75, max_time_diff_detect_to_score75;
	long min_time_diff_detect_to_capture, max_time_diff_detect_to_capture;
	long avg_time_diff_detect_to_score75, avg_time_diff_detect_to_capture;
	long total_capture_count, total_capture_count_score75, total_capture_count_capture;

	total_capture_count = 0;
	total_capture_count_score75 = 0;
	total_capture_count_capture = 0;
	avg_time_diff_detect_to_score75 = 0;
	avg_time_diff_detect_to_capture = 0;
	min_time_diff_detect_to_score75 = 100000;
	max_time_diff_detect_to_score75 = -100000;
	min_time_diff_detect_to_capture = 100000;
	max_time_diff_detect_to_capture = -100000;

start_again:

	// Find StartImaing
	isFound_Start = FALSE;
	sprintf(save_strTimeStamp_Start, "");
	sprintf(save_strTimeStamp_Detect, "");
	sprintf(save_strTimeStampScore75, "");
	sprintf(save_strTimeStamp_Capture, "");
	sprintf(save_strTimeStampError, "");
	sprintf(save_strError, "");
	sprintf(strmax, "");

	while(fgets ( strlineorg, 512, fp ) != NULL)
	{
		TokenCount = 0;

		sprintf(strline, "%s", strlineorg);
		pch = strtok (strline, delimit);
		while (pch != NULL)
		{
			if(TokenCount == 1)
			{
				sprintf(save_strTimeStamp, "%s", pch);
				//strncat(save_strTimeStamp, pch, 12);
				save_strTimeStamp[strlen(save_strTimeStamp)-1]='\0';
			}
			else if(TokenCount == 3 && strcmp(pch, "cbp_fp_startImaging") == 0)
			{
				isFound_Start = TRUE;
				sprintf(save_strTimeStamp_Start, "%s", save_strTimeStamp);
				break;
			}

			pch = strtok (NULL, delimit);

			TokenCount++;
		}

		if(isFound_Start == TRUE)
		{
			//TRACE(strlineorg);
			_AddLog(strlineorg, FALSE);
			break;
		}
	}

	if(isFound_Start == FALSE)
		goto done;

	// Find Stop or UnLock
	isFound_Capture = FALSE;
	isFound_Stop = FALSE;
	isFound_Error = FALSE;
	isFound_unLock = FALSE;
	isFound_Detect = FALSE;
	isReadyFound75 = FALSE;
	isFound_Score75 = FALSE;
	isAlreadyFound75 = FALSE;
	while(fgets ( strlineorg, 512, fp ) != NULL)
	{
		TokenCount = 0;

		sprintf(strline, "%s", strlineorg);
		sprintf(save_EnterExit, "");
		pch = strtok (strline, delimit);
		while (pch != NULL)
		{
			if(TokenCount == 1)
			{
				sprintf(save_strTimeStamp, "%s", pch);
				//strncat(save_strTimeStamp, pch, 12);
				save_strTimeStamp[strlen(save_strTimeStamp)-1]='\0';
			}
			else if(TokenCount == 3)
			{
				if(strcmp(pch, "cbp_fp_unlock") == 0 && isFound_unLock == FALSE)
				{
					isFound_unLock = TRUE;
					//break;
				}
				if(strcmp(pch, "cbp_fp_onCapture") == 0 && isFound_Capture == FALSE)
				{
					isFound_Capture = TRUE;
					sprintf(save_strTimeStamp_Capture, "%s", save_strTimeStamp);
					//break;
				}
				else if(strcmp(pch, "callback_error") == 0 && isFound_Error == FALSE)
				{
					isFound_Error = TRUE;
					sprintf(save_strTimeStampError, "%s", save_strTimeStamp);
					//break;
				}
			}
			else if(TokenCount == 9 && strcmp(pch, "75") != 0 && isFound_Detect == FALSE && isFound_Score75 == FALSE)
			{
				isFound_Detect = TRUE;
				sprintf(save_strTimeStamp_Detect, "%s", save_strTimeStamp);
				//break;
			}
			else if(TokenCount == 9 && strcmp(pch, "75") == 0 && isReadyFound75 == TRUE && isFound_Score75 == FALSE)
			{
				isFound_Score75 = TRUE;
				sprintf(save_strTimeStampScore75, "%s", save_strTimeStamp);
				//break;
			}
			
			pch = strtok (NULL, delimit);

			TokenCount++;
		}

		if(isFound_Capture == TRUE)
		{
			//TRACE(strlineorg);
			_AddLog(strlineorg, FALSE);
			//sprintf(strlineorg, "");
			break;
		}
		else if(isFound_Error == TRUE)
		{
			//TRACE(strlineorg);
			_AddLog(strlineorg, FALSE, 2);
			//sprintf(strlineorg, "");
			break;
		}
		else if(isFound_unLock == TRUE)
		{
			//TRACE(strlineorg);
			_AddLog(strlineorg, FALSE, 1);
			break;
		}
		else if(isFound_Detect == TRUE && isReadyFound75 == FALSE)
		{
			//TRACE(strlineorg);
			_AddLog(strlineorg, FALSE);
			//sprintf(strlineorg, "");
			isReadyFound75 = TRUE;
		}
		else if(isFound_Score75 == TRUE && isAlreadyFound75 == FALSE)
		{
			//TRACE(strlineorg);
			_AddLog(strlineorg, FALSE);
			//sprintf(strlineorg, "");
			isAlreadyFound75 = TRUE;
		}
	}

	/*	
	sprintf(strmax, save_strTimeStamp_Start);
	
	if(strlen(save_strTimeStamp_Detect) > 1)
		sprintf(strmax, "%s,%s", strmax, save_strTimeStamp_Detect);
	if(strlen(save_strTimeStampScore75) > 1)
		sprintf(strmax, "%s,%s", strmax, save_strTimeStampScore75);
	if(strlen(save_strTimeStamp_Capture) > 1)
		sprintf(strmax, "%s,%s", strmax, save_strTimeStamp_Capture);
	if(strlen(save_strTimeStampError) > 1)
		sprintf(strmax, "%s,%s", strmax, save_strTimeStampError);

	_AddLog(strmax, TRUE);
	*/

	time_start = _GetTimeStampToInt(save_strTimeStamp_Start);
	time_detect = _GetTimeStampToInt(save_strTimeStamp_Detect);
	time_score75 = _GetTimeStampToInt(save_strTimeStampScore75);
	time_capture = _GetTimeStampToInt(save_strTimeStamp_Capture);
	time_error = _GetTimeStampToInt(save_strTimeStampError);

	if(time_score75 != 0 && time_detect != 0)
		time_diff_detect_to_score75 = time_score75 - time_detect;
	else
		time_diff_detect_to_score75 = 0;

	if(time_capture != 0 && time_detect != 0)
		time_diff_detect_to_capture = time_capture - time_detect;
	else
		time_diff_detect_to_capture = 0;

	total_capture_count++;

	if(time_diff_detect_to_score75 != 0)
	{
		total_capture_count_score75++;
		avg_time_diff_detect_to_score75 += time_diff_detect_to_score75;

		if(min_time_diff_detect_to_score75 > time_diff_detect_to_score75)
			min_time_diff_detect_to_score75 = time_diff_detect_to_score75;
		if(max_time_diff_detect_to_score75 < time_diff_detect_to_score75)
			max_time_diff_detect_to_score75 = time_diff_detect_to_score75;
	}

	if(time_diff_detect_to_capture != 0)
	{
		total_capture_count_capture++;
		avg_time_diff_detect_to_capture += time_diff_detect_to_capture;

		if(min_time_diff_detect_to_capture > time_diff_detect_to_capture)
			min_time_diff_detect_to_capture = time_diff_detect_to_capture;
		if(max_time_diff_detect_to_capture < time_diff_detect_to_capture)
			max_time_diff_detect_to_capture = time_diff_detect_to_capture;
	}

	sprintf(result, ">>> Detect -> Score75 (%.3f sec), Detect -> onCapture (%.3f sec)", 
		 time_diff_detect_to_score75/1000.0f, time_diff_detect_to_capture/1000.0f);

	if(isFound_Error == FALSE && isFound_unLock == FALSE)
		_AddLog(result, TRUE);
	else
		_AddLog("", FALSE);

	if(strlen(strlineorg) > 10)
	{
		sprintf(strlineorg, "");
		goto start_again;
	}

done:

	if(total_capture_count_score75 > 0)
	{
		avg_time_diff_detect_to_score75 /= total_capture_count_score75;
	}
	else
	{
		avg_time_diff_detect_to_score75 = 0;
		min_time_diff_detect_to_score75 = 0;
		max_time_diff_detect_to_score75 = 0;
	}
	
	if(total_capture_count_capture > 0)
	{
		avg_time_diff_detect_to_capture /= total_capture_count_capture;
	}
	else
	{
		avg_time_diff_detect_to_capture = 0;
		min_time_diff_detect_to_capture = 0;
		max_time_diff_detect_to_capture = 0;
	}

	sprintf(result, "Total StartImaging #(%d), Score75 #(%d), Capture #(%d)\r\n\
avgScore75(%.3f sec), minScore75(%.3f sec), maxScore75(%.3f sec)\r\n\
avgCapture(%.3f sec), minCapture(%.3f sec), maxCapture(%.3f sec)",
				total_capture_count, total_capture_count_score75, total_capture_count_capture,
				avg_time_diff_detect_to_score75/1000.0f, min_time_diff_detect_to_score75/1000.0f, max_time_diff_detect_to_score75/1000.0f,
				avg_time_diff_detect_to_capture/1000.0f, min_time_diff_detect_to_capture/1000.0f, max_time_diff_detect_to_capture/1000.0f); 
	GetDlgItem(IDC_EDIT_SUMMARY)->SetWindowText(result);
}

void CCBPLogViewerDlg::_AalysisLog_rev2(FILE *fp)
{
	int StartImagingCount = 0;
	int TokenCount=0;
	char strlineorg[512], strline[512], *pch;
	char save_strTimeStamp[512], save_EnterExit[512];
	char save_strTimeStamp_Start[512];
	char save_strTimeStamp_Detect[512];
	char save_strTimeStamp_Score75[512];
	char save_strTimeStamp_Capture[512];
	char save_strTimeStamp_onCapture[512];
	char save_strTimeStamp_Unlock[512];
	char save_strTimeStamp_Stop[512];
	char save_strTimeStamp_onStopPreview[512];
	char save_strTimeStampError[512], save_strError[512], strmax[2048];
	char delimit[]=" \t\r\n\v\f"; 
	char result[512];
	BOOL isFound_Start = FALSE, isFound_Stop, isFound_onStopPreview, isFound_unLock, isFound_Detect;
	BOOL isReadyFound75, isAlreadyFound75, isFound_Score75, isFound_Capture, isFound_onCapture, isFound_Error;
	BOOL isAlreadyStop, isAlreadyCapture, isAlreadyonStopPreview;

	long time_start, time_stopimaging, time_stoppreview, time_score25, time_score75, time_capture, time_oncapture, time_error;
	long elapsed_time_score25_to_score75, elapsed_time_score25_to_score75_min, elapsed_time_score25_to_score75_max, elapsed_time_score25_to_score75_avg;
	long elapsed_time_score25_to_oncapture, elapsed_time_score25_to_oncapture_min, elapsed_time_score25_to_oncapture_max, elapsed_time_score25_to_oncapture_avg;
	long elapsed_time_score75_to_stopimaging, elapsed_time_score75_to_stopimaging_min, elapsed_time_score75_to_stopimaging_max, elapsed_time_score75_to_stopimaging_avg;
	long elapsed_time_stopimaging_to_onstoppreview, elapsed_time_stopimaging_to_onstoppreview_min, elapsed_time_stopimaging_to_onstoppreview_max, elapsed_time_stopimaging_to_onstoppreview_avg;
	long elapsed_time_capture_to_oncapture, elapsed_time_capture_to_oncapture_min, elapsed_time_capture_to_oncapture_max, elapsed_time_capture_to_oncapture_avg;
	long total_capture_count, total_capture_count_capture;
	long total_count_score25_to_score75, total_count_score25_to_oncapture, total_count_score75_to_stopimaging, total_count_stopimaging_to_onstoppreview, total_count_capture_to_oncapture;

	int TotalLineCount = 0, CurrentLineNumber = 0;
	while(fgets ( strlineorg, 512, fp ) != NULL)
	{
		TotalLineCount++;
	}

	m_progressLog.SetRange(0, TotalLineCount);
	m_progressLog.SetPos(0);

	fseek(fp, 0L, SEEK_SET);

	total_capture_count = 0;
	total_capture_count_capture = 0;
	total_count_score25_to_score75 = 0;
	total_count_score25_to_oncapture = 0;
	total_count_score75_to_stopimaging = 0;
	total_count_stopimaging_to_onstoppreview = 0;
	total_count_capture_to_oncapture = 0;
	
	elapsed_time_score25_to_score75 = 0;
	elapsed_time_score25_to_score75_min = 100000;
	elapsed_time_score25_to_score75_max = -100000;
	elapsed_time_score25_to_score75_avg = 0;
	
	elapsed_time_score25_to_oncapture = 0;
	elapsed_time_score25_to_oncapture_min = 100000;
	elapsed_time_score25_to_oncapture_max = -100000;
	elapsed_time_score25_to_oncapture_avg = 0;

	elapsed_time_score75_to_stopimaging = 0;
	elapsed_time_score75_to_stopimaging_min = 100000;
	elapsed_time_score75_to_stopimaging_max = -100000;
	elapsed_time_score75_to_stopimaging_avg = 0;

	elapsed_time_stopimaging_to_onstoppreview = 0;
	elapsed_time_stopimaging_to_onstoppreview_min = 100000;
	elapsed_time_stopimaging_to_onstoppreview_max = -100000;
	elapsed_time_stopimaging_to_onstoppreview_avg = 0;

	elapsed_time_capture_to_oncapture = 0;
	elapsed_time_capture_to_oncapture_min = 100000;
	elapsed_time_capture_to_oncapture_max = -100000;
	elapsed_time_capture_to_oncapture_avg = 0;

start_again:

	// Find StartImaing
	isFound_Start = FALSE;
	sprintf(save_strTimeStamp_Start, "");
	sprintf(save_strTimeStamp_Detect, "");
	sprintf(save_strTimeStamp_Score75, "");
	sprintf(save_strTimeStamp_Capture, "");
	sprintf(save_strTimeStamp_onCapture, "");
	sprintf(save_strTimeStamp_Unlock, "");
	sprintf(save_strTimeStamp_Stop, "");
	sprintf(save_strTimeStamp_onStopPreview, "");
	sprintf(save_strTimeStampError, "");
	sprintf(save_strError, "");
	sprintf(strmax, "");

	while(fgets ( strlineorg, 512, fp ) != NULL)
	{
		CurrentLineNumber++;
		m_progressLog.SetPos(CurrentLineNumber);
		
		TokenCount = 0;

		sprintf(strline, "%s", strlineorg);
		pch = strtok (strline, delimit);
		while (pch != NULL)
		{
			if(TokenCount == 1)
			{
				sprintf(save_strTimeStamp, "%s", pch);
				//strncat(save_strTimeStamp, pch, 12);
				save_strTimeStamp[strlen(save_strTimeStamp)-1]='\0';
			}
			else if(TokenCount == 3 && strcmp(pch, "cbp_fp_startImaging") == 0)
			{
				isFound_Start = TRUE;
				sprintf(save_strTimeStamp_Start, "%s", save_strTimeStamp);
				break;
			}

			pch = strtok (NULL, delimit);

			TokenCount++;
		}

		if(isFound_Start == TRUE)
		{
			//TRACE(strlineorg);
			_AddLog(strlineorg, FALSE);
			break;
		}
	}

	if(isFound_Start == FALSE)
		goto done;

	// Find Stop or UnLock
	isFound_Capture = FALSE;	// cbp_fp_capture
	isFound_onCapture = FALSE;	// cbp_fp_onCapture
	isFound_Stop = FALSE;		// cbp_fp_stopImaging
	isFound_onStopPreview = FALSE;		// cbp_fp_onStopPreview
	isFound_Error = FALSE;
	isFound_unLock = FALSE;		// cbp_fp_unlock
	isFound_Detect = FALSE;		// cbp_fp_scorelow
	isReadyFound75 = FALSE;
	isFound_Score75 = FALSE;	// cbp_fp_score75
	isAlreadyFound75 = FALSE;
	isAlreadyCapture = FALSE;
	isAlreadyStop = FALSE;
	isAlreadyonStopPreview = FALSE;

	while(fgets ( strlineorg, 512, fp ) != NULL)
	{
		CurrentLineNumber++;
		m_progressLog.SetPos(CurrentLineNumber);

		TokenCount = 0;

		sprintf(strline, "%s", strlineorg);
		sprintf(save_EnterExit, "");
		pch = strtok (strline, delimit);
		while (pch != NULL)
		{
			if(TokenCount == 1)
			{
				sprintf(save_strTimeStamp, "%s", pch);
				//strncat(save_strTimeStamp, pch, 12);
				save_strTimeStamp[strlen(save_strTimeStamp)-1]='\0';
			}
			else if(TokenCount == 3)
			{
				if(strcmp(pch, "cbp_fp_unlock") == 0 && isFound_unLock == FALSE)
				{
					isFound_unLock = TRUE;
					sprintf(save_strTimeStamp_Unlock, "%s", save_strTimeStamp);
					//break;
				}
				else if(strcmp(pch, "cbp_fp_stopImaging") == 0 && isFound_Stop == FALSE)
				{
					isFound_Stop = TRUE;
					sprintf(save_strTimeStamp_Stop, "%s", save_strTimeStamp);
					//break;
				}
				else if(strcmp(pch, "cbp_fp_onStopPreview") == 0 && isFound_onStopPreview == FALSE)
				{
					isFound_onStopPreview = TRUE;
					sprintf(save_strTimeStamp_onStopPreview, "%s", save_strTimeStamp);
					//break;
				}
				else if(strcmp(pch, "cbp_fp_capture") == 0 && isFound_Capture == FALSE)
				{
					isFound_Capture = TRUE;
					sprintf(save_strTimeStamp_Capture, "%s", save_strTimeStamp);
					//break;
				}
				else if(strcmp(pch, "cbp_fp_onCapture") == 0 && isFound_onCapture == FALSE)
				{
					isFound_onCapture = TRUE;
					sprintf(save_strTimeStamp_onCapture, "%s", save_strTimeStamp);
					total_capture_count_capture++;
					//break;
				}
				else if(strcmp(pch, "callback_error") == 0 && isFound_Error == FALSE)
				{
					isFound_Error = TRUE;
					sprintf(save_strTimeStampError, "%s", save_strTimeStamp);
					//break;
				}
			}
			else if(TokenCount == 9 && strcmp(pch, "75") != 0 && isFound_Detect == FALSE && isFound_Score75 == FALSE)
			{
				isFound_Detect = TRUE;
				sprintf(save_strTimeStamp_Detect, "%s", save_strTimeStamp);
				//break;
			}
			else if(TokenCount == 9 && strcmp(pch, "75") == 0 && isReadyFound75 == TRUE && isFound_Score75 == FALSE)
			{
				isFound_Score75 = TRUE;
				sprintf(save_strTimeStamp_Score75, "%s", save_strTimeStamp);
				//break;
			}
			
			pch = strtok (NULL, delimit);

			TokenCount++;
		}

		if(isFound_Capture == TRUE && isAlreadyCapture == FALSE)
		{
			_AddLog(strlineorg, FALSE);
			isAlreadyCapture = TRUE;
//			break;
		}
		else if(isFound_Error == TRUE)
		{
			_AddLog(strlineorg, FALSE, 2);
			break;
		}
		else if(isFound_unLock == TRUE)
		{
			_AddLog(strlineorg, FALSE, 1);
			break;
		}
		else if(isFound_onCapture == TRUE)
		{
			_AddLog(strlineorg, FALSE);
			break;
		}
		else if(isFound_Stop == TRUE && isAlreadyStop == FALSE)
		{
			_AddLog(strlineorg, FALSE);
			isAlreadyStop = TRUE;
//			break;
		}
		else if(isFound_onStopPreview == TRUE && isAlreadyonStopPreview == FALSE)
		{
			_AddLog(strlineorg, FALSE);
			isAlreadyonStopPreview = TRUE;
//			break;
		}
		else if(isFound_Detect == TRUE && isReadyFound75 == FALSE)
		{
			_AddLog(strlineorg, FALSE);
			isReadyFound75 = TRUE;
		}
		else if(isFound_Score75 == TRUE && isAlreadyFound75 == FALSE)
		{
			_AddLog(strlineorg, FALSE);
			isAlreadyFound75 = TRUE;
		}
	}

	time_start = _GetTimeStampToInt(save_strTimeStamp_Start);
	time_stopimaging = _GetTimeStampToInt(save_strTimeStamp_Stop);
	time_stoppreview = _GetTimeStampToInt(save_strTimeStamp_onStopPreview);
	time_score25 = _GetTimeStampToInt(save_strTimeStamp_Detect);
	time_score75 = _GetTimeStampToInt(save_strTimeStamp_Score75);
	time_capture = _GetTimeStampToInt(save_strTimeStamp_Capture);
	time_oncapture = _GetTimeStampToInt(save_strTimeStamp_onCapture);
	time_error = _GetTimeStampToInt(save_strTimeStampError);

	// elapse_time between Score25 and Score75
	if(time_score75 != 0 && time_score25 != 0)
		elapsed_time_score25_to_score75 = time_score75 - time_score25;
	else
		elapsed_time_score25_to_score75 = 0;

	// elapse_time between Score25 and StopImaging
	if(time_stopimaging != 0 && time_score75 != 0)
		elapsed_time_score75_to_stopimaging = time_stopimaging - time_score75;
	else
		elapsed_time_score75_to_stopimaging = 0;

	// elapse_time between Score25 and onCapture
	if(time_stoppreview != 0 && time_stopimaging != 0)
		elapsed_time_stopimaging_to_onstoppreview = time_stoppreview - time_stopimaging;
	else
		elapsed_time_stopimaging_to_onstoppreview = 0;

	// elapse_time between Score25 and onCapture
	if(time_capture != 0 && time_score25 != 0)
		elapsed_time_score25_to_oncapture = time_capture - time_score25;
	else
		elapsed_time_score25_to_oncapture = 0;

	// elapse_time between Score25 and onCapture
	if(time_capture != 0 && time_oncapture != 0)
		elapsed_time_capture_to_oncapture = time_oncapture - time_capture;
	else
		elapsed_time_capture_to_oncapture = 0;

	total_capture_count++;

	if(elapsed_time_score25_to_score75 != 0)
	{
		total_count_score25_to_score75++;
		elapsed_time_score25_to_score75_avg += elapsed_time_score25_to_score75;

		if(elapsed_time_score25_to_score75_min > elapsed_time_score25_to_score75)
			elapsed_time_score25_to_score75_min = elapsed_time_score25_to_score75;
		if(elapsed_time_score25_to_score75_max < elapsed_time_score25_to_score75)
			elapsed_time_score25_to_score75_max = elapsed_time_score25_to_score75;
	}

	if(elapsed_time_score25_to_oncapture != 0)
	{
		total_count_score25_to_oncapture++;
		elapsed_time_score25_to_oncapture_avg += elapsed_time_score25_to_oncapture;

		if(elapsed_time_score25_to_oncapture_min > elapsed_time_score25_to_oncapture)
			elapsed_time_score25_to_oncapture_min = elapsed_time_score25_to_oncapture;
		if(elapsed_time_score25_to_oncapture_max < elapsed_time_score25_to_oncapture)
			elapsed_time_score25_to_oncapture_max = elapsed_time_score25_to_oncapture;
	}

	if(elapsed_time_score75_to_stopimaging != 0)
	{
		total_count_score75_to_stopimaging++;
		elapsed_time_score75_to_stopimaging_avg += elapsed_time_score75_to_stopimaging;

		if(elapsed_time_score75_to_stopimaging_min > elapsed_time_score75_to_stopimaging)
			elapsed_time_score75_to_stopimaging_min = elapsed_time_score75_to_stopimaging;
		if(elapsed_time_score75_to_stopimaging_max < elapsed_time_score75_to_stopimaging)
			elapsed_time_score75_to_stopimaging_max = elapsed_time_score75_to_stopimaging;
	}

	if(elapsed_time_stopimaging_to_onstoppreview != 0 && time_error == 0)
	{
		total_count_stopimaging_to_onstoppreview++;
		elapsed_time_stopimaging_to_onstoppreview_avg += elapsed_time_stopimaging_to_onstoppreview;

		if(elapsed_time_stopimaging_to_onstoppreview_min > elapsed_time_stopimaging_to_onstoppreview)
			elapsed_time_stopimaging_to_onstoppreview_min = elapsed_time_stopimaging_to_onstoppreview;
		if(elapsed_time_stopimaging_to_onstoppreview_max < elapsed_time_stopimaging_to_onstoppreview)
			elapsed_time_stopimaging_to_onstoppreview_max = elapsed_time_stopimaging_to_onstoppreview;
	}

	if(elapsed_time_capture_to_oncapture != 0 && time_error == 0)
	{
		total_count_capture_to_oncapture++;
		elapsed_time_capture_to_oncapture_avg += elapsed_time_capture_to_oncapture;

		if(elapsed_time_capture_to_oncapture_min > elapsed_time_capture_to_oncapture)
			elapsed_time_capture_to_oncapture_min = elapsed_time_capture_to_oncapture;
		if(elapsed_time_capture_to_oncapture_max < elapsed_time_capture_to_oncapture)
			elapsed_time_capture_to_oncapture_max = elapsed_time_capture_to_oncapture;
	}

	if(isFound_Error == FALSE && isFound_unLock == FALSE)
	{
		sprintf(result, ">>> Score25 -> Score75 (%.3f sec)", elapsed_time_score25_to_score75/1000.0f);
		_AddLog(result, FALSE);

		sprintf(result, ">>> Score25 -> onCapture (%.3f sec)", elapsed_time_score25_to_oncapture/1000.0f);
		_AddLog(result, FALSE);

		sprintf(result, ">>> Score75 -> stopImaging (%.3f sec)", elapsed_time_score75_to_stopimaging/1000.0f);
		_AddLog(result, FALSE);

		sprintf(result, ">>> stopImaging -> onStopPreview (%.3f sec)", elapsed_time_stopimaging_to_onstoppreview/1000.0f);
		_AddLog(result, FALSE);
	
		sprintf(result, ">>> capture -> onCapture (%.3f sec)", elapsed_time_capture_to_oncapture/1000.0f);
		_AddLog(result, TRUE);
	}
	else
		_AddLog("", FALSE);

	if(strlen(strlineorg) > 10)
	{
		sprintf(strlineorg, "");
		goto start_again;
	}

done:

	m_progressLog.SetPos(TotalLineCount);

	if(total_count_score25_to_score75 > 0)
	{
		elapsed_time_score25_to_score75_avg /= total_count_score25_to_score75;
	}
	else
	{
		elapsed_time_score25_to_score75_avg = 0;
		elapsed_time_score25_to_score75_min = 0;
		elapsed_time_score25_to_score75_max = 0;
	}
	
	if(total_count_score25_to_oncapture > 0)
	{
		elapsed_time_score25_to_oncapture_avg /= total_count_score25_to_oncapture;
	}
	else
	{
		elapsed_time_score25_to_oncapture_avg = 0;
		elapsed_time_score25_to_oncapture_min = 0;
		elapsed_time_score25_to_oncapture_max = 0;
	}

	if(total_count_score75_to_stopimaging > 0)
	{
		elapsed_time_score75_to_stopimaging_avg /= total_count_score75_to_stopimaging;
	}
	else
	{
		elapsed_time_score75_to_stopimaging_avg = 0;
		elapsed_time_score75_to_stopimaging_min = 0;
		elapsed_time_score75_to_stopimaging_max = 0;
	}

	if(total_count_stopimaging_to_onstoppreview > 0)
	{
		elapsed_time_stopimaging_to_onstoppreview_avg /= total_count_stopimaging_to_onstoppreview;
	}
	else
	{
		elapsed_time_stopimaging_to_onstoppreview_avg = 0;
		elapsed_time_stopimaging_to_onstoppreview_min = 0;
		elapsed_time_stopimaging_to_onstoppreview_max = 0;
	}

	if(total_count_capture_to_oncapture > 0)
	{
		elapsed_time_capture_to_oncapture_avg /= total_count_capture_to_oncapture;
	}
	else
	{
		elapsed_time_capture_to_oncapture_avg = 0;
		elapsed_time_capture_to_oncapture_min = 0;
		elapsed_time_capture_to_oncapture_max = 0;
	}

	sprintf(result, "Total StartImaging #(%d), Score75 #(%d), Capture #(%d)\r\n\
Score25 ~ Score75 : Avg(%.3f sec), Min(%.3f sec), Max(%.3f sec)\r\n\
Score25 ~ onCapture : Avg(%.3f sec), Min(%.3f sec), Max(%.3f sec)\r\n\
Score75 ~ StopImaging : Avg(%.3f sec), Min(%.3f sec), Max(%.3f sec)\r\n\
StopImaging ~ onStopPreview : Avg(%.3f sec), Min(%.3f sec), Max(%.3f sec)\r\n\
Capture ~ onCapture : Avg(%.3f sec), Min(%.3f sec), Max(%.3f sec)",
				total_capture_count, total_count_score25_to_score75, total_capture_count_capture,
				elapsed_time_score25_to_score75_avg/1000.0f, elapsed_time_score25_to_score75_min/1000.0f, elapsed_time_score25_to_score75_max/1000.0f,
				elapsed_time_score25_to_oncapture_avg/1000.0f, elapsed_time_score25_to_oncapture_min/1000.0f, elapsed_time_score25_to_oncapture_max/1000.0f,
				elapsed_time_score75_to_stopimaging_avg/1000.0f, elapsed_time_score75_to_stopimaging_min/1000.0f, elapsed_time_score75_to_stopimaging_max/1000.0f,
				elapsed_time_stopimaging_to_onstoppreview_avg/1000.0f, elapsed_time_stopimaging_to_onstoppreview_min/1000.0f, elapsed_time_stopimaging_to_onstoppreview_max/1000.0f,
				elapsed_time_capture_to_oncapture_avg/1000.0f, elapsed_time_capture_to_oncapture_min/1000.0f, elapsed_time_capture_to_oncapture_max/1000.0f);
	GetDlgItem(IDC_EDIT_SUMMARY)->SetWindowText(result);

	m_progressLog.SetPos(0);
}

void CCBPLogViewerDlg::_AddLog(char *str, BOOL newline, int error)
{
	int nIndex = m_listFilteredLog.GetItemCount();
	m_listFilteredLog.InsertItem(nIndex, "");
	//m_listFilteredLog.SetItemText(nIndex, 0, str);

	CBPLogs log;
	log.text = str;
	log.error_flag = 0;
	m_database.push_back(log);

	if(error > 0)
	{
		COLORREF color;
		
		if(error == 1)
			color = RGB(128,128,255);
		else
			color = RGB(255,128,128);
		
		for(int i=m_database.size()-1; i>=0; i--)
		{
			if(strlen((char*)m_database[i].text.GetBuffer()) > 2)
			{
				m_database[i].error_flag = error;
			}
			else
				break;
		}
	}

	if(newline)
	{
		m_listFilteredLog.InsertItem(nIndex, "");
		CBPLogs log;
		log.text = "";
		log.error_flag = 0;
		m_database.push_back(log);
	}

	m_EndofList = m_database.size();

	//m_listFilteredLog.SetTopIndex(m_listFilteredLog.GetItemCount()-1);
}

long CCBPLogViewerDlg::_GetTimeStampToInt(char *str)
{
	int length = strlen(str);
	int hour, min, sec, msec;
	char str_hour[4], str_min[4], str_sec[4], str_msec[4];

	if(length == 0)
		return 0;

	memcpy(str_hour, &str[0], 2);
	str_hour[2] = '\0';
	hour = atoi(str_hour);

	memcpy(str_min, &str[3], 2);
	str_min[2] = '\0';
	min = atoi(str_min);

	memcpy(str_sec, &str[6], 2);
	str_sec[2] = '\0';
	sec = atoi(str_sec);

	memcpy(str_msec, &str[9], 3);
	str_msec[3] = '\0';
	msec = atoi(str_msec);

	return (hour * 60 * 60 * 1000) + (min * 60 * 1000) + (sec * 1000) + msec;
}

void CCBPLogViewerDlg::OnBnClickedButtonSaveBriefLog()
{
	CFileDialog dlg(FALSE, 0, 0, 6, "log file(*.log)|*.log||");

	if(dlg.DoModal() == IDOK)
	{
		CString filename = dlg.GetPathName();
		filename.MakeLower();

		if(filename.Find(".log") > 0)
		{
			filename = dlg.GetPathName();
		}
		else
		{
			filename = dlg.GetPathName() + ".log";
		}

		FILE *fp = fopen((char*)filename.GetBuffer(), "wt");
		if(fp == NULL)
			return;

		CString cstr;
		char str_item[512];
		GetDlgItem(IDC_EDIT_SUMMARY)->GetWindowTextA(str_item, 512);
		fprintf(fp, "%s\n\n", str_item);

		for(int i=0; i<m_listFilteredLog.GetItemCount(); i++)
		{
			cstr = m_listFilteredLog.GetItemText(i, 1);
			fprintf(fp, "%s\n", (char*)cstr.GetBuffer());
		}

		fclose(fp);
	}
}

void CCBPLogViewerDlg::OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	LV_ITEM* pItem= &(pDispInfo)->item;

	if(pItem == NULL) return;

	int nRow = pItem->iItem;
	int nCol = pItem->iSubItem;

	if (nRow<0 || nRow >= m_EndofList)
		return;

	CString text, error_flag;

	if(pItem->iSubItem == 0)
	{
		if(m_database[nRow].error_flag == 1)
			error_flag = "CAN";
		else if(m_database[nRow].error_flag == 2)
			error_flag = "ERR";
		else
			error_flag = "";
	}
	else if(pItem->iSubItem == 1)
		text = m_database[nRow].text;

	if(pItem->pszText)
	{
		switch(nCol)
		{
		case 0: 
			lstrcpy(pItem->pszText, error_flag); 
			break;
		case 1: 
			lstrcpy(pItem->pszText, text); 
			break;
		}
	}
	*pResult = 0;
}

void CCBPLogViewerDlg::OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );
	*pResult = 0;

	int error_code = 0;

	CString error_str = m_listFilteredLog.GetItemText(pLVCD->nmcd.dwItemSpec, 0);

	if(error_str.Find("CAN") != -1)
	{
		error_code = 1;
	}
	else if(error_str.Find("ERR") != -1)
	{
		error_code = 2;
	}

	// First thing - check the draw stage. If it's the control's prepaint
	// stage, then tell Windows we want messages for every item.
	if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		COLORREF crText;

		if ( error_code == 0 )
			crText = RGB(0,0,0);
		else if ( error_code == 1 )
			crText = RGB(128,128,255);
		else
			crText = RGB(255,128,128);

		// Store the color back in the NMLVCUSTOMDRAW struct.
		pLVCD->clrText = crText;

		// Tell Windows to paint the control itself.
		*pResult = CDRF_DODEFAULT;
	}
}

