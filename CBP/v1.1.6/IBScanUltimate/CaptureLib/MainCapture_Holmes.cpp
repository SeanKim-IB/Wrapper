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

BOOL CMainCapture::_GetOneFrameImage_Holmes(unsigned char *Image, int ImgSize)
{
    return FALSE;
}

int CMainCapture::_InitializeForCISRegister_Holmes(WORD rowStart, WORD colStart, WORD width, WORD height)
{
    return IBSU_STATUS_OK;
}

int	CMainCapture::Capture_SetLEVoltage_Holmes(int voltageValue)
{
    return IBSU_STATUS_OK;
}

int	CMainCapture::Capture_GetLEVoltage_Holmes(int *voltageValue)
{
    return IBSU_STATUS_OK;
}

int CMainCapture::_SetLEOperationMode_Holmes(WORD addr)
{
    return IBSU_STATUS_OK;
}

AcuisitionState CMainCapture::_GoJob_PreImageProcessing_Holmes(BYTE *InImg, BYTE *OutImg, BOOL *bIsGoodImage)
{
    *bIsGoodImage = FALSE;

    memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize);
    return ACUISITION_NONE;
}

void CMainCapture::_PostImageProcessing_ForPreview_Holmes(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage)
{
    memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize);
    return;
}

void CMainCapture::_PostImageProcessing_ForResult_Holmes(BYTE *InImg, BYTE *OutImg)
{
    memcpy(OutImg, InImg, m_UsbDeviceInfo.CisImgSize);
    return;
}

int CMainCapture::_GoJob_DummyCapture_ForThread_Holmes(BYTE *InImg)
{
    return 0;
}

void CMainCapture::_GoJob_Initialize_ForThread_Holmes()
{
}

BOOL CMainCapture::_GoJob_PostImageProcessing_Holmes(BYTE *InImg, BYTE *OutImg, BOOL bIsGoodImage)
{
    return FALSE;
}

int CMainCapture::_IsNeedInitializeCIS_Holmes()
{
	return FALSE;
}
