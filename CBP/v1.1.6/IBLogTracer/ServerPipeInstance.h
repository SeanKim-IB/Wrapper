/*
 *	$Header: /logger/ServerPipeInstance.h 2     11/11/03 3:15a Administrator $
 *
 *	$History: ServerPipeInstance.h $
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
#if !defined(AFX_SERVERPIPEINSTANCE_H__80301E23_B458_4ADF_8BE0_18E2CD5FE280__INCLUDED_)
#define AFX_SERVERPIPEINSTANCE_H__80301E23_B458_4ADF_8BE0_18E2CD5FE280__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLogInstance;

class CServerPipeInstance : public CObject
{
private:
	DECLARE_DYNAMIC(CServerPipeInstance);

public:
				CServerPipeInstance();
				~CServerPipeInstance();

	BOOL		CreatePipe(CLogInstance *pOwner, LPCTSTR szBaseName);

	BOOL		ConnectPipe()			{ return ConnectNamedPipe(m_hPipe, &m_op); }
	BOOL		DisconnectPipe()		{ return DisconnectNamedPipe(m_hPipe); }
	BOOL		GetOverlappedResult(DWORD *pdwRead);

	BOOL		ReadPipe();
	void		RestartPipe();
	void		ResetEvent()			{ ::ResetEvent(m_op.hEvent); }
	void		QueueMessage();

	CLogInstance *Owner()				{ return m_pOwner; }
	OVERLAPPED	*OP()					{ return &m_op; }
	HANDLE		Pipe()					{ return m_hPipe; }

	TCHAR		m_tszBuffer[PIPEBUFFERSIZE + 1];

protected:
	CLogInstance *m_pOwner;

private:
	HANDLE		m_hPipe;
	OVERLAPPED	m_op;
	DWORD		m_dwRead;
	CString		m_csBaseName;
};

#endif // !defined(AFX_SERVERPIPEINSTANCE_H__80301E23_B458_4ADF_8BE0_18E2CD5FE280__INCLUDED_)
