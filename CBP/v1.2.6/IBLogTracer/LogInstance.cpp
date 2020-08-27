/*
 *	$Header: /logger/LogInstance.cpp 3     11/11/03 3:15a Administrator $
 *
 *	$History: LogInstance.cpp $
 * 
 * *****************  Version 3  *****************
 * User: Administrator Date: 11/11/03   Time: 3:15a
 * Updated in $/logger
 * Completed the move to merging the command and log listener threads.
 * 
 * *****************  Version 2  *****************
 * User: Administrator Date: 11/11/03   Time: 12:58a
 * Updated in $/logger
 * About to collapse the command and log listener threads into one.
 * 
 * *****************  Version 1  *****************
 * User: Administrator Date: 11/07/03   Time: 3:18p
 * Created in $/logger
 */
#include "StdAfx.h"
#include "LogViewer.h"
#include "LogViewerDlg.h"
#include "LogInstance.h"
#include "ServerPipeInstance.h"

#include <io.h>
#include <direct.h>

#include "SecurityToken.h"

extern CLogViewerApp theApp;

IMPLEMENT_DYNAMIC(CLogInstance, CObject);

CLogInstance::CLogInstance() : CObject()
{
	m_processSignalHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CLogInstance::~CLogInstance()
{
	//	Wait until they exit
	WaitForSingleObject(m_processThreadHandle, INFINITE);
	CloseHandle(m_processSignalHandle);
}

BOOL CLogInstance::Create(LPCTSTR szBaseName)
{
	TCHAR tszWindir[MAX_PATH + sizeof(TCHAR)];

	if (m_processSignalHandle == HANDLE(NULL))
		return FALSE;
	
	ASSERT(szBaseName);
	ASSERT(AfxIsValidString(szBaseName));

	m_csBaseName = szBaseName;
	m_csBaseName.Remove('\r');
	m_csBaseName.Remove('\n');

	//	We're creating the logfile in the logger directory under
	//	the windows root. If you want to move the directory so some
	//	other location change this code but don't forget to update
	//	the LogFileViewer as well...
	GetWindowsDirectory(tszWindir, sizeof(tszWindir));
	m_csPath.Format(_T("%s\\logger"), tszWindir);

	if (_access(m_csPath, 0) != 0)
		//	If the folder doesn't exist create it...
		_mkdir(m_csPath);

	m_csFullPath.Format(_T("%s\\%s"), m_csPath, m_csBaseName);

	m_processThreadHandle = HANDLE(_beginthread(LogProcessorThread, 0, this));
	return m_processThreadHandle != LPVOID(-1);
}

void CLogInstance::QueueMessage(LPCTSTR szLogEntry)
{
	ASSERT(szLogEntry);
	ASSERT(AfxIsValidString(szLogEntry));

	//	Lock the object for the addhead call...
	//	(must be fast once we get the lock).
	m_mutex.Lock();
	m_messageQueue.AddHead(szLogEntry);
	m_mutex.Unlock();

	//	Now tell our processing thread there's 
	//	new stuff to write...
	SetEvent(m_processSignalHandle);
}

void CLogInstance::CreateFile()
{
	TRACE("Creating file %s\n", m_csFullPath);
	if ((m_hLogFile = ::CreateFile(
				m_csFullPath, 
				GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, 
				NULL, 
				OPEN_ALWAYS, 
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 
				HANDLE(NULL)
			)
		 ) == INVALID_HANDLE_VALUE
		)
	{
		//	If we failed to create the file log an error on the 
		//	eventlog and exit...
//		_Module.LogEvent(_T("Unable to create logfile %s"), m_csPath);
		_endthread();
	}
	else
	{
		//	What follows may look redundant but don't be tempted to change it.
		//	According to the CreateFile documentation,  if you delete or rename
		//	a file and quickly create a new file with the old name,  NT will
		//	pull file attribute information INCLUDING creation date from the
		//	file cache.  Thus,  the newly created log file will have the creation
		//	date of the original file even though it's 24 hours later...

		//	There are two solutions.  One is to use the native API SetFileTime
		//	API - which wants a 64 bit number representing the number of 100 
		//	nanosecond intervals since January 1 1601 UTC - or we can close
		//	the file handle - use the CTime function to use a CTime object
		//	and reopen the file.
		try
		{
			CFileStatus status;

			//	Close the file so we can change the creation date
			CloseHandle(m_hLogFile);
			CFile::GetStatus(m_csFullPath, status);
			status.m_ctime = CTime::GetCurrentTime();
			CFile::SetStatus(m_csFullPath, status);

			//	No open the file again
			if ((m_hLogFile = ::CreateFile(
						m_csFullPath, 
						GENERIC_WRITE, 
						FILE_SHARE_READ | FILE_SHARE_WRITE, 
						NULL, 
						OPEN_ALWAYS, 
						FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 
						HANDLE(NULL)
					)
				 ) == INVALID_HANDLE_VALUE
				)
			{
				//	If we failed to create the file log an error on the 
				//	eventlog and exit...
//				_Module.LogEvent(_T("Unable to reopen logfile %s"), m_csPath);
				_endthread();
			}
		}
		catch(CFileException *fe)
		{
			TRACE(_T("File exception (%d) caught in Rename\n"), fe->m_lOsError);
			fe->Delete();
		}
	}

	//	Go to the end of the file...
	SetFilePointer(m_hLogFile, 0, 0, SEEK_END);
}

void CLogInstance::AgeLogFiles()
{
	//	First we lock ourself so we can be sure no writes are
	//	attempted on the file we're about to rename
	m_mutex.Lock();

	CString	csPattern;
	TCHAR	tszBaseName[_MAX_FNAME];

	//	Extract the filename without the extension...
	_splitpath(m_csFullPath, NULL, NULL, tszBaseName, NULL);

	//	Build our search pattern
	csPattern.Format(_T("%s\\%s*.log"), m_csPath, tszBaseName);

	CFileFind ff;
	BOOL	  bWorking = ff.FindFile(csPattern);
	CTime	  creationTime,
			  outOfDate = CTime::GetCurrentTime(),
			  now = outOfDate;
	CTimeSpan span(7, 0, 0, 0);	//  7 Days

	outOfDate -= span;

	while (bWorking)
	{
		//	For each file that matches the filename pattern...
		bWorking = ff.FindNextFile();
		ff.GetCreationTime(creationTime);

		if (creationTime < outOfDate)
			//	If the file is older than 7 days delete it
			_tunlink(ff.GetFilePath());
			//	_Module.LogEvent(_T("Unable to delete file %s"), ff.GetFilePath());
	}

	TCHAR tszWindir[_MAX_PATH];

	GetWindowsDirectory(tszWindir, sizeof(tszWindir));
	csPattern.Format(_T("%s\\%s.%s.log"), m_csPath, tszBaseName, now.Format(_T("%Y%m%d")));
	CloseHandle(m_hLogFile);
	
	if (!MoveFile(m_csFullPath, csPattern));
//		_Module.LogEvent(_T("Unable to rename file %s, errcode %d"), m_csFullPath, GetLastError());
	else
		CreateFile();

	m_mutex.Unlock();
}

//	This thread takes messages off the CStringList and writes them to the
//	disk file.
void CLogInstance::LogProcessorThread(LPVOID data)
{
	//	We need an extra scoping level here because we are using objects
	//	inside this procedure which need their destructors run.  If
	//	we leave out the extra level of nesting the _endthread() call 
	//	at the bottom of the procedure bypasses all destructors.
	{
		CLogViewerDlg *dlg = (CLogViewerDlg *)theApp.GetMainWnd();

		CLogInstance *me = (CLogInstance *) data;
		BOOL		 bStop = FALSE;
		HANDLE		 hHandleArray[2];

		ASSERT(me);
		ASSERT_KINDOF(CLogInstance, me);

		hHandleArray[eStop] = m_hStopEvent;
		hHandleArray[eProcess] = me->m_processSignalHandle;

		CString csData;

//		me->CreateFile();

		while (bStop == FALSE)
		{
			switch (WaitForMultipleObjects(2, hHandleArray, FALSE, INFINITE))
			{
			case WAIT_OBJECT_0:
				bStop = TRUE;
				break;

			case WAIT_OBJECT_0 + 1:	
				//	Lock the queue to get the count and the next entry
				me->m_mutex.Lock();

				while (me->m_messageQueue.GetCount())
				{
					csData = me->m_messageQueue.RemoveTail();
					//	Now unlock the queue while we write...
					me->m_mutex.Unlock();

					dlg->_InsertLog(csData);

					//	Seek to the end of the file...
//					SetFilePointer(me->m_hLogFile, 0, NULL, FILE_END);
//					WriteFile(me->m_hLogFile, csData, csData.GetLength(), &dwWritten, NULL);

					//	Lock it again to get the count and the next entry
					me->m_mutex.Lock();
				}
				dlg->_RefreshLog();

				//	Now unlock it again
				me->m_mutex.Unlock();

				//	Force output to disk...
//				FlushFileBuffers(me->m_hLogFile);
				ResetEvent(me->m_processSignalHandle);
				break;
			}
		}

		CloseHandle(me->m_hLogFile);
	}

	_endthread();
}
