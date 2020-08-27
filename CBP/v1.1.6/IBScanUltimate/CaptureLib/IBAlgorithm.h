/////////////////////////////////////////////////////////////////////////////
/*
DESCRIPTION:
	CIBAlgorithm - Class for image processing algorithm
	http://www.integratedbiometrics.com/

NOTES:
	Copyright(C) Integrated Biometrics, 2012

VERSION HISTORY:
	03 July 2012 - First initialize
*/
/////////////////////////////////////////////////////////////////////////////

#ifndef __IB_ALGORITHM_H__
#define __IB_ALGORITHM_H__

#include "IBSU_Global_Def.h"
#include "IBAlgorithm_SBD.h"
#include "IBEncryption.h"
#include <vector>
/////////////////////////////////////////////////////////////////////////////

//const unsigned char AES_KEY_ENCRYPTION[32] =	{'0','1','2','3','4','5','6','7','8','9',
//												 '0','1','2','3','4','5','6','7','8','9',
//												 '0','1','2','3','4','5','6','7','8','9',
//												 '0','1'};
const unsigned char AES_KEY_ENCRYPTION[256] =	{0x30,	0x31,	0x32,	0x33,	0x34,	0x35,	0x36,	0x37,	0x38,	0x39,	0x30,	0x31,	0x32,	0x33,	0x34,	0x35,
												0x36,	0x37,	0x38,	0x39,	0x30,	0x31,	0x32,	0x33,	0x34,	0x35,	0x36,	0x37,	0x38,	0x39,	0x30,	0x31,
												0x23,	0x35,	0xf5,	0x34,	0x17,	0x00,	0xc3,	0x03,	0x2f,	0x39,	0xf3,	0x32,	0x1d,	0x0a,	0xc7,	0x07,
												0x92,	0x50,	0xfe,	0xfc,	0xa2,	0x61,	0xcc,	0xcf,	0x96,	0x54,	0xfa,	0xf8,	0xae,	0x6d,	0xca,	0xc9,
												0x1d,	0x41,	0x28,	0xd0,	0x0a,	0x41,	0xeb,	0xd3,	0x25,	0x78,	0x18,	0xe1,	0x38,	0x72,	0xdf,	0xe6,
												0x95,	0x10,	0x60,	0x72,	0x37,	0x71,	0xac,	0xbd,	0xa1,	0x25,	0x56,	0x45,	0x0f,	0x48,	0x9c,	0x8c,
												0x4b,	0x9f,	0x4c,	0xa6,	0x41,	0xde,	0xa7,	0x75,	0x64,	0xa6,	0xbf,	0x94,	0x5c,	0xd4,	0x60,	0x72,
												0xdf,	0x58,	0xb0,	0x32,	0xe8,	0x29,	0x1c,	0x8f,	0x49,	0x0c,	0x4a,	0xca,	0x46,	0x44,	0xd6,	0x46,
												0x58,	0x69,	0x16,	0xfc,	0x19,	0xb7,	0xb1,	0x89,	0x7d,	0x11,	0x0e,	0x1d,	0x21,	0xc5,	0x6e,	0x6f,
												0x22,	0xfe,	0x2f,	0x9a,	0xca,	0xd7,	0x33,	0x15,	0x83,	0xdb,	0x79,	0xdf,	0xc5,	0x9f,	0xaf,	0x99,
												0x93,	0x10,	0xf8,	0x5a,	0x8a,	0xa7,	0x49,	0xd3,	0xf7,	0xb6,	0x47,	0xce,	0xd6,	0x73,	0x29,	0xa1,
												0xd4,	0x71,	0x8a,	0xa8,	0x1e,	0xa6,	0xb9,	0xbd,	0x9d,	0x7d,	0xc0,	0x62,	0x58,	0xe2,	0x6f,	0xfb,
												0x2b,	0xb8,	0xf7,	0x30,	0xa1,	0x1f,	0xbe,	0xe3,	0x56,	0xa9,	0xf9,	0x2d,	0x80,	0xda,	0xd0,	0x8c,
												0x19,	0x26,	0xfa,	0xcc,	0x07,	0x80,	0x43,	0x71,	0x9a,	0xfd,	0x83,	0x13,	0xc2,	0x1f,	0xec,	0xe8,
												0xab,	0x76,	0x6c,	0x15,	0x0a,	0x69,	0xd2,	0xf6,	0x5c,	0xc0,	0x2b,	0xdb,	0xdc,	0x1a,	0xfb,	0x57,
												0x9f,	0x84,	0xf5,	0x97,	0x98,	0x04,	0xb6,	0xe6,	0x02,	0xf9,	0x35,	0xf5,	0xc0,	0xe6,	0xd9,	0x1d};

class CIBAlgorithm
{
public:	// Construction/destruction:

