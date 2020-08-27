/* *************************************************************************************************
 * IBAlgorithm.cpp
 *
 * DESCRIPTION:
 *     Main implementation file for CIBSMAlgorithm class for IBScanMatcher library
 *     http://www.integratedbiometrics.com
 *
 * NOTES:
 *     Copyright (c) Integrated Biometrics, 2013
 *
 * HISTORY:
 ************************************************************************************************ */
#include "stdafx.h"
#include "IBSMAlgorithm.h"
#include "formatching_table.lut"
#include "cycle_table.lut"
#include "atan2_table.lut"
#include "IB_TrigonometryTable.lut"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#ifdef __linux__
#include <sys/time.h>
#endif


CIBSMAlgorithm::CIBSMAlgorithm()
{
#ifdef _WINDOWS
    TCHAR MyName[MAX_PATH];
    GetModuleFileName(NULL, MyName, MAX_PATH);
    CString FileVersion=_T("");
    DWORD infoSize = 0;
    char *buffer;

    infoSize = GetFileVersionInfoSize(MyName, 0);
    if(infoSize>0)
    {
        buffer = new char[infoSize];
        if(buffer)
        {
            if(GetFileVersionInfo(MyName,0,infoSize, buffer)!=0)
            {
                VS_FIXEDFILEINFO* pFineInfo = NULL;
                UINT bufLen = 0;

                if(VerQueryValue(buffer,_T("\\"),(LPVOID*)&pFineInfo, &bufLen) !=0)
                {
                    WORD majorVer, minorVer, buildNum, revisionNum;
                    majorVer = HIWORD(pFineInfo->dwFileVersionMS);
                    minorVer = LOWORD(pFineInfo->dwFileVersionMS);
                    buildNum = HIWORD(pFineInfo->dwFileVersionLS);
                    revisionNum = LOWORD(pFineInfo->dwFileVersionLS);

                    sprintf(SDKVersion, "%d.%d.%d.%d", majorVer, minorVer, buildNum, revisionNum);
                }
            }
            delete[] buffer;
        }
    }

    if(strlen(SDKVersion)<4)
        sprintf(SDKVersion, "0.0.0.0");
#else
    strcpy(SDKVersion, "0.0.0.0");
#endif

    int imgEnWidth = MAX_IMAGE_W;
    int imgEnHeight = MAX_IMAGE_H;
    int imgEnHalfWidth = imgEnWidth>>1;
    int imgEnHalfHeight = imgEnHeight>>1;
    int imgEnQuadWidth = imgEnHalfWidth>>1;
    int imgEnQuadHeight = imgEnHalfHeight>>1;
    int imgEnOctWidth = imgEnQuadWidth>>1;
    int imgEnOctHeight = imgEnQuadHeight>>1;
    int imgEnHalfOctWidth = imgEnOctWidth>>1;
    int imgEnHalfOctHeight = imgEnOctHeight>>1;

    m_pEnlarge = new unsigned char [imgEnWidth*imgEnHeight];
    m_pSegBuf = new unsigned char [imgEnQuadWidth*imgEnQuadHeight];
    m_pDirBuf = new unsigned char [imgEnQuadWidth*imgEnQuadHeight];
    m_pDirBuf2 = new unsigned char [imgEnQuadWidth*imgEnQuadHeight];
    m_pDirBuf3 = new unsigned char [imgEnQuadWidth*imgEnQuadHeight];
    m_pFrequencyBuf = new unsigned char [imgEnHalfOctWidth*imgEnHalfOctHeight];
    m_pGabor = new int [imgEnWidth*imgEnHeight];
    m_pBinary = new unsigned char [imgEnWidth*imgEnHeight];
    m_pBinaryInv = new unsigned char [imgEnWidth*imgEnHeight];
    m_pThreeBin = new unsigned char [imgEnQuadWidth*imgEnQuadHeight];
    m_pLabeledBin = new unsigned char [imgEnQuadWidth*imgEnQuadHeight];
    m_pSingularBin = new unsigned char [imgEnQuadWidth*imgEnQuadHeight];
    m_pTracingBin = new unsigned char [imgEnQuadWidth*imgEnQuadHeight];
    m_pStackBuf = new unsigned short [imgEnQuadWidth*imgEnQuadHeight*8];
    m_pFeature1 = new unsigned int [MAX_MINUTIAE*8+1];
    m_pFeature2 = new unsigned int [MAX_MINUTIAE*8+1];
    m_pTempDirBuf = new unsigned char [imgEnQuadWidth*imgEnQuadHeight];
    m_X_gradient = new int [imgEnOctWidth*imgEnOctHeight];
    m_Y_gradient = new int [imgEnOctWidth*imgEnOctHeight];
    m_pHalfImage = new unsigned char [imgEnHalfWidth*imgEnHalfHeight];
    m_pGradientBuffer = new unsigned int [imgEnHalfWidth*imgEnHalfHeight];
    m_pTempQuadBuffer = new unsigned char [imgEnHalfWidth*imgEnHalfHeight];
    m_pTempImage = new unsigned char [imgEnWidth*imgEnHeight];
    m_pRemoveArray = new unsigned int [imgEnWidth*imgEnHeight];
    m_pGaborSave = new unsigned char [imgEnWidth*imgEnHeight];
    m_pBinarySave = new unsigned char [imgEnWidth*imgEnHeight];
    m_pThinSave = new unsigned char [imgEnWidth*imgEnHeight];

    _Algo_SetMatchingLevel(4);

    m_MatchCoreLevel = 3;
    m_IndexingLevel = 1;
}

