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


const USHORT WM_DAC_Table[7] = { 0x7D, 0x72, 0x63, 0x51, 0x49, 0x42, 0x38 };

////////////////////////////////////////////////////////////////////////////////
// Implementation

BOOL CMainCapture::_GetOneFrameImage_Watson(unsigned char *Image, int ImgSize)
{
    //////////////////////////////////////////////////////////////////////////////
    // one frame read
    int			i;
    LONG		lActualBytes = 0;
	int			img_pos, TotallActualBytes, remain_size;
    int			nRc;

    TotallActualBytes = 0;
	for (i = 0; TotallActualBytes < ImgSize; i++)
    {
		img_pos = 0;
		nRc = _UsbBulkIn(EP6IN, &m_pAlgo->m_Inter_Img[img_pos], 1024*1024, &lActualBytes);

		if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1))
		{
			return FALSE;
		}
		TotallActualBytes = TotallActualBytes + lActualBytes;

		img_pos = 1024*1024;
		remain_size = ImgSize - 1024*1024;
		nRc = _UsbBulkIn(EP6IN, &m_pAlgo->m_Inter_Img[img_pos], remain_size, &lActualBytes);

		if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1))
		{
			return FALSE;
		}
		TotallActualBytes = TotallActualBytes + lActualBytes;

		if (TotallActualBytes == ImgSize)
        {
            break;
        }
	}

	if(m_propertyInfo.bEnableEncryption)
	{
		//unsigned char* temp[CIS_IMG_SIZE];
		//clock_t start = clock();
		m_pAlgo->m_AESEnc->AES_Decrypt_opt(AES_KEY_ENCRYPTION, &m_pAlgo->m_Inter_Img[2], ImgSize, &Image[2]);

		//clock_t end = clock();
		//_PostTraceLogCallback(0, "-----------> AES_Decrypt elapsed time : %d ms", (end - start));
	}
	else
		memcpy(Image, m_pAlgo->m_Inter_Img, ImgSize);

    return TRUE;
}

