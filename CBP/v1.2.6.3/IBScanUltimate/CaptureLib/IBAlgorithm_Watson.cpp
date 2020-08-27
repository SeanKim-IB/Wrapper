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
// Watson & Watson Mini Algorithm

void CIBAlgorithm::_Algo_Watson_GetDistortionMask_FromSavedFile()
{
    BOOL         isSameDM     = FALSE;
    const size_t maskElements = (IMG_SIZE + 1); // Image plus checksum.
    const size_t maskSize     = sizeof(DWORD) * maskElements;

    if (m_pUsbDevInfo->bNeedDistortionMask)
    {
        if (m_bDistortionMaskPath)
        {
            FILE *fp = fopen(m_cDistortionMaskPath, "rb");
            if (fp != NULL)
            {
                size_t readSize = fread(arrPos_Watson, 1, maskSize, fp);
                fclose(fp);

                if ((readSize == maskSize) && _Algo_ConfirmChecksum(arrPos_Watson, maskElements))
                {
                    isSameDM = TRUE;
                }
            }

            if (!isSameDM)
            {
                _Algo_Init_Distortion_Merge_with_Bilinear();
                fp = fopen(m_cDistortionMaskPath, "wb");
                if (fp != NULL)
                {
                    _Algo_MakeChecksum(arrPos_Watson, maskElements);
                    fwrite(arrPos_Watson, 1, maskSize, fp);
                    fclose(fp);
                }
            }
        }
        else
        {
            _Algo_Init_Distortion_Merge_with_Bilinear();
        }
    }
}