CIBSMAlgorithm::~CIBSMAlgorithm()
{
    delete [] m_pEnlarge;
    delete [] m_pSegBuf;
    delete [] m_pDirBuf;
    delete [] m_pDirBuf2;
    delete [] m_pDirBuf3;
    delete [] m_pFrequencyBuf;
    delete [] m_pGabor;
    delete [] m_pBinary;
    delete [] m_pBinaryInv;
    delete [] m_pThreeBin;
    delete [] m_pLabeledBin;
    delete [] m_pSingularBin;
    delete [] m_pTracingBin;
    delete [] m_pStackBuf;
    delete [] m_pFeature1;
    delete [] m_pFeature2;
    delete [] m_pTempDirBuf;
    delete [] m_X_gradient;
    delete [] m_Y_gradient;
    delete [] m_pHalfImage;
    delete [] m_pGradientBuffer;
    delete [] m_pTempQuadBuffer;
    delete [] m_pTempImage;
    delete [] m_pRemoveArray;
    delete [] m_pGaborSave;
    delete [] m_pBinarySave;
    delete [] m_pThinSave;

/*	std::vector<AllocatedMemory *>::iterator it2 = m_pListAllocatedMemory.begin();
	while( it2 != m_pListAllocatedMemory.end() )
	{
		if( *it2 )
		{
			if( (*it2)->memblock )
			{
				delete (unsigned char*)(*it2)->memblock;
				(*it2)->memblock = NULL;
			}
			delete *it2;
			it2 = m_pListAllocatedMemory.erase(it2);
		}
		else
			++it2;
	}

	m_pListAllocatedMemory.clear();*/
}

void CIBSMAlgorithm::IBSM_SDKInitialize()
{
    memcpy(m_Atan2Table, _atan2_table, sizeof(m_Atan2Table));
    memcpy(m_SingleSinTable, _single_sin_table, sizeof(m_SingleSinTable));
    memcpy(m_DoubleSinTable, _double_sin_table, sizeof(m_DoubleSinTable));
    memcpy(m_SingleCosTable, _single_cos_table, sizeof(m_SingleCosTable));
    memcpy(m_DoubleCosTable, _double_cos_table, sizeof(m_DoubleCosTable));
    memcpy(m_SingleTanTable, _single_tan_table, sizeof(m_SingleTanTable));
    memcpy(m_xIncycleTable, _x_incycle_table, sizeof(m_xIncycleTable));
    memcpy(m_yIncycleTable, _y_incycle_table, sizeof(m_yIncycleTable));
    memcpy(m_xOutcycleTable, _x_outcycle_table, sizeof(m_xOutcycleTable));
    memcpy(m_yOutcycleTable, _y_outcycle_table, sizeof(m_yOutcycleTable));
    memcpy(m_DiffAngleForMatching, _DIFFANGLE_FORMATCHING, sizeof(m_DiffAngleForMatching));

    int i, j;
    for(i=0; i<MAX_IMAGE_H; i++)
    {
        for(j=0; j<MAX_IMAGE_W; j++)
        {
            m_SqrtTable[i][j] = (unsigned short)sqrt((double)i*i + j*j);
        }
    }
//  memcpy(RP_ANGLE_FORMATCHING, _RP_ANGLE_FORMATCHING, sizeof(RP_ANGLE_FORMATCHING));
//  memcpy(RP_R_FORMATCHING, _RP_R_FORMATCHING, sizeof(RP_R_FORMATCHING));
//  memcpy(RLIM_FORMATCHING, _RLIM_FORMATCHING, sizeof(RLIM_FORMATCHING));
//  memcpy(ANGLELIM_FORMATCHING, _ANGLELIM_FORMATCHING, sizeof(ANGLELIM_FORMATCHING));

    _ALGO_AlgorithmInit();
    _ALGO_MatchingInit();
}





