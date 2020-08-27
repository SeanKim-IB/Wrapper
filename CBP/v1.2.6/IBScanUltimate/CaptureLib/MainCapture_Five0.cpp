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

BOOL CMainCapture::_GetOneFrameImage_Five0(unsigned char *Image, int ImgSize)
{
    //////////////////////////////////////////////////////////////////////////////
    // one frame read
    int			i=0;
    LONG		lActualBytes = 0;
    int			nRc, img_pos, remain_size;
    int         lTotalBytes = 0, TotallActualBytes = 0;
	BYTE *inImg = m_pAlgo->m_Inter_Img4;
	BYTE *inImg2 = m_pAlgo->m_Inter_Img5;
	memset(inImg, 0, m_UsbDeviceInfo.CisImgSize);
	memset(inImg2, 0, m_UsbDeviceInfo.CisImgSize);
	memset(Image, 0, m_UsbDeviceInfo.CisImgSize);

	if(m_UsbDeviceInfo.bArrangeMode)
	{
		if( m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER )
		{
			/*nRc = _UsbBulkIn(EP6IN, inImg, ImgSize, &lActualBytes);

			if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0))
			{
				return FALSE;
			}

            memcpy(Image, inImg, m_UsbDeviceInfo.CisImgSize);*/

			img_pos = 0;
			nRc = _UsbBulkIn(EP6IN, &inImg[img_pos], 1024*1024, &lActualBytes);

			if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1))
			{
				return FALSE;
			}
			TotallActualBytes = TotallActualBytes + lActualBytes;

			img_pos = 1024*1024;
			remain_size = ImgSize - 1024*1024;
			nRc = _UsbBulkIn(EP6IN, &inImg[img_pos], remain_size, &lActualBytes);

			if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1))
			{
				return FALSE;
			}
			TotallActualBytes = TotallActualBytes + lActualBytes;

			memcpy(Image, inImg, m_UsbDeviceInfo.CisImgSize);
		}
		else
		{
			nRc = _UsbBulkIn(EP6IN, inImg, ImgSize/2, &lActualBytes);

			if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0))
			{
				return FALSE;
			}

//			TRACE("GetOneFrame : %d (lActualBytes:%d)\n", ImgSize/2, lActualBytes);

			memcpy(Image, &inImg[(m_propertyInfo.nStartingPositionOfRollArea * 27)*m_UsbDeviceInfo.CisImgWidth_Roll], m_UsbDeviceInfo.CisImgSize_Roll);
			/*for (int i=0; i< m_UsbDeviceInfo.ImgHeight_Roll; i++)
			{
				memcpy(&Image[i*m_UsbDeviceInfo.CisImgWidth_Roll], &inImg[i*m_UsbDeviceInfo.CisImgWidth+(m_UsbDeviceInfo.CisImgWidth-m_UsbDeviceInfo.CisImgWidth_Roll)/2], m_UsbDeviceInfo.CisImgWidth_Roll);
			}*/
		}
	}
	else
	{
		for (i = 0; lTotalBytes < ImgSize; i++)
		{
			lTotalBytes = 0;
			_FPGA_SetRegister(0x40, 0x00);
			nRc = _UsbBulkIn(EP6IN, inImg, m_UsbDeviceInfo.CisImgSize/2, &lActualBytes);
			if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0))
			{
				return FALSE;
			}

			lTotalBytes += lActualBytes;
			_FPGA_SetRegister(0x40, 0x01);
			nRc = _UsbBulkIn(EP6IN, inImg + m_UsbDeviceInfo.CisImgSize/2, m_UsbDeviceInfo.CisImgSize/2, &lActualBytes);
			if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0))
			{
				return FALSE;
			}
			lTotalBytes += lActualBytes;
			_FPGA_SetRegister(0x40, 0x00);

			if (lTotalBytes == ImgSize)
			{
				break;
			}
			//TRACE("Wrong received image size = %d / %d\n", lActualBytes, ImgSize);
			//		break;
		}

		_FPGA_SetRegister(0x2F, 0x00);
		_FPGA_SetRegister(0x2F, 0x01);

		if( m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER )
		{
			for (int i=0; i< m_UsbDeviceInfo.CisImgHeight; i++)
			{
				memcpy(&Image[i*m_UsbDeviceInfo.CisImgWidth], &inImg[i*(m_UsbDeviceInfo.CisImgWidth>>1)], (m_UsbDeviceInfo.CisImgWidth>>1));
				memcpy(&Image[(i)*m_UsbDeviceInfo.CisImgWidth+(m_UsbDeviceInfo.CisImgWidth/2)], &inImg[(m_UsbDeviceInfo.CisImgWidth*m_UsbDeviceInfo.CisImgHeight/2)+i*(m_UsbDeviceInfo.CisImgWidth>>1)], (m_UsbDeviceInfo.CisImgWidth>>1));
			}
		}
		else
		{
			for (int i=0; i< m_UsbDeviceInfo.CisImgHeight; i++)
			{
				memcpy(&inImg2[i*m_UsbDeviceInfo.CisImgWidth], &inImg[i*(m_UsbDeviceInfo.CisImgWidth>>1)], (m_UsbDeviceInfo.CisImgWidth>>1));
				memcpy(&inImg2[(i)*m_UsbDeviceInfo.CisImgWidth+(m_UsbDeviceInfo.CisImgWidth/2)], &inImg[(m_UsbDeviceInfo.CisImgWidth*m_UsbDeviceInfo.CisImgHeight/2)+i*(m_UsbDeviceInfo.CisImgWidth>>1)], (m_UsbDeviceInfo.CisImgWidth>>1));
			}

			for (int i=0; i< m_UsbDeviceInfo.ImgHeight_Roll; i++)
			{
				memcpy(&Image[i*m_UsbDeviceInfo.CisImgWidth_Roll], &inImg2[i*m_UsbDeviceInfo.CisImgWidth+(m_UsbDeviceInfo.CisImgWidth-m_UsbDeviceInfo.CisImgWidth_Roll)/2], m_UsbDeviceInfo.CisImgWidth_Roll);
			}
		}
	}

    return TRUE;
}

int CMainCapture::_InitializeForCISRegister_Five0(WORD rowStart, WORD colStart, WORD width, WORD height)
{
	DWORD size;
    BYTE  voltage_value = 0xAF;		// AUO default == 96;
	unsigned short numericUpDownADCoffset = 400;
//	unsigned short numericUpDownADCoffset = 450;
	BYTE	RegisterValueH = (BYTE)(numericUpDownADCoffset >> 8);
	BYTE	RegisterValueL = (BYTE)numericUpDownADCoffset;
	unsigned char read_val;

	// FPGA Power turn on/off through Cypress -------------------------------------------------------------------------------------
    if (_UsbBulkOutIn(EP1OUT, CMD_CAPTURE_END, NULL, 0, -1, NULL, 0, NULL) != IBSU_STATUS_OK) goto done;
	Sleep(100);
    if (_UsbBulkOutIn(EP1OUT, CMD_CAPTURE_START, NULL, 0, -1, NULL, 0, NULL) != IBSU_STATUS_OK) goto done;
	Sleep(250);

	m_UsbDeviceInfo.nFpgaVersion = _GetFpgaVersion(TRUE);

	if( m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_FIVE0_TOF_BETA_ ||
		m_UsbDeviceInfo.bCanUseTOF )
	{
		if( m_propertyInfo.bEnableTOF &&
			(m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER ||
			(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_propertyInfo.bEnableTOFforROLL)) )
		{
			// TOF on
			if (_FPGA_SetRegister(0x30, 0x62) != IBSU_STATUS_OK) goto done;
			if (_FPGA_SetRegister(0x67, 0x01) != IBSU_STATUS_OK) goto done;
		}
		else
		{
			if (_FPGA_SetRegister(0x30, 0x66) != IBSU_STATUS_OK) goto done;
			if (_FPGA_SetRegister(0x67, 0x00) != IBSU_STATUS_OK) goto done;
			if (_FPGA_SetRegister(0x31, 0x3E) != IBSU_STATUS_OK) goto done;
		}
	}

	//----------- IB Added for Initial Power Consumption -----------//
	// S/W reset of ASIC
	if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x200A, 0x01) != IBSU_STATUS_OK) goto done;
	if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2304, 0x00) != IBSU_STATUS_OK) goto done;
	if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x234F, 0x0A) != IBSU_STATUS_OK) goto done;
