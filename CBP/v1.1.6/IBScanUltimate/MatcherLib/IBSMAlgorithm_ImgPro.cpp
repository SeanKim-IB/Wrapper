/* *************************************************************************************************
 * IBAlgorithm_ImgPro.cpp
 *
 * DESCRIPTION:
 *     Image processing for IBScanMatcher library
 *     http://www.integratedbiometrics.com
 *
 * NOTES:
 *     Copyright (c) Integrated Biometrics, 2013
 *
 * HISTORY:
 ************************************************************************************************ */
#include "stdafx.h"
#include "IBSMAlgorithm.h"
#include <math.h>
#ifdef _WINDOWS
#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#endif

void CIBSMAlgorithm::_ALGO_AlgorithmInit(void)
{
    _GenerateBlockNormalizeTable();
    _GenerateGaborTable();
    _GenerateBinarizationTable();
    _GenerateThinningTable();

    for(int i=0; i<361; i++)
    {
        cs[i] = cos(i * 3.14159265 / 180.0);
        cs[360+i] = cs[i];
        sn[i] = sin(i * 3.14159265 / 180.0);
        sn[360+i] = sn[i];
    }
}

void CIBSMAlgorithm::_GenerateGaborTable(void)
{
    int row,cols,angle=0,pitch;
//    double radian;
    double x_prime,x_prime2,y_prime,y_prime2,grad_freq,value;

    for(pitch=D_FREQUENCY_MIN; pitch<=D_FREQUENCY_MAX; pitch++)
    {
        for(angle=0; angle<256; angle+=4)
        {
            for(row=-D_GABOR_HALF_WINDOW; row<=D_GABOR_HALF_WINDOW; row++)
            {
                for(cols=-D_GABOR_HALF_WINDOW; cols<=D_GABOR_HALF_WINDOW; cols++)
                {
                    grad_freq = ((float)1)/pitch;
                    grad_freq = (float)(2.0*3.14159265358979*grad_freq);

//                    radian = pi*angle/180.;
                    x_prime =  (cols*m_SingleCosTable[angle]+row*m_SingleSinTable[angle])/1024.;
                    y_prime = (-cols*m_SingleSinTable[angle]+row*m_SingleCosTable[angle])/1024.;
                    x_prime2 = x_prime*x_prime;
                    y_prime2 = y_prime*y_prime;
                    value=exp(-0.5*( (x_prime2+y_prime2)/D_GABOR_SQURE_DELTA)) * cos(grad_freq*x_prime);
//                  D_GABOR_TABLE[pitch-D_FREQUENCY_MIN][angle][row+GABOR_HALF_WINDOW][cols+GABOR_HALF_WINDOW] = (short)(value*D_GABOR_QUANTIZE);
                    Table_Gabor[(pitch-D_FREQUENCY_MIN)*64*D_GABOR_ROWCOLS_WINDOW+(angle/4)*D_GABOR_ROWCOLS_WINDOW+(row+D_GABOR_HALF_WINDOW)*D_GABOR_COLS_WINDOW+cols+D_GABOR_HALF_WINDOW]=(short)(value*D_GABOR_QUANTIZE);
                }
            }
        }
    }

/*
    FILE *fp = fopen("c:\\GaborTable.inc", "wt");
    int i;
    for(i=0; i<GABOR_TABLE_SIZE; i++)
    {
        if ((i%50) == 0)
            fprintf(fp, "\n");
        fprintf(fp, "%6d, ", D_GABOR_TABLE[i]);
    }
    fclose(fp);
*/
}

void CIBSMAlgorithm::_GenerateBlockNormalizeTable(void)
{
    int i, j;
    int value;

    //Block Normalization Table
    memset(Table_BlockNormalize, 0, sizeof(Table_BlockNormalize));
    for (i=D_CONTRAST_MIN; i<=D_CONTRAST_MAX; i++){
        for (j=0; j<=i+1; j++){
            value = (j*255)/(i+1);
            Table_BlockNormalize[i-D_CONTRAST_MIN][j] = value;
        }
    }
}

void CIBSMAlgorithm::_GenerateBinarizationTable(void)
{
    int i;

    for( i=0; i<QUANTIZED_DIRECTION; i++ )
    {
        g_flt_sin[i] = ( 4 * m_SingleSinTable[i+128])>>10;
        g_flt_cos[i] = (-4 * m_SingleCosTable[i+128])>>10;
        g_flt_dir[i] = (int)(i * 0.046875);
    }
}

void CIBSMAlgorithm::_GenerateThinningTable(void)
{
/*  int i, j;
    int count, index, trans;

    for (i=0; i<256; i++)
    {
        if (i==5 || i==13 || i==20 || i==22 || i==52 || i==54
            || i==65 || i==67 || i==80 || i==88 || i==97 || i==99
            || i==120 || i==133 || i==141 || i==208 || i==216)
            g_thinning_flag[i] = 0;
        else
            g_thinning_flag[i] = 255;
    }

    for (i=0; i<256; i++)
    {
        count = 0;
        index = i;

        for (j=0; j<8; j++)
        {
            if ((index&0x01)==0)
                count++;
            index = index >> 1;
        }

        trans = 0;
        index = i;

        for (j=0; j<7; j++)
        {
            if ((index&0x01)!=0 && (index&0x02)==0)
                trans++;
            index = index >> 1;
        }
        if ((i&0x80)!=0 && (i&0x01)==0)
            trans++;

        g_thinning_live[i] = 1;

        if (count>2 && count<8 && trans==1)
        {
            if ((i&0x08)!=0 || (i&0x20)!=0 || ((i&0x02)!=0 && (i&0x80)!=0))
                g_thinning_live[i] = 0;
        }
    }
*/

    int j,i;
    unsigned int direct2;
    //initial array which transform direction of moving in thining algorithm
    for (j=0; j<8; j++){
        for (i=0; i<256; i++){
            direct2=_getFirstBitFrom(i,j);
            m_RedirectArray[j*256+i]=direct2;

        }
    }
}

unsigned int CIBSMAlgorithm::_getFirstBitFrom(unsigned int value, unsigned int fromBit)
{
    unsigned int j;
    unsigned int result=0;
    fromBit+=3;
    //this function get number of first nonzero bit in variable "value", search start from bit with number "fromBit"
    for (j=0; j<=7; j++){
        if (((value>>((fromBit+16-j)%8))&1)==1){
            result=((fromBit+16-j)%8);
            break;
        }
    }
    return result;
}

int CIBSMAlgorithm::_GetAtan2(int y, int x)
{
    int i, r;
    int im_phi;
    int im_2phi;
    int im_phi2;
    int im_mul;

    im_2phi = QUANTIZED_DIRECTION * 2;
    im_phi = QUANTIZED_DIRECTION;
    im_phi2 = QUANTIZED_DIRECTION / 2;
    im_mul = MAX_MATH_SIZE;

    if (x == 0 && y < 0 ) return (im_phi2+im_phi);
    else if (x == 0 && y >= 0 ) return (im_phi2);
    else if (y == 0 && x >=0 ) return 0;
    else if (y == 0 && x < 0 ) return (im_phi);

    r = abs(y * im_mul / x);

    for (i = 0; i < im_phi2; i++)
        if (r < m_SingleTanTable[i+1]) break;

    if (y > 0){
        if (x < 0) return im_phi - i;
    }
    else{
        if (x < 0) return im_phi + i;
        else return im_2phi - i;
    }

    return i;
}

int CIBSMAlgorithm::_ALGO_ExtractFt(const unsigned char* input_image, unsigned char *output_template, int imgWidth, int imgHeight)
{
    int i, j;
    int qaScore=0, pitch=0, count=0;
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);
    int imgEnHalfWidth = imgEnWidth>>1;
    int imgEnHalfHeight = imgEnHeight>>1;
    int imgEnQuadWidth = imgEnHalfWidth>>1;
    int imgEnQuadHeight = imgEnHalfHeight>>1;
    int imgEnOctWidth = imgEnQuadWidth>>1;
    int imgEnOctHeight = imgEnQuadHeight>>1;
    int imgEnHalfOctWidth = imgEnOctWidth>>1;
    int imgEnHalfOctHeight = imgEnOctHeight>>1;

    m_pFeature1[0] = 0;
    m_pFeature2[0] = 0;

    _ALGO_ImageEnlargement(input_image, m_pEnlarge, imgWidth, imgHeight);
    _ALGO_HistogramStretch(m_pEnlarge, imgWidth, imgHeight);
    _ALGO_BackgroundDetection(m_pEnlarge, m_pSegBuf, imgWidth, imgHeight);
    _ALGO_BlockNormalization(m_pEnlarge, m_pSegBuf, imgWidth, imgHeight);
    _ALGO_OrientationCalculation(m_pEnlarge, m_pSegBuf, m_pDirBuf, m_pDirBuf2, m_pDirBuf3, imgWidth, imgHeight);
    qaScore = _ALGO_FrequencyCalculation(m_pEnlarge, m_pSegBuf, m_pDirBuf, m_pFrequencyBuf, imgWidth, imgHeight);
    _ALGO_GaborFiltration(m_pEnlarge, m_pSegBuf, m_pDirBuf, m_pFrequencyBuf, m_pGabor, imgWidth, imgHeight);

    {
        double MIN[30];
        double MAX[30];
        int i,j,period;
        memset(MIN,1000000000,sizeof(MIN));
        memset(MAX,-1000000000,sizeof(MAX));
        for(i=D_ENLARGE_BAND; i<imgHeight+D_ENLARGE_BAND; i++)
        {
            for(j=D_ENLARGE_BAND; j<imgWidth+D_ENLARGE_BAND; j++)
            {
                period=m_pFrequencyBuf[(i/D_FREQUENCY_BLOCK_SIZE)*imgEnHalfOctWidth+(j/D_FREQUENCY_BLOCK_SIZE)];

                if(MIN[period] > m_pGabor[i*imgEnWidth+j])
                    MIN[period] = m_pGabor[i*imgEnWidth+j];
                else if(MAX[period] < m_pGabor[i*imgEnWidth+j])
                    MAX[period] = m_pGabor[i*imgEnWidth+j];
            }
        }
        for(i=D_ENLARGE_BAND; i<imgHeight+D_ENLARGE_BAND; i++)
        {
            for(j=D_ENLARGE_BAND; j<imgWidth+D_ENLARGE_BAND; j++)
            {
                period=m_pFrequencyBuf[(i/D_FREQUENCY_BLOCK_SIZE)*imgEnHalfOctWidth+(j/D_FREQUENCY_BLOCK_SIZE)];
                m_pGaborSave[(i-D_ENLARGE_BAND)*imgWidth+(j-D_ENLARGE_BAND)] = (unsigned char)(255.0f * (m_pGabor[i*imgEnWidth+j]-MIN[period]) / (float)(MAX[period]-MIN[period]));
            }
        }
    }

    _ALGO_Binarization(m_pGabor, m_pSegBuf, m_pDirBuf, m_pBinary, imgWidth, imgHeight);

    // dump binary image
    {
        int i, j;
        for(i=D_ENLARGE_BAND; i<imgHeight+D_ENLARGE_BAND; i++)
        {
            for(j=D_ENLARGE_BAND; j<imgWidth+D_ENLARGE_BAND; j++)
            {
                m_pBinarySave[(i-D_ENLARGE_BAND)*imgWidth+(j-D_ENLARGE_BAND)] = m_pBinary[i*imgEnWidth+j];
            }
        }
    }

    _ALGO_InvertBinarization(m_pBinary, m_pSegBuf, m_pBinaryInv, imgWidth, imgHeight);

    _ALGO_Thinning(m_pBinary, m_pFeature1, 0, imgWidth, imgHeight, m_pRemoveArray);
    _ALGO_MinutiaeAngleCalculation(m_pBinary, m_pFeature1, 0, imgWidth, imgHeight);
    _ALGO_BrokenLineRecover(m_pBinary, m_pBinaryInv, m_pFeature1, 0, imgWidth, imgHeight);
    _ALGO_OutsideMinutiaeElimination(m_pBinary, m_pSegBuf, m_pFeature1, 0, imgWidth, imgHeight);

    // dump thinning image
    {
        int i, j;
        for(i=D_ENLARGE_BAND; i<imgHeight+D_ENLARGE_BAND; i++)
        {
            for(j=D_ENLARGE_BAND; j<imgWidth+D_ENLARGE_BAND; j++)
            {
                m_pThinSave[(i-D_ENLARGE_BAND)*imgWidth+(j-D_ENLARGE_BAND)] = m_pBinary[i*imgEnWidth+j];
            }
        }
    }

    _ALGO_Thinning(m_pBinaryInv, m_pFeature2, 1, imgWidth, imgHeight, m_pRemoveArray);
    _ALGO_MinutiaeAngleCalculation(m_pBinaryInv, m_pFeature2, 1, imgWidth, imgHeight);
    _ALGO_BrokenLineRecover(m_pBinaryInv, m_pBinary, m_pFeature2, 1, imgWidth, imgHeight);
    _ALGO_OutsideMinutiaeElimination(m_pBinaryInv, m_pSegBuf, m_pFeature2, 1, imgWidth, imgHeight);

    _ALGO_MinutiaeCombination(m_pFeature1, m_pFeature2, imgWidth, imgHeight);

    if(m_pFeature1[0] < 9)
        return 0;

    memcpy(output_template, m_pFeature1, (m_pFeature1[0]+1)*sizeof(unsigned int));

    // prefeagen
    _ALGO_Pre_FeaGen_RemaindBuffer((unsigned int*)output_template);

    m_Singular.FingerType = _ALGO_FingerPattern(m_pSegBuf, m_pDirBuf2, m_pDirBuf3, imgWidth, imgHeight, (unsigned int*)m_pFeature1, &m_Singular,
                    m_pThreeBin, m_pLabeledBin, m_pSingularBin, m_pTracingBin);

    for(i=1; i<imgEnHalfOctHeight-1; i++)
    {
        for(j=1; j<imgEnHalfOctWidth-1; j++)
        {
            if(m_pSegBuf[(i<<2)*imgEnQuadWidth+(j<<2)]==255)
                continue;

            pitch+=m_pFrequencyBuf[i*imgEnHalfOctWidth+j];
            count++;
        }
    }

    if(count==0)
        pitch=0;
    else
        pitch=pitch/count;

    unsigned char quality = qaScore/5000;
    if( quality > 15 ) quality = 15;

    output_template[1024] = quality;
    output_template[1025] = m_Singular.FingerType;
    output_template[1026] = pitch;
    output_template[1027] = m_Singular.DistCoreToDelta;

    return 1;
}

void CIBSMAlgorithm::_ALGO_ImageEnlargement(const unsigned char* input_image, unsigned char *output_image, int imgWidth, int imgHeight)
{
    int y;
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);
    unsigned char fill_value;
    int lowidx=0, highidx=0, lowsum=0, highsum=0, histogram[256];
    int cut_value;

    memset(histogram, 0, sizeof(histogram));

    for(y=0; y<imgWidth*imgHeight; y++)
        histogram[input_image[y]]++;

    for(y=0; y<256; y++)
    {
        if(histogram[y] > 0)
        {
            lowidx = y;
            break;
        }
    }
    for(y=255; y>=0; y--)
    {
        if(histogram[y] > 0)
        {
            highidx = y;
            break;
        }
    }
/*
    if(lowidx >= highidx)
    {
        fill_value = input_image[0];
        invert_flag = 0;
    }
    else
    {
        if(highidx < 2) highidx = 2;
        if(lowidx > 253) lowidx = 253;

        lowsum = histogram[lowidx] + histogram[lowidx+1] + histogram[lowidx+2];
        highsum = histogram[highidx-2] + histogram[highidx-1] + histogram[highidx];

        if(lowsum < highsum)
        {
            invert_flag = 1;
            fill_value = highidx;
        }
        else
        {
            invert_flag = 0;
            fill_value = lowidx;
        }
    }

    if(fill_value >= 128)
        fill_value = 255;
    else
        fill_value = 0;
*/

    if(lowidx == highidx)
        fill_value = 0;
    else
    {
        cut_value = (lowidx + highidx)/5;

        lowsum=0;
        for(y=lowidx; y<lowidx+cut_value; y++)
            lowsum += histogram[y];
        highsum=0;
        for(y=highidx; y>=highidx-cut_value; y--)
            highsum += histogram[y];

        if(lowsum > highsum)
            fill_value = 0;
        else
            fill_value = 255;
    }

    memset(&output_image[0], fill_value, D_ENLARGE_BAND*imgEnWidth);
    for(y=0; y<imgHeight; y++)
    {
        memset(&output_image[(y+D_ENLARGE_BAND)*imgEnWidth], fill_value, D_ENLARGE_BAND);
        memcpy(&output_image[(y+D_ENLARGE_BAND)*imgEnWidth+D_ENLARGE_BAND], &input_image[y*imgWidth], imgWidth);
        memset(&output_image[(y+D_ENLARGE_BAND)*imgEnWidth+D_ENLARGE_BAND+imgWidth], fill_value, D_ENLARGE_BAND);
    }
    memset(&output_image[(D_ENLARGE_BAND+imgHeight)*imgEnWidth], fill_value, D_ENLARGE_BAND*imgEnWidth);

    if(fill_value >= 128)
    {
        for(y=0; y<imgEnWidth*imgEnHeight; y++)
        {
            output_image[y] = 255-output_image[y];
        }
    }

#ifdef DEBUG_IMAGE
    _SaveBitmapImage("d:\\IB_OriginalImage.bmp", input_image, imgWidth, imgHeight, FALSE);
    _SaveBitmapImage("d:\\IB_ImageEnlargement.bmp", output_image, imgEnWidth, imgEnHeight, FALSE);
#endif
}

void CIBSMAlgorithm::_ALGO_MedianFilter(unsigned char *input_image, int imgWidth, int imgHeight, int *histogram)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);

    int i, j;
    int a, b, c;
    int temp;

// enzyme modify
// 사용할 필요가 없다
//  memcpy(temp_image_buffer, input_image, D_ENLARGE_H*D_ENLARGE_W);

    for (i=D_ENLARGE_BAND; i<imgEnHeight-D_ENLARGE_BAND; i++){
        for (j=D_ENLARGE_BAND; j<imgEnWidth-D_ENLARGE_BAND; j++){
            a = input_image[i*imgEnWidth+j];
            b = input_image[i*imgEnWidth+(j+1)];
            c = input_image[(i+1)*imgEnWidth+j];

            if(a>b){
                temp=a;
                a=b;
                b=temp;
            }

            if(b>c){
                temp=b;
                b=c;
                c=temp;
            }
            if(a>b){
                temp=a;
                a=b;
                b=temp;
            }
            input_image[i*imgEnWidth+j] = b;
            histogram[b]++;
        }
    }
}

void CIBSMAlgorithm::_ALGO_HistogramStretch(unsigned char *pImage, int imgWidth, int imgHeight)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);
    int y,x/*,gray*/;
    int histogram[256];
    int LUT[256],value;
    double scale_factor;
    int CUTMIN_PIXELNUM = (int)(IBSM_D_CUT_MINTRES*(imgWidth*imgHeight));
    int CUTMAX_PIXELNUM = (int)(IBSM_D_CUT_MAXTRES*(imgWidth*imgHeight));

    memset(histogram,0,sizeof(histogram));
    memset(LUT,0,sizeof(LUT));
    _ALGO_MedianFilter(pImage, imgWidth, imgHeight, histogram);
/*
    for(y=D_ENLARGE_BAND; y<imgEnHeight-D_ENLARGE_BAND; y++)
    {
        for(x=D_ENLARGE_BAND; x<imgEnWidth-D_ENLARGE_BAND; x++)
        {
            gray=pImage[y*imgEnWidth+x];
            histogram[gray]++;
        }
    }
*/
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

    //스트레칭의 강도를 조절할 수 있는 factor 계산
    if(MAX_TRES==MIN_TRES)
        scale_factor=0;
    else
        scale_factor = 255.0/(MAX_TRES-MIN_TRES);

    for(y=MIN_TRES; y<=MAX_TRES; y++)
    {
        value=(int)((y-MIN_TRES)*scale_factor);
        if(value>255)value=255;
        LUT[y] = value;
    }

    for(y=D_ENLARGE_BAND; y<imgEnHeight-D_ENLARGE_BAND; y++)
    {
        for(x=D_ENLARGE_BAND; x<imgEnWidth-D_ENLARGE_BAND; x++)
        {
            pImage[y*imgEnWidth+x] = LUT[pImage[y*imgEnWidth+x]];
        }
    }

#ifdef DEBUG_IMAGE
    _SaveBitmapImage("d:\\IB_HistogramStretch.bmp", pImage, imgEnWidth, imgEnHeight, FALSE);
#endif
}

void CIBSMAlgorithm::_StackRecursiveFilling(unsigned char *pImage, int imgWidth, int imgHeight, int x, int y, unsigned char TargetColor, unsigned char DestColor, unsigned short *stack)
{
    int dx, dy;
    int top;

    top = -1;               // init_stack
    stack[++top] = y;       // push
    stack[++top] = x;       // push

    while( top >= 0 )
    {
        dx = stack[top--];      // pop
        dy = stack[top--];      // pop
        if(dx < 0 || dy < 0 || dx >= imgWidth || dy >= imgHeight)
            continue;

        if(pImage[dy*imgWidth+dx] == TargetColor)
        {
            pImage[dy*imgWidth+dx] = DestColor;
            stack[++top] = dy;      // push
            stack[++top] = dx-1;    // push

            stack[++top] = dy;      // push
            stack[++top] = dx+1;    // push

            stack[++top] = dy-1;    // push
            stack[++top] = dx;      // push

            stack[++top] = dy+1;    // push
            stack[++top] = dx;      // push
        }
    }
}

void CIBSMAlgorithm::_ALGO_BackgroundDetection(unsigned char *pImage, unsigned char *pSegBuf, int imgWidth, int imgHeight)
{
    int j, i, k, l;
    int tempmean;
    int tempthreshold;
    int tempsum;
    int temp1, temp2;

    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);
    int imgEnHalfWidth = imgEnWidth>>1;
    int imgEnHalfHeight = imgEnHeight>>1;
    int imgEnQuadWidth = imgEnHalfWidth>>1;
    int imgEnQuadHeight = imgEnHalfHeight>>1;

    memset(pSegBuf, 255, imgEnQuadWidth*imgEnQuadHeight);
    memset(m_pHalfImage, 0, imgEnHalfWidth*imgEnHalfHeight);
    memset(m_pGradientBuffer, 0, imgEnHalfWidth*imgEnHalfHeight*sizeof(unsigned int));
    tempmean = 0;

    for (i=D_ENLARGE_BAND; i<imgEnHeight-D_ENLARGE_BAND; i+=2){
        for (j=D_ENLARGE_BAND; j<imgEnWidth-D_ENLARGE_BAND; j+=2){
            m_pHalfImage[(i>>1)*imgEnHalfWidth+(j>>1)] = ( pImage[i*imgEnWidth+j]
                                                    + pImage[i*imgEnWidth+j+1]
                                                    + pImage[(i+1)*imgEnWidth+j]
                                                    + pImage[(i+1)*imgEnWidth+j+1] )>>2;
        }
    }

    for (i=D_ENLARGE_BAND/2; i<imgEnHalfHeight-D_ENLARGE_BAND/2; i+=4){
        for (j=D_ENLARGE_BAND/2; j<imgEnHalfWidth-D_ENLARGE_BAND/2; j+=4){
            for (k=0; k<4; k++){
                for (l=0; l<4; l++){
                    temp1 = (int)m_pHalfImage[(i+k)*imgEnHalfWidth+(j+l)] - (int)m_pHalfImage[(i+k)*imgEnHalfWidth+(j+l+1)];
                    temp2 = (int)m_pHalfImage[(i+k)*imgEnHalfWidth+(j+l)] - (int)m_pHalfImage[(i+k+1)*imgEnHalfWidth+(j+l)];
                    temp1 = temp1*temp1 + temp2*temp2;
                    m_pGradientBuffer[(i+k)*imgEnHalfWidth+(j+l)] = temp1;
                    tempmean += temp1;
                }
            }
        }
    }

    tempmean/=(imgEnWidth*imgEnHeight/4);

    tempthreshold = (int)(55000.0-(tempmean*8.71));
    if (tempthreshold > 49000)  tempthreshold = 49000;
    else if (tempthreshold < 26500) tempthreshold = 26000;

    for (i=D_ENLARGE_BAND>>1; i<imgEnHalfHeight-(D_ENLARGE_BAND>>1); i+=2)
    {
        tempsum = 0;
        for (k=-4; k<=4; k++){
            for (l=-4; l<=4; l++){
                tempsum += m_pGradientBuffer[(i+k)*imgEnHalfWidth+(D_ENLARGE_BAND/2+l)];
            }
        }
        for (j=(D_ENLARGE_BAND>>1)+2; j<imgEnHalfWidth-(D_ENLARGE_BAND>>1); j+=2)
        {
            for (k=-4; k<=4; k++){
                for (l=-6; l<=-5; l++){
                    tempsum -= m_pGradientBuffer[(i+k)*imgEnHalfWidth+(j+l)];
                }
            }
            for (k=-4; k<=4; k++){
                for (l=3; l<=4; l++){
                    tempsum += m_pGradientBuffer[(i+k)*imgEnHalfWidth+(j+l)];
                }
            }
            if (tempsum > tempthreshold)
                pSegBuf[(i>>1)*imgEnQuadWidth+(j>>1)] = 0;
        }
    }

    memcpy(m_pTempQuadBuffer, pSegBuf, imgEnQuadWidth*imgEnQuadHeight);

    _StackRecursiveFilling(m_pTempQuadBuffer, imgEnQuadWidth, imgEnQuadHeight, 0, 0, 255, 0, m_pStackBuf);
    for(i=0; i<imgEnQuadHeight; i++)
    {
        for(j=0; j<imgEnQuadWidth; j++)
        {
            if(m_pTempQuadBuffer[i*imgEnQuadWidth+j] == 255)
            {
                pSegBuf[i*imgEnQuadWidth+j] = 0;
            }
        }
    }

#ifdef DEBUG_IMAGE
    _SaveBitmapImage_UserSize("d:\\IB_BackgroundDetection.bmp", pSegBuf, imgEnQuadWidth, imgEnQuadHeight, imgEnWidth, imgEnHeight, 1, FALSE);
#endif
}

void CIBSMAlgorithm::_ALGO_BlockNormalization(unsigned char *pImage, unsigned char *pSegBuf, int imgWidth, int imgHeight)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);
    int imgEnHalfWidth = imgEnWidth>>1;
    int imgEnQuadWidth = imgEnHalfWidth>>1;
    int i, j, k, l;
    int maxValue, minValue, tempValue;
    int contrast, contrast_index;

    memcpy(m_pTempImage, pImage, imgEnWidth*imgEnHeight);

    for( i=D_ENLARGE_BAND; i<imgEnHeight-D_ENLARGE_BAND; i+=16 )
    {
        for( j=D_ENLARGE_BAND; j<imgEnWidth-D_ENLARGE_BAND; j+=16 )
        {
            maxValue = 0;
            minValue = 255;
            for( k=-8; k<24; k++ )
            {
                for( l=-8; l<24; l++ )
                {
                    tempValue = m_pTempImage[(i+k)*imgEnWidth+(j+l)];
                    maxValue = (tempValue>maxValue)?tempValue:maxValue;
                    minValue = (tempValue<minValue)?tempValue:minValue;
                }
            }

            contrast = maxValue - minValue;

            if(contrast<D_CONTRAST_MIN || contrast>D_CONTRAST_MAX)
                continue;
            else{
                contrast_index = contrast - D_CONTRAST_MIN;
                for( k=0; k<16; k++ )
                {
                    for( l=0; l<16; l++ )
                    {
                        if( pSegBuf[((i+k)>>2)*imgEnQuadWidth+((j+l)>>2)] != 255 )
                        {
                            pImage[(i+k)*imgEnWidth+(j+l)] = Table_BlockNormalize[contrast_index][m_pTempImage[(i+k)*imgEnWidth+(j+l)]-minValue];
                        }
                    }
                }
            }
        }
    }

#ifdef DEBUG_IMAGE
    _SaveBitmapImage("d:\\IB_BlockNormalization.bmp", pImage, imgEnWidth, imgEnHeight, FALSE);
#endif
}

