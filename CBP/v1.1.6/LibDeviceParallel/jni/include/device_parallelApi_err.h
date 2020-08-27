
// device_parallelApi_err.h : device Parallel API header file for the libDeviceParallel
//

#ifndef __DEVICE_PARALLELAPI_ERR_H__
#define __DEVICE_PARALLELAPI_ERR_H__

#pragma once

/*
****************************************************************************************************
* GENERIC ERROR CODES
****************************************************************************************************
*/
                                                        
#define STATUS_OK                                   0       /* Function completed successfully. */
#define ERR_INVALID_PARAM_VALUE                     -1      /* Invalid parameter value. */
#define ERR_MEM_ALLOC                               -2      /* Insufficient memory. */
#define ERR_NOT_SUPPORTED                           -3      /* Requested functionality isn't supported. */
#define ERR_FILE_OPEN                               -4      /* Device file (I2C, SPI) open failed. */
#define ERR_FILE_READ                               -5      /* Device file (I2C, SPI) read failed. */
#define ERR_FILE_WRITE                              -6      /* Device file (I2C, SPI) write failed. */
#define ERR_NOT_INITIALIZED                         -7      /* Needs to be initialized. */


/*
****************************************************************************************************
* GPIO ERROR CODES
****************************************************************************************************
*/
#define ERR_GPIO_EXPORT_FAILED                      -100    /* GPIO enable failed. */
#define ERR_GPIO_UNEXPORT_FAILED                    -101    /* GPIO disable failed. */
#define ERR_GPIO_DIRECTION_FAILED                   -102    /* GPIO set direction failed. */
#define ERR_GPIO_READ_FAILED                        -103    /* GPIO read failed. */
#define ERR_GPIO_WRITE_FAILED                       -104    /* GPIO write failed. */


/*
****************************************************************************************************
* DEVICE-RELATED ERROR CODES
****************************************************************************************************
*/
#define ERR_DEVICE_IO                               -200    /* Device communication failed. */
#define ERR_DEVICE_NOT_FOUND                        -201    /* No device is detected/active. */
#define ERR_DEVICE_NOT_OPENED                       -204    /* Device needs to be opened. */


/*
****************************************************************************************************
* I2C ERROR CODES
****************************************************************************************************
*/
#define ERR_I2C_OPEN_FAILED                         -300    /* I2C open failed. */
#define ERR_I2C_READ_FAILED                         -301    /* I2C read failed. */
#define ERR_I2C_WRITE_FAILED                        -302    /* I2C write failed. */
#define ERR_I2C_ADDRESS_FAILED                      -303    /* I2C address set up failed. */


/*
****************************************************************************************************
* SPI ERROR CODES
****************************************************************************************************
*/
#define ERR_SPI_OPEN_FAILED                         -400    /* SPI open failed. */
#define ERR_SPI_READ_FAILED                         -401    /* SPI read failed. */
#define ERR_SPI_WRITE_FAILED                        -402    /* SPI write failed. */
#define ERR_SPI_COMMAND_FAILED                      -403    /* SPI set up failed. */



/*
****************************************************************************************************
* WARNING CODES
****************************************************************************************************
*/
#define WRN_ALREADY_INITIALIZED                    100      /* Device/component has already been initialized and is ready to be used. */


#endif /* __DEVICE_PARALLELAPI_ERR_H__ */

