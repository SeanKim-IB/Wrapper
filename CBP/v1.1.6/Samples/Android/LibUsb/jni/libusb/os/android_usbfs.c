/*
 * Android backend for libusb
 * Copyright (C) 2007-2008 Daniel Drake <dsd@gentoo.org>
 * Copyright (c) 2001 Johannes Erdfelt <johannes@erdfelt.com>
 *
 * This backend was adapted from the Linux usbfs backend.  On rooted devices,
 * the Linux usbfs can be used.  The following changes were made:
 *
 *     - the interface structure was renamed
 *
 *     - interface functions op_get_device_list() now calls up to a
 *       Java function in the LibUsbManager class to get a list of
 *       devices accessible for the caller
 *
 *     - interface function op_open() and op_close() now call up to
 *       Java functions in the LibUsbManager class to open and close a
 *       connection to the device
 *
 *     - interface functions op_destroy_device(), op_set_configuration(),
 *       op_get_configuration(), op_get_config_descriptor(), and
 *       op_get_device_descriptor() and various called functions were
 *       modified, as appropriate
 *
 * The corresponding LibUsbManager Java class must be compiled and
 * included as a library in your app.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <jni.h>

#include "libusb.h"
#include "libusbi.h"
#include "android_usbfs.h"

/* Android:
 * An app must seek permission before accessing any USB device, for
 * which a dialog will typically be shown to the user, who can allow
 * or deny, with a callback to the app with the choice.  Any access
 * is proscribed, including to descriptors, without permission.  This
 * backend with its matching Java class provide to libusb_get_device_list()
 * only those devices to which the caller has access (as determined by
 * the UsbManager).  The app is responsible for registering for and
 * processing broadcast messages for device attachment and requesting
 * permission so that attached desirable devices appear in this list
 * and can be used with ilbusb.
 */

struct android_device_desc {
	int device_id;
	unsigned char *descriptors;
	int descriptors_length;
};

/* Path of Java class. */
#define ANDROID_CLASS_PATH "org/libusb/LibUsbManager"

/* Saved for calling Java methods and handling Java objects. */
static JavaVM *saved_java_vm = 0;
static jclass class_libusbmanager = 0;
static jmethodID methodid_get_device_array = 0;
static jmethodID methodid_open_device = 0;
static jmethodID methodid_close_device = 0;
static jclass class_devdesc = 0;
static jfieldID fieldid_device_id = 0;
static jfieldID fieldid_descriptors = 0;
static int android_init_done = 0;

static jclass find_class(JNIEnv *env, const char *name, int *r)
{
    jclass cclass = 0;

    if (*r == 0) {
		cclass = (*env)->FindClass(env, name);
		if (!cclass) {
			*r = LIBUSB_ERROR_IO;
			usbi_dbg("failed to find class name=%s", name);
		} else {
			cclass = (*env)->NewGlobalRef(env, cclass);
		}
    }
    return cclass;
}

static jfieldID get_field_id(JNIEnv *env, jclass cclass, const char *name,
		const char *sig, int *r)
{
    jfieldID field_id = 0;

	if (*r == 0) {
    	field_id = (*env)->GetFieldID(env, cclass, name, sig);
		if (!field_id) {
			*r = LIBUSB_ERROR_IO;
			usbi_dbg("failed to get field ID name=%s sig=%s", name, sig);
		}
    }
    return field_id;
}

static jmethodID get_static_method_id(JNIEnv *env, jclass cclass, const char *name,
		const char *sig, int *r)
{
    jmethodID method_id = 0;

	if (*r == 0) {
    	method_id = (*env)->GetStaticMethodID(env, cclass, name, sig);
		if (!method_id) {
			*r = LIBUSB_ERROR_IO;
			usbi_dbg("failed to get method ID name=%s sig=%s", name, sig);
		}
    }
    return method_id;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
	jint res;
	JNIEnv *env;
	int r = 0;

	/* sava pointer to VM for callbacks into Java */
	saved_java_vm = vm;

	res = (*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_2);
	if (res < 0)
		return JNI_VERSION_1_2;
	class_libusbmanager = find_class(env, ANDROID_CLASS_PATH, &r);
	if (class_libusbmanager)
	{
		methodid_get_device_array = get_static_method_id(env, class_libusbmanager, "getDeviceArray", "()[L" ANDROID_CLASS_PATH "$DeviceDesc;", &r);
		methodid_open_device = get_static_method_id(env, class_libusbmanager, "openDevice", "(I)I", &r);
		methodid_close_device = get_static_method_id(env, class_libusbmanager, "closeDevice", "(I)V", &r);
	}
	class_devdesc = find_class(env, ANDROID_CLASS_PATH "$DeviceDesc", &r);
	if (class_devdesc)
	{
		fieldid_device_id = get_field_id(env, class_devdesc, "deviceId", "I", &r);
		fieldid_descriptors = get_field_id(env, class_devdesc, "descriptors", "[B", &r);
	}

	/* block execution of calls into Java if this fails */
	if (!r)
		android_init_done = 1;

	return JNI_VERSION_1_2;
}

static jint attach_to_vm(void **env, int *attached)
{
	jint rc;

	rc = (*saved_java_vm)->GetEnv(saved_java_vm, env, JNI_VERSION_1_2);
	if (rc == 0) {
		*attached = 1;
	} else {
	    rc = (*saved_java_vm)->AttachCurrentThread(saved_java_vm, env, NULL);
		*attached = 0;
	}

	return rc;
}

static void detach_from_vm(int attached)
{
	if (!attached)
		(*saved_java_vm)->DetachCurrentThread(saved_java_vm);
}

static struct android_device_desc *android_get_device_array(int *desc_count)
{
	JNIEnv *env = 0;
	int attached = 0;
	jint r;
	struct android_device_desc *array = 0;
	int count = 0;

	/* make sure init has completed */
	if (!android_init_done)
		return 0;

