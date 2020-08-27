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

BOOL CMainCapture::_GetOneFrameImage_Columbo(unsigned char *Image, int ImgSize)
{
    //////////////////////////////////////////////////////////////////////////////
    // one frame read
    int			i, val;
    LONG		lActualBytes = 0;
    int			nRc;
    int			nDecimatedImgSize = ImgSize;

    UCHAR			outBuffer[64] = {0};

    outBuffer[0] = (UCHAR)(0);		// Gain

    if (m_UsbDeviceInfo.bDecimation == TRUE)
    {
        if (m_UsbDeviceInfo.nDecimation_Mode == DECIMATION_2X)
        {
            nDecimatedImgSize = ImgSize / 4;
        }
        else
        {
            nDecimatedImgSize = ImgSize / 16;
        }
    }

    // We must use _UsbBulkOut(), because [...]
    nRc = _UsbBulkOut(EP1OUT, CMD_GET_ONE_FRAME, outBuffer, 1);

    for (i = 0; lActualBytes < nDecimatedImgSize; i++)
    {
#if defined(__ppi__)
        if (m_bPPIMode)
        {
		    nRc = _PPI_OneFrameBulkRead(Image, nDecimatedImgSize, &lActualBytes);
        }
        else
        {
            nRc = _UsbBulkIn(EP6IN, Image, nDecimatedImgSize, &lActualBytes);
        }
#else
        nRc = _UsbBulkIn(EP6IN, Image, nDecimatedImgSize, &lActualBytes);
#endif
        if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1))
        {
            return FALSE;
        }

        if (lActualBytes == nDecimatedImgSize)
        {
            break;
        }
        else
        {
#ifdef __G_DEBUG__
            TRACE(_T("Get OneFrameImage = %d(%d)\n"), lActualBytes, nRc);
#endif
            // Packet was disappeared. It's firmware bug, but re-try to get one frame image.
            // We must use _UsbBulkOut(), because [...]
            nRc = _UsbBulkOut(EP1OUT, CMD_GET_ONE_FRAME, outBuffer, 1);
        }
    }

	for(i=0;i<nDecimatedImgSize;i++)
	{
		val = Image[i] - 5;
		if(val < 0)
			val = 0;
		else if(val > 210)
			val = 210;
		Image[i] = val * 255 / 210;
	}

    return TRUE;
}

int CMainCapture::_InitializeForCISRegister_Columbo(WORD rowStart, WORD colStart, WORD width, WORD height)
{
	WORD temp;
	
    if (m_UsbDeviceInfo.nFpgaVersion == -1)
    {
        m_UsbDeviceInfo.nFpgaVersion = _GetFpgaVersion();
    }

    Sleep(10);			// for more safe
    //		_ResetFifo();
    if (_UsbBulkOutIn(EP1OUT, CMD_RESET_FIFO, NULL, 0, -1, NULL, 0, NULL) != IBSU_STATUS_OK) goto done;
    Sleep(10);			// for more safe

    if (_Seti_SetRegister(0x03, 0x02) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x04, 0x83) != IBSU_STATUS_OK) goto done; //X,Y flip
    if (_Seti_SetRegister(0x05, 0x0a) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x08, 0x00) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x09, 0x11) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x0a, 0x11) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x10, 0x10) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x11, 0x20) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x12, 0x88) != IBSU_STATUS_OK) goto done;	//To improve brightness
    if (_Seti_SetRegister(0x13, 0x09) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x14, 0x00) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x16, 0xb6) != IBSU_STATUS_OK) goto done;

//	temp = (m_UsbDeviceInfo.nUsbSpeed < USB_HIGH_SPEED) ? 0x0120u : 0x04FFu;	
	temp = 0x04FFu;	
    if (_Seti_SetRegister(0X30, (temp / 256u)) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x31, (temp % 256u)) != IBSU_STATUS_OK) goto done;
	
    if (_Seti_SetRegister(0x32, 0x20) != IBSU_STATUS_OK) goto done;

    if (_Seti_SetRegister(0x41, 0x01) != IBSU_STATUS_OK) goto done; //Horizontal Start H
    if (_Seti_SetRegister(0x42, 0x67) != IBSU_STATUS_OK) goto done; //Horizontal Start L
    if (_Seti_SetRegister(0x43, 0x03) != IBSU_STATUS_OK) goto done; //Horizontal Width H
    if (_Seti_SetRegister(0x44, 0x6F) != IBSU_STATUS_OK) goto done; //Horizontal Width L

    if (_Seti_SetRegister(0x45, 0x00) != IBSU_STATUS_OK) goto done; //Vertical Start H
    if (_Seti_SetRegister(0x46, 0x32) != IBSU_STATUS_OK) goto done; //Vertical Start L
    if (_Seti_SetRegister(0x47, 0x04) != IBSU_STATUS_OK) goto done; //Vertical Height H
    if (_Seti_SetRegister(0x48, 0x4B) != IBSU_STATUS_OK) goto done; //Vertical Height L

    ////////////////////////////////////////////////////////////////////////////
    // Important! To make the frame rate
    // Control for VSYNC blank time
	// 0x0370 - 10fps; 0x0230 - 12fps
