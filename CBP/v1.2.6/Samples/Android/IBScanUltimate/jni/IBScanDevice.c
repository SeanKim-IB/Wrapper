/* *************************************************************************************************
 * IBScanDevice.c
 *
 * DESCRIPTION:
 *     Android JNI wrapper for IBScanUltimate library
 *     http://www.integratedbiometrics.com
 *
 * NOTES:
 *     Copyright (c) Integrated Biometrics, 2013-2017
 *
 * HISTORY:
 *     2013/03/01  First version.
 *     2013/03/08  Changed name of LOG macro.
 *     2013/03/22  Added function to calculate NFIQ score.
 *     2013/03/26  Added function to get extended result image.
 *     2013/04/06  Resolved compiler warning by performing indirection of first
 *                 member of split image array than split image array itself.
 *     2013/10/18  Added captureImageExtended() native method.
 *                 Added callback for extended result information.
 *                 Added enableEvent() native method.
 *     2014/11/24  Added generateZoomOutImageExNative() native method.
 *     2015/04/07  Added getEnhancedImageReservedNative() native method.
 *                 Added setPropertyReservedNative() native method.
 *                 Added enumeration value to IBSU_ImageData.
 *                 (ProcessThres)
 *                 Added wsqEncodeToFileNative() native method.
 *     2015/12/11  Added function for detecting the key button of device was pressed.
 *                 Added getOperableBeeperNative() native method.
 *                 Added setBeeperNative() native method.
 *     2017/04/27  Added getCombineImageExNative() native method.
 *     2017/12/11  Added generateDisplayImageNative() native method.
 *     2018/04/27  Added removeFingerImageNative() native method.
 *                 Added addFingerImageNative() native method. 
 *                 Added isFingerDuplicatedNative() native method. 
 *                 Added isValidFingerGeometryNative() native method.
 *                 Added SaveBitmapImageNative() native method.
************************************************************************************************ */

#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "IBScanDevice.h"

#ifndef _WINDOWS
#include "LinuxPort.h"
#else
#include <Windows.h>
#endif

#include "IBScanUltimateApi.h"

#ifdef __android__
#include <android/log.h>
#include <android/bitmap.h>
#endif

/***************************************************************************************************
 * LOCAL MACROS
 **************************************************************************************************/

#define DEBUG       0
#define MODULE_NAME "ibscandevice"

#if DEBUG
	#ifdef __android__
		#define ANDROID_LOG(...) __android_log_print(ANDROID_LOG_INFO, MODULE_NAME, __VA_ARGS__)
		#define LOG(s) { ANDROID_LOG s; }
	#else
		#define LOG(s) { printf(MODULE_NAME ": "); printf s; fflush(stdout); }
	#endif
#else
	#define LOG(s)
#endif

#define USE_PARAM(p) (void)p

#define IBSCAN_JAVA_CLASS_PATH       "com/integratedbiometrics/ibscanultimate/IBScan"
#define IBSCANDEVICE_JAVA_CLASS_PATH "com/integratedbiometrics/ibscanultimate/IBScanDevice"
#define IBCOMMON_JAVA_CLASS_PATH     "com/integratedbiometrics/ibscancommon/IBCommon"
#define OBJECT_CLASS_PATH            "java/lang/Object"
#define INTEGER_CLASS_PATH           "java/lang/Integer"

/***************************************************************************************************
 * LOCAL VARIABLES
 **************************************************************************************************/

static JavaVM   *SavedJavaVM                                              = NULL;

static jclass    Class_Object                                             = NULL;

static jclass    Class_Integer                                            = NULL;
static jmethodID MethodID_Integer_Integer                                 = NULL;

static jclass    Class_IBScanDevice                                       = NULL;
static jfieldID  FieldID_IBScanDevice_handleNative                        = NULL;
static jmethodID MethodID_IBScanDevice_deviceCommunicationBroken          = NULL;
static jmethodID MethodID_IBScanDevice_deviceImagePreviewAvailable        = NULL;
static jmethodID MethodID_IBScanDevice_deviceFingerCountChanged           = NULL;
static jmethodID MethodID_IBScanDevice_deviceFingerQualityChanged         = NULL;
static jmethodID MethodID_IBScanDevice_deviceAcquisitionBegun             = NULL;
static jmethodID MethodID_IBScanDevice_deviceAcquisitionCompleted         = NULL;
static jmethodID MethodID_IBScanDevice_deviceImageResultAvailable         = NULL;
static jmethodID MethodID_IBScanDevice_deviceImageResultExtendedAvailable = NULL;
static jmethodID MethodID_IBScanDevice_devicePlatenStateChanged           = NULL;
static jmethodID MethodID_IBScanDevice_deviceWarningReceived              = NULL;
static jmethodID MethodID_IBScanDevice_devicePressedKeyButtons            = NULL;

static jclass    Class_IBScanDevice_NativeError                           = NULL;
static jfieldID  FieldID_IBScanDevice_NativeError_code                    = NULL;

static jclass    Class_IBScanDevice_LedState                              = NULL;
static jmethodID MethodID_IBScanDevice_LedState_LedState                  = NULL;

static jclass    Class_IBScanDevice_ImageData                             = NULL;
static jmethodID MethodID_IBScanDevice_ImageData_ImageData                = NULL;
static jfieldID  FieldID_IBScanDevice_ImageData_buffer                    = NULL;
static jfieldID  FieldID_IBScanDevice_ImageData_width                     = NULL;
static jfieldID  FieldID_IBScanDevice_ImageData_height                    = NULL;
static jfieldID  FieldID_IBScanDevice_ImageData_resolutionX               = NULL;
static jfieldID  FieldID_IBScanDevice_ImageData_resolutionY               = NULL;
static jfieldID  FieldID_IBScanDevice_ImageData_frameTime                 = NULL;
static jfieldID  FieldID_IBScanDevice_ImageData_pitch                     = NULL;
static jfieldID  FieldID_IBScanDevice_ImageData_bitsPerPixel              = NULL;
static jfieldID  FieldID_IBScanDevice_ImageData_format                    = NULL;
static jfieldID  FieldID_IBScanDevice_ImageData_isFinal                   = NULL;
static jfieldID  FieldID_IBScanDevice_ImageData_processThres              = NULL;

static jclass    Class_IBScanDevice_ImageFormat                           = NULL;
static jmethodID MethodID_IBScanDevice_ImageFormat_toCode                 = NULL;

static jclass    Class_IBScanDevice_RollingData                           = NULL;
static jmethodID MethodID_IBScanDevice_RollingData_RollingData            = NULL;

static jclass    Class_IBScanDevice_SegmentPosition                       = NULL;
static jmethodID MethodID_IBScanDevice_SegmentPosition_SegmentPosition    = NULL;

static jclass    Class_IBCommon_ImageDataExt                              = NULL;
static jmethodID MethodID_IBCommon_ImageDataExt_ImageDataExt              = NULL;

/***************************************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 **************************************************************************************************/

static jclass    findClass          (JNIEnv *pEnv, const char *name, BOOL *pOk);
static jfieldID  getFieldID         (JNIEnv *pEnv, jclass cclass, const char *name, const char *sig, BOOL *pOk);
static jmethodID getMethodID        (JNIEnv *pEnv, jclass cclass, const char *name, const char *sig, BOOL *pOk);
static void      setNativeError     (JNIEnv *pEnv, jobject obj, int code);
static long      getHandle          (JNIEnv *pEnv, jobject obj);
static jobject   convertImageData   (JNIEnv *pEnv, const IBSU_ImageData *pImage);
static BOOL      unconvertImageData (JNIEnv *pEnv, jobject imageData, IBSU_ImageData *pImage);
static jobject   convertImageDataExt(JNIEnv *pEnv, const IBSM_ImageData *pImage);
static jint      attachToVM         (void **env, BOOL *pAttached);
static void      detachFromVM       (BOOL attached);

static void CALLBACK callbackDeviceCommunicationBroken         (const int deviceHandle, void *pContext);
static void CALLBACK callbackDeviceImagePreviewAvailable       (const int deviceHandle, void *pContext,
    const IBSU_ImageData image);
static void CALLBACK callbackDeviceFingerCountChanged          (const int deviceHandle, void *pContext,
    const IBSU_FingerCountState fingerCountState);
static void CALLBACK callbackDeviceFingerQualityChanged        (const int deviceHandle, void *pContext,
    const IBSU_FingerQualityState *pQualityArray, const int qualityArrayCount);
static void CALLBACK callbackDeviceAcquisitionBegun            (const int deviceHandle, void *pContext,
    const IBSU_ImageType imageType);
static void CALLBACK callbackDeviceAcquisitionCompleted        (const int deviceHandle, void *pContext,
    const IBSU_ImageType imageType);
static void CALLBACK callbackDeviceImageResultAvailable        (const int deviceHandle, void *pContext,
    const IBSU_ImageData image, const IBSU_ImageType imageType,
    const IBSU_ImageData *pSplitImageArray, const int splitImageArrayCount);
static void CALLBACK callbackDevicePlatenStateChanged          (const int deviceHandle, void *pContext,
    const IBSU_PlatenState platenState);
static void CALLBACK callbackDeviceWarningReceived             (const int deviceHandle, void *pContext,
    const int notifyMessage);
static void CALLBACK callbackDeviceImageResultExtendedAvailable(const int deviceHandle, void *pContext,
	const int imageStatus, const IBSU_ImageData image, const IBSU_ImageType imageType,
	const int detectedFingerCount, const int segmentImageArrayCount, const IBSU_ImageData *pSegmentImageArray,
	const IBSU_SegmentPosition *pSegmentPositionArray);
static void CALLBACK callbackDevicePressedKeyButtons           (const int deviceHandle, void *pContext,
    const int pressedKeyButtons);

/***************************************************************************************************
 * GLOBAL FUNCTIONS
 **************************************************************************************************/

/*
 * Handle load of library.
 */