void CIBSMAlgorithm::_ALGO_OrientationCalculation(const unsigned char *pImage, unsigned char *pSegBuf, unsigned char *pDirBuf, unsigned char *pDirBuf2,
                               unsigned char *pDirBuf3, int imgWidth, int imgHeight)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);
    int imgEnHalfWidth = imgEnWidth>>1;
    int imgEnHalfHeight = imgEnHeight>>1;
    int imgEnQuadWidth = imgEnHalfWidth>>1;
    int imgEnQuadHeight = imgEnHalfHeight>>1;

    int X_sum, Y_sum;
    int temp1, temp2;
    int i, j, k, l;
    int cos_sum, sin_sum;
    int angleint;
    int gra_w = imgEnWidth/12;

    memset(pDirBuf, 0, imgEnQuadWidth*imgEnQuadHeight);
    memset(m_pTempDirBuf, 0, imgEnQuadWidth*imgEnQuadHeight);
    memset(m_X_gradient, 0, sizeof(int)*(imgEnWidth/12)*(imgEnHeight/12));
    memset(m_Y_gradient, 0, sizeof(int)*(imgEnWidth/12)*(imgEnHeight/12));

    for (i=D_ENLARGE_BAND; i<imgEnHeight-D_ENLARGE_BAND; i++)
    {
        for (j=D_ENLARGE_BAND; j<imgEnWidth-D_ENLARGE_BAND; j++)
        {
            temp1 = - pImage[(i-1)*imgEnWidth+(j-1)] + pImage[(i-1)*imgEnWidth+(j+1)]
                    - pImage[i*imgEnWidth+(j-1)] + pImage[i*imgEnWidth+(j+1)]
                    - pImage[(i+1)*imgEnWidth+(j-1)] + pImage[(i+1)*imgEnWidth+(j+1)];
            temp2 = - pImage[(i-1)*imgEnWidth+(j-1)] - pImage[(i-1)*imgEnWidth+j] - pImage[(i-1)*imgEnWidth+(j+1)]
                    + pImage[(i+1)*imgEnWidth+(j-1)] + pImage[(i+1)*imgEnWidth+j] + pImage[(i+1)*imgEnWidth+(j+1)];

            m_X_gradient[((i-4)/12)*gra_w+((j-4)/12)] += (temp1*temp2<<1);
            m_Y_gradient[((i-4)/12)*gra_w+((j-4)/12)] += (temp1*temp1 - temp2*temp2);

//          X_gradient[(i>>3)*imgEnOctWidth+(j>>3)] += (temp1*temp2<<1);
//          Y_gradient[(i>>3)*imgEnOctWidth+(j>>3)] += (temp1*temp1 - temp2*temp2);
        }
    }

/*  for (i=(D_ENLARGE_BAND>>3); i<imgEnOctHeight-(D_ENLARGE_BAND>>3); i++)
    {
        for (j=(D_ENLARGE_BAND>>3); j<imgEnOctWidth-(D_ENLARGE_BAND>>3); j++)
        {
            X_sum = 0;
            Y_sum = 0;
            for (k=-1; k<=1; k++){
                for (l=-1; l<=1; l++){
                    X_sum += X_gradient[(i+k)*imgEnOctWidth+(j+l)];
                    Y_sum += Y_gradient[(i+k)*imgEnOctWidth+(j+l)];
                }
            }

            angleint = _GetAtan2(X_sum>>10,Y_sum>>10)>>1;

            pTempDirBuf[(i<<1)*imgEnQuadWidth+(j<<1)] = angleint;
            pTempDirBuf[(i<<1)*imgEnQuadWidth+(j<<1)+1] = angleint;
            pTempDirBuf[((i<<1)+1)*imgEnQuadWidth+(j<<1)] = angleint;
            pTempDirBuf[((i<<1)+1)*imgEnQuadWidth+(j<<1)+1] = angleint;
        }
    }
*/
    for (i=1; i<imgEnHeight/12-1; i++)
    {
        for (j=1; j<imgEnWidth/12-1; j++)
        {
            X_sum = 0;
            Y_sum = 0;
            for (k=-1; k<=1; k++){
                for (l=-1; l<=1; l++){
                    X_sum += m_X_gradient[(i+k)*gra_w+(j+l)];
                    Y_sum += m_Y_gradient[(i+k)*gra_w+(j+l)];
                }
            }
//          angle = atan2(-X_sum, -Y_sum);
//          angleint = (int)((angle/3.14159265358979+1.0)*127.0+1.0);
            angleint = _GetAtan2(X_sum>>10,Y_sum>>10)>>1;

            m_pTempDirBuf[(i*3+1)*imgEnQuadWidth+(j*3+1)] = angleint;
            m_pTempDirBuf[(i*3+1)*imgEnQuadWidth+(j*3+1+1)] = angleint;
            m_pTempDirBuf[(i*3+1)*imgEnQuadWidth+(j*3+1+2)] = angleint;
            m_pTempDirBuf[(i*3+1+1)*imgEnQuadWidth+(j*3+1)] = angleint;
            m_pTempDirBuf[(i*3+1+1)*imgEnQuadWidth+(j*3+1+1)] = angleint;
            m_pTempDirBuf[(i*3+1+1)*imgEnQuadWidth+(j*3+1+2)] = angleint;
            m_pTempDirBuf[(i*3+1+2)*imgEnQuadWidth+(j*3+1)] = angleint;
            m_pTempDirBuf[(i*3+1+2)*imgEnQuadWidth+(j*3+1+1)] = angleint;
            m_pTempDirBuf[(i*3+1+2)*imgEnQuadWidth+(j*3+1+2)] = angleint;
        }
    }

    for (i=(D_ENLARGE_BAND>>2); i<imgEnQuadHeight-(D_ENLARGE_BAND>>2); i++)
    {
        for (j=(D_ENLARGE_BAND>>2); j<imgEnQuadWidth-(D_ENLARGE_BAND>>2); j++)
        {
            if(pSegBuf[i*imgEnQuadWidth+j]==255)
                continue;

            cos_sum = 1;
            sin_sum = 1;
            for (k=-4; k<=4; k++){
                for (l=-4; l<=4; l++){
                    if(pSegBuf[(i+k)*imgEnQuadWidth+(j+l)]!=255)
                    {
                        cos_sum+=m_DoubleCosTable[m_pTempDirBuf[(i+k)*imgEnQuadWidth+(j+l)]];
                        sin_sum+=m_DoubleSinTable[m_pTempDirBuf[(i+k)*imgEnQuadWidth+(j+l)]];
                    }
                }
            }
            pDirBuf[i*imgEnQuadWidth+j] = _GetAtan2(sin_sum, cos_sum)>>1;
        }
    }

    int angle;
    for( i=0; i<imgEnQuadHeight; i++ )
    {
        for( j=0; j<imgEnQuadWidth; j++ )
        {
            if( pSegBuf[i*imgEnQuadWidth+j] == 255 )
            {
                pDirBuf2[i*imgEnQuadWidth+j] = 0;
                pDirBuf3[i*imgEnQuadWidth+j] = 0;
                continue;
            }

            angle = pDirBuf[i*imgEnQuadWidth+j] + 128;
            if(angle < 0)
                angle = angle + 255;

            if( angle < 128 )
                pDirBuf2[i*imgEnQuadWidth+j] = (127-angle);
            else
                pDirBuf2[i*imgEnQuadWidth+j] = (128+255-angle);

            angle = pDirBuf[i*imgEnQuadWidth+j];
            if( angle < 128 )
                pDirBuf3[i*imgEnQuadWidth+j] = (127-angle);
            else
                pDirBuf3[i*imgEnQuadWidth+j] = (128+255-angle);
        }
    }

#ifdef DEBUG_IMAGE
    _SaveBitmapImage_UserSize("d:\\IB_OrientationCalculation.bmp", pDirBuf, imgEnQuadWidth, imgEnQuadHeight, imgEnWidth, imgEnHeight, 1, FALSE);
#endif
}

int CIBSMAlgorithm::_ALGO_FrequencyCalculation(unsigned char *pImage, unsigned char *pSegBuf, unsigned char *pDirBuf, unsigned char *pFrequencyBuf,
                            int imgWidth, int imgHeight)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);
    int imgEnHalfWidth = imgEnWidth>>1;
    int imgEnHalfHeight = imgEnHeight>>1;
    int imgEnQuadWidth = imgEnHalfWidth>>1;
    int imgEnQuadHeight = imgEnHalfHeight>>1;
    int imgEnOctWidth = imgEnQuadWidth>>1;
    int imgEnOctHeight = imgEnQuadHeight>>1;
    int imgEnHalfOctWidth = imgEnOctWidth>>1;
    int imgEnHalfOctHeight = imgEnOctHeight>>1;

    int y,x,angle;
    int angle_sin,angle_cos;
    int i,j;
    int row,cols,xx,yy,count_pixel=0,gray_sum=0,index=0;
    int PERIOD_HISTO[D_FREQUENCY_BOX_WIDTH+1];              //Just histogram of fingerimage
    int PERIOD_CORHISTO[D_FREQUENCY_BOX_WIDTH+1];           //Auto Correlation value of histogram
    int cols2,correlationVAR;
    int pitch;
    int FREQ_QUALITY=0;

    memset(pFrequencyBuf,0,imgEnHalfOctWidth*imgEnHalfOctHeight);

    for(i=D_ENLARGE_BAND; i<imgEnHeight-D_ENLARGE_BAND; i+=D_FREQUENCY_BLOCK_SIZE)
    {
        y = i+D_FREQUENCY_BLOCK_SIZE/2;

        for(j=D_ENLARGE_BAND; j<imgEnWidth-D_ENLARGE_BAND ; j+=D_FREQUENCY_BLOCK_SIZE)
        {
            x = j+D_FREQUENCY_BLOCK_SIZE/2;

            if( pSegBuf[(y>>2)*imgEnQuadWidth+(x>>2)] == 255 )
                continue;

            angle = pDirBuf[(y>>2)*imgEnQuadWidth+(x>>2)];
            angle_sin = m_SingleSinTable[angle];
            angle_cos = m_SingleCosTable[angle];

            //1. Raw PERIOD Calculation
            index=0;
            memset(PERIOD_HISTO, 0, sizeof(int)*(D_FREQUENCY_BOX_WIDTH+1));
            for(cols=x-D_FREQUENCY_BOX_WIDTH/2; cols<=x+D_FREQUENCY_BOX_WIDTH/2; cols++)
            {
                gray_sum=0;
                count_pixel=0;
                for(row=y-D_FREQUENCY_BOX_HEIGHT/2; row<=y+D_FREQUENCY_BOX_HEIGHT/2; row++)
                {
                    xx = (((cols-x)*angle_cos - (row-y)*angle_sin)>>10) + x;
                    yy = (((cols-x)*angle_sin + (row-y)*angle_cos)>>10) + y;
                    if((yy<D_ENLARGE_BAND) || (yy>imgEnHeight-D_ENLARGE_BAND) || (xx<D_ENLARGE_BAND) || (xx>imgEnWidth-D_ENLARGE_BAND))
                        continue;
                    gray_sum+=pImage[yy*imgEnWidth+xx];
                    count_pixel++;
                }
                if(count_pixel<=0)
                    gray_sum=0;
                else
                    gray_sum=gray_sum/count_pixel;
                PERIOD_HISTO[index]=gray_sum;
                index++;
            }
            ///////////////////////////2. Auto Correlation//////////////////////////////
            //Auto Correlation value of histogram
            memset(PERIOD_CORHISTO, 0, sizeof(int)*(D_FREQUENCY_BOX_WIDTH+1));
            for(cols=0; cols<=D_FREQUENCY_BOX_WIDTH; cols++)
            {
                correlationVAR=0;
                for(cols2=0; cols2<=D_FREQUENCY_BOX_WIDTH; cols2++)
                {
                    if(cols2+cols>D_FREQUENCY_BOX_WIDTH)
                        continue;
                    correlationVAR+=(PERIOD_HISTO[cols2]*PERIOD_HISTO[cols2+cols]);
                }
                PERIOD_CORHISTO[cols]=correlationVAR/(D_FREQUENCY_BOX_WIDTH+1-cols);
            }
            ///////////////////////////////////////////////////////////////////////////

            //////////////3. save reliable Pitch////////////////////////////////////////
            pitch = _NewCalculation_Pitch(PERIOD_CORHISTO,&FREQ_QUALITY);
            pFrequencyBuf[(i/D_FREQUENCY_BLOCK_SIZE)*imgEnHalfOctWidth+(j/D_FREQUENCY_BLOCK_SIZE)]=pitch;
            ////////////////////////////////////////////////////////////////////////////
        }
    }

//  memcpy(FRQUENCY_NOT_BUFFER, FRQUENCY_BUFFER, D_FREQUENCY_HEIGHT*D_FREQUENCY_WIDTH);
    //////////////4. Period filling and smoothing////////////////////////////////////////
    _NewCalculationPeriodSmoothing(pFrequencyBuf, imgWidth, imgHeight, m_pTempDirBuf);

/*  for(i=0; i<imgEnHalfOctHeight; i++)
    {
        for(j=0; j<imgEnHalfOctWidth; j++)
        {
            count_pixel=0;

            for(row=0; row<4; row++)
            {
                for(cols=0; cols<4; cols++)
                {
                    if( pSegBuf[(i*4+row)*imgEnQuadWidth+(j*4+cols)] != 255 )
                    {
                        count_pixel=1;
                        row=4;
                        cols=4;
                        break;
                    }
                }
            }

            if(count_pixel == 0)
                pFrequencyBuf[i*imgEnHalfOctWidth+j] = 0;
        }
    }*/

#ifdef DEBUG_IMAGE
    _SaveBitmapImage_UserSize("d:\\IB_FrequencyCalculation.bmp", pFrequencyBuf, imgEnHalfOctWidth, imgEnHalfOctHeight, imgEnWidth, imgEnHeight, 5, FALSE);
#endif

    return FREQ_QUALITY;
}

int CIBSMAlgorithm::_NewCalculation_Pitch(int *PERIOD_SIGNAL, int* quality)
{
    int x,diff_peak;
    int lowpeak=999999999,highpeak=-999999999;
    int /*first_lowindex,*/first_highindex=0;
    int signal2,signal3,signal4,signal5,signal6,signal7,signal8,signal9;//,signal10;

    for(x=1; x<=D_FREQUENCY_BOX_WIDTH-8; x++)
    {
        signal2 = PERIOD_SIGNAL[x-1];
        signal3 = PERIOD_SIGNAL[x];
        signal4 = PERIOD_SIGNAL[x+1];
        signal5 = PERIOD_SIGNAL[x+2];
        signal6 = PERIOD_SIGNAL[x+3];
        signal7 = PERIOD_SIGNAL[x+4];
        signal8 = PERIOD_SIGNAL[x+5];
        signal9 = PERIOD_SIGNAL[x+6];
//        signal10 = PERIOD_SIGNAL[x+7];

        if(signal3<signal2 &&
            (signal3<signal4 || signal3<signal5 ||  signal3<signal6 ||  signal3<signal7 ||  signal3< signal8) &&
            lowpeak==999999999)
        {
            if(signal3<signal4 && signal4<=signal5 )
            {
                //first_lowindex=x;
                lowpeak=signal3;
            }
            else if(signal3<=signal4 && signal3<signal5 && signal5<=signal6 )
            {
                //first_lowindex=x;
                lowpeak=signal3;
            }
            else if(signal3<=signal4 && signal3<=signal5 && signal3<signal6 && signal6<=signal7 )
            {
                //first_lowindex=x;
                lowpeak=signal3;
            }
            else if(signal3<=signal4 && signal3<=signal5 && signal3<=signal6 && signal3<signal7 && signal7<=signal8 )
            {
                //first_lowindex=x;
                lowpeak=signal3;
            }
            else if(signal3<=signal4 && signal3<=signal5 && signal3<=signal6 && signal3<=signal7 && signal3<signal8 && signal8<=signal9 )
            {
                //first_lowindex=x;
                lowpeak=signal3;
            }
        }
        else if(signal3>signal2 &&
            (signal3>signal4 || signal3>signal5 ||  signal3>signal6 ||  signal3>signal7 ||  signal3>signal8) &&
            lowpeak<999999999)
        {
            if(signal3>signal4 && signal4>=signal5 )
            {
                first_highindex=x;
                highpeak=signal3;
            }
            else if(signal3>=signal4 && signal3>signal5 && signal5>=signal6)
            {
                first_highindex=x;
                highpeak=signal3;
            }
            else if(signal3>=signal4 && signal3>=signal5 && signal3>signal6 && signal6>=signal7)
            {
                first_highindex=x;
                highpeak=signal3;
            }
            else if(signal3>=signal4 && signal3>=signal5 && signal3>=signal6 && signal3>signal7 && signal7>=signal8)
            {
                first_highindex=x;
                highpeak=signal3;
            }
            else if(signal3>=signal4 && signal3>=signal5 && signal3>=signal6 && signal3>=signal7 && signal3>signal8 && signal8>=signal9)
            {
                first_highindex=x;
                highpeak=signal3;
            }

        }
        if(highpeak !=-999999999 && lowpeak != 999999999)
            break;
    }

    diff_peak = highpeak-lowpeak;

    if(diff_peak<0)
        diff_peak=0;
    if (diff_peak > 5000)
        diff_peak = 5000;
    *quality= *quality+(diff_peak>>5);
//  if(diff_peak>400)
    if(diff_peak>1000)
    {
        if(first_highindex>D_FREQUENCY_MAX)
            first_highindex=D_FREQUENCY_MAX;
        else if(first_highindex<D_FREQUENCY_MIN)
            first_highindex=D_FREQUENCY_MIN;
        return first_highindex;
    }
    return 0;
}

void CIBSMAlgorithm::_NewCalculationPeriodSmoothing(unsigned char *pFrequencyBuf, int imgWidth, int imgHeight, unsigned char *pTemp)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);
    int imgEnHalfWidth = imgEnWidth>>1;
    int imgEnHalfHeight = imgEnHeight>>1;
    int imgEnQuadWidth = imgEnHalfWidth>>1;
    int imgEnQuadHeight = imgEnHalfHeight>>1;
    int imgEnOctWidth = imgEnQuadWidth>>1;
    int imgEnOctHeight = imgEnQuadHeight>>1;
    int imgEnHalfOctWidth = imgEnOctWidth>>1;
    int imgEnHalfOctHeight = imgEnOctHeight>>1;

    int y,x,row,cols,aver,value,count,zeropitchCount,averPitch,whileLoop=10,sum;

    ///////////////////////////////////////////////////
    /////두꺼운 PERIOD 스무딩
    int THICK_TRES  = 11;
    memcpy(pTemp,pFrequencyBuf,imgEnHalfOctWidth*imgEnHalfOctHeight);
    for(y=1; y<imgEnHalfOctHeight-1; y++)
    {
        for(x=1; x<imgEnHalfOctWidth-1; x++)
        {
            value=pTemp[y*imgEnHalfOctWidth+x];
            if(value<THICK_TRES)continue;
            aver=0;
            count=0;
            for(row=-1; row<=1; row++)
            {
                for(cols=-1; cols<=1; cols++)
                {
                    if(row==0 && cols==0)continue;
                    value=pTemp[(y+row)*imgEnHalfOctWidth+(x+cols)];
                    if(value<D_FREQUENCY_MIN || value>D_FREQUENCY_MAX)continue;
                    aver+=pTemp[(y+row)*imgEnHalfOctWidth+(x+cols)];
                    count++;
                }
            }
            if(count>2)
                pFrequencyBuf[y*imgEnHalfOctWidth+x]=aver/count;
            else
                pFrequencyBuf[y*imgEnHalfOctWidth+x]=0;
        }
    }

    ///////////////////////////////
    /////구해진 PERIOD 스무딩.
    memcpy(pTemp,pFrequencyBuf,imgEnHalfOctWidth*imgEnHalfOctHeight);
    for(y=1; y<imgEnHalfOctHeight-1; y++)
    {
        for(x=1; x<imgEnHalfOctWidth-1; x++)
        {
            sum=0;
            count=0;
            for(row=y-1; row<=y+1; row++)
            {
                for(cols=x-1; cols<=x+1; cols++)
                {
                    if(row==y && cols==x)continue;
                    value=pTemp[row*imgEnHalfOctWidth+cols];
                    if(value<D_FREQUENCY_MIN || value>D_FREQUENCY_MAX)continue;

                    sum+=value;
                    count++;
                }
            }
            if(count>2)
            {
                sum=sum/count;
                if(pTemp[y*imgEnHalfOctWidth+x]-sum>1 || pTemp[y*imgEnHalfOctWidth+x]-sum<-1)
                    pFrequencyBuf[y*imgEnHalfOctWidth+x]=sum;
            }
        }
    }

    ////////////////////////////////
    //////////빈칸 채워넣기
    count=0;
    averPitch=0;
    zeropitchCount=0;
    for(y=1; y<imgEnHalfOctHeight-1; y++)
    {
        for(x=1; x<imgEnHalfOctWidth-1; x++)
        {
            if(pFrequencyBuf[y*imgEnHalfOctWidth+x]==0)
                zeropitchCount++;
            else
            {
                averPitch+=pFrequencyBuf[y*imgEnHalfOctWidth+x];
                count++;
            }
        }
    }
    if(count==0)
        averPitch=9;
    else
        averPitch=averPitch/count;

    while (zeropitchCount>0 && whileLoop>0)
    {
        whileLoop--;

        for(y=1; y<imgEnHalfOctHeight-1; y++)
        {
            for(x=1; x<imgEnHalfOctWidth-1; x++)
            {
                if(pFrequencyBuf[y*imgEnHalfOctWidth+x]>0)continue;

                sum=0;
                count=0;
                for(row=y-1; row<=y+1; row++)
                {
                    for(cols=x-1; cols<=x+1; cols++)
                    {
                        value=pFrequencyBuf[row*imgEnHalfOctWidth+cols];
                        if(value<=0)continue;

                        sum+=value;
                        count++;
                    }
                }
                if(count>2)
                {
                    sum=sum/count;
                    pFrequencyBuf[y*imgEnHalfOctWidth+x]=sum;
                    zeropitchCount--;
                }
            }
        }
    }

    ///////////////////////////////////////////////////
    /////두꺼운 PERIOD 스무딩
    memcpy(pTemp,pFrequencyBuf,imgEnHalfOctWidth*imgEnHalfOctHeight);
    for(y=1; y<imgEnHalfOctHeight-1; y++)
    {
        for(x=1; x<imgEnHalfOctWidth-1; x++)
        {
            value=pTemp[y*imgEnHalfOctWidth+x];
            if(value<THICK_TRES)continue;
            aver=0;
            count=0;
            for(row=-1; row<=1; row++)
            {
                for(cols=-1; cols<=1; cols++)
                {
                    if(row==0 && cols==0)continue;
                    value=pTemp[(y+row)*imgEnHalfOctWidth+(x+cols)];
                    if(value<D_FREQUENCY_MIN || value>D_FREQUENCY_MAX)continue;
                    aver+=pTemp[(y+row)*imgEnHalfOctWidth+(x+cols)];
                    count++;
                }
            }
            if(count>2)
                pFrequencyBuf[y*imgEnHalfOctWidth+x]=aver/count;
            else
                pFrequencyBuf[y*imgEnHalfOctWidth+x]=0;
        }
    }

    ///////////////////////////////
    /////구해진 PERIOD 스무딩.
    memcpy(pTemp,pFrequencyBuf,imgEnHalfOctWidth*imgEnHalfOctHeight);
    for(y=1; y<imgEnHalfOctHeight-1; y++)
    {
        for(x=1; x<imgEnHalfOctWidth-1; x++)
        {
            sum=0;
            count=0;
            for(row=y-1; row<=y+1; row++)
            {
                for(cols=x-1; cols<=x+1; cols++)
                {
                    if(row==y && cols==x)continue;
                    value=pTemp[row*imgEnHalfOctWidth+cols];
                    if(value<D_FREQUENCY_MIN || value>D_FREQUENCY_MAX)continue;

                    sum+=value;
                    count++;
                }
            }
            if(count>2)
            {
                sum=sum/count;
                if(pTemp[y*imgEnHalfOctWidth+x]-sum>1 || pTemp[y*imgEnHalfOctWidth+x]-sum<-1)
                    pFrequencyBuf[y*imgEnHalfOctWidth+x]=sum;
            }
        }
    }

    //마지막 빈칸 채우기
    for(y=0; y<imgEnHalfOctHeight; y++)
    {
        for(x=0; x<imgEnHalfOctWidth; x++)
        {
            value=pFrequencyBuf[y*imgEnHalfOctWidth+x];
            if(value==0)
            {
                pFrequencyBuf[y*imgEnHalfOctWidth+x]=averPitch;
            }
            else if(value>D_FREQUENCY_MAX)
            {
                pFrequencyBuf[y*imgEnHalfOctWidth+x]=D_FREQUENCY_MAX;
            }
            else if(value<D_FREQUENCY_MIN)
            {
                pFrequencyBuf[y*imgEnHalfOctWidth+x]=D_FREQUENCY_MIN;
            }
        }
    }
}

void CIBSMAlgorithm::_ALGO_GaborFiltration(const unsigned char *pImage, const unsigned char *pSegBuf, const unsigned char *pDirBuf,
                         const unsigned char *pFrequencyBuf, int *pGabor, int imgWidth, int imgHeight)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);
    int imgEnHalfWidth = imgEnWidth>>1;
    int imgEnHalfHeight = imgEnHeight>>1;
    int imgEnQuadWidth = imgEnHalfWidth>>1;
    int imgEnQuadHeight = imgEnHalfHeight>>1;
    int imgEnOctWidth = imgEnQuadWidth>>1;
    int imgEnHalfOctWidth = imgEnOctWidth>>1;

#ifdef DEBUG_IMAGE
    int i;
#endif
    int y,x,row,col,angle,pitch_index, tp_x, tp_y;
    int x_index,y_index;
    int gabor_val;
    int index=0,k;
    int GB_WIN_MULTIPLE_256,GB_WIN_MULTIPLE_ANGLE,GB_WIN_DIFFER_MINPERIOD,GB_WIN_PERIOD_INDEX;

    memset(pGabor, 0, imgEnWidth*imgEnHeight*sizeof(int));
    for (y=D_ENLARGE_BAND; y<imgEnHeight-D_ENLARGE_BAND; y++)
    {
        memset(&pGabor[y*imgEnWidth], 0, D_ENLARGE_BAND*sizeof(int));
        memset(&pGabor[y*imgEnWidth+imgEnWidth-D_ENLARGE_BAND], 0, D_ENLARGE_BAND*sizeof(int));
    }
    memset(&pGabor[y*imgEnWidth], 0, imgEnWidth*D_ENLARGE_BAND*4);

    GB_WIN_MULTIPLE_256 = D_GABOR_ROWCOLS_WINDOW<<6;

    for(y=(D_ENLARGE_BAND>>2); y<imgEnQuadHeight-(D_ENLARGE_BAND>>2); y++)
    {
        for(x=(D_ENLARGE_BAND>>2); x<imgEnQuadWidth-(D_ENLARGE_BAND>>2); x++)
        {
            if( pSegBuf[y*imgEnQuadWidth+x]==255 )
            {
                tp_y = y<<2;
                tp_x = x<<2;
                memset(&pGabor[tp_y*imgEnWidth+tp_x], 0, 16);
                memset(&pGabor[(tp_y+1)*imgEnWidth+tp_x], 0, 16);
                memset(&pGabor[(tp_y+2)*imgEnWidth+tp_x], 0, 16);
                memset(&pGabor[(tp_y+3)*imgEnWidth+tp_x], 0, 16);
                continue;
            }

            pitch_index = pFrequencyBuf[(y>>2)*imgEnHalfOctWidth+(x>>2)];
            angle = pDirBuf[y*imgEnQuadWidth+x]>>2;

            GB_WIN_MULTIPLE_ANGLE = angle*D_GABOR_ROWCOLS_WINDOW;
            GB_WIN_DIFFER_MINPERIOD = pitch_index-D_FREQUENCY_MIN;
            GB_WIN_PERIOD_INDEX = GB_WIN_DIFFER_MINPERIOD*GB_WIN_MULTIPLE_256+GB_WIN_MULTIPLE_ANGLE;

            for(y_index=0; y_index<4; y_index++)
            {
                tp_y = y*4+y_index;
                for(x_index=0; x_index<4; x_index++)
                {
                    tp_x = x*4+x_index;
                    gabor_val=0;
                    index = GB_WIN_PERIOD_INDEX;

                    col = tp_x-D_GABOR_HALF_WINDOW;

                    for(row=tp_y-D_GABOR_HALF_WINDOW; row<=tp_y+D_GABOR_HALF_WINDOW-1; row++)
                    {
                        for(k=0; k<15; k++)
                        {
                            gabor_val += pImage[row*imgEnWidth+(col+k)] * Table_Gabor[index+k];
                        }
                        index+=15;
                    }
                    pGabor[tp_y*imgEnWidth+tp_x] = gabor_val;
                }
            }
        }
    }

#ifdef DEBUG_IMAGE
    int min_val=1000000000, max_val=-1000000000;
    int j;//,period;
    unsigned char *pOutGabor = new unsigned char[imgEnWidth*imgEnHeight];
    memset(pOutGabor, 127, imgEnWidth*imgEnHeight);
    for(i=D_ENLARGE_BAND; i<imgEnHeight-D_ENLARGE_BAND; i++)
    {
        for(j=D_ENLARGE_BAND; j<imgEnWidth-D_ENLARGE_BAND; j++)
        {
            if(min_val > pGabor[i*imgEnWidth+j])
                min_val = pGabor[i*imgEnWidth+j];
            else if(max_val < pGabor[i*imgEnWidth+j])
                max_val = pGabor[i*imgEnWidth+j];
        }
    }
    for(i=D_ENLARGE_BAND; i<imgEnHeight-D_ENLARGE_BAND; i++)
    {
        for(j=D_ENLARGE_BAND; j<imgEnWidth-D_ENLARGE_BAND; j++)
        {
            pOutGabor[i*imgEnWidth+j] = (unsigned char)(255.0f * (pGabor[i*imgEnWidth+j]-min_val) / (float)(max_val-min_val));
        }
    }

    _SaveBitmapImage("d:\\IB_GaborFiltration.bmp", pOutGabor, imgEnWidth, imgEnHeight, FALSE);
    delete [] pOutGabor;
