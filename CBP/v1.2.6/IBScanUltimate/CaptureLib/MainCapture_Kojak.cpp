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

BOOL CMainCapture::_GetOneFrameImage_Kojak(unsigned char *Image, int ImgSize)
{
	//////////////////////////////////////////////////////////////////////////////
    // one frame read
    int			i;
    LONG		lActualBytes = 0, TotallActualBytes = 0;
    int			nRc = IBSU_STATUS_OK;
	unsigned char *tmpImage = m_pAlgo->m_Inter_Img5;
	int img_pos, remain_size;

	memset(Image, 0, ImgSize);

	if (m_UsbDeviceInfo.bDecimation == TRUE)
	{
		ImgSize = ImgSize / 4;

		for (i = 0; lActualBytes < ImgSize; i++)
		{
			nRc = _UsbBulkIn(EP6IN, tmpImage, ImgSize, &lActualBytes);

			if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1))
			{
				break;//return FALSE;
			}

		   //TRACE("Get OneFrameImage = %d\n", lActualBytes);
			if (lActualBytes < 500)
			{
				break;
			}

//			memcpy(Image, tmpImage, lActualBytes);
		}
	}
	else if (m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		for (i = 0; TotallActualBytes < ImgSize; i++)
		{
			img_pos = 0;
			nRc = _UsbBulkIn(EP6IN, &tmpImage[img_pos], 1024*1024, &lActualBytes);

			if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1))
			{
				break;//return FALSE;
			}
			TotallActualBytes = TotallActualBytes + lActualBytes;

			img_pos = img_pos + 1024*1024;
			remain_size = ImgSize - 1024*1024;
			nRc = _UsbBulkIn(EP6IN, &tmpImage[img_pos], remain_size, &lActualBytes);

			if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1))
			{
				break;//return FALSE;
			}
			TotallActualBytes = TotallActualBytes + lActualBytes;

		   //TRACE("Get OneFrameImage = %d\n", lActualBytes);
			if (lActualBytes < 500)
			{
				break;
			}

//			memcpy(Image, tmpImage, ImgSize);
		}
	}
	else
	{
		for (i = 0; TotallActualBytes < ImgSize; i++)
		{
			img_pos = 0;
			nRc = _UsbBulkIn(EP6IN, &tmpImage[img_pos], 1024*1024, &lActualBytes);

			if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1))
			{
				break;//return FALSE;
			}
			TotallActualBytes = TotallActualBytes + lActualBytes;

			img_pos = img_pos + 1024*1024;
			nRc = _UsbBulkIn(EP6IN, &tmpImage[img_pos], 1024*1024, &lActualBytes);

			if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1))
			{
				break;//return FALSE;
			}
			TotallActualBytes = TotallActualBytes + lActualBytes;

			img_pos = img_pos + 1024*1024;
			nRc = _UsbBulkIn(EP6IN, &tmpImage[img_pos], 1024*1024, &lActualBytes);

			if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1))
			{
				break;//return FALSE;
			}
			TotallActualBytes = TotallActualBytes + lActualBytes;

			img_pos = img_pos + 1024*1024;
			remain_size = m_UsbDeviceInfo.CisImgSize - 1024*1024*3;
			nRc = _UsbBulkIn(EP6IN, &tmpImage[img_pos], remain_size, &lActualBytes);

			if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1))
			{
				break;//return FALSE;
			}
			TotallActualBytes = TotallActualBytes + lActualBytes;

		   //TRACE("Get OneFrameImage = %d\n", lActualBytes);
			if (lActualBytes < 500)
			{
				break;
			}

//			memcpy(Image, tmpImage, ImgSize);
		}
	}

	if(m_propertyInfo.bEnableEncryption)
	{
        if (ImgSize < m_pAlgo->IMG_SIZE)
        {
            int loop = m_pAlgo->IMG_SIZE/ImgSize;
            int remains = m_pAlgo->IMG_SIZE%ImgSize;
            for (int i=0; i<loop; i++)
            {
                memcpy(&m_pAlgo->m_EncryptImage[ImgSize*i], &tmpImage[0], ImgSize);
            }

            if (remains > 0)
            {
                memcpy(&m_pAlgo->m_EncryptImage[ImgSize*loop], &tmpImage[0], remains);
            }
		    memcpy(m_pAlgo->m_EncryptImage500ppi, m_pAlgo->m_EncryptImage, m_pAlgo->IMG_SIZE);
        }
        else
        {
		    memcpy(m_pAlgo->m_EncryptImage, &tmpImage[0], ImgSize);
		    memcpy(m_pAlgo->m_EncryptImage500ppi, m_pAlgo->m_EncryptImage, m_pAlgo->IMG_SIZE);
        }

		m_pAlgo->m_AESEnc->AES_Decrypt_opt(&tmpImage[0], ImgSize, &Image[0]);
		//m_AESEnc->AES_Encrypt(AES_KEY_ENCRYPTION, Image, ImgSize, tmpImage2);
		//IBSU_SaveBitmapImage("D:\\pat_img_dec2.bmp", Image,	m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight, -m_UsbDeviceInfo.CisImgWidth, 500, 500);
	}
	else
		memcpy(Image, tmpImage, ImgSize);


	if(nRc != IBSU_STATUS_OK)
	{
		memset(Image, 0, ImgSize);
		return FALSE;
	}

    return TRUE;
}

int CMainCapture::_InitializeForCISRegister_Full_Kojak(WORD rowStart, WORD colStart, WORD width, WORD height)
{
    DWORD size;
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

	// LE ON
//	_OnOff_LE_Kojak(1);

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
    if (_MT9M_SetRegister(MT9M_PLL_CONFIG_1,		0x1E01) != IBSU_STATUS_OK) goto done;	// 90 Mhz = (12MHz*0x1E)/(0x01+1)(0x0001+1)
	if (_MT9M_SetRegister(MT9M_PLL_CONFIG_2,		0x0001) != IBSU_STATUS_OK) goto done;

    Sleep(10);
    if (_MT9M_SetRegister(MT9M_PLL_CONTROL,			0x0053) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_GREEN1_GAIN,			0x0010) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_BLUE_GAIN,			0x0010) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_RED_GAIN,			0x0010) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_GREEN2_GAIN,			0x0010) != IBSU_STATUS_OK) goto done;

	if (_MT9M_SetRegister(MT9M_ROW_START,			(1944-height)/2+35+m_propertyInfo.sKojakCalibrationInfo.LensPosRow) != IBSU_STATUS_OK) goto done;//+94);   // 54
	if (_MT9M_SetRegister(MT9M_COLUMN_START,        (2592-width)/2+30-m_propertyInfo.sKojakCalibrationInfo.LensPosCol) != IBSU_STATUS_OK) goto done;//+40);    // 32
	if (_MT9M_SetRegister(MT9M_ROW_SIZE,            height-1) != IBSU_STATUS_OK) goto done;   // 1079
	if (_MT9M_SetRegister(MT9M_COLUMN_SIZE,         width-1) != IBSU_STATUS_OK) goto done;    // 1439

	/*_MT9M_SetRegister(MT9M_ROW_START,			0);//(WORD)rowStart);		// 0x66 for 972, 0x36 for 1080
	_MT9M_SetRegister(MT9M_COLUMN_START,		0);//(WORD)colStart);		// 0xF0 for 1024, 0xB0 for 1152
	_MT9M_SetRegister(MT9M_ROW_SIZE,			(WORD)(height-1));		// 0x03CB = 971, 0x0437 = 1079
	_MT9M_SetRegister(MT9M_COLUMN_SIZE,			(WORD)(width-1));		// 0x03FF = 1023, 0x047F = 1151
*/
	// integration time.
//	_MT9M_SetRegister(MT9M_SHUTTER_WIDTH_LOWER,	0x1900);
	if (_MT9M_SetRegister(MT9M_SHUTTER_WIDTH_LOWER,	0x0BB8) != IBSU_STATUS_OK) goto done;	// default = decimation
	if (_MT9M_SetRegister(MT9M_SHUTTER_WIDTH_UPPER,	0x0000) != IBSU_STATUS_OK) goto done;
	if (_MT9M_SetRegister(MT9M_SHUTTER_DELAY,		0x0000) != IBSU_STATUS_OK) goto done;

	if (_MT9M_SetRegister(MT9M_GLOBAL_GAIN,			0x005F) != IBSU_STATUS_OK) goto done;//0x005F);
	
	if (_MT9M_SetRegister(MT9M_RESTART,				0x0001) != IBSU_STATUS_OK) goto done;
	if (_MT9M_SetRegister(MT9M_RESTART,				0x0000) != IBSU_STATUS_OK) goto done;
	if (_MT9M_SetRegister(MT9M_VERTICAL_BLANK,		0x0000) != IBSU_STATUS_OK) goto done;

	if (_MT9M_SetRegister(0x70, 0x00FF) != IBSU_STATUS_OK) goto done; 	// RD_SEQ_LEN
	if (_MT9M_SetRegister(0x71, 0xA700) != IBSU_STATUS_OK) goto done; 	// RD_PTR_TIMING
	if (_MT9M_SetRegister(0x72, 0xA700) != IBSU_STATUS_OK) goto done; 	// RD_ROW_EN_TIMING
	if (_MT9M_SetRegister(0x73, 0x0C00) != IBSU_STATUS_OK) goto done; 	// RD_RST_EN_TIMING
	if (_MT9M_SetRegister(0x74, 0x0600) != IBSU_STATUS_OK) goto done; 	// RD_BOOST_RST_TIMING
	if (_MT9M_SetRegister(0x75, 0x5617) != IBSU_STATUS_OK) goto done; 	// RD_SHR_TIMING
	if (_MT9M_SetRegister(0x76, 0x6B57) != IBSU_STATUS_OK) goto done; 	// RD_TX_EN_TIMING
	if (_MT9M_SetRegister(0x77, 0x6B57) != IBSU_STATUS_OK) goto done; 	// RD_BOOST_TX_TIMING
	if (_MT9M_SetRegister(0x78, 0xA500) != IBSU_STATUS_OK) goto done; 	// RD_SHS_TIMING
	if (_MT9M_SetRegister(0x79, 0xAB00) != IBSU_STATUS_OK) goto done; 	// RD_VCL_COL_EN_TIMING
	if (_MT9M_SetRegister(0x7A, 0xA904) != IBSU_STATUS_OK) goto done; 	// RD_COLCLAMP_TIMING
	if (_MT9M_SetRegister(0x7B, 0xA700) != IBSU_STATUS_OK) goto done; 	// RD_SHVCL_BAR_TIMING
	if (_MT9M_SetRegister(0x7C, 0xA700) != IBSU_STATUS_OK) goto done; 	// RD_VLN_EN_TIMING
	if (_MT9M_SetRegister(0x7E, 0xA900) != IBSU_STATUS_OK) goto done; 	// RD_BOOST_ROW_TIMING
	if (_MT9M_SetRegister(0x7F, 0x0000) != IBSU_STATUS_OK) goto done; 	// RD_EN_VLN_SH_TIMING
	if (_MT9M_SetRegister(0x29, 0x0480) != IBSU_STATUS_OK) goto done; 	// DAC_CONTROL_2_REG
	if (_MT9M_SetRegister(0x3E, 0x80C7) != IBSU_STATUS_OK) goto done; 	// TX_LOW_VOLTAGE
	if (_MT9M_SetRegister(0x3F, 0x0004) != IBSU_STATUS_OK) goto done; 	// RST_LOW_VOLTAGE
	if (_MT9M_SetRegister(0x41, 0x0000) != IBSU_STATUS_OK) goto done; 	// CLAMP_VOLTAGE
	if (_MT9M_SetRegister(0x48, 0x0010) != IBSU_STATUS_OK) goto done; 	// VREF_AMP_BIAS_CURRENT
	if (_MT9M_SetRegister(0x5F, 0x231D) != IBSU_STATUS_OK) goto done; 	// CAL_THRESHOLD   
	if (_MT9M_SetRegister(0x57, 0x0004) != IBSU_STATUS_OK) goto done; 	// ANTI_ECLIPSE_VOLTAGE
	if (_MT9M_SetRegister(0x2A, 0x1086) != IBSU_STATUS_OK) goto done; 	// DAC_CONTROL_3

/*	//==========================================================
	// MT9P031 rolling artifack optimization
	//==========================================================
	if (_MT9M_SetRegister(0x70, 0x00FF) != IBSU_STATUS_OK) goto done; 	// RD_SEQ_LEN
//	_MT9M_SetRegister(0x70, 0x0079); 	// RD_SEQ_LEN
	if (_MT9M_SetRegister(0x71, 0x7800) != IBSU_STATUS_OK) goto done; 	// RD_PTR_TIMING
	if (_MT9M_SetRegister(0x72, 0x7800) != IBSU_STATUS_OK) goto done; 	// RD_ROW_EN_TIMING
	if (_MT9M_SetRegister(0x73, 0x0300) != IBSU_STATUS_OK) goto done; 	// RD_RST_EN_TIMING
	if (_MT9M_SetRegister(0x74, 0x0300) != IBSU_STATUS_OK) goto done; 	// RD_BOOST_RST_TIMING
	if (_MT9M_SetRegister(0x75, 0x3C00) != IBSU_STATUS_OK) goto done; 	// RD_SHR_TIMING
	if (_MT9M_SetRegister(0x76, 0x4E3D) != IBSU_STATUS_OK) goto done; 	// RD_TX_EN_TIMING
	if (_MT9M_SetRegister(0x77, 0x4E3D) != IBSU_STATUS_OK) goto done; 	// RD_BOOST_TX_TIMING
	if (_MT9M_SetRegister(0x78, 0x774F) != IBSU_STATUS_OK) goto done; 	// RD_SHS_TIMING
	if (_MT9M_SetRegister(0x79, 0x7900) != IBSU_STATUS_OK) goto done; 	// RD_VCL_COL_EN_TIMING
	if (_MT9M_SetRegister(0x7A, 0x7900) != IBSU_STATUS_OK) goto done; 	// RD_COLCLAMP_TIMING
	if (_MT9M_SetRegister(0x7B, 0x7800) != IBSU_STATUS_OK) goto done; 	// RD_SHVCL_BAR_TIMING
	if (_MT9M_SetRegister(0x7C, 0x7800) != IBSU_STATUS_OK) goto done; 	// RD_VLN_EN_TIMING
	if (_MT9M_SetRegister(0x7E, 0x7800) != IBSU_STATUS_OK) goto done; 	// RD_BOOST_ROW_TIMING
	if (_MT9M_SetRegister(0x7F, 0x7800) != IBSU_STATUS_OK) goto done; 	// RD_EN_VLN_SH_TIMING
	if (_MT9M_SetRegister(0x29, 0x0481) != IBSU_STATUS_OK) goto done; 	// DAC_CONTROL_2_REG
	if (_MT9M_SetRegister(0x3E, 0x0087) != IBSU_STATUS_OK) goto done; 	// TX_LOW_VOLTAGE
	if (_MT9M_SetRegister(0x3F, 0x0007) != IBSU_STATUS_OK) goto done; 	// RST_LOW_VOLTAGE
	if (_MT9M_SetRegister(0x41, 0x0003) != IBSU_STATUS_OK) goto done; 	// CLAMP_VOLTAGE
	if (_MT9M_SetRegister(0x48, 0x0018) != IBSU_STATUS_OK) goto done; 	// VREF_AMP_BIAS_CURRENT
	if (_MT9M_SetRegister(0x5F, 0x1C16) != IBSU_STATUS_OK) goto done; 	// CAL_THRESHOLD   
	if (_MT9M_SetRegister(0x57, 0x0007) != IBSU_STATUS_OK) goto done; 	// ANTI_ECLIPSE_VOLTAGE
	if (_MT9M_SetRegister(0x2A, 0xFF74) != IBSU_STATUS_OK) goto done; 	// DAC_CONTROL_3
*/
	FPGAVersion = _GetFpgaVersion(TRUE);

	if(FPGAVersion >= _FPGA_VER_FOR_KOJAK_3_0)
	{
		if (_FPGA_SetRegister(0x5C, 0x00) != IBSU_STATUS_OK) goto done;

		// Analog Touch Enable
		if (_FPGA_SetRegister(0x64, 0x01) != IBSU_STATUS_OK) goto done;
	}

	Sleep(100);
    if (_FPGA_SetRegister(0x01, 0x01) != IBSU_STATUS_OK) goto done;	// set capture flag

	if(FPGAVersion >= _FPGA_VER_FOR_KOJAK_3_0)
	{
		m_DeciCaptureSetting.forSuperDry_Mode = m_propertyInfo.nSuperDryMode;

		m_DeciCaptureSetting.DefaultMinDAC = _KOJAK30_DAC_FOR_MINIMUM_;
		m_DeciCaptureSetting.DefaultMaxDAC = _DAC_FOR_MAXIMUM_;
		if(m_DeciCaptureSetting.forSuperDry_Mode)
		{
			m_DeciCaptureSetting.TargetMinBrightness = _TARGET_MIN_BRIGHTNESS_FOR_DRY_;
			m_DeciCaptureSetting.TargetMaxBrightness = _TARGET_MAX_BRIGHTNESS_FOR_DRY_;
		}
		else
		{
			m_DeciCaptureSetting.TargetMinBrightness = m_propertyInfo.nCaptureBrightThresFlat[0];
			m_DeciCaptureSetting.TargetMaxBrightness = m_propertyInfo.nCaptureBrightThresFlat[1];
		}
		m_DeciCaptureSetting.TargetFingerCount = m_propertyInfo.nNumberOfObjects;
		if(m_propertyInfo.nSuperDryMode)
		{
			if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER)
				m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_DRY_ONE_FINGER_;
			else if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS)
				m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_DRY_TWO_FINGER_;
			else
				m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_DRY_FOUR_FINGER_;
		}
		else
		{
			if(m_propertyInfo.bEnableStartingVoltage == TRUE)
			{
				m_DeciCaptureSetting.DefaultDAC = m_propertyInfo.nStartingVoltage;
			}
			else
			{
				if(m_UsbDeviceInfo.bCanUseTOF &&
					m_propertyInfo.bEnableTOF)
				{
					if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
						m_DeciCaptureSetting.DefaultDAC = _KOJAK30_DAC_FOR_NORMAL_FINGER_WITH_TOF_;
					else
						m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FINGER_WITH_TOF_;
				}
				else if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_NEW_BRD_AND_DOUBLE_P_ ||
					m_propertyInfo.bKojakPLwithDPFilm == TRUE)
				{
					if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS || 
						m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
						m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_OF_DOUBLE_P_;
					else
						m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FOUR_FINGER_OF_DOUBLE_P_;
				}
				else
				{
					if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS || 
						m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
						m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_;
					else
						m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FOUR_FINGER_;
				}
			}
		}
		m_DeciCaptureSetting.CurrentDAC = m_DeciCaptureSetting.DefaultDAC;
		m_DeciCaptureSetting.DefaultGain = _DEFAULT_GAIN_FOR_DECI_;
		m_DeciCaptureSetting.CurrentGain = m_DeciCaptureSetting.DefaultGain;
		m_DeciCaptureSetting.DefaultExposure = _KOJAK30_DEFAULT_EXPOSURE_FOR_DECI_;
		m_DeciCaptureSetting.CurrentExposure = m_DeciCaptureSetting.DefaultExposure;
		m_DeciCaptureSetting.DefaultBitshift = _KOJAK30_DEFAULT_BITSHIFT_FOR_DECI_;
		m_DeciCaptureSetting.CurrentBitshift = m_DeciCaptureSetting.DefaultBitshift;
		m_DeciCaptureSetting.DefaultCutThres = _KOJAK30_DEFAULT_CUTTHRES_FOR_DECI_;
		m_DeciCaptureSetting.CurrentCutThres = m_DeciCaptureSetting.DefaultCutThres;
		m_DeciCaptureSetting.AdditionalFrame = 1;
		m_DeciCaptureSetting.RollCaptureMode = FALSE;
		m_DeciCaptureSetting.DetectionFrameCount = 0;
		m_DeciCaptureSetting.Kojak30_PERIOD = _KOJAK30_PERIOD_DECI_;
		m_DeciCaptureSetting.Kojak30_ELON = _KOJAK30_DEFAULT_ELON_DECI_;

		m_FullCaptureSetting = m_DeciCaptureSetting;
		m_FullCaptureSetting.DefaultExposure = _KOJAK30_NO_BIT_EXPOSURE_FOR_FULLFRAME_;
		m_FullCaptureSetting.DefaultBitshift = _KOJAK30_NO_BIT_BITSHIFT_FOR_FULLFRAME_;
		m_FullCaptureSetting.DefaultCutThres = _KOJAK30_NO_BIT_CUTTHRES_FOR_FULLFRAME_;
		m_FullCaptureSetting.CurrentExposure = m_FullCaptureSetting.DefaultExposure;
		m_FullCaptureSetting.CurrentBitshift = m_FullCaptureSetting.DefaultBitshift;
		m_FullCaptureSetting.CurrentCutThres = m_FullCaptureSetting.DefaultCutThres;
		m_FullCaptureSetting.Kojak30_PERIOD = _KOJAK30_PERIOD_FOR_FULLFRAME_;
		m_FullCaptureSetting.Kojak30_ELON = _KOJAK30_DEFAULT_ELON_FOR_FULLFRAME_;

		if (m_UsbDeviceInfo.nFpgaVersion >= ((0 << 24) | (9 << 16) | (0 << 8) | 0x0D))
		{
			m_UsbDeviceInfo.bDecimation = TRUE;
			m_UsbDeviceInfo.nDecimation_Mode = DECIMATION_2X;
		}
		_ChangeDecimationModeForKojak(m_UsbDeviceInfo.bDecimation, &m_DeciCaptureSetting, TRUE);

		if (_FPGA_SetRegister(0x54, 0x01) != IBSU_STATUS_OK) goto done;
		if (_FPGA_SetRegister(0x56, 0x01) != IBSU_STATUS_OK) goto done;
		if (_FPGA_SetRegister(0x55, 0x01) != IBSU_STATUS_OK) goto done;
	}

	if (m_UsbDeviceInfo.bCanUseTOF)
	{
//		TRACE("Current Sensor Initialize - Flat Init\n");

		// TOF Initialize
		_SetPlateTouchOff();
		Sleep(10);
//		_WriteCurrentSensor(0x00, 0xB99D);
//		Sleep(100);
		_WriteCurrentSensor(0x00, 0x3FFF);
		Sleep(10);
		_WriteCurrentSensor(0x01, 0x0000);
		Sleep(10);
		_SetPlateTouchStatus(1);	// Touch Ready
		Sleep(10);
		_SetPlateTouchOn();
		Sleep(300);
		_LE_OFF();
		Sleep(10);
		_SetPlateTouchStatus(0);	// Touch Idle
	}

	if(FPGAVersion >= _FPGA_VER_FOR_KOJAK_3_0)
	{
		if (_FPGA_SetRegister(0x65, 0x43) != IBSU_STATUS_OK) goto done;
		if (_FPGA_SetRegister(0x53, 0x01) != IBSU_STATUS_OK) goto done;
		if (_FPGA_SetRegister(0x53, 0x00) != IBSU_STATUS_OK) goto done;

		if (_FPGA_SetRegister(0x59, 0x01) != IBSU_STATUS_OK) goto done;

		if (_FPGA_SetRegister(0x5A, 0x88) != IBSU_STATUS_OK) goto done;
		if (_FPGA_SetRegister(0x5B, 0x44) != IBSU_STATUS_OK) goto done;
	}
	
	_Initialize_Encryption_For_Kojak();

    return IBSU_STATUS_OK;

done:
    return IBSU_ERR_DEVICE_IO;
}