jboolean IBScanDevice_OnLoad
    (JavaVM *vm,
     void   *reserved)
{
    JNIEnv *pEnv;
    jint    res;
    BOOL    ok = TRUE;

    USE_PARAM(reserved);

    LOG(("%s\n", __FUNCTION__));

    /* Cache classes, field IDs, and method IDs. */
    res = (*vm)->GetEnv(vm, (void **)&pEnv, JNI_VERSION_1_2);
    if (res < 0)
    {
        LOG(("unable to get JNIEnv reference\n"));
        ok = FALSE;
    }
    else
    {
        /* Cache classes, field IDs, and method IDs. */
		Class_Object = findClass(pEnv, OBJECT_CLASS_PATH, &ok);
		Class_Integer = findClass(pEnv, INTEGER_CLASS_PATH, &ok);
		if (Class_Integer != NULL)
		{
			MethodID_Integer_Integer = getMethodID(pEnv, Class_Integer, "<init>", "(I)V", &ok);
		}
        Class_IBScanDevice = findClass(pEnv, IBSCANDEVICE_JAVA_CLASS_PATH, &ok);
        if (Class_IBScanDevice != NULL)
        {
            FieldID_IBScanDevice_handleNative                        = getFieldID (pEnv, Class_IBScanDevice, "m_handleNative",                             "J",     &ok);
            MethodID_IBScanDevice_deviceCommunicationBroken          = getMethodID(pEnv, Class_IBScanDevice, "callbackDeviceCommunicationBroken",          "()V",   &ok);
            MethodID_IBScanDevice_deviceImagePreviewAvailable        = getMethodID(pEnv, Class_IBScanDevice, "callbackDeviceImagePreviewAvailable",        "(L" IBSCANDEVICE_JAVA_CLASS_PATH "$ImageData;)V", &ok);
            MethodID_IBScanDevice_deviceFingerCountChanged           = getMethodID(pEnv, Class_IBScanDevice, "callbackDeviceFingerCountChanged",           "(I)V",  &ok);
            MethodID_IBScanDevice_deviceFingerQualityChanged         = getMethodID(pEnv, Class_IBScanDevice, "callbackDeviceFingerQualityChanged",         "([I)V", &ok);
            MethodID_IBScanDevice_deviceAcquisitionBegun             = getMethodID(pEnv, Class_IBScanDevice, "callbackDeviceAcquisitionBegun",             "(I)V",  &ok);
            MethodID_IBScanDevice_deviceAcquisitionCompleted         = getMethodID(pEnv, Class_IBScanDevice, "callbackDeviceAcquisitionCompleted",         "(I)V",  &ok);
            MethodID_IBScanDevice_deviceImageResultAvailable         = getMethodID(pEnv, Class_IBScanDevice, "callbackDeviceImageResultAvailable",         "(L" IBSCANDEVICE_JAVA_CLASS_PATH "$ImageData;I[L" IBSCANDEVICE_JAVA_CLASS_PATH "$ImageData;)V", &ok);
            MethodID_IBScanDevice_devicePlatenStateChanged           = getMethodID(pEnv, Class_IBScanDevice, "callbackDevicePlatenStateChanged",           "(I)V",  &ok);
            MethodID_IBScanDevice_deviceWarningReceived              = getMethodID(pEnv, Class_IBScanDevice, "callbackDeviceWarningReceived",              "(I)V",  &ok);
            MethodID_IBScanDevice_deviceImageResultExtendedAvailable = getMethodID(pEnv, Class_IBScanDevice, "callbackDeviceImageResultExtendedAvailable", "(IL" IBSCANDEVICE_JAVA_CLASS_PATH "$ImageData;II[L" IBSCANDEVICE_JAVA_CLASS_PATH "$ImageData;[L" IBSCANDEVICE_JAVA_CLASS_PATH "$SegmentPosition;)V", &ok);
    		MethodID_IBScanDevice_devicePressedKeyButtons            = getMethodID(pEnv, Class_IBScanDevice, "callbackDevicePressedKeyButtons",            "(I)V",  &ok);
        }
        Class_IBScanDevice_NativeError = findClass(pEnv, IBSCANDEVICE_JAVA_CLASS_PATH "$NativeError", &ok);
        if (Class_IBScanDevice_NativeError != NULL)
        {
            FieldID_IBScanDevice_NativeError_code = getFieldID(pEnv, Class_IBScanDevice_NativeError, "code", "I", &ok);
        }
        Class_IBScanDevice_LedState = findClass(pEnv, IBSCANDEVICE_JAVA_CLASS_PATH "$LedState", &ok);
        if (Class_IBScanDevice_LedState != NULL)
        {
            MethodID_IBScanDevice_LedState_LedState = getMethodID(pEnv, Class_IBScanDevice_LedState, "<init>", "(IIJ)V", &ok);
        }
        Class_IBScanDevice_ImageData = findClass(pEnv, IBSCANDEVICE_JAVA_CLASS_PATH "$ImageData", &ok);
        if (Class_IBScanDevice_ImageData != NULL)
        {
            MethodID_IBScanDevice_ImageData_ImageData    = getMethodID(pEnv, Class_IBScanDevice_ImageData, "<init>", "([BIIDDDISIZI)V", &ok);
            FieldID_IBScanDevice_ImageData_buffer        = getFieldID(pEnv, Class_IBScanDevice_ImageData, "buffer",       "[B", &ok);
            FieldID_IBScanDevice_ImageData_width         = getFieldID(pEnv, Class_IBScanDevice_ImageData, "width",        "I",  &ok);
            FieldID_IBScanDevice_ImageData_height        = getFieldID(pEnv, Class_IBScanDevice_ImageData, "height",       "I",  &ok);
            FieldID_IBScanDevice_ImageData_resolutionX   = getFieldID(pEnv, Class_IBScanDevice_ImageData, "resolutionX",  "D",  &ok);
            FieldID_IBScanDevice_ImageData_resolutionY   = getFieldID(pEnv, Class_IBScanDevice_ImageData, "resolutionY",  "D",  &ok);
            FieldID_IBScanDevice_ImageData_frameTime     = getFieldID(pEnv, Class_IBScanDevice_ImageData, "frameTime",    "D",  &ok);
            FieldID_IBScanDevice_ImageData_pitch         = getFieldID(pEnv, Class_IBScanDevice_ImageData, "pitch",        "I",  &ok);
            FieldID_IBScanDevice_ImageData_bitsPerPixel  = getFieldID(pEnv, Class_IBScanDevice_ImageData, "bitsPerPixel", "S",  &ok);
            FieldID_IBScanDevice_ImageData_format        = getFieldID(pEnv, Class_IBScanDevice_ImageData, "format",       "L" IBSCANDEVICE_JAVA_CLASS_PATH "$ImageFormat;", &ok);
            FieldID_IBScanDevice_ImageData_isFinal       = getFieldID(pEnv, Class_IBScanDevice_ImageData, "isFinal",      "Z",  &ok);
            FieldID_IBScanDevice_ImageData_processThres  = getFieldID(pEnv, Class_IBScanDevice_ImageData, "processThres", "I",  &ok);
        }
        Class_IBScanDevice_ImageFormat = findClass(pEnv, IBSCANDEVICE_JAVA_CLASS_PATH "$ImageFormat", &ok);
        if (Class_IBScanDevice_ImageFormat != NULL)
        {
        	MethodID_IBScanDevice_ImageFormat_toCode = getMethodID(pEnv, Class_IBScanDevice_ImageFormat, "toCode", "()I", &ok);
        }
        Class_IBScanDevice_RollingData = findClass(pEnv, IBSCANDEVICE_JAVA_CLASS_PATH "$RollingData", &ok);
        if (Class_IBScanDevice_RollingData != NULL)
        {
            MethodID_IBScanDevice_RollingData_RollingData = getMethodID(pEnv, Class_IBScanDevice_RollingData, "<init>", "(II)V", &ok);
        }
        Class_IBScanDevice_SegmentPosition = findClass(pEnv, IBSCANDEVICE_JAVA_CLASS_PATH "$SegmentPosition", &ok);
        if (Class_IBScanDevice_SegmentPosition != NULL)
        {
            MethodID_IBScanDevice_SegmentPosition_SegmentPosition = getMethodID(pEnv, Class_IBScanDevice_SegmentPosition, "<init>", "(IIIIIIII)V", &ok);
        }
        Class_IBCommon_ImageDataExt = findClass(pEnv, IBCOMMON_JAVA_CLASS_PATH "$ImageDataExt", &ok);
        if (Class_IBCommon_ImageDataExt != NULL)
        {
            MethodID_IBCommon_ImageDataExt_ImageDataExt = getMethodID(pEnv, Class_IBCommon_ImageDataExt, "<init>", "(IIIISSSSSSSSBB[B)V", &ok);
        }
	}

    /* Save pointer to VM for use in callbacks. */
    SavedJavaVM = vm;

    /* Indicate whether initialization was successful. */
    return (ok);
}

/*
 * Perform additional steps to open device.
 */
void IBScanDevice_Open(
    JNIEnv  *pEnv,
    long     devHandle,
    jobject  devHandleJ)
{
    int     nRc;
    jobject devHandleJGlobal;

    LOG(("%s\n", __FUNCTION__));

	/* Get global handle to device. */
    devHandleJGlobal = (*pEnv)->NewGlobalRef(pEnv, devHandleJ);

    /* Register callbacks. */
    nRc = IBSU_RegisterCallbacks(devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK,
                    (void *)callbackDeviceCommunicationBroken, (void *)devHandleJGlobal);
    if (nRc < IBSU_STATUS_OK)
    {
        LOG(("unable to register COMMUNICATION_BREAK callback with error code %d\n", nRc));
    }
    nRc = IBSU_RegisterCallbacks(devHandle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE,
                    (void *)callbackDeviceImagePreviewAvailable, (void *)devHandleJGlobal);
    if (nRc < IBSU_STATUS_OK)
    {
        LOG(("unable to register PREVIEW_IMAGE callback with error code %d\n", nRc));
    }
    nRc = IBSU_RegisterCallbacks(devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT,
                    (void *)callbackDeviceFingerCountChanged, (void *)devHandleJGlobal);
    if (nRc < IBSU_STATUS_OK)
    {
        LOG(("unable to register PREVIEW_IMAGE callback with error code %d\n", nRc));
    }
    nRc = IBSU_RegisterCallbacks(devHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY,
                    (void *)callbackDeviceFingerQualityChanged, (void *)devHandleJGlobal);
    if (nRc < IBSU_STATUS_OK)
    {
        LOG(("unable to register FINGER_QUALITY callback with error code %d\n", nRc));
    }
    nRc = IBSU_RegisterCallbacks(devHandle, ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION,
                    (void *)callbackDeviceAcquisitionBegun, (void *)devHandleJGlobal);
    if (nRc < IBSU_STATUS_OK)
    {
        LOG(("unable to register TAKING_ACQUISITION callback with error code %d\n", nRc));
    }
    nRc = IBSU_RegisterCallbacks(devHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION,
                    (void *)callbackDeviceAcquisitionCompleted, (void *)devHandleJGlobal);
    if (nRc < IBSU_STATUS_OK)
    {
        LOG(("unable to register COMPLETE_ACQUISITION callback with error code %d\n", nRc));
    }
    nRc = IBSU_RegisterCallbacks(devHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE,
                    (void *)callbackDeviceImageResultAvailable, (void *)devHandleJGlobal);
    if (nRc < IBSU_STATUS_OK)
    {
        LOG(("unable to register COMPLETE_ACQUISITION callback with error code %d\n", nRc));
    }
    nRc = IBSU_RegisterCallbacks(devHandle, ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE,
                    (void *)callbackDevicePlatenStateChanged, (void *)devHandleJGlobal);
    if (nRc < IBSU_STATUS_OK)
    {
        LOG(("unable to register CLEAR_PLATEN_AT_CAPTURE callback with error code %d\n", nRc));
    }
    nRc = IBSU_RegisterCallbacks(devHandle, ENUM_IBSU_OPTIONAL_EVENT_NOTIFY_MESSAGE,
                    (void *)callbackDeviceWarningReceived, (void *)devHandleJGlobal);
    if (nRc < IBSU_STATUS_OK)
    {
        LOG(("unable to register NOTIFY_MESSAGE callback with error code %d\n", nRc));
    }
    nRc = IBSU_RegisterCallbacks(devHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX,
                    (void *)callbackDeviceImageResultExtendedAvailable, (void *)devHandleJGlobal);
    if (nRc < IBSU_STATUS_OK)
    {
        LOG(("unable to register RESULT_IMAGE callback with error code %d\n", nRc));
    }
    nRc = IBSU_RegisterCallbacks(devHandle, ENUM_IBSU_ESSENTIAL_EVENT_KEYBUTTON,
                    (void *)callbackDevicePressedKeyButtons, (void *)devHandleJGlobal);
    if (nRc < IBSU_STATUS_OK)
    {
        LOG(("unable to register KEY_BUTTON callback with error code %d\n", nRc));
    }
}

/***************************************************************************************************
 * JNI FUNCTIONS
 **************************************************************************************************/

/*
 * private native void closeNative(NativeError error);
 */
JNIEXPORT void JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(closeNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jobject  error)
{
    long devHandle;
    int  nRc;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_CloseDevice(devHandle);
    setNativeError(pEnv, error, nRc);
}

/*
 * private native void enableEvent(int eventTypeCode, boolean enable, NativeError error);
 */
JNIEXPORT void JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(enableEventNative)(
    JNIEnv  *pEnv,
	jobject  tthis,
	jint     eventCode,
	jboolean enable,
	jobject  error)
{
    long devHandle;
    int  nRc;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    if (enable)
    {
        void *callback = NULL;

        switch (eventCode)
    	{
			case ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK:
				callback = (void *)callbackDeviceCommunicationBroken;
				break;
			case ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE:
				callback = (void *)callbackDeviceImagePreviewAvailable;
				break;
			case ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT:
				callback = (void *)callbackDeviceFingerCountChanged;
				break;
			case ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY:
				callback = (void *)callbackDeviceFingerQualityChanged;
				break;
			case ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION:
				callback = (void *)callbackDeviceAcquisitionBegun;
				break;
			case ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION:
				callback = (void *)callbackDeviceAcquisitionCompleted;
				break;
			case ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE:
				callback = (void *)callbackDeviceImageResultAvailable;
				break;
			case ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE:
				callback = (void *)callbackDevicePlatenStateChanged;
				break;
			case ENUM_IBSU_OPTIONAL_EVENT_NOTIFY_MESSAGE:
				callback = (void *)callbackDeviceWarningReceived;
				break;
			case ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX:
				callback = (void *)callbackDeviceImageResultExtendedAvailable;
				break;
			case ENUM_IBSU_ESSENTIAL_EVENT_KEYBUTTON:
				callback = (void *)callbackDevicePressedKeyButtons;
				break;
			default:
				callback = NULL;
				break;
    	}
        if (callback == NULL)
        {
            nRc = IBSU_ERR_INVALID_PARAM_VALUE;
            LOG(("%s: unknown event code %d\n", __FUNCTION__, eventCode));
        }
        else
        {
            jobject devHandleJGlobal;

        	/* Get global handle to device. */
            devHandleJGlobal = (*pEnv)->NewGlobalRef(pEnv, tthis);
        	nRc = IBSU_RegisterCallbacks(devHandle, eventCode, callback, (void *)devHandleJGlobal);
        }
    }
    else
    {
    	nRc = IBSU_ReleaseCallbacks(devHandle, eventCode);
    }
    setNativeError(pEnv, error, nRc);
}

