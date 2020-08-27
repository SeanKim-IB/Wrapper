#pragma once

#include "IBSU_Global_Def.h"

#define MINUS_MASK_ORIGINAL		0x01
#define MINUS_MASK_MEAN			0x02

class CSBD_Alg
{
public:
	UsbDeviceInfo	*m_pUsbDevInfo;
	PropertyInfo	*m_pPropertyInfo;

	CSBD_Alg(UsbDeviceInfo *pUsbDevInfo, PropertyInfo *pPropertyInfo);
	virtual ~CSBD_Alg();
	int SaveBitmapImage(char *filename, unsigned char *buffer, int width, int height);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SBD
	void _Init_SBD_final(unsigned char *InImg);

	// final image
	void _Apply_SBD_final(unsigned char *InImg, unsigned char *OutImg);
	void _Apply_SBD_final_opt(unsigned char *InImg, unsigned char *OutImg);
	void _Make500DPI_opt(unsigned char *InImg, unsigned char *OutImg);
	
	void _Apply_SBD_final_Five0(unsigned char *InImg, unsigned char *OutImg);
	void _Apply_SBD_final_Five0_double_enlarge(unsigned char *InImg, unsigned char *OutImg);

	// preview image
	void _Apply_SBD_preview(unsigned char *InImg, unsigned char *OutImg);
	void _Make500DPI_preview_opt(unsigned char *InImg, unsigned char *OutImg);

	void _VignettingEffect(short *InImg, short *OutImg, int width, int height);
	void _VignettingEffect_select_80_mean(short *InImg, short *OutImg);
	void _VignettingEffect_select_80_mean_opt(unsigned char *InImg, short *OutImg);

	void _HistogramStretchUsingLUT_int(short *InImg, short *OutImg);
	void _HistogramStretchUsingLUT_int_opt(short *InImg, short *OutImg);
	void _RemoveSpecificNoise_int(short *InImg, short *OutImg);
	void _Unsharp_opt(unsigned char *InImg, unsigned char *OutImg);
	void _StdDevSmoothing_int_opt(short *InImg, short *OutImg);
	void _RemoveSpotNoise(unsigned char *InImg, unsigned char *OutImg);
	void _HistogramStretchPercent(unsigned char *InImg, unsigned char *OutImg);
	void _HistogramStretchPercent_222222(unsigned char *InImg, unsigned char *OutImg);
	void _HistogramStretchPercent_222222_debug(unsigned char *InImg, unsigned char *OutImg);
	void _HistogramStretchPercent_333333(unsigned char *InImg, unsigned char *OutImg);
	void _HistogramStretchPercent_gon_final(unsigned char *InImg, unsigned char *OutImg);
	void _SearchingMaxGrayboxFast(unsigned char *InImg, int *MaxPosX, int *MaxPosY, int *MaxMean, int *MaxHistogram, int m_size);
	void _SearchingMaxGrayboxFast_int(short *InImg, int *MaxPosX, int *MaxPosY, int *MaxMean, int *MaxHistogram, int m_size);

	void _Init_Distortion_Merge_with_Bilinear(float DISTORTION_A, float DISTORTION_B, float DISTORTION_C, float DISTORTION_D, 
																	   int cut_x, int cut_y, int shift_x, int shift_y);
	void Init_Distortion_Merge_with_Bilinear_forDaiwon(int cutx, int cuty, int shift_x, int shift_y);
	void _DistortionRestoration_with_Bilinear(unsigned char *InImg, unsigned char *OutImg);
	void _DistortionRestoration_with_Cubic(unsigned char *Image, unsigned char *output);
	void _HistogramFilling(unsigned char *InImg, unsigned char *OutImg, int width, int height);

