LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE     := pcautil
LOCAL_SRC_FILES  := $(TARGET_ARCH_ABI)/libpcautil.a
include $(PREBUILT_STATIC_LIBRARY)