/*
 * private native void setPropertyNative(int propertyIdCode, String propertyValue,
 *     NativeError error);
 */
JNIEXPORT void JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(setPropertyNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jint     propertyIDCode,
	 jstring  propertyValue,
	 jobject  error)
{
    long        devHandle;
    int         nRc;
    const char *propertyValueC;

    LOG(("%s\n", __FUNCTION__));

    propertyValueC = (*pEnv)->GetStringUTFChars(pEnv, propertyValue, NULL);
    if (propertyValueC == NULL)
    {
        setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
        LOG(("%s: unable to get characters from string\n", __FUNCTION__));
    }
    else
    {
        devHandle = getHandle(pEnv, tthis);
        nRc       = IBSU_SetProperty(devHandle, (IBSU_PropertyId)propertyIDCode, (char *)propertyValueC);
        setNativeError(pEnv, error, nRc);
        (*pEnv)->ReleaseStringUTFChars(pEnv, propertyValue, propertyValueC);
    }
}

/*
 * private native void setPropertyReservedNative(String reservedKey, int propertyIdCode, String propertyValue,
 *     NativeError error);
 */
JNIEXPORT void JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(setPropertyReservedNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jstring  reservedKey,
	 jint     propertyIDCode,
	 jstring  propertyValue,
	 jobject  error)
{
	
    long        devHandle;
    int         nRc;
    const char *reservedKeyC;
    const char *propertyValueC;

    LOG(("%s\n", __FUNCTION__));

    reservedKeyC = (*pEnv)->GetStringUTFChars(pEnv, reservedKey, NULL);
    propertyValueC = (*pEnv)->GetStringUTFChars(pEnv, propertyValue, NULL);
    if (reservedKeyC == NULL)
    {
        setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
        LOG(("%s: unable to get characters from string\n", __FUNCTION__));
    }
    else
    {
	    if (propertyValueC == NULL)
	    {
	        setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
	        LOG(("%s: unable to get characters from string\n", __FUNCTION__));
	    }
	    else
	    {
	        devHandle = getHandle(pEnv, tthis);
	        nRc       = RESERVED_SetProperty(devHandle, (char *)reservedKeyC, (IBSU_PropertyId)propertyIDCode, (char *)propertyValueC);
	        setNativeError(pEnv, error, nRc);
	        (*pEnv)->ReleaseStringUTFChars(pEnv, propertyValue, propertyValueC);
	    }
        (*pEnv)->ReleaseStringUTFChars(pEnv, reservedKey, reservedKeyC);
	}
}

/*
 * private native String getPropertyNative(int propertyIdCode, NativeError error);
 */
JNIEXPORT jstring JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(getPropertyNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jint     propertyIDCode,
	 jobject  error)
{
    jstring propertyValueJ = NULL;
    char    propertyValue[IBSU_MAX_STR_LEN];
    long    devHandle;
    int     nRc;

    LOG(("%s\n", __FUNCTION__));

    memset(&propertyValue[0], 0, sizeof(propertyValue));
    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_GetProperty(devHandle, (IBSU_PropertyId)propertyIDCode, propertyValue);
    setNativeError(pEnv, error, nRc);
    if (nRc >= IBSU_STATUS_OK)
    {
        propertyValueJ = (*pEnv)->NewStringUTF(pEnv, propertyValue);
        if (propertyValueJ == NULL)
        {
            setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
            LOG(("%s: unable to allocate string\n", __FUNCTION__));
        }
    }

	return (propertyValueJ);
}

/*
 * private native boolean isCaptureAvailableNative(int imageTypeCode,
 *     int imageResolutionCode, NativeError error);
 */
JNIEXPORT jboolean JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(isCaptureAvailableNative)
	(JNIEnv  *pEnv,
     jobject  tthis,
     jint     imageTypeCode,
     jint     imageResolutionCode,
     jobject  error)
{
    jboolean modeAvailableJ;
    long     devHandle;
    int      nRc;
    BOOL     modeAvailable = 0;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_IsCaptureAvailable(devHandle, (IBSU_ImageType)imageTypeCode,
                    (IBSU_ImageResolution)imageResolutionCode, &modeAvailable);
    setNativeError(pEnv, error, nRc);

    modeAvailableJ = (modeAvailable != 0) ? JNI_TRUE : JNI_FALSE;

	return (modeAvailableJ);
}

/*
 * private native void beginCaptureImageNative(int imageTypeCode, int imageResolutionCode,
 *     int captureOptions, NativeError error);
 */
JNIEXPORT void JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(beginCaptureImageNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jint     imageTypeCode,
	 jint     imageResolutionCode,
	 jint     captureOptions,
	 jobject  error)
{
    long devHandle;
    int  nRc;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_BeginCaptureImage(devHandle, (IBSU_ImageType)imageTypeCode,
                    (IBSU_ImageResolution)imageResolutionCode, captureOptions);
    setNativeError(pEnv, error, nRc);
}

/*
 * private native void cancelCaptureImageNative(NativeError error);
 */
JNIEXPORT void JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(cancelCaptureImageNative)
	(JNIEnv  *pEnv,
     jobject  tthis,
     jobject  error)
{
    long devHandle;
    int  nRc;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_CancelCaptureImage(devHandle);
    setNativeError(pEnv, error, nRc);
}

/*
 * private native boolean isCaptureActiveNative(NativeError error);
 */
JNIEXPORT jboolean JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(isCaptureActiveNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jobject  error)
{
    jboolean isActiveJ;
    long     devHandle;
    int      nRc;
    BOOL     isActive = 0;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_IsCaptureActive(devHandle, &isActive);
    setNativeError(pEnv, error, nRc);

    isActiveJ = (isActive != 0) ? JNI_TRUE : JNI_FALSE;

    return (isActiveJ);
}

/*
 * private native void captureImageManuallyNative(NativeError error);
 */
JNIEXPORT void JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(captureImageManuallyNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jobject  error)
{
    long devHandle;
    int  nRc;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_TakeResultImageManually(devHandle);
    setNativeError(pEnv, error, nRc);
}

/*
 * private native int getContrastNative(NativeError error);
 */
JNIEXPORT jint JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(getContrastNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jobject  error)
{
    long devHandle;
    int  nRc;
    int  contrastValue = 0;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_GetContrast(devHandle, &contrastValue);
    setNativeError(pEnv, error, nRc);

	return (contrastValue);
}

/*
 * private native void setContrastNative(int contrastValue, NativeError error);
 */
JNIEXPORT void JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(setContrastNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jint     contrastValue,
	 jobject  error)
{
    long devHandle;
    int  nRc;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_SetContrast(devHandle, contrastValue);
    setNativeError(pEnv, error, nRc);
}

/*
 * private native void setLEOperationModeNative(int leOperationModeCode, NativeError error);
 */
JNIEXPORT void JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(setLEOperationModeNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jint     leOperationModeCode,
	 jobject  error)
{
    long devHandle;
    int  nRc;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_SetLEOperationMode(devHandle, (IBSU_LEOperationMode)leOperationModeCode);
    setNativeError(pEnv, error, nRc);
}

/*
 * private native int getLEOperationModeNative(NativeError error);
 */
JNIEXPORT jint JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(getLEOperationModeNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jobject  error)
{
    long                 devHandle;
    int                  nRc;
    IBSU_LEOperationMode leOperationMode = ENUM_IBSU_LE_OPERATION_AUTO;
    int                  leOperationModeCode;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_GetLEOperationMode(devHandle, &leOperationMode);
    setNativeError(pEnv, error, nRc);

    leOperationModeCode = (int)leOperationMode;
	return (leOperationModeCode);
}

/*
 * private native boolean isFingerTouchingNative(NativeError error);
 */
JNIEXPORT jboolean JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(isFingerTouchingNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jobject  error)
{
    jboolean touchInStatusJ;
    long     devHandle;
    int      nRc;
    int      touchInStatus = 0;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_IsTouchedFinger(devHandle, &touchInStatus);
    setNativeError(pEnv, error, nRc);

    touchInStatusJ = (touchInStatus != 0) ? JNI_TRUE : JNI_FALSE;

	return (touchInStatusJ);
}

/*
 * private native LedState getOperableLEDsNative(NativeError error);
 */
JNIEXPORT jobject JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(getOperableLEDsNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jobject  error)
{
    jobject      ledStateJ = NULL;
    long         devHandle;
    int          nRc;
    int          ledCount = 0;
    DWORD        operableLEDs = 0x00000000u;
    IBSU_LedType ledType = ENUM_IBSU_LED_TYPE_NONE;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_GetOperableLEDs(devHandle, &ledType, &ledCount, &operableLEDs);
    setNativeError(pEnv, error, nRc);

    if (nRc == IBSU_STATUS_OK)
    {
        ledStateJ = (*pEnv)->NewObject(pEnv, Class_IBScanDevice_LedState, MethodID_IBScanDevice_LedState_LedState, (jint)ledType, (jint)ledCount, (jlong)operableLEDs);
        if (ledStateJ == NULL)
        {
            setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
            LOG(("%s: unable to allocate LedState object\n", __FUNCTION__));
        }
    }

	return (ledStateJ);
}

/*
 * private native long getLEDsNative(NativeError error);
 */
JNIEXPORT jlong JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(getLEDsNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jobject  error)
{
    long  devHandle;
    int   nRc;
    DWORD activeLEDs = -1;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_GetLEDs(devHandle, &activeLEDs);
    setNativeError(pEnv, error, nRc);

	return (activeLEDs);
}

/*
 * private native void setLEDsNative(long activeLEDs, NativeError error);
 */
JNIEXPORT void JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(setLEDsNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jlong    activeLEDs,
	 jobject  error)
{
    long devHandle;
    int  nRc;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_SetLEDs(devHandle, (DWORD)activeLEDs);
    setNativeError(pEnv, error, nRc);
}

/*
 * private native Object[] captureImageNative(NativeError error);
 */
JNIEXPORT jobjectArray JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(captureImageNative)(
    JNIEnv  *pEnv,
	jobject  tthis,
	jobject  error)
{
	jobjectArray            returns = NULL;
    long                    devHandle;
    int                     nRc;
	IBSU_ImageData          image;
	IBSU_ImageType          imageType;
	IBSU_ImageData          splitImageArray[4];
	int                     splitImageArrayCount;
	IBSU_FingerCountState   fingerCountState;
	IBSU_FingerQualityState qualityArray[4];
	int                     qualityArrayCount;

    LOG(("%s\n", __FUNCTION__));

	devHandle = getHandle(pEnv, tthis);
	nRc       = IBSU_BGetImage(devHandle, &image, &imageType, &splitImageArray[0], 
					&splitImageArrayCount, &fingerCountState, &qualityArray[0], 
					&qualityArrayCount);
	setNativeError(pEnv, error, nRc);
	
	if (nRc == IBSU_STATUS_OK)
	{
        jobject imageJ;

        imageJ = convertImageData(pEnv, &image);
        if (imageJ == NULL)
        {
            LOG(("%s: unable to convert ImageData object\n", __FUNCTION__));
			setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
        }
        else
        {
            jobjectArray splitImageArrayJ;

            splitImageArrayJ = (*pEnv)->NewObjectArray(pEnv, splitImageArrayCount, Class_IBScanDevice_ImageData, NULL);
            if (splitImageArrayJ == NULL)
            {
                LOG(("%s: unable to allocate an array of objects\n", __FUNCTION__));
				setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
			}
            else
            {
                int  i;
                BOOL ok = TRUE;

                for (i = 0; i < splitImageArrayCount; i++)
                {
                    jobject imageTempJ;

                    imageTempJ = convertImageData(pEnv, &splitImageArray[i]);
                    if (imageTempJ == NULL)
                    {
                        LOG(("%s: unable to convert ImageData object\n", __FUNCTION__));
						setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
                        ok = FALSE;
                        break;
                    }
                    (*pEnv)->SetObjectArrayElement(pEnv, splitImageArrayJ, i, imageTempJ);
                }
                if (ok)
                {
					jintArray qualityArrayJ;

					qualityArrayJ = (*pEnv)->NewIntArray(pEnv, qualityArrayCount);
					if (qualityArrayJ == NULL)
					{
						LOG(("%s: unable to allocate data buffer of length %d\n", __FUNCTION__, qualityArrayCount));
						setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
					}
					else
					{
						jobject fingerCountStateJ;

						fingerCountStateJ = (*pEnv)->NewObject(pEnv, Class_Integer, MethodID_Integer_Integer, (jint)fingerCountState);
						if (fingerCountStateJ == NULL)
						{
							LOG(("%s: unable to create Integer object\n", __FUNCTION__));
							setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
						} 
						else
						{
							jobject imageTypeJ;

							imageTypeJ = (*pEnv)->NewObject(pEnv, Class_Integer, MethodID_Integer_Integer, (jint)imageType);
							if (imageTypeJ == NULL)
							{
								LOG(("%s: unable to create Integer object\n", __FUNCTION__));
								setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
							} 
							else
							{
								(*pEnv)->SetIntArrayRegion(pEnv, qualityArrayJ, 0, qualityArrayCount, (jint *)&qualityArray[0]);

								returns = (*pEnv)->NewObjectArray(pEnv, 5, Class_Object, NULL);
								if (returns == NULL)
								{
									LOG(("%s: unable to allocate object array of length 5\n", __FUNCTION__));
									setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
								}
								else
								{
									(*pEnv)->SetObjectArrayElement(pEnv, returns, 0, imageJ);
									(*pEnv)->SetObjectArrayElement(pEnv, returns, 1, imageTypeJ);
									(*pEnv)->SetObjectArrayElement(pEnv, returns, 2, splitImageArrayJ);
									(*pEnv)->SetObjectArrayElement(pEnv, returns, 3, fingerCountStateJ);
									(*pEnv)->SetObjectArrayElement(pEnv, returns, 4, qualityArrayJ);
								}
							}
						}
					}
				}
            }
        }
	}

	return (returns);
}