#endif
}

void CIBSMAlgorithm::_ALGO_Binarization(const int *pGabor, const unsigned char *pSegBuf, const unsigned char *pDirBuf, unsigned char *pOutImage, int imgWidth, int imgHeight)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);
    int imgEnHalfWidth = imgEnWidth>>1;
    int imgEnQuadWidth = imgEnHalfWidth>>1;

    const signed char direction_line_table[13][7][2]={
        {{-3, 0},{-2, 0},{-1, 0},{ 0, 0},{ 1, 0},{ 2, 0},{ 3, 0}},  //0     0~10
        {{-3, 1},{-2, 1},{-1, 0},{ 0, 0},{ 1, 0},{ 2,-1},{ 3,-1}},  //21    11~31
        {{-3, 2},{-2, 1},{-1, 1},{ 0, 0},{ 1,-1},{ 2,-1},{ 3,-2}},  //43    32~53
        {{-3, 3},{-2, 2},{-1, 1},{ 0, 0},{ 1,-1},{ 2,-2},{ 3,-3}},  //64    54~75
        {{-2, 3},{-1, 2},{-1, 1},{ 0, 0},{ 1,-1},{ 1,-2},{ 2,-3}},  //86    76~97
        {{-1, 3},{-1, 2},{ 0, 1},{ 0, 0},{ 0,-1},{ 1,-2},{ 1,-3}},  //107   98~118
        {{ 0, 0},{ 0,-2},{ 0,-1},{ 0, 0},{ 0, 1},{ 0, 2},{ 0, 3}},  //128   118~139
        {{-1,-3},{-1,-2},{ 0,-1},{ 0, 0},{ 0, 1},{ 1, 2},{ 1, 3}},  //150   140~160
        {{-2,-3},{-1,-2},{-1,-1},{ 0, 0},{ 1, 1},{ 1, 2},{ 2, 3}},  //171   161~181
        {{-3,-3},{-2,-2},{-1,-1},{ 0, 0},{ 1, 1},{ 2, 2},{ 3, 3}},  //192   182~202
        {{-3,-2},{-2,-1},{-1,-1},{ 0, 0},{ 1, 1},{ 2, 1},{ 3, 2}},  //214   203~223
        {{-3,-1},{-2,-1},{-1, 0},{ 0, 0},{ 1, 0},{ 2, 1},{ 3, 1}},  //235   224~244
        {{-3, 0},{-2, 0},{-1, 0},{ 0, 0},{ 1, 0},{ 2, 0},{ 3, 0}},  //256   245~255
    };

    int sum1, sum2;
    int i, j, k, l;
    int dx, dy;
    int ty;
    int direction_index;
    int d1, d2, d3;
    int tempX1, tempX2, tempX3, tempX4, tempX5, tempX6, tempX7;
    int tempY1, tempY2, tempY3, tempY4, tempY5, tempY6, tempY7;

    memset(pOutImage, 0, imgEnWidth*imgEnHeight);

    for (i=D_ENLARGE_BAND; i<imgEnHeight-D_ENLARGE_BAND; i+=4)
    {
        d1 = i >> 2;
        for (j=D_ENLARGE_BAND; j<imgEnWidth-D_ENLARGE_BAND; j+=4)
        {
            d2 = j >> 2;

            if(pSegBuf[d1*imgEnQuadWidth+d2] == 255)
                continue;

            d3 = pDirBuf[d1*imgEnQuadWidth+d2];
            direction_index = g_flt_dir[d3];
            dx = g_flt_sin[d3];
            dy = g_flt_cos[d3];

            for( k=0; k<4; k++ )
            {
                ty = i + k;

                for( l=0; l<4; l++ )
                {
                    tempY1=ty+direction_line_table[direction_index][0][0];
                    tempX1=j+direction_line_table[direction_index][0][1];
                    tempY2=ty+direction_line_table[direction_index][1][0];
                    tempX2=j+direction_line_table[direction_index][1][1];
                    tempY3=ty+direction_line_table[direction_index][2][0];
                    tempX3=j+direction_line_table[direction_index][2][1];
                    tempY4=ty+direction_line_table[direction_index][3][0];
                    tempX4=j+direction_line_table[direction_index][3][1];
                    tempY5=ty+direction_line_table[direction_index][4][0];
                    tempX5=j+direction_line_table[direction_index][4][1];
                    tempY6=ty+direction_line_table[direction_index][5][0];
                    tempX6=j+direction_line_table[direction_index][5][1];
                    tempY7=ty+direction_line_table[direction_index][6][0];
                    tempX7=j+direction_line_table[direction_index][6][1];

                    sum1=pGabor[tempY1*imgEnWidth+tempX1]+pGabor[tempY1*imgEnWidth+tempX1];
                    sum2=(pGabor[(tempY1+dy)*imgEnWidth+tempX1+dx]+pGabor[(tempY1-dy)*imgEnWidth+tempX1-dx]);

                    sum1+=pGabor[tempY2*imgEnWidth+tempX2]+pGabor[tempY2*imgEnWidth+tempX2];
                    sum2+=(pGabor[(tempY2+dy)*imgEnWidth+tempX2+dx]+pGabor[(tempY2-dy)*imgEnWidth+tempX2-dx]);

                    sum1+=pGabor[tempY3*imgEnWidth+tempX3]+pGabor[tempY3*imgEnWidth+tempX3];
                    sum2+=(pGabor[(tempY3+dy)*imgEnWidth+tempX3+dx]+pGabor[(tempY3-dy)*imgEnWidth+tempX3-dx]);

                    sum1+=pGabor[tempY4*imgEnWidth+tempX4]+pGabor[tempY4*imgEnWidth+tempX4];
                    sum2+=(pGabor[(tempY4+dy)*imgEnWidth+tempX4+dx]+pGabor[(tempY4-dy)*imgEnWidth+tempX4-dx]);

                    sum1+=pGabor[tempY5*imgEnWidth+tempX5]+pGabor[tempY5*imgEnWidth+tempX5];
                    sum2+=(pGabor[(tempY5+dy)*imgEnWidth+tempX5+dx]+pGabor[(tempY5-dy)*imgEnWidth+tempX5-dx]);

                    sum1+=pGabor[tempY6*imgEnWidth+tempX6]+pGabor[tempY6*imgEnWidth+tempX6];
                    sum2+=(pGabor[(tempY6+dy)*imgEnWidth+tempX6+dx]+pGabor[(tempY6-dy)*imgEnWidth+tempX6-dx]);

                    sum1+=pGabor[tempY7*imgEnWidth+tempX7]+pGabor[tempY7*imgEnWidth+tempX7];
                    sum2+=(pGabor[(tempY7+dy)*imgEnWidth+tempX7+dx]+pGabor[(tempY7-dy)*imgEnWidth+tempX7-dx]);

                    if( sum1 < sum2 )
                        pOutImage[ty*imgEnWidth+j]=255;         //흰색이 융선

                    ++tempX1;
                    ++tempX2;
                    ++tempX3;
                    ++tempX4;
                    ++tempX5;
                    ++tempX6;
                    ++tempX7;

                    sum1=pGabor[tempY1*imgEnWidth+tempX1]+pGabor[tempY1*imgEnWidth+tempX1];
                    sum2=(pGabor[(tempY1+dy)*imgEnWidth+tempX1+dx]+pGabor[(tempY1-dy)*imgEnWidth+tempX1-dx]);

                    sum1+=pGabor[tempY2*imgEnWidth+tempX2]+pGabor[tempY2*imgEnWidth+tempX2];
                    sum2+=(pGabor[(tempY2+dy)*imgEnWidth+tempX2+dx]+pGabor[(tempY2-dy)*imgEnWidth+tempX2-dx]);

                    sum1+=pGabor[tempY3*imgEnWidth+tempX3]+pGabor[tempY3*imgEnWidth+tempX3];
                    sum2+=(pGabor[(tempY3+dy)*imgEnWidth+tempX3+dx]+pGabor[(tempY3-dy)*imgEnWidth+tempX3-dx]);

                    sum1+=pGabor[tempY4*imgEnWidth+tempX4]+pGabor[tempY4*imgEnWidth+tempX4];
                    sum2+=(pGabor[(tempY4+dy)*imgEnWidth+tempX4+dx]+pGabor[(tempY4-dy)*imgEnWidth+tempX4-dx]);

                    sum1+=pGabor[tempY5*imgEnWidth+tempX5]+pGabor[tempY5*imgEnWidth+tempX5];
                    sum2+=(pGabor[(tempY5+dy)*imgEnWidth+tempX5+dx]+pGabor[(tempY5-dy)*imgEnWidth+tempX5-dx]);

                    sum1+=pGabor[tempY6*imgEnWidth+tempX6]+pGabor[tempY6*imgEnWidth+tempX6];
                    sum2+=(pGabor[(tempY6+dy)*imgEnWidth+tempX6+dx]+pGabor[(tempY6-dy)*imgEnWidth+tempX6-dx]);

                    sum1+=pGabor[tempY7*imgEnWidth+tempX7]+pGabor[tempY7*imgEnWidth+tempX7];
                    sum2+=(pGabor[(tempY7+dy)*imgEnWidth+tempX7+dx]+pGabor[(tempY7-dy)*imgEnWidth+tempX7-dx]);

                    if( sum1 < sum2 )
                        pOutImage[ty*imgEnWidth+j+1]=255;           //흰색이 융선

                    ++tempX1;
                    ++tempX2;
                    ++tempX3;
                    ++tempX4;
                    ++tempX5;
                    ++tempX6;
                    ++tempX7;

                    sum1=pGabor[tempY1*imgEnWidth+tempX1]+pGabor[tempY1*imgEnWidth+tempX1];
                    sum2=(pGabor[(tempY1+dy)*imgEnWidth+tempX1+dx]+pGabor[(tempY1-dy)*imgEnWidth+tempX1-dx]);

                    sum1+=pGabor[tempY2*imgEnWidth+tempX2]+pGabor[tempY2*imgEnWidth+tempX2];
                    sum2+=(pGabor[(tempY2+dy)*imgEnWidth+tempX2+dx]+pGabor[(tempY2-dy)*imgEnWidth+tempX2-dx]);

                    sum1+=pGabor[tempY3*imgEnWidth+tempX3]+pGabor[tempY3*imgEnWidth+tempX3];
                    sum2+=(pGabor[(tempY3+dy)*imgEnWidth+tempX3+dx]+pGabor[(tempY3-dy)*imgEnWidth+tempX3-dx]);

                    sum1+=pGabor[tempY4*imgEnWidth+tempX4]+pGabor[tempY4*imgEnWidth+tempX4];
                    sum2+=(pGabor[(tempY4+dy)*imgEnWidth+tempX4+dx]+pGabor[(tempY4-dy)*imgEnWidth+tempX4-dx]);

                    sum1+=pGabor[tempY5*imgEnWidth+tempX5]+pGabor[tempY5*imgEnWidth+tempX5];
                    sum2+=(pGabor[(tempY5+dy)*imgEnWidth+tempX5+dx]+pGabor[(tempY5-dy)*imgEnWidth+tempX5-dx]);

                    sum1+=pGabor[tempY6*imgEnWidth+tempX6]+pGabor[tempY6*imgEnWidth+tempX6];
                    sum2+=(pGabor[(tempY6+dy)*imgEnWidth+tempX6+dx]+pGabor[(tempY6-dy)*imgEnWidth+tempX6-dx]);

                    sum1+=pGabor[tempY7*imgEnWidth+tempX7]+pGabor[tempY7*imgEnWidth+tempX7];
                    sum2+=(pGabor[(tempY7+dy)*imgEnWidth+tempX7+dx]+pGabor[(tempY7-dy)*imgEnWidth+tempX7-dx]);

                    if( sum1 < sum2 )
                        pOutImage[ty*imgEnWidth+j+2]=255;           //흰색이 융선

                    ++tempX1;
                    ++tempX2;
                    ++tempX3;
                    ++tempX4;
                    ++tempX5;
                    ++tempX6;
                    ++tempX7;

                    sum1=pGabor[tempY1*imgEnWidth+tempX1]+pGabor[tempY1*imgEnWidth+tempX1];
                    sum2=(pGabor[(tempY1+dy)*imgEnWidth+tempX1+dx]+pGabor[(tempY1-dy)*imgEnWidth+tempX1-dx]);

                    sum1+=pGabor[tempY2*imgEnWidth+tempX2]+pGabor[tempY2*imgEnWidth+tempX2];
                    sum2+=(pGabor[(tempY2+dy)*imgEnWidth+tempX2+dx]+pGabor[(tempY2-dy)*imgEnWidth+tempX2-dx]);

                    sum1+=pGabor[tempY3*imgEnWidth+tempX3]+pGabor[tempY3*imgEnWidth+tempX3];
                    sum2+=(pGabor[(tempY3+dy)*imgEnWidth+tempX3+dx]+pGabor[(tempY3-dy)*imgEnWidth+tempX3-dx]);

                    sum1+=pGabor[tempY4*imgEnWidth+tempX4]+pGabor[tempY4*imgEnWidth+tempX4];
                    sum2+=(pGabor[(tempY4+dy)*imgEnWidth+tempX4+dx]+pGabor[(tempY4-dy)*imgEnWidth+tempX4-dx]);

                    sum1+=pGabor[tempY5*imgEnWidth+tempX5]+pGabor[tempY5*imgEnWidth+tempX5];
                    sum2+=(pGabor[(tempY5+dy)*imgEnWidth+tempX5+dx]+pGabor[(tempY5-dy)*imgEnWidth+tempX5-dx]);

                    sum1+=pGabor[tempY6*imgEnWidth+tempX6]+pGabor[tempY6*imgEnWidth+tempX6];
                    sum2+=(pGabor[(tempY6+dy)*imgEnWidth+tempX6+dx]+pGabor[(tempY6-dy)*imgEnWidth+tempX6-dx]);

                    sum1+=pGabor[tempY7*imgEnWidth+tempX7]+pGabor[tempY7*imgEnWidth+tempX7];
                    sum2+=(pGabor[(tempY7+dy)*imgEnWidth+tempX7+dx]+pGabor[(tempY7-dy)*imgEnWidth+tempX7-dx]);

                    if( sum1 < sum2 )
                        pOutImage[ty*imgEnWidth+j+3]=255;           //흰색이 융선
                }
            }
        }
    }

    for (i=D_ENLARGE_BAND; i<imgEnHeight-D_ENLARGE_BAND; i++)
    {
        for (j=D_ENLARGE_BAND; j<imgEnWidth+D_ENLARGE_BAND; j++)
        {
/*          sum1 =  pOutImage[i-1][j-1] +
                    pOutImage[i-1][j] +
                    pOutImage[i][j-1] +
                    pOutImage[i][j] +
                    pOutImage[i][j+1] +
                    pOutImage[i+1][j-1] +
                    pOutImage[i+1][j] +
                    pOutImage[i+1][j+1] +
                    pOutImage[i+1][j+1];
*/
            sum1 =  pOutImage[(i-1)*imgEnWidth+(j-1)]+
                    pOutImage[(i-1)*imgEnWidth+(j)]+
                    pOutImage[(i)*imgEnWidth+(j-1)]+
                    pOutImage[(i)*imgEnWidth+(j)]+
                    pOutImage[(i)*imgEnWidth+(j+1)]+
                    pOutImage[(i+1)*imgEnWidth+(j-1)]+
                    pOutImage[(i+1)*imgEnWidth+(j)]+
                    pOutImage[(i+1)*imgEnWidth+(j+1)]+
                    pOutImage[(i+1)*imgEnWidth+(j+1)];

/*          sum1 =  pOutImage[(i-1)*imgEnWidth+(j-1)] +
                    pOutImage[(i-1)*imgEnWidth+j] +
                    pOutImage[(i-1)*imgEnWidth+(j+1)] +
                    pOutImage[i*imgEnWidth+(j-1)] +
                    pOutImage[i*imgEnWidth+j] +
                    pOutImage[i*imgEnWidth+(j+1)] +
                    pOutImage[(i+1)*imgEnWidth+(j-1)] +
                    pOutImage[(i+1)*imgEnWidth+j] +
                    pOutImage[(i+1)*imgEnWidth+(j+1)];
*/
            if (sum1>(255<<2))
                pOutImage[i*imgEnWidth+j]=255;
            else
                pOutImage[i*imgEnWidth+j]=0;
        }
    }

#ifdef DEBUG_IMAGE
    _SaveBitmapImage("d:\\IB_Binarization.bmp", pOutImage, imgEnWidth, imgEnHeight, FALSE);
#endif
}

void CIBSMAlgorithm::_ALGO_InvertBinarization(const unsigned char *pImage, const unsigned char *pSegBuf, unsigned char *pOutImage, int imgWidth, int imgHeight)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);
    int imgEnHalfWidth = imgEnWidth>>1;
    int imgEnQuadWidth = imgEnHalfWidth>>1;
    int i, j;

    memset(pOutImage, 0, imgEnWidth*imgEnHeight);
    for (i=D_ENLARGE_BAND; i<imgEnHeight-D_ENLARGE_BAND; i++)
    {
        for (j=D_ENLARGE_BAND; j<imgEnWidth-D_ENLARGE_BAND; j++)
        {
            if (pSegBuf[(i>>2)*imgEnQuadWidth+(j>>2)] == 255)
                continue;

            pOutImage[i*imgEnWidth+j] = 255-pImage[i*imgEnWidth+j];
        }
    }

#ifdef DEBUG_IMAGE
    _SaveBitmapImage("d:\\IB_Binarization_Inv.bmp", pOutImage, imgEnWidth, imgEnHeight, FALSE);
#endif
}

void CIBSMAlgorithm::_ALGO_Thinning(unsigned char *pImage, unsigned int *feature, int invert_flag, int imgWidth, int imgHeight, unsigned int *remove_array)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);

/*  int i, j;
    unsigned char point_info;
    int count;
    int endpoint;
    int x,y;
    int value;

    do {
        count = 0;
        for( i=D_ENLARGE_BAND; i<imgEnHeight-D_ENLARGE_BAND; i++ )
        {
            for( j=D_ENLARGE_BAND; j<imgEnWidth-D_ENLARGE_BAND; j++ )
            {
                if( pImage[i*imgEnWidth+j] == 255 )
                {
                    point_info  = pImage[(i-1)*imgEnWidth+(j-1)] & THIN_ONE;
                    point_info |= pImage[(i-1)*imgEnWidth+(j  )] & THIN_TWO;
                    point_info |= pImage[(i-1)*imgEnWidth+(j+1)] & THIN_THREE;
                    point_info |= pImage[(i  )*imgEnWidth+(j-1)] & THIN_EIGHT;
                    point_info |= pImage[(i  )*imgEnWidth+(j+1)] & THIN_FOUR;
                    point_info |= pImage[(i+1)*imgEnWidth+(j-1)] & THIN_SEVEN;
                    point_info |= pImage[(i+1)*imgEnWidth+(j  )] & THIN_SIX;
                    point_info |= pImage[(i+1)*imgEnWidth+(j+1)] & THIN_FIVE;

                    if (g_thinning_live[255-point_info] == 0)
                    {
                        remove_array[count] = i<<16 | j;
                        count++;
                    }
                }
            }
        }

        for( i=0; i<count; i++ )
        {
            y = remove_array[i]>>16;
            x = (remove_array[i]) & 0xFFFF;
            pImage[y*imgEnWidth+x] = 0;
        }
    } while(count);

    for( i=D_ENLARGE_BAND; i<imgEnHeight-D_ENLARGE_BAND; i++ )
    {
        for( j=D_ENLARGE_BAND; j<imgEnWidth-D_ENLARGE_BAND; j++ )
        {
            if (pImage[i*imgEnWidth+j] == 255)
            {
                point_info  = pImage[(i-1)*imgEnWidth+(j-1)] & THIN_SIX;
                point_info |= pImage[(i-1)*imgEnWidth+(j  )] & THIN_SEVEN;
                point_info |= pImage[(i-1)*imgEnWidth+(j+1)] & THIN_EIGHT;
                point_info |= pImage[(i  )*imgEnWidth+(j-1)] & THIN_FIVE;
                point_info |= pImage[(i  )*imgEnWidth+(j+1)] & THIN_ONE;
                point_info |= pImage[(i+1)*imgEnWidth+(j-1)] & THIN_FOUR;
                point_info |= pImage[(i+1)*imgEnWidth+(j  )] & THIN_THREE;
                point_info |= pImage[(i+1)*imgEnWidth+(j+1)] & THIN_TWO;

                pImage[i*imgEnWidth+j] = g_thinning_flag[point_info];
            }
        }
    }

    for( i=D_ENLARGE_BAND; i<imgEnHeight-D_ENLARGE_BAND; i++ )
    {
        for( j=D_ENLARGE_BAND; j<imgEnWidth-D_ENLARGE_BAND; j++ )
        {
            if (pImage[i*imgEnWidth+j] == 255)
            {
                value = pImage[(i-1)*imgEnWidth+(j-1)] +
                        pImage[(i-1)*imgEnWidth+(j  )] +
                        pImage[(i-1)*imgEnWidth+(j+1)] +
                        pImage[(i  )*imgEnWidth+(j-1)] +
                        pImage[(i  )*imgEnWidth+(j+1)] +
                        pImage[(i+1)*imgEnWidth+(j-1)] +
                        pImage[(i+1)*imgEnWidth+(j  )] +
                        pImage[(i+1)*imgEnWidth+(j+1)];

                if( value == 765 &&  // * 255+255+255=765 * /
                    pImage[(i-1)*imgEnWidth+(j-1)] == 0 &&
                    pImage[(i-1)*imgEnWidth+(j+1)] == 0 &&
                    pImage[(i+1)*imgEnWidth+(j-1)] == 0 &&
                    pImage[(i+1)*imgEnWidth+(j+1)] == 0
                )
                {
                    pImage[i*imgEnWidth+j] = 0;
                }
            }
        }
    }
    count = 0;
    for( i=D_ENLARGE_BAND; i<imgEnHeight-D_ENLARGE_BAND; i++ )
    {
        for( j=D_ENLARGE_BAND; j<imgEnWidth-D_ENLARGE_BAND; j++ )
        {
            if (pImage[i*imgEnWidth+j] == 255)
            {
                endpoint = (pImage[(i-1)*imgEnWidth+(j-1)]+
                            pImage[(i-1)*imgEnWidth+(j  )]+
                            pImage[(i-1)*imgEnWidth+(j+1)]+
                            pImage[(i  )*imgEnWidth+(j+1)]+
                            pImage[(i+1)*imgEnWidth+(j+1)]+
                            pImage[(i+1)*imgEnWidth+(j  )]+
                            pImage[(i+1)*imgEnWidth+(j-1)]+
                            pImage[(i  )*imgEnWidth+(j-1)]);
                if (endpoint == 255){
                    count++;
                    feature[count] = i*imgEnWidth+j;
                }
                else if(endpoint == 0)  // added by sklee
                {
                    pImage[i*imgEnWidth+j] = 0;
                }
            }
        }
    }
    feature[0] = count;
*/

    //initial array of point shifts for each direction
    int shift[8]={imgEnWidth-1,imgEnWidth,imgEnWidth+1,1,-imgEnWidth+1,-imgEnWidth,-imgEnWidth-1,-1};
    //initial array to decide which point to delete
    char lifePoint[256]={0,
        1,1,0,1,1,0,0,1,1,0,0,0,1,0,0,1,1,1,1,1,
        1,1,1,0,1,0,0,0,1,0,0,1,1,1,1,1,1,1,1,0,
        1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,0,0,0,
        1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,0,1,1,0,0,1,0,0,0,1,1,1,1,1,1,1,0,
        1,0,0,0,1,0,0,1,0,0,0,1,1,0,0,1,1,0,0,1,
        1,0,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,
        0,0,0,1,1,0,0,0,0,1,1,0,0,1,1,0,0,0,0,1,
        1,0,0,0,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,1,
        1,0,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,
        1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,1,0,
        0,0,0,1,1,0,0,0,0,1,1,0,0,1,1};

    unsigned int pointNumber;
	int i,j;
    int direct;
    int okilNumber;
    unsigned int sequence;
    unsigned int featuresCount=1;

    for (j=D_ENLARGE_BAND*imgEnWidth; j<((imgEnHeight-D_ENLARGE_BAND)*imgEnWidth); j+=imgEnWidth)
    {
        for (i=imgEnWidth-D_ENLARGE_BAND+j; i>=D_ENLARGE_BAND+j; i--)
        {
            pointNumber=i;

            if ( pImage[pointNumber])
            {
                okilNumber=
                    ((unsigned int)(pImage[pointNumber-imgEnWidth-1]&0x01)<<6)|
                    ((unsigned int)(pImage[pointNumber-imgEnWidth]&0x01)<<5)|
                    ((unsigned int)(pImage[pointNumber-imgEnWidth+1]&0x01)<<4)|
                    ((unsigned int)(pImage[pointNumber-1]&0x01)<<7)|
                    ((unsigned int)(pImage[pointNumber+1]&0x01)<<3)|
                    ((unsigned int)(pImage[pointNumber+imgEnWidth-1]&0x01))|
                    ((unsigned int)(pImage[pointNumber+imgEnWidth]&0x01)<<1)|
                    ((unsigned int)(pImage[pointNumber+imgEnWidth+1]&0x01)<<2);

                if (okilNumber!=0)
                {
                    if (!lifePoint[okilNumber])
                    {
                        pImage[pointNumber]=0;
                        direct=7;
                        sequence=0;
                        do {
                            direct=m_RedirectArray[direct*256+okilNumber];
                            pointNumber+=shift[direct];
                            okilNumber=
                                ((unsigned int)(pImage[pointNumber-imgEnWidth-1]&0x01)<<6)|
                                ((unsigned int)(pImage[pointNumber-imgEnWidth]&0x01)<<5)|
                                ((unsigned int)(pImage[pointNumber-imgEnWidth+1]&0x01)<<4)|
                                ((unsigned int)(pImage[pointNumber-1]&0x01)<<7)|
                                ((unsigned int)(pImage[pointNumber+1]&0x01)<<3)|
                                ((unsigned int)(pImage[pointNumber+imgEnWidth-1]&0x01))|
                                ((unsigned int)(pImage[pointNumber+imgEnWidth]&0x01)<<1)|
                                ((unsigned int)(pImage[pointNumber+imgEnWidth+1]&0x01)<<2);

                            pImage[pointNumber]=lifePoint[okilNumber]*255;
                            if(pImage[pointNumber])
                                sequence++;
                            else
                                sequence=0;
                        }while (sequence<40);
                    }
                    else
                    {
                        if (((okilNumber&0x1)+((okilNumber>>1)&0x1)+
                            ((okilNumber>>2)&0x1)+((okilNumber>>3)&0x1)+
                            ((okilNumber>>4)&0x1)+((okilNumber>>5)&0x1)+
                            ((okilNumber>>6)&0x1)+((okilNumber>>7)&0x1))==1){
                            feature[featuresCount]=pointNumber;
                            featuresCount++;
                        }
                    }
                }
                else{
                    pImage[pointNumber]=0;
                }
            }
        }
    }
    feature[0]=featuresCount-1;

#ifdef DEBUG_IMAGE
    if(invert_flag == 0)
        _SaveBitmapImage("d:\\IB_Thinning.bmp", pImage, imgEnWidth, imgEnHeight, FALSE);
    else
        _SaveBitmapImage("d:\\IB_Thinning_Inv.bmp", pImage, imgEnWidth, imgEnHeight, FALSE);
#endif
}

/*
0   1   2
4       3
5   6   7
*/
void CIBSMAlgorithm::_ALGO_MinutiaeAngleCalculation(unsigned char *pImage, unsigned int *pFeature, int invert_flag, int imgWidth, int imgHeight)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);

    int i, j, l;
    int length_count;
    unsigned char direction=0, from_direction;
    int start_x, start_y, end_x, end_y;
    int featurenumber = pFeature[0];
    unsigned int tracing_info[20];
    int tracing_count;//, color_count,change_count;
    int line_count;
    int line_shift[8] = {-imgEnWidth-1, -imgEnWidth, -imgEnWidth+1, 1, -1, imgEnWidth-1, imgEnWidth, imgEnWidth+1};
    unsigned int shift_point;
