/* *************************************************************************************************
 * IBAlgorithm.h
 *
 * DESCRIPTION:
 *     Main header file for CIBSMAlgorithm class for IBScanMatcher library
 *     http://www.integratedbiometrics.com
 *
 * NOTES:
 *     Copyright (c) Integrated Biometrics, 2013
 *
 * HISTORY:
 ************************************************************************************************ */

#pragma once

#include "../../include/IBScanUltimate.h"
#include "../../include/IBScanUltimateApi.h"
#include "../../include/IBScanUltimateApi_defs.h"
#include "../../include/IBScanUltimateApi_err.h"
#include "../CaptureLib/IBSU_Global_Def.h"
//#include "ISO_19794_define.h"

class CIBSMAlgorithm
{
public:
    /*******************************************************************************************
     * Constructors and destructors
     ******************************************************************************************/
    CIBSMAlgorithm();
    virtual ~CIBSMAlgorithm();

    /*******************************************************************************************
     * Interface functions
     ******************************************************************************************/
    /*
     * Functions in IBAlgorithm.cpp
     */
	//int IBSM_OpenMatcher(int *matcher_handle);
	//int IBSM_CloseMatcher(const int matcher_handle);
    void IBSM_SDKInitialize();
    int IBSM_GetSDKVersion(char *str_version);
    int IBSM_ExtractTemplate(const unsigned char *pBuf, unsigned char *pTemplate, const int imgWidth,
        const int imgHeight);
    int IBSM_GetProcessedImage(IBSM_ProcessedImageType processed_imagetype, IBSM_ImageData *out_image);
    int IBSM_MatchingTemplate(const unsigned char *pTemplate1, const unsigned char *pTemplate2,
        int *matchingscore);
    int IBSM_SetMatchingLevel(const int matching_level);
    int IBSM_GetMatchingLevel(int *matching_level);
/*    int IBSM_SingleEnrollment(const unsigned char *image1, const unsigned char *image2,
        const unsigned char *image3, const int imgWidth, const int imgHeight, unsigned char *pTemplate,
        int *selected_image_idx);
    int IBSM_MultiEnrollment(const unsigned char *image1, const unsigned char *image2,
        const unsigned char *image3, const unsigned char *image4, const unsigned char *image5,
        const unsigned char *image6, const int imgWidth, const int imgHeight, unsigned char *pTemplate1,
        unsigned char *pTemplate2, int *selected_image_idx);
    int IBSM_CompressImage_ByWSQ(const unsigned char *image, const int imgWidth, const int imgHeight,
        const int imgPixelDepth, const int imgPixelPerInch, const char *commentText,
        unsigned char **compressedData, unsigned int *compressedLength);
    int IBSM_DecompressImage_ByWSQ(const unsigned char *pCompressedImage, const int imgWidth,
        const int imgHeight, int compressedLength, unsigned char **expandedImage, int *outWidth,
        int *outHeight, int *outDepth, int *outPixelPerInch);
    int IBSM_CompressImage_ByJPG(const unsigned char *image, const int imgWidth, const int imgHeight,
        const int imgPixelDepth, const int imgPixelPerInch, const char *commentText,
        unsigned char **compressedData, unsigned int *compressedLength);
    int IBSM_DecompressImage_ByJPG(const unsigned char *pCompressedImage, const int imgWidth,
        const int imgHeight, int compressedLength, unsigned char **expandedImage, int *outWidth,
        int *outHeight, int *outDepth, int *outPixelPerInch);
    int IBSM_CompressImage_ByJPG2000(const unsigned char *image, const int imgWidth, const int imgHeight,
        const int imgPixelDepth, const int imgPixelPerInch, const char *commentText,
        unsigned char **compressedData, unsigned int *compressedLength);
    int IBSM_DecompressImage_ByJPG2000(const unsigned char *pCompressedImage, const int imgWidth,
        const int imgHeight, int compressedLength, unsigned char **expandedImage, int *outWidth,
        int *outHeight, int *outDepth, int *outPixelPerInch);
    int IBSM_CompressImage_ByPNG(const unsigned char *image, const int imgWidth, const int imgHeight,
        const int imgPixelDepth, const int imgPixelPerInch, const char *commentText,
        unsigned char **compressedData, unsigned int *compressedLength);
    int IBSM_DecompressImage_ByPNG(const unsigned char *pCompressedImage, const int imgWidth,
        const int imgHeight, int compressedLength, unsigned char **expandedImage, int *outWidth,
        int *outHeight, int *outDepth, int *outPixelPerInch);
    int IBSM_OpenImageData(LPCSTR filePath, IBSM_ImageData *inImage);
    int IBSM_SaveImageData(LPCSTR filePath, const IBSM_ImageData inImage);
    int IBSM_OpenTemplate(LPCSTR filePath, IBSM_Template *inTemplate);
    int IBSM_SaveTemplate(LPCSTR filePath, const IBSM_Template inTemplate);
    int IBSM_GetSingularInfo(IBSM_SINGULAR *IBSM_Singular); // This function is never used
    int IBSM_GetMatchingInfo(IBSM_MATCHING_INFO *IBSM_MatchingInfoDB,
        IBSM_MATCHING_INFO *IBSM_MatchingInfoIN);           // This function is never used
*/    /*
     * Functions in ISO.cpp
     */
/*    int IBSM_ConvertTemplate_ISOtoIBSM(const ISO_FMR in_template, IBSM_Template **out_template,
        int *out_template_cnt);
    int IBSM_ConvertImage_ISOtoIBSM(const ISO_FIR in_image, IBSM_ImageData **out_image,
        int *out_image_cnt);
    int IBSM_ConvertTemplate_IBSMtoISO(const IBSM_Template in_template, ISO_FMR *out_template);
    int IBSM_ConvertImage_IBSMtoISO(const IBSM_ImageData in_template, ISO_FIR *out_template);
    int IBSM_ConvertTemplate_IBISDKtoIBSM(const IBISDK_Template in_template,
        IBSM_Template *out_template);
    int IBSM_ConvertTemplate_IBSMtoIBISDK(const IBSM_Template in_template,
        IBISDK_Template *out_template);
    int IBSM_OpenFIR(LPCSTR filePath, ISO_FIR *iso_fir);
    int IBSM_SaveFIR(LPCSTR filePath, const ISO_FIR iso_fir);
    int IBSM_OpenFMR(LPCSTR filePath, ISO_FMR *iso_fmr);
    int IBSM_SaveFMR(LPCSTR filePath, const ISO_FMR iso_fmr);
	int IBSM_FreeMemory(void *memblock, IBSM_MemBlockType memblock_type);
	int IBSM_OpenRasterImage(LPCSTR filePath, IBSM_ImageData *out_image);

	void _ImageFlipVertically(unsigned char *pixels_buffer, const int width, const int height);
*/
    /*******************************************************************************************
     * Interface members
     ******************************************************************************************/
    int m_handle;
	
//	std::vector<AllocatedMemory*> m_pListAllocatedMemory;

