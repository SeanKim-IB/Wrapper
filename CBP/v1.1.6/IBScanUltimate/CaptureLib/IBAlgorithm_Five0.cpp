/////////////////////////////////////////////////////////////////////////////
/*
DESCRIPTION:
	CMainCapture - Class for getting image capture core from Live Scanner
	http://www.integratedbiometrics.com/

NOTES:
	Copyright(C) Integrated Biometrics, 2011

VERSION HISTORY:
	19 April 2011 - First initialize
*/
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IBAlgorithm.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#ifdef __linux__
#include <sys/time.h>
#endif

/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Five0 Algorithm



BOOL CIBAlgorithm::_Algo_Five0_AutoCapture(BYTE *InImg, CaptureSetting *pSetting, int frameTime, int ImgSize, int *CaptureGood)
{
//	int	ForgraoundCNT=0, fingerCount=0, AreaTres, RecomGain;
	int			AreaTres;
	int			FlatDiffThres;
	const int	MAX_BRIGHT = pSetting->TargetMaxBrightness, MIN_BRIGHT = pSetting->TargetMinBrightness;
	const int	MIN_CAPTURE_TIME = 600;
	int			MIN_CAPTURE_TIME_FOR_DRY = 600;
	const int	TOLERANCE = 30;
	const int	MAX_STEP_CHANGE = 6;	// default 3
//	int			forSuperDry_Mode=pSetting->forSuperDry_Mode;
	int			REQUIRED_DETECT_FRAME_COUNT = pSetting->DetectionFrameCount;
	int			updated_MIN_BRIGHT = MIN_BRIGHT, updated_MAX_BRIGHT = MAX_BRIGHT;
    int         brightPixelRate;
    int         DAC_MINUS_STEP = 4;


    brightPixelRate = (m_cImgAnalysis.foreground_count2==0)? 0: (int)(m_cImgAnalysis.bright_pixel_count*100)/(m_cImgAnalysis.foreground_count2);
	AreaTres = SINGLE_FLAT_AREA_TRES * m_cImgAnalysis.finger_count;
	if (m_cImgAnalysis.foreground_count2 > m_cImgAnalysis.bright_pixel_count && m_cImgAnalysis.foreground_count >= AreaTres)
	{
		if (brightPixelRate < 10)
		{
			DAC_MINUS_STEP = 12;
		}
		else if (brightPixelRate < 20)
		{
			DAC_MINUS_STEP = 10;
		}
		else if (brightPixelRate < 30)
		{
			DAC_MINUS_STEP = 8;
		}
		else if (brightPixelRate < 40)
		{
			DAC_MINUS_STEP = 4;
		}
		else if (brightPixelRate < 50)
		{
			DAC_MINUS_STEP = 3;
		}
		else if (brightPixelRate < 60)
		{
			DAC_MINUS_STEP = 2;
		}
		else
		{
			DAC_MINUS_STEP = 1;
		}

		if ((m_cImgAnalysis.finger_count > pSetting->TargetFingerCount && brightPixelRate > 40 && m_cImgAnalysis.mean > 25) ||
		    (m_cImgAnalysis.finger_count < pSetting->TargetFingerCount && brightPixelRate > 40 && m_cImgAnalysis.mean > 25))
		{
			pSetting->CurrentDAC += 8;

			if (pSetting->CurrentDAC > pSetting->DefaultMinDAC)
			{
				pSetting->CurrentDAC = pSetting->DefaultMinDAC;
			}

			return FALSE;
		}

	}

    if(m_pPropertyInfo->nWaitTimeForDry > 0)
		MIN_CAPTURE_TIME_FOR_DRY = m_pPropertyInfo->nWaitTimeForDry;

	*CaptureGood = FALSE;

	m_cImgAnalysis.max_same_gain_count = 3;
	//if (m_cImgAnalysis.foreground_count < 100)
	if (m_cImgAnalysis.isDetected == 0)
	{
		pSetting->CurrentGain = pSetting->DefaultGain;
		pSetting->CurrentDAC = pSetting->DefaultDAC;
		m_cImgAnalysis.frame_delay = 2;
		m_cImgAnalysis.good_frame_count = 0;
		m_cImgAnalysis.is_final = FALSE;
		m_cImgAnalysis.final_adjust_gain = FALSE;
		m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
		m_cImgAnalysis.gain_step = 0;
		m_cImgAnalysis.changed_gain_step_count = 0;
		m_cImgAnalysis.same_gain_count = 0;
		m_cImgAnalysis.frame_count = 0;
		m_cImgAnalysis.same_gain_time = 0;
		m_cImgAnalysis.detected_frame_count = 0;
		m_cImgAnalysis.JudgeMeanMode = 0;
		m_cImgAnalysis.JudgeMeanModeCount = 0;

		m_cImgAnalysis.sum_mean = 0;
		m_cImgAnalysis.bChangedMIN_BRIGHT = FALSE;
		m_cImgAnalysis.isChangetoSuperDry = 0;

		return FALSE;
	}
/*
	if (m_cImgAnalysis.isChangetoSuperDry < 3 && m_cImgAnalysis.isDetected == 1 && m_cImgAnalysis.foreground_count < 100)
	{
		m_cImgAnalysis.isChangetoSuperDry++;// = TRUE;

		if(m_cImgAnalysis.isChangetoSuperDry >= 3)
		{
//			if (pSetting->forSuperDry_Mode)
//			{
//				pSetting->CurrentDAC = _DAC_FOR_DRY_FOUR_FINGER_ - 0x10;
//			}
//			else
//			{
				pSetting->CurrentDAC = _DAC_FOR_DRY_FOUR_FINGER_;
//			}
		}
		
		return FALSE;
	}
*/
/*	if(m_cImgAnalysis.isChangetoSuperDry >= 3)
	{
		updated_MIN_BRIGHT = _TARGET_MIN_BRIGHTNESS_FOR_DRY_FIVE0;
		updated_MAX_BRIGHT = _TARGET_MAX_BRIGHTNESS_FOR_DRY_FIVE0;
//		forSuperDry_Mode = 1;
	}
*/
	m_cImgAnalysis.detected_frame_count++;

	// finger count 변경시
	if (m_cImgAnalysis.finger_count != m_cImgAnalysis.saved_finger_count)
	{
		m_cImgAnalysis.good_frame_count = 0;
		m_cImgAnalysis.is_final = FALSE;
		m_cImgAnalysis.final_adjust_gain = FALSE;
		m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
		m_cImgAnalysis.gain_step = 0;
		m_cImgAnalysis.changed_gain_step_count = 0;
		m_cImgAnalysis.same_gain_count = 0;
		m_cImgAnalysis.frame_count = 0;
		m_cImgAnalysis.same_gain_time = 0;
		m_cImgAnalysis.JudgeMeanMode = 0;
		m_cImgAnalysis.JudgeMeanModeCount = 0;

		m_cImgAnalysis.sum_mean = 0;
		m_cImgAnalysis.bChangedMIN_BRIGHT = FALSE;
	}

	m_cImgAnalysis.frame_count++;
	m_cImgAnalysis.frame_delay--;
	if (m_cImgAnalysis.frame_delay < 0 &&
		(!m_cImgAnalysis.is_final || m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER) &&
		m_cImgAnalysis.changed_gain_step_count < MAX_STEP_CHANGE &&
		m_cImgAnalysis.detected_frame_count >= REQUIRED_DETECT_FRAME_COUNT &&		//Frame Speed에 따라서 10라는 숫자를 1초에 맞게 SDK를 변경하도록 한다.		D&G 
		m_cImgAnalysis.JudgeMeanMode == 0)
	{
		if (m_cImgAnalysis.mean < updated_MIN_BRIGHT)
		{
            if (m_cImgAnalysis.changed_gain_step_count > 2/* || m_cImgAnalysis.mean > (MIN_BRIGHT-TOLERANCE)*/)
			{
				m_cImgAnalysis.frame_delay = 1;
			}

			// Let's change voltage
			if (m_cImgAnalysis.mean > (updated_MIN_BRIGHT - TOLERANCE))
			{
				if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
					pSetting->CurrentDAC -= (DAC_MINUS_STEP/2);
				else
					pSetting->CurrentDAC -= (DAC_MINUS_STEP);
			}
			else if (m_cImgAnalysis.mean > (updated_MIN_BRIGHT - TOLERANCE*3/2))
			{
				if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
					pSetting->CurrentDAC -= ((DAC_MINUS_STEP+2)/2);
				else
					pSetting->CurrentDAC -= (DAC_MINUS_STEP+2);
			}			
			else
			{
				if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
					pSetting->CurrentDAC -= ((DAC_MINUS_STEP+4)/2);
				else
					pSetting->CurrentDAC -= (DAC_MINUS_STEP+4);
			}			

			if (pSetting->CurrentDAC < pSetting->DefaultMaxDAC)
			{
				pSetting->CurrentDAC = pSetting->DefaultMaxDAC;
			}

			m_cImgAnalysis.good_frame_count = 0;

			if (m_cImgAnalysis.gain_step <= 0)
			{
				m_cImgAnalysis.gain_step = 1;
				m_cImgAnalysis.changed_gain_step_count++;
			}
		}
		else if (m_cImgAnalysis.mean > updated_MAX_BRIGHT)
		{
            // enzyme 2013-01-31 modify to improve capture speed
			if (m_cImgAnalysis.changed_gain_step_count > 2/* || m_cImgAnalysis.mean < (MAX_BRIGHT+TOLERANCE)*/)
			{
				m_cImgAnalysis.frame_delay = 1;
			}

			// Let's change voltage
			if (m_cImgAnalysis.mean < (updated_MAX_BRIGHT + TOLERANCE))
			{
				pSetting->CurrentDAC += 2*2;
			}
			else if (m_cImgAnalysis.mean < (updated_MAX_BRIGHT + TOLERANCE*3/2))
			{
				pSetting->CurrentDAC += 4*2;
			}
			else
			{
				pSetting->CurrentDAC += 6*2;
			}

			if (pSetting->CurrentDAC > pSetting->DefaultMinDAC)
			{
				pSetting->CurrentDAC = pSetting->DefaultMinDAC;
			}

			m_cImgAnalysis.good_frame_count = 0;

			if (m_cImgAnalysis.gain_step >= 0)
			{
				m_cImgAnalysis.gain_step = -1;
				m_cImgAnalysis.changed_gain_step_count++;
			}
		}
	}

	if (m_cImgAnalysis.pre_gain == pSetting->CurrentGain && m_cImgAnalysis.pre_DAC == pSetting->CurrentDAC &&
		abs(m_cImgAnalysis.foreground_count - m_cImgAnalysis.pre_foreground_count) <= (SINGLE_FLAT_DIFF_TRES*m_cImgAnalysis.finger_count))
	{
		m_cImgAnalysis.same_gain_count++;
		m_cImgAnalysis.same_gain_time += frameTime;
	}
	else
	{
		m_cImgAnalysis.same_gain_count = 0;
		m_cImgAnalysis.same_gain_time = 0;
	}

	m_cImgAnalysis.pre_gain = pSetting->CurrentGain;
	m_cImgAnalysis.pre_DAC = pSetting->CurrentDAC;

	if ((m_cImgAnalysis.mean >= updated_MIN_BRIGHT && m_cImgAnalysis.mean <= updated_MAX_BRIGHT) || m_cImgAnalysis.final_adjust_gain)
	{
		if (m_cImgAnalysis.good_frame_count++ > 0)
		{
			if (m_cImgAnalysis.mean >= updated_MIN_BRIGHT && m_cImgAnalysis.mean <= updated_MAX_BRIGHT)
			{
				m_cImgAnalysis.final_adjust_gain = TRUE;
				m_cImgAnalysis.frame_delay = -1;
			}
		}
	}

	if ((m_cImgAnalysis.final_adjust_gain && m_cImgAnalysis.frame_delay < 0 && m_cImgAnalysis.good_frame_count > 1) ||
		m_cImgAnalysis.same_gain_count >= m_cImgAnalysis.max_same_gain_count ||
            m_cImgAnalysis.same_gain_time > (int)(m_cImgAnalysis.max_same_gain_count * frameTime)
		)
	{
		if (m_cImgAnalysis.finger_count == 0)
		{
			AreaTres = SINGLE_FLAT_AREA_TRES;
			FlatDiffThres = SINGLE_FLAT_DIFF_TRES;
		}
		else
		{
			AreaTres = SINGLE_FLAT_AREA_TRES * m_cImgAnalysis.finger_count;
			FlatDiffThres = SINGLE_FLAT_DIFF_TRES * m_cImgAnalysis.finger_count;
		}

		if (m_cImgAnalysis.finger_count == pSetting->TargetFingerCount &&
			m_cImgAnalysis.foreground_count >= AreaTres &&
			abs(m_cImgAnalysis.foreground_count - m_cImgAnalysis.pre_foreground_count) <= FlatDiffThres &&
			m_cImgAnalysis.mean >= updated_MIN_BRIGHT && m_cImgAnalysis.mean <= updated_MAX_BRIGHT  &&
			  abs(m_cImgAnalysis.center_x - m_cImgAnalysis.pre_center_x) <= 2 &&
			  abs(m_cImgAnalysis.center_y - m_cImgAnalysis.pre_center_y) <= 2
			  //			abs(CenterX-Prev_Prev_CenterX) <= 3 &&
			  //			abs(CenterY-Prev_Prev_CenterY) <= 3
																								  )
		{
			if ((m_cImgAnalysis.detected_frame_count * frameTime) > MIN_CAPTURE_TIME &&
				m_cImgAnalysis.good_frame_count > 4)
			{
				if(pSetting->forSuperDry_Mode)
				{
					if(m_cImgAnalysis.detected_frame_count * frameTime > MIN_CAPTURE_TIME_FOR_DRY)
					{
						m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
						m_cImgAnalysis.is_final = TRUE;
					}
				}
				else
				{
					m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
					m_cImgAnalysis.is_final = TRUE;
				}
			}
		}
	}

    m_cImgAnalysis.pre_foreground_count = m_cImgAnalysis.foreground_count;
    m_cImgAnalysis.pre_center_x = m_cImgAnalysis.center_x;
    m_cImgAnalysis.pre_center_y = m_cImgAnalysis.center_y;

	*CaptureGood = m_cImgAnalysis.is_final;

    return m_cImgAnalysis.is_final;
}