//	Sleep(100);
	//----------- IB Added for Initial Power Consumption -----------//

	if (_FPGA_SetRegister(0x00, 0x01) != IBSU_STATUS_OK) goto done;	// set reset
	Sleep(50);			// for more safe

	if (_FPGA_SetRegister(0x01, 0x00) != IBSU_STATUS_OK) goto done;	// clear capture flag
	Sleep(50);			// for more safe

	// Read FPGA and TFT version
	read_val=0;
	ASIC_GetRegister_Five0(FIVE0_ASIC_MASTER_ADDRESS, 0x2005, &read_val);
	m_UsbDeviceInfo.nASICVersion = read_val;

	m_UsbDeviceInfo.nFpgaVersion = _GetFpgaVersion(TRUE);

	if(m_UsbDeviceInfo.nFpgaVersion >= ((0 << 24) | (9 << 16) | (3 << 8) | 0x00) )		// support arrange mode from FPGA 0.9.3
		m_UsbDeviceInfo.bArrangeMode = TRUE;
	else
		m_UsbDeviceInfo.bArrangeMode = FALSE;

	if(m_UsbDeviceInfo.nFpgaVersion >= ((0 << 24) | (9 << 16) | (5 << 8) | 0x00) )		// support windows mode from FPGA 0.9.5
		m_UsbDeviceInfo.bWindowsMode = TRUE;
	else
		m_UsbDeviceInfo.bWindowsMode = FALSE;

	if(m_UsbDeviceInfo.bArrangeMode)	// higher than FPGA 0.9.3
	{
		if (_FPGA_SetRegister(0x34, 0x01) != IBSU_STATUS_OK) goto done;
		Sleep(50);			// for more safe

		if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			size = width/4 * height;
		else
			size = width/2 * height;
	}
	else
	{
		size = width/2 * height;
	}

	// Cypress reset
	if (_UsbBulkOutIn(EP1OUT, CMD_RESET_FIFO, NULL, 0, -1, NULL, 0, NULL) != IBSU_STATUS_OK) goto done;

	// Set frame_size
	if (_FPGA_SetRegister(0x07, (UCHAR)(size & 0xFF)) != IBSU_STATUS_OK) goto done;		
	if (_FPGA_SetRegister(0x08, (UCHAR)((size >> 8) & 0xFF)) != IBSU_STATUS_OK) goto done;
	if (_FPGA_SetRegister(0x09, (UCHAR)((size >> 16) & 0xFF)) != IBSU_STATUS_OK) goto done;

	// FPGA clear reset 
	Sleep(50);			// for more safe
	if (_FPGA_SetRegister(0x00, 0x00) != IBSU_STATUS_OK) goto done;	// clear reset
	Sleep(50);			// for more safe

	if(m_UsbDeviceInfo.nASICVersion <= 0)
	{
		// EL Initailize ----------------------------------------------------------------------------------------
		if (_FPGA_SetRegister(0x29, 0x00) != IBSU_STATUS_OK) goto done;	// clear reset
		if (_FPGA_SetRegister(0x2a, 0x00) != IBSU_STATUS_OK) goto done;	// clear reset
		if (_FPGA_SetRegister(0x2b, 0x00) != IBSU_STATUS_OK) goto done;	// clear reset
		if (_FPGA_SetRegister(0x2c, 0x0D) != IBSU_STATUS_OK) goto done;	// clear reset
		if (_FPGA_SetRegister(0x2d, 0x00) != IBSU_STATUS_OK) goto done;	// clear reset
		if (_FPGA_SetRegister(0x2e, 0x00) != IBSU_STATUS_OK) goto done;	// clear reset
		if (_UsbBulkOutIn(EP1OUT, CMD_INIT_LE_VOLTAGE, NULL, 0, -1, NULL, 0, NULL) != IBSU_STATUS_OK) goto done;
		if (_UsbBulkOutIn(EP1OUT, CMD_WRITE_LE_VOLTAGE, &voltage_value, 1, -1, NULL, 0, NULL) != IBSU_STATUS_OK) goto done;

		// ASIC initialization
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21A3, 0x09) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21A4, 0x88) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21A5, 0x08) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2313, 0x20) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21AC, 0x08) != IBSU_STATUS_OK) goto done;

		// LTPS Timing
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21A0, 0x01) != IBSU_STATUS_OK) goto done;
		
		// 0x08: Left synchronization pin switch on. 0x88: Both Left & Right  synchronization pins switch 
		//if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21A1, 0x08) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21A1, 0x88) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21A2, 0x88) != IBSU_STATUS_OK) goto done;

		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2090, 0x00) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2091, 0x06) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2094, 0x00) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2095, 0x00) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2096, 0x00) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2097, 0x30) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2098, 0x00) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2099, 0x34) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x209A, 0x00) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x209B, 0x1F) != IBSU_STATUS_OK) goto done;

		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x20A0, 0x00) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x20A1, 0x2C) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x20A4, 0x01) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x20A5, 0xAE) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x20A6, 0x00) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x20A7, 0x30) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x20A8, 0x01) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x20A9, 0xBF) != IBSU_STATUS_OK) goto done;

		// Add (07/25/2016, Document - "New FAP50 Confirmation with 4 Type LE Film 2016.0712.pdf" )
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x211E, 0x01) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x211F, 0x28) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2122, 0x01) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2123, 0x30) != IBSU_STATUS_OK) goto done;

		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2200, 0x10) != IBSU_STATUS_OK) goto done;			// Cfb : 3.625 pF

		Sleep(50);
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2202, RegisterValueH) != IBSU_STATUS_OK) goto done;	// ADC offset :400 (Dec)
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2203, RegisterValueL) != IBSU_STATUS_OK) goto done;

		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x202B, 0x10) != IBSU_STATUS_OK) goto done;	// 7fps Framerate
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x202C, 0xDC) != IBSU_STATUS_OK) goto done;

		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2077, 0x0C) != IBSU_STATUS_OK) goto done;	// LE ON
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2078, 0x50) != IBSU_STATUS_OK) goto done;

		Sleep(100);
		if (_FPGA_SetRegister(0x01, 0x01) != IBSU_STATUS_OK) goto done;	// set capture flag

		Sleep(300);
	}
	else	// from ES2
	{   
		if(m_UsbDeviceInfo.bWindowsMode)
		{
			if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			{
				// windowing
				if (ASIC_SetRegister_Five0(FIVE0_ASIC_MASTER_ADDRESS, 0x2024, 0xD2) != IBSU_STATUS_OK) goto done;	// drop front channels (440 pixel) 4 = 110 = 0x6E 
				if (ASIC_SetRegister_Five0(FIVE0_ASIC_SLAVE_ADDRESS, 0x2025, 0xD2) != IBSU_STATUS_OK) goto done;	// drop rear channels (440 pixel) 4 = 110 = 0x6E 
		
				if (_FPGA_SetRegister(0x35, (UCHAR)(width/2 & 0xFF)) != IBSU_STATUS_OK) goto done;
				if (_FPGA_SetRegister(0x36, (UCHAR)((width/2 >> 8) & 0xFF)) != IBSU_STATUS_OK) goto done;
			}
			else
			{
				if (_FPGA_SetRegister(0x35, (UCHAR)(width & 0xFF)) != IBSU_STATUS_OK) goto done;
				if (_FPGA_SetRegister(0x36, (UCHAR)((width >> 8) & 0xFF)) != IBSU_STATUS_OK) goto done;
			}
		}

		// EL Initailize 
		if (_UsbBulkOutIn(EP1OUT, CMD_INIT_LE_VOLTAGE, NULL, 0, -1, NULL, 0, NULL) != IBSU_STATUS_OK) goto done;
		if (_UsbBulkOutIn(EP1OUT, CMD_WRITE_LE_VOLTAGE, &voltage_value, 1, -1, NULL, 0, NULL) != IBSU_STATUS_OK) goto done;
		Sleep(10);

		// ASIC initialization
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21A3, 0x09) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21A4, 0x88) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21A5, 0x08) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2313, 0x20) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21AC, 0x08) != IBSU_STATUS_OK) goto done;

		// LTPS Timing
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21A0, 0x01) != IBSU_STATUS_OK) goto done;
		// 0x08: Left synchronization pin switch on. 0x88: Both Left & Right  synchronization pins switch 
		//if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21A1, 0x08) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21A1, 0x88) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x21A2, 0x88) != IBSU_STATUS_OK) goto done;

		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2200, 0x10) != IBSU_STATUS_OK) goto done;		// Cfb 

		Sleep(50);
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2202, RegisterValueH) != IBSU_STATUS_OK) goto done;	// ADC offset :400 (Dec)
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2203, RegisterValueL) != IBSU_STATUS_OK) goto done;

		if(m_UsbDeviceInfo.bArrangeMode == TRUE &&
			m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
			if(m_propertyInfo.bEnableTOFforROLL &&
				m_UsbDeviceInfo.bCanUseTOF && m_propertyInfo.bEnableTOF)
			{
				if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x202B, 0x10) != IBSU_STATUS_OK) goto done;	// 7fps Framerate
				if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x202C, 0xDC) != IBSU_STATUS_OK) goto done;

				if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2077, 0x0C) != IBSU_STATUS_OK) goto done;	// LE On time
				if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2078, 0x00) != IBSU_STATUS_OK) goto done;
			}
			else
			{
				if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x202B, 0x0B) != IBSU_STATUS_OK) goto done;	// 10fps Framerate
				if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x202C, 0x92) != IBSU_STATUS_OK) goto done;
				
				if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2077, 0x06) != IBSU_STATUS_OK) goto done;	// LE On time
				if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2078, 0xA0) != IBSU_STATUS_OK) goto done;
			}
		}
		else
		{
			if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x202B, 0x10) != IBSU_STATUS_OK) goto done;	// 7fps Framerate
			if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x202C, 0xDC) != IBSU_STATUS_OK) goto done;

			if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2077, 0x0C) != IBSU_STATUS_OK) goto done;	// LE On time
			if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2078, 0x00) != IBSU_STATUS_OK) goto done;
		}

		// Normal Output - RAW IMAGE
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2048, 0x00) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2041, 0x12 | 0x80) != IBSU_STATUS_OK) goto done; // 0x80 image vertical flip

		// when the output image is vertical flip
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x218A, 0x01) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x218B, 0x23) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x218C, 0x4B) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x218D, 0x65) != IBSU_STATUS_OK) goto done;
	    
		// For Linearity
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2A05, 0x34) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2A06, 0x36) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2305, 0x00) != IBSU_STATUS_OK) goto done;	// default set
		
		if( m_UsbDeviceInfo.bArrangeMode == TRUE )
			_LoadLinearity();

		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2013, 0x01) != IBSU_STATUS_OK) goto done;

		if(m_UsbDeviceInfo.nASICVersion >= 2) // for CUT3
		{
			if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2204, 0x0A) != IBSU_STATUS_OK) goto done;
			if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2205, 0x04) != IBSU_STATUS_OK) goto done;
		}
		else
		{
			if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2204, 0x00) != IBSU_STATUS_OK) goto done;
			if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2205, 0x00) != IBSU_STATUS_OK) goto done;
		}
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2304, 0x02) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x234F, 0x2A) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x234E, 0x02) != IBSU_STATUS_OK) goto done;
		Sleep(400);

		if (_FPGA_SetRegister(0x01, 0x01) != IBSU_STATUS_OK) goto done;	// set capture flag
	}

