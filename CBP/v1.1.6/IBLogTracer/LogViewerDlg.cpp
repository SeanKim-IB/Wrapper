
// LogViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LogViewer.h"
#include "LogViewerDlg.h"

#include "PipeData.h"
#include "ServerPipeInstance.h"
#include "LogMap.h"

#include "MaxDlg.h"
#include "ClearWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HANDLE m_hStopEvent;

CRITICAL_SECTION g_CriticalSection;

extern CString e_maxSize;
extern CString e_clearTerm;

// CLogViewerDlg dialog

CLogViewerDlg::CLogViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogViewerDlg::IDD, pParent)
	, m_chkStatusBar(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	hThread = NULL;
	m_hStopEvent = NULL;
	m_clearCount = 0;
	m_test = 0;
	m_fp = NULL;
	m_DBLogCount = 0;

	InitializeCriticalSection(&g_CriticalSection);

	m_database.clear();
}

void CLogViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG_LIST, m_LogList);
	//DDX_Check(pDX, IDC_CHECK_STATUSBAR, m_chkStatusBar);
//	DDX_Control(pDX, IDC_BUTTON_CLEARWINDOW, m_btnClearWindow);
}

BEGIN_MESSAGE_MAP(CLogViewerDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	//ON_BN_CLICKED(IDC_MAKE_LISTEN_PIPE, &CLogViewerDlg::OnBnClickedMakeListenPipe)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LOG_LIST, &CLogViewerDlg::OnGetdispinfoList)
	ON_NOTIFY(LVN_ODFINDITEM, IDC_LOG_LIST, &CLogViewerDlg::OnOdfinditemList)
	ON_NOTIFY(LVN_ODCACHEHINT, IDC_LOG_LIST, &CLogViewerDlg::OnOdcachehintList)
	ON_MESSAGE(MYWM_NOTIFYICON, onTrayNotify)
	ON_WM_SYSCOMMAND()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_COMMAND(ID_MENU_EXIT, &CLogViewerDlg::OnMenuExit)
	ON_COMMAND(ID_MENU_SHOW, &CLogViewerDlg::OnMenuShow)
	//ON_BN_CLICKED(IDC_CHECK_STATUSBAR, &CLogViewerDlg::OnBnClickedCheckStatusbar)
	//ON_BN_CLICKED(IDC_BUTTON_CLEARWINDOW, &CLogViewerDlg::OnBnClickedButtonClearwindow)
//	ON_BN_CLICKED(IDC_BUTTON_OPENFOLDER, &CLogViewerDlg::OnBnClickedButtonOpenfolder)
	ON_WM_SIZE()
	ON_COMMAND(ID_MENU_CLEARWINDOW, &CLogViewerDlg::OnMenuClearwindow)
	ON_COMMAND(ID_MENU_OPENFILEFOLDER, &CLogViewerDlg::OnMenuOpenfilefolder)
	ON_COMMAND(ID_MENU_SHOWNUMBEROFITEM, &CLogViewerDlg::OnMenuShownumberofitem)
	ON_COMMAND(ID_MENU_LOGFILES, &CLogViewerDlg::OnMenuLogfiles)
	ON_COMMAND(ID_MENU_OPTIONFORCLEARWINDOW, &CLogViewerDlg::OnMenuOptionforclearwindow)
	ON_UPDATE_COMMAND_UI(ID_MENU_LOGFILES, &CLogViewerDlg::OnUpdateMenuLogfiles)
END_MESSAGE_MAP()


// CLogViewerDlg message handlers

BOOL CLogViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	//SetWindowPos(NULL, 0,0, 1024, 470, SWP_NOZORDER);


	HICON hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_NEW) );
    this->SetIcon( hIcon, FALSE );

	GetCurrentDirectory(MAX_PATH, CurPath);

	CRect rect;
	m_LogList.GetWindowRect(&rect);
