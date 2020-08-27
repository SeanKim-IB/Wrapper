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
#include "MainCapture.h"
#include "../IBScanUltimateDLL.h"
#ifdef __linux__
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include "memwatch.h"
#include <sys/time.h>
#include "LinuxPort.h"
#endif


////////////////////////////////////////////////////////////////////////////////
// Implementation

BOOL CMainCapture::_GetOneFrameImage_Curve(unsigned char *Image, int ImgSize)
{
    int		    i, j;
    LONG	    lActualBytes = 0;
    int	 	    nRc;
    UCHAR	    outBuffer[64] = {0};
    LONG	    lReadByte = 0;
    int		    CisWidth, CisHeight, CisWidth1, CisHeight1;

    CisWidth = m_UsbDeviceInfo.CisImgHeight + UPPER_H;			// We need a rotate to make output image
    CisHeight = m_UsbDeviceInfo.CisImgWidth + SIDE_W;			// We need a rotate to make output image

    outBuffer[0] = (UCHAR)NA2CTL_GET_ONE_FRAME;
    outBuffer[1] = (UCHAR)m_UsbDeviceInfo.CisColStart;		// ImagePosX
    outBuffer[2] = (UCHAR)(m_UsbDeviceInfo.CisColStart >> 8);		// ImagePosX
    outBuffer[3] = (UCHAR)(m_UsbDeviceInfo.CisRowStart);		// ImagePosY
    outBuffer[4] = (UCHAR)(m_UsbDeviceInfo.CisRowStart >> 8);		// ImagePosY
    outBuffer[5] = (UCHAR)((CisWidth));		// ImageWidth
    outBuffer[6] = (UCHAR)((CisWidth) >> 8);		// ImageWidth
    outBuffer[7] = (UCHAR)((CisHeight));		// ImageHeight
    outBuffer[8] = (UCHAR)((CisHeight) >> 8);		// ImageHeight
	outBuffer[9] = (UCHAR)m_propertyInfo.nContrastValue;		// Gain

    nRc = _UsbBulkOut(EP1OUT, 0x09/*BufSize*/, outBuffer, 10);

    // 2013-05-21 enzyme add - if you use faster micro processor (i.e Intel i5, i7, ...),
    // you need the time syncronization(min. 1ms) to get a image frame after tranfering request packet.
    // Perhaps this is a Curve's firmware bug.
    Sleep(3);

    ImgSize = CisWidth * CisHeight;

    int loop = (int)(ImgSize / __CURVE_MAX_PACKET_SIZE__) + 1;
    unsigned char	*pBuf = m_pAlgo->m_Curve_Org;
    for (i = 0; i < loop; i++)
    {
        nRc = _UsbBulkIn(EP6IN_CURVE, pBuf, __CURVE_MAX_PACKET_SIZE__, &lReadByte);
        if ((nRc != IBSU_STATUS_OK) || ((nRc == IBSU_STATUS_OK) && (lActualBytes == 0) && (i > 1)))
        {
            return FALSE;
        }

        lActualBytes += lReadByte;
        pBuf += lReadByte;

        if (lActualBytes == ImgSize)
        {
            break;
        }
    }

    // Rotate image
    CisWidth1  = m_UsbDeviceInfo.CisImgWidth + SIDE_W;
    CisHeight1 = m_UsbDeviceInfo.CisImgHeight + UPPER_H;
    for (i = 0; i < CisHeight1; i++)
    {
        for (j = 0; j < CisWidth1; j++)
        {
            Image[(i * CisWidth1) + j] = m_pAlgo->m_Curve_Org[(CisHeight - 1 - j) * CisWidth + (CisHeight1 - 1 - i)];
        }
    }

    return TRUE;
}