done:

	if(m_UsbDeviceInfo.nASICVersion <= 0)
	{
		// stream on
		ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2013, 0x01);
		Sleep(100);
		ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2013, 0x00);
		Sleep(100);

		// Normal Output
		ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2048, 0x00);
		ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2041, 0x12 | 0x80); // 0x80 image vertical flip
		
		// when the output image is vertical flip
		ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x218A, 0x01);
		ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x218B, 0x23);
		ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x218C, 0x4B);
		ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x218D, 0x65);

		ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2013, 0x01);

		//----------- 20161212 Short Circuit Detection Improvement -----------//
		ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2204, 0x00);
		ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2205, 0x00);
		//----------- IB Added for Initial Power Consumption -----------//
		ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x2304, 0x13);
		ASIC_SetRegister_Five0(FIVE0_ASIC_GLOBAL_ADDRESS, 0x234F, 0x2A);
		//----------- IB Added for Initial Power Consumption -----------//
		// Delay Time : 2 Frames (Based on Report Rate)
		Sleep(100);
		//----------- 20161212 Short Circuit Detection Improvement -----------//
	}

#if !defined _POWER_OFF_ANALOG_TOUCH_WHEN_NO_CAPTURE_
	if( (m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_FIVE0_TOF_BETA_ || m_UsbDeviceInfo.bCanUseTOF) &&
		 m_propertyInfo.bEnableTOF &&
		(m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER ||
		(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER && m_propertyInfo.bEnableTOFforROLL)) )
	{
		if (_CalibrateTOF() == TRUE)
		{
			m_bTOFCalibrated = TRUE;
		}
		else
		{
			m_bTOFCalibrated = FALSE;
		}
	}
#endif
	
	m_bNeedtoInitializeCIS_Five0 = FALSE;

	return IBSU_STATUS_OK;
}

int CMainCapture::_InitializeTOFSensor_Five0(BOOL useDigitalTouch, BOOL useAnalogTouch)
{
//	BYTE	outBuffer[64];
/*	if(useDigitalTouch)
	{
//		outBuffer[0] = 0x00;
//		_UsbBulkOutIn(EP1OUT, 0x26, outBuffer, 1, -1, NULL, 0, NULL);//0x26 => CMD_U40_SWITCH
//		Sleep(100);
		if (_FPGA_SetRegister(0x30, 0x55) != IBSU_STATUS_OK) goto done;
	}
*/
	if(useAnalogTouch)
	{
//		outBuffer[0] = 0x01;
//		_UsbBulkOutIn(EP1OUT, 0x26, outBuffer, 1, -1, NULL, 0, NULL);//0x26 => CMD_U40_SWITCH
		
		if (_FPGA_SetRegister(0x68, 0x01) != IBSU_STATUS_OK) goto done;
		if (_FPGA_SetRegister(0x68, 0x00) != IBSU_STATUS_OK) goto done;
		Sleep(10);
		if (_FPGA_SetRegister(0x31, 0x3E) != IBSU_STATUS_OK) goto done;
		Sleep(100);
		if (_FPGA_SetRegister(0x30, 0x62) != IBSU_STATUS_OK) goto done;
		if (_FPGA_SetRegister(0x31, 0x3F) != IBSU_STATUS_OK) goto done;
		Sleep(400);
		if (_FPGA_SetRegister(0x33, 0x03) != IBSU_STATUS_OK) goto done;
		if (_FPGA_SetRegister(0x33, 0x00) != IBSU_STATUS_OK) goto done;
	}

	return 1;

done:
	return 0;
}

int CMainCapture::_CalibrateTOF()
{
	TRACE("called _CalibrateTOF() ==> ");

	BYTE StreamOn = 0;
    double elapsed;
   	struct timeval end_time, start_time;
	int OneCount = 0;
	int TIMEOUT_THRESHOLD = 5000; // 10 seconds timeout

	if(_FPGA_GetRegister(0x2F, &StreamOn) != IBSU_STATUS_OK)
		goto done;

	if(StreamOn == 0)
		_FPGA_SetRegister(0x2F, 0x01);

	_FPGA_SetRegister(0x2F, StreamOn);


    gettimeofday(&start_time, NULL);

	// calibrate analog touch sensor
	_InitializeTOFSensor_Five0(FALSE, TRUE);
	m_Five0_TouchSensorMode = TOUCH_SENSOR_MODE_ANALOG;

	OneCount = 0;
	while(1)
	{
		if(_ReadTOFSensorAnalog_Five0(&m_nAnalogTouch_PlateOrg, &m_nAnalogTouch_FilmOrg,
										&m_nAnalogTouch_Plate, &m_nAnalogTouch_Film) != IBSU_STATUS_OK)
			break;

#ifdef __G_DEBUG__
		TRACE("m_nAnalogTouch_Plate : %d, m_nAnalogTouch_Film : %d\n", m_nAnalogTouch_Plate, m_nAnalogTouch_Film);
#endif
		
		if(m_nAnalogTouch_Plate != -1)
		{
			OneCount++;
			if(OneCount > 5)
				break;
		}
		else
			OneCount = 0;
		Sleep(50);
	
        gettimeofday(&end_time, NULL);
        elapsed = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
        if((int)(elapsed*1000) > TIMEOUT_THRESHOLD)
			goto done;
	}

    gettimeofday(&m_LastCalibrate_tv, NULL);

//	SetEvent(m_hTOFSensorThread_ReadEvent);
	TRACE("OK\n");
	return 1;

done:
	
//	ResetEvent(m_hTOFSensorThread_ReadEvent);
	TRACE("FAILED\n");
	return 0;
}

int CMainCapture::_IsNeedInitializeCIS_Five0()
{
	int framesize = 800*1000, read_size;
	UCHAR reg_val[3];
	if (_FPGA_GetRegister(0x07,	&reg_val[0]) != IBSU_STATUS_OK) goto done;		// 0x66 for 972, 0x36 for 1080
	if (_FPGA_GetRegister(0x08,	&reg_val[1]) != IBSU_STATUS_OK) goto done;		// 0x66 for 972, 0x36 for 1080
	if (_FPGA_GetRegister(0x09,	&reg_val[2]) != IBSU_STATUS_OK) goto done;		// 0x66 for 972, 0x36 for 1080

	read_size = (reg_val[2]<<16) + (reg_val[1]<<8) + (reg_val[0]);

	if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		framesize = 400*1000;
	}

	if( framesize != read_size )
		return TRUE;

done:
	return FALSE;
}