	/*
     * Functions in IBAlgorithm_Match.cpp
     */
    int _ALGO_Matching(FeatureVector* p_feavector_db, FeatureVector* p_feavector_in);
    int _Algo_SetMatchingLevel(int matching_level);
    int _Algo_GetMatchingLevel();

private:
//	AllocatedMemory* FindMemblockInList(void *memblock, IBSM_MemBlockType memblock_type);
//	int DeleteMemblockInList(void *memblock, IBSM_MemBlockType memblock_type);
    /*******************************************************************************************
     * Internal functions
     ******************************************************************************************/
    /*
     * Functions in IBAlgorithm_ImgPro.cpp
     */
    void _ALGO_AlgorithmInit(void);
    int  _ALGO_ExtractFt(const unsigned char* input_image, unsigned char *output_template,
        int imgWidth, int imgHeight);
    void _ALGO_ImageEnlargement(const unsigned char* input_image, unsigned char *output_image,
        int imgWidth, int imgHeight);
    void _ALGO_HistogramStretch(unsigned char *pImage, int imgWidth, int imgHeight);
    void _ALGO_MedianFilter(unsigned char *pImage, int imgWidth, int imgHeight, int *histogram);
    void _ALGO_BackgroundDetection(unsigned char *pImage, unsigned char *pSegBuf, int imgWidth,
        int imgHeight);
    void _ALGO_BlockNormalization(unsigned char *pImage, unsigned char *pSegBuf, int imgWidth,
        int imgHeight);
    void _ALGO_OrientationCalculation(const unsigned char *pImage, unsigned char *pSegBuf,
        unsigned char *pDirBuf, unsigned char *pDirBuf2, unsigned char *pDirBuf3, int imgWidth,
        int imgHeight);
    int _ALGO_FrequencyCalculation(unsigned char *pImage, unsigned char *pDirBuf,
        unsigned char *pSegBuf, unsigned char *pFrequencyBuf, int imgWidth, int imgHeight);
    void _ALGO_GaborFiltration(const unsigned char *pImage, const unsigned char *pSegBuf,
        const unsigned char *pDirBuf, const unsigned char *pFrequencyBuf, int *pGabor,
        int imgWidth, int imgHeight);
    void _ALGO_Binarization(const int *pGabor, const unsigned char *pSegBuf,
        const unsigned char *pDirBuf, unsigned char *pOutImage, int imgWidth, int imgHeight);
    void _ALGO_InvertBinarization(const unsigned char *pImage, const unsigned char *pSegBuf,
        unsigned char *pOutImage, int imgWidth, int imgHeight);
    void _ALGO_Thinning(unsigned char *pImage, unsigned int *feature, int invert_flag,
        int imgWidth, int imgHeight, unsigned int *remove_array);
    void _ALGO_MinutiaeAngleCalculation(unsigned char *pImage, unsigned int *pFeature,
        int invert_flag, int imgWidth, int imgHeight);
    void _ALGO_BrokenLineRecover(unsigned char *pImage, unsigned char *pImageInv,
        unsigned int *pFeature, int invert_flag, int imgWidth, int imgHeight);
    void _ALGO_OutsideMinutiaeElimination(unsigned char *pImage, unsigned char *pSegBuf,
        unsigned int *pFeature, int invert_flag, int imgWidth, int imgHeight);
    void _ALGO_MinutiaeCombination(unsigned int *pFeature1, unsigned int *pFeature2,
        int imgWidth, int imgHeight);
    unsigned char _ALGO_FingerPattern(unsigned char *pQuaBuf, unsigned char *pDirBuf,
        unsigned char *pDirBuf2, int imgWidth, int imgHeight, unsigned int *feature,
        IBSM_SINGULAR *IBSM_Singular, unsigned char *pThreeBin, unsigned char *pLabeledBin,
        unsigned char *pSingularBin, unsigned char *pTracingBin);
    void _ALGO_Pre_FeaGen_RemaindBuffer(unsigned int *minutia);
    