	r = attach_to_vm((void **)&env, &attached);
	if (r) {
		usbi_dbg("failed to get attach to VM");
	} else {
		jobjectArray array_java = (*env)->CallStaticObjectMethod(env, class_libusbmanager, methodid_get_device_array);

		if (array_java) {
			int count_temp;
			struct android_device_desc *array_temp;

			count_temp = (*env)->GetArrayLength(env, array_java);
			array_temp = malloc(sizeof(struct android_device_desc) * count_temp);
			if (array_temp) {
				int r = 0;
				int i;

				memset(array_temp, 0, sizeof(struct android_device_desc) * count_temp);
				for (i = 0;i < count_temp; i++) {
					jobject desc_java;
					jobject descriptors_java;

					desc_java = (*env)->GetObjectArrayElement(env, array_java, i);
					if (!desc_java) {
						usbi_dbg("failed to get element from desc array");
						r = 1;
						break;
					}
					array_temp[i].device_id = (*env)->GetIntField(env, desc_java, fieldid_device_id);
					descriptors_java = (*env)->GetObjectField(env, desc_java, fieldid_descriptors);
					if (!descriptors_java) {
						usbi_dbg("failed to descriptor field");
						r = 1;
						break;
					}
					array_temp[i].descriptors_length = (*env)->GetArrayLength(env, descriptors_java);
					array_temp[i].descriptors = malloc(array_temp[i].descriptors_length);
					if (!array_temp[i].descriptors) {
						usbi_dbg("failed to malloc room for copy of descriptors");
						r = 1;
						break;
					}
					(*env)->GetByteArrayRegion(env, descriptors_java, 0,
							array_temp[i].descriptors_length, (jbyte *)array_temp[i].descriptors);
					(*env)->DeleteLocalRef(env, descriptors_java);
					(*env)->DeleteLocalRef(env, desc_java);
				}
				(*env)->DeleteLocalRef(env, array_java);

				if (r) {
					for (i = 0; i < count; i++) {
						if (array_temp[i].descriptors)
							free(array_temp[i].descriptors);
					}
					free(array_temp);
				} else {
					count = count_temp;
					array = array_temp;
				}
			} else {
				usbi_dbg("failed to malloc room for device descriptions");
			}
		} else {
			usbi_dbg("null device array returned from java function");
		}

		detach_from_vm(attached);
	}

	*desc_count = count;
	return array;
}

static void android_free_device_array(struct android_device_desc *desc_array, int desc_count)
{
	/* make sure init has completed */
	if (!android_init_done)
		return;

	if (desc_array) {
		int i;

		for (i = 0; i < desc_count; i++) {
			if (desc_array[i].descriptors)
				free(desc_array[i].descriptors);
		}
		free(desc_array);
	}
}

static int android_open(int device_id)
{
	JNIEnv *env = 0;
	int attached = 0;
	jint r;
	int fd = -1;

	/* make sure init has completed */
	if (!android_init_done)
		return -1;

	r = attach_to_vm((void **)&env, &attached);
	if (r) {
		usbi_dbg("failed to get attach to VM");
    } else {
		fd = (*env)->CallStaticIntMethod(env, class_libusbmanager, methodid_open_device, device_id);
		detach_from_vm(attached);
	}
	return fd;
}

static void android_close(int device_id)
{
	JNIEnv *env = 0;
	int attached = 0;
	jint r;

	if (!android_init_done)
		return;

	r = attach_to_vm((void **)&env, &attached);
	if (r) {
		usbi_dbg("failed to get attach to VM");
	} else {
		(*env)->CallStaticVoidMethod(env, class_libusbmanager, methodid_close_device, device_id);
		detach_from_vm(attached);
	}
}

struct linux_device_priv {
	unsigned char *dev_descriptor;
	unsigned char *config_descriptor;
	size_t config_descriptor_length;
	int active_config;
	int device_id;
};

struct linux_device_handle_priv {
	int fd;
};

enum reap_action {
	NORMAL = 0,
	/* submission failed after the first URB, so await cancellation/completion
	 * of all the others */
	SUBMIT_FAILED,

	/* cancelled by user or timeout */
	CANCELLED,

	/* completed multi-URB transfer in non-final URB */
	COMPLETED_EARLY,
};

struct linux_transfer_priv {
	union {
		struct usbfs_urb *urbs;
		struct usbfs_urb **iso_urbs;
	};

	enum reap_action reap_action;
	int num_urbs;
	unsigned int awaiting_reap;
	unsigned int awaiting_discard;

	/* next iso packet in user-supplied transfer to be populated */
	int iso_packet_offset;
};

static struct linux_device_priv *__device_priv(struct libusb_device *dev)
{
	return (struct linux_device_priv *) dev->os_priv;
}

static struct linux_device_handle_priv *__device_handle_priv(
	struct libusb_device_handle *handle)
{
	return (struct linux_device_handle_priv *) handle->os_priv;
}

static int op_init(struct libusb_context *ctx)
{
	return 0;
}

static int op_get_device_descriptor(struct libusb_device *dev,
	unsigned char *buffer, int *host_endian)
{
	struct linux_device_priv *priv = __device_priv(dev);

	/* return cached copy */
	*host_endian = 1;
	memcpy(buffer, priv->dev_descriptor, DEVICE_DESC_LENGTH);
	return 0;
}

/* takes a usbfs/descriptors fd seeked to the start of a configuration, and
 * seeks to the next one. */
static int seek_to_next_config(struct libusb_context *ctx, unsigned char **config_desc, size_t *config_desc_length)
{
	struct libusb_config_descriptor config;
	unsigned char tmp[6];

	/* read first 6 bytes of descriptor */
	if (*config_desc_length < 6) {
		usbi_err(ctx, "length too small %d < 6", *config_desc_length);
		return LIBUSB_ERROR_IO;
	}
	memcpy(tmp, *config_desc, sizeof(tmp));

	/* seek forward to end of config */
	usbi_parse_descriptor(tmp, "bbwbb", &config, 1);
	if (config.wTotalLength > *config_desc_length) {
		usbi_err(ctx, "length too small %d < %d", *config_desc_length, config.wTotalLength);
		return LIBUSB_ERROR_IO;
	}
	if (config.wTotalLength < 6) {
		usbi_err(ctx, "bad value %d < 6", config.wTotalLength);
		return LIBUSB_ERROR_IO;
	}

	*config_desc = *config_desc + 6;
	*config_desc_length = *config_desc_length - 6;

	return 0;
}

static int op_get_config_descriptor(struct libusb_device *dev,
	uint8_t config_index, unsigned char *buffer, size_t len, int *host_endian)
{
	struct linux_device_priv *priv = __device_priv(dev);
	unsigned char *config_desc;
	size_t config_desc_length;

	config_desc = priv->config_descriptor;
	config_desc_length = priv->config_descriptor_length;

	/* might need to skip some configuration descriptors to reach the
	 * requested configuration */
	while (config_index > 0) {
		int r = seek_to_next_config(DEVICE_CTX(dev), &config_desc, &config_desc_length);
		if (r < 0)
			return r;
		config_index--;
	}

	/* copy to buffer. */
	if (len > config_desc_length)
		return LIBUSB_ERROR_IO;
	memcpy(buffer, config_desc, len);

	*host_endian = 1;
	return 0;
}

static int op_get_active_config_descriptor(struct libusb_device *dev,
	unsigned char *buffer, size_t len, int *host_endian)
{

	struct linux_device_priv *priv = __device_priv(dev);

	return op_get_config_descriptor(dev, priv->active_config, buffer, len, host_endian);
}

