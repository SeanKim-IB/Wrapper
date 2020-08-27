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
#include "IBAlgorithm.h"
#include <math.h>
#ifdef WINCE
#include "WinCEMath.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#ifdef __linux__
#include <sys/time.h>
#endif

#if defined(_WINDOWS) && !defined(WINCE) && defined(__IBSCAN_ULTIMATE_SDK__) 
/*#pragma comment (lib , "nfiq.lib" )
#pragma comment (lib , "mindtct.lib" )
#pragma comment (lib , "pcasys.lib" )
#pragma comment (lib , "commonnbis.lib" )
*/
#pragma comment (lib , "wsq.lib" )
#pragma comment (lib , "pcasys.lib" )
#pragma comment (lib , "nfseg.lib" )
#pragma comment (lib , "nfiq.lib" )
#pragma comment (lib , "mindtct.lib" )
#pragma comment (lib , "jpegl.lib" )
#pragma comment (lib , "ioutil.lib" )
#pragma comment (lib , "commonnbis.lib" )
#elif defined(WINCE) && defined(__IBSCAN_ULTIMATE_SDK__) 
#pragma comment (lib , "wsq_ARM.lib" )
#pragma comment (lib , "pcasys_ARM.lib" )
#pragma comment (lib , "nfseg_ARM.lib" )
#pragma comment (lib , "nfiq_ARM.lib" )
#pragma comment (lib , "mindtct_ARM.lib" )
#pragma comment (lib , "jpegl_ARM.lib" )
#pragma comment (lib , "ioutil_ARM.lib" )
#pragma comment (lib , "commonnbis_ARM.lib" )
#endif
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Implementation


#define BLOCK_ALLOC_MB (1024*1024)
#define BLOCK_ALLOC_TOTAL_SIZE (31*BLOCK_ALLOC_MB)
//#define BLOCK_ALLOC_TOTAL_SIZE (30*BLOCK_ALLOC_MB)
#define BLOCK_ALLOC_SIGNATURE 0xB10CB10C  /* Sort of looks like BLOCKBLOCK */
#define BLOCK_ALLOC_FREED 0xB10CF8EE      /* Sort of looks like BLOCKFREE */

#ifdef WINCE
#define BLOCKALLOC(objtype, number) (objtype*)BlockAlloc(sizeof(objtype),number)
 #ifdef DEBUG
#define BLOCKFREE(ptr) BlockFree((void *)(ptr));
 #else
#define BLOCKFREE(ptr);
 #endif
#else
#define BLOCKALLOC(objtype, number) new objtype[(number)]
#define BLOCKFREE(ptr) delete[] ptr
#endif
#ifdef WINCE
void *CIBAlgorithm::BlockAlloc(int iObjSize, int iObjNumber)
{
	if( m_BlockAllocNext != NULL )
	{
		int iSize = ((iObjSize * iObjNumber) + 0xF) & 0xFFFFFFF0; // Align on 16 byte boundary
#ifdef DEBUG
		DEBUGMSG(1, (TEXT("CIBAlgorithm::BlockAlloc(iObjSize=0x%x, iObjNumber=0x%x) iSize=0x%x remaing=0x%x\r\n"),
			iObjSize, iObjNumber, iSize, m_BlockAllocSize - m_BlockAllocUsed ));

		*((DWORD *)m_BlockAllocNext) = BLOCK_ALLOC_SIGNATURE;    // So BLOCKFREE can verify not trying to use BLOCKFREE when delete[] should be used
		*(((DWORD *)m_BlockAllocNext)+1) = (DWORD)iSize;        // May be useful
		m_BlockAllocNext += 16;
		m_BlockAllocUsed += 16;
#endif	
		if( m_BlockAllocUsed + iSize <= m_BlockAllocSize )
		{
			BYTE *pRet = m_BlockAllocNext;
			m_BlockAllocNext += iSize;
			m_BlockAllocUsed += iSize;
			DEBUGMSG(1, (TEXT("CIBAlgorithm::BlockAlloc returning=0x%x, remaining=0x%x\r\n"),
			pRet,  m_BlockAllocSize - m_BlockAllocUsed ));
			return pRet;
		}
		else
		{	
			DebugBreak();
			throw IBSU_ERR_MEM_ALLOC;
			return NULL;
		}
	}
	else
	{
		DebugBreak();
		throw IBSU_ERR_MEM_ALLOC;
		return NULL;
	}
}

void CIBAlgorithm::BlockFree(void *ptr)
{
#ifdef DEBUG
	if( *((DWORD *)ptr-4) != BLOCK_ALLOC_SIGNATURE)
	{
		DebugBreak();
	}
	*((DWORD *)ptr-4) = BLOCK_ALLOC_FREED;
#endif
}
#endif	

CIBAlgorithm::CIBAlgorithm(UsbDeviceInfo *pUsbDevInfo, PropertyInfo *pPropertyInfo)
{
#ifdef WINCE
	// Allocate a large chunk of memory from shared memory to avoid using the 32Mb per process memory.
	m_BlockAllocStart = (BYTE *)VirtualAlloc( NULL, BLOCK_ALLOC_TOTAL_SIZE, MEM_RESERVE, PAGE_NOACCESS );
	if( m_BlockAllocStart == NULL )
	{
		DEBUGMSG(1, (TEXT("CIBAlgorithm::CIBAlgorithm VirtualAlloc MEM_RESERVE failed with error=0x%x\r\n"),
		GetLastError() ));
		DebugBreak();
	}
	DEBUGMSG(1, (TEXT("CIBAlgorithm::CIBAlgorithm VirtualAlloc reserved=0x%x\r\n"), m_BlockAllocStart ));

	BYTE *pTemp;
	pTemp = (BYTE *)VirtualAlloc( m_BlockAllocStart, BLOCK_ALLOC_TOTAL_SIZE-1, MEM_COMMIT, PAGE_READWRITE );
	if( pTemp == NULL )
	{
		DEBUGMSG(1, (TEXT("CIBAlgorithm::CIBAlgorithm VirtualAlloc MEM_COMMIT failed with error=0x%x\r\n"),
		GetLastError() ));
		DebugBreak();
	}

    m_BlockAllocSize = BLOCK_ALLOC_TOTAL_SIZE;
	m_BlockAllocUsed = 0;
	m_BlockAllocNext = m_BlockAllocStart;
#endif
	
	try
	{
    // 2013-05-21 enzyme modify -
    //	memcpy(&m_UsbDevInfo, pUsbDevInfo, sizeof(UsbDeviceInfo));
    m_pUsbDevInfo           = pUsbDevInfo;
	m_pPropertyInfo			= pPropertyInfo;

    CIS_IMG_W				= m_pUsbDevInfo->CisImgWidth;
    CIS_IMG_H				= m_pUsbDevInfo->CisImgHeight;
    CIS_IMG_SIZE			= CIS_IMG_H * CIS_IMG_W;
    IMG_W					= m_pUsbDevInfo->ImgWidth;
    IMG_H					= m_pUsbDevInfo->ImgHeight;
    IMG_SIZE				= IMG_H * IMG_W;
    IMG_HALF_W				= (IMG_W >> 1);
    IMG_HALF_H				= (IMG_H >> 1);
    IMG_HALF_SIZE			= IMG_HALF_H * IMG_HALF_W;

	CIS_IMG_W_ROLL	        = m_pUsbDevInfo->CisImgWidth_Roll;
	CIS_IMG_H_ROLL	        = m_pUsbDevInfo->CisImgHeight_Roll;
	CIS_IMG_SIZE_ROLL	    = CIS_IMG_W_ROLL*CIS_IMG_H_ROLL;
	IMG_W_ROLL		        = m_pUsbDevInfo->ImgWidth_Roll;
	IMG_H_ROLL		        = m_pUsbDevInfo->ImgHeight_Roll;
	IMG_SIZE_ROLL		    = IMG_W_ROLL*IMG_H_ROLL;

    ENLARGESIZE_W			= 60;
    ENLARGESIZE_H			= 60;

    //------------------------------------------------------------------------------------------
    // Items	    Watson	DPI		Columbo	DPI		Sherlock DPI	Curve	DPI		Holmes	DPI
    //------------------------------------------------------------------------------------------
    // Width(inch)	1.6				0.8				1.6				0.5906			3.2
    // Height(inch)	1.5				1				1.5				0.7086			3
    // CIS_Width	1088	680		880		1100	800		500		360		609.5	1600	500
    // CIS_Height	1022	681.3	1100	1100	750		500		440		620.9	1500	500
    // IMG_Width	800		500		400		500		800		500		288		487.6	800		250
    // IMG_Height	750		500		500		500		750		500		352		496.7	750		250
    // ZOOM_OUT		5				5				5				5				2
    // ZOOM_Width	160		100		80		100		160		100		57.6	97.5	400		125
    // ZOOM_Height	150		100		100		100		150		100		70.4	99.3	375		125
    //------------------------------------------------------------------------------------------
    ZOOM_OUT = m_pUsbDevInfo->nZoomOut;

    ZOOM_H					= (IMG_H / ZOOM_OUT);
    ZOOM_W					= (IMG_W / ZOOM_OUT);

    ENLARGESIZE_ZOOM_H		= (ENLARGESIZE_H / ZOOM_OUT);
    ENLARGESIZE_ZOOM_W		= (ENLARGESIZE_W / ZOOM_OUT);
    ZOOM_ENLAGE_H			= (ZOOM_H + ENLARGESIZE_ZOOM_H * 2);
    ZOOM_ENLAGE_W			= (ZOOM_W + ENLARGESIZE_ZOOM_W * 2);

	ZOOM_OUT_ROLL		    = m_pUsbDevInfo->nZoomOut;
	ZOOM_H_ROLL		        = (IMG_H_ROLL / ZOOM_OUT_ROLL);
	ZOOM_W_ROLL		        = (IMG_W_ROLL / ZOOM_OUT_ROLL);
	ENLARGESIZE_ZOOM_H_ROLL = (ENLARGESIZE_H / ZOOM_OUT_ROLL);
	ENLARGESIZE_ZOOM_W_ROLL = (ENLARGESIZE_H / ZOOM_OUT_ROLL);
	ZOOM_ENLAGE_H_ROLL      = (ZOOM_H_ROLL + ENLARGESIZE_ZOOM_H_ROLL * 2);
	ZOOM_ENLAGE_W_ROLL      = (ZOOM_W_ROLL + ENLARGESIZE_ZOOM_W_ROLL * 2);

	switch (m_pUsbDevInfo->devType)
	{
	case DEVICE_TYPE_WATSON:
	case DEVICE_TYPE_WATSON_MINI:
		m_AESEnc = new CIBEncryption(AES_KEY_ENCRYPTION);
		arrPos_Watson = BLOCKALLOC( DWORD, IMG_SIZE + 1);		// "+1" is checksum
		m_UM_Watson_F = BLOCKALLOC( short, CIS_IMG_SIZE );
		m_UM_Watson_F_Enhanced = BLOCKALLOC( short, IMG_SIZE );
		break;
	case DEVICE_TYPE_HOLMES:
		break;
	case DEVICE_TYPE_SHERLOCK:
	case DEVICE_TYPE_SHERLOCK_ROIC:
		m_TFT_DefectMaskImg = BLOCKALLOC(BYTE, CIS_IMG_SIZE);
		m_TFT_MaskImg = BLOCKALLOC( BYTE, CIS_IMG_SIZE );
		m_tmpNoiseBuf = BLOCKALLOC( int, CIS_IMG_SIZE );
		mean_minus = BLOCKALLOC( int, CIS_IMG_W );	// vertical noise issue
		mean_cnt = BLOCKALLOC( short, CIS_IMG_W );	// vertical noise issue
		m_UM_Watson_F = BLOCKALLOC( short, CIS_IMG_SIZE );
		m_UM_Watson_F_Enhanced = BLOCKALLOC( short, IMG_SIZE );
		break;
	case DEVICE_TYPE_COLUMBO:
		m_final_image = BLOCKALLOC( BYTE, CIS_IMG_SIZE );	// Added decimated distorition mask
	    memset(m_final_image, 0xFF, CIS_IMG_SIZE);
		arrPos_Columbo = BLOCKALLOC( DWORD, IMG_SIZE * 2 + 1 );	// Added decimated distorition mask
		m_UM_Watson_F = BLOCKALLOC( short, CIS_IMG_SIZE );
		m_UM_Watson_F_Enhanced = BLOCKALLOC( short, IMG_SIZE );
		break;
	case DEVICE_TYPE_CURVE:
	case DEVICE_TYPE_CURVE_SETI:
		m_Curve_refx1_arr = BLOCKALLOC( int, IMG_W );
		m_Curve_refx2_arr = BLOCKALLOC( int, IMG_W );
		m_Curve_refy1_arr = BLOCKALLOC( int, IMG_H );
		m_Curve_refy2_arr = BLOCKALLOC( int, IMG_H );
		m_Curve_p_arr = BLOCKALLOC( int, IMG_H );
		m_Curve_q_arr = BLOCKALLOC( int, IMG_W );
		m_Curve_Org = BLOCKALLOC( BYTE, ((CIS_IMG_W + SIDE_W) * (CIS_IMG_H + UPPER_H) + __CURVE_MAX_PACKET_SIZE__));
		m_Curve_Tmp1 = BLOCKALLOC( BYTE, CIS_IMG_SIZE );
		m_Curve_Tmp2 = BLOCKALLOC( BYTE, CIS_IMG_SIZE );
		m_PinholeMask = BLOCKALLOC( BYTE, CIS_IMG_SIZE );
		break;
	case DEVICE_TYPE_KOJAK:		// There is distortion array for KOJAK in m_SBDAlg
//		arrPos_Kojak = BLOCKALLOC( DWORD, IMG_SIZE + 1);		// "+1" is checksum
		m_Inter_Seg_Img = BLOCKALLOC( BYTE, CIS_IMG_SIZE );
		m_final_image = BLOCKALLOC( BYTE, CIS_IMG_SIZE );	// Added decimated distorition mask
		break;
	case DEVICE_TYPE_FIVE0:		// There is distortion array for KOJAK in m_SBDAlg
		m_TFT_DefectMaskImg = BLOCKALLOC(BYTE, CIS_IMG_SIZE);
		m_TFT_MaskImg = BLOCKALLOC( BYTE, CIS_IMG_SIZE );
		m_TFT_MaskImgforFlat = BLOCKALLOC( BYTE, CIS_IMG_SIZE );
		m_TFT_MaskImgforRoll = BLOCKALLOC( BYTE, CIS_IMG_SIZE_ROLL );
		break;
	default:
		break;
	}

	sum_gray_buffer = BLOCKALLOC(int, CIS_IMG_SIZE);

	m_capture_rolled_buffer = BLOCKALLOC(BYTE, CIS_IMG_SIZE );
	m_capture_rolled_best_buffer = BLOCKALLOC( BYTE, CIS_IMG_SIZE );
	m_capture_rolled_local_best_buffer = BLOCKALLOC( BYTE, CIS_IMG_SIZE );

	memset(m_capture_rolled_buffer, 0, CIS_IMG_SIZE);
	memset(m_capture_rolled_best_buffer, 0, CIS_IMG_SIZE);
	memset(m_capture_rolled_local_best_buffer, 0, CIS_IMG_SIZE);

#if defined(__IBSCAN_SDK__)
	m_GetImgFromApp = BLOCKALLOC( BYTE, ((CIS_IMG_W + SIDE_W) * (CIS_IMG_H + UPPER_H)));
#endif
	m_ImgFromCIS = BLOCKALLOC( BYTE, ((CIS_IMG_W + SIDE_W) * (CIS_IMG_H + UPPER_H)));
	m_ImgOnProcessing = BLOCKALLOC( BYTE, ((CIS_IMG_W + SIDE_W) * (CIS_IMG_H + UPPER_H)));
	m_Inter_Img = BLOCKALLOC( BYTE, ((CIS_IMG_W + SIDE_W) * (CIS_IMG_H + UPPER_H)));
	m_Inter_Img2 = BLOCKALLOC( BYTE, CIS_IMG_SIZE );
	m_Inter_Img3 = BLOCKALLOC( BYTE, CIS_IMG_SIZE );
	m_Inter_Img4 = BLOCKALLOC( BYTE, CIS_IMG_SIZE );
	m_Inter_Img5 = BLOCKALLOC( BYTE, CIS_IMG_SIZE );

	m_OutRollResultImg = BLOCKALLOC(BYTE, IMG_SIZE);
	m_OutResultImg = BLOCKALLOC(BYTE, IMG_SIZE);
	m_OutSplitResultArray = BLOCKALLOC( BYTE, (IMG_SIZE*IBSU_MAX_SEGMENT_COUNT));
	m_OutSplitResultArrayEx = BLOCKALLOC( BYTE, (IMG_SIZE*IBSU_MAX_SEGMENT_COUNT));
	m_OutEnhancedImg = BLOCKALLOC(BYTE, IMG_SIZE);
	m_OutSplitEnhancedArray = BLOCKALLOC( BYTE, (IMG_SIZE*IBSU_MAX_SEGMENT_COUNT));
	m_OutSplitEnhancedArrayEx = BLOCKALLOC( BYTE, (IMG_SIZE*IBSU_MAX_SEGMENT_COUNT));
#ifdef __IBSCAN_ULTIMATE_SDK__
	memset(m_OutRollResultImg, 0xFF, IMG_SIZE);
    memset(m_OutResultImg, 0xFF, IMG_SIZE);
    memset(m_OutSplitResultArray, 0xFF, IMG_SIZE*IBSU_MAX_SEGMENT_COUNT);
    memset(m_OutSplitResultArrayEx, 0xFF, IMG_SIZE*IBSU_MAX_SEGMENT_COUNT);

	memset(m_OutEnhancedImg, 0xFF, IMG_SIZE);
	memset(m_OutSplitEnhancedArray, 0xFF, IMG_SIZE*IBSU_MAX_SEGMENT_COUNT);
    memset(m_OutSplitEnhancedArrayEx, 0xFF, IMG_SIZE*IBSU_MAX_SEGMENT_COUNT);
#else
	memset(m_OutRollResultImg, 0x00, IMG_SIZE);
    memset(m_OutResultImg, 0x00, IMG_SIZE);
    memset(m_OutSplitResultArray, 0x00, IMG_SIZE*IBSU_MAX_SEGMENT_COUNT);
    memset(m_OutSplitResultArrayEx, 0x00, IMG_SIZE*IBSU_MAX_SEGMENT_COUNT);

	memset(m_OutEnhancedImg, 0x00, IMG_SIZE);
    memset(m_OutSplitEnhancedArray, 0x00, IMG_SIZE*IBSU_MAX_SEGMENT_COUNT);
    memset(m_OutSplitEnhancedArrayEx, 0x00, IMG_SIZE*IBSU_MAX_SEGMENT_COUNT);
#endif
	m_OutSplitResultArrayExWidth = BLOCKALLOC( int, IBSU_MAX_SEGMENT_COUNT);
	m_OutSplitResultArrayExHeight = BLOCKALLOC( int, IBSU_MAX_SEGMENT_COUNT);
	m_OutSplitResultArrayExSize = BLOCKALLOC( int, IBSU_MAX_SEGMENT_COUNT);
	m_OutSplitEnhancedArrayExWidth = BLOCKALLOC( int, IBSU_MAX_SEGMENT_COUNT);
	m_OutSplitEnhancedArrayExHeight = BLOCKALLOC( int, IBSU_MAX_SEGMENT_COUNT);
	m_OutSplitEnhancedArrayExSize = BLOCKALLOC( int, IBSU_MAX_SEGMENT_COUNT);
	
    m_TFT_DefectMaskList = NULL;

	stack = BLOCKALLOC( USHORT, (ZOOM_ENLAGE_H * ZOOM_ENLAGE_W * 32));
	m_labeled_segment_enlarge_buffer = BLOCKALLOC( BYTE, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W );
	EnlargeBuf = BLOCKALLOC( BYTE, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W );
	EnlargeBuf_Org = BLOCKALLOC( BYTE, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W );
	m_segment_enlarge_buffer = BLOCKALLOC( BYTE, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W );
	m_segment_enlarge_buffer_for_fingercnt = BLOCKALLOC( BYTE, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W );

	m_WaterImage = BLOCKALLOC( BYTE, m_pUsbDevInfo->nWaterImageSize + 1024 ); /* +4 is checksum */

	G_GammaTable = BLOCKALLOC( BYTE, (48 * 256) );

	if(m_pUsbDevInfo->devType != DEVICE_TYPE_KOJAK &&
		m_pUsbDevInfo->devType != DEVICE_TYPE_FIVE0)
	{
		m_BestFrame.score = 0;
		m_BestFrame.Buffer = BLOCKALLOC( BYTE, CIS_IMG_SIZE );
	}

	m_refy1_arr = BLOCKALLOC( int, IMG_H );
	m_refy2_arr = BLOCKALLOC( int, IMG_H );
	m_p_arr = BLOCKALLOC( short, IMG_H );
	m_refx1_arr = BLOCKALLOC( int, IMG_W );
	m_refx2_arr = BLOCKALLOC( int, IMG_W );
	m_q_arr = BLOCKALLOC( short, IMG_W );

	mean_h = BLOCKALLOC( int, CIS_IMG_W );

    memset(&m_cImgAnalysis, 0, sizeof(m_cImgAnalysis));

    m_rollingStatus = 0;

    m_bDistortionMaskPath = FALSE;
    memset(m_cDistortionMaskPath, '0', 1024);

    m_prevInput_height = -1;
    m_prevInput_width = -1;
    m_prevZoomout_height = -1;
    m_prevzoomout_width = -1;

	m_PPI_Correction_Horizontal	= 0;
	m_PPI_Correction_Vertical	= 0;

    m_pUsbDevInfo->nThresholdGoodRollX = (int)(THRESHOLD_GOOD_ROLL_X * m_pUsbDevInfo->CisImgWidth / 1088);
    m_pUsbDevInfo->nThresholdGoodRollArea = (int)(THRESHOLD_GOOD_ROLL_AREA * m_pUsbDevInfo->CisImgSize / (1088 * 1022));

	m_DryCapture = FALSE;

    if (m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK ||
		m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
    {
	    m_SBDAlg = new CSBD_Alg(pUsbDevInfo, pPropertyInfo);
		if (m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
			m_SBDAlg->m_SumGrayBuf = sum_gray_buffer;
	    m_SBDAlg->m_pcImgAnalysis = &m_cImgAnalysis;
		m_SBDAlg->m_BestRollImg = m_capture_rolled_local_best_buffer;
		m_MagBuffer_short = (USHORT*)&m_SBDAlg->m_SumGrayBuf2[0];
		m_MagBuffer = (BYTE*)&m_SBDAlg->m_SumGrayBuf2[IMG_SIZE*sizeof(USHORT)];

		m_SBDAlg->ZOOM_OUT = ZOOM_OUT;
		m_SBDAlg->ZOOM_H = ZOOM_H;
		m_SBDAlg->ZOOM_W = ZOOM_W;
		m_SBDAlg->ENLARGESIZE_ZOOM_H = ENLARGESIZE_ZOOM_H;
		m_SBDAlg->ENLARGESIZE_ZOOM_W = ENLARGESIZE_ZOOM_W;
		m_SBDAlg->ZOOM_ENLAGE_H = ZOOM_ENLAGE_H;
		m_SBDAlg->ZOOM_ENLAGE_W = ZOOM_ENLAGE_W;

		m_SBDAlg->ZOOM_OUT_ROLL = ZOOM_OUT_ROLL;
		m_SBDAlg->ZOOM_H_ROLL = ZOOM_H_ROLL;
		m_SBDAlg->ZOOM_W_ROLL = ZOOM_W_ROLL;
		m_SBDAlg->ENLARGESIZE_ZOOM_H_ROLL = ENLARGESIZE_ZOOM_H_ROLL;
		m_SBDAlg->ENLARGESIZE_ZOOM_W_ROLL = ENLARGESIZE_ZOOM_W_ROLL;
		m_SBDAlg->ZOOM_ENLAGE_H_ROLL = ZOOM_ENLAGE_H_ROLL;
		m_SBDAlg->ZOOM_ENLAGE_W_ROLL = ZOOM_ENLAGE_W_ROLL;
    }
	else
	{
		m_MagBuffer_short = BLOCKALLOC(USHORT, IMG_SIZE);
		m_MagBuffer = BLOCKALLOC(BYTE, IMG_SIZE);
	}

	m_CalledByValidGeo = FALSE;

	memset(&m_CBPPreviewInfo, 0, sizeof(CBPPreviewInfo));

#if defined(DEBUG) && defined(WINCE)
	DEBUGMSG(1, (TEXT("CIBAlgorithm::CIBAlgorithm BlockAlloc Size=0x%x, BlockAlloc Used=0x%x, Available=0x%x\r\n"),
		m_BlockAllocSize, m_BlockAllocUsed, m_BlockAllocSize-m_BlockAllocUsed ));
#endif
	}
	catch (...)
	{
#ifdef WINCE
		// Clear the entire shared memory block, so no fingerprints are left in shared memory after class destruction
		memset(m_BlockAllocStart, 0, BLOCK_ALLOC_TOTAL_SIZE);
	    VirtualFree(m_BlockAllocStart, BLOCK_ALLOC_TOTAL_SIZE, MEM_DECOMMIT );
	    VirtualFree(m_BlockAllocStart, 0, MEM_RELEASE );
#endif
		throw IBSU_ERR_MEM_ALLOC;
	}
}

CIBAlgorithm::~CIBAlgorithm(void)
{
	switch (m_pUsbDevInfo->devType)
	{
	case DEVICE_TYPE_WATSON:
	case DEVICE_TYPE_WATSON_MINI:
		delete m_AESEnc;
	    BLOCKFREE( arrPos_Watson );
		BLOCKFREE( m_UM_Watson_F );
		BLOCKFREE( m_UM_Watson_F_Enhanced );
		break;
	case DEVICE_TYPE_HOLMES:
		break;
	case DEVICE_TYPE_SHERLOCK:
	case DEVICE_TYPE_SHERLOCK_ROIC:
		BLOCKFREE( m_TFT_DefectMaskImg );
	    BLOCKFREE( m_TFT_MaskImg );
	    BLOCKFREE( m_tmpNoiseBuf );
		BLOCKFREE( mean_minus );
		BLOCKFREE( mean_cnt );
		BLOCKFREE( m_UM_Watson_F );
		BLOCKFREE( m_UM_Watson_F_Enhanced );
		break;
	case DEVICE_TYPE_COLUMBO:
		BLOCKFREE( arrPos_Columbo );
		BLOCKFREE( m_final_image );
		BLOCKFREE( m_UM_Watson_F );
		BLOCKFREE( m_UM_Watson_F_Enhanced );
		break;
	case DEVICE_TYPE_CURVE:
	case DEVICE_TYPE_CURVE_SETI:
		BLOCKFREE( m_Curve_refx1_arr );
		BLOCKFREE( m_Curve_refx2_arr );
		BLOCKFREE( m_Curve_refy1_arr );
		BLOCKFREE( m_Curve_refy2_arr );
		BLOCKFREE( m_Curve_p_arr );
		BLOCKFREE( m_Curve_q_arr );
		BLOCKFREE( m_Curve_Org );
		BLOCKFREE( m_Curve_Tmp1 );
		BLOCKFREE( m_Curve_Tmp2 );
	    BLOCKFREE( m_PinholeMask );
		break;
	case DEVICE_TYPE_KOJAK:
	//	BLOCKFREE( arrPos_Kojak );
		BLOCKFREE( m_Inter_Seg_Img );
		BLOCKFREE( m_final_image );
		break;
	case DEVICE_TYPE_FIVE0:		// There is distortion array for KOJAK in m_SBDAlg
		BLOCKFREE( m_TFT_DefectMaskImg );
		BLOCKFREE( m_TFT_MaskImg );
		BLOCKFREE( m_TFT_MaskImgforFlat );
		BLOCKFREE( m_TFT_MaskImgforRoll );
	default:
		break;
	}

    BLOCKFREE(sum_gray_buffer);

    BLOCKFREE( m_capture_rolled_buffer );
    BLOCKFREE( m_capture_rolled_best_buffer );
    BLOCKFREE( m_capture_rolled_local_best_buffer );
#if defined(__IBSCAN_SDK__)
    BLOCKFREE( m_GetImgFromApp );
#endif
    BLOCKFREE( m_ImgFromCIS );
    BLOCKFREE( m_ImgOnProcessing );
    BLOCKFREE( m_Inter_Img );
    BLOCKFREE( m_Inter_Img2 );
    BLOCKFREE( m_Inter_Img3 );
    BLOCKFREE( m_Inter_Img4 );
    BLOCKFREE( m_Inter_Img5 );

	BLOCKFREE( m_OutRollResultImg );
    BLOCKFREE( m_OutResultImg );
    BLOCKFREE( m_OutEnhancedImg );
    BLOCKFREE( m_OutSplitResultArray );
	BLOCKFREE( m_OutSplitEnhancedArray );
	BLOCKFREE( m_OutSplitResultArrayEx );
	BLOCKFREE( m_OutSplitEnhancedArrayEx );
    BLOCKFREE( m_OutSplitResultArrayExWidth );  
	BLOCKFREE( m_OutSplitResultArrayExHeight ); 
	BLOCKFREE( m_OutSplitResultArrayExSize );  
    BLOCKFREE( m_OutSplitEnhancedArrayExWidth );  
	BLOCKFREE( m_OutSplitEnhancedArrayExHeight ); 
	BLOCKFREE( m_OutSplitEnhancedArrayExSize );  
	
	if (m_pUsbDevInfo->devType != DEVICE_TYPE_KOJAK &&
		m_pUsbDevInfo->devType != DEVICE_TYPE_FIVE0 )
    {
		BLOCKFREE( m_MagBuffer_short );
		BLOCKFREE( m_MagBuffer );
	}

    if (m_TFT_DefectMaskList)
	{
        delete [] m_TFT_DefectMaskList;
    }

    BLOCKFREE( stack );
	BLOCKFREE( m_labeled_segment_enlarge_buffer );

    BLOCKFREE( EnlargeBuf );
    BLOCKFREE( EnlargeBuf_Org );
    BLOCKFREE( m_segment_enlarge_buffer );
    BLOCKFREE( m_segment_enlarge_buffer_for_fingercnt );

    BLOCKFREE( m_WaterImage );

    BLOCKFREE( G_GammaTable );

    BLOCKFREE( m_refy1_arr );
    BLOCKFREE( m_refy2_arr );
    BLOCKFREE( m_p_arr );
    BLOCKFREE( m_refx1_arr );
    BLOCKFREE( m_refx2_arr );
    BLOCKFREE( m_q_arr );
    BLOCKFREE( mean_h );

	if (m_pUsbDevInfo->devType != DEVICE_TYPE_KOJAK &&
		m_pUsbDevInfo->devType != DEVICE_TYPE_FIVE0)
    {
		BLOCKFREE( m_BestFrame.Buffer );
	}

    if (m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK ||
		m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
    {
	    delete m_SBDAlg;
    }

#ifdef WINCE
	// Clear the entire shared memory block, so no fingerprints are left in shared memory after class destruction
	memset(m_BlockAllocStart, 0, BLOCK_ALLOC_TOTAL_SIZE);
	VirtualFree(m_BlockAllocStart, BLOCK_ALLOC_TOTAL_SIZE, MEM_DECOMMIT );
	VirtualFree(m_BlockAllocStart, 0, MEM_RELEASE );
#endif

	std::vector<FingerQuality *>::iterator it = m_pFingerQuality.begin();
	while( it != m_pFingerQuality.end() )
	{
		if( *it )
		{
			delete *it;
			it = m_pFingerQuality.erase(it);
		}
		else
			++it;
	}

	m_pFingerQuality.clear();
}

#ifdef __USE_DEBUG_WITH_BITMAP__
#ifdef _WINDOWS
int CIBAlgorithm::SaveBitmapImage(char *filename, unsigned char *buffer, int width, int height, int invert)
{
    //	return FALSE;

    FILE *fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        return FALSE;
    }

    int i;
    IB_BITMAPFILEHEADER header;
    IB_BITMAPINFO *Info = (IB_BITMAPINFO *)new unsigned char [1064];

    header.bfOffBits = 1078;
    header.bfReserved1 = 0;
    header.bfReserved2 = 0;
    header.bfSize = 1078 + width * height;
    header.bfType = (unsigned short)(('M' << 8) | 'B');

    for (i = 0; i < 256; i++)
    {
        Info->bmiColors[i].rgbBlue = i;
        Info->bmiColors[i].rgbRed = i;
        Info->bmiColors[i].rgbGreen = i;
        Info->bmiColors[i].rgbReserved = 0;
    }
    Info->bmiHeader.biBitCount = 8;
    Info->bmiHeader.biClrImportant = 0;
    Info->bmiHeader.biClrUsed = 0;
    Info->bmiHeader.biCompression = BI_RGB;
    Info->bmiHeader.biHeight = height;
    Info->bmiHeader.biPlanes = 1;
    Info->bmiHeader.biSize = 40;
    Info->bmiHeader.biSizeImage = width * height;
    Info->bmiHeader.biWidth = width;
    Info->bmiHeader.biXPelsPerMeter = 0;
    Info->bmiHeader.biYPelsPerMeter = 0;

    fwrite(&header, 1, 14, fp);
    fwrite(Info, 1, 1064, fp);

    if (invert == TRUE)
    {
        unsigned char *inv_buffer = new unsigned char [width * height];
        for (i = 0; i < width * height; i++)
        {
            inv_buffer[i] = 255 - buffer[i];
        }
        fwrite(inv_buffer, 1, width * height, fp);
        delete [] inv_buffer;
    }
    else
    {
        fwrite(buffer, 1, width * height, fp);
    }

    fclose(fp);

    delete Info;

    return TRUE;
}
#endif
#endif

void CIBAlgorithm::_Algo_MakeUniformityMask(BYTE *WaterImage)
{
    int i;
    int rectTop, rectBottom, rectLeft, rectRight;
    int dumySize;

    dumySize = IMG_W / 4;
    rectTop = CIS_IMG_H / 2 - dumySize;
    rectBottom = CIS_IMG_H / 2 + dumySize;
    rectLeft = CIS_IMG_W / 2 - dumySize;
    rectRight = CIS_IMG_W / 2 + dumySize;
    int gray_val_int = _Algo_SearchingMaxGraybox_Fast(WaterImage, CIS_IMG_W, CIS_IMG_H,
                       rectTop, rectBottom, rectLeft, rectRight);

    int inImg_val;

    for (i = 0; i < CIS_IMG_SIZE; i++)
    {
        inImg_val = WaterImage[i];

        if (inImg_val == 0)
        {
            m_UM_Watson_F[i]  = 0;
        }
        else
        {
            m_UM_Watson_F[i] = (short)((gray_val_int - inImg_val) * 1024 / inImg_val);
        }
    }

    if (m_pUsbDevInfo->devType == DEVICE_TYPE_CURVE || m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
    {
        return;
    }

	unsigned char *Res_WaterImage = new unsigned char [IMG_SIZE];
	m_cImgAnalysis.LEFT = 0;
	m_cImgAnalysis.TOP = 0;
	m_cImgAnalysis.RIGHT = CIS_IMG_W-1;
	m_cImgAnalysis.BOTTOM = CIS_IMG_H-1;
	
	if(m_pUsbDevInfo->devType == DEVICE_TYPE_COLUMBO)
		_Algo_Columbo_DistortionRestoration_with_Bilinear(WaterImage, Res_WaterImage);
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_WATSON ||
			m_pUsbDevInfo->devType == DEVICE_TYPE_WATSON_MINI)
		_Algo_DistortionRestoration_with_Bilinear(WaterImage, Res_WaterImage);
	else
		memcpy(Res_WaterImage, WaterImage, CIS_IMG_SIZE);

	for (i = 0; i < IMG_SIZE; i++)
    {
        inImg_val = Res_WaterImage[i];

        if (inImg_val == 0)
        {
            m_UM_Watson_F_Enhanced[i]  = 0;
        }
        else
        {
            m_UM_Watson_F_Enhanced[i] = (short)((gray_val_int - inImg_val) * 1024 / inImg_val);
        }
    }

	delete [] Res_WaterImage;
}

void CIBAlgorithm::_Algo_RemoveNoise(unsigned char *InImg, unsigned char *OutImg, int ImgSize, int ContrastLevel)
{
    int i, val;

    UCHAR G_NoiseTable[35] =
    {
        11, 11, 11, 11, 11, 11,	11,	// 0 ~ 6
        12, 12, 12, 12, 12, 12,	12,	12,	// 7 ~ 14
        13, 13, 13, 13, 13, 13,	13,	// 15 ~ 21
        14, 14, 14, 14, 14, 14,	// 22 ~ 27
        15, 15, // 28 ~ 29
        16, 16, // 30 ~ 31
        17, 17, // 32 ~ 33
        18	// 34
    };

    int minus_value = (G_NoiseTable[ContrastLevel] < m_cImgAnalysis.noise_histo) ?
                      G_NoiseTable[ContrastLevel] : m_cImgAnalysis.noise_histo;

    for (i = 0; i < ImgSize; i++)
    {
        val = (int)InImg[i] - minus_value;
        if (val < 0)
        {
            val = 0;
        }
        OutImg[i] = val;//Contrast_LUT[InImg[i]];
    }
}

void CIBAlgorithm::_Algo_HistogramStretchForZoomEnlarge(unsigned char *ImageBuf)
{
    const double D_CUT_MINTRES = 0.001;
    const double D_CUT_MAXTRES =	0.001;

    int y, x, gray = 0;
    int histogram[256];
    int LUT[256], value;
    double scale_factor;
    int CUTMIN_PIXELNUM	= (int)(D_CUT_MINTRES * (ZOOM_H * ZOOM_W));
    int CUTMAX_PIXELNUM	= (int)(D_CUT_MAXTRES * (ZOOM_H * ZOOM_W));

    memset(histogram, 0, sizeof(histogram));
    memset(LUT, 0, 256 * sizeof(int));
    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            gray = ImageBuf[y * ZOOM_ENLAGE_W + x];
            histogram[gray]++;
        }
    }

    int minsum = 0, maxsum = 0;
    int MIN_TRES = 0, MAX_TRES = 255;
    for (y = 0; y < 256; y++)
    {
        if (minsum > CUTMIN_PIXELNUM)
        {
            MIN_TRES = y;
            break;
        }
        else if (histogram[y] > 0)
        {
            minsum += histogram[y];
        }
        LUT[y] = 0;
    }
    for (y = 255; y >= MIN_TRES; y--)
    {
        if (maxsum > CUTMAX_PIXELNUM)
        {
            MAX_TRES = y;
            break;
        }
        else if (histogram[y] > 0)
        {
            maxsum += histogram[y];
        }
        LUT[y] = 255;
    }

    //스트레칭의 강도를 조절할 수 있는 factor 계산
    if (MAX_TRES == MIN_TRES)
    {
        scale_factor = 0;
    }
    else
    {
        scale_factor = 255.0 / (MAX_TRES - MIN_TRES);
    }

    for (y = MIN_TRES; y <= MAX_TRES; y++)
    {
        value = (int)((y - MIN_TRES) * scale_factor);
        if (value > 255)
        {
            value = 255;
        }
        LUT[y] = value;
    }

    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            ImageBuf[y * ZOOM_ENLAGE_W + x] = LUT[ImageBuf[y * ZOOM_ENLAGE_W + x]];
        }
    }
}

void CIBAlgorithm::_Algo_HistogramStretchForZoomEnlarge_Kojak_Roll(unsigned char *ImageBuf)
{
    const double D_CUT_MINTRES = 0.001;
    const double D_CUT_MAXTRES =	0.001;

    int y, x, gray = 0;
    int histogram[256];
    int LUT[256], value;
    double scale_factor;
    int CUTMIN_PIXELNUM	= (int)(D_CUT_MINTRES * (ZOOM_H_ROLL * ZOOM_W_ROLL));
    int CUTMAX_PIXELNUM	= (int)(D_CUT_MAXTRES * (ZOOM_H_ROLL * ZOOM_W_ROLL));

    memset(histogram, 0, sizeof(histogram));
    memset(LUT, 0, 256 * sizeof(int));
    for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
    {
        for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
        {
            gray = ImageBuf[y * ZOOM_ENLAGE_W_ROLL + x];
            histogram[gray]++;
        }
    }

    int minsum = 0, maxsum = 0;
    int MIN_TRES = 0, MAX_TRES = 255;
    for (y = 0; y < 256; y++)
    {
        if (minsum > CUTMIN_PIXELNUM)
        {
            MIN_TRES = y;
            break;
        }
        else if (histogram[y] > 0)
        {
            minsum += histogram[y];
        }
        LUT[y] = 0;
    }
    for (y = 255; y >= MIN_TRES; y--)
    {
        if (maxsum > CUTMAX_PIXELNUM)
        {
            MAX_TRES = y;
            break;
        }
        else if (histogram[y] > 0)
        {
            maxsum += histogram[y];
        }
        LUT[y] = 255;
    }

    //스트레칭의 강도를 조절할 수 있는 factor 계산
    if (MAX_TRES == MIN_TRES)
    {
        scale_factor = 0;
    }
    else
    {
        scale_factor = 255.0 / (MAX_TRES - MIN_TRES);
    }

    for (y = MIN_TRES; y <= MAX_TRES; y++)
    {
        value = (int)((y - MIN_TRES) * scale_factor);
        if (value > 255)
        {
            value = 255;
        }
        LUT[y] = value;
    }

    for (y = ENLARGESIZE_ZOOM_H_ROLL; y < ZOOM_ENLAGE_H_ROLL - ENLARGESIZE_ZOOM_H_ROLL; y++)
    {
        for (x = ENLARGESIZE_ZOOM_W_ROLL; x < ZOOM_ENLAGE_W_ROLL - ENLARGESIZE_ZOOM_W_ROLL; x++)
        {
            ImageBuf[y * ZOOM_ENLAGE_W_ROLL + x] = LUT[ImageBuf[y * ZOOM_ENLAGE_W_ROLL + x]];
        }
    }
}

int CIBAlgorithm::_Algo_GetBrightWithRawImage_forDetectOnly(BYTE *InRawImg, int ForgraoundCNT)
{
    int y, x, i, j;
    int ii, tempsum;
    int xx, yy, forgroundCNT = 0;
    int value;
//    int BrightValue;

    //gettimeofday(&start_tv, NULL);
    memset(m_segment_enlarge_buffer, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
    memset(m_segment_enlarge_buffer_for_fingercnt, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
    memset(EnlargeBuf, 0, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);

    m_cImgAnalysis.LEFT = 1;
    m_cImgAnalysis.RIGHT = 1;
    m_cImgAnalysis.TOP = 1;
    m_cImgAnalysis.BOTTOM = 1;

    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        yy = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H * CIS_IMG_W;

        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            xx = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W;

            value = InRawImg[yy + xx];
            value = (((int)value * m_UM_Watson_F[yy + xx]) >> 10) + (int)value;
            if (value > 255)
            {
                value = 255;
            }
            else if (value < 0)
            {
                value = 0;
            }
            EnlargeBuf[(y)*ZOOM_ENLAGE_W + (x)] = value;
        }
    }

    memcpy(EnlargeBuf_Org, EnlargeBuf, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);

    _Algo_HistogramStretchForZoomEnlarge(EnlargeBuf);

    // for finger count
    for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
    {
        for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
        {
            tempsum  = EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W + (j + 1)];
            tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j + 1)];
            tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W + (j + 1)];

            tempsum /= 9;

            // fixed bug.
            if (tempsum >= 1) //Threshold) //mean/20)
            {
                m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W + j] = 255;
            }
        }
    }

    for (ii = 0; ii < 1; ii++)
    {
        memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
        for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
        {
            for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
            {
                if (m_segment_enlarge_buffer[i * ZOOM_ENLAGE_W + j] == 255)
                {
                    continue;
                }

                tempsum  = m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W + j  ];
                tempsum += m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W + j + 1];
                tempsum += m_segment_enlarge_buffer[(i) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer[(i) * ZOOM_ENLAGE_W + j + 1];
                tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W + j  ];
                tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W + j + 1];

                if (tempsum >= 4 * 255)
                {
                    m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W + j] = 255;
                }
            }
        }
    }

    memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);

    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W + x - 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W + x] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W + x + 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x - 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x + 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W + x - 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W + x] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W + x + 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 255;
            }

            if (m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] == 255)
            {
                forgroundCNT++;
            }
        }
    }

    if (forgroundCNT >= ForgraoundCNT)
        return 1;

    return 0;
}

int CIBAlgorithm::_Algo_GetBrightWithRawImage(BYTE *InRawImg, int *ForgraoundCNT, int *CenterX, int *CenterY)
{
    //	struct timeval tv;
    //	struct timeval start_tv;
    //	double elapsed = 0.0;

    int y, x, i, j;
    int mean = 0, count = 0;
    int ii, tempsum;
    int xx, yy, forgroundCNT = 0;
    int value;
    int sum_x = 0, sum_y = 0;
    int BrightValue;

    //gettimeofday(&start_tv, NULL);
    memset(m_segment_enlarge_buffer, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
    memset(m_segment_enlarge_buffer_for_fingercnt, 0, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
    memset(EnlargeBuf, 0, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);
    *CenterX = -1;
    *CenterY = -1;
    *ForgraoundCNT = 0;

    m_cImgAnalysis.LEFT = 1;
    m_cImgAnalysis.RIGHT = 1;
    m_cImgAnalysis.TOP = 1;
    m_cImgAnalysis.BOTTOM = 1;

    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        yy = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H * CIS_IMG_W;

        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            xx = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W;

            value = InRawImg[yy + xx];
            value = (((int)value * m_UM_Watson_F[yy + xx]) >> 10) + (int)value;
            if (value > 255)
            {
                value = 255;
            }
            else if (value < 0)
            {
                value = 0;
            }
            EnlargeBuf[(y)*ZOOM_ENLAGE_W + (x)] = value;
        }
    }

    memcpy(EnlargeBuf_Org, EnlargeBuf, ZOOM_ENLAGE_H * ZOOM_ENLAGE_W);

    _Algo_HistogramStretchForZoomEnlarge(EnlargeBuf);

    mean = 0;
    count = 0;
    for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
    {
        for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
        {
            if (EnlargeBuf[(i)*ZOOM_ENLAGE_W + (j)] > 10)
            {
                mean += EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j)];
                count++;
            }
        }
    }

    if (count == 0)
    {
        //gettimeofday(&tv, NULL);
        //elapsed = (tv.tv_sec - start_tv.tv_sec) + (tv.tv_usec - start_tv.tv_usec) / 1000.0;
        //printf("_Algo_GetBrightWithRawImage(): %1.4f milliseconds\n", elapsed);
        return 0;
    }

    mean /= count;

    /*	int Threshold = mean/20;
    	if(Threshold < 5)
    		Threshold = 5;
    */
    int Threshold = (int)(pow((double)mean / 255.0, 3.0) * 255);
    if (Threshold < 5)
    {
        Threshold = 5;
    }
    else if (Threshold > 250)
    {
        Threshold = 250;
    }

    // for finger count
    for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
    {
        for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
        {
            tempsum  = EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf[(i - 1) * ZOOM_ENLAGE_W + (j + 1)];
            tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf[(i) * ZOOM_ENLAGE_W + (j + 1)];
            tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W + (j - 1)];
            tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W + (j)];
            tempsum += EnlargeBuf[(i + 1) * ZOOM_ENLAGE_W + (j + 1)];

            tempsum /= 9;

            // fixed bug.
            if (tempsum >= Threshold) //mean/20)
            {
                m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W + j] = 255;
            }
        }
    }

    for (ii = 0; ii < 1; ii++)
    {
        memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);
        for (i = ENLARGESIZE_ZOOM_H; i < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; i++)
        {
            for (j = ENLARGESIZE_ZOOM_W; j < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; j++)
            {
                if (m_segment_enlarge_buffer[i * ZOOM_ENLAGE_W + j] == 255)
                {
                    continue;
                }

                tempsum  = m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W + j  ];
                tempsum += m_segment_enlarge_buffer[(i - 1) * ZOOM_ENLAGE_W + j + 1];
                tempsum += m_segment_enlarge_buffer[(i) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer[(i) * ZOOM_ENLAGE_W + j + 1];
                tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W + j - 1];
                tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W + j  ];
                tempsum += m_segment_enlarge_buffer[(i + 1) * ZOOM_ENLAGE_W + j + 1];

                if (tempsum >= 4 * 255)
                {
                    m_segment_enlarge_buffer_for_fingercnt[i * ZOOM_ENLAGE_W + j] = 255;
                }
            }
        }
    }

    memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);

    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W + x - 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W + x] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[(y - 1)*ZOOM_ENLAGE_W + x + 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x - 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x + 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W + x - 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W + x] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else if (m_segment_enlarge_buffer[(y + 1)*ZOOM_ENLAGE_W + x + 1] == 0)
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 0;
            }
            else
            {
                m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] = 255;
            }

            if (m_segment_enlarge_buffer_for_fingercnt[y * ZOOM_ENLAGE_W + x] == 255)
            {
                forgroundCNT++;
            }
        }
    }

    *ForgraoundCNT = forgroundCNT;

    if (forgroundCNT < 100)
    {
        return 0;
    }

    memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, ZOOM_ENLAGE_W * ZOOM_ENLAGE_H);

    /*	mean=0;
    	count=0;
    	for (y=ENLARGESIZE_ZOOM_H; y<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; y++)
    	{
    		for (x=ENLARGESIZE_ZOOM_W; x<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; x++)
    		{
    			if(m_segment_enlarge_buffer[y*ZOOM_ENLAGE_W+x]==0)
    				continue;

    			mean += EnlargeBuf[(y)*ZOOM_ENLAGE_W+(x)];
    			count++;
    			sum_x += x*ZOOM_OUT;
    			sum_y += y*ZOOM_OUT;
    		}
    	}
    //gettimeofday(&tv, NULL);
    //elapsed = (tv.tv_sec - start_tv.tv_sec) + (tv.tv_usec - start_tv.tv_usec) / 1000.0;
    //printf("_Algo_GetBrightWithRawImage(): %1.4f milliseconds\n", elapsed);
    */
    //	int Threshold = mean/20;
    //	//if(Threshold == 0)
    //		Threshold = 1;

    mean = 0;
    count = 0;
    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            if (m_segment_enlarge_buffer[y * ZOOM_ENLAGE_W + x] == 0)
            {
                continue;
            }

            if (EnlargeBuf_Org[(y)*ZOOM_ENLAGE_W + (x)] >= Threshold)
            {
                mean += EnlargeBuf_Org[(y) * ZOOM_ENLAGE_W + (x)];
                count++;
                sum_x += x * ZOOM_OUT;
                sum_y += y * ZOOM_OUT;
            }
        }
    }
    if (count == 0)
    {
        return 0;
    }

    mean /= count;
    *CenterX = sum_x / count;
    *CenterY = sum_y / count;

    BrightValue = mean;


    /////////////////////////////////////////////////////////////////////////////////////
    // find calc rect
    int COUNT;
    for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
    {
        COUNT = 0;
        for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
        {
            //			if(m_segment_enlarge_buffer[y*ZOOM_ENLAGE_W+x]==255)
            if (EnlargeBuf[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            m_cImgAnalysis.LEFT = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W - 30;
            //			m_cImgAnalysis.LEFT=(x-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W - 16;
            if (m_cImgAnalysis.LEFT < 1)
            {
                m_cImgAnalysis.LEFT = 1;
            }
            break;
        }
    }

    for (x = ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x >= ENLARGESIZE_ZOOM_W; x--)
    {
        COUNT = 0;
        for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
        {
            //			if(m_segment_enlarge_buffer[y*ZOOM_ENLAGE_W+x]==255)
            if (EnlargeBuf[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            m_cImgAnalysis.RIGHT = (x - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W + 30;
            //			m_cImgAnalysis.RIGHT=(x-ENLARGESIZE_ZOOM_W)* CIS_IMG_W / ZOOM_W + 16;
            if (m_cImgAnalysis.RIGHT > CIS_IMG_W - 2)
            {
                m_cImgAnalysis.RIGHT = CIS_IMG_W - 2;
            }
            break;
        }
    }

    for (y = ENLARGESIZE_ZOOM_H; y < ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y++)
    {
        COUNT = 0;
        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            //			if(m_segment_enlarge_buffer[y*ZOOM_ENLAGE_W+x]==255)
            if (EnlargeBuf[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            m_cImgAnalysis.TOP = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W - 30;
            //			m_cImgAnalysis.TOP=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H - 16;
            if (m_cImgAnalysis.TOP < 1)
            {
                m_cImgAnalysis.TOP = 1;
            }
            break;
        }
    }

    for (y = ZOOM_ENLAGE_H - ENLARGESIZE_ZOOM_H; y > ENLARGESIZE_ZOOM_H; y--)
    {
        COUNT = 0;
        for (x = ENLARGESIZE_ZOOM_W; x < ZOOM_ENLAGE_W - ENLARGESIZE_ZOOM_W; x++)
        {
            //			if(m_segment_enlarge_buffer[y*ZOOM_ENLAGE_W+x]==255)
            if (EnlargeBuf[y * ZOOM_ENLAGE_W + x] >= Threshold)
            {
                COUNT++;
            }
        }

        if (COUNT > 5)
        {
            m_cImgAnalysis.BOTTOM = (y - ENLARGESIZE_ZOOM_H) * CIS_IMG_W / ZOOM_W + 30;
            //			m_cImgAnalysis.BOTTOM=(y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H + 16;
            if (m_cImgAnalysis.BOTTOM > CIS_IMG_H - 2)
            {
                m_cImgAnalysis.BOTTOM = CIS_IMG_H - 2;
            }
            break;
        }
    }

    return BrightValue;
}

int CIBAlgorithm::_Algo_GetFingerCount(int ImageBright)
{
	int OldSegmentCnt;
	memset(&m_segment_arr, 0, sizeof(NEW_SEGMENT_ARRAY));
	memset(&m_segment_arr_90, 0, sizeof(NEW_SEGMENT_ARRAY));

	/*if(m_pPropertyInfo->bEnableCBPMode == TRUE)
	{
		memset(&m_CBPPreviewInfo, 0, sizeof(CBPPreviewInfo));

		if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
		{
			_Algo_ConvertSegInfoToFinal(m_Inter_Img, m_Inter_Img2);	
			
			if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			{
				memcpy(m_segment_enlarge_buffer_for_fingercnt, m_Inter_Img, ZOOM_ENLAGE_W_ROLL*ZOOM_ENLAGE_H_ROLL);
				memcpy(EnlargeBuf, m_Inter_Img2, ZOOM_ENLAGE_W_ROLL*ZOOM_ENLAGE_H_ROLL);
			}
			else
			{
				memcpy(m_segment_enlarge_buffer_for_fingercnt, m_Inter_Img, ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);
				memcpy(EnlargeBuf, m_Inter_Img2, ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);
			}
		}
	}*/

	OldSegmentCnt = _Algo_GetSegmentInfo(&m_segment_arr, ImageBright);
	
	// 1. not need to segment fingers
	if(OldSegmentCnt <= 1 && m_pPropertyInfo->bEnableCBPMode == FALSE)
		return OldSegmentCnt;

	// 2. calc main direction of fingers
//	int old_main_dir=0;
	_Algo_GetMainDirectionOfFinger();
//	old_main_dir = m_segment_arr.VCenterSegment.Ang;

	// 4. sort segments descending order from top to bottom
	_Algo_SortSegmentByMainDir();

	// 5. merge segment if same finger
	_Algo_MergeSegmentIfSameFinger();

	if(m_pPropertyInfo->bEnableCBPMode == TRUE)
	{
		// to calculate 90
		int x1, x2, x3, x4, y1, y2, y3, y4;
		memset(m_CBPPreviewInfo.SegPosArr_90, 0, sizeof(IBSU_SegmentPosition)*IBSU_MAX_SEGMENT_COUNT);
		memcpy(&m_segment_arr_90, &m_segment_arr, sizeof(NEW_SEGMENT_ARRAY));

		if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
			NEW_SEGMENT new_seg;
			for(int gg=0; gg<m_segment_arr_90.SegmentCnt-1; gg++)
			{
				for(int tt=gg+1; tt<m_segment_arr_90.SegmentCnt; tt++)
				{
					if(m_segment_arr_90.Segment[gg].CenterY < m_segment_arr_90.Segment[tt].CenterY)
					{
						new_seg = m_segment_arr_90.Segment[gg];
						m_segment_arr_90.Segment[gg] = m_segment_arr_90.Segment[tt];
						m_segment_arr_90.Segment[tt] = new_seg;
					}
				}
			}
			if(m_segment_arr_90.SegmentCnt > 1)
				m_segment_arr_90.SegmentCnt = 1;

			for(int i=0; i<ZOOM_ENLAGE_H_ROLL; i++)
			{
				for(int j=0; j<ZOOM_ENLAGE_W_ROLL; j++)
				{
					if(m_segment_enlarge_buffer_for_fingercnt[i*ZOOM_ENLAGE_W_ROLL+j] > 0)
					{
						if(m_segment_arr_90.Segment[0].P1_X < j)
							m_segment_arr_90.Segment[0].P1_X = j;
						if(m_segment_arr_90.Segment[0].P4_X < j)
							m_segment_arr_90.Segment[0].P4_X = j;

						if(m_segment_arr_90.Segment[0].P2_X > j)
							m_segment_arr_90.Segment[0].P2_X = j;
						if(m_segment_arr_90.Segment[0].P3_X > j)
							m_segment_arr_90.Segment[0].P3_X = j;

						if(m_segment_arr_90.Segment[0].P1_Y < i)
							m_segment_arr.Segment[0].P1_Y = i;
						if(m_segment_arr_90.Segment[0].P2_Y < i)
							m_segment_arr_90.Segment[0].P2_Y = i;

						if(m_segment_arr_90.Segment[0].P3_Y > i)
							m_segment_arr_90.Segment[0].P3_Y = i;
						if(m_segment_arr_90.Segment[0].P4_Y > i)
							m_segment_arr_90.Segment[0].P4_Y = i;
					}
				}
			}
		}

		for(int i=0; i<m_segment_arr_90.SegmentCnt; i++)
		{
			m_segment_arr_90.Segment[i].Ang = 90;
		}

		// 6. search exact position of segment
		_Algo_SearchFingerTipPosition_90();

		NEW_SEGMENT_ARRAY tmpArray;
		memcpy(&tmpArray, &m_segment_arr_90, sizeof(tmpArray));

		double cs_radian = cos((m_segment_arr_90.VCenterSegment.Ang-90)*3.14159265358979/180.0);
		double sn_radian = sin((m_segment_arr_90.VCenterSegment.Ang-90)*3.14159265358979/180.0);

		for(int u=0; u<m_segment_arr_90.SegmentCnt; u++)
		{
			tmpArray.Segment[u].P1_X = (int) ( (double)(m_segment_arr_90.Segment[u].P1_X-m_segment_arr_90.VCenterSegment.X)*cs_radian + 
											   (double)(m_segment_arr_90.Segment[u].P1_Y-m_segment_arr_90.VCenterSegment.Y)*sn_radian ) + m_segment_arr_90.VCenterSegment.X;
			tmpArray.Segment[u].P1_Y = (int) ( -(double)(m_segment_arr_90.Segment[u].P1_X-m_segment_arr_90.VCenterSegment.X)*sn_radian + 
											   (double)(m_segment_arr_90.Segment[u].P1_Y-m_segment_arr_90.VCenterSegment.Y)*cs_radian ) + m_segment_arr_90.VCenterSegment.Y;

			tmpArray.Segment[u].P2_X = (int) ( (double)(m_segment_arr_90.Segment[u].P2_X-m_segment_arr_90.VCenterSegment.X)*cs_radian + 
											   (double)(m_segment_arr_90.Segment[u].P2_Y-m_segment_arr_90.VCenterSegment.Y)*sn_radian ) + m_segment_arr_90.VCenterSegment.X;
			tmpArray.Segment[u].P2_Y = (int) ( -(double)(m_segment_arr_90.Segment[u].P2_X-m_segment_arr_90.VCenterSegment.X)*sn_radian + 
											   (double)(m_segment_arr_90.Segment[u].P2_Y-m_segment_arr_90.VCenterSegment.Y)*cs_radian ) + m_segment_arr_90.VCenterSegment.Y;

			tmpArray.Segment[u].P3_X = (int) ( (double)(m_segment_arr_90.Segment[u].P3_X-m_segment_arr_90.VCenterSegment.X)*cs_radian + 
											   (double)(m_segment_arr_90.Segment[u].P3_Y-m_segment_arr_90.VCenterSegment.Y)*sn_radian ) + m_segment_arr_90.VCenterSegment.X;
			tmpArray.Segment[u].P3_Y = (int) ( -(double)(m_segment_arr_90.Segment[u].P3_X-m_segment_arr_90.VCenterSegment.X)*sn_radian + 
											   (double)(m_segment_arr_90.Segment[u].P3_Y-m_segment_arr_90.VCenterSegment.Y)*cs_radian ) + m_segment_arr_90.VCenterSegment.Y;

			tmpArray.Segment[u].P4_X = (int) ( (double)(m_segment_arr_90.Segment[u].P4_X-m_segment_arr_90.VCenterSegment.X)*cs_radian + 
											   (double)(m_segment_arr_90.Segment[u].P4_Y-m_segment_arr_90.VCenterSegment.Y)*sn_radian ) + m_segment_arr_90.VCenterSegment.X;
			tmpArray.Segment[u].P4_Y = (int) ( -(double)(m_segment_arr_90.Segment[u].P4_X-m_segment_arr_90.VCenterSegment.X)*sn_radian + 
											   (double)(m_segment_arr_90.Segment[u].P4_Y-m_segment_arr_90.VCenterSegment.Y)*cs_radian ) + m_segment_arr_90.VCenterSegment.Y;

			tmpArray.Segment[u].CenterX = (tmpArray.Segment[u].P1_X + tmpArray.Segment[u].P2_X + 
											tmpArray.Segment[u].P3_X + tmpArray.Segment[u].P4_X)/4;
			tmpArray.Segment[u].CenterY = (tmpArray.Segment[u].P1_Y + tmpArray.Segment[u].P2_Y + 
											tmpArray.Segment[u].P3_Y + tmpArray.Segment[u].P4_Y)/4;
		}

		NEW_SEGMENT new_seg;
		for(int gg=0; gg<tmpArray.SegmentCnt-1; gg++)
		{
			for(int tt=gg+1; tt<tmpArray.SegmentCnt; tt++)
			{
				if(tmpArray.Segment[gg].CenterX > tmpArray.Segment[tt].CenterX)
				{
					new_seg = tmpArray.Segment[gg];
					tmpArray.Segment[gg] = tmpArray.Segment[tt];
					tmpArray.Segment[tt] = new_seg;

					new_seg = m_segment_arr_90.Segment[gg];
					m_segment_arr_90.Segment[gg] = m_segment_arr_90.Segment[tt];
					m_segment_arr_90.Segment[tt] = new_seg;
				}
			}
		}
		m_CBPPreviewInfo.SegPosArrCount = m_segment_arr_90.SegmentCnt;
		if(m_CBPPreviewInfo.SegPosArrCount > IBSU_MAX_SEGMENT_COUNT)
			m_CBPPreviewInfo.SegPosArrCount = IBSU_MAX_SEGMENT_COUNT;

		if( m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
		{
			for(int i=0; i<m_CBPPreviewInfo.SegPosArrCount; i++)
			{
				x1 = (m_segment_arr_90.Segment[i].P1_X-ENLARGESIZE_ZOOM_W_ROLL) * IMG_W_ROLL / ZOOM_W_ROLL;
				y1 = (m_segment_arr_90.Segment[i].P1_Y-ENLARGESIZE_ZOOM_H_ROLL) * IMG_H_ROLL / ZOOM_H_ROLL;

				x2 = (m_segment_arr_90.Segment[i].P2_X-ENLARGESIZE_ZOOM_W_ROLL) * IMG_W_ROLL / ZOOM_W_ROLL;
				y2 = (m_segment_arr_90.Segment[i].P2_Y-ENLARGESIZE_ZOOM_H_ROLL) * IMG_H_ROLL / ZOOM_H_ROLL;

				x3 = (m_segment_arr_90.Segment[i].P3_X-ENLARGESIZE_ZOOM_W_ROLL) * IMG_W_ROLL / ZOOM_W_ROLL;
				y3 = (m_segment_arr_90.Segment[i].P3_Y-ENLARGESIZE_ZOOM_H_ROLL) * IMG_H_ROLL / ZOOM_H_ROLL;

				x4 = (m_segment_arr_90.Segment[i].P4_X-ENLARGESIZE_ZOOM_W_ROLL) * IMG_W_ROLL / ZOOM_W_ROLL;
				y4 = (m_segment_arr_90.Segment[i].P4_Y-ENLARGESIZE_ZOOM_H_ROLL) * IMG_H_ROLL / ZOOM_H_ROLL;

				m_CBPPreviewInfo.SegPosArr_90[i].x1 = x1 + 16;
				m_CBPPreviewInfo.SegPosArr_90[i].x2 = x2 - 16;
				m_CBPPreviewInfo.SegPosArr_90[i].x3 = x3 - 16;
				m_CBPPreviewInfo.SegPosArr_90[i].x4 = x4 + 16;
				m_CBPPreviewInfo.SegPosArr_90[i].y1 = (IMG_H_ROLL - y1 - 16);
				m_CBPPreviewInfo.SegPosArr_90[i].y2 = (IMG_H_ROLL - y2 - 16);
				m_CBPPreviewInfo.SegPosArr_90[i].y3 = (IMG_H_ROLL - y3 + 16);
				m_CBPPreviewInfo.SegPosArr_90[i].y4 = (IMG_H_ROLL - y4 + 16);
			}
		}
		else
		{
			int new_x1, new_y1, new_x2, new_y2, new_x3, new_y3, new_x4, new_y4;

			for(int i=0; i<m_CBPPreviewInfo.SegPosArrCount; i++)
			{
				x1 = (m_segment_arr_90.Segment[i].P1_X-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W;
				y1 = (m_segment_arr_90.Segment[i].P1_Y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H;

				x2 = (m_segment_arr_90.Segment[i].P2_X-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W;
				y2 = (m_segment_arr_90.Segment[i].P2_Y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H;

				x3 = (m_segment_arr_90.Segment[i].P3_X-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W;
				y3 = (m_segment_arr_90.Segment[i].P3_Y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H;

				x4 = (m_segment_arr_90.Segment[i].P4_X-ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W;
				y4 = (m_segment_arr_90.Segment[i].P4_Y-ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H;

				_Algo_GetFixedDistortionPos(x1, y1, &new_x1, &new_y1);
				_Algo_GetFixedDistortionPos(x2, y2, &new_x2, &new_y2);
				_Algo_GetFixedDistortionPos(x3, y3, &new_x3, &new_y3);
				_Algo_GetFixedDistortionPos(x4, y4, &new_x4, &new_y4);

				m_CBPPreviewInfo.SegPosArr_90[i].x1 = new_x1 + 20;
				m_CBPPreviewInfo.SegPosArr_90[i].x2 = new_x2 - 20;
				m_CBPPreviewInfo.SegPosArr_90[i].x3 = new_x3 - 20;
				m_CBPPreviewInfo.SegPosArr_90[i].x4 = new_x4 + 20;
				m_CBPPreviewInfo.SegPosArr_90[i].y1 = (IMG_H - new_y1 - 16);
				m_CBPPreviewInfo.SegPosArr_90[i].y2 = (IMG_H - new_y2 - 16);
				m_CBPPreviewInfo.SegPosArr_90[i].y3 = (IMG_H - new_y3 + 16);
				m_CBPPreviewInfo.SegPosArr_90[i].y4 = (IMG_H - new_y4 + 16);
			}
		}
	}

	if(m_pPropertyInfo->bEnableCBPMode == TRUE)
	{
		int x1, x2, x3, x4, y1, y2, y3, y4;
		memset(m_CBPPreviewInfo.SegPosArr, 0, sizeof(IBSU_SegmentPosition)*IBSU_MAX_SEGMENT_COUNT);

		if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
			NEW_SEGMENT new_seg;
			for(int gg=0; gg<m_segment_arr.SegmentCnt-1; gg++)
			{
				for(int tt=gg+1; tt<m_segment_arr.SegmentCnt; tt++)
				{
					if(m_segment_arr.Segment[gg].CenterY < m_segment_arr.Segment[tt].CenterY)
					{
						new_seg = m_segment_arr.Segment[gg];
						m_segment_arr.Segment[gg] = m_segment_arr.Segment[tt];
						m_segment_arr.Segment[tt] = new_seg;
					}
				}
			}
			if(m_segment_arr.SegmentCnt > 1)
				m_segment_arr.SegmentCnt = 1;

			for(int i=0; i<ZOOM_ENLAGE_H_ROLL; i++)
			{
				for(int j=0; j<ZOOM_ENLAGE_W_ROLL; j++)
				{
					if(m_segment_enlarge_buffer_for_fingercnt[i*ZOOM_ENLAGE_W_ROLL+j] > 0)
					{
						if(m_segment_arr.Segment[0].P1_X < j)
							m_segment_arr.Segment[0].P1_X = j;
						if(m_segment_arr.Segment[0].P4_X < j)
							m_segment_arr.Segment[0].P4_X = j;

						if(m_segment_arr.Segment[0].P2_X > j)
							m_segment_arr.Segment[0].P2_X = j;
						if(m_segment_arr.Segment[0].P3_X > j)
							m_segment_arr.Segment[0].P3_X = j;

						if(m_segment_arr.Segment[0].P1_Y < i)
							m_segment_arr.Segment[0].P1_Y = i;
						if(m_segment_arr.Segment[0].P2_Y < i)
							m_segment_arr.Segment[0].P2_Y = i;

						if(m_segment_arr.Segment[0].P3_Y > i)
							m_segment_arr.Segment[0].P3_Y = i;
						if(m_segment_arr.Segment[0].P4_Y > i)
							m_segment_arr.Segment[0].P4_Y = i;
					}
				}
			}
		}

		/*for(int i=0; i<m_segment_arr.SegmentCnt; i++)
		{
			m_segment_arr.Segment[i].Ang = 90;
		}*/

		// 6. search exact position of segment
		_Algo_SearchFingerTipPosition();

		NEW_SEGMENT_ARRAY tmpArray;
		memcpy(&tmpArray, &m_segment_arr, sizeof(tmpArray));

		double cs_radian = cos((m_segment_arr.VCenterSegment.Ang-90)*3.14159265358979/180.0);
		double sn_radian = sin((m_segment_arr.VCenterSegment.Ang-90)*3.14159265358979/180.0);

		for(int u=0; u<m_segment_arr.SegmentCnt; u++)
		{
			tmpArray.Segment[u].P1_X = (int) ( (double)(m_segment_arr.Segment[u].P1_X-m_segment_arr.VCenterSegment.X)*cs_radian + 
											   (double)(m_segment_arr.Segment[u].P1_Y-m_segment_arr.VCenterSegment.Y)*sn_radian ) + m_segment_arr.VCenterSegment.X;
			tmpArray.Segment[u].P1_Y = (int) ( -(double)(m_segment_arr.Segment[u].P1_X-m_segment_arr.VCenterSegment.X)*sn_radian + 
											   (double)(m_segment_arr.Segment[u].P1_Y-m_segment_arr.VCenterSegment.Y)*cs_radian ) + m_segment_arr.VCenterSegment.Y;

			tmpArray.Segment[u].P2_X = (int) ( (double)(m_segment_arr.Segment[u].P2_X-m_segment_arr.VCenterSegment.X)*cs_radian + 
											   (double)(m_segment_arr.Segment[u].P2_Y-m_segment_arr.VCenterSegment.Y)*sn_radian ) + m_segment_arr.VCenterSegment.X;
			tmpArray.Segment[u].P2_Y = (int) ( -(double)(m_segment_arr.Segment[u].P2_X-m_segment_arr.VCenterSegment.X)*sn_radian + 
											   (double)(m_segment_arr.Segment[u].P2_Y-m_segment_arr.VCenterSegment.Y)*cs_radian ) + m_segment_arr.VCenterSegment.Y;

			tmpArray.Segment[u].P3_X = (int) ( (double)(m_segment_arr.Segment[u].P3_X-m_segment_arr.VCenterSegment.X)*cs_radian + 
											   (double)(m_segment_arr.Segment[u].P3_Y-m_segment_arr.VCenterSegment.Y)*sn_radian ) + m_segment_arr.VCenterSegment.X;
			tmpArray.Segment[u].P3_Y = (int) ( -(double)(m_segment_arr.Segment[u].P3_X-m_segment_arr.VCenterSegment.X)*sn_radian + 
											   (double)(m_segment_arr.Segment[u].P3_Y-m_segment_arr.VCenterSegment.Y)*cs_radian ) + m_segment_arr.VCenterSegment.Y;

			tmpArray.Segment[u].P4_X = (int) ( (double)(m_segment_arr.Segment[u].P4_X-m_segment_arr.VCenterSegment.X)*cs_radian + 
											   (double)(m_segment_arr.Segment[u].P4_Y-m_segment_arr.VCenterSegment.Y)*sn_radian ) + m_segment_arr.VCenterSegment.X;
			tmpArray.Segment[u].P4_Y = (int) ( -(double)(m_segment_arr.Segment[u].P4_X-m_segment_arr.VCenterSegment.X)*sn_radian + 
											   (double)(m_segment_arr.Segment[u].P4_Y-m_segment_arr.VCenterSegment.Y)*cs_radian ) + m_segment_arr.VCenterSegment.Y;

			tmpArray.Segment[u].CenterX = (tmpArray.Segment[u].P1_X + tmpArray.Segment[u].P2_X + 
											tmpArray.Segment[u].P3_X + tmpArray.Segment[u].P4_X)/4;
			tmpArray.Segment[u].CenterY = (tmpArray.Segment[u].P1_Y + tmpArray.Segment[u].P2_Y + 
											tmpArray.Segment[u].P3_Y + tmpArray.Segment[u].P4_Y)/4;
		}

		NEW_SEGMENT new_seg;
		for(int gg=0; gg<tmpArray.SegmentCnt-1; gg++)
		{
			for(int tt=gg+1; tt<tmpArray.SegmentCnt; tt++)
			{
				if(tmpArray.Segment[gg].CenterX > tmpArray.Segment[tt].CenterX)
				{
					new_seg = tmpArray.Segment[gg];
					tmpArray.Segment[gg] = tmpArray.Segment[tt];
					tmpArray.Segment[tt] = new_seg;

					new_seg = m_segment_arr.Segment[gg];
					m_segment_arr.Segment[gg] = m_segment_arr.Segment[tt];
					m_segment_arr.Segment[tt] = new_seg;
				}
			}
		}
		m_CBPPreviewInfo.SegPosArrCount = m_segment_arr.SegmentCnt;
		if(m_CBPPreviewInfo.SegPosArrCount > IBSU_MAX_SEGMENT_COUNT)
			m_CBPPreviewInfo.SegPosArrCount = IBSU_MAX_SEGMENT_COUNT;

		if( m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
		{
			for(int i=0; i<m_CBPPreviewInfo.SegPosArrCount; i++)
			{
				x1 = (m_segment_arr.Segment[i].P1_X-ENLARGESIZE_ZOOM_W_ROLL) * IMG_W_ROLL / ZOOM_W_ROLL;
				y1 = (m_segment_arr.Segment[i].P1_Y-ENLARGESIZE_ZOOM_H_ROLL) * IMG_H_ROLL / ZOOM_H_ROLL;

				x2 = (m_segment_arr.Segment[i].P2_X-ENLARGESIZE_ZOOM_W_ROLL) * IMG_W_ROLL / ZOOM_W_ROLL;
				y2 = (m_segment_arr.Segment[i].P2_Y-ENLARGESIZE_ZOOM_H_ROLL) * IMG_H_ROLL / ZOOM_H_ROLL;

				x3 = (m_segment_arr.Segment[i].P3_X-ENLARGESIZE_ZOOM_W_ROLL) * IMG_W_ROLL / ZOOM_W_ROLL;
				y3 = (m_segment_arr.Segment[i].P3_Y-ENLARGESIZE_ZOOM_H_ROLL) * IMG_H_ROLL / ZOOM_H_ROLL;

				x4 = (m_segment_arr.Segment[i].P4_X-ENLARGESIZE_ZOOM_W_ROLL) * IMG_W_ROLL / ZOOM_W_ROLL;
				y4 = (m_segment_arr.Segment[i].P4_Y-ENLARGESIZE_ZOOM_H_ROLL) * IMG_H_ROLL / ZOOM_H_ROLL;

				m_CBPPreviewInfo.SegPosArr[i].x1 = x1 + 16;
				m_CBPPreviewInfo.SegPosArr[i].x2 = x2 - 16;
				m_CBPPreviewInfo.SegPosArr[i].x3 = x3 - 16;
				m_CBPPreviewInfo.SegPosArr[i].x4 = x4 + 16;
				m_CBPPreviewInfo.SegPosArr[i].y1 = (IMG_H_ROLL - y1 - 16);
				m_CBPPreviewInfo.SegPosArr[i].y2 = (IMG_H_ROLL - y2 - 16);
				m_CBPPreviewInfo.SegPosArr[i].y3 = (IMG_H_ROLL - y3 + 16);
				m_CBPPreviewInfo.SegPosArr[i].y4 = (IMG_H_ROLL - y4 + 16);
			}
		}
		else
		{
			for(int i=0; i<m_CBPPreviewInfo.SegPosArrCount; i++)
			{
				x1 = (m_segment_arr.Segment[i].P1_X-ENLARGESIZE_ZOOM_W) * IMG_W / ZOOM_W;
				y1 = (m_segment_arr.Segment[i].P1_Y-ENLARGESIZE_ZOOM_H) * IMG_H / ZOOM_H;

				x2 = (m_segment_arr.Segment[i].P2_X-ENLARGESIZE_ZOOM_W) * IMG_W / ZOOM_W;
				y2 = (m_segment_arr.Segment[i].P2_Y-ENLARGESIZE_ZOOM_H) * IMG_H / ZOOM_H;

				x3 = (m_segment_arr.Segment[i].P3_X-ENLARGESIZE_ZOOM_W) * IMG_W / ZOOM_W;
				y3 = (m_segment_arr.Segment[i].P3_Y-ENLARGESIZE_ZOOM_H) * IMG_H / ZOOM_H;

				x4 = (m_segment_arr.Segment[i].P4_X-ENLARGESIZE_ZOOM_W) * IMG_W / ZOOM_W;
				y4 = (m_segment_arr.Segment[i].P4_Y-ENLARGESIZE_ZOOM_H) * IMG_H / ZOOM_H;

				m_CBPPreviewInfo.SegPosArr[i].x1 = x1 + 16;
				m_CBPPreviewInfo.SegPosArr[i].x2 = x2 - 16;
				m_CBPPreviewInfo.SegPosArr[i].x3 = x3 - 16;
				m_CBPPreviewInfo.SegPosArr[i].x4 = x4 + 16;
				m_CBPPreviewInfo.SegPosArr[i].y1 = (IMG_H - y1 - 16);
				m_CBPPreviewInfo.SegPosArr[i].y2 = (IMG_H - y2 - 16);
				m_CBPPreviewInfo.SegPosArr[i].y3 = (IMG_H - y3 + 16);
				m_CBPPreviewInfo.SegPosArr[i].y4 = (IMG_H - y4 + 16);
			}
		}
	}

	return m_segment_arr.SegmentCnt;
}

void CIBAlgorithm::_Algo_GetFixedDistortionPos(int px, int py, int *new_px, int *new_py)
{
	int i,j;
	int search_range = 50;
	int left, right, top, bottom;
	DWORD pos;
	int pos_x, pos_y;

	left = px * IMG_W / CIS_IMG_W - search_range;
	right = px * IMG_W / CIS_IMG_W + search_range;
	top = py * IMG_H / CIS_IMG_H - search_range;
	bottom = py * IMG_H / CIS_IMG_H + search_range;

	if(left < 0) left = 0;
	if(top < 0) top = 0;
	if(right > IMG_W-2) right = IMG_W-1;
	if(bottom > IMG_H-2) bottom = IMG_H-1;

	*new_px = px * IMG_W / CIS_IMG_W;
	*new_py = py * IMG_H / CIS_IMG_H;

	for(i=top; i<bottom; i++)
	{
		for(j=left; j<right; j++)
		{
			pos = (m_SBDAlg->arrPos_wb[i*IMG_W+j]>>10) & 0x3FFFFF;
			pos_x = pos % CIS_IMG_W;
			pos_y = pos / CIS_IMG_W;

			if(abs(pos_x - px) < 2 && abs(pos_y - py) < 2)
			{
				*new_px = j;
				*new_py = i;
				break;
			}
		}
	}
}

int CIBAlgorithm::_Algo_GetForegroundInfo2(SEGMENT_ARRAY *segment_arr, BYTE *backImage, const int imgWidth, const int imgHeight)
{
    int i, j, cnt;
    int sum_x, sum_y, sum_cnt;
    int cx, cy, dx, dy, dist, sum_dx, sum_dy, sum_dxdy, sum_count, SegmentCnt;
    int en_w_band = 8, en_h_band = 8;

    // foreground labeling
    SegmentCnt = 0;
    for (i = en_h_band; i < imgHeight - en_h_band; i++)
    {
        for (j = en_w_band; j < imgWidth - en_w_band; j++)
        {
            if (backImage[i * imgWidth + j] == 0)
            {
                SegmentCnt++;
                _Algo_StackRecursiveFilling(backImage, imgWidth, imgHeight, j, i, 0, SegmentCnt);
            }

            // 계산 불가
            if (SegmentCnt >= MAX_SEGMENT_COUNT)
            {
                return FALSE;
            }
        }
    }

    // calculate center position of foreground
    for (cnt = 1; cnt <= SegmentCnt; cnt++)
    {
        sum_x = 0;
        sum_y = 0;
        sum_cnt = 0;

        for (i = en_h_band; i < imgHeight - en_h_band; i++)
        {
            for (j = en_w_band; j < imgWidth - en_w_band; j++)
            {
                if (backImage[i * imgWidth + j] == cnt)
                {
                    sum_x += j;
                    sum_y += i;
                    sum_cnt++;
                }
            }
        }

        if (sum_cnt >= MIN_FOREGROUND_AREA)
        {
            segment_arr->Segment[segment_arr->SegmentCnt].X = sum_x / sum_cnt;
            segment_arr->Segment[segment_arr->SegmentCnt].Y = sum_y / sum_cnt;
            segment_arr->Segment[segment_arr->SegmentCnt].Label = cnt;
            segment_arr->Segment[segment_arr->SegmentCnt].Area = sum_cnt;
            segment_arr->SegmentCnt++;
        }
        else
        {
            for (i = en_h_band; i < imgHeight - en_h_band; i++)
            {
                for (j = en_w_band; j < imgWidth - en_w_band; j++)
                {
                    if (backImage[i * imgWidth + j] == cnt)
                    {
                        backImage[i * imgWidth + j] = 0;
                    }
                }
            }
        }
    }

    // 더 할 필요 없음
    if (segment_arr->SegmentCnt <= 1)
    {
        return segment_arr->SegmentCnt;
    }

    // calculate direction of foreground
    for (cnt = 0; cnt < segment_arr->SegmentCnt; cnt++)
    {
        cx = segment_arr->Segment[cnt].X;
        cy = segment_arr->Segment[cnt].Y;
        sum_dx = 0;
        sum_dy = 0;
        sum_dxdy = 0;
        sum_count = 0;
        for (i = en_h_band; i < imgHeight - en_h_band; i++)
        {
            for (j = en_w_band; j < imgWidth - en_w_band; j++)
            {
                if (backImage[i * imgWidth + j] == segment_arr->Segment[cnt].Label)
                {
                    dx = j - cx;
                    dy = i - cy;

                    //					dist = SQRT_Table[abs(dy)][abs(dx)];
                    dist = (int)sqrt((double)dy * dy + (double)dx * dx);

                    if (dist == 0)
                    {
                        continue;
                    }

                    dx = dx * 10 / dist;
                    dy = dy * 10 / dist;

                    sum_dx += dx * dx;
                    sum_dy += dy * dy;
                    sum_dxdy += dx * dy;
                    sum_count++;
                }
            }
        }

        segment_arr->Segment[cnt].Ang = _Algo_ATAN2_FULL(2 * sum_dxdy, (sum_dx - sum_dy)) * 180 / 512;
    }

    return segment_arr->SegmentCnt;
}

int CIBAlgorithm::_Algo_RemoveFalseForeground2(SEGMENT_ARRAY *segment_arr, BYTE *backImage, const int imgWidth, const int imgHeight)
{
    int i, j, dx, dy;
    unsigned char ValidFlag[MAX_SEGMENT_COUNT], MinDiffIdx;
    unsigned short DiffRate[MAX_SEGMENT_COUNT], MinDiffRate;
    int x, y, val_BackCount;
    int diffdir, diffdir_btw_two_dir = 0, diffdir_btw_two_pos = 0, RotY;
    double rad, cosangle, sinangle;
    unsigned char ExistMatchedPair;
    SEGMENT_ARRAY	TmpSegArr;
    const int en_h_band = 8;
    const int en_w_band = 8;

    memset(&TmpSegArr, 0, sizeof(SEGMENT_ARRAY));
    memset(ValidFlag, 0, MAX_SEGMENT_COUNT);

    for (i = 0; i < segment_arr->SegmentCnt; i++)
    {
        if (segment_arr->Segment[i].Area >= MIN_FOREGROUND_AREA)
        {
            ValidFlag[i] = 2;
        }
        else
        {
            ValidFlag[i] = 1;
        }
    }

    for (i = 0; i < segment_arr->SegmentCnt; i++)
    {
        // 크기가 충분히 큰 경우만 main이 되어 다른 foreground를 merging 할 수 있음.
        if (ValidFlag[i] != 2)
        {
            continue;
        }

        memset(DiffRate, 10000, MAX_SEGMENT_COUNT * sizeof(unsigned short));
        ExistMatchedPair = FALSE;

        for (j = 0; j < segment_arr->SegmentCnt; j++)
        {
            if (i == j || ValidFlag[i] == 0 || ValidFlag[j] == 0)
            {
                continue;
            }

            dx = segment_arr->Segment[j].X - segment_arr->Segment[i].X;
            dy = segment_arr->Segment[j].Y - segment_arr->Segment[i].Y;
            diffdir_btw_two_pos = (int)(atan2((double)dy, (double)dx) * 180.0f / 3.14159265f);
            if (diffdir_btw_two_pos < 0)
            {
                diffdir_btw_two_pos += 180;
            }

            diffdir = diffdir_btw_two_pos - segment_arr->Segment[i].Ang;

            if (abs(diffdir) > THRESHOLD_DIFFERENT_ANGLE_2)
            {
                continue;
            }

            DiffRate[j] = abs(diffdir_btw_two_dir) * 4 + abs(diffdir) * 6;

            // merging 할 pair가 존재함.
            ExistMatchedPair = TRUE;
        }

        if (ExistMatchedPair == TRUE)
        {
            // 가장 각도 차이가 적은 pair만 합침
            MinDiffRate = 10000;
            MinDiffIdx = 0;
            for (j = 0; j < segment_arr->SegmentCnt; j++)
            {
                if (MinDiffRate > DiffRate[j])
                {
                    MinDiffRate = DiffRate[j];
                    MinDiffIdx = j;
                }
            }

            if (MinDiffRate == 10000)
            {
                continue;
            }

            rad = (segment_arr->Segment[i].Ang - 90) * 3.141592 / 180.0;
            cosangle = cos(rad);
            sinangle = sin(rad);

            // up-axis position
            dx = segment_arr->Segment[MinDiffIdx].X - segment_arr->Segment[i].X;
            dy = segment_arr->Segment[MinDiffIdx].Y - segment_arr->Segment[i].Y;

            RotY = (int)(-dx * sinangle + dy * cosangle) + segment_arr->Segment[i].Y;

            if (RotY < segment_arr->Segment[i].Y)
            {
                ValidFlag[MinDiffIdx] = 0;

                // Change Label j --> Label i
                for (y = en_h_band; y < imgHeight - en_h_band; y++)
                {
                    for (x = en_w_band; x < imgWidth - en_w_band; x++)
                    {
                        if (backImage[y * imgWidth + x] == segment_arr->Segment[MinDiffIdx].Label)
                        {
                            backImage[y * imgWidth + x] = segment_arr->Segment[i].Label;
                        }
                    }
                }
            }
        }
    }

    val_BackCount = 0;
    for (i = 0; i < segment_arr->SegmentCnt; i++)
    {
        if (ValidFlag[i] > 0)
        {
            TmpSegArr.Segment[val_BackCount] = segment_arr->Segment[i];
            val_BackCount++;
        }
    }

    TmpSegArr.SegmentCnt = val_BackCount;

    memcpy(segment_arr, &TmpSegArr, sizeof(SEGMENT_ARRAY));

    return segment_arr->SegmentCnt;
}

void CIBAlgorithm::_Algo_AdjustForegroundDirection2(SEGMENT *segment, BYTE *backImage, const int imgWidth, const int imgHeight)
{
    int i, j;
    int Left = imgWidth, Right = 0, Top = imgHeight, Bottom = 0;
    int CutCount = 0;
    int LeftCut = 0, RightCut = 0, TopCut = 0, BottomCut = 0;
    int InterPoints[10][2], InterPointCnt = 0;
    int StartPos;
    int dx, dy, dist, sum_dx, sum_dy, sum_dxdy, sum_count, magcnt, magratio_1 = 10, magratio_2 = 12;
    int LeftMax, RightMax, TopMax, BottomMax;
    int SegX, SegY;
    const int en_h_band = 8;
    const int en_w_band = 8;

    LeftMax = en_w_band;
    RightMax = imgWidth - en_w_band - 1;
    TopMax = en_h_band;
    BottomMax = imgHeight - en_h_band - 1;

    sum_dx = 0;
    sum_dy = 0;
    sum_count = 0;
    for (i = en_h_band; i < imgHeight - en_h_band; i++)
    {
        for (j = en_w_band; j < imgWidth - en_w_band; j++)
        {
            if (backImage[i * imgWidth + j] == segment->Label)
            {
                if (Left > j)
                {
                    Left = j;
                }
                if (Right < j)
                {
                    Right = j;
                }
                if (Top > i)
                {
                    Top = i;
                }
                if (Bottom < i)
                {
                    Bottom = i;
                }

                sum_dx += j;
                sum_dy += i;
                sum_count++;
            }
        }
    }

    if (sum_count > 0)
    {
        segment->X = sum_dx / sum_count * 4 - en_w_band;
        segment->Y = sum_dy / sum_count * 4 - en_h_band;
    }

    SegX = (segment->X + en_w_band) / 4;
    SegY = (segment->Y + en_h_band) / 4;

    if (Left <= LeftMax)
    {
        LeftCut++;
    }
    if (Right >= RightMax)
    {
        RightCut++;
    }
    if (Top <= TopMax)
    {
        TopCut++;
    }
    if (Bottom >= BottomMax)
    {
        BottomCut++;
    }

    CutCount = LeftCut + RightCut + TopCut + BottomCut;

    if (CutCount > 2 || CutCount == 0)	// return if CutCount isn't 1 or 2.
    {
        return;
    }

    if (LeftCut == 1 && RightCut == 1)	// return case
    {
        return;
    }

    if (TopCut == 1 && BottomCut == 1)	// return case
    {
        return;
    }

    // find intersection points.
    if (CutCount == 1)
    {
        // left or right cut
        if (LeftCut == 1 || RightCut == 1)
        {
            if (LeftCut == 1)
            {
                StartPos = LeftMax;    //ENLARGESIZE_ZOOM_W;
            }
            else if (RightCut == 1)
            {
                StartPos = RightMax;    //ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W-1;
            }
            else
            {
                return;
            }

            for (i = en_h_band; i < imgHeight - en_h_band; i++)
            {
                if (backImage[i * imgWidth + StartPos] == segment->Label)
                {
                    InterPoints[InterPointCnt][0] = StartPos;	// x
                    InterPoints[InterPointCnt][1] = i;	// y
                    InterPointCnt++;
                    break;
                }
            }
            for (i = imgHeight - en_h_band - 1; i >= en_h_band; i--)
            {
                if (backImage[i * imgWidth + StartPos] == segment->Label)
                {
                    InterPoints[InterPointCnt][0] = StartPos;	// x
                    InterPoints[InterPointCnt][1] = i;	// y
                    InterPointCnt++;
                    break;
                }
            }
            if (InterPointCnt != 2)
            {
                return;
            }
        }
        // to or bottom cut
        else if (TopCut == 1 || BottomCut == 1)
        {
            if (TopCut == 1)
            {
                StartPos = TopMax;    //ENLARGESIZE_ZOOM_H;
            }
            else if (BottomCut == 1)
            {
                StartPos = BottomMax;    //ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H-1;
            }
            else
            {
                return;
            }

            for (j = en_w_band; j < imgWidth - en_w_band; j++)
            {
                if (backImage[StartPos * imgWidth + j] == segment->Label)
                {
                    InterPoints[InterPointCnt][0] = j;	// x
                    InterPoints[InterPointCnt][1] = StartPos;	// y
                    InterPointCnt++;
                    break;
                }
            }
            for (j = imgWidth - en_w_band - 1; j >= en_w_band; j--)
            {
                if (backImage[StartPos * imgWidth + j] == segment->Label)
                {
                    InterPoints[InterPointCnt][0] = j;	// x
                    InterPoints[InterPointCnt][1] = StartPos;	// y
                    InterPointCnt++;
                    break;
                }
            }
            if (InterPointCnt != 2)
            {
                return;
            }
        }

        // recalculate foreground direction
        if ((abs(InterPoints[0][0] - InterPoints[1][0]) + abs(InterPoints[0][1] - InterPoints[1][1])) > 5)
        {
            sum_dx = sum_dy = sum_dxdy = sum_count = 0;
            for (i = en_h_band; i < imgHeight - en_h_band; i++)
            {
                for (j = en_w_band; j < imgWidth - en_w_band; j++)
                {
                    if (backImage[i * imgWidth + j] == segment->Label)
                    {
                        dx = j - SegX;
                        dy = i - SegY;

                        dist = (int)sqrt((double)dy * dy + (double)dx * dx);

                        if (dist == 0)
                        {
                            continue;
                        }

                        dx = dx * 10 / dist;
                        dy = dy * 10 / dist;

                        sum_dx += dx * dx;
                        sum_dy += dy * dy;
                        sum_dxdy += dx * dy;
                        sum_count++;
                    }
                }
            }

            if (sum_count <= MIN_FOREGROUND_AREA)
            {
                return;
            }

            magcnt = (abs(InterPoints[0][0] - InterPoints[1][0]) + abs(InterPoints[0][1] - InterPoints[1][1])) * magratio_1;

            if (magcnt > sum_count / 4)
            {
                magcnt = sum_count / 4;
            }

            dx = (InterPoints[0][0] + InterPoints[1][0]) / 2 - SegX;
            dy = (InterPoints[0][1] + InterPoints[1][1]) / 2 - SegY;

            dist = (int)sqrt((double)dy * dy + (double)dx * dx);

            if (dist == 0)
            {
                return;
            }

            dx = dx * 10 / dist;
            dy = dy * 10 / dist;

            sum_dx += dx * dx * magcnt;
            sum_dy += dy * dy * magcnt;
            sum_dxdy += dx * dy * magcnt;
            sum_count += magcnt;

            segment->Ang = _Algo_ATAN2_FULL(2 * sum_dxdy, (sum_dx - sum_dy)) * 180 / 512;
        }
    }
    else if (CutCount == 2)
    {
        // left-top cut
        if (LeftCut == 1 && TopCut == 1)
        {
            StartPos = LeftMax;//ENLARGESIZE_ZOOM_W;
            for (i = imgHeight - en_h_band - 1; i >= en_h_band; i--)
            {
                if (backImage[i * imgWidth + StartPos] == segment->Label)
                {
                    InterPoints[InterPointCnt][0] = StartPos;	// x
                    InterPoints[InterPointCnt][1] = i;	// y
                    InterPointCnt++;
                    break;
                }
            }
            StartPos = TopMax;//ENLARGESIZE_ZOOM_H;
            for (j = imgWidth - en_w_band - 1; j >= en_w_band; j--)
            {
                if (backImage[StartPos * imgWidth + j] == segment->Label)
                {
                    InterPoints[InterPointCnt][0] = j;	// x
                    InterPoints[InterPointCnt][1] = StartPos;	// y
                    InterPointCnt++;
                    break;
                }
            }
            if (InterPointCnt != 2)
            {
                return;
            }
        }
        // left-bottom cut
        else if (LeftCut == 1 && BottomCut == 1)
        {
            StartPos = LeftMax;//ENLARGESIZE_ZOOM_W;
            for (i = en_h_band; i < imgHeight - en_h_band; i++)
            {
                if (backImage[i * imgWidth + StartPos] == segment->Label)
                {
                    InterPoints[InterPointCnt][0] = StartPos;	// x
                    InterPoints[InterPointCnt][1] = i;	// y
                    InterPointCnt++;
                    break;
                }
            }
            StartPos = BottomMax;//ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H-1;
            for (j = en_w_band - imgWidth - 1; j >= en_w_band; j--)
            {
                if (backImage[StartPos * imgWidth + j] == segment->Label)
                {
                    InterPoints[InterPointCnt][0] = j;	// x
                    InterPoints[InterPointCnt][1] = StartPos;	// y
                    InterPointCnt++;
                    break;
                }
            }
            if (InterPointCnt != 2)
            {
                return;
            }
        }
        // right-top cut
        else if (RightCut == 1 && TopCut == 1)
        {
            StartPos = RightMax;//ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W-1;
            for (i = imgHeight - en_h_band - 1; i >= en_h_band; i--)
            {
                if (backImage[i * imgWidth + StartPos] == segment->Label)
                {
                    InterPoints[InterPointCnt][0] = StartPos;	// x
                    InterPoints[InterPointCnt][1] = i;	// y
                    InterPointCnt++;
                    break;
                }
            }
            StartPos = TopMax;//ENLARGESIZE_ZOOM_H;
            for (j = en_w_band; j < imgWidth - en_w_band; j++)
            {
                if (backImage[StartPos * imgWidth + j] == segment->Label)
                {
                    InterPoints[InterPointCnt][0] = j;	// x
                    InterPoints[InterPointCnt][1] = StartPos;	// y
                    InterPointCnt++;
                    break;
                }
            }
            if (InterPointCnt != 2)
            {
                return;
            }
        }
        // right-bottom cut
        else if (RightCut == 1 && BottomCut == 1)
        {
            StartPos = RightMax;//ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W-1;
            for (i = en_h_band; i < imgHeight - en_h_band; i++)
            {
                if (backImage[i * imgWidth + StartPos] == segment->Label)
                {
                    InterPoints[InterPointCnt][0] = StartPos;	// x
                    InterPoints[InterPointCnt][1] = i;	// y
                    InterPointCnt++;
                    break;
                }
            }
            StartPos = BottomMax;//ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H-1;
            for (j = en_w_band; j < imgWidth - en_w_band; j++)
            {
                if (backImage[StartPos * imgWidth + j] == segment->Label)
                {
                    InterPoints[InterPointCnt][0] = j;	// x
                    InterPoints[InterPointCnt][1] = StartPos;	// y
                    InterPointCnt++;
                    break;
                }
            }
            if (InterPointCnt != 2)
            {
                return;
            }
        }

        // recalculate foreground direction
        if (abs(InterPoints[0][0] - InterPoints[1][0]) + abs(InterPoints[0][1] - InterPoints[1][1]) > 5)
        {
            sum_dx = sum_dy = sum_dxdy = sum_count = 0;
            for (i = en_h_band; i < imgHeight - en_h_band; i++)
            {
                for (j = en_w_band; j < imgWidth - en_w_band; j++)
                {
                    if (backImage[i * imgWidth + j] == segment->Label)
                    {
                        dx = j - SegX;
                        dy = i - SegY;

                        dist = (int)sqrt((double)dy * dy + (double)dx * dx);

                        if (dist == 0)
                        {
                            continue;
                        }

                        dx = dx * 10 / dist;
                        dy = dy * 10 / dist;

                        sum_dx += dx * dx;
                        sum_dy += dy * dy;
                        sum_dxdy += dx * dy;
                        sum_count++;
                    }
                }
            }

            if (sum_count <= MIN_FOREGROUND_AREA)
            {
                return;
            }

            magcnt = (abs(InterPoints[0][0] - InterPoints[1][0]) + abs(InterPoints[0][1] - InterPoints[1][1])) * magratio_2;

            if (magcnt > sum_count / 4)
            {
                magcnt = sum_count / 4;
            }

            dx = (InterPoints[0][0] + InterPoints[1][0]) / 2 - SegX;
            dy = (InterPoints[0][1] + InterPoints[1][1]) / 2 - SegY;

            dist = (int)sqrt((double)dy * dy + (double)dx * dx);

            if (dist == 0)
            {
                return;
            }

            dx = dx * 10 / dist;
            dy = dy * 10 / dist;

            sum_dx += dx * dx * magcnt;
            sum_dy += dy * dy * magcnt;
            sum_dxdy += dx * dy * magcnt;
            sum_count += magcnt;

            segment->Ang = _Algo_ATAN2_FULL(2 * sum_dxdy, (sum_dx - sum_dy)) * 180 / 512;
        }
    }
}

int CIBAlgorithm::_Algo_StackRecursiveFilling_ZoomEnlarge(unsigned char *image, int x, int y, unsigned char TargetColor, unsigned char DestColor)
{
    int dx, dy;
    int top, changed_color_cnt = 0;

	int tmpZOOM_ENLAGE_W = ZOOM_ENLAGE_W;
	int tmpZOOM_ENLAGE_H = ZOOM_ENLAGE_H;

	if (m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		tmpZOOM_ENLAGE_W = ZOOM_ENLAGE_W_ROLL;
		tmpZOOM_ENLAGE_H = ZOOM_ENLAGE_H_ROLL;
	}

    top = -1;				// init_stack
    stack[++top] = y;		// push
    stack[++top] = x;		// push

    while (top >= 0)
    {
        dx = stack[top--];		// pop
        dy = stack[top--];		// pop
        if (dx < 0 || dy < 0 || dx >= tmpZOOM_ENLAGE_W || dy >= tmpZOOM_ENLAGE_H)
        {
            continue;
        }

        if (image[(dy * tmpZOOM_ENLAGE_W) + dx] == TargetColor)
        {
            changed_color_cnt++;

            image[(dy * tmpZOOM_ENLAGE_W) + dx] = DestColor;
            stack[++top] = dy;		// push
            stack[++top] = dx - 1;	// push

            stack[++top] = dy;		// push
            stack[++top] = dx + 1;	// push

            stack[++top] = dy - 1;	// push
            stack[++top] = dx;		// push

            stack[++top] = dy + 1;	// push
            stack[++top] = dx;		// push
        }
    }

    return changed_color_cnt;
}

int CIBAlgorithm::_Algo_ATAN2_FULL(int y, int x)
{
#define MAX_MATH_SIZE				1024
#define QUANTIZED_DIRECTION			256
    int single_tan_table[MAX_MATH_SIZE / 2] =
    {
        0, 12, 25, 37, 50, 62, 75, 88, 100, 113, 126, 139, 151, 164, 177, 190, 203, 216, 229, 243, 256, 269, 283, 296, 310, 324, 338, 352, 366, 380, 395, 409, 424, 438, 453, 469, 484, 499, 515, 531, 547, 563, 580, 596, 613, 630, 648, 666, 684, 702,
        721, 740, 759, 779, 799, 819, 840, 861, 883, 905, 928, 951, 974, 999, 1023, 1049, 1075, 1102, 1129, 1158, 1187, 1216, 1247, 1279, 1312, 1345, 1380, 1416, 1453, 1492, 1532, 1574, 1617, 1661, 1708, 1756, 1807, 1860, 1915, 1973, 2034, 2098, 2165, 2235, 2310, 2388, 2472, 2560, 2654, 2754,
        2861, 2976, 3099, 3232, 3375, 3531, 3700, 3885, 4088, 4311, 4560, 4836, 5147, 5499, 5901, 6364, 6903, 7539, 8302, 9233, 10396, 11891, 13881, 16667, 20843, 27801, 41712, 83436, 2147483647, -83441, -41713, -27802, -20844, -16667, -13882, -11891, -10396, -9233, -8302, -7539, -6903, -6364, -5901, -5499, -5148, -4837, -4560, -4311, -4088, -3885,
        -3700, -3531, -3375, -3232, -3099, -2976, -2861, -2754, -2654, -2560, -2472, -2388, -2310, -2235, -2165, -2098, -2034, -1973, -1915, -1860, -1807, -1756, -1708, -1661, -1617, -1574, -1532, -1492, -1453, -1416, -1380, -1345, -1312, -1279, -1247, -1216, -1187, -1158, -1129, -1102, -1075, -1049, -1024, -999, -974, -951, -928, -905, -883, -861,
        -840, -819, -799, -779, -759, -740, -721, -702, -684, -666, -648, -630, -613, -596, -580, -563, -547, -531, -515, -499, -484, -469, -453, -438, -424, -409, -395, -380, -366, -352, -338, -324, -310, -296, -283, -269, -256, -243, -229, -216, -203, -190, -177, -164, -151, -139, -126, -113, -100, -88,
        -75, -62, -50, -37, -25, -12, 0, 12, 25, 37, 50, 62, 75, 88, 100, 113, 126, 139, 151, 164, 177, 190, 203, 216, 229, 243, 256, 269, 283, 296, 310, 324, 338, 352, 366, 380, 394, 409, 424, 438, 453, 469, 484, 499, 515, 531, 547, 563, 580, 596,
        613, 630, 648, 666, 684, 702, 721, 740, 759, 779, 799, 819, 840, 861, 883, 905, 928, 951, 974, 999, 1023, 1049, 1075, 1102, 1129, 1158, 1187, 1216, 1247, 1279, 1312, 1345, 1380, 1416, 1453, 1492, 1532, 1573, 1617, 1661, 1708, 1756, 1807, 1860, 1915, 1973, 2034, 2098, 2165, 2235,
        2310, 2388, 2472, 2560, 2654, 2754, 2861, 2976, 3099, 3232, 3375, 3531, 3700, 3885, 4088, 4311, 4560, 4836, 5147, 5499, 5901, 6364, 6903, 7539, 8302, 9233, 10396, 11890, 13881, 16667, 20843, 27801, 41711, 83432, 1044487955, -83445, -41714, -27802, -20844, -16667, -13882, -11891, -10396, -9233, -8302, -7539, -6903, -6364, -5901, -5499,
        -5148, -4837, -4560, -4311, -4088, -3885, -3700, -3531, -3375, -3232, -3099, -2976, -2861, -2754, -2654, -2560, -2472, -2388, -2310, -2235, -2165, -2098, -2034, -1973, -1915, -1860, -1807, -1756, -1708, -1661, -1617, -1574, -1532, -1492, -1453, -1416, -1380, -1345, -1312, -1279, -1247, -1216, -1187, -1158, -1129, -1102, -1075, -1049, -1024, -999,
        -974, -951, -928, -905, -883, -861, -840, -819, -799, -779, -759, -740, -721, -702, -684, -666, -648, -630, -613, -596, -580, -563, -547, -531, -515, -499, -484, -469, -453, -438, -424, -409, -395, -380, -366, -352, -338, -324, -310, -296, -283, -269, -256, -243, -229, -216, -203, -190, -177, -164,
        -151, -139, -126, -113, -100, -88, -75, -62, -50, -37, -25, -12
    };

    int i, r;
    int im_phi;
    int im_2phi;
    int im_phi2;
    int im_mul;

    im_2phi = QUANTIZED_DIRECTION * 2;
    im_phi = QUANTIZED_DIRECTION;
    im_phi2 = QUANTIZED_DIRECTION / 2;
    im_mul = MAX_MATH_SIZE;

    if (x == 0 && y < 0)
    {
        return (im_phi2 + im_phi);
    }
    else if (x == 0 && y >= 0)
    {
        return (im_phi2);
    }
    else if (y == 0 && x >= 0)
    {
        return 0;
    }
    else if (y == 0 && x < 0)
    {
        return (im_phi);
    }

    r = abs(y * im_mul / x);

    for (i = 0; i < im_phi2; i++)
        if (r < single_tan_table[i + 1])
        {
            break;
        }

    if (y > 0)
    {
        if (x < 0)
        {
            return im_phi - i;
        }
    }
    else
    {
        if (x < 0)
        {
            return im_phi + i;
        }
        else
        {
            return im_2phi - i;
        }
    }

    return i;
}

void CIBAlgorithm::_Algo_Init_Distortion_Merge_with_Bilinear()
{
    float DISTORTION_A = -0.010f;
    float DISTORTION_B = -0.010f;
    float DISTORTION_C = 0.010f;

    float DISTORTION_D = 0.000f;
    int DISTORTION_SX = 0;
    int DISTORTION_SY = 0;
    int DISTORTION_RX = (CIS_IMG_W - 4) / 2;
    int DISTORTION_RY = (CIS_IMG_H - 4) / 2;

    int i, j, x, y;
    float DestX, DestY, SrcX, SrcY;
    float DestR, DestA, SrcR/*, tmp_DISTORTION_A*/;
    float Inv_Distortion_ABCD = 1.0f - DISTORTION_A - DISTORTION_B - DISTORTION_C - DISTORTION_D;
    float p, q;
    int HalfX, HalfY, TargetHalfX, TargetHalfY;
    int shifty = 2;
    int shiftx = 2;

    TargetHalfX = DISTORTION_RX;
    TargetHalfY = DISTORTION_RY;

    HalfX = IMG_W / 2;
    HalfY = IMG_H / 2;

    DISTORTION_A = -0.01f;
    DISTORTION_B = -0.000f;
    DISTORTION_C = 0.000f;
    DISTORTION_D = 0.00f;
    Inv_Distortion_ABCD = 1.0f - DISTORTION_A - DISTORTION_B - DISTORTION_C - DISTORTION_D;

    //	float half_dist = sqrtf((float)((HalfY*1.35)*(HalfY*1.35) + (HalfX*1.35)*(HalfX*1.35)));

    // make table
    for (i = -HalfY; i < HalfY; i++)
    {
        DestY = (float)i / (float)HalfY;

        for (j = -HalfX; j < HalfX; j++)
        {
            DestX = (float)j / (float)HalfX;

            DestR = sqrtf(DestY * DestY + DestX * DestX);
            DestA = atan2f(DestY, DestX);

            SrcR = (DISTORTION_A * DestR * DestR * DestR * DestR + DISTORTION_B * DestR * DestR * DestR + DISTORTION_C * DestR * DestR
                    + DISTORTION_D * DestR + Inv_Distortion_ABCD) * DestR;

            SrcX = SrcR * cosf(DestA) * TargetHalfX + TargetHalfX + DISTORTION_SX + shiftx + 0.5f;
            SrcY = SrcR * sinf(DestA) * TargetHalfY + TargetHalfY + DISTORTION_SY + shifty + 0.5f;

            x = (int)SrcX;
            y = (int)SrcY;

            if (x >= 0 && x < CIS_IMG_W && y >= 0 && y < CIS_IMG_H)
            {
                p = 1.0f - (SrcX - (float)x);
                q = 1.0f - (SrcY - (float)y);

                arrPos_Watson[(i + HalfY)*IMG_W + j + HalfX] = (DWORD)((x & 0x7FF) << 21);
                arrPos_Watson[(i + HalfY)*IMG_W + j + HalfX] |= (DWORD)((y & 0x7FF) << 10);
                arrPos_Watson[(i + HalfY)*IMG_W + j + HalfX] |= (DWORD)((int)(p * 32) << 5);
                arrPos_Watson[(i + HalfY)*IMG_W + j + HalfX] |= (DWORD)(q * 32);
            }
            else
            {
                arrPos_Watson[(i + HalfY)*IMG_W + j + HalfX] = 0xffffffff;
            }
        }
    }
}

void CIBAlgorithm::_Algo_DistortionRestoration_with_Bilinear(unsigned char *InImg, unsigned char *OutImg)
{
    int value;
    int i, j, x, y, pos, sp, sq;
    int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

    memset(OutImg, 0, IMG_SIZE);
    dtLEFT		= m_cImgAnalysis.LEFT * IMG_W / CIS_IMG_W;
    dtRIGHT		= m_cImgAnalysis.RIGHT * IMG_W / CIS_IMG_W;
    dtTOP		= m_cImgAnalysis.TOP * IMG_H / CIS_IMG_H;
    dtBOTTOM	= m_cImgAnalysis.BOTTOM * IMG_H / CIS_IMG_H;
    for (i = dtTOP; i <= dtBOTTOM; i++)
    {
        for (j = dtLEFT; j <= dtRIGHT; j++)
        {
            x = (arrPos_Watson[i * IMG_W + j] >> 21);
            y = (arrPos_Watson[i * IMG_W + j] >> 10) & 0x7FF;
            sp = (arrPos_Watson[i * IMG_W + j] >> 5) & 0x1F;
            sq = arrPos_Watson[i * IMG_W + j] & 0x1F;
            pos = y * CIS_IMG_W + x;

            value = (sp * (sq * InImg[pos] + (32 - sq) * InImg[pos + CIS_IMG_W]) + (32 - sp) * (sq * InImg[pos + 1] + (32 - sq) * InImg[pos + CIS_IMG_W + 1])) >> 10;

            OutImg[i * IMG_W + j] = (unsigned char)value;
        }
    }
}

void CIBAlgorithm::_Algo_VignettingEffect(unsigned char *InImg, unsigned char *OutImg)
{
    int i, j, val;

    memset(OutImg, 0, CIS_IMG_H * CIS_IMG_W);

    for (i = m_cImgAnalysis.TOP; i <= m_cImgAnalysis.BOTTOM; i++)
    {
        for (j = m_cImgAnalysis.LEFT; j <= m_cImgAnalysis.RIGHT; j++)
        {
            val = InImg[i * CIS_IMG_W + j];
            if (val > 0)
            {
                val = ((int)(val * m_UM_Watson_F[i * CIS_IMG_W + j]) >> 10) + (int)val;

                if (val > 255)
                {
                    val = 255;
                }
                else if (val < 0)
                {
                    val = 0;
                }

                OutImg[i * CIS_IMG_W + j] = (unsigned char)val;
            }
        }
    }
}

void CIBAlgorithm::_Algo_RemoveVignettingNoise(unsigned char *InImg, unsigned char *OutImg, unsigned char *TmpImg)
{
    int i, j;
    int center, outsum;
    int BrightThres = 30;
    int MeanBright = 0;

    MeanBright = m_cImgAnalysis.mean;

    BrightThres = BrightThres * MeanBright / 128;
    if (BrightThres > 30)
    {
        BrightThres = 30;
    }
    else if (BrightThres < 10)
    {
        BrightThres = 10;
    }

    memcpy(OutImg, InImg, CIS_IMG_H * CIS_IMG_W);
    for (i = m_cImgAnalysis.TOP; i <= m_cImgAnalysis.BOTTOM; i++)
    {
        for (j = m_cImgAnalysis.LEFT; j <= m_cImgAnalysis.RIGHT; j++)
        {
            center = InImg[i * CIS_IMG_W + j];

            if (center < MeanBright)
            {
                continue;
            }

            outsum = (InImg[(i - 1) * CIS_IMG_W + (j - 1)] + InImg[(i - 1) * CIS_IMG_W + (j)] + InImg[(i - 1) * CIS_IMG_W + (j + 1)] +
                      InImg[(i) * CIS_IMG_W + (j - 1)] + InImg[(i) * CIS_IMG_W + (j + 1)] +
                      InImg[(i + 1) * CIS_IMG_W + (j - 1)] + InImg[(i + 1) * CIS_IMG_W + (j)] + InImg[(i + 1) * CIS_IMG_W + (j + 1)]) >> 3;

            if (center > outsum + BrightThres)
            {
                OutImg[i * CIS_IMG_W + j] = outsum;
            }
        }
    }
}

void CIBAlgorithm::_Algo_Genertate_Sum_Gray(unsigned char *image, int imgWidth, int imgHeight, int rectTop, int rectBottom, int rectLeft, int rectRight)
{
    int xx, yy;

    memset(sum_gray_buffer, 0, CIS_IMG_SIZE * 4);
    sum_gray_buffer[rectTop * imgWidth + rectLeft] =
        image[rectTop * imgWidth + rectLeft];

    for (xx = rectLeft + 1; xx < rectRight; xx++)
        sum_gray_buffer[rectTop * imgWidth + xx] =
            sum_gray_buffer[rectTop * imgWidth + xx - 1] + image[rectTop * imgWidth + xx];

    for (yy = rectTop + 1; yy < rectBottom; yy++)
        sum_gray_buffer[yy * imgWidth + rectLeft] =
            sum_gray_buffer[(yy - 1) * imgWidth + rectLeft] + image[yy * imgWidth + rectLeft];

    for (yy = rectTop + 1; yy < rectBottom; yy++)
    {
        for (xx = rectLeft + 1; xx < rectRight; xx++)
        {
            sum_gray_buffer[yy * imgWidth + xx] = sum_gray_buffer[yy * imgWidth + (xx - 1)]
                                                  + sum_gray_buffer[(yy - 1) * imgWidth + xx]
                                                  + image[yy * imgWidth + xx]
                                                  - sum_gray_buffer[(yy - 1) * imgWidth + (xx - 1)];
        }
    }
}

int CIBAlgorithm::_Algo_SearchingMaxGraybox_Fast(unsigned char *image, int imgWidth, int imgHeight, int rectTop, int rectBottom, int rectLeft, int rectRight)
{
    int y, x; //,j,i;
    int sum = 0; //,count;

    int max_gray = 0;

    int left, right, top, bottom;
    int val1, val2, val3, val4;

    _Algo_Genertate_Sum_Gray(image, imgWidth, imgHeight, rectTop, rectBottom, rectLeft, rectRight);

    for (y = rectTop + 6; y < rectBottom - 5; y += 2)
    {
        top = y - 6;
        bottom = y + 5;
        for (x = rectLeft + 6; x < rectRight - 5; x += 2)
        {
            left = x - 6;
            right = x + 5;

            val1 = sum_gray_buffer[bottom * imgWidth + right];
            val2 = sum_gray_buffer[top * imgWidth + right];
            val3 = sum_gray_buffer[bottom * imgWidth + left];
            val4 = sum_gray_buffer[top * imgWidth + left];

            sum = val1 - val2 - val3 + val4;
            //			sum = sum / 121;

            if (sum > max_gray)
            {
                max_gray = sum;
            }
        }
    }

    return max_gray / 121;
}
//#endif

void CIBAlgorithm::_Algo_Init_GammaTable()
{
    // generate Lookup Table
    float Gamma, Shift;

    memset(G_GammaTable, 0, 48 * 256);
    for (int contrast = 0; contrast < 48; contrast++)
    {
        Gamma = 1.0f + (float)contrast / 12.0f;
        Shift = 0;

        for (int gray = 0; gray < 256; gray++)
        {
            G_GammaTable[(contrast * 256) + gray] = 255 - (int)(255.0f * powf(((float)(255 - gray) - Shift) / (255.0f - Shift), Gamma));
        }
    }
}

void CIBAlgorithm::_Algo_Image_Gamma(unsigned char *InImg, unsigned char *OutImg, int Contrast)
{
    int i, j;
    int value;
    unsigned char *GammaTable = (unsigned char *)&G_GammaTable[Contrast * 256];
	int Minus_Val = 0;

	int LEFT, RIGHT, TOP, BOTTOM;

	LEFT = m_cImgAnalysis.LEFT;
	RIGHT = m_cImgAnalysis.RIGHT;
	TOP = m_cImgAnalysis.TOP;
	BOTTOM = m_cImgAnalysis.BOTTOM;

	int tmpCIS_IMG_W = CIS_IMG_W;
//	int tmpCIS_IMG_H = CIS_IMG_H;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE;

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
	{
		Minus_Val = m_SBDAlg->m_Roll_Minus_Val;
		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
	{
		Minus_Val = 5;
		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
		tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	}

	memcpy(OutImg, InImg, tmpCIS_IMG_SIZE);
	for (i = TOP; i < BOTTOM; i++)
	{
		for (j = LEFT; j < RIGHT; j++)
		{
			value = InImg[i * tmpCIS_IMG_W + j];
			if (value > Minus_Val)
			{
				OutImg[i * tmpCIS_IMG_W + j] = GammaTable[value];
			}
		}
	}
}

void CIBAlgorithm::_Algo_Image_Smoothing(unsigned char *InImg, unsigned char *OutImg)
{
    int i, j, value;
    int LEFT = m_cImgAnalysis.LEFT * IMG_W / CIS_IMG_W;
    int RIGHT = m_cImgAnalysis.RIGHT * IMG_W / CIS_IMG_W;
    int TOP = m_cImgAnalysis.TOP * IMG_H / CIS_IMG_H;
    int BOTTOM = m_cImgAnalysis.BOTTOM * IMG_H / CIS_IMG_H;

    memcpy(OutImg, InImg, CIS_IMG_SIZE);

    for (i = TOP; i < BOTTOM; i++)
    {
        for (j = LEFT; j < RIGHT; j++)
        {
            // 2013-03-19 Gon add - To improve algorithm speed
            if (InImg[i * IMG_W + j] == 0)
            {
                continue;
            }

            value = (int)(((InImg[(i - 1) * IMG_W + j - 1] << 2) +
                           (InImg[(i - 1) * IMG_W + j] << 4) +
                           (InImg[(i - 1) * IMG_W + j + 1] << 2) +
                           (InImg[i * IMG_W + j - 1] << 4) +
                           (InImg[i * IMG_W + j] << 8) +
                           (InImg[i * IMG_W + j + 1] << 4) +
                           (InImg[(i + 1) * IMG_W + j - 1] << 2) +
                           (InImg[(i + 1) * IMG_W + j] << 4) +
                           (InImg[(i + 1) * IMG_W + j + 1] << 2)) / 336);

            OutImg[i * IMG_W + j] = value;
        }
    }
}

void CIBAlgorithm::_Algo_Image_Gamma_Simple(unsigned char *InImg, unsigned char *OutImg, int Contrast, int MergePosX)
{
    int i, j;
    int value;
    unsigned char *gammatable = (unsigned char *)&G_GammaTable[(Contrast * 256)];
	int Minus_Val = 0;
    
	int /*LEFT, RIGHT, */TOP, BOTTOM;

	TOP = m_cImgAnalysis.TOP;
	BOTTOM = m_cImgAnalysis.BOTTOM;

	int tmpCIS_IMG_W = CIS_IMG_W;
//	int tmpCIS_IMG_H = CIS_IMG_H;

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
	{
		Minus_Val = m_SBDAlg->m_Roll_Minus_Val;
		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
	}
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
	{
		Minus_Val = 5;
		tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//		tmpCIS_IMG_H = CIS_IMG_H_ROLL;
	}

	for (i = TOP; i < BOTTOM; i+=2)
	{
		for (j = MergePosX - 10; j <= MergePosX + 10; j+=2)
		{
			value = InImg[i * tmpCIS_IMG_W + j];
			if (value > Minus_Val)
			{
				OutImg[i * tmpCIS_IMG_W + j] = gammatable[value];
			}
			else
			{
				OutImg[i * tmpCIS_IMG_W + j] = value;
			}
		}
	}
}

void CIBAlgorithm::_Algo_SwUniformity(unsigned char *InImg, unsigned char *OutImg, unsigned char *TmpOrigin, int width, int height)
{
    int LEFT = m_cImgAnalysis.LEFT * IMG_W / CIS_IMG_W;
    int RIGHT = m_cImgAnalysis.RIGHT * IMG_W / CIS_IMG_W;
    int TOP = m_cImgAnalysis.TOP * IMG_H / CIS_IMG_H;
    int BOTTOM = m_cImgAnalysis.BOTTOM * IMG_H / CIS_IMG_H;

    int i, j, loop;
    int image_mean_val, val, mean_val, weight, count;
    int temp1, temp2, max_val = -100000000, min_val = 100000000;

    // enzyme add 2012-11-29 for blank image processing
    if (m_cImgAnalysis.LEFT == m_cImgAnalysis.RIGHT && m_cImgAnalysis.TOP == m_cImgAnalysis.BOTTOM)
    {
        memcpy(OutImg, InImg, IMG_SIZE);
        return;
    }

    if (LEFT < 1)
    {
        LEFT = 1;
    }
    if (TOP < 1)
    {
        TOP = 1;
    }
    if (RIGHT > IMG_W - 2)
    {
        RIGHT = IMG_W - 2;
    }
    if (BOTTOM > IMG_H - 2)
    {
        BOTTOM = IMG_H - 2;
    }

    memset(TmpOrigin, 255, IMG_SIZE);

    for (i = TOP; i <= BOTTOM; i++)
    {
        for (j = LEFT; j <= RIGHT; j++)
        {
            TmpOrigin[i * IMG_W + j] = 255 - InImg[i * IMG_W + j];
        }
    }
    memcpy(OutImg, TmpOrigin, IMG_SIZE);

    image_mean_val = 0;
    count = 0;
    for (i = TOP; i <= BOTTOM; i++)
    {
        for (j = LEFT; j <= RIGHT; j++)
        {
            if (TmpOrigin[i * IMG_W + j] < 245)
            {
                image_mean_val += TmpOrigin[i * IMG_W + j];
                count++;
            }
        }
    }
    if (count > 0)
    {
        image_mean_val = image_mean_val / count;
    }

    // calc magnitude
    memset(m_MagBuffer_short, 0, IMG_SIZE * sizeof(USHORT));
    for (i = TOP; i <= BOTTOM; i++)
    {
        for (j = LEFT; j <= RIGHT; j++)
        {
            temp1 = - InImg[(i - 1) * IMG_W + (j - 1)] + InImg[(i - 1) * IMG_W + (j + 1)]
                    - InImg[(i) * IMG_W + (j - 1)] + InImg[(i) * IMG_W + (j + 1)]
                    - InImg[(i + 1) * IMG_W + (j - 1)] + InImg[(i + 1) * IMG_W + (j + 1)];

            temp2 = - InImg[(i - 1) * IMG_W + (j - 1)] - InImg[(i - 1) * IMG_W + (j)] - InImg[(i - 1) * IMG_W + (j + 1)]
                    + InImg[(i + 1) * IMG_W + (j - 1)] + InImg[(i + 1) * IMG_W + (j)] + InImg[(i + 1) * IMG_W + (j + 1)];

            val = temp1 * temp1 + temp2 * temp2;

            if (max_val < val)
            {
                max_val = val;
            }
            if (min_val > val)
            {
                min_val = val;
            }

            m_MagBuffer_short[i * IMG_W + j] = val;
        }
    }

    if ((max_val - min_val) == 0)
    {
        memcpy(OutImg, InImg, IMG_SIZE);
        return;
    }

    memset(m_MagBuffer, 0, IMG_SIZE);

    for (i = TOP; i <= BOTTOM; i++)
    {
        for (j = LEFT; j <= RIGHT; j++)
        {
            m_MagBuffer[i * IMG_W + j] = (unsigned char)((m_MagBuffer_short[i * IMG_W + j] - min_val) * 63 / (max_val - min_val));
            m_MagBuffer[i * IMG_W + j] <<= 2;
        }
    }

    // remove white noise
    for (loop = 0; loop < 1; loop++)
    {
        for (i = TOP; i <= BOTTOM; i++)
        {
            for (j = LEFT; j <= RIGHT; j++)
            {
                mean_val = (TmpOrigin[(i - 1) * IMG_W + (j - 1)] +
                            TmpOrigin[(i - 1) * IMG_W + j] +
                            TmpOrigin[(i - 1) * IMG_W + (j + 1)] +
                            TmpOrigin[(i) * IMG_W + (j - 1)] +
                            TmpOrigin[(i) * IMG_W + (j + 1)] +
                            TmpOrigin[(i + 1) * IMG_W + (j - 1)] +
                            TmpOrigin[(i + 1) * IMG_W + j] +
                            TmpOrigin[(i + 1) * IMG_W + (j + 1)]) >> 3;

                // white noise
                if (TmpOrigin[i * IMG_W + j] > mean_val)
                {
                    weight = TmpOrigin[i * IMG_W + j] - mean_val - (image_mean_val >> 1);
                    if (weight < 0)
                    {
                        continue;
                    }

                    count = 0;
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i - 1)*IMG_W + (j - 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i - 1)*IMG_W + (j)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i - 1)*IMG_W + (j + 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i)*IMG_W + (j - 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i)*IMG_W + (j + 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i + 1)*IMG_W + (j - 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i + 1)*IMG_W + (j)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] < TmpOrigin[(i + 1)*IMG_W + (j + 1)])
                    {
                        count++;
                    }

                    if (count > 1)
                    {
                        continue;
                    }

                    OutImg[i * IMG_W + j] = (TmpOrigin[i * IMG_W + j] * m_MagBuffer[i * IMG_W + j] + mean_val * (255 - m_MagBuffer[i * IMG_W + j])) / 255;
                }
            }
        }
        memcpy(TmpOrigin, OutImg, IMG_SIZE);
    }

    // remove black noise
    for (loop = 0; loop < 1; loop++)
    {
        for (i = TOP; i <= BOTTOM; i++)
        {
            for (j = LEFT; j <= RIGHT; j++)
            {
                mean_val = (TmpOrigin[(i - 1) * IMG_W + (j - 1)] +
                            TmpOrigin[(i - 1) * IMG_W + j] +
                            TmpOrigin[(i - 1) * IMG_W + (j + 1)] +
                            TmpOrigin[(i) * IMG_W + (j - 1)] +
                            TmpOrigin[(i) * IMG_W + (j + 1)] +
                            TmpOrigin[(i + 1) * IMG_W + (j - 1)] +
                            TmpOrigin[(i + 1) * IMG_W + j] +
                            TmpOrigin[(i + 1) * IMG_W + (j + 1)]) >> 3;

                // black noise
                if (TmpOrigin[i * IMG_W + j] < mean_val)
                {
                    count = 0;
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i - 1)*IMG_W + (j - 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i - 1)*IMG_W + (j)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i - 1)*IMG_W + (j + 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i)*IMG_W + (j - 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i)*IMG_W + (j + 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i + 1)*IMG_W + (j - 1)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i + 1)*IMG_W + (j)])
                    {
                        count++;
                    }
                    if (TmpOrigin[i * IMG_W + j] > TmpOrigin[(i + 1)*IMG_W + (j + 1)])
                    {
                        count++;
                    }

                    if (count > 4)
                    {
                        continue;
                    }

                    OutImg[i * IMG_W + j] = (TmpOrigin[i * IMG_W + j] * m_MagBuffer[i * IMG_W + j] + mean_val * (255 - m_MagBuffer[i * IMG_W + j])) / 255;
                }
            }
        }
        memcpy(TmpOrigin, OutImg, IMG_SIZE);
    }

    for (i = TOP; i <= BOTTOM; i++)
    {
        for (j = LEFT; j <= RIGHT; j++)
        {
            // 2013-04-03 Gon add : increase speed
            if (OutImg[i * IMG_W + j] == 255)
            {
                continue;
            }

            val = (TmpOrigin[(i - 1) * IMG_W + (j - 1)] + // 1A
                   (TmpOrigin[(i - 1) * IMG_W + j] << 3) + // 8B
                   TmpOrigin[(i - 1) * IMG_W + (j + 1)] + // 1C
                   (TmpOrigin[i * IMG_W + (j - 1)] << 3) + // 8D
                   (TmpOrigin[i * IMG_W + j] << 5) + // 32E
                   (TmpOrigin[i * IMG_W + (j + 1)] << 3) + // 8F
                   TmpOrigin[(i + 1) * IMG_W + (j - 1)] + // 1G
                   (TmpOrigin[(i + 1) * IMG_W + j] << 3) + // 8H
                   TmpOrigin[(i + 1) * IMG_W + (j + 1)] // 1I
                  ) / 68;

            val = (int)TmpOrigin[i * IMG_W + j] - val;
            if (val < 0)
            {
                val = val >> 1;
            }
            val = (int)TmpOrigin[i * IMG_W + j] + (val<<1);//((val * 256) >> 7);
            if (val > 255)
            {
                val = 255;
            }
            else if (val < 0)
            {
                val = 0;
            }

            OutImg[i * IMG_W + j] = (unsigned char)val;
        }
    }
    memcpy(TmpOrigin, OutImg, IMG_SIZE);

    // sw uniform
    for (loop = 0; loop < 2; loop++)
    {
        for (i = TOP; i <= BOTTOM; i++)
        {
            for (j = LEFT; j <= RIGHT; j++)
            {
                OutImg[i * IMG_W + j] = (TmpOrigin[(i - 1) * IMG_W + (j - 1)] + // 1A
                                         (TmpOrigin[(i - 1) * IMG_W + j] << 3) + // 8B
                                         TmpOrigin[(i - 1) * IMG_W + (j + 1)] + // 1C
                                         (TmpOrigin[i * IMG_W + (j - 1)] << 3) + // 8D
                                         (TmpOrigin[i * IMG_W + j] << 4) + // 32E
                                         (TmpOrigin[i * IMG_W + (j + 1)] << 3) + // 8F
                                         TmpOrigin[(i + 1) * IMG_W + (j - 1)] + // 1G
                                         (TmpOrigin[(i + 1) * IMG_W + j] << 3) + // 8H
                                         TmpOrigin[(i + 1) * IMG_W + (j + 1)] // 1I
                                        ) / 52;
            }
        }
        memcpy(TmpOrigin, OutImg, IMG_SIZE);

        if (loop == 1)
        {
            break;
        }

        for (i = TOP; i <= BOTTOM; i++)
        {
            for (j = LEFT; j <= RIGHT; j++)
            {
                // 2013-04-03 Gon add : increase speed
                if (OutImg[i * IMG_W + j] == 255)
                {
                    continue;
                }

                val = (TmpOrigin[(i - 1) * IMG_W + (j - 1)] + // 1A
                       (TmpOrigin[(i - 1) * IMG_W + j] << 3) + // 8B
                       TmpOrigin[(i - 1) * IMG_W + (j + 1)] + // 1C
                       (TmpOrigin[i * IMG_W + (j - 1)] << 3) + // 8D
                       (TmpOrigin[i * IMG_W + j] << 5) + // 32E
                       (TmpOrigin[i * IMG_W + (j + 1)] << 3) + // 8F
                       TmpOrigin[(i + 1) * IMG_W + (j - 1)] + // 1G
                       (TmpOrigin[(i + 1) * IMG_W + j] << 3) + // 8H
                       TmpOrigin[(i + 1) * IMG_W + (j + 1)] // 1I
                      ) / 68;

                val = (int)TmpOrigin[i * IMG_W + j] - val;
                if (val < 0)
                {
                    val = val >> 1;
                }
                val = (int)TmpOrigin[i * IMG_W + j] + (val<<1);//((val * 256) >> 7);
                if (val > 255)
                {
                    val = 255;
                }
                else if (val < 0)
                {
                    val = 0;
                }

                OutImg[i * IMG_W + j] = (unsigned char)val;
            }
        }
        memcpy(TmpOrigin, OutImg, IMG_SIZE);
    }

    for (i = TOP; i <= BOTTOM; i++)
    {
        for (j = LEFT; j <= RIGHT; j++)
        {
            // 2013-04-03 Gon add : increase speed
            if (OutImg[i * IMG_W + j] == 255)
            {
                continue;
            }

            val = (TmpOrigin[(i - 1) * IMG_W + (j - 1)] + // 1A
                   (TmpOrigin[(i - 1) * IMG_W + j] << 3) + // 8B
                   TmpOrigin[(i - 1) * IMG_W + (j + 1)] + // 1C
                   (TmpOrigin[i * IMG_W + (j - 1)] << 3) + // 8D
                   (TmpOrigin[i * IMG_W + j] << 5) + // 32E
                   (TmpOrigin[i * IMG_W + (j + 1)] << 3) + // 8F
                   TmpOrigin[(i + 1) * IMG_W + (j - 1)] + // 1G
                   (TmpOrigin[(i + 1) * IMG_W + j] << 3) + // 8H
                   TmpOrigin[(i + 1) * IMG_W + (j + 1)] // 1I
                  ) / 68;

            val = (int)TmpOrigin[i * IMG_W + j] - val;
            if (val < 0)
            {
                val = val >> 1;
            }
            val = (int)TmpOrigin[i * IMG_W + j] + (val*6);//((val * 768) >> 7);
            if (val > 255)
            {
                val = 255;
            }
            else if (val < 0)
            {
                val = 0;
            }

            OutImg[i * IMG_W + j] = (unsigned char)val;
        }
    }

    for (i = 0; i < IMG_SIZE; i++)
    {
        OutImg[i] = 255 - OutImg[i];
    }
}

void CIBAlgorithm::_Algo_HistogramNormalize(unsigned char *InImg, unsigned char *OutImg, int imgSize, int contrastTres)
{
    int y, gray = 0;
    int histogram[256];
    int LUT[256], value;
    //	double scale_factor;

    // enzyme add 2012-11-29 for blank image processing
    if (m_cImgAnalysis.LEFT == m_cImgAnalysis.RIGHT && m_cImgAnalysis.TOP == m_cImgAnalysis.BOTTOM)
    {
        memcpy(OutImg, InImg, imgSize);
        return;
    }

    memset(histogram, 0, sizeof(histogram));
    memset(LUT, 0, 256 * sizeof(int));
    for (y = 0; y < imgSize; y++)
    {
        gray = InImg[y];
        histogram[gray]++;
    }

    int maxidx = 0, maxsum = 0;
    for (y = 255; y >= 0; y--)
    {
        maxsum += histogram[y];
        if (maxsum > 100)
        {
            maxidx = y;
            break;
        }
    }

    int Thres = 230 + contrastTres * 5;
    // enzyme modify 2012-11-29 for blank image processing
    // 2013-43-03 gon modify : low bright image doesn't have to enhance.
    if (maxidx > 10 && maxidx < Thres)
    {
        for (y = maxidx; y < 256; y++)
        {
            LUT[y] = Thres;
        }

        for (y = 0; y < maxidx; y++)
        {
            value = (int)(y * Thres / maxidx);
            if (value > 255)
            {
                value = 255;
            }
            LUT[y] = value;
        }
    }
    else
    {
        memcpy(OutImg, InImg, imgSize);
        return;
    }

    for (y = 0; y < imgSize; y++)
    {
        OutImg[y] = LUT[InImg[y]];
    }
}

int CIBAlgorithm::_Algo_GetBright_Simple(unsigned char *InImg, int imgSize, int cutTres)
{
    // 밝기 계산
    int mean = 0, count = 0;
    unsigned char val;

    for (int i = 0; i < imgSize; i++)
    {
        val = InImg[i];
        if (val > cutTres)
        {
            mean += val;
            count++;
        }
    }

    if (count > 0)
    {
        mean /= count;
    }

    return mean;
}

int CIBAlgorithm::_Algo_FindExpectContrast(unsigned char *InImg, unsigned char *TmpImg, int imgSize, int contrastTres, int nGammaLevel)
{
    double		diff_val = 0;
    int			expect_contrast = 0;
    int			mean_sum = 0;
    int			targetBright;
    int			Bright_Before[48];
    int			start_level, end_level;

    targetBright = 90 + contrastTres * 7;

    start_level = 10 + contrastTres, end_level = 20 + contrastTres;
    for (int loop_count = start_level; loop_count <= end_level; loop_count += (end_level - start_level))
    {
        _Algo_Image_Gamma(InImg, TmpImg, loop_count);
        // 밝기 계산
        Bright_Before[loop_count] = _Algo_GetBright_Simple(TmpImg, imgSize, 10);
    }

    diff_val = (Bright_Before[end_level] - Bright_Before[start_level]) / 10.0;
    if (diff_val != 0)
    {
        expect_contrast = 20 + (int)((targetBright - Bright_Before[end_level]) / (double)diff_val + 0.5);
    }
    else
    {
        expect_contrast = nGammaLevel;
    }
    if (expect_contrast > 47)
    {
        expect_contrast = 47;
    }
    else if (expect_contrast < 0)
    {
        expect_contrast = 0;
    }

    _Algo_Image_Gamma(InImg, TmpImg, expect_contrast);
    // 밝기 계산
    mean_sum = _Algo_GetBright_Simple(TmpImg, imgSize, 10);

    if (expect_contrast < start_level)
    {
        diff_val = (Bright_Before[end_level] - mean_sum) / 10.0;
    }

    if (diff_val != 0)
    {
        expect_contrast += (int)((targetBright - mean_sum) / (double)diff_val + 0.5);
    }
    else
    {
        expect_contrast = nGammaLevel;
    }
    if (expect_contrast > 47)
    {
        expect_contrast = 47;
    }
    else if (expect_contrast < 0)
    {
        expect_contrast = 0;
    }

    return expect_contrast;
}

int CIBAlgorithm::_PostImageProcessing_ForRolling(unsigned char *InImg, int frameCount, BOOL *isComplete)
{
    int		ret;
    BOOL	bPrev_Rolling_Complete;

//	int tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//	int tmpCIS_IMG_H = CIS_IMG_H_ROLL;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;

	m_nframeCount = frameCount;

    *isComplete = FALSE;
    if (frameCount <= 2)
    {
		memset(m_capture_rolled_best_buffer, 0, tmpCIS_IMG_SIZE);
		memset(m_capture_rolled_local_best_buffer, 0, tmpCIS_IMG_SIZE);
		memcpy(m_capture_rolled_buffer, InImg, tmpCIS_IMG_SIZE);

        NewRoll_Init_Rolling();
        NewRoll_CheckRollStartCondition(InImg);

		m_ignore_smear = 0;

		/*m_ContrastofFingerprint = SmearDetection_Calibration(InImg);
		if(m_ContrastofFingerprint > 20)
			m_ContrastofFingerprint = 20;
		if(m_ContrastofFingerprint < 5) 
			m_ContrastofFingerprint = 5;*/
		m_ContrastofFingerprint = 20;	// fixed
		NewRoll_GetWidth(InImg, &g_RollingWidth);
#ifdef __G_DEBUG__
		TRACE(_T("\n\n==>m_ContrastofFingerprint W Lock(%d)\n\n"),m_ContrastofFingerprint);
#endif
		return -1;
    }

	if(m_pPropertyInfo->bStartRollWithoutLock == TRUE && frameCount <= 5 && g_Rolling_Saved_Complete == FALSE)		// frameCount=4 at the first time for wo lock mode
	{
		memset(m_capture_rolled_best_buffer, 0, tmpCIS_IMG_SIZE);
		memset(m_capture_rolled_local_best_buffer, 0, tmpCIS_IMG_SIZE);
		memcpy(m_capture_rolled_buffer, InImg, tmpCIS_IMG_SIZE);

		NewRoll_GetFingerPosition(InImg, &g_LastX, &g_LastY);
		m_ignore_smear = 0;
		m_ContrastofFingerprint = 20;	// fixed
		NewRoll_GetWidth(InImg, &g_RollingWidth);

#ifdef __G_DEBUG__
		TRACE(_T("\n\n==>m_ContrastofFingerprint WO Lock(%d)\n\n"),m_ContrastofFingerprint);
#endif
	}

    bPrev_Rolling_Complete = g_Rolling_Complete;
    ret = NewRoll_MergeImage(m_capture_rolled_buffer, InImg, m_capture_rolled_best_buffer);
    if (!bPrev_Rolling_Complete && g_Rolling_Complete)
    {
        *isComplete = TRUE;
    }
/*
#ifdef __G_DEBUG__
	TRACE(_T("==> bDectected : %d, Hor : %d, Ver : %d, bBestDectected : %d, Hor : %d, Ver : %d\n"), 
				bSmearDectected, bSmearDirectionHor, bSmearDirectionVer,
				bBestFrameSmearDectected, bBestFrameSmearDirectionHor, bBestFrameSmearDirectionVer);
#endif

#ifdef __G_DEBUG__
	TRACE(_T("ret(%d), DiffX(%d), TotalArea(%d), g_Rolling_BackTrace(%d), g_Rolling_Saved_Complete(%d), g_Rolling_Complete(%d)\n"),
			ret, abs(g_StartX - g_EndX), TotalCount2, g_Rolling_BackTrace, g_Rolling_Saved_Complete, g_Rolling_Complete);
#endif
*/
    if (ret == ROLL_COMPLETE || ret == ROLL_COMPLETE_BACKTRACE ||
            (ret == NO_FINGER && (g_Rolling_Saved_Complete == TRUE || g_Rolling_Complete == TRUE)))
    {
        if (ret == ROLL_COMPLETE)
        {
       		memcpy(InImg, m_capture_rolled_local_best_buffer, tmpCIS_IMG_SIZE);
            return IBSU_STATUS_OK;
        }

        return -1;
    }

    memcpy(InImg, m_capture_rolled_buffer, tmpCIS_IMG_SIZE);
    return -1;
}

void CIBAlgorithm::NewRoll_Init_Rolling()
{
    g_StartX = -1;
    g_StartY = -1;
    g_EndX = -1;
    g_EndY = -1;
    g_LastX = -1;
    g_LastY = -1;
    g_Stitch_Dir = 0;
    g_StitchCount = 0;
    g_Rolling_Complete = FALSE;
    g_Rolling_Saved_Complete = FALSE;
    g_Rolling_BackTrace = FALSE;

    prev_bright = -1;
    prev_count = -1;
    prev_pos_x = -1;
    prev_pos_y = -1;
    isPassedCount = 0;
	g_SmearDetectCount = 0;
	m_ContrastofFingerprint = 50;		// 0 ~ 75 : 보통 Watson = 50~60, Sherlcok = 70 이상
	bSmearDectected = FALSE;
	bBestFrameSmearDectected = FALSE;
	nInvalidAreaDetected = 0;
	m_best_roll_width = 0;

	bSmearDirectionHor = FALSE;
	bSmearDirectionVer = FALSE;
	bBestFrameSmearDirectionHor = FALSE;
	bBestFrameSmearDirectionVer = FALSE;
}

int CIBAlgorithm::NewRoll_CheckRollStartCondition(unsigned char *InImg)
{
    int i, j;
    int bright, count, pos_x, pos_y, value;
    int diff_bright, diff_pos_x, diff_pos_y;
	int Minus_Val = FOREGROUND_BRIGHT_VALUE;

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

	int tmpCIS_IMG_W = CIS_IMG_W_ROLL;
	int tmpCIS_IMG_H = CIS_IMG_H_ROLL;

	dtLEFT		= m_cImgAnalysis.LEFT;
    dtRIGHT		= m_cImgAnalysis.RIGHT;
    dtTOP		= m_cImgAnalysis.TOP;
    dtBOTTOM	= m_cImgAnalysis.BOTTOM;

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK ||
		m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
	{
		Minus_Val = m_SBDAlg->m_Roll_Minus_Val;
	}

    pos_x = pos_y = bright = 0;
    count = 0;

	for (i = dtTOP; i < dtBOTTOM; i+=2)
	{
		for (j = dtLEFT; j < dtRIGHT; j+=2)
		{
			value = InImg[i * tmpCIS_IMG_W + j];

			if (value > Minus_Val)
			{
				value >>= 4;
				pos_x += j * value;
				pos_y += i * value;
				bright += value;
				count++;
			}
		}
	}
	if (bright > 0)
	{
		pos_x /= bright;
		pos_y /= bright;
		bright /= count;
	}

	if (pos_x < 0 || pos_x > tmpCIS_IMG_W - 1 || pos_y < 0 || pos_y > tmpCIS_IMG_H - 1)
	{
		NewRoll_Init_Rolling();
		return NO_FINGER;
	}

    diff_bright = abs(bright - prev_bright);
    diff_pos_x = abs(pos_x - prev_pos_x);
    diff_pos_y = abs(pos_y - prev_pos_y);

    prev_bright = bright;
    prev_pos_x = pos_x;
    prev_pos_y = pos_y;

    if (diff_bright > THRESHOLD_DIFF_BRIGHT ||
            diff_pos_x > THRESHOLD_POSITION_X ||
            diff_pos_y > THRESHOLD_POSITION_Y ||
            count < REMOVAL_SMALL_AREA)
    {
        isPassedCount = 0;
        return NO_FINGER;
    }

    if (isPassedCount < 3)
    {
        isPassedCount++;
        return FINGER_DETECTED;
    }

    NewRoll_GetFingerPosition(InImg, &g_StartX, &g_StartY);

	g_Prev_MergePosX = g_StartX;

	m_Prev_NewPosX = g_StartX;
	m_Prev_NewPosY = g_StartY;

    // 시작시에는 모든 위치들이 같다.
    g_EndX = g_StartX;
    g_EndY = g_StartY;
    g_LastX = g_StartX;
    g_LastY = g_StartY;

    return SUCCESS;
}

int CIBAlgorithm::NewRoll_GetFingerPosition(unsigned char *InImg, int *PosX, int *PosY)
{
    int i, j;
    unsigned int Sum_X, Sum_Y;
	int TotalBright, TotalCount, value;
	int Minus_Val = FOREGROUND_BRIGHT_VALUE;

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

    int tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//	int tmpCIS_IMG_H = CIS_IMG_H_ROLL;

	dtLEFT		= m_cImgAnalysis.LEFT;
    dtRIGHT		= m_cImgAnalysis.RIGHT;
    dtTOP		= m_cImgAnalysis.TOP;
    dtBOTTOM	= m_cImgAnalysis.BOTTOM;

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
	{
		Minus_Val = m_SBDAlg->m_Roll_Minus_Val;
	}
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
	{
		Minus_Val = 10;
	}

    *PosX = -1;
    *PosY = -1;

    Sum_X = 0;
    Sum_Y = 0;
    TotalBright = 0;
    TotalCount = 0;

	for (i = dtTOP; i < dtBOTTOM; i+=4)
	{
		for (j = dtLEFT; j < dtRIGHT; j+=4)
		{
			value = InImg[i * tmpCIS_IMG_W + j];
			if (value > Minus_Val)
			{
//				value >>= 4;
				Sum_X += j * value;
				Sum_Y += i * value;
				TotalBright += value;
				TotalCount++;
			}
		}
	}

	if(TotalBright > 0)
	{
		*PosX = Sum_X / TotalBright;
		*PosY = Sum_Y / TotalBright;
	}

    if (TotalCount < REMOVAL_SMALL_AREA)
    {
        return NO_FINGER;
    }

    return SUCCESS;
}

int CIBAlgorithm::NewRoll_MergeImage(unsigned char *MergeImg, unsigned char *InputImg, unsigned char *BestMergeImg)
{
    int i, j;
    int SmoothingRange;
    int MergePosX;
    int NewPosX, NewPosY;
    int dx, dy, dist, width;
    int smear3, Smear3_Threshold;
	int Threshold_good_roll_x;
	int roll_dist = 0;
	int Minus_Val = FOREGROUND_BRIGHT_VALUE;
//	int ClearAll = FALSE;

	int tmpIMG_W = IMG_W_ROLL;
//	int tmpIMG_H = IMG_H_ROLL;
	int tmpCIS_IMG_W = CIS_IMG_W_ROLL;
	int tmpCIS_IMG_H = CIS_IMG_H_ROLL;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	int SmearJudgeCount = 1;

	int dtLEFT, dtRIGHT, dtTOP, dtBOTTOM;

    dtLEFT		= m_cImgAnalysis.LEFT;
    dtRIGHT		= m_cImgAnalysis.RIGHT;
    dtTOP		= m_cImgAnalysis.TOP;
    dtBOTTOM	= m_cImgAnalysis.BOTTOM;

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
	{
		Minus_Val = m_SBDAlg->m_Roll_Minus_Val;
	}
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
	{
		Minus_Val = 10;
	}

	Threshold_good_roll_x = (int)(m_pPropertyInfo->nRollMinWidth * tmpCIS_IMG_W / 40.64);
	if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
		Threshold_good_roll_x = (int)(m_pPropertyInfo->nRollMinWidth * (tmpCIS_IMG_W-30) / 40.64);

	int nTHRESHOLD_DIST_BTW_FINGERS = THRESHOLD_DIST_BTW_FINGERS;
	if(m_pUsbDevInfo->devType == DEVICE_TYPE_SHERLOCK || m_pUsbDevInfo->devType == DEVICE_TYPE_SHERLOCK_ROIC || m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
		nTHRESHOLD_DIST_BTW_FINGERS = (int)(THRESHOLD_DIST_BTW_FINGERS*0.8);
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
		nTHRESHOLD_DIST_BTW_FINGERS = (int)(THRESHOLD_DIST_BTW_FINGERS*0.8);

    // 입력 영상의 지문 위치 계산
    int ret = NewRoll_GetFingerPosition(InputImg, &NewPosX, &NewPosY);

    // Stitch 실패 및 끝
    if (m_cImgAnalysis.finger_count == 0 || ret == NO_FINGER)
    {
        if ( m_cImgAnalysis.finger_count == 0 && (g_Rolling_Complete == TRUE || g_Rolling_Saved_Complete == TRUE) )
        {
            return ROLL_COMPLETE;
        }

        m_ignore_smear = 1;
    }

    // 지문을 연결할 위치 계산
    MergePosX = (g_LastX + NewPosX) >> 1;

    // 지문의 회전속도에 따른 Smoothing 영역 계산
    dx = abs(g_LastX - NewPosX);
    dy = abs(g_LastY - NewPosY);
    dist = (int)sqrt((double)dx * dx + (double)dy * dy);
    SmoothingRange = PIXEL_ASSIGN_RANGE + PIXEL_INCREASE_RANGE * (dist >> 3);
    if (SmoothingRange > PIXEL_MAX_RANGE)
    {
        SmoothingRange = PIXEL_MAX_RANGE;
    }

    // 초기 이동 방향을 결정한다.
    if (g_Stitch_Dir == 0)
    {
        if (dx > 0) // 이동이 존재하면
        {
			g_StitchCount = 0;

            if (g_LastX < NewPosX)
            {
                g_Stitch_Dir = 1;
            }
            else
            {
                g_Stitch_Dir = -1;
            }
        }

        g_Rolling_Complete = FALSE;
        g_StartX = g_EndX = g_LastX = NewPosX;
        g_StartY = g_EndY = g_LastY = NewPosY;

		memcpy(MergeImg, InputImg, tmpCIS_IMG_SIZE);
		memcpy(BestMergeImg, InputImg, tmpCIS_IMG_SIZE);

        m_first_brightness = NewRoll_GetBright_Simple(InputImg, g_StartX);
		g_Prev_MergePosX = MergePosX;

		m_Prev_NewPosX = NewPosX;
		m_Prev_NewPosY = NewPosY;

		return SUCCESS;
    }
    else if (g_Stitch_Dir == 1)
    {
        if (g_LastX - 2 > NewPosX)
        {
            g_Stitch_Dir = -1;
			g_StitchCount = 0;
            g_StartX = g_EndX = g_LastX;
            g_StartY = g_EndY = g_LastY;
			
            if (g_Rolling_Complete == TRUE)
            {
                g_Rolling_Complete = FALSE;
                g_Rolling_BackTrace = TRUE;
				
				memcpy(MergeImg, InputImg, tmpCIS_IMG_SIZE);
				memcpy(m_capture_rolled_local_best_buffer, BestMergeImg, tmpCIS_IMG_SIZE);
				memset(BestMergeImg, 0, tmpCIS_IMG_SIZE);

                m_first_brightness = NewRoll_GetBright_Simple(InputImg, g_StartX);
				g_Prev_MergePosX = MergePosX;

				m_Prev_NewPosX = NewPosX;
				m_Prev_NewPosY = NewPosY;

				bSmearDectected = FALSE;
				bSmearDirectionHor = FALSE;
				bSmearDirectionVer = FALSE;

                return ROLL_COMPLETE_BACKTRACE;
            }
			else
			{
				memcpy(MergeImg, InputImg, tmpCIS_IMG_SIZE);
				m_first_brightness = NewRoll_GetBright_Simple(InputImg, g_StartX);
				return SUCCESS;
			}
        }
    }
    else if (g_Stitch_Dir == -1)
    {
        if (g_LastX + 2 < NewPosX)
        {
            g_Stitch_Dir = 1;
			g_StitchCount = 0;
            g_StartX = g_EndX = g_LastX;
            g_StartY = g_EndY = g_LastY;

            if (g_Rolling_Complete == TRUE)
            {
                g_Rolling_Complete = FALSE;
                g_Rolling_BackTrace = TRUE;

				memcpy(MergeImg, InputImg, tmpCIS_IMG_SIZE);
				memcpy(m_capture_rolled_local_best_buffer, BestMergeImg, tmpCIS_IMG_SIZE);
				memset(BestMergeImg, 0, tmpCIS_IMG_SIZE);

                m_first_brightness = NewRoll_GetBright_Simple(InputImg, g_StartX);
				g_Prev_MergePosX = MergePosX;

				m_Prev_NewPosX = NewPosX;
				m_Prev_NewPosY = NewPosY;

				bSmearDectected = FALSE;
				bSmearDirectionHor = FALSE;
				bSmearDirectionVer = FALSE;

                return ROLL_COMPLETE_BACKTRACE;
            }
			else
			{
				memcpy(MergeImg, InputImg, tmpCIS_IMG_SIZE);
				m_first_brightness = NewRoll_GetBright_Simple(InputImg, g_StartX);
				return SUCCESS;
			}
        }
    }

	if(m_pPropertyInfo->nRollMode == ROLL_MODE_USE_NOTICE)
	{
		// new logic //////////////////////////////////////////////////////////////////////////////////////
		// Smear Detection
		if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK ||
			m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
		{
			if( g_StitchCount >= 2 )
				smear3 = SmearDetectionOfFinger(MergeImg, InputImg, MergePosX);
			else
				smear3 = 0;

/*			if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
			{
#ifdef __G_DEBUG__
				TRACE("Original smear3 : %d --> ", smear3 );
#endif
				if(m_segment_arr.Segment[0].Area < 1500)
					smear3 = (int)(smear3 * 0.9f);
				else if(m_segment_arr.Segment[0].Area < 2000)
					smear3 = (int)(smear3 * 0.95f);
				else
					smear3 = smear3;

				if(g_Rolling_Complete == TRUE)
				{
					smear3 = (int)(smear3 * 0.95f);
				}
				else if(m_pPropertyInfo->bStartRollWithoutLock == TRUE && (m_nframeCount < 12 || g_Stitch_Dir == 0))
				{
					if(m_nframeCount < 8)
						smear3 = (int)(smear3 * 0.85f);
					else if(m_nframeCount < 12)
						smear3 = (int)(smear3 * 0.90f);
					else
						smear3 = (int)(smear3 * 0.95f);
				}
			}*/
		}
		else
			smear3 = SmearDetectionOfFinger(MergeImg, InputImg, MergePosX);

		if(m_ContrastofFingerprint < 10)
			Smear3_Threshold = (int)(m_ContrastofFingerprint + m_ContrastofFingerprint * abs(g_Prev_MergePosX - MergePosX) / (nTHRESHOLD_DIST_BTW_FINGERS*1.5));
		else if(m_ContrastofFingerprint < 15)
			Smear3_Threshold = (int)(m_ContrastofFingerprint + m_ContrastofFingerprint*0.85 * abs(g_Prev_MergePosX - MergePosX) / (nTHRESHOLD_DIST_BTW_FINGERS*1.5));
		else
		{
			if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
				Smear3_Threshold = (int)(m_ContrastofFingerprint + m_ContrastofFingerprint * abs(g_Prev_MergePosX - MergePosX) / (nTHRESHOLD_DIST_BTW_FINGERS*1.5));
			else
				Smear3_Threshold = (int)(m_ContrastofFingerprint + m_ContrastofFingerprint*0.7 * abs(g_Prev_MergePosX - MergePosX) / (nTHRESHOLD_DIST_BTW_FINGERS*1.5));
		}

//		if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
//			Smear3_Threshold = (int)(Smear3_Threshold*1.25);

		if(Smear3_Threshold > m_ContrastofFingerprint*1.65)
			Smear3_Threshold = (int)(m_ContrastofFingerprint*1.65);

		if(m_pPropertyInfo->nRollLevel == ROLL_LEVEL_LOW)
		{
			Smear3_Threshold = Smear3_Threshold + (int)(m_ContrastofFingerprint * 0.4);
		}
		else if(m_pPropertyInfo->nRollLevel == ROLL_LEVEL_HIGH)
		{
			Smear3_Threshold = Smear3_Threshold - (int)(m_ContrastofFingerprint * 0.1);
		}

		roll_dist = abs(g_LastX-NewPosX) * tmpIMG_W / tmpCIS_IMG_W;

#ifdef __G_DEBUG__
	TRACE(_T("smear3=%d, Smear3_Threshold=%d, Complete=%d, BackTrace=%d, Saved_Complete=%d, dist=%d\n"), smear3, Smear3_Threshold, 
					g_Rolling_Complete, g_Rolling_BackTrace, g_Rolling_Saved_Complete, roll_dist);
#endif

		// Is detected smear ?
		if(/*m_ignore_smear == 0 && */( smear3 > Smear3_Threshold || roll_dist > nTHRESHOLD_DIST_BTW_FINGERS) )
		{
			g_SmearDetectCount++;

			// smear3가 100이면 (밝기가 어둡거나, 갑작스런 작은 영상 등 불규칙한 영상일 경우)
			if(smear3 == 100)
				bSmearDectected = TRUE;
			
			// 충분한 Rolling이 되지 않았는데 Smear가 detect된 경우
			NewRoll_GetWidth(MergeImg, &width);
			g_RollingWidth = width;
			if( width < Threshold_good_roll_x && g_SmearDetectCount > 0 )
			{
				bSmearDectected = TRUE;
			}

			if(g_SmearDetectCount >= SmearJudgeCount)
			{
				bSmearDectected = TRUE;
			}

			// 이동거리가 너무 멀면 롤링 끝
			if( roll_dist > nTHRESHOLD_DIST_BTW_FINGERS )// && g_SmearDetectCount >= 3)
				bSmearDectected = TRUE;

#ifdef __G_DEBUG__
			if(bSmearDectected == FALSE)
				TRACE(_T("\nSmear is not Detected : %d\n"), g_SmearDetectCount);
			else
				TRACE(_T("\nSmear is Detected : %d\n"), g_SmearDetectCount);
#endif

			if(bSmearDectected == TRUE)
			{
				m_Last_NewPosX = NewPosX;
				m_Last_NewPosY = NewPosY;

				// 기울기를 검사한다.
				int dx = abs(m_Prev_NewPosX - m_Last_NewPosX);
				int dy = abs(m_Prev_NewPosY - m_Last_NewPosY);
				int angle = (int)(atan2((double)dy, (double)dx) * 180 / 3.14159265);
//				TRACE("\n==> angle : %d\n", angle);

				if(angle <= 40)
					bSmearDirectionHor |= TRUE;
				
				if(angle > 40)
					bSmearDirectionVer |= TRUE;

				m_Prev_NewPosX = m_Last_NewPosX;
				m_Prev_NewPosY = m_Last_NewPosY;

				// 최종 연결 영상과 지금까지 가장 크게 연결된 영상간의 크기 비교
				TotalCount1 = TotalCount2 = 0;
				for (i = dtTOP; i < dtBOTTOM; i+=8)
				{
					for (j = dtLEFT; j < dtRIGHT; j+=8)
					{
						if (MergeImg[i*tmpCIS_IMG_W+j] > Minus_Val)
						{
							TotalCount1++;
						}

						if (BestMergeImg[i*tmpCIS_IMG_W+j] > Minus_Val)
						{
							TotalCount2++;
						}
					}
				}

				// 최종 연결된 영상이 더 크면 best 프레임으로 저장
				if (TotalCount2 < TotalCount1)
				{
					if (g_Rolling_BackTrace == FALSE)
					{
						bBestFrameSmearDectected = TRUE;

						if( bSmearDirectionHor )
							bBestFrameSmearDirectionHor = TRUE;
						else
							bBestFrameSmearDirectionHor = FALSE;

						if( bSmearDirectionVer )
							bBestFrameSmearDirectionVer = TRUE;
						else
							bBestFrameSmearDirectionVer = FALSE;
					}
				}
			}
		}
		else
		{
			//TRACE("\n");
			g_SmearDetectCount = 0;
			m_Prev_NewPosX = NewPosX;
			m_Prev_NewPosY = NewPosY;
			m_Last_NewPosX = NewPosX;
			m_Last_NewPosY = NewPosY;
		}
	}

	m_ignore_smear = 0;

/*	// 이동거리가 너무 멀면 롤링 끝
	if( abs(g_LastX-NewPosX) > nTHRESHOLD_DIST_BTW_FINGERS )
	{
		if (g_Rolling_Complete == TRUE || g_Rolling_Saved_Complete == TRUE)
        {
            return ROLL_COMPLETE;
        }
		
		*m_nPGoodFrameCount = 0;
		return NO_FINGER;
	}
*/
    // 지문 Stitch
    if (NewRoll_Stitch_TwoImage(MergeImg, InputImg, m_Inter_Img3, MergePosX, g_Stitch_Dir, SmoothingRange) == NO_FINGER)
    {
        if (g_Rolling_Complete == TRUE || g_Rolling_Saved_Complete == TRUE)
        {
            return ROLL_COMPLETE;
        }

		*m_nPGoodFrameCount = 0;
        return NO_FINGER;
    }

	g_StitchCount++;

    // 지문 연결 후 연결한 위치 저장
    g_LastX = NewPosX;
    g_LastY = NewPosY;
	g_Prev_MergePosX = MergePosX;
	g_Prev_MergePosX = MergePosX;

    // 최종 연결 영상과 지금까지 가장 크게 연결된 영상간의 크기 비교
    TotalCount1 = TotalCount2 = 0;
	for (i = 0; i < tmpCIS_IMG_H; i+=8)
	{
		for (j = 0; j < tmpCIS_IMG_W; j+=8)
		{
			if (MergeImg[i*tmpCIS_IMG_W+j] > Minus_Val)
			{
				TotalCount1++;
			}

			if (BestMergeImg[i*tmpCIS_IMG_W+j] > Minus_Val)
			{
				TotalCount2++;
			}
		}
	}

    // 최종 연결된 영상이 더 크면 best 프레임으로 저장
    if (TotalCount2 < TotalCount1)
    {
        g_EndX = NewPosX;
        g_EndY = NewPosY;
        TotalCount2 = TotalCount1;

		memcpy(BestMergeImg, MergeImg, tmpCIS_IMG_SIZE);

        if (g_Rolling_BackTrace == FALSE)
		{
			memcpy(m_capture_rolled_local_best_buffer, BestMergeImg, tmpCIS_IMG_SIZE);

			if( bSmearDectected )
				bBestFrameSmearDectected = TRUE;
			else
			{
				bBestFrameSmearDectected = FALSE;
				bBestFrameSmearDirectionHor = FALSE;
				bBestFrameSmearDirectionVer = FALSE;
			}

			if( bSmearDirectionHor )
				bBestFrameSmearDirectionHor = TRUE;
			else
				bBestFrameSmearDirectionHor = FALSE;

			if( bSmearDirectionVer )
				bBestFrameSmearDirectionVer = TRUE;
			else
				bBestFrameSmearDirectionVer = FALSE;
		}
    }

	NewRoll_GetWidth(MergeImg, &width);
	g_RollingWidth = width;

	if( width > Threshold_good_roll_x )
	{
		g_Rolling_Saved_Complete = TRUE;
		g_Rolling_Complete = TRUE;
		g_Rolling_BackTrace = FALSE;

		m_best_roll_width = width;
	}

//	TRACE("\n==> WIDTH = %d (thres = %d) \n", width, Threshold_good_roll_x);

    // Stitch 성공
    return SUCCESS;
}

int CIBAlgorithm::NewRoll_Stitch_TwoImage(unsigned char *MergeImg, unsigned char *InputImg, unsigned char *TmpImg, int MergePosX, int StitchDir, int SmoothingRange)
{
    int i, j;
    int weight;
	int Copy_StartX, Copy_EndX, Copy_Width;

	int tmpCIS_IMG_W = CIS_IMG_W_ROLL;
	int tmpCIS_IMG_H = CIS_IMG_H_ROLL;

	unsigned char *pUseImg = TmpImg;

    if (NewRoll_Brightness_Control(InputImg, pUseImg, m_first_brightness, MergePosX) == NO_FINGER)
    {
		m_ignore_smear = 1;
        //return NO_FINGER;
    }

    if (StitchDir == 1)
    {
		Copy_StartX = MergePosX + SmoothingRange;
		Copy_EndX = m_cImgAnalysis.RIGHT;
		Copy_Width = (Copy_EndX - Copy_StartX);
		
		if(Copy_Width < 0)
			return TRUE;

		for (i = 0; i < tmpCIS_IMG_H; i++)
		{
			memcpy(&MergeImg[i * tmpCIS_IMG_W + Copy_StartX], &pUseImg[i * tmpCIS_IMG_W + Copy_StartX], Copy_Width);

			for (j = MergePosX - SmoothingRange; j <= MergePosX + SmoothingRange; j++)
			{
				weight = ((j - MergePosX + SmoothingRange) << 7) / (SmoothingRange << 1);
				MergeImg[i * tmpCIS_IMG_W + j] = ((128 - weight) * MergeImg[i * tmpCIS_IMG_W + j] + weight * pUseImg[i * tmpCIS_IMG_W + j]) >> 7;
			}
			for (j = MergePosX - (SmoothingRange >> 1); j <= MergePosX + (SmoothingRange >> 1); j++)
			{
				weight = ((j - MergePosX + (SmoothingRange >> 1)) << 7) / (SmoothingRange);
				MergeImg[i * tmpCIS_IMG_W + j] = ((128 - weight) * MergeImg[i * tmpCIS_IMG_W + j] + weight * pUseImg[i * tmpCIS_IMG_W + j]) >> 7;
			}
			for (j = MergePosX - (SmoothingRange >> 2); j <= MergePosX + (SmoothingRange >> 2); j++)
			{
				weight = ((j - MergePosX + (SmoothingRange >> 2)) << 7) / (SmoothingRange >> 1);
				MergeImg[i * tmpCIS_IMG_W + j] = ((128 - weight) * MergeImg[i * tmpCIS_IMG_W + j] + weight * pUseImg[i * tmpCIS_IMG_W + j]) >> 7;
			}
		}

        return TRUE;
    }
    else if (StitchDir == -1)
    {
		Copy_StartX = m_cImgAnalysis.LEFT;
		Copy_EndX = MergePosX - SmoothingRange;
		Copy_Width = (Copy_EndX - Copy_StartX);

		if(Copy_Width < 0)
			return TRUE;

		for (i = 0; i < tmpCIS_IMG_H; i++)
		{
			memcpy(&MergeImg[i * tmpCIS_IMG_W + Copy_StartX], &pUseImg[i * tmpCIS_IMG_W + Copy_StartX], Copy_Width);

			for (j = MergePosX - SmoothingRange; j <= MergePosX + SmoothingRange; j++)
			{
				weight = ((j - MergePosX + SmoothingRange) << 7) / (SmoothingRange << 1);
				MergeImg[i * tmpCIS_IMG_W + j] = (weight * MergeImg[i * tmpCIS_IMG_W + j] + (128 - weight) * pUseImg[i * tmpCIS_IMG_W + j]) >> 7;
			}
			for (j = MergePosX - (SmoothingRange >> 1); j <= MergePosX + (SmoothingRange >> 1); j++)
			{
				weight = ((j - MergePosX + (SmoothingRange >> 1)) << 7) / (SmoothingRange);
				MergeImg[i * tmpCIS_IMG_W + j] = (weight * MergeImg[i * tmpCIS_IMG_W + j] + (128 - weight) * pUseImg[i * tmpCIS_IMG_W + j]) >> 7;
			}
			for (j = MergePosX - (SmoothingRange >> 2); j <= MergePosX + (SmoothingRange >> 2); j++)
			{
				weight = ((j - MergePosX + (SmoothingRange >> 2)) << 7) / (SmoothingRange >> 1);
				MergeImg[i * tmpCIS_IMG_W + j] = (weight * MergeImg[i * tmpCIS_IMG_W + j] + (128 - weight) * pUseImg[i * tmpCIS_IMG_W + j]) >> 7;
			}
		}

        return TRUE;
    }

    return FALSE;
}

void CIBAlgorithm::NewRoll_GetWidth(unsigned char *MergeImg, int *Width)
{
	int i, j;
	int left=CIS_IMG_W, right=0;
	int thres = 10;
	memset(mean_h, 0, CIS_IMG_W*sizeof(int));

	*Width = 0;

	int tmpCIS_IMG_W = CIS_IMG_W_ROLL;
	int tmpCIS_IMG_H = CIS_IMG_H_ROLL;
//	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
	{
		thres = _NO_BIT_CMOS_NOISE_FOR_FULLFRAME_ROLL-5;//m_SBDAlg->m_Roll_Minus_Val;
	}
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
	{
		thres = _TFT_NOISE_FOR_FIVE0_-5;
	}

	for(i=0; i<tmpCIS_IMG_H; i+=2)
	{
		for(j=0; j<tmpCIS_IMG_W; j++)
		{
			mean_h[j] += MergeImg[i*tmpCIS_IMG_W+j];
		}
	}
	
	for(j=0; j<tmpCIS_IMG_W; j++)
		mean_h[j] /= (tmpCIS_IMG_H/2);

	for(j=0; j<tmpCIS_IMG_W; j++)
	{
		if(mean_h[j] > thres)
		{
			left=j;
			break;
		}
	}

	for(j=tmpCIS_IMG_W-1; j>=0; j--)
	{
		if(mean_h[j] > thres)
		{
			right=j;
			break;
		}
	}

	if(left >= right)
		return;

	*Width = right-left;
}

void CIBAlgorithm::_Algo_ZoomOut_with_Bilinear(unsigned char *InImg, int imgWidth, int imgHeight, unsigned char *OutImg, int outWidth, int outHeight)
{
    // bilinear interpolation
    int i, j, p, q, refy1, refy2, refx1, refx2;
    int value;
    float temp_i;
    // 2013-05-27 enzyme modify - Possible bug fixed in the decimation
    int IMG_H = imgHeight;
    int IMG_W = imgWidth;

    if (imgHeight != m_prevInput_height ||
            imgWidth != m_prevInput_width ||
            outHeight != m_prevZoomout_height ||
            outWidth != m_prevzoomout_width)
    {
        m_prevInput_height = imgHeight;
        m_prevInput_width = imgWidth;
        m_prevZoomout_height = outHeight;
        m_prevzoomout_width = outWidth;
        for (i = 0; i < outHeight; i++)
        {
            temp_i = (float)i * IMG_H / outHeight;
            m_p_arr[i] = (short)((temp_i - (short)temp_i) * 128);
            m_refy1_arr[i] = (int)temp_i;
            m_refy2_arr[i] = (int)temp_i + 1;
            if (m_refy1_arr[i] > IMG_H - 1)
            {
                m_refy1_arr[i] = IMG_H - 1;
            }
            if (m_refy2_arr[i] > IMG_H - 1)
            {
                m_refy2_arr[i] = IMG_H - 1;
            }

            m_refy1_arr[i] = m_refy1_arr[i] * IMG_W;
            m_refy2_arr[i] = m_refy2_arr[i] * IMG_W;
        }
        for (i = 0; i < outWidth; i++)
        {
            temp_i = (float)i * IMG_W / outWidth;
            m_q_arr[i] = (short)((temp_i - (short)temp_i) * 128);
            m_refx1_arr[i] = (int)temp_i;
            m_refx2_arr[i] = (int)temp_i + 1;
            if (m_refx1_arr[i] > IMG_W - 1)
            {
                m_refx1_arr[i] = IMG_W - 1;
            }
            if (m_refx2_arr[i] > IMG_W - 1)
            {
                m_refx2_arr[i] = IMG_W - 1;
            }
        }
    }

    for (i = 0; i < outHeight; i++)
    {
        refy1 = m_refy1_arr[i];
        refy2 = m_refy2_arr[i];
        p = m_p_arr[i];

        for (j = 0; j < outWidth; j++)
        {
            refx1 = m_refx1_arr[j];
            refx2 = m_refx2_arr[j];
            q = m_q_arr[j];

            value = ((128 - p) * ((128 - q) * InImg[refy1 + refx1] + q * InImg[refy1 + refx2]) +
                     p * ((128 - q) * InImg[refy2 + refx1] + q * InImg[refy2 + refx2])) >> 14;

            OutImg[i * outWidth + j] = (unsigned char)value;
        }
    }
}

void CIBAlgorithm::_Algo_JudgeFingerQuality(FrameImgAnalysis *pFrameImgAnalysis, BOOL *bIsGoodImage, PropertyInfo *pPropertyInfo, CallbackProperty *pClbkProperty, BOOL dispInvalidArea)
{
    if (m_cImgAnalysis.finger_count == 0)
    {
        memset(&pClbkProperty->qualityArray[0], ENUM_IBSU_FINGER_NOT_PRESENT, sizeof(pClbkProperty->qualityArray));
        return;
    }
    const int MIN_RANGE = 15;
    const int MAX_RANGE = 60;

    int i, j, k, label;
    int BrightMean[256];
    int LabelCnt[256];
    int LabelPosX[256];
    int LabelPosY[256];
	int LabelLeft[256];
	int LabelRight[256];
	int LabelTop[256];
	int LabelBottom[256];
    int Window_Pos[4];
    FingerQuality *pFingerQuality;
    FingerQuality *pFingerQuality_i, *pFingerQuality_j, FingerQuality_tmp;
    int finger_count;
	int cis_img_w = CIS_IMG_W;
	int cis_img_h = CIS_IMG_H;
	int zoom_enlarge_h = ZOOM_ENLAGE_H;
	int zoom_enlarge_w = ZOOM_ENLAGE_W;
	int enlargesize_zoom_w = ENLARGESIZE_ZOOM_W;
	int enlargesize_zoom_h = ENLARGESIZE_ZOOM_H;
	int img_w = IMG_W;
	int img_h = IMG_H;
	int zoom_w = ZOOM_W;
	int zoom_h = ZOOM_H;
	int invalid_rate = 6;
//	int finger_area_bottom = m_cImgAnalysis.BOTTOM;

	std::vector<FingerQuality *>::iterator it = m_pFingerQuality.begin();
	while( it != m_pFingerQuality.end() )
	{
		if( *it )
		{
			delete *it;
			it = m_pFingerQuality.erase(it);
		}
		else
			++it;
	}

	m_pFingerQuality.clear();

	if((m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK) || (m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0))
	{
		if (pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
		{
//			if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
			{
				cis_img_w			= CIS_IMG_W_ROLL;
				cis_img_h			= CIS_IMG_H_ROLL;
				zoom_enlarge_h		= ZOOM_ENLAGE_H_ROLL;
				zoom_enlarge_w		= ZOOM_ENLAGE_W_ROLL;
				enlargesize_zoom_w	= ENLARGESIZE_ZOOM_W_ROLL;
				enlargesize_zoom_h	= ENLARGESIZE_ZOOM_H_ROLL;
				img_w				= IMG_W_ROLL;
				img_h				= IMG_H_ROLL;
				zoom_w				= ZOOM_W_ROLL;
				zoom_h				= ZOOM_H_ROLL;
				invalid_rate		= 4;
			}
		}
	}
	else if((m_pUsbDevInfo->devType == DEVICE_TYPE_WATSON) || (m_pUsbDevInfo->devType == DEVICE_TYPE_WATSON_MINI))
	{
		invalid_rate		= 5;
	}
	else if((m_pUsbDevInfo->devType == DEVICE_TYPE_SHERLOCK))
	{
		invalid_rate		= 4;
	}
	else if((m_pUsbDevInfo->devType == DEVICE_TYPE_COLUMBO))
	{
		invalid_rate		= 3;
	}
	else
	{
		//invalid_rate		= 6;
		return; // The curve doesn't need this.
	}

    memset(BrightMean, 0, sizeof(BrightMean));
    memset(LabelCnt, 0, sizeof(BrightMean));
    memset(LabelPosX, 0, sizeof(BrightMean));
    memset(LabelPosY, 0, sizeof(BrightMean));
    memset(Window_Pos, 0, sizeof(Window_Pos));
	//memset(&pClbkProperty->qualityArray[0], ENUM_IBSU_FINGER_NOT_PRESENT, sizeof(pClbkProperty->qualityArray));

	for(i=0; i<256; i++)
	{
		LabelLeft[i] = cis_img_w;
		LabelRight[i] = 0;
		LabelTop[i] = cis_img_h;
		LabelBottom[i] = 0;
	}

	for (i = 0; i < zoom_enlarge_h; i++)
    {
        for (j = 0; j < zoom_enlarge_w; j++)
        {
            label = m_labeled_segment_enlarge_buffer[(i * zoom_enlarge_w) + j];

            if (label > 0 && label < 256)
            {
				for(k = 0 ; k < m_segment_arr.SegmentCnt ; k++)
				{
					if(m_segment_arr.Segment[k].Label == label)
					{
						if (EnlargeBuf[i * zoom_enlarge_w + j] > 10)
						{
							LabelPosX[label] += j;
							LabelPosY[label] += i;
							LabelCnt[label]++;
							BrightMean[label] += EnlargeBuf[i * zoom_enlarge_w + j];

							if(LabelLeft[label] > j)	LabelLeft[label] = j;
							if(LabelRight[label] < j)	LabelRight[label] = j;
							if(LabelTop[label] > i)		LabelTop[label] = i;
							if(LabelBottom[label] < i)	LabelBottom[label] = i;
						}
						break;
					}
				}
            }
        }
    }

    for (i = 0; i < 256; i++)
    {
        if (LabelCnt[i] > 0)
        {
            LabelPosX[i] /= LabelCnt[i];
            LabelPosY[i] /= LabelCnt[i];
            BrightMean[i] /= LabelCnt[i];
			pFingerQuality = new FingerQuality;
			pFingerQuality->x = (LabelPosX[i]-enlargesize_zoom_w) * cis_img_w / zoom_w;
			pFingerQuality->y = (LabelPosY[i]-enlargesize_zoom_h) * cis_img_h / zoom_h;
            pFingerQuality->mean = BrightMean[i];
            pFingerQuality->segment_count = LabelCnt[i] * (ZOOM_OUT * ZOOM_OUT);
			pFingerQuality->pos_in_area = pFingerQuality->x * 4 / cis_img_w;
            m_pFingerQuality.push_back(pFingerQuality);
        }
    }

	if(m_pFingerQuality.size() <= 0)
		return;

    for (i = 0; i < (int)m_pFingerQuality.size() - 1; i++)
    {
        pFingerQuality_i = m_pFingerQuality.at(i);
        for (j = i + 1; j < (int)m_pFingerQuality.size(); j++)
        {
            pFingerQuality_j = m_pFingerQuality.at(j);

            if (pFingerQuality_i->x > pFingerQuality_j->x)
            {
                memcpy(&FingerQuality_tmp, pFingerQuality_i, sizeof(FingerQuality));
                memcpy(pFingerQuality_i, pFingerQuality_j, sizeof(FingerQuality));
                memcpy(pFingerQuality_j, &FingerQuality_tmp, sizeof(FingerQuality));
            }
        }
    }

    for (i = 0; i < (int)m_pFingerQuality.size(); i++)
    {
        pFingerQuality = m_pFingerQuality.at(i);

		j = pFingerQuality->x * 4 / cis_img_w;
        pFingerQuality->pos_in_area = j;
    }

    for (i = 0; i < (int)m_pFingerQuality.size(); i++)
    {
        pFingerQuality_i = m_pFingerQuality.at(i);
        for (j = i + 1; j < (int)m_pFingerQuality.size(); j++)
        {
            pFingerQuality_j = m_pFingerQuality.at(j);

            if (pFingerQuality_i->pos_in_area == pFingerQuality_j->pos_in_area)
            {
                pFingerQuality_j->pos_in_area++;
            }
        }
    }

	// 가장 오른쪽에 있는 index가 3을 넘으면 전체적으로 왼쪽으로 이동
	pFingerQuality_i = m_pFingerQuality.at(m_pFingerQuality.size()-1);
	if(pFingerQuality_i->pos_in_area > 3)
	{
		int move_amount = 3-pFingerQuality_i->pos_in_area;
		for (i = 0; i < (int)m_pFingerQuality.size(); i++)
		{
			pFingerQuality_i = m_pFingerQuality.at(i);
			pFingerQuality_i->pos_in_area = pFingerQuality_i->pos_in_area + move_amount;
			if(pFingerQuality_i->pos_in_area < 0)
				pFingerQuality_i->pos_in_area = 0;
		}
	}

    for (i = 0; i < 4; i++)
    {
        finger_count = 0;
        for (j = 0; j < (int)m_pFingerQuality.size(); j++)
        {
            pFingerQuality = m_pFingerQuality.at(j);
            if (pFingerQuality->pos_in_area == i)
            {
                finger_count++;
                break;
            }
        }

        if (finger_count > 0)
        {
            if (pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
            {
                // Judge the quality of rolling finger
                if (m_rollingStatus >= 2)
                {
                    // Quality good
                    pClbkProperty->qualityArray[i] = ENUM_IBSU_QUALITY_GOOD;
                }
				else if( *bIsGoodImage )
                {
                    // Quality fair
                    pClbkProperty->qualityArray[i] = ENUM_IBSU_QUALITY_FAIR;
                }
                else
                {
                    // Quality poor
                    pClbkProperty->qualityArray[i] = ENUM_IBSU_QUALITY_POOR;
                }
            }
            else
            {
                int AreaTres = pPropertyInfo->nSINGLE_FLAT_AREA_TRES * m_cImgAnalysis.finger_count * 3 / 2;
                // Judge the quality of flat finger
				if( *bIsGoodImage )
                {
                    // Quality good
                    pClbkProperty->qualityArray[i] = ENUM_IBSU_QUALITY_GOOD;
                }
                else if ((pFingerQuality->mean >= (__CAPTURE_MIN_BRIGHT__ - MIN_RANGE) &&
                          pFingerQuality->mean <= (__CAPTURE_MAX_BRIGHT__ + MAX_RANGE)) &&
                         (AreaTres < m_cImgAnalysis.foreground_count)
                        )
                {
                    // Quality fair
                    pClbkProperty->qualityArray[i] = ENUM_IBSU_QUALITY_FAIR;
                }
                else
                {
                    // Quality poor
                    pClbkProperty->qualityArray[i] = ENUM_IBSU_QUALITY_POOR;
                }
            }
        }
		else
		{
			pClbkProperty->qualityArray[i] = ENUM_IBSU_FINGER_NOT_PRESENT;
		}
    }
#if 0
	////Modify Sean to check finger in invalid area
	if( (dispInvalidArea == TRUE) ||  /*(pClbkProperty->imageInfo.IsFinal == TRUE)*/
		((m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK) &&
		 (m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0) &&
		 (pPropertyInfo->ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER)&&
		 (pClbkProperty->imageInfo.IsFinal == TRUE))
		)
#else
	if( (dispInvalidArea == TRUE) ||  (pClbkProperty->imageInfo.IsFinal == TRUE && m_pPropertyInfo->bWarningInvalidArea == TRUE) )
#endif
	{
		nInvalidAreaDetected = 0;
		int left, right, top, /*bottom, */finger_index;
		int factor_w = (int)(24 * cis_img_w / img_w);		//(int)(24 * CIS_IMG_W / IMG_W);
		int factor_h = (int)(24 * cis_img_h / img_h);		//(int)(24 * CIS_IMG_H / IMG_H);
		

		// Invalid area sensitivity for Kojak  by Wade Nov 21 2016
		if((m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK))
		{
			factor_w = factor_w * 2;								
			factor_h = (int)(factor_h * 2.5);
		}
		else if(m_pUsbDevInfo->devType == DEVICE_TYPE_COLUMBO)
		{
			factor_w = factor_w * 2;
			factor_h = (int)(factor_h * 2.5);
		}
		
		for(i=0; i<256; i++)
		{
			if(LabelLeft[i] != cis_img_w)
			{
				left	= (LabelLeft[i]-enlargesize_zoom_w) * cis_img_w / zoom_w;
				right	= (LabelRight[i]-enlargesize_zoom_w) * cis_img_w / zoom_w;
				//bottom	= (LabelTop[i]-enlargesize_zoom_h) * cis_img_h / zoom_h;
				top		= (LabelBottom[i]-enlargesize_zoom_h) * cis_img_h / zoom_h;

				finger_index = (right+left)/2;
				finger_index /= cis_img_w/4;

				if( left < factor_w || right >= cis_img_w - factor_w || top >= cis_img_h - factor_h)
				{
					*bIsGoodImage = FALSE;
					m_cImgAnalysis.frame_count = 0;

					if(left < factor_w)
					{
						pClbkProperty->qualityArray[finger_index] = ENUM_IBSU_QUALITY_INVALID_AREA_LEFT;
						if(pFrameImgAnalysis->is_final == TRUE)
						{
							nInvalidAreaDetected |= IBSU_WRN_QUALITY_INVALID_AREA_HORIZONTALLY;
						}
					}

					if(right >= cis_img_w - factor_w)
					{
						pClbkProperty->qualityArray[finger_index] = ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT;
						if(pFrameImgAnalysis->is_final == TRUE)
						{
							nInvalidAreaDetected |= IBSU_WRN_QUALITY_INVALID_AREA_HORIZONTALLY;
						}
					}
					if(top >= cis_img_h - factor_h)
					{
						pClbkProperty->qualityArray[finger_index] = ENUM_IBSU_QUALITY_INVALID_AREA_TOP;
						if(pFrameImgAnalysis->is_final == TRUE)
						{
							nInvalidAreaDetected |= IBSU_WRN_QUALITY_INVALID_AREA_VERTICALLY;
						}
					}

				}
			}
		}

		for (j = 0; j < (int)m_pFingerQuality.size(); j++)
		{
			pFingerQuality = m_pFingerQuality.at(j);
//			TRACE("pFingerQuality [%d] : %d\n", j, pFingerQuality->pos_in_area);
			
			if (pFingerQuality->y < (cis_img_h /invalid_rate))
			{
				pClbkProperty->qualityArray[pFingerQuality->pos_in_area] = ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM;
				
				if(pFrameImgAnalysis->is_final == TRUE)
				{
					*bIsGoodImage = FALSE;
					nInvalidAreaDetected |= IBSU_WRN_QUALITY_INVALID_AREA_VERTICALLY;
				}
			}
		}
	}
}

void CIBAlgorithm::_Algo_Sharpening(const BYTE *Input, const int imgWidth, const int imgHeight, BYTE *Output)
{
    int i, j, val;

    memcpy(Output, Input, imgWidth * imgHeight);

     //for (i = m_cImgAnalysis.TOP; i < m_cImgAnalysis.BOTTOM; i++)
      //{
        //for (j = m_cImgAnalysis.LEFT; j < m_cImgAnalysis.RIGHT; j++)
	//2015-05-14 modified SangGonLee
     for (i = 1; i < imgHeight-1; i++)
     {	
        for (j = 1; j < imgWidth-1; j++)
        {
            if (Input[i * imgWidth + j] == 0)
            {
                continue;
            }

            val = (int)((Input[i * imgWidth + j] << 1) - ((Input[(i - 1) * imgWidth + j] + Input[i * imgWidth + j - 1] + Input[i * imgWidth + j + 1] + Input[(i + 1) * imgWidth + j]) >> 2));

            if (val > 255)
            {
                val = 255;
            }
            else if (val < 0)
            {
                val = 0;
            }

            Output[i * imgWidth + j] = (unsigned char)val;
        }
    }
}

void CIBAlgorithm::_Algo_Smoothing(const BYTE *Input, const int imgWidth, const int imgHeight, BYTE *Output)
{
    int i, j, val;

    memcpy(Output, Input, imgWidth * imgHeight);

	for (i = m_cImgAnalysis.TOP; i < m_cImgAnalysis.BOTTOM; i++)
    {
        for (j = m_cImgAnalysis.LEFT; j < m_cImgAnalysis.RIGHT; j++)
        {
            if (Input[i * imgWidth + j] == 0)
            {
                continue;
            }

            val = ( Input[(i-1) * imgWidth + (j-1)] +
				    Input[(i-1) * imgWidth + j]*2 +
				    Input[(i-1) * imgWidth + (j+1)] +
				    Input[i * imgWidth + (j-1)]*2 +
				    Input[i * imgWidth + j]*16 +
				    Input[i * imgWidth + (j+1)]*2 +
				    Input[(i+1) * imgWidth + (j-1)] +
				    Input[(i+1) * imgWidth + j]*2 +
				    Input[(i+1) * imgWidth + (j+1)] ) / 28;

            Output[i * imgWidth + j] = (unsigned char)val;
        }
    }
}

void CIBAlgorithm::_Algo_StackRecursiveFilling(BYTE *backImage, const int imgWidth, const int imgHeight,
        int x, int y, BYTE TargetColor, BYTE DestColor)
{
    int dx, dy;
    int top;

    top = -1;				// init_stack
    stack[++top] = y;		// push
    stack[++top] = x;		// push

    while (top >= 0)
    {
        dx = stack[top--];		// pop
        dy = stack[top--];		// pop
        if (dx < 0 || dy < 0 || dx >= imgWidth || dy >= imgHeight)
        {
            continue;
        }

        if (backImage[dy * imgWidth + dx] == TargetColor)
        {
            backImage[dy * imgWidth + dx] = DestColor;
            stack[++top] = dy;		// push
            stack[++top] = dx - 1;	// push

            stack[++top] = dy;		// push
            stack[++top] = dx + 1;	// push

            stack[++top] = dy - 1;	// push
            stack[++top] = dx;		// push

            stack[++top] = dy + 1;	// push
            stack[++top] = dx;		// push
        }
    }
}

void CIBAlgorithm::_Algo_ConvertBkColorBlacktoWhite(BYTE *InImg, BYTE *OutImg, int imgSize)
{
    for (int i = 0; i < imgSize; i++)
    {
        OutImg[i] = 255 - InImg[i];
    }
}

void CIBAlgorithm::_Algo_Process_AnalysisImage(unsigned char *InImg)
{
    int x, y;
    int value, y_index;
    int noise_histo[4], noise_count[4];

    memset(noise_histo, 0, sizeof(noise_histo));

    //Initialization
    m_cImgAnalysis.noise_histo = 0;

    //Image Information Analysis
    // Noise Histo
    memset(noise_histo, 0, sizeof(int) * 4);
    memset(noise_count, 0, sizeof(int) * 4);

    for (y = 0; y < 73; y++)
    {
        y_index = y * CIS_IMG_W;
        for (x = 0; x < 64; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[0] < value && value < 60)
            {
                noise_histo[0] = value;
            }
        }
        for (x = CIS_IMG_W - 64; x < CIS_IMG_W; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[1] < value && value < 60)
            {
                noise_histo[1] = value;
            }
        }
    }
    for (y = CIS_IMG_H - 73; y < CIS_IMG_H; y++)
    {
        y_index = y * CIS_IMG_W;
        for (x = 0; x < 64; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[2] < value && value < 60)
            {
                noise_histo[2] = value;
            }
        }
        for (x = CIS_IMG_W - 64; x < CIS_IMG_W; x++)
        {
            value = InImg[y_index + x];
            if (noise_histo[3] < value && value < 60)
            {
                noise_histo[3] = value;
            }
        }
    }

    int min_noise_histo = noise_histo[0], min_noise_idx = 0;
    for (y = 1; y < 4; y++)
    {
        if (min_noise_histo > noise_histo[y])
        {
            min_noise_histo = noise_histo[y];
            min_noise_idx = y;
        }
    }

    m_cImgAnalysis.noise_histo = noise_histo[min_noise_idx];			// mean of noise image
}

int CIBAlgorithm::NewRoll_Brightness_Control(unsigned char *InputImg, unsigned char *OutImg, unsigned char target_brightness, int MergePosX)
{
    int i, j, val, val_sum, val_cnt, loop_count, cur_bright, diff_bright;
    int LEFT, RIGHT, TOP, BOTTOM;
	int Minus_Val = 0;

	int tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//	int tmpCIS_IMG_H = CIS_IMG_H_ROLL;
	int tmpCIS_IMG_SIZE = CIS_IMG_SIZE_ROLL;
	
	LEFT = m_cImgAnalysis.LEFT;
	RIGHT = m_cImgAnalysis.RIGHT;
	TOP = m_cImgAnalysis.TOP;
	BOTTOM = m_cImgAnalysis.BOTTOM;

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
	{
		Minus_Val =_NO_BIT_CMOS_NOISE_FOR_FULLFRAME_ROLL;// m_SBDAlg->m_Roll_Minus_Val;
	}
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
	{
		Minus_Val = _TFT_NOISE_FOR_FIVE0_;
	}

    cur_bright = NewRoll_GetBright_Simple(InputImg, MergePosX);
    diff_bright = target_brightness - cur_bright;

    if (cur_bright < 10)	// 이미지가 너무 어두운 경우 (지문이 없는 걸로...)
    {
		memcpy(OutImg, InputImg, tmpCIS_IMG_SIZE);
        //TRACE("\n----> too dark (skipped)\n");
        return NO_FINGER;
    }

    if (diff_bright <= -5) // 이미지가 너무 진하면 약간 흐리게...
    {
        //TRACE("\n----> make lighter : %d\n", diff_bright);
		for (loop_count = 63; loop_count >= 48; loop_count -= 2)
		{
			val_sum = 0;
			val_cnt = 0;
			cur_bright = 0;
			for (i = TOP; i <= BOTTOM; i++)
			{
				for (j = MergePosX - 10; j <= MergePosX + 10; j++)
				{
					val = InputImg[i * tmpCIS_IMG_W + j] * loop_count / 64;
					if (val > Minus_Val)
					{
						val_sum += val;
						val_cnt++;
					}
				}
			}
			if (val_cnt > 0)
			{
				cur_bright = val_sum / val_cnt;
			}

			if (target_brightness > cur_bright - 5)
			{
				break;
			}
		}

		memset(OutImg, 0, tmpCIS_IMG_SIZE);
		for (i = TOP; i <= BOTTOM; i++)
		{
			for (j = LEFT; j <= RIGHT; j++)
			{
				val = InputImg[i * tmpCIS_IMG_W + j] * loop_count / 64;
				OutImg[i * tmpCIS_IMG_W + j] = val;
			}
		}

        //TRACE("==> target_brightness : %d, cur_bright : %d, loop_count : %d\n", target_brightness, cur_bright, loop_count);

        return SUCCESS;
    }	// 적당한 이미지는 그냥 복사
    else if (diff_bright < 5)
    {
		//TRACE("==> Good Image : %d\n", diff_bright);
        memcpy(OutImg, InputImg, tmpCIS_IMG_SIZE);
        return SUCCESS;
    }

    // 이미지가 흐린 경우는 진하게...
    for (loop_count = 1; loop_count < 48; loop_count++)
    {
        _Algo_Image_Gamma_Simple(InputImg, OutImg, loop_count, MergePosX);
        cur_bright = NewRoll_GetBright_Simple(OutImg, MergePosX);

        diff_bright = cur_bright - target_brightness;

        if (target_brightness < cur_bright)
        {
            _Algo_Image_Gamma(InputImg, OutImg, loop_count);
            break;
        }
    }

	//TRACE("target_brightness : %d, cur_bright : %d, diff_bright : %d\n", target_brightness, cur_bright, diff_bright);

    if (loop_count >= 48)
    {
        if (diff_bright < -5)
        {
//			TRACE("\n----> too dark (skipped)\n");
            return NO_FINGER;
        }
        else
        {
            _Algo_Image_Gamma(InputImg, OutImg, loop_count - 1);
        }
    }

    //	TRACE("==> target_brightness : %d, cur_bright : %d, loop_count : %d\n", target_brightness, cur_bright, loop_count);

    return SUCCESS;
}

int CIBAlgorithm::NewRoll_GetBright_Simple(const BYTE *InputImage, int MergePosX)
{
    int i, j;
    int bright = 0, count = 0;
	int cut_bright = 5;
	int TOP, BOTTOM;

	TOP = m_cImgAnalysis.TOP;
	BOTTOM = m_cImgAnalysis.BOTTOM;

	int tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//	int tmpCIS_IMG_H = CIS_IMG_H_ROLL;

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
	{
		cut_bright = _NO_BIT_CMOS_NOISE_FOR_FULLFRAME_ROLL;//m_SBDAlg->m_Roll_Minus_Val;
	}
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
	{
		cut_bright = _TFT_NOISE_FOR_FIVE0_;
	}
		
	for (i = TOP; i < BOTTOM; i+=2)
	{
		for (j = MergePosX - 10; j <= MergePosX + 10; j+=2)
		{
			if (InputImage[i * tmpCIS_IMG_W + j] > cut_bright)
			{
				bright += InputImage[i * tmpCIS_IMG_W + j];
				count++;
			}
		}
	}

    if (count > 0)
    {
        bright = bright / count;
    }

    return bright;
}

void CIBAlgorithm::_Algo_Encrypt_Simple(unsigned char *InBuf, unsigned char *OutBuf, size_t buf_size)
{
    size_t i;
    for (i = 0; i < buf_size; i++)
    {
        OutBuf[i] = InBuf[i] ^ (i & 0xFF);
    }
}

void CIBAlgorithm::_Algo_Decrypt_Simple(unsigned char *InBuf, unsigned char *OutBuf, size_t buf_size)
{
    size_t i;
    for (i = 0; i < buf_size; i++)
    {
        OutBuf[i] = InBuf[i] ^ (i & 0xFF);
    }
}

void CIBAlgorithm::_Algo_MakeChecksum(DWORD *InBuf, size_t buf_size)
{
    DWORD	checksum = 0;
    size_t	i;

    for (i = 0; i < buf_size - 1; i++)
    {
        checksum += (InBuf[i] >> 24) & 0xFF;
        checksum += (InBuf[i] >> 16) & 0xFF;
        checksum += (InBuf[i] >> 8) & 0xFF;
        checksum += InBuf[i] & 0xFF;
    }

    InBuf[buf_size - 1] = checksum;
}

BOOL CIBAlgorithm::_Algo_ConfirmChecksum(DWORD *InBuf, size_t buf_size)
{
    DWORD	checksum = 0;
    size_t	i;

    for (i = 0; i < buf_size - 1; i++)
    {
        checksum += (InBuf[i] >> 24) & 0xFF;
        checksum += (InBuf[i] >> 16) & 0xFF;
        checksum += (InBuf[i] >> 8) & 0xFF;
        checksum += InBuf[i] & 0xFF;
    }

    if (checksum == InBuf[buf_size - 1])
    {
        return TRUE;
    }

    return FALSE;
}

int CIBAlgorithm::_Algo_GetNFIQScore(const unsigned char *idata, const int imgWidth, const int imgHeight, const int imgPixelDepth, int *onfiq)
{
#ifdef __IBSCAN_ULTIMATE_SDK__
    int optflag = 0;
    float oconf;
    int imageScanDensity = -1; // Unknown
    int nfiq = 0;

    int rc = comp_nfiq(&nfiq, &oconf, (unsigned char *)idata, imgWidth, imgHeight, imgPixelDepth, imageScanDensity, &optflag);

    *onfiq = nfiq;

    return rc;
#else
    return 0;
#endif
	}

void CIBAlgorithm::_Algo_RemoveTFTDefect(unsigned char *InImg, unsigned char *OutImage, unsigned char *Defect_MaskImg)
{
	if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0 && m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		if (m_TFT_DefectMaskCount == 0)
		{
			memcpy(OutImage, InImg, CIS_IMG_SIZE_ROLL);
			return;
		}

		int i, j, val, cnt, x, y, xx, yy;
		int offset_x = (CIS_IMG_W - CIS_IMG_W_ROLL)/2;
		int offset_y = m_pPropertyInfo->nStartingPositionOfRollArea * 27;
		int left, right, top, bottom;

		left = (CIS_IMG_W - CIS_IMG_W_ROLL)/2;
		right = (CIS_IMG_W - CIS_IMG_W_ROLL)/2 + CIS_IMG_W_ROLL;
		top = 0;
		bottom = CIS_IMG_H_ROLL+offset_y;

		USHORT(*pTFT_DefectMaskList)[19] = (USHORT( *)[19])m_TFT_DefectMaskList;

		memcpy(OutImage, InImg, CIS_IMG_SIZE_ROLL);
		for (i = 0; i < m_TFT_DefectMaskCount; i++)
		{
			val = 0;
			cnt = pTFT_DefectMaskList[i][2];
			if (cnt == 0)
			{
				continue;
			}

			x = pTFT_DefectMaskList[i][0];
			y = pTFT_DefectMaskList[i][1];
			
			if(x < left || x >= right || y < top || y >= bottom)
				continue;

			x = x - offset_x;
			y = y - offset_y;

			for (j = 3; j < cnt * 2 + 3; j += 2)
			{
				xx = pTFT_DefectMaskList[i][j] - offset_x;
				yy = pTFT_DefectMaskList[i][j + 1] - offset_y;
				val += InImg[yy * CIS_IMG_W_ROLL + xx];
			}

			val = val / cnt;
			OutImage[y * CIS_IMG_W_ROLL + x] = val;
		}
	}
	else
	{
		if (m_TFT_DefectMaskCount == 0)
		{
			memcpy(OutImage, InImg, CIS_IMG_SIZE);
			return;
		}

		int i, j, val, cnt, x, y, xx, yy;
		USHORT(*pTFT_DefectMaskList)[19] = (USHORT( *)[19])m_TFT_DefectMaskList;

		memcpy(OutImage, InImg, CIS_IMG_SIZE);
		for (i = 0; i < m_TFT_DefectMaskCount; i++)
		{
			val = 0;
			cnt = pTFT_DefectMaskList[i][2];
			if (cnt == 0)
			{
				continue;
			}

			x = pTFT_DefectMaskList[i][0];
			y = pTFT_DefectMaskList[i][1];
			for (j = 3; j < cnt * 2 + 3; j += 2)
			{
				xx = pTFT_DefectMaskList[i][j];
				yy = pTFT_DefectMaskList[i][j + 1];
				val += InImg[yy * CIS_IMG_W + xx];
			}

			val = val / cnt;
			OutImage[y * CIS_IMG_W + x] = val;
		}
	}
}

BOOL CIBAlgorithm::_Algo_ManualCapture(int maxBright, int minBright, PropertyInfo *propertyInfo, int frameTime)
{
    int			AreaTres;
    const int	MAX_BRIGHT = maxBright, MIN_BRIGHT = minBright;
    const int	MIN_CAPTURE_TIME = 300;

    if (m_cImgAnalysis.foreground_count < 100)
    {
        m_cImgAnalysis.good_frame_count = 0;
        m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
        m_cImgAnalysis.frame_count = 0;
        return FALSE;
    }

    if (m_cImgAnalysis.finger_count != m_cImgAnalysis.saved_finger_count)
    {
        m_cImgAnalysis.good_frame_count = 0;
        m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
        m_cImgAnalysis.frame_count = 0;
    }

    m_cImgAnalysis.frame_count++;
    if (m_cImgAnalysis.finger_count == 0)
    {
        AreaTres = propertyInfo->nSINGLE_FLAT_AREA_TRES;
    }
    else
    {
        AreaTres = propertyInfo->nSINGLE_FLAT_AREA_TRES * m_cImgAnalysis.finger_count;
    }

    if (m_cImgAnalysis.mean >= MIN_BRIGHT && m_cImgAnalysis.mean <= MAX_BRIGHT &&
            m_cImgAnalysis.foreground_count >= AreaTres &&
            abs(m_cImgAnalysis.foreground_count - m_cImgAnalysis.pre_foreground_count) <= SINGLE_FLAT_DIFF_TRES &&
            abs(m_cImgAnalysis.center_x - m_cImgAnalysis.pre_center_x) <= 2 &&
            abs(m_cImgAnalysis.center_y - m_cImgAnalysis.pre_center_y) <= 2
       )
    {
        if ((m_cImgAnalysis.frame_count * frameTime) > MIN_CAPTURE_TIME)
        {
            m_cImgAnalysis.saved_finger_count = m_cImgAnalysis.finger_count;
            return TRUE;
        }
    }

    m_cImgAnalysis.pre_foreground_count = m_cImgAnalysis.foreground_count;
    m_cImgAnalysis.pre_center_x = m_cImgAnalysis.center_x;
    m_cImgAnalysis.pre_center_y = m_cImgAnalysis.center_y;

    return FALSE;
}

int CIBAlgorithm::_Algo_SegmentFinger(unsigned char *ImgData)
{
//	int OldSegmentCnt;
	memset(&m_segment_arr, 0, sizeof(NEW_SEGMENT_ARRAY));

	int ImageBright = _Algo_SegmentationImage(ImgData);

//	OldSegmentCnt = _Algo_GetSegmentInfo(&m_segment_arr, ImageBright);
	_Algo_GetSegmentInfo(&m_segment_arr, ImageBright);

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		for(int i=0; i<m_segment_arr.SegmentCnt; i++)
		{
			m_segment_arr.Segment[i].Ang = 90;
		}
		m_segment_arr.VCenterSegment.Ang = 90;
	}
	
//	TRACE("\n1. OldSegmentCnt : %d\n", m_segment_arr.SegmentCnt);
	
	// 1. not need to segment fingers
/*	if(OldSegmentCnt <= 1)
	{
		return OldSegmentCnt;
	}
*/
	// 2. calc main direction of fingers
//	int old_main_dir=0;

	_Algo_GetMainDirectionOfFinger();
//	old_main_dir = m_segment_arr.VCenterSegment.Ang;

/*	// 3. remove false segment (able to be skipped)
	do
	{
		_Algo_RemoveFalseSegment();

		_Algo_GetMainDirectionOfFinger();
		new_main_dir = m_segment_arr.VCenterSegment.Ang;

		diff_main_dir = abs(new_main_dir - old_main_dir);
		if(diff_main_dir >= 180)
			diff_main_dir -= 180;

		old_main_dir = new_main_dir;

	} while(diff_main_dir > 10);
*/

	// 4. sort segments descending order from top to bottom
	_Algo_SortSegmentByMainDir();

	// 5. merge segment if same finger
	_Algo_MergeSegmentIfSameFinger();
//	TRACE("2. NewSegmentCnt : %d\n", m_segment_arr.SegmentCnt);
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		NEW_SEGMENT new_seg;
		for(int gg=0; gg<m_segment_arr.SegmentCnt-1; gg++)
		{
			for(int tt=gg+1; tt<m_segment_arr.SegmentCnt; tt++)
			{
				if(m_segment_arr.Segment[gg].CenterY < m_segment_arr.Segment[tt].CenterY)
				{
					new_seg = m_segment_arr.Segment[gg];
					m_segment_arr.Segment[gg] = m_segment_arr.Segment[tt];
					m_segment_arr.Segment[tt] = new_seg;
				}
			}
		}
		if(m_segment_arr.SegmentCnt > 1)
			m_segment_arr.SegmentCnt = 1;

		for(int i=0; i<ZOOM_ENLAGE_H_ROLL; i++)
		{
			for(int j=0; j<ZOOM_ENLAGE_W_ROLL; j++)
			{
				if(m_segment_enlarge_buffer_for_fingercnt[i*ZOOM_ENLAGE_W_ROLL+j] > 0)
				{
					if(m_segment_arr.Segment[0].P1_X < j)
						m_segment_arr.Segment[0].P1_X = j;
					if(m_segment_arr.Segment[0].P4_X < j)
						m_segment_arr.Segment[0].P4_X = j;

					if(m_segment_arr.Segment[0].P2_X > j)
						m_segment_arr.Segment[0].P2_X = j;
					if(m_segment_arr.Segment[0].P3_X > j)
						m_segment_arr.Segment[0].P3_X = j;

					if(m_segment_arr.Segment[0].P1_Y < i)
						m_segment_arr.Segment[0].P1_Y = i;
					if(m_segment_arr.Segment[0].P2_Y < i)
						m_segment_arr.Segment[0].P2_Y = i;

					if(m_segment_arr.Segment[0].P3_Y > i)
						m_segment_arr.Segment[0].P3_Y = i;
					if(m_segment_arr.Segment[0].P4_Y > i)
						m_segment_arr.Segment[0].P4_Y = i;
				}
			}
		}
	}

	// 6. search exact position of segment
	_Algo_SearchFingerTipPosition();

/*	int y, x, yy, xx;
	for(y=0; y<IMG_H; y++)
	{
		yy = y * ZOOM_H / IMG_H + ENLARGESIZE_ZOOM_H;
		for(x=0; x<IMG_W; x++)
		{
			xx = x * ZOOM_W / IMG_W + ENLARGESIZE_ZOOM_W;
			ImgData[y*IMG_W+x]=m_segment_enlarge_buffer_for_fingercnt[yy*ZOOM_ENLAGE_W+xx];
		}
	}
*/

	NEW_SEGMENT_ARRAY tmpArray;
	memcpy(&tmpArray, &m_segment_arr, sizeof(tmpArray));

	double cs_radian = cos((m_segment_arr.VCenterSegment.Ang-90)*3.14159265358979/180.0);
	double sn_radian = sin((m_segment_arr.VCenterSegment.Ang-90)*3.14159265358979/180.0);

	for(int u=0; u<m_segment_arr.SegmentCnt; u++)
	{
		tmpArray.Segment[u].P1_X = (int) ( (double)(m_segment_arr.Segment[u].P1_X-m_segment_arr.VCenterSegment.X)*cs_radian + 
										   (double)(m_segment_arr.Segment[u].P1_Y-m_segment_arr.VCenterSegment.Y)*sn_radian ) + m_segment_arr.VCenterSegment.X;
		tmpArray.Segment[u].P1_Y = (int) ( -(double)(m_segment_arr.Segment[u].P1_X-m_segment_arr.VCenterSegment.X)*sn_radian + 
										   (double)(m_segment_arr.Segment[u].P1_Y-m_segment_arr.VCenterSegment.Y)*cs_radian ) + m_segment_arr.VCenterSegment.Y;

		tmpArray.Segment[u].P2_X = (int) ( (double)(m_segment_arr.Segment[u].P2_X-m_segment_arr.VCenterSegment.X)*cs_radian + 
										   (double)(m_segment_arr.Segment[u].P2_Y-m_segment_arr.VCenterSegment.Y)*sn_radian ) + m_segment_arr.VCenterSegment.X;
		tmpArray.Segment[u].P2_Y = (int) ( -(double)(m_segment_arr.Segment[u].P2_X-m_segment_arr.VCenterSegment.X)*sn_radian + 
										   (double)(m_segment_arr.Segment[u].P2_Y-m_segment_arr.VCenterSegment.Y)*cs_radian ) + m_segment_arr.VCenterSegment.Y;

		tmpArray.Segment[u].P3_X = (int) ( (double)(m_segment_arr.Segment[u].P3_X-m_segment_arr.VCenterSegment.X)*cs_radian + 
										   (double)(m_segment_arr.Segment[u].P3_Y-m_segment_arr.VCenterSegment.Y)*sn_radian ) + m_segment_arr.VCenterSegment.X;
		tmpArray.Segment[u].P3_Y = (int) ( -(double)(m_segment_arr.Segment[u].P3_X-m_segment_arr.VCenterSegment.X)*sn_radian + 
										   (double)(m_segment_arr.Segment[u].P3_Y-m_segment_arr.VCenterSegment.Y)*cs_radian ) + m_segment_arr.VCenterSegment.Y;

		tmpArray.Segment[u].P4_X = (int) ( (double)(m_segment_arr.Segment[u].P4_X-m_segment_arr.VCenterSegment.X)*cs_radian + 
										   (double)(m_segment_arr.Segment[u].P4_Y-m_segment_arr.VCenterSegment.Y)*sn_radian ) + m_segment_arr.VCenterSegment.X;
		tmpArray.Segment[u].P4_Y = (int) ( -(double)(m_segment_arr.Segment[u].P4_X-m_segment_arr.VCenterSegment.X)*sn_radian + 
										   (double)(m_segment_arr.Segment[u].P4_Y-m_segment_arr.VCenterSegment.Y)*cs_radian ) + m_segment_arr.VCenterSegment.Y;

		tmpArray.Segment[u].CenterX = (tmpArray.Segment[u].P1_X + tmpArray.Segment[u].P2_X + 
										tmpArray.Segment[u].P3_X + tmpArray.Segment[u].P4_X)/4;
		tmpArray.Segment[u].CenterY = (tmpArray.Segment[u].P1_Y + tmpArray.Segment[u].P2_Y + 
										tmpArray.Segment[u].P3_Y + tmpArray.Segment[u].P4_Y)/4;
	}

	NEW_SEGMENT new_seg;
	for(int gg=0; gg<tmpArray.SegmentCnt-1; gg++)
	{
		for(int tt=gg+1; tt<tmpArray.SegmentCnt; tt++)
		{
			if(tmpArray.Segment[gg].CenterX > tmpArray.Segment[tt].CenterX)
			{
				new_seg = tmpArray.Segment[gg];
				tmpArray.Segment[gg] = tmpArray.Segment[tt];
				tmpArray.Segment[tt] = new_seg;

				new_seg = m_segment_arr.Segment[gg];
				m_segment_arr.Segment[gg] = m_segment_arr.Segment[tt];
				m_segment_arr.Segment[tt] = new_seg;
			}
		}
	}

	if(m_segment_arr.SegmentCnt > IBSU_MAX_SEGMENT_COUNT)
		m_segment_arr.SegmentCnt = IBSU_MAX_SEGMENT_COUNT;

	int i, s, t;
	int startx, starty;
	int tmpIMG_W = IMG_W;
	int tmpIMG_H = IMG_H;
	int tmpIMG_SIZE = IMG_SIZE;
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		tmpIMG_W = IMG_W_ROLL;
		tmpIMG_H = IMG_H_ROLL;
		tmpIMG_SIZE = IMG_SIZE_ROLL;
	}

	for(i=0; i<m_segment_arr.SegmentCnt; i++)
	{
		_Algo_ClipFinger(ImgData, &m_OutSplitResultArrayEx[i*tmpIMG_SIZE], &m_OutSplitResultArrayExWidth[i], &m_OutSplitResultArrayExHeight[i], &m_OutSplitResultArrayExSize[i], 
						m_labeled_segment_enlarge_buffer, &m_segment_arr.Segment[i]);

		startx = (tmpIMG_W-m_OutSplitResultArrayExWidth[i])/2;
		starty = (tmpIMG_H-m_OutSplitResultArrayExHeight[i])/2;

		for(s=0; s<m_OutSplitResultArrayExHeight[i]; s++)
		{
			for(t=0; t<m_OutSplitResultArrayExWidth[i]; t++)
			{
				m_OutSplitResultArray[i*tmpIMG_SIZE + (s+starty)*tmpIMG_W+(startx+t)] = m_OutSplitResultArrayEx[i*tmpIMG_SIZE + s*m_OutSplitResultArrayExWidth[i]+t];
			}
		}
	}

	return m_segment_arr.SegmentCnt;
}

int CIBAlgorithm::_Algo_SegmentFinger_Matcher(unsigned char *ImgData)
{
//	int OldSegmentCnt;
	memset(&m_segment_arr_matcher, 0, sizeof(NEW_SEGMENT_ARRAY));

	int ImageBright = _Algo_SegmentationImage(ImgData);

//	OldSegmentCnt = _Algo_GetSegmentInfo(&m_segment_arr_matcher, ImageBright);
	_Algo_GetSegmentInfo(&m_segment_arr_matcher, ImageBright);

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		for(int i=0; i<m_segment_arr_matcher.SegmentCnt; i++)
		{
			m_segment_arr_matcher.Segment[i].Ang = 90;
		}
		m_segment_arr_matcher.VCenterSegment.Ang = 90;
	}
	
//	TRACE("\n1. OldSegmentCnt : %d\n", m_segment_arr_matcher.SegmentCnt);
	
	// 1. not need to segment fingers
/*	if(OldSegmentCnt <= 1)
	{
		return OldSegmentCnt;
	}
*/
	// 2. calc main direction of fingers
//	int old_main_dir=0;

	_Algo_GetMainDirectionOfFinger_Matcher();
//	old_main_dir = m_segment_arr_matcher.VCenterSegment.Ang;

/*	// 3. remove false segment (able to be skipped)
	do
	{
		_Algo_RemoveFalseSegment();

		_Algo_GetMainDirectionOfFinger();
		new_main_dir = m_segment_arr_matcher.VCenterSegment.Ang;

		diff_main_dir = abs(new_main_dir - old_main_dir);
		if(diff_main_dir >= 180)
			diff_main_dir -= 180;

		old_main_dir = new_main_dir;

	} while(diff_main_dir > 10);
*/

	// 4. sort segments descending order from top to bottom
	_Algo_SortSegmentByMainDir_Matcher();

	// 5. merge segment if same finger
	_Algo_MergeSegmentIfSameFinger_Matcher();
//	TRACE("2. NewSegmentCnt : %d\n", m_segment_arr_matcher.SegmentCnt);
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		NEW_SEGMENT new_seg;
		for(int gg=0; gg<m_segment_arr_matcher.SegmentCnt-1; gg++)
		{
			for(int tt=gg+1; tt<m_segment_arr_matcher.SegmentCnt; tt++)
			{
				if(m_segment_arr_matcher.Segment[gg].CenterY < m_segment_arr_matcher.Segment[tt].CenterY)
				{
					new_seg = m_segment_arr_matcher.Segment[gg];
					m_segment_arr_matcher.Segment[gg] = m_segment_arr_matcher.Segment[tt];
					m_segment_arr_matcher.Segment[tt] = new_seg;
				}
			}
		}
		if(m_segment_arr_matcher.SegmentCnt > 1)
			m_segment_arr_matcher.SegmentCnt = 1;

		for(int i=0; i<ZOOM_ENLAGE_H_ROLL; i++)
		{
			for(int j=0; j<ZOOM_ENLAGE_W_ROLL; j++)
			{
				if(m_segment_enlarge_buffer_for_fingercnt[i*ZOOM_ENLAGE_W_ROLL+j] > 0)
				{
					if(m_segment_arr_matcher.Segment[0].P1_X < j)
						m_segment_arr_matcher.Segment[0].P1_X = j;
					if(m_segment_arr_matcher.Segment[0].P4_X < j)
						m_segment_arr_matcher.Segment[0].P4_X = j;

					if(m_segment_arr_matcher.Segment[0].P2_X > j)
						m_segment_arr_matcher.Segment[0].P2_X = j;
					if(m_segment_arr_matcher.Segment[0].P3_X > j)
						m_segment_arr_matcher.Segment[0].P3_X = j;

					if(m_segment_arr_matcher.Segment[0].P1_Y < i)
						m_segment_arr_matcher.Segment[0].P1_Y = i;
					if(m_segment_arr_matcher.Segment[0].P2_Y < i)
						m_segment_arr_matcher.Segment[0].P2_Y = i;

					if(m_segment_arr_matcher.Segment[0].P3_Y > i)
						m_segment_arr_matcher.Segment[0].P3_Y = i;
					if(m_segment_arr_matcher.Segment[0].P4_Y > i)
						m_segment_arr_matcher.Segment[0].P4_Y = i;
				}
			}
		}
	}

	// 6. search exact position of segment
	_Algo_SearchFingerTipPosition_Matcher();

/*	int y, x, yy, xx;
	for(y=0; y<IMG_H; y++)
	{
		yy = y * ZOOM_H / IMG_H + ENLARGESIZE_ZOOM_H;
		for(x=0; x<IMG_W; x++)
		{
			xx = x * ZOOM_W / IMG_W + ENLARGESIZE_ZOOM_W;
			ImgData[y*IMG_W+x]=m_segment_enlarge_buffer_for_fingercnt[yy*ZOOM_ENLAGE_W+xx];
		}
	}
*/

	NEW_SEGMENT_ARRAY tmpArray;
	memcpy(&tmpArray, &m_segment_arr_matcher, sizeof(tmpArray));

	double cs_radian = cos((m_segment_arr_matcher.VCenterSegment.Ang-90)*3.14159265358979/180.0);
	double sn_radian = sin((m_segment_arr_matcher.VCenterSegment.Ang-90)*3.14159265358979/180.0);

	for(int u=0; u<m_segment_arr_matcher.SegmentCnt; u++)
	{
		tmpArray.Segment[u].P1_X = (int) ( (double)(m_segment_arr_matcher.Segment[u].P1_X-m_segment_arr_matcher.VCenterSegment.X)*cs_radian + 
										   (double)(m_segment_arr_matcher.Segment[u].P1_Y-m_segment_arr_matcher.VCenterSegment.Y)*sn_radian ) + m_segment_arr_matcher.VCenterSegment.X;
		tmpArray.Segment[u].P1_Y = (int) ( -(double)(m_segment_arr_matcher.Segment[u].P1_X-m_segment_arr_matcher.VCenterSegment.X)*sn_radian + 
										   (double)(m_segment_arr_matcher.Segment[u].P1_Y-m_segment_arr_matcher.VCenterSegment.Y)*cs_radian ) + m_segment_arr_matcher.VCenterSegment.Y;

		tmpArray.Segment[u].P2_X = (int) ( (double)(m_segment_arr_matcher.Segment[u].P2_X-m_segment_arr_matcher.VCenterSegment.X)*cs_radian + 
										   (double)(m_segment_arr_matcher.Segment[u].P2_Y-m_segment_arr_matcher.VCenterSegment.Y)*sn_radian ) + m_segment_arr_matcher.VCenterSegment.X;
		tmpArray.Segment[u].P2_Y = (int) ( -(double)(m_segment_arr_matcher.Segment[u].P2_X-m_segment_arr_matcher.VCenterSegment.X)*sn_radian + 
										   (double)(m_segment_arr_matcher.Segment[u].P2_Y-m_segment_arr_matcher.VCenterSegment.Y)*cs_radian ) + m_segment_arr_matcher.VCenterSegment.Y;

		tmpArray.Segment[u].P3_X = (int) ( (double)(m_segment_arr_matcher.Segment[u].P3_X-m_segment_arr_matcher.VCenterSegment.X)*cs_radian + 
										   (double)(m_segment_arr_matcher.Segment[u].P3_Y-m_segment_arr_matcher.VCenterSegment.Y)*sn_radian ) + m_segment_arr_matcher.VCenterSegment.X;
		tmpArray.Segment[u].P3_Y = (int) ( -(double)(m_segment_arr_matcher.Segment[u].P3_X-m_segment_arr_matcher.VCenterSegment.X)*sn_radian + 
										   (double)(m_segment_arr_matcher.Segment[u].P3_Y-m_segment_arr_matcher.VCenterSegment.Y)*cs_radian ) + m_segment_arr_matcher.VCenterSegment.Y;

		tmpArray.Segment[u].P4_X = (int) ( (double)(m_segment_arr_matcher.Segment[u].P4_X-m_segment_arr_matcher.VCenterSegment.X)*cs_radian + 
										   (double)(m_segment_arr_matcher.Segment[u].P4_Y-m_segment_arr_matcher.VCenterSegment.Y)*sn_radian ) + m_segment_arr_matcher.VCenterSegment.X;
		tmpArray.Segment[u].P4_Y = (int) ( -(double)(m_segment_arr_matcher.Segment[u].P4_X-m_segment_arr_matcher.VCenterSegment.X)*sn_radian + 
										   (double)(m_segment_arr_matcher.Segment[u].P4_Y-m_segment_arr_matcher.VCenterSegment.Y)*cs_radian ) + m_segment_arr_matcher.VCenterSegment.Y;

		tmpArray.Segment[u].CenterX = (tmpArray.Segment[u].P1_X + tmpArray.Segment[u].P2_X + 
										tmpArray.Segment[u].P3_X + tmpArray.Segment[u].P4_X)/4;
		tmpArray.Segment[u].CenterY = (tmpArray.Segment[u].P1_Y + tmpArray.Segment[u].P2_Y + 
										tmpArray.Segment[u].P3_Y + tmpArray.Segment[u].P4_Y)/4;
	}

	NEW_SEGMENT new_seg;
	for(int gg=0; gg<tmpArray.SegmentCnt-1; gg++)
	{
		for(int tt=gg+1; tt<tmpArray.SegmentCnt; tt++)
		{
			if(tmpArray.Segment[gg].CenterX > tmpArray.Segment[tt].CenterX)
			{
				new_seg = tmpArray.Segment[gg];
				tmpArray.Segment[gg] = tmpArray.Segment[tt];
				tmpArray.Segment[tt] = new_seg;

				new_seg = m_segment_arr_matcher.Segment[gg];
				m_segment_arr_matcher.Segment[gg] = m_segment_arr_matcher.Segment[tt];
				m_segment_arr_matcher.Segment[tt] = new_seg;
			}
		}
	}

	if(m_segment_arr_matcher.SegmentCnt > IBSU_MAX_SEGMENT_COUNT)
		m_segment_arr_matcher.SegmentCnt = IBSU_MAX_SEGMENT_COUNT;

	if(m_CalledByValidGeo == TRUE)
		return m_segment_arr_matcher.SegmentCnt;

	int i, s, t;
	int startx, starty;
	int tmpIMG_W = IMG_W;
	int tmpIMG_H = IMG_H;
	int tmpIMG_SIZE = IMG_SIZE;
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		tmpIMG_W = IMG_W_ROLL;
		tmpIMG_H = IMG_H_ROLL;
		tmpIMG_SIZE = IMG_SIZE_ROLL;
	}

	for(i=0; i<m_segment_arr_matcher.SegmentCnt; i++)
	{
		_Algo_ClipFinger(ImgData, &m_OutSplitResultArrayEx[i*tmpIMG_SIZE], &m_OutSplitResultArrayExWidth[i], &m_OutSplitResultArrayExHeight[i], &m_OutSplitResultArrayExSize[i], 
						m_labeled_segment_enlarge_buffer, &m_segment_arr_matcher.Segment[i]);

		startx = (tmpIMG_W-m_OutSplitResultArrayExWidth[i])/2;
		starty = (tmpIMG_H-m_OutSplitResultArrayExHeight[i])/2;

		for(s=0; s<m_OutSplitResultArrayExHeight[i]; s++)
		{
			for(t=0; t<m_OutSplitResultArrayExWidth[i]; t++)
			{
				m_OutSplitResultArray[i*tmpIMG_SIZE + (s+starty)*tmpIMG_W+(startx+t)] = m_OutSplitResultArrayEx[i*tmpIMG_SIZE + s*m_OutSplitResultArrayExWidth[i]+t];
			}
		}
	}

	return m_segment_arr_matcher.SegmentCnt;
}

int CIBAlgorithm::_Algo_SegmentFinger_from_SegmentInfo(unsigned char *pSegImg, unsigned char *pEnlargeImg, unsigned char *ImgData)
{
//	int OldSegmentCnt;
	memset(&m_segment_arr, 0, sizeof(NEW_SEGMENT_ARRAY));

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		memcpy(m_segment_enlarge_buffer_for_fingercnt, pSegImg, ZOOM_ENLAGE_W_ROLL*ZOOM_ENLAGE_H_ROLL);
		memcpy(EnlargeBuf, pEnlargeImg, ZOOM_ENLAGE_W_ROLL*ZOOM_ENLAGE_H_ROLL);
	}
	else
	{
		memcpy(m_segment_enlarge_buffer_for_fingercnt, pSegImg, ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);
		memcpy(EnlargeBuf, pEnlargeImg, ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);
	}

	_Algo_GetSegmentInfo(&m_segment_arr, 0);

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		for(int i=0; i<m_segment_arr.SegmentCnt; i++)
		{
			m_segment_arr.Segment[i].Ang = 90;
		}
		m_segment_arr.VCenterSegment.Ang = 90;
	}

	_Algo_GetMainDirectionOfFinger();

	// 4. sort segments descending order from top to bottom
	_Algo_SortSegmentByMainDir();

	// 5. merge segment if same finger
	_Algo_MergeSegmentIfSameFinger();
	
	// 6. search exact position of segment
	_Algo_SearchFingerTipPosition();

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		NEW_SEGMENT new_seg;
		for(int gg=0; gg<m_segment_arr.SegmentCnt-1; gg++)
		{
			for(int tt=gg+1; tt<m_segment_arr.SegmentCnt; tt++)
			{
				if(m_segment_arr.Segment[gg].CenterY < m_segment_arr.Segment[tt].CenterY)
				{
					new_seg = m_segment_arr.Segment[gg];
					m_segment_arr.Segment[gg] = m_segment_arr.Segment[tt];
					m_segment_arr.Segment[tt] = new_seg;
				}
			}
		}
		if(m_segment_arr.SegmentCnt > 1)
			m_segment_arr.SegmentCnt = 1;
	}

	NEW_SEGMENT_ARRAY tmpArray;
	memcpy(&tmpArray, &m_segment_arr, sizeof(tmpArray));

	double cs_radian = cos((m_segment_arr.VCenterSegment.Ang-90)*3.14159265358979/180.0);
	double sn_radian = sin((m_segment_arr.VCenterSegment.Ang-90)*3.14159265358979/180.0);

	for(int u=0; u<m_segment_arr.SegmentCnt; u++)
	{
		tmpArray.Segment[u].P1_X = (int) ( (double)(m_segment_arr.Segment[u].P1_X-m_segment_arr.VCenterSegment.X)*cs_radian + 
										   (double)(m_segment_arr.Segment[u].P1_Y-m_segment_arr.VCenterSegment.Y)*sn_radian ) + m_segment_arr.VCenterSegment.X;
		tmpArray.Segment[u].P1_Y = (int) ( -(double)(m_segment_arr.Segment[u].P1_X-m_segment_arr.VCenterSegment.X)*sn_radian + 
										   (double)(m_segment_arr.Segment[u].P1_Y-m_segment_arr.VCenterSegment.Y)*cs_radian ) + m_segment_arr.VCenterSegment.Y;

		tmpArray.Segment[u].P2_X = (int) ( (double)(m_segment_arr.Segment[u].P2_X-m_segment_arr.VCenterSegment.X)*cs_radian + 
										   (double)(m_segment_arr.Segment[u].P2_Y-m_segment_arr.VCenterSegment.Y)*sn_radian ) + m_segment_arr.VCenterSegment.X;
		tmpArray.Segment[u].P2_Y = (int) ( -(double)(m_segment_arr.Segment[u].P2_X-m_segment_arr.VCenterSegment.X)*sn_radian + 
										   (double)(m_segment_arr.Segment[u].P2_Y-m_segment_arr.VCenterSegment.Y)*cs_radian ) + m_segment_arr.VCenterSegment.Y;

		tmpArray.Segment[u].P3_X = (int) ( (double)(m_segment_arr.Segment[u].P3_X-m_segment_arr.VCenterSegment.X)*cs_radian + 
										   (double)(m_segment_arr.Segment[u].P3_Y-m_segment_arr.VCenterSegment.Y)*sn_radian ) + m_segment_arr.VCenterSegment.X;
		tmpArray.Segment[u].P3_Y = (int) ( -(double)(m_segment_arr.Segment[u].P3_X-m_segment_arr.VCenterSegment.X)*sn_radian + 
										   (double)(m_segment_arr.Segment[u].P3_Y-m_segment_arr.VCenterSegment.Y)*cs_radian ) + m_segment_arr.VCenterSegment.Y;

		tmpArray.Segment[u].P4_X = (int) ( (double)(m_segment_arr.Segment[u].P4_X-m_segment_arr.VCenterSegment.X)*cs_radian + 
										   (double)(m_segment_arr.Segment[u].P4_Y-m_segment_arr.VCenterSegment.Y)*sn_radian ) + m_segment_arr.VCenterSegment.X;
		tmpArray.Segment[u].P4_Y = (int) ( -(double)(m_segment_arr.Segment[u].P4_X-m_segment_arr.VCenterSegment.X)*sn_radian + 
										   (double)(m_segment_arr.Segment[u].P4_Y-m_segment_arr.VCenterSegment.Y)*cs_radian ) + m_segment_arr.VCenterSegment.Y;

		tmpArray.Segment[u].CenterX = (tmpArray.Segment[u].P1_X + tmpArray.Segment[u].P2_X + 
										tmpArray.Segment[u].P3_X + tmpArray.Segment[u].P4_X)/4;
		tmpArray.Segment[u].CenterY = (tmpArray.Segment[u].P1_Y + tmpArray.Segment[u].P2_Y + 
										tmpArray.Segment[u].P3_Y + tmpArray.Segment[u].P4_Y)/4;
	}

	NEW_SEGMENT new_seg;
	for(int gg=0; gg<tmpArray.SegmentCnt-1; gg++)
	{
		for(int tt=gg+1; tt<tmpArray.SegmentCnt; tt++)
		{
			if(tmpArray.Segment[gg].CenterX > tmpArray.Segment[tt].CenterX)
			{
				new_seg = tmpArray.Segment[gg];
				tmpArray.Segment[gg] = tmpArray.Segment[tt];
				tmpArray.Segment[tt] = new_seg;

				new_seg = m_segment_arr.Segment[gg];
				m_segment_arr.Segment[gg] = m_segment_arr.Segment[tt];
				m_segment_arr.Segment[tt] = new_seg;
			}
		}
	}

	if(m_segment_arr.SegmentCnt > IBSU_MAX_SEGMENT_COUNT)
		m_segment_arr.SegmentCnt = IBSU_MAX_SEGMENT_COUNT;

	int i, s, t;
	int startx, starty;

	int tmpIMG_W = IMG_W;
	int tmpIMG_H = IMG_H;
	int tmpIMG_SIZE = IMG_SIZE;
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		tmpIMG_W = IMG_W_ROLL;
		tmpIMG_H = IMG_H_ROLL;
		tmpIMG_SIZE = IMG_SIZE_ROLL;
	}

	for(i=0; i<m_segment_arr.SegmentCnt; i++)
	{
		_Algo_ClipFinger(ImgData, &m_OutSplitResultArrayEx[i*tmpIMG_SIZE], &m_OutSplitResultArrayExWidth[i], &m_OutSplitResultArrayExHeight[i], &m_OutSplitResultArrayExSize[i], 
						m_labeled_segment_enlarge_buffer, &m_segment_arr.Segment[i]);

		startx = (tmpIMG_W-m_OutSplitResultArrayExWidth[i])/2;
		starty = (tmpIMG_H-m_OutSplitResultArrayExHeight[i])/2;

		for(s=0; s<m_OutSplitResultArrayExHeight[i]; s++)
		{
			for(t=0; t<m_OutSplitResultArrayExWidth[i]; t++)
			{
				m_OutSplitResultArray[i*tmpIMG_SIZE + (s+starty)*tmpIMG_W+(startx+t)] = m_OutSplitResultArrayEx[i*tmpIMG_SIZE + s*m_OutSplitResultArrayExWidth[i]+t];
			}
		}
	}

	return m_segment_arr.SegmentCnt;
}

int CIBAlgorithm::_Algo_SegmentFinger_from_SegmentInfo_forRoll(unsigned char *pSegImg, unsigned char *pEnlargeImg, unsigned char *ImgData)
{
//	int OldSegmentCnt;
	memset(&m_segment_arr, 0, sizeof(NEW_SEGMENT_ARRAY));

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		memcpy(m_segment_enlarge_buffer_for_fingercnt, pSegImg, ZOOM_ENLAGE_W_ROLL*ZOOM_ENLAGE_H_ROLL);
		memcpy(EnlargeBuf, pEnlargeImg, ZOOM_ENLAGE_W_ROLL*ZOOM_ENLAGE_H_ROLL);
	}
	else
		return FALSE;

	_Algo_GetSegmentInfo(&m_segment_arr, 0);

	for(int i=0; i<m_segment_arr.SegmentCnt; i++)
	{
		m_segment_arr.Segment[i].Ang = 90;
	}
	m_segment_arr.VCenterSegment.Ang = 90;

	_Algo_GetMainDirectionOfFinger();

	// 4. sort segments descending order from top to bottom
	_Algo_SortSegmentByMainDir();

	// 5. merge segment if same finger
	_Algo_MergeSegmentIfSameFinger();
	
	// 6. search exact position of segment
	_Algo_SearchFingerTipPosition();

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		NEW_SEGMENT new_seg;
		for(int gg=0; gg<m_segment_arr.SegmentCnt-1; gg++)
		{
			for(int tt=gg+1; tt<m_segment_arr.SegmentCnt; tt++)
			{
				if(m_segment_arr.Segment[gg].CenterY < m_segment_arr.Segment[tt].CenterY)
				{
					new_seg = m_segment_arr.Segment[gg];
					m_segment_arr.Segment[gg] = m_segment_arr.Segment[tt];
					m_segment_arr.Segment[tt] = new_seg;
				}
			}
		}
		if(m_segment_arr.SegmentCnt > 1)
			m_segment_arr.SegmentCnt = 1;

		for(int i=0; i<ZOOM_ENLAGE_H_ROLL; i++)
		{
			for(int j=0; j<ZOOM_ENLAGE_W_ROLL; j++)
			{
				if(m_segment_enlarge_buffer_for_fingercnt[i*ZOOM_ENLAGE_W_ROLL+j] > 0)
				{
					if(m_segment_arr.Segment[0].P1_X < j)
						m_segment_arr.Segment[0].P1_X = j;
					if(m_segment_arr.Segment[0].P4_X < j)
						m_segment_arr.Segment[0].P4_X = j;

					if(m_segment_arr.Segment[0].P2_X > j)
						m_segment_arr.Segment[0].P2_X = j;
					if(m_segment_arr.Segment[0].P3_X > j)
						m_segment_arr.Segment[0].P3_X = j;

					if(m_segment_arr.Segment[0].P1_Y < i)
						m_segment_arr.Segment[0].P1_Y = i;
					if(m_segment_arr.Segment[0].P2_Y < i)
						m_segment_arr.Segment[0].P2_Y = i;

					if(m_segment_arr.Segment[0].P3_Y > i)
						m_segment_arr.Segment[0].P3_Y = i;
					if(m_segment_arr.Segment[0].P4_Y > i)
						m_segment_arr.Segment[0].P4_Y = i;
				}
			}
		}
	}

	int s, t;
	int startx, starty;

	int tmpIMG_W = IMG_W_ROLL;
	int tmpIMG_H = IMG_H_ROLL;
//	int tmpIMG_SIZE = IMG_SIZE_ROLL;
	
	_Algo_ClipFinger(ImgData, &m_OutSplitResultArrayEx[0], &m_OutSplitResultArrayExWidth[0], &m_OutSplitResultArrayExHeight[0], &m_OutSplitResultArrayExSize[0], 
						m_labeled_segment_enlarge_buffer, &m_segment_arr.Segment[0]);

	startx = (tmpIMG_W-m_OutSplitResultArrayExWidth[0])/2;
	starty = (tmpIMG_H-m_OutSplitResultArrayExHeight[0])/2;

	for(s=0; s<m_OutSplitResultArrayExHeight[0]; s++)
	{
		for(t=0; t<m_OutSplitResultArrayExWidth[0]; t++)
		{
			m_OutSplitResultArray[(s+starty)*tmpIMG_W+(startx+t)] = m_OutSplitResultArrayEx[s*m_OutSplitResultArrayExWidth[0]+t];
		}
	}

	return m_segment_arr.SegmentCnt;
}

int CIBAlgorithm::_Algo_SegmentationImage(unsigned char *ImgData)
{
	int y,x, i,j;
	int mean=0,count=0;
	int ii, tempsum;
	int xx, yy, forgroundCNT=0;
	int sum_x=0, sum_y=0;
	int tmpZOOM_ENLAGE_W, tmpZOOM_ENLAGE_H, tmpENLARGESIZE_ZOOM_W, tmpENLARGESIZE_ZOOM_H, tmpZOOM_W, tmpZOOM_H, tmpIMG_W, tmpIMG_H, tmpIMG_SIZE;

	tmpZOOM_ENLAGE_W = ZOOM_ENLAGE_W;
	tmpZOOM_ENLAGE_H = ZOOM_ENLAGE_H;
	tmpENLARGESIZE_ZOOM_W = ENLARGESIZE_ZOOM_W;
	tmpENLARGESIZE_ZOOM_H = ENLARGESIZE_ZOOM_H;
	tmpZOOM_W = ZOOM_W;
	tmpZOOM_H = ZOOM_H;
	tmpIMG_W = IMG_W;
	tmpIMG_H = IMG_H;
	tmpIMG_SIZE = tmpIMG_W*tmpIMG_H;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER &&
		m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
	{
		tmpZOOM_ENLAGE_W = ZOOM_ENLAGE_W_ROLL;
		tmpZOOM_ENLAGE_H = ZOOM_ENLAGE_H_ROLL;
		tmpENLARGESIZE_ZOOM_W = ENLARGESIZE_ZOOM_W_ROLL;
		tmpENLARGESIZE_ZOOM_H = ENLARGESIZE_ZOOM_H_ROLL;
		tmpZOOM_W = ZOOM_W_ROLL;
		tmpZOOM_H = ZOOM_H_ROLL;
		tmpIMG_W = IMG_W_ROLL;
		tmpIMG_H = IMG_H_ROLL;
		tmpIMG_SIZE = tmpIMG_W*tmpIMG_H;
	}

	memset(m_segment_enlarge_buffer, 0, tmpZOOM_ENLAGE_W*tmpZOOM_ENLAGE_H);
	memset(m_segment_enlarge_buffer_for_fingercnt, 0, tmpZOOM_ENLAGE_W*tmpZOOM_ENLAGE_H);
	memset(EnlargeBuf, 0, tmpZOOM_ENLAGE_W*tmpZOOM_ENLAGE_H);
	
	for(y=tmpENLARGESIZE_ZOOM_H; y<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H; y++)
	{
		yy = (y-tmpENLARGESIZE_ZOOM_H) * tmpIMG_H / tmpZOOM_H * tmpIMG_W;

		for(x=tmpENLARGESIZE_ZOOM_W; x<tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W; x++)
		{
			xx = (x-tmpENLARGESIZE_ZOOM_W) * tmpIMG_W / tmpZOOM_W;
			
			mean=0;
			count=0;
		
			if( (yy+xx-1-tmpIMG_W) >= 0 && (yy+xx-1-tmpIMG_W) < tmpIMG_SIZE )
			{
				mean+=ImgData[yy+xx-1-IMG_W];
				count++;
			}

			if( (yy+xx-tmpIMG_W) >= 0 && (yy+xx-tmpIMG_W) < tmpIMG_SIZE )
			{
				mean+=ImgData[yy+xx-tmpIMG_W] << 1;
				count+=2;
			}

			if( (yy+xx+1-tmpIMG_W) >= 0 && (yy+xx+1-tmpIMG_W) < tmpIMG_SIZE )
			{
				mean+=ImgData[yy+xx+1-tmpIMG_W];
				count++;
			}

			if( (yy+xx-1) >= 0 && (yy+xx-1) < tmpIMG_SIZE )						
			{
				mean+=ImgData[yy+xx-1] << 1;
				count+=2;
			}

			if( (yy+xx) >= 0 && (yy+xx) < tmpIMG_SIZE )
			{
				mean+=ImgData[yy+xx] << 2;
				count+=4;
			}

			if( (yy+xx+1) >= 0 && (yy+xx+1) < tmpIMG_SIZE )
			{
				mean+=ImgData[yy+xx+1] << 1;
				count+=2;
			}
			
			if( (yy+xx-1+tmpIMG_W) >= 0 && (yy+xx-1+tmpIMG_W) < tmpIMG_SIZE )
			{
				mean+=ImgData[yy+xx-1+tmpIMG_W];
				count++;
			}

			if( (yy+xx+tmpIMG_W) >= 0 && (yy+xx+tmpIMG_W) < tmpIMG_SIZE )
			{
				mean+=ImgData[yy+xx+tmpIMG_W] << 1;
				count+=2;
			}

			if( (yy+xx+1+tmpIMG_W) >= 0 && (yy+xx+1+tmpIMG_W) < tmpIMG_SIZE )
			{
				mean+=ImgData[yy+xx+1+tmpIMG_W];
				count++;
			}

			if(count > 0)
				mean /= count;

			EnlargeBuf[(y)*tmpZOOM_ENLAGE_W+(x)]=mean;
//			EnlargeBuf[(y)*ZOOM_ENLAGE_W+(x)]=ImgData[yy+xx];
		}
	}

	memcpy(EnlargeBuf_Org, EnlargeBuf, tmpZOOM_ENLAGE_H*tmpZOOM_ENLAGE_W);
	_Algo_HistogramNormalizeForZoomEnlarge(EnlargeBuf);

	mean = 0; count = 0;
	for (i=tmpENLARGESIZE_ZOOM_H; i<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H; i++)
	{
		for (j=tmpENLARGESIZE_ZOOM_W; j<tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W; j++)
		{
			if(EnlargeBuf[(i)*tmpZOOM_ENLAGE_W+(j)] > 10)
			{
				mean += EnlargeBuf[(i)*tmpZOOM_ENLAGE_W+(j)];
				count++;
			}
		}
	}

	if(count==0)
		return 0;

	mean /= count;

	int Threshold = (int)(pow((double)mean/255.0, 3.0) * 256);
	if(Threshold < 30)
		Threshold = 30;
	else if(Threshold > 128)
		Threshold = 128;

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_SHERLOCK)
		Threshold /= 2;
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_COLUMBO)
	{
		if(Threshold < 10)
			Threshold = 10;
		else if(Threshold > 30)
			Threshold = 30;
	}

//	TRACE("==> mean : %d, Threshold : %d\n", mean, Threshold);

	// for finger count
	for (i=tmpENLARGESIZE_ZOOM_H; i<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H; i++)
	{
		for (j=tmpENLARGESIZE_ZOOM_W; j<tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W; j++)
		{
			tempsum  = EnlargeBuf[(i-1)*tmpZOOM_ENLAGE_W+(j-1)];
			tempsum += EnlargeBuf[(i-1)*tmpZOOM_ENLAGE_W+(j)];
			tempsum += EnlargeBuf[(i-1)*tmpZOOM_ENLAGE_W+(j+1)];
			tempsum += EnlargeBuf[(i)*tmpZOOM_ENLAGE_W+(j-1)];
			tempsum += EnlargeBuf[(i)*tmpZOOM_ENLAGE_W+(j)];
			tempsum += EnlargeBuf[(i)*tmpZOOM_ENLAGE_W+(j+1)];
			tempsum += EnlargeBuf[(i+1)*tmpZOOM_ENLAGE_W+(j-1)];
			tempsum += EnlargeBuf[(i+1)*tmpZOOM_ENLAGE_W+(j)];
			tempsum += EnlargeBuf[(i+1)*tmpZOOM_ENLAGE_W+(j+1)];
			
			tempsum /= 9;
			
			// fixed bug.
			if(tempsum >= Threshold)//mean/20)
				m_segment_enlarge_buffer_for_fingercnt[i*tmpZOOM_ENLAGE_W+j] = 255;
		}
	}
	
	for(ii=0; ii<1; ii++)
	{
		memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, tmpZOOM_ENLAGE_W*tmpZOOM_ENLAGE_H);
		for (i=tmpENLARGESIZE_ZOOM_H; i<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H; i++)
		{
			for (j=tmpENLARGESIZE_ZOOM_W; j<tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W; j++)
			{
				if(m_segment_enlarge_buffer[i*ZOOM_ENLAGE_W+j] == 255)
					continue;

				tempsum  = m_segment_enlarge_buffer[(i-1)*tmpZOOM_ENLAGE_W+j-1];
				tempsum += m_segment_enlarge_buffer[(i-1)*tmpZOOM_ENLAGE_W+j  ];
				tempsum += m_segment_enlarge_buffer[(i-1)*tmpZOOM_ENLAGE_W+j+1];
				tempsum += m_segment_enlarge_buffer[(i  )*tmpZOOM_ENLAGE_W+j-1];
				tempsum += m_segment_enlarge_buffer[(i  )*tmpZOOM_ENLAGE_W+j+1];
				tempsum += m_segment_enlarge_buffer[(i+1)*tmpZOOM_ENLAGE_W+j-1];
				tempsum += m_segment_enlarge_buffer[(i+1)*tmpZOOM_ENLAGE_W+j  ];
				tempsum += m_segment_enlarge_buffer[(i+1)*tmpZOOM_ENLAGE_W+j+1];

				if(tempsum >= 4*255)
					m_segment_enlarge_buffer_for_fingercnt[i*tmpZOOM_ENLAGE_W+j] = 255;
			}
		}
	}

	memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, tmpZOOM_ENLAGE_W*tmpZOOM_ENLAGE_H);
	_Algo_StackRecursiveFilling(m_segment_enlarge_buffer, tmpZOOM_ENLAGE_W, tmpZOOM_ENLAGE_H, 0, 0, 0, 128);

	for (y=tmpENLARGESIZE_ZOOM_H; y<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H; y++)
	{
		for (x=tmpENLARGESIZE_ZOOM_W; x<tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W; x++)
		{
			if(m_segment_enlarge_buffer[y*tmpZOOM_ENLAGE_W+x] == 0)
				m_segment_enlarge_buffer_for_fingercnt[y*tmpZOOM_ENLAGE_W+x] = 255;
		}
	}

	memcpy(m_segment_enlarge_buffer,m_segment_enlarge_buffer_for_fingercnt,tmpZOOM_ENLAGE_W*tmpZOOM_ENLAGE_H);
	
	for (y=tmpENLARGESIZE_ZOOM_H; y<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H; y++)
	{
		for (x=tmpENLARGESIZE_ZOOM_W; x<tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W; x++)
		{
			if(m_segment_enlarge_buffer[(y-1)*tmpZOOM_ENLAGE_W+x-1]==0)		m_segment_enlarge_buffer_for_fingercnt[y*tmpZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer[(y-1)*tmpZOOM_ENLAGE_W+x]==0)	m_segment_enlarge_buffer_for_fingercnt[y*tmpZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer[(y-1)*tmpZOOM_ENLAGE_W+x+1]==0)	m_segment_enlarge_buffer_for_fingercnt[y*tmpZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer[y*tmpZOOM_ENLAGE_W+x-1]==0)	m_segment_enlarge_buffer_for_fingercnt[y*tmpZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer[y*tmpZOOM_ENLAGE_W+x]==0)		m_segment_enlarge_buffer_for_fingercnt[y*tmpZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer[y*tmpZOOM_ENLAGE_W+x+1]==0)	m_segment_enlarge_buffer_for_fingercnt[y*tmpZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer[(y+1)*tmpZOOM_ENLAGE_W+x-1]==0)	m_segment_enlarge_buffer_for_fingercnt[y*tmpZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer[(y+1)*tmpZOOM_ENLAGE_W+x]==0)	m_segment_enlarge_buffer_for_fingercnt[y*tmpZOOM_ENLAGE_W+x] = 0;
			else if(m_segment_enlarge_buffer[(y+1)*tmpZOOM_ENLAGE_W+x+1]==0)	m_segment_enlarge_buffer_for_fingercnt[y*tmpZOOM_ENLAGE_W+x] = 0;
			else	m_segment_enlarge_buffer_for_fingercnt[y*tmpZOOM_ENLAGE_W+x] = 255;

			if(m_segment_enlarge_buffer_for_fingercnt[y*tmpZOOM_ENLAGE_W+x] == 255)
				forgroundCNT++;
		}
	}

	if( forgroundCNT < 100 )
		return 0;

	memcpy(m_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, tmpZOOM_ENLAGE_W*tmpZOOM_ENLAGE_H);

	mean=0;
	count=0;
	for (y=tmpENLARGESIZE_ZOOM_H; y<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H; y++)
	{
		for (x=tmpENLARGESIZE_ZOOM_W; x<tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W; x++)
		{
			if(m_segment_enlarge_buffer[y*tmpZOOM_ENLAGE_W+x]==0)
				continue;
			
			if(EnlargeBuf_Org[(y)*tmpZOOM_ENLAGE_W+(x)] >= Threshold)
			{
				mean += EnlargeBuf_Org[(y)*tmpZOOM_ENLAGE_W+(x)];
				count++;
				sum_x += x*5;
				sum_y += y*5;
			}
		}
	}
	if( count==0 )
		return 0;
	
	mean /= count;

	return mean;
}

int CIBAlgorithm::_Algo_GetSegmentInfo(NEW_SEGMENT_ARRAY *segment_arr, int ImageBright)
{
	int i, j, cnt;
	int sum_x, sum_y, sum_cnt;
	int cx, cy, dx, dy, dist, sum_dx, sum_dy, sum_dxdy, sum_count, SegmentCnt;
	int Area, MinAreaThreshold = MIN_MAIN_FOREGROUND_AREA;

	int tmpZOOM_ENLAGE_W = ZOOM_ENLAGE_W;
	int tmpZOOM_ENLAGE_H = ZOOM_ENLAGE_H;
	int tmpENLARGESIZE_ZOOM_W = ENLARGESIZE_ZOOM_W;
	int tmpENLARGESIZE_ZOOM_H = ENLARGESIZE_ZOOM_H;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		MinAreaThreshold = 100;
		tmpZOOM_ENLAGE_W = ZOOM_ENLAGE_W_ROLL;
		tmpZOOM_ENLAGE_H = ZOOM_ENLAGE_H_ROLL;
		tmpENLARGESIZE_ZOOM_W = ENLARGESIZE_ZOOM_W_ROLL;
		tmpENLARGESIZE_ZOOM_H = ENLARGESIZE_ZOOM_H_ROLL;
	}

	memcpy(m_labeled_segment_enlarge_buffer, m_segment_enlarge_buffer_for_fingercnt, tmpZOOM_ENLAGE_H*tmpZOOM_ENLAGE_W);

	// foreground labeling
	SegmentCnt=0;
	for (i=tmpENLARGESIZE_ZOOM_H; i<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H; i++)
	{
		for (j=tmpENLARGESIZE_ZOOM_W; j<tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W; j++)
		{
			if(m_labeled_segment_enlarge_buffer[(i*tmpZOOM_ENLAGE_W)+j] == 255)
			{
				SegmentCnt++;
				Area = _Algo_StackRecursiveFilling_ZoomEnlarge(m_labeled_segment_enlarge_buffer, j, i, 255, SegmentCnt);

				if(Area <= MinAreaThreshold)
				{
					_Algo_StackRecursiveFilling_ZoomEnlarge(m_labeled_segment_enlarge_buffer, j, i, SegmentCnt, 0);
					SegmentCnt--;
				}
			}

			// 계산 불가
			if(SegmentCnt >= MAX_SEGMENT_COUNT) return 3;
		}
	}

	if(SegmentCnt <= 0)
		return 0;

	// calculate center position of foreground
	for(cnt=1; cnt<=SegmentCnt; cnt++)
	{
		sum_x=0;
		sum_y=0;
		sum_cnt=0;

		for (i=tmpENLARGESIZE_ZOOM_H; i<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H; i++)
		{
			for (j=tmpENLARGESIZE_ZOOM_W; j<tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W; j++)
			{
				if( m_labeled_segment_enlarge_buffer[(i*tmpZOOM_ENLAGE_W)+j] == cnt )
				{
					sum_x += j;
					sum_y += i;
					sum_cnt++;
				}
			}
		}

		if(sum_cnt >= MinAreaThreshold)
		{
			segment_arr->Segment[segment_arr->SegmentCnt].X = sum_x / sum_cnt;
			segment_arr->Segment[segment_arr->SegmentCnt].Y = sum_y / sum_cnt;
			segment_arr->Segment[segment_arr->SegmentCnt].Label = cnt;
			segment_arr->Segment[segment_arr->SegmentCnt].Area = sum_cnt;
			segment_arr->SegmentCnt++;
		}
		else
		{
			for (i=tmpENLARGESIZE_ZOOM_H; i<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H; i++)
			{
				for (j=tmpENLARGESIZE_ZOOM_W; j<tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W; j++)
				{
					if( m_labeled_segment_enlarge_buffer[(i*tmpZOOM_ENLAGE_W)+j] == cnt )
					{
						m_labeled_segment_enlarge_buffer[(i*tmpZOOM_ENLAGE_W)+j] = 0;
					}
				}
			}
		}
	}
	
	// calculate direction of foreground
	for(cnt=0; cnt<segment_arr->SegmentCnt; cnt++)
	{
		cx = segment_arr->Segment[cnt].X;
		cy = segment_arr->Segment[cnt].Y;
		sum_dx = 0;
		sum_dy = 0;
		sum_dxdy = 0;
		sum_count = 0;
		for (i=tmpENLARGESIZE_ZOOM_H; i<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H; i++)
		{
			for (j=tmpENLARGESIZE_ZOOM_W; j<tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W; j++)
			{
				if( m_labeled_segment_enlarge_buffer[i*tmpZOOM_ENLAGE_W+j] == segment_arr->Segment[cnt].Label )
				{
					dx = j - cx;
					dy = i - cy;

//					dist = SQRT_Table[abs(dy)][abs(dx)];
					dist = (int)sqrt((double)dy*dy + (double)dx*dx);

					if(dist == 0)
						continue;
					
					dx = dx * 10 / dist;
					dy = dy * 10 / dist;

					sum_dx += dx*dx;
					sum_dy += dy*dy;
					sum_dxdy += dx*dy;
					sum_count++;
				}
			}
		}

		segment_arr->Segment[cnt].Ang = _Algo_ATAN2_FULL(2*sum_dxdy, (sum_dx - sum_dy)) * 180 / 512;
	}

	return segment_arr->SegmentCnt;
}

void CIBAlgorithm::_Algo_GetMainDirectionOfFinger()
{
	int i;
	double angle_cos, angle_sin;

	angle_cos = 0;
	angle_sin = 0;

	if(m_segment_arr.SegmentCnt == 0)
		return;

	for(i=0; i<m_segment_arr.SegmentCnt; i++)
	{
		m_segment_arr.VCenterSegment.X += m_segment_arr.Segment[i].X;
		m_segment_arr.VCenterSegment.Y += m_segment_arr.Segment[i].Y;

		angle_cos += cos(m_segment_arr.Segment[i].Ang*3.14159265358979/90.0) * m_segment_arr.Segment[i].Area * 10;
		angle_sin += sin(m_segment_arr.Segment[i].Ang*3.14159265358979/90.0) * m_segment_arr.Segment[i].Area * 10;
	}

	angle_cos /= m_segment_arr.SegmentCnt;
	angle_sin /= m_segment_arr.SegmentCnt;

	m_segment_arr.VCenterSegment.X /= m_segment_arr.SegmentCnt;
	m_segment_arr.VCenterSegment.Y /= m_segment_arr.SegmentCnt;
	m_segment_arr.VCenterSegment.Ang = _Algo_ATAN2_FULL((int)angle_sin, (int)angle_cos) * 180 / 512;
}

void CIBAlgorithm::_Algo_GetMainDirectionOfFinger_Matcher()
{
	int i;
	double angle_cos, angle_sin;

	angle_cos = 0;
	angle_sin = 0;

	if(m_segment_arr_matcher.SegmentCnt == 0)
		return;

	for(i=0; i<m_segment_arr_matcher.SegmentCnt; i++)
	{
		m_segment_arr_matcher.VCenterSegment.X += m_segment_arr_matcher.Segment[i].X;
		m_segment_arr_matcher.VCenterSegment.Y += m_segment_arr_matcher.Segment[i].Y;

		angle_cos += cos(m_segment_arr_matcher.Segment[i].Ang*3.14159265358979/90.0) * m_segment_arr_matcher.Segment[i].Area * 10;
		angle_sin += sin(m_segment_arr_matcher.Segment[i].Ang*3.14159265358979/90.0) * m_segment_arr_matcher.Segment[i].Area * 10;
	}

	angle_cos /= m_segment_arr_matcher.SegmentCnt;
	angle_sin /= m_segment_arr_matcher.SegmentCnt;

	m_segment_arr_matcher.VCenterSegment.X /= m_segment_arr_matcher.SegmentCnt;
	m_segment_arr_matcher.VCenterSegment.Y /= m_segment_arr_matcher.SegmentCnt;
	m_segment_arr_matcher.VCenterSegment.Ang = _Algo_ATAN2_FULL((int)angle_sin, (int)angle_cos) * 180 / 512;
}

void CIBAlgorithm::_Algo_SwapNewSegment(NEW_SEGMENT *src1, NEW_SEGMENT *src2)
{
    NEW_SEGMENT temp;
    temp = *src1;
    *src1 = *src2;
    *src2 = temp;
}

void CIBAlgorithm::_Algo_SortSegmentByMainDir()
{
	int i, j;
	int cx, cy;
	double cs_radian, sn_radian;
	NEW_SEGMENT_ARRAY tmp_arr;
	memcpy(&tmp_arr, &m_segment_arr, sizeof(NEW_SEGMENT_ARRAY));

	cx = m_segment_arr.VCenterSegment.X;
	cy = m_segment_arr.VCenterSegment.Y;
	cs_radian = cos((m_segment_arr.VCenterSegment.Ang-90)*3.14159265358979/180.0);
	sn_radian = sin((m_segment_arr.VCenterSegment.Ang-90)*3.14159265358979/180.0);

	// rotate center pos by main dir
	for(i=0; i<m_segment_arr.SegmentCnt; i++)
	{
		tmp_arr.Segment[i].X = (int) ( (double)(m_segment_arr.Segment[i].X-cx)*cs_radian + (double)(m_segment_arr.Segment[i].Y-cy)*sn_radian ) + cx;
		tmp_arr.Segment[i].Y = (int) ( -(double)(m_segment_arr.Segment[i].X-cx)*sn_radian + (double)(m_segment_arr.Segment[i].Y-cy)*cs_radian ) + cy;
		tmp_arr.Segment[i].Ang = tmp_arr.Segment[i].Ang - m_segment_arr.VCenterSegment.Ang + 90;
	}

	// sort segment by y position
	for(i=0; i<tmp_arr.SegmentCnt-1; i++)
	{
		for(j=i+1; j<tmp_arr.SegmentCnt; j++)
		{
			if(tmp_arr.Segment[i].Y < tmp_arr.Segment[j].Y)
			{
                _Algo_SwapNewSegment(&tmp_arr.Segment[i], &tmp_arr.Segment[j]);
                _Algo_SwapNewSegment(&m_segment_arr.Segment[i], &m_segment_arr.Segment[j]);
			}
		}
	}
}

void CIBAlgorithm::_Algo_SortSegmentByMainDir_Matcher()
{
	int i, j;
	int cx, cy;
	double cs_radian, sn_radian;
	NEW_SEGMENT_ARRAY tmp_arr;
	memcpy(&tmp_arr, &m_segment_arr_matcher, sizeof(NEW_SEGMENT_ARRAY));

	cx = m_segment_arr_matcher.VCenterSegment.X;
	cy = m_segment_arr_matcher.VCenterSegment.Y;
	cs_radian = cos((m_segment_arr_matcher.VCenterSegment.Ang-90)*3.14159265358979/180.0);
	sn_radian = sin((m_segment_arr_matcher.VCenterSegment.Ang-90)*3.14159265358979/180.0);

	// rotate center pos by main dir
	for(i=0; i<m_segment_arr_matcher.SegmentCnt; i++)
	{
		tmp_arr.Segment[i].X = (int) ( (double)(m_segment_arr_matcher.Segment[i].X-cx)*cs_radian + (double)(m_segment_arr_matcher.Segment[i].Y-cy)*sn_radian ) + cx;
		tmp_arr.Segment[i].Y = (int) ( -(double)(m_segment_arr_matcher.Segment[i].X-cx)*sn_radian + (double)(m_segment_arr_matcher.Segment[i].Y-cy)*cs_radian ) + cy;
		tmp_arr.Segment[i].Ang = tmp_arr.Segment[i].Ang - m_segment_arr_matcher.VCenterSegment.Ang + 90;
	}

	// sort segment by y position
	for(i=0; i<tmp_arr.SegmentCnt-1; i++)
	{
		for(j=i+1; j<tmp_arr.SegmentCnt; j++)
		{
			if(tmp_arr.Segment[i].Y < tmp_arr.Segment[j].Y)
			{
                _Algo_SwapNewSegment(&tmp_arr.Segment[i], &tmp_arr.Segment[j]);
                _Algo_SwapNewSegment(&m_segment_arr_matcher.Segment[i], &m_segment_arr_matcher.Segment[j]);
			}
		}
	}
}

void CIBAlgorithm::_Algo_MergeSegmentIfSameFinger()
{
	int i, j, dx, dy;
	int diffdir_btw_two_pos, diffdir;
	unsigned char same_seg_flag[MAX_SEGMENT_COUNT];
	unsigned char valid_seg_flag[MAX_SEGMENT_COUNT];
	NEW_SEGMENT_ARRAY tmp_arr;

	memset(valid_seg_flag, 1, m_segment_arr.SegmentCnt);

	for(i=0; i<m_segment_arr.SegmentCnt-1; i++)
	{
		if(valid_seg_flag[i] == 0) continue;

		memset(same_seg_flag, 0xFF, m_segment_arr.SegmentCnt);

		for(j=i+1; j<m_segment_arr.SegmentCnt; j++)
		{
			if(i == j) 
				continue;
			
			if(valid_seg_flag[j] == 0) 
				continue;
			
//			if(m_segment_arr.Segment[i].Y <= m_segment_arr.Segment[j].Y)
//				continue;

			dx = m_segment_arr.Segment[j].X - m_segment_arr.Segment[i].X;
			dy = m_segment_arr.Segment[j].Y - m_segment_arr.Segment[i].Y;
			diffdir_btw_two_pos = (int)(atan2((double)dy, (double)dx) * 180.0f / 3.14159265f);
			if(diffdir_btw_two_pos < 0)	diffdir_btw_two_pos += 180;

			diffdir = diffdir_btw_two_pos - m_segment_arr.Segment[i].Ang;

			if(abs(diffdir) > THRESHOLD_DIFFERENT_ANGLE_2)
				continue;

			same_seg_flag[j] = i;
			valid_seg_flag[j] = 0;
		}
	}

	memset(&tmp_arr, 0, sizeof(NEW_SEGMENT_ARRAY));
	tmp_arr.VCenterSegment = m_segment_arr.VCenterSegment;
	for(i=0; i<m_segment_arr.SegmentCnt; i++)
	{
		if(valid_seg_flag[i] == 1)
		{
			tmp_arr.Segment[tmp_arr.SegmentCnt] = m_segment_arr.Segment[i];
			tmp_arr.SegmentCnt++;
		}
	}

	memcpy(&m_segment_arr, &tmp_arr, sizeof(NEW_SEGMENT_ARRAY));
}

void CIBAlgorithm::_Algo_MergeSegmentIfSameFinger_Matcher()
{
	int i, j, dx, dy;
	int diffdir_btw_two_pos, diffdir;
	unsigned char same_seg_flag[MAX_SEGMENT_COUNT];
	unsigned char valid_seg_flag[MAX_SEGMENT_COUNT];
	NEW_SEGMENT_ARRAY tmp_arr;

	memset(valid_seg_flag, 1, m_segment_arr_matcher.SegmentCnt);

	for(i=0; i<m_segment_arr_matcher.SegmentCnt-1; i++)
	{
		if(valid_seg_flag[i] == 0) continue;

		memset(same_seg_flag, 0xFF, m_segment_arr_matcher.SegmentCnt);

		for(j=i+1; j<m_segment_arr_matcher.SegmentCnt; j++)
		{
			if(i == j) 
				continue;
			
			if(valid_seg_flag[j] == 0) 
				continue;
			
//			if(m_segment_arr_matcher.Segment[i].Y <= m_segment_arr_matcher.Segment[j].Y)
//				continue;

			dx = m_segment_arr_matcher.Segment[j].X - m_segment_arr_matcher.Segment[i].X;
			dy = m_segment_arr_matcher.Segment[j].Y - m_segment_arr_matcher.Segment[i].Y;
			diffdir_btw_two_pos = (int)(atan2((double)dy, (double)dx) * 180.0f / 3.14159265f);
			if(diffdir_btw_two_pos < 0)	diffdir_btw_two_pos += 180;

			diffdir = diffdir_btw_two_pos - m_segment_arr_matcher.Segment[i].Ang;

			if(abs(diffdir) > THRESHOLD_DIFFERENT_ANGLE_2)
				continue;

			same_seg_flag[j] = i;
			valid_seg_flag[j] = 0;
		}
	}

	memset(&tmp_arr, 0, sizeof(NEW_SEGMENT_ARRAY));
	tmp_arr.VCenterSegment = m_segment_arr_matcher.VCenterSegment;
	for(i=0; i<m_segment_arr_matcher.SegmentCnt; i++)
	{
		if(valid_seg_flag[i] == 1)
		{
			tmp_arr.Segment[tmp_arr.SegmentCnt] = m_segment_arr_matcher.Segment[i];
			tmp_arr.SegmentCnt++;
		}
	}

	memcpy(&m_segment_arr_matcher, &tmp_arr, sizeof(NEW_SEGMENT_ARRAY));
}

void CIBAlgorithm::_Algo_SearchFingerTipPosition()
{
	int seg_cnt, i, j;
	int RotAxisX, RotAxisY, RotLineX, RotLineY;
	int Count_up, Count_dn;
	double rad, cosangle, sinangle, rad_90, cosangle_90, sinangle_90;
	int sum_mean;
	int TopImage, BottomImage, LeftImage, RightImage;
	int ZeroCount, First_zero_idx;
	int MaxVal, MinVal, CutThres, CUT_THRES;
	int *MeanV = new int[ZOOM_ENLAGE_H*10];
	int *MeanV_Filtered = new int[ZOOM_ENLAGE_H*10];
	int *MeanH = new int[ZOOM_ENLAGE_W*10];
	int *MeanH_Filtered = new int[ZOOM_ENLAGE_W*10];
	int *Mean_Count = new int[ZOOM_ENLAGE_W*10];
	int x1, y1, x2, y2, x3, y3, x4, y4;
	int Width1, Width2, Height1, Height2;
	int MaxWidth, MaxHeight;
	int ReduceX, ReduceY;
	
	int tmpZOOM_ENLAGE_W = ZOOM_ENLAGE_W;
	int tmpZOOM_ENLAGE_H = ZOOM_ENLAGE_H;
	int tmpENLARGESIZE_ZOOM_W = ENLARGESIZE_ZOOM_W;
	int tmpENLARGESIZE_ZOOM_H = ENLARGESIZE_ZOOM_H;
	int tmpZOOM_W = ZOOM_W;
	int tmpZOOM_H = ZOOM_H;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		tmpZOOM_ENLAGE_W = ZOOM_ENLAGE_W_ROLL;
		tmpZOOM_ENLAGE_H = ZOOM_ENLAGE_H_ROLL;
		tmpENLARGESIZE_ZOOM_W = ENLARGESIZE_ZOOM_W_ROLL;
		tmpENLARGESIZE_ZOOM_H = ENLARGESIZE_ZOOM_H_ROLL;
		tmpZOOM_W = ZOOM_W_ROLL;
		tmpZOOM_H = ZOOM_H_ROLL;
	}

	for(seg_cnt=0; seg_cnt<m_segment_arr.SegmentCnt; seg_cnt++)
	{
		rad = -(m_segment_arr.Segment[seg_cnt].Ang-90) * 3.141592 / 180.0;
		cosangle = cos(rad);
		sinangle = sin(rad);

		rad_90 = -(m_segment_arr.Segment[seg_cnt].Ang-180) * 3.141592 / 180.0;
		cosangle_90 = cos(rad_90);
		sinangle_90 = sin(rad_90);

		memset(MeanV, 0, tmpZOOM_ENLAGE_H*10*sizeof(int));
		memset(MeanV_Filtered, 0, tmpZOOM_ENLAGE_H*10*sizeof(int));
		memset(MeanH, 0, tmpZOOM_ENLAGE_W*10*sizeof(int));
		memset(MeanH_Filtered, 0, tmpZOOM_ENLAGE_W*10*sizeof(int));
		memset(Mean_Count, 0, tmpZOOM_ENLAGE_W*10*sizeof(int));

		for(i=1; i<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H*2; i++)
		{
			// find FingerTip pos
			RotAxisX = (int)(m_segment_arr.Segment[seg_cnt].X + i * sinangle);
			RotAxisY = (int)(m_segment_arr.Segment[seg_cnt].Y + i * cosangle);

			Count_up=0;
			Count_dn=0;

			if(RotAxisX >= tmpENLARGESIZE_ZOOM_W && RotAxisY >= tmpENLARGESIZE_ZOOM_H && 
				RotAxisX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotAxisY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
			{
				// horizontal line
				for(j=1; j<tmpZOOM_ENLAGE_W/2; j++)
				{
					// right
					RotLineX = (int)(RotAxisX + j * sinangle_90);
					RotLineY = (int)(RotAxisY + j * cosangle_90);
					if(RotLineX >= tmpENLARGESIZE_ZOOM_W && RotLineY >= tmpENLARGESIZE_ZOOM_H && 
						RotLineX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotLineY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer[RotLineY*tmpZOOM_ENLAGE_W+RotLineX] == m_segment_arr.Segment[seg_cnt].Label )
						{
							MeanV[tmpZOOM_ENLAGE_H-1-i] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Count_up++;
							MeanH[tmpZOOM_ENLAGE_W+j] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Mean_Count[tmpZOOM_ENLAGE_W+j]++;
						}
					}

					// left
					RotLineX = (int)(RotAxisX - j * sinangle_90);
					RotLineY = (int)(RotAxisY - j * cosangle_90);
					if(RotLineX >= tmpENLARGESIZE_ZOOM_W && RotLineY >= tmpENLARGESIZE_ZOOM_H && 
						RotLineX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotLineY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer[RotLineY*tmpZOOM_ENLAGE_W+RotLineX] == m_segment_arr.Segment[seg_cnt].Label )
						{
							MeanV[tmpZOOM_ENLAGE_H-1-i] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Count_up++;
							MeanH[tmpZOOM_ENLAGE_W-1-j] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Mean_Count[tmpZOOM_ENLAGE_W-1-j]++;
						}
					}
				}
			}

			// down-axis position
			RotAxisX = (int)(m_segment_arr.Segment[seg_cnt].X - i * sinangle);
			RotAxisY = (int)(m_segment_arr.Segment[seg_cnt].Y - i * cosangle);
		
			if(RotAxisX >= tmpENLARGESIZE_ZOOM_W && RotAxisY >= tmpENLARGESIZE_ZOOM_H && 
				RotAxisX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotAxisY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
			{
				// horizontal line
				for(j=1; j<tmpZOOM_ENLAGE_W/2; j++)
				{
					// right
					RotLineX = (int)(RotAxisX + j * sinangle_90);
					RotLineY = (int)(RotAxisY + j * cosangle_90);
					if(RotLineX >= tmpENLARGESIZE_ZOOM_W && RotLineY >= tmpENLARGESIZE_ZOOM_H && 
						RotLineX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotLineY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer[RotLineY*tmpZOOM_ENLAGE_W+RotLineX] == m_segment_arr.Segment[seg_cnt].Label )
						{
							MeanV[i+tmpZOOM_ENLAGE_H] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Count_dn++;
							MeanH[tmpZOOM_ENLAGE_W+j] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Mean_Count[tmpZOOM_ENLAGE_W+j]++;
						}
					}

					// left
					RotLineX = (int)(RotAxisX - j * sinangle_90);
					RotLineY = (int)(RotAxisY - j * cosangle_90);
					if(RotLineX >= tmpENLARGESIZE_ZOOM_W && RotLineY >= tmpENLARGESIZE_ZOOM_H && 
						RotLineX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotLineY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer[RotLineY*tmpZOOM_ENLAGE_W+RotLineX] == m_segment_arr.Segment[seg_cnt].Label )
						{
							MeanV[i+tmpZOOM_ENLAGE_H] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Count_dn++;
							MeanH[tmpZOOM_ENLAGE_W-1-j] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Mean_Count[tmpZOOM_ENLAGE_W-1-j]++;
						}
					}
				}
			}

			if(Count_up > 0)	MeanV[tmpZOOM_ENLAGE_H-1-i] /= Count_up;
			if(Count_dn > 0)	MeanV[i+tmpZOOM_ENLAGE_H] /= Count_dn;
		}

		for(i=0; i<tmpZOOM_ENLAGE_W*2; i++)
		{
			if(Mean_Count[i] > 0)
				MeanH[i] /= Mean_Count[i];
		}

		MeanV[tmpZOOM_ENLAGE_H] = (MeanV[tmpZOOM_ENLAGE_H-1]+MeanV[tmpZOOM_ENLAGE_H+1])/2;
		MeanH[tmpZOOM_ENLAGE_W] = (MeanH[tmpZOOM_ENLAGE_W-1]+MeanH[tmpZOOM_ENLAGE_W+1])/2;

		// for horizontal
		MaxVal=-1;
		MinVal=1000000;
		for(i=2; i<tmpZOOM_ENLAGE_W*2-2; i++)
		{
			sum_mean = MeanH[i-1];
			sum_mean += MeanH[i  ];
			sum_mean += MeanH[i+1];

			sum_mean /= 3;
			MeanH_Filtered[i] = sum_mean;

			if(MaxVal < MeanH_Filtered[i])
				MaxVal = MeanH_Filtered[i];
			if(MinVal > MeanH_Filtered[i])
				MinVal = MeanH_Filtered[i];
		}

//		LeftImage=tmpZOOM_ENLAGE_W*2, RightImage=0;
		LeftImage= 2, RightImage=tmpZOOM_ENLAGE_W*2-2;
		for(i=2; i<tmpZOOM_ENLAGE_W*2-2; i++)
		{
			if(MeanH_Filtered[i] > 5)
			{
				LeftImage = i;
				break;
			}
		}
		for(i=tmpZOOM_ENLAGE_W*2-2; i>=2; i--)
		{
			if(MeanH_Filtered[i] > 5)
			{
				RightImage = i;
				break;
			}
		}

		LeftImage -= 1;
		RightImage += 1;

		// mean filter
		MaxVal=-1;
		MinVal=1000000;
		for(i=2; i<tmpZOOM_ENLAGE_H*2-2; i++)
		{
			sum_mean = MeanV[i-1];
			sum_mean += MeanV[i  ];
			sum_mean += MeanV[i+1];

			sum_mean /= 3;
			MeanV_Filtered[i] = sum_mean;

			if(MaxVal < MeanV_Filtered[i])
				MaxVal = MeanV_Filtered[i];
			if(MinVal > MeanV_Filtered[i])
				MinVal = MeanV_Filtered[i];
		}

		// calculate image height
//		TopImage=tmpZOOM_ENLAGE_H*2, BottomImage=0;
		TopImage=2, BottomImage=tmpZOOM_ENLAGE_H*2-2;
		for(i=2; i<tmpZOOM_ENLAGE_H*2-2; i++)
		{
			if(MeanV_Filtered[i] > 5)
			{
				TopImage = i;
				break;
			}
		}
		for(i=tmpZOOM_ENLAGE_H*2-2; i>=2; i--)
		{
			if(MeanV_Filtered[i] > 5)
			{
				BottomImage = i;
				break;
			}
		}

		int ClipFingerWidth = (RightImage-LeftImage);
		if(ClipFingerWidth < 50)
			ClipFingerWidth = 50;
		
		MaxVal=-1;
		MinVal=1000000;
		for(i=TopImage+(RightImage-LeftImage); i<BottomImage; i++)
		{
			sum_mean = MeanV[i-1];
			sum_mean += MeanV[i  ];
			sum_mean += MeanV[i+1];

			sum_mean /= 3;
			MeanV_Filtered[i] = sum_mean;

			if(MaxVal < MeanV_Filtered[i])
				MaxVal = MeanV_Filtered[i];
			if(MinVal > MeanV_Filtered[i])
				MinVal = MeanV_Filtered[i];
		}

		// find first under threshold
		// for vertical
		ZeroCount=0;
		First_zero_idx = BottomImage;

/*		FILE *fp = fopen("d:\\MeanV_Filtered.csv", "wt");
		fprintf(fp, "MaxVal,%d,MinVal,%d,(MaxVal-MinVal),%d\n", MaxVal, MinVal, MaxVal-MinVal);
		for(int lo=0; lo<ZOOM_ENLAGE_H*2; lo++)
		{
			fprintf(fp, "%d,%d\n", lo, MeanV_Filtered[lo]);
		}
		fclose(fp);
*/
		CutThres = (MaxVal-MinVal)/3;

		/*if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			CutThres = (MaxVal-MinVal)/4;
		else if(m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
				m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_TWO_FINGERS)
		{
			CutThres = (MaxVal-MinVal)/3;
			if(MaxVal < 50)
				CutThres = 5;
			else if(MaxVal < 75)
				CutThres = 10;
			else if(MaxVal < 100)
				CutThres = 15;
			else
				CutThres = 20;
		}
		else if(m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
				m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS)
		{
			CutThres = (MaxVal-MinVal)/3;
			if(MaxVal < 50)
				CutThres = 10;
			else if(MaxVal < 75)
				CutThres = 25;
			else if(MaxVal < 100)
				CutThres = 40;
			else
				CutThres = 55;
		}
		else
		{
			CutThres = (MaxVal-MinVal)/4;
			if(MaxVal < 50)
				CutThres = 20;
			else if(MaxVal < 75)
				CutThres = 30;
			else if(MaxVal < 100)
				CutThres = 40;
			else if(MaxVal < 125)
				CutThres = 50;
			else if(MaxVal < 150)
				CutThres = 60;
			else if(MaxVal < 175)
				CutThres = 70;
			else
				CutThres = 80;
		}*/

		if(TopImage+ClipFingerWidth*1.5 > BottomImage)
		{
			First_zero_idx = BottomImage;
		}
		else
		{
			for(i=(int)(TopImage+ClipFingerWidth*1.5f); i<BottomImage; i++)
			{
				if(i < TopImage+ClipFingerWidth*1.5+30)
					CUT_THRES = MinVal + CutThres*2;
				else if(i < TopImage+ClipFingerWidth*1.5+60)
					CUT_THRES = MinVal + CutThres*3;
				else if(i < TopImage+ClipFingerWidth*1.5+90)
					CUT_THRES = MinVal + CutThres*4;
				else
					CUT_THRES = MinVal + CutThres*5;

				if(MeanV_Filtered[i] < CUT_THRES && MeanV_Filtered[i] >= 0)
					ZeroCount++;
				else
					ZeroCount=0;
				
				if(ZeroCount >= 2)
				{
					First_zero_idx = i;
					break;
				}
			}
		}

		TopImage -= 1;
		First_zero_idx += 2;

		x1 = (LeftImage-tmpZOOM_ENLAGE_W);
		y1 = (tmpZOOM_ENLAGE_H-TopImage);

		x2 = (RightImage-tmpZOOM_ENLAGE_W);
		y2 = (tmpZOOM_ENLAGE_H-TopImage);

		x3 = (RightImage-tmpZOOM_ENLAGE_W);
		y3 = (tmpZOOM_ENLAGE_H-First_zero_idx);

		x4 = (LeftImage-tmpZOOM_ENLAGE_W);
		y4 = (tmpZOOM_ENLAGE_H-First_zero_idx);

		ReduceX=0;
		ReduceY=0;
		Width1 = abs(x2-x1)+1;
		Width2 = abs(x3-x4)+1;
		Height1 = abs(y1-y4)+1;
		Height2 = abs(y2-y3)+1;

		if(Width1 > Width2)		MaxWidth = Width1;
		else					MaxWidth = Width2;
		if(Height1 > Height2)	MaxHeight = Height1;
		else					MaxHeight = Height2;
		
		if(MaxWidth > tmpZOOM_W)
			ReduceX = (int)((double)(MaxWidth - tmpZOOM_W)/2.0+0.5);
		if(MaxHeight > tmpZOOM_H)
			ReduceY = (int)((double)(MaxHeight - tmpZOOM_H)/2.0+0.5);

		x1 = x1 + ReduceX;
		x4 = x4 + ReduceX;
		x2 = x2 - ReduceX;
		x3 = x3 - ReduceX;

		y1 = y1 - ReduceY;
		y4 = y4 + ReduceY;
		y2 = y2 - ReduceY;
		y3 = y3 + ReduceY;

		m_segment_arr.Segment[seg_cnt].TipX = (int)(m_segment_arr.Segment[seg_cnt].X + y1 * sinangle);
		m_segment_arr.Segment[seg_cnt].TipY = (int)(m_segment_arr.Segment[seg_cnt].Y + y1 * cosangle);
		m_segment_arr.Segment[seg_cnt].KnuckleX = (int)(m_segment_arr.Segment[seg_cnt].X + y3 * sinangle);
		m_segment_arr.Segment[seg_cnt].KnuckleY = (int)(m_segment_arr.Segment[seg_cnt].Y + y3 * cosangle);
		m_segment_arr.Segment[seg_cnt].LeftX = (int)(m_segment_arr.Segment[seg_cnt].X + x1 * sinangle_90);
		m_segment_arr.Segment[seg_cnt].LeftY = (int)(m_segment_arr.Segment[seg_cnt].Y + x1 * cosangle_90);
		m_segment_arr.Segment[seg_cnt].RightX = (int)(m_segment_arr.Segment[seg_cnt].X + x2 * sinangle_90);
		m_segment_arr.Segment[seg_cnt].RightY = (int)(m_segment_arr.Segment[seg_cnt].Y + x2 * cosangle_90);
		m_segment_arr.Segment[seg_cnt].CenterX = (m_segment_arr.Segment[seg_cnt].KnuckleX + m_segment_arr.Segment[seg_cnt].TipX) / 2;
		m_segment_arr.Segment[seg_cnt].CenterY = (m_segment_arr.Segment[seg_cnt].KnuckleY + m_segment_arr.Segment[seg_cnt].TipY) / 2;

		m_segment_arr.Segment[seg_cnt].P1_X = (int)(x2*cosangle+y1*sinangle + m_segment_arr.Segment[seg_cnt].X);
		m_segment_arr.Segment[seg_cnt].P1_Y = (int)(-x2*sinangle+y1*cosangle + m_segment_arr.Segment[seg_cnt].Y);
		m_segment_arr.Segment[seg_cnt].P2_X = (int)(x1*cosangle+y1*sinangle + m_segment_arr.Segment[seg_cnt].X);
		m_segment_arr.Segment[seg_cnt].P2_Y = (int)(-x1*sinangle+y1*cosangle + m_segment_arr.Segment[seg_cnt].Y);
		m_segment_arr.Segment[seg_cnt].P3_X = (int)(x1*cosangle+y3*sinangle + m_segment_arr.Segment[seg_cnt].X);
		m_segment_arr.Segment[seg_cnt].P3_Y = (int)(-x1*sinangle+y3*cosangle + m_segment_arr.Segment[seg_cnt].Y);
		m_segment_arr.Segment[seg_cnt].P4_X = (int)(x2*cosangle+y3*sinangle + m_segment_arr.Segment[seg_cnt].X);
		m_segment_arr.Segment[seg_cnt].P4_Y = (int)(-x2*sinangle+y3*cosangle + m_segment_arr.Segment[seg_cnt].Y);

/*		m_segment_arr.Segment[seg_cnt].P1_X = m_segment_arr.Segment[seg_cnt].TipX - m_segment_arr.Segment[seg_cnt].X + m_segment_arr.Segment[seg_cnt].RightX;
		m_segment_arr.Segment[seg_cnt].P1_Y = m_segment_arr.Segment[seg_cnt].TipY - m_segment_arr.Segment[seg_cnt].Y + m_segment_arr.Segment[seg_cnt].RightY;
		m_segment_arr.Segment[seg_cnt].P2_X = m_segment_arr.Segment[seg_cnt].TipX - m_segment_arr.Segment[seg_cnt].X + m_segment_arr.Segment[seg_cnt].LeftX;
		m_segment_arr.Segment[seg_cnt].P2_Y = m_segment_arr.Segment[seg_cnt].TipY - m_segment_arr.Segment[seg_cnt].Y + m_segment_arr.Segment[seg_cnt].LeftY;
		m_segment_arr.Segment[seg_cnt].P3_X = m_segment_arr.Segment[seg_cnt].KnuckleX - m_segment_arr.Segment[seg_cnt].X + m_segment_arr.Segment[seg_cnt].LeftX;
		m_segment_arr.Segment[seg_cnt].P3_Y = m_segment_arr.Segment[seg_cnt].KnuckleY - m_segment_arr.Segment[seg_cnt].Y + m_segment_arr.Segment[seg_cnt].LeftY;
		m_segment_arr.Segment[seg_cnt].P4_X = m_segment_arr.Segment[seg_cnt].KnuckleX - m_segment_arr.Segment[seg_cnt].X + m_segment_arr.Segment[seg_cnt].RightX;
		m_segment_arr.Segment[seg_cnt].P4_Y = m_segment_arr.Segment[seg_cnt].KnuckleY - m_segment_arr.Segment[seg_cnt].Y + m_segment_arr.Segment[seg_cnt].RightY;
*/
		////////////////////////////////////////////////////////////////////////////////////
		// remove crease area in image
		// Line Equation : y = sin(BackDir) / cos(BackDir) * (x + BackX) + BackY
		RotAxisX = m_segment_arr.Segment[seg_cnt].KnuckleX;
		RotAxisY = m_segment_arr.Segment[seg_cnt].KnuckleY;
		
		rad = (m_segment_arr.Segment[seg_cnt].Ang-90) * 3.141592 / 180.0;
		cosangle = cos(rad);
		sinangle = sin(rad);

		for(i=tmpENLARGESIZE_ZOOM_H; i<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H; i++)
		{
			for(j=tmpENLARGESIZE_ZOOM_W; j<tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W; j++)
			{
				if(cosangle != 0)
				{
					if( i < (int)(sinangle / cosangle * (j - RotAxisX) + RotAxisY))
					{
						if(m_labeled_segment_enlarge_buffer[i*tmpZOOM_ENLAGE_W+j] == m_segment_arr.Segment[seg_cnt].Label)
							m_labeled_segment_enlarge_buffer[i*tmpZOOM_ENLAGE_W+j] = 0;
					}
				}
				else
				{
					if( i < RotAxisY)
					{
						if(m_labeled_segment_enlarge_buffer[i*tmpZOOM_ENLAGE_W+j] == m_segment_arr.Segment[seg_cnt].Label)
							m_labeled_segment_enlarge_buffer[i*tmpZOOM_ENLAGE_W+j] = 0;
					}
				}
			}
		}
		////////////////////////////////////////////////////////////////////////////////////	
	}

	delete [] MeanV;
	delete [] MeanV_Filtered;
	delete [] MeanH;
	delete [] MeanH_Filtered;
	delete [] Mean_Count;
}

void CIBAlgorithm::_Algo_SearchFingerTipPosition_90()
{
	int seg_cnt, i, j;
	int RotAxisX, RotAxisY, RotLineX, RotLineY;
	int Count_up, Count_dn;
	double rad, cosangle, sinangle, rad_90, cosangle_90, sinangle_90;
	int sum_mean;
	int TopImage, BottomImage, LeftImage, RightImage;
	int ZeroCount, First_zero_idx;
	int MaxVal, MinVal, CutThres, CUT_THRES;
	int *MeanV = new int[ZOOM_ENLAGE_H*10];
	int *MeanV_Filtered = new int[ZOOM_ENLAGE_H*10];
	int *MeanH = new int[ZOOM_ENLAGE_W*10];
	int *MeanH_Filtered = new int[ZOOM_ENLAGE_W*10];
	int *Mean_Count = new int[ZOOM_ENLAGE_W*10];
	int x1, y1, x2, y2, x3, y3, x4, y4;
	int Width1, Width2, Height1, Height2;
	int MaxWidth, MaxHeight;
	int ReduceX, ReduceY;
	
	int tmpZOOM_ENLAGE_W = ZOOM_ENLAGE_W;
	int tmpZOOM_ENLAGE_H = ZOOM_ENLAGE_H;
	int tmpENLARGESIZE_ZOOM_W = ENLARGESIZE_ZOOM_W;
	int tmpENLARGESIZE_ZOOM_H = ENLARGESIZE_ZOOM_H;
	int tmpZOOM_W = ZOOM_W;
	int tmpZOOM_H = ZOOM_H;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		tmpZOOM_ENLAGE_W = ZOOM_ENLAGE_W_ROLL;
		tmpZOOM_ENLAGE_H = ZOOM_ENLAGE_H_ROLL;
		tmpENLARGESIZE_ZOOM_W = ENLARGESIZE_ZOOM_W_ROLL;
		tmpENLARGESIZE_ZOOM_H = ENLARGESIZE_ZOOM_H_ROLL;
		tmpZOOM_W = ZOOM_W_ROLL;
		tmpZOOM_H = ZOOM_H_ROLL;
	}

	for(seg_cnt=0; seg_cnt<m_segment_arr_90.SegmentCnt; seg_cnt++)
	{
		rad = -(m_segment_arr_90.Segment[seg_cnt].Ang-90) * 3.141592 / 180.0;
		cosangle = cos(rad);
		sinangle = sin(rad);

		rad_90 = -(m_segment_arr_90.Segment[seg_cnt].Ang-180) * 3.141592 / 180.0;
		cosangle_90 = cos(rad_90);
		sinangle_90 = sin(rad_90);

		memset(MeanV, 0, tmpZOOM_ENLAGE_H*10*sizeof(int));
		memset(MeanV_Filtered, 0, tmpZOOM_ENLAGE_H*10*sizeof(int));
		memset(MeanH, 0, tmpZOOM_ENLAGE_W*10*sizeof(int));
		memset(MeanH_Filtered, 0, tmpZOOM_ENLAGE_W*10*sizeof(int));
		memset(Mean_Count, 0, tmpZOOM_ENLAGE_W*10*sizeof(int));

		for(i=1; i<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H*2; i++)
		{
			// find FingerTip pos
			RotAxisX = (int)(m_segment_arr_90.Segment[seg_cnt].X + i * sinangle);
			RotAxisY = (int)(m_segment_arr_90.Segment[seg_cnt].Y + i * cosangle);

			Count_up=0;
			Count_dn=0;

			if(RotAxisX >= tmpENLARGESIZE_ZOOM_W && RotAxisY >= tmpENLARGESIZE_ZOOM_H && 
				RotAxisX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotAxisY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
			{
				// horizontal line
				for(j=1; j<tmpZOOM_ENLAGE_W/2; j++)
				{
					// right
					RotLineX = (int)(RotAxisX + j * sinangle_90);
					RotLineY = (int)(RotAxisY + j * cosangle_90);
					if(RotLineX >= tmpENLARGESIZE_ZOOM_W && RotLineY >= tmpENLARGESIZE_ZOOM_H && 
						RotLineX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotLineY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer[RotLineY*tmpZOOM_ENLAGE_W+RotLineX] == m_segment_arr_90.Segment[seg_cnt].Label )
						{
							MeanV[tmpZOOM_ENLAGE_H-1-i] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Count_up++;
							MeanH[tmpZOOM_ENLAGE_W+j] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Mean_Count[tmpZOOM_ENLAGE_W+j]++;
						}
					}

					// left
					RotLineX = (int)(RotAxisX - j * sinangle_90);
					RotLineY = (int)(RotAxisY - j * cosangle_90);
					if(RotLineX >= tmpENLARGESIZE_ZOOM_W && RotLineY >= tmpENLARGESIZE_ZOOM_H && 
						RotLineX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotLineY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer[RotLineY*tmpZOOM_ENLAGE_W+RotLineX] == m_segment_arr_90.Segment[seg_cnt].Label )
						{
							MeanV[tmpZOOM_ENLAGE_H-1-i] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Count_up++;
							MeanH[tmpZOOM_ENLAGE_W-1-j] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Mean_Count[tmpZOOM_ENLAGE_W-1-j]++;
						}
					}
				}
			}

			// down-axis position
			RotAxisX = (int)(m_segment_arr_90.Segment[seg_cnt].X - i * sinangle);
			RotAxisY = (int)(m_segment_arr_90.Segment[seg_cnt].Y - i * cosangle);
		
			if(RotAxisX >= tmpENLARGESIZE_ZOOM_W && RotAxisY >= tmpENLARGESIZE_ZOOM_H && 
				RotAxisX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotAxisY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
			{
				// horizontal line
				for(j=1; j<tmpZOOM_ENLAGE_W/2; j++)
				{
					// right
					RotLineX = (int)(RotAxisX + j * sinangle_90);
					RotLineY = (int)(RotAxisY + j * cosangle_90);
					if(RotLineX >= tmpENLARGESIZE_ZOOM_W && RotLineY >= tmpENLARGESIZE_ZOOM_H && 
						RotLineX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotLineY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer[RotLineY*tmpZOOM_ENLAGE_W+RotLineX] == m_segment_arr_90.Segment[seg_cnt].Label )
						{
							MeanV[i+tmpZOOM_ENLAGE_H] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Count_dn++;
							MeanH[tmpZOOM_ENLAGE_W+j] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Mean_Count[tmpZOOM_ENLAGE_W+j]++;
						}
					}

					// left
					RotLineX = (int)(RotAxisX - j * sinangle_90);
					RotLineY = (int)(RotAxisY - j * cosangle_90);
					if(RotLineX >= tmpENLARGESIZE_ZOOM_W && RotLineY >= tmpENLARGESIZE_ZOOM_H && 
						RotLineX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotLineY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer[RotLineY*tmpZOOM_ENLAGE_W+RotLineX] == m_segment_arr_90.Segment[seg_cnt].Label )
						{
							MeanV[i+tmpZOOM_ENLAGE_H] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Count_dn++;
							MeanH[tmpZOOM_ENLAGE_W-1-j] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Mean_Count[tmpZOOM_ENLAGE_W-1-j]++;
						}
					}
				}
			}

			if(Count_up > 0)	MeanV[tmpZOOM_ENLAGE_H-1-i] /= Count_up;
			if(Count_dn > 0)	MeanV[i+tmpZOOM_ENLAGE_H] /= Count_dn;
		}

		for(i=0; i<tmpZOOM_ENLAGE_W*2; i++)
		{
			if(Mean_Count[i] > 0)
				MeanH[i] /= Mean_Count[i];
		}

		MeanV[tmpZOOM_ENLAGE_H] = (MeanV[tmpZOOM_ENLAGE_H-1]+MeanV[tmpZOOM_ENLAGE_H+1])/2;
		MeanH[tmpZOOM_ENLAGE_W] = (MeanH[tmpZOOM_ENLAGE_W-1]+MeanH[tmpZOOM_ENLAGE_W+1])/2;

		// for horizontal
		MaxVal=-1;
		MinVal=1000000;
		for(i=2; i<tmpZOOM_ENLAGE_W*2-2; i++)
		{
			sum_mean = MeanH[i-1];
			sum_mean += MeanH[i  ];
			sum_mean += MeanH[i+1];

			sum_mean /= 3;
			MeanH_Filtered[i] = sum_mean;

			if(MaxVal < MeanH_Filtered[i])
				MaxVal = MeanH_Filtered[i];
			if(MinVal > MeanH_Filtered[i])
				MinVal = MeanH_Filtered[i];
		}

//		LeftImage=tmpZOOM_ENLAGE_W*2, RightImage=0;
		LeftImage= 2, RightImage=tmpZOOM_ENLAGE_W*2-2;
		for(i=2; i<tmpZOOM_ENLAGE_W*2-2; i++)
		{
			if(MeanH_Filtered[i] > 5)
			{
				LeftImage = i;
				break;
			}
		}
		for(i=tmpZOOM_ENLAGE_W*2-2; i>=2; i--)
		{
			if(MeanH_Filtered[i] > 5)
			{
				RightImage = i;
				break;
			}
		}

		LeftImage -= 1;
		RightImage += 1;

		// mean filter
		MaxVal=-1;
		MinVal=1000000;
		for(i=2; i<tmpZOOM_ENLAGE_H*2-2; i++)
		{
			sum_mean = MeanV[i-1];
			sum_mean += MeanV[i  ];
			sum_mean += MeanV[i+1];

			sum_mean /= 3;
			MeanV_Filtered[i] = sum_mean;

			if(MaxVal < MeanV_Filtered[i])
				MaxVal = MeanV_Filtered[i];
			if(MinVal > MeanV_Filtered[i])
				MinVal = MeanV_Filtered[i];
		}

		// calculate image height
//		TopImage=tmpZOOM_ENLAGE_H*2, BottomImage=0;
		TopImage=2, BottomImage=tmpZOOM_ENLAGE_H*2-2;
		for(i=2; i<tmpZOOM_ENLAGE_H*2-2; i++)
		{
			if(MeanV_Filtered[i] > 5)
			{
				TopImage = i;
				break;
			}
		}
		for(i=tmpZOOM_ENLAGE_H*2-2; i>=2; i--)
		{
			if(MeanV_Filtered[i] > 5)
			{
				BottomImage = i;
				break;
			}
		}

		int ClipFingerWidth = (RightImage-LeftImage);
		if(ClipFingerWidth < 50)
			ClipFingerWidth = 50;
		
		MaxVal=-1;
		MinVal=1000000;
		for(i=TopImage+(RightImage-LeftImage); i<BottomImage; i++)
		{
			sum_mean = MeanV[i-1];
			sum_mean += MeanV[i  ];
			sum_mean += MeanV[i+1];

			sum_mean /= 3;
			MeanV_Filtered[i] = sum_mean;

			if(MaxVal < MeanV_Filtered[i])
				MaxVal = MeanV_Filtered[i];
			if(MinVal > MeanV_Filtered[i])
				MinVal = MeanV_Filtered[i];
		}

		// find first under threshold
		// for vertical
		ZeroCount=0;
		First_zero_idx = BottomImage;

/*		FILE *fp = fopen("d:\\MeanV_Filtered.csv", "wt");
		fprintf(fp, "MaxVal,%d,MinVal,%d,(MaxVal-MinVal),%d\n", MaxVal, MinVal, MaxVal-MinVal);
		for(int lo=0; lo<ZOOM_ENLAGE_H*2; lo++)
		{
			fprintf(fp, "%d,%d\n", lo, MeanV_Filtered[lo]);
		}
		fclose(fp);
*/
		CutThres = (MaxVal-MinVal)/3;

		/*if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			CutThres = (MaxVal-MinVal)/4;
		else if(m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
				m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_TWO_FINGERS)
		{
			CutThres = (MaxVal-MinVal)/3;
			if(MaxVal < 50)
				CutThres = 5;
			else if(MaxVal < 75)
				CutThres = 10;
			else if(MaxVal < 100)
				CutThres = 15;
			else
				CutThres = 20;
		}
		else if(m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
				m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS)
		{
			CutThres = (MaxVal-MinVal)/3;
			if(MaxVal < 50)
				CutThres = 10;
			else if(MaxVal < 75)
				CutThres = 25;
			else if(MaxVal < 100)
				CutThres = 40;
			else
				CutThres = 55;
		}
		else
		{
			CutThres = (MaxVal-MinVal)/4;
			if(MaxVal < 50)
				CutThres = 20;
			else if(MaxVal < 75)
				CutThres = 30;
			else if(MaxVal < 100)
				CutThres = 40;
			else if(MaxVal < 125)
				CutThres = 50;
			else if(MaxVal < 150)
				CutThres = 60;
			else if(MaxVal < 175)
				CutThres = 70;
			else
				CutThres = 80;
		}*/

		if(TopImage+ClipFingerWidth*1.5 > BottomImage)
		{
			First_zero_idx = BottomImage;
		}
		else
		{
			for(i=(int)(TopImage+ClipFingerWidth*1.5f); i<BottomImage; i++)
			{
				if(i < TopImage+ClipFingerWidth*1.5+30)
					CUT_THRES = MinVal + CutThres*2;
				else if(i < TopImage+ClipFingerWidth*1.5+60)
					CUT_THRES = MinVal + CutThres*3;
				else if(i < TopImage+ClipFingerWidth*1.5+90)
					CUT_THRES = MinVal + CutThres*4;
				else
					CUT_THRES = MinVal + CutThres*5;

				if(MeanV_Filtered[i] < CUT_THRES && MeanV_Filtered[i] >= 0)
					ZeroCount++;
				else
					ZeroCount=0;
				
				if(ZeroCount >= 2)
				{
					First_zero_idx = i;
					break;
				}
			}
		}

		TopImage -= 1;
		First_zero_idx += 2;

		x1 = (LeftImage-tmpZOOM_ENLAGE_W);
		y1 = (tmpZOOM_ENLAGE_H-TopImage);

		x2 = (RightImage-tmpZOOM_ENLAGE_W);
		y2 = (tmpZOOM_ENLAGE_H-TopImage);

		x3 = (RightImage-tmpZOOM_ENLAGE_W);
		y3 = (tmpZOOM_ENLAGE_H-First_zero_idx);

		x4 = (LeftImage-tmpZOOM_ENLAGE_W);
		y4 = (tmpZOOM_ENLAGE_H-First_zero_idx);

		ReduceX=0;
		ReduceY=0;
		Width1 = abs(x2-x1)+1;
		Width2 = abs(x3-x4)+1;
		Height1 = abs(y1-y4)+1;
		Height2 = abs(y2-y3)+1;

		if(Width1 > Width2)		MaxWidth = Width1;
		else					MaxWidth = Width2;
		if(Height1 > Height2)	MaxHeight = Height1;
		else					MaxHeight = Height2;
		
		if(MaxWidth > tmpZOOM_W)
			ReduceX = (int)((double)(MaxWidth - tmpZOOM_W)/2.0+0.5);
		if(MaxHeight > tmpZOOM_H)
			ReduceY = (int)((double)(MaxHeight - tmpZOOM_H)/2.0+0.5);

		x1 = x1 + ReduceX;
		x4 = x4 + ReduceX;
		x2 = x2 - ReduceX;
		x3 = x3 - ReduceX;

		y1 = y1 - ReduceY;
		y4 = y4 + ReduceY;
		y2 = y2 - ReduceY;
		y3 = y3 + ReduceY;

		m_segment_arr_90.Segment[seg_cnt].TipX = (int)(m_segment_arr_90.Segment[seg_cnt].X + y1 * sinangle);
		m_segment_arr_90.Segment[seg_cnt].TipY = (int)(m_segment_arr_90.Segment[seg_cnt].Y + y1 * cosangle);
		m_segment_arr_90.Segment[seg_cnt].KnuckleX = (int)(m_segment_arr_90.Segment[seg_cnt].X + y3 * sinangle);
		m_segment_arr_90.Segment[seg_cnt].KnuckleY = (int)(m_segment_arr_90.Segment[seg_cnt].Y + y3 * cosangle);
		m_segment_arr_90.Segment[seg_cnt].LeftX = (int)(m_segment_arr_90.Segment[seg_cnt].X + x1 * sinangle_90);
		m_segment_arr_90.Segment[seg_cnt].LeftY = (int)(m_segment_arr_90.Segment[seg_cnt].Y + x1 * cosangle_90);
		m_segment_arr_90.Segment[seg_cnt].RightX = (int)(m_segment_arr_90.Segment[seg_cnt].X + x2 * sinangle_90);
		m_segment_arr_90.Segment[seg_cnt].RightY = (int)(m_segment_arr_90.Segment[seg_cnt].Y + x2 * cosangle_90);
		m_segment_arr_90.Segment[seg_cnt].CenterX = (m_segment_arr_90.Segment[seg_cnt].KnuckleX + m_segment_arr_90.Segment[seg_cnt].TipX) / 2;
		m_segment_arr_90.Segment[seg_cnt].CenterY = (m_segment_arr_90.Segment[seg_cnt].KnuckleY + m_segment_arr_90.Segment[seg_cnt].TipY) / 2;

		m_segment_arr_90.Segment[seg_cnt].P1_X = (int)(x2*cosangle+y1*sinangle + m_segment_arr_90.Segment[seg_cnt].X);
		m_segment_arr_90.Segment[seg_cnt].P1_Y = (int)(-x2*sinangle+y1*cosangle + m_segment_arr_90.Segment[seg_cnt].Y);
		m_segment_arr_90.Segment[seg_cnt].P2_X = (int)(x1*cosangle+y1*sinangle + m_segment_arr_90.Segment[seg_cnt].X);
		m_segment_arr_90.Segment[seg_cnt].P2_Y = (int)(-x1*sinangle+y1*cosangle + m_segment_arr_90.Segment[seg_cnt].Y);
		m_segment_arr_90.Segment[seg_cnt].P3_X = (int)(x1*cosangle+y3*sinangle + m_segment_arr_90.Segment[seg_cnt].X);
		m_segment_arr_90.Segment[seg_cnt].P3_Y = (int)(-x1*sinangle+y3*cosangle + m_segment_arr_90.Segment[seg_cnt].Y);
		m_segment_arr_90.Segment[seg_cnt].P4_X = (int)(x2*cosangle+y3*sinangle + m_segment_arr_90.Segment[seg_cnt].X);
		m_segment_arr_90.Segment[seg_cnt].P4_Y = (int)(-x2*sinangle+y3*cosangle + m_segment_arr_90.Segment[seg_cnt].Y);

/*		m_segment_arr_90.Segment[seg_cnt].P1_X = m_segment_arr_90.Segment[seg_cnt].TipX - m_segment_arr_90.Segment[seg_cnt].X + m_segment_arr_90.Segment[seg_cnt].RightX;
		m_segment_arr_90.Segment[seg_cnt].P1_Y = m_segment_arr_90.Segment[seg_cnt].TipY - m_segment_arr_90.Segment[seg_cnt].Y + m_segment_arr_90.Segment[seg_cnt].RightY;
		m_segment_arr_90.Segment[seg_cnt].P2_X = m_segment_arr_90.Segment[seg_cnt].TipX - m_segment_arr_90.Segment[seg_cnt].X + m_segment_arr_90.Segment[seg_cnt].LeftX;
		m_segment_arr_90.Segment[seg_cnt].P2_Y = m_segment_arr_90.Segment[seg_cnt].TipY - m_segment_arr_90.Segment[seg_cnt].Y + m_segment_arr_90.Segment[seg_cnt].LeftY;
		m_segment_arr_90.Segment[seg_cnt].P3_X = m_segment_arr_90.Segment[seg_cnt].KnuckleX - m_segment_arr_90.Segment[seg_cnt].X + m_segment_arr_90.Segment[seg_cnt].LeftX;
		m_segment_arr_90.Segment[seg_cnt].P3_Y = m_segment_arr_90.Segment[seg_cnt].KnuckleY - m_segment_arr_90.Segment[seg_cnt].Y + m_segment_arr_90.Segment[seg_cnt].LeftY;
		m_segment_arr_90.Segment[seg_cnt].P4_X = m_segment_arr_90.Segment[seg_cnt].KnuckleX - m_segment_arr_90.Segment[seg_cnt].X + m_segment_arr_90.Segment[seg_cnt].RightX;
		m_segment_arr_90.Segment[seg_cnt].P4_Y = m_segment_arr_90.Segment[seg_cnt].KnuckleY - m_segment_arr_90.Segment[seg_cnt].Y + m_segment_arr_90.Segment[seg_cnt].RightY;
*/
		////////////////////////////////////////////////////////////////////////////////////
		// remove crease area in image
		// Line Equation : y = sin(BackDir) / cos(BackDir) * (x + BackX) + BackY
		RotAxisX = m_segment_arr_90.Segment[seg_cnt].KnuckleX;
		RotAxisY = m_segment_arr_90.Segment[seg_cnt].KnuckleY;
		
		rad = (m_segment_arr_90.Segment[seg_cnt].Ang-90) * 3.141592 / 180.0;
		cosangle = cos(rad);
		sinangle = sin(rad);

		for(i=tmpENLARGESIZE_ZOOM_H; i<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H; i++)
		{
			for(j=tmpENLARGESIZE_ZOOM_W; j<tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W; j++)
			{
				if(cosangle != 0)
				{
					if( i < (int)(sinangle / cosangle * (j - RotAxisX) + RotAxisY))
					{
						if(m_labeled_segment_enlarge_buffer[i*tmpZOOM_ENLAGE_W+j] == m_segment_arr_90.Segment[seg_cnt].Label)
							m_labeled_segment_enlarge_buffer[i*tmpZOOM_ENLAGE_W+j] = 0;
					}
				}
				else
				{
					if( i < RotAxisY)
					{
						if(m_labeled_segment_enlarge_buffer[i*tmpZOOM_ENLAGE_W+j] == m_segment_arr_90.Segment[seg_cnt].Label)
							m_labeled_segment_enlarge_buffer[i*tmpZOOM_ENLAGE_W+j] = 0;
					}
				}
			}
		}
		////////////////////////////////////////////////////////////////////////////////////	
	}

	delete [] MeanV;
	delete [] MeanV_Filtered;
	delete [] MeanH;
	delete [] MeanH_Filtered;
	delete [] Mean_Count;
}

void CIBAlgorithm::_Algo_SearchFingerTipPosition_Matcher()
{
	int seg_cnt, i, j;
	int RotAxisX, RotAxisY, RotLineX, RotLineY;
	int Count_up, Count_dn;
	double rad, cosangle, sinangle, rad_90, cosangle_90, sinangle_90;
	int sum_mean;
	int TopImage, BottomImage, LeftImage, RightImage;
	int ZeroCount, First_zero_idx;
	int MaxVal, MinVal, CutThres, CUT_THRES;
	int *MeanV = new int[ZOOM_ENLAGE_H*10];
	int *MeanV_Filtered = new int[ZOOM_ENLAGE_H*10];
	int *MeanH = new int[ZOOM_ENLAGE_W*10];
	int *MeanH_Filtered = new int[ZOOM_ENLAGE_W*10];
	int *Mean_Count = new int[ZOOM_ENLAGE_W*10];
	int x1, y1, x2, y2, x3, y3, x4, y4;
	int Width1, Width2, Height1, Height2;
	int MaxWidth, MaxHeight;
	int ReduceX, ReduceY;
	
	int tmpZOOM_ENLAGE_W = ZOOM_ENLAGE_W;
	int tmpZOOM_ENLAGE_H = ZOOM_ENLAGE_H;
	int tmpENLARGESIZE_ZOOM_W = ENLARGESIZE_ZOOM_W;
	int tmpENLARGESIZE_ZOOM_H = ENLARGESIZE_ZOOM_H;
	int tmpZOOM_W = ZOOM_W;
	int tmpZOOM_H = ZOOM_H;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		tmpZOOM_ENLAGE_W = ZOOM_ENLAGE_W_ROLL;
		tmpZOOM_ENLAGE_H = ZOOM_ENLAGE_H_ROLL;
		tmpENLARGESIZE_ZOOM_W = ENLARGESIZE_ZOOM_W_ROLL;
		tmpENLARGESIZE_ZOOM_H = ENLARGESIZE_ZOOM_H_ROLL;
		tmpZOOM_W = ZOOM_W_ROLL;
		tmpZOOM_H = ZOOM_H_ROLL;
	}

	for(seg_cnt=0; seg_cnt<m_segment_arr_matcher.SegmentCnt; seg_cnt++)
	{
		rad = -(m_segment_arr_matcher.Segment[seg_cnt].Ang-90) * 3.141592 / 180.0;
		cosangle = cos(rad);
		sinangle = sin(rad);

		rad_90 = -(m_segment_arr_matcher.Segment[seg_cnt].Ang-180) * 3.141592 / 180.0;
		cosangle_90 = cos(rad_90);
		sinangle_90 = sin(rad_90);

		memset(MeanV, 0, tmpZOOM_ENLAGE_H*10*sizeof(int));
		memset(MeanV_Filtered, 0, tmpZOOM_ENLAGE_H*10*sizeof(int));
		memset(MeanH, 0, tmpZOOM_ENLAGE_W*10*sizeof(int));
		memset(MeanH_Filtered, 0, tmpZOOM_ENLAGE_W*10*sizeof(int));
		memset(Mean_Count, 0, tmpZOOM_ENLAGE_W*10*sizeof(int));

		for(i=1; i<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H*2; i++)
		{
			// find FingerTip pos
			RotAxisX = (int)(m_segment_arr_matcher.Segment[seg_cnt].X + i * sinangle);
			RotAxisY = (int)(m_segment_arr_matcher.Segment[seg_cnt].Y + i * cosangle);

			Count_up=0;
			Count_dn=0;

			if(RotAxisX >= tmpENLARGESIZE_ZOOM_W && RotAxisY >= tmpENLARGESIZE_ZOOM_H && 
				RotAxisX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotAxisY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
			{
				// horizontal line
				for(j=1; j<tmpZOOM_ENLAGE_W/2; j++)
				{
					// right
					RotLineX = (int)(RotAxisX + j * sinangle_90);
					RotLineY = (int)(RotAxisY + j * cosangle_90);
					if(RotLineX >= tmpENLARGESIZE_ZOOM_W && RotLineY >= tmpENLARGESIZE_ZOOM_H && 
						RotLineX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotLineY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer[RotLineY*tmpZOOM_ENLAGE_W+RotLineX] == m_segment_arr_matcher.Segment[seg_cnt].Label )
						{
							MeanV[tmpZOOM_ENLAGE_H-1-i] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Count_up++;
							MeanH[tmpZOOM_ENLAGE_W+j] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Mean_Count[tmpZOOM_ENLAGE_W+j]++;
						}
					}

					// left
					RotLineX = (int)(RotAxisX - j * sinangle_90);
					RotLineY = (int)(RotAxisY - j * cosangle_90);
					if(RotLineX >= tmpENLARGESIZE_ZOOM_W && RotLineY >= tmpENLARGESIZE_ZOOM_H && 
						RotLineX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotLineY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer[RotLineY*tmpZOOM_ENLAGE_W+RotLineX] == m_segment_arr_matcher.Segment[seg_cnt].Label )
						{
							MeanV[tmpZOOM_ENLAGE_H-1-i] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Count_up++;
							MeanH[tmpZOOM_ENLAGE_W-1-j] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Mean_Count[tmpZOOM_ENLAGE_W-1-j]++;
						}
					}
				}
			}

			// down-axis position
			RotAxisX = (int)(m_segment_arr_matcher.Segment[seg_cnt].X - i * sinangle);
			RotAxisY = (int)(m_segment_arr_matcher.Segment[seg_cnt].Y - i * cosangle);
		
			if(RotAxisX >= tmpENLARGESIZE_ZOOM_W && RotAxisY >= tmpENLARGESIZE_ZOOM_H && 
				RotAxisX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotAxisY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
			{
				// horizontal line
				for(j=1; j<tmpZOOM_ENLAGE_W/2; j++)
				{
					// right
					RotLineX = (int)(RotAxisX + j * sinangle_90);
					RotLineY = (int)(RotAxisY + j * cosangle_90);
					if(RotLineX >= tmpENLARGESIZE_ZOOM_W && RotLineY >= tmpENLARGESIZE_ZOOM_H && 
						RotLineX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotLineY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer[RotLineY*tmpZOOM_ENLAGE_W+RotLineX] == m_segment_arr_matcher.Segment[seg_cnt].Label )
						{
							MeanV[i+tmpZOOM_ENLAGE_H] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Count_dn++;
							MeanH[tmpZOOM_ENLAGE_W+j] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Mean_Count[tmpZOOM_ENLAGE_W+j]++;
						}
					}

					// left
					RotLineX = (int)(RotAxisX - j * sinangle_90);
					RotLineY = (int)(RotAxisY - j * cosangle_90);
					if(RotLineX >= tmpENLARGESIZE_ZOOM_W && RotLineY >= tmpENLARGESIZE_ZOOM_H && 
						RotLineX < tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W && RotLineY < tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H)
					{
						if( m_labeled_segment_enlarge_buffer[RotLineY*tmpZOOM_ENLAGE_W+RotLineX] == m_segment_arr_matcher.Segment[seg_cnt].Label )
						{
							MeanV[i+tmpZOOM_ENLAGE_H] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Count_dn++;
							MeanH[tmpZOOM_ENLAGE_W-1-j] += EnlargeBuf[RotLineY*tmpZOOM_ENLAGE_W+RotLineX];
							Mean_Count[tmpZOOM_ENLAGE_W-1-j]++;
						}
					}
				}
			}

			if(Count_up > 0)	MeanV[tmpZOOM_ENLAGE_H-1-i] /= Count_up;
			if(Count_dn > 0)	MeanV[i+tmpZOOM_ENLAGE_H] /= Count_dn;
		}

		for(i=0; i<tmpZOOM_ENLAGE_W*2; i++)
		{
			if(Mean_Count[i] > 0)
				MeanH[i] /= Mean_Count[i];
		}

		MeanV[tmpZOOM_ENLAGE_H] = (MeanV[tmpZOOM_ENLAGE_H-1]+MeanV[tmpZOOM_ENLAGE_H+1])/2;
		MeanH[tmpZOOM_ENLAGE_W] = (MeanH[tmpZOOM_ENLAGE_W-1]+MeanH[tmpZOOM_ENLAGE_W+1])/2;

		// for horizontal
		MaxVal=-1;
		MinVal=1000000;
		for(i=2; i<tmpZOOM_ENLAGE_W*2-2; i++)
		{
			sum_mean = MeanH[i-1];
			sum_mean += MeanH[i  ];
			sum_mean += MeanH[i+1];

			sum_mean /= 3;
			MeanH_Filtered[i] = sum_mean;

			if(MaxVal < MeanH_Filtered[i])
				MaxVal = MeanH_Filtered[i];
			if(MinVal > MeanH_Filtered[i])
				MinVal = MeanH_Filtered[i];
		}

//		LeftImage=tmpZOOM_ENLAGE_W*2, RightImage=0;
		LeftImage= 2, RightImage=tmpZOOM_ENLAGE_W*2-2;
		for(i=2; i<tmpZOOM_ENLAGE_W*2-2; i++)
		{
			if(MeanH_Filtered[i] > 5)
			{
				LeftImage = i;
				break;
			}
		}
		for(i=tmpZOOM_ENLAGE_W*2-2; i>=2; i--)
		{
			if(MeanH_Filtered[i] > 5)
			{
				RightImage = i;
				break;
			}
		}

		LeftImage -= 1;
		RightImage += 1;

		// mean filter
		MaxVal=-1;
		MinVal=1000000;
		for(i=2; i<tmpZOOM_ENLAGE_H*2-2; i++)
		{
			sum_mean = MeanV[i-1];
			sum_mean += MeanV[i  ];
			sum_mean += MeanV[i+1];

			sum_mean /= 3;
			MeanV_Filtered[i] = sum_mean;

			if(MaxVal < MeanV_Filtered[i])
				MaxVal = MeanV_Filtered[i];
			if(MinVal > MeanV_Filtered[i])
				MinVal = MeanV_Filtered[i];
		}

		// calculate image height
//		TopImage=tmpZOOM_ENLAGE_H*2, BottomImage=0;
		TopImage=2, BottomImage=tmpZOOM_ENLAGE_H*2-2;
		for(i=2; i<tmpZOOM_ENLAGE_H*2-2; i++)
		{
			if(MeanV_Filtered[i] > 5)
			{
				TopImage = i;
				break;
			}
		}
		for(i=tmpZOOM_ENLAGE_H*2-2; i>=2; i--)
		{
			if(MeanV_Filtered[i] > 5)
			{
				BottomImage = i;
				break;
			}
		}

		int ClipFingerWidth = (RightImage-LeftImage);
		if(ClipFingerWidth < 50)
			ClipFingerWidth = 50;
		
		MaxVal=-1;
		MinVal=1000000;
		for(i=TopImage+(RightImage-LeftImage); i<BottomImage; i++)
		{
			sum_mean = MeanV[i-1];
			sum_mean += MeanV[i  ];
			sum_mean += MeanV[i+1];

			sum_mean /= 3;
			MeanV_Filtered[i] = sum_mean;

			if(MaxVal < MeanV_Filtered[i])
				MaxVal = MeanV_Filtered[i];
			if(MinVal > MeanV_Filtered[i])
				MinVal = MeanV_Filtered[i];
		}

		// find first under threshold
		// for vertical
		ZeroCount=0;
		First_zero_idx = BottomImage;

/*		FILE *fp = fopen("d:\\MeanV_Filtered.csv", "wt");
		fprintf(fp, "MaxVal,%d,MinVal,%d,(MaxVal-MinVal),%d\n", MaxVal, MinVal, MaxVal-MinVal);
		for(int lo=0; lo<ZOOM_ENLAGE_H*2; lo++)
		{
			fprintf(fp, "%d,%d\n", lo, MeanV_Filtered[lo]);
		}
		fclose(fp);
*/
		CutThres = (MaxVal-MinVal)/3;

		/*if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
			CutThres = (MaxVal-MinVal)/4;
		else if(m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_SINGLE_FINGER ||
				m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_TWO_FINGERS)
		{
			CutThres = (MaxVal-MinVal)/3;
			if(MaxVal < 50)
				CutThres = 5;
			else if(MaxVal < 75)
				CutThres = 10;
			else if(MaxVal < 100)
				CutThres = 15;
			else
				CutThres = 20;
		}
		else if(m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_THREE_FINGERS ||
				m_pPropertyInfo->ImageType == ENUM_IBSU_FLAT_FOUR_FINGERS)
		{
			CutThres = (MaxVal-MinVal)/3;
			if(MaxVal < 50)
				CutThres = 10;
			else if(MaxVal < 75)
				CutThres = 25;
			else if(MaxVal < 100)
				CutThres = 40;
			else
				CutThres = 55;
		}
		else
		{
			CutThres = (MaxVal-MinVal)/4;
			if(MaxVal < 50)
				CutThres = 20;
			else if(MaxVal < 75)
				CutThres = 30;
			else if(MaxVal < 100)
				CutThres = 40;
			else if(MaxVal < 125)
				CutThres = 50;
			else if(MaxVal < 150)
				CutThres = 60;
			else if(MaxVal < 175)
				CutThres = 70;
			else
				CutThres = 80;
		}*/

		if(TopImage+ClipFingerWidth*1.5 > BottomImage)
		{
			First_zero_idx = BottomImage;
		}
		else
		{
			for(i=(int)(TopImage+ClipFingerWidth*1.5f); i<BottomImage; i++)
			{
				if(i < TopImage+ClipFingerWidth*1.5+30)
					CUT_THRES = MinVal + CutThres*2;
				else if(i < TopImage+ClipFingerWidth*1.5+60)
					CUT_THRES = MinVal + CutThres*3;
				else if(i < TopImage+ClipFingerWidth*1.5+90)
					CUT_THRES = MinVal + CutThres*4;
				else
					CUT_THRES = MinVal + CutThres*5;

				if(MeanV_Filtered[i] < CUT_THRES && MeanV_Filtered[i] >= 0)
					ZeroCount++;
				else
					ZeroCount=0;
				
				if(ZeroCount >= 2)
				{
					First_zero_idx = i;
					break;
				}
			}
		}

		TopImage -= 1;
		First_zero_idx += 2;

		x1 = (LeftImage-tmpZOOM_ENLAGE_W);
		y1 = (tmpZOOM_ENLAGE_H-TopImage);

		x2 = (RightImage-tmpZOOM_ENLAGE_W);
		y2 = (tmpZOOM_ENLAGE_H-TopImage);

		x3 = (RightImage-tmpZOOM_ENLAGE_W);
		y3 = (tmpZOOM_ENLAGE_H-First_zero_idx);

		x4 = (LeftImage-tmpZOOM_ENLAGE_W);
		y4 = (tmpZOOM_ENLAGE_H-First_zero_idx);

		ReduceX=0;
		ReduceY=0;
		Width1 = abs(x2-x1)+1;
		Width2 = abs(x3-x4)+1;
		Height1 = abs(y1-y4)+1;
		Height2 = abs(y2-y3)+1;

		if(Width1 > Width2)		MaxWidth = Width1;
		else					MaxWidth = Width2;
		if(Height1 > Height2)	MaxHeight = Height1;
		else					MaxHeight = Height2;
		
		if(MaxWidth > tmpZOOM_W)
			ReduceX = (int)((double)(MaxWidth - tmpZOOM_W)/2.0+0.5);
		if(MaxHeight > tmpZOOM_H)
			ReduceY = (int)((double)(MaxHeight - tmpZOOM_H)/2.0+0.5);

		x1 = x1 + ReduceX;
		x4 = x4 + ReduceX;
		x2 = x2 - ReduceX;
		x3 = x3 - ReduceX;

		y1 = y1 - ReduceY;
		y4 = y4 + ReduceY;
		y2 = y2 - ReduceY;
		y3 = y3 + ReduceY;

		m_segment_arr_matcher.Segment[seg_cnt].TipX = (int)(m_segment_arr_matcher.Segment[seg_cnt].X + y1 * sinangle);
		m_segment_arr_matcher.Segment[seg_cnt].TipY = (int)(m_segment_arr_matcher.Segment[seg_cnt].Y + y1 * cosangle);
		m_segment_arr_matcher.Segment[seg_cnt].KnuckleX = (int)(m_segment_arr_matcher.Segment[seg_cnt].X + y3 * sinangle);
		m_segment_arr_matcher.Segment[seg_cnt].KnuckleY = (int)(m_segment_arr_matcher.Segment[seg_cnt].Y + y3 * cosangle);
		m_segment_arr_matcher.Segment[seg_cnt].LeftX = (int)(m_segment_arr_matcher.Segment[seg_cnt].X + x1 * sinangle_90);
		m_segment_arr_matcher.Segment[seg_cnt].LeftY = (int)(m_segment_arr_matcher.Segment[seg_cnt].Y + x1 * cosangle_90);
		m_segment_arr_matcher.Segment[seg_cnt].RightX = (int)(m_segment_arr_matcher.Segment[seg_cnt].X + x2 * sinangle_90);
		m_segment_arr_matcher.Segment[seg_cnt].RightY = (int)(m_segment_arr_matcher.Segment[seg_cnt].Y + x2 * cosangle_90);
		m_segment_arr_matcher.Segment[seg_cnt].CenterX = (m_segment_arr_matcher.Segment[seg_cnt].KnuckleX + m_segment_arr_matcher.Segment[seg_cnt].TipX) / 2;
		m_segment_arr_matcher.Segment[seg_cnt].CenterY = (m_segment_arr_matcher.Segment[seg_cnt].KnuckleY + m_segment_arr_matcher.Segment[seg_cnt].TipY) / 2;

		m_segment_arr_matcher.Segment[seg_cnt].P1_X = (int)(x2*cosangle+y1*sinangle + m_segment_arr_matcher.Segment[seg_cnt].X);
		m_segment_arr_matcher.Segment[seg_cnt].P1_Y = (int)(-x2*sinangle+y1*cosangle + m_segment_arr_matcher.Segment[seg_cnt].Y);
		m_segment_arr_matcher.Segment[seg_cnt].P2_X = (int)(x1*cosangle+y1*sinangle + m_segment_arr_matcher.Segment[seg_cnt].X);
		m_segment_arr_matcher.Segment[seg_cnt].P2_Y = (int)(-x1*sinangle+y1*cosangle + m_segment_arr_matcher.Segment[seg_cnt].Y);
		m_segment_arr_matcher.Segment[seg_cnt].P3_X = (int)(x1*cosangle+y3*sinangle + m_segment_arr_matcher.Segment[seg_cnt].X);
		m_segment_arr_matcher.Segment[seg_cnt].P3_Y = (int)(-x1*sinangle+y3*cosangle + m_segment_arr_matcher.Segment[seg_cnt].Y);
		m_segment_arr_matcher.Segment[seg_cnt].P4_X = (int)(x2*cosangle+y3*sinangle + m_segment_arr_matcher.Segment[seg_cnt].X);
		m_segment_arr_matcher.Segment[seg_cnt].P4_Y = (int)(-x2*sinangle+y3*cosangle + m_segment_arr_matcher.Segment[seg_cnt].Y);

/*		m_segment_arr_matcher.Segment[seg_cnt].P1_X = m_segment_arr_matcher.Segment[seg_cnt].TipX - m_segment_arr_matcher.Segment[seg_cnt].X + m_segment_arr_matcher.Segment[seg_cnt].RightX;
		m_segment_arr_matcher.Segment[seg_cnt].P1_Y = m_segment_arr_matcher.Segment[seg_cnt].TipY - m_segment_arr_matcher.Segment[seg_cnt].Y + m_segment_arr_matcher.Segment[seg_cnt].RightY;
		m_segment_arr_matcher.Segment[seg_cnt].P2_X = m_segment_arr_matcher.Segment[seg_cnt].TipX - m_segment_arr_matcher.Segment[seg_cnt].X + m_segment_arr_matcher.Segment[seg_cnt].LeftX;
		m_segment_arr_matcher.Segment[seg_cnt].P2_Y = m_segment_arr_matcher.Segment[seg_cnt].TipY - m_segment_arr_matcher.Segment[seg_cnt].Y + m_segment_arr_matcher.Segment[seg_cnt].LeftY;
		m_segment_arr_matcher.Segment[seg_cnt].P3_X = m_segment_arr_matcher.Segment[seg_cnt].KnuckleX - m_segment_arr_matcher.Segment[seg_cnt].X + m_segment_arr_matcher.Segment[seg_cnt].LeftX;
		m_segment_arr_matcher.Segment[seg_cnt].P3_Y = m_segment_arr_matcher.Segment[seg_cnt].KnuckleY - m_segment_arr_matcher.Segment[seg_cnt].Y + m_segment_arr_matcher.Segment[seg_cnt].LeftY;
		m_segment_arr_matcher.Segment[seg_cnt].P4_X = m_segment_arr_matcher.Segment[seg_cnt].KnuckleX - m_segment_arr_matcher.Segment[seg_cnt].X + m_segment_arr_matcher.Segment[seg_cnt].RightX;
		m_segment_arr_matcher.Segment[seg_cnt].P4_Y = m_segment_arr_matcher.Segment[seg_cnt].KnuckleY - m_segment_arr_matcher.Segment[seg_cnt].Y + m_segment_arr_matcher.Segment[seg_cnt].RightY;
*/
		////////////////////////////////////////////////////////////////////////////////////
		// remove crease area in image
		// Line Equation : y = sin(BackDir) / cos(BackDir) * (x + BackX) + BackY
		RotAxisX = m_segment_arr_matcher.Segment[seg_cnt].KnuckleX;
		RotAxisY = m_segment_arr_matcher.Segment[seg_cnt].KnuckleY;
		
		rad = (m_segment_arr_matcher.Segment[seg_cnt].Ang-90) * 3.141592 / 180.0;
		cosangle = cos(rad);
		sinangle = sin(rad);

		for(i=tmpENLARGESIZE_ZOOM_H; i<tmpZOOM_ENLAGE_H-tmpENLARGESIZE_ZOOM_H; i++)
		{
			for(j=tmpENLARGESIZE_ZOOM_W; j<tmpZOOM_ENLAGE_W-tmpENLARGESIZE_ZOOM_W; j++)
			{
				if(cosangle != 0)
				{
					if( i < (int)(sinangle / cosangle * (j - RotAxisX) + RotAxisY))
					{
						if(m_labeled_segment_enlarge_buffer[i*tmpZOOM_ENLAGE_W+j] == m_segment_arr_matcher.Segment[seg_cnt].Label)
							m_labeled_segment_enlarge_buffer[i*tmpZOOM_ENLAGE_W+j] = 0;
					}
				}
				else
				{
					if( i < RotAxisY)
					{
						if(m_labeled_segment_enlarge_buffer[i*tmpZOOM_ENLAGE_W+j] == m_segment_arr_matcher.Segment[seg_cnt].Label)
							m_labeled_segment_enlarge_buffer[i*tmpZOOM_ENLAGE_W+j] = 0;
					}
				}
			}
		}
		////////////////////////////////////////////////////////////////////////////////////	
	}

	delete [] MeanV;
	delete [] MeanV_Filtered;
	delete [] MeanH;
	delete [] MeanH_Filtered;
	delete [] Mean_Count;
}

void CIBAlgorithm::_Algo_HistogramNormalizeForZoomEnlarge(unsigned char *ImageBuf)
{
const double D_CUT_MINTRES = 0.001;
const double D_CUT_MAXTRES = 0.001;

	int y,x,gray=0;
	int histogram[256];
	int LUT[256];
	int CUTMIN_PIXELNUM	= (int)(D_CUT_MINTRES*(ZOOM_H*ZOOM_W));
	int CUTMAX_PIXELNUM	= (int)(D_CUT_MAXTRES*(ZOOM_H*ZOOM_W));
//	int count=1;
	
	memset(histogram,0,sizeof(histogram));
	memset(LUT,0,256*sizeof(int));
	for(y=ENLARGESIZE_ZOOM_H; y<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; y++)
	{
		for(x=ENLARGESIZE_ZOOM_W; x<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; x++)
		{
			gray=ImageBuf[y*ZOOM_ENLAGE_W+x];
			histogram[gray]++;
		}
	}
	
	int minsum=0,maxsum=0;
	int MIN_TRES=0,MAX_TRES=255;
	for(y=0; y<256; y++)
	{
		if(minsum>CUTMIN_PIXELNUM)
		{
			MIN_TRES=y;
			break;
		}
		else if(histogram[y]>0)
		{
			minsum+=histogram[y];
		}
		LUT[y]=0;
	}
	for(y=255; y>=MIN_TRES; y--)
	{
		if(maxsum>CUTMAX_PIXELNUM)
		{
			MAX_TRES=y;
			break;
		}
		else if(histogram[y]>0)
		{
			maxsum+=histogram[y];
		}
		LUT[y]=255;
	}
	
	int sum_of_histogram=0;
	for(y=MIN_TRES; y<=MAX_TRES; y++)
	{
		sum_of_histogram += histogram[y];
	}

	if(sum_of_histogram==0 || MAX_TRES==MIN_TRES)
		return;
	
	int cur_sum_of_histogram=0;
	for(y=MIN_TRES; y<=MAX_TRES; y++)
	{
		cur_sum_of_histogram += histogram[y];

		if(cur_sum_of_histogram==0)
			LUT[y] = 0;
		else
		{
			LUT[y] = (y-MIN_TRES) * 255 * sum_of_histogram / cur_sum_of_histogram / (MAX_TRES-MIN_TRES);
			if(LUT[y] > 255)
				LUT[y] = 255;
		}
	}

	for(y=ENLARGESIZE_ZOOM_H; y<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; y++)
	{
		for(x=ENLARGESIZE_ZOOM_W; x<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; x++)
		{
			ImageBuf[y*ZOOM_ENLAGE_W+x] = LUT[ImageBuf[y*ZOOM_ENLAGE_W+x]];
		}
	}
}
/*
void CIBAlgorithm::_Algo_SplitImage(unsigned char *InImg, unsigned char *SplitImg, int *SplitImgCount)
{
	int i, x, y;
	int cx, cy, label, sx, sy, seg_x, seg_y;

	memset(SplitImg, 255, imgWidth*imgHeight);

	for(i=0; i<m_segment_arr.SegmentCnt; i++)
	{
		cx = (m_segment_arr.Segment[i].CenterX-ENLARGESIZE_ZOOM_W) * imgWidth / ZOOM_W;
		cy = (m_segment_arr.Segment[i].CenterY-ENLARGESIZE_ZOOM_H) * imgHeight / ZOOM_H;
		label = m_segment_arr.Segment[i].Label;

		if(i < 2)
			sy = 0;
		else
			sy = imgHeight/2;

		for(y=cy-imgHeight/4; y<cy+imgHeight/4; y++, sy++)
		{
			if(y < 0 || y >= imgHeight )
				continue;

			if(i % 2 == 0)
				sx = 0;
			else
				sx = imgWidth/2;
	
			seg_y = y * ZOOM_H / imgHeight + ENLARGESIZE_ZOOM_H;

			for(x=cx-imgWidth/4; x<cx+imgWidth/4; x++, sx++)
			{
				if(x < 0 || x >= imgWidth )
					continue;

				seg_x = x * ZOOM_W / imgWidth + ENLARGESIZE_ZOOM_W;

				if(m_labeled_segment_enlarge_buffer[seg_y*ZOOM_ENLAGE_W+seg_x] == label)
					SplitImg[sy*imgWidth+sx] = InImg[y*imgWidth+x];
			}
		}
	}

	*SplitImgCount = m_segment_arr.SegmentCnt;
}*/

void CIBAlgorithm::_Algo_ClipFinger(BYTE *InputImg, BYTE *OutputImage, int *OutputWidth, int *OutputHeight, int *OutputSize, 
									BYTE *LabelBuf, NEW_SEGMENT *segment)
{
	// rotate img
	int i, j, s, t, buf1, buf2;
	double x, y;
	int p, q;
	int xs, ys;
	double r;
	double cs, sn;
	int data, count;
	int imgWidth, imgHeight, imgSize;
//	int x1, y1, x2, y2, x3, y3, x4, y4;
	int xx1, yy1, xx2, yy2, xx3, yy3, xx4, yy4;
	int cx, cy;
	unsigned char *TmpLabelBuf = (unsigned char *)m_MagBuffer_short;

	int tmpIMG_W = IMG_W;
	int tmpIMG_H = IMG_H;
	int tmpIMG_SIZE = IMG_SIZE;
	int tmpZOOM_W = ZOOM_W;
	int tmpZOOM_H = ZOOM_H;
	int tmpZOOM_ENLAGE_W = ZOOM_ENLAGE_W;
//	int tmpZOOM_ENLAGE_H = ZOOM_ENLAGE_H;
	int tmpENLARGESIZE_ZOOM_W = ENLARGESIZE_ZOOM_W;
	int tmpENLARGESIZE_ZOOM_H = ENLARGESIZE_ZOOM_H;
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		tmpIMG_W = IMG_W_ROLL;
		tmpIMG_H = IMG_H_ROLL;
		tmpIMG_SIZE = IMG_SIZE_ROLL;
		tmpZOOM_W = ZOOM_W_ROLL;
		tmpZOOM_H = ZOOM_H_ROLL;
		tmpZOOM_ENLAGE_W = ZOOM_ENLAGE_W_ROLL;
//		tmpZOOM_ENLAGE_H = ZOOM_ENLAGE_H_ROLL;
		tmpENLARGESIZE_ZOOM_W = ENLARGESIZE_ZOOM_W_ROLL;
		tmpENLARGESIZE_ZOOM_H = ENLARGESIZE_ZOOM_H_ROLL;
	}

	memset(TmpLabelBuf, 0, tmpIMG_W*tmpIMG_H);
	*OutputWidth = 0;
	*OutputHeight = 0;
	*OutputSize = 0;

	for(i=0; i<tmpIMG_H; i++)
	{
		yy1 = (i * tmpZOOM_H / tmpIMG_H + tmpENLARGESIZE_ZOOM_H) * tmpZOOM_ENLAGE_W;

		for(j=0; j<tmpIMG_W; j++)
		{
			xx1 = j * tmpZOOM_W / tmpIMG_W + tmpENLARGESIZE_ZOOM_W;

			TmpLabelBuf[i*tmpIMG_W+j] = LabelBuf[yy1+xx1];
		}
	}

	xx1 = (segment->P1_X-tmpENLARGESIZE_ZOOM_W) * tmpIMG_W / tmpZOOM_W;
	yy1 = (segment->P1_Y-tmpENLARGESIZE_ZOOM_H) * tmpIMG_H / tmpZOOM_H;

	xx2 = (segment->P2_X-tmpENLARGESIZE_ZOOM_W) * tmpIMG_W / tmpZOOM_W;
	yy2 = (segment->P2_Y-tmpENLARGESIZE_ZOOM_H) * tmpIMG_H / tmpZOOM_H;

	xx3 = (segment->P3_X-tmpENLARGESIZE_ZOOM_W) * tmpIMG_W / tmpZOOM_W;
	yy3 = (segment->P3_Y-tmpENLARGESIZE_ZOOM_H) * tmpIMG_H / tmpZOOM_H;

	xx4 = (segment->P4_X-tmpENLARGESIZE_ZOOM_W) * tmpIMG_W / tmpZOOM_W;
	yy4 = (segment->P4_Y-tmpENLARGESIZE_ZOOM_H) * tmpIMG_H / tmpZOOM_H;

	cx = (xx1+xx2+xx3+xx4)/4;
	cy = (yy1+yy2+yy3+yy4)/4;
	
	imgWidth = ((int)sqrt((double)(xx1-xx2)*(xx1-xx2)+(double)(yy1-yy2)*(yy1-yy2))+(int)sqrt((double)(xx3-xx4)*(xx3-xx4)+(double)(yy3-yy4)*(yy3-yy4)))/2;
	imgHeight = ((int)sqrt((double)(xx1-xx4)*(xx1-xx4)+(double)(yy1-yy4)*(yy1-yy4))+(int)sqrt((double)(xx3-xx2)*(xx3-xx2)+(double)(yy3-yy2)*(yy3-yy2)))/2;

	if(imgWidth%4 != 0)
		imgWidth = imgWidth + (4-imgWidth%4);
	if(imgHeight%4 != 0)
		imgHeight = imgHeight + (4-imgHeight%4);

	if(imgWidth >= tmpIMG_W)
		imgWidth = tmpIMG_W;
	if(imgHeight >= tmpIMG_H)
		imgHeight = tmpIMG_H;

	imgSize = imgWidth*imgHeight;

	memset(OutputImage, 0, tmpIMG_SIZE);

	xs = imgWidth/2;
	ys = imgHeight/2;

	r = (segment->Ang-90) * 3.141592 / 180.0;
	cs = cos(r);
	sn = sin(r);

	for (i=-ys; i<ys; i++) 
	{
		for (j=-xs; j<xs; j++)
		{
			x = j*cs - i*sn + cx;
			y = j*sn + i*cs + cy;
			
			if (y>0) buf1 = (int)y;
			else     buf1 = (int)(y - 1);
			if (x>0) buf2 = (int)x;
			else     buf2 = (int)(x - 1);
			

			if ((buf1 < IMG_H-1) && (buf1 >= 0) && (buf2 < IMG_W-1) && (buf2 >= 0))
			{
				count=0;
				
				if(m_pPropertyInfo->nNumberOfObjects > 1)
				{
					for(s=-10; s<=10; s+=5)
					{
						if ( (buf1+s) > tmpIMG_H-1 || (buf1+s < 0) )
							continue;

						for(t=-10; t<=10; t+=5)
						{
							if ( (buf2+t) > tmpIMG_W-1 || (buf2+t < 0) )
								continue;

							if(TmpLabelBuf[(buf1+s)*tmpIMG_W+(buf2+t)] > 0 && TmpLabelBuf[(buf1+s)*tmpIMG_W+(buf2+t)] != segment->Label)
							{
								count=1;
								break;
							}
						}
					}
				}
				
				if(count == 0)
				{
					if(((buf1+1)*tmpIMG_W+(buf2+1)) > tmpIMG_SIZE)
					{
						data = 0x00;
					}
					else
					{
						q = (int)((y - buf1) * 128);
						p = (int)((x - buf2) * 128);

						data = ( (128-q) * ( (128-p)*InputImg[buf1*tmpIMG_W+buf2] + p*InputImg[buf1*tmpIMG_W+buf2+1] )
								+ q*( (128-p)*InputImg[(buf1+1)*tmpIMG_W+buf2] + p*InputImg[(buf1+1)*tmpIMG_W+buf2+1] ) ) >> 14 ;
					}
					
					OutputImage[(i+ys)*imgWidth+(j+xs)]=data;
				}
			}
		}
	}

	*OutputWidth = imgWidth;
	*OutputHeight = imgHeight;
	*OutputSize = imgSize;

	if(m_pPropertyInfo->ImageType != ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		BYTE *TmpOut = (BYTE *)m_MagBuffer_short;
		memcpy(TmpOut, OutputImage, imgWidth*imgHeight);
		_Algo_Sharpening((const BYTE*)TmpOut, imgWidth, imgHeight, OutputImage);
	}
}

int CIBAlgorithm::SmearDetection_Calibration(unsigned char *InImg)
{
	int i, j;
	int centerx, centery;
	int /*bright, merged_diff, */merged_diff_hor, merged_diff_ver, merged_cnt;
	int cut_bright = 0;
	int LEFT, RIGHT, TOP, BOTTOM;

	LEFT = m_cImgAnalysis.LEFT;
	RIGHT = m_cImgAnalysis.RIGHT;
	TOP = m_cImgAnalysis.TOP;
	BOTTOM = m_cImgAnalysis.BOTTOM;
	
	centerx = (LEFT+RIGHT)/2;
	centery = (TOP+BOTTOM)/2;

	int tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//	int tmpCIS_IMG_H = CIS_IMG_H_ROLL;

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK ||
		m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
	{
		cut_bright = m_SBDAlg->m_Roll_Minus_Val;
	}

	merged_diff_ver = 0;
	merged_diff_hor = 0;
	merged_cnt = 0;
	for(i=TOP; i<BOTTOM; i++)
	{
		for(j=centerx-5; j<centerx+5; j++)
		{
			if(InImg[i*tmpCIS_IMG_W+j] < cut_bright)
				continue;

			merged_diff_hor += abs(InImg[i*tmpCIS_IMG_W+(j-1)] - InImg[i*tmpCIS_IMG_W+(j+1)]);
			merged_diff_ver += abs(InImg[(i-1)*tmpCIS_IMG_W+j] - InImg[(i+1)*tmpCIS_IMG_W+j]);
			merged_cnt++;
		}
	}

	for(j=LEFT; j<RIGHT; j++)
	{
		for(i=centery-5; i<centery+5; i++)
		{
			if(InImg[i*tmpCIS_IMG_W+j] < cut_bright)
				continue;

			merged_diff_hor += abs(InImg[i*tmpCIS_IMG_W+(j-1)] - InImg[i*tmpCIS_IMG_W+(j+1)]); 
			merged_diff_ver += abs(InImg[(i-1)*tmpCIS_IMG_W+j] - InImg[(i+1)*tmpCIS_IMG_W+j]);
			merged_cnt++;
		}
	}

	if(merged_cnt > 0)
	{
		merged_diff_ver /= merged_cnt;
		merged_diff_hor /= merged_cnt;
	}

	if(merged_diff_hor > merged_diff_ver)
		merged_diff_ver = merged_diff_hor;

	return merged_diff_ver;
}

int CIBAlgorithm::SmearDetectionOfFinger(unsigned char *LeftImg, unsigned char *RightImg, int MergePosX)
{
	// calculation time : less 1 msec in pc.

	int i, j;
	int /*merged_diff_hor, */merged_diff_ver;
	int merged_cnt;
	int bright_r, bright_l, maxbright;

	bright_l = NewRoll_GetBright_Simple(LeftImg, MergePosX);
	bright_r = NewRoll_GetBright_Simple(RightImg, MergePosX);

	if(bright_r <= 0 || bright_l <= 0)
	{
		m_ignore_smear = 1;
		return 100;
	}

	maxbright = bright_l;
	if(maxbright < bright_r)
		maxbright = bright_r;
	
//	merged_diff_hor = 0;
	merged_diff_ver = 0;
	merged_cnt = 0;

	int startx = MergePosX-100;
	int endx = MergePosX+100;
	int starty = m_cImgAnalysis.TOP + (m_cImgAnalysis.BOTTOM-m_cImgAnalysis.TOP)/10;
	int endy = m_cImgAnalysis.BOTTOM - (m_cImgAnalysis.BOTTOM-m_cImgAnalysis.TOP)/10;

	if(abs(starty-endy) < 50)
	{
		m_ignore_smear = 1;
		return 100;
	}

	int tmpCIS_IMG_W = CIS_IMG_W_ROLL;
//	int tmpCIS_IMG_H = CIS_IMG_H_ROLL;
	int cut_bright = 0;

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
	{
		cut_bright = 40;//m_SBDAlg->m_Roll_Minus_Val;
	}
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
	{
		cut_bright = _TFT_NOISE_FOR_FIVE0_;
	}

	if(startx < 0) startx = 0;
	if(endx > tmpCIS_IMG_W) endx = tmpCIS_IMG_W;

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK ||
		m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
	{
		for(i=starty; i<endy; i+=2)
		{
			for(j=startx; j<endx; j+=2)
			{
				if(LeftImg[i*tmpCIS_IMG_W+j] < cut_bright || RightImg[i*tmpCIS_IMG_W+j] < cut_bright)
					continue;

				merged_diff_ver += abs(LeftImg[i*tmpCIS_IMG_W+j]*bright_r - RightImg[i*tmpCIS_IMG_W+j]*bright_l);
				merged_cnt++;
			}
		}
		if(merged_cnt > 0)
			merged_diff_ver /= merged_cnt*(bright_r+bright_l)/2;
	}
	else
	{
		for(i=starty; i<endy; i+=2)
		{
			for(j=startx; j<endx; j+=2)
			{
				merged_diff_ver += abs(LeftImg[i*tmpCIS_IMG_W+j]*bright_r - RightImg[i*tmpCIS_IMG_W+j]*bright_l);
				merged_cnt++;
			}
		}
	
		if(merged_cnt > 0)
			merged_diff_ver /= merged_cnt*(bright_r+bright_l)/2;
	}
//	TRACE("bright_l : %d, bright_r : %d\n", bright_l, bright_r);

	if(m_pUsbDevInfo->devType == DEVICE_TYPE_FIVE0)
		merged_diff_ver = (int)(merged_diff_ver * 0.6);
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_KOJAK)
		merged_diff_ver = (int)(merged_diff_ver * 0.8);
	else if(m_pUsbDevInfo->devType == DEVICE_TYPE_SHERLOCK)
	{
		// little bit blur for DP film
		if( m_pUsbDevInfo->devType == DEVICE_TYPE_SHERLOCK &&
			m_pUsbDevInfo->nFpgaVersion >= _FPGA_VER_FOR_SHERLOCK_DOUBLE_P_ )
		{
			merged_diff_ver = (int)(merged_diff_ver * 0.65);
		}
		else
		{
			merged_diff_ver = (int)(merged_diff_ver * 0.75);
		}
	}
	else 
		merged_diff_ver = (int)(merged_diff_ver * 0.75);

	return (int)merged_diff_ver;
}

void CIBAlgorithm::_Algo_UpdateBestFrame(BYTE *InImg, const FrameImgAnalysis imgAnalysis)
{
	int Score=0;
	int ScoreBright=0, ScoreArea=0;
	int BestBright = (__CAPTURE_MIN_BRIGHT__+__CAPTURE_MAX_BRIGHT__)/2;
	int BestArea = SINGLE_FLAT_AREA_TRES * 3;

	if( m_pUsbDevInfo->devType == DEVICE_TYPE_WATSON || 
		m_pUsbDevInfo->devType == DEVICE_TYPE_WATSON_MINI ||
		m_pUsbDevInfo->devType == DEVICE_TYPE_SHERLOCK || 
		m_pUsbDevInfo->devType == DEVICE_TYPE_SHERLOCK_ROIC ||
		m_pUsbDevInfo->devType == DEVICE_TYPE_HOLMES )
	{
		BestBright = __CAPTURE_MAX_BRIGHT__;
	}
	else if( m_pUsbDevInfo->devType == DEVICE_TYPE_COLUMBO )
	{
		BestBright = __CAPTURE_MAX_BRIGHT__+5;
	}
	else if( m_pUsbDevInfo->devType == DEVICE_TYPE_CURVE ||
			 m_pUsbDevInfo->devType == DEVICE_TYPE_CURVE_SETI )
	{
		BestBright = __CAPTURE_MAX_BRIGHT__+10;
	}

	ScoreBright = 100 - (abs(BestBright-imgAnalysis.mean) * 100 / BestBright);
	if(ScoreBright < 0)
		ScoreBright = 0;

	if(imgAnalysis.finger_count > 0)
	{
		if( m_pUsbDevInfo->devType == DEVICE_TYPE_CURVE ||
			 m_pUsbDevInfo->devType == DEVICE_TYPE_CURVE_SETI )
		{
			ScoreArea = (m_CaptureInfo.lessTHAN3 + m_CaptureInfo.lessTHAN4) * 100 / (35000*imgAnalysis.finger_count);
			if(ScoreArea > 100)
				ScoreArea = 100;
		}
		else
		{
			ScoreArea = imgAnalysis.foreground_count * 25 / (BestArea*imgAnalysis.finger_count);
			if(ScoreArea > 100)
				ScoreArea = 100;
		}
	}
	
	Score = ScoreBright*35 + ScoreArea*65;

	if(m_BestFrame.score == 0 || m_BestFrame.score < Score)
	{
		memcpy(m_BestFrame.Buffer, InImg, CIS_IMG_SIZE);
		m_BestFrame.score = Score;
		memcpy(&m_BestFrame.imgAnalysis, &imgAnalysis, sizeof(FrameImgAnalysis));
	}

#ifdef __G_DEBUG__
	TRACE(_T("BestFrameScore:%d(Area:%d,Bright:%d), Score:%d\n"), m_BestFrame.score, ScoreArea, ScoreBright, Score);
#endif
}

int CIBAlgorithm::_Algo_GetBackgroundNoiseAuto(unsigned char *InImg, unsigned char *SegmentImg)
{
	int i, j;
	int Bright = 0;
	int BrightCount = 0;
	int xx, yy;

	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		for(i=ENLARGESIZE_ZOOM_H_ROLL; i<ZOOM_ENLAGE_H_ROLL-ENLARGESIZE_ZOOM_H_ROLL; i++)
		{
			yy = (i - ENLARGESIZE_ZOOM_H_ROLL) * CIS_IMG_H_ROLL / ZOOM_H_ROLL;

			if(yy < m_cImgAnalysis.TOP || yy > m_cImgAnalysis.BOTTOM)
				continue;

			for(j=ENLARGESIZE_ZOOM_W_ROLL; j<ZOOM_ENLAGE_W_ROLL-ENLARGESIZE_ZOOM_W_ROLL; j++)
			{
				xx = (j - ENLARGESIZE_ZOOM_W_ROLL) * CIS_IMG_W_ROLL / ZOOM_W_ROLL;
				
				if(xx < m_cImgAnalysis.LEFT || xx > m_cImgAnalysis.RIGHT)
					continue;
		
				if(SegmentImg[i*ZOOM_ENLAGE_W_ROLL+j] == 0)
				{
					Bright += InImg[yy*CIS_IMG_W_ROLL+xx];
					BrightCount++;
				}
			}
		}

		if(BrightCount > 0)
			Bright /= BrightCount;
	}
	else
	{
		for(i=ENLARGESIZE_ZOOM_H; i<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; i++)
		{
			yy = (i - ENLARGESIZE_ZOOM_H) * CIS_IMG_H / ZOOM_H;

			if(yy < m_cImgAnalysis.TOP*2 || yy > m_cImgAnalysis.BOTTOM*2)
				continue;

			for(j=ENLARGESIZE_ZOOM_W; j<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; j++)
			{
				xx = (j - ENLARGESIZE_ZOOM_W) * CIS_IMG_W / ZOOM_W;
				
				if(xx < m_cImgAnalysis.LEFT*2 || xx > m_cImgAnalysis.RIGHT*2)
					continue;
		
				if(SegmentImg[i*ZOOM_ENLAGE_W+j] == 0)
				{
					Bright += InImg[yy*CIS_IMG_W+xx];
					BrightCount++;
				}
			}
		}

		if(BrightCount > 0)
			Bright /= BrightCount;
	}

	return Bright;
}

void CIBAlgorithm::_Algo_ConvertSegInfoToFinal(unsigned char *pNewSeg, unsigned char *pNewEnlarge)
{
	int i, j;
	int x, y, pos, pos_x, pos_y, xx, yy, value, sp, sq;
	int move_x_org = (CIS_IMG_W-CIS_IMG_W_ROLL)/2;
	int move_x_tgt = (IMG_W-IMG_W_ROLL)/2;
	
	if(m_pPropertyInfo->ImageType == ENUM_IBSU_ROLL_SINGLE_FINGER)
	{
		memset(pNewSeg, 0, ZOOM_ENLAGE_W_ROLL*ZOOM_ENLAGE_H_ROLL);
		memset(pNewEnlarge, 0, ZOOM_ENLAGE_W_ROLL*ZOOM_ENLAGE_H_ROLL);

		for(i=ENLARGESIZE_ZOOM_H_ROLL; i<ZOOM_ENLAGE_H_ROLL-ENLARGESIZE_ZOOM_H_ROLL; i++)
		{
			y = (int)((double)(i-ENLARGESIZE_ZOOM_H_ROLL) * IMG_H_ROLL / ZOOM_H_ROLL);		// 1500

			for(j=ENLARGESIZE_ZOOM_W_ROLL; j<ZOOM_ENLAGE_W_ROLL-ENLARGESIZE_ZOOM_W_ROLL; j++)
			{
				x = (int)((double)(j-ENLARGESIZE_ZOOM_W_ROLL) * IMG_W_ROLL / ZOOM_W_ROLL);		// 1600

				pos = (m_SBDAlg->arrPos_wb[y*IMG_W+x+move_x_tgt]>>10) & 0x3FFFFF;
				pos_x = pos % CIS_IMG_W;							// 2056
				pos_y = pos / CIS_IMG_W;							// 1928
				sp = ((m_SBDAlg->arrPos_wb[y*IMG_W+x+move_x_tgt]>>5)&0x1F);
				sq = (m_SBDAlg->arrPos_wb[y*IMG_W+x+move_x_tgt]&0x1F);

				xx = (int)((double)((pos_x-move_x_org) * ZOOM_W_ROLL / CIS_IMG_W_ROLL) + ENLARGESIZE_ZOOM_W_ROLL + 0.5);
				yy = (int)((double)(pos_y * ZOOM_H_ROLL / CIS_IMG_H_ROLL) + ENLARGESIZE_ZOOM_H_ROLL + 0.5);

				if(yy < 1 || yy >= ZOOM_ENLAGE_H_ROLL-1 || xx < 1 || xx >= ZOOM_ENLAGE_W_ROLL-1)
					continue;

				pos = yy*ZOOM_ENLAGE_W_ROLL+xx;

				value = ( sp*( sq*m_segment_enlarge_buffer[pos] + (32-sq)*m_segment_enlarge_buffer[pos+ZOOM_ENLAGE_W_ROLL] )
					+ (32-sp)*( sq*m_segment_enlarge_buffer[pos+1] + (32-sq)*m_segment_enlarge_buffer[pos+ZOOM_ENLAGE_W_ROLL+1] ) ) >> 10;

				if(value > 250)
					value = 255;
				else
					value = 0;

				pNewSeg[i*ZOOM_ENLAGE_W_ROLL+j] = value;

				value = ( sp*( sq*EnlargeBuf[pos] + (32-sq)*EnlargeBuf[pos+ZOOM_ENLAGE_W_ROLL] )
					+ (32-sp)*( sq*EnlargeBuf[pos+1] + (32-sq)*EnlargeBuf[pos+ZOOM_ENLAGE_W_ROLL+1] ) ) >> 10;

				pNewEnlarge[i*ZOOM_ENLAGE_W_ROLL+j] = value;
			}
		}
	}
	else
	{
		memset(pNewSeg, 0, ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);
		memset(pNewEnlarge, 0, ZOOM_ENLAGE_W*ZOOM_ENLAGE_H);

		for(i=ENLARGESIZE_ZOOM_H; i<ZOOM_ENLAGE_H-ENLARGESIZE_ZOOM_H; i++)
		{
			for(j=ENLARGESIZE_ZOOM_W; j<ZOOM_ENLAGE_W-ENLARGESIZE_ZOOM_W; j++)
			{
				x = (int)((double)(j-ENLARGESIZE_ZOOM_W) * IMG_W / ZOOM_W);		// 1600
				y = (int)((double)(i-ENLARGESIZE_ZOOM_H) * IMG_H / ZOOM_H);		// 1500

				pos = (m_SBDAlg->arrPos_wb[y*IMG_W+x]>>10) & 0x3FFFFF;
				pos_x = pos % CIS_IMG_W;							// 2056
				pos_y = pos / CIS_IMG_W;							// 1928
				sp = ((m_SBDAlg->arrPos_wb[y*IMG_W+x]>>5)&0x1F);
				sq = (m_SBDAlg->arrPos_wb[y*IMG_W+x]&0x1F);

				xx = (int)((double)(pos_x * ZOOM_W / CIS_IMG_W) + ENLARGESIZE_ZOOM_W + 0.5);
				yy = (int)((double)(pos_y * ZOOM_H / CIS_IMG_H) + ENLARGESIZE_ZOOM_H + 0.5);

				if(yy < 1 || yy >= ZOOM_ENLAGE_H-1 || xx < 1 || xx >= ZOOM_ENLAGE_W-1)
					continue;

				pos = yy*ZOOM_ENLAGE_W+xx;

	//			value = m_segment_enlarge_buffer[pos];

			value = ( sp*( sq*m_segment_enlarge_buffer[pos] + (32-sq)*m_segment_enlarge_buffer[pos+ZOOM_ENLAGE_W] )
				+ (32-sp)*( sq*m_segment_enlarge_buffer[pos+1] + (32-sq)*m_segment_enlarge_buffer[pos+ZOOM_ENLAGE_W+1] ) ) >> 10;

				if(value > 250)
					value = 255;
				else
					value = 0;

				pNewSeg[i*ZOOM_ENLAGE_W+j] = value;

	//			value = EnlargeBuf[pos];

				value = ( sp*( sq*EnlargeBuf[pos] + (32-sq)*EnlargeBuf[pos+ZOOM_ENLAGE_W] )
					+ (32-sp)*( sq*EnlargeBuf[pos+1] + (32-sq)*EnlargeBuf[pos+ZOOM_ENLAGE_W+1] ) ) >> 10;

				pNewEnlarge[i*ZOOM_ENLAGE_W+j] = value;
			}
		}
	}	
}

void CIBAlgorithm::_Algo_Erosion(unsigned char *pNewSeg, int width, int height)
{
	int i, j;
	unsigned char *TmpBuf = m_Inter_Img4;// unsigned char [width*height];

	memcpy(TmpBuf, pNewSeg, width*height);

	for(i=1; i<height-1; i++)
	{
		for(j=1; j<width-1; j++)
		{
			if( TmpBuf[(i-1)*width+j-1] == 0 ||
				TmpBuf[(i-1)*width+j] == 0 ||
				TmpBuf[(i-1)*width+j+1] == 0 ||
				TmpBuf[i*width+j-1] == 0 ||
				TmpBuf[i*width+j+1] == 0 ||
				TmpBuf[(i+1)*width+j-1] == 0 ||
				TmpBuf[(i+1)*width+j] == 0 ||
				TmpBuf[(i+1)*width+j+1] == 0)
				pNewSeg[i*width+j] = 0;
		}
	}

//	delete [] TmpBuf;
}

void CIBAlgorithm::_Algo_Dilation(unsigned char *pNewSeg, int width, int height)
{
	int i, j;
	unsigned char *TmpBuf = m_Inter_Img4;// unsigned char [width*height];

	memcpy(TmpBuf, pNewSeg, width*height);

	for(i=1; i<height-1; i++)
	{
		for(j=1; j<width-1; j++)
		{
			if( TmpBuf[(i-1)*width+j-1] == 255 ||
				TmpBuf[(i-1)*width+j] == 255 ||
				TmpBuf[(i-1)*width+j+1] == 255||
				TmpBuf[i*width+j-1] == 255 ||
				TmpBuf[i*width+j+1] == 255 ||
				TmpBuf[(i+1)*width+j-1] == 255 ||
				TmpBuf[(i+1)*width+j] == 255 ||
				TmpBuf[(i+1)*width+j+1] == 255)
				pNewSeg[i*width+j] = 255;
		}
	}

//	delete [] TmpBuf;
}

#if defined(__IBSCAN_SDK__)
void CIBAlgorithm::_Algo_Make_250DPI(unsigned char *InImg, unsigned char *OutImg)
{
	int i, j;
	int LEFT, RIGHT, TOP, BOTTOM;
	int HALF_CIS_IMG_W = CIS_IMG_W>>1;
	memset(OutImg, 0, CIS_IMG_SIZE);

	TOP = (m_cImgAnalysis.TOP>>1)<<1;
	BOTTOM = (m_cImgAnalysis.BOTTOM>>1)<<1;
	LEFT = (m_cImgAnalysis.LEFT>>1)<<1;
	RIGHT = (m_cImgAnalysis.RIGHT>>1)<<1;

	for( i=TOP; i<=BOTTOM; i+=2 )
	{
		for( j=LEFT; j<=RIGHT; j+=2 )
		{
			OutImg[(i>>1)*HALF_CIS_IMG_W+(j>>1)] = (InImg[i*CIS_IMG_W+j] + InImg[i*CIS_IMG_W+j+1] + InImg[(i+1)*CIS_IMG_W+j] + InImg[(i+1)*CIS_IMG_W+j+1])>>2;
		}
	}
}

void CIBAlgorithm::_Algo_Image_Gamma_250DPI(unsigned char *InImg, unsigned char *OutImg, int Contrast)
{
//	if( m_bCaptureThread_StopMessage ) return;

	int i, j;
	int value;//, index;
	int LEFT, RIGHT, TOP, BOTTOM, HALF_CIS_IMG_W = CIS_IMG_W>>1;
	unsigned char *GammaTable = (unsigned char *)&G_GammaTable[Contrast*256];

	memset(OutImg, 0, CIS_IMG_SIZE);

	TOP = m_cImgAnalysis.TOP >> 1;
	BOTTOM = m_cImgAnalysis.BOTTOM >> 1;
	LEFT = m_cImgAnalysis.LEFT >> 1;
	RIGHT = m_cImgAnalysis.RIGHT >> 1;

	for( i=TOP; i<=BOTTOM; i++ )
	{
		for( j=LEFT; j<=RIGHT; j++ )
		{
			value=InImg[i*HALF_CIS_IMG_W+j];
			if( value > 0 )
			{
				OutImg[i*HALF_CIS_IMG_W+j] = GammaTable[value];
			}
		}
	}
}
#endif
