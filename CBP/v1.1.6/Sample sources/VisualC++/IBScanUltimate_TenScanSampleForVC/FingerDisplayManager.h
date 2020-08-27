#pragma once

#include "afxwin.h"

#define ERROR_WRONG_PARAM		0xFF
#define STATE_SELECTED			0x01
#define STATE_COMPLETED			0x01
#define STATE_DEFAULT			0x00

#define ONE_FINGER_MODE			0x01
#define TWO_FINGER_MODE			0x02

class CFingerDisplayManager
{
public:
	CFingerDisplayManager(void);
	CFingerDisplayManager(CStatic *view, int Width, int Height, BYTE Capture_Mode);
	~CFingerDisplayManager(void);

	BOOL SetFingerSelected(int index, BYTE state);
	BYTE GetFingerSelected(int index);
	BOOL SetFingerCompleted(int index, BYTE state);
	BYTE GetFingerCompleted(int index);
	BOOL SelectFinger(CPoint point);
	BOOL SelectFinger(BYTE index);
	BYTE GetSelectedFingerIndex();
	BOOL ToggleEnableFinger(CPoint point);
	BOOL ToggleEnableFinger(BYTE index);
	BOOL IsEnableFinger(BYTE index);
	
	void SetCaptureMode(BYTE Capture_Mode);
	void InitializeVariables(void);
	void UpdateDisplayWindow(void);

private:
	CStatic			*m_pView;
	CRect			m_FingerRect[10];
	BYTE			m_FingerState[10];
	BOOL			m_FingerEnable[10];
	BYTE			m_FingerCaptureMode;
	UINT			m_WindowWidth;
	UINT			m_WindowHeight;
	CRect			m_PalmRect[2];			// only for display
};
