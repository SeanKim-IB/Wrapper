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
#ifdef WINCE
#include "WinCEMath.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#ifdef __linux__
#include <sys/time.h>
#endif

/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Columbo Algorithm


void CIBAlgorithm::_Algo_Columbo_Init_Distortion_Merge_with_Bilinear()
{
    //	int CIS_IMG_H = m_UsbDeviceInfo.CisImgHeight;
    //	int CIS_IMG_W = m_UsbDeviceInfo.CisImgWidth;

    //-값은 화면을 오목하게 만든다. + 값은 화면을 볼록하게 만든다.
    //A를 바꾸면 변화량이 가장 크고 C가 가장작다.
    /*float DISTORTION_A = -0.010f;
    float DISTORTION_B = -0.010f;
    float DISTORTION_C = 0.010f;*/

    // 2013-03-08 Gon modify - We change this value to meet the PIV certification.
    //	int cut_width = 1100 - 992;// - 32; // 950
    //	int cut_height = 880 - 736;//680;// - 16;	// 760

    //int cut_width = 106; //기준보드 497 dpi 컷
    //int cut_height = 134;

	int cut_width = 112; //넚이와 높이가 4:5 비율이니 컷팅 비율도 4:5로 줄여야 한다. 
    int cut_height = 142;// 즉 컷팅되는 비율이 4:5 여야 한다. (가로 10을 짤랐으면 세로는 12.5)

    float DISTORTION_A, DISTORTION_B, DISTORTION_C;

    float DISTORTION_D = 0.000f;
    float DISTORTION_RX = (float)(CIS_IMG_W - cut_width) / 2.0f;
    float DISTORTION_RY = (float)(CIS_IMG_H - cut_height) / 2.0f;

    int i, j, x, y;
    float DestX, DestY, SrcX, SrcY;
    float DestR, DestA, SrcR/*, tmp_DISTORTION_A*/;
    float Inv_Distortion_ABCD = 0;//1.0f - DISTORTION_A - DISTORTION_B - DISTORTION_C - DISTORTION_D;
    float p, q;
    int HalfX, HalfY;
    float TargetHalfX, TargetHalfY;
    float shiftx = cut_width / 2.0f;
    float shifty = cut_height / 2.0f;

    TargetHalfX = DISTORTION_RX;
    TargetHalfY = DISTORTION_RY;

    HalfX = IMG_W / 2;
    HalfY = IMG_H / 2;

    DISTORTION_A = -0.0025f;//0.00005f;
    DISTORTION_B = 0;//-0.0025f;
    DISTORTION_C = 0;//-0.005f;
    DISTORTION_D = 0.00f;
    Inv_Distortion_ABCD = 1.0f - DISTORTION_A - DISTORTION_B - DISTORTION_C - DISTORTION_D;

    // make table
    for (i = -HalfY; i < HalfY; i++)
    {
        DestY = (float)i / (float)HalfY;

        for (j = -HalfX; j < HalfX; j++)
        {
            DestX = (float)j / (float)HalfX;

            DestR = sqrtf(DestY * DestY + DestX * DestX);
            DestA = atan2f(DestY, DestX);

            SrcR = (DISTORTION_A * DestR * DestR * DestR * DestR + DISTORTION_B * DestR * DestR * DestR + DISTORTION_C * DestR * DestR
                    + DISTORTION_D * DestR + Inv_Distortion_ABCD) * DestR;

            SrcX = SrcR * cosf(DestA) * TargetHalfX + TargetHalfX + shiftx + 0.5f;
            SrcY = SrcR * sinf(DestA) * TargetHalfY + TargetHalfY + shifty + 0.5f;

            x = (int)SrcX;
            y = (int)SrcY;

            if (x >= 0 && x < CIS_IMG_W && y >= 0 && y < CIS_IMG_H)
            {
                p = 1.0f - (SrcX - (float)x);
                q = 1.0f - (SrcY - (float)y);

                arrPos_Columbo[(i + HalfY)*IMG_W + j + HalfX] = (unsigned long)((x & 0x7FF) << 21);
                arrPos_Columbo[(i + HalfY)*IMG_W + j + HalfX] |= (unsigned long)((y & 0x7FF) << 10);
                arrPos_Columbo[(i + HalfY)*IMG_W + j + HalfX] |= (unsigned long)((int)(p * 32) << 5);
                arrPos_Columbo[(i + HalfY)*IMG_W + j + HalfX] |= (unsigned long)(q * 32);
            }
            else
            {
                arrPos_Columbo[(i + HalfY)*IMG_W + j + HalfX] = 0xffffffff;
            }
        }
    }

    shiftx = (float)cut_width / 4.0f;
    shifty = (float)cut_height / 4.0f;

    // for decimation
    DISTORTION_RX = (float)(CIS_IMG_W - cut_width) / 4.0f;
    DISTORTION_RY = (float)(CIS_IMG_H - cut_height) / 4.0f;

    Inv_Distortion_ABCD = 0;//1.0f - DISTORTION_A - DISTORTION_B - DISTORTION_C - DISTORTION_D;

    TargetHalfX = DISTORTION_RX;
    TargetHalfY = DISTORTION_RY;

    HalfX = IMG_W / 2;
    HalfY = IMG_H / 2;

    DISTORTION_A = -0.0025f;//0.00005f;
    DISTORTION_B = 0;//-0.0025f;
    DISTORTION_C = 0;//-0.005f;
    DISTORTION_D = 0.00f;
    Inv_Distortion_ABCD = 1.0f - DISTORTION_A - DISTORTION_B - DISTORTION_C - DISTORTION_D;

    DWORD *arrPos_Columbo_deci_4 = &arrPos_Columbo[IMG_SIZE];
    // make table
    for (i = -HalfY; i < HalfY; i++)
    {
        DestY = (float)i / (float)HalfY;

        for (j = -HalfX; j < HalfX; j++)
        {
            DestX = (float)j / (float)HalfX;

            DestR = sqrtf(DestY * DestY + DestX * DestX);
            DestA = atan2f(DestY, DestX);

            SrcR = (DISTORTION_A * DestR * DestR * DestR * DestR + DISTORTION_B * DestR * DestR * DestR + DISTORTION_C * DestR * DestR
                    + DISTORTION_D * DestR + Inv_Distortion_ABCD) * DestR;

            SrcX = SrcR * cosf(DestA) * TargetHalfX + TargetHalfX + shiftx + 0.5f;
            SrcY = SrcR * sinf(DestA) * TargetHalfY + TargetHalfY + shifty + 0.5f;

            x = (int)SrcX;
            y = (int)SrcY;

            if (x >= 0 && x < CIS_IMG_W / 2 && y >= 0 && y < CIS_IMG_H / 2)
            {
                p = 1.0f - (SrcX - (float)x);
                q = 1.0f - (SrcY - (float)y);

                arrPos_Columbo_deci_4[(i + HalfY)*IMG_W + j + HalfX] = (unsigned long)((x & 0x7FF) << 21);
                arrPos_Columbo_deci_4[(i + HalfY)*IMG_W + j + HalfX] |= (unsigned long)((y & 0x7FF) << 10);
                arrPos_Columbo_deci_4[(i + HalfY)*IMG_W + j + HalfX] |= (unsigned long)((int)(p * 32) << 5);
                arrPos_Columbo_deci_4[(i + HalfY)*IMG_W + j + HalfX] |= (unsigned long)(q * 32);
            }
            else
            {
                arrPos_Columbo_deci_4[(i + HalfY)*IMG_W + j + HalfX] = 0xffffffff;
            }
        }
    }
}