int CMainCapture::_InitializeForCISRegister_Curve(WORD rowStart, WORD colStart, WORD width, WORD height)
{
    UCHAR       val;

    int nWidth, nHeight;
    nWidth  = m_UsbDeviceInfo.CisImgHeight + UPPER_H;			// We need a rotate to make output image
    nHeight = m_UsbDeviceInfo.CisImgWidth + SIDE_W;			// We need a rotate to make output image
    _Hynix_SetImageSize(m_UsbDeviceInfo.CisColStart - SIDE_W, m_UsbDeviceInfo.CisRowStart, nWidth, nHeight);

    _Hynix_ReadDeviceStatus(&val);

    // Important notice!!
    // Cypress firmware makes the USB mode to USB1.1 interface automatically whenever Curve is used.
    // So according to connected USB host interface,
    // SDK should be called NA2CTL_SET_NORMAL_OPERATION to change the USB mode the USB2.0 interface
    // or NA2CTL_SET_IDLE to change the USB mode the USB1.1 interface
//	if( m_UsbDeviceInfo.nUsbSpeed != __USB_HIGH_SPEED__ )
    {
        _Hynix_ChangeOperationMode(NA2CTL_SET_NORMAL_OPERATION);
    }
/*    else
    {
        _Hynix_ChangeOperationMode(NA2CTL_SET_IDLE);
    }
*/
    _Hynix_ResetFifo();

    return IBSU_STATUS_OK;
}

int	CMainCapture::Capture_SetLEVoltage_Curve(int voltageValue)
{
    return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
}

int	CMainCapture::Capture_GetLEVoltage_Curve(int *voltageValue)
{
    return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
}

int CMainCapture::_SetLEOperationMode_Curve(WORD addr)
{
    int         nRc = IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
    UCHAR		outBuffer[64] = {0};

    // Selective support the LE control using touch sensor for TBN
    if (m_UsbDeviceInfo.bEnableOperationLE)
    {
        outBuffer[0] = (UCHAR)NA2CTL_TOUCH_CONTROL;
        if ((IBSU_LEOperationMode)addr == ENUM_IBSU_LE_OPERATION_AUTO)
        {
            outBuffer[1] = (UCHAR)1;
        }
        else if ((IBSU_LEOperationMode)addr == ENUM_IBSU_LE_OPERATION_ON)
        {
            outBuffer[1] = (UCHAR)0;
        }
        else
        {
            return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
        }

        nRc = _UsbBulkOutIn(EP1OUT, 0x01/*BufSize*/, outBuffer, 2, -1, NULL, 0, NULL);

    }

    return nRc;
}