int CMainCapture::_LoadLinearity()
{
	UCHAR			eeprom_data_read[256]={0,};
	int				i = 0, loop;
	
	if(_ReadByteFromEEPROM(0, &eeprom_data_read[0]) != IBSU_STATUS_OK) goto done;
	if(_ReadByteFromEEPROM(1, &eeprom_data_read[1]) != IBSU_STATUS_OK) goto done;

	if(eeprom_data_read[0] == 255 || eeprom_data_read[1] == 255)
		goto done;

	// EEPROM version 1.0
	if(eeprom_data_read[0] == 1 && eeprom_data_read[1] == 0)
	{
		if(_ReadByteFromEEPROM(3, &eeprom_data_read[3]) != IBSU_STATUS_OK) goto done;

		loop = eeprom_data_read[3];

		for(i = EEPROM_PACKET_DATA_AREA_ADDRESS_START; loop > 0; i+=PACKET_DATA_SIZE, loop--)
		{
			if(_ReadByteFromEEPROM(i, &eeprom_data_read[i]) != IBSU_STATUS_OK) goto done;
			if(_ReadByteFromEEPROM(i+1, &eeprom_data_read[i+1]) != IBSU_STATUS_OK) goto done;
			if(_ReadByteFromEEPROM(i+2, &eeprom_data_read[i+2]) != IBSU_STATUS_OK) goto done;
			if(_ReadByteFromEEPROM(i+3, &eeprom_data_read[i+3]) != IBSU_STATUS_OK) goto done;
		}

		if(_ReadByteFromEEPROM(i, &eeprom_data_read[i]) != IBSU_STATUS_OK) goto done;

		UCHAR checkSum = _CalcCheckSum(&eeprom_data_read[3], (eeprom_data_read[3]*PACKET_DATA_SIZE) + 1);
		if(eeprom_data_read[i] != checkSum)
		{
			goto done;
		}
		//------------------------------------------------------------------------------

		// Set ASIC Register -------------------------------------------------------------------
		loop = eeprom_data_read[3];

		for(i = EEPROM_PACKET_DATA_AREA_ADDRESS_START; loop > 0; i+=PACKET_DATA_SIZE, loop--)
		{
			UCHAR	ASIC_Addr_type		= eeprom_data_read[i];
			WORD	ASIC_Addr			= (eeprom_data_read[i+1] << 8) | eeprom_data_read[i+2];
			UCHAR	data				= eeprom_data_read[i+3];

			if (ASIC_SetRegister_Five0(ASIC_Addr_type, ASIC_Addr, data) != IBSU_STATUS_OK)
			{
				goto done;
			}
			//TRACE("ASIC_SetRegister_Five0 : %X, %X, %X\n", ASIC_Addr_type, ASIC_Addr, data);
		}
	}
	//--------------------------------------------------------------------------------------

	return TRUE;

done:

	return FALSE;
}

int CMainCapture::_ReadByteFromEEPROM(UCHAR regAddr, UCHAR *regVal)
{
	UCHAR		outBuffer[MAX_DATA_BYTES+1]={0,};
	int			nRc;
	LONG			nReadByte=0;
	UCHAR		inBuffer[256];
	
	outBuffer[0] = (UCHAR)regAddr;
	outBuffer[1] = (UCHAR)0x01;

	nRc = _UsbBulkOutIn( EP1OUT, FIVE0_READ_ASIC_EEPROM, outBuffer, 2, EP1IN, (PUCHAR)inBuffer, 1, &nReadByte );

	if(nRc == IBSU_STATUS_OK)
		*regVal = inBuffer[0];

	Sleep(10);

	return nRc;
}

int	CMainCapture::Capture_SetLEVoltage_Five0(int voltageValue)
{
    return IBSU_STATUS_OK;
    /*	switch(voltageValue)
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
    	}

    	return IBSU_ERR_INVALID_PARAM_VALUE;
    */
}

int	CMainCapture::Capture_GetLEVoltage_Five0(int *voltageValue)
{
    return IBSU_STATUS_OK;
    /*	int nRc;
    	int ReadVoltage;

    	nRc = _GetLEVoltage(&ReadVoltage);

    	switch(ReadVoltage)
    	{
    	case 0x96:	*voltageValue = 0;	break;
    	case 0x91:	*voltageValue = 1;	break;
    	case 0x8C:	*voltageValue = 2;	break;
    	case 0x87:	*voltageValue = 3;	break;
    	case 0x82:	*voltageValue = 4;	break;
    	case 0x7D:	*voltageValue = 5;	break;
    	case 0x78:	*voltageValue = 6;	break;
    	case 0x73:	*voltageValue = 7;	break;
    	case 0x6E:	*voltageValue = 8;	break;
    	case 0x69:	*voltageValue = 9;	break;
    	case 0x64:	*voltageValue = 10;	break;
    	case 0x5F:	*voltageValue = 11;	break;
    	case 0x5A:	*voltageValue = 12;	break;
    	case 0x55:	*voltageValue = 13;	break;
    	case 0x50:	*voltageValue = 14;	break;
    	case 0x4B:	*voltageValue = 15;	break;
    	default:	*voltageValue = 0;	break;
    	}

    	return nRc;
    */
}

int CMainCapture::_SetLEOperationMode_Five0(WORD addr)
{
    UCHAR		outBuffer[64]={0};

    if (addr == ADDRESS_LE_AUTO_MODE)
    {
    return IBSU_STATUS_OK;
    }

    if (addr == ADDRESS_LE_ON_MODE)
    {
        // LE ON operation
        outBuffer[0] = (UCHAR)0xFF;
        return _UsbBulkOutIn( EP1OUT, CMD_INIT_LE_VOLTAGE, outBuffer, 0, -1, NULL, 0, NULL );
    }

    // LE OFF operation
    return _FPGA_SetRegister(0x27, 0x00);
}

AcuisitionState CMainCapture::_GoJob_PreImageProcessing_Five0(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage)
{
	int Bright = 0;
    *bIsGoodImage = FALSE;

	if (m_nFrameCount == 0 && m_propertyInfo.nLEOperationMode == ADDRESS_LE_OFF_MODE)
    {
		if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
			memset(m_pAlgo->m_TFT_MaskImg, 0, m_UsbDeviceInfo.CisImgSize);
			memcpy(m_pAlgo->m_TFT_MaskImg, InImg, m_UsbDeviceInfo.CisImgSize_Roll);
			memset(OutImg, 0, m_UsbDeviceInfo.CisImgSize_Roll);
		}
		else
		{
			memcpy(m_pAlgo->m_TFT_MaskImg, InImg, m_UsbDeviceInfo.CisImgSize);
			memset(OutImg, 0, m_UsbDeviceInfo.CisImgSize);
		}
        m_propertyInfo.nLEOperationMode = ADDRESS_LE_ON_MODE;
        _SetLEOperationMode(m_propertyInfo.nLEOperationMode);
//		_FPGA_SetRegister(0x27, 0x01);
		_ApplyCaptureSettings_Five0(m_FullCaptureSetting.CurrentDAC);
//            m_nFrameCount++;
		return ACUISITION_NOT_COMPLETE;
    }

	if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		m_pAlgo->_Algo_Five0_AnalysisImage(InImg, OutImg, m_UsbDeviceInfo.CisImgSize_Roll, 10);		
		m_pAlgo->m_cImgAnalysis.isDetected = m_pAlgo->_Algo_Five0_GetBrightWithRawImage_forDetectOnly_Roll(OutImg, 100);
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Five0_GetBrightWithRawImage_Roll(OutImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);

		// 지문 개수 루틴
		m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(Bright);
	}
	else
	{
		m_pAlgo->_Algo_Five0_AnalysisImage(InImg, OutImg, m_UsbDeviceInfo.CisImgSize, 10);
		memcpy(m_pAlgo->m_SBDAlg->m_OriginalImg, OutImg, m_UsbDeviceInfo.CisImgSize);

		m_pAlgo->m_cImgAnalysis.isDetected = m_pAlgo->_Algo_Five0_GetBrightWithRawImage_forDetectOnly(m_pAlgo->m_SBDAlg->m_OriginalImg, 100);
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Five0_GetBrightWithRawImage(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
            &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright, &m_pAlgo->m_cImgAnalysis.bright_pixel_count);

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
				if(m_propertyInfo.bEnableTOF && m_propertyInfo.bEnableTOFforROLL)
				{
					m_pAlgo->_Algo_Five0_AutoCapture_TOF(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_FullCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood, m_nAnalogTouch_Plate);
					_ApplyCaptureSettings_Five0(m_FullCaptureSetting.CurrentDAC);
				}
				else
				{
					m_pAlgo->_Algo_Five0_AutoCapture(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_FullCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood);
					_ApplyCaptureSettings_Five0(m_FullCaptureSetting.CurrentDAC);
				}
			}
			else
			{
				if(m_propertyInfo.bEnableTOF)
				{
					m_pAlgo->_Algo_Five0_AutoCapture_TOF(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_FullCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood, m_nAnalogTouch_Plate);
					_ApplyCaptureSettings_Five0(m_FullCaptureSetting.CurrentDAC);
				}
				else
				{
					m_pAlgo->_Algo_Five0_AutoCapture(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_FullCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood);
					_ApplyCaptureSettings_Five0(m_FullCaptureSetting.CurrentDAC);
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
//						m_FullCaptureSetting.CurrentGain = m_DeciCaptureSetting.CurrentGain;
//						m_FullCaptureSetting.CurrentDAC = m_DeciCaptureSetting.CurrentDAC;
						*bIsGoodImage = TRUE;
				}
			}
        }
    }
    else
    {
		m_pAlgo->_Algo_Five0_AutoCapture(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_FullCaptureSetting, (int)(m_clbkProperty.imageInfo.FrameTime * 1000), m_UsbDeviceInfo.CisImgSize/4, &m_pAlgo->m_CaptureGood);
		_Set_CIS_GainRegister(m_propertyInfo.nContrastValue, FALSE, TRUE);
    }

    IBSU_FingerQualityState savedQualityArray[4];
    memcpy(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray));
	m_pAlgo->_Algo_JudgeFingerQuality(&m_pAlgo->m_cImgAnalysis, bIsGoodImage, &m_propertyInfo, &m_clbkProperty, m_DisplayWindow.dispInvalidArea); //Modify Sean to check finger in invalid area

    if (!m_bFirstSent_clbkClearPlaten)
	{
        if (memcmp(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray)) != 0)
        {
            _PostCallback(CALLBACK_FINGER_QUALITY);
        }
	}

    return ACUISITION_NONE;
}

