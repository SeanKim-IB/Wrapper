LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := png
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libpng.a
include $(PREBUILT_STATIC_LIBRARY)
