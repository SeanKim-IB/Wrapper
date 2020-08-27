/*
 *	$Header: /logger/LogInstance.h 2     11/11/03 12:58a Administrator $
 *
 *	$History: LogInstance.h $
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
#if !defined(AFX_LOGINSTANCE_H__3267ED2C_7373_471E_BB4F_ECC7E0D453E8__INCLUDED_)
#define AFX_LOGINSTANCE_H__3267ED2C_7373_471E_BB4F_ECC7E0D453E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLogInstance : public CObject
{
private:
	DECLARE_DYNAMIC(CLogInstance);

public:
				CLogInstance();
				~CLogInstance();

	BOOL		Create(LPCTSTR szBaseName);
	void		QueueMessage(LPCTSTR szLogEntry);
	void		AgeLogFiles();

	LPCTSTR		BaseName()				{ return m_csBaseName; }

private:
	void		CreateFile();

	static void LogProcessorThread(LPVOID data);

	HANDLE		m_processSignalHandle,
				m_processThreadHandle,
				m_hLogFile;
	CString		m_csBaseName,
				m_csFullPath,
				m_csPath;
	CStringList m_messageQueue;
	CMutex		m_mutex;
};

#endif