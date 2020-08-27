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

int CIBAlgorithm::_Algo_GetBrightWithRawImageEnhanced(BYTE *InRawImg, FrameImgAnalysis *imgAnalysis)
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

	BYTE *m_segment_enlarge_buffer_enhanced = new BYTE [ZOOM_ENLAGE_W*ZOOM_ENLAGE_H];
	BYTE *m_segment_enlarge_buffer_for_fingercnt_enhanced = new BYTE [ZOOM_ENLAGE_W*ZOOM_ENLAGE_H];
	BYTE *EnlargeBuf_enhanced = new BYTE [ZOOM_ENLAGE_W*ZOOM_ENLAGE_H];
	BYTE *EnlargeBuf_Org_enhanced = new BYTE [ZOOM_ENLAGE_W*ZOOM_ENLAGE_H];

    //gettimeofday(&start_tv, NULL);
    memset(m_segment_enlarge_buffer_enhanced, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
    memset(m_segment_enlarge_buffer_for_fingercnt_enhanced, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
    memset(EnlargeBuf_enhanced, 0, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);
	
	memset(imgAnalysis, 0, sizeof(FrameImgAnalysis));
	imgAnalysis->center_x = -1;
	imgAnalysis->center_y = -1;
    imgAnalysis->LEFT = 1;
    imgAnalysis->RIGHT = 1;
    imgAnalysis->TOP = 1;
    imgAnalysis->BOTTOM = 1;

    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
{
        yy = (y - ENLARGESIZE_ZOOM_H) * IMG_H / ZOOM_H * IMG_W;

        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
            {
            xx = (x - ENLARGESIZE_ZOOM_W) * IMG_W / ZOOM_W;

            value = InRawImg[yy + xx];
            EnlargeBuf_enhanced[(y)*ZOOM_ENLAGE_W + (x)] = value;
        }
    }

    memcpy(EnlargeBuf_Org_enhanced, EnlargeBuf_enhanced, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);

    _Algo_HistogramStretchForZoomEnlarge(EnlargeBuf_enhanced);

    mean = 0;
    count = 0;
    for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
        {
        for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
            {
            if (EnlargeBuf_enhanced[(i)*ZOOM_ENLAGE_W + (j)] > 10)
                {
                mean += EnlargeBuf_enhanced[(i) * ZOOM_ENLAGE_W + (j)];
                count++;
                }
            }
        }

    if (count == 0)
        {
        //gettimeofday(&tv, NULL);
        //elapsed = (tv.tv_sec - start_tv.tv_sec) + (tv.tv_usec - start_tv.tv_usec) / 1000.0;
        //printf("_Algo_GetBrightWithRawImage(): %1.4f milliseconds\n", elapsed);
		delete [] m_segment_enlarge_buffer_enhanced;
		delete [] m_segment_enlarge_buffer_for_fingercnt_enhanced;
		delete [] EnlargeBuf_enhanced;
		delete [] EnlargeBuf_Org_enhanced;
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
            tempsum  = EnlargeBuf_enhanced[(i - 1) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf_enhanced[(i - 1) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf_enhanced[(i - 1) * ZOOM_ENLAGE_W + (j + 1)];
            tempsum += EnlargeBuf_enhanced[(i) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf_enhanced[(i) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf_enhanced[(i) * ZOOM_ENLAGE_W + (j + 1)];
            tempsum += EnlargeBuf_enhanced[(i + 1) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf_enhanced[(i + 1) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf_enhanced[(i + 1) * ZOOM_ENLAGE_W + (j + 1)];

            tempsum /= 9;

            // fixed bug.
            if (tempsum >= Threshold) //mean/20)
    {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[i * ZOOM_ENLAGE_W + j] = 255;
            }
        }
    }

    for (ii = 0; ii < 1; ii++)
        {
        memcpy(m_segment_enlarge_buffer_enhanced, m_segment_enlarge_buffer_for_fingercnt_enhanced, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
        for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
            {
            for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
                {
                if (m_segment_enlarge_buffer_enhanced[i * ZOOM_ENLAGE_W + j] == 255)
                    {
                        continue;
                    }

                tempsum  = m_segment_enlarge_buffer_enhanced[(i - 1) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i - 1) * ZOOM_ENLAGE_W + j  ];
                tempsum += m_segment_enlarge_buffer_enhanced[(i - 1) * ZOOM_ENLAGE_W + j + 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i) * ZOOM_ENLAGE_W + j + 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i + 1) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i + 1) * ZOOM_ENLAGE_W + j  ];
                tempsum += m_segment_enlarge_buffer_enhanced[(i + 1) * ZOOM_ENLAGE_W + j + 1];

                if (tempsum >= 4 * 255)
                {
                    m_segment_enlarge_buffer_for_fingercnt_enhanced[i * ZOOM_ENLAGE_W + j] = 255;
                }
            }
        }
    }

    memcpy(m_segment_enlarge_buffer_enhanced, m_segment_enlarge_buffer_for_fingercnt_enhanced, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);

    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
{
        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
    {
            if (m_segment_enlarge_buffer_enhanced[(y - 1)*ZOOM_ENLAGE_W + x - 1] == 0)
        {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
        }
            else if (m_segment_enlarge_buffer_enhanced[(y - 1)*ZOOM_ENLAGE_W + x] == 0)
        {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
        }
            else if (m_segment_enlarge_buffer_enhanced[(y - 1)*ZOOM_ENLAGE_W + x + 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
    }
            else if (m_segment_enlarge_buffer_enhanced[y * ZOOM_ENLAGE_W + x - 1] == 0)
    {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[y * ZOOM_ENLAGE_W + x] == 0)
        {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
        }
            else if (m_segment_enlarge_buffer_enhanced[y * ZOOM_ENLAGE_W + x + 1] == 0)
        {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[(y + 1)*ZOOM_ENLAGE_W + x - 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[(y + 1)*ZOOM_ENLAGE_W + x] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[(y + 1)*ZOOM_ENLAGE_W + x + 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 255;
        }

            if (m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] == 255)
                {
                forgroundCNT++;
                }
            }
            }

	imgAnalysis->foreground_count = forgroundCNT;

    if (forgroundCNT < 100)
            {
		delete [] m_segment_enlarge_buffer_enhanced;
		delete [] m_segment_enlarge_buffer_for_fingercnt_enhanced;
		delete [] EnlargeBuf_enhanced;
		delete [] EnlargeBuf_Org_enhanced;
        return 0;
    }

    memcpy(m_segment_enlarge_buffer_enhanced, m_segment_enlarge_buffer_for_fingercnt_enhanced, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);

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
	for (y=ENLARGESIZE_ZOOM_H; y<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; y++)
	{
		for (x=ENLARGESIZE_ZOOM_W; x<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; x++)
		{
			if (m_segment_enlarge_buffer_enhanced[y * ZOOM_ENLAGE_W + x] == 0)
			{
				continue;
			}

			if (EnlargeBuf_Org_enhanced[(y)*ZOOM_ENLAGE_W + (x)] >= Threshold)
			{
				mean += EnlargeBuf_Org_enhanced[(y) * ZOOM_ENLAGE_W + (x)];
				count++;
				sum_x += x * ZOOM_OUT;
				sum_y += y * ZOOM_OUT;
			}
		}
	}
	if (count == 0)
	{
		delete [] m_segment_enlarge_buffer_enhanced;
		delete [] m_segment_enlarge_buffer_for_fingercnt_enhanced;
		delete [] EnlargeBuf_enhanced;
		delete [] EnlargeBuf_Org_enhanced;
		return 0;
	}

    mean /= count;
	imgAnalysis->center_x = sum_x / count;
	imgAnalysis->center_y = sum_y / count;
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
			if (EnlargeBuf_enhanced[y * ZOOM_ENLAGE_W + x] >= Threshold)
			{
				COUNT++;
			}
		}

		if (COUNT > 5)
		{
			imgAnalysis->LEFT = (x - ENLARGESIZE_ZOOM_W) * IMG_W / ZOOM_W - 30;
			//			m_cImgAnalysis.LEFT=(x-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W - 16;
			if (imgAnalysis->LEFT < 1)
			{
				imgAnalysis->LEFT = 1;
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
			if (EnlargeBuf_enhanced[y * ZOOM_ENLAGE_W + x] >= Threshold)
			{
				COUNT++;
			}
		}

		if (COUNT > 5)
		{
			imgAnalysis->RIGHT = (x - ENLARGESIZE_ZOOM_W) * IMG_W / ZOOM_W + 30;
			//			m_cImgAnalysis.RIGHT=(x-ENLARGESIZE_ZOOM_W)* CIS_IMG_W / ZOOM_W + 16;
			if (imgAnalysis->RIGHT > IMG_W - 2)
			{
				imgAnalysis->RIGHT = IMG_W - 2;
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
			if (EnlargeBuf_enhanced[y * ZOOM_ENLAGE_W + x] >= Threshold)
			{
				COUNT++;
			}
		}

		if (COUNT > 5)
		{
			imgAnalysis->TOP = (y - ENLARGESIZE_ZOOM_H) * IMG_W / ZOOM_W - 30;
			//			m_cImgAnalysis.TOP=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H - 16;
			if (imgAnalysis->TOP < 1)
			{
				imgAnalysis->TOP = 1;
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
			if (EnlargeBuf_enhanced[y * ZOOM_ENLAGE_W + x] >= Threshold)
			{
				COUNT++;
			}
		}

		if (COUNT > 5)
		{
			imgAnalysis->BOTTOM = (y - ENLARGESIZE_ZOOM_H) * IMG_W / ZOOM_W + 30;
			//			m_cImgAnalysis.BOTTOM=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H + 16;
			if (imgAnalysis->BOTTOM > IMG_H - 2)
			{
				imgAnalysis->BOTTOM = IMG_H - 2;
			}
			break;
		}
	}

	delete [] m_segment_enlarge_buffer_enhanced;
	delete [] m_segment_enlarge_buffer_for_fingercnt_enhanced;
	delete [] EnlargeBuf_enhanced;
	delete [] EnlargeBuf_Org_enhanced;

	return BrightValue;
}



int CIBAlgorithm::_Algo_ROIC_GetBrightWithRawImageEnhanced(BYTE *InRawImg, FrameImgAnalysis *imgAnalysis)
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
	int Threshold;

	BYTE *m_segment_enlarge_buffer_enhanced = new BYTE [ZOOM_ENLAGE_W * ZOOM_ENLAGE_H];
	BYTE *m_segment_enlarge_buffer_for_fingercnt_enhanced = new BYTE [ZOOM_ENLAGE_W * ZOOM_ENLAGE_H];
	BYTE *EnlargeBuf_enhanced = new BYTE [ZOOM_ENLAGE_W * ZOOM_ENLAGE_H];
	BYTE *EnlargeBuf_Org_enhanced = new BYTE [ZOOM_ENLAGE_W * ZOOM_ENLAGE_H];

    //gettimeofday(&start_tv, NULL);
    memset(m_segment_enlarge_buffer_enhanced, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
    memset(m_segment_enlarge_buffer_for_fingercnt_enhanced, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
    memset(EnlargeBuf_enhanced, 0, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);
    
	memset(imgAnalysis, 0, sizeof(FrameImgAnalysis));
	imgAnalysis->center_x = -1;
    imgAnalysis->center_y = -1;
	imgAnalysis->foreground_count = 0;
	imgAnalysis->LEFT = 1;
    imgAnalysis->RIGHT = 1;
    imgAnalysis->TOP = 1;
    imgAnalysis->BOTTOM = 1;

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
            EnlargeBuf_enhanced[(y)*ZOOM_ENLAGE_W + (x)] = value;
        }
    }

    memcpy(EnlargeBuf_Org_enhanced, EnlargeBuf_enhanced, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);

    _Algo_HistogramStretchForZoomEnlarge(EnlargeBuf_enhanced);

    mean = 0;
    count = 0;
    for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
    {
        for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
        {
            if (EnlargeBuf_enhanced[(i)*ZOOM_ENLAGE_W + (j)] > 10)
            {
                mean += EnlargeBuf_enhanced[(i) * ZOOM_ENLAGE_W + (j)];
                count++;
            }
        }
    }

    if (count == 0)
    {
        //gettimeofday(&tv, NULL);
        //elapsed = (tv.tv_sec - start_tv.tv_sec) + (tv.tv_usec - start_tv.tv_usec) / 1000.0;
        //printf("_Algo_GetBrightWithRawImage(): %1.4f milliseconds\n", elapsed);
		BrightValue = 0;
		goto done;
       // return 0;
    }

    mean /= count;

    /*	int Threshold = mean/20;
    	if(Threshold < 5)
    		Threshold = 5;
    */
    Threshold = (int)(pow((double)mean / 255.0, 3.0) * 255);
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
            tempsum  = EnlargeBuf_enhanced[(i - 1) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf_enhanced[(i - 1) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf_enhanced[(i - 1) * ZOOM_ENLAGE_W + (j + 1)];
            tempsum += EnlargeBuf_enhanced[(i) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf_enhanced[(i) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf_enhanced[(i) * ZOOM_ENLAGE_W + (j + 1)];
            tempsum += EnlargeBuf_enhanced[(i + 1) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf_enhanced[(i + 1) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf_enhanced[(i + 1) * ZOOM_ENLAGE_W + (j + 1)];

            tempsum /= 9;

            // fixed bug.
            if (tempsum >= Threshold) //mean/20)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[i * ZOOM_ENLAGE_W + j] = 255;
            }
        }
    }

    for (ii = 0; ii < 1; ii++)
    {
        memcpy(m_segment_enlarge_buffer_enhanced, m_segment_enlarge_buffer_for_fingercnt_enhanced, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
        for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
        {
            for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
            {
                if (m_segment_enlarge_buffer_enhanced[i * ZOOM_ENLAGE_W + j] == 255)
                {
                    continue;
                }

                tempsum  = m_segment_enlarge_buffer_enhanced[(i - 1) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i - 1) * ZOOM_ENLAGE_W + j  ];
                tempsum += m_segment_enlarge_buffer_enhanced[(i - 1) * ZOOM_ENLAGE_W + j + 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i) * ZOOM_ENLAGE_W + j + 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i + 1) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i + 1) * ZOOM_ENLAGE_W + j  ];
                tempsum += m_segment_enlarge_buffer_enhanced[(i + 1) * ZOOM_ENLAGE_W + j + 1];

                if (tempsum >= 4 * 255)
                {
                    m_segment_enlarge_buffer_for_fingercnt_enhanced[i * ZOOM_ENLAGE_W + j] = 255;
                }
            }
        }
    }

    memcpy(m_segment_enlarge_buffer_enhanced, m_segment_enlarge_buffer_for_fingercnt_enhanced, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);

    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            if (m_segment_enlarge_buffer_enhanced[(y - 1)*ZOOM_ENLAGE_W + x - 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[(y - 1)*ZOOM_ENLAGE_W + x] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[(y - 1)*ZOOM_ENLAGE_W + x + 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[y * ZOOM_ENLAGE_W + x - 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[y * ZOOM_ENLAGE_W + x] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[y * ZOOM_ENLAGE_W + x + 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[(y + 1)*ZOOM_ENLAGE_W + x - 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[(y + 1)*ZOOM_ENLAGE_W + x] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[(y + 1)*ZOOM_ENLAGE_W + x + 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 255;
            }

            if (m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] == 255)
            {
                forgroundCNT++;
            }
        }
    }

	imgAnalysis->foreground_count = forgroundCNT;

    if (forgroundCNT < 100)
    {
		BrightValue = 0;
		goto done;
        //return 0;
    }

    memcpy(m_segment_enlarge_buffer_enhanced, m_segment_enlarge_buffer_for_fingercnt_enhanced, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);

    mean = 0;
    count = 0;
    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            if (m_segment_enlarge_buffer_enhanced[y * ZOOM_ENLAGE_W + x] == 0)
            {
                continue;
            }

            if (EnlargeBuf_Org_enhanced[(y)*ZOOM_ENLAGE_W + (x)] >= Threshold)
            {
                mean += EnlargeBuf_Org_enhanced[(y) * ZOOM_ENLAGE_W + (x)];
                count++;
                sum_x += x * ZOOM_OUT;
                sum_y += y * ZOOM_OUT;
            }
        }
    }
    if (count == 0)
    {
		BrightValue = 0;
        goto done;//return 0;
    }

    mean /= count;
	imgAnalysis->center_x = sum_x / count;
    imgAnalysis->center_y = sum_y / count;

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
            if (EnlargeBuf_enhanced[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            imgAnalysis->LEFT = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W - 30;
            //			m_cImgAnalysis.LEFT=(x-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W - 16;
            if (imgAnalysis->LEFT < 1)
            {
                imgAnalysis->LEFT = 1;
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
            if (EnlargeBuf_enhanced[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            imgAnalysis->RIGHT = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W + 30;
            //			m_cImgAnalysis.RIGHT=(x-ENLARGESIZE_ZOOM_W)* CIS_IMG_W / ZOOM_W + 16;
            if (imgAnalysis->RIGHT > CIS_IMG_W - 2)
            {
                imgAnalysis->RIGHT = CIS_IMG_W - 2;
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
            if (EnlargeBuf_enhanced[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            imgAnalysis->TOP = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W - 30;
            //			m_cImgAnalysis.TOP=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H - 16;
            if (imgAnalysis->TOP < 1)
            {
                imgAnalysis->TOP = 1;
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
            if (EnlargeBuf_enhanced[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            imgAnalysis->BOTTOM = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W + 30;
            //			m_cImgAnalysis.BOTTOM=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H + 16;
            if (imgAnalysis->BOTTOM > CIS_IMG_H - 2)
            {
                imgAnalysis->BOTTOM = CIS_IMG_H - 2;
            }
            break;
        }
    }

done:
	delete [] m_segment_enlarge_buffer_enhanced;
	delete [] m_segment_enlarge_buffer_for_fingercnt_enhanced;
	delete [] EnlargeBuf_enhanced;
	delete [] EnlargeBuf_Org_enhanced;

    return BrightValue;	
}


int CIBAlgorithm::_Algo_Columbo_GetBrightWithRawImageEnhanced(BYTE *InRawImg, FrameImgAnalysis *imgAnalysis)
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
	int Threshold;

	BYTE *m_segment_enlarge_buffer_enhanced = new BYTE[ZOOM_ENLAGE_W * ZOOM_ENLAGE_H];
	BYTE *m_segment_enlarge_buffer_for_fingercnt_enhanced = new BYTE[ZOOM_ENLAGE_W * ZOOM_ENLAGE_H];
	BYTE *EnlargeBuf_Org_enhanced = new BYTE[ZOOM_ENLAGE_W * ZOOM_ENLAGE_H];
	BYTE *EnlargeBuf_enhanced = new BYTE[ZOOM_ENLAGE_W * ZOOM_ENLAGE_H];

    //gettimeofday(&start_tv, NULL);
    memset(m_segment_enlarge_buffer_enhanced, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
    memset(m_segment_enlarge_buffer_for_fingercnt_enhanced, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
    memset(EnlargeBuf_enhanced, 0, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);
    
	memset(imgAnalysis, 0, sizeof(FrameImgAnalysis));
	imgAnalysis->center_x = -1;
    imgAnalysis->center_y = -1;
	imgAnalysis->foreground_count = 0;
    imgAnalysis->LEFT = 1;
    imgAnalysis->RIGHT = 1;
    imgAnalysis->TOP = 1;
    imgAnalysis->BOTTOM = 1;

    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        yy = (y - ENLARGESIZE_ZOOM_H) * IMG_H / ZOOM_H * IMG_W;

        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            xx = (x - ENLARGESIZE_ZOOM_W) * IMG_W / ZOOM_W;

            value = InRawImg[yy + xx];
            EnlargeBuf_enhanced[(y)*ZOOM_ENLAGE_W + (x)] = value;
        }
    }

    memcpy(EnlargeBuf_Org_enhanced, EnlargeBuf_enhanced, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);

    _Algo_HistogramStretchForZoomEnlarge(EnlargeBuf_enhanced);

    mean = 0;
    count = 0;
    for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
    {
        for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
        {
            if (EnlargeBuf_enhanced[(i)*ZOOM_ENLAGE_W + (j)] > 10)
            {
                mean += EnlargeBuf_enhanced[(i) * ZOOM_ENLAGE_W + (j)];
                count++;
            }
        }
    }

    if (count == 0)
    {
        //gettimeofday(&tv, NULL);
        //elapsed = (tv.tv_sec - start_tv.tv_sec) + (tv.tv_usec - start_tv.tv_usec) / 1000.0;
        //printf("_Algo_GetBrightWithRawImage(): %1.4f milliseconds\n", elapsed);
        //return 0;
		BrightValue = 0;
		goto done;
    }

    mean /= count;

    /*	int Threshold = mean/20;
    	if(Threshold < 5)
    		Threshold = 5;
    */
    Threshold = (int)(pow((double)mean / 255.0, 3.0) * 255);
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
            tempsum  = EnlargeBuf_enhanced[(i - 1) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf_enhanced[(i - 1) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf_enhanced[(i - 1) * ZOOM_ENLAGE_W + (j + 1)];
            tempsum += EnlargeBuf_enhanced[(i) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf_enhanced[(i) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf_enhanced[(i) * ZOOM_ENLAGE_W + (j + 1)];
            tempsum += EnlargeBuf_enhanced[(i + 1) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf_enhanced[(i + 1) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf_enhanced[(i + 1) * ZOOM_ENLAGE_W + (j + 1)];

            tempsum /= 9;

            // fixed bug.
            if (tempsum >= Threshold) //mean/20)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[i * ZOOM_ENLAGE_W + j] = 255;
            }
        }
    }

    for (ii = 0; ii < 1; ii++)
    {
        memcpy(m_segment_enlarge_buffer_enhanced, m_segment_enlarge_buffer_for_fingercnt_enhanced, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
        for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
        {
            for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
            {
                if (m_segment_enlarge_buffer_enhanced[i * ZOOM_ENLAGE_W + j] == 255)
                {
                    continue;
                }

                tempsum  = m_segment_enlarge_buffer_enhanced[(i - 1) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i - 1) * ZOOM_ENLAGE_W + j  ];
                tempsum += m_segment_enlarge_buffer_enhanced[(i - 1) * ZOOM_ENLAGE_W + j + 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i) * ZOOM_ENLAGE_W + j + 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i + 1) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer_enhanced[(i + 1) * ZOOM_ENLAGE_W + j  ];
                tempsum += m_segment_enlarge_buffer_enhanced[(i + 1) * ZOOM_ENLAGE_W + j + 1];

                if (tempsum >= 4 * 255)
                {
                    m_segment_enlarge_buffer_for_fingercnt_enhanced[i * ZOOM_ENLAGE_W + j] = 255;
                }
            }
        }
    }

    memcpy(m_segment_enlarge_buffer_enhanced, m_segment_enlarge_buffer_for_fingercnt_enhanced, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);

    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            if (m_segment_enlarge_buffer_enhanced[(y - 1)*ZOOM_ENLAGE_W + x - 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[(y - 1)*ZOOM_ENLAGE_W + x] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[(y - 1)*ZOOM_ENLAGE_W + x + 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[y * ZOOM_ENLAGE_W + x - 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[y * ZOOM_ENLAGE_W + x] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[y * ZOOM_ENLAGE_W + x + 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[(y + 1)*ZOOM_ENLAGE_W + x - 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[(y + 1)*ZOOM_ENLAGE_W + x] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer_enhanced[(y + 1)*ZOOM_ENLAGE_W + x + 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else
            {
                m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] = 255;
            }

            if (m_segment_enlarge_buffer_for_fingercnt_enhanced[y * ZOOM_ENLAGE_W + x] == 255)
            {
                forgroundCNT++;
            }
        }
    }

	forgroundCNT = forgroundCNT*3/2;
	imgAnalysis->foreground_count = forgroundCNT;

    if (forgroundCNT < 100)
    {
        return 0;
    }

    memcpy(m_segment_enlarge_buffer_enhanced, m_segment_enlarge_buffer_for_fingercnt_enhanced, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);

    mean = 0;
    count = 0;
    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            if (m_segment_enlarge_buffer_enhanced[y * ZOOM_ENLAGE_W + x] == 0)
            {
                continue;
            }

            if (EnlargeBuf_Org_enhanced[(y)*ZOOM_ENLAGE_W + (x)] >= Threshold)
            {
                mean += EnlargeBuf_Org_enhanced[(y) * ZOOM_ENLAGE_W + (x)];
                count++;
                sum_x += x * ZOOM_OUT;
                sum_y += y * ZOOM_OUT;
            }
        }
    }
    if (count == 0)
    {
		BrightValue = 0;
        //return 0;
		goto done;
    }

    mean /= count;
	imgAnalysis->center_x = sum_x / count;
    imgAnalysis->center_y = sum_y / count;

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
            if (EnlargeBuf_enhanced[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            // enzyme 2013-02-04 modify -
            //			m_cImgAnalysis.LEFT=(x-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W - 30;
            imgAnalysis->LEFT = (x - ENLARGESIZE_ZOOM_W) * IMG_W / ZOOM_W - 70;
            //			m_cImgAnalysis.LEFT=(x-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W - 16;
            if (imgAnalysis->LEFT < 1)
            {
                imgAnalysis->LEFT = 1;
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
            if (EnlargeBuf_enhanced[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            // enzyme 2013-02-04 modify -
            //			m_cImgAnalysis.RIGHT=(x-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W + 30;
            imgAnalysis->RIGHT = (x - ENLARGESIZE_ZOOM_W) * IMG_W / ZOOM_W + 70;
            //			m_cImgAnalysis.RIGHT=(x-ENLARGESIZE_ZOOM_W)* CIS_IMG_W / ZOOM_W + 16;
            if (imgAnalysis->RIGHT > IMG_W - 2)
            {
                imgAnalysis->RIGHT = IMG_W - 2;
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
            if (EnlargeBuf_enhanced[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            // enzyme 2013-02-04 modify -
            //			m_cImgAnalysis.TOP=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W - 30;
            imgAnalysis->TOP = (y - ENLARGESIZE_ZOOM_H) * IMG_W / ZOOM_W - 90;
            //			m_cImgAnalysis.TOP=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H - 16;
            if (imgAnalysis->TOP < 1)
            {
                imgAnalysis->TOP = 1;
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
            if (EnlargeBuf_enhanced[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            // enzyme 2013-02-04 modify -
            //			m_cImgAnalysis.BOTTOM=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W + 30;
            imgAnalysis->BOTTOM = (y - ENLARGESIZE_ZOOM_H) * IMG_W / ZOOM_W + 90;
            //			m_cImgAnalysis.BOTTOM=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H + 16;
            if (imgAnalysis->BOTTOM > IMG_H - 2)
            {
                imgAnalysis->BOTTOM = IMG_H - 2;
            }
            break;
        }
    }

done:

	delete [] m_segment_enlarge_buffer_enhanced;
	delete [] m_segment_enlarge_buffer_for_fingercnt_enhanced;
	delete [] EnlargeBuf_enhanced;
	delete [] EnlargeBuf_Org_enhanced;

    return BrightValue;
}


void CIBAlgorithm::_Algo_Fast_Distortion(unsigned char *InImg, unsigned char *OutImg)
{
    int value;
    int i, j, x, y, pos, sp, sq;
    int a, b, c, d;
    int LEFT = m_cImgAnalysis.LEFT;
    int RIGHT = m_cImgAnalysis.RIGHT;
    int TOP = m_cImgAnalysis.TOP;
    int BOTTOM = m_cImgAnalysis.BOTTOM;
    DWORD disVal;
    int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;


    // 실제 사용되는 영역만 distortion restoration을 한다.
#ifdef __IBSCAN_ULTIMATE_SDK__
    memset(OutImg, 255, IMG_SIZE);
#else
    memset(OutImg, 0, IMG_SIZE);		// For IBScanSDK (background is black)
#endif
    dtLEFT		= LEFT * IMG_W / CIS_IMG_W;
    dtRIGHT		= RIGHT * IMG_W / CIS_IMG_W;
    dtTOP		= TOP * IMG_H / CIS_IMG_H;
    dtBOTTOM	= BOTTOM * IMG_H / CIS_IMG_H;

    for (i = 0; i < IMG_HALF_H; i++)
    {
        if (dtTOP > i && dtBOTTOM < (IMG_H - 1 - i))
        {
            continue;
        }

        for (j = 0; j < IMG_HALF_W; j++)
        {
            if (dtLEFT > j && dtRIGHT < (IMG_W - 1 - j))
            {
                continue;
            }

            disVal = arrPos_Watson[i * IMG_W + j];
            x	= disVal >> 21;
            y	= (disVal >> 10) & 0x7FF;
            sp	= (disVal >> 5) & 0x1F;
            sq	= disVal & 0x1F;
            a	= sp * sq;
            b	= (32 - sp) * sq;
            c	= sp * (32 - sq);
            d	= (32 - sp) * (32 - sq);

            if (LEFT <= x && RIGHT >= x && TOP <= y && BOTTOM >= y)
            {
                pos = y * CIS_IMG_W + x;
                if (InImg[pos] > 0)
                {
                    value = (a * InImg[pos] + b * InImg[pos + 1] + c * InImg[pos + CIS_IMG_W] + d * InImg[pos + CIS_IMG_W + 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                    OutImg[i * IMG_W + j] = 255 - (unsigned char)value;
#else
                    OutImg[i * IMG_W + j] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                }
            }

            if (LEFT <= (CIS_IMG_W - 1 - x) && RIGHT >= (CIS_IMG_W - 1 - x) && TOP <= y && BOTTOM >= y)
            {
                pos = y * CIS_IMG_W + (CIS_IMG_W - 1 - x);
                if (InImg[pos] > 0)
                {
                    value = (a * InImg[pos] + b * InImg[pos - 1] + c * InImg[pos + CIS_IMG_W] + d * InImg[pos + CIS_IMG_W - 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                    OutImg[i * IMG_W + (IMG_W - 1 - j)] = 255 - (unsigned char)value;
#else
                    OutImg[i * IMG_W + (IMG_W - 1 - j)] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                }
            }

            if (LEFT <= x && RIGHT >= x && TOP <= (CIS_IMG_H - 1 - y) && BOTTOM >= (CIS_IMG_H - 1 - y))
            {
                pos = (CIS_IMG_H - 1 - y) * CIS_IMG_W + x;
                if (InImg[pos] > 0)
                {
                    value = (a * InImg[pos] + b * InImg[pos + 1] + c * InImg[pos - CIS_IMG_W] + d * InImg[pos - CIS_IMG_W + 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                    OutImg[(IMG_H - 1 - i)*IMG_W + j] = 255 - (unsigned char)value;
#else
                    OutImg[(IMG_H - 1 - i)*IMG_W + j] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                }
            }

            if (LEFT <= (CIS_IMG_W - 1 - x) && RIGHT >= (CIS_IMG_W - 1 - x) && TOP <= (CIS_IMG_H - 1 - y) && BOTTOM >= (CIS_IMG_H - 1 - y))
            {
                pos = (CIS_IMG_H - 1 - y) * CIS_IMG_W + (CIS_IMG_W - 1 - x);
                if (InImg[pos] > 0)
                {
                    value = (a * InImg[pos] + b * InImg[pos - 1] + c * InImg[pos - CIS_IMG_W] + d * InImg[pos - CIS_IMG_W - 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                    OutImg[(IMG_H - 1 - i)*IMG_W + (IMG_W - 1 - j)] = 255 - (unsigned char)value;
#else
                    OutImg[(IMG_H - 1 - i)*IMG_W + (IMG_W - 1 - j)] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                }
            }
        }
    }
}


void CIBAlgorithm::_Algo_Fast_Distortion_Columbo(unsigned char *InImg, unsigned char *OutImg)
{
    int value;
    int i, j, x, y, pos, sp, sq;
    int a, b, c, d;
    int LEFT = m_cImgAnalysis.LEFT;
    int RIGHT = m_cImgAnalysis.RIGHT;
    int TOP = m_cImgAnalysis.TOP;
    int BOTTOM = m_cImgAnalysis.BOTTOM;
    DWORD disVal;
    int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;


    // 실제 사용되는 영역만 distortion restoration을 한다.
#ifdef __IBSCAN_ULTIMATE_SDK__
    memset(OutImg, 255, IMG_SIZE);
#else
    memset(OutImg, 0, IMG_SIZE);		// For IBScanSDK (background is black)
#endif
    dtLEFT		= LEFT * IMG_W / CIS_IMG_W;
    dtRIGHT		= RIGHT * IMG_W / CIS_IMG_W;
    dtTOP		= TOP * IMG_H / CIS_IMG_H;
    dtBOTTOM	= BOTTOM * IMG_H / CIS_IMG_H;

    for (i = 0; i < IMG_HALF_H; i++)
    {
        if (dtTOP > i && dtBOTTOM < (IMG_H - 1 - i))
        {
            continue;
        }

        for (j = 0; j < IMG_HALF_W; j++)
        {
            if (dtLEFT > j && dtRIGHT < (IMG_W - 1 - j))
            {
                continue;
            }

            disVal = arrPos_Columbo[i * IMG_W + j];
            x	= disVal >> 21;
            y	= (disVal >> 10) & 0x7FF;
            sp	= (disVal >> 5) & 0x1F;
            sq	= disVal & 0x1F;
            a	= sp * sq;
            b	= (32 - sp) * sq;
            c	= sp * (32 - sq);
            d	= (32 - sp) * (32 - sq);

            if (LEFT <= x && RIGHT >= x && TOP <= y && BOTTOM >= y)
            {
                pos = y * CIS_IMG_W + x;
                if (InImg[pos] > 0)
                {
                    value = (a * InImg[pos] + b * InImg[pos + 1] + c * InImg[pos + CIS_IMG_W] + d * InImg[pos + CIS_IMG_W + 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                    OutImg[i * IMG_W + j] = 255 - (unsigned char)value;
#else
                    OutImg[i * IMG_W + j] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                }
            }

            if (LEFT <= (CIS_IMG_W - 1 - x) && RIGHT >= (CIS_IMG_W - 1 - x) && TOP <= y && BOTTOM >= y)
            {
                pos = y * CIS_IMG_W + (CIS_IMG_W - 1 - x);
                if (InImg[pos] > 0)
                {
                    value = (a * InImg[pos] + b * InImg[pos - 1] + c * InImg[pos + CIS_IMG_W] + d * InImg[pos + CIS_IMG_W - 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                    OutImg[i * IMG_W + (IMG_W - 1 - j)] = 255 - (unsigned char)value;
#else
                    OutImg[i * IMG_W + (IMG_W - 1 - j)] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                }
            }

            if (LEFT <= x && RIGHT >= x && TOP <= (CIS_IMG_H - 1 - y) && BOTTOM >= (CIS_IMG_H - 1 - y))
            {
                pos = (CIS_IMG_H - 1 - y) * CIS_IMG_W + x;
                if (InImg[pos] > 0)
                {
                    value = (a * InImg[pos] + b * InImg[pos + 1] + c * InImg[pos - CIS_IMG_W] + d * InImg[pos - CIS_IMG_W + 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                    OutImg[(IMG_H - 1 - i)*IMG_W + j] = 255 - (unsigned char)value;
#else
                    OutImg[(IMG_H - 1 - i)*IMG_W + j] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                }
            }

            if (LEFT <= (CIS_IMG_W - 1 - x) && RIGHT >= (CIS_IMG_W - 1 - x) && TOP <= (CIS_IMG_H - 1 - y) && BOTTOM >= (CIS_IMG_H - 1 - y))
            {
                pos = (CIS_IMG_H - 1 - y) * CIS_IMG_W + (CIS_IMG_W - 1 - x);
                if (InImg[pos] > 0)
                {
                    value = (a * InImg[pos] + b * InImg[pos - 1] + c * InImg[pos - CIS_IMG_W] + d * InImg[pos - CIS_IMG_W - 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                    OutImg[(IMG_H - 1 - i)*IMG_W + (IMG_W - 1 - j)] = 255 - (unsigned char)value;
#else
                    OutImg[(IMG_H - 1 - i)*IMG_W + (IMG_W - 1 - j)] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                }
            }
        }
    }
}



void CIBAlgorithm::_Algo_Fast_Distortion_250DPI_Columbo(unsigned char *InImg, unsigned char *OutImg)
{
    DWORD *arrPos_Columbo_deci_4 = &arrPos_Columbo[IMG_SIZE];

    int value;
    int i, j, x, y, pos, sp, sq;
    int a, b, c, d;
    DWORD disVal;
    int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

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

    // 실제 사용되는 영역만 distortion restoration을 한다.
#ifdef __IBSCAN_ULTIMATE_SDK__
    memset(OutImg, 255, IMG_SIZE);
#else
    memset(OutImg, 0, IMG_SIZE);		// For IBScanSDK (background is black)
#endif
    dtLEFT		= m_cImgAnalysis.LEFT * IMG_W / CIS_IMG_W;
    dtRIGHT		= m_cImgAnalysis.RIGHT * IMG_W / CIS_IMG_W;
    dtTOP		= m_cImgAnalysis.TOP * IMG_H / CIS_IMG_H;
    dtBOTTOM	= m_cImgAnalysis.BOTTOM * IMG_H / CIS_IMG_H;

    if (m_pUsbDevInfo->nDecimation_Mode == DECIMATION_4X)
    {
        for (i = 0; i < IMG_HALF_H; i++)
        {
            if (dtTOP > i && dtBOTTOM < (IMG_H - 1 - i))
            {
                continue;
            }

            for (j = 0; j < IMG_HALF_W; j++)
            {
                if (dtLEFT > j && dtRIGHT < (IMG_W - 1 - j))
                {
                    continue;
                }

                disVal = arrPos_Columbo_deci_4[i * IMG_W + j];
                x	= (disVal >> 21) >> 1;
                y	= ((disVal >> 10) & 0x7FF) >> 1;
                sp	= (disVal >> 5) & 0x1F;
                sq	= disVal & 0x1F;
                a	= sp * sq;
                b	= (32 - sp) * sq;
                c	= sp * (32 - sq);
                d	= (32 - sp) * (32 - sq);

                if (LEFT <= x && RIGHT >= x && TOP <= y && BOTTOM >= y)
                {
                    pos = y * CIS_IMG_W_dev_4 + x;
                    if (InImg[pos] > 0)
                    {
                        value = (a * InImg[pos] + b * InImg[pos + 1] + c * InImg[pos + CIS_IMG_W_dev_4] + d * InImg[pos + CIS_IMG_W_dev_4 + 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                        OutImg[i * IMG_W + j] = 255 - (unsigned char)value;
#else
                        OutImg[i * IMG_W + j] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                    }
                }

                if (LEFT <= (CIS_IMG_W_dev_4 - 1 - x) && RIGHT >= (CIS_IMG_W_dev_4 - 1 - x) && TOP <= y && BOTTOM >= y)
                {
                    pos = y * CIS_IMG_W_dev_4 + (CIS_IMG_W_dev_4 - 1 - x);
                    if (InImg[pos] > 0)
                    {
                        value = (a * InImg[pos] + b * InImg[pos - 1] + c * InImg[pos + CIS_IMG_W_dev_4] + d * InImg[pos + CIS_IMG_W_dev_4 - 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                        OutImg[i * IMG_W + (IMG_W - 1 - j)] = 255 - (unsigned char)value;
#else
                        OutImg[i * IMG_W + (IMG_W - 1 - j)] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                    }
                }

                if (LEFT <= x && RIGHT >= x && TOP <= (CIS_IMG_H_dev_4 - 1 - y) && BOTTOM >= (CIS_IMG_H_dev_4 - 1 - y))
                {
                    pos = (CIS_IMG_H_dev_4 - 1 - y) * CIS_IMG_W_dev_4 + x;
                    if (InImg[pos] > 0)
                    {
                        value = (a * InImg[pos] + b * InImg[pos + 1] + c * InImg[pos - CIS_IMG_W_dev_4] + d * InImg[pos - CIS_IMG_W_dev_4 + 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                        OutImg[(IMG_H - 1 - i)*IMG_W + j] = 255 - (unsigned char)value;
#else
                        OutImg[(IMG_H - 1 - i)*IMG_W + j] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                    }
                }

                if (LEFT <= (CIS_IMG_W_dev_4 - 1 - x) && RIGHT >= (CIS_IMG_W_dev_4 - 1 - x) && TOP <= (CIS_IMG_H_dev_4 - 1 - y) && BOTTOM >= (CIS_IMG_H_dev_4 - 1 - y))
                {
                    pos = (CIS_IMG_H_dev_4 - 1 - y) * CIS_IMG_W_dev_4 + (CIS_IMG_W_dev_4 - 1 - x);
                    if (InImg[pos] > 0)
                    {
                        value = (a * InImg[pos] + b * InImg[pos - 1] + c * InImg[pos - CIS_IMG_W_dev_4] + d * InImg[pos - CIS_IMG_W_dev_4 - 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                        OutImg[(IMG_H - 1 - i)*IMG_W + (IMG_W - 1 - j)] = 255 - (unsigned char)value;
#else
                        OutImg[(IMG_H - 1 - i)*IMG_W + (IMG_W - 1 - j)] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                    }
                }
            }
        }
    }
    else
    {
        for (i = 0; i < IMG_HALF_H; i++)
        {
            if (dtTOP > i && dtBOTTOM < (IMG_H - 1 - i))
            {
                continue;
            }

            for (j = 0; j < IMG_HALF_W; j++)
            {
                if (dtLEFT > j && dtRIGHT < (IMG_W - 1 - j))
                {
                    continue;
                }

                disVal = arrPos_Columbo_deci_4[i * IMG_W + j];
                x	= disVal >> 21;
                y	= (disVal >> 10) & 0x7FF;
                sp	= (disVal >> 5) & 0x1F;
                sq	= disVal & 0x1F;
                a	= sp * sq;
                b	= (32 - sp) * sq;
                c	= sp * (32 - sq);
                d	= (32 - sp) * (32 - sq);

                if (LEFT <= x && RIGHT >= x && TOP <= y && BOTTOM >= y)
                {
                    pos = y * CIS_IMG_W_dev_4 + x;
                    if (InImg[pos] > 0)
                    {
                        value = (a * InImg[pos] + b * InImg[pos + 1] + c * InImg[pos + CIS_IMG_W_dev_4] + d * InImg[pos + CIS_IMG_W_dev_4 + 1]) >> 10;

 #ifdef __IBSCAN_ULTIMATE_SDK__
                        OutImg[i * IMG_W + j] = 255 - (unsigned char)value;
#else
                        OutImg[i * IMG_W + j] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                    }
                }

                if (LEFT <= (CIS_IMG_W_dev_4 - 1 - x) && RIGHT >= (CIS_IMG_W_dev_4 - 1 - x) && TOP <= y && BOTTOM >= y)
                {
                    pos = y * CIS_IMG_W_dev_4 + (CIS_IMG_W_dev_4 - 1 - x);
                    if (InImg[pos] > 0)
                    {
                        value = (a * InImg[pos] + b * InImg[pos - 1] + c * InImg[pos + CIS_IMG_W_dev_4] + d * InImg[pos + CIS_IMG_W_dev_4 - 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                        OutImg[i * IMG_W + (IMG_W - 1 - j)] = 255 - (unsigned char)value;
#else
                        OutImg[i * IMG_W + (IMG_W - 1 - j)] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                    }
                }

                if (LEFT <= x && RIGHT >= x && TOP <= (CIS_IMG_H_dev_4 - 1 - y) && BOTTOM >= (CIS_IMG_H_dev_4 - 1 - y))
                {
                    pos = (CIS_IMG_H_dev_4 - 1 - y) * CIS_IMG_W_dev_4 + x;
                    if (InImg[pos] > 0)
                    {
                        value = (a * InImg[pos] + b * InImg[pos + 1] + c * InImg[pos - CIS_IMG_W_dev_4] + d * InImg[pos - CIS_IMG_W_dev_4 + 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                        OutImg[(IMG_H - 1 - i)*IMG_W + j] = 255 - (unsigned char)value;
#else
                        OutImg[(IMG_H - 1 - i)*IMG_W + j] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                    }
                }

                if (LEFT <= (CIS_IMG_W_dev_4 - 1 - x) && RIGHT >= (CIS_IMG_W_dev_4 - 1 - x) && TOP <= (CIS_IMG_H_dev_4 - 1 - y) && BOTTOM >= (CIS_IMG_H_dev_4 - 1 - y))
                {
                    pos = (CIS_IMG_H_dev_4 - 1 - y) * CIS_IMG_W_dev_4 + (CIS_IMG_W_dev_4 - 1 - x);
                    if (InImg[pos] > 0)
                    {
                        value = (a * InImg[pos] + b * InImg[pos - 1] + c * InImg[pos - CIS_IMG_W_dev_4] + d * InImg[pos - CIS_IMG_W_dev_4 - 1]) >> 10;

#ifdef __IBSCAN_ULTIMATE_SDK__
                        OutImg[(IMG_H - 1 - i)*IMG_W + (IMG_W - 1 - j)] = 255 - (unsigned char)value;
#else
                        OutImg[(IMG_H - 1 - i)*IMG_W + (IMG_W - 1 - j)] = (unsigned char)value;		// For IBScanSDK (background is black)
#endif
                    }
                }
            }
        }
    }
}


void CIBAlgorithm::_Algo_VignettingEffectEnhanced(unsigned char *InImg, unsigned char *OutImg, FrameImgAnalysis imgAnalysis)
{
    int i, j, val;

    memset(OutImg, 0, IMG_H * IMG_W);

    for (i = imgAnalysis.TOP; i <= imgAnalysis.BOTTOM; i++)
    {
        for (j = imgAnalysis.LEFT; j <= imgAnalysis.RIGHT; j++)
        {
            val = InImg[i * IMG_W + j];
            if (val > 0)
            {
                val = ((int)(val * m_UM_Watson_F_Enhanced[i * IMG_W + j]) >> 10) + (int)val;

                if (val > 255)
                {
                    val = 255;
                }
                else if (val < 0)
                {
                    val = 0;
                }

                OutImg[i * IMG_W + j] = (unsigned char)val;
            }
        }
    }
}


int CIBAlgorithm::_Algo_FindExpectContrastEnhanced(unsigned char *InImg, unsigned char *TmpImg, int imgSize, int contrastTres, int nGammaLevel, FrameImgAnalysis imgAnalysis)
{
    double		diff_val = 0;
    int			expect_contrast = 0;
    int			mean_sum = 0;
    int			targetBright;
    int			Bright_Before[48];
    int			start_level, end_level;

    targetBright = 90 + contrastTres * 7;

    start_level = 10 + contrastTres, end_level = 20 + contrastTres;
    for (int loop_count = start_level; loop_count <= end_level; loop_count += (end_level - start_level))
    {
        _Algo_Image_GammaEnhanced(InImg, TmpImg, loop_count, imgAnalysis);
        // 밝기 계산
        Bright_Before[loop_count] = _Algo_GetBright_Simple(TmpImg, imgSize, 10);
    }

    diff_val = (Bright_Before[end_level] - Bright_Before[start_level]) / 10.0;
    if (diff_val != 0)
    {
        expect_contrast = 20 + (int)((targetBright - Bright_Before[end_level]) / (double)diff_val + 0.5);
    }
    else
    {
        expect_contrast = nGammaLevel;
    }
    if (expect_contrast > 47)
    {
        expect_contrast = 47;
    }
    else if (expect_contrast < 0)
    {
        expect_contrast = 0;
    }

    _Algo_Image_GammaEnhanced(InImg, TmpImg, expect_contrast, imgAnalysis);
    // 밝기 계산
    mean_sum = _Algo_GetBright_Simple(TmpImg, imgSize, 10);

    if (expect_contrast < start_level)
    {
        diff_val = (Bright_Before[end_level] - mean_sum) / 10.0;
    }

    if (diff_val != 0)
    {
        expect_contrast += (int)((targetBright - mean_sum) / (double)diff_val + 0.5);
    }
    else
    {
        expect_contrast = nGammaLevel;
    }
    if (expect_contrast > 47)
    {
        expect_contrast = 47;
    }
    else if (expect_contrast < 0)
    {
        expect_contrast = 0;
    }

    return expect_contrast;
}


void CIBAlgorithm::_Algo_Image_GammaEnhanced(unsigned char *InImg, unsigned char *OutImg, int Contrast, FrameImgAnalysis imgAnalysis)
{
    int i, j;
    int value;
    unsigned char *GammaTable = (unsigned char *)&G_GammaTable[Contrast * 256];

    memset(OutImg, 0, IMG_SIZE);
    for (i = imgAnalysis.TOP; i < imgAnalysis.BOTTOM; i++)
    {
        for (j = imgAnalysis.LEFT; j < imgAnalysis.RIGHT; j++)
        {
            value = InImg[i * IMG_W + j];
            if (value > 0)
            {
                OutImg[i * IMG_W + j] = GammaTable[value];
            }
        }
    }
}


void CIBAlgorithm::_Algo_RemoveVignettingNoiseEnhanced(unsigned char *InImg, unsigned char *OutImg, unsigned char *TmpImg, FrameImgAnalysis imgAnalysis)
{
    int i, j;
    int center, outsum;
    int BrightThres = 30;
    int MeanBright = 0;

    MeanBright = imgAnalysis.mean;

    BrightThres = BrightThres * MeanBright / 128;
    if (BrightThres > 30)
    {
        BrightThres = 30;
    }
    else if (BrightThres < 10)
    {
        BrightThres = 10;
    }

    memcpy(OutImg, InImg, IMG_H * IMG_W);
    for (i = imgAnalysis.TOP; i <= imgAnalysis.BOTTOM; i++)
    {
        for (j = imgAnalysis.LEFT; j <= imgAnalysis.RIGHT; j++)
        {
            center = InImg[i * IMG_W + j];

            if (center < MeanBright)
        {
            continue;
        }

            outsum = (InImg[(i - 1) * IMG_W + (j - 1)] + InImg[(i - 1) * IMG_W + (j)] + InImg[(i - 1) * IMG_W + (j + 1)] +
                      InImg[(i) * IMG_W + (j - 1)] + InImg[(i) * IMG_W + (j + 1)] +
                      InImg[(i + 1) * IMG_W + (j - 1)] + InImg[(i + 1) * IMG_W + (j)] + InImg[(i + 1) * IMG_W + (j + 1)]) >> 3;

            if (center > outsum + BrightThres)
        {
                OutImg[i * IMG_W + j] = outsum;
            }
        }
    }
}


void CIBAlgorithm::_Algo_HistogramNormalizeEnhanced(unsigned char *InImg, unsigned char *OutImg, int imgSize, int contrastTres, FrameImgAnalysis imgAnalysis)
{
    int y, gray = 0;
    int histogram[256];
    int LUT[256], value;
    //	double scale_factor;

    // enzyme add 2012-11-29 for blank image processing
    if (imgAnalysis.LEFT == imgAnalysis.RIGHT && imgAnalysis.TOP == imgAnalysis.BOTTOM)
    {
        memcpy(OutImg, InImg, imgSize);
        return;
    }

    memset(histogram, 0, sizeof(histogram));
    memset(LUT, 0, 256 * sizeof(int));
    for (y = 0; y < imgSize; y++)
    {
        gray = InImg[y];
        histogram[gray]++;
    }

    int maxidx = 0, maxsum = 0;
    for (y = 255; y >= 0; y--)
    {
        maxsum += histogram[y];
        if (maxsum > 100)
        {
            maxidx = y;
            break;
        }
    }

    int Thres = 230 + contrastTres * 5;
    // enzyme modify 2012-11-29 for blank image processing
    // 2013-43-03 gon modify : low bright image doesn't have to enhance.
    if (maxidx > 10 && maxidx < Thres)
    {
        for (y = maxidx; y < 256; y++)
        {
            LUT[y] = Thres;
        }

        for (y = 0; y < maxidx; y++)
        {
            value = (int)(y * Thres / maxidx);
            if (value > 255)
            {
                value = 255;
            }
            LUT[y] = value;
        }
    }
    else
    {
        memcpy(OutImg, InImg, imgSize);
        return;
    }

    for (y = 0; y < imgSize; y++)
    {
        OutImg[y] = LUT[InImg[y]];
    }
}


void CIBAlgorithm::_Algo_SwUniformityEnhanced(unsigned char *InImg, unsigned char *OutImg, unsigned char *TmpOrigin, int width, int height, FrameImgAnalysis imgAnalysis)
{
    int LEFT = imgAnalysis.LEFT;
    int RIGHT = imgAnalysis.RIGHT;
    int TOP = imgAnalysis.TOP;
    int BOTTOM = imgAnalysis.BOTTOM;

    int i, j, loop;
    int image_mean_val, val, mean_val, weight, count;
    int temp1, temp2, max_val = -100000000, min_val = 100000000;

    // enzyme add 2012-11-29 for blank image processing
    if (imgAnalysis.LEFT == imgAnalysis.RIGHT && imgAnalysis.TOP == imgAnalysis.BOTTOM)
    {
        memcpy(OutImg, InImg, IMG_SIZE);
        return;
    }

    if (LEFT < 1)
    {
        LEFT = 1;
    }
    if (TOP < 1)
    {
        TOP = 1;
    }
    if (RIGHT > IMG_W - 2)
    {
        RIGHT = IMG_W - 2;
    }
    if (BOTTOM > IMG_H - 2)
    {
        BOTTOM = IMG_H - 2;
    }

    memset(TmpOrigin, 255, IMG_SIZE);

    for (i = TOP; i <= BOTTOM; i++)
    {
        for (j = LEFT; j <= RIGHT; j++)
        {
            TmpOrigin[i * IMG_W + j] = 255 - InImg[i * IMG_W + j];
        }
    }
    memcpy(OutImg, TmpOrigin, IMG_SIZE);

    image_mean_val = 0;
    count = 0;
    for (i = TOP; i <= BOTTOM; i++)
    {
        for (j = LEFT; j <= RIGHT; j++)
        {
            if (TmpOrigin[i * IMG_W + j] < 245)
            {
                image_mean_val += TmpOrigin[i * IMG_W + j];
                count++;
            }
        }
    }
    if (count > 0)
    {
        image_mean_val = image_mean_val / count;
    }

    // calc magnitude
    memset(m_MagBuffer_short, 0, IMG_SIZE * sizeof(USHORT));
    for (i = TOP; i <= BOTTOM; i++)
    {
        for (j = LEFT; j <= RIGHT; j++)
        {
            temp1 = - InImg[(i - 1) * IMG_W + (j - 1)] + InImg[(i - 1) * IMG_W + (j + 1)]
                    - InImg[(i) * IMG_W + (j - 1)] + InImg[(i) * IMG_W + (j + 1)]
                    - InImg[(i + 1) * IMG_W + (j - 1)] + InImg[(i + 1) * IMG_W + (j + 1)];

            temp2 = - InImg[(i - 1) * IMG_W + (j - 1)] - InImg[(i - 1) * IMG_W + (j)] - InImg[(i - 1) * IMG_W + (j + 1)]
                    + InImg[(i + 1) * IMG_W + (j - 1)] + InImg[(i + 1) * IMG_W + (j)] + InImg[(i + 1) * IMG_W + (j + 1)];

            val = temp1 * temp1 + temp2 * temp2;

            if (max_val < val)
            {
                max_val = val;
            }
            if (min_val > val)
            {
                min_val = val;
            }

            m_MagBuffer_short[i * IMG_W + j] = val;
        }
    }

    if ((max_val - min_val) == 0)
    {
        memcpy(OutImg, InImg, IMG_SIZE);
        return;
    }

    memset(m_MagBuffer, 0, IMG_SIZE);

    for (i = TOP; i <= BOTTOM; i++)
    {
        for (j = LEFT; j <= RIGHT; j++)
        {
            m_MagBuffer[i * IMG_W + j] = (unsigned char)((m_MagBuffer_short[i * IMG_W + j] - min_val) * 63 / (max_val - min_val));
            m_MagBuffer[i * IMG_W + j] <<= 2;
        }
    }

    // remove white noise
    for (loop = 0; loop < 1; loop++)
    {
        for (i = TOP; i <= BOTTOM; i++)
        {
            for (j = LEFT; j <= RIGHT; j++)
            {
                mean_val = (TmpOrigin[(i - 1) * IMG_W + (j - 1)] +
                            TmpOrigin[(i - 1) * IMG_W + j] +
                            TmpOrigin[(i - 1) * IMG_W + (j + 1)] +
                            TmpOrigin[(i) * IMG_W + (j - 1)] +
                            TmpOrigin[(i) * IMG_W + (j + 1)] +
                            TmpOrigin[(i + 1) * IMG_W + (j - 1)] +
                            TmpOrigin[(i + 1) * IMG_W + j] +
                            TmpOrigin[(i + 1) * IMG_W + (j + 1)]) >> 3;

                // white noise
                if (TmpOrigin[i * IMG_W + j] > mean_val)
                {
                    weight = TmpOrigin[i * IMG_W + j] - mean_val - (image_mean_val >> 1);
                    if (weight < 0)
                    {
                        continue;
                    }

                    count = 0;
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i - 1)*IMG_W + (j - 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i - 1)*IMG_W + (j)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i - 1)*IMG_W + (j + 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i)*IMG_W + (j - 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i)*IMG_W + (j + 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i + 1)*IMG_W + (j - 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i + 1)*IMG_W + (j)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i + 1)*IMG_W + (j + 1)])
                    {
                        count++;
                    }

                    if (count > 1)
                    {
                        continue;
                    }

                    OutImg[i * IMG_W + j] = (TmpOrigin[i * IMG_W + j] * m_MagBuffer[i * IMG_W + j] + mean_val * (255 - m_MagBuffer[i * IMG_W + j])) / 255;
                }
            }
        }
        memcpy(TmpOrigin, OutImg, IMG_SIZE);
    }

    // remove black noise
    for (loop = 0; loop < 1; loop++)
    {
        for (i = TOP; i <= BOTTOM; i++)
        {
            for (j = LEFT; j <= RIGHT; j++)
            {
                mean_val = (TmpOrigin[(i - 1) * IMG_W + (j - 1)] +
                            TmpOrigin[(i - 1) * IMG_W + j] +
                            TmpOrigin[(i - 1) * IMG_W + (j + 1)] +
                            TmpOrigin[(i) * IMG_W + (j - 1)] +
                            TmpOrigin[(i) * IMG_W + (j + 1)] +
                            TmpOrigin[(i + 1) * IMG_W + (j - 1)] +
                            TmpOrigin[(i + 1) * IMG_W + j] +
                            TmpOrigin[(i + 1) * IMG_W + (j + 1)]) >> 3;

                // black noise
                if (TmpOrigin[i * IMG_W + j] < mean_val)
                {
                    count = 0;
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i - 1)*IMG_W + (j - 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i - 1)*IMG_W + (j)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i - 1)*IMG_W + (j + 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i)*IMG_W + (j - 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i)*IMG_W + (j + 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i + 1)*IMG_W + (j - 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i + 1)*IMG_W + (j)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i + 1)*IMG_W + (j + 1)])
                    {
                        count++;
                    }

                    if (count > 4)
                    {
                        continue;
                    }

                    OutImg[i * IMG_W + j] = (TmpOrigin[i * IMG_W + j] * m_MagBuffer[i * IMG_W + j] + mean_val * (255 - m_MagBuffer[i * IMG_W + j])) / 255;
                }
            }
        }
        memcpy(TmpOrigin, OutImg, IMG_SIZE);
    }

    for (i = TOP; i <= BOTTOM; i++)
    {
        for (j = LEFT; j <= RIGHT; j++)
        {
            // 2013-04-03 Gon add : increase speed
            if (OutImg[i * IMG_W + j] == 255)
            {
                continue;
            }

            val = (TmpOrigin[(i - 1) * IMG_W + (j - 1)] + // 1A
                   (TmpOrigin[(i - 1) * IMG_W + j] << 3) + // 8B
                   TmpOrigin[(i - 1) * IMG_W + (j + 1)] + // 1C
                   (TmpOrigin[i * IMG_W + (j - 1)] << 3) + // 8D
                   (TmpOrigin[i * IMG_W + j] << 5) + // 32E
                   (TmpOrigin[i * IMG_W + (j + 1)] << 3) + // 8F
                   TmpOrigin[(i + 1) * IMG_W + (j - 1)] + // 1G
                   (TmpOrigin[(i + 1) * IMG_W + j] << 3) + // 8H
                   TmpOrigin[(i + 1) * IMG_W + (j + 1)] // 1I
                  ) / 68;

            val = (int)TmpOrigin[i * IMG_W + j] - val;
            if (val < 0)
            {
                val = val >> 1;
            }
            val = (int)TmpOrigin[i * IMG_W + j] + (val<<1);//((val * 256) >> 7);
            if (val > 255)
            {
                val = 255;
            }
            else if (val < 0)
            {
                val = 0;
            }

            OutImg[i * IMG_W + j] = (unsigned char)val;
        }
    }
    memcpy(TmpOrigin, OutImg, IMG_SIZE);

    // sw uniform
    for (loop = 0; loop < 2; loop++)
    {
        for (i = TOP; i <= BOTTOM; i++)
        {
            for (j = LEFT; j <= RIGHT; j++)
            {
                OutImg[i * IMG_W + j] = (TmpOrigin[(i - 1) * IMG_W + (j - 1)] + // 1A
                                         (TmpOrigin[(i - 1) * IMG_W + j] << 3) + // 8B
                                         TmpOrigin[(i - 1) * IMG_W + (j + 1)] + // 1C
                                         (TmpOrigin[i * IMG_W + (j - 1)] << 3) + // 8D
                                         (TmpOrigin[i * IMG_W + j] << 4) + // 32E
                                         (TmpOrigin[i * IMG_W + (j + 1)] << 3) + // 8F
                                         TmpOrigin[(i + 1) * IMG_W + (j - 1)] + // 1G
                                         (TmpOrigin[(i + 1) * IMG_W + j] << 3) + // 8H
                                         TmpOrigin[(i + 1) * IMG_W + (j + 1)] // 1I
                                        ) / 52;
            }
        }
        memcpy(TmpOrigin, OutImg, IMG_SIZE);

        if (loop == 1)
        {
            break;
        }

        for (i = TOP; i <= BOTTOM; i++)
        {
            for (j = LEFT; j <= RIGHT; j++)
            {
                // 2013-04-03 Gon add : increase speed
                if (OutImg[i * IMG_W + j] == 255)
                {
                    continue;
                }

                val = (TmpOrigin[(i - 1) * IMG_W + (j - 1)] + // 1A
                       (TmpOrigin[(i - 1) * IMG_W + j] << 3) + // 8B
                       TmpOrigin[(i - 1) * IMG_W + (j + 1)] + // 1C
                       (TmpOrigin[i * IMG_W + (j - 1)] << 3) + // 8D
                       (TmpOrigin[i * IMG_W + j] << 5) + // 32E
                       (TmpOrigin[i * IMG_W + (j + 1)] << 3) + // 8F
                       TmpOrigin[(i + 1) * IMG_W + (j - 1)] + // 1G
                       (TmpOrigin[(i + 1) * IMG_W + j] << 3) + // 8H
                       TmpOrigin[(i + 1) * IMG_W + (j + 1)] // 1I
                      ) / 68;

                val = (int)TmpOrigin[i * IMG_W + j] - val;
                if (val < 0)
                {
                    val = val >> 1;
                }
                val = (int)TmpOrigin[i * IMG_W + j] + (val<<1);//((val * 256) >> 7);
                if (val > 255)
                {
                    val = 255;
                }
                else if (val < 0)
                {
                    val = 0;
                }

                OutImg[i * IMG_W + j] = (unsigned char)val;
            }
        }
        memcpy(TmpOrigin, OutImg, IMG_SIZE);
    }

    for (i = TOP; i <= BOTTOM; i++)
    {
        for (j = LEFT; j <= RIGHT; j++)
        {
            // 2013-04-03 Gon add : increase speed
            if (OutImg[i * IMG_W + j] == 255)
            {
                continue;
            }

            val = (TmpOrigin[(i - 1) * IMG_W + (j - 1)] + // 1A
                   (TmpOrigin[(i - 1) * IMG_W + j] << 3) + // 8B
                   TmpOrigin[(i - 1) * IMG_W + (j + 1)] + // 1C
                   (TmpOrigin[i * IMG_W + (j - 1)] << 3) + // 8D
                   (TmpOrigin[i * IMG_W + j] << 5) + // 32E
                   (TmpOrigin[i * IMG_W + (j + 1)] << 3) + // 8F
                   TmpOrigin[(i + 1) * IMG_W + (j - 1)] + // 1G
                   (TmpOrigin[(i + 1) * IMG_W + j] << 3) + // 8H
                   TmpOrigin[(i + 1) * IMG_W + (j + 1)] // 1I
                  ) / 68;

            val = (int)TmpOrigin[i * IMG_W + j] - val;
            if (val < 0)
            {
                val = val >> 1;
            }
            val = (int)TmpOrigin[i * IMG_W + j] + (val*6);//((val * 768) >> 7);
            if (val > 255)
            {
                val = 255;
            }
            else if (val < 0)
            {
                val = 0;
            }

            OutImg[i * IMG_W + j] = (unsigned char)val;
        }
    }

    for (i = 0; i < IMG_SIZE; i++)
    {
        OutImg[i] = 255 - OutImg[i];
    }
}


int CIBAlgorithm::_Algo_SegmentFingerEnhanced(unsigned char *ImgData, NEW_SEGMENT_ARRAY *m_new_segment_arr, 
											  BYTE *m_OutSplitResultArr, BYTE *m_OutSplitResultArrEx, 
											  int *m_OutSplitResultArrExWidth, int *m_OutSplitResultArrExHeight, int *m_OutSplitResultArrExSize)
{
//	int OldSegmentCnt;
	BYTE *m_segment_enlarge_buffer_enhanced = new BYTE[ZOOM_ENLAGE_W*ZOOM_ENLAGE_H];
	BYTE *m_segment_enlarge_buffer_for_fingercnt_enhanced = new BYTE[ZOOM_ENLAGE_W*ZOOM_ENLAGE_H];
	BYTE *m_labeled_segment_enlarge_buffer_enhanced = new BYTE[ZOOM_ENLAGE_W*ZOOM_ENLAGE_H];
	BYTE *EnlargeBuf_enhanced = new BYTE[ZOOM_ENLAGE_W*ZOOM_ENLAGE_H];
	BYTE *EnlargeBuf_Org_enhanced = new BYTE[ZOOM_ENLAGE_W*ZOOM_ENLAGE_H];

	memset(m_new_segment_arr, 0, sizeof(NEW_SEGMENT_ARRAY));

	int ImageBright = _Algo_SegmentationImageEnhanced(ImgData,
													m_segment_enlarge_buffer_enhanced,
													m_segment_enlarge_buffer_for_fingercnt_enhanced,
													EnlargeBuf_enhanced,
													EnlargeBuf_Org_enhanced);

//	OldSegmentCnt = _Algo_GetSegmentInfo(&m_segment_arr, ImageBright);
	_Algo_GetSegmentInfoEnhanced(m_new_segment_arr, ImageBright,
								m_segment_enlarge_buffer_for_fingercnt_enhanced,
								m_labeled_segment_enlarge_buffer_enhanced);

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		for(int i=0; i<m_new_segment_arr->SegmentCnt; i++)
		{
			m_new_segment_arr->Segment[i].Ang = 90;
		}
		m_new_segment_arr->VCenterSegment.Ang = 90;
	}
	
	_Algo_GetMainDirectionOfFingerEnhanced(m_new_segment_arr);

	// 4. sort segments descending order from top to bottom
	_Algo_SortSegmentByMainDirEnhanced(m_new_segment_arr);

	// 5. merge segment if same finger
	_Algo_MergeSegmentIfSameFingerEnhanced(m_new_segment_arr);
//	TRACE("2. NewSegmentCnt : %d\n", m_new_segment_arr->SegmentCnt);
	
	// 6. search exact position of segment
	_Algo_SearchFingerTipPositionEnhanced(m_new_segment_arr, EnlargeBuf_enhanced, m_labeled_segment_enlarge_buffer_enhanced);

	NEW_SEGMENT_ARRAY tmpArray;
	memcpy(&tmpArray, m_new_segment_arr, sizeof(tmpArray));

	double cs_radian = cos((m_new_segment_arr->VCenterSegment.Ang-90)*3.14159265358979/180.0);
	double sn_radian = sin((m_new_segment_arr->VCenterSegment.Ang-90)*3.14159265358979/180.0);

	for(int u=0; u<m_new_segment_arr->SegmentCnt; u++)
	{
		tmpArray.Segment[u].P1_X = (int) ( (double)(m_new_segment_arr->Segment[u].P1_X-m_new_segment_arr->VCenterSegment.X)*cs_radian + 
										   (double)(m_new_segment_arr->Segment[u].P1_Y-m_new_segment_arr->VCenterSegment.Y)*sn_radian ) + m_new_segment_arr->VCenterSegment.X;
		tmpArray.Segment[u].P1_Y = (int) ( -(double)(m_new_segment_arr->Segment[u].P1_X-m_new_segment_arr->VCenterSegment.X)*sn_radian + 
										   (double)(m_new_segment_arr->Segment[u].P1_Y-m_new_segment_arr->VCenterSegment.Y)*cs_radian ) + m_new_segment_arr->VCenterSegment.Y;

		tmpArray.Segment[u].P2_X = (int) ( (double)(m_new_segment_arr->Segment[u].P2_X-m_new_segment_arr->VCenterSegment.X)*cs_radian + 
										   (double)(m_new_segment_arr->Segment[u].P2_Y-m_new_segment_arr->VCenterSegment.Y)*sn_radian ) + m_new_segment_arr->VCenterSegment.X;
		tmpArray.Segment[u].P2_Y = (int) ( -(double)(m_new_segment_arr->Segment[u].P2_X-m_new_segment_arr->VCenterSegment.X)*sn_radian + 
										   (double)(m_new_segment_arr->Segment[u].P2_Y-m_new_segment_arr->VCenterSegment.Y)*cs_radian ) + m_new_segment_arr->VCenterSegment.Y;

		tmpArray.Segment[u].P3_X = (int) ( (double)(m_new_segment_arr->Segment[u].P3_X-m_new_segment_arr->VCenterSegment.X)*cs_radian + 
										   (double)(m_new_segment_arr->Segment[u].P3_Y-m_new_segment_arr->VCenterSegment.Y)*sn_radian ) + m_new_segment_arr->VCenterSegment.X;
		tmpArray.Segment[u].P3_Y = (int) ( -(double)(m_new_segment_arr->Segment[u].P3_X-m_new_segment_arr->VCenterSegment.X)*sn_radian + 
										   (double)(m_new_segment_arr->Segment[u].P3_Y-m_new_segment_arr->VCenterSegment.Y)*cs_radian ) + m_new_segment_arr->VCenterSegment.Y;

		tmpArray.Segment[u].P4_X = (int) ( (double)(m_new_segment_arr->Segment[u].P4_X-m_new_segment_arr->VCenterSegment.X)*cs_radian + 
										   (double)(m_new_segment_arr->Segment[u].P4_Y-m_new_segment_arr->VCenterSegment.Y)*sn_radian ) + m_new_segment_arr->VCenterSegment.X;
		tmpArray.Segment[u].P4_Y = (int) ( -(double)(m_new_segment_arr->Segment[u].P4_X-m_new_segment_arr->VCenterSegment.X)*sn_radian + 
										   (double)(m_new_segment_arr->Segment[u].P4_Y-m_new_segment_arr->VCenterSegment.Y)*cs_radian ) + m_new_segment_arr->VCenterSegment.Y;

		tmpArray.Segment[u].CenterX = (tmpArray.Segment[u].P1_X + tmpArray.Segment[u].P2_X + 
										tmpArray.Segment[u].P3_X + tmpArray.Segment[u].P4_X)/4;
		tmpArray.Segment[u].CenterY = (tmpArray.Segment[u].P1_Y + tmpArray.Segment[u].P2_Y + 
										tmpArray.Segment[u].P3_Y + tmpArray.Segment[u].P4_Y)/4;
	}

	NEW_SEGMENT new_seg;
	for(int gg=0; gg<tmpArray.SegmentCnt-1; gg++)
	{
		for(int tt=gg+1; tt<tmpArray.SegmentCnt; tt++)
		{
			if(tmpArray.Segment[gg].CenterX > tmpArray.Segment[tt].CenterX)
			{
				new_seg = tmpArray.Segment[gg];
				tmpArray.Segment[gg] = tmpArray.Segment[tt];
				tmpArray.Segment[tt] = new_seg;

				new_seg = m_new_segment_arr->Segment[gg];
				m_new_segment_arr->Segment[gg] = m_new_segment_arr->Segment[tt];
				m_new_segment_arr->Segment[tt] = new_seg;
			}
		}
	}

	int i, s, t;
	int startx, starty;
	for(i=0; i<m_new_segment_arr->SegmentCnt; i++)
	{
		_Algo_ClipFinger(ImgData, 
						&m_OutSplitResultArrEx[i*IMG_SIZE], 
						&m_OutSplitResultArrExWidth[i], 
						&m_OutSplitResultArrExHeight[i], 
						&m_OutSplitResultArrExSize[i], 
						m_labeled_segment_enlarge_buffer_enhanced, 
						&m_new_segment_arr->Segment[i]);

		startx = (IMG_W-m_OutSplitResultArrExWidth[i])/2;
		starty = (IMG_H-m_OutSplitResultArrExHeight[i])/2;

		for(s=0; s<m_OutSplitResultArrExHeight[i]; s++)
		{
			for(t=0; t<m_OutSplitResultArrExWidth[i]; t++)
			{
				m_OutSplitResultArr[i*IMG_SIZE + (s+starty)*IMG_W+(startx+t)] = 
					m_OutSplitResultArrEx[i*IMG_SIZE + s*m_OutSplitResultArrExWidth[i]+t];
			}
		}
	}

	delete [] m_segment_enlarge_buffer_enhanced;
	delete [] m_segment_enlarge_buffer_for_fingercnt_enhanced;
	delete [] m_labeled_segment_enlarge_buffer_enhanced;
	delete [] EnlargeBuf_enhanced;
	delete [] EnlargeBuf_Org_enhanced;
	
	return m_new_segment_arr->SegmentCnt;
}

int CIBAlgorithm::_Algo_SegmentationImageEnhanced(unsigned char *ImgData,
												  BYTE *m_segment_enlarge_buffer_enhanced,
												  BYTE *m_segment_enlarge_buffer_for_fingercnt_enhanced,
												  BYTE *EnlargeBuf_enhanced,
												  BYTE *EnlargeBuf_Org_enhanced)
{
	int y,x, i,j;
	int mean=0,count=0;
	int ii, tempsum;
	int xx, yy, forgroundCNT=0;
	int sum_x=0, sum_y=0;
	
	memset(m_segment_enlarge_buffer_enhanced, 0, ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);
	memset(m_segment_enlarge_buffer_for_fingercnt_enhanced, 0, ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);
	memset(EnlargeBuf_enhanced, 0, ZOOM_ENLAGE_H*ZOOM_ENLAGE_W);
	
	for(y=ENLARGESIZE_ZOOM_H; y<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; y++)
	{
		yy = (y-ENLARGESIZE_ZOOM_H) * IMG_H / ZOOM_H * IMG_W;

		for(x=ENLARGESIZE_ZOOM_W; x<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; x++)
		{
			xx = (x-ENLARGESIZE_ZOOM_W) * IMG_W / ZOOM_W;
			
			mean=0;
			count=0;
		
			if( (yy+xx-1-IMG_W) >= 0 && (yy+xx-1-IMG_W) < IMG_SIZE )
			{
				mean+=ImgData[yy+xx-1-IMG_W];
				count++;
			}

			if( (yy+xx-IMG_W) >= 0 && (yy+xx-IMG_W) < IMG_SIZE )
			{
				mean+=ImgData[yy+xx-IMG_W] << 1;
				count+=2;
			}

			if( (yy+xx+1-IMG_W) >= 0 && (yy+xx+1-IMG_W) < IMG_SIZE )
			{
				mean+=ImgData[yy+xx+1-IMG_W];
				count++;
			}

			if( (yy+xx-1) >= 0 && (yy+xx-1) < IMG_SIZE )						
			{
				mean+=ImgData[yy+xx-1] << 1;
				count+=2;
			}

			if( (yy+xx) >= 0 && (yy+xx) < IMG_SIZE )
			{
				mean+=ImgData[yy+xx] << 2;
				count+=4;
			}

			if( (yy+xx+1) >= 0 && (yy+xx+1) < IMG_SIZE )
			{
				mean+=ImgData[yy+xx+1] << 1;
				count+=2;
			}
			
			if( (yy+xx-1+IMG_W) >= 0 && (yy+xx-1+IMG_W) < IMG_SIZE )
			{
				mean+=ImgData[yy+xx-1+IMG_W];
				count++;
			}

			if( (yy+xx+IMG_W) >= 0 && (yy+xx+IMG_W) < IMG_SIZE )
			{
				mean+=ImgData[yy+xx+IMG_W] << 1;
				count+=2;
			}

			if( (yy+xx+1+IMG_W) >= 0 && (yy+xx+1+IMG_W) < IMG_SIZE )
			{
				mean+=ImgData[yy+xx+1+IMG_W];
				count++;
			}

			if(count > 0)
				mean /= count;

			EnlargeBuf_enhanced[(y)*ZOOM_ENLAGE_W+(x)]=mean;
//			EnlargeBuf[(y)*ZOOM_ENLAGE_W+(x)]=ImgData[yy+xx];
		}
	}

	memcpy(EnlargeBuf_Org_enhanced, EnlargeBuf_enhanced, ZOOM_ENLAGE_H*ZOOM_ENLAGE_W);
	_Algo_HistogramNormalizeForZoomEnlarge(EnlargeBuf_enhanced);

	mean = 0; count = 0;
	for (i=ENLARGESIZE_ZOOM_H; i<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; i++)
	{
		for (j=ENLARGESIZE_ZOOM_W; j<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; j++)
		{
			if(EnlargeBuf_enhanced[(i)*ZOOM_ENLAGE_W+(j)] > 10)
			{
				mean += EnlargeBuf_enhanced[(i)*ZOOM_ENLAGE_W+(j)];
				count++;
			}
		}
	}

	if(count==0)
		return 0;

	mean /= count;

	int Threshold = (int)(pow((double)mean/255.0, 3.0) * 256);
	if(Threshold < 30)
		Threshold = 30;
	else if(Threshold > 128)
		Threshold = 128;

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_SHERLOCK)
		Threshold /= 2;
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_COLUMBO)
	{
		if(Threshold < 10)
			Threshold = 10;
		else if(Threshold > 30)
			Threshold = 30;
	}

//	TRACE("==> mean : %d, Threshold : %d\n", mean, Threshold);

	// for finger count
	for (i=ENLARGESIZE_ZOOM_H; i<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; i++)
	{
		for (j=ENLARGESIZE_ZOOM_W; j<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; j++)
		{
			tempsum  = EnlargeBuf_enhanced[(i-1)*ZOOM_ENLAGE_W+(j-1)];
			tempsum += EnlargeBuf_enhanced[(i-1)*ZOOM_ENLAGE_W+(j)];
			tempsum += EnlargeBuf_enhanced[(i-1)*ZOOM_ENLAGE_W+(j+1)];
			tempsum += EnlargeBuf_enhanced[(i)*ZOOM_ENLAGE_W+(j-1)];
			tempsum += EnlargeBuf_enhanced[(i)*ZOOM_ENLAGE_W+(j)];
			tempsum += EnlargeBuf_enhanced[(i)*ZOOM_ENLAGE_W+(j+1)];
			tempsum += EnlargeBuf_enhanced[(i+1)*ZOOM_ENLAGE_W+(j-1)];
			tempsum += EnlargeBuf_enhanced[(i+1)*ZOOM_ENLAGE_W+(j)];
			tempsum += EnlargeBuf_enhanced[(i+1)*ZOOM_ENLAGE_W+(j+1)];
			
			tempsum /= 9;
			
			// fixed bug.
			if(tempsum >= Threshold)//mean/20)
				m_segment_enlarge_buffer_for_fingercnt_enhanced[i*ZOOM_ENLAGE_W+j] = 255;
		}
	}
	
	for(ii=0; ii<1; ii++)
	{
		memcpy(m_segment_enlarge_buffer_enhanced, m_segment_enlarge_buffer_for_fingercnt_enhanced, ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);
		for (i=ENLARGESIZE_ZOOM_H; i<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; i++)
		{
			for (j=ENLARGESIZE_ZOOM_W; j<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; j++)
			{
				if(m_segment_enlarge_buffer_enhanced[i*ZOOM_ENLAGE_W+j] == 255)
					continue;

				tempsum  = m_segment_enlarge_buffer_enhanced[(i-1)*ZOOM_ENLAGE_W+j-1];
				tempsum += m_segment_enlarge_buffer_enhanced[(i-1)*ZOOM_ENLAGE_W+j  ];
				tempsum += m_segment_enlarge_buffer_enhanced[(i-1)*ZOOM_ENLAGE_W+j+1];
				tempsum += m_segment_enlarge_buffer_enhanced[(i  )*ZOOM_ENLAGE_W+j-1];
				tempsum += m_segment_enlarge_buffer_enhanced[(i  )*ZOOM_ENLAGE_W+j+1];
				tempsum += m_segment_enlarge_buffer_enhanced[(i+1)*ZOOM_ENLAGE_W+j-1];
				tempsum += m_segment_enlarge_buffer_enhanced[(i+1)*ZOOM_ENLAGE_W+j  ];
				tempsum += m_segment_enlarge_buffer_enhanced[(i+1)*ZOOM_ENLAGE_W+j+1];

				if(tempsum >= 4*255)
					m_segment_enlarge_buffer_for_fingercnt_enhanced[i*ZOOM_ENLAGE_W+j] = 255;
			}
		}
	}

	memcpy(m_segment_enlarge_buffer_enhanced, m_segment_enlarge_buffer_for_fingercnt_enhanced, ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);
	_Algo_StackRecursiveFilling(m_segment_enlarge_buffer_enhanced, ZOOM_ENLAGE_W, ZOOM_ENLAGE_H, 0, 0, 0, 128);

	for (y=ENLARGESIZE_ZOOM_H; y<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; y++)
	{
		for (x=ENLARGESIZE_ZOOM_W; x<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; x++)
		{
			if(m_segment_enlarge_buffer_enhanced[y*ZOOM_ENLAGE_W+x] == 0)
				m_segment_enlarge_buffer_for_fingercnt_enhanced[y*ZOOM_ENLAGE_W+x] = 255;
		}
	}

	memcpy(m_segment_enlarge_buffer_enhanced,m_segment_enlarge_buffer_for_fingercnt_enhanced,ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);
	
	for (y=ENLARGESIZE_ZOOM_H; y<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; y++)
	{
		for (x=ENLARGESIZE_ZOOM_W; x<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; x++)
		{
			if(m_segment_enlarge_buffer_enhanced[(y-1)*ZOOM_ENLAGE_W+x-1]==0)		m_segment_enlarge_buffer_for_fingercnt_enhanced[y*ZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer_enhanced[(y-1)*ZOOM_ENLAGE_W+x]==0)	m_segment_enlarge_buffer_for_fingercnt_enhanced[y*ZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer_enhanced[(y-1)*ZOOM_ENLAGE_W+x+1]==0)	m_segment_enlarge_buffer_for_fingercnt_enhanced[y*ZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer_enhanced[y*ZOOM_ENLAGE_W+x-1]==0)	m_segment_enlarge_buffer_for_fingercnt_enhanced[y*ZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer_enhanced[y*ZOOM_ENLAGE_W+x]==0)		m_segment_enlarge_buffer_for_fingercnt_enhanced[y*ZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer_enhanced[y*ZOOM_ENLAGE_W+x+1]==0)	m_segment_enlarge_buffer_for_fingercnt_enhanced[y*ZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer_enhanced[(y+1)*ZOOM_ENLAGE_W+x-1]==0)	m_segment_enlarge_buffer_for_fingercnt_enhanced[y*ZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer_enhanced[(y+1)*ZOOM_ENLAGE_W+x]==0)	m_segment_enlarge_buffer_for_fingercnt_enhanced[y*ZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer_enhanced[(y+1)*ZOOM_ENLAGE_W+x+1]==0)	m_segment_enlarge_buffer_for_fingercnt_enhanced[y*ZOOM_ENLAGE_W+x] = 0;
			else	m_segment_enlarge_buffer_for_fingercnt_enhanced[y*ZOOM_ENLAGE_W+x] = 255;

			if(m_segment_enlarge_buffer_for_fingercnt_enhanced[y*ZOOM_ENLAGE_W+x] == 255)
				forgroundCNT++;
		}
	}

	if( forgroundCNT < 100 )
		return 0;

	memcpy(m_segment_enlarge_buffer_enhanced, m_segment_enlarge_buffer_for_fingercnt_enhanced, ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);

	mean=0;
	count=0;
	for (y=ENLARGESIZE_ZOOM_H; y<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; y++)
	{
		for (x=ENLARGESIZE_ZOOM_W; x<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; x++)
		{
			if(m_segment_enlarge_buffer_enhanced[y*ZOOM_ENLAGE_W+x]==0)
				continue;
			
			if(EnlargeBuf_Org_enhanced[(y)*ZOOM_ENLAGE_W+(x)] >= Threshold)
			{
				mean += EnlargeBuf_Org_enhanced[(y)*ZOOM_ENLAGE_W+(x)];
				count++;
				sum_x += x*5;
				sum_y += y*5;
			}
		}
	}
	if( count==0 )
		return 0;

	mean /= count;

	return mean;
}

int CIBAlgorithm::_Algo_GetSegmentInfoEnhanced(NEW_SEGMENT_ARRAY *segment_arr, int ImageBright,
											   BYTE *m_segment_enlarge_buffer_for_fingercnt_enhanced,
											   BYTE *m_labeled_segment_enlarge_buffer_enhanced)
{
	int i, j, cnt;
	int sum_x, sum_y, sum_cnt;
	int cx, cy, dx, dy, dist, sum_dx, sum_dy, sum_dxdy, sum_count, SegmentCnt;
	int Area, MinAreaThreshold = MIN_MAIN_FOREGROUND_AREA;

	memcpy(m_labeled_segment_enlarge_buffer_enhanced, m_segment_enlarge_buffer_for_fingercnt_enhanced, ZOOM_ENLAGE_H*ZOOM_ENLAGE_W);

	// foreground labeling
	SegmentCnt=0;
	for (i=ENLARGESIZE_ZOOM_H; i<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; i++)
	{
		for (j=ENLARGESIZE_ZOOM_W; j<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; j++)
		{
			if(m_labeled_segment_enlarge_buffer_enhanced[(i*ZOOM_ENLAGE_W)+j] == 255)
			{
				SegmentCnt++;
				Area = _Algo_StackRecursiveFilling_ZoomEnlarge(m_labeled_segment_enlarge_buffer_enhanced, j, i, 255, SegmentCnt);

				if(Area <= MinAreaThreshold)
				{
					_Algo_StackRecursiveFilling_ZoomEnlarge(m_labeled_segment_enlarge_buffer_enhanced, j, i, SegmentCnt, 0);
					SegmentCnt--;
				}
			}

			// 계산 불가
			if(SegmentCnt >= MAX_SEGMENT_COUNT) return 3;
		}
	}

	if(SegmentCnt <= 0)
		return 0;

	// calculate center position of foreground
	for(cnt=1; cnt<=SegmentCnt; cnt++)
	{
		sum_x=0;
		sum_y=0;
		sum_cnt=0;

		for (i=ENLARGESIZE_ZOOM_H; i<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; i++)
		{
			for (j=ENLARGESIZE_ZOOM_W; j<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; j++)
			{
				if( m_labeled_segment_enlarge_buffer_enhanced[(i*ZOOM_ENLAGE_W)+j] == cnt )
				{
					sum_x += j;
					sum_y += i;
					sum_cnt++;
				}
			}
		}

		if(sum_cnt >= MinAreaThreshold)
		{
			segment_arr->Segment[segment_arr->SegmentCnt].X = sum_x / sum_cnt;
			segment_arr->Segment[segment_arr->SegmentCnt].Y = sum_y / sum_cnt;
			segment_arr->Segment[segment_arr->SegmentCnt].Label = cnt;
			segment_arr->Segment[segment_arr->SegmentCnt].Area = sum_cnt;
			segment_arr->SegmentCnt++;
		}
		else
		{
			for (i=ENLARGESIZE_ZOOM_H; i<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; i++)
			{
				for (j=ENLARGESIZE_ZOOM_W; j<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; j++)
				{				
					if( m_labeled_segment_enlarge_buffer_enhanced[(i*ZOOM_ENLAGE_W)+j] == cnt )
					{
						m_labeled_segment_enlarge_buffer_enhanced[(i*ZOOM_ENLAGE_W)+j] = 0;
					}
				}
			}
		}
	}
	
	// calculate direction of foreground
	for(cnt=0; cnt<segment_arr->SegmentCnt; cnt++)
	{
		cx = segment_arr->Segment[cnt].X;
		cy = segment_arr->Segment[cnt].Y;
		sum_dx = 0;
		sum_dy = 0;
		sum_dxdy = 0;
		sum_count = 0;
		for (i=ENLARGESIZE_ZOOM_H; i<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; i++)
		{
			for (j=ENLARGESIZE_ZOOM_W; j<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; j++)
			{			
				if( m_labeled_segment_enlarge_buffer_enhanced[(i*ZOOM_ENLAGE_W)+j] == segment_arr->Segment[cnt].Label )
				{
					dx = j - cx;
					dy = i - cy;

//					dist = SQRT_Table[abs(dy)][abs(dx)];
					dist = (int)sqrt((double)dy*dy + (double)dx*dx);

					if(dist == 0)
						continue;
					
					dx = dx * 10 / dist;
					dy = dy * 10 / dist;

					sum_dx += dx*dx;
					sum_dy += dy*dy;
					sum_dxdy += dx*dy;
					sum_count++;
				}
			}
		}

		segment_arr->Segment[cnt].Ang = _Algo_ATAN2_FULL(2*sum_dxdy, (sum_dx - sum_dy)) * 180 / 512;
	}

	return segment_arr->SegmentCnt;
}

void CIBAlgorithm::_Algo_GetMainDirectionOfFingerEnhanced(NEW_SEGMENT_ARRAY *m_new_segment_arr)
{
	int i;
	double angle_cos, angle_sin;

	angle_cos = 0;
	angle_sin = 0;

	if(m_new_segment_arr->SegmentCnt == 0)
		return;

	for(i=0; i<m_new_segment_arr->SegmentCnt; i++)
	{
		m_new_segment_arr->VCenterSegment.X += m_new_segment_arr->Segment[i].X;
		m_new_segment_arr->VCenterSegment.Y += m_new_segment_arr->Segment[i].Y;

		angle_cos += cos(m_new_segment_arr->Segment[i].Ang*3.14159265358979/90.0) * m_new_segment_arr->Segment[i].Area * 10;
		angle_sin += sin(m_new_segment_arr->Segment[i].Ang*3.14159265358979/90.0) * m_new_segment_arr->Segment[i].Area * 10;
	}

	angle_cos /= m_new_segment_arr->SegmentCnt;
	angle_sin /= m_new_segment_arr->SegmentCnt;

	m_new_segment_arr->VCenterSegment.X /= m_new_segment_arr->SegmentCnt;
	m_new_segment_arr->VCenterSegment.Y /= m_new_segment_arr->SegmentCnt;
	m_new_segment_arr->VCenterSegment.Ang = _Algo_ATAN2_FULL((int)angle_sin, (int)angle_cos) * 180 / 512;
}

void CIBAlgorithm::_Algo_SortSegmentByMainDirEnhanced(NEW_SEGMENT_ARRAY *m_new_segment_arr)
{
	int i, j;
	int cx, cy;
	double cs_radian, sn_radian;
	NEW_SEGMENT_ARRAY tmp_arr;
	memcpy(&tmp_arr, &m_segment_arr, sizeof(NEW_SEGMENT_ARRAY));

	cx = m_new_segment_arr->VCenterSegment.X;
	cy = m_new_segment_arr->VCenterSegment.Y;
	cs_radian = cos((m_new_segment_arr->VCenterSegment.Ang-90)*3.14159265358979/180.0);
	sn_radian = sin((m_new_segment_arr->VCenterSegment.Ang-90)*3.14159265358979/180.0);

	// rotate center pos by main dir
	for(i=0; i<m_new_segment_arr->SegmentCnt; i++)
	{
		tmp_arr.Segment[i].X = (int) ( (double)(m_new_segment_arr->Segment[i].X-cx)*cs_radian + (double)(m_new_segment_arr->Segment[i].Y-cy)*sn_radian ) + cx;
		tmp_arr.Segment[i].Y = (int) ( -(double)(m_new_segment_arr->Segment[i].X-cx)*sn_radian + (double)(m_new_segment_arr->Segment[i].Y-cy)*cs_radian ) + cy;
		tmp_arr.Segment[i].Ang = tmp_arr.Segment[i].Ang - m_new_segment_arr->VCenterSegment.Ang + 90;
	}

	// sort segment by y position
	for(i=0; i<tmp_arr.SegmentCnt-1; i++)
	{
		for(j=i+1; j<tmp_arr.SegmentCnt; j++)
		{
			if(tmp_arr.Segment[i].Y < tmp_arr.Segment[j].Y)
			{
                _Algo_SwapNewSegment(&tmp_arr.Segment[i], &tmp_arr.Segment[j]);
                _Algo_SwapNewSegment(&m_new_segment_arr->Segment[i], &m_new_segment_arr->Segment[j]);
			}
		}
	}
}

void CIBAlgorithm::_Algo_MergeSegmentIfSameFingerEnhanced(NEW_SEGMENT_ARRAY *m_new_segment_arr)
{
	int i, j, dx, dy;
	int diffdir_btw_two_pos, diffdir;
	unsigned char same_seg_flag[MAX_SEGMENT_COUNT];
	unsigned char valid_seg_flag[MAX_SEGMENT_COUNT];
	NEW_SEGMENT_ARRAY tmp_arr;

	memset(valid_seg_flag, 1, m_new_segment_arr->SegmentCnt);

	for(i=0; i<m_new_segment_arr->SegmentCnt-1; i++)
	{
		if(valid_seg_flag[i] == 0) continue;

		memset(same_seg_flag, 0xFF, m_new_segment_arr->SegmentCnt);

		for(j=i+1; j<m_new_segment_arr->SegmentCnt; j++)
		{
			if(i == j) 
				continue;
			
			if(valid_seg_flag[j] == 0) 
				continue;
			
//			if(m_new_segment_arr->Segment[i].Y <= m_new_segment_arr->Segment[j].Y)
//				continue;

			dx = m_new_segment_arr->Segment[j].X - m_new_segment_arr->Segment[i].X;
			dy = m_new_segment_arr->Segment[j].Y - m_new_segment_arr->Segment[i].Y;
			diffdir_btw_two_pos = (int)(atan2((double)dy, (double)dx) * 180.0f / 3.14159265f);
			if(diffdir_btw_two_pos < 0)	diffdir_btw_two_pos += 180;

			diffdir = diffdir_btw_two_pos - m_new_segment_arr->Segment[i].Ang;

			if(abs(diffdir) > THRESHOLD_DIFFERENT_ANGLE_2)
				continue;

			same_seg_flag[j] = i;
			valid_seg_flag[j] = 0;
		}
	}

	memset(&tmp_arr, 0, sizeof(NEW_SEGMENT_ARRAY));
	tmp_arr.VCenterSegment = m_new_segment_arr->VCenterSegment;
	for(i=0; i<m_new_segment_arr->SegmentCnt; i++)
	{
		if(valid_seg_flag[i] == 1)
		{
			tmp_arr.Segment[tmp_arr.SegmentCnt] = m_new_segment_arr->Segment[i];
			tmp_arr.SegmentCnt++;
		}
	}

	memcpy(&m_segment_arr, &tmp_arr, sizeof(NEW_SEGMENT_ARRAY));
}

void CIBAlgorithm::_Algo_SearchFingerTipPositionEnhanced(NEW_SEGMENT_ARRAY *m_new_segment_arr,
														 BYTE *EnlargeBuf_Enhanced,
														 BYTE *m_labeled_segment_enlarge_buffer_enhanced)
{
	int seg_cnt, i, j;
	int RotAxisX, RotAxisY, RotLineX, RotLineY;
	int Count_up, Count_dn;
	double rad, cosangle, sinangle, rad_90, cosangle_90, sinangle_90;
	int sum_mean;
	int TopImage, BottomImage, LeftImage, RightImage;
	int ZeroCount, First_zero_idx;
	int MaxVal, MinVal, CutThres, CUT_THRES;
	int *MeanV = new int[ZOOM_ENLAGE_H*2];
	int *MeanV_Filtered = new int[ZOOM_ENLAGE_H*2];
	int *MeanH = new int[ZOOM_ENLAGE_W*2];
	int *MeanH_Filtered = new int[ZOOM_ENLAGE_W*2];
	int *Mean_Count = new int[ZOOM_ENLAGE_W*2];
	int x1, y1, x2, y2, x3, y3, x4, y4;
	int Width1, Width2, Height1, Height2;
	int MaxWidth, MaxHeight;
	int ReduceX, ReduceY;
	
	for(seg_cnt=0; seg_cnt<m_new_segment_arr->SegmentCnt; seg_cnt++)
	{
		rad = -(m_new_segment_arr->Segment[seg_cnt].Ang-90) * 3.141592 / 180.0;
		cosangle = cos(rad);
		sinangle = sin(rad);

		rad_90 = -(m_new_segment_arr->Segment[seg_cnt].Ang-180) * 3.141592 / 180.0;
		cosangle_90 = cos(rad_90);
		sinangle_90 = sin(rad_90);

		memset(MeanV, 0, ZOOM_ENLAGE_H*2*sizeof(int));
		memset(MeanV_Filtered, 0, ZOOM_ENLAGE_H*2*sizeof(int));
		memset(MeanH, 0, ZOOM_ENLAGE_W*2*sizeof(int));
		memset(MeanH_Filtered, 0, ZOOM_ENLAGE_W*2*sizeof(int));
		memset(Mean_Count, 0, ZOOM_ENLAGE_W*2*sizeof(int));

		for(i=1; i<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H*2; i++)
		{
			// find FingerTip pos
			RotAxisX = (int)(m_new_segment_arr->Segment[seg_cnt].X + i * sinangle);
			RotAxisY = (int)(m_new_segment_arr->Segment[seg_cnt].Y + i * cosangle);

			Count_up=0;
			Count_dn=0;

			if(RotAxisX >= ENLARGESIZE_ZOOM_W && RotAxisY >= ENLARGESIZE_ZOOM_H && 
				RotAxisX < ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W && RotAxisY < ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H)
			{
				// horizontal line
				for(j=1; j<ZOOM_ENLAGE_W/2; j++)
				{
					// right
					RotLineX = (int)(RotAxisX + j * sinangle_90);
					RotLineY = (int)(RotAxisY + j * cosangle_90);
					if(RotLineX >= ENLARGESIZE_ZOOM_W && RotLineY >= ENLARGESIZE_ZOOM_H && 
						RotLineX < ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W && RotLineY < ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer_enhanced[RotLineY*ZOOM_ENLAGE_W+RotLineX] == m_new_segment_arr->Segment[seg_cnt].Label )
						{
							MeanV[ZOOM_ENLAGE_H-1-i] += EnlargeBuf_Enhanced[RotLineY*ZOOM_ENLAGE_W+RotLineX];
							Count_up++;
							MeanH[ZOOM_ENLAGE_W+j] += EnlargeBuf_Enhanced[RotLineY*ZOOM_ENLAGE_W+RotLineX];
							Mean_Count[ZOOM_ENLAGE_W+j]++;
						}
					}

					// left
					RotLineX = (int)(RotAxisX - j * sinangle_90);
					RotLineY = (int)(RotAxisY - j * cosangle_90);
					if(RotLineX >= ENLARGESIZE_ZOOM_W && RotLineY >= ENLARGESIZE_ZOOM_H && 
						RotLineX < ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W && RotLineY < ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer_enhanced[RotLineY*ZOOM_ENLAGE_W+RotLineX] == m_new_segment_arr->Segment[seg_cnt].Label )
						{
							MeanV[ZOOM_ENLAGE_H-1-i] += EnlargeBuf_Enhanced[RotLineY*ZOOM_ENLAGE_W+RotLineX];
							Count_up++;
							MeanH[ZOOM_ENLAGE_W-1-j] += EnlargeBuf_Enhanced[RotLineY*ZOOM_ENLAGE_W+RotLineX];
							Mean_Count[ZOOM_ENLAGE_W-1-j]++;
						}
					}
				}
			}

			// down-axis position
			RotAxisX = (int)(m_new_segment_arr->Segment[seg_cnt].X - i * sinangle);
			RotAxisY = (int)(m_new_segment_arr->Segment[seg_cnt].Y - i * cosangle);
		
			if(RotAxisX >= ENLARGESIZE_ZOOM_W && RotAxisY >= ENLARGESIZE_ZOOM_H && 
				RotAxisX < ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W && RotAxisY < ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H)
			{
				// horizontal line
				for(j=1; j<ZOOM_ENLAGE_W/2; j++)
				{
					// right
					RotLineX = (int)(RotAxisX + j * sinangle_90);
					RotLineY = (int)(RotAxisY + j * cosangle_90);
					if(RotLineX >= ENLARGESIZE_ZOOM_W && RotLineY >= ENLARGESIZE_ZOOM_H && 
						RotLineX < ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W && RotLineY < ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer_enhanced[RotLineY*ZOOM_ENLAGE_W+RotLineX] == m_new_segment_arr->Segment[seg_cnt].Label )
						{
							MeanV[i+ZOOM_ENLAGE_H] += EnlargeBuf_Enhanced[RotLineY*ZOOM_ENLAGE_W+RotLineX];
							Count_dn++;
							MeanH[ZOOM_ENLAGE_W+j] += EnlargeBuf_Enhanced[RotLineY*ZOOM_ENLAGE_W+RotLineX];
							Mean_Count[ZOOM_ENLAGE_W+j]++;
						}
					}

					// left
					RotLineX = (int)(RotAxisX - j * sinangle_90);
					RotLineY = (int)(RotAxisY - j * cosangle_90);
					if(RotLineX >= ENLARGESIZE_ZOOM_W && RotLineY >= ENLARGESIZE_ZOOM_H && 
						RotLineX < ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W && RotLineY < ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer_enhanced[RotLineY*ZOOM_ENLAGE_W+RotLineX] == m_new_segment_arr->Segment[seg_cnt].Label )
						{
							MeanV[i+ZOOM_ENLAGE_H] += EnlargeBuf_Enhanced[RotLineY*ZOOM_ENLAGE_W+RotLineX];
							Count_dn++;
							MeanH[ZOOM_ENLAGE_W-1-j] += EnlargeBuf_Enhanced[RotLineY*ZOOM_ENLAGE_W+RotLineX];
							Mean_Count[ZOOM_ENLAGE_W-1-j]++;
						}
					}
				}
			}

			if(Count_up > 0)	MeanV[ZOOM_ENLAGE_H-1-i] /= Count_up;
			if(Count_dn > 0)	MeanV[i+ZOOM_ENLAGE_H] /= Count_dn;
		}

		for(i=0; i<ZOOM_ENLAGE_W*2; i++)
		{
			if(Mean_Count[i] > 0)
				MeanH[i] /= Mean_Count[i];
		}

		MeanV[ZOOM_ENLAGE_H] = (MeanV[ZOOM_ENLAGE_H-1]+MeanV[ZOOM_ENLAGE_H+1])/2;
		MeanH[ZOOM_ENLAGE_W] = (MeanH[ZOOM_ENLAGE_W-1]+MeanH[ZOOM_ENLAGE_W+1])/2;

		// mean filter
		MaxVal=-1;
		MinVal=1000000;
		for(i=2; i<ZOOM_ENLAGE_H*2-2; i++)
		{
			sum_mean = MeanV[i-1];
			sum_mean += MeanV[i  ];
			sum_mean += MeanV[i+1];

			sum_mean /= 3;
			MeanV_Filtered[i] = sum_mean;

			if(MaxVal < MeanV_Filtered[i])
				MaxVal = MeanV_Filtered[i];
			if(MinVal > MeanV_Filtered[i])
				MinVal = MeanV_Filtered[i];
		}

		// calculate image height
		TopImage=ZOOM_ENLAGE_H*2, BottomImage=0;
		for(i=2; i<ZOOM_ENLAGE_H*2-2; i++)
		{
			if(MeanV_Filtered[i] > 5)
			{
				TopImage = i;
				break;
			}
		}
		for(i=ZOOM_ENLAGE_H*2-2; i>=2; i--)
		{
			if(MeanV_Filtered[i] > 5)
			{
				BottomImage = i;
				break;
			}
		}
		
		// find first under threshold
		ZeroCount=0;
		First_zero_idx = BottomImage;

		if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			CutThres = (MaxVal-MinVal)/4;
		else
		{
			CutThres = (MaxVal-MinVal)/4;
			if(MaxVal < 50)
				CutThres = 5;
			else if(MaxVal < 75)
				CutThres = 10;
			else if(MaxVal < 100)
				CutThres = 15;
			else
				CutThres = 20;
		}
		for(i=TopImage+50; i<BottomImage; i++)
		{
			if(i < TopImage+100)
				CUT_THRES = MinVal + CutThres;
			else if(i < TopImage+150)
				CUT_THRES = MinVal + CutThres*2;
			else if(i < TopImage+200)
				CUT_THRES = MinVal + CutThres*3;
			else
				CUT_THRES = MinVal + CutThres*4;

			if(MeanV_Filtered[i] < CUT_THRES && MeanV_Filtered[i] >= 0)
				ZeroCount++;
			else
				ZeroCount=0;
			
			if(ZeroCount >= 2)
			{
				First_zero_idx = i;
				break;
			}
		}

		// mean filter
		MaxVal=-1;
		MinVal=1000000;
		for(i=2; i<ZOOM_ENLAGE_W*2-2; i++)
		{
			sum_mean = MeanH[i-1];
			sum_mean += MeanH[i  ];
			sum_mean += MeanH[i+1];

			sum_mean /= 3;
			MeanH_Filtered[i] = sum_mean;

			if(MaxVal < MeanH_Filtered[i])
				MaxVal = MeanH_Filtered[i];
			if(MinVal > MeanH_Filtered[i])
				MinVal = MeanH_Filtered[i];
		}

		LeftImage=ZOOM_ENLAGE_W*2, RightImage=0;
		for(i=2; i<ZOOM_ENLAGE_W*2-2; i++)
		{
			if(MeanH_Filtered[i] > 5)
			{
				LeftImage = i;
				break;
			}
		}
		for(i=ZOOM_ENLAGE_W*2-2; i>=2; i--)
		{
			if(MeanH_Filtered[i] > 5)
			{
				RightImage = i;
				break;
			}
		}

		LeftImage -= 1;
		RightImage += 1;
		TopImage -= 1;
		First_zero_idx += 1;

		x1 = (LeftImage-ZOOM_ENLAGE_W);
		y1 = (ZOOM_ENLAGE_H-TopImage);

		x2 = (RightImage-ZOOM_ENLAGE_W);
		y2 = (ZOOM_ENLAGE_H-TopImage);

		x3 = (RightImage-ZOOM_ENLAGE_W);
		y3 = (ZOOM_ENLAGE_H-First_zero_idx);

		x4 = (LeftImage-ZOOM_ENLAGE_W);
		y4 = (ZOOM_ENLAGE_H-First_zero_idx);

		ReduceX=0;
		ReduceY=0;
		Width1 = abs(x2-x1)+1;
		Width2 = abs(x3-x4)+1;
		Height1 = abs(y1-y4)+1;
		Height2 = abs(y2-y3)+1;

		if(Width1 > Width2)		MaxWidth = Width1;
		else					MaxWidth = Width2;
		if(Height1 > Height2)	MaxHeight = Height1;
		else					MaxHeight = Height2;
		
		if(MaxWidth > ZOOM_W)
			ReduceX = (int)((double)(MaxWidth - ZOOM_W)/2.0+0.5);
		if(MaxHeight > ZOOM_H)
			ReduceY = (int)((double)(MaxHeight - ZOOM_H)/2.0+0.5);

		x1 = x1 + ReduceX;
		x4 = x4 + ReduceX;
		x2 = x2 - ReduceX;
		x3 = x3 - ReduceX;

		y1 = y1 - ReduceY;
		y4 = y4 + ReduceY;
		y2 = y2 - ReduceY;
		y3 = y3 + ReduceY;

		m_new_segment_arr->Segment[seg_cnt].TipX = (int)(m_new_segment_arr->Segment[seg_cnt].X + y1 * sinangle);
		m_new_segment_arr->Segment[seg_cnt].TipY = (int)(m_new_segment_arr->Segment[seg_cnt].Y + y1 * cosangle);
		m_new_segment_arr->Segment[seg_cnt].KnuckleX = (int)(m_new_segment_arr->Segment[seg_cnt].X + y3 * sinangle);
		m_new_segment_arr->Segment[seg_cnt].KnuckleY = (int)(m_new_segment_arr->Segment[seg_cnt].Y + y3 * cosangle);
		m_new_segment_arr->Segment[seg_cnt].LeftX = (int)(m_new_segment_arr->Segment[seg_cnt].X + x1 * sinangle_90);
		m_new_segment_arr->Segment[seg_cnt].LeftY = (int)(m_new_segment_arr->Segment[seg_cnt].Y + x1 * cosangle_90);
		m_new_segment_arr->Segment[seg_cnt].RightX = (int)(m_new_segment_arr->Segment[seg_cnt].X + x2 * sinangle_90);
		m_new_segment_arr->Segment[seg_cnt].RightY = (int)(m_new_segment_arr->Segment[seg_cnt].Y + x2 * cosangle_90);
		m_new_segment_arr->Segment[seg_cnt].CenterX = (m_new_segment_arr->Segment[seg_cnt].KnuckleX + m_new_segment_arr->Segment[seg_cnt].TipX) / 2;
		m_new_segment_arr->Segment[seg_cnt].CenterY = (m_new_segment_arr->Segment[seg_cnt].KnuckleY + m_new_segment_arr->Segment[seg_cnt].TipY) / 2;

		m_new_segment_arr->Segment[seg_cnt].P1_X = (int)(x2*cosangle+y1*sinangle + m_new_segment_arr->Segment[seg_cnt].X);
		m_new_segment_arr->Segment[seg_cnt].P1_Y = (int)(-x2*sinangle+y1*cosangle + m_new_segment_arr->Segment[seg_cnt].Y);
		m_new_segment_arr->Segment[seg_cnt].P2_X = (int)(x1*cosangle+y1*sinangle + m_new_segment_arr->Segment[seg_cnt].X);
		m_new_segment_arr->Segment[seg_cnt].P2_Y = (int)(-x1*sinangle+y1*cosangle + m_new_segment_arr->Segment[seg_cnt].Y);
		m_new_segment_arr->Segment[seg_cnt].P3_X = (int)(x1*cosangle+y3*sinangle + m_new_segment_arr->Segment[seg_cnt].X);
		m_new_segment_arr->Segment[seg_cnt].P3_Y = (int)(-x1*sinangle+y3*cosangle + m_new_segment_arr->Segment[seg_cnt].Y);
		m_new_segment_arr->Segment[seg_cnt].P4_X = (int)(x2*cosangle+y3*sinangle + m_new_segment_arr->Segment[seg_cnt].X);
		m_new_segment_arr->Segment[seg_cnt].P4_Y = (int)(-x2*sinangle+y3*cosangle + m_new_segment_arr->Segment[seg_cnt].Y);

/*		m_new_segment_arr->Segment[seg_cnt].P1_X = m_new_segment_arr->Segment[seg_cnt].TipX - m_new_segment_arr->Segment[seg_cnt].X + m_new_segment_arr->Segment[seg_cnt].RightX;
		m_new_segment_arr->Segment[seg_cnt].P1_Y = m_new_segment_arr->Segment[seg_cnt].TipY - m_new_segment_arr->Segment[seg_cnt].Y + m_new_segment_arr->Segment[seg_cnt].RightY;
		m_new_segment_arr->Segment[seg_cnt].P2_X = m_new_segment_arr->Segment[seg_cnt].TipX - m_new_segment_arr->Segment[seg_cnt].X + m_new_segment_arr->Segment[seg_cnt].LeftX;
		m_new_segment_arr->Segment[seg_cnt].P2_Y = m_new_segment_arr->Segment[seg_cnt].TipY - m_new_segment_arr->Segment[seg_cnt].Y + m_new_segment_arr->Segment[seg_cnt].LeftY;
		m_new_segment_arr->Segment[seg_cnt].P3_X = m_new_segment_arr->Segment[seg_cnt].KnuckleX - m_new_segment_arr->Segment[seg_cnt].X + m_new_segment_arr->Segment[seg_cnt].LeftX;
		m_new_segment_arr->Segment[seg_cnt].P3_Y = m_new_segment_arr->Segment[seg_cnt].KnuckleY - m_new_segment_arr->Segment[seg_cnt].Y + m_new_segment_arr->Segment[seg_cnt].LeftY;
		m_new_segment_arr->Segment[seg_cnt].P4_X = m_new_segment_arr->Segment[seg_cnt].KnuckleX - m_new_segment_arr->Segment[seg_cnt].X + m_new_segment_arr->Segment[seg_cnt].RightX;
		m_new_segment_arr->Segment[seg_cnt].P4_Y = m_new_segment_arr->Segment[seg_cnt].KnuckleY - m_new_segment_arr->Segment[seg_cnt].Y + m_new_segment_arr->Segment[seg_cnt].RightY;
*/
		////////////////////////////////////////////////////////////////////////////////////
		// remove crease area in image
		// Line Equation : y = sin(BackDir) / cos(BackDir) * (x + BackX) + BackY
		RotAxisX = m_new_segment_arr->Segment[seg_cnt].KnuckleX;
		RotAxisY = m_new_segment_arr->Segment[seg_cnt].KnuckleY;
		
		rad = (m_new_segment_arr->Segment[seg_cnt].Ang-90) * 3.141592 / 180.0;
		cosangle = cos(rad);
		sinangle = sin(rad);

		for(i=ENLARGESIZE_ZOOM_H; i<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; i++)
		{
			for(j=ENLARGESIZE_ZOOM_W; j<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; j++)
			{
				if(cosangle != 0)
				{
					if( i < (int)(sinangle / cosangle * (j - RotAxisX) + RotAxisY))
					{
						if(m_labeled_segment_enlarge_buffer_enhanced[i*ZOOM_ENLAGE_W+j] == m_new_segment_arr->Segment[seg_cnt].Label)
							m_labeled_segment_enlarge_buffer_enhanced[i*ZOOM_ENLAGE_W+j] = 0;
					}
				}
				else
				{
					if( i < RotAxisY)
					{
						if(m_labeled_segment_enlarge_buffer_enhanced[i*ZOOM_ENLAGE_W+j] == m_new_segment_arr->Segment[seg_cnt].Label)
							m_labeled_segment_enlarge_buffer_enhanced[i*ZOOM_ENLAGE_W+j] = 0;
					}
				}
			}
		}
		////////////////////////////////////////////////////////////////////////////////////	
	}

	delete [] MeanV;
	delete [] MeanV_Filtered;
	delete [] MeanH;
	delete [] MeanH_Filtered;
	delete [] Mean_Count;
}