//  unsigned char point_info;

    for(length_count=3; length_count<=ANGLE_TRACE_LENGTH; length_count++)
    {
        for (j=1; j<=featurenumber; j++)
        {
            tracing_count = 0;
            start_y = (pFeature[j]&0x3fffff)/imgEnWidth;
            start_x = (pFeature[j]&0x3fffff)%imgEnWidth;

            if (pImage[(start_y-1)*imgEnWidth+(start_x-1)] == 255)  direction = 0;
            if (pImage[(start_y-1)*imgEnWidth+(start_x)] == 255)    direction = 1;
            if (pImage[(start_y-1)*imgEnWidth+(start_x+1)] == 255)  direction = 2;
            if (pImage[(start_y)*imgEnWidth+(start_x+1)] == 255)    direction = 3;
            if (pImage[(start_y+1)*imgEnWidth+(start_x+1)] == 255)  direction = 7;
            if (pImage[(start_y+1)*imgEnWidth+(start_x)] == 255)    direction = 6;
            if (pImage[(start_y+1)*imgEnWidth+(start_x-1)] == 255)  direction = 5;
            if (pImage[(start_y)*imgEnWidth+(start_x-1)] == 255)    direction = 4;

            tracing_info[tracing_count]=pFeature[j]&0x3fffff;           //현재 위치 백업
            tracing_count++;
            shift_point = (pFeature[j]&0x3fffff)+line_shift[direction];

            do
            {
                from_direction = 7 - direction;
                start_y = shift_point/imgEnWidth;
                start_x = shift_point%imgEnWidth;

                line_count = 0;

                if (pImage[(start_y-1)*imgEnWidth+(start_x-1)] == 255 && from_direction != 0){
                    direction = 0;  line_count++;
                }
                if (pImage[(start_y-1)*imgEnWidth+(start_x)] == 255 && from_direction != 1){
                    direction = 1;  line_count++;
                }
                if (pImage[(start_y-1)*imgEnWidth+(start_x+1)] == 255 && from_direction != 2){
                    direction = 2;  line_count++;
                }
                if (pImage[(start_y)*imgEnWidth+(start_x+1)] == 255 && from_direction != 3){
                    direction = 3;  line_count++;
                }
                if (pImage[(start_y+1)*imgEnWidth+(start_x+1)] == 255 && from_direction != 7){
                    direction = 7;  line_count++;
                }
                if (pImage[(start_y+1)*imgEnWidth+(start_x)] == 255 && from_direction != 6){
                    direction = 6;  line_count++;
                }
                if (pImage[(start_y+1)*imgEnWidth+(start_x-1)] == 255 && from_direction != 5){
                    direction = 5;  line_count++;
                }
                if (pImage[(start_y)*imgEnWidth+(start_x-1)] == 255 && from_direction != 4){
                    direction = 4;  line_count++;
                }

                tracing_info[tracing_count]=shift_point;            //현재 위치 백업
                tracing_count++;
                shift_point = shift_point+line_shift[direction];

            }while(tracing_count<=length_count && line_count == 1);

            if (tracing_count <= length_count)
            {
                for (i=0; i<tracing_count-1; i++){                                  //현재위치까지 융선 삭제
                    pImage[tracing_info[i]] = 0;
                }

/*              // added by sklee
                for(i=0; i<8; i++)
                {
                    if(pImage[tracing_info[tracing_count-1]+line_shift[i]] == 255)
                    {
                        start_y = (tracing_info[tracing_count-1]+line_shift[i])/imgEnWidth;
                        start_x = (tracing_info[tracing_count-1]+line_shift[i])%imgEnWidth;

                        color_count=0;
                        if(pImage[(start_y-1)*imgEnWidth+(start_x-1)] == 255)   color_count++;
                        if(pImage[(start_y-1)*imgEnWidth+(start_x  )] == 255)   color_count++;
                        if(pImage[(start_y-1)*imgEnWidth+(start_x+1)] == 255)   color_count++;
                        if(pImage[(start_y  )*imgEnWidth+(start_x-1)] == 255)   color_count++;
                        if(pImage[(start_y  )*imgEnWidth+(start_x+1)] == 255)   color_count++;
                        if(pImage[(start_y+1)*imgEnWidth+(start_x-1)] == 255)   color_count++;
                        if(pImage[(start_y+1)*imgEnWidth+(start_x  )] == 255)   color_count++;
                        if(pImage[(start_y+1)*imgEnWidth+(start_x+1)] == 255)   color_count++;

                        if (color_count == 1)
                        {
                            pImage[tracing_info[tracing_count-1]] = 255;
                            i++;
                            break;
                        }
                    }
                }*/

                for (i=j; i<featurenumber; i++)                                 //특징점 정보 삭제
                    pFeature[i]=pFeature[i+1];
                featurenumber--;

                if (line_count==0)                                              //단점을 만난 경우
                {
                    pImage[tracing_info[tracing_count-1]] = 0;

                    for(i=j; i<=featurenumber; i++){                                //그 해당 단점도 삭제
                        if ((pFeature[i]&0x3fffff)==tracing_info[tracing_count-1]){
                            for(l=i; l<featurenumber; l++){
                                pFeature[l]=pFeature[l+1];
                            }
                            featurenumber--;
                        }
                    }
                }
                j--;
            }
            else
            {
                if (length_count == ANGLE_TRACE_LENGTH)                             //13픽셀까지 정보가 존재하는 경우
                {
                    start_y = pFeature[j]/imgEnWidth;
                    start_x = pFeature[j]%imgEnWidth;
                    end_y = tracing_info[tracing_count-1]/imgEnWidth;
                    end_x = tracing_info[tracing_count-1]%imgEnWidth;

                    pFeature[j]|=(((int)((atan2((double)((end_y-start_y)),      //특징점의 각도 계산
                        (double)(-(end_x-start_x)))/3.14159265358979)*180.0)+180)<<22);
                }
            }
        }
    }

    pFeature[0] = featurenumber;
}

void CIBSMAlgorithm::_ALGO_BrokenLineRecover(unsigned char *pImage, unsigned char *pImageInv, unsigned int *pFeature, int invert_flag, int imgWidth, int imgHeight)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);

    int i, j, k;
    int x1, x2, y1, y2, dx, dy;
    int distance, angle;
    int minutiae_angle;
    double degree;
    int index;
    int count;
    int featurenumber;
    int line_info[30];
    int line_count;
    int goout;
    unsigned char   feature_live[1500];
    unsigned int    temp_feature[1500];

    featurenumber = pFeature[0];
    memset(feature_live, 0, 1500);
    memset(temp_feature, 0, sizeof(temp_feature));
    int zz = 0;

    for (i=1; i<=featurenumber-1; i++)
    {
        y1 = (pFeature[i]&0x3fffff)/imgEnWidth;
        x1 = (pFeature[i]&0x3fffff)%imgEnWidth;
        for (j=i+1; j<=featurenumber; j++)
        {
            y2 = (pFeature[j]&0x3fffff)/imgEnWidth;
            x2 = (pFeature[j]&0x3fffff)%imgEnWidth;

            distance = (y2-y1)*(y2-y1)+(x2-x1)*(x2-x1);

            if (distance < 16*16)
            {
                temp_feature[i] = 255;
                temp_feature[j] = 255;
                angle = (pFeature[i]>>22) - (pFeature[j]>>22);

                if (angle < 0)
                    angle = -angle;
                if (angle >= 360)
                    angle -= 360;
                if (angle >= 360)
                    angle -= 360;

                if ( (angle > (180-55) && angle < (180+55)) || distance < (6*6) )
                {
                    degree = (atan2((double)((y1-y2)),(double)((-x1+x2))));             //특징점 간의 각도 계산
                    minutiae_angle=(int)((degree/3.141592)*180.0)+180;

                    minutiae_angle = (minutiae_angle-((pFeature[j]>>22))+180 - 360*((pFeature[j]>>22)+180>=360));

                    if (minutiae_angle < 0)
                        minutiae_angle = -minutiae_angle;
                    if (minutiae_angle >= 360)
                        minutiae_angle -= 360;
                    if (minutiae_angle >= 360)
                        minutiae_angle -= 360;

                    distance=(int)(sqrt((double)(distance)))+1;                         //특징점 간의 거리 계산

                    if ((minutiae_angle < (27*16/distance)) || (minutiae_angle > 360-(27*16/distance)) || (distance<6))     //거리에 따른 거리와 각도 오차범위 내에 존재하면
                    {
                        temp_feature[zz] = pFeature[i];
                        temp_feature[zz+1] = pFeature[j];
                        zz+=2;

                        line_count = 0;
                        goout = 0;
                        for (k=1; k<distance; k++)
                        {
                            dx=(int)(cos(degree)*k);                            //각도에 따른 거리까지의 좌표값 도출
                            dy=(int)(sin(degree)*k);
                            index = (y2+dy)*imgEnWidth-dx+x2;

                            line_info[line_count] = index;
                            line_count++;

                            if ((pImage[(y2+dy)*imgEnWidth+(x2-dx)]!=0)||(pImage[(y2+dy)*imgEnWidth+(x2-dx+1)]!=0)){    //연결이 되었으면 나가기
                                if ((k!=1)&&(k!=distance-1)&&(k!=distance-2)&&(k!=2)){
                                    goout = 1;
                                    break;
                                }
                            }
                        }
                        if (goout == 1)
                            continue;

                        for (k=0; k<line_count; k++)
                        {
                            dy = line_info[k] / imgEnWidth;
                            dx = line_info[k] % imgEnWidth;
                            pImage[dy*imgEnWidth+dx] = 255;                     //연결하기, 미러이미지는 배경으로 바꾸기
                        }
                        feature_live[i] = 255;                                          //삭제할 특징점 마킹
                        feature_live[j] = 255;
                    }
                }
            }
        }
    }
    count = 1;
    for (i=1; i<=featurenumber; i++){                                           //지울 특징점 반영
        if (!feature_live[i])
        {
            pFeature[count]=pFeature[i];
            count++;
        }
    }
    pFeature[0] = count - 1;

#ifdef DEBUG_IMAGE
    if(invert_flag == 0)
        _SaveBitmapImage("d:\\IB_PostThinning.bmp", pImage, imgEnWidth, imgEnHeight, FALSE);
    else
        _SaveBitmapImage("d:\\IB_PostThinning_Inv.bmp", pImage, imgEnWidth, imgEnHeight, FALSE);
#endif
}

void CIBSMAlgorithm::_ALGO_OutsideMinutiaeElimination(unsigned char *pImage, unsigned char *pSegBuf, unsigned int *pFeature, int invert_flag, int imgWidth, int imgHeight)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHalfWidth = imgEnWidth>>1;
    int imgEnQuadWidth = imgEnHalfWidth>>1;

    int i, j;
    unsigned int k;
    int col, row;
    int mask = 2;
    int count = 0;
    int eliminate_flag;
    for (k=1; k<=pFeature[0]; k++)
    {
        row = (pFeature[k]&0x3fffff)/imgEnWidth;
        col = (pFeature[k]&0x3fffff)%imgEnWidth;

        eliminate_flag = 0;
        for(i=-mask; i<=mask; i++)
        {
            for (j=-mask; j<=mask; j++)
            {
                if (pSegBuf[((row>>2)+i)*imgEnQuadWidth+((col>>2)+j)] == 255)
                {
                    eliminate_flag = 1;
                    i = mask + 1;
                    break;
                }
            }
        }
        if (eliminate_flag == 0)
        {
            count++;
            pFeature[count] = pFeature[k];
        }
    }
    pFeature[0] = count;
}

void CIBSMAlgorithm::_ALGO_MinutiaeCombination(unsigned int *pFeature1, unsigned int *pFeature2, int imgWidth, int imgHeight)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    unsigned int i, feature_count;
    unsigned int temp_row, temp_col;
    unsigned int minutiae_coordinate;
    unsigned int featurenumber1=pFeature1[0];
    unsigned int featurenumber2=pFeature2[0];

    // ending point
    feature_count=0;
    for (i=1; i<=featurenumber1; i++)
    {
        if(feature_count>MAX_MINUTIAE){
            pFeature1[0] = MAX_MINUTIAE;
            break;
        }

        minutiae_coordinate = (pFeature1[feature_count+1])&0x3fffff;
        temp_row = (minutiae_coordinate/imgEnWidth)-D_ENLARGE_BAND;
        temp_col = (minutiae_coordinate%imgEnWidth)-D_ENLARGE_BAND;

        minutiae_coordinate = 360 - (pFeature1[feature_count+1]>>22);
        minutiae_coordinate |= temp_row<<9;
        minutiae_coordinate |= temp_col<<20;
        minutiae_coordinate |= 0x80000000;
        pFeature1[feature_count+1]=minutiae_coordinate;
        feature_count++;
    }

    // bifurcation point
    for (i=1; i<=featurenumber2; i++)
    {
        if(feature_count>MAX_MINUTIAE){
            pFeature1[0] = MAX_MINUTIAE;
            break;
        }

        minutiae_coordinate=(pFeature2[i])&0x3fffff;
        temp_row = (minutiae_coordinate/imgEnWidth)-D_ENLARGE_BAND;
        temp_col = (minutiae_coordinate%imgEnWidth)-D_ENLARGE_BAND;

        minutiae_coordinate = 360 - (pFeature2[i]>>22);
        minutiae_coordinate |= temp_row<<9;
        minutiae_coordinate |= temp_col<<20;
        minutiae_coordinate &= 0x7FFFFFFF;
        pFeature1[feature_count+1]=minutiae_coordinate;
        feature_count++;
    }

    pFeature1[0]=feature_count;
    if (pFeature1[0] > MAX_MINUTIAE)
        pFeature1[0] = MAX_MINUTIAE;

    // Angle 순으로 정렬
#ifdef __SCT_ANGLESORT_DEVEL_MODE__
    unsigned int j, temp_value;
    for( i=1; i<pFeature1[0]; i++ )
    {
        for( j=1; j<pFeature1[0]; j++ )
        {
            if( (pFeature1[j]&0x1ff) > (pFeature1[j+1]&0x1ff) )
            {
                temp_value = pFeature1[j];
                pFeature1[j] = pFeature1[j+1];
                pFeature1[j+1] = temp_value;
            }
        }
    }
#endif
}

unsigned char CIBSMAlgorithm::_ALGO_FingerPattern(unsigned char *pSegBuf, unsigned char *pDirBuf, unsigned char *pDirBuf2,
                                               int imgWidth, int imgHeight, unsigned int *feature, IBSM_SINGULAR *IBSM_Singular,
                                               unsigned char *pThreeBin, unsigned char *pLabeledBin, unsigned char *pSingularBin, unsigned char *pTracingBin)
{
    int imgEnWidth = imgWidth + (D_ENLARGE_BAND<<1);
    int imgEnHeight = imgHeight + (D_ENLARGE_BAND<<1);
    int imgEnHalfWidth = imgEnWidth>>1;
    int imgEnHalfHeight = imgEnHeight>>1;
    int imgEnQuadWidth = imgEnHalfWidth>>1;
    int imgEnQuadHeight = imgEnHalfHeight>>1;

    int posx, posy, i;//, j;
    unsigned char Result=0;
    int ori_NumCore=0, ori_NumDelta=0;
    int label_0_cnt=0, label_1_cnt=0, label_2_cnt=0;
    int TracingAngleMinMax[2] = {0,0};

    IB_RECT IB_Rect;

    // initialize
    memset(IBSM_Singular, 0, sizeof(IBSM_SINGULAR));
    memset(pThreeBin, 0, imgEnQuadWidth*imgEnQuadHeight);
    memset(pLabeledBin, 0, imgEnQuadWidth*imgEnQuadHeight);
    memset(pSingularBin, 0, imgEnQuadWidth*imgEnQuadHeight);
    memset(pTracingBin, 0, imgEnQuadWidth*imgEnQuadHeight);

    IB_Rect.left = MAX_IMAGE_W;
    IB_Rect.right = 0;
    IB_Rect.top = MAX_IMAGE_H;
    IB_Rect.bottom = 0;

    for(i=0; i<(int)feature[0]; i++)
    {
        posx = ((feature[i+1]>>20) & 0x7ff);
        posy = ((feature[i+1]>>9) & 0x7ff);

        if(IB_Rect.left > posx) IB_Rect.left = posx;
        if(IB_Rect.right < posx) IB_Rect.right = posx;
        if(IB_Rect.top > posy) IB_Rect.top = posy;
        if(IB_Rect.bottom < posy) IB_Rect.bottom = posy;
    }

    // make three-binary image
    _MakeThreeBinaryImage(pSegBuf, pDirBuf, imgEnQuadWidth, imgEnQuadHeight, pThreeBin, pLabeledBin, &label_0_cnt, &label_1_cnt, &label_2_cnt);

    // find cores and deltas
    _FindCoresAndDeltas(pSegBuf, pThreeBin, pSingularBin, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular);

#ifdef DEBUG_IMAGE
    _SaveBitmapImage_UserSize("d:\\IB_ThreeBinary.bmp", pThreeBin, imgEnQuadWidth, imgEnQuadHeight, imgEnWidth, imgEnHeight, 1, FALSE);
    _SaveBitmapImage_UserSize("d:\\IB_ThreeBinary_Labeled.bmp", pLabeledBin, imgEnQuadWidth, imgEnQuadHeight, imgEnWidth, imgEnHeight, 1, FALSE);
    _SaveBitmapImage_UserSize("d:\\IB_ThreeBinary_Singular.bmp", pSingularBin, imgEnQuadWidth, imgEnQuadHeight, imgEnWidth, imgEnHeight, 1, FALSE);
#endif

    if (IBSM_Singular->NumCore >= MAX_SINGULAR_CNT || IBSM_Singular->NumDelta >= MAX_SINGULAR_CNT)
        return ((D_FP_UNKNOWN_TYPE<<4) | D_FP_UNKNOWN_TYPE);

    ori_NumCore = IBSM_Singular->NumCore;
    ori_NumDelta = IBSM_Singular->NumDelta;

    if(_JudgeArch(pLabeledBin, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular, label_0_cnt, label_1_cnt, label_2_cnt) == 1)
    {
        unsigned char fingertype = _JudgeArchAnother(pThreeBin, pTracingBin, pDirBuf2, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular, IB_Rect);
#ifdef DEBUG_IMAGE
    _SaveBitmapImage_UserSize("d:\\IB_ThreeBinary_Tracing_core.bmp", pTracingBin, imgEnQuadWidth, imgEnQuadHeight, imgEnWidth, imgEnHeight, 1, FALSE);
#endif
        return fingertype;
    }

    Result = _JudgeStrangeWhorl(pDirBuf2, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular);
    if(Result != 0)
        return Result;

    _RemoveCloseCoreDelta(IBSM_Singular);
    _RemoveOutsideCoreDelta(IBSM_Singular, IB_Rect);

    memset(pTracingBin, 0, imgEnQuadWidth*imgEnQuadHeight);
    _FindDeltaCurve(pThreeBin, pTracingBin, pSegBuf, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular, 171, 1, 100, 0);
    _FindDeltaCurve(pThreeBin, pTracingBin, pSegBuf, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular, 1, 86, 150, 1);
    _FindDeltaCurve(pThreeBin, pTracingBin, pSegBuf, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular, 86, 171, 200, 2);
    // tracing length 가 짧은 delta는 지운다.
    _RemoveFalseDelta(IBSM_Singular);

#ifdef DEBUG_IMAGE
    _SaveBitmapImage_UserSize("d:\\IB_ThreeBinary_Tracing_delta.bmp", pTracingBin, imgEnQuadWidth, imgEnQuadHeight, imgEnWidth, imgEnHeight, 1, FALSE);
#endif

    memset(pTracingBin, 0, imgEnQuadWidth*imgEnQuadHeight);
    _FindSingularCurve(pSingularBin, pTracingBin, pThreeBin, pSegBuf, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular, TracingAngleMinMax, 171, 1, 100, 0);
    _FindSingularCurve(pSingularBin, pTracingBin, pThreeBin, pSegBuf, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular, TracingAngleMinMax, 1, 86, 150, 1);
    _FindSingularCurve(pSingularBin, pTracingBin, pThreeBin, pSegBuf, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular, TracingAngleMinMax, 86, 171, 200, 2);
    _RemoveFalseCore(pDirBuf2, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular, IB_Rect);

#ifdef DEBUG_IMAGE
    _SaveBitmapImage_UserSize("d:\\IB_ThreeBinary_Tracing_core.bmp", pTracingBin, imgEnQuadWidth, imgEnQuadHeight, imgEnWidth, imgEnHeight, 1, FALSE);
#endif

    // Core의 방향을 추출한다.
    if(_FindUpperCore(pDirBuf2, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular, 1) == 0)
        return ((D_FP_UNKNOWN_TYPE<<4) | D_FP_UNKNOWN_TYPE);

//  if(_FindUpperCore_Min5_Weight(pDirBuf2, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular) == 0)
//      return ((D_FP_UNKNOWN_TYPE<<4) | D_FP_UNKNOWN_TYPE);

    // 문형을 판별한다.
    Result = _JudgeFingerIndexing(pLabeledBin, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular, IB_Rect, TracingAngleMinMax,
                                    label_0_cnt, label_1_cnt, label_2_cnt, ori_NumCore, ori_NumDelta);

    if((Result>>4) == D_FP_WHORL_TYPE)
    {
        if(IBSM_Singular->NumCore == 2)
        {
            int dx, dy;
            dx = IBSM_Singular->Core[1].Pos.x - IBSM_Singular->Core[0].Pos.x;
            dy = IBSM_Singular->Core[1].Pos.y - IBSM_Singular->Core[0].Pos.y;

            IBSM_Singular->DistCoreToDelta = (unsigned char)sqrt((double)(dx*dx + dy*dy));
        }
    }
    else if(((Result>>4) == D_FP_LEFT_LOOP_TYPE && (Result&0xF) == D_FP_LEFT_LOOP_TYPE) ||
            ((Result>>4) == D_FP_RIGHT_LOOP_TYPE && (Result&0xF) == D_FP_RIGHT_LOOP_TYPE))
    {
        if(IBSM_Singular->NumCore == 1 && IBSM_Singular->NumDelta == 1)
        {
            int dx, dy;
            dx = IBSM_Singular->Delta[0].Pos.x - IBSM_Singular->Core[0].Pos.x;
            dy = IBSM_Singular->Delta[0].Pos.y - IBSM_Singular->Core[0].Pos.y;

            IBSM_Singular->DistCoreToDelta = (unsigned char)sqrt((double)(dx*dx + dy*dy));
        }
    }

    return Result;
}

void CIBSMAlgorithm::_MakeThreeBinaryImage(unsigned char *pSegBuf, unsigned char *pDirBuf, int imgEnQuadWidth, int imgEnQuadHeight,
                                          unsigned char *pThreeBin, unsigned char *pLabeledBin, int *label_0_cnt, int *label_170_cnt, int *label_255_cnt)
{
    int s, t;

    for(s=0; s<imgEnQuadHeight; s++)
    {
        for(t=0; t<imgEnQuadWidth; t++)
        {
            if(pSegBuf[s*imgEnQuadWidth+t] == 0)
                pThreeBin[s*imgEnQuadWidth+t] = 1 + ((pDirBuf[s*imgEnQuadWidth+t]*3>>8)<<8)/3;
            else
                pThreeBin[s*imgEnQuadWidth+t] = 128;
        }
    }

    memcpy(pLabeledBin, pThreeBin, imgEnQuadWidth*imgEnQuadHeight);

    *label_0_cnt = _LabelingUsingColor(pLabeledBin, imgEnQuadWidth, imgEnQuadHeight, 1);
    *label_170_cnt = _LabelingUsingColor(pLabeledBin, imgEnQuadWidth, imgEnQuadHeight, 86);
    *label_255_cnt = _LabelingUsingColor(pLabeledBin, imgEnQuadWidth, imgEnQuadHeight, 171);
}

int CIBSMAlgorithm::_LabelingUsingColor(unsigned char *pLabeledBin, int imgEnQuadWidth, int imgEnQuadHeight, int color)
{
    int label_num=2;//,count=0;
    int y,x;

    if(color==86)
        label_num=102;
    else if(color==171)
        label_num=202;

    for(y=0; y<imgEnQuadHeight; y++)
    {
        for(x=0; x<imgEnQuadWidth; x++)
        {
            if(pLabeledBin[y*imgEnQuadWidth+x] == color)
            {
                _ExpandLabel(pLabeledBin, imgEnQuadWidth, imgEnQuadHeight, x, y, color, label_num);
                if(label_num>30 && label_num<100)
                    label_num=label_num;
                label_num++;
            }
        }
    }
    if(color==86)
        label_num-=100;
    else if(color==171)
        label_num-=200;

    return label_num-2;
}

int CIBSMAlgorithm::_ExpandLabel(unsigned char *pLabeledBin, int imgEnQuadWidth, int imgEnQuadHeight, int x, int y, unsigned char get_color, unsigned char set_color)
{
    int count=0;
    int dx, dy;
    int top;

    top = -1;               // init_stack
    m_pStackBuf[++top] = y;     // push
    m_pStackBuf[++top] = x;     // push

    while( top >= 0 )
    {
        dx = m_pStackBuf[top--];        // pop
        dy = m_pStackBuf[top--];        // pop
        if(dx < 0 || dy < 0 || dx >= imgEnQuadWidth || dy >= imgEnQuadHeight)
            continue;

        pLabeledBin[dy*imgEnQuadWidth+dx] = set_color;
        count++;

        if( pLabeledBin[(dy-1)*imgEnQuadWidth+dx] == get_color )
        {
            m_pStackBuf[++top] = dy-1;      // push
            m_pStackBuf[++top] = dx;        // push
        }
        if( pLabeledBin[dy*imgEnQuadWidth+(dx-1)] == get_color )
        {
            m_pStackBuf[++top] = dy;        // push
            m_pStackBuf[++top] = dx-1;      // push
        }
        if( pLabeledBin[dy*imgEnQuadWidth+(dx+1)]==get_color )
        {
            m_pStackBuf[++top] = dy;        // push
            m_pStackBuf[++top] = dx+1;      // push
        }
        if( pLabeledBin[(dy+1)*imgEnQuadWidth+dx]==get_color )
        {
            m_pStackBuf[++top] = dy+1;      // push
            m_pStackBuf[++top] = dx;        // push
        }
        else
            continue;
    }

    return count;
}

