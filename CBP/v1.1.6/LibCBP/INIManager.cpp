#include "StdAfx.h"
#include "INIManager.h"

CINIManager::CINIManager()
{
	m_INI_filename = _T("");
}

CINIManager::CINIManager(CString inifilename)
{
	m_INI_filename = inifilename;
}

CINIManager::~CINIManager(void)
{
}

void CINIManager::SetFilename(CString inifilename)
{
	m_INI_filename = inifilename;
}

int CINIManager::ISExistINI()
{
	FILE *fp = fopen(m_INI_filename, "rb");

	if(fp == NULL)
		return FALSE;

	fclose(fp);

	return TRUE;
}

int CINIManager::WriteProperty(CString AppName, CString KeyName, CString Value)
{
	if( WritePrivateProfileString(_T(AppName), _T(KeyName), Value, m_INI_filename) == TRUE )
		return TRUE;

	return FALSE;
}

int CINIManager::WriteProperty(CString AppName, CString KeyName, int Value)
{
	CString str;
	str.Format("%d", Value);

	if( WritePrivateProfileString(_T(AppName), _T(KeyName), str, m_INI_filename) == TRUE )
		return TRUE;

	return FALSE;
}

int CINIManager::ReadProperty(CString AppName, CString KeyName, CString &readval)
{
	CString TmpRead;
	if( GetPrivateProfileString(_T(AppName), _T(KeyName), NULL, TmpRead.GetBuffer(256), 256, m_INI_filename) > 0 )
	{
		readval = TmpRead;
		return TRUE;
	}

	return WriteProperty(_T(AppName), _T(KeyName), readval);
}

int CINIManager::ReadProperty(CString AppName, CString KeyName, int &readval)
{
	CString TmpRead;
	if( GetPrivateProfileString(_T(AppName), _T(KeyName), NULL, TmpRead.GetBuffer(256), 256, m_INI_filename) > 0 )
	{
		readval = atoi(TmpRead);
		return TRUE;
	}

	return WriteProperty(_T(AppName), _T(KeyName), readval);
}
