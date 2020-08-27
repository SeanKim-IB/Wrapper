#include "stdafx.h"
#include "FingerDisplayManager.h"


CFingerDisplayManager::CFingerDisplayManager(void)
{
	CFingerDisplayManager(NULL, 300, 205, ONE_FINGER_MODE);
}

CFingerDisplayManager::CFingerDisplayManager(CStatic *pView, int Width, int Height, BYTE Capture_Mode)
{
	m_pView = pView;
	m_WindowWidth = Width;
	m_WindowHeight = Height;
	m_FingerCaptureMode = Capture_Mode;

	InitializeVariables();

/*	300 x 205
	m_FingerRect[LEFT_LITTLE] = CRect(12, 70, 36, 140); // left little
	m_FingerRect[LEFT_RING] = CRect(37, 46, 61, 140); // left ring
	m_FingerRect[LEFT_MIDDLE] = CRect(62, 37, 86, 140); // left middle
	m_FingerRect[LEFT_INDEX] = CRect(87, 51, 111, 140); // left index
	m_FingerRect[LEFT_THUMB] = CRect(112, 100, 143, 140); // left Thumb

	m_FingerRect[RIGHT_THUMB] = CRect(300-143, 100, 300-112, 140); // left Thumb
	m_FingerRect[RIGHT_INDEX] = CRect(300-111, 51, 300-87, 140); // left index
	m_FingerRect[RIGHT_MIDDLE] = CRect(300-86, 37, 300-62, 140); // left middle
	m_FingerRect[RIGHT_RING] = CRect(300-61, 46, 300-37, 140); // left ring
	m_FingerRect[RIGHT_LITTLE] = CRect(300-36, 70, 300-12, 140); // left little

	m_PalmRect[0] = CRect(12+4, 140, 140-8, 200);
	m_PalmRect[1] = CRect(300-140+8, 140, 300-12-4, 200);
*/
	// 200 x 138
	m_FingerRect[LEFT_LITTLE] = CRect(8, 47, 24, 94); // left little
	m_FingerRect[LEFT_RING] = CRect(25, 31, 41, 94); // left ring
	m_FingerRect[LEFT_MIDDLE] = CRect(41, 25, 57, 94); // left middle
	m_FingerRect[LEFT_INDEX] = CRect(58, 34, 74, 94); // left index
	m_FingerRect[LEFT_THUMB] = CRect(75, 67, 95, 94); // left Thumb

	m_FingerRect[RIGHT_THUMB] = CRect(200-95, 67, 200-75, 94); // left Thumb
	m_FingerRect[RIGHT_INDEX] = CRect(200-74, 34, 200-58, 94); // left index
	m_FingerRect[RIGHT_MIDDLE] = CRect(200-57, 25, 200-41, 94); // left middle
	m_FingerRect[RIGHT_RING] = CRect(200-41, 31, 200-25, 94); // left ring
	m_FingerRect[RIGHT_LITTLE] = CRect(200-24, 47, 200-8, 94); // left little

	m_PalmRect[0] = CRect(11, 94, 88, 134);
	m_PalmRect[1] = CRect(200-88, 94, 200-11, 134);
}

CFingerDisplayManager::~CFingerDisplayManager(void)
{
}

void CFingerDisplayManager::InitializeVariables()
{
	int i;
	memset(m_FingerState, STATE_DEFAULT, sizeof(m_FingerState));
	for(i=0; i<10; i++)
	{
		m_FingerEnable[i] = TRUE;
	}
}

