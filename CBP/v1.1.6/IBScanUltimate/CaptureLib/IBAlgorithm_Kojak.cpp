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
// Kojak Algorithm
void CIBAlgorithm::_Algo_Kojak_GetDistortionMask_FromSavedFile()
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
				size_t readSize = fread(m_SBDAlg->arrPos_wb, 1, maskSize, fp);
                fclose(fp);

                if ((readSize == maskSize) && _Algo_ConfirmChecksum(m_SBDAlg->arrPos_wb, maskElements))
                {
                    isSameDM = TRUE;
                }
            }

            if (!isSameDM)
            {
                _Algo_Kojak_Init_Distortion_Merge_with_Bilinear();
                fp = fopen(m_cDistortionMaskPath, "wb");
                if (fp != NULL)
                {
                    _Algo_MakeChecksum(m_SBDAlg->arrPos_wb, maskElements);
                    fwrite(m_SBDAlg->arrPos_wb, 1, maskSize, fp);
                    fclose(fp);
                }
            }
        }
        else
        {
            _Algo_Kojak_Init_Distortion_Merge_with_Bilinear();
        }
    }
}

void CIBAlgorithm::_Algo_Kojak_Init_Distortion_Merge_with_Bilinear()
{
    m_SBDAlg->_Init_Distortion_Merge_with_Bilinear(-0.0000000000002f, 0.0000000007f, -0.0000007f, 0.000185f, 96 + m_PPI_Correction_Horizontal, 90 + m_PPI_Correction_Vertical, 0, 5);
}

void CIBAlgorithm::_Algo_Kojak_DistortionRestoration_with_Bilinear(unsigned char *InImg, unsigned char *OutImg)
{
	m_SBDAlg->_Make500DPI_opt(InImg, OutImg);
/*	int value;
    int i, j, x, y, pos, sp, sq;
    int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

    memset(OutImg, 0, IMG_SIZE);
    dtLEFT		= m_cImgAnalysis.LEFT * IMG_W / CIS_IMG_W;
    dtRIGHT		= m_cImgAnalysis.RIGHT * IMG_W / CIS_IMG_W;
    dtTOP		= m_cImgAnalysis.TOP * IMG_H / CIS_IMG_H;
    dtBOTTOM	= m_cImgAnalysis.BOTTOM * IMG_H / CIS_IMG_H;
    for (i = dtTOP; i <= dtBOTTOM; i++)
    {
        for (j = dtLEFT; j <= dtRIGHT; j++)
        {
            x = (arrPos_Kojak[i * IMG_W + j] >> 21);
            y = (arrPos_Kojak[i * IMG_W + j] >> 10) & 0x7FF;
            sp = (arrPos_Kojak[i * IMG_W + j] >> 5) & 0x1F;
            sq = arrPos_Kojak[i * IMG_W + j] & 0x1F;
            pos = y * CIS_IMG_W + x;

            value = (sp * (sq * InImg[pos] + (32 - sq) * InImg[pos + CIS_IMG_W]) + (32 - sp) * (sq * InImg[pos + 1] + (32 - sq) * InImg[pos + CIS_IMG_W + 1])) >> 10;

            OutImg[i * IMG_W + j] = (unsigned char)value;
        }
    }*/
}

int CIBAlgorithm::_Algo_Kojak_GetBrightWithRawImage_forDetectOnly(BYTE *InRawImg, int ForgraoundCNT)
{
	int y, x, i, j;
	int ii, tempsum;
	int xx, yy, forgroundCNT = 0;
	int value;

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

	if(m_pUsbDevInfo->bDecimation == TRUE)
		m_Minus_Value = _DEFAULT_CMOS_NOISE_FOR_DECI_;
	else
		m_Minus_Value = _NO_BIT_CMOS_NOISE_FOR_FULLFRAME_;

	for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
	{
		yy = (y - ENLARGESIZE_ZOOM_H) * (CIS_IMG_H/2) / ZOOM_H;

		for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
		{
			xx = (x - ENLARGESIZE_ZOOM_W) * (CIS_IMG_W/2) / ZOOM_W;
			value = InRawImg[yy * (CIS_IMG_W/2) + xx];
			if(value < m_Minus_Value)
			{
				value = 0;
			}
			else
			{
				value = (int)(value * m_SBDAlg->diff_image_withMinusMean_preview[yy*CIS_IMG_W/2+xx] / 1024) + value - m_Minus_Value;
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

//			tempsum /= 9;

			// fixed bug.
			if (tempsum >= 9)//Threshold) //mean/20)
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

				if (tempsum >= 1020)//4 * 255)
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

/*			if (m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] == 255)
			{
				forgroundCNT++;
			}
*/		}
	}

	for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
	{
		for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
		{
			if (m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] == 0)
				continue;

			tempsum=0;
			if (m_segment_enlarge_buffer_for_fingercnt[(y-1) * ZOOM_ENLAGE_W + x-1] == 255)	tempsum++;
			if (m_segment_enlarge_buffer_for_fingercnt[(y-1) * ZOOM_ENLAGE_W + x] == 255)	tempsum++;
			if (m_segment_enlarge_buffer_for_fingercnt[(y-1) * ZOOM_ENLAGE_W + x+1] == 255)	tempsum++;
			if (m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x-1] == 255)	tempsum++;
			if (m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x+1] == 255)	tempsum++;
			if (m_segment_enlarge_buffer_for_fingercnt[(y+1) * ZOOM_ENLAGE_W + x-1] == 255)	tempsum++;
			if (m_segment_enlarge_buffer_for_fingercnt[(y+1) * ZOOM_ENLAGE_W + x] == 255)	tempsum++;
			if (m_segment_enlarge_buffer_for_fingercnt[(y+1) * ZOOM_ENLAGE_W + x+1] == 255)	tempsum++;

			if(tempsum > 3)
				forgroundCNT++;
		}
	}

	if(forgroundCNT >= ForgraoundCNT)
		return 1;

	return 0;
}

void CIBAlgorithm::_ALGO_MedianFilter(unsigned char *input_image, int imgWidth, int imgHeight, int *histogram)
{
    int i, j;
    int a, b, c;
    int temp;

// enzyme modify
// 사용할 필요가 없다
//  memcpy(temp_image_buffer, input_image, D_ENLARGE_H*D_ENLARGE_W);

    for (i=0; i<imgHeight-1; i++){
        for (j=0; j<imgWidth-1; j++){
            a = input_image[i*imgWidth+j];
            b = input_image[i*imgWidth+(j+1)];
            c = input_image[(i+1)*imgWidth+j];

            if(a>b){
                temp=a;
                a=b;
                b=temp;
            }

            if(b>c){
                temp=b;
                b=c;
                c=temp;
            }
            if(a>b){
                temp=a;
                a=b;
                b=temp;
            }
            input_image[i*imgWidth+j] = b;
            histogram[b]++;
        }
    }
}

void CIBAlgorithm::_ALGO_HistogramStretch(unsigned char *pImage, int imgWidth, int imgHeight)
{
#define D_CUT_MINTRES		0.05
#define D_CUT_MAXTRES		0.05

    int y/*,gray*/;
    int histogram[256];
    int LUT[256],value;
    double scale_factor;
    int CUTMIN_PIXELNUM = (int)(D_CUT_MINTRES*(imgWidth*imgHeight));
    int CUTMAX_PIXELNUM = (int)(D_CUT_MAXTRES*(imgWidth*imgHeight));

    memset(histogram,0,sizeof(histogram));
    memset(LUT,0,sizeof(LUT));
    _ALGO_MedianFilter(pImage, imgWidth, imgHeight, histogram);

    int minsum=0,maxsum=0;
    int MIN_TRES=0,MAX_TRES=255;
    for(y=0; y<256; y++)
    {
        if(minsum>CUTMIN_PIXELNUM)
        {
            MIN_TRES=y;
            break;
        }
        else if(histogram[y]>0)
        {
            minsum+=histogram[y];
        }
        LUT[y]=0;
    }
    for(y=255; y>=MIN_TRES; y--)
    {
        if(maxsum>CUTMAX_PIXELNUM)
        {
            MAX_TRES=y;
            break;
        }
        else if(histogram[y]>0)
        {
            maxsum+=histogram[y];
        }
        LUT[y]=255;
    }

	if(MAX_TRES < 50)
		MAX_TRES = 50;

    //스트레칭의 강도를 조절할 수 있는 factor 계산
    if(MAX_TRES==MIN_TRES)
        scale_factor=0;
    else
        scale_factor = 255.0/(MAX_TRES-MIN_TRES);

    for(y=MIN_TRES; y<=MAX_TRES; y++)
    {
        value=(int)((y-MIN_TRES)*scale_factor);
        if(value>255)value=255;
        LUT[y] = value;
    }

	for(y=0; y<imgWidth*imgHeight; y++)
    {
        pImage[y] = LUT[pImage[y]];
    }
/*    for(y=0; y<imgHeight; y++)
    {
        for(x=0; x<imgWidth; x++)
        {
            pImage[y*imgWidth+x] = LUT[pImage[y*imgWidth+x]];
        }
    }*/
}