/*
 * private native Object[] captureImageExtendedNative(NativeError error);
 */
JNIEXPORT jobjectArray JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(captureImageExtendedNative)(
    JNIEnv  *pEnv,
	jobject  tthis,
	jobject  error)
{
	jobjectArray            returns = NULL;
    long                    devHandle;
    int                     nRc;
    int                     imageStatus;
	IBSU_ImageData          image;
	IBSU_ImageType          imageType;
	int                     detectedFingerCount;
	IBSU_ImageData          segmentImageArray[4];
	IBSU_SegmentPosition    segmentPositionArray[4];
	int                     segmentImageArrayCount;
	IBSU_FingerCountState   fingerCountState;
	IBSU_FingerQualityState qualityArray[4];
	int                     qualityArrayCount;

    LOG(("%s\n", __FUNCTION__));

	devHandle = getHandle(pEnv, tthis);
	nRc       = IBSU_BGetImageEx(devHandle, &imageStatus, &image, &imageType, &detectedFingerCount,
	                &segmentImageArray[0], &segmentPositionArray[0], &segmentImageArrayCount, 
	                &fingerCountState, &qualityArray[0], &qualityArrayCount);
	setNativeError(pEnv, error, nRc);
	
	if (nRc == IBSU_STATUS_OK)
	{
		jobject imageStatusJ;

		imageStatusJ = (*pEnv)->NewObject(pEnv, Class_Integer, MethodID_Integer_Integer, (jint)imageStatus);
		if (imageStatusJ == NULL)
		{
			LOG(("%s: unable to create Integer object\n", __FUNCTION__));
			setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
		} 
		else
		{
        	jobject imageJ;

        	imageJ = convertImageData(pEnv, &image);
        	if (imageJ == NULL)
        	{
            	LOG(("%s: unable to convert ImageData object\n", __FUNCTION__));
				setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
        	}
        	else
        	{
            	jobjectArray segmentImageArrayJ;

	            segmentImageArrayJ = (*pEnv)->NewObjectArray(pEnv, segmentImageArrayCount, Class_IBScanDevice_ImageData, NULL);
    	        if (segmentImageArrayJ == NULL)
        	    {
            	    LOG(("%s: unable to allocate an array of objects\n", __FUNCTION__));
					setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
				}
	            else
    	        {
        	        int  i;
            	    BOOL ok = TRUE;

                	for (i = 0; i < segmentImageArrayCount; i++)
                	{
                    	jobject imageTempJ;

	                    imageTempJ = convertImageData(pEnv, &segmentImageArray[i]);
    	                if (imageTempJ == NULL)
        	            {
            	            LOG(("%s: unable to convert ImageData object\n", __FUNCTION__));
							setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
                    	    ok = FALSE;
                        	break;
                    	}
                    	(*pEnv)->SetObjectArrayElement(pEnv, segmentImageArrayJ, i, imageTempJ);
	                }
    	            if (ok)
        	        {
						jintArray qualityArrayJ;

						qualityArrayJ = (*pEnv)->NewIntArray(pEnv, qualityArrayCount);
						if (qualityArrayJ == NULL)
						{
							LOG(("%s: unable to allocate data buffer of length %d\n", __FUNCTION__, qualityArrayCount));
							setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
						}
						else
						{
							jobject fingerCountStateJ;
	
							(*pEnv)->SetIntArrayRegion(pEnv, qualityArrayJ, 0, qualityArrayCount, (jint *)&qualityArray[0]);

							fingerCountStateJ = (*pEnv)->NewObject(pEnv, Class_Integer, MethodID_Integer_Integer, (jint)fingerCountState);
							if (fingerCountStateJ == NULL)
							{
								LOG(("%s: unable to create Integer object\n", __FUNCTION__));
								setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
							} 
							else
							{
								jobject imageTypeJ;

								imageTypeJ = (*pEnv)->NewObject(pEnv, Class_Integer, MethodID_Integer_Integer, (jint)imageType);
								if (imageTypeJ == NULL)
								{
									LOG(("%s: unable to create Integer object\n", __FUNCTION__));
									setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
								} 
								else
								{
									jobject detectedFingerCountJ;
									
									detectedFingerCountJ = (*pEnv)->NewObject(pEnv, Class_Integer, MethodID_Integer_Integer, (jint)detectedFingerCount);
									if (detectedFingerCountJ == NULL)
									{
										LOG(("%s: unable to create Integer object\n", __FUNCTION__));
										setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
									} 
									else
									{
										jobject segmentPositionArrayJ;
									
										segmentPositionArrayJ = (*pEnv)->NewObjectArray(pEnv, segmentImageArrayCount, Class_IBScanDevice_SegmentPosition, NULL);
										if (segmentPositionArrayJ == NULL)
										{
											LOG(("%s: unable to allocate object array of length %d\n", __FUNCTION__, segmentImageArrayCount));
											setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);										
										}
										else
										{										
        	        						ok = TRUE;

                							for (i = 0; i < segmentImageArrayCount; i++)
                							{
                    							jobject segmentPositionJ;

	                    						segmentPositionJ = (*pEnv)->NewObject(pEnv, Class_IBScanDevice_SegmentPosition, MethodID_IBScanDevice_SegmentPosition_SegmentPosition, 
	                    							(jint)segmentPositionArray[i].x1, (jint)segmentPositionArray[i].y1, (jint)segmentPositionArray[i].x2, (jint)segmentPositionArray[i].y2,
	                    							(jint)segmentPositionArray[i].x3, (jint)segmentPositionArray[i].y3, (jint)segmentPositionArray[i].x4, (jint)segmentPositionArray[i].y4);
    	                						if (segmentPositionJ == NULL)
        	            						{
            	            						LOG(("%s: unable to convert SegmentPosition object\n", __FUNCTION__));
													setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
                    	    						ok = FALSE;
                        							break;
                    							}
                    							(*pEnv)->SetObjectArrayElement(pEnv, segmentPositionArrayJ, i, segmentPositionJ);
	                						}
    	            						if (ok)
        	        						{
												returns = (*pEnv)->NewObjectArray(pEnv, 8, Class_Object, NULL);
												if (returns == NULL)
												{
													LOG(("%s: unable to allocate object array of length 8\n", __FUNCTION__));
													setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
												}
												else
												{
													(*pEnv)->SetObjectArrayElement(pEnv, returns, 0, imageStatusJ);
													(*pEnv)->SetObjectArrayElement(pEnv, returns, 1, imageJ);
													(*pEnv)->SetObjectArrayElement(pEnv, returns, 2, imageTypeJ);
													(*pEnv)->SetObjectArrayElement(pEnv, returns, 3, detectedFingerCountJ);
													(*pEnv)->SetObjectArrayElement(pEnv, returns, 4, segmentImageArrayJ);
													(*pEnv)->SetObjectArrayElement(pEnv, returns, 5, segmentPositionArrayJ);
													(*pEnv)->SetObjectArrayElement(pEnv, returns, 6, fingerCountStateJ);
													(*pEnv)->SetObjectArrayElement(pEnv, returns, 7, qualityArrayJ);
												}
											}
										}
									}
								}
							}
						}
					}
            	}
        	}
        }
	}

	return (returns);
}

/*
 * private native int getPlatenStateAtCaptureNative(NativeError error);
 */
JNIEXPORT jint JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(getPlatenStateAtCaptureNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jobject  error)
{
    long             devHandle;
    int              nRc;
	IBSU_PlatenState platenState;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_BGetClearPlatenAtCapture(devHandle, &platenState);
    setNativeError(pEnv, error, nRc);

	return ((jint)platenState);
}

/*
 * private native RollingData getRollingInfoNative(NativeError error);
 */
JNIEXPORT jobject JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(getRollingInfoNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jobject  error)
{
	jobject           rollingData = NULL;
    long              devHandle;
    int               nRc;
	IBSU_RollingState rollingState;
	int               rollingLineX;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_BGetRollingInfo(devHandle, &rollingState, &rollingLineX);
    setNativeError(pEnv, error, nRc);

	if (nRc == IBSU_STATUS_OK)
	{
		rollingData = (*pEnv)->NewObject(pEnv, Class_IBScanDevice_RollingData, 
			MethodID_IBScanDevice_RollingData_RollingData, (jint)rollingState, rollingLineX);
		if (rollingData == NULL)
		{
			LOG(("%s: unable to allocate RollingData object\n", __FUNCTION__));
			setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
		}
	}

	return (rollingData);
}

/*
 * private native int calculateNfiqScoreNative(ImageData image, NativeError error);
 */
JNIEXPORT jint JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(calculateNfiqScoreNative)(
    JNIEnv  *pEnv,
	jobject  tthis,
	jobject  image,
	jobject  error)
{
    IBSU_ImageData imageDataC;
    BOOL           ok;
    int            nfiqScore = 0;

    LOG(("%s\n", __FUNCTION__));

    /* Extract C image data from Java.  We will need to free the buffer later. */
    ok = unconvertImageData(pEnv, image, &imageDataC);
    if (!ok)
    {
        setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
    	LOG(("%s: unable to unconvert image\n", __FUNCTION__));
    }
    else
    {
        int  nRc;
        long devHandle;

		devHandle = getHandle(pEnv, tthis);
    	nRc       = IBSU_GetNFIQScore(devHandle, imageDataC.Buffer, imageDataC.Width,
                        imageDataC.Height, imageDataC.BitsPerPixel, &nfiqScore);
        setNativeError(pEnv, error, nRc);

        /* Free the buffer. */
        free(imageDataC.Buffer);
    }

    return (nfiqScore);
}

/*
 * private native Object[] getResultImageExtNative(int fingerPositionCode, NativeError error);
 */
JNIEXPORT jobjectArray JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(getResultImageExtNative)(
	JNIEnv  *pEnv,
	jobject  tthis,
	jint     fingerPositionCode,
	jobject  error)
{
	jobjectArray   returns = NULL;
	int            nRc;
	long           devHandle;
	IBSM_ImageData resultImage;
	IBSM_ImageData splitResultImages[4];
	int            splitResultCount;

	memset(&resultImage, 0, sizeof(resultImage));
	memset(&splitResultImages, 0, sizeof(splitResultImages));

	devHandle = getHandle(pEnv, tthis);
	nRc       = IBSU_GetIBSM_ResultImageInfo(devHandle, fingerPositionCode, &resultImage,
			&splitResultImages[0], &splitResultCount);
	setNativeError(pEnv, error, nRc);

	if (nRc == IBSU_STATUS_OK)
	{
        jobject resultImageJ;

        resultImageJ = convertImageDataExt(pEnv, &resultImage);
        if (resultImageJ == NULL)
        {
            LOG(("%s: unable to convert ImageDataExt object\n", __FUNCTION__));
			setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
        }
        else
        {
            jobjectArray splitResultImagesJ;

            splitResultImagesJ = (*pEnv)->NewObjectArray(pEnv, splitResultCount, Class_IBCommon_ImageDataExt, NULL);
            if (splitResultImagesJ == NULL)
            {
                LOG(("%s: unable to allocate an array of objects\n", __FUNCTION__));
				setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
			}
            else
            {
                int  i;
                BOOL ok = TRUE;

                for (i = 0; i < splitResultCount; i++)
                {
                    jobject imageTempJ;

                    imageTempJ = convertImageDataExt(pEnv, &splitResultImages[i]);
                    if (imageTempJ == NULL)
                    {
                        LOG(("%s: unable to convert ImageData object\n", __FUNCTION__));
						setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
                        ok = FALSE;
                        break;
                    }
                    (*pEnv)->SetObjectArrayElement(pEnv, splitResultImagesJ, i, imageTempJ);
                }
                if (ok)
                {
					returns = (*pEnv)->NewObjectArray(pEnv, 2, Class_Object, NULL);
					if (returns == NULL)
					{
						LOG(("%s: unable to allocate object array of length 2\n", __FUNCTION__));
						setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
					}
					else
					{
						(*pEnv)->SetObjectArrayElement(pEnv, returns, 0, resultImageJ);
						(*pEnv)->SetObjectArrayElement(pEnv, returns, 1, splitResultImagesJ);
					}
				}
            }
        }
	}

	return (returns);
}

