// Parallel.h : Parallel header file for the IBScanUltimate DLL
//

#ifndef __PARALLEL_H__
#define __PARALLEL_H__

#pragma once

#include "IBSU_Global_Def.h"
#include "IBScanUltimateApi_defs.h"


#define	PARALLEL_MAX_STR_LEN 	(32)
#define __I2C_WRITE_ADDRESS_COLUMBO_CIS__		(0X6E)
#define __I2C_READ_ADDRESS_COLUMBO_CIS__		(0X6F)
#define __I2C_WRITE_ADDRESS_COLUMBO_EL__		(0XC0)
#define __I2C_READ_ADDRESS_COLUMBO_EL__			(0XC1)


typedef struct parallel_device {	
	char			cProductID[PARALLEL_MAX_STR_LEN];
	char			cSerialNumber[PARALLEL_MAX_STR_LEN];
	char			cVendorID[PARALLEL_MAX_STR_LEN];
	char			cIBIA_VendorID[PARALLEL_MAX_STR_LEN];
	char			cIBIA_Version[PARALLEL_MAX_STR_LEN];
	char			cIBIA_DeviceID[PARALLEL_MAX_STR_LEN];
	char			cFirmware[PARALLEL_MAX_STR_LEN];
	char			cDevRevision[PARALLEL_MAX_STR_LEN];
	char			cProductionDate[PARALLEL_MAX_STR_LEN];
	char			cServiceDate[PARALLEL_MAX_STR_LEN];
	char			cFPGA[PARALLEL_MAX_STR_LEN];
	char			cReserved_1[PARALLEL_MAX_STR_LEN];
	unsigned short  idVendor;
	unsigned short	idProduct;
} Parallel_device, *pParallel_device;


#endif /* __PARALLEL_H__ */