void CIBAlgorithm::_ALGO_HistogramStretch_Roll(unsigned char *pImage, int imgWidth, int imgHeight, int MergePosX)
{
#define D_CUT_MINTRES		0.05
#define D_CUT_MAXTRES		0.05

    int y/*,gray*/;
    int histogram[256];
    int LUT[256],value;
    double scale_factor;
    int CUTMIN_PIXELNUM = (int)(D_CUT_MINTRES*(200 * CIS_IMG_H_ROLL));
    int CUTMAX_PIXELNUM = (int)(D_CUT_MAXTRES*(200 * CIS_IMG_H_ROLL));

    memset(histogram,0,sizeof(histogram));
    memset(LUT,0,sizeof(LUT));

	int i, j;
	int TOP, BOTTOM;
	int tmpCIS_IMG_W = CIS_IMG_W_ROLL;

	TOP = m_cImgAnalysis.TOP;
	BOTTOM = m_cImgAnalysis.BOTTOM;

	for (i = TOP; i < BOTTOM; i++)
	{
		for (j = MergePosX - 100; j <= MergePosX + 100; j++)
		{
			histogram[pImage[i*tmpCIS_IMG_W+j]]++;
		}
	}

    int minsum=0,maxsum=0;
    int MIN_TRES=0,MAX_TRES=255;
    for(y=0; y<256; y++)
    {
        if(minsum>CUTMIN_PIXELNUM)
        {
            MIN_TRES=y;
            break;
        }
        else if(histogram[y]>0)
        {
            minsum+=histogram[y];
        }
        LUT[y]=0;
    }
    for(y=255; y>=MIN_TRES; y--)
    {
        if(maxsum>CUTMAX_PIXELNUM)
        {
            MAX_TRES=y;
            break;
        }
        else if(histogram[y]>0)
        {
            maxsum+=histogram[y];
        }
        LUT[y]=255;
    }

	if(MAX_TRES < 50)
		MAX_TRES = 50;

    //스트레칭의 강도를 조절할 수 있는 factor 계산
    if(MAX_TRES==MIN_TRES)
        scale_factor=0;
    else
        scale_factor = 255.0/(MAX_TRES-MIN_TRES);

    for(y=MIN_TRES; y<=MAX_TRES; y++)
    {
        value=(int)((y-MIN_TRES)*scale_factor);
        if(value>255)value=255;
        LUT[y] = value;
    }

	for (i = TOP; i < BOTTOM; i++)
	{
		for (j = MergePosX - 100; j <= MergePosX + 100; j++)
		{
		    pImage[i*tmpCIS_IMG_W+j] = LUT[pImage[i*tmpCIS_IMG_W+j]];
		}
	}
}

int CIBAlgorithm::_Algo_Kojak_GetBrightWithRawImage_Final(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY, int *Bright)
{
	int j, i, k, l, y, x;
    int tempmean;
    int tempthreshold;
    int tempsum;
    int temp1, temp2;

	int xx, yy, forgroundCNT = 0;
	int value, mean, count, sum_x, sum_y, BrightValue, Threshold = 10, m_Minus_Value ;

	if(m_pUsbDevInfo->bDecimation == TRUE)
		m_Minus_Value = _DEFAULT_CMOS_NOISE_FOR_DECI_;
	else
		m_Minus_Value = _NO_BIT_CMOS_NOISE_FOR_FULLFRAME_;

    int imgEnWidth = CIS_IMG_W + (D_ENLARGE_BAND<<1);
    int imgEnHeight = CIS_IMG_H + (D_ENLARGE_BAND<<1);
    int imgEnHalfWidth = imgEnWidth>>1;
    int imgEnHalfHeight = imgEnHeight>>1;
    int imgEnQuadWidth = imgEnHalfWidth>>1;
    int imgEnQuadHeight = imgEnHalfHeight>>1;

	unsigned char *pSegBuf = (unsigned char*)m_Inter_Img2;
	unsigned char *m_pHalfImage = (unsigned char*)m_Inter_Img3;
	unsigned char *m_pEnhancedImg = (unsigned char*)m_Inter_Img4;
	unsigned int *m_pGradientBuffer = (unsigned int*)&m_SBDAlg->m_SumGrayBuf2[0];

    memset(pSegBuf, 0, imgEnQuadWidth*imgEnQuadHeight);
    memset(m_pHalfImage, 0, imgEnHalfWidth*imgEnHalfHeight);
    memset(m_pGradientBuffer, 0, imgEnHalfWidth*imgEnHalfHeight*sizeof(unsigned int));
    tempmean = 0;
	memset(m_segment_enlarge_buffer, 0, ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);
	memset(m_segment_enlarge_buffer_for_fingercnt, 0, ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);

	memcpy(m_pEnhancedImg, InRawImg, (CIS_IMG_W/2)*(CIS_IMG_H/2));
	_ALGO_HistogramStretch(m_pEnhancedImg, CIS_IMG_W/2, CIS_IMG_H/2);

	for (i=0; i<CIS_IMG_H/2; i++){
		memcpy(&m_pHalfImage[(i+(D_ENLARGE_BAND>>1))*imgEnHalfWidth+(D_ENLARGE_BAND>>1)], &m_pEnhancedImg[i*CIS_IMG_W/2], CIS_IMG_W/2);
	}
	
	for (i=D_ENLARGE_BAND/2; i<imgEnHalfHeight-D_ENLARGE_BAND/2; i+=4){
        for (j=D_ENLARGE_BAND/2; j<imgEnHalfWidth-D_ENLARGE_BAND/2; j+=4){
            for (k=0; k<4; k++){
                for (l=0; l<4; l++){
                    temp1 = (int)m_pHalfImage[(i+k)*imgEnHalfWidth+(j+l)] - (int)m_pHalfImage[(i+k)*imgEnHalfWidth+(j+l+1)];
                    temp2 = (int)m_pHalfImage[(i+k)*imgEnHalfWidth+(j+l)] - (int)m_pHalfImage[(i+k+1)*imgEnHalfWidth+(j+l)];
                    temp1 = temp1*temp1 + temp2*temp2;
                    m_pGradientBuffer[(i+k)*imgEnHalfWidth+(j+l)] = temp1;
                    tempmean += temp1;
                }
            }
        }
    }

    tempmean/=(imgEnWidth*imgEnHeight/4);

    tempthreshold = (int)(55000.0-(tempmean*8.71));
    if (tempthreshold > 49000)  tempthreshold = 49000;
    else if (tempthreshold < 26500) tempthreshold = 26000;
	tempthreshold = tempthreshold*2/3;

    for (i=D_ENLARGE_BAND>>1; i<imgEnHalfHeight-(D_ENLARGE_BAND>>1); i+=2)
    {
        tempsum = 0;
        for (k=-4; k<=4; k++){
            for (l=-4; l<=4; l++){
                tempsum += m_pGradientBuffer[(i+k)*imgEnHalfWidth+(D_ENLARGE_BAND/2+l)];
            }
        }
        for (j=(D_ENLARGE_BAND>>1)+2; j<imgEnHalfWidth-(D_ENLARGE_BAND>>1); j+=2)
        {
            for (k=-4; k<=4; k++){
                for (l=-6; l<=-5; l++){
                    tempsum -= m_pGradientBuffer[(i+k)*imgEnHalfWidth+(j+l)];
                }
            }
            for (k=-4; k<=4; k++){
                for (l=3; l<=4; l++){
                    tempsum += m_pGradientBuffer[(i+k)*imgEnHalfWidth+(j+l)];
                }
            }
            if (tempsum > tempthreshold)
                pSegBuf[(i>>1)*imgEnQuadWidth+(j>>1)] = 255;
        }
    }
	
	// for finger count
	for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
	{
		yy = (((i-ENLARGESIZE_ZOOM_H) * CIS_IMG_H) / ZOOM_H + D_ENLARGE_BAND)/4;

		for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
		{
			xx = (((j-ENLARGESIZE_ZOOM_W) * CIS_IMG_W) / ZOOM_W + D_ENLARGE_BAND)/4;
	
			if(pSegBuf[(yy)*imgEnQuadWidth+(xx)] == 0)
				continue;

			count = 0;
			if( pSegBuf[(yy-1)*imgEnQuadWidth+(xx-1)] == 255 ) count++;
			if( pSegBuf[(yy-1)*imgEnQuadWidth+(xx)] == 255 ) count++;
			if( pSegBuf[(yy-1)*imgEnQuadWidth+(xx+1)] == 255 ) count++;
			if( pSegBuf[(yy)*imgEnQuadWidth+(xx-1)] == 255 ) count++;
			//if( pSegBuf[(yy)*imgEnQuadWidth+(xx)] == 255 ) count++;
			if( pSegBuf[(yy)*imgEnQuadWidth+(xx+1)] == 255 ) count++;
			if( pSegBuf[(yy+1)*imgEnQuadWidth+(xx-1)] == 255 ) count++;
			if( pSegBuf[(yy+1)*imgEnQuadWidth+(xx)] == 255 ) count++;
			if( pSegBuf[(yy+1)*imgEnQuadWidth+(xx+1)] == 255 ) count++;
			
			if(count>5)
				m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W + j] = 255;
		}
	}

	// for finger count
