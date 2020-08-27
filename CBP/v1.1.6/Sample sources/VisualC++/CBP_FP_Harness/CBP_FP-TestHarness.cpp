#include "stdafx.h"
#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include "CBP_FP.h"
#include "CBP_FP-TestHarness.h"

// @todo put in DLL of the implimented wrapping layer
const wchar_t * dllPath = L"CBP_FP_Wrapper.dll";

HINSTANCE scannerDriverDll = NULL;

// pointers types for explicit referencing of DLL functions
typedef int (STDCALL *FP_CBP_FP_REGISTERCALLBACKS)(struct cbp_fp_callBacks *callBacks);
typedef void (STDCALL *FP_CBP_FP_INITIALIZE)(char *requestID);
typedef void (STDCALL *FP_CBP_FP_UNINITIALIZE)(char *requestID);
typedef void (STDCALL *FP_CBP_FP_ENUMDEVICES)(char *requestID);
typedef void (STDCALL *FP_CBP_FP_OPEN)(struct cbp_fp_device *device);
typedef void (STDCALL *FP_CBP_FP_CLOSE)(int handle);
typedef void (STDCALL *FP_CBP_FP_SETPROPERTY)(int handle, struct cbp_fp_property *property);
typedef char * (STDCALL *FP_CBP_FP_GETPROPERTY)(int handle, char *name);
typedef void (STDCALL *FP_CBP_FP_LOCK)(int handle);
typedef void (STDCALL *FP_CBP_FP_UNLOCK)(int handle);
typedef void (STDCALL *FP_CBP_FP_GETLOCKINFO)(int handle);
typedef void (STDCALL *FP_CBP_FP_CALIBRATE)(int handle);
typedef void (STDCALL *FP_CBP_FP_CONFIGURE)(int handle);
typedef void (STDCALL *FP_CBP_FP_GETDIRTINESS)(int handle);
typedef void (STDCALL *FP_CBP_FP_POWERSAVE)(int handle, bool powerSaveOn);
typedef void (STDCALL *FP_CBP_FP_STARTIMAGING)(int handle, cbp_fp_slapType slapType, cbp_fp_collectionType collectionType);
typedef void (STDCALL *FP_CBP_FP_STOPIMAGING)(int handle);
typedef void (STDCALL *FP_CBP_FP_CAPTURE)(int handle, cbp_fp_slapType slapType, cbp_fp_collectionType collectionType);

FP_CBP_FP_REGISTERCALLBACKS fp_cbp_fp_registercallbacks;
FP_CBP_FP_INITIALIZE fp_cbp_fp_initialize;
FP_CBP_FP_UNINITIALIZE fp_cbp_fp_uninitialize;
FP_CBP_FP_ENUMDEVICES fp_cbp_fp_enumdevices;
FP_CBP_FP_OPEN fp_cbp_fp_open;
FP_CBP_FP_CLOSE fp_cbp_fp_close;
FP_CBP_FP_SETPROPERTY fp_cbp_fp_setproperty;
FP_CBP_FP_GETPROPERTY fp_cbp_fp_getproperty;
FP_CBP_FP_LOCK fp_cbp_fp_lock;
FP_CBP_FP_UNLOCK fp_cbp_fp_unlock;
FP_CBP_FP_GETLOCKINFO fp_cbp_fp_getlockinfo;
FP_CBP_FP_CALIBRATE fp_cbp_fp_calibrate;
FP_CBP_FP_CONFIGURE fp_cbp_fp_configure;
FP_CBP_FP_GETDIRTINESS fp_cbp_fp_getdirtiness;
FP_CBP_FP_POWERSAVE fp_cbp_fp_powersave;
FP_CBP_FP_STARTIMAGING fp_cbp_fp_startimaging;
FP_CBP_FP_STOPIMAGING fp_cbp_fp_stopimaging;
FP_CBP_FP_CAPTURE fp_cbp_fp_capture;

