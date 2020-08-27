#pragma once


// ClientWindowFunctions dialog

class ClientWindowFunctions : public CDialog
{
	DECLARE_DYNAMIC(ClientWindowFunctions)

public:
	ClientWindowFunctions(CWnd* pParent = NULL);   // standard constructor
	virtual ~ClientWindowFunctions();

// Dialog Data
	enum { IDD = IDD_DIALOG_CLIENTWINDOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
