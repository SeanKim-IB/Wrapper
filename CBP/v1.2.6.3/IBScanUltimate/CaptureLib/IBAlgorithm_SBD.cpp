#include "stdafx.h"
#include "IBAlgorithm_SBD.h"
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

CSBD_Alg::CSBD_Alg(UsbDeviceInfo *pUsbDevInfo, PropertyInfo *pPropertyInfo)
{
	m_pUsbDevInfo			= pUsbDevInfo;
	m_pPropertyInfo			= pPropertyInfo;

	CIS_IMG_W				= pUsbDevInfo->CisImgWidth;
    CIS_IMG_H				= pUsbDevInfo->CisImgHeight;
    CIS_IMG_SIZE			= CIS_IMG_H * CIS_IMG_W;
    IMG_W					= pUsbDevInfo->ImgWidth;
    IMG_H					= pUsbDevInfo->ImgHeight;
    IMG_SIZE				= IMG_H * IMG_W;

	IMG_W_ROLL		        = pUsbDevInfo->ImgWidth_Roll;
	IMG_H_ROLL		        = pUsbDevInfo->ImgHeight_Roll;
	IMG_SIZE_ROLL		    = IMG_W_ROLL * IMG_H_ROLL;
	CIS_IMG_W_ROLL	        = pUsbDevInfo->CisImgWidth_Roll;
	CIS_IMG_H_ROLL	        = pUsbDevInfo->CisImgHeight_Roll;
	CIS_IMG_SIZE_ROLL	    = CIS_IMG_W_ROLL*CIS_IMG_H_ROLL;

	int cis_img_size = CIS_IMG_W*CIS_IMG_H;
	if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
	{
		cis_img_size = (int)((CIS_IMG_W*1.65+64)*(CIS_IMG_H*1.65+64));

		m_ResultImg = new unsigned char [cis_img_size];						// 4MB
		m_SumGrayBuf = new int [cis_img_size];								// 16MB

		memset(m_ResultImg, 0, cis_img_size);
		memset(m_SumGrayBuf, 0, cis_img_size*sizeof(int));
	}
	
//	m_OriginalImg = new unsigned char [CIS_IMG_SIZE];					// 4MB
//	m_FirstFrameImg = new unsigned char [CIS_IMG_SIZE];					// 4MB
//	m_ResultImg = new unsigned char [CIS_IMG_SIZE];						// 4MB
//	m_ResultImg500 = new unsigned char [IMG_SIZE];						// 4MB
//	m_ResultImg_withMinus = new unsigned char [CIS_IMG_SIZE];			// 4MB
//	m_ResultImg500_withMinus = new unsigned char [IMG_SIZE];			// 4MB
//	tmpResult = new unsigned char [CIS_IMG_SIZE];						// 4MB
//	tmpResult2 = new unsigned char [CIS_IMG_SIZE];						// 4MB
	m_OriginalImg_int = new short [cis_img_size];						// 16MB
	m_ResultImg_int = new short [cis_img_size];							// 16MB
//	m_ResultImg2_int = new int [CIS_IMG_SIZE];							// 16MB
//	diff_image = new float [CIS_IMG_SIZE];								// 16MB
	diff_image_withMinus = new float [CIS_IMG_SIZE];					// 16MB
	diff_image_withMinusMean = new float [CIS_IMG_SIZE];				// 16MB
	diff_image_withMinusMean_preview = new short [CIS_IMG_SIZE/4];		// 4MB
//	m_SumGrayBuf = new int [CIS_IMG_SIZE];								// 16MB
	m_SumGrayBuf2 = new int [cis_img_size];								// 16MB
//	weight_table_bright = new double [CIS_IMG_SIZE];					// 32MB
//	weight_table_diff= new double [CIS_IMG_SIZE];						// 32MB
//	weight_table_bright = new float [cis_img_size];						// 32MB
	weight_table_diff= new float [cis_img_size];						// 32MB
//	weight_table_total= new double [CIS_IMG_SIZE];						// 32MB
	arrPos_wb = new DWORD [IMG_SIZE+1];									// 10MB
	arrPos_wb_KOJAK_ROLL = new DWORD [IMG_SIZE_ROLL+1];			// 10MB
	m_Stretch_LUT = new int [1024];

	m_OriginalImg = (unsigned char*)&m_ResultImg_int[0];

//	memset(m_OriginalImg, 0, CIS_IMG_SIZE);
//	memset(m_FirstFrameImg, 0, CIS_IMG_SIZE);
//	memset(m_ResultImg, 0, CIS_IMG_SIZE);
//	memset(m_ResultImg_withMinus, 0, CIS_IMG_SIZE);
//	memset(tmpResult, 0, CIS_IMG_SIZE);
//	memset(tmpResult2, 0, CIS_IMG_SIZE);
	memset(m_OriginalImg_int, 0, cis_img_size*sizeof(short));
	memset(m_ResultImg_int, 0, cis_img_size*sizeof(short));
//	memset(m_ResultImg2_int, 0, CIS_IMG_SIZE*sizeof(int));
	memset(m_Stretch_LUT, 0, 1024);
//	memset(diff_image, 0, CIS_IMG_SIZE*sizeof(float));
	memset(diff_image_withMinus, 0, CIS_IMG_SIZE*sizeof(float));
	memset(diff_image_withMinusMean, 0, CIS_IMG_SIZE*sizeof(float));
	memset(diff_image_withMinusMean_preview, 0, CIS_IMG_SIZE/4*sizeof(short));
//	memset(m_SumGrayBuf, 0, CIS_IMG_SIZE*sizeof(int));
	memset(m_SumGrayBuf2, 0, cis_img_size*sizeof(int));
//	memset(weight_table_bright, 0, CIS_IMG_SIZE*sizeof(float));
	memset(weight_table_diff, 0, cis_img_size*sizeof(float));
//	memset(weight_table_total, 0, CIS_IMG_SIZE*sizeof(double));
	memset(arrPos_wb, 0, IMG_SIZE*sizeof(DWORD));
	memset(arrPos_wb_KOJAK_ROLL, 0, IMG_SIZE_ROLL*sizeof(DWORD));

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
	{
		sprintf(m_strMinCutThres, "%s", "35");
		sprintf(m_strNoiseAmount, "%s", "40.0");
		sprintf(m_strBrightWeightRatio, "%s", "0.2");
		sprintf(m_strStretchPercent, "%s", "40.0");			// only for preview
		sprintf(m_strUnsharpParam, "%s", "1");				//Appendix F = 0.5
	}
	else
	{
		sprintf(m_strMinCutThres, "%s", "0");
		sprintf(m_strNoiseAmount, "%s", "10");
		sprintf(m_strBrightWeightRatio, "%s", "1");
		sprintf(m_strStretchPercent, "%s", "0");
		sprintf(m_strUnsharpParam, "%s", "1");				//Appendix F = 0.5
	}

	m_MinusMaskType = MINUS_MASK_MEAN;
	m_CMOS_NOISE_AMOUNT = 20;
	m_nContrastTres = 7;//6;//5;//7;

	m_Roll_Minus_Val = m_CMOS_NOISE_AMOUNT;//atoi(m_strMinCutThres);
}

CSBD_Alg::~CSBD_Alg()
{
//	delete [] m_OriginalImg;
//	delete [] m_FirstFrameImg;
//	delete [] m_ResultImg;
//	delete [] m_ResultImg500;
//	delete [] m_ResultImg_withMinus;
//	delete [] m_ResultImg500_withMinus;
//	delete [] tmpResult;
//	delete [] tmpResult2;
	delete [] m_OriginalImg_int;
	delete [] m_ResultImg_int;
//	delete [] m_ResultImg2_int;
	delete [] m_Stretch_LUT;
//	delete [] diff_image;
	delete [] diff_image_withMinus;
	delete [] diff_image_withMinusMean;
	delete [] diff_image_withMinusMean_preview;
	if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
	{
		delete [] m_ResultImg;
		delete [] m_SumGrayBuf;
	}
	delete [] m_SumGrayBuf2;
//	delete [] weight_table_bright;
	delete [] weight_table_diff;
//	delete [] weight_table_total;
	delete [] arrPos_wb;
	delete [] arrPos_wb_KOJAK_ROLL;
}

int CSBD_Alg::SaveBitmapImage(char *filename, unsigned char *buffer, int width, int height)
{
	FILE *fp = fopen(filename, "wb");
	if(fp == NULL)
		return FALSE;
	
	IB_BITMAPFILEHEADER header;
	IB_BITMAPINFO *Info = (IB_BITMAPINFO *)new unsigned char [1064];
	
	header.bfOffBits = 1078;
	header.bfReserved1 = 0;
	header.bfReserved2 = 0;
	header.bfSize = 1078 + width*height;
	header.bfType = (unsigned short)(('M'<<8) | 'B');
	
	for(int i=0; i<256; i++)
	{
		Info->bmiColors[i].rgbBlue = i;
		Info->bmiColors[i].rgbRed = i;
		Info->bmiColors[i].rgbGreen = i;
		Info->bmiColors[i].rgbReserved = 0;
	}
	Info->bmiHeader.biBitCount = 8;
	Info->bmiHeader.biClrImportant = 0;
	Info->bmiHeader.biClrUsed = 0;
	Info->bmiHeader.biCompression = BI_RGB;
	Info->bmiHeader.biHeight = height;
	Info->bmiHeader.biPlanes = 1;
	Info->bmiHeader.biSize = 40;
	Info->bmiHeader.biSizeImage = width*height;
	Info->bmiHeader.biWidth = width;
	Info->bmiHeader.biXPelsPerMeter = 0;
	Info->bmiHeader.biYPelsPerMeter = 0;
	
	fwrite(&header, 1, 14, fp);
	fwrite(Info, 1, 1064, fp);
	fwrite(buffer, 1, width*height, fp);
	fclose(fp);
	
	delete Info;
	
	return TRUE;
}

int CSBD_Alg::SaveBitmapImage_int(char *filename, short *buffer, int width, int height)
{
	FILE *fp = fopen(filename, "wb");
	if(fp == NULL)
		return FALSE;
	
	IB_BITMAPFILEHEADER header;
	IB_BITMAPINFO *Info = (IB_BITMAPINFO *)new unsigned char [1064];
	
	header.bfOffBits = 1078;
	header.bfReserved1 = 0;
	header.bfReserved2 = 0;
	header.bfSize = 1078 + width*height;
	header.bfType = (unsigned short)(('M'<<8) | 'B');
	
	for(int i=0; i<256; i++)
	{
		Info->bmiColors[i].rgbBlue = i;
		Info->bmiColors[i].rgbRed = i;
		Info->bmiColors[i].rgbGreen = i;
		Info->bmiColors[i].rgbReserved = 0;
	}
	Info->bmiHeader.biBitCount = 8;
	Info->bmiHeader.biClrImportant = 0;
	Info->bmiHeader.biClrUsed = 0;
	Info->bmiHeader.biCompression = BI_RGB;
	Info->bmiHeader.biHeight = height;
	Info->bmiHeader.biPlanes = 1;
	Info->bmiHeader.biSize = 40;
	Info->bmiHeader.biSizeImage = width*height;
	Info->bmiHeader.biWidth = width;
	Info->bmiHeader.biXPelsPerMeter = 0;
	Info->bmiHeader.biYPelsPerMeter = 0;
	
	fwrite(&header, 1, 14, fp);
	fwrite(Info, 1, 1064, fp);
	unsigned char *tmp = new unsigned char [width*height];
	for(int i=0; i<width*height; i++)
	{
		if(buffer[i] > 255)
			tmp[i] = 255;
		else
			tmp[i] = (unsigned char)buffer[i];
	}
	fwrite(tmp, 1, width*height, fp);
	fclose(fp);

	delete [] tmp;
	delete Info;
	
	return TRUE;
}

void CSBD_Alg::_Init_SBD_final(unsigned char *InImg)
{
	int i, j;
	float diff;
	int /*max_val=0,*/count=0;
	int gray_val;
//	int mask_size=51;
//	int half_mask_size=mask_size/2;
	int histogram[256], histo_sum;
	int mean, /*max_mean, */max_mean_xpos, max_mean_ypos, pixel_count;

	////////////////////////////////////////////////////////////////////////////////////////
	////Uniformity  mask를 적용할때 - 값 테이블과 + 값테이블을 균등하게 맞추기 위해
	////히스토그램의 평균값을 잡아서 테이블을 +,- 균등하게 적용되도록 만든다.
	///////////이미지의 평균값을 구한다....///////////////////
	int y,x,enlarge=100/*,center=0*/;
	memset(histogram,0,sizeof(int)*256);

	for(y=enlarge; y<CIS_IMG_H-enlarge; y++)
	{
		for(x=enlarge; x<CIS_IMG_W-enlarge; x++)
		{
			gray_val=InImg[y*CIS_IMG_W+x];
			histogram[gray_val]+=1;
		}
	}

	count=0;
	histo_sum=0;
	for(i=0; i<256; i++)
	{
		count+=histogram[i];
		histo_sum+=histogram[i]*i;
	}

	if(count==0)
		gray_val=200;
	else
		gray_val=histo_sum/count;

	for(i=0; i<CIS_IMG_SIZE; i++)
	{
		if(InImg[i] == 0)
		{
			diff_image_withMinusMean[i] = 0;
			continue;
		}

		diff = (float)gray_val-InImg[i];
		diff_image_withMinusMean[i] = diff/InImg[i];
	}

	//diff_image_withMinusMean[0] = (float)gray_val;

	////////////////////////////////////////////////////////////////////////////////////////
	// 원래의 방식으로 uniformity mask 초기화 : 가장 밝은 패치에서 상위 80% 위치를 기준
	// 가장 밝은 patch 위치 찾기  "-" uniformity mask 안하기
//	max_mean=0;
	max_mean_xpos=0;
	max_mean_ypos=0;

	_SearchingMaxGrayboxFast(InImg, &max_mean_xpos, &max_mean_ypos, &mean, histogram, 51);

	pixel_count=0;

	for(i=mean; i<256; i++)
		pixel_count += histogram[i];

	histo_sum=0;
	for(i=mean; i<256; i++)
	{
		histo_sum+=histogram[i];
		if(histo_sum > pixel_count*0.8)
		{
			gray_val = i;
			break;
		}
	}

	for(i=0; i<CIS_IMG_SIZE; i++)
	{
		if(InImg[i] == 0)
		{
			diff_image_withMinus[i] = 0;
			continue;
		}

		diff = (float)gray_val-InImg[i];
		if(diff < 0)
			diff = 0;
		diff_image_withMinus[i] = diff/InImg[i];
	}

	// preview
	int value;
	for(i=0; i<CIS_IMG_H/2; i++)
	{
		for(j=0; j<CIS_IMG_W/2; j++)
		{
			value = (int)(diff_image_withMinusMean[i*2*CIS_IMG_W+j*2] * 1024);
			if(value > 32767)
				value = 32767;
			else if(value < -32767)
				value = -32767;
			diff_image_withMinusMean_preview[i*CIS_IMG_W/2+j] = (short)value;
		}
	}

	for(i=0; i<30; i++)
	{
		m_StretchPreviewLUT[i] = 0;
	}
	for(i=30; i<256; i++)
	{
		m_StretchPreviewLUT[i] = (i-30)*255/(255-30);
	}

	// OpenDeviceEx로 열었을때 꼭 실행되야하는 부분
	int move_x_tgt = (IMG_W - IMG_W_ROLL)/2;
	int move_x_org = (CIS_IMG_W - CIS_IMG_W_ROLL)/2;
	int pos, pos_x, pos_y;
	int sp, sq;
	DWORD distortion_value;
	for(i=0; i<IMG_H_ROLL; i++)
	{
		for(j=0; j<IMG_W_ROLL; j++)
		{
			distortion_value = arrPos_wb[i*IMG_W+j+move_x_tgt];
			
			pos = (distortion_value>>10) & 0x3FFFFF;
			sp = (distortion_value>>5) & 0x1F;
			sq = distortion_value & 0x1F;

			pos_x = pos % CIS_IMG_W;
			pos_y = pos / CIS_IMG_W;
			pos = pos_y * CIS_IMG_W_ROLL + pos_x - move_x_org;

			distortion_value = (DWORD)((pos & 0x3FFFFF) << 10);
			distortion_value |= (DWORD)(sp << 5);
			distortion_value |= (DWORD)sq;

			arrPos_wb_KOJAK_ROLL[i*IMG_W_ROLL+j] = distortion_value;
		}
	}

	float curpos, dist;	
	int k,l;
	float Alpha = -1.4f;
	for(k=-2; k<=2; k++)
	{
		for(l=0; l<=32; l++)
		{
			curpos = (float)l/32.0f;
			dist = sqrtf((k-curpos)*(k-curpos));
			if(dist >= 0.0f && dist < 1.0f)
				Cubic_UV[k+2][l] = (Alpha + 2.0f) * dist*dist*dist - (Alpha + 3.0f) * dist*dist + 1.0f;
			else if(dist >= 1.0f && dist < 2.0f)
				Cubic_UV[k+2][l] = Alpha * dist*dist*dist - (5.0f * Alpha) * dist*dist + (8.0f * Alpha) * dist - 4.0f * Alpha;
			else
				Cubic_UV[k+2][l] = 0;
			
//			Cubic_UV_int[k+2][l] = (int)(Cubic_UV[k+2][l] * 1024);
		}
	}
}

void CSBD_Alg::_Apply_SBD_preview(unsigned char *InImg, unsigned char *OutImg)
{
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
#ifdef __embedded__
		_VignettingEffect_preview(InImg, OutImg,_NO_BIT_CMOS_NOISE_FOR_FULLFRAME_ROLL);//_NO_BIT_CMOS_NOISE_FOR_FULLFRAME_);
#else
		unsigned char *TmpBuf1 = (unsigned char *)&m_SumGrayBuf[CIS_IMG_SIZE_ROLL];
		_VignettingEffect_preview_roll(InImg, TmpBuf1, _NO_BIT_CMOS_NOISE_FOR_FULLFRAME_ROLL);//_NO_BIT_CMOS_NOISE_FOR_FULLFRAME_);
		_HistogramStretchPercent_preview(TmpBuf1, OutImg, CIS_IMG_W_ROLL, CIS_IMG_H_ROLL);
#endif
	}
	else
		_VignettingEffect_preview(InImg, OutImg,_DEFAULT_CMOS_NOISE_FOR_DECI_/2);//atoi(m_strMinCutThres)+20);
}


void CSBD_Alg::_Apply_SBD_final(unsigned char *InImg, unsigned char *OutImg)
{
	int MINIMUM_CIS_NOISE = m_CMOS_NOISE_AMOUNT;//atoi(m_strMinCutThres);// / 2.0 + 0.5;
	int i, j, val;
//	int DEF_LOOP_COUNT=1;
	int tmpCIS_IMG_W = CIS_IMG_W;
//	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT;
		dtRIGHT		= m_pcImgAnalysis->RIGHT;
		dtTOP		= m_pcImgAnalysis->TOP;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
		dtLEFT		= m_pcImgAnalysis->LEFT*2;
		dtRIGHT		= m_pcImgAnalysis->RIGHT*2;
		dtTOP		= m_pcImgAnalysis->TOP*2;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM*2;

		tmpCIS_IMG_W = CIS_IMG_W;
//		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
	}

	// convert unsigend char to integer
	memset(m_OriginalImg_int, 0, tmpCIS_IMG_SIZE*sizeof(short));

	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			m_OriginalImg_int[i*tmpCIS_IMG_W+j] = InImg[i*tmpCIS_IMG_W+j];
		}
	}


	// Apply UM Mask
//	_VignettingEffect(m_OriginalImg_int, m_ResultImg_int);
	_VignettingEffect_select_80_mean(m_OriginalImg_int, m_ResultImg_int);


	// Background Removal
//	memcpy(m_OriginalImg_int, m_ResultImg_int, CIS_IMG_SIZE*sizeof(int));
	memset(m_OriginalImg_int, 0, tmpCIS_IMG_SIZE*sizeof(short));
	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			if(m_ResultImg_int[i*tmpCIS_IMG_W+j]>255)
				m_ResultImg_int[i*tmpCIS_IMG_W+j]=255;
			val = m_ResultImg_int[i*tmpCIS_IMG_W+j] - MINIMUM_CIS_NOISE;
			if(val < 0)
				val = 0;
			m_OriginalImg_int[i*tmpCIS_IMG_W+j] = val;
		}
	}


	// Make linearity
	// VignettingEffect에서 변형된 linearity를 다시 원복시킴
//	memcpy(m_OriginalImg_int, m_ResultImg_int, CIS_IMG_SIZE*sizeof(int));
	_HistogramStretchUsingLUT_int(m_OriginalImg_int, m_ResultImg_int);

memset(m_OriginalImg_int, 0, tmpCIS_IMG_SIZE*sizeof(short));
for(i=dtTOP; i<dtBOTTOM; i++)
{
	for(j=dtLEFT; j<dtRIGHT; j++)
	{
		val=m_ResultImg_int[i*tmpCIS_IMG_W+j];//+150;
	if(val>1023)
		val=1023;
		m_OriginalImg_int[i*tmpCIS_IMG_W+j]=val;
	}
}

	// S
	// 대상 : 현재 영상이 가지고 있는 가장 밝은 화소 값의 80% 위치에 해당하는 화소들
	// 목적 : 해당 화소들의 개수들이 특정 값 이하일때 노이즈로 간주 --> 주변 값 평균으로 대체
//	memcpy(m_OriginalImg_int, m_ResultImg_int, CIS_IMG_SIZE*sizeof(int));
	_RemoveSpecificNoise_int(m_OriginalImg_int, m_ResultImg_int);

	// BD
	// 대상 : 픽셀단위로 blur weight 를 계산하여 적용
	// 목적 : 밝기가 높고, 편차가 낮은 영역을 많이 blur하기 위한 Tweight(0.0 ~ 1.0)를 설계
	// Tweight에 따라 Dmean과 현재 값을 적절히 blur함.
//	memcpy(m_OriginalImg_int, m_ResultImg_int, CIS_IMG_SIZE*sizeof(int));
	_StdDevSmoothing_int_opt(m_ResultImg_int, m_OriginalImg_int);

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtRIGHT		= m_pcImgAnalysis->RIGHT-3;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM-3;
	}
	else
	{
		dtRIGHT		= m_pcImgAnalysis->RIGHT*2-3;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM*2-3;
	}

	//memset(m_ResultImg_int, 0, CIS_IMG_SIZE*sizeof(int));
	//for(i=dtTOP; i<dtBOTTOM; i++)
	//{
	//	for(j=dtLEFT; j<dtRIGHT; j++)
	//	{
	//		val=m_ResultImg2_int[i*CIS_IMG_W+j]-150;
	//		if(val<0)
	//			val=0;
	//		m_ResultImg_int[i*CIS_IMG_W+j]=val;
	//	}
	//}

	// make an image from int to unsigned char
//	memcpy(m_OriginalImg_int, m_ResultImg_int, CIS_IMG_SIZE*sizeof(int));
	memset(m_OriginalImg, 0, tmpCIS_IMG_SIZE);
	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
	{
			val = m_OriginalImg_int[i*tmpCIS_IMG_W+j];//-150;
			if(val<0)
				val=0;
			else if(val > 255)
			val = 255;

			m_OriginalImg[i*tmpCIS_IMG_W+j] = (unsigned char)val;
		}
	}


	// unsharp
	// 지문의 Ridge Valley를 부각하기위해 Valley 부분에 -penalty를 부여함.
//	memcpy(m_OriginalImg, OutImg, CIS_IMG_SIZE);
	_Unsharp_opt(m_OriginalImg, OutImg);

	// remove dot noise
	// CMOS의 white defect를 제거하기 위해 주변은 0값이지만 자신은 값이 존재하는 위치를 판별
//	memcpy(m_OriginalImg, OutImg, CIS_IMG_SIZE);
	_RemoveSpotNoise(OutImg, m_OriginalImg);

	// histogram stretch
	// Dynamic range 확보를 위해 160 gray level을 200 gray level로 Stretch (25%)하는 기본 설계를 가짐
	// linear stretch이므로 linearity의 변화는 없음 