/*
 * private native int getResultImageExtNative(..., NativeError error);
 */
JNIEXPORT jint JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(generateZoomOutImageExNative)(
	JNIEnv *pEnv,
	jobject tthis,
	jbyteArray inImage,
	jint inWidth,
	jint inHeight,
	jbyteArray outImage,
	jint outWidth,
	jint outHeight,
	jbyte bkColor,
	jobject  error)
{
	int nRc;
	//jobject imageJ = NULL;
	unsigned char *inImgData;

	IBSU_ImageData resultImage;
	resultImage.Buffer = (unsigned char*)malloc(sizeof(unsigned char)*outWidth*outHeight);
	resultImage.BitsPerPixel=8;
	resultImage.Width = outWidth;
	resultImage.Height = outHeight;
	resultImage.Format =0;
	resultImage.FrameTime = 60;
	resultImage.IsFinal =FALSE;
	resultImage.Pitch = -outWidth;
	resultImage.ResolutionX =500.0;
	resultImage.ResolutionY =500.0;

	inImgData = (unsigned char *)(*pEnv)->GetByteArrayElements(pEnv, inImage, NULL);
	

	nRc = IBSU_GenerateZoomOutImageEx(inImgData,inWidth,inHeight,resultImage.Buffer,outWidth,outHeight,bkColor);
    setNativeError(pEnv, error, nRc);

	if(nRc == IBSU_STATUS_OK)
	{
		(*pEnv)->SetByteArrayRegion(pEnv,outImage,0,outWidth*outHeight,(jbyte*)resultImage.Buffer);
	}
	free(resultImage.Buffer);
	(*pEnv)->ReleaseByteArrayElements(pEnv, inImage, (jbyte *)inImgData, 0);


	return nRc;
}
JNIEXPORT void JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(createBmpExNative)(
	JNIEnv *pEnv,
	jobject tthis,
	jbyteArray inImage,
	jobject bmpImage,
	jobject  error)
{
	int nRc;
	unsigned char *inImgData;
	inImgData = (unsigned char *)(*pEnv)->GetByteArrayElements(pEnv, inImage, NULL);

    setNativeError(pEnv, error, nRc);
	void* bitmapPixels;
	AndroidBitmapInfo info;
	int ret;

 	AndroidBitmap_getInfo(pEnv, bmpImage, &info);
	
	AndroidBitmap_lockPixels(pEnv, bmpImage, &bitmapPixels);
   
	BYTE* newBitmapPixels = (BYTE*) bitmapPixels;
	int x;
	int y;
	for ( y=0; y<info.height; y++)
	{
		for ( x=0; x<info.width; x++)
		  {
				newBitmapPixels[(y * info.width + x) * 4] =
				newBitmapPixels[(y * info.width + x) * 4 + 1] = 
				newBitmapPixels[(y * info.width + x) * 4 + 2] = 
										inImgData[(info.height - y - 1) * info.width + x];
				newBitmapPixels[(y * info.width + x) * 4 + 3] = (BYTE)255;
		  }
	}

	(*pEnv)->ReleaseByteArrayElements(pEnv, inImage, (jbyte *)inImgData, 0);
	AndroidBitmap_unlockPixels(pEnv, bmpImage);

}
/*
 * private native int wsqEncodeToFileNative(..., NativeError error);
 */
JNIEXPORT jint JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(wsqEncodeToFileNative)(
    JNIEnv    *pEnv,
    jobject    tthis,
	jstring    filePath,
    jbyteArray image,
	jint       width,
	jint       height,
	jint       pitch,
	jint       bitPerPixel,
	jint       pixelPerInch,
	jdouble    bitRate,
	jstring    commentText,
   jobject    error)
{
	unsigned char *imgData;
	int            nRc;
	const char *filePathC;
	const char *commentTextC;

    filePathC = (*pEnv)->GetStringUTFChars(pEnv, filePath, NULL);
    commentTextC = (*pEnv)->GetStringUTFChars(pEnv, commentText, NULL);
	//

	imgData = (unsigned char *)(*pEnv)->GetByteArrayElements(pEnv, image, NULL);
	if (imgData == NULL)
	{
      LOG(("%s: unable to convert WSQ imageData object\n", __FUNCTION__));
		setNativeError(pEnv, error, IBSU_ERR_INVALID_ACCESS_POINTER);
		return -1;
	}

	nRc       = IBSU_WSQEncodeToFile((char*)filePathC, imgData, width, height, pitch, bitPerPixel, pixelPerInch, bitRate, (char*)commentTextC);
    setNativeError(pEnv, error, nRc);

	(*pEnv)->ReleaseByteArrayElements(pEnv, image, (jbyte *)imgData, 0);
	(*pEnv)->ReleaseStringUTFChars(pEnv, filePath, filePathC);
	(*pEnv)->ReleaseStringUTFChars(pEnv, commentText, commentTextC);

	return nRc;
}

/*
 * private native Object[] getEnhancedImageNative(NativeError error);
 */
JNIEXPORT jobjectArray JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(getEnhancedImageReservedNative)(
    JNIEnv  *pEnv,
	jobject  tthis,
    jstring  reservedKey,
	jobject  image,
	jobject  error)
{
	jobjectArray            returns = NULL;
    long                    devHandle;
    int                     nRc;
    BOOL           			ok;
    IBSU_ImageData          imageDataC;
	IBSU_ImageData          enhancedImage;
	int                     segmentImageArrayCount;
	IBSU_ImageData          segmentImageArray[4];
	IBSU_SegmentPosition    segmentPositionArray[4];
    const char *reservedKeyC;

    LOG(("%s\n", __FUNCTION__));

    /* Extract C image data from Java.  We will need to free the buffer later. */
    ok = unconvertImageData(pEnv, image, &imageDataC);
    reservedKeyC = (*pEnv)->GetStringUTFChars(pEnv, reservedKey, NULL);
    if (reservedKeyC == NULL)
    {
        setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
        LOG(("%s: unable to get characters from string\n", __FUNCTION__));
    }
	else
	{
	    if (!ok)
	    {
	        setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
	    	LOG(("%s: unable to unconvert image\n", __FUNCTION__));
	    }
	    else
	    {
			devHandle = getHandle(pEnv, tthis);
			nRc       = RESERVED_GetEnhancedImage(devHandle, (char *)reservedKeyC, imageDataC, &enhancedImage,
			                &segmentImageArrayCount, &segmentImageArray[0], &segmentPositionArray[0]);
			setNativeError(pEnv, error, nRc);
			
			if (nRc == IBSU_STATUS_OK)
			{
		    	jobject enhancedImageJ;
		
		    	enhancedImageJ = convertImageData(pEnv, &enhancedImage);
		    	if (enhancedImageJ == NULL)
		    	{
		        	LOG(("%s: unable to convert ImageData object\n", __FUNCTION__));
					setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
		    	}
		    	else
		    	{
		        	jobjectArray segmentImageArrayJ;
		
		            segmentImageArrayJ = (*pEnv)->NewObjectArray(pEnv, segmentImageArrayCount, Class_IBScanDevice_ImageData, NULL);
			        if (segmentImageArrayJ == NULL)
		    	    {
		        	    LOG(("%s: unable to allocate an array of objects\n", __FUNCTION__));
						setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
					}
		            else
			        {
		    	        int  i;
		        	    BOOL ok = TRUE;
		
		            	for (i = 0; i < segmentImageArrayCount; i++)
		            	{
		                	jobject imageTempJ;
		
		                    imageTempJ = convertImageData(pEnv, &segmentImageArray[i]);
			                if (imageTempJ == NULL)
		    	            {
		        	            LOG(("%s: unable to convert ImageData object\n", __FUNCTION__));
								setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
		                	    ok = FALSE;
		                    	break;
		                	}
		                	(*pEnv)->SetObjectArrayElement(pEnv, segmentImageArrayJ, i, imageTempJ);
		                }
			            if (ok)
		    	        {
							jobject detectedFingerCountJ;
							
							detectedFingerCountJ = (*pEnv)->NewObject(pEnv, Class_Integer, MethodID_Integer_Integer, (jint)segmentImageArrayCount);
							if (detectedFingerCountJ == NULL)
							{
								LOG(("%s: unable to create Integer object\n", __FUNCTION__));
								setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
							} 
							else
							{
								jobject segmentPositionArrayJ;
							
								segmentPositionArrayJ = (*pEnv)->NewObjectArray(pEnv, segmentImageArrayCount, Class_IBScanDevice_SegmentPosition, NULL);
								if (segmentPositionArrayJ == NULL)
								{
									LOG(("%s: unable to allocate object array of length %d\n", __FUNCTION__, segmentImageArrayCount));
									setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);										
								}
								else
								{										
		    						ok = TRUE;
		
									for (i = 0; i < segmentImageArrayCount; i++)
									{
		    							jobject segmentPositionJ;
		
		        						segmentPositionJ = (*pEnv)->NewObject(pEnv, Class_IBScanDevice_SegmentPosition, MethodID_IBScanDevice_SegmentPosition_SegmentPosition, 
		        							(jint)segmentPositionArray[i].x1, (jint)segmentPositionArray[i].y1, (jint)segmentPositionArray[i].x2, (jint)segmentPositionArray[i].y2,
		        							(jint)segmentPositionArray[i].x3, (jint)segmentPositionArray[i].y3, (jint)segmentPositionArray[i].x4, (jint)segmentPositionArray[i].y4);
		        						if (segmentPositionJ == NULL)
		        						{
		            						LOG(("%s: unable to convert SegmentPosition object\n", __FUNCTION__));
											setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
		    	    						ok = FALSE;
		        							break;
		    							}
		    							(*pEnv)->SetObjectArrayElement(pEnv, segmentPositionArrayJ, i, segmentPositionJ);
		    						}
		    						if (ok)
		    						{
										returns = (*pEnv)->NewObjectArray(pEnv, 4, Class_Object, NULL);
										if (returns == NULL)
										{
											LOG(("%s: unable to allocate object array of length 4\n", __FUNCTION__));
											setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
										}
										else
										{
											(*pEnv)->SetObjectArrayElement(pEnv, returns, 0, enhancedImageJ);
											(*pEnv)->SetObjectArrayElement(pEnv, returns, 1, detectedFingerCountJ);
											(*pEnv)->SetObjectArrayElement(pEnv, returns, 2, segmentImageArrayJ);
											(*pEnv)->SetObjectArrayElement(pEnv, returns, 3, segmentPositionArrayJ);
										}
									}
								}
							}
		            	}
		        	}
				}
	        }
	        /* Free the buffer. */
	        free(imageDataC.Buffer);
		}
        (*pEnv)->ReleaseStringUTFChars(pEnv, reservedKey, reservedKeyC);
	}

	return (returns);
}
/*
 * private native int getcombineImageNative();
 */