//	m_LogList.InsertColumn(LOGVIEWER::COLUMN_IMAGE, _T(""), LVCFMT_IMAGE, 60, LOGVIEWER::COLUMN_IMAGE);
	m_LogList.InsertColumn(LOGVIEWER::COLUMN_DATE, _T("Timestamp"), LVCFMT_CENTER, 150, LOGVIEWER::COLUMN_DATE);
	m_LogList.InsertColumn(LOGVIEWER::COLUMN_CATEGORY, _T("Category"), LVCFMT_CENTER, 150, LOGVIEWER::COLUMN_CATEGORY);
	//m_LogList.InsertColumn(LOGVIEWER::COLUMN_LOG_CODE, _T("LogCode"), LVCFMT_CENTER, 60, LOGVIEWER::COLUMN_LOG_CODE);
	m_LogList.InsertColumn(LOGVIEWER::COLUMN_ERROR, _T("Error"), LVCFMT_CENTER, 60, LOGVIEWER::COLUMN_ERROR);
	m_LogList.InsertColumn(LOGVIEWER::COLUMN_THREAD, _T("Thread"), LVCFMT_CENTER, 100, LOGVIEWER::COLUMN_THREAD);
	//m_LogList.InsertColumn(LOGVIEWER::COLUMN_MESSAGE, _T("Message"), LVCFMT_LEFT, rect.Width() - 600, LOGVIEWER::COLUMN_MESSAGE);
	m_LogList.InsertColumn(LOGVIEWER::COLUMN_MESSAGE, _T("Message"), LVCFMT_LEFT, rect.Width(), LOGVIEWER::COLUMN_MESSAGE);
	m_LogList.SetExtendedStyle(m_LogList.GetExtendedStyle() | LVS_EX_FULLROWSELECT );

	m_temp = 0;
	m_count = 0;
	e_maxSize = "20000";
	e_clearTerm = "50000";


	//GetFileName();

	//m_fp = fopen(m_filePath, "at");

	TraySetting();

	m_TrayMenu.LoadMenu(IDR_MENU);

	//GetDlgItem(IDC_STATIC)->ShowWindow(false);
	//GetDlgItem(IDC_LOG_COUNT)->ShowWindow(false);

	MakePipe();

	//GetFileName();
	//m_fp = fopen(m_filePath, "at");

	PostMessage(WM_CLOSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLogViewerDlg::TraySetting(void)
{
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(nid);
	nid.hWnd = m_hWnd;
	//nid.uID = IDI_TRAY;
	nid.uID = IDI_ICON_NEW;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	//nid.uCallbackMessage = WM_TRAYICON_MSG;
	nid.uCallbackMessage = MYWM_NOTIFYICON;
	nid.hIcon = AfxGetApp()->LoadIcon(IDI_ICON_NEW);

	char strTitle[256];
	GetWindowText(strTitle, sizeof(strTitle));
	lstrcpy(nid.szTip, strTitle);
	Shell_NotifyIcon(NIM_ADD, &nid);
	SendMessage(WM_SETICON, (WPARAM)TRUE, (LPARAM)nid.hIcon);
	//m_bTrayStatus = true;
}

void CLogViewerDlg::GetFileName()
{
	CTime t = CTime::GetCurrentTime();

	int year = t.GetYear();
	int month = t.GetMonth();
	int day = t.GetDay();
	int hour = t.GetHour();
	int minute = t.GetMinute();

	CString y, m, d, h, n;
	//CString ext = "_log.txt";
	CString ext = ".log";

	y.Format("%04d", year);
	m.Format("%02d", month);
	d.Format("%02d", day);
	h.Format("%02d", hour);
	n.Format("%02d", minute);

	m_fileName += "_"+y+m+d+h+n+ext;

	m_filePath.Format("%s\\%s", CurPath, m_fileName);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLogViewerDlg::OnPaint()
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
HCURSOR CLogViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CLogViewerDlg::MakePipe()
{
	// make listen pipe

	//EnterCriticalSection(&g_CriticalSection);
	m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	if (m_hStopEvent != INVALID_HANDLE_VALUE)
	{
		//	We were able to create our events so go ahead...	
		hThread = HANDLE(_beginthread(LogListenerThreadProc, 0, NULL));
	}
	//LeaveCriticalSection(&g_CriticalSection);
}

void CLogViewerDlg::LogListenerThreadProc(LPVOID /*data*/)
{

	CLogViewerDlg *dlg = (CLogViewerDlg *)theApp.GetMainWnd();

	CPipeData			*pd = new CPipeData;
	CServerPipeInstance *pCommandPipe = new CServerPipeInstance,
						*pPipe;
	CLogInstance		*pLog;
	CSleeper			sleeper;
	CLogMap				logMap;

	ASSERT(pd);
	ASSERT_KINDOF(CPipeData, pd);

	ASSERT(pCommandPipe);
	ASSERT_KINDOF(CServerPipeInstance, pCommandPipe);
	
	BOOL  bStop = FALSE;
	DWORD dwRead, dwWaitVal;


	if (pCommandPipe->CreatePipe(NULL, COMMANDPIPENAME))
	{
		pd->AddPipe(pCommandPipe);
		
		while (bStop == FALSE)
		{
			//	Wait for something to happen
			//	We are waiting on two handles plus one handle per named pipe.
			//	Handle 0 is the handle that tells the thread to terminate
			//	Handle 1 is the handle for the command named pipe we use
			//	to create log objects and named pipes.
			//	Handles 2 - n are the pipe event handles (in each pipe's
			//	OVERLAPPED structure).
			dwWaitVal = pd->Wait();

			switch (dwWaitVal)
			{
			case WAIT_FAILED:
			case WAIT_TIMEOUT:
				//	Something went wrong so terminate this thread...
//					_Module.LogEvent(_T("Wait failure in CLogData::LogListenerThread"));
				//	Fall through
				
			case WAIT_OBJECT_0:		//	Stop event
				bStop = TRUE;
				break;

			case WAIT_OBJECT_0 + 1:	//	Command pipe
				if (pCommandPipe->GetOverlappedResult(&dwRead))
				{
					//	Determine the outcome of the operation (connect or read)
					//	If dwRead i 0 it means we completed a connection,
					//	otherwise we completed a read
					if (dwRead)
					{
						pCommandPipe->m_tszBuffer[dwRead] = 0;

						//	Break the string down into command and arguments...
						CString command = _tcstok(pCommandPipe->m_tszBuffer, _T("^")),
								argument = _tcstok(NULL, _T("^"));


						dlg->m_fileName = argument;

						argument += _T(".log");

						if (command.CompareNoCase(_T("createlog")) == 0)
						{
							//	Maximum handle count for WaitForMultipleObjects
							//	is MAXIMUM_WAIT_OBJECTS, so no point in even
							//	trying to honour a request that would take us
							//	past that count.
							if (pd->GetCount() < MAXIMUM_WAIT_OBJECTS)
							{
								pPipe = new CServerPipeInstance;

								if ((pLog = logMap.GetLog(argument)) == (CLogInstance *) NULL)
								{
									//	We don't yet have this logfile 
									//	and we failed to create one
									delete pPipe;
									pPipe = (CServerPipeInstance *) NULL;
//										_Module.LogEvent(_T("Unable to create log instance object for %s"), argument);
								}

								if (pPipe != (CServerPipeInstance *) NULL && pPipe->CreatePipe(pLog, argument))
									pd->AddPipe(pPipe);
							}
//								else
//									_Module.LogEvent(_T("Cannot create pipe %s - already have %d handles"), argument, MAXIMUM_WAIT_OBJECTS);
						}
					}

					pCommandPipe->ResetEvent();
				}
				
				if (GetLastError() == ERROR_BROKEN_PIPE || !pCommandPipe->ReadPipe())
					//	Of course, it could have been a broken pipe
					//	(our client went away).  If so, restart the pipe
					//	instance...
					pCommandPipe->RestartPipe();

				break;

			default:
				//	Be sure that dwWaitVal is a valid pipe reference...
				if (dwWaitVal >= 2 && dwWaitVal < DWORD(pd->GetCount()))
				{
					//	It seems to be valid, get the pipeinstance that
					//	created this notification.
					pPipe = (CServerPipeInstance *) pd->GetPipe(WORD(dwWaitVal));

					ASSERT(pPipe);
					ASSERT_KINDOF(CServerPipeInstance, pPipe);

					if (pPipe->GetOverlappedResult(&dwRead))
					{
						//	Determine the outcome of the operation (connect or read)
						//	If dwRead isn't 0 it means we completed a read,
						//	otherwise we completed a connection
						if (dwRead)
							pPipe->QueueMessage();	//	queue the message...
					}

					pPipe->ResetEvent();

					if (GetLastError() == ERROR_BROKEN_PIPE || !pPipe->ReadPipe())
						//	Of course, it could have been a broken pipe
						//	(our client went away).  If so, remove the pipe
						//	instance...
						pd->RemovePipe(dwWaitVal);
					else
						//	Move this pipe to the end of the event array so the
						//	other pipes have a chance at being serviced.
						pd->RotatePipes(dwWaitVal);
				}

				break;
			}
		}
	}

	//	We're exiting so delete all the objects we created...
	delete pd;
	delete pCommandPipe;

	_endthread();
}

void CLogViewerDlg::_InsertLog(CString str_log)
{

	m_clearCount++;

	EnterCriticalSection(&g_CriticalSection);

	if(m_test==0)
	{
		GetFileName();
		m_fp = fopen(m_filePath, "at");
		m_test++;
	}
	else
	{
	}
	
	int max = _ttoi(e_maxSize);
	int term = _ttoi(e_clearTerm);

	m_count++;
	//m_clearCount++;

	if(m_clearCount>=term)
	{
		TRACE("step 1\n");
		m_clearCount = 0;
		PostMessage(WM_COMMAND, ID_MENU_CLEARWINDOW);
		//OnMenuClearwindow();
	}
	else
	{
	}

	m_DBLogCount = m_database.size()+1;
	m_database.push_back(CLogData(str_log));
	CLogData log(str_log);	
	
	//fprintf(m_fp, "%s %s %s %s %s %s\n", log.m_COLUMN_DATE, log.m_COLUMN_CATEGORY, log.m_COLUMN_LOG_CODE, log.m_COLUMN_ERROR, log.m_COLUMN_THREAD, log.m_COLUMN_MESSAGE);

	fprintf(m_fp, "%s, %s, %s, %s, %s\n", log.m_COLUMN_DATE, log.m_COLUMN_CATEGORY, log.m_COLUMN_ERROR, log.m_COLUMN_THREAD, log.m_COLUMN_MESSAGE);

	if(m_count>=max)
	{
		fclose(m_fp);
		GetFileName();
		m_fp = fopen(m_filePath, "at");
		m_count = 0;
	}
	else
	{
	}

	LeaveCriticalSection(&g_CriticalSection);
	
}

void CLogViewerDlg::_RefreshLog()
{
	//EnterCriticalSection(&g_CriticalSection);

	TRACE("step Enter_RefreshLog \n");

	if(m_Old_Log_Count == m_DBLogCount)
	{
		TRACE("step EnterIfLoop_RefreshLog \n");
		return;
	}
	else
	{
		TRACE("step ExitIfLoop_RefreshLog \n");
	}

	//m_LogList.SetRedraw(FALSE);
	m_Old_Log_Count = m_DBLogCount;
	TRACE("step LogCount_RefreshLog : %d \n", m_Old_Log_Count);
	//m_LogList.SetItemCount(m_Old_Log_Count);
	m_LogList.SetItemCountEx(m_Old_Log_Count,LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
	//m_LogList.SetItemCountEx(150,LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
	TRACE("step SetItemCount_RefreshLog \n");
	//m_LogList.SetRedraw();
	
	CString str;
	str.Format("%d", m_Old_Log_Count);

	GetDlgItem(IDC_LOG_COUNT)->SetWindowText(str);

	CSize size;
	size.cy=(m_LogList.GetItemCount()-1)*15;
	size.cx=0;
	m_LogList.Scroll(size);

	TRACE("step Exit_RefreshLog \n");
	//LeaveCriticalSection(&g_CriticalSection);
}

//This function is called when the list needs data. This is the most
//critical function when working with virtual lists.
void CLogViewerDlg::OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	EnterCriticalSection(&g_CriticalSection);
	

	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	//Create a pointer to the item
	LV_ITEM* pItem= &(pDispInfo)->item;

	//Which item number?
	int itemid = pItem->iItem;

	TRACE("step EnterOnGetdispinfoList() \n");

	//Do the list need text information?
	//if (pItem->mask & LVIF_TEXT )
	if (pItem->mask & LVIF_TEXT && itemid < m_DBLogCount)
	{
		CString text;

		CString temp;

		//Which column?
		/*if(pItem->iSubItem == 0)			text = m_database[itemid].m_COLUMN_DATE;
		else if (pItem->iSubItem == 1)		text = m_database[itemid].m_COLUMN_CATEGORY;
		else if (pItem->iSubItem == 2)		text = m_database[itemid].m_COLUMN_LOG_CODE;
		else if (pItem->iSubItem == 3)		text = m_database[itemid].m_COLUMN_ERROR;
		else if (pItem->iSubItem == 4)		text = m_database[itemid].m_COLUMN_THREAD;
		else if (pItem->iSubItem == 5)		text = m_database[itemid].m_COLUMN_MESSAGE;*/

		if(pItem->iSubItem == 0)			text = m_database[itemid].m_COLUMN_DATE;
		else if (pItem->iSubItem == 1)		text = m_database[itemid].m_COLUMN_CATEGORY;
		else if (pItem->iSubItem == 2)		text = m_database[itemid].m_COLUMN_ERROR;
		else if (pItem->iSubItem == 3)		text = m_database[itemid].m_COLUMN_THREAD;
		else if (pItem->iSubItem == 4)		text = m_database[itemid].m_COLUMN_MESSAGE;

	//	TRACE("step MidOnGetdispinfoList() \n");


		//Copy the text to the LV_ITEM structure
		//Maximum number of characters is in pItem->cchTextMax
		lstrcpyn(pItem->pszText, text, pItem->cchTextMax);
	}
	//TRACE("step ExitOnGetdispinfoList() \n");

	LeaveCriticalSection(&g_CriticalSection);
/*
	//Do the list need image information?
	if( pItem->mask & LVIF_IMAGE) 
	{
		//Set which image to use
		pItem->iImage=m_database[itemid].m_image;
		
		//Show check box?
		if(IsCheckBoxesVisible())
		{
			//To enable check box, we have to enable state mask...
			pItem->mask |= LVIF_STATE;
			pItem->stateMask = LVIS_STATEIMAGEMASK;

			if(m_database[itemid].m_checked)
			{
				//Turn check box on..
				pItem->state = INDEXTOSTATEIMAGEMASK(2);
			}
			else
			{
				//Turn check box off
				pItem->state = INDEXTOSTATEIMAGEMASK(1);
			}
		}
	}
*/
	*pResult = 0;
}

//This functions is called when the user "writes" in the list box to find an item.
void CLogViewerDlg::OnOdfinditemList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// pNMHDR has information about the item we should find
	// In pResult we should save which item that should be selected

	//EnterCriticalSection(&g_CriticalSection);

	NMLVFINDITEM* pFindInfo = (NMLVFINDITEM*)pNMHDR;
	TRACE("step OnOdfinditemList() \n");

	/* pFindInfo->iStart is from which item we should search.
	   We search to bottom, and then restart at top and will stop
	   at pFindInfo->iStart, unless we find an item that match
	 */

	// Set the default return value to -1
	// That means we didn't find any match.
	*pResult = -1;

	//Is search NOT based on string?
	if( (pFindInfo->lvfi.flags & LVFI_STRING) == 0 )
	{
		//This will probably never happend...
		return;
	}

	/*
		Let's look on a sample list;
		
		  Name
		  Anders
		* Anna
		  Annika
		  Bob
		  Emma
		  Emmanuel

		Anna is selected. 
		If "A" is written, Annika should be selected.
		If "AND" is written, Anders should be selected. 
		If "ANNK" is written, the selection should stay on Anna.
		If "E" is written, Emma should be selected.

    */

	//This is the string we search for
	CString searchstr = pFindInfo->lvfi.psz;

//	TRACE(_T("Find: %s\n"), searchstr);
	
	int startPos = pFindInfo->iStart;
	//Is startPos outside the list (happens if last item is selected)
	if(startPos >= m_LogList.GetItemCount())
		startPos = 0;

	int currentPos=startPos;
	
	//Let's search...
	do
	{		
		//Do this word begins with all characters in searchstr?
		if( _tcsnicmp(m_database[currentPos].m_COLUMN_DATE, searchstr, searchstr.GetLength()) == 0)
		{
			//Select this item and stop search.
			*pResult = currentPos;
			break;
		}

		//Go to next item
		currentPos++;

		//Need to restart at top?
		if(currentPos >= m_LogList.GetItemCount())
			currentPos = 0;

	//Stop if back to start
	}while(currentPos != startPos);	

	//LeaveCriticalSection(&g_CriticalSection);
}

//This is called to give you a chance to catch data. Useless in most cases :-)
void CLogViewerDlg::OnOdcachehintList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//EnterCriticalSection(&g_CriticalSection);

	NMLVCACHEHINT* pCacheHint = (NMLVCACHEHINT*)pNMHDR;
	TRACE("step OnOdcachehintList() \n");

/*	TRACE(	_T("Chache item %d to item %d\n"),
			pCacheHint->iFrom,
			pCacheHint->iTo );*/

	//... cache the data pCacheHint->iFrom to pCacheHint->iTo ...
	//LeaveCriticalSection(&g_CriticalSection);

	*pResult = 0;
}


void CLogViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: ¿©±â¿¡ ¸Þ½ÃÁö Ã³¸®±â ÄÚµå¸¦ Ãß°¡ ¹×/¶Ç´Â ±âº»°ªÀ» È£ÃâÇÕ´Ï´Ù.

	if((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		//CAboutDlg dlgAbout;
		//dlgAbout.DoModal();
	}

	else if(nID == SC_MINIMIZE)
	{
		ShowWindow(SW_HIDE);
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}

	CDialog::OnSysCommand(nID, lParam);
}

void CLogViewerDlg::OnDestroy()
{
	CDialog::OnDestroy();

	_endthread();

	if(m_fp)	
		fclose(m_fp);

	// TODO: ¿©±â¿¡ ¸Þ½ÃÁö Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.
}

LRESULT CLogViewerDlg::onTrayNotify(WPARAM wParam, LPARAM lParam)
{
	UINT uMsg = (UINT) lParam;
	bool ret = TRUE;

	if (uMsg == WM_RBUTTONUP)
	{
		CPoint pt;
		GetCursorPos(&pt);
		m_TrayMenu.GetSubMenu(0)->TrackPopupMenu(TPM_RIGHTALIGN|TPM_LEFTBUTTON ,pt.x,pt.y,this);
		ret = FALSE;
	}
	else if (uMsg == WM_LBUTTONDBLCLK)
		ShowWindow(SW_SHOW);

	else if (uMsg == WM_LBUTTONUP)
		PostMessage(WM_CANCELMODE, 0, 0);

	return ret;
	return 0L;
}



