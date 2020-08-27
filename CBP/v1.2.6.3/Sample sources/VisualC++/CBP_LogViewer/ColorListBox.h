#if !defined(AFX_COLORLISTBOX_H__66DAD731_D5F8_11D3_A636_00105A7C2F91__INCLUDED_)
#define AFX_COLORLISTBOX_H__66DAD731_D5F8_11D3_A636_00105A7C2F91__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorListBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorListBox window

// ColorListBox.h : header file

//-------------------------------------------------------------------
//
//	CColorListBox class - 
//		A CListBox-derived class with optional colored items.
//
//		Version: 1.0	01/10/1998 Copyright © Patrice Godard
//
//		Version: 2.0	09/17/1999 Copyright © Paul M. Meidinger
//
//-------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CColorListBox window

class CColorListBox : public CListBox
{
// Construction
public:
	CColorListBox();

// Attributes
public:

// Operations
public:
	int AddString(LPCTSTR lpszItem);								// Adds a string to the list box
	int AddString(LPCTSTR lpszItem, COLORREF rgb);					// Adds a colored string to the list box
	int InsertString(int nIndex, LPCTSTR lpszItem);					// Inserts a string to the list box
	int InsertString(int nIndex, LPCTSTR lpszItem, COLORREF rgb);	// Inserts a colored string to the list box
	void SetItemColor(int nIndex, COLORREF rgb);					// Sets the color of an item in the list box
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorListBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorListBox)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORLISTBOX_H__66DAD731_D5F8_11D3_A636_00105A7C2F91__INCLUDED_)