JNIEXPORT jobject JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(getCombineImageNative)(
    JNIEnv  *pEnv,
	jobject  tthis,
    jobject  image1,
	jobject  image2,
	jint  whichHand,
	jobject  error)
{
	//jobject returns = NULL;
	jobject resultImageJ = NULL;
	int nRc;
	BOOL           ok;
	//long devHandle;
	IBSU_ImageData firstimage;
	IBSU_ImageData secondimage;
	IBSU_ImageData resultImage;

	memset(&resultImage,0,sizeof(resultImage));
	memset(&firstimage,0,sizeof(firstimage));
	memset(&secondimage,0,sizeof(secondimage));

    LOG(("%s\n", __FUNCTION__));

    /* Extract C image data from Java.  We will need to free the buffer later. */
    ok = unconvertImageData(pEnv,image1,&firstimage);
    if (!ok)
    {
        setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
    	LOG(("%s: unable to unconvert image\n", __FUNCTION__));
    }
    else
    {
		ok = unconvertImageData(pEnv,image2,&secondimage);
		 if (!ok)
		{
			setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
    		LOG(("%s: unable to unconvert image\n", __FUNCTION__));
		}
		else
		{
			nRc = IBSU_CombineImage(firstimage,secondimage,whichHand,&resultImage);
	
			if (nRc == IBSU_STATUS_OK)
			{
				resultImageJ = convertImageData(pEnv,&resultImage);
			}
			/* Free the buffer. */
			free(secondimage.Buffer);
		 }
		 /* Free the buffer. */
	     free(firstimage.Buffer);	 
	}


	return resultImageJ;


}

/*
 * private native int getcombineImageNative();
 */
JNIEXPORT jobject JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(getCombineImageExNative)(
    JNIEnv  *pEnv,
	jobject  tthis,
    jobject  image1,
	jobject  image2,
	jint  whichHand,
	jobject  error)
{
	jobject returns = NULL;
	int nRc;
	BOOL           ok;
	//long devHandle;
	IBSU_ImageData firstimage;
	IBSU_ImageData secondimage;
	IBSU_ImageData resultImage;
	int                     segmentImageArrayCount;
	IBSU_ImageData          segmentImageArray[4];
	IBSU_SegmentPosition    segmentPositionArray[4];

	memset(&resultImage,0,sizeof(resultImage));
	memset(&firstimage,0,sizeof(firstimage));
	memset(&secondimage,0,sizeof(secondimage));

    LOG(("%s\n", __FUNCTION__));

    /* Extract C image data from Java.  We will need to free the buffer later. */
    ok = unconvertImageData(pEnv,image1,&firstimage);
    if (!ok)
    {
        setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
    	LOG(("%s: unable to unconvert image\n", __FUNCTION__));
    }
    else
    {
		ok = unconvertImageData(pEnv,image2,&secondimage);
		 if (!ok)
		{
			setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
    		LOG(("%s: unable to unconvert image\n", __FUNCTION__));
		}
		else
		{
			nRc = IBSU_CombineImageEx(firstimage,secondimage,whichHand,&resultImage,&segmentImageArray[0],&segmentPositionArray[0],&segmentImageArrayCount);
	
			if (nRc == IBSU_STATUS_OK)
			{
				jobject resultImageJ;
				
				resultImageJ = convertImageData(pEnv,&resultImage);
		    	if (resultImageJ == NULL)
		    	{
		        	LOG(("%s: unable to convert ImageData object\n", __FUNCTION__));
					setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
		    	}
		    	else
		    	{
		        	jobjectArray segmentImageArrayJ;
		
		            segmentImageArrayJ = (*pEnv)->NewObjectArray(pEnv, segmentImageArrayCount, Class_IBScanDevice_ImageData, NULL);
			        if (segmentImageArrayJ == NULL)
		    	    {
		        	    LOG(("%s: unable to allocate an array of objects\n", __FUNCTION__));
						setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
					}
		            else
			        {
		    	        int  i;
		        	    BOOL ok = TRUE;
		
		            	for (i = 0; i < segmentImageArrayCount; i++)
		            	{
		                	jobject imageTempJ;
		
		                    imageTempJ = convertImageData(pEnv, &segmentImageArray[i]);
			                if (imageTempJ == NULL)
		    	            {
		        	            LOG(("%s: unable to convert ImageData object\n", __FUNCTION__));
								setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
		                	    ok = FALSE;
		                    	break;
		                	}
		                	(*pEnv)->SetObjectArrayElement(pEnv, segmentImageArrayJ, i, imageTempJ);
		                }
			            if (ok)
		    	        {
							jobject detectedFingerCountJ;
							
							detectedFingerCountJ = (*pEnv)->NewObject(pEnv, Class_Integer, MethodID_Integer_Integer, (jint)segmentImageArrayCount);
							if (detectedFingerCountJ == NULL)
							{
								LOG(("%s: unable to create Integer object\n", __FUNCTION__));
								setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
							} 
							else
							{
								jobject segmentPositionArrayJ;
							
								segmentPositionArrayJ = (*pEnv)->NewObjectArray(pEnv, segmentImageArrayCount, Class_IBScanDevice_SegmentPosition, NULL);
								if (segmentPositionArrayJ == NULL)
								{
									LOG(("%s: unable to allocate object array of length %d\n", __FUNCTION__, segmentImageArrayCount));
									setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);										
								}
								else
								{										
		    						ok = TRUE;
		
									for (i = 0; i < segmentImageArrayCount; i++)
									{
		    							jobject segmentPositionJ;
		
		        						segmentPositionJ = (*pEnv)->NewObject(pEnv, Class_IBScanDevice_SegmentPosition, MethodID_IBScanDevice_SegmentPosition_SegmentPosition, 
		        							(jint)segmentPositionArray[i].x1, (jint)segmentPositionArray[i].y1, (jint)segmentPositionArray[i].x2, (jint)segmentPositionArray[i].y2,
		        							(jint)segmentPositionArray[i].x3, (jint)segmentPositionArray[i].y3, (jint)segmentPositionArray[i].x4, (jint)segmentPositionArray[i].y4);
		        						if (segmentPositionJ == NULL)
		        						{
		            						LOG(("%s: unable to convert SegmentPosition object\n", __FUNCTION__));
											setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
		    	    						ok = FALSE;
		        							break;
		    							}
		    							(*pEnv)->SetObjectArrayElement(pEnv, segmentPositionArrayJ, i, segmentPositionJ);
		    						}
		    						if (ok)
		    						{
										returns = (*pEnv)->NewObjectArray(pEnv, 4, Class_Object, NULL);
										if (returns == NULL)
										{
											LOG(("%s: unable to allocate object array of length 4\n", __FUNCTION__));
											setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
										}
										else
										{
											(*pEnv)->SetObjectArrayElement(pEnv, returns, 0, resultImageJ);
											(*pEnv)->SetObjectArrayElement(pEnv, returns, 1, segmentImageArrayJ);
											(*pEnv)->SetObjectArrayElement(pEnv, returns, 2, segmentPositionArrayJ);
											(*pEnv)->SetObjectArrayElement(pEnv, returns, 3, detectedFingerCountJ);
										}
									}
								}
							}
		            	}
		            }
		        }
			}
			/* Free the buffer. */
			free(secondimage.Buffer);
		 }
		 /* Free the buffer. */
	     free(firstimage.Buffer);	 
	}


	return returns;


}


/*
 * private native int getOperableBeeperNative(NativeError error);
 */
JNIEXPORT jint JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(getOperableBeeperNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jobject  error)
{
    long                 devHandle;
    int                  nRc;
    IBSU_BeeperType      beeperType;
    int                  beeperTypeCode;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_GetOperableBeeper(devHandle, &beeperType);
    setNativeError(pEnv, error, nRc);

    beeperTypeCode = (int)beeperType;
	return (beeperTypeCode);
}

/*
 * private native void setBeeper(IBSU_BeepPattern beepPattern, long soundTone, long duration, long reserved_1, long reserved_2, NativeError error);
 */
JNIEXPORT void JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(setBeeperNative)
	(JNIEnv  *pEnv,
	 jobject  tthis,
	 jint     beepPattern,
	 jint     soundTone,
	 jint     duration,
	 jint     reserved_1,
	 jint     reserved_2,
	 jobject  error)
{
    long devHandle;
    int  nRc;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_SetBeeper(devHandle, (IBSU_BeepPattern)beepPattern, soundTone, duration, reserved_1, reserved_2);
    setNativeError(pEnv, error, nRc);
}

/*
 * private native int generateDisplayImageNative(..., NativeError error);
 */
JNIEXPORT jint JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(generateDisplayImageNative)(
	JNIEnv *pEnv,
	jobject tthis,
	jbyteArray inImage,
	jint inWidth,
	jint inHeight,
	jbyteArray outImage,
	jint outWidth,
	jint outHeight,
	jbyte bkColor,
	jint outFormat,
	jint outQualityLevel,
	jboolean outVerticalFlip,
	jobject  error)
{
	int nRc;
	//jobject imageJ = NULL;
	unsigned char *inImgData;
	unsigned char *outImgData;
	int imgLen = outWidth*outHeight;


	inImgData = (unsigned char *)(*pEnv)->GetByteArrayElements(pEnv, inImage, NULL);
	outImgData = (unsigned char *)(*pEnv)->GetByteArrayElements(pEnv, outImage, NULL);
	

	nRc = IBSU_GenerateDisplayImage(inImgData,inWidth,inHeight,
		outImgData,outWidth,outHeight,bkColor, outFormat, outQualityLevel, outVerticalFlip);
	setNativeError(pEnv, error, nRc);

	(*pEnv)->ReleaseByteArrayElements(pEnv, outImage, (jbyte *)outImgData, 0);
	(*pEnv)->ReleaseByteArrayElements(pEnv, inImage, (jbyte *)inImgData, 0);

	return nRc;
}



/***************************************************************************************************
 * LOCAL FUNCTIONS (CALLBACKS)
 **************************************************************************************************/

/*
 * Communication break notification.
 */
static void CALLBACK callbackDeviceCommunicationBroken(
    const int  deviceHandle,
    void      *pContext)
{
    JNIEnv *pEnv = NULL;
    jint    rc;
	BOOL    attached = FALSE;

	USE_PARAM(deviceHandle);

    LOG(("%s\n", __FUNCTION__));

    // Attach thread to VM, if necessary.
	rc = attachToVM((void **)&pEnv, &attached);
    if (rc == 0)
    {
        jobject deviceJ;

        deviceJ = (jobject)pContext;
        (*pEnv)->CallVoidMethod(pEnv, deviceJ, MethodID_IBScanDevice_deviceCommunicationBroken);
		detachFromVM(attached);
    }
}

/*
 * Preview image available notification.
 */
static void CALLBACK callbackDeviceImagePreviewAvailable(
    const int             deviceHandle,
    void                 *pContext,
    const IBSU_ImageData  image)
{
    JNIEnv *pEnv = NULL;
    jint    rc;
	BOOL    attached = FALSE;

	USE_PARAM(deviceHandle);

    LOG(("%s\n", __FUNCTION__));

    // Attach thread to VM, if necessary.
	rc = attachToVM((void **)&pEnv, &attached);
    if (rc == 0)
    {
        jobject deviceJ;
        jobject imageJ;

        deviceJ = (jobject)pContext;
        imageJ  = convertImageData(pEnv, &image);
        if (imageJ == NULL)
        {
            LOG(("%s: unable to convert ImageData object\n", __FUNCTION__));
        }
        else
        {
            (*pEnv)->CallVoidMethod(pEnv, deviceJ, MethodID_IBScanDevice_deviceImagePreviewAvailable, imageJ);
        }
		detachFromVM(attached);
    }
}

/*
 * Finger count change notification.
 */
static void CALLBACK callbackDeviceFingerCountChanged(
    const int                    deviceHandle,
    void                        *pContext,
    const IBSU_FingerCountState  fingerCountState)
{
    JNIEnv *pEnv = NULL;
    jint    rc;
	BOOL    attached = FALSE;

	USE_PARAM(deviceHandle);

    LOG(("%s\n", __FUNCTION__));

    /* Attach thread to VM, if necessary. */
	rc = attachToVM((void **)&pEnv, &attached);
	if (rc == 0)
    {
        jobject deviceJ;

        deviceJ = (jobject)pContext;
        (*pEnv)->CallVoidMethod(pEnv, deviceJ, MethodID_IBScanDevice_deviceFingerCountChanged, (jint)fingerCountState);
		detachFromVM(attached);
    }
}

/*
 * Finger quality change notification.
 */
static void CALLBACK callbackDeviceFingerQualityChanged(
    const int                      deviceHandle,
    void                          *pContext,
    const IBSU_FingerQualityState *pQualityArray,
    const int                      qualityArrayCount)
{
    JNIEnv *pEnv = NULL;
    jint    rc;
	BOOL    attached = FALSE;

	USE_PARAM(deviceHandle);

    LOG(("%s\n", __FUNCTION__));

    /* Attach thread to VM, if necessary. */
	rc = attachToVM((void **)&pEnv, &attached);
    if (rc == 0)
    {
        jintArray qualityArrayJ;

        qualityArrayJ = (*pEnv)->NewIntArray(pEnv, qualityArrayCount);
        if (qualityArrayJ == NULL)
        {
            LOG(("%s: unable to allocate data buffer of length %i\n", __FUNCTION__, qualityArrayCount));
        }
        else
        {
            jobject deviceJ;

            deviceJ = (jobject)pContext;
            (*pEnv)->SetIntArrayRegion(pEnv, qualityArrayJ, 0, qualityArrayCount, (jint *)pQualityArray);
            (*pEnv)->CallVoidMethod(pEnv, deviceJ, MethodID_IBScanDevice_deviceFingerQualityChanged, qualityArrayJ);
        }
		detachFromVM(attached);
    }
}