	CIBAlgorithm(UsbDeviceInfo *pUsbDevInfo, PropertyInfo *pPropertyInfo);
	virtual			~CIBAlgorithm(void);

public:	// Implementation:

	CSBD_Alg		*m_SBDAlg;
	int				m_nframeCount;
	CIBEncryption	*m_AESEnc;

	UsbDeviceInfo	*m_pUsbDevInfo;
	PropertyInfo	*m_pPropertyInfo;
	int				SaveBitmapImage(char *filename, unsigned char *buffer, int width, int height, int invert);
	void			_Algo_MakeUniformityMask(BYTE *WaterImage);
	void			_Algo_RemoveNoise(unsigned char *InImg, unsigned char *OutImg, int ImgSize, int ContrastLevel);
	void			_Algo_HistogramStretchForZoomEnlarge(unsigned char *ImageBuf);
	void			_Algo_HistogramStretchForZoomEnlarge_Kojak_Roll(unsigned char *ImageBuf);
	int				_Algo_GetBrightWithRawImage_forDetectOnly(BYTE *InImg, int ForgraoundCNT);
	int				_Algo_GetBrightWithRawImage(BYTE *InImg, int *ForgraoundCNT, int *CenterX, int *CenterY);
	int				_Algo_GetBrightWithRawImageEnhanced(BYTE *InRawImg, FrameImgAnalysis *imgAnalysis);
	int				_Algo_GetFingerCount(int ImageBright);
	int				_Algo_GetForegroundInfo2(SEGMENT_ARRAY *segment_arr, BYTE *backImage, const int imgWidth, const int imgHeight);
	int				_Algo_RemoveFalseForeground2(SEGMENT_ARRAY *segment_arr, BYTE *backImage, const int imgWidth, const int imgHeight);
	void			_Algo_AdjustForegroundDirection2(SEGMENT *segment, BYTE *backImage, const int imgWidth, const int imgHeight);
	int				_Algo_StackRecursiveFilling_ZoomEnlarge(unsigned char *image, int x, int y, unsigned char TargetColor, unsigned char DestColor);
	int				_Algo_ATAN2_FULL(int y, int x);
	void			_Algo_GetFixedDistortionPos(int px, int py, int *new_px, int *new_py);

	void			_Algo_Init_GammaTable();


	void			_Algo_Init_Distortion_Merge_with_Bilinear();
	void			_Algo_DistortionRestoration_with_Bilinear(unsigned char *InImg, unsigned char *OutImg);
	void			_Algo_VignettingEffect(unsigned char *InImg, unsigned char *OutImg);
	void			_Algo_VignettingEffectEnhanced(unsigned char *InImg, unsigned char *OutImg, FrameImgAnalysis imgAnalysis);
	void			_Algo_RemoveVignettingNoise(unsigned char *InImg, unsigned char *OutImg, unsigned char *TmpImg);
	void			_Algo_RemoveVignettingNoiseEnhanced(unsigned char *InImg, unsigned char *OutImg, unsigned char *TmpImg, FrameImgAnalysis imgAnalysis);

	int				_Algo_SearchingMaxGraybox_Fast(unsigned char *image, int imgWidth, int imgHeight, int rectTop, int rectBottom, int rectLeft, int rectRight);
	void			_Algo_Genertate_Sum_Gray(unsigned char *image, int imgWidth, int imgHeight, int rectTop, int rectBottom, int rectLeft, int rectRight);
	void			_Algo_Image_Gamma(unsigned char *InImg, unsigned char *OutImg, int Contrast);
	void			_Algo_Image_GammaEnhanced(unsigned char *InImg, unsigned char *OutImg, int Contrast, FrameImgAnalysis imgAnalysis);
	void			_Algo_Image_Smoothing(unsigned char *InImg, unsigned char *OutImg);
	void			_Algo_SwUniformity(unsigned char *InImg, unsigned char *OutImg, unsigned char *TmpOrigin, int width, int height);
	void			_Algo_SwUniformityEnhanced(unsigned char *InImg, unsigned char *OutImg, unsigned char *TmpOrigin, int width, int height, FrameImgAnalysis imgAnalysis);
	void			_Algo_HistogramNormalize(unsigned char *InImg, unsigned char *OutImg, int imgSize, int contrastTres);
	void			_Algo_HistogramNormalizeEnhanced(unsigned char *InImg, unsigned char *OutImg, int imgSize, int contrastTres, FrameImgAnalysis imgAnalysis);
	int				_Algo_GetBright_Simple(unsigned char *InImg, int imgSize, int cutTres);
	int				_Algo_FindExpectContrast(unsigned char *InImg, unsigned char *TmpImg, int imgSize, int contrastTres, int nGammaLevel);
	int				_Algo_FindExpectContrastEnhanced(unsigned char *InImg, unsigned char *TmpImg, int imgSize, int contrastTres, int nGammaLevel, FrameImgAnalysis imgAnalysis);
	void			_Algo_ZoomOut_with_Bilinear(unsigned char *InImg, int imgWidth, int imgHeight, unsigned char *OutImg, int outWidth, int outHeight);
	void			_Algo_JudgeFingerQuality(FrameImgAnalysis *pFrameImgAnalysis, BOOL *bIsGoodImage, PropertyInfo *pPropertyInfo, CallbackProperty *pClbkProperty, BOOL dispInvalidArea); //Modify Sean to check finger in invalid area
	void			_Algo_Sharpening(const BYTE *Input, const int imgWidth, const int imgHeight, BYTE *Output);
	void			_Algo_Smoothing(const BYTE *Input, const int imgWidth, const int imgHeight, BYTE *Output);
	void			_Algo_StackRecursiveFilling(BYTE *backImage, const int imgWidth, const int imgHeight, 
												int x, int y, BYTE TargetColor, BYTE DestColor);
	void			_Algo_ConvertBkColorBlacktoWhite(BYTE *InImg, BYTE *OutImg, int imgSize);
	
#if defined(__IBSCAN_SDK__)
	void			_Algo_Image_Gamma_250DPI(unsigned char *InImg, unsigned char *OutImg, int Contrast);	//	new add
	void			_Algo_Make_250DPI(unsigned char *InImg, unsigned char *OutImg);
	void			_Algo_MakeWatson_250DPI(unsigned char *InImg, unsigned char *OutImg);
#endif