int CMainCapture::_InitializeForCISRegister_Half_Kojak(WORD rowStart, WORD colStart, WORD width, WORD height)
{
    DWORD size;
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

	// LE ON
//	_OnOff_LE_Kojak(1);

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
    if (_MT9M_SetRegister(MT9M_PLL_CONFIG_1,		0x1E01) != IBSU_STATUS_OK) goto done;	// 90 Mhz = (12MHz*0x1E)/(0x01+1)(0x0001+1)
	if (_MT9M_SetRegister(MT9M_PLL_CONFIG_2,		0x0001) != IBSU_STATUS_OK) goto done;

    Sleep(10);
    if (_MT9M_SetRegister(MT9M_PLL_CONTROL,			0x0053) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_GREEN1_GAIN,			0x0010) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_BLUE_GAIN,			0x0010) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_RED_GAIN,			0x0010) != IBSU_STATUS_OK) goto done;
    if (_MT9M_SetRegister(MT9M_GREEN2_GAIN,			0x0010) != IBSU_STATUS_OK) goto done;

	if (_MT9M_SetRegister(MT9M_ROW_START,			(1944-m_UsbDeviceInfo.CisImgHeight)/2+35 + m_propertyInfo.nStartingPositionOfRollArea*100+m_propertyInfo.sKojakCalibrationInfo.LensPosRow) != IBSU_STATUS_OK) goto done;//+94);   // 54
	if (_MT9M_SetRegister(MT9M_COLUMN_START,        (2592-width)/2+30-m_propertyInfo.sKojakCalibrationInfo.LensPosCol) != IBSU_STATUS_OK) goto done;//+40);    // 32
	if (_MT9M_SetRegister(MT9M_ROW_SIZE,            height-1) != IBSU_STATUS_OK) goto done;   // 1079
	if (_MT9M_SetRegister(MT9M_COLUMN_SIZE,         width-1) != IBSU_STATUS_OK) goto done;    // 1439
	/*_MT9M_SetRegister(MT9M_ROW_START,			0);//(WORD)rowStart);		// 0x66 for 972, 0x36 for 1080
	_MT9M_SetRegister(MT9M_COLUMN_START,		0);//(WORD)colStart);		// 0xF0 for 1024, 0xB0 for 1152
	_MT9M_SetRegister(MT9M_ROW_SIZE,			(WORD)(height-1));		// 0x03CB = 971, 0x0437 = 1079
	_MT9M_SetRegister(MT9M_COLUMN_SIZE,			(WORD)(width-1));		// 0x03FF = 1023, 0x047F = 1151
*/
	// integration time.
	if (_MT9M_SetRegister(MT9M_SHUTTER_WIDTH_LOWER,	0x1000) != IBSU_STATUS_OK) goto done;		//	roll default
	if (_MT9M_SetRegister(MT9M_SHUTTER_WIDTH_UPPER,	0x0000) != IBSU_STATUS_OK) goto done;
	if (_MT9M_SetRegister(MT9M_SHUTTER_DELAY,		0x0000) != IBSU_STATUS_OK) goto done;

	if (_MT9M_SetRegister(MT9M_GLOBAL_GAIN,			0x005F) != IBSU_STATUS_OK) goto done;//0x005F);

	if (_MT9M_SetRegister(MT9M_RESTART,				0x0001) != IBSU_STATUS_OK) goto done;
	if (_MT9M_SetRegister(MT9M_RESTART,				0x0000) != IBSU_STATUS_OK) goto done;
	if (_MT9M_SetRegister(MT9M_VERTICAL_BLANK,		0x0000) != IBSU_STATUS_OK) goto done;

	if (_MT9M_SetRegister(0x70, 0x00FF) != IBSU_STATUS_OK) goto done; 	// RD_SEQ_LEN
	if (_MT9M_SetRegister(0x71, 0xA700) != IBSU_STATUS_OK) goto done; 	// RD_PTR_TIMING
	if (_MT9M_SetRegister(0x72, 0xA700) != IBSU_STATUS_OK) goto done; 	// RD_ROW_EN_TIMING
	if (_MT9M_SetRegister(0x73, 0x0C00) != IBSU_STATUS_OK) goto done; 	// RD_RST_EN_TIMING
	if (_MT9M_SetRegister(0x74, 0x0600) != IBSU_STATUS_OK) goto done; 	// RD_BOOST_RST_TIMING
	if (_MT9M_SetRegister(0x75, 0x5617) != IBSU_STATUS_OK) goto done; 	// RD_SHR_TIMING
	if (_MT9M_SetRegister(0x76, 0x6B57) != IBSU_STATUS_OK) goto done; 	// RD_TX_EN_TIMING
	if (_MT9M_SetRegister(0x77, 0x6B57) != IBSU_STATUS_OK) goto done; 	// RD_BOOST_TX_TIMING
	if (_MT9M_SetRegister(0x78, 0xA500) != IBSU_STATUS_OK) goto done; 	// RD_SHS_TIMING
	if (_MT9M_SetRegister(0x79, 0xAB00) != IBSU_STATUS_OK) goto done; 	// RD_VCL_COL_EN_TIMING
	if (_MT9M_SetRegister(0x7A, 0xA904) != IBSU_STATUS_OK) goto done; 	// RD_COLCLAMP_TIMING
	if (_MT9M_SetRegister(0x7B, 0xA700) != IBSU_STATUS_OK) goto done; 	// RD_SHVCL_BAR_TIMING
	if (_MT9M_SetRegister(0x7C, 0xA700) != IBSU_STATUS_OK) goto done; 	// RD_VLN_EN_TIMING
	if (_MT9M_SetRegister(0x7E, 0xA900) != IBSU_STATUS_OK) goto done; 	// RD_BOOST_ROW_TIMING
	if (_MT9M_SetRegister(0x7F, 0x0000) != IBSU_STATUS_OK) goto done; 	// RD_EN_VLN_SH_TIMING
	if (_MT9M_SetRegister(0x29, 0x0480) != IBSU_STATUS_OK) goto done; 	// DAC_CONTROL_2_REG
	if (_MT9M_SetRegister(0x3E, 0x80C7) != IBSU_STATUS_OK) goto done; 	// TX_LOW_VOLTAGE
	if (_MT9M_SetRegister(0x3F, 0x0004) != IBSU_STATUS_OK) goto done; 	// RST_LOW_VOLTAGE
	if (_MT9M_SetRegister(0x41, 0x0000) != IBSU_STATUS_OK) goto done; 	// CLAMP_VOLTAGE
	if (_MT9M_SetRegister(0x48, 0x0010) != IBSU_STATUS_OK) goto done; 	// VREF_AMP_BIAS_CURRENT
	if (_MT9M_SetRegister(0x5F, 0x231D) != IBSU_STATUS_OK) goto done; 	// CAL_THRESHOLD   
	if (_MT9M_SetRegister(0x57, 0x0004) != IBSU_STATUS_OK) goto done; 	// ANTI_ECLIPSE_VOLTAGE
	if (_MT9M_SetRegister(0x2A, 0x1086) != IBSU_STATUS_OK) goto done; 	// DAC_CONTROL_3

/*	//==========================================================
	// MT9P031 rolling artifack optimization
	//==========================================================
	if (_MT9M_SetRegister(0x70, 0x00FF) != IBSU_STATUS_OK) goto done; 	// RD_SEQ_LEN
//	_MT9M_SetRegister(0x70, 0x0079); 	// RD_SEQ_LEN
	if (_MT9M_SetRegister(0x71, 0x7800) != IBSU_STATUS_OK) goto done; 	// RD_PTR_TIMING
	if (_MT9M_SetRegister(0x72, 0x7800) != IBSU_STATUS_OK) goto done; 	// RD_ROW_EN_TIMING
	if (_MT9M_SetRegister(0x73, 0x0300) != IBSU_STATUS_OK) goto done; 	// RD_RST_EN_TIMING
	if (_MT9M_SetRegister(0x74, 0x0300) != IBSU_STATUS_OK) goto done; 	// RD_BOOST_RST_TIMING
	if (_MT9M_SetRegister(0x75, 0x3C00) != IBSU_STATUS_OK) goto done; 	// RD_SHR_TIMING
	if (_MT9M_SetRegister(0x76, 0x4E3D) != IBSU_STATUS_OK) goto done; 	// RD_TX_EN_TIMING
	if (_MT9M_SetRegister(0x77, 0x4E3D) != IBSU_STATUS_OK) goto done; 	// RD_BOOST_TX_TIMING
	if (_MT9M_SetRegister(0x78, 0x774F) != IBSU_STATUS_OK) goto done; 	// RD_SHS_TIMING
	if (_MT9M_SetRegister(0x79, 0x7900) != IBSU_STATUS_OK) goto done; 	// RD_VCL_COL_EN_TIMING
	if (_MT9M_SetRegister(0x7A, 0x7900) != IBSU_STATUS_OK) goto done; 	// RD_COLCLAMP_TIMING
	if (_MT9M_SetRegister(0x7B, 0x7800) != IBSU_STATUS_OK) goto done; 	// RD_SHVCL_BAR_TIMING
	if (_MT9M_SetRegister(0x7C, 0x7800) != IBSU_STATUS_OK) goto done; 	// RD_VLN_EN_TIMING
	if (_MT9M_SetRegister(0x7E, 0x7800) != IBSU_STATUS_OK) goto done; 	// RD_BOOST_ROW_TIMING
	if (_MT9M_SetRegister(0x7F, 0x7800) != IBSU_STATUS_OK) goto done; 	// RD_EN_VLN_SH_TIMING
	if (_MT9M_SetRegister(0x29, 0x0481) != IBSU_STATUS_OK) goto done; 	// DAC_CONTROL_2_REG
	if (_MT9M_SetRegister(0x3E, 0x0087) != IBSU_STATUS_OK) goto done; 	// TX_LOW_VOLTAGE
	if (_MT9M_SetRegister(0x3F, 0x0007) != IBSU_STATUS_OK) goto done; 	// RST_LOW_VOLTAGE
	if (_MT9M_SetRegister(0x41, 0x0003) != IBSU_STATUS_OK) goto done; 	// CLAMP_VOLTAGE
	if (_MT9M_SetRegister(0x48, 0x0018) != IBSU_STATUS_OK) goto done; 	// VREF_AMP_BIAS_CURRENT
	if (_MT9M_SetRegister(0x5F, 0x1C16) != IBSU_STATUS_OK) goto done; 	// CAL_THRESHOLD   
	if (_MT9M_SetRegister(0x57, 0x0007) != IBSU_STATUS_OK) goto done; 	// ANTI_ECLIPSE_VOLTAGE
	if (_MT9M_SetRegister(0x2A, 0xFF74) != IBSU_STATUS_OK) goto done; 	// DAC_CONTROL_3
*/
	FPGAVersion = _GetFpgaVersion(TRUE);

	if(FPGAVersion >= _FPGA_VER_FOR_KOJAK_3_0)
	{
		if (_FPGA_SetRegister(0x5C, 0x00) != IBSU_STATUS_OK) goto done;

		// Analog Touch Enable
		if (_FPGA_SetRegister(0x64, 0x01) != IBSU_STATUS_OK) goto done;
	}

    Sleep(100);
    if (_FPGA_SetRegister(0x01, 0x01) != IBSU_STATUS_OK) goto done;	// set capture flag

	if(FPGAVersion >= _FPGA_VER_FOR_KOJAK_3_0)
	{
		m_DeciCaptureSetting.forSuperDry_Mode = m_propertyInfo.nSuperDryMode;

		m_DeciCaptureSetting.DefaultMinDAC = _KOJAK30_DAC_FOR_MINIMUM_;
		m_DeciCaptureSetting.DefaultMaxDAC = _DAC_FOR_MAXIMUM_;
		if(m_DeciCaptureSetting.forSuperDry_Mode)
		{
			m_DeciCaptureSetting.TargetMinBrightness = _TARGET_MIN_BRIGHTNESS_FOR_DRY_;
			m_DeciCaptureSetting.TargetMaxBrightness = _TARGET_MAX_BRIGHTNESS_FOR_DRY_;
		}
		else
		{
			m_DeciCaptureSetting.TargetMinBrightness = m_propertyInfo.nCaptureBrightThresFlat[0];
			m_DeciCaptureSetting.TargetMaxBrightness = m_propertyInfo.nCaptureBrightThresFlat[1];
		}
		m_DeciCaptureSetting.TargetFingerCount = m_propertyInfo.nNumberOfObjects;
		if(m_propertyInfo.nSuperDryMode)
		{
			if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER)
				m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_DRY_ONE_FINGER_;
			else if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS)
				m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_DRY_TWO_FINGER_;
			else
				m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_DRY_FOUR_FINGER_;
		}
		else
		{
			if(m_propertyInfo.bEnableStartingVoltage == TRUE)
			{
				m_DeciCaptureSetting.DefaultDAC = m_propertyInfo.nStartingVoltage;
			}
			else
			{
				if(m_UsbDeviceInfo.bCanUseTOF &&
					m_propertyInfo.bEnableTOF)
				{
					if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
						m_DeciCaptureSetting.DefaultDAC = _KOJAK30_DAC_FOR_NORMAL_FINGER_WITH_TOF_;
					else
						m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FINGER_WITH_TOF_;
				}
				else if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_NEW_BRD_AND_DOUBLE_P_ ||
					m_propertyInfo.bKojakPLwithDPFilm == TRUE)
				{
					if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS || 
						m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
						m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_OF_DOUBLE_P_;
					else
						m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FOUR_FINGER_OF_DOUBLE_P_;
				}
				else
				{
					if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS || 
						m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
						m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_SINGLE_FINGER_;
					else
						m_DeciCaptureSetting.DefaultDAC = _DAC_FOR_NORMAL_FOUR_FINGER_;
				}
			}
		}
		m_DeciCaptureSetting.CurrentDAC = m_DeciCaptureSetting.DefaultDAC;
		m_DeciCaptureSetting.DefaultGain = _DEFAULT_GAIN_FOR_DECI_;
		m_DeciCaptureSetting.CurrentGain = m_DeciCaptureSetting.DefaultGain;
		m_DeciCaptureSetting.DefaultExposure = _KOJAK30_DEFAULT_EXPOSURE_FOR_DECI_;
		m_DeciCaptureSetting.CurrentExposure = m_DeciCaptureSetting.DefaultExposure;
		m_DeciCaptureSetting.DefaultBitshift = _KOJAK30_DEFAULT_BITSHIFT_FOR_DECI_;
		m_DeciCaptureSetting.CurrentBitshift = m_DeciCaptureSetting.DefaultBitshift;
		m_DeciCaptureSetting.DefaultCutThres = _KOJAK30_DEFAULT_CUTTHRES_FOR_DECI_;
		m_DeciCaptureSetting.CurrentCutThres = m_DeciCaptureSetting.DefaultCutThres;
		m_DeciCaptureSetting.AdditionalFrame = 1;
		m_DeciCaptureSetting.RollCaptureMode = FALSE;
		m_DeciCaptureSetting.DetectionFrameCount = 0;
		m_DeciCaptureSetting.Kojak30_PERIOD = _KOJAK30_PERIOD_DECI_;
		m_DeciCaptureSetting.Kojak30_ELON = _KOJAK30_DEFAULT_ELON_DECI_;

		m_FullCaptureSetting = m_DeciCaptureSetting;
		m_FullCaptureSetting.DefaultExposure = _KOJAK30_NO_BIT_EXPOSURE_FOR_FULLFRAME_;
		m_FullCaptureSetting.DefaultBitshift = _KOJAK30_NO_BIT_BITSHIFT_FOR_FULLFRAME_;
		m_FullCaptureSetting.DefaultCutThres = _KOJAK30_NO_BIT_CUTTHRES_FOR_FULLFRAME_;
		m_FullCaptureSetting.CurrentExposure = m_FullCaptureSetting.DefaultExposure;
		m_FullCaptureSetting.CurrentBitshift = m_FullCaptureSetting.DefaultBitshift;
		m_FullCaptureSetting.CurrentCutThres = m_FullCaptureSetting.DefaultCutThres;
		m_FullCaptureSetting.Kojak30_PERIOD = _KOJAK30_PERIOD_FOR_FULLFRAME_;
		m_FullCaptureSetting.Kojak30_ELON = _KOJAK30_DEFAULT_ELON_FOR_FULLFRAME_;

		m_UsbDeviceInfo.bDecimation = FALSE;
		_ChangeDecimationModeForKojak(m_UsbDeviceInfo.bDecimation, &m_DeciCaptureSetting, TRUE);

		if (_FPGA_SetRegister(0x54, 0x01) != IBSU_STATUS_OK) goto done;
		if (_FPGA_SetRegister(0x56, 0x01) != IBSU_STATUS_OK) goto done;
		if (_FPGA_SetRegister(0x55, 0x01) != IBSU_STATUS_OK) goto done;
	}

	if(FPGAVersion >= _FPGA_VER_FOR_KOJAK_3_0)
	{
		if (_FPGA_SetRegister(0x54, 0x01) != IBSU_STATUS_OK) goto done;

		if (_FPGA_SetRegister(0x56, 0x00) != IBSU_STATUS_OK) goto done;
		if (_FPGA_SetRegister(0x55, 0x01) != IBSU_STATUS_OK) goto done;
	}

	if (m_UsbDeviceInfo.bCanUseTOF)
	{
		// TOF Initialize
		_SetPlateTouchOff();
		Sleep(10);
//		_WriteCurrentSensor(0x00, 0xB99D);
//		Sleep(100);
		_WriteCurrentSensor(0x00, 0x3FFF);
		Sleep(10);
		_WriteCurrentSensor(0x01, 0x0000);
		Sleep(10);
		_SetPlateTouchStatus(1);	// Touch Ready
		Sleep(10);
		_SetPlateTouchOn();
		Sleep(300);
		_LE_OFF();
		Sleep(10);
		_SetPlateTouchStatus(0);	// Touch Idle
	}

	if(FPGAVersion >= _FPGA_VER_FOR_KOJAK_3_0)
	{
 		if (_FPGA_SetRegister(0x65, 0x43) != IBSU_STATUS_OK) goto done;
		if (_FPGA_SetRegister(0x53, 0x01) != IBSU_STATUS_OK) goto done;
		if (_FPGA_SetRegister(0x53, 0x00) != IBSU_STATUS_OK) goto done;

		if (_FPGA_SetRegister(0x59, 0x01) != IBSU_STATUS_OK) goto done;

		if (_FPGA_SetRegister(0x5A, 0x88) != IBSU_STATUS_OK) goto done;
		if (_FPGA_SetRegister(0x5B, 0x44) != IBSU_STATUS_OK) goto done;
	}

	_Initialize_Encryption_For_Kojak();

    return IBSU_STATUS_OK;

done:
    return IBSU_ERR_DEVICE_IO;
}

int	CMainCapture::Capture_SetLEVoltage_Kojak(int voltageValue)
{
//    return IBSU_STATUS_OK;
	/*switch(voltageValue)
	{
	case 0:		return _SetLEVoltage(0x96);
	case 1:		return _SetLEVoltage(0x91);
	case 2:		return _SetLEVoltage(0x8C);
	case 3:		return _SetLEVoltage(0x87);
	case 4:		return _SetLEVoltage(0x82);
	case 5:		return _SetLEVoltage(0x7D);
	case 6:		return _SetLEVoltage(0x78);
	case 7:		return _SetLEVoltage(0x73);
	case 8:		return _SetLEVoltage(0x6E);
	case 9:		return _SetLEVoltage(0x69);
	case 10:	return _SetLEVoltage(0x64);
	case 11:	return _SetLEVoltage(0x5F);
	case 12:	return _SetLEVoltage(0x5A);
	case 13:	return _SetLEVoltage(0x55);
	case 14:	return _SetLEVoltage(0x50);
	case 15:	return _SetLEVoltage(0x4B);
	default:
		return IBSU_ERR_INVALID_PARAM_VALUE;
	}*/

	// for Transformer (0.8.0)
	if(m_UsbDeviceInfo.nFpgaVersion ==  ( (0 << 16) | (8 << 8) | 0) )
	{
		switch(voltageValue)
		{
			case 0:		return _SetLEVoltage(0xFF);
			case 1:		return _SetLEVoltage(0xFF);
			case 2:		return _SetLEVoltage(0xFF);
			case 3:		return _SetLEVoltage(0xFF);
			case 4:		return _SetLEVoltage(0xFF);
			case 5:		return _SetLEVoltage(0xFF);
			case 6:		return _SetLEVoltage(0xFF);
			case 7:		return _SetLEVoltage(0xFF);
			case 8:		return _SetLEVoltage(0xFF);
			case 9:		return _SetLEVoltage(0xFF);
			case 10:	return _SetLEVoltage(0xFF);
			case 11:	return _SetLEVoltage(0xFF);
			case 12:	return _SetLEVoltage(0xFB);
			case 13:	return _SetLEVoltage(0xF8);
			case 14:	return _SetLEVoltage(0xF4);
			case 15:	return _SetLEVoltage(0xF0);
			default:
				return IBSU_ERR_INVALID_PARAM_VALUE;
		}
	}
	else if(m_UsbDeviceInfo.nFpgaVersion ==  ( (0 << 16) | (8 << 8) | 1) )
	{
		// for old Rogers (0.8.1)
		switch(voltageValue)
		{
			case 0:		return _SetLEVoltage(0x01);
			case 1:		return _SetLEVoltage(0x01);
			case 2:		return _SetLEVoltage(0x01);
			case 3:		return _SetLEVoltage(0x01);
			case 4:		return _SetLEVoltage(0x01);
			case 5:		return _SetLEVoltage(0x01);
			case 6:		return _SetLEVoltage(0x01);
			case 7:		return _SetLEVoltage(0x01);
			case 8:		return _SetLEVoltage(0x01);
			case 9:		return _SetLEVoltage(0x02);
			case 10:	return _SetLEVoltage(0x03);
			case 11:	return _SetLEVoltage(0x04);
			case 12:	return _SetLEVoltage(0x05);
			case 13:	return _SetLEVoltage(0x06);
			case 14:	return _SetLEVoltage(0x07);
			case 15:	return _SetLEVoltage(0x08);
			default:
				return IBSU_ERR_INVALID_PARAM_VALUE;
		}
	}
	else if(m_UsbDeviceInfo.nFpgaVersion ==  ( (0 << 16) | (8 << 8) | 2) )
	{
		// for new Rogers (0.8.2)
		switch(voltageValue)
		{
			case 0:		return _SetLEVoltage(0x01);
			case 1:		return _SetLEVoltage(0x01);
			case 2:		return _SetLEVoltage(0x01);
			case 3:		return _SetLEVoltage(0x01);
			case 4:		return _SetLEVoltage(0x01);
			case 5:		return _SetLEVoltage(0x01);
			case 6:		return _SetLEVoltage(0x01);
			case 7:		return _SetLEVoltage(0x01);
			case 8:		return _SetLEVoltage(0x01);
			case 9:		return _SetLEVoltage(0x02);
			case 10:	return _SetLEVoltage(0x03);
			case 11:	return _SetLEVoltage(0x05);
			case 12:	return _SetLEVoltage(0x06);
			case 13:	return _SetLEVoltage(0x07);
			case 14:	return _SetLEVoltage(0x08);
			case 15:	return _SetLEVoltage(0x08);
			default:
				return IBSU_ERR_INVALID_PARAM_VALUE;
		}
	}
	else if(m_UsbDeviceInfo.nFpgaVersion >=  ( (1 << 24) | (0 << 16) | (0 << 8) | 0x00) )				// GON
	{
		return IBSU_STATUS_OK;
	}
	else if(m_UsbDeviceInfo.nFpgaVersion >=  ( (0 << 24) | (9 << 16) | (0 << 8) | 0x0D) )				// GON
	{
		// for Transformer (0.9.0.D)
		return _SetLEVoltage(voltageValue);
	}
	else if(m_UsbDeviceInfo.nFpgaVersion >=  ( (0 << 24) | (8 << 16) | (3 << 8) | 0x00) )
	{
		// for new Rogers (0.8.3)
		switch(voltageValue)
		{
			case 0:		return _SetLEVoltage(0x01);
			case 1:		return _SetLEVoltage(0x01);
			case 2:		return _SetLEVoltage(0x01);
			case 3:		return _SetLEVoltage(0x01);
			case 4:		return _SetLEVoltage(0x01);
			case 5:		return _SetLEVoltage(0x01);
			case 6:		return _SetLEVoltage(0x01);
			case 7:		return _SetLEVoltage(0x01);
			case 8:		return _SetLEVoltage(0x01);
			case 9:		return _SetLEVoltage(0x02);
			case 10:	return _SetLEVoltage(0x02);
			case 11:	return _SetLEVoltage(0x03);
			case 12:	return _SetLEVoltage(0x04);
			case 13:	return _SetLEVoltage(0x05);
			case 14:	return _SetLEVoltage(0x06);
			case 15:	return _SetLEVoltage(0x07);
			default:
				return IBSU_ERR_INVALID_PARAM_VALUE;
		}
	}

	return IBSU_ERR_INVALID_PARAM_VALUE;
}