BOOL CIBAlgorithm::_Algo_Five0_AutoCapture_TOF(BYTE *InImg, CaptureSetting *pSetting, int frameTime, int ImgSize, int *CaptureGood,
											   int m_nAnalogTouch_Plate)
{
//	int	ForgraoundCNT=0, fingerCount=0, AreaTres, RecomGain;
	int			AreaTres;
	int			FlatDiffThres;
	const int	MAX_BRIGHT = pSetting->TargetMaxBrightness, MIN_BRIGHT = pSetting->TargetMinBrightness;
	const int	MIN_CAPTURE_TIME = 600;
	int			MIN_CAPTURE_TIME_FOR_DRY = 600;
	const int	TOLERANCE = 30;
	const int	MAX_STEP_CHANGE = 6;	// default 3
//	int			forSuperDry_Mode=pSetting->forSuperDry_Mode;
	int			REQUIRED_DETECT_FRAME_COUNT = pSetting->DetectionFrameCount;
	int			updated_MIN_BRIGHT = MIN_BRIGHT, updated_MAX_BRIGHT = MAX_BRIGHT;
//    int         brightPixelRate;
//    int         DAC_MINUS_STEP = 4;

    if(m_pPropertyInfo->nWaitTimeForDry > 0)
		MIN_CAPTURE_TIME_FOR_DRY = m_pPropertyInfo->nWaitTimeForDry;

	*CaptureGood = FALSE;

	m_cImgAnalysis.max_same_gain_count = 3;
	
	if (!m_cImgAnalysis.is_final && m_nAnalogTouch_Plate < 300 + 80 * (m_pPropertyInfo->nNumberOfObjects-1))
	{
		pSetting->DACPowerOffThreshold++;
		if(pSetting->DACPowerOffThreshold > 1)
		{
			pSetting->DACPowerOff = TRUE;
			pSetting->DACPowerOffThreshold = 1;
			pSetting->DefaultDAC = _DAC_FOR_TOF_FIVE0_;
			memset(InImg, 0, m_pUsbDevInfo->CisImgSize);

			pSetting->CurrentGain = pSetting->DefaultGain;
			pSetting->CurrentDAC = pSetting->DefaultDAC;
			m_cImgAnalysis.frame_delay = 2;
			m_cImgAnalysis.good_frame_count = 0;
			m_cImgAnalysis.is_final = FALSE;
			m_cImgAnalysis.final_adjust_gain = FALSE;
			m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
			m_cImgAnalysis.gain_step = 0;
			m_cImgAnalysis.changed_gain_step_count = 0;
			m_cImgAnalysis.same_gain_count = 0;
			m_cImgAnalysis.frame_count = 0;
			m_cImgAnalysis.same_gain_time = 0;
			m_cImgAnalysis.detected_frame_count = 0;
			m_cImgAnalysis.JudgeMeanMode = 0;
			m_cImgAnalysis.JudgeMeanModeCount = 0;

			m_cImgAnalysis.sum_mean = 0;
			m_cImgAnalysis.bChangedMIN_BRIGHT = FALSE;
			m_cImgAnalysis.isChangetoSuperDry = 0;

			return FALSE;
		}
		else
			return FALSE;
	}

	pSetting->DACPowerOffThreshold = 0;

	if (!m_cImgAnalysis.is_final && pSetting->DACPowerOff == TRUE && 
		m_nAnalogTouch_Plate >= 300 + 80 * (m_pPropertyInfo->nNumberOfObjects-1))
	{
		//TRACE("Jump to Low Limit Voltage (finger_count : %d, m_nAnalogTouch_Plate : %d)\n", m_cImgAnalysis.finger_count, m_nAnalogTouch_Plate);
		pSetting->DACPowerOff = FALSE;

		if(m_nAnalogTouch_Plate < 500)
			pSetting->CurrentDAC = 0xF0;
		else if(m_nAnalogTouch_Plate < 700)
			pSetting->CurrentDAC = 0xE0;
		else
			pSetting->CurrentDAC = 0xD0;

		return FALSE;
	}

	m_cImgAnalysis.detected_frame_count++;

	// finger count 변경시
	if (m_cImgAnalysis.finger_count != m_cImgAnalysis.saved_finger_count)
	{
		m_cImgAnalysis.good_frame_count = 0;
		m_cImgAnalysis.is_final = FALSE;
		m_cImgAnalysis.final_adjust_gain = FALSE;
		m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
		m_cImgAnalysis.gain_step = 0;
		m_cImgAnalysis.changed_gain_step_count = 0;
		m_cImgAnalysis.same_gain_count = 0;
		m_cImgAnalysis.frame_count = 0;
		m_cImgAnalysis.same_gain_time = 0;
		m_cImgAnalysis.JudgeMeanMode = 0;
		m_cImgAnalysis.JudgeMeanModeCount = 0;

		m_cImgAnalysis.sum_mean = 0;
		m_cImgAnalysis.bChangedMIN_BRIGHT = FALSE;
	}

	m_cImgAnalysis.frame_count++;
	m_cImgAnalysis.frame_delay--;
	if (m_cImgAnalysis.frame_delay < 0 &&
		(!m_cImgAnalysis.is_final || m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER) &&
		m_cImgAnalysis.changed_gain_step_count < MAX_STEP_CHANGE &&
		m_cImgAnalysis.detected_frame_count >= REQUIRED_DETECT_FRAME_COUNT &&		//Frame Speed에 따라서 10라는 숫자를 1초에 맞게 SDK를 변경하도록 한다.		D&G 
		m_cImgAnalysis.JudgeMeanMode == 0)
	{
		if (m_cImgAnalysis.mean < updated_MIN_BRIGHT)
		{
            if (m_cImgAnalysis.changed_gain_step_count > 2/* || m_cImgAnalysis.mean > (MIN_BRIGHT-TOLERANCE)*/)
			{
				m_cImgAnalysis.frame_delay = 1;
			}

			// Let's change voltage
			if (m_cImgAnalysis.mean > (updated_MIN_BRIGHT - TOLERANCE))
			{
				if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
					pSetting->CurrentDAC -= 2;
				else
					pSetting->CurrentDAC -= 2*2;
			}
			else if (m_cImgAnalysis.mean > (updated_MIN_BRIGHT - TOLERANCE*3/2))
			{
				if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
					pSetting->CurrentDAC -= 4;
				else
					pSetting->CurrentDAC -= 4*2;
			}
			else
			{
				if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
					pSetting->CurrentDAC -= 6;
				else
					pSetting->CurrentDAC -= 6*2;
			}			

			if (pSetting->CurrentDAC < pSetting->DefaultMaxDAC)
			{
				pSetting->CurrentDAC = pSetting->DefaultMaxDAC;
			}

			m_cImgAnalysis.good_frame_count = 0;

			if (m_cImgAnalysis.gain_step <= 0)
			{
				m_cImgAnalysis.gain_step = 1;
				m_cImgAnalysis.changed_gain_step_count++;
			}
		}
		else if (m_cImgAnalysis.mean > updated_MAX_BRIGHT)
		{
            // enzyme 2013-01-31 modify to improve capture speed
			if (m_cImgAnalysis.changed_gain_step_count > 2/* || m_cImgAnalysis.mean < (MAX_BRIGHT+TOLERANCE)*/)
			{
				m_cImgAnalysis.frame_delay = 1;
			}

			// Let's change voltage
			if (m_cImgAnalysis.mean < (updated_MAX_BRIGHT + TOLERANCE))
			{
				pSetting->CurrentDAC += 2*2;
			}
			else if (m_cImgAnalysis.mean < (updated_MAX_BRIGHT + TOLERANCE*3/2))
			{
				pSetting->CurrentDAC += 4*2;
			}
			else
			{
				pSetting->CurrentDAC += 6*2;
			}

			if (pSetting->CurrentDAC > pSetting->DefaultMinDAC)
			{
				pSetting->CurrentDAC = pSetting->DefaultMinDAC;
			}

			m_cImgAnalysis.good_frame_count = 0;

			if (m_cImgAnalysis.gain_step >= 0)
			{
				m_cImgAnalysis.gain_step = -1;
				m_cImgAnalysis.changed_gain_step_count++;
			}
		}
	}
	
	if (!m_cImgAnalysis.is_final &&
		m_nAnalogTouch_Plate >= (220 + 50*m_pPropertyInfo->nNumberOfObjects))
	{
		if (m_cImgAnalysis.finger_count == 0 && pSetting->CurrentDAC <= 0x90)
		{
			pSetting->CurrentDAC = 0x90;
		}
	}

	if (m_cImgAnalysis.pre_gain == pSetting->CurrentGain && m_cImgAnalysis.pre_DAC == pSetting->CurrentDAC &&
		abs(m_cImgAnalysis.foreground_count - m_cImgAnalysis.pre_foreground_count) <= (SINGLE_FLAT_DIFF_TRES*m_cImgAnalysis.finger_count))
	{
		m_cImgAnalysis.same_gain_count++;
		m_cImgAnalysis.same_gain_time += frameTime;
	}
	else
	{
		m_cImgAnalysis.same_gain_count = 0;
		m_cImgAnalysis.same_gain_time = 0;
	}

	m_cImgAnalysis.pre_gain = pSetting->CurrentGain;
	m_cImgAnalysis.pre_DAC = pSetting->CurrentDAC;

	if ((m_cImgAnalysis.mean >= updated_MIN_BRIGHT && m_cImgAnalysis.mean <= updated_MAX_BRIGHT) || m_cImgAnalysis.final_adjust_gain)
	{
		if (m_cImgAnalysis.good_frame_count++ > 0)
		{
			if (m_cImgAnalysis.mean >= updated_MIN_BRIGHT && m_cImgAnalysis.mean <= updated_MAX_BRIGHT)
			{
				m_cImgAnalysis.final_adjust_gain = TRUE;
				m_cImgAnalysis.frame_delay = -1;
			}
		}
	}

	if ((m_cImgAnalysis.final_adjust_gain && m_cImgAnalysis.frame_delay < 0 && m_cImgAnalysis.good_frame_count > 1) ||
		m_cImgAnalysis.same_gain_count >= m_cImgAnalysis.max_same_gain_count ||
            m_cImgAnalysis.same_gain_time > (int)(m_cImgAnalysis.max_same_gain_count * frameTime)
		)
	{
		if (m_cImgAnalysis.finger_count == 0)
		{
			AreaTres = SINGLE_FLAT_AREA_TRES;
			FlatDiffThres = SINGLE_FLAT_DIFF_TRES;
		}
		else
		{
			AreaTres = SINGLE_FLAT_AREA_TRES * m_cImgAnalysis.finger_count;
			FlatDiffThres = SINGLE_FLAT_DIFF_TRES * m_cImgAnalysis.finger_count;
		}

		if (m_cImgAnalysis.finger_count == pSetting->TargetFingerCount &&
			m_cImgAnalysis.foreground_count >= AreaTres &&
			abs(m_cImgAnalysis.foreground_count - m_cImgAnalysis.pre_foreground_count) <= FlatDiffThres &&
			m_cImgAnalysis.mean >= updated_MIN_BRIGHT && m_cImgAnalysis.mean <= updated_MAX_BRIGHT  &&
			  abs(m_cImgAnalysis.center_x - m_cImgAnalysis.pre_center_x) <= 2 &&
			  abs(m_cImgAnalysis.center_y - m_cImgAnalysis.pre_center_y) <= 2
			  //			abs(CenterX-Prev_Prev_CenterX) <= 3 &&
			  //			abs(CenterY-Prev_Prev_CenterY) <= 3
																								  )
		{
			if ((m_cImgAnalysis.detected_frame_count * frameTime) > MIN_CAPTURE_TIME/* &&
				m_cImgAnalysis.good_frame_count > 4*/)
			{
				if(pSetting->forSuperDry_Mode)
				{
					if(m_cImgAnalysis.detected_frame_count * frameTime > MIN_CAPTURE_TIME_FOR_DRY)
					{
						m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
						m_cImgAnalysis.is_final = TRUE;
					}
				}
				else
				{
					m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
					m_cImgAnalysis.is_final = TRUE;
				}
			}
		}
	}

    m_cImgAnalysis.pre_foreground_count = m_cImgAnalysis.foreground_count;
    m_cImgAnalysis.pre_center_x = m_cImgAnalysis.center_x;
    m_cImgAnalysis.pre_center_y = m_cImgAnalysis.center_y;

	*CaptureGood = m_cImgAnalysis.is_final;

    return m_cImgAnalysis.is_final;
}

