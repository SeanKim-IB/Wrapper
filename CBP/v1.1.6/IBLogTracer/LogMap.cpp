/*
 *	$Header: $
 *
 *	$History: $
 */
#include "stdafx.h"
#include "LogInstance.h"
#include "LogMap.h"

IMPLEMENT_DYNAMIC(CLogMap, CMapStringToOb);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CLogMap::CLogMap() : CMapStringToOb()
{
	//	Initialise the aging thread handle to NULL.  We only start that
	//	thread if we have at least one CLogFile instance
	m_agingThreadHandle = HANDLE(NULL);
}

CLogMap::~CLogMap()
{
	CLogInstance *pLog;
	POSITION	 pos = GetStartPosition();
	CString		 key;

	while (pos != POSITION(NULL))
	{
		GetNextAssoc(pos, key, (CObject*&) pLog);

		ASSERT(pLog);
		ASSERT_KINDOF(CLogInstance, pLog);

		delete pLog;
	}

	if (m_agingThreadHandle != HANDLE(NULL))
		//	If we launched the aging thread we should wait for it to exit
		WaitForSingleObject(m_agingThreadHandle, INFINITE);
}

CLogInstance *CLogMap::GetLog(LPCTSTR szLogFileName)
{
	CLogInstance *pLog;

	if (!Lookup(szLogFileName, (CObject*&) pLog))
	{
		//	We don't yet have a logfile of this name, so let's create one
		pLog = new CLogInstance;

		ASSERT(pLog);
		ASSERT_KINDOF(CLogInstance, pLog);

		if (pLog->Create(szLogFileName))
			SetAt(szLogFileName, pLog);
		else
		{
			//	We failed to create the object so delete it and set the
			//	handle to NULL
			delete pLog;
			pLog = (CLogInstance *) NULL;
		}
	}

	//	If we haven't yet launched the aging thread now is the time to do it
//	if (m_agingThreadHandle == HANDLE(NULL) && pLog != (CLogInstance *) NULL)
//		m_agingThreadHandle = HANDLE(_beginthread(LogAgerThreadProc, 0, this));

	return pLog;
}

void CLogMap::AgeLogFiles()
{
	CLogInstance *pLog;
	POSITION	 pos;
	CString		 csKey;

	pos = GetStartPosition();

	while (pos != POSITION(NULL))
	{
		GetNextAssoc(pos, csKey, (CObject*&) pLog);

		ASSERT(pLog);
		ASSERT_KINDOF(CLogInstance, pLog);

		pLog->AgeLogFiles();
	}
}

void CLogMap::LogAgerThreadProc(LPVOID data)
{
	//	We need an extra scoping level here because we are using objects
	//	inside this procedure which need their destructors run.  If
	//	we leave out the extra level of nesting the _endthread() call 
	//	at the bottom of the procedure bypasses all destructors.
	{
		CLogMap *me = (CLogMap *) data;
		BOOL	bStop = FALSE;

		ASSERT(me);
		ASSERT_KINDOF(CLogMap, me);

		while (bStop == FALSE)
		{
			//	Calculate the number of seconds until local midnight
			COleDateTime	 now = COleDateTime::GetCurrentTime(),
							 then = now;
			COleDateTimeSpan span(1, 0, 0, 0);
			int				 countDown;

			then += span;
			then = floor(then);
			span = then - now;
			countDown = int(span.GetTotalSeconds());

			switch (WaitForSingleObject(m_hStopEvent, countDown * 1000))
			{
			case WAIT_OBJECT_0:
				bStop = TRUE;
				break;

			case WAIT_TIMEOUT:
				//	Time to age the logfiles.
				me->AgeLogFiles();
				break;
			}
		}
	}

	_endthread();
}
