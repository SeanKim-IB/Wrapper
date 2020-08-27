// IBSU_ConsoleSampleForVC.cpp : Defines the entry point for the console application.
//

	#include "stdafx.h"
#ifdef WIN32
	#include <conio.h>
	#include <windows.h>
#else
	#include <termios.h>
	#include <sys/time.h>
#endif


#include "IBScanUltimateApi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <time.h>

#ifdef WIN32
// Constants to change epoch time
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

// for timezone
struct timezone
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};
#endif

using namespace std;

int deviceCount = 0;
int deviceHandle = -1;

char context[4];  // Dummy variable for callbacks
int m_SavedFingerCountState = -1;


void GetConfiguration();
void CALLBACK OnPreviewImageAvailable(const int deviceHandle,
       void*     pContext,
	   const IBSU_ImageData image );
void CALLBACK OnFingerCountChange(
       const int deviceHandle,
       void*     pContext,
	   const IBSU_FingerCountState fingerCountState );
void CALLBACK OnDeviceCountChange(
       int       detectedDevices,
       void*     pContext );
void CALLBACK OnInitProgressChange(
       const int deviceIndex,
       void*     pContext,
	   const int progressValue );
void CALLBACK OnDeviceComunicationBreak (
   const int brokenDeviceHandle,
       void*     pContext );
void CALLBACK OnTakingAcquisition (
      const int                   deviceHandle,
      void                        *pContext,
	  const IBSU_ImageType        imageType );
void CALLBACK OnCompleteAcquisition (
      const int                   deviceHandle,
      void                        *pContext,
	  const IBSU_ImageType        imageType );
void CALLBACK OnResultImageAvailable (
      const int                   deviceHandle,
      void                        *pContext,
      const IBSU_ImageData        image,
	  const IBSU_ImageType        imageType,
      const IBSU_ImageData        *pSplitImageArray,
      const int                   splitImageArrayCount);
void CALLBACK OnEvent_ClearPlatenAtCapture (
      const int                   deviceHandle,
      void                        *pContext,
      const IBSU_PlatenState      platenState);

BOOL OpenCaptureDevice();
double FramesPerSecond();
BOOL DisplayManu();
void RunCaptureTest();



void GetConfiguration() {

   int nRc = IBSU_GetDeviceCount( &deviceCount );
   if( nRc < IBSU_STATUS_OK ) {
      printf("Error %d Retrieving device count\n", nRc);
      return;
   }
   printf("Found %d devices attached\n", deviceCount);

}


#ifndef WIN32

char _getch(void) {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return (buf);
}

#else

// gettimeofday in windows
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if( NULL != tv )
	{
		// Get system time
		GetSystemTimeAsFileTime(&ft);

		// Make a unsigned 64 bit
		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		// Change to epoch time
		tmpres -= DELTA_EPOCH_IN_MICROSECS;

		// Change from 100nano to 1micro
		tmpres /= 10;

		// Change to sec and micorsec
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (tmpres % 1000000UL);
	}

	// Process to timezone
	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

	return 0;
}

#endif

struct timeval start_tv;
int previewCount = 0;

void CALLBACK OnPreviewImageAvailable(
       const int deviceHandle,
       void*     pContext,
	   const IBSU_ImageData image ) {

//   putchar('C');
	int touchValue;
	char IsFinal[32];
	if( image.IsFinal )
		strcpy(IsFinal, "TRUE");
	else
		strcpy(IsFinal, "FALSE");

	IBSU_IsTouchedFinger(deviceHandle, &touchValue);
	printf("C");
//	printf("Frame time : %d ms, IsFinal = %s, touchValue = %d\n", (int)(image.FrameTime*1000), IsFinal, touchValue);
	fflush(stdout);

	previewCount++;
}