static int initialize_device(struct libusb_device *dev, struct android_device_desc *adesc)
{
	struct linux_device_priv *priv = __device_priv(dev);
	unsigned char *dev_buf;
	unsigned char *config_buf;

	/* always use bus 1; just use device ID as device address */
	/* enzyme update - device_id has 2byte, so we need to use bus_number and device_address. */
	dev->bus_number = (adesc->device_id/1000);
	dev->device_address = (adesc->device_id%1000);
	priv->dev_descriptor = NULL;
	priv->config_descriptor = NULL;

	dev_buf = malloc(DEVICE_DESC_LENGTH);
	if (!dev_buf) {
		return LIBUSB_ERROR_NO_MEM;
	}
	memcpy(dev_buf, adesc->descriptors, DEVICE_DESC_LENGTH);

	config_buf = malloc(adesc->descriptors_length - DEVICE_DESC_LENGTH);
	if (!config_buf) {
		free(dev_buf);
		return LIBUSB_ERROR_NO_MEM;
	}
	memcpy(config_buf, adesc->descriptors + DEVICE_DESC_LENGTH, adesc->descriptors_length - DEVICE_DESC_LENGTH);

	priv->device_id = adesc->device_id;
	priv->dev_descriptor = dev_buf;
	priv->config_descriptor = config_buf;
	priv->config_descriptor_length = adesc->descriptors_length - DEVICE_DESC_LENGTH;
	priv->active_config = 0; /* do not know */

	/* bit of a hack: set num_configurations now because cache_active_config()
	 * calls usbi_get_config_index_by_value() which uses it */
	dev->num_configurations = dev_buf[DEVICE_DESC_LENGTH - 1];

	return 0;
}

static int op_get_device_list(struct libusb_context *ctx,
	struct discovered_devs **_discdevs)
{
	struct android_device_desc *desc_array;
	int desc_cnt = 0;
	int i;

	desc_array = android_get_device_array(&desc_cnt);
	if (!desc_array) {
		usbi_err(ctx, "get device list failed");
		return LIBUSB_ERROR_IO;
	}
	for (i = 0; i < desc_cnt; i++) {
		struct discovered_devs *discdevs;
		unsigned long session_id;
		int need_unref = 0;
		struct libusb_device *dev;
		int r = 0;

		session_id = desc_array[i].device_id;
		usbi_dbg("device id %d", desc_array[i].device_id);
        
        // Bug fixed in Android by YoungKim (enzyme)
        // Android has same session id even if USB device is changed at same bus and port
/*		dev = usbi_get_device_by_session_id(ctx, session_id);
		if (dev) {
			usbi_dbg("using existing device for %d (session %ld)",
				desc_array[i].device_id, session_id);
			r = 0;
		} else {
*/
			usbi_dbg("allocating new device for %d (session %ld)",
				desc_array[i].device_id, session_id);
			dev = usbi_alloc_device(ctx, session_id);
			if (!dev)
				return LIBUSB_ERROR_NO_MEM;
			need_unref = 1;
			r = initialize_device(dev, &desc_array[i]);
			if (r >= 0)
				r = usbi_sanitize_device(dev);
//		}

		if (r >= 0) {
			discdevs = discovered_devs_append(*_discdevs, dev);
			if (!discdevs)
				r = LIBUSB_ERROR_NO_MEM;
			else
				*_discdevs = discdevs;
		}

		if (need_unref)
			libusb_unref_device(dev);
	}

	android_free_device_array(desc_array, desc_cnt);

	return 0;
}

static int op_open(struct libusb_device_handle *handle)
{
	struct linux_device_handle_priv *hpriv = __device_handle_priv(handle);
	struct linux_device_priv *priv = __device_priv(handle->dev);

	hpriv->fd = android_open(priv->device_id);
	if (hpriv->fd < 0) {
		usbi_err(HANDLE_CTX(handle), "open failed");
		return LIBUSB_ERROR_IO;
	}

	return usbi_add_pollfd(HANDLE_CTX(handle), hpriv->fd, POLLOUT);
}

static void op_close(struct libusb_device_handle *handle)
{
	struct linux_device_handle_priv *hpriv = __device_handle_priv(handle);
	struct linux_device_priv *priv = __device_priv(handle->dev);
	int fd = hpriv->fd;
	int device_id = priv->device_id;
	usbi_remove_pollfd(HANDLE_CTX(handle), fd);
	android_close(device_id);
}

static int op_get_configuration(struct libusb_device_handle *handle,
	int *config)
{
	struct linux_device_priv *priv = __device_priv(handle->dev);

	*config = priv->active_config;
	return 0;
}

static int op_set_configuration(struct libusb_device_handle *handle, int config)
{
	struct linux_device_priv *priv = __device_priv(handle->dev);
	int fd = __device_handle_priv(handle)->fd;
	int r = ioctl(fd, IOCTL_USBFS_SETCONFIG, &config);
	if (r) {
		if (errno == EINVAL)
			return LIBUSB_ERROR_NOT_FOUND;
		else if (errno == EBUSY)
			return LIBUSB_ERROR_BUSY;
		else if (errno == ENODEV)
			return LIBUSB_ERROR_NO_DEVICE;

		usbi_err(HANDLE_CTX(handle), "failed, error %d errno %d", r, errno);
		return LIBUSB_ERROR_OTHER;
	}
	priv->active_config = config;
	return 0;
}

static int op_claim_interface(struct libusb_device_handle *handle, int iface)
{
	int fd = __device_handle_priv(handle)->fd;
	int r = ioctl(fd, IOCTL_USBFS_CLAIMINTF, &iface);
	if (r) {
		if (errno == ENOENT)
			return LIBUSB_ERROR_NOT_FOUND;
		else if (errno == EBUSY)
			return LIBUSB_ERROR_BUSY;
		else if (errno == ENODEV)
			return LIBUSB_ERROR_NO_DEVICE;

		usbi_err(HANDLE_CTX(handle),
			"claim interface failed, error %d errno %d", r, errno);
		return LIBUSB_ERROR_OTHER;
	}
	return 0;
}

static int op_release_interface(struct libusb_device_handle *handle, int iface)
{
	int fd = __device_handle_priv(handle)->fd;
	int r = ioctl(fd, IOCTL_USBFS_RELEASEINTF, &iface);
	if (r) {
		if (errno == ENODEV)
			return LIBUSB_ERROR_NO_DEVICE;

		usbi_err(HANDLE_CTX(handle),
			"release interface failed, error %d errno %d", r, errno);
		return LIBUSB_ERROR_OTHER;
	}
	return 0;
}

static int op_set_interface(struct libusb_device_handle *handle, int iface,
	int altsetting)
{
	int fd = __device_handle_priv(handle)->fd;
	struct usbfs_setinterface setintf;
	int r;

	setintf.interface = iface;
	setintf.altsetting = altsetting;
	r = ioctl(fd, IOCTL_USBFS_SETINTF, &setintf);
	if (r) {
		if (errno == EINVAL)
			return LIBUSB_ERROR_NOT_FOUND;
		else if (errno == ENODEV)
			return LIBUSB_ERROR_NO_DEVICE;

		usbi_err(HANDLE_CTX(handle),
			"setintf failed error %d errno %d", r, errno);
		return LIBUSB_ERROR_OTHER;
	}

	return 0;
}