//	memcpy(m_OriginalImg, OutImg, CIS_IMG_SIZE);
//	_HistogramStretchPercent(m_OriginalImg, OutImg);
//	if(atoi(m_strStretchPercent) != 0)
//		_HistogramStretchPercent_222222(m_OriginalImg, OutImg);
//	else
//		memcpy(OutImg, m_OriginalImg, CIS_IMG_SIZE);
	_HistogramStretchPercent_222222_debug(m_OriginalImg, OutImg);
}

void CSBD_Alg::_Apply_SBD_final_opt(unsigned char *InImg, unsigned char *OutImg)
{
//	int MINIMUM_CIS_NOISE = m_CMOS_NOISE_AMOUNT;//atoi(m_strMinCutThres);// / 2.0 + 0.5;
	int i, j, val;
//	int DEF_LOOP_COUNT=1;
	int tmpCIS_IMG_W = CIS_IMG_W;
//	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT;
		dtRIGHT		= m_pcImgAnalysis->RIGHT;
		dtTOP		= m_pcImgAnalysis->TOP;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
		dtLEFT		= m_pcImgAnalysis->LEFT*2;
		dtRIGHT		= m_pcImgAnalysis->RIGHT*2;
		dtTOP		= m_pcImgAnalysis->TOP*2;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM*2;

		tmpCIS_IMG_W = CIS_IMG_W;
//		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
	}

	//SaveBitmapImage("d:\\debugging image\\inputimage2.bmp", InImg, tmpCIS_IMG_W, tmpCIS_IMG_H);

	_VignettingEffect_select_80_mean_opt(InImg, m_ResultImg_int);


	// Make linearity
	// VignettingEffect에서 변형된 linearity를 다시 원복시킴
//	memcpy(m_OriginalImg_int, m_ResultImg_int, CIS_IMG_SIZE*sizeof(int));
	_HistogramStretchUsingLUT_int_opt(m_ResultImg_int, m_OriginalImg_int);


	// S
	// 대상 : 현재 영상이 가지고 있는 가장 밝은 화소 값의 80% 위치에 해당하는 화소들
	// 목적 : 해당 화소들의 개수들이 특정 값 이하일때 노이즈로 간주 --> 주변 값 평균으로 대체
//	memcpy(m_OriginalImg_int, m_ResultImg_int, CIS_IMG_SIZE*sizeof(int));
	_RemoveSpecificNoise_int(m_OriginalImg_int, m_ResultImg_int);

	// BD
	// 대상 : 픽셀단위로 blur weight 를 계산하여 적용
	// 목적 : 밝기가 높고, 편차가 낮은 영역을 많이 blur하기 위한 Tweight(0.0 ~ 1.0)를 설계
	// Tweight에 따라 Dmean과 현재 값을 적절히 blur함.
//	memcpy(m_OriginalImg_int, m_ResultImg_int, CIS_IMG_SIZE*sizeof(int));
	_StdDevSmoothing_int_opt(m_ResultImg_int, m_OriginalImg_int);

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtRIGHT		= m_pcImgAnalysis->RIGHT-3;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM-3;
	}
	else
	{
		dtRIGHT		= m_pcImgAnalysis->RIGHT*2-3;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM*2-3;
	}

	// make an image from int to unsigned char
//	memcpy(m_OriginalImg_int, m_ResultImg_int, CIS_IMG_SIZE*sizeof(int));
	memset(m_OriginalImg, 0, tmpCIS_IMG_SIZE);
	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
	{
			val = m_OriginalImg_int[i*tmpCIS_IMG_W+j]-150;
			if(val<0)
				val=0;
			else if(val > 255)
			val = 255;

			m_OriginalImg[i*tmpCIS_IMG_W+j] = (unsigned char)val;
		}
	}


	// unsharp
	// 지문의 Ridge Valley를 부각하기위해 Valley 부분에 -penalty를 부여함.
	//memcpy(m_OriginalImg, OutImg, CIS_IMG_SIZE);
	_Unsharp_opt(m_OriginalImg, OutImg);
	//memcpy(OutImg, m_OriginalImg, tmpCIS_IMG_SIZE);

	// remove dot noise
	// CMOS의 white defect를 제거하기 위해 주변은 0값이지만 자신은 값이 존재하는 위치를 판별
//	memcpy(m_OriginalImg, OutImg, CIS_IMG_SIZE);
	_RemoveSpotNoise(OutImg, m_OriginalImg);

	// histogram stretch
	// Dynamic range 확보를 위해 160 gray level을 200 gray level로 Stretch (25%)하는 기본 설계를 가짐
	// linear stretch이므로 linearity의 변화는 없음 
//	memcpy(m_OriginalImg, OutImg, CIS_IMG_SIZE);
//	_HistogramStretchPercent(m_OriginalImg, OutImg);
//	if(atoi(m_strStretchPercent) != 0)
//		_HistogramStretchPercent_222222(m_OriginalImg, OutImg);
//	else
//		memcpy(OutImg, m_OriginalImg, CIS_IMG_SIZE);
	//_HistogramStretchPercent_222222_debug(m_OriginalImg, OutImg);
	_HistogramStretchPercent_gon_final(m_OriginalImg, OutImg);
}

void CSBD_Alg::_Apply_SBD_final_Five0(unsigned char *InImg, unsigned char *OutImg)
{
	int MINIMUM_CIS_NOISE = m_CMOS_NOISE_AMOUNT;//atoi(m_strMinCutThres);// / 2.0 + 0.5;
	int i, j, val;
//	int DEF_LOOP_COUNT=1;
	int tmpCIS_IMG_W = CIS_IMG_W;
	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT;
		dtRIGHT		= m_pcImgAnalysis->RIGHT;
		dtTOP		= m_pcImgAnalysis->TOP;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
		dtLEFT		= (m_pcImgAnalysis->LEFT-1)*2;
		dtRIGHT		= (m_pcImgAnalysis->RIGHT+1)*2;
		dtTOP		= (m_pcImgAnalysis->TOP-1)*2;
		dtBOTTOM	= (m_pcImgAnalysis->BOTTOM+1)*2;

		tmpCIS_IMG_W = CIS_IMG_W;
		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

		if(dtLEFT < 0) dtLEFT = 0;
		if(dtTOP < 0) dtTOP = 0;
		if(dtRIGHT > CIS_IMG_W) dtRIGHT = CIS_IMG_W;
		if(dtBOTTOM > CIS_IMG_H) dtBOTTOM = CIS_IMG_H;
	}

	// convert unsigend char to integer
	memset(m_ResultImg_int, 0, tmpCIS_IMG_SIZE*sizeof(short));

	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			m_ResultImg_int[i*tmpCIS_IMG_W+j] = InImg[i*tmpCIS_IMG_W+j];
		}
	}


	// Apply UM Mask
//	_VignettingEffect(m_OriginalImg_int, m_ResultImg_int);
	_VignettingEffect_select_80_mean(m_ResultImg_int, m_OriginalImg_int);


	// Background Removal
//	memcpy(m_OriginalImg_int, m_ResultImg_int, CIS_IMG_SIZE*sizeof(int));
	memset(m_ResultImg_int, 0, tmpCIS_IMG_SIZE*sizeof(short));
	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			if(m_OriginalImg_int[i*tmpCIS_IMG_W+j]>255)
				m_OriginalImg_int[i*tmpCIS_IMG_W+j]=255;
			val = m_OriginalImg_int[i*tmpCIS_IMG_W+j] - MINIMUM_CIS_NOISE;
			if(val < 0)
				val = 0;
			m_ResultImg_int[i*tmpCIS_IMG_W+j] = val;
		}
	}

	_HistogramStretchUsingLUT_int(m_ResultImg_int, m_OriginalImg_int);

	// S
	// 대상 : 현재 영상이 가지고 있는 가장 밝은 화소 값의 80% 위치에 해당하는 화소들
	// 목적 : 해당 화소들의 개수들이 특정 값 이하일때 노이즈로 간주 --> 주변 값 평균으로 대체
//	memcpy(m_OriginalImg_int, m_ResultImg_int, CIS_IMG_SIZE*sizeof(int));
	_RemoveSpecificNoise_int(m_OriginalImg_int, m_ResultImg_int);

	// BD
	// 대상 : 픽셀단위로 blur weight 를 계산하여 적용
	// 목적 : 밝기가 높고, 편차가 낮은 영역을 많이 blur하기 위한 Tweight(0.0 ~ 1.0)를 설계
	// Tweight에 따라 Dmean과 현재 값을 적절히 blur함.
//	memcpy(m_OriginalImg_int, m_ResultImg_int, CIS_IMG_SIZE*sizeof(int));
	_StdDevSmoothing_int_opt(m_ResultImg_int, m_OriginalImg_int);

	memset(m_OriginalImg, 0, tmpCIS_IMG_SIZE);
	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			val = m_OriginalImg_int[i*tmpCIS_IMG_W+j];//-150;
			if(val<0)
				val=0;
			else if(val > 255)
				val = 255;

			m_OriginalImg[i*tmpCIS_IMG_W+j] = (unsigned char)val;
		}
	}

	_Unsharp_opt(m_OriginalImg, OutImg);

	memcpy(m_OriginalImg, OutImg, tmpCIS_IMG_SIZE);
	_HistogramStretchPercent_222222_debug(m_OriginalImg, OutImg);

	// make fill the blank histogram
	memcpy(m_OriginalImg, OutImg, tmpCIS_IMG_SIZE);
	_HistogramFilling(m_OriginalImg, OutImg, tmpCIS_IMG_W, tmpCIS_IMG_H);
}

void CSBD_Alg::_Apply_SBD_final_Five0_double_enlarge(unsigned char *InImg, unsigned char *OutImg)
{
//	int MINIMUM_CIS_NOISE = m_CMOS_NOISE_AMOUNT;//atoi(m_strMinCutThres);// / 2.0 + 0.5;
	int i, j, val;
//	int DEF_LOOP_COUNT=1;
	int tmpCIS_IMG_W = CIS_IMG_W;
	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
	
	int tmpEnCIS_IMG_W = 0;
	int tmpEnCIS_IMG_H = 0;
	int tmpEnCIS_IMG_SIZE = 0;

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT;
		dtRIGHT		= m_pcImgAnalysis->RIGHT;
		dtTOP		= m_pcImgAnalysis->TOP;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
		dtLEFT		= (m_pcImgAnalysis->LEFT-1)*2;
		dtRIGHT		= (m_pcImgAnalysis->RIGHT+1)*2;
		dtTOP		= (m_pcImgAnalysis->TOP-1)*2;
		dtBOTTOM	= (m_pcImgAnalysis->BOTTOM+1)*2;

		tmpCIS_IMG_W = CIS_IMG_W;
		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

		if(dtLEFT < 0) dtLEFT = 0;
		if(dtTOP < 0) dtTOP = 0;
		if(dtRIGHT > CIS_IMG_W) dtRIGHT = CIS_IMG_W;
		if(dtBOTTOM > CIS_IMG_H) dtBOTTOM = CIS_IMG_H;
	}

	// convert unsigend char to integer
	memset(m_OriginalImg_int, 0, tmpCIS_IMG_SIZE*sizeof(short));

	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			m_OriginalImg_int[i*tmpCIS_IMG_W+j] = InImg[i*tmpCIS_IMG_W+j];
		}
	}

	// Apply UM Mask
	_VignettingEffect(m_OriginalImg_int, m_ResultImg_int, tmpCIS_IMG_W, tmpCIS_IMG_H);
//	SaveBitmapImage_int("D:\\debugging image\\five-1_22.bmp", m_ResultImg_int, tmpCIS_IMG_W, tmpCIS_IMG_H);

	// Make linearity
	_HistogramStretchUsingLUT_int(m_ResultImg_int, m_OriginalImg_int);
//	SaveBitmapImage_int("D:\\debugging image\\five-1_33.bmp", m_OriginalImg_int, tmpCIS_IMG_W, tmpCIS_IMG_H);

	dtLEFT = (int)(dtLEFT*1.65) + 32;
	dtRIGHT = (int)(dtRIGHT*1.65) + 32;
	dtTOP = (int)(dtTOP*1.65) + 32;
	dtBOTTOM = (int)(dtBOTTOM*1.65) + 32;

	tmpEnCIS_IMG_W = (int)(tmpCIS_IMG_W*1.65+64);
	tmpEnCIS_IMG_H = (int)(tmpCIS_IMG_H*1.65+64);
	tmpEnCIS_IMG_SIZE = tmpEnCIS_IMG_W*tmpEnCIS_IMG_H;

	memcpy(m_ResultImg_int, m_OriginalImg_int, tmpEnCIS_IMG_W*tmpEnCIS_IMG_H*sizeof(short));
	_DoubleEnlarge_int(m_ResultImg_int, tmpCIS_IMG_W, tmpCIS_IMG_H, m_OriginalImg_int, tmpEnCIS_IMG_W, tmpEnCIS_IMG_H);

//	SaveBitmapImage("D:\\debugging image\\five-1_11.bmp", tmpResult, tmpEnCIS_IMG_W, tmpEnCIS_IMG_H);


	// S
	_RemoveSpecificNoise_int_double_enlarge(m_OriginalImg_int, m_ResultImg_int, tmpEnCIS_IMG_W, tmpEnCIS_IMG_H);
//	SaveBitmapImage_int("D:\\debugging image\\five-1_44.bmp", m_ResultImg_int, tmpEnCIS_IMG_W, tmpEnCIS_IMG_H);

	// BD
	_StdDevSmoothing_int_opt_double_enlarge(m_ResultImg_int, m_OriginalImg_int, tmpEnCIS_IMG_W, tmpEnCIS_IMG_H);
//	SaveBitmapImage_int("D:\\debugging image\\five-1_55.bmp", m_OriginalImg_int, tmpEnCIS_IMG_W, tmpEnCIS_IMG_H);

	memset(m_OriginalImg, 0, tmpEnCIS_IMG_SIZE);
	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			val = m_OriginalImg_int[i*tmpEnCIS_IMG_W+j];//-150;
			if(val<0)
				val=0;
			else if(val > 255)
				val = 255;

			m_OriginalImg[i*tmpEnCIS_IMG_W+j] = (unsigned char)val;
		}
	}
	
	_Unsharp_opt_double_enlarge(m_OriginalImg, tmpEnCIS_IMG_W, tmpEnCIS_IMG_H, m_ResultImg);
//	SaveBitmapImage("D:\\debugging image\\five-1_66.bmp", m_ResultImg, tmpEnCIS_IMG_W, tmpEnCIS_IMG_H);

	_HistogramStretchPercent_222222_double_enlarge(m_ResultImg, tmpEnCIS_IMG_W, tmpEnCIS_IMG_H, m_OriginalImg);
//	SaveBitmapImage("D:\\debugging image\\five-1_77.bmp", m_OriginalImg, tmpEnCIS_IMG_W, tmpEnCIS_IMG_H);

	_DeDoubleEnlarge(m_OriginalImg, tmpEnCIS_IMG_W, tmpEnCIS_IMG_H, OutImg, tmpCIS_IMG_W, tmpCIS_IMG_H);
//	SaveBitmapImage("D:\\debugging image\\five-1_88.bmp", OutImg, tmpCIS_IMG_W, tmpCIS_IMG_H);
}

void CSBD_Alg::_VignettingEffect(short *InImg, short *OutImg, int width, int height)
{
	int i, j, val;
	int count=0;
	int mask_size=51;
//	int half_mask_size=mask_size/2;
	int sum;
	int histogram[1024];
	int max_mean, max_mean_xpos, max_mean_ypos;
//	int LOW_CUT_THRES=0;
//	int HIGH_CUT_THRES=0;
//	int Process_Count=0;
//	int MINIMUM_CIS_NOISE = atoi(m_strMinCutThres);
	int LinearityLUT[1024];

//	int mean=0;
	int offset_x = (CIS_IMG_W-CIS_IMG_W_ROLL)/2;
	int offset_y = m_pPropertyInfo->nStartingPositionOfRollArea * 27;
	int mask_width = CIS_IMG_W;

	if(m_pPropertyInfo->ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER)
		offset_x = 0;

	memcpy(OutImg, InImg, width*height*sizeof(short));

	if(m_MinusMaskType == MINUS_MASK_ORIGINAL)	// minus mask를 적용해서 uniformity 영상을 구함
	{
		for(i=0; i<height; i++)
		{
			for(j=0; j<width; j++)
			{
//				if(InImg[i*width+j] < MINIMUM_CIS_NOISE)		// 개선 필요
//					continue;

				val = (int)(InImg[i*width+j]*diff_image_withMinus[(i+offset_y)*mask_width+j+offset_x]) + InImg[i*width+j];
			
				if(val > 1023)
					val = 1023;

				OutImg[i*width+j] = val;
			}
		}
	}
	else
	{
		for(i=0; i<height; i++)
		{
			for(j=0; j<width; j++)
			{
//				if(InImg[i*width+j] < MINIMUM_CIS_NOISE)		// 개선 필요
//					continue;

				val = (int)(InImg[i*width+j]*diff_image_withMinusMean[(i+offset_y)*mask_width+j+offset_x]) + InImg[i*width+j];
			
				if(val > 1023)
					val = 1023;

				OutImg[i*width+j] = val;
			}
		}
	}

	// 가장 밝은 patch 위치 찾기
	max_mean=0;
	max_mean_xpos=0;
	max_mean_ypos=0;

	_SearchingMaxGrayboxFast_int(OutImg, &max_mean_xpos, &max_mean_ypos, &max_mean, histogram, mask_size);

	if(max_mean == 255)
	{
		for(i=0; i<1024; i++)
		{
			m_Stretch_LUT[i] = i;
		}
		return;
	}

	// find mode value
	int max_value=histogram[0];
	int mode_value=0;
	for(i=0; i<1024; i++)
	{
		if(max_value < histogram[i])
		{
			max_value = histogram[i];
			mode_value = i;
		}
	}

	int new_cut_thres = abs(max_mean - mode_value);
	int histo_cnt=0;
	sum=0;
	for(i=new_cut_thres; i<1024; i++)
	{
		sum += (i*histogram[i]);
		histo_cnt+=histogram[i];
	}

	max_mean = sum / histo_cnt;

	int max_histogram=0, min_histogram=1023;
	for(i=0; i<1024; i++)
	{
		if(histogram[i] > 0)
		{
			min_histogram = i;
			break;
		}
	}

	for(i=1023; i>=0; i--)
	{
		if(histogram[i] > 0)
		{
			max_histogram = i;
			break;
		}
	}

	int bright_weight_thres = 1023;
	double bright_weight_ratio = atof(m_strBrightWeightRatio);
	sum=0;
	for(i=1023; i>=0; i--)
	{
		sum+=histogram[i];

		if(sum > (mask_size*mask_size)*bright_weight_ratio)
		{
			bright_weight_thres = i;
			break;
		}
	}

	int white_from_mean = (max_histogram - max_mean);//*2;
	int black_from_mean = (max_mean - min_histogram);//*2;
	double NOISE_AMOUNT = atof(m_strNoiseAmount);//30.0;		// default 20.0

	if(NOISE_AMOUNT > white_from_mean)
		NOISE_AMOUNT = white_from_mean;
	if(NOISE_AMOUNT > black_from_mean)
		NOISE_AMOUNT = black_from_mean;

#ifdef __G_DEBUG__
	TRACE("Pos (%d, %d), Mean : %d, Noise_Amount : %.1f(white : %d, black : %d)\n", 
				max_mean_xpos, max_mean_ypos, max_mean, NOISE_AMOUNT, white_from_mean, black_from_mean);
#endif

	for(i=0; i<1024; i++)
		m_Stretch_LUT[i] = -1;

	for(i=0; i<1024; i++)
	{
		val = i;

		if(val>max_mean)
		{
			if(val > bright_weight_thres)
				val = (int)(val - (val-max_mean) * NOISE_AMOUNT / white_from_mean);
			else
				val = (int)(val - (val-max_mean) * (NOISE_AMOUNT*val/bright_weight_thres) / white_from_mean);
		}
		else if(val<max_mean)
		{
			if(val > bright_weight_thres)
				val = (int)(val + (max_mean-val) * NOISE_AMOUNT / black_from_mean);
			else
				val = (int)(val + (max_mean-val) * (NOISE_AMOUNT*val/bright_weight_thres) / black_from_mean +0.5);
		}

		if(val>1023)
			val=1023;
		if(val<0)
			val=0;

		if(m_Stretch_LUT[val] > i || m_Stretch_LUT[val] == -1)
			m_Stretch_LUT[val] = i;

		LinearityLUT[i] = val;
	}

	int find_left = -1;
	int find_right = -1;
	int recom_pos = 0;
	for(i=0; i<1024; i++)
	{
		if( m_Stretch_LUT[i] == -1 )
		{
			find_left = -1;
			find_right = -1;
			count = 0;
			recom_pos = 0;
			for(j=i-1; j>=0; j--)
			{
				if(m_Stretch_LUT[j] != -1)
				{
					find_left = m_Stretch_LUT[j];
					recom_pos += find_left;
					count++;
					break;
				}
			}

			for(j=i+1; j<1024; j++)
			{
				if(m_Stretch_LUT[j] != -1)
				{
					find_right = m_Stretch_LUT[j];
					recom_pos += find_right;
					count++;
					break;
				}
			}

			if(count > 0)
			{
				m_Stretch_LUT[i] = (int)((double)recom_pos / count + 0.5);
			}
		}
	}

	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			OutImg[i*width+j] = LinearityLUT[OutImg[i*width+j]];
		}
	}
}

int CSBD_Alg::_GetBackNoiseValue(int x, int y)
{
	int xx, yy;
	
	yy = (y * ZOOM_H) / CIS_IMG_H + ENLARGESIZE_ZOOM_H;
	xx = (x * ZOOM_W) / CIS_IMG_W + ENLARGESIZE_ZOOM_W;

	return m_segment_arr_background_substract[yy*ZOOM_ENLAGE_W+xx];
}

int CSBD_Alg::_GetBackNoiseValue_forRoll(int x, int y)
{
	int xx, yy;
	
	yy = (y * ZOOM_H_ROLL) / CIS_IMG_H_ROLL + ENLARGESIZE_ZOOM_H_ROLL;
	xx = (x * ZOOM_W_ROLL) / CIS_IMG_W_ROLL + ENLARGESIZE_ZOOM_W_ROLL;

	return m_segment_arr_background_substract[yy*ZOOM_ENLAGE_W_ROLL+xx];
}

void CSBD_Alg::_VignettingEffect_select_80_mean(short *InImg, short *OutImg)
{
	int i, j, val;
	int count=0;
	int mask_size=51;
//	int half_mask_size=mask_size/2;
	int sum;
	int histogram[1024];
	int max_mean, max_mean_xpos, max_mean_ypos;
//	int LOW_CUT_THRES=0;
//	int HIGH_CUT_THRES=0;
//	int Process_Count=0;
	int MINIMUM_CIS_NOISE = m_CMOS_NOISE_AMOUNT;//atoi(m_strMinCutThres);
	int LinearityLUT[1024];

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

	int tmpCIS_IMG_W = CIS_IMG_W;
//	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
	int offset_x;
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT;
		dtRIGHT		= m_pcImgAnalysis->RIGHT;
		dtTOP		= m_pcImgAnalysis->TOP;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;

		offset_x = (CIS_IMG_W-CIS_IMG_W_ROLL)/2;
	}
	else
	{
		dtLEFT		= (m_pcImgAnalysis->LEFT-1)*2;
		dtRIGHT		= (m_pcImgAnalysis->RIGHT+1)*2;
		dtTOP		= (m_pcImgAnalysis->TOP-1)*2;
		dtBOTTOM	= (m_pcImgAnalysis->BOTTOM+1)*2;

		tmpCIS_IMG_W = CIS_IMG_W;
//		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

		if(dtLEFT < 0) dtLEFT = 0;
		if(dtTOP < 0) dtTOP = 0;
		if(dtRIGHT > CIS_IMG_W) dtRIGHT = CIS_IMG_W;
		if(dtBOTTOM > CIS_IMG_H) dtBOTTOM = CIS_IMG_H;

		offset_x = 0;
	}