AcuisitionState CMainCapture::_GoJob_PreImageProcessing_Curve(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage)
{
    *bIsGoodImage = FALSE;

    m_pAlgo->_Algo_Curve_AnalysisImage(m_pAlgo->m_Curve_Org, &m_pAlgo->m_CaptureInfo);
    m_pAlgo->_Algo_Curve_CuttingDumyArea_with_Noise(InImg, OutImg, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.CisImgSize, m_propertyInfo.nContrastValue, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
    m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->m_CaptureInfo.real_histo;
#ifdef __USE_DEBUG_WITH_BITMAP__
    SaveBitmapImage("c:\\InImg.bmp", OutImg, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight, TRUE);
#endif

    // 지문 개수 루틴
    m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_Curve_GetFingerCount(OutImg, m_pAlgo->m_cImgAnalysis.mean);
    if (m_pAlgo->m_cImgAnalysis.finger_count == 0)
    {
        memset(OutImg, 0, m_UsbDeviceInfo.CisImgSize);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Need following info. because Curve CIS has a lot of noise,
    // it is very difficult to measure the exact detected finger area.
    m_pAlgo->m_cImgAnalysis.LEFT   = 1;
    m_pAlgo->m_cImgAnalysis.RIGHT  = m_UsbDeviceInfo.CisImgWidth - 1;
    m_pAlgo->m_cImgAnalysis.TOP    = 1;
    m_pAlgo->m_cImgAnalysis.BOTTOM = m_UsbDeviceInfo.CisImgHeight - 1;
    ////////////////////////////////////////////////////////////////////////////

    if (m_nFrameCount > 0 && m_pAlgo->m_cImgAnalysis.finger_count == 0)
    {
        m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_CLEARD;
    }

    if (m_bFirstSent_clbkClearPlaten && m_clbkProperty.nPlatenState == ENUM_IBSU_PLATEN_CLEARD)
    {
        m_bFirstSent_clbkClearPlaten = FALSE;
        _PostCallback(CALLBACK_CLEAR_PLATEN);
    }

    m_SavedFingerState = m_clbkProperty.nFingerState;
    if (m_pAlgo->m_cImgAnalysis.finger_count == 0)		// add 0.10.4 by enzyme
    {
        m_clbkProperty.nFingerState = ENUM_IBSU_NON_FINGER;
    }
    else if (m_propertyInfo.nNumberOfObjects > m_pAlgo->m_cImgAnalysis.finger_count)
    {
        m_clbkProperty.nFingerState = ENUM_IBSU_TOO_FEW_FINGERS;
    }
    else if (m_propertyInfo.nNumberOfObjects == m_pAlgo->m_cImgAnalysis.finger_count)
    {
        m_clbkProperty.nFingerState = ENUM_IBSU_FINGER_COUNT_OK;
    }
    else
    {
        m_clbkProperty.nFingerState = ENUM_IBSU_TOO_MANY_FINGERS;
    }

    if (!m_bFirstSent_clbkClearPlaten && m_SavedFingerState != m_clbkProperty.nFingerState)
    {
        _PostCallback(CALLBACK_FINGER_COUNT);
    }

    if ((m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CONTRAST) == IBSU_OPTION_AUTO_CONTRAST)
    {
        *bIsGoodImage = _Algo_Curve_AutoCapture(&m_pAlgo->m_CaptureInfo, &m_propertyInfo, (int)(m_clbkProperty.imageInfo.FrameTime * 1000));
        _Set_CIS_GainRegister(m_propertyInfo.nContrastValue, TRUE);
    }
    else
    {
        *bIsGoodImage = _Algo_Curve_ManualCapture(__CAPTURE_MAX_BRIGHT__ + 50, __CAPTURE_MIN_BRIGHT__ + 35, &m_propertyInfo, (int)(m_clbkProperty.imageInfo.FrameTime * 1000));
        _Set_CIS_GainRegister(m_propertyInfo.nContrastValue, FALSE);
    }

    IBSU_FingerQualityState savedQualityArray[4];
    memcpy(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray));
    m_pAlgo->_Algo_Curve_JudgeFingerQuality(&m_pAlgo->m_cImgAnalysis, *bIsGoodImage, &m_propertyInfo, &m_clbkProperty);

    if (!m_bFirstSent_clbkClearPlaten)
        if (memcmp(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray)) != 0)
        {
            _PostCallback(CALLBACK_FINGER_QUALITY);
        }

    return ACUISITION_NONE;
}

void CMainCapture::_PostImageProcessing_ForPreview_Curve(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage)
{
    m_pAlgo->_Algo_Curve_DistortionRestoration_with_Bilinear(InImg, m_pAlgo->m_Inter_Img3);
    m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_Inter_Img3, OutImg, m_UsbDeviceInfo.ImgWidth * m_UsbDeviceInfo.ImgHeight);
}