static int op_clear_halt(struct libusb_device_handle *handle,
	unsigned char endpoint)
{
	int fd = __device_handle_priv(handle)->fd;
	unsigned int _endpoint = endpoint;
	int r = ioctl(fd, IOCTL_USBFS_CLEAR_HALT, &_endpoint);
	if (r) {
		if (errno == ENOENT)
			return LIBUSB_ERROR_NOT_FOUND;
		else if (errno == ENODEV)
			return LIBUSB_ERROR_NO_DEVICE;

		usbi_err(HANDLE_CTX(handle),
			"clear_halt failed error %d errno %d", r, errno);
		return LIBUSB_ERROR_OTHER;
	}

	return 0;
}

static int op_reset_device(struct libusb_device_handle *handle)
{
	int fd = __device_handle_priv(handle)->fd;
	int r = ioctl(fd, IOCTL_USBFS_RESET, NULL);
	if (r) {
		if (errno == ENODEV)
			return LIBUSB_ERROR_NOT_FOUND;

		usbi_err(HANDLE_CTX(handle),
			"reset failed error %d errno %d", r, errno);
		return LIBUSB_ERROR_OTHER;
	}

	return 0;
}

static int op_kernel_driver_active(struct libusb_device_handle *handle,
	int interface)
{
	int fd = __device_handle_priv(handle)->fd;
	struct usbfs_getdriver getdrv;
	int r;

	getdrv.interface = interface;
	r = ioctl(fd, IOCTL_USBFS_GETDRIVER, &getdrv);
	if (r) {
		if (errno == ENODATA)
			return 0;
		else if (errno == ENODEV)
			return LIBUSB_ERROR_NO_DEVICE;

		usbi_err(HANDLE_CTX(handle),
			"get driver failed error %d errno %d", r, errno);
		return LIBUSB_ERROR_OTHER;
	}

	return 1;
}

static int op_detach_kernel_driver(struct libusb_device_handle *handle,
	int interface)
{
	int fd = __device_handle_priv(handle)->fd;
	struct usbfs_ioctl command;
	int r;

	command.ifno = interface;
	command.ioctl_code = IOCTL_USBFS_DISCONNECT;
	command.data = NULL;

	r = ioctl(fd, IOCTL_USBFS_IOCTL, &command);
	if (r) {
		if (errno == ENODATA)
			return LIBUSB_ERROR_NOT_FOUND;
		else if (errno == EINVAL)
			return LIBUSB_ERROR_INVALID_PARAM;
		else if (errno == ENODEV)
			return LIBUSB_ERROR_NO_DEVICE;

		usbi_err(HANDLE_CTX(handle),
			"detach failed error %d errno %d", r, errno);
		return LIBUSB_ERROR_OTHER;
	}

	return 0;
}

static int op_attach_kernel_driver(struct libusb_device_handle *handle,
	int interface)
{
	int fd = __device_handle_priv(handle)->fd;
	struct usbfs_ioctl command;
	int r;

	command.ifno = interface;
	command.ioctl_code = IOCTL_USBFS_CONNECT;
	command.data = NULL;

	r = ioctl(fd, IOCTL_USBFS_IOCTL, &command);
	if (r < 0) {
		if (errno == ENODATA)
			return LIBUSB_ERROR_NOT_FOUND;
		else if (errno == EINVAL)
			return LIBUSB_ERROR_INVALID_PARAM;
		else if (errno == ENODEV)
			return LIBUSB_ERROR_NO_DEVICE;
		else if (errno == EBUSY)
			return LIBUSB_ERROR_BUSY;

		usbi_err(HANDLE_CTX(handle),
			"attach failed error %d errno %d", r, errno);
		return LIBUSB_ERROR_OTHER;
	} else if (r == 0) {
		return LIBUSB_ERROR_NOT_FOUND;
	}

	return 0;
}

static void op_destroy_device(struct libusb_device *dev)
{
	struct linux_device_priv *priv = __device_priv(dev);
	if (priv->dev_descriptor)
		free(priv->dev_descriptor);
	if (priv->config_descriptor)
		free(priv->config_descriptor);
}

static void free_iso_urbs(struct linux_transfer_priv *tpriv)
{
	int i;
	for (i = 0; i < tpriv->num_urbs; i++) {
		struct usbfs_urb *urb = tpriv->iso_urbs[i];
		if (!urb)
			break;
		free(urb);
	}

	free(tpriv->iso_urbs);
}

static int submit_bulk_transfer(struct usbi_transfer *itransfer,
	unsigned char urb_type)
{
	struct libusb_transfer *transfer =
		__USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);
	struct linux_transfer_priv *tpriv = usbi_transfer_get_os_priv(itransfer);
	struct linux_device_handle_priv *dpriv =
		__device_handle_priv(transfer->dev_handle);
	struct usbfs_urb *urbs;
	int r;
	int i;
	size_t alloc_size;

	/* usbfs places a 16kb limit on bulk URBs. we divide up larger requests
	 * into smaller units to meet such restriction, then fire off all the
	 * units at once. it would be simpler if we just fired one unit at a time,
	 * but there is a big performance gain through doing it this way. */
	int num_urbs = transfer->length / MAX_BULK_BUFFER_LENGTH;
	int last_urb_partial = 0;

	if ((transfer->length % MAX_BULK_BUFFER_LENGTH) > 0) {
		last_urb_partial = 1;
		num_urbs++;
	}
	usbi_dbg("need %d urbs for new transfer with length %d", num_urbs,
		transfer->length);
	alloc_size = num_urbs * sizeof(struct usbfs_urb);
	urbs = malloc(alloc_size);
	if (!urbs)
		return LIBUSB_ERROR_NO_MEM;
	memset(urbs, 0, alloc_size);
	tpriv->urbs = urbs;
	tpriv->num_urbs = num_urbs;
	tpriv->awaiting_discard = 0;
	tpriv->awaiting_reap = 0;
	tpriv->reap_action = NORMAL;

	for (i = 0; i < num_urbs; i++) {
		struct usbfs_urb *urb = &urbs[i];
		urb->usercontext = itransfer;
		urb->type = urb_type;
		urb->endpoint = transfer->endpoint;
		urb->buffer = transfer->buffer + (i * MAX_BULK_BUFFER_LENGTH);
		if (i == num_urbs - 1 && last_urb_partial)
			urb->buffer_length = transfer->length % MAX_BULK_BUFFER_LENGTH;
		else
			urb->buffer_length = MAX_BULK_BUFFER_LENGTH;

		r = ioctl(dpriv->fd, IOCTL_USBFS_SUBMITURB, urb);
		if (r < 0) {
			int j;

			if (errno == ENODEV) {
				r = LIBUSB_ERROR_NO_DEVICE;
			} else {
				usbi_err(TRANSFER_CTX(transfer),
					"submiturb failed error %d errno=%d", r, errno);
				r = LIBUSB_ERROR_IO;
			}
	
			/* if the first URB submission fails, we can simply free up and
			 * return failure immediately. */
			if (i == 0) {
				usbi_dbg("first URB failed, easy peasy");
				free(urbs);
				return r;
			}

			/* if it's not the first URB that failed, the situation is a bit
			 * tricky. we must discard all previous URBs. there are
			 * complications:
			 *  - discarding is asynchronous - discarded urbs will be reaped
			 *    later. the user must not have freed the transfer when the
			 *    discarded URBs are reaped, otherwise libusb will be using
			 *    freed memory.
			 *  - the earlier URBs may have completed successfully and we do
			 *    not want to throw away any data.
			 * so, in this case we discard all the previous URBs BUT we report
			 * that the transfer was submitted successfully. then later when
			 * the final discard completes we can report error to the user.
			 */
			tpriv->reap_action = SUBMIT_FAILED;
			for (j = 0; j < i; j++) {
				int tmp = ioctl(dpriv->fd, IOCTL_USBFS_DISCARDURB, &urbs[j]);
				if (tmp == 0)
					tpriv->awaiting_discard++;
				else if (errno == EINVAL)
					tpriv->awaiting_reap++;
				else
					usbi_warn(TRANSFER_CTX(transfer),
						"unrecognised discard return %d", tmp);
			}

			usbi_dbg("reporting successful submission but waiting for %d "
				"discards and %d reaps before reporting error",
				tpriv->awaiting_discard, tpriv->awaiting_reap);
			return 0;
		}
	}

	return 0;
}

