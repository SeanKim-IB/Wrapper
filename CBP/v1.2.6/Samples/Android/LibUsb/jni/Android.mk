# NDK Make file for LIBUSB

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

#LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES:= \
	libusb/core.c \
	libusb/descriptor.c \
	libusb/io.c \
	libusb/sync.c \
	libusb/os/android_usbfs.c \
	libusb/compat/core.c


LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/android \
	$(LOCAL_PATH)/libusb \
	$(LOCAL_PATH)/libusb/os \
	$(LOCAL_PATH)/libusb/compat

LOCAL_CFLAGS += -W -Wall -fPIC -DPIC -Wno-sign-compare

LOCAL_MODULE:= usb

LOCAL_LDLIBS := -llog

LOCAL_PRELINK_MODULE := false 

include $(BUILD_SHARED_LIBRARY)