    /*
     * Functions in IBAlgorithm_NBIS.cpp
     */
    int _ALGO_CompressWSQ(const unsigned char *pUncompressedImage, const int imgWidth,
        const int imgHeight, const int imgPixelDepth, const int imgPixelPerInch,
        const char *commentText, unsigned char **compressedData, unsigned int *compressedLength);
    int _ALGO_DecompressWSQ(const unsigned char *pCompressedImage, const int imgWidth,
        const int imgHeight, int compressedLength, unsigned char **decompressImage,
        int *outWidth, int *outHeight, int *outDepth, int *outPixelPerInch);
    /*
     * Functions in IBAlgorithm_Enroll.cpp
     */
    int _ALGO_SingleEnrollment(const unsigned char *image1, const unsigned char *image2,
        const unsigned char *image3, const int imgWidth, const int imgHeight,
        unsigned char *outtemplate);
    int _ALGO_MultiEnrollment(const unsigned char *pTemplate1, const unsigned char *pTemplate2,
        const unsigned char *pTemplate3, const unsigned char *image1, const unsigned char *image2,
        const unsigned char *image3, const int imgWidth, const int imgHeight,
        unsigned char *outtemplate1, unsigned char *outtemplate2);

    /*******************************************************************************************
     * Local functions
     ******************************************************************************************/
    /*
     * Functions in IBAlgorithm_ImgPro.cpp
     */
    void _GenerateGaborTable(void);
    void _GenerateBlockNormalizeTable(void);
    void _GenerateBinarizationTable(void);
    void _GenerateThinningTable(void);
    unsigned int _getFirstBitFrom(unsigned int value, unsigned int fromBit);
    int _GetAtan2(int y, int x);
    void _StackRecursiveFilling(unsigned char *pImage, int imgWidth, int imgHeight, int x, int y,
        unsigned char TargetColor, unsigned char DestColor, unsigned short *stack);
    int _NewCalculation_Pitch(int *PERIOD_SIGNAL, int* quality);
    void _NewCalculationPeriodSmoothing(unsigned char *pFrequencyBuf, int imgWidth, int imgHeight,
        unsigned char *pTemp);
    void _MakeThreeBinaryImage(unsigned char *pQuaBuf, unsigned char *pDirBuf, int imgEnQuadWidth,
        int imgEnQuadHeight, unsigned char *pThreeBin, unsigned char *pLabeledBin, int *label_0_cnt,
        int *label_170_cnt, int *label_255_cnt);
    int _LabelingUsingColor(unsigned char *pLabeledBin, int imgEnQuadWidth, int imgEnQuadHeight,
        int color);
    int _ExpandLabel(unsigned char *pLabeledBin, int imgEnQuadWidth, int imgEnQuadHeight,
        int x, int y, unsigned char get_color, unsigned char set_color);
    void _FindCoresAndDeltas(unsigned char *pSegBuf, unsigned char *pThreeBin,
        unsigned char *pSingularBin, int imgEnQuadWidth, int imgEnQuadHeight,
        IBSM_SINGULAR *IBSM_Singular);
    unsigned char _JudgeArch(unsigned char *pLabelBin, int imgEnQuadWidth, int imgEnQuadHeight,
        IBSM_SINGULAR *IBSM_Singular, int label_0_cnt, int label_1_cnt, int label_2_cnt);
    unsigned char _JudgeArchAnother(unsigned char *pThreeBin, unsigned char *pTracingBin,
        unsigned char *pDirBuf, int imgEnQuadWidth, int imgEnQuadHeight,
        IBSM_SINGULAR *IBSM_Singular, IB_RECT IB_Rect);
    int _FindArchCurve(unsigned char *pThreeBin, unsigned char *pTracingBin, int imgEnQuadWidth,
        int imgEnQuadHeight, int startx, int starty, IB_TRACING *pTracingInfo,
        unsigned char EdgeValue1, unsigned char EdgeValue2, unsigned char PaintValue);
    int _FindUpperCore_Virtual(unsigned char *pDirBuf, int imgEnQuadWidth, int imgEnQuadHeight,
        IBSM_SINGULAR *IBSM_Singular);
    unsigned char _JudgeStrangeWhorl(unsigned char *pDirBuf, int imgEnQuadWidth, int imgEnQuadHeight,
        IBSM_SINGULAR *IBSM_Singular);
    int _FindUpperCore(unsigned char *pDirBuf, int imgEnQuadWidth, int imgEnQuadHeight,
        IBSM_SINGULAR *IBSM_Singular, int Flag);
    void _RemoveCloseCoreDelta(IBSM_SINGULAR *IBSM_Singular);
    void _RemoveOutsideCoreDelta(IBSM_SINGULAR *IBSM_Singular, IB_RECT IB_Rect);
    void _FindDeltaCurve(unsigned char *pThreeBin, unsigned char *pTracingBin, unsigned char *pSegBuf,
        int imgEnQuadWidth, int imgEnQuadHeight, IBSM_SINGULAR *IBSM_Singular, unsigned char EdgeValue1,
        unsigned char EdgeValue2, unsigned char PaintValue, int DeltaFlag);
    void _RemoveFalseDelta(IBSM_SINGULAR *IBSM_Singular);
    void _FindSingularCurve(unsigned char *pSingularBin,unsigned char *pTracingBin,
        unsigned char *pThreeBin, unsigned char *pSegBuf, int imgEnQuadWidth, int imgEnQuadHeight,
        IBSM_SINGULAR *IBSM_Singular, int *TracingAngleMinMax, unsigned char EdgeValue1,
        unsigned char EdgeValue2, unsigned char PaintValue, int CoreFlag);
    void _RemoveFalseCore(unsigned char *pDirBuf, int imgEnQuadWidth, int imgEnQuadHeight,
        IBSM_SINGULAR *IBSM_Singular, IB_RECT IB_Rect);
    int _FindUpperCore_Min5_Weight(unsigned char *pDirBuf, int imgEnQuadWidth, int imgEnQuadHeight,
        IBSM_SINGULAR *IBSM_Singular);
    unsigned char _JudgeFingerIndexing(unsigned char *pLabeledBin, int imgEnQuadWidth,
        int imgEnQuadHeight, IBSM_SINGULAR *IBSM_Singular, IB_RECT IB_Rect, int *TracingAngleMinMax,
        int label_0_cnt, int label_1_cnt, int label_2_cnt, int ori_core, int ori_delta);
    unsigned char _JudgeWhorl(IBSM_SINGULAR *IBSM_Singular);
    unsigned char _JudgePerpectWhorl(IBSM_SINGULAR *IBSM_Singular, IB_RECT IB_Rect, int ori_core,
        int ori_delta);
    unsigned char _JudgeTracingPattern(IBSM_SINGULAR *IBSM_Singular);
    unsigned char _JudgeLongDistWhorl(IBSM_SINGULAR *IBSM_Singular);
    unsigned char _JudgeTentedArch(IBSM_SINGULAR *IBSM_Singular, int *TracingAngleMinMax);
    unsigned char _JudgeRightLoop(unsigned char *pLabeledBin, int imgEnQuadWidth, int imgEnQuadHeight,
        IBSM_SINGULAR *IB_Singular);
    unsigned char _JudgeShiftedWhorl(unsigned char *pLabeledBin, int imgEnQuadWidth, int imgEnQuadHeight,
        IBSM_SINGULAR *IB_Singular, unsigned char Flag, int label_0_cnt, int label_1_cnt, int label_2_cnt);
    unsigned char _JudgeCorePosIsOutSide(IBSM_SINGULAR *IBSM_Singular, IB_RECT IB_Rect, int type);
    unsigned char _JudgeBestLeftRight(IBSM_SINGULAR *IBSM_Singular, int ori_core, int ori_delta);
    unsigned char _Judge180degreeToTented(IBSM_SINGULAR *IBSM_Singular);
    unsigned char _JudgeLeftLoop(unsigned char *pLabeledBin, int imgEnQuadWidth, int imgEnQuadHeight,
        IBSM_SINGULAR *IB_Singular);
    unsigned char _JudgeWrongDeltaPos(IBSM_SINGULAR *IBSM_Singular, unsigned char Flag);
    /*
     * Functions in IBAlgorithm_Match.cpp
     */
    void _ALGO_MatchingInit();
    int _GetAtanValue(int y, int x);
    int _SearchCandidateMinutiae(FeatureVector* p_feavector_db, FeatureVector* p_feavector_in,
        CANDIDATE_Pair* p_Candidate_Pair);
    int _Neighbor_Matching_Calculation_New(int minutiae_num_db, int minutiae_num_in,
        FeatureVector* m_feavector_db,FeatureVector* m_feavector_in, int *MATNEIGBOR8_DB,
        int *MATNEIGBOR8_IN, int *MATNEIGBOR8_COUNT);
    int _Judge_TracingTree(int origin_db, int target_db, int origin_in, int target_in,
        FeatureVector* m_feavector_db, FeatureVector* m_feavector_in, int* Diff_angle, int* Diff_thetta);
    int _Judge_JointTree(int origin_db, int target_db, int origin_in, int target_in,
        FeatureVector* p_feavector_db,FeatureVector* p_feavector_in);
    int _DistanceCheck_ForCandidate(int minutiae_num, FeatureVector* feavector);
    int _Neighbor_Matching_CalculationFORCANDIDATE(int minutiae_num_db, int minutiae_num_in,
        FeatureVector* p_feavector_db, FeatureVector* p_feavector_in);
    int _FeaGen(unsigned int* feature, FeatureVector* p_feavector, int fea_ver);
    int _PenaltyCalc(FeatureVector* p_feavector_db, FeatureVector* p_feavector_in, int Total_NMS,
        char *STEP_MATCEDMINUTIAE_DB, char *STEP_MATCEDMINUTIAE_IN,
        unsigned short *STEP_MATCEDMINUTIAE_SCORE_MINUTIAE, unsigned char MatchedMinutiaCount);
    void _AlignFeature(FeatureVector *m_fea_db, FeatureVector *m_fea_in, FeatureVector *m_fea_out,
        char *MATCEDMINUTIAE_DB,    char *MATCEDMINUTIAE_IN, unsigned short *MATCEDMINUTIAE_SCORE_MINUTIAE,
        unsigned char MATCEDMINUTIAE_MATCHEDCOUNT);
    int _Area_Calculation(FeatureVector* m_feavector_db, FeatureVector* m_feavector_in,
        int* db_back_min_x, int* db_back_min_y, int* db_back_max_x, int* db_back_max_y,
        int* in_back_min_x, int* in_back_min_y, int* in_back_max_x, int* in_back_max_y,
        int* db_mat_min_x, int* db_mat_min_y, int* db_mat_max_x, int* db_mat_max_y,
        int* in_mat_min_x, int* in_mat_min_y, int* in_mat_max_x, int* in_mat_max_y,
        int* db_comm_min_x, int* db_comm_min_y, int* db_comm_max_x, int* db_comm_max_y,
        int* in_comm_min_x, int* in_comm_min_y, int* in_comm_max_x, int* in_comm_max_y,
        char *MATCEDMINUTIAE_DB, char *MATCEDMINUTIAE_IN, unsigned short *MATCEDMINUTIAE_SCORE_MINUTIAE,
        unsigned char MATCEDMINUTIAE_MATCHEDCOUNT );
    int _Calculate_Minutiae_NeighborDist_New(FeatureVector *m_feavector_db, FeatureVector *m_feavector_in,
        int db_comm_min_x, int db_comm_max_x, int db_comm_min_y, int db_comm_max_y, int db_mat_min_x,
        int db_mat_max_x, int db_mat_min_y, int db_mat_max_y, int in_comm_min_x, int in_comm_max_x,
        int in_comm_min_y, int in_comm_max_y, int in_mat_min_x, int in_mat_max_x, int in_mat_min_y,
        int in_mat_max_y, int* db_goodquality_nonmatchedminutiae_num_in_commarea,
        int* db_goodquality_nonmatchedminutiae_num_in_matarea,
        int* in_goodquality_nonmatchedminutiae_num_in_commarea,
        int* in_goodquality_nonmatchedminutiae_num_in_matarea,
        int* db_badquality_nonmatchedminutiae_num_in_commarea,
        int* db_badquality_nonmatchedminutiae_num_in_matarea,
        int* in_badquality_nonmatchedminutiae_num_in_commarea,
        int* in_badquality_nonmatchedminutiae_num_in_matarea,
        int* db_goodquality_matchedminutiae_num_in_commarea,
        int* db_goodquality_matchedminutiae_num_in_matarea,
        int* in_goodquality_matchedminutiae_num_in_commarea,
        int* in_goodquality_matchedminutiae_num_in_matarea,
        int* db_badquality_matchedminutiae_num_in_commarea,
        int* db_badquality_matchedminutiae_num_in_matarea,
        int* in_badquality_matchedminutiae_num_in_commarea,
        int* in_badquality_matchedminutiae_num_in_matarea,
        int* db_minutiae_totalnum_in_commarea, int* db_minutiae_totalnum_in_matarea,
        int* in_minutiae_totalnum_in_commarea, int* in_minutiae_totalnum_in_matarea,
        char *MATCEDMINUTIAE_DB, char *MATCEDMINUTIAE_IN,
        unsigned short *MATCEDMINUTIAE_SCORE_MINUTIAE,
        unsigned char MATCEDMINUTIAE_MATCHEDCOUNT,
        unsigned char *Minutiae_Quality_DB, unsigned char *Minutiae_Quality_IN);
    int _JudgeFingerPattern(const FeatureVector *ref_feature, const FeatureVector *inp_feature);
    /*
     * Functions in IBAlgorithm.cpp
     */
    // Bitmap functions
    int _OpenBitmapImage(const char *filename, unsigned char **pBuf, IB_BITMAPINFO *pInfo);
    int _SaveBitmapImage(const char *filename, const unsigned char *pBuf, const int imgWidth,
        const int imgHeight, const BOOL Invert);
    int _SaveBitmapImage_UserSize(const char *filename, const unsigned char *pBuf, const int imgWidth,
        const int imgHeight, const int TargetWidth, const int TargetHeight, const unsigned char magnifier,
        const BOOL Invert);
    // Support IBISDK template
    short IBSM_IsTemplate_317(const unsigned char *feature404);
    short IBSM_IsTemplate_320(const unsigned char *feature404);
    /*
     * Functions in IBAlgorithm_ISO.cpp
     */
    int Endian();
    WORD BigEndianWord(WORD convert);
    DWORD BigEndianDWord(DWORD convert);
    REPRESENTATIONHEADER Read_FIR_Representation(FILE* fp, ISO_FIR_ORG *iso_fir);
    int PreRead_FIR_Extended(FILE *fp, int &size);
    EXTENDEDDATA Read_FIR_Extended(FILE *fp, REPRESENTATIONHEADER rp, int &size,int ExtIndex);
    int Write_FIR_Representation(FILE* fp, ISO_FIR_ORG iso_fir, int RepIndex);
    void Write_FIR_Extended(FILE *fp, EXTENDEDDATA Ext);
    FINGERREPRESENTATION Read_FMR_Representation(FILE *fp, ISO_FMR_ORG *iso_fmr);
    int PreRead_FMR_Extended(FILE *fp, int &size);
    EXTENDEDDATA FMR_ReadExtended(FILE *fp, FINGERREPRESENTATION fr, int &size);