void CMainCapture::_PostImageProcessing_ForResult_Curve(BYTE *InImg, BYTE *OutImg)
{
    if (m_pAlgo->m_cImgAnalysis.finger_count == 0)
    {
        memset(OutImg, 255, m_UsbDeviceInfo.ImgSize);
        memset(m_pAlgo->m_OutSplitResultArray, 255, m_UsbDeviceInfo.ImgSize);
        return;
    }

    m_pAlgo->_Algo_Curve_OrgImageEnhance(InImg, OutImg);
    m_pAlgo->_Algo_Curve_DistortionRestoration_with_Bilinear(InImg, m_pAlgo->m_Inter_Img2);

    unsigned char *TmpImg = m_pAlgo->m_Curve_Tmp1;
    memcpy(TmpImg, m_pAlgo->m_Inter_Img2, IMAGE_WIDTH * IMAGE_HEIGHT);

    unsigned char(*enlargeFrint_backup)[ENLARGE_IMG_W] = (unsigned char( *)[ENLARGE_IMG_W])m_pAlgo->m_Inter_Img2;
    unsigned char(*enlargeFrint)[ENLARGE_IMG_W] = (unsigned char( *)[ENLARGE_IMG_W])m_pAlgo->m_Inter_Img3;
    unsigned char(*image_background_buffer)[ENLARGE_QUARTER_W] = (unsigned char( *)[ENLARGE_QUARTER_W])m_pAlgo->m_Inter_Img4;

    m_pAlgo->EnlargeImage(m_pAlgo->m_Inter_Img2, enlargeFrint);					//Image Enlargement for below processing
    memcpy(enlargeFrint_backup, enlargeFrint, ENLARGE_IMG_H * ENLARGE_IMG_W);	//Backup no processed image

    m_pAlgo->MedianFilter(enlargeFrint, (unsigned char( *)[ENLARGE_IMG_W])m_pAlgo->m_MagBuffer_short);									//1. small thermal noise elimination
    m_pAlgo->HistogramStretch(enlargeFrint);								//2. image enhancement for dry finger
    m_pAlgo->BackgroundCheck(enlargeFrint, image_background_buffer, m_pAlgo->m_CaptureInfo.current_histo, m_pAlgo->m_CaptureInfo.noise_histo, (unsigned char *)m_pAlgo->m_MagBuffer_short);
    //3. background check for dry finger noise elimination
    int i, val;
    for (i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++)
    {
        val = TmpImg[i] - m_pAlgo->m_cImgAnalysis.noise_histo;
        if (val < 0)
        {
            val = 0;
        }
        TmpImg[i] = val;
    }

    m_pAlgo->EnlargeImage(TmpImg, enlargeFrint_backup);					//Image Enlargement for below processing

    m_pAlgo->Back_Ground_display(image_background_buffer, enlargeFrint_backup);
    m_pAlgo->Reverse_Enlarge(enlargeFrint_backup, OutImg);

    memcpy(m_pAlgo->m_Inter_Img2, OutImg, m_UsbDeviceInfo.ImgWidth * m_UsbDeviceInfo.ImgHeight);

    m_pAlgo->_ALGO_Curve_Vignetting(m_pAlgo->m_Inter_Img2, OutImg);
    m_pAlgo->_ALGO_Curve_HistogramEnhance(OutImg, m_pAlgo->m_Inter_Img2);
    m_pAlgo->_ALGO_Curve_SWUniformity(m_pAlgo->m_Inter_Img2, OutImg);

    // Convert to Result image
	memset(m_pAlgo->m_OutSplitResultArray, 0x00, m_UsbDeviceInfo.ImgSize*IBSU_MAX_SEGMENT_COUNT);
	memset(m_pAlgo->m_OutSplitResultArrayEx, 0x00, m_UsbDeviceInfo.ImgSize*IBSU_MAX_SEGMENT_COUNT);
	m_pAlgo->_Algo_SegmentFinger(OutImg);

	// Convert to Splited image
	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitResultArray, m_pAlgo->m_OutSplitResultArray, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize);
	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitResultArrayEx, m_pAlgo->m_OutSplitResultArrayEx, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize);
	// Convert to Result image
	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);
}

int CMainCapture::_Hynix_SetRegister(UCHAR adr, UCHAR val)
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};

    outBuffer[0] = (UCHAR)NA2CTL_WRITE_CIS_REGISTER;
    outBuffer[1] = (UCHAR)adr;
    outBuffer[2] = (UCHAR)val;

    return _UsbBulkOutIn(EP1OUT, 0x02/*BufSize*/, outBuffer, 3, -1, NULL, 0, NULL);
}

int CMainCapture::_Hynix_GetRegister(UCHAR adr, UCHAR *val)
{
    UCHAR		bData[MAX_DATA_BYTES + 1] = {0};
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};
    int			nRc;

    outBuffer[0] = (UCHAR)NA2CTL_READ_CIS_REGISTER;
    outBuffer[1] = (UCHAR)adr;
    nRc = _UsbBulkOutIn(EP1OUT, 0x01/*BufSize*/, outBuffer, 2, EP1IN, &bData[0], 1, NULL);
    if (nRc == IBSU_STATUS_OK)
    {
        *val = bData[0];
    }
    else
    {
        *val = 0;
    }

    return nRc;
}