//	int sum_value=0;
	
    memset(EnlargeBuf, 0, ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);
	for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
	{
		yy = ((i-ENLARGESIZE_ZOOM_H) * CIS_IMG_H/2) / ZOOM_H;

		for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
		{
			xx = ((j-ENLARGESIZE_ZOOM_W) * CIS_IMG_W/2) / ZOOM_W;

			value = InRawImg[yy * (CIS_IMG_W/2) + xx];

			if(value >= m_Minus_Value)
			{
				value = (int)(value * m_SBDAlg->diff_image_withMinusMean_preview[yy*(CIS_IMG_W/2)+xx] / 1024) + value - m_Minus_Value;
				
				if (value > 255)
				{
					value = 255;
				}
				else if (value < 0)
				{
					value = 0;
				}
				EnlargeBuf[i*ZOOM_ENLAGE_W+j] = value;
			}
		}
	}

	memcpy(EnlargeBuf_Org, EnlargeBuf, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);

	_Algo_HistogramStretchForZoomEnlarge(EnlargeBuf);

	int mean_of_foreground = 0;
	int foreground_count = 0;
	for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
	{
		for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
		{
			if (m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W + j] == 255)
			{
				foreground_count++;
				mean_of_foreground += EnlargeBuf[i * ZOOM_ENLAGE_W + j];
			}
		}
	}
	if(foreground_count > 100)
		mean_of_foreground /= foreground_count;
	else
		mean_of_foreground = 0;

	mean_of_foreground = mean_of_foreground * 9 * 2 / 3;
	int sum_mean;
	for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
	{
		for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
		{
			if (m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W + j] != 255)
			{
				sum_mean = (EnlargeBuf[(i-1) * ZOOM_ENLAGE_W + (j-1)] +
							EnlargeBuf[(i-1) * ZOOM_ENLAGE_W + (j)] +
							EnlargeBuf[(i-1) * ZOOM_ENLAGE_W + (j+1)] +
							EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j-1)] +
							EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j)] +
							EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j+1)] +
							EnlargeBuf[(i+1) * ZOOM_ENLAGE_W + (j-1)] +
							EnlargeBuf[(i+1) * ZOOM_ENLAGE_W + (j)] +
							EnlargeBuf[(i+1) * ZOOM_ENLAGE_W + (j+1)]);

				if(sum_mean > mean_of_foreground)
					m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W + j] = 255;
			}
		}
	}

	for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
	{
		for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
		{
			if (m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W + j] == 255)
			{
				forgroundCNT++;
			}
		}
	}

	*ForgraoundCNT = forgroundCNT;

	if (forgroundCNT < 100)
	{
		BrightValue = 0;
		goto progress_done;
	}

	memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);

	mean = 0;
	count = 0;
	sum_x = 0;
	sum_y = 0;
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
		BrightValue = 0;
		goto progress_done;
	}

	if(m_SBDAlg->m_SegmentBrightCnt > 100)
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
		if(m_SBDAlg->m_SegmentBrightCnt > 100)
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

    //memcpy(m_pTempQuadBuffer, pSegBuf, imgEnQuadWidth*imgEnQuadHeight);

    //_StackRecursiveFilling(m_pTempQuadBuffer, imgEnQuadWidth, imgEnQuadHeight, 0, 0, 255, 0, m_pStackBuf);
    //for(i=0; i<imgEnQuadHeight; i++)
    //{
    //    for(j=0; j<imgEnQuadWidth; j++)
    //    {
    //        if(m_pTempQuadBuffer[i*imgEnQuadWidth+j] == 255)
    //        {
    //            pSegBuf[i*imgEnQuadWidth+j] = 0;
    //        }
    //    }
    //}

progress_done:

//	delete [] pSegBuf;
//	delete [] m_pHalfImage;
//	delete [] m_pEnhancedImg;
//	delete [] m_pGradientBuffer;

	return BrightValue;
}

int CIBAlgorithm::_Algo_Kojak_GetBrightWithRawImage_forDetectOnly_Roll(BYTE *InRawImg, int ForgraoundCNT)
{
    int y, x, i, j;
    int ii, tempsum;
    int xx, yy, forgroundCNT = 0;
    int value;
	int m_Minus_Value = _NO_BIT_CMOS_NOISE_FOR_FULLFRAME_ROLL;
	int offset_x = (CIS_IMG_W-CIS_IMG_W_ROLL)/2;

	memset(m_segment_enlarge_buffer, 0, ZOOM_ENLAGE_W_ROLL * ZOOM_ENLAGE_H_ROLL);
    memset(m_segment_enlarge_buffer_for_fingercnt, 0, ZOOM_ENLAGE_W_ROLL * ZOOM_ENLAGE_H_ROLL);
    memset(EnlargeBuf, 0, ZOOM_ENLAGE_W_ROLL * ZOOM_ENLAGE_H_ROLL);

    m_cImgAnalysis.LEFT = 1;
    m_cImgAnalysis.RIGHT = 1;
    m_cImgAnalysis.TOP = 1;
    m_cImgAnalysis.BOTTOM = 1;

    for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
    {
        yy = (y - ENLARGESIZE_ZOOM_H_ROLL) * CIS_IMG_H_ROLL / ZOOM_H_ROLL;

        for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
        {
            xx = (x - ENLARGESIZE_ZOOM_W_ROLL) * CIS_IMG_W_ROLL / ZOOM_W_ROLL;

            value = InRawImg[yy * CIS_IMG_W_ROLL + xx];
			if(value < m_Minus_Value)
			{
				value = 0;
			}
			else
			{
				value = (int)(value * m_SBDAlg->diff_image_withMinusMean_preview[yy/2*CIS_IMG_W/2+xx/2+offset_x/2] / 1024)  + value - m_Minus_Value;
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

            //tempsum /= 9;

            // fixed bug.
            if (tempsum >= 9) //Threshold) //mean/20)
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

                if (tempsum >= 1020)//4 * 255)
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
/*
            if (m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] == 255)
            {
                forgroundCNT++;
            }
*/        }
    }

	for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
	{
		for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
		{
			if (m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x] == 0)
				continue;

			tempsum=0;
			if (m_segment_enlarge_buffer_for_fingercnt[(y-1) * ZOOM_ENLAGE_W_ROLL + x-1] == 255)	tempsum++;
			if (m_segment_enlarge_buffer_for_fingercnt[(y-1) * ZOOM_ENLAGE_W_ROLL + x] == 255)	tempsum++;
			if (m_segment_enlarge_buffer_for_fingercnt[(y-1) * ZOOM_ENLAGE_W_ROLL + x+1] == 255)	tempsum++;
			if (m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x-1] == 255)	tempsum++;
			if (m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W_ROLL + x+1] == 255)	tempsum++;
			if (m_segment_enlarge_buffer_for_fingercnt[(y+1) * ZOOM_ENLAGE_W_ROLL + x-1] == 255)	tempsum++;
			if (m_segment_enlarge_buffer_for_fingercnt[(y+1) * ZOOM_ENLAGE_W_ROLL + x] == 255)	tempsum++;
			if (m_segment_enlarge_buffer_for_fingercnt[(y+1) * ZOOM_ENLAGE_W_ROLL + x+1] == 255)	tempsum++;

			if(tempsum > 3)
				forgroundCNT++;
		}
	}

    if (forgroundCNT >= ForgraoundCNT)
        return 1;

    return 0;
}