int CMainCapture::_InitializeForCISRegister_Watson(WORD rowStart, WORD colStart, WORD width, WORD height)
{
    DWORD size;
	UCHAR org[512], enc[512], dec[512];
	int FPGAVersion;
	

    size = width * height;

	
    //-----------------------------------------------------
    if (_FPGA_SetRegister(0x00, 0x01) != IBSU_STATUS_OK) goto done;	// set reset
    if (_FPGA_SetRegister(0x01, 0x00) != IBSU_STATUS_OK) goto done;	// clear capture flag
    Sleep(100);
    if (_ResetFifo() != IBSU_STATUS_OK) goto done;

	
	

    if (_FPGA_SetRegister(0x07, (UCHAR)(size & 0xFF)) != IBSU_STATUS_OK) goto done;		// frame_size
    if (_FPGA_SetRegister(0x08, (UCHAR)((size >> 8) & 0xFF)) != IBSU_STATUS_OK) goto done;
    if (_FPGA_SetRegister(0x09, (UCHAR)((size >> 16) & 0xFF)) != IBSU_STATUS_OK) goto done;

    if (_FPGA_SetRegister(0x00, 0x00) != IBSU_STATUS_OK) goto done;	// clear reset

    //-----------------------------------------------------
    //	센서 초기화 - 제조사로 부터 받음

    // [Reset]
    if (_MT9M_SetRegister(MT9M_RESET,				0x0001) != IBSU_STATUS_OK) goto done;	// Set Reset
    if (_MT9M_SetRegister(MT9M_RESET,				0x0000) != IBSU_STATUS_OK) goto done;	// Clear Reset

    if (_MT9M_SetRegister(MT9M_READ_MODE_2,			0x0047) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_CLAMP_VOLTAGE,		0x0000) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_ADC_REF_LO_VOLTAGE,	0x0003) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_ADC_REF_HI_VOLTAGE,	0x0003) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_RD_EN_VLN_SH_TIMING,	0x0000) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_READ_MODE_1,			0x8006) != IBSU_STATUS_OK) goto done;	// Set parallel mode
    if (_MT9M_SetRegister(MT9M_FORMATTER_2,			0x3070) != IBSU_STATUS_OK) goto done;	// Parallel data and clock out
    if (_MT9M_SetRegister(MT9M_FORMATTER_1,			0x111E) != IBSU_STATUS_OK) goto done;	// FV_LV timing adjustment
    if (_MT9M_SetRegister(MT9M_RESTART,				0x0001) != IBSU_STATUS_OK) goto done;
    Sleep(20);
    if (_MT9M_SetRegister(MT9M_RESTART,				0x0000) != IBSU_STATUS_OK) goto done;

    // PLL Setting
    if (_MT9M_SetRegister(MT9M_PLL_CONTROL,			0x0051) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_PLL_CONTROL,			0x0050) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_PLL_CONTROL,			0x0051) != IBSU_STATUS_OK) goto done;
    //	_MT9M_SetRegister(MT9M_PLL_CONFIG_1,		0x2800);	//PLL Divider = 0x2800	// 80 Mhz 27.5fps
    if (_MT9M_SetRegister(MT9M_PLL_CONFIG_1,		0x5501) != IBSU_STATUS_OK) goto done;	// 85 Mhz --> ok

    Sleep(10);
    if (_MT9M_SetRegister(MT9M_PLL_CONTROL,			0x0053) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_GREEN1_GAIN,			0x0010) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_BLUE_GAIN,			0x0010) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_RED_GAIN,			0x0010) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_GREEN2_GAIN,			0x0010) != IBSU_STATUS_OK) goto done;

    // [Still - 1024 x 972]
    if (_MT9M_SetRegister(MT9M_ROW_START,	(WORD)rowStart) != IBSU_STATUS_OK) goto done;		// 0x66 for 972, 0x36 for 1080
    if (_MT9M_SetRegister(MT9M_COLUMN_START,	(WORD)colStart) != IBSU_STATUS_OK) goto done;		// 0xF0 for 1024, 0xB0 for 1152
    if (_MT9M_SetRegister(MT9M_ROW_SIZE,	(WORD)(height - 1)) != IBSU_STATUS_OK) goto done;		// 0x03CB = 971, 0x0437 = 1079
    if (_MT9M_SetRegister(MT9M_COLUMN_SIZE,	(WORD)(width - 1)) != IBSU_STATUS_OK) goto done;		// 0x03FF = 1023, 0x047F = 1151
    if (_MT9M_SetRegister(MT9M_HORIZONTAL_BLANK,	0x0161) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_VERTICAL_BLANK,		0x07B4) != IBSU_STATUS_OK) goto done;

    if (_MT9M_SetRegister(0x22,						0x0000) != IBSU_STATUS_OK) goto done;	// Row Mode = 0x0
	if (_MT9M_SetRegister(0x23,						0x0000) != IBSU_STATUS_OK) goto done;	// Column Mode = 0x0
	if (_MT9M_SetRegister(MT9M_SHUTTER_WIDTH_UPPER,	0x0000) != IBSU_STATUS_OK) goto done;
	if (_MT9M_SetRegister(MT9M_SHUTTER_WIDTH_LOWER,	0x0320) != IBSU_STATUS_OK) goto done;	// 1353
	if (_MT9M_SetRegister(MT9M_SHUTTER_DELAY,		0x0000) != IBSU_STATUS_OK) goto done;

	// recommended setting
	if (m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON_MINI)
	{
		if (_MT9M_SetRegister(MT9M_READ_MODE_2,			0xC047) != IBSU_STATUS_OK) goto done;    // Watson Mini need mirror
	}
	else
	{
		if (_MT9M_SetRegister(MT9M_READ_MODE_2,			0x0047) != IBSU_STATUS_OK) goto done;    // 0x0047
	}

	if (_MT9M_SetRegister(MT9M_CLAMP_VOLTAGE,		0x0000) != IBSU_STATUS_OK) goto done;
	if (_MT9M_SetRegister(MT9M_ADC_REF_LO_VOLTAGE,	0x0003) != IBSU_STATUS_OK) goto done;
	if (_MT9M_SetRegister(MT9M_ADC_REF_HI_VOLTAGE,	0x0003) != IBSU_STATUS_OK) goto done;
	if (_MT9M_SetRegister(MT9M_RD_EN_VLN_SH_TIMING,	0x0000) != IBSU_STATUS_OK) goto done;
	if (_MT9M_SetRegister(MT9M_READ_MODE_1,			0x8006) != IBSU_STATUS_OK) goto done;	// Set parallel mode
	if (_MT9M_SetRegister(MT9M_FORMATTER_2,			0x3070) != IBSU_STATUS_OK) goto done;	// Parallel data and clock out
	if (_MT9M_SetRegister(MT9M_FORMATTER_1,			0x111E) != IBSU_STATUS_OK) goto done;	// FV_LV timing adjustment
	if (_MT9M_SetRegister(MT9M_RESTART,				0x0001) != IBSU_STATUS_OK) goto done;
	Sleep(20);
	if (_MT9M_SetRegister(MT9M_RESTART,				0x0000) != IBSU_STATUS_OK) goto done;

	// integration time.
	if (_MT9M_SetRegister(MT9M_SHUTTER_WIDTH_LOWER,	0x0BB8) != IBSU_STATUS_OK) goto done;
	if (_MT9M_SetRegister(MT9M_SHUTTER_WIDTH_UPPER,	0x0000) != IBSU_STATUS_OK) goto done;
	if (_MT9M_SetRegister(MT9M_SHUTTER_DELAY,		0x0000) != IBSU_STATUS_OK) goto done;

	// RGB gain
	//	_MT9M_SetRegister(MT9M_GLOBAL_GAIN,			0x0071);

	Sleep(100);
	if (_FPGA_SetRegister(0x01, 0x01) != IBSU_STATUS_OK) goto done;	// set capture flag

	//memcpy(buff, (PUCHAR)AES_KEY_ENCRYPTION, 256);

	FPGAVersion = _GetFpgaVersion();
	
	if(FPGAVersion >= _FPGA_VER_FOR_WATSON_MINI_ENCRYPTION)
	{
		memset(org, 0, 512);
		memset(enc, 0, 512);
		memset(dec, 0, 512);

		memcpy(org, (PUCHAR)AES_KEY_ENCRYPTION, 256);


		CIBEncryption *RSA = new CIBEncryption(AES_KEY_ENCRYPTION);
		RSA->AES_Key_Encrypt(org, enc);
		delete RSA;
		_AES_KeyTransfer_For_Watson(enc, 512);
	}

	////////////////////////////////
	/* AES KEY TRANSFER - Waiting */
	////////////////////////////////
	// Wait until receiving is done
	if(FPGAVersion >= _FPGA_VER_FOR_WATSON_MINI_ENCRYPTION)
	{
		int		trial = 30;
		UCHAR	state = 0;

		for(; trial >=0; trial--)
		{
			Sleep(100);
			_FPGA_GetRegister( 0x32, &state);
			if(state==1)
				break;
		}

		char str[100];

		if(state == 1)
		{
			sprintf(str,"Encryption Initialization succeeded.");

		}
		else
		{
			sprintf(str,"Encryption Initialization failed.");
		}
		TRACE(str);
		_PostTraceLogCallback(0, str);

		// Set 0x30 back from 1 to  0
		_FPGA_SetRegister( 0x30, 0x00 );


		if(m_propertyInfo.bEnableEncryption == FALSE)
			_FPGA_SetRegister( 0x33, 0x00 );
		else
			_FPGA_SetRegister( 0x33, 0x01 );
	}

    return IBSU_STATUS_OK;

