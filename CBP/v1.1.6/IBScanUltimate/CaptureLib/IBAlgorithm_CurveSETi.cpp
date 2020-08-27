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

/* to know the location of the code which is occured memory leak */
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Curve Algorithm
void CIBAlgorithm::_Algo_CurveSETi_AnalysisImage(unsigned char *InImg, GoodCaptureInfo *capinfo)
{
	int x,y;
	int value, real_count, y_index;
	int current_count;
	int noise_histo[4], noise_count[4];
    int size_h = 8, size_w = 8;

	//Initialization
	capinfo->prev_histo = capinfo->current_histo;
	capinfo->prev_lessTHAN2 = capinfo->lessTHAN2;
	capinfo->prev_lessTHAN3 = capinfo->lessTHAN3;
	capinfo->prev_lessTHAN4 = capinfo->lessTHAN4;
	capinfo->noise_histo=0;
	capinfo->noise_lessTHAN2=0;
	capinfo->noise_lessTHAN3=0;
	capinfo->noise_lessTHAN4=0;
	capinfo->current_histo=0;
	capinfo->lessTHAN2=0;
	capinfo->lessTHAN3=0;
	capinfo->lessTHAN4=0;
	capinfo->real_histo=0;

	//Image Information Analysis

    //Image Information Analysis
    // Noise Histo
    memset(noise_histo, 0, sizeof(int) * 4);
    memset(noise_count, 0, sizeof(int) * 4);

    for (y = 0; y < size_h; y++)
    {
        y_index = y * CIS_IMG_W;
        for (x = 0; x < size_w; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[0] < value && value < 60)
            {
                noise_histo[0] = value;
            }
        }
        for (x = CIS_IMG_W - size_w; x < CIS_IMG_W; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[1] < value && value < 60)
            {
                noise_histo[1] = value;
            }
        }
    }
    for (y = CIS_IMG_H - size_h; y < CIS_IMG_H; y++)
    {
        y_index = y * CIS_IMG_W;
        for (x = 0; x < size_w; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[2] < value && value < 60)
            {
                noise_histo[2] = value;
            }
        }
        for (x = CIS_IMG_W - size_w; x < CIS_IMG_W; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[3] < value && value < 60)
            {
                noise_histo[3] = value;
            }
        }
    }

    int min_noise_histo = noise_histo[0], min_noise_idx = 0;
    for (y = 1; y < 4; y++)
    {
        if (min_noise_histo > noise_histo[y])
        {
            min_noise_histo = noise_histo[y];
            min_noise_idx = y;
        }
    }

	real_count=0;
	current_count=0;
	for (y=0; y<CIS_IMG_H; y+=2)
	{
		for(x=0; x<CIS_IMG_W; x+=2)
		{	
			value=InImg[y*CIS_IMG_W+x];
			if(value > 0)
			{
				capinfo->current_histo+=value;
				current_count++;
			}

			if(value > 10)
			{
				capinfo->real_histo+=value;
				real_count++;
			}

			if(value<LESS_THAN1)
				continue;
			else if(value<LESS_THAN2)
				capinfo->lessTHAN2+=4;
			else if(value<LESS_THAN3)
				capinfo->lessTHAN3+=4;
			else if(value<LESS_THAN4)
				capinfo->lessTHAN4+=4;
		}
	}
	
	if(current_count>0)
		capinfo->current_histo/=current_count;
	else
		capinfo->current_histo=0;

	if(real_count>0)
		capinfo->real_histo/=real_count;
	else
		capinfo->real_histo=0;

	capinfo->noise_histo = noise_histo[min_noise_idx];
    m_cImgAnalysis.noise_histo = noise_histo[min_noise_idx];			// mean of noise image
}