	// new rolling
	int				_PostImageProcessing_ForRolling(unsigned char *InImg, int frameCount, BOOL *isComplete);
	void			NewRoll_Init_Rolling();
	int				NewRoll_Stitch_TwoImage(unsigned char *MergeImg, unsigned char *InputImg, unsigned char *TmpImg, int MergePosX, int StitchDir, int SmoothingRange);
	int				NewRoll_MergeImage(unsigned char *MergeImg, unsigned char *InputImg, unsigned char *BestMergeImg);
	int				NewRoll_GetFingerPosition(unsigned char *InImg, int *PosX, int *PosY);
	int				NewRoll_CheckRollStartCondition(unsigned char *InImg);
	int				NewRoll_Brightness_Control(unsigned char *InputImg, unsigned char *OutImg, unsigned char target_brightness, int MergePosX);
	int				NewRoll_GetBright_Simple(const BYTE *InputImage, int MergePosX);
	void			NewRoll_GetWidth(unsigned char *MergeImg, int *Width);
	void			_Algo_Image_Gamma_Simple(unsigned char *InImg, unsigned char *OutImg, int Contrast, int MergePosX);

	void			_Algo_Process_AnalysisImage(unsigned char *InImg);


	void			_Algo_Fast_Distortion(unsigned char *InImg, unsigned char *OutImg);
	void			_Algo_Fast_Distortion_Columbo(unsigned char *InImg, unsigned char *OutImg);
	void			_Algo_Fast_Distortion_250DPI_Columbo(unsigned char *InImg, unsigned char *OutImg);

	void			_Algo_Encrypt_Simple(unsigned char *InBuf, unsigned char *OutBuf, size_t buf_size);
	void			_Algo_Decrypt_Simple(unsigned char *InBuf, unsigned char *OutBuf, size_t buf_size);
	void			_Algo_MakeChecksum(DWORD *InBuf, size_t buf_size);
	BOOL			_Algo_ConfirmChecksum(DWORD *InBuf, size_t buf_size);
	
	int				_Algo_GetNFIQScore(const unsigned char *idata, const int imgWidth, const int imgHeight, const int imgPixelDepth, int *onfiq);
	void			_Algo_RemoveTFTDefect(unsigned char *InImg, unsigned char *OutImage, unsigned char *Defect_MaskImg);
	BOOL			_Algo_ManualCapture(int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime);


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// For Watson Algorithm
	BOOL			_Algo_Watson_AutoCaptureByGain(BYTE *InImg, int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime);
	BOOL			_Algo_Watson_AutoCaptureByVoltage(BYTE *InImg, int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime);
	BOOL			_Algo_Watson_AutoCaptureByExposure(BYTE *InImg, int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime);
	void			_Algo_Watson_GetDistortionMask_FromSavedFile();
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// For Curve Algorithm
	unsigned char	*m_PinholeMask;
	void			_Algo_Curve_AnalysisImage(unsigned char *InImg, GoodCaptureInfo *capinfo);
	void			_Algo_Curve_CuttingDumyArea_with_Noise(unsigned char *InImg, unsigned char *OutImg, unsigned char *TmpImg, int ImgSize, int ContrastLevel, int *center_x, int *center_y);
	int				_Algo_Curve_GetFingerCount(unsigned char *InImg, int ImageBright);
	int			    _Algo_Curve_GoodCapture(GoodCaptureInfo *capinfo, PropertyInfo *propertyInfo, BOOL *SetRegisterForDry);
	void			_Algo_Curve_JudgeFingerQuality(FrameImgAnalysis *pFrameImgAnalysis, BOOL bIsGoodImage, PropertyInfo *pPropertyInfo, CallbackProperty *pClbkProperty);
	void			_Algo_Curve_Init_Pinhole();
	BOOL			_Algo_Curve_DetectPinhole_using_Ratio(const unsigned char *InImg);
	int				_Algo_Curve_GetBrightWithRawImage(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY);
	void			_ALGO_Curve_HistogramEnhance(unsigned char *pInputImg, unsigned char *pOutputImg);
	void			_ALGO_Curve_Vignetting(unsigned char *pInputImg, unsigned char *pOutputImg);
	void			_ALGO_Curve_SWUniformity(unsigned char *pInputImg, unsigned char *pOutputImg);