done:
    return IBSU_ERR_DEVICE_IO;
}

int	CMainCapture::Capture_SetLEVoltage_Watson(int voltageValue)
{
	switch(voltageValue)
	{
        case 9:
            return _SetLEVoltage(7);
        case 10:
            return _SetLEVoltage(9);
	default:
		return _SetLEVoltage(voltageValue);
	}
}

int	CMainCapture::Capture_SetLEVoltage_WatsonMini(int voltageValue)
{
    if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_WATSON_MINI_DAC)
	{
		_FPGA_SetVoltage(0x01);

		switch (voltageValue)
		{
		case 0:
			return _SetLEVoltage(WM_DAC_Table[0]);
		case 1:
			return _SetLEVoltage(WM_DAC_Table[0]);
		case 2:
			return _SetLEVoltage(WM_DAC_Table[0]);
		case 3:
			return _SetLEVoltage(WM_DAC_Table[0]);
		case 4:
			return _SetLEVoltage(WM_DAC_Table[0]);
		case 5:
			return _SetLEVoltage(WM_DAC_Table[0]);
		case 6:
			return _SetLEVoltage(WM_DAC_Table[0]);
		case 7:
			return _SetLEVoltage(WM_DAC_Table[0]);
		case 8:
			return _SetLEVoltage(WM_DAC_Table[0]);
		case 9:
			return _SetLEVoltage(WM_DAC_Table[0]);
		case 10:
			return _SetLEVoltage(WM_DAC_Table[1]);
		case 11:
			return _SetLEVoltage(WM_DAC_Table[2]);
		case 12:
			return _SetLEVoltage(WM_DAC_Table[3]);
		case 13:
			return _SetLEVoltage(WM_DAC_Table[4]);
		case 14:
			return _SetLEVoltage(WM_DAC_Table[5]);
		case 15:
			return _SetLEVoltage(WM_DAC_Table[6]);
		default:
			return IBSU_ERR_INVALID_PARAM_VALUE;
		}
	}
	else
	{
	    switch (voltageValue)
	    {
	        case 0:
	            return _FPGA_SetVoltage(0x01);
	        case 1:
	            return _FPGA_SetVoltage(0x01);
	        case 2:
	            return _FPGA_SetVoltage(0x01);
	        case 3:
	            return _FPGA_SetVoltage(0x01);
	        case 4:
	            return _FPGA_SetVoltage(0x01);
	        case 5:
	            return _FPGA_SetVoltage(0x01);
	        case 6:
	            return _FPGA_SetVoltage(0x01);
	        case 7:
	            return _FPGA_SetVoltage(0x01);
	        case 8:
	            return _FPGA_SetVoltage(0x01);
	        case 9:
	            return _FPGA_SetVoltage(0x01);
	        case 10:
	            return _FPGA_SetVoltage(0x02);
	        case 11:
	            return _FPGA_SetVoltage(0x04);
	        case 12:
	            return _FPGA_SetVoltage(0x08);
	        case 13:
	            return _FPGA_SetVoltage(0x10);
	        case 14:
	            return _FPGA_SetVoltage(0x20);
	        case 15:
	            return _FPGA_SetVoltage(0x40);
	        default:
	            return IBSU_ERR_INVALID_PARAM_VALUE;
	    }
	}
}