// list of API name space
#define CBP_FP_REGISTERCALLBACKS "cbp_fp_registerCallBacks"
#define CBP_FP_INITIALIZE "cbp_fp_initialize"
#define CBP_FP_UNINITIALIZE "cbp_fp_uninitialize"
#define CBP_FP_ENUMDEVICES "cbp_fp_enumDevices"
#define CBP_FP_OPEN "cbp_fp_open"
#define CBP_FP_CLOSE "cbp_fp_close"
#define CBP_FP_SETPROPERTY "cbp_fp_setProperty"
#define CBP_FP_GETPROPERTY "cbp_fp_getProperty"
#define CBP_FP_LOCK "cbp_fp_lock"
#define CBP_FP_UNLOCK "cbp_fp_unlock"
#define CBP_FP_GETLOCKINFO "cbp_fp_getLockInfo"
#define CBP_FP_CALIBRATE "cbp_fp_calibrate"
#define CBP_FP_CONFIGURE "cbp_fp_configure"
#define CBP_FP_GETDIRTINESS "cbp_fp_getDirtiness"
#define CBP_FP_POWERSAVE "cbp_fp_powerSave"
#define CBP_FP_STARTIMAGING "cbp_fp_startImaging"
#define CBP_FP_STOPIMAGING "cbp_fp_stopImaging"
#define CBP_FP_CAPTURE "cbp_fp_capture"


void getTextofWindowsError(DWORD errorNumber, char *messageBuffer);
void GetAnalysisCodeString(int code, char * codeString);

////////////////////////////////////////////////////////////////////
// cbp_fp_onClose()
////////////////////////////////////////////////////////////////////
void cbp_fp_onClose(int handle)
{
	printf(">>> cbp_fp_onClose - Successfully closed the device\n");

	fp_cbp_fp_uninitialize("vendorTest");
}

////////////////////////////////////////////////////////////////////
// cbp_fp_onError()
////////////////////////////////////////////////////////////////////
void cbp_fp_onError(int errorCode, char * errorMessage)
{
	// Fatal error
	printf(">>> cbp_fp_onError (%d) %s\n", errorCode, errorMessage);

}

////////////////////////////////////////////////////////////////////
// cbp_fp_onWarning()
////////////////////////////////////////////////////////////////////
void cbp_fp_onWarning(int warningCode, char * errorMessage)
{
	// Non-fatal error.  Just a warning.
	printf(">>> cbp_fp_onWarning (%d) %s\n", warningCode, errorMessage);
}


////////////////////////////////////////////////////////////////////
// cbp_fp_onInitialize()
////////////////////////////////////////////////////////////////////
void cbp_fp_onInitialize(char *requestID)
{
	printf(">>> cbp_fp_onInitialize() - Successfully initialized the device\n");

	fp_cbp_fp_enumdevices(requestID);
}


////////////////////////////////////////////////////////////////////
// cbp_fp_onCalibrate()
////////////////////////////////////////////////////////////////////
void cbp_fp_onCalibrate(int handle)
{
	printf(">>> cbp_fp_onCalibrate() - Successfully calibrated the device\n");
}


////////////////////////////////////////////////////////////////////
// cbp_fp_onUninitialize()
////////////////////////////////////////////////////////////////////
void cbp_fp_onUninitialize(char *requestID)
{
	printf(">>> cbp_fp_onUninitialize() - Successfully uninitialized the device\n");

	printf("\nHit <Enter> to exit\n");
}


////////////////////////////////////////////////////////////////////
// cbp_fp_onCapture()
////////////////////////////////////////////////////////////////////
void cbp_fp_onCapture(int handle, cbp_fp_grayScaleCapture * grayScaleCapture)
{
	printf(">>> cbp_fp_onCapture() - successfully captured the final slap iamge\n");
	printf(">>> final captured image dimension %d x %d\n", grayScaleCapture->width, grayScaleCapture->height);

	fp_cbp_fp_unlock(handle);

}


////////////////////////////////////////////////////////////////////
// cbp_fp_onConfigure()
////////////////////////////////////////////////////////////////////
void cbp_fp_onConfigure(int handle, bool configChanged)
{
	printf(">>> cbp_fp_onConfigure() Successfully configured the device\n");
}



////////////////////////////////////////////////////////////////////
// cbp_fp_OnEnumDevices()
////////////////////////////////////////////////////////////////////
void cbp_fp_OnEnumDevices(char *requestID, struct cbp_fp_device *devices[], int deviceListLen)
{

	printf("cbp_fp_OnEnumDevices() - Detected %d device(s) connected to the workstation\n", deviceListLen);
	fp_cbp_fp_open(NULL);//devices[0]);
}

