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

/* to know the location of the code which is occured memory leak */
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


////////////////////////////////////////////////////////////////////////////////
// Implementation
BOOL CMainCapture::_GetOneFrameImage_CurveSETi(unsigned char *Image, int ImgSize)
{
	int		    i;
	LONG	    lActualBytes=0;
	int	 	    nRc;
	UCHAR	    outBuffer[64]={0};
	LONG	    lReadByte=0;
	int		    CisWidth, CisHeight;

	CisWidth = m_UsbDeviceInfo.CisImgWidth;			// We need a rotate to make output image
	CisHeight = m_UsbDeviceInfo.CisImgHeight;			// We need a rotate to make output image

	outBuffer[0] = (UCHAR)NA2CTL_GET_ONE_FRAME;
	outBuffer[1] = (UCHAR)m_UsbDeviceInfo.CisColStart;		// ImagePosX
	outBuffer[2] = (UCHAR)(m_UsbDeviceInfo.CisColStart >> 8);		// ImagePosX
	outBuffer[3] = (UCHAR)(m_UsbDeviceInfo.CisRowStart);		// ImagePosY
	outBuffer[4] = (UCHAR)(m_UsbDeviceInfo.CisRowStart >> 8);		// ImagePosY
	outBuffer[5] = (UCHAR)((CisWidth));		// ImageWidth
	outBuffer[6] = (UCHAR)((CisWidth) >> 8);		// ImageWidth
	outBuffer[7] = (UCHAR)((CisHeight));		// ImageHeight
	outBuffer[8] = (UCHAR)((CisHeight) >> 8);		// ImageHeight
	outBuffer[9] = (UCHAR)0x00;		// Gain (Dummy)

	nRc = _UsbBulkOut( EP1OUT, 0x09/*BufSize*/, outBuffer, 10 );
    
    // 2013-05-21 enzyme add - if you use faster micro processor (i.e Intel i5, i7, ...),
    // you need the time syncronization(min. 1ms) to get a image frame after tranfering request packet.
    // Perhaps this is a Curve's firmware bug.
    Sleep(3);

    /* To support WinUsb driver and old Curve driver at same time. */

    int loop = (int)(ImgSize / __CURVE_SETI_MAX_PACKET_SIZE__) + 1;
    unsigned char	*pBuf = Image;
    for (i = 0; i < loop; i++)
    {
        nRc = _UsbBulkIn(EP6IN_CURVE, pBuf, __CURVE_SETI_MAX_PACKET_SIZE__, &lReadByte);
		if( i == 0 && nRc == IBSU_STATUS_OK && lReadByte == 0 )
		{
#ifdef __G_DEBUG__
			TRACE(_T("=====> Warning! Missed frame i=%d, nRc=%d, nReadByte=%d\n"), i, nRc, lReadByte);
#endif
			nRc = _UsbBulkOut( EP1OUT, 0x09/*BufSize*/, outBuffer, 10 );
		}

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

	int val, iii, jjj;
	int neigh;
	unsigned char *tmpBuf = &m_pAlgo->m_Curve_Org[0];

	memcpy(tmpBuf, Image, ImgSize);
	for(iii=0; iii<CisWidth; iii++)
	{
		for(jjj=0; jjj<CisHeight; jjj++)
		{
			Image[(CisHeight-1-jjj)*CisWidth+iii] = tmpBuf[iii*CisHeight+jjj];
		}
	}

	memcpy(tmpBuf, Image, ImgSize);
	for(iii=1; iii<CisHeight-1; iii++)
	{
		for(jjj=1; jjj<CisWidth-1; jjj++)
		{
			val = m_pAlgo->m_Curve_Org[iii*CisWidth+jjj];
			if(val < 10)
				continue;

			neigh = tmpBuf[(iii-1)*CisWidth+jjj-1];
			neigh += tmpBuf[(iii-1)*CisWidth+jjj];
			neigh += tmpBuf[(iii-1)*CisWidth+jjj+1];
			neigh += tmpBuf[iii*CisWidth+jjj-1];
			neigh += tmpBuf[iii*CisWidth+jjj+1];
			neigh += tmpBuf[(iii+1)*CisWidth+jjj-1];
			neigh += tmpBuf[(iii+1)*CisWidth+jjj];
			neigh += tmpBuf[(iii+1)*CisWidth+jjj+1];
			
			if(val > neigh)
			{
				Image[iii*CisWidth+jjj] = neigh/8;
			}
		}
	}

	for(int iii=0; iii<ImgSize; iii++)
	{
		val = Image[iii]-5;
		if(val < 0)
			val = 0;
		else if(val > 210)
			val = 210;
		Image[iii] = val * 255 / 210;
	}
	
	memcpy(tmpBuf, Image, ImgSize);
	for(int iii=1; iii<CisHeight-1; iii++)
	{
		for(int jjj=1; jjj<CisWidth-1; jjj++)
		{
			Image[iii*CisWidth+jjj] = (tmpBuf[(iii-1)*CisWidth+jjj-1]+
										tmpBuf[(iii-1)*CisWidth+jjj]*4+
										tmpBuf[(iii-1)*CisWidth+jjj+1]+
										tmpBuf[(iii)*CisWidth+jjj-1]*4+
										tmpBuf[(iii)*CisWidth+jjj]*8+
										tmpBuf[(iii)*CisWidth+jjj+1]*4+
										tmpBuf[(iii+1)*CisWidth+jjj-1]+
										tmpBuf[(iii+1)*CisWidth+jjj]*4+
										tmpBuf[(iii+1)*CisWidth+jjj+1])/28;
		}
	}

	return TRUE;
}

int CMainCapture::_PowerCaptureInit_CurveSETi(GoodCaptureInfo *capinfo, PropertyInfo *propertyInfo)
{
    _OninitCISRegister_CurveSETi();

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
	capinfo->skip_count = 0;
	capinfo->same_gain_count = 0;

    return 1;
}

int CMainCapture::_InitializeForCISRegister_CurveSETi(WORD rowStart, WORD colStart, WORD width, WORD height)
{
	UCHAR		outBuffer[1]={0};

	// windowing
	int nWidth = height*2;//864;
	int nHeight = width*2;//720;
//	int nWidth = m_UsbDeviceInfo.CisImgWidth;
//	int nHeight = m_UsbDeviceInfo.CisImgHeight;
	int nSHor = (1600-nWidth)/2-1+colStart;
	int nSVer = (1200-nHeight)/2-1+rowStart;
	int nShValue;
	int nSlValue;
	int nWhValue;
	int nWlValue;
	int nHhValue;
	int nHlValue;

	Sleep(10);			// for more safe
//	_ResetFifo();
	outBuffer[0] = CMD_RESET_FIFO;
	if (_UsbBulkOutIn( EP1OUT, 0x01, outBuffer, 1, -1, NULL, 0, NULL ) != IBSU_STATUS_OK) goto done;
	Sleep(10);			// for more safe

	if (_CurveSETi_SetRegister(0x03,	0x02) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x04,	0x80) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x05,	0x0a) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x08,	0x00) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x09,	0x11) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x0A,	0x11) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x10,	0x10) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x11,	0x20) != IBSU_STATUS_OK) goto done;