int	CMainCapture::Capture_GetLEVoltage_Watson(int *voltageValue)
{
//	int nRc;
	int ReadVoltage;

    _GetLEVoltage(&ReadVoltage);

    switch(ReadVoltage)
	{
        case 7:
            return *voltageValue = 9;
        case 9:
            return *voltageValue = 10;
	default:
		return *voltageValue = ReadVoltage;
	}
}

int	CMainCapture::Capture_GetLEVoltage_WatsonMini(int *voltageValue)
{
    int nRc;
    BYTE ReadVoltage;
	int	 LEReadVoltage;

	if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_WATSON_MINI_DAC)
	{
		nRc = _GetLEVoltage(&LEReadVoltage);

		if(LEReadVoltage == WM_DAC_Table[0])
		{
			*voltageValue = 9;
		}
		else if(LEReadVoltage == WM_DAC_Table[1])
		{
			*voltageValue = 10;
		}
		else if(LEReadVoltage == WM_DAC_Table[2])
		{
			*voltageValue =11;
		}
		else if(LEReadVoltage == WM_DAC_Table[3])
		{
			*voltageValue =12;
		}
		else if(LEReadVoltage == WM_DAC_Table[4])
		{
			*voltageValue =13;
		}
		else if(LEReadVoltage == WM_DAC_Table[5])
		{
			*voltageValue =14;
		}
		else if(LEReadVoltage == WM_DAC_Table[6])
		{
			*voltageValue =15;
		}
		else
		{
			*voltageValue = 0;
		}
	}
	else
	{
	    nRc = _FPGA_GetVoltage(&ReadVoltage);
	
	    switch (ReadVoltage)
	    {
	        case 0x01:
	            *voltageValue = 9;
	            break;
	        case 0x02:
	            *voltageValue = 10;
	            break;
	        case 0x04:
	            *voltageValue = 11;
	            break;
	        case 0x08:
	            *voltageValue = 12;
	            break;
	        case 0x10:
	            *voltageValue = 13;
	            break;
	        case 0x20:
	            *voltageValue = 14;
	            break;
	        case 0x40:
	            *voltageValue = 15;
	            break;
	        default:
	            *voltageValue = 0;
	            break;
	    }
	}

    return nRc;
}

int CMainCapture::_SetLEOperationMode_Watson(WORD addr)
{
    UCHAR		outBuffer[64] = {0};

    /*	outBuffer[0] = (UCHAR)addr;

    	return _UsbBulkOutIn( EP1OUT, CMD_WRITE_LE_MODE, outBuffer, 1, -1, NULL, 0, NULL );
    */
    //////////////////////////////////////////////////////////////////
    // enzyme modify 2012-11-26 Bug fixed on Watson Mini
    if (m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON_MINI)
    {
        if ((IBSU_LEOperationMode)addr == ENUM_IBSU_LE_OPERATION_ON)
        {
            return _FPGA_SetRegister(0xA0, m_propertyInfo.nVoltageValue);
        }
        else if ((IBSU_LEOperationMode)addr == ENUM_IBSU_LE_OPERATION_OFF)
        {
            return _FPGA_SetRegister(0xA0, 0x00);
        }
        else
        {
            return IBSU_ERR_CHANNEL_IO_COMMAND_FAILED;
        }
    }
    else
    {
        outBuffer[0] = (UCHAR)addr;
        return _UsbBulkOutIn(EP1OUT, CMD_WRITE_LE_MODE, outBuffer, 1, -1, NULL, 0, NULL);
    }
    //////////////////////////////////////////////////////////////////
}