static int submit_iso_transfer(struct usbi_transfer *itransfer)
{
	struct libusb_transfer *transfer =
		__USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);
	struct linux_transfer_priv *tpriv = usbi_transfer_get_os_priv(itransfer);
	struct linux_device_handle_priv *dpriv =
		__device_handle_priv(transfer->dev_handle);
	struct usbfs_urb **urbs;
	size_t alloc_size;
	int num_packets = transfer->num_iso_packets;
	int i;
	int this_urb_len = 0;
	int num_urbs = 1;
	int packet_offset = 0;
	unsigned int packet_len;
	unsigned char *urb_buffer = transfer->buffer;

	/* usbfs places a 32kb limit on iso URBs. we divide up larger requests
	 * into smaller units to meet such restriction, then fire off all the
	 * units at once. it would be simpler if we just fired one unit at a time,
	 * but there is a big performance gain through doing it this way. */

	/* calculate how many URBs we need */
	for (i = 0; i < num_packets; i++) {
		int space_remaining = MAX_ISO_BUFFER_LENGTH - this_urb_len;
		packet_len = transfer->iso_packet_desc[i].length;

		if (packet_len > space_remaining) {
			num_urbs++;
			this_urb_len = packet_len;
		} else {
			this_urb_len += packet_len;
		}
	}
	usbi_dbg("need %d 32k URBs for transfer", num_urbs);

	alloc_size = num_urbs * sizeof(*urbs);
	urbs = malloc(alloc_size);
	if (!urbs)
		return LIBUSB_ERROR_NO_MEM;
	memset(urbs, 0, alloc_size);

	tpriv->iso_urbs = urbs;
	tpriv->num_urbs = num_urbs;
	tpriv->awaiting_discard = 0;
	tpriv->awaiting_reap = 0;
	tpriv->reap_action = NORMAL;
	tpriv->iso_packet_offset = 0;

	/* allocate + initialize each URB with the correct number of packets */
	for (i = 0; i < num_urbs; i++) {
		struct usbfs_urb *urb;
		int space_remaining_in_urb = MAX_ISO_BUFFER_LENGTH;
		int urb_packet_offset = 0;
		unsigned char *urb_buffer_orig = urb_buffer;
		int j;
		int k;

		/* swallow up all the packets we can fit into this URB */
		while (packet_offset < transfer->num_iso_packets) {
			packet_len = transfer->iso_packet_desc[packet_offset].length;
			if (packet_len <= space_remaining_in_urb) {
				/* throw it in */
				urb_packet_offset++;
				packet_offset++;
				space_remaining_in_urb -= packet_len;
				urb_buffer += packet_len;
			} else {
				/* it can't fit, save it for the next URB */
				break;
			}
		}

		alloc_size = sizeof(*urb)
			+ (urb_packet_offset * sizeof(struct usbfs_iso_packet_desc));
		urb = malloc(alloc_size);
		if (!urb) {
			free_iso_urbs(tpriv);
			return LIBUSB_ERROR_NO_MEM;
		}
		memset(urb, 0, alloc_size);
		urbs[i] = urb;

		/* populate packet lengths */
		for (j = 0, k = packet_offset - urb_packet_offset;
				k < packet_offset; k++, j++) {
			packet_len = transfer->iso_packet_desc[k].length;
			urb->iso_frame_desc[j].length = packet_len;
		}

		urb->usercontext = itransfer;
		urb->type = USBFS_URB_TYPE_ISO;
		/* FIXME: interface for non-ASAP data? */
		urb->flags = USBFS_URB_ISO_ASAP;
		urb->endpoint = transfer->endpoint;
		urb->number_of_packets = urb_packet_offset;
		urb->buffer = urb_buffer_orig;
	}

	/* submit URBs */
	for (i = 0; i < num_urbs; i++) {
		int r = ioctl(dpriv->fd, IOCTL_USBFS_SUBMITURB, urbs[i]);
		if (r < 0) {
			int j;

			if (errno == ENODEV) {
				r = LIBUSB_ERROR_NO_DEVICE;
			} else {
				usbi_err(TRANSFER_CTX(transfer),
					"submiturb failed error %d errno=%d", r, errno);
				r = LIBUSB_ERROR_IO;
			}

			/* if the first URB submission fails, we can simply free up and
			 * return failure immediately. */
			if (i == 0) {
				usbi_dbg("first URB failed, easy peasy");
				free_iso_urbs(tpriv);
				return r;
			}

			/* if it's not the first URB that failed, the situation is a bit
			 * tricky. we must discard all previous URBs. there are
			 * complications:
			 *  - discarding is asynchronous - discarded urbs will be reaped
			 *    later. the user must not have freed the transfer when the
			 *    discarded URBs are reaped, otherwise libusb will be using
			 *    freed memory.
			 *  - the earlier URBs may have completed successfully and we do
			 *    not want to throw away any data.
			 * so, in this case we discard all the previous URBs BUT we report
			 * that the transfer was submitted successfully. then later when
			 * the final discard completes we can report error to the user.
			 */
			tpriv->reap_action = SUBMIT_FAILED;
			for (j = 0; j < i; j++) {
				int tmp = ioctl(dpriv->fd, IOCTL_USBFS_DISCARDURB, urbs[j]);
				if (tmp == 0)
					tpriv->awaiting_discard++;
				else if (errno == EINVAL)
					tpriv->awaiting_reap++;
				else
					usbi_warn(TRANSFER_CTX(transfer),
						"unrecognised discard return %d", tmp);
			}

			usbi_dbg("reporting successful submission but waiting for %d "
				"discards and %d reaps before reporting error",
				tpriv->awaiting_discard, tpriv->awaiting_reap);
			return 0;
		}
	}

	return 0;
}

