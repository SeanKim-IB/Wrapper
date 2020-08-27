#include <Windows.h>
#include <stdio.h>

#include "IBScanUltimateApi.h"

typedef int (WINAPI *IBSU_GetSDKVersion_Func)(IBSU_SdkVersion *pVerinfo);
typedef int (WINAPI *IBSU_GetDeviceCount_Func)(int *pDeviceCount);
typedef int (WINAPI *IBSU_GetDeviceDescription_Func)(const int deviceIndex, IBSU_DeviceDesc *pDeviceDesc);
typedef int (WINAPI *IBSU_OpenDevice_Func)(const int deviceIndex, int *pHandle);
typedef int (WINAPI *IBSU_CloseDevice_Func)(const int handle);
typedef int (WINAPI *IBSU_RegisterCallbacks_Func)(const int handle, const IBSU_Events event, void *pCallbackFunction, void *pContext);

void CALLBACK OnInitProgressChange(
       const int deviceIndex,
       void*     pContext,
	   const int progressValue );

void CALLBACK OnInitProgressChange(
       const int deviceIndex,
       void*     pContext,
	   const int progressValue ) {

   printf("\nInitializing device... %d%%\n ", progressValue);
   fflush(stdout);
}


char context[4];  // Dummy variable for callbacks

int main()
{
	int i, nRc, deviceHandle;
	IBSU_GetSDKVersion_Func lpGetSDKVersion_Func;
	IBSU_GetDeviceCount_Func lpGetDeviceCount_Func;
	IBSU_GetDeviceDescription_Func lpGetDeviceDescription_Func;
	IBSU_RegisterCallbacks_Func lpRegisterCallbacks_Func;
	IBSU_OpenDevice_Func lpOpenDevice_Func;
	IBSU_CloseDevice_Func lpCloseDevice_Func;
	IBSU_SdkVersion sdkVer;
	int deviceCount;
	IBSU_DeviceDesc deviceDesc;

	HMODULE ibsModule = LoadLibrary(TEXT("IBScanUltimate.dll"));
	if (!ibsModule)
	{
		printf("Failed to load!\n");
		return -1;
	}
	else
	{
		printf("Loaded successfully\n");
	}

	if (!FreeLibrary(ibsModule))
	{
		printf("Failed to free library\n");
		return -1;
	}

	printf("Unloaded IBScanUltiamte.dll successfully\n\n");



/////////////////////////////////////////////////////////////////
// Additional test codes
	ibsModule = LoadLibrary(TEXT("IBScanUltimate.dll"));
	if (!ibsModule)
	{
		printf("Failed to load!\n");
		return -1;
	}
	else
	{
		printf("Reloaded successfully\n");
	}

	lpGetSDKVersion_Func = (IBSU_GetSDKVersion_Func)GetProcAddress(ibsModule, "IBSU_GetSDKVersion");
	lpGetSDKVersion_Func(&sdkVer);
	printf("\tIBScanUltimate ver = %s\n",sdkVer.File);

	lpGetDeviceCount_Func = (IBSU_GetDeviceCount_Func)GetProcAddress(ibsModule, "IBSU_GetDeviceCount");
	lpGetDeviceCount_Func(&deviceCount);
	printf("\tdeviceCount = %d\n", deviceCount);

	for (i=0; i<deviceCount; i++)
	{
		lpGetDeviceDescription_Func = (IBSU_GetDeviceDescription_Func)GetProcAddress(ibsModule, "IBSU_GetDeviceDescription");
		lpGetDeviceDescription_Func(i, &deviceDesc);
		printf("\t%s_v%s (%s)\n", deviceDesc.productName, deviceDesc.fwVersion, deviceDesc.serialNumber);
	}

	lpRegisterCallbacks_Func = (IBSU_RegisterCallbacks_Func)GetProcAddress(ibsModule, "IBSU_RegisterCallbacks");
	nRc = lpRegisterCallbacks_Func ( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, (void *)OnInitProgressChange, context );
	if( nRc < IBSU_STATUS_OK ) {
		printf("Problem registering init progress change callback: %d\n", nRc);
		exit(1);
	}

	lpOpenDevice_Func = (IBSU_OpenDevice_Func)GetProcAddress(ibsModule, "IBSU_OpenDevice");
	nRc = lpOpenDevice_Func(0, &deviceHandle);
	printf("\topendevice = %d\n", nRc);
	if (nRc < IBSU_STATUS_OK)
	{
		printf("Error %d opening scanner\n", nRc);
		deviceHandle = -1;
		goto FreeLibrary;
	}

	if( deviceHandle >= 0 ) {
		printf("\tclosedevice = %d\n", nRc);
		lpCloseDevice_Func = (IBSU_CloseDevice_Func)GetProcAddress(ibsModule, "IBSU_CloseDevice");
		lpCloseDevice_Func( deviceHandle );
		deviceHandle = -1;
	}

	printf("Test succeeded\n");
	return 0;

FreeLibrary:
	if (!FreeLibrary(ibsModule))
	{
		printf("Failed to free library\n");
		return -1;
	}

	printf("Failed! Now program will be terminated\n");
	return 0;
}