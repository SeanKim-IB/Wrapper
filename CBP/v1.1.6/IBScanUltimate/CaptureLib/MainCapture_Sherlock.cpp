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

BOOL CMainCapture::_GetOneFrameImage_Sherlock(unsigned char *Image, int ImgSize)
{
    //////////////////////////////////////////////////////////////////////////////
    // one frame read
    int			i, j;
    LONG		lActualBytes = 0;
    int			nRc;

    if (m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK)
    {
        // 2013-03-08 Gon add - this is temporarily method to remove synchronization issue between FPGA to ASIC chip.
        //		ASIC_SetRegister(ASIC_MASTER_ADDRESS, 0x10, 0x80);
        for (i = 0; lActualBytes < ImgSize; i++)
        {
            //			nRc = _UsbBulkOutIn( -1, 0, NULL, 0, EP6IN, m_pAlgo->m_Inter_Img, ImgSize, &lActualBytes );
            //nRc = _UsbBulkIn(EP6IN, m_pAlgo->m_Inter_Img, ImgSize, &lActualBytes);
			nRc = _UsbBulkIn(EP6IN, Image, ImgSize, &lActualBytes);

            if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1))
            {
                // 2013-03-08 Gon add - this is temporarily method to remove synchronization issue between FPGA to ASIC chip.
                //				ASIC_SetRegister(ASIC_MASTER_ADDRESS, 0x10, 0x00);
                return FALSE;
            }


            if (lActualBytes == ImgSize)
            {
                break;
            }
        }

        ///////////////////////////////////////////////////////////////////////////
        // 2013-05-28 enzyme delete - Probably it's unused logic
        /*		for( i=0; lActualBytes<ImgSize; i++ )
        		{
        	//			nRc = _UsbBulkOutIn( -1, 0, NULL, 0, EP6IN, m_pAlgo->m_Inter_Img, ImgSize, &lActualBytes );
        			nRc = _UsbBulkIn(EP6IN, m_pAlgo->m_Inter_Img, ImgSize, &lActualBytes );

        			if( nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1) )
        			{
        				// gon add 2013-03-08 - this is temporarily method to remove synchronization issue between FPGA to ASIC chip.
        //				ASIC_SetRegister(ASIC_MASTER_ADDRESS, 0x10, 0x00);
        				return FALSE;
        			}

        			if( lActualBytes == ImgSize )
        				break;
        		}
        */
        ///////////////////////////////////////////////////////////////////////////

        // 2013-03-08 Gon add - this is temporarily method to remove synchronization issue between FPGA to ASIC chip.
        //		ASIC_SetRegister(ASIC_MASTER_ADDRESS, 0x10, 0x00);

#ifdef __G_PERFORMANCE_DEBUG__
    gettimeofday(&m_pd_start_tv, NULL);
#endif
//        for (i = 0; i < m_UsbDeviceInfo.ImgHeight; i++)
//        {
//            for (j = 0; j < m_UsbDeviceInfo.ImgWidth - 8; j++)
//            {
//                Image[i * m_UsbDeviceInfo.ImgWidth + j] = m_pAlgo->m_Inter_Img[(m_UsbDeviceInfo.ImgWidth - 1 - j) * m_UsbDeviceInfo.ImgHeight + i];
//            }
///*            for (j = m_UsbDeviceInfo.ImgWidth - 8; j < m_UsbDeviceInfo.ImgWidth; j++)
//            {
//                Image[i * m_UsbDeviceInfo.ImgWidth + j] = 0;
//            }
//*/        }		
    }
    else
    {
        for (i = 0; lActualBytes < ImgSize; i++)
        {
            //			nRc = _UsbBulkOutIn( -1, 0, NULL, 0, EP6IN, m_pAlgo->m_Inter_Img, ImgSize, &lActualBytes );
            nRc = _UsbBulkIn(EP6IN, m_pAlgo->m_Inter_Img, ImgSize, &lActualBytes);

            if (nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1))
            {
                return FALSE;
            }

            if (lActualBytes == ImgSize)
            {
                break;
            }
        }

        ///////////////////////////////////////////////////////////////////////////
        // 2013-05-28 enzyme delete - Probably it's unused logic
        /*		for( i=0; lActualBytes<ImgSize; i++ )
        		{
        	//			nRc = _UsbBulkOutIn( -1, 0, NULL, 0, EP6IN, m_pAlgo->m_Inter_Img, ImgSize, &lActualBytes );
        			nRc = _UsbBulkIn(EP6IN, m_pAlgo->m_Inter_Img, ImgSize, &lActualBytes );

        			if( nRc != IBSU_STATUS_OK || (nRc == IBSU_STATUS_OK && lActualBytes == 0 && i > 1) )
        				return FALSE;

        			if( lActualBytes == ImgSize )
        				break;
        		}
        */
        ///////////////////////////////////////////////////////////////////////////

        for (i = 0; i < m_UsbDeviceInfo.ImgHeight; i++)
        {
            for (j = 0; j < m_UsbDeviceInfo.ImgWidth; j++)
            {
                Image[(m_UsbDeviceInfo.ImgHeight - 1 - i)*m_UsbDeviceInfo.ImgWidth + (m_UsbDeviceInfo.ImgWidth - 1 - j)] = m_pAlgo->m_Inter_Img[j * m_UsbDeviceInfo.ImgHeight + i];
                //					Image[i*m_UsbDeviceInfo.ImgWidth+j] = m_pAlgo->m_Inter_Img[j*m_UsbDeviceInfo.ImgHeight+i];
            }
        }
    }

    return TRUE;
}

int CMainCapture::_InitializeForCISRegister_Sherlock(WORD rowStart, WORD colStart, WORD width, WORD height)
{
    DWORD size;

    size = width * height;

    if (m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK)
    {
        /*
        		// rev 0
        		ASIC_Reset();		// ASIC Reset
        		ASIC_Initialize();		// ASIC Initialize

        		//-----------------------------------------------------
        		_FPGA_SetRegister( 0x00, 0x01 );	// set reset
        		Sleep(50);			// for more safe
        		_FPGA_SetRegister( 0x01, 0x00 );	// clear capture flag
        		Sleep(50);			// for more safe
        		_ResetFifo();

        		_FPGA_SetRegister( 0x07, (UCHAR)(size & 0xFF) );		// frame_size
        		_FPGA_SetRegister( 0x08, (UCHAR)((size>>8) & 0xFF) );
        		_FPGA_SetRegister( 0x09, (UCHAR)((size>>16) & 0xFF) );

        		Sleep(50);			// for more safe
        		_FPGA_SetRegister( 0x00, 0x00 );	// clear reset
        		Sleep(50);			// for more safe

        		//-----------------------------------------------------
        		_FPGA_SetRegister( 0x01, 0x01 );	// set capture flag

        		ASIC_SetRegister(ASIC_MASTER_ADDRESS, 0x10, 0x80);

        //		_FPGA_SetRegister(0x25, 0x00);	// LE OFF
        		Sleep(1000);			// Need delay time to get first one frame image from ASIC
        */

        if (_FPGA_SetRegister(0x00, 0x01) != IBSU_STATUS_OK) goto done;	// set reset
        Sleep(50);			// for more safe
        if (_FPGA_SetRegister(0x01, 0x00) != IBSU_STATUS_OK) goto done;	// clear capture flag
        Sleep(50);			// for more safe
        if (_ResetFifo() != IBSU_STATUS_OK) goto done;

        if (_FPGA_SetRegister(0x07, (UCHAR)(size & 0xFF)) != IBSU_STATUS_OK) goto done;		// frame_size
        if (_FPGA_SetRegister(0x08, (UCHAR)((size >> 8) & 0xFF)) != IBSU_STATUS_OK) goto done;
        if (_FPGA_SetRegister(0x09, (UCHAR)((size >> 16) & 0xFF)) != IBSU_STATUS_OK) goto done;

        /////////////////////////////////////////////////////////////////////
        // 2013-03-08 Gon add - To control LE ON time
        size = 480000;			// Good result

        if (_FPGA_SetRegister(0x2C, (UCHAR)((size >> 16) & 0xFF)) != IBSU_STATUS_OK) goto done;	// LE on time
        if (_FPGA_SetRegister(0x2D, (UCHAR)((size >> 8) & 0xFF)) != IBSU_STATUS_OK) goto done;	// LE on time
        if (_FPGA_SetRegister(0x2E, (UCHAR)(size & 0xFF)) != IBSU_STATUS_OK) goto done;	// LE on time

        size = 2000;
        if (_FPGA_SetRegister(0x2B, (UCHAR)(size & 0xFF)) != IBSU_STATUS_OK) goto done;	// LE on time
        if (_FPGA_SetRegister(0x2A, (UCHAR)((size >> 8) & 0xFF)) != IBSU_STATUS_OK) goto done;	// LE on time
        if (_FPGA_SetRegister(0x29, (UCHAR)((size >> 16) & 0xFF)) != IBSU_STATUS_OK) goto done;	// LE on time
        /////////////////////////////////////////////////////////////////////

        Sleep(50);			// for more safe
        if (_FPGA_SetRegister(0x00, 0x00) != IBSU_STATUS_OK) goto done;	// clear reset
        Sleep(50);			// for more safe

        // rev 1
        if (ASIC_Reset() != IBSU_STATUS_OK) goto done;			// ASIC Reset
        Sleep(50);			// for more safe
        if (ASIC_AnalogPower() != IBSU_STATUS_OK) goto done;		// ASIC Analog Power
        Sleep(50);			// for more safe
        if (ASIC_Initialize() != IBSU_STATUS_OK) goto done;		// ASIC Initialize

		if (ASIC_SetRegister(ASIC_MASTER_ADDRESS, 0x22, 0x8C) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister(ASIC_MASTER_ADDRESS, 0x33, 0x32) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister(ASIC_MASTER_ADDRESS, 0x35, 0x30) != IBSU_STATUS_OK) goto done;

		if (ASIC_SetRegister(ASIC_SLAVE_ADDRESS, 0x22, 0x8C) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister(ASIC_SLAVE_ADDRESS, 0x33, 0x32) != IBSU_STATUS_OK) goto done;
		if (ASIC_SetRegister(ASIC_SLAVE_ADDRESS, 0x35, 0x30) != IBSU_STATUS_OK) goto done;

        // 2013-03-11 enzyme modify - To solve issue of movement frame
        //-----------------------------------------------------
        Sleep(100);
        if (_FPGA_SetRegister(0x01, 0x01) != IBSU_STATUS_OK) goto done;	// set capture flag

        //		ASIC_SetRegister(ASIC_MASTER_ADDRESS, 0x10, 0x80);

        //		Sleep(1000);			// Need delay time to get first one frame image from ASIC
    }
    else if (m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK_ROIC)
    {
        //-----------------------------------------------------
        if (_FPGA_SetRegister(0x00, 0x01) != IBSU_STATUS_OK) goto done;	// set reset
        Sleep(50);			// for more safe
        if (_FPGA_SetRegister(0x01, 0x00) != IBSU_STATUS_OK) goto done;	// clear capture flag
        Sleep(50);			// for more safe
        if (_ResetFifo() != IBSU_STATUS_OK) goto done;

        if (_FPGA_SetRegister(0x07, (UCHAR)(size & 0xFF)) != IBSU_STATUS_OK) goto done;		// frame_size
        if (_FPGA_SetRegister(0x08, (UCHAR)((size >> 8) & 0xFF)) != IBSU_STATUS_OK) goto done;
        if (_FPGA_SetRegister(0x09, (UCHAR)((size >> 16) & 0xFF)) != IBSU_STATUS_OK) goto done;

        Sleep(50);			// for more safe
        if (_FPGA_SetRegister(0x00, 0x00) != IBSU_STATUS_OK) goto done;	// clear reset
        Sleep(50);			// for more safe

        if (_FPGA_SetRegister(0xC0, 0x02) != IBSU_STATUS_OK) goto done;		// Pico : 2Pf
        if (_FPGA_SetRegister(0xC1, 0x01) != IBSU_STATUS_OK) goto done;		// LE ON Time : 1

        if (_FPGA_SetRegister(0x25, 0x00) != IBSU_STATUS_OK) goto done;		// LE OFF

        if (_FPGA_SetRegister(0x01, 0x01) != IBSU_STATUS_OK) goto done;	// set capture flag
    }

    return IBSU_STATUS_OK;

done:
    return IBSU_ERR_DEVICE_IO;
}