int CIBSMAlgorithm::IBSM_ExtractTemplate(const unsigned char *pBuf, unsigned char *pTemplate, int imgWidth, int imgHeight)
{
    if(_ALGO_ExtractFt(pBuf, pTemplate, imgWidth, imgHeight) == 0)
        return IBSU_ERR_DUPLICATE_EXTRACTION_FAILED;

    m_SaveSizeX = imgWidth;
    m_SaveSizeY = imgHeight;

//  memcpy(pTemplate, pFeature1, (MAX_MINUTIAE+1)*sizeof(unsigned int));

    return IBSU_STATUS_OK;
}

int CIBSMAlgorithm::IBSM_GetProcessedImage(IBSM_ProcessedImageType processed_imagetype, IBSM_ImageData *out_image)
{
    switch(processed_imagetype)
    {
        case ENUM_IBSM_PROCESSED_IMAGE_TYPE_GABOR:
            out_image->ImageData = m_pGaborSave;
            break;
        case ENUM_IBSM_PROCESSED_IMAGE_TYPE_BINARY:
            out_image->ImageData = m_pBinarySave;
            break;
        case ENUM_IBSM_PROCESSED_IMAGE_TYPE_THIN:
            out_image->ImageData = m_pThinSave;
            break;
        default:
            out_image->ImageData = NULL;
            return IBSU_ERR_INVALID_PARAM_VALUE;
    }

    out_image->ImageFormat = IBSM_IMG_FORMAT_NO_BIT_PACKING;
    out_image->ImpressionType = IBSM_IMPRESSION_TYPE_UNKNOWN;
    out_image->FingerPosition = IBSM_FINGER_POSITION_UNKNOWN;
    out_image->CaptureDeviceTechID = IBSM_CAPTURE_DEVICE_UNKNOWN_OR_UNSPECIFIED;
    out_image->CaptureDeviceVendorID = IBSM_CAPTURE_DEVICE_VENDOR_ID_UNREPORTED;
    out_image->CaptureDeviceTypeID = IBSM_CAPTURE_DEVICE_TYPE_ID_UNKNOWN;
    out_image->ScanSamplingX = 500;
    out_image->ScanSamplingY = 500;
    out_image->ImageSamplingX = 500;
    out_image->ImageSamplingY = 500;
    out_image->ImageSizeX = (unsigned short)m_SaveSizeX;
    out_image->ImageSizeY = (unsigned short)m_SaveSizeY;
    out_image->ScaleUnit = IBSM_SCALE_UNIT_INCH;
    out_image->BitDepth = 8;
    out_image->ImageDataLength = out_image->ImageSizeX*out_image->ImageSizeY;

    return IBSU_STATUS_OK;
}

int CIBSMAlgorithm::IBSM_MatchingTemplate(const unsigned char *pTemplate1, const unsigned char *pTemplate2, int *matchingscore)
{
    FeatureVector   feavector_db;
    FeatureVector   feavector_in;

    if(((unsigned int)(pTemplate1[0]))<9 || ((unsigned int)(pTemplate2[0]))<9)
    {
        *matchingscore = 0;
		return IBSU_STATUS_OK;
    }

    _FeaGen((unsigned int*)pTemplate1, &feavector_db, 3);
    _FeaGen((unsigned int*)pTemplate2, &feavector_in, 3);

#ifdef __ROC_TEST__
    *matchingscore = _ALGO_Matching(&feavector_db, &feavector_in);
    if(*matchingscore < 0)
        *matchingscore = 0;
#else
/*  if( _JudgeFingerPattern(&feavector_db, &feavector_in) == 0 )
    {
        *matchingscore = 0;
    }
    else
*/  {
        *matchingscore = _ALGO_Matching(&feavector_db, &feavector_in);
        if(*matchingscore < m_MatchingThreshold)
            *matchingscore = 0;
    }
#endif

    return IBSU_STATUS_OK;
}