//	int mean=0;

	int ridge_pixel_tres=100;
	int ridge_pixel_cnt=0;
	int ridge_pixel_255_tres=200;
	int ridge_pixel_255_cnt=0;
	int ridge_pixel_255_percent=0;
	int GoMean=MINUS_MASK_MEAN;

	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			if(InImg[i*tmpCIS_IMG_W+j]>ridge_pixel_tres)
			ridge_pixel_cnt++;
			if(InImg[i*tmpCIS_IMG_W+j]>=ridge_pixel_255_tres)
			ridge_pixel_255_cnt++;
		}
	}
	if(ridge_pixel_cnt>0)
		ridge_pixel_255_percent = ridge_pixel_255_cnt*100 / ridge_pixel_cnt;
	else
		GoMean=MINUS_MASK_MEAN;

	if(ridge_pixel_255_percent>_THRES_255_80_MEAN_ALGO_)
		GoMean=MINUS_MASK_ORIGINAL;

	m_Percent_of_255 = ridge_pixel_255_percent;
	
	m_GoMean = GoMean;

	memset(OutImg, 0, tmpCIS_IMG_SIZE*sizeof(short));

	if(m_MinusMaskType == MINUS_MASK_ORIGINAL)	// minus mask를 적용해서 uniformity 영상을 구함
	{
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			for(j=dtLEFT; j<dtRIGHT; j++)
			{
				if(InImg[i*tmpCIS_IMG_W+j] < MINIMUM_CIS_NOISE)		// 개선 필요
				continue;

				val = (int)(InImg[i*tmpCIS_IMG_W+j]*diff_image_withMinus[i*CIS_IMG_W+j+offset_x]) + InImg[i*tmpCIS_IMG_W+j];
		
				if(val > 1023)
					val = 1023;

				OutImg[i*tmpCIS_IMG_W+j] = val;
			}
		}
	}
	else if(GoMean == MINUS_MASK_ORIGINAL)	// minus mask를 적용해서 uniformity 영상을 구함
	{
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			for(j=dtLEFT; j<dtRIGHT; j++)
			{
				if(InImg[i*tmpCIS_IMG_W+j] < MINIMUM_CIS_NOISE)		// 개선 필요
				continue;

				val = (int)(InImg[i*tmpCIS_IMG_W+j]*diff_image_withMinus[i*CIS_IMG_W+j+offset_x]) + InImg[i*tmpCIS_IMG_W+j];
		
				if(val > 1023)
					val = 1023;

				OutImg[i*tmpCIS_IMG_W+j] = val;
			}
		}
	}
	else if(GoMean == MINUS_MASK_MEAN)	// minus mask를 적용해서 uniformity 영상을 구함
	{
		//Default
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			for(j=dtLEFT; j<dtRIGHT; j++)
			{
				if(InImg[i*tmpCIS_IMG_W+j] < MINIMUM_CIS_NOISE)		// 개선 필요
				continue;

				val = (int)(InImg[i*tmpCIS_IMG_W+j]*diff_image_withMinusMean[i*CIS_IMG_W+j+offset_x]) + InImg[i*tmpCIS_IMG_W+j];
		
				if(val > 1023)				
					val = 1023;

				OutImg[i*tmpCIS_IMG_W+j] = val;
			}
		}
	}

	// 가장 밝은 patch 위치 찾기
	max_mean=0;
	max_mean_xpos=0;
	max_mean_ypos=0;

	_SearchingMaxGrayboxFast_int(OutImg, &max_mean_xpos, &max_mean_ypos, &max_mean, histogram, mask_size);

	// find mode value
	int max_value=histogram[0];
	int mode_value=0;
	for(i=0; i<1024; i++)
	{
		if(max_value < histogram[i])
		{
			max_value = histogram[i];
			mode_value = i;
		}
	}

	int new_cut_thres = abs(max_mean - mode_value);
	int histo_cnt=0;
	sum=0;
	for(i=new_cut_thres; i<1024; i++)
	{
		sum += (i*histogram[i]);
		histo_cnt+=histogram[i];
	}

	max_mean = sum / histo_cnt;

	int max_histogram=0, min_histogram=1023;
	for(i=0; i<1024; i++)
	{
		if(histogram[i] > 0)
		{
			min_histogram = i;
			break;
		}
	}

	for(i=1023; i>=0; i--)
	{
		if(histogram[i] > 0)
		{
			max_histogram = i;
			break;
		}
	}

	int bright_weight_thres = 1023;
	double bright_weight_ratio = atof(m_strBrightWeightRatio);
	sum=0;
	for(i=1023; i>=0; i--)
	{
		sum+=histogram[i];

		if(sum > (mask_size*mask_size)*bright_weight_ratio/*0.1*/)
		{
			bright_weight_thres = i;
			break;
		}
	}

	int white_from_mean = (max_histogram - max_mean);
	int black_from_mean = (max_mean - min_histogram);
	double NOISE_AMOUNT = atof(m_strNoiseAmount);//30.0;		// default 20.0

	// modified
	int min_bright_thres = 20;
	int max_bright_thres = 60;
	int min_noise_amount = 10;
	int max_noise_amount = 40;

	if(m_SegmentBright > max_bright_thres)
	{
		NOISE_AMOUNT = max_noise_amount;
	}
	else if(m_SegmentBright < min_bright_thres)
	{
		NOISE_AMOUNT = min_noise_amount;
	}
	else
	{
		NOISE_AMOUNT = min_noise_amount + (max_noise_amount-min_noise_amount) * (m_SegmentBright - min_bright_thres) / (max_bright_thres-min_bright_thres);
	}

	if(NOISE_AMOUNT > white_from_mean)
		NOISE_AMOUNT = white_from_mean;
	if(NOISE_AMOUNT > black_from_mean)
		NOISE_AMOUNT = black_from_mean;

//	if(m_UseFixedNoiseAmount == 0)
//		NOISE_AMOUNT=20;			// 마지막 frame은 bitshift를 사용하지 않으므로 20으로 고정

//	CString str;
//	str.Format("Pos (%d, %d), Mean : %d, Noise_Amount : %.1f(white : %d, black : %d", 
//				max_mean_xpos, max_mean_ypos, max_mean, NOISE_AMOUNT, white_from_mean, black_from_mean);
//	SetWindowText(str);

	for(i=0; i<1024; i++)
		m_Stretch_LUT[i] = -1;

	for(i=0; i<1024; i++)
	{
		val = i;

		if(val>max_mean)
		{
			if(val > bright_weight_thres)
				val = (int)(val - (val-max_mean) * NOISE_AMOUNT / white_from_mean);
			else
				val = (int)(val - (val-max_mean) * (NOISE_AMOUNT*val/bright_weight_thres) / white_from_mean);
		}
		else if(val<max_mean)
		{
			if(val > bright_weight_thres)
				val = (int)(val + (max_mean-val) * NOISE_AMOUNT / black_from_mean);
			else
				val = (int)(val + (max_mean-val) * (NOISE_AMOUNT*val/bright_weight_thres) / black_from_mean+0.5);
		}

		if(val>1023)
			val=1023;
		if(val<0)
			val=0;

		if(m_Stretch_LUT[val] > i || m_Stretch_LUT[val] == -1)
			m_Stretch_LUT[val] = i;

		LinearityLUT[i] = val;
	}

	int find_left = -1;
	int find_right = -1;
	int recom_pos = 0;
	for(i=0; i<1024; i++)
	{
		if( m_Stretch_LUT[i] == -1 )
		{
			find_left = -1;
			find_right = -1;
			count = 0;
			recom_pos = 0;
			for(j=i-1; j>=0; j--)
			{
				if(m_Stretch_LUT[j] != -1)
				{
					find_left = m_Stretch_LUT[j];
					recom_pos += find_left;
					count++;
					break;
				}
			}

			for(j=i+1; j<1024; j++)
			{
				if(m_Stretch_LUT[j] != -1)
				{
					find_right = m_Stretch_LUT[j];
					recom_pos += find_right;
					count++;
					break;
				}
			}

			if(count > 0)
			{
				m_Stretch_LUT[i] = (int)((double)recom_pos / count + 0.5);
			}
		}
	}

	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			OutImg[i*tmpCIS_IMG_W+j] = LinearityLUT[OutImg[i*tmpCIS_IMG_W+j]];
		}
	}
}

void CSBD_Alg::_VignettingEffect_select_80_mean_opt(unsigned char *InImg, short *OutImg)
{
	int i, j, val;
	int count=0;
	int mask_size=51;
//	int half_mask_size=mask_size/2;
	int sum;
	int histogram[1024];
	int max_mean, max_mean_xpos, max_mean_ypos;
//	int LOW_CUT_THRES=0;
//	int HIGH_CUT_THRES=0;
//	int Process_Count=0;
//	int MINIMUM_CIS_NOISE = m_CMOS_NOISE_AMOUNT;//atoi(m_strMinCutThres);
	int LinearityLUT[1024];

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

	int tmpCIS_IMG_W = CIS_IMG_W;
//	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
	int offset_x, offset_y;
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT;
		dtRIGHT		= m_pcImgAnalysis->RIGHT;
		dtTOP		= m_pcImgAnalysis->TOP;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;

		offset_x = (CIS_IMG_W-CIS_IMG_W_ROLL)/2;
		offset_y = m_pPropertyInfo->nStartingPositionOfRollArea*100;
	}
	else
	{
		dtLEFT		= (m_pcImgAnalysis->LEFT-1)*2;
		dtRIGHT		= (m_pcImgAnalysis->RIGHT+1)*2;
		dtTOP		= (m_pcImgAnalysis->TOP-1)*2;
		dtBOTTOM	= (m_pcImgAnalysis->BOTTOM+1)*2;

		tmpCIS_IMG_W = CIS_IMG_W;
//		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

		offset_x = 0;
		offset_y = 0;
	}
	
	memset(OutImg, 0, tmpCIS_IMG_SIZE*sizeof(short));


//	int mean=0;

	int ridge_pixel_tres=100;
	int ridge_pixel_cnt=0;
	int ridge_pixel_255_tres=200;
	int ridge_pixel_255_cnt=0;
	int ridge_pixel_255_percent=0;
	int GoMean=MINUS_MASK_MEAN;

	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			if(InImg[i*tmpCIS_IMG_W+j]>ridge_pixel_tres)
			ridge_pixel_cnt++;
			if(InImg[i*tmpCIS_IMG_W+j]>=ridge_pixel_255_tres)
			ridge_pixel_255_cnt++;
		}
	}
	if(ridge_pixel_cnt>0)
		ridge_pixel_255_percent = ridge_pixel_255_cnt*100 / ridge_pixel_cnt;
	else
		GoMean=MINUS_MASK_MEAN;

	if(ridge_pixel_255_percent>_THRES_255_80_MEAN_ALGO_)
		GoMean=MINUS_MASK_ORIGINAL;

#ifdef __G_DEBUG__
	TRACE("ridge_pixel_255_percent : %d\n", ridge_pixel_255_percent);
#endif

	m_Percent_of_255 = ridge_pixel_255_percent;
	
	m_GoMean = GoMean;

	if(m_MinusMaskType == MINUS_MASK_ORIGINAL)	// minus mask를 적용해서 uniformity 영상을 구함
	{
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			for(j=dtLEFT; j<dtRIGHT; j++)
			{
				if(InImg[i*tmpCIS_IMG_W+j] == 0)		// 개선 필요
					continue;

				val = (int)(InImg[i*tmpCIS_IMG_W+j]*diff_image_withMinus[(i+offset_y)*CIS_IMG_W+j+offset_x]) + InImg[i*tmpCIS_IMG_W+j];
		
				if(val > 1023)
					val = 1023;

				OutImg[i*tmpCIS_IMG_W+j] = val;
			}
		}
	}
	else if(GoMean == MINUS_MASK_ORIGINAL)	// minus mask를 적용해서 uniformity 영상을 구함
	{
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			for(j=dtLEFT; j<dtRIGHT; j++)
			{
				if(InImg[i*tmpCIS_IMG_W+j] == 0)		// 개선 필요
					continue;

				val = (int)(InImg[i*tmpCIS_IMG_W+j]*diff_image_withMinus[(i+offset_y)*CIS_IMG_W+j+offset_x]) + InImg[i*tmpCIS_IMG_W+j];
		
				if(val > 1023)
					val = 1023;

				OutImg[i*tmpCIS_IMG_W+j] = val;
			}
		}
	}
	else if(GoMean == MINUS_MASK_MEAN)	// minus mask를 적용해서 uniformity 영상을 구함
	{
		//Default
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			for(j=dtLEFT; j<dtRIGHT; j++)
			{
				if(InImg[i*tmpCIS_IMG_W+j] == 0)		// 개선 필요
					continue;

				val = (int)(InImg[i*tmpCIS_IMG_W+j]*diff_image_withMinusMean[(i+offset_y)*CIS_IMG_W+j+offset_x]) + InImg[i*tmpCIS_IMG_W+j];
		
				if(val > 1023)
					val = 1023;

				OutImg[i*tmpCIS_IMG_W+j] = val;
			}
		}
	}

	// 가장 밝은 patch 위치 찾기
	max_mean=0;
	max_mean_xpos=0;
	max_mean_ypos=0;

	_SearchingMaxGrayboxFast_int(OutImg, &max_mean_xpos, &max_mean_ypos, &max_mean, histogram, mask_size);

	// find mode value
	int max_value=histogram[0];
	int mode_value=0;
	for(i=0; i<1024; i++)
	{
		if(max_value < histogram[i])
		{
			max_value = histogram[i];
			mode_value = i;
		}
	}

	int new_cut_thres = abs(max_mean - mode_value);
	int histo_cnt=0;
	sum=0;
	for(i=new_cut_thres; i<1024; i++)
	{
		sum += (i*histogram[i]);
		histo_cnt+=histogram[i];
	}

	max_mean = sum / histo_cnt;

	int max_histogram=0, min_histogram=1023;
	for(i=0; i<1024; i++)
	{
		if(histogram[i] > 0)
		{
			min_histogram = i;
			break;
		}
	}

	for(i=1023; i>=0; i--)
	{
		if(histogram[i] > 0)
		{
			max_histogram = i;
			break;
		}
	}

	int bright_weight_thres = 1023;
	double bright_weight_ratio = atof(m_strBrightWeightRatio);
	sum=0;
	for(i=1023; i>=0; i--)
	{
		sum+=histogram[i];

		if(sum > (mask_size*mask_size)*bright_weight_ratio)
		{
			bright_weight_thres = i;
			break;
		}
	}

	int white_from_mean = (max_histogram - max_mean);
	int black_from_mean = (max_mean - min_histogram);
	double NOISE_AMOUNT = atof(m_strNoiseAmount);//30.0;		// default 20.0

	// modified
	int min_bright_thres = 20;
	int max_bright_thres = 60;
	int min_noise_amount = 10;
	int max_noise_amount = 40;

	if(m_SegmentBright > max_bright_thres)
	{
		NOISE_AMOUNT = max_noise_amount;
	}
	else if(m_SegmentBright < min_bright_thres)
	{
		NOISE_AMOUNT = min_noise_amount;
	}
	else
	{
		NOISE_AMOUNT = min_noise_amount + (max_noise_amount-min_noise_amount) * (m_SegmentBright - min_bright_thres) / (max_bright_thres-min_bright_thres);
	}

	NOISE_AMOUNT = 20;

	if(NOISE_AMOUNT > white_from_mean)
		NOISE_AMOUNT = white_from_mean;
	if(NOISE_AMOUNT > black_from_mean)
		NOISE_AMOUNT = black_from_mean;

//	if(m_UseFixedNoiseAmount == 0)
//		NOISE_AMOUNT=20;			// 마지막 frame은 bitshift를 사용하지 않으므로 20으로 고정

//	CString str;
//	str.Format("Pos (%d, %d), Mean : %d, Noise_Amount : %.1f(white : %d, black : %d", 
//				max_mean_xpos, max_mean_ypos, max_mean, NOISE_AMOUNT, white_from_mean, black_from_mean);
//	SetWindowText(str);

	for(i=0; i<1024; i++)
		m_Stretch_LUT[i] = -1;

	for(i=0; i<1024; i++)
	{
		val = i;

		if(val>max_mean)
		{
			if(val > bright_weight_thres)
				val = (int)(val - (val-max_mean) * NOISE_AMOUNT / white_from_mean);
			else
				val = (int)(val - (val-max_mean) * (NOISE_AMOUNT*val/bright_weight_thres) / white_from_mean);
		}
		else if(val<max_mean)
		{
			if(val > bright_weight_thres)
				val = (int)(val + (max_mean-val) * NOISE_AMOUNT / black_from_mean);
			else
				val = (int)(val + (max_mean-val) * (NOISE_AMOUNT*val/bright_weight_thres) / black_from_mean+0.5);
		}

		if(val>1023)
			val=1023;
		if(val<0)
			val=0;

		if(m_Stretch_LUT[val] > i || m_Stretch_LUT[val] == -1)
			m_Stretch_LUT[val] = i;

		LinearityLUT[i] = val;
	}

	int find_left = -1;
	int find_right = -1;
	int recom_pos = 0;
	for(i=0; i<1024; i++)
	{
		if( m_Stretch_LUT[i] == -1 )
		{
			find_left = -1;
			find_right = -1;
			count = 0;
			recom_pos = 0;
			for(j=i-1; j>=0; j--)
			{
				if(m_Stretch_LUT[j] != -1)
				{
					find_left = m_Stretch_LUT[j];
					recom_pos += find_left;
					count++;
					break;
				}
			}

			for(j=i+1; j<1024; j++)
			{
				if(m_Stretch_LUT[j] != -1)
				{
					find_right = m_Stretch_LUT[j];
					recom_pos += find_right;
					count++;
					break;
				}
			}

			if(count > 0)
			{
				m_Stretch_LUT[i] = (int)((double)recom_pos / count + 0.5);
			}
		}
	}

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			for(j=dtLEFT; j<dtRIGHT; j++)
			{
				val = LinearityLUT[OutImg[i*tmpCIS_IMG_W+j]];
				if(val>255)
					val=255;

				val = val - _GetBackNoiseValue_forRoll(j, i);
				if(val < 0)
					val = 0;
				OutImg[i*tmpCIS_IMG_W+j] = val;
			}
		}
	}
	else
	{
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			for(j=dtLEFT; j<dtRIGHT; j++)
			{
				val = LinearityLUT[OutImg[i*tmpCIS_IMG_W+j]];
				if(val>255)
					val=255;

				val = val - _GetBackNoiseValue(j, i);
				if(val < 0)
					val = 0;
				OutImg[i*tmpCIS_IMG_W+j] = val;
			}
		}
	}
}

void CSBD_Alg::_VignettingEffect_preview(unsigned char *InImg, unsigned char *OutImg, int cis_noise)
{
	int i, j, val;
	int MINIMUM_CIS_NOISE = cis_noise;//atoi(m_strMinCutThres)+20;
	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;
	
    dtLEFT		= m_pcImgAnalysis->LEFT;
    dtRIGHT		= m_pcImgAnalysis->RIGHT;
    dtTOP		= m_pcImgAnalysis->TOP;
    dtBOTTOM	= m_pcImgAnalysis->BOTTOM;
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		memset(OutImg, 0, CIS_IMG_SIZE_ROLL);

		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			for(j=dtLEFT; j<dtRIGHT; j++)
			{
				if(InImg[i*CIS_IMG_W_ROLL+j] < MINIMUM_CIS_NOISE)		// 개선 필요
					continue;

				val = InImg[i*CIS_IMG_W_ROLL+j] - MINIMUM_CIS_NOISE;//( (InImg[i*(CIS_IMG_W/2)+j] * diff_image_withMinusMean_preview[i*(CIS_IMG_W/2)+j]) >> 10 ) + InImg[i*(CIS_IMG_W/2)+j] - MINIMUM_CIS_NOISE;
			
				if(val < 0)
					val = 0;
//				else if(val > 255)
//					val = 255;

				OutImg[i*CIS_IMG_W_ROLL+j] = val;
			}
		}
	}
	else
	{
		if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
		{
			dtLEFT		= m_pcImgAnalysis->LEFT * 2;
			dtRIGHT		= m_pcImgAnalysis->RIGHT * 2;
			dtTOP		= m_pcImgAnalysis->TOP * 2;
			dtBOTTOM	= m_pcImgAnalysis->BOTTOM * 2;

			memset(OutImg, 0, CIS_IMG_SIZE);

			for(i=dtTOP; i<dtBOTTOM; i++)
			{
				for(j=dtLEFT; j<dtRIGHT; j++)
				{
					if(InImg[i*CIS_IMG_W+j] < MINIMUM_CIS_NOISE)		// 개선 필요
						continue;

					val = (int)(InImg[i*CIS_IMG_W+j] * diff_image_withMinusMean[i*CIS_IMG_W+j]) + InImg[i*CIS_IMG_W+j] - MINIMUM_CIS_NOISE;
				
					if(val < 0)
						val = 0;
					else if(val > 255)
						val = 255;

					OutImg[i*CIS_IMG_W+j] = val;
				}
			}
		}
		else
		{
			memset(OutImg, 0, CIS_IMG_SIZE/4);

			for(i=dtTOP; i<dtBOTTOM; i++)
			{
				for(j=dtLEFT; j<dtRIGHT; j++)
				{
					if(InImg[i*(CIS_IMG_W/2)+j] < MINIMUM_CIS_NOISE)		// 개선 필요
						continue;

					val = (int)(InImg[i*(CIS_IMG_W/2)+j] * diff_image_withMinusMean_preview[i*(CIS_IMG_W/2)+j] / 1024) + InImg[i*(CIS_IMG_W/2)+j] - MINIMUM_CIS_NOISE;
				
					if(val < 0)
						val = 0;
					else if(val > 255)
						val = 255;

					OutImg[i*(CIS_IMG_W/2)+j] = val;
				}
			}
		}
	}
}

void CSBD_Alg::_VignettingEffect_preview_roll(unsigned char *InImg, unsigned char *OutImg, int cis_noise)
{
	int i, j, val;
	int MINIMUM_CIS_NOISE = cis_noise;//atoi(m_strMinCutThres)+20;
	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;
	int move_x_org = (CIS_IMG_W-CIS_IMG_W_ROLL)/2;
	int move_y_org = 0;

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
		move_y_org = m_pPropertyInfo->nStartingPositionOfRollArea * 100;
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
		move_y_org = m_pPropertyInfo->nStartingPositionOfRollArea * 27;
	
    dtLEFT		= m_pcImgAnalysis->LEFT;
    dtRIGHT		= m_pcImgAnalysis->RIGHT;
    dtTOP		= m_pcImgAnalysis->TOP;
    dtBOTTOM	= m_pcImgAnalysis->BOTTOM;
	
	memset(OutImg, 0, CIS_IMG_SIZE_ROLL);

	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			if(InImg[i*CIS_IMG_W_ROLL+j] < MINIMUM_CIS_NOISE)		// 개선 필요
				continue;

			val = ( (int)(InImg[i*CIS_IMG_W_ROLL+j] * diff_image_withMinusMean[(i+move_y_org)*CIS_IMG_W+j+move_x_org])) + InImg[i*CIS_IMG_W_ROLL+j] - MINIMUM_CIS_NOISE;
		
			if(val < 0)
				val = 0;
			else if(val > 255)
				val = 255;

			OutImg[i*CIS_IMG_W_ROLL+j] = val;
		}
	}
}