void CFingerDisplayManager::UpdateDisplayWindow()
{
	int Width = 200;
	int Height = 138;
	int i;

	CClientDC dc(m_pView);

	CDC hMemDc;
	hMemDc.CreateCompatibleDC(&dc);
	CBitmap hBmpBuffer;
	hBmpBuffer.CreateCompatibleBitmap(&dc, Width, Height);
	HGDIOBJ hOldBmp = hMemDc.SelectObject(&hBmpBuffer);
	
	CRect rect = CRect(0,0,300,205);
	CBrush white_brush;
	white_brush.CreateSolidBrush(RGB(255,255,255));
	hMemDc.FillRect(rect, &white_brush);
	white_brush.DeleteObject();

	hMemDc.SelectObject(GetStockObject(NULL_BRUSH));

	CPen pen, pen_select, pen_disble, pen_non_comp, pen_comp, *oldpen;
	pen.CreatePen(PS_SOLID, 3, RGB(255,0,0));
	pen_select.CreatePen(PS_SOLID, 5, RGB(113,171,255));
	pen_disble.CreatePen(PS_SOLID, 3, RGB(60,60,60));
	pen_comp.CreatePen(PS_SOLID, 3, RGB(0,200,0));
	pen_non_comp.CreatePen(PS_SOLID, 3, RGB(128,100,80));
	oldpen = hMemDc.SelectObject(&pen);

	CBrush brush_comp, brush_disable, brush_non_comp, *old_brush;
	brush_disable.CreateSolidBrush(RGB(160, 160, 160));
	brush_comp.CreateSolidBrush(RGB(100, 255, 100));
	brush_non_comp.CreateSolidBrush(RGB(253,234,218));
	old_brush = hMemDc.SelectObject(&brush_comp);

	// Draw Disable Finger
	hMemDc.SelectObject(brush_disable);
	hMemDc.SelectObject(&pen_disble);
	for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
	{
		if( IsEnableFinger(i) == TRUE )
			continue;

		RoundRect(hMemDc, m_FingerRect[i].left, m_FingerRect[i].top, m_FingerRect[i].right, m_FingerRect[i].bottom, 18, 18);
		hMemDc.MoveTo(m_FingerRect[i].left+3, m_FingerRect[i].top+5);
		hMemDc.LineTo(m_FingerRect[i].right-3, m_FingerRect[i].bottom-5);
		hMemDc.MoveTo(m_FingerRect[i].right-3, m_FingerRect[i].top+5);
		hMemDc.LineTo(m_FingerRect[i].left+3, m_FingerRect[i].bottom-5);
	}

	// Draw Selected Finger (check icon)
	hMemDc.SelectObject(&pen_select);
	for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
	{
		if( IsEnableFinger(i) == FALSE )
			continue;

		if( GetFingerSelected(i) == STATE_SELECTED )
		{
			hMemDc.MoveTo((m_FingerRect[i].left+m_FingerRect[i].right)/2-5, m_FingerRect[i].top-12);
			hMemDc.LineTo((m_FingerRect[i].left+m_FingerRect[i].right)/2, m_FingerRect[i].top-4);
			hMemDc.LineTo((m_FingerRect[i].left+m_FingerRect[i].right)/2+5, m_FingerRect[i].top-20);
		}
	}

	// Draw Completed Finger
	hMemDc.SelectObject(&brush_comp);
	hMemDc.SelectObject(pen_comp);
	for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
	{
		if( IsEnableFinger(i) == FALSE )
			continue;
		
		if( GetFingerCompleted(i) == STATE_COMPLETED )
		{
			hMemDc.SelectObject(pen_comp);
			hMemDc.SelectObject(&brush_comp);
		}
		else
		{
			hMemDc.SelectObject(pen_non_comp);
			hMemDc.SelectObject(brush_non_comp);
		}

		RoundRect(hMemDc, m_FingerRect[i].left, m_FingerRect[i].top, m_FingerRect[i].right, m_FingerRect[i].bottom, 18, 18);
	}

	// Draw Palm : only for display
	hMemDc.SelectObject(pen_non_comp);
	hMemDc.SelectObject(brush_non_comp);
	for(i=0; i<=1; i++)
	{
		RoundRect(hMemDc, m_PalmRect[i].left, m_PalmRect[i].top, m_PalmRect[i].right, m_PalmRect[i].bottom, 36, 36);
	}	
	
	hMemDc.SelectObject(old_brush);
	hMemDc.SelectObject(oldpen);

	BitBlt(dc.m_hDC, 0, 0, Width, Height, hMemDc.m_hDC, 0, 0, SRCCOPY);
	hMemDc.SelectObject(hOldBmp);

	pen.DeleteObject();
	pen_select.DeleteObject();
	pen_disble.DeleteObject();
	pen_non_comp.DeleteObject();
	pen_comp.DeleteObject();
	brush_disable.DeleteObject();
	brush_comp.DeleteObject();
	brush_non_comp.DeleteObject();
	hMemDc.DeleteDC();
	hBmpBuffer.DeleteObject();
}

BOOL CFingerDisplayManager::SetFingerSelected(int index, BYTE state)
{
	if( m_FingerCaptureMode == ONE_FINGER_MODE ) 
	{
		if(index < LEFT_LITTLE || index > RIGHT_LITTLE)
			return FALSE;

		if(m_FingerEnable[index] == TRUE)
			m_FingerState[index] |= state;
	}
	else if( m_FingerCaptureMode == TWO_FINGER_MODE ) 
	{
		if(index < LEFT_RING_LITTLE || index > RIGHT_RING_LITTLE)
			return FALSE;

		if(m_FingerEnable[(index-LEFT_RING_LITTLE)*2] == TRUE)
			m_FingerState[(index-LEFT_RING_LITTLE)*2] |= state;
		
		if(m_FingerEnable[(index-LEFT_RING_LITTLE)*2+1] == TRUE)
			m_FingerState[(index-LEFT_RING_LITTLE)*2+1] |= state;
	}

	UpdateDisplayWindow();

	return TRUE;
}

BYTE CFingerDisplayManager::GetFingerSelected(int index)
{
	if(index < LEFT_LITTLE || index > RIGHT_LITTLE)
		return FALSE;

	return m_FingerState[index] & 0x0F;
}