int CIBSMAlgorithm::IBSM_SetMatchingLevel(const int matching_level)
{
    int nRc = _Algo_SetMatchingLevel(matching_level);

    if(nRc == -1)
        return IBSU_ERR_INVALID_PARAM_VALUE;

    return IBSU_STATUS_OK;
}

int CIBSMAlgorithm::IBSM_GetMatchingLevel(int *matching_level)
{
    *matching_level = _Algo_GetMatchingLevel();

    return IBSU_STATUS_OK;
}

short CIBSMAlgorithm::IBSM_IsTemplate_317(const unsigned char *feature404)
{
    unsigned int i, j, position;
    int tetta, row1, row2, column1, column2;
    unsigned int *pintfea = (unsigned int*)feature404;

    if( pintfea[0] < 9 || pintfea[0] > 100 )
        return -1;

    for( i=1; i<=pintfea[0]; i++ )
    {
        position = pintfea[i];

        tetta = (position>>18)&0x1FF;
        if (tetta < 0 || tetta > 360)           // 이전 버전의 버그문제로 360도까지 허용함.
            return 0;

        column1 = (position>>9)&0x1FF;
        if( column1 < 0 || column1 >= 352 )
            return 0;

        row1 = (position)&0x1FF;
        if( row1 < 0 || row1 >= 288 )
            return 0;

        for( j=i+1; j<=pintfea[0]; j++ )
        {
            position = pintfea[j];
            row2 = (position)&0x1FF;
            column2 = (position>>9)&0x1FF;
            if( row1 == row2 && column1 == column2 )
                return 0;       // Invalid Template
        }
    }

    return 1;
}

short CIBSMAlgorithm::IBSM_IsTemplate_320(const unsigned char *feature404)
{
    unsigned int i, j, k, position, remaind_size, neighbor_num, neighbor_num_arr[D_NEIGHBOR];
    int tetta, row1, row2, column1, column2;
    unsigned int *pintfea = (unsigned int*)feature404;

    if( pintfea[0] < 9 || pintfea[0] > MAX_MINUTIAE )
        return -1;

    for (i=1; i<=pintfea[0]; i++)
    {
        position = pintfea[i];

        column1 = (position>>18)&0x1FF;
        if (column1 < 0 || column1 >= 352)
            return 0;

        row1 = (position>>9)&0x1FF;
        if (row1 < 0 || row1 >= 288)
            return 0;

        tetta = (position)&0x1FF;
        if (tetta < 0 || tetta > 360)           // 이전 버전의 버그문제로 360도까지 허용함.
            return 0;

        for (j=i+1; j<=pintfea[0]; j++)
        {
            position = pintfea[j];
            row2 = (position>>9)&0x1FF;
            column2 = (position>>18)&0x1FF;
            if (row1 == row2 && column1 == column2)
                return 0;       // Invalid Template
        }
    }
    remaind_size = ((MAX_MINUTIAE_FOR_IBISDK-pintfea[0])<<2)>>3;
    if( remaind_size > pintfea[0] )
        remaind_size = pintfea[0];

    for( i=0; i<remaind_size; i++ )
    {
        for( j=0; j<D_NEIGHBOR; j++ )
        {
            neighbor_num = feature404[(pintfea[0]+1)*4+i*8+j];
            neighbor_num_arr[j] = neighbor_num;

            if( neighbor_num > pintfea[0] )
                return 0;
        }

        for( j=0; j<D_NEIGHBOR-1; j++ )
        {
            for( k=j+1; k<D_NEIGHBOR; k++ )
            {
                if( neighbor_num_arr[j] == neighbor_num_arr[k] )
                {
                    return 0;
                }
            }
        }
    }

    return 1;
}

