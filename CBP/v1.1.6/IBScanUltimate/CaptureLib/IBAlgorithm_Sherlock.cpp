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
// Sherlock Algorithm



BOOL CIBAlgorithm::_Algo_Sherlock_AutoCapture(BYTE *InImg, int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime)
{
    //	int	ForgraoundCNT=0, fingerCount=0, AreaTres, RecomGain;
    int			AreaTres;
    const int	MAX_BRIGHT = maxBright, MIN_BRIGHT = minBright;
    const int	TOLERANCE = 25;
    const int	MAX_STEP_CHANGE = 3;	// default 3
    const int	MIN_FRAME_COUNT = 8;

    m_cImgAnalysis.max_same_gain_count = 2;
    //	if( ForgraoundCNT < (propertyInfo->nSINGLE_FLAT_AREA_TRES>>4) )
    //TRACE("<====> forground_count=%d\n", m_cImgAnalysis.foreground_count);
    if (m_cImgAnalysis.foreground_count < 100)
    {
        propertyInfo->nContrastValue = m_pUsbDevInfo->nDefaultContrastValue;
        m_cImgAnalysis.frame_delay = 1;
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
        m_cImgAnalysis.is_complete_voltage_control = FALSE;
		m_cImgAnalysis.bFinger_detected_on_Sherlock = FALSE;
        return FALSE;
    }

	if (m_cImgAnalysis.bFinger_detected_on_Sherlock == FALSE &&
		!m_cImgAnalysis.bLowLeOnClock_on_Sherock &&
		m_cImgAnalysis.foreground_count > 500)
	{
		m_cImgAnalysis.bFinger_detected_on_Sherlock = TRUE;
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
            m_cImgAnalysis.max_same_gain_count = 10;
        }
    }

    m_cImgAnalysis.frame_count++;
    m_cImgAnalysis.frame_delay--;
    if (m_cImgAnalysis.frame_delay < 0 &&
            //!m_cImgAnalysis.is_final &&
            m_cImgAnalysis.changed_gain_step_count < MAX_STEP_CHANGE)
    {
		int nMinBright = MIN_BRIGHT;
		int nMaxBright = MAX_BRIGHT;
		if (propertyInfo->nContrastValue < __ROIC_VOLTAGE_MAX_VALUE__ && 
			propertyInfo->nContrastValue > (__ASIC_VOLTAGE_DEFAULT_VALUE__) && 
			m_cImgAnalysis.mean < (MIN_BRIGHT + 20))
		{
			nMinBright = (MIN_BRIGHT + 20);
			nMaxBright = (MAX_BRIGHT + 15);
		}

        if (m_cImgAnalysis.mean < nMinBright)
        {
            //			m_propertyInfo.nContrastValue += 1;
            if (m_cImgAnalysis.mean > (MIN_BRIGHT - TOLERANCE))
            {
                //				if( frameTime < 70 )
                //					m_cImgAnalysis.frame_delay = 1;
                propertyInfo->nContrastValue += 1;
            }
            else
            {
                propertyInfo->nContrastValue += 3;
            }

            if (propertyInfo->nContrastValue > __ROIC_VOLTAGE_MAX_VALUE__)
            {
                propertyInfo->nContrastValue = __ROIC_VOLTAGE_MAX_VALUE__;
            }

            m_cImgAnalysis.good_frame_count = 0;

            if (m_cImgAnalysis.gain_step <= 0)
            {
                m_cImgAnalysis.gain_step = 1;
                m_cImgAnalysis.changed_gain_step_count++;
            }
        }
        else if (m_cImgAnalysis.mean > nMaxBright)
        {
            //			m_propertyInfo.nContrastValue -= 1;
            if (m_cImgAnalysis.mean < (MAX_BRIGHT + TOLERANCE))
            {
                //				if( frameTime < 70 )
                //					m_cImgAnalysis.frame_delay = 1;
                propertyInfo->nContrastValue -= 1;
            }
            else
            {
                propertyInfo->nContrastValue -= 3;
            }

            if (propertyInfo->nContrastValue < __ROIC_VOLTAGE_MIN_VALUE__)
            {
                propertyInfo->nContrastValue = __ROIC_VOLTAGE_MIN_VALUE__;
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

    //TRACE("<====> mean=%d, voltage=%d, good_frame_count=%d, same_gain_count=%d, same_gain_time=%d, frame_count=%d\n",
    //		m_cImgAnalysis.mean, propertyInfo->nContrastValue,
    //		m_cImgAnalysis.good_frame_count, m_cImgAnalysis.same_gain_count,
    //		m_cImgAnalysis.same_gain_time, m_cImgAnalysis.frame_count);

    if ((m_cImgAnalysis.final_adjust_gain && m_cImgAnalysis.frame_delay < 0 && m_cImgAnalysis.good_frame_count > 1) ||
            m_cImgAnalysis.same_gain_count >= m_cImgAnalysis.max_same_gain_count ||
            m_cImgAnalysis.same_gain_time > (int)(m_cImgAnalysis.max_same_gain_count * 60)
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
            if (m_cImgAnalysis.frame_count > MIN_FRAME_COUNT)
            {
                m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
                m_cImgAnalysis.is_final = TRUE;
            }
        }
    }

    m_cImgAnalysis.pre_gain = propertyInfo->nContrastValue;
    m_cImgAnalysis.pre_foreground_count = m_cImgAnalysis.foreground_count;
    m_cImgAnalysis.pre_center_x = m_cImgAnalysis.center_x;
    m_cImgAnalysis.pre_center_y = m_cImgAnalysis.center_y;

    return m_cImgAnalysis.is_final;
}

void CIBAlgorithm::_Algo_ROIC_AnalysisImage(unsigned char *InImg)
{
    int x, y;
    int value, y_index;
    int noise_histo[4], noise_count[4];

    memset(noise_histo, 0, sizeof(noise_histo));

    //Initialization
    m_cImgAnalysis.noise_histo = 0;

    //Image Information Analysis
    // Noise Histo
    memset(noise_histo, 0, sizeof(int) * 4);
    memset(noise_count, 0, sizeof(int) * 4);

    for (y = 0; y < 50; y++)
    {
        y_index = y * CIS_IMG_W;
        for (x = 0; x < 50; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[0] < value/* && value < 60*/)
            {
                noise_histo[0] = value;
            }
        }
        for (x = CIS_IMG_W - 50; x < CIS_IMG_W; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[1] < value/* && value < 60*/)
            {
                noise_histo[1] = value;
            }
        }
    }
    for (y = CIS_IMG_H - 50; y < CIS_IMG_H; y++)
    {
        y_index = y * CIS_IMG_W;
        for (x = 0; x < 50; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[2] < value/* && value < 60*/)
            {
                noise_histo[2] = value;
            }
        }
        for (x = CIS_IMG_W - 50; x < CIS_IMG_W; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[3] < value/* && value < 60*/)
            {
                noise_histo[3] = value;
            }
        }
    }

    /*	for(y=1; y<4; y++)
    	{
    		if(noise_count[y]>0)
    			noise_histo[y] /= noise_count[y];
    	}
    */
    int min_noise_histo = noise_histo[0], min_noise_idx = 0;
    for (y = 1; y < 4; y++)
    {
        if (min_noise_histo > noise_histo[y])
        {
            min_noise_histo = noise_histo[y];
            min_noise_idx = y;
        }
    }

    m_cImgAnalysis.noise_histo = noise_histo[min_noise_idx];			// mean of noise image
}

