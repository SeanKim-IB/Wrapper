/*
 *	$Header: /logger/ServerPipeInstance.cpp 2     11/11/03 3:15a Administrator $
 *
 *	$History: ServerPipeInstance.cpp $
 * 
 * *****************  Version 2  *****************
 * User: Administrator Date: 11/11/03   Time: 3:15a
 * Updated in $/logger
 * Completed the move to merging the command and log listener threads.
 * 
 * *****************  Version 1  *****************
 * User: Administrator Date: 11/11/03   Time: 12:57a
 * Created in $/logger
 * 
 * *****************  Version 1  *****************
 * User: Administrator Date: 11/07/03   Time: 3:18p
 * Created in $/logger
 */
#include "stdafx.h"
#include "SecurityToken.h"
#include "LogInstance.h"
#include "ServerPipeInstance.h"
#include "sleeper.h"

IMPLEMENT_DYNAMIC(CServerPipeInstance, CObject);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CServerPipeInstance::CServerPipeInstance() : CObject()
{
	m_pOwner = (CLogInstance *) NULL;
	m_hPipe = HANDLE(NULL);
	memset(&m_op, 0, sizeof(OVERLAPPED));
}

CServerPipeInstance::~CServerPipeInstance()
{
	if (m_hPipe != HANDLE(NULL) && m_hPipe != INVALID_HANDLE_VALUE)
	{
		DisconnectPipe();
		CloseHandle(m_hPipe);
	}

	if (m_op.hEvent != HANDLE(NULL) && m_op.hEvent != INVALID_HANDLE_VALUE)
		CloseHandle(m_op.hEvent);
}

BOOL CServerPipeInstance::CreatePipe(CLogInstance *pOwner, LPCTSTR szBaseName)
{
	if (pOwner != (CLogInstance *) NULL)
		ASSERT_KINDOF(CLogInstance, pOwner);

	ASSERT(szBaseName);
	ASSERT(AfxIsValidString(szBaseName));

	m_pOwner = pOwner;
	m_csBaseName = szBaseName;
	m_op.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_op.hEvent == HANDLE(NULL))
	{
		//	If we failed to create the event handle
//		_Module.LogEvent(_T("Unable to create signal event for pipe %s"), szBaseName);
		return FALSE;
	}

	CSecurityToken sd;
	CString		   csName;

	csName.Format(_T("\\\\.\\pipe\\%s"), m_csBaseName);

	//	Create our pipe using overlapped I/O
	m_hPipe = CreateNamedPipe(
		csName, 
		PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
		PIPE_UNLIMITED_INSTANCES,
		0,
		PIPEBUFFERSIZE,
		INFINITE,
		&sd
		);

	if (m_hPipe == INVALID_HANDLE_VALUE)
	{
//		_Module.LogEvent(_T("Unable to create named pipe %s"), csName);
		return FALSE;
	}

	return TRUE;
}

BOOL CServerPipeInstance::GetOverlappedResult(DWORD *pdwRead)
{
	BOOL bResult;

	ASSERT(pdwRead);
	ASSERT(AfxIsValidAddress(pdwRead, sizeof(DWORD), TRUE));

	if ((bResult = ::GetOverlappedResult(m_hPipe, &m_op, pdwRead, FALSE)) != FALSE && *pdwRead)
		m_tszBuffer[*pdwRead] = 0;

	return bResult;
}

BOOL CServerPipeInstance::ReadPipe()
{
	//	Try to read from the pipe...
	ReadFile(m_hPipe, m_tszBuffer, PIPEBUFFERSIZE, &m_dwRead, &m_op);

	if (GetLastError() == ERROR_BROKEN_PIPE)
	{
		//	Pipe broken, disconnect
		DisconnectPipe();
		return FALSE;
	}

	return TRUE;
}

void CServerPipeInstance::RestartPipe()
{
	CSleeper sleeper;

	DisconnectPipe();
	sleeper.Sleep(10);
	ConnectPipe();
	sleeper.Sleep(10);
	ReadPipe();
}

void CServerPipeInstance::QueueMessage()
{
	ASSERT(m_pOwner);
	ASSERT_KINDOF(CLogInstance, m_pOwner);

	m_pOwner->QueueMessage(m_tszBuffer);
}
