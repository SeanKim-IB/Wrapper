////////////////////////////////////////////////////////////////////////////
/*
 * This is an example to create libDeviceParallel shared library
 * for the Linux based Embedded System want to use IB Columbo PI scanner.
 * It may not work with your Embedded System, but it might be useful 
 */
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <linux/i2c.h>
#include <getopt.h>
#include <linux/spi/spidev.h>

#include "device_parallelApi.h"
#include "device_parallelApi_def.h"
#include "device_parallelApi_err.h"

#define DEVICE_PATH_BASE_GPIO "/sys/class/gpio"		// Depends on target system (it is not fixed)
#define DEVICE_PATH_I2C "/dev/i2c-1"			    // Depends on target system (it is not fixed)
#define DEVICE_PATH_SPI "/dev/spidev1.1"	        // Depends on target system (it is not fixed)


#define __G_DEBUG__

#define __MAX_BUFFER__      1024
#define GPIO_MAIN_POWER     1
#define GPIO_LE_POWER       2
#define GPIO_CIS_RESET      3
#define GPIO_LE_ENABLE      4
#define GPIO_LIVE_STATE     5

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

#define SPI_MODE	0
#define SPI_BPW		8
#define SPI_DELAY	0

#define PAGE_SIZE 64

char buf_spi_tx[1024];
struct spi_ioc_transfer xfer[2];

typedef union _IO_PACKET {
	struct _CMD_PACKET {
		unsigned char Ioctl;
		unsigned char Buffer[PAGE_SIZE-1];
	} cmd;
	struct _STAT_PACKET {
		unsigned char Data[PAGE_SIZE];
	} stat;
} IO_PACKET, *PIO_PACKET;

typedef struct _node {
    int handle;
    struct _node *next;
} NODE;

NODE *m_head, *m_tail;

int m_fd_i2c = -1;
int m_fd_spi = -1;


void _init_list();
void _delete_all();
int _delete_node(int handle);
NODE *_search_node(int handle);
int _createHandle();
int _gpioExport(int pin);
int _gpioUnexport(int pin);
int _gpioDirection(int pin, int direction);
int _gpioRead(int pin);
int _gpioWrite(int pin, int value);
int _lsValidHandle(int handle);



//////////////////////////////////////////////////////////////////////////////////////////
// Linked list to manage handle
void _init_list()
{
    m_head = (NODE *)malloc(sizeof(NODE));
    m_tail = (NODE *)malloc(sizeof(NODE));

    m_head->next = m_tail;
    m_tail->next = m_tail;

#if defined(__G_DEBUG__)
    printf("Initialize the list,...\n");
#endif
}

void _insert_node(int handle)
{
    NODE *n = (NODE *)malloc(sizeof(NODE));
    
    n->handle = handle;
    n->next = m_head->next;
    m_head->next = n;

#if defined(__G_DEBUG__)
    printf("INSERT [%d]\n", handle);
#endif
}

void _delete_all()
{
    NODE *t, *u;

    t = m_head->next;

    while (t != m_tail)
    {
        u = t;
        t = t->next;
        free(u);
    }

    m_head->next = m_tail;

#if defined(__G_DEBUG__)
    printf("DELETED ALL\n");
#endif
}

int _delete_node(int handle)
{
    NODE *s, *t;

    s = _search_node(handle);

    if (s == NULL)
    {
        return -1;
    }

    t = s->next;
    s->next = t->next;
    free(t);

#if defined(__G_DEBUG__)
    printf("DELETE [%d]\n", handle);
#endif

    return 0;
}

NODE *_search_node(int handle)
{
    NODE *t, *n;

    n = m_head;
    t = n->next;

    while ((handle != t->handle) && t != m_tail)
    {
        n = n->next;
        t = n->next;
    }

    if (t == m_tail)
    {
#if defined(__G_DEBUG__)
        printf("Can't find the handle!\n");
#endif
        return NULL;
    }

    return n;
}

int _createHandle()
{
    int maxHandle = -1;
    NODE *t, *n;

    n = m_head;
    t = n->next;

    while (t != m_tail)
    {
        if (t->handle > maxHandle)
        {
            maxHandle = t->handle;
        }

        n = n->next;
        t = n->next;
    }

    maxHandle++;
#if defined(__G_DEBUG__)
        printf("New handle was created (%d)\n", maxHandle);
#endif
    return maxHandle;
}