void CIBAlgorithm::_Algo_CurveSETi_AnalysisImage_for_Dummy(unsigned char *InImg, GoodCaptureInfo *capinfo)
{
	int x,y;
	int value, real_count, y_index;
	int current_count;
	int noise_histo[4], noise_count[4];
    int size_h = 8, size_w = 8;

	//Initialization
	capinfo->prev_histo = capinfo->current_histo;
	capinfo->prev_lessTHAN2 = capinfo->lessTHAN2;
	capinfo->prev_lessTHAN3 = capinfo->lessTHAN3;
	capinfo->prev_lessTHAN4 = capinfo->lessTHAN4;
	capinfo->noise_histo=0;
	capinfo->noise_lessTHAN2=0;
	capinfo->noise_lessTHAN3=0;
	capinfo->noise_lessTHAN4=0;
	capinfo->current_histo=0;
	capinfo->lessTHAN2=0;
	capinfo->lessTHAN3=0;
	capinfo->lessTHAN4=0;
	capinfo->real_histo=0;

	//Image Information Analysis

    //Image Information Analysis
    // Noise Histo
    memset(noise_histo, 0, sizeof(int) * 4);
    memset(noise_count, 0, sizeof(int) * 4);

    for (y = 0; y < size_h; y++)
    {
        y_index = y * CIS_IMG_W;
        for (x = 0; x < size_w; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[0] < value && value < 60)
            {
                noise_histo[0] = value;
            }
        }
        for (x = CIS_IMG_W - size_w; x < CIS_IMG_W; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[1] < value && value < 60)
            {
                noise_histo[1] = value;
            }
        }
    }
    for (y = CIS_IMG_H - size_h; y < CIS_IMG_H; y++)
    {
        y_index = y * CIS_IMG_W;
        for (x = 0; x < size_w; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[2] < value && value < 60)
            {
                noise_histo[2] = value;
            }
        }
        for (x = CIS_IMG_W - size_w; x < CIS_IMG_W; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[3] < value && value < 60)
            {
                noise_histo[3] = value;
            }
        }
    }

    int min_noise_histo = noise_histo[0], min_noise_idx = 0;
    for (y = 1; y < 4; y++)
    {
        if (min_noise_histo > noise_histo[y])
        {
            min_noise_histo = noise_histo[y];
            min_noise_idx = y;
        }
    }

	real_count=0;
	current_count=0;
	for (y=0; y<CIS_IMG_H; y+=2)
	{
		for(x=0; x<CIS_IMG_W; x+=2)
		{	
			value=InImg[y*CIS_IMG_W+x];
			if(value > 0)
			{
				capinfo->current_histo+=value;
				current_count++;
			}

			if(value > 10)
			{
				capinfo->real_histo+=value;
				real_count++;
			}

			if(value<LESS_THAN1)
				continue;
			else if(value<LESS_THAN2)
				capinfo->lessTHAN2+=4;
			else if(value<LESS_THAN3)
				capinfo->lessTHAN3+=4;
			else if(value<LESS_THAN4)
				capinfo->lessTHAN4+=4;
		}
	}
	
	if(current_count>0)
		capinfo->current_histo/=current_count;
	else
		capinfo->current_histo=0;

	if(real_count>0)
		capinfo->real_histo/=real_count;
	else
		capinfo->real_histo=0;

	capinfo->noise_histo = noise_histo[min_noise_idx];
    m_cImgAnalysis.noise_histo = noise_histo[min_noise_idx];			// mean of noise image
}

void CIBAlgorithm::_Algo_CurveSETi_CuttingDumyArea_with_Noise(unsigned char *InImg, unsigned char *OutImg, unsigned char *TmpImg, int ImgSize, int ContrastLevel, int *center_x, int *center_y)
{
	int IMG_WIDTH_USB = CIS_IMG_W;
	int IMG_HEIGHT_USB = CIS_IMG_H;

	int x, y, val;
    int minus_value = m_CaptureInfo.real_histo;
    int sum_x = 0, sum_y = 0, count = 0;

	for (y=0; y<IMG_HEIGHT_USB; y++)
	{
		for(x=0; x<IMG_WIDTH_USB; x++)
		{	
            val = (int)InImg[y*IMG_WIDTH_USB + x];
//            OutImg[(y)*IMG_WIDTH_USB + (x)] = val; //Contrast_LUT[InImg[i]];
            val = val - minus_value;
            if (val > 0)
            {
                count++;
                sum_x += x;
                sum_y += y;
            }
        }
    }

    *center_x = 0;
    *center_y = 0;
    if (count > 0)
    {
        *center_x = sum_x / count;
        *center_y = sum_y / count;
    }
}