AcuisitionState CMainCapture::_GoJob_PreImageProcessing_Watson(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage)
{
	BOOL tmpbIsGoodImage = FALSE;
    *bIsGoodImage = FALSE;

    // WATSON & WATSON MINI
    m_pAlgo->_Algo_Process_AnalysisImage(InImg);
    m_pAlgo->_Algo_RemoveNoise(InImg, OutImg, m_UsbDeviceInfo.CisImgSize, m_propertyInfo.nContrastValue);
	m_pAlgo->m_cImgAnalysis.isDetected = m_pAlgo->_Algo_GetBrightWithRawImage_forDetectOnly(OutImg, 100);
    m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_GetBrightWithRawImage(OutImg, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);

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

		if (m_propertyInfo.bStartRollWithoutLock == TRUE &&
			m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
			if(m_pAlgo->m_rollingStatus < 2)
			{
				*bIsGoodImage = FALSE;
				m_nGoodFrameCount = 0;
				m_pAlgo->NewRoll_Init_Rolling();
				m_pAlgo->m_rollingStatus = 0;
			}
		}
    }
    else if (m_propertyInfo.nNumberOfObjects > m_pAlgo->m_cImgAnalysis.finger_count)
    {
        m_clbkProperty.nFingerState = ENUM_IBSU_TOO_FEW_FINGERS;

		if (m_propertyInfo.bStartRollWithoutLock == TRUE &&
			m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
			if(m_pAlgo->m_rollingStatus < 2)
			{
				*bIsGoodImage = FALSE;
				m_nGoodFrameCount = 0;
				m_pAlgo->NewRoll_Init_Rolling();
				m_pAlgo->m_rollingStatus = 0;
			}
		}
    }
    else if (m_propertyInfo.nNumberOfObjects == m_pAlgo->m_cImgAnalysis.finger_count)
    {
        m_clbkProperty.nFingerState = ENUM_IBSU_FINGER_COUNT_OK;

		if (m_propertyInfo.bStartRollWithoutLock &&
			m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
			*bIsGoodImage = TRUE;
			if(m_nGoodFrameCount <= 1)
			{
				m_nGoodFrameCount = 3;
				m_pAlgo->NewRoll_Init_Rolling();
				_PostCallback(CALLBACK_TAKING_ACQUISITION);
		        m_pAlgo->m_rollingStatus = 1;
			}
		}
    }
    else
    {
        m_clbkProperty.nFingerState = ENUM_IBSU_TOO_MANY_FINGERS;

		if (//m_propertyInfo.bStartRollWithoutLock == TRUE &&
			m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
			if(m_pAlgo->m_rollingStatus > 0)
			{
				_PostCallback(CALLBACK_NOTIFY_MESSAGE, 0, 0, 0, IBSU_WRN_MULTIPLE_FINGERS_DURING_ROLL);
			}
			
			*bIsGoodImage = FALSE;
			m_nGoodFrameCount = 0;
			m_pAlgo->NewRoll_Init_Rolling();
			m_pAlgo->m_rollingStatus = 0;
		}
    }

    if (!m_bFirstSent_clbkClearPlaten && m_SavedFingerState != m_clbkProperty.nFingerState)
    {
        _PostCallback(CALLBACK_FINGER_COUNT);
    }
    ///////////////////////////////////////////////////////////////////////////

    if ((m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CONTRAST) == IBSU_OPTION_AUTO_CONTRAST)
    {
        if (m_UsbDeviceInfo.LeVoltageControl == SUPPORT_LE_VOLTAGE_CONTROL)
        {
            //tmpbIsGoodImage = m_pAlgo->_Algo_Watson_AutoCaptureByVoltage(OutImg, m_propertyInfo.nCaptureBrightThresFlat[1], m_propertyInfo.nCaptureBrightThresFlat[0], 
			//					&m_propertyInfo, (int)(m_clbkProperty.imageInfo.FrameTime * 1000));
			tmpbIsGoodImage = m_pAlgo->_Algo_Watson_AutoCaptureByExposure(OutImg, m_propertyInfo.nCaptureBrightThresFlat[1], m_propertyInfo.nCaptureBrightThresFlat[0], 
								&m_propertyInfo, (int)(m_clbkProperty.imageInfo.FrameTime * 1000));
            ////////////////////////////////////////////////////////////////
            // enzyme add 2012-11-26 bug fixed
            if (m_propertyInfo.nLEOperationMode != ENUM_IBSU_LE_OPERATION_OFF)
            {
                Capture_SetLEVoltage(m_propertyInfo.nVoltageValue);
            }
            ////////////////////////////////////////////////////////////////
            _Set_CIS_GainRegister(m_propertyInfo.nContrastValue, TRUE);
			_MT9M_SetRegister(MT9M_SHUTTER_WIDTH_LOWER,	m_propertyInfo.nExposureValue);
        }
        else
        {
            tmpbIsGoodImage = m_pAlgo->_Algo_Watson_AutoCaptureByGain(OutImg, m_propertyInfo.nCaptureBrightThresFlat[1], m_propertyInfo.nCaptureBrightThresFlat[0],
								&m_propertyInfo, (int)(m_clbkProperty.imageInfo.FrameTime * 1000));
            _Set_CIS_GainRegister(m_propertyInfo.nContrastValue, TRUE);
        }
    }
    else
    {
        tmpbIsGoodImage = m_pAlgo->_Algo_ManualCapture(m_propertyInfo.nCaptureBrightThresFlat[1], m_propertyInfo.nCaptureBrightThresFlat[0],
								&m_propertyInfo, (int)(m_clbkProperty.imageInfo.FrameTime * 1000));
        _Set_CIS_GainRegister(m_propertyInfo.nContrastValue, FALSE);
    }

	if(m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER ||
		m_propertyInfo.bStartRollWithoutLock == FALSE)
		*bIsGoodImage = tmpbIsGoodImage;

    IBSU_FingerQualityState savedQualityArray[4];
    memcpy(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray));
	m_pAlgo->_Algo_JudgeFingerQuality(&m_pAlgo->m_cImgAnalysis, bIsGoodImage, &m_propertyInfo, &m_clbkProperty, m_DisplayWindow.dispInvalidArea); //Modify Sean to check finger in invalid area

    if (!m_bFirstSent_clbkClearPlaten)
        if (memcmp(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray)) != 0)
        {
            _PostCallback(CALLBACK_FINGER_QUALITY);
        }

    return ACUISITION_NONE;
}

