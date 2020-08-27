// LibCBP.h : main header file for the GAMC DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "CBP_FP.h"
#include "IBScanUltimateApi.h"
#include "CBP_FP_Global_Defs.h"
#include <vector>

#include "INIManager.h"

// CLibCBPApp
// See LibCBP.cpp for the implementation of this class
//

class CLibCBPApp : public CWinApp
{
public:
	CLibCBPApp();

// Overrides
public:
	virtual ~CLibCBPApp(void);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Callback functions.
	/// Please refer to IBScanUltimateApi_defs.h in include folder.
	static void CALLBACK OnEvent_DeviceCommunicationBreak(const int deviceHandle, void* pContext);
	static void CALLBACK OnEvent_PreviewImage(const int deviceHandle, void* pContext, const IBSU_ImageData image);
	static void CALLBACK OnEvent_FingerCount(const int deviceHandle, void* pContext, const IBSU_FingerCountState fingerCountState);
	static void CALLBACK OnEvent_FingerQuality(const int deviceHandle, void *pContext, const IBSU_FingerQualityState *pQualityArray, const int qualityArrayCount);
	static void CALLBACK OnEvent_DeviceCount(const int detectedDevices, void *pContext);
	static void CALLBACK OnEvent_InitProgress(const int deviceIndex, void *pContext, const int progressValue);
	static void CALLBACK OnEvent_TakingAcquisition(const int deviceHandle, void *pContext, const IBSU_ImageType imageType);
	static void CALLBACK OnEvent_CompleteAcquisition(const int deviceHandle, void *pContext, const IBSU_ImageType imageType);
    static void CALLBACK OnEvent_ResultImageEx(const int deviceHandle, void *pContext, const int imageStatus, const IBSU_ImageData image, 
												const IBSU_ImageType imageType, const int detectedFingerCount, const int segmentImageArrayCount, 
												const IBSU_ImageData *pSegmentImageArray, const IBSU_SegmentPosition *pSegmentPositionArray);
	static void CALLBACK OnEvent_ClearPlatenAtCapture(const int deviceHandle, void *pContext, const IBSU_PlatenState platenState);
	static void CALLBACK OnEvent_AsyncOpenDevice(const int deviceIndex, void *pContext, const int deviceHandle, const int errorCode);
	static void CALLBACK OnEvent_NotifyMessage(const int deviceHandle, void *pContext, const int message);
	static void CALLBACK OnEvent_PressedKeyButtons(const int deviceHandle, void *pContext, const int pressedKeyButtons);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	virtual BOOL InitInstance();
	virtual BOOL ExitInstance();
	void	 callback_error(int err_code, char *error_location);
	void	 callback_warning(int warn_code, char *warn_location);
	void     DLL_calibrate(int handle);
	void     DLL_capture(int handle, cbp_fp_slapType slapType, cbp_fp_collectionType collectionType);
	void     DLL_close(int handle);
	void     DLL_configure(int handle); // lock required
	void     DLL_enumDevices(char *requestID);
	void     DLL_getDirtiness(int handle);  // lock required
	void     DLL_getLockInfo(int handle);
	char*    DLL_getProperty(int handle, char *name);  // no callback
	void     DLL_initialize(char *requestID);
	void     DLL_lock(int handle);
	void     DLL_open(struct cbp_fp_device *device);
	void     DLL_powerSave(int handle, bool powerSaveOn); // lock required
	int      DLL_registerCallBacks(struct cbp_fp_callBacks *callBacks); // no callback
	void     DLL_setProperty(int handle, struct cbp_fp_property *property); // no callback
	void     DLL_startImaging(int handle, cbp_fp_slapType slapType, cbp_fp_collectionType collectionType);
	void     DLL_stopImaging(int handle);
	void     DLL_uninitialize(char *requestID);
	void     DLL_unlock(int handle);

	void	_LogMessage(char *msg, char *flag, char *result, int requiredDebugLevel);
	void	_GetAnalysisCodeString(int code, char * codeString);
	void	_GetSlapTypeCodeString(cbp_fp_slapType code, char * codeString);
	void	_GetCollectionTypeCodeString(cbp_fp_collectionType code, char * codeString);

	// control LEDs
	void	_ControlLEDforKojak(int handle, IBSU_FingerQualityState *fingerQualityState, int finger_count, int ledColor,
								cbp_fp_slapType slapType, cbp_fp_collectionType collectionType);