/*
 * Taking acquisition for result image notification.
 */
static void CALLBACK callbackDeviceAcquisitionBegun(
    const int             deviceHandle,
    void                 *pContext,
    const IBSU_ImageType  imageType)
{
    JNIEnv *pEnv = NULL;
    jint    rc;
	BOOL    attached = FALSE;

	USE_PARAM(deviceHandle);

    LOG(("%s\n", __FUNCTION__));

    /* Attach thread to VM, if necessary. */
    rc = attachToVM((void **)&pEnv, &attached);
	if (rc == 0)
    {
        jobject deviceJ;

        deviceJ = (jobject)pContext;
        (*pEnv)->CallVoidMethod(pEnv, deviceJ, MethodID_IBScanDevice_deviceAcquisitionBegun, (jint)imageType);
		detachFromVM(attached);
    }
}

/*
 * Complete acquisition for result image notification.
 */
static void CALLBACK callbackDeviceAcquisitionCompleted(
    const int             deviceHandle,
    void                 *pContext,
    const IBSU_ImageType  imageType)
{
    JNIEnv *pEnv = NULL;
    jint    rc;
	BOOL    attached = FALSE;

	USE_PARAM(deviceHandle);

    LOG(("%s\n", __FUNCTION__));

    /* Attach thread to VM, if necessary. */
    rc = attachToVM((void **)&pEnv, &attached);
    if (rc == 0)
    {
        jobject deviceJ;

        deviceJ = (jobject)pContext;
        (*pEnv)->CallVoidMethod(pEnv, deviceJ, MethodID_IBScanDevice_deviceAcquisitionCompleted, (jint)imageType);
		detachFromVM(attached);
    }
}

/*
 * Result image notification
 */
static void CALLBACK callbackDeviceImageResultAvailable(
    const int             deviceHandle,
    void                 *pContext,
    const IBSU_ImageData  image,
    const IBSU_ImageType  imageType,
    const IBSU_ImageData *pSplitImageArray,
    const int             splitImageArrayCount)
{
    JNIEnv *pEnv = NULL;
    jint    rc;
	BOOL    attached = FALSE;

	USE_PARAM(deviceHandle);

    LOG(("%s\n", __FUNCTION__));

    /* Attach thread to VM, if necessary. */
    rc = attachToVM((void **)&pEnv, &attached);
    if (rc == 0)
    {
        jobject imageJ;

        imageJ = convertImageData(pEnv, &image);
        if (imageJ == NULL)
        {
            LOG(("%s: unable to convert ImageData object\n", __FUNCTION__));
        }
        else
        {
            jobjectArray splitImageArrayJ;

            splitImageArrayJ = (*pEnv)->NewObjectArray(pEnv, splitImageArrayCount, Class_IBScanDevice_ImageData, NULL);
            if (splitImageArrayJ == NULL)
            {
                LOG(("%s: unable to allocate an array of objects\n", __FUNCTION__));
            }
            else
            {
                int  i;
                BOOL ok = TRUE;

                for (i = 0; i < splitImageArrayCount; i++)
                {
                    jobject imageTempJ;

                    imageTempJ = convertImageData(pEnv, &pSplitImageArray[i]);
                    if (imageTempJ == NULL)
                    {
                        LOG(("%s: unable to convert ImageData object\n", __FUNCTION__));
                        ok = FALSE;
                        break;
                    }
                    (*pEnv)->SetObjectArrayElement(pEnv, splitImageArrayJ, i, imageTempJ);
                }
                if (ok)
                {
                    jobject deviceJ;

                    deviceJ = (jobject)pContext;

                    (*pEnv)->CallVoidMethod(pEnv, deviceJ, MethodID_IBScanDevice_deviceImageResultAvailable, imageJ, (jint)imageType, splitImageArrayJ);
                }
            }
        }
		detachFromVM(attached);
    }
}

/*
 * The finger needs to be removed from sensor platen to start new capture.
 */
static void CALLBACK callbackDevicePlatenStateChanged(
    const int               deviceHandle,
    void                   *pContext,
    const IBSU_PlatenState  platenState)
{
    JNIEnv *pEnv = NULL;
    jint    rc;
	BOOL    attached = FALSE;

	USE_PARAM(deviceHandle);

    LOG(("%s\n", __FUNCTION__));

    /* Attach thread to VM, if necessary. */
    rc = attachToVM((void **)&pEnv, &attached);
    if (rc == 0)
    {
        jobject deviceJ;

        deviceJ = (jobject)pContext;
        (*pEnv)->CallVoidMethod(pEnv, deviceJ, MethodID_IBScanDevice_devicePlatenStateChanged, (jint)platenState);
		detachFromVM(attached);
    }
}

/*
 * Warning message notification.
 */
static void CALLBACK callbackDeviceWarningReceived(
    const int  deviceHandle,
    void      *pContext,
    const int  notifyMessage)
{
    JNIEnv *pEnv = NULL;
    jint    rc;
	BOOL    attached = FALSE;

	USE_PARAM(deviceHandle);

    LOG(("%s\n", __FUNCTION__));

    /* Attach thread to VM, if necessary. */
    rc = attachToVM((void **)&pEnv, &attached);
    if (rc == 0)
    {
        jobject deviceJ;

        deviceJ = (jobject)pContext;
        (*pEnv)->CallVoidMethod(pEnv, deviceJ, MethodID_IBScanDevice_deviceWarningReceived, (jint)notifyMessage);
		detachFromVM(attached);
    }
}

/*
 * Result image notification
 */
static void CALLBACK callbackDeviceImageResultExtendedAvailable(
    const int                   deviceHandle,
    void                       *pContext,
    const int                   imageStatus,
    const IBSU_ImageData        image,
    const IBSU_ImageType        imageType,
    const int                   detectedFingerCount,
    const int                   segmentImageArrayCount,
    const IBSU_ImageData       *pSegmentImageArray,
    const IBSU_SegmentPosition *pSegmentPositionArray)
{
    JNIEnv *pEnv = NULL;
    jint    rc;
	BOOL    attached = FALSE;

	USE_PARAM(deviceHandle);

    LOG(("%s\n", __FUNCTION__));

    /* Attach thread to VM, if necessary. */
    rc = attachToVM((void **)&pEnv, &attached);
    if (rc == 0)
    {
        jobject imageJ;

        imageJ = convertImageData(pEnv, &image);
        if (imageJ == NULL)
        {
            LOG(("%s: unable to convert ImageData object\n", __FUNCTION__));
        }
        else
        {
            jobjectArray segmentImageArrayJ;

            segmentImageArrayJ = (*pEnv)->NewObjectArray(pEnv, segmentImageArrayCount, Class_IBScanDevice_ImageData, NULL);
            if (segmentImageArrayJ == NULL)
            {
                LOG(("%s: unable to allocate an array of objects\n", __FUNCTION__));
            }
            else
            {
                int  i;
                BOOL ok = TRUE;

                for (i = 0; i < segmentImageArrayCount; i++)
                {
                    jobject imageTempJ;

                    imageTempJ = convertImageData(pEnv, &pSegmentImageArray[i]);
                    if (imageTempJ == NULL)
                    {
                        LOG(("%s: unable to convert ImageData object\n", __FUNCTION__));
                        ok = FALSE;
                        break;
                    }
                    (*pEnv)->SetObjectArrayElement(pEnv, segmentImageArrayJ, i, imageTempJ);
                }
                if (ok)
                {
                	jobjectArray segmentPositionArrayJ;

                	segmentPositionArrayJ = (*pEnv)->NewObjectArray(pEnv, segmentImageArrayCount, Class_IBScanDevice_SegmentPosition, NULL);
                	if (segmentPositionArrayJ == NULL)
                	{
                		LOG(("%s: unable to allocate an array of objects\n", __FUNCTION__));
                	}
                	else
                	{
                		ok = TRUE;

                		for (i = 0; i < segmentImageArrayCount; i++)
                		{
                			jobject imagePositionJ;

                			imagePositionJ = (*pEnv)->NewObject(pEnv, Class_IBScanDevice_SegmentPosition, MethodID_IBScanDevice_SegmentPosition_SegmentPosition,
        							(jint)pSegmentPositionArray[i].x1, (jint)pSegmentPositionArray[i].y1, (jint)pSegmentPositionArray[i].x2, (jint)pSegmentPositionArray[i].y2,
        							(jint)pSegmentPositionArray[i].x3, (jint)pSegmentPositionArray[i].y3, (jint)pSegmentPositionArray[i].x4, (jint)pSegmentPositionArray[i].y4);
                            if (imagePositionJ == NULL)
                            {
                                LOG(("%s: unable to convert SegmentPosition object\n", __FUNCTION__));
                                ok = FALSE;
                                break;
                            }
                            (*pEnv)->SetObjectArrayElement(pEnv, segmentPositionArrayJ, i, imagePositionJ);
                		}

                        if (ok)
                        {
                            jobject deviceJ;

                            deviceJ = (jobject)pContext;
                            (*pEnv)->CallVoidMethod(pEnv, deviceJ, MethodID_IBScanDevice_deviceImageResultExtendedAvailable,
                    		        (jint)imageStatus, imageJ, (jint)imageType, (jint)detectedFingerCount,
                    		        segmentImageArrayJ, segmentPositionArrayJ);
                        }
                	}
                }
            }
        }
		detachFromVM(attached);
    }
}
/*
 * Key button notification.
 */
static void CALLBACK callbackDevicePressedKeyButtons(
    const int  deviceHandle,
    void      *pContext,
    const int  pressedKeyButtons)
{
    JNIEnv *pEnv = NULL;
    jint    rc;
	BOOL    attached = FALSE;

	USE_PARAM(deviceHandle);

    LOG(("%s\n", __FUNCTION__));

    /* Attach thread to VM, if necessary. */
    rc = attachToVM((void **)&pEnv, &attached);
    if (rc == 0)
    {
        jobject deviceJ;

        deviceJ = (jobject)pContext;
        (*pEnv)->CallVoidMethod(pEnv, deviceJ, MethodID_IBScanDevice_devicePressedKeyButtons, (jint)pressedKeyButtons);
		detachFromVM(attached);
    }
}
/***************************************************************************************************
 * LOCAL FUNCTIONS (HELPERS)
 **************************************************************************************************/

/*
 * Find a class and log result.
 */
static jclass findClass(
    JNIEnv     *pEnv,
    const char *name,
    BOOL       *pOk)
{
    jclass cclass = NULL;

    if (*pOk == TRUE)
    {
		cclass = (*pEnv)->FindClass(pEnv, name);
		if (cclass == NULL)
		{
			LOG(("unable to get reference to \"%s\" class\n", name));
			*pOk = FALSE;
		}
		else
		{
			LOG(("got reference to \"%s\" class\n", name));
			cclass = (*pEnv)->NewGlobalRef(pEnv, cclass);
			if (cclass == NULL)
			{
				LOG(("unable to get global reference to \"%s\" class\n", name));
				*pOk = FALSE;
			}
		}
    }
    return (cclass);
}

/*
 * Get a field ID and log result.
 */
static jfieldID getFieldID(
    JNIEnv     *pEnv,
    jclass      cclass,
    const char *name,
    const char *sig,
    BOOL       *pOk)
{
    jfieldID fieldID = NULL;

    if (*pOk == TRUE)
    {
		fieldID = (*pEnv)->GetFieldID(pEnv, cclass, name, sig);
		if (fieldID == NULL)
		{
			LOG(("unable to get reference to \"%s\" field with signature \"%s\"\n", name, sig));
			*pOk = FALSE;
		}
		else
		{
			LOG(("got reference to \"%s\" field with signature \"%s\"\n", name, sig));
		}
    }
    return (fieldID);
}

/*
 * Get a method ID and log result.
 */
static jmethodID getMethodID(
    JNIEnv     *pEnv,
    jclass      cclass,
    const char *name,
    const char *sig,
    BOOL       *pOk)
{
    jmethodID methodID = NULL;

    if (*pOk == TRUE)
    {
		methodID = (*pEnv)->GetMethodID(pEnv, cclass, name, sig);
		if (methodID == NULL)
		{
			LOG(("unable to get reference to \"%s\" method with signature \"%s\"\n", name, sig));
			*pOk = FALSE;
		}
		else
		{
			LOG(("got reference to \"%s\" method with signature \"%s\"\n", name, sig));
		}
    }
    return (methodID);
}