void CMainCapture::_PostImageProcessing_ForPreview_Five0(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage)
{
#if !(defined(__embedded__)  || defined(WINCE))
	if (m_UsbDeviceInfo.nFpgaVersion >= ((0 << 16) | (9 << 8) | 1) /*0.9.1*/)
	{
		memcpy(m_pAlgo->m_Inter_Img2, InImg, m_UsbDeviceInfo.CisImgSize);
		m_pAlgo->_Algo_RemoveTFTDefect(m_pAlgo->m_Inter_Img2, InImg, m_pAlgo->m_TFT_DefectMaskImg);

/*		if (m_propertyInfo.bEnableTOF)
		{
			m_pAlgo->_Algo_Five0_RemoveAnalogTouchSensorNoise(InImg, InImg, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight, 30);
		}
*/	}
#endif

#ifdef __embedded__
	if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
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

		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(InImg, OutImg, m_UsbDeviceInfo.ImgSize_Roll);
	}
	else if( m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
  			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
	{
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(InImg, OutImg, m_UsbDeviceInfo.ImgSize);
	}
#else
	if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
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

        if (m_FullCaptureSetting.CurrentDAC < 0xE7)
        {
		    m_pAlgo->m_SBDAlg->_VignettingEffect_preview_roll(InImg, m_pAlgo->m_Inter_Img2, 5);
		    m_pAlgo->_Algo_HistogramNormalize(m_pAlgo->m_Inter_Img2, OutImg, m_UsbDeviceInfo.ImgSize_Roll, 0);
        }
        else
        {
		    m_pAlgo->m_SBDAlg->_VignettingEffect_preview_roll(InImg, OutImg, 5);
        }
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgSize_Roll);
	}
	else if( m_propertyInfo.ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
  			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS ||
			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS )
	{
        if (m_FullCaptureSetting.CurrentDAC < 0xE7)
        {
			m_pAlgo->m_SBDAlg->_VignettingEffect_preview(InImg, m_pAlgo->m_Inter_Img2, 5);
			m_pAlgo->_Algo_HistogramNormalize(m_pAlgo->m_Inter_Img2, OutImg, m_UsbDeviceInfo.ImgSize, 0);
		}
        else
        {
		    m_pAlgo->m_SBDAlg->_VignettingEffect_preview(InImg, OutImg, 5);
        }
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgSize);
	}
#endif
    return;
}

void CMainCapture::_PostImageProcessing_ForResult_Five0(BYTE *InImg, BYTE *OutImg)
{
	int Bright=0;

    if( m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		m_pAlgo->_Algo_RemoveTFTDefect(InImg, m_pAlgo->m_Inter_Img2, m_pAlgo->m_TFT_DefectMaskImg);
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Five0_GetBrightWithRawImage_Roll(m_pAlgo->m_Inter_Img2, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);

		m_pAlgo->m_SBDAlg->m_MinusMaskType = MINUS_MASK_MEAN;
		m_pAlgo->m_SBDAlg->m_TARGET_STRETCH_HISTOGRAM = 200 + m_pAlgo->m_SBDAlg->m_nContrastTres*5;
		if(m_pAlgo->m_SBDAlg->m_TARGET_STRETCH_HISTOGRAM > 255)
			m_pAlgo->m_SBDAlg->m_TARGET_STRETCH_HISTOGRAM = 255;
		m_pAlgo->m_SBDAlg->m_CMOS_NOISE_AMOUNT = 0;
		//m_pAlgo->m_SBDAlg->_Apply_SBD_final_Five0(m_pAlgo->m_Inter_Img2, OutImg);
		sprintf(m_pAlgo->m_SBDAlg->m_strUnsharpParam, "%.1f", m_propertyInfo.nSharpenValue);
		m_pAlgo->m_SBDAlg->_Apply_SBD_final_Five0_double_enlarge(m_pAlgo->m_Inter_Img2, OutImg);

		// Algo split image
		memset(m_pAlgo->m_OutSplitResultArray, 0x00, m_UsbDeviceInfo.ImgSize*IBSU_MAX_SEGMENT_COUNT);
		memset(m_pAlgo->m_OutSplitResultArrayEx, 0x00, m_UsbDeviceInfo.ImgSize*IBSU_MAX_SEGMENT_COUNT);

		m_pAlgo->_Algo_SegmentFinger(OutImg);

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
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitResultArray, m_pAlgo->m_OutSplitResultArray, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize_Roll);
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(m_pAlgo->m_OutSplitResultArrayEx, m_pAlgo->m_OutSplitResultArrayEx, m_pAlgo->m_segment_arr.SegmentCnt*m_UsbDeviceInfo.ImgSize_Roll);
		// Convert to Result image
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgSize_Roll);
	}
	else
	{
		m_pAlgo->_Algo_RemoveTFTDefect(InImg, m_pAlgo->m_Inter_Img2, m_pAlgo->m_TFT_DefectMaskImg);

		m_pAlgo->m_SBDAlg->m_MinusMaskType = MINUS_MASK_MEAN;
		m_pAlgo->m_SBDAlg->m_TARGET_STRETCH_HISTOGRAM = 200 + m_pAlgo->m_SBDAlg->m_nContrastTres*5;
		if(m_pAlgo->m_SBDAlg->m_TARGET_STRETCH_HISTOGRAM > 255)
			m_pAlgo->m_SBDAlg->m_TARGET_STRETCH_HISTOGRAM = 255;
		m_pAlgo->m_SBDAlg->m_CMOS_NOISE_AMOUNT = 0;
		//m_pAlgo->m_SBDAlg->_Apply_SBD_final_Five0(m_pAlgo->m_Inter_Img2, OutImg);
		sprintf(m_pAlgo->m_SBDAlg->m_strUnsharpParam, "%.1f", m_propertyInfo.nSharpenValue);
		m_pAlgo->m_SBDAlg->_Apply_SBD_final_Five0_double_enlarge(m_pAlgo->m_Inter_Img2, OutImg);

		// Algo split image
		memset(m_pAlgo->m_OutSplitResultArray, 0x00, m_UsbDeviceInfo.ImgSize*IBSU_MAX_SEGMENT_COUNT);
		memset(m_pAlgo->m_OutSplitResultArrayEx, 0x00, m_UsbDeviceInfo.ImgSize*IBSU_MAX_SEGMENT_COUNT);
		
		m_pAlgo->_Algo_SegmentFinger(OutImg);

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
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgSize);
	}

    return;
}

int CMainCapture::_GoJob_DummyCapture_ForThread_Five0(BYTE *InImg)
{
	int Bright, finger_count;

//	_FPGA_SetRegister(0x27, 0x00); // LE off from FPGA
    m_propertyInfo.nLEOperationMode = ADDRESS_LE_OFF_MODE;
    _SetLEOperationMode(m_propertyInfo.nLEOperationMode);

	if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		m_pAlgo->_Algo_Five0_AnalysisImage(InImg, m_pAlgo->m_SBDAlg->m_OriginalImg, m_UsbDeviceInfo.CisImgSize_Roll, 10);          
		m_pAlgo->m_cImgAnalysis.isDetected = m_pAlgo->_Algo_Five0_GetBrightWithRawImage_forDetectOnly_Roll(m_pAlgo->m_SBDAlg->m_OriginalImg, 100);
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Five0_GetBrightWithRawImage_Roll(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);

		// 지문개수루틴
		finger_count = m_pAlgo->_Algo_GetFingerCount(Bright);
	}
	else
	{
		m_pAlgo->_Algo_Five0_AnalysisImage(InImg, m_pAlgo->m_SBDAlg->m_OriginalImg, m_UsbDeviceInfo.CisImgSize, 10);
		m_pAlgo->m_cImgAnalysis.isDetected = m_pAlgo->_Algo_Five0_GetBrightWithRawImage_forDetectOnly(m_pAlgo->m_SBDAlg->m_OriginalImg, 100);
		m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Five0_GetBrightWithRawImage(m_pAlgo->m_SBDAlg->m_OriginalImg, &m_pAlgo->m_cImgAnalysis.foreground_count, 
			&m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright, &m_pAlgo->m_cImgAnalysis.bright_pixel_count);

		// 지문 개수 루틴
		finger_count = m_pAlgo->_Algo_GetFingerCount(Bright);
	}

	if( finger_count == 0 )
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

