#pragma once

#ifndef __ISO_19794_DEFINE_H__
#define __ISO_19794_DEFINE_H__

#include <string>
#include <math.h>
#include <sstream>
#include <vector>

//Main definitions
#define MAX_MINUTIAE_BLOCKS						256			// 1 to 255
#define MAX_CERTIFICATION_BLOCKS                256			// 0 to 255
#define MAX_QUALITY_BLOCKS                      256			// 0 to 255
#define MAX_EXTENDED_BLOCKS						16			// 
#define MAX_RIDGEDATA_BLOCKS					256			// 1 to 255
#define MAX_COREDATA_BLOCKS						16			// 0 to 15
#define MAX_DELTADATA_BLOCKS					16			// 0 to 15
#define MAX_COORDINATES_BLOCKS					100			// 4 to 99
#define MAX_FINGERSEGMENT_BLOCKS				8			// 0 to 4 and 255
#define MAX_FINGERANNOTATIONS_BLOCKS			8			// 1 to 4


//FINGER  REPRESENTATIONS
#define MAX_TEMPLATE_REP                         352
#define MIN_TEMPLATE_REP                           1
#define MAX_RECORD_REP                           672
#define MIN_RECORD_REP                             1
//QualityBlocks
#define MAX_VALID_REP_NUM                         15
#define MIN_HORIZ_IMG_SAMPLERATE                  98
#define MIN_VERT_IMG_SAMPLERATE                   98
#define MAX_IMG_WIDTH_PIXELS                   16383
#define MAX_IMG_HEIGHT_PIXELS                  16383
#define MAX_NUM_MINUTIAE                         255 
#define pow2_32                           4294967296
#define pow2_16                                65536
//ExtendedData
#define MAX_NUM_CORES                             15
#define MAX_NUM_DELTAS                            15

//EXTENDED DATA AREA TYPE CODE
#define RIDGE_COUNT_DATA				0x0001
#define CORE_AND_DELTA_DATA				0x0002
#define ZONAL_QUALITY_DATA				0x0003

//Error Codes
#define FILE_OPEN_ERROR                      2
#define INCORRECT_FORMATID                   3
#define INCORRECT_VERSION_NUMBER             4
#define INVALID_RECORD_LENGTH                5
#define INVALID_NUMBER_OF_REPS               6
#define DEVICE_CERTIFICATION_FLAG_ZERO       7
#define INVALID_NUM_OF_DISTINCT_REPS         8
#define INVALID_REP_LENGTH                   9
#define INVALID_DEVICE_TECH_ID              10
#define INVALID_DATETIME                    11
#define INVALID_QUALITY_BLK_NUM             12 
#define INVALID_SCORE                       13
#define INVALID_CERTIFICATION_BLK_NUM       14
#define RESERVED_ID_USED                    15  
#define INVALID_FINGER_POSITION             16
#define INVALID_REP_NUMBER                  17
#define INVALID_SCANNER_SAMPLE_RATE         18
#define INVALID_IMG_SAMPLE_RATE             19
#define INVALID_IMPRESSION_TYPE             20
#define INVALID_SIZE                        21
#define INVALID_MINUTIAE_FIELD_LENGTH       22
#define INVALID_RIDGE_END_TYPE              23
#define INVALID_NUM_MINUTIAE                24
#define INVALID_MINUTIAE_TYPE               25
#define INVALID_LOCATION                    26
#define INVALID_ANGLE                       27
#define INVALID_MINUTIAE_QUALITY            28
#define INVALID_BIT_DEPTH                   29
#define INVALID_COMPRESSION_ALGM            30 
#define INVALID_SCALE_UNITS                 31
#define INVALID_DATA_LENGTH                 32  
#define INVALID_REQUEST                     33
/*********Codes for Extended data validation********/
#define MISMATCHED_Code_BLOCK               34
#define INVALID_NUM_SEGMENTS                35
#define INVALID_NUM_ANNOTATIONS             36
#define INVALID_NUM_COORDINATES             37
#define RESERVED_TYPE_CODE                  38
#define INVALID_INDEX                       39
#define INVALID_RIDGE_EXTRACT_TYPE          40
#define INVALID_NUM_CORES                   41
#define INVALID_NUM_DELTAS                  42
#define INVALID_CELL_SIZE                   43
#define INVALID_SEQUENCE                    44//previous 1st index is > than the new index1 to be added
#define INVALID_ANNOTATION_CODE             45

//#define SUCCESS                            100
#define L_ENDIAN                           0
#define B_ENDIAN                           1

#pragma pack(push,1) 

using namespace std;

struct BT4_4
{
	BYTE val:4;
	BYTE Val2:4;
};