void CMainCapture::_PostImageProcessing_ForPreview_Watson(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage)
{
	if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER &&
		m_propertyInfo.bRollImageOverride == TRUE )
	{
		m_pAlgo->m_cImgAnalysis.LEFT = 1;
		m_pAlgo->m_cImgAnalysis.RIGHT = m_UsbDeviceInfo.CisImgWidth-2;
		m_pAlgo->m_cImgAnalysis.TOP = 1;
		m_pAlgo->m_cImgAnalysis.BOTTOM = m_UsbDeviceInfo.CisImgHeight-2;

		memcpy(m_pAlgo->m_Inter_Img5, m_pAlgo->m_capture_rolled_local_best_buffer, m_UsbDeviceInfo.CisImgSize);
		for(int i=0; i<m_UsbDeviceInfo.CisImgSize; i++)
		{
			if(m_pAlgo->m_capture_rolled_local_best_buffer[i] < InImg[i])
				m_pAlgo->m_Inter_Img5[i] = InImg[i];
		}

		memcpy(InImg, m_pAlgo->m_Inter_Img5, m_UsbDeviceInfo.CisImgSize);
	}
	
	if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
		m_pAlgo->_Algo_GetFingerCount(0);

#if defined(__IBSCAN_SDK__)
#if defined(__embedded__)
    m_pAlgo->_Algo_Fast_Distortion(InImg, OutImg);
#else
    if (m_propertyInfo.bEnableFastFrameMode && m_propertyInfo.bRawCaptureStart)
    {
		m_pAlgo->_Algo_Fast_Distortion(InImg, m_pAlgo->m_Inter_Img2);
		m_pAlgo->_Algo_MakeWatson_250DPI(m_pAlgo->m_Inter_Img2, OutImg);
    }
    else
    {
        m_pAlgo->_Algo_VignettingEffect(InImg, m_pAlgo->m_Inter_Img2);
        m_pAlgo->_Algo_RemoveVignettingNoise(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img3);
        m_pAlgo->_Algo_Image_Gamma(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2, m_nGammaLevel);
        m_pAlgo->_Algo_DistortionRestoration_with_Bilinear(m_pAlgo->m_Inter_Img2, OutImg);
    }