	void _DoubleEnlarge(unsigned char *InImg, int in_width, int in_height, unsigned char *OutImg, int out_width, int out_height);
	void _DoubleEnlarge_int(short *InImg, int in_width, int in_height, short *OutImg, int out_width, int out_height);
	void _DeDoubleEnlarge(unsigned char *InImg, int in_width, int in_height, unsigned char *OutImg, int out_width, int out_height);
	void _RemoveSpecificNoise_int_double_enlarge(short *InImg, short *OutImg, int width, int height);
	void _StdDevSmoothing_int_opt_double_enlarge(short *InImg, short *OutImg, int width, int height);
	void _Unsharp_opt_double_enlarge(unsigned char *InImg, int width, int height, unsigned char *OutImg);
	void _HistogramStretchPercent_222222_double_enlarge(unsigned char *InImg, int width, int height, unsigned char *OutImg);
	void _SearchingMaxGrayboxFast_double_enlarge(unsigned char *InImg, int width, int height, int *MaxPosX, int *MaxPosY, int *MaxMean, int *MaxHistogram, int m_size);
	int SaveBitmapImage_int(char *filename, short *buffer, int width, int height);

public:
	// for preview
	void _VignettingEffect_preview(unsigned char *InImg, unsigned char *OutImg, int cis_noise);
	void _VignettingEffect_preview_roll(unsigned char *InImg, unsigned char *OutImg, int cis_noise);
	void _HistogramStretchPercent_preview(unsigned char *InImg, unsigned char *OutImg, int width, int height);
	void _HistogramStretchPercent_222222_preview(unsigned char *InImg, unsigned char *OutImg);
	void _HistogramStretchPercent_222222_preview_roll(unsigned char *InImg, unsigned char *OutImg);
	void _SearchingMaxGrayboxFast_preview(unsigned char *InImg, int width, int height, int *MaxPosX, int *MaxPosY, int *MaxMean, int *MaxHistogram, int m_size);
	void _DistortionRestoration_with_Bilinear_preview(unsigned char *InImg, unsigned char *OutImg);
	void _MakeMosaicImage(unsigned char *InImg);
	void _MakeCleanImage(unsigned char *InImg);

	int _GetBackNoiseValue(int x, int y);
	int _GetBackNoiseValue_forRoll(int x, int y);

	BYTE	*m_OriginalImg;
	BYTE	*m_ResultImg;
	BYTE	*m_BestRollImg;
//	BYTE	*m_ResultImg_withMinus;
//	BYTE	*m_ResultImg500;
//	BYTE	*m_ResultImg500_withMinus;
//	BYTE	*m_FirstFrameImg;
//	BYTE	*tmpResult;
//	BYTE	*tmpResult2;
	BYTE	*m_segment_arr_background_substract;
	BYTE	*m_segment_arr_background_substract_tmp;

	// variables
	char m_strMinCutThres[16];
	char m_strNoiseAmount[16];
	char m_strBrightWeightRatio[16];
	char m_strStretchPercent[16];
	char m_strUnsharpParam[16];
	UINT m_MinusMaskType;
	int	m_Roll_Minus_Val;
	FrameImgAnalysis	*m_pcImgAnalysis;
	unsigned char m_StretchPreviewLUT[256];

	int m_GoMean;
	int m_Percent_of_255;
	int	m_SegmentBright;
	int	m_SegmentBrightCnt;
	int m_CMOS_NOISE_AMOUNT;
//	int m_nSaveCount;
	int m_TARGET_STRETCH_HISTOGRAM;
	int m_nContrastTres;

	int		CIS_IMG_W;
	int		CIS_IMG_H;
	int		CIS_IMG_SIZE;
	int		IMG_W;
	int		IMG_H;
	int		IMG_SIZE;
	int		IMG_W_ROLL;
	int		IMG_H_ROLL;
	int		IMG_SIZE_ROLL;
	int		CIS_IMG_W_ROLL;
	int		CIS_IMG_H_ROLL;
	int		CIS_IMG_SIZE_ROLL;
	
	int		ZOOM_OUT;
	int		ZOOM_H;
	int		ZOOM_W;
	int		ENLARGESIZE_ZOOM_H;
	int		ENLARGESIZE_ZOOM_W;
	int		ZOOM_ENLAGE_H;
	int		ZOOM_ENLAGE_W;

	int		ZOOM_OUT_ROLL;
	int		ZOOM_H_ROLL;
	int		ZOOM_W_ROLL;
	int		ENLARGESIZE_ZOOM_H_ROLL;
	int		ENLARGESIZE_ZOOM_W_ROLL;
	int		ZOOM_ENLAGE_H_ROLL;
	int		ZOOM_ENLAGE_W_ROLL;

	DWORD	*arrPos_wb;
	DWORD	*arrPos_wb_KOJAK_ROLL;

	short		*m_OriginalImg_int;
	short		*m_ResultImg_int;
//	int		*m_ResultImg2_int;
	int		*m_Stretch_LUT;
	float	*diff_image_withMinus;
	float	*diff_image_withMinusMean;
	short		*diff_image_withMinusMean_preview;
//	float	*diff_image;
//	float	*final_table;
	int		*m_SumGrayBuf;
	int		*m_SumGrayBuf2;
	float	*weight_table_bright;
	float	*weight_table_diff;
//	double	*weight_table_total;
	float	Cubic_UV[5][129];
};