void CLogViewerDlg::OnClose()
{
	// TODO: ¿©±â¿¡ ¸Þ½ÃÁö Ã³¸®±â ÄÚµå¸¦ Ãß°¡ ¹×/¶Ç´Â ±âº»°ªÀ» È£ÃâÇÕ´Ï´Ù.
	ShowWindow(SW_HIDE);

	//CDialog::OnClose();
}

void CLogViewerDlg::OnMenuExit()
{
	// TODO: ¿©±â¿¡ ¸í·É Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(nid);
	nid.hWnd = m_hWnd;
	nid.uID = IDR_MAINFRAME;

	Shell_NotifyIcon(NIM_DELETE, &nid);

	exit(0);

}

void CLogViewerDlg::OnMenuShow()
{
	// TODO: ¿©±â¿¡ ¸í·É Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.
	ShowWindow(SW_SHOW);
}

void CLogViewerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: ¿©±â¿¡ ¸Þ½ÃÁö Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.

	CRect rect;
	WINDOWPLACEMENT place;
	GetWindowPlacement(&place);
	//int left = 5;

	//int temp_x, temp_y = 0;

	if(m_LogList.GetSafeHwnd() != NULL)
    {
		//m_LogList.SetWindowPos(NULL, 10,40, cx-20, cy-80, SWP_NOZORDER);
		m_LogList.SetWindowPos(NULL, 10,40, cx-20, cy-50, SWP_NOZORDER);
	}
}