int	CMainCapture::Capture_GetLEVoltage_Kojak(int *voltageValue)
{

	int nRc;
	int ReadVoltage;

	nRc = _GetLEVoltage(&ReadVoltage);

	if(m_UsbDeviceInfo.nFpgaVersion ==  ( (0 << 16) | (8 << 8) | 0) )
	{
		switch(ReadVoltage)
		{
		case 0xFF:	*voltageValue = 11;	break;
		case 0xFB:	*voltageValue = 12;	break;
		case 0xF8:	*voltageValue = 13;	break;
		case 0xF4:	*voltageValue = 14;	break;
		case 0xF0:	*voltageValue = 15;	break;
		default:	*voltageValue = 0;	break;
		}
	}
	else if(m_UsbDeviceInfo.nFpgaVersion ==  ( (0 << 16) | (8 << 8) | 1) )
	{
		switch(ReadVoltage)
		{
		case 0x01:  *voltageValue = 8;	break;
		case 0x02:	*voltageValue = 9;	break;
		case 0x03:	*voltageValue = 10;	break;
		case 0x04:	*voltageValue = 11;	break;
		case 0x05:	*voltageValue = 12;	break;
		case 0x06:	*voltageValue = 13;	break;
		case 0x07:	*voltageValue = 14;	break;
		case 0x08:	*voltageValue = 15;	break;
		default:	*voltageValue = 0;	break;
		}
	}
	else if(m_UsbDeviceInfo.nFpgaVersion ==  ( (0 << 16) | (8 << 8) | 2) )
	{
		switch(ReadVoltage)
		{
		case 0x01:  *voltageValue = 8;	break;
		case 0x02:	*voltageValue = 9;	break;
		case 0x03:	*voltageValue = 10;	break;
		case 0x05:	*voltageValue = 11;	break;
		case 0x06:	*voltageValue = 12;	break;
		case 0x07:	*voltageValue = 13;	break;
		case 0x08:	*voltageValue = 14;	break;
//		case 0x08:	*voltageValue = 15;	break;
		default:	*voltageValue = 0;	break;
		}
	}
	else if(m_UsbDeviceInfo.nFpgaVersion >=  ( (0 << 24) | (9 << 16) | (0 << 8) | 0x0D) )				// GON
	{
		if(nRc == IBSU_STATUS_OK)
			*voltageValue = ReadVoltage;
	}
	else if(m_UsbDeviceInfo.nFpgaVersion >=  ( (0 << 24) | (8 << 16) | (3 << 8) | 0x00) )
	{
		switch(ReadVoltage)
		{
		case 0x01:  *voltageValue = 8;	break;
		//case 0x02:	*voltageValue = 9;	break;
		//case 0x03:	*voltageValue = 10;	break;
		case 0x02:	*voltageValue = 11;	break;
		case 0x03:	*voltageValue = 11;	break;
		case 0x04:	*voltageValue = 12;	break;
		case 0x05:	*voltageValue = 13;	break;
		case 0x06:	*voltageValue = 14;	break;
		case 0x07:	*voltageValue = 15;	break;

		default:	*voltageValue = 0;	break;
		}
	}	

	return IBSU_ERR_INVALID_PARAM_VALUE;

	return nRc;
}

int CMainCapture::_SetLEOperationMode_Kojak(WORD addr)
{
    UCHAR		outBuffer[64]={0};

    outBuffer[0] = (UCHAR)addr;
    return _UsbBulkOutIn( EP1OUT, CMD_WRITE_LE_MODE, outBuffer, 1, -1, NULL, 0, NULL );
}

AcuisitionState CMainCapture::_GoJob_PreImageProcessing_Kojak(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage)
{
	int Bright = 0, i, j;
    *bIsGoodImage = FALSE;

	if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize_Roll);
		
		m_pAlgo->m_cImgAnalysis.isDetected = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_forDetectOnly_Roll(InImg, 100);
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_Roll(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);

		// 지문 개수 루틴
		m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(Bright);
	}
	else
	{
		if(m_UsbDeviceInfo.bDecimation == TRUE)
		{
			memcpy(m_pAlgo->m_SBDAlg->m_OriginalImg, InImg, m_UsbDeviceInfo.CisImgSize/4);
			memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize/4);
		}
		else
		{
			for(i=0; i<m_UsbDeviceInfo.CisImgHeight/2; i++)
			{
				for(j=0; j<m_UsbDeviceInfo.CisImgWidth/2; j++)
				{
					m_pAlgo->m_SBDAlg->m_OriginalImg[i*m_UsbDeviceInfo.CisImgWidth/2+j] = InImg[(i*m_UsbDeviceInfo.CisImgWidth+j)*2];
				}
			}
		}
	
		m_pAlgo->m_cImgAnalysis.isDetected = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_forDetectOnly(m_pAlgo->m_SBDAlg->m_OriginalImg, 100);
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);
//		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_Final(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
//			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);
//		if(m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER)	// 결정이 필요한 사항
//			m_pAlgo->m_cImgAnalysis.mean = (m_pAlgo->m_SBDAlg->m_SegmentBright+m_pAlgo->m_cImgAnalysis.mean)/2;

		// 지문 개수 루틴
		m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(Bright);
	}

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
    int savedFingerState = m_clbkProperty.nFingerState;
    if (m_pAlgo->m_cImgAnalysis.finger_count == 0)		// add 0.10.4 by enzyme
    {
        m_clbkProperty.nFingerState = ENUM_IBSU_NON_FINGER;
		
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

    if (!m_bFirstSent_clbkClearPlaten && savedFingerState != m_clbkProperty.nFingerState)
    {
        _PostCallback(CALLBACK_FINGER_COUNT);
    }
    ///////////////////////////////////////////////////////////////////////////

    if ((m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CONTRAST) == IBSU_OPTION_AUTO_CONTRAST)
    {
        if (m_UsbDeviceInfo.LeVoltageControl == SUPPORT_LE_VOLTAGE_CONTROL)
        {
			if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			{
				m_pAlgo->_Algo_Kojak_AutoCapture(&m_FullCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood);
				_ApplyCaptureSettings(m_FullCaptureSetting);
			}
			else
			{
				m_pAlgo->_Algo_Kojak_AutoCapture(&m_DeciCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood);
				_ApplyCaptureSettings(m_DeciCaptureSetting);
			}

            if (m_bFirstSent_clbkClearPlaten)
                m_pAlgo->m_CaptureGood = FALSE;

			if(m_pAlgo->m_CaptureGood == TRUE)
			{
				if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
				{
					*bIsGoodImage = TRUE;
				}
				else
				{
					m_FullCaptureSetting.CurrentGain = m_DeciCaptureSetting.CurrentGain;
					m_FullCaptureSetting.CurrentDAC = m_DeciCaptureSetting.CurrentDAC;
					*bIsGoodImage = TRUE;
				}
			}
        }
    }
    else
    {
		m_pAlgo->_Algo_Kojak_AutoCapture(&m_FullCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood);
		_Set_CIS_GainRegister(m_propertyInfo.nContrastValue, FALSE, TRUE);
    }

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

AcuisitionState CMainCapture::_GoJob_PreImageProcessing_Kojak_CurrentSensor(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage)
{
	int Bright = 0, i, j;
    *bIsGoodImage = FALSE;

	m_TOF_CurVal = 0;
	m_TOF_Thres = 0;

	if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize_Roll);
		
		m_pAlgo->m_cImgAnalysis.isDetected = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_forDetectOnly_Roll(InImg, 100);
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_Roll(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);

		// 지문 개수 루틴
		m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(Bright);
	}
	else
	{
		if(m_UsbDeviceInfo.bDecimation == TRUE)
		{
			memcpy(m_pAlgo->m_SBDAlg->m_OriginalImg, InImg, m_UsbDeviceInfo.CisImgSize/4);
			memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize/4);
		}
		else
		{
			for(i=0; i<m_UsbDeviceInfo.CisImgHeight/2; i++)
			{
				for(j=0; j<m_UsbDeviceInfo.CisImgWidth/2; j++)
				{
					m_pAlgo->m_SBDAlg->m_OriginalImg[i*m_UsbDeviceInfo.CisImgWidth/2+j] = InImg[(i*m_UsbDeviceInfo.CisImgWidth+j)*2];
				}
			}
			memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize);
		}

		m_pAlgo->m_cImgAnalysis.isDetected = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_forDetectOnly(m_pAlgo->m_SBDAlg->m_OriginalImg, 100);
		/*m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);
		m_pAlgo->SaveBitmapImage("d:\\segment1.bmp", m_pAlgo->m_segment_enlarge_buffer, 
			m_pAlgo->ZOOM_ENLAGE_W, m_pAlgo->ZOOM_ENLAGE_H, FALSE);*/
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_Rev1(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);
		//m_pAlgo->SaveBitmapImage("d:\\segment2.bmp", m_pAlgo->m_segment_enlarge_buffer, 
			//m_pAlgo->ZOOM_ENLAGE_W, m_pAlgo->ZOOM_ENLAGE_H, FALSE);

		// 지문 개수 루틴
		m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(Bright);

#ifdef __G_DEBUG__
		TRACE("m_pAlgo->m_cImgAnalysis.mean : %d, m_SegmentBright : %d, DAC : %02X\n", m_pAlgo->m_cImgAnalysis.mean, m_pAlgo->m_SBDAlg->m_SegmentBright, m_DeciCaptureSetting.CurrentDAC);
#endif
		m_pAlgo->m_cImgAnalysis.mean = (m_pAlgo->m_cImgAnalysis.mean + m_pAlgo->m_SBDAlg->m_SegmentBright)/2;
	}

	m_Brightness = m_pAlgo->m_cImgAnalysis.mean;

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
    int savedFingerState = m_clbkProperty.nFingerState;
    if (m_pAlgo->m_cImgAnalysis.finger_count == 0)		// add 0.10.4 by enzyme
    {
        m_clbkProperty.nFingerState = ENUM_IBSU_NON_FINGER;
		
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

    if (!m_bFirstSent_clbkClearPlaten && savedFingerState != m_clbkProperty.nFingerState)
    {
        _PostCallback(CALLBACK_FINGER_COUNT);
    }
    ///////////////////////////////////////////////////////////////////////////

/*	if(m_CaptureProgressStatus == ENUM_IBSM_CAPTURE_PROGRESS_NONE)
	{
		if(m_clbkProperty.nPlatenState == ENUM_IBSU_PLATEN_CLEARD)
		{
			_Start_PlateTouch();
			m_CaptureProgressStatus = ENUM_IBSM_CAPTURE_PROGRESS_TOUCH;
			m_TouchCount = 0;
			m_TouchFrameCount = 0;
			m_TOF_ON_Count = 0;
			m_TOF_OFF_Count = 0;
			memset(m_TOFArr, 0, sizeof(m_TOFArr));
			m_TOFArr_idx=0;
			m_DeciCaptureSetting.CurrentDAC = m_DeciCaptureSetting.DefaultDAC;
		}
	}
	
	// 현재 상태 파악		
	if(m_CaptureProgressStatus == ENUM_IBSM_CAPTURE_PROGRESS_TOUCH)
	{
		m_TouchFrameCount++;
		if(m_TouchFrameCount > 10000)
			m_TouchFrameCount = 100;

		if(_Check_PlateTouch() == TRUE)
			m_TouchCount++;
		else
			m_TouchCount=0;
		if(m_TouchCount >= 1 && m_TouchFrameCount >= 2)
		{
			m_TouchCount=0;
			m_CaptureProgressStatus = ENUM_IBSM_CAPTURE_PROGRESS_TOF;
			_Start_TOF();
		}
	}
	
	else if(m_CaptureProgressStatus == ENUM_IBSM_CAPTURE_PROGRESS_TOF)
	{
		m_pAlgo->m_cImgAnalysis.isDetectedTOF = _Check_TOF(&m_TOF_CurVal, &m_TOF_Thres);
		m_TOF_DAC = m_DeciCaptureSetting.CurrentDAC;
		//_PostCallback(CALLBACK_TOFCURRENT);

		if(m_pAlgo->m_cImgAnalysis.isDetectedTOF || m_pAlgo->m_cImgAnalysis.isDetected)
		{
			m_TOF_ON_Count++;
			m_TOF_OFF_Count=0;
		}
		else
		{
			m_TOF_OFF_Count++;
			m_TOF_ON_Count=0;
		}

		if(m_TOF_ON_Count >= 1)
		{
			m_CaptureProgressStatus = ENUM_IBSM_CAPTURE_PROGRESS_CAPTURING;
		}
		if(m_TOF_OFF_Count >= 5)
		{
			_Start_PlateTouch();
			m_CaptureProgressStatus = ENUM_IBSM_CAPTURE_PROGRESS_TOUCH;
			m_TouchCount = 0;
			m_TOF_ON_Count = 0;
			m_TOF_OFF_Count = 0;
			m_DeciCaptureSetting.CurrentDAC = m_DeciCaptureSetting.DefaultDAC;
		}
	}
	
	else if(m_CaptureProgressStatus == ENUM_IBSM_CAPTURE_PROGRESS_CAPTURING)
	{
		m_pAlgo->m_cImgAnalysis.isDetectedTOF = _Check_TOF(&m_TOF_CurVal, &m_TOF_Thres);
		m_TOF_DAC = m_DeciCaptureSetting.CurrentDAC;
		//_PostCallback(CALLBACK_TOFCURRENT);

		if(m_pAlgo->m_cImgAnalysis.isDetectedTOF || m_pAlgo->m_cImgAnalysis.isDetected)
			m_TOF_OFF_Count=0;
		else
			m_TOF_OFF_Count++;

//			TRACE("ENUM_IBSM_CAPTURE_PROGRESS_CAPTURING : %X, %d, %d, %d--", m_TOF_CurVal, m_pAlgo->m_cImgAnalysis.isDetectedTOF, m_pAlgo->m_cImgAnalysis.isDetected, m_TOF_OFF_Count);

		if(m_TOF_OFF_Count >= 5)
		{
			_Start_PlateTouch();
			m_CaptureProgressStatus = ENUM_IBSM_CAPTURE_PROGRESS_TOUCH;
			m_TouchCount = 0;
			m_TOF_ON_Count = 0;
			m_TOF_OFF_Count = 0;
			m_DeciCaptureSetting.CurrentDAC = m_DeciCaptureSetting.DefaultDAC;
		}
	}
*/
	int tmp_isDetectedTOF = _Check_TOF(&m_TOF_CurVal, &m_TOF_Thres);
	m_pAlgo->m_mainTOFValue = m_TOF_CurVal;

	if(m_TOF_MaxVal < m_TOF_CurVal)
	{
		m_TOF_MaxVal = m_TOF_CurVal;
		m_TOF_MaxVal_DAC = m_CurrentCaptureDAC;
	}

	if(m_CurrentCaptureDACMax > m_CurrentCaptureDAC)
	{
		m_CurrentCaptureDACMax = m_CurrentCaptureDAC;
		m_CurrentCaptureDACMax_TOFValue = m_TOF_CurVal;
	}

	if(tmp_isDetectedTOF)
	{
		m_TOF_OFF_Count=0;
		m_TOF_ON_Count++;
		if(m_TOF_ON_Count >= 3)
		{
			m_TOF_ON_Count=3;
			m_pAlgo->m_cImgAnalysis.isDetectedTOF = TRUE;
		}
	}
	else
	{
		m_TOF_ON_Count=0;
		m_TOF_OFF_Count++;
		if(m_TOF_OFF_Count >= 3)
		{
			m_TOF_OFF_Count=3;
			m_pAlgo->m_cImgAnalysis.isDetectedTOF = FALSE;
		}
	}

#ifdef __G_DEBUG__
	TRACE("DAC : %02X, Current TOF(%4d) - thres(%4d) - Diff(%4d) isDetectedTOF : %d\n", 
		m_CurrentCaptureDAC, m_TOF_CurVal, m_TOF_Thres, m_TOF_CurVal-m_TOF_Thres, m_pAlgo->m_cImgAnalysis.isDetectedTOF);
#endif

    if ((m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CONTRAST) == IBSU_OPTION_AUTO_CONTRAST)
    {
        if (m_UsbDeviceInfo.LeVoltageControl == SUPPORT_LE_VOLTAGE_CONTROL/* &&
			m_CaptureProgressStatus == ENUM_IBSM_CAPTURE_PROGRESS_CAPTURING*/)
        {
			if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			{
				m_pAlgo->_Algo_Kojak_AutoCapture_CurrentSensor(&m_FullCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood);
				_ApplyCaptureSettings(m_FullCaptureSetting);
			}
			else
			{
				if(m_propertyInfo.bEnableDecimation)
				{
					m_pAlgo->_Algo_Kojak_AutoCapture_CurrentSensor(&m_DeciCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood);
					_ApplyCaptureSettings(m_DeciCaptureSetting);
				}
				else
				{
					m_pAlgo->_Algo_Kojak_AutoCapture_CurrentSensor(&m_FullCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood);
					_ApplyCaptureSettings(m_FullCaptureSetting);
				}
			}

            if (m_bFirstSent_clbkClearPlaten)
                m_pAlgo->m_CaptureGood = FALSE;

			if(m_pAlgo->m_CaptureGood == TRUE)
			{
				if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
				{
					*bIsGoodImage = TRUE;
				}
				else
				{
					if(m_propertyInfo.bEnableDecimation)
					{
						m_FullCaptureSetting.CurrentGain = m_DeciCaptureSetting.CurrentGain;
						m_FullCaptureSetting.CurrentDAC = m_DeciCaptureSetting.CurrentDAC;
					}
					*bIsGoodImage = TRUE;
				}
			}
        }
    }
    else
    {
		if (m_UsbDeviceInfo.LeVoltageControl == SUPPORT_LE_VOLTAGE_CONTROL/* && 
				m_CaptureProgressStatus == ENUM_IBSM_CAPTURE_PROGRESS_CAPTURING*/)
		{
			m_pAlgo->_Algo_Kojak_AutoCapture_CurrentSensor(&m_FullCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood);
			m_DeciCaptureSetting.CurrentDAC = _GetContrast_FromGainTable(m_propertyInfo.nContrastValue, FALSE);
			_Set_CIS_GainRegister(m_propertyInfo.nContrastValue, FALSE, TRUE);
		}
    }

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

#ifdef _KOJAK_ATOF_16B_
AcuisitionState CMainCapture::_GoJob_PreImageProcessing_Kojak_CurrentSensor_and_ATOF(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage)
{
	int Bright = 0, i, j;
    *bIsGoodImage = FALSE;

	m_TOF_CurVal = 0;
	m_TOF_Thres = 0;

	int tmp_isDetectedTOF = _Check_TOF(&m_TOF_CurVal, &m_TOF_Thres);
	m_pAlgo->m_mainTOFValue = m_TOF_CurVal;

	if(m_TOF_MaxVal < m_TOF_CurVal)
	{
		m_TOF_MaxVal = m_TOF_CurVal;
		m_TOF_MaxVal_DAC = m_CurrentCaptureDAC;
	}

	if(m_CurrentCaptureDACMax > m_CurrentCaptureDAC)
	{
		m_CurrentCaptureDACMax = m_CurrentCaptureDAC;
		m_CurrentCaptureDACMax_TOFValue = m_TOF_CurVal;
	}

	// Kojak 3.0 mode
	if(m_Kojak30_OperationMode == 2)
	{
		m_pAlgo->m_TrueCount = 0;
		// Calc difference of Analog TOF
		int ATOF_Base = m_TOF_Analog2 + (m_propertyInfo.sKojakCalibrationInfo.ATOF_sign*m_propertyInfo.sKojakCalibrationInfo.ATOF_Diff);
		m_TOF_Analog_Thres = 350 + 200 * (m_propertyInfo.nNumberOfObjects-1);

		m_pAlgo->m_mainATOFValue = (int)((float)((m_TOF_Analog - ATOF_Base) * m_propertyInfo.sKojakCalibrationInfo.ATOF_slope) / 1000.0f);
		if(m_pAlgo->m_mainATOFValue < 0)
			m_pAlgo->m_mainATOFValue = 0;

		if(m_pAlgo->m_isEnabledATOF == FALSE)
		{
			ATOF_Base = m_TOF_Analog2 + (m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_sign*m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_Diff);

			m_pAlgo->m_mainATOFValue = (int)((float)((m_TOF_Analog - ATOF_Base) * m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_slope) / 1000.0f);
			if(m_pAlgo->m_mainATOFValue < 0)
				m_pAlgo->m_mainATOFValue = 0;

			m_TOF_Analog_Thres = 250 + 150 * (m_propertyInfo.nNumberOfObjects-1);
		}

		if( m_pAlgo->m_mainATOFValue > m_TOF_Analog_Thres)
		{
			m_ATOF_OFF_Count=0;
			m_ATOF_ON_Count++;
			
			if(m_ATOF_ON_Count >= 3)
			{
				m_ATOF_ON_Count=3;
				m_pAlgo->m_isEnabledATOF = TRUE;

				// LE on
				_FPGA_SetRegister(0x56, 0x00);

				m_TOF_ON_Count=3;
				m_pAlgo->m_cImgAnalysis.isDetectedTOF = TRUE;
			}
		}
		else
		{
			m_ATOF_ON_Count=0;
			m_ATOF_OFF_Count++;
			if(m_ATOF_OFF_Count >= 3)
			{
				m_ATOF_OFF_Count=3;
				m_pAlgo->m_isEnabledATOF = FALSE;

				// LE off
				_FPGA_SetRegister(0x56, 0x01);
				m_DeciCaptureSetting.CurrentDAC = m_DeciCaptureSetting.DefaultDAC;
				m_FullCaptureSetting.CurrentDAC = m_FullCaptureSetting.DefaultDAC;
				m_Kojak30_OperationMode = 2;
				_Kojak30_InitializeOperationMode(m_Kojak30_OperationMode);
			}
		}
	}
	else
	{
		m_pAlgo->m_TrueCount++;
		if(m_pAlgo->m_isEnabledATOF == TRUE && m_pAlgo->m_TrueCount > 3)
		{
			if(tmp_isDetectedTOF)
			{
				m_TOF_OFF_Count=0;
				m_TOF_ON_Count++;
				if(m_TOF_ON_Count >= 3)
				{
					m_TOF_ON_Count=3;
					m_pAlgo->m_cImgAnalysis.isDetectedTOF = TRUE;
				}
			}
			else
			{
				m_TOF_ON_Count=0;
				m_TOF_OFF_Count++;
				if(m_TOF_OFF_Count >= 3)
				{
					m_TOF_OFF_Count=3;
					m_pAlgo->m_cImgAnalysis.isDetectedTOF = FALSE;
				}
			}
		}
	}	

#ifdef __G_DEBUG__
	TRACE("DAC : %02X, Current TOF(%4d) - thres(%4d) - Diff(%4d) isDetectedTOF : %d, A_TOF1 : %d, A_TOF2 : %d, A_TOF(%4d > %4d) m_isEnabledATOF : %d\n", 
		m_CurrentCaptureDAC, m_TOF_CurVal, m_TOF_Thres, m_TOF_CurVal-m_TOF_Thres, m_pAlgo->m_cImgAnalysis.isDetectedTOF, 
		m_TOF_Analog, m_TOF_Analog2, m_pAlgo->m_mainATOFValue, m_TOF_Analog_Thres, m_pAlgo->m_isEnabledATOF);
#endif

/*	if(m_pAlgo->m_isEnabledATOF == FALSE)
	{
		memset(InImg, 0, m_UsbDeviceInfo.CisImgSize);
	}*/

	if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize_Roll);
		
		m_pAlgo->m_cImgAnalysis.isDetected = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_forDetectOnly_Roll(InImg, 100);
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_Roll(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);

		// 지문 개수 루틴
		m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(Bright);
	}
	else
	{
		if(m_UsbDeviceInfo.bDecimation == TRUE)
		{
			memcpy(m_pAlgo->m_SBDAlg->m_OriginalImg, InImg, m_UsbDeviceInfo.CisImgSize/4);
			memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize/4);
		}
		else
		{
			for(i=0; i<m_UsbDeviceInfo.CisImgHeight/2; i++)
			{
				for(j=0; j<m_UsbDeviceInfo.CisImgWidth/2; j++)
				{
					m_pAlgo->m_SBDAlg->m_OriginalImg[i*m_UsbDeviceInfo.CisImgWidth/2+j] = InImg[(i*m_UsbDeviceInfo.CisImgWidth+j)*2];
				}
			}
		}
	
		m_pAlgo->m_cImgAnalysis.isDetected = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_forDetectOnly(m_pAlgo->m_SBDAlg->m_OriginalImg, 100);
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);

		// 지문 개수 루틴
		m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(Bright);

#ifdef __G_DEBUG__
		TRACE("m_pAlgo->m_cImgAnalysis.mean : %d, m_SegmentBright : %d, DAC : %02X\n", m_pAlgo->m_cImgAnalysis.mean, m_pAlgo->m_SBDAlg->m_SegmentBright, m_DeciCaptureSetting.CurrentDAC);