void CALLBACK OnFingerCountChange(
       const int deviceHandle,
       void*     pContext,
	   const IBSU_FingerCountState fingerCountState ) {

	if( m_SavedFingerCountState != (int)fingerCountState )
	{
		char pcState[32];  
		if( fingerCountState == ENUM_IBSU_FINGER_COUNT_OK )
			strcpy(pcState, "FINGER_COUNT_OK");
		else if( fingerCountState == ENUM_IBSU_TOO_MANY_FINGERS )
			strcpy(pcState, "TOO_MANY_FINGERS");
		else if( fingerCountState == ENUM_IBSU_TOO_FEW_FINGERS )
			strcpy(pcState, "TOO_FEW_FINGERS");
		else if( fingerCountState == ENUM_IBSU_NON_FINGER )
			strcpy(pcState, "NON-FINGER");
		else
			strcpy(pcState, "UNKNOWN");

		printf("\n-- Finger count changed -- Device= %d, State= %s\n", deviceHandle, pcState);
		fflush(stdout);

		m_SavedFingerCountState = (int)fingerCountState;
	}
}

void CALLBACK OnDeviceCountChange(
       int       detectedDevices,
       void*     pContext ) {

   printf("\nDeviceCountChange: now %d devices\n ", detectedDevices);
   fflush(stdout);
}

void CALLBACK OnInitProgressChange(
       const int deviceIndex,
       void*     pContext,
	   const int progressValue ) {

   printf("\nInitializing device... %d%%\n ", progressValue);
   fflush(stdout);
}

void CALLBACK OnDeviceComunicationBreak (
   const int brokenDeviceHandle ,
       void*     pContext
   ) {

   int rc;

   printf("\nDevice Communications break\n");
   rc = IBSU_CloseDevice( brokenDeviceHandle );
   if( brokenDeviceHandle == deviceHandle ) {
      deviceHandle = -1;
   }
   fflush(stdout);
}

void CALLBACK OnTakingAcquisition (
      const int                   deviceHandle,
      void                        *pContext,
	  const IBSU_ImageType        imageType
)
{
	if( imageType == ENUM_IBSU_ROLL_SINGLE_FINGER )
	{
		printf("\nWhen done remove finger from sensor!\n ");
		fflush(stdout);
	}
}

void CALLBACK OnCompleteAcquisition (
      const int                   deviceHandle,
      void                        *pContext,
	  const IBSU_ImageType        imageType
)
{
	printf("\nCompleteAcquisition\n ");
   fflush(stdout);
}

void CALLBACK OnResultImageAvailable (
      const int                   deviceHandle,
      void                        *pContext,
      const IBSU_ImageData        image,
	  const IBSU_ImageType        imageType,
      const IBSU_ImageData        *pSplitImageArray,
      const int                   splitImageArrayCount
)
{
	double framesPerSecond = FramesPerSecond();
	printf("\nStopped. %1.1f frames per second\n",  framesPerSecond);


	char imgTypeName[IBSU_MAX_STR_LEN]={0};
	switch( imageType )
	{
	case ENUM_IBSU_ROLL_SINGLE_FINGER:
		strcpy(imgTypeName, "Rolling single finger"); break;
	case ENUM_IBSU_FLAT_SINGLE_FINGER:
		strcpy(imgTypeName, "Flat single finger"); break;
	case ENUM_IBSU_FLAT_TWO_FINGERS:
		strcpy(imgTypeName, "Flat two fingers"); break;
	case ENUM_IBSU_FLAT_FOUR_FINGERS:
		strcpy(imgTypeName, "Flat 4 fingers"); break;
	default:
		strcpy(imgTypeName, "Unknown"); break;
	}

	printf("\n%s Image acquisition complete\n", imgTypeName);

	printf("Saving image...\n");
	char saveFileName[128];
	sprintf(saveFileName, "ResultImage_%s.bmp", imgTypeName);
	if( IBSU_SaveBitmapImage(saveFileName, (BYTE*)image.Buffer,
						 image.Width, image.Height, image.Pitch,
						 image.ResolutionX, image.ResolutionY) != IBSU_STATUS_OK )
	{
		printf("Failed to save bitmap image!");
		return;
	}

	if( splitImageArrayCount > 1 )
	{
		for( int i=0; i<splitImageArrayCount; i++ )
		{
			sprintf(saveFileName, "ResultImage__Split_%02d.bmp", i);
			if( IBSU_SaveBitmapImage(saveFileName, (BYTE*)(pSplitImageArray+i)->Buffer,
								 image.Width, image.Height, image.Pitch,
								 image.ResolutionX, image.ResolutionY) != IBSU_STATUS_OK )
			{
				printf("Failed to save splite image!");
				return;
			}
		}
	}

	printf("\n\nPress enter!\n");
	fflush(stdout);
}