int CIBAlgorithm::_Algo_Five0_GetBrightWithRawImage_forDetectOnly(BYTE *InRawImg, int ForgraoundCNT)
{
	int y, x, i, j;
	int ii, tempsum;
	int xx, yy, forgroundCNT = 0;
	int value;
    int NOISE_H = _TFT_LE_NOISE_HEIGHT_FOR_FIVE0_;

	if(m_pPropertyInfo->bEnableTOF)
		NOISE_H = 0;

	memset(m_segment_enlarge_buffer, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
	memset(m_segment_enlarge_buffer_for_fingercnt, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
	memset(EnlargeBuf, 0, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);
	m_SBDAlg->m_SegmentBright = 0;
	m_SBDAlg->m_SegmentBrightCnt = 0;

	m_cImgAnalysis.LEFT = 1;
	m_cImgAnalysis.RIGHT = 1;
	m_cImgAnalysis.TOP = 1;
	m_cImgAnalysis.BOTTOM = 1;

	int m_Minus_Value;

	m_Minus_Value = _TFT_NOISE_FOR_FIVE0_;

	for (y = ENLARGESIZE_ZOOM_H+NOISE_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
	{
		yy = (y - ENLARGESIZE_ZOOM_H) * (CIS_IMG_H) / ZOOM_H;

		for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
		{
			xx = (x - ENLARGESIZE_ZOOM_W) * (CIS_IMG_W) / ZOOM_W;
			value = InRawImg[yy * (CIS_IMG_W) + xx];
			if(value < m_Minus_Value)
			{
				value = 0;
			}
			else
			{
				value = (int)(value * m_SBDAlg->diff_image_withMinusMean_preview[(yy*CIS_IMG_W/2+xx)/2] / 1024) + value - m_Minus_Value;
				if (value > 255)
				{
					value = 255;
				}
				else if (value < 0)
				{
					value = 0;
				}
			}
			
			EnlargeBuf[(y)*ZOOM_ENLAGE_W + (x)] = value;
		}
	}

	memcpy(EnlargeBuf_Org, EnlargeBuf, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);

    _Algo_HistogramStretchForZoomEnlarge(EnlargeBuf);

	// for finger count
	for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
	{
		for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
		{
			tempsum  = EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W + (j - 1)];
			tempsum += EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W + (j)];
			tempsum += EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W + (j + 1)];
			tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j - 1)];
			tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j)];
			tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j + 1)];
			tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W + (j - 1)];
			tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W + (j)];
			tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W + (j + 1)];

			tempsum /= 9;

			// fixed bug.
			if (tempsum >= 1)//Threshold) //mean/20)
			{
				m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W + j] = 255;
			}
		}
	}

	for (ii = 0; ii < 1; ii++)
	{
		memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
		for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
		{
			for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
			{
				if (m_segment_enlarge_buffer[i * ZOOM_ENLAGE_W + j] == 255)
				{
					continue;
				}

				tempsum  = m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W + j - 1];
				tempsum += m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W + j  ];
				tempsum += m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W + j + 1];
				tempsum += m_segment_enlarge_buffer[(i) * ZOOM_ENLAGE_W + j - 1];
				tempsum += m_segment_enlarge_buffer[(i) * ZOOM_ENLAGE_W + j + 1];
				tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W + j - 1];
				tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W + j  ];
				tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W + j + 1];

				if (tempsum >= 4 * 255)
				{
					m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W + j] = 255;
				}
			}
		}
	}