//	temp = (m_UsbDeviceInfo.nUsbSpeed < USB_HIGH_SPEED) ? 0x01A0u : 0x0000u;	
	temp = 0x0000u;	
    if (_Seti_SetRegister(0X49, (temp / 256u)) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x4A, (temp % 256u)) != IBSU_STATUS_OK) goto done;
    ////////////////////////////////////////////////////////////////////////////
    if (_Seti_SetRegister(0x4B,    0x00) != IBSU_STATUS_OK) goto done; //?
    if (_Seti_SetRegister(0x4C,    0x00) != IBSU_STATUS_OK) goto done; //?
  
//	temp = (m_UsbDeviceInfo.nUsbSpeed < USB_HIGH_SPEED) ? 0x07FFu : 0x00032u;	
	temp = 0x00032u;	
    if (_Seti_SetRegister(0X4D, (temp / 256u)) != IBSU_STATUS_OK) goto done; // H blank H
    if (_Seti_SetRegister(0x4E, (temp % 256u)) != IBSU_STATUS_OK) goto done; // H blank L

    if (_Seti_SetRegister(0x60,	   0x0b) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x61,    0x16) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x62,    0x32) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x71,    0x08) != IBSU_STATUS_OK) goto done;

    if (_Seti_SetRegister(0x80,    0xE0) != IBSU_STATUS_OK) goto done; //IPFUN Ori : e8
    if (_Seti_SetRegister(0x81,    0x0F) != IBSU_STATUS_OK) goto done; //SIGCNT Ori : 07

    if (_Seti_SetRegister(0x90,    0x00) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x91,    0x08) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x92,    0x10) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x93,    0x40) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x94,    0x04) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x95,    0x01) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x96,    0x80) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x97,    0x08) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x98,    0x10) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x99,    0x08) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x9a,    0x03) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x9b,    0xb0) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x9c,    0x08) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x9d,    0x24) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x9e,    0x30) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0xb8,	   0x28) != IBSU_STATUS_OK) goto done;
    if (_Seti_SetRegister(0x03,    0x05) != IBSU_STATUS_OK) goto done;

    return IBSU_STATUS_OK;

done:
    return IBSU_ERR_DEVICE_IO;
}

int	CMainCapture::Capture_SetLEVoltage_Columbo(int voltageValue)
{
    if (m_UsbDeviceInfo.nFpgaVersion >= ((1 << 16) | (1 << 8) | 2))
    {
        // This is for new transformer
        switch (voltageValue)
        {
            case 11:
                return _SetLEVoltage(0x78);
            case 12:
                return _SetLEVoltage(0x6F);
            case 13:
                return _SetLEVoltage(0x66);
            case 14:
                return _SetLEVoltage(0x5D);
            case 15:
                return _SetLEVoltage(0x54);

            default:
                return IBSU_ERR_INVALID_PARAM_VALUE;
        }
    }

    // This is only for HW ver1 because it does not have capacitor
    // So it will fixed next HW version.
    switch (voltageValue)
    {
        case 0:
            return _SetLEVoltage(0x83);             // Do not use
        case 1:
            return _SetLEVoltage(0x82);             // Do not use
        case 2:
            return _SetLEVoltage(0x81);             // Do not use
        case 3:
            return _SetLEVoltage(0x80);             // Do not use
        case 4:
            return _SetLEVoltage(0x7F);             // Do not use
        case 5:
            return _SetLEVoltage(0x7E);             // Do not use
        case 6:
            return _SetLEVoltage(0x7D);             // Do not use
        case 7:
            return _SetLEVoltage(0x7C);             // Do not use
        case 8:
            return _SetLEVoltage(0x7B);             // Do not use
        case 9:
            return _SetLEVoltage(0x7A);             // Do not use
        case 10:
            return _SetLEVoltage(0x79);             // Do not use
        case 11:
            return _SetLEVoltage(0x78);
        case 12:
            return _SetLEVoltage(0x68);
        case 13:
            return _SetLEVoltage(0x58);
        case 14:
            return _SetLEVoltage(0x48);
        case 15:
            return _SetLEVoltage(0x41);

        default:
            return IBSU_ERR_INVALID_PARAM_VALUE;
    }
}