    /*******************************************************************************************
     * Local members
     ******************************************************************************************/

    // algorithm variables
    char                SDKVersion[128];
    double              cs[721], sn[721];
    short               Table_Gabor[D_GABOR_TABLE_SIZE];
    unsigned char       Table_BlockNormalize[256][256];
    signed int          g_flt_sin[QUANTIZED_DIRECTION];
    signed int          g_flt_cos[QUANTIZED_DIRECTION];
    signed int          g_flt_dir[QUANTIZED_DIRECTION];

    // Variable used in _ALGO_Thinning()
    unsigned char       m_RedirectArray[256*8];  // Initialized in _GenerateThinningTable()
//  unsigned char       g_thinning_flag[256];
//  unsigned char       g_thinning_live[256];

    // Tables initialized in IBSM_SDKIntiailize()
    unsigned char       m_Atan2Table[288][352];
    signed int          m_SingleSinTable[MAX_MATH_SIZE/2];
    signed int          m_DoubleSinTable[MAX_MATH_SIZE/2];
    signed int          m_DoubleCosTable[MAX_MATH_SIZE/2];
    signed int          m_SingleTanTable[MAX_MATH_SIZE/2];
    signed int          m_SingleCosTable[MAX_MATH_SIZE/2];
    signed char         m_xIncycleTable[MAX_ANGLE][POINTS_OF_CYCLE];  // Only initialized, never used
    signed char         m_yIncycleTable[MAX_ANGLE][POINTS_OF_CYCLE];  // Only initialized, never used
    signed char         m_xOutcycleTable[MAX_ANGLE][POINTS_OF_CYCLE]; // Only initialized, never used
    signed char         m_yOutcycleTable[MAX_ANGLE][POINTS_OF_CYCLE]; // Only initialized, never used

