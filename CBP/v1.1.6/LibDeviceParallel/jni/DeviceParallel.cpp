////////////////////////////////////////////////////////////////////////////
/*
 * This is a blank example to create libDeviceParallel shared library
 * for the Linux based Embedded System want to use IB Columbo PI scanner.
 * It may not work with your Embedded System, but it might be useful 
 */
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <getopt.h>
#include <linux/videodev2.h>
#include <linux/spi/spidev.h>

#include "util.h"

// This is a temporary data for demostration based on binary that includes device information and mask data
//#include "mask.lut"
// This is a temporary data for demostration based on binary that includes raw fingerprint image
//#include "raw_image.lut"

#include "device_parallelApi.h"
#include "device_parallelApi_def.h"
#include "device_parallelApi_err.h"
#include <android/log.h> 
#define LOG_DEBUG_TAG "IBSCAN" 
#define V4L2_CID_MXC_SWITCH_FP   0x8000007

/* Image interface */
#define DEVICE_PATH_PARALLEL "/dev/video0"          // Depends on target system (it is not fixed)
const int GPIO_LIVE_STATE = 58;
static int DEVICE_DESCRIPTOR = -1;               // File Descriptor for Finger Print sensor
const int DEFAULT_IMAGE_WIDTH = 880;
const int DEFAULT_IMAGE_HEIGHT = 1100;
#define DEFAULT_TIMEOUT_MS 5000
static int img_timeout = DEFAULT_TIMEOUT_MS;   // timeout in ms must not be less than 1 second
int sleep_timeout;

typedef struct {
    void* start;
    size_t length;
} buffer;

//typedef int DEVICE_CONTROL;

unsigned int BUFFER_COUNT;
buffer* FRAME_BUFFERS;



#define __MAX_BUFFER__      1024

/* SPI Bus */
#define SPI_MODE 0
#define SPI_BPW   8
#define SPI_DELAY 0

#define PAGE_SIZE 64
 char buf_spi_tx[1024];
struct spi_ioc_transfer xfer[2];

typedef struct _node {
    int handle;
    struct _node *next;
} NODE;

NODE *m_head, *m_tail;

static const char *DEVICE_PATH_SPI = "/dev/spidev1.0";
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

// Local Utility Functions
int _lsValidHandle(int handle)
{
//    int nDevices;
//    int nRc;
    
    if ((DEVICE_DESCRIPTOR == -1) || (handle == -1))
        return -1;

    return STATUS_OK;
}

/*Internal functions */
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

int init_mmap(int fd) {
    struct v4l2_requestbuffers req;
    CLEAR(req);
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if(-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        if(EINVAL == errno) {
            LOGE("device does not support memory mapping");
            return ERROR_LOCAL;
        } else {
            return errnoexit("VIDIOC_REQBUFS");
        }
    }

    if(req.count < 2) {
        LOGE("Insufficient buffer memory");
        return ERROR_LOCAL;
    }

    FRAME_BUFFERS = (buffer*)calloc(req.count, sizeof(*FRAME_BUFFERS));
    if(!FRAME_BUFFERS) {
        LOGE("Out of memory");
        return ERROR_LOCAL;
    }

    for(BUFFER_COUNT = 0; BUFFER_COUNT < req.count; ++BUFFER_COUNT) {
        struct v4l2_buffer buf;
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = BUFFER_COUNT;

        if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf)) {
            return errnoexit("VIDIOC_QUERYBUF");
        }

		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] buf[%d].length = %d\r\n", BUFFER_COUNT, buf.length);

        FRAME_BUFFERS[BUFFER_COUNT].length = buf.length;
        FRAME_BUFFERS[BUFFER_COUNT].start = mmap(NULL, buf.length,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

        if(MAP_FAILED == FRAME_BUFFERS[BUFFER_COUNT].start) {
            return errnoexit("mmap");
        }
    }

    return STATUS_OK;
}

