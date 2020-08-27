/*
 *	$Header: /logger/PipeData.h 2     11/11/03 3:15a Administrator $
 *
 *	$History: PipeData.h $
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
#if !defined(AFX_PIPEDATA_H__4FAE45EC_03E8_42A5_BCAE_341A7696223B__INCLUDED_)
#define AFX_PIPEDATA_H__4FAE45EC_03E8_42A5_BCAE_341A7696223B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CServerPipeInstance;

class CPipeData : public CObject
{
	DECLARE_DYNAMIC(CPipeData);
public:
					CPipeData();
	virtual			~CPipeData();

	void			AddPipe(CServerPipeInstance *pPipe);
	void			RemovePipe(DWORD wIndex);
	void			RotatePipes(DWORD wIndex);

	CServerPipeInstance	*GetPipe(DWORD wIndex)
		{ return (CServerPipeInstance *) m_pipeArray.GetAt(wIndex); }

	DWORD			GetCount() const
		{ return DWORD(m_handleArray.GetSize()); }

	DWORD			Wait()
		{ return DWORD(WaitForMultipleObjects(m_handleArray.GetSize(), m_handleArray.GetData(), FALSE, INFINITE)); }

private:
	CObArray		m_pipeArray;
	CArray<HANDLE, HANDLE>m_handleArray;
};

#endif // !defined(AFX_PIPEDATA_H__4FAE45EC_03E8_42A5_BCAE_341A7696223B__INCLUDED_)