void CIBAlgorithm::_Algo_ASIC_AnalysisImage(unsigned char *InImg)
{
    // 2013-04-03 Gon modify : when ASIC noise removed, we have to consider m_TFT_MaskImg
    int x, y;
    int y_index;
    int noise_histo[4], noise_count[4];
    int min_noise_histo, min_noise_idx;
    int window_size = 64, val;

	int WIDTH = CIS_IMG_H;
	int HEIGHT = CIS_IMG_W;

	if(!m_pPropertyInfo->bNoPreviewImage)
	{
		WIDTH = CIS_IMG_W;
		HEIGHT = CIS_IMG_H;
	}

    //Initialization
    m_cImgAnalysis.noise_histo = 0;
    m_cImgAnalysis.noise_histo2 = 0;

    //Image Information Analysis
    // Noise Histo
    memset(noise_histo, 0, sizeof(int) * 4);
    memset(noise_count, 0, sizeof(int) * 4);

    for (y = 0; y < window_size; y++)
    {
        y_index = y * WIDTH;
        for (x = 0; x < window_size; x++)
        {
            val = InImg[y_index + x] - m_TFT_MaskImg[y_index + x];
            if (val > 0)
            {
                noise_histo[0] += InImg[y_index + x];
            }
        }

        for (x = WIDTH - window_size; x < WIDTH; x++)
        {
            val = InImg[y_index + x] - m_TFT_MaskImg[y_index + x];
            if (val > 0)
            {
                noise_histo[1] += InImg[y_index + x];
            }
        }
    }
    for (y = HEIGHT / 2 - window_size - 10; y < HEIGHT / 2 - 10; y++)
    {
        y_index = y * WIDTH;
        for (x = 0; x < window_size; x++)
        {
            val = InImg[y_index + x] - m_TFT_MaskImg[y_index + x];
            if (val > 0)
            {
                noise_histo[2] += InImg[y_index + x];
            }
        }

        for (x = WIDTH - window_size; x < WIDTH; x++)
        {
            val = InImg[y_index + x] - m_TFT_MaskImg[y_index + x];
            if (val > 0)
            {
                noise_histo[3] += InImg[y_index + x];
            }
        }
    }

    min_noise_histo = noise_histo[0];
    min_noise_idx = 0;
    for (y = 1; y < 4; y++)
    {
        if (min_noise_histo > noise_histo[y])
        {
            min_noise_histo = noise_histo[y];
            min_noise_idx = y;
        }
    }

    m_cImgAnalysis.noise_histo = noise_histo[min_noise_idx] / (window_size * window_size) + 5;			// mean of noise image

    memset(noise_histo, 0, sizeof(int) * 4);
    memset(noise_count, 0, sizeof(int) * 4);

    for (y = HEIGHT / 2 + 10; y < HEIGHT / 2 + 10 + window_size; y++)
    {
        y_index = y * WIDTH;
        for (x = 0; x < window_size; x++)
        {
            val = InImg[y_index + x] - m_TFT_MaskImg[y_index + x];
            if (val > 0)
            {
                noise_histo[0] += InImg[y_index + x];
            }
        }

        for (x = WIDTH - window_size; x < WIDTH; x++)
        {
            val = InImg[y_index + x] - m_TFT_MaskImg[y_index + x];
            if (val > 0)
            {
                noise_histo[1] += InImg[y_index + x];
            }
        }
    }
    for (y = HEIGHT - window_size; y < HEIGHT; y++)
    {
        y_index = y * WIDTH;
        for (x = 0; x < window_size; x++)
        {
            val = InImg[y_index + x] - m_TFT_MaskImg[y_index + x];
            if (val > 0)
            {
                noise_histo[2] += InImg[y_index + x];
            }
        }

        for (x = WIDTH - window_size; x < WIDTH; x++)
        {
            val = InImg[y_index + x] - m_TFT_MaskImg[y_index + x];
            if (val > 0)
            {
                noise_histo[3] += InImg[y_index + x];
            }
        }
    }

    min_noise_histo = noise_histo[0];
    min_noise_idx = 0;
    for (y = 1; y < 4; y++)
    {
        if (min_noise_histo > noise_histo[y])
        {
            min_noise_histo = noise_histo[y];
            min_noise_idx = y;
        }
    }

    m_cImgAnalysis.noise_histo2 = noise_histo[min_noise_idx] / (window_size * window_size) + 5;			// mean of noise image

    if (m_cImgAnalysis.noise_histo2 > m_cImgAnalysis.noise_histo)
    {
        m_cImgAnalysis.noise_histo = m_cImgAnalysis.noise_histo2;
    }
}