int uninit_device() {
    for(unsigned int i = 0; i < BUFFER_COUNT; ++i) {
        if(-1 == munmap(FRAME_BUFFERS[i].start, FRAME_BUFFERS[i].length)) {
            return errnoexit("munmap");
        }
    }

    free(FRAME_BUFFERS);
    return STATUS_OK;
}

int init_device(int fd, int width, int height) {
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

    if(-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
        if(EINVAL == errno) {
            LOGE("not a valid V4L2 device");
            return ERROR_LOCAL;
        } else {
            return errnoexit("VIDIOC_QUERYCAP");
        }
    }

    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        LOGE("device is not a video capture device");
        return ERROR_LOCAL;
    }

    if(!(cap.capabilities & V4L2_CAP_STREAMING)) {
        LOGE("device does not support streaming i/o");
        return ERROR_LOCAL;
    }

    CLEAR(cropcap);
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if(0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//        crop.c = cropcap.defrect;
        cropcap.bounds.width = DEFAULT_IMAGE_WIDTH;
        cropcap.bounds.height = DEFAULT_IMAGE_HEIGHT;
        cropcap.bounds.left = 0;
        cropcap.bounds.top = 0;
        crop.c = cropcap.bounds;

        if(-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
            switch(errno) {
                case EINVAL:
                    break;
                default:
                    break;
            }
        }
    }

    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;

    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if(-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) {
        return errnoexit("VIDIOC_S_FMT");
    }

    min = fmt.fmt.pix.width * 2;
    if(fmt.fmt.pix.bytesperline < min) {
        fmt.fmt.pix.bytesperline = min;
    }

    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if(fmt.fmt.pix.sizeimage < min) {
        fmt.fmt.pix.sizeimage = min;
    }

    return init_mmap(fd);
}


int open_device(const char* dev_name, int* fd) {
    struct stat st;
    if(-1 == stat(dev_name, &st)) {
        LOGE("Cannot identify '%s': %d, %s", dev_name, errno, strerror(errno));
        return ERR_DEVICE_NOT_FOUND;
    }

    if(!S_ISCHR(st.st_mode)) {
        LOGE("%s is not a valid device", dev_name);
        return ERR_DEVICE_NOT_FOUND;
    }

    *fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
    if(-1 == *fd) {
        LOGE("Cannot open '%s': %d, %s", dev_name, errno, strerror(errno));
        if(EACCES == errno) {
            LOGE("Insufficient permissions on '%s': %d, %s", dev_name, errno,
                    strerror(errno));
        }
        return ERR_DEVICE_NOT_OPENED;
    }

    return STATUS_OK;
}

int start_capture(int fd) {
    unsigned int i;
    enum v4l2_buf_type type;

    for(i = 0; i < BUFFER_COUNT; ++i) {
        struct v4l2_buffer buf;
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

		// Ask to receive new frame
        if(-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
            LOGE("VIDIOC_QBUF error");

            return errnoexit("VIDIOC_QBUF");
        }
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(-1 == xioctl(fd, VIDIOC_STREAMON, &type)) {
        LOGE("Capture not started -- in error\n");
        return errnoexit("VIDIOC_STREAMON");
    }
    LOGI("Capture started");
    return STATUS_OK;
}

int read_frame(int fd, buffer* frame_buffers, int width, int height,
    int* idx, int* bytes_transferred) {
    struct v4l2_buffer buf;
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if(-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
        switch(errno) {
            case EAGAIN:
                //LOGE("DQBUF try again");
                return WARNING_LOCAL;
            case EIO:
            default:
                return errnoexit("VIDIOC_DQBUF");
        }
    }
    LOGI("Got buffer");
    assert(buf.index < BUFFER_COUNT);

	// Ask to receive new frame
    if(-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
        return errnoexit("VIDIOC_QBUF");
    }
    *idx = buf.index;
    *bytes_transferred = buf.bytesused;
    return STATUS_OK;
}

int process_camera(int fd, buffer* frame_buffers, int width,
        int height, int* idx, int* bytes_transferred) {
    int nRC;
    
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] fd(%d)\r\n", fd);
    if(fd == -1) {
        return -1;
    }
    sleep_timeout = img_timeout/1000;
    for(;;) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;

//		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "Enter select()\r\n");
        int result = select(fd + 1, &fds, NULL, NULL, &tv);
//		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "Exit select()\r\n");
        if(-1 == result) {
            if(EINTR == errno) {
				__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "In progress - EINTR occurred\r\n");
                continue;
            }
            errnoexit("select");
        } else if(0 == result) {
            LOGE("select timeout");
            return ERROR_LOCAL;
        }