int CIBSMAlgorithm::_OpenBitmapImage(const char *filename, unsigned char **pBuf, IB_BITMAPINFO *pInfo)
{
	size_t readSize;

    FILE *fp = fopen(filename, "rb");
    if(fp == NULL)
        return 0;

    IB_BITMAPFILEHEADER header;
    IB_BITMAPINFOHEADER info;

    readSize = fread(&header, 1, sizeof(IB_BITMAPFILEHEADER), fp);

    if(readSize != (size_t)sizeof(IB_BITMAPFILEHEADER) || header.bfType != 19778)
    {
        fclose(fp);
        return 0;
    }

    readSize = fread(&info, 1, sizeof(IB_BITMAPINFOHEADER), fp);
    fseek(fp, 1024, SEEK_CUR);

    if(readSize != (size_t)sizeof(IB_BITMAPINFOHEADER) || info.biBitCount != 8)
    {
        fclose(fp);
        return 0;
    }

    memcpy(&pInfo->bmiHeader, &info, sizeof(IB_BITMAPINFOHEADER));

    if(*pBuf != NULL)
        delete [] *pBuf;

    *pBuf = new unsigned char [info.biWidth*info.biHeight];

    readSize = fread(*pBuf, 1, info.biWidth*info.biHeight, fp);

    if(readSize != (size_t)(info.biWidth*info.biHeight))
    {
        fclose(fp);
        return 0;
    }

    fclose(fp);

    return 1;
}

int CIBSMAlgorithm::_SaveBitmapImage(const char *filename, const unsigned char *pBuf, const int imgWidth, const int imgHeight, const BOOL Invert)
{
    FILE *fp = fopen(filename, "wb");
    if(fp == NULL)
        return 0;

    int i;
    IB_BITMAPFILEHEADER header;
    IB_BITMAPINFO *Info = (IB_BITMAPINFO *)new unsigned char [1064];

    header.bfOffBits = 1078;
    header.bfReserved1 = 0;
    header.bfReserved2 = 0;
    header.bfSize = 1078 + imgWidth*imgHeight;
    header.bfType = (unsigned short)(('M'<<8) | 'B');

    for(i=0; i<256; i++)
    {
        Info->bmiColors[i].rgbBlue =
        Info->bmiColors[i].rgbRed =
        Info->bmiColors[i].rgbGreen = i;
        Info->bmiColors[i].rgbReserved = 0;
    }
    Info->bmiHeader.biBitCount = 8;
    Info->bmiHeader.biClrImportant = 0;
    Info->bmiHeader.biClrUsed = 0;
    Info->bmiHeader.biCompression = BI_RGB;
    Info->bmiHeader.biHeight = imgHeight;
    Info->bmiHeader.biPlanes = 1;
    Info->bmiHeader.biSize = 40;
    Info->bmiHeader.biSizeImage = imgWidth*imgHeight;
    Info->bmiHeader.biWidth = imgWidth;
    Info->bmiHeader.biXPelsPerMeter = 0;
    Info->bmiHeader.biYPelsPerMeter = 0;

    fwrite(&header, 1, 14, fp);
    fwrite(Info, 1, 1064, fp);

    if(Invert == TRUE)
    {
        unsigned char *inv_buffer = new unsigned char [imgWidth*imgHeight];
        for(i=0; i<imgWidth*imgHeight; i++)
            inv_buffer[i] = 255 - pBuf[i];
        fwrite(inv_buffer, 1, imgWidth*imgHeight, fp);
        delete [] inv_buffer;
    }
    else
        fwrite(pBuf, 1, imgWidth*imgHeight, fp);

    fclose(fp);

    delete Info;

    return TRUE;
}

int CIBSMAlgorithm::_SaveBitmapImage_UserSize(const char *filename, const unsigned char *pBuf, const int imgWidth, const int imgHeight,
                                const int TargetWidth, const int TargetHeight, const unsigned char magnifier, const BOOL Invert)
{
    unsigned char *pTargetBuf = new unsigned char [TargetWidth*TargetHeight];
    int i, j, s, t;

    for(i=0; i<TargetHeight; i++)
    {
        s = i * imgHeight / TargetHeight;
        for(j=0; j<TargetWidth; j++)
        {
            t = j * imgWidth / TargetWidth;
            pTargetBuf[i*TargetWidth+j] = pBuf[s*imgWidth+t]*magnifier;
        }
    }

    int ret = _SaveBitmapImage(filename, (const unsigned char*)pTargetBuf, TargetWidth, TargetHeight, Invert);

    delete [] pTargetBuf;

    return ret;
}