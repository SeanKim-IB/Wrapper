#pragma once

#include "stdafx.h"

class CINIManager
{
public:
	CINIManager();
	CINIManager(CString inifilename);
	~CINIManager(void);

	void SetFilename(CString inifilename);
	int ISExistINI();
	
	int WriteProperty(CString AppName, CString KeyName, CString Value);
	int WriteProperty(CString AppName, CString KeyName, int Value);

	int ReadProperty(CString AppName, CString KeyName, CString &readval);
	int ReadProperty(CString AppName, CString KeyName, int &readval);

private:
	CString m_INI_filename;
};