int	CMainCapture::Capture_GetLEVoltage_Columbo(int *voltageValue)
{
    int nRc;
    int readVoltage;

    nRc = _GetLEVoltage(&readVoltage);

	// This includes both values for HW version 1 and HW with new transformer.
    switch (readVoltage)
    {
        case 0x83:
            *voltageValue = 0;
            break;
        case 0x82:
            *voltageValue = 1;
            break;
        case 0x81:
            *voltageValue = 2;
            break;
        case 0x80:
            *voltageValue = 3;
            break;
        case 0x7F:
            *voltageValue = 4;
            break;
        case 0x7E:
            *voltageValue = 5;
            break;
        case 0x7D:
            *voltageValue = 6;
            break;
        case 0x7C:
            *voltageValue = 7;
            break;
        case 0x7B:
            *voltageValue = 8;
            break;
        case 0x7A:
            *voltageValue = 9;
            break;
        case 0x79:
            *voltageValue = 10;
            break;
        case 0x78:
            *voltageValue = 11;
            break;
        case 0x6F: // New transformer
        case 0x68:
            *voltageValue = 12;
            break;
		case 0x66: // New transformer     
        case 0x58:
            *voltageValue = 13;
            break;
        case 0x5D: // New transformer
        case 0x48:
            *voltageValue = 14;
            break;
        case 0x54: // New transformer
        case 0x41:
            *voltageValue = 15;
            break;
        default:
            *voltageValue = 0;
            break;
    }

    return nRc;
}

int CMainCapture::_SetLEOperationMode_Columbo(WORD addr)
{
    UCHAR outBuffer[64] = {0};

    outBuffer[0] = (UCHAR)addr;
    return _UsbBulkOutIn(EP1OUT, CMD_WRITE_LE_MODE, outBuffer, 1, -1, NULL, 0, NULL);
}