void CIBAlgorithm::_Algo_Columbo_DistortionRestoration_with_Bilinear(unsigned char *InImg, unsigned char *OutImg)
{
    //	if( m_bCaptureThread_StopMessage ) return;

    int value;
    int i, j, x, y, pos;
    //	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;
    int maxval;

    memset(OutImg, 0, IMG_SIZE);
    //	dtLEFT		= m_cImgAnalysis.LEFT * 400 / 880;
    //	dtRIGHT		= m_cImgAnalysis.RIGHT * 400 / 880;
    //	dtTOP		= m_cImgAnalysis.TOP * 500 / 1100;
    //	dtBOTTOM	= m_cImgAnalysis.BOTTOM * 500 / 1100;

    for (i = 0; i < IMG_H; i++)
    {
        for (j = 0; j < IMG_W; j++)
        {
            x = (arrPos_Columbo[i * IMG_W + j] >> 21);
            y = (arrPos_Columbo[i * IMG_W + j] >> 10) & 0x7FF;
            pos = y * CIS_IMG_W + x;

            maxval = 0;
            if (InImg[pos] > maxval)
            {
                maxval = InImg[pos];
            }
            if (InImg[pos + CIS_IMG_W] > maxval)
            {
                maxval = InImg[pos + CIS_IMG_W];
            }
            if (InImg[pos + 1] > maxval)
            {
                maxval = InImg[pos + 1];
            }
            if (InImg[pos + CIS_IMG_W + 1] > maxval)
            {
                maxval = InImg[pos + CIS_IMG_W + 1];
            }

            value = maxval;

            OutImg[i * IMG_W + j] = (unsigned char)value;
        }
    }
}

int CIBAlgorithm::_Algo_Columbo_GetBrightWithRawImage(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY)
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

	forgroundCNT = forgroundCNT*3/2;
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
            // enzyme 2013-02-04 modify -
            //			m_cImgAnalysis.LEFT=(x-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W - 30;
            m_cImgAnalysis.LEFT = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W - 70;
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
            // enzyme 2013-02-04 modify -
            //			m_cImgAnalysis.RIGHT=(x-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W + 30;
            m_cImgAnalysis.RIGHT = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W + 70;
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
            // enzyme 2013-02-04 modify -
            //			m_cImgAnalysis.TOP=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W - 30;
            m_cImgAnalysis.TOP = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W - 90;
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
            // enzyme 2013-02-04 modify -
            //			m_cImgAnalysis.BOTTOM=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W + 30;
            m_cImgAnalysis.BOTTOM = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W + 90;
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

BOOL CIBAlgorithm::_Algo_Columbo_AutoCapture(BYTE *InImg, int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime)
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
    const int	TOLERANCE = 25;
    const int	MAX_STEP_CHANGE = 5;	// default 3