BOOL OpenCaptureDevice() {
	int nRc = IBSU_OpenDevice(0, &deviceHandle);
	if( nRc < IBSU_STATUS_OK ) {
		printf("Error %d opening scanner\n", nRc);
		deviceHandle = -1;
		return FALSE;
	}

	// Get Image size to be supported in device
//	char cValue[IBSU_MAX_STR_LEN + 1];
//	nRc = IBSU_GetProperty(deviceHandle, ENUM_IBSU_PROPERTY_IMAGE_WIDTH, &cValue[0] );
//	int resultWidth = atoi(cValue);
//	nRc = IBSU_GetProperty(deviceHandle, ENUM_IBSU_PROPERTY_IMAGE_HEIGHT, &cValue[0] );
//	int resultHeight = atoi(cValue);

//	if( nRc < IBSU_STATUS_OK ) {
//		printf("Problem to get property of result image: %d\n", nRc);
//		return FALSE;
//	}

//	int imageSize = resultWidth * resultHeight;
//	int pos = 10;		// Default contrast
//	nRc = IBSU_SetContrast(deviceHandle, pos);  // Contrast value (range: 0 <= value <= @ref IBSU_MAX_CONTRAST_VALUE)
//	if( nRc < IBSU_STATUS_OK ) {
//		printf("Problem setting contrast: %d\n", nRc);
//		return FALSE;
//	}

	nRc = IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, (void *)OnPreviewImageAvailable, context );
	if( nRc < IBSU_STATUS_OK ) {
		printf("Problem registering preview image callback: %d\n", nRc);
		return FALSE;
	}


	nRc = IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, (void *)OnFingerCountChange, context );
	if( nRc < IBSU_STATUS_OK ) {
		printf("Problem registering object count callback: %d\n", nRc);
		return FALSE;
	}

	nRc =  IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, (void *)OnDeviceComunicationBreak, context );
	if( nRc < IBSU_STATUS_OK ) {
		printf("Problem registering communication break callback: %d\n", nRc);
		return FALSE;
	}

	nRc =  IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, (void *)OnTakingAcquisition, context );
	if( nRc < IBSU_STATUS_OK ) {
		printf("Problem registering taking acquisition callback: %d\n", nRc);
		return FALSE;
	}

	nRc =  IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, (void *)OnCompleteAcquisition, context );
	if( nRc < IBSU_STATUS_OK ) {
		printf("Problem registering complete acquisition callback: %d\n", nRc);
		return FALSE;
	}

	nRc =  IBSU_RegisterCallbacks( deviceHandle, ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE, (void *)OnResultImageAvailable, context );
	if( nRc < IBSU_STATUS_OK ) {
		printf("Problem registering result image callback: %d\n", nRc);
		return FALSE;
	}

  return TRUE;

}

double FramesPerSecond() {
   struct timeval tv;
   double elapsed = 0.0;

   gettimeofday(&tv, NULL);
   elapsed = (tv.tv_sec - start_tv.tv_sec) +
     (tv.tv_usec - start_tv.tv_usec) / 1000000.0;
   return ((double)previewCount / elapsed);
}