/*
        sleep(1);
        if(sleep_timeout == 0) {
           
            LOGE("Read Frame timedout in dbg use error for production -- resetting sleep timeout");
            sleep_timeout = img_timeout/1000;
        }
        sleep(1);
        sleep_timeout--;
*/

//        usleep(250000);
//		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI]Enter read_frame()\r\n");
        if((nRC = read_frame(fd, frame_buffers, width, height, idx, bytes_transferred)) == STATUS_OK) {
            break;
        }
        
            if (nRC == ERROR_LOCAL)
               return ERR_FILE_READ;
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_control_init(%d)\r\n", index);
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_control_init(%d)\r\n", STATUS_OK);
    _init_list();
    DEVICE_DESCRIPTOR = -1;
    sleep_timeout = 0;
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_control_close(%d)\r\n", index);
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_control_close(%d)\r\n", STATUS_OK);
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
//	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_control_reg_read(%d - %d)\r\n", index, (int)eCtrl);
    int pin;
    int nRc;

    *pState = 0;
    if (index == 0)
    {
        switch(eCtrl)
        {
          case LIVE_STATE:
            pin = GPIO_LIVE_STATE; break;
          default:
			__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_control_reg_read(ERR_INVALID_PARAM_VALUE)\r\n");
            return ERR_INVALID_PARAM_VALUE;
    	}

   		nRc = _gpioRead(pin);

        if (-1 == nRc)
        {
			__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_control_reg_read(ERR_GPIO_READ_FAILED)\r\n");
            return ERR_GPIO_READ_FAILED;
        }
        
//		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Read GPIO = %d\r\n", nRc);
        *pState = 1;
    }
    else
    {
        /* Your embedded system does not a plen to support multiple IB scanners at the same time */
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_control_reg_read(ERR_INVALID_PARAM_VALUE)\r\n");
        return ERR_INVALID_PARAM_VALUE;
    }

//	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_control_reg_read(%d)\r\n", STATUS_OK);
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_control_reg_write(%d - %d - %d)\r\n", index, (int)eCtrl, value);
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_control_reg_write(%d)\r\n", STATUS_OK);
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
//	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_count()\r\n");
    unsigned char pState;
    int nRC;

    *pDeviceCount = 0;
    if(DEVICE_DESCRIPTOR == -1)
    {
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter open_device(%s)\r\n", DEVICE_PATH_PARALLEL);
      if ((nRC = open_device(DEVICE_PATH_PARALLEL, &DEVICE_DESCRIPTOR)) != STATUS_OK)
		{
			__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_count(%d)\r\n", nRC);
        return nRC;
		}
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit open_device(%s - %d)\r\n", DEVICE_PATH_PARALLEL, DEVICE_DESCRIPTOR);
    }  
    
//    *pDeviceCount = device_control_reg_read (0, LIVE_STATE, &pState);
    if ((nRC = device_control_reg_read (0, LIVE_STATE, &pState)) != STATUS_OK)
	{
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_count(%d)\r\n", nRC);
    	return nRC;
    }

  	*pDeviceCount = pState;
//    DEVICE_DESCRIPTOR = pState;