    // Variables used in various functions
    unsigned short      m_SqrtTable[MAX_IMAGE_H][MAX_IMAGE_W];
    unsigned char       m_AngleLimForMatching[MAX_DIAGONAL_LENGTH];
    unsigned char       m_RLimForMatching[MAX_DIAGONAL_LENGTH];
    unsigned short      m_RpRForMatching[MAX_DIAGONAL_LENGTH];
    unsigned short      m_RpAngleForMatching[MAX_DIAGONAL_LENGTH];
    unsigned char       m_DiffAngleForMatching[361*2];      // fixed

    // Variables used by _FeaGen()
    unsigned char       m_Distance[MAX_DIAGONAL_LENGTH][100];
    unsigned char       m_DistanceCount[MAX_DIAGONAL_LENGTH];  // Also used by IBSM_ConvertTemplate_IBSMtoIBISDK()

    // Variables used by _ALGO_Matching()
    char                m_StepMatchedMinutiaeDB[MAX_MINUTIAE][MAX_MINUTIAE];
    char                m_StepMatchedMinutiaeIN[MAX_MINUTIAE][MAX_MINUTIAE];
    unsigned char       m_StepMatchedMinutiaeMatchedCount[MAX_MINUTIAE];
    unsigned short      m_StepMatchedMinutiaeScoreMinutiae[MAX_MINUTIAE][MAX_MINUTIAE];
    unsigned short      m_StepMatchedMinutiaeScore[MAX_MINUTIAE];