////////////////////////////////////////////////////////////////////
// cbp_fp_onGetDirtiness()
////////////////////////////////////////////////////////////////////
void cbp_fp_onGetDirtiness(int handle, int level)
{
	printf(">>> cbp_fp_onGetDirtiness() - Dirtiness level %d\n", level);
}


////////////////////////////////////////////////////////////////////
// cbp_fp_onGetLockInfo()
////////////////////////////////////////////////////////////////////
void cbp_fp_onGetLockInfo(int handle, struct cbp_fp_lock_info * lockInfo)
{
	printf(">>> cbp_fp_onGetLockInfo() handle %d, PID %d, duration %d\n", handle, lockInfo->lockPID, lockInfo->lockDuration);
}


////////////////////////////////////////////////////////////////////
// cbp_fp_onLock()
////////////////////////////////////////////////////////////////////
void cbp_fp_onLock(int handle, int pid)
{
	printf(">>> cbp_fp_onLock() - Successfully locked the device. Handle %d PID %d\n", handle, pid);
}


////////////////////////////////////////////////////////////////////
// cbp_fp_onPreview()
////////////////////////////////////////////////////////////////////
void cbp_fp_onPreview(int handle, struct cbp_fp_grayScalePreview * grayScalePreview)
{
	// The cbp_fp_grayScalePreview contains the raw preview image slap and its dimension
	printf("cbp_fp_onPreview()...\n");
}


////////////////////////////////////////////////////////////////////
// cbp_fp_onPreviewAnalysis()
////////////////////////////////////////////////////////////////////
void cbp_fp_onPreviewAnalysis(int handle, struct cbp_fp_previewAnalysis * previewAnalysis)
{
	struct cbp_fp_fingerAttributes fingerAttributes;
	char codeString[100] = { 0 };

	printf("\n>>> cbp_fp_onPreviewAnalysis() - Vendor analysis data:\n");

	printf("CollectionType: %d\n", previewAnalysis->collectionType);
	printf("SlapType: %d\n", previewAnalysis->slapType);

	for (int i = 0; i < previewAnalysis->fingerAttributeListLen; i++)
	{
		fingerAttributes = previewAnalysis->fingerAttributeList[i];
		GetAnalysisCodeString(fingerAttributes.analysisCodeList[0], codeString);
		printf("FINGER [%d] %s, score %d\n", i, codeString, fingerAttributes.score);
	}
}



////////////////////////////////////////////////////////////////////
// GetAnalysisCodeString()
////////////////////////////////////////////////////////////////////
void GetAnalysisCodeString(int code, char * codeString)
{

	switch (code)
	{
	case CBP_FP_ANALYSIS_CODE_GOOD_PRINT:
		sprintf(codeString, "Good print");
		break;

	case CBP_FP_ANALYSIS_CODE_NO_PRINT:
		sprintf(codeString, "No print");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_DARK:
		sprintf(codeString, "Too dark");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_LIGHT:
		sprintf(codeString, "Too light");
		break;

	case CBP_FP_ANALYSIS_CODE_BAD_SHAPE:
		sprintf(codeString, "Bad shape");
		break;

	case CBP_FP_ANALYSIS_CODE_WRONG_SLAP:
		sprintf(codeString, "Wrong slap");
		break;

	case CBP_FP_ANALYSIS_CODE_BAD_POSITION:
		sprintf(codeString, "Bad position");
		break;

	case CBP_FP_ANALYSIS_CODE_ROTATE_CLOCKWISE:
		sprintf(codeString, "Rotate clockwise");
		break;

	case CBP_FP_ANALYSIS_CODE_ROTATE_COUNTERCLOCKWISE:
		sprintf(codeString, "Rotate counterclockwise");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_HIGH:
		sprintf(codeString, "Too high");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_LOW:
		sprintf(codeString, "Too low");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_LEFT:
		sprintf(codeString, "Too left");
		break;

	case CBP_FP_ANALYSIS_CODE_TOO_RIGHT:
		sprintf(codeString, "Too right");
		break;

	case CBP_FP_ANALYSIS_CODE_SPOOF_DETECTED:
		sprintf(codeString, "Spoof detected");
		break;

	default:
		sprintf(codeString, "Unknown code %d", code);
		break;
	}
}