BOOL CIBAlgorithm::_Algo_Watson_AutoCaptureByGain(BYTE *InImg, int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime)
{
    //	int	ForgraoundCNT=0, fingerCount=0, AreaTres, RecomGain;
    int			AreaTres;
    const int	MAX_BRIGHT = maxBright, MIN_BRIGHT = minBright;
    const int	MIN_CAPTURE_TIME = 600;
#if defined(__android__)
    const int	TOLERANCE = 30;
    const int	MAX_STEP_CHANGE = 3;	// default 3
#elif defined(__embedded__) || defined(WINCE)
    const int	TOLERANCE = 30;
    const int	MAX_STEP_CHANGE = 3;	// default 3
#else
    const int	TOLERANCE = 30;
    const int	MAX_STEP_CHANGE = 5;	// default 3
#endif

    m_cImgAnalysis.max_same_gain_count = 5;
    //	if( ForgraoundCNT < (propertyInfo->nSINGLE_FLAT_AREA_TRES>>4) )
    if (m_cImgAnalysis.foreground_count < 100)
    {
        propertyInfo->nContrastValue = m_pUsbDevInfo->nDefaultContrastValue;
        m_cImgAnalysis.frame_delay = 2;
        m_cImgAnalysis.good_frame_count = 0;
        m_cImgAnalysis.is_final = FALSE;
        m_cImgAnalysis.final_adjust_gain = FALSE;
        m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
        m_cImgAnalysis.gain_step = 0;
        m_cImgAnalysis.changed_gain_step_count = 0;
        m_cImgAnalysis.same_gain_count = 0;
        m_cImgAnalysis.pre_gain = m_pUsbDevInfo->nDefaultContrastValue;
        m_cImgAnalysis.frame_count = 0;
        m_cImgAnalysis.same_gain_time = 0;
        return FALSE;
    }

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
    }

    if (m_cImgAnalysis.finger_count > 0 && m_cImgAnalysis.foreground_count > (propertyInfo->nSINGLE_FLAT_AREA_TRES * m_cImgAnalysis.finger_count))
    {
        int dry_mean_level = 40 + (propertyInfo->nContrastValue - 10) * 3;
        if (m_cImgAnalysis.mean < dry_mean_level)
        {
            m_cImgAnalysis.max_same_gain_count = 60;
        }
    }

    m_cImgAnalysis.frame_count++;
    m_cImgAnalysis.frame_delay--;
    if (m_cImgAnalysis.frame_delay < 0 &&
            !m_cImgAnalysis.is_final &&
            m_cImgAnalysis.changed_gain_step_count < MAX_STEP_CHANGE)
    {
        if (m_cImgAnalysis.mean < MIN_BRIGHT)
        {
            if (m_cImgAnalysis.mean > (MIN_BRIGHT - TOLERANCE))
            {
                // enzyme 2013-01-31 modify to improve capture speed
                //				if( frameTime < 70 )
                //					m_cImgAnalysis.frame_delay = 1;
				propertyInfo->nContrastValue += 1;
            }
            else
            {
				propertyInfo->nContrastValue += 2;
            }

            if (propertyInfo->nContrastValue > __AUTO_CONTRAST_MAX_VALUE__)
            {
                propertyInfo->nContrastValue = __AUTO_CONTRAST_MAX_VALUE__;
            }

            m_cImgAnalysis.good_frame_count = 0;

            if (m_cImgAnalysis.gain_step <= 0)
            {
                m_cImgAnalysis.gain_step = 1;
                m_cImgAnalysis.changed_gain_step_count++;
            }
        }
        else if (m_cImgAnalysis.mean > MAX_BRIGHT)
        {
            if (m_cImgAnalysis.mean < (MAX_BRIGHT + TOLERANCE))
            {
                // enzyme 2013-01-31 modify to improve capture speed
                //				if( frameTime < 70 )
                //					m_cImgAnalysis.frame_delay = 1;
				propertyInfo->nContrastValue -= 1;
            }
            else
            {
				propertyInfo->nContrastValue -= 2;
            }

            if (propertyInfo->nContrastValue < 0)
            {
                propertyInfo->nContrastValue = 0;
            }

            m_cImgAnalysis.good_frame_count = 0;

            if (m_cImgAnalysis.gain_step >= 0)
            {
                m_cImgAnalysis.gain_step = -1;
                m_cImgAnalysis.changed_gain_step_count++;
            }
        }
    }

    if (m_cImgAnalysis.pre_gain == propertyInfo->nContrastValue)
    {
        m_cImgAnalysis.same_gain_count++;
        m_cImgAnalysis.same_gain_time += frameTime;
    }
    else
    {
        m_cImgAnalysis.same_gain_count = 0;
        m_cImgAnalysis.same_gain_time = 0;
    }

    m_cImgAnalysis.pre_gain = propertyInfo->nContrastValue;

    if ((m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT) || m_cImgAnalysis.final_adjust_gain)
    {
        if (m_cImgAnalysis.good_frame_count++ > 0)
        {
            if (m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT)
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
            AreaTres = propertyInfo->nSINGLE_FLAT_AREA_TRES;
        }
        else
        {
            AreaTres = propertyInfo->nSINGLE_FLAT_AREA_TRES * m_cImgAnalysis.finger_count;
        }

        if (m_cImgAnalysis.foreground_count >= AreaTres &&
                abs(m_cImgAnalysis.foreground_count - m_cImgAnalysis.pre_foreground_count) <= SINGLE_FLAT_DIFF_TRES &&
                abs(m_cImgAnalysis.center_x - m_cImgAnalysis.pre_center_x) <= 2 &&
                abs(m_cImgAnalysis.center_y - m_cImgAnalysis.pre_center_y) <= 2
                //			abs(CenterX-Prev_Prev_CenterX) <= 3 &&
                //			abs(CenterY-Prev_Prev_CenterY) <= 3
           )
        {
            if ((m_cImgAnalysis.frame_count * frameTime) > MIN_CAPTURE_TIME)
            {
                m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
                m_cImgAnalysis.is_final = TRUE;
            }
        }
    }

    m_cImgAnalysis.pre_foreground_count = m_cImgAnalysis.foreground_count;
    m_cImgAnalysis.pre_center_x = m_cImgAnalysis.center_x;
    m_cImgAnalysis.pre_center_y = m_cImgAnalysis.center_y;

    return m_cImgAnalysis.is_final;
}