//    _Algo_Dilation(m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W, ZOOM_ENLAGE_H);
	memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);

	for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
	{
		for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
		{
			if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W + x - 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W + x] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W + x + 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x - 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x + 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W + x - 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W + x] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W + x + 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 255;
			}

			if (m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] == 255)
			{
				forgroundCNT++;
			}
		}
	}

    m_cImgAnalysis.foreground_count2 = 0;
	if(forgroundCNT >= ForgraoundCNT)
	{
//		TRACE("forgroundCNT : %d, ForgraoundCNT : %d\n", forgroundCNT, ForgraoundCNT);
        m_cImgAnalysis.foreground_count2 = forgroundCNT;
		return 1;
	}

	return 0;
}

int CIBAlgorithm::_Algo_Five0_GetBrightWithRawImage_forDetectOnly_Roll(BYTE *InRawImg, int ForgraoundCNT)
{
	int y, x, i, j;
	int ii, tempsum;
	int xx, yy, forgroundCNT = 0;
	int value;
	int offset_x = (CIS_IMG_W - CIS_IMG_W_ROLL)/2;
    int NOISE_H = _TFT_LE_NOISE_HEIGHT_FOR_FIVE0_;

	if(m_pPropertyInfo->bEnableTOF)
		NOISE_H = 0;

	memset(m_segment_enlarge_buffer, 0, ZOOM_ENLAGE_W_ROLL * ZOOM_ENLAGE_H_ROLL);
	memset(m_segment_enlarge_buffer_for_fingercnt, 0, ZOOM_ENLAGE_W_ROLL * ZOOM_ENLAGE_H_ROLL);
	memset(EnlargeBuf, 0, ZOOM_ENLAGE_H_ROLL * ZOOM_ENLAGE_W_ROLL);
	m_SBDAlg->m_SegmentBright = 0;
	m_SBDAlg->m_SegmentBrightCnt = 0;

	m_cImgAnalysis.LEFT = 1;
	m_cImgAnalysis.RIGHT = 1;
	m_cImgAnalysis.TOP = 1;
	m_cImgAnalysis.BOTTOM = 1;

	int m_Minus_Value;

	m_Minus_Value = _TFT_NOISE_FOR_FIVE0_;

	for (y = ENLARGESIZE_ZOOM_H_ROLL+NOISE_H; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
	{
		yy = (y - ENLARGESIZE_ZOOM_H_ROLL) * (CIS_IMG_H_ROLL) / ZOOM_H_ROLL;

		for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
		{
			xx = (x - ENLARGESIZE_ZOOM_W_ROLL) * (CIS_IMG_W_ROLL) / ZOOM_W_ROLL;
			value = InRawImg[yy * (CIS_IMG_W_ROLL) + xx];
			if(value < m_Minus_Value)
			{
				value = 0;
			}
			else
			{
				value = (int)(value * m_SBDAlg->diff_image_withMinusMean_preview[yy/2*CIS_IMG_W/2+xx/2+offset_x/2] / 1024) + value - m_Minus_Value;
				if (value > 255)
				{
					value = 255;
				}
				else if (value < 0)
				{
					value = 0;
				}
			}
			
			EnlargeBuf[(y)*ZOOM_ENLAGE_W_ROLL + (x)] = value;
		}
	}

	memcpy(EnlargeBuf_Org, EnlargeBuf, ZOOM_ENLAGE_H_ROLL * ZOOM_ENLAGE_W_ROLL);

	_Algo_HistogramStretchForZoomEnlarge_Kojak_Roll(EnlargeBuf);

	// for finger count
	for (i = ENLARGESIZE_ZOOM_H_ROLL; i < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; i++)
	{
		for (j = ENLARGESIZE_ZOOM_W_ROLL; j < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; j++)
		{
			tempsum  = EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W_ROLL + (j - 1)];
			tempsum += EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W_ROLL + (j)];
			tempsum += EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W_ROLL + (j + 1)];
			tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W_ROLL + (j - 1)];
			tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W_ROLL + (j)];
			tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W_ROLL + (j + 1)];
			tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W_ROLL + (j - 1)];
			tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W_ROLL + (j)];
			tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W_ROLL + (j + 1)];

			tempsum /= 9;

			// fixed bug.
			if (tempsum >= 1)//Threshold) //mean/20)
			{
				m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W_ROLL + j] = 255;
			}
		}
	}

	for (ii = 0; ii < 1; ii++)
	{
		memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W_ROLL * ZOOM_ENLAGE_H_ROLL);
		for (i = ENLARGESIZE_ZOOM_H_ROLL; i < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; i++)
		{
			for (j = ENLARGESIZE_ZOOM_W_ROLL; j < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; j++)
			{
				if (m_segment_enlarge_buffer[i * ZOOM_ENLAGE_W_ROLL + j] == 255)
				{
					continue;
				}

				tempsum  = m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W_ROLL + j - 1];
				tempsum += m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W_ROLL + j  ];
				tempsum += m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W_ROLL + j + 1];
				tempsum += m_segment_enlarge_buffer[(i) * ZOOM_ENLAGE_W_ROLL + j - 1];
				tempsum += m_segment_enlarge_buffer[(i) * ZOOM_ENLAGE_W_ROLL + j + 1];
				tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W_ROLL + j - 1];
				tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W_ROLL + j  ];
				tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W_ROLL + j + 1];

				if (tempsum >= 4 * 255)
				{
					m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W_ROLL + j] = 255;
				}
			}
		}
	}

	memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W_ROLL * ZOOM_ENLAGE_H_ROLL);

	for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
	{
		for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
		{
			if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W_ROLL + x - 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W_ROLL + x] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W_ROLL + x + 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W_ROLL + x - 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W_ROLL + x] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W_ROLL + x + 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W_ROLL + x - 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W_ROLL + x] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W_ROLL + x + 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 255;
			}

			if (m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] == 255)
			{
				forgroundCNT++;
			}
		}
	}

    m_cImgAnalysis.foreground_count2 = 0;
	if(forgroundCNT >= ForgraoundCNT)
	{
//		TRACE("forgroundCNT : %d, ForgraoundCNT : %d\n", forgroundCNT, ForgraoundCNT);
        m_cImgAnalysis.foreground_count2 = forgroundCNT;
		return 1;
	}

	return 0;
}