////////////////////////////////////////////////////////////////////
// cbp_fp_onStopPreview()
////////////////////////////////////////////////////////////////////
void cbp_fp_onStopPreview(int handle)
{
	printf(">>> cbp_fp_onStopPreview() - Successfully interrupt the previewing process\n");
}

////////////////////////////////////////////////////////////////////
// cbp_fp_onUnLock()
////////////////////////////////////////////////////////////////////
void cbp_fp_onUnLock(int handle)
{
	printf(">>> cbp_fp_onUnLock() - Successfully unlocked the device. Handle %d\n", handle);

	fp_cbp_fp_close(handle);
}


////////////////////////////////////////////////////////////////////
// cbp_fp_onPowerSave()
////////////////////////////////////////////////////////////////////
void cbp_fp_onPowerSave(int handle, bool setting)
{
	printf(">>> cbp_fp_onPowerSave() - Successfully turned the power save mode on\n");
}



////////////////////////////////////////////////////////////////////
// cbp_fp_onOpen()
////////////////////////////////////////////////////////////////////
void cbp_fp_onOpen(int handle, struct cbp_fp_scanner_connection * scannerConnection)
{
	printf(">>> cbp_fp_onOpen() - Successfully opened the device\n");

	// Lock the device
	fp_cbp_fp_lock(handle);

	// calibrate device
	fp_cbp_fp_calibrate(handle);

	// Get lock information
	fp_cbp_fp_getlockinfo(handle);

	// Start the previewing proccess
	printf("\n>>> cbp_fp_onOpen() - Start the previewing. Place your hand on the device.\n\n");

	cbp_fp_slapType slapType = slap_rightHand;
	cbp_fp_collectionType collectionType = collection_flat;

	fp_cbp_fp_startimaging(handle, slapType, collectionType);

	// Simulate the preview process for 5 seconds, then capture the final image.
	// The cbp_fp_onCapture callback has the final captured image.
	// The returned data in the cbp_fp_onPreviewAnalysis callback has
	// individual vendor finger analysis data (score, attributes, etc.).  Some vendor
	// may not provide the analysis data.
	// Client application may decide not to use the vendor analysis data and 
	// do its own slap analysis.  In this case, the cbp_fp_onPreview callback has 
	// the raw slap image for the client application to use for finger analysis.
	Sleep(5000);

	printf(">>> cbp_fp_onOpen() - Capture the final image\n");
	// Call capture when threshold reached.Done with preview

	fp_cbp_fp_capture(handle, slapType, collectionType);
}


////////////////////////////////////////////////////////////////////
// initAPI()
////////////////////////////////////////////////////////////////////
void InitAPIs()
{
	fp_cbp_fp_registercallbacks = NULL;
	fp_cbp_fp_initialize = NULL;
	fp_cbp_fp_uninitialize = NULL;
	fp_cbp_fp_enumdevices = NULL;
	fp_cbp_fp_open = NULL;
	fp_cbp_fp_close = NULL;
	fp_cbp_fp_setproperty = NULL;
	fp_cbp_fp_getproperty = NULL;
	fp_cbp_fp_lock = NULL;
	fp_cbp_fp_unlock = NULL;
	fp_cbp_fp_getlockinfo = NULL;
	fp_cbp_fp_calibrate = NULL;
	fp_cbp_fp_configure = NULL;
	fp_cbp_fp_getdirtiness = NULL;
	fp_cbp_fp_powersave = NULL;
	fp_cbp_fp_startimaging = NULL;
	fp_cbp_fp_stopimaging = NULL;
	fp_cbp_fp_capture = NULL;
}

////////////////////////////////////////////////////////////////////
// getTextofWindowsError()
////////////////////////////////////////////////////////////////////
void getTextofWindowsError(DWORD errorNumber, char *messageBuffer)
{
	TCHAR * message;

	(void)FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNumber,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&message,
		0,
		NULL);

	if (message != NULL)
	{
		sprintf(messageBuffer, "(%d) \"%ls\"\n", GetLastError(), message);

		LocalFree(message);
	}
}

