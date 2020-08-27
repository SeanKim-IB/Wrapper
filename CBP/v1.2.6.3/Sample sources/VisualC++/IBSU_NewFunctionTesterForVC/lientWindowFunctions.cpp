// lientWindowFunctions.cpp : implementation file
//

#include "stdafx.h"
#include "IBSU_NewFunctionTester.h"
#include "lientWindowFunctions.h"


// ClientWindowFunctions dialog

IMPLEMENT_DYNAMIC(ClientWindowFunctions, CDialog)

ClientWindowFunctions::ClientWindowFunctions(CWnd* pParent /*=NULL*/)
	: CDialog(ClientWindowFunctions::IDD, pParent)
{

}

ClientWindowFunctions::~ClientWindowFunctions()
{
}

void ClientWindowFunctions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ClientWindowFunctions, CDialog)
END_MESSAGE_MAP()


// ClientWindowFunctions message handlers