struct BT2_14//cant use for fread since it causes Big Endian effect
{
	WORD ValLBits : 14;
	WORD ValMBits : 2;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Finger Image Quality. 5 bytes
typedef struct tag_REPQUALITY
{	
	BYTE QualityScore;//255 = failed attempt to clculate quality score, 0-100 range
	WORD QualityAlgmVendorId; //registered id with IBIA
	WORD QualityAlgmId;//0000 to FFFF vendor algorithm and version
} REPQUALITY;

//Certification block 3 bytes. Exists if cert flag is set and number of certifications is greater than zero
typedef struct tag_CERTDETAILS
{
	WORD CertAuthID; //registered by IBIA
	BYTE CertSchemeId;//Use table 6 Image quality specification for identification or verification 
} CERTDETAILS;

//Qualified finger minutiae pixel record
typedef struct tag_MINUTIAERECORD
{
	//5 or 6 byte data representation for each minutiae
	WORD MinutiaeType : 2;
	WORD XPosition : 14;
	WORD Reserved : 2;
	WORD YPosition : 14;
	BYTE Angle;//1.40625 (360/256) degrees. Range [0-255]
	BYTE MinutiaeQuality;//254 =  quality not reported, 255 = failure to acquie quality score. Range [0,100]. exists if minutiaefield length is set to 6 byte
} MINUTIAERECORD;

//Extended Data. Exists if extendeddatablk length in finger rep header is greater than zero
typedef struct tag_EXTENDEDDATA
{
	WORD TypeCode;
	WORD BlkLength; //includes area type code and area length code	
	void *ExtendedData;
} EXTENDEDDATA;

//Extended data for ridge count
typedef struct tag_RIDGEDATA
{
	BYTE index1;
	BYTE index2;
	BYTE RidgeCount;
} RIDGEDATA;

typedef struct tag_RIDGECOUNT
{
	BYTE ExtractionType;
	//3 byte data block for each minutiae pair 
	RIDGEDATA RData[MAX_RIDGEDATA_BLOCKS];
} RIDGECOUNT;

//Extended data for core and delta 4 05 bytes
typedef struct tag_COREDATA
{
	//4 or 5 byte data block for each core
	WORD XPosition : 14;
	WORD CoreInfoType : 2; //00 or 01
	WORD YPosition : 14;
	WORD ReservedY : 2;//0	
	BYTE Angle; //1.40625 [0,255]//exists if coreinfotype is set
} COREDATA;

//Delta record 4 or 7 bytes
typedef struct tag_DELTADATA
{	
	//4 or 7 byte data block for each delta. Since angle information depends of DeltaInfoType
	WORD XPosition : 14;
	WORD DeltaInfoType : 2;//00 or 01
	WORD YPosition : 14;
	WORD ReservedY : 2; //0
	BYTE Angles[3];//If all 3 angles not available then angle fields  filled by repeating other angles for the delta//exists if deltainfotype is set
} DELTADATA;

typedef struct tag_COREDELTAFORMAT
{
	BYTE NumCoresLSB : 4; //[0-15]
	BYTE NumCoresMSBReserved : 4;
	COREDATA CData[MAX_COREDATA_BLOCKS];
	BYTE NumDeltaLSB : 4;
	BYTE NumDeltaMSBReserved : 4;
	DELTADATA DData[MAX_DELTADATA_BLOCKS];
} COREDELTAFORMAT;

//Zonal quality
typedef struct tag_ZONALQUALITY
{
	WORD QualityVendorID;
	WORD QualityAlgorithmID;
	BYTE CellWidth;//1 to 255
	BYTE CellHeight;//1 to 255
	BYTE QualityBitDepth; //1 to 8 bits
	BYTE *CellQuality;//insert array for cell quality bits data arranged in raster order, The data is packed into bytes and last is left justified with unused bits set to 0.
} ZONALQUALITY;

//Vendore format 4 bit 4 bit to store class information of the fingerprint which is defined as a combination of
//2 different classes hence two 4 bit representation each half byte between [0-5][0-5] 
typedef struct tag_VENDORFORMAT
{
	BYTE Quality;	
	BYTE class1 : 4;
	BYTE class2 : 4;
	BYTE Pitch;
	BYTE CorDeltaDist;
	//Insert comment explaining the different classification indexes used for fingerprints
} VENDORFORMAT;

//General header of 15 t\bytes
typedef struct tag_FMR_GENERALHEADER
{
	BYTE FormatIdentifier[4];
	BYTE VersionNumber[4];
	DWORD RecordLength;
	WORD NumRepresentation;
	BYTE DeviceCertificationFlag;//0 implies none representations contain certification record, 1 implies representations contain certification record
} FMR_GENERALHEADER;

//Finger representation header. Variable length
typedef struct tag_FINGERREPRESENTATION
{
	DWORD RepLength;
	BYTE CaptureDateTime[9];//yyyy.mm.dd hr.min.sec.0000 hrs is in 24 hr format
	BYTE CapDevTechID;//00 unknown, 13-Electro luminescent
	WORD CapDevVendorId;//00 - unreported vendor
	WORD CapDevTypeID;//00 - unreported type by the vendor
	BYTE NumQualityBlocks;
	REPQUALITY FQuality[MAX_QUALITY_BLOCKS];
	BYTE NumCert; //exists if certification flagis set in general header
	CERTDETAILS Cert[MAX_CERTIFICATION_BLOCKS];
	BYTE FingerPosition; //use table 7, 0 - unknown finger
	BYTE RepNumber;
	WORD XSamplingRate; //pixels/cm min 98pp/cm (250 pixels/inch)
	WORD YSamplingRate; //pixels/cm (250 pixels/inch)
	BYTE ImpressionType;
	WORD Width; //in pixels
	WORD Height; // in pixels
	BYTE MinutiaeFieldLength : 4; // valid values are 5 0r 6
	BYTE RidgeEndingType : 4; //0 or 1
	BYTE NumberOfMinutiae;
	MINUTIAERECORD Record[MAX_MINUTIAE_BLOCKS];
	WORD ExtendedDataLength;//lenght of entire extended data in bytes	
	EXTENDEDDATA ExtInfo[MAX_EXTENDED_BLOCKS];
} FINGERREPRESENTATION;

typedef struct tag_ISO_FMR_ORG
{
	FMR_GENERALHEADER gh;
	vector<FINGERREPRESENTATION> FR;
} ISO_FMR_ORG;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Extended data for segmentation format. In my opinion it is for multifinger image data
struct COORDINATES
{
	WORD X;
	WORD Y;
};

struct FINGERSEGMENT
{
	BYTE FingerPosition; //[0-10] from table 6
	BYTE FingerQuality;//[0-100] 255= failed attempt to calculate score, 254 = no attempt made to calculate score
	BYTE NumCoordinates;//[4-99]
	COORDINATES XY[MAX_COORDINATES_BLOCKS];
	//WORD XCoordinate; //horizontal pixel offset to right wrt origin of img
	//WORD YCoordinate;; //Vertical offset down relative to origin of img
	BYTE FingerOrientation;//precision of 1.40625 [0-255]
};

struct SEGMENTATION
{
	DWORD SegmentationQualityAlgmVendor_AlgmID;
	BYTE SegmentationQualityScore; //score to assess segmented location of the finger
	DWORD ImgQualityVendor_AlgmID;//00 00 00 00 = id not reported
	BYTE NumSegments; //255 = segmentation attempt failed [0,1,2,3,4] possible values. explanation is not very clear
	FINGERSEGMENT FgSgm[MAX_FINGERSEGMENT_BLOCKS];
};

//Extended data for annotation data format.
struct FINGERANNOTATIONS
{
	BYTE FingerPosition;
	BYTE AnnotationCode;
};

struct ANNOTATION
{
	BYTE NumAnnotations;
	//Each annotation has 2 byte data
	FINGERANNOTATIONS AnnoInfo[MAX_FINGERANNOTATIONS_BLOCKS];// Table 6 thru 8  = single finger to multi finger /palm
	//BYTE AnnotationCode; // 01Hex amputated to 02Hex bandageges or unable to print
};

//Extended data for comment
struct COMMENT
{
	string comment;//length of data  in extended data blck structure determines the size of the comment data which is lengthOfData - 4 bytes to exclude type and length variables
};

//General Header 16 bytes
class FIR_GENERALHEADER
{
public:
	BYTE FormatIdentifier[4];
	BYTE VersionNumber[4];
	DWORD RecordLength;
	WORD NumRepresentations;
	bool CertificationFlag;
	BYTE NumDistinctPositions;
};

//Representation Header
class REPRESENTATIONHEADER
{
public:
	DWORD RepLength;
	BYTE CaptureDateTime[9];//yy.mm.dd hr.min.sec.0000 hrs is in 24 hr format
	BYTE CaptureDeviceTechID;
	WORD CaptureDeviceVendorID;//00 = vendor not reported
	WORD CaptureDeviceTypeID; //00 = type unreported by product owner
	BYTE NumQualityBlocks;
	REPQUALITY Qualityb[MAX_QUALITY_BLOCKS];
	BYTE NumCertifications; // Exists only if certification flag is set
	CERTDETAILS cert[MAX_CERTIFICATION_BLOCKS];
	BYTE FingerPosition;
	BYTE RepresentationNumber;
	BYTE ScaleUnits;// 01Hex ppi, 02hex ppcm
	WORD DeviceXSamplingRate;
	WORD DeviceYSamplingRate;
	WORD ImageXSamplingRate;
	WORD ImageYSamplingRate;
	BYTE BitDepth;// 1 - 16 bits
	BYTE ImgCompressionAlgm;
	BYTE ImpressionType;
	WORD HorizontalLineLength;//Num of pixels in horizontal line of transmitted image
	WORD VerticalLineLength; //Num of vertical lines in transmitted image
	DWORD ImgDataLength;
	BYTE *ImgData;
	EXTENDEDDATA ExtInfo[MAX_EXTENDED_BLOCKS];
};

typedef struct tag_ISO_FIR_ORG
{
	FIR_GENERALHEADER gh;
	vector<REPRESENTATIONHEADER> RP;
}
ISO_FIR_ORG;
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma pack(pop)

#endif //__ISO_19794_DEFINE_H__