void CSBD_Alg::_HistogramStretchUsingLUT_int(short *InImg, short *OutImg)
{
	int i, j;
	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

	int tmpCIS_IMG_W = CIS_IMG_W;
//	int tmpCIS_IMG_H = CIS_IMG_H;
//	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT;
		dtRIGHT		= m_pcImgAnalysis->RIGHT;
		dtTOP		= m_pcImgAnalysis->TOP;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
//		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
		dtLEFT		= (m_pcImgAnalysis->LEFT-1)*2;
		dtRIGHT		= (m_pcImgAnalysis->RIGHT+1)*2;
		dtTOP		= (m_pcImgAnalysis->TOP-1)*2;
		dtBOTTOM	= (m_pcImgAnalysis->BOTTOM+1)*2;

		tmpCIS_IMG_W = CIS_IMG_W;
//		tmpCIS_IMG_H = CIS_IMG_H;
//		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
	}

	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			OutImg[i*tmpCIS_IMG_W+j] = m_Stretch_LUT[InImg[i*tmpCIS_IMG_W+j]];
		}
	}
}

void CSBD_Alg::_HistogramStretchUsingLUT_int_opt(short *InImg, short *OutImg)
{
	int i, j, val;
	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

	int tmpCIS_IMG_W = CIS_IMG_W;
//	int tmpCIS_IMG_H = CIS_IMG_H;
//	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT;
		dtRIGHT		= m_pcImgAnalysis->RIGHT;
		dtTOP		= m_pcImgAnalysis->TOP;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
//		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
		dtLEFT		= (m_pcImgAnalysis->LEFT-1)*2;
		dtRIGHT		= (m_pcImgAnalysis->RIGHT+1)*2;
		dtTOP		= (m_pcImgAnalysis->TOP-1)*2;
		dtBOTTOM	= (m_pcImgAnalysis->BOTTOM+1)*2;

		tmpCIS_IMG_W = CIS_IMG_W;
//		tmpCIS_IMG_H = CIS_IMG_H;
//		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
	}

	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			val = m_Stretch_LUT[InImg[i*tmpCIS_IMG_W+j]] + 150;
			if(val > 1023)
				val = 1023;
			OutImg[i*tmpCIS_IMG_W+j] = val;
		}
	}
}

void CSBD_Alg::_RemoveSpecificNoise_int(short *InImg, short *OutImg)
{
	int i, j;
	int max_bright, /*max_bright_cnt, */pixel_bright;
	int histogram[1024], total_pixel_cnt;
	double NOISE_THRES = 0.1;//atof(m_NoiseThres);
	double noise_ratio;
	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;
	
	int tmpCIS_IMG_W = CIS_IMG_W;
	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT;
		dtRIGHT		= m_pcImgAnalysis->RIGHT;
		dtTOP		= m_pcImgAnalysis->TOP;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
		dtLEFT		= (m_pcImgAnalysis->LEFT-1)*2;
		dtRIGHT		= (m_pcImgAnalysis->RIGHT+1)*2;
		dtTOP		= (m_pcImgAnalysis->TOP-1)*2;
		dtBOTTOM	= (m_pcImgAnalysis->BOTTOM+1)*2;

		tmpCIS_IMG_W = CIS_IMG_W;
		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

		if(dtLEFT < 0) dtLEFT = 0;
		if(dtTOP < 0) dtTOP = 0;
		if(dtRIGHT > CIS_IMG_W) dtRIGHT = CIS_IMG_W;
		if(dtBOTTOM > CIS_IMG_H) dtBOTTOM = CIS_IMG_H;
	}

	memcpy(OutImg, InImg, tmpCIS_IMG_SIZE*sizeof(short));
	
 	if(dtLEFT < 1)	dtLEFT = 1;
	if(dtTOP < 1)	dtTOP = 1;
	if(dtRIGHT > tmpCIS_IMG_W-1)	dtRIGHT = tmpCIS_IMG_W-1;
	if(dtBOTTOM > tmpCIS_IMG_H-1)	dtBOTTOM = tmpCIS_IMG_H-1;
		
	memset(histogram, 0, sizeof(histogram));
	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			histogram[InImg[i*tmpCIS_IMG_W+j]]++;
		}
	}

	total_pixel_cnt = 0;
	for(i=16; i<1024; i++)
		total_pixel_cnt += histogram[i];

	max_bright= 0;
	for(i=1023; i>=0; i--)
	{
		if(histogram[i] > 0)
		{
			max_bright = i;
			break;
		}
	}

	max_bright = (int)(max_bright - max_bright*0.2);
	if(max_bright<0)
		max_bright=0;
			
//	max_bright_cnt = 0;
	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			pixel_bright = InImg[i*tmpCIS_IMG_W+j];
						
			if(pixel_bright >= max_bright)
			{
				noise_ratio = histogram[pixel_bright] * 100.0 / total_pixel_cnt;

				if(NOISE_THRES >= noise_ratio)
				{
					OutImg[i*tmpCIS_IMG_W+j] = ( InImg[(i-1)*tmpCIS_IMG_W+j-1] + 
											  InImg[(i-1)*tmpCIS_IMG_W+j] +
											  InImg[(i-1)*tmpCIS_IMG_W+j+1] +
											  InImg[i*tmpCIS_IMG_W+j-1] +
											  InImg[i*tmpCIS_IMG_W+j+1] +
											  InImg[(i+1)*tmpCIS_IMG_W+j-1] +
											  InImg[(i+1)*tmpCIS_IMG_W+j] +
											  InImg[(i+1)*tmpCIS_IMG_W+j+1] ) / 8;
				}
			}
		}
	}
}

void CSBD_Alg::_Unsharp_opt(unsigned char *InImg, unsigned char *OutImg)
{
	int i, j, xx, yy;
	int left, right, top, bottom;
	int val;
	double unsharp_param = atof(m_strUnsharpParam);
	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;
	int width, height;

	int tmpCIS_IMG_W = CIS_IMG_W;
	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT;
		dtRIGHT		= m_pcImgAnalysis->RIGHT;
		dtTOP		= m_pcImgAnalysis->TOP;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
		dtLEFT		= (m_pcImgAnalysis->LEFT-1)*2;
		dtRIGHT		= (m_pcImgAnalysis->RIGHT+1)*2;
		dtTOP		= (m_pcImgAnalysis->TOP-1)*2;
		dtBOTTOM	= (m_pcImgAnalysis->BOTTOM+1)*2;

		tmpCIS_IMG_W = CIS_IMG_W;
		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

		if(dtLEFT < 0) dtLEFT = 0;
		if(dtTOP < 0) dtTOP = 0;
		if(dtRIGHT > CIS_IMG_W) dtRIGHT = CIS_IMG_W;
		if(dtBOTTOM > CIS_IMG_H) dtBOTTOM = CIS_IMG_H;
	}

/*	if(dtLEFT < 5)	dtLEFT = 5;
	if(dtTOP < 5)	dtTOP = 5;
	if(dtRIGHT > tmpCIS_IMG_W-4)	dtRIGHT = tmpCIS_IMG_W-4;
	if(dtBOTTOM > tmpCIS_IMG_H-4)	dtBOTTOM = tmpCIS_IMG_H-4;
*/
	memcpy(OutImg, InImg, tmpCIS_IMG_SIZE);

	memset(m_SumGrayBuf, 0, tmpCIS_IMG_SIZE * 4);
	m_SumGrayBuf[dtTOP*tmpCIS_IMG_W+dtLEFT] = InImg[dtTOP*tmpCIS_IMG_W+dtLEFT];

	for(xx=dtLEFT+1; xx<dtRIGHT; xx++)
		m_SumGrayBuf[dtTOP*tmpCIS_IMG_W+xx] =
		m_SumGrayBuf[dtTOP*tmpCIS_IMG_W+xx-1] + InImg[dtTOP*tmpCIS_IMG_W+xx];

	for(yy=dtTOP+1; yy<dtBOTTOM; yy++)
		m_SumGrayBuf[yy*tmpCIS_IMG_W+dtLEFT] =
		m_SumGrayBuf[(yy-1)*tmpCIS_IMG_W+dtLEFT] + InImg[yy*tmpCIS_IMG_W+dtLEFT];

	for(yy=dtTOP+1; yy<dtBOTTOM; yy++)
	{
		for(xx=dtLEFT+1; xx<dtRIGHT; xx++)
		{
			m_SumGrayBuf[yy*tmpCIS_IMG_W+xx] = m_SumGrayBuf[yy*tmpCIS_IMG_W+(xx-1)]
												+ m_SumGrayBuf[(yy-1)*tmpCIS_IMG_W+xx]
												+ InImg[yy*tmpCIS_IMG_W+xx]
												- m_SumGrayBuf[(yy-1)*tmpCIS_IMG_W+(xx-1)];
		}
	}

	for (i=dtTOP; i<dtBOTTOM; i++)
	{
        top = i - (4+1);
        bottom = i + 4;

		if(top < 0) top = 0;
		if(bottom > tmpCIS_IMG_H-1) bottom = tmpCIS_IMG_H-1;

		height = (bottom-top);

		for (j=dtLEFT; j<dtRIGHT; j++)
		{
			// to make fast
			if (InImg[i * tmpCIS_IMG_W + j] == 0)
				continue;

			left = j - (4+1);
            right = j + 4;

			if(left < 0) left = 0;
			if(right > tmpCIS_IMG_W-1) right = tmpCIS_IMG_W-1;

			width = (right-left);

 			val = (int)(InImg[i * tmpCIS_IMG_W + j] - 
				(m_SumGrayBuf[bottom * tmpCIS_IMG_W + right] - m_SumGrayBuf[top * tmpCIS_IMG_W + right] - 
				m_SumGrayBuf[bottom * tmpCIS_IMG_W + left] + m_SumGrayBuf[top * tmpCIS_IMG_W + left]) / (width*height));

			if (val < 0)
			{
				val = (int)(val * (1.0 - InImg[i * tmpCIS_IMG_W + j] / (255.0)) * unsharp_param);
				val = (int)InImg[i * tmpCIS_IMG_W + j] + val;

				if (val < 0)
					val = 0;

				OutImg[i * tmpCIS_IMG_W + j] = (unsigned char)val;
			}
        }
    }
}

void CSBD_Alg::_StdDevSmoothing_int_opt(short *InImg, short *OutImg)
{
	int i, j;
	float mean_d_local, std_d_local, weight_bright;
    int left, right, top, bottom;
	int xx, yy;
//	int band = 3;
	
//	int mask_size=51;
//	int half_mask_size=mask_size/2;

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

	int tmpCIS_IMG_W = CIS_IMG_W;
	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT;
		dtRIGHT		= m_pcImgAnalysis->RIGHT;
		dtTOP		= m_pcImgAnalysis->TOP;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
		dtLEFT		= (m_pcImgAnalysis->LEFT-1)*2;
		dtRIGHT		= (m_pcImgAnalysis->RIGHT+1)*2;
		dtTOP		= (m_pcImgAnalysis->TOP-1)*2;
		dtBOTTOM	= (m_pcImgAnalysis->BOTTOM+1)*2;

		tmpCIS_IMG_W = CIS_IMG_W;
		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

		if(dtLEFT < 0) dtLEFT = 0;
		if(dtTOP < 0) dtTOP = 0;
		if(dtRIGHT > CIS_IMG_W) dtRIGHT = CIS_IMG_W;
		if(dtBOTTOM > CIS_IMG_H) dtBOTTOM = CIS_IMG_H;
	}

	int mean;
	double current_weight;

//	double max_intensity = 255;//*255;//*255;

	//memcpy(OutImg, InImg, CIS_IMG_SIZE*sizeof(int));
	memcpy(OutImg, InImg, tmpCIS_IMG_SIZE*sizeof(short));

	// calc bright weight
//	memset(weight_table_bright, 0, CIS_IMG_SIZE*sizeof(float));
//	for(i=0; i<CIS_IMG_SIZE; i++)
//		weight_table_bright[i] = 0;

/*	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
//			weight = InImg[i*CIS_IMG_W+j];
			weight_table_bright[i*tmpCIS_IMG_W+j] = InImg[i*tmpCIS_IMG_W+j] / 255.0f;//max_intensity;
			if(weight_table_bright[i*tmpCIS_IMG_W+j] > 1.0)
				weight_table_bright[i*tmpCIS_IMG_W+j] = 1.0;
		}
	}
*/
	// calc dev weight
	memset(weight_table_diff, 0, tmpCIS_IMG_SIZE*sizeof(float));
//	for(i=0; i<CIS_IMG_SIZE; i++)
//		weight_table_diff[i] = 0;

	// opt code
	// http://www.strchr.com/standard_deviation_in_one_pass
/*	for(i=band; i<CIS_IMG_H-band; i++)
	{
		for(j=band; j<CIS_IMG_W-band; j++)
		{
			mean_d_local = 0;
			std_d_local = 0;
	
			for(k=-2; k<=2; k++)
			{
				for(l=-2; l<=2; l++)
				{
					mean_d_local += InImg[(i+k)*CIS_IMG_W+(j+l)];
					std_d_local += InImg[(i+k)*CIS_IMG_W+(j+l)]*InImg[(i+k)*CIS_IMG_W+(j+l)];
				}
			}

			mean_d_local = mean_d_local / 25;
			std_d_local = std_d_local / 25 - mean_d_local * mean_d_local;
			std_d_local = sqrt(std_d_local);

			std_d_local = std_d_local - 15;
			if(std_d_local < 0)
				std_d_local = 0;
			if(std_d_local > 100)
				std_d_local = 100;

			weight_table_diff[i*CIS_IMG_W+j] = std_d_local / 100.0f;
		}
	}
*/
//	LEFT = 0;
//	RIGHT = CIS_IMG_W;
//	TOP  = 0;
//	BOTTOM = CIS_IMG_H;

	// 60 ~ 70ms
	memset(m_SumGrayBuf, 0, tmpCIS_IMG_SIZE * sizeof(int));
	memset(m_SumGrayBuf2, 0, tmpCIS_IMG_SIZE * sizeof(int));
	m_SumGrayBuf[dtTOP*tmpCIS_IMG_W+dtLEFT] = InImg[dtTOP*tmpCIS_IMG_W+dtLEFT];
	m_SumGrayBuf2[dtTOP*tmpCIS_IMG_W+dtLEFT] = InImg[dtTOP*tmpCIS_IMG_W+dtLEFT]*InImg[dtTOP*tmpCIS_IMG_W+dtLEFT];

	for(xx=dtLEFT+1; xx<dtRIGHT; xx++)
	{
		m_SumGrayBuf[dtTOP*tmpCIS_IMG_W+xx] =
		m_SumGrayBuf[dtTOP*tmpCIS_IMG_W+xx-1] + InImg[dtTOP*tmpCIS_IMG_W+xx];
		m_SumGrayBuf2[dtTOP*tmpCIS_IMG_W+xx] =
		m_SumGrayBuf2[dtTOP*tmpCIS_IMG_W+xx-1] + InImg[dtTOP*tmpCIS_IMG_W+xx]*InImg[dtTOP*tmpCIS_IMG_W+xx];
	}

	for(yy=dtTOP+1; yy<dtBOTTOM; yy++)
	{
		m_SumGrayBuf[yy*tmpCIS_IMG_W+dtLEFT] =
		m_SumGrayBuf[(yy-1)*tmpCIS_IMG_W+dtLEFT] + InImg[yy*tmpCIS_IMG_W+dtLEFT];
		m_SumGrayBuf2[yy*tmpCIS_IMG_W+dtLEFT] =
		m_SumGrayBuf2[(yy-1)*tmpCIS_IMG_W+dtLEFT] + InImg[yy*tmpCIS_IMG_W+dtLEFT]*InImg[yy*tmpCIS_IMG_W+dtLEFT];
	}

	for(yy=dtTOP+1; yy<dtBOTTOM; yy++)
	{
		for(xx=dtLEFT+1; xx<dtRIGHT; xx++)
		{
			m_SumGrayBuf[yy*tmpCIS_IMG_W+xx] = m_SumGrayBuf[yy*tmpCIS_IMG_W+(xx-1)]
												+ m_SumGrayBuf[(yy-1)*tmpCIS_IMG_W+xx]
												+ InImg[yy*tmpCIS_IMG_W+xx]
												- m_SumGrayBuf[(yy-1)*tmpCIS_IMG_W+(xx-1)];
			m_SumGrayBuf2[yy*tmpCIS_IMG_W+xx] = m_SumGrayBuf2[yy*tmpCIS_IMG_W+(xx-1)]
												+ m_SumGrayBuf2[(yy-1)*tmpCIS_IMG_W+xx]
												+ InImg[yy*tmpCIS_IMG_W+xx]*InImg[yy*tmpCIS_IMG_W+xx]
												- m_SumGrayBuf2[(yy-1)*tmpCIS_IMG_W+(xx-1)];
		}
	}

//	LEFT = band;//half_mask_size;
//	RIGHT = CIS_IMG_W-band;//half_mask_size-1;
//	TOP = band;//half_mask_size;
//	BOTTOM = CIS_IMG_H-band;//half_mask_size-1;

	if(dtLEFT < 3)	dtLEFT = 3;
	if(dtTOP < 3)	dtTOP = 3;
	if(dtRIGHT > tmpCIS_IMG_W-2)	dtRIGHT = tmpCIS_IMG_W-2;
	if(dtBOTTOM > tmpCIS_IMG_H-2)	dtBOTTOM = tmpCIS_IMG_H-2;

	// 280 ms
	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		top = i - (2+1);
        bottom = i + 2;

		for(j=dtLEFT; j<dtRIGHT; j++)
		{
            left = j - (2+1);
            right = j + 2;

            mean_d_local = (m_SumGrayBuf[bottom * tmpCIS_IMG_W + right] - m_SumGrayBuf[top * tmpCIS_IMG_W + right] - 
							m_SumGrayBuf[bottom * tmpCIS_IMG_W + left] + m_SumGrayBuf[top * tmpCIS_IMG_W + left]) / 25.0f;
            std_d_local = (m_SumGrayBuf2[bottom * tmpCIS_IMG_W + right] - m_SumGrayBuf2[top * tmpCIS_IMG_W + right] - 
						   m_SumGrayBuf2[bottom * tmpCIS_IMG_W + left] + m_SumGrayBuf2[top * tmpCIS_IMG_W + left]) / 25.0f 
						  - mean_d_local * mean_d_local;

			// to make fast
			if(std_d_local < 225)
			{
				weight_table_diff[i*tmpCIS_IMG_W+j] = 0;
				continue;
			}
			else if(std_d_local > 13225)
			{
				weight_table_diff[i*tmpCIS_IMG_W+j] = 1.0;
				continue;
			}

			// 200 ms
			std_d_local = sqrt(std_d_local) - 15;
			weight_table_diff[i*tmpCIS_IMG_W+j] = std_d_local / 100.0f;
		}
	}

	// merge weight
//	memset(weight_table_total, 0, sizeof(weight_table_total));
//	for(i=0; i<CIS_IMG_SIZE; i++)
//		weight_table_total[i] = 0;

//	memset(weight_table_total, 0, sizeof(weight_table_total));
	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			// 경계는 덜 뭉게도록 diff weight를 - 가중치를 준다.
//			weight_table_total[i*CIS_IMG_W+j] = weight_table_bright[i*CIS_IMG_W+j] - weight_table_bright[i*CIS_IMG_W+j] * weight_table_diff[i*CIS_IMG_W+j];
			if(InImg[i*tmpCIS_IMG_W+j] > 254)
				weight_bright = 1.0f;
			else
				weight_bright = InImg[i*tmpCIS_IMG_W+j] / 255.0f;//max_intensity;

			weight_table_diff[i*tmpCIS_IMG_W+j] = weight_bright * (1.0f - weight_table_diff[i*tmpCIS_IMG_W+j]);
		}
	}

//	max_weight = 1.0;
	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		top = i - (1+1);
        bottom = i + 1;

		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			// to make fast
//			if(InImg[i*CIS_IMG_W+j] == 0)
//				continue;

/*			mean = (int)((InImg[(i-1)*CIS_IMG_W+j-1] + 
							InImg[(i-1)*CIS_IMG_W+j] + 
							InImg[(i-1)*CIS_IMG_W+j+1] + 
							InImg[i*CIS_IMG_W+j-1] + 
//							InImg[i*CIS_IMG_W+j] + 
							InImg[i*CIS_IMG_W+j+1] + 
							InImg[(i+1)*CIS_IMG_W+j-1] + 
							InImg[(i+1)*CIS_IMG_W+j] + 
							InImg[(i+1)*CIS_IMG_W+j+1]) / 8.0 + 0.5);
*/

            left = j - (1+1);
            right = j + 1;		

			mean = (int)((m_SumGrayBuf[bottom * tmpCIS_IMG_W + right] -
							m_SumGrayBuf[top * tmpCIS_IMG_W + right] - 
							m_SumGrayBuf[bottom * tmpCIS_IMG_W + left] + 
							m_SumGrayBuf[top * tmpCIS_IMG_W + left] - 
							InImg[i*tmpCIS_IMG_W+j])/8.0 + 0.5);

//			current_weight = weight_table_total[i*CIS_IMG_W+j];
			current_weight = weight_table_diff[i*tmpCIS_IMG_W+j];
			OutImg[i*tmpCIS_IMG_W+j] = (int)((1.0-current_weight) * InImg[i*tmpCIS_IMG_W+j] + current_weight * mean);// / max_weight);
		}
	}
}

void CSBD_Alg::_RemoveSpotNoise(unsigned char *InImg, unsigned char *OutImg)
{
	int i, j;
	int count=0;
	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

	int tmpCIS_IMG_W = CIS_IMG_W;
	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT;
		dtRIGHT		= m_pcImgAnalysis->RIGHT;
		dtTOP		= m_pcImgAnalysis->TOP;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
		dtLEFT		= m_pcImgAnalysis->LEFT*2;
		dtRIGHT		= m_pcImgAnalysis->RIGHT*2;
		dtTOP		= m_pcImgAnalysis->TOP*2;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM*2;

		tmpCIS_IMG_W = CIS_IMG_W;
		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
	}

	if(dtLEFT < 1)	dtLEFT = 1;
	if(dtTOP < 1)	dtTOP = 1;
	if(dtRIGHT > tmpCIS_IMG_W-1)	dtRIGHT = tmpCIS_IMG_W-1;
	if(dtBOTTOM > tmpCIS_IMG_H-1)	dtBOTTOM = tmpCIS_IMG_H-1;

	memcpy(OutImg, InImg, tmpCIS_IMG_SIZE);

	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			if(InImg[i*tmpCIS_IMG_W+j] > 0)
			{
				count = 0;
				if(InImg[(i-1)*tmpCIS_IMG_W+j-1] == 0) count++;
				if(InImg[(i-1)*tmpCIS_IMG_W+j] == 0) count++;
				if(InImg[(i-1)*tmpCIS_IMG_W+j+1] == 0) count++;
				if(InImg[(i)*tmpCIS_IMG_W+j-1] == 0) count++;
				if(InImg[(i)*tmpCIS_IMG_W+j+1] == 0) count++;
				if(InImg[(i+1)*tmpCIS_IMG_W+j-1] == 0) count++;
				if(InImg[(i+1)*tmpCIS_IMG_W+j] == 0) count++;
				if(InImg[(i+1)*tmpCIS_IMG_W+j+1] == 0) count++;

				if(count >= 7)
					OutImg[i*tmpCIS_IMG_W+j] = 0;
			}
		}
	}
}