#endif
		m_pAlgo->m_cImgAnalysis.mean = (m_pAlgo->m_cImgAnalysis.mean + m_pAlgo->m_SBDAlg->m_SegmentBright)/2;
	}


	if(m_Kojak30_OperationMode == 2 && m_pAlgo->m_cImgAnalysis.mean >= _KOJAK30_BRIGHTNESS_THRES_FROM_AT_TO_CS)
	{
		m_Kojak30_OperationMode = 1;
		m_pAlgo->m_cImgAnalysis.false_contact_count = 0;
		_Kojak30_InitializeOperationMode(m_Kojak30_OperationMode);
		return ACUISITION_NONE;
	}

	m_Brightness = m_pAlgo->m_cImgAnalysis.mean;

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
    int savedFingerState = m_clbkProperty.nFingerState;
    if (m_pAlgo->m_cImgAnalysis.finger_count == 0)		// add 0.10.4 by enzyme
    {
        m_clbkProperty.nFingerState = ENUM_IBSU_NON_FINGER;
		
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

    if (!m_bFirstSent_clbkClearPlaten && savedFingerState != m_clbkProperty.nFingerState)
    {
        _PostCallback(CALLBACK_FINGER_COUNT);
    }
    ///////////////////////////////////////////////////////////////////////////

    if ((m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CONTRAST) == IBSU_OPTION_AUTO_CONTRAST)
    {
        if (m_UsbDeviceInfo.LeVoltageControl == SUPPORT_LE_VOLTAGE_CONTROL/* &&
			m_CaptureProgressStatus == ENUM_IBSM_CAPTURE_PROGRESS_CAPTURING*/)
        {
			if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			{
				m_pAlgo->_Algo_Kojak_AutoCapture_CurrentSensor_and_ATOF(&m_FullCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood, &m_Kojak30_OperationMode);
				_ApplyCaptureSettings(m_FullCaptureSetting);
			}
			else
			{
				m_pAlgo->_Algo_Kojak_AutoCapture_CurrentSensor_and_ATOF(&m_DeciCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood, &m_Kojak30_OperationMode);
				_ApplyCaptureSettings(m_DeciCaptureSetting);
			}

			if (m_bFirstSent_clbkClearPlaten)
                m_pAlgo->m_CaptureGood = FALSE;

			if(m_pAlgo->m_CaptureGood == TRUE)
			{
				if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
				{
					*bIsGoodImage = TRUE;
				}
				else
				{
					m_FullCaptureSetting.CurrentGain = m_DeciCaptureSetting.CurrentGain;
					m_FullCaptureSetting.CurrentDAC = m_DeciCaptureSetting.CurrentDAC;
					*bIsGoodImage = TRUE;
				}
			}
        }
    }
    else
    {
		if (m_UsbDeviceInfo.LeVoltageControl == SUPPORT_LE_VOLTAGE_CONTROL/* && 
				m_CaptureProgressStatus == ENUM_IBSM_CAPTURE_PROGRESS_CAPTURING*/)
		{
			m_pAlgo->_Algo_Kojak_AutoCapture_CurrentSensor(&m_FullCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood);
			m_DeciCaptureSetting.CurrentDAC = _GetContrast_FromGainTable(m_propertyInfo.nContrastValue, FALSE);
			_Set_CIS_GainRegister(m_propertyInfo.nContrastValue, FALSE, TRUE);
		}
    }

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

#else

AcuisitionState CMainCapture::_GoJob_PreImageProcessing_Kojak_CurrentSensor_and_ATOF(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage)
{
	int Bright = 0, i, j;
    *bIsGoodImage = FALSE;

	m_TOF_CurVal = 0;
	m_TOF_Thres = 0;

	int tmp_isDetectedTOF = _Check_TOF(&m_TOF_CurVal, &m_TOF_Thres);
	m_pAlgo->m_mainTOFValue = m_TOF_CurVal;

	if(m_TOF_MaxVal < m_TOF_CurVal)
	{
		m_TOF_MaxVal = m_TOF_CurVal;
		m_TOF_MaxVal_DAC = m_CurrentCaptureDAC;
	}

	if(m_CurrentCaptureDACMax > m_CurrentCaptureDAC)
	{
		m_CurrentCaptureDACMax = m_CurrentCaptureDAC;
		m_CurrentCaptureDACMax_TOFValue = m_TOF_CurVal;
	}

	// Calc difference of Analog TOF
	int ATOF_Base = m_TOF_Analog2 + (m_propertyInfo.sKojakCalibrationInfo.ATOF_sign*m_propertyInfo.sKojakCalibrationInfo.ATOF_Diff);
	m_TOF_Analog_Thres = 350 + 200 * (m_propertyInfo.nNumberOfObjects-1);

	m_pAlgo->m_mainATOFValue = (int)((float)((m_TOF_Analog - ATOF_Base) * m_propertyInfo.sKojakCalibrationInfo.ATOF_slope) / 1000.0f);
	if(m_pAlgo->m_mainATOFValue < 0)
		m_pAlgo->m_mainATOFValue = 0;

	if(m_pAlgo->m_isEnabledATOF == FALSE)
	{
		ATOF_Base = m_TOF_Analog2 + (m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_sign*m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_Diff);

		m_pAlgo->m_mainATOFValue = (int)((float)((m_TOF_Analog - ATOF_Base) * m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_slope) / 1000.0f);
		if(m_pAlgo->m_mainATOFValue < 0)
			m_pAlgo->m_mainATOFValue = 0;

		m_TOF_Analog_Thres = 250 + 150 * (m_propertyInfo.nNumberOfObjects-1);
	}

	//if(m_nFrameCount <= 100)
	//{
	//	_FPGA_SetRegister(0x56, 0x01);
	//	m_DeciCaptureSetting.CurrentDAC = 0xFF;
	//	m_FullCaptureSetting.CurrentDAC = 0xFF;

	//	if(m_nFrameCount == 100)
	//		_FPGA_SetRegister(0x56, 0x00);
	//}
	//else {
	if( m_pAlgo->m_mainATOFValue > m_TOF_Analog_Thres)
	{
		m_ATOF_OFF_Count=0;
		m_ATOF_ON_Count++;
		
		if(m_ATOF_ON_Count >= 3)
		{
			m_ATOF_ON_Count=3;
			m_pAlgo->m_isEnabledATOF = TRUE;

			// LE on
			_FPGA_SetRegister(0x56, 0x00);
		}
	}
	else
	{
		m_ATOF_ON_Count=0;
		m_ATOF_OFF_Count++;
		if(m_ATOF_OFF_Count >= 3)
		{
			m_ATOF_OFF_Count=3;
			m_pAlgo->m_isEnabledATOF = FALSE;

			// LE off
			_FPGA_SetRegister(0x56, 0x01);

			m_DeciCaptureSetting.CurrentDAC = m_DeciCaptureSetting.DefaultDAC;
			m_FullCaptureSetting.CurrentDAC = m_DeciCaptureSetting.DefaultDAC;
		}
	}
	//}

	//FILE *fp = fopen("d:\\LEON_OFF.txt", "at");
	//fprintf(fp, "%X\t%d\t%d\n", m_DeciCaptureSetting.CurrentDAC, m_TOF_Analog, m_TOF_Analog2);
	//fclose(fp);

	if(tmp_isDetectedTOF)
	{
		m_TOF_OFF_Count=0;
		m_TOF_ON_Count++;
		if(m_TOF_ON_Count >= 3)
		{
			m_TOF_ON_Count=3;
			m_pAlgo->m_cImgAnalysis.isDetectedTOF = TRUE;
		}
	}
	else
	{
		m_TOF_ON_Count=0;
		m_TOF_OFF_Count++;
		if(m_TOF_OFF_Count >= 3)
		{
			m_TOF_OFF_Count=3;
			m_pAlgo->m_cImgAnalysis.isDetectedTOF = FALSE;
		}
	}

#ifdef __G_DEBUG__
	TRACE("DAC : %02X, Current TOF(%4d) - thres(%4d) - Diff(%4d) isDetectedTOF : %d, A_TOF1 : %d, A_TOF2 : %d, A_TOF(%4d > %4d) m_isEnabledATOF : %d\n", 
		m_CurrentCaptureDAC, m_TOF_CurVal, m_TOF_Thres, m_TOF_CurVal-m_TOF_Thres, m_pAlgo->m_cImgAnalysis.isDetectedTOF, 
		m_TOF_Analog, m_TOF_Analog2, m_pAlgo->m_mainATOFValue, m_TOF_Analog_Thres, m_pAlgo->m_isEnabledATOF);
#endif

/*	if(m_pAlgo->m_isEnabledATOF == FALSE)
	{
		memset(InImg, 0, m_UsbDeviceInfo.CisImgSize);
	}*/

	if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize_Roll);
		
		m_pAlgo->m_cImgAnalysis.isDetected = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_forDetectOnly_Roll(InImg, 100);
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_Roll(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);

		// 지문 개수 루틴
		m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(Bright);
	}
	else
	{
		if(m_UsbDeviceInfo.bDecimation == TRUE)
		{
			memcpy(m_pAlgo->m_SBDAlg->m_OriginalImg, InImg, m_UsbDeviceInfo.CisImgSize/4);
			memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize/4);
		}
		else
		{
			for(i=0; i<m_UsbDeviceInfo.CisImgHeight/2; i++)
			{
				for(j=0; j<m_UsbDeviceInfo.CisImgWidth/2; j++)
				{
					m_pAlgo->m_SBDAlg->m_OriginalImg[i*m_UsbDeviceInfo.CisImgWidth/2+j] = InImg[(i*m_UsbDeviceInfo.CisImgWidth+j)*2];
				}
			}
			memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize);
		}
	
		m_pAlgo->m_cImgAnalysis.isDetected = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_forDetectOnly(m_pAlgo->m_SBDAlg->m_OriginalImg, 100);
		/*m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);
		m_pAlgo->SaveBitmapImage("d:\\segment1.bmp", m_pAlgo->m_segment_enlarge_buffer, 
			m_pAlgo->ZOOM_ENLAGE_W, m_pAlgo->ZOOM_ENLAGE_H, FALSE);*/
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_Rev1(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);
		//m_pAlgo->SaveBitmapImage("d:\\segment2.bmp", m_pAlgo->m_segment_enlarge_buffer, 
		//	m_pAlgo->ZOOM_ENLAGE_W, m_pAlgo->ZOOM_ENLAGE_H, FALSE);

		// 지문 개수 루틴
		m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(Bright);

#ifdef __G_DEBUG__
		TRACE("m_pAlgo->m_cImgAnalysis.mean : %d, m_SegmentBright : %d, DAC : %02X\n", m_pAlgo->m_cImgAnalysis.mean, m_pAlgo->m_SBDAlg->m_SegmentBright, m_DeciCaptureSetting.CurrentDAC);
#endif
		m_pAlgo->m_cImgAnalysis.mean = (m_pAlgo->m_cImgAnalysis.mean + m_pAlgo->m_SBDAlg->m_SegmentBright)/2;
	}

	m_Brightness = m_pAlgo->m_cImgAnalysis.mean;

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
    int savedFingerState = m_clbkProperty.nFingerState;
    if (m_pAlgo->m_cImgAnalysis.finger_count == 0)		// add 0.10.4 by enzyme
    {
        m_clbkProperty.nFingerState = ENUM_IBSU_NON_FINGER;
		
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

    if (!m_bFirstSent_clbkClearPlaten && savedFingerState != m_clbkProperty.nFingerState)
    {
        _PostCallback(CALLBACK_FINGER_COUNT);
    }
    ///////////////////////////////////////////////////////////////////////////

    if ((m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CONTRAST) == IBSU_OPTION_AUTO_CONTRAST)
    {
        if (m_UsbDeviceInfo.LeVoltageControl == SUPPORT_LE_VOLTAGE_CONTROL/* &&
			m_CaptureProgressStatus == ENUM_IBSM_CAPTURE_PROGRESS_CAPTURING*/)
        {
			if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			{
				m_pAlgo->_Algo_Kojak_AutoCapture_CurrentSensor_and_ATOF(&m_FullCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood);
				_ApplyCaptureSettings(m_FullCaptureSetting);
			}
			else
			{
				m_pAlgo->_Algo_Kojak_AutoCapture_CurrentSensor_and_ATOF(&m_DeciCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood);
				_ApplyCaptureSettings(m_DeciCaptureSetting);
			}

            if (m_bFirstSent_clbkClearPlaten)
                m_pAlgo->m_CaptureGood = FALSE;

			if(m_pAlgo->m_CaptureGood == TRUE)
			{
				if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
				{
					*bIsGoodImage = TRUE;
				}
				else
				{
					m_FullCaptureSetting.CurrentGain = m_DeciCaptureSetting.CurrentGain;
					m_FullCaptureSetting.CurrentDAC = m_DeciCaptureSetting.CurrentDAC;
					*bIsGoodImage = TRUE;
				}
			}
        }
    }
    else
    {
		if (m_UsbDeviceInfo.LeVoltageControl == SUPPORT_LE_VOLTAGE_CONTROL/* && 
				m_CaptureProgressStatus == ENUM_IBSM_CAPTURE_PROGRESS_CAPTURING*/)
		{
			m_pAlgo->_Algo_Kojak_AutoCapture_CurrentSensor(&m_FullCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood);
			m_DeciCaptureSetting.CurrentDAC = _GetContrast_FromGainTable(m_propertyInfo.nContrastValue, FALSE);
			_Set_CIS_GainRegister(m_propertyInfo.nContrastValue, FALSE, TRUE);
		}
    }

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
#endif


void CMainCapture::_PostImageProcessing_ForPreview_Kojak(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage)
{
#ifdef __embedded__
	if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		if(m_propertyInfo.bRollImageOverride == TRUE)
		{
			m_pAlgo->m_SBDAlg->m_pcImgAnalysis->LEFT = 1;
			m_pAlgo->m_SBDAlg->m_pcImgAnalysis->RIGHT = m_UsbDeviceInfo.CisImgWidth_Roll-2;
			m_pAlgo->m_SBDAlg->m_pcImgAnalysis->TOP = 1;
			m_pAlgo->m_SBDAlg->m_pcImgAnalysis->BOTTOM = m_UsbDeviceInfo.CisImgHeight_Roll-2;

			unsigned char *TmpBuf = (unsigned char *)m_pAlgo->m_SBDAlg->m_SumGrayBuf;
			memcpy(TmpBuf, m_pAlgo->m_SBDAlg->m_BestRollImg, m_UsbDeviceInfo.CisImgSize_Roll);
			for(int i=0; i<m_UsbDeviceInfo.CisImgSize_Roll; i++)
			{
				if(m_pAlgo->m_SBDAlg->m_BestRollImg[i] < InImg[i])
					TmpBuf[i] = InImg[i];
			}

			memcpy(InImg, TmpBuf, m_UsbDeviceInfo.CisImgSize_Roll);
		}

		m_pAlgo->m_SBDAlg->_Apply_SBD_preview(InImg, m_pAlgo->m_Inter_Img2);
		m_pAlgo->m_SBDAlg->_Make500DPI_preview_opt(m_pAlgo->m_Inter_Img2, OutImg);		// 자체적으로 invert 기능이 포함됨
	
//		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgSize_Roll);
	}
	else if( m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
  			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
	{
		int i, j;
		int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;
	
		dtLEFT		= m_pAlgo->m_cImgAnalysis.LEFT * m_UsbDeviceInfo.ImgWidth / m_UsbDeviceInfo.CisImgWidth;
		dtRIGHT		= m_pAlgo->m_cImgAnalysis.RIGHT * m_UsbDeviceInfo.ImgWidth / m_UsbDeviceInfo.CisImgWidth;
		dtTOP		= m_pAlgo->m_cImgAnalysis.TOP * m_UsbDeviceInfo.ImgHeight / m_UsbDeviceInfo.CisImgHeight;
		dtBOTTOM	= m_pAlgo->m_cImgAnalysis.BOTTOM * m_UsbDeviceInfo.ImgHeight / m_UsbDeviceInfo.CisImgHeight;
 
		m_pAlgo->m_SBDAlg->_Apply_SBD_preview(InImg, m_pAlgo->m_Inter_Img2);
		m_pAlgo->m_SBDAlg->_Make500DPI_preview_opt(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img);		// 자체적으로 invert 기능이 포함됨
		
//		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img, m_UsbDeviceInfo.ImgSize/4);

		memset(OutImg, 255, m_UsbDeviceInfo.ImgSize);
		for(i=dtTOP; i<dtBOTTOM; i++)
		{
			for(j=dtLEFT; j<dtRIGHT; j++)
			{
				OutImg[(i*2)*m_UsbDeviceInfo.ImgWidth+(j*2)] = m_pAlgo->m_Inter_Img[i*m_UsbDeviceInfo.ImgWidth/2+j];
				OutImg[(i*2)*m_UsbDeviceInfo.ImgWidth+(j*2+1)] = m_pAlgo->m_Inter_Img[i*m_UsbDeviceInfo.ImgWidth/2+j];
				OutImg[(i*2+1)*m_UsbDeviceInfo.ImgWidth+(j*2)] = m_pAlgo->m_Inter_Img[i*m_UsbDeviceInfo.ImgWidth/2+j];
				OutImg[(i*2+1)*m_UsbDeviceInfo.ImgWidth+(j*2+1)] = m_pAlgo->m_Inter_Img[i*m_UsbDeviceInfo.ImgWidth/2+j];
			}
		}

		//m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgSize);
	}
	else
		return;
#else
	if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		m_pAlgo->m_SBDAlg->m_pcImgAnalysis->LEFT = 1;
		m_pAlgo->m_SBDAlg->m_pcImgAnalysis->RIGHT = m_UsbDeviceInfo.CisImgWidth_Roll-2;
		m_pAlgo->m_SBDAlg->m_pcImgAnalysis->TOP = 1;
		m_pAlgo->m_SBDAlg->m_pcImgAnalysis->BOTTOM = m_UsbDeviceInfo.CisImgHeight_Roll-2;

		if(m_propertyInfo.bRollImageOverride == TRUE)
		{
			unsigned char *TmpBuf = (unsigned char *)m_pAlgo->m_SBDAlg->m_SumGrayBuf;
			memcpy(TmpBuf, m_pAlgo->m_SBDAlg->m_BestRollImg, m_UsbDeviceInfo.CisImgSize_Roll);
			for(int i=0; i<m_UsbDeviceInfo.CisImgSize_Roll; i++)
			{
				if(m_pAlgo->m_SBDAlg->m_BestRollImg[i] < InImg[i])
					TmpBuf[i] = InImg[i];
			}

			memcpy(InImg, TmpBuf, m_UsbDeviceInfo.CisImgSize_Roll);
		}

		m_pAlgo->m_SBDAlg->_VignettingEffect_preview_roll(InImg, m_pAlgo->m_Inter_Img, _NO_BIT_CMOS_NOISE_FOR_FULLFRAME_ROLL);
		m_pAlgo->_Algo_HistogramNormalize(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.CisImgSize_Roll, 0);
		//m_pAlgo->m_SBDAlg->_Apply_SBD_preview(InImg, m_pAlgo->m_Inter_Img2);
		m_pAlgo->m_SBDAlg->_Make500DPI_preview_opt(m_pAlgo->m_Inter_Img2, OutImg);		// 자체적으로 invert 기능이 포함됨

		m_pAlgo->_Algo_ConvertSegInfoToFinal(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2);
		memcpy(m_pAlgo->EnlargeBuf, m_pAlgo->m_Inter_Img2, m_pAlgo->ZOOM_ENLAGE_W_ROLL*m_pAlgo->ZOOM_ENLAGE_H_ROLL);
		memcpy(m_pAlgo->m_labeled_segment_enlarge_buffer, m_pAlgo->m_Inter_Img, m_pAlgo->ZOOM_ENLAGE_W_ROLL*m_pAlgo->ZOOM_ENLAGE_H_ROLL);
		m_pAlgo->_Algo_GetFingerCount(0);

//		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgSize_Roll);
	}
	else if( m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
  			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
	{
		if(m_propertyInfo.bEnableDecimation == TRUE)
		{
			int i, j;
			int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;
		
			dtLEFT		= m_pAlgo->m_cImgAnalysis.LEFT * m_UsbDeviceInfo.ImgWidth / m_UsbDeviceInfo.CisImgWidth;
			dtRIGHT		= m_pAlgo->m_cImgAnalysis.RIGHT * m_UsbDeviceInfo.ImgWidth / m_UsbDeviceInfo.CisImgWidth;
			dtTOP		= m_pAlgo->m_cImgAnalysis.TOP * m_UsbDeviceInfo.ImgHeight / m_UsbDeviceInfo.CisImgHeight;
			dtBOTTOM	= m_pAlgo->m_cImgAnalysis.BOTTOM * m_UsbDeviceInfo.ImgHeight / m_UsbDeviceInfo.CisImgHeight;
	 
			m_pAlgo->m_SBDAlg->_VignettingEffect_preview(InImg, m_pAlgo->m_Inter_Img, _DEFAULT_CMOS_NOISE_FOR_DECI_/2);
			m_pAlgo->_Algo_HistogramNormalize(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.CisImgSize/4, 0);
	//		m_pAlgo->m_SBDAlg->_Apply_SBD_preview(InImg, m_pAlgo->m_Inter_Img2);
			m_pAlgo->m_SBDAlg->_Make500DPI_preview_opt(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img);		// 자체적으로 invert 기능이 포함됨
			
	//		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img, m_UsbDeviceInfo.ImgSize/4);

			memset(OutImg, 255, m_UsbDeviceInfo.ImgSize);
			for(i=dtTOP; i<dtBOTTOM; i++)
			{
				for(j=dtLEFT; j<dtRIGHT; j++)
				{
					OutImg[(i*2)*m_UsbDeviceInfo.ImgWidth+(j*2)] = m_pAlgo->m_Inter_Img[i*m_UsbDeviceInfo.ImgWidth/2+j];
					OutImg[(i*2)*m_UsbDeviceInfo.ImgWidth+(j*2+1)] = m_pAlgo->m_Inter_Img[i*m_UsbDeviceInfo.ImgWidth/2+j];
					OutImg[(i*2+1)*m_UsbDeviceInfo.ImgWidth+(j*2)] = m_pAlgo->m_Inter_Img[i*m_UsbDeviceInfo.ImgWidth/2+j];
					OutImg[(i*2+1)*m_UsbDeviceInfo.ImgWidth+(j*2+1)] = m_pAlgo->m_Inter_Img[i*m_UsbDeviceInfo.ImgWidth/2+j];
				}
			}
			//m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgSize);
		}
		else
		{
			m_pAlgo->m_SBDAlg->m_pcImgAnalysis->LEFT = 1;
			m_pAlgo->m_SBDAlg->m_pcImgAnalysis->RIGHT = m_UsbDeviceInfo.CisImgWidth-2;
			m_pAlgo->m_SBDAlg->m_pcImgAnalysis->TOP = 1;
			m_pAlgo->m_SBDAlg->m_pcImgAnalysis->BOTTOM = m_UsbDeviceInfo.CisImgHeight-2;

			m_pAlgo->m_SBDAlg->_DistortionRestoration_with_Bilinear(InImg, OutImg);
			m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgSize);
		}

		m_pAlgo->_Algo_ConvertSegInfoToFinal(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2);
		memcpy(m_pAlgo->EnlargeBuf, m_pAlgo->m_Inter_Img2, m_pAlgo->ZOOM_ENLAGE_W*m_pAlgo->ZOOM_ENLAGE_H);
		memcpy(m_pAlgo->m_labeled_segment_enlarge_buffer, m_pAlgo->m_Inter_Img, m_pAlgo->ZOOM_ENLAGE_W*m_pAlgo->ZOOM_ENLAGE_H);
		m_pAlgo->_Algo_GetFingerCount(0);

		//m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgSize);
	}
#endif
}

