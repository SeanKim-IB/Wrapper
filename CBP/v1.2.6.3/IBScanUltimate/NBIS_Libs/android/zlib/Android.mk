LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := zlib
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libzlib.a
include $(PREBUILT_STATIC_LIBRARY)