int CIBAlgorithm::_Algo_CurveSETi_GetFingerCount(unsigned char *InImg, int ImageBright)
{
	if (m_CaptureInfo.real_histo > 10 &&
		m_CaptureInfo.current_histo - m_CaptureInfo.noise_histo > 10 &&
		(m_CaptureInfo.lessTHAN2+m_CaptureInfo.lessTHAN3+m_CaptureInfo.lessTHAN4) > 500)
	{
			return 1;
	}

	return 0;
}

int CIBAlgorithm::_Algo_CurveSETi_GoodCapture(GoodCaptureInfo *capinfo, PropertyInfo *propertyInfo, BOOL *SetRegisterForDry)
{
	const int	Dry_Wet_Tres = 30;
	const int	HISTO_THRESHOLD = 100;
	const int	REAL_HISTO_THRESHOLD = 120;
	int			diff_less3;
//	int			minus_gain = 0;
//	int			val;

    *SetRegisterForDry = FALSE;

	capinfo->tot_frame++;

	//Fingerprint Detection Conditions
	if (capinfo->real_histo > 10 &&
		capinfo->current_histo - capinfo->noise_histo > 10 &&
		(capinfo->lessTHAN2+capinfo->lessTHAN3+capinfo->lessTHAN4) > 500)
	{
		capinfo->skip_count++;
		if (capinfo->skip_count < 2)
			return 1;

		capinfo->detected_count++;

		if (propertyInfo->nContrastValue>0 &&
			(capinfo->current_histo>HISTO_THRESHOLD || capinfo->real_histo>REAL_HISTO_THRESHOLD))
		{
#ifdef __G_DEBUG__
			TRACE(_T("-> Control Gain #1 : current_histo(%d), real_histo(%d)\n"), capinfo->current_histo, capinfo->real_histo);
#endif
			propertyInfo->nContrastValue -= (propertyInfo->nContrastValue*2/5 + 1);
			if (propertyInfo->nContrastValue < 0)
				propertyInfo->nContrastValue = 0;
		}
		else if (capinfo->current_histo<=HISTO_THRESHOLD)
		{
			diff_less3 = abs((capinfo->lessTHAN3+capinfo->lessTHAN4)-(capinfo->prev_lessTHAN3+capinfo->prev_lessTHAN4));

			if ((capinfo->current_histo-capinfo->noise_histo)>Dry_Wet_Tres)		//Normal Fingerprint Condition
			{
				if ((diff_less3>=0 && diff_less3<(capinfo->lessTHAN3+capinfo->lessTHAN4)/5) ||
					(diff_less3<0 && -diff_less3<(capinfo->lessTHAN3+capinfo->lessTHAN4)/5))
					capinfo->condition_stay++;
#ifdef __G_DEBUG__
				TRACE(_T("capinfo->current_histo-capinfo->noise_histo : %d\n"), (capinfo->current_histo-capinfo->noise_histo));
#endif

				if (capinfo->condition_stay>1 && (capinfo->lessTHAN3+capinfo->lessTHAN4)>15000)						//Normal stable End Condition
				{
					return 2;
				}
				else if ((capinfo->lessTHAN3+capinfo->lessTHAN4)>35000 && capinfo->noise_histo<Dry_Wet_Tres)		//Normal good End Condition 2
				{
					return 2;
				}
				else if (capinfo->detected_count>1 && (capinfo->lessTHAN3+capinfo->lessTHAN4)>25000 && capinfo->noise_histo<10)		//Normal good End Condition 3
				{
					return 2;
				}
			}
			else if ((capinfo->current_histo-capinfo->noise_histo)<=Dry_Wet_Tres)		//Dry
			{
#ifdef __G_DEBUG__
 				TRACE(_T("-> Detect Dry\n"));
#endif
				if (capinfo->dry_count>2 && propertyInfo->nIntegrationValue < __CURVE_SETI_MAX_EXPOSURE_VALUE)
				{
					propertyInfo->nIntegrationValue = __CURVE_SETI_MAX_EXPOSURE_VALUE;
					*SetRegisterForDry = TRUE;
					capinfo->STOP_DETECT_COUNT=11;
				}
				capinfo->dry_count++;
			}
		}
		if (capinfo->detected_count>=capinfo->STOP_DETECT_COUNT)		//Last image from Fingerprint detection when it is dry.
		{
			if ((capinfo->noise_histo>=capinfo->current_histo || capinfo->current_histo<5 ||
				(capinfo->current_histo-capinfo->noise_histo)<2 ||
				(capinfo->noise_lessTHAN3>capinfo->lessTHAN3 && capinfo->noise_lessTHAN4>capinfo->lessTHAN4)))		//If the finger is removing...
			{
#ifdef __G_DEBUG__
				TRACE(_T("-> STOP_DETECT_COUNT >> noise_histo(%d), current_histo(%d), noise_lessTHAN3(%d), noise_lessTHAN4(%d), lessTHAN3(%d), lessTHAN4(%d)\n"),
					capinfo->noise_histo, capinfo->current_histo, capinfo->noise_lessTHAN3, capinfo->noise_lessTHAN4, capinfo->lessTHAN3, capinfo->lessTHAN4);
#endif
				capinfo->detected_count = capinfo->STOP_DETECT_COUNT-2;
				return 1;				//Go to the first
			}

			if (capinfo->detected_count > 3 && capinfo->real_histo < REAL_HISTO_THRESHOLD)
			{
				if (capinfo->dry_count > 0 && (capinfo->lessTHAN2+capinfo->lessTHAN3+capinfo->lessTHAN4) > 7500 && propertyInfo->nIntegrationValue == __CURVE_SETI_MAX_EXPOSURE_VALUE)
				{
					return 2;
				}
				else if ((capinfo->lessTHAN3+capinfo->lessTHAN4) > 12000 || (capinfo->lessTHAN2+capinfo->lessTHAN3+capinfo->lessTHAN4) > 20000)
				{
					return 2;
				}
			}

			if (capinfo->real_histo > REAL_HISTO_THRESHOLD && propertyInfo->nContrastValue == 0)
            {
				if (capinfo->same_gain_count++ > 1)
				{
					return 2;
				}
            }
		}

		capinfo->prev_lessTHAN3 = capinfo->lessTHAN3;
		capinfo->prev_lessTHAN4 = capinfo->lessTHAN4;
		capinfo->prev_histo = capinfo->current_histo;

		return 1;				//Go to the first
	}
	else
	{
		capinfo->dry_count=0;
		capinfo->STOP_DETECT_COUNT=4;
		capinfo->condition_stay=0;
		capinfo->detected_count=0;
		capinfo->skip_count = 0;
		capinfo->same_gain_count = 0;
		propertyInfo->nIntegrationValue = __CURVE_SETI_DEFAULT_EXPOSURE_VALUE;
		propertyInfo->nContrastValue = __DEFAULT_CURVE_CONTRAST_VALUE__;
		capinfo->prev_lessTHAN3=INIT_PREV;
		capinfo->prev_lessTHAN4=INIT_PREV;
		capinfo->prev_histo=INIT_PREV;

		if (capinfo->noise_histo > HISTO_THRESHOLD && propertyInfo->nContrastValue > 0)
		{
			capinfo->NOISE_DETECT=1;
            propertyInfo->nContrastValue -= (propertyInfo->nContrastValue/3);
			if (propertyInfo->nContrastValue < 0)
				propertyInfo->nContrastValue = 0;
		}

		return 0;
	}

	return -1;
}