void CSBD_Alg::_HistogramStretchPercent(unsigned char *InImg, unsigned char *OutImg)
{
	int i;
	int /*lowthresh, */highthresh=0;
	double scale_factor;
	unsigned char Contrast_LUT[256];
	int histogram[256];
	int maxposx, maxposy, maxmean;
	int sum, val, max_value;
	int mask_size = 51;
	double Percent = 0.0;
	int CutHistoValue = 160;
	double StretchingPercent = atof(m_strStretchPercent);

	memcpy(OutImg, InImg, CIS_IMG_SIZE);

//	lowthresh = 0;

	_SearchingMaxGrayboxFast(InImg, &maxposx, &maxposy, &maxmean, histogram, mask_size);

	sum = 0;
	for(i=255; i>=0; i--)
	{
		sum+=histogram[i];
		if(sum > (mask_size*mask_size)*0.1)
		{
			highthresh = i;
			break;
		}
	}

	scale_factor = StretchingPercent / (255-CutHistoValue);

	if(highthresh <= CutHistoValue)
		Percent = StretchingPercent;
	else
		Percent = StretchingPercent - scale_factor * (highthresh-CutHistoValue);

	max_value = (int)(highthresh + highthresh * Percent / 100);
	
	scale_factor = (double)max_value / highthresh;
	
	for(i=0; i<256; i++)
	{
		val = (int)(i*scale_factor);
//		val=val-10;
//		if(val<0)
//			val=0;
		if(val > 255)
			val = 255;
		Contrast_LUT[i] = val;
	}

	for(i=0; i<CIS_IMG_SIZE; i++)
		OutImg[i] = Contrast_LUT[InImg[i]];
}

void CSBD_Alg::_HistogramStretchPercent_preview(unsigned char *InImg, unsigned char *OutImg, int width, int height)
{
	int i;
	int /*lowthresh, */highthresh=0;
	double scale_factor;
	unsigned char Contrast_LUT[256];
	int histogram[256];
	int maxposx, maxposy, maxmean;
	int sum, val, max_value;
	int mask_size = 51;
	double Percent = 0.0;
	int CutHistoValue = 160;
	double StretchingPercent = atof(m_strStretchPercent);
    int imageSize = width * height;

	memcpy(OutImg, InImg, imageSize);

//	lowthresh = 0;

	_SearchingMaxGrayboxFast_preview(InImg, width, height, &maxposx, &maxposy, &maxmean, histogram, mask_size);

	sum = 0;
	for(i=255; i>=0; i--)
	{
		sum+=histogram[i];
		if(sum > (mask_size*mask_size)*0.1)
		{
			highthresh = i;
			break;
		}
	}

	scale_factor = StretchingPercent / (255-CutHistoValue);

	if(highthresh <= CutHistoValue)
		Percent = StretchingPercent;
	else
		Percent = StretchingPercent - scale_factor * (highthresh-CutHistoValue);

	max_value = (int)(highthresh + highthresh * Percent / 100);
	
	scale_factor = (double)max_value / highthresh;
	
	for(i=0; i<256; i++)
	{
		val = (int)(i*scale_factor);
//		val=val-10;
//		if(val<0)
//			val=0;
		if(val > 255)
			val = 255;
		Contrast_LUT[i] = val;
	}

	for(i=0; i<imageSize; i++)
		OutImg[i] = Contrast_LUT[InImg[i]];
}

void CSBD_Alg::_HistogramStretchPercent_222222(unsigned char *InImg, unsigned char *OutImg)
{
	int i;
	int /*lowthresh, */highthresh=0;
	double scale_factor;
	unsigned char Contrast_LUT[256];
	int histogram[256];
	int histogram_whole[256];
	int maxposx, maxposy, maxmean;
	int sum, val;
	int mask_size = 101;					//// 곤아 여기
//	double Percent = 0.0;
//	int CutHistoValue = 160;
//	double StretchingPercent = atof(m_strStretchPercent);

//	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

	int tmpCIS_IMG_W = CIS_IMG_W;
	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
//		dtLEFT		= m_pcImgAnalysis->LEFT;
//		dtRIGHT		= m_pcImgAnalysis->RIGHT;
//		dtTOP		= m_pcImgAnalysis->TOP;
//		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
//		dtLEFT		= m_pcImgAnalysis->LEFT*2;
//		dtRIGHT		= m_pcImgAnalysis->RIGHT*2;
//		dtTOP		= m_pcImgAnalysis->TOP*2;
//		dtBOTTOM	= m_pcImgAnalysis->BOTTOM*2;

		tmpCIS_IMG_W = CIS_IMG_W;
		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
	}

	memcpy(OutImg, InImg, tmpCIS_IMG_SIZE);

//	lowthresh = 0;


//	int noise=20,count;
	int y,x;
	sum=0;
//	count=0;
	memset(histogram,0,sizeof(int)*256);
	memset(histogram_whole,0,sizeof(int)*256);
	

	int HIGH_B=235;
//	int LOW_B=230;
//	int HIGH_RIDGE=50;
//	int LOW_RIDGE=30;

//	if(hmax > HIGH_RIDGE)
//		MAX_BRIGHT=HIGH_B;
//	else if(hmax<LOW_RIDGE)
//		MAX_BRIGHT=LOW_B;
//	else
//	{
//		MAX_BRIGHT=LOW_B + (HIGH_B-LOW_B) * (hmax-LOW_RIDGE) / (HIGH_RIDGE-LOW_RIDGE);		
//	}

	
	_SearchingMaxGrayboxFast(InImg, &maxposx, &maxposy, &maxmean, histogram, mask_size);	//// 곤아 여기

	sum=0;
//	count=0;
	int max=0/*,max_sat=0*/;
	float band=0.25f;
	float max_band=0.012f;	//0.06
/*	float saturate=0.025;

	for(i=255; i>=0; i--)
	{
		sum+=histogram[i];

		if(sum > (mask_size*mask_size)*saturate)
		{
			max_sat=i;
			break;
		}
	}
*/
	sum=0;
	for(i=255; i>=0; i--)
	{
		sum+=histogram[i];

		if(sum > (mask_size*mask_size)*max_band)
		{
			max=i;
			break;
		}
	}

	for(y=100; y<tmpCIS_IMG_H-100; y++)
	{
		for(x=100; x<tmpCIS_IMG_W-100; x++)
		{
			val=InImg[y*tmpCIS_IMG_W+x];
			histogram_whole[val]+=1;
		}
	}

//	int hmax=0;
	int MAX_BRIGHT=255;
	sum=0;
	
	for(i=max+1; i<=255; i++)
	{
		if(histogram_whole[i]>10)
			sum++;		
	}
	if(sum > 0)
	{
		MAX_BRIGHT=HIGH_B;
	}

	

	sum=0;
	
	for(i=255; i>=0; i--)
	{
		sum+=histogram[i];		
		if(sum > (mask_size*mask_size)*band)
		{
			highthresh = i;
			break;
		}
	}
	if(highthresh>3)									//BUG?
		highthresh=highthresh-3;
	else
		return;

	int targetbright;	
	
	targetbright= MAX_BRIGHT-(max-highthresh);
	scale_factor = (float) targetbright / highthresh;

	if(scale_factor<1) return;

	for(i=max+1; i<=255; i++)
	{
		val=MAX_BRIGHT+i-max;;
		if(val>255)
			val=255;
		
		Contrast_LUT[i]=val;
	}

	for(i=highthresh; i<=max; i++)
	{
		Contrast_LUT[i]=targetbright+i-highthresh;
	}

	for(i=0; i<highthresh; i++)
	{
		Contrast_LUT[i]=(unsigned char)(scale_factor*i);
	}

	for(i=0; i<tmpCIS_IMG_SIZE; i++)
		OutImg[i] = Contrast_LUT[InImg[i]];
}

void CSBD_Alg::_HistogramStretchPercent_222222_debug(unsigned char *InImg, unsigned char *OutImg)
{
	int i,j;
	int /*lowthresh, */highthresh=0;
	double scale_factor;
	unsigned char Contrast_LUT[256];
	int histogram[256];
	int histogram_whole[256];
	int maxposx, maxposy, maxmean;
	int sum, val;
	int mask_size = 101;					//// 곤아 여기
//	double Percent = 0.0;
//	int CutHistoValue = 160;

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;
	
	int tmpCIS_IMG_W = CIS_IMG_W;
	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT;
		dtRIGHT		= m_pcImgAnalysis->RIGHT;
		dtTOP		= m_pcImgAnalysis->TOP;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
		dtLEFT		= (m_pcImgAnalysis->LEFT-1)*2;
		dtRIGHT		= (m_pcImgAnalysis->RIGHT+1)*2;
		dtTOP		= (m_pcImgAnalysis->TOP-1)*2;
		dtBOTTOM	= (m_pcImgAnalysis->BOTTOM+1)*2;

		tmpCIS_IMG_W = CIS_IMG_W;
		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

		if(dtLEFT < 0) dtLEFT = 0;
		if(dtTOP < 0) dtTOP = 0;
		if(dtRIGHT > CIS_IMG_W) dtRIGHT = CIS_IMG_W;
		if(dtBOTTOM > CIS_IMG_H) dtBOTTOM = CIS_IMG_H;
	}

//	lowthresh = 0;

//	int noise=20,count;
	int y,x;
	sum=0;
//	count=0;
	memset(histogram,0,sizeof(int)*256);
	memset(histogram_whole,0,sizeof(int)*256);
	
	memset(OutImg, 0, tmpCIS_IMG_SIZE);

	int HIGH_B=m_TARGET_STRETCH_HISTOGRAM;//235;
//	int LOW_B=230;
//	int HIGH_RIDGE=50;
//	int LOW_RIDGE=30;

	
	_SearchingMaxGrayboxFast(InImg, &maxposx, &maxposy, &maxmean, histogram, mask_size);	//// 곤아 여기

	sum=0;
//	count=0;
	int max=0/*,max_sat=0*/;
	float band=0.125;//0.25f;
	float max_band=0.006f;//0.012f;	//0.06

	sum=0;
	for(i=255; i>=0; i--)
	{
		sum+=histogram[i];

		if(sum > (mask_size*mask_size)*max_band)
		{
			max=i;
			break;
		}
	}

	for(y=100; y<tmpCIS_IMG_H-100; y++)
	{
		for(x=100; x<tmpCIS_IMG_W-100; x++)
		{
			val=InImg[y*CIS_IMG_W+x];
			histogram_whole[val]+=1;
		}
	}

//	int hmax=0;
	int MAX_BRIGHT=m_TARGET_STRETCH_HISTOGRAM;//235;		//Default 255
	sum=0;
	
	for(i=max+1; i<=255; i++)
	{
		if(histogram_whole[i]>10)
			sum++;		
	}
	if(sum > 0)
	{
		MAX_BRIGHT=HIGH_B;
	}

	

	sum=0;
	highthresh=0;
	for(i=255; i>=0; i--)
	{
		sum+=histogram[i];		
		if(sum > (mask_size*mask_size)*band)
		{
			highthresh = i;
			break;
		}
	}
	if(highthresh>3)									//BUG?
		highthresh=highthresh-3;
	else
	{
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			memcpy(&OutImg[i*tmpCIS_IMG_W+dtLEFT], &InImg[i*tmpCIS_IMG_W+dtLEFT], (dtRIGHT-dtLEFT+1));
		}
		return;
	}

	int targetbright;	
	
	targetbright= MAX_BRIGHT-(max-highthresh);
	scale_factor = (float) targetbright / highthresh;

	if(scale_factor<1)
	{
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			memcpy(&OutImg[i*tmpCIS_IMG_W+dtLEFT], &InImg[i*tmpCIS_IMG_W+dtLEFT], (dtRIGHT-dtLEFT+1));
		}
		return;
	}

	for(i=max+1; i<=255; i++)
	{
		val=MAX_BRIGHT+i-max;;
		if(val>255)
			val=255;
		
		Contrast_LUT[i]=val;
	}
	for(i=highthresh; i<=max; i++)
	{
		Contrast_LUT[i]=targetbright+i-highthresh;
	}

	for(i=0; i<highthresh; i++)
	{
		Contrast_LUT[i]=(unsigned char)(scale_factor*i);
	}

	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			OutImg[i*tmpCIS_IMG_W+j] = Contrast_LUT[InImg[i*tmpCIS_IMG_W+j]];
		}
	}
}

void CSBD_Alg::_HistogramStretchPercent_gon_final(unsigned char *InImg, unsigned char *OutImg)
{
	int i,j;
	int /*lowthresh, */highthresh=0;
	double scale_factor;
	unsigned char Contrast_LUT[256];
	int histogram[256];
	int histogram_whole[256];
	int maxposx, maxposy, maxmean;
	int sum, val;
	int mask_size = 101;					//// 곤아 여기

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;
	
	int tmpCIS_IMG_W = CIS_IMG_W;
	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT;
		dtRIGHT		= m_pcImgAnalysis->RIGHT;
		dtTOP		= m_pcImgAnalysis->TOP;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
		dtLEFT		= (m_pcImgAnalysis->LEFT-1)*2;
		dtRIGHT		= (m_pcImgAnalysis->RIGHT+1)*2;
		dtTOP		= (m_pcImgAnalysis->TOP-1)*2;
		dtBOTTOM	= (m_pcImgAnalysis->BOTTOM+1)*2;

		tmpCIS_IMG_W = CIS_IMG_W;
		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

		if(dtLEFT < 0) dtLEFT = 0;
		if(dtTOP < 0) dtTOP = 0;
		if(dtRIGHT > CIS_IMG_W) dtRIGHT = CIS_IMG_W;
		if(dtBOTTOM > CIS_IMG_H) dtBOTTOM = CIS_IMG_H;
	}

	int y,x;
	sum=0;
	memset(histogram,0,sizeof(int)*256);
	memset(histogram_whole,0,sizeof(int)*256);
	
	memset(OutImg, 0, tmpCIS_IMG_SIZE);

	int HIGH_B=m_TARGET_STRETCH_HISTOGRAM;
	
	_SearchingMaxGrayboxFast(InImg, &maxposx, &maxposy, &maxmean, histogram, mask_size);	//// 곤아 여기

	sum=0;
//	count=0;
	int max=0/*,max_sat=0*/;
	float band=0.125;//0.25f;
	float max_band=0.006f;//0.012f;	//0.06

	sum=0;
	for(i=255; i>=0; i--)
	{
		sum+=histogram[i];

		if(sum > (mask_size*mask_size)*max_band)
		{
			max=i;
			break;
		}
	}

	for(y=100; y<tmpCIS_IMG_H-100; y++)
	{
		for(x=100; x<tmpCIS_IMG_W-100; x++)
		{
			val=InImg[y*CIS_IMG_W+x];
			histogram_whole[val]+=1;
		}
	}

//	int hmax=0;
	int MAX_BRIGHT=m_TARGET_STRETCH_HISTOGRAM;//235;		//Default 255
	sum=0;
	
	for(i=max+1; i<=255; i++)
	{
		if(histogram_whole[i]>10)
			sum++;		
	}
	if(sum > 0)
	{
		MAX_BRIGHT=HIGH_B;
	}

	sum=0;
	highthresh=0;
	for(i=255; i>=0; i--)
	{
		sum+=histogram[i];		
		if(sum > (mask_size*mask_size)*band)
		{
			highthresh = i;
			break;
		}
	}
	if(highthresh>3)									//BUG?
		highthresh=highthresh-3;
	else
	{
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			memcpy(&OutImg[i*tmpCIS_IMG_W+dtLEFT], &InImg[i*tmpCIS_IMG_W+dtLEFT], (dtRIGHT-dtLEFT+1));
		}
		return;
	}

	int targetbright;	
	
	targetbright= MAX_BRIGHT-(max-highthresh);
	scale_factor = (float) targetbright / highthresh;

	if(scale_factor<1)
	{
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			memcpy(&OutImg[i*tmpCIS_IMG_W+dtLEFT], &InImg[i*tmpCIS_IMG_W+dtLEFT], (dtRIGHT-dtLEFT+1));
		}
		return;
	}

	for(i=max+1; i<=255; i++)
	{
		val=MAX_BRIGHT+i-max;
		if(val>255)
			val=255;
		
		Contrast_LUT[i]=val;
	}
	for(i=highthresh; i<=max; i++)
	{
		Contrast_LUT[i]=targetbright+i-highthresh;
	}

	for(i=0; i<highthresh; i++)
	{
		Contrast_LUT[i]=(unsigned char)(scale_factor*i);
	}

	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			OutImg[i*tmpCIS_IMG_W+j] = Contrast_LUT[InImg[i*tmpCIS_IMG_W+j]];
		}
	}
}

void CSBD_Alg::_HistogramStretchPercent_222222_preview(unsigned char *InImg, unsigned char *OutImg)
{
	int i, j, dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

    dtLEFT		= m_pcImgAnalysis->LEFT;
    dtRIGHT		= m_pcImgAnalysis->RIGHT;
    dtTOP		= m_pcImgAnalysis->TOP;
    dtBOTTOM	= m_pcImgAnalysis->BOTTOM;

	memset(OutImg, 0, CIS_IMG_SIZE/4);
	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			OutImg[i*(CIS_IMG_W/2)+j] = m_StretchPreviewLUT[InImg[i*(CIS_IMG_W/2)+j]];
		}
	}
}

void CSBD_Alg::_HistogramStretchPercent_222222_preview_roll(unsigned char *InImg, unsigned char *OutImg)
{
	int i, j, dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

    dtLEFT		= m_pcImgAnalysis->LEFT*2;
    dtRIGHT		= m_pcImgAnalysis->RIGHT*2;
    dtTOP		= m_pcImgAnalysis->TOP*2;
    dtBOTTOM	= m_pcImgAnalysis->BOTTOM*2;

	memset(OutImg, 0, CIS_IMG_SIZE);
	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			OutImg[i*CIS_IMG_W+j] = m_StretchPreviewLUT[InImg[i*CIS_IMG_W+j]];
		}
	}
}


void CSBD_Alg::_HistogramStretchPercent_333333(unsigned char *InImg, unsigned char *OutImg)
{
	int i;
	int /*lowthresh, */highthresh=0;
	double scale_factor;
	unsigned char Contrast_LUT[256];
	int histogram[256];
	int maxposx, maxposy, maxmean;
	int sum, val;
	int mask_size = 101;					//// 곤아 여기
//	double Percent = 0.0;
//	int CutHistoValue = 160;
//	double StretchingPercent = atof(m_strStretchPercent);

	memcpy(OutImg, InImg, CIS_IMG_SIZE);

//	lowthresh = 0;


//	int noise=20,count;
	int y,x;
	sum=0;
//	count=0;
	memset(histogram,0,sizeof(int)*256);
	for(y=100; y<CIS_IMG_H-100; y++)
	{
		for(x=100; x<CIS_IMG_W-100; x++)
		{
			val=InImg[y*CIS_IMG_W+x];
			histogram[val]+=1;
		}
	}

	int hmax=0;
	int MAX_BRIGHT=240;
	sum=0;
	
	for(i=255; i>=0; i--)
	{
		sum+=histogram[i];
		if(sum > 1000)
		{
			hmax=i;
			break;
		}
	}

	int HIGH_B=245;
	int LOW_B=230;
	int HIGH_RIDGE=50;
	int LOW_RIDGE=30;

	if(hmax > HIGH_RIDGE)
		MAX_BRIGHT=HIGH_B;
	else if(hmax<LOW_RIDGE)
		MAX_BRIGHT=LOW_B;
	else
	{
		MAX_BRIGHT=LOW_B + (HIGH_B-LOW_B) * (hmax-LOW_RIDGE) / (HIGH_RIDGE-LOW_RIDGE);		
	}

	_SearchingMaxGrayboxFast(InImg, &maxposx, &maxposy, &maxmean, histogram, mask_size);	//// 곤아 여기

	sum=0;
//	count=0;
	int max=0;
	float band=0.25;	

	for(i=255; i>=0; i--)
	{
		if(histogram[i]>0)
		{
			max=i;
			break;
		}
	}
	
	for(i=255; i>=0; i--)
	{
		sum+=histogram[i];		
		if(sum > (mask_size*mask_size)*band)
		{
			highthresh = i;
			break;
		}
	}
	
	if(max-highthresh < 15)
		highthresh = max;

	if(highthresh>3)									//BUG?
		highthresh=highthresh-3;
	else
		return;

	int targetbright;	
	
	targetbright= MAX_BRIGHT-(max-highthresh);
	scale_factor = (float) targetbright / highthresh;

	if(scale_factor<1) return;

	for(i=max+1; i<=255; i++)
	{
		val=MAX_BRIGHT+i-max;;
		if(val>255)
			val=255;
		
		Contrast_LUT[i]=val;
	}

	for(i=highthresh; i<=max; i++)
	{
		Contrast_LUT[i]=targetbright+i-highthresh;
	}

	for(i=0; i<highthresh; i++)
	{
		Contrast_LUT[i]=(unsigned char)(scale_factor*i);
	}

/*	sum = 0;
	int count=0;
	float band=0.3;					//// 곤아 여기
	for(i=255; i>=0; i--)
	{
		sum+=histogram[i];		
		if(sum > (mask_size*mask_size)*band)
		{
			highthresh = i;
			break;
		}
	}
	count=sum;

	sum=0;
	int low_thresh=0;
	for(i=highthresh-1; i>0; i--)
	{
		sum+=histogram[i];
		if(sum>count)
		{
			low_thresh=i;
			break;
		}		
	}


	if(low_thresh<10)						//// 곤아 여기
		low_thresh=10;

	int target_brightness=230;
	scale_factor = (float) target_brightness / low_thresh;
	
	for(i=highthresh; i<=255; i++)
	{
		val = target_brightness+i-highthresh;
		if(val>255)
			val=255;
		if(val<0)
			val=0;
		Contrast_LUT[i] = val;
	}

	for(i=highthresh-1; i>=low_thresh; i--)
	{
		val = target_brightness+i-highthresh;
		if(val>255)
			val=255;
		if(val<0)
			val=0;
		Contrast_LUT[i] = val;
	}
	

	for(i=0; i<low_thresh; i++)
	{
		val = i*scale_factor;
		if(val>target_brightness)
			val=target_brightness;
		Contrast_LUT[i] = val;
	}
*/
	for(i=0; i<CIS_IMG_SIZE; i++)
		OutImg[i] = Contrast_LUT[InImg[i]];
}

void CSBD_Alg::_SearchingMaxGrayboxFast(unsigned char *InImg, int *MaxPosX, int *MaxPosY, int *MaxMean, int *MaxHistogram, int m_size)
{
    int y, x; //,j,i;
    int sum = 0; //,count;
	int max_gray = 0;
    int left, right, top, bottom;
	int xx, yy;
	
	int mask_size=m_size;
	int half_mask_size=mask_size/2;
	int mask_size_square=mask_size*mask_size;

	int LEFT = 0;//half_mask_size;
	int RIGHT = CIS_IMG_W;//half_mask_size-1;
	int TOP = 0;//half_mask_size;
	int BOTTOM = CIS_IMG_H;//half_mask_size-1;
    
	int tmpCIS_IMG_W = CIS_IMG_W;
	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
		tmpCIS_IMG_W = CIS_IMG_W;
		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
	}

	LEFT = 0;//half_mask_size;
	RIGHT = tmpCIS_IMG_W;//half_mask_size-1;
	TOP = 0;//half_mask_size;
	BOTTOM = tmpCIS_IMG_H;//half_mask_size-1;

	memset(m_SumGrayBuf, 0, tmpCIS_IMG_SIZE * sizeof(int));
	m_SumGrayBuf[TOP*tmpCIS_IMG_W+LEFT] = InImg[TOP*tmpCIS_IMG_W+LEFT];

	for(xx=LEFT+1; xx<RIGHT; xx++)
		m_SumGrayBuf[TOP*tmpCIS_IMG_W+xx] =
		m_SumGrayBuf[TOP*tmpCIS_IMG_W+xx-1] + InImg[TOP*tmpCIS_IMG_W+xx];

	for(yy=TOP+1; yy<BOTTOM; yy++)
		m_SumGrayBuf[yy*tmpCIS_IMG_W+LEFT] =
		m_SumGrayBuf[(yy-1)*tmpCIS_IMG_W+LEFT] + InImg[yy*tmpCIS_IMG_W+LEFT];

	for(yy=TOP+1; yy<BOTTOM; yy++)
	{
		for(xx=LEFT+1; xx<RIGHT; xx++)
		{
			m_SumGrayBuf[yy*tmpCIS_IMG_W+xx] = m_SumGrayBuf[yy*tmpCIS_IMG_W+(xx-1)]
												+ m_SumGrayBuf[(yy-1)*tmpCIS_IMG_W+xx]
												+ InImg[yy*tmpCIS_IMG_W+xx]
												- m_SumGrayBuf[(yy-1)*tmpCIS_IMG_W+(xx-1)];
		}
	}

	LEFT = 100;//half_mask_size;
	RIGHT = tmpCIS_IMG_W-100;//half_mask_size-1;
	TOP = 100;//half_mask_size;
	BOTTOM = tmpCIS_IMG_H-100;//half_mask_size-1;

	for(y=TOP+(half_mask_size+1); y<BOTTOM-half_mask_size; y++)
    {
        top = y - (half_mask_size+1);
        bottom = y + half_mask_size;

        for (x=LEFT+(half_mask_size+1); x<RIGHT-half_mask_size; x++)
        {
            left = x - (half_mask_size+1);
            right = x + half_mask_size;

            sum = (m_SumGrayBuf[bottom * tmpCIS_IMG_W + right] - m_SumGrayBuf[top * tmpCIS_IMG_W + right] - 
					m_SumGrayBuf[bottom * tmpCIS_IMG_W + left] + m_SumGrayBuf[top * tmpCIS_IMG_W + left]) / (mask_size_square);

			if(sum>max_gray)
            {
                max_gray = sum;
				*MaxPosX = x;
				*MaxPosY = y;
            }
        }
    }

	*MaxMean = max_gray;// / (mask_size*mask_size);

	if((*MaxPosX)-half_mask_size < 0)
		*MaxPosX = half_mask_size;
	else if((*MaxPosX)+half_mask_size > tmpCIS_IMG_W-2)
		*MaxPosX = tmpCIS_IMG_W-2-half_mask_size;

	if((*MaxPosY)-half_mask_size < 0)
		*MaxPosY = half_mask_size;
	else if((*MaxPosY)+half_mask_size > tmpCIS_IMG_H-2)
		*MaxPosY = tmpCIS_IMG_H-2-half_mask_size;

	memset(MaxHistogram, 0, 1024);
	
	for(y=(*MaxPosY)-half_mask_size; y<=(*MaxPosY)+half_mask_size; y++)
	{
		for(x=(*MaxPosX)-half_mask_size; x<=(*MaxPosX)+half_mask_size; x++)
		{
			MaxHistogram[InImg[y*tmpCIS_IMG_W+x]]++;
		}
	}
}