AcuisitionState CMainCapture::_GoJob_PreImageProcessing_Columbo(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage)
{
    /*	*bIsGoodImage = FALSE;

    	memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize);
    	return ACUISITION_NONE;
    */
    *bIsGoodImage = FALSE;

    if (m_UsbDeviceInfo.bDecimation == TRUE)
    {
        m_pAlgo->_Algo_Process_AnalysisImage_250DPI_Columbo(InImg);
        m_pAlgo->_Algo_RemoveNoise_250DPI_Columbo(InImg, OutImg, m_propertyInfo.nContrastValue);
        m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Columbo_GetBrightWithRawImage_250DPI_Columbo(OutImg, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
    }
    else
    {
        m_pAlgo->_Algo_Process_AnalysisImage(InImg);
        m_pAlgo->_Algo_RemoveNoise(InImg, OutImg, m_UsbDeviceInfo.CisImgSize, m_propertyInfo.nContrastValue);
        m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Columbo_GetBrightWithRawImage(OutImg, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Need check
    // enzyme 2013-02-04 delete - Replaced orginal routine becasue we can find exact detected finger area
    //	m_pAlgo->m_cImgAnalysis.TOP = 1;
    //	m_pAlgo->m_cImgAnalysis.LEFT = 1;
    //	m_pAlgo->m_cImgAnalysis.RIGHT = m_UsbDeviceInfo.CisImgWidth - 2;
    //	m_pAlgo->m_cImgAnalysis.BOTTOM = m_UsbDeviceInfo.CisImgHeight - 2;
    ////////////////////////////////////////////////////////////////////////////


    // 지문 개수 루틴
    m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(m_pAlgo->m_cImgAnalysis.mean);

    if (m_nFrameCount > 0 && m_pAlgo->m_cImgAnalysis.finger_count == 0)
    {
        m_clbkProperty.nPlatenState = ENUM_IBSU_PLATEN_CLEARD;
    }

    if (m_bFirstSent_clbkClearPlaten && m_clbkProperty.nPlatenState == ENUM_IBSU_PLATEN_CLEARD)
    {
        m_bFirstSent_clbkClearPlaten = FALSE;
        _PostCallback(CALLBACK_CLEAR_PLATEN);
    }

    ///////////////////////////////////////////////////////////////////////////
    // 2013-05-28 enzyme modify - change varient to understand better
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
    ///////////////////////////////////////////////////////////////////////////

    if ((m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CONTRAST) == IBSU_OPTION_AUTO_CONTRAST)
    {
#if defined(WINCE)
//        *bIsGoodImage = m_pAlgo->_Algo_Columbo_AutoCapture(OutImg, m_propertyInfo.nCaptureBrightThresFlat[1]+10, m_propertyInfo.nCaptureBrightThresFlat[0]-5, 
//							&m_propertyInfo, (int)(m_clbkProperty.imageInfo.FrameTime * 1000));
        *bIsGoodImage = m_pAlgo->_Algo_Columbo_AutoCaptureByExposure(OutImg, m_propertyInfo.nCaptureBrightThresFlat[1]+10, m_propertyInfo.nCaptureBrightThresFlat[0]-5, 
							&m_propertyInfo, (int)(m_clbkProperty.imageInfo.FrameTime * 1000));
#else
//        *bIsGoodImage = m_pAlgo->_Algo_Columbo_AutoCapture(OutImg, m_propertyInfo.nCaptureBrightThresFlat[1], m_propertyInfo.nCaptureBrightThresFlat[0], 
//							&m_propertyInfo, (int)(m_clbkProperty.imageInfo.FrameTime * 1000));
        *bIsGoodImage = m_pAlgo->_Algo_Columbo_AutoCaptureByExposure(OutImg, m_propertyInfo.nCaptureBrightThresFlat[1], m_propertyInfo.nCaptureBrightThresFlat[0], 
							&m_propertyInfo, (int)(m_clbkProperty.imageInfo.FrameTime * 1000));
#endif
        ////////////////////////////////////////////////////////////////
        // enzyme add 2012-11-26 bug fixed
        if (m_propertyInfo.nLEOperationMode != ENUM_IBSU_LE_OPERATION_OFF)
        {
            Capture_SetLEVoltage(m_propertyInfo.nVoltageValue);
        }
        ////////////////////////////////////////////////////////////////
        _Set_CIS_GainRegister(m_propertyInfo.nContrastValue, TRUE);
		_Seti_SetRegister(0X30, (m_propertyInfo.nExposureValue / 256u));
		_Seti_SetRegister(0x31, (m_propertyInfo.nExposureValue % 256u));
    }
    else
    {
        *bIsGoodImage = m_pAlgo->_Algo_ManualCapture(m_propertyInfo.nCaptureBrightThresFlat[1], m_propertyInfo.nCaptureBrightThresFlat[0], 
							&m_propertyInfo, (int)(m_clbkProperty.imageInfo.FrameTime * 1000));
        _Set_CIS_GainRegister(m_propertyInfo.nContrastValue, FALSE);
    }

    IBSU_FingerQualityState savedQualityArray[4];
    memcpy(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray));
	m_pAlgo->_Algo_JudgeFingerQuality(&m_pAlgo->m_cImgAnalysis, bIsGoodImage, &m_propertyInfo, &m_clbkProperty, m_DisplayWindow.dispInvalidArea);

    if (!m_bFirstSent_clbkClearPlaten)
        if (memcmp(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray)) != 0)
        {
            _PostCallback(CALLBACK_FINGER_QUALITY);
        }

    return ACUISITION_NONE;

}

void CMainCapture::_PostImageProcessing_ForPreview_Columbo(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage)
{
	switch (m_propertyInfo.nImageProcessThres)
	{
	case IMAGE_PROCESS_THRES_LOW:
    if (m_UsbDeviceInfo.bDecimation == TRUE)
    {
			m_pAlgo->_Algo_Fast_Distortion_250DPI_Columbo(InImg, OutImg);
    }
    else
    {
			m_pAlgo->_Algo_Fast_Distortion_Columbo(InImg, OutImg);
    }
		break;
	case IMAGE_PROCESS_THRES_MEDIUM:
    if (m_UsbDeviceInfo.bDecimation == TRUE)
    {
			m_pAlgo->_Algo_VignettingEffect_250DPI_Columbo(InImg, m_pAlgo->m_Inter_Img2);
			m_pAlgo->_Algo_Columbo_DistortionRestoration_with_Bilinear_250DPI_Columbo(m_pAlgo->m_Inter_Img2, OutImg);
			m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgWidth * m_UsbDeviceInfo.ImgHeight);
    }
    else
    {
			m_pAlgo->_Algo_VignettingEffect(InImg, m_pAlgo->m_Inter_Img2);
			m_pAlgo->_Algo_Columbo_DistortionRestoration_with_Bilinear(m_pAlgo->m_Inter_Img2, OutImg);
			m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgWidth * m_UsbDeviceInfo.ImgHeight);
    }
		break;
	case IMAGE_PROCESS_THRES_HIGH:
    _UpdateGammaLevel(__AUTO_COLUMBO_CONTRAST_MAX_VALUE__);
    if (m_UsbDeviceInfo.bDecimation == TRUE)
    {
        m_pAlgo->_Algo_VignettingEffect_250DPI_Columbo(InImg, m_pAlgo->m_Inter_Img2);
        m_pAlgo->_Algo_RemoveVignettingNoise_250DPI_Columbo(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img3);
        m_pAlgo->_Algo_Image_Gamma_250DPI_Columbo(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img3, m_nGammaLevel);
        m_pAlgo->_Algo_Columbo_DistortionRestoration_with_Bilinear_250DPI_Columbo(m_pAlgo->m_Inter_Img3, m_pAlgo->m_Inter_Img2);
    }
    else
    {
        m_pAlgo->_Algo_VignettingEffect(InImg, m_pAlgo->m_Inter_Img2);
        m_pAlgo->_Algo_RemoveVignettingNoise(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img3);
        m_pAlgo->_Algo_Image_Gamma(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img3, m_nGammaLevel);
        m_pAlgo->_Algo_Columbo_DistortionRestoration_with_Bilinear(m_pAlgo->m_Inter_Img3, m_pAlgo->m_Inter_Img2);
    }

    if (bIsGoodImage)
    {
        m_pAlgo->_Algo_HistogramNormalize(m_pAlgo->m_Inter_Img2, OutImg, m_UsbDeviceInfo.ImgWidth * m_UsbDeviceInfo.ImgHeight, m_propertyInfo.nContrastTres);
        m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgWidth * m_UsbDeviceInfo.ImgHeight);
    }
    else
    {
        m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_Inter_Img2, OutImg, m_UsbDeviceInfo.ImgWidth * m_UsbDeviceInfo.ImgHeight);
    }
		break;
	default:
		return;
	}
}