static int submit_control_transfer(struct usbi_transfer *itransfer)
{
	struct linux_transfer_priv *tpriv = usbi_transfer_get_os_priv(itransfer);
	struct libusb_transfer *transfer =
		__USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);
	struct linux_device_handle_priv *dpriv =
		__device_handle_priv(transfer->dev_handle);
	struct usbfs_urb *urb;
	int r;

	if (transfer->length - LIBUSB_CONTROL_SETUP_SIZE > MAX_CTRL_BUFFER_LENGTH)
		return LIBUSB_ERROR_INVALID_PARAM;

	urb = malloc(sizeof(struct usbfs_urb));
	if (!urb)
		return LIBUSB_ERROR_NO_MEM;
	memset(urb, 0, sizeof(struct usbfs_urb));
	tpriv->urbs = urb;
	tpriv->reap_action = NORMAL;

	urb->usercontext = itransfer;
	urb->type = USBFS_URB_TYPE_CONTROL;
	urb->endpoint = transfer->endpoint;
	urb->buffer = transfer->buffer;
	urb->buffer_length = transfer->length;

	r = ioctl(dpriv->fd, IOCTL_USBFS_SUBMITURB, urb);
	if (r < 0) {
		free(urb);
		if (errno == ENODEV)
			return LIBUSB_ERROR_NO_DEVICE;

		usbi_err(TRANSFER_CTX(transfer),
			"submiturb failed error %d errno=%d", r, errno);
		return LIBUSB_ERROR_IO;
	}
	return 0;
}

static int op_submit_transfer(struct usbi_transfer *itransfer)
{
	struct libusb_transfer *transfer =
		__USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);

	switch (transfer->type) {
	case LIBUSB_TRANSFER_TYPE_CONTROL:
		return submit_control_transfer(itransfer);
	case LIBUSB_TRANSFER_TYPE_BULK:
		return submit_bulk_transfer(itransfer, USBFS_URB_TYPE_BULK);
	case LIBUSB_TRANSFER_TYPE_INTERRUPT:
		return submit_bulk_transfer(itransfer, USBFS_URB_TYPE_INTERRUPT);
	case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
		return submit_iso_transfer(itransfer);
	default:
		usbi_err(TRANSFER_CTX(transfer),
			"unknown endpoint type %d", transfer->type);
		return LIBUSB_ERROR_INVALID_PARAM;
	}
}

static int cancel_control_transfer(struct usbi_transfer *itransfer)
{
	struct linux_transfer_priv *tpriv = usbi_transfer_get_os_priv(itransfer);
	struct libusb_transfer *transfer =
		__USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);
	struct linux_device_handle_priv *dpriv =
		__device_handle_priv(transfer->dev_handle);
	int r;

	tpriv->reap_action = CANCELLED;
	r = ioctl(dpriv->fd, IOCTL_USBFS_DISCARDURB, tpriv->urbs);
	if(r) {
		if (errno == EINVAL) {
			usbi_dbg("URB not found --> assuming ready to be reaped");
			return 0;
		} else {
			usbi_err(TRANSFER_CTX(transfer),
				"unrecognised DISCARD code %d", errno);
			return LIBUSB_ERROR_OTHER;
		}
	}

	return 0;
}

static void cancel_bulk_transfer(struct usbi_transfer *itransfer)
{
	struct linux_transfer_priv *tpriv = usbi_transfer_get_os_priv(itransfer);
	struct libusb_transfer *transfer =
		__USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);
	struct linux_device_handle_priv *dpriv =
		__device_handle_priv(transfer->dev_handle);
	int i;

	tpriv->reap_action = CANCELLED;
	tpriv->awaiting_reap = 0;
	tpriv->awaiting_discard = 0;
	for (i = 0; i < tpriv->num_urbs; i++) {
		int tmp = ioctl(dpriv->fd, IOCTL_USBFS_DISCARDURB, &tpriv->urbs[i]);
		if (tmp == 0)
			tpriv->awaiting_discard++;
		else if (errno == EINVAL)
			tpriv->awaiting_reap++;
		else
			usbi_warn(TRANSFER_CTX(transfer),
				"unrecognised discard return %d", errno);
	}
}

static void cancel_iso_transfer(struct usbi_transfer *itransfer)
{
	struct linux_transfer_priv *tpriv = usbi_transfer_get_os_priv(itransfer);
	struct libusb_transfer *transfer =
		__USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);
	struct linux_device_handle_priv *dpriv =
		__device_handle_priv(transfer->dev_handle);
	int i;

	tpriv->reap_action = CANCELLED;
	tpriv->awaiting_reap = 0;
	tpriv->awaiting_discard = 0;
	for (i = 0; i < tpriv->num_urbs; i++) {
		int tmp = ioctl(dpriv->fd, IOCTL_USBFS_DISCARDURB, tpriv->iso_urbs[i]);
		if (tmp == 0)
			tpriv->awaiting_discard++;
		else if (errno == EINVAL)
			tpriv->awaiting_reap++;
		else
			usbi_warn(TRANSFER_CTX(transfer),
				"unrecognised discard return %d", errno);
	}
}

static int op_cancel_transfer(struct usbi_transfer *itransfer)
{
	struct libusb_transfer *transfer =
		__USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);

	switch (transfer->type) {
	case LIBUSB_TRANSFER_TYPE_CONTROL:
		return cancel_control_transfer(itransfer);
	case LIBUSB_TRANSFER_TYPE_BULK:
	case LIBUSB_TRANSFER_TYPE_INTERRUPT:
		cancel_bulk_transfer(itransfer);
		return 0;
	case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
		cancel_iso_transfer(itransfer);
		return 0;
	default:
		usbi_err(TRANSFER_CTX(transfer),
			"unknown endpoint type %d", transfer->type);
		return LIBUSB_ERROR_INVALID_PARAM;
	}
}

static void op_clear_transfer_priv(struct usbi_transfer *itransfer)
{
	struct libusb_transfer *transfer =
		__USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);
	struct linux_transfer_priv *tpriv = usbi_transfer_get_os_priv(itransfer);

	switch (transfer->type) {
	case LIBUSB_TRANSFER_TYPE_CONTROL:
	case LIBUSB_TRANSFER_TYPE_BULK:
	case LIBUSB_TRANSFER_TYPE_INTERRUPT:
		free(tpriv->urbs);
		break;
	case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
		free_iso_urbs(tpriv);
		break;
	default:
		usbi_err(TRANSFER_CTX(transfer),
			"unknown endpoint type %d", transfer->type);
	}
}

