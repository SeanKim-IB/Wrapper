////////////////////////////////////////////////////////////////////////////
/*
 * This is a blank example to create libDeviceParallel shared library
 * for the Linux based Embedded System want to use IB Columbo PI scanner.
 * It may not work with your Embedded System, but it might be useful 
 */
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>

// This is a temporary data for demostration based on binary that includes device information and mask data
#include "mask.lut"
// This is a temporary data for demostration based on binary that includes raw fingerprint image
#include "raw_image.lut"

#include "device_parallelApi.h"
#include "device_parallelApi_def.h"
#include "device_parallelApi_err.h"


/*
****************************************************************************************************
* device_control_init()
* 
* DESCRIPTION:
*     Initializes an device control to control (IN/OUT) GPIO's 
*     regarding Main Power, LE Power, CIS camera reset, LE enable.
*
* ARGUMENTS:
*     index  Device index is given from Host (Your Embedded System)
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_control_init (const int index)
{
    return STATUS_OK;
}

/*
****************************************************************************************************
* device_control_close()
* 
* DESCRIPTION:
*     Releases a device control
*
* ARGUMENTS:
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_control_close (const int index)
{
	return STATUS_OK;
}

/*
****************************************************************************************************
* device_control_reg_read()
* 
* DESCRIPTION:
*     Get a state of the device control register (Read GPIO value)
*     regarding Main Power, LE Power, CIS camera reset, LE enable.
*
* ARGUMENTS:
*     index  Device index is given from Host (Your Embedded System)
*     eCtrl  refer to Enum DEVICE_CONTROL
*            'MAIN_POWER' : Main power for IBScanner PPI module
*            'LE_POWER'   : LE power for operating IB LE film
*            'CIS_RESET'  : CIS reset control
*            'LE_ENABLE'  : LE power on/off
*            'LIVE_STATE' : State of live pin
*     pState Pointer to variable that will receive state of GPIO pin.
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_control_reg_read (const int index, DEVICE_CONTROL eCtrl, unsigned char *pState)
{
    return STATUS_OK;
}

/*
****************************************************************************************************
* device_control_reg_write()
* 
* DESCRIPTION:
*     Get a state of the device control register (Read GPIO value)
*     regarding Main Power, LE Power, CIS camera reset, LE enable.
*
* ARGUMENTS:
*     index  Device index is given from Host (Your Embedded System)
*     eCtrl  refer to Enum DEVICE_CONTROL
*            'MAIN_POWER' : Main power for IBScanner PPI module
*            'LE_POWER'   : LE power for operating IB LE film
*            'CIS_RESET'  : CIS reset control
*            'LE_ENABLE'  : LE power on/off
*            'LIVE_STATE' : State of live pin
*     value  State value that will write to GPIO pin.
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_control_reg_write (const int index, DEVICE_CONTROL eCtrl, const unsigned char value)
{
    return STATUS_OK;
}

/*
****************************************************************************************************
* device_count()
* 
* DESCRIPTION:
*     Get actual number of connected IB devices
*
* ARGUMENTS:
*     pDeviceCount Pointer to variable that will receive number of connected IB devices.
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_count(int *pDeviceCount)
{
    *pDeviceCount = 1;

    return STATUS_OK;
}

/*
****************************************************************************************************
* device_init()
* 
* DESCRIPTION:
*     Initializes IB devices
*
* ARGUMENTS:
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_init ()
{
    return STATUS_OK;
}

/*
****************************************************************************************************
* device_open()
* 
* DESCRIPTION:
*     Open IB device that has a parallel interface, given a particular device index
*
* ARGUMENTS:
*     pHandle  Pointer to variable that will receive device handle for subsequent function calls.
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_open (int *pHandle)
{
	
	*pHandle = 0;
    return STATUS_OK;
}

/*
****************************************************************************************************
* device_close()
* 
* DESCRIPTION:
*     Close IB device by device handle
*
* ARGUMENTS:
*     handle  Device handle obtained by device_open()
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_close (const int handle)
{
    return STATUS_OK;
}

/*
****************************************************************************************************
* device_parallel_init()
* 
* DESCRIPTION:
*     Initializes a parallel interface with the register setups for the Embedded System
*
* ARGUMENTS:
*     handle  Device handle obtained by device_open()
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_parallel_init (const int handle)
{
    return STATUS_OK;
}

/*
****************************************************************************************************
* device_parallel_bulk_read()
* 
* DESCRIPTION:
*     Reads image data via parallel interface for a paticular device by handle.
*     A timeout value of zero means no timeout.
*     The timeout value is given in milliseconds.
*
* ARGUMENTS:
*     handle  Device handle obtained by device_open()
*     pBuffer Pointer to variable that will receive the image data that is read.
*     buffer_len The number of bytes to read. This number must be less than or equal to the size,
*                in bytes, of buffer.
*     pLen_transferre Pointer to a integer variable that receives the actual number of bytes
*                     that were copied into buffer.
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_parallel_bulk_read (const int handle, unsigned char *pBuffer, const int buffer_len, int *pLen_transferred, const int timeout)
{
	memcpy(pBuffer, &raw_image[0], buffer_len);
	*pLen_transferred = buffer_len;

    return STATUS_OK;
}

/*
****************************************************************************************************
* device_parallel_close()
* 
* DESCRIPTION:
*     Releases a parallel interface
*
* ARGUMENTS:
*     handle  Device handle obtained by device_open()
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_parallel_close (const int handle)
{
    return STATUS_OK;
}

/*
****************************************************************************************************
* device_i2c_init()
* 
* DESCRIPTION:
*     Initializes an i2C interface with the register setups for the Embedded System
*
* ARGUMENTS:
*     handle  Device handle obtained by device_open()
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_i2c_init (const int handle)
{
	return STATUS_OK;
}

/*
****************************************************************************************************
* device_i2c_read()
* 
* DESCRIPTION:
*     Reads register value of CIS camera via I2C interface
*
* ARGUMENTS:
*     handle  Device handle obtained by device_open()
*     dev_addr  I2C address of device for the communication between a paticular device and Embedded System
*     cmd  A register address to read the register value of CIS
*     cmd_len  The number of bytes for 'cmd'. This number must be less than or equal to the size,
*              in bytes, of 'cmd'
*     pBuffer  A buffer that receiveds the register value of CIS that is read.
*     buffer_len  The number of bytes to read. This number must be less than or equal to the size,
*                 in bytes, of 'buffer'
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_i2c_read (const int handle, const unsigned char dev_addr, const unsigned char *cmd, const int cmd_len,
                     unsigned char *pBuffer, const int buffer_len)
{
	return STATUS_OK;
}

/*
****************************************************************************************************
* device_i2c_write()
* 
* DESCRIPTION:
*     Write register value of CIS camera via I2C interface
*
* ARGUMENTS:
*     handle  Device handle obtained by device_open()
*     dev_addr  I2C address of device for the communication between a paticular device and Embedded System
*     cmd  A register address to write the register value of CIS
*     cmd_len  The number of bytes for 'cmd'. This number must be less than or equal to the size,
*              in bytes, of 'cmd'
*     pBuffer  A buffer that send the register value of CIS.
*     buffer_len  The number of bytes to read. This number must be less than or equal to the size,
*                 in bytes, of 'buffer'
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_i2c_write (const int handle, const unsigned char dev_addr, const unsigned char *cmd, const int cmd_len,
                      const unsigned char *buffer, const int buffer_len)
{
	return STATUS_OK;
}

/*
****************************************************************************************************
* device_i2c_close()
* 
* DESCRIPTION:
*     Releases an i2C interface
*
* ARGUMENTS:
*     handle  Device handle obtained by device_open()
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_i2c_close(const int handle)
{
    return STATUS_OK;
}

/*
****************************************************************************************************
* device_spi_init()
* 
* DESCRIPTION:
*     Initializes a SPI interface with the register setups for the Embedded System
*
* ARGUMENTS:
*     handle  Device handle obtained by device_open()
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_spi_init (const int handle)
{
    return STATUS_OK;
}

/*
****************************************************************************************************
* device_spi_read()
* 
* DESCRIPTION:
*     Reads uniformity mask data via SPI interface for a paticular device by handle.
*
* ARGUMENTS:
*     handle  Device handle obtained by device_open()
*     cmd  A commnand to read the uniformity mask data
*     cmd_len  The number of bytes for 'cmd'. This number must be less than or equal to the size,
*              in bytes, of 'cmd'
*     dummy  A dummy command.
*     dummy_len  The number of bytes for 'dummy'. This number must be less than or equal to the size,
*                 in bytes, of 'dummy'
*     pBuffer  A buffer that receives the uniformity mask data that is read.
*     buffer_len  The number of bytes to read. This number must be less than or equal to the size,
*                 in bytes, of 'buffer'
*     pLen_transferred  A pointer to an integer vaiable that receives the actual number of bytes
*                       that were copied into buffer
*     timeout  The timeout (in milliseconds) that is function should wait before giving up due to
*              no response being received. For an unlimited timeout, use value 0.
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/

int device_spi_read (const int handle, const unsigned char *cmd, const int cmd_len, unsigned char *dummy, const int dummy_len,
                     unsigned char *pBuffer, const int buffer_len, int *pLen_transferred, const int timeout)
{
	if (cmd[0] == 0xE8)
	{
		int pageNum = (cmd[1]<<6) + (cmd[2]>>2);
		memcpy(pBuffer, &mask[pageNum*buffer_len], buffer_len);
		*pLen_transferred = buffer_len;
	}
	
    return STATUS_OK;
}

/*
****************************************************************************************************
* device_spi_write()
* 
* DESCRIPTION:
*     write uniformity mask data via SPI interface for a paticular device by handle.
*
* ARGUMENTS:
*     handle  Device handle obtained by device_open()
*     cmd  A commnand to write the uniformity mask data
*     cmd_len  The number of bytes for 'cmd'. This number must be less than or equal to the size,
*              in bytes, of 'cmd'
*     dummy  A dummy command.
*     dummy_len  The number of bytes for 'dummy'. This number must be less than or equal to the size,
*                 in bytes, of 'dummy'
*     buffer  A buffer that send the uniformity mask data.
*     buffer_len  The number of bytes to read. This number must be less than or equal to the size,
*                 in bytes, of 'buffer'
*     pLen_transferred  A pointer to an integer vaiable that send the actual number of bytes
*                       that were copied into buffer
*     timeout  The timeout (in milliseconds) that is function should wait before giving up due to
*              no response being received. For an unlimited timeout, use value 0.
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_spi_write (const int handle, const unsigned char *cmd, const int cmd_len, unsigned char *dummy, const int dummy_len,
                      unsigned char *buffer, const int buffer_len, int *pLen_transferred, const int timeout)
{
    return STATUS_OK;
}

/*
****************************************************************************************************
* device_api_close()
* 
* DESCRIPTION:
*     Releases a SPI interface
*
* ARGUMENTS:
*     handle  Device handle obtained by device_open()
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_spi_close (const int handle)
{
    return STATUS_OK;
}