int CMainCapture::_Hynix_SetImageSize(int x, int y, int width, int height)
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};

    outBuffer[0] = (UCHAR)NA2CTL_WRITE_WIN_POS;
    outBuffer[1] = (UCHAR)(x);
    outBuffer[2] = (UCHAR)(x		>> 8);
    outBuffer[3] = (UCHAR)(y);
    outBuffer[4] = (UCHAR)(y		>> 8);
    outBuffer[5] = (UCHAR)(width);
    outBuffer[6] = (UCHAR)(width	>> 8);
    outBuffer[7] = (UCHAR)(height);
    outBuffer[8] = (UCHAR)(height	>> 8);

    return _UsbBulkOutIn(EP1OUT, 0x08/*BufSize*/, outBuffer, 9, -1, NULL, 0, NULL);
}

int CMainCapture::_Hynix_ReadDeviceStatus(UCHAR *val)
{
    UCHAR		bData[MAX_DATA_BYTES + 1] = {0};
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};
    int			nRc;

    outBuffer[0] = (UCHAR)NA2CTL_READ_STATUS;
    nRc = _UsbBulkOutIn(EP1OUT, 0x00/*BufSize*/, outBuffer, 1, EP1IN, &bData[0], 3, NULL);
    if (nRc == IBSU_STATUS_OK)
    {
        *val = bData[0];
    }
    else
    {
        *val = 0;
    }

    return nRc;
}

int CMainCapture::_Hynix_ChangeOperationMode(UCHAR mode)
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};

    outBuffer[0] = (UCHAR)mode;

    return _UsbBulkOutIn(EP1OUT, 0x00/*BufSize*/, outBuffer, 1, -1, NULL, 0, NULL);
}

int CMainCapture::_Hynix_ResetFifo()
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};

    outBuffer[0] = (UCHAR)NA2CTL_RESET_FIFO;

    return _UsbBulkOutIn(EP1OUT, 0x00/*BufSize*/, outBuffer, 1, -1, NULL, 0, NULL);
}

int CMainCapture::_Hynix_WriteEEPROM(WORD addr, const BYTE *buf, int len)
{
    UCHAR		outBuffer[64]={0};
	int nRc;

	// Unlock Write Protection
	outBuffer[0] = CMD_EEPROM_WRITEPROTECTION;
	outBuffer[1] = 0;                 // 0 : Unlock,  1 : Lock
	if((nRc = _UsbBulkOutIn(EP1OUT, 1/*BufSize*/, outBuffer, 2, -1, NULL, 0, NULL)) != IBSU_STATUS_OK)
		return nRc;

	outBuffer[0] = NA2CTL_WRITE_EEPROM;
	outBuffer[1] = (UCHAR)(addr&0xFF);
	outBuffer[2] = (UCHAR)((addr>>8)&0x1F);
	outBuffer[3] = (UCHAR)len;
	for( int i = 0; i < len; i++ )
	{
		outBuffer[i+4] = buf[i];
	}

	if((nRc = _UsbBulkOutIn( EP1OUT, 3+len/*BufSize*/, outBuffer, 4+len, -1, NULL, 0, NULL)) != IBSU_STATUS_OK)
		return nRc;

	// Lock Write Protection
	outBuffer[0] = CMD_EEPROM_WRITEPROTECTION;
	outBuffer[1] = 1;                 // 0 : Unlock,  1 : Lock
	if((nRc = _UsbBulkOutIn(EP1OUT, 1 /*BufSize*/, outBuffer, 2, -1, NULL, 0, NULL)) != IBSU_STATUS_OK)
		return nRc;

	return IBSU_STATUS_OK;
}

int CMainCapture::_Hynix_ReadEEPROM(WORD addr, BYTE *buf, int len)
{
    UCHAR		outBuffer[64] = {0};

    outBuffer[0] = NA2CTL_READ_EEPROM;
    outBuffer[1] = (UCHAR)(addr & 0xFF);
    outBuffer[2] = (UCHAR)((addr >> 8) & 0x1F);
    outBuffer[3] = (UCHAR)len;

    return _UsbBulkOutIn(EP1OUT, 3/*BufSize*/, outBuffer, 4, EP1IN, buf, len, NULL);
}

