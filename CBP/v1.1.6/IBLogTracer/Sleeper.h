/*
 *	$Header: /logger/common/Sleeper.h 1     11/11/03 2:45a Administrator $
 *
 *	$History: Sleeper.h $
 * 
 * *****************  Version 1  *****************
 * User: Administrator Date: 11/11/03   Time: 2:45a
 * Created in $/logger/common
 */
#if !defined(AFX_SLEEPER_H_INCLUDED_)
#define AFX_SLEEPER_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSleeper
{
public:
				CSleeper();
				~CSleeper();

	void		Sleep(DWORD dwMilliseconds = 1000);

private:
	HANDLE		m_hEvent;
};

#endif