//////////////////////////////////////////////////////////////////////////////////////////
// For managing GPIO
int _gpioExport(int pin)
{
    char buffer[__MAX_BUFFER__]={0};
    int fd;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open export for GPIO writing!\n");
        return -1;
    }
    sprintf(buffer, "%d", pin); 
    write(fd, buffer, strlen(buffer));
    close(fd);
    return 0;
}

int _gpioUnexport(int pin)
{
    char buffer[__MAX_BUFFER__]={0};
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open unexport for GPIO writing!\n");
        return -1;
    }
    sprintf(buffer, "%d", pin); 
    write(fd, buffer, strlen(buffer));
    close(fd);
    return 0;
}

int _gpioDirection(int pin, int direction) // 1 for output, 0 for input
{
	static const char s_directions_str[]  = "in\0out";
    char path[__MAX_BUFFER__]={0};
    int fd;

    sprintf(path, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open GPIO direction for writing!\n");
        return -1;
    }

	if (-1 == write(fd, &s_directions_str[IN == direction ? 0 : 3], IN == direction ? 2 : 3))
    {
		fprintf(stderr, "Failed to set direction!\n");
		return -1;
	}
    close(fd);
    return 0;
}

int _gpioRead(int pin)
{
    char path[__MAX_BUFFER__];
    char value_str[3]={0};
    int fd;

    sprintf(path, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open GPIO value for reading!\n");
        return -1;
    }

	if (-1 == read(fd, value_str, 3))
    {
		fprintf(stderr, "Failed to read value!\n");
		return -1;
	}
    close(fd);

    return (atoi(value_str));
}

int _gpioWrite(int pin, int value)
{
	static const char s_values_str[]  = "01";
    char path[__MAX_BUFFER__]={0};
    int fd;

    sprintf(path, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open GPIO value for writing!\n");
        return -1;
    }

	if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
		fprintf(stderr, "Failed to write value!\n");
		return -1;
	}
    close(fd);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Etc
