/*
 *	$Header: /logger/common/SecurityToken.h 1     11/11/03 2:45a Administrator $
 *
 *	$History: SecurityToken.h $
 * 
 * *****************  Version 1  *****************
 * User: Administrator Date: 11/11/03   Time: 2:45a
 * Created in $/logger/common
 * 
 * *****************  Version 1  *****************
 * User: Administrator Date: 11/07/03   Time: 3:18p
 * Created in $/logger
 */
#if !defined(AFX_SECURITYDESCRIPTOR_H__3267ED2C_7373_471E_BB4F_ECC7E0D453E8__INCLUDED_)
#define AFX_SECURITYDESCRIPTOR_H__3267ED2C_7373_471E_BB4F_ECC7E0D453E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSecurityToken  
{
public:
					CSecurityToken(BOOL bInheritHandles = FALSE);

	SECURITY_ATTRIBUTES *operator&()	{ return &m_sa; }

private:
    SECURITY_DESCRIPTOR m_sd;
	SECURITY_ATTRIBUTES m_sa;
};

#endif // !defined(AFX_SECURITYDESCRIPTOR_H__3267ED2C_7373_471E_BB4F_ECC7E0D453E8__INCLUDED_)