BOOL CIBAlgorithm::_Algo_Watson_AutoCaptureByVoltage(BYTE *InImg, int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime)
{
    //	int	ForgraoundCNT=0, fingerCount=0, AreaTres, RecomGain;
    int			AreaTres;
    const int	MAX_BRIGHT = maxBright, MIN_BRIGHT = minBright;
    const int	MIN_CAPTURE_TIME = 600;
#if defined(__android__)
    const int	TOLERANCE = 30;
    const int	MAX_STEP_CHANGE = 3;	// default 3
#elif defined(__embedded__) || defined(WINCE)
    const int	TOLERANCE = 30;
    const int	MAX_STEP_CHANGE = 3;	// default 3
#else
    const int	TOLERANCE = 30;
    const int	MAX_STEP_CHANGE = 5;	// default 3
#endif

	int nDEFAULT_VOLTAGE_VALUE = __WATSON_DEFAULT_VOLTAGE_VALUE__;
	
    //TRACE("nContrastValue : %d, nVoltageValue : %\n", propertyInfo->nContrastValue, propertyInfo->nVoltageValue);

    m_cImgAnalysis.max_same_gain_count = 5;
    //	if( ForgraoundCNT < (propertyInfo->nSINGLE_FLAT_AREA_TRES>>4) )
    //if (m_cImgAnalysis.foreground_count < 100)
	if (m_cImgAnalysis.isDetected == 0)
    {
        propertyInfo->nContrastValue = m_pUsbDevInfo->nDefaultContrastValue;
		propertyInfo->nVoltageValue = nDEFAULT_VOLTAGE_VALUE;
        m_cImgAnalysis.frame_delay = 2;
        m_cImgAnalysis.good_frame_count = 0;
        m_cImgAnalysis.is_final = FALSE;
        m_cImgAnalysis.final_adjust_gain = FALSE;
        m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
        m_cImgAnalysis.gain_step = 0;
        m_cImgAnalysis.changed_gain_step_count = 0;
        m_cImgAnalysis.same_gain_count = 0;
		m_cImgAnalysis.pre_gain = nDEFAULT_VOLTAGE_VALUE;
        m_cImgAnalysis.frame_count = 0;
        m_cImgAnalysis.same_gain_time = 0;
        m_cImgAnalysis.is_complete_voltage_control = FALSE;
        return FALSE;
    }

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
        m_cImgAnalysis.is_complete_voltage_control = FALSE;
    }

    if (m_cImgAnalysis.finger_count > 0 && m_cImgAnalysis.foreground_count > (propertyInfo->nSINGLE_FLAT_AREA_TRES * m_cImgAnalysis.finger_count))
    {
        int dry_mean_level = 40 + (propertyInfo->nContrastValue - 10) * 3;
        if (m_cImgAnalysis.mean < dry_mean_level)
        {
            m_cImgAnalysis.max_same_gain_count = 60;
        }
    }

    m_cImgAnalysis.frame_count++;
    m_cImgAnalysis.frame_delay--;
    if (m_cImgAnalysis.frame_delay < 0 &&
            //!m_cImgAnalysis.is_final &&
            m_cImgAnalysis.changed_gain_step_count < MAX_STEP_CHANGE)
    {
        if (m_cImgAnalysis.mean < MIN_BRIGHT)
        {
            if (m_cImgAnalysis.changed_gain_step_count > 2/* || m_cImgAnalysis.mean > (MIN_BRIGHT-TOLERANCE)*/)
            {
                m_cImgAnalysis.frame_delay = 1;
            }

            if (propertyInfo->nVoltageValue >= __AUTO_VOLTAGE_MAX_VALUE__)
            {
                m_cImgAnalysis.is_complete_voltage_control = TRUE;
            }

            if (!m_cImgAnalysis.is_complete_voltage_control &&
                propertyInfo->nContrastValue >= __AUTO_CONTRAST_MAX_VALUE__)
            {
                // Let's change voltage
                propertyInfo->nVoltageValue += 1;

                if (propertyInfo->nVoltageValue > __AUTO_VOLTAGE_MAX_VALUE__)
                {
                    propertyInfo->nVoltageValue = __AUTO_VOLTAGE_MAX_VALUE__;
                }
                m_cImgAnalysis.frame_delay = 3;
            }
            else
            {
                // Let's change gain
                if (m_cImgAnalysis.mean > (MIN_BRIGHT - TOLERANCE))
                {
                    //					if( frameTime < 70 )
                    //						m_cImgAnalysis.frame_delay = 1;
					propertyInfo->nContrastValue += 1;
                }
                else
                {
#ifdef __android__
                    propertyInfo->nContrastValue += 3;
#else
					propertyInfo->nContrastValue += 2;
#endif
                }

                if (propertyInfo->nContrastValue > __AUTO_CONTRAST_MAX_VALUE__)
                {
                    propertyInfo->nContrastValue = __AUTO_CONTRAST_MAX_VALUE__;
                }
            }

            m_cImgAnalysis.good_frame_count = 0;

            if (m_cImgAnalysis.gain_step <= 0)
            {
                m_cImgAnalysis.gain_step = 1;
                m_cImgAnalysis.changed_gain_step_count++;
            }
        }
        else if (m_cImgAnalysis.mean > MAX_BRIGHT)
        {
            // enzyme 2013-01-31 modify to improve capture speed
            if (m_cImgAnalysis.changed_gain_step_count > 2/* || m_cImgAnalysis.mean < (MAX_BRIGHT+TOLERANCE)*/)
            {
                m_cImgAnalysis.frame_delay = 1;
            }

            if (propertyInfo->nVoltageValue <= nDEFAULT_VOLTAGE_VALUE)
            {
                m_cImgAnalysis.is_complete_voltage_control = TRUE;
            }

            if (!m_cImgAnalysis.is_complete_voltage_control)
            {
                // Let's change voltage
                propertyInfo->nVoltageValue -= 1;

                if (propertyInfo->nVoltageValue < nDEFAULT_VOLTAGE_VALUE)
                {
                    propertyInfo->nVoltageValue = nDEFAULT_VOLTAGE_VALUE;
                }
            }
            else
            {
                // Let's change gain
                if (m_cImgAnalysis.mean < (MAX_BRIGHT + TOLERANCE))
                {
                    //					// enzyme 2013-01-31 modify to improve capture speed
                    //					if( frameTime < 70 )
                    //						m_cImgAnalysis.frame_delay = 1;
					propertyInfo->nContrastValue -= 1;
                }
                else
                {
#ifdef __android__
                    propertyInfo->nContrastValue -= 3;
#else
					propertyInfo->nContrastValue -= 2;
#endif
                }

                if (propertyInfo->nContrastValue < 0)
                {
                    propertyInfo->nContrastValue = 0;
                }
            }

            m_cImgAnalysis.good_frame_count = 0;

            if (m_cImgAnalysis.gain_step >= 0)
            {
                m_cImgAnalysis.gain_step = -1;
                m_cImgAnalysis.changed_gain_step_count++;
            }
        }
    }

    if (m_cImgAnalysis.pre_gain == propertyInfo->nContrastValue &&
            abs(m_cImgAnalysis.foreground_count - m_cImgAnalysis.pre_foreground_count) <= SINGLE_FLAT_DIFF_TRES)
    {
        m_cImgAnalysis.same_gain_count++;
        m_cImgAnalysis.same_gain_time += frameTime;
    }
    else
    {
        m_cImgAnalysis.same_gain_count = 0;
        m_cImgAnalysis.same_gain_time = 0;
    }

    m_cImgAnalysis.pre_gain = propertyInfo->nContrastValue;

    if ((m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT) || m_cImgAnalysis.final_adjust_gain)
    {
        if (m_cImgAnalysis.good_frame_count++ > 0)
        {
            if (m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT)
            {
                m_cImgAnalysis.final_adjust_gain = TRUE;
                m_cImgAnalysis.frame_delay = -1;
            }
        }
    }

    //TRACE("<====> frame_delay=%d, mean=%d, gain=%d, voltage=%d, good_frame_count=%d, same_gain_count=%d, same_gain_time=%d, frame_count=%d\n",
    //		m_cImgAnalysis.frame_delay, m_cImgAnalysis.mean, propertyInfo->nContrastValue,
    //		propertyInfo->nVoltageValue, m_cImgAnalysis.good_frame_count,
    //		m_cImgAnalysis.same_gain_count, m_cImgAnalysis.same_gain_time,
    //		m_cImgAnalysis.frame_count);


    if ((m_cImgAnalysis.final_adjust_gain && m_cImgAnalysis.frame_delay < 0 && m_cImgAnalysis.good_frame_count > 1) ||
            m_cImgAnalysis.same_gain_count >= m_cImgAnalysis.max_same_gain_count ||
            m_cImgAnalysis.same_gain_time > (int)(m_cImgAnalysis.max_same_gain_count * frameTime)
       )
    {
        if (m_cImgAnalysis.finger_count == 0)
        {
            AreaTres = propertyInfo->nSINGLE_FLAT_AREA_TRES;
        }
        else
        {
            AreaTres = propertyInfo->nSINGLE_FLAT_AREA_TRES * m_cImgAnalysis.finger_count;
        }

        if (m_cImgAnalysis.foreground_count >= AreaTres &&
                abs(m_cImgAnalysis.foreground_count - m_cImgAnalysis.pre_foreground_count) <= SINGLE_FLAT_DIFF_TRES &&
                abs(m_cImgAnalysis.center_x - m_cImgAnalysis.pre_center_x) <= 2 &&
                abs(m_cImgAnalysis.center_y - m_cImgAnalysis.pre_center_y) <= 2
                //			abs(CenterX-Prev_Prev_CenterX) <= 3 &&
                //			abs(CenterY-Prev_Prev_CenterY) <= 3
           )
        {
            if ((m_cImgAnalysis.frame_count * frameTime) > MIN_CAPTURE_TIME)
            {
                m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
                m_cImgAnalysis.is_final = TRUE;
            }
        }
    }

    m_cImgAnalysis.pre_foreground_count = m_cImgAnalysis.foreground_count;
    m_cImgAnalysis.pre_center_x = m_cImgAnalysis.center_x;
    m_cImgAnalysis.pre_center_y = m_cImgAnalysis.center_y;

    return m_cImgAnalysis.is_final;
}

