/*
 *	$Header: $
 *
 *	$History: $
 */
#if !defined(AFX_LOGMAP_H__6A6A18A5_9A1D_40DC_90AA_9A13573266CC__INCLUDED_)
#define AFX_LOGMAP_H__6A6A18A5_9A1D_40DC_90AA_9A13573266CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLogInstance;

class CLogMap : public CMapStringToOb  
{
	DECLARE_DYNAMIC(CLogMap);
public:
					CLogMap();
	virtual			~CLogMap();

	CLogInstance	*GetLog(LPCTSTR szLogFileName);

private:
	static void		LogAgerThreadProc(LPVOID data);
	void			AgeLogFiles();

	HANDLE			m_agingThreadHandle;
};

#endif // !defined(AFX_LOGMAP_H__6A6A18A5_9A1D_40DC_90AA_9A13573266CC__INCLUDED_)