void CMainCapture::_GoJob_Initialize_ForThread_Five0()
{
    m_UsbDeviceInfo.LeVoltageControl = SUPPORT_LE_VOLTAGE_CONTROL;

	int WaterImageSize = m_UsbDeviceInfo.nWaterImageSize;

	BYTE *WaterImage = m_pAlgo->m_WaterImage;

	m_UsbDeviceInfo.nFpgaVersion = _GetFpgaVersion(TRUE);

    memset(WaterImage, 0, WaterImageSize);
    memset(m_pAlgo->m_TFT_DefectMaskImg, 0, m_UsbDeviceInfo.CisImgSize);
    m_pAlgo->m_TFT_DefectMaskCount = 0;
    
    if (m_UsbDeviceInfo.bNeedMask)
    {
        if (m_UsbDeviceInfo.nFpgaVersion >= ((0 << 24) | (9 << 16) | (1 << 8) | 0x00) )		// support mask function from FPGA 0.9.1
	    {
	        if(_Algo_Five0_GetWaterMaskFromFlashMemory(WaterImage, WaterImageSize) == TRUE)
			{
				int pos = 0, cnt;
				for (int x = 0; x < m_UsbDeviceInfo.CisImgSize; x++)
				{
					if (m_pAlgo->m_TFT_DefectMaskImg[x] == 255)
					{
						m_pAlgo->m_TFT_DefectMaskCount++;
					}
				}
	
				if (m_pAlgo->m_TFT_DefectMaskCount > 0)
				{
					int CIS_IMG_H = m_UsbDeviceInfo.CisImgHeight;
					int CIS_IMG_W = m_UsbDeviceInfo.CisImgWidth;
	
					if (m_pAlgo->m_TFT_DefectMaskList)
						delete [] m_pAlgo->m_TFT_DefectMaskList;

					m_pAlgo->m_TFT_DefectMaskList = new USHORT[m_pAlgo->m_TFT_DefectMaskCount * 19];
					USHORT(*pTFT_DefectMaskList)[19] = (USHORT( *)[19])m_pAlgo->m_TFT_DefectMaskList;
	
					for (int y = 0; y < CIS_IMG_H; y++)
					{
						for (int x = 0; x < CIS_IMG_W; x++)
						{
							if (m_pAlgo->m_TFT_DefectMaskImg[y * CIS_IMG_W + x] == 255)
							{
								pTFT_DefectMaskList[pos][0] = x;
								pTFT_DefectMaskList[pos][1] = y;
								cnt = 0;
								for (int s = -1; s <= 1; s++)
								{
									if ((y + s) < 0 || (y + s) >= CIS_IMG_H - 1)
									{
										continue;
									}
	
									for (int t = -1; t <= 1; t++)
									{
										if ((x + t) < 0 || (x + t) >= CIS_IMG_W - 1)
										{
											continue;
										}
	
										if (m_pAlgo->m_TFT_DefectMaskImg[(y + s)*CIS_IMG_W + (x + t)] != 255)
										{
											pTFT_DefectMaskList[pos][cnt * 2 + 3] = (x + t);
											pTFT_DefectMaskList[pos][cnt * 2 + 4] = (y + s);
											cnt++;
										}
									}
								}
								pTFT_DefectMaskList[pos][2] = cnt;
								pos++;
							}
						}
					}
				}
	
				m_pAlgo->m_pPropertyInfo->ImageType = ENUM_IBSU_TYPE_NONE;
				memcpy(m_pAlgo->m_Inter_Img5, WaterImage, m_UsbDeviceInfo.CisImgSize);
				m_pAlgo->_Algo_RemoveTFTDefect(m_pAlgo->m_Inter_Img5, WaterImage, NULL);
			}
		}
    }

	if(m_UsbDeviceInfo.bCanUseTOF)
	{
		m_propertyInfo.bEnableTOFforROLL = FALSE;
	}
	else
	{
		m_propertyInfo.bEnableTOF = FALSE;
		m_propertyInfo.bEnableTOFforROLL = FALSE;
	}

	//m_pAlgo->_Algo_MakeUniformityMask(WaterImage);
	m_pAlgo->_Algo_Init_GammaTable();
	m_pAlgo->m_SBDAlg->_Init_SBD_final(WaterImage);
}

int CMainCapture::_ApplyCaptureSettings_Five0(unsigned char DAC_Value)
{
	BYTE	outBuffer[64];
	
	outBuffer[0] = DAC_Value;

	m_CurrentCaptureDAC = DAC_Value;

	return _UsbBulkOutIn(EP1OUT, CMD_WRITE_LE_VOLTAGE, outBuffer, 1, -1, NULL, 0, NULL);
}

BOOL CMainCapture::_GoJob_PostImageProcessing_Five0(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage)
{
	BOOL			bIsFinal = FALSE;
    BOOL			bCompleteAcquisition = FALSE;
    int				nRc = -1;
	int Bright = 0;

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
		if(m_pAlgo->m_cImgAnalysis.finger_count > 0)
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
			m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_Five0_GetBrightWithRawImage_Roll(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y, &Bright);

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
             m_propertyInfo.ImageType == ENUM_IBSU_FLAT_TWO_FINGERS	  ||
			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
			 m_propertyInfo.ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS)
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
			// .... post image processing to make perfect image
			// Take Result image
			_PostCallback(CALLBACK_COMPLETE_ACQUISITION);
#if defined(WINCE)
			for (int i=0; i<3; i++)
			{
				Sleep(10);
				if ((int)m_pOverlayText.size() > 0 ||
					(int)m_pOverlayTextEx.size() > 0 ||
					(int)m_pOverlayLineEx.size() > 0 ||
					(int)m_pOverlayQuadrangleEx.size() > 0 ||
					(int)m_pOverlayShapeEx.size() > 0)
				{
					std::vector<OverlayText *>dstOverlayText;
					std::vector<OverlayTextEx *>dstOverlayTextEx;
					std::vector<OverlayLineEx *>dstOverlayLineEx;
					std::vector<OverlayQuadrangleEx *>dstOverlayQuadrangleEx;
					std::vector<OverlayShapeEx *>dstOverlayShapeEx;
					std::copy(m_pOverlayText.begin(), m_pOverlayText.end(), std::back_inserter(dstOverlayText));
					std::copy(m_pOverlayTextEx.begin(), m_pOverlayTextEx.end(), std::back_inserter(dstOverlayTextEx));
					std::copy(m_pOverlayLineEx.begin(), m_pOverlayLineEx.end(), std::back_inserter(dstOverlayLineEx));
					std::copy(m_pOverlayQuadrangleEx.begin(), m_pOverlayQuadrangleEx.end(), std::back_inserter(dstOverlayQuadrangleEx));
					std::copy(m_pOverlayShapeEx.begin(), m_pOverlayShapeEx.end(), std::back_inserter(dstOverlayShapeEx));
					_DrawClientWindow(m_pAlgo->m_OutResultImg, m_DisplayWindow, &dstOverlayText, &dstOverlayTextEx, &dstOverlayLineEx, &dstOverlayQuadrangleEx, &dstOverlayShapeEx);
					break;
				}
			}
#endif
			m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(m_pAlgo->m_cImgAnalysis.mean);

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

BOOL CMainCapture::_Algo_Five0_GetWaterMaskFromFlashMemory(unsigned char *mask_buf, LONG mask_buf_size)
{
	unsigned char *TotalBuf = new unsigned char [mask_buf_size];
    m_pAlgo->m_TFT_DefectMaskCount = 0;
 
 	// read water mask
    int nRc = -1;
    BOOL isSameUM = FALSE;
//    int voltageValue = 0;

    if (m_propertyInfo.bEnablePowerSaveMode)
    {
        if (_SndUsbFwCaptureStop() != IBSU_STATUS_OK) goto done;
        if (_SndUsbFwCaptureStart() != IBSU_STATUS_OK) goto done;
        if (_InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart,
            m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight) != IBSU_STATUS_OK) goto done;
    }

	if (m_bUniformityMaskPath)
    {
        FILE *fp;

        fp = fopen(m_cUniformityMaskPath, "rb");
        if (fp != NULL)
        {
            const size_t maskSize = m_UsbDeviceInfo.CisImgSize + m_UsbDeviceInfo.CisImgSize/8 + 8; // 8 is the checksum size
            size_t       readSize;

            /* Size+8 is compatible with original version of Columbo um files. Only Size+4 is needed. */
            readSize = fread(TotalBuf, 1, maskSize, fp);
            fclose(fp);

            if (readSize == maskSize)
            {
                m_pAlgo->_Algo_Decrypt_Simple(TotalBuf, TotalBuf, maskSize);
                if (m_pAlgo->_Algo_ConfirmChecksum((DWORD *)TotalBuf, (maskSize / sizeof(DWORD))))
                {
                    isSameUM = TRUE;
                }
            }
        }
    }

    if (isSameUM)
    {
        _PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, 90);

		// copy to mask buf
		memcpy(mask_buf, TotalBuf, m_UsbDeviceInfo.CisImgSize);
		_TFT_DefectMaskDecompress(&TotalBuf[m_UsbDeviceInfo.CisImgSize], m_pAlgo->m_TFT_DefectMaskImg, m_UsbDeviceInfo.CisImgSize);
	
        if (m_propertyInfo.bEnablePowerSaveMode)
        {
            _SndUsbFwCaptureStop();
        }

		delete [] TotalBuf;

        return TRUE;
    }

    _PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, 30);
	nRc = _ReadMaskFromEP8IN_Five0(TotalBuf, m_UsbDeviceInfo.CisImgSize + m_UsbDeviceInfo.CisImgSize/8, 0);		// Bitshift 1인 놈만 현재는 사용한다.
    if (nRc != IBSU_STATUS_OK)
    {
        goto done;
    }

	memcpy(mask_buf, TotalBuf, m_UsbDeviceInfo.CisImgSize);
	_TFT_DefectMaskDecompress(&TotalBuf[m_UsbDeviceInfo.CisImgSize], m_pAlgo->m_TFT_DefectMaskImg, m_UsbDeviceInfo.CisImgSize);

     _PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, 70);
	if (m_bUniformityMaskPath)
    {
        FILE *fp;
        fp = fopen(m_cUniformityMaskPath, "wb");
        if (fp != NULL)
        {
            const size_t   maskSize = m_UsbDeviceInfo.CisImgSize + m_UsbDeviceInfo.CisImgSize/8 + 8; // 8 is the checksum size
            unsigned char *tmpMask = NULL;

            /* Size+8 is compatible with original version of Columbo um files. Only Size+4 is needed. */
			tmpMask = new unsigned char [maskSize];
            memcpy(tmpMask, mask_buf, m_UsbDeviceInfo.CisImgSize);
			_TFT_DefectMaskCompress(m_pAlgo->m_TFT_DefectMaskImg, &tmpMask[m_UsbDeviceInfo.CisImgSize], m_UsbDeviceInfo.CisImgSize);			
            m_pAlgo->_Algo_MakeChecksum((DWORD *)tmpMask, (maskSize / sizeof(DWORD)));
            m_pAlgo->_Algo_Encrypt_Simple(tmpMask, tmpMask, maskSize);
            (void)fwrite(tmpMask, 1, maskSize, fp);
            fclose(fp);
            delete [] tmpMask;
        }
    }

    _PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, 90);

    if (m_propertyInfo.bEnablePowerSaveMode == TRUE)
    {
        _SndUsbFwCaptureStop();
    }

	delete [] TotalBuf;

    return TRUE;