void CIBSMAlgorithm::_FindCoresAndDeltas(unsigned char *pSegBuf, unsigned char *pThreeBin, unsigned char *pSingularBin,
                                        int imgEnQuadWidth, int imgEnQuadHeight, IBSM_SINGULAR *IBSM_Singular)
{
    int i, j, s;
    unsigned char label_c = 0;
    unsigned char label_d = 120;
    int S_MinDist = 1;
    int ZeroCnt, PlusCnt, MinusCnt, diff, value;
    int SingularIndex[8] = {  0,  0,
                            0,  1,
                            1,  1,
                            1,  0};
    int ColorCnt[256];
    memset(ColorCnt, 0, sizeof(ColorCnt));

    for(i=S_MinDist; i<imgEnQuadHeight-S_MinDist; i++)
    {
        for(j=S_MinDist; j<imgEnQuadWidth-S_MinDist; j++)
        {
            if(pSegBuf[(i-S_MinDist)*imgEnQuadWidth+(j-S_MinDist)] == 255 ||
                pSegBuf[(i-S_MinDist)*imgEnQuadWidth+(j)] == 255 ||
                pSegBuf[(i-S_MinDist)*imgEnQuadWidth+(j+S_MinDist)] == 255 ||
                pSegBuf[(i)*imgEnQuadWidth+(j-S_MinDist)] == 255 ||
                pSegBuf[(i)*imgEnQuadWidth+(j)] == 255 ||
                pSegBuf[(i)*imgEnQuadWidth+(j+S_MinDist)] == 255 ||
                pSegBuf[(i+S_MinDist)*imgEnQuadWidth+(j-S_MinDist)] == 255 ||
                pSegBuf[(i+S_MinDist)*imgEnQuadWidth+(j)] == 255 ||
                pSegBuf[(i+S_MinDist)*imgEnQuadWidth+(j+S_MinDist)] == 255)
                continue;

            ZeroCnt = 0;
            PlusCnt = 0;
            MinusCnt = 0;

            for(s=0; s<4; s++)
            {
                if(pThreeBin[(i+SingularIndex[s*2+1])*imgEnQuadWidth+(j+SingularIndex[s*2])] > 0 &&
                    pThreeBin[(i+SingularIndex[(s+1)%4*2+1])*imgEnQuadWidth+(j+SingularIndex[(s+1)%4*2])] > 0 &&
                    pThreeBin[(i+SingularIndex[s*2+1])*imgEnQuadWidth+(j+SingularIndex[s*2])] != 128)
                {
                    diff = pThreeBin[(i+SingularIndex[s*2+1])*imgEnQuadWidth+(j+SingularIndex[s*2])] -
                        pThreeBin[(i+SingularIndex[(s+1)%4*2+1])*imgEnQuadWidth+(j+SingularIndex[(s+1)%4*2])];

                    if(diff < 0)
                        MinusCnt++;
                    else if(diff == 0)
                        ZeroCnt++;
                    else
                        PlusCnt++;
                }
            }

            if(ZeroCnt == 1 && MinusCnt == 2 && PlusCnt == 1)
            {
                if(pSingularBin[(i-1)*imgEnQuadWidth+j-1] == 0 &&
                    pSingularBin[(i-1)*imgEnQuadWidth+j] == 0 &&
                    pSingularBin[(i-1)*imgEnQuadWidth+j+1] == 0 &&
                    pSingularBin[i*imgEnQuadWidth+j-1] == 0)
                {
                    label_c += 10;

                    value = label_c;
                }
                else
                {
                    if(pSingularBin[(i-1)*imgEnQuadWidth+j-1] != 0)
                        value = pSingularBin[(i-1)*imgEnQuadWidth+j-1];
                    else if(pSingularBin[(i-1)*imgEnQuadWidth+j] != 0)
                        value = pSingularBin[(i-1)*imgEnQuadWidth+j];
                    else if(pSingularBin[(i-1)*imgEnQuadWidth+j+1] != 0)
                        value = pSingularBin[(i-1)*imgEnQuadWidth+j+1];
                    else if(pSingularBin[i*imgEnQuadWidth+j-1] != 0)
                        value = pSingularBin[i*imgEnQuadWidth+j-1];
                }

                pSingularBin[i*imgEnQuadWidth+j] = value;
            }

            if(ZeroCnt == 1 && MinusCnt == 1 && PlusCnt == 2)
            {
                if(pSingularBin[(i-1)*imgEnQuadWidth+j-1] == 0 &&
                    pSingularBin[(i-1)*imgEnQuadWidth+j] == 0 &&
                    pSingularBin[(i-1)*imgEnQuadWidth+j+1] == 0 &&
                    pSingularBin[i*imgEnQuadWidth+j-1] == 0)
                {
                    label_d += 10;

                    value = label_d;
                }
                else
                {
                    if(pSingularBin[(i-1)*imgEnQuadWidth+j-1] != 0)
                        value = pSingularBin[(i-1)*imgEnQuadWidth+j-1];
                    else if(pSingularBin[(i-1)*imgEnQuadWidth+j] != 0)
                        value = pSingularBin[(i-1)*imgEnQuadWidth+j];
                    else if(pSingularBin[(i-1)*imgEnQuadWidth+j+1] != 0)
                        value = pSingularBin[(i-1)*imgEnQuadWidth+j+1];
                    else if(pSingularBin[i*imgEnQuadWidth+j-1] != 0)
                        value = pSingularBin[i*imgEnQuadWidth+j-1];
                }

                pSingularBin[i*imgEnQuadWidth+j] = value;
            }
        }
    }

    for(i=0; i<imgEnQuadHeight; i++)
    {
        for(j=0; j<imgEnQuadWidth; j++)
        {
            ColorCnt[pSingularBin[i*imgEnQuadWidth+j]]++;
        }
    }

    for(i=0; i<imgEnQuadHeight; i++)
    {
        for(j=0; j<imgEnQuadWidth; j++)
        {
            if(ColorCnt[pSingularBin[i*imgEnQuadWidth+j]] >= 2)
                pSingularBin[i*imgEnQuadWidth+j] = 0;
        }
    }

    for(i=S_MinDist; i<imgEnQuadHeight-S_MinDist; i++)
    {
        for(j=S_MinDist; j<imgEnQuadWidth-S_MinDist; j++)
        {
            // Core 찾기
            if(pSingularBin[i*imgEnQuadWidth+j] > 0 && pSingularBin[i*imgEnQuadWidth+j] < 120 && (IBSM_Singular->NumCore < MAX_SINGULAR_CNT) )
            {
                IBSM_Singular->Core[IBSM_Singular->NumCore].Pos.x = j;
                IBSM_Singular->Core[IBSM_Singular->NumCore].Pos.y = i;
                IBSM_Singular->NumCore++;

                pSingularBin[i*imgEnQuadWidth+j] = 128;
            }

            // Delta 찾기
            else if(pSingularBin[i*imgEnQuadWidth+j] >= 130 && pSingularBin[i*imgEnQuadWidth+j] < 255 && (IBSM_Singular->NumDelta < MAX_SINGULAR_CNT))
            {
                IBSM_Singular->Delta[IBSM_Singular->NumDelta].Pos.x = j;
                IBSM_Singular->Delta[IBSM_Singular->NumDelta].Pos.y = i;
                IBSM_Singular->NumDelta++;

                pSingularBin[i*imgEnQuadWidth+j] = 255;
            }
        }
    }
}

unsigned char CIBSMAlgorithm::_JudgeArch(unsigned char *pLabelBin, int imgEnQuadWidth, int imgEnQuadHeight,
                                        IBSM_SINGULAR *IBSM_Singular, int label_0_cnt, int label_1_cnt, int label_2_cnt)
{
    int i, j, s, t;
    int MaxCount=0;
    int CountLabel_Black[100], CountLabel_White[100];
    unsigned char label;
    unsigned char MaxAreaLabel_White = 202, MaxAreaLabel_Black = 2;

    if(IBSM_Singular->NumCore != 0 || IBSM_Singular->NumDelta != 0)
        return 0;

    if(label_0_cnt == 0 || label_2_cnt == 0)
        return 0;

    memset(CountLabel_Black, 0, sizeof(CountLabel_Black));
    memset(CountLabel_White, 0, sizeof(CountLabel_Black));

    if(label_0_cnt >= 1)
    {
        for(i=0; i<imgEnQuadHeight; i++)
        {
            for(j=0; j<imgEnQuadWidth; j++)
            {
                label = pLabelBin[i*imgEnQuadWidth+j];
                if(label >= MaxAreaLabel_Black && label < 100)
                    CountLabel_Black[label-MaxAreaLabel_Black]++;
            }
        }

        MaxCount = CountLabel_Black[0];

        for(j=1; j<label_0_cnt; j++)
        {
            if(MaxCount < CountLabel_Black[j])
            {
                MaxCount = CountLabel_Black[j];
                MaxAreaLabel_Black = j+2;
            }
        }
    }

    if(label_2_cnt >= 1)
    {
        for(i=0; i<imgEnQuadHeight; i++)
        {
            for(j=0; j<imgEnQuadWidth; j++)
            {
                label = pLabelBin[i*imgEnQuadWidth+j];
                if(label >= MaxAreaLabel_White)
                    CountLabel_White[label-MaxAreaLabel_White]++;
            }
        }

        MaxCount = CountLabel_White[0];

        for(j=1; j<label_2_cnt; j++)
        {
            if(MaxCount < CountLabel_White[j])
            {
                MaxCount = CountLabel_White[j];
                MaxAreaLabel_White = j+202;
            }
        }
    }

    for(i=1; i<imgEnQuadHeight-1; i++)
    {
        for(j=1; j<imgEnQuadWidth-1; j++)
        {
            if(pLabelBin[i*imgEnQuadWidth+j] == MaxAreaLabel_Black)
            {
                for(s=-1; s<=1; s++)
                {
                    for(t=-1; t<=1; t++)
                    {
                        if(pLabelBin[(i+s)*imgEnQuadWidth+(j+t)] == MaxAreaLabel_White)
                            return 0;
                    }
                }
            }
        }
    }

    return 1;
}

unsigned char CIBSMAlgorithm::_JudgeArchAnother(unsigned char *pThreeBin, unsigned char *pTracingBin, unsigned char *pDirBuf,
                                                int imgEnQuadWidth, int imgEnQuadHeight, IBSM_SINGULAR *IBSM_Singular, IB_RECT IB_Rect)
{
    IB_TRACING *pTracingInfo_B = new IB_TRACING[MAX_TRACING_LINE_CNT];
    IB_TRACING *pTracingInfo_W = new IB_TRACING[MAX_TRACING_LINE_CNT];
    IB_TRACING pTracingInfo_tmp;
    int i, j, s, t, return_value;
    int startx, starty;
    int ColorCount[4], TracingInfoCnt_B, TracingInfoCnt_W, CurveImgEmptyFlag;
    int dx, dy, mindist, dist;
    int min_index_black, min_index_white;
    int BlackStartPos, BlackEndPos, WhiteStartPos, WhiteEndPos;
    unsigned char BlackColor=100, WhiteColor=200;
    unsigned char FingerType = (D_FP_UNKNOWN_TYPE<<4) | D_FP_UNKNOWN_TYPE;

    TracingInfoCnt_B = 0;
    TracingInfoCnt_W = 0;

    memset(pTracingInfo_B, 0, sizeof(IB_TRACING)*MAX_TRACING_LINE_CNT);
    memset(pTracingInfo_W, 0, sizeof(IB_TRACING)*MAX_TRACING_LINE_CNT);
    memset(&pTracingInfo_tmp, 0, sizeof(IB_TRACING));

    memset(pTracingBin, 0, imgEnQuadWidth*imgEnQuadHeight);

    for(i=1; i<imgEnQuadHeight-1; i++)
    {
        for(j=1; j<imgEnQuadWidth-1; j++)
        {
            memset(ColorCount, 0, sizeof(ColorCount));

            for(s=-1; s<=1; s++)
            {
                for(t=-1; t<=1; t++)
                {
                    if(pThreeBin[(i+s)*imgEnQuadWidth+j+t] == 1) ColorCount[0]++;
                    else if(pThreeBin[(i+s)*imgEnQuadWidth+j+t] == 86) ColorCount[1]++;
                    else if(pThreeBin[(i+s)*imgEnQuadWidth+j+t] == 171) ColorCount[2]++;
                    else if(pThreeBin[(i+s)*imgEnQuadWidth+j+t] == 128) ColorCount[3]++;
                }
            }

            if( pThreeBin[i*imgEnQuadWidth+j] == 86 && ColorCount[3] > 0 && ColorCount[1] > 0 && ColorCount[2] > 0 )
            {
                CurveImgEmptyFlag = 0;
                for(s=-2; s<=2; s++)
                {
                    for(t=-2; t<=2; t++)
                    {
                        if(pTracingBin[(i+s)*imgEnQuadWidth+(j+t)] != 0)
                            CurveImgEmptyFlag = 1;
                    }
                }

                if(CurveImgEmptyFlag != 0)
                    continue;

                startx = j;
                starty = i;

                return_value = _FindArchCurve(pThreeBin, pTracingBin, imgEnQuadWidth, imgEnQuadHeight,
                                                startx, starty, &pTracingInfo_B[TracingInfoCnt_B], 171, 86, BlackColor);

                if(return_value == 1)
                {
                    TracingInfoCnt_B++;
                    BlackColor++;
                }
            }
            else if( pThreeBin[i*imgEnQuadWidth+j] == 1 && ColorCount[3] > 0 && ColorCount[0] > 0 && ColorCount[1] > 0)
            {
                CurveImgEmptyFlag = 0;
                for(s=-2; s<=2; s++)
                {
                    for(t=-2; t<=2; t++)
                    {
                        if(pTracingBin[(i+s)*imgEnQuadWidth+(j+t)] != 0)
                            CurveImgEmptyFlag = 1;
                    }
                }

                if(CurveImgEmptyFlag != 0)
                    continue;

                startx = j;
                starty = i;

                return_value = _FindArchCurve(pThreeBin, pTracingBin, imgEnQuadWidth, imgEnQuadHeight,
                                                startx, starty, &pTracingInfo_W[TracingInfoCnt_W], 86, 1, WhiteColor);

                if(return_value == 1){
                    TracingInfoCnt_W++;
                    WhiteColor++;
                }
            }

            if(TracingInfoCnt_B >= MAX_TRACING_LINE_CNT || TracingInfoCnt_W >= MAX_TRACING_LINE_CNT)
            {
                FingerType = (D_FP_UNKNOWN_TYPE<<4) | D_FP_UNKNOWN_TYPE;
                goto exit;
            }
        }
    }

    for(i=0; i<1; i++)
    {
        for(j=i+1; j<TracingInfoCnt_B; j++)
        {
            if(pTracingInfo_B[i].NumPoints < pTracingInfo_B[j].NumPoints)
            {
                pTracingInfo_tmp = pTracingInfo_B[j];
                pTracingInfo_B[j] = pTracingInfo_B[i];
                pTracingInfo_B[i] = pTracingInfo_tmp;
            }
        }

        for(j=i+1; j<TracingInfoCnt_W; j++)
        {
            if(pTracingInfo_W[i].NumPoints < pTracingInfo_W[j].NumPoints)
            {
                pTracingInfo_tmp = pTracingInfo_W[j];
                pTracingInfo_W[j] = pTracingInfo_W[i];
                pTracingInfo_W[i] = pTracingInfo_tmp;
            }
        }
    }

    if(pTracingInfo_B[0].NumPoints == 0 || pTracingInfo_W[0].NumPoints == 0)
    {
        FingerType = (D_FP_UNKNOWN_TYPE<<4) | D_FP_UNKNOWN_TYPE;
        goto exit;
    }

    BlackStartPos = (pTracingInfo_B[0].Points[pTracingInfo_B[0].NumPoints-1].y<<2) - D_ENLARGE_BAND;
    BlackEndPos = (pTracingInfo_B[0].Points[0].y<<2) - D_ENLARGE_BAND;
    WhiteStartPos = (pTracingInfo_W[0].Points[pTracingInfo_W[0].NumPoints-1].y<<2) - D_ENLARGE_BAND;
    WhiteEndPos = (pTracingInfo_W[0].Points[0].y<<2) - D_ENLARGE_BAND;

    if (BlackStartPos > BlackEndPos)
        BlackStartPos = BlackEndPos;
    if (WhiteStartPos > WhiteEndPos)
        WhiteStartPos = WhiteEndPos;

    if(IB_Rect.top > BlackStartPos || IB_Rect.top > WhiteStartPos)
    {
        FingerType = (D_FP_UNKNOWN_TYPE<<4) | D_FP_UNKNOWN_TYPE;
        goto exit;
    }

    mindist = 10000000;
    min_index_black = -1;
    min_index_white = -1;
    for(i=0; i<pTracingInfo_B[0].NumPoints; i++)
    {
        for(j=0; j<pTracingInfo_W[0].NumPoints; j++)
        {
            dx = pTracingInfo_W[0].Points[j].x - pTracingInfo_B[0].Points[i].x;
            dy = pTracingInfo_W[0].Points[j].y - pTracingInfo_B[0].Points[i].y;

//          dist = (int)sqrt((double)(dx*dx + dy*dy));
            dist = m_SqrtTable[abs(dy)][abs(dx)];

            if(mindist > dist)
            {
                mindist = dist;

                min_index_black = i;
                min_index_white = j;
            }
        }
    }

    if(min_index_white != -1 && min_index_black != -1)
    {
        IBSM_Singular->NumCore = 1;
        IBSM_Singular->Core[0].Pos.x = (pTracingInfo_W[0].Points[min_index_white].x + pTracingInfo_B[0].Points[min_index_black].x)/2;
        IBSM_Singular->Core[0].Pos.y = (pTracingInfo_W[0].Points[min_index_white].y + pTracingInfo_B[0].Points[min_index_black].y)/2;
    }

    if(mindist < 10 && IBSM_Singular->NumCore == 1)
    {
        _FindUpperCore_Virtual(pDirBuf, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular);

        if(IBSM_Singular->Core[0].Dir.a[3] > 180 && IBSM_Singular->Core[0].Dir.a[3] < 360)
            FingerType = (D_FP_UNKNOWN_TYPE<<4) | D_FP_UNKNOWN_TYPE;
        else if(IBSM_Singular->Core[0].Dir.a[3] >= 90+30)
            FingerType = (D_FP_ARCH_TYPE<<4) | D_FP_LEFT_LOOP_TYPE;
        else if(IBSM_Singular->Core[0].Dir.a[3] <= 90-30)
            FingerType = (D_FP_ARCH_TYPE<<4) | D_FP_RIGHT_LOOP_TYPE;
        else
            FingerType = (D_FP_UNKNOWN_TYPE<<4) | D_FP_UNKNOWN_TYPE;

        goto exit;
    }

    FingerType = (D_FP_ARCH_TYPE<<4) | D_FP_ARCH_TYPE;

exit:

    delete  pTracingInfo_B;
    delete  pTracingInfo_W;

    return FingerType;
}

int CIBSMAlgorithm::_FindArchCurve(unsigned char *pThreeBin, unsigned char *pTracingBin, int imgEnQuadWidth, int imgEnQuadHeight,
                                 int startx, int starty, IB_TRACING *pTracingInfo, unsigned char EdgeValue1, unsigned char EdgeValue2, unsigned char PaintValue)
{
    int s, SX, SY, Old_SX=0, Old_SY=0;

    int Exit;

    int TraceIndex[16] = { 1,  0,
                           1, -1,
                           0, -1,
                          -1, -1,
                          -1,  0,
                          -1,  1,
                           0,  1,
                           1,  1};
    int CurrentDirection=0;

    memset(pTracingInfo, 0, sizeof(IB_TRACING));

    SX = startx;
    SY = starty;

    pTracingBin[SY*imgEnQuadWidth+SX] = PaintValue;

    CurrentDirection = -1;

    for(s=-1; s<8; s++)
    {
        if(pThreeBin[(SY+TraceIndex[(s+8)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+8)%8*2])] == EdgeValue1 &&
            pThreeBin[(SY+TraceIndex[(s+1)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+1)%8*2])] == EdgeValue2)
        {
            pTracingBin[(SY+TraceIndex[(s+1)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+1)%8*2])] = PaintValue;

            SX += TraceIndex[(s+1)%8*2];
            SY += TraceIndex[(s+1)%8*2+1];

            Old_SX = SX;
            Old_SY = SY;

            pTracingInfo->Points[0].x = SX;
            pTracingInfo->Points[0].y = SY;

            CurrentDirection = (s+1) % 8;

            break;
        }
    }

    if(CurrentDirection == -1)
        return FALSE;

    Exit = 0;
    pTracingInfo->NumPoints = 1;

    while(!Exit)
    {
        for(s=-3; s<=3; s++)
        {
            if(pThreeBin[(SY+TraceIndex[(s+CurrentDirection+8)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+CurrentDirection+8)%8*2])] == EdgeValue1 &&
                pThreeBin[(SY+TraceIndex[(s+1+CurrentDirection+8)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+1+CurrentDirection+8)%8*2])] == EdgeValue2)
            {
                if(pTracingBin[(SY+TraceIndex[(s+1+CurrentDirection+8)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+1+CurrentDirection+8)%8*2])] != 0)
                {
                    Exit = 1;
                    break;
                }

                pTracingBin[(SY+TraceIndex[(s+1+CurrentDirection+8)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+1+CurrentDirection+8)%8*2])] = PaintValue;

                SX += TraceIndex[(s+1+CurrentDirection+8)%8*2];
                SY += TraceIndex[(s+1+CurrentDirection+8)%8*2+1];

                CurrentDirection = (s+CurrentDirection+8)%8;

                Exit = 0;

                pTracingInfo->Points[pTracingInfo->NumPoints].x = SX;
                pTracingInfo->Points[pTracingInfo->NumPoints].y = SY;
                pTracingInfo->NumPoints++;

                if(pTracingInfo->NumPoints >= MAX_TRACING_CNT)
                {
                    Exit = 1;
                }

                if(Old_SX == SX && Old_SY == SY)
                {
                    pTracingBin[(Old_SY-1)*imgEnQuadWidth+(Old_SX-1)] = 0;
                    pTracingBin[(Old_SY-1)*imgEnQuadWidth+Old_SX] = 0;
                    pTracingBin[(Old_SY-1)*imgEnQuadWidth+(Old_SX+1)] = 0;
                    pTracingBin[Old_SY*imgEnQuadWidth+(Old_SX-1)] = 0;
                    pTracingBin[Old_SY*imgEnQuadWidth+Old_SX] = 0;
                    pTracingBin[Old_SY*imgEnQuadWidth+(Old_SX+1)] = 0;
                    pTracingBin[(Old_SY+1)*imgEnQuadWidth+(Old_SX-1)] = 0;
                    pTracingBin[(Old_SY+1)*imgEnQuadWidth+Old_SX] = 0;
                    pTracingBin[(Old_SY+1)*imgEnQuadWidth+(Old_SX+1)] = 0;

                    Exit = 1;
                }

                break;
            }
            else
            {
                Exit = 1;
            }
        }
    }

    if(pTracingInfo->NumPoints < 2)
        return FALSE;

    return TRUE;
}

// 코어의 방향을 판단
int CIBSMAlgorithm::_FindUpperCore_Virtual(unsigned char *pDirBuf, int imgEnQuadWidth, int imgEnQuadHeight, IBSM_SINGULAR *IBSM_Singular)
{
    int i, s, t;
    int /*MinIndex,*/dx, dy, CX, CY, TmpVar;
//    int Candidate_UCoreCnt;
    int DiffAngle, DiffAngleSum_U[16], DiffAngleSum_L[16];
    unsigned char CoreAngle;

    int CoreDirIdx[16][9][2]={
                        {{-4, 0},{-3, 0},{-2, 0},{-1, 0},{ 0, 0},{ 1,  0},{ 2,  0},{ 3,  0},{ 4,  0}},  // 0  0
                        {{-4, 1},{-3, 1},{-2, 1},{-1, 0},{ 0, 0},{ 1,  0},{ 2, -1},{ 3, -1},{ 4, -1}},  // 11.25  0
                        {{-4, 2},{-3, 2},{-2, 1},{-1, 1},{ 0, 0},{ 1, -1},{ 2, -1},{ 3, -2},{ 4, -2}},  // 22.5  0
                        {{-4, 3},{-3, 2},{-2, 2},{-1, 1},{ 0, 0},{ 1, -1},{ 2, -2},{ 3, -2},{ 4, -3}},  // 33.75  0
                        {{-4, 4},{-3, 3},{-2, 2},{-1, 1},{ 0, 0},{ 1, -1},{ 2, -2},{ 3, -3},{ 4, -4}},  // 45  0
                        {{-3, 4},{-2, 3},{-2, 2},{-1, 1},{ 0, 0},{ 1, -1},{ 2, -2},{ 2, -3},{ 3, -4}},  // 56.25  0
                        {{-2, 4},{-2, 3},{-1, 2},{-1, 1},{ 0, 0},{ 1, -1},{ 1, -2},{ 2, -3},{ 2, -4}},  // 67.5  0
                        {{-1, 4},{-1, 3},{-1, 2},{ 0, 1},{ 0, 0},{ 0, -1},{ 1, -2},{ 1, -3},{ 1, -4}},  // 78.75  0
                        {{ 0, 4},{ 0, 3},{ 0, 2},{ 0, 1},{ 0, 0},{ 0, -1},{ 0, -2},{ 0, -3},{ 0, -4}},  // 90  0
                        {{ 1, 4},{ 1, 3},{ 1, 2},{ 0, 1},{ 0, 0},{ 0, -1},{-1, -2},{-1, -3},{-1, -4}},  // 101.25  0
                        {{ 2, 4},{ 2, 3},{ 1, 2},{ 1, 1},{ 0, 0},{-1, -1},{-1, -2},{-2, -3},{-2, -4}},  // 112.5  0
                        {{ 3, 4},{ 2, 3},{ 2, 2},{ 1, 1},{ 0, 0},{-1, -1},{-2, -2},{-2, -3},{-3, -4}},  // 123.75  0
                        {{ 4, 4},{ 3, 3},{ 2, 2},{ 1, 1},{ 0, 0},{-1, -1},{-2, -2},{-3, -3},{-4, -4}},  // 135  0
                        {{ 4, 3},{ 3, 2},{ 2, 2},{ 1, 1},{ 0, 0},{-1, -1},{-2, -2},{-3, -2},{-4, -3}},  // 146.25  0
                        {{ 4, 2},{ 3, 2},{ 2, 1},{ 1, 1},{ 0, 0},{-1, -1},{-2, -1},{-3, -2},{-4, -2}},  // 157.5  0
                        {{ 4, 1},{ 3, 1},{ 2, 1},{ 1, 0},{ 0, 0},{-1,  0},{-2, -1},{-3, -1},{-4, -1}}   // 168.75  0
    };

//    MinIndex = 0;
    //Candidate_UCoreCnt = 0;

    for(i=0; i<IBSM_Singular->NumCore; i++)
    {
        IBSM_Singular->Core[i].Dir.a[3] = 0;

        CX = IBSM_Singular->Core[i].Pos.x;
        CY = IBSM_Singular->Core[i].Pos.y;

        memset(DiffAngleSum_U, 0, 16*4);
        memset(DiffAngleSum_L, 0, 16*4);

        for(s=0; s<16; s++)
        {
            CoreAngle = (int)((double)s * 16);

            for(t=0; t<3; t++)
            {
                dy = CY + CoreDirIdx[s][t][1];
                dx = CX + CoreDirIdx[s][t][0];
                DiffAngle = abs(CoreAngle - pDirBuf[dy*imgEnQuadWidth+dx]);
                if(DiffAngle >= 128)    DiffAngle = 255 - DiffAngle;
                DiffAngleSum_U[s] += DiffAngle;
            }

            for(t=6; t<9; t++)
            {
                dy = CY + CoreDirIdx[s][t][1];
                dx = CX + CoreDirIdx[s][t][0];
                DiffAngle = abs(CoreAngle - pDirBuf[dy*imgEnQuadWidth+dx]);
                if(DiffAngle >= 128)    DiffAngle = 255 - DiffAngle;
                DiffAngleSum_L[s] += DiffAngle;
            }
        }

        TmpVar = DiffAngleSum_L[0];

        for(s=1; s<16; s++)
        {
            if(TmpVar > DiffAngleSum_L[s])
            {
                TmpVar = DiffAngleSum_L[s];

//                MinIndex = s;

                IBSM_Singular->Core[i].Dir.a[3] = (int)((double)s * 11.25);
            }
        }

        for(s=0; s<16; s++)
        {
            if(TmpVar > DiffAngleSum_U[s])
            {
                TmpVar = DiffAngleSum_U[s];

//                MinIndex = s+16;

                IBSM_Singular->Core[i].Dir.a[3] = (int)((double)s * 11.25 + 180);
            }
        }
    }

    return TRUE;
}

unsigned char CIBSMAlgorithm::_JudgeStrangeWhorl(unsigned char *pDirBuf, int imgEnQuadWidth, int imgEnQuadHeight, IBSM_SINGULAR *IBSM_Singular)
{
    int i, j, k;
    int CoreCoreAngle, UCoreDeltaAngle, LCoreDeltaAngle, CoreCoreDist, UCoreDeltaDist, LCoreDeltaDist;
    int DiffAngle1, DiffAngle2, DiffAngle3;
    int dx, dy;

    if(IBSM_Singular->NumCore < 2 || IBSM_Singular->NumDelta < 1)
        return FALSE;

    IBSM_SINGULAR tmpIBSM_Singular;

    memcpy(&tmpIBSM_Singular, IBSM_Singular, sizeof(IBSM_SINGULAR));

    _FindUpperCore(pDirBuf, imgEnQuadWidth, imgEnQuadHeight, &tmpIBSM_Singular, 0);

    for(i=0; i<IBSM_Singular->NumCore; i++)
    {
        if(IBSM_Singular->NumCore >= 3 || IBSM_Singular->NumDelta >= 3)
            continue;

        if(tmpIBSM_Singular.Core[i].Dir.a[3] >= 180 && tmpIBSM_Singular.Core[i].Dir.a[3] <= 360)
            continue;

        for(k=0; k<IBSM_Singular->NumCore; k++)
        {
            if(i == k)
                continue;

            if(IBSM_Singular->NumCore >= 3 || IBSM_Singular->NumDelta >= 3)
                continue;

            if(tmpIBSM_Singular.Core[k].Dir.a[3] > 0 && tmpIBSM_Singular.Core[k].Dir.a[3] < 180)
                continue;

            dx = tmpIBSM_Singular.Core[k].Pos.x - tmpIBSM_Singular.Core[i].Pos.x;
            dy = tmpIBSM_Singular.Core[k].Pos.y - tmpIBSM_Singular.Core[i].Pos.y;

            CoreCoreAngle = _GetAtan2(-dy, dx) * 360 >> 9;

//          CoreCoreDist = (int)sqrt((double)(dx*dx + dy*dy));
            CoreCoreDist = m_SqrtTable[abs(dy)][abs(dx)];

            for(j=0; j<IBSM_Singular->NumDelta; j++)
            {
                dx = tmpIBSM_Singular.Delta[j].Pos.x - tmpIBSM_Singular.Core[i].Pos.x;
                dy = tmpIBSM_Singular.Delta[j].Pos.y - tmpIBSM_Singular.Core[i].Pos.y;

                UCoreDeltaAngle = _GetAtan2(-dy, dx) * 360 >> 9;
//              UCoreDeltaDist = (int)sqrt((double)(dx*dx + dy*dy));
                UCoreDeltaDist = m_SqrtTable[abs(dy)][abs(dx)];

                dx = tmpIBSM_Singular.Delta[j].Pos.x - tmpIBSM_Singular.Core[k].Pos.x;
                dy = tmpIBSM_Singular.Delta[j].Pos.y - tmpIBSM_Singular.Core[k].Pos.y;

                LCoreDeltaAngle = _GetAtan2(-dy, dx) * 360 >> 9;
//              LCoreDeltaDist = (int)sqrt((double)(dx*dx + dy*dy));
                LCoreDeltaDist = m_SqrtTable[abs(dy)][abs(dx)];

                DiffAngle1 = abs(CoreCoreAngle - UCoreDeltaAngle);
                DiffAngle2 = abs(CoreCoreAngle - LCoreDeltaAngle);
                DiffAngle3 = abs(UCoreDeltaAngle - LCoreDeltaAngle);

                if(DiffAngle1 > 180) DiffAngle1 = 360 - DiffAngle1;
                if(DiffAngle2 > 180) DiffAngle2 = 360 - DiffAngle2;
                if(DiffAngle3 > 180) DiffAngle3 = 360 - DiffAngle3;

                if(CoreCoreAngle > 0 && CoreCoreAngle < 180 &&
                    UCoreDeltaDist > LCoreDeltaDist &&
                    DiffAngle1 < 20 && DiffAngle3 < 25 &&
                    CoreCoreDist < 30 && CoreCoreDist > 5 &&
                    LCoreDeltaDist < 13)
                {
                    if(UCoreDeltaAngle < 90 - 10 && UCoreDeltaAngle > 0)
                    {
                        return (D_FP_WHORL_TYPE<<4) | D_FP_RIGHT_LOOP_TYPE;
                    }
                    else if(UCoreDeltaAngle > 90 + 10 && UCoreDeltaAngle < 180)
                    {
                        return (D_FP_WHORL_TYPE<<4) | D_FP_LEFT_LOOP_TYPE;
                    }
                }
            }
        }
    }

    return 0;
}