int CIBAlgorithm::_Algo_Five0_GetBrightWithRawImage(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY, int *Bright, int *BrightPixelCount)
{
    int y, x, i, j;
	int mean = 0, count = 0;
	int ii, tempsum;
	int xx, yy, forgroundCNT = 0;
	int value;
	int sum_x = 0, sum_y = 0;
	int BrightValue = 0;
    int NOISE_H = _TFT_LE_NOISE_HEIGHT_FOR_FIVE0_;

	if(m_pPropertyInfo->bEnableTOF)
		NOISE_H = 0;

	memset(m_segment_enlarge_buffer, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
	memset(m_segment_enlarge_buffer_for_fingercnt, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
	memset(EnlargeBuf, 0, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);
	*CenterX = -1;
	*CenterY = -1;
	*ForgraoundCNT = 0;
    *BrightPixelCount = 0;
	m_SBDAlg->m_SegmentBright = 0;
	m_SBDAlg->m_SegmentBrightCnt = 0;

	m_cImgAnalysis.LEFT = 1;
	m_cImgAnalysis.RIGHT = 1;
	m_cImgAnalysis.TOP = 1;
	m_cImgAnalysis.BOTTOM = 1;
	int m_Minus_Value;

	m_Minus_Value = _TFT_NOISE_FOR_FIVE0_;

	for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
	{
		yy = (y - ENLARGESIZE_ZOOM_H) * (CIS_IMG_H) / ZOOM_H;

		for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
		{
			xx = (x - ENLARGESIZE_ZOOM_W) * (CIS_IMG_W) / ZOOM_W;
			value = InRawImg[yy * (CIS_IMG_W) + xx];
			if(value < m_Minus_Value)
			{
				value = 0;
			}
			else
			{
				value = (int)(value * m_SBDAlg->diff_image_withMinusMean_preview[(yy*CIS_IMG_W/2+xx)/2] / 1024) + value - m_Minus_Value;
			if (value > 255)
			{
				value = 255;
			}
			else if (value < 0)
			{
				value = 0;
			}
			}
			
			EnlargeBuf[(y)*ZOOM_ENLAGE_W + (x)] = value;
		}
	}

	memcpy(EnlargeBuf_Org, EnlargeBuf, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);

	_Algo_HistogramStretchForZoomEnlarge(EnlargeBuf);

	mean = 0;
	count = 0;
	for (i = ENLARGESIZE_ZOOM_H+NOISE_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
	{
		for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
		{
			if (EnlargeBuf[(i)*ZOOM_ENLAGE_W + (j)] > 10)
			{
				mean += EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j)];
				count++;
			}
		}
	}

	if (count == 0)
	{
		//gettimeofday(&tv, NULL);
		//elapsed = (tv.tv_sec - start_tv.tv_sec) + (tv.tv_usec - start_tv.tv_usec) / 1000.0;
		//printf("_Algo_GetBrightWithRawImage(): %1.4f milliseconds\n", elapsed);
		return 0;
	}

	mean /= count;
	*Bright = mean;
    *BrightPixelCount = count;

	/*	int Threshold = mean/20;
	if(Threshold < 5)
	Threshold = 5;
	*/
	int Threshold = (int)(pow((double)mean / 255.0, 3.0) * 255);
	if (Threshold < 5)
	{
		Threshold = 5;
	}
	else if (Threshold > 250)
	{
		Threshold = 250;
	}

	// for finger count
	for (i = ENLARGESIZE_ZOOM_H+NOISE_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
	{
		for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
		{
			tempsum  = EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W + (j - 1)];
			tempsum += EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W + (j)];
			tempsum += EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W + (j + 1)];
			tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j - 1)];
			tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j)];
			tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j + 1)];
			tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W + (j - 1)];
			tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W + (j)];
			tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W + (j + 1)];

			tempsum /= 9;

			// fixed bug.
			if (tempsum >= Threshold) //mean/20)
			{
				m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W + j] = 255;
			}
		}
	}

	for (ii = 0; ii < 1; ii++)
	{
		memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
		for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
		{
			for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
			{
				if (m_segment_enlarge_buffer[i * ZOOM_ENLAGE_W + j] == 255)
				{
					continue;
				}

				tempsum  = m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W + j - 1];
				tempsum += m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W + j  ];
				tempsum += m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W + j + 1];
				tempsum += m_segment_enlarge_buffer[(i) * ZOOM_ENLAGE_W + j - 1];
				tempsum += m_segment_enlarge_buffer[(i) * ZOOM_ENLAGE_W + j + 1];
				tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W + j - 1];
				tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W + j  ];
				tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W + j + 1];

				if (tempsum >= 4 * 255)
				{
					m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W + j] = 255;
				}
			}
		}
	}

	memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);

	for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
	{
		for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
		{
			if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W + x - 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W + x] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W + x + 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x - 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x + 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W + x - 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W + x] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W + x + 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
			}
			else
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 255;
			}

			if (m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] == 255)
			{
				forgroundCNT++;
			}
		}
	}

	*ForgraoundCNT = forgroundCNT;

	if (forgroundCNT < 100)
	{
		return 0;
	}

	memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);

	mean = 0;
	count = 0;
	for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
	{
		for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
		{
			if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x] == 0)
			{
				continue;
			}

			if (EnlargeBuf_Org[(y)*ZOOM_ENLAGE_W + (x)] >= 10)//Threshold)
			{
				mean += EnlargeBuf_Org[(y) * ZOOM_ENLAGE_W + (x)];
				count++;
				sum_x += x * ZOOM_OUT;
				sum_y += y * ZOOM_OUT;
			} 

			if(EnlargeBuf_Org[(y) * ZOOM_ENLAGE_W + (x)] < 256)
			{ 
				m_SBDAlg->m_SegmentBright += EnlargeBuf_Org[(y) * ZOOM_ENLAGE_W + (x)];
				m_SBDAlg->m_SegmentBrightCnt++;
			}
		}
	}
	if (count == 0)
	{
		return 0;
	}

	if(m_SBDAlg->m_SegmentBrightCnt > 0)
		m_SBDAlg->m_SegmentBright = m_SBDAlg->m_SegmentBright / m_SBDAlg->m_SegmentBrightCnt;
	else
	{
		for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
		{
			for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
			{
				if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x] == 0)
				{
					continue;
				}

				if(EnlargeBuf_Org[(y) * ZOOM_ENLAGE_W + (x)] < 256)
				{
					m_SBDAlg->m_SegmentBright += EnlargeBuf_Org[(y) * ZOOM_ENLAGE_W + (x)];
					m_SBDAlg->m_SegmentBrightCnt++;
				}
			}
		}
		if(m_SBDAlg->m_SegmentBrightCnt > 0)
			m_SBDAlg->m_SegmentBright = m_SBDAlg->m_SegmentBright / m_SBDAlg->m_SegmentBrightCnt;
		else
			m_SBDAlg->m_SegmentBright =0;
	}

	mean /= count;
	*CenterX = sum_x / count;
	*CenterY = sum_y / count;

	BrightValue = mean;

	/////////////////////////////////////////////////////////////////////////////////////
	// find calc rect
	int COUNT;
	for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
	{
		COUNT = 0;
		for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
		{
			if (EnlargeBuf[y * ZOOM_ENLAGE_W + x] >= Threshold)
			{
				COUNT++;
			}
		}

		if (COUNT > 5)
		{
			m_cImgAnalysis.LEFT = (x - ENLARGESIZE_ZOOM_W) * (CIS_IMG_W/2) / ZOOM_W - 30;
			if (m_cImgAnalysis.LEFT < 1)
			{
				m_cImgAnalysis.LEFT = 1;
			}
			break;
		}
	}

	for (x = ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x >= ENLARGESIZE_ZOOM_W; x--)
	{
		COUNT = 0;
		for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
		{
			if (EnlargeBuf[y * ZOOM_ENLAGE_W + x] >= Threshold)
			{
				COUNT++;
			}
		}

		if (COUNT > 5)
		{
			m_cImgAnalysis.RIGHT = (x - ENLARGESIZE_ZOOM_W) * (CIS_IMG_W/2) / ZOOM_W + 30 + 6;
			if (m_cImgAnalysis.RIGHT > (CIS_IMG_W/2) - 1)
			{
				m_cImgAnalysis.RIGHT = (CIS_IMG_W/2) - 1;
			}
			break;
		}
	}

	for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
	{
		COUNT = 0;
		for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
		{
			if (EnlargeBuf[y * ZOOM_ENLAGE_W + x] >= Threshold)
			{
				COUNT++;
			}
		}

		if (COUNT > 5)
		{
			m_cImgAnalysis.TOP = (y - ENLARGESIZE_ZOOM_H) * (CIS_IMG_H/2) / ZOOM_H - 30 - 16;
			if (m_cImgAnalysis.TOP < 1)
			{
				m_cImgAnalysis.TOP = 1;
			}
			break;
		}
	}

	for (y = ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y > ENLARGESIZE_ZOOM_H; y--)
	{
		COUNT = 0;
		for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
		{
			if (EnlargeBuf[y * ZOOM_ENLAGE_W + x] >= Threshold)
			{
				COUNT++;
			}
		}

		if (COUNT > 5)
		{
			m_cImgAnalysis.BOTTOM = (y - ENLARGESIZE_ZOOM_H) * (CIS_IMG_H/2) / ZOOM_H + 30 + 16;
			if (m_cImgAnalysis.BOTTOM > (CIS_IMG_H/2) - 1)
			{
				m_cImgAnalysis.BOTTOM = (CIS_IMG_H/2) - 1;
			}
			break;
		}
	}

	return BrightValue;
}