static int handle_bulk_completion(struct usbi_transfer *itransfer,
	struct usbfs_urb *urb)
{
	struct linux_transfer_priv *tpriv = usbi_transfer_get_os_priv(itransfer);
	int num_urbs = tpriv->num_urbs;
	int urb_idx = urb - tpriv->urbs;
	enum libusb_transfer_status status = LIBUSB_TRANSFER_COMPLETED;

	usbi_dbg("handling completion status %d of bulk urb %d/%d", urb->status,
		urb_idx + 1, num_urbs);

	if (urb->status == 0 ||
			(urb->status == -EOVERFLOW && urb->actual_length > 0))
		itransfer->transferred += urb->actual_length;

	if (tpriv->reap_action != NORMAL) {
		/* cancelled, submit_fail, or completed early */
		if (urb->status == -ENOENT) {
			usbi_dbg("CANCEL: detected a cancelled URB");
			if (tpriv->awaiting_discard == 0)
				usbi_err(ITRANSFER_CTX(itransfer),
					"CANCEL: cancelled URB but not awaiting discards?");
			else
				tpriv->awaiting_discard--;
		} else if (urb->status == 0) {
			usbi_dbg("CANCEL: detected a completed URB");

			/* FIXME we could solve this extreme corner case with a memmove
			 * or something */
			if (tpriv->reap_action == COMPLETED_EARLY)
				usbi_warn(ITRANSFER_CTX(itransfer), "SOME DATA LOST! "
					"(completed early but remaining urb completed)");

			if (tpriv->awaiting_reap == 0)
				usbi_err(ITRANSFER_CTX(itransfer),
					"CANCEL: completed URB not awaiting reap?");
			else
				tpriv->awaiting_reap--;
		} else if (urb->status == -EPIPE || urb->status == -EOVERFLOW) {
			if (tpriv->awaiting_reap == 0)
				usbi_err(ITRANSFER_CTX(itransfer),
					"CANCEL: completed URB not awaiting reap?");
			else
				tpriv->awaiting_reap--;
		} else {
			usbi_warn(ITRANSFER_CTX(itransfer),
				"unhandled CANCEL urb status %d", urb->status);
		}

		if (tpriv->awaiting_reap == 0 && tpriv->awaiting_discard == 0) {
			usbi_dbg("CANCEL: last URB handled, reporting");
			if (tpriv->reap_action == CANCELLED) {
				free(tpriv->urbs);
				usbi_handle_transfer_cancellation(itransfer);
				return 0;
			} else if (tpriv->reap_action == COMPLETED_EARLY) {
				goto out;
			} else {
				status = LIBUSB_TRANSFER_ERROR;
				goto out;
			}
		}
		return 0;
	}

	switch (urb->status) {
	case 0:
		break;
	case -EPIPE:
		usbi_dbg("detected endpoint stall");
		status = LIBUSB_TRANSFER_STALL;
		goto out;
	case -EOVERFLOW:
		/* overflow can only ever occur in the last urb */
		usbi_dbg("overflow, actual_length=%d", urb->actual_length);
		status = LIBUSB_TRANSFER_OVERFLOW;
		goto out;
	case -ETIME:
	case -EPROTO:
	case -EILSEQ:
		usbi_dbg("low level error %d", urb->status);
		status = LIBUSB_TRANSFER_ERROR;
		goto out;
	default:
		usbi_warn(ITRANSFER_CTX(itransfer),
			"unrecognised urb status %d", urb->status);
		status = LIBUSB_TRANSFER_ERROR;
		goto out;
	}

	/* if we're the last urb or we got less data than requested then we're
	 * done */
	if (urb_idx == num_urbs - 1) {
		usbi_dbg("last URB in transfer --> complete!");
	} else if (urb->actual_length < urb->buffer_length) {
		struct libusb_transfer *transfer =
			__USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);
		struct linux_device_handle_priv *dpriv =
			__device_handle_priv(transfer->dev_handle);
		int i;

		usbi_dbg("short transfer %d/%d --> complete!", urb->actual_length,
			urb->buffer_length);

		/* we have to cancel the remaining urbs and wait for their completion
		 * before reporting results */
		tpriv->reap_action = COMPLETED_EARLY;
		for (i = urb_idx + 1; i < tpriv->num_urbs; i++) {
			int r = ioctl(dpriv->fd, IOCTL_USBFS_DISCARDURB, &tpriv->urbs[i]);
			if (r == 0)
				tpriv->awaiting_discard++;
			else if (errno == EINVAL)
				tpriv->awaiting_reap++;
			else
				usbi_warn(ITRANSFER_CTX(itransfer),
					"unrecognised discard return %d", errno);
		}
		return 0;
	} else {
		return 0;
	}

out:
	free(tpriv->urbs);
	usbi_handle_transfer_completion(itransfer, status);
	return 0;
}

static int handle_iso_completion(struct usbi_transfer *itransfer,
	struct usbfs_urb *urb)
{
	struct libusb_transfer *transfer =
		__USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);
	struct linux_transfer_priv *tpriv = usbi_transfer_get_os_priv(itransfer);
	int num_urbs = tpriv->num_urbs;
	int urb_idx = 0;
	int i;

	for (i = 0; i < num_urbs; i++) {
		if (urb == tpriv->iso_urbs[i]) {
			urb_idx = i + 1;
			break;
		}
	}
	if (urb_idx == 0) {
		usbi_err(TRANSFER_CTX(transfer), "could not locate urb!");
		return LIBUSB_ERROR_NOT_FOUND;
	}

	usbi_dbg("handling completion status %d of iso urb %d/%d", urb->status,
		urb_idx, num_urbs);

	if (urb->status == 0) {
		/* copy isochronous results back in */

		for (i = 0; i < urb->number_of_packets; i++) {
			struct usbfs_iso_packet_desc *urb_desc = &urb->iso_frame_desc[i];
			struct libusb_iso_packet_descriptor *lib_desc =
				&transfer->iso_packet_desc[tpriv->iso_packet_offset++];
			lib_desc->status = urb_desc->status;
			lib_desc->actual_length = urb_desc->actual_length;
		}
	}

	if (tpriv->reap_action != NORMAL) { /* cancelled or submit_fail */
		if (urb->status == -ENOENT) {
			usbi_dbg("CANCEL: detected a cancelled URB");
			if (tpriv->awaiting_discard == 0)
				usbi_err(TRANSFER_CTX(transfer),
					"CANCEL: cancelled URB but not awaiting discards?");
			else
				tpriv->awaiting_discard--;
		} else if (urb->status == 0) {
			usbi_dbg("CANCEL: detected a completed URB");
			if (tpriv->awaiting_reap == 0)
				usbi_err(TRANSFER_CTX(transfer),
					"CANCEL: completed URB not awaiting reap?");
			else
				tpriv->awaiting_reap--;
		} else {
			usbi_warn(TRANSFER_CTX(transfer),
				"unhandled CANCEL urb status %d", urb->status);
		}

		if (tpriv->awaiting_reap == 0 && tpriv->awaiting_discard == 0) {
			usbi_dbg("CANCEL: last URB handled, reporting");
			free_iso_urbs(tpriv);
			if (tpriv->reap_action == CANCELLED)
				usbi_handle_transfer_cancellation(itransfer);
			else
				usbi_handle_transfer_completion(itransfer,
					LIBUSB_TRANSFER_ERROR);
		}
		return 0;
	}

	switch (urb->status) {
	case 0:
		break;
	case -ETIME:
	case -EPROTO:
	case -EILSEQ:
		usbi_dbg("low-level USB error %d", urb->status);
		break;
	default:
		usbi_warn(TRANSFER_CTX(transfer),
			"unrecognised urb status %d", urb->status);
		break;
	}

	/* if we're the last urb or we got less data than requested then we're
	 * done */
	if (urb_idx == num_urbs) {
		usbi_dbg("last URB in transfer --> complete!");
		free_iso_urbs(tpriv);
		usbi_handle_transfer_completion(itransfer, LIBUSB_TRANSFER_COMPLETED);
	}

	return 0;
}