	void			_Algo_Curve_OrgImageEnhance(unsigned char *pInputImg, unsigned char *pOutputImg);


	void			EnlargeImage(unsigned char* input_image,unsigned char output_image[ENLARGE_IMG_H][ENLARGE_IMG_W]);
	void			MedianFilter(unsigned char image_buffer[ENLARGE_IMG_H][ENLARGE_IMG_W], unsigned char temp_buffer[ENLARGE_IMG_H][ENLARGE_IMG_W]);
	void			HistogramStretch(unsigned char image_buffer[ENLARGE_IMG_H][ENLARGE_IMG_W]);
	void			StackFillOutside(unsigned char (*image)[ENLARGE_QUARTER_W], int x, int y, unsigned char fromValue, unsigned char toValue);
	void			BackgroundCheck(unsigned char image_buffer[ENLARGE_IMG_H][ENLARGE_IMG_W], unsigned char quabuf[ENLARGE_QUARTER_H][ENLARGE_QUARTER_W],
												int current_histo, int noise_histo, unsigned char *temp_buffer);
	void			Reverse_Enlarge(unsigned char Enlarge_buffer[ENLARGE_IMG_H][ENLARGE_IMG_W], unsigned char* image_buffer);
	void			Back_Ground_display(unsigned char Backgound_buffer[ENLARGE_QUARTER_H][ENLARGE_QUARTER_W], unsigned char Enlarge_buffer[ENLARGE_IMG_H][ENLARGE_IMG_W]);
	void			_Algo_Curve_Init_Distortion_Merge_with_Bilinear();
	void			_Algo_Curve_DistortionRestoration_with_Bilinear(unsigned char *InImg, unsigned char *OutImg);

//////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// For Sherlock Algorithm
	void			_Algo_ROIC_RemoveNoise(unsigned char *InImg, unsigned char *OutImg, int ImgSize, int ContrastLevel);
	int				_Algo_ROIC_GetBrightWithRawImage(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY);
	int				_Algo_ROIC_GetBrightWithRawImageEnhanced(BYTE *InRawImg, FrameImgAnalysis *imgAnalysis);
	void			_Algo_Remove_Noise_using_remove_mask(BYTE *InputImage, BYTE *OutImage, int imgWidth, int imgHeight);
	void			_Algo_Remove_Noise_using_remove_mask_asic(BYTE *InputImage, BYTE *OutImage, int imgWidth, int imgHeight);
	void			_Algo_Remove_Dot_Noise_asic(BYTE *InputImage, BYTE *OutImage, int imgWidth, int imgHeight);
	void			_Algo_Remove_Vertical_Noise(BYTE *InputImage, BYTE *OutImage, int imgWidth, int imgHeight);
	BOOL			_Algo_Sherlock_AutoCapture(BYTE *InImg, int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime);
	BOOL			_Algo_Sherlock_ManualCapture(BYTE *InImg, int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime);
	void			_Algo_ROIC_AnalysisImage(unsigned char *InImg);
	void			_Algo_ASIC_AnalysisImage(unsigned char *InImg);
	void			_Algo_ROIC_RemoveVignettingNoise(unsigned char *InImg, unsigned char *OutImg);
//////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// For Columbo Algorithm
	BOOL			_Algo_Columbo_AutoCapture(BYTE *InImg, int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime);
	BOOL			_Algo_Columbo_AutoCaptureByExposure(BYTE *InImg, int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime);
	void			_Algo_Columbo_Init_Distortion_Merge_with_Bilinear();
	void			_Algo_Columbo_DistortionRestoration_with_Bilinear(unsigned char *InImg, unsigned char *OutImg);
	int				_Algo_Columbo_GetBrightWithRawImage(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY);
	int				_Algo_Columbo_GetBrightWithRawImageEnhanced(BYTE *InRawImg, FrameImgAnalysis *imgAnalysis);
	void			_Algo_Columbo_GetDistortionMask_FromSavedFile();
	int				_Algo_Columbo_GetBrightWithRawImage_250DPI_Columbo(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY);
	void			_Algo_Process_AnalysisImage_250DPI_Columbo(unsigned char *InImg);
	void			_Algo_RemoveNoise_250DPI_Columbo(unsigned char *InImg, unsigned char *OutImg, int ContrastLevel);
	void			_Algo_VignettingEffect_250DPI_Columbo(unsigned char *InImg, unsigned char *OutImg);
	void			_Algo_RemoveVignettingNoise_250DPI_Columbo(unsigned char *InImg, unsigned char *OutImg, unsigned char *TmpImg);
	void			_Algo_Image_Gamma_250DPI_Columbo(unsigned char *InImg, unsigned char *OutImg, int Contrast);
	void			_Algo_Columbo_DistortionRestoration_with_Bilinear_250DPI_Columbo(unsigned char *InImg, unsigned char *OutImg);
//////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// For Curve-SETi Algorithm
	int				_Algo_CurveSETi_GoodCapture(GoodCaptureInfo *capinfo, PropertyInfo *propertyInfo, BOOL *SetRegisterForDry);
	void			_Algo_CurveSETi_Init_Distortion_Merge_with_Biliner();
	void			_Algo_CurveSETi_DistortionRestoration_with_Bilinear(unsigned char *InImg, unsigned char *OutImg);
	void			_Algo_CurveSETi_AnalysisImage(unsigned char *InImg, GoodCaptureInfo *capinfo);
	void			_Algo_CurveSETi_AnalysisImage_for_Dummy(unsigned char *InImg, GoodCaptureInfo *capinfo);
	void			_Algo_CurveSETi_CuttingDumyArea_with_Noise(unsigned char *InImg, unsigned char *OutImg, unsigned char *TmpImg, int ImgSize, int ContrastLevel, int *center_x, int *center_y);
	int				_Algo_CurveSETi_GetFingerCount(unsigned char *InImg, int ImageBright);
	void			_Algo_CurveSETi_JudgeFingerQuality(FrameImgAnalysis *pFrameImgAnalysis, BOOL bIsGoodImage, PropertyInfo *pPropertyInfo, CallbackProperty *pClbkProperty);
//////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// For Kojak Algorithm
	BOOL			_Algo_Kojak_AutoCapture(CaptureSetting *pSetting, int frameTime, int ImgSize, int *CaptureGood);
	BOOL			_Algo_Kojak_AutoCapture_CurrentSensor(CaptureSetting *pSetting, int frameTime, int ImgSize, int *CaptureGood);
	void			_Algo_Kojak_GetDistortionMask_FromSavedFile();
	int				_Algo_Kojak_GetBrightWithRawImage(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY, int *Bright);
	int				_Algo_Kojak_GetBrightWithRawImage_forDetectOnly(BYTE *InRawImg, int ForgraoundCNT);
	int				_Algo_Kojak_GetBrightWithRawImage_Final(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY, int *Bright);
	int				_Algo_Kojak_GetBrightWithRawImage_Roll(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY, int *Bright);
	int				_Algo_Kojak_GetBrightWithRawImage_forDetectOnly_Roll(BYTE *InRawImg, int ForgraoundCNT);
	void			_Algo_Kojak_Init_Distortion_Merge_with_Bilinear();
	void			_Algo_Kojak_DistortionRestoration_with_Bilinear(unsigned char *InImg, unsigned char *OutImg);
	void			_Algo_Process_AnalysisImage_Kojak(unsigned char *InImg);
	void			_Algo_RemoveNoise_Kojak(unsigned char *InImg, unsigned char *OutImg, int ImgSize, int ContrastLevel);
	int				_Algo_JudgeMeanOr80Mask(unsigned char *InImg, int ImgSize);
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	void			_ALGO_MedianFilter(unsigned char *input_image, int imgWidth, int imgHeight, int *histogram);
	void			_ALGO_HistogramStretch(unsigned char *pImage, int imgWidth, int imgHeight);
	void			_ALGO_HistogramStretch_Roll(unsigned char *pImage, int imgWidth, int imgHeight, int MergePosX);
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// For FAP50 Algorithm
	BOOL			_Algo_Five0_AutoCapture(BYTE *InImg, CaptureSetting *pSetting, int frameTime, int ImgSize, int *CaptureGood);
	BOOL			_Algo_Five0_AutoCapture_TOF(BYTE *InImg, CaptureSetting *pSetting, int frameTime, int ImgSize, int *CaptureGood, int m_nAnalogTouch_Plate);
	int				_Algo_Five0_GetBrightWithRawImage_forDetectOnly(BYTE *InRawImg, int ForgraoundCNT);
	int				_Algo_Five0_GetBrightWithRawImage(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY, int *Bright, int *BrightPixelCount);
	int				_Algo_Five0_GetBrightWithRawImage_forDetectOnly_Roll(BYTE *InRawImg, int ForgraoundCNT);
	int				_Algo_Five0_GetBrightWithRawImage_Roll(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY, int *Bright);
	void			_Algo_Five0_AnalysisImage(unsigned char *InImg, unsigned char *OutImg, int ImgSize, int ContrastLevel);
	void			_Algo_Five0_RemoveAnalogTouchSensorNoise(unsigned char *InImg, unsigned char *OutImg, int ImgWidth, int ImgHeight, int ContrastLevel);
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	int				_Algo_GetBackgroundNoiseAuto(unsigned char *InImg, unsigned char *SegmentImg);
	void			_Algo_ConvertSegInfoToFinal(unsigned char *pNewSeg, unsigned char *pNewEnlarge);

