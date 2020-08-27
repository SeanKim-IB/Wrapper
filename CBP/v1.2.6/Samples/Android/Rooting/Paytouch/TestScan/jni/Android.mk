# NDK Make file for TestScan

LOCAL_PATH      := $(call my-dir)
#TARGET_ARCH     := arm
TARGET_PLATFORM := android-9
include $(CLEAR_VARS)

LOCAL_CFLAGS += -D__linux__ -D__embedded__ -D__android__ -W -Wall

LOCAL_C_INCLUDES       := $(LOCAL_PATH)/../../../../include
LOCAL_SRC_FILES        := testScanU.cpp
LOCAL_MODULE           := testScanU
LOCAL_LDLIBS           := -llog
LOCAL_SHARED_LIBRARIES += usb1.0 ibscanultimate

include $(BUILD_EXECUTABLE)

$(call import-module, libusb1.0)
$(call import-module, IBScanUltimate)