#endif
#elif defined(__IBSCAN_ULTIMATE_SDK__)
	switch (m_propertyInfo.nImageProcessThres)
    {
	case IMAGE_PROCESS_THRES_LOW:
		m_pAlgo->_Algo_Fast_Distortion(InImg, OutImg);
		break;
	case IMAGE_PROCESS_THRES_MEDIUM:
	    m_pAlgo->_Algo_VignettingEffect(InImg, m_pAlgo->m_Inter_Img2);
		m_pAlgo->_Algo_DistortionRestoration_with_Bilinear(m_pAlgo->m_Inter_Img2, OutImg);
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgSize);
		break;
	case IMAGE_PROCESS_THRES_HIGH:
		m_pAlgo->_Algo_VignettingEffect(InImg, m_pAlgo->m_Inter_Img2);
		m_pAlgo->_Algo_RemoveVignettingNoise(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img3);
		m_pAlgo->_Algo_Image_Gamma(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2, m_nGammaLevel);
		m_pAlgo->_Algo_DistortionRestoration_with_Bilinear(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img);

		if (bIsGoodImage)
		{
			m_pAlgo->_Algo_HistogramNormalize(m_pAlgo->m_Inter_Img, OutImg, m_UsbDeviceInfo.ImgWidth * m_UsbDeviceInfo.ImgHeight, m_propertyInfo.nContrastTres);
			m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgWidth * m_UsbDeviceInfo.ImgHeight);
		}
		else
		{
			m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_Inter_Img, OutImg, m_UsbDeviceInfo.ImgWidth * m_UsbDeviceInfo.ImgHeight);
		}
		break;
	default:
		return;
	}
#endif
}

void CMainCapture::_PostImageProcessing_ForResult_Watson(BYTE *InImg, BYTE *OutImg)
{
    int expect_contrast;

    m_pAlgo->_Algo_VignettingEffect(InImg, m_pAlgo->m_Inter_Img2);
    m_pAlgo->_Algo_RemoveVignettingNoise(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img3);
    expect_contrast = m_pAlgo->_Algo_FindExpectContrast(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.CisImgSize, m_propertyInfo.nContrastTres, m_nGammaLevel);
    m_pAlgo->_Algo_Image_Gamma(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2, expect_contrast);
    m_pAlgo->_Algo_DistortionRestoration_with_Bilinear(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img);
    m_pAlgo->_Algo_HistogramNormalize(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.ImgSize, m_propertyInfo.nContrastTres);
    m_pAlgo->_Algo_SwUniformity(m_pAlgo->m_Inter_Img2, OutImg, m_pAlgo->m_Inter_Img3, m_UsbDeviceInfo.ImgWidth, m_UsbDeviceInfo.ImgHeight);

#if defined(__IBSCAN_ULTIMATE_SDK__)
    // Algo split image
	memset(m_pAlgo->m_OutSplitResultArray, 0x00, m_UsbDeviceInfo.ImgSize*IBSU_MAX_SEGMENT_COUNT);
	memset(m_pAlgo->m_OutSplitResultArrayEx, 0x00, m_UsbDeviceInfo.ImgSize*IBSU_MAX_SEGMENT_COUNT);
	m_pAlgo->_Algo_SegmentFinger(OutImg);

    // Convert to Splited image
	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitResultArray, m_pAlgo->m_OutSplitResultArray, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize);
	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitResultArrayEx, m_pAlgo->m_OutSplitResultArrayEx, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize);
    // Convert to Result image
	m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);
#endif
}