/////////////////////////////////////////////////////////////////////////////////
// LoadAPIs() - loads the DLL and retrieves the address of an exported functions
/////////////////////////////////////////////////////////////////////////////////
DWORD LoadAPIs(const wchar_t * dllPath)
{
	DWORD rVal = CBP_FP_OK;

	char message[256] = { 0 };
	char messageBuffer[256] = { 0 };

	scannerDriverDll = LoadLibrary(dllPath);

	if (scannerDriverDll == NULL)
	{
		rVal = GetLastError();

		printf(">>> Failed to load %s. Error %d\n", dllPath, rVal);
		return CBP_FP_ERROR;
	}

	printf("**** Library %s loaded successfully ****\n", dllPath);

	fp_cbp_fp_initialize = (FP_CBP_FP_INITIALIZE)GetProcAddress(scannerDriverDll, CBP_FP_INITIALIZE);
	if (!fp_cbp_fp_initialize)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_INITIALIZE);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_registercallbacks = (FP_CBP_FP_REGISTERCALLBACKS)GetProcAddress(scannerDriverDll, CBP_FP_REGISTERCALLBACKS);
	if (!fp_cbp_fp_registercallbacks)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_REGISTERCALLBACKS);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_uninitialize = (FP_CBP_FP_UNINITIALIZE)GetProcAddress(scannerDriverDll, CBP_FP_UNINITIALIZE);
	if (!fp_cbp_fp_uninitialize)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_UNINITIALIZE);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_enumdevices = (FP_CBP_FP_ENUMDEVICES)GetProcAddress(scannerDriverDll, CBP_FP_ENUMDEVICES);
	if (!fp_cbp_fp_enumdevices)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_ENUMDEVICES);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_open = (FP_CBP_FP_OPEN)GetProcAddress(scannerDriverDll, CBP_FP_OPEN);
	if (!fp_cbp_fp_open)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_OPEN);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_close = (FP_CBP_FP_CLOSE)GetProcAddress(scannerDriverDll, CBP_FP_CLOSE);
	if (!fp_cbp_fp_close)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_CLOSE);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_setproperty = (FP_CBP_FP_SETPROPERTY)GetProcAddress(scannerDriverDll, CBP_FP_SETPROPERTY);
	if (!fp_cbp_fp_setproperty)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_SETPROPERTY);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_getproperty = (FP_CBP_FP_GETPROPERTY)GetProcAddress(scannerDriverDll, CBP_FP_GETPROPERTY);
	if (!fp_cbp_fp_getproperty)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_GETPROPERTY);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_lock = (FP_CBP_FP_LOCK)GetProcAddress(scannerDriverDll, CBP_FP_LOCK);
	if (!fp_cbp_fp_lock)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_LOCK);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_unlock = (FP_CBP_FP_UNLOCK)GetProcAddress(scannerDriverDll, CBP_FP_UNLOCK);
	if (!fp_cbp_fp_unlock)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_UNLOCK);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_getlockinfo = (FP_CBP_FP_GETLOCKINFO)GetProcAddress(scannerDriverDll, CBP_FP_GETLOCKINFO);
	if (!fp_cbp_fp_getlockinfo)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_GETLOCKINFO);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_calibrate = (FP_CBP_FP_CALIBRATE)GetProcAddress(scannerDriverDll, CBP_FP_CALIBRATE);
	if (!fp_cbp_fp_calibrate)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_CALIBRATE);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_configure = (FP_CBP_FP_CONFIGURE)GetProcAddress(scannerDriverDll, CBP_FP_CONFIGURE);
	if (!fp_cbp_fp_configure)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_CONFIGURE);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_getdirtiness = (FP_CBP_FP_GETDIRTINESS)GetProcAddress(scannerDriverDll, CBP_FP_GETDIRTINESS);
	if (!fp_cbp_fp_getdirtiness)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_GETDIRTINESS);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_powersave = (FP_CBP_FP_POWERSAVE)GetProcAddress(scannerDriverDll, CBP_FP_POWERSAVE);
	if (!fp_cbp_fp_powersave)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_POWERSAVE);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_startimaging = (FP_CBP_FP_STARTIMAGING)GetProcAddress(scannerDriverDll, CBP_FP_STARTIMAGING);
	if (!fp_cbp_fp_startimaging)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_STARTIMAGING);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_stopimaging = (FP_CBP_FP_STOPIMAGING)GetProcAddress(scannerDriverDll, CBP_FP_STOPIMAGING);
	if (!fp_cbp_fp_stopimaging)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_STOPIMAGING);
		rVal = GetLastError();
		goto cleanup;
	}

	fp_cbp_fp_capture = (FP_CBP_FP_CAPTURE)GetProcAddress(scannerDriverDll, CBP_FP_CAPTURE);
	if (!fp_cbp_fp_capture)
	{
		sprintf(message, "error - could not load function %s\n", CBP_FP_CAPTURE);
		rVal = GetLastError();
		goto cleanup;
	}