	void			_Algo_Erosion(unsigned char *pNewSeg, int width, int height);
	void			_Algo_Dilation(unsigned char *pNewSeg, int width, int height);
	int				_Algo_CheckAppendixFQuality(unsigned char *InImgArr, int *WidthArr, int *HeightArr, int SegmentCnt);
	int				_Algo_AnalysisHistogram(unsigned char *InImg, int Width, int Height, int index);
	int				_Algo_CheckWetFinger(unsigned char *InImgArr, int *WidthArr, int *HeightArr, int SegmentCnt);
	int				_Algo_AnalysisBlockStd_forWet(unsigned char *InImg, int Width, int Height, int index, int *WetRatio);
	int				_Algo_AnalysisBlockStd_forSmear(unsigned char *InImg, int Width, int Height, int index, int *WetRatio);
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// For Smear Algorithm
	int SmearDetection_Calibration(unsigned char *InImg);
	int SmearDetectionOfFinger(unsigned char *LeftImg, unsigned char *RightImg, int MergePosX);
//	int SmearDetectionOfFinger_Watson(unsigned char *LeftImg, unsigned char *RightImg, int MergePosX);
//	int SmearDetectionOfFinger_Sherlock(unsigned char *LeftImg, unsigned char *RightImg, int MergePosX);
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// For new segmentation
	int				_Algo_SegmentFinger(unsigned char *ImgData);
	int				_Algo_SegmentationImage(unsigned char *ImgData);
	int				_Algo_GetSegmentInfo(NEW_SEGMENT_ARRAY *segment_arr, int ImageBright);
	void			_Algo_GetMainDirectionOfFinger();
    void            _Algo_SwapNewSegment(NEW_SEGMENT *src1, NEW_SEGMENT *src2);
	void			_Algo_SortSegmentByMainDir();
	void			_Algo_MergeSegmentIfSameFinger();
	void			_Algo_SearchFingerTipPosition();
	void			_Algo_SearchFingerTipPosition_90();
	void			_Algo_HistogramNormalizeForZoomEnlarge(unsigned char *ImageBuf);
	void			_Algo_ClipFinger(BYTE *InputImg, BYTE *OutputImage, int *OutputWidth, int *OutputHeight, int *OutputSize, BYTE *LabelBuf, NEW_SEGMENT *segment);

