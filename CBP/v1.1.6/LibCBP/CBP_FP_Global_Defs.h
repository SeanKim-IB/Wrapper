#ifndef __CBP_FP_GLOBAL_DEFS_H__
#define __CBP_FP_GLOBAL_DEFS_H__

#include "IBScanUltimateApi.h"

#define INI_FILE_NAME					"CBP_FP_Wrapper.ini"
#define STRING_SCANNER_MAKE				"Integrated Biometrics"
#define MAX_LOG_SIZE					(2024*1024)
#define MAX_LOG_COUNT_INDEX				3
#define pszLinuxFileVersion				"1.1.0.0"
#define NONE_FINGER						0xFF
#define LEFT_LITTLE						0x00
#define LEFT_RING						0x01
#define LEFT_MIDDLE						0x02
#define LEFT_INDEX						0x03
#define LEFT_THUMB						0x04
#define RIGHT_THUMB						0x05
#define RIGHT_INDEX						0x06
#define RIGHT_MIDDLE					0x07
#define RIGHT_RING						0x08
#define RIGHT_LITTLE					0x09
#define LEFT_RING_LITTLE				0x0A
#define LEFT_INDEX_MIDDLE				0x0B
#define BOTH_THUMBS						0x0C
#define RIGHT_INDEX_MIDDLE				0x0D
#define RIGHT_RING_LITTLE				0x0E
#define LEFT_HAND						0x0F
#define THUMB_SUM						0x10
#define BOTH_LEFT_THUMB					0x11
#define BOTH_RIGHT_THUMB				0x12
#define RIGHT_HAND						0x13

#define __LED_COLOR_GREEN__				0
#define __LED_COLOR_RED__				1
#define __LED_COLOR_YELLOW__			2

#define	MAX_LOCK_INFO_COUNT				16

typedef struct tag_Point
{
    int x;
	int y;
}
Point;

// thread parameter
typedef struct {
	CWnd						*pWnd;
	int							handle;
}ThreadParam_cbp_fp_calibrate;

typedef struct {
	CWnd						*pWnd;
	int							handle;
	cbp_fp_slapType				slapType;
	cbp_fp_collectionType		collectionType;
}ThreadParam_cbp_fp_capture;
	
typedef struct {
	CWnd						*pWnd;
	int							handle;
}ThreadParam_cbp_fp_close;

typedef struct {
	CWnd						*pWnd;
	int							handle;
}ThreadParam_cbp_fp_configure;

typedef struct {
	CWnd						*pWnd;
	char						*pRequestID;
}ThreadParam_cbp_fp_enumDevices;

typedef struct {
	CWnd						*pWnd;
	int							handle;
}ThreadParam_cbp_fp_getDirtiness;

typedef struct {
	CWnd						*pWnd;
	int							handle;
	//int							ThreadCount;
}ThreadParam_cbp_fp_getLockInfo;

typedef struct {
	CWnd						*pWnd;
	char						*pRequestID;
}ThreadParam_cbp_fp_initialize;

typedef struct {
	CWnd						*pWnd;
	int							handle;
}ThreadParam_cbp_fp_lock;

typedef struct {
	CWnd						*pWnd;
	cbp_fp_device				*device;
}ThreadParam_cbp_fp_open;

typedef struct {
	CWnd						*pWnd;
	int							handle;
	bool						powerSaveOn;
}ThreadParam_cbp_fp_powerSave;

typedef struct {
	CWnd						*pWnd;
	int							handle;
	cbp_fp_slapType				slapType;
	cbp_fp_collectionType		collectionType;
}ThreadParam_cbp_fp_startImaging;

typedef struct {
	CWnd						*pWnd;
	int							handle;
}ThreadParam_cbp_fp_stopImaging;

typedef struct {
	CWnd						*pWnd;
	char						*pRequestID;
}ThreadParam_cbp_fp_uninitialize;

typedef struct {
	CWnd						*pWnd;
	int							handle;
}ThreadParam_cbp_fp_unlock;

typedef struct {
	int							m_nDevHandle;
	char						m_sRequestID[CBP_FP_MAX_PROPERTY_VALUE_MAX];
	int							m_nFingerCount;
	BOOL						m_bInitialized;
	BOOL						m_bReceivedResultImage;
	BOOL						m_bLocked;
	IBSU_FingerQualityState		m_FingerQualityState[IBSU_MAX_SEGMENT_COUNT];
	cbp_fp_slapType				m_SlapType;				// latest slap type
	cbp_fp_collectionType		m_CollectionType;		// latest collection type
	BOOL						m_StopPreview;
	BOOL						m_ClearPlaten;
}CBPUsbDevice;

typedef struct {
	cbp_fp_lock_info			m_LockInfo;
	long						m_LockCreateTime;
} CBPLockInfo;

#endif