// 코어의 방향을 판단
int CIBSMAlgorithm::_FindUpperCore(unsigned char *pDirBuf, int imgEnQuadWidth, int imgEnQuadHeight, IBSM_SINGULAR *IBSM_Singular, int Flag)
{
//    int MinIndex=0;
    int i, s, t, dx, dy;//, k, l;
    int CX, CY, TmpVar;
    unsigned char CoreAngle;
    int DiffAngle, DiffAngleSum_U[16], DiffAngleSum_L[16];
    int MinDiffAngleIdx, MinDiffAngleSum;
    int CoreDirIdx[16][9][2]={
                        {{-4, 0},{-3, 0},{-2, 0},{-1, 0},{ 0, 0},{ 1,  0},{ 2,  0},{ 3,  0},{ 4,  0}},  /* 0  0  */
                        {{-4, 1},{-3, 1},{-2, 1},{-1, 0},{ 0, 0},{ 1,  0},{ 2, -1},{ 3, -1},{ 4, -1}},  /* 11.25  0  */
                        {{-4, 2},{-3, 2},{-2, 1},{-1, 1},{ 0, 0},{ 1, -1},{ 2, -1},{ 3, -2},{ 4, -2}},  /* 22.5  0  */
                        {{-4, 3},{-3, 2},{-2, 2},{-1, 1},{ 0, 0},{ 1, -1},{ 2, -2},{ 3, -2},{ 4, -3}},  /* 33.75  0  */
                        {{-4, 4},{-3, 3},{-2, 2},{-1, 1},{ 0, 0},{ 1, -1},{ 2, -2},{ 3, -3},{ 4, -4}},  /* 45  0  */
                        {{-3, 4},{-2, 3},{-2, 2},{-1, 1},{ 0, 0},{ 1, -1},{ 2, -2},{ 2, -3},{ 3, -4}},  /* 56.25  0  */
                        {{-2, 4},{-2, 3},{-1, 2},{-1, 1},{ 0, 0},{ 1, -1},{ 1, -2},{ 2, -3},{ 2, -4}},  /* 67.5  0  */
                        {{-1, 4},{-1, 3},{-1, 2},{ 0, 1},{ 0, 0},{ 0, -1},{ 1, -2},{ 1, -3},{ 1, -4}},  /* 78.75  0  */
                        {{ 0, 4},{ 0, 3},{ 0, 2},{ 0, 1},{ 0, 0},{ 0, -1},{ 0, -2},{ 0, -3},{ 0, -4}},  /* 90  0  */
                        {{ 1, 4},{ 1, 3},{ 1, 2},{ 0, 1},{ 0, 0},{ 0, -1},{-1, -2},{-1, -3},{-1, -4}},  /* 101.25  0  */
                        {{ 2, 4},{ 2, 3},{ 1, 2},{ 1, 1},{ 0, 0},{-1, -1},{-1, -2},{-2, -3},{-2, -4}},  /* 112.5  0  */
                        {{ 3, 4},{ 2, 3},{ 2, 2},{ 1, 1},{ 0, 0},{-1, -1},{-2, -2},{-2, -3},{-3, -4}},  /* 123.75  0  */
                        {{ 4, 4},{ 3, 3},{ 2, 2},{ 1, 1},{ 0, 0},{-1, -1},{-2, -2},{-3, -3},{-4, -4}},  /* 135  0  */
                        {{ 4, 3},{ 3, 2},{ 2, 2},{ 1, 1},{ 0, 0},{-1, -1},{-2, -2},{-3, -2},{-4, -3}},  /* 146.25  0  */
                        {{ 4, 2},{ 3, 2},{ 2, 1},{ 1, 1},{ 0, 0},{-1, -1},{-2, -1},{-3, -2},{-4, -2}},  /* 157.5  0  */
                        {{ 4, 1},{ 3, 1},{ 2, 1},{ 1, 0},{ 0, 0},{-1,  0},{-2, -1},{-3, -1},{-4, -1}}   /* 168.75  0  */
    };

    if(IBSM_Singular->NumCore < 1)
        return FALSE;

    for(i=0; i<IBSM_Singular->NumCore; i++)
    {
        IBSM_Singular->Core[i].Dir.a[3] = 0;

        CX = IBSM_Singular->Core[i].Pos.x;
        CY = IBSM_Singular->Core[i].Pos.y;

        memset(DiffAngleSum_U, 0, 16*4);
        memset(DiffAngleSum_L, 0, 16*4);

        for(s=0; s<16; s++)
        {
            CoreAngle = (int)((double)s * 16);

            for(t=0; t<3; t++)
            {
                dy = CY + CoreDirIdx[s][t][1];
                dx = CX + CoreDirIdx[s][t][0];
                DiffAngle = abs(CoreAngle - pDirBuf[dy*imgEnQuadWidth+dx]);
                if(DiffAngle >= 128) DiffAngle = 255 - DiffAngle;
                DiffAngleSum_U[s] += DiffAngle;
            }

            for(t=6; t<9; t++)
            {
                dy = CY + CoreDirIdx[s][t][1];
                dx = CX + CoreDirIdx[s][t][0];
                DiffAngle = abs(CoreAngle - pDirBuf[dy*imgEnQuadWidth+dx]);
                if(DiffAngle >= 128) DiffAngle = 255 - DiffAngle;
                DiffAngleSum_L[s] += DiffAngle;
            }
        }

//        MinIndex = 0;
        TmpVar = DiffAngleSum_L[0];

        for(s=1; s<16; s++)
        {
            if(TmpVar > DiffAngleSum_L[s])
            {
                TmpVar = DiffAngleSum_L[s];
                //MinIndex = s;
                IBSM_Singular->Core[i].Dir.a[3] = (int)((double)s * 11.25);
            }
        }

        for(s=0; s<16; s++)
        {
            if(TmpVar > DiffAngleSum_U[s])
            {
                TmpVar = DiffAngleSum_U[s];
                //MinIndex = s+16;
                IBSM_Singular->Core[i].Dir.a[3] = (int)((double)s * 11.25 + 180);
            }
        }
    }

    if(Flag == 0)
        return 0;

    // Core가 2개 인 경우, 둘다 Upper 코어로 지정되었을 때 Unknown 처리한다.
    if(IBSM_Singular->NumDelta == 2)
    {
        if(IBSM_Singular->Core[0].Dir.a[3] > 0+45 && IBSM_Singular->Core[0].Dir.a[3] < 180-45 &&
            IBSM_Singular->Core[1].Dir.a[3] > 0+45 && IBSM_Singular->Core[1].Dir.a[3] < 180-45)
        {
            IBSM_Singular->NumCore = 1;
        }
    }

    MinDiffAngleIdx = -1;
    MinDiffAngleSum = 360;
    for(i=0; i<IBSM_Singular->NumCore; i++)
    {
        DiffAngle = abs(90 - IBSM_Singular->Core[i].Dir.a[3]);

        if(MinDiffAngleSum > DiffAngle)
        {
            MinDiffAngleSum = DiffAngle;
            MinDiffAngleIdx = i;
        }
    }

    if(MinDiffAngleIdx < 0 || MinDiffAngleSum > 90)
    {
        IBSM_Singular->NumCore = 0;
        return 0;
    }

    if(IBSM_Singular->NumCore > 1)
    {
        if(MinDiffAngleIdx != 0)
        {
            IB_CORE tmpCore;
            tmpCore = IBSM_Singular->Core[0];
            IBSM_Singular->Core[0] = IBSM_Singular->Core[MinDiffAngleIdx];
            IBSM_Singular->Core[MinDiffAngleIdx] = tmpCore;
        }
    }

    return 1;
}

void CIBSMAlgorithm::_RemoveCloseCoreDelta(IBSM_SINGULAR *IBSM_Singular)
{
    int dx, dy;
    int DistCoreDelta;
    int i, j;
    int DeleteCoreFlag[MAX_SINGULAR_CNT], DeleteDeltaFlag[MAX_SINGULAR_CNT];

    if(IBSM_Singular->NumCore < 1 || IBSM_Singular->NumDelta < 1)
        return;

    IBSM_SINGULAR tmpIBSM_Singular;

    memset(DeleteCoreFlag, 0, sizeof(DeleteCoreFlag));

    for(i=0; i<IBSM_Singular->NumCore; i++)
    {
        memset(DeleteDeltaFlag, 0, sizeof(DeleteDeltaFlag));

        for(j=0; j<IBSM_Singular->NumDelta; j++)
        {
            dx = (IBSM_Singular->Core[i].Pos.x - IBSM_Singular->Delta[j].Pos.x)<<2;
            dy = (IBSM_Singular->Core[i].Pos.y - IBSM_Singular->Delta[j].Pos.y)<<2;

//          DistCoreDelta = (int)sqrt((double)(dx*dx + dy*dy));
            DistCoreDelta = m_SqrtTable[abs(dy)][abs(dx)];

            if(DistCoreDelta < THRES_CLOSE_DISTANCE)
            {
                DeleteCoreFlag[i] = 1;
                DeleteDeltaFlag[j] = 1;
            }
        }

        memset(&tmpIBSM_Singular, 0, sizeof(IBSM_SINGULAR));

        for(j=0; j<IBSM_Singular->NumDelta; j++)
        {
            if(DeleteDeltaFlag[j] == 0)
            {
                tmpIBSM_Singular.Delta[tmpIBSM_Singular.NumDelta] = IBSM_Singular->Delta[j];
                tmpIBSM_Singular.NumDelta++;
            }
        }

        IBSM_Singular->NumDelta = tmpIBSM_Singular.NumDelta;
        for(j=0; j<tmpIBSM_Singular.NumDelta; j++)
            IBSM_Singular->Delta[j] = tmpIBSM_Singular.Delta[j];
    }

    tmpIBSM_Singular.NumCore = 0;
    for(j=0; j<IBSM_Singular->NumCore; j++)
    {
        if(DeleteCoreFlag[j] == 0)
        {
            tmpIBSM_Singular.Core[tmpIBSM_Singular.NumCore] = IBSM_Singular->Core[j];
            tmpIBSM_Singular.NumCore++;
        }
    }

    IBSM_Singular->NumCore = tmpIBSM_Singular.NumCore;
    for(j=0; j<tmpIBSM_Singular.NumCore; j++)
        IBSM_Singular->Core[j] = tmpIBSM_Singular.Core[j];
}

void CIBSMAlgorithm::_RemoveOutsideCoreDelta(IBSM_SINGULAR *IBSM_Singular, IB_RECT IB_Rect)
{
    int RealCorePosX, RealCorePosY, Bound=10;
    int i;

    IBSM_SINGULAR tmpIBSM_Singular;

    memset(&tmpIBSM_Singular, 0, sizeof(IBSM_SINGULAR));

    for(i=0; i<IBSM_Singular->NumCore; i++)
    {
        RealCorePosX = (IBSM_Singular->Core[i].Pos.x<<2) - D_ENLARGE_BAND;
        RealCorePosY = (IBSM_Singular->Core[i].Pos.y<<2) - D_ENLARGE_BAND;

        if((RealCorePosX >= IB_Rect.left+Bound) && (RealCorePosX <= IB_Rect.right-Bound) &&
            (RealCorePosY >= IB_Rect.top+Bound) && (RealCorePosY <= IB_Rect.bottom-Bound))
        {
            tmpIBSM_Singular.Core[tmpIBSM_Singular.NumCore] = IBSM_Singular->Core[i];
            tmpIBSM_Singular.NumCore++;
        }
    }

    for(i=0; i<IBSM_Singular->NumDelta; i++)
    {
        RealCorePosX = (IBSM_Singular->Delta[i].Pos.x<<2) - D_ENLARGE_BAND;
        RealCorePosY = (IBSM_Singular->Delta[i].Pos.y<<2) - D_ENLARGE_BAND;

        if((RealCorePosX >= IB_Rect.left+Bound) && (RealCorePosX <= IB_Rect.right-Bound) &&
            (RealCorePosY >= IB_Rect.top+Bound) && (RealCorePosY <= IB_Rect.bottom-Bound))
        {
            tmpIBSM_Singular.Delta[tmpIBSM_Singular.NumDelta] = IBSM_Singular->Delta[i];
            tmpIBSM_Singular.NumDelta++;
        }
    }

    memcpy(IBSM_Singular, &tmpIBSM_Singular, sizeof(IBSM_SINGULAR));
}

void CIBSMAlgorithm::_FindDeltaCurve(unsigned char *pThreeBin, unsigned char *pTracingBin, unsigned char *pSegBuf, int imgEnQuadWidth, int imgEnQuadHeight, IBSM_SINGULAR *IBSM_Singular,
                    unsigned char EdgeValue1, unsigned char EdgeValue2, unsigned char PaintValue, int DeltaFlag)
{
    int s, i, j, SX, SY, Old_SX=0, Old_SY=0, TracingCount;
    int SameTime;
    int Exit;
    int TraceIndex[16] = { 1,  0,
                           1, -1,
                           0, -1,
                          -1, -1,
                          -1,  0,
                          -1,  1,
                           0,  1,
                           1,  1};
    int CurrentDirection=0;

    for(i=0; i<IBSM_Singular->NumDelta; i++)
    {
        SameTime = 0;

        SX = IBSM_Singular->Delta[i].Pos.x;
        SY = IBSM_Singular->Delta[i].Pos.y;

        pTracingBin[SY*imgEnQuadWidth+SX] = PaintValue;

        CurrentDirection = -1;

        for(s=-1; s<8; s++)
        {
            if(pThreeBin[(SY+TraceIndex[(s+8)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+8)%8*2])] == EdgeValue2 &&
                pThreeBin[(SY+TraceIndex[(s+1)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+1)%8*2])] == EdgeValue1)
            {
                pTracingBin[(SY+TraceIndex[(s+1)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+1)%8*2])] = PaintValue;

                SX += TraceIndex[(s+1)%8*2];
                SY += TraceIndex[(s+1)%8*2+1];

                Old_SX = SX;
                Old_SY = SY;

                CurrentDirection = (s+1) % 8;

                break;
            }
        }

        if(pSegBuf[SY*imgEnQuadWidth+SX] == 255 || CurrentDirection == -1)
            continue;

        Exit = 0;
        TracingCount = 1;

        while(!Exit)
        {
            for(s=-3; s<=3; s++)
            {
                if(pThreeBin[(SY+TraceIndex[(s+CurrentDirection+8)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+CurrentDirection+8)%8*2])] == EdgeValue2 &&
                    pThreeBin[(SY+TraceIndex[(s+1+CurrentDirection+8)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+1+CurrentDirection+8)%8*2])] == EdgeValue1)
                {
                    pTracingBin[(SY+TraceIndex[(s+1+CurrentDirection+8)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+1+CurrentDirection+8)%8*2])] = PaintValue;

                    SX += TraceIndex[(s+1+CurrentDirection+8)%8*2];
                    SY += TraceIndex[(s+1+CurrentDirection+8)%8*2+1];

                    CurrentDirection = (s+CurrentDirection+8)%8;

                    Exit = 0;

                    TracingCount++;

                    if(TracingCount > MAX_TRACING_CNT)
                    {
                        Exit = 1;
                    }

                    if(Old_SX == SX && Old_SY == SY)
                    {
                        Exit = 1;
                    }

                    for(j=0; j<IBSM_Singular->NumCore; j++)
                    {
                        if(IBSM_Singular->Core[j].Pos.x <= SX+1 && IBSM_Singular->Core[j].Pos.x >= SX-1 &&
                            IBSM_Singular->Core[j].Pos.y <= SY+1 && IBSM_Singular->Core[j].Pos.y >= SY-1)
                        {
                            SameTime = 1;
                            break;
                        }
                    }

                    if(SameTime == 1)
                    {
                        SameTime = 0;
                        Exit = 1;
                    }

                    break;
                }
                else
                {
                    Exit = 1;
                }
            }
        }

        IBSM_Singular->Delta[i].Len.l[DeltaFlag] = TracingCount;
    }
}

void CIBSMAlgorithm::_RemoveFalseDelta(IBSM_SINGULAR *IBSM_Singular)
{
    int i;
    int Flag;

    IBSM_SINGULAR tmpIBSM_Singular;

    memset(&tmpIBSM_Singular, 0, sizeof(IBSM_SINGULAR));

    for(i=0; i<IBSM_Singular->NumDelta; i++)
    {
        Flag = 0;

        if(IBSM_Singular->Delta[i].Len.l[0] < 5) Flag++;
        if(IBSM_Singular->Delta[i].Len.l[1] < 5) Flag++;
        if(IBSM_Singular->Delta[i].Len.l[2] < 5) Flag++;

        if(Flag < 2)
        {
            tmpIBSM_Singular.Delta[tmpIBSM_Singular.NumDelta] = IBSM_Singular->Delta[i];
            tmpIBSM_Singular.NumDelta++;
        }
    }

    memcpy(IBSM_Singular->Delta, tmpIBSM_Singular.Delta, sizeof(IB_DELTA)*tmpIBSM_Singular.NumDelta);
    IBSM_Singular->NumDelta = tmpIBSM_Singular.NumDelta;
}

void CIBSMAlgorithm::_FindSingularCurve(unsigned char *pSingularBin,unsigned char *pTracingBin, unsigned char *pThreeBin, unsigned char *pSegBuf, int imgEnQuadWidth, int imgEnQuadHeight,
                        IBSM_SINGULAR *IBSM_Singular, int *TracingAngleMinMax, unsigned char EdgeValue1, unsigned char EdgeValue2, unsigned char PaintValue, int CoreFlag)
{
    int s, i, j, SX, SY, Old_SX=0, Old_SY=0, TracingCount, TmpAngle;
    int SameTime, back=0, g, h, Exit;
    int TraceIndex[16] = { 1,  0,
                           1, -1,
                           0, -1,
                          -1, -1,
                          -1,  0,
                          -1,  1,
                           0,  1,
                           1,  1};
    int CurrentDirection=0;

    if(IBSM_Singular->NumCore == 1 && IBSM_Singular->NumDelta <= 1 && CoreFlag == 0)
    {
        TracingAngleMinMax[0] = 10000;
        TracingAngleMinMax[1] = -10000;
    }

    for(i=0; i<IBSM_Singular->NumCore; i++)
    {
        SameTime = 0;

        SX = IBSM_Singular->Core[i].Pos.x;
        SY = IBSM_Singular->Core[i].Pos.y;

        pTracingBin[SY*imgEnQuadWidth+SX] = PaintValue;

        CurrentDirection = -1;

        for(s=-1; s<8; s++)
        {
            if(pThreeBin[(SY+TraceIndex[(s+8)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+8)%8*2])] == EdgeValue1 &&
                pThreeBin[(SY+TraceIndex[(s+1)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+1)%8*2])] == EdgeValue2)
            {
                pTracingBin[(SY+TraceIndex[(s+1)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+1)%8*2])] = PaintValue;

                SX += TraceIndex[(s+1)%8*2];
                SY += TraceIndex[(s+1)%8*2+1];

                Old_SX = SX;
                Old_SY = SY;

                CurrentDirection = (s+1) % 8;

                break;
            }
        }

        if(pSegBuf[SY*imgEnQuadWidth+SX] == 255 || CurrentDirection == -1)
            continue;

        Exit = 0;
        TracingCount = 1;

        while(!Exit)
        {
            for(s=-3; s<=3; s++)
            {
                if(pThreeBin[(SY+TraceIndex[(s+CurrentDirection+8)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+CurrentDirection+8)%8*2])] == EdgeValue1 &&
                    pThreeBin[(SY+TraceIndex[(s+1+CurrentDirection+8)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+1+CurrentDirection+8)%8*2])] == EdgeValue2)
                {
                    pTracingBin[(SY+TraceIndex[(s+1+CurrentDirection+8)%8*2+1])*imgEnQuadWidth+(SX+TraceIndex[(s+1+CurrentDirection+8)%8*2])] = PaintValue;

                    SX += TraceIndex[(s+1+CurrentDirection+8)%8*2];
                    SY += TraceIndex[(s+1+CurrentDirection+8)%8*2+1];

                    if(TraceIndex[(s+1+CurrentDirection+8)%8*2] < 0)
                    {
                        IBSM_Singular->Core[i].Pattern.p[CoreFlag*4]++;
                    }
                    else if(TraceIndex[(s+1+CurrentDirection+8)%8*2] > 0)
                    {
                        IBSM_Singular->Core[i].Pattern.p[CoreFlag*4+1]++;
                    }

                    if(TraceIndex[(s+1+CurrentDirection+8)%8*2+1] < 0)
                    {
                        IBSM_Singular->Core[i].Pattern.p[CoreFlag*4+2]++;
                    }
                    else if(TraceIndex[(s+1+CurrentDirection+8)%8*2+1] > 0)
                    {
                        IBSM_Singular->Core[i].Pattern.p[CoreFlag*4+3]++;
                    }

                    CurrentDirection = (s+CurrentDirection+8)%8;
                    Exit = 0;
                    TracingCount++;

                    if(TracingCount > MAX_TRACING_CNT)
                    {
                        Exit = 1;
                    }

                    if(Old_SX == SX && Old_SY == SY)
                    {
                        pTracingBin[(Old_SY-1)*imgEnQuadWidth+(Old_SX-1)] = 0;
                        pTracingBin[(Old_SY-1)*imgEnQuadWidth+Old_SX] = 0;
                        pTracingBin[(Old_SY-1)*imgEnQuadWidth+(Old_SX+1)] = 0;
                        pTracingBin[Old_SY*imgEnQuadWidth+(Old_SX-1)] = 0;
                        pTracingBin[Old_SY*imgEnQuadWidth+Old_SX] = 0;
                        pTracingBin[Old_SY*imgEnQuadWidth+(Old_SX+1)] = 0;
                        pTracingBin[(Old_SY+1)*imgEnQuadWidth+(Old_SX-1)] = 0;
                        pTracingBin[(Old_SY+1)*imgEnQuadWidth+Old_SX] = 0;
                        pTracingBin[(Old_SY+1)*imgEnQuadWidth+(Old_SX+1)] = 0;

                        Exit = 1;
                    }

                    for(j=0; j<IBSM_Singular->NumDelta; j++)
                    {
                        if(IBSM_Singular->Delta[j].Pos.x <= SX+1 && IBSM_Singular->Delta[j].Pos.x >= SX-1 &&
                            IBSM_Singular->Delta[j].Pos.y <= SY+1 && IBSM_Singular->Delta[j].Pos.y >= SY-1)
                        {
                            IBSM_Singular->Core[i].Meet.m[j]++;
                            SameTime = 1;
                        }
                    }

                    if(SameTime == 1)
                    {
                        SameTime = 0;
                        Exit = 1;
                    }

                    if(TracingCount > 5 && IBSM_Singular->NumCore == 1 && IBSM_Singular->NumDelta <= 1 && CoreFlag == 0)
                    {
                        TmpAngle = _GetAtan2(-(SY - Old_SY), SX - Old_SX) * 360 >> 9;

                        if(TracingAngleMinMax[0] > TmpAngle)
                            TracingAngleMinMax[0] = TmpAngle;
                        if(TracingAngleMinMax[1] < TmpAngle)
                            TracingAngleMinMax[1] = TmpAngle;
                    }

                    break;
                }
                else
                {
                    Exit = 1;
                }
            }
        }

        back=0;
        for(g=-3; g<=3; g++)
        {
            for(h=-3; h<=3; h++)
            {
                if(pSegBuf[(SY+g)*imgEnQuadWidth+(SX+h)] == 255 || pSingularBin[(SY+g)*imgEnQuadWidth+(SX+h)] == 255)
                {
                    back=1;
                    break;
                }
            }
        }

        IBSM_Singular->Core[i].Meet.m[CoreFlag] = back;

        if(TracingCount >= 1)
        {
            IBSM_Singular->Core[i].Dir.a[CoreFlag] = _GetAtan2(-(SY - Old_SY), SX - Old_SX) * 360 >> 9;
            IBSM_Singular->Core[i].Len.l[CoreFlag] = TracingCount;
        }
        else
        {
            IBSM_Singular->Core[i].Dir.a[CoreFlag] = 0;
            IBSM_Singular->Core[i].Len.l[CoreFlag] = 0;
        }
    }
}

void CIBSMAlgorithm::_RemoveFalseCore(unsigned char *pDirBuf, int imgEnQuadWidth, int imgEnQuadHeight, IBSM_SINGULAR *IBSM_Singular, IB_RECT IB_Rect)
{
    ////////////////////////////////////////////////////////////////////////////////////////
    int MaxAngleDiff[10], AngleDiff;
    int i, s, t;
    float a;
    int dx, dy;
    int b1, b2, result1, result2;
    int DeleteFlag = 0, LowQualityIndex, HighQualityIndex;
    int MinCoreDist, MinDeltaDist, DistCoreToCore, DistDeltaToCore;
    int RealCorePosX, RealCorePosY, RealDeltaPosX, RealDeltaPosY;

    IBSM_SINGULAR tmpIBSM_Singular;

    memset(&tmpIBSM_Singular, 0, sizeof(IBSM_SINGULAR));
    memset(MaxAngleDiff, 0, 10*4);

    for(i=0; i<IBSM_Singular->NumCore; i++)
    {
        for(s=0; s<3; s++)
        {
            for(t=1; t<4; t++)
            {
                AngleDiff = abs(IBSM_Singular->Core[i].Dir.a[s] - IBSM_Singular->Core[i].Dir.a[t]);
                if(AngleDiff > 180) AngleDiff = 360 - AngleDiff;

                if(MaxAngleDiff[i] < AngleDiff)
                    MaxAngleDiff[i] = AngleDiff;
            }
        }
    }

    for(i=0; i<IBSM_Singular->NumCore; i++)
    {
        if(MaxAngleDiff[i] > 90)
        {
            tmpIBSM_Singular.Core[tmpIBSM_Singular.NumCore] = IBSM_Singular->Core[i];
            tmpIBSM_Singular.NumCore++;
        }
    }
    IBSM_Singular->NumCore = tmpIBSM_Singular.NumCore;

    ////////////////////////////////////////////////////////////////////////////////////////

    if(IBSM_Singular->NumCore != 2 || IBSM_Singular->NumDelta != 1)
        return;

    dx = IBSM_Singular->Core[0].Pos.x - IBSM_Singular->Core[1].Pos.x;
    dy = IBSM_Singular->Core[0].Pos.y - IBSM_Singular->Core[1].Pos.y;

    a = - 1.0f / ((float)dy / (float)dx);

    b1 = IBSM_Singular->Core[0].Pos.y - (int)(a * IBSM_Singular->Core[0].Pos.x);
    b2 = IBSM_Singular->Core[1].Pos.y - (int)(a * IBSM_Singular->Core[1].Pos.x);

    result1 = (int)(a * IBSM_Singular->Delta[0].Pos.x) + b1 - IBSM_Singular->Delta[0].Pos.y;
    result2 = (int)(a * IBSM_Singular->Delta[0].Pos.x) + b2 - IBSM_Singular->Delta[0].Pos.y;

    if(b1 > b2)
    {
        if(result1 > 0 && result2 < 0)
        {
            DeleteFlag = 1;
        }
    }
    else if(b2 > b1)
    {
        if(result2 > 0 && result1 < 0)
        {
            DeleteFlag = 1;
        }
    }

    if(DeleteFlag == 0)
        return;

    HighQualityIndex = 0;
    LowQualityIndex = 1;

    memset(&tmpIBSM_Singular, 0, sizeof(IBSM_SINGULAR));
    tmpIBSM_Singular.NumCore = IBSM_Singular->NumCore;
    for(i=0; i<tmpIBSM_Singular.NumCore; i++)
        tmpIBSM_Singular.Core[i].Pos = IBSM_Singular->Core[i].Pos;

    _FindUpperCore(pDirBuf, imgEnQuadWidth, imgEnQuadHeight, &tmpIBSM_Singular, 0);

    if(tmpIBSM_Singular.Core[LowQualityIndex].Dir.a[3] > 0 && tmpIBSM_Singular.Core[LowQualityIndex].Dir.a[3] < 180)
        return;

    RealCorePosX = (tmpIBSM_Singular.Core[LowQualityIndex].Pos.x<<2) - D_ENLARGE_BAND;
    RealCorePosY = (tmpIBSM_Singular.Core[LowQualityIndex].Pos.y<<2) - D_ENLARGE_BAND;

    RealDeltaPosX = (tmpIBSM_Singular.Delta[0].Pos.x<<2) - D_ENLARGE_BAND;
    RealDeltaPosY = (tmpIBSM_Singular.Delta[0].Pos.y<<2) - D_ENLARGE_BAND;

    MinCoreDist = RealCorePosX - IB_Rect.left;
    if(MinCoreDist > (IB_Rect.right - RealCorePosX))
        MinCoreDist = (IB_Rect.right - RealCorePosX);
    if(MinCoreDist > (RealCorePosY - IB_Rect.top))
        MinCoreDist = (RealCorePosY - IB_Rect.top);
    if(MinCoreDist > (IB_Rect.bottom - RealCorePosY))
        MinCoreDist = (IB_Rect.bottom - RealCorePosY);

    MinDeltaDist = RealDeltaPosX - IB_Rect.left;
    if(MinDeltaDist > (IB_Rect.right - RealDeltaPosX))
        MinDeltaDist = (IB_Rect.right - RealDeltaPosX);
    if(MinDeltaDist > (RealDeltaPosY - IB_Rect.top))
        MinDeltaDist = (RealDeltaPosY - IB_Rect.top);
    if(MinDeltaDist > (IB_Rect.bottom - RealDeltaPosY))
        MinDeltaDist = (IB_Rect.bottom - RealDeltaPosY);

    if(MinCoreDist > MinDeltaDist)
        return;

    dx = IBSM_Singular->Core[HighQualityIndex].Pos.x - IBSM_Singular->Core[LowQualityIndex].Pos.x;
    dy = IBSM_Singular->Core[HighQualityIndex].Pos.y - IBSM_Singular->Core[LowQualityIndex].Pos.y;
//  DistCoreToCore = (int)sqrt((double)dx*dx + dy*dy);
    DistCoreToCore = m_SqrtTable[abs(dy)][abs(dx)];

    dx = IBSM_Singular->Core[HighQualityIndex].Pos.x - IBSM_Singular->Delta[0].Pos.x;
    dy = IBSM_Singular->Core[HighQualityIndex].Pos.y - IBSM_Singular->Delta[0].Pos.y;
//  DistDeltaToCore = (int)sqrt((double)dx*dx + dy*dy);
    DistDeltaToCore = m_SqrtTable[abs(dy)][abs(dx)];

    if(DistDeltaToCore > DistCoreToCore)
        return;

    if(IBSM_Singular->Core[LowQualityIndex].Dir.a[3] > 180 && IBSM_Singular->Core[LowQualityIndex].Dir.a[3] < 360)
        return;

    if(LowQualityIndex == 1)
    {
        IBSM_Singular->NumCore = 1;
        return;
    }

    IB_CORE tmpCore;

    tmpCore = IBSM_Singular->Core[0];
    IBSM_Singular->Core[0] = IBSM_Singular->Core[1];
    IBSM_Singular->Core[1] = tmpCore;
    IBSM_Singular->NumCore = 1;
}