int _lsValidHandle(int handle)
{
    int nDevices;
    int nRc;
    
    if ((nRc = device_count(&nDevices)) != STATUS_OK)
    {
        return nRc;
    }

    if (nDevices == 0)
    {
        return ERR_DEVICE_NOT_FOUND;
    }

    if (_search_node(handle) == NULL)
    {
        return ERR_DEVICE_NOT_OPENED;
    }

    return STATUS_OK;
}







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
    if (index == 0)
    {
        /* Enable GPIO pins */
        if (-1 == _gpioExport(GPIO_MAIN_POWER) ||
            -1 == _gpioExport(GPIO_LE_POWER) ||
            -1 == _gpioExport(GPIO_CIS_RESET) ||
            -1 == _gpioExport(GPIO_LE_ENABLE) ||
            -1 == _gpioExport(GPIO_LIVE_STATE))
        {
            return ERR_GPIO_EXPORT_FAILED;
        }

        /* Set GPIO directions */
        if (-1 == _gpioDirection(GPIO_MAIN_POWER, OUT) ||
            -1 == _gpioDirection(GPIO_LE_POWER, OUT) ||
            -1 == _gpioDirection(GPIO_CIS_RESET, OUT) ||
            -1 == _gpioDirection(GPIO_LE_ENABLE, OUT) ||
            -1 == _gpioDirection(GPIO_LIVE_STATE, IN))
        {
            return ERR_GPIO_DIRECTION_FAILED;
        }
    }
    else
    {
        /* Your embedded system does not a plen to support multiple IB scanners at the same time */
        return ERR_INVALID_PARAM_VALUE;
    }

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
    int pin;
    unsigned char nRc;

    *pState = -1;
    if (index == 0)
    {
        switch(eCtrl)
        {
        case LIVE_STATE:
            pin = GPIO_LIVE_STATE; break;
        default:
            return ERR_INVALID_PARAM_VALUE;
        }

        /* Read GPIO value */
        nRc =  (unsigned char)_gpioRead(pin);
        if (-1 == nRc)
        {
            return ERR_GPIO_READ_FAILED;
        }
        
        *pState = nRc;
    }
    else
    {
        /* Your embedded system does not a plen to support multiple IB scanners at the same time */
        return ERR_INVALID_PARAM_VALUE;
    }

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
    int pin;

    if (index == 0)
    {
        switch(eCtrl)
        {
        case MAIN_POWER:
            pin = GPIO_MAIN_POWER; break;
        case LE_POWER:
            pin = GPIO_LE_POWER; break;
        case CIS_RESET:
            pin = GPIO_CIS_RESET; break;
        case LE_ENABLE:
            pin = GPIO_LE_ENABLE; break;
        default:
            return ERR_INVALID_PARAM_VALUE;
        }

        /* Write GPIO value */
        if (-1 == _gpioWrite(pin, value))
        {
            return ERR_GPIO_WRITE_FAILED;
        }
    }
    else
    {
        /* Your embedded system does not a plen to support multiple IB scanners at the same time */
        return ERR_INVALID_PARAM_VALUE;
    }

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
    int liveState;
    int nRc;
    *pDeviceCount = 0;

    if ((nRc = device_control_reg_read(0, LIVE_STATE, (unsigned char*)&liveState)) != STATUS_OK)
    {
        return nRc;
    }

    if (liveState == HIGH)
    {
        *pDeviceCount = 1;
    }

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
    _init_list();

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
    int nDevices;
    int nRc;
    
    *pHandle = -1;
    if ((nRc = device_count(&nDevices)) != STATUS_OK)
    {
        return nRc;
    }

    if (nDevices == 0)
    {
        return ERR_DEVICE_NOT_FOUND;
    }

    *pHandle = _createHandle();
    
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
    if (-1 == _delete_node(handle))
    {
        return ERR_DEVICE_NOT_OPENED;
    }

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
    int nRc;

    if ((nRc = _lsValidHandle(handle)) != STATUS_OK)
    {
        return nRc;
    }

    /* Need to add an intialize code for parallel interface */

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
    int nRc;

    if ((nRc = _lsValidHandle(handle)) != STATUS_OK)
    {
        return nRc;
    }

    /* Need to add an proper code to receive bulk image data via parallel interface */

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
    int nRc;

    if ((nRc = _lsValidHandle(handle)) != STATUS_OK)
    {
        return nRc;
    }

	m_fd_i2c = open(DEVICE_PATH_I2C, O_RDWR);
	if (m_fd_i2c < 0)
	{
		printf("%s\n", strerror(errno));
		return ERR_I2C_OPEN_FAILED;
	}
	
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
    int nRc;

    if ((nRc = _lsValidHandle(handle)) != STATUS_OK)
    {
        return nRc;
    }
    
    if (-1 == m_fd_i2c)
    {
        return ERR_I2C_OPEN_FAILED;
    }
	
	if (device_i2c_write(handle, dev_addr, cmd, cmd_len, pBuffer, 0) != STATUS_OK)
	{
	    printf("read_from_device(): address reset did not work\n");
	    return ERR_I2C_WRITE_FAILED;
	}
	
	if (read(m_fd_i2c, (char*)pBuffer, buffer_len) != buffer_len)
	{
	    printf("read(): %s\n", strerror(errno));
	    return ERR_I2C_READ_FAILED;
	}
	
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
    IO_PACKET	io_packet;
    int         lByteCount;
    int         nRc;

    if ((nRc = _lsValidHandle(handle)) != STATUS_OK)
    {
        return nRc;
    }
    
    if (-1 == m_fd_i2c)
    {
        return ERR_I2C_OPEN_FAILED;
    }
	
    lByteCount = cmd_len + buffer_len;
	if (lByteCount > PAGE_SIZE)
	{
	    printf("Can't write more than %d bytes at a time.\n", PAGE_SIZE);
	    return ERR_INVALID_PARAM_VALUE;
	}
	
	if (ioctl(m_fd_i2c, I2C_SLAVE, dev_addr) < 0)
	{
		printf("%s\n", strerror(errno));
		device_i2c_close();
		return ERR_I2C_ADDRESS_FAILED;
	}

    if (cmd_len > 0)
    {
        memcpy(&io_packet.stat.Data[0], cmd, cmd_len);
    }
    
    if (buffer_len > 0)
    {
        memcpy(&io_packet.stat.Data[cmd_len], buffer, buffer_len);
    }
	
	if (write(m_fd_i2c, (char*)(&io_packet), lByteCount) < 0)
	{
	    printf("write(): %s\n", strerror(errno));
	    return ERR_I2C_WRITE_FAILED;
	}
	
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
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_i2c_close()
{
    close(m_fd_i2c);

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
    int nRc;

    if ((nRc = _lsValidHandle(handle)) != STATUS_OK)
    {
        return nRc;
    }

    __u8    mode, lsb, bits;
    __u32 speed=2500000;

    m_fd_spi = open(DEVICE_PATH_SPI,O_RDWR);
    if (m_fd_spi < 0)
    {
        printf("%s\n", strerror(errno));
        return ERR_SPI_OPEN_FAILED;
    }

    if (ioctl(m_fd_spi, SPI_IOC_RD_MODE, &mode) < 0)
    {
        return ERR_SPI_COMMAND_FAILED;
    }
    if (ioctl(m_fd_spi, SPI_IOC_RD_LSB_FIRST, &lsb) < 0)
    {
        return ERR_SPI_COMMAND_FAILED;
    }
    if (ioctl(m_fd_spi, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0) 
    {
        return ERR_SPI_COMMAND_FAILED;
    }
    if (ioctl(m_fd_spi, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) 
    {
        return ERR_SPI_COMMAND_FAILED;
    }

    printf("%s: spi mode %d, %d bits %sper word, %d Hz max\n",DEVICE_PATH_SPI, mode, bits, lsb ? "(lsb first) " : "", speed);

    //xfer[0].tx_buf = (unsigned long)buf;
    xfer[0].len = 3; /* Length of  command to write*/
    xfer[0].cs_change = 0; /* Keep CS activated */
    xfer[0].delay_usecs = 0, //delay in us
    xfer[0].speed_hz = 2500000, //speed
    xfer[0].bits_per_word = 8, // bites per word 8

    //xfer[1].rx_buf = (unsigned long) buf2;
    xfer[1].len = 4; /* Length of Data to read */
    xfer[1].cs_change = 0; /* Keep CS activated */
    xfer[0].delay_usecs = 0;
    xfer[0].speed_hz = 2500000;
    xfer[0].bits_per_word = 8;

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
    int nRc;

    if ((nRc = _lsValidHandle(handle)) != STATUS_OK)
    {
        return nRc;
    }

    int status;

    memset(buf_spi_tx, 0, sizeof buf_spi_tx);
    memset(pBuffer, 0, buffer_len);

    memcpy(&buf_spi_tx[0], &cmd[0], cmd_len);
    memcpy(&buf_spi_tx[cmd_len], &dummy[0], dummy_len);

    xfer[0].tx_buf = (unsigned long)buf_spi_tx;
    xfer[0].len = cmd_len+dummy_len; /* Length of  command to write*/
    xfer[1].rx_buf = (unsigned long) pBuffer;
    xfer[1].len = buffer_len; /* Length of Data to read */
    status = ioctl(m_fd_spi, SPI_IOC_MESSAGE(2), xfer);
    if (status < 0)
    {
        return ERR_SPI_READ_FAILED;
    }

    *pLen_transferred = buffer_len;
    //printf("env: %02x %02x %02x\n", buf[0], buf[1], buf[2]);
    //printf("ret: %02x %02x %02x %02x\n", buf2[0], buf2[1], buf2[2], buf2[3]);

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
    int nRc;

    if ((nRc = _lsValidHandle(handle)) != STATUS_OK)
    {
        return nRc;
    }

    int status;

    memset(buf_spi_tx, 0, sizeof buf_spi_tx);
    memcpy(&buf_spi_tx[0], &cmd[0], cmd_len);
    memcpy(&buf_spi_tx[cmd_len], &dummy[0], dummy_len);
    memcpy(&buf_spi_tx[cmd_len+dummy_len], buffer, buffer_len);

    xfer[0].tx_buf = (unsigned long)buf_spi_tx;
    xfer[0].len = cmd_len+dummy_len; /* Length of  command to write*/
    status = ioctl(m_fd_spi, SPI_IOC_MESSAGE(1), xfer);
    if (status < 0)
    {
        return ERR_SPI_WRITE_FAILED;
    }

    *pLen_transferred = cmd_len + dummy_len + buffer_len;
    //printf("env: %02x %02x %02x\n", buf[0], buf[1], buf[2]);
    //printf("ret: %02x %02x %02x %02x\n", buf2[0], buf2[1], buf2[2], buf2[3]);

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
*
* RETURNS:
*     0, if successful.
*     Error code < 0, otherwise.  See error codes in 'device_parallelApi_err'.
****************************************************************************************************
*/
int device_spi_close ()
{
	close(m_fd_spi);

    return STATUS_OK;
}