BOOL CFingerDisplayManager::SetFingerCompleted(int index, BYTE state)
{
	if(m_FingerCaptureMode == ONE_FINGER_MODE)
	{
		if(index < LEFT_LITTLE || index > RIGHT_LITTLE)
			return FALSE;

		m_FingerState[index] = m_FingerState[index] & 0x0F;
		m_FingerState[index] |= state << 4;
	}
	else if(m_FingerCaptureMode == TWO_FINGER_MODE)
	{
		if(index < LEFT_RING_LITTLE || index > RIGHT_RING_LITTLE)
			return FALSE;

		if(m_FingerEnable[(index-LEFT_RING_LITTLE)*2] == TRUE)
		{
			m_FingerState[(index-LEFT_RING_LITTLE)*2] = m_FingerState[(index-LEFT_RING_LITTLE)*2] & 0x0F;
			m_FingerState[(index-LEFT_RING_LITTLE)*2] |= state << 4;
		}
		
		if(m_FingerEnable[(index-LEFT_RING_LITTLE)*2+1] == TRUE)
		{
			m_FingerState[(index-LEFT_RING_LITTLE)*2+1] = m_FingerState[(index-LEFT_RING_LITTLE)*2+1] & 0x0F;
			m_FingerState[(index-LEFT_RING_LITTLE)*2+1] |= state << 4;
		}
	}

	UpdateDisplayWindow();

	return TRUE;
}

BYTE CFingerDisplayManager::GetFingerCompleted(int index)
{
	if(index < LEFT_LITTLE || index > RIGHT_LITTLE)
		return FALSE;

	return (m_FingerState[index] & 0xF0) >> 4;
}

BOOL CFingerDisplayManager::SelectFinger(CPoint point)
{
	int i;

	for(i=0; i<10; i++)
	{
		m_FingerState[i] = m_FingerState[i] & 0xF0;
	}

	if(m_FingerCaptureMode == ONE_FINGER_MODE)
	{
		for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
		{
			if(m_FingerRect[i].PtInRect(point) == TRUE)
			{
				SetFingerSelected(i, STATE_SELECTED);
				break;
			}
		}
	}
	else if(m_FingerCaptureMode == TWO_FINGER_MODE)
	{
		for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i+=2)
		{
			if(m_FingerRect[i].PtInRect(point) == TRUE || m_FingerRect[i+1].PtInRect(point) == TRUE)
			{
				SetFingerSelected(i/2+LEFT_RING_LITTLE, STATE_SELECTED);
				break;
			}
		}
	}
	
	UpdateDisplayWindow();

	return TRUE;
}

BOOL CFingerDisplayManager::SelectFinger(BYTE index)
{
	int i;

	for(i=0; i<10; i++)
	{
		m_FingerState[i] = m_FingerState[i] & 0xF0;
	}

	if(m_FingerCaptureMode == ONE_FINGER_MODE)
	{
		if(index < LEFT_LITTLE || index > RIGHT_LITTLE)
			return FALSE;

		for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
		{
			if(i == index)
				SetFingerSelected(i, STATE_SELECTED);
		}
	}
	else if(m_FingerCaptureMode == TWO_FINGER_MODE)
	{
		if(index < LEFT_RING_LITTLE || index > RIGHT_RING_LITTLE)
			return FALSE;

		for(i=LEFT_RING_LITTLE; i<=RIGHT_RING_LITTLE; i++)
		{
			if(i == index)
				SetFingerSelected(i, STATE_SELECTED);
		}
	}

	UpdateDisplayWindow();

	return TRUE;
}

BYTE CFingerDisplayManager::GetSelectedFingerIndex()
{
	int i;

	if(m_FingerCaptureMode == ONE_FINGER_MODE)
	{
		for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
		{
			if( GetFingerSelected(i) == STATE_SELECTED )
				return i;
		}
	}
	else if(m_FingerCaptureMode == TWO_FINGER_MODE)
	{
		for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
		{
			if( GetFingerSelected(i) == STATE_SELECTED )
				return i/2 + LEFT_RING_LITTLE;
		}
	}

	return NONE_FINGER;
}

BOOL CFingerDisplayManager::ToggleEnableFinger(CPoint point)
{
	int i;

	for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
	{
		if(m_FingerRect[i].PtInRect(point) == TRUE)
		{
			m_FingerEnable[i] = !m_FingerEnable[i];

			if(m_FingerEnable[i] == FALSE)
				m_FingerState[i] = m_FingerState[i] & 0xF0;

			break;
		}
	}
	
	UpdateDisplayWindow();

	return TRUE;
}

BOOL CFingerDisplayManager::ToggleEnableFinger(BYTE index)
{
	if(index < LEFT_LITTLE || index > RIGHT_LITTLE)
		return FALSE;

	m_FingerEnable[index] = TRUE;

	UpdateDisplayWindow();

	return TRUE;	
}

BOOL CFingerDisplayManager::IsEnableFinger(BYTE index)
{
	if(index < LEFT_LITTLE || index > RIGHT_LITTLE)
		return FALSE;

	return m_FingerEnable[index];
}

void CFingerDisplayManager::SetCaptureMode(BYTE Capture_Mode)
{
	m_FingerCaptureMode = Capture_Mode;
}