//	_CurveSETi_SetRegister(0x12,	0x89);
	if (_CurveSETi_SetRegister(0x12,	0x88) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x13,	0x09) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x14,	0x00) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x16,	0xb6) != IBSU_STATUS_OK) goto done;
	
	_OninitCISRegister_CurveSETi();

	// decimation mode
//	_CurveSETi_SetRegister(0x40,	0x44);//Horizontal Start H 1/4영상
//	_CurveSETi_SetRegister(0x40,	0x45);//Horizontal Start H 1/4영상
	if (_CurveSETi_SetRegister(0x40,	0x00) != IBSU_STATUS_OK) goto done;//Horizontal Start H full 영상

	nShValue = (nSHor)/256;
	nSlValue = ((nSHor)%256)-1;
	nWhValue = (nWidth)/256;
	nWlValue = ((nWidth)%256)-1;

	if (_CurveSETi_SetRegister(0x41,	nShValue) != IBSU_STATUS_OK) goto done;//Horizontal Start H
	if (_CurveSETi_SetRegister(0x42,	nSlValue) != IBSU_STATUS_OK) goto done; //Horizontal Start L
	if (_CurveSETi_SetRegister(0x43,	nWhValue) != IBSU_STATUS_OK) goto done; //Horizontal Width H
	if (_CurveSETi_SetRegister(0x44,	nWlValue) != IBSU_STATUS_OK) goto done; //Horizontal Width L

	nShValue = (nSVer)/256;
	nSlValue = ((nSVer)%256)-1;
	nHhValue = (nHeight)/256;
	nHlValue = ((nHeight)%256)-1;
	
	if (_CurveSETi_SetRegister(0x45,	nShValue) != IBSU_STATUS_OK) goto done; //Vertal Start H
	if (_CurveSETi_SetRegister(0x46,	nSlValue) != IBSU_STATUS_OK) goto done; //Vertal Start L
	if (_CurveSETi_SetRegister(0x47,	nHhValue) != IBSU_STATUS_OK) goto done; //Vertical Height H
	if (_CurveSETi_SetRegister(0x48,	nHlValue) != IBSU_STATUS_OK) goto done; //Vertical Height L

