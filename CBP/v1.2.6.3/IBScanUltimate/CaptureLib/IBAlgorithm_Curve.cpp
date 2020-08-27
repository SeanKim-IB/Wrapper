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
#include "VignettingTable_Curve.lut"

/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Curve Algorithm


void CIBAlgorithm::_Algo_Curve_AnalysisImage(unsigned char *InImg, GoodCaptureInfo *capinfo)
{
    int x, y;
    int value, y_index, real_count;
    int IMG_WIDTH_USB = CIS_IMG_H + UPPER_H;
    int IMG_HEIGHT_USB = CIS_IMG_W + SIDE_W;
    int mean_noise = 0, mean_noise_count = 0;


    //Initialization
    capinfo->noise_histo = 0;
    capinfo->noise_lessTHAN2 = 0;
    capinfo->noise_lessTHAN3 = 0;
    capinfo->noise_lessTHAN4 = 0;
    capinfo->current_histo = 0;
    capinfo->lessTHAN2 = 0;
    capinfo->lessTHAN3 = 0;
    capinfo->lessTHAN4 = 0;
    capinfo->real_histo = 0;

    //Image Information Analysis
    real_count = 0;
    for (y = SIDE_W; y < IMG_HEIGHT_USB; y += 2)
    {
        y_index = y * IMG_WIDTH_USB;
        for (x = 0; x < CUT_NOISE; x++)
        {
            value = InImg[y_index + x];
            capinfo->noise_histo += value;

            //			if( value > 10 )
            {
                mean_noise += value;
                mean_noise_count++;
            }

            if (value < LESS_THAN1)
            {
                continue;
            }
            else if (value < LESS_THAN2)
            {
                capinfo->noise_lessTHAN2 += 16;
            }
            else if (value < LESS_THAN3)
            {
                capinfo->noise_lessTHAN3 += 16;
            }
            else if (value < LESS_THAN4)
            {
                capinfo->noise_lessTHAN4 += 16;
            }
        }
        for (x = UPPER_H; x < IMG_WIDTH_USB; x += 2)
        {
            value = InImg[y_index + x];
            capinfo->current_histo += (value << 2);
            if (InImg[y_index + x] > 10)
            {
                capinfo->real_histo += (value << 2);
                real_count += 4;
            }
            if (value < LESS_THAN1)
            {
                continue;
            }
            else if (value < LESS_THAN2)
            {
                capinfo->lessTHAN2 += 4;
            }
            else if (value < LESS_THAN3)
            {
                capinfo->lessTHAN3 += 4;
            }
            else if (value < LESS_THAN4)
            {
                capinfo->lessTHAN4 += 4;
            }
        }
    }
    if (mean_noise_count > 0)
    {
        capinfo->noise_histo /= mean_noise_count;    //((CIS_IMG_W/*360*/>>1)*CUT_NOISE);
    }

    capinfo->current_histo /= (CIS_IMG_W/*360*/*CIS_IMG_H/*440*/);
    if (real_count > 0)
    {
        capinfo->real_histo /= real_count;    //(RAW_IMG_HEIGHT*RAW_IMG_WIDTH);
    }


    if (mean_noise_count > 0)
    {
        mean_noise /= mean_noise_count;
    }

    m_cImgAnalysis.noise_histo = mean_noise; //capinfo->noise_histo+4;			// mean of noise image
}

