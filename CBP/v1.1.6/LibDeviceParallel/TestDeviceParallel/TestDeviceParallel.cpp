/*******************************************************************************
 * testDeviceParallel.c
 * 
 * DESCRIPTION:
 *     Command-line test application for DeviceParallel
 *     http://www.integratedbiometrics.com
 *
 * NOTES:
 *     Copyright (c) Integrated Biometrics, 2017-2017
 *     
 * HISTORY:
 *     2013/04/06  Initialize.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>

#include "device_parallelApi.h"
#include "device_parallelApi_def.h"
#include "device_parallelApi_err.h"


using namespace std;


#define I2C_WRITE_ADDRESS_CIS 			    0x6E
#define I2C_READ_ADDRESS_CIS 				0x6F

#define LOW  0
#define HIGH 1



/*******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************/

/* The handle of the currently-opened device. */
static int m_deviceHandle = -1;

/* image buffer to receive a frame image */
unsigned char *m_pImage = NULL;


/*******************************************************************************
 * _getch()
 *
 * DESCRIPTION:
 *     Get a character from the terminal on Linux.
 ******************************************************************************/
 
static char _getch(void) 
{
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


/*******************************************************************************
 * ReadFlashMemory()
 *
 * DESCRIPTION:
 *     Read data of flash memory located on Columbo PI.
 ******************************************************************************/

static int ReadFlashMemory(unsigned char *pInfo)
{
	unsigned char cmd[16], dummy[16];
    int transferred;
    int nRc;
    int i;
	
    if (m_deviceHandle == -1)
    {
        return -1;
    }

	nRc = device_spi_init(m_deviceHandle);
    if (nRc != STATUS_OK)
    {
        return nRc;
    }

    for (i=0; i<1894; i++)
	{
		cmd[0] = 0xE8;
		cmd[1] = (i>>6)&0x3F;
		cmd[2] = ((i&0x3F)<<2);
		cmd[3] = 0;
		
		if ((nRc = device_spi_read(m_deviceHandle, cmd, 4, dummy, 4, (unsigned char*)&pInfo[i*528], 528, &transferred, 3000)) != STATUS_OK)
		{
			perror("READ_SCANNER_INFO_ERROR");
			break;
		}
	}

	nRc = device_spi_close();
    if (nRc != STATUS_OK)
    {
        return nRc;
    }
	
	return STATUS_OK;
}	


/*******************************************************************************
 * SetCISRegister()
 *
 * DESCRIPTION:
 *     Wrapper to write the CIS register. 
 ******************************************************************************/

static int SetCISRegister(unsigned char addr, unsigned char value)
{
    unsigned char cmd[2]={0};
    unsigned char buffer[2]={0};

    cmd[0] = addr;
    buffer[0] = value;

	return device_i2c_write(m_deviceHandle, I2C_WRITE_ADDRESS_CIS, cmd, 1, buffer, 1);
}

/*******************************************************************************
 * Initialize_CISResgister()
 *
 * DESCRIPTION:
 *     Initialize CIS register to receive image from CIS camera. 
 ******************************************************************************/

static int Initialize_CISResgister(unsigned short rowStart, unsigned short colStart, unsigned short width, unsigned short height)
{
	unsigned short temp;

    if (m_deviceHandle == -1)
    {
        return -1;
    }

    device_i2c_init(m_deviceHandle);

	if (SetCISRegister(0x03, 0x02) != STATUS_OK) goto done;
	if (SetCISRegister(0x04, 0x83) != STATUS_OK) goto done;
    if (SetCISRegister(0x05, 0x0a) != STATUS_OK) goto done;
    if (SetCISRegister(0x08, 0x00) != STATUS_OK) goto done;
    if (SetCISRegister(0x09, 0x11) != STATUS_OK) goto done;
    if (SetCISRegister(0x0a, 0x11) != STATUS_OK) goto done;
    if (SetCISRegister(0x10, 0x10) != STATUS_OK) goto done;
    if (SetCISRegister(0x11, 0x20) != STATUS_OK) goto done;
    if (SetCISRegister(0x12, 0x88) != STATUS_OK) goto done;
    if (SetCISRegister(0x13, 0x09) != STATUS_OK) goto done;
    if (SetCISRegister(0x14, 0x00) != STATUS_OK) goto done;
    if (SetCISRegister(0x16, 0xb6) != STATUS_OK) goto done;

		temp = 0x04FFu;	
    if (SetCISRegister(0X30, (temp / 256u)) != STATUS_OK) goto done;
    if (SetCISRegister(0X31, (temp % 256u)) != STATUS_OK) goto done;
	
    if (SetCISRegister(0X32, 0x20) != STATUS_OK) goto done;

    if (SetCISRegister(0x41, 0x01) != STATUS_OK) goto done;
    if (SetCISRegister(0x42, 0x67) != STATUS_OK) goto done; //Horizontal Start L
    if (SetCISRegister(0x43, 0x03) != STATUS_OK) goto done; //Horizontal Width H
    if (SetCISRegister(0x44, 0x6F) != STATUS_OK) goto done; //Horizontal Width L

    if (SetCISRegister(0X45, 0x00) != STATUS_OK) goto done; //Vertical Start H
    if (SetCISRegister(0X46, 0x32) != STATUS_OK) goto done; //Vertical Start L
    if (SetCISRegister(0X47, 0x04) != STATUS_OK) goto done; //Vertical Height H
    if (SetCISRegister(0X48, 0x4B) != STATUS_OK) goto done; //Vertical Height L

    ////////////////////////////////////////////////////////////////////////////
    // Important! To make the frame rate
    // Control for VSYNC blank time
	// 0x0370 - 10fps; 0x0230 - 12fps
		temp = 0x0000u;	
    if (SetCISRegister(0X49, (temp / 256u)) != STATUS_OK) goto done;
    if (SetCISRegister(0X4A, (temp % 256u)) != STATUS_OK) goto done;
    ////////////////////////////////////////////////////////////////////////////
    if (SetCISRegister(0X4B, 0x00) != STATUS_OK) goto done;
    if (SetCISRegister(0X4C, 0x00) != STATUS_OK) goto done;
  
		temp = 0x00032u;	
    if (SetCISRegister(0X4D, (temp / 256u)) != STATUS_OK) goto done; // H blank H
    if (SetCISRegister(0X4E, (temp % 256u)) != STATUS_OK) goto done; // H blank L

    if (SetCISRegister(0x60, 0x0b) != STATUS_OK) goto done;
    if (SetCISRegister(0x61, 0x16) != STATUS_OK) goto done;
    if (SetCISRegister(0x62, 0x32) != STATUS_OK) goto done;
    if (SetCISRegister(0x71, 0x08) != STATUS_OK) goto done;

    if (SetCISRegister(0x80, 0xE0) != STATUS_OK) goto done; //IPFUN Ori : e8
    if (SetCISRegister(0x80, 0x0F) != STATUS_OK) goto done; //SIGCNT Ori : 07

    if (SetCISRegister(0x90, 0x00) != STATUS_OK) goto done;
    if (SetCISRegister(0x91, 0x08) != STATUS_OK) goto done;
    if (SetCISRegister(0x92, 0x10) != STATUS_OK) goto done;
    if (SetCISRegister(0x93, 0x40) != STATUS_OK) goto done;
    if (SetCISRegister(0x94, 0x04) != STATUS_OK) goto done;
    if (SetCISRegister(0x95, 0x01) != STATUS_OK) goto done;
    if (SetCISRegister(0x96, 0x80) != STATUS_OK) goto done;
    if (SetCISRegister(0x97, 0x08) != STATUS_OK) goto done;
    if (SetCISRegister(0x98, 0x10) != STATUS_OK) goto done;
    if (SetCISRegister(0x99, 0x08) != STATUS_OK) goto done;
    if (SetCISRegister(0x9a, 0x03) != STATUS_OK) goto done;
    if (SetCISRegister(0x9b, 0xb0) != STATUS_OK) goto done;
    if (SetCISRegister(0x9c, 0x08) != STATUS_OK) goto done;
    if (SetCISRegister(0x9d, 0x24) != STATUS_OK) goto done;
    if (SetCISRegister(0x9e, 0x30) != STATUS_OK) goto done;
    if (SetCISRegister(0xb8, 0x28) != STATUS_OK) goto done;
    if (SetCISRegister(0x03, 0x05) != STATUS_OK) goto done;

	device_i2c_close();

	return STATUS_OK;
		
done:
	
	device_i2c_close();
		
    perror("INIT_CIS_ERROR");
 	
	return -1;
}


/*******************************************************************************
 * GetOneFrameImage()
 *
 * DESCRIPTION:
 *     Receives a frame image via parallel interface.
 ******************************************************************************/

int GetOneFrameImage(unsigned char *pImage)
{
    int buffer_len = 968000;
    int transferred;

    if (m_deviceHandle == -1)
    {
        return -1;
    }

    device_parallel_init(m_deviceHandle);

    if (device_parallel_bulk_read(m_deviceHandle, pImage, buffer_len, &transferred, 3000) != STATUS_OK)
    {
        return -1;
    }

	return STATUS_OK;
}

/*******************************************************************************
 * DisplayMenu()
 *
 * DESCRIPTION:
 *     Display the menu, accept a character, and perform the associated action.
 ******************************************************************************/

static int DisplayMenu()
{
    char ch;
    int nRc;

    printf("\nReady.   Enter choice:\n" \
           "\t1. CIS initialize.\n" \
           "\t2. Get OneFrameImage from Columbo PI.\n" \
           "\tq. End program\n:");

     ch = _getch();
     switch(ch) 
     {
         case '1':
             printf("==>%c\n", ch);
             nRc = Initialize_CISResgister(0, 0, 880, 1100);
             if (nRc != STATUS_OK) 
             {
                 return 0;
             }
             break;

         case '2':
             printf("==>%c\n", ch);
             nRc = GetOneFrameImage(m_pImage);
             if (nRc != STATUS_OK) 
             {
                 return 0;
             }
             break;

         case 'q':
             if(m_deviceHandle >= 0) 
             {
                 device_close(m_deviceHandle);
                 m_deviceHandle = -1;
             }
             printf("==>%c\n", ch);
             return 0;

         default:
             printf("???\n");
             break;
    }

    return 1;
}

/*******************************************************************************
 * RunCaptureTest()
 *
 * DESCRIPTION:
 *     Run a capture test.
 ******************************************************************************/
 
static void RunCaptureTest()
{
    while(1)
    {
        if(!DisplayMenu())
            break;

        usleep(1000);
    }
}


/*******************************************************************************
 * main()
 * 
 * DESCRIPTION:
 *     Application entry point
 ******************************************************************************/
 
int main(int argc, char* argv[])
{
#define PROPERTY_LENGTH     32
	unsigned char *pInfo = new unsigned char [1000960];
    int             deviceCount = 0;
    int             nRc;
    char		cProductID[PROPERTY_LENGTH+1]={0};
	char		cSerialNumber[PROPERTY_LENGTH+1]={0};
	char		cVendorID[PROPERTY_LENGTH+1]={0};
	char		cIBIA_VendorID[PROPERTY_LENGTH+1]={0};
	char		cIBIA_Version[PROPERTY_LENGTH+1]={0};
	char		cIBIA_DeviceID[PROPERTY_LENGTH+1]={0};
	char		cFirmware[PROPERTY_LENGTH+1]={0};
	char		cDevRevision[PROPERTY_LENGTH+1]={0};
	char		cProductionDate[PROPERTY_LENGTH+1]={0};
	char		cServiceDate[PROPERTY_LENGTH+1]={0};
	char		cReserved_1[PROPERTY_LENGTH+1]={0};
	unsigned short idVendor;
	unsigned short idProduct;
    string         strDevice;

    /* Initialize device */
    nRc = device_control_init(0);
    if (nRc  < STATUS_OK)
    {
        printf("Error %d device_control_init()\n", nRc);
        exit(1);
    }

    nRc = device_control_reg_write(0, MAIN_POWER, HIGH);
    if (nRc  < STATUS_OK)
    {
        printf("Error %d GPIO writing for main power\n", nRc);
        exit(1);
    }
    nRc = device_control_reg_write(0, LE_POWER, HIGH);
    if (nRc  < STATUS_OK)
    {
        printf("Error %d GPIO writing for LE power\n", nRc);
        exit(1);
    }
    nRc = device_control_reg_write(0, CIS_RESET, HIGH);
    if (nRc  < STATUS_OK)
    {
        printf("Error %d GPIO writing for CIS reset\n", nRc);
        exit(1);
    }
    nRc = device_control_reg_write(0, LE_ENABLE, HIGH);
    if (nRc  < STATUS_OK)
    {
        printf("Error %d GPIO writing LE enable\n", nRc);
        exit(1);
    }

    /* Get the number of attached scanners. */
    device_count(&deviceCount);
    if (deviceCount <= 0)
    {
        printf("No IB Scan devices attached... exiting\n");
        exit(1);
    }

    /* Intialize device */
	nRc = device_init();
    if (nRc  < STATUS_OK)
    {
        printf("Error %d device initialize\n", nRc);
        exit(1);
    }
	
    /* Open device */
    nRc = device_open(&m_deviceHandle);
    if (nRc  < STATUS_OK)
    {
        printf("Error %d device open\n", nRc);
        exit(1);
    }

    /* Get device description from flash memory. */
    nRc = ReadFlashMemory(pInfo);
    if (nRc < STATUS_OK) 
    {
        printf("Error %d Retrieving device description\n", nRc);
        device_close(m_deviceHandle);
        exit(1);
    }

	memcpy(cProductID,      &pInfo[0*PROPERTY_LENGTH], PROPERTY_LENGTH);
	memcpy(cSerialNumber,   &pInfo[1*PROPERTY_LENGTH], PROPERTY_LENGTH);
	memcpy(cVendorID,       &pInfo[2*PROPERTY_LENGTH], PROPERTY_LENGTH);
	memcpy(cIBIA_VendorID,  &pInfo[3*PROPERTY_LENGTH], PROPERTY_LENGTH);
	memcpy(cIBIA_Version,   &pInfo[4*PROPERTY_LENGTH], PROPERTY_LENGTH);
	memcpy(cIBIA_DeviceID,  &pInfo[5*PROPERTY_LENGTH], PROPERTY_LENGTH);
	memcpy(cFirmware,       &pInfo[6*PROPERTY_LENGTH], PROPERTY_LENGTH);
	memcpy(cDevRevision,    &pInfo[7*PROPERTY_LENGTH], PROPERTY_LENGTH);
	memcpy(cProductionDate, &pInfo[8*PROPERTY_LENGTH], PROPERTY_LENGTH);
	memcpy(cServiceDate,    &pInfo[9*PROPERTY_LENGTH], PROPERTY_LENGTH);
	memcpy(cReserved_1,     &pInfo[10*PROPERTY_LENGTH], PROPERTY_LENGTH);
	memcpy(&idVendor,       &pInfo[11*PROPERTY_LENGTH], 2);
	memcpy(&idProduct,      &pInfo[11*PROPERTY_LENGTH+2], 2);

    /* Format description of scanner. */
    if (cProductID[0] == 0)
    {
        strDevice = "unknown device";
    }
    else 
    {
        strDevice  = cProductID;
        strDevice += "_";
        strDevice += cFirmware;
        strDevice += " S/N(";
        strDevice += cSerialNumber;
        strDevice += ")";
    }
    cout << strDevice << endl;

    /* Display menu for functional test. */
	m_pImage = new unsigned char [1000960];
    RunCaptureTest();

    /* Close the device handle, if still open. */
    if (m_deviceHandle >= 0) 
    {
        nRc = device_close(m_deviceHandle);
        if (nRc < STATUS_OK)
        {
            printf("Error %d closing device\n", nRc);
        }
    }

    printf("\nFinished.\n");
	
	delete [] m_pImage;
	delete [] pInfo;	
	
	return 0;	
}