void CIBAlgorithm::_Algo_CurveSETi_Init_Distortion_Merge_with_Biliner()
{
	float temp_i;

	int ACTIVE_IMG_W = CIS_IMG_W;
	int ACTIVE_IMG_H = CIS_IMG_H;
	int cutWidth = (CIS_IMG_W-ACTIVE_IMG_W)/2;
	int cutHeight = (CIS_IMG_H-ACTIVE_IMG_H)/2;

	for(int i=0; i<IMG_H; i++ )
	{
		temp_i = (float)i*ACTIVE_IMG_H/IMG_H+cutHeight;
		m_Curve_p_arr[i] = (short)((temp_i - (short)temp_i) * 128);
		m_Curve_refy1_arr[i] = (int)temp_i;
		m_Curve_refy2_arr[i] = (int)temp_i+1;
		if(m_Curve_refy1_arr[i]>(CIS_IMG_H-1))	m_Curve_refy1_arr[i]=(CIS_IMG_H-1);
		if(m_Curve_refy2_arr[i]>(CIS_IMG_H-1))	m_Curve_refy2_arr[i]=(CIS_IMG_H-1);
		m_Curve_refy1_arr[i] = m_Curve_refy1_arr[i]*(CIS_IMG_W);
		m_Curve_refy2_arr[i] = m_Curve_refy2_arr[i]*(CIS_IMG_W);
	}

	for(int i=0; i<IMG_W; i++ )
	{
		temp_i = (float)i*ACTIVE_IMG_W/IMG_W+cutWidth;
		m_Curve_q_arr[i] = (short)((temp_i - (short)temp_i) * 128);
		m_Curve_refx1_arr[i] = (int)temp_i;
		m_Curve_refx2_arr[i] = (int)temp_i+1;
		if(m_Curve_refx1_arr[i]>(CIS_IMG_W-1))	m_Curve_refx1_arr[i]=(CIS_IMG_W-1);
		if(m_Curve_refx2_arr[i]>(CIS_IMG_W-1))	m_Curve_refx2_arr[i]=(CIS_IMG_W-1);
	}
}

