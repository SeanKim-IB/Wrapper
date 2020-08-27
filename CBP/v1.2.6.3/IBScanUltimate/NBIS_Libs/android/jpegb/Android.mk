LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := jpegb
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libjpegb.a
include $(PREBUILT_STATIC_LIBRARY)