int CIBAlgorithm::_Algo_Five0_GetBrightWithRawImage_Roll(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY, int *Bright)
{
    int y, x, i, j;
	int mean = 0, count = 0;
	int ii, tempsum;
	int xx, yy, forgroundCNT = 0;
	int value;
	int sum_x = 0, sum_y = 0;
	int BrightValue = 0;
	int offset = (CIS_IMG_W-CIS_IMG_W_ROLL)/2;
    int NOISE_H = _TFT_LE_NOISE_HEIGHT_FOR_FIVE0_;

	if(m_pPropertyInfo->bEnableTOF)
		NOISE_H = 0;

	memset(m_segment_enlarge_buffer, 0, ZOOM_ENLAGE_W_ROLL * ZOOM_ENLAGE_H_ROLL);
	memset(m_segment_enlarge_buffer_for_fingercnt, 0, ZOOM_ENLAGE_W_ROLL * ZOOM_ENLAGE_H_ROLL);
	memset(EnlargeBuf, 0, ZOOM_ENLAGE_H_ROLL * ZOOM_ENLAGE_W_ROLL);
	*CenterX = -1;
	*CenterY = -1;
	*ForgraoundCNT = 0;
	m_SBDAlg->m_SegmentBright = 0;
	m_SBDAlg->m_SegmentBrightCnt = 0;

	m_cImgAnalysis.LEFT = 1;
	m_cImgAnalysis.RIGHT = 1;
	m_cImgAnalysis.TOP = 1;
	m_cImgAnalysis.BOTTOM = 1;

	int m_Minus_Value;

	m_Minus_Value = _TFT_NOISE_FOR_FIVE0_;

	for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
	{
		yy = (y - ENLARGESIZE_ZOOM_H_ROLL) * (CIS_IMG_H_ROLL) / ZOOM_H_ROLL;

		for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
		{
			xx = (x - ENLARGESIZE_ZOOM_W_ROLL) * (CIS_IMG_W_ROLL) / ZOOM_W_ROLL;
			value = InRawImg[yy * (CIS_IMG_W_ROLL) + xx];
			if(value < m_Minus_Value)
			{
				value = 0;
			}
			else
			{
				value = (int)(value * m_SBDAlg->diff_image_withMinusMean_preview[yy/2*CIS_IMG_W/2+xx/2+offset/2] / 1024) + value - m_Minus_Value;
			if (value > 255)
			{
				value = 255;
			}
			else if (value < 0)
			{
				value = 0;
			}
			}
			
			EnlargeBuf[(y)*ZOOM_ENLAGE_W_ROLL + (x)] = value;
		}
	}

	memcpy(EnlargeBuf_Org, EnlargeBuf, ZOOM_ENLAGE_H_ROLL * ZOOM_ENLAGE_W_ROLL);

    _Algo_HistogramStretchForZoomEnlarge_Kojak_Roll(EnlargeBuf);

	mean = 0;
	count = 0;
	for (i = ENLARGESIZE_ZOOM_H_ROLL+NOISE_H; i < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; i++)
	{
		for (j = ENLARGESIZE_ZOOM_W_ROLL; j < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; j++)
		{
			if (EnlargeBuf[(i)*ZOOM_ENLAGE_W_ROLL + (j)] > 10)
			{
				mean += EnlargeBuf[(i) * ZOOM_ENLAGE_W_ROLL + (j)];
				count++;
			}
		}
	}

	if (count == 0)
	{
		//gettimeofday(&tv, NULL);
		//elapsed = (tv.tv_sec - start_tv.tv_sec) + (tv.tv_usec - start_tv.tv_usec) / 1000.0;
		//printf("_Algo_GetBrightWithRawImage(): %1.4f milliseconds\n", elapsed);
		return 0;
	}

	mean /= count;

	*Bright = mean;

	/*	int Threshold = mean/20;
	if(Threshold < 5)
	Threshold = 5;
	*/
	int Threshold = (int)(pow((double)mean / 255.0, 3.0) * 255);
	if (Threshold < 5)
	{
		Threshold = 5;
	}
	else if (Threshold > 250)
	{
		Threshold = 250;
	}

	// for finger count
	for (i = ENLARGESIZE_ZOOM_H_ROLL+NOISE_H; i < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; i++)
	{
		for (j = ENLARGESIZE_ZOOM_W_ROLL; j < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; j++)
		{
			tempsum  = EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W_ROLL + (j - 1)];
			tempsum += EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W_ROLL + (j)];
			tempsum += EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W_ROLL + (j + 1)];
			tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W_ROLL + (j - 1)];
			tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W_ROLL + (j)];
			tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W_ROLL + (j + 1)];
			tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W_ROLL + (j - 1)];
			tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W_ROLL + (j)];
			tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W_ROLL + (j + 1)];

			tempsum /= 9;

			// fixed bug.
			if (tempsum >= Threshold) //mean/20)
			{
				m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W_ROLL + j] = 255;
			}
		}
	}

	for (ii = 0; ii < 1; ii++)
	{
		memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W_ROLL * ZOOM_ENLAGE_H_ROLL);
		for (i = ENLARGESIZE_ZOOM_H_ROLL; i < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; i++)
		{
			for (j = ENLARGESIZE_ZOOM_W_ROLL; j < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; j++)
			{
				if (m_segment_enlarge_buffer[i * ZOOM_ENLAGE_W_ROLL + j] == 255)
				{
					continue;
				}

				tempsum  = m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W_ROLL + j - 1];
				tempsum += m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W_ROLL + j  ];
				tempsum += m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W_ROLL + j + 1];
				tempsum += m_segment_enlarge_buffer[(i) * ZOOM_ENLAGE_W_ROLL + j - 1];
				tempsum += m_segment_enlarge_buffer[(i) * ZOOM_ENLAGE_W_ROLL + j + 1];
				tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W_ROLL + j - 1];
				tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W_ROLL + j  ];
				tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W_ROLL + j + 1];

				if (tempsum >= 4 * 255)
				{
					m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W_ROLL + j] = 255;
				}
			}
		}
	}

	memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W_ROLL * ZOOM_ENLAGE_H_ROLL);

	for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
	{
		for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
		{
			if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W_ROLL + x - 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W_ROLL + x] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W_ROLL + x + 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W_ROLL + x - 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W_ROLL + x] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W_ROLL + x + 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W_ROLL + x - 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W_ROLL + x] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W_ROLL + x + 1] == 0)
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 0;
			}
			else
			{
				m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] = 255;
			}

			if (m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] == 255)
			{
				forgroundCNT++;
			}
		}
	}

	*ForgraoundCNT = forgroundCNT;

	if (forgroundCNT < 100)
	{
		return 0;
	}

	memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W_ROLL * ZOOM_ENLAGE_H_ROLL);

	mean = 0;
	count = 0;
	for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
	{
		for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
		{
			if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W_ROLL + x] == 0)
			{
				continue;
			}

			if (EnlargeBuf_Org[(y)*ZOOM_ENLAGE_W_ROLL + (x)] >= 10)//Threshold)
			{
				mean += EnlargeBuf_Org[(y) * ZOOM_ENLAGE_W_ROLL + (x)];
				count++;
				sum_x += x * ZOOM_OUT;
				sum_y += y * ZOOM_OUT;
			}

			if(EnlargeBuf_Org[(y) * ZOOM_ENLAGE_W_ROLL + (x)] < 256)
			{
				m_SBDAlg->m_SegmentBright += EnlargeBuf_Org[(y) * ZOOM_ENLAGE_W_ROLL + (x)];
				m_SBDAlg->m_SegmentBrightCnt++;
			}
		}
	}
	if (count == 0)
	{
		return 0;
	}

	if(m_SBDAlg->m_SegmentBrightCnt > 0)
		m_SBDAlg->m_SegmentBright = m_SBDAlg->m_SegmentBright / m_SBDAlg->m_SegmentBrightCnt;
	else
	{
		for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
		{
			for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
			{
				if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W_ROLL + x] == 0)
				{
					continue;
				}

				if(EnlargeBuf_Org[(y) * ZOOM_ENLAGE_W_ROLL + (x)] < 256)
				{
					m_SBDAlg->m_SegmentBright += EnlargeBuf_Org[(y) * ZOOM_ENLAGE_W_ROLL + (x)];
					m_SBDAlg->m_SegmentBrightCnt++;
				}
			}
		}
		if(m_SBDAlg->m_SegmentBrightCnt > 0)
			m_SBDAlg->m_SegmentBright = m_SBDAlg->m_SegmentBright / m_SBDAlg->m_SegmentBrightCnt;
		else
			m_SBDAlg->m_SegmentBright =0;
	}

	mean /= count;
	*CenterX = sum_x / count;
	*CenterY = sum_y / count;

	BrightValue = mean;

	/////////////////////////////////////////////////////////////////////////////////////
	// find calc rect
	int COUNT;
	for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
	{
		COUNT = 0;
		for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
		{
			if (EnlargeBuf[y * ZOOM_ENLAGE_W_ROLL + x] >= Threshold)
			{
				COUNT++;
			}
		}

		if (COUNT > 5)
		{
			m_cImgAnalysis.LEFT = (x - ENLARGESIZE_ZOOM_W_ROLL) * CIS_IMG_W_ROLL / ZOOM_W_ROLL - 30;
			if (m_cImgAnalysis.LEFT < 1)
			{
				m_cImgAnalysis.LEFT = 1;
			}
			break;
		}
	}

	for (x = ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x >= ENLARGESIZE_ZOOM_W_ROLL; x--)
	{
		COUNT = 0;
		for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
		{
			if (EnlargeBuf[y * ZOOM_ENLAGE_W_ROLL + x] >= Threshold)
			{
				COUNT++;
			}
		}

		if (COUNT > 5)
		{
			m_cImgAnalysis.RIGHT = (x - ENLARGESIZE_ZOOM_W_ROLL) * CIS_IMG_W_ROLL / ZOOM_W_ROLL + 30 + 6;
			if (m_cImgAnalysis.RIGHT > CIS_IMG_W_ROLL - 1)
			{
				m_cImgAnalysis.RIGHT = CIS_IMG_W_ROLL - 1;
			}
			break;
		}
	}

	for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
	{
		COUNT = 0;
		for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
		{
			if (EnlargeBuf[y * ZOOM_ENLAGE_W_ROLL + x] >= Threshold)
			{
				COUNT++;
			}
		}

		if (COUNT > 5)
		{
			m_cImgAnalysis.TOP = (y - ENLARGESIZE_ZOOM_H_ROLL) * CIS_IMG_H_ROLL / ZOOM_H_ROLL - 30 - 16;
			if (m_cImgAnalysis.TOP < 1)
			{
				m_cImgAnalysis.TOP = 1;
			}
			break;
		}
	}

	for (y = ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y > ENLARGESIZE_ZOOM_H_ROLL; y--)
	{
		COUNT = 0;
		for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
		{
			if (EnlargeBuf[y * ZOOM_ENLAGE_W_ROLL + x] >= Threshold)
			{
				COUNT++;
			}
		}

		if (COUNT > 5)
		{
			m_cImgAnalysis.BOTTOM = (y - ENLARGESIZE_ZOOM_H_ROLL) * CIS_IMG_H_ROLL / ZOOM_H_ROLL + 30 + 16;
			if (m_cImgAnalysis.BOTTOM > CIS_IMG_H_ROLL - 1)
			{
				m_cImgAnalysis.BOTTOM = CIS_IMG_H_ROLL - 1;
			}
			break;
		}
	}

	return BrightValue;
}