int	CMainCapture::Capture_SetLEVoltage_Sherlock(int voltageValue)
{
    return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
}

int	CMainCapture::Capture_GetLEVoltage_Sherlock(int *voltageValue)
{
    return IBSU_ERR_DEVICE_NOT_SUPPORTED_FEATURE;
}

int CMainCapture::_SetLEOperationMode_Sherlock(WORD addr)
{
    if ((IBSU_LEOperationMode)addr == ENUM_IBSU_LE_OPERATION_ON)
    {
        return _FPGA_SetRegister(0x25, 0x01);    // LE ON
    }
    else if ((IBSU_LEOperationMode)addr == ENUM_IBSU_LE_OPERATION_OFF)
    {
        return _FPGA_SetRegister(0x25, 0x00);    // LE OFF
    }
    else
    {
        return IBSU_ERR_CHANNEL_IO_COMMAND_FAILED;
    }
}

AcuisitionState CMainCapture::_GoJob_PreImageProcessing_Sherlock(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage)
{
	BOOL tmpbIsGoodImage = FALSE;
    *bIsGoodImage = FALSE;

    if (m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK ||
            m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK_ROIC)
    {
        if (m_nFrameCount == 0 && m_propertyInfo.nLEOperationMode == ADDRESS_LE_OFF_MODE)
        {
            memcpy(m_pAlgo->m_TFT_MaskImg, InImg, m_UsbDeviceInfo.CisImgSize);
            memset(OutImg, 0, m_UsbDeviceInfo.CisImgSize);
            m_propertyInfo.nLEOperationMode = ADDRESS_LE_ON_MODE;
            _SetLEOperationMode(m_propertyInfo.nLEOperationMode);
//            m_nFrameCount++;
			return ACUISITION_NONE;
        }

        // 배경값(최소배경값)을 구하여 그만큼만 배경 노이즈를 제거
        if (m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK)
        {
            m_pAlgo->_Algo_ASIC_AnalysisImage(InImg);
        }
        else
        {
            m_pAlgo->_Algo_ROIC_AnalysisImage(InImg);
        }

        if (m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK)
        {
            // 2013-03-08 Gon modify - To remove dot noise from TFT
			if(m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			{
				m_pAlgo->_Algo_Remove_Noise_using_remove_mask_asic(InImg, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
				m_pAlgo->m_bRotatedImage = FALSE;
				m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_ROIC_GetBrightWithRawImage(m_pAlgo->m_Inter_Img2, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
			}
			else
			{
				if(!m_propertyInfo.bNoPreviewImage)
				{
					m_pAlgo->_Algo_Remove_Noise_using_remove_mask_asic(InImg, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
					m_pAlgo->m_bRotatedImage = FALSE;
					m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_ROIC_GetBrightWithRawImage(m_pAlgo->m_Inter_Img2, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
				}
				else
				{
					m_pAlgo->m_bRotatedImage = TRUE;
					m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_ROIC_GetBrightWithRawImage(InImg, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
				}
			}
            // 지문 개수 루틴
            m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(m_pAlgo->m_cImgAnalysis.mean);

			// 2014-05-28 Gon modify - bug fix : wrong foreground count on Sherlock
			if(!m_propertyInfo.bNoPreviewImage || m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			{
				int x, y;
				int ForeCount[256];
				int label, max_label_cnt;
				memset(ForeCount, 0, sizeof(ForeCount));
				for (y = m_pAlgo->ENLARGESIZE_ZOOM_H; y < m_pAlgo->ZOOM_ENLAGE_H - m_pAlgo->ENLARGESIZE_ZOOM_H; y++)
				{
					for (x = m_pAlgo->ENLARGESIZE_ZOOM_W; x < m_pAlgo->ZOOM_ENLAGE_W - m_pAlgo->ENLARGESIZE_ZOOM_W; x++)
					{
						label = m_pAlgo->m_labeled_segment_enlarge_buffer[y * m_pAlgo->ZOOM_ENLAGE_W + x];
						ForeCount[label]++;
					}
				}

				max_label_cnt = ForeCount[1];
				for (label = 2; label < 255; label++)
				{
					if (max_label_cnt < ForeCount[label])
					{
						max_label_cnt = ForeCount[label];
					}
				}

				if (max_label_cnt < 100)
				{
					memset(m_pAlgo->m_Inter_Img2, 0, m_pAlgo->CIS_IMG_SIZE);
					m_pAlgo->m_cImgAnalysis.foreground_count = 0;
					m_pAlgo->m_cImgAnalysis.LEFT =
						m_pAlgo->m_cImgAnalysis.RIGHT =
							m_pAlgo->m_cImgAnalysis.TOP =
								m_pAlgo->m_cImgAnalysis.BOTTOM = 0;
				}

			}
#if defined(__embedded__)  || defined(WINCE)
	        if(!m_propertyInfo.bNoPreviewImage || m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			{
				if (m_UsbDeviceInfo.nFpgaVersion < ((1 << 16) | (0 << 8) | 3)/*1.0.3*/)
				{
					m_pAlgo->_Algo_Remove_Dot_Noise_asic(m_pAlgo->m_Inter_Img2, OutImg, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
				}
				else
				{
					memcpy(OutImg, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.CisImgSize);
				}
			}
			else
			{
				memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize);
			}
#else
	        if(!m_propertyInfo.bNoPreviewImage || m_propertyInfo.ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			{
				if (m_UsbDeviceInfo.nFpgaVersion >= ((1 << 16) | (0 << 8) | 3)/*1.0.3*/)
				{
					m_pAlgo->_Algo_RemoveTFTDefect(m_pAlgo->m_Inter_Img2, OutImg, m_pAlgo->m_TFT_DefectMaskImg);
				}
				else
				{
					m_pAlgo->_Algo_Remove_Dot_Noise_asic(m_pAlgo->m_Inter_Img2, OutImg, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
				}
			}
			else
			{
				memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize);
			}
#endif
        }
        else
        {
            m_pAlgo->_Algo_Remove_Noise_using_remove_mask(InImg, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
            m_pAlgo->_Algo_Remove_Vertical_Noise(m_pAlgo->m_Inter_Img2, OutImg, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
			m_pAlgo->m_bRotatedImage = FALSE;
            m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_ROIC_GetBrightWithRawImage(OutImg, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);
            // 지문 개수 루틴
            m_pAlgo->m_cImgAnalysis.finger_count = m_pAlgo->_Algo_GetFingerCount(m_pAlgo->m_cImgAnalysis.mean);
        }
     }

    /*
    Change m_nFrameCount from 0 to 2 to remove blank buffer. LE is offed and turned on when Sherlock is initialized
    and it caused blank buffer. 2013-06-12 by Enzyme
    */
    if (m_nFrameCount > 2 && m_pAlgo->m_cImgAnalysis.finger_count == 0)
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
        tmpbIsGoodImage = m_pAlgo->_Algo_Sherlock_AutoCapture(OutImg, m_propertyInfo.nCaptureBrightThresFlat[1], m_propertyInfo.nCaptureBrightThresFlat[0], 
							&m_propertyInfo, (int)(m_clbkProperty.imageInfo.FrameTime * 1000));
        _Set_CIS_GainRegister(m_propertyInfo.nContrastValue, TRUE);
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
	m_pAlgo->_Algo_JudgeFingerQuality(&m_pAlgo->m_cImgAnalysis, bIsGoodImage, &m_propertyInfo, &m_clbkProperty, m_DisplayWindow.dispInvalidArea);

	if (!m_bFirstSent_clbkClearPlaten)
		if (memcmp(&savedQualityArray[0], &m_clbkProperty.qualityArray[0], sizeof(m_clbkProperty.qualityArray)) != 0)
		{
			_PostCallback(CALLBACK_FINGER_QUALITY);
		}

    return ACUISITION_NONE;
}

void CMainCapture::_PostImageProcessing_ForPreview_Sherlock(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage)
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
		m_pAlgo->_Algo_Make_250DPI(InImg, m_pAlgo->m_Inter_Img2);
		m_pAlgo->_Algo_Image_Gamma_250DPI(m_pAlgo->m_Inter_Img2, OutImg, m_nGammaLevel);
    }
    else
    {
        _UpdateGammaLevel(__AUTO_CONTRAST_MAX_VALUE__);
        m_pAlgo->_Algo_VignettingEffect(InImg, m_pAlgo->m_Inter_Img2);
        // 2013-04-03 gon removed : to enhance ridge detail.
        //	m_pAlgo->_Algo_ROIC_RemoveVignettingNoise(m_pAlgo->m_Inter_Img3, m_pAlgo->m_Inter_Img2);
        m_pAlgo->_Algo_Image_Gamma(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img3, m_nGammaLevel);
        m_pAlgo->_Algo_Image_Smoothing(m_pAlgo->m_Inter_Img3, OutImg);
    }
#endif
#elif defined(__IBSCAN_ULTIMATE_SDK__)
	switch (m_propertyInfo.nImageProcessThres)
    {
	case IMAGE_PROCESS_THRES_LOW:
        m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(InImg, OutImg, m_UsbDeviceInfo.ImgSize);
		break;
	case IMAGE_PROCESS_THRES_MEDIUM:
		m_pAlgo->_Algo_VignettingEffect(InImg, OutImg);
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(OutImg, OutImg, m_UsbDeviceInfo.ImgWidth * m_UsbDeviceInfo.ImgHeight);
		break;
	case IMAGE_PROCESS_THRES_HIGH:
    _UpdateGammaLevel(__AUTO_CONTRAST_MAX_VALUE__);
    m_pAlgo->_Algo_VignettingEffect(InImg, m_pAlgo->m_Inter_Img2);
    // 2013-04-03 gon removed : to enhance ridge detail.
    //	m_pAlgo->_Algo_ROIC_RemoveVignettingNoise(m_pAlgo->m_Inter_Img3, m_pAlgo->m_Inter_Img2);
    m_pAlgo->_Algo_Image_Gamma(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img3, m_nGammaLevel);
    m_pAlgo->_Algo_Image_Smoothing(m_pAlgo->m_Inter_Img3, m_pAlgo->m_Inter_Img);

    if (bIsGoodImage)
    {
        m_pAlgo->_Algo_HistogramNormalize(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.ImgWidth * m_UsbDeviceInfo.ImgHeight, m_propertyInfo.nContrastTres);
        m_pAlgo->_Algo_Image_Smoothing(m_pAlgo->m_Inter_Img2, OutImg);
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

void CMainCapture::_PostImageProcessing_ForResult_Sherlock(BYTE *InImg, BYTE *OutImg)
{
    int expect_contrast;

	if (m_propertyInfo.bNoPreviewImage && m_propertyInfo.ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER)
    {
		memcpy(m_pAlgo->m_Inter_Img2, InImg, m_UsbDeviceInfo.CisImgSize);
        m_pAlgo->_Algo_Remove_Noise_using_remove_mask_asic(m_pAlgo->m_Inter_Img2, InImg, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
    }

#if defined(__embedded__)  || defined(WINCE)
	if (m_UsbDeviceInfo.nFpgaVersion >= ((1 << 16) | (0 << 8) | 3)/*1.0.3*/)
	{
		memcpy(m_pAlgo->m_Inter_Img2, InImg, m_UsbDeviceInfo.CisImgSize);
		m_pAlgo->_Algo_RemoveTFTDefect(m_pAlgo->m_Inter_Img2, InImg, m_pAlgo->m_TFT_DefectMaskImg);
	}
#endif
    m_pAlgo->_Algo_VignettingEffect(InImg, m_pAlgo->m_Inter_Img);
    //	m_pAlgo->_Algo_ROIC_RemoveVignettingNoise(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img);
    expect_contrast = m_pAlgo->_Algo_FindExpectContrast(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.CisImgSize, m_propertyInfo.nContrastTres, m_nGammaLevel);
    m_pAlgo->_Algo_Image_Gamma(m_pAlgo->m_Inter_Img, m_pAlgo->m_Inter_Img2, expect_contrast);
    m_pAlgo->_Algo_HistogramNormalize(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img, m_UsbDeviceInfo.ImgSize, m_propertyInfo.nContrastTres);
    m_pAlgo->_Algo_SwUniformity(m_pAlgo->m_Inter_Img, OutImg, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.ImgWidth, m_UsbDeviceInfo.ImgHeight);

#if defined(__IBSCAN_ULTIMATE_SDK__)
    // Algo split image
    // 2013-03-18 enzyme bug fixed - Our background color is 0x00
    // and it makes strange image when you set two-fingers but you get a one-finger with IBSU_TakeResultImageManually()
    //	memset(m_pAlgo->m_OutSplitResultArray, 0xFF, m_UsbDeviceInfo.ImgSize*2);
    //	if( m_propertyInfo.nNumberOfObjects > 1 )
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



int CMainCapture::ASIC_GetRegister(unsigned char main_addr, unsigned char reg_addr, unsigned char *reg_value)
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};
    int			nRc;
    UCHAR		inBuffer[MAX_DATA_BYTES + 1] = {0};

    outBuffer[0] = (UCHAR)main_addr;
    outBuffer[1] = (UCHAR)reg_addr;

    nRc = _UsbBulkOutIn(EP1OUT, CMD_READ_ASIC_REG, outBuffer, 2, EP1IN, (PUCHAR)inBuffer, 1, NULL);
    if (nRc == IBSU_STATUS_OK)
    {
        *reg_value = inBuffer[0];
    }
    else
    {
        *reg_value = 0;
    }

    return nRc;
}

int	CMainCapture::ASIC_SetRegister(unsigned char main_addr, unsigned char reg_addr, unsigned char reg_value)
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};

    outBuffer[0] = (UCHAR)main_addr;
    outBuffer[1] = (UCHAR)reg_addr;
    outBuffer[2] = (UCHAR)reg_value;

    return _UsbBulkOutIn(EP1OUT, CMD_WRITE_ASIC_REG, outBuffer, 3, -1, NULL, 0, NULL);
}

int	CMainCapture::ASIC_SetRegister_Five0(unsigned char main_addr, WORD reg_addr, unsigned char reg_value)
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};

	outBuffer[0] = (BYTE)main_addr;
    outBuffer[1] = (BYTE)((reg_addr >> 8) & 0xFF);
    outBuffer[2] = (BYTE)(reg_addr & 0xFF);
    outBuffer[3] = (BYTE)reg_value;

    return _UsbBulkOutIn(EP1OUT, CMD_WRITE_ASIC_REG, outBuffer, 4, -1, NULL, 0, NULL);
}

int	CMainCapture::ASIC_GetRegister_Five0(unsigned char main_addr, WORD reg_addr, unsigned char *reg_value)
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};
    int			nRc;
	UCHAR		inBuffer[MAX_DATA_BYTES + 1] = {0};

	outBuffer[0] = (BYTE)main_addr;
    outBuffer[1] = (BYTE)((reg_addr >> 8) & 0xFF);
    outBuffer[2] = (BYTE)(reg_addr & 0xFF);

    nRc = _UsbBulkOutIn(EP1OUT, CMD_READ_ASIC_REG, outBuffer, 3, EP1IN, (PUCHAR)inBuffer, 1, NULL);

	if(nRc == IBSU_STATUS_OK)
		*reg_value = inBuffer[0];

	return nRc;
}

int	CMainCapture::ASIC_Reset()
{
    return _UsbBulkOutIn(EP1OUT, CMD_ASIC_RESET, NULL, 0, -1, NULL, 0, NULL);

    /*	int ret = _FPGA_SetRegister(0x26, 0x00);
    	if(ret == IBSU_STATUS_OK)
    	{
    		Sleep(10);
    		ret = _FPGA_SetRegister(0x26, 0x01);
    	}

    	return ret;*/
}

int	CMainCapture::ASIC_AnalogPower()
{
    //	return _UsbBulkOutIn( EP1OUT, CMD_ANALOG_POWER_ON, NULL, 0, -1, NULL, 0, NULL );
/*    char cCmtValue[3];
    char propertyValue[32];

    _FPGA_GetRegister(0xF0, (UCHAR &)cCmtValue[0]);
    _FPGA_GetRegister(0xF1, (UCHAR &)cCmtValue[1]);
    _FPGA_GetRegister(0xF2, (UCHAR &)cCmtValue[2]);
    sprintf((char *)propertyValue, "%d.%d.%d", cCmtValue[0], cCmtValue[1], cCmtValue[2]);

    if ((cCmtValue[0] >= 1) && (cCmtValue[1] >= 0) && (cCmtValue[2] >= 2))
    {
        m_UsbDeviceInfo.bNewFPCB = TRUE;
        m_UsbDeviceInfo.nDefaultContrastValue = __ASIC_VOLTAGE_DEFAULT_VALUE__;
    }

    if (strcmp(propertyValue, "0.9.0") == 0 || strcmp(propertyValue, "1.0.0") == 0)
    {
        return _UsbBulkOutIn(EP1OUT, CMD_ANALOG_POWER_ON, NULL, 0, -1, NULL, 0, NULL);
    }
    else
    {
        return _FPGA_SetRegister(0x28, 0x01);
    }
*/
    if (m_UsbDeviceInfo.nFpgaVersion == -1)
    {
        m_UsbDeviceInfo.nFpgaVersion = _GetFpgaVersion();
    }

    if (m_UsbDeviceInfo.nFpgaVersion >= ((1 << 16) | (0 << 8) | 2))
	{
        m_UsbDeviceInfo.bNewFPCB = TRUE;
        m_UsbDeviceInfo.nDefaultContrastValue = __ASIC_VOLTAGE_DEFAULT_VALUE__;
	}
	
	if ((m_UsbDeviceInfo.nFpgaVersion == ((0 << 16) | (9 << 8) | 0)) ||
		(m_UsbDeviceInfo.nFpgaVersion == ((1 << 16) | (0 << 8) | 0))){
        return _UsbBulkOutIn(EP1OUT, CMD_ANALOG_POWER_ON, NULL, 0, -1, NULL, 0, NULL);
	}
	else
	{
        return _FPGA_SetRegister(0x28, 0x01);
	}
}

int	CMainCapture::ASIC_Initialize()
{
    int i, loop_count, ret;

#if ( _ASIC_FRAMERATE_ == _ASIC_11_2_FRAME_ )
    // setting ASIC(master 01)
    unsigned char asic_master_01[][2] =
    {
        {0x4B, 0x05},
        {0x48, 0x80},
        {0x4D, 0x58},
        {0x48, 0xC0},
        {0x48, 0xD0},
        {0x5D, 0x10},
        {0x60, 0x0B},
        {0x61, 0x25},
        {0x5E, 0x00},
        {0x5F, 0x00},
        {0x62, 0x00},
        {0x59, 0x0B},
        {0x5A, 0x18},
        {0x4E, 0x1E}
    };
    // setting ASIC(master 02)
    unsigned char asic_master_02[][2] =
    {
        {0x48, 0xD0},
        {0x49, 0x20},
        {0x49, 0x30},
        {0x49, 0x38},
        {0x49, 0x3C},
        {0x48, 0xD1},
        {0x49, 0x3E},
        {0x4A, 0x80},
        {0x4A, 0xC0},
        {0x4A, 0xE0},
        {0x4A, 0xF0},
        {0x4A, 0xF8},
        {0x4A, 0xFC},
        {0x4A, 0xFE},
        {0x4A, 0xFF},
        {0x03, 0xB5},
        {0x41, 0x80},
        {0x42, 0x42},
        {0x46, 0x02},
        {0x45, 0x98},
        {0x43, 0x0D},
        {0x44, 0x08},
        {0x41, 0xFF}
    };
    // setting ASIC(master 03)
    unsigned char asic_master_03[][2] =
    {
        {0x02, 0xE0},
        {0x03, 0xB4},
        {0x04, 0x58},
        {0x05, 0x00},
        {0x06, 0x96},
        {0x07, 0x7F},
        {0x08, 0x6D},
        {0x09, 0x31},
        {0x0A, 0x20},
        {0x0B, 0x7F},
        {0x0C, 0x00},
        {0x0D, 0xFE},
        {0x0E, 0x01},
        {0x0F, 0x20},
        {0x10, 0x00},
        {0x11, 0x00},
        {0x12, 0x07},
        {0x13, 0x65},
        {0x14, 0xB0},
        {0x15, 0xFF},
        {0x16, 0xA0},
        {0x17, 0x00},
        {0x18, 0x00},
        {0x19, 0x0A},
        {0x1A, 0x02},
        {0x1B, 0x00},
        {0x1C, 0x01},
        {0x1D, 0x06},
        {0x1E, 0x00},
        {0x1F, 0x00},
        {0x20, 0xC9},
        {0x21, 0x03},
        {0x22, 0x66},
        {0x23, 0x61},
        {0x24, 0x61},
        {0x25, 0x61},
        {0x26, 0x61},
        {0x27, 0x00},
        {0x28, 0x06},
        {0x29, 0x06},
        {0x2A, 0x06},
        {0x2B, 0x06},
        {0x2C, 0x00},
        {0x2D, 0x08},
        {0x2E, 0x02},
        {0x2F, 0x81},
        {0x30, 0xEE},
        {0x31, 0x04},
        {0x32, 0x03},
        {0x33, 0x31},
        {0x34, 0x20},
        {0x35, 0x98},
        {0x36, 0x05},
        {0x37, 0x00},
        {0x38, 0x04},
        {0x39, 0x04},
        {0x3A, 0x08},
        {0x3B, 0x28},
        {0x3C, 0x2C},
        {0x3D, 0x01},
        {0x3E, 0x0A}
    };
    // setting ASIC(slave 01)
    unsigned char asic_slave_01[][2] =
    {
        {0x4B, 0x25},
        {0x48, 0x80},
        {0x4D, 0x58},
        {0x48, 0xC0},
        {0x48, 0xD0},
        {0x5D, 0x00},
        {0x60, 0x00},
        {0x61, 0x00},
        {0x5E, 0x00},
        {0x5F, 0x00},
        {0x62, 0x00},
        {0x59, 0x0B},
        {0x5A, 0x18},
        {0x4E, 0x1E}
    };
    // setting ASIC(slave 02)
    unsigned char asic_slave_02[][2] =
    {
        {0x48, 0xD0},
        {0x49, 0x20},
        {0x49, 0x30},
        {0x49, 0x38},
        {0x49, 0x3C},
        {0x48, 0xD0},
        {0x49, 0x3C},
        {0x4A, 0x00},
        {0x4A, 0x00},
        {0x4A, 0x00},
        {0x4A, 0x00},
        {0x4A, 0x00},
        {0x4A, 0x00},
        {0x4A, 0x00},
        {0x4A, 0x00},
        {0x03, 0xB5},
        {0x41, 0x80},
        {0x42, 0x42},
        {0x46, 0x02},
        {0x45, 0x98},
        {0x43, 0x0E},
        {0x44, 0x08},
        {0x41, 0xFF}
    };
    // setting ASIC(slave 03)
    unsigned char asic_slave_03[][2] =
    {
        {0x02, 0xE0},
        {0x03, 0x90},
        {0x04, 0x58},
        {0x05, 0x00},
        {0x06, 0x96},
        {0x07, 0x7C},
        {0x08, 0x6D},
        {0x09, 0x31},
        {0x0A, 0x20},
        {0x0B, 0x6D},
        {0x0C, 0x00},
        {0x0D, 0xFE},
        {0x0E, 0x01},
        {0x0F, 0x20},
        {0x10, 0x00},
        {0x11, 0x00},
        {0x12, 0x07},
        {0x13, 0x65},
        {0x14, 0xB0},
        {0x15, 0xFF},
        {0x16, 0xA0},
        {0x17, 0x00},
        {0x18, 0x00},
        {0x19, 0x0A},
        {0x1A, 0x02},
        {0x1B, 0x00},
        {0x1C, 0x01},
        {0x1D, 0x06},
        {0x1E, 0x00},
        {0x1F, 0x00},
        {0x20, 0xC9},
        {0x21, 0x03},
        {0x22, 0x66},
        {0x23, 0x61},
        {0x24, 0x61},
        {0x25, 0x61},
        {0x26, 0x61},
        {0x27, 0x00},
        {0x28, 0x06},
        {0x29, 0x06},
        {0x2A, 0x06},
        {0x2B, 0x06},
        {0x2C, 0x00},
        {0x2D, 0x08},
        {0x2E, 0x02},
        {0x2F, 0x81},
        {0x30, 0xEE},
        {0x31, 0x04},
        {0x32, 0x03},
        {0x33, 0x31},
        {0x34, 0x20},
        {0x35, 0x98},
        {0x36, 0x05},
        {0x37, 0x00},
        {0x38, 0x04},
        {0x39, 0x04},
        {0x3A, 0x08},
        {0x3B, 0x28},
        {0x3C, 0x2C},
        {0x3D, 0x01},
        {0x3E, 0x0A},
        {0x4B, 0x25},
        {0x03, 0x90}
    };

    unsigned char asic_master_04[][2] =
    {
        {0x20, 0xC9},
        {0x21, 0x03},
        {0x22, 0x66},
        {0x23, 0x61},
        {0x24, 0x61},
        {0x25, 0x61},
        {0x26, 0x61},
        {0x2E, 0x02},
        {0x2F, 0x81},
        {0x30, 0xEE},
        {0x31, 0x04},
        {0x32, 0x03},
        {0x33, 0x31},
        {0x34, 0x20},
        {0x35, 0x98},
        {0x36, 0x05},
        {0x37, 0x00},
        {0x38, 0x04},
        {0x39, 0x04},
        {0x3A, 0x08},
        {0x3B, 0x28},
        {0x3C, 0x2C},
        {0x43, 0x0F},
        {0x44, 0x0F}
    };

    unsigned char asic_slave_04[][2] =
    {
        {0x20, 0xC9},
        {0x21, 0x03},
        {0x22, 0x66},
        {0x23, 0x61},
        {0x24, 0x61},
        {0x25, 0x61},
        {0x26, 0x61},
        {0x2E, 0x02},
        {0x2F, 0x81},
        {0x30, 0xEE},
        {0x31, 0x04},
        {0x32, 0x03},
        {0x33, 0x31},
        {0x34, 0x20},
        {0x35, 0x98},
        {0x36, 0x05},
        {0x37, 0x00},
        {0x38, 0x04},
        {0x39, 0x04},
        {0x3A, 0x08},
        {0x3B, 0x28},
        {0x3C, 0x2C},
        {0x43, 0x0F},
        {0x44, 0x0F}
    };
#elif ( _ASIC_FRAMERATE_ == _ASIC_12_0_FRAME_ )
    // setting ASIC(master 01)
    unsigned char asic_master_01[][2] =
    {
        {0x4B, 0x05},
        {0x48, 0x80},
        {0x4D, 0x58},
        {0x48, 0xC0},
        {0x48, 0xD0},
        {0x5D, 0x10},
        {0x60, 0x0B},
        {0x61, 0x25},
        {0x5E, 0x00},
        {0x5F, 0x00},
        {0x62, 0x00},
        {0x59, 0x0B},
        {0x5A, 0x18},
        {0x4E, 0x1E}
    };
    // setting ASIC(master 02)
    unsigned char asic_master_02[][2] =
    {
        {0x48, 0xD0},
        {0x49, 0x20},
        {0x49, 0x30},
        {0x49, 0x38},
        {0x49, 0x3C},
        {0x48, 0xD1},
        {0x49, 0x3E},
        {0x4A, 0x80},
        {0x4A, 0xC0},
        {0x4A, 0xE0},
        {0x4A, 0xF0},
        {0x4A, 0xF8},
        {0x4A, 0xFC},
        {0x4A, 0xFE},
        {0x4A, 0xFF},
        {0x03, 0xB5},
        {0x41, 0x80},
        {0x42, 0x42},
        {0x46, 0x02},
        {0x45, 0x98},
        {0x43, 0x0D},
        {0x44, 0x08},
        {0x41, 0xFF}
    };
    // setting ASIC(master 03)
    unsigned char asic_master_03[][2] =
    {
        {0x02, 0xE0},
        {0x03, 0xB4},
        {0x04, 0x58},
        {0x05, 0x00},
        {0x06, 0x96},
        {0x07, 0x7F},
        {0x08, 0x6D},
        {0x09, 0x31},
        {0x0A, 0x20},
        {0x0B, 0x7F},
        {0x0C, 0x00},
        {0x0D, 0xFE},
        {0x0E, 0x01},
        {0x0F, 0x20},
        {0x10, 0x00},
        {0x11, 0x00},
        {0x12, 0x07},
        {0x13, 0x65},
        {0x14, 0x83},
        {0x15, 0xFF},
        {0x16, 0xA0},
        {0x17, 0x00},
        {0x18, 0x00},
        {0x19, 0x0A},
        {0x1A, 0x02},
        {0x1B, 0x00},
        {0x1C, 0x01},
        {0x1D, 0x06},
        {0x1E, 0x00},
        {0x1F, 0x00},
        {0x20, 0xC9},
        {0x21, 0x03},
        {0x22, 0x52},
        {0x23, 0x61},
        {0x24, 0x61},
        {0x25, 0x61},
        {0x26, 0x61},
        {0x27, 0x00},
        {0x28, 0x06},
        {0x29, 0x06},
        {0x2A, 0x06},
        {0x2B, 0x06},
        {0x2C, 0x00},
        {0x2D, 0x08},
        {0x2E, 0x02},
        {0x2F, 0x81},
        {0x30, 0xEE},
        {0x31, 0x04},
        {0x32, 0x03},
        {0x33, 0x31},
        {0x34, 0x20},
        {0x35, 0x48},
        {0x36, 0x05},
        {0x37, 0x00},
        {0x38, 0x04},
        {0x39, 0x04},
        {0x3A, 0x08},
        {0x3B, 0x28},
        {0x3C, 0x2C},
        {0x3D, 0x01},
        {0x3E, 0x0A}
    };
    // setting ASIC(slave 01)
    unsigned char asic_slave_01[][2] =
    {
        {0x4B, 0x25},
        {0x48, 0x80},
        {0x4D, 0x58},
        {0x48, 0xC0},
        {0x48, 0xD0},
        {0x5D, 0x00},
        {0x60, 0x00},
        {0x61, 0x00},
        {0x5E, 0x00},
        {0x5F, 0x00},
        {0x62, 0x00},
        {0x59, 0x0B},
        {0x5A, 0x18},
        {0x4E, 0x1E}
    };
    // setting ASIC(slave 02)
    unsigned char asic_slave_02[][2] =
    {
        {0x48, 0xD0},
        {0x49, 0x20},
        {0x49, 0x30},
        {0x49, 0x38},
        {0x49, 0x3C},
        {0x48, 0xD0},
        {0x49, 0x3C},
        {0x4A, 0x00},
        {0x4A, 0x00},
        {0x4A, 0x00},
        {0x4A, 0x00},
        {0x4A, 0x00},
        {0x4A, 0x00},
        {0x4A, 0x00},
        {0x4A, 0x00},
        {0x03, 0xB5},
        {0x41, 0x80},
        {0x42, 0x42},
        {0x46, 0x02},
        {0x45, 0x98},
        {0x43, 0x0E},
        {0x44, 0x08},
        {0x41, 0xFF}
    };
    // setting ASIC(slave 03)
    unsigned char asic_slave_03[][2] =
    {
        {0x02, 0xE0},
        {0x03, 0x90},
        {0x04, 0x58},
        {0x05, 0x00},
        {0x06, 0x96},
        {0x07, 0x7C},
        {0x08, 0x6D},
        {0x09, 0x31},
        {0x0A, 0x20},
        {0x0B, 0x6D},
        {0x0C, 0x00},
        {0x0D, 0xFE},
        {0x0E, 0x01},
        {0x0F, 0x20},
        {0x10, 0x00},
        {0x11, 0x00},
        {0x12, 0x07},
        {0x13, 0x65},
        {0x14, 0x83},
        {0x15, 0xFF},
        {0x16, 0xA0},
        {0x17, 0x00},
        {0x18, 0x00},
        {0x19, 0x0A},
        {0x1A, 0x02},
        {0x1B, 0x00},
        {0x1C, 0x01},
        {0x1D, 0x06},
        {0x1E, 0x00},
        {0x1F, 0x00},
        {0x20, 0xC9},
        {0x21, 0x03},
        {0x22, 0x52},
        {0x23, 0x61},
        {0x24, 0x61},
        {0x25, 0x61},
        {0x26, 0x61},
        {0x27, 0x00},
        {0x28, 0x06},
        {0x29, 0x06},
        {0x2A, 0x06},
        {0x2B, 0x06},
        {0x2C, 0x00},
        {0x2D, 0x08},
        {0x2E, 0x02},
        {0x2F, 0x81},
        {0x30, 0xEE},
        {0x31, 0x04},
        {0x32, 0x03},
        {0x33, 0x31},
        {0x34, 0x20},
        {0x35, 0x48},
        {0x36, 0x05},
        {0x37, 0x00},
        {0x38, 0x04},
        {0x39, 0x04},
        {0x3A, 0x08},
        {0x3B, 0x28},
        {0x3C, 0x2C},
        {0x3D, 0x01},
        {0x3E, 0x0A},
        {0x4B, 0x25},
        {0x03, 0x90}
    };

    unsigned char asic_master_04[][2] =
    {
        {0x20, 0xC9},
        {0x21, 0x03},
        {0x22, 0x52},
        {0x23, 0x61},
        {0x24, 0x61},
        {0x25, 0x61},
        {0x26, 0x61},
        {0x2E, 0x02},
        {0x2F, 0x81},
        {0x30, 0xEE},
        {0x31, 0x04},
        {0x32, 0x03},
        {0x33, 0x31},
        {0x34, 0x20},
        {0x35, 0x48},
        {0x36, 0x05},
        {0x37, 0x00},
        {0x38, 0x04},
        {0x39, 0x04},
        {0x3A, 0x08},
        {0x3B, 0x28},
        {0x3C, 0x2C}
    };

    unsigned char asic_slave_04[][2] =
    {
        {0x20, 0xC9},
        {0x21, 0x03},
        {0x22, 0x52},
        {0x23, 0x61},
        {0x24, 0x61},
        {0x25, 0x61},
        {0x26, 0x61},
        {0x2E, 0x02},
        {0x2F, 0x81},
        {0x30, 0xEE},
        {0x31, 0x04},
        {0x32, 0x03},
        {0x33, 0x31},
        {0x34, 0x20},
        {0x35, 0x48},
        {0x36, 0x05},
        {0x37, 0x00},
        {0x38, 0x04},
        {0x39, 0x04},
        {0x3A, 0x08},
        {0x3B, 0x28},
        {0x3C, 0x2C}
    };
#endif

    // gon modify 2013-03-08 - Do not need
    unsigned char asic_last[][2] =
    {
        {0x12, 0x05},
        {0x14, 0x8C},
        // 2013-03-08 gon modify - SD change this value
        //									{0x5D, 0x0A},
        //									{0x61, 0x29},
        //									{0x4C, 0x9B},
        {0x5D, 0x10},
        //									{0x61, 0x25},
        //									{0x4C, 0x9B},
        {0x10, 0x80}
    };

    loop_count = sizeof(asic_master_01) / 2;
    for (i = 0; i < loop_count; i++)
    {
        ret = ASIC_SetRegister(ASIC_MASTER_ADDRESS, asic_master_01[i][0], asic_master_01[i][1]);
        if (ret != IBSU_STATUS_OK)
        {
            return ret;
        }
    }
    Sleep(10);

    loop_count = sizeof(asic_slave_01) / 2;
    for (i = 0; i < loop_count; i++)
    {
        ret = ASIC_SetRegister(ASIC_SLAVE_ADDRESS, asic_slave_01[i][0], asic_slave_01[i][1]);
        if (ret != IBSU_STATUS_OK)
        {
            return ret;
        }
    }
    Sleep(10);

    loop_count = sizeof(asic_master_02) / 2;
    for (i = 0; i < loop_count; i++)
    {
        ret = ASIC_SetRegister(ASIC_MASTER_ADDRESS, asic_master_02[i][0], asic_master_02[i][1]);
        if (ret != IBSU_STATUS_OK)
        {
            return ret;
        }
    }
    Sleep(10);

    loop_count = sizeof(asic_slave_02) / 2;
    for (i = 0; i < loop_count; i++)
    {
        ret = ASIC_SetRegister(ASIC_SLAVE_ADDRESS, asic_slave_02[i][0], asic_slave_02[i][1]);
        if (ret != IBSU_STATUS_OK)
        {
            return ret;
        }
    }
    Sleep(10);

    loop_count = sizeof(asic_master_03) / 2;
    for (i = 0; i < loop_count; i++)
    {
        ret = ASIC_SetRegister(ASIC_MASTER_ADDRESS, asic_master_03[i][0], asic_master_03[i][1]);
        if (ret != IBSU_STATUS_OK)
        {
            return ret;
        }
    }
    Sleep(10);

    loop_count = sizeof(asic_slave_03) / 2;
    for (i = 0; i < loop_count; i++)
    {
        ret = ASIC_SetRegister(ASIC_SLAVE_ADDRESS, asic_slave_03[i][0], asic_slave_03[i][1]);
        if (ret != IBSU_STATUS_OK)
        {
            return ret;
        }
    }
    Sleep(10);

    loop_count = sizeof(asic_master_04) / 2;
    for (i = 0; i < loop_count; i++)
    {
        ret = ASIC_SetRegister(ASIC_MASTER_ADDRESS, asic_master_04[i][0], asic_master_04[i][1]);
        if (ret != IBSU_STATUS_OK)
        {
            return ret;
        }
    }
    Sleep(10);

    loop_count = sizeof(asic_slave_04) / 2;
    for (i = 0; i < loop_count; i++)
    {
        ret = ASIC_SetRegister(ASIC_SLAVE_ADDRESS, asic_slave_04[i][0], asic_slave_04[i][1]);
        if (ret != IBSU_STATUS_OK)
        {
            return ret;
        }
    }
    Sleep(10);

    if (m_UsbDeviceInfo.bNewFPCB == TRUE)
    {
        ASIC_SetRegister(ASIC_SLAVE_ADDRESS, 0x03, 0x92);
    }

    loop_count = sizeof(asic_last) / 2;
    for (i = 0; i < loop_count; i++)
    {
        ret = ASIC_SetRegister(ASIC_MASTER_ADDRESS, asic_last[i][0], asic_last[i][1]);
        if (ret != IBSU_STATUS_OK)
        {
            return ret;
        }
    }
    Sleep(10);

//    m_MasterValue = 0x0F;
//    m_SlaveValue = 0x0F;

    /*	for(i=0; i<loop_count; i++)
    	{
    		ret = ASIC_SetRegister(ASIC_SLAVE_ADDRESS, asic_last[i][0], asic_last[i][1]);
    		if(ret != IBSU_STATUS_OK)
    			return ret;
    //		Sleep(1);
    	}
    */
    return IBSU_STATUS_OK;
}

int CMainCapture::_GoJob_DummyCapture_ForThread_Sherlock(BYTE *InImg)
{
    int			finger_count;

    m_pAlgo->_Algo_ROIC_AnalysisImage(InImg);
    m_pAlgo->_Algo_Remove_Noise_using_remove_mask(InImg, m_pAlgo->m_Inter_Img2, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
    m_pAlgo->_Algo_Remove_Vertical_Noise(m_pAlgo->m_Inter_Img2, m_pAlgo->m_Inter_Img, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
    m_pAlgo->m_cImgAnalysis.mean = m_pAlgo->_Algo_ROIC_GetBrightWithRawImage(m_pAlgo->m_Inter_Img, &m_pAlgo->m_cImgAnalysis.foreground_count, &m_pAlgo->m_cImgAnalysis.center_x, &m_pAlgo->m_cImgAnalysis.center_y);

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




int CMainCapture::ASIC_Analog_Power_On()
{
    return _UsbBulkOutIn(EP1OUT, CMD_ANALOG_POWER_ON, NULL, 0, -1, NULL, 0, NULL);
}

int CMainCapture::_ROIC_SetRegister(WORD adr, WORD val)
{
    UCHAR		outBuffer[MAX_DATA_BYTES + 1] = {0};

    outBuffer[0] = (UCHAR)val;

    return _UsbBulkOutIn(EP1OUT, CMD_WRITE_CIS_REG, outBuffer, 1, -1, NULL, 0, NULL);
}

void CMainCapture::_GoJob_Initialize_ForThread_Sherlock()
{
    int WaterImageSize = m_UsbDeviceInfo.nWaterImageSize;

//    BYTE *WaterImage = new BYTE[WaterImageSize + 4 /* +4 is checksum */];
    BYTE *WaterImage = m_pAlgo->m_WaterImage;

    memset(WaterImage, 0, WaterImageSize);
    if (m_UsbDeviceInfo.bNeedMask)
    {
        if (!_Algo_Sherlock_GetWaterMaskFromFlashMemory(WaterImage, WaterImageSize))
	    {
		    memset(WaterImage, 0, WaterImageSize);
	    }
    }

    m_pAlgo->_Algo_MakeUniformityMask(WaterImage);
//    delete [] WaterImage;

    m_pAlgo->_Algo_Init_GammaTable();

	if(m_UsbDeviceInfo.nFpgaVersion >= _FPGA_VER_FOR_SHERLOCK_DOUBLE_P_)
	{
		_PostTraceLogCallback(0, "=====> Detected Sherlock with DP");
	}

    //	_PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, 90);
}

BOOL CMainCapture::_Algo_Sherlock_GetWaterMaskFromFlashMemory(unsigned char *mask_buf, LONG mask_buf_size)
{
    LONG i, page_count;
    int remained_buf_size;
    LONG /*lSize, */lActualBytes, writecnt = 0;
    unsigned char packet_buf[10];
    unsigned char checksum, read_buf[UM_MAX_PAGE_SIZE];
    int	progressCount = 0, progressValue = 0;
    int voltageValue = 0;
    int nRc;
    BOOL isSameUM = FALSE;
	unsigned char *InImg = mask_buf;
    double bright[2];

    if (m_propertyInfo.bEnablePowerSaveMode)
    {
        _SndUsbFwCaptureStop();
        _SndUsbFwCaptureStart();
        _InitializeForCISRegister(m_UsbDeviceInfo.CisRowStart, m_UsbDeviceInfo.CisColStart, m_UsbDeviceInfo.CisImgWidth, m_UsbDeviceInfo.CisImgHeight);
    }

    if (m_UsbDeviceInfo.bNewFPCB && m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK)
    {
		while (_GetOneFrameImage(InImg, m_UsbDeviceInfo.CisImgSize))
		{
            TFT_MasterSlaveDeviation(InImg, bright);

            if (bright[0] < 20)
            {
                ASIC_SetRegister(ASIC_MASTER_ADDRESS, 0x43, ++m_MasterValue);
            }
            if (bright[1] < 20)
            {
                ASIC_SetRegister(ASIC_SLAVE_ADDRESS, 0x43, ++m_SlaveValue);
            }
            if (bright[0] > 20 && bright[1] > 20)
            {
                break;
            }
        }
    }

    Capture_SetLEVoltage(__DEFAULT_VOLTAGE_VALUE__);
    Capture_GetLEVoltage(&voltageValue);
    if (voltageValue == __DEFAULT_VOLTAGE_VALUE__)
    {
        m_UsbDeviceInfo.LeVoltageControl = SUPPORT_LE_VOLTAGE_CONTROL;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    m_UsbDeviceInfo.nFpgaVersion = _GetFpgaVersion();
    if (m_UsbDeviceInfo.devType == DEVICE_TYPE_SHERLOCK && m_UsbDeviceInfo.nFpgaVersion >= ((1 << 16) | (0 << 8) | 3)/*1.0.3*/)
    {
        int DefectMaskSize = m_UsbDeviceInfo.ImgSize;
        m_pAlgo->m_TFT_DefectMaskCount = 0;
        if (_ReadTFT_DefectMaskFromFlashMemory(m_pAlgo->m_TFT_DefectMaskImg, DefectMaskSize))
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
        }
        else
        {
            memset(m_pAlgo->m_TFT_DefectMaskImg, 0, DefectMaskSize);
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    page_count = mask_buf_size / (UM_ONE_PAGE_SIZE - 1);
    remained_buf_size = mask_buf_size - page_count * (UM_ONE_PAGE_SIZE - 1);

    if (m_bUniformityMaskPath)
    {
        FILE *fp;

        fp = fopen(m_cUniformityMaskPath, "rb");
        if (fp != NULL)
        {
            const size_t maskSize = mask_buf_size + 4; // 4 is the checksum size
            size_t       readSize;

            readSize = fread(mask_buf, 1, maskSize, fp);
            fclose(fp);

            if (readSize == maskSize)
            {
                m_pAlgo->_Algo_Decrypt_Simple(mask_buf, mask_buf, maskSize);
                if (m_pAlgo->_Algo_ConfirmChecksum((DWORD *)mask_buf, maskSize / sizeof(DWORD)))
                {
                    isSameUM = TRUE;
                }
            }
        }

        if (isSameUM)
        {
            // Read sampling mask from device
            for (int j = 0; j < 2; j++)
            {
                if (j == 0)
                {
                    memset(read_buf, 0, sizeof(read_buf));
                    if (_UM_CONT_READ_FLASH(read_buf, UM_MAX_PAGE_SIZE, 0, j) == FALSE)
                    {
                        isSameUM = FALSE;
                        break;
                    }

                    checksum = _UM_MAKE_CHECKSUM(read_buf, UM_ONE_PAGE_SIZE - 1);

                    if (checksum != read_buf[UM_ONE_PAGE_SIZE - 1])
                    {
                        isSameUM = FALSE;
                        break;
                    }

                    if (memcmp(&mask_buf[j * (UM_ONE_PAGE_SIZE - 1)], read_buf, (UM_ONE_PAGE_SIZE - 1)) != 0)
                    {
                        isSameUM = FALSE;
                        break;
                    }
                }
                else
                {
                    writecnt = 0;
                    packet_buf[writecnt++] = (j >> 6) & 0xFF;
                    packet_buf[writecnt++] = (unsigned char)(j << 2);
                    packet_buf[writecnt++] = 0;
                    packet_buf[writecnt++] = 0;
                    packet_buf[writecnt++] = 0;
                    packet_buf[writecnt++] = 0;
                    packet_buf[writecnt++] = 0;

                    // start bit set
                    _FPGA_SetRegister(0x24, 0x01);

                    nRc = _UsbBulkOutIn(EP2OUT, 0xE8, (UCHAR *)packet_buf, writecnt,
                                        EP8IN, (UCHAR *)read_buf, UM_MAX_PAGE_SIZE, &lActualBytes);
                    if (nRc != IBSU_STATUS_OK || lActualBytes != UM_MAX_PAGE_SIZE)
                    {
                        isSameUM = FALSE;
                        break;
                    }

                    checksum = _UM_MAKE_CHECKSUM(read_buf, UM_ONE_PAGE_SIZE - 1);

                    if (checksum != read_buf[UM_ONE_PAGE_SIZE - 1])
                    {
                        if (j != (page_count - 1))
                        {
                            isSameUM = FALSE;
                            break;
                        }
                    }

                    if (memcmp(&mask_buf[j * (UM_ONE_PAGE_SIZE - 1)], read_buf, (UM_ONE_PAGE_SIZE - 1)) != 0)
                    {
                        isSameUM = FALSE;
                        break;
                    }
                }
            }
        }
    }

    if (isSameUM)
    {
        if (m_propertyInfo.bEnablePowerSaveMode == TRUE)
        {
            _SndUsbFwCaptureStop();
        }

        return TRUE;
    }

    for (i = 0; i < page_count; i++)
    {
        if ((i % 200) == 0)
        {
            progressCount++;
            progressValue = 10 + (i * 80) / page_count;// / progressCount*7 + 10;
            _PostCallback(CALLBACK_INIT_PROGRESS, 0, 0, progressValue);
        }

        if (i == 0)
        {
            memset(read_buf, 0, sizeof(read_buf));
            if (_UM_CONT_READ_FLASH(read_buf, UM_MAX_PAGE_SIZE, 0, i) == FALSE)
            {
                goto done;
            }

            checksum = _UM_MAKE_CHECKSUM(read_buf, UM_ONE_PAGE_SIZE - 1);

            if (checksum != read_buf[UM_ONE_PAGE_SIZE - 1])
            {
                goto done;
            }

            memcpy(&mask_buf[i * (UM_ONE_PAGE_SIZE - 1)], read_buf, (UM_ONE_PAGE_SIZE - 1));
        }
        else
        {
            writecnt = 0;
            packet_buf[writecnt++] = (i >> 6) & 0xFF;
            packet_buf[writecnt++] = (unsigned char)(i << 2);
            packet_buf[writecnt++] = 0;
            packet_buf[writecnt++] = 0;
            packet_buf[writecnt++] = 0;
            packet_buf[writecnt++] = 0;
            packet_buf[writecnt++] = 0;

            // start bit set
            _FPGA_SetRegister(0x24, 0x01);

            nRc = _UsbBulkOutIn(EP2OUT, 0xE8, (UCHAR *)packet_buf, writecnt,
                                EP8IN, (UCHAR *)read_buf, UM_MAX_PAGE_SIZE, &lActualBytes);
            if (nRc != IBSU_STATUS_OK || lActualBytes != UM_MAX_PAGE_SIZE)
            {
                goto done;
            }

            checksum = _UM_MAKE_CHECKSUM(read_buf, UM_ONE_PAGE_SIZE - 1);

            if (checksum != read_buf[UM_ONE_PAGE_SIZE - 1])
            {
                if (i != (page_count - 1))
                {
                    goto done;
                }
            }

            memcpy(&mask_buf[i * (UM_ONE_PAGE_SIZE - 1)], read_buf, (UM_ONE_PAGE_SIZE - 1));
        }
    }

    if (remained_buf_size > 0)
    {
        writecnt = 0;
        packet_buf[writecnt++] = (i >> 6) & 0xFF;
        packet_buf[writecnt++] = (unsigned char)(i << 2);
        packet_buf[writecnt++] = 0;
        packet_buf[writecnt++] = 0;
        packet_buf[writecnt++] = 0;
        packet_buf[writecnt++] = 0;
        packet_buf[writecnt++] = 0;

        // start bit set
        _FPGA_SetRegister(0x24, 0x01);

        nRc = _UsbBulkOutIn(EP2OUT, 0xE8, (UCHAR *)packet_buf, writecnt, EP8IN, (UCHAR *)&mask_buf[i * UM_ONE_PAGE_SIZE], UM_MAX_PAGE_SIZE, &lActualBytes);
        if (nRc != IBSU_STATUS_OK || lActualBytes != UM_MAX_PAGE_SIZE)
        {
            goto done;
        }

        checksum = _UM_MAKE_CHECKSUM(read_buf, remained_buf_size);

        if (checksum != read_buf[remained_buf_size])
        {
            goto done;
        }

        memcpy(&mask_buf[i * (UM_ONE_PAGE_SIZE - 1)], read_buf, remained_buf_size);
    }

    if (m_bUniformityMaskPath)
    {
        FILE *fp;
        fp = fopen(m_cUniformityMaskPath, "wb");
        if (fp != NULL)
        {
            unsigned char *tmpMask = new unsigned char [mask_buf_size + 4];
            memcpy(tmpMask, mask_buf, mask_buf_size);
            m_pAlgo->_Algo_MakeChecksum((DWORD *)tmpMask, (mask_buf_size + 4) / sizeof(DWORD));
            m_pAlgo->_Algo_Encrypt_Simple(tmpMask, tmpMask, mask_buf_size + 4);
            fwrite(tmpMask, mask_buf_size + 4, 1, fp);
            fclose(fp);
            delete [] tmpMask;
        }
    }
    if (m_propertyInfo.bEnablePowerSaveMode == TRUE)
    {
        _SndUsbFwCaptureStop();
    }

    return TRUE;


done:
    if (m_propertyInfo.bEnablePowerSaveMode == TRUE)
    {
        _SndUsbFwCaptureStop();
    }

    return FALSE;
}

BOOL CMainCapture::_ReadTFT_DefectMaskFromFlashMemory(unsigned char *mask_buf, long mask_buf_size)
{
    if (mask_buf == NULL)
    {
        return FALSE;
    }

    LONG i, page_count;
    int remained_buf_size;
    LONG lActualBytes, writecnt;
    unsigned char packet_buf[10];
    unsigned char checksum, read_buf[UM_MAX_PAGE_SIZE];
    unsigned char temp_compressedBuf[(800 * 750) / 8 + 1024];
    int nRc;
    int compressed_buf_size = 75117;

    page_count = compressed_buf_size / (UM_ONE_PAGE_SIZE - 1);
    remained_buf_size = compressed_buf_size - page_count * (UM_ONE_PAGE_SIZE - 1);
    memset(temp_compressedBuf, 0, sizeof(temp_compressedBuf));

    for (i = 0; i < page_count; i++)
    {
        if (i == 0)
        {
            memset(read_buf, 0, sizeof(read_buf));

            if (_UM_CONT_READ_FLASH(read_buf, UM_MAX_PAGE_SIZE, 0, i + 1175) == FALSE)
            {
                goto done;
            }

            checksum = _UM_MAKE_CHECKSUM(read_buf, UM_ONE_PAGE_SIZE - 1);

            if (checksum != read_buf[UM_ONE_PAGE_SIZE - 1])
            {
                goto done;
            }

            memcpy(&temp_compressedBuf[i * (UM_ONE_PAGE_SIZE - 1)], read_buf, (UM_ONE_PAGE_SIZE - 1));
        }
        else
        {
            writecnt = 0;
            //packet_buf[writecnt++] = (i>>6)&0xFF;
            packet_buf[writecnt++] = ((i + 1175) >> 6) & 0xFF;
            //packet_buf[writecnt++] = (unsigned char)(i<<2);
            packet_buf[writecnt++] = (unsigned char)((i + 1175) << 2);
            packet_buf[writecnt++] = 0;
            packet_buf[writecnt++] = 0;
            packet_buf[writecnt++] = 0;
            packet_buf[writecnt++] = 0;
            packet_buf[writecnt++] = 0;

            // start bit set
            _FPGA_SetRegister(0x24, 0x01);

            nRc = _UsbBulkOutIn(EP2OUT, 0xE8, (UCHAR *)packet_buf, writecnt,
                                EP8IN, (UCHAR *)read_buf, UM_MAX_PAGE_SIZE, &lActualBytes);
            if (nRc != IBSU_STATUS_OK || lActualBytes != UM_MAX_PAGE_SIZE)
            {
                goto done;
            }

            checksum = _UM_MAKE_CHECKSUM(read_buf, UM_ONE_PAGE_SIZE - 1);

            if (checksum != read_buf[UM_ONE_PAGE_SIZE - 1])
            {
                if (i != (page_count - 1))
                {
                    goto done;
                }
            }

            //memcpy(&mask_buf[i*(UM_ONE_PAGE_SIZE-1)], read_buf, (UM_ONE_PAGE_SIZE-1));
            memcpy(&temp_compressedBuf[i * (UM_ONE_PAGE_SIZE - 1)], read_buf, (UM_ONE_PAGE_SIZE - 1));
        }
    }

    if (remained_buf_size > 0)
    {
        writecnt = 0;
        //	packet_buf[writecnt++] = (i>>6)&0xFF;
        packet_buf[writecnt++] = ((i + 1175) >> 6) & 0xFF;
        //packet_buf[writecnt++] = (unsigned char)(i<<2);
        packet_buf[writecnt++] = (unsigned char)((i + 1175) << 2);
        packet_buf[writecnt++] = 0;
        packet_buf[writecnt++] = 0;
        packet_buf[writecnt++] = 0;
        packet_buf[writecnt++] = 0;
        packet_buf[writecnt++] = 0;

        // start bit set
        _FPGA_SetRegister(0x24, 0x01);

        nRc = _UsbBulkOutIn(EP2OUT, 0xE8, (UCHAR *)packet_buf, writecnt, EP8IN, (UCHAR *)&mask_buf[i * UM_ONE_PAGE_SIZE], UM_MAX_PAGE_SIZE, &lActualBytes);
        if (nRc != IBSU_STATUS_OK || lActualBytes != UM_MAX_PAGE_SIZE)
        {
            goto done;
        }

        checksum = _UM_MAKE_CHECKSUM(read_buf, remained_buf_size);

        if (checksum != read_buf[remained_buf_size])
        {
            goto done;
        }

        //memcpy(&mask_buf[i*(UM_ONE_PAGE_SIZE-1)], read_buf, remained_buf_size);
        memcpy(&temp_compressedBuf[i * (UM_ONE_PAGE_SIZE - 1)], read_buf, remained_buf_size);
    }

    _TFT_DefectMaskDecompress(temp_compressedBuf, mask_buf, mask_buf_size);

    return TRUE;

done:
    return FALSE;
}

BOOL CMainCapture::_TFT_DefectMaskDecompress(unsigned char *compress_buf, unsigned char *mask_buf, int maskBufSize)
{
    if (mask_buf == NULL || compress_buf == NULL || (maskBufSize % 8) != 0)
    {
        return FALSE;
    }

    BYTE value;
    int compress_buf_len = maskBufSize / 8;

    for (int i = 0; i < compress_buf_len; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            value = (compress_buf[i] >> (7 - j)) & 0x01;
            if (value == 0x01)
            {
                mask_buf[i * 8 + j] = 0xFF;
            }
            else
            {
                mask_buf[i * 8 + j] = 0x00;
            }
        }
    }

    return TRUE;
}

BOOL CMainCapture::_TFT_DefectMaskCompress(unsigned char *mask_buf, unsigned char *compress_buf, int maskBufSize)
{
    if (mask_buf == NULL || compress_buf == NULL || (maskBufSize % 8) != 0)
    {
        return FALSE;
    }

	int compress_buf_len = maskBufSize / 8;

	for( int i=0; i<compress_buf_len; i++ )
	{
		compress_buf[i] = 0;
		for( int j=0; j<8; j++ )
		{
			compress_buf[i] += (mask_buf[i*8+j] & 0x01) << (7-j);
		}
	}

    return TRUE;
}

int	CMainCapture::Reserved_GetEnhancdImage_Sherlock(const IBSU_ImageData inImage,
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

#if defined(__embedded__)  || defined(WINCE)
	if (m_UsbDeviceInfo.nFpgaVersion >= ((1 << 16) | (0 << 8) | 3)/*1.0.3*/)
	{
		memcpy(Inter_Img2, (unsigned char*)inImage.Buffer, m_UsbDeviceInfo.ImgSize);
		m_pAlgo->_Algo_RemoveTFTDefect(Inter_Img2, Inter_Img1, m_pAlgo->m_TFT_DefectMaskImg);
	}
	else
		memcpy(Inter_Img1, (unsigned char*)inImage.Buffer, m_UsbDeviceInfo.ImgSize);
#else
	memcpy(Inter_Img1, (unsigned char*)inImage.Buffer, m_UsbDeviceInfo.ImgSize);
#endif

	switch (inImage.ProcessThres)
	{
	case IMAGE_PROCESS_THRES_LOW:
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(Inter_Img1, Inter_Img2, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);
		m_pAlgo->_Algo_ROIC_GetBrightWithRawImageEnhanced(Inter_Img2, &imgAnalysis);
		m_pAlgo->_Algo_VignettingEffectEnhanced(Inter_Img2, Inter_Img1, imgAnalysis);
		break;
	case IMAGE_PROCESS_THRES_MEDIUM:
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(Inter_Img1, Inter_Img1, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);
		m_pAlgo->_Algo_ROIC_GetBrightWithRawImageEnhanced(Inter_Img1, &imgAnalysis);
		break;
	case IMAGE_PROCESS_THRES_HIGH:
		m_pAlgo->_Algo_ConvertBkColorBlacktoWhite(Inter_Img1, Inter_Img1, m_UsbDeviceInfo.ImgWidth*m_UsbDeviceInfo.ImgHeight);
		m_pAlgo->_Algo_ROIC_GetBrightWithRawImageEnhanced(Inter_Img1, &imgAnalysis);
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

int CMainCapture::_IsNeedInitializeCIS_Sherlock()
{
	int img_size=0;
	UCHAR reg_val[4];
	if (_FPGA_GetRegister(0x07, &reg_val[0]) != IBSU_STATUS_OK) goto done;		// frame_size
    if (_FPGA_GetRegister(0x08, &reg_val[1]) != IBSU_STATUS_OK) goto done;
    if (_FPGA_GetRegister(0x09, &reg_val[2]) != IBSU_STATUS_OK) goto done;

	img_size = (reg_val[2]<<16) + (reg_val[1]<<8) + (reg_val[0]);

	if( img_size != m_UsbDeviceInfo.CisImgSize )
		return TRUE;
done:
	return FALSE;
}