void CSBD_Alg::_SearchingMaxGrayboxFast_preview(unsigned char *InImg, int width, int height, int *MaxPosX, int *MaxPosY, int *MaxMean, int *MaxHistogram, int m_size)
{
    int y, x; //,j,i;
    int sum = 0; //,count;
	int max_gray = 0;
    int left, right, top, bottom;
	int xx, yy;
	
	int mask_size=m_size;
	int half_mask_size=mask_size/2;
//	int mask_size_square=mask_size*mask_size;

	int LEFT = 0;//half_mask_size;
	int RIGHT = width;//half_mask_size-1;
	int TOP = 0;//half_mask_size;
	int BOTTOM = height;//half_mask_size-1;
    int imageSize = width * height;
    
	memset(m_SumGrayBuf, 0, imageSize * sizeof(int));
	m_SumGrayBuf[TOP*width+LEFT] = InImg[TOP*width+LEFT];

	for(xx=LEFT+1; xx<RIGHT; xx++)
		m_SumGrayBuf[TOP*width+xx] =
		m_SumGrayBuf[TOP*width+xx-1] + InImg[TOP*width+xx];

	for(yy=TOP+1; yy<BOTTOM; yy++)
		m_SumGrayBuf[yy*width+LEFT] =
		m_SumGrayBuf[(yy-1)*width+LEFT] + InImg[yy*width+LEFT];

	for(yy=TOP+1; yy<BOTTOM; yy++)
	{
		for(xx=LEFT+1; xx<RIGHT; xx++)
		{
			m_SumGrayBuf[yy*width+xx] = m_SumGrayBuf[yy*width+(xx-1)]
												+ m_SumGrayBuf[(yy-1)*width+xx]
												+ InImg[yy*width+xx]
												- m_SumGrayBuf[(yy-1)*width+(xx-1)];
		}
	}

	LEFT = 100/2;//half_mask_size;
	RIGHT = width-50;//(CIS_IMG_W-100)/2;//half_mask_size-1;
	TOP = 100/2;//half_mask_size;
	BOTTOM = height-50;//half_mask_size-1;

	for(y=TOP+(half_mask_size+1); y<BOTTOM-half_mask_size; y++)
    {
        top = y - (half_mask_size+1);
        bottom = y + half_mask_size;

        for (x=LEFT+(half_mask_size+1); x<RIGHT-half_mask_size; x++)
        {
            left = x - (half_mask_size+1);
            right = x + half_mask_size;

            sum = (m_SumGrayBuf[bottom * width + right] - m_SumGrayBuf[top * width + right] - 
					m_SumGrayBuf[bottom * width + left] + m_SumGrayBuf[top * width + left]) / (mask_size*mask_size);

			if(sum>max_gray)
            {
                max_gray = sum;
				*MaxPosX = x;
				*MaxPosY = y;
            }
        }
    }

	*MaxMean = max_gray;// / (mask_size*mask_size);

	if((*MaxPosX)-half_mask_size < 0)
		*MaxPosX = half_mask_size;
	else if((*MaxPosX)+half_mask_size > width-2)
		*MaxPosX = width-2-half_mask_size;

	if((*MaxPosY)-half_mask_size < 0)
		*MaxPosY = half_mask_size;
	else if((*MaxPosY)+half_mask_size > height-2)
		*MaxPosY = height-2-half_mask_size;

	memset(MaxHistogram, 0, 1024);
	
	for(y=(*MaxPosY)-half_mask_size; y<=(*MaxPosY)+half_mask_size; y++)
	{
		for(x=(*MaxPosX)-half_mask_size; x<=(*MaxPosX)+half_mask_size; x++)
		{
			MaxHistogram[InImg[y*width+x]]++;
		}
	}
}

void CSBD_Alg::_SearchingMaxGrayboxFast_int(short *InImg, int *MaxPosX, int *MaxPosY, int *MaxMean, int *MaxHistogram, int m_size)
{
    int y, x; //,j,i;
    int sum = 0; //,count;
	int max_gray = 0;
    int left, right, top, bottom;
	int xx, yy;
	
	int mask_size=m_size;
	int half_mask_size=mask_size/2;
	int mask_size_square=mask_size*mask_size;

	int LEFT = 0;//half_mask_size;
	int RIGHT = CIS_IMG_W;//half_mask_size-1;
	int TOP = 0;//half_mask_size;
	int BOTTOM = CIS_IMG_H;//half_mask_size-1;

	int tmpCIS_IMG_W = CIS_IMG_W;
	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else
	{
		tmpCIS_IMG_W = CIS_IMG_W;
		tmpCIS_IMG_H = CIS_IMG_H;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
	}

	LEFT = 0;
	RIGHT = tmpCIS_IMG_W;
	TOP = 0;
	BOTTOM = tmpCIS_IMG_H;
    
	memset(m_SumGrayBuf, 0, tmpCIS_IMG_SIZE * 4);
	m_SumGrayBuf[TOP*tmpCIS_IMG_W+LEFT] = InImg[TOP*tmpCIS_IMG_W+LEFT];

	for(xx=LEFT+1; xx<RIGHT; xx++)
		m_SumGrayBuf[TOP*tmpCIS_IMG_W+xx] =
		m_SumGrayBuf[TOP*tmpCIS_IMG_W+xx-1] + InImg[TOP*tmpCIS_IMG_W+xx];

	for(yy=TOP+1; yy<BOTTOM; yy++)
		m_SumGrayBuf[yy*tmpCIS_IMG_W+LEFT] =
		m_SumGrayBuf[(yy-1)*tmpCIS_IMG_W+LEFT] + InImg[yy*tmpCIS_IMG_W+LEFT];

	for(yy=TOP+1; yy<BOTTOM; yy++)
	{
		for(xx=LEFT+1; xx<RIGHT; xx++)
		{
			m_SumGrayBuf[yy*tmpCIS_IMG_W+xx] = m_SumGrayBuf[yy*tmpCIS_IMG_W+(xx-1)]
												+ m_SumGrayBuf[(yy-1)*tmpCIS_IMG_W+xx]
												+ InImg[yy*tmpCIS_IMG_W+xx]
												- m_SumGrayBuf[(yy-1)*tmpCIS_IMG_W+(xx-1)];
		}
	}

	LEFT = 100;//half_mask_size;
	RIGHT = tmpCIS_IMG_W-100;//half_mask_size-1;
	TOP = 100;//half_mask_size;
	BOTTOM = tmpCIS_IMG_H-100;//half_mask_size-1;

	for(y=TOP+(half_mask_size+1); y<BOTTOM-half_mask_size; y++)
    {
        top = y - (half_mask_size+1);
        bottom = y + half_mask_size;

        for (x=LEFT+(half_mask_size+1); x<RIGHT-half_mask_size; x++)
        {
            left = x - (half_mask_size+1);
            right = x + half_mask_size;

            sum = (m_SumGrayBuf[bottom * tmpCIS_IMG_W + right] - m_SumGrayBuf[top * tmpCIS_IMG_W + right] - 
					m_SumGrayBuf[bottom * tmpCIS_IMG_W + left] + m_SumGrayBuf[top * tmpCIS_IMG_W + left]) / (mask_size_square);

			if(sum>max_gray)
            {
                max_gray = sum;
				*MaxPosX = x;
				*MaxPosY = y;
            }
        }
    }

	*MaxMean = max_gray;// / (mask_size*mask_size);

	if((*MaxPosX)-half_mask_size < 0)
		*MaxPosX = half_mask_size;
	else if((*MaxPosX)+half_mask_size > tmpCIS_IMG_W-2)
		*MaxPosX = tmpCIS_IMG_W-2-half_mask_size;

	if((*MaxPosY)-half_mask_size < 0)
		*MaxPosY = half_mask_size;
	else if((*MaxPosY)+half_mask_size > tmpCIS_IMG_H-2)
		*MaxPosY = tmpCIS_IMG_H-2-half_mask_size;

	memset(MaxHistogram, 0, 1024*4);
	
	for(y=(*MaxPosY)-half_mask_size; y<=(*MaxPosY)+half_mask_size; y++)
	{
		for(x=(*MaxPosX)-half_mask_size; x<=(*MaxPosX)+half_mask_size; x++)
		{
			MaxHistogram[InImg[y*tmpCIS_IMG_W+x]]++;
		}
	}
}

void CSBD_Alg::_Make500DPI_opt(unsigned char *InImg, unsigned char *OutImg)
{
	int i, j, xx, yy;
	int band = 3;
	double weight, mean_d_local, std_d_local;
	int left, right, top, bottom;
	unsigned char *tmpResult = (unsigned char*)&m_OriginalImg_int[0];
	unsigned char *tmpResult2 = (unsigned char*)&m_OriginalImg_int[IMG_SIZE];

	_DistortionRestoration_with_Cubic(InImg, tmpResult);
	_DistortionRestoration_with_Bilinear(InImg, tmpResult2);
	
	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

	int tmpIMG_W = IMG_W;
	int tmpIMG_H = IMG_H;
	int tmpIMG_SIZE = IMG_SIZE;
	int tmpCIS_IMG_W = CIS_IMG_W;
	int tmpCIS_IMG_H = CIS_IMG_H;
//	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		tmpIMG_W = IMG_W_ROLL;
		tmpIMG_H = IMG_H_ROLL;
		tmpIMG_SIZE = IMG_SIZE_ROLL;
		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
//		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;

		dtLEFT		= m_pcImgAnalysis->LEFT * tmpIMG_W / tmpCIS_IMG_W - 2;
		dtRIGHT		= m_pcImgAnalysis->RIGHT * tmpIMG_W / tmpCIS_IMG_W + 2;
		dtTOP		= m_pcImgAnalysis->TOP * tmpIMG_H / tmpCIS_IMG_H - 2;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM * tmpIMG_H / tmpCIS_IMG_H + 2;
	}
	else
	{
		dtLEFT		= m_pcImgAnalysis->LEFT * tmpIMG_W * 2 / tmpCIS_IMG_W - 2;
	    dtRIGHT		= m_pcImgAnalysis->RIGHT * tmpIMG_W * 2 / tmpCIS_IMG_W + 2;
	    dtTOP		= m_pcImgAnalysis->TOP * tmpIMG_H * 2 / tmpCIS_IMG_H - 2;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM * tmpIMG_H * 2 / tmpCIS_IMG_H + 2;
	}

	if(dtLEFT < 0) dtLEFT = 0;
	if(dtRIGHT > tmpIMG_W) dtRIGHT = tmpIMG_W;
	if(dtTOP < 0) dtTOP = 0;
	if(dtBOTTOM < tmpIMG_H) dtBOTTOM = tmpIMG_H;

	memset(m_SumGrayBuf, 0, tmpIMG_SIZE * sizeof(int));
	memset(m_SumGrayBuf2, 0, tmpIMG_SIZE * sizeof(int));
	m_SumGrayBuf[dtTOP*tmpIMG_W+dtLEFT] = tmpResult[dtTOP*tmpIMG_W+dtLEFT];
	m_SumGrayBuf2[dtTOP*tmpIMG_W+dtLEFT] = tmpResult[dtTOP*tmpIMG_W+dtLEFT]*tmpResult[dtTOP*tmpIMG_W+dtLEFT];

	for(xx=dtLEFT+1; xx<dtRIGHT; xx++)
	{
		m_SumGrayBuf[dtTOP*tmpIMG_W+xx] =
		m_SumGrayBuf[dtTOP*tmpIMG_W+xx-1] + tmpResult[dtTOP*tmpIMG_W+xx];
		m_SumGrayBuf2[dtTOP*tmpIMG_W+xx] =
		m_SumGrayBuf2[dtTOP*tmpIMG_W+xx-1] + tmpResult[dtTOP*tmpIMG_W+xx]*tmpResult[dtTOP*tmpIMG_W+xx];
	}

	for(yy=dtTOP+1; yy<dtBOTTOM; yy++)
	{
		m_SumGrayBuf[yy*tmpIMG_W+dtLEFT] =
		m_SumGrayBuf[(yy-1)*tmpIMG_W+dtLEFT] + tmpResult[yy*tmpIMG_W+dtLEFT];
		m_SumGrayBuf2[yy*tmpIMG_W+dtLEFT] =
		m_SumGrayBuf2[(yy-1)*tmpIMG_W+dtLEFT] + tmpResult[yy*tmpIMG_W+dtLEFT]*tmpResult[yy*tmpIMG_W+dtLEFT];
	}

	for(yy=dtTOP+1; yy<dtBOTTOM; yy++)
	{
		for(xx=dtLEFT+1; xx<dtRIGHT; xx++)
		{
			m_SumGrayBuf[yy*tmpIMG_W+xx] = m_SumGrayBuf[yy*tmpIMG_W+(xx-1)]
												+ m_SumGrayBuf[(yy-1)*tmpIMG_W+xx]
												+ tmpResult[yy*tmpIMG_W+xx]
												- m_SumGrayBuf[(yy-1)*tmpIMG_W+(xx-1)];
			m_SumGrayBuf2[yy*tmpIMG_W+xx] = m_SumGrayBuf2[yy*tmpIMG_W+(xx-1)]
												+ m_SumGrayBuf2[(yy-1)*tmpIMG_W+xx]
												+ tmpResult[yy*tmpIMG_W+xx]*tmpResult[yy*tmpIMG_W+xx]
												- m_SumGrayBuf2[(yy-1)*tmpIMG_W+(xx-1)];
		}
	}

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT * tmpIMG_W / tmpCIS_IMG_W - 1;
		dtRIGHT		= m_pcImgAnalysis->RIGHT * tmpIMG_W / tmpCIS_IMG_W + 1;
		dtTOP		= m_pcImgAnalysis->TOP * tmpIMG_H / tmpCIS_IMG_H - 1;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM * tmpIMG_H / tmpCIS_IMG_H + 1;
	}
	else
	{
		dtLEFT		= m_pcImgAnalysis->LEFT * tmpIMG_W * 2 / tmpCIS_IMG_W - 1;
	    dtRIGHT		= m_pcImgAnalysis->RIGHT * tmpIMG_W * 2 / tmpCIS_IMG_W + 1;
	    dtTOP		= m_pcImgAnalysis->TOP * tmpIMG_H * 2 / tmpCIS_IMG_H - 1;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM * tmpIMG_H * 2 / tmpCIS_IMG_H + 1;
	}

	if(dtTOP < band)	dtTOP = band;
	if(dtLEFT < band)	dtLEFT = band;
	if(dtRIGHT > tmpIMG_W-band)	dtRIGHT = tmpIMG_W-band;
	if(dtBOTTOM > tmpIMG_H-band)	dtBOTTOM = tmpIMG_H-band;

	memset(OutImg, 0, tmpIMG_SIZE);

	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		top = i - (2+1);
        bottom = i + 2;

		for(j=dtLEFT; j<dtRIGHT; j++)
		{
            left = j - (2+1);
            right = j + 2;

            mean_d_local = (m_SumGrayBuf[bottom * tmpIMG_W + right] - m_SumGrayBuf[top * tmpIMG_W + right] - 
							m_SumGrayBuf[bottom * tmpIMG_W + left] + m_SumGrayBuf[top * tmpIMG_W + left]) / 25.0;
            std_d_local = (m_SumGrayBuf2[bottom * tmpIMG_W + right] - m_SumGrayBuf2[top * tmpIMG_W + right] - 
						  m_SumGrayBuf2[bottom * tmpIMG_W + left] + m_SumGrayBuf2[top * tmpIMG_W + left]) / 25.0 
						  - mean_d_local * mean_d_local;

			// to make fast
			if(std_d_local < 9)
			{
				OutImg[i*tmpIMG_W+j] = tmpResult2[i*tmpIMG_W+j];
				continue;
			}
			else if(std_d_local > 100)
			{
				OutImg[i*tmpIMG_W+j] = tmpResult[i*tmpIMG_W+j];
				continue;
			}

			weight = (sqrt(std_d_local) - 3) / 7.0f;

			OutImg[i*tmpIMG_W+j] = (unsigned char)(weight*tmpResult[i*tmpIMG_W+j] + (1.0-weight)*tmpResult2[i*tmpIMG_W+j]);
		}
	}

	for(i=0; i<tmpIMG_H; i++)
	{
		memcpy(&OutImg[i*tmpIMG_W], &tmpResult2[i*tmpIMG_W], 5);
		memcpy(&OutImg[i*tmpIMG_W+tmpIMG_W-6], &tmpResult2[i*tmpIMG_W+tmpIMG_W-6], 5);
	}
	for(i=0; i<5; i++)
	{
		memcpy(&OutImg[i*tmpIMG_W], &tmpResult2[i*tmpIMG_W], tmpIMG_W);
		memcpy(&OutImg[(tmpIMG_H-1-i)*tmpIMG_W], &tmpResult2[(tmpIMG_H-1-i)*tmpIMG_W], tmpIMG_W);
	}
}

void CSBD_Alg::_Make500DPI_preview_opt(unsigned char *InImg, unsigned char *OutImg)
{
#ifdef __embedded__
	_DistortionRestoration_with_Bilinear_preview(InImg, OutImg);
#else
	_DistortionRestoration_with_Bilinear_preview(InImg, OutImg);
#endif
}

void CSBD_Alg::_DistortionRestoration_with_Cubic(unsigned char *Image, unsigned char *output)
{
	float Value, TempValue, U, V;
	int value_int;
	int i, j, k, l, pos, curposy, curposx;
	
	// 실제 사용되는 영역만 distortion restoration을 한다.
//	for (i=0; i<IMG_SIZE; i++)
//	{

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

	int tmpIMG_W = IMG_W;
	int tmpIMG_H = IMG_H;
	int tmpIMG_SIZE = IMG_SIZE;
	int tmpCIS_IMG_W = CIS_IMG_W;
	int tmpCIS_IMG_H = CIS_IMG_H;
//	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
//	int pos_x, pos_y;
//	int move_x_org = 0;
//	int move_x_tgt = 0;
	DWORD *p_arrPos_wb = arrPos_wb;
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		p_arrPos_wb = arrPos_wb_KOJAK_ROLL;

		tmpIMG_W = IMG_W_ROLL;
		tmpIMG_H = IMG_H_ROLL;
		tmpIMG_SIZE = IMG_SIZE_ROLL;
		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
//		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
//		move_x_org = (CIS_IMG_W - CIS_IMG_W_ROLL)/2;
//		move_x_tgt = (IMG_W - IMG_W_ROLL)/2;		
	
		dtLEFT		= m_pcImgAnalysis->LEFT * tmpIMG_W / tmpCIS_IMG_W - 1;
		dtRIGHT		= m_pcImgAnalysis->RIGHT * tmpIMG_W / tmpCIS_IMG_W + 1;
		dtTOP		= m_pcImgAnalysis->TOP * tmpIMG_H / tmpCIS_IMG_H - 1;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM * tmpIMG_H / tmpCIS_IMG_H + 1;
	}
	else
	{
		dtLEFT		= m_pcImgAnalysis->LEFT * tmpIMG_W * 2 / tmpCIS_IMG_W - 1;
	    dtRIGHT		= m_pcImgAnalysis->RIGHT * tmpIMG_W * 2 / tmpCIS_IMG_W + 1;
	    dtTOP		= m_pcImgAnalysis->TOP * tmpIMG_H * 2 / tmpCIS_IMG_H - 1;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM * tmpIMG_H * 2 / tmpCIS_IMG_H + 1;
	}

	if(dtLEFT < 0) dtLEFT = 0;
	if(dtTOP < 0) dtTOP = 0;
	if(dtRIGHT > tmpIMG_W) dtRIGHT = tmpIMG_W;
	if(dtBOTTOM > tmpIMG_H) dtBOTTOM = tmpIMG_H;

	memset(output, 0, tmpIMG_SIZE);

	for (i=dtTOP; i<dtBOTTOM; i++)
	{
		for (j=dtLEFT; j<dtRIGHT; j++)
		{
			pos = (p_arrPos_wb[i*tmpIMG_W+j]>>10)&0x3FFFFF;
			curposx = 32-((p_arrPos_wb[i*tmpIMG_W+j]>>5)&0x1F);
			curposy = 32-(p_arrPos_wb[i*tmpIMG_W+j]&0x1F);
			
			Value = 0.0f;
			for(k=-2; k<=2; k++)
			{
				U = Cubic_UV[k+2][curposy];
				if(U == 0) continue;
				TempValue = 0.0f;
				for(l=-2; l<=2; l++)
				{
					V = Cubic_UV[l+2][curposx];
					if(V == 0) continue;
					
					TempValue += V * (float)Image[pos + k*tmpCIS_IMG_W+l];
				}
				Value += TempValue * U;
			}

			value_int = (int)(Value+0.5f);
			
			if(value_int > 255) value_int = 255;
			else if(value_int < 0) value_int = 0;

			output[i*tmpIMG_W+j] = (unsigned char)value_int;
		}
	}
}