//	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_count(STATUS_OK - %d)\r\n", pState);
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_init()\r\n");
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_init()\r\n");
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_open()\r\n");
  int nDevices;
    int nRc;
    
    *pHandle = -1;
    if ((nRc = device_count(&nDevices)) != STATUS_OK)
    {
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_open(%d)\r\n", nRc);
        return nRc;
    }

    if (nDevices == 0)
    {
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_open(ERR_DEVICE_NOT_FOUND)\r\n");
        return ERR_DEVICE_NOT_FOUND;
    }

    *pHandle = _createHandle();
	
    
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_open(STATUS_OK)\r\n");
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_close(%d)\r\n", handle);

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int result = STATUS_OK;
    
    if (-1 == DEVICE_DESCRIPTOR)
    {
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_close(ERR_DEVICE_NOT_OPENED)\r\n");
        return ERR_DEVICE_NOT_OPENED;
    }
    
    if(-1 != DEVICE_DESCRIPTOR && -1 == xioctl(DEVICE_DESCRIPTOR, VIDIOC_STREAMOFF, &type))
    {
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_close(VIDIOC_STREAMOFF)\r\n");
        return errnoexit("VIDIOC_STREAMOFF");
    }
    
    for(unsigned int i = 0; i < BUFFER_COUNT; ++i)
    {
        if(-1 == munmap(FRAME_BUFFERS[i].start, FRAME_BUFFERS[i].length))
        {
			__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_close(munmap)\r\n");
            return errnoexit("munmap");
        }
    }
    
    free(FRAME_BUFFERS);
    
    if(-1 != DEVICE_DESCRIPTOR && -1 == close(DEVICE_DESCRIPTOR))
    {
        
        result = errnoexit("close");
    }
    
    DEVICE_DESCRIPTOR = -1;

	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_close(%d)\r\n", result);
	
    return result;
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_parallel_init(%d)\r\n", handle);
  if(DEVICE_DESCRIPTOR == -1 || (_lsValidHandle(handle) != STATUS_OK))
  {
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_parallel_init(ERR_DEVICE_NOT_OPENED)\r\n");
    return ERR_DEVICE_NOT_OPENED;
  }

	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] init_device(%d - %d - %d)\r\n", DEVICE_DESCRIPTOR, DEFAULT_IMAGE_WIDTH, DEFAULT_IMAGE_HEIGHT);
  if(init_device(DEVICE_DESCRIPTOR, DEFAULT_IMAGE_WIDTH, DEFAULT_IMAGE_HEIGHT) == ERROR_LOCAL)
  {
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_parallel_init(ERR_NOT_INITIALIZED)\r\n");
    return ERR_NOT_INITIALIZED;
  }

	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] start_capture(%d)\r\n", DEVICE_DESCRIPTOR);
  if (start_capture(DEVICE_DESCRIPTOR) != STATUS_OK)
  {
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_parallel_init(ERR_NOT_INITIALIZED)\r\n");
    return ERR_NOT_INITIALIZED;
  }
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_parallel_init(STATUS_OK)\r\n");
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_parallel_bulk_read(%d - %d - %d)\r\n", handle, buffer_len, timeout);
  int nRC;
  static int idx;
  static int bytes_transferred;
  if(DEVICE_DESCRIPTOR == -1 || (_lsValidHandle(handle) != STATUS_OK))
  {
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_parallel_bulk_read(ERR_DEVICE_NOT_OPENED)\r\n");
    return ERR_DEVICE_NOT_OPENED;
  }
  /* wrw test */
    struct v4l2_control control;
    control.id = 1010;

    __android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "Reading from alternate %x is val\n", control.value);

    if (0 == ioctl (DEVICE_DESCRIPTOR, VIDIOC_G_CTRL, &control))
    {
        __android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "Reading from alternate %x is val\n", control.value);

    } else
        __android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "Error Reading from alternate");

    control.id = 1010;
    control.value = 0x00E0;
    if (0 == ioctl (DEVICE_DESCRIPTOR, VIDIOC_S_CTRL, &control))
    {
        __android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "Writing I2c success alt");

    } else
        __android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "Error Writing from alternate");

    /* end wrw test */
  nRC = process_camera(DEVICE_DESCRIPTOR, FRAME_BUFFERS, DEFAULT_IMAGE_WIDTH, DEFAULT_IMAGE_HEIGHT, &idx, &bytes_transferred);
  if(nRC != STATUS_OK) 
  {
    idx = -1;
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_parallel_bulk_read(%d)\r\n", nRC);
    return nRC;

  }
    

	unsigned char *ptr = (unsigned char*)FRAME_BUFFERS[idx].start;
	memcpy(pBuffer, ptr, bytes_transferred);
	*pLen_transferred = bytes_transferred;
  idx = -1;

	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_parallel_bulk_read(STATUS_OK - %d)\r\n", *pLen_transferred);
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_parallel_close(%d)\r\n", handle);

    if(DEVICE_DESCRIPTOR == -1 || (_lsValidHandle(handle) != STATUS_OK))
    {
        __android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_parallel_close(ERR_DEVICE_NOT_OPENED)\r\n");
        return ERR_DEVICE_NOT_OPENED;
    }
    
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(DEVICE_DESCRIPTOR, VIDIOC_STREAMOFF, &type)){
        LOGE("VIDIOC_STREAMOFF");
        return ERROR_LOCAL;
	}

	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_parallel_close(STATUS_OK)\r\n");
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_i2c_init(%d)\r\n", handle);
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_i2c_init(STATUS_OK)\r\n");
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_i2c_read(handle:%d, I2C slave:0x%02X, address:0x%02X)\r\n", handle, dev_addr, cmd[0]);
     