void CIBAlgorithm::_Algo_ROIC_RemoveNoise(unsigned char *InImg, unsigned char *OutImg, int ImgSize, int ContrastLevel)
{
    int i, val;

    int minus_value = m_cImgAnalysis.noise_histo + 30;

    for (i = 0; i < ImgSize; i++)
    {
        val = (int)InImg[i] - minus_value;
        if (val < 0)
        {
            val = 0;
        }
        OutImg[i] = val;//Contrast_LUT[InImg[i]];
    }
}

int CIBAlgorithm::_Algo_ROIC_GetBrightWithRawImage(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY)
{
    //	struct timeval tv;
    //	struct timeval start_tv;
    //	double elapsed = 0.0;

    int y, x, i, j;
    int mean = 0, count = 0;
    int ii, tempsum;
    int xx, yy, forgroundCNT = 0;
    int value;
    int sum_x = 0, sum_y = 0;
    int BrightValue;

    //gettimeofday(&start_tv, NULL);
    memset(m_segment_enlarge_buffer, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
    memset(m_segment_enlarge_buffer_for_fingercnt, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
    memset(EnlargeBuf, 0, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);
    *CenterX = -1;
    *CenterY = -1;
    *ForgraoundCNT = 0;

    m_cImgAnalysis.LEFT = 1;
    m_cImgAnalysis.RIGHT = 1;
    m_cImgAnalysis.TOP = 1;
    m_cImgAnalysis.BOTTOM = 1;

    if (!m_pPropertyInfo->bNoPreviewImage || m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER || m_bRotatedImage == FALSE)
    {
        for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
        {
            yy = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H * CIS_IMG_W;

            for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
            {
                xx = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W;

                value = InRawImg[yy + xx];
                //			value = ( ((int)value*m_UM_Watson_F[yy+xx]) >> 10 ) + (int)value;
                //			if(value > 255) value = 255;
                //			else if(value < 0) value = 0;
                EnlargeBuf[(y)*ZOOM_ENLAGE_W + (x)] = value;
            }
        }
    }
    else
    {
        for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
        {
            yy = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H;

            for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
            {
                xx = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W;

                value = InRawImg[(CIS_IMG_W-1-xx) * CIS_IMG_H + yy] - m_TFT_MaskImg[(CIS_IMG_W-1-xx) * CIS_IMG_H + yy] - m_cImgAnalysis.noise_histo;
                if (value <0) value = 0;
                //			value = ( ((int)value*m_UM_Watson_F[yy+xx]) >> 10 ) + (int)value;
                //			if(value > 255) value = 255;
                //			else if(value < 0) value = 0;
                EnlargeBuf[(y)*ZOOM_ENLAGE_W + (x)] = value;
            }
        }
    }

    memcpy(EnlargeBuf_Org, EnlargeBuf, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);

    _Algo_HistogramStretchForZoomEnlarge(EnlargeBuf);

    mean = 0;
    count = 0;
    for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
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

    /*	mean=0;
    	count=0;
    	for (y=ENLARGESIZE_ZOOM_H; y<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; y++)
    	{
    		for (x=ENLARGESIZE_ZOOM_W; x<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; x++)
    		{
    			if(m_segment_enlarge_buffer[y*ZOOM_ENLAGE_W+x]==0)
    				continue;

    			mean += EnlargeBuf[(y)*ZOOM_ENLAGE_W+(x)];
    			count++;
    			sum_x += x*ZOOM_OUT;
    			sum_y += y*ZOOM_OUT;
    		}
    	}
    //gettimeofday(&tv, NULL);
    //elapsed = (tv.tv_sec - start_tv.tv_sec) + (tv.tv_usec - start_tv.tv_usec) / 1000.0;
    //printf("_Algo_GetBrightWithRawImage(): %1.4f milliseconds\n", elapsed);
    */
    //	int Threshold = mean/20;
    //	//if(Threshold == 0)
    //		Threshold = 1;

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

            if (EnlargeBuf_Org[(y)*ZOOM_ENLAGE_W + (x)] >= Threshold)
            {
                mean += EnlargeBuf_Org[(y) * ZOOM_ENLAGE_W + (x)];
                count++;
                sum_x += x * ZOOM_OUT;
                sum_y += y * ZOOM_OUT;
            }
        }
    }
    if (count == 0)
    {
        return 0;
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
            //			if(m_segment_enlarge_buffer[y*ZOOM_ENLAGE_W+x]==255)
            if (EnlargeBuf[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            m_cImgAnalysis.LEFT = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W - 30;
            //			m_cImgAnalysis.LEFT=(x-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W - 16;
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
            //			if(m_segment_enlarge_buffer[y*ZOOM_ENLAGE_W+x]==255)
            if (EnlargeBuf[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            m_cImgAnalysis.RIGHT = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W + 30;
            //			m_cImgAnalysis.RIGHT=(x-ENLARGESIZE_ZOOM_W)* CIS_IMG_W / ZOOM_W + 16;
            if (m_cImgAnalysis.RIGHT > CIS_IMG_W - 2)
            {
                m_cImgAnalysis.RIGHT = CIS_IMG_W - 2;
            }
            break;
        }
    }

    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        COUNT = 0;
        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            //			if(m_segment_enlarge_buffer[y*ZOOM_ENLAGE_W+x]==255)
            if (EnlargeBuf[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            m_cImgAnalysis.TOP = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W - 30;
            //			m_cImgAnalysis.TOP=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H - 16;
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
            //			if(m_segment_enlarge_buffer[y*ZOOM_ENLAGE_W+x]==255)
            if (EnlargeBuf[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            m_cImgAnalysis.BOTTOM = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W + 30;
            //			m_cImgAnalysis.BOTTOM=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H + 16;
            if (m_cImgAnalysis.BOTTOM > CIS_IMG_H - 2)
            {
                m_cImgAnalysis.BOTTOM = CIS_IMG_H - 2;
            }
            break;
        }
    }

    return BrightValue;
}

void CIBAlgorithm::_Algo_Remove_Noise_using_remove_mask(BYTE *InputImage, BYTE *OutImage, int imgWidth, int imgHeight)
{
    int i, j, val;

    memset(OutImage, 0, imgWidth * imgHeight);
    memset(mean_minus, 0, CIS_IMG_W * sizeof(int));
    memset(mean_cnt, 0, CIS_IMG_W * sizeof(short));

    for (i = 0; i < imgWidth * imgHeight; i++)
    {
        val = InputImage[i] - m_TFT_MaskImg[i] - 5;
        m_tmpNoiseBuf[i] = val;
    }

    for (i = 0; i < imgHeight; i++)
    {
        for (j = 0; j < imgWidth; j++)
        {
            if (m_tmpNoiseBuf[i * imgWidth + j] < 0)
            {
                mean_minus[j] += m_tmpNoiseBuf[i * imgWidth + j];
                mean_cnt[j]++;
            }
        }
    }

    for (j = 0; j < imgWidth; j++)
    {
        if (mean_cnt[j] > 0)
        {
            mean_minus[j] = mean_minus[j] / (mean_cnt[j] * 2);
        }
    }

    for (i = 0; i < imgHeight; i++)
    {
        for (j = 0; j < imgWidth; j++)
        {
            if (m_tmpNoiseBuf[i * imgWidth + j] > 0)
            {
                val = m_tmpNoiseBuf[i * imgWidth + j] - mean_minus[j];
                if (val > 255)
                {
                    val = 255;
                }
                OutImage[i * imgWidth + j] = val;
            }
        }
    }
}

void CIBAlgorithm::_Algo_Remove_Noise_using_remove_mask_asic(BYTE *InputImage, BYTE *OutImage, int imgWidth, int imgHeight)
{
    // 2013-04-03 Gon modify : To remove TFT background noise.
    int i, j, val;
    int noise_histo = m_cImgAnalysis.noise_histo;
	int imgSize = imgWidth*imgHeight;
	int maskVal[8]={0};

/*	FILE *fp = fopen("e:\\InputImage.raw", "wb");
	fwrite(InputImage, 1, 750*800, fp);
	fclose(fp);
*/
    memset(OutImage, 0, imgWidth*imgHeight);

/*    for (i = 0; i < imgHeight; i++)
    {
		for (j = 0; j < imgWidth-8; j++)
		{
			// Unrolling
			val = InputImage[(imgWidth-1-j)*imgHeight+i] - m_TFT_MaskImg[(imgWidth-1-j)*imgHeight+i] - noise_histo;
			if (val > 0)
			{
				OutImage[i*imgWidth+j] = val;
			}
		}
    }
*/
	for (i = 0; i < imgSize; i+=8)
    {
		// Unrolling
		maskVal[0] = m_TFT_MaskImg[i];
		maskVal[1] = m_TFT_MaskImg[i+1];
		maskVal[2] = m_TFT_MaskImg[i+2];
		maskVal[3] = m_TFT_MaskImg[i+3];
		maskVal[4] = m_TFT_MaskImg[i+4];
		maskVal[5] = m_TFT_MaskImg[i+5];
		maskVal[6] = m_TFT_MaskImg[i+6];
		maskVal[7] = m_TFT_MaskImg[i+7];
		val = InputImage[i] - maskVal[0] - noise_histo;
		if (val < 0)
			val = 0;
		InputImage[i] = val;

		val = InputImage[i+1] - maskVal[1] - noise_histo;
		if (val < 0)
			val = 0;
		InputImage[i+1] = val;

		val = InputImage[i+2] - maskVal[2] - noise_histo;
		if (val < 0)
			val = 0;
		InputImage[i+2] = val;

		val = InputImage[i+3] - maskVal[3] - noise_histo;
		if (val < 0)
			val = 0;
		InputImage[i+3] = val;

		val = InputImage[i+4] - maskVal[4] - noise_histo;
		if (val < 0)
			val = 0;
		InputImage[i+4] = val;

		val = InputImage[i+5] - maskVal[5] - noise_histo;
		if (val < 0)
			val = 0;
		InputImage[i+5] = val;

		val = InputImage[i+6] - maskVal[6] - noise_histo;
		if (val < 0)
			val = 0;
		InputImage[i+6] = val;

		val = InputImage[i+7] - maskVal[7] - noise_histo;
		if (val < 0)
			val = 0;
		InputImage[i+7] = val;
    }

	for (i = 0; i < imgHeight; i++)
    {
		for (j = 0; j < imgWidth-8; j++)
		{
			// Unrolling
			val = InputImage[(imgWidth-1-j)*imgHeight+i];
//			if (val > 0)
			{
				OutImage[i*imgWidth+j] = val;
			}
		}
    }
/*
	fp = fopen("e:\\OutImage.raw", "wb");
	fwrite(OutImage, 1, 750*800, fp);
	fclose(fp);
*/
/*    memset(OutImage, 0, imgSize);
    i = imgSize - 1;
    do
    {
        // Use the unrolling method for the code optionization in ARM
        val = InputImage[i] - m_TFT_MaskImg[i] - noise_histo;
        if (val > 0)
        {
            OutImage[i] = val;
        }
        i--;

        val = InputImage[i] - m_TFT_MaskImg[i] - noise_histo;
        if (val > 0)
        {
            OutImage[i] = val;
        }
        i--;

        val = InputImage[i] - m_TFT_MaskImg[i] - noise_histo;
        if (val > 0)
        {
            OutImage[i] = val;
        }
        i--;

        val = InputImage[i] - m_TFT_MaskImg[i] - noise_histo;
        if (val > 0)
        {
            OutImage[i] = val;
        }
        i--;
        val = InputImage[i] - m_TFT_MaskImg[i] - noise_histo;
        if (val > 0)
        {
            OutImage[i] = val;
        }
        i--;

        val = InputImage[i] - m_TFT_MaskImg[i] - noise_histo;
        if (val > 0)
        {
            OutImage[i] = val;
        }
        i--;

        val = InputImage[i] - m_TFT_MaskImg[i] - noise_histo;
        if (val > 0)
        {
            OutImage[i] = val;
        }
        i--;

        val = InputImage[i] - m_TFT_MaskImg[i] - noise_histo;
        if (val > 0)
        {
            OutImage[i] = val;
        }
        i--;
    } while (i != -1);
*/
}

void CIBAlgorithm::_Algo_Remove_Dot_Noise_asic(BYTE *InputImage, BYTE *OutImage, int imgWidth, int imgHeight)
{
    int i, j, ii, jj, s, t;
    int low_count = 0;
    unsigned short ref_coord_x[2048];

    for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
    {
        ref_coord_x[j] = (j - ENLARGESIZE_ZOOM_W) * imgWidth / ZOOM_W + 2;
    }

    memcpy(OutImage, InputImage, imgWidth * imgHeight);

    for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
    {
        ii = (i - ENLARGESIZE_ZOOM_H) * imgHeight / ZOOM_H + 2;

        for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
        {
            if (m_labeled_segment_enlarge_buffer[i * ZOOM_ENLAGE_W + j] > 0)
            {
                continue;
            }

            jj = ref_coord_x[j];

            for (s = ii; s < ii + ZOOM_OUT; s++)
            {
                for (t = jj; t < jj + ZOOM_OUT; t++)
                {
                    if (InputImage[s * imgWidth + t] == 0)
                    {
                        continue;
                    }

                    low_count = 0;
                    if (InputImage[(s - 1)*imgWidth + (t - 1)] == 0)
                    {
                        low_count++;
                    }
                    if (InputImage[(s - 1)*imgWidth + t] == 0)
                    {
                        low_count++;
                    }
                    if (InputImage[(s - 1)*imgWidth + (t + 1)] == 0)
                    {
                        low_count++;
                    }
                    if (InputImage[s * imgWidth + (t - 1)] == 0)
                    {
                        low_count++;
                    }
                    if (InputImage[s * imgWidth + (t + 1)] == 0)
                    {
                        low_count++;
                    }
                    if (InputImage[(s + 1)*imgWidth + (t - 1)] == 0)
                    {
                        low_count++;
                    }
                    if (InputImage[(s + 1)*imgWidth + t] == 0)
                    {
                        low_count++;
                    }
                    if (InputImage[(s + 1)*imgWidth + (t + 1)] == 0)
                    {
                        low_count++;
                    }

                    if (low_count > 4)
                    {
                        OutImage[s * imgWidth + t] = 0;
                    }
                }
            }
        }
    }
}

void CIBAlgorithm::_Algo_Remove_Vertical_Noise(BYTE *InputImage, BYTE *OutImage, int imgWidth, int imgHeight)
{
    int i, j, s, t, val, min_noise_val;
    int noise_val[5], posy[5] = {1, 30, imgHeight / 2, imgHeight - 32, imgHeight - 2};

    for (j = 0; j < imgWidth; j++)
    {
        for (s = 0; s < 5; s++)
        {
            noise_val[s] = 0;

            for (i = posy[s] - 1; i <= posy[s] + 1; i++)
            {
                if (noise_val[s] < InputImage[i * imgWidth + j])
                {
                    noise_val[s] = InputImage[i * imgWidth + j];
                }
            }
        }

        for (s = 0; s < 2; s++)
        {
            for (t = s + 1; t < 5; t++)
            {
                if (noise_val[s] > noise_val[t])
                {
                    val = noise_val[s];
                    noise_val[s] = noise_val[t];
                    noise_val[t] = val;
                }
            }
        }

        min_noise_val = noise_val[1]; // 4th value to avoid dead pixel

        for (i = 0; i < imgHeight; i++)
        {
            val = InputImage[i * imgWidth + j] - min_noise_val - 5;
            if (val < 0)
            {
                val = 0;
            }
            else if (val > 255)
            {
                val = 255;
            }

            OutImage[i * imgWidth + j] = val;
        }
    }
}

void CIBAlgorithm::_Algo_ROIC_RemoveVignettingNoise(unsigned char *InImg, unsigned char *OutImg)
{
    int i, j;
    int center, outsum;
    int BrightThres = 30;
    int MeanBright = 0;

    center = 0;
    for (i = m_cImgAnalysis.TOP; i <= m_cImgAnalysis.BOTTOM; i++)
    {
        for (j = m_cImgAnalysis.LEFT; j <= m_cImgAnalysis.RIGHT; j++)
        {
            if (InImg[i * CIS_IMG_W + j] > 5)
            {
                MeanBright += InImg[i * CIS_IMG_W + j];
                center++;
            }
        }
    }

    if (center > 0)
    {
        MeanBright = MeanBright / center;
    }
    else
    {
        memcpy(OutImg, InImg, CIS_IMG_H * CIS_IMG_W);
        return;
    }

    BrightThres = BrightThres * MeanBright / 128;
    if (BrightThres > 60)
    {
        BrightThres = 60;
    }
    else if (BrightThres < 30)
    {
        BrightThres = 30;
    }

    memcpy(OutImg, InImg, CIS_IMG_H * CIS_IMG_W);
    for (i = m_cImgAnalysis.TOP; i <= m_cImgAnalysis.BOTTOM; i++)
    {
        for (j = m_cImgAnalysis.LEFT; j <= m_cImgAnalysis.RIGHT; j++)
        {
            center = InImg[i * CIS_IMG_W + j] * 3;
            outsum = (InImg[(i - 1) * CIS_IMG_W + (j - 1)] + InImg[(i - 1) * CIS_IMG_W + (j)] + InImg[(i - 1) * CIS_IMG_W + (j + 1)] +
                      InImg[(i) * CIS_IMG_W + (j - 1)] + InImg[(i) * CIS_IMG_W + (j + 1)] +
                      InImg[(i + 1) * CIS_IMG_W + (j - 1)] + InImg[(i + 1) * CIS_IMG_W + (j)] + InImg[(i + 1) * CIS_IMG_W + (j + 1)]) >> 3;

            if (center < outsum)
            {
                OutImg[i * CIS_IMG_W + j] = outsum;
            }
            else if ((765 - center) < (255 - outsum))
            {
                OutImg[i * CIS_IMG_W + j] = outsum;
            }
        }
    }
}