void CMainCapture::_PostImageProcessing_ForResult_Columbo(BYTE *InImg, BYTE *OutImg)
{
    m_pAlgo->_Algo_VignettingEffect(InImg, m_pAlgo->m_Inter_Img2);
    m_pAlgo->_Algo_RemoveVignettingNoise(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img3);
    int expect_contrast = m_pAlgo->_Algo_FindExpectContrast(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img3, m_UsbDeviceInfo.CisImgSize, m_propertyInfo.nContrastTres, m_nGammaLevel);
    m_pAlgo->_Algo_Image_Gamma(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2, expect_contrast);
    m_pAlgo->_Algo_Columbo_DistortionRestoration_with_Bilinear(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img3);

	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_Inter_Img2, m_pAlgo->m_final_image, m_UsbDeviceInfo.CisImgSize);

    m_pAlgo->_Algo_HistogramNormalize(m_pAlgo->m_Inter_Img3, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.ImgSize, m_propertyInfo.nContrastTres);
    m_pAlgo->_Algo_SwUniformity(m_pAlgo->m_Inter_Img2, OutImg, m_pAlgo->m_Inter_Img3, m_UsbDeviceInfo.ImgWidth, m_UsbDeviceInfo.ImgHeight);

    // Algo split image : always 1
	memset(m_pAlgo->m_OutSplitResultArray, 0x00, m_UsbDeviceInfo.ImgSize*IBSU_MAX_SEGMENT_COUNT);
	memset(m_pAlgo->m_OutSplitResultArrayEx, 0x00, m_UsbDeviceInfo.ImgSize*IBSU_MAX_SEGMENT_COUNT);
	m_pAlgo->_Algo_SegmentFinger(OutImg);

    // Convert to Splited image
	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitResultArray, m_pAlgo->m_OutSplitResultArray, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize);
	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitResultArrayEx, m_pAlgo->m_OutSplitResultArrayEx, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize);
    // Convert to Result image
	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);

#ifdef _WINDOWS
	if(m_propertyInfo.bIsSpoofSupported == TRUE &&
		m_propertyInfo.bEnableSpoof == TRUE)
	{
		nex_sdk_load_image_bytes(OutImg, m_UsbDeviceInfo.ImgHeight, m_UsbDeviceInfo.ImgWidth);
		int Score = nex_sdk_get_score(NEX_SDK_CLASSIFIER_MLP);

		if(Score < m_propertyInfo.nSpoofLevel * 100)
		{
			TRACE("score(%d) / SpoofLevel(%d)\n", Score, m_propertyInfo.nSpoofLevel);
			m_UsbDeviceInfo.bSpoofDetected = TRUE;
		}
	}
#endif 
}