done:

	delete [] TotalBuf;

    if (m_propertyInfo.bEnablePowerSaveMode == TRUE)
    {
        _SndUsbFwCaptureStop();
    }
    else
    {
        /* Need to reset CPLD because CPLD was hanged due to unexpected reason */
        _SndUsbFwCaptureStop();
        _SndUsbFwCaptureStart();
        _InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
    }

    return FALSE;
}
/*
int	CMainCapture::_WriteMask_Five0(const BYTE *MaskData, const int MaskDataLen, const int section, const int start_page)
{
#define FLASH_WRITE_START		(0x37)
#define FLASH_READ_START		(0x38)
#define FLASH_PAGE_NUM_HIGH		(0x39)
#define FLASH_PAGE_NUM_LOW		(0x3A)
#define FLASH_BANK_SELECT		(0x3B)
#define FLASH_READ_BYTE_LOW		(0x3C)
#define FLASH_READ_BYTE_MIDDLE	(0x3D)
#define FLASH_READ_BYTE_HIGH	(0x3E)
#define FLASH_READ_EP_OFF		(0x3F)
#define FLASH_SELECT_SECTION	(0x43)
#define FLASH_PAGE_SIZE			528

	IO_PACKET	io_packet;
    int			nRc = IBSU_STATUS_OK;
    LONG		lByteCount;
	int			i = 0;
	int			page_num;
	LONG		outDataLen = 3;
	unsigned char outputdata_temp[512];
	unsigned char send_data[514];
	ULONG cbSent = 0;
	int timeout = __BULK_TIMEOUT__;

	int page_count = MaskDataLen / 511;
	
	lByteCount = outDataLen + 1;

	io_packet.cmd.Ioctl     = (UCHAR)0x06;

	if ((nRc=_FPGA_SetRegister(FLASH_SELECT_SECTION, section)) != IBSU_STATUS_OK)
    {
        goto release;
    }

	for ( i=0; i<page_count; i++)
	{
		page_num = i + start_page;

		memset(send_data, 0, 514);
		memcpy(&send_data[2], &MaskData[i*511], 511);
		send_data[2+511] = _CalcCheckSum(&send_data[2], 511);

		outputdata_temp[0] = FLASH_BANK_SELECT;
		outputdata_temp[1] = 0x01;
		outputdata_temp[2] = (i%2);
		memcpy(io_packet.cmd.Buffer, outputdata_temp, outDataLen);
		nRc = m_pUsbDevice->WritePipe(EP1OUT, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		outputdata_temp[0] = FLASH_PAGE_NUM_HIGH;
		outputdata_temp[1] = 0x01;
		outputdata_temp[2] = ((page_num>>8)&0xFF);
		memcpy(io_packet.cmd.Buffer, outputdata_temp, outDataLen);
		nRc = m_pUsbDevice->WritePipe(EP1OUT, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		outputdata_temp[0] = FLASH_PAGE_NUM_LOW;
		outputdata_temp[1] = 0x01;
		outputdata_temp[2] = (page_num&0xFF);
		memcpy(io_packet.cmd.Buffer, outputdata_temp, outDataLen);
		nRc = m_pUsbDevice->WritePipe(EP1OUT, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		nRc = m_pUsbDevice->WritePipe(EP2OUT, send_data, 514, &cbSent, timeout);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		outputdata_temp[0] = FLASH_WRITE_START;
		outputdata_temp[1] = 0x01;
		outputdata_temp[2] = 0x01;
		memcpy(io_packet.cmd.Buffer, outputdata_temp, outDataLen);
		nRc = m_pUsbDevice->WritePipe(EP1OUT, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		outputdata_temp[0] = FLASH_WRITE_START;
		outputdata_temp[1] = 0x01;
		outputdata_temp[2] = (0x00);
		memcpy(io_packet.cmd.Buffer, outputdata_temp, outDataLen);
		nRc = m_pUsbDevice->WritePipe(EP1OUT, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		TRACE("mask write : %d, check_sum : %d\n", i,send_data[2+511]);
//		Sleep(41);
		Sleep(18);
	}

	if(page_count*511 < MaskDataLen)
	{
		page_num = i + start_page;

		int remain_size = MaskDataLen%511;

		memset(send_data, 0, 514);
		memcpy(&send_data[2], &MaskData[i*511], remain_size);
		send_data[2+511] = _CalcCheckSum(&send_data[2], 511);

		outputdata_temp[0] = FLASH_BANK_SELECT;
		outputdata_temp[1] = 0x01;
		outputdata_temp[2] = (i%2);
		memcpy(io_packet.cmd.Buffer, outputdata_temp, outDataLen);
		nRc = m_pUsbDevice->WritePipe(EP1OUT, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		outputdata_temp[0] = FLASH_PAGE_NUM_HIGH;
		outputdata_temp[1] = 0x01;
		outputdata_temp[2] = ((page_num>>8)&0xFF);
		memcpy(io_packet.cmd.Buffer, outputdata_temp, outDataLen);
		nRc = m_pUsbDevice->WritePipe(EP1OUT, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		outputdata_temp[0] = FLASH_PAGE_NUM_LOW;
		outputdata_temp[1] = 0x01;
		outputdata_temp[2] = (page_num&0xFF);
		memcpy(io_packet.cmd.Buffer, outputdata_temp, outDataLen);
		nRc = m_pUsbDevice->WritePipe(EP1OUT, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		nRc = m_pUsbDevice->WritePipe(EP2OUT, send_data, 514, &cbSent, timeout);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		outputdata_temp[0] = FLASH_WRITE_START;
		outputdata_temp[1] = 0x01;
		outputdata_temp[2] = 0x01;
		memcpy(io_packet.cmd.Buffer, outputdata_temp, outDataLen);
		nRc = m_pUsbDevice->WritePipe(EP1OUT, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		outputdata_temp[0] = FLASH_WRITE_START;
		outputdata_temp[1] = 0x01;
		outputdata_temp[2] = (0x00);
		memcpy(io_packet.cmd.Buffer, outputdata_temp, outDataLen);
		nRc = m_pUsbDevice->WritePipe(EP1OUT, (PUCHAR)&io_packet.cmd.Ioctl, lByteCount, &cbSent, timeout);
		if(nRc != IBSU_STATUS_OK)
			goto release;

		TRACE("mask write : %d, check_sum : %d\n", i,send_data[2+511]);
	}

release:

	return nRc;
}
*/

int CMainCapture::_ReadMaskFromEP8IN_Five0(unsigned char *mask_buf, int mask_buf_size, const int section)
{
#define FLASH_WRITE_START		(0x37)
#define FLASH_READ_START		(0x38)
#define FLASH_PAGE_NUM_HIGH		(0x39)
#define FLASH_PAGE_NUM_LOW		(0x3A)
#define FLASH_BANK_SELECT		(0x3B)
#define FLASH_READ_BYTE_LOW		(0x3C)
#define FLASH_READ_BYTE_MIDDLE	(0x3D)
#define FLASH_READ_BYTE_HIGH	(0x3E)
#define FLASH_READ_EP_OFF		(0x3F)
#define FLASH_SELECT_SECTION	(0x43)
#define FLASH_PAGE_SIZE				528

//	int			  start_page_addr = 0;

	long          page_count;
    int           remained_buf_size;
    unsigned char checksum;
    UCHAR         *readBuffer;
    unsigned char *pMaskImg;
	int read_size = m_UsbDeviceInfo.nWaterImageSize;

    readBuffer = new UCHAR[read_size];
#ifdef WINCE
	if( readBuffer == NULL ){
		DebugBreak();
	}
#endif
	memset(readBuffer, 0, read_size);

    int   nRc = -1;
    long  i = 0;
    LONG  lActualBytes = 0;

	// section select
	if ((nRc=_FPGA_SetRegister(FLASH_SELECT_SECTION, section)) != IBSU_STATUS_OK)
    {
        goto done;
    }

    page_count = mask_buf_size / (UM_ONE_PAGE_SIZE - 1);
    remained_buf_size = mask_buf_size - page_count * (UM_ONE_PAGE_SIZE - 1);
	
	if ((nRc=_FPGA_SetRegister(FLASH_READ_EP_OFF, 1)) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_EP_OFF, 1) : %d\n"), nRc);
#endif
        goto done;
    }
	
	_ResetFifo();

	if ((nRc=_FPGA_SetRegister(FLASH_READ_BYTE_HIGH, ((read_size>>16)&0xFF))) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_BYTE_HIGH) : %d\n"), nRc);