	// new segmentation for Matcher
	int				_Algo_SegmentFinger_Matcher(unsigned char *ImgData);
	void			_Algo_GetMainDirectionOfFinger_Matcher();
	void			_Algo_SortSegmentByMainDir_Matcher();
	void			_Algo_MergeSegmentIfSameFinger_Matcher();
	void			_Algo_SearchFingerTipPosition_Matcher();

	// Enhanced
	int				_Algo_SegmentFingerEnhanced(unsigned char *ImgData, NEW_SEGMENT_ARRAY *m_new_segment_arr,
												BYTE *m_OutSplitResultArray, BYTE *m_OutSplitResultArrayEx, int *m_OutSplitResultArrayExWidth, 
									  int *m_OutSplitResultArrayExHeight, int *m_OutSplitResultArrayExSize);
	int				_Algo_SegmentationImageEnhanced(unsigned char *ImgData,BYTE *m_segment_enlarge_buffer_enhanced,
												  BYTE *m_segment_enlarge_buffer_for_fingercnt_enhanced,
												  BYTE *EnlargeBuf_enhanced,
												  BYTE *EnlargeBuf_Org_enhanced);
	int				_Algo_GetSegmentInfoEnhanced(NEW_SEGMENT_ARRAY *segment_arr, int ImageBright,
												BYTE *m_segment_enlarge_buffer_for_fingercnt,
												BYTE *m_labeled_segment_enlarge_buffer_enhanced);
	void			_Algo_GetMainDirectionOfFingerEnhanced(NEW_SEGMENT_ARRAY *m_new_segment_arr);
	void			_Algo_SortSegmentByMainDirEnhanced(NEW_SEGMENT_ARRAY *m_new_segment_arr);
	void			_Algo_MergeSegmentIfSameFingerEnhanced(NEW_SEGMENT_ARRAY *m_new_segment_arr);
	void			_Algo_SearchFingerTipPositionEnhanced(NEW_SEGMENT_ARRAY *m_new_segment_arr,
														BYTE *EnlargeBuf_enhanced,
														BYTE *m_labeled_segment_enlarge_buffer_enhanced);
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	int				_Algo_SegmentFinger_from_SegmentInfo(unsigned char *pSegImg, unsigned char *pEnlargeImg, unsigned char *ImgData);
	int				_Algo_SegmentFinger_from_SegmentInfo_forRoll(unsigned char *pSegImg, unsigned char *pEnlargeImg, unsigned char *ImgData);