BOOL DisplayManu()
{
	int nRc;
	DWORD captureOptions = 0;
	BOOL isAvailable;
     char ch;
//	 system("clear");
     printf("\nReady.   Enter choice:\n" \
		 "\t1. Start capture for flat single finger.\n" \
		 "\t2. Start capture for flat two fingers.\n" \
		 "\t3. Start capture for rolling single finger.\n" \
		 "\t4. Abort Capture\n" \
		 "\t5. End program\n:");
     ch = _getch();
     switch( ch ) {
     case '1' :
        printf("==>%c\n", ch);

//        if( deviceHandle == -1 ) {
           if( !OpenCaptureDevice() ) {
			   printf("Problem OpenCaptureDevice\n");
              return FALSE;
           }
//        }

		isAvailable;
		IBSU_IsCaptureAvailable( deviceHandle, ENUM_IBSU_FLAT_SINGLE_FINGER, ENUM_IBSU_IMAGE_RESOLUTION_500, &isAvailable );
		if( ! isAvailable ) {
			printf("Capture mode FLAT_SINGLE_FINGER not available\n");
			return FALSE;
		}

		// determine capture options
		captureOptions = 0;

		captureOptions |= IBSU_OPTION_AUTO_CONTRAST;
		captureOptions |= IBSU_OPTION_AUTO_CAPTURE;
		//  captureOptions |= IBSU_OPTION_IGNORE_FINGER_COUNT;

		nRc = IBSU_BeginCaptureImage( deviceHandle, ENUM_IBSU_FLAT_SINGLE_FINGER, ENUM_IBSU_IMAGE_RESOLUTION_500, captureOptions );
        if( nRc >= IBSU_STATUS_OK ) {
              printf("Setting up for scan with callback...Displayed 'C'=Image callback.\n");
              previewCount = 0;
              gettimeofday(&start_tv, NULL);
        }
        else {
           printf("Problem starting capture: %d\n", nRc);
           return FALSE;
        }
        break;

     case '2' :
        printf("==>%c\n", ch);

//        if( deviceHandle == -1 ) {
           if( !OpenCaptureDevice() ) {
			   printf("Problem OpenCaptureDevice\n");
              return FALSE;
           }
//        }

		isAvailable;
		IBSU_IsCaptureAvailable( deviceHandle, ENUM_IBSU_FLAT_TWO_FINGERS, ENUM_IBSU_IMAGE_RESOLUTION_500, &isAvailable );
		if( ! isAvailable ) {
			printf("Capture mode FLAT_TWO_FINGERS not available\n");
			return FALSE;
		}

		// determine capture options
		captureOptions = 0;

		captureOptions |= IBSU_OPTION_AUTO_CONTRAST;
		captureOptions |= IBSU_OPTION_AUTO_CAPTURE;
		//  captureOptions |= IBSU_OPTION_IGNORE_FINGER_COUNT;

		nRc = IBSU_BeginCaptureImage( deviceHandle, ENUM_IBSU_FLAT_TWO_FINGERS, ENUM_IBSU_IMAGE_RESOLUTION_500, captureOptions );
        if( nRc >= IBSU_STATUS_OK ) {
              printf("Setting up for scan with callback...Displayed 'C'=Image callback.\n");
              previewCount = 0;
              gettimeofday(&start_tv, NULL);
        }
        else {
           printf("Problem starting capture: %d\n", nRc);
           return FALSE;
        }
        break;

     case '3' :
        printf("==>%c\n", ch);

//        if( deviceHandle == -1 ) {
           if( !OpenCaptureDevice() ) {
			   printf("Problem OpenCaptureDevice\n");
              return FALSE;
           }
//        }

		isAvailable;
		IBSU_IsCaptureAvailable( deviceHandle, ENUM_IBSU_ROLL_SINGLE_FINGER, ENUM_IBSU_IMAGE_RESOLUTION_500, &isAvailable );
		if( ! isAvailable ) {
			printf("Capture mode ROLL_SINGLE_FINGER not available\n");
			return FALSE;
		}

		// determine capture options
		captureOptions = 0;

		captureOptions |= IBSU_OPTION_AUTO_CONTRAST;
		captureOptions |= IBSU_OPTION_AUTO_CAPTURE;
		//  captureOptions |= IBSU_OPTION_IGNORE_FINGER_COUNT;

		nRc = IBSU_BeginCaptureImage( deviceHandle, ENUM_IBSU_ROLL_SINGLE_FINGER, ENUM_IBSU_IMAGE_RESOLUTION_500, captureOptions );
        if( nRc >= IBSU_STATUS_OK ) {
              printf("Setting up for scan with callback...Displayed 'C'=Image callback.\n");
              previewCount = 0;
              gettimeofday(&start_tv, NULL);
        }
        else {
           printf("Problem starting capture: %d\n", nRc);
           return FALSE;
        }
        break;

     case '4':
        {
			printf("==>%c\n", ch);
			nRc = IBSU_CancelCaptureImage( deviceHandle );
			if( nRc < IBSU_STATUS_OK ) {
				printf("Problem Aborting capture: %d\n", nRc);
				return FALSE;
			}
			else {
				printf("\nCapture stopped.\n");
			}
        }
        break;

     case '5':
 		if( deviceHandle >= 0 ) {
			IBSU_CloseDevice( deviceHandle );
			deviceHandle = -1;
		}
		printf("==>%c\n", ch);
        return FALSE;
        break;

     default:
        printf("???\n");
     }

	 return TRUE;
}

