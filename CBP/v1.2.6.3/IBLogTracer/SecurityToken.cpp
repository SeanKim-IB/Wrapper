/*
 *	$Header: /logger/common/SecurityToken.cpp 1     11/11/03 2:45a Administrator $
 *
 *	$History: SecurityToken.cpp $
 * 
 * *****************  Version 1  *****************
 * User: Administrator Date: 11/11/03   Time: 2:45a
 * Created in $/logger/common
 * 
 * *****************  Version 1  *****************
 * User: Administrator Date: 11/07/03   Time: 3:18p
 * Created in $/logger
 */
#include "stdafx.h"
#include "SecurityToken.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSecurityToken::CSecurityToken(BOOL bInheritHandles)
{
    InitializeSecurityDescriptor(&m_sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&m_sd, TRUE, NULL, FALSE);

	m_sa.nLength = sizeof(m_sa);
	m_sa.lpSecurityDescriptor = &m_sd;
	m_sa.bInheritHandle = bInheritHandles;
}
