#pragma once

struct CListCtrl_DataRecord
{
	CListCtrl_DataRecord()
	{}

	CListCtrl_DataRecord(const CString& finger ,const CString& fingerstatus)
		:m_finger(finger)
		:m_fingerstatus(fingerstatus)
	{}

	CString m_finger;
	CString m_fingerstatus;

	CString GetCellText(int col, bool title)const
	{
		switch(col)
		{
		case 0:
			{
				static const CString title0(_T("FINGER"));
				return title ? title0 : m_finger;
			}
		case 1:
			{
				static const CString title1(_T("ANNOTATION"));
				return title? title1 : m_fingerstatus;
			}
		default:
			static const CString emptyStr; return emptyStr;
		}
	}

	int GetColCount() 
		const{ return 2 };
};

class CListCtrl_DataModel
{
	vector<CListCtrl_DataRecord> m_Records;
	int 

}