// 코어의 방향을 판단
int CIBSMAlgorithm::_FindUpperCore_Min5_Weight(unsigned char *pDirBuf, int imgEnQuadWidth, int imgEnQuadHeight, IBSM_SINGULAR *IBSM_Singular)
{
//    int MinIndex;
    int i, s, t;
    int CX, CY, Dir;
    unsigned char CoreAngle;
    unsigned char DiffAngleSum_Flag[32];
    int DiffAngle, DiffAngleSum[32];
    int DiffAngleSum_Index[32];
    int TmpVar,TmpVar2, TmpVarSum;
    int mag = 3;
    int dx, dy;
    int CoreDirIdx[16][9][2]={
                        {{-4, 0},{-3, 0},{-2, 0},{-1, 0},{ 0, 0},{ 1,  0},{ 2,  0},{ 3,  0},{ 4,  0}},  /* 0  0  */
                        {{-4, 1},{-3, 1},{-2, 1},{-1, 0},{ 0, 0},{ 1,  0},{ 2, -1},{ 3, -1},{ 4, -1}},  /* 11.25  0  */
                        {{-4, 2},{-3, 2},{-2, 1},{-1, 1},{ 0, 0},{ 1, -1},{ 2, -1},{ 3, -2},{ 4, -2}},  /* 22.5  0  */
                        {{-4, 3},{-3, 2},{-2, 2},{-1, 1},{ 0, 0},{ 1, -1},{ 2, -2},{ 3, -2},{ 4, -3}},  /* 33.75  0  */
                        {{-4, 4},{-3, 3},{-2, 2},{-1, 1},{ 0, 0},{ 1, -1},{ 2, -2},{ 3, -3},{ 4, -4}},  /* 45  0  */
                        {{-3, 4},{-2, 3},{-2, 2},{-1, 1},{ 0, 0},{ 1, -1},{ 2, -2},{ 2, -3},{ 3, -4}},  /* 56.25  0  */
                        {{-2, 4},{-2, 3},{-1, 2},{-1, 1},{ 0, 0},{ 1, -1},{ 1, -2},{ 2, -3},{ 2, -4}},  /* 67.5  0  */
                        {{-1, 4},{-1, 3},{-1, 2},{ 0, 1},{ 0, 0},{ 0, -1},{ 1, -2},{ 1, -3},{ 1, -4}},  /* 78.75  0  */
                        {{ 0, 4},{ 0, 3},{ 0, 2},{ 0, 1},{ 0, 0},{ 0, -1},{ 0, -2},{ 0, -3},{ 0, -4}},  /* 90  0  */
                        {{ 1, 4},{ 1, 3},{ 1, 2},{ 0, 1},{ 0, 0},{ 0, -1},{-1, -2},{-1, -3},{-1, -4}},  /* 101.25  0  */
                        {{ 2, 4},{ 2, 3},{ 1, 2},{ 1, 1},{ 0, 0},{-1, -1},{-1, -2},{-2, -3},{-2, -4}},  /* 112.5  0  */
                        {{ 3, 4},{ 2, 3},{ 2, 2},{ 1, 1},{ 0, 0},{-1, -1},{-2, -2},{-2, -3},{-3, -4}},  /* 123.75  0  */
                        {{ 4, 4},{ 3, 3},{ 2, 2},{ 1, 1},{ 0, 0},{-1, -1},{-2, -2},{-3, -3},{-4, -4}},  /* 135  0  */
                        {{ 4, 3},{ 3, 2},{ 2, 2},{ 1, 1},{ 0, 0},{-1, -1},{-2, -2},{-3, -2},{-4, -3}},  /* 146.25  0  */
                        {{ 4, 2},{ 3, 2},{ 2, 1},{ 1, 1},{ 0, 0},{-1, -1},{-2, -1},{-3, -2},{-4, -2}},  /* 157.5  0  */
                        {{ 4, 1},{ 3, 1},{ 2, 1},{ 1, 0},{ 0, 0},{-1,  0},{-2, -1},{-3, -1},{-4, -1}}   /* 168.75  0  */
    };

    if(IBSM_Singular->NumCore != 1)
        return 1;

    //MinIndex = 0;

    memset(DiffAngleSum_Flag, 0, 32);
    memset(DiffAngleSum, 0, 32*4);
    memset(DiffAngleSum_Index, 0, 32*4);

    for(i=0; i<32; i++)
        DiffAngleSum_Index[i] = i;

    CX = IBSM_Singular->Core[0].Pos.x;
    CY = IBSM_Singular->Core[0].Pos.y;

    if(IBSM_Singular->NumCore == 1 && IBSM_Singular->NumDelta == 1 && IBSM_Singular->Core[0].Len.l[0] < 20)
        mag = 0;

    for(s=0; s<16; s++)
    {
        CoreAngle = (int)((double)s * 16);

        for(t=0; t<3; t++)
        {
            dy = CY + CoreDirIdx[s][t][1]*2 + CoreDirIdx[s][3][1]*mag;
            dx = CX + CoreDirIdx[s][t][0]*2 + CoreDirIdx[s][3][0]*mag;
            DiffAngle = abs(CoreAngle - pDirBuf[dy*imgEnQuadWidth+dx]);
            if(DiffAngle >= 128)    DiffAngle = 255 - DiffAngle;
            DiffAngleSum[s+16] += DiffAngle;
        }

        for(t=6; t<9; t++)
        {
            dy = CY + CoreDirIdx[s][t][1]*2 + CoreDirIdx[s][5][1]*mag;
            dx = CX + CoreDirIdx[s][t][0]*2 + CoreDirIdx[s][5][0]*mag;
            DiffAngle = abs(CoreAngle - pDirBuf[dy*imgEnQuadWidth+dx]);
            if(DiffAngle >= 128)    DiffAngle = 255 - DiffAngle;
            DiffAngleSum[s] += DiffAngle;
        }
    }

    for(s=0; s<5; s++)
    {
        for(t=s+1; t<32; t++)
        {
            if(DiffAngleSum[s] > DiffAngleSum[t])
            {
                TmpVar2 = DiffAngleSum_Index[t];
                DiffAngleSum_Index[t] = DiffAngleSum_Index[s];
                DiffAngleSum_Index[s] = TmpVar2;

                TmpVar = DiffAngleSum[s];
                DiffAngleSum[s] = DiffAngleSum[t];
                DiffAngleSum[t] = TmpVar;
            }
        }

        DiffAngleSum_Flag[DiffAngleSum_Index[s]] = 1;
    }

    for(s=0; s<32-8; s++)
    {
        TmpVar = 0;
        TmpVarSum = 0;
        Dir = 0;
        for(t=s; t<s+8; t++)
        {
            if(DiffAngleSum_Flag[t] == 1)
            {
                TmpVar++;
                TmpVarSum += (10000-DiffAngleSum[t]);
            }
        }

        for(t=s; t<s+8; t++)
        {
            if(DiffAngleSum_Flag[t] == 1)
            {
                Dir += ((10000-DiffAngleSum[t]) * t);
            }
        }

        if(TmpVar >= 5)
        {
            IBSM_Singular->Core[0].Dir.a[3] = (int)((double)Dir / (double)TmpVarSum * 11.25);
            return 1;
        }
    }

    return 0;
}

unsigned char CIBSMAlgorithm::_JudgeFingerIndexing(unsigned char *pLabeledBin, int imgEnQuadWidth, int imgEnQuadHeight, IBSM_SINGULAR *IBSM_Singular, IB_RECT IB_Rect,
                                                  int *TracingAngleMinMax, int label_0_cnt, int label_1_cnt, int label_2_cnt, int ori_core, int ori_delta)
{
//    int Flag = 0;
    int MinCoreX, MaxDeltaX, i;
    int dx, dy, dist;

    if (IBSM_Singular->NumCore == 2 &&
        IBSM_Singular->Core[0].Len.l[0] >= 15 && IBSM_Singular->Core[0].Len.l[1] >= 15 && IBSM_Singular->Core[0].Len.l[2] >= 15 &&
        IBSM_Singular->Core[1].Len.l[0] >= 15 && IBSM_Singular->Core[1].Len.l[1] >= 15 && IBSM_Singular->Core[1].Len.l[2] >= 15)
    {
        dx = (IBSM_Singular->Core[1].Pos.x - IBSM_Singular->Core[0].Pos.x)<<2;
        dy = (IBSM_Singular->Core[1].Pos.y - IBSM_Singular->Core[0].Pos.y)<<2;
//      dist = (int)sqrt((double)(dx*dx + dy*dy));
        dist = m_SqrtTable[abs(dy)][abs(dx)];
        if (dist <= 100)
            return (D_FP_WHORL_TYPE<<4)|D_FP_WHORL_TYPE;
    }

    if(_JudgeWhorl(IBSM_Singular) == 1)
    {
        if(_JudgePerpectWhorl(IBSM_Singular, IB_Rect, ori_core, ori_delta) == 1)
            return (D_FP_WHORL_TYPE<<4)|D_FP_WHORL_TYPE;
        if(_JudgeTracingPattern(IBSM_Singular) == 1)
            return (D_FP_UNKNOWN_TYPE<<4)|D_FP_UNKNOWN_TYPE;

        return _JudgeLongDistWhorl(IBSM_Singular);
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    // (C2, D2 | C2, D1) - C2가 모두 D보다 위에 존재
    if(IBSM_Singular->NumCore == 2 && IBSM_Singular->NumDelta == 1)
    {
        MinCoreX = 10000;
        for(i=0; i<IBSM_Singular->NumCore; i++)
        {
            if(MinCoreX > IBSM_Singular->Core[i].Pos.x)
                MinCoreX = IBSM_Singular->Core[i].Pos.x;
        }

        MaxDeltaX = 0;
        for(i=0; i<IBSM_Singular->NumDelta; i++)
        {
            if(MaxDeltaX < IBSM_Singular->Delta[i].Pos.x)
                MaxDeltaX = IBSM_Singular->Delta[i].Pos.x;
        }

        if(MinCoreX <= MaxDeltaX)
        {
            IBSM_Singular->NumCore = 1;
            //Flag =1;
        }
    }

    if(IBSM_Singular->NumCore == 2 && IBSM_Singular->NumDelta == 0)
    {
        int RealCorePosX, RealCorePosY;
        RealCorePosX = (IBSM_Singular->Core[1].Pos.x<<2) - D_ENLARGE_BAND;
        RealCorePosY = (IBSM_Singular->Core[1].Pos.y<<2) - D_ENLARGE_BAND;
        if (RealCorePosX < IB_Rect.left || RealCorePosX > IB_Rect.right || RealCorePosY < IB_Rect.top || RealCorePosY > IB_Rect.bottom)
            IBSM_Singular->NumCore = 1;
    }
    /////////////////////////////////////////////////////////////////////////////////////////

    if(_JudgeTentedArch(IBSM_Singular, TracingAngleMinMax) == 1)
    {
        if(_JudgeTracingPattern(IBSM_Singular) == 1)
            return (D_FP_UNKNOWN_TYPE<<4)|D_FP_UNKNOWN_TYPE;
        else if(IBSM_Singular->Core[0].Dir.a[3] > IBSM_Singular->Core[0].Dir.a[0])
            return (D_FP_TENTED_ARCH_TYPE<<4)|D_FP_LEFT_LOOP_TYPE;
        else if(IBSM_Singular->Core[0].Dir.a[3] < IBSM_Singular->Core[0].Dir.a[0])
            return (D_FP_TENTED_ARCH_TYPE<<4)|D_FP_RIGHT_LOOP_TYPE;
        else
            return (D_FP_TENTED_ARCH_TYPE<<4)|D_FP_TENTED_ARCH_TYPE;
    }

    if(_JudgeRightLoop(pLabeledBin, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular) == 1)
    {
        if(_JudgeShiftedWhorl(pLabeledBin, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular, D_FP_RIGHT_LOOP_TYPE, label_0_cnt, label_1_cnt, label_2_cnt) == 1)
            return ((D_FP_RIGHT_LOOP_TYPE<<4) | D_FP_WHORL_TYPE);
        else if(_JudgeCorePosIsOutSide(IBSM_Singular, IB_Rect, D_FP_RIGHT_LOOP_TYPE) == 1)
            return (D_FP_RIGHT_LOOP_TYPE<<4)|D_FP_WHORL_TYPE;
        else if (_JudgeBestLeftRight(IBSM_Singular, ori_core, ori_delta) == 1)
            return (D_FP_RIGHT_LOOP_TYPE<<4)|D_FP_RIGHT_LOOP_TYPE;
        else if(_Judge180degreeToTented(IBSM_Singular) == 1)
            return (D_FP_RIGHT_LOOP_TYPE<<4)|D_FP_TENTED_ARCH_TYPE;
        else if(_JudgeTracingPattern(IBSM_Singular) == 1)
            return (D_FP_UNKNOWN_TYPE<<4)|D_FP_UNKNOWN_TYPE;
        else if(_JudgeWrongDeltaPos(IBSM_Singular, D_FP_RIGHT_LOOP_TYPE) == 1)      // sklee add : 2012-11-26
            return (D_FP_UNKNOWN_TYPE<<4)|D_FP_UNKNOWN_TYPE;
        else
            return (D_FP_RIGHT_LOOP_TYPE<<4)|D_FP_RIGHT_LOOP_TYPE;
    }

    if(_JudgeLeftLoop(pLabeledBin, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular) == 1)
    {
        if(_JudgeShiftedWhorl(pLabeledBin, imgEnQuadWidth, imgEnQuadHeight, IBSM_Singular, D_FP_LEFT_LOOP_TYPE, label_0_cnt, label_1_cnt, label_2_cnt) == 1)
            return ((D_FP_LEFT_LOOP_TYPE<<4) | D_FP_WHORL_TYPE);
        else if(_JudgeCorePosIsOutSide(IBSM_Singular, IB_Rect, D_FP_LEFT_LOOP_TYPE) == 1)
            return (D_FP_LEFT_LOOP_TYPE<<4)|D_FP_WHORL_TYPE;
        else if (_JudgeBestLeftRight(IBSM_Singular, ori_core, ori_delta) == 1)
            return (D_FP_LEFT_LOOP_TYPE<<4)|D_FP_LEFT_LOOP_TYPE;
        else if(_Judge180degreeToTented(IBSM_Singular) == 1)
            return (D_FP_LEFT_LOOP_TYPE<<4)|D_FP_TENTED_ARCH_TYPE;
        else if(_JudgeTracingPattern(IBSM_Singular) == 1)
            return (D_FP_UNKNOWN_TYPE<<4)|D_FP_UNKNOWN_TYPE;
        else if(_JudgeWrongDeltaPos(IBSM_Singular, D_FP_LEFT_LOOP_TYPE) == 1)           // sklee add : 2012-11-26
            return (D_FP_UNKNOWN_TYPE<<4)|D_FP_UNKNOWN_TYPE;
        else
            return (D_FP_LEFT_LOOP_TYPE<<4)|D_FP_LEFT_LOOP_TYPE;
    }

    return (D_FP_UNKNOWN_TYPE<<4)|D_FP_UNKNOWN_TYPE;
}

unsigned char CIBSMAlgorithm::_JudgeWhorl(IBSM_SINGULAR *IBSM_Singular)
{
    int Count;
    int i, MinCoreX, /*MaxDeltaX,*/ LowerCoreIdx, CoreDiffAngle;

    if(IBSM_Singular->NumCore != 2 || IBSM_Singular->NumDelta > 2)
        return 0;

    /////////////////////////////////////////////////////////////////////////////////////////
    // C에서 추적한 선 중 2개 이상이 짧을 경우 Whorl이 아님
    Count = 0;
    if(IBSM_Singular->Core[0].Len.l[0] < 5) Count++;
    if(IBSM_Singular->Core[0].Len.l[1] < 5) Count++;
    if(IBSM_Singular->Core[0].Len.l[2] < 5) Count++;

    if(IBSM_Singular->Core[1].Len.l[0] < 5) Count++;
    if(IBSM_Singular->Core[1].Len.l[1] < 5) Count++;
    if(IBSM_Singular->Core[1].Len.l[2] < 5) Count++;

    if(Count >= 2)
        return 0;
    /////////////////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////////////////////
    // Upper코어가 Lower보다 너무 낮은 경우 Unknown
    if(IBSM_Singular->Core[0].Pos.x < IBSM_Singular->Core[1].Pos.x - 25)
        return 0;
    /////////////////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////////////////////
    // Upper코어위에 Lower코어가 가깝게 있는 경우 Unknown
    if(IBSM_Singular->Core[1].Pos.x - IBSM_Singular->Core[0].Pos.x > 0 && abs(IBSM_Singular->Core[1].Pos.y - IBSM_Singular->Core[0].Pos.y) < 10)
        return 0;
    /////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////////
    // (C2, D2) - 아래 C는 D2를 모두 만남
    if(IBSM_Singular->Core[0].Pos.x > IBSM_Singular->Core[1].Pos.x)
        LowerCoreIdx = 1;
    else
        LowerCoreIdx = 0;

    MinCoreX = 0;
    for(i=0; i<10; i++)
    {
        if(IBSM_Singular->Core[LowerCoreIdx].Meet.m[i] > 0)
            MinCoreX++;
    }

    if(MinCoreX < 2 && IBSM_Singular->NumDelta == 2)
        return 0;
    /////////////////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////////////////////
    // (C2, D>0) - C2 사이에 D가 있으면 안된다.
    if(IBSM_Singular->NumDelta == 1)
    {
        float a;
        int dx, dy;
        int b1, b2, result1, result2;

        int CentralCoreX, CentralCoreY;
        int CoreCoreDist, CentralCoreDeltaDist;

        CentralCoreX = (IBSM_Singular->Core[0].Pos.x + IBSM_Singular->Core[1].Pos.x)/2;
        CentralCoreY = (IBSM_Singular->Core[0].Pos.y + IBSM_Singular->Core[1].Pos.y)/2;

        dx = IBSM_Singular->Core[0].Pos.x - IBSM_Singular->Core[1].Pos.x;
        dy = IBSM_Singular->Core[0].Pos.y - IBSM_Singular->Core[1].Pos.y;

//      CoreCoreDist = (int)sqrt((double)(dx*dx + dy*dy));
        CoreCoreDist = m_SqrtTable[abs(dy)][abs(dx)];

        dx = CentralCoreX - IBSM_Singular->Delta[0].Pos.x;
        dy = CentralCoreY - IBSM_Singular->Delta[0].Pos.y;

//      CentralCoreDeltaDist = (int)sqrt((double)(dx*dx + dy*dy));
        CentralCoreDeltaDist = m_SqrtTable[abs(dy)][abs(dx)];

        if(CentralCoreDeltaDist*2 < CoreCoreDist)
        {
            dx = IBSM_Singular->Core[0].Pos.x - IBSM_Singular->Core[1].Pos.x;
            dy = IBSM_Singular->Core[0].Pos.y - IBSM_Singular->Core[1].Pos.y;

            a = - 1.0f / ((float)dy / (float)dx);

            b1 = IBSM_Singular->Core[0].Pos.y - (int)(a * IBSM_Singular->Core[0].Pos.x);
            b2 = IBSM_Singular->Core[1].Pos.y - (int)(a * IBSM_Singular->Core[1].Pos.x);

            for(i=0; i<IBSM_Singular->NumDelta; i++)
            {
                result1 = (int)(a*IBSM_Singular->Delta[i].Pos.x) + b1 - IBSM_Singular->Delta[i].Pos.y;
                result2 = (int)(a*IBSM_Singular->Delta[i].Pos.x) + b2 - IBSM_Singular->Delta[i].Pos.y;

                if(b1 > b2)
                {
                    if(result1 > 0 && result2 < 0)
                        return 0;
                }
                else if(b2 > b1)
                {
                    if(result2 > 0 && result1 < 0)
                        return 0;
                }
            }
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////////////////////
    // (C2, D1) - C2 모두 D를 만날 경우 Whorl
    if(IBSM_Singular->Core[0].Meet.m[0] > 0 && IBSM_Singular->Core[1].Meet.m[0] > 0 && IBSM_Singular->NumDelta == 1)
        return 1;
    /////////////////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////////////////////
    // (C2) - C2의 방향차가 180도 언저리여야 한다.
    CoreDiffAngle = abs(abs(IBSM_Singular->Core[0].Dir.a[3] - IBSM_Singular->Core[1].Dir.a[3]) - 180);
    if(CoreDiffAngle > 60)
        return 0;
    /////////////////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////////////////////
    // (C2) - C2는 모두 Tracing 끝이 Background를 만나야 한다.
    if(IBSM_Singular->NumDelta == 0)
    {
        int Sum = 0;
        for(i=0; i<3; i++)
        {
            Sum += (IBSM_Singular->Core[0].Meet.m[i] + IBSM_Singular->Core[1].Meet.m[i]);
        }

        if(Sum < 5)
            return 0;
    }
    /////////////////////////////////////////////////////////////////////////////////////////

    return 1;
}

unsigned char CIBSMAlgorithm::_JudgePerpectWhorl(IBSM_SINGULAR *IBSM_Singular, IB_RECT IB_Rect, int ori_core, int ori_delta)
{
    int dx, dy;
    int CoreDist;//, CoreAngle;
    int /*DeltaDist, */DeltaAngle;
    int CoreDeltaAngle1, CoreDeltaAngle2;
    int Tmpvalue;
    int bottom_Bound = 20;
    int up_Bound = 35;
    int dist_bound = 30;

    if (ori_core != 2 || ori_delta > 2 )//|| original_core != SingularCount[0] || original_delta != SingularCount[1])
        return 0;

    dx = (IBSM_Singular->Core[1].Pos.x - IBSM_Singular->Core[0].Pos.x)<<2;
    dy = (IBSM_Singular->Core[1].Pos.y - IBSM_Singular->Core[0].Pos.y)<<2;

//  CoreDist = (int)sqrt((double)(dx*dx + dy*dy));
    CoreDist = m_SqrtTable[abs(dy)][abs(dx)];
    //CoreAngle = _GetAtan2(-dy, dx) * 360 >> 9;

    if (/*CoreAngle < 180-Bound || CoreAngle > 180+Bound ||*/ CoreDist > 75)
        return 0;

    if (ori_delta == 2)
    {
        dx = (IBSM_Singular->Delta[1].Pos.x - IBSM_Singular->Delta[0].Pos.x)<<2;
        dy = (IBSM_Singular->Delta[1].Pos.y - IBSM_Singular->Delta[0].Pos.y)<<2;

//      DeltaDist = (int)sqrt((double)(dx*dx + dy*dy));
        //DeltaDist = m_SqrtTable[abs(dy)][abs(dx)];
        DeltaAngle = _GetAtan2(-dy, dx) * 360 >> 9;

        dx = (IBSM_Singular->Delta[0].Pos.x - IBSM_Singular->Core[1].Pos.x)<<2;
        dy = (IBSM_Singular->Delta[0].Pos.y - IBSM_Singular->Core[1].Pos.y)<<2;

        CoreDeltaAngle1 = _GetAtan2(-dy, dx) * 360 >> 9;

        dx = (IBSM_Singular->Delta[1].Pos.x - IBSM_Singular->Core[1].Pos.x)<<2;
        dy = (IBSM_Singular->Delta[1].Pos.y - IBSM_Singular->Core[1].Pos.y)<<2;

        CoreDeltaAngle2 = _GetAtan2(-dy, dx) * 360 >> 9;

        if (CoreDeltaAngle1 > CoreDeltaAngle2)
        {
            Tmpvalue = CoreDeltaAngle1;
            CoreDeltaAngle1 = CoreDeltaAngle2;
            CoreDeltaAngle2 = Tmpvalue;
        }

        if (DeltaAngle < 270-bottom_Bound || DeltaAngle > 270+bottom_Bound)
            return 0;
        if (CoreDeltaAngle1 < 135-up_Bound || CoreDeltaAngle1 > 135+bottom_Bound)
            return 0;
        if (CoreDeltaAngle2 < 225-bottom_Bound || CoreDeltaAngle2 > 225+up_Bound)
            return 0;
        return 1;
    }
    else if (ori_delta == 1)
    {
        dx = (IBSM_Singular->Delta[0].Pos.x - IBSM_Singular->Core[1].Pos.x)<<2;
        dy = (IBSM_Singular->Delta[0].Pos.y - IBSM_Singular->Core[1].Pos.y)<<2;

        CoreDeltaAngle1 = _GetAtan2(-dy, dx) * 360 >> 9;

        if ((CoreDeltaAngle1 >= 135-up_Bound && CoreDeltaAngle1 <= 135+bottom_Bound) ||
            (CoreDeltaAngle1 >= 225-bottom_Bound && CoreDeltaAngle1 <= 225+up_Bound))
            return 1;

        return 0;
    }
    else
    {
        if (IBSM_Singular->Core[0].Pos.x > (IB_Rect.left+dist_bound)>>2 && IBSM_Singular->Core[0].Pos.x < (IB_Rect.right-dist_bound)>>2 &&
            IBSM_Singular->Core[1].Pos.x > (IB_Rect.left+dist_bound)>>2 && IBSM_Singular->Core[1].Pos.x < (IB_Rect.right-dist_bound)>>2 &&
            IBSM_Singular->Core[0].Pos.y > (IB_Rect.top+dist_bound)>>2 && IBSM_Singular->Core[0].Pos.y < (IB_Rect.bottom-dist_bound)>>2 &&
            IBSM_Singular->Core[1].Pos.y > (IB_Rect.top+dist_bound)>>2 && IBSM_Singular->Core[1].Pos.y < (IB_Rect.bottom-dist_bound)>>2)
            return 1;
    }

    return 0;
}

unsigned char CIBSMAlgorithm::_JudgeTracingPattern(IBSM_SINGULAR *IBSM_Singular)
{
    if(IBSM_Singular->NumCore == 1 && IBSM_Singular->NumDelta < 2)
    {
        if(IBSM_Singular->Core[0].Pattern.p[0] > 2 && IBSM_Singular->Core[0].Pattern.p[1] > 2 &&
            IBSM_Singular->Core[0].Pattern.p[2] > 5 && IBSM_Singular->Core[0].Pattern.p[3] > 5)
            return 1;

        if(IBSM_Singular->Core[0].Pattern.p[0] > 5 && IBSM_Singular->Core[0].Pattern.p[1] > 5 &&
            IBSM_Singular->Core[0].Pattern.p[2] > 2 && IBSM_Singular->Core[0].Pattern.p[3] > 2)
            return 1;
    }
    else if(IBSM_Singular->NumCore == 2 && IBSM_Singular->NumDelta == 0)
    {
        if(IBSM_Singular->Core[0].Pattern.p[0] > 2 && IBSM_Singular->Core[0].Pattern.p[1] > 2 &&
            IBSM_Singular->Core[0].Pattern.p[2] > 7 && IBSM_Singular->Core[0].Pattern.p[3] > 20)
            return 1;

        if(IBSM_Singular->Core[0].Pattern.p[0] > 7 && IBSM_Singular->Core[0].Pattern.p[1] > 20 &&
            IBSM_Singular->Core[0].Pattern.p[2] > 2 && IBSM_Singular->Core[0].Pattern.p[3] > 2)
            return 1;
    }

    return 0;
}

unsigned char CIBSMAlgorithm::_JudgeLongDistWhorl(IBSM_SINGULAR *IBSM_Singular)
{
    int CoreDist, dx, dy;
    int /*CoreAngle, */UpperCoreDir;
    int CoreDistThres = 75;

    dx = (IBSM_Singular->Core[1].Pos.x - IBSM_Singular->Core[0].Pos.x)<<2;
    dy = (IBSM_Singular->Core[1].Pos.y - IBSM_Singular->Core[0].Pos.y)<<2;
    UpperCoreDir = IBSM_Singular->Core[0].Dir.a[3];

//  CoreDist = (int)sqrt((double)(dx*dx + dy*dy));
    CoreDist = m_SqrtTable[abs(dy)][abs(dx)];
    //CoreAngle = _GetAtan2(-dy, dx) * 360 >> 9;

    if(CoreDist > CoreDistThres)
    {
        if(IBSM_Singular->Core[0].Dir.a[0] >= UpperCoreDir /*&& UpperCoreDir <= 180 */&& abs(IBSM_Singular->Core[0].Dir.a[0]-UpperCoreDir) >= 20)
            return (D_FP_WHORL_TYPE<<4)|D_FP_RIGHT_LOOP_TYPE;
        else if(IBSM_Singular->Core[0].Dir.a[0] <= UpperCoreDir/* && UpperCoreDir >= 180*/ && abs(IBSM_Singular->Core[0].Dir.a[0]-UpperCoreDir) >= 20)
            return (D_FP_WHORL_TYPE<<4)|D_FP_LEFT_LOOP_TYPE;
        else
            return (D_FP_UNKNOWN_TYPE<<4)|D_FP_UNKNOWN_TYPE;
    }

    return (D_FP_WHORL_TYPE<<4)|D_FP_WHORL_TYPE;
}

unsigned char CIBSMAlgorithm::_JudgeTentedArch(IBSM_SINGULAR *IBSM_Singular, int *TracingAngleMinMax)
{
    int AngleDiff1, AngleDiff2;

    if(IBSM_Singular->NumCore != 1/* || SingularCount[1] != 1 */)
        return 0;

    // Delta가 존재하는데, Core가 Delta와 만나지 않으면 Unknown
    if(IBSM_Singular->Core[0].Meet.m[0] == 0 && IBSM_Singular->NumDelta == 1)
        return 0;

    AngleDiff1 = abs(TracingAngleMinMax[0] - TracingAngleMinMax[1]);
    if(AngleDiff1 > 180) AngleDiff1 = 360 - AngleDiff1;
    if(AngleDiff1 > 30) return 0;

    // Delta가 Core 위에 존재할 경우 Unknown
    if(IBSM_Singular->Core[0].Pos.y < IBSM_Singular->Delta[0].Pos.y)
        return 0;

    AngleDiff1 = abs(IBSM_Singular->Core[0].Dir.a[1] - IBSM_Singular->Core[0].Dir.a[0]);
    AngleDiff2 = abs(IBSM_Singular->Core[0].Dir.a[0] - IBSM_Singular->Core[0].Dir.a[2]);

    if(AngleDiff1 > 90 && AngleDiff1 < 180 && AngleDiff2 > 90 && AngleDiff2 < 180
         && IBSM_Singular->Core[0].Dir.a[3] >= (90-20) && IBSM_Singular->Core[0].Dir.a[3] <= (90+20) &&
         abs(IBSM_Singular->Core[0].Dir.a[0] - IBSM_Singular->Core[0].Dir.a[3]) < 20)
    {
        return 1;
    }

    return 0;
}

unsigned char CIBSMAlgorithm::_JudgeRightLoop(unsigned char *pLabeledBin, int imgEnQuadWidth, int imgEnQuadHeight, IBSM_SINGULAR *IBSM_Singular)
{
    int AngleDiff2;
    int MinY, s, t;
    unsigned char AppliedColor[3];
    int AppliedColorWhiteIdx=0;

    // Core = 1, Delta <= 1
    if(IBSM_Singular->NumCore != 1 || IBSM_Singular->NumDelta > 1)
        return 0;

    if(IBSM_Singular->Core[0].Dir.a[3] >= 180 && IBSM_Singular->Core[0].Dir.a[3] <= 360)
        return 0;

    // Delta가 존재하는데, Core가 Delta와 만나지 않으면 Unknown
    if(IBSM_Singular->NumDelta == 1 && IBSM_Singular->Core[0].Meet.m[0] == 0)
        return 0;

    // Delta가 Core 위에 존재할 경우 Unknown
    if(IBSM_Singular->NumDelta == 1 && IBSM_Singular->Core[0].Pos.y < IBSM_Singular->Delta[0].Pos.y)
        return 0;


    ////////////////////////////////////////////////////////////////////////
    int CoreX = IBSM_Singular->Core[0].Pos.x;
    int CoreY = IBSM_Singular->Core[0].Pos.y;
    AppliedColor[0] = pLabeledBin[CoreY*imgEnQuadWidth+CoreX];

    if(AppliedColor[0] != pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX])
    {
        AppliedColor[1] = pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX];

        if(AppliedColor[0] != pLabeledBin[CoreY*imgEnQuadWidth+CoreX+1] &&
            AppliedColor[1] != pLabeledBin[CoreY*imgEnQuadWidth+CoreX+1])
        {
            AppliedColor[2] = pLabeledBin[CoreY*imgEnQuadWidth+CoreX+1];
        }
        else if(AppliedColor[0] != pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX+1] &&
            AppliedColor[1] != pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX+1])
        {
            AppliedColor[2] = pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX+1];
        }
    }
    else if(AppliedColor[0] != pLabeledBin[CoreY*imgEnQuadWidth+CoreX+1])
    {
        AppliedColor[1] = pLabeledBin[CoreY*imgEnQuadWidth+CoreX+1];

        if(AppliedColor[0] != pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX+1] &&
            AppliedColor[1] != pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX+1])
        {
            AppliedColor[2] = pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX+1];
        }
        else
            AppliedColor[2] = 128;
    }

    MinY = 1000000;

    if(AppliedColor[0] >= 171) AppliedColorWhiteIdx = 0;
    if(AppliedColor[1] >= 171) AppliedColorWhiteIdx = 1;
    if(AppliedColor[2] >= 171) AppliedColorWhiteIdx = 2;

    for(s=0; s<imgEnQuadHeight; s++)
    {
        for(t=0; t<imgEnQuadWidth; t++)
        {
            if(pLabeledBin[s*imgEnQuadWidth+t] == AppliedColor[AppliedColorWhiteIdx])
            {
                if(MinY > s)
                    MinY = s;
            }
        }
    }

    if(MinY >= IBSM_Singular->Core[0].Pos.y)
        return 0;
    ////////////////////////////////////////////////////////////////////////


    // Delta가 없을 경우 녹색과 파란색의 각도를 구한다.
    if(IBSM_Singular->NumDelta == 0)
    {
        if(IBSM_Singular->Core[0].Dir.a[2] > IBSM_Singular->Core[0].Dir.a[0])
            AngleDiff2 = IBSM_Singular->Core[0].Dir.a[0] + 360 - IBSM_Singular->Core[0].Dir.a[2];
        else
            AngleDiff2 = IBSM_Singular->Core[0].Dir.a[0] - IBSM_Singular->Core[0].Dir.a[2];
    }
    // Delta가 있을 경우 녹색과 파란색의 각도를 구한다.
    else
    {
        if(IBSM_Singular->Core[0].Dir.a[2] > IBSM_Singular->Core[0].Dir.a[0])
            AngleDiff2 = IBSM_Singular->Core[0].Dir.a[0] + 360 - IBSM_Singular->Core[0].Dir.a[2];
        else
            AngleDiff2 = IBSM_Singular->Core[0].Dir.a[0] - IBSM_Singular->Core[0].Dir.a[2];
    }

    // Core-Delta 방향과 Core의 흐름방향의 차이가 20보다 크고, Core의 흐름방향이 150 이하일 경우
    if( (IBSM_Singular->Core[0].Dir.a[0] - IBSM_Singular->Core[0].Dir.a[3]) > 20 && IBSM_Singular->Core[0].Dir.a[3] <= 90)
    {
        // 만약 Delta가 있거나 Core Tracing 길이가 30 이상이면.
        if(IBSM_Singular->NumDelta == 1 || IBSM_Singular->Core[0].Len.l[0] > 10)
            return 1;
    }

    // 녹색과 파란색의 차이가 180을 넘고 Core 방향이 70을 넘지 않으면
    if(AngleDiff2 > 180 && IBSM_Singular->Core[0].Dir.a[3] <= (90-20))
        return 1;

    return 0;
}