////////////////////////////////////////////////////////////////////////////
// Important! To make the frame rate
// Control for VSYNC blank time
    if (_CurveSETi_SetRegister(0x49,	0x01) != IBSU_STATUS_OK) goto done; //V blank H      // 0x03 (0x03-10fps, 0x02-12fps)
	if (_CurveSETi_SetRegister(0x4A,	0x20) != IBSU_STATUS_OK) goto done; //V blank L      // 0x70 (0x70-10fps, 0x30-12fps)
////////////////////////////////////////////////////////////////////////////
//	_CurveSETi_SetRegister(0x4A,	0x32);
	if (_CurveSETi_SetRegister(0x4B,	0x00) != IBSU_STATUS_OK) goto done; //?
	if (_CurveSETi_SetRegister(0x4C,	0x00) != IBSU_STATUS_OK) goto done; //?
	if (_CurveSETi_SetRegister(0x4D,	0x00) != IBSU_STATUS_OK) goto done; //H blank H
	if (_CurveSETi_SetRegister(0x4E,	0x00) != IBSU_STATUS_OK) goto done; //H blank L

//	_CurveSETi_SetRegister(0x4E,	0x02); //H blank L

	if (_CurveSETi_SetRegister(0x60,	0x0b) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x61,	0x16) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x62,	0x32) != IBSU_STATUS_OK) goto done;
//	_CurveSETi_SetRegister(0x63,	0x80);
	if (_CurveSETi_SetRegister(0x71,	0x08) != IBSU_STATUS_OK) goto done;

	if (_CurveSETi_SetRegister(0x80,	0xE0) != IBSU_STATUS_OK) goto done; //IPFUN
	if (_CurveSETi_SetRegister(0x81,	0x0F) != IBSU_STATUS_OK) goto done; //SIGCNT Ori : 07
//	_CurveSETi_SetRegister(0x80,	0xE0); //IPFUN Ori : e8
//	_CurveSETi_SetRegister(0x81,	0x0D); //SIGCNT Ori : 07


	if (_CurveSETi_SetRegister(0x90,	0x00) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x91,	0x08) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x92,	0x10) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x93,	0x40) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x94,	0x04) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x95,	0x01) != IBSU_STATUS_OK) goto done;
//	_CurveSETi_SetRegister(0x96,	0x00);
	if (_CurveSETi_SetRegister(0x96,	0x80) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x97,	0x08) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x98,	0x10) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x99,	0x08) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x9a,	0x03) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x9b,	0xb0) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x9c,	0x08) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x9d,	0x24) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0x9e,	0x30) != IBSU_STATUS_OK) goto done;
	if (_CurveSETi_SetRegister(0xb8,	0x28) != IBSU_STATUS_OK) goto done;
//	_CurveSETi_SetRegister(0x81,	0x0F); //sigcnt
	if (_CurveSETi_SetRegister(0x03,	0x05) != IBSU_STATUS_OK) goto done;

    // Important notice!!
    // Cypress firmware makes the USB mode to USB1.1 interface automatically whenever Curve is used.
    // So according to connected USB host interface,
    // SDK should be called NA2CTL_SET_NORMAL_OPERATION to change the USB mode the USB2.0 interface
    // or NA2CTL_SET_IDLE to change the USB mode the USB1.1 interface