	void			_Algo_UpdateBestFrame(BYTE *InImg, const FrameImgAnalysis frameImgAnaysis);

	NEW_SEGMENT_ARRAY m_segment_arr;
	NEW_SEGMENT_ARRAY m_segment_arr_90;
	NEW_SEGMENT_ARRAY m_segment_arr_matcher;

	int				m_CaptureGood;
	int				m_mainTOFValue;

public:

//	DWORD arrPos_Watson[IMG_SIZE];
	DWORD *arrPos_Watson;
	DWORD *arrPos_Columbo;
	int *m_Curve_refx1_arr;
	int *m_Curve_refx2_arr;
	int *m_Curve_refy1_arr;
	int *m_Curve_refy2_arr;
	int *m_Curve_p_arr;
	int *m_Curve_q_arr;

	BOOL m_DryCapture;
	int *m_nPGoodFrameCount;
	BOOL m_bRotatedImage;

	int *mean_h;

//	int sum_gray_buffer[MAX_CIS_IMG_SIZE];
	int *sum_gray_buffer;

	unsigned char			*m_capture_rolled_buffer;			// Buffer for rolling
	unsigned char			*m_capture_rolled_best_buffer;	// Best buffer for rolling
	unsigned char			*m_capture_rolled_local_best_buffer;	// Best buffer for rolling
	int						m_best_roll_width;
	// stitching
	int g_StartX, g_StartY;			// 처음 위치
	int g_EndX, g_EndY;				// 가장 끝 위치
	int g_LastX, g_LastY;			// 최근 위치
	int g_Stitch_Dir;				// 이동 방향 1 : -->   2 : <-- 
	int g_RollingWidth;				// 롤링 크기 (pixel)
	int g_StitchCount;
	int g_SmearDetectCount;			// 미끄러짐에 의한 Skip 회수 저장
	int m_ContrastofFingerprint;	// Device 별 Max Contrast 값을 계산함
	BOOL g_Rolling_Complete;		// 충분한 지문이 들어왔는지 체크
	BOOL g_Rolling_Saved_Complete;		// 충분한 지문이 들어왔는지 체크
	BOOL g_Rolling_BackTrace;
	int TotalCount1, TotalCount2;
	int g_Prev_MergePosX;

	BOOL m_bDistortionMaskPath;
	char m_cDistortionMaskPath[1024];

	// check rolling start condition 
	int prev_count;
	int prev_bright;
	int prev_pos_x;
	int prev_pos_y;
	int isPassedCount;
	BOOL bSmearDectected;
	BOOL bBestFrameSmearDectected;
	BOOL bSmearDirectionHor;
	BOOL bSmearDirectionVer;
	BOOL bBestFrameSmearDirectionHor;
	BOOL bBestFrameSmearDirectionVer;
	int	 nInvalidAreaDetected;