    // Variables used by _ALGO_ExtractFt()
    unsigned char       *m_pEnlarge;
    unsigned char       *m_pSegBuf;
    unsigned char       *m_pDirBuf;
    unsigned char       *m_pDirBuf2;
    unsigned char       *m_pDirBuf3;
    unsigned char       *m_pFrequencyBuf;
    int                 *m_pGabor;
    unsigned char       *m_pBinary;
    unsigned char       *m_pBinaryInv;
    unsigned char       *m_pThreeBin;
    unsigned char       *m_pLabeledBin;
    unsigned char       *m_pSingularBin;
    unsigned char       *m_pTracingBin;
    unsigned int        *m_pFeature1;
    unsigned int        *m_pFeature2;
    unsigned char       *m_pGaborSave;  // Also returned by IBSM_GetProcessedImage()
    unsigned char       *m_pBinarySave; // Also returned by IBSM_GetProcessedImage()
    unsigned char       *m_pThinSave;   // Also returned by IBSM_GetProcessedImage()
    unsigned int        *m_pRemoveArray;
    IBSM_SINGULAR        m_Singular;    // Also returned by IBSM_GetSingularInfo()

    // Variable used by _ALGO_BackgroundDetection() and _ExpandLabel()
    unsigned short       *m_pStackBuf;