	// lock functions
	BOOL	_CheckDuplicate_LockInfo(int handle, long processID);
	int		_AddToArr_LockInfo(int handle, long processID);
	int		_DeleteFromArr_LockInfo(int handle, long processID);
	int		_FindFromArr_LockInfo(int handle, long processID, CBPLockInfo *lockinfo);

	void	_DrawLine(unsigned char *Img, int sx, int sy, int ex, int ey, int width, int height, unsigned char color);

	// debug fuction
	void	_Read_INI_Info();
	void	_LogManage();
	
	CString _Get_DLL_PATH();
	CString _Get_EXE_PATH();

	// static thread fucntion
	static UINT _THREAD_cbp_fp_calibrate(LPVOID pParam);
	static UINT _THREAD_cbp_fp_initialize(LPVOID pParam);
	static UINT _THREAD_cbp_fp_capture(LPVOID pParam);
	static UINT _THREAD_cbp_fp_close(LPVOID pParam);
	static UINT _THREAD_cbp_fp_configure(LPVOID pParam);
	static UINT _THREAD_cbp_fp_enumDevices(LPVOID pParam);
	static UINT _THREAD_cbp_fp_getDirtiness(LPVOID pParam);
	static UINT _THREAD_cbp_fp_getLockInfo(LPVOID pParam);
	//static UINT _THREAD_cbp_fp_getProerty(LPVOID pParam);
	//static UINT _THREAD_cbp_fp_setProperty(LPVOID pParam);
	static UINT _THREAD_cbp_fp_lock(LPVOID pParam);
	static UINT _THREAD_cbp_fp_open(LPVOID pParam);
	static UINT _THREAD_cbp_fp_powerSave(LPVOID pParam);
	//static UINT _THREAD_cbp_fp_registerCallBacks(LPVOID pParam);
	static UINT _THREAD_cbp_fp_startImaging(LPVOID pParam);
	static UINT _THREAD_cbp_fp_stopImaging(LPVOID pParam);
	static UINT _THREAD_cbp_fp_uninitialize(LPVOID pParam);
	static UINT _THREAD_cbp_fp_unlock(LPVOID pParam);

	ThreadParam_cbp_fp_calibrate	*pParam_calibrate;
	ThreadParam_cbp_fp_capture		*pParam_capture;
	ThreadParam_cbp_fp_close		*pParam_close;
	ThreadParam_cbp_fp_configure	*pParam_configure;
	ThreadParam_cbp_fp_enumDevices	*pParam_enumDevices;
	ThreadParam_cbp_fp_getDirtiness *pParam_getDirtiness;
	ThreadParam_cbp_fp_getLockInfo	*pParam_getLockInfo;
	ThreadParam_cbp_fp_initialize	*pParam_initialize;
	ThreadParam_cbp_fp_lock			*pParam_lock;
	ThreadParam_cbp_fp_open			*pParam_open;
	ThreadParam_cbp_fp_powerSave	*pParam_powerSave;
	ThreadParam_cbp_fp_startImaging	*pParam_startImaging;
	ThreadParam_cbp_fp_stopImaging	*pParam_stopImaging;
	ThreadParam_cbp_fp_uninitialize	*pParam_uninitialize;
	ThreadParam_cbp_fp_unlock		*pParam_unlock;

	CBPUsbDevice* _GetUsbDevicesFromHandle(int handle);
	CBPUsbDevice* _GetUsbDevicesFromRequestID(char *requestID);
	CBPUsbDevice* _GetUsbDevicesFromNotOpened();
	BOOL _DeleteUsbDeviceFromRequestID(char *requestID);
	BOOL _DeleteUsbDeviceFromHandle(int handle);

	std::vector<CBPUsbDevice*>	m_CBPUsbDevices;
	cbp_fp_callBacks			m_Callbacks;
	int							m_DebugLevel;
	char						m_DebugFileName[CBP_FP_MAX_PROPERTY_VALUE_MAX*4];
	char						m_CurrentFolder[CBP_FP_MAX_PROPERTY_VALUE_MAX*4];
	char						m_strRet[2048];//CBP_FP_MAX_PROPERTY_VALUE_MAX];
	char						m_Message[2048];
	int							m_AddBlinkIndex;
	int							m_DebugFileSizeLimit;
	int							m_DebugFileCount;
	
	CINIManager					m_INIMgr;
	CString						m_VFoundLogFiles[MAX_LOG_COUNT_INDEX+1]; // Put log files in vector<string>
	int							m_VFoundLogFilesCount;
	int							m_IsValidGeoCnt;
	int							m_Score100Count;
public:

	DECLARE_MESSAGE_MAP()
};