int CMainCapture::_GoJob_DummyCapture_ForThread_Watson(BYTE *InImg)
{
    int			finger_count;

    m_pAlgo->_Algo_Process_AnalysisImage(InImg);
    m_pAlgo->_Algo_RemoveNoise(InImg, m_pAlgo->m_Inter_Img, m_UsbDeviceInfo.CisImgHeight * m_UsbDeviceInfo.CisImgWidth, m_propertyInfo.nContrastValue);
    m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_GetBrightWithRawImage(m_pAlgo->m_Inter_Img, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);

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

void CMainCapture::_GoJob_Initialize_ForThread_Watson()
{
    int WaterImageSize = m_UsbDeviceInfo.nWaterImageSize;

//    BYTE *WaterImage = new BYTE[WaterImageSize + 4 /* +4 is checksum */];
    BYTE *WaterImage = m_pAlgo->m_WaterImage;

	m_UsbDeviceInfo.nFpgaVersion = _GetFpgaVersion(FALSE);

    memset(WaterImage, 0, WaterImageSize);
    if (m_UsbDeviceInfo.bNeedMask)
    {
	    if( !(m_UsbDeviceInfo.devType == DEVICE_TYPE_WATSON && 
		    strcmp(m_propertyInfo.cFirmware, "0.11.0") <= 0) )
	    {
		    if( !_Algo_GetWaterMaskFromFlashMemory(WaterImage, WaterImageSize) )
		    {
			    memset(WaterImage, 0, WaterImageSize);
		    }
	    }
    }

    /* This must be done before making the uniformity mask. */
    m_pAlgo->_Algo_Watson_GetDistortionMask_FromSavedFile();

    m_pAlgo->_Algo_MakeUniformityMask(WaterImage);
//    delete [] WaterImage;

    m_pAlgo->_Algo_Init_GammaTable();
}

int	CMainCapture::Reserved_GetEnhancdImage_Watson(const IBSU_ImageData inImage,
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

	switch (inImage.ProcessThres)
	{
	case IMAGE_PROCESS_THRES_LOW:
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite((unsigned char*)inImage.Buffer, Inter_Img1, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);
		m_pAlgo->_Algo_GetBrightWithRawImageEnhanced(Inter_Img1, &imgAnalysis);
		m_pAlgo->_Algo_VignettingEffectEnhanced(Inter_Img1, Inter_Img2, imgAnalysis);
		m_pAlgo->_Algo_RemoveVignettingNoiseEnhanced(Inter_Img2, Inter_Img1, Inter_Img3, imgAnalysis);
		break;
	case IMAGE_PROCESS_THRES_MEDIUM:
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite((unsigned char*)inImage.Buffer, Inter_Img2, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);
		m_pAlgo->_Algo_GetBrightWithRawImageEnhanced(Inter_Img2, &imgAnalysis);
		m_pAlgo->_Algo_RemoveVignettingNoiseEnhanced(Inter_Img2, Inter_Img1, Inter_Img3, imgAnalysis);
		break;
	case IMAGE_PROCESS_THRES_HIGH:
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite((unsigned char*)inImage.Buffer, Inter_Img1, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);
		m_pAlgo->_Algo_GetBrightWithRawImageEnhanced(Inter_Img1, &imgAnalysis);
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

int CMainCapture::_IsNeedInitializeCIS_Watson()
{
	WORD reg_val[4];
	if (_MT9M_GetRegister(MT9M_ROW_START,	&reg_val[0]) != IBSU_STATUS_OK) goto done;		// 0x66 for 972, 0x36 for 1080
	if (_MT9M_GetRegister(MT9M_COLUMN_START,	&reg_val[1]) != IBSU_STATUS_OK) goto done;	// 0xF0 for 1024, 0xB0 for 1152
	if (_MT9M_GetRegister(MT9M_ROW_SIZE,	&reg_val[2]) != IBSU_STATUS_OK) goto done;		// 0x03CB = 971, 0x0437 = 1079
	if (_MT9M_GetRegister(MT9M_COLUMN_SIZE,	&reg_val[3]) != IBSU_STATUS_OK) goto done;		// 0x03FF = 1023, 0x047F = 1151

	if(reg_val[0] != 0x0056 ||
		reg_val[1] != 0x00D0 ||
		reg_val[2] != 0x03FD ||
		reg_val[3] != 0x043F)
		return TRUE;
done:
	return FALSE;
}



int CMainCapture::_AES_KeyTransfer_For_Watson(unsigned char* key, int size)
{
	int nRc = IBSU_STATUS_OK;

	//////////////////////////////
	/* AES KEY TRANSFER - START */
	//////////////////////////////
	//if(m_pAlgo != NULL)
	{
		BOOL bResult;

		// EP2 SWITCH
		_FPGA_SetRegister( 0x30, 0x01 );
		
		
		//m_pAlgo->m_AESEnc->AES_Get_AES_KEY_ALL(packet_buf, 512);

	#if 1
		// Send AES KEY via USB EP2
		{
//			bResult = m_pUsbDevice->WritePipe(EP2OUT, key, size, &cbSent, timeout);
			bResult = _UsbBulkOut(EP2OUT, key[0], &key[1], size-1, 1000);
			if(bResult != IBSU_STATUS_OK)
			{
				TRACE("AES_SET_KEY ERROR!\n");
				return FALSE;
			}
		}
	#endif

		// Notice Key transfering is completed
		_FPGA_SetRegister( 0x31, 0x01 );
		_FPGA_SetRegister( 0x31, 0x00 );
	}
	////////////////////////////
	/* AES KEY TRANSFER - END */
	////////////////////////////
	return nRc;
}