cleanup:

	if (message[0] != '\0')
	{
		printf("LoadAPIs() - %s\n", message);

		if (rVal != CBP_FP_OK)
		{
			getTextofWindowsError(rVal, messageBuffer);
			printf("LoadAPIs() - System error %s\n", messageBuffer);
		}
	}
	else
	{
		if (rVal == CBP_FP_OK)
		{
			printf("LoadAPIs() success.\n");
		}
		else
		{
			printf("LoadAPIs() failed - %d\n", rVal);
		}

	}

	return rVal;
}


//////////////////////////////////////////////////////////////////////////
// UnloadAPIs() - Unloads and frees up memory of the specified device DLL
//////////////////////////////////////////////////////////////////////////
DWORD UnloadAPIs()
{
	DWORD rVal;
	DWORD freeSuccess;

	rVal = CBP_FP_OK;

	if (scannerDriverDll != NULL)
	{
		freeSuccess = FreeLibrary(scannerDriverDll);
		if (freeSuccess == 0)
		{
			rVal = GetLastError();
		}
	}

	printf("TPLoader.DLL: unloadAPIs() success\n");

	return rVal;
}


///////////////////////////////////////////////////////////////////////////
// main() - Test program to exercise all the Vendor DLL
// and to demonstrate CBP FP API usages.
///////////////////////////////////////////////////////////////////////////
int main()
{
	DWORD rVal;

	struct cbp_fp_callBacks * callbacks;

	InitAPIs();

	rVal = LoadAPIs(dllPath);

	if (rVal != CBP_FP_OK)
	{
		printf("Failed to load APIs %d\n", rVal);
		return CBP_FP_ERROR;
	}

	callbacks = (struct cbp_fp_callBacks *) malloc(sizeof(struct cbp_fp_callBacks));;
	memset(callbacks, 0, sizeof(struct cbp_fp_callBacks));

	callbacks->cbp_fp_onInitialize = cbp_fp_onInitialize;
	callbacks->cbp_fp_onUninitialize = cbp_fp_onUninitialize;
	callbacks->cbp_fp_onOpen = cbp_fp_onOpen;
	callbacks->cbp_fp_onClose = cbp_fp_onClose;
	callbacks->cbp_fp_onError = cbp_fp_onError;
	callbacks->cbp_fp_onWarning = cbp_fp_onWarning;
	callbacks->cbp_fp_onCalibrate = cbp_fp_onCalibrate;
	callbacks->cbp_fp_onCapture = cbp_fp_onCapture;
	callbacks->cbp_fp_onConfigure = cbp_fp_onConfigure;
	callbacks->cbp_fp_onEnumDevices = cbp_fp_OnEnumDevices;
	callbacks->cbp_fp_onGetDirtiness = cbp_fp_onGetDirtiness;
	callbacks->cbp_fp_onGetLockInfo = cbp_fp_onGetLockInfo;
	callbacks->cbp_fp_onLock = cbp_fp_onLock;
	callbacks->cbp_fp_onPreview = cbp_fp_onPreview;
	callbacks->cbp_fp_onPreviewAnalysis = cbp_fp_onPreviewAnalysis;
	callbacks->cbp_fp_onStopPreview = cbp_fp_onStopPreview;
	callbacks->cbp_fp_onUnLock = cbp_fp_onUnLock;
	callbacks->cbp_fp_onPowerSave = cbp_fp_onPowerSave;

	// Register all the callbacks
	rVal = fp_cbp_fp_registercallbacks(callbacks);
	if (rVal != CBP_FP_OK)
	{
		printf(">>> Failed to register callbacks\n");
		return CBP_FP_ERROR;
	}

	// Initialize the device
	fp_cbp_fp_initialize("vendorTest");

	getchar();
	printf("\nProgram is terminated\n");

	// cleanup
	UnloadAPIs();

	free(callbacks);
	callbacks = NULL;

	return CBP_FP_OK;

}