int CMainCapture::_GoJob_DummyCapture_ForThread_Columbo(BYTE *InImg)
{
    int			finger_count;

    if (m_UsbDeviceInfo.bDecimation == TRUE)
    {
        m_pAlgo->_Algo_Process_AnalysisImage_250DPI_Columbo(InImg);
        m_pAlgo->_Algo_RemoveNoise_250DPI_Columbo(InImg, m_pAlgo->m_Inter_Img, m_propertyInfo.nContrastValue);
        m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Columbo_GetBrightWithRawImage_250DPI_Columbo(m_pAlgo->m_Inter_Img, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
    }
    else
    {
        m_pAlgo->_Algo_Process_AnalysisImage(InImg);
        m_pAlgo->_Algo_RemoveNoise(InImg, m_pAlgo->m_Inter_Img, m_UsbDeviceInfo.CisImgSize, m_propertyInfo.nContrastValue);
        m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Columbo_GetBrightWithRawImage(m_pAlgo->m_Inter_Img, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
    }

    // 지문 개수 루틴
    finger_count = m_pAlgo->_Algo_GetFingerCount(m_pAlgo->m_cImgAnalysis.mean);
    if (finger_count == 0)
    {
		std::vector<FingerQuality *>::iterator it = m_pAlgo->m_pFingerQuality.begin();
		while( it != m_pAlgo->m_pFingerQuality.end() )
		{
			if( *it )
			{
				delete *it;
				it = m_pAlgo->m_pFingerQuality.erase(it);
			}
			else
				++it;
		}

		m_pAlgo->m_pFingerQuality.clear();
        return 1;
    }

    return 0;
}

int CMainCapture::_Seti_SetRegister(UCHAR adr, UCHAR val)
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};

    outBuffer[0] = (UCHAR)adr;
    outBuffer[1] = (UCHAR)val;

    int nRc = -1;

    //return _UsbBulkOutIn( EP1OUT, CMD_WRITE_SETI_REG, outBuffer, 2, -1, NULL, 0, NULL );
    nRc = _UsbBulkOutIn(EP1OUT, CMD_WRITE_SETI_REG, outBuffer, 2, -1, NULL, 0, NULL);

    return nRc;

}

void CMainCapture::_GoJob_Initialize_ForThread_Columbo()
{
    BOOL  isOK = FALSE;
    int   WaterImageSize = m_UsbDeviceInfo.nWaterImageSize;
//    BYTE *WaterImage = NULL;

//    WaterImage = new BYTE[WaterImageSize + 8 /* +4 is checksum */];
    BYTE *WaterImage = m_pAlgo->m_WaterImage;

    m_UsbDeviceInfo.nFpgaVersion = _GetFpgaVersion();

    memset(WaterImage, 0, WaterImageSize);
    if (m_UsbDeviceInfo.bNeedMask)
    {
    /* Check if version number is less than 1.1.0 and serial number begins with "SAMPLE". */
    if ((m_UsbDeviceInfo.nFpgaVersion < ((1 << 16) | (1 << 8) | 0)) && (strncmp(m_propertyInfo.cSerialNumber, "SAMPLE", 6) == 0))
    {
        if (m_bUniformityMaskPath)
        {
            FILE *fp;
            fp = fopen(m_cUniformityMaskPath, "rb");
            if (fp != NULL)
            {
                const size_t maskSize = m_UsbDeviceInfo.CisImgSize + 8;  // Size+2 is compatible with original version of Columbo um files. Only Size+1 is needed.
                size_t       readSize;

                readSize = fread(WaterImage, 1, maskSize, fp);
                fclose(fp);

                if (readSize == maskSize)
                {
                    m_pAlgo->_Algo_Decrypt_Simple(WaterImage, WaterImage, maskSize);
                    if (m_pAlgo->_Algo_ConfirmChecksum((DWORD *)WaterImage, maskSize / sizeof(DWORD)))
                    {
                        isOK = TRUE;
                    }
                }
            }
        }

        if (!isOK)
        {
            memset(WaterImage, 0, WaterImageSize);
        }
    }
    else
    {
        if (!_Algo_Columbo_GetWaterMaskFromFlashMemory(WaterImage, WaterImageSize))
        {
            memset(WaterImage, 0, WaterImageSize);
        }
    }
    }

    /* This must be done before making the uniformity mask. */
    m_pAlgo->_Algo_Columbo_GetDistortionMask_FromSavedFile();

    m_pAlgo->_Algo_MakeUniformityMask(WaterImage);
//    delete [] WaterImage;

    m_pAlgo->_Algo_Init_GammaTable();
}

int CMainCapture::_SetDecimation_Columbo(BOOL enable)
{
    UCHAR outBuffer[2];

    outBuffer[0] = 0x05;
    if (enable)
    {
        outBuffer[1] = 0x01;
    }
    else
    {
        outBuffer[1] = 0x00;
    }

    return _UsbBulkOutIn(EP1OUT, CMD_WRITE_FPGA_REG, outBuffer, 2, -1, NULL, 0, NULL);
}

