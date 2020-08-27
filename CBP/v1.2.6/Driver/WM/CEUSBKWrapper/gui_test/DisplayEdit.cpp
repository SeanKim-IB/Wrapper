// DisplayEdit.cpp : implementation file
//

#include "stdafx.h"
#include "USBTest.h"
#include "DisplayEdit.h"


// DisplayEdit

IMPLEMENT_DYNAMIC(DisplayEdit, CEdit)

DisplayEdit::DisplayEdit()
{
	SendMessage(EM_SETLIMITTEXT, -1, 0);  // No maximum on the size
}

DisplayEdit::~DisplayEdit()
{
}


BEGIN_MESSAGE_MAP(DisplayEdit, CEdit)
END_MESSAGE_MAP()



// DisplayEdit message handlers