void CSBD_Alg::_DistortionRestoration_with_Bilinear(unsigned char *InImg, unsigned char *OutImg)
{
	int value;
	int i, j, pos, sp, sq;

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;
	
	int tmpIMG_W = IMG_W;
	int tmpIMG_H = IMG_H;
	int tmpIMG_SIZE = IMG_SIZE;
	int tmpCIS_IMG_W = CIS_IMG_W;
	int tmpCIS_IMG_H = CIS_IMG_H;
//	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;
//	int pos_x, pos_y;
//	int move_x_org = 0;
//	int move_x_tgt = 0;
	DWORD *p_arrPos_wb = arrPos_wb;
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		p_arrPos_wb = arrPos_wb_KOJAK_ROLL;

		tmpIMG_W = IMG_W_ROLL;
		tmpIMG_H = IMG_H_ROLL;
		tmpIMG_SIZE = IMG_SIZE_ROLL;
		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
//		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
//		move_x_org = (CIS_IMG_W - CIS_IMG_W_ROLL)/2;
//		move_x_tgt = (IMG_W - IMG_W_ROLL)/2;

		dtLEFT		= m_pcImgAnalysis->LEFT * tmpIMG_W / tmpCIS_IMG_W - 1;
		dtRIGHT		= m_pcImgAnalysis->RIGHT * tmpIMG_W / tmpCIS_IMG_W + 1;
		dtTOP		= m_pcImgAnalysis->TOP * tmpIMG_H / tmpCIS_IMG_H - 1;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM * tmpIMG_H / tmpCIS_IMG_H + 1;
	}
	else
	{
		dtLEFT		= m_pcImgAnalysis->LEFT * tmpIMG_W * 2 / tmpCIS_IMG_W - 1;
	    dtRIGHT		= m_pcImgAnalysis->RIGHT * tmpIMG_W * 2 / tmpCIS_IMG_W + 1;
	    dtTOP		= m_pcImgAnalysis->TOP * tmpIMG_H * 2 / tmpCIS_IMG_H - 1;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM * tmpIMG_H * 2 / tmpCIS_IMG_H + 1;
	}

	if(dtLEFT < 0) dtLEFT = 0;
	if(dtTOP < 0) dtTOP = 0;
	if(dtRIGHT > tmpIMG_W) dtRIGHT = tmpIMG_W;
	if(dtBOTTOM > tmpIMG_H) dtBOTTOM = tmpIMG_H;

	// 실제 사용되는 영역만 distortion restoration을 한다.
	memset(OutImg, 0, tmpIMG_SIZE);

	for (i=dtTOP; i<dtBOTTOM; i++)
	{
		for (j=dtLEFT; j<dtRIGHT; j++)
		{
			pos = (p_arrPos_wb[i*tmpIMG_W+j]>>10)&0x3FFFFF;
			sp = ((p_arrPos_wb[i*tmpIMG_W+j]>>5)&0x1F);
			sq = (p_arrPos_wb[i*tmpIMG_W+j]&0x1F);

			value = ( sp*( sq*InImg[pos] + (32-sq)*InImg[pos+tmpCIS_IMG_W] ) + (32-sp)*( sq*InImg[pos+1] + (32-sq)*InImg[pos+tmpCIS_IMG_W+1] ) ) >> 10;

			OutImg[i*tmpIMG_W+j] = (unsigned char)value;
		}
	}
}

void CSBD_Alg::_DistortionRestoration_with_Bilinear_preview(unsigned char *InImg, unsigned char *OutImg)
{
	int i, j, pos, pos_x, pos_y;//, sp, sq, value;
	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;
//	int move_x_org = (CIS_IMG_W-CIS_IMG_W_ROLL)/2;
//	int move_x_tgt = (IMG_W-IMG_W_ROLL)/2;
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		dtLEFT		= m_pcImgAnalysis->LEFT * IMG_W_ROLL / CIS_IMG_W_ROLL;
		dtRIGHT		= m_pcImgAnalysis->RIGHT * IMG_W_ROLL / CIS_IMG_W_ROLL;
		dtTOP		= m_pcImgAnalysis->TOP * IMG_H_ROLL / CIS_IMG_H_ROLL;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM * IMG_H_ROLL / CIS_IMG_H_ROLL;

		if(dtLEFT < 1) dtLEFT = 1;
		if(dtTOP < 1) dtTOP = 1;
		if(dtRIGHT > IMG_W_ROLL-2) dtRIGHT = IMG_W_ROLL-2;
		if(dtBOTTOM > IMG_H_ROLL-2) dtBOTTOM = IMG_H_ROLL-2;

		//memset(OutImg, 0, IMG_SIZE_ROLL);
		memset(OutImg, 255, IMG_SIZE_ROLL);

		// 실제 사용되는 영역만 distortion restoration을 한다.
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			for(j=dtLEFT; j<dtRIGHT; j++)
			{
/*				pos = (arrPos_wb[i*IMG_W+(j+move_x_tgt)]>>10) & 0x3FFFFF;
				pos_x = pos % CIS_IMG_W;
				pos_y = pos / CIS_IMG_W;
				pos = pos_y*CIS_IMG_W_ROLL+pos_x-move_x_org;
				OutImg[i*IMG_W_ROLL+j] = 255-InImg[pos];*/
				pos = (arrPos_wb_KOJAK_ROLL[i*IMG_W_ROLL+j]>>10) & 0x3FFFFF;
				OutImg[i*IMG_W_ROLL+j] = 255-InImg[pos];
			}
		}
	}
	else
	{
		dtLEFT		= m_pcImgAnalysis->LEFT * IMG_W / CIS_IMG_W;
		dtRIGHT		= m_pcImgAnalysis->RIGHT * IMG_W / CIS_IMG_W;
		dtTOP		= m_pcImgAnalysis->TOP * IMG_H / CIS_IMG_H;
		dtBOTTOM	= m_pcImgAnalysis->BOTTOM * IMG_H / CIS_IMG_H;

		if(dtLEFT < 1) dtLEFT = 1;
		if(dtTOP < 1) dtTOP = 1;
		if(dtRIGHT > IMG_W/2-2) dtRIGHT = IMG_W/2-2;
		if(dtBOTTOM > IMG_H/2-2) dtBOTTOM = IMG_H/2-2;

		//memset(OutImg, 0, IMG_SIZE/4);
		memset(OutImg, 255, IMG_SIZE/4);

		// 실제 사용되는 영역만 distortion restoration을 한다.
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			for(j=dtLEFT; j<dtRIGHT; j++)
			{
				pos = (arrPos_wb[i*2*IMG_W+j*2]>>10) & 0x3FFFFF;
				pos_x = (pos % CIS_IMG_W) / 2;
				pos_y = (pos / CIS_IMG_W) / 2;
				pos = pos_y*(CIS_IMG_W/2)+pos_x;
//				sp = arrP_wb[i*2*IMG_W+j*2];
//				sq = arrQ_wb[i*2*IMG_W+j*2];
//				value = ( sp*( sq*InImg[pos] + (128-sq)*InImg[pos+CIS_IMG_W/2] ) + (128-sp)*( sq*InImg[pos+1] + (128-sq)*InImg[pos+CIS_IMG_W/2+1] ) ) >> 14;
//				OutImg[i*IMG_W/2+j] = value;

				OutImg[i*IMG_W/2+j] = 255-InImg[pos];
			}
		}
	}
}

void CSBD_Alg::_Init_Distortion_Merge_with_Bilinear(float DISTORTION_A, float DISTORTION_B, float DISTORTION_C, float DISTORTION_D, 
																	   int cut_x, int cut_y, int shift_x, int shift_y)
{
//	int DISTORTION_SX = 0;
//	int DISTORTION_SY = 0;

	int cut_width = cut_x;//48; 
    int cut_height = cut_y;//58;

	int DISTORTION_RX = (CIS_IMG_W-cut_width)/2;
	int DISTORTION_RY = (CIS_IMG_H-cut_height)/2;
	
	int i, j, x, y;
	float DestX, DestY, SrcX, SrcY;
	float DestR, DestA, SrcR, percent;
	float p, q;
	int HalfX, HalfY, TargetHalfX, TargetHalfY;
//	int shifty = cut_height/2 + shift_y;
//	int shiftx = cut_width/2 + shift_x;
	
	TargetHalfX = DISTORTION_RX;
	TargetHalfY = DISTORTION_RY;
	
	HalfX = IMG_W/2;
	HalfY = IMG_H/2;
	
	// make table
	for(i=-HalfY; i<HalfY; i++)
	{
		DestY = (float)i * TargetHalfY / (float)HalfY;
		
		for(j=-HalfX; j<HalfX; j++)
		{			
			DestX = (float)j * TargetHalfX / (float)HalfX;
			
			DestR = sqrtf(DestY*DestY + DestX*DestX);
			DestA = atan2f(DestY, DestX);

			percent = DISTORTION_A*DestR*DestR*DestR*DestR + DISTORTION_B*DestR*DestR*DestR + DISTORTION_C*DestR*DestR + DISTORTION_D*DestR;

			SrcR = DestR * (1.0f - percent);

			SrcX = SrcR * cosf(DestA) + CIS_IMG_W/2 + shift_x + 0.5f;
			SrcY = SrcR * sinf(DestA) + CIS_IMG_H/2 + shift_y + 0.5f;
			 
			x = (int)SrcX;
			y = (int)SrcY;
			
			if (x >= 0 && x < CIS_IMG_W && y >= 0 && y < CIS_IMG_H)
            {
                p = 1.0f - (SrcX - (float)x);
                q = 1.0f - (SrcY - (float)y);

                arrPos_wb[(i + HalfY)*IMG_W + j + HalfX] = (DWORD)(((y * CIS_IMG_W + x) & 0x3FFFFF) << 10);
                arrPos_wb[(i + HalfY)*IMG_W + j + HalfX] |= (DWORD)((int)(p * 32) << 5);
                arrPos_wb[(i + HalfY)*IMG_W + j + HalfX] |= (DWORD)(q * 32);
            }
            else
            {
                arrPos_wb[(i + HalfY)*IMG_W + j + HalfX] = 0xffffffff;
            }
		}
	}
}

void CSBD_Alg::Init_Distortion_Merge_with_Bilinear_forDaiwon(int cutx, int cuty, int shift_x, int shift_y)
{
	int DISTORTION_SX = 0;
	int DISTORTION_SY = 0;
	const int DEFAULT_CUT_X = 2;	//		-38(기존 기본값) + 40(대원에게 전달받은 x평균);
	const int DEFAULT_CUT_Y = 5;	//		-30(기존 기본값) + 35(대원에게 전달받은 y평균);

	int cut_width = DEFAULT_CUT_X + cutx;//48; 
    int cut_height = DEFAULT_CUT_Y + cuty;//58;

	int DISTORTION_RX = (CIS_IMG_W-cut_width)/2;
	int DISTORTION_RY = (CIS_IMG_H-cut_height)/2;
	
	int i, j, x, y;
	float DestX, DestY, SrcX, SrcY;
	float DestR, DestA, SrcR, percent;
	float p, q;
	int HalfX, HalfY, TargetHalfX, TargetHalfY;
	int shifty = cut_height/2 + shift_y;
	int shiftx = cut_width/2 + shift_x;
	
	TargetHalfX = DISTORTION_RX;
	TargetHalfY = DISTORTION_RY;
	
	HalfX = IMG_W/2;
	HalfY = IMG_H/2;

	int s, fi;
	float f_idx[11] = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f};
	float fi_distort, index, weight;
	float diag_dist = (float)sqrtf((float)TargetHalfX*TargetHalfX + TargetHalfY*TargetHalfY);
	
	// Daiwon Lens
	//float f_distort[11] = {0.0, -0.03, -0.10, -0.14, -0.10, -0.02, 0.05, -0.33, -1.04, -2.45, -5.00};
	float f_distort[11] = {0.0f, -0.03f, -0.10f, -0.10f, -0.10f, -0.02f, 0.00f, -0.33f, -1.04f, -2.50f, -5.00f};

	//float diag_dist_base = sqrtf((m_CIS_IMG_W-100)*(m_CIS_IMG_W-100)/4 + (m_CIS_IMG_H-100)*(m_CIS_IMG_H-100)/4);
	float diag_dist_base = sqrtf((float)(CIS_IMG_W-100)*(CIS_IMG_W-100)/4 + (CIS_IMG_H-100)*(CIS_IMG_H-100)/4);

	// ACE lens (not matched - modified?)
	/*
	float f_distort[11] = {0.0, -0.05, -0.17, -0.22, 0.03, 0.43, 0.50, -0.05, -1.23, -2.95, -5.00};
	float diag_dist_base = sqrtf((m_CIS_IMG_W)*(m_CIS_IMG_W)/4 + (m_CIS_IMG_H)*(m_CIS_IMG_H)/4);
	
	CString TmpRead, field_name;
	for(s=0; s<11; s++)
	{
		field_name.Format("F%02d", s);
		if( GetPrivateProfileString(_T("test"), _T(field_name), NULL, TmpRead.GetBuffer(256), 256, "d:\\setting.ini") > 0 )
		{
			f_distort[s] = atof(TmpRead.GetBuffer());
		}
	}*/
	

	// make table
	for(i=-HalfY; i<HalfY; i++)
	{
		DestY = (float)i * TargetHalfY / (float)HalfY;
		
		for(j=-HalfX; j<HalfX; j++)
		{			
			DestX = (float)j * TargetHalfX / (float)HalfX;
			
			DestR = sqrtf(DestY*DestY + DestX*DestX);
			DestA = atan2f(DestY, DestX);

			index = DestR / diag_dist_base;
			if(index >= 1.0)
			{
				fi_distort = f_distort[10];
			}
			else
			{
				fi = -1;
				fi_distort = 0.0f;
				for(s=0; s<10; s++)
				{
					if(f_idx[s] <= index && f_idx[s+1] > index)
					{
						fi = s;
						weight = (f_idx[s+1]-index) / 0.1f;
						fi_distort = weight*f_distort[s] + (1.0f-weight)*f_distort[s+1];
						break;
					}
				}
			}

			percent = fi_distort;
			// Daiwon
			SrcR = DestR + diag_dist * percent / 145;	// right answer

			// Ace Solutech
			//SrcR = DestR + diag_dist * percent / 100;

			SrcX = SrcR * cosf(DestA) + CIS_IMG_W/2 + shift_x + 0.5f;
			SrcY = SrcR * sinf(DestA) + CIS_IMG_H/2 + shift_y + 0.5f;

			x = (int)SrcX;
			y = (int)SrcY;
			
			if(x>=0 && x<CIS_IMG_W && y>=0 && y<CIS_IMG_H)
			{
				p = 1.0f - (SrcX - (float)x);
                q = 1.0f - (SrcY - (float)y);

                arrPos_wb[(i + HalfY)*IMG_W + j + HalfX] = (DWORD)(((y * CIS_IMG_W + x) & 0x3FFFFF) << 10);
                arrPos_wb[(i + HalfY)*IMG_W + j + HalfX] |= (DWORD)((int)(p * 32) << 5);
                arrPos_wb[(i + HalfY)*IMG_W + j + HalfX] |= (DWORD)(q * 32);
			}
			else
			{
				arrPos_wb[(i + HalfY)*IMG_W + j + HalfX] = 0xffffffff;
			}
		}
	}
}

void CSBD_Alg::_MakeMosaicImage(unsigned char *InImg)
{
	int y, x, s, t;
	int median_window[64];
	int val;
	int mosaic_size=6;
	int pad_x = (IMG_W/2)%mosaic_size;
	int pad_y = (IMG_H/2)%mosaic_size;
	unsigned char *tmpResult = (unsigned char *)&m_OriginalImg_int[0];

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;
	
    dtLEFT		= m_pcImgAnalysis->LEFT * IMG_W / CIS_IMG_W;
    dtRIGHT		= m_pcImgAnalysis->RIGHT * IMG_W / CIS_IMG_W;
    dtTOP		= m_pcImgAnalysis->TOP * IMG_H / CIS_IMG_H;
    dtBOTTOM	= m_pcImgAnalysis->BOTTOM * IMG_H / CIS_IMG_H;

	if(dtLEFT < 1) dtLEFT = 1;
	if(dtTOP < 1) dtTOP = 1;
	if(dtRIGHT > IMG_W/2-pad_x) dtRIGHT = IMG_W/2-pad_x;
	if(dtBOTTOM > IMG_H/2-pad_y) dtBOTTOM = IMG_H/2-pad_y;

	memcpy(tmpResult, InImg, IMG_SIZE/4);
	memset(InImg, 0, IMG_SIZE/4);

	for(y=dtTOP; y<dtBOTTOM; y+=mosaic_size)
	{
		for(x=dtLEFT; x<dtRIGHT; x+=mosaic_size)
		{
			for(s=0; s<mosaic_size; s++)
			{
				for(t=0; t<mosaic_size; t++)
				{
					median_window[s*mosaic_size+t] = tmpResult[(y+s)*(IMG_W/2)+(x+t)];
				}
			}

			for(s=0; s<mosaic_size*mosaic_size/2; s++)
			{
				for(t=s+1; t<mosaic_size*mosaic_size; t++)
				{
					if(median_window[s] < median_window[t])
					{
						val = median_window[s];
						median_window[s] = median_window[t];
						median_window[t] = val;
					}
				}
			}
			
			val = median_window[mosaic_size*mosaic_size/4];

			for(s=0; s<mosaic_size; s++)
			{
				for(t=0; t<mosaic_size; t++)
				{
					InImg[(y+s)*(IMG_W/2)+(x+t)] = val;
				}
			}
		}
	}
}

void CSBD_Alg::_MakeCleanImage(unsigned char *InImg)
{
	int y, x, s, t;
	int median_window[64];
	int val;
	int mosaic_size=2;
	int pad_x = (IMG_W/2)%mosaic_size;
	int pad_y = (IMG_H/2)%mosaic_size;

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;
	unsigned char *tmpResult = (unsigned char *)&m_OriginalImg_int[0];
	
    dtLEFT		= m_pcImgAnalysis->LEFT * IMG_W / CIS_IMG_W;
    dtRIGHT		= m_pcImgAnalysis->RIGHT * IMG_W / CIS_IMG_W;
    dtTOP		= m_pcImgAnalysis->TOP * IMG_H / CIS_IMG_H;
    dtBOTTOM	= m_pcImgAnalysis->BOTTOM * IMG_H / CIS_IMG_H;

	if(dtLEFT < 1) dtLEFT = 1;
	if(dtTOP < 1) dtTOP = 1;
	if(dtRIGHT > IMG_W/2-pad_x) dtRIGHT = IMG_W/2-pad_x;
	if(dtBOTTOM > IMG_H/2-pad_y) dtBOTTOM = IMG_H/2-pad_y;

	memcpy(tmpResult, InImg, IMG_SIZE/4);
	memset(InImg, 0, IMG_SIZE/4);

	for(y=dtTOP; y<dtBOTTOM; y+=mosaic_size)
	{
		for(x=dtLEFT; x<dtRIGHT; x+=mosaic_size)
		{
			if(tmpResult[y*(IMG_W/2)+x] < 20)
				continue;

			for(s=0; s<mosaic_size; s++)
			{
				for(t=0; t<mosaic_size; t++)
				{
					median_window[s*mosaic_size+t] = tmpResult[(y+s)*(IMG_W/2)+(x+t)];
				}
			}

			for(s=0; s<3; s++)
			{
				for(t=s+1; t<mosaic_size*mosaic_size; t++)
				{
					if(median_window[s] < median_window[t])
					{
						val = median_window[s];
						median_window[s] = median_window[t];
						median_window[t] = val;
					}
				}
			}
			
			val = median_window[2];//(median_window[1]+median_window[2])/2;

			for(s=0; s<mosaic_size; s++)
			{
				for(t=0; t<mosaic_size; t++)
				{
					InImg[(y+s)*(IMG_W/2)+(x+t)] = val;
				}
			}
		}
	}
}

void CSBD_Alg::_HistogramFilling(unsigned char *InImg, unsigned char *OutImg, int width, int height)
{
	//return;
/*	int i,j;
	int val;
	for(i=1; i<height-1; i++)
	{
		for(j=1; j<width-1; j++)
		{
			OutImg[i*width+j] = (unsigned char)((float)(InImg[(i-1)*width+j-1] + 
												InImg[(i-1)*width+j] + 
												InImg[(i-1)*width+j+1] + 
												InImg[i*width+j-1] + 
												InImg[i*width+j]*16 + 
												InImg[i*width+j+1] + 
												InImg[(i+1)*width+j-1] + 
												InImg[(i+1)*width+j] + 
												InImg[(i+1)*width+j+1])/24.0f + 0.5f);
		}
	}
*/
	int tmpCIS_IMG_W = width;
	int tmpCIS_IMG_H = height;
//	int tmpCIS_IMG_SIZE = width*height;
	
	int tmpEnCIS_IMG_W = 0;
	int tmpEnCIS_IMG_H = 0;
//	int tmpEnCIS_IMG_SIZE = 0;

	tmpEnCIS_IMG_W = (int)(tmpCIS_IMG_W*1.65+64);
	tmpEnCIS_IMG_H = (int)(tmpCIS_IMG_H*1.65+64);
//	tmpEnCIS_IMG_SIZE = tmpEnCIS_IMG_W*tmpEnCIS_IMG_H;

	unsigned char *m_ResultImg = new unsigned char [tmpEnCIS_IMG_W*tmpEnCIS_IMG_H];
	_DoubleEnlarge(InImg, tmpCIS_IMG_W, tmpCIS_IMG_H, m_ResultImg, tmpEnCIS_IMG_W, tmpEnCIS_IMG_H);
	_DeDoubleEnlarge(m_ResultImg, tmpEnCIS_IMG_W, tmpEnCIS_IMG_H, OutImg, tmpCIS_IMG_W, tmpCIS_IMG_H);
	delete [] m_ResultImg;
}

void CSBD_Alg::_RemoveSpecificNoise_int_double_enlarge(short *InImg, short *OutImg, int width, int height)
{
	int i, j;
	int max_bright, pixel_bright;
	int histogram[1024], total_pixel_cnt;
	double NOISE_THRES = 0.1;//atof(m_NoiseThres);
	double noise_ratio;
	int LEFT, RIGHT, TOP, BOTTOM;
	
	LEFT = 5;
	RIGHT = width-5;
	TOP  = 5;
	BOTTOM = height-5;

	memset(histogram, 0, sizeof(histogram));
	for(i=TOP; i<BOTTOM; i++)
	{
		for(j=LEFT; j<RIGHT; j++)
		{
			histogram[InImg[i*width+j]]++;
		}
	}

	total_pixel_cnt = 0;
	for(i=16; i<1024; i++)
		total_pixel_cnt += histogram[i];

	max_bright= 0;
	for(i=1023; i>=0; i--)
	{
		if(histogram[i] > 0)
		{
			max_bright = i;
			break;
		}
	}

	max_bright = (int)(max_bright - max_bright*0.2);
	if(max_bright<0)
		max_bright=0;
			
	memcpy(OutImg, InImg, width*height*sizeof(short));

	for (i=TOP; i<BOTTOM; i++)
	{
		for (j=LEFT; j<RIGHT; j++)
		{
			pixel_bright = InImg[i*width+j];
						
			if(pixel_bright >= max_bright)
			{
				noise_ratio = histogram[pixel_bright] * 100.0 / total_pixel_cnt;

				if(NOISE_THRES >= noise_ratio)
				{
					OutImg[i*width+j] = ( InImg[(i-1)*width+(j-1)] + 
										InImg[(i-1)*width+(j)] + 
										InImg[(i-1)*width+(j+1)] + 
										InImg[(i)*width+(j-1)] + 
										InImg[(i)*width+(j+1)] + 
										InImg[(i+1)*width+(j-1)] + 
										InImg[(i+1)*width+(j)] + 
										InImg[(i+1)*width+(j+1)] ) / 8;
				}
			}
		}
	}
}