unsigned char CIBSMAlgorithm::_JudgeShiftedWhorl(unsigned char *pLabeledBin, int imgEnQuadWidth, int imgEnQuadHeight, IBSM_SINGULAR *IBSM_Singular, unsigned char Flag,
                                                 int label_0_cnt, int label_1_cnt, int label_2_cnt)
{
    unsigned char StartColor=0, MidColor, CoreNearColor[4];
    int ReachFlag, ReachMidFlag;
	int LowBlockPos[2]={0}, LowBlockFlag, TmpValue;
    int ColorCount[10], ColorCentralPos[10][2], MidColorCount[10], MidColorCentralPos[10][2];
    int ColorLowIndex=0, MidColorLowIndex=0;
    int i, j;
    int CoreX, CoreY;

    if(IBSM_Singular->NumCore != 1 || IBSM_Singular->NumDelta > 0)
        return 0;

    memset(ColorCount, 0, 10*4);
    memset(ColorCentralPos, 0, 10*4*2);

    memset(MidColorCount, 0, 10*4);
    memset(MidColorCentralPos, 0, 10*4*2);

    MidColor = 102;

    if(Flag == D_FP_LEFT_LOOP_TYPE)
    {
        StartColor = 202;
    }
    else if(Flag == D_FP_RIGHT_LOOP_TYPE)
    {
        StartColor = 2;
    }

    LowBlockFlag = 0;

    CoreX = IBSM_Singular->Core[0].Pos.x;
    CoreY = IBSM_Singular->Core[0].Pos.y;

    CoreNearColor[0] = pLabeledBin[CoreY*imgEnQuadWidth+CoreX];
    CoreNearColor[1] = pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX];
    CoreNearColor[2] = pLabeledBin[CoreY*imgEnQuadWidth+CoreX+1];
    CoreNearColor[3] = pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX+1];

    if((Flag == D_FP_RIGHT_LOOP_TYPE && label_0_cnt >= 2) ||
        (Flag == D_FP_LEFT_LOOP_TYPE && label_2_cnt >= 2))
    {
        LowBlockFlag = 1;

        for(i=0; i<imgEnQuadHeight; i++)
        {
            for(j=0; j<imgEnQuadWidth; j++)
            {
                if(pLabeledBin[i*imgEnQuadWidth+j] >= StartColor && pLabeledBin[i*imgEnQuadWidth+j] < StartColor+10 &&
                    CoreNearColor[0] != pLabeledBin[i*imgEnQuadWidth+j] && CoreNearColor[1] != pLabeledBin[i*imgEnQuadWidth+j]
                    && CoreNearColor[2] != pLabeledBin[i*imgEnQuadWidth+j] && CoreNearColor[3] != pLabeledBin[i*imgEnQuadWidth+j])
                {
                    ColorCount[pLabeledBin[i*imgEnQuadWidth+j]-StartColor]++;
                    ColorCentralPos[pLabeledBin[i*imgEnQuadWidth+j]-StartColor][0] += j;
                    ColorCentralPos[pLabeledBin[i*imgEnQuadWidth+j]-StartColor][1] += i;
                }

                if(pLabeledBin[i*imgEnQuadWidth+j] >= MidColor && pLabeledBin[i*imgEnQuadWidth+j] < MidColor+10 &&
                    CoreNearColor[0] != pLabeledBin[i*imgEnQuadWidth+j] && CoreNearColor[1] != pLabeledBin[i*imgEnQuadWidth+j]
                    && CoreNearColor[2] != pLabeledBin[i*imgEnQuadWidth+j] && CoreNearColor[3] != pLabeledBin[i*imgEnQuadWidth+j])
                {
                    MidColorCount[pLabeledBin[i*imgEnQuadWidth+j]-MidColor]++;
                    MidColorCentralPos[pLabeledBin[i*imgEnQuadWidth+j]-MidColor][0] += j;
                    MidColorCentralPos[pLabeledBin[i*imgEnQuadWidth+j]-MidColor][1] += i;
                }
            }
        }

        for(i=0; i<10; i++)
        {
            if(ColorCount[i] > 0)
            {
                ColorCentralPos[i][0] = ColorCentralPos[i][0] / ColorCount[i];
                ColorCentralPos[i][1] = ColorCentralPos[i][1] / ColorCount[i];
            }

            if(MidColorCount[i] > 0)
            {
                MidColorCentralPos[i][0] = MidColorCentralPos[i][0] / MidColorCount[i];
                MidColorCentralPos[i][1] = MidColorCentralPos[i][1] / MidColorCount[i];
            }
        }

        // 흰색 또는 검은색에서 상위 2개 소팅
        for(i=0; i<9; i++)
        {
            for(j=i+1; j<10; j++)
            {
                if(ColorCount[i] < ColorCount[j])
                {
                    TmpValue = ColorCount[i];
                    ColorCount[i] = ColorCount[j];
                    ColorCount[j] = TmpValue;

                    TmpValue = ColorCentralPos[i][0];
                    ColorCentralPos[i][0] = ColorCentralPos[j][0];
                    ColorCentralPos[j][0] = TmpValue;

                    TmpValue = ColorCentralPos[i][1];
                    ColorCentralPos[i][1] = ColorCentralPos[j][1];
                    ColorCentralPos[j][1] = TmpValue;
                }
            }
        }

        // 아래쪽 인덱스 및 위치 검출
        ReachFlag = 0;
        for(i=0; i<10; i++)
        {
            if(ColorCentralPos[i][1] < IBSM_Singular->Core[0].Pos.y)
            {
                ColorLowIndex = i;

                LowBlockPos[0] = ColorCentralPos[i][0];
                LowBlockPos[1] = ColorCentralPos[i][1];

                ReachFlag = 1;
                break;
            }
        }

        // 회색에서 상위 2개 소팅
        for(i=0; i<9; i++)
        {
            for(j=i+1; j<10; j++)
            {
                if(MidColorCount[i] < MidColorCount[j])
                {
                    TmpValue = MidColorCount[i];
                    MidColorCount[i] = MidColorCount[j];
                    MidColorCount[j] = TmpValue;

                    TmpValue = MidColorCentralPos[i][0];
                    MidColorCentralPos[i][0] = MidColorCentralPos[j][0];
                    MidColorCentralPos[j][0] = TmpValue;

                    TmpValue = MidColorCentralPos[i][1];
                    MidColorCentralPos[i][1] = MidColorCentralPos[j][1];
                    MidColorCentralPos[j][1] = TmpValue;
                }
            }
        }

        // 아래쪽 인덱스 검출
        ReachMidFlag = 0;
        for(i=0; i<10; i++)
        {
            if(MidColorCentralPos[i][1] < IBSM_Singular->Core[0].Pos.y)
            {
                MidColorLowIndex = i;
                ReachMidFlag = 1;
                break;
            }
        }
    }
    else
        return 0;

    if(ReachFlag == 0 || ReachMidFlag == 0)
        return 0;

    if( ((Flag == D_FP_RIGHT_LOOP_TYPE && label_0_cnt == 2) || (Flag == D_FP_LEFT_LOOP_TYPE && label_2_cnt == 2))
        && ColorCount[ColorLowIndex] > 50)
        return 1;

    if(ColorCount[ColorLowIndex] < MidColorCount[MidColorLowIndex] &&
        IBSM_Singular->Core[0].Pos.y > MidColorCentralPos[MidColorLowIndex][1])
        return 0;

    if(LowBlockFlag == 1 && ColorCount[ColorLowIndex] > 50)
    {
        if(Flag == D_FP_LEFT_LOOP_TYPE)
        {
            if(LowBlockPos[1] < IBSM_Singular->Core[0].Pos.y && LowBlockPos[0] > IBSM_Singular->Core[0].Pos.x)
                return 1;
        }
        else if(Flag == D_FP_RIGHT_LOOP_TYPE)
        {
            if(LowBlockPos[1] < IBSM_Singular->Core[0].Pos.y && LowBlockPos[0] < IBSM_Singular->Core[0].Pos.x)
                return 1;
        }
    }

    return 0;
}

unsigned char CIBSMAlgorithm::_JudgeCorePosIsOutSide(IBSM_SINGULAR *IBSM_Singular, IB_RECT IB_Rect, int type)
{
    int BoundX = 60, BoundY = 50;
    int RealCorePosX, RealCorePosY;

    RealCorePosX = (IBSM_Singular->Core[0].Pos.x<<2) - D_ENLARGE_BAND;
    RealCorePosY = (IBSM_Singular->Core[0].Pos.y<<2) - D_ENLARGE_BAND;

    if (type == D_FP_LEFT_LOOP_TYPE)
    {
        if((RealCorePosX < IB_Rect.left + BoundX) || (RealCorePosY > IB_Rect.bottom - BoundY) )
        {
            return 1;
        }
    }
    else if(type == D_FP_RIGHT_LOOP_TYPE)
    {
        if((RealCorePosX < IB_Rect.left + BoundX) || (RealCorePosY < IB_Rect.top + BoundY))
        {
            return 1;
        }
    }

    return 0;
}

unsigned char CIBSMAlgorithm::_JudgeBestLeftRight(IBSM_SINGULAR *IBSM_Singular, int ori_core, int ori_delta)
{
    int Range = 30;

    if (ori_core == 1 && ori_delta == 1 && abs(IBSM_Singular->Core[0].Dir.a[3]-IBSM_Singular->Core[0].Dir.a[0]) > Range)
        return 1;
    if (ori_core == 1 && ori_delta == 0 && abs(IBSM_Singular->Core[0].Dir.a[3]-IBSM_Singular->Core[0].Dir.a[0]) > Range)
        return 1;

    return 0;
}

unsigned char CIBSMAlgorithm::_Judge180degreeToTented(IBSM_SINGULAR *IBSM_Singular)
{
    int Range = 30;

    if(abs(IBSM_Singular->Core[0].Dir.a[3]-180) < Range)
        return 1;

    return 0;
}

unsigned char CIBSMAlgorithm::_JudgeLeftLoop(unsigned char *pLabeledBin, int imgEnQuadWidth, int imgEnQuadHeight, IBSM_SINGULAR *IBSM_Singular)
{
    int AngleDiff1;
    int MinY, s, t;
    unsigned char AppliedColor[3];
    int AppliedColorWhiteIdx=0;

    // Core = 1, Delta <= 1
    if(IBSM_Singular->NumCore != 1 || IBSM_Singular->NumDelta > 1)
        return 0;

    if(IBSM_Singular->Core[0].Dir.a[3] >= 180 && IBSM_Singular->Core[0].Dir.a[3] <= 360)
        return 0;

    // Delta가 존재하는데, Core가 Delta와 만나지 않으면 Unknown
    if(IBSM_Singular->NumDelta == 1 && IBSM_Singular->Core[0].Meet.m[0] == 0)
        return 0;

    // Delta가 Core 위에 존재할 경우 Unknown
    if(IBSM_Singular->NumDelta == 1 && IBSM_Singular->Core[0].Pos.y < IBSM_Singular->Delta[0].Pos.y)
        return 0;

    ////////////////////////////////////////////////////////////////////////
    int CoreX = IBSM_Singular->Core[0].Pos.x;
    int CoreY = IBSM_Singular->Core[0].Pos.y;
    AppliedColor[0] = pLabeledBin[CoreY*imgEnQuadWidth+CoreX];

    if(AppliedColor[0] != pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX])
    {
        AppliedColor[1] = pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX];

        if(AppliedColor[0] != pLabeledBin[CoreY*imgEnQuadWidth+CoreX+1] &&
            AppliedColor[1] != pLabeledBin[CoreY*imgEnQuadWidth+CoreX+1])
        {
            AppliedColor[2] = pLabeledBin[CoreY*imgEnQuadWidth+CoreX+1];
        }
        else if(AppliedColor[0] != pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX+1] &&
            AppliedColor[1] != pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX+1])
        {
            AppliedColor[2] = pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX+1];
        }
    }
    else if(AppliedColor[0] != pLabeledBin[CoreY*imgEnQuadWidth+CoreX+1])
    {
        AppliedColor[1] = pLabeledBin[CoreY*imgEnQuadWidth+CoreX+1];

        if(AppliedColor[0] != pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX+1] &&
            AppliedColor[1] != pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX+1])
        {
            AppliedColor[2] = pLabeledBin[(CoreY+1)*imgEnQuadWidth+CoreX+1];
        }
        else
            AppliedColor[2] = 128;
    }

    MinY = 1000000;

    if(AppliedColor[0] >= 171) AppliedColorWhiteIdx = 0;
    if(AppliedColor[1] >= 171) AppliedColorWhiteIdx = 1;
    if(AppliedColor[2] >= 171) AppliedColorWhiteIdx = 2;

    for(s=0; s<imgEnQuadHeight; s++)
    {
        for(t=0; t<imgEnQuadWidth; t++)
        {
            if(pLabeledBin[s*imgEnQuadWidth+t] == AppliedColor[AppliedColorWhiteIdx])
            {
                if(MinY > s)
                    MinY = s;
            }
        }
    }

    if(MinY >= IBSM_Singular->Core[0].Pos.y)
        return 0;
    ////////////////////////////////////////////////////////////////////////


    // Delta가 없을 경우 녹색과 빨간색의 각도를 구한다.
    if(IBSM_Singular->NumDelta == 0)
    {
        if(IBSM_Singular->Core[0].Dir.a[1] < IBSM_Singular->Core[0].Dir.a[0])
            AngleDiff1 = 360 - IBSM_Singular->Core[0].Dir.a[0] + IBSM_Singular->Core[0].Dir.a[1];
        else
            AngleDiff1 = IBSM_Singular->Core[0].Dir.a[1] - IBSM_Singular->Core[0].Dir.a[0];
    }
    // Delta가 있을 경우 녹색과 빨간색의 각도를 구한다.
    else
    {
        if(IBSM_Singular->Core[0].Dir.a[1] < IBSM_Singular->Core[0].Dir.a[0])
            AngleDiff1 = 360 - IBSM_Singular->Core[0].Dir.a[0] + IBSM_Singular->Core[0].Dir.a[1];
        else
            AngleDiff1 = IBSM_Singular->Core[0].Dir.a[1] - IBSM_Singular->Core[0].Dir.a[0];
    }

    // Core-Delta 방향과 Core의 흐름방향의 차이가 -20보다 작고, Core의 흐름방향이 210 이상일 경우
    if( (IBSM_Singular->Core[0].Dir.a[0] - IBSM_Singular->Core[0].Dir.a[3]) < -10 && IBSM_Singular->Core[0].Dir.a[3] >= 90 )
    {
        if(IBSM_Singular->NumDelta == 1 || IBSM_Singular->Core[0].Len.l[0] > 5)
            return 1;
    }

    // 녹색과 빨간색의 차이가 180을 넘고 Core 방향이 200을 넘으면
    if(AngleDiff1 > 180 && IBSM_Singular->Core[0].Dir.a[3] >= (90+20))
        return 1;

    return 0;
}

unsigned char CIBSMAlgorithm::_JudgeWrongDeltaPos(IBSM_SINGULAR *IBSM_Singular, unsigned char Flag)
{
    if(IBSM_Singular->NumCore == 1 && IBSM_Singular->NumDelta == 1)
    {
        if(Flag == D_FP_LEFT_LOOP_TYPE)
        {
            if( !(IBSM_Singular->Core[0].Pos.x < IBSM_Singular->Delta[0].Pos.x && IBSM_Singular->Core[0].Dir.a[3] > 90 && IBSM_Singular->Core[0].Dir.a[3] < 180) )
                return 1;
        }
        else if(Flag == D_FP_RIGHT_LOOP_TYPE)
        {
            if( !(IBSM_Singular->Core[0].Pos.x > IBSM_Singular->Delta[0].Pos.x && IBSM_Singular->Core[0].Dir.a[3] < 90 ) )
                return 1;
        }
    }

    return 0;
}

void CIBSMAlgorithm::_ALGO_Pre_FeaGen_RemaindBuffer(unsigned int *minutia)
{
    unsigned char (*Distance)[100] = new unsigned char [MAX_DIAGONAL_LENGTH][100];
    unsigned char *Distance_count = new unsigned char [MAX_DIAGONAL_LENGTH];
    int dist, min_dist=MAX_DIAGONAL_LENGTH, max_dist=0;
    int i,j,m,n,x,y,count=0;
    int remaind_size;
    unsigned char* charFeature=(unsigned char*)minutia;

    FeatureVector feavector;
    FeatureVector *p_feavector = (FeatureVector*)&feavector;

    p_feavector->num = minutia[0];
    if( p_feavector->num > MAX_MINUTIAE )
        p_feavector->num = MAX_MINUTIAE;

    remaind_size=((MAX_MINUTIAE-p_feavector->num)<<2)>>3;

    // 계산된 remaind_size 가 feature_num보다 크면 보정해야 함!
    if( remaind_size > p_feavector->num )
        remaind_size = p_feavector->num;

    for(i=1; i<=p_feavector->num; i++)
    {
        p_feavector->x[i-1] = (minutia[i]>>20)&0x7FF;
        p_feavector->y[i-1] = (minutia[i]>>9)&0x7FF;
        p_feavector->angle[i-1] = (minutia[i])&0x1FF;
    }

    //Distance 구하기
    memset(Distance_count,0,sizeof(Distance_count));
    for(i=0; i<remaind_size; i++)
    {
        for(j=0; j<p_feavector->num; j++)
        {
            if( i == j )
                continue;
            x = abs(p_feavector->x[i]-p_feavector->x[j]);
            y = abs(p_feavector->y[i]-p_feavector->y[j]);
            dist=m_SqrtTable[y][x];
            count=Distance_count[dist];
            Distance[dist][count]=j+1;
            Distance_count[dist]+=1;

            if( dist < min_dist ) min_dist = dist;
            else if( dist > max_dist ) max_dist = dist;
        }
        n=0;
        for(j=min_dist; j<=max_dist; j++)
        {
            count = Distance_count[j];
            if(count<=0)continue;

            for(m=0; m<count; m++)
            {
                p_feavector->neighbor_info[i].minutiae_num[n] = Distance[j][m];
                n++;
                if(n>=D_NEIGHBOR)
                {
                    j=MAX_DIAGONAL_LENGTH;
                    break;
                }
            }
        }
        memset(Distance_count+min_dist,0,max_dist-min_dist+1);
        for(j=0; j<D_NEIGHBOR; j++)
        {
            charFeature[(p_feavector->num+1)*4+i*8+j]=(unsigned char)p_feavector->neighbor_info[i].minutiae_num[j];
        }
    }

	delete [] Distance;
	delete [] Distance_count;
}
/*
void CIBSMAlgorithm::_ImageFlipVertically(unsigned char *pixels_buffer, const int width, const int height)
{
	const int rows = height / 2;		// Iterate only half the buffer to get a full flip
	const int row_stride = width;
    unsigned char* temp_row = (unsigned char*)malloc(row_stride);

    int source_offset, target_offset;

    for (int rowIndex = 0; rowIndex < rows; rowIndex++)
    {
        source_offset = rowIndex * row_stride;
        target_offset = (height - rowIndex - 1) * row_stride;

        memcpy(temp_row, pixels_buffer + source_offset, row_stride);
        memcpy(pixels_buffer + source_offset, pixels_buffer + target_offset, row_stride);
        memcpy(pixels_buffer + target_offset, temp_row, row_stride);
    }

    free(temp_row);
    temp_row = NULL;
}
*/