//     YOU NEED TO UNCOMMENT IF YOU ARE EXPECTING TO READ DEVICE
     struct v4l2_control control;
     control.id = *cmd;
  if(DEVICE_DESCRIPTOR == -1)
  {
         __android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_i2c_read(ERR_DEVICE_NOT_OPENED)\r\n");
    return ERR_DEVICE_NOT_OPENED;
  }
     if(dev_addr == 0xC0) {
         control.id = V4L2_CID_MXC_SWITCH_FP;
         control.value = *cmd;
     }
  if (0 == ioctl (DEVICE_DESCRIPTOR, VIDIOC_G_CTRL, &control)) 
  {
        pBuffer[0] = control.value;
         __android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_i2c_read addr :ad%x val: %x(STATUS_OK)\r\n", dev_addr, control.value);
        return STATUS_OK;

  }
     
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_i2c_read(ERR_I2C_READ_FAILED)\r\n");
   return ERR_I2C_READ_FAILED;
	
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_i2c_write(handle:%d, I2C slave:0x%02X, address:0x%02X, value:0x%02X)\r\n", handle, dev_addr, cmd[0], buffer[0]);

//     YOU NEED TO UNCOMMENT THIS IF YOU WISH TO READ
     struct v4l2_control control;

  memset (&control, 0, sizeof (control));
     control.id = *cmd;
  control.value = buffer[0];
  if(DEVICE_DESCRIPTOR == -1)
  {
         __android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_i2c_write(ERR_DEVICE_NOT_OPENED)\r\n");
    return ERR_DEVICE_NOT_OPENED;
  }
     if(dev_addr == 0xC0) {
       control.id = V4L2_CID_MXC_SWITCH_FP;
       unsigned int tmp = *cmd;
       tmp = tmp << 8;
       control.value |= tmp;
     }
  if (-1 == ioctl (DEVICE_DESCRIPTOR, VIDIOC_S_CTRL, &control)) 
  {
        LOGE("ERROR VIDIOC_S_CTRL -- could not write value %x", control.value);
        return ERR_I2C_WRITE_FAILED; 
  }

	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_i2c_write(STATUS_OK)\r\n");
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_i2c_close(%d)\r\n", handle);
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_i2c_close(STATUS_OK)\r\n");
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_spi_init(%d)\r\n", handle);
    int nRc;

    if ((nRc = _lsValidHandle(handle)) != STATUS_OK)
    {
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_init(%d)\r\n", nRc);
        return nRc;
    }

    __u8    mode, lsb, bits;
    __u32 speed=2500000;

    m_fd_spi = open(DEVICE_PATH_SPI,O_RDWR);
    if (m_fd_spi < 0)
    {
        printf("%s\n", strerror(errno));
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_init(ERR_SPI_OPEN_FAILED)\r\n");
        return ERR_SPI_OPEN_FAILED;
    }

    if (ioctl(m_fd_spi, SPI_IOC_RD_MODE, &mode) < 0)
    {
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_init(ERR_SPI_COMMAND_FAILED)\r\n");
        return ERR_SPI_COMMAND_FAILED;
    }
    if (ioctl(m_fd_spi, SPI_IOC_RD_LSB_FIRST, &lsb) < 0)
    {
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_init(ERR_SPI_COMMAND_FAILED)\r\n");
        return ERR_SPI_COMMAND_FAILED;
    }
    if (ioctl(m_fd_spi, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0) 
    {
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_init(ERR_SPI_COMMAND_FAILED)\r\n");
        return ERR_SPI_COMMAND_FAILED;
    }
    if (ioctl(m_fd_spi, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) 
    {
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_init(ERR_SPI_COMMAND_FAILED)\r\n");
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

	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_init(STATUS_OK)\r\n");
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
	int pageNum = (cmd[1]<<6) + (cmd[2]>>2);
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_spi_read(%d, 0x%02X, page : %d)\r\n", handle, cmd[0], pageNum);
    if (handle < 0) {
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_read(-1)\r\n");
        return -1;
    }

/*
// It is temporary to read Columbo property information
	if (cmd[0] == 0xE8 && cmd[1]== 0x00 && cmd[2] == 0x00)
	{
		memcpy(pBuffer, &mask[pageNum*buffer_len], buffer_len);
		*pLen_transferred = buffer_len;
		
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_read(STATUS_OK)\r\n");
		return STATUS_OK;
	}
*/
    int status;
	unsigned char command[16]={0};
//	pageNum = pageNum -1;
	command[0] = 0xE8;
	command[1] = (pageNum>>6)&0x3F;
	command[2] = ((pageNum&0x3F)<<2);
	command[3] = 0;

    memset(buf_spi_tx, 0, sizeof buf_spi_tx);
    memset(pBuffer, 0, buffer_len);

//    memcpy(&buf_spi_tx[0], &cmd[0], cmd_len);
    memcpy(&buf_spi_tx[0], &command[0], cmd_len);
    memcpy(&buf_spi_tx[cmd_len], &dummy[0], dummy_len);

    xfer[0].tx_buf = (unsigned long)buf_spi_tx;
    xfer[0].len = cmd_len+dummy_len; /* Length of  command to write*/
    xfer[1].rx_buf = (unsigned long) pBuffer;
    xfer[1].len = buffer_len; /* Length of Data to read */
    status = ioctl(m_fd_spi, SPI_IOC_MESSAGE(2), xfer);
    if (status < 0)
    {
        LOGE(" ERR_SPI_READ_FAILED");
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_read(ERR_SPI_READ_FAILED)\r\n");
        return ERR_SPI_READ_FAILED;
    }

    *pLen_transferred = buffer_len;
    //printf("env: %02x %02x %02x\n", buf[0], buf[1], buf[2]);
    //printf("ret: %02x %02x %02x %02x\n", buf2[0], buf2[1], buf2[2], buf2[3]);

	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_read(STATUS_OK)\r\n");
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_spi_write(%d)\r\n", handle);
    if (handle != 1) {
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_write(-1)\r\n");
        return -1;
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
		__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_write(ERR_SPI_WRITE_FAILED)\r\n");
        return ERR_SPI_WRITE_FAILED;
    }

    *pLen_transferred = cmd_len + dummy_len + buffer_len;
    //printf("env: %02x %02x %02x\n", buf[0], buf[1], buf[2]);
    //printf("ret: %02x %02x %02x %02x\n", buf2[0], buf2[1], buf2[2], buf2[3]);

	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_write(STATUS_OK)\r\n");
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
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Enter device_spi_close(%d)\r\n", handle);
     close(m_fd_spi);
	__android_log_print(ANDROID_LOG_INFO, LOG_DEBUG_TAG, "[PPI] Exit device_spi_close(STATUS_OK)\r\n");
    return STATUS_OK;
}

