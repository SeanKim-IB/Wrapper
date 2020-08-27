#include "stdafx.h"
#include "IBScanUltimate_SalesDemo.h"
#include "IBScanUltimate_SalesDemoDlg.h"
#include "FingerDisplayManager.h"
#include "IBScanUltimateApi.h"
#include "IBScanUltimateApi_defs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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

	// 240 x 160
	/*m_FingerRect[LEFT_LITTLE] = CRect(8, 47, 28, 107); // left little17
	m_FingerRect[LEFT_RING] = CRect(29, 31, 49, 104); // left ring17
	m_FingerRect[LEFT_MIDDLE] = CRect(49, 25, 69, 104); // left middle17
	m_FingerRect[LEFT_INDEX] = CRect(69, 34, 89, 104); // left index17
	m_FingerRect[LEFT_THUMB] = CRect(90, 67, 110, 107); // left Thumb20

	m_FingerRect[RIGHT_THUMB] = CRect(240-112, 67, 240-92, 107); // left Thumb
	m_FingerRect[RIGHT_INDEX] = CRect(240-91, 34, 240-71, 104); // left index
	m_FingerRect[RIGHT_MIDDLE] = CRect(240-71, 25, 240-51, 104); // left middle
	m_FingerRect[RIGHT_RING] = CRect(240-51, 31, 240-31, 104); // left ring
	m_FingerRect[RIGHT_LITTLE] = CRect(240-31, 47, 240-11, 107); // left little*/

	//300 x 160
	m_FingerRect[LEFT_LITTLE] = CRect(26, 37, 45, 99); // left little19
	m_FingerRect[LEFT_RING] = CRect(46, 21, 65, 96); // left ring19
	m_FingerRect[LEFT_MIDDLE] = CRect(66, 15, 85, 96); // left middle19
	m_FingerRect[LEFT_INDEX] = CRect(86, 24, 105, 96); // left index19
	m_FingerRect[LEFT_THUMB] = CRect(106, 57, 128, 99); // left Thumb22

	m_FingerRect[RIGHT_THUMB] = CRect(300-132, 57, 300-110, 99); // left Thumb
	m_FingerRect[RIGHT_INDEX] = CRect(300-109, 24, 300-90, 96); // left index
	m_FingerRect[RIGHT_MIDDLE] = CRect(300-89, 15, 300-70, 96); // left middle
	m_FingerRect[RIGHT_RING] = CRect(300-69, 21, 300-50, 96); // left ring
	m_FingerRect[RIGHT_LITTLE] = CRect(300-49, 37, 300-30, 99); // left little

	m_PalmRect[0] = CRect(29, 97, 118, 147);
	m_PalmRect[1] = CRect(300-122, 97, 300-33, 147);
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
	int Width = 300;
	int Height = 150;
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
	pen_select.CreatePen(PS_SOLID, 3, RGB(0,200,0));
	pen_disble.CreatePen(PS_SOLID, 3, RGB(60,60,60));
	pen_comp.CreatePen(PS_SOLID, 3, RGB(128,100,80));
	pen_non_comp.CreatePen(PS_SOLID, 3, RGB(128,100,80));
	oldpen = hMemDc.SelectObject(&pen);

	//brush_select add
	CBrush brush_comp, brush_disable, brush_non_comp,brush_select, *old_brush ;
	brush_disable.CreateSolidBrush(RGB(160, 160, 160));
	brush_select.CreateSolidBrush(RGB(100,255,100));
	brush_comp.CreateSolidBrush(RGB(253, 234, 218));
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

	// Draw Selected Finger
	hMemDc.SelectObject(&brush_select);
	hMemDc.SelectObject(pen_select);
	for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
	{
		if( IsEnableFinger(i) == FALSE )
			continue;

		if( GetFingerSelected(i) == STATE_SELECTED )
		{
			hMemDc.SelectObject(pen_select);
			hMemDc.SelectObject(&brush_select);
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
	brush_select.DeleteObject(); //
	brush_comp.DeleteObject();
	brush_non_comp.DeleteObject();
	hMemDc.DeleteDC();
	hBmpBuffer.DeleteObject();
}

BOOL CFingerDisplayManager::SetFingerSelected(int index, BYTE state)
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
	else if(m_FingerCaptureMode == FOUR_FINGER_MODE)
	{
		if(index != LEFT_HAND && index != RIGHT_HAND && index !=BOTH_THUMBS && index !=LEFT_THUMB && index !=RIGHT_THUMB)
			return FALSE;

		switch(index)
		{
		case LEFT_HAND:

			if(m_FingerEnable[LEFT_INDEX] ==  TRUE)
				m_FingerState[LEFT_INDEX] = m_FingerState[LEFT_INDEX] & 0x0F;
				m_FingerState[LEFT_INDEX] |= state << 4;
			if(m_FingerEnable[LEFT_MIDDLE] ==  TRUE)
				m_FingerState[LEFT_MIDDLE] = m_FingerState[LEFT_MIDDLE] & 0x0F;
				m_FingerState[LEFT_MIDDLE] |= state << 4;
			if(m_FingerEnable[LEFT_RING] ==  TRUE)
				m_FingerState[LEFT_RING] = m_FingerState[LEFT_RING] & 0x0F;
				m_FingerState[LEFT_RING] |= state << 4;
			if(m_FingerEnable[LEFT_LITTLE] ==  TRUE)
				m_FingerState[LEFT_LITTLE] = m_FingerState[LEFT_LITTLE] & 0x0F;
				m_FingerState[LEFT_LITTLE] |= state << 4;
			break;

		case RIGHT_HAND:
			if(m_FingerEnable[RIGHT_INDEX] ==  TRUE)
				m_FingerState[RIGHT_INDEX] = m_FingerState[RIGHT_INDEX] & 0x0F;
				m_FingerState[RIGHT_INDEX] |= state << 4;
			if(m_FingerEnable[RIGHT_MIDDLE] ==  TRUE)
				m_FingerState[RIGHT_MIDDLE] = m_FingerState[RIGHT_MIDDLE] & 0x0F;
				m_FingerState[RIGHT_MIDDLE] |= state << 4;
			if(m_FingerEnable[RIGHT_RING] ==  TRUE)
				m_FingerState[RIGHT_RING] = m_FingerState[RIGHT_RING] & 0x0F;
				m_FingerState[RIGHT_RING] |= state << 4;
			if(m_FingerEnable[RIGHT_LITTLE] ==  TRUE)
				m_FingerState[RIGHT_LITTLE] = m_FingerState[RIGHT_LITTLE] & 0x0F;
				m_FingerState[RIGHT_LITTLE] |= state << 4;
			break;

		case BOTH_THUMBS:
			if(m_FingerEnable[RIGHT_THUMB] ==  TRUE)
				m_FingerState[RIGHT_THUMB] = m_FingerState[RIGHT_THUMB] & 0x0F;
				m_FingerState[RIGHT_THUMB] |= state << 4;
			if(m_FingerEnable[LEFT_THUMB] ==  TRUE)
				m_FingerState[LEFT_THUMB] = m_FingerState[LEFT_THUMB] & 0x0F;
				m_FingerState[LEFT_THUMB] |= state << 4;
			break;
		case LEFT_THUMB:
			if(m_FingerEnable[LEFT_THUMB] ==  TRUE)
			{
				m_FingerState[LEFT_THUMB] = m_FingerState[LEFT_THUMB] & 0x0F;
				m_FingerState[LEFT_THUMB] |= state << 4;
			}
			break;
		case RIGHT_THUMB:
			if(m_FingerEnable[RIGHT_THUMB] ==  TRUE)
			{
				m_FingerState[RIGHT_THUMB] = m_FingerState[RIGHT_THUMB] & 0x0F;
				m_FingerState[RIGHT_THUMB] |= state << 4;
				break;
			}
			break;
		}
	}
	else if(m_FingerCaptureMode == FOUR_ROLL_FINGER_MODE)
	{
		if(index != LEFT_HAND && index != RIGHT_HAND && index !=BOTH_THUMBS && index != LEFT_THUMB && index != LEFT_INDEX
			&& index != LEFT_MIDDLE && index != LEFT_RING && index != LEFT_LITTLE && index != RIGHT_THUMB && index != RIGHT_INDEX
			&& index != RIGHT_MIDDLE && index != RIGHT_RING && index != RIGHT_LITTLE )
			return FALSE;

		switch(index)
		{
		case LEFT_HAND:

			if(m_FingerEnable[LEFT_INDEX] ==  TRUE)
				m_FingerState[LEFT_INDEX] = m_FingerState[LEFT_INDEX] & 0x0F;
				m_FingerState[LEFT_INDEX] |= state << 4;
			if(m_FingerEnable[LEFT_MIDDLE] ==  TRUE)
				m_FingerState[LEFT_MIDDLE] = m_FingerState[LEFT_MIDDLE] & 0x0F;
				m_FingerState[LEFT_MIDDLE] |= state << 4;
			if(m_FingerEnable[LEFT_RING] ==  TRUE)
				m_FingerState[LEFT_RING] = m_FingerState[LEFT_RING] & 0x0F;
				m_FingerState[LEFT_RING] |= state << 4;
			if(m_FingerEnable[LEFT_LITTLE] ==  TRUE)
				m_FingerState[LEFT_LITTLE] = m_FingerState[LEFT_LITTLE] & 0x0F;
				m_FingerState[LEFT_LITTLE] |= state << 4;
			break;

		case RIGHT_HAND:
			if(m_FingerEnable[RIGHT_INDEX] ==  TRUE)
				m_FingerState[RIGHT_INDEX] = m_FingerState[RIGHT_INDEX] & 0x0F;
				m_FingerState[RIGHT_INDEX] |= state << 4;
			if(m_FingerEnable[RIGHT_MIDDLE] ==  TRUE)
				m_FingerState[RIGHT_MIDDLE] = m_FingerState[RIGHT_MIDDLE] & 0x0F;
				m_FingerState[RIGHT_MIDDLE] |= state << 4;
			if(m_FingerEnable[RIGHT_RING] ==  TRUE)
				m_FingerState[RIGHT_RING] = m_FingerState[RIGHT_RING] & 0x0F;
				m_FingerState[RIGHT_RING] |= state << 4;
			if(m_FingerEnable[RIGHT_LITTLE] ==  TRUE)
				m_FingerState[RIGHT_LITTLE] = m_FingerState[RIGHT_LITTLE] & 0x0F;
				m_FingerState[RIGHT_LITTLE] |= state << 4;
			break;

		case BOTH_THUMBS:
			if(m_FingerEnable[RIGHT_THUMB] ==  TRUE)
				m_FingerState[RIGHT_THUMB] = m_FingerState[RIGHT_THUMB] & 0x0F;
				m_FingerState[RIGHT_THUMB] |= state << 4;
			if(m_FingerEnable[LEFT_THUMB] ==  TRUE)
				m_FingerState[LEFT_THUMB] = m_FingerState[LEFT_THUMB] & 0x0F;
				m_FingerState[LEFT_THUMB] |= state << 4;
			break;
		default:
			m_FingerState[index] = m_FingerState[index] & 0x0F;
			m_FingerState[index] |= state << 4;	
		}
	}
	else if(m_FingerCaptureMode == ALL_FINGER_MODE)
	{
		if(index == BOTH_LEFT_THUMB)
		{
			index = LEFT_THUMB;
		}
		if(index == BOTH_RIGHT_THUMB)
		{
			index = RIGHT_THUMB;
		}

		if(index < LEFT_LITTLE || index >RIGHT_RING_LITTLE)
			return FALSE;

		if(index <= RIGHT_LITTLE)
		{
			if(m_FingerEnable[index] == TRUE)
			{
				m_FingerState[index] = m_FingerState[index] & 0x0F;
				m_FingerState[index] |= state << 4;
			}
		}else //if(index >RIGHT_LITTLE && index <= RIGHT_RING_LITTLE)
		{
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
	}

	UpdateDisplayWindow();

	return TRUE;
}

BYTE CFingerDisplayManager::GetFingerSelected(int index)
{
	/*if(index < LEFT_LITTLE || index > RIGHT_LITTLE)
	return FALSE;

	return m_FingerState[index] & 0x0F;*/
	if(index < LEFT_LITTLE || index > RIGHT_LITTLE )
		return FALSE;

	return (m_FingerState[index] & 0xF0) >> 4;
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
	else if(m_FingerCaptureMode == FOUR_FINGER_MODE)
	{
		if(index != LEFT_HAND && index != RIGHT_HAND && index !=BOTH_THUMBS)
			return FALSE;
		if(index == LEFT_HAND)
		{
			m_FingerState[LEFT_INDEX] = m_FingerState[LEFT_INDEX] & 0x0F;
			m_FingerState[LEFT_INDEX] |= state << 4;
			m_FingerState[LEFT_MIDDLE] = m_FingerState[LEFT_MIDDLE] & 0x0F;
			m_FingerState[LEFT_MIDDLE] |= state << 4;
			m_FingerState[LEFT_RING] = m_FingerState[LEFT_RING] & 0x0F;
			m_FingerState[LEFT_RING] |= state << 4;
			m_FingerState[LEFT_LITTLE] = m_FingerState[LEFT_LITTLE] & 0x0F;
			m_FingerState[LEFT_LITTLE] |= state << 4;
		}
		else if(index == RIGHT_HAND)
		{
			m_FingerState[RIGHT_INDEX] = m_FingerState[RIGHT_INDEX] & 0x0F;
			m_FingerState[RIGHT_INDEX] |= state << 4;
			m_FingerState[RIGHT_MIDDLE] = m_FingerState[RIGHT_MIDDLE] & 0x0F;
			m_FingerState[RIGHT_MIDDLE] |= state << 4;
			m_FingerState[RIGHT_RING] = m_FingerState[RIGHT_RING] & 0x0F;
			m_FingerState[RIGHT_RING] |= state << 4;
			m_FingerState[RIGHT_LITTLE] = m_FingerState[RIGHT_LITTLE] & 0x0F;
			m_FingerState[RIGHT_LITTLE] |= state << 4;
		}
		else if(index == BOTH_THUMBS)
		{
			m_FingerState[RIGHT_THUMB] = m_FingerState[RIGHT_THUMB] & 0x0F;
			m_FingerState[RIGHT_THUMB] |= state << 4;
			m_FingerState[LEFT_THUMB] = m_FingerState[LEFT_THUMB] & 0x0F;
			m_FingerState[LEFT_THUMB] |= state << 4;
		}
	}
	else if(m_FingerCaptureMode == FOUR_ROLL_FINGER_MODE)
	{
		if(index != LEFT_HAND && index != RIGHT_HAND && index !=BOTH_THUMBS && index != LEFT_THUMB && index != LEFT_INDEX
			&& index != LEFT_MIDDLE && index != LEFT_RING && index != LEFT_LITTLE && index != RIGHT_THUMB && index != RIGHT_INDEX
			&& index != RIGHT_MIDDLE && index != RIGHT_RING && index != RIGHT_LITTLE )
			return FALSE;
		if(index == LEFT_HAND)
		{
			m_FingerState[LEFT_INDEX] = m_FingerState[LEFT_INDEX] & 0x0F;
			m_FingerState[LEFT_INDEX] |= state << 4;
			m_FingerState[LEFT_MIDDLE] = m_FingerState[LEFT_MIDDLE] & 0x0F;
			m_FingerState[LEFT_MIDDLE] |= state << 4;
			m_FingerState[LEFT_RING] = m_FingerState[LEFT_RING] & 0x0F;
			m_FingerState[LEFT_RING] |= state << 4;
			m_FingerState[LEFT_LITTLE] = m_FingerState[LEFT_LITTLE] & 0x0F;
			m_FingerState[LEFT_LITTLE] |= state << 4;
		}
		else if(index == RIGHT_HAND)
		{
			m_FingerState[RIGHT_INDEX] = m_FingerState[RIGHT_INDEX] & 0x0F;
			m_FingerState[RIGHT_INDEX] |= state << 4;
			m_FingerState[RIGHT_MIDDLE] = m_FingerState[RIGHT_MIDDLE] & 0x0F;
			m_FingerState[RIGHT_MIDDLE] |= state << 4;
			m_FingerState[RIGHT_RING] = m_FingerState[RIGHT_RING] & 0x0F;
			m_FingerState[RIGHT_RING] |= state << 4;
			m_FingerState[RIGHT_LITTLE] = m_FingerState[RIGHT_LITTLE] & 0x0F;
			m_FingerState[RIGHT_LITTLE] |= state << 4;
		}
		else if(index == BOTH_THUMBS)
		{
			m_FingerState[RIGHT_THUMB] = m_FingerState[RIGHT_THUMB] & 0x0F;
			m_FingerState[RIGHT_THUMB] |= state << 4;
			m_FingerState[LEFT_THUMB] = m_FingerState[LEFT_THUMB] & 0x0F;
			m_FingerState[LEFT_THUMB] |= state << 4;
		}else
		{
			m_FingerState[index] = m_FingerState[index] & 0x0F;
			m_FingerState[index] |= state << 4;
		}
	}
	else if(m_FingerCaptureMode == ALL_FINGER_MODE)
	{
		if(index == BOTH_LEFT_THUMB)
		{
			index = LEFT_THUMB;
		}
		if(index == BOTH_RIGHT_THUMB)
		{
			index = RIGHT_THUMB;
		}

		if(index < LEFT_LITTLE || index >RIGHT_RING_LITTLE)
			return FALSE;

		if(index <= RIGHT_LITTLE)
		{
			if(m_FingerEnable[index] == TRUE)
			{
				m_FingerState[index] = m_FingerState[index] & 0x0F;
				m_FingerState[index] |= state << 4;
			}
		}else// if(index >RIGHT_LITTLE && index < RIGHT_RING_LITTLE)
		{
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
	else if(m_FingerCaptureMode == FOUR_FINGER_MODE)
	{
		if(m_FingerRect[LEFT_INDEX].PtInRect(point) == TRUE || m_FingerRect[LEFT_MIDDLE].PtInRect(point) == TRUE || m_FingerRect[LEFT_RING].PtInRect(point) == TRUE || m_FingerRect[LEFT_LITTLE].PtInRect(point) == TRUE)
		{
			SetFingerSelected(LEFT_HAND, STATE_SELECTED);
		}
		else if(m_FingerRect[RIGHT_INDEX].PtInRect(point) == TRUE || m_FingerRect[RIGHT_MIDDLE].PtInRect(point) == TRUE || m_FingerRect[RIGHT_RING].PtInRect(point) == TRUE || m_FingerRect[RIGHT_LITTLE].PtInRect(point) == TRUE)
		{
			SetFingerSelected(RIGHT_HAND, STATE_SELECTED);
		}
		else if(m_FingerRect[LEFT_THUMB].PtInRect(point) == TRUE || m_FingerRect[RIGHT_THUMB].PtInRect(point) == TRUE)
		{
			SetFingerSelected(BOTH_THUMBS, STATE_SELECTED);
		}
	}
	else if(m_FingerCaptureMode == FOUR_ROLL_FINGER_MODE)
	{
		for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
		{
			if(m_FingerRect[i].PtInRect(point) == TRUE)
			{
				SetFingerSelected(i, STATE_SELECTED);
				break;
			}
		}

		if(m_FingerRect[LEFT_INDEX].PtInRect(point) == TRUE || m_FingerRect[LEFT_MIDDLE].PtInRect(point) == TRUE || m_FingerRect[LEFT_RING].PtInRect(point) == TRUE || m_FingerRect[LEFT_LITTLE].PtInRect(point) == TRUE)
		{
			SetFingerSelected(LEFT_HAND, STATE_SELECTED);
		}
		else if(m_FingerRect[RIGHT_INDEX].PtInRect(point) == TRUE || m_FingerRect[RIGHT_MIDDLE].PtInRect(point) == TRUE || m_FingerRect[RIGHT_RING].PtInRect(point) == TRUE || m_FingerRect[RIGHT_LITTLE].PtInRect(point) == TRUE)
		{
			SetFingerSelected(RIGHT_HAND, STATE_SELECTED);
		}
		else if(m_FingerRect[LEFT_THUMB].PtInRect(point) == TRUE || m_FingerRect[RIGHT_THUMB].PtInRect(point) == TRUE)
		{
			SetFingerSelected(BOTH_THUMBS, STATE_SELECTED);
		}
	}
	else if( m_FingerCaptureMode == ALL_FINGER_MODE)
	{
		for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
		{
			if(m_FingerRect[i].PtInRect(point) == TRUE)
			{
				SetFingerSelected(i, STATE_SELECTED);
				break;
			}
		}

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
	int i,k;

	for(i=0; i<10; i++)
	{
		m_FingerState[i] = m_FingerState[i] & 0xC0;
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
	else if(m_FingerCaptureMode == FOUR_FINGER_MODE)
	{

		if(index != LEFT_HAND && index != RIGHT_HAND && index !=BOTH_THUMBS && index !=LEFT_THUMB && index !=RIGHT_THUMB)
			return FALSE;
		
		//int k;
		if(index == LEFT_THUMB || index == RIGHT_THUMB)
		{
			for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
			{
				if(i == index)
					SetFingerSelected(i, STATE_SELECTED);
			}
		}
		else
		{
			int arrf[3]={LEFT_HAND,BOTH_THUMBS,RIGHT_HAND};
			for(k = 0; k <=2 ; k++)
			{
				if(arrf[k] == index)
				SetFingerSelected(arrf[k], STATE_SELECTED);
			}
		}
	}
	else if(m_FingerCaptureMode == FOUR_ROLL_FINGER_MODE)
	{
		if(index != LEFT_HAND && index != RIGHT_HAND && index !=BOTH_THUMBS && index != LEFT_THUMB && index != LEFT_INDEX
			&& index != LEFT_MIDDLE && index != LEFT_RING && index != LEFT_LITTLE && index != RIGHT_THUMB && index != RIGHT_INDEX
			&& index != RIGHT_MIDDLE && index != RIGHT_RING && index != RIGHT_LITTLE)
			return FALSE;
		
//		if(index < LEFT_LITTLE || index > RIGHT_LITTLE)
//			return FALSE;

		//int k;
		int arrf[13]={LEFT_HAND,BOTH_THUMBS,RIGHT_HAND,LEFT_THUMB,LEFT_INDEX
			, LEFT_MIDDLE ,LEFT_RING ,LEFT_LITTLE ,RIGHT_THUMB, RIGHT_INDEX
			, RIGHT_MIDDLE , RIGHT_RING ,RIGHT_LITTLE};
		for(k = 0; k <=12 ; k++)
		{
			if(arrf[k] == index)
			SetFingerSelected(arrf[k], STATE_SELECTED);
		}
		
	}
	else if(m_FingerCaptureMode ==  ALL_FINGER_MODE)
	{
		if(index == BOTH_LEFT_THUMB)
		{
			index = LEFT_THUMB;
		}
		if(index == BOTH_RIGHT_THUMB)
		{
			index = RIGHT_THUMB;
		}

		if(index >= LEFT_LITTLE && index <= RIGHT_LITTLE)
		{
			for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
			{
				if(i == index)
					SetFingerSelected(i, STATE_SELECTED);
			}

		}else// if(index >LEFT_RING_LITTLE && index >RIGHT_RING_LITTLE )
		{
			for(i=LEFT_RING_LITTLE; i<=RIGHT_RING_LITTLE; i++)
			{
				if(i == index)
					SetFingerSelected(i, STATE_SELECTED);
			}
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

	else if(m_FingerCaptureMode == FOUR_FINGER_MODE)
	{
		if(GetFingerSelected(LEFT_INDEX) == STATE_SELECTED || GetFingerSelected(LEFT_MIDDLE) == STATE_SELECTED || GetFingerSelected(LEFT_RING) == STATE_SELECTED || GetFingerSelected(LEFT_LITTLE) == STATE_SELECTED)
			return LEFT_HAND;
		else if(GetFingerSelected(RIGHT_INDEX) == STATE_SELECTED || GetFingerSelected(RIGHT_MIDDLE) == STATE_SELECTED || GetFingerSelected(RIGHT_RING) == STATE_SELECTED || GetFingerSelected(RIGHT_LITTLE) == STATE_SELECTED)
			return RIGHT_HAND;
		else if(GetFingerSelected(RIGHT_THUMB) == STATE_SELECTED || GetFingerSelected(LEFT_THUMB) == STATE_SELECTED)
			return BOTH_THUMBS;
	}
	else if(m_FingerCaptureMode == FOUR_ROLL_FINGER_MODE)
	{
		if(GetFingerSelected(LEFT_INDEX) == STATE_SELECTED || GetFingerSelected(LEFT_MIDDLE) == STATE_SELECTED || GetFingerSelected(LEFT_RING) == STATE_SELECTED || GetFingerSelected(LEFT_LITTLE) == STATE_SELECTED)
			return LEFT_HAND;
		else if(GetFingerSelected(RIGHT_INDEX) == STATE_SELECTED || GetFingerSelected(RIGHT_MIDDLE) == STATE_SELECTED || GetFingerSelected(RIGHT_RING) == STATE_SELECTED || GetFingerSelected(RIGHT_LITTLE) == STATE_SELECTED)
			return RIGHT_HAND;
		else if(GetFingerSelected(RIGHT_THUMB) == STATE_SELECTED || GetFingerSelected(LEFT_THUMB) == STATE_SELECTED)
			return BOTH_THUMBS;
		else 
		{
			for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
			{
				if( GetFingerSelected(i) == STATE_SELECTED )
					return i;
			}
		}
	}
	else if(m_FingerCaptureMode == ALL_FINGER_MODE)
	{
		for(i=LEFT_LITTLE; i<=RIGHT_LITTLE; i++)
		{
			if( GetFingerSelected(i) == STATE_SELECTED )
				return i;
		}

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
			{
				m_FingerState[i] = m_FingerState[i] & 0xF0;
			}

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