//	if( m_UsbDeviceInfo.nUsbSpeed != __USB_HIGH_SPEED__ )
    {
        outBuffer[0] = NA2CTL_SET_NORMAL_OPERATION;
    }
/*    else
    {
        outBuffer[0] = NA2CTL_SET_IDLE;
    }
*/
    if (_UsbBulkOutIn( EP1OUT, 0x01, outBuffer, 1, -1, NULL, 0, NULL ) != IBSU_STATUS_OK) goto done;

    return IBSU_STATUS_OK;

done:
    return IBSU_ERR_DEVICE_IO;
}

int	CMainCapture::Capture_SetLEVoltage_CurveSETi(int voltageValue)
{
	return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
}

int	CMainCapture::Capture_GetLEVoltage_CurveSETi(int *voltageValue)
{
	return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
}

int CMainCapture::_SetLEOperationMode_CurveSETi( WORD addr)
{
	int         nRc = IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
    UCHAR		outBuffer[64]={0};

    // Selective support the LE control using touch sensor for TBN
    if( m_UsbDeviceInfo.bEnableOperationLE )
	{
        outBuffer[0] = (UCHAR)NA2CTL_TOUCH_CONTROL;
		if( (IBSU_LEOperationMode)addr == ENUM_IBSU_LE_OPERATION_AUTO )
            outBuffer[1] = (UCHAR)1;
		else if( (IBSU_LEOperationMode)addr == ENUM_IBSU_LE_OPERATION_ON )
            outBuffer[1] = (UCHAR)0;
		else
			return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;

	    nRc = _UsbBulkOutIn( EP1OUT, 0x01/*BufSize*/, outBuffer, 2, -1, NULL, 0, NULL );

		if( (IBSU_LEOperationMode)addr == ENUM_IBSU_LE_OPERATION_ON )
		{
			UCHAR outBuffer[2] = {0x6E};
			nRc = _UsbBulkOutIn( EP1OUT, 0x00/*BufSize*/, outBuffer, 1, -1, NULL, 0, NULL );
		}
	}

    return nRc;
}