void CSBD_Alg::_StdDevSmoothing_int_opt_double_enlarge(short *InImg, short *OutImg, int width, int height)
{
	int i, j;
	float mean_d_local, std_d_local, weight_bright;
    int left, right, top, bottom;
	int xx, yy;
	int band = 0;
	
//	int mask_size=51;
//	int half_mask_size=mask_size/2;

	int LEFT = band;//half_mask_size;
	int RIGHT = width-band;//half_mask_size-1;
	int TOP = band;//half_mask_size;
	int BOTTOM = height-band;//half_mask_size-1;

	int mean;
//	int diff_thres = 15;
	double current_weight;

//	double max_intensity = 255;//*255;//*255;

	memcpy(OutImg, InImg, width*height*sizeof(short));

	LEFT = 0;
	RIGHT = width;
	TOP  = 0;
	BOTTOM = height;

	// calc bright weight
/*	for(i=TOP; i<BOTTOM; i++)
	{
		for(j=LEFT; j<RIGHT; j++)
		{
//			weight = InImg[i*CIS_IMG_W+j];
			weight_table_bright[i*width+j] = InImg[i*width+j] / max_intensity;
			if(weight_table_bright[i*width+j] > 1.0)
				weight_table_bright[i*width+j] = 1.0;
		}
	}
*/
	// calc dev weight
	memset(weight_table_diff, 0, width*height*sizeof(float));

	// 60 ~ 70ms
	memset(m_SumGrayBuf, 0, width*height * sizeof(int));
	memset(m_SumGrayBuf2, 0, width*height * sizeof(int));
	m_SumGrayBuf[TOP*width+LEFT] = InImg[TOP*width+LEFT];
	m_SumGrayBuf2[TOP*width+LEFT] = InImg[TOP*width+LEFT]*InImg[TOP*width+LEFT];

	for(xx=LEFT+1; xx<RIGHT; xx++)
	{
		m_SumGrayBuf[TOP*width+xx] =
		m_SumGrayBuf[TOP*width+xx-1] + InImg[TOP*width+xx];
		m_SumGrayBuf2[TOP*width+xx] =
		m_SumGrayBuf2[TOP*width+xx-1] + InImg[TOP*width+xx]*InImg[TOP*width+xx];
	}

	for(yy=TOP+1; yy<BOTTOM; yy++)
	{
		m_SumGrayBuf[yy*width+LEFT] =
		m_SumGrayBuf[(yy-1)*width+LEFT] + InImg[yy*width+LEFT];
		m_SumGrayBuf2[yy*width+LEFT] =
		m_SumGrayBuf2[(yy-1)*width+LEFT] + InImg[yy*width+LEFT]*InImg[yy*width+LEFT];
	}

	for(yy=TOP+1; yy<BOTTOM; yy++)
	{
		for(xx=LEFT+1; xx<RIGHT; xx++)
		{
			m_SumGrayBuf[yy*width+xx] = m_SumGrayBuf[yy*width+(xx-1)]
												+ m_SumGrayBuf[(yy-1)*width+xx]
												+ InImg[yy*width+xx]
												- m_SumGrayBuf[(yy-1)*width+(xx-1)];
			m_SumGrayBuf2[yy*width+xx] = m_SumGrayBuf2[yy*width+(xx-1)]
												+ m_SumGrayBuf2[(yy-1)*width+xx]
												+ InImg[yy*width+xx]*InImg[yy*width+xx]
												- m_SumGrayBuf2[(yy-1)*width+(xx-1)];
		}
	}

	LEFT = 5;
	RIGHT = width-5;
	TOP  = 5;
	BOTTOM = height-5;

	for(i=TOP; i<BOTTOM; i++)
	{
		top = i - (2+1);
        bottom = i + 2;

		for(j=LEFT; j<RIGHT; j++)
		{
            left = j - (2+1);
            right = j + 2;

            mean_d_local = (m_SumGrayBuf[bottom * width + right] - m_SumGrayBuf[top * width + right] - 
							m_SumGrayBuf[bottom * width + left] + m_SumGrayBuf[top * width + left]) / 25.0f;
            std_d_local = (m_SumGrayBuf2[bottom * width + right] - m_SumGrayBuf2[top * width + right] - 
						  m_SumGrayBuf2[bottom * width + left] + m_SumGrayBuf2[top * width + left]) / 25.0f
						  - mean_d_local * mean_d_local;

			// to make fast
			if(std_d_local < 225)
			{
				weight_table_diff[i*width+j] = 0;
				continue;
			}
			else if(std_d_local > 13225)
			{
				weight_table_diff[i*width+j] = 1.0;
				continue;
			}

			// 200 ms
			std_d_local = sqrt(std_d_local) - 15;
			weight_table_diff[i*width+j] = std_d_local / 100.0f;
		}
	}
	
	// merge weight
//	memset(weight_table_total, 0, sizeof(weight_table_total));
//	for(i=0; i<CIS_IMG_SIZE; i++)
//		weight_table_total[i] = 0;

	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			// 경계는 덜 뭉게도록 diff weight를 - 가중치를 준다.
			if(InImg[i*width+j] > 254)
				weight_bright = 1.0f;
			else
				weight_bright = InImg[i*width+j] / 255.0f;//max_intensity;

			weight_table_diff[i*width+j] = weight_bright * (1.0f - weight_table_diff[i*width+j]);
		}
	}

//	max_weight = 1.0;
	for(i=TOP; i<BOTTOM; i++)
	{
		top = i - (1+1);
        bottom = i + 1;

		for(j=LEFT; j<RIGHT; j++)
		{
			// to make fast
//			if(InImg[i*CIS_IMG_W+j] == 0)
//				continue;

/*			mean = (int)((InImg[(i-1)*CIS_IMG_W+j-1] + 
							InImg[(i-1)*CIS_IMG_W+j] + 
							InImg[(i-1)*CIS_IMG_W+j+1] + 
							InImg[i*CIS_IMG_W+j-1] + 
//							InImg[i*CIS_IMG_W+j] + 
							InImg[i*CIS_IMG_W+j+1] + 
							InImg[(i+1)*CIS_IMG_W+j-1] + 
							InImg[(i+1)*CIS_IMG_W+j] + 
							InImg[(i+1)*CIS_IMG_W+j+1]) / 8.0 + 0.5);
*/

            left = j - (1+1);
            right = j + 1;

			mean = (int)((m_SumGrayBuf[bottom * width + right] -
							m_SumGrayBuf[top * width + right] - 
							m_SumGrayBuf[bottom * width + left] + 
							m_SumGrayBuf[top * width + left] - 
							InImg[i*width+j])/8 + 0.5);

			current_weight = weight_table_diff[i*width+j];
			OutImg[i*width+j] = (int)((2.0-current_weight) * InImg[i*width+j] + current_weight * mean) / 2;// / max_weight);
		}
	}
}

void CSBD_Alg::_DoubleEnlarge(unsigned char *InImg, int in_width, int in_height, unsigned char *OutImg, int out_width, int out_height)
{
	int i, j;
	int cx, cy, fr_x, fr_y;
	int val, temp1, temp2;

	int pad_w = 32;
	int pad_h = 32;
	int en_width = out_width - pad_w*2;
	int en_height = out_height - pad_h*2;

	double fx, fy, nx, ny;
	double nWidthFactor = (double)in_width / (double)en_width;
    double nHeightFactor = (double)in_height / (double)en_height;

	for(i=pad_h; i<out_height-pad_h; i++)
	{
        fr_y = (int)floor((i-pad_h) * nHeightFactor);
		if (fr_y >= in_height-1) fr_y = in_height-1;
        cy = fr_y + 1;
        if (cy >= in_height-1) cy = in_height-1;
        fy = (i-pad_h) * nHeightFactor - fr_y;
        ny = 1.0 - fy;

		for(j=pad_w; j<out_width-pad_w; j++)
		{
			fr_x = (int)floor((j-pad_w) * nWidthFactor);
			if (fr_x >= in_width-1) fr_x = in_width-1;
			cx = fr_x + 1;
			if (cx >= in_width-1) cx = in_width-1;
			fx = (j-pad_w) * nWidthFactor - fr_x;
			nx = 1.0 - fx;

			temp1 = (unsigned char)(nx * InImg[fr_y*in_width+fr_x] + fx * InImg[fr_y*in_width+cx]);
			temp2 = (unsigned char)(nx * InImg[cy*in_width+fr_x] + fx * InImg[cy*in_width+cx]);
			val = (unsigned char)(ny * (double)(temp1) + fy * (double)(temp2));
			OutImg[i*out_width+j] = val;
		}
	}

	for(i=0; i<pad_h; i++)
	{
		for(j=pad_w; j<out_width-pad_w; j++)
		{
			OutImg[i*out_width+j] = OutImg[(pad_h-1-i+pad_h)*out_width+j];
		}
		for(j=pad_w; j<out_width-pad_w; j++)
		{
			OutImg[(out_height-1-i)*out_width+j] = OutImg[(out_height-1-pad_h-pad_h+i)*out_width+j];
		}
	}
	for(i=0; i<out_height; i++)
	{
		for(j=0; j<pad_w; j++)
		{
			OutImg[i*out_width+j] = OutImg[i*out_width+pad_w+pad_w-j];
			OutImg[i*out_width+out_width-pad_w+j] = OutImg[i*out_width+out_width-1-pad_w-j];
		}
	}
}

void CSBD_Alg::_DoubleEnlarge_int(short *InImg, int in_width, int in_height, short *OutImg, int out_width, int out_height)
{
	int i, j;
	int cx, cy, fr_x, fr_y;
	int val, temp1, temp2;

	int pad_w = 32;
	int pad_h = 32;
	int en_width = out_width - pad_w*2;
	int en_height = out_height - pad_h*2;

	double fx, fy, nx, ny;
	double nWidthFactor = (double)in_width / (double)en_width;
    double nHeightFactor = (double)in_height / (double)en_height;

	for(i=pad_h; i<out_height-pad_h; i++)
	{
        fr_y = (int)floor((i-pad_h) * nHeightFactor);
		if (fr_y >= in_height-1) fr_y = in_height-1;
        cy = fr_y + 1;
        if (cy >= in_height-1) cy = in_height-1;
        fy = (i-pad_h) * nHeightFactor - fr_y;
        ny = 1.0 - fy;

		for(j=pad_w; j<out_width-pad_w; j++)
		{
			fr_x = (int)floor((j-pad_w) * nWidthFactor);
			if (fr_x >= in_width-1) fr_x = in_width-1;
			cx = fr_x + 1;
			if (cx >= in_width-1) cx = in_width-1;
			fx = (j-pad_w) * nWidthFactor - fr_x;
			nx = 1.0 - fx;

			temp1 = (short)(nx * InImg[fr_y*in_width+fr_x] + fx * InImg[fr_y*in_width+cx]);
			temp2 = (short)(nx * InImg[cy*in_width+fr_x] + fx * InImg[cy*in_width+cx]);
			val = (short)(ny * (double)(temp1) + fy * (double)(temp2));
			OutImg[i*out_width+j] = val;
		}
	}

	for(i=0; i<pad_h; i++)
	{
		for(j=pad_w; j<out_width-pad_w; j++)
		{
			OutImg[i*out_width+j] = OutImg[(pad_h-1-i+pad_h)*out_width+j];
		}
		for(j=pad_w; j<out_width-pad_w; j++)
		{
			OutImg[(out_height-1-i)*out_width+j] = OutImg[(out_height-1-pad_h-pad_h+i)*out_width+j];
		}
	}
	for(i=0; i<out_height; i++)
	{
		for(j=0; j<pad_w; j++)
		{
			OutImg[i*out_width+j] = OutImg[i*out_width+pad_w+pad_w-j];
			OutImg[i*out_width+out_width-pad_w+j] = OutImg[i*out_width+out_width-1-pad_w-j];
		}
	}
}

void CSBD_Alg::_DeDoubleEnlarge(unsigned char *InImg, int in_width, int in_height, unsigned char *OutImg, int out_width, int out_height)
{
	int i, j;
	int cx, cy, fr_x, fr_y;
	int val, temp1, temp2;
	int pad_w = 32;
	int pad_h = 32;
	int width = in_width-pad_w*2;
	int height = in_height-pad_h*2;
	double fx, fy, nx, ny;
	double nWidthFactor = (double)width / (double)out_width;
    double nHeightFactor = (double)height / (double)out_height;

	for(i=0; i<out_height; i++)
	{
        fr_y = (int)floor(i * nHeightFactor);
        cy = fr_y + 1;
        if (cy >= height) cy = fr_y;
        fy = i * nHeightFactor - fr_y;
        ny = 1.0 - fy;

		fr_y = fr_y + pad_h;
		cy = cy + pad_h;

		for(j=0; j<out_width; j++)
		{
			fr_x = (int)floor(j * nWidthFactor);
			cx = fr_x + 1;
			if (cx >= width) cx = fr_x;
			fx = j * nWidthFactor - fr_x;
			nx = 1.0 - fx;

			fr_x = fr_x + pad_w;
			cx = cx + pad_w;

			temp1 = (unsigned char)(nx * InImg[fr_y*in_width+fr_x] + fx * InImg[fr_y*in_width+cx]);
			temp2 = (unsigned char)(nx * InImg[cy*in_width+fr_x] + fx * InImg[cy*in_width+cx]);
			val = (unsigned char)(ny * (double)(temp1) + fy * (double)(temp2));
			OutImg[i*out_width+j] = val;
		}
	}
}

void CSBD_Alg::_HistogramStretchPercent_222222_double_enlarge(unsigned char *InImg, int width, int height, unsigned char *OutImg)
{
	int i,j;
	int /*lowthresh, */highthresh=0;
	double scale_factor;
	unsigned char Contrast_LUT[256];
	int histogram[256];
	int histogram_whole[256];
	int maxposx, maxposy, maxmean;
	int sum, val;
	int mask_size = 101;					//// 곤아 여기
//	double Percent = 0.0;
//	int CutHistoValue = 160;

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;
	
	int tmpCIS_IMG_W = width;
	int tmpCIS_IMG_H = height;
	int tmpCIS_IMG_SIZE = tmpCIS_IMG_W*tmpCIS_IMG_H;

	dtLEFT = 32;
	dtRIGHT = width-32;
	dtTOP = 32;
	dtBOTTOM = height-32;

//	lowthresh = 0;

//	int noise=20,count;
	int y,x;
	sum=0;
//	count=0;
	memset(histogram,0,sizeof(int)*256);
	memset(histogram_whole,0,sizeof(int)*256);
	
	memset(OutImg, 0, tmpCIS_IMG_SIZE);

	int HIGH_B=m_TARGET_STRETCH_HISTOGRAM;//235;
//	int LOW_B=230;
//	int HIGH_RIDGE=50;
//	int LOW_RIDGE=30;

	
	_SearchingMaxGrayboxFast_double_enlarge(InImg, width, height, &maxposx, &maxposy, &maxmean, histogram, mask_size);	//// 곤아 여기

	sum=0;
//	count=0;
	int max=0/*,max_sat=0*/;
	float band=0.125;//0.25f;
	float max_band=0.006f;//0.012f;	//0.06

	sum=0;
	for(i=255; i>=0; i--)
	{
		sum+=histogram[i];

		if(sum > (mask_size*mask_size)*max_band)
		{
			max=i;
			break;
		}
	}

	for(y=100; y<tmpCIS_IMG_H-100; y++)
	{
		for(x=100; x<tmpCIS_IMG_W-100; x++)
		{
			val=InImg[y*CIS_IMG_W+x];
			histogram_whole[val]+=1;
		}
	}

//	int hmax=0;
	int MAX_BRIGHT=m_TARGET_STRETCH_HISTOGRAM;//235;		//Default 255
	sum=0;
	
	for(i=max+1; i<=255; i++)
	{
		if(histogram_whole[i]>10)
			sum++;		
	}
	if(sum > 0)
	{
		MAX_BRIGHT=HIGH_B;
	}

	

	sum=0;
	highthresh=0;
	for(i=255; i>=0; i--)
	{
		sum+=histogram[i];		
		if(sum > (mask_size*mask_size)*band)
		{
			highthresh = i;
			break;
		}
	}
	if(highthresh>3)									//BUG?
		highthresh=highthresh-3;
	else
	{
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			memcpy(&OutImg[i*tmpCIS_IMG_W+dtLEFT], &InImg[i*tmpCIS_IMG_W+dtLEFT], (dtRIGHT-dtLEFT+1));
		}
		return;
	}

	int targetbright;	
	
	targetbright= MAX_BRIGHT-(max-highthresh);
	scale_factor = (float) targetbright / highthresh;

	if(scale_factor<1)
	{
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			memcpy(&OutImg[i*tmpCIS_IMG_W+dtLEFT], &InImg[i*tmpCIS_IMG_W+dtLEFT], (dtRIGHT-dtLEFT+1));
		}
		return;
	}

	for(i=max+1; i<=255; i++)
	{
		val=MAX_BRIGHT+i-max;;
		if(val>255)
			val=255;
		
		Contrast_LUT[i]=val;
	}
	for(i=highthresh; i<=max; i++)
	{
		Contrast_LUT[i]=targetbright+i-highthresh;
	}

	for(i=0; i<highthresh; i++)
	{
		Contrast_LUT[i]=(unsigned char)(scale_factor*i);
	}

	for(i=dtTOP; i<dtBOTTOM; i++)
	{
		for(j=dtLEFT; j<dtRIGHT; j++)
		{
			OutImg[i*tmpCIS_IMG_W+j] = Contrast_LUT[InImg[i*tmpCIS_IMG_W+j]];
		}
	}
}

void CSBD_Alg::_SearchingMaxGrayboxFast_double_enlarge(unsigned char *InImg, int width, int height, int *MaxPosX, int *MaxPosY, int *MaxMean, int *MaxHistogram, int m_size)
{
    int y, x; //,j,i;
    int sum = 0; //,count;
	int max_gray = 0;
    int left, right, top, bottom;
	int xx, yy;
	
	int mask_size=m_size;
	int half_mask_size=mask_size/2;

	int DOUBLE_EN_IMG_W = width;
	int DOUBLE_EN_IMG_H = height;
	int DOUBLE_EN_IMG_SIZE = width*height;

	int LEFT = 0;//half_mask_size;
	int RIGHT = DOUBLE_EN_IMG_W;//half_mask_size-1;
	int TOP = 0;//half_mask_size;
	int BOTTOM = DOUBLE_EN_IMG_H;//half_mask_size-1;
    
	memset(m_SumGrayBuf, 0, DOUBLE_EN_IMG_SIZE * sizeof(int));
	m_SumGrayBuf[TOP*DOUBLE_EN_IMG_W+LEFT] = InImg[TOP*DOUBLE_EN_IMG_W+LEFT];

	for(xx=LEFT+1; xx<RIGHT; xx++)
		m_SumGrayBuf[TOP*DOUBLE_EN_IMG_W+xx] =
		m_SumGrayBuf[TOP*DOUBLE_EN_IMG_W+xx-1] + InImg[TOP*DOUBLE_EN_IMG_W+xx];

	for(yy=TOP+1; yy<BOTTOM; yy++)
		m_SumGrayBuf[yy*DOUBLE_EN_IMG_W+LEFT] =
		m_SumGrayBuf[(yy-1)*DOUBLE_EN_IMG_W+LEFT] + InImg[yy*DOUBLE_EN_IMG_W+LEFT];

	for(yy=TOP+1; yy<BOTTOM; yy++)
	{
		for(xx=LEFT+1; xx<RIGHT; xx++)
		{
			m_SumGrayBuf[yy*DOUBLE_EN_IMG_W+xx] = m_SumGrayBuf[yy*DOUBLE_EN_IMG_W+(xx-1)]
												+ m_SumGrayBuf[(yy-1)*DOUBLE_EN_IMG_W+xx]
												+ InImg[yy*DOUBLE_EN_IMG_W+xx]
												- m_SumGrayBuf[(yy-1)*DOUBLE_EN_IMG_W+(xx-1)];
		}
	}

	LEFT = 100;//half_mask_size;
	RIGHT = DOUBLE_EN_IMG_W-100;//half_mask_size-1;
	TOP = 100;//half_mask_size;
	BOTTOM = DOUBLE_EN_IMG_H-100;//half_mask_size-1;

	for(y=TOP+(half_mask_size+1); y<BOTTOM-half_mask_size; y++)
    {
        top = y - (half_mask_size+1);
        bottom = y + half_mask_size;

        for (x=LEFT+(half_mask_size+1); x<RIGHT-half_mask_size; x++)
        {
            left = x - (half_mask_size+1);
            right = x + half_mask_size;

            sum = (m_SumGrayBuf[bottom * DOUBLE_EN_IMG_W + right] - m_SumGrayBuf[top * DOUBLE_EN_IMG_W + right] - 
					m_SumGrayBuf[bottom * DOUBLE_EN_IMG_W + left] + m_SumGrayBuf[top * DOUBLE_EN_IMG_W + left]) / (mask_size*mask_size);

			if(sum>max_gray)
            {
                max_gray = sum;
				*MaxPosX = x;
				*MaxPosY = y;
            }
        }
    }

	*MaxMean = max_gray;// / (mask_size*mask_size);

	if((*MaxPosX)-half_mask_size < 0)
		*MaxPosX = half_mask_size;
	else if((*MaxPosX)+half_mask_size > DOUBLE_EN_IMG_W-2)
		*MaxPosX = DOUBLE_EN_IMG_W-2-half_mask_size;

	if((*MaxPosY)-half_mask_size < 0)
		*MaxPosY = half_mask_size;
	else if((*MaxPosY)+half_mask_size > DOUBLE_EN_IMG_H-2)
		*MaxPosY = DOUBLE_EN_IMG_H-2-half_mask_size;

	memset(MaxHistogram, 0, 1024);
	
	for(y=(*MaxPosY)-half_mask_size; y<=(*MaxPosY)+half_mask_size; y++)
	{
		for(x=(*MaxPosX)-half_mask_size; x<=(*MaxPosX)+half_mask_size; x++)
		{
			MaxHistogram[InImg[y*DOUBLE_EN_IMG_W+x]]++;
		}
	}
}

void CSBD_Alg::_Unsharp_opt_double_enlarge(unsigned char *InImg, int width, int height, unsigned char *OutImg)
{
	int i, j, xx, yy;
	int LEFT, RIGHT, TOP, BOTTOM;
	int left, right, top, bottom;
	int val;
	double unsharp_param = atof(m_strUnsharpParam);
	int length_x, length_y;

	memcpy(OutImg, InImg, width*height);

	LEFT = 0;
	RIGHT = width;
	TOP  = 0;
	BOTTOM = height;

	memset(m_SumGrayBuf, 0, width*height * 4);
	m_SumGrayBuf[TOP*width+LEFT] = InImg[TOP*width+LEFT];

	for(xx=LEFT+1; xx<RIGHT; xx++)
		m_SumGrayBuf[TOP*width+xx] =
		m_SumGrayBuf[TOP*width+xx-1] + InImg[TOP*width+xx];

	for(yy=TOP+1; yy<BOTTOM; yy++)
		m_SumGrayBuf[yy*width+LEFT] =
		m_SumGrayBuf[(yy-1)*width+LEFT] + InImg[yy*width+LEFT];

	for(yy=TOP+1; yy<BOTTOM; yy++)
	{
		for(xx=LEFT+1; xx<RIGHT; xx++)
		{
			m_SumGrayBuf[yy*width+xx] = m_SumGrayBuf[yy*width+(xx-1)]
												+ m_SumGrayBuf[(yy-1)*width+xx]
												+ InImg[yy*width+xx]
												- m_SumGrayBuf[(yy-1)*width+(xx-1)];
		}
	}

	//LEFT = 6;
	//RIGHT = width-6;
	//TOP  = 6;
	//BOTTOM = height-6;

	for (i=TOP; i<BOTTOM; i++)
	{
        top = i - (4+1);
        bottom = i + 4;

		if(top < 0) top = 0;
		if(bottom > height-1) bottom = height-1;

		length_y = (bottom-top);

		for (j=LEFT; j<RIGHT; j++)
		{
			// to make fast
			if (InImg[i * width + j] == 0)
				continue;

			left = j - (4+1);
            right = j + 4;

			if(left < 0) left = 0;
			if(right > width-1) right = width-1;

			length_x = (right-left);

			val = (int)(InImg[i * width + j] - 
				(m_SumGrayBuf[bottom * width + right] - m_SumGrayBuf[top * width + right] - 
				m_SumGrayBuf[bottom * width + left] + m_SumGrayBuf[top * width + left]) / (length_x*length_y));

			if (val < 0)
			{
				val = (int)(val * (1.0 - InImg[i * width + j] / (255.0)) * unsharp_param);
				val = (int)InImg[i * width + j] + val;

				if (val < 0)
					val = 0;

				OutImg[i * width + j] = (unsigned char)val;
			}
        }
    }
}