	RollingInfo				m_rollingInfo;
	int						m_rollingStatus;
	unsigned char			m_first_brightness;
	int						m_Prev_NewPosX;
	int						m_Prev_NewPosY;
	int						m_Last_NewPosX;
	int						m_Last_NewPosY;
	int						m_ignore_smear;

//	unsigned char			m_Table_BlockNormalize[256][256];

public:	// Members variables
#if defined(__IBSCAN_SDK__)
    BYTE                *m_GetImgFromApp;
#endif
	BYTE				*m_ImgFromCIS;
	BYTE				*m_ImgOnProcessing;
	BYTE				*m_Inter_Img;
	BYTE				*m_Inter_Img2;
	BYTE				*m_Inter_Img3;
	BYTE				*m_Inter_Img4;
	BYTE				*m_Inter_Img5;
	BYTE				*m_Inter_Seg_Img;
	BYTE				*m_Curve_Org;
	BYTE				*m_Curve_Tmp1;
	BYTE				*m_Curve_Tmp2;
	BYTE				*m_OutRollResultImg;
	BYTE				*m_OutResultImg;
	BYTE				*m_OutSplitResultArray;
	BYTE				*m_OutSplitResultArrayEx;
	int					*m_OutSplitResultArrayExWidth;
	int					*m_OutSplitResultArrayExHeight;
	int					*m_OutSplitResultArrayExSize;
	BYTE				*m_OutEnhancedImg;
	BYTE				*m_OutSplitEnhancedArray;
	BYTE				*m_OutSplitEnhancedArrayEx;
	int					*m_OutSplitEnhancedArrayExWidth;
	int					*m_OutSplitEnhancedArrayExHeight;
	int					*m_OutSplitEnhancedArrayExSize;
	USHORT				*m_MagBuffer_short;
	BYTE				*m_MagBuffer;

	BYTE				*m_TFT_DefectMaskImg;
	USHORT				*m_TFT_DefectMaskList;
	int					m_TFT_DefectMaskCount;

	FrameImgAnalysis	m_cImgAnalysis;

	GoodCaptureInfo		m_CaptureInfo;

	BYTE				*m_TFT_MaskImg;
	BYTE				*m_TFT_MaskImgforFlat;
	BYTE				*m_TFT_MaskImgforRoll;
	int					*m_tmpNoiseBuf;

	int					*mean_minus;	// vertical noise issue
	short				*mean_cnt;		// vertical noise issue

	BestFrameInfo		m_BestFrame;

	BYTE				*m_final_image;	// for RESERVED_GetFinalImageByNative()

	std::vector<FingerQuality *> m_pFingerQuality;  // List of finger quality

	CBPPreviewInfo		m_CBPPreviewInfo;

public: // Members variables

	// Algorithm
	BOOL			m_CalledByValidGeo;

	unsigned short *stack;
	unsigned char *m_labeled_segment_enlarge_buffer;
	unsigned char *EnlargeBuf;
	unsigned char *EnlargeBuf_Org;
	unsigned char *m_segment_enlarge_buffer;
	unsigned char *m_segment_enlarge_buffer_for_fingercnt;

	short *m_UM_Watson_F;
    unsigned char *m_WaterImage;
	short *m_UM_Watson_F_Enhanced;

	unsigned char *G_GammaTable;

	int				m_prevInput_height;
	int				m_prevInput_width;
	int				m_prevZoomout_height;
	int				m_prevzoomout_width;

	int				*m_refy1_arr;
	int				*m_refy2_arr;
	short			*m_p_arr;
	int				*m_refx1_arr;
	int				*m_refx2_arr;
	short			*m_q_arr;

	int				CIS_IMG_W;
	int				CIS_IMG_H;
	int				CIS_IMG_SIZE;
	int				IMG_H;
	int				IMG_W;
	int				IMG_SIZE;
	int				CIS_IMG_W_ROLL;
	int				CIS_IMG_H_ROLL;
	int				CIS_IMG_SIZE_ROLL;
	int				IMG_W_ROLL;
	int				IMG_H_ROLL;
	int				IMG_SIZE_ROLL;
	int				IMG_HALF_W;
	int				IMG_HALF_H;
	int				IMG_HALF_SIZE;
	int				ENLARGESIZE_W;
	int				ENLARGESIZE_H;

	int				ZOOM_OUT;
	int				ZOOM_H;
	int				ZOOM_W;
	int				ENLARGESIZE_ZOOM_H;
	int				ENLARGESIZE_ZOOM_W;
	int				ZOOM_ENLAGE_H;
	int				ZOOM_ENLAGE_W;

	int				ZOOM_OUT_ROLL;
	int				ZOOM_H_ROLL;
	int				ZOOM_W_ROLL;
	int				ENLARGESIZE_ZOOM_H_ROLL;
	int				ENLARGESIZE_ZOOM_W_ROLL;
	int				ZOOM_ENLAGE_H_ROLL;
	int				ZOOM_ENLAGE_W_ROLL;

	int				m_PPI_Correction_Horizontal;
	int				m_PPI_Correction_Vertical;
protected: // Const variables

private:

#ifdef WINCE
	void *CIBAlgorithm::BlockAlloc(int iObjSize, int iObjNumber);
	void CIBAlgorithm::BlockFree(void *ptr);

	BYTE *m_BlockAllocStart;
    int m_BlockAllocSize;
	int m_BlockAllocUsed;
	BYTE *m_BlockAllocNext;
#endif

};


/////////////////////////////////////////////////////////////////////////////
#endif //__IB_ALGORITHM_H__
/////////////////////////////////////////////////////////////////////////////