int CMainCapture::_GoJob_DummyCapture_ForThread_Curve(BYTE *InImg)
{
    int			finger_count;

    m_pAlgo->_Algo_Curve_AnalysisImage(m_pAlgo->m_Curve_Org, &m_pAlgo->m_CaptureInfo);
    m_pAlgo->_Algo_Curve_CuttingDumyArea_with_Noise(InImg, m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img3, m_UsbDeviceInfo.CisImgSize, m_propertyInfo.nContrastValue, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
    m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->m_CaptureInfo.real_histo/* - m_pAlgo->m_CaptureInfo.noise_histo*/;

    // 지문 개수 루틴
    finger_count = m_pAlgo->_Algo_Curve_GetFingerCount(m_pAlgo->m_Inter_Img2, m_pAlgo->m_cImgAnalysis.mean);
    if (finger_count == 0)
    {
        return 1;
    }

    return 0;
}

void CMainCapture::_GoJob_Initialize_ForThread_Curve()
{
    m_pAlgo->_Algo_Curve_Init_Distortion_Merge_with_Bilinear();
    m_pAlgo->_Algo_Curve_Init_Pinhole();
    m_pAlgo->_Algo_Init_GammaTable();
}

int CMainCapture::_PowerCaptureInit_Curve(GoodCaptureInfo *capinfo, PropertyInfo *propertyInfo)
{
    propertyInfo->nContrastValue = __DEFAULT_CURVE_CONTRAST_VALUE__;
    _Set_CIS_GainRegister(propertyInfo->nContrastValue, FALSE);
    _Hynix_SetRegister(0x31, 0x7f);
    _Hynix_SetRegister(0x32, 0x7f);
    _Hynix_SetRegister(0x33, 0x7f);

    propertyInfo->nIntegrationValue = __DEFAULT_CURVE_INTEGRATION_VALUE__ + 2;
    _Hynix_SetRegister(0x25, propertyInfo->nIntegrationValue);
    _Hynix_SetRegister(0x26, 0x00);
    _Hynix_SetRegister(0x27, 0x00);

    _OninitCISRegister_Curve(0, propertyInfo);
    _Set_CIS_GainRegister(m_propertyInfo.nContrastValue, FALSE);

    capinfo->lessTHAN2 = 0;
    capinfo->lessTHAN3 = 0;
    capinfo->lessTHAN4 = 0;
    capinfo->prev_lessTHAN2 = INIT_PREV;
    capinfo->prev_lessTHAN3 = INIT_PREV;
    capinfo->prev_lessTHAN4 = INIT_PREV;
    capinfo->prev_histo = INIT_PREV;
    capinfo->noise_histo = 0;
    capinfo->current_histo = 0;
    capinfo->detected_count = 0;
    capinfo->condition_stay = 0;
    capinfo->STOP_DETECT_COUNT = 4;
    capinfo->GRAP_NULL_FRAME = 0;
    capinfo->dry_count = 0;
    capinfo->isDetectGain = 34;
    capinfo->tot_frame = 0;
    capinfo->integration_count = 0;

    return 1;
}

void CMainCapture::_OninitCISRegister_Curve(int option, PropertyInfo *propertyInfo)
{
    if (option < 1)
    {
        propertyInfo->nContrastValue = __DEFAULT_CURVE_CONTRAST_VALUE__;
        _Hynix_SetRegister(0x31, 0x10);			// R gain
        _Hynix_SetRegister(0x32, 0x10);			// G gain
        _Hynix_SetRegister(0x33, 0x10);			// B gain
    }

    //	integration_time = 8;
    propertyInfo->nIntegrationValue = __DEFAULT_CURVE_INTEGRATION_VALUE__;
    _Hynix_SetRegister(0x25, propertyInfo->nIntegrationValue);
    _Hynix_SetRegister(0x26, 0x00);
    _Hynix_SetRegister(0x27, 0x00);
}

BOOL CMainCapture::_Algo_Curve_AutoCapture(GoodCaptureInfo *capinfo, PropertyInfo *propertyInfo, int frameTime)
{
    BOOL		SetRegisterForDry;
    //	const int	MIN_CAPTURE_TIME = 200;
    int			nRc;

    m_pAlgo->m_cImgAnalysis.frame_count++;
    nRc = m_pAlgo->_Algo_Curve_GoodCapture(capinfo, propertyInfo, &SetRegisterForDry);
    if (SetRegisterForDry)
    {
        _Hynix_SetRegister(0x25, propertyInfo->nIntegrationValue);
        _Hynix_SetRegister(0x26, 0x00);
        _Hynix_SetRegister(0x27, 0x00);

        //BlackLevel Threshold
        _Hynix_SetRegister(0x01, 0x01);
        _Hynix_SetRegister(0x40, 0x00);
        _Hynix_SetRegister(0x41, 0xEE);
        _Hynix_SetRegister(0x42, 0xEE);
        _Hynix_SetRegister(0x43, 0xEE);

        //RGB Gain
        _Hynix_SetRegister(0x31, 0x06);			// R gain
        _Hynix_SetRegister(0x32, 0x06);			// G gain
        _Hynix_SetRegister(0x33, 0x06);			// B gain

        Sleep(100);
    }

    if (nRc == 0)
    {
        _OninitCISRegister_Curve(m_pAlgo->m_CaptureInfo.NOISE_DETECT, propertyInfo);
        m_pAlgo->m_cImgAnalysis.frame_count = 0;
    }
    else if (nRc == 2 && m_clbkProperty.nPlatenState == ENUM_IBSU_PLATEN_CLEARD)
    {
        // It is essential logic in capture function, but sales do not want it on Curve due to capture speed
        /*		if( abs(m_pAlgo->m_cImgAnalysis.center_x-m_pAlgo->m_cImgAnalysis.pre_center_x) <= 2 &&
        			abs(m_pAlgo->m_cImgAnalysis.center_y-m_pAlgo->m_cImgAnalysis.pre_center_y) <= 2
        		)
        		{
        			if( (m_pAlgo->m_cImgAnalysis.frame_count*frameTime) > MIN_CAPTURE_TIME )
        			{
        				m_pAlgo->m_cImgAnalysis.is_final = TRUE;
        			}
        		}
        */
        m_pAlgo->m_cImgAnalysis.is_final = TRUE;
    }

    m_pAlgo->m_cImgAnalysis.pre_center_x = m_pAlgo->m_cImgAnalysis.center_x;
    m_pAlgo->m_cImgAnalysis.pre_center_y = m_pAlgo->m_cImgAnalysis.center_y;

    return m_pAlgo->m_cImgAnalysis.is_final;
}

BOOL CMainCapture::_Algo_Curve_ManualCapture(int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime)
{
    const int	MAX_BRIGHT = maxBright, MIN_BRIGHT = minBright;
    const int	MIN_CAPTURE_TIME = 300;

    if (m_pAlgo->m_cImgAnalysis.finger_count == 0)
    {
        m_pAlgo->m_cImgAnalysis.saved_finger_count = m_pAlgo->m_cImgAnalysis.finger_count;
        m_pAlgo->m_cImgAnalysis.frame_count = 0;

        return FALSE;
    }

    m_pAlgo->m_cImgAnalysis.frame_count++;
    if (m_pAlgo->m_cImgAnalysis.mean >= MIN_BRIGHT && m_pAlgo->m_cImgAnalysis.mean <= MAX_BRIGHT &&
            abs(m_pAlgo->m_cImgAnalysis.center_x - m_pAlgo->m_cImgAnalysis.pre_center_x) <= 2 &&
            abs(m_pAlgo->m_cImgAnalysis.center_y - m_pAlgo->m_cImgAnalysis.pre_center_y) <= 2
       )
    {
        if ((m_pAlgo->m_cImgAnalysis.frame_count * frameTime) > MIN_CAPTURE_TIME)
        {
            m_pAlgo->m_cImgAnalysis.saved_finger_count = m_pAlgo->m_cImgAnalysis.finger_count;
            return TRUE;
        }
    }

    m_pAlgo->m_cImgAnalysis.pre_center_x = m_pAlgo->m_cImgAnalysis.center_x;
    m_pAlgo->m_cImgAnalysis.pre_center_y = m_pAlgo->m_cImgAnalysis.center_y;

    return FALSE;
}

int CMainCapture::_IsNeedInitializeCIS_Curve()
{
	return TRUE;
}