BOOL CIBAlgorithm::_Algo_Watson_AutoCaptureByExposure(BYTE *InImg, int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime)
{
    //	int	ForgraoundCNT=0, fingerCount=0, AreaTres, RecomGain;
    int			AreaTres;
    const int	MAX_BRIGHT = maxBright, MIN_BRIGHT = minBright;
    const int	MIN_CAPTURE_TIME = 600;
#if defined(__android__)
    const int	TOLERANCE = 30;
    const int	MAX_STEP_CHANGE = 3;	// default 3
#elif defined(__embedded__) || defined(WINCE)
    const int	TOLERANCE = 30;
    const int	MAX_STEP_CHANGE = 3;	// default 3
#else
    const int	TOLERANCE = 30;
    const int	MAX_STEP_CHANGE = 5;	// default 3
#endif

    //TRACE("nContrastValue : %d, nVoltageValue : %\n", propertyInfo->nContrastValue, propertyInfo->nVoltageValue);

    m_cImgAnalysis.max_same_gain_count = 5;
    //	if( ForgraoundCNT < (propertyInfo->nSINGLE_FLAT_AREA_TRES>>4) )
    //if (m_cImgAnalysis.foreground_count < 100)
	if (m_cImgAnalysis.isDetected == 0)
    {
        propertyInfo->nContrastValue = m_pUsbDevInfo->nDefaultContrastValue;
		propertyInfo->nVoltageValue = __WATSON_DEFAULT_VOLTAGE_VALUE__;
		propertyInfo->nExposureValue = __WATSON_DEFAULT_EXPOSURE_VALUE__;
        m_cImgAnalysis.frame_delay = 2;
        m_cImgAnalysis.good_frame_count = 0;
        m_cImgAnalysis.is_final = FALSE;
        m_cImgAnalysis.final_adjust_gain = FALSE;
        m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
        m_cImgAnalysis.gain_step = 0;
        m_cImgAnalysis.changed_gain_step_count = 0;
        m_cImgAnalysis.same_gain_count = 0;
		m_cImgAnalysis.pre_gain = __WATSON_DEFAULT_VOLTAGE_VALUE__;
		m_cImgAnalysis.pre_exp = __WATSON_DEFAULT_EXPOSURE_VALUE__;
        m_cImgAnalysis.frame_count = 0;
        m_cImgAnalysis.same_gain_time = 0;
        m_cImgAnalysis.is_complete_voltage_control = FALSE;
        m_cImgAnalysis.is_complete_exposure_control = FALSE;
        return FALSE;
    }

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
        m_cImgAnalysis.is_complete_voltage_control = FALSE;
        m_cImgAnalysis.is_complete_exposure_control = FALSE;
    }

    if (m_cImgAnalysis.finger_count > 0 && m_cImgAnalysis.foreground_count > (propertyInfo->nSINGLE_FLAT_AREA_TRES * m_cImgAnalysis.finger_count))
    {
        int dry_mean_level = 40 + (propertyInfo->nContrastValue - 10) * 3;
        if (m_cImgAnalysis.mean < dry_mean_level)
        {
            m_cImgAnalysis.max_same_gain_count = 60;
        }
    }

    m_cImgAnalysis.frame_count++;
    m_cImgAnalysis.frame_delay--;
    if (m_cImgAnalysis.frame_delay < 0 &&
            //!m_cImgAnalysis.is_final &&
            m_cImgAnalysis.changed_gain_step_count < MAX_STEP_CHANGE)
    {
        if (m_cImgAnalysis.mean < MIN_BRIGHT)
        {
            if (m_cImgAnalysis.changed_gain_step_count > 2/* || m_cImgAnalysis.mean > (MIN_BRIGHT-TOLERANCE)*/)
            {
                m_cImgAnalysis.frame_delay = 1;
            }

			if(propertyInfo->nExposureValue < __WATSON_DEFAULT_EXPOSURE_VALUE__	)
			{
				propertyInfo->nExposureValue += 0x64;
				if(propertyInfo->nExposureValue > __WATSON_DEFAULT_EXPOSURE_VALUE__ )
					propertyInfo->nExposureValue = __WATSON_DEFAULT_EXPOSURE_VALUE__;
			}
			else
			{
				if (propertyInfo->nVoltageValue >= __AUTO_VOLTAGE_MAX_VALUE__)
				{
					m_cImgAnalysis.is_complete_voltage_control = TRUE;
				}

				if (!m_cImgAnalysis.is_complete_voltage_control &&
					propertyInfo->nContrastValue >= __AUTO_CONTRAST_MAX_VALUE__)
				{
					// Let's change voltage
					propertyInfo->nVoltageValue += 1;

					if (propertyInfo->nVoltageValue > __AUTO_VOLTAGE_MAX_VALUE__)
					{
						propertyInfo->nVoltageValue = __AUTO_VOLTAGE_MAX_VALUE__;
					}
					m_cImgAnalysis.frame_delay = 3;
				}
				else
				{
					// Let's change gain
					if (m_cImgAnalysis.mean > (MIN_BRIGHT - TOLERANCE))
					{
						propertyInfo->nContrastValue += 1;
					}
					else
					{
#ifdef __android__
						propertyInfo->nContrastValue += 3;
#else
						propertyInfo->nContrastValue += 2;
#endif
					}

					if (propertyInfo->nContrastValue > __AUTO_CONTRAST_MAX_VALUE__)
					{
						propertyInfo->nContrastValue = __AUTO_CONTRAST_MAX_VALUE__;
					}
				}
			}

            m_cImgAnalysis.good_frame_count = 0;

            if (m_cImgAnalysis.gain_step <= 0)
            {
                m_cImgAnalysis.gain_step = 1;
                m_cImgAnalysis.changed_gain_step_count++;
            }
        }
        else if (m_cImgAnalysis.mean > MAX_BRIGHT)
        {
            // enzyme 2013-01-31 modify to improve capture speed
            if (m_cImgAnalysis.changed_gain_step_count > 2/* || m_cImgAnalysis.mean < (MAX_BRIGHT+TOLERANCE)*/)
            {
                m_cImgAnalysis.frame_delay = 1;
            }

			if (propertyInfo->nVoltageValue <= __WATSON_DEFAULT_VOLTAGE_VALUE__)
			{
				m_cImgAnalysis.is_complete_voltage_control = TRUE;
			}

			if (!m_cImgAnalysis.is_complete_voltage_control)
			{
				// Let's change voltage
				propertyInfo->nVoltageValue -= 1;

				if (propertyInfo->nVoltageValue < __WATSON_DEFAULT_VOLTAGE_VALUE__)
				{
					propertyInfo->nVoltageValue = __WATSON_DEFAULT_VOLTAGE_VALUE__;
				}
			}
			else
			{
				// first control gain value, and then exposure
				if(propertyInfo->nContrastValue <= 0)
				{
					if(propertyInfo->nExposureValue > __WATSON_MINIMUM_EXPOSURE_VALUE__	)
					{
						propertyInfo->nExposureValue -= 0x64;
						if(propertyInfo->nExposureValue <= __WATSON_MINIMUM_EXPOSURE_VALUE__ )
							propertyInfo->nExposureValue = __WATSON_MINIMUM_EXPOSURE_VALUE__;
					}
				}
				else
				{
					// Let's change gain
					if (m_cImgAnalysis.mean < (MAX_BRIGHT + TOLERANCE))
					{
						propertyInfo->nContrastValue -= 1;
					}
					else
					{
#ifdef __android__
						propertyInfo->nContrastValue -= 3;
#else
						propertyInfo->nContrastValue -= 2;
#endif
					}

					if (propertyInfo->nContrastValue < 0)
					{
						propertyInfo->nContrastValue = 0;
					}
				}
			}

			m_cImgAnalysis.good_frame_count = 0;

			if (m_cImgAnalysis.gain_step >= 0)
			{
				m_cImgAnalysis.gain_step = -1;
				m_cImgAnalysis.changed_gain_step_count++;
			}
        }
    }

    if (m_cImgAnalysis.pre_gain == propertyInfo->nContrastValue &&
		m_cImgAnalysis.pre_exp == propertyInfo->nExposureValue &&
            abs(m_cImgAnalysis.foreground_count - m_cImgAnalysis.pre_foreground_count) <= SINGLE_FLAT_DIFF_TRES)
    {
        m_cImgAnalysis.same_gain_count++;
        m_cImgAnalysis.same_gain_time += frameTime;
    }
    else
    {
        m_cImgAnalysis.same_gain_count = 0;
        m_cImgAnalysis.same_gain_time = 0;
    }

    m_cImgAnalysis.pre_gain = propertyInfo->nContrastValue;
    m_cImgAnalysis.pre_exp = propertyInfo->nExposureValue;

    if ((m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT) || m_cImgAnalysis.final_adjust_gain)
    {
        if (m_cImgAnalysis.good_frame_count++ > 0)
        {
            if (m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT)
            {
                m_cImgAnalysis.final_adjust_gain = TRUE;
                m_cImgAnalysis.frame_delay = -1;
            }
        }
    }

    //TRACE("<====> frame_delay=%d, mean=%d, gain=%d, voltage=%d, good_frame_count=%d, same_gain_count=%d, same_gain_time=%d, frame_count=%d\n",
    //		m_cImgAnalysis.frame_delay, m_cImgAnalysis.mean, propertyInfo->nContrastValue,
    //		propertyInfo->nVoltageValue, m_cImgAnalysis.good_frame_count,
    //		m_cImgAnalysis.same_gain_count, m_cImgAnalysis.same_gain_time,
    //		m_cImgAnalysis.frame_count);


    if ((m_cImgAnalysis.final_adjust_gain && m_cImgAnalysis.frame_delay < 0 && m_cImgAnalysis.good_frame_count > 1) ||
            m_cImgAnalysis.same_gain_count >= m_cImgAnalysis.max_same_gain_count ||
            m_cImgAnalysis.same_gain_time > (int)(m_cImgAnalysis.max_same_gain_count * frameTime)
       )
    {
        if (m_cImgAnalysis.finger_count == 0)
        {
            AreaTres = propertyInfo->nSINGLE_FLAT_AREA_TRES;
        }
        else
        {
            AreaTres = propertyInfo->nSINGLE_FLAT_AREA_TRES * m_cImgAnalysis.finger_count;
        }

        if (m_cImgAnalysis.foreground_count >= AreaTres &&
                abs(m_cImgAnalysis.foreground_count - m_cImgAnalysis.pre_foreground_count) <= SINGLE_FLAT_DIFF_TRES &&
                abs(m_cImgAnalysis.center_x - m_cImgAnalysis.pre_center_x) <= 2 &&
                abs(m_cImgAnalysis.center_y - m_cImgAnalysis.pre_center_y) <= 2
                //			abs(CenterX-Prev_Prev_CenterX) <= 3 &&
                //			abs(CenterY-Prev_Prev_CenterY) <= 3
           )
        {
            if ((m_cImgAnalysis.frame_count * frameTime) > MIN_CAPTURE_TIME)
            {
                m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
                m_cImgAnalysis.is_final = TRUE;
            }
        }
    }

    m_cImgAnalysis.pre_foreground_count = m_cImgAnalysis.foreground_count;
    m_cImgAnalysis.pre_center_x = m_cImgAnalysis.center_x;
    m_cImgAnalysis.pre_center_y = m_cImgAnalysis.center_y;

    return m_cImgAnalysis.is_final;
}