void CMainCapture::_PostImageProcessing_ForResult_Kojak(BYTE *InImg, BYTE *OutImg)
{
	int i, j, x, y, val, Bright=0;

	m_pAlgo->m_SBDAlg->m_segment_arr_background_substract = &m_pAlgo->m_Inter_Img5[0];
	m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp = &m_pAlgo->m_Inter_Img5[m_pAlgo->CIS_IMG_SIZE/4];

	if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_Roll(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);

		m_pAlgo->m_SBDAlg->m_pcImgAnalysis->LEFT = 1;
		m_pAlgo->m_SBDAlg->m_pcImgAnalysis->RIGHT = m_UsbDeviceInfo.CisImgWidth_Roll-2;
		m_pAlgo->m_SBDAlg->m_pcImgAnalysis->TOP = 1;
		m_pAlgo->m_SBDAlg->m_pcImgAnalysis->BOTTOM = m_UsbDeviceInfo.CisImgHeight_Roll-2;
	}
	else
	{
		for(i=0; i<m_UsbDeviceInfo.CisImgHeight/2; i++)
		{
			for(j=0; j<m_UsbDeviceInfo.CisImgWidth/2; j++)
			{
				m_pAlgo->m_SBDAlg->m_OriginalImg[i*m_UsbDeviceInfo.CisImgWidth/2+j] = InImg[(i*m_UsbDeviceInfo.CisImgWidth+j)*2];
			}
		}

		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_Final(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);

		m_pAlgo->_Algo_Erosion(m_pAlgo->m_segment_enlarge_buffer_for_fingercnt, m_pAlgo->ZOOM_ENLAGE_W, m_pAlgo->ZOOM_ENLAGE_H);
		m_pAlgo->_Algo_Erosion(m_pAlgo->m_segment_enlarge_buffer_for_fingercnt, m_pAlgo->ZOOM_ENLAGE_W, m_pAlgo->ZOOM_ENLAGE_H);
		m_pAlgo->_Algo_Dilation(m_pAlgo->m_segment_enlarge_buffer_for_fingercnt, m_pAlgo->ZOOM_ENLAGE_W, m_pAlgo->ZOOM_ENLAGE_H);

		if (!((m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CONTRAST) == IBSU_OPTION_AUTO_CONTRAST))
		{
			int i, val, minus_val;//, j;

			if(m_pAlgo->m_cImgAnalysis.mean > 150)
				minus_val = 50;
			else if(m_pAlgo->m_cImgAnalysis.mean > 130)
				minus_val = 40;
			else if(m_pAlgo->m_cImgAnalysis.mean > 110)
				minus_val = 30;
			else if(m_pAlgo->m_cImgAnalysis.mean > 100)
				minus_val = 20;
			else if(m_pAlgo->m_cImgAnalysis.mean > 90)
				minus_val = 10;
			else
				minus_val = 0;

			if(minus_val > 0)
			{
				for(i=0; i<m_UsbDeviceInfo.CisImgSize; i++)
				{
					val = InImg[i] - minus_val;
					if(val < 0)
						val = 0;
					InImg[i] = val;
				}
			}
		}
	}

	m_pAlgo->m_SBDAlg->m_CMOS_NOISE_AMOUNT = m_pAlgo->_Algo_GetBackgroundNoiseAuto(InImg, m_pAlgo->m_segment_enlarge_buffer_for_fingercnt) + 15;//10;

#ifdef __G_DEBUG__
    TRACE("m_cImgAnalysis.noise_histo : %d\n", m_pAlgo->m_SBDAlg->m_CMOS_NOISE_AMOUNT);
#endif

	if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		memset(m_pAlgo->m_SBDAlg->m_segment_arr_background_substract, m_pAlgo->m_SBDAlg->m_CMOS_NOISE_AMOUNT, m_pAlgo->ZOOM_ENLAGE_W_ROLL*m_pAlgo->ZOOM_ENLAGE_H_ROLL);
		memset(m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp, m_pAlgo->m_SBDAlg->m_CMOS_NOISE_AMOUNT, m_pAlgo->ZOOM_ENLAGE_W_ROLL*m_pAlgo->ZOOM_ENLAGE_H_ROLL);

		for (y = m_pAlgo->ENLARGESIZE_ZOOM_H_ROLL; y < m_pAlgo->ZOOM_ENLAGE_H_ROLL - m_pAlgo->ENLARGESIZE_ZOOM_H_ROLL; y++)
		{
			for (x = m_pAlgo->ENLARGESIZE_ZOOM_W_ROLL; x < m_pAlgo->ZOOM_ENLAGE_W_ROLL - m_pAlgo->ENLARGESIZE_ZOOM_W_ROLL; x++)
			{
				if(m_pAlgo->m_segment_enlarge_buffer_for_fingercnt[y*m_pAlgo->ZOOM_ENLAGE_W_ROLL+x] == 255)
					m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[y*m_pAlgo->ZOOM_ENLAGE_W_ROLL+x] = 10;
			}
		}

		for (y = m_pAlgo->ENLARGESIZE_ZOOM_H_ROLL; y < m_pAlgo->ZOOM_ENLAGE_H_ROLL - m_pAlgo->ENLARGESIZE_ZOOM_H_ROLL; y++)
		{
			for (x = m_pAlgo->ENLARGESIZE_ZOOM_W_ROLL; x < m_pAlgo->ZOOM_ENLAGE_W_ROLL - m_pAlgo->ENLARGESIZE_ZOOM_W_ROLL; x++)
			{
				if(m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[y*m_pAlgo->ZOOM_ENLAGE_W_ROLL+x] == 10)
				{
					val = m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-2)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x-2)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-2)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x-1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-2)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-2)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x+1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-2)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x+2)];

					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-1)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x-2)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-1)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x-1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-1)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-1)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x+1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-1)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x+2)];

					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x-2)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x-1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x+1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x+2)];

					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+1)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x-2)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+1)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x-1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+1)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+1)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x+1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+1)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x+2)];

					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+2)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x-2)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+2)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x-1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+2)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+2)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x+1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+2)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x+2)];

					m_pAlgo->m_SBDAlg->m_segment_arr_background_substract[(y)*m_pAlgo->ZOOM_ENLAGE_W_ROLL+(x)] = val / 25;
				}
			}
		}

		m_pAlgo->m_SBDAlg->m_MinusMaskType = MINUS_MASK_MEAN;
		m_pAlgo->m_SBDAlg->m_TARGET_STRETCH_HISTOGRAM = 200 + m_pAlgo->m_SBDAlg->m_nContrastTres*5;
		if(m_pAlgo->m_SBDAlg->m_TARGET_STRETCH_HISTOGRAM > 255)
			m_pAlgo->m_SBDAlg->m_TARGET_STRETCH_HISTOGRAM = 255;
		if(m_propertyInfo.bReservedEnhanceResultImage)
			sprintf(m_pAlgo->m_SBDAlg->m_strUnsharpParam, "3.0");
		else
			sprintf(m_pAlgo->m_SBDAlg->m_strUnsharpParam, "%.1f", m_propertyInfo.nSharpenValue);
		m_pAlgo->m_SBDAlg->_Apply_SBD_final_opt(InImg, m_pAlgo->m_Inter_Img2);
		m_pAlgo->m_SBDAlg->_Make500DPI_opt(m_pAlgo->m_Inter_Img2, OutImg);

		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_Inter_Img2, m_pAlgo->m_final_image, m_UsbDeviceInfo.CisImgSize_Roll);
		memcpy(m_pAlgo->m_Inter_Img2, m_pAlgo->m_final_image, m_UsbDeviceInfo.CisImgSize_Roll);
		for(y=1; y<m_UsbDeviceInfo.CisImgHeight_Roll-1; y++)
		{
			for(x=1; x<m_UsbDeviceInfo.CisImgWidth_Roll-1; x++)
			{
				m_pAlgo->m_final_image[y*m_UsbDeviceInfo.CisImgWidth_Roll+x] = 
					(m_pAlgo->m_Inter_Img2[(y-1)*m_UsbDeviceInfo.CisImgWidth_Roll+x-1] +
					m_pAlgo->m_Inter_Img2[(y-1)*m_UsbDeviceInfo.CisImgWidth_Roll+x]*4 +
					m_pAlgo->m_Inter_Img2[(y-1)*m_UsbDeviceInfo.CisImgWidth_Roll+x+1] +
					m_pAlgo->m_Inter_Img2[y*m_UsbDeviceInfo.CisImgWidth_Roll+x-1]*4 +
					m_pAlgo->m_Inter_Img2[y*m_UsbDeviceInfo.CisImgWidth_Roll+x]*128 +
					m_pAlgo->m_Inter_Img2[y*m_UsbDeviceInfo.CisImgWidth_Roll+x+1]*4 +
					m_pAlgo->m_Inter_Img2[(y+1)*m_UsbDeviceInfo.CisImgWidth_Roll+x-1] +
					m_pAlgo->m_Inter_Img2[(y+1)*m_UsbDeviceInfo.CisImgWidth_Roll+x]*4 +
					m_pAlgo->m_Inter_Img2[(y+1)*m_UsbDeviceInfo.CisImgWidth_Roll+x+1])/148;
			}
		}

		memset(m_pAlgo->m_OutSplitResultArray, 0x00, m_UsbDeviceInfo.ImgSize_Roll*IBSU_MAX_SEGMENT_COUNT);
		memset(m_pAlgo->m_OutSplitResultArrayEx, 0x00, m_UsbDeviceInfo.ImgSize_Roll*IBSU_MAX_SEGMENT_COUNT);

		m_pAlgo->_Algo_ConvertSegInfoToFinal(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2);	
		m_pAlgo->_Algo_SegmentFinger_from_SegmentInfo_forRoll(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2, OutImg);

/*		m_pAlgo->_Algo_CheckAppendixFQuality(m_pAlgo->m_OutSplitResultArrayEx, 
											m_pAlgo->m_OutSplitResultArrayExWidth, 
											m_pAlgo->m_OutSplitResultArrayExHeight,
											m_pAlgo->m_segment_arr.SegmentCnt);*/

		if(m_propertyInfo.bWetFingerDetect)
		{
			m_pAlgo->_Algo_CheckWetFinger(m_pAlgo->m_OutSplitResultArrayEx, 
											m_pAlgo->m_OutSplitResultArrayExWidth, 
											m_pAlgo->m_OutSplitResultArrayExHeight,
											m_pAlgo->m_segment_arr.SegmentCnt);
		}

		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitResultArray, m_pAlgo->m_OutSplitResultArray, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize_Roll);
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitResultArrayEx, m_pAlgo->m_OutSplitResultArrayEx, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize_Roll);
		// Convert to Result image
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgSize_Roll);
	}
	else
	{
		memset(m_pAlgo->m_SBDAlg->m_segment_arr_background_substract, m_pAlgo->m_SBDAlg->m_CMOS_NOISE_AMOUNT, m_pAlgo->ZOOM_ENLAGE_W*m_pAlgo->ZOOM_ENLAGE_H);
		memset(m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp, m_pAlgo->m_SBDAlg->m_CMOS_NOISE_AMOUNT, m_pAlgo->ZOOM_ENLAGE_W*m_pAlgo->ZOOM_ENLAGE_H);

		for (y = m_pAlgo->ENLARGESIZE_ZOOM_H; y < m_pAlgo->ZOOM_ENLAGE_H - m_pAlgo->ENLARGESIZE_ZOOM_H; y++)
		{
			for (x = m_pAlgo->ENLARGESIZE_ZOOM_W; x < m_pAlgo->ZOOM_ENLAGE_W - m_pAlgo->ENLARGESIZE_ZOOM_W; x++)
			{
				if(m_pAlgo->m_segment_enlarge_buffer_for_fingercnt[y*m_pAlgo->ZOOM_ENLAGE_W+x] == 255)
					m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[y*m_pAlgo->ZOOM_ENLAGE_W+x] = 10;
			}
		}

		for (y = m_pAlgo->ENLARGESIZE_ZOOM_H; y < m_pAlgo->ZOOM_ENLAGE_H - m_pAlgo->ENLARGESIZE_ZOOM_H; y++)
		{
			for (x = m_pAlgo->ENLARGESIZE_ZOOM_W; x < m_pAlgo->ZOOM_ENLAGE_W - m_pAlgo->ENLARGESIZE_ZOOM_W; x++)
			{
				if(m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[y*m_pAlgo->ZOOM_ENLAGE_W+x] == 10)
				{
					val = m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-2)*m_pAlgo->ZOOM_ENLAGE_W+(x-2)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-2)*m_pAlgo->ZOOM_ENLAGE_W+(x-1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-2)*m_pAlgo->ZOOM_ENLAGE_W+(x)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-2)*m_pAlgo->ZOOM_ENLAGE_W+(x+1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-2)*m_pAlgo->ZOOM_ENLAGE_W+(x+2)];

					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-1)*m_pAlgo->ZOOM_ENLAGE_W+(x-2)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-1)*m_pAlgo->ZOOM_ENLAGE_W+(x-1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-1)*m_pAlgo->ZOOM_ENLAGE_W+(x)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-1)*m_pAlgo->ZOOM_ENLAGE_W+(x+1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y-1)*m_pAlgo->ZOOM_ENLAGE_W+(x+2)];

					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y)*m_pAlgo->ZOOM_ENLAGE_W+(x-2)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y)*m_pAlgo->ZOOM_ENLAGE_W+(x-1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y)*m_pAlgo->ZOOM_ENLAGE_W+(x)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y)*m_pAlgo->ZOOM_ENLAGE_W+(x+1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y)*m_pAlgo->ZOOM_ENLAGE_W+(x+2)];

					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+1)*m_pAlgo->ZOOM_ENLAGE_W+(x-2)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+1)*m_pAlgo->ZOOM_ENLAGE_W+(x-1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+1)*m_pAlgo->ZOOM_ENLAGE_W+(x)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+1)*m_pAlgo->ZOOM_ENLAGE_W+(x+1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+1)*m_pAlgo->ZOOM_ENLAGE_W+(x+2)];

					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+2)*m_pAlgo->ZOOM_ENLAGE_W+(x-2)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+2)*m_pAlgo->ZOOM_ENLAGE_W+(x-1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+2)*m_pAlgo->ZOOM_ENLAGE_W+(x)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+2)*m_pAlgo->ZOOM_ENLAGE_W+(x+1)];
					val += m_pAlgo->m_SBDAlg->m_segment_arr_background_substract_tmp[(y+2)*m_pAlgo->ZOOM_ENLAGE_W+(x+2)];

					m_pAlgo->m_SBDAlg->m_segment_arr_background_substract[(y)*m_pAlgo->ZOOM_ENLAGE_W+(x)] = val / 25;
				}
			}
		}

		m_pAlgo->m_SBDAlg->m_MinusMaskType = MINUS_MASK_MEAN;
		m_pAlgo->m_SBDAlg->m_TARGET_STRETCH_HISTOGRAM = 200 + m_pAlgo->m_SBDAlg->m_nContrastTres*5;
		if(m_pAlgo->m_SBDAlg->m_TARGET_STRETCH_HISTOGRAM > 255)
			m_pAlgo->m_SBDAlg->m_TARGET_STRETCH_HISTOGRAM = 255; 
		if(m_propertyInfo.bReservedEnhanceResultImage)
			sprintf(m_pAlgo->m_SBDAlg->m_strUnsharpParam, "3.0");
		else
			sprintf(m_pAlgo->m_SBDAlg->m_strUnsharpParam, "%.1f", m_propertyInfo.nSharpenValue);
		m_pAlgo->m_SBDAlg->_Apply_SBD_final_opt(InImg, m_pAlgo->m_Inter_Img2);
		m_pAlgo->m_SBDAlg->_Make500DPI_opt(m_pAlgo->m_Inter_Img2, OutImg);

		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_Inter_Img2, m_pAlgo->m_final_image, m_UsbDeviceInfo.CisImgSize);

		memcpy(m_pAlgo->m_Inter_Img2, m_pAlgo->m_final_image, m_UsbDeviceInfo.CisImgSize);
		for(y=1; y<m_UsbDeviceInfo.CisImgHeight-1; y++)
		{
			for(x=1; x<m_UsbDeviceInfo.CisImgWidth-1; x++)
			{
				m_pAlgo->m_final_image[y*m_UsbDeviceInfo.CisImgWidth+x] = 
					(m_pAlgo->m_Inter_Img2[(y-1)*m_UsbDeviceInfo.CisImgWidth+x-1] +
					m_pAlgo->m_Inter_Img2[(y-1)*m_UsbDeviceInfo.CisImgWidth+x]*4 +
					m_pAlgo->m_Inter_Img2[(y-1)*m_UsbDeviceInfo.CisImgWidth+x+1] +
					m_pAlgo->m_Inter_Img2[y*m_UsbDeviceInfo.CisImgWidth+x-1]*4 +
					m_pAlgo->m_Inter_Img2[y*m_UsbDeviceInfo.CisImgWidth+x]*128 +
					m_pAlgo->m_Inter_Img2[y*m_UsbDeviceInfo.CisImgWidth+x+1]*4 +
					m_pAlgo->m_Inter_Img2[(y+1)*m_UsbDeviceInfo.CisImgWidth+x-1] +
					m_pAlgo->m_Inter_Img2[(y+1)*m_UsbDeviceInfo.CisImgWidth+x]*4 +
					m_pAlgo->m_Inter_Img2[(y+1)*m_UsbDeviceInfo.CisImgWidth+x+1])/148;
			}
		}

		// Algo split image
		memset(m_pAlgo->m_OutSplitResultArray, 0x00, m_UsbDeviceInfo.ImgSize*IBSU_MAX_SEGMENT_COUNT);
		memset(m_pAlgo->m_OutSplitResultArrayEx, 0x00, m_UsbDeviceInfo.ImgSize*IBSU_MAX_SEGMENT_COUNT);
		
		m_pAlgo->_Algo_ConvertSegInfoToFinal(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2);	
		m_pAlgo->_Algo_SegmentFinger_from_SegmentInfo(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2, OutImg);

/*		m_pAlgo->_Algo_CheckAppendixFQuality(m_pAlgo->m_OutSplitResultArrayEx, 
											m_pAlgo->m_OutSplitResultArrayExWidth, 
											m_pAlgo->m_OutSplitResultArrayExHeight,
											m_pAlgo->m_segment_arr.SegmentCnt);*/

		if(m_propertyInfo.bWetFingerDetect)
		{
			m_pAlgo->_Algo_CheckWetFinger(m_pAlgo->m_OutSplitResultArrayEx, 
											m_pAlgo->m_OutSplitResultArrayExWidth, 
											m_pAlgo->m_OutSplitResultArrayExHeight,
											m_pAlgo->m_segment_arr.SegmentCnt);
		}

		// Convert to Splited image
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitResultArray, m_pAlgo->m_OutSplitResultArray, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize);
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitResultArrayEx, m_pAlgo->m_OutSplitResultArrayEx, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize);
		// Convert to Result image
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);
	}
}

int CMainCapture::_GoJob_DummyCapture_ForThread_Kojak(BYTE *InImg)
{
	int			finger_count = 0;
	int			Bright = 0;
	int			i, j;

	if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_Roll(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);
//		if(m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER)	// 결정이 필요한 사항
//			m_pAlgo->m_cImgAnalysis.mean = (m_pAlgo->m_SBDAlg->m_SegmentBright+m_pAlgo->m_cImgAnalysis.mean)/2;

		// 지문 개수 루틴
		m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(Bright);
	}
	else
	{
		if(m_UsbDeviceInfo.bDecimation == TRUE)
		{
			memcpy(m_pAlgo->m_Inter_Img5, InImg, m_UsbDeviceInfo.CisImgSize/4);
		}
		else
		{
			for(i=0; i<m_UsbDeviceInfo.CisImgHeight/2; i++)
			{
				for(j=0; j<m_UsbDeviceInfo.CisImgWidth/2; j++)
				{
                    m_pAlgo->m_Inter_Img5[i*m_UsbDeviceInfo.CisImgWidth/2+j] = InImg[(i*m_UsbDeviceInfo.CisImgWidth+j)*2];
				}
			}
		}
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage(m_pAlgo->m_Inter_Img5, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);
//	m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->m_SBDAlg->m_SegmentBright;

		// 지문 개수 루틴
		finger_count = m_pAlgo->_Algo_GetFingerCount(Bright);
	}

	int tmp_isDetectedTOF = _Check_TOF(&m_TOF_CurVal, &m_TOF_Thres);

	if(tmp_isDetectedTOF)
	{
		m_TOF_OFF_Count=0;
		m_TOF_ON_Count++;
		if(m_TOF_ON_Count >= 3)
		{
			m_TOF_ON_Count=3;
			m_pAlgo->m_cImgAnalysis.isDetectedTOF = TRUE;
		}
	}
	else
	{
		m_TOF_ON_Count=0;
		m_TOF_OFF_Count++;
		if(m_TOF_OFF_Count >= 3)
		{
			m_TOF_OFF_Count=3;
			m_pAlgo->m_cImgAnalysis.isDetectedTOF = FALSE;
		}
	}
	
	//TRACE("===> tmp_isDetectedTOF : %d, isDetectedTOF : %d, (%d / %d)\n", 
	//	tmp_isDetectedTOF, m_pAlgo->m_cImgAnalysis.isDetectedTOF, m_TOF_CurVal, m_TOF_Thres);

	if( finger_count == 0 && m_pAlgo->m_cImgAnalysis.isDetectedTOF == FALSE)
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

