#ifndef IBSU_GLOBAL_H
#define IBSU_GLOBAL_H

//#include "IBScanUltimateApi.h"
#include "IBScanNFIQ2Api.h"

#if defined(_WINDOWS) && defined(_DEBUG)
#define __G_DEBUG__
#endif

#ifdef __android__
// --------------------------------------------------------------
// Android Debug code
#include <android/log.h> 
#define LOG_DEBUG_TAG "IBSCAN" 


// End Android Debug code
// ------------------------------------------------------------#endif
#endif

#if defined(_WINDOWS)
    #define G_TRACE(...) TRACE(__VA_ARGS__)
#elif defined(__android__)
    #define G_TRACE(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_DEBUG_TAG, __VA_ARGS__)
#elif defined(__linux__)
    #define G_TRACE(...) printf(__VA_ARGS__)
#endif
#define __MAX_PIPE_DATA__					4096
#define __MAX_TRACE_MESSAGE_SIZE__			(__MAX_PIPE_DATA__-128)
#define __PIPE_WAIT_TIME__					100

#if defined(__linux__)
#define pszLinuxFileVersion					"1.9.5.7"
#define pszLinuxProdutVersion				"1.9.5.7"
#endif


typedef enum
{
	LOG_ERROR,
	LOG_WARNING,
	LOG_INFORMATION
} LogCode;

typedef struct tagTraceLog
{
	char	timestamp[24];
	char	category[64];
	LogCode	code;
	int		error;
	int		tid;
	char	message[__MAX_TRACE_MESSAGE_SIZE__];
} TraceLog;


#endif