int CMainCapture::_SetDecimationMode_Columbo(DecimationMode mode)
{
    if (mode == DECIMATION_NONE)
    {
        return IBSU_ERR_INVALID_PARAM_VALUE;
    }

    UCHAR outBuffer[2];

    outBuffer[0] = 0x06;
    outBuffer[1] = (UCHAR)mode;

    return _UsbBulkOutIn(EP1OUT, CMD_WRITE_FPGA_REG, outBuffer, 2, -1, NULL, 0, NULL);
}

int	CMainCapture::Reserved_GetEnhancdImage_Columbo(const IBSU_ImageData inImage,
										   IBSU_ImageData *enhancedImage,
										   int *segmentImageArrayCount,
										   IBSU_ImageData *segmentImageArray,
										   IBSU_SegmentPosition *segmentPositionArray)
{
	int nRc = IBSU_STATUS_OK;
#if defined(__IBSCAN_ULTIMATE_SDK__)
	int expect_contrast;
	FrameImgAnalysis imgAnalysis;
	int nDetectedFingerCount = 0;
	NEW_SEGMENT_ARRAY m_new_segment_arr;

	memset(&imgAnalysis, 0, sizeof(imgAnalysis));

	BYTE *Inter_Img1 = new BYTE [m_pAlgo->CIS_IMG_SIZE];
	BYTE *Inter_Img2 = new BYTE [m_pAlgo->CIS_IMG_SIZE];
	BYTE *Inter_Img3 = new BYTE [m_pAlgo->CIS_IMG_SIZE];

//	memcpy(&imgAnalysis, &m_pAlgo->m_cImgAnalysis, sizeof(FrameImgAnalysis));

	switch (inImage.ProcessThres)
	{
	case IMAGE_PROCESS_THRES_LOW:
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite((unsigned char*)inImage.Buffer, Inter_Img1, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);
		m_pAlgo->_Algo_Columbo_GetBrightWithRawImageEnhanced(Inter_Img1, &imgAnalysis);
		m_pAlgo->_Algo_VignettingEffectEnhanced(Inter_Img1, Inter_Img2, imgAnalysis);
		m_pAlgo->_Algo_RemoveVignettingNoiseEnhanced(Inter_Img2, Inter_Img1, Inter_Img3, imgAnalysis);
		break;
	case IMAGE_PROCESS_THRES_MEDIUM:
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite((unsigned char*)inImage.Buffer, Inter_Img2, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);
		m_pAlgo->_Algo_Columbo_GetBrightWithRawImageEnhanced(Inter_Img2, &imgAnalysis);
		m_pAlgo->_Algo_RemoveVignettingNoiseEnhanced(Inter_Img2, Inter_Img1, Inter_Img3, imgAnalysis);
		break;
	case IMAGE_PROCESS_THRES_HIGH:
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite((unsigned char*)inImage.Buffer, Inter_Img1, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);
		m_pAlgo->_Algo_Columbo_GetBrightWithRawImageEnhanced(Inter_Img1, &imgAnalysis);
		break;
	case IMAGE_PROCESS_THRES_PERFECT:
		nRc = IBSU_WRN_ALREADY_ENHANCED_IMAGE;
		goto done;
	default:
		nRc = IBSU_ERR_INVALID_PARAM_VALUE;
		goto done;
	}
		
	expect_contrast = m_pAlgo->_Algo_FindExpectContrastEnhanced(Inter_Img1, Inter_Img2, m_UsbDeviceInfo.ImgSize, m_propertyInfo.nContrastTres, m_nGammaLevel, imgAnalysis);
	m_pAlgo->_Algo_Image_GammaEnhanced(Inter_Img1, Inter_Img2, expect_contrast, imgAnalysis);
	m_pAlgo->_Algo_HistogramNormalizeEnhanced(Inter_Img2, Inter_Img1, m_UsbDeviceInfo.ImgSize, m_propertyInfo.nContrastTres, imgAnalysis);
	m_pAlgo->_Algo_SwUniformityEnhanced(Inter_Img1, m_pAlgo->m_OutEnhancedImg, Inter_Img3, m_UsbDeviceInfo.ImgWidth, m_UsbDeviceInfo.ImgHeight, imgAnalysis);
	// Algo split image
	memset(m_pAlgo->m_OutSplitEnhancedArrayEx, 0x00, m_UsbDeviceInfo.ImgSize*IBSU_MAX_SEGMENT_COUNT);
	nDetectedFingerCount = m_pAlgo->_Algo_SegmentFingerEnhanced(m_pAlgo->m_OutEnhancedImg, &m_new_segment_arr, m_pAlgo->m_OutSplitEnhancedArray, m_pAlgo->m_OutSplitEnhancedArrayEx,
						m_pAlgo->m_OutSplitEnhancedArrayExWidth, m_pAlgo->m_OutSplitEnhancedArrayExHeight, 
						m_pAlgo->m_OutSplitEnhancedArrayExSize);

	// Convert to Splited image
	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitEnhancedArrayEx, m_pAlgo->m_OutSplitEnhancedArrayEx, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize);
	// Convert to Result image
	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutEnhancedImg, m_pAlgo->m_OutEnhancedImg, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);

	memcpy(enhancedImage, &inImage, sizeof(IBSU_ImageData));
	enhancedImage->Buffer = (void*)m_pAlgo->m_OutEnhancedImg;
	enhancedImage->ProcessThres = IMAGE_PROCESS_THRES_PERFECT;

	*segmentImageArrayCount = nDetectedFingerCount;
	// For segment images
	for( int i=0; i<nDetectedFingerCount; i++ )
	{
		int x1, y1, x2, y2, x3, y3, x4, y4;
		memcpy(&segmentImageArray[i], &inImage, sizeof(IBSU_ImageData));
		segmentImageArray[i].Width = m_pAlgo->m_OutSplitEnhancedArrayExWidth[i];
		segmentImageArray[i].Height = m_pAlgo->m_OutSplitEnhancedArrayExHeight[i];
		segmentImageArray[i].Pitch = -m_pAlgo->m_OutSplitEnhancedArrayExWidth[i];
		segmentImageArray[i].Buffer = (BYTE*)&m_pAlgo->m_OutSplitEnhancedArrayEx[i*m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight];

		x1 = (m_new_segment_arr.Segment[i].P1_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
		y1 = (m_new_segment_arr.Segment[i].P1_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

		x2 = (m_new_segment_arr.Segment[i].P2_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
		y2 = (m_new_segment_arr.Segment[i].P2_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

		x3 = (m_new_segment_arr.Segment[i].P3_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
		y3 = (m_new_segment_arr.Segment[i].P3_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

		x4 = (m_new_segment_arr.Segment[i].P4_X-m_pAlgo->ENLARGESIZE_ZOOM_W) * m_pAlgo->IMG_W / m_pAlgo->ZOOM_W;
		y4 = (m_new_segment_arr.Segment[i].P4_Y-m_pAlgo->ENLARGESIZE_ZOOM_H) * m_pAlgo->IMG_H / m_pAlgo->ZOOM_H;

		segmentPositionArray[i].x1 = x1;
		segmentPositionArray[i].x2 = x2;
		segmentPositionArray[i].x3 = x3;
		segmentPositionArray[i].x4 = x4;
		segmentPositionArray[i].y1 = (m_pAlgo->IMG_H - y1);
		segmentPositionArray[i].y2 = (m_pAlgo->IMG_H - y2);
		segmentPositionArray[i].y3 = (m_pAlgo->IMG_H - y3);
		segmentPositionArray[i].y4 = (m_pAlgo->IMG_H - y4);
	}

done:
	delete [] Inter_Img1;
	delete [] Inter_Img2;
	delete [] Inter_Img3;
#endif
	return nRc;
}

int	CMainCapture::Reserved_GetFinalImageByNative_Columbo(IBSU_ImageData *finalImage)
{
	int nRc = IBSU_STATUS_OK;
#if defined(__IBSCAN_ULTIMATE_SDK__)
	
	if(m_UsbDeviceInfo.devType == DEVICE_TYPE_COLUMBO)
	{
		finalImage->Buffer = m_pAlgo->m_final_image;
		finalImage->BitsPerPixel = 8;
		finalImage->Format = IBSU_IMG_FORMAT_GRAY;
		finalImage->FrameTime = 0;
		finalImage->Width = m_UsbDeviceInfo.CisImgWidth;
		finalImage->Height = m_UsbDeviceInfo.CisImgHeight;
		finalImage->IsFinal = 1;
		finalImage->Pitch = -m_UsbDeviceInfo.CisImgWidth;
		finalImage->ProcessThres = IMAGE_PROCESS_THRES_PERFECT;
		finalImage->ResolutionX = m_UsbDeviceInfo.scanResolutionX;
		finalImage->ResolutionY = m_UsbDeviceInfo.scanResolutionY;
	}

#endif
	return nRc;
}