void CLogViewerDlg::OnMenuClearwindow()
{
	// TODO: ¿©±â¿¡ ¸í·É Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.
	EnterCriticalSection(&g_CriticalSection);

	m_clearCount = 0;
	m_DBLogCount = 0;
	TRACE("step OnMenuClearwindow() \n");
//	m_LogList.DeleteAllItems();
	TRACE("step OnMenuClearwindow()[After DeleteAllItems] \n");
	m_database.clear();
	TRACE("step OnMenuClearwindow()[After DB clear] \n");
//	Sleep(10000);
	_RefreshLog();
	TRACE("step OnMenuClearwindow()[After RefreshLog] \n");

	LeaveCriticalSection(&g_CriticalSection);

}

void CLogViewerDlg::OnMenuOpenfilefolder()
{
	// TODO: ¿©±â¿¡ ¸í·É Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.
	ShellExecute(NULL, _T("open"), "C:\\Program Files\\Integrated Biometrics\\IBScanUltimateSDK\\", NULL, NULL, SW_SHOW);
}

void CLogViewerDlg::OnMenuShownumberofitem()
{
	// TODO: ¿©±â¿¡ ¸í·É Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.
	CMenu *hMenu = GetMenu();

	if(hMenu->GetMenuState(ID_MENU_SHOWNUMBEROFITEM, MF_BYCOMMAND)!=MF_CHECKED)
	{
		hMenu->CheckMenuItem(ID_MENU_SHOWNUMBEROFITEM, MF_CHECKED);
		GetDlgItem(IDC_STATIC)->ShowWindow(true);
		GetDlgItem(IDC_LOG_COUNT)->ShowWindow(true);

	}
	else
	{
		hMenu->CheckMenuItem(ID_MENU_SHOWNUMBEROFITEM, MF_UNCHECKED);
		GetDlgItem(IDC_STATIC)->ShowWindow(false);
		GetDlgItem(IDC_LOG_COUNT)->ShowWindow(false);
	}

}

void CLogViewerDlg::OnMenuLogfiles()
{
	// TODO: ¿©±â¿¡ ¸í·É Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.
	CMaxDlg Dlg;

	INT_PTR nResult = Dlg.DoModal();

	/*if(nResult == IDOK)
	{
		AfxMessageBox
	}*/
}

void CLogViewerDlg::OnMenuOptionforclearwindow()
{
	// TODO: ¿©±â¿¡ ¸í·É Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.
	CClearWindow Dlg;

	if( Dlg.DoModal() == IDOK)
	{
		e_clearTerm = Dlg.m_ClearTerm;
	}
}

void CLogViewerDlg::OnUpdateMenuLogfiles(CCmdUI *pCmdUI)
{
	// TODO: ¿©±â¿¡ ¸í·É ¾÷µ¥ÀÌÆ® UI Ã³¸®±â ÄÚµå¸¦ Ãß°¡ÇÕ´Ï´Ù.
}