#if defined(__IBSCAN_SDK__)
void CIBAlgorithm::_Algo_MakeWatson_250DPI(unsigned char *InImg, unsigned char *OutImg)
{
	int i, j;
	int LEFT, RIGHT, TOP, BOTTOM;
	int HALF_IMG_W = IMG_W>>1;
	memset(OutImg, 0, IMG_SIZE);

	TOP = (m_cImgAnalysis.TOP*IMG_H/CIS_IMG_H>>1)<<1;
	BOTTOM = (m_cImgAnalysis.BOTTOM*IMG_H/CIS_IMG_H>>1)<<1;
	LEFT = (m_cImgAnalysis.LEFT*IMG_W/CIS_IMG_W>>1)<<1;
	RIGHT = (m_cImgAnalysis.RIGHT*IMG_W/CIS_IMG_W>>1)<<1;

	for( i=TOP; i<=BOTTOM; i+=2 )
	{
		for( j=LEFT; j<=RIGHT; j+=2 )
		{
			OutImg[(i>>1)*HALF_IMG_W+(j>>1)] = (InImg[i*IMG_W+j] + InImg[i*IMG_W+j+1] + InImg[(i+1)*IMG_W+j] + InImg[(i+1)*IMG_W+j+1])>>2;
		}
	}
}
#endif

