# NDK Make file for LIBUSB

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

#LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES:= \
	libusb/core.c \
	libusb/descriptor.c \
	libusb/hotplug.c \
	libusb/io.c \
	libusb/sync.c \
	libusb/strerror.c \
	libusb/os/linux_usbfs.c \
	libusb/os/poll_posix.c \
	libusb/os/threads_posix.c \
	libusb/os/linux_netlink.c \


LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/android \
	$(LOCAL_PATH)/libusb \
	$(LOCAL_PATH)/libusb/os

LOCAL_CFLAGS += -W -Wall -fPIC -DPIC -Wno-sign-compare

LOCAL_MODULE:= usb1.0

LOCAL_LDLIBS := -llog

LOCAL_PRELINK_MODULE := false 

include $(BUILD_SHARED_LIBRARY)