static int handle_control_completion(struct usbi_transfer *itransfer,
	struct usbfs_urb *urb)
{
	struct linux_transfer_priv *tpriv = usbi_transfer_get_os_priv(itransfer);
	int status;

	usbi_dbg("handling completion status %d", urb->status);

	if (urb->status == 0)
		itransfer->transferred += urb->actual_length;

	if (tpriv->reap_action == CANCELLED) {
		if (urb->status != 0 && urb->status != -ENOENT)
			usbi_warn(ITRANSFER_CTX(itransfer),
				"cancel: unrecognised urb status %d", urb->status);
		free(tpriv->urbs);
		usbi_handle_transfer_cancellation(itransfer);
		return 0;
	}

	switch (urb->status) {
	case 0:
		itransfer->transferred = urb->actual_length;
		status = LIBUSB_TRANSFER_COMPLETED;
		break;
	case -EPIPE:
		usbi_dbg("unsupported control request");
		status = LIBUSB_TRANSFER_STALL;
		break;
	case -ETIME:
	case -EPROTO:
	case -EILSEQ:
		usbi_dbg("low-level bus error occurred");
		status = LIBUSB_TRANSFER_ERROR;
		break;
	default:
		usbi_warn(ITRANSFER_CTX(itransfer),
			"unrecognised urb status %d", urb->status);
		status = LIBUSB_TRANSFER_ERROR;
		break;
	}

	free(tpriv->urbs);
	usbi_handle_transfer_completion(itransfer, status);
	return 0;
}

static int reap_for_handle(struct libusb_device_handle *handle)
{
	struct linux_device_handle_priv *hpriv = __device_handle_priv(handle);
	int r;
	struct usbfs_urb *urb;
	struct usbi_transfer *itransfer;
	struct libusb_transfer *transfer;

	r = ioctl(hpriv->fd, IOCTL_USBFS_REAPURBNDELAY, &urb);
	if (r == -1 && errno == EAGAIN)
		return 1;
	if (r < 0) {
		if (errno == ENODEV)
			return LIBUSB_ERROR_NO_DEVICE;

		usbi_err(HANDLE_CTX(handle), "reap failed error %d errno=%d",
			r, errno);
		return LIBUSB_ERROR_IO;
	}

	itransfer = urb->usercontext;
	transfer = __USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);

	usbi_dbg("urb type=%d status=%d transferred=%d", urb->type, urb->status,
		urb->actual_length);

	switch (transfer->type) {
	case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
		return handle_iso_completion(itransfer, urb);
	case LIBUSB_TRANSFER_TYPE_BULK:
	case LIBUSB_TRANSFER_TYPE_INTERRUPT:
		return handle_bulk_completion(itransfer, urb);
	case LIBUSB_TRANSFER_TYPE_CONTROL:
		return handle_control_completion(itransfer, urb);
	default:
		usbi_err(HANDLE_CTX(handle), "unrecognised endpoint type %x",
			transfer->type);
		return LIBUSB_ERROR_OTHER;
	}
}

static int op_handle_events(struct libusb_context *ctx,
	struct pollfd *fds, nfds_t nfds, int num_ready)
{
	int r;
	int i = 0;

	pthread_mutex_lock(&ctx->open_devs_lock);
	for (i = 0; i < nfds && num_ready > 0; i++) {
		struct pollfd *pollfd = &fds[i];
		struct libusb_device_handle *handle;
		struct linux_device_handle_priv *hpriv = NULL;

		if (!pollfd->revents)
			continue;

		num_ready--;
		list_for_each_entry(handle, &ctx->open_devs, list) {
			hpriv =  __device_handle_priv(handle);
			if (hpriv->fd == pollfd->fd)
				break;
		}

		if (pollfd->revents & POLLERR) {
			usbi_remove_pollfd(HANDLE_CTX(handle), hpriv->fd);
			usbi_handle_disconnect(handle);
			continue;
		}

		r = reap_for_handle(handle);
		if (r == 1 || r == LIBUSB_ERROR_NO_DEVICE)
			continue;
		else if (r < 0)
			goto out;
	}

	r = 0;
out:
	pthread_mutex_unlock(&ctx->open_devs_lock);
	return r;
}

const struct usbi_os_backend android_usbfs_backend = {
	.name = "Linux usbfs",
	.init = op_init,
	.exit = NULL,
	.get_device_list = op_get_device_list,
	.get_device_descriptor = op_get_device_descriptor,
	.get_active_config_descriptor = op_get_active_config_descriptor,
	.get_config_descriptor = op_get_config_descriptor,

	.open = op_open,
	.close = op_close,
	.get_configuration = op_get_configuration,
	.set_configuration = op_set_configuration,
	.claim_interface = op_claim_interface,
	.release_interface = op_release_interface,

	.set_interface_altsetting = op_set_interface,
	.clear_halt = op_clear_halt,
	.reset_device = op_reset_device,

	.kernel_driver_active = op_kernel_driver_active,
	.detach_kernel_driver = op_detach_kernel_driver,
	.attach_kernel_driver = op_attach_kernel_driver,

	.destroy_device = op_destroy_device,

	.submit_transfer = op_submit_transfer,
	.cancel_transfer = op_cancel_transfer,
	.clear_transfer_priv = op_clear_transfer_priv,

	.handle_events = op_handle_events,

	.device_priv_size = sizeof(struct linux_device_priv),
	.device_handle_priv_size = sizeof(struct linux_device_handle_priv),
	.transfer_priv_size = sizeof(struct linux_transfer_priv),
	.add_iso_packet_size = 0,
};