AcuisitionState CMainCapture::_GoJob_PreImageProcessing_CurveSETi(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage)
{
	*bIsGoodImage = FALSE;

	m_pAlgo->_Algo_CurveSETi_AnalysisImage(InImg, &m_pAlgo->m_CaptureInfo);
    m_pAlgo->_Algo_CurveSETi_CuttingDumyArea_with_Noise(InImg, OutImg, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.CisImgSize, m_propertyInfo.nContrastValue, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
	memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize);
	m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->m_CaptureInfo.real_histo;
#ifdef __USE_DEBUG_WITH_BITMAP__
	SaveBitmapImage("c:\\InImg.bmp", OutImg, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight, TRUE);
#endif

	// 지문 개수 루틴
	m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_CurveSETi_GetFingerCount(OutImg, m_pAlgo->m_cImgAnalysis.mean);
	if( m_pAlgo->m_cImgAnalysis.finger_count == 0 )
		memset(OutImg, 0, m_UsbDeviceInfo.CisImgSize);

////////////////////////////////////////////////////////////////////////////
// Need following info. because Curve CIS has a lot of noise,
// it is very difficult to measure the exact detected finger area. 
	m_pAlgo->m_cImgAnalysis.LEFT   = 1;
	m_pAlgo->m_cImgAnalysis.RIGHT  = m_UsbDeviceInfo.CisImgWidth-1;
	m_pAlgo->m_cImgAnalysis.TOP    = 1;
	m_pAlgo->m_cImgAnalysis.BOTTOM = m_UsbDeviceInfo.CisImgHeight-1;
////////////////////////////////////////////////////////////////////////////

	if( m_nFrameCount > 0 && m_pAlgo->m_cImgAnalysis.finger_count == 0 )
		m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_CLEARD;

	if( m_bFirstSent_clbkClearPlaten && m_clbkProperty.nPlatenState == ENUM_IBSU_PLATEN_CLEARD )
	{
		m_bFirstSent_clbkClearPlaten = FALSE;
		_PostCallback(CALLBACK_CLEAR_PLATEN);
	}

	m_SavedFingerState = m_clbkProperty.nFingerState;
	if( m_pAlgo->m_cImgAnalysis.finger_count == 0 )		// add 0.10.4 by enzyme
		m_clbkProperty.nFingerState = ENUM_IBSU_NON_FINGER;
	else if( m_propertyInfo.nNumberOfObjects > m_pAlgo->m_cImgAnalysis.finger_count )
		m_clbkProperty.nFingerState = ENUM_IBSU_TOO_FEW_FINGERS;
	else if( m_propertyInfo.nNumberOfObjects == m_pAlgo->m_cImgAnalysis.finger_count )
		m_clbkProperty.nFingerState = ENUM_IBSU_FINGER_COUNT_OK;
	else
		m_clbkProperty.nFingerState = ENUM_IBSU_TOO_MANY_FINGERS;

	if( !m_bFirstSent_clbkClearPlaten && m_SavedFingerState != m_clbkProperty.nFingerState )
	{
		_PostCallback(CALLBACK_FINGER_COUNT);
	}

    if ((m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CONTRAST) == IBSU_OPTION_AUTO_CONTRAST)
    {
        *bIsGoodImage = _Algo_CurveSeti_AutoCapture(&m_pAlgo->m_CaptureInfo, &m_propertyInfo, (int)(m_clbkProperty.imageInfo.FrameTime * 1000));
        _Set_CIS_GainRegister(m_propertyInfo.nContrastValue, TRUE);
    }
    else
    {
        *bIsGoodImage = _Algo_CurveSeti_ManualCapture(__CAPTURE_MAX_BRIGHT__ + 50, __CAPTURE_MIN_BRIGHT__ + 35, &m_propertyInfo, (int)(m_clbkProperty.imageInfo.FrameTime * 1000));
        _Set_CIS_GainRegister(m_propertyInfo.nContrastValue, FALSE);
    }

	IBSU_FingerQualityState savedQualityArray[4];
	memcpy(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray));
	m_pAlgo->_Algo_CurveSETi_JudgeFingerQuality(&m_pAlgo->m_cImgAnalysis, *bIsGoodImage, &m_propertyInfo, &m_clbkProperty);

	if( !m_bFirstSent_clbkClearPlaten )
		if( memcmp(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray)) != 0 )
			_PostCallback(CALLBACK_FINGER_QUALITY);

	return ACUISITION_NONE;
}

void CMainCapture::_PostImageProcessing_ForPreview_CurveSETi(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage)
{
	m_pAlgo->_Algo_CurveSETi_DistortionRestoration_with_Bilinear(InImg, m_pAlgo->m_Inter_Img3);
	m_pAlgo->_Algo_Sharpening(m_pAlgo->m_Inter_Img3, m_UsbDeviceInfo.ImgWidth, m_UsbDeviceInfo.ImgHeight, m_pAlgo->m_Inter_Img2);
	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_Inter_Img2, OutImg, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);
}