void CIBAlgorithm::_Algo_Five0_AnalysisImage(unsigned char *InImg, unsigned char *OutImg, int ImgSize, int ContrastLevel)
{
    int i, val;

    for (i = 0; i < ImgSize; i++)
    {
		val = (int)InImg[i] - m_TFT_MaskImg[i] - ContrastLevel;
//		val = (int)InImg[i] - m_TFT_MaskImg[i];
        if (val < 0)
        {
            val = 0;
        }
        OutImg[i] = val;//Contrast_LUT[InImg[i]];
    }
}

void CIBAlgorithm::_Algo_Five0_RemoveAnalogTouchSensorNoise(unsigned char *InImg, unsigned char *OutImg, int ImgWidth, int ImgHeight, int ContrastLevel)
{
	int i, j;
	int analog_noise_line[1000];

	memset(analog_noise_line, 0, sizeof(analog_noise_line));

	for (i = 0; i < ImgHeight; i++)
	{
		for (j = 0; j < ImgWidth; j++)
		{
			if(InImg[i*ImgWidth+j] > 0 && InImg[i*ImgWidth+j] < 40)
				analog_noise_line[i]++;
		}
	}

	memcpy(m_Inter_Img5, InImg, ImgWidth*ImgHeight);
	for (i = 1; i < ImgHeight-1; i++)
	{
		if(analog_noise_line[i] > 300)
		{
			for (j = 1; j < ImgWidth-1; j++)
			{
				if(m_Inter_Img5[i*ImgWidth+j] < 20)
					OutImg[i*ImgWidth+j] = 0;
				else
				{
					OutImg[i*ImgWidth+j] = (m_Inter_Img5[(i-1)*ImgWidth+j-1] + 
										m_Inter_Img5[(i-1)*ImgWidth+j] + 
										m_Inter_Img5[(i-1)*ImgWidth+j+1] + 
										m_Inter_Img5[(i+1)*ImgWidth+j-1] +
										m_Inter_Img5[(i+1)*ImgWidth+j] +
										m_Inter_Img5[(i+1)*ImgWidth+j+1])/6;
				}
			}
		}
	}
}