int CIBAlgorithm::_Algo_Kojak_GetBrightWithRawImage(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY, int *Bright)
{
    int y, x, i, j;
	int mean = 0, count = 0;
	int ii, tempsum;
	int xx, yy, forgroundCNT = 0;
	int value;
	int sum_x = 0, sum_y = 0;
	int BrightValue = 0;

	memset(m_segment_enlarge_buffer, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
	memset(m_segment_enlarge_buffer_for_fingercnt, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
	memset(EnlargeBuf, 0, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);
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

	if(m_pUsbDevInfo->bDecimation == TRUE)
		m_Minus_Value = _DEFAULT_CMOS_NOISE_FOR_DECI_;
	else
		m_Minus_Value = _NO_BIT_CMOS_NOISE_FOR_FULLFRAME_;

	for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
	{
		yy = (y - ENLARGESIZE_ZOOM_H) * (CIS_IMG_H/2) / ZOOM_H;

		for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
		{
			xx = (x - ENLARGESIZE_ZOOM_W) * (CIS_IMG_W/2) / ZOOM_W;
			value = InRawImg[yy * (CIS_IMG_W/2) + xx];
			if(value >= m_Minus_Value)
			{
				value = (int)(value * m_SBDAlg->diff_image_withMinusMean_preview[yy*CIS_IMG_W/2+xx] / 1024) + value - m_Minus_Value;
				if (value > 255)
				{
					value = 255;
				}
				else if (value < 0)
				{
					value = 0;
				}
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

				if (tempsum >= 1020)//4 * 255)
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

	if(m_SBDAlg->m_SegmentBrightCnt > 100)
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
		if(m_SBDAlg->m_SegmentBrightCnt > 100)
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


int CIBAlgorithm::_Algo_Kojak_GetBrightWithRawImage_Roll(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY, int *Bright)
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
	int m_Minus_Value = _NO_BIT_CMOS_NOISE_FOR_FULLFRAME_ROLL;
	int offset_x = (CIS_IMG_W-CIS_IMG_W_ROLL)/2;

    //gettimeofday(&start_tv, NULL);
    memset(m_segment_enlarge_buffer, 0, ZOOM_ENLAGE_W_ROLL * ZOOM_ENLAGE_H_ROLL);
    memset(m_segment_enlarge_buffer_for_fingercnt, 0, ZOOM_ENLAGE_W_ROLL * ZOOM_ENLAGE_H_ROLL);
    memset(EnlargeBuf, 0, ZOOM_ENLAGE_H_ROLL * ZOOM_ENLAGE_W_ROLL);
    *CenterX = -1;
    *CenterY = -1;
    *ForgraoundCNT = 0;

    m_cImgAnalysis.LEFT = 1;
    m_cImgAnalysis.RIGHT = 1;
    m_cImgAnalysis.TOP = 1;
    m_cImgAnalysis.BOTTOM = 1;

    for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
    {
        yy = (y - ENLARGESIZE_ZOOM_H_ROLL) * CIS_IMG_H_ROLL / ZOOM_H_ROLL;

        for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
        {
            xx = (x - ENLARGESIZE_ZOOM_W_ROLL) * CIS_IMG_W_ROLL / ZOOM_W_ROLL;

            value = InRawImg[yy * CIS_IMG_W_ROLL + xx];
			if(value >= m_Minus_Value)
			{
				value = (int)(value * m_SBDAlg->diff_image_withMinusMean_preview[yy/2*CIS_IMG_W/2+xx/2+offset_x/2] / 1024)  + value - m_Minus_Value;
				if (value > 255)
				{
					value = 255;
				}
				else if (value < 0)
				{
					value = 0;
				}
				EnlargeBuf[y*ZOOM_ENLAGE_W_ROLL+x] = value;
			}
        }
    }

	memcpy(EnlargeBuf_Org, EnlargeBuf, ZOOM_ENLAGE_H_ROLL * ZOOM_ENLAGE_W_ROLL);

    _Algo_HistogramStretchForZoomEnlarge_Kojak_Roll(EnlargeBuf);

    mean = 0;
    count = 0;
    for (i = ENLARGESIZE_ZOOM_H_ROLL; i < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; i++)
    {
        for (j = ENLARGESIZE_ZOOM_W_ROLL; j < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; j++)
        {
            if (EnlargeBuf[i*ZOOM_ENLAGE_W_ROLL+j] > 10)
            {
                mean += EnlargeBuf[i*ZOOM_ENLAGE_W_ROLL+j];
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

                if (tempsum >= 1020)//4 * 255)
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

            //if (EnlargeBuf_Org[(y)*ZOOM_ENLAGE_W_ROLL + (x)] >= 1)//Threshold)
            {
                mean += EnlargeBuf_Org[(y) * ZOOM_ENLAGE_W_ROLL + (x)];
                count++;
                sum_x += x * ZOOM_OUT_ROLL;
                sum_y += y * ZOOM_OUT_ROLL;
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
    for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
    {
        COUNT = 0;
        for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
        {
            //			if(m_segment_enlarge_buffer[y*ZOOM_ENLAGE_W+x]==255)
            if (EnlargeBuf[y * ZOOM_ENLAGE_W_ROLL + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            m_cImgAnalysis.LEFT = (x - ENLARGESIZE_ZOOM_W_ROLL) * CIS_IMG_W_ROLL / ZOOM_W_ROLL - 30;
            //			m_cImgAnalysis.LEFT=(x-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W - 16;
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
            //			if(m_segment_enlarge_buffer[y*ZOOM_ENLAGE_W+x]==255)
            if (EnlargeBuf[y * ZOOM_ENLAGE_W_ROLL + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
			m_cImgAnalysis.RIGHT = (x - ENLARGESIZE_ZOOM_W_ROLL) * CIS_IMG_W_ROLL / ZOOM_W_ROLL + 30;
            //			m_cImgAnalysis.RIGHT=(x-ENLARGESIZE_ZOOM_W)* CIS_IMG_W / ZOOM_W + 16;
            if (m_cImgAnalysis.RIGHT > CIS_IMG_W_ROLL - 2)
            {
                m_cImgAnalysis.RIGHT = CIS_IMG_W_ROLL - 2;
            }
            break;
        }
    }

    for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
    {
        COUNT = 0;
        for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
        {
            //			if(m_segment_enlarge_buffer[y*ZOOM_ENLAGE_W+x]==255)
            if (EnlargeBuf[y * ZOOM_ENLAGE_W_ROLL + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            m_cImgAnalysis.TOP = (y - ENLARGESIZE_ZOOM_H_ROLL) * CIS_IMG_H_ROLL / ZOOM_H_ROLL - 30 - 16;
            //			m_cImgAnalysis.TOP=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H - 16;
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
            //			if(m_segment_enlarge_buffer[y*ZOOM_ENLAGE_W+x]==255)
            if (EnlargeBuf[y * ZOOM_ENLAGE_W_ROLL + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            m_cImgAnalysis.BOTTOM = (y - ENLARGESIZE_ZOOM_H_ROLL) * CIS_IMG_H_ROLL / ZOOM_H_ROLL + 30 + 16;
            //			m_cImgAnalysis.BOTTOM=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H + 16;
            if (m_cImgAnalysis.BOTTOM > CIS_IMG_H_ROLL - 2)
            {
                m_cImgAnalysis.BOTTOM = CIS_IMG_H_ROLL - 2;
            }
            break;
        }
    }

    return BrightValue;
}

BOOL CIBAlgorithm::_Algo_Kojak_AutoCapture(CaptureSetting *pSetting, int frameTime, int ImgSize, int *CaptureGood)
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
		m_cImgAnalysis.is_complete_voltage_control = FALSE;
		m_cImgAnalysis.detected_frame_count = 0;
		m_cImgAnalysis.JudgeMeanMode = 0;
		m_cImgAnalysis.JudgeMeanModeCount = 0;

		m_cImgAnalysis.sum_mean = 0;
		m_cImgAnalysis.bChangedMIN_BRIGHT = FALSE;
		m_cImgAnalysis.isChangetoSuperDry = 0;

		return FALSE;
	}

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
				pSetting->CurrentDAC -= 0x10;//_DAC_FOR_DRY_FOUR_FINGER_;
				if(pSetting->CurrentDAC <= 0xC9)
					pSetting->CurrentDAC = 0xC9;

//			}
		}
		
		return FALSE;
	}
/*
	if(m_cImgAnalysis.isChangetoSuperDry >= 3)
	{
		updated_MIN_BRIGHT = _TARGET_MIN_BRIGHTNESS_FOR_DRY_;
		updated_MAX_BRIGHT = _TARGET_MAX_BRIGHTNESS_FOR_DRY_;
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
		m_cImgAnalysis.is_complete_voltage_control = FALSE;
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

			if (pSetting->CurrentDAC <= pSetting->DefaultMaxDAC)
			{
				m_cImgAnalysis.is_complete_voltage_control = TRUE;
			}

			if (!m_cImgAnalysis.is_complete_voltage_control)
			{
				// Let's change voltage
				if (m_cImgAnalysis.mean > (updated_MIN_BRIGHT - TOLERANCE))
				{
					if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
						pSetting->CurrentDAC -= 1;
					else
						pSetting->CurrentDAC -= 3;
				}
				else if (m_cImgAnalysis.mean > (updated_MIN_BRIGHT - TOLERANCE*3/2))
				{
					if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
						pSetting->CurrentDAC -= 2;
					else
						pSetting->CurrentDAC -= 6;
				}
				else
				{
					if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
						pSetting->CurrentDAC -= 3;
					else
						pSetting->CurrentDAC -= 9;
				}			

				if (pSetting->CurrentDAC < pSetting->DefaultMaxDAC)
				{
					pSetting->CurrentDAC = pSetting->DefaultMaxDAC;
				}
			}
			else
			{
				// Let's change gain
				if (m_cImgAnalysis.mean > (updated_MIN_BRIGHT - TOLERANCE))
				{
					pSetting->CurrentGain += 0x0001;
				}
				else
				{
					pSetting->CurrentGain += 0x0002;
				}

				if (pSetting->CurrentGain > MAX_GAIN)
				{
					pSetting->CurrentGain = MAX_GAIN;
				}
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

			if (pSetting->CurrentDAC >= pSetting->DefaultMinDAC)
			{
				m_cImgAnalysis.is_complete_voltage_control = TRUE;
			}

			if (!m_cImgAnalysis.is_complete_voltage_control)
			{
				// Let's change voltage
				if (m_cImgAnalysis.mean < (updated_MAX_BRIGHT + TOLERANCE))
				{
					if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
						pSetting->CurrentDAC += 1;
					else
						pSetting->CurrentDAC += 2;
				}
				else if (m_cImgAnalysis.mean < (updated_MAX_BRIGHT + TOLERANCE*3/2))
				{
					if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
						pSetting->CurrentDAC += 2;
					else
						pSetting->CurrentDAC += 4;
				}
				else
				{
					if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
						pSetting->CurrentDAC += 3;
					else
						pSetting->CurrentDAC += 6;
				}

				if (pSetting->CurrentDAC > pSetting->DefaultMinDAC)
				{
					pSetting->CurrentDAC = pSetting->DefaultMinDAC;
				}
			}
			else
			{
				// Let's change gain
				if (m_cImgAnalysis.mean < (updated_MAX_BRIGHT + TOLERANCE))
				{
					pSetting->CurrentGain -= 0x0001;
				}
				else
				{
					pSetting->CurrentGain -= 0x0002;
				}

				if (pSetting->CurrentGain < MIN_GAIN)
				{
					pSetting->CurrentGain = MIN_GAIN;
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
			m_cImgAnalysis.mean >= updated_MIN_BRIGHT && m_cImgAnalysis.mean <= updated_MAX_BRIGHT/*  &&
			  abs(m_cImgAnalysis.center_x - m_cImgAnalysis.pre_center_x) <= 2 &&
			  abs(m_cImgAnalysis.center_y - m_cImgAnalysis.pre_center_y) <= 2*/
			  //			abs(CenterX-Prev_Prev_CenterX) <= 3 &&
			  //			abs(CenterY-Prev_Prev_CenterY) <= 3
																								  )
		{
			if ((m_cImgAnalysis.detected_frame_count * frameTime) > MIN_CAPTURE_TIME)
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

BOOL CIBAlgorithm::_Algo_Kojak_AutoCapture_CurrentSensor(CaptureSetting *pSetting, int frameTime, int ImgSize, int *CaptureGood)
{
//	int	ForgraoundCNT=0, fingerCount=0, AreaTres, RecomGain;
	int			AreaTres;
	int			FlatDiffThres;
	const int	MAX_BRIGHT = pSetting->TargetMaxBrightness, MIN_BRIGHT = pSetting->TargetMinBrightness;
	/*const*/ int	MIN_CAPTURE_TIME = 600;
	int			MIN_CAPTURE_TIME_FOR_DRY = 600;
	const int	TOLERANCE = 30;
	const int	MAX_STEP_CHANGE = 6;	// default 3
//	int			forSuperDry_Mode=pSetting->forSuperDry_Mode;
	int			REQUIRED_DETECT_FRAME_COUNT = pSetting->DetectionFrameCount;
	int			updated_MIN_BRIGHT = MIN_BRIGHT, updated_MAX_BRIGHT = MAX_BRIGHT;

	pSetting->PrevDAC = pSetting->CurrentDAC;

	if(m_pPropertyInfo->nWaitTimeForDry > 0)
		MIN_CAPTURE_TIME_FOR_DRY = m_pPropertyInfo->nWaitTimeForDry;

	*CaptureGood = FALSE;

	m_cImgAnalysis.max_same_gain_count = 3;

	if(m_pPropertyInfo->bEnableDecimation == FALSE)
	{
		MIN_CAPTURE_TIME = 1000;
		m_cImgAnalysis.max_same_gain_count = 5;
	}

	//if (m_cImgAnalysis.foreground_count < 100)
//	if (m_cImgAnalysis.isDetected == 0)
	if (!m_cImgAnalysis.is_final && m_cImgAnalysis.isDetected == 0 && m_cImgAnalysis.isDetectedTOF == FALSE)
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
		m_cImgAnalysis.is_complete_voltage_control = FALSE;
		m_cImgAnalysis.detected_frame_count = 0;
		m_cImgAnalysis.JudgeMeanMode = 0;
		m_cImgAnalysis.JudgeMeanModeCount = 0;

		m_cImgAnalysis.sum_mean = 0;
		m_cImgAnalysis.bChangedMIN_BRIGHT = FALSE;
		m_cImgAnalysis.isChangetoSuperDry = 0;

		return FALSE;
	}

	m_cImgAnalysis.detected_frame_count++;
	m_cImgAnalysis.frame_count++;
	m_cImgAnalysis.frame_delay--;

/*	if(!m_cImgAnalysis.is_final && m_cImgAnalysis.foreground_count < 100 && m_cImgAnalysis.isDetectedTOF == TRUE && 
		m_cImgAnalysis.finger_count == 0 && m_cImgAnalysis.detected_frame_count > 2)
	{
		//TRACE("Detected : Small Foreground < 100 \n");
		if(pSetting->CurrentDAC > _DAC_FOR_NORMAL_FINGER_WITH_TOF_ - 0x20)
			pSetting->CurrentDAC = pSetting->CurrentDAC - 0x10;
		else if(pSetting->CurrentDAC > _DAC_FOR_NORMAL_FINGER_WITH_TOF_ - 0x40)
			pSetting->CurrentDAC = pSetting->CurrentDAC - 0x08;
		else
			pSetting->CurrentDAC = pSetting->CurrentDAC - 0x04;
		
		if (pSetting->CurrentDAC < _DAC_FOR_MAXIMUM_TOF_)
			pSetting->CurrentDAC = _DAC_FOR_MAXIMUM_TOF_;

		if (m_cImgAnalysis.finger_count != m_cImgAnalysis.saved_finger_count)
		{
			m_cImgAnalysis.detected_frame_count = 0;
			pSetting->CurrentDAC = pSetting->DefaultDAC;
		}

		return FALSE;
	}*/

	// finger count 변경시
	if (m_cImgAnalysis.finger_count != m_cImgAnalysis.saved_finger_count)
	{
		if(m_cImgAnalysis.finger_count == 0)
		{
			m_cImgAnalysis.detected_frame_count = 0;
			pSetting->CurrentDAC = pSetting->DefaultDAC;
		}

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
		m_cImgAnalysis.JudgeMeanMode = 0;
		m_cImgAnalysis.JudgeMeanModeCount = 0;

		m_cImgAnalysis.sum_mean = 0;
		m_cImgAnalysis.bChangedMIN_BRIGHT = FALSE;
	}

	int DACThres = 10;

	//DACThres = 10 + (0xFF - pSetting->CurrentDAC) * 10 / (0xFF - 0xA0);

	if( ( m_cImgAnalysis.mean < DACThres && m_mainTOFValue > 1800 ) || m_mainTOFValue > 3000 )//2500)
	{
		pSetting->CurrentDAC += 3;
		if (pSetting->CurrentDAC > pSetting->DefaultMinDAC)
		{
			pSetting->CurrentDAC = pSetting->DefaultMinDAC;
		}

		//m_vecCaptureData.clear();

		return TRUE;
	}

	if( (m_cImgAnalysis.mean >= updated_MIN_BRIGHT && 
		m_cImgAnalysis.mean <= updated_MAX_BRIGHT && 
		m_mainTOFValue > 2500) )
	/*if( (m_SBDAlg->m_SegmentBright >= updated_MIN_BRIGHT && 
		m_SBDAlg->m_SegmentBright <= updated_MAX_BRIGHT && 
		m_mainTOFValue > 2800) )*/
	{
		goto capture_logic;
	}

	if (m_cImgAnalysis.frame_delay < 0 &&
		(!m_cImgAnalysis.is_final || m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER ||
		m_cImgAnalysis.mean < updated_MIN_BRIGHT || m_cImgAnalysis.mean > updated_MAX_BRIGHT) &&
		/*m_cImgAnalysis.changed_gain_step_count < MAX_STEP_CHANGE &&*/
		m_cImgAnalysis.detected_frame_count >= REQUIRED_DETECT_FRAME_COUNT &&		//Frame Speed에 따라서 10라는 숫자를 1초에 맞게 SDK를 변경하도록 한다.		D&G 
		m_cImgAnalysis.JudgeMeanMode == 0)
	{
		if (m_cImgAnalysis.mean < updated_MIN_BRIGHT)
		{

            if (m_cImgAnalysis.changed_gain_step_count > 2/* || m_cImgAnalysis.mean > (MIN_BRIGHT-TOLERANCE)*/)
			{
				m_cImgAnalysis.frame_delay = 1;
			}

			if (pSetting->CurrentDAC <= pSetting->DefaultMaxDAC)
			{
				m_cImgAnalysis.is_complete_voltage_control = TRUE;
			}

			if (!m_cImgAnalysis.is_complete_voltage_control)
			{
				// Let's change voltage
				if (m_cImgAnalysis.mean > (updated_MIN_BRIGHT - TOLERANCE))
				{
					if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
						pSetting->CurrentDAC -= 1;
					else
						pSetting->CurrentDAC -= 4;//3;
				}
				else if (m_cImgAnalysis.mean > (updated_MIN_BRIGHT - TOLERANCE*3/2))
				{
					if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
						pSetting->CurrentDAC -= 2;
					else
						pSetting->CurrentDAC -= 8;//6;
				}
				else
				{
					if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
						pSetting->CurrentDAC -= 3;
					else
						pSetting->CurrentDAC -= 12;//9;
				}			

				if (pSetting->CurrentDAC < pSetting->DefaultMaxDAC)
				{
					pSetting->CurrentDAC = pSetting->DefaultMaxDAC;
				}
			}
			else
			{
				// Let's change gain
				if (m_cImgAnalysis.mean > (updated_MIN_BRIGHT - TOLERANCE))
				{
					pSetting->CurrentGain += 0x0001;
				}
				else
				{
					pSetting->CurrentGain += 0x0002;
				}

				if (pSetting->CurrentGain > MAX_GAIN)
				{
					pSetting->CurrentGain = MAX_GAIN;
				}
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

			if (pSetting->CurrentDAC >= pSetting->DefaultMinDAC)
			{
				m_cImgAnalysis.is_complete_voltage_control = TRUE;
			}

			if (!m_cImgAnalysis.is_complete_voltage_control)
			{
				// Let's change voltage
				if (m_cImgAnalysis.mean < (updated_MAX_BRIGHT + TOLERANCE))
				{
					if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
						pSetting->CurrentDAC += 1;
					else
						pSetting->CurrentDAC += 2;
				}
				else if (m_cImgAnalysis.mean < (updated_MAX_BRIGHT + TOLERANCE*3/2))
				{
					if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
						pSetting->CurrentDAC += 2;
					else
						pSetting->CurrentDAC += 4;
				}
				else
				{
					if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_cImgAnalysis.is_final)
						pSetting->CurrentDAC += 3;
					else
						pSetting->CurrentDAC += 6;
				}

				if (pSetting->CurrentDAC > pSetting->DefaultMinDAC)
				{
					pSetting->CurrentDAC = pSetting->DefaultMinDAC;
				}
			}
			else
			{
				// Let's change gain
				if (m_cImgAnalysis.mean < (updated_MAX_BRIGHT + TOLERANCE))
				{
					pSetting->CurrentGain -= 0x0001;
				}
				else
				{
					pSetting->CurrentGain -= 0x0002;
				}

				if (pSetting->CurrentGain < MIN_GAIN)
				{
					pSetting->CurrentGain = MIN_GAIN;
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

capture_logic:

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
			m_cImgAnalysis.mean >= updated_MIN_BRIGHT && m_cImgAnalysis.mean <= updated_MAX_BRIGHT/*  &&
			  abs(m_cImgAnalysis.center_x - m_cImgAnalysis.pre_center_x) <= 2 &&
			  abs(m_cImgAnalysis.center_y - m_cImgAnalysis.pre_center_y) <= 2*/
			  //			abs(CenterX-Prev_Prev_CenterX) <= 3 &&
			  //			abs(CenterY-Prev_Prev_CenterY) <= 3
																								  )
		{
			if ((m_cImgAnalysis.detected_frame_count * frameTime) > MIN_CAPTURE_TIME)
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

void CIBAlgorithm::_Algo_RemoveNoise_Kojak(unsigned char *InImg, unsigned char *OutImg, int ImgSize, int ContrastLevel)
{
    int i, j, val;

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

	if( m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		memset(OutImg, 0, CIS_IMG_SIZE);
		for(i=0; i<CIS_IMG_H_ROLL; i++)
		{
			for(j=(CIS_IMG_W-CIS_IMG_W_ROLL)/2; j<(CIS_IMG_W+CIS_IMG_W_ROLL)/2; j++)
			{
				val = (int)InImg[i*CIS_IMG_W+j] - minus_value;
				if (val < 0)
				{
					val = 0;
				}
				OutImg[i*CIS_IMG_W+j] = val;//Contrast_LUT[InImg[i]];
			}
		}

		int count;
		unsigned char *TmpImg = new unsigned char [ImgSize];
		memcpy(TmpImg, OutImg, ImgSize);

		// 추가적인 dot noise 제거
		for(i=1; i<CIS_IMG_H_ROLL-1; i++)
		{
			for(j=(CIS_IMG_W-CIS_IMG_W_ROLL)/2+1; j<(CIS_IMG_W+CIS_IMG_W_ROLL)/2-1; j++)
			{
				if(TmpImg[i*CIS_IMG_W+j] > 0)
				{
					count=0;
					if(TmpImg[(i-1)*CIS_IMG_W+j-1] < 5)	count++;
					if(TmpImg[(i-1)*CIS_IMG_W+j] < 5 )	count++;
					if(TmpImg[(i-1)*CIS_IMG_W+j+1] < 5)	count++;
					if(TmpImg[i*CIS_IMG_W+j-1] < 5)	count++;
					if(TmpImg[i*CIS_IMG_W+j+1] < 5)	count++;
					if(TmpImg[(i+1)*CIS_IMG_W+j-1] < 5)	count++;
					if(TmpImg[(i+1)*CIS_IMG_W+j] < 5)	count++;
					if(TmpImg[(i+1)*CIS_IMG_W+j+1] < 5)	count++;

					if(count > 6)
						OutImg[i*CIS_IMG_W+j] = 0;
				}
			}
		}

		delete [] TmpImg;
	}
	else if( m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER || 
			 m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_TWO_FINGERS  ||
			 m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
		     m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
	{
		for (i = 0; i < ImgSize; i++)
		{
			val = (int)InImg[i] - minus_value;
			if (val < 0)
			{
				val = 0;
			}
			OutImg[i] = val;//Contrast_LUT[InImg[i]];
		}

		int j, count;
		unsigned char *TmpImg = new unsigned char [ImgSize];
		memcpy(TmpImg, OutImg, ImgSize);

		// 추가적인 dot noise 제거
		for (i=1; i<CIS_IMG_H-1; i++)
		{
			for (j=1; j<CIS_IMG_W-1; j++)
			{
				if(TmpImg[i*CIS_IMG_W+j] > 0)
				{
					count=0;
					if(TmpImg[(i-1)*CIS_IMG_W+j-1] < 5)	count++;
					if(TmpImg[(i-1)*CIS_IMG_W+j] < 5 )	count++;
					if(TmpImg[(i-1)*CIS_IMG_W+j+1] < 5)	count++;
					if(TmpImg[i*CIS_IMG_W+j-1] < 5)	count++;
					if(TmpImg[i*CIS_IMG_W+j+1] < 5)	count++;
					if(TmpImg[(i+1)*CIS_IMG_W+j-1] < 5)	count++;
					if(TmpImg[(i+1)*CIS_IMG_W+j] < 5)	count++;
					if(TmpImg[(i+1)*CIS_IMG_W+j+1] < 5)	count++;

					if(count > 6)
						OutImg[i*CIS_IMG_W+j] = 0;
				}
			}
		}

		delete [] TmpImg;
	}
}

void CIBAlgorithm::_Algo_Process_AnalysisImage_Kojak(unsigned char *InImg)
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

	if( m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		for (y = 0; y < 73; y++)
		{
			y_index = y * CIS_IMG_W;
			for (x = (CIS_IMG_W-CIS_IMG_W_ROLL)/2; x < (CIS_IMG_W-CIS_IMG_W_ROLL)/2+64; x++)
			{
				value = InImg[y_index + x];
				if (noise_histo[0] < value && value < 60)
				{
					noise_histo[0] = value;
				}
			}
			for (x = (CIS_IMG_W+CIS_IMG_W_ROLL)/2-64; x < (CIS_IMG_W+CIS_IMG_W_ROLL)/2; x++)
			{
				value = InImg[y_index + x];
				if (noise_histo[1] < value && value < 60)
				{
					noise_histo[1] = value;
				}
			}
		}
		for (y = CIS_IMG_H_ROLL - 73; y < CIS_IMG_H_ROLL; y++)
		{
			y_index = y * CIS_IMG_W;
			for (x = (CIS_IMG_W-CIS_IMG_W_ROLL)/2; x < (CIS_IMG_W-CIS_IMG_W_ROLL)/2+64; x++)
			{
				value = InImg[y_index + x];
				if (noise_histo[2] < value && value < 60)
				{
					noise_histo[2] = value;
				}
			}
			for (x = (CIS_IMG_W+CIS_IMG_W_ROLL)/2-64; x < (CIS_IMG_W+CIS_IMG_W_ROLL)/2; x++)
			{
				value = InImg[y_index + x];
				if (noise_histo[3] < value && value < 60)
				{
					noise_histo[3] = value;
				}
			}
		}
	}
	else if( m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
			 m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
			 m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
		     m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
	{
		for (y = 0; y < 73; y++)
		{
			y_index = y * CIS_IMG_W;
			for (x = 0; x < 64; x++)
			{
				value = InImg[y_index + x];
				if (noise_histo[0] < value && value < 60)
				{
					noise_histo[0] = value;
				}
			}
			for (x = CIS_IMG_W - 64; x < CIS_IMG_W; x++)
			{
				value = InImg[y_index + x];
				if (noise_histo[1] < value && value < 60)
				{
					noise_histo[1] = value;
				}
			}
		}
		for (y = CIS_IMG_H - 73; y < CIS_IMG_H; y++)
		{
			y_index = y * CIS_IMG_W;
			for (x = 0; x < 64; x++)
			{
				value = InImg[y_index + x];
				if (noise_histo[2] < value && value < 60)
				{
					noise_histo[2] = value;
				}
			}
			for (x = CIS_IMG_W - 64; x < CIS_IMG_W; x++)
			{
				value = InImg[y_index + x];
				if (noise_histo[3] < value && value < 60)
				{
					noise_histo[3] = value;
				}
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

int CIBAlgorithm::_Algo_JudgeMeanOr80Mask(unsigned char *InImg, int ImgSize)
{
	int i;
	int ridge_pixel_tres=100;
	int ridge_pixel_cnt=0;
	int ridge_pixel_255_tres=200;
	int ridge_pixel_255_cnt=0;
	int ridge_pixel_255_percent=0;

	for(i=0; i<ImgSize; i++)
	{
		if(InImg[i]>ridge_pixel_tres)
			ridge_pixel_cnt++;
		if(InImg[i]>=ridge_pixel_255_tres)
			ridge_pixel_255_cnt++;
	}
	if(ridge_pixel_cnt>0)
		ridge_pixel_255_percent = ridge_pixel_255_cnt*100 / ridge_pixel_cnt;

	return ridge_pixel_255_percent;
}

int CIBAlgorithm::_Algo_CheckAppendixFQuality(unsigned char *InImgArr, int *WidthArr, int *HeightArr, int SegmentCnt)
{
	int i;
	unsigned char *ImgPtr;
	int Width, Height;
	int nRc[4], imgsize = m_pUsbDevInfo->ImgSize;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		imgsize = m_pUsbDevInfo->ImgSize_Roll;

	for(i=0; i<SegmentCnt; i++)
	{
		ImgPtr = &InImgArr[i*imgsize];
		Width = WidthArr[i];
		Height = HeightArr[i];

		nRc[i] = _Algo_AnalysisHistogram(ImgPtr, Width, Height, i);

//		CString str;
//		str.Format("d:\\debugging image\\%d.bmp", i);
//		m_SBDAlg->SaveBitmapImage((char*)str.GetBuffer(), ImgPtr, Width, Height);

		m_pUsbDevInfo->CapturedBrightness[i] = nRc[i];

//		if(nRc != IBSU_STATUS_OK)
//			return nRc;
	}

	for(i=0; i<SegmentCnt; i++)
	{
		if(nRc[i] != IBSU_STATUS_OK)
			return nRc[i];
	}

	return IBSU_STATUS_OK;
}

int CIBAlgorithm::_Algo_AnalysisHistogram(unsigned char *InImg, int Width, int Height, int index)
{
	int histogram[256];
	int i;
	int total_bright_count = 0;
	int total_over_240_count = 0;
	int total_over_250_count = 0;
	int brightness = 0;
	float ratio1, ratio2;
	int dynamic_range = 0;

	memset(histogram, 0, sizeof(histogram));

	for(i=0; i<Width*Height; i++)
	{
		histogram[InImg[i]]++;
	}

	for(i=0; i<256; i++)
	{
		if(histogram[i] > 0)
			dynamic_range++;
	}

	for(i=5; i<256; i++)
	{
		total_bright_count += histogram[i];
	}

	total_bright_count = m_segment_arr.Segment[index].Area * ZOOM_OUT * ZOOM_OUT;
	
	for(i=5; i<256; i++)
	{
		brightness += histogram[i]*i;
	}
	
	if(total_bright_count > 0)
		brightness /= total_bright_count;

	for(i=240; i<256; i++)
	{
		total_over_240_count += histogram[i];
	}

	for(i=250; i<256; i++)
	{
		total_over_250_count += histogram[i];
	}

	ratio1 = (float)total_over_240_count / (float)total_bright_count * 100.0f;
	ratio2 = (float)total_over_250_count / (float)total_bright_count * 100.0f;
	
#ifdef __G_DEBUG__
	TRACE("ratio1 : %f(%d), ratio2 : %f(%d), brightness : %d, dynamic_range : %d\n", 
		ratio1, total_over_240_count, ratio2, total_over_250_count, brightness, dynamic_range);
#endif

	if(ratio1 > 2.0 || ratio2 > 0.2 || brightness > 130 || dynamic_range < 200)
	{
		return IBSU_WRN_INVALID_BRIGHTNESS_FINGERS;
	}

	return IBSU_STATUS_OK;
}

int CIBAlgorithm::_Algo_CheckWetFinger(unsigned char *InImgArr, int *WidthArr, int *HeightArr, int SegmentCnt)
{
	int i;
	unsigned char *ImgPtr;
	int Width, Height;
	int nRc[4], imgsize = m_pUsbDevInfo->ImgSize;
	int WetRatio;

	memset(m_pUsbDevInfo->CapturedBrightness, 0, sizeof(m_pUsbDevInfo->CapturedBrightness));
	memset(m_pUsbDevInfo->CapturedWetRatio, 0, sizeof(m_pUsbDevInfo->CapturedWetRatio));

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		imgsize = m_pUsbDevInfo->ImgSize_Roll;

	for(i=0; i<SegmentCnt; i++)
	{
		ImgPtr = &InImgArr[i*imgsize];
		Width = WidthArr[i];
		Height = HeightArr[i];

//		CString str;
//		str.Format("d:\\debugging image\\%d.bmp", i);
//		m_SBDAlg->SaveBitmapImage((char*)str.GetBuffer(), ImgPtr, Width, Height);

		nRc[i] = _Algo_AnalysisBlockStd_forWet(ImgPtr, Width, Height, i, &WetRatio);

//		str.Format("d:\\debugging image\\%d_wet.bmp", i);
//		m_SBDAlg->SaveBitmapImage((char*)str.GetBuffer(), ImgPtr, Width, Height);

		m_pUsbDevInfo->CapturedBrightness[i] = nRc[i];
		m_pUsbDevInfo->CapturedWetRatio[i] = WetRatio;

//		if(nRc != IBSU_STATUS_OK)
//			return nRc;
	}

	for(i=0; i<SegmentCnt; i++)
	{
		if(nRc[i] != IBSU_STATUS_OK)
			return nRc[i];
	}

	return IBSU_STATUS_OK;
}

int CIBAlgorithm::_Algo_AnalysisBlockStd_forWet(unsigned char *InImg, int Width, int Height, int index, int *WetRatio)
{
	int i, j;
	
	int *SumGrayBuf1 = m_SBDAlg->m_SumGrayBuf;
	int *SumGrayBuf2 = m_SBDAlg->m_SumGrayBuf2;
	unsigned char *pInImg = (unsigned char*)m_SBDAlg->m_ResultImg_int;
	
	int dtLEFT = 0;
	int dtTOP = 0;
	int dtRIGHT = Width;
	int dtBOTTOM = Height;
	int left, right, top, bottom;
	int xx, yy;
	int width, height, mean_val = 0, mean_val_cnt = 0;
	float mean_d_local, std_d_local;
	int WindowX = 12, WindowY = 12, step = 6;
	int BackgroundCount, WetAreaCount, TotalCount;
	int wet_threshold;
//	CString str;

	memcpy(pInImg, InImg, Width*Height);
	_ALGO_HistogramStretch(pInImg, Width, Height);

//	str.Format("d:\\debugging image\\%d_process.bmp", index);
//	m_SBDAlg->SaveBitmapImage((char*)str.GetBuffer(), pInImg, Width, Height);

	// total mean_val;
	for(i=0; i<Width*Height; i++)
	{
		if(pInImg[i] > 5)
		{
			mean_val += pInImg[i];
			mean_val_cnt++;
		}
	}
	if(mean_val_cnt > 0)
		mean_val = mean_val / mean_val_cnt;

	memset(SumGrayBuf1, 0, Width * Height * 4);
	memset(SumGrayBuf2, 0, Width * Height * 4);
	SumGrayBuf1[dtTOP*Width+dtLEFT] = pInImg[dtTOP*Width+dtLEFT];
	SumGrayBuf2[dtTOP*Width+dtLEFT] = pInImg[dtTOP*Width+dtLEFT]*pInImg[dtTOP*Width+dtLEFT];

	for(xx=dtLEFT+1; xx<dtRIGHT; xx++)
	{
		SumGrayBuf1[dtTOP*Width+xx] = SumGrayBuf1[dtTOP*Width+xx-1] + pInImg[dtTOP*Width+xx];
		SumGrayBuf2[dtTOP*Width+xx] = SumGrayBuf2[dtTOP*Width+xx-1] + pInImg[dtTOP*Width+xx]*pInImg[dtTOP*Width+xx];
	}

	for(yy=dtTOP+1; yy<dtBOTTOM; yy++)
	{
		SumGrayBuf1[yy*Width+dtLEFT] = SumGrayBuf1[(yy-1)*Width+dtLEFT] + pInImg[yy*Width+dtLEFT];
		SumGrayBuf2[yy*Width+dtLEFT] = SumGrayBuf2[(yy-1)*Width+dtLEFT] + pInImg[yy*Width+dtLEFT]*pInImg[yy*Width+dtLEFT];
	}

	for(yy=dtTOP+1; yy<dtBOTTOM; yy++)
	{
		for(xx=dtLEFT+1; xx<dtRIGHT; xx++)
		{
			SumGrayBuf1[yy*Width+xx] = SumGrayBuf1[yy*Width+(xx-1)]
												+ SumGrayBuf1[(yy-1)*Width+xx]
												+ pInImg[yy*Width+xx]
												- SumGrayBuf1[(yy-1)*Width+(xx-1)];
			SumGrayBuf2[yy*Width+xx] = SumGrayBuf2[yy*Width+(xx-1)]
												+ SumGrayBuf2[(yy-1)*Width+xx]
												+ pInImg[yy*Width+xx]*pInImg[yy*Width+xx]
												- SumGrayBuf2[(yy-1)*Width+(xx-1)];
		}
	}

	BackgroundCount = 0;
	WetAreaCount = 0;
	TotalCount = 0;
	for (i=dtTOP; i<dtBOTTOM; i+=step)
	{
        top = i;
        bottom = i + WindowY;

		if(bottom > Height-1) bottom = Height-1;

		height = (bottom-top);

		for (j=dtLEFT; j<dtRIGHT; j+=step)
		{
			left = j;
            right = j + WindowX;

			if(right > Width-1) right = Width-1;

			width = (right-left);

			mean_d_local = (float)(SumGrayBuf1[bottom * Width + right] - SumGrayBuf1[top * Width + right] - 
							SumGrayBuf1[bottom * Width + left] + SumGrayBuf1[top * Width + left]) / (float)(width*height);
            std_d_local = (float)(SumGrayBuf2[bottom * Width + right] - SumGrayBuf2[top * Width + right] - 
						   SumGrayBuf2[bottom * Width + left] + SumGrayBuf2[top * Width + left]) / (float)(width*height)
						  - mean_d_local * mean_d_local;

			//std_d_local = sqrt(std_d_local);

			if(mean_d_local < mean_val*2/3)//*2/5)	// background
			{
				BackgroundCount++;
/*				for (s=0; s<step; s++)
				{
					for (t=0; t<step; t++)
					{
						InImg[(i+s)*Width+(j+t)] = 100;
					}
				}*/
				continue;
			}

			int threshold;
			threshold = 2500;
			threshold = threshold - (int)(mean_d_local * 500 / mean_val);

			if(std_d_local < threshold)//2500)//1000)	// wet area std : 31.6
			{
				WetAreaCount++;
				//for (int s=0; s<step; s++)
				//{
				//	for (int t=0; t<step; t++)
				//	{
				//		InImg[(i+s)*Width+(j+t)] = 200;
				//	}
				//}
			}

			TotalCount++;
		}
	}

	int wet_ratio = 0;

	// using wet block count
	wet_ratio = WetAreaCount;
	*WetRatio = wet_ratio;
	wet_threshold = m_pPropertyInfo->nWetFingerDetectLevelThres[m_pPropertyInfo->nWetFingerDetectLevel-1];

#ifdef __G_DEBUG__
	TRACE("WetAreaCount : %d, TotalCount : %d\n", WetAreaCount, TotalCount);
#endif

	if(wet_ratio >= wet_threshold)
		return IBSU_WRN_WET_FINGERS;

	return IBSU_STATUS_OK;
}