#ifdef __DUMMY_DETECT_FINGER_USING_ATOF__
int CMainCapture::_GoJob_DummyCapture_ForThread_Kojak_ATOF(BYTE *InImg)
{
	int tmp_isDetectedTOF = _Check_TOF(&m_TOF_CurVal, &m_TOF_Thres);
	m_pAlgo->m_mainTOFValue = m_TOF_CurVal;

	// Calc difference of Analog TOF
	int ATOF_Base = m_TOF_Analog2 + (m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_sign*m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_Diff);

	m_pAlgo->m_mainATOFValue = (int)((float)((m_TOF_Analog - ATOF_Base) * m_propertyInfo.sKojakCalibrationInfo.ATOF_LEOFF_slope) / 1000.0f);
	if(m_pAlgo->m_mainATOFValue < 0)
		m_pAlgo->m_mainATOFValue = 0;

	if(m_propertyInfo.nNumberOfObjects > 1)
		m_TOF_Analog_Thres = 250 + 150 * (m_propertyInfo.nNumberOfObjects-1);
	else
		m_TOF_Analog_Thres = 250;

	if( m_pAlgo->m_mainATOFValue > m_TOF_Analog_Thres)
	{
		m_ATOF_OFF_Count=0;
		m_ATOF_ON_Count++;
		
		if(m_ATOF_ON_Count >= 3)
		{
			m_ATOF_ON_Count=3;
			m_pAlgo->m_isEnabledATOF = TRUE;
		}
	}
	else
	{
		m_ATOF_ON_Count=0;
		m_ATOF_OFF_Count++;
		if(m_ATOF_OFF_Count >= 3)
		{
			m_ATOF_OFF_Count=3;
			m_pAlgo->m_isEnabledATOF = FALSE;
		}
	}

	if( m_pAlgo->m_isEnabledATOF == FALSE)
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
#endif

void CMainCapture::_GoJob_Initialize_ForThread_Kojak()
{
	int WaterImageSize = m_UsbDeviceInfo.nWaterImageSize;

//    BYTE *WaterImage = new BYTE[WaterImageSize + 4 /* +4 is checksum */];
    BYTE *WaterImage = m_pAlgo->m_WaterImage;

	m_UsbDeviceInfo.nFpgaVersion = _GetFpgaVersion(TRUE);

    memset(WaterImage, 0, WaterImageSize);
    if (m_UsbDeviceInfo.bNeedMask)
    {
        if (m_UsbDeviceInfo.nFpgaVersion >= ((0 << 24) | (9 << 16) | (0 << 8) | 0x0D) )
        {
            _Algo_Kojak_GetWaterMaskFromFlashMemory(WaterImage, WaterImageSize);
        }
    }
	
	// Change ADC Thres to 0xFE(Maximum) so we can use DAC from 0xFF(100V) to 0x40(300V)
	_SetADCThres(0xFE);

	// Set to use internal transformer 
	_UsbBulkOutIn(EP1OUT, 0x20, 0, 0, -1, NULL, 0, NULL);

    /* This must be done before making the uniformity mask. */
    m_pAlgo->_Algo_Kojak_GetDistortionMask_FromSavedFile();
    //m_pAlgo->_Algo_MakeUniformityMask(WaterImage);
	m_pAlgo->m_SBDAlg->_Init_SBD_final(WaterImage);
//    delete [] WaterImage; 

/*	int voltageValue;
	Capture_SetLEVoltage(__DEFAULT_VOLTAGE_VALUE_FOR_KOJAK__);
    Capture_GetLEVoltage(&voltageValue);
    if (voltageValue == __DEFAULT_VOLTAGE_VALUE_FOR_KOJAK__)
    {
        m_UsbDeviceInfo.LeVoltageControl = SUPPORT_LE_VOLTAGE_CONTROL;
    }
	else
	{
		m_UsbDeviceInfo.LeVoltageControl = NOT_SUPPORT_LE_VOLTAGE_CONTROL;
	}
*/
	m_UsbDeviceInfo.LeVoltageControl = SUPPORT_LE_VOLTAGE_CONTROL;

	m_pAlgo->_Algo_Init_GammaTable();

	m_Send_CaptureStart_BtnStat = -1;

	if(m_UsbDeviceInfo.bCanUseTOF == FALSE)
	{
		m_propertyInfo.bEnableTOF = FALSE;
		m_propertyInfo.bEnableTOFforROLL = FALSE;
	}

	if(m_UsbDeviceInfo.nFpgaVersion == _FPGA_VER_FOR_KOJAK_PL_WITH_DOUBLE_P_ &&
		strncmp(m_propertyInfo.cSerialNumber, "1715", 4) >= 0 &&
		strncmp(m_propertyInfo.cSerialNumber, "1716", 4) < 0 )
	{
		m_propertyInfo.bKojakPLwithDPFilm = TRUE;
		_PostTraceLogCallback(0, "=====> Detected Kojak PL with DP");
	}

	_SndUsbFwCaptureStop();
	_SndUsbFwCaptureStart();
	_InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
}

int CMainCapture::_OnOff_LE_Kojak(unsigned char on_off)
{
	if(m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
		return IBSU_ERR_NOT_SUPPORTED;

	int nRC = IBSU_ERR_COMMAND_FAILED;

	if(on_off == 0)			// LE OFF
		nRC = _UsbBulkOutIn(EP1OUT, CMD_KOJAK_LE_OFF, NULL, 0, -1, NULL, 0, NULL);
	else if(on_off == 1)	// LE ON
	{
		_UsbBulkOutIn(EP1OUT, CMD_KOJAK_MASK_RELAY_OFF, NULL, 0, -1, NULL, 0, NULL);
		Sleep(100);
		nRC = _UsbBulkOutIn(EP1OUT, CMD_KOJAK_LE_ON, NULL, 0, -1, NULL, 0, NULL);
		Sleep(100);
	}
	else
		nRC = IBSU_ERR_INVALID_PARAM_VALUE;

	return nRC;
}

/*int CMainCapture::_ClearButtonStat_Kojak(unsigned char btn_idx)
{
	if(m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
		return IBSU_ERR_NOT_SUPPORTED;

	int nRC;
	unsigned char write_buf[4];
	write_buf[0] = btn_idx;

	nRC = _UsbBulkOutIn(EP1OUT, CMD_KOJAK_CLEAR_BUTTON_STAT, write_buf, 1, -1, NULL, 0, NULL);

	return nRC;
}
*/

int CMainCapture::_GetButtonStat_Kojak(unsigned char *btn1_stat, unsigned char *btn2_stat)
{
	if(m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
		return IBSU_ERR_NOT_SUPPORTED;

	int nRC;
	LONG readbyte=0;
	unsigned char read_buf[4];

	*btn1_stat = 0;
	*btn2_stat = 0;

	nRC = _UsbBulkOutIn(EP1OUT, CMD_KOJAK_READ_BUTTON_STAT, NULL, 0, EP1IN, read_buf, 2, &readbyte);

	if(nRC == IBSU_STATUS_OK && readbyte == 2)
	{
		*btn1_stat = read_buf[0];
		*btn2_stat = read_buf[1];
	}
	
	return nRC;
}

/*int CMainCapture::_PowerOffButton_Kojak(unsigned char btn_idx)
{
	if(m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
		return IBSU_ERR_NOT_SUPPORTED;

	int nRC;
	unsigned char write_buf[4];
	write_buf[0] = btn_idx;

	nRC = _UsbBulkOutIn(EP1OUT, CMD_KOJAK_POWER_OFF_BUTTON, write_buf, 1, -1, NULL, 0, NULL);

	return nRC;
}
*/

void CMainCapture::_SetDryCapture_Kojak()
{
	m_pAlgo->m_DryCapture = TRUE;
}

int CMainCapture::_SetActiveLEDs_Kojak(WORD addr, DWORD val)
{
	KOJAK_LED activeLEDs;
	int blinkLEDs = 0;

	memset(&activeLEDs, 0, sizeof(KOJAK_LED));

	if (((val & IBSU_LED_F_BLINK_GREEN) == IBSU_LED_F_BLINK_GREEN) ||
		((val & IBSU_LED_F_BLINK_RED) == IBSU_LED_F_BLINK_RED))
	{
		blinkLEDs = 8; 
	}

	if ((val & IBSU_LED_F_PROGRESS_ROLL) == IBSU_LED_F_PROGRESS_ROLL)
	{
		activeLEDs.Roll = 1;
	}
	if ((val & IBSU_LED_F_PROGRESS_LEFT_HAND) == IBSU_LED_F_PROGRESS_LEFT_HAND)
	{
		activeLEDs.LeftProgress = 1;
	}
	if ((val & IBSU_LED_F_PROGRESS_TWO_THUMB) == IBSU_LED_F_PROGRESS_TWO_THUMB)
	{
		activeLEDs.ThumbProgress = 1;
	}
	if ((val & IBSU_LED_F_PROGRESS_RIGHT_HAND) == IBSU_LED_F_PROGRESS_RIGHT_HAND)
	{
		activeLEDs.RightProgress = 1;
	}

	if (((val & IBSU_LED_F_LEFT_LITTLE_GREEN) == IBSU_LED_F_LEFT_LITTLE_GREEN) ||
		((val & IBSU_LED_F_LEFT_LITTLE_RED) == IBSU_LED_F_LEFT_LITTLE_RED))
	{
		if ((val & IBSU_LED_F_LEFT_LITTLE_GREEN) == IBSU_LED_F_LEFT_LITTLE_GREEN)
			activeLEDs.LeftLittle |= 1;
		if ((val & IBSU_LED_F_LEFT_LITTLE_RED) == IBSU_LED_F_LEFT_LITTLE_RED)
			activeLEDs.LeftLittle |= 2;
		activeLEDs.LeftLittle |= 4;
		activeLEDs.LeftLittle |= blinkLEDs;
	}

	if (((val & IBSU_LED_F_LEFT_RING_GREEN) == IBSU_LED_F_LEFT_RING_GREEN) ||
		((val & IBSU_LED_F_LEFT_RING_RED) == IBSU_LED_F_LEFT_RING_RED))
	{
		if ((val & IBSU_LED_F_LEFT_RING_GREEN) == IBSU_LED_F_LEFT_RING_GREEN)
			activeLEDs.LeftRing |= 1;
		if ((val & IBSU_LED_F_LEFT_RING_RED) == IBSU_LED_F_LEFT_RING_RED)
			activeLEDs.LeftRing |= 2;
		activeLEDs.LeftRing |= 4;
		activeLEDs.LeftRing |= blinkLEDs;
	}

	if (((val & IBSU_LED_F_LEFT_MIDDLE_GREEN) == IBSU_LED_F_LEFT_MIDDLE_GREEN) ||
		((val & IBSU_LED_F_LEFT_MIDDLE_RED) == IBSU_LED_F_LEFT_MIDDLE_RED))
	{
		if ((val & IBSU_LED_F_LEFT_MIDDLE_GREEN) == IBSU_LED_F_LEFT_MIDDLE_GREEN)
			activeLEDs.LeftMiddle |= 1;
		if ((val & IBSU_LED_F_LEFT_MIDDLE_RED) == IBSU_LED_F_LEFT_MIDDLE_RED)
			activeLEDs.LeftMiddle |= 2;
		activeLEDs.LeftMiddle |= 4;
		activeLEDs.LeftMiddle |= blinkLEDs;
	}

	if (((val & IBSU_LED_F_LEFT_INDEX_GREEN) == IBSU_LED_F_LEFT_INDEX_GREEN) ||
		((val & IBSU_LED_F_LEFT_INDEX_RED) == IBSU_LED_F_LEFT_INDEX_RED))
	{
		if ((val & IBSU_LED_F_LEFT_INDEX_GREEN) == IBSU_LED_F_LEFT_INDEX_GREEN)
			activeLEDs.LeftIndex |= 1;
		if ((val & IBSU_LED_F_LEFT_INDEX_RED) == IBSU_LED_F_LEFT_INDEX_RED)
			activeLEDs.LeftIndex |= 2;
		activeLEDs.LeftIndex |= 4;
		activeLEDs.LeftIndex |= blinkLEDs;
	}

	if (((val & IBSU_LED_F_LEFT_THUMB_GREEN) == IBSU_LED_F_LEFT_THUMB_GREEN) ||
		((val & IBSU_LED_F_LEFT_THUMB_RED) == IBSU_LED_F_LEFT_THUMB_RED))
	{
		if ((val & IBSU_LED_F_LEFT_THUMB_GREEN) == IBSU_LED_F_LEFT_THUMB_GREEN)
			activeLEDs.LeftThumb |= 1;
		if ((val & IBSU_LED_F_LEFT_THUMB_RED) == IBSU_LED_F_LEFT_THUMB_RED)
			activeLEDs.LeftThumb |= 2;
		activeLEDs.LeftThumb |= 4;
		activeLEDs.LeftThumb |= blinkLEDs;
	}

	if (((val & IBSU_LED_F_RIGHT_THUMB_GREEN) == IBSU_LED_F_RIGHT_THUMB_GREEN) ||
		((val & IBSU_LED_F_RIGHT_THUMB_RED) == IBSU_LED_F_RIGHT_THUMB_RED))
	{
		if ((val & IBSU_LED_F_RIGHT_THUMB_GREEN) == IBSU_LED_F_RIGHT_THUMB_GREEN)
			activeLEDs.RightThumb |= 1;
		if ((val & IBSU_LED_F_RIGHT_THUMB_RED) == IBSU_LED_F_RIGHT_THUMB_RED)
			activeLEDs.RightThumb |= 2;
		activeLEDs.RightThumb |= 4;
		activeLEDs.RightThumb |= blinkLEDs;
	}

	if (((val & IBSU_LED_F_RIGHT_INDEX_GREEN) == IBSU_LED_F_RIGHT_INDEX_GREEN) ||
		((val & IBSU_LED_F_RIGHT_INDEX_RED) == IBSU_LED_F_RIGHT_INDEX_RED))
	{
		if ((val & IBSU_LED_F_RIGHT_INDEX_GREEN) == IBSU_LED_F_RIGHT_INDEX_GREEN)
			activeLEDs.RightIndex |= 1;
		if ((val & IBSU_LED_F_RIGHT_INDEX_RED) == IBSU_LED_F_RIGHT_INDEX_RED)
			activeLEDs.RightIndex |= 2;
		activeLEDs.RightIndex |= 4;
		activeLEDs.RightIndex |= blinkLEDs;
	}

	if (((val & IBSU_LED_F_RIGHT_MIDDLE_GREEN) == IBSU_LED_F_RIGHT_MIDDLE_GREEN) ||
		((val & IBSU_LED_F_RIGHT_MIDDLE_RED) == IBSU_LED_F_RIGHT_MIDDLE_RED))
	{
		if ((val & IBSU_LED_F_RIGHT_MIDDLE_GREEN) == IBSU_LED_F_RIGHT_MIDDLE_GREEN)
			activeLEDs.RightMiddle |= 1;
		if ((val & IBSU_LED_F_RIGHT_MIDDLE_RED) == IBSU_LED_F_RIGHT_MIDDLE_RED)
			activeLEDs.RightMiddle |= 2;
		activeLEDs.RightMiddle |= 4;
		activeLEDs.RightMiddle |= blinkLEDs;
	}

	if (((val & IBSU_LED_F_RIGHT_RING_GREEN) == IBSU_LED_F_RIGHT_RING_GREEN) ||
		((val & IBSU_LED_F_RIGHT_RING_RED) == IBSU_LED_F_RIGHT_RING_RED))
	{
		if ((val & IBSU_LED_F_RIGHT_RING_GREEN) == IBSU_LED_F_RIGHT_RING_GREEN)
			activeLEDs.RightRing |= 1;
		if ((val & IBSU_LED_F_RIGHT_RING_RED) == IBSU_LED_F_RIGHT_RING_RED)
			activeLEDs.RightRing |= 2;
		activeLEDs.RightRing |= 4;
		activeLEDs.RightRing |= blinkLEDs;
	}

	if (((val & IBSU_LED_F_RIGHT_LITTLE_GREEN) == IBSU_LED_F_RIGHT_LITTLE_GREEN) ||
		((val & IBSU_LED_F_RIGHT_LITTLE_RED) == IBSU_LED_F_RIGHT_LITTLE_RED))
	{
		if ((val & IBSU_LED_F_RIGHT_LITTLE_GREEN) == IBSU_LED_F_RIGHT_LITTLE_GREEN)
			activeLEDs.RightLittle |= 1;
		if ((val & IBSU_LED_F_RIGHT_LITTLE_RED) == IBSU_LED_F_RIGHT_LITTLE_RED)
			activeLEDs.RightLittle |= 2;
		activeLEDs.RightLittle |= 4;
		activeLEDs.RightLittle |= blinkLEDs;
	}

	return _UsbBulkOutIn(EP1OUT, CMD_KOJAK_WRITE_LED, (unsigned char*)&activeLEDs, 6, -1, NULL, 0, NULL);
}

void CMainCapture::_ChangeDecimationModeForKojak(BOOL DecimationMode, CaptureSetting *CurrentCaptureSetting, BOOL ApplyChangeSetting)
{
	int decimation = 0x0000;
	int SetFPGAImgSize = m_UsbDeviceInfo.CisImgSize;

	// Send row / colum decimation
//	UCHAR		outBuffer[64] = {0};

	if(DecimationMode) // Decimation ON
	{
		decimation = 0x0001;
		SetFPGAImgSize = m_UsbDeviceInfo.CisImgSize/4;
	}
	else // Decimation OFF
	{
		decimation = 0x0000;
		if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			SetFPGAImgSize = m_UsbDeviceInfo.CisImgSize_Roll;
		else
			SetFPGAImgSize = m_UsbDeviceInfo.CisImgSize;
	}

	// optimize onStopImaging()
	//Sleep(100);  //removed
	
	//-----------------------------------------------------
	_FPGA_SetRegister(0x00, 0x01);	// set reset
	_FPGA_SetRegister(0x01, 0x00);	// clear capture flag

	_FPGA_SetRegister(0x07, (UCHAR)(SetFPGAImgSize & 0xFF));
	_FPGA_SetRegister(0x08, (UCHAR)((SetFPGAImgSize >> 8) & 0xFF));
	_FPGA_SetRegister(0x09, (UCHAR)((SetFPGAImgSize >> 16) & 0xFF));


	// optimize onStopImaging()
    /* removed
    int width, height;
	if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
    {
        width = m_UsbDeviceInfo.CisImgWidth_Roll;
        height = m_UsbDeviceInfo.CisImgHeight_Roll;
	    _MT9M_SetRegister(MT9M_ROW_START,			(1944-m_UsbDeviceInfo.CisImgHeight)/2+35 + m_propertyInfo.nStartingPositionOfRollArea*100+m_propertyInfo.sKojakCalibrationInfo.LensPosRow);//+94);   // 54
	    _MT9M_SetRegister(MT9M_COLUMN_START,        (2592-width)/2+30-m_propertyInfo.sKojakCalibrationInfo.LensPosCol);//+40);    // 32
	    _MT9M_SetRegister(MT9M_ROW_SIZE,            height-1);   // 1079
	    _MT9M_SetRegister(MT9M_COLUMN_SIZE,         width-1);    // 1439
    }
    else
    {
        width = m_UsbDeviceInfo.CisImgWidth;
        height = m_UsbDeviceInfo.CisImgHeight;
	    _MT9M_SetRegister(MT9M_ROW_START,			(1944-height)/2+35+m_propertyInfo.sKojakCalibrationInfo.LensPosRow);//+94);   // 54
	    _MT9M_SetRegister(MT9M_COLUMN_START,        (2592-width)/2+30-m_propertyInfo.sKojakCalibrationInfo.LensPosCol);//+40);    // 32
	    _MT9M_SetRegister(MT9M_ROW_SIZE,            height-1);   // 1079
	    _MT9M_SetRegister(MT9M_COLUMN_SIZE,         width-1);    // 1439
    }
	*/

	// row decimation
	_MT9M_SetRegister(0x0022, decimation);
	
	// column decimation
	_MT9M_SetRegister(0x0023, decimation);

	if(ApplyChangeSetting)
		_ApplyCaptureSettings(*CurrentCaptureSetting);

	// optimize onStopImaging()
    /* removed
	_MT9M_SetRegister(0x70, 0x00FF);	// RD_SEQ_LEN
	_MT9M_SetRegister(0x71, 0xA700); 	// RD_PTR_TIMING
	_MT9M_SetRegister(0x72, 0xA700); 	// RD_ROW_EN_TIMING
	_MT9M_SetRegister(0x73, 0x0C00); 	// RD_RST_EN_TIMING
	_MT9M_SetRegister(0x74, 0x0600); 	// RD_BOOST_RST_TIMING
	_MT9M_SetRegister(0x75, 0x5617); 	// RD_SHR_TIMING
	_MT9M_SetRegister(0x76, 0x6B57); 	// RD_TX_EN_TIMING
	_MT9M_SetRegister(0x77, 0x6B57); 	// RD_BOOST_TX_TIMING
	_MT9M_SetRegister(0x78, 0xA500); 	// RD_SHS_TIMING
	_MT9M_SetRegister(0x79, 0xAB00); 	// RD_VCL_COL_EN_TIMING
	_MT9M_SetRegister(0x7A, 0xA904); 	// RD_COLCLAMP_TIMING
	_MT9M_SetRegister(0x7B, 0xA700); 	// RD_SHVCL_BAR_TIMING
	_MT9M_SetRegister(0x7C, 0xA700); 	// RD_VLN_EN_TIMING
	_MT9M_SetRegister(0x7E, 0xA900); 	// RD_BOOST_ROW_TIMING
	_MT9M_SetRegister(0x7F, 0x0000); 	// RD_EN_VLN_SH_TIMING
	_MT9M_SetRegister(0x29, 0x0480); 	// DAC_CONTROL_2_REG
	_MT9M_SetRegister(0x3E, 0x80C7); 	// TX_LOW_VOLTAGE
	_MT9M_SetRegister(0x3F, 0x0004); 	// RST_LOW_VOLTAGE
	_MT9M_SetRegister(0x41, 0x0000);	// CLAMP_VOLTAGE
	_MT9M_SetRegister(0x48, 0x0010); 	// VREF_AMP_BIAS_CURRENT
	_MT9M_SetRegister(0x5F, 0x231D); 	// CAL_THRESHOLD   
	_MT9M_SetRegister(0x57, 0x0004); 	// ANTI_ECLIPSE_VOLTAGE
	_MT9M_SetRegister(0x2A, 0x1086); 	// DAC_CONTROL_3
	*/

/*	// Add CIS patch
	//==========================================================
	// MT9P031 rolling artifack optimization
	//==========================================================
	_MT9M_SetRegister(0x70, 0x00FF); 	// RD_SEQ_LEN
//	_MT9M_SetRegister(0x70, 0x0079); 	// RD_SEQ_LEN
	_MT9M_SetRegister(0x71, 0x7800); 	// RD_PTR_TIMING
	_MT9M_SetRegister(0x72, 0x7800); 	// RD_ROW_EN_TIMING
	_MT9M_SetRegister(0x73, 0x0300); 	// RD_RST_EN_TIMING
	_MT9M_SetRegister(0x74, 0x0300); 	// RD_BOOST_RST_TIMING
	_MT9M_SetRegister(0x75, 0x3C00); 	// RD_SHR_TIMING
	_MT9M_SetRegister(0x76, 0x4E3D); 	// RD_TX_EN_TIMING
	_MT9M_SetRegister(0x77, 0x4E3D); 	// RD_BOOST_TX_TIMING
	_MT9M_SetRegister(0x78, 0x774F); 	// RD_SHS_TIMING
	_MT9M_SetRegister(0x79, 0x7900); 	// RD_VCL_COL_EN_TIMING
	_MT9M_SetRegister(0x7A, 0x7900); 	// RD_COLCLAMP_TIMING
	_MT9M_SetRegister(0x7B, 0x7800); 	// RD_SHVCL_BAR_TIMING
	_MT9M_SetRegister(0x7C, 0x7800); 	// RD_VLN_EN_TIMING
	_MT9M_SetRegister(0x7E, 0x7800); 	// RD_BOOST_ROW_TIMING
	_MT9M_SetRegister(0x7F, 0x7800); 	// RD_EN_VLN_SH_TIMING
	_MT9M_SetRegister(0x29, 0x0481); 	// DAC_CONTROL_2_REG
	_MT9M_SetRegister(0x3E, 0x0087); 	// TX_LOW_VOLTAGE
	_MT9M_SetRegister(0x3F, 0x0007); 	// RST_LOW_VOLTAGE
	_MT9M_SetRegister(0x41, 0x0003); 	// CLAMP_VOLTAGE
	_MT9M_SetRegister(0x48, 0x0018); 	// VREF_AMP_BIAS_CURRENT
	_MT9M_SetRegister(0x5F, 0x1C16); 	// CAL_THRESHOLD   
	_MT9M_SetRegister(0x57, 0x0007); 	// ANTI_ECLIPSE_VOLTAGE
	_MT9M_SetRegister(0x2A, 0xFF74); 	// DAC_CONTROL_3
*/
	// soft reset on
	_MT9M_SetRegister(0x000B, 0x0001);
	Sleep(50);
	// soft reset off
	_MT9M_SetRegister(0x000B, 0x0000);
	
	// optimize onStopImaging()
    Sleep(10);//Sleep(100);

	_ResetFifo();

	_FPGA_SetRegister(0x00, 0x00);	// clear reset
	_FPGA_SetRegister(0x01, 0x01);	// set capture flag
}

void CMainCapture::_ApplyCaptureSettings(CaptureSetting Settings)
{
	UCHAR		outBuffer[64] = {0};

	// Send Gain
	_MT9M_SetRegister(MT9M_GLOBAL_GAIN, Settings.CurrentGain);
	// Send Exposure
	_MT9M_SetRegister(MT9M_SHUTTER_WIDTH_LOWER, Settings.CurrentExposure);
	// Send BitShift
	_FPGA_SetRegister(0x31, Settings.CurrentBitshift);
	// Send CutThres
	_FPGA_SetRegister(0x32, Settings.CurrentCutThres);

	if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
	{
		_FPGA_SetRegister(0x5A, Settings.Kojak30_PERIOD);
		_FPGA_SetRegister(0x5B, Settings.Kojak30_ELON);
		_FPGA_SetRegister(0x5D, 0x04);
	}

	// Send DAC
	{
		outBuffer[0] = Settings.CurrentDAC;

		_UsbBulkOutIn(EP1OUT, 0x77, outBuffer, 1, -1, NULL, 0, NULL);
	}

	m_CurrentCaptureDAC = Settings.CurrentDAC;
}

BOOL CMainCapture::_GoJob_PostImageProcessing_Kojak(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage)
{
    //	static int		nGoodFrameCount=0;
    //	static double	timeAfterBeginCaptureImage = 0;
    //	static int		SavedFingerCountStatus = ENUM_IBSU_FINGER_COUNT_OK;
    BOOL			bIsFinal = FALSE;
    BOOL			bCompleteAcquisition = FALSE;
    int				nRc = -1;
	int				Bright=0;

    if ((m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CAPTURE) == IBSU_OPTION_AUTO_CAPTURE)
    {
        if (m_clbkProperty.nFingerState == ENUM_IBSU_FINGER_COUNT_OK && bIsGoodImage == TRUE)
        {
            bIsFinal = TRUE;
        }
    }

    /*if (GetTakeResultImageManually())
    {
		if(m_pAlgo->m_cImgAnalysis.finger_count > 0)
			bIsFinal = TRUE;
		else
			SetTakeResultImageManually(FALSE);
    }*/
	if (GetTakeResultImageManually())
    {
		if(m_pAlgo->m_cImgAnalysis.finger_count > 0 ||
			(m_propertyInfo.bEnableCBPMode == TRUE &&
			m_pAlgo->g_Rolling_Complete == TRUE))
		{
			bIsFinal = TRUE;
			nRc = IBSU_STATUS_OK;

			if (m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER &&
				m_pAlgo->m_rollingStatus < 1)
			{
				SetTakeResultImageManually(FALSE);
				m_pAlgo->m_rollingStatus = 1;

				if(m_nGoodFrameCount <= 1)
					m_pAlgo->NewRoll_Init_Rolling();

				m_nGoodFrameCount = 3;

				_PostCallback(CALLBACK_TAKING_ACQUISITION);
                m_pAlgo->m_rollingStatus = 1;

				_PostImageProcessing_ForPreview(InImg, OutImg, bIsGoodImage);
				return FALSE;
			}
		}
		else
		{
			bIsFinal = FALSE;
			m_pAlgo->m_rollingStatus = 0;
			m_nGoodFrameCount = 0;
			SetTakeResultImageManually(FALSE);
		}
    }

    // For IBSU_OPTION_IGNORE_FINGER_COUNT
    if ((m_propertyInfo.nCaptureOptions & IBSU_OPTION_IGNORE_FINGER_COUNT) == IBSU_OPTION_IGNORE_FINGER_COUNT &&
            (m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CAPTURE) == IBSU_OPTION_AUTO_CAPTURE
       )
    {
        if (m_clbkProperty.nFingerState == ENUM_IBSU_TOO_MANY_FINGERS ||
                m_clbkProperty.nFingerState == ENUM_IBSU_TOO_FEW_FINGERS)
        {
            m_timeAfterBeginCaptureImage += m_clbkProperty.imageInfo.FrameTime;
            if ((int)(m_timeAfterBeginCaptureImage * 1000) > m_propertyInfo.nIgnoreFingerTime)
            {
                bIsFinal = TRUE;
            }
        }
        else if ((m_clbkProperty.nFingerState == ENUM_IBSU_TOO_MANY_FINGERS ||
                  m_clbkProperty.nFingerState == ENUM_IBSU_TOO_FEW_FINGERS) &&
                 (m_SavedFingerCountStatus != m_clbkProperty.nFingerState))
        {
            // Initialize for IBSU_OPTION_IGNORE_FINGER_COUNT
            m_timeAfterBeginCaptureImage = 0;
        }
    }

    m_SavedFingerCountStatus = m_clbkProperty.nFingerState;

    if (bIsFinal && m_bIsActiveCapture)
    {
        m_nGoodFrameCount++;
    }

    if (m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
    {
        if (m_nGoodFrameCount > 1)
        {
            if (m_nGoodFrameCount == 2)
            {
                _PostCallback(CALLBACK_TAKING_ACQUISITION);
                m_pAlgo->m_rollingStatus = 1;
            }

			nRc = m_pAlgo->_PostImageProcessing_ForRolling(InImg, m_nGoodFrameCount, &bCompleteAcquisition);
			if (m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK)
            {
				m_pAlgo->m_bRotatedImage = FALSE;
                m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_ROIC_GetBrightWithRawImage(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
            }
            else if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
            {
				m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Kojak_GetBrightWithRawImage_Roll(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);
            }
            else
            {
                m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_GetBrightWithRawImage(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
            }

			for( int i=0; i<IBSU_MAX_SEGMENT_COUNT; i++ )
			{
				if( m_clbkProperty.qualityArray[i] == ENUM_IBSU_QUALITY_INVALID_AREA_LEFT	||
					m_clbkProperty.qualityArray[i] == ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT	||
					m_clbkProperty.qualityArray[i] == ENUM_IBSU_QUALITY_INVALID_AREA_TOP	||
					m_clbkProperty.qualityArray[i] == ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM  )
				{
					bCompleteAcquisition = FALSE;
					m_nGoodFrameCount = 0;
					nRc = -1;
				}
			}

			if (GetTakeResultImageManually())
			{
				bCompleteAcquisition = TRUE;
				nRc = IBSU_STATUS_OK;
				memcpy(InImg, m_pAlgo->m_capture_rolled_local_best_buffer, m_pAlgo->CIS_IMG_SIZE_ROLL);
			}

            if (bCompleteAcquisition)
            {
				if(m_pAlgo->m_rollingStatus < 2)
					_PostCallback(CALLBACK_COMPLETE_ACQUISITION);
                m_pAlgo->m_rollingStatus = 2;
            }

			if( nRc == IBSU_STATUS_OK &&
                (m_pDlgUsbManager->m_bIsReadySend[CALLBACK_RESULT_IMAGE] || m_pDlgUsbManager->m_bIsReadySend[CALLBACK_RESULT_IMAGE_EX]) )
			{
				if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
					m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(Bright);
				else
					m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(m_pAlgo->m_cImgAnalysis.mean);
				_PostCallback(CALLBACK_FINGER_COUNT);

				// fixed wrong finger quality callback
				IBSU_FingerQualityState savedQualityArray[4];
				memcpy(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray));

				if(!m_DisplayWindow.dispInvalidArea || 
					m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
				{
					m_clbkProperty.imageInfo.IsFinal = TRUE;
					m_pAlgo->m_cImgAnalysis.is_final = TRUE;
					m_pAlgo->_Algo_JudgeFingerQuality(&m_pAlgo->m_cImgAnalysis, &bIsGoodImage, &m_propertyInfo, &m_clbkProperty, m_DisplayWindow.dispInvalidArea);
					if (!m_bFirstSent_clbkClearPlaten)
					{
						if (memcmp(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray)) != 0)
						{
							_PostCallback(CALLBACK_FINGER_QUALITY);
						}
					}
				}

                // .... post image processing to make perfect image
                // Take Result image
#ifdef __DUMMY_DETECT_FINGER_USING_ATOF__
				if( m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0 && bIsFinal == TRUE )
				{
					// LE off
					_FPGA_SetRegister(0x56, 0x01);
					m_DeciCaptureSetting.CurrentDAC = m_DeciCaptureSetting.DefaultDAC;
					m_FullCaptureSetting.CurrentDAC = m_FullCaptureSetting.DefaultDAC;
					// Send DAC
					{
						BYTE outBuffer[2];
						outBuffer[0] = m_FullCaptureSetting.CurrentDAC;
						_UsbBulkOutIn(EP1OUT, 0x77, outBuffer, 1, -1, NULL, 0, NULL);
					}
#ifdef _KOJAK_ATOF_16B_
					m_Kojak30_OperationMode = 2;
					_Kojak30_InitializeOperationMode(m_Kojak30_OperationMode);
#endif
				}
#endif
				_PostImageProcessing_ForResult(InImg, OutImg);
                m_nGoodFrameCount = 0;
                SetTakeResultImageManually(FALSE);
                m_timeAfterBeginCaptureImage = 0;
                m_pAlgo->m_rollingStatus = 3;
                return TRUE;
            }
            else if (nRc != IBSU_STATUS_OK && !bIsFinal && 
				     m_pAlgo->m_cImgAnalysis.finger_count == 0)
            {
                if (m_pAlgo->g_Rolling_Saved_Complete == FALSE)
                {
                    m_nGoodFrameCount = 0;
                }
			}
        }
        else
        {
            // For IBSU_BGetRollingInfo function
            memset(m_pAlgo->m_capture_rolled_local_best_buffer, 0, m_UsbDeviceInfo.CisImgSize_Roll);
            m_pAlgo->m_rollingStatus = 0;
            m_pAlgo->g_LastX = -1;
        }
    }
	else if (m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
             m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
	{
/*#ifdef __G_DEBUG__
		int diff_mean;

 		diff_mean = abs(m_pAlgo->m_cImgAnalysis.final_mean-m_pAlgo->m_cImgAnalysis.mean);
        TRACE("bIsFinal=%d, gain = %d, mean=%d, diff_mean=%d, fc=%d, prev_fc=%d, diff_fc=%d, x=%d, prev_x=%d, diff_x=%d, y=%d, prev_y=%d, diff_y=%d\n", 
			bIsFinal, m_pAlgo->m_pPropertyInfo->nContrastValue, m_pAlgo->m_cImgAnalysis.mean, diff_mean, m_pAlgo->m_cImgAnalysis.foreground_count, m_pAlgo->m_cImgAnalysis.pre_foreground_count, m_pAlgo->m_cImgAnalysis.diff_foreground_count,
			m_pAlgo->m_cImgAnalysis.center_x, m_pAlgo->m_cImgAnalysis.pre_center_x, m_pAlgo->m_cImgAnalysis.center_x,
			m_pAlgo->m_cImgAnalysis.center_y, m_pAlgo->m_cImgAnalysis.pre_center_y, m_pAlgo->m_cImgAnalysis.diff_center_y);
#endif
*/
		if ((!m_UsbDeviceInfo.bDecimation && m_UsbDeviceInfo.nDecimation_Mode != DECIMATION_NONE) &&
			(m_pAlgo->m_cImgAnalysis.diff_foreground_count > SINGLE_FLAT_DIFF_TRES ||
			 m_pAlgo->m_cImgAnalysis.diff_center_x > 5 ||
			 m_pAlgo->m_cImgAnalysis.diff_center_y > 5))
		{
			if (!m_UsbDeviceInfo.bDecimation && m_clbkProperty.nFingerState == ENUM_IBSU_FINGER_COUNT_OK && bIsGoodImage == TRUE)
			{
			}
			else
			{
				bIsFinal = FALSE;
				m_pAlgo->m_cImgAnalysis.is_final = FALSE;
			}
		}

		if (m_propertyInfo.bEnableCaptureOnRelease && m_UsbDeviceInfo.bDecimation == FALSE)
		{
			if (m_bFirstPutFingerOnSensor &&
				m_SavedFingerState != m_clbkProperty.nFingerState &&
				m_clbkProperty.nFingerState == ENUM_IBSU_NON_FINGER)
			{
				memcpy(&m_pAlgo->m_cImgAnalysis, &m_pAlgo->m_BestFrame.imgAnalysis, sizeof(FrameImgAnalysis));
				InImg = m_pAlgo->m_BestFrame.Buffer;
				bIsFinal = TRUE;
			}
			else if (!GetTakeResultImageManually())
			{
				bIsFinal = FALSE;
			}
		}

        if (bIsFinal)
        {
			if (m_UsbDeviceInfo.bDecimation == TRUE)	// for Manual capture or trigger capture
			{
				bIsFinal = FALSE;
				bIsGoodImage = FALSE;
				m_nRemainedDecimationFrameCount--;
				if (m_nRemainedDecimationFrameCount >= 0 && m_nRemainedDecimationFrameCount < (__DEFAULT_KOJAK_DECIMATION_COUNT__-1))
				{
					memcpy(OutImg, m_pAlgo->m_OutResultImg, m_UsbDeviceInfo.ImgSize);
				}
				else
				{
					m_UsbDeviceInfo.bDecimation = FALSE;
					_PostImageProcessing_ForPreview(InImg, OutImg, bIsGoodImage);
				}

				if((m_propertyInfo.nCaptureOptions & IBSU_OPTION_AUTO_CONTRAST) == IBSU_OPTION_AUTO_CONTRAST)
					m_FullCaptureSetting.CurrentDAC = m_DeciCaptureSetting.CurrentDAC;
				else
					m_FullCaptureSetting.CurrentDAC = _GetContrast_FromGainTable(m_propertyInfo.nContrastValue, FALSE);

				m_FullCaptureSetting.CurrentGain = m_DeciCaptureSetting.CurrentGain;
				_ChangeDecimationModeForKojak(FALSE, &m_FullCaptureSetting, TRUE);
				// optimize onStopImaging()
				// removed
				if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0)
					_GetOneFrameImage(InImg, m_UsbDeviceInfo.CisImgSize);
				m_UsbDeviceInfo.bDecimation = FALSE;

				return FALSE;
			}

			// fixed wrong finger quality callback
			if(!m_DisplayWindow.dispInvalidArea || 
				m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			{
				bIsFinal = TRUE;
				bIsGoodImage = TRUE;
				m_clbkProperty.imageInfo.IsFinal = TRUE;
				m_pAlgo->m_cImgAnalysis.is_final = TRUE;
				//BOOL bTmpGoodImage = TRUE;

				IBSU_FingerQualityState savedQualityArray[4];
				memcpy(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray));
				m_pAlgo->_Algo_JudgeFingerQuality(&m_pAlgo->m_cImgAnalysis, &bIsGoodImage, &m_propertyInfo, &m_clbkProperty, m_DisplayWindow.dispInvalidArea);

				if (!m_bFirstSent_clbkClearPlaten)
				{
					if (memcmp(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray)) != 0)
					{
						_PostCallback(CALLBACK_FINGER_QUALITY);
					}
				}
			}

			// .... post image processing to make perfect image
			// Take Result image
			_PostCallback(CALLBACK_COMPLETE_ACQUISITION);

#ifdef __DUMMY_DETECT_FINGER_USING_ATOF__
			if( m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0 && bIsFinal == TRUE )
			{
				// LE off
				_FPGA_SetRegister(0x56, 0x01);
				m_DeciCaptureSetting.CurrentDAC = m_DeciCaptureSetting.DefaultDAC;
				m_FullCaptureSetting.CurrentDAC = m_FullCaptureSetting.DefaultDAC;
				// Send DAC
				{
					BYTE outBuffer[2];
					outBuffer[0] = m_FullCaptureSetting.CurrentDAC;
					_UsbBulkOutIn(EP1OUT, 0x77, outBuffer, 1, -1, NULL, 0, NULL);
				}
#ifdef _KOJAK_ATOF_16B_
				m_Kojak30_OperationMode = 2;
				_Kojak30_InitializeOperationMode(m_Kojak30_OperationMode);
#endif
			}
#endif

			_PostImageProcessing_ForResult(InImg, OutImg);

			m_nGoodFrameCount = 0;
			SetTakeResultImageManually(FALSE);
			m_timeAfterBeginCaptureImage = 0;

			// Reset parameters for next capture.  This applies only to Columbo.
			m_nRemainedDecimationFrameCount = __DEFAULT_KOJAK_DECIMATION_COUNT__;

			return TRUE;
        }
        else
        {
			if(m_propertyInfo.bEnableDecimation == TRUE)
			{
				if (m_UsbDeviceInfo.bDecimation == FALSE)
				{
					_ChangeDecimationModeForKojak(TRUE, &m_DeciCaptureSetting, TRUE);
					m_UsbDeviceInfo.bDecimation = TRUE;
					m_pAlgo->m_CaptureGood = FALSE;
					return FALSE;
				}
			}
		}
    }
	else if ( m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
              m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
			  m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
		      m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
    {
        if (bIsFinal)
        {
            // .... post image processing to make perfect image
            // Take Result image
#ifdef __DUMMY_DETECT_FINGER_USING_ATOF__
			if( m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0 && bIsFinal == TRUE )
			{
				// LE off
				_FPGA_SetRegister(0x56, 0x01);
				m_DeciCaptureSetting.CurrentDAC = m_DeciCaptureSetting.DefaultDAC;
				m_FullCaptureSetting.CurrentDAC = m_FullCaptureSetting.DefaultDAC;
				// Send DAC
				{
					BYTE outBuffer[2];
					outBuffer[0] = m_FullCaptureSetting.CurrentDAC;
					_UsbBulkOutIn(EP1OUT, 0x77, outBuffer, 1, -1, NULL, 0, NULL);
				}
#ifdef _KOJAK_ATOF_16B_
				m_Kojak30_OperationMode = 2;
				_Kojak30_InitializeOperationMode(m_Kojak30_OperationMode);
#endif
			}
#endif

            _PostImageProcessing_ForResult(InImg, OutImg);
            m_nGoodFrameCount = 0;
            SetTakeResultImageManually(FALSE);
            m_timeAfterBeginCaptureImage = 0;
			return TRUE;
        }
    }

    _PostImageProcessing_ForPreview(InImg, OutImg, bIsGoodImage);

    return FALSE;
}

int CMainCapture::_IsNeedInitializeCIS_Kojak()
{
	WORD reg_val[4];
	if (_MT9M_GetRegister(MT9M_ROW_START,	&reg_val[0]) != IBSU_STATUS_OK) goto done;		// 0x66 for 972, 0x36 for 1080
    if (_MT9M_GetRegister(MT9M_COLUMN_START,	&reg_val[1]) != IBSU_STATUS_OK) goto done;	// 0xF0 for 1024, 0xB0 for 1152
    if (_MT9M_GetRegister(MT9M_ROW_SIZE,	&reg_val[2]) != IBSU_STATUS_OK) goto done;		// 0x03CB = 971, 0x0437 = 1079
    if (_MT9M_GetRegister(MT9M_COLUMN_SIZE,	&reg_val[3]) != IBSU_STATUS_OK) goto done;		// 0x03FF = 1023, 0x047F = 1151

	if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		if( reg_val[0] != (0x002B + m_propertyInfo.nStartingPositionOfRollArea*100+m_propertyInfo.sKojakCalibrationInfo.LensPosRow) ||
			reg_val[1] != (0x030E - m_propertyInfo.sKojakCalibrationInfo.LensPosCol) ||
			reg_val[2] != 0x03FD ||
			reg_val[3] != 0x043F )
			return TRUE;
	}
	else if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
			m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
			m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS ||
			m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS )
	{
		if( reg_val[0] != (0x002B + m_propertyInfo.sKojakCalibrationInfo.LensPosRow) ||
			reg_val[1] != (0x012A - m_propertyInfo.sKojakCalibrationInfo.LensPosCol) ||
			reg_val[2] != 0x0787 ||
			reg_val[3] != 0x0807 )
			return TRUE;
	}
	else
		return TRUE;

done:
	return FALSE;
}


int CMainCapture::_SetPlateTouchOn()
{
	return _UsbBulkOutIn(EP1OUT, CMD_KOJAK_TOUCH_ON, NULL, 0, -1, NULL, 0, NULL);
}

int CMainCapture::_SetPlateTouchOff()
{
	return _UsbBulkOutIn(EP1OUT, CMD_KOJAK_TOUCH_OFF, NULL, 0, -1, NULL, 0, NULL);
}

int CMainCapture::_SetPlateTouchStatus(BYTE state)
{
	int nRC;
	BYTE send_data[2];
	send_data[0] = state;
	nRC = _UsbBulkOutIn(EP1OUT, CMD_KOJAK_TOUCH_SET_STATE, send_data, 1, -1, NULL, 0, NULL);
	return nRC;
}

int CMainCapture::_SetRelayStatus(BYTE addr, BYTE val)
{
	int nRC;
	BYTE send_data[2];
	send_data[0] = val;
	nRC = _UsbBulkOutIn(EP1OUT, addr, send_data, 1, -1, NULL, 0, NULL);
	return nRC;
}

int CMainCapture::_GetPlateTouchStatus(BYTE &flag)
{
	int nRC;
	BYTE send_data[2];
	nRC = _UsbBulkOutIn(EP1OUT, CMD_KOJAK_TOUCH_READ, NULL, 0, EP1IN, send_data, 1, NULL);
	flag = send_data[0];

	TRACE("\nTouch : %d==> ", flag);
	return nRC;
}

int CMainCapture::_LE_OFF()
{
	return _UsbBulkOutIn(EP1OUT, CMD_INIT_LE_VOLTAGE, NULL, 0, -1, NULL, 0, NULL);
}

int CMainCapture::_WriteCurrentSensor(BYTE addr, WORD Value)
{
    UCHAR		outBuffer[32];
	outBuffer[0] = addr;
	outBuffer[1] = (Value>>8)&0xFF;
	outBuffer[2] = Value&0xFF;

	return _UsbBulkOutIn(EP1OUT, CMD_KOJAK_WRITE_TOF, outBuffer, 3, -1, NULL, 0, NULL);
}

int CMainCapture::_ReadCurrentSensorPrototype(WORD &state)
{
	int nRC;
    UCHAR		inBuffer[32];
    int minVal, tmpVal;
    double slope;

    nRC = _UsbBulkOutIn(EP1OUT, CMD_KOJAK_READ_TOF, NULL, 0, EP1IN, inBuffer, 2, NULL);
    state = (inBuffer[1]<<8) | inBuffer[0];

    minVal = (((BYTE)m_propertyInfo.cTOFAnalogPlate[0] << 8) | (BYTE)m_propertyInfo.cTOFAnalogPlate[1]);
    slope = (double)(((BYTE)m_propertyInfo.cTOFAnalogPlate[2] << 8) | (BYTE)m_propertyInfo.cTOFAnalogPlate[3]) / 10000.0f;

	if(state > 0x1000)
    {
		state = -1;
    }
    else if (minVal > 0 && slope > 0)
    {
		tmpVal = state - minVal;
		if(tmpVal < 0) tmpVal = 0;
		state = (int)(tmpVal*slope);
    }

	return nRC;
}

WORD CMainCapture::_CalculateCurrentLimitPrototype()
{
	if(m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
		return 0xFFFF;

	// Reference table for dp film
	// 0xFF -> 430
	// 0xAC -> 950
	WORD table_current_limit_dp[7] = {430, 520, 624, 720, 820, 950, 1000};
	WORD table_DAC_dp[7] = {0xFF, 0xF0, 0xDE, 0xCE, 0xBE, 0xAC, 0x00};
	int table_cnt = 7;
	int tolerance = 35;

	int i;
	DWORD currentDAC;
	WORD currentDAC_limit=0xFFFF;
	float weight;

	if(m_UsbDeviceInfo.bDecimation)
		currentDAC = (m_DeciCaptureSetting.CurrentDAC + m_DeciCaptureSetting.PrevDAC)/2;
	else
		currentDAC = (m_FullCaptureSetting.CurrentDAC + m_FullCaptureSetting.PrevDAC)/2;

	int minVal;
    minVal = (((BYTE)m_propertyInfo.cTOFAnalogPlate[0] << 8) | (BYTE)m_propertyInfo.cTOFAnalogPlate[1]);

	for(i=0; i<table_cnt; i++)
		table_current_limit_dp[i] = table_current_limit_dp[i] + tolerance + i*5 - minVal;

	for(i=0; i<table_cnt-1; i++)
	{
		if(table_DAC_dp[i] >= currentDAC && table_DAC_dp[i+1] <= currentDAC)
		{
			weight = (float)(table_DAC_dp[i]-currentDAC) / (float)(table_DAC_dp[i]-table_DAC_dp[i+1]);
			currentDAC_limit = (WORD)((1.0f-weight) * table_current_limit_dp[i] + weight * table_current_limit_dp[i+1]);
			break;
		}
	}

	TRACE("_CalculateCurrentLimitPrototype = %X", currentDAC_limit);

	return currentDAC_limit;
}

int CMainCapture::_ReadCurrentSensor(WORD &state)
{
	int nRC;
    UCHAR		inBuffer[32];

    nRC = _UsbBulkOutIn(EP1OUT, CMD_KOJAK_READ_TOF, NULL, 0, EP1IN, inBuffer, 2, NULL, 1000);
    state = (inBuffer[1]<<8) | inBuffer[0];

	return nRC;
}

int CMainCapture::_ReadAnalogTOFFromKojak(WORD &c_state, WORD &a_state, WORD &a_state2)
{
	int nRC = IBSU_STATUS_OK;
	unsigned char ATOF_L=0, ATOF_H=0;
	//unsigned char CTOF_L=0, CTOF_H=0;
	unsigned char ATOF_L2=0, ATOF_H2=0;
    
	if((nRC = _FPGA_GetRegister(0x60, &ATOF_H)) != IBSU_STATUS_OK) goto done;
	if((nRC = _FPGA_GetRegister(0x61, &ATOF_L)) != IBSU_STATUS_OK) goto done;

	if((nRC = _FPGA_GetRegister(0x66, &ATOF_H2)) != IBSU_STATUS_OK) goto done;
	if((nRC = _FPGA_GetRegister(0x67, &ATOF_L2)) != IBSU_STATUS_OK) goto done;

	_ReadCurrentSensor(c_state);

done:
	
	a_state = (ATOF_H<<8) + ATOF_L;
	//c_state = (CTOF_H<<8) + CTOF_L;
	a_state2 = (ATOF_H2<<8) + ATOF_L2;

	return nRC;
}

WORD CMainCapture::_CalculateCurrentLimit()
{
	if(m_UsbDeviceInfo.devType != DEVICE_TYPE_KOJAK)
		return 0xFFFF;

	// Reference table for dp film
	// 0xFF -> 430
	// 0xAC -> 950
	WORD table_current_limit_dp[3] = {430, 520, 3000};
	WORD table_DAC_dp[7] = {0xFF, 0xAC, 0x00};
	int table_cnt = 3;

	int i;
	DWORD currentDAC;
	WORD currentDAC_limit=0xFFFF;
	float weight;

	if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_3_0_WITH_DAIWON_AND_70VPP_)
	{
		table_DAC_dp[0] = 0xDF;
		table_DAC_dp[1] = 0x9C;
	}

	if(m_UsbDeviceInfo.bDecimation)
		currentDAC = (m_DeciCaptureSetting.CurrentDAC + m_DeciCaptureSetting.PrevDAC)/2;
	else
		currentDAC = (m_FullCaptureSetting.CurrentDAC + m_FullCaptureSetting.PrevDAC)/2;

	table_current_limit_dp[0] = m_propertyInfo.sKojakCalibrationInfo.ACMin;
	table_current_limit_dp[1] = m_propertyInfo.sKojakCalibrationInfo.ACMax;

	for(i=0; i<table_cnt-1; i++)
	{
		if(table_DAC_dp[i] >= currentDAC && table_DAC_dp[i+1] <= currentDAC)
		{
			weight = (float)(table_DAC_dp[i]-currentDAC) / (float)(table_DAC_dp[i]-table_DAC_dp[i+1]);
			currentDAC_limit = (WORD)((1.0f-weight) * table_current_limit_dp[i] + weight * table_current_limit_dp[i+1]);
			break;
		}
	}

	TRACE("_CalculateCurrentLimit = %X", currentDAC_limit);

	return currentDAC_limit;
}

int CMainCapture::_Start_PlateTouch()
{
	m_CaptureProgressStatus = ENUM_IBSM_CAPTURE_PROGRESS_TOUCH;
	m_TouchCount=0;
	
	_LE_OFF();
//	Sleep(10);
	_SetPlateTouchStatus(1);
//	Sleep(50);		// can remove this because GetOneFrame takes about 100ms

	return TRUE;
}

BOOL CMainCapture::_Check_PlateTouch()
{
	BYTE state=1;
	if(_GetPlateTouchStatus(state) == IBSU_STATUS_OK)
	{
		if(state == 0)
			return TRUE;
	}
	
	return FALSE;
}

int CMainCapture::_Start_TOF()
{
	m_CaptureProgressStatus = ENUM_IBSM_CAPTURE_PROGRESS_TOF;

	m_TOF_ON_Count=0;
	m_TOF_OFF_Count=0;

	m_ATOF_OFF_Count=0;
	m_ATOF_ON_Count=0;

	_SetPlateTouchStatus(0);

	int SetVoltage = _DAC_FOR_NORMAL_FINGER_WITH_TOF_;

	m_DeciCaptureSetting.CurrentDAC = m_DeciCaptureSetting.DefaultDAC;
	m_DeciCaptureSetting.PrevDAC = m_DeciCaptureSetting.CurrentDAC;

	m_FullCaptureSetting.CurrentDAC = m_FullCaptureSetting.DefaultDAC;
	m_FullCaptureSetting.PrevDAC = m_FullCaptureSetting.CurrentDAC;

	if(m_UsbDeviceInfo.bDecimation)
	{
		SetVoltage = m_DeciCaptureSetting.CurrentDAC;
	}
	else
	{
		SetVoltage = m_FullCaptureSetting.CurrentDAC;
	}

	UCHAR		outBuffer[64] = {0};
	outBuffer[0] = SetVoltage;				// start TOF at lowest voltage
	_UsbBulkOutIn(EP1OUT, 0x77, outBuffer, 1, -1, NULL, 0, NULL);

	return TRUE;
}

BOOL CMainCapture::_Check_TOF(int *CurTOF, int *ThresTOF)
{
	WORD current_TOF=0x0000;
	WORD analog_TOF=0x0000;
	WORD analog_TOF2=0x0000;
	WORD thres_TOF=0x0000;
	
	if(m_propertyInfo.sKojakCalibrationInfo.Version == 0x00)
	{
		_ReadCurrentSensorPrototype(current_TOF);
		thres_TOF = _CalculateCurrentLimitPrototype();
	}
	else if(m_propertyInfo.sKojakCalibrationInfo.Version >= 0x01)
	{
		if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER ||
			m_UsbDeviceInfo.nFpgaVersion < _FPGA_VER_FOR_KOJAK_3_0)
			_ReadCurrentSensor(current_TOF);
		else
			_ReadAnalogTOFFromKojak(current_TOF, analog_TOF, analog_TOF2);

		if(current_TOF > 10000)
		{
			current_TOF = 0x00;
		}

		m_TOF_Analog = analog_TOF;
		m_TOF_Analog2 = analog_TOF2;

		TRACE("current_TOF : %X(%d), analog_TOF : %X(%d)\n", current_TOF, current_TOF, analog_TOF, analog_TOF);
		thres_TOF = _CalculateCurrentLimit();
	}

	m_TOFArr_idx = m_TOFArr_idx%3;
	m_TOFArr[m_TOFArr_idx] = current_TOF;
	m_TOFArr_idx = (m_TOFArr_idx+1)%3;

	current_TOF = (m_TOFArr[0]+m_TOFArr[1]+m_TOFArr[2])/3;

	*CurTOF = current_TOF;
	*ThresTOF = thres_TOF;

	if(current_TOF >= thres_TOF)
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CMainCapture::_ReadTOFStatus(BYTE *GroundCheck, BYTE *Idle)
{
	int nRC;
	BYTE send_data[3];
	nRC = _UsbBulkOutIn(EP1OUT, CMD_KOJAK_TOUCH_GET_STATE, NULL, 0, EP1IN, send_data, 2, NULL);

	*GroundCheck = send_data[0];				// 0 : ground(Idle), 1 : non-ground(Ready)
	*Idle = (send_data[1] & 0x06) >> 1;			// 1 : idle, 2 : ready

	if(nRC == IBSU_STATUS_OK)
		return TRUE;

	return FALSE;
}

int CMainCapture::_SetADCThres(unsigned char ADCThres)
{
    if (m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK &&
		m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_KOJAK_NEW_BRD_AND_DOUBLE_P_AND_CURRENTSENSOR_)
    {
		UCHAR outBuffer[2];
		outBuffer[0] = (UCHAR)ADCThres;
		return _UsbBulkOutIn(EP1OUT, CMD_WRITE_ADC_THRES, outBuffer, 1, -1, NULL, 0, NULL);
    }

	return IBSU_ERR_NOT_SUPPORTED;
}

int	CMainCapture::Reserved_GetFinalImageByNative_Kojak(IBSU_ImageData *finalImage)
{
	int nRc = IBSU_STATUS_OK;
#if defined(__IBSCAN_ULTIMATE_SDK__)
	
	if(m_UsbDeviceInfo.devType == DEVICE_TYPE_KOJAK)
	{
		finalImage->Buffer = m_pAlgo->m_final_image;
		finalImage->BitsPerPixel = 8;
		finalImage->Format = IBSU_IMG_FORMAT_GRAY;
		finalImage->FrameTime = 0;
		finalImage->IsFinal = 1;
		finalImage->ProcessThres = IMAGE_PROCESS_THRES_PERFECT;
		finalImage->ResolutionX = m_UsbDeviceInfo.scanResolutionX;
		finalImage->ResolutionY = m_UsbDeviceInfo.scanResolutionY;
		
		if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
			finalImage->Width = m_UsbDeviceInfo.CisImgWidth_Roll;
			finalImage->Height = m_UsbDeviceInfo.CisImgHeight_Roll;
			finalImage->Pitch = -m_UsbDeviceInfo.CisImgWidth_Roll;
		}
		else
		{
			finalImage->Width = m_UsbDeviceInfo.CisImgWidth;
			finalImage->Height = m_UsbDeviceInfo.CisImgHeight;
			finalImage->Pitch = -m_UsbDeviceInfo.CisImgWidth;
		}
	}
#endif
	return nRc;
}

int CMainCapture::_AES_KeyTransfer_For_Kojak(unsigned char* key, int size)
{
	int nRc = IBSU_STATUS_OK;

	//////////////////////////////
	/* AES KEY TRANSFER - START */
	//////////////////////////////
	//if(m_pAlgo != NULL)
	{
		BOOL bResult;

		// EP2 SWITCH
		_FPGA_SetRegister( 0x40, 0x01 );
		
		
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
		_FPGA_SetRegister( 0x41, 0x01 );
		_FPGA_SetRegister( 0x41, 0x00 );
	}
	////////////////////////////
	/* AES KEY TRANSFER - END */
	////////////////////////////
	return nRc;
}

int CMainCapture::_Initialize_Encryption_For_Kojak()
{
	int FPGAVersion = _GetFpgaVersion(TRUE);

	if(FPGAVersion >= _FPGA_VER_FOR_KOJAK_3_0 &&
		m_pAlgo != NULL &&
		m_pAlgo->m_AESEnc != NULL)
	{
		UCHAR org[256], enc[512];
		memset(org, 0, 256);
		memset(enc, 0, 512);

		if(m_EncryptionMode == ENUM_IBSU_ENCRYPTION_KEY_RANDOM)
		{
			m_pAlgo->m_AESEnc->AES_Set_Key_Random();
		}
		else if(m_EncryptionMode == ENUM_IBSU_ENCRYPTION_KEY_CUSTOM)
		{
			m_pAlgo->m_AESEnc->AES_Set_Key(m_pAlgo->m_AESEnc->m_primary_key);
		}
		else
		{
			m_pAlgo->m_AESEnc->AES_Set_Key((unsigned char*)AES_KEY_ENCRYPTION);
		}

		memcpy(org, (PUCHAR)m_pAlgo->m_AESEnc->m_rkey_all, 256);

		CIBEncryption *RSA = new CIBEncryption(org);
		RSA->AES_Key_Encrypt(org, enc);
		delete RSA;
		_AES_KeyTransfer(enc, 512);

		////////////////////////////////
		/* AES KEY TRANSFER - Waiting */
		////////////////////////////////
		// Wait until receiving is done
		int		trial = 30;
		UCHAR	state = 0;

		for(; trial >=0; trial--)
		{
			Sleep(100);
			_FPGA_GetRegister( 0x42, &state);
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
		_FPGA_SetRegister( 0x40, 0x00 );

		if(m_propertyInfo.bEnableEncryption == FALSE)
			_FPGA_SetRegister( 0x43, 0x00 );
		else
			_FPGA_SetRegister( 0x43, 0x01 );

		return IBSU_STATUS_OK;
	}

	return IBSU_ERR_NOT_SUPPORTED;
}

#ifdef _KOJAK_ATOF_16B_
int CMainCapture::_Kojak30_InitializeOperationMode(int Mode)
{
	// Mode 1 : for old Kojak mode (no using LEOnOff - Always LE on, using current sensor only)
	// Mode 2 : for Kojak3.0 mode (using all snesor, both AT and CS)

	switch(Mode)
	{
	case 1:
		_FPGA_SetRegister(0x55, 0x00);	// 0x00 : always LE on / 0x01 : LE on-off mode
		_FPGA_SetRegister(0x56, 0x00);	// 0x00 : LE ON
		_FPGA_SetRegister(0x5B, 0x20);	// recommend 0x20 for old operation mode of Kojak 3.0
		_FPGA_SetRegister(0x5C, 0x01);	// 0x01 : always LE on / 0x00 : LE on-off mode
		break;
	
	case 2:
		_FPGA_SetRegister(0x55, 0x01);	// 0x00 : always LE on / 0x01 : LE on-off mode
		_FPGA_SetRegister(0x56, 0x01);	// 0x00 : LE ON / 0x01 : LE OFF
		if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			_FPGA_SetRegister(0x5B, m_FullCaptureSetting.Kojak30_ELON);
		else
			_FPGA_SetRegister(0x5B, m_DeciCaptureSetting.Kojak30_ELON);
		_FPGA_SetRegister(0x5C, 0x00);	// 0x01 : always LE on / 0x00 : LE on-off mode
		break;

	default:
		return IBSU_ERR_INVALID_PARAM_VALUE;
	}

	m_Kojak30_OperationMode = Mode;

	if(m_Kojak30_OperationMode == 1)
	{
		if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
			m_FullCaptureSetting.forSuperDry_Mode = m_propertyInfo.nSuperDryMode;

			m_FullCaptureSetting.DefaultMinDAC = _DAC_FOR_MINIMUM_;
			m_FullCaptureSetting.DefaultMaxDAC = _DAC_FOR_MAXIMUM_;
			m_FullCaptureSetting.DefaultBitshift = _NO_BIT_BITSHIFT_FOR_FULLFRAME_ROLL_;
			m_FullCaptureSetting.DefaultCutThres = _NO_BIT_CUTTHRES_FOR_FULLFRAME_ROLL_;
			m_FullCaptureSetting.Kojak30_PERIOD = _KOJAK30_PERIOD_ROLL_;
			m_FullCaptureSetting.Kojak30_ELON = _KOJAK30_ALWAYS_ELON_FOR_FULLFRAME_;

			if(m_FullCaptureSetting.forSuperDry_Mode)
			{
				m_FullCaptureSetting.TargetMinBrightness = _TARGET_MIN_BRIGHTNESS_FOR_DRY_;
				m_FullCaptureSetting.TargetMaxBrightness = _TARGET_MAX_BRIGHTNESS_FOR_DRY_;
			}
			else
			{
				m_FullCaptureSetting.TargetMinBrightness = m_propertyInfo.nCaptureBrightThresRoll[0];
				m_FullCaptureSetting.TargetMaxBrightness = m_propertyInfo.nCaptureBrightThresRoll[1];
			}

			m_FullCaptureSetting.TargetFingerCount = 1;
			m_FullCaptureSetting.DefaultGain = _NO_BIT_GAIN_FOR_FULLFRAME_;
			m_FullCaptureSetting.CurrentGain = m_FullCaptureSetting.DefaultGain;
			m_FullCaptureSetting.DefaultExposure = _DEFAULT_EXPOSURE_FOR_FULLFRAME_ROLL_;
			m_FullCaptureSetting.CurrentExposure = m_FullCaptureSetting.DefaultExposure;
			m_FullCaptureSetting.CurrentBitshift = m_FullCaptureSetting.DefaultBitshift;
			m_FullCaptureSetting.CurrentCutThres = m_FullCaptureSetting.DefaultCutThres;
			m_FullCaptureSetting.AdditionalFrame = 1;
			m_FullCaptureSetting.RollCaptureMode = TRUE;
			m_FullCaptureSetting.DetectionFrameCount = 0;

			m_DeciCaptureSetting = m_FullCaptureSetting;

			m_UsbDeviceInfo.bDecimation = FALSE;
			m_CurrentCaptureDAC = m_FullCaptureSetting.CurrentDAC;
		}
		else if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
			m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
			m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS ||
			m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS )
		{
			m_DeciCaptureSetting.forSuperDry_Mode = m_propertyInfo.nSuperDryMode;

			m_DeciCaptureSetting.DefaultMinDAC = _DAC_FOR_MINIMUM_;
			m_DeciCaptureSetting.DefaultMaxDAC = _DAC_FOR_MAXIMUM_;
			m_DeciCaptureSetting.Kojak30_PERIOD = _KOJAK30_PERIOD_DECI_;
			m_DeciCaptureSetting.Kojak30_ELON = _KOJAK30_ALWAYS_ELON_FOR_FULLFRAME_;

			if(m_DeciCaptureSetting.forSuperDry_Mode)
			{
				m_DeciCaptureSetting.TargetMinBrightness = _TARGET_MIN_BRIGHTNESS_FOR_DRY_;
				m_DeciCaptureSetting.TargetMaxBrightness = _TARGET_MAX_BRIGHTNESS_FOR_DRY_;
			}
			else
			{
				m_DeciCaptureSetting.TargetMinBrightness = m_propertyInfo.nCaptureBrightThresFlat[0];
				m_DeciCaptureSetting.TargetMaxBrightness = m_propertyInfo.nCaptureBrightThresFlat[1];
			}
			m_DeciCaptureSetting.TargetFingerCount = m_propertyInfo.nNumberOfObjects;
			m_DeciCaptureSetting.DefaultGain = _KOJAK30_CS_MODE_DEFAULT_GAIN_FOR_DECI_;
			m_DeciCaptureSetting.CurrentGain = m_DeciCaptureSetting.DefaultGain;
			m_DeciCaptureSetting.DefaultExposure = _KOJAK30_CS_MODE_DEFAULT_EXPOSURE_FOR_DECI_;
			m_DeciCaptureSetting.CurrentExposure = m_DeciCaptureSetting.DefaultExposure;
			m_DeciCaptureSetting.DefaultBitshift = _KOJAK30_CS_MODE_DEFAULT_BITSHIFT_FOR_DECI_;
			m_DeciCaptureSetting.CurrentBitshift = m_DeciCaptureSetting.DefaultBitshift;
			m_DeciCaptureSetting.DefaultCutThres = _KOJAK30_CS_MODE_DEFAULT_CUTTHRES_FOR_DECI_;
			m_DeciCaptureSetting.CurrentCutThres = m_DeciCaptureSetting.DefaultCutThres;
			m_DeciCaptureSetting.AdditionalFrame = 1;
			m_DeciCaptureSetting.RollCaptureMode = FALSE;
			m_DeciCaptureSetting.DetectionFrameCount = 0;

			m_FullCaptureSetting = m_DeciCaptureSetting;
			m_FullCaptureSetting.DefaultExposure = _KOJAK30_CS_MODE_EXPOSURE_FOR_FULLFRAME_;
			m_FullCaptureSetting.DefaultBitshift = _KOJAK30_CS_MODE_BITSHIFT_FOR_FULLFRAME_;
			m_FullCaptureSetting.DefaultCutThres = _KOJAK30_CS_MODE_CUTTHRES_FOR_FULLFRAME_;
			m_FullCaptureSetting.CurrentExposure = m_FullCaptureSetting.DefaultExposure;
			m_FullCaptureSetting.CurrentBitshift = m_FullCaptureSetting.DefaultBitshift;
			m_FullCaptureSetting.CurrentCutThres = m_FullCaptureSetting.DefaultCutThres;
			m_FullCaptureSetting.Kojak30_PERIOD = _KOJAK30_PERIOD_FOR_FULLFRAME_;
			m_FullCaptureSetting.Kojak30_ELON = _KOJAK30_DEFAULT_ELON_FOR_FULLFRAME_;

			if (m_UsbDeviceInfo.nFpgaVersion >= ((0 << 24) | (9 << 16) | (0 << 8) | 0x0D))
			{
				m_UsbDeviceInfo.bDecimation = TRUE;
				m_UsbDeviceInfo.nDecimation_Mode = DECIMATION_2X;
			}

			m_CurrentCaptureDAC = m_DeciCaptureSetting.CurrentDAC;
		}
	}
	else if(m_Kojak30_OperationMode == 2)
	{
		if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
			m_FullCaptureSetting.forSuperDry_Mode = m_propertyInfo.nSuperDryMode;

			m_FullCaptureSetting.DefaultMinDAC = _DAC_FOR_MINIMUM_;
			m_FullCaptureSetting.DefaultMaxDAC = _DAC_FOR_MAXIMUM_;
			m_FullCaptureSetting.DefaultBitshift = _NO_BIT_BITSHIFT_FOR_FULLFRAME_ROLL_;
			m_FullCaptureSetting.DefaultCutThres = _NO_BIT_CUTTHRES_FOR_FULLFRAME_ROLL_;
			m_FullCaptureSetting.Kojak30_PERIOD = _KOJAK30_PERIOD_ROLL_;
			m_FullCaptureSetting.Kojak30_ELON = _KOJAK30_DEFAULT_ELON_FOR_FULLFRAME_ROLL_;

			if(m_FullCaptureSetting.forSuperDry_Mode)
			{
				m_FullCaptureSetting.TargetMinBrightness = _TARGET_MIN_BRIGHTNESS_FOR_DRY_;
				m_FullCaptureSetting.TargetMaxBrightness = _TARGET_MAX_BRIGHTNESS_FOR_DRY_;
			}
			else
			{
				m_FullCaptureSetting.TargetMinBrightness = m_propertyInfo.nCaptureBrightThresRoll[0];
				m_FullCaptureSetting.TargetMaxBrightness = m_propertyInfo.nCaptureBrightThresRoll[1];
			}

			m_FullCaptureSetting.TargetFingerCount = 1;
			m_FullCaptureSetting.DefaultGain = _NO_BIT_GAIN_FOR_FULLFRAME_;
			m_FullCaptureSetting.CurrentGain = m_FullCaptureSetting.DefaultGain;
			m_FullCaptureSetting.DefaultExposure = _KOJAK30_DEFAULT_EXPOSURE_FOR_FULLFRAME_ROLL_;
			m_FullCaptureSetting.CurrentExposure = m_FullCaptureSetting.DefaultExposure;
			m_FullCaptureSetting.CurrentBitshift = m_FullCaptureSetting.DefaultBitshift;
			m_FullCaptureSetting.CurrentCutThres = m_FullCaptureSetting.DefaultCutThres;
			m_FullCaptureSetting.AdditionalFrame = 1;
			m_FullCaptureSetting.RollCaptureMode = TRUE;
			m_FullCaptureSetting.DetectionFrameCount = 0;

			m_DeciCaptureSetting = m_FullCaptureSetting;

			m_UsbDeviceInfo.bDecimation = FALSE;
			m_CurrentCaptureDAC = m_FullCaptureSetting.CurrentDAC;
		}
		else if(m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
			m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
			m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS ||
			m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS )
		{
			m_DeciCaptureSetting.forSuperDry_Mode = m_propertyInfo.nSuperDryMode;

			m_DeciCaptureSetting.DefaultMinDAC = _DAC_FOR_MINIMUM_;
			m_DeciCaptureSetting.DefaultMaxDAC = _DAC_FOR_MAXIMUM_;
			m_DeciCaptureSetting.Kojak30_PERIOD = _KOJAK30_PERIOD_DECI_;
			m_DeciCaptureSetting.Kojak30_ELON = _KOJAK30_DEFAULT_ELON_DECI_;

			if(m_DeciCaptureSetting.forSuperDry_Mode)
			{
				m_DeciCaptureSetting.TargetMinBrightness = _TARGET_MIN_BRIGHTNESS_FOR_DRY_;
				m_DeciCaptureSetting.TargetMaxBrightness = _TARGET_MAX_BRIGHTNESS_FOR_DRY_;
			}
			else
			{
				m_DeciCaptureSetting.TargetMinBrightness = m_propertyInfo.nCaptureBrightThresFlat[0];
				m_DeciCaptureSetting.TargetMaxBrightness = m_propertyInfo.nCaptureBrightThresFlat[1];
			}
			m_DeciCaptureSetting.TargetFingerCount = m_propertyInfo.nNumberOfObjects;
			m_DeciCaptureSetting.DefaultGain = _DEFAULT_GAIN_FOR_DECI_;
			m_DeciCaptureSetting.CurrentGain = m_DeciCaptureSetting.DefaultGain;
			m_DeciCaptureSetting.DefaultExposure = _KOJAK30_DEFAULT_EXPOSURE_FOR_DECI_;
			m_DeciCaptureSetting.CurrentExposure = m_DeciCaptureSetting.DefaultExposure;
			m_DeciCaptureSetting.DefaultBitshift = _KOJAK30_DEFAULT_BITSHIFT_FOR_DECI_;
			m_DeciCaptureSetting.CurrentBitshift = m_DeciCaptureSetting.DefaultBitshift;
			m_DeciCaptureSetting.DefaultCutThres = _KOJAK30_DEFAULT_CUTTHRES_FOR_DECI_;
			m_DeciCaptureSetting.CurrentCutThres = m_DeciCaptureSetting.DefaultCutThres;
			m_DeciCaptureSetting.AdditionalFrame = 1;
			m_DeciCaptureSetting.RollCaptureMode = FALSE;
			m_DeciCaptureSetting.DetectionFrameCount = 0;

			m_FullCaptureSetting = m_DeciCaptureSetting;
			m_FullCaptureSetting.DefaultExposure = _KOJAK30_NO_BIT_EXPOSURE_FOR_FULLFRAME_;
			m_FullCaptureSetting.DefaultBitshift = _KOJAK30_NO_BIT_BITSHIFT_FOR_FULLFRAME_;
			m_FullCaptureSetting.DefaultCutThres = _KOJAK30_NO_BIT_CUTTHRES_FOR_FULLFRAME_;
			m_FullCaptureSetting.CurrentExposure = m_FullCaptureSetting.DefaultExposure;
			m_FullCaptureSetting.CurrentBitshift = m_FullCaptureSetting.DefaultBitshift;
			m_FullCaptureSetting.CurrentCutThres = m_FullCaptureSetting.DefaultCutThres;
			m_FullCaptureSetting.Kojak30_PERIOD = _KOJAK30_PERIOD_FOR_FULLFRAME_;
			m_FullCaptureSetting.Kojak30_ELON = _KOJAK30_DEFAULT_ELON_FOR_FULLFRAME_;

			if (m_UsbDeviceInfo.nFpgaVersion >= ((0 << 24) | (9 << 16) | (0 << 8) | 0x0D))
			{
				m_UsbDeviceInfo.bDecimation = TRUE;
				m_UsbDeviceInfo.nDecimation_Mode = DECIMATION_2X;
			}

			m_CurrentCaptureDAC = m_DeciCaptureSetting.CurrentDAC;
		}
	}

	if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		_ApplyCaptureSettings(m_FullCaptureSetting);
	}
	else
	{
		_ApplyCaptureSettings(m_DeciCaptureSetting);
	}

	return IBSU_STATUS_OK;
}
#endif