/*
 * Set native error.
 */
static void setNativeError(
    JNIEnv  *pEnv,
    jobject  obj,
    int      code)
{
    (*pEnv)->SetIntField(pEnv, obj, FieldID_IBScanDevice_NativeError_code, (jint)code);
}

/*
 * Set native error.
 */
static long getHandle(
    JNIEnv  *pEnv,
    jobject  obj)
{
    jlong handle = (*pEnv)->GetLongField(pEnv, obj, FieldID_IBScanDevice_handleNative);

    return ((long)handle);
}

/*
 * Convert C ImageData to Java ImageData
 */
static jobject convertImageData(
    JNIEnv               *pEnv,
    const IBSU_ImageData *pImage)
{
    long       bufferLength;
    jbyteArray bufferJ;
    jobject    imageJ = NULL;

	if (pImage->Pitch < 0)
		bufferLength = pImage->Height * (-pImage->Pitch);
	else
		bufferLength = pImage->Height * pImage->Pitch;

    bufferJ = (*pEnv)->NewByteArray(pEnv, bufferLength);
    if (bufferJ == NULL)
    {
        LOG(("%s: unable to allocate data buffer of size %ld\n", __FUNCTION__, bufferLength));
    }
    else
    {
        (*pEnv)->SetByteArrayRegion(pEnv, bufferJ, 0, bufferLength, pImage->Buffer);
        imageJ  = (*pEnv)->NewObject(pEnv, Class_IBScanDevice_ImageData,
                        MethodID_IBScanDevice_ImageData_ImageData, bufferJ, (jint)pImage->Width,
                        (jint)pImage->Height, (jdouble)pImage->ResolutionX, (jdouble)pImage->ResolutionY,
						(jdouble)pImage->FrameTime, (jint)pImage->Pitch, (jshort)pImage->BitsPerPixel,
						(jint)pImage->Format, (jboolean)pImage->IsFinal, (jint)pImage->ProcessThres);
        if (imageJ == NULL)
        {
            LOG(("%s: unable to allocate ImageData object\n", __FUNCTION__));
        }
    }

    return (imageJ);
}

/*
 * Convert Java ImageData to C ImageData.
 */
static BOOL unconvertImageData(
	JNIEnv         *pEnv,
	jobject         imageData,
	IBSU_ImageData *pImage)
{
	BOOL       ok = FALSE;
	jbyteArray bufferJ;

	memset(pImage, 0, sizeof(IBSU_ImageData));

	/* Get buffer field. */
	bufferJ = (jbyteArray)(*pEnv)->GetObjectField(pEnv, imageData, FieldID_IBScanDevice_ImageData_buffer);
	if (bufferJ == NULL)
	{
        LOG(("%s: unable to get buffer field\n", __FUNCTION__));
	}
	else
	{
		void *buffer;

		buffer = (*pEnv)->GetByteArrayElements(pEnv, bufferJ, NULL);
		if (buffer == NULL)
		{
	        LOG(("%s: unable to get buffer field elements\n", __FUNCTION__));
		}
		else
		{
			jobject format;
			jsize   length;

			/* Allocate array for buffer.  This must be freed later. */
			length         = (*pEnv)->GetArrayLength(pEnv, bufferJ);
			pImage->Buffer = malloc(length);
			if (pImage->Buffer == NULL)
			{
				LOG(("%s: unable to allocate memory for buffer\n", __FUNCTION__));
			}
			else
			{
				memcpy(pImage->Buffer, buffer, length);

				/* Get width field. */
				pImage->Width        = (*pEnv)->GetIntField(pEnv, imageData, FieldID_IBScanDevice_ImageData_width);
				/* Get height field. */
				pImage->Height       = (*pEnv)->GetIntField(pEnv, imageData, FieldID_IBScanDevice_ImageData_height);
				/* Get resolutionX field. */
				pImage->ResolutionX  = (*pEnv)->GetDoubleField(pEnv, imageData, FieldID_IBScanDevice_ImageData_resolutionX);
				/* Get resolutionY field. */
				pImage->ResolutionY  = (*pEnv)->GetDoubleField(pEnv, imageData, FieldID_IBScanDevice_ImageData_resolutionY);
				/* Get frameTime field. */
				pImage->FrameTime    = (*pEnv)->GetDoubleField(pEnv, imageData, FieldID_IBScanDevice_ImageData_frameTime);
				/* Get pitch field. */
				pImage->Pitch        = (*pEnv)->GetIntField(pEnv, imageData, FieldID_IBScanDevice_ImageData_pitch);
				/* Get bitsPerPixel field. */
				pImage->BitsPerPixel = (BYTE)(*pEnv)->GetShortField(pEnv, imageData, FieldID_IBScanDevice_ImageData_bitsPerPixel);
				/* Get format field. */
				format = (*pEnv)->GetObjectField(pEnv, imageData, FieldID_IBScanDevice_ImageData_format);
				if (format == NULL)
				{
					LOG(("%s: unable to get format field\n", __FUNCTION__));
					free(pImage->Buffer);
				}
				else
				{
					pImage->Format  = (IBSU_ImageFormat)(*pEnv)->CallIntMethod(pEnv, format, MethodID_IBScanDevice_ImageFormat_toCode);
					/* Get isFinal field. */
					pImage->IsFinal = (*pEnv)->GetBooleanField(pEnv, imageData, FieldID_IBScanDevice_ImageData_isFinal);
					/* Get processThres field. */
					pImage->ProcessThres = (*pEnv)->GetIntField(pEnv, imageData, FieldID_IBScanDevice_ImageData_processThres);

					ok = TRUE;
				}
			}
		}
		(*pEnv)->ReleaseByteArrayElements(pEnv, bufferJ, (jbyte *)buffer, 0);
	}

	return (ok);
}

/*
 * Convert C ImageDataExt to Java ImageDataExt.
 */
static jobject convertImageDataExt(
    JNIEnv               *pEnv,
    const IBSM_ImageData *pImage)
{
    jbyteArray bufferJ;
    jobject    imageJ = NULL;

    bufferJ = (*pEnv)->NewByteArray(pEnv, pImage->ImageDataLength);
    if (bufferJ == NULL)
    {
    	LOG(("unable to allocate data buffer of size %d\n", pImage->ImageDataLength));
    }
    else
    {
        (*pEnv)->SetByteArrayRegion(pEnv, bufferJ, 0, pImage->ImageDataLength, (const jbyte *)pImage->ImageData);
        imageJ  = (*pEnv)->NewObject(pEnv, Class_IBCommon_ImageDataExt,
                        MethodID_IBCommon_ImageDataExt_ImageDataExt, (jint)pImage->ImageFormat,
                        (jint)pImage->ImpressionType, (jint)pImage->FingerPosition, (jint)pImage->CaptureDeviceTechID,
                        (jshort)pImage->CaptureDeviceVendorID, (jshort)pImage->CaptureDeviceTypeID,
                        (jshort)pImage->ScanSamplingX, (jshort)pImage->ScanSamplingY, (jshort)pImage->ImageSamplingX,
                        (jshort)pImage->ImageSamplingY, (jshort)pImage->ImageSizeX, (jshort)pImage->ImageSizeY,
                        (jbyte)pImage->ScaleUnit, (jbyte)pImage->BitDepth, bufferJ);
        if (imageJ == NULL)
        {
        	LOG(("unable to allocate ImageData object\n"));
        }
    }

    return (imageJ);
}

/*
 * Attach thread from VM.
 */
static jint attachToVM(void **env, BOOL *pAttached)
{
	jint rc;

    /* Attach thread to VM, if necessary. */
	rc = (*SavedJavaVM)->GetEnv(SavedJavaVM, env, JNI_VERSION_1_2);
	if (rc == 0)
	{
		*pAttached = TRUE;
	}
	else
	{
	    rc = (*SavedJavaVM)->AttachCurrentThread(SavedJavaVM, env, NULL);
		*pAttached = FALSE;

		if (rc < 0)
		{
			LOG(("failed to attach thread to VM\n"));
		}
	}

	return (rc);
}

/*
 * Detach thread from VM.
 */
static void detachFromVM(BOOL attached)
{
	if (attached)
	{
		(*SavedJavaVM)->DetachCurrentThread(SavedJavaVM);
	}
}

/*
 * private native int removeFingerImageNative();
 */
JNIEXPORT jint JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(removeFingerImageNative)(
    JNIEnv  *pEnv,
	jobject  tthis,
    jlong 	 fIndex,
	jobject  error)
{
    long                 devHandle;
    jint                 nRc = IBSU_STATUS_OK;

    LOG(("%s\n", __FUNCTION__));

    devHandle = getHandle(pEnv, tthis);
    nRc       = IBSU_RemoveFingerImage(devHandle, (DWORD)fIndex);
    setNativeError(pEnv, error, nRc);

	return nRc;
}

/*
 * private native int addFingerImageNative();
 */
JNIEXPORT jint JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(addFingerImageNative)(
    JNIEnv  *pEnv,
	jobject  tthis,
	jobject  image,
    jlong 	 fIndex,
    jint 	 imageType,
    jboolean flagForce,
	jobject  error)
{
    long                 devHandle;
    jint                 nRc = IBSU_STATUS_OK;
	IBSU_ImageData 		 fingerImage;
	BOOL			 	 ok;

	memset(&fingerImage,0,sizeof(fingerImage));

    LOG(("%s\n", __FUNCTION__));

	ok = unconvertImageData(pEnv,image, &fingerImage);
	 if (!ok)
	{
		setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
		LOG(("%s: unable to unconvert image\n", __FUNCTION__));
	}
	else
	{
		devHandle = getHandle(pEnv, tthis);
		nRc       = IBSU_AddFingerImage(devHandle, fingerImage, (DWORD)fIndex, (IBSU_ImageType)imageType, flagForce);
		setNativeError(pEnv, error, nRc);
	}

	return nRc;
}

/*
 * private native int isFingerDuplicatedNative();
 */
JNIEXPORT jlong JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(isFingerDuplicatedNative)(
    JNIEnv  *pEnv,
	jobject  tthis,
	jobject  image,
    jlong 	 fIndex,
    jint 	 imageType,
    jint 	 securityLevel,
	jobject  error)
{
    long                 devHandle;
    jint                 nRc = IBSU_STATUS_OK;
	IBSU_ImageData 		 fingerImage;
	BOOL			 	 ok = FALSE;
	DWORD				 pMatchedPosition = IBSU_FINGER_NONE;

	memset(&fingerImage,0,sizeof(fingerImage));

    LOG(("%s\n", __FUNCTION__));

	ok = unconvertImageData(pEnv,image, &fingerImage);
	 if (!ok)
	{
		setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
		LOG(("%s: unable to unconvert image\n", __FUNCTION__));
	}
	else
	{
		devHandle = getHandle(pEnv, tthis);
		nRc       = IBSU_IsFingerDuplicated(devHandle, fingerImage, (DWORD)fIndex, (IBSU_ImageType)imageType, securityLevel, &pMatchedPosition);
		setNativeError(pEnv, error, nRc);
	}

	return pMatchedPosition;
}

/*
 * private native int isValidFingerGeometryNative();
 */
JNIEXPORT jboolean JNICALL IBSCANDEVICE_BUILD_JNI_FNCT(isValidFingerGeometryNative)(
    JNIEnv  *pEnv,
	jobject  tthis,
	jobject  image,
    jlong 	 fIndex,
    jint 	 imageType,
 	jobject  error)
{
    long                 devHandle;
    jint                 nRc = IBSU_STATUS_OK;
	IBSU_ImageData 		 fingerImage;
	BOOL			 	 ok;
	BOOL				 pValid = FALSE;

	memset(&fingerImage,0,sizeof(fingerImage));

    LOG(("%s\n", __FUNCTION__));

	ok = unconvertImageData(pEnv,image, &fingerImage);
	 if (!ok)
	{
		setNativeError(pEnv, error, IBSU_ERR_MEM_ALLOC);
		LOG(("%s: unable to unconvert image\n", __FUNCTION__));
	}
	else
	{
		devHandle = getHandle(pEnv, tthis);
		nRc = IBSU_IsValidFingerGeometry(devHandle, fingerImage, (DWORD)fIndex, (IBSU_ImageType)imageType, &pValid);
		setNativeError(pEnv, error, nRc);
	}

	return pValid;
}