    // Variables set in IBSM_ExtractTemplate(), returned by IBSM_GetProcessedImage()
    int                  m_SaveSizeX;
    int                  m_SaveSizeY;

    // Variables used by _ALGO_OrientationCalculation()
    unsigned char       *m_pTempDirBuf; // Also used by _ALGO_FrequencyCalculation()
    int                 *m_X_gradient;
    int                 *m_Y_gradient;

    // Variables used by _ALGO_BackgroundDetection()
    unsigned char       *m_pHalfImage;
    unsigned int        *m_pGradientBuffer;
    unsigned char       *m_pTempQuadBuffer;

    // Variable used by _ALGO_BlockNormalization()
    unsigned char       *m_pTempImage;

    // Algorithm state variables preserved between calls
    int                 m_MatchingLevel;
    int                 m_MatchingThreshold;

    // Set in constructor or _JudgeFingerPattern() (never called), used in _SearchCandidateMinutiae()
    int                 m_MatchCoreLevel;

    // Set in constructor, used in _JudgeFingerPattern() (never called)
    int                 m_IndexingLevel;

#ifdef SAVE_MATCHING_INFO
    IBSM_MATCHING_INFO    m_MatchingInfoDB;
    IBSM_MATCHING_INFO  m_MatchingInfoIN;
#endif
};