void CIBAlgorithm::_Algo_Curve_CuttingDumyArea_with_Noise(unsigned char *InImg, unsigned char *OutImg, unsigned char *TmpImg, int ImgSize, int ContrastLevel, int *center_x, int *center_y)
{
    int x, y, val;
    int minus_value = m_CaptureInfo.real_histo;
    int sum_x = 0, sum_y = 0, count = 0;

    for (y = 0; y < (CIS_IMG_H + 0); y++)
    {
        for (x = 0; x < (CIS_IMG_W + 0); x++)
        {
            val = (int)InImg[y * (CIS_IMG_W + SIDE_W) + x];
            OutImg[(y)*CIS_IMG_W + (x)] = val; //Contrast_LUT[InImg[i]];
            val = val - minus_value;
            if (val > 0)
            {
                count++;
                sum_x += (x - SIDE_W);
                sum_y += (y - UPPER_H);
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

int CIBAlgorithm::_Algo_Curve_GoodCapture(GoodCaptureInfo *capinfo, PropertyInfo *propertyInfo, BOOL *SetRegisterForDry)
{
    const int	Dry_Wet_Tres = 20;
    const int	HISTO_THRESHOLD = 120;
    const int	REAL_HISTO_THRESHOLD = 150;
    int			diff_less3;
    int			minus_gain;
    int         tmp_calculated_gain;


    *SetRegisterForDry = FALSE;

    capinfo->tot_frame++;

    //Fingerprint Detection Conditions
    if ((capinfo->current_histo + capinfo->current_histo / 3 > capinfo->noise_histo * 2  && capinfo->current_histo > 4) ||
            (capinfo->noise_histo > 80 && capinfo->current_histo - capinfo->noise_histo > 10) ||
            (capinfo->noise_lessTHAN3 < 20 && capinfo->noise_lessTHAN3 * 2 < capinfo->lessTHAN3 && capinfo->noise_lessTHAN4 * 3 < capinfo->lessTHAN4
             && capinfo->noise_lessTHAN2 + capinfo->noise_lessTHAN2 / 8 < capinfo->lessTHAN2) ||
            (capinfo->noise_lessTHAN3 >= 20 && capinfo->noise_lessTHAN3 * 2 < capinfo->lessTHAN3 && capinfo->noise_lessTHAN4 * 3 < capinfo->lessTHAN4) ||
            (capinfo->lessTHAN4 > 1600 && capinfo->noise_lessTHAN4 * 2 < capinfo->lessTHAN4)
       )
    {
        capinfo->detected_count++;

        if (propertyInfo->nContrastValue > 2/*30*/ && (capinfo->noise_histo > HISTO_THRESHOLD || capinfo->current_histo > HISTO_THRESHOLD || capinfo->real_histo > REAL_HISTO_THRESHOLD))
        {
            //			minus_gain = propertyInfo->nContrastValue/3;
            tmp_calculated_gain = propertyInfo->nContrastValue * 7 + 17;
            minus_gain = tmp_calculated_gain / 3;

            if (capinfo->current_histo > HISTO_THRESHOLD)
            {
                minus_gain = minus_gain + (capinfo->current_histo - HISTO_THRESHOLD) * 3;
            }
            else if (capinfo->real_histo > HISTO_THRESHOLD)
            {
                minus_gain = minus_gain + (capinfo->real_histo - REAL_HISTO_THRESHOLD) * 3;
            }

            //			if( minus_gain > (propertyInfo->nContrastValue/2) )
            //				minus_gain = propertyInfo->nContrastValue/2;
            if (minus_gain > (tmp_calculated_gain / 2))
            {
                minus_gain = tmp_calculated_gain / 2;
            }
            //			propertyInfo->nContrastValue -= minus_gain;
            propertyInfo->nContrastValue -= (minus_gain - 17) / 7;
            if (propertyInfo->nContrastValue < 2/*30*/)
            {
                propertyInfo->nContrastValue = 2/*30*/;
            }
        }
        else if (capinfo->current_histo <= HISTO_THRESHOLD)
        {
            diff_less3 = (capinfo->lessTHAN3 + capinfo->lessTHAN4) - (capinfo->prev_lessTHAN3 + capinfo->prev_lessTHAN4);

            if ((capinfo->current_histo - capinfo->noise_histo) > Dry_Wet_Tres)		//Normal Fingerprint Condition
                //			if( (capinfo.real_histo-capinfo.noise_histo)>Dry_Wet_Tres )		//Normal Fingerprint Condition
            {
                if ((diff_less3 >= 0 && diff_less3 < (capinfo->lessTHAN3 + capinfo->lessTHAN4) / 5) ||
                        (diff_less3 < 0 && -diff_less3 < (capinfo->lessTHAN3 + capinfo->lessTHAN4) / 5))		//Fingerprint stability check
                {
                    capinfo->condition_stay++;
                }

                if (capinfo->real_histo > 120 && propertyInfo->nContrastValue > 0)
                {
                    propertyInfo->nContrastValue -= 4;
                    if (propertyInfo->nContrastValue < 0)
                    {
                        propertyInfo->nContrastValue = 0;
                    }
                    return 1;
                }

                if (capinfo->condition_stay > 1)													//Normal stable End Condition
                {
                    //TRACE("Normal stable End Condition (%d %d)\n", capinfo->lessTHAN3, capinfo->lessTHAN4);
                    return 2;
                }
                else if ((capinfo->lessTHAN3 + capinfo->lessTHAN4) > 35000 && capinfo->noise_histo < 30)		//Normal good End Condition 2
                {
                    //TRACE("Normal stable End Condition2 (%d %d)\n", capinfo->lessTHAN3, capinfo->lessTHAN4);
                    return 2;
                }
                else if (capinfo->detected_count > 1 && (capinfo->lessTHAN3 + capinfo->lessTHAN4) > 20000 && capinfo->noise_histo < 10)		//Normal good End Condition 3
                {
                    //TRACE("Normal good End Condition 3 (%d %d)\n", capinfo->lessTHAN3, capinfo->lessTHAN4);
                    return 2;
                }
            }
            else if ((capinfo->current_histo - capinfo->noise_histo) <= Dry_Wet_Tres)		//Dry Fingerprint Condition
                //			else if( (capinfo.real_histo-capinfo.noise_histo)<=Dry_Wet_Tres )		//Dry Fingerprint Condition
            {
                if (capinfo->dry_count > 0 && propertyInfo->nIntegrationValue < __AUTO_CURVE_INTEGRATION_MAX_VALUE__)			//Set Register for dry fingerprint
                {
                    propertyInfo->nIntegrationValue = __AUTO_CURVE_INTEGRATION_MAX_VALUE__;
                    *SetRegisterForDry = TRUE;
                    capinfo->STOP_DETECT_COUNT = 6;
                }
                capinfo->dry_count++;
            }
        }
        if (capinfo->detected_count >= capinfo->STOP_DETECT_COUNT)		//Last image from Fingerprint detection when it is dry.
        {
            if ((capinfo->noise_histo >= capinfo->current_histo || capinfo->current_histo < 5 || (capinfo->current_histo - capinfo->noise_histo) < 2 ||
                    (capinfo->noise_lessTHAN3 > capinfo->lessTHAN3 && capinfo->noise_lessTHAN4 > capinfo->lessTHAN4)))		//If the finger is removing...
            {
                //				capinfo.detected_count = capinfo.STOP_DETECT_COUNT-2;
                capinfo->detected_count = capinfo->STOP_DETECT_COUNT - 1;
                capinfo->prev_lessTHAN3 = capinfo->lessTHAN3;
                capinfo->prev_lessTHAN4 = capinfo->lessTHAN4;
                capinfo->prev_histo = capinfo->current_histo;
                return 1;				//Go to the first
            }

            if (capinfo->real_histo > 120 && propertyInfo->nContrastValue > 0)
            {
                propertyInfo->nContrastValue -= 4;
                if (propertyInfo->nContrastValue < 0)
                {
                    propertyInfo->nContrastValue = 0;
                }
                return 1;
            }

            //TRACE("Last image from Fingerprint detection when it is dry\n");
            return 2;
        }

        capinfo->prev_lessTHAN3 = capinfo->lessTHAN3;
        capinfo->prev_lessTHAN4 = capinfo->lessTHAN4;
        capinfo->prev_histo = capinfo->current_histo;
        return 1;				//Go to the first
    }
    else
    {
        //if the image is the first empty frame
        if (capinfo->tot_frame == 1 || (capinfo->detected_count == 0 && capinfo->isDetectGain != propertyInfo->nContrastValue))
        {
            capinfo->GRAP_NULL_FRAME = 1;
            capinfo->isDetectGain = propertyInfo->nContrastValue;
        }

        //Initializing
        capinfo->dry_count = 0;
        capinfo->STOP_DETECT_COUNT = 4;
        capinfo->condition_stay = 0;
        capinfo->prev_lessTHAN3 = INIT_PREV;
        capinfo->prev_lessTHAN4 = INIT_PREV;
        capinfo->prev_histo = INIT_PREV;
        capinfo->detected_count = 0;

        if (capinfo->noise_histo > HISTO_THRESHOLD && propertyInfo->nContrastValue > 2/*30*/)		//Decrease Gain if there are lots of noises
        {
            capinfo->NOISE_DETECT = 1;
            //			propertyInfo->nContrastValue = propertyInfo->nContrastValue-propertyInfo->nContrastValue/3;
            tmp_calculated_gain = propertyInfo->nContrastValue * 7 + 17;
            propertyInfo->nContrastValue = tmp_calculated_gain - tmp_calculated_gain / 3;
            propertyInfo->nContrastValue = (propertyInfo->nContrastValue - 17) / 7;
            if (propertyInfo->nContrastValue < 2/*30*/)
            {
                propertyInfo->nContrastValue = 2/*30*/;
            }
        }

        return 0;
    }
}

int CIBAlgorithm::_Algo_Curve_GetFingerCount(unsigned char *InImg, int ImageBright)
{
    if ((m_CaptureInfo.current_histo + m_CaptureInfo.current_histo / 3 > m_CaptureInfo.noise_histo * 2  && m_CaptureInfo.current_histo > 4) ||
            (m_CaptureInfo.noise_histo > 80 && m_CaptureInfo.current_histo - m_CaptureInfo.noise_histo > 10) ||
            (m_CaptureInfo.noise_lessTHAN3 < 20 && m_CaptureInfo.noise_lessTHAN3 * 2 < m_CaptureInfo.lessTHAN3 && m_CaptureInfo.noise_lessTHAN4 * 3 < m_CaptureInfo.lessTHAN4 && m_CaptureInfo.noise_lessTHAN2 + m_CaptureInfo.noise_lessTHAN2 / 8 < m_CaptureInfo.lessTHAN2) ||
            (m_CaptureInfo.noise_lessTHAN3 >= 20 && m_CaptureInfo.noise_lessTHAN3 * 2 < m_CaptureInfo.lessTHAN3 && m_CaptureInfo.noise_lessTHAN4 * 3 < m_CaptureInfo.lessTHAN4) ||
            (m_CaptureInfo.lessTHAN4 > 1600 && m_CaptureInfo.noise_lessTHAN4 * 2 < m_CaptureInfo.lessTHAN4)
       )
    {
        if (_Algo_Curve_DetectPinhole_using_Ratio(InImg) == FALSE)
        {
            return 1;
        }
    }

    return 0;
}

void CIBAlgorithm::_Algo_Curve_JudgeFingerQuality(FrameImgAnalysis *pFrameImgAnalysis, BOOL bIsGoodImage, PropertyInfo *pPropertyInfo, CallbackProperty *pClbkProperty)
{
    const int MIN_RANGE = 35;
    const int MAX_RANGE = 50;

    memset(&pClbkProperty->qualityArray[0], ENUM_IBSU_FINGER_NOT_PRESENT, sizeof(pClbkProperty->qualityArray));

    // Update only the finger quality nearest to the single finger.
    int pos = pFrameImgAnalysis->center_x * 4 / m_pUsbDevInfo->CisImgWidth;

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

void CIBAlgorithm::_Algo_Curve_Init_Pinhole()
{
    int i, j;
    int radiusx, radiusy;
    int IMAGE_WIDTH_HALF = (CIS_IMG_W >> 1);
    int IMAGE_HEIGHT_HALF = (CIS_IMG_H >> 1);

    radiusx = (int)(CIS_IMG_W / 4);
    radiusy = (int)(CIS_IMG_H / 4);

    for (i = 0; i < CIS_IMG_H; i++)
    {
        for (j = 0; j < CIS_IMG_W; j++)
        {
            if ((int)((i - IMAGE_HEIGHT_HALF) * (i - IMAGE_HEIGHT_HALF) / (float)(radiusy * radiusy) + (j - IMAGE_WIDTH_HALF) * (j - IMAGE_WIDTH_HALF) / (float)(radiusx * radiusx)) > 1)
            {
                m_PinholeMask[i * CIS_IMG_W + j] = 255;
            }
            else
            {
                m_PinholeMask[i * CIS_IMG_W + j] = 0;
            }
        }
    }
}

BOOL CIBAlgorithm::_Algo_Curve_DetectPinhole_using_Ratio(const unsigned char *InImg)
{
    int i, j;
    int Count255_outer, Count255_inner;
    int Biased_Left, Biased_Right, Biased_Top, Biased_Bottom;
    int Biased_RatioX, Biased_RatioY;
    int BrightThreshold = 255;
    int IMAGE_WIDTH_HALF = (CIS_IMG_W >> 1);
    int IMAGE_HEIGHT_HALF = (CIS_IMG_H >> 1);
    int ratio;

    ratio = 0;
    Count255_outer = Count255_inner = 0;
    Biased_Left = Biased_Right = Biased_Top = Biased_Bottom = 0;
    Biased_RatioX = 0;
    Biased_RatioY = 0;

    for (i = 0; i < CIS_IMG_H; i++)
    {
        for (j = 0; j < CIS_IMG_W; j++)
        {
            if (InImg[i * CIS_IMG_W + j] >= BrightThreshold)
            {
                if (m_PinholeMask[i * CIS_IMG_W + j] == 255)
                {
                    Count255_inner++;
                }
                else
                {
                    Count255_outer++;
                }

                if (j < IMAGE_WIDTH_HALF)
                {
                    Biased_Left++;
                }
                else
                {
                    Biased_Right++;
                }

                if (i < IMAGE_HEIGHT_HALF)
                {
                    Biased_Top++;
                }
                else
                {
                    Biased_Bottom++;
                }
            }
        }
    }

    if (Count255_outer > Count255_inner)
    {
        if (Count255_inner > 0)
        {
            ratio = Count255_outer * 100 / Count255_inner;
        }
    }
    else
    {
        if (Count255_outer > 0)
        {
            ratio = Count255_inner * 100 / Count255_outer;
        }
    }

    if (Biased_Left > Biased_Right)
    {
        if (Biased_Right > 0)
        {
            Biased_RatioX = Biased_Left * 100 / Biased_Right;
        }
    }
    else
    {
        if (Biased_Left > 0)
        {
            Biased_RatioX = Biased_Right * 100 / Biased_Left;
        }
    }

    if (Biased_Top > Biased_Bottom)
    {
        if (Biased_Bottom > 0)
        {
            Biased_RatioY = Biased_Top * 100 / Biased_Bottom;
        }
    }
    else
    {
        if (Biased_Top > 0)
        {
            Biased_RatioY = Biased_Bottom * 100 / Biased_Top;
        }
    }

    if (ratio < 250 &&			// ratio btw inner and outer
            Biased_RatioX < 400 &&	// ratio btw left side and right side
            Biased_RatioY < 400 &&	// ratio btw up side and down side
            ratio > 0 &&			// non-zero pixel
            Biased_RatioX > 0 &&
            Biased_RatioY > 0 &&
            Count255_outer > 0 &&
            Count255_inner > 0 &&
            (Count255_outer + Count255_inner) < 6000)
    {
        return TRUE;
    }

    return FALSE;		// Pinhole is not detected
}

int CIBAlgorithm::_Algo_Curve_GetBrightWithRawImage(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY)
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

    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        yy = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H * CIS_IMG_W;

        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            xx = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W;

            value = InRawImg[yy + xx];
            /*			value = ( ((int)value*m_UM_Watson_F[yy+xx]) >> 10 ) + (int)value;
            			if(value > 255) value = 255;
            			else if(value < 0) value = 0;
            */			EnlargeBuf[(y)*ZOOM_ENLAGE_W + (x)] = value;
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

    //	if( Threshold < m_CaptureInfo.noise_histo )
    //		Threshold = m_CaptureInfo.noise_histo;


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








//Image enlargement for further processing
void CIBAlgorithm::EnlargeImage(unsigned char *input_image, unsigned char output_image[ENLARGE_IMG_H][ENLARGE_IMG_W])
{
    int y, x;

    memset(&output_image[0][0], 0, ENLARGE_CELL_H * ENLARGE_IMG_W);
    for (y = 0; y < IMAGE_WIDTH; y++)
    {
        for (x = 0; x < IMAGE_HEIGHT; x++)
        {
            output_image[y + ENLARGE_CELL_H][x + ENLARGE_CELL_W] = input_image[x * IMAGE_WIDTH + y];
        }
    }
    memset(&output_image[ENLARGE_CELL_H + IMAGE_WIDTH][0], 0, ENLARGE_CELL_H * ENLARGE_IMG_W);
}

//Median Filter for thermal noises
void CIBAlgorithm::MedianFilter(unsigned char image_buffer[ENLARGE_IMG_H][ENLARGE_IMG_W], unsigned char temp_buffer[ENLARGE_IMG_H][ENLARGE_IMG_W])
{
    int i, j;
    int a, b, c;
    int tp;

    memcpy(temp_buffer, image_buffer, ENLARGE_IMG_H * ENLARGE_IMG_W);

    for (i = ENLARGE_CELL_H; i < ENLARGE_IMG_H - ENLARGE_CELL_H; i++)
    {
        for (j = ENLARGE_CELL_W; j < ENLARGE_IMG_W - ENLARGE_CELL_W; j++)
        {
            a = temp_buffer[i][j];
            b = temp_buffer[i][j + 1];
            c = temp_buffer[i + 1][j];

            if (a > b)
            {
                tp = a;
                a = b;
                b = tp;
            }

            if (b > c)
            {
                tp = b;
                b = c;
                c = tp;
            }
            if (a > b)
            {
                tp = a;
                a = b;
                b = tp;
            }
            image_buffer[i][j] = b;
        }
    }
}

//Histogram stretching for image enhancement
void CIBAlgorithm::HistogramStretch(unsigned char image_buffer[ENLARGE_IMG_H][ENLARGE_IMG_W])
{
    double percent_low = 0.293;
    double percent_high = 0.05;
    int pixel_low = (int)(percent_low * (IMAGE_HEIGHT * IMAGE_WIDTH));
    int pixel_high = (int)(percent_high * (IMAGE_HEIGHT * IMAGE_WIDTH));
    int histogram[256];
    int histogram_stretch[256];
    int sum = 0;
    int min_value = 0;
    int max_value = 256;
    double koef;
    int temp;
    int i, j;

    for (i = 0; i < 256; i++)
    {
        histogram[i] = 0;
    }

    for (i = ENLARGE_CELL_H; i < ENLARGE_IMG_H - ENLARGE_CELL_H; i++)
    {
        for (j = ENLARGE_CELL_W; j < ENLARGE_IMG_W - ENLARGE_CELL_W; j++)
        {
            temp = image_buffer[i][j];
            histogram[temp]++;
        }
    }

    while ((sum < pixel_low) && (min_value < 256))
    {
        sum += histogram[min_value];
        min_value++;
    }

    sum = 0;

    while ((sum < pixel_high) && (max_value > min_value))
    {
        max_value--;
        sum += histogram[max_value];
    }

    for (j = 0; j <= min_value; j++)
    {
        if (j > 255)
        {
            continue;
        }
        histogram_stretch[j] = 0x00;
    }

    for (j = max_value; j <= 255; j++)
    {
        if (j > 255 || j < 0)
        {
            continue;
        }
        histogram_stretch[j] = 0xFF;
    }

    if (max_value != min_value)
    {
        koef = 255.0 / (max_value - min_value);
    }
    else
    {
        koef = 0.0;
    }

    for (j = min_value + 1; j <= max_value - 1; j++)
    {
        histogram_stretch[j] = (int)(koef * (j - min_value));

    }

    for (i = ENLARGE_CELL_H; i < ENLARGE_IMG_H - ENLARGE_CELL_H; i++)
    {
        for (j = ENLARGE_CELL_W; j < ENLARGE_IMG_W - ENLARGE_CELL_W; j++)
        {
            temp = image_buffer[i][j];
            image_buffer[i][j] = histogram_stretch[temp];
        }
    }
}

void CIBAlgorithm::StackFillOutside(unsigned char(*image)[ENLARGE_QUARTER_W], int x, int y, unsigned char fromValue, unsigned char toValue)
{
    int dx, dy;
    int top;

    top = -1;				// init_stack
    stack[++top] = y;		// push
    stack[++top] = x;		// push

    while (top >= 0)
    {
        dx = stack[top--];		// pop
        dy = stack[top--];		// pop
        if (dx < 0 || dy < 0 || dx >= D_ENLARGE_QUARTER_H || dy >= D_ENLARGE_QUARTER_W)
        {
            continue;
        }

        if (image[dy][dx] == fromValue)
        {
            image[dy][dx] = toValue;
            stack[++top] = dy;		// push
            stack[++top] = dx - 1;	// push

            stack[++top] = dy;		// push
            stack[++top] = dx + 1;	// push

            stack[++top] = dy - 1;	// push
            stack[++top] = dx;		// push

            stack[++top] = dy + 1;	// push
            stack[++top] = dx;		// push
        }
    }
}

//Background information extraction function
void CIBAlgorithm::BackgroundCheck(unsigned char image_buffer[ENLARGE_IMG_H][ENLARGE_IMG_W], unsigned char quabuf[ENLARGE_QUARTER_H][ENLARGE_QUARTER_W],
                                   int current_histo, int noise_histo, unsigned char *temp_buffer)
{
    int pt_size = 0;
    unsigned char(*smallimg_buffer)[ENLARGE_HALF_W] = (unsigned char( *)[ENLARGE_HALF_W])temp_buffer;
    pt_size += ENLARGE_HALF_W * ENLARGE_HALF_H * sizeof(unsigned char);
    unsigned int (*gradient_buffer)[ENLARGE_HALF_W] = (unsigned int ( *)[ENLARGE_HALF_W])&temp_buffer[pt_size];
    pt_size += ENLARGE_HALF_W * ENLARGE_HALF_H * sizeof(unsigned int);
    unsigned char(*smooth_buffer)[ENLARGE_QUARTER_W] = (unsigned char( *)[ENLARGE_QUARTER_W])&temp_buffer[pt_size];

    int j, i, k, l;
    int tempmean;
    int tempthreshold;
    int tempsum;
    int temp1, temp2;

    memset(quabuf, 255, ENLARGE_QUARTER_H * ENLARGE_QUARTER_W);
    memset(gradient_buffer, 0, ENLARGE_HALF_H * ENLARGE_HALF_W * sizeof(unsigned int));
    memset(smallimg_buffer, 0, ENLARGE_HALF_H * ENLARGE_HALF_W);
    tempmean = 0;

    for (i = ENLARGE_CELL_H; i < ENLARGE_IMG_H - ENLARGE_CELL_H; i += 2)
    {
        for (j = ENLARGE_CELL_W; j < ENLARGE_IMG_W - ENLARGE_CELL_W; j += 2)
        {
            smallimg_buffer[i / 2][j / 2] = (image_buffer[i][j]
                                             + image_buffer[i][j + 1]
                                             + image_buffer[i + 1][j]
                                             + image_buffer[i + 1][j + 1]) / 4;
        }
    }

    for (i = ENLARGE_CELL_H / 2; i < ENLARGE_HALF_H - ENLARGE_CELL_H / 2; i += 4)
    {
        for (j = ENLARGE_CELL_W / 2; j < ENLARGE_HALF_W - ENLARGE_CELL_W / 2; j += 4)
        {
            for (k = 0; k < 4; k++)
            {
                for (l = 0; l < 4; l++)
                {
                    temp1 = (int)smallimg_buffer[i + k][j + l] - (int)smallimg_buffer[i + k][j + l + 1];
                    temp2 = (int)smallimg_buffer[i + k][j + l] - (int)smallimg_buffer[i + k + 1][j + l];
                    temp1 = temp1 * temp1 + temp2 * temp2;
                    gradient_buffer[i + k][j + l] = temp1;
                    tempmean += temp1;
                }
            }
        }
    }

    tempmean /= (ENLARGE_HALF_H * ENLARGE_HALF_W);

    if (((current_histo - noise_histo) < 100 && noise_histo > 20) ||
            ((current_histo - noise_histo) < 30 && noise_histo > 0))
    {
        tempthreshold = 600000 - (abs(current_histo - noise_histo) * 19500);
        if (tempthreshold < 100000)
        {
            tempthreshold = 100000;
        }
    }
    else
    {
        tempthreshold = (int)(55000.0 - (tempmean * 8.71));
        if (tempthreshold > 49000)
        {
            tempthreshold = 49000;
        }
        else if (tempthreshold < 26500)
        {
            tempthreshold = 26000;
        }
    }

    for (i = ENLARGE_CELL_H / 2; i < ENLARGE_HALF_H - ENLARGE_CELL_H / 2; i += 2)
    {
        tempsum = 0;
        for (k = -4; k <= 4; k++)
        {
            for (l = -4; l <= 4; l++)
            {
                tempsum += gradient_buffer[i + k][ENLARGE_CELL_W / 2 + l];
            }
        }
        for (j = ENLARGE_CELL_W / 2 + 2; j < ENLARGE_HALF_W - ENLARGE_CELL_W / 2; j += 2)
        {
            for (k = -4; k <= 4; k++)
            {
                for (l = -6; l <= -5; l++)
                {
                    tempsum -= gradient_buffer[i + k][j + l];
                }
            }
            for (k = -4; k <= 4; k++)
            {
                for (l = 3; l <= 4; l++)
                {
                    tempsum += gradient_buffer[i + k][j + l];
                }
            }
            if (tempsum > tempthreshold)
            {
                quabuf[i / 2][j / 2] = 0;
            }
        }
    }

    memcpy(smooth_buffer, quabuf, ENLARGE_QUARTER_H * ENLARGE_QUARTER_W);

    //	FillOutside(smooth_buffer, 0, 0, 255, 0);
    StackFillOutside(smooth_buffer, 0, 0, 255, 0);
    for (i = 0; i < ENLARGE_QUARTER_H; i++)
    {
        for (j = 0; j < ENLARGE_QUARTER_W; j++)
        {
            if (smooth_buffer[i][j] == 255)
            {
                quabuf[i][j] = 0;
            }
        }
    }
}

//Make the image to original size
void CIBAlgorithm::Reverse_Enlarge(unsigned char Enlarge_buffer[ENLARGE_IMG_H][ENLARGE_IMG_W], unsigned char *image_buffer)
{
    int y, x;

    for (x = 0; x < IMG_W; x++)
    {
        for (y = 0; y < IMG_H; y++)
        {
            image_buffer[y * IMG_W + x] = Enlarge_buffer[x + ENLARGE_CELL_H][y + ENLARGE_CELL_H];
        }
    }
}

//Apply the background information to the image
void CIBAlgorithm::Back_Ground_display(unsigned char Backgound_buffer[ENLARGE_QUARTER_H][ENLARGE_QUARTER_W], unsigned char Enlarge_buffer[ENLARGE_IMG_H][ENLARGE_IMG_W])
{
    int y, x;

    for (y = 0; y < ENLARGE_IMG_H; y++)
    {
        for (x = 0; x < ENLARGE_IMG_W; x++)
        {
            if (Backgound_buffer[y >> 2][x >> 2] == 255)
            {
                Enlarge_buffer[y][x] = 0;
            }
        }
    }
}

void CIBAlgorithm::_Algo_Curve_Init_Distortion_Merge_with_Bilinear()
{
    float temp_i;

    for (int i = 0; i < IMG_H; i++)
    {
        temp_i = (float)i * CIS_IMG_H / IMG_H;
        m_Curve_p_arr[i] = (short)((temp_i - (short)temp_i) * 128);
        m_Curve_refy1_arr[i] = (int)temp_i;
        m_Curve_refy2_arr[i] = (int)temp_i + 1;
        if (m_Curve_refy1_arr[i] > CIS_IMG_H - 1)
        {
            m_Curve_refy1_arr[i] = CIS_IMG_H - 1;
        }
        if (m_Curve_refy2_arr[i] > CIS_IMG_H - 1)
        {
            m_Curve_refy2_arr[i] = CIS_IMG_H - 1;
        }

        m_Curve_refy1_arr[i] = m_Curve_refy1_arr[i] * CIS_IMG_W;
        m_Curve_refy2_arr[i] = m_Curve_refy2_arr[i] * CIS_IMG_W;
    }

    for (int i = 0; i < IMG_W; i++)
    {
        temp_i = (float)i * CIS_IMG_W / IMG_W;
        m_Curve_q_arr[i] = (short)((temp_i - (short)temp_i) * 128);
        m_Curve_refx1_arr[i] = (int)temp_i;
        m_Curve_refx2_arr[i] = (int)temp_i + 1;
        if (m_Curve_refx1_arr[i] > CIS_IMG_W - 1)
        {
            m_Curve_refx1_arr[i] = CIS_IMG_W - 1;
        }
        if (m_Curve_refx2_arr[i] > CIS_IMG_W - 1)
        {
            m_Curve_refx2_arr[i] = CIS_IMG_W - 1;
        }
    }
}

void CIBAlgorithm::_Algo_Curve_DistortionRestoration_with_Bilinear(unsigned char *InImg, unsigned char *OutImg)
{
    int i, j, p, q, refy1, refy2, refx1, refx2;
    int Value;

    for (i = 0; i < IMG_H; i++)
    {
        refy1 = m_Curve_refy1_arr[i];
        refy2 = m_Curve_refy2_arr[i];
        p = m_Curve_p_arr[i];

        for (j = 0; j < IMG_W; j++)
        {
            refx1 = m_Curve_refx1_arr[j];
            refx2 = m_Curve_refx2_arr[j];
            q = m_Curve_q_arr[j];

            Value = ((128 - p) * ((128 - q) * InImg[refy1 + refx1] + q * InImg[refy1 + refx2]) +
                     p * ((128 - q) * InImg[refy2 + refx1] + q * InImg[refy2 + refx2])) >> 14;

            OutImg[i * IMG_W + j] = (unsigned char)Value;
        }
    }
}

void CIBAlgorithm::_ALGO_Curve_HistogramEnhance(unsigned char *pInputImg, unsigned char *pOutputImg)
{
    int i, j;
    int Bright, Cnt, TargetBright = 110;
    int MinDiff = 256, CurDiff;
    unsigned char *GammaTable = (unsigned char *)&G_GammaTable[0];
    int Gamma_Idx = 0, GoodGammaIdx = -1;

    while (1)
    {
        GammaTable = (unsigned char *)&G_GammaTable[Gamma_Idx * 256];

        Bright = 0;
        Cnt = 0;

        for (i = 0; i < IMAGE_HEIGHT; i += 4)
        {
            for (j = 0; j < IMAGE_WIDTH; j += 4)
            {
                if (pInputImg[i * IMAGE_WIDTH + j] > 20)
                {
                    Bright += GammaTable[pInputImg[i * IMAGE_WIDTH + j]];
                    Cnt++;
                }
            }
        }

        if (Cnt > 0)
        {
            Bright = Bright / Cnt;
        }

        CurDiff = abs(TargetBright - Bright);

        if (MinDiff > CurDiff)
        {
            MinDiff = CurDiff;
            GoodGammaIdx = Gamma_Idx;
        }

        //TRACE("Bright : %d, CurDiff : %d, MinDiff : %d, GoodGammaIdx : %d\n", Bright, CurDiff, MinDiff, GoodGammaIdx);

        if (GoodGammaIdx > -1 && CurDiff > MinDiff)
        {
            break;
        }

        if (CurDiff / 3 > 0)
        {
            Gamma_Idx += CurDiff / 3;
        }
        else
        {
            Gamma_Idx++;
        }

        if (Gamma_Idx > 46)
        {
            GoodGammaIdx = 47;
            break;
        }
    }

    if (GoodGammaIdx > -1)
    {
        GammaTable = (unsigned char *)&G_GammaTable[GoodGammaIdx * 256];

        for (i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++)
        {
            pOutputImg[i] = GammaTable[pInputImg[i]];
        }
    }
}

void CIBAlgorithm::_Algo_Curve_OrgImageEnhance(unsigned char *pInputImg, unsigned char *pOutputImg)
{
    int i, j;
    int a, b, c;
    int tp, loop;

    unsigned char *TmpImg = m_Curve_Tmp1;
    unsigned char *TmpImg2 = m_Curve_Tmp2;

    memcpy(TmpImg, pInputImg, CIS_IMG_W * CIS_IMG_H);
    memcpy(TmpImg2, pInputImg, CIS_IMG_W * CIS_IMG_H);

    // remove noise
    int center, outsum;
    int BrightThres = 30;
    int MeanBright = 0;

    MeanBright = m_cImgAnalysis.mean;

    BrightThres = BrightThres * MeanBright / 128;
    if (BrightThres > 30)
    {
        BrightThres = 30;
    }
    else if (BrightThres < 10)
    {
        BrightThres = 10;
    }

    for (loop = 0; loop < 5; loop++)
    {
        memcpy(TmpImg2, TmpImg, CIS_IMG_W * CIS_IMG_H);
        for (i = 1; i < CIS_IMG_H - 1; i++)
        {
            for (j = 1; j < CIS_IMG_W - 1; j++)
            {
                center = TmpImg[i * CIS_IMG_W + j];

                outsum = (TmpImg[(i - 1) * CIS_IMG_W + (j - 1)] + TmpImg[(i - 1) * CIS_IMG_W + (j)] + TmpImg[(i - 1) * CIS_IMG_W + (j + 1)] +
                          TmpImg[(i) * CIS_IMG_W + (j - 1)] + TmpImg[(i) * CIS_IMG_W + (j + 1)] +
                          TmpImg[(i + 1) * CIS_IMG_W + (j - 1)] + TmpImg[(i + 1) * CIS_IMG_W + (j)] + TmpImg[(i + 1) * CIS_IMG_W + (j + 1)]) >> 3;

                if (center > outsum + BrightThres)
                {
                    TmpImg2[i * CIS_IMG_W + j] = (center + outsum * 2) / 3;
                }
                else if (center < outsum + BrightThres)
                {
                    TmpImg2[i * CIS_IMG_W + j] = (center + outsum * 2) / 3;
                }
            }
        }
    }

    memcpy(TmpImg, TmpImg2, CIS_IMG_W * CIS_IMG_H);

    for (i = 1; i < CIS_IMG_H - 1; i++)
    {
        for (j = 1; j < CIS_IMG_W - 1; j++)
        {
            a = TmpImg[i * CIS_IMG_W + j];
            b = TmpImg[i * CIS_IMG_W + j - 1];
            c = TmpImg[(i - 1) * CIS_IMG_W + j];

            if (a > b)
            {
                tp = a;
                a = b;
                b = tp;
            }

            if (b > c)
            {
                tp = b;
                b = c;
                c = tp;
            }
            if (a > b)
            {
                tp = a;
                a = b;
                b = tp;
            }
            TmpImg2[i * CIS_IMG_W + j] = b;
        }
    }

    memcpy(TmpImg, TmpImg2, CIS_IMG_W * CIS_IMG_H);

    for (i = 1; i < CIS_IMG_H - 1; i++)
    {
        for (j = 1; j < CIS_IMG_W - 1; j++)
        {
            a = TmpImg[i * CIS_IMG_W + j];
            b = TmpImg[i * CIS_IMG_W + j + 1];
            c = TmpImg[(i + 1) * CIS_IMG_W + j];

            if (a > b)
            {
                tp = a;
                a = b;
                b = tp;
            }

            if (b > c)
            {
                tp = b;
                b = c;
                c = tp;
            }
            if (a > b)
            {
                tp = a;
                a = b;
                b = tp;
            }
            TmpImg2[i * CIS_IMG_W + j] = b;
        }
    }

    memcpy(pInputImg, TmpImg2, CIS_IMG_W * CIS_IMG_H);
}

void CIBAlgorithm::_ALGO_Curve_Vignetting(unsigned char *pInputImg, unsigned char *pOutputImg)
{
    int i, j;
    int value;// value_sum, cnt;
    int Is_Inverted_Image = 1;
    int vig_value;
    int histogram[256];

    unsigned char  *TmpImg = m_Curve_Tmp1;
    unsigned char  *TmpImg2 = m_Curve_Tmp2;

    memset(pOutputImg, 255, IMG_SIZE);
    memset(TmpImg, 0, D_ENLARGE_W * D_ENLARGE_H);
    memset(TmpImg2, 0, D_ENLARGE_W * D_ENLARGE_H);
    memset(histogram, 0, sizeof(histogram));

    //////////////////////////////////////////////////////////////
    // judge which background
    int low_count = 0, high_count = 0;
    for (i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++)
    {
        value = pInputImg[i];
        histogram[value]++;
    }
    for (i = 0; i < 20; i++)
    {
        low_count += histogram[i];
    }
    for (i = 255 - 20; i < 256; i++)
    {
        high_count += histogram[i];
    }

    if (high_count > low_count)
    {
        Is_Inverted_Image = 1;
    }
    else
    {
        Is_Inverted_Image = 0;
    }
    //////////////////////////////////////////////////////////////

    if (Is_Inverted_Image == 1)
    {
        for (i = 0; i < IMAGE_HEIGHT; i++)
        {
            for (j = 0; j < IMAGE_WIDTH; j++)
            {
                value = 255 - pInputImg[i * IMAGE_WIDTH + j];
                value -= 10;
                if (value < 0)
                {
                    value = 0;
                }
                TmpImg[(i + D_ENLARGE_BAND_H) * D_ENLARGE_W + (j + D_ENLARGE_BAND_W)] = value;
            }
        }
    }
    else
    {
        for (i = 0; i < IMAGE_HEIGHT; i++)
        {
            for (j = 0; j < IMAGE_WIDTH; j++)
            {
                value = pInputImg[i * IMAGE_WIDTH + j];
                value -= 10;
                if (value < 0)
                {
                    value = 0;
                }
                TmpImg[(i + D_ENLARGE_BAND_H) * D_ENLARGE_W + (j + D_ENLARGE_BAND_W)] = pInputImg[i * IMAGE_WIDTH + j];
            }
        }
    }

    for (i = D_ENLARGE_BAND_H; i < D_ENLARGE_H - D_ENLARGE_BAND_H; i++)
    {
        for (j = D_ENLARGE_BAND_W; j < D_ENLARGE_W - D_ENLARGE_BAND_W; j++)
        {
            vig_value = vigImage_Curve[(i - D_ENLARGE_BAND_H) * IMAGE_WIDTH + (j - D_ENLARGE_BAND_W)] - 50;
            value = (int)(TmpImg[i * D_ENLARGE_W + j] + ((TmpImg[i * D_ENLARGE_W + j] * vig_value) / 96));
            if (value > 255)
            {
                value = 255;
            }
            TmpImg2[i * D_ENLARGE_W + j] = value;      // invert
        }
    }

    if (Is_Inverted_Image == 1)
    {
        for (i = 0; i < IMAGE_HEIGHT; i++)
        {
            for (j = 0; j < IMAGE_WIDTH; j++)
            {
                pOutputImg[i * IMAGE_WIDTH + j] = 255 - TmpImg2[(i + D_ENLARGE_BAND_H) * D_ENLARGE_W + (j + D_ENLARGE_BAND_W)];
            }
        }
    }
    else
    {
        for (i = 0; i < IMAGE_HEIGHT; i++)
        {
            for (j = 0; j < IMAGE_WIDTH; j++)
            {
                pOutputImg[i * IMAGE_WIDTH + j] = TmpImg2[(i + D_ENLARGE_BAND_H) * D_ENLARGE_W + (j + D_ENLARGE_BAND_W)];
            }
        }
    }
}

void CIBAlgorithm::_ALGO_Curve_SWUniformity(unsigned char *pInputImg, unsigned char *pOutputImg)
{
    int i, j;
    int value;
    int histogram[256];
    int Is_Inverted_Image = 1;

    unsigned char  *TmpImg = m_Curve_Tmp1;
    unsigned char  *TmpImg2 = m_Curve_Tmp2;

    memset(pOutputImg, 255, IMG_SIZE);
    memset(TmpImg, 0, D_ENLARGE_W * D_ENLARGE_H);
    memset(TmpImg2, 0, D_ENLARGE_W * D_ENLARGE_H);
    memset(histogram, 0, sizeof(histogram));

    //////////////////////////////////////////////////////////////
    // judge which background
    int low_count = 0, high_count = 0;
    for (i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++)
    {
        value = pInputImg[i];
        histogram[value]++;
    }
    for (i = 0; i < 20; i++)
    {
        low_count += histogram[i];
    }
    for (i = 255 - 20; i < 256; i++)
    {
        high_count += histogram[i];
    }

    if (high_count > low_count)
    {
        Is_Inverted_Image = 1;
    }
    else
    {
        Is_Inverted_Image = 0;
    }
    //////////////////////////////////////////////////////////////

    if (Is_Inverted_Image == 1)
    {
        for (i = 0; i < IMAGE_HEIGHT; i++)
        {
            for (j = 0; j < IMAGE_WIDTH; j++)
            {
                value = 255 - pInputImg[i * IMAGE_WIDTH + j];
                value -= 10;
                if (value < 0)
                {
                    value = 0;
                }
                TmpImg[(i + D_ENLARGE_BAND_H) * D_ENLARGE_W + (j + D_ENLARGE_BAND_W)] = value;
            }
        }
    }
    else
    {
        for (i = 0; i < IMAGE_HEIGHT; i++)
        {
            for (j = 0; j < IMAGE_WIDTH; j++)
            {
                value = pInputImg[i * IMAGE_WIDTH + j];
                value -= 10;
                if (value < 0)
                {
                    value = 0;
                }
                TmpImg[(i + D_ENLARGE_BAND_H) * D_ENLARGE_W + (j + D_ENLARGE_BAND_W)] = pInputImg[i * IMAGE_WIDTH + j];
            }
        }
    }

    for (i = 0; i < D_ENLARGE_H * D_ENLARGE_W; i++)
    {
        TmpImg[i] = 255 - TmpImg[i];
    }
    memcpy(TmpImg2, TmpImg, D_ENLARGE_H * D_ENLARGE_W);

    int val, loop;
    // sw uniform
    for (loop = 0; loop < 2; loop++)
    {
        for (i = D_ENLARGE_BAND_H; i < D_ENLARGE_H - D_ENLARGE_BAND_H; i++)
        {
            for (j = D_ENLARGE_BAND_W; j < D_ENLARGE_W - D_ENLARGE_BAND_W; j++)
            {
                TmpImg2[i * D_ENLARGE_W + j] = (TmpImg[(i - 1) * D_ENLARGE_W + (j - 1)] + // 1A
                                                (TmpImg[(i - 1) * D_ENLARGE_W + j] << 3) + // 8B
                                                TmpImg[(i - 1) * D_ENLARGE_W + (j + 1)] + // 1C
                                                (TmpImg[i * D_ENLARGE_W + (j - 1)] << 3) + // 8D
                                                (TmpImg[i * D_ENLARGE_W + j] << 5) + // 32E
                                                (TmpImg[i * D_ENLARGE_W + (j + 1)] << 3) + // 8F
                                                TmpImg[(i + 1) * D_ENLARGE_W + (j - 1)] + // 1G
                                                (TmpImg[(i + 1) * D_ENLARGE_W + j] << 3) + // 8H
                                                TmpImg[(i + 1) * D_ENLARGE_W + (j + 1)] // 1I
                                               ) / 68;
            }
        }
        memcpy(TmpImg, TmpImg2, D_ENLARGE_W * D_ENLARGE_H);

        if (loop == 1)
        {
            break;
        }

        for (i = D_ENLARGE_BAND_H; i < D_ENLARGE_H - D_ENLARGE_BAND_H; i++)
        {
            for (j = D_ENLARGE_BAND_W; j < D_ENLARGE_W - D_ENLARGE_BAND_W; j++)
            {
                // 2013-04-03 Gon add : increase speed
                if (TmpImg2[i * IMG_W + j] == 255)
                {
                    continue;
                }

                val = (TmpImg[(i - 1) * D_ENLARGE_W + (j - 1)] + // 1A
                       (TmpImg[(i - 1) * D_ENLARGE_W + j] << 3) + // 8B
                       TmpImg[(i - 1) * D_ENLARGE_W + (j + 1)] + // 1C
                       (TmpImg[i * D_ENLARGE_W + (j - 1)] << 3) + // 8D
                       (TmpImg[i * D_ENLARGE_W + j] << 5) + // 32E
                       (TmpImg[i * D_ENLARGE_W + (j + 1)] << 3) + // 8F
                       TmpImg[(i + 1) * D_ENLARGE_W + (j - 1)] + // 1G
                       (TmpImg[(i + 1) * D_ENLARGE_W + j] << 3) + // 8H
                       TmpImg[(i + 1) * D_ENLARGE_W + (j + 1)] // 1I
                      ) / 68;

                val = (int)TmpImg[i * D_ENLARGE_W + j] - val;
                if (val > 0)
                {
                    val = val >> 1;
                }
                val = (int)TmpImg[i * D_ENLARGE_W + j] + ((val * 512) >> 7);
                if (val > 255)
                {
                    val = 255;
                }
                else if (val < 0)
                {
                    val = 0;
                }

                TmpImg2[i * D_ENLARGE_W + j] = (unsigned char)val;
            }
        }
        memcpy(TmpImg, TmpImg2, D_ENLARGE_W * D_ENLARGE_H);
    }

    for (i = D_ENLARGE_BAND_H; i < D_ENLARGE_H - D_ENLARGE_BAND_H; i++)
    {
        for (j = D_ENLARGE_BAND_W; j < D_ENLARGE_W - D_ENLARGE_BAND_W; j++)
        {
            // 2013-04-03 Gon add : increase speed
            if (TmpImg2[i * D_ENLARGE_W + j] == 255)
            {
                continue;
            }

            val = (TmpImg[(i - 1) * D_ENLARGE_W + (j - 1)] + // 1A
                   (TmpImg[(i - 1) * D_ENLARGE_W + j] << 3) + // 8B
                   TmpImg[(i - 1) * D_ENLARGE_W + (j + 1)] + // 1C
                   (TmpImg[i * D_ENLARGE_W + (j - 1)] << 3) + // 8D
                   (TmpImg[i * D_ENLARGE_W + j] << 5) + // 32E
                   (TmpImg[i * D_ENLARGE_W + (j + 1)] << 3) + // 8F
                   TmpImg[(i + 1) * D_ENLARGE_W + (j - 1)] + // 1G
                   (TmpImg[(i + 1) * D_ENLARGE_W + j] << 3) + // 8H
                   TmpImg[(i + 1) * D_ENLARGE_W + (j + 1)] // 1I
                  ) / 68;

            val = (int)TmpImg[i * D_ENLARGE_W + j] - val;
            if (val > 0)
            {
                val = val >> 1;
            }
            val = (int)TmpImg[i * D_ENLARGE_W + j] + ((val * 768) >> 7);
            if (val > 255)
            {
                val = 255;
            }
            else if (val < 0)
            {
                val = 0;
            }

            TmpImg2[i * D_ENLARGE_W + j] = (unsigned char)val;
        }
    }

    for (i = 0; i < D_ENLARGE_H * D_ENLARGE_W; i++)
    {
        TmpImg2[i] = 255 - TmpImg2[i];
    }
    memcpy(TmpImg, TmpImg2, D_ENLARGE_H * D_ENLARGE_W);

    if (Is_Inverted_Image == 1)
    {
        for (i = 0; i < IMAGE_HEIGHT; i++)
        {
            for (j = 0; j < IMAGE_WIDTH; j++)
            {
                pOutputImg[i * IMAGE_WIDTH + j] = 255 - TmpImg2[(i + D_ENLARGE_BAND_H) * D_ENLARGE_W + (j + D_ENLARGE_BAND_W)];
            }
        }
    }
    else
    {
        for (i = 0; i < IMAGE_HEIGHT; i++)
        {
            for (j = 0; j < IMAGE_WIDTH; j++)
            {
                pOutputImg[i * IMAGE_WIDTH + j] = TmpImg2[(i + D_ENLARGE_BAND_H) * D_ENLARGE_W + (j + D_ENLARGE_BAND_W)];
            }
        }
    }
}
