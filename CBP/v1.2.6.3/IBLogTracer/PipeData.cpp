/*
 *	$Header: /logger/PipeData.cpp 2     11/11/03 3:15a Administrator $
 *
 *	$History: PipeData.cpp $
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
#include "LogMap.h"
#include "LogInstance.h"
#include "ServerPipeInstance.h"
#include "PipeData.h"

IMPLEMENT_DYNAMIC(CPipeData, CObject);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CPipeData::CPipeData() : CObject()
{
	m_handleArray.Add(m_hStopEvent);
	
	//	Add a placeholder pointer which we'll never use (better not try)
	//	so that indices in the pipe array match indices in the handle array.
	m_pipeArray.Add(NULL);
}

CPipeData::~CPipeData()
{
	HANDLE h;

	while (m_handleArray.GetSize())
	{
		h = m_handleArray[0];
		m_handleArray.RemoveAt(0);
		CloseHandle(h);
	}
}

void CPipeData::AddPipe(CServerPipeInstance *pPipe)
{
	ASSERT(pPipe);
	ASSERT_KINDOF(CServerPipeInstance, pPipe);

	//	Add the pipe instance event handle to the event array
	m_handleArray.Add(pPipe->OP()->hEvent);

	//	And add the pipe to the pipe array
	m_pipeArray.Add(pPipe);

	//	Now start the pipe...
	pPipe->ConnectPipe();
	pPipe->ReadPipe();
}

void CPipeData::RemovePipe(DWORD dwIndex)
{
	ASSERT(dwIndex >= 2 && dwIndex < DWORD(m_pipeArray.GetSize()));

	CServerPipeInstance *pPipe;

	//	Remove the handle from the handle array
	m_handleArray.RemoveAt(dwIndex);

	//	And remove the pipe from the pipe array
	pPipe = (CServerPipeInstance *) m_pipeArray.GetAt(dwIndex);
	m_pipeArray.RemoveAt(dwIndex);

	ASSERT(pPipe);
	ASSERT_KINDOF(CServerPipeInstance, pPipe);

	delete pPipe;
}

//	This strange looking function simply reorders the pipe and event
//	handle arrays so that the most recently serviced pipe goes to the
//	end of the array, thus giving other pipes a chance at being serviced.
void CPipeData::RotatePipes(DWORD wIndex)
{
	if (wIndex == DWORD(m_handleArray.GetUpperBound()))
		return;

	HANDLE				hTemp = m_handleArray.GetAt(wIndex);
	CServerPipeInstance *pTemp = (CServerPipeInstance *) m_pipeArray.GetAt(wIndex);

	//	Take the pipe and the event object out of their current location
	m_handleArray.RemoveAt(wIndex);
	m_pipeArray.RemoveAt(wIndex);

	//	And add em back in at the end...
	m_handleArray.Add(hTemp);
	m_pipeArray.Add(pTemp);
}