void CIBAlgorithm::_Algo_CurveSETi_DistortionRestoration_with_Bilinear(unsigned char *InImg, unsigned char *OutImg)
{
	int i, j, p, q, refy1, refy2, refx1, refx2;
	int Value;

	for(i=0; i<IMG_H; i++)
	{
		refy1 = m_Curve_refy1_arr[i];
		refy2 = m_Curve_refy2_arr[i];
		p = m_Curve_p_arr[i];

		for (j=0; j<IMG_W; j++)
		{
			refx1 = m_Curve_refx1_arr[j];
			refx2 = m_Curve_refx2_arr[j];
			q = m_Curve_q_arr[j];

			Value = ( (128-p) * ( (128-q)*InImg[refy1+refx1] + q*InImg[refy1+refx2] ) +
						p * ( (128-q)*InImg[refy2+refx1] + q*InImg[refy2+refx2] ) ) >> 14;

			OutImg[i*IMG_W+j] = (unsigned char)Value;
		}
	}
}

void CIBAlgorithm::_Algo_CurveSETi_JudgeFingerQuality(FrameImgAnalysis *pFrameImgAnalysis, BOOL bIsGoodImage, PropertyInfo *pPropertyInfo, CallbackProperty *pClbkProperty)
{
	int IMG_GAP_W = 30;

	const int MIN_RANGE = 35;
    const int MAX_RANGE = 50;

    memset(&pClbkProperty->qualityArray[0], ENUM_IBSU_FINGER_NOT_PRESENT, sizeof(pClbkProperty->qualityArray));

    // Update only the finger quality nearest to the single finger.
    int pos = pFrameImgAnalysis->center_x * 4 / (m_pUsbDevInfo->CisImgWidth - IMG_GAP_W - 30);

    if (m_cImgAnalysis.finger_count > 0)
    {
        // Judge the quality of flat finger
        if (bIsGoodImage)
        {
            // Quality good
            pClbkProperty->qualityArray[pos] = ENUM_IBSU_QUALITY_GOOD;
        }
        else if ((m_cImgAnalysis.mean >= (__CAPTURE_MIN_BRIGHT__ - MIN_RANGE)) &&
                 (m_cImgAnalysis.mean <= (__CAPTURE_MAX_BRIGHT__ + MAX_RANGE)))
        {
            // Quality fair
            pClbkProperty->qualityArray[pos] = ENUM_IBSU_QUALITY_FAIR;
        }
        else
        {
            // Quality poor
            pClbkProperty->qualityArray[pos] = ENUM_IBSU_QUALITY_POOR;
        }
    }
}