#endif
        goto done;
    }
	
	if ((nRc=_FPGA_SetRegister(FLASH_READ_BYTE_MIDDLE, ((read_size>>8)&0xFF))) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_BYTE_MIDDLE) : %d\n"), nRc);
#endif
        goto done;
    }
	
	if ((nRc=_FPGA_SetRegister(FLASH_READ_BYTE_LOW, (read_size&0xFF))) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_BYTE_LOW) : %d\n"), nRc);
#endif
        goto done;
    }

	if ((nRc=_FPGA_SetRegister(FLASH_READ_START, 1)) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_START) : %d\n"), nRc);
#endif
        goto done;
    }

	if(read_size > 1024*1024)
	{
		if ((nRc = _UsbBulkOutIn(-1, 0x00, NULL, 0, EP8IN, readBuffer, 1024*1024, &lActualBytes)) != IBSU_STATUS_OK)
		{
#ifdef __G_DEBUG__
 			TRACE(_T("_UsbBulkIn() : lActualBytes %d, nRc %d\n"), lActualBytes, nRc);
#endif
			goto done;
		}
		int remained_size = read_size - 1024*1024;
		if ((nRc = _UsbBulkOutIn(-1, 0x00, NULL, 0, EP8IN, &readBuffer[1024*1024], remained_size, &lActualBytes)) != IBSU_STATUS_OK)
		{
#ifdef __G_DEBUG__
 			TRACE(_T("_UsbBulkIn() : lActualBytes %d, nRc %d\n"), lActualBytes, nRc);
#endif
			goto done;
		}
	}
	else
	{
		if ((nRc = _UsbBulkOutIn(-1, 0x00, NULL, 0, EP8IN, readBuffer, read_size, &lActualBytes)) != IBSU_STATUS_OK)
		{
#ifdef __G_DEBUG__
 			TRACE(_T("_UsbBulkIn() : lActualBytes %d, nRc %d\n"), lActualBytes, nRc);
#endif
			goto done;
		}
	}


#ifdef __G_DEBUG__
    TRACE(_T("lActualBytes : %d\n"), lActualBytes);
#endif
	
	if ((nRc=_FPGA_SetRegister(FLASH_READ_EP_OFF, 0)) != IBSU_STATUS_OK)
    {
#ifdef __G_DEBUG__
 	    TRACE(_T("_FPGA_SetRegister(FLASH_READ_EP_OFF) : %d\n"), nRc);
#endif
        goto done;
    }
	
    pMaskImg = &readBuffer[0];

    for (i = 0; i < page_count; i++)
    {
        checksum = _UM_MAKE_CHECKSUM(&pMaskImg[i * (UM_MAX_PAGE_SIZE)], (UM_ONE_PAGE_SIZE - 1));
        if (checksum != pMaskImg[(i * (UM_MAX_PAGE_SIZE)) + (UM_ONE_PAGE_SIZE - 1)])
        {
			nRc = IBSU_ERR_CHANNEL_IO_READ_FAILED;
            goto done;
        }
        memcpy(&mask_buf[i * (UM_ONE_PAGE_SIZE - 1)], &pMaskImg[i * (UM_MAX_PAGE_SIZE)], (UM_ONE_PAGE_SIZE - 1));
    }

    if (remained_buf_size > 0)
    {
        checksum = _UM_MAKE_CHECKSUM(&pMaskImg[i * (UM_MAX_PAGE_SIZE)], (UM_ONE_PAGE_SIZE - 1));
        if (checksum != pMaskImg[(i * (UM_MAX_PAGE_SIZE)) + (UM_ONE_PAGE_SIZE - 1)])
        {
			nRc = IBSU_ERR_CHANNEL_IO_READ_FAILED;
            goto done;
        }
        memcpy(&mask_buf[i * (UM_ONE_PAGE_SIZE - 1)], &pMaskImg[i * (UM_MAX_PAGE_SIZE)], remained_buf_size);
    }

	delete [] readBuffer;
    return IBSU_STATUS_OK;

done:
	delete [] readBuffer;
    return nRc;
}

int CMainCapture::_ReadTOFSensorDigital_Five0(int *plate, int *film)
{
	BYTE Value;
	int nRc;
	
	if((nRc = _FPGA_GetRegister(0x32, &Value)) != IBSU_STATUS_OK) goto done;

	*plate = Value & 0x01;
	*film = ((Value & 0x02) >> 1 );

	return IBSU_STATUS_OK;

done:

	*plate = 0;
	*film = 0;

	return nRc;
}

int CMainCapture::_ReadTOFSensorAnalog_Five0(int *plate_org, int *film_org, int *plate, int *film)
{
//	int nRc;
//	BYTE tempValue[4];
//    int minVal, maxVal;
//    int refMin = 5480, refMax = 6467;
//    double slope;
//
//	if((nRc = _FPGA_GetRegister(0x60, &tempValue[0])) != IBSU_STATUS_OK) goto done;
//	if((nRc = _FPGA_GetRegister(0x61, &tempValue[1])) != IBSU_STATUS_OK) goto done;
//	if((nRc = _FPGA_GetRegister(0x62, &tempValue[2])) != IBSU_STATUS_OK) goto done;
//	if((nRc = _FPGA_GetRegister(0x63, &tempValue[3])) != IBSU_STATUS_OK) goto done;
//
//	*plate = ( tempValue[0]<<8 ) | tempValue[1];
//	*film = ( tempValue[2]<<8 ) | tempValue[3];
//
//    minVal = (((BYTE)m_propertyInfo.cTOFAnalogPlate[0] << 8) | (BYTE)m_propertyInfo.cTOFAnalogPlate[1]);
//    maxVal = (((BYTE)m_propertyInfo.cTOFAnalogPlate[2] << 8) | (BYTE)m_propertyInfo.cTOFAnalogPlate[3]);
//	
//	TRACE("plate(%d) minVal(%d) maxVal(%d)\n", *plate, minVal, maxVal);
//
//    if (minVal == refMin && maxVal == refMax)
//    {
//        slope = 1;
//    }
//    else if (minVal > 0 && maxVal > minVal)
//    {
//        slope = (double)(refMax-refMin)/(maxVal-minVal);
//    }
//    else
//    {
//        minVal = refMin;
//        maxVal = refMax;
//        slope = 1;
//    }
//
//
//    if (*plate == 0 || *plate < (minVal-2000))
//	{
//		*plate = -1;
//	}
//
//	if (*plate > 1000)
//	{
////TRACE("plate(%d) slope(%.1f) after(%d)\n", *plate-minVal, slope, (int)((*plate-minVal)*slope));
//		*plate = (int)((*plate - minVal)*slope);
//		if (*plate < 0)
//			*plate = 0;
//	}
//
//	return IBSU_STATUS_OK;
//
//done:
//
//	*plate = 0;
//	*film = 0;
//
//	return nRc;

	int nRc;
	BYTE tempValue[4];
    int minVal;
    double slope;

	if((nRc = _FPGA_GetRegister(0x60, &tempValue[0])) != IBSU_STATUS_OK) goto done;
	if((nRc = _FPGA_GetRegister(0x61, &tempValue[1])) != IBSU_STATUS_OK) goto done;
	if((nRc = _FPGA_GetRegister(0x62, &tempValue[2])) != IBSU_STATUS_OK) goto done;
	if((nRc = _FPGA_GetRegister(0x63, &tempValue[3])) != IBSU_STATUS_OK) goto done;

	*plate_org = ( tempValue[0]<<8 ) | tempValue[1];
	*film_org = ( tempValue[2]<<8 ) | tempValue[3];
	*film = *film_org;

    minVal = (((BYTE)m_propertyInfo.cTOFAnalogPlate[0] << 8) | (BYTE)m_propertyInfo.cTOFAnalogPlate[1]);
    slope = (double)(((BYTE)m_propertyInfo.cTOFAnalogPlate[2] << 8) | (BYTE)m_propertyInfo.cTOFAnalogPlate[3]) / 10000.0f;

	m_nAnalogTouch_Min = minVal;
	m_nAnalogTouch_Slope = (float)slope;
	
    if (*plate_org == 0 || 
		*plate_org < (minVal-500) || 
		*plate_org > (minVal+2000))
	{
		*plate = -1;
	}
	else
	{
		*plate = (int)((*plate_org - minVal)*slope);
		if (*plate < 0)
			*plate = 0;
	}

	return IBSU_STATUS_OK;

done:

	*plate_org = 0;
	*film_org = 0;
	*plate = 0;
	*film = 0;

	return nRc;
}