void CMainCapture::_PostImageProcessing_ForResult_CurveSETi(BYTE *InImg, BYTE *OutImg)
{
	if(m_pAlgo->m_cImgAnalysis.finger_count == 0)
	{
		memset(OutImg, 255, m_UsbDeviceInfo.ImgSize);
		memset(m_pAlgo->m_OutSplitResultArray, 255, m_UsbDeviceInfo.ImgSize);
		return;
	}

	m_pAlgo->_Algo_HistogramNormalize(InImg, m_pAlgo->m_Inter_Img3, m_UsbDeviceInfo.CisImgSize, 5);
	m_pAlgo->_Algo_CurveSETi_DistortionRestoration_with_Bilinear(m_pAlgo->m_Inter_Img3, m_pAlgo->m_Inter_Img2);
	m_pAlgo->_Algo_Sharpening(m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.ImgWidth, m_UsbDeviceInfo.ImgHeight, OutImg);
//    m_pAlgo->_ALGO_Curve_HistogramEnhance(OutImg, m_pAlgo->m_Inter_Img2);
//	memcpy(OutImg, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.ImgSize);
//    m_pAlgo->_ALGO_Curve_SWUniformity(m_pAlgo->m_Inter_Img2, OutImg);
	
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

int CMainCapture::_GoJob_DummyCapture_ForThread_CurveSETi(BYTE *InImg)
{
	int			finger_count;

	m_pAlgo->_Algo_CurveSETi_AnalysisImage_for_Dummy(InImg, &m_pAlgo->m_CaptureInfo);
	m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->m_CaptureInfo.real_histo;

	// 지문 개수 루틴
	finger_count = m_pAlgo->_Algo_CurveSETi_GetFingerCount(InImg, m_pAlgo->m_cImgAnalysis.mean);
	if( finger_count == 0 )
		return 1;

	return 0;
}

void CMainCapture::_GoJob_Initialize_ForThread_CurveSETi()
{
	m_pAlgo->_Algo_CurveSETi_Init_Distortion_Merge_with_Biliner();
	m_pAlgo->_Algo_Init_GammaTable();
}

int CMainCapture::_OninitCISRegister_CurveSETi()
{
	int nExposureTime = __CURVE_SETI_DEFAULT_EXPOSURE_VALUE;
	int hValueExTime = nExposureTime/256;
	int lValueExTime = (nExposureTime%256);

	_CurveSETi_SetRegister(0X30, hValueExTime);
	_CurveSETi_SetRegister(0x31, lValueExTime);

	m_propertyInfo.nContrastValue = __AUTO_CURVE_CONTRAST_MAX_VALUE__;
	_Set_CIS_GainRegister(m_propertyInfo.nContrastValue, FALSE);

	return 0;
}

BOOL CMainCapture::_Algo_CurveSeti_AutoCapture(GoodCaptureInfo *capinfo, PropertyInfo *propertyInfo, int frameTime)
{
    BOOL		SetRegisterForDry;
//    const int	MIN_CAPTURE_TIME = 200;
    int			nRc;

    m_pAlgo->m_cImgAnalysis.frame_count++;
	nRc = m_pAlgo->_Algo_CurveSETi_GoodCapture(capinfo, propertyInfo, &SetRegisterForDry);
    if (SetRegisterForDry)
    {
		int hValueExTime = m_propertyInfo.nIntegrationValue/256;
		int lValueExTime = (m_propertyInfo.nIntegrationValue%256);

		_CurveSETi_SetRegister(0X30, hValueExTime);
		_CurveSETi_SetRegister(0x31, lValueExTime);

//        Sleep(100);
    }

    if (nRc == 0)
    {
        _OninitCISRegister_CurveSETi();
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

BOOL CMainCapture::_Algo_CurveSeti_ManualCapture(int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime)
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

int CMainCapture::_CurveSETi_SetRegister(UCHAR adr, UCHAR val)
{
	UCHAR		outBuffer[MAX_DATA_BYTES+1]={0};

	outBuffer[0] = (UCHAR)CMD_WRITE_SETI_REG_CURVE;
	outBuffer[1] = (UCHAR)adr;
	outBuffer[2] = (UCHAR)val;

	int nRc = -1;

	nRc = _UsbBulkOutIn( EP1OUT, 0x03, outBuffer, 3, -1, NULL, 0, NULL );

	return nRc;

}

int CMainCapture::_CurveSETi_GetRegister(UCHAR adr, UCHAR *val)
{
	UCHAR		outBuffer[MAX_DATA_BYTES+1]={0};

	UCHAR       inBuffer[MAX_DATA_BYTES + 1] = {0};

	outBuffer[0] = (UCHAR)CMD_READ_SETI_REG_CURVE;
	outBuffer[1] = (UCHAR)adr;

	int nRc = -1;

	nRc = _UsbBulkOutIn(EP1OUT, 0x02, outBuffer, 2, EP1IN, (PUCHAR)inBuffer, 1, NULL);
    if (nRc == IBSU_STATUS_OK)
    {
	    *val = inBuffer[0];
    }
    else
    {
        *val = 0;
    }

	return nRc;
}

int CMainCapture::_IsNeedInitializeCIS_CurveSETi()
{
	return TRUE;
}