#endif

    m_cImgAnalysis.max_same_gain_count = 5;

	m_cImgAnalysis.diff_foreground_count = abs(m_cImgAnalysis.foreground_count - m_cImgAnalysis.pre_foreground_count);
	m_cImgAnalysis.diff_center_x = abs(m_cImgAnalysis.center_x - m_cImgAnalysis.pre_center_x);
	m_cImgAnalysis.diff_center_y = abs(m_cImgAnalysis.center_y - m_cImgAnalysis.pre_center_y);

    //	if( ForgraoundCNT < (propertyInfo->nSINGLE_FLAT_AREA_TRES>>4) )
    if (m_cImgAnalysis.foreground_count < 100)
    {
        propertyInfo->nContrastValue = m_pUsbDevInfo->nDefaultContrastValue;
        propertyInfo->nVoltageValue = __DEFAULT_VOLTAGE_VALUE__;
        m_cImgAnalysis.frame_delay = 2;
        m_cImgAnalysis.good_frame_count = 0;
        m_cImgAnalysis.is_final = FALSE;
        m_cImgAnalysis.final_adjust_gain = FALSE;
        m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
        m_cImgAnalysis.gain_step = 0;
        m_cImgAnalysis.changed_gain_step_count = 0;
        m_cImgAnalysis.same_gain_count = 0;
        m_cImgAnalysis.pre_gain = __DEFAULT_VOLTAGE_VALUE__;
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
            !m_cImgAnalysis.is_final &&
            m_cImgAnalysis.changed_gain_step_count < MAX_STEP_CHANGE)
    {
        if (m_cImgAnalysis.mean < MIN_BRIGHT)
        {
            // enzyme 2013-01-31 modify to improve capture speed
            if (m_cImgAnalysis.changed_gain_step_count > 2/* || m_cImgAnalysis.mean > (MIN_BRIGHT-TOLERANCE)*/)
            {
                m_cImgAnalysis.frame_delay = 1;
            }

            if (propertyInfo->nVoltageValue >= __AUTO_VOLTAGE_MAX_VALUE__)
            {
                m_cImgAnalysis.is_complete_voltage_control = TRUE;
            }

            if (!m_cImgAnalysis.is_complete_voltage_control)
            {
                // Let's change voltage
                propertyInfo->nVoltageValue += 1;

                if (propertyInfo->nVoltageValue > __AUTO_VOLTAGE_MAX_VALUE__)
                {
                    propertyInfo->nVoltageValue = __AUTO_VOLTAGE_MAX_VALUE__;
                }
            }
            else
            {
                // Let's change gain
                if (m_cImgAnalysis.mean > (MIN_BRIGHT - TOLERANCE))
                {
                    // enzyme 2013-01-31 modify to improve capture speed
                    //					if( frameTime < 70 )
                    //						m_cImgAnalysis.frame_delay = 1;
#if defined(WINCE)
                    propertyInfo->nContrastValue += 2;
#else
                    propertyInfo->nContrastValue += 1;
#endif
                }
                else
				{
#if defined(__android__)
                    propertyInfo->nContrastValue += 3;
#elif defined(WINCE)
                    propertyInfo->nContrastValue += 4;
#else
                    propertyInfo->nContrastValue += 2;
#endif
				}

                propertyInfo->nContrastValue += 1;

                if (propertyInfo->nContrastValue > __AUTO_COLUMBO_CONTRAST_MAX_VALUE__)
                {
                    propertyInfo->nContrastValue = __AUTO_COLUMBO_CONTRAST_MAX_VALUE__;
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

            if (propertyInfo->nVoltageValue <= __DEFAULT_VOLTAGE_VALUE__)
            {
                m_cImgAnalysis.is_complete_voltage_control = TRUE;
            }

            if (!m_cImgAnalysis.is_complete_voltage_control)
            {
                // Let's change voltage
                propertyInfo->nVoltageValue -= 1;

                if (propertyInfo->nVoltageValue < __DEFAULT_VOLTAGE_VALUE__)
                {
                    propertyInfo->nVoltageValue = __DEFAULT_VOLTAGE_VALUE__;
                }
            }
            else
            {
                // Let's change gain
                if (m_cImgAnalysis.mean < (MAX_BRIGHT + TOLERANCE))
                {
                    // enzyme 2013-01-31 modify to improve capture speed
                    //					if( frameTime < 70 )
                    //						m_cImgAnalysis.frame_delay = 1;
#if defined(WINCE)
                    propertyInfo->nContrastValue -= 2;
#else
                    propertyInfo->nContrastValue -= 1;
#endif
                }
                else
				{
#if defined(__android__)
                    propertyInfo->nContrastValue -= 3;
#elif defined(WINCE)
                    propertyInfo->nContrastValue -= 4;
#else
                    propertyInfo->nContrastValue -= 2;
#endif
				}

                propertyInfo->nContrastValue -= 1;
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

    /**
     * 2013-06-11 enzyme modify - to make fast capture (Do not need this routine in the flat scanner
    	if( (m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT) || m_cImgAnalysis.final_adjust_gain )
    	{
    		if( m_cImgAnalysis.good_frame_count++ > 0 )
    		{
    			if( m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT )
    			{
    				m_cImgAnalysis.final_adjust_gain = TRUE;
    				m_cImgAnalysis.frame_delay = -1;
    			}
    		}
    	}
    */
    if ((m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT) || m_cImgAnalysis.final_adjust_gain)
    {
        m_cImgAnalysis.good_frame_count = 2;
        //		if( m_cImgAnalysis.good_frame_count++ > 0 )
        {
            if (m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT)
            {
                m_cImgAnalysis.final_adjust_gain = TRUE;
                m_cImgAnalysis.frame_delay = -1;
            }
        }
    }

    //TRACE("<====> mean=%d, gain=%d, voltage=%d, good_frame_count=%d, same_gain_count=%d, same_gain_time=%d, frame_count=%d\n",
    //		m_cImgAnalysis.mean, propertyInfo->nContrastValue, propertyInfo->nVoltageValue, m_cImgAnalysis.good_frame_count,
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
				m_cImgAnalysis.final_mean = m_cImgAnalysis.mean;
                m_cImgAnalysis.is_final = TRUE;
            }
        }
    }

    m_cImgAnalysis.pre_foreground_count = m_cImgAnalysis.foreground_count;
    m_cImgAnalysis.pre_center_x = m_cImgAnalysis.center_x;
    m_cImgAnalysis.pre_center_y = m_cImgAnalysis.center_y;

    return m_cImgAnalysis.is_final;
}

BOOL CIBAlgorithm::_Algo_Columbo_AutoCaptureByExposure(BYTE *InImg, int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime)
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
    const int	TOLERANCE = 25;
    const int	MAX_STEP_CHANGE = 5;	// default 3
#endif

    m_cImgAnalysis.max_same_gain_count = 5;

	m_cImgAnalysis.diff_foreground_count = abs(m_cImgAnalysis.foreground_count - m_cImgAnalysis.pre_foreground_count);
	m_cImgAnalysis.diff_center_x = abs(m_cImgAnalysis.center_x - m_cImgAnalysis.pre_center_x);
	m_cImgAnalysis.diff_center_y = abs(m_cImgAnalysis.center_y - m_cImgAnalysis.pre_center_y);

    //	if( ForgraoundCNT < (propertyInfo->nSINGLE_FLAT_AREA_TRES>>4) )
    if (m_cImgAnalysis.foreground_count < 100)
    {
        propertyInfo->nContrastValue = m_pUsbDevInfo->nDefaultContrastValue;
        propertyInfo->nVoltageValue = __DEFAULT_VOLTAGE_VALUE__;
        propertyInfo->nExposureValue = __COLUMBO_DEFAULT_EXPOSURE_VALUE__;
        m_cImgAnalysis.frame_delay = 2;
        m_cImgAnalysis.good_frame_count = 0;
        m_cImgAnalysis.is_final = FALSE;
        m_cImgAnalysis.final_adjust_gain = FALSE;
        m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
        m_cImgAnalysis.gain_step = 0;
        m_cImgAnalysis.changed_gain_step_count = 0;
        m_cImgAnalysis.same_gain_count = 0;
        m_cImgAnalysis.pre_gain = __DEFAULT_VOLTAGE_VALUE__;
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
            !m_cImgAnalysis.is_final &&
            m_cImgAnalysis.changed_gain_step_count < MAX_STEP_CHANGE)
    {
        if (m_cImgAnalysis.mean < MIN_BRIGHT)
        {
            // enzyme 2013-01-31 modify to improve capture speed
            if (m_cImgAnalysis.changed_gain_step_count > 2/* || m_cImgAnalysis.mean > (MIN_BRIGHT-TOLERANCE)*/)
            {
                m_cImgAnalysis.frame_delay = 1;
            }

			if(propertyInfo->nExposureValue < __COLUMBO_DEFAULT_EXPOSURE_VALUE__ )
			{
				propertyInfo->nExposureValue += 0x64;
				if(propertyInfo->nExposureValue > __COLUMBO_DEFAULT_EXPOSURE_VALUE__ )
					propertyInfo->nExposureValue = __COLUMBO_DEFAULT_EXPOSURE_VALUE__;
			}
			else
			{
				if (propertyInfo->nVoltageValue >= __AUTO_VOLTAGE_MAX_VALUE__)
				{
					m_cImgAnalysis.is_complete_voltage_control = TRUE;
				}

				if (!m_cImgAnalysis.is_complete_voltage_control)
				{
					// Let's change voltage
					propertyInfo->nVoltageValue += 1;

					if (propertyInfo->nVoltageValue > __AUTO_VOLTAGE_MAX_VALUE__)
					{
						propertyInfo->nVoltageValue = __AUTO_VOLTAGE_MAX_VALUE__;
					}
				}
				else
				{
					// Let's change gain
					if (m_cImgAnalysis.mean > (MIN_BRIGHT - TOLERANCE))
					{
						// enzyme 2013-01-31 modify to improve capture speed
						//					if( frameTime < 70 )
						//						m_cImgAnalysis.frame_delay = 1;
	#if defined(WINCE)
						propertyInfo->nContrastValue += 2;
	#else
						propertyInfo->nContrastValue += 1;
	#endif
					}
					else
					{
	#if defined(__android__)
						propertyInfo->nContrastValue += 3;
	#elif defined(WINCE)
						propertyInfo->nContrastValue += 4;
	#else
						propertyInfo->nContrastValue += 2;
	#endif
					}

					propertyInfo->nContrastValue += 1;

					if (propertyInfo->nContrastValue > __AUTO_COLUMBO_CONTRAST_MAX_VALUE__)
					{
						propertyInfo->nContrastValue = __AUTO_COLUMBO_CONTRAST_MAX_VALUE__;
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

            if (propertyInfo->nVoltageValue <= __DEFAULT_VOLTAGE_VALUE__)
            {
                m_cImgAnalysis.is_complete_voltage_control = TRUE;
            }

            if (!m_cImgAnalysis.is_complete_voltage_control)
            {
                // Let's change voltage
                propertyInfo->nVoltageValue -= 1;

                if (propertyInfo->nVoltageValue < __DEFAULT_VOLTAGE_VALUE__)
                {
                    propertyInfo->nVoltageValue = __DEFAULT_VOLTAGE_VALUE__;
                }
            }
            else
            {
				// first control gain value, and then exposure
				if(propertyInfo->nContrastValue <= 0)
				{
					if(propertyInfo->nExposureValue > __COLUMBO_MINIMUM_EXPOSURE_VALUE__	)
					{
						propertyInfo->nExposureValue -= 0x64;
						if(propertyInfo->nExposureValue <= __COLUMBO_MINIMUM_EXPOSURE_VALUE__ )
							propertyInfo->nExposureValue = __COLUMBO_MINIMUM_EXPOSURE_VALUE__;
					}
				}
				else
				{
					// Let's change gain
					if (m_cImgAnalysis.mean < (MAX_BRIGHT + TOLERANCE))
					{
// enzyme 2013-01-31 modify to improve capture speed
//						if( frameTime < 70 )
//							m_cImgAnalysis.frame_delay = 1;
#if defined(WINCE)
						propertyInfo->nContrastValue -= 2;
#else
						propertyInfo->nContrastValue -= 1;
#endif
					}
					else
					{
#if defined(__android__)
						propertyInfo->nContrastValue -= 3;
#elif defined(WINCE)
						propertyInfo->nContrastValue -= 4;
#else
						propertyInfo->nContrastValue -= 2;
#endif
					}

					propertyInfo->nContrastValue -= 1;
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

    /**
     * 2013-06-11 enzyme modify - to make fast capture (Do not need this routine in the flat scanner
    	if( (m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT) || m_cImgAnalysis.final_adjust_gain )
    	{
    		if( m_cImgAnalysis.good_frame_count++ > 0 )
    		{
    			if( m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT )
    			{
    				m_cImgAnalysis.final_adjust_gain = TRUE;
    				m_cImgAnalysis.frame_delay = -1;
    			}
    		}
    	}
    */
    if ((m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT) || m_cImgAnalysis.final_adjust_gain)
    {
        m_cImgAnalysis.good_frame_count = 2;
        //		if( m_cImgAnalysis.good_frame_count++ > 0 )
        {
            if (m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT)
            {
                m_cImgAnalysis.final_adjust_gain = TRUE;
                m_cImgAnalysis.frame_delay = -1;
            }
        }
    }

    //TRACE("<====> mean=%d, gain=%d, voltage=%d, good_frame_count=%d, same_gain_count=%d, same_gain_time=%d, frame_count=%d\n",
    //		m_cImgAnalysis.mean, propertyInfo->nContrastValue, propertyInfo->nVoltageValue, m_cImgAnalysis.good_frame_count,
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
				m_cImgAnalysis.final_mean = m_cImgAnalysis.mean;
                m_cImgAnalysis.is_final = TRUE;
            }
        }
    }

    m_cImgAnalysis.pre_foreground_count = m_cImgAnalysis.foreground_count;
    m_cImgAnalysis.pre_center_x = m_cImgAnalysis.center_x;
    m_cImgAnalysis.pre_center_y = m_cImgAnalysis.center_y;

    return m_cImgAnalysis.is_final;
}

void CIBAlgorithm::_Algo_Columbo_GetDistortionMask_FromSavedFile()
{
    BOOL         isSameDM     = FALSE;
    const size_t maskElements = (IMG_SIZE * 2 + 1); // Image plus checksum. Added decimated distorition mask
    const size_t maskSize     = sizeof(DWORD) * maskElements;

    if (m_pUsbDevInfo->bNeedDistortionMask)
    {
        if (m_bDistortionMaskPath)
        {
            FILE *fp = fopen(m_cDistortionMaskPath, "rb");
            if (fp != NULL)
            {
                size_t readSize = fread(arrPos_Columbo, 1, maskSize, fp);
                fclose(fp);

                if ((readSize == maskSize) && _Algo_ConfirmChecksum(arrPos_Columbo, maskElements))
                {
                    isSameDM = TRUE;
                }
            }

            if (!isSameDM)
            {
                _Algo_Columbo_Init_Distortion_Merge_with_Bilinear();
                fp = fopen(m_cDistortionMaskPath, "wb");
                if (fp != NULL)
                {
                    _Algo_MakeChecksum(arrPos_Columbo, maskElements);
                    fwrite(arrPos_Columbo, 1, maskSize, fp);
                    fclose(fp);
                }
            }
        }
        else
        {
            _Algo_Columbo_Init_Distortion_Merge_with_Bilinear();
        }
    }

}

void CIBAlgorithm::_Algo_Process_AnalysisImage_250DPI_Columbo(unsigned char *InImg)
{
    int x, y;
    int value, y_index;
    int noise_histo[4], noise_count[4];
    int CIS_IMG_W_dev_4 = CIS_IMG_W / 2;
    int CIS_IMG_H_dev_4 = CIS_IMG_H / 2;
    int size_h = 36, size_w = 32;

    if (m_pUsbDevInfo->nDecimation_Mode == DECIMATION_4X)
    {
        CIS_IMG_W_dev_4 /= 2;
        CIS_IMG_H_dev_4 /= 2;
        size_h = 18;
        size_w = 16;
    }

    memset(noise_histo, 0, sizeof(noise_histo));

    //Initialization
    m_cImgAnalysis.noise_histo = 0;

    //Image Information Analysis
    // Noise Histo
    memset(noise_histo, 0, sizeof(int) * 4);
    memset(noise_count, 0, sizeof(int) * 4);

    for (y = 0; y < size_h; y++)
    {
        y_index = y * CIS_IMG_W_dev_4;
        for (x = 0; x < size_w; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[0] < value && value < 60)
            {
                noise_histo[0] = value;
            }
        }
        for (x = CIS_IMG_W_dev_4 - size_w; x < CIS_IMG_W_dev_4; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[1] < value && value < 60)
            {
                noise_histo[1] = value;
            }
        }
    }
    for (y = CIS_IMG_H_dev_4 - size_h; y < CIS_IMG_H_dev_4; y++)
    {
        y_index = y * CIS_IMG_W_dev_4;
        for (x = 0; x < size_w; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[2] < value && value < 60)
            {
                noise_histo[2] = value;
            }
        }
        for (x = CIS_IMG_W_dev_4 - size_w; x < CIS_IMG_W_dev_4; x++)
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

    m_cImgAnalysis.noise_histo = noise_histo[min_noise_idx];			// mean of noise image
}

void CIBAlgorithm::_Algo_RemoveNoise_250DPI_Columbo(unsigned char *InImg, unsigned char *OutImg, int ContrastLevel)
{
    int i, val;
    int CIS_IMG_W_dev_4 = CIS_IMG_W / 2;
    int CIS_IMG_H_dev_4 = CIS_IMG_H / 2;

    if (m_pUsbDevInfo->nDecimation_Mode == DECIMATION_4X)
    {
        CIS_IMG_W_dev_4 /= 2;
        CIS_IMG_H_dev_4 /= 2;
    }

    UCHAR G_NoiseTable[35] =
    {
        11, 11, 11, 11, 11, 11,	11,	// 0 ~ 6
        12, 12, 12, 12, 12, 12,	12,	12,	// 7 ~ 14
        13, 13, 13, 13, 13, 13,	13,	// 15 ~ 21
        14, 14, 14, 14, 14, 14,	// 22 ~ 27
        15, 15, // 28 ~ 29
        16, 16, // 30 ~ 31
        17, 17, // 32 ~ 33
        18	// 34
    };

    int minus_value = (G_NoiseTable[ContrastLevel] < m_cImgAnalysis.noise_histo) ?
                      G_NoiseTable[ContrastLevel] : m_cImgAnalysis.noise_histo;

    for (i = 0; i < CIS_IMG_W_dev_4 * CIS_IMG_H_dev_4; i++)
    {
        val = (int)InImg[i] - minus_value;
        if (val < 0)
        {
            val = 0;
        }
        OutImg[i] = val;//Contrast_LUT[InImg[i]];
    }
}

int CIBAlgorithm::_Algo_Columbo_GetBrightWithRawImage_250DPI_Columbo(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY)
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
    int CIS_IMG_W_dev_4 = CIS_IMG_W / 2;
    int CIS_IMG_H_dev_4 = CIS_IMG_H / 2;

    if (m_pUsbDevInfo->nDecimation_Mode == DECIMATION_4X)
    {
        CIS_IMG_W_dev_4 /= 2;
        CIS_IMG_H_dev_4 /= 2;
    }

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
        yy = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_H_dev_4 / ZOOM_H * CIS_IMG_W_dev_4;

        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            xx = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W_dev_4 / ZOOM_W;

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

	forgroundCNT = forgroundCNT*3/2;
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
            // enzyme 2013-02-04 modify -
            //			m_cImgAnalysis.LEFT=(x-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W - 30;
            m_cImgAnalysis.LEFT = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W - 70;
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
            // enzyme 2013-02-04 modify -
            //			m_cImgAnalysis.RIGHT=(x-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W + 30;
            m_cImgAnalysis.RIGHT = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W + 70;
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
            // enzyme 2013-02-04 modify -
            //			m_cImgAnalysis.TOP=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W - 30;
            m_cImgAnalysis.TOP = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W - 90;
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
            // enzyme 2013-02-04 modify -
            //			m_cImgAnalysis.BOTTOM=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W + 30;
            m_cImgAnalysis.BOTTOM = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W + 90;
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

void CIBAlgorithm::_Algo_VignettingEffect_250DPI_Columbo(unsigned char *InImg, unsigned char *OutImg)
{
    int i, j, val;
    int CIS_IMG_W_dev_4 = CIS_IMG_W / 2;
    int CIS_IMG_H_dev_4 = CIS_IMG_H / 2;
    int TOP = m_cImgAnalysis.TOP / 2;
    int BOTTOM = m_cImgAnalysis.BOTTOM / 2;
    int LEFT = m_cImgAnalysis.LEFT / 2;
    int RIGHT = m_cImgAnalysis.RIGHT / 2;
    int mag = 2;

    if (m_pUsbDevInfo->nDecimation_Mode == DECIMATION_4X)
    {
        CIS_IMG_W_dev_4 /= 2;
        CIS_IMG_H_dev_4 /= 2;
        TOP /= 2;
        BOTTOM /= 2;
        LEFT /= 2;
        RIGHT /= 2;
        mag = 4;
    }

    if (TOP <= 0)
    {
        TOP = 1;
    }
    if (BOTTOM >= CIS_IMG_H_dev_4)
    {
        BOTTOM = CIS_IMG_H_dev_4 - 1;
    }
    if (LEFT <= 0)
    {
        LEFT = 1;
    }
    if (RIGHT >= CIS_IMG_W_dev_4)
    {
        RIGHT = CIS_IMG_W_dev_4 - 1;
    }

    memset(OutImg, 0, CIS_IMG_W_dev_4 * CIS_IMG_H_dev_4);

    for (i = TOP; i <= BOTTOM; i++)
    {
        for (j = LEFT; j <= RIGHT; j++)
        {
            val = InImg[i * CIS_IMG_W_dev_4 + j];
            if (val > 0)
            {
                val = ((int)(val * m_UM_Watson_F[i * mag * CIS_IMG_W + j * mag]) >> 10) + (int)val;

                if (val > 255)
                {
                    val = 255;
                }
                else if (val < 0)
                {
                    val = 0;
                }

                OutImg[i * CIS_IMG_W_dev_4 + j] = (unsigned char)val;
            }
        }
    }
}

void CIBAlgorithm::_Algo_RemoveVignettingNoise_250DPI_Columbo(unsigned char *InImg, unsigned char *OutImg, unsigned char *TmpImg)
{
    int CIS_IMG_W_dev_4 = CIS_IMG_W / 2;
    int CIS_IMG_H_dev_4 = CIS_IMG_H / 2;
 
    memcpy(OutImg, InImg, CIS_IMG_W_dev_4 * CIS_IMG_H_dev_4);

	// To improve speed, we skip the normal image processing.
#if 0
    int i, j;
    int center, outsum;
    int BrightThres = 30;
    int MeanBright = 0;
    int TOP = m_cImgAnalysis.TOP / 2;
    int BOTTOM = m_cImgAnalysis.BOTTOM / 2;
    int LEFT = m_cImgAnalysis.LEFT / 2;
    int RIGHT = m_cImgAnalysis.RIGHT / 2;

    if (m_pUsbDevInfo->nDecimation_Mode == DECIMATION_4X)
    {
        CIS_IMG_W_dev_4 /= 2;
        CIS_IMG_H_dev_4 /= 2;
        TOP /= 2;
        BOTTOM /= 2;
        LEFT /= 2;
        RIGHT /= 2;
    }

    if (TOP <= 0)
    {
        TOP = 1;
    }
    if (BOTTOM >= CIS_IMG_H_dev_4)
    {
        BOTTOM = CIS_IMG_H_dev_4 - 1;
    }
    if (LEFT <= 0)
    {
        LEFT = 1;
    }
    if (RIGHT >= CIS_IMG_W_dev_4)
    {
        RIGHT = CIS_IMG_W_dev_4 - 1;
    }

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

    memcpy(OutImg, InImg, CIS_IMG_W_dev_4 * CIS_IMG_H_dev_4);
    for (i = TOP; i <= BOTTOM; i++)
    {
        for (j = LEFT; j <= RIGHT; j++)
        {
            center = InImg[i * CIS_IMG_W_dev_4 + j];

            if (center < MeanBright)
            {
                continue;
            }

            outsum = (InImg[(i - 1) * CIS_IMG_W_dev_4 + (j - 1)] + InImg[(i - 1) * CIS_IMG_W_dev_4 + (j)] + InImg[(i - 1) * CIS_IMG_W_dev_4 + (j + 1)] +
                      InImg[(i) * CIS_IMG_W_dev_4 + (j - 1)] + InImg[(i) * CIS_IMG_W_dev_4 + (j + 1)] +
                      InImg[(i + 1) * CIS_IMG_W_dev_4 + (j - 1)] + InImg[(i + 1) * CIS_IMG_W_dev_4 + (j)] + InImg[(i + 1) * CIS_IMG_W_dev_4 + (j + 1)]) >> 3;

            if (center > outsum + BrightThres)
            {
                OutImg[i * CIS_IMG_W_dev_4 + j] = outsum;
            }
        }
    }
#endif
}

void CIBAlgorithm::_Algo_Image_Gamma_250DPI_Columbo(unsigned char *InImg, unsigned char *OutImg, int Contrast)
{
    int i, j;
    int value;
    int CIS_IMG_W_dev_4 = CIS_IMG_W / 2;
    int CIS_IMG_H_dev_4 = CIS_IMG_H / 2;
    int TOP = m_cImgAnalysis.TOP / 2;
    int BOTTOM = m_cImgAnalysis.BOTTOM / 2;
    int LEFT = m_cImgAnalysis.LEFT / 2;
    int RIGHT = m_cImgAnalysis.RIGHT / 2;

    if (m_pUsbDevInfo->nDecimation_Mode == DECIMATION_4X)
    {
        CIS_IMG_W_dev_4 /= 2;
        CIS_IMG_H_dev_4 /= 2;
        TOP /= 2;
        BOTTOM /= 2;
        LEFT /= 2;
        RIGHT /= 2;
    }

    if (TOP <= 0)
    {
        TOP = 1;
    }
    if (BOTTOM >= CIS_IMG_H_dev_4)
    {
        BOTTOM = CIS_IMG_H_dev_4 - 1;
    }
    if (LEFT <= 0)
    {
        LEFT = 1;
    }
    if (RIGHT >= CIS_IMG_W_dev_4)
    {
        RIGHT = CIS_IMG_W_dev_4 - 1;
    }

    unsigned char *GammaTable = (unsigned char *)&G_GammaTable[Contrast * 256];

    memset(OutImg, 0, CIS_IMG_W_dev_4 * CIS_IMG_H_dev_4);
    for (i = TOP; i < BOTTOM; i++)
    {
        for (j = LEFT; j < RIGHT; j++)
        {
            value = InImg[i * CIS_IMG_W_dev_4 + j];
            if (value > 0)
            {
                OutImg[i * CIS_IMG_W_dev_4 + j] = GammaTable[value];
            }
        }
    }
}

void CIBAlgorithm::_Algo_Columbo_DistortionRestoration_with_Bilinear_250DPI_Columbo(unsigned char *InImg, unsigned char *OutImg)
{
    DWORD *arrPos_Columbo_deci_4 = &arrPos_Columbo[IMG_SIZE];

    int i, j, x, y, pos, sp, sq, val;
    int CIS_IMG_W_dev_4 = CIS_IMG_W / 2;
    int CIS_IMG_H_dev_4 = CIS_IMG_H / 2;
    int TOP = m_cImgAnalysis.TOP * IMG_H / CIS_IMG_H;
    int BOTTOM = m_cImgAnalysis.BOTTOM * IMG_H / CIS_IMG_H;
    int LEFT = m_cImgAnalysis.LEFT * IMG_W / CIS_IMG_W;
    int RIGHT = m_cImgAnalysis.RIGHT * IMG_W / CIS_IMG_W;

    if (m_pUsbDevInfo->nDecimation_Mode == DECIMATION_4X)
    {
        CIS_IMG_W_dev_4 /= 2;
        CIS_IMG_H_dev_4 /= 2;
    }

    if (TOP < 1)
    {
        TOP = 1;
    }
    else if (TOP > IMG_H - 1)
    {
        TOP = IMG_H - 1;
    }

    if (BOTTOM < 1)
    {
        BOTTOM = 1;
    }
    else if (BOTTOM > IMG_H - 1)
    {
        BOTTOM = IMG_H - 1;
    }

    if (LEFT < 1)
    {
        LEFT = 1;
    }
    else if (LEFT > IMG_W - 1)
    {
        LEFT = IMG_W - 1;
    }

    if (RIGHT < 1)
    {
        RIGHT = 1;
    }
    else if (RIGHT > IMG_W - 1)
    {
        RIGHT = IMG_W - 1;
    }

    memset(OutImg, 0, IMG_SIZE);

    if (m_pUsbDevInfo->nDecimation_Mode == DECIMATION_2X)
    {
        for (i = TOP; i <= BOTTOM; i++)
        {
            for (j = LEFT; j <= RIGHT; j++)
            {
                x = arrPos_Columbo_deci_4[i * IMG_W + j] >> 21;
                y = (arrPos_Columbo_deci_4[i * IMG_W + j] >> 10) & 0x7FF;
                sp = (arrPos_Columbo_deci_4[i * IMG_W + j] >> 5) & 0x1F;
                sq = arrPos_Columbo_deci_4[i * IMG_W + j] & 0x1F;

                pos = y * CIS_IMG_W_dev_4 + x;

                val = (sp * (sq * InImg[pos] + (32 - sq) * InImg[pos + CIS_IMG_W_dev_4]) +
                       (32 - sp) * (sq * InImg[pos + 1] + (32 - sq) * InImg[pos + CIS_IMG_W_dev_4 + 1])) >> 10;

                OutImg[i * IMG_W + j] = val;
            }
        }
    }
    else
    {
        for (i = TOP; i <= BOTTOM; i++)
        {
            for (j = LEFT; j <= RIGHT; j++)
            {
                x = arrPos_Columbo_deci_4[i * IMG_W + j] >> 21;
                y = (arrPos_Columbo_deci_4[i * IMG_W + j] >> 10) & 0x7FF;
                pos = (y + 1) / 2 * CIS_IMG_W_dev_4 + (x + 1) / 2;
                OutImg[i * IMG_W + j] = InImg[pos];
            }
        }
    }
}

