
// FirmwareEditer.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CFirmwareEditerApp:
// �� Ŭ������ ������ ���ؼ��� FirmwareEditer.cpp�� �����Ͻʽÿ�.
//

class CFirmwareEditerApp : public CWinApp
{
public:
	CFirmwareEditerApp();

// �������Դϴ�.
	public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CFirmwareEditerApp theApp;