void RunCaptureTest()
{
	while( TRUE )
	{
		if( !DisplayManu() )
			break;

#ifdef WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}
}


int main(int argc, char* argv[])
{
   IBSU_SdkVersion version;
   int nRc = IBSU_GetSDKVersion(&version);
   if( nRc < IBSU_STATUS_OK ) {
      printf("Error %d Retrieving version info\n", nRc);
      exit(1);
   }
   printf("IBScanUltimate Product version: %s, File version: %s\n", version.Product, version.File);

   GetConfiguration();

   if(deviceCount ==0 ) {
      printf("No IB Scan devices attached... exiting\n");
      exit(1);
   }

   nRc = IBSU_RegisterCallbacks ( NULL, ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, (void *)OnDeviceCountChange, context );
   if( nRc < IBSU_STATUS_OK ) {
      printf("Problem registering device count change callback: %d\n", nRc);
      exit(1);
   }

   nRc = IBSU_RegisterCallbacks ( NULL, ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, (void *)OnInitProgressChange, context );
   if( nRc < IBSU_STATUS_OK ) {
      printf("Problem registering init progress change callback: %d\n", nRc);
      exit(1);
   }

   for( int i = 0; i < deviceCount; i++ ) {
      IBSU_DeviceDesc devDesc;
      int rc = IBSU_GetDeviceDescription( i, &devDesc );
      if( rc < IBSU_STATUS_OK ) {
         printf("Error %d Retrieving device description, index # %d\n", rc, i);
         continue;
      }

      string strDevice;
      if( devDesc.productName[0] == 0 )
         strDevice = "unknown device";
      else {
         strDevice = devDesc.productName;
         strDevice += "_";
         strDevice += devDesc.fwVersion;
         strDevice += " S/N(";
         strDevice += devDesc.serialNumber;
         strDevice += ") on ";
         strDevice += devDesc.interfaceType;
      }

      cout << strDevice << endl;

//      if( !OpenCaptureDevice() ) {
//       exit(1);
//      }

      RunCaptureTest();

      if( deviceHandle >= 0 ) {
         rc = IBSU_CloseDevice( deviceHandle );

      }

      printf("\nFinished.\n");

   }

}

